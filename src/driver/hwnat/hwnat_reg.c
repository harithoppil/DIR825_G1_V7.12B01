/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_reg.c#1 $
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
** $Log: hwnat_reg.c,v $
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

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
#ifdef HWNAT_SW_MODEL

extern uint8 hwnat_table_write(uint8 name, uint16 entry, uint32 *data, uint16 len);
extern uint8 hwnat_table_read(uint8 name, uint16 entry, uint32 *data, uint16 len);
extern void hwnat_data_dump(uint8 module, uint32 *data);

extern void hwnat_reg_write(uint16 index, uint32 val);
extern uint32 hwnat_reg_read(uint16 index);

#else

static uint8 hwnat_write_gtbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len);
static uint8 hwnat_write_htbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len);
static uint8 hwnat_write_ftbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len);
static uint8 hwnat_write_mtbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len);

static uint8 hwnat_read_gtbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len);
static uint8 hwnat_read_htbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len);
static uint8 hwnat_read_ftbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len);
static uint8 hwnat_read_mtbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len);

#endif

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

DEFINE_SPINLOCK(gt_lock);
DEFINE_SPINLOCK(ht_lock);
DEFINE_SPINLOCK(ft_lock);
DEFINE_SPINLOCK(mt_lock);

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#ifdef HWNAT_SW_MODEL

uint8 hwnat_write_tbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	return hwnat_table_write(tbl_id, entry, data, len<<2);
}

uint8 hwnat_read_tbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	return hwnat_table_read(tbl_id, entry, data, len<<2);
}

void hwnat_write_reg(uint32 reg, uint32 val)
{
	hwnat_reg_write(reg - CR_HWNAT_BASE, val);
}

uint32 hwnat_read_reg(uint32 reg)
{
	return hwnat_reg_read(reg - CR_HWNAT_BASE);
}

#else

uint8 hwnat_write_tbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	switch (tbl_id) {
		case HASH_TBL_ID:
			return hwnat_write_htbl(tbl_id, entry, data, len);
		case FLOW_TBL_ID:
			return hwnat_write_ftbl(tbl_id, entry, data, len);
		case MAC_ADDR_TBL_ID:
			return hwnat_write_mtbl(tbl_id, entry, data, len);
		default:
			return hwnat_write_gtbl(tbl_id, entry, data, len);
	}
	return 0;
}

uint8 hwnat_read_tbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	switch (tbl_id) {
		case HASH_TBL_ID:
			return hwnat_read_htbl(tbl_id, entry, data, len);
		case FLOW_TBL_ID:
			return hwnat_read_ftbl(tbl_id, entry, data, len);
		case MAC_ADDR_TBL_ID:
			return hwnat_read_mtbl(tbl_id, entry, data, len);
		default:
			return hwnat_read_gtbl(tbl_id, entry, data, len);
	}
	return 0;
}

void hwnat_write_reg(uint32 reg, uint32 val)
{
	VPint(reg) = val;
}

uint32 hwnat_read_reg(uint32 reg)
{
	return VPint(reg);
}

static uint8 hwnat_write_gtbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	unsigned long flags;
	int i;

	spin_lock_irqsave(&gt_lock, flags);
	for (i = 0; i < len; i++) 
		VPint(CR_HWNAT_GH(i)) = data[i];
	VPint(CR_HWNAT_GTS) = ((entry << GTS_ENTRY_SHIFT) & GTS_ENTRY) | ((tbl_id << GTS_TS_SHIFT) & GTS_TS);
	VPint(CR_HWNAT_GTS) |= GTS_TW;
	while (!(VPint(CR_HWNAT_GTS) & GTS_TW_DONE)) ;
	spin_unlock_irqrestore(&gt_lock, flags);

	return 0;
}

static uint8 hwnat_write_htbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	unsigned long flags;

	spin_lock_irqsave(&ht_lock, flags);
	VPint(CR_HWNAT_HH0) = data[0];
	VPint(CR_HWNAT_HTS) = (entry << HTS_ENTRY_SHIFT) & HTS_ENTRY;
	VPint(CR_HWNAT_HTS) |= HTS_TW;
	while (!(VPint(CR_HWNAT_HTS) & HTS_TW_DONE)) ;
	spin_unlock_irqrestore(&ht_lock, flags);

	return 0;
}

static uint8 hwnat_write_ftbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	unsigned long flags;
	int i;

	spin_lock_irqsave(&ft_lock, flags);
	for (i = 0; i < len; i++) 
		VPint(CR_HWNAT_FH(i)) = data[i];
	VPint(CR_HWNAT_FTS) = (entry << FTS_ENTRY_SHIFT) & FTS_ENTRY;
	VPint(CR_HWNAT_FTS) |= FTS_TW;
	while (!(VPint(CR_HWNAT_FTS) & FTS_TW_DONE)) ;
	spin_unlock_irqrestore(&ft_lock, flags);

	return 0;
}

static uint8 hwnat_write_mtbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	unsigned long flags;
	int i;

	spin_lock_irqsave(&mt_lock, flags);
	for (i = 0; i < len; i++) 
		VPint(CR_HWNAT_MH(i)) = data[i];
	VPint(CR_HWNAT_MTS) = (entry << MTS_ENTRY_SHIFT) & MTS_ENTRY;
	VPint(CR_HWNAT_MTS) |= MTS_TW;
	while (!(VPint(CR_HWNAT_MTS) & MTS_TW_DONE)) ;
	spin_unlock_irqrestore(&mt_lock, flags);

	return 0;
}

static uint8 hwnat_read_gtbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	unsigned long flags;
	int i;

	spin_lock_irqsave(&gt_lock, flags);
	VPint(CR_HWNAT_GTS) = ((entry << GTS_ENTRY_SHIFT) & GTS_ENTRY) | (tbl_id << GTS_TS_SHIFT);
	VPint(CR_HWNAT_GTS) |= GTS_TR;
	while (!(VPint(CR_HWNAT_GTS) & GTS_TR_DONE)) ;
	for (i = 0; i < len; i++) 
		data[i] = VPint(CR_HWNAT_GH(i));
	spin_unlock_irqrestore(&gt_lock, flags);

	return 0;
}

static uint8 hwnat_read_htbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	unsigned long flags;

	spin_lock_irqsave(&ht_lock, flags);
	VPint(CR_HWNAT_HTS) = (entry << HTS_ENTRY_SHIFT) & HTS_ENTRY;
	VPint(CR_HWNAT_HTS) |= GTS_TR;
	while (!(VPint(CR_HWNAT_HTS) & HTS_TR_DONE)) ;
	data[0] = VPint(CR_HWNAT_HH0);
	spin_unlock_irqrestore(&ht_lock, flags);

	return 0;
}

static uint8 hwnat_read_ftbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	unsigned long flags;
	int i;

	spin_lock_irqsave(&ft_lock, flags);
	VPint(CR_HWNAT_FTS) = (entry << FTS_ENTRY_SHIFT) & FTS_ENTRY;
	VPint(CR_HWNAT_FTS) |= FTS_TR;
	while (!(VPint(CR_HWNAT_FTS) & FTS_TR_DONE)) ;
	for (i = 0; i < len; i++) 
		data[i] = VPint(CR_HWNAT_FH(i));
	spin_unlock_irqrestore(&ft_lock, flags);

	return 0;
}

static uint8 hwnat_read_mtbl(uint8 tbl_id, uint16 entry, uint32 *data, uint16 len)
{
	unsigned long flags;
	int i;

	spin_lock_irqsave(&mt_lock, flags);
	VPint(CR_HWNAT_MTS) = (entry << MTS_ENTRY_SHIFT) & MTS_ENTRY;
	VPint(CR_HWNAT_MTS) |= MTS_TR;
	while (!(VPint(CR_HWNAT_MTS) & MTS_TR_DONE)) ;
	for (i = 0; i < len; i++) 
		data[i] = VPint(CR_HWNAT_MH(i));
	spin_unlock_irqrestore(&mt_lock, flags);

	return 0;
}

#endif

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/

static int do_hwnat_reg_rd(int argc, char *argv[], void *p);
static int do_hwnat_reg_wr(int argc, char *argv[], void *p);
static int do_hwnat_reg_dump(int argc, char *argv[], void *p);

static const cmds_t hwnat_reg_cmds[] = {
	{"rd",			do_hwnat_reg_rd,			0x02,  	1,  "<addr>"},
	{"wr",			do_hwnat_reg_wr,			0x02,  	2,  "<addr> <val>"},
	{"dump",		do_hwnat_reg_dump,			0x02,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

int do_hwnat_reg(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_reg_cmds, argc, argv, p);
}

int do_hwnat_reg_rd(int argc, char *argv[], void *p)
{
	unsigned long reg;

	reg = (unsigned long)simple_strtoul(argv[1], NULL, 16);
	printk("\r\n<Address>\t<Value>\r\n");
	printk("0x%08lx\t0x%08lx\r\n", (unsigned long) (CR_HWNAT_BASE + reg), (unsigned long) hwnat_read_reg(reg + CR_HWNAT_BASE));

	return 0;
}

int do_hwnat_reg_wr(int argc, char *argv[], void *p)
{
	unsigned long reg;
	unsigned long val;

	reg = (unsigned long)simple_strtoul(argv[1], NULL, 16);
	val = (unsigned long)simple_strtoul(argv[2], NULL, 16);
	hwnat_write_reg(reg + CR_HWNAT_BASE, val);

	return 0;
}

int do_hwnat_reg_dump(int argc, char *argv[], void *p)
{
	int index;

	printk("CR_HWNAT_CTL0       (0x%08x) = 0x%08lx\n", CR_HWNAT_CTL0, hwnat_read_reg(CR_HWNAT_CTL0)); 	
	printk("CR_HWNAT_CTL1       (0x%08x) = 0x%08lx\n", CR_HWNAT_CTL1, hwnat_read_reg(CR_HWNAT_CTL1)); 	
	printk("CR_HWNAT_GSR        (0x%08x) = 0x%08lx\n", CR_HWNAT_GSR, hwnat_read_reg(CR_HWNAT_GSR)); 	
	printk("CR_HWNAT_SSR        (0x%08x) = 0x%08lx\n", CR_HWNAT_SSR, hwnat_read_reg(CR_HWNAT_SSR)); 	
	printk("CR_HWNAT_GTS        (0x%08x) = 0x%08lx\n", CR_HWNAT_GTS, hwnat_read_reg(CR_HWNAT_GTS)); 	
	printk("CR_HWNAT_HTS        (0x%08x) = 0x%08lx\n", CR_HWNAT_HTS, hwnat_read_reg(CR_HWNAT_HTS)); 	
	printk("CR_HWNAT_FTS        (0x%08x) = 0x%08lx\n", CR_HWNAT_FTS, hwnat_read_reg(CR_HWNAT_FTS)); 	
	printk("CR_HWNAT_MTS        (0x%08x) = 0x%08lx\n", CR_HWNAT_MTS, hwnat_read_reg(CR_HWNAT_MTS)); 	
	printk("CR_HWNAT_GV         (0x%08x) = 0x%08lx\n", CR_HWNAT_GV, hwnat_read_reg(CR_HWNAT_GV)); 	
	printk("CR_HWNAT_TR         (0x%08x) = 0x%08lx\n", CR_HWNAT_TR, hwnat_read_reg(CR_HWNAT_TR)); 	
	printk("CR_HWNAT_TRD        (0x%08x) = 0x%08lx\n", CR_HWNAT_TRD, hwnat_read_reg(CR_HWNAT_TRD)); 	
	for (index = 0; index < AGE_STATUS_SIZE; index++) 
		printk("CR_HWNAT_AS(%d)      (0x%08x) = 0x%08lx\n", index, CR_HWNAT_AS(index), hwnat_read_reg(CR_HWNAT_AS(index))); 	

	for (index = 0; index < GTH_SIZE; index++) 
		printk("CR_HWNAT_GH(%d)      (0x%08x) = 0x%08lx\n", index, CR_HWNAT_GH(index), hwnat_read_reg(CR_HWNAT_GH(index))); 	

	printk("CR_HWNAT_HH0        (0x%08x) = 0x%08lx\n", CR_HWNAT_HH0, hwnat_read_reg(CR_HWNAT_HH0)); 	

	for (index = 0; index < FTH_SIZE; index++) 
		printk("CR_HWNAT_FH(%02d)     (0x%08x) = 0x%08lx\n", index, CR_HWNAT_FH(index), hwnat_read_reg(CR_HWNAT_FH(index))); 	

	for (index = 0; index < MTH_SIZE; index++) 
		printk("CR_HWNAT_MH(%d)      (0x%08x) = 0x%08lx\n", index, CR_HWNAT_MH(index), hwnat_read_reg(CR_HWNAT_MH(index))); 	

	printk("CR_HWNAT_AFS        (0x%08x) = 0x%08lx\n", CR_HWNAT_AFS, hwnat_read_reg(CR_HWNAT_AFS)); 	
	printk("CR_HWNAT_AFC        (0x%08x) = 0x%08lx\n", CR_HWNAT_AFC, hwnat_read_reg(CR_HWNAT_AFC)); 

	return 0;
}

