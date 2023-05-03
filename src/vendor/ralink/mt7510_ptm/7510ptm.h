
#ifndef _TC3262PTM_H
#define _TC3262PTM_H

#define KERNEL_2_6_36 		(LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31))

#define read_reg_word(reg) 			regRead32(reg)
#define write_reg_word(reg, wdata) 	regWrite32(reg, wdata)

#define REGISTER_TESTTYPE			0
#define DATA_PATH_TESTTYPE			1
#define PATH_NO_TESTTYPE			2
#define PREEMPTION_TESTTYPE			3
#define SP_PRIORITY_TESTTYPE		4
#define WRR_PRIORITY_TESTTYPE		5
#define SP_WRR_TESTTYPE				6
#define VLAN_TAG_TESTTYPE			7
#define VLAN_UNTAG_TESTTYPE			8
#define VLAN_DBL_TAG_TESTTYPE		9
#define VLAN_DBL_UNTAG_TESTTYPE		10
#define IPCS_INSERT_TESTTYPE		11
#define IPCS_CHECK_TESTTYPE			12
#define BACP_PKT_TESTTYPE			13
#define RUNT_PKT_TESTTYPE			14
#define LONG_PKT_TESTTYPE			15
#define CRCERR_PKT_TESTTYPE			17
#define DATA_BURST_TESTTYPE			18
#define MULTI_CHANNEL_TESTTYPE		19
#define SINGLE_PKT_TESTTYPE			80
#define	ALL_LOOPBACK_TESTTYPE		88


#define ETHER_TYPE_IP		0x0800
#define ETHER_TYPE_8021Q	0x8100
#define ETHER_TYPE_8021AD	0x88a8
#define ETHER_TYPE_QINQ		0x9100
#define ETHER_TYPE_SLOW		0x8809


#define TO_LINE0            0
#define TO_LINE1            1
#define TO_BOTH_LINES       2
#define TO_NO_BONDING       3


/*****************************
 * PTM Module Registers      *
 *****************************/
#define LINE_NUM				2
#define BEARER_NUM				2
#define PREEMPT_NUM				2
#define PATH_NUM				(LINE_NUM * BEARER_NUM * PREEMPT_NUM)

#define TX_QUEUE_NUM			8
#define TX_QUEUE_LEN			64
#define RX_QUEUE_NUM			1
#define RX_QUEUE_LEN			128

#if KERNEL_2_6_36
#define PTM_NAPI_WEIGHT			(RX_QUEUE_LEN >> 1)
#endif

#define PTM_BEARER_0			(0x0)
#define PTM_BEARER_1	      	(0x1)

#define PATH_LINE_SHIFT			2
#define	PATH_BEARER_SHIFT		1
#define PATH_PREEMPT_SHIFT		0

#define TX_QUEUE_MASK			0x7

/*****************************
 * PTM MAC REGISTERS	     *
 *****************************/

#define PTM_REG_BASE			0xBFB62000
#define PTM_RST_REG				(0x00 | PTM_REG_BASE)
#define PTM_CTRL_REG			(0x04 | PTM_REG_BASE)
#define PTM_BACP_FIELD_0		(0x10 | PTM_REG_BASE)
#define PTM_BACP_FIELD_1		(0x14 | PTM_REG_BASE)
#define PTM_BACP_FIELD_2		(0x18 | PTM_REG_BASE)
#define PTM_BACP_FIELD_3		(0x1c | PTM_REG_BASE)
#define PTM_BACP_FIELD_EN		(0x20 | PTM_REG_BASE)
#define PTM_TX_UBUF_WR_CNT_L0	(0x80 | PTM_REG_BASE)
#define PTM_TX_UBUF_WR_CNT_L1	(0x84 | PTM_REG_BASE)
#define PTM_TX_UBUF_RD_CNT_L0	(0x88 | PTM_REG_BASE)
#define PTM_TX_UBUF_RD_CNT_L1	(0x8c | PTM_REG_BASE)
#define PTM_TX_UBUF_CNT_CTRL	(0x90 | PTM_REG_BASE)
#define PTM_RX_UBUF_CNT			(0x94 | PTM_REG_BASE)
#define PTM_RX_UBUF_CNT_CTRL	(0x98 | PTM_REG_BASE)
#define TMAC_PKT_CNT_P0			(0x100 | PTM_REG_BASE)
#define TMAC_PKT_CNT_P1			(0x104 | PTM_REG_BASE)
#define TMAC_PKT_CNT_P2			(0x108 | PTM_REG_BASE)
#define TMAC_PKT_CNT_P3			(0x10c | PTM_REG_BASE)
#define TMAC_PKT_CNT_P4			(0x110 | PTM_REG_BASE)
#define TMAC_PKT_CNT_P5			(0x114 | PTM_REG_BASE)
#define TMAC_PKT_CNT_P6			(0x118 | PTM_REG_BASE)
#define TMAC_PKT_CNT_P7			(0x11c | PTM_REG_BASE)
#define TMAC_PKT_CNT_CLR		(0x120 | PTM_REG_BASE)
#define RMAC_PKT_CNT_P0			(0x130 | PTM_REG_BASE)
#define RMAC_PKT_CNT_P1			(0x134 | PTM_REG_BASE)
#define RMAC_PKT_CNT_P2			(0x138 | PTM_REG_BASE)
#define RMAC_PKT_CNT_P3			(0x13c | PTM_REG_BASE)
#define RMAC_PKT_CNT_P4			(0x140 | PTM_REG_BASE)
#define RMAC_PKT_CNT_P5			(0x144 | PTM_REG_BASE)
#define RMAC_PKT_CNT_P6			(0x148 | PTM_REG_BASE)
#define RMAC_PKT_CNT_P7			(0x14c | PTM_REG_BASE)
#define RMAC_PKT_CNT_CLR		(0x150 | PTM_REG_BASE)
#define RMAC_CRCE_CNT_P0		(0x160 | PTM_REG_BASE)
#define RMAC_CRCE_CNT_P1		(0x164 | PTM_REG_BASE)
#define RMAC_CRCE_CNT_P2		(0x168 | PTM_REG_BASE)
#define RMAC_CRCE_CNT_P3		(0x16c | PTM_REG_BASE)
#define RMAC_CRCE_CNT_P4		(0x170 | PTM_REG_BASE)
#define RMAC_CRCE_CNT_P5		(0x174 | PTM_REG_BASE)
#define RMAC_CRCE_CNT_P6		(0x178 | PTM_REG_BASE)
#define RMAC_CRCE_CNT_P7		(0x17c | PTM_REG_BASE)
#define RMAC_CRCE_CNT_CLR		(0x180 | PTM_REG_BASE)


/*****************************
 * PTM_RST_REG defination    *
 *****************************/

#define RST_RESET				(1 << 1)
#define RST_SW_RESET			(1 << 0)

/*****************************
 * PTM_CTRL_REG defination   *
 *****************************/

#define CTRL_TX_FIFO_SIZE		(1 << 24)
#define U_CELL_BASE_MODE		(1 << 17)
#define UL_PEDGE_DRV			(1 << 16)
#define CTRL_RX_EN				(1 << 8) 
#define CTRL_TX_EN				(1 << 0)

/***********************************
 * PTM_BACP_ENABLE_BIT defination   *
 ***********************************/
 #define BACP_DST_MAC_EN		(1<<0)
 #define BACP_ETHER_TYPE_EN		(1<<1)
 #define BACP_ORG_SUB_EN		(1<<2)
 #define BACP_ITU_OUI_EN		(1<<3)
 #define BACP_ITU_SUB_EN		(1<<4)
 #define BACP_VERSION_EN		(1<<5)

/*****************************
 * PTM_RX_ERR_BITS defination *
 *****************************/
#define PTM_RX_W0_ERR_BITS		(0x7<<5)
#define PTM_RX_W1_ERR_BITS		((0x1<<23)|(0x1<<26))

/* Master SCU register*/
#define SCU_RESET_REG			0xbfb00834

/*****************************
 * Master Bonding Registers  *
 *****************************/
#define BONDING_REG_BASE		0xbfb6f000
#define BONDING_COMMON1			(0x004 | BONDING_REG_BASE)
#define BONDING_TXPAF_CFG0		(0x008 | BONDING_REG_BASE)
#define BONDING_TXPAF_CFG1		(0x00c | BONDING_REG_BASE)
#define BONDING_RXPAF_CFG0		(0x010 | BONDING_REG_BASE)
#define BONDING_RXPAF_CFG1		(0x014 | BONDING_REG_BASE)
#define BONDING_LINE0_B0_MAX	(0x018 | BONDING_REG_BASE)
#define BONDING_LINE0_B1_MAX	(0x01c | BONDING_REG_BASE)
#define BONDING_LINE1_B0_MAX	(0x020 | BONDING_REG_BASE)
#define BONDING_LINE1_B1_MAX	(0x024 | BONDING_REG_BASE)
#define BONDING_LINE0_B0_MIN	(0x028 | BONDING_REG_BASE)
#define BONDING_LINE0_B1_MIN	(0x02c | BONDING_REG_BASE)
#define BONDING_LINE1_B0_MIN	(0x030 | BONDING_REG_BASE)
#define BONDING_LINE1_B1_MIN	(0x034 | BONDING_REG_BASE)
#define BONDING_RX_CFG			(0x038 | BONDING_REG_BASE)
#define BONDING_TXFRAG0			(0x03c | BONDING_REG_BASE)
#define BONDING_TXFRAG1			(0x040 | BONDING_REG_BASE)
#define BONDING_TXFRAG2			(0x044 | BONDING_REG_BASE)
#define BONDING_TXFRAG3			(0x048 | BONDING_REG_BASE)
#define BONDING_TXFRAG4			(0x04c | BONDING_REG_BASE)
#define BONDING_TXFRAG5			(0x050 | BONDING_REG_BASE)
#define BONDING_TXFRAG6			(0x054 | BONDING_REG_BASE)
#define BONDING_TXFRAG7			(0x058 | BONDING_REG_BASE)
#define BONDING_RXFRAG0			(0x05c | BONDING_REG_BASE)
#define BONDING_RXFRAG1			(0x060 | BONDING_REG_BASE)
#define BONDING_RXFRAG2			(0x064 | BONDING_REG_BASE)
#define BONDING_RXFRAG3			(0x068 | BONDING_REG_BASE)
#define BONDING_RXFRAG4			(0x06c | BONDING_REG_BASE)
#define BONDING_RXFRAG5			(0x070 | BONDING_REG_BASE)
#define BONDING_RXFRAG6			(0x074 | BONDING_REG_BASE)
#define BONDING_RXFRAG7			(0x078 | BONDING_REG_BASE)
#define BONDING_RXPKT0			(0x07c | BONDING_REG_BASE)
#define BONDING_RXPKT1			(0x080 | BONDING_REG_BASE)
#define BONDING_RXPKT2			(0x084 | BONDING_REG_BASE)
#define BONDING_RXPKT3			(0x088 | BONDING_REG_BASE)
#define BONDING_RXPKT4			(0x08c | BONDING_REG_BASE)
#define BONDING_RXPKT5			(0x090 | BONDING_REG_BASE)
#define BONDING_RXPKT6			(0x094 | BONDING_REG_BASE)
#define BONDING_RXPKT7			(0x098 | BONDING_REG_BASE)
#define BONDING_RXERRPKT0		(0x09c | BONDING_REG_BASE)
#define BONDING_RXERRPKT1		(0x0a0 | BONDING_REG_BASE)
#define BONDING_RXERRPKT2		(0x0a4 | BONDING_REG_BASE)
#define BONDING_RXERRPKT3		(0x0a8 | BONDING_REG_BASE)
#define BONDING_RXERRPKT4		(0x0ac | BONDING_REG_BASE)
#define BONDING_RXERRPKT5		(0x0b0 | BONDING_REG_BASE)
#define BONDING_RXERRPKT6		(0x0b4 | BONDING_REG_BASE)
#define BONDING_RXERRPKT7		(0x0b8 | BONDING_REG_BASE)
#define BONDING_TXPKTCFG		(0x0bc | BONDING_REG_BASE)
#define BONDING_MASTER_ADR		(0x0c0 | BONDING_REG_BASE)
#define BONDING_SLAVE_ADR		(0x0c4 | BONDING_REG_BASE)
#define BONDING_RBUS_CFG		(0x0d0 | BONDING_REG_BASE)
#define BONDING_TX_MEM_CFG		(0x0d4 | BONDING_REG_BASE)
#define BONDING_DEBUG_OUT1		(0x0e0 | BONDING_REG_BASE)
#define BONDING_DEBUG_CTL1		(0x0e4 | BONDING_REG_BASE)
#define BONDING_RBUS_STATUS0	(0x0f0 | BONDING_REG_BASE)
#define BONDING_MEM_CFG			(0x0f4 | BONDING_REG_BASE)
#define BONDING_DEBUG_CTL		(0x0f8 | BONDING_REG_BASE)
#define BONDING_DEBUF_OUT0		(0x0fc | BONDING_REG_BASE)
#define BONDING_U2R_TX			(0x100 | BONDING_REG_BASE)
#define BONDING_R2U_TX			(0x104 | BONDING_REG_BASE)
#define BONDING_R2U_RX			(0x108 | BONDING_REG_BASE)
#define BONDING_U2R_RX			(0x10c | BONDING_REG_BASE)

/* Slave SCU register offset */
#define S_SCU_RESET_REG_OFF			0x834

/*********************************
 * SLAVE Bonding Registers Offset *
 *********************************/
#define S_BONDING_REG_BASE_OFF		0x0
#define S_BONDING_COMMON1_OFF		0x004
#define S_BONDING_TXPAF_CFG0_OFF	0x008
#define S_BONDING_TXPAF_CFG1_OFF	0x00c
#define S_BONDING_RXPAF_CFG0_OFF	0x010
#define S_BONDING_RXPAF_CFG1_OFF	0x014
#define S_BONDING_LINE0_B0_MAX_OFF	0x018
#define S_BONDING_LINE0_B1_MAX_OFF	0x01c
#define S_BONDING_LINE1_B0_MAX_OFF	0x020
#define S_BONDING_LINE1_B1_MAX_OFF	0x024
#define S_BONDING_LINE0_B0_MIN_OFF	0x028
#define S_BONDING_LINE0_B1_MIN_OFF	0x02c
#define S_BONDING_LINE1_B0_MIN_OFF	0x030
#define S_BONDING_LINE1_B1_MIN_OFF	0x034
#define S_BONDING_RX_CFG_OFF		0x038
#define S_BONDING_TXFRAG0_OFF		0x03c
#define S_BONDING_TXFRAG1_OFF		0x040
#define S_BONDING_TXFRAG2_OFF		0x044
#define S_BONDING_TXFRAG3_OFF		0x048
#define S_BONDING_TXFRAG4_OFF		0x04c
#define S_BONDING_TXFRAG5_OFF		0x050
#define S_BONDING_TXFRAG6_OFF		0x054
#define S_BONDING_TXFRAG7_OFF		0x058
#define S_BONDING_RXFRAG0_OFF		0x05c
#define S_BONDING_RXFRAG1_OFF		0x060
#define S_BONDING_RXFRAG2_OFF		0x064
#define S_BONDING_RXFRAG3_OFF		0x068
#define S_BONDING_RXFRAG4_OFF		0x06c
#define S_BONDING_RXFRAG5_OFF		0x070
#define S_BONDING_RXFRAG6_OFF		0x074
#define S_BONDING_RXFRAG7_OFF		0x078
#define S_BONDING_RXPKT0_OFF		0x07c
#define S_BONDING_RXPKT1_OFF		0x080
#define S_BONDING_RXPKT2_OFF		0x084
#define S_BONDING_RXPKT3_OFF		0x088
#define S_BONDING_RXPKT4_OFF		0x08c
#define S_BONDING_RXPKT5_OFF		0x090
#define S_BONDING_RXPKT6_OFF		0x094
#define S_BONDING_RXPKT7_OFF		0x098
#define S_BONDING_RXERRPKT0_OFF		0x09c
#define S_BONDING_RXERRPKT1_OFF		0x0a0
#define S_BONDING_RXERRPKT2_OFF		0x0a4
#define S_BONDING_RXERRPKT3_OFF		0x0a8
#define S_BONDING_RXERRPKT4_OFF		0x0ac
#define S_BONDING_RXERRPKT5_OFF		0x0b0
#define S_BONDING_RXERRPKT6_OFF		0x0b4
#define S_BONDING_RXERRPKT7_OFF		0x0b8
#define S_BONDING_TXPKTCFG_OFF		0x0bc
#define S_BONDING_MASTER_ADR_OFF	0x0c0
#define S_BONDING_SLAVE_ADR_OFF		0x0c4
#define S_BONDING_RBUS_CFG_OFF		0x0d0
#define S_BONDING_TX_MEM_CFG_OFF	0x0d4
#define S_BONDING_DEBUG_OUT1_OFF	0x0e0
#define S_BONDING_DEBUG_CTL1_OFF	0x0e4
#define S_BONDING_RBUS_STATUS0_OFF	0x0f0
#define S_BONDING_MEM_CFG_OFF		0x0f4
#define S_BONDING_DEBUG_CTL_OFF		0x0f8
#define S_BONDING_DEBUG_OUT0_OFF	0x0fc
#define S_BONDING_U2R_TX_OFF		0x100
#define S_BONDING_R2U_TX_OFF		0x104
#define S_BONDING_R2U_RX_OFF		0x108
#define S_BONDING_U2R_RX_OFF		0x10c

/*****************************
 * BONDING COMMON1 REG DEF   *
 *****************************/
#define COMMON1_UTOPIA_RX_EN	(1<<16)

/*****************************
 * FRAME ENGINE REGISTERS OFFSET *
 *****************************/

#define FE_GLO_CFG_OFF          (0x0000)
#define CDMP_VLAN_CT_OFF		(0x0400)
#define CDM_VLAN_GE_OFF         (0x1400)
#define GDM2_FWD_CFG_OFF		(0x1500)
#define GDM2_LEN_CFG_OFF		(0x1524)
#define GDM2_CHN_EN_OFF			(0x152c)
#define GDM2_TX_GET_CNT_OFF		(0x1600)
#define GDM2_TX_OK_CNT_OFF		(0x1604)
#define GDM2_TX_DROP_CNT_OFF	(0x1608)
#define GDM2_TX_OK_BYTE_CNT_OFF	(0x160c)
#define GDM2_RX_OK_CNT_OFF		(0x1650)
#define GDM2_RX_OVER_DROP_CNT_OFF	(0x1654)
#define GDM2_RX_ERROR_DROP_CNT_OFF	(0x1658)
#define GDM2_RX_OK_BYTE_CNT_OFF		(0x165c)

/*****************************
 * QDMA	 REGISTERS           *
 *****************************/
 
#define QDMA_REG_BASE			0xbfb51800
#define QDMA_TX_PKT_SUM			(0x0104 | QDMA_REG_BASE)
#define QDMA_RX_PKT_SUM			(0x0110 | QDMA_REG_BASE)
#define QDMA_RCDROP_FWD_GREEN	(0x0120 | QDMA_REG_BASE)
#define QDMA_RCDROP_CPU_GREEN	(0x012c | QDMA_REG_BASE)

/******************************
* Master PTM-TC registers     *
******************************/

#define TPSTC_REG_BASE			0xbf900000
#define TPSTC_TX_CFG			(0x0f00 | TPSTC_REG_BASE)
#define TPSTC_TX_B0_OK			(0x0f24 | TPSTC_REG_BASE)
#define TPSTC_TX_B0_FIFO_UNDERRUN (0x0f28 | TPSTC_REG_BASE)
#define TPSTC_TX_B1_OK			(0x0f38 | TPSTC_REG_BASE)
#define TPSTC_TX_B1_FIFO_UNDERRUN (0x0f3c | TPSTC_REG_BASE)
#define TPSTC_RX_CFG			(0x1080 | TPSTC_REG_BASE)
#define TPSTC_RX_B0_OK			(0x1094 | TPSTC_REG_BASE)
#define TPSTC_RX_B1_OK			(0x1098 | TPSTC_REG_BASE)
#define TPSTC_RX_FIFO_FULL		(0x10a0 | TPSTC_REG_BASE)
#define TPSTC_RX_CRC_ERR		(0x10a8 | TPSTC_REG_BASE)

/****************************
* PTM-TC REG DEF
*****************************/

#define TPSTC_MODE_MASK			0xf
#define TPSTC_PTM_MODE			0x2
#define TPSTC_REDUNDANT_MODE	(1<<30)


/******************************
* Slave PTM-TC register Offset *
******************************/

#define TPSTC_TX_CFG_OFF			(0x0f00)
#define TPSTC_TX_B0_OK_OFF			(0x0f24)
#define TPSTC_TX_B0_FIFO_UNDERRUN_OFF (0x0f28)
#define TPSTC_TX_B1_OK_OFF			(0x0f38)
#define TPSTC_TX_B1_FIFO_UNDERRUN_OFF (0x0f3c)
#define TPSTC_RX_CFG_OFF			(0x1080)
#define TPSTC_RX_B0_OK_OFF			(0x1094)
#define TPSTC_RX_B1_OK_OFF			(0x1098)
#define TPSTC_RX_FIFO_FULL_OFF		(0x10a0)
#define TPSTC_RX_CRC_ERR_OFF		(0x10a8)

/*****************************
 * DEBUG UTILITY             *
 *****************************/
 
#define	DBG_L0		0
#define	DBG_L1		1
#define	DBG_L2		2
#define	DBG_L3		3
#define DBG_L4		4

#ifdef PTM_DEBUG
#define PTM_DBG(level, FORMAT, ARG...)	{ \
				if (ptmDbgLevel >= level) \
					printk(FORMAT, ##ARG); }
#else
#define PTM_DBG(level, FORMAT, ARG...)
#endif
					
/*********************
 * PTM Control Block *
	uint8 modemst;
 *********************/

typedef struct ptmMIB_s
{
	uint32 rxBytes;			/* Receive Octets */
	uint32 rxPkts;			/* Receive Packets */
	uint32 rxDiscards;		/* Receive Discard Packets */
	uint32 rxErrors;		/* Receive Error Packets */
	uint32 txBytes;			/* Transmit Octets */
	uint32 txPkts;			/* Transmit Packets */
	uint32 txDiscards;		/* Transmit Discard Packets */
	uint32 rxErrCrc;
	uint32 rxErrLong;
	uint32 rxErrRunt;
	uint32 rxErrIp4Cs;
	uint32 rxErrL4Cs;

} ptmMIB_t;


typedef struct ptmStats_s
{
	ptmMIB_t ptmMIB;

} ptmStats_t;


typedef struct ptmAdapter_s
{
	uint8 bearer;
	uint8 txCntLimiter[TX_QUEUE_NUM];
	ptmStats_t ptmStats;
	struct net_device_stats stats;
#if KERNEL_2_6_36
	struct net_device	*dev;
	struct napi_struct	napi;
#endif
#ifdef TR068_LED
	struct timer_list ptm_timer;
#endif
} ptmAdapter_t;


typedef union
{
	struct
	{
		uint32 resv			: 1;
		uint32 tsid			: 5;
		uint32 tse			: 1;
		uint32 dei			: 1;
		uint32 resv2		: 15;
		uint32 oam			: 1;
		uint32 pathNo		: 4;	
		uint32 queue		: 4;	
	} bits;
	uint32 word;

} txMsgWord0_t;


typedef union
{
	struct
	{
		uint32 ipCsIns		: 1;
		uint32 uco			: 1;
		uint32 tco			: 1;
		uint32 tso			: 1;
		uint32 resv			: 6;
		uint32 fPort		: 3;
		uint32 vlanEn		: 1;
		uint32 vlanTpID		: 2;
		uint32 vlanTag		: 16;	 
	} bits;
	uint32 word;

} txMsgWord1_t;


typedef union
{
	struct
	{
		uint32 resv			: 2;
		uint32 isRaw		: 1;
		uint32 resv2		: 20;
		uint32 isBacp		: 1;
		uint32 crcErr		: 1;
		uint32 isLong		: 1;
		uint32 isRunt		: 1;
		uint32 oam			: 1;
		uint32 pathNo		: 4;		
	} bits;
	uint32 word;

} rxMsgWord0_t;


typedef union
{
	struct
	{
		uint32 resv			: 3;
		uint32 ipv6			: 1;
		uint32 ipv4			: 1;
		uint32 ip4CsErr		: 1;
		uint32 tcpAck		: 1;
		uint32 l4Valid		: 1;
		uint32 l4CsErr		: 1;
		uint32 sPort		: 4;
		uint32 crsn			: 5;
		uint32 ppeEntry		: 14;		
	} bits;
	uint32 word;

} rxMsgWord1_t;


typedef union
{
	struct
	{
		uint32 resv			: 13;
		uint32 vlanHit		: 1;
		uint32 vlanTpID		: 2;
		uint32 vlanTag		: 16;	
	} bits;
	uint32 word;

} rxMsgWord2_t;


typedef struct ptmTxMsg_s
{
	txMsgWord0_t txMsgW0;
	txMsgWord1_t txMsgW1;

} ptmTxMsg_t;


typedef struct ptmRxMsg_s
{
	rxMsgWord0_t rxMsgW0;
	rxMsgWord1_t rxMsgW1;
	rxMsgWord2_t rxMsgW2;
	uint32	resv;
	
} ptmRxMsg_t;

struct ptmTxMsgBuf_s;

struct ptmTxMsgBuf_s
{
	ptmTxMsg_t ptmTxMsg;
	uint32 resv;
	struct ptmTxMsgBuf_s *nextAvaiMsg;
#ifdef MSG_CACHED
	/* don't use following 16 bytes!
	 * It might be destroyed when 
	 * ptmRxMsg is write-back to dram
	 * because cache is 32-byte align! */
	uint32 dontUse[4];
#endif
} ; 

typedef struct ptmTxMsgBuf_s ptmTxMsgBuf_t;

typedef struct ptmRxMsgBuf_s
{
	ptmRxMsg_t ptmRxMsg;
#ifdef MSG_CACHED
	/* don't use following 16 bytes!
	 * It might be destroyed when 
	 * ptmRxMsg is write-back to dram
	 * because cache is 32-byte align! */
	uint32 dontUse[4];
#endif
} ptmRxMsgBuf_t;


#endif



