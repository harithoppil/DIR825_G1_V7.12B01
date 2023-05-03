/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/tc3262/tc3262ptm.h#2 $
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
** $Log: tc3262ptm.h,v $
** Revision 1.1.1.1  2010/09/30 21:14:53  josephxu
** modules/public, private
**
** Revision 1.1.1.1  2010/04/09 09:34:42  feiyan
** New TC Linux Make Flow Trunk
**
** Revision 1.1.1.1  2009/12/17 01:47:37  josephxu
** 20091217, from Hinchu ,with VoIP
**
 */
#ifndef _TC3262PTM_H
#define _TC3262PTM_H

#define KERNEL_2_6_36 		(LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31))

/* scatter & gather mode */
//#define TC3262_PTM_SG_MODE

#define SG_MAX_PKT_LEN	   		(128)

#define PTM_TXDESCP_NO			64		/* Max tx buffer Cnts , default=64 */
#define PTM_RXDESCP_NO			128  	/* Max rx buffer Cnts , default=256 */
#if KERNEL_2_6_36
#define PTM_NAPI_WEIGHT			32
#endif
#define PTM_RXDESCP_SIZE		32		/* 8 DWords */
#define PTM_TXDESCP_SIZE		16		/* 4 DWords */

#define TX_BUF_RELEASE_THRESHOLD 4  	/* default:4 */

#define TX_QUEUE_NUM 			8  
#define RX_QUEUE_NUM 			2  

#define K0_TO_K1(x)				(((uint32)x) | 0xa0000000)
#define K1_TO_PHY(x)			(((uint32)x) & 0x1fffffff)

#define PTM_STATISTIC_ON		1
#define PTM_STATISTIC_OFF		0

#define PTM_PRIORITY_MASK  		(0x7)

#define PTM_BEARER_MASK  		(0x1)
#define PTM_BEARER_SHIFT  		(3)

/*****************************
 * PTM Module Registers      *
 *****************************/
#define PTM_BEARER_NUM			2

#define PTM_B0	      			(0x0)
#define PTM_B1	      			(0x1)

#define PTM_B1_OFFSET  			(0x200)

#define PTM_CH0	      			(0x0)
#define PTM_CH1	      			(0x1)

#define CR_PTM_BASE   	  		0xBFB68000
#define CR_PTM_PRCR(b)      	(0x00 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PIMR(b)      	(0x04 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PISR(b)  	   	(0x08 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_TQER(b)     		(0x0c | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PFSR(b)     		(0x10 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PMAR(b)     		(0x14 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PMAR1(b)     	(0x18 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PDPSR(b)     	(0x1c | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PPCRR(b)     	(0x20 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PVPMR(b)     	(0x24 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTPR0(b)     	(0x28 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTPR1(b)     	(0x2c | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTPR2(b)     	(0x30 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTPR3(b)     	(0x34 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTCPR(b)     	(0x38 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRXLR(b)     	(0x3c | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PDER(b)     		(0x40 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRRBR(b)     	(0x44 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTRBR(b)     	(0x48 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRRSR(b)     	(0x4c | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTRS01R(b)    	(0x50 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTRS23R(b)    	(0x54 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTRS45R(b)    	(0x58 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTRS67R(b)    	(0x5c | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRHPR(b)    		(0x60 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PSGCR(b)    		(0x64 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTHPR(b, ring)  	((0x68 | CR_PTM_BASE | (b*PTM_B1_OFFSET)) + (ring) * 4)
#define CR_PTM_PTTPR(b, ring)  	((0x88 | CR_PTM_BASE | (b*PTM_B1_OFFSET)) + (ring) * 4)
#define CR_PTM_PTLWR(b)    		(0xa8 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTUSR(b)    		(0xac | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTIETR(b)    	(0xb0 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PPRCTR(b)    	(0xb4 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRDLR(b)    		(0xb8 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTDLR(b)    		(0xbc | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTQCR(b)    		(0xc0 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTSLR(b)    		(0xc4 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PSBCR0(b)    	(0xc8 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PSBCR1(b)    	(0xcc | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PSBCR2(b)    	(0xd0 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PSBCR3(b)    	(0xd4 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PWWR0(b)    		(0xd8 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PWWR1(b)    		(0xdc | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PWWR2(b)    		(0xe0 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PWWR3(b)    		(0xe4 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRTPR(b, ring)	((0xe8 | CR_PTM_BASE | (b*PTM_B1_OFFSET)) + (ring) * 4)
#define CR_PTM_PRXCR0(b)    	(0xf0 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRXCR1(b)    	(0xf4 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRIETR0(b)    	(0xf8 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRIETR1(b)    	(0xfc | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTPC(b)    		(0x100 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTBC(b)    		(0x104 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRPC(b)    		(0x108 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRBC(b)    		(0x10c | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRBPC(b)    		(0x110 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRMPC(b)    		(0x114 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRRPC(b)    		(0x118 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRLPC(b)    		(0x11c | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRCEPC(b)   		(0x120 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PTFC(b)   		(0x124 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PRBSR(b)   		(0x130 | CR_PTM_BASE | (b*PTM_B1_OFFSET))
#define CR_PTM_PDRPCR(b, ring)	((0x138 | CR_PTM_BASE | (b*PTM_B1_OFFSET)) + (ring) * 4)
#define CR_PTM_PRPPCR(b, ring)	((0x140 | CR_PTM_BASE | (b*PTM_B1_OFFSET)) + (ring) * 4)
#define CR_PTM_PESR(b)   		(0x148 | CR_PTM_BASE | (b*PTM_B1_OFFSET))

/***************************************
 * Ethernet Module Register Definition *
 ***************************************/

/* CR_PTM_PRCR */
#define PRCT_HW_RESET				(1<<0)
#define PRCT_SW_RESET				(1<<1)

/* CR_PTM_PIMR or CR_PTM_PISR */
#define PIMR_CH0_TX_DONE			(1<<0)
#define PIMR_CH0_RX_DONE			(1<<1)
#define PIMR_CH0_RX_FIFO_OVR		(1<<2)
#define PIMR_CH0_DMA_ERR			(1<<3)
#define PIMR_CH1_TX_DONE			(1<<4)
#define PIMR_CH1_RX_DONE			(1<<5)
#define PIMR_CH1_RX_FIFO_OVR		(1<<6)
#define PIMR_CH1_DMA_ERR			(1<<7)
#define PIMR_CH0_TX_OWNER_ERR		(1<<8)
#define PIMR_CH0_RX_OWNER_ERR		(1<<9)
#define PIMR_CH1_TX_OWNER_ERR		(1<<10)
#define PIMR_CH1_RX_OWNER_ERR		(1<<11)
#define PIMR_CH0_RX_RING_FULL		(1<<12)
#define PIMR_CH1_RX_RING_FULL		(1<<13)

/* CR_PTM_PFSR */
#define PFSR_TXPAD_EN				(1<<28)
#define PFSR_HIGH_SPD_DROP			(1<<25)
#define PFSR_NAT_EN					(1<<24)
#define PFSR_RX_RUNT				(1<<10)
#define PFSR_RX_LONG				(1<<11)
#define PFSR_RX_ALL					(1<<12)
#define PFSR_RX_BCAST				(1<<16)
#define PFSR_RX_MCAST				(1<<17)
#define PFSR_RX_CRC_DIS				(1<<18)
#define PFSR_RX_UNTAG				(1<<19)

/* CR_PTM_PPRCTR */
#define PPRCTR_RX_PKT_DONE_SHIFT	(16)
#define PPRCTR_TX_PKT_DONE_SHIFT	(0)

/* CR_PTM_PDER */
#define PDER_CH0_RX_DMA_EN			(1<<0)
#define PDER_CH0_TX_DMA_EN			(1<<1)
#define PDER_CH1_RX_DMA_EN			(1<<2)
#define PDER_CH1_TX_DMA_EN			(1<<3)

/* CR_PTM_PSGCR */
#define PSGCR_SG_EN					(1<<31)
#define PSGCR_SG_PKT_LEN_MASK		(0x7ff)

/***************************
 * PTM Transmit Descriptor *
 ***************************/

typedef union {
	struct {
    	uint32 owner      : 1;
    	uint32 reserved   : 12;
    	uint32 txhdr_size : 11;
    	uint32 reserved_1 : 4;
    	uint32 ur_abort   : 1;
    	uint32 txpkt_ur   : 1;
    	uint32 reserved_2 : 2;
	} bits;
	uint32 word;
} tdes0_t;

typedef union {
	struct {
	  	uint32 reserved   : 1;
    	uint32 time_pkt   : 1;
    	uint32 txic       : 1;
    	uint32 ipcs_ins   : 1;
    	uint32 vlan_ins   : 1;
    	uint32 vlan_tag   : 16;
    	uint32 txbuf_size : 11;
	} bits;
	uint32 word;
} tdes1_t;

typedef union {
	uint32 txbuf_addr;
	uint32 word;
} tdes2_t;

typedef union {
	uint32 txhdr_addr;
	uint32 word;
} tdes3_t;

typedef struct ptmTxDescr_s {
	tdes0_t tdes0;
	tdes1_t tdes1;
	tdes2_t tdes2;
  	tdes3_t tdes3;
} ptmTxDescr_t;

/**************************
 * PTM Receive Descriptor *
 **************************/

typedef union {
	struct {
    	uint32 owner  	  : 1;
    	uint32 vlan_hit   : 1;
    	uint32 pppoe_pkt  : 1;
    	uint32 tcp_ack    : 1;
    	uint32 ip_err     : 1;
    	uint32 l3id       : 3;
    	uint32 eth_pkt    : 1;
    	uint32 collision  : 1;
    	uint32 runt       : 1;
    	uint32 ftl        : 1;
    	uint32 crc_err    : 1;
    	uint32 rx_err     : 1;
    	uint32 broadcast  : 1;
    	uint32 multicast  : 1;
    	uint32 l4id       : 3;
    	uint32 priority   : 2;
    	uint32 rx_length  : 11;
	} bits;
	uint32 word;
} rdes0_t;

typedef union {
	struct {
		uint32 ip_fragmt  : 1;
    	uint32 ip_option  : 1;
    	uint32 time_pkt   : 1;
    	uint32 rxlen_err  : 1;
    	uint32 align_err  : 1;
    	uint32 vlan_tag   : 16;
		uint32 rx_buf_size: 11;
	} bits;
	uint32 word;
} rdes1_t;

typedef union {
	uint32 rxbuf_addr;
	uint32 word;
} rdes2_t;

typedef union {
	uint32 rxhdr_addr;
	uint32 word;
} rdes3_t;

typedef union {
	uint32 word;
} rdes4_t;
typedef union {
	uint32 word;
} rdes5_t;
typedef union {
	uint32 word;
} rdes6_t;
typedef union {
	uint32 word;
} rdes7_t;

typedef struct ptmRxDescr_s {
	rdes0_t rdes0;
	rdes1_t rdes1;
	rdes2_t rdes2;
  	rdes3_t rdes3;
	rdes4_t rdes4;
	rdes5_t rdes5;
	rdes6_t rdes6;
	rdes7_t rdes7;
} ptmRxDescr_t;

/*********************
 * PTM Control Block *
 *********************/

/* ----- MIB-II ----- */
typedef struct ptmMIB_II_s {
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
} ptmMIB_II_t;

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
	uint32 rxOverrunInt;

	uint32 rxPktIPChkSumErr;
} inSiliconStat_t;

typedef struct ptmRxMemPool_s {
	ptmRxDescr_t rxDescpBuf[RX_QUEUE_NUM][PTM_RXDESCP_NO];
} ptmRxMemPool_t;

typedef struct ptmTxMemPool_s {
	ptmTxDescr_t txDescpBuf[TX_QUEUE_NUM][PTM_TXDESCP_NO];
} ptmTxMemPool_t;

/* ----- Statistics for GMAC ----- */
typedef struct ptmStat_s {
	ptmMIB_II_t MIB_II;	/* MIB-II */
	inSiliconStat_t inSilicon;	
} ptmStat_t;

/* ----- Adapter Card Table ------ */
typedef struct ptmAdapter_s {
	int bearer;
	int ptm_intr;

	uint8  macAddr[6];  /* MAC-Address */

	ptmRxMemPool_t      *ptmRxMemPool_p;
  	dma_addr_t          ptmRxMemPool_phys_p;
	ptmTxMemPool_t 		*ptmTxMemPool_p;
  	dma_addr_t          ptmTxMemPool_phys_p;

	ptmStat_t           ptmStat;
	uint8 statisticOn;  /* Flag to record statistics or not */

	uint32 txDescrRingBaseVAddr[TX_QUEUE_NUM];  /* Transmit Descr Ring Virtual Address */
	uint32 rxDescrRingBaseVAddr[RX_QUEUE_NUM];  /* Receive Descr Ring Virtual Address */
	uint32 txCurrentDescp[TX_QUEUE_NUM];        /* index to current tx descriptor */
	uint32 txUnReleasedDescp[TX_QUEUE_NUM];     /* index to the unreleased descp of Tx */
	uint32 txUnReleasedBufCnt[TX_QUEUE_NUM];    /* Unreleased buffer cnt hold by Tx	*/
	uint32 rxCurrentDescp[RX_QUEUE_NUM];        /* index to current rx descriptor */

	uint32 rxRingSize;  /* Receive Descr Ring Size */
	uint32 txRingSize;  /* Receive Descr Ring Size */
	uint32 rxDescrSize; /* The size of a Rx descr entry in the descr ring */
	uint32 txDescrSize; /* The size of a Tx descr entry in the descr ring */

	struct sk_buff *txskbs[TX_QUEUE_NUM][PTM_TXDESCP_NO];
	struct sk_buff *rxskbs[RX_QUEUE_NUM][PTM_RXDESCP_NO];
#ifdef TC3262_PTM_SG_MODE
	struct sk_buff *sg_rxskbs[RX_QUEUE_NUM][PTM_RXDESCP_NO];
#endif

	spinlock_t lock;
	struct timer_list ptm_timer;
	struct net_device_stats stats;

#if KERNEL_2_6_36
	struct net_device	*dev;
	struct napi_struct	napi;
#endif
} ptmAdapter_t;

/************************************************************************
*        F U N C T I O N   P R O T O T Y P E S
*************************************************************************
*/

#endif /* _TC3262PTM_H */

