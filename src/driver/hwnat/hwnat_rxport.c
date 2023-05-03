/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_rxport.c#1 $
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
** $Log: hwnat_rxport.c,v $
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
#include <linux/inet.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <asm/unaligned.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/cmdparse.h>

#include "hwnat.h"
#include "hwnat_reg.h"
#include "hwnat_rxport.h"

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                            M A C R O S
*************************************************************************
*/
#define CR_GMAC0_HWNAT    			0xBFB5019C
#define CR_GMAC1_HWNAT    			0xBFB5819C

#define CR_PTM_B0_PFSR     			0xBFB68010
#define CR_PTM_B1_PFSR     			0xBFB68210

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

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

void hwnat_rxport_init(void)
{
	/* enable hwnat on GMAC0 */
	VPint(CR_GMAC0_HWNAT) |= (1<<12);
	/* enable hwnat on GMAC1 */
	VPint(CR_GMAC1_HWNAT) |= (1<<12);

	/* enable hwnat on PTM B0 and tx padding */
	VPint(CR_PTM_B0_PFSR) |= (1<<24) | (1<<28);
	/* enable hwnat on PTM B1 and tx padding */
	VPint(CR_PTM_B1_PFSR) |= (1<<24) | (1<<28);
}

void hwnat_rxport_exit(void)
{
	/* disable hwnat on GMAC0 */
	VPint(CR_GMAC0_HWNAT) &= ~(1<<12);
	/* disable hwnat on GMAC1 */
	VPint(CR_GMAC1_HWNAT) &= ~(1<<12);

	/* disable hwnat on PTM B0 */
	VPint(CR_PTM_B0_PFSR) &= ~(1<<24);
	/* disable hwnat on PTM B1 */
	VPint(CR_PTM_B1_PFSR) &= ~(1<<24);
}

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/

static int do_hwnat_rxport_enable(int argc, char *argv[], void *p);
static int do_hwnat_rxport_disable(int argc, char *argv[], void *p);
static int do_hwnat_rxport_dump(int argc, char *argv[], void *p);

static const cmds_t hwnat_rxport_cmds[] = {
	{"enable",		do_hwnat_rxport_enable,			0x02,  	0,  NULL},
	{"disable",		do_hwnat_rxport_disable,		0x02,  	0,  NULL},
	{"dump",		do_hwnat_rxport_dump,			0x02,  	0,  NULL},
	{NULL,			NULL,							0x10,	0,	NULL},
};

int do_hwnat_rxport(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_rxport_cmds, argc, argv, p);
}

int do_hwnat_rxport_enable(int argc, char *argv[], void *p)
{
	int port = -1;

	if (argc >= 2) {
		port = simple_strtoul(argv[1], NULL, 10);
	}

	switch (port) {
		case 0:
			/* enable hwnat on GMAC0 */
			VPint(CR_GMAC0_HWNAT) |= (1<<12);
			break;
		case 1:
			/* enable hwnat on GMAC1 */
			VPint(CR_GMAC1_HWNAT) |= (1<<12);
			break;
		case 2:
			/* enable hwnat on PTM B0 and tx padding */
			VPint(CR_PTM_B0_PFSR) |= (1<<24) | (1<<28);
			break;
		case 3:
			/* enable hwnat on PTM B1 and tx padding */
			VPint(CR_PTM_B1_PFSR) |= (1<<24) | (1<<28);
			break;
		default:
			/* enable hwnat on GMAC0 */
			VPint(CR_GMAC0_HWNAT) |= (1<<12);
			/* enable hwnat on GMAC1 */
			VPint(CR_GMAC1_HWNAT) |= (1<<12);

			/* enable hwnat on PTM B0 and tx padding */
			VPint(CR_PTM_B0_PFSR) |= (1<<24) | (1<<28);
			/* enable hwnat on PTM B1 and tx padding */
			VPint(CR_PTM_B1_PFSR) |= (1<<24) | (1<<28);
			break;
	}


	return 0;
}

int do_hwnat_rxport_disable(int argc, char *argv[], void *p)
{
	int port = -1;

	if (argc >= 2) {
		port = simple_strtoul(argv[1], NULL, 10);
	}

	switch (port) {
		case 0:
			/* disable hwnat on GMAC0 */
			VPint(CR_GMAC0_HWNAT) &= ~(1<<12);
			break;
		case 1:
			/* disable hwnat on GMAC1 */
			VPint(CR_GMAC1_HWNAT) &= ~(1<<12);
			break;
		case 2:
			/* disable hwnat on PTM B0 */
			VPint(CR_PTM_B0_PFSR) &= ~(1<<24);
			break;
		case 3:
			/* disable hwnat on PTM B1 */
			VPint(CR_PTM_B1_PFSR) &= ~(1<<24);
			break;
		default:
			/* disable hwnat on GMAC0 */
			VPint(CR_GMAC0_HWNAT) &= ~(1<<12);
			/* disable hwnat on GMAC1 */
			VPint(CR_GMAC1_HWNAT) &= ~(1<<12);

			/* disable hwnat on PTM B0 */
			VPint(CR_PTM_B0_PFSR) &= ~(1<<24);
			/* disable hwnat on PTM B1 */
			VPint(CR_PTM_B1_PFSR) &= ~(1<<24);
			break;
	}

	return 0;
}

int do_hwnat_rxport_dump(int argc, char *argv[], void *p)
{
	printk("gmac0		= %d\n", (VPint(CR_GMAC0_HWNAT) & (1<<12)) ? 1 : 0);
	printk("gmac1		= %d\n", (VPint(CR_GMAC1_HWNAT) & (1<<12)) ? 1 : 0);
	printk("ptm b0		= %d\n", (VPint(CR_PTM_B0_PFSR) & (1<<24)) ? 1 : 0);
	printk("ptm b1		= %d\n", (VPint(CR_PTM_B1_PFSR) & (1<<24)) ? 1 : 0);

	return 0;
}
