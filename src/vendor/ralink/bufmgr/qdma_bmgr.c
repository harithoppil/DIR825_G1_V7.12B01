#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include "qdma_bmgr.h"
#include "qdma_dev.h"
#include "qdma_dvt.h"

#ifdef MT7510_DMA_DSCP_CACHE
	#include <asm/r4kcache.h>
#endif /* MT7510_DMA_DSCP_CACHE */

/*****************************************************************************
******************************************************************************
   Define the Global variable 
******************************************************************************
******************************************************************************/
QDMA_Private_T *gpQdmaPriv = NULL ;


/******************************************************************************
 Packet Receive
******************************************************************************/
/******************************************************************************
******************************************************************************/
static inline void qdma_bm_add_rx_dscp(struct QDMA_DscpInfo_S *diPtr) 
{
	if(!gpQdmaPriv->rxStartPtr) {
		gpQdmaPriv->rxStartPtr = diPtr ;
		diPtr->next = gpQdmaPriv->rxStartPtr ;
	} else {
		diPtr->next = gpQdmaPriv->rxStartPtr->next ;
		gpQdmaPriv->rxStartPtr->next = diPtr ;
		gpQdmaPriv->rxStartPtr = diPtr ;
	}
}

/******************************************************************************
******************************************************************************/
static inline struct QDMA_DscpInfo_S *qdma_bm_remove_rx_dscp(void)
{
	struct QDMA_DscpInfo_S *diPtr = NULL ;

	if(gpQdmaPriv->rxStartPtr) {
		diPtr = gpQdmaPriv->rxStartPtr->next ;
		
		if(gpQdmaPriv->rxStartPtr == diPtr) {
			gpQdmaPriv->rxStartPtr = NULL ;
		} else {
			gpQdmaPriv->rxStartPtr->next = diPtr->next ;
		}
	}

	return diPtr ;
}

/******************************************************************************
******************************************************************************/
static inline struct QDMA_DscpInfo_S *qdma_bm_get_unused_rx_dscp(void)
{
	struct QDMA_DscpInfo_S *diPtr = NULL ;
	ulong flags ;
	
	spin_lock_irqsave(&gpQdmaPriv->rxLock, flags) ;
	if(gpQdmaPriv->rxStartPtr) {
		if(!gpQdmaPriv->rxEndPtr) {
			diPtr = gpQdmaPriv->rxStartPtr ;
			gpQdmaPriv->rxEndPtr = diPtr ;
		} else if(gpQdmaPriv->rxEndPtr->next != gpQdmaPriv->rxStartPtr) {
			diPtr = gpQdmaPriv->rxEndPtr->next ;
			gpQdmaPriv->rxEndPtr = diPtr ; ;
		}
	} 
	spin_unlock_irqrestore(&gpQdmaPriv->rxLock, flags) ;

	return diPtr ;
}

/******************************************************************************
******************************************************************************/
int qdma_bm_receive_packets(uint maxPkts) 
{
#ifdef MT7510_DMA_DSCP_CACHE
	QDMA_DMA_DSCP_T *prxDscp ;
#else
	QDMA_DMA_DSCP_T rxDscp ;
#endif /* MT7510_DMA_DSCP_CACHE */
	struct QDMA_DscpInfo_S dscpInfo ;
	ulong flags ;
	uint cnt = maxPkts ;
	uint pktCount = 0 ;
	
	do {
#ifdef MT7510_DMA_DSCP_CACHE
		spin_lock_irqsave(&gpQdmaPriv->rxLock, flags) ;

		//dma_cache_inv((unsigned long)gpQdmaPriv->rxStartPtr->dscpPtr, CACHE_LINE_SIZE);
		protected_cache_op(Hit_Invalidate_D, ((unsigned long)(gpQdmaPriv->rxStartPtr->dscpPtr)));
		if(!gpQdmaPriv->rxStartPtr || gpQdmaPriv->rxStartPtr==gpQdmaPriv->rxEndPtr || !gpQdmaPriv->rxStartPtr->dscpPtr->ctrl.done) {
			goto ret ;
		}
		prxDscp = gpQdmaPriv->rxStartPtr->dscpPtr;
		
		memcpy(&dscpInfo, gpQdmaPriv->rxStartPtr, sizeof(struct QDMA_DscpInfo_S)) ;
		
		gpQdmaPriv->rxStartPtr = gpQdmaPriv->rxStartPtr->next ;
		spin_unlock_irqrestore(&gpQdmaPriv->rxLock, flags) ;
		
		pktCount++ ;
		
		/* check DSCP cotent */
		if(!prxDscp->msg_addr || !prxDscp->ctrl.msg_len || !prxDscp->pkt_addr || !prxDscp->ctrl.pkt_len) {
			QDMA_ERR("The content of the RX DSCP is incorrect.\n") ;
			gpQdmaPriv->counters.rxDscpIncorrect++ ; 
			break ;
		}
		
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
		if(dscpInfo.msgPtr) {
			memcpy(dscpInfo.msgPtr, rxDscp.msg, rxDscp.ctrl.msg_len) ;
		} else {
			dscpInfo.msgPtr = (void *)rxDscp.msg ;
		}
#else
		dma_unmap_single(NULL, prxDscp->msg_addr, prxDscp->ctrl.msg_len, DMA_FROM_DEVICE) ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
		dma_unmap_single(NULL, prxDscp->pkt_addr, skb_tailroom(dscpInfo.skb), DMA_FROM_DEVICE) ;

		if(gpQdmaPriv->devCfg.bmRecvCallbackFunction) {
			if(gpQdmaPriv->devCfg.bmRecvCallbackFunction(dscpInfo.msgPtr, prxDscp->ctrl.msg_len, dscpInfo.skb, prxDscp->ctrl.pkt_len)) {
				gpQdmaPriv->counters.rxPktErrs++ ;
			} else {
				gpQdmaPriv->counters.rxCounts++ ;
			} 
		} else {
			gpQdmaPriv->counters.noRxCbErrs++ ;
		}
#else
		spin_lock_irqsave(&gpQdmaPriv->rxLock, flags) ;
		if(!gpQdmaPriv->rxStartPtr || gpQdmaPriv->rxStartPtr==gpQdmaPriv->rxEndPtr || !gpQdmaPriv->rxStartPtr->dscpPtr->ctrl.done) {
			goto ret ;
		}
		memcpy(&rxDscp, gpQdmaPriv->rxStartPtr->dscpPtr, sizeof(QDMA_DMA_DSCP_T)) ;
		memcpy(&dscpInfo, gpQdmaPriv->rxStartPtr, sizeof(struct QDMA_DscpInfo_S)) ;
		
		gpQdmaPriv->rxStartPtr = gpQdmaPriv->rxStartPtr->next ;
		spin_unlock_irqrestore(&gpQdmaPriv->rxLock, flags) ;
		
		pktCount++ ;
		
		/* check DSCP cotent */
		if(!rxDscp.msg_addr || !rxDscp.ctrl.msg_len || !rxDscp.pkt_addr || !rxDscp.ctrl.pkt_len) {
			QDMA_ERR("The content of the RX DSCP is incorrect.\n") ;
			gpQdmaPriv->counters.rxDscpIncorrect++ ; 
			break ;
		}
		
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
		if(dscpInfo.msgPtr) {
			memcpy(dscpInfo.msgPtr, rxDscp.msg, rxDscp.ctrl.msg_len) ;
		} else {
			dscpInfo.msgPtr = (void *)rxDscp.msg ;
		}
#else
		dma_unmap_single(NULL, rxDscp.msg_addr, rxDscp.ctrl.msg_len, DMA_FROM_DEVICE) ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
		dma_unmap_single(NULL, rxDscp.pkt_addr, skb_tailroom(dscpInfo.skb), DMA_FROM_DEVICE) ;

		if(gpQdmaPriv->devCfg.bmRecvCallbackFunction) {
			if(gpQdmaPriv->devCfg.bmRecvCallbackFunction(dscpInfo.msgPtr, rxDscp.ctrl.msg_len, dscpInfo.skb, rxDscp.ctrl.pkt_len)) {
				gpQdmaPriv->counters.rxPktErrs++ ;
			} else {
				gpQdmaPriv->counters.rxCounts++ ;
			} 
		} else {
			gpQdmaPriv->counters.noRxCbErrs++ ;
		}
#endif /* MT7510_DMA_DSCP_CACHE */
	} while((!maxPkts) || (--cnt)) ;

	return pktCount ;

ret:
	spin_unlock_irqrestore(&gpQdmaPriv->rxLock, flags);
	return pktCount ;
}

/******************************************************************************
******************************************************************************/
int qdma_bm_hook_receive_buffer(void *pMsg, uint msgLen, struct sk_buff *skb)
{
	struct QDMA_DscpInfo_S *pNewDscpInfo ;
	QDMA_DMA_DSCP_T *pRxDscp ;
	dma_addr_t dmaPktAddr ;
	uint base = gpQdmaPriv->csrBaseAddr ;
	ulong flags ;
	int ret = 0 ;

#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	if(!skb) {
#else
	if(!pMsg || msgLen<=0 || !skb) {
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
		QDMA_ERR("The input arguments are wrong, pMsg:%.8x, msgLen:%d, skb:%.8x, skbLen:%d.\n", (uint)pMsg, msgLen, (uint)skb, skb_tailroom(skb)) ; 
		return -EFAULT ;
	}
	
	pNewDscpInfo = qdma_bm_get_unused_rx_dscp() ;
	if(pNewDscpInfo == NULL) {
		QDMA_ERR("There is not any free RX DSCP.\n") ; 
		gpQdmaPriv->counters.noRxDscps++ ;
		return -ENOMEM ;
	}
	
#ifdef CONFIG_RX_2B_OFFSET
	QDMA_MSG(DEG_MSG, "Adjust the skb->tail location for net IP alignment\n") ;
	if(((uint)skb->tail & 3) == 0) {
		skb_reserve(skb, NET_IP_ALIGN) ;
	}
	dmaPktAddr = dma_map_single(NULL, (void *)((uint)skb->tail-NET_IP_ALIGN), (size_t)skb_tailroom(skb), DMA_FROM_DEVICE) ;
#else
	dmaPktAddr = dma_map_single(NULL, (void *)((uint)skb->tail), (size_t)skb_tailroom(skb), DMA_FROM_DEVICE) ;
#endif /* CONFIG_RX_2B_OFFSET */
	
	spin_lock_irqsave(&gpQdmaPriv->rxLock, flags) ;
	pRxDscp = gpQdmaPriv->rxUsingPtr->dscpPtr ;
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	memset(pRxDscp->msg, 0, QDMA_DSCP_MSG_LENS) ;
#else
	pRxDscp->msg_addr = dma_map_single(NULL, pMsg, (size_t)msgLen, DMA_FROM_DEVICE) ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
	pRxDscp->pkt_addr = dmaPktAddr ;
	pRxDscp->next_idx = pNewDscpInfo->dscpIdx ;
	pRxDscp->ctrl.msg_len = msgLen ;
	pRxDscp->ctrl.pkt_len = skb_tailroom(skb) ;
#ifdef CONFIG_SUPPORT_SELF_TEST
	pRxDscp->ctrl.done = gpQdmaPriv->devCfg.rxDscpDoneBit ;
#else
	pRxDscp->ctrl.done = 0 ;
#endif /* CONFIG_SUPPORT_SELF_TEST */

#ifdef MT7510_DMA_DSCP_CACHE
	//dma_cache_wback_inv((unsigned long)pRxDscp, CACHE_LINE_SIZE);
	protected_cache_op(Hit_Writeback_Inv_D, ((unsigned long)(pRxDscp)));
#endif /* MT7510_DMA_DSCP_CACHE */

	QDMA_MSG(DEG_MSG, "Hook RX DSCP to RXDMA. RX_CPU_IDX:%.8x, RX_NULL_IDX:%.8x\n", gpQdmaPriv->rxUsingPtr->dscpIdx, pNewDscpInfo->dscpIdx) ;
	QDMA_MSG(DEG_MSG, "RXDSCP: DONE:%d, MSG:%.8x, MSGLEN:%d, PKT:%.8x, PKTLEN:%d, NEXT_IDX:%d\n", 
													(uint)pRxDscp->ctrl.done, 
													(uint)pRxDscp->msg_addr,
													(uint)pRxDscp->ctrl.msg_len,
													(uint)pRxDscp->pkt_addr,
													(uint)pRxDscp->ctrl.pkt_len,
													(uint)pRxDscp->next_idx) ;
													
	gpQdmaPriv->rxUsingPtr->msgPtr = (void *)pMsg ;
	gpQdmaPriv->rxUsingPtr->skb = skb ;
	gpQdmaPriv->rxUsingPtr = pNewDscpInfo ;

	wmb() ;
	
	/* Setting DMA Rx Descriptor Register */
	qdmaSetRxCpuIdx(base, pNewDscpInfo->dscpIdx) ;
	spin_unlock_irqrestore(&gpQdmaPriv->rxLock, flags) ;
	
	return ret ;
}

/******************************************************************************
******************************************************************************/
int qdma_bm_recycle_receive_buffer(void) 
{
	QDMA_DMA_DSCP_T *pRxDscp ;
	struct QDMA_DscpInfo_S *diPtr ;
	uint base = gpQdmaPriv->csrBaseAddr ;
	ulong flags ;

	if(!gpQdmaPriv->rxStartPtr || !gpQdmaPriv->rxEndPtr) {
		return -EFAULT ;
	}

	spin_lock_irqsave(&gpQdmaPriv->rxLock, flags) ;
	
	while(gpQdmaPriv->rxStartPtr != gpQdmaPriv->rxEndPtr) {
		diPtr = gpQdmaPriv->rxStartPtr ;
		pRxDscp = gpQdmaPriv->rxStartPtr->dscpPtr ;

		if(/*diPtr->msgPtr && pRxDscp->msg_addr && */diPtr->skb && pRxDscp->pkt_addr) {
#ifndef TCSUPPORT_MERGED_DSCP_FORMAT
			dma_unmap_single(NULL, pRxDscp->msg_addr, pRxDscp->ctrl.msg_len, DMA_FROM_DEVICE) ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
			dma_unmap_single(NULL, pRxDscp->pkt_addr, skb_tailroom(diPtr->skb), DMA_FROM_DEVICE) ;
	
			if(gpQdmaPriv->devCfg.bmRecvCallbackFunction) {
				gpQdmaPriv->devCfg.bmRecvCallbackFunction(diPtr->msgPtr, pRxDscp->ctrl.msg_len, diPtr->skb, pRxDscp->ctrl.pkt_len) ;
			}
		}
		
		gpQdmaPriv->rxStartPtr = diPtr->next ;		
	}

	diPtr = gpQdmaPriv->rxStartPtr ;
	gpQdmaPriv->rxUsingPtr = diPtr ;
	qdmaSetRxCpuIdx(base, diPtr->dscpIdx) ;
	qdmaSetRxDmaIdx(base, diPtr->dscpIdx) ;
	
	spin_unlock_irqrestore(&gpQdmaPriv->rxLock, flags) ;
	
	return 0 ;
}

/******************************************************************************
 Packet Transmit
******************************************************************************/
/******************************************************************************
******************************************************************************/
static inline int qdma_bm_push_tx_dscp(struct QDMA_DscpInfo_S *diPtr) 
{
	if(diPtr->next != NULL) {
		QDMA_ERR("The TX DSCP is not return from tx used pool\n") ;
		return -1 ;
	}
	
	diPtr->msgPtr = NULL ;
	diPtr->skb = NULL ;

	if(!gpQdmaPriv->txHeadPtr) {
		gpQdmaPriv->txHeadPtr = diPtr ;
		gpQdmaPriv->txTailPtr = diPtr ;
	} else {
		gpQdmaPriv->txTailPtr->next = diPtr ;
		gpQdmaPriv->txTailPtr = gpQdmaPriv->txTailPtr->next ;
	}
	
	return 0 ;
}


/******************************************************************************
******************************************************************************/
static inline struct QDMA_DscpInfo_S *qdma_bm_pop_tx_dscp(void)
{
	struct QDMA_DscpInfo_S *diPtr ;
	ulong flags ;
	
	spin_lock_irqsave(&gpQdmaPriv->txLock, flags);
	diPtr = gpQdmaPriv->txHeadPtr ;
	if(gpQdmaPriv->txHeadPtr == gpQdmaPriv->txTailPtr) {
		gpQdmaPriv->txHeadPtr = NULL ;
		gpQdmaPriv->txTailPtr = NULL ;
	} else {
		gpQdmaPriv->txHeadPtr = gpQdmaPriv->txHeadPtr->next ;
	}
	spin_unlock_irqrestore(&gpQdmaPriv->txLock, flags);

	if(diPtr) {
		diPtr->next = NULL ;
	}
	
	return diPtr ;
}

/******************************************************************************
******************************************************************************/
int qdma_bm_transmit_done(int amount) 
{
#ifdef MT7510_DMA_DSCP_CACHE
	QDMA_DMA_DSCP_T *ptxDscp ;
#else
	QDMA_DMA_DSCP_T txDscp ;
#endif /*MT7510_DMA_DSCP_CACHE */
	int ret = 0 ;
	struct QDMA_DscpInfo_S *diPtr ;
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint entryLen, headIdx, irqValue=0, irqDepth=CONFIG_IRQ_DEPTH ;
	uint *irqPtr ;
	int i=0, j, idx = 0 ;
	uint RETRY=3 ;
	ulong flags ;
	uint irqStatus ;
	
	spin_lock_irqsave(&gpQdmaPriv->txLock, flags) ;

	irqStatus = qdmaGetIrqStatus(base) ;
	headIdx = (irqStatus & IRQ_STATUS_HEAD_IDX_MASK) >> IRQ_STATUS_HEAD_IDX_SHIFT ;
	entryLen = (irqStatus & IRQ_STATUS_ENTRY_LEN_MASK) >> IRQ_STATUS_ENTRY_LEN_SHIFT ;
	if(entryLen == 0) {
		goto out2 ;
	}

	entryLen = (amount && amount<entryLen) ? amount : entryLen ;
	for(i=0 ; i<entryLen ; i++) {
		irqPtr = (uint *)gpQdmaPriv->irqQueueAddr + ((headIdx+i)%irqDepth) ;
		
		RETRY = 3 ;
		while(RETRY--) {
			irqValue = *irqPtr ;
			if(irqValue == CONFIG_IRQ_DEF_VALUE) {
				QDMA_ERR("There is no data available in IRQ queue. irq value:%.8x, irq ptr:%.8x TIMEs:%d\n", (uint)irqValue, (uint)irqPtr, RETRY) ;
				if(RETRY <= 0) {
					gpQdmaPriv->counters.IrqQueueAsynchronous++ ;
					ret = -ENODATA ;
					goto out1 ;
				}
			} else {
				*irqPtr = CONFIG_IRQ_DEF_VALUE ;
				break ;
			}
		}
		
		idx = (irqValue & IRQ_CFG_IDX_MASK) ;
		if(idx<0 || idx>=gpQdmaPriv->txDscpNum) {
			QDMA_ERR("The TX DSCP index %d is invalid.\n", idx) ;
			gpQdmaPriv->counters.txIrqQueueIdxErrs++ ;
			ret = -EFAULT ;
			continue ;
		}
		
		diPtr = (struct QDMA_DscpInfo_S *)gpQdmaPriv->dscpInfoAddr + idx ;
		if(diPtr->dscpIdx!=idx || diPtr->next!=NULL) {
			QDMA_ERR("The content of the TX DSCP_INFO(%.8x) is incorrect. ENTRY_LEN:%d, HEAD_IDX:%d, IRQ_VALUE:%.8x.\n", (uint)diPtr, entryLen, headIdx, irqValue) ;
			gpQdmaPriv->counters.txDscpIncorrect++ ;
			ret = -EFAULT ;
			continue ;
		}
		
#ifdef MT7510_DMA_DSCP_CACHE
		ptxDscp = diPtr->dscpPtr;
		//dma_cache_inv((unsigned long)ptxDscp, 32);
		protected_cache_op(Hit_Invalidate_D, ((unsigned long)(ptxDscp)));

#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
		if(ptxDscp->ctrl.drop_pkt) {
			gpQdmaPriv->counters.txQdmaDropCounts++ ;
		} else
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
		{
#ifdef CONFIG_TX_WB_DONE
		if(!ptxDscp->ctrl.done) {
			QDMA_ERR("The done bit of TX DSCP is incorrect. ADDR:%.8x, IDX:%d.\n", (uint)diPtr->dscpPtr, diPtr->dscpIdx) ;
			gpQdmaPriv->counters.txDscpDoneErrs++ ;
			ret = -EFAULT ;
			continue ;
		}
#endif /* CONFIG_TX_WB_DONE */
		}		
			
#ifndef TCSUPPORT_MERGED_DSCP_FORMAT
		if(ptxDscp->msg_addr) {
			dma_unmap_single(NULL, ptxDscp->msg_addr, ptxDscp->ctrl.msg_len, DMA_TO_DEVICE) ;
		}
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
		if(ptxDscp->pkt_addr) {
			dma_unmap_single(NULL, ptxDscp->pkt_addr, ptxDscp->ctrl.pkt_len, DMA_TO_DEVICE) ;
		}
#else /* ifndef MT7510_DMA_DSCP_CACHE */
		//Copy the DSCP from non-cache memory to cache memory
		memcpy(&txDscp, diPtr->dscpPtr, sizeof(QDMA_DMA_DSCP_T)) ;

#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
		if(txDscp.ctrl.drop_pkt) {
			gpQdmaPriv->counters.txQdmaDropCounts++ ;
		} else
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
		{
#ifdef CONFIG_TX_WB_DONE
			if(!txDscp.ctrl.done) {
				QDMA_ERR("The done bit of TX DSCP is incorrect. ADDR:%.8x, IDX:%d.\n", (uint)diPtr->dscpPtr, diPtr->dscpIdx) ;
				gpQdmaPriv->counters.txDscpDoneErrs++ ;
				ret = -EFAULT ;
				continue ;
			}
#endif /* CONFIG_TX_WB_DONE */
		}		
			
#ifndef TCSUPPORT_MERGED_DSCP_FORMAT
		if(txDscp.msg_addr) {
			dma_unmap_single(NULL, txDscp.msg_addr, txDscp.ctrl.msg_len, DMA_TO_DEVICE) ;
		}
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
		if(txDscp.pkt_addr) {
			dma_unmap_single(NULL, txDscp.pkt_addr, txDscp.ctrl.pkt_len, DMA_TO_DEVICE) ;
		}
#endif /* MT7510_DMA_DSCP_CACHE */

		if(gpQdmaPriv->devCfg.bmXmitCallbackFunction) {
			if(gpQdmaPriv->devCfg.bmXmitCallbackFunction(diPtr->msgPtr, diPtr->skb) != 0) {
				gpQdmaPriv->counters.txRecycleErrs++ ;
			} else {
				gpQdmaPriv->counters.txRecycleCounts++ ;
			} 
		} else {
			gpQdmaPriv->counters.noTxCbErrs++ ;
		}
		
		qdma_bm_push_tx_dscp(diPtr) ;
	}

out1:
	for(j=0 ; j<(i>>7) ; j++) {
		qdmaSetIrqClearLen(base, 0x80) ;
	}
	qdmaSetIrqClearLen(base, (i&0x7F)) ;

out2:
	spin_unlock_irqrestore(&gpQdmaPriv->txLock, flags) ;
	return ret ;
}

/******************************************************************************
******************************************************************************/
int qdma_bm_transmit_packet(void *pMsg, uint msgLen, struct sk_buff *skb)
{
	struct QDMA_DscpInfo_S *pNewDscpInfo ;
	QDMA_DMA_DSCP_T *pTxDscp ;
	uint base = gpQdmaPriv->csrBaseAddr ;
	ulong flags ;//, pktLen ;
	int ret = 0 ;
	int a ;
	
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	if(!skb || skb->len<=0 || skb->len>CONFIG_MAX_PKT_LENS) 
#else
	if(!pMsg || msgLen<=0 || msgLen>16 || !skb || skb->len<=0 || skb->len>CONFIG_MAX_PKT_LENS) 
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
	{
		QDMA_ERR("The input arguments are wrong, pMsg:%.8x, msgLen:%d, skb:%.8x, skbLen:%d.\n", (uint)pMsg, msgLen, (uint)skb, skb->len) ; 
		return -EFAULT ;
	}

#ifndef CONFIG_TX_POLLING_BY_MAC
	/* recycle TX DSCP when send packets in tx polling mode */
	if(gpQdmaPriv->devCfg.flags.isTxPolling == QDMA_ENABLE) {
		if(qdmaGetIrqEntryLen(base) >= gpQdmaPriv->devCfg.txRecycleThreshold) {
			qdma_bm_transmit_done(0) ;
		}
	}
#endif /* CONFIG_TX_POLLING_BY_MAC */

	/* Get unused TX DSCP from TX unused DSCP link list */	
	pNewDscpInfo = qdma_bm_pop_tx_dscp() ;
	if(pNewDscpInfo == NULL) {
		gpQdmaPriv->counters.noTxDscps++ ;
		return -ENOSR ;
	}

	spin_lock_irqsave(&gpQdmaPriv->txLock, flags);
	pTxDscp = gpQdmaPriv->txUsingPtr->dscpPtr ;
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	memset(pTxDscp->msg, 0, QDMA_DSCP_MSG_LENS) ;
	if(!pMsg) {
		if(!gpQdmaPriv->devCfg.bmXmitMsgCallbackFunction || (ret = gpQdmaPriv->devCfg.bmXmitMsgCallbackFunction((void *)pTxDscp->msg, skb)) != 0) {
			qdma_bm_push_tx_dscp(pNewDscpInfo) ;
			spin_unlock_irqrestore(&gpQdmaPriv->txLock, flags) ;
			return ret ;
		}
	} else {
		memcpy(pTxDscp->msg, pMsg, msgLen) ;
	}
#else
	pTxDscp->msg_addr = dma_map_single(NULL, pMsg, (size_t)msgLen, DMA_TO_DEVICE) ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
	pTxDscp->next_idx = pNewDscpInfo->dscpIdx ;
	pTxDscp->pkt_addr = dma_map_single(NULL, skb->data, skb->len /* pktLen */, DMA_TO_DEVICE) ;
	pTxDscp->ctrl.msg_len = msgLen ;
	pTxDscp->ctrl.pkt_len = skb->len /* pktLen */; 
#ifdef CONFIG_SUPPORT_SELF_TEST
	pTxDscp->ctrl.done = gpQdmaPriv->devCfg.txDscpDoneBit ;
#else
	pTxDscp->ctrl.done = 0 ;
#endif /* CONFIG_SUPPORT_SELF_TEST */
	pTxDscp->ctrl.no_drop = (skb->mark&QOS_NODROP_MARK) ? 1 : 0 ;

#ifdef MT7510_DMA_DSCP_CACHE
	//dma_cache_wback_inv((unsigned long)pTxDscp, CACHE_LINE_SIZE);
	protected_cache_op(Hit_Writeback_Inv_D, ((unsigned long)(pTxDscp)));
#endif /* MT7510_DMA_DSCP_CACHE */

	QDMA_MSG(DEG_MSG, "Hook TX DSCP to TXDMA. TX_CPU_IDX:%d, TX_NULL_IDX:%d\n", gpQdmaPriv->txUsingPtr->dscpIdx, pNewDscpInfo->dscpIdx) ;
	QDMA_MSG(DEG_MSG, "TXDSCP: DONE:%d, MSG:%.8x, MSGLEN:%d, PKT:%.8x, PKTLEN:%d, NEXT_IDX:%d\n", 
																(uint)pTxDscp->ctrl.done, 
																(uint)pTxDscp->msg_addr,
																(uint)pTxDscp->ctrl.msg_len,
																(uint)pTxDscp->pkt_addr,
																(uint)pTxDscp->ctrl.pkt_len,
																(uint)pTxDscp->next_idx) ;

	gpQdmaPriv->txUsingPtr->msgPtr = (void *)pMsg ;
	gpQdmaPriv->txUsingPtr->skb = skb ;
	gpQdmaPriv->txUsingPtr = pNewDscpInfo ;

	wmb() ;

#ifdef CONFIG_SUPPORT_SELF_TEST
	gpQdmaPriv->devCfg.countDown-- ;
	if(!gpQdmaPriv->devCfg.countDown) {
		qdmaSetTxCpuIdx(base, pNewDscpInfo->dscpIdx) ;
		gpQdmaPriv->devCfg.countDown = (gpQdmaPriv->devCfg.waitTxMaxNums) ? (gpQdmaPriv->devCfg.waitTxMaxNums) : ((random32()%(gpQdmaPriv->txDscpNum-1))+1) ;
	} 
#else
	qdmaSetTxCpuIdx(base, pNewDscpInfo->dscpIdx) ;
#endif /* CONFIG_SUPPORT_SELF_TEST */
	
	gpQdmaPriv->counters.txCounts++ ;
	spin_unlock_irqrestore(&gpQdmaPriv->txLock, flags);

	QDMA_MSG(DEG_MSG, "GLG:%.8x, IRQStatus:%.8x, CSR info: RX_CPU_IDX:%d, RX_DMA_IDX:%d, TX_CPU_IDX:%d, TX_DMA_IDX:%d\n", 
																qdmaGetGlbCfg(base), 
																qdmaGetIrqStatus(base), 
																qdmaGetRxCpuIdx(base), 
																qdmaGetRxDmaIdx(base), 
																qdmaGetTxCpuIdx(base), 
																qdmaGetTxDmaIdx(base)) ;
	return ret ;
}

/******************************************************************************
******************************************************************************/
int qdma_bm_recycle_transmit_buffer(void)
{
	struct QDMA_DscpInfo_S *diPtr ;
	QDMA_DMA_DSCP_T *pTxDscp ;
	uint base = gpQdmaPriv->csrBaseAddr ;
	ulong flags ;
	int i ;

	spin_lock_irqsave(&gpQdmaPriv->txLock, flags) ;

	for(i=0 ; i<gpQdmaPriv->txDscpNum ; i++) {
		diPtr = (struct QDMA_DscpInfo_S *)gpQdmaPriv->dscpInfoAddr + i ;
		if(diPtr->next==NULL && diPtr!=gpQdmaPriv->txTailPtr && diPtr!=gpQdmaPriv->txUsingPtr) {
			pTxDscp = diPtr->dscpPtr ;
		
			if(/*diPtr->msgPtr && pTxDscp->msg_addr && */diPtr->skb && pTxDscp->pkt_addr) {
#ifndef TCSUPPORT_MERGED_DSCP_FORMAT
				dma_unmap_single(NULL, pTxDscp->msg_addr, pTxDscp->ctrl.msg_len, DMA_TO_DEVICE) ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
				dma_unmap_single(NULL, pTxDscp->pkt_addr, pTxDscp->ctrl.pkt_len, DMA_TO_DEVICE) ;
		
				if(gpQdmaPriv->devCfg.bmXmitCallbackFunction) {
					gpQdmaPriv->devCfg.bmXmitCallbackFunction(diPtr->msgPtr, diPtr->skb) ;
				}
			}
		
			qdma_bm_push_tx_dscp(diPtr) ;
		}
	}

	spin_unlock_irqrestore(&gpQdmaPriv->txLock, flags) ;

	diPtr = gpQdmaPriv->txUsingPtr ;
	qdmaSetTxCpuIdx(base, diPtr->dscpIdx) ;
	qdmaSetTxDmaIdx(base, diPtr->dscpIdx) ;
	
	return 0 ;
}

/******************************************************************************
 Proc function for QDMA debug
******************************************************************************/
/******************************************************************************
******************************************************************************/
static void __dump_csr(void)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	printk("QDMA CSR: 0x0000 QDMA_CSR_INFO					:%.8x\n", IO_GREG(QDMA_CSR_INFO(base))) ;
	printk("QDMA CSR: 0x0004 QDMA_CSR_GLB_CFG				:%.8x\n", IO_GREG(QDMA_CSR_GLB_CFG(base))) ;   
	printk("QDMA CSR: 0x0008 QDMA_CSR_TX_DSCP_BASE			:%.8x\n", IO_GREG(QDMA_CSR_TX_DSCP_BASE(base))) ;
	printk("QDMA CSR: 0x000C QDMA_CSR_RX_DSCP_BASE			:%.8x\n", IO_GREG(QDMA_CSR_RX_DSCP_BASE(base))) ;
	printk("QDMA CSR: 0x0010 QDMA_CSR_TX_CPU_IDX			:%.8x\n", IO_GREG(QDMA_CSR_TX_CPU_IDX(base))) ;   
	printk("QDMA CSR: 0x0014 QDMA_CSR_TX_DMA_IDX			:%.8x\n", IO_GREG(QDMA_CSR_TX_DMA_IDX(base))) ;   
	printk("QDMA CSR: 0x0018 QDMA_CSR_RX_CPU_IDX			:%.8x\n", IO_GREG(QDMA_CSR_RX_CPU_IDX(base))) ;   
	printk("QDMA CSR: 0x001C QDMA_CSR_RX_DMA_IDX			:%.8x\n", IO_GREG(QDMA_CSR_RX_DMA_IDX(base))) ;   
	printk("QDMA CSR: 0x0020 QDMA_CSR_HWFWD_DSCP_BASE		:%.8x\n", IO_GREG(QDMA_CSR_HWFWD_DSCP_BASE(base))) ;   
	printk("QDMA CSR: 0x0024 QDMA_CSR_HWFWD_AVAIL_DSCP_NUM	:%.8x\n", IO_GREG(QDMA_CSR_HWFWD_AVAIL_DSCP_NUM(base))) ;
	printk("QDMA CSR: 0x0028 QDMA_CSR_HWFWD_USED_DSCP_NUM	:%.8x\n", IO_GREG(QDMA_CSR_HWFWD_USED_DSCP_NUM(base))) ;   
	printk("QDMA CSR: 0x0030 QDMA_CSR_HWFWD_TX_IDX			:%.8x\n", IO_GREG(QDMA_CSR_HWFWD_TX_IDX(base))) ;
	printk("QDMA CSR: 0x0034 QDMA_CSR_HWFWD_RX_IDX			:%.8x\n", IO_GREG(QDMA_CSR_HWFWD_RX_IDX(base))) ;
	printk("QDMA CSR: 0x0038 QDMA_CSR_HWFWD_FREE_IDX		:%.8x\n", IO_GREG(QDMA_CSR_HWFWD_FREE_IDX(base))) ;   
	printk("QDMA CSR: 0x0050 QDMA_CSR_INT_STATUS			:%.8x\n", IO_GREG(QDMA_CSR_INT_STATUS(base))) ;   
	printk("QDMA CSR: 0x0054 QDMA_CSR_INT_ENABLE			:%.8x\n", IO_GREG(QDMA_CSR_INT_ENABLE(base))) ;   
	printk("QDMA CSR: 0x0058 QDMA_CSR_DELAY_INT_CFG			:%.8x\n", IO_GREG(QDMA_CSR_DELAY_INT_CFG(base))) ;   
	printk("QDMA CSR: 0x0060 QDMA_CSR_IRQ_BASE				:%.8x\n", IO_GREG(QDMA_CSR_IRQ_BASE(base))) ;
	printk("QDMA CSR: 0x0064 QDMA_CSR_IRQ_CFG				:%.8x\n", IO_GREG(QDMA_CSR_IRQ_CFG(base))) ;   
	printk("QDMA CSR: 0x0068 QDMA_CSR_IRQ_CLEAR_LEN			:%.8x\n", IO_GREG(QDMA_CSR_IRQ_CLEAR_LEN(base))) ;   
	printk("QDMA CSR: 0x006C QDMA_CSR_IRQ_STATUS			:%.8x\n", IO_GREG(QDMA_CSR_IRQ_STATUS(base))) ;   
	printk("QDMA CSR: 0x0070 QDMA_CSR_IRQ_PTIME				:%.8x\n", IO_GREG(QDMA_CSR_IRQ_PTIME(base))) ;   
	printk("QDMA CSR: 0x0080 QDMA_CSR_TXQOS_CHN07_CFG		:%.8x\n", IO_GREG(QDMA_CSR_TXQOS_CHN07_CFG(base))) ;   
	printk("QDMA CSR: 0x0084 QDMA_CSR_TXQOS_CHN815_CFG		:%.8x\n", IO_GREG(QDMA_CSR_TXQOS_CHN815_CFG(base))) ;
	printk("QDMA CSR: 0x0088 QDMA_CSR_TXQOS_WRR_CFG			:%.8x\n", IO_GREG(QDMA_CSR_TXQOS_WRR_CFG(base))) ;   
	printk("QDMA CSR: 0x0090 QDMA_CSR_TXDMA_SCH_CFG			:%.8x\n", IO_GREG(QDMA_CSR_TXDMA_SCH_CFG(base))) ;   
	printk("QDMA CSR: 0x0094 QDMA_CSR_TXBUF_THR_CFG			:%.8x\n", IO_GREG(QDMA_CSR_TXBUF_THR_CFG(base))) ;   
	printk("QDMA CSR: 0x0098 QDMA_CSR_TXQ_DROP_CFG			:%.8x\n", IO_GREG(QDMA_CSR_TXQ_DROP_CFG(base))) ;
	printk("QDMA CSR: 0x00C0 QDMA_CSR_GPON_RPT_CFG			:%.8x\n", IO_GREG(QDMA_CSR_GPON_RPT_CFG(base))) ;
	printk("QDMA CSR: 0x00C4 QDMA_CSR_GPON_TRTCM_CFG		:%.8x\n", IO_GREG(QDMA_CSR_GPON_TRTCM_CFG(base))) ;   
	printk("QDMA CSR: 0x00E0 QDMA_CSR_EPON_RPT_CFG			:%.8x\n", IO_GREG(QDMA_CSR_EPON_RPT_CFG(base))) ;
	printk("QDMA CSR: 0x00E4 QDMA_CSR_EPON_QTHRESHLD_CFG	:%.8x\n", IO_GREG(QDMA_CSR_EPON_QTHRESHLD_CFG(base))) ;   
	printk("QDMA CSR: 0x01E0 QDMA_CSR_DBG_TX_DSCP_SUM		:%.8x\n", IO_GREG(QDMA_CSR_DBG_TX_DSCP_SUM(base))) ;   
	printk("QDMA CSR: 0x01E4 QDMA_CSR_DBG_TX_PKT_SUM		:%.8x\n", IO_GREG(QDMA_CSR_DBG_TX_PKT_SUM(base))) ;   
	printk("QDMA CSR: 0x01E8 QDMA_CSR_DBG_RX_PKT_SUM		:%.8x\n", IO_GREG(QDMA_CSR_DBG_RX_PKT_SUM(base))) ;   
	printk("QDMA CSR: 0x01EC QDMA_CSR_DBG_LMGR_STA			:%.8x\n", IO_GREG(QDMA_CSR_DBG_LMGR_STA(base))) ;
#else
	printk("0x0000: QDMA_CSR_INFO                           :%.8x\n", IO_GREG(QDMA_CSR_INFO(base))) ;	    
	printk("0x0004: QDMA_CSR_GLB_CFG                        :%.8x\n", IO_GREG(QDMA_CSR_GLB_CFG(base))) ;	    
	printk("0x0008: QDMA_CSR_TX_DSCP_BASE	                :%.8x\n", IO_GREG(QDMA_CSR_TX_DSCP_BASE(base))) ;     
	printk("0x000C: QDMA_CSR_RX_DSCP_BASE	                :%.8x\n", IO_GREG(QDMA_CSR_RX_DSCP_BASE(base))) ; 
	printk("0x0010: QDMA_CSR_TX_CPU_IDX                     :%.8x\n", IO_GREG(QDMA_CSR_TX_CPU_IDX(base))) ;
	printk("0x0014: QDMA_CSR_TX_DMA_IDX                     :%.8x\n", IO_GREG(QDMA_CSR_TX_DMA_IDX(base))) ;
	printk("0x0018: QDMA_CSR_RX_CPU_IDX                     :%.8x\n", IO_GREG(QDMA_CSR_RX_CPU_IDX(base))) ;
	printk("0x001C: QDMA_CSR_RX_DMA_IDX                     :%.8x\n", IO_GREG(QDMA_CSR_RX_DMA_IDX(base))) ;
	printk("0x0020: QDMA_CSR_HWFWD_DSCP_BASE                :%.8x\n", IO_GREG(QDMA_CSR_HWFWD_DSCP_BASE(base))) ;
	printk("0x0024: QDMA_CSR_HWFWD_TX_IDX                   :%.8x\n", IO_GREG(QDMA_CSR_HWFWD_TX_IDX(base))) ;
	printk("0x0028: QDMA_CSR_HWFWD_RX_IDX                   :%.8x\n", IO_GREG(QDMA_CSR_HWFWD_RX_IDX(base))) ;
	printk("0x002C: QDMA_CSR_HWFWD_FREE_IDX                 :%.8x\n", IO_GREG(QDMA_CSR_HWFWD_FREE_IDX(base))) ;
	printk("0x0030: QDMA_CSR_HWFWD_AVAIL_DSCP_NUM           :%.8x\n", IO_GREG(QDMA_CSR_HWFWD_AVAIL_DSCP_NUM(base))) ;
	printk("0x0034: QDMA_CSR_HWFWD_USED_DSCP_NUM	        :%.8x\n", IO_GREG(QDMA_CSR_HWFWD_USED_DSCP_NUM(base))) ;
	printk("0x0040: QDMA_CSR_LMGR_CHNL_RETIRE               :%.8x\n", IO_GREG(QDMA_CSR_LMGR_CHNL_RETIRE(base))) ;
	printk("0x0044: QDMA_CSR_LMGR_CHNL_RETIRE_MASK          :%.8x\n", IO_GREG(QDMA_CSR_LMGR_CHNL_RETIRE_MASK(base))) ;
	printk("0x0050: QDMA_CSR_INT_STATUS                     :%.8x\n", IO_GREG(QDMA_CSR_INT_STATUS(base))) ;
	printk("0x0054: QDMA_CSR_INT_ENABLE                     :%.8x\n", IO_GREG(QDMA_CSR_INT_ENABLE(base))) ;
	printk("0x0058: QDMA_CSR_DELAY_INT_CFG                  :%.8x\n", IO_GREG(QDMA_CSR_DELAY_INT_CFG(base))) ;
	printk("0x0060: QDMA_CSR_IRQ_BASE    	                :%.8x\n", IO_GREG(QDMA_CSR_IRQ_BASE(base))) ;
	printk("0x0064: QDMA_CSR_IRQ_CFG    	                :%.8x\n", IO_GREG(QDMA_CSR_IRQ_CFG(base))) ;
	printk("0x0068: QDMA_CSR_IRQ_CLEAR_LEN                  :%.8x\n", IO_GREG(QDMA_CSR_IRQ_CLEAR_LEN(base))) ;
	printk("0x006C: QDMA_CSR_IRQ_STATUS	                    :%.8x\n", IO_GREG(QDMA_CSR_IRQ_STATUS(base))) ;
	printk("0x0070: QDMA_CSR_IRQ_PTIME		                :%.8x\n", IO_GREG(QDMA_CSR_IRQ_PTIME(base))) ;
	printk("0x0080: QDMA_CSR_TXQOS_CHN07_CFG		        :%.8x\n", IO_GREG(QDMA_CSR_TXQOS_CHN07_CFG(base))) ;
	printk("0x0084: QDMA_CSR_TXQOS_CHN815_CFG	            :%.8x\n", IO_GREG(QDMA_CSR_TXQOS_CHN815_CFG(base))) ;
	printk("0x0088: QDMA_CSR_TXQOS_WRR_CFG		            :%.8x\n", IO_GREG(QDMA_CSR_TXQOS_WRR_CFG(base))) ;
	printk("0x008C: QDMA_CSR_TXDMA_PREFETCH_CFG	            :%.8x\n", IO_GREG(QDMA_CSR_TXDMA_PREFETCH_CFG(base))) ;
	printk("0x0090: QDMA_CSR_TXBUF_THR_CFG		            :%.8x\n", IO_GREG(QDMA_CSR_TXBUF_THR_CFG(base))) ;
	printk("0x00A0: QDMA_CSR_TXQ_CNGST_CFG		            :%.8x\n", IO_GREG(QDMA_CSR_TXQ_CNGST_CFG(base))) ;
	printk("0x00A4: QDMA_CSR_TXQ_GRN_MAX_THRSHLD            :%.8x\n", IO_GREG(QDMA_CSR_TXQ_GRN_MAX_THRSHLD(base))) ;
	printk("0x00A8: QDMA_CSR_TXQ_GRN_MIN_THRSHLD            :%.8x\n", IO_GREG(QDMA_CSR_TXQ_GRN_MIN_THRSHLD(base))) ;
	printk("0x00AC: QDMA_CSR_TXQ_YLW_MAX_THRSHLD            :%.8x\n", IO_GREG(QDMA_CSR_TXQ_YLW_MAX_THRSHLD(base))) ;
	printk("0x00B0: QDMA_CSR_TXQ_YLW_MIN_THRSHLD            :%.8x\n", IO_GREG(QDMA_CSR_TXQ_YLW_MIN_THRSHLD(base))) ;
	printk("0x00B4: QDMA_CSR_TRTCM_CFG	                    :%.8x\n", IO_GREG(QDMA_CSR_TRTCM_CFG(base))) ;
	printk("0x00C0: QDMA_CSR_GPON_TCONT_COLOR_CFG           :%.8x\n", IO_GREG(QDMA_CSR_GPON_TCONT_COLOR_CFG(base))) ;
	printk("0x00C4: QDMA_CSR_GPON_TCONT_TRTCM_CFG           :%.8x\n", IO_GREG(QDMA_CSR_GPON_TCONT_TRTCM_CFG(base))) ;
	printk("0x00D0: QDMA_CSR_EPON_RPT_CFG                   :%.8x\n", IO_GREG(QDMA_CSR_EPON_RPT_CFG(base))) ;
	printk("0x00D4: QDMA_CSR_EPON_QTHRESHLD_CFG             :%.8x\n", IO_GREG(QDMA_CSR_EPON_QTHRESHLD_CFG(base))) ;
	printk("0x0100: QDMA_CSR_DBG_TX_CPU_DSCP_STAT           :%.8x\n", IO_GREG(QDMA_CSR_DBG_TX_CPU_DSCP_STAT(base))) ;			
	printk("0x0104: QDMA_CSR_DBG_TX_CPU_PKT_STAT            :%.8x\n", IO_GREG(QDMA_CSR_DBG_TX_CPU_PKT_STAT(base))) ;			
	printk("0x0108: QDMA_CSR_DBG_TX_FWD_DSCP_STAT           :%.8x\n", IO_GREG(QDMA_CSR_DBG_TX_FWD_DSCP_STAT(base))) ;	
	printk("0x010C: QDMA_CSR_DBG_TX_FWD_PKT_STAT            :%.8x\n", IO_GREG(QDMA_CSR_DBG_TX_FWD_PKT_STAT(base))) ;
	printk("0x0110: QDMA_CSR_DBG_RX_CPU_PKT_STAT            :%.8x\n", IO_GREG(QDMA_CSR_DBG_RX_CPU_PKT_STAT(base))) ;
	printk("0x0114: QDMA_CSR_DBG_RX_FWD_PKT_STAT            :%.8x\n", IO_GREG(QDMA_CSR_DBG_RX_FWD_PKT_STAT(base))) ;
	printk("0x0118: QDMA_CSR_DBG_LMGR_STA      	            :%.8x\n", IO_GREG(QDMA_CSR_DBG_LMGR_STA(base))) ;
	printk("0x011C: QDMA_CSR_DBG_QDMA_STATUS                :%.8x\n", IO_GREG(QDMA_CSR_DBG_QDMA_STATUS(base))) ;
	printk("0x0120: QDMA_CSR_DBG_RCDROP_FWD_GREEN_STAT	    :%.8x\n", IO_GREG(QDMA_CSR_DBG_RCDROP_FWD_GREEN_STAT(base))) ;
	printk("0x0124: QDMA_CSR_DBG_RCDROP_FWD_YELLOW_STAT     :%.8x\n", IO_GREG(QDMA_CSR_DBG_RCDROP_FWD_YELLOW_STAT(base))) ;
	printk("0x0128: QDMA_CSR_DBG_RCDROP_FWD_RED_STAT        :%.8x\n", IO_GREG(QDMA_CSR_DBG_RCDROP_FWD_RED_STAT(base))) ;
	printk("0x012C: QDMA_CSR_DBG_RCDROP_CPU_GREEN_STAT	    :%.8x\n", IO_GREG(QDMA_CSR_DBG_RCDROP_CPU_GREEN_STAT(base))) ;
	printk("0x0130: QDMA_CSR_DBG_RCDROP_CPU_YELLOW_STAT     :%.8x\n", IO_GREG(QDMA_CSR_DBG_RCDROP_CPU_YELLOW_STAT(base))) ;
	printk("0x0134: QDMA_CSR_DBG_RCDROP_CPU_RED_STAT        :%.8x\n", IO_GREG(QDMA_CSR_DBG_RCDROP_CPU_RED_STAT(base))) ;
	printk("0x0138: QDMA_CSR_DBG_RETDROP_STAT               :%.8x\n", IO_GREG(QDMA_CSR_DBG_RETDROP_STAT(base))) ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
}

/******************************************************************************
******************************************************************************/
static void __dump_dscp(void)
{
	struct QDMA_DscpInfo_S *diPtr ;
	int i, idx=1 ;

	idx = 1 ;
	diPtr = gpQdmaPriv->txHeadPtr ;
	printk("Unused Tx DSCP Link List:\n") ;
	
#ifdef MT7510_DMA_DSCP_CACHE
	dma_cache_inv((unsigned long)(gpQdmaPriv->txHeadPtr->dscpPtr), (CONFIG_TX_DSCP_NUM+CONFIG_RX_DSCP_NUM)*sizeof(QDMA_DMA_DSCP_T));
#endif /* MT7510_DMA_DSCP_CACHE */
	
	while(diPtr) {
		printk("%d: DSCP Idx:%d, DSCP Ptr:%.8x, Done Bit:%d\n", idx, diPtr->dscpIdx, 
																(uint)diPtr->dscpPtr, 
																diPtr->dscpPtr->ctrl.done) ;
		diPtr = diPtr->next ;
		idx++ ;
	} 
	
	idx = 1 ;
	printk("\nUsing Tx DSCP Set:\n") ;
	for(i=0 ; i<gpQdmaPriv->txDscpNum ; i++) {
		diPtr = (struct QDMA_DscpInfo_S *)gpQdmaPriv->dscpInfoAddr + i ;
		if(diPtr->next == NULL && diPtr!=gpQdmaPriv->txTailPtr) {
			printk("%d: DSCP Idx:%d, DSCP Ptr:%.8x, Done Bit:%d, MsgLen:%d, MsgAddr:%.8x(%.8x), PktLen:%d, PktAddr:%.8x(%.8x), Next Idx:%d\n", 
																idx, diPtr->dscpIdx, 
																(uint)diPtr->dscpPtr, 
																diPtr->dscpPtr->ctrl.done,
																diPtr->dscpPtr->ctrl.msg_len, 
																(uint)diPtr->msgPtr, 
																(uint)diPtr->dscpPtr->msg_addr, 
																diPtr->dscpPtr->ctrl.pkt_len, 
																(uint)diPtr->skb, 
																(uint)diPtr->dscpPtr->pkt_addr,
																diPtr->dscpPtr->next_idx) ;
			idx++ ;	
		}
		
		if((i&0xFF) == 0xFF) {
			msleep(0) ;
		}
	}
	
	idx = 1 ;
	diPtr = gpQdmaPriv->rxStartPtr ;
	printk("\nRx DSCP Ring: RxStartIdx:%d, RxEndIdx:%d\n", gpQdmaPriv->rxStartPtr->dscpIdx, gpQdmaPriv->rxEndPtr->dscpIdx) ;
	do {
		if(diPtr) {
			printk("%d: DSCP Idx:%d, DSCP Ptr:%.8x, Done Bit:%d, MsgLen:%d, MsgAddr:%.8x(%.8x), PktLen:%d, PktAddr:%.8x(%.8x), Next Idx:%d\n", 
																idx, diPtr->dscpIdx, 
																(uint)diPtr->dscpPtr, 
																diPtr->dscpPtr->ctrl.done,
																diPtr->dscpPtr->ctrl.msg_len, 
																(uint)diPtr->msgPtr, 
																(uint)diPtr->dscpPtr->msg_addr, 
																diPtr->dscpPtr->ctrl.pkt_len, 
																(uint)diPtr->skb, 
																(uint)diPtr->dscpPtr->pkt_addr,
																diPtr->dscpPtr->next_idx) ;
			diPtr = diPtr->next ;
			idx++ ;
		}
	} while(diPtr!=NULL && diPtr!=gpQdmaPriv->rxStartPtr) ;
}

/******************************************************************************
******************************************************************************/
static void __dump_irq_queue(void)
{
	int i ;
	uint *irqPtr, irqValue ;
	
	for(i=0 ; i<gpQdmaPriv->irqDepth ; i++) {
		irqPtr = (uint *)gpQdmaPriv->irqQueueAddr + i ;
		irqValue = *irqPtr ;
		printk("IRQ Queue:%.4x,     Content:%.8x\n", i, irqValue) ;
		
		if((i&0xFF) == 0xFF) {
			msleep(0) ;
		}
	}
}

/******************************************************************************
******************************************************************************/
static void __dump_hwfwd(void)
{
	int i ;
	uint base = gpQdmaPriv->csrBaseAddr ;
	QDMA_DMA_DSCP_T *pHwDscp ;
	uint hwTotalDscpSize = sizeof(QDMA_DMA_DSCP_T) * gpQdmaPriv->hwFwdDscpNum ;
	uint hwTotalMsgSize = CONFIG_HWFWD_MSG_LENS * gpQdmaPriv->hwFwdDscpNum ;
	uint *pHwMsg, *pHwPkt ;
	
	printk("Hardware Forwarding DSCP Link List:\n") ;
	for(i=0 ; i<gpQdmaPriv->hwFwdDscpNum ; i++) {
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
		pHwDscp = (QDMA_DMA_DSCP_T *)(gpQdmaPriv->hwFwdBaseAddr) + i ;
		pHwMsg = pHwDscp->msg ;
		pHwPkt = (uint *)(gpQdmaPriv->hwFwdBaseAddr + hwTotalDscpSize + hwTotalMsgSize) + (CONFIG_MAX_PKT_LENS/sizeof(uint))*i ;
#else
		pHwDscp = (QDMA_DMA_DSCP_T *)(gpQdmaPriv->hwFwdBaseAddr) + i ;
		pHwMsg = (uint *)(gpQdmaPriv->hwFwdBaseAddr + hwTotalDscpSize) + (CONFIG_HWFWD_MSG_LENS/sizeof(uint))*i ;
		pHwPkt = (uint *)(gpQdmaPriv->hwFwdBaseAddr + hwTotalDscpSize + hwTotalMsgSize) + (CONFIG_MAX_PKT_LENS/sizeof(uint))*i ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
		
		printk("%.4d: DSCP:%.8x(%.8x), Done:%d, MsgLen:%d, MsgAddr:%.8x(%.8x), PktLen:%d, PktAddr:%.8x(%.8x), Next:%d\n", 
															i, (uint)pHwDscp, 
															qdmaGetHwDscpBase(base)+i*sizeof(QDMA_DMA_DSCP_T),
															pHwDscp->ctrl.done,
															pHwDscp->ctrl.msg_len,
															(uint)pHwMsg,
															(uint)pHwDscp->msg_addr, 
															pHwDscp->ctrl.pkt_len,
															(uint)pHwPkt,
															(uint)pHwDscp->pkt_addr,
															pHwDscp->next_idx) ;
		printk("      Msg Content:%.8x, %.8x\n", (uint)(*pHwMsg), (uint)(*(pHwMsg+1))) ;

		if((i&0xFF) == 0xFF) {
			msleep(0) ;
		}
	}
}

/******************************************************************************
******************************************************************************/
static int qdma_bm_counters_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	int index=0 ;
	off_t pos=0, begin=0 ;

	index += sprintf(buf+ index, "Tx DSCP Counts:                    %u\n", gpQdmaPriv->counters.txCounts) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Tx Recycle DSCP Counts:            %u\n", gpQdmaPriv->counters.txRecycleCounts) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Tx QDMA Dropped Counts:            %u\n", gpQdmaPriv->counters.txQdmaDropCounts) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Rx DSCP Counts:                    %u\n", gpQdmaPriv->counters.rxCounts) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "IRQ Queue Asynchronous             %u\n", gpQdmaPriv->counters.IrqQueueAsynchronous) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "IRQ Queue Index Errors:            %u\n", gpQdmaPriv->counters.txIrqQueueIdxErrs) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Tx DSCP Content Incorrect:         %u\n", gpQdmaPriv->counters.txDscpIncorrect) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Tx Recycle Errors:                 %u\n", gpQdmaPriv->counters.txRecycleErrs) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Tx Done Bit Errors:                %u\n", gpQdmaPriv->counters.txDscpDoneErrs) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Rx DSCP Content Incorrect:         %u\n", gpQdmaPriv->counters.rxDscpIncorrect) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Rx Packet Errors:                  %u\n", gpQdmaPriv->counters.rxPktErrs) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "No Tx Callback Errors:             %u\n", gpQdmaPriv->counters.noTxCbErrs) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "No Tx DSCP Errors:                 %u\n", gpQdmaPriv->counters.noTxDscps) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "No Rx Callback Errors:             %u\n", gpQdmaPriv->counters.noRxCbErrs) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "No Rx DSCP Errors:                 %u\n", gpQdmaPriv->counters.noRxDscps) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Rx Chorent Interrupt:              %u\n", gpQdmaPriv->counters.intRxCoherent) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Tx Chorent Interrupt:              %u\n", gpQdmaPriv->counters.intTxCoherent) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "IRQ Queue Full Interrupt:          %u\n", gpQdmaPriv->counters.intIrqFull) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "No Link DSCP Interrupt:            %u\n", gpQdmaPriv->counters.intNoLinkDscp) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Rx Buffer Overrun Interrupt:       %u\n", gpQdmaPriv->counters.intRxBuffOverrun) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Tx Buffer Underrun Interrupt:      %u\n", gpQdmaPriv->counters.intTxBuffUnderrun) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "No Rx Buffer Interrupt:            %u\n", gpQdmaPriv->counters.intNoRxBuff) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "No Tx Buffer Interrupt:            %u\n", gpQdmaPriv->counters.intNoTxBuff) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "No Rx DSCP Interrupt:              %u\n", gpQdmaPriv->counters.intNoRxDscp) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "No Tx DSCP Interrupt:              %u\n", gpQdmaPriv->counters.intNoTxDscp) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Rx Done Interrupt:                 %u\n", gpQdmaPriv->counters.intRxDone) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "Tx Done Interrupt:                 %u\n", gpQdmaPriv->counters.intTxDone) ;
	CHK_BUF() ;
#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
	index += sprintf(buf+ index, "QDMA Tx DSCP Counts:               %u\n", qdmaGetDbgTxDscpSummary(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA Tx Packet Counts:             %u\n", qdmaGetDbgTxPktSummary(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA Rx Packet Counts:             %u\n", qdmaGetDbgRxPktSummary(base)) ;
	CHK_BUF() ;
#else
	index += sprintf(buf+ index, "QDMA HW Tx CPU DSCPs               %u\n", qdmaGetDbgTxCpuDscp(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Tx CPU Packets             %u\n", qdmaGetDbgTxCpuPacket(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Tx FWD DSCPs               %u\n", qdmaGetDbgTxFwdDscp(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Tx FWD Packets             %u\n", qdmaGetDbgTxFwdPacket(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Rx CPU Packets             %u\n", qdmaGetDbgRxCpuPacket(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Rx FWD Packets             %u\n", qdmaGetDbgRxFwdPacket(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Drop CPU Green Packets     %u\n", qdmaGetDbgRcDropCpuGreen(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Drop CPU Yellow Packets    %u\n", qdmaGetDbgRcDropCpuYellow(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Drop CPU Red Packets       %u\n", qdmaGetDbgRcDropCpuRed(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Drop FWD Green Packets     %u\n", qdmaGetDbgRcDropFwdGreen(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Drop FWD Yellow Packets    %u\n", qdmaGetDbgRcDropFwdYellow(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Drop FWD Red Packets       %u\n", qdmaGetDbgRcDropFwdRed(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Retire Drop Packets        %u\n", qdmaGetDbgRetDrop(base)) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA HW Status:                    0x%.8x\n", qdmaGetDbgQdmaStatus(base)) ;
	CHK_BUF() ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
	index += sprintf(buf+ index, "QDMA Is LinkMgr Busy:              %u\n", (qdmaIsLmgrBusy(base))?1:0) ;
	CHK_BUF() ;
	index += sprintf(buf+ index, "QDMA LinkMgr Free Counts:          %u\n", qdmaLmgrFreeCount(base)) ;
	CHK_BUF() ;

	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0)		index = 0 ;
	if(index>count)		index = count ;
	return index ;
}

/******************************************************************************
******************************************************************************/
static int qdma_bm_counters_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[64] ;
	uint cmd ;
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;

	sscanf(val_string, "%d", &cmd) ;

	if(cmd == 1) {
		memset(&gpQdmaPriv->counters, 0, sizeof(BM_Counters_T)) ;

#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
		qdmaSetDbgTxDscpSummary(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgTxPktSummary(gpQdmaPriv->csrBaseAddr, 0)	;
		qdmaSetDbgRxPktSummary(gpQdmaPriv->csrBaseAddr, 0)	;
#else
		qdmaSetDbgTxCpuDscp(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgTxCpuPacket(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgTxFwdDscp(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgTxFwdPacket(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgRxCpuPacket(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgRxFwdPacket(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgRcDropFwdGreen(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgRcDropFwdYellow(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgRcDropFwdRed(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgRcDropCpuGreen(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgRcDropCpuYellow(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgRcDropCpuRed(gpQdmaPriv->csrBaseAddr, 0) ;
		qdmaSetDbgRetDrop(gpQdmaPriv->csrBaseAddr, 0) ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
	}
	
	return count ;
}

/******************************************************************************
******************************************************************************/
static int qdma_bm_debug_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{

	return 0 ;
}

/******************************************************************************
******************************************************************************/
static int qdma_bm_debug_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[64], cmd[32], subcmd[32] ;
	uint value ;
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;

	sscanf(val_string, "%s %s", cmd, subcmd) ;

#ifdef CONFIG_DEBUG
	if(!strcmp(cmd, "level")) {
		value = subcmd[0] - 48 ;
		if(value <= DEG_MSG) {
			gpQdmaPriv->devCfg.dbgLevel = value ;
		}
		printk("Debug Level: %d\n", gpQdmaPriv->devCfg.dbgLevel) ;		
	} else if(!strcmp(cmd, "dump")) {
		if(!strcmp(subcmd, "csr")) {
			__dump_csr() ;
		} else if(!strcmp(subcmd, "dscp")) {
			__dump_dscp() ;
		} else if(!strcmp(subcmd, "irq")) {
			__dump_irq_queue() ;
		} else if(!strcmp(subcmd, "hwfwd")) {
			__dump_hwfwd() ;
		}
	} else if(!strcmp(cmd, "test")) {
		if(!strcmp(subcmd, "wrr")) {
			int i, j ;
			QDMA_TxQosScheduler_T txQos ;
			
			for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
				txQos.channel = i ;
				txQos.qosType = i%QDMA_TXQOS_TYPE_NUMS ;
				for(j=0 ; j<CONFIG_QDMA_QUEUE ; j++) {
					txQos.queue[j].weight = i*10+j ;
				}
				qdma_set_tx_qos(&txQos) ;
			}
			
			for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
				memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T)) ;
				
				txQos.channel = i ;
				qdma_get_tx_qos(&txQos) ;
				printk("Channel:%d, Type:%d, Q0:%d, Q1:%d, Q2:%d, Q3:%d, Q4:%d, Q5:%d, Q6:%d, Q7:%d\n",
									i, txQos.qosType, 
									txQos.queue[0].weight,
									txQos.queue[1].weight,
									txQos.queue[2].weight,
									txQos.queue[3].weight,
									txQos.queue[4].weight,
									txQos.queue[5].weight,
									txQos.queue[6].weight,
									txQos.queue[7].weight) ;									
			}
		}
	}
#endif /* CONFIG_DEBUG */

	return count ;
}

/******************************************************************************
 QDMA setting and configuration function
******************************************************************************/
#ifndef CONFIG_TX_POLLING_BY_MAC
/******************************************************************************
******************************************************************************/
static int qdma_bm_polling_tx_recycle(void) 
{
	while(gpQdmaPriv->devCfg.flags.isTxPolling == QDMA_ENABLE) {
		qdma_bm_transmit_done(0) ;
		msleep(2000) ;
	}
	
	return 0 ;
}
#endif /* CONFIG_TX_POLLING_BY_MAC */

/******************************************************************************
******************************************************************************/
int qdma_bm_tx_polling_mode(QDMA_Mode_t txMode, unchar txThreshold)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(txMode == QDMA_ENABLE) {
		gpQdmaPriv->devCfg.txRecycleThreshold = txThreshold ;
		
		if(gpQdmaPriv->devCfg.flags.isTxPolling == QDMA_DISABLE) {
			qdmaDisableInt(base, (INT_MASK_TX_DONE|INT_STATUS_IRQ_FULL)) ;
		
			gpQdmaPriv->devCfg.flags.isTxPolling = txMode ;
#ifndef CONFIG_TX_POLLING_BY_MAC
			kernel_thread((int (*)(void *))qdma_bm_polling_tx_recycle, NULL, 0) ;
#endif /* CONFIG_TX_POLLING_BY_MAC */
		}
	} else {
		if(gpQdmaPriv->devCfg.flags.isTxPolling == QDMA_ENABLE) {
			gpQdmaPriv->devCfg.flags.isTxPolling = txMode ;
			qdmaEnableInt(base, (INT_MASK_TX_DONE|INT_STATUS_IRQ_FULL)) ;
		}
	}
	
	return 0 ;
}

/*****************************************************************
*****************************************************************/
static void qdma_bm_task(unsigned long data)
{
	uint quota = 128 ; 
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(qdma_bm_receive_packets(quota) < quota) {
		qdmaEnableInt(base, (INT_MASK_NO_RX_CPU_DSCP|INT_MASK_RX_DONE)) ;
	} else {
		tasklet_schedule(&gpQdmaPriv->task) ;
	}
}

/*****************************************************************
*****************************************************************/
static irqreturn_t qdma_isr(int irq, void *dev_id)
{
	QDMA_DMA_DSCP_T *pTxDscp, *pRxDscp ;
	uint intStatus, intMask ;
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint idx ;
	
	intMask = qdmaGetIntMask(base) ;
	intStatus = qdmaGetIntStatus(base) & intMask ;
	QDMA_MSG(DBG_WARN, "QDMA interrupt service routine is return, Status:%.8x, Mask:%.8x\n", qdmaGetIntStatus(base), qdmaGetIntMask(base)) ;
	
	if(intStatus & INT_STATUS_QDMA_DONE) {
		if(intStatus & INT_STATUS_RX_DONE) {
			qdmaClearIntStatus(base, INT_STATUS_RX_DONE) ;
			if(gpQdmaPriv->devCfg.flags.isRxNapi == QDMA_ENABLE) {
				if(gpQdmaPriv->devCfg.bmEventCallbackFunction) {
					gpQdmaPriv->devCfg.bmEventCallbackFunction(QDMA_EVENT_RECV_PKTS) ;
				}
			} else {
				qdmaDisableInt(base, (INT_MASK_NO_RX_CPU_DSCP|INT_MASK_RX_DONE)) ;
				tasklet_schedule(&gpQdmaPriv->task) ;
			}
			gpQdmaPriv->counters.intRxDone++ ;
			QDMA_MSG(DBG_WARN, "IRQ: rx DSCP DONE interrupt.\n") ;
		}
		
		if(intStatus & INT_STATUS_TX_DONE) {
			qdmaClearIntStatus(base, INT_STATUS_TX_DONE) ;
			qdma_bm_transmit_done(0) ;
			gpQdmaPriv->counters.intTxDone++ ;
			QDMA_MSG(DBG_WARN, "IRQ: tx DSCP DONE interrupt\n") ;
		}
	}

	if(intStatus & INT_STATUS_EXTERNAL) {
		if((intStatus & INT_STATUS_XPON_PHY) && gpQdmaPriv->devCfg.bmXponPhyIntHandler) {
			QDMA_MSG(DBG_WARN, "IRQ: External XPON PHY device interrupt\n") ;
			qdmaClearIntStatus(base, INT_STATUS_XPON_PHY) ;
			gpQdmaPriv->devCfg.bmXponPhyIntHandler() ;
		} 
		if((intStatus & INT_STATUS_ATM_SAR) && gpQdmaPriv->devCfg.bmSarIntHandler) {
			QDMA_MSG(DBG_WARN, "IRQ: External ATM SAR device interrupt\n") ;
			qdmaClearIntStatus(base, INT_STATUS_ATM_SAR) ;
			gpQdmaPriv->devCfg.bmSarIntHandler() ;
		} 
		if((intStatus & INT_STATUS_PTM) && gpQdmaPriv->devCfg.bmPtmIntHandler) {
			QDMA_MSG(DBG_WARN, "IRQ: External PTM device interrupt\n") ;
			qdmaClearIntStatus(base, INT_STATUS_PTM) ;
			gpQdmaPriv->devCfg.bmPtmIntHandler() ;
		} 
		if((intStatus & INT_STATUS_EPON_MAC) && gpQdmaPriv->devCfg.bmEponMacIntHandler) {
			QDMA_MSG(DBG_WARN, "IRQ: External EPON MAC interrupt\n") ;
			qdmaClearIntStatus(base, INT_STATUS_EPON_MAC) ;
			gpQdmaPriv->devCfg.bmEponMacIntHandler() ;
		} 
		if((intStatus & INT_STATUS_GPON_MAC) && gpQdmaPriv->devCfg.bmGponMacIntHandler) {
			QDMA_MSG(DBG_WARN, "IRQ: External GPON MAC interrupt\n") ;
			qdmaClearIntStatus(base, INT_STATUS_GPON_MAC) ;
			gpQdmaPriv->devCfg.bmGponMacIntHandler() ;
		}
	}
	
	if(intStatus & INT_STATUS_QDMA_FAULT) {
		if(intStatus & INT_STATUS_RX_COHERENT) {
			QDMA_ERR("IRQ: RX_DMA finds data coherent event when checking DONE bit, RX_DMA_IDX:%d.\n", qdmaGetRxDmaIdx(base)) ;
			qdmaClearIntStatus(base, INT_STATUS_RX_COHERENT) ;
#ifdef CONFIG_SUPPORT_SELF_TEST
			qdmaDisableRxDma(base) ;
			idx = qdmaGetRxDmaIdx(base) ;
			if(idx>=0 && idx<gpQdmaPriv->rxDscpNum) {
				pRxDscp = ((struct QDMA_DscpInfo_S *)gpQdmaPriv->dscpInfoAddr + (idx+gpQdmaPriv->txDscpNum))->dscpPtr ;
				pRxDscp->ctrl.done = 0 ;
#ifdef MT7510_DMA_DSCP_CACHE
				/*Write back DSCP*/
				//dma_cache_wback_inv((unsigned long)pRxDscp, CACHE_LINE_SIZE);
				protected_cache_op(Hit_Writeback_Inv_D, ((unsigned long)(pRxDscp)));
#endif /* MT7510_DMA_DSCP_CACHE */
			}
			qdmaEnableRxDma(base) ;
#endif /* CONFIG_SUPPORT_SELF_TEST */
			gpQdmaPriv->counters.intRxCoherent++ ;
		}
		
		if(intStatus & INT_STATUS_TX_COHERENT) {
			QDMA_ERR("IRQ: TX_DMA finds data coherent event when checking DONE bit, TX_DMA_IDX:%d.\n", qdmaGetTxDmaIdx(base)) ;
			qdmaClearIntStatus(base, INT_STATUS_TX_COHERENT) ;
#ifdef CONFIG_SUPPORT_SELF_TEST
			qdmaDisableTxDma(base) ;
			idx = qdmaGetTxDmaIdx(base) ;
			if(idx>=0 && idx<gpQdmaPriv->txDscpNum) {
				pTxDscp = ((struct QDMA_DscpInfo_S *)gpQdmaPriv->dscpInfoAddr + idx)->dscpPtr ;
				pTxDscp->ctrl.done = 0 ;
#ifdef MT7510_DMA_DSCP_CACHE
				/*Write back DSCP*/
				//dma_cache_wback_inv((unsigned long)pTxDscp, CACHE_LINE_SIZE);
				protected_cache_op(Hit_Writeback_Inv_D, ((unsigned long)(pTxDscp)));
#endif /* MT7510_DMA_DSCP_CACHE */
			}
			qdmaEnableTxDma(base) ;
#endif /* CONFIG_SUPPORT_SELF_TEST */
			gpQdmaPriv->counters.intTxCoherent++ ;
		}
		
		if(intStatus & INT_STATUS_IRQ_FULL) {
			qdmaClearIntStatus(base, INT_STATUS_IRQ_FULL) ;
//			qdma_bm_transmit_done(0) ;
			gpQdmaPriv->counters.intIrqFull++ ;
			QDMA_ERR("IRQ: IRQ full interrupt, entryLen:%d.\n", qdmaGetIrqEntryLen(base)) ;
		}
		
		if(intStatus & INT_STATUS_NO_LINK_DSCP) {
			//QDMA_ERR("IRQ: NO HW Link manger descriptor available.\n") ;
			qdmaClearIntStatus(base, INT_STATUS_NO_LINK_DSCP) ;
			if(gpQdmaPriv->devCfg.bmEventCallbackFunction) {
				gpQdmaPriv->devCfg.bmEventCallbackFunction(QDMA_EVENT_TX_CROWDED) ;
			} 
			gpQdmaPriv->counters.intNoLinkDscp++ ;
		}

		if(intStatus & INT_STATUS_RX_BUF_OVRN) {
			QDMA_ERR("IRQ: Rx Buffer over-run occurred.\n") ;
			gpQdmaPriv->counters.intRxBuffOverrun++ ;
		}
		
		if(intStatus & INT_STATUS_TX_BUF_UDRN) {
			QDMA_ERR("IRQ: Tx Buffer under-run occurred..\n") ;
			gpQdmaPriv->counters.intTxBuffUnderrun++ ;
		}
		
		if(intStatus & INT_STATUS_NO_RX_BUF) {
			QDMA_ERR("IRQ: No Rx on-chip buffer available.\n") ;
			gpQdmaPriv->counters.intNoRxBuff++ ;
		}
		
		if(intStatus & INT_STATUS_NO_TX_BUF) {
			QDMA_ERR("IRQ: No Tx on-chip buffer available.\n") ;
			gpQdmaPriv->counters.intNoTxBuff++ ;
		}

		if(intStatus & INT_STATUS_NO_RX_CPU_DSCP) {
			QDMA_MSG(DBG_WARN, "IRQ: NO RX CPU descriptor available interrupt.\n") ;
			qdmaClearIntStatus(base, INT_STATUS_NO_RX_CPU_DSCP) ;
			if(gpQdmaPriv->rxStartPtr == gpQdmaPriv->rxEndPtr) {
				/* need inform to high layer driver */ ;
				if(gpQdmaPriv->devCfg.bmEventCallbackFunction) {
					gpQdmaPriv->devCfg.bmEventCallbackFunction(QDMA_EVENT_NO_RX_BUFFER) ;
				} 
			} 
			gpQdmaPriv->counters.intNoRxDscp++ ;
		}
		
		if(intStatus & INT_STATUS_NO_TX_CPU_DSCP) {
			QDMA_ERR("IRQ: NO TX CPU descriptor available interrupt.\n") ;
			qdmaClearIntStatus(base, INT_STATUS_NO_TX_CPU_DSCP) ;
			gpQdmaPriv->counters.intNoTxDscp++ ;
		}
	}
	
	return IRQ_HANDLED ;
}

#ifdef MT7510_DMA_DSCP_CACHE
/*****************************************************************
*****************************************************************/
void qdmaSetDMADescrCacheReg(uint txDscpNum, uint rxDscpNum,uint dscpBaseAddr, uint dscpPhyAddr)
{
	uint txdscp_paddr_start, txdscp_vaddr_start, txdscp_vaddr_end; 
	uint rxdscp_paddr_start, rxdscp_vaddr_start, rxdscp_vaddr_end;


	txdscp_paddr_start = dscpPhyAddr;
	txdscp_vaddr_start = dscpBaseAddr;
	txdscp_vaddr_end = dscpBaseAddr + txDscpNum*sizeof(QDMA_DMA_DSCP_T) - 4;
	
	rxdscp_paddr_start = dscpPhyAddr + (txDscpNum*sizeof(QDMA_DMA_DSCP_T)/2);
	rxdscp_vaddr_start = dscpBaseAddr + txDscpNum*sizeof(QDMA_DMA_DSCP_T);
	rxdscp_vaddr_end = dscpBaseAddr + (txDscpNum+rxDscpNum)*sizeof(QDMA_DMA_DSCP_T) - 4;

	txdscp_vaddr_start = TO_CACHE_LINE_VADDR(txdscp_vaddr_start);
	txdscp_vaddr_end = TO_CACHE_LINE_VADDR(txdscp_vaddr_end);
	rxdscp_vaddr_start = TO_CACHE_LINE_VADDR(rxdscp_vaddr_start);
	rxdscp_vaddr_end = TO_CACHE_LINE_VADDR(rxdscp_vaddr_end);

	//printk("uncache_dscp_base=%x\n",uncache_dscp_base);
	//printk("txdscp_vaddr_start=%x\n",txdscp_vaddr_start);
	//printk("txdscp_vaddr_end=%x\n",txdscp_vaddr_end);
	//printk("txdscp_paddr_start=%x\n",txdscp_paddr_start);
	//printk("rxdscp_vaddr_start=%x\n",rxdscp_vaddr_start);
	//printk("rxdscp_vaddr_end=%x\n",rxdscp_vaddr_end);
	//printk("rxdscp_paddr_start=%x\n",rxdscp_paddr_start);
	msleep(10);
	
	/* init dscp cache line register */
	IO_SREG((TXDSP_PADDR_START_BASE), txdscp_paddr_start);
	IO_SREG((TXDSP_VADDR_START_BASE), txdscp_vaddr_start);
	IO_SREG((TXDSP_VADDR_END_BASE), txdscp_vaddr_end);
	IO_SREG((RXDSP_PADDR_START_BASE), rxdscp_paddr_start);
	IO_SREG((RXDSP_VADDR_START_BASE), rxdscp_vaddr_start);
	IO_SREG((RXDSP_VADDR_END_BASE), rxdscp_vaddr_end);
}

/*****************************************************************
*****************************************************************/
void qdmaClearDMADescrCacheReg(void){
	IO_SREG((TXDSP_PADDR_START_BASE), 0xffffffff);
	IO_SREG((TXDSP_VADDR_START_BASE), 0xffffffff);
	IO_SREG((TXDSP_VADDR_END_BASE), 0xffffffff);
	IO_SREG((RXDSP_PADDR_START_BASE), 0xffffffff);
	IO_SREG((RXDSP_VADDR_START_BASE), 0xffffffff);
	IO_SREG((RXDSP_VADDR_END_BASE), 0xffffffff);
	
}

static uint dscpCacheBaseAddr = NULL;

#endif /* MT7510_DMA_DSCP_CACHE */

/******************************************************************************
******************************************************************************/
static int qdma_bm_dscp_init(uint txDscpNum, uint rxDscpNum, uint hwDscpNum, uint irqDepth, uint msgLen, uint pktLen)
{
	struct QDMA_DscpInfo_S *diPtr ;
	QDMA_DMA_DSCP_T *pHwDscp ;
	dma_addr_t dscpDmaAddr, irqDmaAddr, hwFwdDmaAddr ;
	uint dscpBaseAddr, hwTotalDscpSize, hwTotalMsgSize, hwTotalPktSize ;
	uint base ;
	uint i ;

	base = gpQdmaPriv->csrBaseAddr ;
	
	if((txDscpNum>4095 || txDscpNum<=0) || (rxDscpNum>2048 || rxDscpNum<=0) || hwDscpNum>=4096 || irqDepth>4095 || (pktLen>CONFIG_MAX_PKT_LENS || pktLen<64)) {
		QDMA_ERR("The initial parameters are invalid.\n") ;
		return -EFAULT ;
	}
	if((pktLen & 0x3) != 0) {
		QDMA_ERR("The max packets lens must be a multiple of 4.\n") ;
		return -EFAULT ;
	}
	
	gpQdmaPriv->txDscpNum = txDscpNum ;
	gpQdmaPriv->rxDscpNum = rxDscpNum ;
	gpQdmaPriv->hwFwdDscpNum = hwDscpNum ;
	gpQdmaPriv->irqDepth =irqDepth ;
	
	/******************************************
	* Allocate descriptor DMA memory          *
	*******************************************/
#ifdef MT7510_DMA_DSCP_CACHE
	/*Allocate 3*space, 1st space for hardware DMA, 2nd and 3rd for virutal space mapping*/
	dscpCacheBaseAddr = (uint) dma_alloc_noncoherent(NULL, sizeof(QDMA_DMA_DSCP_T)*((txDscpNum+rxDscpNum)/2*3), &dscpDmaAddr, GFP_KERNEL);
	if(!dscpCacheBaseAddr) {
		QDMA_ERR("Allocate memory for TX/RX DSCP failed.\n") ; 
		return -ENOMEM ;
	}
	dma_cache_wback_inv((unsigned long)dscpCacheBaseAddr, sizeof(QDMA_DMA_DSCP_T)*((txDscpNum+rxDscpNum)/2*3));
	dscpBaseAddr = (uint)(dscpCacheBaseAddr + sizeof(QDMA_DMA_DSCP_T)*((txDscpNum+rxDscpNum)/2));

	/*Init DSCP cache register*/
	qdmaSetDMADescrCacheReg(txDscpNum, rxDscpNum, dscpBaseAddr, dscpDmaAddr);

	//Set the TX_DSCP_BASE and RX_DSCP_BASE address
	qdmaSetTxDscpBase(base, dscpDmaAddr) ;
	qdmaSetRxDscpBase(base, (dscpDmaAddr + txDscpNum*sizeof(QDMA_DMA_DSCP_T)/2)) ;	
#else
	dscpBaseAddr = (uint)dma_alloc_coherent(NULL, sizeof(QDMA_DMA_DSCP_T)*(txDscpNum+rxDscpNum), &dscpDmaAddr, GFP_KERNEL) ;
	if(!dscpBaseAddr) {
		QDMA_ERR("Allocate memory for TX/RX DSCP failed.\n") ; 
		return -ENOMEM ;
	}

	//Set the TX_DSCP_BASE and RX_DSCP_BASE address
	qdmaSetTxDscpBase(base, dscpDmaAddr) ;
	qdmaSetRxDscpBase(base, (dscpDmaAddr + txDscpNum*sizeof(QDMA_DMA_DSCP_T))) ;
#endif /* MT7510_DMA_DSCP_CACHE */

	/******************************************
	* Allocate memory for IRQ queue           *
	******************************************/
	if(irqDepth) {
		gpQdmaPriv->irqQueueAddr = (uint)dma_alloc_coherent(NULL, 4*irqDepth, &irqDmaAddr, GFP_KERNEL) ;
		if(!gpQdmaPriv->irqQueueAddr) {
			QDMA_ERR("Allocate memory for IRQ queue failed.\n") ;
			return -ENOMEM ;
		}
		memset((void *)gpQdmaPriv->irqQueueAddr, CONFIG_IRQ_DEF_VALUE, 4*irqDepth) ;
	
		/* Setting the IRQ queue information to QDMA register */
		qdmaSetIrqBase(base, irqDmaAddr) ;
		qdmaSetIrqDepth(base, irqDepth) ;
	}
	
	/***************************************************
	* Allocate memory for TX/RX DSCP Information node  *
	****************************************************/
	gpQdmaPriv->dscpInfoAddr = (uint)kmalloc(sizeof(struct QDMA_DscpInfo_S)*(txDscpNum+rxDscpNum), GFP_KERNEL) ;
	if(!gpQdmaPriv->dscpInfoAddr) {
		QDMA_ERR("Alloc memory for TX/RX DSCP information node failed\n") ;
		return -ENOMEM ;
	} 
	
	//Create unused tx descriptor link list and using rx descriptor ring
	for(i=0 ; i<(txDscpNum+rxDscpNum) ; i++) {
		diPtr = (struct QDMA_DscpInfo_S *)gpQdmaPriv->dscpInfoAddr + i ;
		diPtr->dscpPtr = (QDMA_DMA_DSCP_T *)dscpBaseAddr + i ;
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT		
		diPtr->dscpPtr->msg_addr = (uint)dscpDmaAddr + sizeof(QDMA_DMA_DSCP_T)*i + QDMA_DSCP_MSG_OFFSET ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */

		if(i < txDscpNum) {
			diPtr->dscpIdx = i ;
			diPtr->next = NULL ;
			qdma_bm_push_tx_dscp(diPtr) ;
		} else {
			diPtr->dscpIdx = i - txDscpNum ;
			qdma_bm_add_rx_dscp(diPtr) ;
		}
	}	
		
	/***************************************************
	* Initialization first DSCP for TxDMA              *
	****************************************************/
	diPtr = qdma_bm_pop_tx_dscp() ;
	if(!diPtr) {
		QDMA_ERR("There is not any free TX DSCP.\n") ; 
		return -ENOSR ;
	}
	gpQdmaPriv->txUsingPtr = diPtr ;
	qdmaSetTxCpuIdx(base, diPtr->dscpIdx) ;
	qdmaSetTxDmaIdx(base, diPtr->dscpIdx) ;
	
	/***************************************************
	* Initialization first DSCP for RxDMA              *
	****************************************************/
	diPtr = qdma_bm_get_unused_rx_dscp() ;
	if(diPtr == NULL) {
		QDMA_ERR("There is not any free RX DSCP.\n") ;
		return -ENOSR ;
	} 
	gpQdmaPriv->rxUsingPtr = diPtr ;
	qdmaSetRxCpuIdx(base, diPtr->dscpIdx) ;
	qdmaSetRxDmaIdx(base, diPtr->dscpIdx) ;

	/***************************************************
	* Initialization DSCP for hardware forwarding      *
	****************************************************/
	if(hwDscpNum) {
		hwTotalDscpSize = sizeof(QDMA_DMA_DSCP_T) * hwDscpNum ;
		hwTotalMsgSize = msgLen * hwDscpNum ;
#ifdef CONFIG_RX_2B_OFFSET
		hwTotalPktSize = (pktLen+4) * hwDscpNum ;
#else
		hwTotalPktSize = pktLen * hwDscpNum ;
#endif /* CONFIG_RX_2B_OFFSET */

		gpQdmaPriv->hwFwdBaseAddr = (uint)dma_alloc_coherent(NULL, (hwTotalDscpSize+hwTotalMsgSize+hwTotalPktSize), &hwFwdDmaAddr, GFP_KERNEL) ;
		if(!gpQdmaPriv->hwFwdBaseAddr) {
			QDMA_ERR("Allocate memory for hardware forwarding failed.\n") ;
			return -ENOMEM ;
		}

		for(i=0 ; i<hwDscpNum ; i++) {
			pHwDscp = (QDMA_DMA_DSCP_T *)gpQdmaPriv->hwFwdBaseAddr + i ;
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
			pHwDscp->msg_addr = hwFwdDmaAddr + sizeof(QDMA_DMA_DSCP_T)*i + QDMA_DSCP_MSG_OFFSET ;
			pHwDscp->ctrl.msg_len = QDMA_DSCP_MSG_LENS ;
#else
			pHwDscp->msg_addr = (hwFwdDmaAddr + hwTotalDscpSize) + msgLen*i ;
			pHwDscp->ctrl.msg_len = msgLen ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
#if (defined CONFIG_RX_2B_OFFSET && !defined CONFIG_NEW_QDMA_CSR_OFFSET)
			pHwDscp->pkt_addr = (hwFwdDmaAddr + hwTotalDscpSize + hwTotalMsgSize) + (pktLen+4)*i + 2 ;
#else
			pHwDscp->pkt_addr = (hwFwdDmaAddr + hwTotalDscpSize + hwTotalMsgSize) + pktLen*i ;
#endif /* CONFIG_RX_2B_OFFSET */
			pHwDscp->next_idx = (i+1) % hwDscpNum ;
			pHwDscp->ctrl.pkt_len = pktLen ;
			pHwDscp->ctrl.done = 0 ;
		}
		
		qdmaSetHwDscpBase(base, hwFwdDmaAddr) ;
		qdmaSetHwDscpNum(base, hwDscpNum)	;
		qdmaSetHwTxIdx(base, 0) ;
		qdmaSetHwRxIdx(base, 0) ;
		qdmaSetHwFreeIdx(base, (hwDscpNum-1))	; //It's used to fill with tail descriptor.
	}
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
static int qdma_bm_dscp_deinit(void) 
{
	struct QDMA_DscpInfo_S *diPtr ;
	uint totalDscpNum, base ;
	int i ;

	base = gpQdmaPriv->csrBaseAddr ;
	totalDscpNum = gpQdmaPriv->txDscpNum + gpQdmaPriv->rxDscpNum ;
	
	qdmaDisableTxDma(base) ;
	qdmaDisableRxDma(base) ;
	qdmaDisableHwFwdDma(base) ;

	if(gpQdmaPriv->dscpInfoAddr) {
		for(i=0 ; i<totalDscpNum ; i++) {
			diPtr = (struct QDMA_DscpInfo_S *)gpQdmaPriv->dscpInfoAddr + i ;
			
#ifndef TCSUPPORT_MERGED_DSCP_FORMAT
			if(diPtr->msgPtr && diPtr->dscpPtr->msg_addr) {
				dma_unmap_single(NULL, diPtr->dscpPtr->msg_addr, (size_t)diPtr->dscpPtr->ctrl.msg_len, DMA_BIDIRECTIONAL) ;
			}
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
			if(diPtr->skb && diPtr->dscpPtr->pkt_addr) {
				dma_unmap_single(NULL, diPtr->dscpPtr->pkt_addr, (size_t)diPtr->dscpPtr->ctrl.pkt_len, DMA_BIDIRECTIONAL) ;
			}
		}

		diPtr = (struct QDMA_DscpInfo_S *)gpQdmaPriv->dscpInfoAddr ;
		if(diPtr->dscpPtr) {
#ifdef MT7510_DMA_DSCP_CACHE
			qdmaClearDMADescrCacheReg();
			dma_free_noncoherent(NULL, sizeof(QDMA_DMA_DSCP_T)*(totalDscpNum/2*3), (void *)dscpCacheBaseAddr, qdmaGetTxDscpBase(gpQdmaPriv->csrBaseAddr)) ;
#else
			dma_free_coherent(NULL, sizeof(QDMA_DMA_DSCP_T)*totalDscpNum, (void *)diPtr->dscpPtr, qdmaGetTxDscpBase(gpQdmaPriv->csrBaseAddr)) ;
#endif /* MT7510_DMA_DSCP_CACHE */
			diPtr->dscpPtr = NULL ;
		}
	
		kfree(diPtr) ;
		gpQdmaPriv->dscpInfoAddr = 0 ;
	}
	
	if(gpQdmaPriv->irqQueueAddr) {
		dma_free_coherent(NULL, 4*gpQdmaPriv->irqDepth, (void *)gpQdmaPriv->irqQueueAddr, qdmaGetIrqBase(gpQdmaPriv->csrBaseAddr)) ;
		gpQdmaPriv->irqQueueAddr = 0 ;
	}
	
	if(gpQdmaPriv->hwFwdBaseAddr) {
		uint maxSize = (CONFIG_HWFWD_MSG_LENS + CONFIG_MAX_PKT_LENS + sizeof(QDMA_DMA_DSCP_T)) * qdmaGetHwDscpNum(gpQdmaPriv->csrBaseAddr) ;
		
		dma_free_coherent(NULL, maxSize, (void *)gpQdmaPriv->hwFwdBaseAddr, qdmaGetHwDscpBase(gpQdmaPriv->csrBaseAddr)) ;
		gpQdmaPriv->hwFwdBaseAddr = 0 ;
	}
	
	return 0 ;
}

/******************************************************************************
******************************************************************************/
static void __exit qdma_module_cleanup(void) 
{   
	qdma_dvt_deinit() ;
	remove_proc_entry("qdma/counters", NULL) ;
	remove_proc_entry("qdma/debug", NULL) ;
	remove_proc_entry("qdma", NULL) ;
	
	if(gpQdmaPriv != NULL) {
		if(gpQdmaPriv->devCfg.flags.isTxPolling == QDMA_ENABLE) {
			gpQdmaPriv->devCfg.flags.isTxPolling = QDMA_DISABLE ;
			msleep(100) ;
		}
		
		qdma_bm_dscp_deinit() ;
	
		if(gpQdmaPriv->csrBaseAddr) {
			iounmap((uint *)gpQdmaPriv->csrBaseAddr) ;
			gpQdmaPriv->csrBaseAddr = 0 ;
		}
	
		if(gpQdmaPriv->devCfg.flags.isIsrRequest) {
			free_irq(CONFIG_QDMA_IRQ, NULL) ;
		}
			
		kfree(gpQdmaPriv) ;
		gpQdmaPriv = NULL ;
	}
}

/******************************************************************************
******************************************************************************/
static int __init qdma_module_init(void)
{
	int ret ;
	struct proc_dir_entry *qdma_proc;

	/* Initial device private data */
	gpQdmaPriv = (QDMA_Private_T *)kmalloc(sizeof(QDMA_Private_T), GFP_KERNEL) ;
	if(gpQdmaPriv == NULL) {
		QDMA_ERR("Alloc private data memory failed\n") ;
		return -ENOMEM ;
	} else {
		memset(gpQdmaPriv, 0, sizeof(QDMA_Private_T)) ;
		gpQdmaPriv->devCfg.txRecycleThreshold = 32 ;
#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
		gpQdmaPriv->devCfg.txQueueTrtcmScale = QDMA_TRTCM_SCALE_128BYTE ;
		gpQdmaPriv->devCfg.gponTrtcmScale = QDMA_TRTCM_SCALE_128BYTE ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
#ifdef CONFIG_DEBUG
		gpQdmaPriv->devCfg.dbgLevel = DBG_ERR ;
#endif /* CONFIG_DEBUG */
#ifdef CONFIG_SUPPORT_SELF_TEST
		gpQdmaPriv->devCfg.waitTxMaxNums = 1 ;
		gpQdmaPriv->devCfg.countDown = 1 ;
#endif /* CONFIG_SUPPORT_SELF_TEST */
		spin_lock_init(&gpQdmaPriv->txLock) ;
		spin_lock_init(&gpQdmaPriv->rxLock) ;
		spin_lock_init(&gpQdmaPriv->irqLock) ;
	}
	
	/* Base Register remap of QDMA */
	gpQdmaPriv->csrBaseAddr = (uint)(ioremap_nocache(CONFIG_QDMA_BASE_ADDR, 0xFF)) ; 
	if(!gpQdmaPriv->csrBaseAddr) {
		QDMA_ERR("ioremap the QDMA base address failed.\n") ;
		return -EFAULT ;
	}
	
	/* Initial for design and verification function */
	if((ret = qdma_bm_dscp_init(CONFIG_TX_DSCP_NUM, CONFIG_RX_DSCP_NUM, CONFIG_HWFWD_DSCP_NUM, CONFIG_IRQ_DEPTH, CONFIG_HWFWD_MSG_LENS, CONFIG_MAX_PKT_LENS)) != 0) {
		QDMA_ERR("QDMA DSCP initialization failed.\n") ;
		return ret ;
	}
	
	/* Initial the QDMA tasklet */
	tasklet_init(&gpQdmaPriv->task, qdma_bm_task, 0) ;

	/***************************************************
	* QDMA device initialization                       *
	****************************************************/
	if((ret = qdma_dev_init()) != 0) {
		QDMA_ERR("QDMA hardware device initialization failed.\n") ;
		return ret ;
	}


 	/* Register QDMA interrupt */
	if(request_irq(CONFIG_QDMA_IRQ, qdma_isr, 0, "qdma", NULL) != 0) {
		QDMA_ERR("Request the interrupt service routine fail, irq:%d.\n", CONFIG_QDMA_IRQ) ;
		return -ENODEV ;
	}
	gpQdmaPriv->devCfg.flags.isIsrRequest = 1 ;

	/* Initial proc file node */
	proc_mkdir("qdma", NULL);
	qdma_proc = create_proc_entry("qdma/counters", 0, NULL) ;
	if(qdma_proc) {
		qdma_proc->read_proc = qdma_bm_counters_read_proc ;
		qdma_proc->write_proc = qdma_bm_counters_write_proc ;
	}
	qdma_proc = create_proc_entry("qdma/debug", 0, NULL) ;
	if(qdma_proc) {
		qdma_proc->read_proc = qdma_bm_debug_read_proc ;
		qdma_proc->write_proc = qdma_bm_debug_write_proc ;
	}

	/* Initial for design and verification function */
	if((ret = qdma_dvt_init()) != 0) {
		QDMA_ERR("QDMA verification test initialization failed.\n") ;
		return ret ;
	}
	
	QDMA_MSG(DEG_MSG, "QDMA init done. TX Head:%.8x, TX Tail:%.8x, Tx Num: %d. RX Start:%.8x, Num: %d.\n", 
											(uint)gpQdmaPriv->txHeadPtr->dscpPtr, (uint)gpQdmaPriv->txHeadPtr->dscpPtr, gpQdmaPriv->txDscpNum,
											(uint)gpQdmaPriv->rxStartPtr->dscpPtr, gpQdmaPriv->rxDscpNum) ;
	
	return 0 ;
}


module_init(qdma_module_init)
module_exit(qdma_module_cleanup)
