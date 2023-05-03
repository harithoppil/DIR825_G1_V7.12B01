/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_mac.c#1 $
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
** $Log: hwnat_mac.c,v $
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
#include <linux/jhash.h>
#include <linux/random.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <asm/unaligned.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/cmdparse.h>

#include "hwnat.h"
#include "hwnat_reg.h"
#include "hwnat_mac.h"

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                            M A C R O S
*************************************************************************
*/
#define MAC_ADDR_FIND_SIZE		16

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/
typedef struct {
	uint8 is_used;
	uint16 entry;
	atomic_t refcnt;
	uint8 addr[ETH_ALEN];
} mac_addr_t;


/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static void hwnat_mac_addr_tbl_write(uint16 entry, mac_addr_t *mac_addr);
static void hwnat_mac_addr_tbl_read(uint16 entry, mac_addr_t *mac_addr);

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

mac_addr_t mac_addr_tbl[MAC_ADDR_TBL_SIZE];

DEFINE_SPINLOCK(mac_addr_lock);

static uint32 hwnat_mac_addr_salt;

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

void hwnat_mac_addr_init(void)
{
	uint16 entry;

	get_random_bytes(&hwnat_mac_addr_salt, sizeof(hwnat_mac_addr_salt));

	for (entry = 0; entry < MAC_ADDR_TBL_SIZE; entry++) {
		mac_addr_tbl[entry].is_used = 0;
		mac_addr_tbl[entry].entry = entry;
		atomic_set(&mac_addr_tbl[entry].refcnt, 0);
		memset(mac_addr_tbl[entry].addr, 0x0, ETH_ALEN);

		hwnat_mac_addr_tbl_write(entry, &mac_addr_tbl[entry]);
	}
}

void hwnat_mac_addr_exit(void)
{
}

static inline uint16 hwnat_mac_addr_hash(const uint8 *mac)
{
	/* use 1 byte of OUI cnd 3 bytes of NIC */
	uint32 key = get_unaligned((uint32 *)(mac + 2));
	return jhash_1word(key, hwnat_mac_addr_salt) & (MAC_ADDR_TBL_SIZE - 1);
}

mac_addr_t *hwnat_mac_addr_find(uint8 *addr)
{
	uint16 hash = hwnat_mac_addr_hash(addr);
	mac_addr_t *mac_addr;
	uint16 i, entry;

	for (i = 0; i < MAC_ADDR_FIND_SIZE; i++) {
		entry = (hash + i) & (MAC_ADDR_TBL_SIZE - 1);
		mac_addr = &mac_addr_tbl[entry];
		if (mac_addr->is_used && !compare_ether_addr(mac_addr->addr, addr)) {
			atomic_inc(&mac_addr->refcnt);
			return mac_addr;
		}
	}

	return NULL;
}

mac_addr_t *hwnat_mac_addr_create(uint8 *addr)
{
	uint16 hash = hwnat_mac_addr_hash(addr);
	mac_addr_t *mac_addr;
	uint16 i, entry;

	for (i = 0; i < MAC_ADDR_FIND_SIZE; i++) {
		entry = (hash + i) & (MAC_ADDR_TBL_SIZE - 1);
		mac_addr = &mac_addr_tbl[entry];
		if (!mac_addr->is_used) {
			mac_addr->is_used = 1;
			atomic_set(&mac_addr->refcnt, 1);
			memcpy(mac_addr->addr, addr, ETH_ALEN);
			return mac_addr;
		}
	}

	return NULL;
}

uint8 *hwnat_mac_addr_get(uint8 entry)
{
	mac_addr_t *mac_addr;
	uint8 *addr;
	unsigned long flags;

	spin_lock_irqsave(&mac_addr_lock, flags);

	mac_addr = &mac_addr_tbl[entry];
	if (mac_addr->is_used) 
		addr = mac_addr->addr;
	else
		addr = NULL;

	spin_unlock_irqrestore(&mac_addr_lock, flags);

	return addr;
}

int hwnat_mac_addr_insert(uint8 *addr)
{
	mac_addr_t *mac_addr;
	unsigned long flags;

	spin_lock_irqsave(&mac_addr_lock, flags);

	mac_addr = hwnat_mac_addr_find(addr);
	if (mac_addr) {
		spin_unlock_irqrestore(&mac_addr_lock, flags);
		return mac_addr->entry;
	}

	mac_addr = hwnat_mac_addr_create(addr);
	if (mac_addr) {
		hwnat_mac_addr_tbl_write(mac_addr->entry, mac_addr);
		spin_unlock_irqrestore(&mac_addr_lock, flags);
		return mac_addr->entry;
	}

	spin_unlock_irqrestore(&mac_addr_lock, flags);

	return -1;
}

int hwnat_mac_addr_remove(uint8 entry)
{
	mac_addr_t *mac_addr;
	unsigned long flags;

	spin_lock_irqsave(&mac_addr_lock, flags);

	mac_addr = &mac_addr_tbl[entry];
	if (mac_addr->is_used) {
		if (atomic_dec_and_test(&mac_addr->refcnt)) {
			mac_addr->is_used = 0;
			memset(mac_addr->addr, 0x0, ETH_ALEN);
			hwnat_mac_addr_tbl_write(mac_addr->entry, mac_addr);
		}
	}

	spin_unlock_irqrestore(&mac_addr_lock, flags);

	return 0;
}

void hwnat_mac_addr_tbl_write(uint16 entry, mac_addr_t *mac_addr)
{
	uint32 table_hold[MTH_SIZE];

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[1] = (mac_addr->addr[5]&0xff) | ((mac_addr->addr[4]<<8)&0xff00) | 
		((mac_addr->addr[3]<<16)&0xff0000) | ((mac_addr->addr[2]<<24)&0xff000000);
	table_hold[0] = (mac_addr->addr[1]&0xff) | ((mac_addr->addr[0]<<8)&0xff00);
	hwnat_write_tbl(MAC_ADDR_TBL_ID, entry, table_hold, MTH_SIZE);
}

void hwnat_mac_addr_tbl_read(uint16 entry, mac_addr_t *mac_addr)
{
	uint32 table_hold[MTH_SIZE];

	hwnat_read_tbl(MAC_ADDR_TBL_ID, entry, table_hold, MTH_SIZE);
	mac_addr->addr[5] = table_hold[1]&0xff;
	mac_addr->addr[4] = (table_hold[1]&0xff00)>>8;
	mac_addr->addr[3] = (table_hold[1]&0xff0000)>>16;
	mac_addr->addr[2] = (table_hold[1]&0xff000000)>>24;
	mac_addr->addr[1] = table_hold[0]&0xff;
	mac_addr->addr[0] = (table_hold[0]&0xff00)>>8;
}

int hwnat_mac_addr_in_ether(char *bufp, char *ptr)  
{  
    int i, j;  
    unsigned char val;  
    unsigned char c;  
  
    i = 0;  
    do {  
        j = val = 0;  
  
        /* We might get a semicolon here - not required. */  
        if (i && (*bufp == ':')) {  
            bufp++;  
        }  
  
        do {  
            c = *bufp;  
            if (((unsigned char)(c - '0')) <= 9) {  
                c -= '0';  
            } else if (((unsigned char)((c|0x20) - 'a')) <= 5) {  
                c = (c|0x20) - ('a'-10);  
            } else if (j && (c == ':' || c == 0)) {  
                break;  
            } else {  
                return -1;  
            }  
            ++bufp;  
            val <<= 4;  
            val += c;  
        } while (++j < 2);  
        *ptr++ = val;  
    } while (++i < ETH_ALEN);  
  
    return (int) (*bufp);   /* Error if we don't end at end of string. */  
}

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/
static int do_hwnat_mac_addr_rd(int argc, char *argv[], void *p);
static int do_hwnat_mac_addr_wr(int argc, char *argv[], void *p);

static int do_hwnat_mac_addr_tbl(int argc, char *argv[], void *p);
static int do_hwnat_mac_addr_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_mac_addr_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_mac_addr_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_mac_addr_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_mac_addr_cmds[] = {
	{"rd",			do_hwnat_mac_addr_rd,			0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_mac_addr_wr,			0x02,  	2,  "<entry> <macaddr>"},
	{"tbl",			do_hwnat_mac_addr_tbl,			0x12,  	0,  NULL},
	{NULL,			NULL,							0x10,	0,	NULL},
};

static const cmds_t hwnat_mac_addr_tbl_cmds[] = {
	{"dump",		do_hwnat_mac_addr_tbl_dump,		0x02,  	0,  NULL},
	{"raw",			do_hwnat_mac_addr_tbl_raw,		0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_mac_addr_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_mac_addr_tbl_comp,		0x02,  	0,  NULL},
	{NULL,			NULL,							0x10,	0,	NULL},
};

int do_hwnat_mac_addr(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_mac_addr_cmds, argc, argv, p);
}

int do_hwnat_mac_addr_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	mac_addr_t mac_addr;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_mac_addr_tbl_read(entry, &mac_addr);

	printk("/*%03d*/ ", entry);
	printk("%02x:%02x:%02x:%02x:%02x:%02x\n", 
				mac_addr.addr[0], mac_addr.addr[1], mac_addr.addr[2], mac_addr.addr[3], 
				mac_addr.addr[4], mac_addr.addr[5]);

	return 0;
}

int do_hwnat_mac_addr_wr(int argc, char *argv[], void *p)
{
	uint16 entry;
	mac_addr_t *mac_addr;
	uint8 macaddr[ETH_ALEN];

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	mac_addr = &mac_addr_tbl[entry];

	if (hwnat_mac_addr_in_ether(argv[2], macaddr)) {
		printk("err: mac addr format err\n");
		return 0;
	}

	memcpy(mac_addr->addr, macaddr, ETH_ALEN);

	hwnat_mac_addr_tbl_write(entry, mac_addr);

	return 0;
}

int do_hwnat_mac_addr_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_mac_addr_tbl_cmds, argc, argv, p);
}

int do_hwnat_mac_addr_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;

	printk("mac_addr\n");
	for (entry = 0; entry < MAC_ADDR_TBL_SIZE; entry++) {
		printk("/*%03d*/ %02x:%02x:%02x:%02x:%02x:%02x\n", entry,
					mac_addr_tbl[entry].addr[0], mac_addr_tbl[entry].addr[1], 
					mac_addr_tbl[entry].addr[2], mac_addr_tbl[entry].addr[3], 
					mac_addr_tbl[entry].addr[4], mac_addr_tbl[entry].addr[5]);
	}
	return 0;
}

int do_hwnat_mac_addr_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[MTH_SIZE];
	mac_addr_t *mac_addr;
	uint16 entry, i;

	printk("mac_addr\n");
	for (entry = 0; entry < MAC_ADDR_TBL_SIZE; entry++) {
		mac_addr = &mac_addr_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[1] = (mac_addr->addr[5]&0xff) | ((mac_addr->addr[4]<<8)&0xff00) | 
			((mac_addr->addr[3]<<16)&0xff0000) | ((mac_addr->addr[2]<<24)&0xff000000);
		table_hold[0] = (mac_addr->addr[1]&0xff) | ((mac_addr->addr[0]<<8)&0xff00);

		printk("/*%02d*/", entry);
		for (i = 0; i < MTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_mac_addr_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[MTH_SIZE];
	uint16 entry, i;
	uint8 dump_one = 0;

	if (argc >= 2) {
		entry = simple_strtoul(argv[1], NULL, 10);
		dump_one = 1;
		goto hwraw_dump;
	}

	printk("mac_addr\n");
	for (entry = 0; entry < MAC_ADDR_TBL_SIZE; entry++) {
hwraw_dump:
		hwnat_read_tbl(MAC_ADDR_TBL_ID, entry, table_hold, MTH_SIZE);
		printk("/*%02d*/", entry);
		for (i = 0; i < MTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");

		if (dump_one)
			break;
	}
	return 0;
}

int do_hwnat_mac_addr_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[MTH_SIZE];
	uint32 table_hold[MTH_SIZE];
	mac_addr_t *mac_addr;
	uint16 entry, i;

	for (entry = 0; entry < MAC_ADDR_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(MAC_ADDR_TBL_ID, entry, hw_table_hold, MTH_SIZE);

		mac_addr = &mac_addr_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[1] = (mac_addr->addr[5]&0xff) | ((mac_addr->addr[4]<<8)&0xff00) | 
			((mac_addr->addr[3]<<16)&0xff0000) | ((mac_addr->addr[2]<<24)&0xff000000);
		table_hold[0] = (mac_addr->addr[1]&0xff) | ((mac_addr->addr[0]<<8)&0xff00);

		if (memcmp(hw_table_hold, table_hold, sizeof(hw_table_hold)) != 0) {
			printk("error /*%02d*/\n", entry);
			for (i = 0; i < MTH_SIZE; i++)
				printk(" %08lx", hw_table_hold[i]);
			printk("\n");
			for (i = 0; i < MTH_SIZE; i++)
				printk(" %08lx", table_hold[i]);
			printk("\n");
		}
	}
	return 0;
}

