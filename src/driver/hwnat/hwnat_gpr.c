/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_gpr.c#1 $
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
** $Log: hwnat_gpr.c,v $
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
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/mii.h>
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/cmdparse.h>

#include "hwnat.h"
#include "hwnat_reg.h"

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

typedef struct {
	uint8 op;
	uint16 mask;
	uint16 data;
} gpr_t;

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static void hwnat_gpr_tbl_write(uint16 entry, gpr_t *gpr);
static void hwnat_gpr_tbl_read(uint16 entry, gpr_t *gpr0, gpr_t *gpr1);

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

gpr_t gpr_tbl[GPR_TBL_SIZE] = {
/*	op	mask	*/
/*  0 */ { 1, 0xffff, 0x0 },	/* IPv4 total length */	
/*  1 */ { 1, 0xff00, 0x0 },	/* IPv4 TTL */	
/*  2 */ { 1, 0xffff, 0x0 },	/* IPv6 payload length */
/*  3 */ { 1, 0x00ff, 0x0 },	/* IPv6 HL */
/*  4 */ { 1, 0xffff, 0x0 },	
/*  5 */ { 1, 0xffff, 0x0 },	
/*  6 */ { 1, 0xffff, 0x0 },	
/*  7 */ { 1, 0xffff, 0x0 },	
/*  8 */ { 1, 0xffff, 0x0 },	
/*  9 */ { 1, 0xffff, 0x0 },	
/* 10 */ { 1, 0xffff, 0x0 },
/* 11 */ { 1, 0xffff, 0x0 },
/* 12 */ { 1, 0xffff, 0x0 },	
/* 13 */ { 1, 0xffff, 0x0 },	
/* 14 */ { 1, 0xffff, 0x0 },	
/* 15 */ { 1, 0xffff, 0x0 },	
/* 16 */ { 1, 0xffff, 0x0 },	
/* 17 */ { 1, 0xffff, 0x0 },	
/* 18 */ { 1, 0xffff, 0x0 },
/* 19 */ { 1, 0xffff, 0x0 },	
/* 20 */ { 1, 0xffff, 0x0 },	
/* 21 */ { 1, 0xffff, 0x0 },	
/* 22 */ { 1, 0xffff, 0x0 },	
/* 23 */ { 1, 0xffff, 0x0 },	
/* 24 */ { 1, 0xffff, 0x0 },	
/* 25 */ { 1, 0xffff, 0x0 },	
/* 26 */ { 1, 0xffff, 0x0 },
/* 27 */ { 1, 0xffff, 0x0 },
/* 28 */ { 1, 0xffff, 0x0 },	
/* 29 */ { 1, 0xffff, 0x0 },	
/* 30 */ { 1, 0xffff, 0x0 },	
/* 31 */ { 1, 0xffff, 0x0 },	
};

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

void hwnat_gpr_init(void)
{
	uint16 entry;

	for (entry = 0; entry < GPR_TBL_SIZE; entry++)
		hwnat_gpr_tbl_write(entry, &gpr_tbl[entry]);
}

void hwnat_gpr_exit(void)
{
}

void hwnat_gpr_tbl_write(uint16 entry, gpr_t *gpr)
{
	uint32 table_hold[GTH_SIZE];

	/* select gpr set0 */
	hwnat_write_reg(CR_HWNAT_CTL1, hwnat_read_reg(CR_HWNAT_CTL1) | CTL1_GPR_SET);

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = (gpr->data&0xffff) | ((gpr->mask<<16)&0xffff0000);
	table_hold[6] = gpr->op&0x1;
	hwnat_write_tbl(GPR_TBL_ID, entry, table_hold, GTH_SIZE);

	/* select gpr set1 */
	hwnat_write_reg(CR_HWNAT_CTL1, hwnat_read_reg(CR_HWNAT_CTL1) & ~CTL1_GPR_SET);

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = (gpr->data&0xffff) | ((gpr->mask<<16)&0xffff0000);
	table_hold[6] = gpr->op&0x1;
	hwnat_write_tbl(GPR_TBL_ID, entry, table_hold, GTH_SIZE);

}

void hwnat_gpr_tbl_read(uint16 entry, gpr_t *gpr0, gpr_t *gpr1)
{
	uint32 table_hold[GTH_SIZE];

	/* select gpr set0 */
	hwnat_write_reg(CR_HWNAT_CTL1, hwnat_read_reg(CR_HWNAT_CTL1) | CTL1_GPR_SET);

	hwnat_read_tbl(GPR_TBL_ID, entry, table_hold, GTH_SIZE);
	gpr0->data = table_hold[7] & 0xffff;
	gpr0->mask = (table_hold[7] & 0xffff0000) >> 16;
	gpr0->op = table_hold[6] & 0x1;

	/* select gpr set1 */
	hwnat_write_reg(CR_HWNAT_CTL1, hwnat_read_reg(CR_HWNAT_CTL1) & ~CTL1_GPR_SET);

	hwnat_read_tbl(GPR_TBL_ID, entry, table_hold, GTH_SIZE);
	gpr1->data = table_hold[7] & 0xffff;
	gpr1->mask = (table_hold[7] & 0xffff0000) >> 16;
	gpr1->op = table_hold[6] & 0x1;

}

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/
static int do_hwnat_gpr_rd(int argc, char *argv[], void *p);
static int do_hwnat_gpr_wr(int argc, char *argv[], void *p);

static int do_hwnat_gpr_tbl(int argc, char *argv[], void *p);
static int do_hwnat_gpr_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_gpr_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_gpr_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_gpr_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_gpr_cmds[] = {
	{"rd",			do_hwnat_gpr_rd,		0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_gpr_wr,		0x02,  	4,  "<entry> <op> <mask> <data>"},
	{"tbl",			do_hwnat_gpr_tbl,		0x12,  	0,  NULL},
	{NULL,			NULL,					0x10,	0,	NULL},
};

static const cmds_t hwnat_gpr_tbl_cmds[] = {
	{"dump",		do_hwnat_gpr_tbl_dump,	0x02,  	0,  NULL},
	{"raw",			do_hwnat_gpr_tbl_raw,	0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_gpr_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_gpr_tbl_comp,	0x02,  	0,  NULL},
	{NULL,			NULL,					0x10,	0,	NULL},
};

int do_hwnat_gpr(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_gpr_cmds, argc, argv, p);
}

int do_hwnat_gpr_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	gpr_t gpr0;
	gpr_t gpr1;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_gpr_tbl_read(entry, &gpr0, &gpr1);

	printk("/*%02d*/", entry);
	printk(" %d %04x %04x", gpr0.op, gpr0.mask, gpr0.data);
	printk(" %d %04x %04x\n", gpr1.op, gpr1.mask, gpr1.data);

	return 0;
}

int do_hwnat_gpr_wr(int argc, char *argv[], void *p)
{
	uint16 entry;
	gpr_t *gpr;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	gpr = &gpr_tbl[entry];

	gpr->op = (uint8) simple_strtoul(argv[2], NULL, 10);
	gpr->mask = (uint16) simple_strtoul(argv[3], NULL, 16);
	gpr->data = (uint16) simple_strtoul(argv[4], NULL, 16);

	hwnat_gpr_tbl_write(entry, gpr);

	return 0;
}

int do_hwnat_gpr_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_gpr_tbl_cmds, argc, argv, p);
}

int do_hwnat_gpr_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;

	printk("op mask data\n");
	for (entry = 0; entry < GPR_TBL_SIZE; entry++) {
		printk("/*%02d*/", entry);
		printk(" %d %04x %04x\n", gpr_tbl[entry].op, gpr_tbl[entry].mask, gpr_tbl[entry].data);
	}

	return 0;
}

int do_hwnat_gpr_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	gpr_t *gpr;
	uint16 entry, i;

	printk("gpr\n");
	for (entry = 0; entry < GPR_TBL_SIZE; entry++) {
		gpr = &gpr_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (gpr->data&0xffff) | ((gpr->mask<<16)&0xffff0000);
		table_hold[6] = gpr->op&0x1;

		printk("/*%02d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}

	return 0;
}

int do_hwnat_gpr_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	uint16 entry, i;

	printk("gpr\n");
	for (entry = 0; entry < GPR_TBL_SIZE; entry++) {
		hwnat_read_tbl(GPR_TBL_ID, entry, table_hold, GTH_SIZE);

		printk("/*%02d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}

	return 0;
}

int do_hwnat_gpr_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[GTH_SIZE];
	uint32 table_hold[GTH_SIZE];
	gpr_t *gpr;
	uint16 entry, i;

	for (entry = 0; entry < GPR_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(GPR_TBL_ID, entry, hw_table_hold, GTH_SIZE);

		gpr = &gpr_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (gpr->data&0xffff) | ((gpr->mask<<16)&0xffff0000);
		table_hold[6] = gpr->op&0x1;

		if (memcmp(hw_table_hold, table_hold, sizeof(hw_table_hold)) != 0) {
			printk("error /*%02d*/\n", entry);
			for (i = 0; i < GTH_SIZE; i++)
				printk(" %08lx", hw_table_hold[i]);
			printk("\n");
			for (i = 0; i < GTH_SIZE; i++)
				printk(" %08lx", table_hold[i]);
			printk("\n");
		}
	}

	return 0;
}

