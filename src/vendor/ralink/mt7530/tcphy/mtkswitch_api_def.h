/*
** $Id: $
*/
/************************************************************************
 *
 *	Copyright (C) 2012 Mediatek Inc.
 *	All Rights Reserved.
 *
 * Mediatek Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Mediatek Inc.
 *
 *************************************************************************/
/*
** $Log$
**
 */

#ifndef _GMAC_MT7530_DEF_H_
#define _GMAC_MT7530_DEF_H_

#ifdef TCSUPPORT_MT7530_SWITCH_API

#include <linux/types.h>
//#include <asm/tc3162/tc3162.h>
#ifdef TCSUPPORT_2_6_36_KERNEL
#include "../../../linux-2.6.36/arch/mips/include/asm/tc3162/tc3162.h"
#else
#include "../../../linux/include/asm-mips/tc3162/tc3162.h"
#endif

#ifdef TCSUPPORT_CPU_MT7510
#undef DEFAULT_USE_EXT_SWIC
#endif
#ifdef TCSUPPORT_CPU_MT7520
#define DEFAULT_USE_EXT_SWIC
#endif
#if defined(TCSUPPORT_CPU_RT65168) || defined(TCSUPPORT_CPU_TC3182)	// for early MT7530 FPGA verification only
#if !defined(TCSUPPORT_CT) 
#define DEFAULT_USE_EXT_SWIC
#endif
#endif

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

#define RAETH_GSW_CTLAPI		0x89FB

/* define  */
typedef enum
{
	CMD_RSV = 0,
	// user API start from ID 0x0010
	CMD_USRAPI_BEGIN = 0x0010,
	CMD_GET_BRGLEARNINGIND,
	CMD_SET_BRGLEARNINGIND,
	CMD_GET_PORTBRGIND,
	CMD_SET_PORTBRGIND,
	CMD_GET_DISCARDUNKNOWNMACIND,
	CMD_SET_DISCARDUNKNOWNMACIND,
	CMD_GET_AGETIME,
	CMD_SET_AGETIME,
	CMD_GET_PORTMAC,
	CMD_GET_PORTSTATUS,
	CMD_GET_MIBCNT,

	CMD_SET_QUEUE_PRIORITY,
	CMD_GET_QUEUE_MAX_SIZE,
	CMD_SET_ALLOC_QUEUE_SIZE,
	CMD_GET_ALLOC_QUEUE_SIZE,
	CMD_SET_WEIGHT,
	CMD_GET_WEIGHT,
	CMD_SET_BACK_PRESSURE,
	CMD_GET_BACK_PRESSURE,
	CMD_SET_DROP_POLICY,
	CMD_GET_DROP_POLICY,
	CMD_SET_TRAFFIC_DESCRIPTOR,
	CMD_GET_TRAFFIC_DESCRIPTOR,
	CMD_CLR_TRAFFIC_DESCRIPTOR,

	CMD_SET_AUTO_DETECT,
	CMD_GET_AUTO_DETECT,
	CMD_SET_LOOPBACK_CONF,
	CMD_GET_LOOPBACK_CONF,
	CMD_GET_CONFIG_STAT,
	CMD_SET_MAX_FRAME_SIZE,
	CMD_GET_MAX_FRAME_SIZE,
	CMD_SET_DTEDCE_Ind,
	CMD_GET_DTEDCE_Ind,
	CMD_SET_PAUSE_TIME,
	CMD_GET_PAUSE_TIME,

	CMD_GET_PORT_CHANGEDCNT,
	CMD_GET_PORT_MACLMT,
	CMD_SET_PORT_MACLMT,
	CMD_SET_PORT_SPDMD,
	CMD_GET_PORT_PAUSE,
	CMD_SET_PORT_PAUSE,
	CMD_GET_PORT_POLEN,
	CMD_SET_PORT_POLEN,
	CMD_GET_PORT_POL,
	CMD_SET_PORT_POL,
	CMD_GET_PORT_RATELMTEN,
	CMD_SET_PORT_RATELMTEN,
	CMD_GET_PORT_RATELMT,
	CMD_SET_PORT_RATELMT,
	CMD_GET_PORT_LOOPDET,
	CMD_SET_PORT_LOOPDET,
	CMD_SET_PORT_DISLOOPED,
	CMD_GET_PORT_ACT,
	CMD_SET_PORT_ACT,
	CMD_GET_PORT_AN,
	CMD_SET_PORT_AN,
	CMD_SET_PORT_AN_RESTART,
	CMD_GET_PORT_AN_FAIL,
	CMD_GET_PORT_LINK_LOSS,
	CMD_GET_PORT_FAIL,
	CMD_GET_PORT_CONGESTION,

	CMD_GET_TX_TIMESTAMP,
	CMD_GET_RX_TIMESTAMP,
	CMD_GET_CURRTIME,
	CMD_SET_CURRTIME,
	CMD_SET_OFFSET,
	CMD_GET_PTPSTATE,
	CMD_SET_PTPSTATE,

	CMD_SET_VLAN_ENTRY_ENABLE,
	CMD_SET_VLAN_ENTRY_EGTAG,
	CMD_GET_VLAN_ENTRY_EGTAG,
	CMD_SET_VLAN_ENTRY_STAG,
	CMD_GET_VLAN_ENTRY_STAG,
	CMD_ADD_VLAN_PORT_STAG,
	CMD_SET_VLAN_ENTRY_ETAGMODE,

	// internal API start from ID 0x1000
	CMD_SET_MIBCNT_EN = 0x1000,
	CMD_SET_MIBCNT_CLR,
	CMD_SET_RXOCT_MODE,
	CMD_SET_TXOCT_MODE,
	CMD_SET_BXPKT_MODE,
	// debug API start from ID 0x7F00
	CMD_APIDBGDUMP_EN = 0x7F00,
	CMD_DO_P6Cal,

} mt7530_switch_api_cmdid;

#ifdef TCSUPPORT_CPU_MT7510
#undef DEFAULT_USE_EXT_SWIC
#endif
#ifdef TCSUPPORT_CPU_MT7520
#define DEFAULT_USE_EXT_SWIC
#endif
#if defined(TCSUPPORT_CPU_RT65168) || defined(TCSUPPORT_CPU_TC3182)	// for early MT7530 FPGA verification only
#define DEFAULT_USE_EXT_SWIC
#endif

#define MT7530_ACL_RULE_NUM	(64)
#define MT7530_ACL_ACTION_NUM	(64)	// MT7530 new design 32-->64

#define MT7530_ACL_RULE_TBL_READ	(4)
#define MT7530_ACL_RULE_TBL_WRITE	(5)
#define MT7530_ACL_MASK_TBL_READ	(8)
#define MT7530_ACL_MASK_TBL_WRITE	(9)
#define MT7530_ACL_CTRL_TBL_READ	(10)
#define MT7530_ACL_CTRL_TBL_WRITE	(11)
#define MT7530_ACL_RATE_TBL_READ	(12)
#define MT7530_ACL_RATE_TBL_WRITE	(13)
#define MT7530_ACL_TRTCM_TBL_READ	(6)
#define MT7530_ACL_TRTCM_TBL_WRITE	(7)

#define MT7530_FC_BLK_UNIT_SIZE		(256 * 2)	// unit=256 bytes*512blocks, but register can set 512bytes*8_bits_register
#define MT7530_FC_BLK_UNIT_SIZE_SHRINK	(128 * 2)	// unit=128 bytes*512blocks, but register can set 256bytes*8_bits_register

/************************************************************************
*                            M A C R O S
*************************************************************************
*/

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/
/*
#ifndef uint32
typedef unsigned long		uint32;
//#define uint32 (unsigned int)
#endif
#ifndef uint16
typedef unsigned short		uint16;
//#define uint16 (unsigned short)
#endif
#ifndef uint8
typedef unsigned char		uint8;
//#define uint8 (unsigned char)
#endif
*/

//
// ISO C Standard C99 update define uint8_t, uint16_t, uint32_t in <stdint.h>
//   but current source code not include this stdint.h yet
/*
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif
#ifndef uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif
*/

typedef struct
{
	mt7530_switch_api_cmdid cmdid;
	u32 PortQueueId;	// port id, or queue id, or ...
	void *paramext_ptr;	// pointer to any paramext struct, or just 32-bit variables
	int ret_value;
} mt7530_switch_api_params;

typedef struct
{
	u32 p1;
	u32 p2;
} mt7530_switch_api_paramext2;

typedef struct
{
	u32 p1;
	u32 p2;
	u32 p3;
	u32 p4;
} mt7530_switch_api_paramext4;

typedef struct
{
	u32 p1;
	u32 p2;
	u32 p3;
	u32 p4;
	u32 p5;
	u32 p6;
} mt7530_switch_api_paramext6;

typedef struct
{
	u32 p1;
	u32 p2;
	u32 p3;
	u32 p4;
	u32 p5;
	u32 p6;
	u32 p7;
	u32 p8;
} mt7530_switch_api_paramext8;

typedef struct
{
	u32 p1;
	u32 p2;
	u32 p3;
	u32 p4;
	u32 p5;
	u32 p6;
	u32 p7;
	u32 p8;
	u32 p9;
} mt7530_switch_api_paramext9;


typedef struct
{
	mt7530_switch_api_MibCntType MibCntType;
	u32 Out__Cnt;
}mt7530_switch_GetMibCnt_param;

typedef struct
{
	u8 index_param;
	u16 MII_BMCR_value;		// MII_BMCR(0x00) : Basic mode control register.
	u16 MII_ADVERTISE_value;	// MII_ADVERTISE(0x04) : Advertisement control register.
	u16 MII_CTRL1000_value;		// MII_CTRL1000(0x09) : 1000BASE-T Control register
} mt7530_switch_MIILinkType;


/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

int macMT7530gswAPIDispatch(struct ifreq *ifr);

int macMT7530GetPortPhyAddr(u8 port);


/************************************************************************
*                        P U B L I C   D A T A
*************************************************************************
*/



/************************************************************************
*                    R E G I S T E R   D E F I N E
*************************************************************************
*/

#define REG_MFC_ADDR			(0x0010)
#define REG_MFC_MIRROR_PORT_OFFT	(0)
#define REG_MFC_MIRROR_PORT_LENG	(3)
#define REG_MFC_MIRROR_PORT_RELMASK	(0x00000007)
#define REG_MFC_MIRROR_PORT_MASK	(REG_MFC_MIRROR_PORT_RELMASK << REG_MFC_MIRROR_PORT_OFFT)
#define REG_MFC_MIRROR_EN_OFFT		(3)
#define REG_MFC_MIRROR_EN_LENG		(1)
#define REG_MFC_MIRROR_EN_RELMASK	(0x00000001)
#define REG_MFC_MIRROR_EN_MASK		(REG_MFC_MIRROR_EN_RELMASK << REG_MFC_MIRROR_EN_OFFT)

#define REG_ATA1_ADDR			(0x0074)
#define REG_ATA2_ADDR			(0x0078)

#define REG_ATWD_ADDR			(0x007C)
#define REG_ATWD_STATUS_OFFT		(2)
#define REG_ATWD_STATUS_LENG		(2)
#define REG_ATWD_STATUS_RELMASK		(0x00000003)
#define REG_ATWD_STATUS_MASK		(REG_ATWD_STATUS_RELMASK << REG_ATWD_STATUS_OFFT)
#define REG_ATWD_PORT_OFFT		(4)
#define REG_ATWD_PORT_LENG		(8)
#define REG_ATWD_PORT_RELMASK		(0x000000FF)
#define REG_ATWD_PORT_MASK		(REG_ATWD_PORT_RELMASK << REG_ATWD_PORT_OFFT)
#define REG_ATWD_LEAKY_EN_OFFT		(12)
#define REG_ATWD_LEAKY_EN_LENG		(1)
#define REG_ATWD_LEAKY_EN_RELMASK	(0x00000001)
#define REG_ATWD_LEAKY_EN_MASK		(REG_ATWD_LEAKY_EN_RELMASK << REG_ATWD_LEAKY_EN_OFFT)
#define REG_ATWD_EG_TAG_OFFT		(13)
#define REG_ATWD_EG_TAG_LENG		(3)
#define REG_ATWD_EG_TAG_RELMASK		(0x00000007)
#define REG_ATWD_EG_TAG_MASK		(REG_ATWD_EG_TAG_RELMASK << REG_ATWD_EG_TAG_OFFT)
#define REG_ATWD_USR_PRI_OFFT		(16)
#define REG_ATWD_USR_PRI_LENG		(3)
#define REG_ATWD_USR_PRI_RELMASK	(0x00000007)
#define REG_ATWD_USR_PRI_MASK		(REG_ATWD_USR_PRI_RELMASK << REG_ATWD_USR_PRI_OFFT)
#define REG_ATWD_SA_MIR_EN_OFFT		(19)
#define REG_ATWD_SA_MIR_EN_LENG		(1)
#define REG_ATWD_SA_MIR_EN_RELMASK	(0x00000001)
#define REG_ATWD_SA_MIR_EN_MASK		(REG_ATWD_SA_MIR_EN_RELMASK << REG_ATWD_SA_MIR_EN_OFFT)
#define REG_ATWD_SA_PORT_FW_OFFT	(20)
#define REG_ATWD_SA_PORT_FW_LENG	(3)
#define REG_ATWD_SA_PORT_FW_RELMASK	(0x00000007)
#define REG_ATWD_SA_PORT_FW_MASK	(REG_ATWD_SA_PORT_FW_RELMASK << REG_ATWD_SA_PORT_FW_OFFT)

#define REG_ATC_ADDR			(0x0080)
#define REG_ATC_AC_CMD_OFFT		(0)
#define REG_ATC_AC_CMD_LENG		(3)
#define REG_ATC_AC_CMD_RELMASK		(0x00000007)
#define REG_ATC_AC_CMD_MASK		(REG_ATC_AC_CMD_RELMASK << REG_ATC_AC_CMD_OFFT)
#define REG_ATC_AC_SAT_OFFT		(4)
#define REG_ATC_AC_SAT_LENG		(2)
#define REG_ATC_AC_SAT_RELMASK		(0x00000003)
#define REG_ATC_AC_SAT_MASK		(REG_ATC_AC_SAT_RELMASK << REG_ATC_AC_SAT_OFFT)
#define REG_ATC_AC_MAT_OFFT		(8)
#define REG_ATC_AC_MAT_LENG		(4)
#define REG_ATC_AC_MAT_RELMASK		(0x0000000F)
#define REG_ATC_AC_MAT_MASK		(REG_ATC_AC_MAT_RELMASK << REG_ATC_AC_MAT_OFFT)
#define REG_AT_SRCH_HIT_OFFT		(13)
#define REG_AT_SRCH_HIT_RELMASK		(0x00000001)
#define REG_AT_SRCH_HIT_MASK		(REG_AT_SRCH_HIT_RELMASK << REG_AT_SRCH_HIT_OFFT)
#define REG_AT_SRCH_END_OFFT		(14)
#define REG_AT_SRCH_END_RELMASK		(0x00000001)
#define REG_AT_SRCH_END_MASK		(REG_AT_SRCH_END_RELMASK << REG_AT_SRCH_END_OFFT)
#define REG_ATC_BUSY_OFFT		(15)
#define REG_ATC_BUSY_LENG		(1)
#define REG_ATC_BUSY_RELMASK		(0x00000001)
#define REG_ATC_BUSY_MASK		(REG_ATC_BUSY_RELMASK << REG_ATC_BUSY_OFFT)
#define REG_AT_ADDR_OFFT		(16)
#define REG_AT_ADDR_LENG		(12)
#define REG_AT_ADDR_RELMASK		(0x00000FFF)
#define REG_AT_ADDR_MASK		(REG_AT_ADDR_RELMASK << REG_AT_ADDR_OFFT)

#define REG_TSRA1_ADDR			(0x0084)
#define REG_TSRA2_ADDR			(0x0088)
#define REG_ATRD_ADDR			(0x008C)

#define REG_GFCCR0_ADDR			(0x1FE0)
#define REG_FC_EN_OFFT			(31)
#define REG_FC_EN_RELMASK		(0x00000001)
#define REG_FC_EN_MASK			(REG_FC_EN_RELMASK << REG_FC_EN_OFFT)

#define REG_SSC_P0_ADDR			(0x2000)

#define REG_PCR_P0_ADDR			(0x2004)
#define REG_PCR_VLAN_MIS_OFFT		(2)
#define REG_PCR_VLAN_MIS_LENG		(1)
#define REG_PCR_VLAN_MIS_RELMASK	(0x00000001)
#define REG_PCR_VLAN_MIS_MASK		(REG_PCR_VLAN_MIS_RELMASK << REG_PCR_VLAN_MIS_OFFT)
#define REG_PCR_ACL_MIR_OFFT		(7)
#define REG_PCR_ACL_MIR_LENG		(1)
#define REG_PCR_ACL_MIR_RELMASK		(0x00000001)
#define REG_PCR_ACL_MIR_MASK		(REG_PCR_ACL_MIR_RELMASK << REG_PCR_ACL_MIR_OFFT)
#define REG_PORT_RX_MIR_OFFT		(8)
#define REG_PORT_RX_MIR_LENG		(1)
#define REG_PORT_RX_MIR_RELMASK		(0x00000001)
#define REG_PORT_RX_MIR_MASK		(REG_PORT_RX_MIR_RELMASK << REG_PORT_RX_MIR_OFFT)
#define REG_PORT_TX_MIR_OFFT		(9)
#define REG_PORT_TX_MIR_LENG		(1)
#define REG_PORT_TX_MIR_RELMASK		(0x00000001)
#define REG_PORT_TX_MIR_MASK		(REG_PORT_TX_MIR_RELMASK << REG_PORT_TX_MIR_OFFT)
#define REG_PCR_EG_TAG_OFFT		(28)
#define REG_PCR_EG_TAG_LENG		(2)
#define REG_PCR_EG_TAG_RELMASK		(0x00000003)
#define REG_PCR_EG_TAG_MASK		(REG_PCR_EG_TAG_RELMASK << REG_PCR_EG_TAG_OFFT)

#define REG_PIC_P0_ADDR			(0x2008)
#define REG_PIC_IGMP_MIR_OFFT		(19)
#define REG_PIC_IGMP_MIR_LENG		(1)
#define REG_PIC_IGMP_MIR_RELMASK	(0x00000001)
#define REG_PIC_IGMP_MIR_MASK		(REG_PIC_IGMP_MIR_RELMASK << REG_PIC_IGMP_MIR_OFFT)

#define REG_PSC_P0_ADDR			(0x200C)

#define REG_PVC_P0_ADDR			(0x2010)
#define REG_PVC_ACC_FRM_OFFT		(0)
#define REG_PVC_ACC_FRM_LENG		(2)
#define REG_PVC_ACC_FRM_RELMASK		(0x00000003)
#define REG_PVC_ACC_FRM_MASK		(REG_PVC_ACC_FRM_RELMASK << REG_PVC_ACC_FRM_OFFT)
#define REG_PVC_EG_TAG_OFFT		(8)
#define REG_PVC_EG_TAG_LENG		(3)
#define REG_PVC_EG_TAG_RELMASK		(0x00000007)
#define REG_PVC_EG_TAG_MASK		(REG_PVC_EG_TAG_RELMASK << REG_PVC_EG_TAG_OFFT)

#define REG_PPBV1_P0_ADDR		(0x2014)
#define REG_PPBV2_P0_ADDR		(0x2018)
#define REG_BSR_P0_ADDR			(0x201C)
#define REG_STAG01_P0_ADDR		(0x2020)
#define REG_STAG23_P0_ADDR		(0x2024)
#define REG_STAG45_P0_ADDR		(0x2028)
#define REG_STAG67_P0_ADDR		(0x202C)

#define REG_CMACCR_ADDR			(0x30E0)
#define REG_MTCC_LMT_OFFT		(9)
#define REG_MTCC_LMT_LENG		(4)
#define REG_MTCC_LMT_RELMASK		(0x0000000F)
#define REG_MTCC_LMT_MASK		(REG_MTCC_LMT_RELMASK << REG_MTCC_LMT_OFFT)



#endif // #ifdef TCSUPPORT_MT7530_SWITCH_API

#endif // #ifndef _GMAC_MT7530_API_H_

