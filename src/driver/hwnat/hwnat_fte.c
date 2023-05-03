/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_fte.c#1 $
*/
/************************************************************************
 *
 *	Copyright (C) 2010 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * ected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
/*
** $Log: hwnat_fte.c,v $
** Revision 1.8  2011/06/30 12:07:32  lino
** hwnat enhance: IPv6 and QinQ support
**
** Revision 1.7  2011/06/10 04:37:49  lino
** add RT65168 support
**
** Revision 1.6  2011/06/09 13:58:17  lino
** add RT65168 support
**
** Revision 1.5  2011/06/09 13:47:20  lino
** add RT65168 support
**
** Revision 1.4  2011/06/08 10:02:23  lino
** add RT65168 support
**
** Revision 1.3  2011/06/03 10:35:04  lino
** add RT65168 support
**
** Revision 1.2  2011/06/03 02:41:44  lino
** add RT65168 support
**
** Revision 1.1.2.1  2011/04/01 09:16:41  lino
** add RT65168 support
**
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/crc32.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/mii.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/pktflow.h>
#include <net/ip.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/cmdparse.h>
#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE)
#include <linux/if_bridge.h>
#include "../net/bridge/br_private.h"
#endif

#include "hwnat.h"
#include "hwnat_reg.h"
#include "hwnat_fte.h"
#include "hwnat_mac.h"
#include "hwnat_pktflow.h"

#ifdef HWNAT_SW_MODEL
#include <hwnat_emulation.h>
#endif

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/
#ifdef HWNAT_SW_MODEL
#define HWNAT_INT						SWR_INT
#else
#define HWNAT_INT						18
#endif

/* define tuple index */
#define FLOW_IPV4_TCP_UDP				0
#define FLOW_PPPOE_IPV4_TCP_UDP			1
#define FLOW_IPV4_ICMP					2
#define FLOW_PPPOE_IPV4_ICMP			3
#define FLOW_IPV4_GRE					4
#define FLOW_PPPOE_IPV4_GRE				5
#define FLOW_IPV4_ESP					6
#define FLOW_PPPOE_IPV4_ESP				7
#define FLOW_IPV4_ONLY					8
#define FLOW_PPPOE_IPV4_ONLY			9
#define FLOW_IPV6						10
#define FLOW_PPPOE_IPV6					11

#define FLOW_BRIDGE						0
#define FLOW_NAT_ROUTING				1
#define FLOW_PURE_ROUTING				2

/* define high/low priority */
#define FLOW_HIGH_PRIORITY				1
#define FLOW_LOW_PRIORITY				0

/* define flow alloc fail try counter for high priority */
#define FLOW_RETRY_ALLOC				5

/************************************************************************
*                            M A C R O S
*************************************************************************
*/

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/

typedef struct {
	uint8	protocol;
    uint32	saddr;          
    uint32	daddr;          
    uint16	sport;    
    uint16	dport;       
} flow_ipv4_tuple_t;

typedef struct {
	uint8	protocol;
	struct in6_addr daddr;
} flow_ipv6_tuple_t;

typedef struct {
	uint8   tuple_index;
	uint8	ipv6_protocol;

	union {
		flow_ipv4_tuple_t ipv4_tuple;		
		flow_ipv6_tuple_t ipv6_tuple;		
	};
} flow_tuple_t;

typedef struct {
	uint8	mac_da;
	uint8	mac_sa;

	union {
    	uint32	saddr;          
    	uint32	daddr;          
	};

	union {
    	uint16	sport;    
    	uint16	dport;       
	};

	uint16	vlan_tag;
	uint16	pppoe_sess_id;
	uint16	outer_vlan_tag;
	uint8	tc2206_tag;
} flow_arg_t;

typedef struct {
	uint8 head;
} hash_t;

typedef struct {
	struct list_head list;

	uint8 mode;
	uint16 hash;
	uint16 entry;
	uint8 next;
	uint8 valid;
	uint16 vlan_tag;
	uint8 qid;
	flow_tuple_t flow_tuple;
	flow_arg_t flow_arg;
	uint16 tx_portmap;
	uint8 action;
	uint8 time;
	uint8 time_init;
	uint32 pkt_cnt;

	struct nf_conn *ct;
	uint8 ct_dir;

	uint8 priority; 		/* indicate high priority flow */

	struct net_bridge_fdb_entry *fdb_src;
} flow_t;

typedef struct {
	struct list_head flow_hash[HASH_TBL_SIZE];

	struct list_head flow_free;
	int nr_flow_free;

	struct timer_list timer;			/* scan timer */
	uint8 timeout;

	uint8 qid;

	/* add flow or not */
	uint8 add_flow_on;

	/* debug */
	uint8 dbg;
	uint8 dbg_protocol;
	uint16 dbg_rxportmap;
	uint16 dbg_txportmap;
} flow_ctl_t;

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

static void hwnat_hash_init(void);
static void hwnat_hash_tbl_write(uint16 entry, uint16 head);
static void hwnat_hash_tbl_read(uint16 entry, uint16 *head);

static void hwnat_flow_init(void);
static void hwnat_flow_tbl_write(uint16 entry, flow_t *flow);
static void hwnat_flow_tbl_read(uint16 entry, flow_t *flow);
static void hwnat_flow_tbl_set(uint16 entry, uint16 tx_portmap, uint16 action, uint8 qid);

/************************************************************************
*                        P U B L I C   D A T A
*************************************************************************
*/

/************************************************************************
*                      E X T E R N A L   D A T A
*************************************************************************
*/

/************************************************************************
*                       P R I V A T E   D A T A
*************************************************************************
*/

//hash_t hash_tbl[HASH_TBL_SIZE];

flow_t flow_tbl[FLOW_TBL_SIZE];

flow_ctl_t flow_ctl;

DEFINE_SPINLOCK(fte_lock);

#ifdef HWNAT_SW_MODEL
struct timer_list fte_timer;
#endif

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

uint16 flow_hash_fn(uint8 *tuple, int len)
{
	int i;
	uint8 tuple_crc[FLOW_TUPLE_LEN];

	/* reverse tuple key */
	for (i = FLOW_TUPLE_LEN - 1; i >= 0; i--)
		tuple_crc[i] = tuple[FLOW_TUPLE_LEN - 1 - i];

	return (ether_crc(len, tuple_crc)>>23) & 0x1ff;
}

void flow_make_tuple(flow_tuple_t *flow_tuple, uint8 *tuple)
{
	memset(tuple, 0x55, FLOW_TUPLE_LEN);

	/* ipv4 tuple */
	if (flow_tuple->ipv6_protocol == 0) {
		if ((flow_tuple->tuple_index == FLOW_IPV4_ONLY) || 
				(flow_tuple->tuple_index == FLOW_PPPOE_IPV4_ONLY))
			tuple += 10;
		else
			tuple += 6;
		*tuple++ = flow_tuple->tuple_index;
		*tuple++ = flow_tuple->ipv4_tuple.protocol;
		memcpy(tuple, &flow_tuple->ipv4_tuple.saddr, sizeof(uint32));
		tuple += sizeof(uint32);
		memcpy(tuple, &flow_tuple->ipv4_tuple.daddr, sizeof(uint32));
		tuple += sizeof(uint32);
		if ((flow_tuple->tuple_index != FLOW_IPV4_ONLY) && 
				(flow_tuple->tuple_index != FLOW_PPPOE_IPV4_ONLY)) {
			memcpy(tuple, &flow_tuple->ipv4_tuple.sport, sizeof(uint16));
			tuple += sizeof(uint16);
		}
		if ((flow_tuple->tuple_index != FLOW_IPV4_ONLY) && 
				(flow_tuple->tuple_index != FLOW_PPPOE_IPV4_ONLY)) {
			memcpy(tuple, &flow_tuple->ipv4_tuple.dport, sizeof(uint16));
			tuple += sizeof(uint16);
		}
	} else {
	/* ipv6 tuple */
		tuple += 2;
		*tuple++ = flow_tuple->tuple_index;
		*tuple++ = flow_tuple->ipv6_tuple.protocol;
		memcpy(tuple, &flow_tuple->ipv6_tuple.daddr, 16);
	}
}

void flow_make_arg(flow_arg_t *flow_arg, uint8 *arg)
{
	memset(arg, 0, FLOW_ARG_LEN);
	*arg++ = flow_arg->mac_da;
	*arg++ = flow_arg->mac_sa;
	memcpy(arg, &flow_arg->saddr, sizeof(uint32));
	arg += sizeof(uint32);
	memcpy(arg, &flow_arg->sport, sizeof(uint16));
	arg += sizeof(uint16);
	memcpy(arg, &flow_arg->vlan_tag, sizeof(uint16));
	arg += sizeof(uint16);
	memcpy(arg, &flow_arg->pppoe_sess_id, sizeof(uint16));
	arg += sizeof(uint16);
	memcpy(arg, &flow_arg->outer_vlan_tag, sizeof(uint16));
	arg += sizeof(uint16);
	memcpy(arg, &flow_arg->tc2206_tag, sizeof(uint8));
	arg += sizeof(uint8);
}

flow_t *flow_alloc(uint8 priority)
{
	flow_t *flow = NULL;
	unsigned long flags;
	int retry = 0;

flow_alloc_retry:

	spin_lock_irqsave(&fte_lock, flags);

	if (list_empty(&flow_ctl.flow_free))
		goto flow_alloc_exit;

	flow = list_entry(flow_ctl.flow_free.next, flow_t, list);
	list_del(&flow->list);

flow_alloc_exit:

	spin_unlock_irqrestore(&fte_lock, flags);

	if (priority && (flow == NULL)) {
		/* remove oldest flow */
		hwnat_flow_remove_by_oldest();
		retry++;
		if (retry <= FLOW_RETRY_ALLOC)
			goto flow_alloc_retry;
	}

	return flow;
}

void flow_free(flow_t *flow)
{
	unsigned long flags;

	spin_lock_irqsave(&fte_lock, flags);
	list_add_tail(&flow->list, &flow_ctl.flow_free);
	spin_unlock_irqrestore(&fte_lock, flags);
}

void flow_init(uint8 mode, flow_t *flow, flow_tuple_t *flow_tuple, flow_arg_t *flow_arg, 
				uint16 tx_portmap, uint16 action, struct nf_conn *ct, uint8 ct_dir, uint8 qid,
				uint8 priority, struct net_bridge_fdb_entry *fdb_src)
{
	flow->mode = mode;
	flow->valid = 1;
	flow->vlan_tag = 0;
	flow->qid = qid;

	/* init flow tuple */
	flow->flow_tuple.tuple_index = flow_tuple->tuple_index;
	flow->flow_tuple.ipv6_protocol = flow_tuple->ipv6_protocol;
	if (flow_tuple->ipv6_protocol == 0) {
		flow->flow_tuple.ipv4_tuple.protocol = flow_tuple->ipv4_tuple.protocol;
		flow->flow_tuple.ipv4_tuple.saddr = flow_tuple->ipv4_tuple.saddr;
		flow->flow_tuple.ipv4_tuple.daddr = flow_tuple->ipv4_tuple.daddr;
		flow->flow_tuple.ipv4_tuple.sport = flow_tuple->ipv4_tuple.sport;
		flow->flow_tuple.ipv4_tuple.dport = flow_tuple->ipv4_tuple.dport;
	} else {
		flow->flow_tuple.ipv6_tuple.protocol = flow_tuple->ipv6_tuple.protocol;
		memcpy(&flow->flow_tuple.ipv6_tuple.daddr, &flow_tuple->ipv6_tuple.daddr, 16);
	}

	/* init flow argument */
	flow->flow_arg.mac_da = flow_arg->mac_da;
	flow->flow_arg.mac_sa = flow_arg->mac_sa;
	flow->flow_arg.saddr = flow_arg->saddr;
	flow->flow_arg.sport = flow_arg->sport;
	flow->flow_arg.vlan_tag = flow_arg->vlan_tag;
	flow->flow_arg.outer_vlan_tag = flow_arg->outer_vlan_tag;
	flow->flow_arg.pppoe_sess_id = flow_arg->pppoe_sess_id;
	flow->flow_arg.tc2206_tag = flow_arg->tc2206_tag;

	flow->tx_portmap = tx_portmap;
	flow->action = action;
	flow->time = flow_ctl.timeout;
	flow->time_init = flow_ctl.timeout;
	flow->pkt_cnt = 0;

	/* log netfilter conntrack */
	flow->ct = ct;
	flow->ct_dir = ct_dir;

	/* record high priority or not */
	flow->priority = priority;

	/* log bridge fdb */
	flow->fdb_src = fdb_src;
}

void flow_insert(flow_t *flow)
{
	uint8 tuple[FLOW_TUPLE_LEN];
	uint16 hash;
	flow_t *head_flow;
	uint16 head;
	unsigned long flags;

	spin_lock_irqsave(&fte_lock, flags);

	flow_make_tuple(&flow->flow_tuple, tuple);

	hash = flow_hash_fn(tuple, FLOW_TUPLE_LEN);

	head = flow->entry;
	flow->hash = hash;
	if (list_empty(&flow_ctl.flow_hash[hash])) {
		flow->next = 0;
	} else {
		head_flow = list_entry(flow_ctl.flow_hash[hash].next, flow_t, list);

		flow->next = head_flow->entry;
	}

	hwnat_flow_tbl_write(flow->entry, flow);
	hwnat_hash_tbl_write(hash, head);

	list_add(&flow->list, &flow_ctl.flow_hash[hash]);
	flow_ctl.nr_flow_free--;

	if (flow->ct) {
   		flow->ct->pktflow_key[flow->ct_dir] = flow->entry;

		set_bit(IPS_PKTFLOWED_BIT, &flow->ct->status);  
	}

	spin_unlock_irqrestore(&fte_lock, flags);
			
	if (hwnat_debug) {
		printk("INSERT e=%d ff=%d h=%d", flow->entry, flow_ctl.nr_flow_free, hash);
		if (flow->flow_tuple.ipv6_protocol) {
			printk("<%u><" NIP6_FMT "> ",
				flow->flow_tuple.ipv6_tuple.protocol,
				NIP6(flow->flow_tuple.ipv6_tuple.daddr));
		} else {
			printk("<%u><" NIPQUAD_FMT ":%u " NIPQUAD_FMT ":%u> ",
				flow->flow_tuple.ipv4_tuple.protocol,
				NIPQUAD(flow->flow_tuple.ipv4_tuple.saddr), flow->flow_tuple.ipv4_tuple.sport,
				NIPQUAD(flow->flow_tuple.ipv4_tuple.daddr), flow->flow_tuple.ipv4_tuple.dport);
		}
		printk("ct=%p ct_status=%lx dir=%d\n",
			flow->ct, flow->ct ? flow->ct->status : 0, flow->ct_dir);
	}
}

void flow_remove(uint16 entry)
{
	flow_t *flow;
	flow_t *prev_flow;
	flow_t *next_flow;
	uint16 hash;
	unsigned long flags;

	flow = &flow_tbl[entry];

	if (flow_ctl.timeout == 0)
		return;
		
	if (flow->valid == 0)
		return;
				
	if (hwnat_debug) {
		printk("REMOVE e=%d ff=%d ", entry, flow_ctl.nr_flow_free);
		if (flow->flow_tuple.ipv6_protocol) {
			printk("<%u><" NIP6_FMT ">\n",
				flow->flow_tuple.ipv6_tuple.protocol,
				NIP6(flow->flow_tuple.ipv6_tuple.daddr));
		} else {
			printk("<%u><" NIPQUAD_FMT ":%u " NIPQUAD_FMT ":%u>\n",
				flow->flow_tuple.ipv4_tuple.protocol,
				NIPQUAD(flow->flow_tuple.ipv4_tuple.saddr), flow->flow_tuple.ipv4_tuple.sport,
				NIPQUAD(flow->flow_tuple.ipv4_tuple.daddr), flow->flow_tuple.ipv4_tuple.dport);
		}
	}

	spin_lock_irqsave(&fte_lock, flags);

	hash = flow->hash;

	if (flow->list.prev == &flow_ctl.flow_hash[hash]) {
		if (flow->list.next != &flow_ctl.flow_hash[hash])  {
			next_flow = list_entry(flow->list.next, flow_t, list);

			hwnat_hash_tbl_write(hash, next_flow->entry);
		} else {
			hwnat_hash_tbl_write(hash, 0);
		}
	} else {
		if (flow->list.next != &flow_ctl.flow_hash[hash])  {
			prev_flow = list_entry(flow->list.prev, flow_t, list);
			next_flow = list_entry(flow->list.next, flow_t, list);

			prev_flow->next = next_flow->entry;
			hwnat_flow_tbl_write(prev_flow->entry, prev_flow);
		} else {
			prev_flow = list_entry(flow->list.prev, flow_t, list);

			prev_flow->next = 0;
			hwnat_flow_tbl_write(prev_flow->entry, prev_flow);
		}
	}

	/* remove mac address table */
	if (flow->mode != FLOW_BRIDGE) {
		hwnat_mac_addr_remove(flow->flow_arg.mac_da);
		hwnat_mac_addr_remove(flow->flow_arg.mac_sa);
	}

	list_del(&flow->list);
	list_add_tail(&flow->list, &flow_ctl.flow_free);

	flow->hash = 0;
	flow->next = 0;
	flow->valid = 0;
	flow->vlan_tag = 0;
	flow->qid = 0;
	memset(&flow->flow_tuple, 0, sizeof(flow_tuple_t));
	memset(&flow->flow_arg, 0, sizeof(flow_arg_t));
	flow->tx_portmap = 0;
	flow->action = 0;
	flow->time = 0;
	flow->time_init = 0;
	flow->pkt_cnt = 0;
	hwnat_flow_tbl_write(flow->entry, flow);

	/* log netfilter conntrack */
	if (flow->ct) {
		clear_bit(IPS_PKTFLOWED_BIT, &flow->ct->status);  
	}

	flow->ct = NULL;
	flow->ct_dir = 0;

	flow->priority = FLOW_LOW_PRIORITY;

	flow_ctl.nr_flow_free++;

	spin_unlock_irqrestore(&fte_lock, flags);
}

flow_t *flow_find(uint8 *tuple)
{
	uint16 hash = flow_hash_fn(tuple, FLOW_TUPLE_LEN);
	flow_t *flow = NULL, *n;
	uint8 tuple_data[FLOW_TUPLE_LEN];

	list_for_each_entry_safe(flow, n, &flow_ctl.flow_hash[hash], list) {
		flow_make_tuple(&flow->flow_tuple, tuple_data);
		if (memcmp(tuple, tuple_data, FLOW_TUPLE_LEN) == 0)
			return flow;
	}

	return NULL;
}

int hwnat_flow_add(Pktflow_t *pf)
{
	flow_t  *flow;
	uint8	tuple[FLOW_TUPLE_LEN];
	flow_tuple_t flow_tuple;
	flow_arg_t flow_arg;
	uint8 	protocol = pf->protocol; 
	uint8 	priority = pf->tx.info.multicast ? FLOW_HIGH_PRIORITY : FLOW_LOW_PRIORITY; 
	uint16 	tx_portmap = (1<<pf->tx.info.itf); 
	uint16 	action = 0; 
	uint16	rx_pppoe = pf->rx.info.hdrs & (1<<PPPoE_2516);
	uint16	rx_ipv6 = pf->rx.info.hdrs & (1<<ETH_IPV6);
	uint8 flow_mode;
	struct nf_conn *ct;
	uint8 ct_dir;
	struct net_bridge_fdb_entry *fdb_src;
	static uint8 qid = 0;

	if (unlikely(flow_ctl.add_flow_on == 0))
		return 1;

	/* check if TX and RX protocol is the same */
	if ((pf->tx.info.hdrs & ((1<<ETH_IPV4) | (1<<ETH_IPV6))) !=
		(pf->tx.info.hdrs & ((1<<ETH_IPV4) | (1<<ETH_IPV6))))
		return 1;

	/*
	if ((protocol != IPPROTO_TCP) && (protocol != IPPROTO_UDP) && (protocol != IPPROTO_ICMP)) {
		printk("err: unsupport protocol\n");
		return 1;
	}
	*/

	if (rx_ipv6) {
		flow_tuple.tuple_index = FLOW_IPV6;
	} else {
		switch (protocol) {
			case IPPROTO_TCP:
				flow_tuple.tuple_index = FLOW_IPV4_TCP_UDP;
				break;
			case IPPROTO_UDP:
				flow_tuple.tuple_index = FLOW_IPV4_TCP_UDP;
				break;
			case IPPROTO_ICMP:
				flow_tuple.tuple_index = FLOW_IPV4_ICMP;
				break;
			default:
				flow_tuple.tuple_index = FLOW_IPV4_ONLY;
				break;
		}
	}
	if (rx_pppoe)
		flow_tuple.tuple_index++;
	if (rx_ipv6) {
		flow_tuple.ipv6_protocol = 1;
		flow_tuple.ipv6_tuple.protocol = protocol;
		memcpy(&flow_tuple.ipv6_tuple.daddr, &pf->rx.tuple.ip6_u.daddr, 16);
	} else {
		flow_tuple.ipv6_protocol = 0;
		flow_tuple.ipv4_tuple.protocol = protocol;
		flow_tuple.ipv4_tuple.saddr = pf->rx.tuple.ip4_u.saddr; 
		flow_tuple.ipv4_tuple.daddr = pf->rx.tuple.ip4_u.daddr;
		flow_tuple.ipv4_tuple.sport = pf->rx.tuple.ip4_u.port.source;
		flow_tuple.ipv4_tuple.dport = pf->rx.tuple.ip4_u.port.dest;
	}

	flow_make_tuple(&flow_tuple, tuple);

	if (flow_find(tuple)) {
		if (hwnat_debug) 	
			printk("err: duplicate flow\n");
		return 1;
	}

	/* debug */
	if (flow_ctl.dbg) {
		if (flow_ctl.dbg_rxportmap & (1<<pf->rx.info.itf)) {
			if (flow_ctl.dbg_protocol == 0)
				tx_portmap |= flow_ctl.dbg_txportmap;
			else if (flow_ctl.dbg_protocol == protocol)
				tx_portmap |= flow_ctl.dbg_txportmap;
		}
	}

	/* check if rx and tx mac address are the same? */
	if (!memcmp(&pf->rx.l2hdr[0], &pf->tx.l2hdr[0], ETH_ALEN) &&
		!memcmp(&pf->rx.l2hdr[6], &pf->tx.l2hdr[6], ETH_ALEN))
		flow_mode = FLOW_BRIDGE;
	else {
		if (rx_ipv6) {
			flow_mode = FLOW_PURE_ROUTING;
		} else {
			if ((pf->rx.tuple.ip4_u.saddr == pf->tx.tuple.ip4_u.saddr) &&
				(pf->rx.tuple.ip4_u.daddr == pf->tx.tuple.ip4_u.daddr))
				flow_mode = FLOW_PURE_ROUTING;
			else
				flow_mode = FLOW_NAT_ROUTING;
		}

		if ((protocol != IPPROTO_TCP) && (protocol != IPPROTO_UDP) && (protocol != IPPROTO_ICMP)) 
			return 1;

		/* don't support QinQ in routing mode */
		/*
		if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) 
			return 1;
		*/
	}

	/* clear flow arguments */
	memset(&flow_arg, 0x0, sizeof(flow_arg));

	/* get outter vlan tag */
	if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) 
		flow_arg.outer_vlan_tag = *(uint16 *) pktflow_l2_get(&pf->tx, DIR_TX, OUT_VLAN_8021Q, 2);

	/* get vlan tag */
	if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
		flow_arg.vlan_tag = *(uint16 *) pktflow_l2_get(&pf->tx, DIR_TX, VLAN_8021Q, 2);

	/* get pppoe tag */
	if (pf->tx.info.hdrs & (1<<PPPoE_2516)) 
		flow_arg.pppoe_sess_id = *(uint16 *) pktflow_l2_get(&pf->tx, DIR_TX, PPPoE_2516, 2);

	/* get tc2206 special tag */
	if (pf->tx.info.hdrs & (1<<TC2206_STAG)) 
		flow_arg.tc2206_tag = *(uint8 *) pktflow_l2_get(&pf->tx, DIR_TX, TC2206_STAG, 4);

	if (flow_mode == FLOW_BRIDGE) {
		if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
			if (pf->tx.info.hdrs & (1<<TC2206_STAG)) 
				action = 154;
			else 
				action = 153;
		} else {
			if (pf->tx.info.hdrs & (1<<VLAN_8021Q))
				action = 2;
			else
				action = 1;

			if (pf->tx.info.hdrs & (1<<TC2206_STAG)) 
				action += 2;
		}
	} else {
		flow_arg.mac_da = hwnat_mac_addr_insert(&pf->tx.l2hdr[0]);
		flow_arg.mac_sa = hwnat_mac_addr_insert(&pf->tx.l2hdr[6]);

		/* cannot allocate MAC DA or SA table */
		if ((flow_arg.mac_da < 0) || (flow_arg.mac_sa < 0)) {
			if (flow_arg.mac_da >= 0)
				hwnat_mac_addr_remove(flow_arg.mac_da);
			if (flow_arg.mac_sa >= 0)
				hwnat_mac_addr_remove(flow_arg.mac_sa);
			return 1;
		}

		/* Routing mode */
		if (flow_mode == FLOW_PURE_ROUTING) {
			if (pf->tx.info.hdrs & (1<<TC2206_STAG)) {
				if (pf->rx.info.hdrs & (1<<PPPoE_2516)) {
					if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
						if (rx_ipv6) 
							action = 227;
						else
							action = 217;
					} else {
						if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) {
							if (rx_ipv6) 
								action = 151;
							else
								action = 129;
						} else {
							if (rx_ipv6) 
								action = 149;
							else
								action = 131;
						}
					}
				} else { /* RX PPPoE_2516 */
					if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
						if (rx_ipv6) 
							action = 223;
						else
							action = 213;
					} else {
						if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) {
							if (rx_ipv6) 
								action = 143;
							else
								action = 123;
						} else {
							if (rx_ipv6) 
								action = 141;
							else
								action = 121;
						}
					}
				}
			} else { /* TC2206_STAG */
				if (pf->rx.info.hdrs & (1<<PPPoE_2516)) {
					if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
						if (rx_ipv6) 
							action = 225;
						else
							action = 215;
					} else {
						if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) {
							if (rx_ipv6) 
								action = 147;
							else
								action = 127;
						} else {
							if (rx_ipv6) 
								action = 145;
							else
								action = 125;
						}
					}
				} else { /* RX PPPoE_2516 */
					if (pf->tx.info.hdrs & (1<<PPPoE_2516)) {
						if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
							if (rx_ipv6) 
								action = 221;
							else
								action = 211;
						} else {
							if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) {
								if (rx_ipv6) 
									action = 139;
								else
									action = 119;
							} else {
								if (rx_ipv6) 
									action = 137;
								else
									action = 117;
							}
						}
					} else { /* TX PPPoE_2516 */
						if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
							if (rx_ipv6) 
								action = 219;
							else
								action = 209;
						} else {
							if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) {
								if (rx_ipv6) 
									action = 135;
								else
									action = 115;
							} else {
								if (rx_ipv6) 
									action = 133;
								else
									action = 113;
							}
						}
					}
				}
			}
		} else {
			/* IPv4 NAT Routing mode */
			/* LAN->WAN */
			if (pf->rx.tuple.ip4_u.daddr == pf->tx.tuple.ip4_u.daddr) {

				flow_arg.saddr = pf->tx.tuple.ip4_u.saddr;
				flow_arg.sport = pf->tx.tuple.ip4_u.port.source;

				if (pf->tx.info.hdrs & (1<<PPPoE_2516)) {
					/* PPPoE + TC2206 + QinQ + ... 
					   PPPoE + TC2206 + VLAN + ... 
					   PPPoE + TC2206 + ... 
					   PPPoE + QinQ + ... 
					   PPPoE + VLAN + ... 
					   PPPoE + ...  */
					switch (protocol) {
						case IPPROTO_TCP:
							if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
								action = 173;
							} else {
								if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
									action = 50;
								else
									action = 41;
							}
							break;
						case IPPROTO_UDP:
							if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
								action = 176;
							} else {
								if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
									action = 53;
								else
									action = 44;
							}
							break;
						case IPPROTO_ICMP:
							flow_arg.sport = pf->tx.tuple.ip4_u.port.dest;
							if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
								action = 179;
							} else {
								if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
									action = 56;
								else
									action = 47;
							}
							break;
					}
				} else { /* TX PPPoE_2516 */
					/* TC2206 + QinQ + ... 
					   TC2206 + VLAN + ... 
					   TC2206 + ... 
					   QinQ + ... 
					   VLAN + ... 
					   ...  */
					switch (protocol) {
						case IPPROTO_TCP:
							if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
								action = 155;
							} else {
								if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
									action = 23;
								else
									action = 5;
							}
							break;
						case IPPROTO_UDP:
							if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
								action = 158;
							} else {
								if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
									action = 26;
								else
									action = 8;
							}
							break;
						case IPPROTO_ICMP:
							flow_arg.sport = pf->tx.tuple.ip4_u.port.dest;
							if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
								action = 161;
							} else {
								if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
									action = 29;
								else
									action = 11;
							}
							break;
					}
				}
			} else {
			/* WAN->LAN */
				flow_arg.daddr = pf->tx.tuple.ip4_u.daddr;
				flow_arg.dport = pf->tx.tuple.ip4_u.port.dest;

				if (pf->rx.info.hdrs & (1<<PPPoE_2516)) {
					/* PPPoE Decap + TC2206 + QinQ + ... 
					   PPPoE Decap + TC2206 + VLAN + ... 
					   PPPoE Decap + TC2206 + ... 
					   PPPoE Decap + QinQ + ... 
					   PPPoE Decap + VLAN + ... 
					   PPPoE Decap + ...  */
					switch (protocol) {
						case IPPROTO_TCP:
							if (pf->tx.info.hdrs & (1<<TC2206_STAG)) {
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 200;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 104;
									else
										action = 95;
								}
							} else { /* TC2206_STAG */
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 191;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 86;
									else
										action = 77;
								}
							}
							break;
						case IPPROTO_UDP:
							if (pf->tx.info.hdrs & (1<<TC2206_STAG)) {
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 203;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 107;
									else
										action = 98;
								}
							} else { /* TC2206_STAG */
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 194;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 89;
									else
										action = 80;
								}
							}
							break;
						case IPPROTO_ICMP:
							if (pf->tx.info.hdrs & (1<<TC2206_STAG)) {
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 206;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 110;
									else
										action = 101;
								}
							} else { /* TC2206_STAG */
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 197;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 92;
									else
										action = 83;
								}
							}
							break;
					}
				} else { /* RX PPPoE_2516 */
					/* TC2206 + QinQ + ... 
					   TC2206 + VLAN + ... 
					   TC2206 + ... 
					   QinQ + ... 
					   VLAN + ... 
					   ...  */
					switch (protocol) {
						case IPPROTO_TCP:
							if (pf->tx.info.hdrs & (1<<TC2206_STAG)) {
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 182;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 68;
									else
										action = 59;
								}
							} else { /* TC2206_STAG */
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 164;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 32;
									else
										action = 14;
								}
							}
							break;
						case IPPROTO_UDP:
							if (pf->tx.info.hdrs & (1<<TC2206_STAG)) {
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 185;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 71;
									else
										action = 62;
								}
							} else { /* TC2206_STAG */
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 167;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 35;
									else
										action = 17;
								}
							}
							break;
						case IPPROTO_ICMP:
							if (pf->tx.info.hdrs & (1<<TC2206_STAG)) {
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 188;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 74;
									else
										action = 65;
								}
							} else { /* TC2206_STAG */
								if (pf->tx.info.hdrs & (1<<OUT_VLAN_8021Q)) {
									action = 170;
								} else {
									if (pf->tx.info.hdrs & (1<<VLAN_8021Q)) 
										action = 38;
									else
										action = 20;
								}
							}
							break;
					}
				}
			}
		}
	}
	ct = pf->rx.ct_p;
	ct_dir = pf->rx.nf_dir;

	fdb_src = pf->fdb_src;

	flow = flow_alloc(priority);
	if (flow == NULL) {
		if (hwnat_debug) 	
			printk("err: no free flow\n");
		return 1;
	}

	if ((flow_ctl.qid >= HWNAT_MIN_QID) && (flow_ctl.qid <= HWNAT_MAX_QID))
		flow_init(flow_mode, flow, &flow_tuple, &flow_arg, tx_portmap, action, ct, ct_dir, flow_ctl.qid, priority, fdb_src);
	else {
		/* test mode */
		flow_init(flow_mode, flow, &flow_tuple, &flow_arg, tx_portmap, action, ct, ct_dir, qid, priority, fdb_src);

		qid++;
		qid = qid & 0x3;
	}
	flow_insert(flow);

	return 0;
}

void hwnat_flow_remove(uint16 entry)
{
	flow_remove(entry);
}

void hwnat_flow_remove_by_fdb(struct net_bridge_fdb_entry *fdb)
{
	flow_t *flow;
	flow_t flow_entry;
	uint16 entry;

	for (entry = 0; entry < FLOW_TBL_SIZE; entry++) {
		flow = &flow_tbl[entry];

		if (flow->valid == 0)
			continue;

#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE)
		if (flow->fdb_src == fdb) {
			flow_remove(entry);
		}
#endif
	}
}

void hwnat_flow_remove_by_oldest(void)
{
	flow_t *flow;
	flow_t flow_entry;
	uint16 entry;
	uint16 remove_entry = 0;
	uint8 remove_time = flow_ctl.timeout;

	for (entry = 0; entry < FLOW_TBL_SIZE; entry++) {
		flow = &flow_tbl[entry];

		/* found one free flow */
		if (flow->valid == 0)
			return;

		/* found one high priority flow */
		if (flow->priority)
			continue;

		/* get hw table hold registers */
		hwnat_flow_tbl_read(entry, &flow_entry);
		if (flow_entry.time < remove_time) {
			remove_entry = entry;
			remove_time = flow_entry.time;
		}
	}

	if (remove_entry != 0)
		flow_remove(remove_entry);
}

void hwnat_flow_update_ipv4_mc(__be32 group, unsigned int portmap)
{
	flow_t *flow;
	flow_t flow_entry;
	uint16 entry;

	for (entry = 0; entry < FLOW_TBL_SIZE; entry++) {
		flow = &flow_tbl[entry];

		if ((flow->valid == 0) || (flow->flow_tuple.ipv6_protocol)) 
			continue;

		/* check if mutlicast group is the same as hwnat flow */
		if (flow->flow_tuple.ipv4_tuple.daddr == group) {
			if (portmap == 0) 
				flow_remove(entry);
			else {
				flow->flow_arg.tc2206_tag = portmap;
				hwnat_flow_tbl_write(flow->entry, flow);
			}
		}
	}
}

void hwnat_flow_update_ip6_mc(struct in6_addr group, unsigned int portmap)
{
	flow_t *flow;
	flow_t flow_entry;
	uint16 entry;

	for (entry = 0; entry < FLOW_TBL_SIZE; entry++) {
		flow = &flow_tbl[entry];

		if ((flow->valid == 0) || (!flow->flow_tuple.ipv6_protocol)) 
			continue;

		/* check if mutlicast group is the same as hwnat flow */
		if (memcmp(&flow->flow_tuple.ipv6_tuple.daddr, &group, 16) == 0) {
			if (portmap == 0) 
				flow_remove(entry);
			else {
				flow->flow_arg.tc2206_tag = portmap;
				hwnat_flow_tbl_write(flow->entry, flow);
			}
		}
	}
}

static void hwnat_fte_scan(unsigned long data)
{
	flow_t *flow;
	flow_t flow_entry;
	uint16 entry;

	for (entry = 0; entry < FLOW_TBL_SIZE; entry++) {
		flow = &flow_tbl[entry];

		if (flow->valid == 0)
			continue;

		/* get hw table hold registers */
		hwnat_flow_tbl_read(entry, &flow_entry);

		if (flow->pkt_cnt != flow_entry.pkt_cnt) {
			flow->pkt_cnt = flow_entry.pkt_cnt;

			/* refresh nfct */
			if (flow->ct) {
				pktflow_refresh_nfct(flow->ct, flow_ctl.timeout*HZ);
			}

			/* refresh bridge fdb */
#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE)
			if (flow->mode == FLOW_BRIDGE) {
				struct net_bridge_fdb_entry *fdb_src = flow->fdb_src;

				if (fdb_src != NULL) 
					fdb_src->ageing_timer = jiffies;
			}
#endif
		}
	}

#ifndef HWNAT_SW_MODEL
{
	irqreturn_t hwnat_fte_isr(int irq, void *dev_id);

	hwnat_fte_isr(0, NULL);
}
#endif

	mod_timer(&flow_ctl.timer, jiffies+1*HZ);
}

#ifdef HWNAT_SW_MODEL
static void hwnat_fte_expire(unsigned long data)
{
	extern void hwnat_timer_process(void);

	hwnat_timer_process();
	mod_timer(&fte_timer, jiffies+1*HZ);
}
#endif

irqreturn_t hwnat_fte_isr(int irq, void *dev_id)
{
	uint32 as[AGE_STATUS_SIZE];
	int index, entry, bit;

#ifdef HWNAT_SW_MODEL
	/* de-assert interrupt */
	VPint(CR_INTC_ISR) = 0;
#else
	hwnat_write_reg(CR_HWNAT_ISR, hwnat_read_reg(CR_HWNAT_ISR) | ISR_INT);
	hwnat_write_reg(CR_HWNAT_ISR, hwnat_read_reg(CR_HWNAT_ISR) & ~ISR_INT);

	/* load aging status register */
	VPint(CR_HWNAT_CTL0) &= ~CTL0_LOAD_AGE;
	VPint(CR_HWNAT_CTL0) |= CTL0_LOAD_AGE;
#endif

	for (index = 0; index < AGE_STATUS_SIZE; index++) {
		as[index] = hwnat_read_reg(CR_HWNAT_AS(index));
		//if (as[index] != 0x0)
		//	printk("as[%d]=%08x\n", index, as[index]);
		//printk("%08x as[%d]=%08lx\n", CR_HWNAT_AS(index), index, as[index]);
	}

	for (index = AGE_STATUS_SIZE - 1; index >= 0; index--) {
		//printk("as[%d]=%08x\n", index, as[index]);
		/* no flow age out */
		if (as[index] == 0)
			continue;

		for (bit = 0; bit < 32; bit++) {
			if (as[index] & (1<<bit)) {
				entry = 32 * (7 - index) + bit;
				flow_remove(entry);
			}
		}
	}

	return IRQ_HANDLED;
}

void hwnat_fte_init(void)
{
	INIT_LIST_HEAD(&flow_ctl.flow_free);
	flow_ctl.nr_flow_free = 0;

	/* add flow */
	flow_ctl.add_flow_on = 1;

	/* debug */
	flow_ctl.dbg = 0;
	flow_ctl.dbg_protocol = 0;
	flow_ctl.dbg_rxportmap = 0;
	flow_ctl.dbg_txportmap = 0;

	hwnat_hash_init();
	hwnat_flow_init();

	init_timer(&flow_ctl.timer);
	flow_ctl.timer.data     = (unsigned long) NULL;
	flow_ctl.timer.function = hwnat_fte_scan;

	mod_timer(&flow_ctl.timer, jiffies+1*HZ);
	flow_ctl.timeout = FLOW_DEF_TIME;
	flow_ctl.qid = 0;

#ifdef HWNAT_SW_MODEL
	init_timer(&fte_timer);
	fte_timer.data     = (unsigned long) NULL;
	fte_timer.function = hwnat_fte_expire;

	mod_timer(&fte_timer, jiffies+1*HZ);
#endif

#ifdef HWNAT_SW_MODEL
	request_irq(HWNAT_INT, hwnat_fte_isr, 0, "HWNAT", NULL);
#endif
}

void hwnat_fte_exit(void)
{
	del_timer_sync(&flow_ctl.timer);

#ifdef HWNAT_SW_MODEL
	del_timer_sync(&fte_timer);

	free_irq(HWNAT_INT, NULL);
#endif
}

void hwnat_hash_init(void)
{
	uint16 entry;

	for (entry = 0; entry < HASH_TBL_SIZE; entry++) {
		hwnat_hash_tbl_write(entry, 0);

		INIT_LIST_HEAD(&flow_ctl.flow_hash[entry]);
	}
}

void hwnat_hash_tbl_write(uint16 entry, uint16 head)
{
	uint32 table_hold[HTH_SIZE];

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[0] = (head & 0xff);
	hwnat_write_tbl(HASH_TBL_ID, entry, table_hold, HTH_SIZE);
}

void hwnat_hash_tbl_read(uint16 entry, uint16 *head)
{
	uint32 table_hold[HTH_SIZE];

	hwnat_read_tbl(HASH_TBL_ID, entry, table_hold, HTH_SIZE);
	*head = table_hold[0] & 0xff;
}

void hwnat_flow_init(void)
{
	flow_t *flow;
	uint16 entry;

	memset(flow_tbl, 0, sizeof(flow_tbl));
	for (entry = 0; entry < FLOW_TBL_SIZE; entry++) {
		flow = &flow_tbl[entry];

		flow->entry = entry;
		hwnat_flow_tbl_write(entry, flow);
		
		/* entry 0 is reserved for invalid entry indication */
		if (entry != 0) {
			flow_ctl.nr_flow_free++;
			INIT_LIST_HEAD(&flow->list);
			list_add_tail(&flow->list, &flow_ctl.flow_free);
		}
	}

	/* init aging timer */
	/* set base unit = 1 second */
	hwnat_write_reg(CR_HWNAT_AFS, (HWNAT_CLK*1000000)&AFS_CLKDIV);
	/* set aging time=1, 1 max int time, 1 max int no */
	hwnat_write_reg(CR_HWNAT_AFC, ((1<<AFC_INTTIME_SHIFT)&AFC_INTTIME) |
		((1<<AFC_INTNO_SHIFT)&AFC_INTNO) |
		((1<<AFC_AGETIME_SHIFT)&AFC_AGETIME));
}

void hwnat_flow_tbl_write(uint16 entry, flow_t *flow)
{
	uint32 table_hold[FTH_SIZE];
	uint32 tuple[FLOW_TUPLE_LEN>>2];
	uint32 arg[FLOW_ARG_LEN>>2];

	flow_make_tuple(&flow->flow_tuple, (uint8 *) tuple);
	flow_make_arg(&flow->flow_arg, (uint8 *) arg);

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[11] = flow->pkt_cnt;
	table_hold[10] = (flow->time_init&0xff) | ((flow->time<<8)&0xff00) | 
			((flow->action<<16)&0xff0000) | ((flow->tx_portmap<<24)&0xff000000);
	table_hold[9] = ((flow->tx_portmap>>8)&0xff) | ((arg[3]<<8)&0xffffff00);
	table_hold[8] = ((arg[3]>>24)&0xff) | ((arg[2]<<8)&0xffffff00);
	table_hold[7] = ((arg[2]>>24)&0xff) | ((arg[1]<<8)&0xffffff00);
	table_hold[6] = ((arg[1]>>24)&0xff) | ((arg[0]<<8)&0xffffff00);
	table_hold[5] = ((arg[0]>>24)&0xff) | ((tuple[4]<<8)&0xffffff00);
	table_hold[4] = ((tuple[4]>>24)&0xff) | ((tuple[3]<<8)&0xffffff00);
	table_hold[3] = ((tuple[3]>>24)&0xff) | ((tuple[2]<<8)&0xffffff00);
	table_hold[2] = ((tuple[2]>>24)&0xff) | ((tuple[1]<<8)&0xffffff00);
	table_hold[1] = ((tuple[1]>>24)&0xff) | ((tuple[0]<<8)&0xffffff00);
	table_hold[0] = ((tuple[0]>>24)&0xff) | ((flow->qid<<8)&0x700) | ((flow->vlan_tag<<11)&0x7f800) |
		((flow->valid<<19)&0x80000) | ((flow->next<<20)&0xff00000);

	hwnat_write_tbl(FLOW_TBL_ID, entry, table_hold, FTH_SIZE);
}

void hwnat_flow_tbl_read(uint16 entry, flow_t *flow)
{
	uint32 table_hold[FTH_SIZE];

	hwnat_read_tbl(FLOW_TBL_ID, entry, table_hold, FTH_SIZE);

	flow->pkt_cnt = table_hold[11];
	flow->time_init = table_hold[10]&0xff;
	flow->time = (table_hold[10]&0xff00)>>8;
	flow->action = (table_hold[10]&0xff0000)>>16;
	flow->tx_portmap = ((table_hold[10]&0xff000000)>>24) | ((table_hold[9]&0xff)<<8);
	flow->qid = (table_hold[0]&0x700)>>8;
	flow->valid = (table_hold[0]&0x80000)>>19;
	flow->next = (table_hold[0]&0xff00000)>>20;
}

void hwnat_flow_tbl_set(uint16 entry, uint16 tx_portmap, uint16 action, uint8 qid)
{
	uint32 table_hold[FTH_SIZE];
	flow_t *flow;

	flow = &flow_tbl[entry];

	flow->action = action;
	flow->tx_portmap = tx_portmap;
	flow->qid = qid;

	hwnat_read_tbl(FLOW_TBL_ID, entry, table_hold, FTH_SIZE);

	table_hold[10] = (table_hold[10]&0x0000ffff) |
			((action<<16)&0xff0000) | ((tx_portmap<<24)&0xff000000);
	table_hold[9] = (table_hold[9]&0xffffff00) | ((tx_portmap>>8)&0xff);
	table_hold[0] = (table_hold[0]&0xfffff8ff) | ((qid<<8)&0x700);

	hwnat_write_tbl(FLOW_TBL_ID, entry, table_hold, FTH_SIZE);
}

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/

static int do_hwnat_hash_tbl(int argc, char *argv[], void *p);
static int do_hwnat_hash_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_hash_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_hash_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_hash_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_hash_cmds[] = {
	{"tbl",			do_hwnat_hash_tbl,			0x12,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_hash_tbl_cmds[] = {
	{"dump",		do_hwnat_hash_tbl_dump,		0x02,  	0,  NULL},
	{"raw",			do_hwnat_hash_tbl_raw,		0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_hash_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_hash_tbl_comp,		0x02,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

int do_hwnat_hash(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_hash_cmds, argc, argv, p);
}

int do_hwnat_hash_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_hash_tbl_cmds, argc, argv, p);
}

int do_hwnat_hash_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;
	flow_t *flow;
	uint16 head;

	printk("head\n");
	for (entry = 0; entry < HASH_TBL_SIZE; entry++) {
		if (list_empty(&flow_ctl.flow_hash[entry])) {
			head = 0;
		} else {
			flow = list_entry(flow_ctl.flow_hash[entry].next, flow_t, list);
			head = flow->entry;
		}
		printk("%03d %d\n", entry, head);
	}
	return 0;
}

int do_hwnat_hash_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[HTH_SIZE];
	flow_t *flow;
	uint16 entry, i;
	uint16 head;

	printk("hash\n");
	for (entry = 0; entry < HASH_TBL_SIZE; entry++) {
		if (list_empty(&flow_ctl.flow_hash[entry])) {
			head = 0;
		} else {
			flow = list_entry(flow_ctl.flow_hash[entry].next, flow_t, list);
			head = flow->entry;
		}

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[0] = (head & 0xff);

		printk("/*%03d*/", entry);
		for (i = 0; i < HTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_hash_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[HTH_SIZE];
	uint16 entry, i;

	printk("hash\n");
	for (entry = 0; entry < HASH_TBL_SIZE; entry++) {
		hwnat_read_tbl(HASH_TBL_ID, entry, table_hold, HTH_SIZE);
		printk("/*%03d*/", entry);
		for (i = 0; i < HTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_hash_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[HTH_SIZE];
	uint32 table_hold[HTH_SIZE];
	flow_t *flow;
	uint16 entry, i;
	uint16 head;

	for (entry = 0; entry < HASH_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(HASH_TBL_ID, entry, hw_table_hold, HTH_SIZE);

		if (list_empty(&flow_ctl.flow_hash[entry])) {
			head = 0;
		} else {
			flow = list_entry(flow_ctl.flow_hash[entry].next, flow_t, list);
			head = flow->entry;
		}

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[0] = (head & 0xff);

		if (memcmp(hw_table_hold, table_hold, sizeof(hw_table_hold)) != 0) {
			printk("error /*%03d*/\n", entry);
			for (i = 0; i < HTH_SIZE; i++)
				printk(" %08lx", hw_table_hold[i]);
			printk("\n");
			for (i = 0; i < HTH_SIZE; i++)
				printk(" %08lx", table_hold[i]);
			printk("\n");
		}
	}
	return 0;
}


static int do_hwnat_flow_tbl(int argc, char *argv[], void *p);
static int do_hwnat_flow_tbl_ff(int argc, char *argv[], void *p);
static int do_hwnat_flow_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_flow_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_flow_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_flow_tbl_comp(int argc, char *argv[], void *p);
static int do_hwnat_flow_tbl_set(int argc, char *argv[], void *p);

static int do_hwnat_flow_add(int argc, char *argv[], void *p);
static int do_hwnat_flow_remove(int argc, char *argv[], void *p);
static int do_hwnat_flow_flush(int argc, char *argv[], void *p);
static int do_hwnat_flow_timeout(int argc, char *argv[], void *p);
static int do_hwnat_flow_add_on(int argc, char *argv[], void *p);
static int do_hwnat_flow_qid(int argc, char *argv[], void *p);
static int do_hwnat_flow_debug(int argc, char *argv[], void *p);
static int do_hwnat_flow_transbrg(int argc, char *argv[], void *p);
static int do_hwnat_flow_update4mc(int argc, char *argv[], void *p);
static int do_hwnat_flow_update6mc(int argc, char *argv[], void *p);

static int do_hwnat_flow_add(int argc, char *argv[], void *p);
static int do_hwnat_flow_add_bridge(int argc, char *argv[], void *p);
static int do_hwnat_flow_add_route(int argc, char *argv[], void *p);
static int do_hwnat_flow_add_bridge6(int argc, char *argv[], void *p);
static int do_hwnat_flow_add_route6(int argc, char *argv[], void *p);

static const cmds_t hwnat_flow_cmds[] = {
	{"tbl",			do_hwnat_flow_tbl,			0x12,  	0,  NULL},
	{"add",			do_hwnat_flow_add,			0x12,  	0,  NULL},
	{"remove",		do_hwnat_flow_remove,		0x02,  	1,  NULL},
	{"flush",		do_hwnat_flow_flush,		0x02,  	0,  NULL},
	{"timeout",		do_hwnat_flow_timeout,		0x02,  	1,  "<timeout>"},
	{"add_on",		do_hwnat_flow_add_on,		0x02,  	0,  "<1|0>"},
	{"qid",			do_hwnat_flow_qid,			0x02,  	1,  "<qid>"},
	{"debug",		do_hwnat_flow_debug,		0x02,  	4,  "<dbg> <protocol> <rxportmap> <txportmap>"},
	{"update4mc",	do_hwnat_flow_update4mc,	0x02,  	2,  "<dip> <tx_portmap>"},
	{"update6mc",	do_hwnat_flow_update6mc,	0x02,  	2,  "<dip> <tx_portmap>"},
	//{"transbrg",	do_hwnat_flow_transbrg,		0x02,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_flow_tbl_cmds[] = {
	{"ff",			do_hwnat_flow_tbl_ff,		0x02,  	0,  NULL},
	{"dump",		do_hwnat_flow_tbl_dump,		0x02,  	0,  NULL},
	{"raw",			do_hwnat_flow_tbl_raw,		0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_flow_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_flow_tbl_comp,		0x02,  	0,  NULL},
	{"set",			do_hwnat_flow_tbl_set,		0x02,  	1,  "<entry> <tx_portmap> <action> <qid>"},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_flow_add_cmds[] = {
	{"bridge",		do_hwnat_flow_add_bridge,	0x02,  	12,  "<tuple> <id> <sip> <dip> <sport> <dport> <vlan> <outer_vlan> <tc2206_tag> <tx_portmap> <action> <qid>"},
	{"route",		do_hwnat_flow_add_route,	0x02,  	17,  "<tuple> <id> <sip> <dip> <sport> <dport> <mac_da> <mac_sa> <new_addr> <new_port> <vlan> <pppoe_sess_id> <outer_vlan> <tc2206_tag> <tx_portmap> <action> <qid>"},
	{"bridge6",		do_hwnat_flow_add_bridge6,	0x02,  	8,  "<tuple> <dip> <vlan> <outer_vlan> <tc2206_tag> <tx_portmap> <action> <qid>"},
	{"route6",		do_hwnat_flow_add_route6,	0x02,  	11,  "<tuple> <dip> <mac_da> <mac_sa> <vlan> <pppoe_sess_id> <outer_vlan> <tc2206_tag> <tx_portmap> <action> <qid>"},
	{NULL,			NULL,						0x10,	0,	NULL},
};


int do_hwnat_flow(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_flow_cmds, argc, argv, p);
}

int do_hwnat_flow_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_flow_tbl_cmds, argc, argv, p);
}

int do_hwnat_flow_tbl_ff(int argc, char *argv[], void *p)
{
	printk("flow ff=%d\n", flow_ctl.nr_flow_free);
	return 0;
}

int do_hwnat_flow_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;
	flow_t *flow, *n;

	printk("flow ff=%d\n", flow_ctl.nr_flow_free);
	for (entry = 0; entry < HASH_TBL_SIZE; entry++) {
		if (!list_empty(&flow_ctl.flow_hash[entry])) {
			printk("[%03d]\n", entry);
			list_for_each_entry_safe(flow, n, &flow_ctl.flow_hash[entry], list) {
				printk("%d: <%d ", flow->entry, flow->flow_tuple.tuple_index);
				if (flow->flow_tuple.ipv6_protocol) {
					printk("%u " NIP6_FMT ">",
						flow->flow_tuple.ipv6_tuple.protocol,
						NIP6(flow->flow_tuple.ipv6_tuple.daddr));
				} else {
					printk("%u " NIPQUAD_FMT ":%u " NIPQUAD_FMT ":%u>",
						flow->flow_tuple.ipv4_tuple.protocol,
						NIPQUAD(flow->flow_tuple.ipv4_tuple.saddr), flow->flow_tuple.ipv4_tuple.sport,
						NIPQUAD(flow->flow_tuple.ipv4_tuple.daddr), flow->flow_tuple.ipv4_tuple.dport);
				}
				printk("<MAC=%d %d>",
						flow->flow_arg.mac_da,
						flow->flow_arg.mac_sa);
				if (!flow->flow_tuple.ipv6_protocol) {
					printk("<" NIPQUAD_FMT ":%u>",
						NIPQUAD(flow->flow_arg.saddr), flow->flow_arg.sport);
				}
				printk("<%04x %04x %04x %d>",
						flow->flow_arg.vlan_tag,
						flow->flow_arg.pppoe_sess_id,
						flow->flow_arg.outer_vlan_tag,
						flow->flow_arg.tc2206_tag);
				printk("<%04x %d q=%d> pkt_cnt=%08lx\n",
						flow->tx_portmap, flow->action, flow->qid, flow->pkt_cnt);
			}
		}
	}
	return 0;
}

int do_hwnat_flow_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[FTH_SIZE];
	flow_t *flow;
	uint16 entry, i;

	printk("flow\n");
	for (entry = 0; entry < FLOW_TBL_SIZE; entry++) {
		flow = &flow_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		/*
		table_hold[11] = flow->pkt_cnt;
		table_hold[10] = (flow->time_init&0xff) | ((flow->time<<8)&0xff00) | 
				((flow->action<<16)&0xff0000) | ((flow->tx_portmap<<24)&0xff000000);
		table_hold[9] = ((flow->tx_portmap>>8)&0xff) | ((flow->arg[3]<<8)&0xffffff00);
		table_hold[8] = ((flow->arg[3]>>24)&0xff) | ((flow->arg[2]<<8)&0xffffff00);
		table_hold[7] = ((flow->arg[2]>>24)&0xff) | ((flow->arg[1]<<8)&0xffffff00);
		table_hold[6] = ((flow->arg[1]>>24)&0xff) | ((flow->arg[0]<<8)&0xffffff00);
		table_hold[5] = ((flow->arg[0]>>24)&0xff) | ((flow->tuple[4]<<8)&0xffffff00);
		table_hold[4] = ((flow->tuple[4]>>24)&0xff) | ((flow->tuple[3]<<8)&0xffffff00);
		table_hold[3] = ((flow->tuple[3]>>24)&0xff) | ((flow->tuple[2]<<8)&0xffffff00);
		table_hold[2] = ((flow->tuple[2]>>24)&0xff) | ((flow->tuple[1]<<8)&0xffffff00);
		table_hold[1] = ((flow->tuple[1]>>24)&0xff) | ((flow->tuple[0]<<8)&0xffffff00);
		table_hold[0] = ((flow->tuple[0]>>24)&0xff) | ((flow->valid<<8)&0x100) | 
				((flow->next<<9)&0x1fe00);
		*/

		printk("/*%03d*/", entry);
		for (i = 0; i < FTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_flow_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[FTH_SIZE];
	uint16 entry, i;
	uint8 dump_one = 0;

	if (argc >= 2) {
		entry = simple_strtoul(argv[1], NULL, 10);
		dump_one = 1;
		goto hwraw_dump;
	}

	printk("flow\n");
	for (entry = 0; entry < FLOW_TBL_SIZE; entry++) {
hwraw_dump:
		hwnat_read_tbl(FLOW_TBL_ID, entry, table_hold, FTH_SIZE);
		printk("/*%03d*/", entry);
		for (i = 0; i < FTH_SIZE; i++) {
			if (i == 7)
				printk("\n       ");
			printk(" %08lx", table_hold[i]);
		}
		printk("\n");

		if (dump_one)
			break;
	}
	return 0;
}

int do_hwnat_flow_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[FTH_SIZE];
	uint32 table_hold[FTH_SIZE];
	flow_t *flow;
	uint16 entry, i;

	for (entry = 0; entry < FLOW_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(FLOW_TBL_ID, entry, hw_table_hold, FTH_SIZE);

		flow = &flow_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		/*
		table_hold[11] = flow->pkt_cnt;
		table_hold[10] = (flow->time_init&0xff) | ((flow->time<<8)&0xff00) | 
				((flow->action<<16)&0xff0000) | ((flow->tx_portmap<<24)&0xff000000);
		table_hold[9] = ((flow->tx_portmap>>8)&0xff) | ((flow->arg[3]<<8)&0xffffff00);
		table_hold[8] = ((flow->arg[3]>>24)&0xff) | ((flow->arg[2]<<8)&0xffffff00);
		table_hold[7] = ((flow->arg[2]>>24)&0xff) | ((flow->arg[1]<<8)&0xffffff00);
		table_hold[6] = ((flow->arg[1]>>24)&0xff) | ((flow->arg[0]<<8)&0xffffff00);
		table_hold[5] = ((flow->arg[0]>>24)&0xff) | ((flow->tuple[4]<<8)&0xffffff00);
		table_hold[4] = ((flow->tuple[4]>>24)&0xff) | ((flow->tuple[3]<<8)&0xffffff00);
		table_hold[3] = ((flow->tuple[3]>>24)&0xff) | ((flow->tuple[2]<<8)&0xffffff00);
		table_hold[2] = ((flow->tuple[2]>>24)&0xff) | ((flow->tuple[1]<<8)&0xffffff00);
		table_hold[1] = ((flow->tuple[1]>>24)&0xff) | ((flow->tuple[0]<<8)&0xffffff00);
		table_hold[0] = ((flow->tuple[0]>>24)&0xff) | ((flow->valid<<8)&0x100) | 
				((flow->next<<9)&0x1fe00);
		*/

		if (memcmp(hw_table_hold, table_hold, sizeof(hw_table_hold)) != 0) {
			printk("error /*%03d*/\n", entry);
			for (i = 0; i < FTH_SIZE; i++)
				printk(" %08lx", hw_table_hold[i]);
			printk("\n");
			for (i = 0; i < FTH_SIZE; i++)
				printk(" %08lx", table_hold[i]);
			printk("\n");
		}
	}
	return 0;
}

int do_hwnat_flow_tbl_set(int argc, char *argv[], void *p)
{
	uint16 entry;
	flow_t flow_entry;
	uint16 tx_portmap; 
	uint16 action; 
	uint8 qid;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	if (argc >= 5) {
		tx_portmap = (uint16) simple_strtoul(argv[2], NULL, 16);
		action = (uint16) simple_strtoul(argv[3], NULL, 10);
		qid = (uint8) simple_strtoul(argv[4], NULL, 10);

		hwnat_flow_tbl_set(entry, tx_portmap, action, qid);
	} else {
		hwnat_flow_tbl_read(entry, &flow_entry);

		printk("[%03d]\n", entry);
		printk("tx_portmap=%04x\n", flow_entry.tx_portmap);
		printk("action    =%d\n", flow_entry.action);
		printk("qid       =%d\n", flow_entry.qid);
	}

	return 0;
}

int do_hwnat_flow_add(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_flow_add_cmds, argc, argv, p);
}

int do_hwnat_flow_add_bridge(int argc, char *argv[], void *p)
{
	flow_t  *flow;
	uint8	tuple[FLOW_TUPLE_LEN];
	flow_tuple_t flow_tuple;
	flow_arg_t flow_arg;
    uint16	tx_portmap;       
    uint16	action;       
	uint8	qid;

	flow_tuple.tuple_index = (uint8) simple_strtoul(argv[1], NULL, 10);
	flow_tuple.ipv6_protocol = 0;
	flow_tuple.ipv4_tuple.protocol = (uint8) simple_strtoul(argv[2], NULL, 10);
	flow_tuple.ipv4_tuple.saddr = in_aton(argv[3]);
	flow_tuple.ipv4_tuple.daddr = in_aton(argv[4]);
	flow_tuple.ipv4_tuple.sport = (uint16) simple_strtoul(argv[5], NULL, 10);
	flow_tuple.ipv4_tuple.dport = (uint16) simple_strtoul(argv[6], NULL, 10);

	tx_portmap = (uint16) simple_strtoul(argv[10], NULL, 16);
	action = (uint16) simple_strtoul(argv[11], NULL, 10);
	qid = (uint8) simple_strtoul(argv[12], NULL, 10);

	flow_make_tuple(&flow_tuple, tuple);

	if (flow_find(tuple)) {
		printk("duplicate flow\n");
		return 0;
	}

	memset(&flow_arg, 0x0, sizeof(flow_arg));

	flow_arg.vlan_tag = (uint16) simple_strtoul(argv[7], NULL, 16);
	flow_arg.outer_vlan_tag = (uint16) simple_strtoul(argv[8], NULL, 16);
	flow_arg.tc2206_tag = (uint8) simple_strtoul(argv[9], NULL, 10);

	printk("TUPLE\n");
	printk("----------------------------------------------------\n");
	printk("tuple     =%d\n", flow_tuple.tuple_index);
	printk("protocol  =%d\n", flow_tuple.ipv4_tuple.protocol);
	printk("saddr     =" NIPQUAD_FMT "\n", NIPQUAD(flow_tuple.ipv4_tuple.saddr));
	printk("daddr     =" NIPQUAD_FMT "\n", NIPQUAD(flow_tuple.ipv4_tuple.daddr));
	printk("sport     =%d\n", flow_tuple.ipv4_tuple.sport);
	printk("dport     =%d\n", flow_tuple.ipv4_tuple.dport);
	printk("----------------------------------------------------\n");
	printk("vlan tag  =%04x\n", flow_arg.vlan_tag);
	printk("outer vlan=%04x\n", flow_arg.outer_vlan_tag);
	printk("tc2206tag =%d\n", flow_arg.tc2206_tag);
	printk("----------------------------------------------------\n");
	printk("tx_portmap=%04x\n", tx_portmap);
	printk("action    =%d\n", action);
	printk("qid       =%d\n", qid);

	flow = flow_alloc(FLOW_LOW_PRIORITY);
	if (flow == NULL) {
		printk("no free flow\n");
		return 0;
	}

	flow_init(FLOW_BRIDGE, flow, &flow_tuple, &flow_arg, tx_portmap, action, NULL, 0, qid, FLOW_LOW_PRIORITY, NULL);
	flow_insert(flow);
	printk("INSERT flow entry=%d hash=%d\n", flow->entry, flow->hash);

	return 0;
}

int do_hwnat_flow_add_route(int argc, char *argv[], void *p)
{
	flow_t  *flow;
	uint8	tuple[FLOW_TUPLE_LEN];
	flow_tuple_t flow_tuple;
	flow_arg_t flow_arg;
    uint16	tx_portmap;       
    uint16	action;       
    uint8	qid;       
	uint8 macaddr[6];

	flow_tuple.tuple_index = (uint8) simple_strtoul(argv[1], NULL, 10);
	flow_tuple.ipv6_protocol = 0;
	flow_tuple.ipv4_tuple.protocol = (uint8) simple_strtoul(argv[2], NULL, 10);
	flow_tuple.ipv4_tuple.saddr = in_aton(argv[3]);
	flow_tuple.ipv4_tuple.daddr = in_aton(argv[4]);
	flow_tuple.ipv4_tuple.sport = (uint16) simple_strtoul(argv[5], NULL, 10);
	flow_tuple.ipv4_tuple.dport = (uint16) simple_strtoul(argv[6], NULL, 10);

	tx_portmap = (uint16) simple_strtoul(argv[15], NULL, 16);
	action = (uint16) simple_strtoul(argv[16], NULL, 10);
	qid = (uint8) simple_strtoul(argv[17], NULL, 10);

	flow_make_tuple(&flow_tuple, tuple);

	if (flow_find(tuple)) {
		printk("duplicate flow\n");
		return 0;
	}

	if (hwnat_mac_addr_in_ether(argv[7], macaddr)) {
		printk("err: mac addr format err\n");
		return 0;
	}

	flow_arg.mac_da = hwnat_mac_addr_insert(macaddr);

	if (hwnat_mac_addr_in_ether(argv[8], macaddr)) {
		printk("err: mac addr format err\n");
		return 0;
	}

	flow_arg.mac_sa = hwnat_mac_addr_insert(macaddr);
	flow_arg.saddr = in_aton(argv[9]);
	flow_arg.sport = (uint16) simple_strtoul(argv[10], NULL, 10);
	flow_arg.vlan_tag = (uint16) simple_strtoul(argv[11], NULL, 16);
	flow_arg.pppoe_sess_id = (uint16) simple_strtoul(argv[12], NULL, 16);
	flow_arg.outer_vlan_tag = (uint16) simple_strtoul(argv[13], NULL, 16);
	flow_arg.tc2206_tag = (uint8) simple_strtoul(argv[14], NULL, 10);

	printk("TUPLE\n");
	printk("----------------------------------------------------\n");
	printk("tuple     =%d\n", flow_tuple.tuple_index);
	printk("protocol  =%d\n", flow_tuple.ipv4_tuple.protocol);
	printk("saddr     =" NIPQUAD_FMT "\n", NIPQUAD(flow_tuple.ipv4_tuple.saddr));
	printk("daddr     =" NIPQUAD_FMT "\n", NIPQUAD(flow_tuple.ipv4_tuple.daddr));
	printk("sport     =%d\n", flow_tuple.ipv4_tuple.sport);
	printk("dport     =%d\n", flow_tuple.ipv4_tuple.dport);
	printk("ARG\n");
	printk("----------------------------------------------------\n");
	printk("mac da    =%d\t%s\n", flow_arg.mac_da, argv[7]);
	printk("mac sa    =%d\t%s\n", flow_arg.mac_sa, argv[8]);
	printk("new addr  =" NIPQUAD_FMT "\n", NIPQUAD(flow_arg.saddr));
	printk("new port  =%d\n", flow_arg.sport);
	printk("vlan tag  =%04x\n", flow_arg.vlan_tag);
	printk("pppoe sid =%04x\n", flow_arg.pppoe_sess_id);
	printk("outer vlan=%04x\n", flow_arg.outer_vlan_tag);
	printk("tc2206tag =%d\n", flow_arg.tc2206_tag);
	printk("----------------------------------------------------\n");
	printk("tx_portmap=%04x\n", tx_portmap);
	printk("action    =%d\n", action);
	printk("qid       =%d\n", qid);

	flow = flow_alloc(FLOW_LOW_PRIORITY);
	if (flow == NULL) {
		printk("no free flow\n");
		return 0;
	}

	flow_init(FLOW_NAT_ROUTING, flow, &flow_tuple, &flow_arg, tx_portmap, action, NULL, 0, qid, FLOW_LOW_PRIORITY, NULL);
	flow_insert(flow);
	printk("INSERT flow entry=%d hash=%d\n", flow->entry, flow->hash);

	return 0;
}

int do_hwnat_flow_add_bridge6(int argc, char *argv[], void *p)
{
	flow_t  *flow;
	uint8	tuple[FLOW_TUPLE_LEN];
	flow_tuple_t flow_tuple;
	flow_arg_t flow_arg;
    uint16	tx_portmap;       
    uint16	action;       
	uint8	qid;

	flow_tuple.tuple_index = (uint8) simple_strtoul(argv[1], NULL, 10);
	flow_tuple.ipv6_protocol = 1;
	in6_pton(argv[2], -1 /* len */,  &flow_tuple.ipv6_tuple.daddr, -1, NULL);

	tx_portmap = (uint16) simple_strtoul(argv[6], NULL, 16);
	action = (uint16) simple_strtoul(argv[7], NULL, 10);
	qid = (uint8) simple_strtoul(argv[8], NULL, 10);

	flow_make_tuple(&flow_tuple, tuple);

	if (flow_find(tuple)) {
		printk("duplicate flow\n");
		return 0;
	}

	memset(&flow_arg, 0x0, sizeof(flow_arg));

	flow_arg.vlan_tag = (uint16) simple_strtoul(argv[3], NULL, 16);
	flow_arg.outer_vlan_tag = (uint16) simple_strtoul(argv[4], NULL, 16);
	flow_arg.tc2206_tag = (uint8) simple_strtoul(argv[5], NULL, 10);

	printk("TUPLE\n");
	printk("----------------------------------------------------\n");
	printk("tuple     =%d\n", flow_tuple.tuple_index);
	printk("daddr     =" NIP6_FMT "\n", 
			NIP6(flow_tuple.ipv6_tuple.daddr));
	printk("----------------------------------------------------\n");
	printk("vlan tag  =%04x\n", flow_arg.vlan_tag);
	printk("outer vlan=%04x\n", flow_arg.outer_vlan_tag);
	printk("tc2206tag =%d\n", flow_arg.tc2206_tag);
	printk("----------------------------------------------------\n");
	printk("tx_portmap=%04x\n", tx_portmap);
	printk("action    =%d\n", action);
	printk("qid       =%d\n", qid);

	flow = flow_alloc(FLOW_LOW_PRIORITY);
	if (flow == NULL) {
		printk("no free flow\n");
		return 0;
	}

	flow_init(FLOW_BRIDGE, flow, &flow_tuple, &flow_arg, tx_portmap, action, NULL, 0, qid, FLOW_LOW_PRIORITY, NULL);
	flow_insert(flow);
	printk("INSERT flow entry=%d hash=%d\n", flow->entry, flow->hash);

	return 0;
}

int do_hwnat_flow_add_route6(int argc, char *argv[], void *p)
{
	flow_t  *flow;
	uint8	tuple[FLOW_TUPLE_LEN];
	flow_tuple_t flow_tuple;
	flow_arg_t flow_arg;
    uint16	tx_portmap;       
    uint16	action;       
    uint8	qid;       
	uint8 macaddr[6];

	flow_tuple.tuple_index = (uint8) simple_strtoul(argv[1], NULL, 10);
	flow_tuple.ipv6_protocol = 1;
	in6_pton(argv[2], -1 /* len */,  &flow_tuple.ipv6_tuple.daddr, -1, NULL);

	tx_portmap = (uint16) simple_strtoul(argv[9], NULL, 16);
	action = (uint16) simple_strtoul(argv[10], NULL, 10);
	qid = (uint8) simple_strtoul(argv[11], NULL, 10);

	flow_make_tuple(&flow_tuple, tuple);

	if (flow_find(tuple)) {
		printk("duplicate flow\n");
		return 0;
	}

	if (hwnat_mac_addr_in_ether(argv[3], macaddr)) {
		printk("err: mac addr format err\n");
		return 0;
	}

	flow_arg.mac_da = hwnat_mac_addr_insert(macaddr);

	if (hwnat_mac_addr_in_ether(argv[4], macaddr)) {
		printk("err: mac addr format err\n");
		return 0;
	}

	flow_arg.mac_sa = hwnat_mac_addr_insert(macaddr);
	flow_arg.saddr = 0x0;
	flow_arg.sport = 0x0;
	flow_arg.vlan_tag = (uint16) simple_strtoul(argv[5], NULL, 16);
	flow_arg.pppoe_sess_id = (uint16) simple_strtoul(argv[6], NULL, 16);
	flow_arg.outer_vlan_tag = (uint16) simple_strtoul(argv[7], NULL, 16);
	flow_arg.tc2206_tag = (uint8) simple_strtoul(argv[8], NULL, 10);

	printk("TUPLE\n");
	printk("----------------------------------------------------\n");
	printk("tuple     =%d\n", flow_tuple.tuple_index);
	printk("daddr     =" NIP6_FMT "\n", 
			NIP6(flow_tuple.ipv6_tuple.daddr));
	printk("ARG\n");
	printk("----------------------------------------------------\n");
	printk("mac da    =%d\t%s\n", flow_arg.mac_da, argv[7]);
	printk("mac sa    =%d\t%s\n", flow_arg.mac_sa, argv[8]);
	printk("new addr  =" NIPQUAD_FMT "\n", NIPQUAD(flow_arg.saddr));
	printk("new port  =%d\n", flow_arg.sport);
	printk("vlan tag  =%04x\n", flow_arg.vlan_tag);
	printk("pppoe sid =%04x\n", flow_arg.pppoe_sess_id);
	printk("outer vlan=%04x\n", flow_arg.outer_vlan_tag);
	printk("tc2206tag =%d\n", flow_arg.tc2206_tag);
	printk("----------------------------------------------------\n");
	printk("tx_portmap=%04x\n", tx_portmap);
	printk("action    =%d\n", action);
	printk("qid       =%d\n", qid);

	flow = flow_alloc(FLOW_LOW_PRIORITY);
	if (flow == NULL) {
		printk("no free flow\n");
		return 0;
	}

	flow_init(FLOW_PURE_ROUTING, flow, &flow_tuple, &flow_arg, tx_portmap, action, NULL, 0, qid, FLOW_LOW_PRIORITY, NULL);
	flow_insert(flow);
	printk("INSERT flow entry=%d hash=%d\n", flow->entry, flow->hash);

	return 0;
}

int do_hwnat_flow_update4mc(int argc, char *argv[], void *p)
{
	__be32 group;
    uint16 portmap;       

	group = in_aton(argv[1]);
	portmap = (uint16) simple_strtoul(argv[2], NULL, 16);

	hwnat_flow_update_ipv4_mc(group, portmap);

	return 0;
}

int do_hwnat_flow_update6mc(int argc, char *argv[], void *p)
{
	struct in6_addr group;
    uint16 portmap;       

	in6_pton(argv[1], -1 /* len */,  &group, -1, NULL);
	portmap = (uint16) simple_strtoul(argv[2], NULL, 16);

	hwnat_flow_update_ip6_mc(group, portmap);

	return 0;
}

int do_hwnat_flow_remove(int argc, char *argv[], void *p)
{
	uint16 entry;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	flow_remove(entry);
	printk("REMOVE flow entry=%d\n", entry);

	return 0;
}

int do_hwnat_flow_flush(int argc, char *argv[], void *p)
{
	uint16 entry;

	for (entry = 0; entry < FLOW_TBL_SIZE; entry++) 
		flow_remove(entry);

	return 0;
}

int do_hwnat_flow_timeout(int argc, char *argv[], void *p)
{
	flow_ctl.timeout = (uint8) simple_strtoul(argv[1], NULL, 10);
	printk("flow timeout = %d seconds\n", flow_ctl.timeout);

	return 0;
}

int do_hwnat_flow_add_on(int argc, char *argv[], void *p)
{
	if (argc >= 2)
		flow_ctl.add_flow_on = (uint8) (simple_strtoul(argv[1], NULL, 10) ? 1 : 0);
	else
		printk("add flow: %d\n", flow_ctl.add_flow_on);

	return 0;
}

int do_hwnat_flow_qid(int argc, char *argv[], void *p)
{
	flow_ctl.qid = (uint8) simple_strtoul(argv[1], NULL, 10);
	printk("flow default qid = %d\n", flow_ctl.qid);

	return 0;
}

int do_hwnat_flow_debug(int argc, char *argv[], void *p)
{
	flow_ctl.dbg = (uint8) simple_strtoul(argv[1], NULL, 10);
	flow_ctl.dbg_protocol = (uint8) simple_strtoul(argv[2], NULL, 10);
	flow_ctl.dbg_rxportmap = (uint16) simple_strtoul(argv[3], NULL, 16);
	flow_ctl.dbg_txportmap = (uint16) simple_strtoul(argv[4], NULL, 16);

	return 0;
}

#if 0
int do_hwnat_flow_transbrg(int argc, char *argv[], void *p)
{
	uint32 table_hold[FTH_SIZE];
	uint32 tuple[FLOW_TUPLE_LEN>>2];
	uint32 arg[FLOW_ARG_LEN>>2];
	uint16 hash;
	uint8 cmds[80];

	/* port 0 forward to port 1 */
	memset(tuple, 0x55, FLOW_TUPLE_LEN);
	memset(arg, 0x00, FLOW_ARG_LEN);
	tuple[4] = 0x550a0000;

	hash = flow_hash_fn((uint8 *) tuple, FLOW_TUPLE_LEN);

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[11] = 0x0;
	table_hold[10] = (0xff&0xff) | ((0xff<<8)&0xff00) | 
			((0x0<<16)&0xff0000) | ((0x2<<24)&0xff000000);
	table_hold[9] = ((0x0>>8)&0xff) | ((arg[3]<<8)&0xffffff00);
	table_hold[8] = ((arg[3]>>24)&0xff) | ((arg[2]<<8)&0xffffff00);
	table_hold[7] = ((arg[2]>>24)&0xff) | ((arg[1]<<8)&0xffffff00);
	table_hold[6] = ((arg[1]>>24)&0xff) | ((arg[0]<<8)&0xffffff00);
	table_hold[5] = ((arg[0]>>24)&0xff) | ((tuple[4]<<8)&0xffffff00);
	table_hold[4] = ((tuple[4]>>24)&0xff) | ((tuple[3]<<8)&0xffffff00);
	table_hold[3] = ((tuple[3]>>24)&0xff) | ((tuple[2]<<8)&0xffffff00);
	table_hold[2] = ((tuple[2]>>24)&0xff) | ((tuple[1]<<8)&0xffffff00);
	table_hold[1] = ((tuple[1]>>24)&0xff) | ((tuple[0]<<8)&0xffffff00);
	table_hold[0] = ((tuple[0]>>24)&0xff) | ((0<<8)&0x700) | ((1<<11)&0x800) | 
			((0<<12)&0xff000);

	hwnat_write_tbl(FLOW_TBL_ID, 1, table_hold, FTH_SIZE);
	hwnat_hash_tbl_write(hash, 1);

	/* port 1 forward to port 0 */
	memset(tuple, 0x55, FLOW_TUPLE_LEN);
	memset(arg, 0x00, FLOW_ARG_LEN);
	tuple[4] = 0x550a0001;

	hash = flow_hash_fn((uint8 *) tuple, FLOW_TUPLE_LEN);

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[11] = 0x0;
	table_hold[10] = (0xff&0xff) | ((0xff<<8)&0xff00) | 
			((0x0<<16)&0xff0000) | ((0x1<<24)&0xff000000);
	table_hold[9] = ((0x0>>8)&0xff) | ((arg[3]<<8)&0xffffff00);
	table_hold[8] = ((arg[3]>>24)&0xff) | ((arg[2]<<8)&0xffffff00);
	table_hold[7] = ((arg[2]>>24)&0xff) | ((arg[1]<<8)&0xffffff00);
	table_hold[6] = ((arg[1]>>24)&0xff) | ((arg[0]<<8)&0xffffff00);
	table_hold[5] = ((arg[0]>>24)&0xff) | ((tuple[4]<<8)&0xffffff00);
	table_hold[4] = ((tuple[4]>>24)&0xff) | ((tuple[3]<<8)&0xffffff00);
	table_hold[3] = ((tuple[3]>>24)&0xff) | ((tuple[2]<<8)&0xffffff00);
	table_hold[2] = ((tuple[2]>>24)&0xff) | ((tuple[1]<<8)&0xffffff00);
	table_hold[1] = ((tuple[1]>>24)&0xff) | ((tuple[0]<<8)&0xffffff00);
	table_hold[0] = ((tuple[0]>>24)&0xff) | ((0<<8)&0x700) | ((1<<11)&0x800) | 
			((0<<12)&0xff000);

	hwnat_write_tbl(FLOW_TBL_ID, 2, table_hold, FTH_SIZE);
	hwnat_hash_tbl_write(hash, 2);

	sprintf(cmds, "hwnat policy2 wr 0 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 1 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 2 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 3 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 4 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 5 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 6 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 7 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 8 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 9 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 10 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 11 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 12 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 13 ffff 10");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat policy2 wr 14 ffff 10");
	cmdparse(NULL, cmds, NULL);  

	sprintf(cmds, "hwnat cmpr wr 126 1 1 0000 1 0000 255 0 255 0 1 0 30 0 00");  
	cmdparse(NULL, cmds, NULL);  

	sprintf(cmds, "hwnat itf wr 0 126 0 1500");
	cmdparse(NULL, cmds, NULL);  
	sprintf(cmds, "hwnat itf wr 1 126 0 1500");
	cmdparse(NULL, cmds, NULL);  

	return 0;
}
#endif
