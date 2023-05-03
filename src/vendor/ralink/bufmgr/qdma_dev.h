#ifndef _QDMA_DEV_H_
#define _QDMA_DEV_H_

#include "qdma_reg.h"
#include "qdma_glb.h"


#define QDMA_DSCP_MSG_OFFSET		16
#define QDMA_DSCP_MSG_LENS			16

typedef struct {
	uint 			msg_addr ;
	struct {
#ifdef __BIG_ENDIAN
		uint done				: 1 ;
		uint drop_pkt			: 1 ;
		uint resv1				: 5 ;
		uint no_drop			: 1 ;
		uint msg_len			: 8 ;
		uint pkt_len			: 16 ;
#else
		uint pkt_len			: 16 ;
		uint msg_len			: 8 ;
		uint no_drop			: 1 ;
		uint resv1				: 5 ;
		uint drop_pkt			: 1 ;
		uint done				: 1 ;
#endif /* __BIG_ENDIAN */
	} ctrl ;
	uint 			pkt_addr ;
#ifdef __BIG_ENDIAN
	uint resv2					: 20 ;
	uint next_idx				: 12 ;
#else
	uint next_idx				: 12 ;
	uint resv2					: 20 ;
#endif /* __BIG_ENDIAN */
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	uint msg[4] ;
#else
#ifdef MT7510_DMA_DSCP_CACHE
	uint dummy[4];
#endif /* MT7510_DMA_DSCP_CACHE */
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
} QDMA_DMA_DSCP_T ;

#define qdmaGetQdmaInfo(base)			IO_GREG(QDMA_CSR_INFO(base))
#define qdmaSetGlbCfg(base, val)		IO_SREG(QDMA_CSR_GLB_CFG(base), val)
#define qdmaGetGlbCfg(base)				IO_GREG(QDMA_CSR_GLB_CFG(base))
#define qdmaIsSetRx2bOffset(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_RX_2B_OFFSET)
#define qdmaIsSetDscpByteSwap(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_DSCP_BYTE_SWAP)
#define qdmaIsSetPayloadByteSwap(base)	(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_PAYLOAD_BYTE_SWAP)	
#define qdmaIsSetIrqEnable(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_IRQ_EN)		
#define qdmaIsSetHwFwdEnable(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_HW_FWD_EN)	
#define qdmaIsSetCheckDone(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_CHECK_DONE)	
#define qdmaIsSetTxWbDone(base)			(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_TX_WB_DONE)	
#define qdmaIsRxDmaBusy(base)			(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_RX_DMA_BUSY)	
#define qdmaIsSetRxDmaEnable(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_RX_DMA_EN)	
#define qdmaIsTxDmaBusy(base)			(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_TX_DMA_BUSY)	
#define qdmaIsSetTxDmaEnable(base)		(IO_GREG(QDMA_CSR_GLB_CFG(base)) & GLB_CFG_TX_DMA_EN)
#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	#define qdmaSetRxMsgLens(base, val)		IO_SMASK(QDMA_CSR_GLB_CFG(base), GLB_CFG_MSG_LEN_MASK, GLB_CFG_MSG_LEN_SHIFT, val)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
#define qdmaEnableHwFwdDma(base)		IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_HW_FWD_EN)
#define qdmaDisableHwFwdDma(base)		IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_HW_FWD_EN)
#define qdmaEnableUmacLoopback(base)	IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_UMAC_LOOPBACK)
#define qdmaDisableUmacLoopback(base)	IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_UMAC_LOOPBACK)
#define qdmaEnableQdmaLoopback(base)	IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_QDMA_LOOPBACK)
#define qdmaDisableQdmaLoopback(base)	IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_QDMA_LOOPBACK)
#define qdmaEnableRxDma(base)			IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_RX_DMA_EN)
#define qdmaDisableRxDma(base)			IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_RX_DMA_EN)
#define qdmaEnableTxDma(base)			IO_SBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_TX_DMA_EN)
#define qdmaDisableTxDma(base)			IO_CBITS(QDMA_CSR_GLB_CFG(base), GLB_CFG_TX_DMA_EN)
#define qdmaSetBurstSize(base, val)		IO_SMASK(QDMA_CSR_GLB_CFG(base), GLB_CFG_BST_SE_MASK, GLB_CFG_BST_SE_SHIFT, val)
#define qdmaGetBurstSize(base)			IO_GMASK(QDMA_CSR_GLB_CFG(base), GLB_CFG_BST_SE_MASK, GLB_CFG_BST_SE_SHIFT)
#define qdmaSetTxDscpBase(base, val)	IO_SREG(QDMA_CSR_TX_DSCP_BASE(base), val)
#define qdmaGetTxDscpBase(base)			IO_GREG(QDMA_CSR_TX_DSCP_BASE(base))
#define qdmaSetRxDscpBase(base, val)	IO_SREG(QDMA_CSR_RX_DSCP_BASE(base), val)
#define qdmaGetRxDscpBase(base)			IO_GREG(QDMA_CSR_RX_DSCP_BASE(base))
#define qdmaSetTxCpuIdx(base, val)		IO_SMASK(QDMA_CSR_TX_CPU_IDX(base), TX_CPU_IDX_MASK, TX_CPU_IDX_SHIFT, val)
#define qdmaGetTxCpuIdx(base)			IO_GMASK(QDMA_CSR_TX_CPU_IDX(base), TX_CPU_IDX_MASK, TX_CPU_IDX_SHIFT)
#define qdmaSetTxDmaIdx(base, val)		IO_SMASK(QDMA_CSR_TX_DMA_IDX(base), TX_DMA_IDX_MASK, TX_DMA_IDX_SHIFT, val)
#define qdmaGetTxDmaIdx(base)			IO_GMASK(QDMA_CSR_TX_DMA_IDX(base), TX_DMA_IDX_MASK, TX_DMA_IDX_SHIFT)
#define qdmaSetRxCpuIdx(base, val)		IO_SMASK(QDMA_CSR_RX_CPU_IDX(base), RX_CPU_IDX_MASK, RX_CPU_IDX_SHIFT, val)
#define qdmaGetRxCpuIdx(base)			IO_GMASK(QDMA_CSR_RX_CPU_IDX(base), RX_CPU_IDX_MASK, RX_CPU_IDX_SHIFT)
#define qdmaSetRxDmaIdx(base, val)		IO_SMASK(QDMA_CSR_RX_DMA_IDX(base), RX_DMA_IDX_MASK, RX_DMA_IDX_SHIFT, val)
#define qdmaGetRxDmaIdx(base)			IO_GMASK(QDMA_CSR_RX_DMA_IDX(base), RX_DMA_IDX_MASK, RX_DMA_IDX_SHIFT)

#define qdmaSetHwDscpBase(base, val)	IO_SREG(QDMA_CSR_HWFWD_DSCP_BASE(base), val)
#define qdmaGetHwDscpBase(base)			IO_GREG(QDMA_CSR_HWFWD_DSCP_BASE(base))
#define qdmaSetHwDscpNum(base, val)		IO_SMASK(QDMA_CSR_HWFWD_AVAIL_DSCP_NUM(base), HWFWD_AVAIL_DSCP_NUM_MASK, HWFWD_AVAIL_DSCP_NUM_SHIFT, val)
#define qdmaGetHwDscpNum(base)			IO_GMASK(QDMA_CSR_HWFWD_AVAIL_DSCP_NUM(base), HWFWD_AVAIL_DSCP_NUM_MASK, HWFWD_AVAIL_DSCP_NUM_SHIFT)
#define qdmaGetHwRxDscpUsage(base)		IO_GMASK(QDMA_CSR_HWFWD_USED_DSCP_NUM(base), HWFWD_RXDSCP_USAGE_MASK, HWFWD_RXDSCP_USAGE_SHIFT)
#define qdmaGetHwTxDscpUsage(base)		IO_GMASK(QDMA_CSR_HWFWD_USED_DSCP_NUM(base), HWFWD_TXDSCP_USAGE_MASK, HWFWD_TXDSCP_USAGE_SHIFT)
#define qdmaSetHwTxIdx(base, val)		IO_SMASK(QDMA_CSR_HWFWD_TX_IDX(base), HWFWD_TX_IDX_MASK, HWFWD_TX_IDX_SHIFT, val)
#define qdmaGetHwTxIdx(base)			IO_GMASK(QDMA_CSR_HWFWD_TX_IDX(base), HWFWD_TX_IDX_MASK, HWFWD_TX_IDX_SHIFT)
#define qdmaSetHwRxIdx(base, val)		IO_SMASK(QDMA_CSR_HWFWD_RX_IDX(base), HWFWD_RX_IDX_MASK, HWFWD_RX_IDX_SHIFT, val)
#define qdmaGetHwRxIdx(base)			IO_GMASK(QDMA_CSR_HWFWD_RX_IDX(base), HWFWD_RX_IDX_MASK, HWFWD_RX_IDX_SHIFT)
#define qdmaSetHwFreeIdx(base, val)		IO_SMASK(QDMA_CSR_HWFWD_FREE_IDX(base), HWFWD_FREE_IDX_MASK, HWFWD_FREE_IDX_SHIFT, val)
#define qdmaGetHwFreeIdx(base)			IO_GMASK(QDMA_CSR_HWFWD_FREE_IDX(base), HWFWD_FREE_IDX_MASK, HWFWD_FREE_IDX_SHIFT)

#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
	#define qdmaSetTxChannelRetireMask(base, channel)	IO_SBITS(QDMA_CSR_LMGR_CHNL_RETIRE_MASK(base), (1<<(LMGR_CHNL_RETIRE_TX_MASK_SHIFT+channel)))
	#define qdmaClearTxChannelRetireMask(base, channel)	IO_CBITS(QDMA_CSR_LMGR_CHNL_RETIRE_MASK(base), (1<<(LMGR_CHNL_RETIRE_TX_MASK_SHIFT+channel)))
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

#define qdmaClearIntStatus(base, val)	IO_SREG(QDMA_CSR_INT_STATUS(base), val)
#define qdmaGetIntStatus(base)			IO_GREG(QDMA_CSR_INT_STATUS(base))
#define qdmaSetDelayIntCfg(base, val)	IO_SREG(QDMA_CSR_DELAY_INT_CFG(base), val)
#define qdmaGetDelayIntCfg(base)		IO_GREG(QDMA_CSR_DELAY_INT_CFG(base))
#define qdmaSetIrqBase(base, val)		IO_SREG(QDMA_CSR_IRQ_BASE(base), val)
#define qdmaGetIrqBase(base)			IO_GREG(QDMA_CSR_IRQ_BASE(base))
#define qdmaSetIrqConfig(base, val)		IO_SREG(QDMA_CSR_IRQ_CFG(base), val)
#define qdmaGetIrqConfig(base)			IO_GREG(QDMA_CSR_IRQ_CFG(base))
#define qdmaSetIrqThreshold(base, val)	IO_SMASK(QDMA_CSR_IRQ_CFG(base), IRQ_CFG_THRESHOLD_MASK, IRQ_CFG_THRESHOLD_SHIFT, val)
#define qdmaGetIrqThreshold(base)		IO_GMASK(QDMA_CSR_IRQ_CFG(base), IRQ_CFG_THRESHOLD_MASK, IRQ_CFG_THRESHOLD_SHIFT)
#define qdmaSetIrqDepth(base, val)		IO_SMASK(QDMA_CSR_IRQ_CFG(base), IRQ_CFG_DEPTH_MASK, IRQ_CFG_DEPTH_SHIFT, val)
#define qdmaGetIrqDepth(base)			IO_GMASK(QDMA_CSR_IRQ_CFG(base), IRQ_CFG_DEPTH_MASK, IRQ_CFG_DEPTH_SHIFT)
#define qdmaSetIrqClearLen(base, val)	IO_SMASK(QDMA_CSR_IRQ_CLEAR_LEN(base), IRQ_CLEAR_LEN_MASK, IRQ_CLEAR_LEN_SHIFT, val)
#define qdmaGetIrqClearLen(base)		IO_GMASK(QDMA_CSR_IRQ_CLEAR_LEN(base), IRQ_CLEAR_LEN_MASK, IRQ_CLEAR_LEN_SHIFT)
#define qdmaGetIrqStatus(base)			IO_GREG(QDMA_CSR_IRQ_STATUS(base))
#define qdmaGetIrqEntryLen(base)		IO_GMASK(QDMA_CSR_IRQ_STATUS(base), IRQ_STATUS_ENTRY_LEN_MASK, IRQ_STATUS_ENTRY_LEN_SHIFT)
#define qdmaGetIrqHeadIdx(base)			IO_GMASK(QDMA_CSR_IRQ_STATUS(base), IRQ_STATUS_HEAD_IDX_MASK, IRQ_STATUS_HEAD_IDX_SHIFT)
#define qdmaSetIrqPtime(base, val)		IO_SMASK(QDMA_CSR_IRQ_PTIME(base), IRQ_PTIME_MASK, IRQ_PTIME_SHIFT, val)
#define qdmaGetIrqPtime(base)			IO_GMASK(QDMA_CSR_IRQ_PTIME(base), IRQ_PTIME_MASK, IRQ_PTIME_SHIFT)

#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
	#define qdmaSetTxQosWeightByPacket(base)	IO_CBITS(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_WEIGHT_BASE_BY_BYTE)
	#define qdmaSetTxQosWeightByByte(base)		IO_SBITS(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_WEIGHT_BASE_BY_BYTE)
	#define qdmaIsTxQosWeightByByte(base)		(IO_GREG(QDMA_CSR_TXQOS_CHN07_CFG(base)) & TXQOS_WEIGHT_BASE_BY_BYTE)
	#define qdmaSetTxQosWeightScale64(base)		IO_CBITS(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_WEIGHT_SCALE_16B)
	#define qdmaSetTxQosWeightScale16(base)		IO_SBITS(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_WEIGHT_SCALE_16B)
	#define qdmaIsTxQosWeightScale16(base)		(IO_GREG(QDMA_CSR_TXQOS_CHN07_CFG(base)) & TXQOS_WEIGHT_SCALE_16B)

	#define qdmaEnableTxDmaPrefetch(base)		IO_SBITS(QDMA_CSR_TXDMA_PREFETCH_CFG(base), TXDMA_PREFECTH_EN)
	#define qdmaDisableTxDmaPrefetch(base)		IO_CBITS(QDMA_CSR_TXDMA_PREFETCH_CFG(base), TXDMA_PREFECTH_EN)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */


#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	#define qdmaGponSchedulingMode(base)	IO_SMASK(QDMA_CSR_TXDMA_SCH_CFG(base), TXDMA_SCHEDULING_MASK, TXDMA_SCHEDULING_SHIFT, TXDMA_SCHEDULING_GPON)
	#define qdmaEponSchedulingMode(base)	IO_SMASK(QDMA_CSR_TXDMA_SCH_CFG(base), TXDMA_SCHEDULING_MASK, TXDMA_SCHEDULING_SHIFT, TXDMA_SCHEDULING_EPON)
	#define qdmaGponReportMode(base)		IO_SMASK(QDMA_CSR_TXDMA_SCH_CFG(base), TXDMA_REPORT_MASK, TXDMA_REPORT_SHIFT, TXDMA_REPORT_GPON)
	#define qdmaEponReportMode(base)		IO_SMASK(QDMA_CSR_TXDMA_SCH_CFG(base), TXDMA_REPORT_MASK, TXDMA_REPORT_SHIFT, TXDMA_REPORT_EPON)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

#define qdmaEnableTxBufCtrl(base)		IO_SBITS(QDMA_CSR_TXBUF_THR_CFG(base), TXBUF_CTRL_EN)
#define qdmaDisableTxBufCtrl(base)		IO_CBITS(QDMA_CSR_TXBUF_THR_CFG(base), TXBUF_CTRL_EN)
#define qdmaTxBufChnnelThreshold(base, val)	IO_SMASK(QDMA_CSR_TXBUF_THR_CFG(base), TXBUF_CHN_THRSHLD_MASK, TXBUF_CHN_THRSHLD_SHIFT, val)
#define qdmaTxBufTotalThreshold(base, val)	IO_SMASK(QDMA_CSR_TXBUF_THR_CFG(base), TXBUF_TOTAL_THRSHLD_MASK, TXBUF_TOTAL_THRSHLD_SHIFT, val)

#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	#define qdmaEnableTrtcm(base)			IO_SBITS(QDMA_CSR_GPON_RPT_CFG(base), GPON_REPORT_EN)
	#define qdmaDisableTrtcm(base)			IO_CBITS(QDMA_CSR_GPON_RPT_CFG(base), GPON_REPORT_EN)
	#define qdmaSetTrtcmUnit(base, val)		IO_SMASK(QDMA_CSR_GPON_RPT_CFG(base), GPON_REPORT_UNIT_MASK, GPON_REPORT_UNIT_SHIFT, val)
	#define qdmaGetTrtcmUnit(base)			IO_GMASK(QDMA_CSR_GPON_RPT_CFG(base), GPON_REPORT_UNIT_MASK, GPON_REPORT_UNIT_SHIFT)
#else
	#define qdmaEnableTrtcmColorMode(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_TRTCM_EN)
	#define qdmaDisableTrtcmColorMode(base)		IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_TRTCM_EN)
	#define qdmaIsSetTrtcmColorMode(base)		(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & TXQ_TRTCM_EN)
	#define qdmaEnableDeiDropMode(base)			IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DEI_EN)
	#define qdmaDisableDeiDropMode(base)		IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_DEI_EN)
	#define qdmaIsSetDeiDropMode(base)			(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & TXQ_DEI_EN)
	#define qdmaEnableThresholdMode(base)		IO_SBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_THRESHOLD_EN)
	#define qdmaDisableThresholdMode(base)		IO_CBITS(QDMA_CSR_TXQ_CNGST_CFG(base), TXQ_THRESHOLD_EN)
	#define qdmaIsSetThresholdMode(base)		(IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) & TXQ_THRESHOLD_EN)

	#define qdmaEnableGponColorMode(base)		IO_SBITS(QDMA_CSR_GPON_TCONT_COLOR_CFG(base), GPON_TCONT_COLOR_ENABLE)
	#define qdmaDisableGponColorMode(base)		IO_CBITS(QDMA_CSR_GPON_TCONT_COLOR_CFG(base), GPON_TCONT_COLOR_ENABLE)
//	#define qdmaSetGponColorScale(base, val)	IO_SMASK(QDMA_CSR_GPON_TCONT_COLOR_CFG(base), GPON_TRTCM_PARA_SCALE_MASK, GPON_TRTCM_PARA_SCALE_SHIFT, val)
//	#define qdmaGetGponColorScale(base)			IO_GMASK(QDMA_CSR_GPON_TCONT_COLOR_CFG(base), GPON_TRTCM_PARA_SCALE_MASK, GPON_TRTCM_PARA_SCALE_SHIFT)
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	#define qdmaSetDbgTxDscpSummary(base, val)		IO_SREG(QDMA_CSR_DBG_TX_DSCP_SUM(base), val)
	#define qdmaGetDbgTxDscpSummary(base)			IO_GREG(QDMA_CSR_DBG_TX_DSCP_SUM(base))
	#define qdmaSetDbgTxPktSummary(base, val)		IO_SREG(QDMA_CSR_DBG_TX_PKT_SUM(base), val)
	#define qdmaGetDbgTxPktSummary(base)			IO_GREG(QDMA_CSR_DBG_TX_PKT_SUM(base))
	#define qdmaSetDbgRxPktSummary(base, val)		IO_SREG(QDMA_CSR_DBG_RX_PKT_SUM(base), val)
	#define qdmaGetDbgRxPktSummary(base)			IO_GREG(QDMA_CSR_DBG_RX_PKT_SUM(base))
#else
	#define qdmaSetDbgTxCpuDscp(base, val)			IO_SREG(QDMA_CSR_DBG_TX_CPU_DSCP_STAT(base), val)
	#define qdmaGetDbgTxCpuDscp(base)				IO_GREG(QDMA_CSR_DBG_TX_CPU_DSCP_STAT(base))
	#define qdmaSetDbgTxCpuPacket(base, val)		IO_SREG(QDMA_CSR_DBG_TX_CPU_PKT_STAT(base), val)
	#define qdmaGetDbgTxCpuPacket(base)				IO_GREG(QDMA_CSR_DBG_TX_CPU_PKT_STAT(base))
	#define qdmaSetDbgTxFwdDscp(base, val)			IO_SREG(QDMA_CSR_DBG_TX_FWD_DSCP_STAT(base), val)
	#define qdmaGetDbgTxFwdDscp(base)				IO_GREG(QDMA_CSR_DBG_TX_FWD_DSCP_STAT(base))
	#define qdmaSetDbgTxFwdPacket(base, val)		IO_SREG(QDMA_CSR_DBG_TX_FWD_PKT_STAT(base), val)
	#define qdmaGetDbgTxFwdPacket(base)				IO_GREG(QDMA_CSR_DBG_TX_FWD_PKT_STAT(base))
	#define qdmaSetDbgRxCpuPacket(base, val)		IO_SREG(QDMA_CSR_DBG_RX_CPU_PKT_STAT(base), val)
	#define qdmaGetDbgRxCpuPacket(base)				IO_GREG(QDMA_CSR_DBG_RX_CPU_PKT_STAT(base))
	#define qdmaSetDbgRxFwdPacket(base, val)		IO_SREG(QDMA_CSR_DBG_RX_FWD_PKT_STAT(base), val)
	#define qdmaGetDbgRxFwdPacket(base)				IO_GREG(QDMA_CSR_DBG_RX_FWD_PKT_STAT(base))
	#define qdmaSetDbgRcDropFwdGreen(base, val)		IO_SREG(QDMA_CSR_DBG_RCDROP_FWD_GREEN_STAT(base), val)
	#define qdmaGetDbgRcDropFwdGreen(base)			IO_GREG(QDMA_CSR_DBG_RCDROP_FWD_GREEN_STAT(base))
	#define qdmaSetDbgRcDropFwdYellow(base, val)	IO_SREG(QDMA_CSR_DBG_RCDROP_FWD_YELLOW_STAT(base), val)
	#define qdmaGetDbgRcDropFwdYellow(base)			IO_GREG(QDMA_CSR_DBG_RCDROP_FWD_YELLOW_STAT(base))
	#define qdmaSetDbgRcDropFwdRed(base, val)		IO_SREG(QDMA_CSR_DBG_RCDROP_FWD_RED_STAT(base), val)
	#define qdmaGetDbgRcDropFwdRed(base)			IO_GREG(QDMA_CSR_DBG_RCDROP_FWD_RED_STAT(base))
	#define qdmaSetDbgRcDropCpuGreen(base, val)		IO_SREG(QDMA_CSR_DBG_RCDROP_CPU_GREEN_STAT(base), val)
	#define qdmaGetDbgRcDropCpuGreen(base)			IO_GREG(QDMA_CSR_DBG_RCDROP_CPU_GREEN_STAT(base))
	#define qdmaSetDbgRcDropCpuYellow(base, val)	IO_SREG(QDMA_CSR_DBG_RCDROP_CPU_YELLOW_STAT(base), val)
	#define qdmaGetDbgRcDropCpuYellow(base)			IO_GREG(QDMA_CSR_DBG_RCDROP_CPU_YELLOW_STAT(base))
	#define qdmaSetDbgRcDropCpuRed(base, val)		IO_SREG(QDMA_CSR_DBG_RCDROP_CPU_RED_STAT(base), val)
	#define qdmaGetDbgRcDropCpuRed(base)			IO_GREG(QDMA_CSR_DBG_RCDROP_CPU_RED_STAT(base))
	#define qdmaSetDbgRetDrop(base, val)			IO_SREG(QDMA_CSR_DBG_RETDROP_STAT(base), val)
	#define qdmaGetDbgRetDrop(base)					IO_GREG(QDMA_CSR_DBG_RETDROP_STAT(base))
	#define qdmaGetDbgQdmaStatus(base)				IO_GREG(QDMA_CSR_DBG_QDMA_STATUS(base))
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
#define qdmaIsLmgrBusy(base)			(IO_GREG(QDMA_CSR_DBG_LMGR_STA(base)) & DBG_LMGR_STA_LMGR_QVLD)
#define qdmaLmgrFreeCount(base)			IO_GMASK(QDMA_CSR_DBG_LMGR_STA(base), DBG_LMGR_STA_LMGR_FREE_CNT_MASK, DBG_LMGR_STA_LMGR_FREE_CNT_SHIFT)

#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
	int __inline__ pseSetWanPcpConfig(unchar type, unchar mode) ;
	int __inline__ pseGetWanPcpConfig(unchar type) ;
	int __inline__ qdmaSetChannelRetire(unchar channel) ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
int __inline__ qdmaSetTxQosScheduler(unchar channel, unchar mode, unchar weight[8]) ;
int __inline__ qdmaGetTxQosScheduler(unchar channel, unchar *pMode, unchar weight[8]) ;

#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	int __inline__ qdmaSetTrtcm(unchar channel, unchar type, ushort value) ;
	int __inline__ qdmaGetTrtcm(unchar channel, unchar type) ;
#else
	int __inline__ qdmaSetTxQueueCngsScale(unchar maxScale, unchar minScale) ;
	int __inline__ qdmaGetTxQueueCngsScale(unchar *pMaxScale, unchar *pMinScale) ;
	int __inline__ qdmaSetTxQueueDropProbability(unchar grnDrop, unchar ylwDrop) ;
	int __inline__ qdmaGetTxQueueDropProbability(unchar *pGrnDrop, unchar *pYlwDrop) ;
	int __inline__ qdmaSetTxQueueGreenMaxThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) ;
	int __inline__ qdmaGetTxQueueGreenMaxThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) ;
	int __inline__ qdmaSetTxQueueGreenMinThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) ;
	int __inline__ qdmaGetTxQueueGreenMinThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) ;
	int __inline__ qdmaSetTxQueueYellowMaxThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) ;
	int __inline__ qdmaGetTxQueueYellowMaxThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) ;
	int __inline__ qdmaSetTxQueueYellowMinThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) ;
	int __inline__ qdmaGetTxQueueYellowMinThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) ;
	int __inline__ qdmaSetTxQueueTrtcmConfig(unchar idx, unchar type, unchar scale, ushort value) ;
	int __inline__ qdmaGetTxQueueTrtcmConfig(unchar idx, unchar scale, unchar type) ;
	int __inline__ qdmaSetGponTrtcmConfig(unchar channel, unchar type, unchar scale, ushort value) ;
	int __inline__ qdmaGetGponTrtcmConfig(unchar channel, unchar type) ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

int __inline__ qdmaSetEponReportConfig(unchar channel, unchar mode) ;
int __inline__ qdmaGetEponReportConfig(unchar channel, unchar *pMode) ;
int __inline__ qdmaSetEponThreshold(unchar channel, unchar queue, unchar type, ushort value) ;
int __inline__ qdmaGetEponThreshold(unchar channel, unchar queue, unchar type) ;

int qdmaEnableInt(uint base, uint bit) ;
int qdmaDisableInt(uint base, uint bit) ;
int qdmaSetIntMask(uint base, uint value) ;
int qdmaGetIntMask(uint base) ;

int qdma_dev_init(void) ;

#endif /* _QDMA_DEV_H_ */