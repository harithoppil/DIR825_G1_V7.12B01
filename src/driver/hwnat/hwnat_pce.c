/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_pce.c#1 $
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
** $Log: hwnat_pce.c,v $
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
	uint8 valid;
	uint8 map;
	uint16 mask;
	uint8 op;
	uint16 value;
	uint16 jump1;
	uint16 jump1_offset;
	uint16 jump0;
	uint16 jump0_offset;
	uint8 gs_valid;
	uint8 gs_flag;
	uint8 gs_index;
	uint8 l3_l4;
	uint8 l3_l4_offset;
} comparator_t;

typedef struct {
	uint32 map[GTH_SIZE];
} policy_t;

typedef struct {
	uint16 itf_map;
	uint16 tuple_index;
} policy_result_t;

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

static void hwnat_cmpr_init(void);
static void hwnat_cmpr_tbl_write(uint16 entry, comparator_t *cmpr);
static void hwnat_cmpr_tbl_read(uint16 entry, comparator_t *cmpr);

static void hwnat_policy_init(void);
static void hwnat_policy_tbl_write(uint16 entry, policy_t *policy);
static void hwnat_policy_tbl_read(uint16 entry, policy_t *policy);

static void hwnat_policy_result_init(void);
static void hwnat_policy_result_tbl_write(uint16 entry, policy_result_t *policy_result);
static void hwnat_policy_result_tbl_read(uint16 entry, policy_result_t *policy_result);

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

comparator_t comparator_tbl[COMPARATOR_TBL_SIZE] = {
/*	valid	map	mask	op	value	J1	J1 Offset	J0	J0 Offset	G/S Valid	G/S	G/S Index	L3/L4	L3/L4 Offset*/
/*   0 */ { 1, 0, 0xffff, 1, 0xAAAA,   1,  2, 128,  0, 0,  0,  0, 0, 0x3f },	/* RFC1483 Bridge LLC */
/*   1 */ { 1, 0, 0xffff, 1, 0x0300,   2,  2, 128,  0, 0,  0,  0, 0, 0x3f },	/*  */
/*   2 */ { 1, 0, 0xffff, 1, 0x80c2,   4,  2,   3,  0, 0,  0,  0, 0, 0x3f },	/*  */
/*   3 */ { 1, 0, 0xffff, 1, 0x0000,  22,  2, 128,  0, 1,  1,  8, 0, 0x3f },	/* RFC1483 Routing LLC */
/*   4 */ { 1, 0, 0xffff, 1, 0x0007,   6,  2,   5,  0, 0,  0,  0, 0, 0x3f },	/*  */
/*   5 */ { 1, 0, 0xffff, 1, 0x0001,   7,  2, 128,  0, 0,  0,  0, 0, 0x3f },	/*  */
/*   6 */ { 1, 0, 0xffff, 1, 0x0000,  15,  2, 128,  0, 1,  1,  9, 0, 0x3f },	/* RFC1483 Bridge LLC */
/*   7 */ { 1, 0, 0xffff, 1, 0x0000,  15,  2, 128,  0, 1,  1, 10, 0, 0x3f },	/* RFC1483 Bridge LLC */
/*   8 */ { 1, 0, 0xffff, 1, 0x0000,  15,  2, 128,  0, 1,  1, 11, 0, 0x3f },	/* RFC1483 Bridge VCMux */
/*   9 */ { 1, 0, 0xff00, 1, 0x4500,  28,  0,  10,  0, 0,  0,  0, 0, 0x3f },	/* RFC1483 Routing VCMux */
/*  10 */ { 1, 0, 0xf000, 1, 0x6000,  29,  0, 128,  0, 0,  0,  0, 0, 0x3f },	/*  */
/*  11 */ { 1, 0, 0xffff, 1, 0xFEFE,  12,  2, 128,  0, 0,  0,  0, 0, 0x3f },	/* PPPoA LLC */
/*  12 */ { 1, 0, 0xffff, 1, 0x03CF,  13,  2, 128,  0, 1,  1, 12, 0, 0x3f },	/*  */
/*  13 */ { 1, 0, 0xffff, 1, 0x0021,  28,  2,  14,  0, 1,  1, 13, 0, 0x3f },	/* PPPoA VCMux IPv4 */
/*  14 */ { 1, 0, 0xffff, 1, 0x0057,  29,  2, 128,  0, 1,  1, 14, 0, 0x3f },	/* PPPoA VCMux IPv6 */
/*  15 */ { 1, 0, 0x0000, 1, 0x0000,  16, 12, 128,  0, 0,  0,  0, 0, 0x3f },	/* offset += 12 */
/*  16 */ { 1, 0, 0xffff, 1, 0x8901,  17,  2,  18,  0, 1,  1,  0, 0, 0x3f },	/* 2206 stag */
/*  17 */ { 1, 0, 0x0000, 1, 0x0000,  18,  6,  18,  0, 0,  0,  0, 0, 0x3f },	/* 2206 inport */
/*  18 */ { 1, 1, 0xffff, 1, 0x8100,  19,  2,  20,  0, 1,  1,  1, 0, 0x3f },	/* outer VLAN */
/*  19 */ { 1, 0, 0x0000, 1, 0x0000,  20,  2,  20,  0, 1,  0,  4, 0, 0x3f },	/* outer VLAN tag */
/*  20 */ { 1, 1, 0xffff, 1, 0x8100,  21,  2,  22,  0, 1,  1,  2, 0, 0x3f },	/* inner VLAN */
/*  21 */ { 1, 0, 0x0000, 1, 0x0000,  22,  2,  22,  0, 1,  0,  5, 0, 0x3f },	/* inner VLAN tag */
/*  22 */ { 1, 1, 0xffff, 1, 0x0800,  28,  2,  23,  0, 1,  0, 31, 0, 0x3f },	/* Ethernet (IPv4) */
/*  23 */ { 1, 1, 0xffff, 1, 0x86dd,  29,  2,  24,  0, 1,  0, 31, 0, 0x3f },	/* Ethernet (IPv6) */
/*  24 */ { 1, 1, 0xffff, 1, 0x8864,  25,  2, 128,  0, 1,  0, 31, 0, 0x3f },	/* PPPoE */
/*  25 */ { 1, 0, 0xffff, 1, 0x1100,  26,  6, 128,  0, 0,  0,  0, 0, 0x3f },	/* PPPoE ver/code */
/*  26 */ { 1, 1, 0xffff, 1, 0x0021,  28,  2,  27,  0, 1,  1,  4, 0, 0x3f },	/* PPPoE+IPv4 */
/*  27 */ { 1, 1, 0xffff, 1, 0x0057,  29,  2, 128,  0, 1,  1,  5, 0, 0x3f },	/* PPPoE+IPv6 */
/*  28 */ { 1, 0, 0xff00, 1, 0x4500,  36,  2, 128,  0, 1,  1,  6, 1,    0 },	/* IPv4 */
/*  29 */ { 1, 0, 0xf000, 1, 0x6000,  30,  4, 128,  0, 1,  1,  7, 1,    0 },	/* IPv6 */
/*  30 */ { 1, 0, 0x0000, 1, 0x0000,  31,  2, 128,  0, 1,  0,  2, 0, 0x3f },	/* IPv6 Payload Length */
/*  31 */ { 1, 0, 0xff00, 1, 0x0600,  35,  0,  32,  0, 0,  0,  0, 0, 0x3f },	/* Next Header: TCP */
/*  32 */ { 1, 0, 0xff00, 1, 0x1100,  35,  0,  33,  0, 0,  0,  0, 0, 0x3f },	/* Next Header: UDP */
/*  33 */ { 1, 0, 0xff00, 1, 0x0100,  35,  0,  34,  0, 0,  0,  0, 0, 0x3f },	/* Next Header: ICMP */
/*  34 */ { 1, 0, 0xff00, 1, 0x3a00,  35,  0,  35,  0, 0,  0,  0, 0, 0x3f },	/* Next Header: ICMPv6 */
/*  35 */ { 1, 1, 0x00fe, 0, 0x0000, 255,  0, 128,  0, 1,  0,  3, 0,   34 },	/* IPv6 Hop Limit != 0 & 1 */
/*  36 */ { 1, 0, 0x0000, 1, 0x0000,  37,  4, 128,  0, 1,  0,  0, 0, 0x3f },	/* IPv4 Total Length */
/*  37 */ { 1, 0, 0x3fff, 1, 0x0000,  38,  2, 128,  0, 0,  0,  0, 0, 0x3f },	/* IP frag */
/*  38 */ { 1, 0, 0xfe00, 0, 0x0000,  39,  0, 128,  0, 1,  0,  1, 0, 0x3f },	/* IPv4 TTL !=0 & 1 */
/*  39 */ { 1, 1, 0x00ff, 1, 0x0006,  44, 24,  40,  0, 0,  0,  0, 0,   12 },	/* TCP */
/*  40 */ { 1, 1, 0x00ff, 1, 0x0011, 255, 24,  41,  0, 0,  0,  0, 0,   12 },	/* UDP */
/*  41 */ { 1, 1, 0x00ff, 1, 0x0001, 255, 24,  42,  0, 0,  0,  0, 0,   12 },	/* ICMP */
/*  42 */ { 1, 1, 0x00ff, 1, 0x002f, 255, 24,  43,  0, 0,  0,  0, 0,   12 },	/* GRE */
/*  43 */ { 1, 1, 0x00ff, 1, 0x0032, 255, 24, 255,  0, 0,  0,  0, 0,   12 },	/* ESP */
/*  44 */ { 1, 1, 0x0007, 1, 0x0000, 255,  0, 128,  0, 0,  0,  0, 0, 0x3f },	/* TCP flags */
/*  45 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  46 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  47 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  48 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  49 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  50 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  51 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  52 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  53 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  54 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  55 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  56 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  57 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  58 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  59 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  60 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  61 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  62 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  63 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  64 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  65 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  66 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  67 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  68 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  69 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  70 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  71 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  72 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  73 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  74 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  75 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  76 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  77 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  78 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  79 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  80 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  81 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  82 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  83 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  84 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  85 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  86 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  87 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  88 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  89 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  90 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  91 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  92 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  93 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  94 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  95 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  96 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  97 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  98 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/*  99 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 100 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 101 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 102 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 103 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 104 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 105 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 106 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 107 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 108 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 109 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 110 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 111 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 112 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 113 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 114 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 115 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 116 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 117 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 118 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 119 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 120 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 121 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 122 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 123 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 124 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 125 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
/* 126 */ { 1, 1, 0x0000, 1, 0x0000, 128,  0, 128,  0, 0,  0,  0, 0,    0 },	/* escape HWNAT */
/* 127 */ { 0, 0, 0x0000, 0, 0x0000,   0,  0,   0,  0, 0,  0,  0, 0,    0 },	/*  */
};

policy_t policy_tbl[POLICY_TBL_SIZE] = {
/*000*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*001*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*002*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*003*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*004*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*005*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*006*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*007*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*008*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*009*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*010*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*011*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*012*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*013*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*014*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*015*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*016*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*017*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*018*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*019*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*020*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*021*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*022*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x4444440c, 0x00000000, 0x00000000, 0x00000000 }, 
/*023*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xcccccc4c, 0x00000000, 0x00000000, 0x00000000 }, 
/*024*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xdddddddc, 0x00000000, 0x00000000, 0x00000000 }, 
/*025*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*026*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xddddddcc, 0x00000000, 0x00000000, 0x00000000 }, 
/*027*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffdc, 0x00000000, 0x00000000, 0x00000000 }, 
/*028*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*029*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*030*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*031*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*032*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*033*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*034*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*035*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffff5c, 0x00000000, 0x00000000, 0x00000000 }, 
/*036*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*037*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*038*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*039*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x500000fc, 0x00000000, 0x00000000, 0x00000000 }, 
/*040*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xf50000fc, 0x00000000, 0x00000000, 0x00000000 }, 
/*041*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff5000fc, 0x00000000, 0x00000000, 0x00000000 }, 
/*042*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfff500fc, 0x00000000, 0x00000000, 0x00000000 }, 
/*043*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffff50fc, 0x00000000, 0x00000000, 0x00000000 }, 
/*044*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*045*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*046*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*047*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*048*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*049*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*050*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*051*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*052*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*053*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*054*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*055*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*056*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*057*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*058*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*059*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*060*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*061*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*062*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*063*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*064*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*065*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*066*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*067*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*068*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*069*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*070*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*071*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*072*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*073*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*074*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*075*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*076*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*077*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*078*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*079*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*080*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*081*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*082*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*083*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*084*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*085*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*086*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*087*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*088*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*089*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*090*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*091*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*092*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*093*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*094*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*095*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*096*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*097*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*098*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*099*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*100*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*101*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*102*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*103*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*104*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*105*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*106*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*107*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*108*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*109*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*110*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*111*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*112*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*113*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*114*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*115*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*116*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*117*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*118*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*119*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*120*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*121*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*122*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*123*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*124*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*125*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }, 
/*126*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffff4, 0x00000000, 0x00000000, 0x00000000 }, 
/*127*/ { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xfffffffc, 0x00000000, 0x00000000, 0x00000000 }
};

policy_result_t policy_result_tbl[POLICY_RESULT_TBL_SIZE] = {
/*	intmap	ti	*/
/*  0 */ { 0xffff,  0 },	/**/
/*  1 */ { 0xffff,  1 },	/**/
/*  2 */ { 0xffff,  0 },	/**/
/*  3 */ { 0xffff,  1 },	/**/
/*  4 */ { 0xffff,  2 },	/**/
/*  5 */ { 0xffff,  3 },	/**/
/*  6 */ { 0xffff,  4 },	/**/
/*  7 */ { 0xffff,  5 },	/**/
/*  8 */ { 0xffff,  6 },	/**/
/*  9 */ { 0xffff,  7 },	/**/
/* 10 */ { 0xffff,  8 },	/**/
/* 11 */ { 0xffff,  9 },	/**/
/* 12 */ { 0xffff,  10 },	/**/
/* 13 */ { 0xffff,  11 },	/**/
/* 14 */ { 0xffff,  12 },	/**/
};

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

void hwnat_pce_init(void)
{
	hwnat_cmpr_init();
	hwnat_policy_init();
	hwnat_policy_result_init();
}

void hwnat_pce_exit(void)
{
}

void hwnat_cmpr_init(void)
{
	uint16 entry;

	for (entry = 0; entry < COMPARATOR_TBL_SIZE; entry++)
		hwnat_cmpr_tbl_write(entry, &comparator_tbl[entry]);
}

void hwnat_cmpr_tbl_write(uint16 entry, comparator_t *cmpr)
{
	uint32 table_hold[GTH_SIZE];

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = (cmpr->l3_l4_offset&0x3f) | ((cmpr->l3_l4<<6)&0x40) | 
		((cmpr->gs_index<<7)&0xf80) | ((cmpr->gs_flag<<12)&0x1000) | ((cmpr->gs_valid<<13)&0x2000) | 
		((cmpr->jump0_offset<<14)&0x1fc000) | ((cmpr->jump0<<21)&0x1fe00000) | 
		((cmpr->jump1_offset<<29)&0xe0000000);
	table_hold[6] = ((cmpr->jump1_offset>>3)&0xf) | ((cmpr->jump1<<4)&0xff0) | 
		((cmpr->value<<12)&0xffff000) | ((cmpr->op<<28)&0x30000000) | ((cmpr->mask<<30)&0xc0000000);
	table_hold[5] = ((cmpr->mask>>2)&0x3fff) | ((cmpr->map<<14)&0x4000) | 
		((cmpr->valid<<15)&0x8000);
	hwnat_write_tbl(COMPARATOR_TBL_ID, entry, table_hold, GTH_SIZE);
}

void hwnat_cmpr_tbl_read(uint16 entry, comparator_t *cmpr)
{
	uint32 table_hold[GTH_SIZE];

	hwnat_read_tbl(COMPARATOR_TBL_ID, entry, table_hold, GTH_SIZE);
	cmpr->l3_l4_offset = table_hold[7]&0x3f;
	cmpr->l3_l4 = (table_hold[7]&0x40)>>6;
	cmpr->gs_index = (table_hold[7]&0xf80)>>7;
	cmpr->gs_flag = (table_hold[7]&0x1000)>>12;
	cmpr->gs_valid = (table_hold[7]&0x2000)>>13;
	cmpr->jump0_offset = (table_hold[7]&0x1fc000)>>14;
	cmpr->jump0 = (table_hold[7]&0x1fe00000)>>21;
	cmpr->jump1_offset = ((table_hold[7]&0xe0000000)>>29) | ((table_hold[6]&0xf)<<3);
	cmpr->jump1 = (table_hold[6]&0xff0)>>4;
	cmpr->value = (table_hold[6]&0xffff000)>>12;
	cmpr->op = (table_hold[6]&0x30000000)>>28;
	cmpr->mask = ((table_hold[6]&0xc0000000)>>30) | ((table_hold[5]&0x3fff)<<2);
	cmpr->map = (table_hold[5]&0x4000)>>14;
	cmpr->valid = (table_hold[5]&0x8000)>>15;
}

void hwnat_policy_init(void)
{
	uint16 entry;

	for (entry = 0; entry < POLICY_TBL_SIZE; entry++)
		hwnat_policy_tbl_write(entry, &policy_tbl[entry]);
}

void hwnat_policy_tbl_write(uint16 entry, policy_t *policy)
{
	uint32 table_hold[GTH_SIZE];
	uint16 i;

	memset(table_hold, 0, sizeof(table_hold));
	for (i = 0; i < GTH_SIZE; i++)
		table_hold[i] = policy->map[i];
	hwnat_write_tbl(POLICY_TBL_ID, entry, table_hold, GTH_SIZE);
}

void hwnat_policy_tbl_read(uint16 entry, policy_t *policy)
{
	uint32 table_hold[GTH_SIZE];
	uint16 i;

	hwnat_read_tbl(POLICY_TBL_ID, entry, table_hold, GTH_SIZE);
	for (i = 0; i < GTH_SIZE; i++)
		policy->map[i] = table_hold[i];
}

void hwnat_policy_result_init(void)
{
	uint16 entry;

	for (entry = 0; entry < POLICY_RESULT_TBL_SIZE; entry++)
		hwnat_policy_result_tbl_write(entry, &policy_result_tbl[entry]);
}

void hwnat_policy_result_tbl_write(uint16 entry, policy_result_t *policy_result)
{
	uint32 table_hold[GTH_SIZE];

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = ((policy_result->tuple_index)&0x7f) | ((policy_result->itf_map<<7)&0x7fff80);
	hwnat_write_tbl(POLICY_RESULT_TBL_ID, entry, table_hold, GTH_SIZE);
}

void hwnat_policy_result_tbl_read(uint16 entry, policy_result_t *policy_result)
{
	uint32 table_hold[GTH_SIZE];

	hwnat_read_tbl(POLICY_RESULT_TBL_ID, entry, table_hold, GTH_SIZE);
	policy_result->tuple_index = table_hold[7]&0x7f;
	policy_result->itf_map = (table_hold[7]&0x7fff80)>>7;
}

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/
static int do_hwnat_cmpr_rd(int argc, char *argv[], void *p);
static int do_hwnat_cmpr_wr(int argc, char *argv[], void *p);

static int do_hwnat_cmpr_tbl(int argc, char *argv[], void *p);
static int do_hwnat_cmpr_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_cmpr_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_cmpr_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_cmpr_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_cmpr_cmds[] = {
	{"rd",			do_hwnat_cmpr_rd,			0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_cmpr_wr,			0x02,  	15,  "<entry> <valid> <map> <mask> <op> <value> <j1> <j1_offset> <j0> <j0_offset> <gs_valid> <gs_flag> <gs_index> <l3_l4> <l3_l4_offset>"},
	{"tbl",			do_hwnat_cmpr_tbl,			0x12,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_cmpr_tbl_cmds[] = {
	{"dump",		do_hwnat_cmpr_tbl_dump,		0x02,  	0,  NULL},
	{"raw",			do_hwnat_cmpr_tbl_raw,		0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_cmpr_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_cmpr_tbl_comp,		0x02,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

int do_hwnat_cmpr(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_cmpr_cmds, argc, argv, p);
}

int do_hwnat_cmpr_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	comparator_t cmpr;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_cmpr_tbl_read(entry, &cmpr);

	printk("/*%03d*/ ", entry);
	printk("%d %d %04x %d %04x %3d %2d %3d %2d %d %d %02d %d %02x\n", 
				cmpr.valid, cmpr.map, cmpr.mask, cmpr.op, cmpr.value, 
				cmpr.jump1, cmpr.jump1_offset, cmpr.jump0, cmpr.jump0_offset, 
				cmpr.gs_valid, cmpr.gs_flag, cmpr.gs_index, cmpr.l3_l4, cmpr.l3_l4_offset);

	return 0;
}

int do_hwnat_cmpr_wr(int argc, char *argv[], void *p)
{
	uint16 entry;
	comparator_t *cmpr;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	cmpr = &comparator_tbl[entry];

	cmpr->valid = (uint8) simple_strtoul(argv[2], NULL, 10);
	cmpr->map = (uint8) simple_strtoul(argv[3], NULL, 10);
	cmpr->mask = (uint16) simple_strtoul(argv[4], NULL, 16);
	cmpr->op = (uint8) simple_strtoul(argv[5], NULL, 10);
	cmpr->value = (uint16) simple_strtoul(argv[6], NULL, 16);
	cmpr->jump1 = (uint16) simple_strtoul(argv[7], NULL, 10);
	cmpr->jump1_offset = (uint16) simple_strtoul(argv[8], NULL, 10);
	cmpr->jump0 = (uint16) simple_strtoul(argv[9], NULL, 10);
	cmpr->jump0_offset = (uint16) simple_strtoul(argv[10], NULL, 10);
	cmpr->gs_valid = (uint8) simple_strtoul(argv[11], NULL, 10);
	cmpr->gs_flag = (uint8) simple_strtoul(argv[12], NULL, 10);
	cmpr->gs_index = (uint8) simple_strtoul(argv[13], NULL, 10);
	cmpr->l3_l4 = (uint8) simple_strtoul(argv[14], NULL, 10);
	cmpr->l3_l4_offset = (uint8) simple_strtoul(argv[15], NULL, 16);
	
	hwnat_cmpr_tbl_write(entry, cmpr);

	return 0;
}

int do_hwnat_cmpr_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_cmpr_tbl_cmds, argc, argv, p);
}

int do_hwnat_cmpr_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;

	printk("valid	map	mask	op	value	J1	J1 Offset	J0	J0 Offset	G/S Valid	G/S	G/S Index	L3/L4	L3/L4 Offset\n");
	for (entry = 0; entry < COMPARATOR_TBL_SIZE; entry++) {
		printk("/*%03d*/ ", entry);
		printk("%d %d %04x %d %04x %3d %2d %3d %2d %d %d %02d %d %02x\n", 
					comparator_tbl[entry].valid, comparator_tbl[entry].map,
					comparator_tbl[entry].mask, comparator_tbl[entry].op, comparator_tbl[entry].value, 
					comparator_tbl[entry].jump1, comparator_tbl[entry].jump1_offset, 
					comparator_tbl[entry].jump0, comparator_tbl[entry].jump0_offset, 
					comparator_tbl[entry].gs_valid, comparator_tbl[entry].gs_flag, 
					comparator_tbl[entry].gs_index,
					comparator_tbl[entry].l3_l4, comparator_tbl[entry].l3_l4_offset);
	}
	return 0;
}

int do_hwnat_cmpr_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	comparator_t *cmpr;
	uint16 entry, i;

	printk("comparator\n");
	for (entry = 0; entry < COMPARATOR_TBL_SIZE; entry++) {
		cmpr = &comparator_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (cmpr->l3_l4_offset&0x3f) | ((cmpr->l3_l4<<6)&0x40) | 
			((cmpr->gs_index<<7)&0xf80) | ((cmpr->gs_flag<<12)&0x1000) | ((cmpr->gs_valid<<13)&0x2000) | 
			((cmpr->jump0_offset<<14)&0x1fc000) | ((cmpr->jump0<<21)&0x1fe00000) | 
			((cmpr->jump1_offset<<29)&0xe0000000);
		table_hold[6] = ((cmpr->jump1_offset>>3)&0xf) | ((cmpr->jump1<<4)&0xff0) | 
			((cmpr->value<<12)&0xffff000) | ((cmpr->op<<28)&0x30000000) | ((cmpr->mask<<30)&0xc0000000);
		table_hold[5] = ((cmpr->mask>>2)&0x3fff) | ((cmpr->map<<14)&0x4000) | 
			((cmpr->valid<<15)&0x8000);

		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_cmpr_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	uint16 entry, i;
	uint8 dump_one = 0;

	if (argc >= 2) {
		entry = simple_strtoul(argv[1], NULL, 10);
		dump_one = 1;
		goto hwraw_dump;
	}

	printk("comparator\n");
	for (entry = 0; entry < COMPARATOR_TBL_SIZE; entry++) {
hwraw_dump:
		hwnat_read_tbl(COMPARATOR_TBL_ID, entry, table_hold, GTH_SIZE);
		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");

		if (dump_one)
			break;
	}
	return 0;
}

int do_hwnat_cmpr_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[GTH_SIZE];
	uint32 table_hold[GTH_SIZE];
	comparator_t *cmpr;
	uint16 entry, i;

	for (entry = 0; entry < COMPARATOR_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(COMPARATOR_TBL_ID, entry, hw_table_hold, GTH_SIZE);

		cmpr = &comparator_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (cmpr->l3_l4_offset&0x3f) | ((cmpr->l3_l4<<6)&0x40) | 
			((cmpr->gs_index<<7)&0xf80) | ((cmpr->gs_flag<<12)&0x1000) | ((cmpr->gs_valid<<13)&0x2000) | 
			((cmpr->jump0_offset<<14)&0x1fc000) | ((cmpr->jump0<<21)&0x1fe00000) | 
			((cmpr->jump1_offset<<29)&0xe0000000);
		table_hold[6] = ((cmpr->jump1_offset>>3)&0xf) | ((cmpr->jump1<<4)&0xff0) | 
			((cmpr->value<<12)&0xffff000) | ((cmpr->op<<28)&0x30000000) | ((cmpr->mask<<30)&0xc0000000);
		table_hold[5] = ((cmpr->mask>>2)&0x3fff) | ((cmpr->map<<14)&0x4000) | 
			((cmpr->valid<<15)&0x8000);

		if (memcmp(hw_table_hold, table_hold, sizeof(hw_table_hold)) != 0) {
			printk("error /*%03d*/\n", entry);
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

static int do_hwnat_policy_rd(int argc, char *argv[], void *p);
static int do_hwnat_policy_wr(int argc, char *argv[], void *p);

static int do_hwnat_policy_tbl(int argc, char *argv[], void *p);
static int do_hwnat_policy_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_policy_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_policy_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_policy_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_policy_cmds[] = {
	{"rd",			do_hwnat_policy_rd,			0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_policy_wr,			0x02,  	5,  "<entry> <map0> <map1> <map2> <map3>"},
	{"tbl",			do_hwnat_policy_tbl,		0x12,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_policy_tbl_cmds[] = {
	{"dump",		do_hwnat_policy_tbl_dump,	0x02,  	0,  NULL},
	{"raw",			do_hwnat_policy_tbl_raw,	0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_policy_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_policy_tbl_comp,	0x02,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

int do_hwnat_policy(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_policy_cmds, argc, argv, p);
}

int do_hwnat_policy_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	policy_t policy;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_policy_tbl_read(entry, &policy);

	printk("/*%03d*/ ", entry);
	printk("%08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n", 
					policy.map[0], policy.map[1], policy.map[2], policy.map[3],
					policy.map[4], policy.map[5], policy.map[6], policy.map[7]);

	return 0;
}

int do_hwnat_policy_wr(int argc, char *argv[], void *p)
{
	uint16 entry;
	policy_t *policy;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	policy = &policy_tbl[entry];

	policy->map[4] = (uint32) simple_strtoul(argv[2], NULL, 16);
	policy->map[5] = (uint32) simple_strtoul(argv[3], NULL, 16);
	policy->map[6] = (uint32) simple_strtoul(argv[4], NULL, 16);
	policy->map[7] = (uint32) simple_strtoul(argv[5], NULL, 16);
	
	hwnat_policy_tbl_write(entry, policy);

	return 0;
}

int do_hwnat_policy_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_policy_tbl_cmds, argc, argv, p);
}

int do_hwnat_policy_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;

	printk("map\n");
	for (entry = 0; entry < POLICY_TBL_SIZE; entry++) {
		printk("/*%03d*/", entry);
		printk("%08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n", 
					policy_tbl[entry].map[0], policy_tbl[entry].map[1],
					policy_tbl[entry].map[2], policy_tbl[entry].map[3],
					policy_tbl[entry].map[4], policy_tbl[entry].map[5],
					policy_tbl[entry].map[6], policy_tbl[entry].map[7]);
	}
	return 0;
}

int do_hwnat_policy_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	policy_t *policy;
	uint16 entry, i;

	printk("policy\n");
	for (entry = 0; entry < POLICY_TBL_SIZE; entry++) {
		policy = &policy_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		for (i = 0; i < GTH_SIZE; i++)
			table_hold[i] = policy->map[i];

		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_policy_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	uint16 entry, i;

	printk("policy\n");
	for (entry = 0; entry < POLICY_TBL_SIZE; entry++) {
		hwnat_read_tbl(POLICY_TBL_ID, entry, table_hold, GTH_SIZE);
		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_policy_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[GTH_SIZE];
	uint32 table_hold[GTH_SIZE];
	policy_t *policy;
	uint16 entry, i;

	for (entry = 0; entry < POLICY_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(POLICY_TBL_ID, entry, hw_table_hold, GTH_SIZE);

		policy = &policy_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		for (i = 0; i < GTH_SIZE; i++)
			table_hold[i] = policy->map[i];

		if (memcmp(hw_table_hold, table_hold, sizeof(hw_table_hold)) != 0) {
			printk("error /*%03d*/\n", entry);
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

static int do_hwnat_policy_result_rd(int argc, char *argv[], void *p);
static int do_hwnat_policy_result_wr(int argc, char *argv[], void *p);

static int do_hwnat_policy_result_tbl(int argc, char *argv[], void *p);
static int do_hwnat_policy_result_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_policy_result_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_policy_result_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_policy_result_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_policy_result_cmds[] = {
	{"rd",			do_hwnat_policy_result_rd,			0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_policy_result_wr,			0x02,  	3,  "<entry> <itf_map> <tuple_index>"},
	{"tbl",			do_hwnat_policy_result_tbl,			0x12,  	0,  NULL},
	{NULL,			NULL,								0x10,	0,	NULL},
};

static const cmds_t hwnat_policy_result_tbl_cmds[] = {
	{"dump",		do_hwnat_policy_result_tbl_dump,	0x02,  	0,  NULL},
	{"raw",			do_hwnat_policy_result_tbl_raw,		0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_policy_result_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_policy_result_tbl_comp,	0x02,  	0,  NULL},
	{NULL,			NULL,								0x10,	0,	NULL},
};

int do_hwnat_policy_result(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_policy_result_cmds, argc, argv, p);
}

int do_hwnat_policy_result_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	policy_result_t policy_result;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_policy_result_tbl_read(entry, &policy_result);

	printk("/*%03d*/ ", entry);
	printk("%04x %0d\n", policy_result.itf_map, policy_result.tuple_index);

	return 0;
}

int do_hwnat_policy_result_wr(int argc, char *argv[], void *p)
{
	uint16 entry;
	policy_result_t *policy_result;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	policy_result = &policy_result_tbl[entry];

	policy_result->itf_map = (uint16) simple_strtoul(argv[2], NULL, 16);
	policy_result->tuple_index = (uint16) simple_strtoul(argv[3], NULL, 10);
	
	hwnat_policy_result_tbl_write(entry, policy_result);

	return 0;
}

int do_hwnat_policy_result_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_policy_result_tbl_cmds, argc, argv, p);
}

int do_hwnat_policy_result_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;

	printk("itfmap ti\n");
	for (entry = 0; entry < POLICY_RESULT_TBL_SIZE; entry++) {
		printk("/*%03d*/", entry);
		printk("%04x %0d\n", 
					policy_result_tbl[entry].itf_map, policy_result_tbl[entry].tuple_index);
	}
	return 0;
}

int do_hwnat_policy_result_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	policy_result_t *policy_result;
	uint16 entry, i;

	printk("policy_result\n");
	for (entry = 0; entry < POLICY_RESULT_TBL_SIZE; entry++) {
		policy_result = &policy_result_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = ((policy_result->tuple_index)&0x7f) | ((policy_result->itf_map<<7)&0x7fff80);

		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_policy_result_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	uint16 entry, i;

	printk("policy_result\n");
	for (entry = 0; entry < POLICY_RESULT_TBL_SIZE; entry++) {
		hwnat_read_tbl(POLICY_RESULT_TBL_ID, entry, table_hold, GTH_SIZE);
		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_policy_result_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[GTH_SIZE];
	uint32 table_hold[GTH_SIZE];
	policy_result_t *policy_result;
	uint16 entry, i;

	for (entry = 0; entry < POLICY_RESULT_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(POLICY_RESULT_TBL_ID, entry, hw_table_hold, GTH_SIZE);

		policy_result = &policy_result_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = ((policy_result->tuple_index)&0x7f) | ((policy_result->itf_map<<7)&0x7fff80);

		if (memcmp(hw_table_hold, table_hold, sizeof(hw_table_hold)) != 0) {
			printk("error /*%03d*/\n", entry);
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

