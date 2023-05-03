/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_tue.c#1 $
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
** $Log: hwnat_tue.c,v $
** Revision 1.2  2011/06/03 02:41:48  lino
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
	uint16 tuple_fmt[5];
} tuple_t;

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static void hwnat_tuple_init(void);
static void hwnat_tuple_tbl_write(uint16 entry, tuple_t *tuple);
static void hwnat_tuple_tbl_read(uint16 entry, tuple_t *tuple);

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

tuple_t tuple_tbl[TUPLE_TBL_SIZE] = {
/*	tuple_fmt */
/*  0 */ { 0x9121, 0x9184, 0x9204, 0xA002, 0x2042 },	/* IPv4 TCP|UDP */
/*  1 */ { 0x9121, 0x9184, 0x9204, 0xA002, 0x2042 },	/* PPPoE + IPv4 TCP|UDP */
/*  2 */ { 0x9121, 0x9184, 0x9204, 0xA002, 0x2082 },	/* IPv4 ICMP */
/*  3 */ { 0x9121, 0x9184, 0x9204, 0xA002, 0x2082 },	/* PPPoE + IPv4 ICMP */
/*  4 */ { 0x9121, 0x9184, 0x9204, 0x20C2, 0x0000 },	/* IPv4 GRE */
/*  5 */ { 0x9121, 0x9184, 0x9204, 0x20C2, 0x0000 },	/* PPPoE + IPv4 GRE */
/*  6 */ { 0x9121, 0x9184, 0x9204, 0x2004, 0x0000 },	/* IPv4 ESP */
/*  7 */ { 0x9121, 0x9184, 0x9204, 0x2004, 0x0000 },	/* PPPoE + IPv4 ESP */
/*  8 */ { 0x9121, 0x9184, 0x9204, 0x0000, 0x0000 },	/* IPv4 only */
/*  9 */ { 0x9121, 0x9184, 0x9204, 0x0000, 0x0000 },	/* PPPoE + IPv4 only */
/* 10 */ { 0x90C1, 0x1310, 0x0000, 0x0000, 0x0000 },	/* IPv6 */
/* 11 */ { 0x90C1, 0x1310, 0x0000, 0x0000, 0x0000 },	/* PPPoE + IPv6 */
/* 12 */ { 0x40F3, 0x0000, 0x0000, 0x0000, 0x0000 },	/* test */
};

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

void hwnat_tue_init(void)
{
	hwnat_tuple_init();
}

void hwnat_tue_exit(void)
{
}

void hwnat_tuple_init(void)
{
	uint16 entry;

	for (entry = 0; entry < TUPLE_TBL_SIZE; entry++)
		hwnat_tuple_tbl_write(entry, &tuple_tbl[entry]);
}

void hwnat_tuple_tbl_write(uint16 entry, tuple_t *tuple)
{
	uint32 table_hold[GTH_SIZE];

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = (tuple->tuple_fmt[4] & 0xffff) | ((tuple->tuple_fmt[3]<<16)&0xffff0000);
	table_hold[6] = (tuple->tuple_fmt[2] & 0xffff) | ((tuple->tuple_fmt[1]<<16)&0xffff0000);
	table_hold[5] = (tuple->tuple_fmt[0] & 0xffff);
	hwnat_write_tbl(TUPLE_TBL_ID, entry, table_hold, GTH_SIZE);
}

void hwnat_tuple_tbl_read(uint16 entry, tuple_t *tuple)
{
	uint32 table_hold[GTH_SIZE];

	hwnat_read_tbl(TUPLE_TBL_ID, entry, table_hold, GTH_SIZE);
	tuple->tuple_fmt[4] = table_hold[7] & 0xffff;
	tuple->tuple_fmt[3] = (table_hold[7] & 0xffff0000)>>16;
	tuple->tuple_fmt[2] = table_hold[6] & 0xffff;
	tuple->tuple_fmt[1] = (table_hold[6] & 0xffff0000)>>16;
	tuple->tuple_fmt[0] = table_hold[5] & 0xffff;
}

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/
static int do_hwnat_tuple_rd(int argc, char *argv[], void *p);
static int do_hwnat_tuple_wr(int argc, char *argv[], void *p);

static int do_hwnat_tuple_tbl(int argc, char *argv[], void *p);
static int do_hwnat_tuple_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_tuple_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_tuple_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_tuple_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_tuple_cmds[] = {
	{"rd",			do_hwnat_tuple_rd,			0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_tuple_wr,			0x02,  	6,  "<entry> <fmt0> <fmt1> <fmt2> <fmt3> <fmt4>"},
	{"tbl",			do_hwnat_tuple_tbl,			0x12,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_tuple_tbl_cmds[] = {
	{"dump",		do_hwnat_tuple_tbl_dump,	0x02,  	0,  NULL},
	{"raw",			do_hwnat_tuple_tbl_raw,		0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_tuple_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_tuple_tbl_comp,	0x02,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

int do_hwnat_tuple(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_tuple_cmds, argc, argv, p);
}

int do_hwnat_tuple_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	tuple_t tuple;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_tuple_tbl_read(entry, &tuple);

	printk("/*%02d*/ ", entry);
	printk("%04x %04x %04x %04x %04x\n",
					tuple.tuple_fmt[0], tuple.tuple_fmt[1], tuple.tuple_fmt[2], tuple.tuple_fmt[3], 
					tuple.tuple_fmt[4]);

	return 0;
}

int do_hwnat_tuple_wr(int argc, char *argv[], void *p)
{
	uint16 entry;
	tuple_t *tuple;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	tuple = &tuple_tbl[entry];

	tuple->tuple_fmt[0] = (uint16) simple_strtoul(argv[2], NULL, 16);
	tuple->tuple_fmt[1] = (uint16) simple_strtoul(argv[3], NULL, 16);
	tuple->tuple_fmt[2] = (uint16) simple_strtoul(argv[4], NULL, 16);
	tuple->tuple_fmt[3] = (uint16) simple_strtoul(argv[5], NULL, 16);
	tuple->tuple_fmt[4] = (uint16) simple_strtoul(argv[6], NULL, 16);

	hwnat_tuple_tbl_write(entry, tuple);

	return 0;
}

int do_hwnat_tuple_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_tuple_tbl_cmds, argc, argv, p);
}

int do_hwnat_tuple_tbl_dump(int argc, char *argv[], void *p)
{
	int entry;

	printk("tuple_fmt\n");
	for (entry = 0; entry < TUPLE_TBL_SIZE; entry++) {
		printk("/*%02d*/ ", entry);
		printk("%04x %04x %04x %04x %04x\n",
					tuple_tbl[entry].tuple_fmt[0], tuple_tbl[entry].tuple_fmt[1], 
					tuple_tbl[entry].tuple_fmt[2], tuple_tbl[entry].tuple_fmt[3], 
					tuple_tbl[entry].tuple_fmt[4]);
	}
	return 0;
}

int do_hwnat_tuple_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	tuple_t *tuple;
	int entry, i;

	printk("tuple\n");
	for (entry = 0; entry < TUPLE_TBL_SIZE; entry++) {
		tuple = &tuple_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (tuple->tuple_fmt[4] & 0xffff) | ((tuple->tuple_fmt[3]<<16)&0xffff0000);
		table_hold[6] = (tuple->tuple_fmt[2] & 0xffff) | ((tuple->tuple_fmt[1]<<16)&0xffff0000);
		table_hold[5] = (tuple->tuple_fmt[0] & 0xffff);

		printk("/*%02d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_tuple_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	int entry, i;

	printk("tuple\n");
	for (entry = 0; entry < TUPLE_TBL_SIZE; entry++) {
		hwnat_read_tbl(TUPLE_TBL_ID, entry, table_hold, GTH_SIZE);
		printk("/*%02d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_tuple_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[GTH_SIZE];
	uint32 table_hold[GTH_SIZE];
	tuple_t *tuple;
	int entry, i;

	for (entry = 0; entry < TUPLE_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(TUPLE_TBL_ID, entry, hw_table_hold, GTH_SIZE);

		tuple = &tuple_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (tuple->tuple_fmt[4] & 0xffff) | ((tuple->tuple_fmt[3]<<16)&0xffff0000);
		table_hold[6] = (tuple->tuple_fmt[2] & 0xffff) | ((tuple->tuple_fmt[1]<<16)&0xffff0000);
		table_hold[5] = (tuple->tuple_fmt[0] & 0xffff);

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

