#ifndef _QDMA_REG_H_
#define _QDMA_REG_H_

#include <asm/io.h>

/*******************************************************
 register access utility
********************************************************/
#ifdef CONFIG_SIMULATION
	#define IO_GREG(reg)							0				
	#define IO_SREG(reg, value)					
	#define IO_SBITS(reg, bit)					
	#define IO_CBITS(reg, bit)					
	#define IO_GREG_REP(reg, buf, count)			0
	#define IO_SREG_REP(reg, buf, count)		
#else
	#define IO_GREG(reg)							ioread32((void __iomem *)(reg))
	#define IO_SREG(reg, value)						iowrite32(value, (void __iomem *)(reg))
	#define IO_GMASK(reg, mask, shift)				((ioread32((void __iomem *)(reg)) & mask) >> shift)
	#define IO_SMASK(reg, mask, shift, value)		{ uint t = ioread32((void __iomem *)(reg)); iowrite32(((t&~(mask))|((value<<shift)&mask)), (void __iomem *)(reg)); }
	#define IO_SBITS(reg, bit)						{ uint t = ioread32((void __iomem *)(reg)); iowrite32((t|bit), (void __iomem *)(reg)); }
	#define IO_CBITS(reg, bit)						{ uint t = ioread32((void __iomem *)(reg)); iowrite32((t&~(bit)), (void __iomem *)(reg)); }
	#define IO_GREG_REP(reg, buf, count)			ioread32_rep((void __iomem *)(reg), buf, count)
	#define IO_SREG_REP(reg, buf, count)			iowrite32_rep((void __iomem *)(reg), buf, count)
#endif /* CONFIG_SIMULATION */


/*******************************************************
********************************************************/
#define CONFIG_QDMA_BASE_ADDR						(0x1FB51800)

#define CONFIG_QDMA_IRQ								(23)


/*******************************************************
 CSR for QDMA
********************************************************/
#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	#define QDMA_CSR_INFO(base)							(base+0x0000)
	#define QDMA_CSR_GLB_CFG(base)						(base+0x0004)
	#define QDMA_CSR_TX_DSCP_BASE(base)					(base+0x0008)
	#define QDMA_CSR_RX_DSCP_BASE(base)					(base+0x000C)
	#define QDMA_CSR_TX_CPU_IDX(base)					(base+0x0010)
	#define QDMA_CSR_TX_DMA_IDX(base)					(base+0x0014)
	#define QDMA_CSR_RX_CPU_IDX(base)					(base+0x0018)
	#define QDMA_CSR_RX_DMA_IDX(base)					(base+0x001C)
	
	#define QDMA_CSR_HWFWD_DSCP_BASE(base)				(base+0x0020)
	#define QDMA_CSR_HWFWD_AVAIL_DSCP_NUM(base)			(base+0x0024)
	#define QDMA_CSR_HWFWD_USED_DSCP_NUM(base)			(base+0x0028)
	#define QDMA_CSR_HWFWD_TX_IDX(base)					(base+0x0030)
	#define QDMA_CSR_HWFWD_RX_IDX(base)					(base+0x0034)
	#define QDMA_CSR_HWFWD_FREE_IDX(base)				(base+0x0038)
	
	#define QDMA_CSR_INT_STATUS(base)					(base+0x0050)
	#define QDMA_CSR_INT_ENABLE(base)					(base+0x0054)
	#define QDMA_CSR_DELAY_INT_CFG(base)				(base+0x0058)
	#define QDMA_CSR_IRQ_BASE(base)						(base+0x0060)
	#define QDMA_CSR_IRQ_CFG(base)						(base+0x0064)
	#define QDMA_CSR_IRQ_CLEAR_LEN(base)				(base+0x0068)
	#define QDMA_CSR_IRQ_STATUS(base)					(base+0x006C)
	#define QDMA_CSR_IRQ_PTIME(base)					(base+0x0070)
	
	#define QDMA_CSR_TXQOS_CHN07_CFG(base)				(base+0x0080)
	#define QDMA_CSR_TXQOS_CHN815_CFG(base)				(base+0x0084)
	#define QDMA_CSR_TXQOS_WRR_CFG(base)				(base+0x0088)
	#define QDMA_CSR_TXDMA_SCH_CFG(base)				(base+0x0090)
	#define QDMA_CSR_TXBUF_THR_CFG(base)				(base+0x0094)
	#define QDMA_CSR_TXQ_DROP_CFG(base)					(base+0x0098)
	
	#define QDMA_CSR_GPON_RPT_CFG(base)					(base+0x00C0)
	#define QDMA_CSR_GPON_TRTCM_CFG(base)				(base+0x00C4)
	
	#define QDMA_CSR_EPON_RPT_CFG(base)					(base+0x00E0)
	#define QDMA_CSR_EPON_QTHRESHLD_CFG(base)			(base+0x00E4)
	
	#define QDMA_CSR_DBG_TX_DSCP_SUM(base)				(base+0x01E0)
	#define QDMA_CSR_DBG_TX_PKT_SUM(base)				(base+0x01E4)
	#define QDMA_CSR_DBG_RX_PKT_SUM(base)				(base+0x01E8)
	#define QDMA_CSR_DBG_LMGR_STA(base)					(base+0x01EC)
#else
	#define QDMA_CSR_INFO(base)							(base+0x0000)
	#define QDMA_CSR_GLB_CFG(base)						(base+0x0004)
	#define QDMA_CSR_TX_DSCP_BASE(base)					(base+0x0008)
	#define QDMA_CSR_RX_DSCP_BASE(base)					(base+0x000C)
	#define QDMA_CSR_TX_CPU_IDX(base)					(base+0x0010)
	#define QDMA_CSR_TX_DMA_IDX(base)					(base+0x0014)
	#define QDMA_CSR_RX_CPU_IDX(base)					(base+0x0018)
	#define QDMA_CSR_RX_DMA_IDX(base)					(base+0x001C)
	
	#define QDMA_CSR_HWFWD_DSCP_BASE(base)				(base+0x0020)
	#define QDMA_CSR_HWFWD_TX_IDX(base)					(base+0x0024)
	#define QDMA_CSR_HWFWD_RX_IDX(base)					(base+0x0028)
	#define QDMA_CSR_HWFWD_FREE_IDX(base)				(base+0x002C)
	#define QDMA_CSR_HWFWD_AVAIL_DSCP_NUM(base)			(base+0x0030)
	#define QDMA_CSR_HWFWD_USED_DSCP_NUM(base)			(base+0x0034)
	#define QDMA_CSR_LMGR_CHNL_RETIRE(base)				(base+0x0040)
	#define QDMA_CSR_LMGR_CHNL_RETIRE_MASK(base)		(base+0x0044)
	
	#define QDMA_CSR_INT_STATUS(base)					(base+0x0050)
	#define QDMA_CSR_INT_ENABLE(base)					(base+0x0054)
	#define QDMA_CSR_DELAY_INT_CFG(base)				(base+0x0058)
	#define QDMA_CSR_IRQ_BASE(base)						(base+0x0060)
	#define QDMA_CSR_IRQ_CFG(base)						(base+0x0064)
	#define QDMA_CSR_IRQ_CLEAR_LEN(base)				(base+0x0068)
	#define QDMA_CSR_IRQ_STATUS(base)					(base+0x006C)
	#define QDMA_CSR_IRQ_PTIME(base)					(base+0x0070)
	
	#define QDMA_CSR_TXQOS_CHN07_CFG(base)				(base+0x0080)
	#define QDMA_CSR_TXQOS_CHN815_CFG(base)				(base+0x0084)
	#define QDMA_CSR_TXQOS_WRR_CFG(base)				(base+0x0088)
	#define QDMA_CSR_TXDMA_PREFETCH_CFG(base)			(base+0x008C)
	#define QDMA_CSR_TXBUF_THR_CFG(base)				(base+0x0090)
	
	#define QDMA_CSR_TXQ_CNGST_CFG(base)				(base+0x00A0)
	#define QDMA_CSR_TXQ_GRN_MAX_THRSHLD(base)			(base+0x00A4)
	#define QDMA_CSR_TXQ_GRN_MIN_THRSHLD(base)			(base+0x00A8)
	#define QDMA_CSR_TXQ_YLW_MAX_THRSHLD(base)			(base+0x00AC)
	#define QDMA_CSR_TXQ_YLW_MIN_THRSHLD(base)			(base+0x00B0)
	#define QDMA_CSR_TRTCM_CFG(base)					(base+0x00B4)
	
	#define QDMA_CSR_GPON_TCONT_COLOR_CFG(base)			(base+0x00C0)
	#define QDMA_CSR_GPON_TCONT_TRTCM_CFG(base)			(base+0x00C4)
	
	#define QDMA_CSR_EPON_RPT_CFG(base)					(base+0x00D0)
	#define QDMA_CSR_EPON_QTHRESHLD_CFG(base)			(base+0x00D4)
	
	#define QDMA_CSR_DBG_TX_CPU_DSCP_STAT(base)			(base+0x0100)
	#define QDMA_CSR_DBG_TX_CPU_PKT_STAT(base)			(base+0x0104)
	#define QDMA_CSR_DBG_TX_FWD_DSCP_STAT(base)			(base+0x0108)
	#define QDMA_CSR_DBG_TX_FWD_PKT_STAT(base)			(base+0x010C)
	#define QDMA_CSR_DBG_RX_CPU_PKT_STAT(base)			(base+0x0110)
	#define QDMA_CSR_DBG_RX_FWD_PKT_STAT(base)			(base+0x0114)
	#define QDMA_CSR_DBG_LMGR_STA(base)					(base+0x0118)
	#define QDMA_CSR_DBG_QDMA_STATUS(base)				(base+0x011C)
	#define QDMA_CSR_DBG_RCDROP_FWD_GREEN_STAT(base)	(base+0x0120)
	#define QDMA_CSR_DBG_RCDROP_FWD_YELLOW_STAT(base)	(base+0x0124)
	#define QDMA_CSR_DBG_RCDROP_FWD_RED_STAT(base)		(base+0x0128)
	#define QDMA_CSR_DBG_RCDROP_CPU_GREEN_STAT(base)	(base+0x012C)
	#define QDMA_CSR_DBG_RCDROP_CPU_YELLOW_STAT(base)	(base+0x0130)
	#define QDMA_CSR_DBG_RCDROP_CPU_RED_STAT(base)		(base+0x0134)
	#define QDMA_CSR_DBG_RETDROP_STAT(base)				(base+0x0138)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

/*******************************************************************************
*******************************************************************************/
/* QDMA_CSR_INFO(base) */

/* QDMA_CSR_GLB_CFG(base) */
#define GLB_CFG_RX_2B_OFFSET						(1<<31)
#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	#define GLB_CFG_MSG_LEN_SHIFT						(29)
	#define GLB_CFG_MSG_LEN_MASK						(0x3<<GLB_CFG_MSG_LEN_SHIFT)
	#define VAL_MSG_LEN_2_WORD							(0x00)
	#define VAL_MSG_LEN_4_WORD							(0x01)
	#define VAL_MSG_LEN_6_WORD							(0x10)
	#define VAL_MSG_LEN_8_WORD							(0x11)
#else
	#define GLB_CFG_TX_DSCP_PREFERENCE_SHIFT			(29)
	#define GLB_CFG_TX_DSCP_PREFERENCE_MASK				(0x3<<GLB_CFG_TX_DSCP_PREFERENCE_SHIFT)
	#define PREFER_ROIND_ROBIN							(0x00)
	#define PREFER_CPU_DSCP_CHAIN						(0x01)
	#define PREFER_FWD_DSCP_CHAIN						(0x11)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
#define GLB_CFG_DSCP_LEN_32							(1<<28)
#define GLB_CFG_DSCP_BYTE_SWAP						(1<<27)
#define GLB_CFG_PAYLOAD_BYTE_SWAP					(1<<26)
#define GLB_CFG_DSCPMSG_BURST						(1<<24)
#define GLB_CFG_IRQ_EN								(1<<19)
#define GLB_CFG_HW_FWD_EN							(1<<18)
#define GLB_CFG_UMAC_LOOPBACK						(1<<17)
#define GLB_CFG_QDMA_LOOPBACK						(1<<16)
#define GLB_CFG_CHECK_DONE							(1<<7)
#define GLB_CFG_TX_WB_DONE							(1<<6)
#define GLB_CFG_BST_SE_SHIFT						(4)
#define GLB_CFG_BST_SE_MASK							(0x3<<GLB_CFG_BST_SE_SHIFT)
#define VAL_BST_4_DWORD								(0x0)
#define VAL_BST_8_DWORD								(0x1)
#define VAL_BST_16_DWARD							(0x2)
#define VAL_BST_32_DWARD							(0x3)
#define GLB_CFG_RX_DMA_BUSY							(1<<3)
#define GLB_CFG_RX_DMA_EN							(1<<2)
#define GLB_CFG_TX_DMA_BUSY							(1<<1)
#define GLB_CFG_TX_DMA_EN							(1<<0)
                                                	
/* QDMA_CSR_TX_DSCP_BASE(base) */

/* QDMA_CSR_RX_DSCP_BASE(base) */

/* QDMA_CSR_TX_CPU_IDX(base) */
#define TX_CPU_IDX_SHIFT							(0)
#define TX_CPU_IDX_MASK								(0xFFF<<TX_CPU_IDX_SHIFT)

/* QDMA_CSR_TX_DMA_IDX(base) */
#define TX_DMA_IDX_SHIFT							(0)
#define TX_DMA_IDX_MASK								(0xFFF<<TX_DMA_IDX_SHIFT)

/* QDMA_CSR_RX_CPU_IDX(base) */
#define RX_CPU_IDX_SHIFT							(0)
#define RX_CPU_IDX_MASK								(0xFFF<<RX_CPU_IDX_SHIFT)

/* QDMA_CSR_RX_DMA_IDX(base) */
#define RX_DMA_IDX_SHIFT							(0)
#define RX_DMA_IDX_MASK								(0xFFF<<RX_DMA_IDX_SHIFT)

/* QDMA_CSR_HWFWD_DSCP_BASE(base) */

/* QDMA_CSR_HWFWD_AVAIL_DSCP_NUM(base) */
#define HWFWD_AVAIL_DSCP_NUM_SHIFT					(0)
#define HWFWD_AVAIL_DSCP_NUM_MASK					(0xFFF<<HWFWD_AVAIL_DSCP_NUM_SHIFT)

/* QDMA_CSR_HWFWD_USED_DSCP_NUM(base) */
#define HWFWD_RXDSCP_USAGE_SHIFT					(16)
#define HWFWD_RXDSCP_USAGE_MASK						(0xFFF<<HWFWD_RXDSCP_USAGE_SHIFT)
#define HWFWD_TXDSCP_USAGE_SHIFT					(0)
#define HWFWD_TXDSCP_USAGE_MASK						(0xFFF<<HWFWD_TXDSCP_USAGE_SHIFT)

/* QDMA_CSR_HWFWD_TX_IDX(base) */
#define HWFWD_TX_IDX_SHIFT							(0)
#define HWFWD_TX_IDX_MASK							(0xFFF<<HWFWD_TX_IDX_SHIFT)

/* QDMA_CSR_HWFWD_RX_IDX(base) */
#define HWFWD_RX_IDX_SHIFT							(0)
#define HWFWD_RX_IDX_MASK							(0xFFF<<HWFWD_RX_IDX_SHIFT)

/* QDMA_CSR_HWFWD_FREE_IDX(base) */
#define HWFWD_FREE_IDX_SHIFT						(0)
#define HWFWD_FREE_IDX_MASK							(0xFFF<<HWFWD_FREE_IDX_SHIFT)

#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
	/* QDMA_CSR_LMGR_CHNL_RETIRE(base) */
	#define LMGR_CHNL_RETIRE_CMD						(1<<31)
	#define LMGR_CHNL_RETIRE_DONE						(1<<30)
	#define LMGR_CHNL_RETIRE_CHN_SHIFT					(0)
	#define LMGR_CHNL_RETIRE_CHN_MASK					(0xF<<LMGR_CHNL_RETIRE_CHN_SHIFT)

	/* QDMA_CSR_LMGR_CHNL_RETIRE_MASK(base) */
	#define LMGR_CHNL_RETIRE_TX_MASK_SHIFT				(0)
	#define LMGR_CHNL_RETIRE_TX_MASK_MASK				(0xFFFF<<LMGR_CHNL_RETIRE_TX_MASK_SHIFT)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

/* QDMA_CSR_INT_STATUS(base) */
#define INT_STATUS_XPON_PHY							(1<<24)
#define INT_STATUS_ATM_SAR							(1<<19)
#define INT_STATUS_PTM								(1<<18)
#define INT_STATUS_EPON_MAC							(1<<17)
#define INT_STATUS_GPON_MAC							(1<<16)
#define INT_STATUS_RX_COHERENT						(1<<13)
#define INT_STATUS_TX_COHERENT						(1<<12)
#define INT_STATUS_IRQ_FULL							(1<<9)
#define INT_STATUS_NO_LINK_DSCP						(1<<8)
#define INT_STATUS_RX_BUF_OVRN						(1<<7)
#define INT_STATUS_TX_BUF_UDRN						(1<<6)
#define INT_STATUS_NO_RX_BUF						(1<<5)
#define INT_STATUS_NO_TX_BUF						(1<<4)
#define INT_STATUS_NO_RX_CPU_DSCP					(1<<3)
#define INT_STATUS_NO_TX_CPU_DSCP					(1<<2)
#define INT_STATUS_RX_DONE							(1<<1)
#define INT_STATUS_TX_DONE							(1<<0)
#define INT_STATUS_QDMA_DONE						(0x00000003)
#define INT_STATUS_QDMA_FAULT						(0x000033FC)
#define INT_STATUS_EXTERNAL							(0x030F0000)


/* QDMA_CSR_INT_ENABLE(base) */
#define INT_MASK_XPON_PHY							(1<<24)
#define INT_MASK_ATM_SAR							(1<<19)
#define INT_MASK_PTM								(1<<18)
#define INT_MASK_EPON_MAC							(1<<17)
#define INT_MASK_GPON_MAC							(1<<16)
#define INT_MASK_RX_COHERENT						(1<<13)
#define INT_MASK_TX_COHERENT						(1<<12)
#define INT_MASK_IRQ_FULL							(1<<9)
#define INT_MASK_NO_LINK_DSCP						(1<<8)
#define INT_MASK_RX_BUF_OVRN						(1<<7)
#define INT_MASK_TX_BUF_UDRN						(1<<6)
#define INT_MASK_NO_RX_BUF							(1<<5)
#define INT_MASK_NO_TX_BUF							(1<<4)
#define INT_MASK_NO_RX_CPU_DSCP						(1<<3)
#define INT_MASK_NO_TX_CPU_DSCP						(1<<2)
#define INT_MASK_RX_DONE							(1<<1)
#define INT_MASK_TX_DONE							(1<<0)

/* QDMA_CSR_DELAY_INT_CFG(base) */
#define DLY_INT_TXDLY_INT_EN						(1<<31)
#define DLY_INT_TXMAX_PINT_SHIFT					(24)
#define DLY_INT_TXMAX_PINT_MASK						(0x7F<<DLY_INT_TXMAX_PINT_SHIFT)
#define DLY_INT_TXMAX_PTIME_SHIFT					(16)
#define DLY_INT_TXMAX_PTIME_MASK					(0xFF<<DLY_INT_TXMAX_PTIME_SHIFT)
#define DLY_INT_RXDLY_INT_EN						(1<<15)
#define DLY_INT_RXMAX_PINT_SHIFT					(8)
#define DLY_INT_RXMAX_PINT_MASK						(0x7F<<DLY_INT_RXMAX_PINT_SHIFT)
#define DLY_INT_RXMAX_PTIME_SHIFT					(0)
#define DLY_INT_RXMAX_PTIME_MASK					(0xFF<<DLY_INT_RXMAX_PTIME_SHIFT)

/* QDMA_CSR_IRQ_BASE(base) */

/* QDMA_CSR_IRQ_CFG(base) */
#define IRQ_CFG_THRESHOLD_SHIFT						(16)
#define IRQ_CFG_THRESHOLD_MASK						(0xFFF<<IRQ_CFG_THRESHOLD_SHIFT)
#define IRQ_CFG_DEPTH_SHIFT							(0)
#define IRQ_CFG_DEPTH_MASK							(0xFFF<<IRQ_CFG_DEPTH_SHIFT)

#define IRQ_CFG_IDX_MASK							0xFFF

/* QDMA_CSR_IRQ_CLEAR_LEN(base) */
#define IRQ_CLEAR_LEN_SHIFT							(0)
#define IRQ_CLEAR_LEN_MASK							(0xFF<<IRQ_CLEAR_LEN_SHIFT)

/* QDMA_CSR_IRQ_STATUS(base) */
#define IRQ_STATUS_ENTRY_LEN_SHIFT					(16)
#define IRQ_STATUS_ENTRY_LEN_MASK					(0xFFF<<IRQ_STATUS_ENTRY_LEN_SHIFT)
#define IRQ_STATUS_HEAD_IDX_SHIFT					(0)
#define IRQ_STATUS_HEAD_IDX_MASK					(0xFFF<<IRQ_STATUS_HEAD_IDX_SHIFT)
                                               	
/* QDMA_CSR_IRQ_PTIME(base) */
#define IRQ_PTIME_SHIFT								(0)
#define IRQ_PTIME_MASK								(0xFFFF<<IRQ_PTIME_SHIFT)

/* QDMA_CSR_TXQOS_CHN07_CFG	 */
#define TXQOS_WRR_ONLY								(0)
#define TXQOS_SP_ONLY								(1)
#define TXQOS_SP_WRR_7								(2)
#define TXQOS_SP_WRR_6								(3)
#define TXQOS_SP_WRR_5								(4)
#define TXQOS_SP_WRR_4								(5)
#define TXQOS_SP_WRR_3								(6)
#define TXQOS_SP_WRR_2								(7)
#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
	#define TXQOS_WEIGHT_SCALE_16B						(1<<31)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
#define TXQOS_CHN7_SCH_SHIFT						(28)
#define TXQOS_CHN7_SCH_MASK							(0x7<<TXQOS_CHN7_SCH_SHIFT)
#define TXQOS_CHN6_SCH_SHIFT						(24)
#define TXQOS_CHN6_SCH_MASK							(0x7<<TXQOS_CHN6_SCH_SHIFT)
#define TXQOS_CHN5_SCH_SHIFT						(20)
#define TXQOS_CHN5_SCH_MASK							(0x7<<TXQOS_CHN5_SCH_SHIFT)
#define TXQOS_CHN4_SCH_SHIFT						(16)
#define TXQOS_CHN4_SCH_MASK							(0x7<<TXQOS_CHN4_SCH_SHIFT)
#define TXQOS_CHN3_SCH_SHIFT						(12)
#define TXQOS_CHN3_SCH_MASK							(0x7<<TXQOS_CHN3_SCH_SHIFT)
#define TXQOS_CHN2_SCH_SHIFT						(8)
#define TXQOS_CHN2_SCH_MASK							(0x7<<TXQOS_CHN2_SCH_SHIFT)
#define TXQOS_CHN1_SCH_SHIFT						(4)
#define TXQOS_CHN1_SCH_MASK							(0x7<<TXQOS_CHN1_SCH_SHIFT)
#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
	#define TXQOS_WEIGHT_BASE_BY_BYTE					(1<<3)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
#define TXQOS_CHN0_SCH_SHIFT						(0)
#define TXQOS_CHN0_SCH_MASK							(0x7<<TXQOS_CHN0_SCH_SHIFT)

/* QDMA_CSR_TXQOS_CHN815_CFG */
#define TXQOS_CHN15_SCH_SHIFT						(28)
#define TXQOS_CHN15_SCH_MASK						(0x7<<TXQOS_CHN15_SCH_SHIFT)
#define TXQOS_CHN14_SCH_SHIFT						(24)
#define TXQOS_CHN14_SCH_MASK						(0x7<<TXQOS_CHN14_SCH_SHIFT)
#define TXQOS_CHN13_SCH_SHIFT						(20)
#define TXQOS_CHN13_SCH_MASK						(0x7<<TXQOS_CHN13_SCH_SHIFT)
#define TXQOS_CHN12_SCH_SHIFT						(16)
#define TXQOS_CHN12_SCH_MASK						(0x7<<TXQOS_CHN12_SCH_SHIFT)
#define TXQOS_CHN11_SCH_SHIFT						(12)
#define TXQOS_CHN11_SCH_MASK						(0x7<<TXQOS_CHN11_SCH_SHIFT)
#define TXQOS_CHN10_SCH_SHIFT						(8)
#define TXQOS_CHN10_SCH_MASK						(0x7<<TXQOS_CHN10_SCH_SHIFT)
#define TXQOS_CHN9_SCH_SHIFT						(4)
#define TXQOS_CHN9_SCH_MASK							(0x7<<TXQOS_CHN9_SCH_SHIFT)
#define TXQOS_CHN8_SCH_SHIFT						(0)
#define TXQOS_CHN8_SCH_MASK							(0x7<<TXQOS_CHN8_SCH_SHIFT)

/* QDMA_CSR_TXQOS_WRR_CFG */
#define TXQOS_WRR_RWCMD								(1<<31)
#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	#define TXQOS_WRR_DONE								(1<<24)
#else 
	#define TXQOS_WRR_DONE								(1<<30)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
#define TXQOS_WRR_VALUE_SHIFT						(8)
#define TXQOS_WRR_VALUE_MASK						(0xFF<<TXQOS_WRR_VALUE_SHIFT)
#define TXQOS_WRR_CHANNEL_SHIFT						(3)
#define TXQOS_WRR_CHANNEL_MASK						(0xF<<TXQOS_WRR_CHANNEL_SHIFT)
#define TXQOS_WRR_QUEUE_SHIFT						(0)
#define TXQOS_WRR_QUEUE_MASK						(0x7<<TXQOS_WRR_QUEUE_SHIFT)

#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
	/* QDMA_CSR_TXDMA_PREFETCH_CFG(base) */
	#define TXDMA_PREFECTH_EN						(1<<0)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	/* QDMA_CSR_TXDMA_SCH_CFG */
	#define TXDMA_REPORT_GPON							(0x01)
	#define TXDMA_REPORT_EPON							(0x10)
	#define TXDMA_REPORT_SHIFT							(2)
	#define TXDMA_REPORT_MASK							(0x3<<TXDMA_REPORT_SHIFT)
	#define TXDMA_SCHEDULING_GPON						(0x01)
	#define TXDMA_SCHEDULING_EPON						(0x10)
	#define TXDMA_SCHEDULING_SHIFT						(0)
	#define TXDMA_SCHEDULING_MASK						(0x3<<TXDMA_SCHEDULING_SHIFT)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

/* QDMA_CSR_TXBUF_THR_CFG */
#define TXBUF_CTRL_EN								(1<<31)
#define TXBUF_CHN_THRSHLD_SHIFT						(8)
#define TXBUF_CHN_THRSHLD_MASK						(0xFF<<TXBUF_CHN_THRSHLD_SHIFT)
#define TXBUF_TOTAL_THRSHLD_SHIFT					(0)
#define TXBUF_TOTAL_THRSHLD_MASK					(0xFF<<TXBUF_TOTAL_THRSHLD_SHIFT)

#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	/* QDMA_CSR_GPON_RPT_CFG(base) */
	#define GPON_REPORT_EN								(1<<4)
	#define GPON_REPORT_UNIT_SHIFT						(0)
	#define GPON_REPORT_UNIT_MASK						(0xF<<GPON_REPORT_UNIT_SHIFT)

	/* QDMA_CSR_GPON_TRTCM_CFG */
	#define GPON_TRTCM_RWCMD							(1<<31)
	#define GPON_TRTCM_DONE								(1<<24)
	#define GPON_TRTCM_VALUE_SHIFT						(8)
	#define GPON_TRTCM_VALUE_MASK						(0xFFFF<<GPON_TRTCM_VALUE_SHIFT)
	#define GPON_TRTCM_CHN_SHIFT						(2)
	#define GPON_TRTCM_CHN_MASK							(0xF<<GPON_TRTCM_CHN_SHIFT)
	#define GPON_TRTCM_TYPE_SHIFT						(0)
	#define GPON_TRTCM_TYPE_MASK						(0x3<<GPON_TRTCM_TYPE_SHIFT)
#else
	/* QDMA_CSR_TXQ_CNGST_CFG(base) */
	#define TXQ_TRTCM_EN								(1<<31)
	#define TXQ_DEI_EN									(1<<30)
	#define TXQ_THRESHOLD_EN							(1<<29)
	#define TXQ_MAX_THRSHLD_SCALE_SHIFT					(26)
	#define TXQ_MAX_THRSHLD_SCALE_MASK					(0x3<<TXQ_MAX_THRSHLD_SCALE_SHIFT)
	#define TXQ_MIN_THRSHLD_SCALE_SHIFT					(24)
	#define TXQ_MIN_THRSHLD_SCALE_MASK					(0x3<<TXQ_MIN_THRSHLD_SCALE_SHIFT)
//	#define TXQ_TRAFFIC_SHAPING_SCALE_SHIFT				(16)
//	#define TXQ_TRAFFIC_SHAPING_SCALE_MASK				(0xF<<TXQ_TRAFFIC_SHAPING_SCALE_SHIFT)
	#define TXQ_YLW_DROP_PROBABILITY_SHIFT				(8)
	#define TXQ_YLW_DROP_PROBABILITY_MASK				(0xFF<<TXQ_YLW_DROP_PROBABILITY_SHIFT)
	#define TXQ_GRN_DROP_PROBABILITY_SHIFT				(0)
	#define TXQ_GRN_DROP_PROBABILITY_MASK				(0xFF<<TXQ_GRN_DROP_PROBABILITY_SHIFT)
	
	/* QDMA_CSR_TXQ_GRN_MAX_THRSHLD(base) */
	#define TXQ7_GRN_MAX_THRESHOLD_SHIFT				(28)
	#define TXQ7_GRN_MAX_THRESHOLD_MASK					(0xF<<TXQ7_GRN_MAX_THRESHOLD_SHIFT)
	#define TXQ6_GRN_MAX_THRESHOLD_SHIFT				(24)
	#define TXQ6_GRN_MAX_THRESHOLD_MASK					(0xF<<TXQ6_GRN_MAX_THRESHOLD_SHIFT)
	#define TXQ5_GRN_MAX_THRESHOLD_SHIFT				(20)
	#define TXQ5_GRN_MAX_THRESHOLD_MASK					(0xF<<TXQ5_GRN_MAX_THRESHOLD_SHIFT)
	#define TXQ4_GRN_MAX_THRESHOLD_SHIFT				(16)
	#define TXQ4_GRN_MAX_THRESHOLD_MASK					(0xF<<TXQ4_GRN_MAX_THRESHOLD_SHIFT)
	#define TXQ3_GRN_MAX_THRESHOLD_SHIFT				(12)
	#define TXQ3_GRN_MAX_THRESHOLD_MASK					(0xF<<TXQ3_GRN_MAX_THRESHOLD_SHIFT)
	#define TXQ2_GRN_MAX_THRESHOLD_SHIFT				(8)
	#define TXQ2_GRN_MAX_THRESHOLD_MASK					(0xF<<TXQ2_GRN_MAX_THRESHOLD_SHIFT)
	#define TXQ1_GRN_MAX_THRESHOLD_SHIFT				(4)
	#define TXQ1_GRN_MAX_THRESHOLD_MASK					(0xF<<TXQ1_GRN_MAX_THRESHOLD_SHIFT)
	#define TXQ0_GRN_MAX_THRESHOLD_SHIFT				(0)
	#define TXQ0_GRN_MAX_THRESHOLD_MASK					(0xF<<TXQ0_GRN_MAX_THRESHOLD_SHIFT)
	
	/* QDMA_CSR_TXQ_GRN_MIN_THRSHLD(base) */
	#define TXQ7_GRN_MIN_THRESHOLD_SHIFT				(28)
	#define TXQ7_GRN_MIN_THRESHOLD_MASK					(0xF<<TXQ7_GRN_MIN_THRESHOLD_SHIFT)
	#define TXQ6_GRN_MIN_THRESHOLD_SHIFT				(24)
	#define TXQ6_GRN_MIN_THRESHOLD_MASK					(0xF<<TXQ6_GRN_MIN_THRESHOLD_SHIFT)
	#define TXQ5_GRN_MIN_THRESHOLD_SHIFT				(20)
	#define TXQ5_GRN_MIN_THRESHOLD_MASK					(0xF<<TXQ5_GRN_MIN_THRESHOLD_SHIFT)
	#define TXQ4_GRN_MIN_THRESHOLD_SHIFT				(16)
	#define TXQ4_GRN_MIN_THRESHOLD_MASK					(0xF<<TXQ4_GRN_MIN_THRESHOLD_SHIFT)
	#define TXQ3_GRN_MIN_THRESHOLD_SHIFT				(12)
	#define TXQ3_GRN_MIN_THRESHOLD_MASK					(0xF<<TXQ3_GRN_MIN_THRESHOLD_SHIFT)
	#define TXQ2_GRN_MIN_THRESHOLD_SHIFT				(8)
	#define TXQ2_GRN_MIN_THRESHOLD_MASK					(0xF<<TXQ2_GRN_MIN_THRESHOLD_SHIFT)
	#define TXQ1_GRN_MIN_THRESHOLD_SHIFT				(4)
	#define TXQ1_GRN_MIN_THRESHOLD_MASK					(0xF<<TXQ1_GRN_MIN_THRESHOLD_SHIFT)
	#define TXQ0_GRN_MIN_THRESHOLD_SHIFT				(0)
	#define TXQ0_GRN_MIN_THRESHOLD_MASK					(0xF<<TXQ0_GRN_MIN_THRESHOLD_SHIFT)
	
	/* QDMA_CSR_TXQ_YLW_MAX_THRSHLD(base) */
	#define TXQ7_YLW_MAX_THRESHOLD_SHIFT				(28)
	#define TXQ7_YLW_MAX_THRESHOLD_MASK					(0xF<<TXQ7_YLW_MAX_THRESHOLD_SHIFT)
	#define TXQ6_YLW_MAX_THRESHOLD_SHIFT				(24)
	#define TXQ6_YLW_MAX_THRESHOLD_MASK					(0xF<<TXQ6_YLW_MAX_THRESHOLD_SHIFT)
	#define TXQ5_YLW_MAX_THRESHOLD_SHIFT				(20)
	#define TXQ5_YLW_MAX_THRESHOLD_MASK					(0xF<<TXQ5_YLW_MAX_THRESHOLD_SHIFT)
	#define TXQ4_YLW_MAX_THRESHOLD_SHIFT				(16)
	#define TXQ4_YLW_MAX_THRESHOLD_MASK					(0xF<<TXQ4_YLW_MAX_THRESHOLD_SHIFT)
	#define TXQ3_YLW_MAX_THRESHOLD_SHIFT				(12)
	#define TXQ3_YLW_MAX_THRESHOLD_MASK					(0xF<<TXQ3_YLW_MAX_THRESHOLD_SHIFT)
	#define TXQ2_YLW_MAX_THRESHOLD_SHIFT				(8)
	#define TXQ2_YLW_MAX_THRESHOLD_MASK					(0xF<<TXQ2_YLW_MAX_THRESHOLD_SHIFT)
	#define TXQ1_YLW_MAX_THRESHOLD_SHIFT				(4)
	#define TXQ1_YLW_MAX_THRESHOLD_MASK					(0xF<<TXQ1_YLW_MAX_THRESHOLD_SHIFT)
	#define TXQ0_YLW_MAX_THRESHOLD_SHIFT				(0)
	#define TXQ0_YLW_MAX_THRESHOLD_MASK					(0xF<<TXQ0_YLW_MAX_THRESHOLD_SHIFT)
	
	/* QDMA_CSR_TXQ_YLW_MIN_THRSHLD(base) */
	#define TXQ7_YLW_MIN_THRESHOLD_SHIFT				(28)
	#define TXQ7_YLW_MIN_THRESHOLD_MASK					(0xF<<TXQ7_YLW_MIN_THRESHOLD_SHIFT)
	#define TXQ6_YLW_MIN_THRESHOLD_SHIFT				(24)
	#define TXQ6_YLW_MIN_THRESHOLD_MASK					(0xF<<TXQ6_YLW_MIN_THRESHOLD_SHIFT)
	#define TXQ5_YLW_MIN_THRESHOLD_SHIFT				(20)
	#define TXQ5_YLW_MIN_THRESHOLD_MASK					(0xF<<TXQ5_YLW_MIN_THRESHOLD_SHIFT)
	#define TXQ4_YLW_MIN_THRESHOLD_SHIFT				(16)
	#define TXQ4_YLW_MIN_THRESHOLD_MASK					(0xF<<TXQ4_YLW_MIN_THRESHOLD_SHIFT)
	#define TXQ3_YLW_MIN_THRESHOLD_SHIFT				(12)
	#define TXQ3_YLW_MIN_THRESHOLD_MASK					(0xF<<TXQ3_YLW_MIN_THRESHOLD_SHIFT)
	#define TXQ2_YLW_MIN_THRESHOLD_SHIFT				(8)
	#define TXQ2_YLW_MIN_THRESHOLD_MASK					(0xF<<TXQ2_YLW_MIN_THRESHOLD_SHIFT)
	#define TXQ1_YLW_MIN_THRESHOLD_SHIFT				(4)
	#define TXQ1_YLW_MIN_THRESHOLD_MASK					(0xF<<TXQ1_YLW_MIN_THRESHOLD_SHIFT)
	#define TXQ0_YLW_MIN_THRESHOLD_SHIFT				(0)
	#define TXQ0_YLW_MIN_THRESHOLD_MASK					(0xF<<TXQ0_YLW_MIN_THRESHOLD_SHIFT)

	/* QDMA_CSR_TRTCM_CFG */
	#define TRTCM_RWCMD									(1<<31)
	#define TRTCM_DONE									(1<<30)
	#define TRTCM_SCALE_SHIFT							(24)
	#define TRTCM_SCALE_MASK							(0xF<<TRTCM_SCALE_SHIFT)
	#define TRTCM_VALUE_SHIFT							(8)
	#define TRTCM_VALUE_MASK							(0xFFFF<<TRTCM_VALUE_SHIFT)
	#define TRTCM_IDX_SHIFT								(2)
	#define TRTCM_IDX_MASK								(0x1F<<TRTCM_IDX_SHIFT)
	#define TRTCM_TYPE_SHIFT							(0)
	#define TRTCM_TYPE_MASK								(0x3<<TRTCM_TYPE_SHIFT)

	/* QDMA_CSR_GPON_TCONT_COLOR_CFG(base) */
	#define GPON_TCONT_COLOR_ENABLE						(1<<31)
//	#define GPON_TRTCM_PARA_SCALE_SHIFT					(24)
//	#define GPON_TRTCM_PARA_SCALE_MASK					(0xF<<GPON_TRTCM_PARA_SCALE_SHIFT)

	/* QDMA_CSR_GPON_TCONT_TRTCM_CFG(base) */
	#define GPON_TRTCM_RWCMD							(1<<31)
	#define GPON_TRTCM_DONE								(1<<30)
	#define GPON_TRTCM_SCALE_SHIFT						(24)
	#define GPON_TRTCM_SCALE_MASK						(0xF<<GPON_TRTCM_SCALE_SHIFT)
	#define GPON_TRTCM_VALUE_SHIFT						(8)
	#define GPON_TRTCM_VALUE_MASK						(0xFFFF<<GPON_TRTCM_VALUE_SHIFT)
	#define GPON_TRTCM_CHANNEL_SHIFT					(2)
	#define GPON_TRTCM_CHANNEL_MASK						(0xF<<GPON_TRTCM_CHANNEL_SHIFT)
	#define GPON_TRTCM_TYPE_SHIFT						(0)
	#define GPON_TRTCM_TYPE_MASK						(0x3<<GPON_TRTCM_TYPE_SHIFT)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

/* QDMA_CSR_EPON_RPT_CFG */
#define EPON_RPT_ONE_THRESHOLD						(0)
#define EPON_RPT_TWO_THRESHOLD						(1)
#define EPON_RPT_THREE_THRESHOLD					(2)
#define EPON_RPT_CH7_SHIFT							(14)
#define EPON_RPT_CH7_MASK							(0x3<<EPON_RPT_CH7_SHIFT)
#define EPON_RPT_CH6_SHIFT							(12)
#define EPON_RPT_CH6_MASK							(0x3<<EPON_RPT_CH6_SHIFT)
#define EPON_RPT_CH5_SHIFT							(10)
#define EPON_RPT_CH5_MASK							(0x3<<EPON_RPT_CH5_SHIFT)
#define EPON_RPT_CH4_SHIFT							(8)
#define EPON_RPT_CH4_MASK							(0x3<<EPON_RPT_CH4_SHIFT)
#define EPON_RPT_CH3_SHIFT							(6)
#define EPON_RPT_CH3_MASK							(0x3<<EPON_RPT_CH3_SHIFT)
#define EPON_RPT_CH2_SHIFT							(4)
#define EPON_RPT_CH2_MASK							(0x3<<EPON_RPT_CH2_SHIFT)
#define EPON_RPT_CH1_SHIFT							(2)
#define EPON_RPT_CH1_MASK							(0x3<<EPON_RPT_CH1_SHIFT)
#define EPON_RPT_CH0_SHIFT							(0)
#define EPON_RPT_CH0_MASK							(0x3<<EPON_RPT_CH0_SHIFT)


/* QDMA_CSR_EPON_QTHRESHLD_CFG */
#define EPON_QTHRESHLD_RWCMD						(1<<31)
#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	#define EPON_QTHRESHLD_DONE							(1<<24)
#else 
	#define EPON_QTHRESHLD_DONE							(1<<30)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
#define EPON_QTHRESHLD_VALUE_SHIFT					(8)
#define EPON_QTHRESHLD_VALUE_MASK					(0xFFFF<<EPON_QTHRESHLD_VALUE_SHIFT)
#define EPON_QTHRESHLD_TYPE_SHIFT					(6)
#define EPON_QTHRESHLD_TYPE_MASK					(0x3<<EPON_QTHRESHLD_TYPE_SHIFT)
#define EPON_QTHRESHLD_CHANNEL_SHIFT				(3)
#define EPON_QTHRESHLD_CHANNEL_MASK					(0x7<<EPON_QTHRESHLD_CHANNEL_SHIFT)
#define EPON_QTHRESHLD_QUEUE_SHIFT					(0)
#define EPON_QTHRESHLD_QUEUE_MASK					(0x7<<EPON_QTHRESHLD_QUEUE_SHIFT)

/* QDMA_CSR_DBG_LMGR_STA(base) */
#define DBG_LMGR_STA_LMGR_QVLD						(1<<31)
#define DBG_LMGR_STA_LMGR_FREE_CNT_SHIFT			(0)
#define DBG_LMGR_STA_LMGR_FREE_CNT_MASK				(0xFFF<<DBG_LMGR_STA_LMGR_FREE_CNT_SHIFT)


#endif /* _QDMA_REG_H_ */
