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

#ifndef _GEMAC_H
#define _GEMAC_H

#include <linux/version.h>
#include <linux/mii.h>

//#define CONFIG_ETHERNET_DEBUG             1
#ifdef CONFIG_ETHERNET_DEBUG
#define	etdebug(fmt, args...) printk(fmt, ##args)
#define DUMP_RX_UNTAG                     0x01
#define DUMP_RX_TAG                       0x02
#define DUMP_TX_UNTAG                     0x04
#define DUMP_TX_TAG                       0x08
#define DUMP_ALL_UNTAG                    0x05
#define DUMP_ALL_TAG                      0x0A
#define DUMP_ALL                          0x0F
#else
#define	etdebug(fmt, args...)
#endif

#define SWITCH_REG_ENTRY                 "tc3162/switch_reg"
#define KERNEL_2_6_36 		(LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31))

//#define TCSUPPORT_WAN_ETHER 1
#define TC_CONSOLE_ENABLE       1

#ifdef TC_CONSOLE_ENABLE
#define SKBUF_TCCONSOLE         (1 << 27)
#endif

#if defined(TCSUPPORT_WAN_ETHER)
int isComeFromWan(struct sk_buff *skb);
void setComeFromWan(struct sk_buff *skb, int yes);
#define CB_MAGIC	0x3262
#endif

#define SG_MAX_PKT_LEN	   		(128)

#define MAC_TXDESCP_NO			128		/* Max tx buffer Cnts , default=128 */
#define MAC_RXDESCP_NO			128 	/* Max rx buffer Cnts , default=128 */

#define MAC_NAPI_WEIGHT			32

#define MAC_RXDESCP_SIZE		16		/* 4 DWords */
#define MAC_TXDESCP_SIZE		16		/* 4 DWords */

#define TX_BUF_RELEASE_THRESHOLD 4  	/* default:4 */

#define TX_QUEUE_NUM 			4 
#define RX_QUEUE_NUM 			2 

#define K0_TO_K1(x)				(((unsigned int)x) | 0xa0000000)
#define K1_TO_PHY(x)			(((unsigned int)x) & 0x1fffffff)

#define MAC_STATISTIC_ON		1
#define MAC_STATISTIC_OFF		0

#define RAETH_CHECKSUM_OFFLOAD
#define VLAN_TAG_USED           0


#define GMAC_PRIORITY_MASK  	(TX_QUEUE_NUM-1)

/* auto detect 4 port switch ic */
#define SWIC_ADM6996M		1
#define SWIC_IP175C			2
#define SWIC_MARVEL6060     3
#define SWIC_RTL8305        4
#define SWIC_RTL8306SD		5
#define SWIC_TC2206	        7
#define SWIC_RT63365		8
#define SWIC_RT62806		9


/*****************************
 * Ethernet Module Registers *
 *****************************/
#define FE_BASE     		0xBFB50000
#define MDIO_ACCESS         (FE_BASE + 0x00)
#define MDIO_CFG            (FE_BASE + 0x04)
#define FE_DMA_GLO_CFG      (FE_BASE + 0x08)
#define FE_RST_GLO          (FE_BASE + 0x0C)
#define FE_INT_STATUS       (FE_BASE + 0x10)
#define FE_INT_ENABLE       (FE_BASE + 0x14)
#define FC_DROP_STA         (FE_BASE + 0x18)
#define FOE_TS_T            (FE_BASE + 0x1C)
#define FE_VLAN_ID(n)       (FE_BASE + 0xa8 + (n)*4)

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

#define GDMA1_BASE     		(FE_BASE + 0x0020)
#define GDMA1_FWD_CFG       (GDMA1_BASE + 0x00)
#define GDMA1_SCH_CFG       (GDMA1_BASE + 0x04)
#define GDMA1_SHRP_CFG      (GDMA1_BASE + 0x08)
#define GDMA1_MAC_ADRL      (GDMA1_BASE + 0x0C)
#define GDMA1_MAC_ADRH      (GDMA1_BASE + 0x10)

#define PSE_BASE     		(FE_BASE + 0x0040)
#define PSE_FQFC_CFG        (PSE_BASE + 0x00)
#define CDMA_FC_CFG         (PSE_BASE + 0x04)
#define GDMA1_FC_CFG        (PSE_BASE + 0x08)
#define GDMA2_FC_CFG        (PSE_BASE + 0x0C)
#define CDMA_OQ_STA         (PSE_BASE + 0x10)
#define GDMA1_OQ_STA        (PSE_BASE + 0x14)
#define GDMA2_OQ_STA        (PSE_BASE + 0x18)
#define PSE_IQ_STA          (PSE_BASE + 0x1C)

#define GDMA2_BASE     		(FE_BASE + 0x0060)
#define GDMA2_FWD_CFG       (GDMA2_BASE + 0x00)
#define GDMA2_SCH_CFG       (GDMA2_BASE + 0x04)
#define GDMA2_SHRP_CFG      (GDMA2_BASE + 0x08)
#define GDMA2_MAC_ADRL      (GDMA2_BASE + 0x0C)
#define GDMA2_MAC_ADRH      (GDMA2_BASE + 0x10)

#define CDMA_BASE     		(FE_BASE + 0x0080)
#define CDMA_CSG_CFG        (CDMA_BASE + 0x00)
#define CDMA_SCH_CFG        (CDMA_BASE + 0x04)

#define PDMA_FC_CFG1		(FE_BASE + 0x01ec)
#define PDMA_FC_CFG2		(FE_BASE + 0x01f0)

#define GDMA_CNT_BASE  		(FE_BASE + 0x0400)
#define GDMA_TX_GBCNT1     	(GDMA_CNT_BASE + 0x300)
#define GDMA_TX_GPCNT1     	(GDMA_CNT_BASE + 0x304)
#define GDMA_TX_SKIPCNT1   	(GDMA_CNT_BASE + 0x308)
#define GDMA_TX_COLCNT1   	(GDMA_CNT_BASE + 0x30C)
#define GDMA_RX_GBCNT1     	(GDMA_CNT_BASE + 0x320)
#define GDMA_RX_GPCNT1     	(GDMA_CNT_BASE + 0x324)
#define GDMA_RX_OERCNT1    	(GDMA_CNT_BASE + 0x328)
#define GDMA_RX_FERCNT1    	(GDMA_CNT_BASE + 0x32C)
#define GDMA_RX_SERCNT1    	(GDMA_CNT_BASE + 0x330)
#define GDMA_RX_LERCNT1    	(GDMA_CNT_BASE + 0x334)
#define GDMA_RX_CERCNT1    	(GDMA_CNT_BASE + 0x338)
#define GDMA_RX_FCCNT1    	(GDMA_CNT_BASE + 0x33C)

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

#define GSW_AGC             (GSW_ARL_BASE + 0x0C)
#define GSW_MFC     		(GSW_ARL_BASE + 0x10)
#define GSW_VTC     		(GSW_ARL_BASE + 0x14)

#define GSW_PMCR(n)     	(GSW_MAC_BASE + (n)*0x100)
#define GSW_PMSR(n)     	(GSW_MAC_BASE + (n)*0x100 + 0x08)
#define GSW_PINT_EN(n)     	(GSW_MAC_BASE + (n)*0x100 + 0x10)
#define GSW_GMACCR          (GSW_MAC_BASE + 0xE0)
#define GSW_SMACCR0     	(GSW_MAC_BASE + 0xE4)
#define GSW_SMACCR1     	(GSW_MAC_BASE + 0xE8)
#define GSW_CKGCR		    (GSW_MAC_BASE + 0xF0)
#define GSW_GPINT_EN	    (GSW_MAC_BASE + 0xF4)
#define GSW_GPINT_STS	    (GSW_MAC_BASE + 0xF8)

#define GSW_ESR(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x00)
#define GSW_INTS(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x04)
#define GSW_TGPC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x10)
#define GSW_TBOC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x14)
#define GSW_TGOC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x18)
#define GSW_TEPC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x1C)
#define GSW_RGPC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x20)
#define GSW_RBOC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x24)
#define GSW_RGOC(n)    		(GSW_MIB_BASE + (n)*0x100 + 0x28)
#define GSW_REPC1(n)   		(GSW_MIB_BASE + (n)*0x100 + 0x2C)
#define GSW_REPC2(n)   		(GSW_MIB_BASE + (n)*0x100 + 0x30)
#define GSW_MIBCNTEN  		(GSW_MIB_BASE + 0x800)
#define GSW_AECNT1  		(GSW_MIB_BASE + 0x804)
#define GSW_AECNT2  		(GSW_MIB_BASE + 0x808)

#define GSW_CFG_PPSC     	(GSW_CFG_BASE + 0x0)
#define GSW_CFG_PIAC     	(GSW_CFG_BASE + 0x4)
#define GSW_CFG_IMR     	(GSW_CFG_BASE + 0x8)
#define GSW_CFG_ISR     	(GSW_CFG_BASE + 0xC)
#define GSW_CFG_CPC     	(GSW_CFG_BASE + 0x10)
#define GSW_CFG_GPC     	(GSW_CFG_BASE + 0x14)

/***************************************
 * Ethernet Module Register Definition *
 ***************************************/

/* RSTCTRL2 */
#define EPHY_RST    					(1<<24)
#define ESW_RST    						(1<<23)
#define FE_RST    						(1<<21)

/* FE_VLAN_ID */
#define VLAN_ID1_SHIFT   				(16)
#define VLAN_IDl       					(0xfff<<VLAN_ID1_SHIFT)
#define VLAN_ID0_SHIFT   				(0)
#define VLAN_ID0       					(0xfff<<VLAN_ID0_SHIFT)

/* PDMA_GLO_CFG */
#define PDMA_BYTE_SWAP    				(1<<31)
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
#define GDM_TCI_81XX					(1<<24)
#define GDM_DROP_256B					(1<<23)
#define GDM_ICS_EN						(1<<22)
#define GDM_TCS_EN						(1<<21)
#define GDM_UCS_EN						(1<<20)
#define GDM_JMB_EN						(1<<19)
#define GDM_DISPAD						(1<<18)
#define GDM_DISCRC						(1<<17)
#define GDM_STRPCRC						(1<<16)
#define GDM_UFRC_P_SHIFT				(12)
#define GDM_UFRC_P						(0x7<<GDM_UFRC_P_SHIFT)
#define GDM_BFRC_P_SHIFT				(8)
#define GDM_BFRC_P						(0x7<<GDM_BFRC_P_SHIFT)
#define GDM_MFRC_P_SHIFT				(4)
#define GDM_MFRC_P						(0x7<<GDM_MFRC_P_SHIFT)
#define GDM_OFRC_P_SHIFT				(0)
#define GDM_OFRC_P						(0x7<<GDM_MFRC_P_SHIFT)

/* define GDMA port */
#define GDM_P_CPU						(0)
#define GDM_P_GDMA1						(1)
#define GDM_P_GDMA2						(2)
#define GDM_P_PPE						(6)
#define GDM_P_DISCARD					(7)

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

#define GDM_WT(n)						(((n) >= 8) ? 7 : (((n)-1) & 0x7))

/* CDMA_CSG_CFG */
#define INS_VLAN_SHIFT					(16)
#define INS_VLAN						(0xffff<<INS_VLAN_SHIFT)
#define ICS_GEN_EN						(1<<2)
#define UCS_GEN_EN						(1<<1)
#define TCS_GEN_EN						(1<<0)

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

/* GSW_TGPC */
#define TX_BAD_CNT_SHIFT				(16)
#define TX_BAD_CNT						(0xffff<<TX_BAD_CNT_SHIFT)
#define TX_GOOD_CNT_SHIFT				(0)
#define TX_GOOD_CNT						(0xffff<<TX_GOOD_CNT_SHIFT)

/* GSW_RGPC */
#define RX_BAD_CNT_SHIFT				(16)
#define RX_BAD_CNT						(0xffff<<RX_BAD_CNT_SHIFT)
#define RX_GOOD_CNT_SHIFT				(0)
#define RX_GOOD_CNT						(0xffff<<RX_GOOD_CNT_SHIFT)

/* GSW_REPC1 */
#define RX_CTRL_DROP_CNT_SHIFT			(16)
#define RX_CTRL_DROP_CNT				(0xffff<<RX_CTRL_DROP_CNT_SHIFT)
#define RX_ING_DROP_CNT_SHIFT			(0)
#define RX_ING_DROP_CNT					(0xffff<<RX_ING_DROP_CNT_SHIFT)

/* GSW_REPC2 */
#define RX_ARL_DROP_CNT_SHIFT			(16)
#define RX_ARL_DROP_CNT					(0xffff<<RX_ARL_DROP_CNT_SHIFT)
#define RX_FILTER_DROP_CNT_SHIFT		(0)
#define RX_FILTER_DROP_CNT				(0xffff<<RX_FILTER_DROP_CNT_SHIFT)

/* GSW_CFG_PPSC */
#define PHY_AP_EN						(1<<31)
#define PHY_PRE_EN						(1<<30)
#define PHY_MDC_CFG_SHIFT				(24)
#define PHY_MDC_CFG						(0x3f<<PHY_MDC_CFG_SHIFT)
#define EMB_AN_EN						(1<<23)
#define EEE_AN_EN_SHIFT					(16)
#define EEE_AN_EN						(16)
#define PHY_END_ADDR_SHIFT				(8)
#define PHY_END_ADDR					(0x1f<<PHY_END_ADDR_SHIFT)
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

/* GSW_CFG_GPC */
#define RX_CLK_MODE						(1<<2)


#define CHK_BUF() pos = begin + index; if (pos < off) { index = 0; begin = pos; }; if (pos > off + count) goto done;

typedef enum {
	SWITCH_PORT0 = 0,
	SWITCH_PORT1,
	SWITCH_PORT2,
	SWITCH_PORT3,
	#ifdef CONFIG_TCSUPPORT_ETH4_WAN_PORT
	SWITCH_PORT4,
	#endif
	SWITCH_CPU_PORT,
	SWITCH_PORT_MAX
}switch_port_t;


struct phy_link_state {
	unsigned int pLinkStatus;
	unsigned int pSpeed;
	unsigned int pDuplex;
};
/*=========================================
      PDMA RX Descriptor Format define
=========================================*/

//-------------------------------------------------

typedef union {
	struct _PDMA_RXD_INFO1_
	{
		unsigned int    PDP0;
	} bits;
	unsigned int word;
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
	unsigned int word;
} PDMA_RXD_INFO2_T;
//-------------------------------------------------

typedef union {
	struct _PDMA_RXD_INFO3_
	{
		unsigned int    UN_USE1;
	} bits;
	unsigned int word;
} PDMA_RXD_INFO3_T;
//-------------------------------------------------

typedef union {
	struct _PDMA_RXD_INFO4_
	{
#ifdef __BIG_ENDIAN
		unsigned int    IPFVLD_bit           : 1;
		unsigned int    L4FVLD_bit           : 1;
		unsigned int    IPF                  : 1;
		unsigned int    L4F                 : 1;
		unsigned int    AIS                 : 1;
		unsigned int    SP                  : 3;
		unsigned int    AI                  : 8;
		unsigned int    UN_USE1             : 1;
		unsigned int    FVLD                : 1;
		unsigned int    FOE_Entry           : 14;
#else
		unsigned int    FOE_Entry           : 14;
		unsigned int    FVLD                : 1;
		unsigned int    UN_USE1             : 1;
		unsigned int    AI                  : 8;
		unsigned int    SP                  : 3;
		unsigned int    AIS                 : 1;
		unsigned int    L4F                 : 1;
		unsigned int    IPF                  : 1;
		unsigned int    L4FVLD_bit           : 1;
		unsigned int    IPFVLD_bit           : 1;
#endif
	} bits;
	unsigned int word;
} PDMA_RXD_INFO4_T;

struct PDMA_rxdesc {
	PDMA_RXD_INFO1_T rxd_info1;
	PDMA_RXD_INFO2_T rxd_info2;
	PDMA_RXD_INFO3_T rxd_info3;
	PDMA_RXD_INFO4_T rxd_info4;
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
	unsigned int word;
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
	unsigned int word;
} PDMA_TXD_INFO2_T;
//-------------------------------------------------

typedef union {
	struct _PDMA_TXD_INFO3_
	{
		unsigned int    SDP1;
	} bits;
	unsigned int word;
} PDMA_TXD_INFO3_T;
//-------------------------------------------------

typedef union {
	struct _PDMA_TXD_INFO4_
	{
#ifdef __BIG_ENDIAN
		unsigned int    ICO		        : 1;
		unsigned int    UCO			: 1;
		unsigned int    TCO                 : 1;
		unsigned int    UN_USE1             : 2;
		unsigned int    PN                  : 3;
		unsigned int    UDF			: 4;
		unsigned int    UN_USE2             : 1;
		unsigned int    QN                  : 3;
		unsigned int    UN_USE3             : 2;
		unsigned int    RESV            	: 1;
		unsigned int    INSP                : 1;
		unsigned int    SIDX                : 4;
		unsigned int    INSV                : 1;
		unsigned int    VPRI                : 3;
		unsigned int    VIDX                : 4;
#else
		unsigned int    VIDX                : 4;
		unsigned int    VPRI                : 3;
		unsigned int    INSV                : 1;
		unsigned int    SIDX                : 4;
		unsigned int    INSP                : 1;
		unsigned int    RESV            	: 1;
		unsigned int    UN_USE3             : 2;
		unsigned int    QN                  : 3;
		unsigned int    UN_USE2             : 1;
		unsigned int    UDF			: 4;
		unsigned int    PN                  : 3;
		unsigned int    UN_USE1             : 2;
		unsigned int    TCO                 : 1;
		unsigned int    UCO			: 1;
		unsigned int    ICO		        : 1;
#endif
	} bits;
	unsigned int word;
} PDMA_TXD_INFO4_T;

struct PDMA_txdesc {
	PDMA_TXD_INFO1_T txd_info1;
	PDMA_TXD_INFO2_T txd_info2;
	PDMA_TXD_INFO3_T txd_info3;
	PDMA_TXD_INFO4_T txd_info4;
};

typedef struct PDMA_txdesc macTxDescr_t;

/******************************
 * Giga Switch IOCTL Commands *
 ******************************/

/* ioctl commands */
#define RAETH_GSW_REG_READ		0x89F1
#define RAETH_GSW_REG_WRITE		0x89F2

#define RAETH_REG_READ			0x89F3
#define RAETH_REG_WRITE			0x89F4

typedef struct _gsw_reg {
	__u32 off;
	__u32 val;
} gsw_reg;

/*********************
 * Mac Control Block *
 *********************/

/* ----- MIB-II ----- */
typedef struct macMIB_II_s {
	unsigned int inOctets;			/* Receive Octets */
	unsigned int inUnicastPkts;		/* Receive Unicast Packets */
	unsigned int inMulticastPkts;		/* Receive Multicast Packets */
	unsigned int inDiscards;			/* Receive Discard Packets */
	unsigned int inErrors;			/* Receive Error Packets */
	unsigned int inUnknownProtocols;	/* Receive Unknown Prototol Packets */
	unsigned int outOctets;			/* Transmit Octets */
	unsigned int outUnicastPkts;		/* Transmit Unicast Packets */
	unsigned int outMulticastPkts;	/* Transmit Multicast Packets */
	unsigned int outDiscards;			/* Transmit Discard Packets */
	unsigned int outErrors;			/* Transmit Error Packets */
} macMIB_II_t;

/* ----- Adapter Statistics ----- */
typedef struct inSiliconStat_s {
	unsigned int txJabberTimeCnt;
	unsigned int txLossOfCarrierCnt;
	unsigned int txNoCarrierCnt;
	unsigned int txLateCollisionCnt;
	unsigned int txExCollisionCnt;
	unsigned int txHeartbeatFailCnt;
	unsigned int txCollisionCnt;
	unsigned int txExDeferralCnt;
	unsigned int txUnderRunCnt;

	unsigned int rxAlignErr;
	unsigned int rxSymbolErr;
	unsigned int rxMiiErr;
	unsigned int rxCrcErr;
	unsigned int rxEtherFrameLengthErr; /* 60 > size(Packet) or 1518 < size(Packet) */
	unsigned int rx802p3FrameLengthErr; /* value of length field of 802.3 packet is
	                                 larger than real packet payload */
	unsigned int rxDribblingErr;
	unsigned int rxRuntErr;
	unsigned int rxLongErr;
	unsigned int rxCollisionErr;

	unsigned int rxPktIPChkSumErr;

	unsigned int rxEnQueueNum;          /* Number of packets enqueued in macRxRingproc() */
	unsigned int rxDeQueueNum;          /* Number of packets dequeued in macRxToUpperTask() */
	unsigned int txEnQueueNum;          /* Number of packets enqueued in macSend() */
  	unsigned int txDeQueueNum;          /* Number of packets dequeued in macTxRingproc() */
} inSiliconStat_t;


/* ----- Ethernet Link Profile ----- */
typedef struct macPhyLinkProfile_s {
	unsigned int linkSpeed;							/* 10Mbps or 100Mbps */
	unsigned int duplexMode;							/* Half/Full Duplex Mode */
	unsigned int enetMode;
	unsigned int ANCompFlag;							/* auto_negotiation complete Flag */
	unsigned int PollCount;							/* auto_negotiation polling check count */
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
struct macAdapter {
  	macRxMemPool_t      *macRxMemPool_p;
  	dma_addr_t        	macRxMemPool_phys_p;
  	macTxMemPool_t      *macTxMemPool_p;
  	dma_addr_t        	macTxMemPool_phys_p;
  	macStat_t           macStat;
  	unsigned char statisticOn;  /* Flag to record statistics or not */
  	unsigned int enetPhyAddr;
  	unsigned int txDescrRingBaseVAddr[TX_QUEUE_NUM];  /* Transmit Descr Ring Virtual Address */
  	unsigned int rxDescrRingBaseVAddr;  			  /* Receive Descr Ring Virtual Address */
  	unsigned int txCurrentDescp[TX_QUEUE_NUM];        /* index to current tx descriptor */
  	unsigned int txUnReleasedDescp[TX_QUEUE_NUM];     /* index to the unreleased descp of Tx */
  	unsigned int txUnReleasedBufCnt[TX_QUEUE_NUM];    /* Unreleased buffer cnt hold by Tx	*/
  	unsigned int rxCurrentDescp;        			  /* index to current rx descriptor */
  	unsigned int rxRingSize;  /* Receive Descr Ring Size */
  	unsigned int txRingSize;  /* Receive Descr Ring Size */
  	unsigned int rxDescrSize; /* The size of a Rx descr entry in the descr ring */
  	unsigned int txDescrSize; /* The size of a Tx descr entry in the descr ring */
	struct sk_buff *txskbs[TX_QUEUE_NUM][MAC_TXDESCP_NO];
	struct sk_buff *rxskbs[MAC_RXDESCP_NO];
	#ifdef TC3262_GMAC_SG_MODE
	struct sk_buff *sg_rxskbs[MAC_RXDESCP_NO];
	#endif
	struct mii_if_info mii_if;
  	spinlock_t lock;
  	struct net_device_stats stats;
	#if KERNEL_2_6_36
	struct napi_struct	napi;
	#endif
};

typedef struct phyDeviceList_s {
	unsigned short companyId;
	char vendorName[30];
} phyDeviceList_t;

typedef struct dmtConsoleData_s {
        unsigned char *data;
        unsigned int len;
} dmtConsoleData_t;

/************************************************************************
*        F U N C T I O N   P R O T O T Y P E S
*************************************************************************/
#ifdef TC_CONSOLE_ENABLE
extern int tcconsole_proc(struct sk_buff * skb);
extern void uart_msg_to_tcconsole(char *msg, int len);
extern void (*send_uart_msg)(char* msg, int len);
extern void create_tcconsole_proc(void);
extern void delete_tcconsole_proc(void);
#endif
extern unsigned short miiStationRead(unsigned int PhyAddr, unsigned int PhyReg);
extern void miiStationWrite(unsigned int PhyAddr,unsigned int PhyReg,unsigned int MiiData);
extern int tc3262_gmac_tx(struct sk_buff *skb, struct net_device *dev);
extern int tc3262_gmac_set_macaddr(struct net_device * dev, void * p);

#endif /* _FEMAC_H */

