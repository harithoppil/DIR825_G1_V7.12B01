/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_nfe.c#1 $
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
** $Log: hwnat_nfe.c,v $
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
#include <linux/inet.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <linux/pktflow.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <asm/unaligned.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/cmdparse.h>

#include "hwnat.h"
#include "hwnat_reg.h"
#include "hwnat_nfe.h"

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                            M A C R O S
*************************************************************************
*/

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

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
DEFINE_SPINLOCK(nfe_lock);

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

void hwnat_nfe_init(void)
{
	unsigned long flags;

	spin_lock_irqsave(&nfe_lock, flags);
	/* disable flow control, set bp mode=1 */
	VPint(CR_NFE_MISC) &= ~(MISC_MAC_RX_XFC | MISC_BP_MODE);
	VPint(CR_NFE_MISC) |= (1<<MISC_BP_MODE_SHIFT);
	spin_unlock_irqrestore(&nfe_lock, flags);
}

void hwnat_nfe_exit(void)
{
}

void hwnat_nfe_qos_set(uint8 port, uint8 policy, uint16 w0, uint16 w1, uint16 w2, uint16 w3)
{
	unsigned long flags;

	spin_lock_irqsave(&nfe_lock, flags);
	if (policy == QUEUE_SCH_MODE_WRR) {
		VPint(CR_NFE_PERWRRCR(port)) = w0<<WRR_0_SHIFT | w1<<WRR_1_SHIFT | w2<<WRR_2_SHIFT | w3<<WRR_3_SHIFT;
	} else if (policy == QUEUE_SCH_MODE_RC) {
		VPint(CR_NFE_PERCR0(port)) = w0<<RC_0_SHIFT | w1<<RC_1_SHIFT;
		VPint(CR_NFE_PERCR1(port)) = w2<<RC_2_SHIFT | w3<<RC_3_SHIFT;
	}

	VPint(CR_NFE_EPQSMSCR) = (VPint(CR_NFE_EPQSMSCR) & ~(0x3<<(port<<2))) | (policy<<(port<<2));
	spin_unlock_irqrestore(&nfe_lock, flags);
}

void hwnat_nfe_qos_get(uint8 port, uint8 *policy, uint16 *w0, uint16 *w1, uint16 *w2, uint16 *w3)
{
	unsigned long flags;

	spin_lock_irqsave(&nfe_lock, flags);
	*policy = (VPint(CR_NFE_EPQSMSCR) & (0x3<<(port<<2))) >> (port<<2);
	if (*policy == QUEUE_SCH_MODE_WRR) {
		*w0 = (VPint(CR_NFE_PERWRRCR(port))&WRR_0)>>WRR_0_SHIFT;
		*w1 = (VPint(CR_NFE_PERWRRCR(port))&WRR_1)>>WRR_1_SHIFT;
		*w2 = (VPint(CR_NFE_PERWRRCR(port))&WRR_2)>>WRR_2_SHIFT;
		*w3 = (VPint(CR_NFE_PERWRRCR(port))&WRR_3)>>WRR_3_SHIFT;
	} else if (*policy == QUEUE_SCH_MODE_RC) {
		*w0 = (VPint(CR_NFE_PERCR0(port))&RC_0)>>RC_0_SHIFT;
		*w1 = (VPint(CR_NFE_PERCR0(port))&RC_1)>>RC_1_SHIFT;
		*w2 = (VPint(CR_NFE_PERCR1(port))&RC_2)>>RC_2_SHIFT;
		*w3 = (VPint(CR_NFE_PERCR1(port))&RC_3)>>RC_3_SHIFT;
	}
	spin_unlock_irqrestore(&nfe_lock, flags);
}

void hwnat_nfe_get_stats(struct pktflow_stats *stats, int port)
{
	unsigned long flags;

	spin_lock_irqsave(&nfe_lock, flags);

	VPint(CR_NFE_MIBSCR) = port<<MIBSCR_PORTID_SHIFT | MIBSCR_READ;

	stats->rx_packets = VPint(CR_NFE_MIBRXPKT);
	stats->rx_bytes = VPint(CR_NFE_MIBRXBYTE);
	stats->rx_dropped = VPint(CR_NFE_MIBRXDROP);
	stats->tx_packets_q0 = VPint(CR_NFE_MIBTXPKTQ0);
	stats->tx_packets_q1 = VPint(CR_NFE_MIBTXPKTQ1);
	stats->tx_packets_q2 = VPint(CR_NFE_MIBTXPKTQ2);
	stats->tx_packets_q3 = VPint(CR_NFE_MIBTXPKTQ3);
	stats->tx_packets = stats->tx_packets_q0 + stats->tx_packets_q1 + 
							stats->tx_packets_q2 + stats->tx_packets_q3;
	stats->tx_bytes = VPint(CR_NFE_MIBTXBYTE);
	stats->tx_dropped = 0;

	spin_unlock_irqrestore(&nfe_lock, flags);
}

void hwnat_nfe_clear_stats(int port)
{
	unsigned long flags;

	spin_lock_irqsave(&nfe_lock, flags);
	VPint(CR_NFE_MIBSCR) = port<<MIBSCR_PORTID_SHIFT | MIBSCR_CLEAR;
	spin_unlock_irqrestore(&nfe_lock, flags);
}

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/
static int do_hwnat_nfe_stats(int argc, char *argv[], void *p);
static int do_hwnat_nfe_fb(int argc, char *argv[], void *p);

static const cmds_t hwnat_nfe_cmds[] = {
	{"stats",		do_hwnat_nfe_stats,			0x02,  	0,  NULL},
	{"fb",			do_hwnat_nfe_fb,			0x02,  	0,  NULL},
};

int do_hwnat_nfe(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_nfe_cmds, argc, argv, p);
}

int do_hwnat_nfe_stats(int argc, char *argv[], void *p)
{
	int port;
	uint8 dump_one = 0;
	unsigned long flags;

	if (argc >= 2) {
		if (strcmp(argv[1], "clear") == 0) {
			for (port = 0; port < HWNAT_MAX_PORT_NO; port++) {
				spin_lock_irqsave(&nfe_lock, flags);
				VPint(CR_NFE_MIBSCR) = port<<MIBSCR_PORTID_SHIFT | MIBSCR_CLEAR;
				spin_unlock_irqrestore(&nfe_lock, flags);
			}
		} else {
			port = simple_strtoul(argv[1], NULL, 10);
			dump_one = 1;
			goto stats_dump;
		}
	}

	for (port = 0; port < HWNAT_MAX_PORT_NO; port++) {
stats_dump:
		spin_lock_irqsave(&nfe_lock, flags);

		VPint(CR_NFE_MIBSCR) = port<<MIBSCR_PORTID_SHIFT | MIBSCR_READ;

		printk("[Port%d]\n", port);
		printk("inPkts              = 0x%08lx, ", VPint(CR_NFE_MIBRXPKT));
		printk("inBytes             = 0x%08lx\n", VPint(CR_NFE_MIBRXBYTE));
		printk("inDrops             = 0x%08lx\n", VPint(CR_NFE_MIBRXDROP));
		printk("txPkts (Q0)         = 0x%08lx, ", VPint(CR_NFE_MIBTXPKTQ0));
		printk("txPkts (Q1)         = 0x%08lx\n", VPint(CR_NFE_MIBTXPKTQ1));
		printk("txPkts (Q2)         = 0x%08lx, ", VPint(CR_NFE_MIBTXPKTQ2));
		printk("txPkts (Q3)         = 0x%08lx\n", VPint(CR_NFE_MIBTXPKTQ3));
		printk("txBytes             = 0x%08lx\n", VPint(CR_NFE_MIBTXBYTE));

		spin_unlock_irqrestore(&nfe_lock, flags);

		if (dump_one)
			break;
	}

	return 0;
}

int do_hwnat_nfe_fb(int argc, char *argv[], void *p)
{
	uint8 fb_cnt;

	VPint(CR_NFE_FBCMR) = RD_FB_CNT;
	fb_cnt = VPint(CR_NFE_FBCMR) & FB_CNT;

	printk("fb=0x%02x\n", fb_cnt);

	return 0;
}

