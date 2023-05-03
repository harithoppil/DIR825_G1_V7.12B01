/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat_mde.c#1 $
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
** $Log: hwnat_mde.c,v $
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
#include <asm/io.h>
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

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/

typedef struct {
	uint16 value;
} const_val_t;

typedef struct {
	uint8 m;
	uint8 op_vec;
} mod_macro_t;

typedef struct {
	uint8 m;
	uint8 lsel;
	uint8 op_index;
} mod_vec_t;

typedef struct {
	uint8 status_index;
	uint8 m;
	uint8 op;
	uint8 src_pkt_loc;
	/* checksum */
	uint8 chksum_add;
	uint8 chksum_sub;
	uint8 chksum_reset;
	uint8 conti;
	uint8 data_src;
	uint8 data_idx;
	uint8 data_len;
	uint8 ext_op_e;
	uint16 ext_op_data;
} mod_op_t;

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static void hwnat_const_init(void);
static void hwnat_const_tbl_write(uint16 entry, const_val_t *const_val);
static void hwnat_const_tbl_read(uint16 entry, const_val_t *const_val);

static void hwnat_mod_macro_init(void);
static void hwnat_mod_macro_tbl_write(uint16 entry, mod_macro_t *mod_macro);
static void hwnat_mod_macro_tbl_read(uint16 entry, mod_macro_t *mod_macro);

static void hwnat_mod_vec_init(void);
static void hwnat_mod_vec_tbl_write(uint16 entry, mod_vec_t *mod_vec);
static void hwnat_mod_vec_tbl_read(uint16 entry, mod_vec_t *mod_vec);

static void hwnat_mod_op_init(void);
static void hwnat_mod_op_tbl_write(uint16 entry, mod_op_t *mod_op);
static void hwnat_mod_op_tbl_read(uint16 entry, mod_op_t *mod_op);

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

const_val_t const_tbl[CONST_TBL_SIZE] = {
/*      value   */
/*  0 */ { 0x8864 },    /* PPPoE Header */
/*  1 */ { 0x1100 },    /* */
/*  2 */ { 0x0021 },    /* PPP IPv4 */
/*  3 */ { 0x0057 },    /* PPP IPv6 */
/*  4 */ { 0x0800 },    /* IPv4 */
/*  5 */ { 0x86dd },    /* IPv6 */
/*  6 */ { 0x8100 },    /* VLAN */
/*  7 */ { 0x8901 },    /* TC2206 tag */
/*  8 */ { 0x0601 },    /* */
/*  9 */ { 0x0300 },    /* TC2206 stag */
/* 10 */ { 0xAAAA },    /* RFC1483 Bridge LLC */
/* 11 */ { 0x0300 },    /* */
/* 12 */ { 0x80c2 },    /* */
/* 13 */ { 0x0001 },    /* */
/* 14 */ { 0x0000 },    /* RFC1483 Bridge VCMux */
/* 15 */ { 0xAAAA },    /* RFC1483 Route LLC */
/* 16 */ { 0x0300 },    /* */
/* 17 */ { 0x0000 },    /* */
/* 18 */ { 0xFEFE },    /* PPPoA LLC */
/* 19 */ { 0x03CF },    /* */
/* 20 */ { 0x8100 },	/* outer VLAN	*/
};

mod_macro_t mod_macro_tbl[MOD_MACRO_TBL_SIZE] = {
/*	m	op vec */
/*   0 */ { 0,   0 },	/* NOP	*/
/*   1 */ { 0,   1 },	/* Bridge Mode	*/
/*   2 */ { 0,   4 },	/* VLAN + Bridge Mode	*/
/*   3 */ { 0,   8 },	/* TC2206 Bridge Mode	*/
/*   4 */ { 0,  12 },	/* TC2206 + VLAN Bridge Mode	*/
/*   5 */ { 1,  17 },	/* LAN->WAN TCP NAT	*/
/*   6 */ { 1, 103 },	/* 	*/
/*   7 */ { 0, 110 },	/* 	*/
/*   8 */ { 1,  17 },	/* LAN->WAN UDP NAT	*/
/*   9 */ { 1, 103 },	/* 	*/
/*  10 */ { 0, 111 },	/* 	*/
/*  11 */ { 1,  17 },	/* LAN->WAN ICMP NAT	*/
/*  12 */ { 1, 103 },	/* 	*/
/*  13 */ { 0, 114 },	/* 	*/
/*  14 */ { 1,  17 },	/* WAN->LAN TCP NAT	*/
/*  15 */ { 1, 105 },	/* 	*/
/*  16 */ { 0, 112 },	/* 	*/
/*  17 */ { 1,  17 },	/* WAN->LAN UDP NAT	*/
/*  18 */ { 1, 105 },	/* 	*/
/*  19 */ { 0, 113 },	/* 	*/
/*  20 */ { 1,  17 },	/* WAN->LAN ICMP NAT	*/
/*  21 */ { 1, 105 },	/* 	*/
/*  22 */ { 0, 114 },	/* 	*/
/*  23 */ { 1,  20 },	/* LAN->WAN VLAN + TCP NAT	*/
/*  24 */ { 1, 103 },	/* 	*/
/*  25 */ { 0, 110 },	/* 	*/
/*  26 */ { 1,  20 },	/* LAN->WAN VLAN + UDP NAT	*/
/*  27 */ { 1, 103 },	/* 	*/
/*  28 */ { 0, 111 },	/* 	*/
/*  29 */ { 1,  20 },	/* LAN->WAN VLAN + ICMP NAT	*/
/*  30 */ { 1, 103 },	/* 	*/
/*  31 */ { 0, 114 },	/* 	*/
/*  32 */ { 1,  20 },	/* WAN->LAN VLAN + TCP NAT	*/
/*  33 */ { 1, 105 },	/* 	*/
/*  34 */ { 0, 112 },	/* 	*/
/*  35 */ { 1,  20 },	/* WAN->LAN VLAN + UDP NAT	*/
/*  36 */ { 1, 105 },	/* 	*/
/*  37 */ { 0, 113 },	/* 	*/
/*  38 */ { 1,  20 },	/* WAN->LAN VLAN + ICMP NAT	*/
/*  39 */ { 1, 105 },	/* 	*/
/*  40 */ { 0, 114 },	/* 	*/
/*  41 */ { 1,  24 },	/* LAN->WAN PPPoE + TCP NAT	*/
/*  42 */ { 1, 103 },	/* 	*/
/*  43 */ { 0, 110 },	/* 	*/
/*  44 */ { 1,  24 },	/* LAN->WAN PPPoE + UDP NAT	*/
/*  45 */ { 1, 103 },	/* 	*/
/*  46 */ { 0, 111 },	/* 	*/
/*  47 */ { 1,  24 },	/* LAN->WAN PPPoE + ICMP NAT	*/
/*  48 */ { 1, 103 },	/* 	*/
/*  49 */ { 0, 114 },	/* 	*/
/*  50 */ { 1,  29 },	/* LAN->WAN VLAN + PPPoE + TCP NAT	*/
/*  51 */ { 1, 103 },	/* 	*/
/*  52 */ { 0, 110 },	/* 	*/
/*  53 */ { 1,  29 },	/* LAN->WAN VLAN + PPPoE + UDP NAT	*/
/*  54 */ { 1, 103 },	/* 	*/
/*  55 */ { 0, 111 },	/* 	*/
/*  56 */ { 1,  29 },	/* LAN->WAN VLAN + PPPoE + ICMP NAT	*/
/*  57 */ { 1, 103 },	/* 	*/
/*  58 */ { 0, 114 },	/* 	*/
/*  59 */ { 1,  35 },	/* WAN->LAN TC2206 + TCP NAT	*/
/*  60 */ { 1, 105 },	/* 	*/
/*  61 */ { 0, 112 },	/* 	*/
/*  62 */ { 1,  35 },	/* WAN->LAN TC2206 + UDP NAT	*/
/*  63 */ { 1, 105 },	/* 	*/
/*  64 */ { 0, 113 },	/* 	*/
/*  65 */ { 1,  35 },	/* WAN->LAN TC2206 + ICMP NAT	*/
/*  66 */ { 1, 105 },	/* 	*/
/*  67 */ { 0, 114 },	/* 	*/
/*  68 */ { 1,  39 },	/* WAN->LAN TC2206 + VLAN + TCP NAT	*/
/*  69 */ { 1, 105 },	/* 	*/
/*  70 */ { 0, 112 },	/* 	*/
/*  71 */ { 1,  39 },	/* WAN->LAN TC2206 + VLAN + UDP NAT	*/
/*  72 */ { 1, 105 },	/* 	*/
/*  73 */ { 0, 113 },	/* 	*/
/*  74 */ { 1,  39 },	/* WAN->LAN TC2206 + VLAN + ICMP NAT	*/
/*  75 */ { 1, 105 },	/* 	*/
/*  76 */ { 0, 114 },	/* 	*/
/*  77 */ { 1,  44 },	/* WAN->LAN PPPoE Decap + TCP NAT	*/
/*  78 */ { 1, 105 },	/* 	*/
/*  79 */ { 0, 112 },	/* 	*/
/*  80 */ { 1,  44 },	/* WAN->LAN PPPoE Decap + UDP NAT	*/
/*  81 */ { 1, 105 },	/* 	*/
/*  82 */ { 0, 113 },	/* 	*/
/*  83 */ { 1,  44 },	/* WAN->LAN PPPoE Decap + ICMP NAT	*/
/*  84 */ { 1, 105 },	/* 	*/
/*  85 */ { 0, 114 },	/* 	*/
/*  86 */ { 1,  49 },	/* WAN->LAN PPPoE Decap + VLAN + TCP NAT	*/
/*  87 */ { 1, 105 },	/* 	*/
/*  88 */ { 0, 112 },	/* 	*/
/*  89 */ { 1,  49 },	/* WAN->LAN PPPoE Decap + VLAN + UDP NAT	*/
/*  90 */ { 1, 105 },	/* 	*/
/*  91 */ { 0, 113 },	/* 	*/
/*  92 */ { 1,  49 },	/* WAN->LAN PPPoE Decap + VLAN + ICMP NAT	*/
/*  93 */ { 1, 105 },	/* 	*/
/*  94 */ { 0, 114 },	/* 	*/
/*  95 */ { 1,  55 },	/* WAN->LAN PPPoE Decap + TC2206 + TCP NAT	*/
/*  96 */ { 1, 105 },	/* 	*/
/*  97 */ { 0, 112 },	/* 	*/
/*  98 */ { 1,  55 },	/* WAN->LAN PPPoE Decap + TC2206 + UDP NAT	*/
/*  99 */ { 1, 105 },	/* 	*/
/* 100 */ { 0, 113 },	/* 	*/
/* 101 */ { 1,  55 },	/* WAN->LAN PPPoE Decap + TC2206 + ICMP NAT	*/
/* 102 */ { 1, 105 },	/* 	*/
/* 103 */ { 0, 114 },	/* 	*/
/* 104 */ { 1,  61 },	/* WAN->LAN PPPoE Decap + TC2206 + VLAN + TCP NAT	*/
/* 105 */ { 1, 105 },	/* 	*/
/* 106 */ { 0, 112 },	/* 	*/
/* 107 */ { 1,  61 },	/* WAN->LAN PPPoE Decap + TC2206 + VLAN + UDP NAT	*/
/* 108 */ { 1, 105 },	/* 	*/
/* 109 */ { 0, 113 },	/* 	*/
/* 110 */ { 1,  61 },	/* WAN->LAN PPPoE Decap + TC2206 + VLAN + ICMP NAT	*/
/* 111 */ { 1, 105 },	/* 	*/
/* 112 */ { 0, 114 },	/* 	*/
/* 113 */ { 1,  17 },	/* IPv4 Routing	*/
/* 114 */ { 0, 107 },	/* 	*/
/* 115 */ { 1,  20 },	/* VLAN + IPv4 Routing	*/
/* 116 */ { 0, 107 },	/* 	*/
/* 117 */ { 1,  24 },	/* PPPoE + IPv4 Routing	*/
/* 118 */ { 0, 107 },	/* 	*/
/* 119 */ { 1,  29 },	/* VLAN + PPPoE + IPv4 Routing	*/
/* 120 */ { 0, 107 },	/* 	*/
/* 121 */ { 1,  35 },	/* TC2206 + IPv4 Routing	*/
/* 122 */ { 0, 107 },	/* 	*/
/* 123 */ { 1,  39 },	/* TC2206 + VLAN + IPv4 Routing	*/
/* 124 */ { 0, 107 },	/* 	*/
/* 125 */ { 1,  44 },	/* PPPoE Decap + IPv4 Routing	*/
/* 126 */ { 0, 107 },	/* 	*/
/* 127 */ { 1,  49 },	/* PPPoE Decap + VLAN + IPv4 Routing	*/
/* 128 */ { 0, 107 },	/* 	*/
/* 129 */ { 1,  55 },	/* PPPoE Decap + TC2206 + IPv4 Routing	*/
/* 130 */ { 0, 107 },	/* 	*/
/* 131 */ { 1,  61 },	/* PPPoE Decap + TC2206 + VLAN + IPv4 Routing	*/
/* 132 */ { 0, 107 },	/* 	*/
/* 133 */ { 1,  17 },	/* IPv6 Routing	*/
/* 134 */ { 0, 109 },	/* 	*/
/* 135 */ { 1,  20 },	/* VLAN + IPv6 Routing	*/
/* 136 */ { 0, 109 },	/* 	*/
/* 137 */ { 1,  68 },	/* PPPoE + IPv6 Routing	*/
/* 138 */ { 0, 109 },	/* 	*/
/* 139 */ { 1,  73 },	/* VLAN + PPPoE + IPv6 Routing	*/
/* 140 */ { 0, 109 },	/* 	*/
/* 141 */ { 1,  35 },	/* TC2206 + IPv6 Routing	*/
/* 142 */ { 0, 109 },	/* 	*/
/* 143 */ { 1,  39 },	/* TC2206 + VLAN + IPv6 Routing	*/
/* 144 */ { 0, 109 },	/* 	*/
/* 145 */ { 1,  79 },	/* PPPoE Decap + IPv6 Routing	*/
/* 146 */ { 0, 109 },	/* 	*/
/* 147 */ { 1,  84 },	/* PPPoE Decap + VLAN + IPv6 Routing	*/
/* 148 */ { 0, 109 },	/* 	*/
/* 149 */ { 1,  90 },	/* PPPoE Decap + TC2206 + IPv6 Routing	*/
/* 150 */ { 0, 109 },	/* 	*/
/* 151 */ { 1,  96 },	/* PPPoE Decap + TC2206 + VLAN + IPv6 Routing	*/
/* 152 */ { 0, 109 },	/* 	*/
/* 153 */ { 0, 115 },	/* QinQ Bridge Mode	*/
/* 154 */ { 0, 119 },	/* TC2206 + QinQ Bridge Mode	*/
/* 155 */ { 1, 124 },	/* LAN->WAN VLAN + TCP NAT	*/
/* 156 */ { 1, 103 },	/* 	*/
/* 157 */ { 0, 110 },	/* 	*/
/* 158 */ { 1, 124 },	/* LAN->WAN VLAN + UDP NAT	*/
/* 159 */ { 1, 103 },	/* 	*/
/* 160 */ { 0, 111 },	/* 	*/
/* 161 */ { 1, 124 },	/* LAN->WAN VLAN + ICMP NAT	*/
/* 162 */ { 1, 103 },	/* 	*/
/* 163 */ { 0, 114 },	/* 	*/
/* 164 */ { 1, 124 },	/* WAN->LAN VLAN + TCP NAT	*/
/* 165 */ { 1, 105 },	/* 	*/
/* 166 */ { 0, 112 },	/* 	*/
/* 167 */ { 1, 124 },	/* WAN->LAN VLAN + UDP NAT	*/
/* 168 */ { 1, 105 },	/* 	*/
/* 169 */ { 0, 113 },	/* 	*/
/* 170 */ { 1, 124 },	/* WAN->LAN VLAN + ICMP NAT	*/
/* 171 */ { 1, 105 },	/* 	*/
/* 172 */ { 0, 114 },	/* 	*/
/* 173 */ { 1, 128 },	/* LAN->WAN VLAN + PPPoE + TCP NAT	*/
/* 174 */ { 1, 103 },	/* 	*/
/* 175 */ { 0, 110 },	/* 	*/
/* 176 */ { 1, 128 },	/* LAN->WAN VLAN + PPPoE + UDP NAT	*/
/* 177 */ { 1, 103 },	/* 	*/
/* 178 */ { 0, 111 },	/* 	*/
/* 179 */ { 1, 128 },	/* LAN->WAN VLAN + PPPoE + UDP NAT	*/
/* 180 */ { 1, 103 },	/* 	*/
/* 181 */ { 0, 114 },	/* 	*/
/* 182 */ { 1, 134 },	/* WAN->LAN TC2206 + VLAN + TCP NAT	*/
/* 183 */ { 1, 105 },	/* 	*/
/* 184 */ { 0, 112 },	/* 	*/
/* 185 */ { 1, 134 },	/* WAN->LAN TC2206 + VLAN + UDP NAT	*/
/* 186 */ { 1, 105 },	/* 	*/
/* 187 */ { 0, 113 },	/* 	*/
/* 188 */ { 1, 134 },	/* WAN->LAN TC2206 + VLAN + ICMP NAT	*/
/* 189 */ { 1, 105 },	/* 	*/
/* 190 */ { 0, 114 },	/* 	*/
/* 191 */ { 1, 139 },	/* WAN->LAN PPPoE Decap + VLAN + TCP NAT	*/
/* 192 */ { 1, 105 },	/* 	*/
/* 193 */ { 0, 112 },	/* 	*/
/* 194 */ { 1, 139 },	/* WAN->LAN PPPoE Decap + VLAN + UDP NAT	*/
/* 195 */ { 1, 105 },	/* 	*/
/* 196 */ { 0, 113 },	/* 	*/
/* 197 */ { 1, 139 },	/* WAN->LAN PPPoE Decap + VLAN + ICMP NAT	*/
/* 198 */ { 1, 105 },	/* 	*/
/* 199 */ { 0, 114 },	/* 	*/
/* 200 */ { 1, 145 },	/* WAN->LAN PPPoE Decap + TC2206 + VLAN + TCP NAT	*/
/* 201 */ { 1, 105 },	/* 	*/
/* 202 */ { 0, 112 },	/* 	*/
/* 203 */ { 1, 145 },	/* WAN->LAN PPPoE Decap + TC2206 + VLAN + UDP NAT	*/
/* 204 */ { 1, 105 },	/* 	*/
/* 205 */ { 0, 113 },	/* 	*/
/* 206 */ { 1, 145 },	/* WAN->LAN PPPoE Decap + TC2206 + VLAN + ICMP NAT	*/
/* 207 */ { 1, 105 },	/* 	*/
/* 208 */ { 0, 114 },	/* 	*/
/* 209 */ { 1, 124 },	/* VLAN + IPv4 Routing	*/
/* 210 */ { 0, 107 },	/* 	*/
/* 211 */ { 1, 128 },	/* VLAN + PPPoE + IPv4 Routing	*/
/* 212 */ { 0, 107 },	/* 	*/
/* 213 */ { 1, 134 },	/* TC2206 + VLAN + IPv4 Routing	*/
/* 214 */ { 0, 107 },	/* 	*/
/* 215 */ { 1, 139 },	/* PPPoE Decap + VLAN + IPv4 Routing	*/
/* 216 */ { 0, 107 },	/* 	*/
/* 217 */ { 1, 145 },	/* PPPoE Decap + TC2206 + VLAN + IPv4 Routing	*/
/* 218 */ { 0, 107 },	/* 	*/
/* 219 */ { 1, 124 },	/* VLAN + IPv6 Routing	*/
/* 220 */ { 0, 109 },	/* 	*/
/* 221 */ { 1, 152 },	/* VLAN + PPPoE + IPv6 Routing	*/
/* 222 */ { 0, 109 },	/* 	*/
/* 223 */ { 1, 134 },	/* TC2206 + VLAN + IPv6 Routing	*/
/* 224 */ { 0, 109 },	/* 	*/
/* 225 */ { 1, 139 },	/* PPPoE Decap + VLAN + IPv6 Routing	*/
/* 226 */ { 0, 109 },	/* 	*/
/* 227 */ { 1, 164 },	/* PPPoE Decap + TC2206 + VLAN + IPv6 Routing	*/
/* 228 */ { 0, 109 },	/* 	*/
};

mod_vec_t mod_vec_tbl[MOD_VEC_TBL_SIZE] = {
/*	m	lsel	op vec */
/*   0 */ { 0, 0,   0 },	/* NOP	NOP	*/
/*   1 */ { 1, 0,   4 },	/* TC2206 Decap	Bridge Mode	*/
/*   2 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*   3 */ { 0, 1,  44 },	/* Qos Remarking		*/
/*   4 */ { 1, 0,   4 },	/* TC2206 Decap	VLAN + Bridge Mode	*/
/*   5 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*   6 */ { 1, 0,   5 },	/* VLAN Encap		*/
/*   7 */ { 0, 1,  44 },	/* Qos Remarking		*/
/*   8 */ { 1, 0,   4 },	/* TC2206 Decap	TC2206 Bridge Mode	*/
/*   9 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  10 */ { 1, 0,   1 },	/* TC2206 Encap		*/
/*  11 */ { 0, 1,  44 },	/* Qos Remarking		*/
/*  12 */ { 1, 0,   4 },	/* TC2206 Decap	TC2206 + VLAN Bridge Mode	*/
/*  13 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  14 */ { 1, 0,   1 },	/* TC2206 Encap		*/
/*  15 */ { 1, 0,   5 },	/* VLAN Encap		*/
/*  16 */ { 0, 1,  44 },	/* Qos Remarking		*/
/*  17 */ { 1, 0,  42 },	/* MAC DA/SA	Normal	*/
/*  18 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  19 */ { 0, 0,  11 },	/* VLAN Decap		*/
/*  20 */ { 1, 0,  42 },	/* MAC DA/SA	VLAN Encap	*/
/*  21 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  22 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  23 */ { 0, 0,   5 },	/* VLAN Encap		*/
/*  24 */ { 1, 0,  42 },	/* MAC DA/SA	PPPoE Encap	*/
/*  25 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  26 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  27 */ { 1, 0,  13 },	/* PPPoE Encap		*/
/*  28 */ { 0, 0,  16 },	/* IPv4 PPP		*/
/*  29 */ { 1, 0,  42 },	/* MAC DA/SA	VLAN + PPPoE Encap	*/
/*  30 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  31 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  32 */ { 1, 0,   5 },	/* VLAN Encap		*/
/*  33 */ { 1, 0,  13 },	/* PPPoE Encap		*/
/*  34 */ { 0, 0,  16 },	/* IPv4 PPP		*/
/*  35 */ { 1, 0,  42 },	/* MAC DA/SA	TC2206 Encap	*/
/*  36 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  37 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  38 */ { 0, 0,   1 },	/* TC2206 Encap		*/
/*  39 */ { 1, 0,  42 },	/* MAC DA/SA	TC2206 + VLAN Encap	*/
/*  40 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  41 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  42 */ { 1, 0,   1 },	/* VLAN Encap		*/
/*  43 */ { 0, 0,   5 },	/* TC2206 Encap		*/
/*  44 */ { 1, 0,  42 },	/* MAC DA/SA	PPPoE Decap	*/
/*  45 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  46 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  47 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/*  48 */ { 0, 0,  21 },	/* IPv4 EtherType		*/
/*  49 */ { 1, 0,  42 },	/* MAC DA/SA	PPPoE Decap + VLAN	*/
/*  50 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  51 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  52 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/*  53 */ { 1, 0,   5 },	/* VLAN Encap		*/
/*  54 */ { 0, 0,  21 },	/* IPv4 EtherType		*/
/*  55 */ { 1, 0,  42 },	/* MAC DA/SA	PPPoE Decap + TC2206	*/
/*  56 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  57 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  58 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/*  59 */ { 1, 0,   1 },	/* TC2206 Encap		*/
/*  60 */ { 0, 0,  21 },	/* IPv4 EtherType		*/
/*  61 */ { 1, 0,  42 },	/* MAC DA/SA	PPPoE Decap + TC2206 + VLAN	*/
/*  62 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  63 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  64 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/*  65 */ { 1, 0,   1 },	/* TC2206 Encap		*/
/*  66 */ { 1, 0,   5 },	/* VLAN Encap		*/
/*  67 */ { 0, 0,  21 },	/* IPv4 EtherType		*/
/*  68 */ { 1, 0,  42 },	/* MAC DA/SA	IPv6 PPPoE Encap	*/
/*  69 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  70 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  71 */ { 1, 0,  13 },	/* PPPoE Encap		*/
/*  72 */ { 0, 0,  18 },	/* IPv6 PPP		*/
/*  73 */ { 1, 0,  42 },	/* MAC DA/SA	VLAN + IPv6 PPPoE Encap	*/
/*  74 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  75 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  76 */ { 1, 0,   5 },	/* VLAN Encap		*/
/*  77 */ { 1, 0,  13 },	/* PPPoE Encap		*/
/*  78 */ { 0, 0,  18 },	/* IPv6 PPP		*/
/*  79 */ { 1, 0,  42 },	/* MAC DA/SA	IPv6 PPPoE Decap	*/
/*  80 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  81 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  82 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/*  83 */ { 0, 0,  22 },	/* IPv6 EtherType		*/
/*  84 */ { 1, 0,  42 },	/* MAC DA/SA	IPv6  PPPoE Decap + VLAN	*/
/*  85 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  86 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  87 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/*  88 */ { 1, 0,   5 },	/* VLAN Encap		*/
/*  89 */ { 0, 0,  22 },	/* IPv6 EtherType		*/
/*  90 */ { 1, 0,  42 },	/* MAC DA/SA	IPv6 PPPoE Decap + TC2206	*/
/*  91 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  92 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  93 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/*  94 */ { 1, 0,   1 },	/* TC2206 Encap		*/
/*  95 */ { 0, 0,  22 },	/* IPv6 EtherType		*/
/*  96 */ { 1, 0,  42 },	/* MAC DA/SA	IPv6 PPPoE Decap + TC2206 + VLAN	*/
/*  97 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/*  98 */ { 1, 0,  11 },	/* VLAN Decap		*/
/*  99 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/* 100 */ { 1, 0,   1 },	/* TC2206 Encap		*/
/* 101 */ { 1, 0,   5 },	/* VLAN Encap		*/
/* 102 */ { 0, 0,  22 },	/* IPv6 EtherType		*/
/* 103 */ { 1, 1,  23 },	/* TTL--	LAN->WAN	*/
/* 104 */ { 0, 1,  27 },	/* IPv4 SIP		*/
/* 105 */ { 1, 1,  23 },	/* TTL--	WAN->LAN	*/
/* 106 */ { 0, 1,  33 },	/* IPv4 DIP		*/
/* 107 */ { 1, 1,  23 },	/* TTL--	IPv4 Routing	*/
/* 108 */ { 0, 1,  39 },	/* IPv4 Chksum		*/
/* 109 */ { 0, 1,  40 },	/* HL--	IPv6 Routing	*/
/* 110 */ { 0, 2,  29 },	/* SPORT	TCP SPORT	*/
/* 111 */ { 0, 2,  31 },	/* SPORT	UDP SPORT	*/
/* 112 */ { 0, 2,  35 },	/* DPORT	TCP DPORT	*/
/* 113 */ { 0, 2,  37 },	/* DPORT	UDP DPORT	*/
/* 114 */ { 0, 2,  25 },	/* ICMP	ICMP	*/
/* 115 */ { 1, 0,   4 },	/* TC2206 Decap	QinQ Bridge Mode	*/
/* 116 */ { 1, 0,  11 },	/* VLAN Decap		*/
/* 117 */ { 1, 0,   7 },	/* VLAN Encap		*/
/* 118 */ { 0, 1,  44 },	/* Qos Remarking		*/
/* 119 */ { 1, 0,   4 },	/* TC2206 Decap	TC2206 + QinQ Bridge Mode	*/
/* 120 */ { 1, 0,  11 },	/* VLAN Decap		*/
/* 121 */ { 1, 0,   1 },	/* TC2206 Encap		*/
/* 122 */ { 1, 0,   7 },	/* VLAN Encap		*/
/* 123 */ { 0, 1,  44 },	/* Qos Remarking		*/
/* 124 */ { 1, 0,  42 },	/* MAC DA/SA	VLAN Encap	*/
/* 125 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/* 126 */ { 1, 0,  11 },	/* VLAN Decap		*/
/* 127 */ { 0, 0,   7 },	/* VLAN Encap		*/
/* 128 */ { 1, 0,  42 },	/* MAC DA/SA	VLAN + PPPoE Encap	*/
/* 129 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/* 130 */ { 1, 0,  11 },	/* VLAN Decap		*/
/* 131 */ { 1, 0,   7 },	/* VLAN Encap		*/
/* 132 */ { 1, 0,  13 },	/* PPPoE Encap		*/
/* 133 */ { 0, 0,  16 },	/* IPv4 PPP		*/
/* 134 */ { 1, 0,  42 },	/* MAC DA/SA	TC2206 + VLAN Encap	*/
/* 135 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/* 136 */ { 1, 0,  11 },	/* VLAN Decap		*/
/* 137 */ { 1, 0,   1 },	/* TC2206 Encap		*/
/* 138 */ { 0, 0,   7 },	/* VLAN Encap		*/
/* 139 */ { 1, 0,  42 },	/* MAC DA/SA	PPPoE Decap + VLAN	*/
/* 140 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/* 141 */ { 1, 0,  11 },	/* VLAN Decap		*/
/* 142 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/* 143 */ { 1, 0,   7 },	/* VLAN Encap		*/
/* 144 */ { 0, 0,  21 },	/* IPv4 EtherType		*/
/* 145 */ { 1, 0,  42 },	/* MAC DA/SA	PPPoE Decap + TC2206 + VLAN	*/
/* 146 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/* 147 */ { 1, 0,  11 },	/* VLAN Decap		*/
/* 148 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/* 149 */ { 1, 0,   1 },	/* TC2206 Encap		*/
/* 150 */ { 1, 0,   7 },	/* VLAN Encap		*/
/* 151 */ { 0, 0,  21 },	/* IPv4 EtherType		*/
/* 152 */ { 1, 0,  42 },	/* MAC DA/SA	VLAN + IPv6 PPPoE Encap	*/
/* 153 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/* 154 */ { 1, 0,  11 },	/* VLAN Decap		*/
/* 155 */ { 1, 0,   7 },	/* VLAN Encap		*/
/* 156 */ { 1, 0,  13 },	/* PPPoE Encap		*/
/* 157 */ { 0, 0,  18 },	/* IPv6 PPP		*/
/* 158 */ { 1, 0,  42 },	/* MAC DA/SA	IPv6  PPPoE Decap + VLAN	*/
/* 159 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/* 160 */ { 1, 0,  11 },	/* VLAN Decap		*/
/* 161 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/* 162 */ { 1, 0,   7 },	/* VLAN Encap		*/
/* 163 */ { 0, 0,  22 },	/* IPv6 EtherType		*/
/* 164 */ { 1, 0,  42 },	/* MAC DA/SA	IPv6 PPPoE Decap + TC2206 + VLAN	*/
/* 165 */ { 1, 0,   4 },	/* TC2206 Decap		*/
/* 166 */ { 1, 0,  11 },	/* VLAN Decap		*/
/* 167 */ { 1, 0,  20 },	/* PPPoE Decap		*/
/* 168 */ { 1, 0,   1 },	/* TC2206 Encap		*/
/* 169 */ { 1, 0,   7 },	/* VLAN Encap		*/
/* 170 */ { 0, 0,  22 },	/* IPv6 EtherType		*/
};

mod_op_t mod_op_tbl[MOD_OP_TBL_SIZE] = {
/*	Status	m	op	src pkt	a	s	r	c	src	idx	len	e	opdata */
/*   0 */ { 31, 0, 3,  0, 0, 0, 0, 0,  7,  0,  0, 0, 0 },	/* nop	*/
/*   1 */ { 31, 1, 0, 12, 0, 0, 0, 0,  2, 14,  4, 0, 0 },	/* TC2206 Tag	*/
/*   2 */ { 31, 1, 0, 12, 0, 0, 0, 0,  0, 14,  1, 0, 0 },	/* TC2206 egress portmap	*/
/*   3 */ { 31, 0, 0, 12, 0, 0, 0, 0,  2, 18,  1, 0, 0 },	/* TC2206 STag	*/
/*   4 */ {  0, 0, 1, 12, 0, 0, 0, 0,  7,  0,  8, 0, 0 },	/* Remove TC2206 Tag	*/
/*   5 */ { 31, 1, 0, 12, 0, 0, 0, 0,  2, 12,  2, 0, 0 },	/* Insert VLAN (0x8100)	*/
/*   6 */ { 31, 0, 0, 12, 0, 0, 0, 1,  0,  8,  2, 4, 0x7ff },	/* Insert VLAN ID	*/
/*   7 */ { 31, 1, 0, 12, 0, 0, 0, 0,  2, 40,  2, 0, 0 },	/* Insert outer VLAN (0x8100)	*/
/*   8 */ { 31, 1, 0, 12, 0, 0, 0, 1,  0, 12,  2, 4, 0x7ff },	/* Insert outer VLAN ID	*/
/*   9 */ { 31, 1, 0, 12, 0, 0, 0, 0,  2, 12,  2, 0, 0 },	/* Insert inner VLAN (0x8100)	*/
/*  10 */ { 31, 0, 0, 12, 0, 0, 0, 0,  0,  8,  2, 4, 0x7ff },	/* Insert inner VLAN ID	*/
/*  11 */ {  1, 1, 1, 12, 0, 0, 0, 0,  7,  0,  4, 0, 0 },	/* Remove outer VLAN	*/
/*  12 */ {  2, 0, 1, 12, 0, 0, 0, 0,  7,  0,  4, 0, 0 },	/* Remove inner VLAN	*/
/*  13 */ { 31, 1, 1, 12, 0, 0, 0, 0,  7,  0,  2, 0, 0 },	/* Remove EtherType	*/
/*  14 */ { 31, 1, 0, 12, 0, 0, 0, 0,  2,  0,  4, 0, 0 },	/* PPPoE Header	*/
/*  15 */ { 31, 0, 0, 12, 0, 0, 0, 0,  0, 10,  2, 0, 0 },	/* PPPoE Session ID	*/
/*  16 */ { 31, 1, 0, 12, 0, 0, 0, 1,  5,  0,  2, 0, 2 },	/* "PPPoE Payload Length (= IP Total Length + 2 (0x0021))"	*/
/*  17 */ { 31, 0, 0, 12, 0, 0, 0, 0,  2,  4,  2, 0, 0 },	/* IPv4 PPP	*/
/*  18 */ { 31, 1, 0, 12, 0, 0, 0, 1,  5,  4,  2, 0, 42 },	/* "PPPoE Payload Length (=IPv6 Payload Length + Header Length + 2 (0x0021))"	*/
/*  19 */ { 31, 0, 0, 12, 0, 0, 0, 0,  2,  6,  2, 0, 0 },	/* IPv6 PPP	*/
/*  20 */ { 31, 0, 1, 12, 0, 0, 0, 0,  7,  0, 10, 0, 0 },	/* Remove PPPoE Header	*/
/*  21 */ { 31, 0, 0, 12, 0, 0, 0, 0,  2,  8,  2, 0, 0 },	/* IPv4 EtherType	*/
/*  22 */ { 31, 0, 0, 12, 0, 0, 0, 0,  2, 10,  2, 0, 0 },	/* IPv6 EtherType	*/
/*  23 */ { 31, 1, 2,  0, 1, 1, 1, 1,  7,  0,  2, 4, 0x7fe },	/* DSCP/IPP/TOS remarking	*/
/*  24 */ { 31, 0, 2,  8, 1, 1, 0, 1,  5,  2,  1, 1, 1 },	/* TTL--	*/
/*  25 */ { 31, 1, 2,  2, 0, 0, 0, 0,  4,  2,  2, 0, 0 },	/* ICMP Checksum	*/
/*  26 */ { 31, 0, 2,  4, 1, 1, 1, 0,  0,  6,  2, 0, 0 },	/* ICMP ID	*/
/*  27 */ { 31, 1, 2, 10, 0, 0, 0, 0,  4, 10,  2, 0, 0 },	/* IPv4 Checksum	*/
/*  28 */ { 31, 0, 2, 12, 1, 1, 0, 1,  0,  2,  4, 7, 0 },	/* SIP	*/
/*  29 */ { 31, 1, 2,  0, 1, 1, 1, 0,  0,  6,  2, 0, 0 },	/* SPORT	*/
/*  30 */ { 31, 0, 2, 16, 0, 0, 0, 1,  4, 16,  2, 6, 0 },	/* TCP Checksum	*/
/*  31 */ { 31, 1, 2,  0, 1, 1, 1, 0,  0,  6,  2, 0, 0 },	/* SPORT	*/
/*  32 */ { 31, 0, 2,  6, 0, 0, 0, 1,  4,  6,  2, 6, 0 },	/* UDP Checksum	*/
/*  33 */ { 31, 1, 2, 10, 0, 0, 0, 0,  4, 10,  2, 0, 0 },	/* IPv4 Checksum	*/
/*  34 */ { 31, 0, 2, 16, 1, 1, 0, 1,  0,  2,  4, 7, 0 },	/* DIP	*/
/*  35 */ { 31, 1, 2,  2, 1, 1, 1, 0,  0,  6,  2, 0, 0 },	/* DPORT	*/
/*  36 */ { 31, 0, 2, 16, 0, 0, 0, 1,  4, 16,  2, 6, 0 },	/* TCP Checksum	*/
/*  37 */ { 31, 1, 2,  2, 1, 1, 1, 0,  0,  6,  2, 0, 0 },	/* DPORT	*/
/*  38 */ { 31, 0, 2,  6, 0, 0, 0, 1,  4,  6,  2, 6, 0 },	/* UDP Checksum	*/
/*  39 */ { 31, 0, 2, 10, 0, 0, 0, 0,  4, 10,  2, 0, 0 },	/* IPv4 Checksum	*/
/*  40 */ { 31, 1, 2,  0, 0, 0, 0, 1,  7,  0,  0, 4, 0x7fe },	/* IPv6 traffic class remarking	*/
/*  41 */ { 31, 0, 2,  7, 0, 0, 0, 1,  5,  7,  1, 1, 1 },	/* HL--	*/
/*  42 */ { 31, 1, 2,  0, 0, 0, 0, 0,  1,  0,  1, 0, 0 },	/* MAC DA	*/
/*  43 */ { 31, 0, 2,  6, 0, 0, 0, 0,  1,  1,  1, 0, 0 },	/* MAC SA	*/
/*  44 */ { 31, 1, 2,  0, 1, 1, 1, 1,  7,  0,  2, 4, 0x7fe },	/* DSCP/IPP/TOS remarking	*/
/*  45 */ {  6, 0, 2, 10, 0, 0, 0, 0,  4, 10,  2, 0, 0 },	/* IPv4 Checksum	*/
};

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

void hwnat_mde_init(void)
{
	hwnat_mac_addr_init();
	hwnat_const_init();
	hwnat_mod_macro_init();
	hwnat_mod_vec_init();
	hwnat_mod_op_init();
}

void hwnat_mde_exit(void)
{
}

void hwnat_const_init(void)
{
	uint16 entry;

	for (entry = 0; entry < CONST_TBL_SIZE; entry++)
		hwnat_const_tbl_write(entry, &const_tbl[entry]);
}

void hwnat_const_tbl_write(uint16 entry, const_val_t *const_val)
{
	uint32 table_hold[GTH_SIZE];

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = const_val->value & 0xffff;
	hwnat_write_tbl(CONST_TBL_ID, entry, table_hold, GTH_SIZE);
}

void hwnat_const_tbl_read(uint16 entry, const_val_t *const_val)
{
	uint32 table_hold[GTH_SIZE];

	hwnat_read_tbl(CONST_TBL_ID, entry, table_hold, GTH_SIZE);
	const_val->value = table_hold[7] & 0xffff;
}

void hwnat_mod_macro_init(void)
{
	uint16 entry;

	for (entry = 0; entry < MOD_MACRO_TBL_SIZE; entry++)
		hwnat_mod_macro_tbl_write(entry, &mod_macro_tbl[entry]);
}

void hwnat_mod_macro_tbl_write(uint16 entry, mod_macro_t *mod_macro)
{
	uint32 table_hold[GTH_SIZE];

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = (mod_macro->op_vec&0xff) | ((mod_macro->m<<8)&0x100);
	hwnat_write_tbl(MOD_MACRO_TBL_ID, entry, table_hold, GTH_SIZE);
}

void hwnat_mod_macro_tbl_read(uint16 entry, mod_macro_t *mod_macro)
{
	uint32 table_hold[GTH_SIZE];

	hwnat_read_tbl(MOD_MACRO_TBL_ID, entry, table_hold, GTH_SIZE);
	mod_macro->op_vec = table_hold[7]&0xff;
	mod_macro->m = (table_hold[7]&0x100)>>8;
}

void hwnat_mod_vec_init(void)
{
	uint16 entry;

	for (entry = 0; entry < MOD_VEC_TBL_SIZE; entry++)
		hwnat_mod_vec_tbl_write(entry, &mod_vec_tbl[entry]);
}

void hwnat_mod_vec_tbl_write(uint16 entry, mod_vec_t *mod_vec)
{
	uint32 table_hold[GTH_SIZE];

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = (mod_vec->op_index&0xff) | ((mod_vec->lsel<<8)&0x300) | ((mod_vec->m<<10)&0x400);
	hwnat_write_tbl(MOD_VEC_TBL_ID, entry, table_hold, GTH_SIZE);
}

void hwnat_mod_vec_tbl_read(uint16 entry, mod_vec_t *mod_vec)
{
	uint32 table_hold[GTH_SIZE];

	hwnat_read_tbl(MOD_VEC_TBL_ID, entry, table_hold, GTH_SIZE);
	mod_vec->op_index = table_hold[7]&0xff;
	mod_vec->lsel = (table_hold[7]&0x300)>>8;
	mod_vec->m = (table_hold[7]&0x400)>>10;
}

void hwnat_mod_op_init(void)
{
	uint16 entry;

	for (entry = 0; entry < MOD_OP_TBL_SIZE; entry++)
		hwnat_mod_op_tbl_write(entry, &mod_op_tbl[entry]);
}

void hwnat_mod_op_tbl_write(uint16 entry, mod_op_t *mod_op)
{
	uint32 table_hold[GTH_SIZE];

	memset(table_hold, 0, sizeof(table_hold));
	table_hold[7] = (mod_op->ext_op_data&0x7ff) | ((mod_op->ext_op_e<<11)&0x3800) | 
			((mod_op->data_len<<14)&0x3c000) | ((mod_op->data_idx<<18)&0x1fc0000) | 
			((mod_op->data_src<<25)&0xe000000) | ((mod_op->conti<<28)&0x10000000) | 
			((mod_op->chksum_reset<<29)&0x20000000) | ((mod_op->chksum_sub<<30)&0x40000000) | 
			((mod_op->chksum_add<<31)&0x80000000);
	table_hold[6] = (mod_op->src_pkt_loc&0x7f) | ((mod_op->op<<7)&0x180) | 
			((mod_op->m<<9)&0x200) | ((mod_op->status_index<<10)&0x7c00);
	hwnat_write_tbl(MOD_OP_TBL_ID, entry, table_hold, GTH_SIZE);
}

void hwnat_mod_op_tbl_read(uint16 entry, mod_op_t *mod_op)
{
	uint32 table_hold[GTH_SIZE];

	hwnat_read_tbl(MOD_OP_TBL_ID, entry, table_hold, GTH_SIZE);
	mod_op->ext_op_data = table_hold[7]&0x7ff;
	mod_op->ext_op_e = (table_hold[7]&0x3800)>>11;
	mod_op->data_len = (table_hold[7]&0x3c00)>>14;
	mod_op->data_idx = (table_hold[7]&0x1fc000)>>18;
	mod_op->data_src = (table_hold[7]&0xe000000)>>18;
	mod_op->conti = (table_hold[7]&0x10000000)>>28;
	mod_op->chksum_reset = (table_hold[7]&0x20000000)>>29;
	mod_op->chksum_sub = (table_hold[7]&0x40000000)>>30;
	mod_op->chksum_add = (table_hold[7]&0x80000000)>>31;
	mod_op->src_pkt_loc = table_hold[6]&0x7f;
	mod_op->op = (table_hold[6]&0x180)>>7;
	mod_op->m = (table_hold[6]&0x200)>>9;
	mod_op->status_index = (table_hold[6]&0x7c00)>>10;
}

/************************************************************************
*                    C I     C O M M A N D S 
*************************************************************************
*/
static int do_hwnat_const_rd(int argc, char *argv[], void *p);
static int do_hwnat_const_wr(int argc, char *argv[], void *p);

static int do_hwnat_const_tbl(int argc, char *argv[], void *p);
static int do_hwnat_const_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_const_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_const_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_const_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_const_cmds[] = {
	{"rd",			do_hwnat_const_rd,			0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_const_wr,			0x02,  	2,  "<entry> <value>"},
	{"tbl",			do_hwnat_const_tbl,			0x12,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_const_tbl_cmds[] = {
	{"dump",		do_hwnat_const_tbl_dump,	0x02,  	0,  NULL},
	{"raw",			do_hwnat_const_tbl_raw,		0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_const_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_const_tbl_comp,	0x02,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

int do_hwnat_const(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_const_cmds, argc, argv, p);
}

int do_hwnat_const_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	const_val_t const_val;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_const_tbl_read(entry, &const_val);

	printk("/*%02d*/ ", entry);
	printk("%04x\n", const_val.value);

	return 0;
}

int do_hwnat_const_wr(int argc, char *argv[], void *p)
{
	uint16 entry;
	const_val_t *const_val;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	const_val = &const_tbl[entry];

	const_val->value = (uint16) simple_strtoul(argv[2], NULL, 16);

	hwnat_const_tbl_write(entry, const_val);

	return 0;
}

int do_hwnat_const_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_const_tbl_cmds, argc, argv, p);
}

int do_hwnat_const_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;

	printk("const\n");
	for (entry = 0; entry < CONST_TBL_SIZE; entry++) {
		printk("/*%02d*/ ", entry);
		printk("%04x\n", const_tbl[entry].value);
	}
	return 0;
}

int do_hwnat_const_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	const_val_t *const_val;
	uint16 entry, i;

	printk("const\n");
	for (entry = 0; entry < CONST_TBL_SIZE; entry++) {
		const_val = &const_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = const_val->value & 0xffff;

		printk("/*%02d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_const_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	uint16 entry, i;

	printk("const\n");
	for (entry = 0; entry < CONST_TBL_SIZE; entry++) {
		hwnat_read_tbl(CONST_TBL_ID, entry, table_hold, GTH_SIZE);
		printk("/*%02d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_const_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[GTH_SIZE];
	uint32 table_hold[GTH_SIZE];
	const_val_t *const_val;
	uint16 entry, i;

	for (entry = 0; entry < CONST_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(CONST_TBL_ID, entry, hw_table_hold, GTH_SIZE);

		const_val = &const_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = const_val->value & 0xffff;

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

static int do_hwnat_mod_macro_rd(int argc, char *argv[], void *p);
static int do_hwnat_mod_macro_wr(int argc, char *argv[], void *p);

static int do_hwnat_mod_macro_tbl(int argc, char *argv[], void *p);
static int do_hwnat_mod_macro_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_mod_macro_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_mod_macro_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_mod_macro_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_mod_macro_cmds[] = {
	{"rd",			do_hwnat_mod_macro_rd,			0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_mod_macro_wr,			0x02,  	3,  "<entry> <m> <op_vec>"},
	{"tbl",			do_hwnat_mod_macro_tbl,			0x12,  	0,  NULL},
	{NULL,			NULL,							0x10,	0,	NULL},
};

static const cmds_t hwnat_mod_macro_tbl_cmds[] = {
	{"dump",		do_hwnat_mod_macro_tbl_dump,	0x02,  	0,  NULL},
	{"raw",			do_hwnat_mod_macro_tbl_raw,		0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_mod_macro_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_mod_macro_tbl_comp,	0x02,  	0,  NULL},
	{NULL,			NULL,							0x10,	0,	NULL},
};

int do_hwnat_mod_macro(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_mod_macro_cmds, argc, argv, p);
}

int do_hwnat_mod_macro_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	mod_macro_t mod_macro;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_mod_macro_tbl_read(entry, &mod_macro);

	printk("/*%03d*/ ", entry);
	printk("%d %3d\n", mod_macro.m, mod_macro.op_vec);

	return 0;
}

int do_hwnat_mod_macro_wr(int argc, char *argv[], void *p)
{
	uint16 entry;
	mod_macro_t *mod_macro;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	mod_macro = &mod_macro_tbl[entry];

	mod_macro->m = (uint8) simple_strtoul(argv[2], NULL, 10);
	mod_macro->op_vec = (uint8) simple_strtoul(argv[3], NULL, 10);

	hwnat_mod_macro_tbl_write(entry, mod_macro);

	return 0;
}

int do_hwnat_mod_macro_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_mod_macro_tbl_cmds, argc, argv, p);
}

int do_hwnat_mod_macro_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;

	printk("m op_vec\n");
	for (entry = 0; entry < MOD_MACRO_TBL_SIZE; entry++) {
		printk("/*%03d*/ %d %3d\n", entry,
					mod_macro_tbl[entry].m, mod_macro_tbl[entry].op_vec);
	}
	return 0;
}

int do_hwnat_mod_macro_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	mod_macro_t *mod_macro;
	uint16 entry, i;

	printk("mod_macro\n");
	for (entry = 0; entry < MOD_MACRO_TBL_SIZE; entry++) {
		mod_macro = &mod_macro_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (mod_macro->op_vec&0xff) | ((mod_macro->m<<8)&0x100);

		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_mod_macro_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	uint16 entry, i;

	printk("mod_macro\n");
	for (entry = 0; entry < MOD_MACRO_TBL_SIZE; entry++) {
		hwnat_read_tbl(MOD_MACRO_TBL_ID, entry, table_hold, GTH_SIZE);
		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_mod_macro_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[GTH_SIZE];
	uint32 table_hold[GTH_SIZE];
	mod_macro_t *mod_macro;
	uint16 entry, i;

	for (entry = 0; entry < MOD_MACRO_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(MOD_MACRO_TBL_ID, entry, hw_table_hold, GTH_SIZE);

		mod_macro = &mod_macro_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (mod_macro->op_vec&0xff) | ((mod_macro->m<<8)&0x100);

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


static int do_hwnat_mod_vec_rd(int argc, char *argv[], void *p);
static int do_hwnat_mod_vec_wr(int argc, char *argv[], void *p);

static int do_hwnat_mod_vec_tbl(int argc, char *argv[], void *p);
static int do_hwnat_mod_vec_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_mod_vec_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_mod_vec_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_mod_vec_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_mod_vec_cmds[] = {
	{"rd",			do_hwnat_mod_vec_rd,		0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_mod_vec_wr,		0x02,  	4,  "<entry> <m> <lsel> <op_index>"},
	{"tbl",			do_hwnat_mod_vec_tbl,		0x12,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_mod_vec_tbl_cmds[] = {
	{"dump",		do_hwnat_mod_vec_tbl_dump,	0x02,  	0,  NULL},
	{"raw",			do_hwnat_mod_vec_tbl_raw,	0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_mod_vec_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_mod_vec_tbl_comp,	0x02,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

int do_hwnat_mod_vec(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_mod_vec_cmds, argc, argv, p);
}

int do_hwnat_mod_vec_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	mod_vec_t mod_vec;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_mod_vec_tbl_read(entry, &mod_vec);

	printk("/*%03d*/ ", entry);
	printk("%d %d %3d\n", mod_vec.m, mod_vec.lsel, mod_vec.op_index); 

	return 0;
}

int do_hwnat_mod_vec_wr(int argc, char *argv[], void *p)
{
	uint16 entry;
	mod_vec_t *mod_vec;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	mod_vec = &mod_vec_tbl[entry];

	mod_vec->m = (uint8) simple_strtoul(argv[2], NULL, 10);
	mod_vec->lsel = (uint8) simple_strtoul(argv[3], NULL, 10);
	mod_vec->op_index = (uint8) simple_strtoul(argv[4], NULL, 10);

	hwnat_mod_vec_tbl_write(entry, mod_vec);

	return 0;
}

int do_hwnat_mod_vec_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_mod_vec_tbl_cmds, argc, argv, p);
}

int do_hwnat_mod_vec_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;

	printk("m lsel op_index\n");
	for (entry = 0; entry < MOD_VEC_TBL_SIZE; entry++) {
		printk("/*%03d*/ %d %d %3d\n", entry,
					mod_vec_tbl[entry].m, mod_vec_tbl[entry].lsel, mod_vec_tbl[entry].op_index); 
	}
	return 0;
}

int do_hwnat_mod_vec_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	mod_vec_t *mod_vec;
	uint16 entry, i;

	printk("mod_vec\n");
	for (entry = 0; entry < MOD_VEC_TBL_SIZE; entry++) {
		mod_vec = &mod_vec_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (mod_vec->op_index&0xff) | ((mod_vec->lsel<<8)&0x300) | ((mod_vec->m<<10)&0x400);

		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_mod_vec_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	uint16 entry, i;

	printk("mod_vec\n");
	for (entry = 0; entry < MOD_VEC_TBL_SIZE; entry++) {
		hwnat_read_tbl(MOD_VEC_TBL_ID, entry, table_hold, GTH_SIZE);
		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_mod_vec_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[GTH_SIZE];
	uint32 table_hold[GTH_SIZE];
	mod_vec_t *mod_vec;
	uint16 entry, i;

	for (entry = 0; entry < MOD_VEC_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(MOD_VEC_TBL_ID, entry, hw_table_hold, GTH_SIZE);

		mod_vec = &mod_vec_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (mod_vec->op_index&0xff) | ((mod_vec->lsel<<8)&0x300) | ((mod_vec->m<<10)&0x400);

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


static int do_hwnat_mod_op_rd(int argc, char *argv[], void *p);
static int do_hwnat_mod_op_wr(int argc, char *argv[], void *p);

static int do_hwnat_mod_op_tbl(int argc, char *argv[], void *p);
static int do_hwnat_mod_op_tbl_dump(int argc, char *argv[], void *p);
static int do_hwnat_mod_op_tbl_raw(int argc, char *argv[], void *p);
static int do_hwnat_mod_op_tbl_hwraw(int argc, char *argv[], void *p);
static int do_hwnat_mod_op_tbl_comp(int argc, char *argv[], void *p);

static const cmds_t hwnat_mod_op_cmds[] = {
	{"rd",			do_hwnat_mod_op_rd,			0x02,  	1,  "<entry>"},
	{"wr",			do_hwnat_mod_op_wr,			0x02,  	14, "<entry> <status> <m> <op> <src_pkt_loc> <a> <s> <r> <c> <src> <idx> <len> <ext_op_e> <ext_op_data>"},
	{"tbl",			do_hwnat_mod_op_tbl,		0x12,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

static const cmds_t hwnat_mod_op_tbl_cmds[] = {
	{"dump",		do_hwnat_mod_op_tbl_dump,	0x02,  	0,  NULL},
	{"raw",			do_hwnat_mod_op_tbl_raw,	0x02,  	0,  NULL},
	{"hwraw",		do_hwnat_mod_op_tbl_hwraw,	0x02,  	0,  NULL},
	{"comp",		do_hwnat_mod_op_tbl_comp,	0x02,  	0,  NULL},
	{NULL,			NULL,						0x10,	0,	NULL},
};

int do_hwnat_mod_op(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_mod_op_cmds, argc, argv, p);
}

int do_hwnat_mod_op_rd(int argc, char *argv[], void *p)
{
	uint16 entry;
	mod_op_t mod_op;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);
	hwnat_mod_op_tbl_read(entry, &mod_op);

	printk("/*%03d*/ ", entry);
	printk("%2d %d %d %2d %d %d %d %d | %d %2d %2d | %d %2d\n", 
			mod_op.status_index, mod_op.m, mod_op.op, mod_op.src_pkt_loc, 
			mod_op.chksum_add, mod_op.chksum_sub, mod_op.chksum_reset, 
			mod_op.conti, mod_op.data_src, mod_op.data_idx, mod_op.data_len, 
			mod_op.ext_op_e, mod_op.ext_op_data);

	return 0;
}

int do_hwnat_mod_op_wr(int argc, char *argv[], void *p)
{
	uint16 entry;
	mod_op_t *mod_op;

	entry = (uint16) simple_strtoul(argv[1], NULL, 10);

	mod_op = &mod_op_tbl[entry];

	mod_op->status_index = (uint8) simple_strtoul(argv[2], NULL, 10);
	mod_op->m = (uint8) simple_strtoul(argv[3], NULL, 10);
	mod_op->op = (uint8) simple_strtoul(argv[4], NULL, 10);
	mod_op->src_pkt_loc = (uint8) simple_strtoul(argv[5], NULL, 10);
	mod_op->chksum_add = (uint8) simple_strtoul(argv[6], NULL, 10);
	mod_op->chksum_sub = (uint8) simple_strtoul(argv[7], NULL, 10);
	mod_op->chksum_reset = (uint8) simple_strtoul(argv[8], NULL, 10);
	mod_op->conti = (uint8) simple_strtoul(argv[9], NULL, 10);
	mod_op->data_src = (uint8) simple_strtoul(argv[10], NULL, 10);
	mod_op->data_idx = (uint8) simple_strtoul(argv[11], NULL, 10);
	mod_op->data_len = (uint8) simple_strtoul(argv[12], NULL, 10);
	mod_op->ext_op_e = (uint8) simple_strtoul(argv[13], NULL, 10);
	mod_op->ext_op_data = (uint8) simple_strtoul(argv[14], NULL, 10);

	hwnat_mod_op_tbl_write(entry, mod_op);

	return 0;
}

int do_hwnat_mod_op_tbl(int argc, char *argv[], void *p)
{
	return subcmd(hwnat_mod_op_tbl_cmds, argc, argv, p);
}

int do_hwnat_mod_op_tbl_dump(int argc, char *argv[], void *p)
{
	uint16 entry;

	printk("  status_index m op src_pkt_loc a s r c | src idx len | ext_op_e ext_op_data\n");
	for (entry = 0; entry < MOD_OP_TBL_SIZE; entry++) {
		printk("/*%03d*/ %2d %d %d %2d %d %d %d %d | %d %2d %2d | %d %2d\n", entry,
					mod_op_tbl[entry].status_index, mod_op_tbl[entry].m, mod_op_tbl[entry].op,
					mod_op_tbl[entry].src_pkt_loc, 
					mod_op_tbl[entry].chksum_add, mod_op_tbl[entry].chksum_sub,
					mod_op_tbl[entry].chksum_reset, mod_op_tbl[entry].conti,
					mod_op_tbl[entry].data_src, mod_op_tbl[entry].data_idx,
					mod_op_tbl[entry].data_len, mod_op_tbl[entry].ext_op_e,
					mod_op_tbl[entry].ext_op_data);
	}
	return 0;
}

int do_hwnat_mod_op_tbl_raw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	mod_op_t *mod_op;
	uint16 entry, i;

	printk("mod_op\n");
	for (entry = 0; entry < MOD_OP_TBL_SIZE; entry++) {
		mod_op = &mod_op_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (mod_op->ext_op_data&0x7ff) | ((mod_op->ext_op_e<<11)&0x3800) | 
				((mod_op->data_len<<14)&0x3c000) | ((mod_op->data_idx<<18)&0x1fc0000) | 
				((mod_op->data_src<<25)&0xe000000) | ((mod_op->conti<<28)&0x10000000) | 
				((mod_op->chksum_reset<<29)&0x20000000) | ((mod_op->chksum_sub<<30)&0x40000000) | 
				((mod_op->chksum_add<<31)&0x80000000);
		table_hold[6] = (mod_op->src_pkt_loc&0x7f) | ((mod_op->op<<7)&0x180) | 
				((mod_op->m<<9)&0x200) | ((mod_op->status_index<<10)&0x7c00);

		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_mod_op_tbl_hwraw(int argc, char *argv[], void *p)
{
	uint32 table_hold[GTH_SIZE];
	uint16 entry, i;

	printk("mod_op\n");
	for (entry = 0; entry < MOD_OP_TBL_SIZE; entry++) {
		hwnat_read_tbl(MOD_OP_TBL_ID, entry, table_hold, GTH_SIZE);
		printk("/*%03d*/", entry);
		for (i = 0; i < GTH_SIZE; i++)
			printk(" %08lx", table_hold[i]);
		printk("\n");
	}
	return 0;
}

int do_hwnat_mod_op_tbl_comp(int argc, char *argv[], void *p)
{
	uint32 hw_table_hold[GTH_SIZE];
	uint32 table_hold[GTH_SIZE];
	mod_op_t *mod_op;
	uint16 entry, i;

	for (entry = 0; entry < MOD_OP_TBL_SIZE; entry++) {
		/* get hw table hold registers */
		hwnat_read_tbl(MOD_OP_TBL_ID, entry, hw_table_hold, GTH_SIZE);

		mod_op = &mod_op_tbl[entry];

		memset(table_hold, 0, sizeof(table_hold));
		table_hold[7] = (mod_op->ext_op_data&0x7ff) | ((mod_op->ext_op_e<<11)&0x3800) | 
				((mod_op->data_len<<14)&0x3c000) | ((mod_op->data_idx<<18)&0x1fc0000) | 
				((mod_op->data_src<<25)&0xe000000) | ((mod_op->conti<<28)&0x10000000) | 
				((mod_op->chksum_reset<<29)&0x20000000) | ((mod_op->chksum_sub<<30)&0x40000000) | 
				((mod_op->chksum_add<<31)&0x80000000);
		table_hold[6] = (mod_op->src_pkt_loc&0x7f) | ((mod_op->op<<7)&0x180) | 
				((mod_op->m<<9)&0x200) | ((mod_op->status_index<<10)&0x7c00);

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


