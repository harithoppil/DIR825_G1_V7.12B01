/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_pktflow.c#1 $
*/
/************************************************************************
 *
 *	Copyright (C) 2010 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
/*
** $Log: hwnat_pktflow.c,v $
** Revision 1.5  2011/06/30 12:07:32  lino
** hwnat enhance: IPv6 and QinQ support
**
** Revision 1.4  2011/06/10 04:37:49  lino
** add RT65168 support
**
** Revision 1.3  2011/06/08 10:02:23  lino
** add RT65168 support
**
** Revision 1.2  2011/06/03 02:41:46  lino
** add RT65168 support
**
** Revision 1.1.2.1  2011/04/01 09:16:43  lino
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
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/mii.h>
#include <linux/pktflow.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
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
#include "hwnat_rxport.h"
#include "hwnat_reg.h"
#include "hwnat_pktflow.h"
#include "hwnat_fte.h"
#include "hwnat_nfe.h"

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                            M A C R O S
*************************************************************************
*/
#define PKTFLOW_TAB_BITS		9
#define PKTFLOW_TAB_SIZE     	(1 << PKTFLOW_TAB_BITS)
#define PKTFLOW_TAB_MASK     	(PKTFLOW_TAB_SIZE - 1)

#define PKTFLOW_F_HWFLOW		(1<<0)
#define PKTFLOW_F_SWFLOW		(1<<1)
#define PKTFLOW_F_HASHED		(1<<2)

/* define dbg definition */
#define PKTFLOW_DBG_INFO		(1<<0)
#define PKTFLOW_DBG_SKB			(1<<1)

#define IN6_IS_ADDR_MULTICAST(a) (((__const uint8_t *) (a))[0] == 0xff)

#define IN6_IS_ADDR_MC_NODELOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const uint8_t *) (a))[1] & 0xf) == 0x1))

#define IN6_IS_ADDR_MC_LINKLOCAL(a) \
	(IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const uint8_t *) (a))[1] & 0xf) == 0x2))

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/

typedef union {
	struct {
    	uint32 tx_port_map 	: 16;
    	uint32 tx_queue   	: 3;
    	uint32 pkt_good  	: 1;
    	uint32 rx_port_map  : 4;
    	uint32 fte_vlan_tag : 8;
	} bits;
	uint32 word;
} dbginfo0_t;

typedef union {
	struct {
		uint32 en_nat  		: 1;
    	uint32 rxport_tag 	: 4;
    	uint32 last_qos   	: 5;
    	uint32 chksum_err  	: 1;
    	uint32 reserved  	: 5;
    	uint32 cmpr_code   	: 8;
		uint32 policy_hit	: 6;
		uint32 reserved1	: 2;
	} bits;
	uint32 word;
} dbginfo1_t;

typedef union {
	struct {
		uint32 tuple_hit	: 7;
    	uint32 hash_hit		: 9;
    	uint32 flow_idx   	: 8;
    	uint32 mdf_idx  	: 8;
	} bits;
	uint32 word;
} dbginfo2_t;

typedef union {
	struct {
		uint32 l2_addr		: 7;
    	uint32 l3_addr		: 7;
    	uint32 qos_miss		: 1;
    	uint32 qos_act0   	: 1;
    	uint32 l4_addr		: 7;
    	uint32 pkt_len		: 7;
    	uint32 qos_act1		: 2;
	} bits;
	uint32 word;
} dbginfo3_t;

typedef union {
	struct {
		uint32 policy_st	: 7;
    	uint32 l3_mtu_fail	: 1;
    	uint32 fte_st		: 3;
    	uint32 mdf_pkt_err 	: 1;
    	uint32 fte_port_err : 1;
    	uint32 qos_act2 	: 3;
		uint32 pce_dir		: 1;
    	uint32 hash_dir		: 1;
    	uint32 tue_dir		: 1;
    	uint32 fte_dir 		: 1;
    	uint32 pce_stm 		: 2;
    	uint32 tuple_stm 	: 5;
    	uint32 hash_stm 	: 2;
    	uint32 fte_stm 		: 3;
	} bits;
	uint32 word;
} dbginfo4_t;

typedef struct dbginfo_s {
	dbginfo0_t dbginfo0;
	dbginfo1_t dbginfo1;
	dbginfo2_t dbginfo2;
	dbginfo3_t dbginfo3;
	dbginfo4_t dbginfo4;
} dbginfo_t;

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static void pktflow_expire(unsigned long data);

/************************************************************************
*                        P U B L I C   D A T A
*************************************************************************
*/

/************************************************************************
*                      E X T E R N A L   D A T A
*************************************************************************
*/
extern uint32 hwnat_udp_on;
extern uint32 hwnat_icmp_on;
extern uint32 hwnat_ipv6_on;
extern uint32 hwnat_multicast_on;
extern uint32 hwnat_ipv6_multicast_on;

/************************************************************************
*                       P R I V A T E   D A T A
*************************************************************************
*/

static struct list_head *pktflow_tab;

static unsigned int pktflow_rnd;

DEFINE_SPINLOCK(pktflow_lock);

static uint32 pktflow_cnt = 0;

static uint32 pktflow_dbg = 0;

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

const char * strPktflowEncap[] = {
    "ETH_802x",
    "TC2206_STAG",
    "OUT_VLAN_8021Q",
    "VLAN_8021Q",
    "PPPoE_2516",
    "PROTO_MAX"
};

void pktflow_l2_dump( PktflowHeader_t * bHdr_p, PktflowDir_t dir)
{
    register int i, ix, length, offset = 0;
    PktflowEncap_t type;
    char * value = bHdr_p->l2hdr;

    for (ix = 0; ix < bHdr_p->info.count; ix++) {
        type = bHdr_p->encap[ix];

        switch (type) {
            case ETH_802x: 
				length = ETH_ALEN + ETH_ALEN;     
				break;
            case TC2206_STAG: 
				length = (dir == DIR_RX) ? 8 : 6;   
				break;
            case OUT_VLAN_8021Q: 
				length = VLAN_HLEN; 
				break;
            case VLAN_8021Q: 
				length = VLAN_HLEN; 
				break;
            case PPPoE_2516: 
				length = sizeof(struct pppoe_hdr) + sizeof(uint16_t);        
				break;
            default: 
				printk("unknown type %d\n", type);
               	return;
        }

        printk("\tENCAP %d. %10s +%2d %2d [ ",
                ix, strPktflowEncap[type], offset, length );

        for (i = 0; i < length; i++)
            printk("%02x ", (uint8_t)value[i]);

        offset += length;
        value += length;

        printk( "]\n" );
    }
}

void pktflow_tuple_dump(PktflowTuple_t * bTuple_p)
{
    printk( "\tTUPLE: Src<" NIPQUAD_FMT ":%u> Dst<" NIPQUAD_FMT ":%u>\n",
            NIPQUAD(bTuple_p->ip4_u.saddr), bTuple_p->ip4_u.port.source,
            NIPQUAD(bTuple_p->ip4_u.daddr), bTuple_p->ip4_u.port.dest);
}

void pktflow_dump(Pktflow_t *pktflow_p)
{
    if ( pktflow_p == NULL )
        return;

    printk( "PKTFLOW prot<%u> flags<%04x> count<%04x>\n",
            pktflow_p->protocol, pktflow_p->flags, pktflow_p->hit_count);

    printk( "  RX conn_id<%02u>\n",
            pktflow_p->rx.info.itf);
    pktflow_tuple_dump( &pktflow_p->rx.tuple );
	pktflow_l2_dump(&pktflow_p->rx, DIR_RX);

    printk( "  TX conn_id<%02u>\n",
            pktflow_p->tx.info.itf);
    pktflow_tuple_dump( &pktflow_p->tx.tuple );
	pktflow_l2_dump(&pktflow_p->tx, DIR_TX);
}

void pktflow_simple_tuple_dump(PktflowTuple_t * bTuple_p)
{
    printk("<" NIPQUAD_FMT ":%u " NIPQUAD_FMT ":%u>",
            NIPQUAD(bTuple_p->ip4_u.saddr), bTuple_p->ip4_u.port.source,
            NIPQUAD(bTuple_p->ip4_u.daddr), bTuple_p->ip4_u.port.dest);
}

void pktflow_simple_dump(Pktflow_t *pktflow_p)
{
    if ( pktflow_p == NULL )
        return;

    printk("%u", pktflow_p->protocol);

    printk("|");
    pktflow_simple_tuple_dump( &pktflow_p->rx.tuple );

    printk("|");
    pktflow_simple_tuple_dump( &pktflow_p->tx.tuple );
    printk("\n");
}

static void dump_skb(struct sk_buff *skb) 
{
	char tmp[80];
	char *p = skb->data;
	char *t = tmp;
	int i, n = 0;

	printk("ERR skb=%08lx data=%08lx len=%d\n", (uint32) skb, (uint32) skb->data, skb->len);
	for (i = 0; i < skb->len; i++) {
		t += sprintf(t, "%02x ", *p++ & 0xff);
		if ((i & 0x0f) == 0x0f) {
			printk("%04x: %s\n", n, tmp);
			n += 16;
			t = tmp;
		}
	}
	if (i & 0x0f)
		printk("%04x: %s\n", n, tmp);
}

static void dump_hwnat_dbginfo(dbginfo_t *dbg_info)
{
	printk("nat_en=%d tx_port_map=%04x q=%d good=%d rx_port_map=%d vlan_tag=%04x\n", 
		dbg_info->dbginfo1.bits.en_nat,
		dbg_info->dbginfo0.bits.tx_port_map,
		dbg_info->dbginfo0.bits.tx_queue,
		dbg_info->dbginfo0.bits.pkt_good,
		dbg_info->dbginfo0.bits.rx_port_map,
		dbg_info->dbginfo0.bits.fte_vlan_tag);
	printk("  rxport_tag=%x fifo_err=%d cmpr=%d policy=%d tuple=%d hash=%d flow=%d mdf=%d\n",
		dbg_info->dbginfo1.bits.rxport_tag,
		dbg_info->dbginfo1.bits.chksum_err,
		dbg_info->dbginfo1.bits.cmpr_code,
		dbg_info->dbginfo1.bits.policy_hit,
		dbg_info->dbginfo2.bits.tuple_hit,
		dbg_info->dbginfo2.bits.hash_hit,
		dbg_info->dbginfo2.bits.flow_idx,
		dbg_info->dbginfo2.bits.mdf_idx);
	printk("  QOS last=%d miss=%d act=%02x\n",
		dbg_info->dbginfo1.bits.last_qos,
		dbg_info->dbginfo3.bits.qos_miss,
		(dbg_info->dbginfo3.bits.qos_act0<<5 | dbg_info->dbginfo3.bits.qos_act1<<3 |
		dbg_info->dbginfo4.bits.qos_act2));
	printk("  l2=%d l3=%d l4=%d pkt_len=%d\n",
		dbg_info->dbginfo3.bits.l2_addr,
		dbg_info->dbginfo3.bits.l3_addr,
		dbg_info->dbginfo3.bits.l4_addr,
		dbg_info->dbginfo3.bits.pkt_len);
	printk("  ST policy=%04x l3_mtu_fail=%d fte=%x loc_err=%d txport0=%d\n", 
		dbg_info->dbginfo4.bits.policy_st,
		dbg_info->dbginfo4.bits.l3_mtu_fail,
		dbg_info->dbginfo4.bits.fte_st,
		dbg_info->dbginfo4.bits.mdf_pkt_err,
		dbg_info->dbginfo4.bits.fte_port_err);
	printk("  DIR pce=%d hash=%d tue=%d fte=%d STM pce=%d tuple=%x hash=%d fte=%x\n", 
		dbg_info->dbginfo4.bits.pce_dir,
		dbg_info->dbginfo4.bits.hash_dir,
		dbg_info->dbginfo4.bits.tue_dir,
		dbg_info->dbginfo4.bits.fte_dir,
		dbg_info->dbginfo4.bits.pce_stm,
		dbg_info->dbginfo4.bits.tuple_stm,
		dbg_info->dbginfo4.bits.hash_stm,
		dbg_info->dbginfo4.bits.fte_stm);
}

static uint32 pktflow_hashkey(Pktflow_t *pktflow_p)
{
	PktflowHeader_t *bHdr_p;
	PktflowTuple_t *bTuple_p;

    bHdr_p = &pktflow_p->rx;
	bTuple_p = &bHdr_p->tuple;

	if (bHdr_p->info.hdrs & (1<<ETH_IPV4))
		return jhash_3words(bTuple_p->ip4_u.saddr, bTuple_p->ip4_u.daddr, bTuple_p->ip4_u.ports, pktflow_rnd) & PKTFLOW_TAB_MASK;
	else
		return jhash_3words(bTuple_p->ip6_u.daddr.in6_u.u6_addr32[3], bTuple_p->ip6_u.daddr.in6_u.u6_addr32[2], bTuple_p->ip6_u.daddr.in6_u.u6_addr32[1], pktflow_rnd) & PKTFLOW_TAB_MASK;
}

Pktflow_t *pktflow_find(Pktflow_t *pktflow_p, uint32 hash)
{
	Pktflow_t *pf, *ret=NULL;
	unsigned long flags;

	spin_lock_irqsave(&pktflow_lock, flags);
	list_for_each_entry(pf, &pktflow_tab[hash], list) {
		if (pktflow_p->protocol == pf->protocol &&
			(((pktflow_p->rx.info.hdrs & (1<<ETH_IPV4)) && 
			  pktflow_p->rx.tuple.ip4_u.saddr == pf->rx.tuple.ip4_u.saddr && 
			  pktflow_p->rx.tuple.ip4_u.daddr == pf->rx.tuple.ip4_u.daddr &&
			  pktflow_p->rx.tuple.ip4_u.port.source == pf->rx.tuple.ip4_u.port.source &&
			  pktflow_p->rx.tuple.ip4_u.port.dest == pf->rx.tuple.ip4_u.port.dest) ||
			(((pktflow_p->rx.info.hdrs & (1<<ETH_IPV6)) && 
			  !memcmp(&pktflow_p->rx.tuple.ip6_u.daddr, &pf->rx.tuple.ip6_u.daddr, 16))))) {
			ret = pf;
			/* HIT */
			atomic_inc(&pf->refcnt);
			break;
		}
	}
	spin_unlock_irqrestore(&pktflow_lock, flags);

	return ret;
}

int pktflow_hash(Pktflow_t *pktflow_p)
{
	int ret;
	unsigned long flags;
	uint32 hash;

	hash = pktflow_hashkey(pktflow_p);

	spin_lock_irqsave(&pktflow_lock, flags);
	if (!(pktflow_p->flags & PKTFLOW_F_HASHED)) {
		list_add(&pktflow_p->list, &pktflow_tab[hash]);
		pktflow_p->flags |= PKTFLOW_F_HASHED;
		atomic_inc(&pktflow_p->refcnt);
		pktflow_cnt++;
		ret = 1;
	} else {
		printk("pktflow_hash(): request for already hashed, "
			  	"called from %p\n", __builtin_return_address(0));
		ret = 0;
	}
	spin_unlock_irqrestore(&pktflow_lock, flags);

	return ret;
}

int pktflow_unhash(Pktflow_t *pktflow_p)
{
	int ret;
	unsigned long flags;
	uint32 hash;

	hash = pktflow_hashkey(pktflow_p);

	spin_lock_irqsave(&pktflow_lock, flags);
	if (pktflow_p->flags & PKTFLOW_F_HASHED) {
		list_del(&pktflow_p->list);
		pktflow_p->flags &= ~PKTFLOW_F_HASHED;
		atomic_dec(&pktflow_p->refcnt);
		pktflow_cnt--;
		ret = 1;
	} else {
		ret = 0;
	}
	spin_unlock_irqrestore(&pktflow_lock, flags);

	return ret;
}

Pktflow_t *pktflow_hash_init(struct sk_buff *skb)
{
	Pktflow_t *pf;

	pf = pktflow_null(skb);

	pf->flags &= ~(PKTFLOW_F_HWFLOW | PKTFLOW_F_SWFLOW | PKTFLOW_F_HASHED);
	pf->flags |= PKTFLOW_F_SWFLOW;
	pf->hit_count = 1;
	INIT_LIST_HEAD(&pf->list);

	init_timer(&pf->timer);
	pf->timer.data     = (unsigned long)pf;
	pf->timer.function = pktflow_expire;

	atomic_set(&pf->refcnt, 1);

	pf->timeout = 10*HZ;

	pktflow_hash(pf);

	return pf;
}

void pktflow_start(Pktflow_t *pktflow_p)
{
	/* reset it expire in its timeout */
	mod_timer(&pktflow_p->timer, jiffies+pktflow_p->timeout);

	atomic_dec(&pktflow_p->refcnt);
}

static void pktflow_expire(unsigned long data)
{
	Pktflow_t *pktflow_p = (Pktflow_t *)data;
	unsigned long flags;

	/*
	printk("pktflow_expire: Proto<%d> Src<" NIPQUAD_FMT ":%u> Dst<" NIPQUAD_FMT ":%u>\n",
		pktflow_p->protocol, 
		NIPQUAD(pktflow_p->rx.tuple.saddr), pktflow_p->rx.tuple.port.source,
		NIPQUAD(pktflow_p->rx.tuple.daddr), pktflow_p->rx.tuple.port.dest);
	*/
	/*
	if (hwnat_debug & 0x40) {
		printk("pktflow_exp: ");
		pktflow_simple_dump(pktflow_p);
	}
	*/

	/*
	 *	hey, I'm using it
	 */
	atomic_inc(&pktflow_p->refcnt);

	/*
	 *	unhash it if it is hashed in the conn table
	 */
	if (!pktflow_unhash(pktflow_p))
		goto expire_later;

	/*
	 *	refcnt==1 implies I'm the only one referrer
	 */
	if (likely(atomic_read(&pktflow_p->refcnt) == 1)) {
		/* delete the timer if it is activated by other users */
		if (timer_pending(&pktflow_p->timer))
			del_timer(&pktflow_p->timer);

		pktflow_put(pktflow_p);
		return;
	}

	/* hash it back to the table */
	pktflow_hash(pktflow_p);

expire_later:
	printk("err: delayed refcnt=%d pktflow_cnt=%d\n",
		  atomic_read(&pktflow_p->refcnt), pktflow_cnt);

	pktflow_start(pktflow_p);
}

void pktflow_expire_now(Pktflow_t *pf)
{
	if (del_timer(&pf->timer))
		mod_timer(&pf->timer, jiffies);
}

void pktflow_l2_log(PktflowHeader_t * bHdr_p, PktflowEncap_t encap, size_t len, void *data_p)
{
	register short int *d;
   	register const short int *s;

	bHdr_p->info.hdrs |= (1U << encap);
   	bHdr_p->encap[ bHdr_p->info.count++ ] = encap;

	s = (const short int *)data_p;
	d = (short int *)&(bHdr_p->l2hdr[bHdr_p->length]);
	bHdr_p->length += len;

	switch (len) {
		case 20: *(d+9)=*(s+9);
				 *(d+8)=*(s+8);
				 *(d+7)=*(s+7);
		case 14: *(d+6)=*(s+6);
		case 12: *(d+5)=*(s+5);
		case 10: *(d+4)=*(s+4);
		case  8: *(d+3)=*(s+3);
		case  6: *(d+2)=*(s+2);
		case  4: *(d+1)=*(s+1);
		case  2: *(d+0)=*(s+0);
			break;
		default:
			break;
	}
}

char *pktflow_l2_get(PktflowHeader_t * bHdr_p, PktflowDir_t dir, PktflowEncap_t encap, int offset)
{
    register int ix, length;
    PktflowEncap_t type;
    char * value = bHdr_p->l2hdr;

    for (ix = 0; ix < bHdr_p->info.count; ix++) {
        type = bHdr_p->encap[ix];
		if (type == encap)
			return (value+offset);

        switch (type) {
            case ETH_802x: 
				length = ETH_ALEN + ETH_ALEN;     
				break;
            case TC2206_STAG: 
				length = (dir == DIR_RX) ? 8 : 6;   
				break;
            case OUT_VLAN_8021Q : 
				length = VLAN_HLEN; 
				break;
            case VLAN_8021Q : 
				length = VLAN_HLEN; 
				break;
            case PPPoE_2516 : 
				length = sizeof(struct pppoe_hdr) + sizeof(uint16_t);        
				break;
            default: 
               	return NULL;
        }

        value += length;
    }

	return NULL;
}

void pktflow_log(struct sk_buff *skb, PktflowDir_t dir, size_t len, void *data_p)
{
    PktflowHeader_t *bHdr_p;
    Pktflow_t *pktflow_p;
	struct ethhdr *eth_p = (struct ethhdr *) data_p;
	__be16 h_proto;
	__be16 pppoe_proto;

    pktflow_p = skb->pktflow_p;

    bHdr_p = &pktflow_p->rx + dir;

	/* skip broadcast address */
	if (is_broadcast_ether_addr(eth_p->h_dest))
		goto skip;

	if (!hwnat_multicast_on && is_multicast_ether_addr(eth_p->h_dest)) 
		goto skip;

	if (is_multicast_ether_addr(eth_p->h_dest)) 
		bHdr_p->info.multicast = 1;

	pktflow_l2_log(bHdr_p, ETH_802x, ETH_ALEN + ETH_ALEN, data_p);

   	data_p += ETH_ALEN + ETH_ALEN;
	h_proto = *(__be16 *) data_p;

	/* check TC2206 special tag */
	if (h_proto == 0x8901) {
		int tc2206_stag_len = (dir == DIR_RX) ? 8 : 6; 

		pktflow_l2_log(bHdr_p, TC2206_STAG, tc2206_stag_len, data_p);

    	data_p += tc2206_stag_len;
		h_proto = *(__be16 *) data_p;
	}

	/* check VLAN tag */
	if (h_proto == ETH_P_8021Q) {
        void *vlan_data_p = data_p;

    	data_p += VLAN_HLEN;
		h_proto = *(__be16 *) data_p;

		/* check QinQ VLAN tag */
		if (h_proto == ETH_P_8021Q) {
			/* don't learn QinQ now */
			if (dir == DIR_TX)
				goto skip;

			pktflow_l2_log(bHdr_p, OUT_VLAN_8021Q, VLAN_HLEN, vlan_data_p);

			pktflow_l2_log(bHdr_p, VLAN_8021Q, VLAN_HLEN, data_p);

    		data_p += VLAN_HLEN;
			h_proto = *(__be16 *) data_p;
		} else {
			pktflow_l2_log(bHdr_p, VLAN_8021Q, VLAN_HLEN, vlan_data_p);
		}
	}

	/* check IPv4 or IPv6 packet */
	if ((h_proto != ETH_P_IP) && (h_proto != ETH_P_PPP_SES) &&
		(h_proto != ETH_P_IPV6))
		goto skip;

	/* skip ether type */
   	data_p += 2;
	/* check PPPoE packet */
	if (h_proto == ETH_P_PPP_SES) {
		pktflow_l2_log(bHdr_p, PPPoE_2516, PPPOE_SES_HLEN, data_p);

		data_p += sizeof(struct pppoe_hdr);

		pppoe_proto = *(__be16 *) data_p;

		/* skip ppp header */
    	data_p += 2;

		if (pppoe_proto == PPP_IP)
			h_proto = ETH_P_IP;
		else if (pppoe_proto == PPP_IPV6)
			h_proto = ETH_P_IPV6;
		else
			goto skip;
	}

	/* IPv4 packet */
	if (h_proto == ETH_P_IP) {
       	PktflowTuple_t *bTuple_p = &bHdr_p->tuple;
        struct iphdr *iph = (struct iphdr *)((uint8_t *) data_p);

        /* if non IPv4 or with IP options, or fragmented */
        if ((iph->version != 4) || (iph->ihl != 5)
             || (iph->frag_off & htons(IP_OFFSET | IP_MF)))
            goto skip;

        if (iph->protocol == IPPROTO_TCP) {
            struct tcphdr *tcph;
            tcph = (struct tcphdr *)((uint8_t *) iph + sizeof(struct iphdr));

            if (tcph->rst | tcph->fin)    /* Discontinue if TCP RST/FIN */
                goto skip;

            bTuple_p->ip4_u.port.source = tcph->source;
            bTuple_p->ip4_u.port.dest = tcph->dest;
        } else if (iph->protocol == IPPROTO_UDP) {
            struct udphdr *udph;

			if (!hwnat_udp_on)
				goto skip;

            udph = (struct udphdr *)( (uint8_t *) iph + sizeof(struct iphdr));
            bTuple_p->ip4_u.port.source = udph->source;
            bTuple_p->ip4_u.port.dest = udph->dest;
        } else if (iph->protocol == IPPROTO_ICMP) {
            struct icmphdr *icmph;

			if (!hwnat_icmp_on)
				goto skip;

            icmph = (struct icmphdr *)( (uint8_t *) iph + sizeof(struct iphdr));
			if ((icmph->type != ICMP_ECHOREPLY) && (icmph->type != ICMP_ECHO))
                goto skip;

            bTuple_p->ip4_u.port.source = (icmph->type << 8) | icmph->code;
            bTuple_p->ip4_u.port.dest = icmph->un.echo.id;
		} else {
            bTuple_p->ip4_u.port.source = 0;
            bTuple_p->ip4_u.port.dest = 0;
		}

        bTuple_p->ip4_u.saddr = iph->saddr;
        bTuple_p->ip4_u.daddr = iph->daddr;
        pktflow_p->protocol = iph->protocol;

		bHdr_p->info.hdrs |= (1U << ETH_IPV4);
	}
	/* IPv6 packet */
	else if (h_proto == ETH_P_IPV6) {
       	PktflowTuple_t *bTuple_p = &bHdr_p->tuple;
		struct ipv6hdr *hdr = (struct ipv6hdr *)((uint8_t *) data_p);

		if (!hwnat_ipv6_on)
			goto skip;

        /* if non IPv6 */
		if (hdr->version != 6)
            goto skip;

		/* check if ICMPv6 */
        if (hdr->nexthdr == IPPROTO_ICMPV6) {
			if (!hwnat_icmp_on)
				goto skip;
		}

		/* check if IPv6 multicast */
		if (is_multicast_ether_addr(eth_p->h_dest) &&
			(IN6_IS_ADDR_MC_NODELOCAL(&hdr->daddr) || IN6_IS_ADDR_MC_LINKLOCAL(&hdr->daddr)))
				goto skip;

		if (!hwnat_ipv6_multicast_on && is_multicast_ether_addr(eth_p->h_dest)) 
				goto skip;

		memcpy(&bTuple_p->ip6_u.daddr, &hdr->daddr, 16);

        pktflow_p->protocol = hdr->nexthdr;

		bHdr_p->info.hdrs |= (1U << ETH_IPV6);
	}

    return;

skip:   
    pktflow_free(skb);
}

int pktflow_rx_dbg(struct sk_buff *skb, int port, dbginfo_t *dbg_info)
{
	int ret = 0;

	if (!dbg_info)
		return ret;

	/* skip CPU only packets */
	if (dbg_info->dbginfo0.bits.tx_port_map & ~(1<<HWNAT_PORT_CPU)) {
		ret = 1;

		/* dump hwnat debug info */
		if (pktflow_dbg & PKTFLOW_DBG_INFO)
			dump_hwnat_dbginfo(dbg_info);

		/* dump skb content */
		if (pktflow_dbg & PKTFLOW_DBG_SKB)
			dump_skb(skb);
	}

	if (ret)
		dev_kfree_skb(skb);

	return ret;
}

int pktflow_rx(struct sk_buff *skb, int port, unsigned int *dbg_info)
{
	Pktflow_t *pktflow_p;
	struct ethhdr *eth_p = (struct ethhdr *) skb->data;

	if (!hwnat_active)
		return 0;

	if (unlikely(pktflow_dbg)) {
		if (pktflow_rx_dbg(skb, port, (dbginfo_t *) dbg_info))
			return 1;
	}

	/* multicast packet get from high-priority queue */
	if (hwnat_multicast_on && is_multicast_ether_addr(eth_p->h_dest)) 
    	pktflow_p = pktflow_hp_get();                    
	else
    pktflow_p = pktflow_get();                    
    if (unlikely(pktflow_p == NULL)) 
		return 0;

	skb->pktflow_p = pktflow_p;
    pktflow_p->skb = skb;

    pktflow_p->rx.info.itf = port;

    pktflow_p->rx.length = 0;

	pktflow_log(skb, DIR_RX, skb->len, skb->data);

	if (skb->pktflow_p != NULL) 
		return 0;
	else {
		pktflow_free(skb);
		return 0;
	}
}

int pktflow_tx(struct sk_buff *skb, int port)
{
	Pktflow_t *pf;
	Pktflow_t *pktflow_p = skb->pktflow_p;
	uint32 hash;
	struct ethhdr *eth_p = (struct ethhdr *) skb->data;

	if (!hwnat_active)
		return 0;

    if (unlikely(pktflow_p == NULL)) 
		return 0;

	pktflow_log(skb, DIR_TX, skb->len, skb->data);

	if (skb->pktflow_p == NULL) {
		return 0;
	}

    pktflow_p->tx.dev_p = skb->dev;

    pktflow_p->tx.info.itf = port;

    pktflow_p->tx.length = 0;

	hash = pktflow_hashkey(pktflow_p);

	pf = pktflow_find(pktflow_p, hash);

	if (pf) {
		if (pf->flags & PKTFLOW_F_SWFLOW) {
			pf->hit_count++;

			/* update pktflow information */
			pf->fdb_src = pktflow_p->fdb_src;

			/* update pktflow's stag information */
			/* get tc2206 special tag */
			if ((pf->tx.info.hdrs & (1<<TC2206_STAG)) &&
			 	(pktflow_p->tx.info.hdrs & (1<<TC2206_STAG))) { 
				uint8 *org_tc2206_tag = (uint8 *) pktflow_l2_get(&pf->tx, DIR_TX, TC2206_STAG, 4);
				uint8 *new_tc2206_tag = (uint8 *) pktflow_l2_get(&pktflow_p->tx, DIR_TX, TC2206_STAG, 4);
				
				if (*org_tc2206_tag != *new_tc2206_tag) {
					*org_tc2206_tag = *new_tc2206_tag;
				}
			}

			if ((pf->hit_count >= hwnat_hitcnt) ||
					(pf->rx.info.multicast && (pf->hit_count >= hwnat_multicast_hitcnt))) {

				if (hwnat_flow_add(pf)) {
					/* fail */
				} else {
					pf->flags &= ~PKTFLOW_F_SWFLOW;
					pf->flags |= PKTFLOW_F_HWFLOW;
				}
			}
		}
		pktflow_start(pf);
	} else {
		pf = pktflow_hash_init(skb);
		pktflow_start(pf);
	}

	pktflow_free(skb);

	return 0;
}

void pktflow_nfct_close(struct nf_conn *ct)
{
#if 0
    if ((ct->pktflow_key[IP_CT_DIR_ORIGINAL] != 0) ||
    	(ct->pktflow_key[IP_CT_DIR_REPLY] != 0))
		printk("pkt_nfct_close key1=%d key2=%d\n",
    		ct->pktflow_key[IP_CT_DIR_ORIGINAL],
    		ct->pktflow_key[IP_CT_DIR_REPLY]);
#endif

    if (ct->pktflow_key[IP_CT_DIR_ORIGINAL] != 0) {
		hwnat_flow_remove(ct->pktflow_key[IP_CT_DIR_ORIGINAL]);
    	ct->pktflow_key[IP_CT_DIR_ORIGINAL] = 0;
	}

    if (ct->pktflow_key[IP_CT_DIR_REPLY] != 0) {
		hwnat_flow_remove(ct->pktflow_key[IP_CT_DIR_REPLY]);
    	ct->pktflow_key[IP_CT_DIR_REPLY] = 0;
	}
}

void pktflow_fdb_delete(struct net_bridge_fdb_entry *f)
{
	if (f)
		hwnat_flow_remove_by_fdb(f);
}

void pktflow_update_ipv4_mc(__be32 group, unsigned int portmap)
{
	hwnat_flow_update_ipv4_mc(group, portmap);
}

void pktflow_update_ip6_mc(struct in6_addr group, unsigned int portmap)
{
	hwnat_flow_update_ip6_mc(group, portmap);
}

int hwnat_pktflow_init(void)
{
	int idx;

	pktflow_tab = (struct list_head *) kmalloc(PKTFLOW_TAB_SIZE*sizeof(struct list_head), GFP_ATOMIC);
	if (!pktflow_tab)
		return -ENOMEM;

	for (idx = 0; idx < PKTFLOW_TAB_SIZE; idx++) {
		INIT_LIST_HEAD(&pktflow_tab[idx]);
	}

	/* calculate the random value for connection hash */
	get_random_bytes(&pktflow_rnd, sizeof(pktflow_rnd));

	//printk("pktflow register \n");
	pktflow_register(pktflow_rx, pktflow_tx, pktflow_nfct_close, pktflow_fdb_delete, 
			hwnat_nfe_get_stats, hwnat_nfe_clear_stats, pktflow_update_ipv4_mc, pktflow_update_ip6_mc);

	return 0;
}

void hwnat_pktflow_exit(void)
{
	int idx;
	unsigned long flags;
	Pktflow_t *pf, *tmp;

	pktflow_unregister();

	spin_lock_irqsave(&pktflow_lock, flags);
	for (idx = 0; idx < PKTFLOW_TAB_SIZE; idx++) {
		list_for_each_entry_safe(pf, tmp, &pktflow_tab[idx], list) {
			del_timer(&pf->timer);
			list_del(&pf->list);
			pktflow_put(pf);
		}
	}
	spin_unlock_irqrestore(&pktflow_lock, flags);

	kfree(pktflow_tab);
}

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/

static int do_hwnat_pktflow_info(int argc, char *argv[], void *p);
static int do_hwnat_pktflow_debug(int argc, char *argv[], void *p);

static const cmds_t hwnat_pktflow_cmds[] = {
	{"info",		do_hwnat_pktflow_info,		0x02,  	0,  NULL},
	{"debug",		do_hwnat_pktflow_debug,		0x02,  	1,  "<dbg>"},
	{NULL,			NULL,						0x10,	0,	NULL},
};

int do_hwnat_pktflow(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_pktflow_cmds, argc, argv, p);
}

int do_hwnat_pktflow_info(int argc, char *argv[], void *p)
{
	printk("pktflow_cnt=%d\n", pktflow_cnt);
	return 0;
}

int do_hwnat_pktflow_debug(int argc, char *argv[], void *p)
{
	pktflow_dbg = simple_strtoul(argv[1], NULL, 16);

	return 0;
}

