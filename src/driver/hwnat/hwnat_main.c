/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_main.c#1 $
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
** $Log: hwnat_main.c,v $
** Revision 1.5  2011/06/30 12:07:32  lino
** hwnat enhance: IPv6 and QinQ support
**
** Revision 1.4  2011/06/10 07:53:43  lino
** add RT65168 support
**
** Revision 1.3  2011/06/08 10:02:23  lino
** add RT65168 support
**
** Revision 1.2  2011/06/03 02:41:45  lino
** add RT65168 support
**
** Revision 1.1.2.1  2011/04/01 09:16:42  lino
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
//#include "../../version/tcversion.h"
#include "../include/tcversion.h"
#include "hwnat.h"
#include "hwnat_pktflow.h"
#include "hwnat_itf.h"
#include "hwnat_gpr.h"
#include "hwnat_pce.h"
#include "hwnat_tue.h"
#include "hwnat_fte.h"
#include "hwnat_mde.h"
#include "hwnat_mac.h"
#include "hwnat_qos.h"
#include "hwnat_nfe.h"
#include "hwnat_rxport.h"
#include "hwnat_reg.h"

#ifdef HWNAT_SW_MODEL
#include <hwnat_emulation.h>
#endif

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                            M A C R O S
*************************************************************************
*/
#define HWNAT_HITCNT			256
#define HWNAT_MULTICAST_HITCNT	32

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/

#ifdef HWNAT_SW_MODEL
typedef struct hwnatStat_s {
	uint32 inPkts;
	uint32 inHitPkts;
	uint32 inMissPkts;
	uint32 txPortPkts;
} hwnatStat_t;

hwnatStat_t hwnatPortStat[INTERFACE_TBL_SIZE];

DEFINE_SPINLOCK(hwnat_lock);
#endif

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static void hwnat_cmd_init(void);
static void hwnat_cmd_exit(void);

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

uint32 hwnat_active = 1;
uint32 hwnat_debug = 0;
uint32 hwnat_hitcnt = HWNAT_HITCNT;
uint32 hwnat_multicast_hitcnt = HWNAT_MULTICAST_HITCNT;

uint32 hwnat_udp_on = 1;
uint32 hwnat_icmp_on = 0;
uint32 hwnat_ipv6_on = 1;
uint32 hwnat_multicast_on = 1;
uint32 hwnat_ipv6_multicast_on = 0;

static int do_hwnat_active(int argc, char *argv[], void *p);
static int do_hwnat_debug(int argc, char *argv[], void *p);
static int do_hwnat_hitcnt(int argc, char *argv[], void *p);
static int do_hwnat_mc_hitcnt(int argc, char *argv[], void *p);
static int do_hwnat_icmp(int argc, char *argv[], void *p);
static int do_hwnat_udp(int argc, char *argv[], void *p);
static int do_hwnat_ipv6(int argc, char *argv[], void *p);
static int do_hwnat_multicast(int argc, char *argv[], void *p);
static int do_hwnat_ipv6_multicast(int argc, char *argv[], void *p);
static int do_hwnat_stat(int argc, char *argv[], void *p);

static const cmds_t hwnat_cmds[] = {
	{"active",		do_hwnat_active,		0x02,  	1,  "<active>"},
	{"debug",		do_hwnat_debug,			0x02,  	1,  "<debug>"},
	{"hitcnt",		do_hwnat_hitcnt,		0x02,  	1,  "<hitcnt>"},
	{"mchitcnt",	do_hwnat_mc_hitcnt,		0x02,  	1,  "<hitcnt>"},
	{"icmp",		do_hwnat_icmp,			0x02,  	1,  "<icmp>"},
	{"udp",			do_hwnat_udp,			0x02,  	1,  "<udp>"},
	{"ipv6",		do_hwnat_ipv6,			0x02,  	1,  "<ipv6>"},
	{"multicast",	do_hwnat_multicast,		0x02,  	1,  "<multicast>"},
	{"ipv6mc",		do_hwnat_ipv6_multicast,0x02,  	1,  "<multicast>"},
#ifdef HWNAT_SW_MODEL
	{"stat",		do_hwnat_stat,			0x02,  	0,  NULL},
#endif
	{"pktflow",		do_hwnat_pktflow,		0x12,  	0,  NULL},
	{"reg",			do_hwnat_reg,			0x12,  	0,  NULL},
	{"itf",			do_hwnat_itf,			0x12,  	0,  NULL},
	{"gpr",			do_hwnat_gpr,			0x12,  	0,  NULL},
	{"cmpr",		do_hwnat_cmpr,			0x12,  	0,  NULL},
	{"policy",		do_hwnat_policy,		0x12,  	0,  NULL},
	{"policy2",		do_hwnat_policy_result,	0x12,  	0,  NULL},
	{"tuple",		do_hwnat_tuple,			0x12,  	0,  NULL},
	{"hash",		do_hwnat_hash,			0x12,  	0,  NULL},
	{"flow",		do_hwnat_flow,			0x12,  	0,  NULL},
	{"macaddr",		do_hwnat_mac_addr,		0x12,  	0,  NULL},
	{"const",		do_hwnat_const,			0x12,  	0,  NULL},
	{"mod_macro",	do_hwnat_mod_macro,		0x12,  	0,  NULL},
	{"mod_vec",		do_hwnat_mod_vec,		0x12,  	0,  NULL},
	{"mod_op",		do_hwnat_mod_op,		0x12,  	0,  NULL},
	{"qos",			do_hwnat_qos,			0x12,  	0,  NULL},
	{"nfe",			do_hwnat_nfe,			0x12,  	0,  NULL},
	{"rxport",		do_hwnat_rxport,		0x12,  	0,  NULL},
	{NULL,			NULL,					0x10,	0,	NULL},
};

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#ifdef HWNAT_SW_MODEL

typedef int (*hwnat_hook_t) (struct sk_buff *skb, struct net_device *dev);

extern void hwnat_sw_bind(hwnat_hook_t hook);

static void dump_skb(char *title, struct sk_buff *skb, int len) 
{
	char tmp[80];
	char *p = skb->data;
	char *t = tmp;
	int i, n = 0;
	int dump_len = (len != 0) ? min(len, skb->len) : skb-len;

	printk("%s dev=%s skb=%08lx data=%08lx len=%d\n", 
				title, skb->dev->name, (uint32) skb, (uint32) skb->data, skb->len);
	for (i = 0; i < dump_len; i++) {
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

static inline struct sk_buff *hwnat_alloc_skb2k(void)
{
	return skbmgr_dev_alloc_skb2k();
}

int hwnat_pkt_proc(struct sk_buff *skb, struct net_device *dev)
{
	static uint32 out_len;
	static uint8 *module_name[] = {"PCE", "TUE", "HSE", "FTE", "MDE"};

	unsigned long flags;
	procStatus static_status;
	procStatus *status = &static_status;
	procStatus *nat_status;
	int inport = 0;
	int i;
	char tmp[80];
	char *t = tmp;
	struct sk_buff *out_skb;
	int prev_port;
	struct net_device *out_dev;
	uint8 out_dev_name[16];

	if (hwnat_active == 0) 
		return 0;

	if (strcmp(dev->name, "eth0") == 0)
		inport = 0;
	else if ((strcmp(dev->name, "eth1") == 0) || (strcmp(dev->name, "nas0") == 0))
		inport = 1;
	else {
		printk("err: unknown port\n");
		return 0;
	}

	hwnatPortStat[inport].inPkts++;

	out_skb = hwnat_alloc_skb2k();
	if (out_skb == NULL)
		return 0;

	/* do software HWNAT process */
	spin_lock_irqsave(&hwnat_lock, flags);
	nat_status = run_nat_process(skb->data, skb->len, out_skb->data, &out_len, inport);
	memcpy(status, nat_status, sizeof(procStatus));
	spin_unlock_irqrestore(&hwnat_lock, flags);

	skb_put(out_skb, out_len);
	
	/* flow miss */
	if (status->process_stop == 1)
		hwnatPortStat[inport].inMissPkts++;
	else
		hwnatPortStat[inport].inHitPkts++;

	/* start to dump debug message */
	if (hwnat_debug) {
		if ((hwnat_debug & 0x80000000) && (status->process_stop == 1))
			goto debug_exit;
		if (hwnat_debug & 0x3f) 
			printk("\n");
	}
	if (hwnat_debug & 0x4) {
		dump_skb("IN ", skb, 64);
	}
	if (hwnat_debug & 0x8) {
		dump_skb("IN ", skb, 0);
	}

	if (hwnat_debug & 0x10) {
		dump_skb("OUT", out_skb, 64);
	}
	if (hwnat_debug & 0x20) {
		dump_skb("OUT", out_skb, 0);
	}

	if (hwnat_debug & 0x1) {
		printk("status=%d module=%s itf=%d cmpr=%d policy=%d tuple=%d hash=%d tx_portmap=%04x\n", 
				status->process_stop, module_name[status->final_module], status->interface_final_entry,
				status->comparator_exit_code, status->policy_final_entry, status->tuple_final_entry,
				status->hash_final_entry, status->flow_outPortBitMap);
	}
	if (hwnat_debug & 0x2) {
		printk("cmpr entry status:\n");
		for (i = 0; i < COMPARATOR_TBL_SIZE; i++) {
			if (status->comparator_entry_status[i] & 0x80000000)
				t += sprintf(t, "%3d ", status->comparator_entry_status[i] & 0x7fffffff);
			else
				break;
			if ((i & 0x0f) == 0x0f) {
				printk("%s\n", tmp);
				t = tmp;
			}
		}
		if (i & 0x0f)
			printk("%s\n", tmp);

		printk("flow hit status:\n");
		t = tmp;
		for (i = 0; i < FLOW_TBL_SIZE; i++) {
			if (status->flow_hit_status[i] & 0x80000000)
				t += sprintf(t, "%3d ", status->flow_hit_status[i] & 0xff);
			else
				break;
			if ((i & 0x0f) == 0x0f) {
				printk("%s\n", tmp);
				t = tmp;
			}
		}
		if (i & 0x0f)
			printk("%s\n", tmp);

		printk("mde entry status:\n");
		t = tmp;
		for (i = 0; i < MOD_OP_TBL_SIZE; i++) {
			if (status->mde_entry_status[i] & 0x80000000)
				t += sprintf(t, "%d/%d/%d ", 
						status->mde_entry_status[i]>>16 & 0xff,
						status->mde_entry_status[i]>>8 & 0xff, 
						status->mde_entry_status[i] & 0xff);
			else
				break;
			if ((i & 0x0f) == 0x0f) {
				printk("%s\n", tmp);
				t = tmp;
			}
		}
		if (i & 0x0f)
			printk("%s\n", tmp);

	}
debug_exit:

	/* flow hit */
	if ((status->process_stop == 0) && (status->final_module == 4)) {

		/* filter out this packet */
		if (status->flow_outPortBitMap == 0) {
			dev_kfree_skb_any(skb);
			dev_kfree_skb_any(out_skb);
			return 1;
		}

		prev_port = -1;

		for (i = 0; i < INTERFACE_TBL_SIZE; i++) {
			if (status->flow_outPortBitMap & (1<<i)) {
				hwnatPortStat[i].txPortPkts++;

				/* tx port is not cpu port */
				if (i != 15) {

					if (prev_port != -1) {
						struct sk_buff *skb2;

						if ((skb2 = skb_clone(out_skb, GFP_ATOMIC)) == NULL) {
							printk("err: alloc skb fail\n");
							break;
						}

						if (i == 1)
							sprintf(out_dev_name, "nas0");
						else
							sprintf(out_dev_name, "eth%d", i);
						out_dev = dev_get_by_name(out_dev_name);
						skb2->dev = out_dev;
						dev_queue_xmit(skb2);
					}
					
					prev_port = i;
				}
			}
		}

		if (prev_port != -1) {
			struct net_device *out_dev;

			if (prev_port == 1)
				sprintf(out_dev_name, "nas0");
			else
				sprintf(out_dev_name, "eth%d", prev_port);
			out_dev = dev_get_by_name(out_dev_name);
			out_skb->dev = out_dev;
			dev_queue_xmit(out_skb);
		}

		dev_kfree_skb_any(skb);
		return 1;
	}

	dev_kfree_skb_any(out_skb);

	return 0;
}

#endif

static int do_hwnat(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_cmds, argc, argv, p);
}

int do_hwnat_active(int argc, char *argv[], void *p)
{
	if ((uint32) simple_strtoul(argv[1], NULL, 16))
		hwnat_active = 1;
	else
		hwnat_active = 0;
	return 0;
}

int do_hwnat_debug(int argc, char *argv[], void *p)
{
	hwnat_debug = (uint32) simple_strtoul(argv[1], NULL, 16);
	return 0;
}

int do_hwnat_hitcnt(int argc, char *argv[], void *p)
{
	hwnat_hitcnt = (uint32) simple_strtoul(argv[1], NULL, 10);
	return 0;
}

int do_hwnat_mc_hitcnt(int argc, char *argv[], void *p)
{
	hwnat_multicast_hitcnt = (uint32) simple_strtoul(argv[1], NULL, 10);
	return 0;
}

int do_hwnat_icmp(int argc, char *argv[], void *p)
{
	hwnat_icmp_on = simple_strtoul(argv[1], NULL, 16) ? 1 : 0;
	return 0;
}

int do_hwnat_udp(int argc, char *argv[], void *p)
{
	hwnat_udp_on = simple_strtoul(argv[1], NULL, 16) ? 1 : 0;
	return 0;
}

int do_hwnat_ipv6(int argc, char *argv[], void *p)
{
	hwnat_ipv6_on = simple_strtoul(argv[1], NULL, 16) ? 1 : 0;
	return 0;
}

int do_hwnat_multicast(int argc, char *argv[], void *p)
{
	hwnat_multicast_on = simple_strtoul(argv[1], NULL, 16) ? 1 : 0;
	return 0;
}

int do_hwnat_ipv6_multicast(int argc, char *argv[], void *p)
{
	hwnat_ipv6_multicast_on = simple_strtoul(argv[1], NULL, 16) ? 1 : 0;
	return 0;
}

#ifdef HWNAT_SW_MODEL
int do_hwnat_stat(int argc, char *argv[], void *p)
{
	int i;
	int port;

	if (argc >= 2) {
		port = simple_strtoul(argv[1], NULL, 10);

		printk("[Port%d]\n", port);
		printk("inPkts              = 0x%08lx, ", hwnatPortStat[port].inPkts);
		printk("inHitPkts           = 0x%08lx\n", hwnatPortStat[port].inHitPkts);
		printk("inMissPkts          = 0x%08lx, ", hwnatPortStat[port].inMissPkts);
		printk("txPortPkts          = 0x%08lx\n", hwnatPortStat[port].txPortPkts);
	} else {
		for (i = 0; i < INTERFACE_TBL_SIZE; i++) {
			printk("[Port%d]\n", i);
			printk("inPkts              = 0x%08lx, ", hwnatPortStat[i].inPkts);
			printk("inHitPkts           = 0x%08lx\n", hwnatPortStat[i].inHitPkts);
			printk("inMissPkts          = 0x%08lx, ", hwnatPortStat[i].inMissPkts);
			printk("txPortPkts          = 0x%08lx\n", hwnatPortStat[i].txPortPkts);
		}
	}
	return 0;
}
#endif

static void hwnat_start(void)
{
	int ret;

	printk("%s\n", MODULE_VERSION_HWNAT);

	//printk("reset \n");
	/* software reset hwnat */
	hwnat_write_reg(CR_HWNAT_CTL0, CTL0_SW_RST);
	mdelay(100);

	//printk("cmd init \n");
	hwnat_cmd_init();
	//printk("itf init \n");
	hwnat_itf_init();
	//printk("gpr init \n");
	hwnat_gpr_init();
	//printk("pce init \n");
	hwnat_pce_init();
	//printk("tue init \n");
	hwnat_tue_init();
	//printk("fte init \n");
	hwnat_fte_init();
	//printk("mde init \n");
	hwnat_mde_init();
	//printk("qos init \n");
	hwnat_qos_init();
	//printk("nfe init \n");
	hwnat_nfe_init();

	//printk("enable hwnat \n");
	/* set ipv4udp, tc2206 inport and vlan index */
	hwnat_write_reg(CR_HWNAT_CTL1, (CTL1_IPV4UDP_INDEX<<CTL1_IPV4UDP_SHIFT) | 
			(CTL1_PKTDATA1_INDEX<<CTL1_PKTDATA1_SHIFT) | (CTL1_PKTDATA0_INDEX<<CTL1_PKTDATA0_SHIFT) |
			(CTL1_VLAN_INDEX<<CTL1_VLAN_SHIFT));
	/* set tc2206 inport mask */
	hwnat_write_reg(CR_HWNAT_QOSPKTDATA, (TC2206_INPORT_MASK<<PKTDATA1_MASK_SHIFT) | 
			(TC2206_INPORT_MASK<<PKTDATA0_MASK_SHIFT));
	/* enable hwnat */
	hwnat_write_reg(CR_HWNAT_CTL0, (CTL0_IPV4CHKSUM_INDEX<<CTL0_IPV4CHKSUM_IDX_SHIFT) | 
			(CTL0_FLOWDO_INDEX<<CTL0_FLOWDO_SHIFT) | 
			(CTL0_IPV6UDP_INDEX<<CTL0_IPV6UDP_SHIFT) |
			CTL0_AGE_INT | CTL0_HWNAT_EN);

#ifdef HWNAT_SW_MODEL
	memset(hwnatPortStat, 0, sizeof(hwnatPortStat));
	hwnat_sw_bind(hwnat_pkt_proc);
#endif

	//printk("pktflow init \n");
	ret = hwnat_pktflow_init();
	if (ret < 0) {
		printk("can't alloc pktflow table.\n");
		return;
	}

	//printk("rxport init \n");
	hwnat_rxport_init();
}

static void hwnat_stop(void)
{
	hwnat_rxport_exit();

	hwnat_pktflow_exit();

#ifdef HWNAT_SW_MODEL
	hwnat_sw_bind(NULL);
#endif

	/* software reset hwnat */
	hwnat_write_reg(CR_HWNAT_CTL0, CTL0_SW_RST);
	/* disable hwnat */
	hwnat_write_reg(CR_HWNAT_CTL0, hwnat_read_reg(CR_HWNAT_CTL0) & ~CTL0_HWNAT_EN);

	hwnat_cmd_exit();
	hwnat_itf_exit();
	hwnat_gpr_exit();
	hwnat_pce_exit();
	hwnat_tue_exit();
	hwnat_fte_exit();
	hwnat_mde_exit();
	hwnat_qos_exit();
	hwnat_nfe_exit();
}

static void hwnat_cmd_init(void)
{
	cmds_t hwnat_cmd;

	/* init hwnat root ci-cmd */
	hwnat_cmd.name = "hwnat";
	hwnat_cmd.func = do_hwnat;
	hwnat_cmd.flags = 0x12;
	hwnat_cmd.argcmin = 0;
	hwnat_cmd.argc_errmsg = NULL;

	/* register hwnat ci-cmd */
	cmd_register(&hwnat_cmd);
}

static void hwnat_cmd_exit(void)
{
	cmd_unregister("hwnat");
}

static int __init hwnat_init(void)
{
	hwnat_start();
  	return 0;
}

static void __exit hwnat_exit(void)
{
	hwnat_stop();
}

module_init(hwnat_init);
module_exit(hwnat_exit);
MODULE_LICENSE("GPL");
