/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_itf.c#1 $
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
** $Log: hwnat_itf.c,v $
** Revision 1.2  2011/06/03 02:41:44  lino
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
	uint16 comparator_tbl_index;
	uint16 l2h;
	uint16 mtu;
} interface_t;

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static void hwnat_itf_tbl_write(uint16 entry, interface_t *itf);
static void hwnat_itf_tbl_read(uint16 entry, interface_t *itf);

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

interface_t interface_tbl[INTERFACE_TBL_SIZE] = {
/*	CTblIdx	L2H	MTU	*/
/*  0 */ { 15,  0, 1500 },	/*GMAC1	*/
/*  1 */ { 15,  0, 1500 },	/*GMAC2	*/
/*  2 */ { 15,  0, 1500 },	/*PTM0 (bearer 0)	*/
/*  3 */ { 15,  0, 1500 },	/*PTM1 (bearer 1)	*/
/*  4 */ {  0, 10, 1500 },	/*PVC0	*/
/*  5 */ {  8,  2, 1500 },	/*PVC1	*/
/*  6 */ {  0,  8, 1500 },	/*PVC2	*/
/*  7 */ {  9,  0, 1500 },	/*PVC3	*/
/*  8 */ { 11,  6, 1500 },	/*PVC4	*/
/*  9 */ { 13,  2, 1500 },	/*PVC5	*/
/* 10 */ { 13,  2, 1500 },	/*PVC6	*/
/* 11 */ { 13,  2, 1500 },	/*PVC7	*/
/* 12 */ { 15,  0, 1500 },	/*USB	*/
/* 13 */ { 15,  0, 1500 },	/*WiFi	*/
/* 14 */ { 15,  0, 1500 },	/*	*/
/* 15 */ { 15,  0, 1500 },	/*CPU	*/
};

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

void hwnat_itf_init(void)
{
	uint16 entry;

	for (entry = 0; entry < INTERFACE_TBL_SIZE; entry++)
		hwnat_itf_tbl_write(entry, &interface_tbl[entry]);

	hwnat_write_reg(CR_HWNAT_RXPORT01, 
			(interface_tbl[0].comparator_tbl_index<<RXPORT01_0_CMPR_SHIFT) |
			(interface_tbl[0].l2h<<RXPORT01_0_L2H_SHIFT) |
			(interface_tbl[1].comparator_tbl_index<<RXPORT01_1_CMPR_SHIFT) |
			(interface_tbl[1].l2h<<RXPORT01_1_L2H_SHIFT));

	hwnat_write_reg(CR_HWNAT_RXPORT23, 
			(interface_tbl[2].comparator_tbl_index<<RXPORT23_2_CMPR_SHIFT) |
			(interface_tbl[2].l2h<<RXPORT23_2_L2H_SHIFT) |
			(interface_tbl[3].comparator_tbl_index<<RXPORT23_3_CMPR_SHIFT) |
			(interface_tbl[3].l2h<<RXPORT23_3_L2H_SHIFT));
}

void hwnat_itf_exit(void)
{
}

void hwnat_itf_tbl_write(uint16 entry, interface_t *itf)
{
	uint32 table_hold[GTH_SIZE];
	uint32 reg;

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = (itf->mtu & 0x7ff) | ((itf->l2h<<11)&0x3f800) | 
						((itf->comparator_tbl_index<<18) & 0x1fc0000);
	hwnat_write_tbl(INTERFACE_TBL_ID, entry, table_hold, GTH_SIZE);

	if (entry == 0) {
		hwnat_write_reg(CR_HWNAT_RXPORT01, 
				(hwnat_read_reg(CR_HWNAT_RXPORT01) & (RXPORT01_1_CMPR | RXPORT01_1_L2H)) |
				(itf->comparator_tbl_index<<RXPORT01_0_CMPR_SHIFT) |
				(itf->l2h<<RXPORT01_0_L2H_SHIFT));
	} else if (entry == 1) {
		hwnat_write_reg(CR_HWNAT_RXPORT01, 
				(hwnat_read_reg(CR_HWNAT_RXPORT01) & (RXPORT01_0_CMPR | RXPORT01_0_L2H)) |
				(itf->comparator_tbl_index<<RXPORT01_1_CMPR_SHIFT) |
				(itf->l2h<<RXPORT01_1_L2H_SHIFT));
	} else if (entry == 2) {
		hwnat_write_reg(CR_HWNAT_RXPORT23, 
				(hwnat_read_reg(CR_HWNAT_RXPORT23) & (RXPORT23_3_CMPR | RXPORT23_3_L2H)) |
				(itf->comparator_tbl_index<<RXPORT23_2_CMPR_SHIFT) |
				(itf->l2h<<RXPORT23_2_L2H_SHIFT));
	} else if (entry == 3) {
		hwnat_write_reg(CR_HWNAT_RXPORT23, 
				(hwnat_read_reg(CR_HWNAT_RXPORT23) & (RXPORT23_2_CMPR | RXPORT23_2_L2H)) |
				(itf->comparator_tbl_index<<RXPORT23_3_CMPR_SHIFT) |
				(itf->l2h<<RXPORT23_3_L2H_SHIFT));
	}
}

void hwnat_itf_tbl_read(uint16 entry, interface_t *itf)
{
	uint32 table_hold[GTH_SIZE];

	hwnat_read_tbl(INTERFACE_TBL_ID, entry, table_hold, GTH_SIZE);
	itf->mtu = table_hold[7] & 0x7ff;
	itf->l2h = (table_hold[7] & 0x3f800) >> 11;
	itf->comparator_tbl_index = (table_hold[7] & 0x1fc0000) >> 18;

	if (entry == 0) {
		itf->l2h = (hwnat_read_reg(CR_HWNAT_RXPORT01) & RXPORT01_0_L2H) >> RXPORT01_0_L2H_SHIFT;
		itf->comparator_tbl_index = (hwnat_read_reg(CR_HWNAT_RXPORT01) & RXPORT01_0_CMPR) >> RXPORT01_0_CMPR_SHIFT;
	} else if (entry == 1) {
		itf->l2h = (hwnat_read_reg(CR_HWNAT_RXPORT01) & RXPORT01_1_L2H) >> RXPORT01_1_L2H_SHIFT;
		itf->comparator_tbl_index = (hwnat_read_reg(CR_HWNAT_RXPORT01) & RXPORT01_1_CMPR) >> RXPORT01_1_CMPR_SHIFT;
	} else if (entry == 2) {
		itf->l2h = (hwnat_read_reg(CR_HWNAT_RXPORT23) & RXPORT23_2_L2H) >> RXPORT23_2_L2H_SHIFT;
		itf->comparator_tbl_index = (hwnat_read_reg(CR_HWNAT_RXPORT23) & RXPORT23_2_CMPR) >> RXPORT23_2_CMPR_SHIFT;
	} else if (entry == 3) {
		itf->l2h = (hwnat_read_reg(CR_HWNAT_RXPORT23) & RXPORT23_3_L2H) >> RXPORT23_3_L2H_SHIFT;
		itf->comparator_tbl_index = (hwnat_read_reg(CR_HWNAT_RXPORT23) & RXPORT23_3_CMPR) >> RXPORT23_3_CMPR_SHIFT;
	}
}

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/

static int do_hwnat_itf_rd(int argc, char *argv[], void *p);
static int do_hwnat_itf_wr(int argc, char *argv[], void *p);

static int do_hwnat_itf_tbl(int argc, char *argv[], void *p);
static int do_hwnat_itf_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_itf_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_itf_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_itf_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_itf_cmds[] = {
	{"rd",			do_hwnat_itf_rd,		0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_itf_wr,		0x02,  	4,  "<entry> <index> <l2> <mtu>"},
	{"tbl",			do_hwnat_itf_tbl,		0x12,  	0,  NULL},
	{NULL,			NULL,					0x10,	0,	NULL},
};

static const cmds_t hwnat_itf_tbl_cmds[] = {
	{"dump",		do_hwnat_itf_tbl_dump,	0x02,  	0,  NULL},
	{"raw",			do_hwnat_itf_tbl_raw,	0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_itf_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_itf_tbl_comp,	0x02,  	0,  NULL},
	{NULL,			NULL,					0x10,	0,	NULL},
};

int do_hwnat_itf(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_itf_cmds, argc, argv, p);
}

int do_hwnat_itf_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	interface_t itf;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_itf_tbl_read(entry, &itf);

	printk("/*%02d*/ ", entry);
	printk("%3d %2d %4d\n", itf.comparator_tbl_index, 
				itf.l2h, itf.mtu);

	return 0;
}

int do_hwnat_itf_wr(int argc, char *argv[], void *p)
{
	uint16 entry;
	interface_t *itf;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	itf = &interface_tbl[entry];

	itf->comparator_tbl_index = (uint16) simple_strtoul(argv[2], NULL, 10);
	itf->l2h = (uint16) simple_strtoul(argv[3], NULL, 10);
	itf->mtu = (uint16) simple_strtoul(argv[4], NULL, 10);

	hwnat_itf_tbl_write(entry, itf);

	return 0;
}

int do_hwnat_itf_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_itf_tbl_cmds, argc, argv, p);
}

int do_hwnat_itf_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;

	printk("CmprTblIdx L2H MTU\n");
	for (entry = 0; entry < INTERFACE_TBL_SIZE; entry++) {
		printk("/*%02d*/ ", entry);
		printk("%3d %2d %4d\n", interface_tbl[entry].comparator_tbl_index, 
				interface_tbl[entry].l2h, interface_tbl[entry].mtu);
	}

	return 0;
}

int do_hwnat_itf_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	interface_t *itf;
	uint16 entry, i;

	printk("interface_tbl\n");
	for (entry = 0; entry < INTERFACE_TBL_SIZE; entry++) {
		itf = &interface_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (itf->mtu & 0x7ff) | ((itf->l2h<<11)&0x3f800) | 
							((itf->comparator_tbl_index<<18) & 0x1fc0000);
		printk("/*%02d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}

	return 0;
}

int do_hwnat_itf_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	uint16 entry, i;
	uint8 dump_one = 0;

	if (argc >= 2) {
		entry = simple_strtoul(argv[1], NULL, 10);
		dump_one = 1;
		goto hwraw_dump;
	}

	printk("interface_tbl\n");
	for (entry = 0; entry < INTERFACE_TBL_SIZE; entry++) {
hwraw_dump:
		hwnat_read_tbl(INTERFACE_TBL_ID, entry, table_hold, GTH_SIZE);
		printk("/*%02d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");

		if (dump_one)
			break;
	}

	return 0;
}

int do_hwnat_itf_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[GTH_SIZE];
	uint32 table_hold[GTH_SIZE];
	interface_t *itf;
	uint16 entry, i;

	for (entry = 0; entry < INTERFACE_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(INTERFACE_TBL_ID, entry, hw_table_hold, GTH_SIZE);

		itf = &interface_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (itf->mtu & 0x7ff) | ((itf->l2h<<11)&0x3f800) | 
							((itf->comparator_tbl_index<<18) & 0x1fc0000);

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

