/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/hwnat/hwnat.h#1 $
*/
/************************************************************************
 *
 *	Copyright (C) 2010 Trendchip Technologies, Corp.
 *	All Rights Reserved. sfdsf
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
** $Log: hwnat.h,v $
** Revision 1.4  2011/06/30 12:07:32  lino
** hwnat enhance: IPv6 and QinQ support
**
** Revision 1.3  2011/06/08 10:02:23  lino
** add RT65168 support
**
** Revision 1.2  2011/06/03 02:41:43  lino
** add RT65168 support
**
** Revision 1.1.2.1  2011/04/01 09:16:41  lino
** add RT65168 support
**
 */
#ifndef __HWNAT_H
#define __HWNAT_H

/**************************
 * HWNAT clock            *
 **************************/

#define HWNAT_CLK					(100)

/**************************
 * HWNAT port size        *
 **************************/

#define HWNAT_MAX_PORT_NO			4

#define HWNAT_PORT_GMAC0			0
#define HWNAT_PORT_GMAC1			1
#define HWNAT_PORT_PTM0				2
#define HWNAT_PORT_PTM1				3
#define HWNAT_PORT_CPU				15

#define HWNAT_MIN_QID				0
#define HWNAT_MAX_QID				3

/**************************
 * HWNAT tables size      *
 **************************/

#define INTERFACE_TBL_SIZE			16
#define GPR_TBL_SIZE				32
#define COMPARATOR_TBL_SIZE			128
#define POLICY_TBL_SIZE				128
#define POLICY_RESULT_TBL_SIZE		64
#define TUPLE_TBL_SIZE				64
#define HASH_TBL_SIZE				512
#define FLOW_TBL_SIZE				256
#define MAC_ADDR_TBL_SIZE			256
#define CONST_TBL_SIZE				64
#define MOD_MACRO_TBL_SIZE			256
#define MOD_VEC_TBL_SIZE			256
#define MOD_OP_TBL_SIZE				128
#define QOS_TBL_SIZE				32

#define AGE_STATUS_SIZE				8

/*******************************
 * HWNAT tables hold size      *
 *******************************/

#define GTH_SIZE					8
#define HTH_SIZE					1
#define FTH_SIZE					12
#define MTH_SIZE					2

/*****************************
 * HWNAT Module Registers *
 *****************************/

#define CR_HWNAT_BASE    			0xBFBE0000
#define CR_HWNAT_CTL0      			(0x00 + CR_HWNAT_BASE)
#define CR_HWNAT_CTL1      			(0x04 + CR_HWNAT_BASE)
#define CR_HWNAT_GSR      			(0x08 + CR_HWNAT_BASE)
#define CR_HWNAT_SSR      			(0x0c + CR_HWNAT_BASE)
#define CR_HWNAT_GTS      			(0x10 + CR_HWNAT_BASE)
#define CR_HWNAT_HTS      			(0x14 + CR_HWNAT_BASE)
#define CR_HWNAT_FTS      			(0x18 + CR_HWNAT_BASE)
#define CR_HWNAT_MTS      			(0x1c + CR_HWNAT_BASE)
#define CR_HWNAT_GV      			(0x20 + CR_HWNAT_BASE)
#define CR_HWNAT_TR      			(0x24 + CR_HWNAT_BASE)
#define CR_HWNAT_TRD      			(0x28 + CR_HWNAT_BASE)
#define CR_HWNAT_AS(index)  		(0x50 + CR_HWNAT_BASE + ((index)*4))
#define CR_HWNAT_GH(hold)  			(0x70 + CR_HWNAT_BASE + ((hold)*4))
#define CR_HWNAT_HH0      			(0x90 + CR_HWNAT_BASE)
#define CR_HWNAT_FH(hold)  			(0x94 + CR_HWNAT_BASE + ((hold)*4))
#define CR_HWNAT_MH(hold)  			(0xc4 + CR_HWNAT_BASE + ((hold)*4))
#define CR_HWNAT_AFS      			(0xcc + CR_HWNAT_BASE)
#define CR_HWNAT_AFC      			(0xd0 + CR_HWNAT_BASE)
#define CR_HWNAT_RXPORT01    		(0xd4 + CR_HWNAT_BASE)
#define CR_HWNAT_RXPORT23    		(0xd8 + CR_HWNAT_BASE)
#define CR_HWNAT_QOSPKTDATA    		(0xdc + CR_HWNAT_BASE)
#define CR_HWNAT_ISR	    		(0xe0 + CR_HWNAT_BASE)

/************************************
 * HWNAT Module Register Definition *
 ************************************/

/* CR_HWNAT_CTL0 */
#define CTL0_HWNAT_EN				(1<<0)
#define CTL0_SW_RST					(1<<1)
#define CTL0_AGE_INT				(1<<2)
#define CTL0_LOAD_AGE				(1<<3)
#define CTL0_IPV4CHKSUM_DROP		(1<<6)
#define CTL0_IPV4CHKSUM_DIS			(1<<7)
#define CTL0_IPV6UDP_SHIFT			(8)
#define CTL0_IPV6UDP				(0x7f<<CTL0_IPV6UDP_SHIFT)
#define CTL0_FLOWDO_SHIFT			(17)
#define CTL0_FLOWDO					(0xff<<CTL0_FLOWDO_SHIFT)
#define CTL0_IPV4CHKSUM_IDX_SHIFT	(25)
#define CTL0_IPV4CHKSUM_IDX			(0x7f<<CTL0_IPV4CHKSUM_IDX_SHIFT)

/* ipv6 udp comparator index */
#define CTL0_IPV6UDP_INDEX			(32)
/* flow table direct out index */
#define CTL0_FLOWDO_INDEX			(0)
/* ipv4 checksum comparator index */
#define CTL0_IPV4CHKSUM_INDEX		(28)

/* CR_HWNAT_CTL1 */
#define CTL1_GPR_SET				(1<<31)
#define CTL1_IPV4UDP_SHIFT			(21)
#define CTL1_IPV4UDP				(0x7f<<CTL1_IPV4UDP_SHIFT)
#define CTL1_PKTDATA1_SHIFT			(14)
#define CTL1_PKTDATA1				(0x7f<<CTL1_PKTDATA1_SHIFT)
#define CTL1_PKTDATA0_SHIFT			(7)
#define CTL1_PKTDATA0				(0x7f<<CTL1_PKTDATA0_SHIFT)
#define CTL1_VLAN_SHIFT				(0)
#define CTL1_VLAN 					(0x7f<<CTL1_VLAN_SHIFT)

/* ipv4 udp comparator index */
#define CTL1_IPV4UDP_INDEX			(40)
/* pkt data1 comparator index */
#define CTL1_PKTDATA1_INDEX			(0)
/* pkt data0 comparator index */
#define CTL1_PKTDATA0_INDEX			(17)
/* vlan id comparator index */
#define CTL1_VLAN_INDEX				(19)

/* CR_HWNAT_GSR */
#define GSR_FTBL_AGEOUT				(1<<7)

/* CR_HWNAT_GTS */
#define GTS_ENTRY_SHIFT				(0)
#define GTS_ENTRY					(0xfffff<<GTS_ENTRY_SHIFT)
#define GTS_TS_SHIFT				(20)
#define GTS_TS						(0xf<<GTS_TS_SHIFT)
#define GTS_INTERFACE_TBL			(0)
#define GTS_GPR_TBL					(1)
#define GTS_COMPARATOR_TBL			(2)
#define GTS_POLICY_TBL				(3)
#define GTS_POLICY_RESULT_TBL		(4)
#define GTS_TUPLE_TBL				(5)
#define GTS_CONST_TBL				(6)
#define GTS_MOD_MACRO_TBL			(7)
#define GTS_MOD_VEC_TBL				(8)
#define GTS_MOD_OP_TBL				(9)
#define GTS_QOS0_TBL				(10)
#define GTS_QOS1_TBL				(11)
#define GTS_QOS2_TBL				(12)
#define GTS_QOS3_TBL				(13)
#define GTS_TR_DONE					(1<<28)
#define GTS_TW_DONE					(1<<29)
#define GTS_TW						(1<<30)
#define GTS_TR						(1<<31)

/* CR_HWNAT_HTS */
#define HTS_ENTRY_SHIFT				(0)
#define HTS_ENTRY					(0xfffff<<HTS_ENTRY_SHIFT)
#define HTS_TR_DONE					(1<<28)
#define HTS_TW_DONE					(1<<29)
#define HTS_TW						(1<<30)
#define HTS_TR						(1<<31)

/* CR_HWNAT_FTS */
#define FTS_ENTRY_SHIFT				(0)
#define FTS_ENTRY					(0xfffff<<FTS_ENTRY_SHIFT)
#define FTS_TR_DONE					(1<<28)
#define FTS_TW_DONE					(1<<29)
#define FTS_TW						(1<<30)
#define FTS_TR						(1<<31)

/* CR_HWNAT_MTS */
#define MTS_ENTRY_SHIFT				(0)
#define MTS_ENTRY					(0xfffff<<MTS_ENTRY_SHIFT)
#define MTS_TR_DONE					(1<<28)
#define MTS_TW_DONE					(1<<29)
#define MTS_TW						(1<<30)
#define MTS_TR						(1<<31)

/* CR_HWNAT_TR */
#define TR_IT						(1<<0)
#define TR_GPR						(1<<1)
#define TR_CMPR						(1<<2)
#define TR_POLICY					(1<<3)
#define TR_POLICY_RESULT			(1<<4)
#define TR_TUPLE					(1<<5)
#define TR_CONST					(1<<7)
#define TR_MOD_MACRO				(1<<8)
#define TR_MOD_VEC					(1<<9)
#define TR_MOD_OP					(1<<10)
#define TR_HASH						(1<<12)
#define TR_FLOW						(1<<13)
#define TR_MAC						(1<<14)

/* CR_HWNAT_TRD */
#define TRD_IT						(1<<0)
#define TRD_GPR						(1<<1)
#define TRD_CMPR					(1<<2)
#define TRD_POLICY					(1<<3)
#define TRD_POLICY_RESULT			(1<<4)
#define TRD_TUPLE					(1<<5)
#define TRD_CONST					(1<<7)
#define TRD_MOD_MACRO				(1<<8)
#define TRD_MOD_VEC					(1<<9)
#define TRD_MOD_OP					(1<<10)
#define TRD_HASH					(1<<12)
#define TRD_FLOW					(1<<13)
#define TRD_MAC						(1<<14)

/* CR_HWNAT_AFS */
#define AFS_CLKDIV_SHIFT			(0)
#define AFS_CLKDIV					(0x3fffffff<<AFS_CLKDIV_SHIFT)

/* CR_HWNAT_AFC */
#define AFC_INTTIME_SHIFT 			(8)
#define AFC_INTTIME					(0xff<<AFC_INTTIME_SHIFT)
#define AFC_INTNO_SHIFT 			(16)
#define AFC_INTNO					(0xff<<AFC_INTNO_SHIFT)
#define AFC_AGETIME_SHIFT 			(24)
#define AFC_AGETIME					(0xff<<AFC_AGETIME_SHIFT)

/* CR_HWNAT_RXPORT01 */
#define RXPORT01_0_CMPR_SHIFT		(0)
#define RXPORT01_0_CMPR				(0x7f<<RXPORT01_0_CMPR_SHIFT)
#define RXPORT01_0_L2H_SHIFT		(7)
#define RXPORT01_0_L2H				(0x7f<<RXPORT01_0_L2H_SHIFT)
#define RXPORT01_1_CMPR_SHIFT		(16)
#define RXPORT01_1_CMPR				(0x7f<<RXPORT01_1_CMPR_SHIFT)
#define RXPORT01_1_L2H_SHIFT		(23)
#define RXPORT01_1_L2H				(0x7f<<RXPORT01_1_L2H_SHIFT)

/* CR_HWNAT_RXPORT23 */
#define RXPORT23_2_CMPR_SHIFT		(0)
#define RXPORT23_2_CMPR				(0x7f<<RXPORT23_2_CMPR_SHIFT)
#define RXPORT23_2_L2H_SHIFT		(7)
#define RXPORT23_2_L2H				(0x7f<<RXPORT23_2_L2H_SHIFT)
#define RXPORT23_3_CMPR_SHIFT		(16)
#define RXPORT23_3_CMPR				(0x7f<<RXPORT23_3_CMPR_SHIFT)
#define RXPORT23_3_L2H_SHIFT		(23)
#define RXPORT23_3_L2H				(0x7f<<RXPORT23_3_L2H_SHIFT)

/* CR_HWNAT_QOSPKTDATA */
#define PKTDATA1_MASK_SHIFT			(16)
#define PKTDATA1_MASK				(0xffff<<PKTDATA1_MASK_SHIFT)
#define PKTDATA0_MASK_SHIFT			(0)
#define PKTDATA0_MASK				(0xffff<<PKTDATA0_MASK_SHIFT)

/* TC2206 inport mask */
#define TC2206_INPORT_MASK			(0x7)

/* CR_HWNAT_ISR */
#define ISR_INT_SHIFT				(0)
#define ISR_INT						(0x1<<ISR_INT_SHIFT)

/*****************************
 * NFE Module Registers *
 *****************************/

#define CR_NFE_BASE    				0xBFBE0000
#define CR_NFE_FBCMR    			(0x228 + CR_NFE_BASE)
#define CR_NFE_EPQSMSCR    			(0x23c + CR_NFE_BASE)
#define CR_NFE_PERWRRCR(port)		(0x240 + 0xc*(port) + CR_NFE_BASE)
#define CR_NFE_PERCR0(port)			(0x244 + 0xc*(port) + CR_NFE_BASE)
#define CR_NFE_PERCR1(port)			(0x248 + 0xc*(port) + CR_NFE_BASE)
#define CR_NFE_MIBSCR    			(0x270 + CR_NFE_BASE)
#define CR_NFE_MIBRXPKT    			(0x274 + CR_NFE_BASE)
#define CR_NFE_MIBRXBYTE   			(0x278 + CR_NFE_BASE)
#define CR_NFE_MIBRXDROP   			(0x27c + CR_NFE_BASE)
#define CR_NFE_MIBTXPKTQ0  			(0x280 + CR_NFE_BASE)
#define CR_NFE_MIBTXPKTQ1  			(0x284 + CR_NFE_BASE)
#define CR_NFE_MIBTXPKTQ2  			(0x288 + CR_NFE_BASE)
#define CR_NFE_MIBTXPKTQ3  			(0x28c + CR_NFE_BASE)
#define CR_NFE_MIBTXBYTE  			(0x290 + CR_NFE_BASE)
#define CR_NFE_MISC	    			(0x2ac + CR_NFE_BASE)

/* CR_NFE_FBCMR */
#define RD_FB_CNT_SHIFT				(24)
#define RD_FB_CNT					(1<<RD_FB_CNT_SHIFT)
#define FB_CNT						(0xff)

/* CR_NFE_EPQSMSCR */
#define P0_QSM_SHIFT				(0)
#define P0_QSM						(0x3<<P0_QSM_SHIFT)
#define P1_QSM_SHIFT				(4)
#define P1_QSM						(0x3<<P1_QSM_SHIFT)
#define P2_QSM_SHIFT				(8)
#define P2_QSM						(0x3<<P2_QSM_SHIFT)
#define P3_QSM_SHIFT				(12)
#define P3_QSM						(0x3<<P3_QSM_SHIFT)

#define QUEUE_SCH_MODE_WRR			0
#define QUEUE_SCH_MODE_RC			1
#define QUEUE_SCH_MODE_SP_RC		2
#define QUEUE_SCH_MODE_SP			3

/* CR_NFE_PERWRRCR */
#define WRR_0_SHIFT					(0)
#define WRR_0						(0xf<<WRR_0_SHIFT)
#define WRR_1_SHIFT					(4)
#define WRR_1						(0xf<<WRR_1_SHIFT)
#define WRR_2_SHIFT					(8)
#define WRR_2						(0xf<<WRR_2_SHIFT)
#define WRR_3_SHIFT					(12)
#define WRR_3						(0xf<<WRR_3_SHIFT)

/* CR_NFE_PERCR0 */
#define RC_0_SHIFT					(0)
#define RC_0						(0x3fff<<RC_0_SHIFT)
#define RC_1_SHIFT					(16)
#define RC_1						(0x3fff<<RC_1_SHIFT)

/* CR_NFE_PERCR1 */
#define RC_2_SHIFT					(0)
#define RC_2						(0x3fff<<RC_2_SHIFT)
#define RC_3_SHIFT					(16)
#define RC_3						(0x3fff<<RC_3_SHIFT)

/* CR_NFE_MIBSCR */
#define MIBSCR_PORTID_SHIFT			(12)
#define MIBSCR_PORTID				(0x3<<MIBSCR_PORTID_SHIFT)
#define MIBSCR_FREEZE_SHIFT			(7)
#define MIBSCR_FREEZE				(0x1<<MIBSCR_FREEZE_SHIFT)
#define MIBSCR_CLEAR_SHIFT			(3)
#define MIBSCR_CLEAR				(0x1<<MIBSCR_CLEAR_SHIFT)
#define MIBSCR_READ_SHIFT			(0)
#define MIBSCR_READ					(0x1<<MIBSCR_READ_SHIFT)

/* CR_NFE_MISC */
#define MISC_MAC_BKPR_EN_SHIFT		(24)
#define MISC_MAC_BKPR_EN			(0xf<<MISC_MAC_BKPR_EN_SHIFT)
#define MISC_BP_MODE_SHIFT			(12)
#define MISC_BP_MODE				(0x3<<MISC_BP_MODE_SHIFT)
#define MISC_MAC_RX_XFC_SHIFT		(8)
#define MISC_MAC_RX_XFC				(0xf<<MISC_MAC_RX_XFC_SHIFT)


#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u"

#define NIP6(addr) \
	ntohs((addr).s6_addr16[0]), \
	ntohs((addr).s6_addr16[1]), \
	ntohs((addr).s6_addr16[2]), \
	ntohs((addr).s6_addr16[3]), \
	ntohs((addr).s6_addr16[4]), \
	ntohs((addr).s6_addr16[5]), \
	ntohs((addr).s6_addr16[6]), \
	ntohs((addr).s6_addr16[7])
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#define NIP6_SEQFMT "%04x%04x%04x%04x%04x%04x%04x%04x"

#if defined(__LITTLE_ENDIAN)
#define HIPQUAD(addr) \
	((unsigned char *)&addr)[3], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[0]
#elif defined(__BIG_ENDIAN)
#define HIPQUAD	NIPQUAD
#else
#error "Please fix asm/byteorder.h"
#endif /* __LITTLE_ENDIAN */

/************************************************************************
*        F U N C T I O N   P R O T O T Y P E S
*************************************************************************
*/

/************************************************************************
*                      E X T E R N A L   D A T A
*************************************************************************
*/
extern uint32 hwnat_active;
extern uint32 hwnat_debug;
extern uint32 hwnat_hitcnt;
extern uint32 hwnat_multicast_hitcnt;

#endif /* __HWNAT_H */

