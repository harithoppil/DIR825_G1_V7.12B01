/*
** $Id: //BBN_Linux/Branch/Branch_for_SDK_Release_MultiChip_20111013/tclinux_phoenix/modules/private/raeth/femac.h#7 $
*/
/************************************************************************
 *
 *	Copyright (C) 2008 Trendchip Technologies, Corp.
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
** $Log: femac.h,v $
 */
#ifndef _FEMAC_7510_H
#define _FEMAC_7510_H


/* scatter & gather mode */
//#define TC3262_GMAC_SG_MODE

#define SG_MAX_PKT_LEN	   		(128)

#ifndef LOOPBACK_SUPPORT
#define MAC_TXDESCP_NO			256		/* Max tx buffer Cnts , default=128 */
#else
#define MAC_TXDESCP_NO			128		/* Max tx buffer Cnts , default=128 */
#endif
#define MAC_RXDESCP_NO			128 	/* Max rx buffer Cnts , default=128 */

#define MAC_NAPI_WEIGHT			128     /*32*/

#define MAC_RXDESCP_SIZE		16		/* 4 DWords */
#define MAC_TXDESCP_SIZE		16		/* 4 DWords */

#define TX_BUF_RELEASE_THRESHOLD 4  	/* default:4 */

#ifndef LOOPBACK_SUPPORT
#define TX_QUEUE_NUM 			1	//Only need One queue for MT7510
#else
#define TX_QUEUE_NUM 			4

#endif
#define RX_QUEUE_NUM 			1

#define K0_TO_K1(x)				(((uint32)x) | 0xa0000000)
#define K1_TO_PHY(x)			(((uint32)x) & 0x1fffffff)

#define MAC_STATISTIC_ON		1
#define MAC_STATISTIC_OFF		0

#define GMAC_PRIORITY_MASK  	(TX_QUEUE_NUM-1)

/*****************************
 * Ethernet Module Registers *
 *****************************/

#define FE_BASE     		0xBFB50000
#define FE_DMA_GLO_CFG      (FE_BASE + 0x00)
#define FE_RST_GLO          (FE_BASE + 0x04)
#define FE_INT_STATUS       (FE_BASE + 0x08)
#define FE_INT_ENABLE       (FE_BASE + 0x0c)
#define FOE_TS_T            (FE_BASE + 0x10)
#define IPV6_EXT            (FE_BASE + 0x14)

#define PDMA_BASE     		(FE_BASE + 0x0800)
#define TX_BASE_PTR(n)    	(PDMA_BASE + (n)*0x10 + 0x000)
#define TX_MAX_CNT(n)    	(PDMA_BASE + (n)*0x10 + 0x004)
#define TX_CTX_IDX(n)     	(PDMA_BASE + (n)*0x10 + 0x008)
#define TX_DTX_IDX(n) 		(PDMA_BASE + (n)*0x10 + 0x00C)

#define RX_BASE_PTR(n)     	(PDMA_BASE + (n)*0x10 + 0x100)
#define RX_MAX_CNT(n)  		(PDMA_BASE + (n)*0x10 + 0x104)
#define RX_CALC_IDX(n)     	(PDMA_BASE + (n)*0x10 + 0x108)
#define RX_DRX_IDX(n)      	(PDMA_BASE + (n)*0x10 + 0x10C)

#define PDMA_INFO        	(PDMA_BASE + 0x200)
#define PDMA_GLO_CFG     	(PDMA_BASE + 0x204)
#define PDMA_RST_IDX       	(PDMA_BASE + 0x208)
#define DLY_INT_CFG        	(PDMA_BASE + 0x20C)
#define FREEQ_THRES        	(PDMA_BASE + 0x210)
#define INT_STATUS         	(PDMA_BASE + 0x220)
#define INT_MASK           	(PDMA_BASE + 0x228)
#define SCH_Q01_CFG        	(PDMA_BASE + 0x280)
#define SCH_Q23_CFG        	(PDMA_BASE + 0x284)


#define PSE_BASE     		(FE_BASE + 0x0100)
#define PSE_FQFC_CFG        (PSE_BASE + 0x00)
#define PSE_IQ_REV1         (PSE_BASE + 0x08)
#define PSE_IQ_REV2         (PSE_BASE + 0x0c)
#define PSE_IQ_STA1        	(PSE_BASE + 0x10)
#define PSE_IQ_STA2        	(PSE_BASE + 0x14)
#define PSE_OQ_STA1         (PSE_BASE + 0x18)
#define PSE_OQ_STA2        	(PSE_BASE + 0x1c)
#define PSE_DROP_COUNT_0    (PSE_BASE + 0x80)
#define PSE_DROP_COUNT_1    (PSE_BASE + 0x84)
#define PSE_DROP_COUNT_2    (PSE_BASE + 0x88)
#define PSE_DROP_COUNT_4    (PSE_BASE + 0x90)
#define PSE_DROP_COUNT_5    (PSE_BASE + 0x94)

#define GDMA1_BASE     		(FE_BASE + 0x0500)
#define GDMA1_FWD_CFG       (GDMA1_BASE + 0x00)
#define GDMA1_SHRP_CFG      (GDMA1_BASE + 0x04)
#define GDMA1_MAC_ADRL      (GDMA1_BASE + 0x08)
#define GDMA1_MAC_ADRH      (GDMA1_BASE + 0x0c)
#define GDMA1_VLAN_GEN    (GDMA1_BASE + 0x10)

#define CDMA_BASE     		(FE_BASE + 0x400)
#define CDMA_CSG_CFG        (CDMA_BASE + 0x00)
#define CDMA_PPP_GEN        (CDMA_BASE + 0x04)

#define GDMA2_BASE     		(FE_BASE + 0x1500)
#define GDMA2_FWD_CFG       (GDMA2_BASE + 0x00)
#define GDMA2_SHRP_CFG      (GDMA2_BASE + 0x04)
#define GDMA2_MAC_ADRL      (GDMA2_BASE + 0x08)
#define GDMA2_MAC_ADRH      (GDMA2_BASE + 0x0c)
#define GDMA2_VLAN_CHECK    (GDMA2_BASE + 0x10)


//count define
#define GDMA_COUNT_BASE 	(FE_BASE + 0x2400)
#define GDMA_RX_BYTECNT_L   (GDMA_COUNT_BASE + 0x00)
#define GDMA_RX_BYTECNT_H   (GDMA_COUNT_BASE + 0x04)
#define GDMA_RX_PKTCNT      (GDMA_COUNT_BASE + 0x08)
#define GDMA_RX_OERCNT    	(GDMA_COUNT_BASE + 0x10)
#define GDMA_RX_FCSCNT    	(GDMA_COUNT_BASE + 0x14)
#define GDMA_RX_RUNTCNT    	(GDMA_COUNT_BASE + 0x18)
#define GDMA_RX_LONGCNT   	(GDMA_COUNT_BASE + 0x1c)
#define GDMA_RX_ITUCCNT    	(GDMA_COUNT_BASE + 0x20)
#define GDMA_RX_FCCNT    	(GDMA_COUNT_BASE + 0x24)
#define GDMA_TX_ABORTCNT   	(GDMA_COUNT_BASE + 0x28)
#define GDMA_TX_COLCNT   	(GDMA_COUNT_BASE + 0x2C)
#define GDMA_TX_BYTECNT_L   (GDMA_COUNT_BASE + 0x30)
#define GDMA_TX_BYTECNT_H   (GDMA_COUNT_BASE + 0x34)
#define GDMA_TX_PKTCNT      (GDMA_COUNT_BASE + 0x38)


#define GDMA2_COUNT_BASE 	(FE_BASE + 0x1600)
#define GDMA2_TX_GETCNT     (GDMA2_COUNT_BASE + 0x00)
#define GDMA2_TX_OKCNT     	(GDMA2_COUNT_BASE + 0x4)
#define GDMA2_TX_DROPCNT   	(GDMA2_COUNT_BASE + 0x8)

#define GDMA2_RX_OKCNT     	(GDMA2_COUNT_BASE + 0x50)
#define GDMA2_RX_OVDROPCNT    (GDMA2_COUNT_BASE + 0x54)
#define GDMA2_RX_ERRDROPCNT    (GDMA2_COUNT_BASE + 0x58)
#define GDMA2_RX_ETHERPCNT  (GDMA2_COUNT_BASE + 0x60)
#define GDMA2_RX_ETHDROPCNT (GDMA2_COUNT_BASE + 0x68)
#define GDMA2_RX_ETHBCCNT   (GDMA2_COUNT_BASE + 0x6c)
#define GDMA2_RX_ETHMCCNT   (GDMA2_COUNT_BASE + 0x70)
#define GDMA2_RX_ETHCRCCNT  (GDMA2_COUNT_BASE + 0x74)
#define GDMA2_RX_ETHFRACCNT (GDMA2_COUNT_BASE + 0x78)
#define GDMA2_RX_ETHJABCNT  (GDMA2_COUNT_BASE + 0x7c)
#define GDMA2_RX_ETHRUNTCNT (GDMA2_COUNT_BASE + 0x80)
#define GDMA2_RX_ETHLONGCNT (GDMA2_COUNT_BASE + 0x84)


/********************************
 * Giga Switch Module Registers *
 ********************************/

#define GSW_BASE     		0xBFB58000
#define GSW_ARL_BASE     	(GSW_BASE + 0x0000)
#define GSW_BMU_BASE     	(GSW_BASE + 0x1000)
#define GSW_PORT_BASE     	(GSW_BASE + 0x2000)
#define GSW_MAC_BASE     	(GSW_BASE + 0x3000)
#define GSW_MIB_BASE     	(GSW_BASE + 0x4000)
#define GSW_CFG_BASE     	(GSW_BASE + 0x7000)

#define GSW_MFC     		(GSW_ARL_BASE + 0x10)
#define GSW_PSC(n)     		(GSW_PORT_BASE + (n)*0x100 + 0x0C)

#define GSW_PMCR(n)     	(GSW_MAC_BASE + (n)*0x100)
#define GSW_PMSR(n)     	(GSW_MAC_BASE + (n)*0x100 + 0x08)
#define GSW_PINT_EN(n)     	(GSW_MAC_BASE + (n)*0x100 + 0x10)
#define GSW_SMACCR0     	(GSW_MAC_BASE + 0xe4)
#define GSW_SMACCR1     	(GSW_MAC_BASE + 0xe8)
#define GSW_CKGCR		(GSW_MAC_BASE + 0xf0)

#define GSW_TX_DROC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x00)
#define GSW_TX_CRC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x04)
#define GSW_TX_UNIC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x08)
#define GSW_TX_MULC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x0c)
#define GSW_TX_BROC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x10)
#define GSW_TX_COLC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x14)
#define GSW_TX_SCOLC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x18)
#define GSW_TX_MCOLC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x1c)
#define GSW_TX_DEFC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x20)
#define GSW_TX_LCOLC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x24)
#define GSW_TX_ECOLC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x28)
#define GSW_TX_PAUC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x2c)
#define GSW_TX_OCL(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x48)
#define GSW_TX_OCH(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x4c)

#define GSW_RX_DROC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x60)
#define GSW_RX_FILC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x64)
#define GSW_RX_UNIC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x68)
#define GSW_RX_MULC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x6c)
#define GSW_RX_BROC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x70)
#define GSW_RX_ALIGE(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x74)
#define GSW_RX_CRC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x78)
#define GSW_RX_RUNT(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x7c)
#define GSW_RX_FRGE(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x80)
#define GSW_RX_LONG(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x84)
#define GSW_RX_JABE(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x88)
#define GSW_RX_PAUC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x8c)
#define GSW_RX_OCL(n)    		(GSW_MIB_BASE + (n)*0x100 + 0xa8)
#define GSW_RX_OCH(n)    		(GSW_MIB_BASE + (n)*0x100 + 0xac)
#define GSW_RX_INGC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0xb4)
#define GSW_RX_ARLC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0xb8)

#define EXT_GSW_TX_DROC(n)    		(0x4000 + (n)*0x100)
#define EXT_GSW_TX_CRC(n)    		(0x4004 + (n)*0x100)
#define EXT_GSW_TX_UNIC(n)    		(0x4008 + (n)*0x100)
#define EXT_GSW_TX_MULC(n)    		(0x400c + (n)*0x100)
#define EXT_GSW_TX_BROC(n)    		(0x4010 + (n)*0x100)
#define EXT_GSW_TX_COLC(n)    		(0x4014 + (n)*0x100)
#define EXT_GSW_TX_SCOLC(n)    		(0x4018 + (n)*0x100)
#define EXT_GSW_TX_MCOLC(n)    		(0x401c + (n)*0x100)
#define EXT_GSW_TX_DEFC(n)    		(0x4020 + (n)*0x100)
#define EXT_GSW_TX_LCOLC(n)    		(0x4024 + (n)*0x100)
#define EXT_GSW_TX_ECOLC(n)    		(0x4028 + (n)*0x100)
#define EXT_GSW_TX_PAUC(n)    		(0x402c + (n)*0x100)
#define EXT_GSW_TX_OCL(n)    		(0x4048 + (n)*0x100)
#define EXT_GSW_TX_OCH(n)    		(0x404c + (n)*0x100)

#define EXT_GSW_RX_DROC(n)    		(0x4060 + (n)*0x100)
#define EXT_GSW_RX_FILC(n)    		(0x4064 + (n)*0x100)
#define EXT_GSW_RX_UNIC(n)    		(0x4068 + (n)*0x100)
#define EXT_GSW_RX_MULC(n)    		(0x406c + (n)*0x100)
#define EXT_GSW_RX_BROC(n)    		(0x4070 + (n)*0x100)
#define EXT_GSW_RX_ALIGE(n)    		(0x4074 + (n)*0x100)
#define EXT_GSW_RX_CRC(n)    		(0x4078 + (n)*0x100)
#define EXT_GSW_RX_RUNT(n)    		(0x407c + (n)*0x100)
#define EXT_GSW_RX_FRGE(n)    		(0x4080 + (n)*0x100)
#define EXT_GSW_RX_LONG(n)    		(0x4084 + (n)*0x100)
#define EXT_GSW_RX_JABE(n)    		(0x4088 + (n)*0x100)
#define EXT_GSW_RX_PAUC(n)    		(0x408c + (n)*0x100)
#define EXT_GSW_RX_OCL(n)    		(0x40a8 + (n)*0x100)
#define EXT_GSW_RX_OCH(n)    		(0x40ac + (n)*0x100)
#define EXT_GSW_RX_INGC(n)    		(0x40b4 + (n)*0x100)
#define EXT_GSW_RX_ARLC(n)    		(0x40b8 + (n)*0x100)

#define GSW_PVC(n)     	(0x2010 + (n)*0x100)
#define DEFAULT_TPID  	(0x8100)

#define GSW_CFG_PPSC     	(GSW_CFG_BASE + 0x18)
#define GSW_CFG_PIAC     	(GSW_CFG_BASE + 0x1c)
#define GSW_CFG_GPC     	(GSW_CFG_BASE + 0x14)


#define GSW_VLAN_REG		(GSW_BASE+0x94)
#define GSW_ATA1_REG		(GSW_BASE+0x74)
#define GSW_ATC_REG			(GSW_BASE+0x80)
/***************************************
 * Ethernet Module Register Definition *
 ***************************************/

/* RSTCTRL2 */
#define EPHY_RST						(1<<1)//no EPHY RESET, SET to the same with PDMA_REST
#define ESW_RST							(1<<23)
#define FE_RST							(1<<1)//means PDMA RESET

/* FE_VLAN_ID */
#define VLAN_ID1_SHIFT   				(16)
#define VLAN_IDl       					(0xfff<<VLAN_ID1_SHIFT)
#define VLAN_ID0_SHIFT   				(0)
#define VLAN_ID0       					(0xfff<<VLAN_ID0_SHIFT)

/* PDMA_GLO_CFG */
#define RX_2BYTE_OFFSET					(1<<31)
#define CSR_CLKGATE						(1<<30)
#define PDMA_BYTE_SWAP    				(1<<29)
#define PDMA_BIG_ENDIAN    				(1<<7)
#define TX_WB_DDONE       				(1<<6)
#define PDMA_BT_SIZE_SHIFT  			(4)
#define PDMA_BT_SIZE       				(0x3<<PDMA_BT_SIZE_SHIFT)
#define RX_DMA_BUSY       				(1<<3)
#define RX_DMA_EN         				(1<<2)
#define TX_DMA_BUSY       				(1<<1)
#define TX_DMA_EN         				(1<<0)

#define PDMA_BT_SIZE_4DW   				(0)
#define PDMA_BT_SIZE_8DW   				(1)
#define PDMA_BT_SIZE_16DW   			(2)
#define PDMA_BT_SIZE_32DW   			(3)

/* PDMA_RST_IDX */
#define RST_DRX_IDX(n)					(1<<(16+(n)))
#define RST_DTX_IDX(n)					(1<<(n))

/* DLY_INT_CFG */
#define TXDLY_INT_EN					(1<<31)
#define TXMAX_PINT_SHIFT				(24)
#define TXMAX_PINT						(0x7f<<TXMAX_PINT_SHIFT)
#define TXMAX_PTIME_SHIFT				(16)
#define TXMAX_PTIME						(0xff<<TXMAX_PTIME_SHIFT)
#define RXDLY_INT_EN					(1<<15)
#define RXMAX_PINT_SHIFT				(8)
#define RXMAX_PINT						(0x7f<<RXMAX_PINT_SHIFT)
#define RXMAX_PTIME_SHIFT				(0)
#define RXMAX_PTIME						(0xff<<RXMAX_PTIME_SHIFT)

/* INT_STATUS or INT_MASK */
#define RX_COHERENT						(1<<31)
#define RX_DLY_INT						(1<<30)
#define TX_COHERENT						(1<<29)
#define TX_DLY_INT						(1<<28)
#define RX_DONE_INT1					(1<<17)
#define RX_DONE_INT0					(1<<16)
#define TX_DONE_INT3					(1<<3)
#define TX_DONE_INT2					(1<<2)
#define TX_DONE_INT1					(1<<1)
#define TX_DONE_INT0					(1<<0)

/* SCH_Q01_CFG */
#define MAX_BLK_SIZE1					(1<<31)
#define MAX_RATE_ULMT1					(1<<30)
#define MAX_WEIGHT1_SHIFT				(28)
#define MAX_WEIGHT1						(0x3<<MAX_WEIGHT1_SHIFT)
#define MIN_RATE_RATIO1_SHIFT			(26)
#define MIN_RATE_RATIO1					(0x3<<MIN_RATE_RATIO1_SHIFT)
#define MAX_RATE1_SHIFT					(16)
#define MAX_RATE1						(0x3ff<<MAX_RATE_RATIO1_SHIFT)
#define MAX_BLK_SIZE0					(1<<15)
#define MAX_RATE_ULMT0					(1<<14)
#define MAX_WEIGHT0_SHIFT				(12)
#define MAX_WEIGHT0						(0x3<<MAX_WEIGHT0_SHIFT)
#define MIN_RATE_RATIO0_SHIFT			(10)
#define MIN_RATE_RATIO0					(0x3<<MIN_RATE_RATIO0_SHIFT)
#define MAX_RATE0_SHIFT					(0)
#define MAX_RATE0						(0x3ff<<MAX_RATE0_SHIFT)

/* SCH_Q23_CFG */
#define MAX_BLK_SIZE3					(1<<31)
#define MAX_RATE_ULMT3					(1<<30)
#define MAX_WEIGHT3_SHIFT				(28)
#define MAX_WEIGHT3						(0x3<<MAX_WEIGHT3_SHIFT)
#define MIN_RATE_RATIO3_SHIFT			(26)
#define MIN_RATE_RATIO3					(0x3<<MIN_RATE_RATIO3_SHIFT)
#define MAX_RATE3_SHIFT					(16)
#define MAX_RATE3						(0x3ff<<MAX_RATE_RATIO3_SHIFT)
#define MAX_BLK_SIZE2					(1<<15)
#define MAX_RATE_ULMT2					(1<<14)
#define MAX_WEIGHT2_SHIFT				(12)
#define MAX_WEIGHT2						(0x3<<MAX_WEIGHT2_SHIFT)
#define MIN_RATE_RATIO2_SHIFT			(10)
#define MIN_RATE_RATIO2					(0x3<<MIN_RATE_RATIO2_SHIFT)
#define MAX_RATE2_SHIFT					(0)
#define MAX_RATE2						(0x3ff<<MAX_RATE2_SHIFT)

#define MAX_WEIGHT_1023 				(0)
#define MAX_WEIGHT_2047 				(1)
#define MAX_WEIGHT_4095 				(2)
#define MAX_WEIGHT_8191 				(3)

#define MIN_RATIO0 						(0)
#define MIN_RATIO1 						(1)
#define MIN_RATIO2 						(2)
#define MIN_RATIO3 						(3)

/* GDMA1_FWD_CFG or GDMA2_FWD_CFG */
#define GDM_JMB_LEN_SHIFT				(28)
#define GDM_JMB_LEN						(0xf<<GDM_JMB_LEN_SHIFT)
#define GDM_20US_TICK_SLT				(1<<25)

#define GDM_INSV_EN						(1<<26)
#define GDM_UNTAG_EN					(1<<25)
#define GDM_STAG_EN						(1<<24)
#define GDM_ICS_EN						(1<<22)
#define GDM_TCS_EN						(1<<21)
#define GDM_UCS_EN						(1<<20)
#define GDM_DROP_256B					(1<<19)
#define GDM_DISPAD						(1<<18)
#define GDM_DISCRC						(1<<17)
#define GDM_STRPCRC						(1<<16)
#define GDM_UFRC_P_SHIFT				(12)
#define GDM_UFRC_P						(0xf<<GDM_UFRC_P_SHIFT)
#define GDM_BFRC_P_SHIFT				(8)
#define GDM_BFRC_P						(0xf<<GDM_BFRC_P_SHIFT)
#define GDM_MFRC_P_SHIFT				(4)
#define GDM_MFRC_P						(0xf<<GDM_MFRC_P_SHIFT)
#define GDM_OFRC_P_SHIFT				(0)
#define GDM_OFRC_P						(0xf<<GDM_MFRC_P_SHIFT)

#define GDM2_UNDERRUN_RETRY				(1<<27)
#define GDM2_DROP_256B					(1<<26)
#define GDM2_DROP_LONG					(1<<25)
#define GDM2_DROP_RUNT					(1<<24)
#define GDM2_DROP_CRC_ERR				(1<<23)
#define GDM2_JMB_EN						(1<<19)


/* define GDMA port */
#define GDM_P_PDMA						(0x0)
#define GDM_P_GDMA1						(0x1)
#define GDM_P_GDMA2						(0x2)
#define GDM_P_PPE						(0x4)
#define GDM_P_QDMA						(0x5)
#define GDM_P_DISCARD					(0x7)
#define GDM_P_CPU						GDM_P_PDMA


/* GDMA1_SCH_CFG or GDMA2_SCH_CFG */
#define GDM_SCH_MOD_SHIFT				(24)
#define GDM_SCH_MOD						(0x3<<GDM_SCH_MOD_SHIFT)
#define GDM_WT_Q3_SHIFT					(12)
#define GDM_WT_Q3						(0x7<<GDM_WT_Q3_SHIFT)
#define GDM_WT_Q2_SHIFT					(8)
#define GDM_WT_Q2						(0x7<<GDM_WT_Q2_SHIFT)
#define GDM_WT_Q1_SHIFT					(4)
#define GDM_WT_Q1						(0x7<<GDM_WT_Q1_SHIFT)
#define GDM_WT_Q0_SHIFT					(0)
#define GDM_WT_Q0						(0x7<<GDM_WT_Q0_SHIFT)

#define GDM_SCH_MOD_WRR					(0)
#define GDM_SCH_MOD_SP					(1)

#define GDM_WT(n)						((n>=8) ? 7 : ((n)-1)&0x7)

/* CDMA_CSG_CFG */
#define INS_VLAN_SHIFT					(16)
#define INS_VLAN						(0xffff<<INS_VLAN_SHIFT)
#define CDM_STAG_EN							(1<<0)

/* GSW_MFC */
#define MFC_BC_FFP_SHIFT				(24)
#define MFC_BC_FFP						(0xff<<MFC_BC_FFP_SHIFT)
#define MFC_UNM_FFP_SHIFT				(16)
#define MFC_UNM_FFP						(0xff<<MFC_UNM_FFP_SHIFT)
#define MFC_UNU_FFP_SHIFT				(8)
#define MFC_UNU_FFP						(0xff<<MFC_UNU_FFP_SHIFT)
#define MFC_CPU_EN						(1<<7)
#define MFC_CPU_PORT_SHIFT				(4)
#define MFC_CPU_PORT					(0x7<<MFC_CPU_PORT_SHIFT)
#define MFC_MIRROR_EN					(1<<3)
#define MFC_MIRROR_PORT_SHIFT			(0)
#define MFC_MIRROT_PORT					(0x7<<MFC_MIRROR_PORT_SHIFT)

/* GSW_PMCR */
#define IPG_CFG_PN_SHIFT				(18)
#define IPG_CFG_PN						(0x3<<IPG_CFG_PN_SHIFT)
#define EXT_PHY_PN						(1<<17)
#define MAC_MODE_PN						(1<<16)
#define FORCE_MODE_PN					(1<<15)
#define MAC_TX_EN_PN					(1<<14)
#define MAC_RX_EN_PN					(1<<13)
#define RGMII_MODE_PN					(1<<12)
#define BKOFF_EN_PN						(1<<9)
#define BACKPR_EN_PN					(1<<8)
#define ENABLE_EEE1G_PN					(1<<7)
#define ENABLE_EEE100_PN				(1<<6)
#define ENABLE_RX_FC_PN					(1<<5)
#define ENABLE_TX_FC_PN					(1<<4)
#define FORCE_SPD_PN_SHIFT				(2)
#define FORCE_SPD_PN					(0x3<<FORCE_SPD_PN_SHIFT)
#define FORCE_DPX_PN					(1<<1)
#define FORCE_LNK_PN					(1<<0)

#define IPG_CFG_NORMAL					(0)
#define IPG_CFG_SHORT					(1)

#define PN_SPEED_10M					(0)
#define PN_SPEED_100M					(1)
#define PN_SPEED_1000M					(2)

/* GSW_PMSR */
#define EEE1G_STS						(1<<7)
#define EEE100_STS						(1<<6)
#define RX_FC_STS						(1<<5)
#define TX_FC_STS						(1<<4)
#define MAC_SPD_STS_SHIFT				(2)
#define MAC_SPD_STS						(0x3<<MAC_SPD_STS_SHIFT)
#define MAC_DPX_STS						(1<<1)
#define MAC_LINK_STS					(1<<0)


/* GSW_CFG_PPSC */
#define PHY_AP_EN_SHIFT					(1<<24)
#define PHY_AP_EN						(0x7f<<PHY_END_ADDR_SHIFT)

#define PHY_EEE_EN_SHIFT				(1<<16)
#define PHY_EEE_EN						(0x7f<<PHY_END_ADDR_SHIFT)

#define PHY_PRE_EN						(1<<15)
#define PHY_END_ADDR_SHIFT				(8)
#define PHY_END_ADDR					(0x1f<<PHY_END_ADDR_SHIFT)
#define PHY_MDC_CFG_SHIFT				(1<<6)
#define PHY_MDC_CFG						(0x3<<PHY_MDC_CFG_SHIFT)
#define PHY_ST_ADDR_SHIFT				(0)
#define PHY_ST_ADDR						(0x1f<<PHY_ST_ADDR_SHIFT)

/* GSW_CFG_PIAC */
#define PHY_ACS_ST						(1<<31)
#define MDIO_REG_ADDR_SHIFT				(25)
#define MDIO_REG_ADDR					(0x1f<<MDIO_REG_ADDR_SHIFT)
#define MDIO_PHY_ADDR_SHIFT				(20)
#define MDIO_PHY_ADDR					(0x1f<<MDIO_PHY_ADDR_SHIFT)
#define MDIO_CMD_SHIFT					(18)
#define MDIO_CMD						(0x3<<MDIO_CMD_SHIFT)
#define MDIO_ST_SHIFT					(16)
#define MDIO_ST							(0x3<<MDIO_ST_SHIFT)
#define MDIO_RW_DATA_SHIFT				(0)
#define MDIO_RW_DATA					(0xffff<<MDIO_RW_DATA_SHIFT)

#define PHY_ACS_ST_START				(1)
#define MDIO_CMD_WRITE					(1)
#define MDIO_CMD_READ					(2)
#define MDIO_ST_START					(1)

#define MDIO_CL45_CMD_ADDR				(0)
#define MDIO_CL45_CMD_WRITE				(1)
#define MDIO_CL45_CMD_READ				(3)
#define MDIO_CL45_CMD_POSTREAD_INCADDR			(2)
#define MDIO_CL45_ST_START				(0)

#define NORMAL_READ					(1<<0)
#define POST_READ					(1<<1)

/* GSW_CFG_GPC */
#define RX_CLK_MODE						(1<<2)

//FPORT DEFINE
#define DPORT_PDMA				0
#define DPORT_GDMA1				1
#define DPORT_GDMA2				2//means wan , like SAR/PTM/xPON
#define DPORT_PPE				4
#define DPORT_QDMA				5//send to CPU via QDMA
#define DPORT_QDMA_HW			6//send to CPU via QDMA HW
#define DPORT_DISCARD			7
#define DPORT_CPU				DPORT_PDMA

//PKT_INFO define
#define IPV6_H						(1<<5)
#define IPV4_H						(1<<4)
#define IPV4_H_INV					(1<<3)
#define TU_H						(1<<1)
#define TU_H_C_INV					(1<<0)


#ifdef MT7510_DMA_DSCP_CACHE
#define TXDSP_PADDR_START_BASE	0xbfb00864
#define TXDSP_VADDR_START_BASE	(TXDSP_PADDR_START_BASE+0x4)
#define TXDSP_VADDR_END_BASE	(TXDSP_PADDR_START_BASE+0x8)
#define RXDSP_PADDR_START_BASE	(TXDSP_PADDR_START_BASE+0xc)
#define RXDSP_VADDR_START_BASE	(TXDSP_PADDR_START_BASE+0x10)
#define RXDSP_VADDR_END_BASE	(TXDSP_PADDR_START_BASE+0x14)

#define CACHE_LINE_VADDR_OFFSET 0x8000000 //128M

#define TO_CACHE_LINE_VADDR(x)	((x) & (~(0x1 << 31)))  //Use virtual address > 128M, and clear the 31 bit
#define	CACHE_LINE_SIZE			(32)

#define PHY_TO_K0(x)				(((uint32)x) | 0x80000000)


#endif

/*=========================================
      PDMA RX Descriptor Format define
=========================================*/

//-------------------------------------------------

typedef union {
	struct _PDMA_RXD_INFO1_
	{
		unsigned int    PDP0;
	} bits;
	uint32 word;
} PDMA_RXD_INFO1_T;
//-------------------------------------------------

typedef union {
	struct _PDMA_RXD_INFO2_
	{
#ifdef __BIG_ENDIAN
		unsigned int    DDONE_bit             : 1;
		unsigned int    LS0                   : 1;
		unsigned int    PLEN0                 : 14;
		unsigned int    UN_USED               : 1;
		unsigned int    LS1                   : 1;
		unsigned int    PLEN1                 : 14;
#else
		unsigned int    PLEN1                 : 14;
		unsigned int    LS1                   : 1;
		unsigned int    UN_USED               : 1;
		unsigned int    PLEN0                 : 14;
		unsigned int    LS0                   : 1;
		unsigned int    DDONE_bit             : 1;
#endif
	} bits;
	uint32 word;
} PDMA_RXD_INFO2_T;
//-------------------------------------------------

typedef union {
	struct _PDMA_RXD_INFO3_
	{
		unsigned int    UN_USE1;
	} bits;
	uint32 word;
} PDMA_RXD_INFO3_T;
//-------------------------------------------------

typedef union {
	struct _PDMA_RXD_INFO4_
	{
#ifdef __BIG_ENDIAN
		unsigned int    RSV           		: 3;
		unsigned int    PKT_INFO           	: 6;
		unsigned int    SPORT               : 4;
		unsigned int    CRSN                : 5;
		unsigned int    FOE_Entry           : 14;
#else
		unsigned int    FOE_Entry           : 14;
		unsigned int    CRSN                : 5;
		unsigned int    SPORT               : 4;
		unsigned int    PKT_INFO           	: 6;
		unsigned int    RSV           		: 3;
#endif
	} bits;
	uint32 word;
} PDMA_RXD_INFO4_T;

struct PDMA_rxdesc {
	PDMA_RXD_INFO1_T rxd_info1;
	PDMA_RXD_INFO2_T rxd_info2;
	PDMA_RXD_INFO3_T rxd_info3;
	PDMA_RXD_INFO4_T rxd_info4;
	#ifdef MT7510_DMA_DSCP_CACHE
	uint32	dummy[4];
	#endif
};

typedef struct PDMA_rxdesc macRxDescr_t;

/*=========================================
      PDMA TX Descriptor Format define
=========================================*/
//-------------------------------------------------

typedef union {
	struct _PDMA_TXD_INFO1_
	{
		unsigned int    SDP0;
	} bits;
	uint32 word;
} PDMA_TXD_INFO1_T;
//-------------------------------------------------

typedef union {
	struct _PDMA_TXD_INFO2_
	{
#ifdef __BIG_ENDIAN
		unsigned int    DDONE_bit             : 1;
		unsigned int    LS0_bit               : 1;
		unsigned int    SDL0                  : 14;
		unsigned int    BURST_bit             : 1;
		unsigned int    LS1_bit               : 1;
		unsigned int    SDL1                  : 14;
#else
		unsigned int    SDL1                  : 14;
		unsigned int    LS1_bit               : 1;
		unsigned int    BURST_bit             : 1;
		unsigned int    SDL0                  : 14;
		unsigned int    LS0_bit               : 1;
		unsigned int    DDONE_bit             : 1;
#endif
	} bits;
	uint32 word;
} PDMA_TXD_INFO2_T;
//-------------------------------------------------

typedef union {
	struct _PDMA_TXD_INFO3_
	{
		unsigned int    SDP1;
	} bits;
	uint32 word;
} PDMA_TXD_INFO3_T;
//-------------------------------------------------

typedef union {
	struct _PDMA_TXD_INFO4_
	{
#ifdef __BIG_ENDIAN
		unsigned int    ICO		        	: 1;
		unsigned int    UCO					: 1;
		unsigned int    TCO					: 1;
		unsigned int    TSO                 : 1;
		unsigned int    UDF					: 6;
		unsigned int    PN             		: 3;//the same with FPORT
		unsigned int    INSV                : 1;
		unsigned int    TPID             	: 2;
		unsigned int    VPRI                : 3;
		unsigned int    CFI                 : 1;
		unsigned int    VIDX                : 12;
#else
		unsigned int    VIDX                : 12;
		unsigned int    CFI                 : 1;
		unsigned int    VPRI                : 3;
		unsigned int    TPID             	: 2;
		unsigned int    INSV                : 1;
		unsigned int    PN             		: 3;//the same with FPORT
		unsigned int    UDF					: 6;
		unsigned int    TSO                 : 1;
		unsigned int    TCO					: 1;
		unsigned int    UCO					: 1;
		unsigned int    ICO		        	: 1;
#endif

	} bits;
	uint32 word;
} PDMA_TXD_INFO4_T;

struct PDMA_txdesc {
	PDMA_TXD_INFO1_T txd_info1;
	PDMA_TXD_INFO2_T txd_info2;
	PDMA_TXD_INFO3_T txd_info3;
	PDMA_TXD_INFO4_T txd_info4;
	#ifdef MT7510_DMA_DSCP_CACHE
	uint32	dummy[4];
	#endif
};

typedef struct PDMA_txdesc macTxDescr_t;

#if (0)	// move to tcswitch.h
/******************************
 * Giga Switch IOCTL Commands *
 ******************************/

/* ioctl commands */
#define RAETH_GSW_REG_READ		0x89F1
#define RAETH_GSW_REG_WRITE		0x89F2
#define RAETH_REG_READ			0x89F3
#define RAETH_REG_WRITE			0x89F4
#define RAETH_GSW_PHY_READ		0x89F5
#define RAETH_GSW_PHY_WRITE		0x89F6

#define RAETH_GSWEXT_REG_READ		0x89F7 //MTK120625 ///YM
#define RAETH_GSWEXT_REG_WRITE		0x89F8 //MTK120625 ///YM
#define RAETH_GSWEXT_PHY_READ		0x89F9 //MTK120625 ///YM
#define RAETH_GSWEXT_PHY_WRITE		0x89FA //MTK120625 ///YM

#ifdef TCSUPPORT_MT7530_SWITCH_API
#define RAETH_GSW_CTLAPI		0x89FB
#endif
#endif

typedef struct _gsw_reg {
	__u32 off;
	__u32 val;
} gsw_reg;

/*********************
 * Mac Control Block *
 *********************/

/* ----- MIB-II ----- */
typedef struct macMIB_II_s {
	uint32 inOctets;			/* Receive Octets */
	uint32 inUnicastPkts;		/* Receive Unicast Packets */
	uint32 inMulticastPkts;		/* Receive Multicast Packets */
	uint32 inDiscards;			/* Receive Discard Packets */
	uint32 inErrors;			/* Receive Error Packets */
	uint32 inUnknownProtocols;	/* Receive Unknown Prototol Packets */
	uint32 outOctets;			/* Transmit Octets */
	uint32 outUnicastPkts;		/* Transmit Unicast Packets */
	uint32 outMulticastPkts;	/* Transmit Multicast Packets */
	uint32 outDiscards;			/* Transmit Discard Packets */
	uint32 outErrors;			/* Transmit Error Packets */
} macMIB_II_t;

/* ----- Adapter Statistics ----- */
typedef struct inSiliconStat_s {
	uint32 txJabberTimeCnt;
	uint32 txLossOfCarrierCnt;
	uint32 txNoCarrierCnt;
	uint32 txLateCollisionCnt;
	uint32 txExCollisionCnt;
	uint32 txHeartbeatFailCnt;
	uint32 txCollisionCnt;
	uint32 txExDeferralCnt;
	uint32 txUnderRunCnt;

	uint32 rxAlignErr;
	uint32 rxSymbolErr;
	uint32 rxMiiErr;
	uint32 rxCrcErr;
	uint32 rxEtherFrameLengthErr; /* 60 > size(Packet) or 1518 < size(Packet) */
	uint32 rx802p3FrameLengthErr; /* value of length field of 802.3 packet is
	                                 larger than real packet payload */
	uint32 rxDribblingErr;
	uint32 rxRuntErr;
	uint32 rxLongErr;
	uint32 rxCollisionErr;

	uint32 rxPktIPChkSumErr;

	uint32 rxEnQueueNum;          /* Number of packets enqueued in macRxRingproc() */
	uint32 rxDeQueueNum;          /* Number of packets dequeued in macRxToUpperTask() */
	uint32 txEnQueueNum;          /* Number of packets enqueued in macSend() */
  	uint32 txDeQueueNum;          /* Number of packets dequeued in macTxRingproc() */
} inSiliconStat_t;


/* ----- Ethernet Link Profile ----- */
typedef struct macPhyLinkProfile_s {
	uint32 linkSpeed;							/* 10Mbps or 100Mbps */
	uint32 duplexMode;							/* Half/Full Duplex Mode */
	uint32 enetMode;
	uint32 ANCompFlag;							/* auto_negotiation complete Flag */
	uint32 PollCount;							/* auto_negotiation polling check count */
} macPhyLinkProfile_t;

/* ----- Ethernet private memory pool ----- */
typedef struct macRxMemPool_s {
	macRxDescr_t rxDescpBuf[MAC_RXDESCP_NO];
} macRxMemPool_t;

typedef struct macTxMemPool_s {
	macTxDescr_t txDescpBuf[TX_QUEUE_NUM][MAC_TXDESCP_NO];
} macTxMemPool_t;

/* ----- Statistics for GMAC ----- */
typedef struct macStat_s {
	macMIB_II_t MIB_II;	/* MIB-II */
	inSiliconStat_t inSilicon;
} macStat_t;

/* ----- Adapter Card Table ------ */
typedef struct macAdapter_s {
  	uint8  macAddr[6];  /* MAC-Address */

  	macPhyLinkProfile_t *macPhyLinkProfile_p;

  	macRxMemPool_t      *macRxMemPool_p;
  	dma_addr_t        	macRxMemPool_phys_p;
  	macTxMemPool_t      *macTxMemPool_p;
  	dma_addr_t        	macTxMemPool_phys_p;

  	macStat_t           macStat;
  	uint8 statisticOn;  /* Flag to record statistics or not */

  	uint32 resetNum;    /* Number of Reset the LAN Cont. */
  	uint32 enetPhyAddr;
  	uint8  enetPhyId;

  	uint32 txDescrRingBaseVAddr[TX_QUEUE_NUM];  /* Transmit Descr Ring Virtual Address */
  	uint32 rxDescrRingBaseVAddr;  			  /* Receive Descr Ring Virtual Address */
  	uint32 txCurrentDescp[TX_QUEUE_NUM];        /* index to current tx descriptor */
  	uint32 txUnReleasedDescp[TX_QUEUE_NUM];     /* index to the unreleased descp of Tx */
  	uint32 txUnReleasedBufCnt[TX_QUEUE_NUM];    /* Unreleased buffer cnt hold by Tx	*/
  	uint32 rxCurrentDescp;        			  /* index to current rx descriptor */

  	uint32 rxRingSize;  /* Receive Descr Ring Size */
  	uint32 txRingSize;  /* Receive Descr Ring Size */
  	uint32 rxDescrSize; /* The size of a Rx descr entry in the descr ring */
  	uint32 txDescrSize; /* The size of a Tx descr entry in the descr ring */

	struct sk_buff *txskbs[TX_QUEUE_NUM][MAC_TXDESCP_NO];
	struct sk_buff *rxskbs[MAC_RXDESCP_NO];
#ifdef TC3262_GMAC_SG_MODE
	struct sk_buff *sg_rxskbs[MAC_RXDESCP_NO];
#endif

	struct mii_if_info mii_if;

  	spinlock_t lock;
  	struct net_device_stats stats;
#if KERNEL_2_6_36
	struct net_device	*dev;
	struct napi_struct	napi;
#endif
} macAdapter_t;

typedef struct phyDeviceList_s {
	uint16 companyId;
	char vendorName[30];
} phyDeviceList_t;


/************************************************************************
*        F U N C T I O N   P R O T O T Y P E S
*************************************************************************
*/

uint32 calcnt(uint32 basereg);

#endif /* _FEMAC_H */

