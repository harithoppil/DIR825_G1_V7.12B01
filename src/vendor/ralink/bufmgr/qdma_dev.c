#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include "qdma_dev.h"
#include "qdma_bmgr.h"


#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
/******************************************************************************
******************************************************************************/
#define GDM2_CHNL_RELEASE_CMD						(1<<0)
#define GDM2_CHNL_RELEASE_DONE						(1<<1)
#define GDM2_CHNK_RELEASE_CHN_SHIFT					(4)
#define GDM2_CHNK_RELEASE_CHN_MASK					(0xF<<GDM2_CHNK_RELEASE_CHN_SHIFT)
static int __pseChannelRelease(unchar channel)
{
	uint regAddr = 0xBFB51528 ;
	uint chnRelease ;
	int RETRY = 3 ;
	
	chnRelease = (GDM2_CHNL_RELEASE_CMD | ((channel<<GDM2_CHNK_RELEASE_CHN_SHIFT)&GDM2_CHNK_RELEASE_CHN_MASK)) ;
	IO_SREG(regAddr, chnRelease) ;

	while(RETRY--) {
		chnRelease = IO_GREG(regAddr) ;
		QDMA_MSG(DEG_MSG, "Read PSE Channel Release: %.8x\n", chnRelease) ;
		if(chnRelease&GDM2_CHNL_RELEASE_DONE) {
			/* Clear the channel release */
			chnRelease = ((channel<<GDM2_CHNK_RELEASE_CHN_SHIFT)&GDM2_CHNK_RELEASE_CHN_MASK) ;
			IO_SREG(regAddr, chnRelease) ;
			QDMA_MSG(DEG_MSG, "Write PSE Channel Release: %.8x\n", chnRelease) ;
			break ;
		}
		mdelay(5) ;
	}
	if(RETRY < 0) {
		QDMA_ERR("Timeout for GDM2 channel release, channel:%d.\n", channel) ;
		return -ETIME ;
	}
	return 0 ;
}

/******************************************************************************
******************************************************************************/
#define CDM_TX_ENCODING_SHIFT					(0)
#define CDM_TX_ENCODING_MASK					(0xF<<CDM_TX_ENCODING_SHIFT)
#define CDM_RX_DECODING_SHIFT					(4)
#define CDM_RX_DECODING_MASK					(0xF<<CDM_RX_DECODING_SHIFT)
#define GDM_RX_DECODING_SHIFT					(8)
#define GDM_RX_DECODING_MASK					(0xF<<GDM_RX_DECODING_SHIFT)
int __inline__ pseSetWanPcpConfig(unchar type, unchar mode)
{
	uint regAddr = 0xBFB51514 ;
	
	if(type == 0) { /* CDM Tx Encoding */
		IO_SMASK(regAddr, CDM_TX_ENCODING_MASK, CDM_TX_ENCODING_SHIFT, mode) ;
	} else if(type == 1) { /* CDM Rx Decoding */
		IO_SMASK(regAddr, CDM_RX_DECODING_MASK, CDM_RX_DECODING_SHIFT, mode) ;
	} else if(type == 2) { /* GDM Rx Decoding */
		IO_SMASK(regAddr, GDM_RX_DECODING_MASK, GDM_RX_DECODING_SHIFT, mode) ;
	} 

	return 0 ;
}

int __inline__ pseGetWanPcpConfig(unchar type)
{
	uint regAddr = 0xBFB51514 ;
	uint pcpMode ;
	
	if(type == 0) { /* CDM Tx Encoding */
		pcpMode = IO_GMASK(regAddr, CDM_TX_ENCODING_MASK, CDM_TX_ENCODING_SHIFT) ;
	} else if(type == 1) { /* CDM Rx Decoding */
		pcpMode = IO_GMASK(regAddr, CDM_RX_DECODING_MASK, CDM_RX_DECODING_SHIFT) ;
	} else if(type == 2) { /* GDM Rx Decoding */
		pcpMode = IO_GMASK(regAddr, GDM_RX_DECODING_MASK, GDM_RX_DECODING_SHIFT) ;
	} 

	return pcpMode ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetChannelRetire(unchar channel)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint chnRetire ;
	int ret=0, RETRY = 3 ;
	int i ;
	
	qdmaSetTxChannelRetireMask(base, channel) ;
	
	/* Release the GDM2 channel */
	for(i=0 ; i<2 ; i++) {
		ret = __pseChannelRelease(channel) ;
		mdelay(10) ;
	}
	
	/* Release the QDMA channel */
	chnRetire = (LMGR_CHNL_RETIRE_CMD | ((channel<<LMGR_CHNL_RETIRE_CHN_SHIFT)&LMGR_CHNL_RETIRE_CHN_MASK)) ;
	IO_SREG(QDMA_CSR_LMGR_CHNL_RETIRE(base), chnRetire) ;
	while(RETRY--) {
		chnRetire = IO_GREG(QDMA_CSR_LMGR_CHNL_RETIRE(base)) ;
		QDMA_MSG(DEG_MSG, "Read QDMA Channel Release: %.8x\n", chnRetire) ;
		if(chnRetire&LMGR_CHNL_RETIRE_DONE) {			
			/* Clear the channel retire */
			chnRetire = ((channel<<LMGR_CHNL_RETIRE_CHN_SHIFT)&LMGR_CHNL_RETIRE_CHN_MASK) ;
			IO_SREG(QDMA_CSR_LMGR_CHNL_RETIRE(base), chnRetire) ;
			QDMA_MSG(DEG_MSG, "Set QDMA Channel Release: %.8x\n", chnRetire) ;
			break ;
		}
		mdelay(5) ;
	}
	if(RETRY < 0) {
		QDMA_ERR("Timeout for channel retire, channel:%d.\n", channel) ;
		ret = -ETIME ;
	}
	
	qdmaClearTxChannelRetireMask(base, channel) ;
	
	return ret ;
}

#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetTxQosScheduler(unchar channel, unchar mode, unchar weight[8])
{
	int i ;
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint wrrCfg = 0 ;
	int RETRY = 3 ;

	if(channel == 0) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN0_SCH_MASK, TXQOS_CHN0_SCH_SHIFT, mode) ;
	} else if(channel == 1) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN1_SCH_MASK, TXQOS_CHN1_SCH_SHIFT, mode) ;
	} else if(channel == 2) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN2_SCH_MASK, TXQOS_CHN2_SCH_SHIFT, mode) ;
	} else if(channel == 3) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN3_SCH_MASK, TXQOS_CHN3_SCH_SHIFT, mode) ;
	} else if(channel == 4) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN4_SCH_MASK, TXQOS_CHN4_SCH_SHIFT, mode) ;
	} else if(channel == 5) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN5_SCH_MASK, TXQOS_CHN5_SCH_SHIFT, mode) ;
	} else if(channel == 6) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN6_SCH_MASK, TXQOS_CHN6_SCH_SHIFT, mode) ;
	} else if(channel == 7) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN7_SCH_MASK, TXQOS_CHN7_SCH_SHIFT, mode) ;
	} else if(channel == 8) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN8_SCH_MASK, TXQOS_CHN8_SCH_SHIFT, mode) ;
	} else if(channel == 9) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN9_SCH_MASK, TXQOS_CHN9_SCH_SHIFT, mode) ;
	} else if(channel == 10) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN10_SCH_MASK, TXQOS_CHN10_SCH_SHIFT, mode) ;
	} else if(channel == 11) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN11_SCH_MASK, TXQOS_CHN11_SCH_SHIFT, mode) ;
	} else if(channel == 12) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN12_SCH_MASK, TXQOS_CHN12_SCH_SHIFT, mode) ;
	} else if(channel == 13) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN13_SCH_MASK, TXQOS_CHN13_SCH_SHIFT, mode) ;
	} else if(channel == 14) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN14_SCH_MASK, TXQOS_CHN14_SCH_SHIFT, mode) ;
	} else if(channel == 15) {
		IO_SMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN15_SCH_MASK, TXQOS_CHN15_SCH_SHIFT, mode) ;
	}
	
	for(i=0 ; i<8 ; i++) {
		if(weight[i] != 255) {
			wrrCfg = (TXQOS_WRR_RWCMD | 
					  ((weight[i]<<TXQOS_WRR_VALUE_SHIFT)&TXQOS_WRR_VALUE_MASK) |
					  ((channel<<TXQOS_WRR_CHANNEL_SHIFT)&TXQOS_WRR_CHANNEL_MASK) |
					  ((i<<TXQOS_WRR_QUEUE_SHIFT)&TXQOS_WRR_QUEUE_MASK)) ;
			IO_SREG(QDMA_CSR_TXQOS_WRR_CFG(base), wrrCfg) ;
		
			RETRY = 3 ;
			while(RETRY--) {
				wrrCfg = IO_GREG(QDMA_CSR_TXQOS_WRR_CFG(base)) ;
				
				if(wrrCfg&TXQOS_WRR_DONE) {
					break ;
				}
				mdelay(1) ;
			}
			if(RETRY < 0) {
				QDMA_ERR("Timeout for setting WRR configuration, channel:%d, queue:%d.\n", channel, i) ;
				return -ETIME ;
			}
		}
	}
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetTxQosScheduler(unchar channel, unchar *pMode, unchar weight[8])
{
	int i ;
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint wrrCfg = 0 ;
	int RETRY = 3 ;
	
	if(channel == 0) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN0_SCH_MASK, TXQOS_CHN0_SCH_SHIFT) ;
	} else if(channel == 1) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN1_SCH_MASK, TXQOS_CHN1_SCH_SHIFT) ;
	} else if(channel == 2) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN2_SCH_MASK, TXQOS_CHN2_SCH_SHIFT) ;
	} else if(channel == 3) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN3_SCH_MASK, TXQOS_CHN3_SCH_SHIFT) ;
	} else if(channel == 4) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN4_SCH_MASK, TXQOS_CHN4_SCH_SHIFT) ;
	} else if(channel == 5) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN5_SCH_MASK, TXQOS_CHN5_SCH_SHIFT) ;
	} else if(channel == 6) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN6_SCH_MASK, TXQOS_CHN6_SCH_SHIFT) ;
	} else if(channel == 7) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN07_CFG(base), TXQOS_CHN7_SCH_MASK, TXQOS_CHN7_SCH_SHIFT) ;
	} else if(channel == 8) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN8_SCH_MASK, TXQOS_CHN8_SCH_SHIFT) ;
	} else if(channel == 9) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN9_SCH_MASK, TXQOS_CHN9_SCH_SHIFT) ;
	} else if(channel == 10) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN10_SCH_MASK, TXQOS_CHN10_SCH_SHIFT) ;
	} else if(channel == 11) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN11_SCH_MASK, TXQOS_CHN11_SCH_SHIFT) ;
	} else if(channel == 12) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN12_SCH_MASK, TXQOS_CHN12_SCH_SHIFT) ;
	} else if(channel == 13) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN13_SCH_MASK, TXQOS_CHN13_SCH_SHIFT) ;
	} else if(channel == 14) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN14_SCH_MASK, TXQOS_CHN14_SCH_SHIFT) ;
	} else if(channel == 15) {
		*pMode = IO_GMASK(QDMA_CSR_TXQOS_CHN815_CFG(base), TXQOS_CHN15_SCH_MASK, TXQOS_CHN15_SCH_SHIFT) ;
	}

	for(i=0 ; i<8 ; i++) {
		wrrCfg = (((channel<<TXQOS_WRR_CHANNEL_SHIFT)&TXQOS_WRR_CHANNEL_MASK) |
				  ((i<<TXQOS_WRR_QUEUE_SHIFT)&TXQOS_WRR_QUEUE_MASK)) ;
		IO_SREG(QDMA_CSR_TXQOS_WRR_CFG(base), wrrCfg) ;
	
		RETRY = 3 ;
		while(RETRY--) {
			wrrCfg = IO_GREG(QDMA_CSR_TXQOS_WRR_CFG(base)) ;
			
			if(wrrCfg&TXQOS_WRR_DONE) {
				weight[i] =  ((wrrCfg&TXQOS_WRR_VALUE_MASK)>>TXQOS_WRR_VALUE_SHIFT) ;
				break ;
			} 
			mdelay(1) ;
		}
		if(RETRY < 0) {
			QDMA_ERR("Timeout for getting WRR configuration, channel:%d, queue:%d.\n", channel, i) ;
			return -ETIME ;
		}
	}
	return 0 ;
}

#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetTrtcm(unchar channel, unchar type, ushort value)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint trtcmCfg = 0 ;
	int RETRY = 3 ;
	
	trtcmCfg = (GPON_TRTCM_RWCMD | 
				((value<<GPON_TRTCM_VALUE_SHIFT)&GPON_TRTCM_VALUE_MASK) |
				((channel<<GPON_TRTCM_CHN_SHIFT)&GPON_TRTCM_CHN_MASK) |
				((type<<GPON_TRTCM_TYPE_SHIFT)&GPON_TRTCM_TYPE_MASK)) ;
	IO_SREG(QDMA_CSR_GPON_TRTCM_CFG(base), trtcmCfg) ;

	while(RETRY--) {
		trtcmCfg = IO_GREG(QDMA_CSR_GPON_TRTCM_CFG(base)) ;
		
		if(trtcmCfg&GPON_TRTCM_DONE) {
			return 0 ;
		}
		mdelay(1) ;
	}

	QDMA_ERR("Timeout for set trTCM configuration.\n") ;
	
	return -ETIME ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetTrtcm(unchar channel, unchar type)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint trtcmCfg = 0 ;
	int RETRY = 3 ;
	
	trtcmCfg = (((channel<<GPON_TRTCM_CHN_SHIFT)&GPON_TRTCM_CHN_MASK) |
				((type<<GPON_TRTCM_TYPE_SHIFT)&GPON_TRTCM_TYPE_MASK)) ;
	IO_SREG(QDMA_CSR_GPON_TRTCM_CFG(base), trtcmCfg) ;

	while(RETRY--) {
		trtcmCfg = IO_GREG(QDMA_CSR_GPON_TRTCM_CFG(base)) ;
		
		if(trtcmCfg&GPON_TRTCM_DONE) {
			return ((trtcmCfg&GPON_TRTCM_VALUE_MASK)>>GPON_TRTCM_VALUE_SHIFT) ;
		} 
		mdelay(1) ;
	}

	QDMA_ERR("Timeout for get trTCM configuration.\n") ;
	
	return -ETIME ;
}

#else

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetTxQueueCngsScale(unchar maxScale, unchar minScale) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint txqCngsCfg = 0 ;

	txqCngsCfg = IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) ;
	txqCngsCfg = (txqCngsCfg&~TXQ_MAX_THRSHLD_SCALE_MASK) | ((maxScale<<TXQ_MAX_THRSHLD_SCALE_SHIFT)&TXQ_MAX_THRSHLD_SCALE_MASK) ;
	txqCngsCfg = (txqCngsCfg&~TXQ_MIN_THRSHLD_SCALE_MASK) | ((minScale<<TXQ_MIN_THRSHLD_SCALE_SHIFT)&TXQ_MIN_THRSHLD_SCALE_MASK) ;
	
	IO_SREG(QDMA_CSR_TXQ_CNGST_CFG(base), txqCngsCfg) ;

	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetTxQueueCngsScale(unchar *pMaxScale, unchar *pMinScale) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint txqCngsCfg = 0 ;

	txqCngsCfg = IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) ;
	
	*pMaxScale = (txqCngsCfg & TXQ_MAX_THRSHLD_SCALE_MASK) >> TXQ_MAX_THRSHLD_SCALE_SHIFT ;
	*pMinScale = (txqCngsCfg & TXQ_MIN_THRSHLD_SCALE_MASK) >> TXQ_MIN_THRSHLD_SCALE_SHIFT ;

	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetTxQueueDropProbability(unchar grnDrop, unchar ylwDrop) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint txqCngsCfg = 0 ;

	txqCngsCfg = IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) ;
	txqCngsCfg = (txqCngsCfg&~TXQ_YLW_DROP_PROBABILITY_MASK) | ((ylwDrop<<TXQ_YLW_DROP_PROBABILITY_SHIFT)&TXQ_YLW_DROP_PROBABILITY_MASK) ;
	txqCngsCfg = (txqCngsCfg&~TXQ_GRN_DROP_PROBABILITY_MASK) | ((grnDrop<<TXQ_GRN_DROP_PROBABILITY_SHIFT)&TXQ_GRN_DROP_PROBABILITY_MASK) ;
	
	IO_SREG(QDMA_CSR_TXQ_CNGST_CFG(base), txqCngsCfg) ;

	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetTxQueueDropProbability(unchar *pGrnDrop, unchar *pYlwDrop) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint txqCngsCfg = 0 ;

	txqCngsCfg = IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base)) ;
	
	*pYlwDrop = (txqCngsCfg & TXQ_YLW_DROP_PROBABILITY_MASK) >> TXQ_YLW_DROP_PROBABILITY_SHIFT ;
	*pGrnDrop = (txqCngsCfg & TXQ_GRN_DROP_PROBABILITY_MASK) >> TXQ_GRN_DROP_PROBABILITY_SHIFT ;
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetTxQueueGreenMaxThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint grnMaxCfg = 0 ;
	
	grnMaxCfg = (((txqIdx[0]<<TXQ0_GRN_MAX_THRESHOLD_SHIFT)&TXQ0_GRN_MAX_THRESHOLD_MASK) |
				 ((txqIdx[1]<<TXQ1_GRN_MAX_THRESHOLD_SHIFT)&TXQ1_GRN_MAX_THRESHOLD_MASK) |
				 ((txqIdx[2]<<TXQ2_GRN_MAX_THRESHOLD_SHIFT)&TXQ2_GRN_MAX_THRESHOLD_MASK) |
				 ((txqIdx[3]<<TXQ3_GRN_MAX_THRESHOLD_SHIFT)&TXQ3_GRN_MAX_THRESHOLD_MASK) |
				 ((txqIdx[4]<<TXQ4_GRN_MAX_THRESHOLD_SHIFT)&TXQ4_GRN_MAX_THRESHOLD_MASK) |
				 ((txqIdx[5]<<TXQ5_GRN_MAX_THRESHOLD_SHIFT)&TXQ5_GRN_MAX_THRESHOLD_MASK) |
				 ((txqIdx[6]<<TXQ6_GRN_MAX_THRESHOLD_SHIFT)&TXQ6_GRN_MAX_THRESHOLD_MASK) |
				 ((txqIdx[7]<<TXQ7_GRN_MAX_THRESHOLD_SHIFT)&TXQ7_GRN_MAX_THRESHOLD_MASK)) ;
	
	IO_SREG(QDMA_CSR_TXQ_GRN_MAX_THRSHLD(base), grnMaxCfg) ;
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetTxQueueGreenMaxThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint grnMaxCfg = 0 ;
	
	grnMaxCfg = IO_GREG(QDMA_CSR_TXQ_GRN_MAX_THRSHLD(base)) ;
	txqIdx[0] = ((grnMaxCfg & TXQ0_GRN_MAX_THRESHOLD_MASK) >> TXQ0_GRN_MAX_THRESHOLD_SHIFT) ;
	txqIdx[1] = ((grnMaxCfg & TXQ1_GRN_MAX_THRESHOLD_MASK) >> TXQ1_GRN_MAX_THRESHOLD_SHIFT) ;
	txqIdx[2] = ((grnMaxCfg & TXQ2_GRN_MAX_THRESHOLD_MASK) >> TXQ2_GRN_MAX_THRESHOLD_SHIFT) ;
	txqIdx[3] = ((grnMaxCfg & TXQ3_GRN_MAX_THRESHOLD_MASK) >> TXQ3_GRN_MAX_THRESHOLD_SHIFT) ;
	txqIdx[4] = ((grnMaxCfg & TXQ4_GRN_MAX_THRESHOLD_MASK) >> TXQ4_GRN_MAX_THRESHOLD_SHIFT) ;
	txqIdx[5] = ((grnMaxCfg & TXQ5_GRN_MAX_THRESHOLD_MASK) >> TXQ5_GRN_MAX_THRESHOLD_SHIFT) ;
	txqIdx[6] = ((grnMaxCfg & TXQ6_GRN_MAX_THRESHOLD_MASK) >> TXQ6_GRN_MAX_THRESHOLD_SHIFT) ;
	txqIdx[7] = ((grnMaxCfg & TXQ7_GRN_MAX_THRESHOLD_MASK) >> TXQ7_GRN_MAX_THRESHOLD_SHIFT) ;
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetTxQueueGreenMinThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint grnMinCfg = 0 ;
	
	grnMinCfg = (((txqIdx[0]<<TXQ0_GRN_MIN_THRESHOLD_SHIFT)&TXQ0_GRN_MIN_THRESHOLD_MASK) |
				 ((txqIdx[1]<<TXQ1_GRN_MIN_THRESHOLD_SHIFT)&TXQ1_GRN_MIN_THRESHOLD_MASK) |
				 ((txqIdx[2]<<TXQ2_GRN_MIN_THRESHOLD_SHIFT)&TXQ2_GRN_MIN_THRESHOLD_MASK) |
				 ((txqIdx[3]<<TXQ3_GRN_MIN_THRESHOLD_SHIFT)&TXQ3_GRN_MIN_THRESHOLD_MASK) |
				 ((txqIdx[4]<<TXQ4_GRN_MIN_THRESHOLD_SHIFT)&TXQ4_GRN_MIN_THRESHOLD_MASK) |
				 ((txqIdx[5]<<TXQ5_GRN_MIN_THRESHOLD_SHIFT)&TXQ5_GRN_MIN_THRESHOLD_MASK) |
				 ((txqIdx[6]<<TXQ6_GRN_MIN_THRESHOLD_SHIFT)&TXQ6_GRN_MIN_THRESHOLD_MASK) |
				 ((txqIdx[7]<<TXQ7_GRN_MIN_THRESHOLD_SHIFT)&TXQ7_GRN_MIN_THRESHOLD_MASK)) ;
	
	IO_SREG(QDMA_CSR_TXQ_GRN_MIN_THRSHLD(base), grnMinCfg) ;
			
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetTxQueueGreenMinThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint grnMinCfg = 0 ;
	
	grnMinCfg = IO_GREG(QDMA_CSR_TXQ_GRN_MIN_THRSHLD(base)) ;
	txqIdx[0] = ((grnMinCfg & TXQ0_GRN_MIN_THRESHOLD_MASK) >> TXQ0_GRN_MIN_THRESHOLD_SHIFT) ;
	txqIdx[1] = ((grnMinCfg & TXQ1_GRN_MIN_THRESHOLD_MASK) >> TXQ1_GRN_MIN_THRESHOLD_SHIFT) ;
	txqIdx[2] = ((grnMinCfg & TXQ2_GRN_MIN_THRESHOLD_MASK) >> TXQ2_GRN_MIN_THRESHOLD_SHIFT) ;
	txqIdx[3] = ((grnMinCfg & TXQ3_GRN_MIN_THRESHOLD_MASK) >> TXQ3_GRN_MIN_THRESHOLD_SHIFT) ;
	txqIdx[4] = ((grnMinCfg & TXQ4_GRN_MIN_THRESHOLD_MASK) >> TXQ4_GRN_MIN_THRESHOLD_SHIFT) ;
	txqIdx[5] = ((grnMinCfg & TXQ5_GRN_MIN_THRESHOLD_MASK) >> TXQ5_GRN_MIN_THRESHOLD_SHIFT) ;
	txqIdx[6] = ((grnMinCfg & TXQ6_GRN_MIN_THRESHOLD_MASK) >> TXQ6_GRN_MIN_THRESHOLD_SHIFT) ;
	txqIdx[7] = ((grnMinCfg & TXQ7_GRN_MIN_THRESHOLD_MASK) >> TXQ7_GRN_MIN_THRESHOLD_SHIFT) ;
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetTxQueueYellowMaxThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint ylwMaxCfg = 0 ;
	
	ylwMaxCfg = (((txqIdx[0]<<TXQ0_YLW_MAX_THRESHOLD_SHIFT)&TXQ0_YLW_MAX_THRESHOLD_MASK) |
				 ((txqIdx[1]<<TXQ1_YLW_MAX_THRESHOLD_SHIFT)&TXQ1_YLW_MAX_THRESHOLD_MASK) |
				 ((txqIdx[2]<<TXQ2_YLW_MAX_THRESHOLD_SHIFT)&TXQ2_YLW_MAX_THRESHOLD_MASK) |
				 ((txqIdx[3]<<TXQ3_YLW_MAX_THRESHOLD_SHIFT)&TXQ3_YLW_MAX_THRESHOLD_MASK) |
				 ((txqIdx[4]<<TXQ4_YLW_MAX_THRESHOLD_SHIFT)&TXQ4_YLW_MAX_THRESHOLD_MASK) |
				 ((txqIdx[5]<<TXQ5_YLW_MAX_THRESHOLD_SHIFT)&TXQ5_YLW_MAX_THRESHOLD_MASK) |
				 ((txqIdx[6]<<TXQ6_YLW_MAX_THRESHOLD_SHIFT)&TXQ6_YLW_MAX_THRESHOLD_MASK) |
				 ((txqIdx[7]<<TXQ7_YLW_MAX_THRESHOLD_SHIFT)&TXQ7_YLW_MAX_THRESHOLD_MASK)) ;
	
	IO_SREG(QDMA_CSR_TXQ_YLW_MAX_THRSHLD(base), ylwMaxCfg) ;
		
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetTxQueueYellowMaxThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint ylwMaxCfg = 0 ;
	
	ylwMaxCfg = IO_GREG(QDMA_CSR_TXQ_YLW_MAX_THRSHLD(base)) ;
	txqIdx[0] = ((ylwMaxCfg & TXQ0_YLW_MAX_THRESHOLD_MASK) >> TXQ0_YLW_MAX_THRESHOLD_SHIFT) ;
	txqIdx[1] = ((ylwMaxCfg & TXQ1_YLW_MAX_THRESHOLD_MASK) >> TXQ1_YLW_MAX_THRESHOLD_SHIFT) ;
	txqIdx[2] = ((ylwMaxCfg & TXQ2_YLW_MAX_THRESHOLD_MASK) >> TXQ2_YLW_MAX_THRESHOLD_SHIFT) ;
	txqIdx[3] = ((ylwMaxCfg & TXQ3_YLW_MAX_THRESHOLD_MASK) >> TXQ3_YLW_MAX_THRESHOLD_SHIFT) ;
	txqIdx[4] = ((ylwMaxCfg & TXQ4_YLW_MAX_THRESHOLD_MASK) >> TXQ4_YLW_MAX_THRESHOLD_SHIFT) ;
	txqIdx[5] = ((ylwMaxCfg & TXQ5_YLW_MAX_THRESHOLD_MASK) >> TXQ5_YLW_MAX_THRESHOLD_SHIFT) ;
	txqIdx[6] = ((ylwMaxCfg & TXQ6_YLW_MAX_THRESHOLD_MASK) >> TXQ6_YLW_MAX_THRESHOLD_SHIFT) ;
	txqIdx[7] = ((ylwMaxCfg & TXQ7_YLW_MAX_THRESHOLD_MASK) >> TXQ7_YLW_MAX_THRESHOLD_SHIFT) ;
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetTxQueueYellowMinThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint ylwMinCfg = 0 ;
	
	ylwMinCfg = (((txqIdx[0]<<TXQ0_YLW_MIN_THRESHOLD_SHIFT)&TXQ0_YLW_MIN_THRESHOLD_MASK) |
				 ((txqIdx[1]<<TXQ1_YLW_MIN_THRESHOLD_SHIFT)&TXQ1_YLW_MIN_THRESHOLD_MASK) |
				 ((txqIdx[2]<<TXQ2_YLW_MIN_THRESHOLD_SHIFT)&TXQ2_YLW_MIN_THRESHOLD_MASK) |
				 ((txqIdx[3]<<TXQ3_YLW_MIN_THRESHOLD_SHIFT)&TXQ3_YLW_MIN_THRESHOLD_MASK) |
				 ((txqIdx[4]<<TXQ4_YLW_MIN_THRESHOLD_SHIFT)&TXQ4_YLW_MIN_THRESHOLD_MASK) |
				 ((txqIdx[5]<<TXQ5_YLW_MIN_THRESHOLD_SHIFT)&TXQ5_YLW_MIN_THRESHOLD_MASK) |
				 ((txqIdx[6]<<TXQ6_YLW_MIN_THRESHOLD_SHIFT)&TXQ6_YLW_MIN_THRESHOLD_MASK) |
				 ((txqIdx[7]<<TXQ7_YLW_MIN_THRESHOLD_SHIFT)&TXQ7_YLW_MIN_THRESHOLD_MASK)) ;
	
	IO_SREG(QDMA_CSR_TXQ_YLW_MIN_THRSHLD(base), ylwMinCfg) ;

	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetTxQueueYellowMinThreshold(unchar txqIdx[CONFIG_QDMA_QUEUE]) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint ylwMinCfg = 0 ;
	
	ylwMinCfg = IO_GREG(QDMA_CSR_TXQ_YLW_MIN_THRSHLD(base)) ;
	txqIdx[0] = ((ylwMinCfg & TXQ0_YLW_MIN_THRESHOLD_MASK) >> TXQ0_YLW_MIN_THRESHOLD_SHIFT) ;
	txqIdx[1] = ((ylwMinCfg & TXQ1_YLW_MIN_THRESHOLD_MASK) >> TXQ1_YLW_MIN_THRESHOLD_SHIFT) ;
	txqIdx[2] = ((ylwMinCfg & TXQ2_YLW_MIN_THRESHOLD_MASK) >> TXQ2_YLW_MIN_THRESHOLD_SHIFT) ;
	txqIdx[3] = ((ylwMinCfg & TXQ3_YLW_MIN_THRESHOLD_MASK) >> TXQ3_YLW_MIN_THRESHOLD_SHIFT) ;
	txqIdx[4] = ((ylwMinCfg & TXQ4_YLW_MIN_THRESHOLD_MASK) >> TXQ4_YLW_MIN_THRESHOLD_SHIFT) ;
	txqIdx[5] = ((ylwMinCfg & TXQ5_YLW_MIN_THRESHOLD_MASK) >> TXQ5_YLW_MIN_THRESHOLD_SHIFT) ;
	txqIdx[6] = ((ylwMinCfg & TXQ6_YLW_MIN_THRESHOLD_MASK) >> TXQ6_YLW_MIN_THRESHOLD_SHIFT) ;
	txqIdx[7] = ((ylwMinCfg & TXQ7_YLW_MIN_THRESHOLD_MASK) >> TXQ7_YLW_MIN_THRESHOLD_SHIFT) ;
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetTxQueueTrtcmConfig(unchar idx, unchar type, unchar scale, ushort value) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint trtcmCfg = 0 ;
	int RETRY = 3 ;
	
	trtcmCfg = (TRTCM_RWCMD | 
				((value<<TRTCM_VALUE_SHIFT)&TRTCM_VALUE_MASK) |
				((scale<<TRTCM_SCALE_SHIFT)&TRTCM_SCALE_MASK) |
				((idx<<TRTCM_IDX_SHIFT)&TRTCM_IDX_MASK) |
				((type<<TRTCM_TYPE_SHIFT)&TRTCM_TYPE_MASK)) ;
	IO_SREG(QDMA_CSR_TRTCM_CFG(base), trtcmCfg) ;

	while(RETRY--) {
		trtcmCfg = IO_GREG(QDMA_CSR_TRTCM_CFG(base)) ;
		
		if(trtcmCfg&TRTCM_DONE) {
			return 0 ;
		}
		mdelay(1) ;
	}

	QDMA_ERR("Timeout for set tx queue trTCM configuration.\n") ;
	
	return -ETIME ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetTxQueueTrtcmConfig(unchar idx, unchar scale, unchar type) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint trtcmCfg = 0 ;
	int RETRY = 3 ;
	
	trtcmCfg = (((idx<<TRTCM_IDX_SHIFT)&TRTCM_IDX_MASK) |
				((scale<<TRTCM_SCALE_SHIFT)&TRTCM_SCALE_MASK) |
				((type<<TRTCM_TYPE_SHIFT)&TRTCM_TYPE_MASK)) ;
	IO_SREG(QDMA_CSR_TRTCM_CFG(base), trtcmCfg) ;

	while(RETRY--) {
		trtcmCfg = IO_GREG(QDMA_CSR_TRTCM_CFG(base)) ;
		
		if(trtcmCfg&TRTCM_DONE) {
			return ((trtcmCfg&TRTCM_VALUE_MASK)>>TRTCM_VALUE_SHIFT) ;
		} 
		mdelay(1) ;
	}

	QDMA_ERR("Timeout for get tx queue trTCM configuration.\n") ;
	
	return -ETIME ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetGponTrtcmConfig(unchar channel, unchar type, unchar scale, ushort value) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint trtcmCfg = 0 ;
	int RETRY = 3 ;
	
	trtcmCfg = (GPON_TRTCM_RWCMD | 
				((value<<GPON_TRTCM_VALUE_SHIFT)&GPON_TRTCM_VALUE_MASK) |
				((scale<<GPON_TRTCM_SCALE_SHIFT)&GPON_TRTCM_SCALE_MASK) |
				((channel<<GPON_TRTCM_CHANNEL_SHIFT)&GPON_TRTCM_CHANNEL_MASK) |
				((type<<GPON_TRTCM_TYPE_SHIFT)&GPON_TRTCM_TYPE_MASK)) ;
	IO_SREG(QDMA_CSR_GPON_TCONT_TRTCM_CFG(base), trtcmCfg) ;

	while(RETRY--) {
		trtcmCfg = IO_GREG(QDMA_CSR_GPON_TCONT_TRTCM_CFG(base)) ;
		
		if(trtcmCfg&GPON_TRTCM_DONE) {
			return 0 ;
		}
		mdelay(1) ;
	}

	QDMA_ERR("Timeout for set GPON trTCM configuration.\n") ;
	
	return -ETIME ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetGponTrtcmConfig(unchar channel, unchar type) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint trtcmCfg = 0 ;
	int RETRY = 3 ;
	
	trtcmCfg = (((channel<<GPON_TRTCM_CHANNEL_SHIFT)&GPON_TRTCM_CHANNEL_MASK) |
				((type<<GPON_TRTCM_TYPE_SHIFT)&GPON_TRTCM_TYPE_MASK)) ;
	IO_SREG(QDMA_CSR_GPON_TCONT_TRTCM_CFG(base), trtcmCfg) ;

	while(RETRY--) {
		trtcmCfg = IO_GREG(QDMA_CSR_GPON_TCONT_TRTCM_CFG(base)) ;
		
		if(trtcmCfg&GPON_TRTCM_DONE) {
			return ((trtcmCfg&GPON_TRTCM_VALUE_MASK)>>GPON_TRTCM_VALUE_SHIFT) ;
		} 
		mdelay(1) ;
	}

	QDMA_ERR("Timeout for get GPON trTCM configuration.\n") ;
	
	return -ETIME ;
}
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetEponReportConfig(unchar channel, unchar mode)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	
	if(channel == 0) {
		IO_SMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH0_MASK, EPON_RPT_CH0_SHIFT, mode) ;
	} else if(channel == 1) {
		IO_SMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH1_MASK, EPON_RPT_CH1_SHIFT, mode) ;
	} else if(channel == 2) {
		IO_SMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH2_MASK, EPON_RPT_CH2_SHIFT, mode) ;
	} else if(channel == 3) {
		IO_SMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH3_MASK, EPON_RPT_CH3_SHIFT, mode) ;
	} else if(channel == 4) {
		IO_SMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH4_MASK, EPON_RPT_CH4_SHIFT, mode) ;
	} else if(channel == 5) {
		IO_SMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH5_MASK, EPON_RPT_CH5_SHIFT, mode) ;
	} else if(channel == 6) {
		IO_SMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH6_MASK, EPON_RPT_CH6_SHIFT, mode) ;
	} else if(channel == 7) {
		IO_SMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH7_MASK, EPON_RPT_CH7_SHIFT, mode) ;
	} 
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetEponReportConfig(unchar channel, unchar *pMode)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	
	if(channel == 0) {
		*pMode = IO_GMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH0_MASK, EPON_RPT_CH0_SHIFT) ;
	} else if(channel == 1) {
		*pMode = IO_GMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH1_MASK, EPON_RPT_CH1_SHIFT) ;
	} else if(channel == 2) {
		*pMode = IO_GMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH2_MASK, EPON_RPT_CH2_SHIFT) ;
	} else if(channel == 3) {
		*pMode = IO_GMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH3_MASK, EPON_RPT_CH3_SHIFT) ;
	} else if(channel == 4) {
		*pMode = IO_GMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH4_MASK, EPON_RPT_CH4_SHIFT) ;
	} else if(channel == 5) {
		*pMode = IO_GMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH5_MASK, EPON_RPT_CH5_SHIFT) ;
	} else if(channel == 6) {
		*pMode = IO_GMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH6_MASK, EPON_RPT_CH6_SHIFT) ;
	} else if(channel == 7) {
		*pMode = IO_GMASK(QDMA_CSR_EPON_RPT_CFG(base), EPON_RPT_CH7_MASK, EPON_RPT_CH7_SHIFT) ;
	} 
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaSetEponThreshold(unchar channel, unchar queue, unchar type, ushort value)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint eponThresholdCfg = 0 ;
	int RETRY = 3 ;
	
	eponThresholdCfg = (EPON_QTHRESHLD_RWCMD | 
				((value<<EPON_QTHRESHLD_VALUE_SHIFT)&EPON_QTHRESHLD_VALUE_MASK) |
				((type<<EPON_QTHRESHLD_TYPE_SHIFT)&EPON_QTHRESHLD_TYPE_MASK) |
				((channel<<EPON_QTHRESHLD_CHANNEL_SHIFT)&EPON_QTHRESHLD_CHANNEL_MASK) |
				((queue<<EPON_QTHRESHLD_QUEUE_SHIFT)&EPON_QTHRESHLD_QUEUE_MASK)) ;
	IO_SREG(QDMA_CSR_EPON_QTHRESHLD_CFG(base), eponThresholdCfg) ;

	while(RETRY--) {
		eponThresholdCfg = IO_GREG(QDMA_CSR_EPON_QTHRESHLD_CFG(base)) ;
		
		if(eponThresholdCfg&EPON_QTHRESHLD_DONE) {
			return 0 ;
		}
		mdelay(1) ;
	}

	QDMA_ERR("Timeout for set EPON Threshold configuration.\n") ;
	
	return -ETIME ;
}

/******************************************************************************
******************************************************************************/
int __inline__ qdmaGetEponThreshold(unchar channel, unchar queue, unchar type)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint eponThresholdCfg = 0 ;
	int RETRY = 3 ;
	
	eponThresholdCfg = (((type<<EPON_QTHRESHLD_TYPE_SHIFT)&EPON_QTHRESHLD_TYPE_MASK) |
				((channel<<EPON_QTHRESHLD_CHANNEL_SHIFT)&EPON_QTHRESHLD_CHANNEL_MASK) |
				((queue<<EPON_QTHRESHLD_QUEUE_SHIFT)&EPON_QTHRESHLD_QUEUE_MASK)) ;
	IO_SREG(QDMA_CSR_EPON_QTHRESHLD_CFG(base), eponThresholdCfg) ;

	while(RETRY--) {
		eponThresholdCfg = IO_GREG(QDMA_CSR_EPON_QTHRESHLD_CFG(base)) ;
		
		if(eponThresholdCfg&EPON_QTHRESHLD_DONE) {
			return ((eponThresholdCfg&EPON_QTHRESHLD_VALUE_MASK)>>EPON_QTHRESHLD_VALUE_SHIFT) ;
		} 
		mdelay(1) ;
	}

	QDMA_ERR("Timeout for get EPON Threshold configuration.\n") ;
	
	return -ETIME ;
}

/******************************************************************************
******************************************************************************/
int qdmaEnableInt(uint base, uint bit)
{
	ulong flags ;
	uint RETRY=3 ;
	uint t ;
	
	spin_lock_irqsave(&gpQdmaPriv->irqLock, flags) ;
	//IO_SBITS(QDMA_CSR_INT_ENABLE(base), bit) ;
	
	while(RETRY--) {
		t = ioread32((void __iomem *)(QDMA_CSR_INT_ENABLE(base))) ;
		if(t == 0) {
			printk("########WARNING: Enable INT: Get INT_ENABLE:%.8x, RETRY:%d\n", t, 3-RETRY) ;
		} else {
			iowrite32((t|bit), (void __iomem *)(QDMA_CSR_INT_ENABLE(base))) ;
			break ;
		}
	}

	spin_unlock_irqrestore(&gpQdmaPriv->irqLock, flags) ;
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int qdmaDisableInt(uint base, uint bit)
{
	ulong flags ;
	uint RETRY=3 ;
	uint t ;
	
	spin_lock_irqsave(&gpQdmaPriv->irqLock, flags) ;

	//IO_CBITS(QDMA_CSR_INT_ENABLE(base), bit) ;
	while(RETRY--) {
		t = ioread32((void __iomem *)(QDMA_CSR_INT_ENABLE(base))) ;
		if(t == 0) {
			printk("########WARNING: Disable INT: Get INT_ENABLE:%.8x, RETRY:%d\n", t, 3-RETRY) ;
		} else {
			iowrite32((t&(~bit)), (void __iomem *)(QDMA_CSR_INT_ENABLE(base))) ;
			break ;
		}
	}
		
	spin_unlock_irqrestore(&gpQdmaPriv->irqLock, flags) ;
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int qdmaSetIntMask(uint base, uint value)
{
	ulong flags ;
	
	spin_lock_irqsave(&gpQdmaPriv->irqLock, flags) ;
	IO_SREG(QDMA_CSR_INT_ENABLE(base), value) ;
	spin_unlock_irqrestore(&gpQdmaPriv->irqLock, flags) ;
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int qdmaGetIntMask(uint base)
{
	ulong flags, value ;
	
	spin_lock_irqsave(&gpQdmaPriv->irqLock, flags);
	value = IO_GREG(QDMA_CSR_INT_ENABLE(base)) ;
	spin_unlock_irqrestore(&gpQdmaPriv->irqLock, flags);
	
	return value ;
}

/******************************************************************************
******************************************************************************/
int qdma_dev_init(void) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint glbCfg=0, intEnable=0 ;
	
	/********************************************
	* enable/disable the qdma interrupt         *
	*********************************************/
	intEnable = (INT_STATUS_RX_COHERENT | INT_STATUS_TX_COHERENT | INT_STATUS_IRQ_FULL | INT_STATUS_NO_LINK_DSCP | INT_STATUS_NO_RX_CPU_DSCP | INT_MASK_RX_DONE | INT_MASK_TX_DONE) ;
	qdmaSetIntMask(base, intEnable) ;
	
	/********************************************
	* Setting the global register               *
	*********************************************/
	glbCfg |= (GLB_CFG_DSCP_BYTE_SWAP | GLB_CFG_PAYLOAD_BYTE_SWAP | ((VAL_BST_32_DWARD<<GLB_CFG_BST_SE_SHIFT)&GLB_CFG_BST_SE_MASK)) ;

#ifdef TCSUPPORT_MERGED_DSCP_FORMAT	
	glbCfg |= GLB_CFG_DSCP_LEN_32 ;
	glbCfg |= GLB_CFG_DSCPMSG_BURST ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */

	if(gpQdmaPriv->irqDepth) {
		glbCfg |= GLB_CFG_IRQ_EN ;
	}
	
#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	/* config rx message length = 16 byte */
	glbCfg |= ((VAL_MSG_LEN_4_WORD<<GLB_CFG_MSG_LEN_SHIFT) & GLB_CFG_MSG_LEN_MASK)  ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

#ifdef CONFIG_RX_2B_OFFSET
	glbCfg |= GLB_CFG_RX_2B_OFFSET ;
#endif /* CONFIG_RX_2B_OFFSET */

#ifdef CONFIG_TX_WB_DONE
	glbCfg |= GLB_CFG_TX_WB_DONE ;
#endif /* GLB_CFG_TX_WB_DONE */
	
	qdmaSetGlbCfg(base, glbCfg) ;
	
	return 0 ;
}

