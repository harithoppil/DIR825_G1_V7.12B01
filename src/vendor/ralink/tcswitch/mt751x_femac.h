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

#define CONFIG_ETHERNET_DEBUG             1
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

#define RX_BUF_LEN 			(2048 - NET_SKB_PAD - 64 - (sizeof(struct skb_shared_info)))		
#define RX_MAX_PKT_LEN 		1536

#define TC3262_GMAC_SKB_RECYCLE   1
//#define TCSUPPORT_WAN_ETHER     1
#define TC_CONSOLE_ENABLE       1

#ifdef TC_CONSOLE_ENABLE
#define SKBUF_TCCONSOLE         (1 << 27)
#endif

#define SG_MAX_PKT_LEN	   		(128)

#define MAC_TXDESCP_NO			128		/* Max tx buffer Cnts , default=128 */

//#define MAC_RXDESCP_NO			128 	/* Max rx buffer Cnts , default=128 */
#define MAC_RXDESCP_NO_RES			128
#define	MAC_RXDESCP_NO_DEFAULT	128

#define MAC_RXDESCP_NO					(MAC_RXDESCP_NO_DEFAULT + MAC_RXDESCP_NO_RES)

#define MAC_NAPI_WEIGHT			128//64
#define MAC_RECV_THLD       	15

#define MAC_RXDESCP_SIZE		16		/* 4 DWords */
#define MAC_TXDESCP_SIZE		16		/* 4 DWords */

#define TX_BUF_RELEASE_THRESHOLD 4  	/* default:4 */

#define TX_QUEUE_NUM 			4 
#define RX_QUEUE_NUM 			2 

#define K1_TO_PHY(x)			(((unsigned int)x) & 0x1fffffff)

#define MAC_STATISTIC_ON		1
#define MAC_STATISTIC_OFF		0

#define RAETH_CHECKSUM_OFFLOAD
#define VLAN_TAG_USED           0


#define GMAC_PRIORITY_MASK  	(TX_QUEUE_NUM-1)

#define SPEC_TPID               0x00 /* Special TAG, not VLAN tag ETH_P_8021Q */
#define START_VLAN_VID	        0x001
/*****************************
 * Ethernet Module Registers *
 *****************************/
#define GSW_BASE     		0xBFB58000
 
#define FE_GLO_CFG              (CR_MAC_BASE + 0x00)
#define FE_RST_GLO              (CR_MAC_BASE + 0x04)
#define FE_INT_STATUS           (CR_MAC_BASE + 0x08)
#define FE_INT_ENABLE           (CR_MAC_BASE + 0x0C)
#define FE_FOE_TS_T             (CR_MAC_BASE + 0x10)
#define FE_IPV6_EXT             (CR_MAC_BASE + 0x14)

#define PSE_FQFC_CFG            (CR_MAC_BASE + 0x100)
#define PSE_IQ_REV1             (CR_MAC_BASE + 0x108)
#define PSE_IQ_REV2             (CR_MAC_BASE + 0x10C)
#define PSE_IQ_STA1             (CR_MAC_BASE + 0x110)
#define PSE_IQ_STA2             (CR_MAC_BASE + 0x114)
#define PSE_OQ_STA1             (CR_MAC_BASE + 0x118)
#define PSE_OQ_STA2             (CR_MAC_BASE + 0x11C)
#define PSE_MIR_PORT            (CR_MAC_BASE + 0x120)

#define CDMP_VLAN_CTRL          (CR_MAC_BASE + 0x400)
#define CDMP_PPP_GEN            (CR_MAC_BASE + 0x404)

#define GDM1_FWD_CFG            (CR_MAC_BASE + 0x500)
#define GDM1_SHRP_CFG           (CR_MAC_BASE + 0x504)
#define GDM1_MAC_ADRL           (CR_MAC_BASE + 0x508)
#define GDM1_MAC_ADRH           (CR_MAC_BASE + 0x50c)
#define GDM1_VLAN_GEN           (CR_MAC_BASE + 0x510)
#define GDM1_LEN_CFG            (CR_MAC_BASE + 0x514)

#define PDMA_BASE               (CR_MAC_BASE + 0x800)
#define TX_BASE_PTR(n)          (PDMA_BASE + (n)*0x10 + 0x000)
#define TX_MAX_CNT(n)           (PDMA_BASE + (n)*0x10 + 0x004)
#define TX_CTX_IDX(n)           (PDMA_BASE + (n)*0x10 + 0x008)
#define TX_DTX_IDX(n)           (PDMA_BASE + (n)*0x10 + 0x00C)

#define RX_BASE_PTR(n)          (PDMA_BASE + (n)*0x10 + 0x100)
#define RX_MAX_CNT(n)           (PDMA_BASE + (n)*0x10 + 0x104)
#define RX_CALC_IDX(n)          (PDMA_BASE + (n)*0x10 + 0x108)
#define RX_DRX_IDX(n)           (PDMA_BASE + (n)*0x10 + 0x10C)
#define PDMA_INFO               (PDMA_BASE + 0x200)
#define PDMA_GLO_CFG            (PDMA_BASE + 0x204)
#define PDMA_RST_IDX            (PDMA_BASE + 0x208)
#define DLY_INT_CFG             (PDMA_BASE + 0x20C)
#define FREEQ_THRES             (PDMA_BASE + 0x210)
#define INT_STATUS              (PDMA_BASE + 0x220) 
#define INT_MASK                (PDMA_BASE + 0x228)
#define SCH_Q01_CFG             (PDMA_BASE + 0x280)
#define SCH_Q23_CFG             (PDMA_BASE + 0x284)

#define GDMA2_BASE              (CR_MAC_BASE + 0x1500)
#define GDMA2_FWD_CFG           (GDMA2_BASE + 0x00)
#define GDMA2_SHRP_CFG          (GDMA2_BASE + 0x04)
#define GDMA2_MAC_ADRL          (GDMA2_BASE + 0x08)
#define GDMA2_MAC_ADRH          (GDMA2_BASE + 0x0c)

//count define
#define GDMA_COUNT_BASE         0xBFB52400
#define GDM1_RX_GBCNT_L         (GDMA_COUNT_BASE + 0x00)
#define GDM1_RX_GBCNT_H         (GDMA_COUNT_BASE + 0x04)
#define GDM1_RX_GPCNT           (GDMA_COUNT_BASE + 0x08)
#define GDM1_RX_OERCNT          (GDMA_COUNT_BASE + 0x10)
#define GDM1_RX_FERCNT          (GDMA_COUNT_BASE + 0x14)
#define GDM1_RX_SERCNT          (GDMA_COUNT_BASE + 0x18)
#define GDM1_RX_LERCNT          (GDMA_COUNT_BASE + 0x1C)
#define GDM1_RX_CERCNT          (GDMA_COUNT_BASE + 0x20)
#define GDM1_RX_FCCNT           (GDMA_COUNT_BASE + 0x24)
#define GDM1_TX_SKIPCNT         (GDMA_COUNT_BASE + 0x28)
#define GDM1_TX_COLCNT          (GDMA_COUNT_BASE + 0x2C)
#define GDM1_TX_GBCNT_L         (GDMA_COUNT_BASE + 0x30)
#define GDM1_TX_GBCNT_H         (GDMA_COUNT_BASE + 0x34)
#define GDM1_TX_GPCNT           (GDMA_COUNT_BASE + 0x38)

#define GSW_ARL_BASE            (CR_GSW_BASE + 0x00)
#define GSW_MFC                 (CR_GSW_BASE + 0x10)
#define GSW_VTC     		    (CR_GSW_BASE + 0x14)
#define GSW_ATAI                (CR_GSW_BASE + 0x74)/* Address Table Access I Register */
#define GSW_ATAII               (CR_GSW_BASE + 0x78)/* Address Table Access II Register */
#define GSW_ATWD                (CR_GSW_BASE + 0x7C)/* Address Table Write Data Register */
#define GSW_ATC                 (CR_GSW_BASE + 0x80)/* Address Table Control Register */
#define GSW_TSRAI               (CR_GSW_BASE + 0x84)/* Table Search Read Address-I Register */
#define GSW_TSRAII              (CR_GSW_BASE + 0x88)/* Table Search Read Address-II Register */
#define GSW_ATRD                (CR_GSW_BASE + 0x8C)/* Address Table Read Data Register */
#define GSW_VTCR                (CR_GSW_BASE + 0x90)/* VLAN Table Control Register */
#define GSW_VAWDI               (CR_GSW_BASE + 0x94)/* VLAN and ACL Write Data I Register */
#define GSW_VAWDII              (CR_GSW_BASE + 0x98)/* VLAN and ACL Write Data II Register */

#define GSW_BMU_BASE            (CR_GSW_BASE + 0x1000)


#define GSW_PORT_BASE           (CR_GSW_BASE + 0x2000)
#define GSW_SSC(n)              (GSW_PORT_BASE + (n) * 0x100)/* STP State Control Register */
#define GSW_PCR(n)              (GSW_PORT_BASE + (n) * 0x100 + 0x04)/* Port Control Register */
#define GSW_PIC(n)              (GSW_PORT_BASE + (n) * 0x100 + 0x08)/* Port IGMP Control Register */
#define GSW_PSC(n)              (GSW_PORT_BASE + (n) * 0x100 + 0x0C)/* Port Security Control Register */
#define GSW_PVC(n)              (GSW_PORT_BASE + (n) * 0x100 + 0x10)/* Port VLAN Control Register */
#define GSW_PPBV1(n)            (GSW_PORT_BASE + (n) * 0x100 + 0x14)/* Port-and-Protocol Based VLAN-I Register */

#define GSW_MAC_BASE            (CR_GSW_BASE + 0x3000)
#define GSW_GMACCR              (GSW_MAC_BASE + 0xE0)
#define GSW_CKGCR		        (GSW_MAC_BASE + 0xF0)
#define GSW_PMCR(n)         	(GSW_MAC_BASE + (n)*0x100)
#define GSW_PMSR(n)         	(GSW_MAC_BASE + (n)*0x100 + 0x08)
#define GSW_PINT_EN(n)        	(GSW_MAC_BASE + (n)*0x100 + 0x10)


#define GSW_MIB_BASE            0xBFB5C000
#define GSW_TDPC(n)    		    (GSW_MIB_BASE + (n)*0x100 + 0x00)/* Tx Drop Packet Counter */
#define GSW_TCRC(n)    	    	(GSW_MIB_BASE + (n)*0x100 + 0x04)/* Tx CRC Packet Counter */
#define GSW_TUPC(n)        		(GSW_MIB_BASE + (n)*0x100 + 0x08)/* Tx Unicast Packet Counter */
#define GSW_TMPC(n)        		(GSW_MIB_BASE + (n)*0x100 + 0x0C)/* Tx Multicast Packet Counter */
#define GSW_TBPC(n)        		(GSW_MIB_BASE + (n)*0x100 + 0x08)/* Tx Broadcast Packet Counter */
#define GSW_TCEC(n)        		(GSW_MIB_BASE + (n)*0x100 + 0x14)/* Tx Collision Event Counter */
#define GSW_TSCEC(n)       		(GSW_MIB_BASE + (n)*0x100 + 0x18)/* Tx Single Collision Event Counter */
#define GSW_TMCEC(n)       		(GSW_MIB_BASE + (n)*0x100 + 0x1C)/* Tx Multiple Collision Event Counter */
#define GSW_TDEC(n)        		(GSW_MIB_BASE + (n)*0x100 + 0x20)/* Tx Deferred Event Counter */
#define GSW_TLCEC(n)       		(GSW_MIB_BASE + (n)*0x100 + 0x24)/* Tx Late Collision Event Counter */
#define GSW_TXCEC(n)            (GSW_MIB_BASE + (n)*0x100 + 0x28)/* Tx Excessive Collision Event Counter */
#define GSW_TPPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x2C)/* Tx Pause Packet Counter */
#define GSW_RDPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x60)/* Rx Drop Packet Counter */
#define GSW_RFPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x64)/* Rx Filtering Packet Counter */
#define GSW_RUPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x68)/* Rx Unicast Packet Counter */
#define GSW_RMPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x6C)/* Rx Multicast Packet Counter */
#define GSW_RBPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x70)/* Rx Broadcast Packet Counter */
#define GSW_RAEPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x74)/* Rx Alignment Error Packet Counter */
#define GSW_RCEPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x78)/* Rx CRC(FCS) Error Packet Counter */
#define GSW_RUSPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x7C)/* Rx Undersize Packet Counter */
#define GSW_RFEPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x80)/* Rx Fragment Error Packet Counter */
#define GSW_ROSPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x84)/* Rx Oversize Packet Counter */
#define GSW_RJEPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x88)/* Rx Jabber Error Packet Counter */
#define GSW_RPPC(n)   	    	(GSW_MIB_BASE + (n)*0x100 + 0x8C)/* Rx Pause Packet Counter */
#define GSW_RDPC_CTRL(n)   	    (GSW_MIB_BASE + (n)*0x100 + 0xB0)/* Rx CTRL Drop Packet Counter */
#define GSW_RDPC_ING(n)   	    (GSW_MIB_BASE + (n)*0x100 + 0xB4)/* Rx Ingress Drop Packet Counter */
#define GSW_RDPC_ARL(n)   	    (GSW_MIB_BASE + (n)*0x100 + 0xB8)/* Rx ARL Drop Packet Counter */

#define GSW_CFG_BASE            (CR_GSW_BASE + 0x7000)
#define GSW_CFG_IMR         	(GSW_CFG_BASE + 0x08)
#define GSW_CFG_ISR         	(GSW_CFG_BASE + 0x0C)
#define GSW_CFG_CPC         	(GSW_CFG_BASE + 0x10)
#define GSW_CFG_GPC         	(GSW_CFG_BASE + 0x14)
#define GSW_CFG_PIAC            (GSW_CFG_BASE + 0x1c)

#define GSW_SMACCR0     	(GSW_MAC_BASE + 0xe4)
#define GSW_SMACCR1     	(GSW_MAC_BASE + 0xe8)

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
// TODO: Check!!!
// #define GSW_PVC(n)     	(0x2010 + (n)*0x100)
#define DEFAULT_TPID  	(0x8100)

#define GSW_CFG_PPSC     	(GSW_CFG_BASE + 0x18)

#define GSW_VLAN_REG		(GSW_BASE+0x94)
#define GSW_ATA1_REG		(GSW_BASE+0x74)
#define GSW_ATC_REG			(GSW_BASE+0x80)

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
#define RX_2BYTE_OFFSET                 (1<<31)
#define CSR_CLKGATE                     (1<<30)
#define PDMA_BYTE_SWAP                  (1<<29)
#define PDMA_BIG_ENDIAN                 (1<<7)
#define TX_WB_DDONE                     (1<<6)
#define PDMA_BT_SIZE_SHIFT              (4)
#define PDMA_BT_SIZE                    (0x3<<PDMA_BT_SIZE_SHIFT)
#define RX_DMA_BUSY                     (1<<3)
#define RX_DMA_EN                       (1<<2)
#define TX_DMA_BUSY                     (1<<1)
#define TX_DMA_EN                       (1<<0)

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
#define GDM_INSV_EN                     (1<<26)
#define GDM_UNTAG_EN                    (1<<25)
#define GDM_STAG_EN                     (1<<24)
#define GDM_ICS_EN                      (1<<22)
#define GDM_TCS_EN                      (1<<21)
#define GDM_UCS_EN                      (1<<20)
#define GDM_DISPAD                                      (1<<18)
#define GDM_DISCRC                                      (1<<17)
#define GDM_STRPCRC                                     (1<<16)
#define GDM_UFRC_P_SHIFT                                (12)
#define GDM_UFRC_P                                      (0xf<<GDM_UFRC_P_SHIFT)
#define GDM_BFRC_P_SHIFT                                (8)
#define GDM_BFRC_P                                      (0xf<<GDM_BFRC_P_SHIFT)
#define GDM_MFRC_P_SHIFT                                (4)
#define GDM_MFRC_P                                      (0xf<<GDM_MFRC_P_SHIFT)
#define GDM_OFRC_P_SHIFT                                (0)
#define GDM_OFRC_P                                      (0xf<<GDM_MFRC_P_SHIFT)

/* define GDMA port */
#define GDM_P_CPU                       (0x0)
#define GDM_P_GDMA1                     (0x1)
#define GDM_P_GDMA2                     (0x2)
#define GDM_P_PPE                       (0x4)
#define GDM_P_QDMA                      (0x5)
#define GDM_P_DISCARD                   (0x7)



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
#define CDM_STAG_EN                     (1<<0)


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

#define IPV6_H                                          (1<<5)
#define IPV4_H                                          (1<<4)
#define IPV4_H_INV                                      (1<<3)
#define TU_H                                            (1<<1)
#define TU_H_C_INV                                      (1<<0)

/************************************************************************
*                          C O N S T A N T S
*************************************************************************/
/* define loopback mode test */
#define LOOPBACK_TX			0x01
#define LOOPBACK_RX_DROP	0x02
#define LOOPBACK_RX_CHK		0x03
#define LOOPBACK_TX_QOS		0x04
#define LOOPBACK_MODE_MASK	0x0f
#define LOOPBACK_MODE(x)	((x) & 0x0f)
#define LOOPBACK_TX_IPCS	(1<<4)
#define LOOPBACK_TX_VLAN	(1<<5)
#define LOOPBACK_TX_RANDOM	(1<<6)
#define LOOPBACK_MSG		(1<<7)
#define LOOPBACK_PKT		(1<<11)
#define LOOPBACK_EXT		(1<<12)

/* ADMTEK6996M register */
#define ADM_PORT0_BASIC		0x01
#define ADM_PORT1_BASIC		0x03
#define ADM_PORT2_BASIC		0x05
#define ADM_PORT3_BASIC		0x07
#define ADM_PORT4_BASIC		0x08
#define ADM_PORT5_BASIC		0x09
#define ADM_CHIP_ID0		0xa0
#define ADM_CHIP_ID1		0xa1

#ifndef TCSUPPORT_QOS
#define QOS_REMARKING  1  
#endif
#define TCSUPPORT_HW_QOS
#ifdef QOS_REMARKING  
#define QOS_REMARKING_MASK    0x00000007
#define QOS_REMARKING_FLAG    0x00000001
#endif
#if defined (QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
#define QOS_DMAWRR_USERDEFINE 0x1
#define PTQCR_WRR_EN			(1<<4)
#define PTQCR_WRR_SELECT 	 	(1<<6)
#endif

#ifdef TCSUPPORT_QOS
#define		QOS_FILTER_MARK		0xf0
#define 	QOS_HH_PRIORITY		0x10
#define 	QOS_H_PRIORITY		0x20
#define 	QOS_M_PRIORITY		0x30
#define		QOS_L_PRIORITY		0x40
#define		NULLQOS				-1
#define 	QOS_SW_PQ			0	//will use hw at the same time
#define		QOS_SW_WRR			1
#define		QOS_SW_CAR			2
#define 	QOS_HW_WRR			3
#define		QOS_HW_PQ			4
#endif

/************************************************************************
*                            M A C R O S
*************************************************************************/
#define CHK_BUF() pos = begin + index; if (pos < off) { index = 0; begin = pos; }; if (pos > off + count) goto done;

typedef enum {
	SWITCH_PORT0 = 0,
	SWITCH_PORT1,
	SWITCH_PORT2,
	SWITCH_PORT3,
	SWITCH_PORT4,
	SWITCH_PORT_MAX
}switch_port_t;

/*=========================================
      PDMA RX Descriptor Format define
=========================================*/
typedef union {
	struct _PDMA_RXD_INFO1_	{
		unsigned int    PDP0;
	} bits;
	unsigned int word;
} PDMA_RXD_INFO1_T;

typedef union {
	struct _PDMA_RXD_INFO2_ {
		unsigned int    DDONE_bit             : 1;
		unsigned int    LS0                   : 1;
		unsigned int    PLEN0                 : 14;
		unsigned int    UN_USED               : 1;
		unsigned int    LS1                   : 1;
		unsigned int    PLEN1                 : 14;
	} bits;
	unsigned int word;
} PDMA_RXD_INFO2_T;

typedef union {
	struct _PDMA_RXD_INFO3_ {
		unsigned int    UN_USE1;
	} bits;
	unsigned int word;
} PDMA_RXD_INFO3_T;

typedef union {
	struct _PDMA_RXD_INFO4_ {
		unsigned int    RSV                   : 3;
		unsigned int    PKT_INFO              : 6;
		unsigned int    SPORT                 : 4;
		unsigned int    CRSN                  : 5;
		unsigned int    FOE_Entry             : 14;
	} bits;
	unsigned int word;
} PDMA_RXD_INFO4_T;

struct PDMA_rxdesc {
	PDMA_RXD_INFO1_T rxd_info1;
	PDMA_RXD_INFO2_T rxd_info2;
	PDMA_RXD_INFO3_T rxd_info3;
	PDMA_RXD_INFO4_T rxd_info4;
};

/*=========================================
      PDMA TX Descriptor Format define
=========================================*/
typedef union {
	struct _PDMA_TXD_INFO1_ {
		unsigned int    SDP0;
	} bits;
	unsigned int word;
} PDMA_TXD_INFO1_T;

typedef union {
	struct _PDMA_TXD_INFO2_ {
		unsigned int    DDONE_bit             : 1;
		unsigned int    LS0_bit               : 1;
		unsigned int    SDL0                  : 14;
		unsigned int    BURST_bit             : 1;
		unsigned int    LS1_bit               : 1;
		unsigned int    SDL1                  : 14;
	} bits;
	unsigned int word;
} PDMA_TXD_INFO2_T;

typedef union {
	struct _PDMA_TXD_INFO3_	{
		unsigned int    SDP1;
	} bits;
	unsigned int word;
} PDMA_TXD_INFO3_T;

typedef union {
	struct _PDMA_TXD_INFO4_ {
		unsigned int    ICO                   : 1;
		unsigned int    UCO                   : 1;
		unsigned int    TCO                   : 1;
		unsigned int    TSO                   : 1;
		unsigned int    UDF                   : 6;
		unsigned int    PN                    : 3;//the same with FPORT
		unsigned int    INSV                  : 1;
		unsigned int    TPID                  : 2;
		unsigned int    VPRI                  : 3;
		unsigned int    CFI                   : 1;
		unsigned int    VIDX                  : 12;
	} bits;
	unsigned int word;
} PDMA_TXD_INFO4_T;

struct PDMA_txdesc {
	PDMA_TXD_INFO1_T txd_info1;
	PDMA_TXD_INFO2_T txd_info2;
	PDMA_TXD_INFO3_T txd_info3;
	PDMA_TXD_INFO4_T txd_info4;
};

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

/* ----- Ethernet private memory pool ----- */
typedef struct macRxMemPool_s {
	struct PDMA_rxdesc rxDescpBuf[MAC_RXDESCP_NO];
} macRxMemPool_t;

typedef struct macTxMemPool_s {
	struct PDMA_txdesc txDescpBuf[TX_QUEUE_NUM][MAC_TXDESCP_NO];
} macTxMemPool_t;

/* ----- Statistics for GMAC ----- */
typedef struct macStat_s {
	macMIB_II_t MIB_II;	/* MIB-II */
	inSiliconStat_t inSilicon;	
} macStat_t;

/* ----- Adapter Card Table ------ */
typedef struct macAdapter {
  	macRxMemPool_t *macRxMemPool_p;
  	dma_addr_t     macRxMemPool_phys_p;
  	macTxMemPool_t *macTxMemPool_p;
  	dma_addr_t    macTxMemPool_phys_p;
  	macStat_t     macStat;
  	unsigned char statisticOn;  /* Flag to record statistics or not */
  	unsigned int enetPhyAddr;
  	// unsigned char enetPhyId;
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
  	struct tasklet_struct lk_task;
	struct napi_struct	napi;
}macAdapter_t;

struct tcswitch {
	unsigned short id;
	char phy_ver;
	char nr_ports;
	char *name;
};

struct phy_link_state {
	unsigned int LinkStatus;
	unsigned int Speed;
	unsigned int Duplex;
};

struct device_priv {
	struct net_device_stats stat;/* The new statistics table. */
	struct phy_link_state link;  /* Link state */
	unsigned char phy_id;
};

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
// extern unsigned short miiStationRead(unsigned int PhyAddr, unsigned int PhyReg);
// extern void miiStationWrite(unsigned int PhyAddr,unsigned int PhyReg,unsigned int MiiData);
// extern int tc3262_gmac_tx(struct sk_buff *skb, struct net_device *dev);
// extern int tc3262_gmac_set_macaddr(struct net_device * dev, void * p);

#define read_reg_word(reg) 			regRead32(reg)
#define write_reg_word(reg, wdata) 	regWrite32(reg, wdata)

#define MT751X_MAC_MOD_NAME    "mt751x_mac"
#define MT751X_MAC_DEBUG_VER   "T&W mt751X debug MAC v0.1 (Camel)"

#endif /* _FEMAC_H */

