#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#include "qdma_dev.h"
#include "qdma_bmgr.h"
#include "qdma_api.h"

/******************************************************************************
 Descriptor:	It's used to init the QDMA software driver and hardware device.
 				This function must be called if the upper layer application wanna
 				use the QDMA to send/receive packets.
 Input Args:	The pointer of the QDMA_InitCfg_t
 Ret Value:		0: init successful otherwise failed.
******************************************************************************/
int qdma_init(QDMA_InitCfg_t *pInitCfg)
{
	int ret ;

	if((ret = qdma_receive_packet_mode(pInitCfg->rxRecvMode)) != 0) {
		return ret ;
	}
	
	if(pInitCfg->txRecycleMode == QDMA_TX_POLLING) {
		if(pInitCfg->txRecycleThreshold > 0) {
			ret = qdma_bm_tx_polling_mode(QDMA_ENABLE, pInitCfg->txRecycleThreshold) ;
		} else {
			ret = qdma_bm_tx_polling_mode(QDMA_ENABLE, 32) ;
		}
	} else {
		ret = qdma_bm_tx_polling_mode(QDMA_DISABLE, 0) ;
	}
	if(ret != 0) {
		return ret ;
	}

	if(pInitCfg->txIrqThreshold != 0) {
		if((ret = qdma_set_tx_delay(pInitCfg->txIrqThreshold, pInitCfg->txIrqPTime)) != 0) {
			return ret ;
		}
	}
	
	if(pInitCfg->rxDelayInt != 0) {
		if((ret = qdma_set_rx_delay(pInitCfg->rxDelayInt, pInitCfg->rxDelayPTime)) != 0) {
			return ret ;
		}
	}
 	
	qdma_register_callback_function(QDMA_CALLBACK_TX_FINISHED, pInitCfg->cbXmitFinish) ;
	qdma_register_callback_function(QDMA_CALLBACK_RX_PACKET, pInitCfg->cbRecvPkts) ; 
	qdma_register_callback_function(QDMA_CALLBACK_EVENT_HANDLER, pInitCfg->cbEventHandler) ;
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	qdma_register_callback_function(QDMA_CALLBACK_TX_MSG, pInitCfg->cbXmitMsg) ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
	qdma_register_callback_function(QDMA_CALLBACK_PTM_HANDLER, pInitCfg->cbPtmHandler) ;
	qdma_register_callback_function(QDMA_CALLBACK_SAR_HANDLER, pInitCfg->cbSarHandler) ;
	qdma_register_callback_function(QDMA_CALLBACK_GPON_MAC_HANDLER, pInitCfg->cbGponMacHandler) ;
	qdma_register_callback_function(QDMA_CALLBACK_EPON_MAC_HANDLER, pInitCfg->cbEponMacHandler) ;
	qdma_register_callback_function(QDMA_CALLBACK_XPON_PHY_HANDLER, pInitCfg->cbXponPhyHandler) ;

	return 0 ;
}
EXPORT_SYMBOL(qdma_init) ;

/******************************************************************************
 Descriptor:	It's used to deinit the QDMA software driver and hardware device.
 				This function must be called if the upper layer application wanna
 				transfer to another application.
 Input Args:	None
 Ret Value:		0: init successful otherwise failed.
******************************************************************************/
int qdma_deinit(void)
{
	qdma_receive_packet_mode(QDMA_RX_INTERRUPT) ;
	qdma_bm_tx_polling_mode(QDMA_DISABLE, 0) ;
 	
	qdma_unregister_callback_function(QDMA_CALLBACK_TX_FINISHED) ;
	qdma_unregister_callback_function(QDMA_CALLBACK_RX_PACKET) ; 
	qdma_unregister_callback_function(QDMA_CALLBACK_EVENT_HANDLER) ;
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	qdma_unregister_callback_function(QDMA_CALLBACK_TX_MSG) ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
	qdma_unregister_callback_function(QDMA_CALLBACK_PTM_HANDLER) ;
	qdma_unregister_callback_function(QDMA_CALLBACK_SAR_HANDLER) ;
	qdma_unregister_callback_function(QDMA_CALLBACK_GPON_MAC_HANDLER) ;
	qdma_unregister_callback_function(QDMA_CALLBACK_EPON_MAC_HANDLER) ;
	qdma_unregister_callback_function(QDMA_CALLBACK_XPON_PHY_HANDLER) ;

	return 0 ;
}
EXPORT_SYMBOL(qdma_deinit) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable the TXDMA, RXDMA and HWFWD mode
 Input Args:	arg1: TX DMA mode (QDMA_ENABLE/QDMA_DISABLE)
 				arg2: RX DMA mode (QDMA_ENABLE/QDMA_DISABLE)
 				arg3: Hardware forwarding mode (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value: 	No
******************************************************************************/
void qdma_dma_mode(QDMA_Mode_t txMode, QDMA_Mode_t rxMode, QDMA_Mode_t hwFwdMode)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(txMode == QDMA_ENABLE) {
		qdmaEnableTxDma(base) ;
	} else {
		qdmaDisableTxDma(base) ;
	}
	
	if(rxMode == QDMA_ENABLE) {
		qdmaEnableRxDma(base) ;
	} else {
		qdmaDisableRxDma(base) ;
	}

	if(hwFwdMode == QDMA_ENABLE) {
		qdmaEnableHwFwdDma(base) ;
	} else {
		qdmaDisableHwFwdDma(base) ;
	}
}
EXPORT_SYMBOL(qdma_dma_mode) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable the QDMA loopback mode
 Input Args:	arg1: loopback mode (QDMA_LOOPBACK_DISABLE/QDMA_LOOPBACK_QDMA/QDMA_LOOPBACK_UMAC)
 Ret Value: 	No
******************************************************************************/
void qdma_loopback_mode(QDMA_LoopbackMode_t lbMode)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(lbMode == QDMA_LOOPBACK_DISABLE) {
		qdmaDisableUmacLoopback(base) ;
		qdmaDisableQdmaLoopback(base) ;
	} else if(lbMode == QDMA_LOOPBACK_QDMA) {
		qdmaDisableUmacLoopback(base) ;
		qdmaEnableQdmaLoopback(base) ;
	} else if(lbMode == QDMA_LOOPBACK_UMAC) {
		qdmaDisableQdmaLoopback(base) ;
		qdmaEnableUmacLoopback(base) ;
	}
}
EXPORT_SYMBOL(qdma_loopback_mode) ;

/******************************************************************************
 Descriptor:	It's used to configure the TX interrupt delay parameters.
 Input Args:	arg1: irq queue threshold (0~irqDepth). When valid irq entry 
 				len > irq queue threshold, the hardware will generate an interrupt. 
 				arg2: irq pending time (0~255). This argument is specified max 
 				pending for the irq queue interrupt. The uint of the pending time 
 				is 20us.
 Ret Value:		0: setting successful otherwise failed.
******************************************************************************/
int qdma_set_tx_delay(unchar txIrqThreshold, ushort txIrqPtime)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	
	if(txIrqThreshold<=0 || txIrqThreshold>gpQdmaPriv->irqDepth || txIrqPtime>0xFFFF) {
		return -EINVAL ;
	}
	
	qdmaSetIrqThreshold(base, txIrqThreshold) ;
	qdmaSetIrqPtime(base, txIrqPtime) ;
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_set_tx_delay) ;

/******************************************************************************
 Descriptor:	It's used to configure the RX interrupt delay parameters.
 Input Args:	arg1: Specified max number of pending interrupts. When the 
 				number of pending interrupts equal or greater than the arg1 or 
 				interrupt pending time reach the limit (arg2*20us), a final
 				RX_DLY_INT is generated.
 				arg2: Specified max pending time for the internal RX_DONE_INT. 
 				When the pending time equal or greater arg2*20us or the number
 				of RX_DONT_INT equal or greater than arg1, an final RX_DLY_INT 
 				is generated.
 Ret Value:		0: setting successful otherwise failed.
******************************************************************************/
int qdma_set_rx_delay(unchar rxMaxInt, unchar rxMaxPtime)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(rxMaxInt != 0) {
		qdmaSetDelayIntCfg(base, (DLY_INT_RXDLY_INT_EN | (rxMaxInt<<DLY_INT_RXMAX_PINT_SHIFT) | (rxMaxPtime<<DLY_INT_RXMAX_PTIME_SHIFT))) ;
	} else {
		qdmaSetDelayIntCfg(base, 0) ;
	}

	return 0 ;
}
EXPORT_SYMBOL(qdma_set_rx_delay) ;

#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
/******************************************************************************
 Descriptor:	It's used to configure the RX message lens.
 Input Args:	arg1: Specified the rx message lens in bytes.
 Ret Value:		0: setting successful otherwise failed.
******************************************************************************/
int qdma_set_rx_msg_lens(unchar rxMsgLens)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(rxMsgLens == 8) {
		qdmaSetRxMsgLens(base, VAL_MSG_LEN_2_WORD) ;
	} else if(rxMsgLens == 16) {
		qdmaSetRxMsgLens(base, VAL_MSG_LEN_4_WORD) ;
	} else if(rxMsgLens == 24) {
		qdmaSetRxMsgLens(base, VAL_MSG_LEN_6_WORD) ;
	} else if(rxMsgLens == 32) {
		qdmaSetRxMsgLens(base, VAL_MSG_LEN_8_WORD) ;
	} 

	return 0 ;
}
EXPORT_SYMBOL(qdma_set_rx_msg_lens) ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

/******************************************************************************
 Descriptor:	It's used to register the QDMA callback functions. The QDMA 
 				driver support several callback function type that is define 
 				in QDMA_CbType_t enum.
 Input Args:	arg1: callback function type that is define in QDMA_CbType_t enum.
 				arg2: the pointer of the callback function.
 Ret Value:		0: register successful otherwise failed.
******************************************************************************/
int qdma_register_callback_function(QDMA_CbType_t type, void *pCbFun)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(!pCbFun) {
		return -EINVAL ;	
	}
	
	switch(type) {
		case QDMA_CALLBACK_TX_FINISHED:
			gpQdmaPriv->devCfg.bmXmitCallbackFunction = (qdma_callback_xmit_finish_t)pCbFun ;
			break ;
			
		case QDMA_CALLBACK_RX_PACKET:
			gpQdmaPriv->devCfg.bmRecvCallbackFunction = (qdma_callback_recv_packet_t)pCbFun ;	
			break ;
			
		case QDMA_CALLBACK_EVENT_HANDLER:
			gpQdmaPriv->devCfg.bmEventCallbackFunction = (qdma_callback_event_handler_t)pCbFun ;	
			break ;
						
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
		case QDMA_CALLBACK_TX_MSG:
			gpQdmaPriv->devCfg.bmXmitMsgCallbackFunction = (qdma_callback_xmit_msg_t)pCbFun ;	
			break ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */

		case QDMA_CALLBACK_PTM_HANDLER :
			gpQdmaPriv->devCfg.bmPtmIntHandler = (qdma_callback_int_handler_t)pCbFun ;
			qdmaEnableInt(base, INT_MASK_PTM) ;
			break ;
			
		case QDMA_CALLBACK_SAR_HANDLER :
			gpQdmaPriv->devCfg.bmSarIntHandler = (qdma_callback_int_handler_t)pCbFun ;
			qdmaEnableInt(base, INT_MASK_ATM_SAR) ;
			break ;
			
		case QDMA_CALLBACK_GPON_MAC_HANDLER :
			gpQdmaPriv->devCfg.bmGponMacIntHandler = (qdma_callback_int_handler_t)pCbFun ;
			qdmaEnableInt(base, INT_MASK_GPON_MAC) ;
			break ;
			
		case QDMA_CALLBACK_EPON_MAC_HANDLER :
			gpQdmaPriv->devCfg.bmEponMacIntHandler = (qdma_callback_int_handler_t)pCbFun ;
			qdmaEnableInt(base, INT_MASK_EPON_MAC) ;
			break ;
			
		case QDMA_CALLBACK_XPON_PHY_HANDLER :
			gpQdmaPriv->devCfg.bmXponPhyIntHandler = (qdma_callback_int_handler_t)pCbFun ;
			qdmaEnableInt(base, INT_MASK_XPON_PHY) ;
			break ;
			
		default:
			return -EFAULT ;	
	}
	return 0 ;
}
EXPORT_SYMBOL(qdma_register_callback_function) ;

/******************************************************************************
 Description:	It's used to unregister the QDMA callback functions.
 Input Args:	arg1: callback function type that is define in QDMA_CbType_t enum.
 Ret Value:		0: unregister successful otherwise failed.
******************************************************************************/
int qdma_unregister_callback_function(QDMA_CbType_t type)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	switch(type) {
		case QDMA_CALLBACK_TX_FINISHED:
			gpQdmaPriv->devCfg.bmXmitCallbackFunction = NULL ;
			break ;
			
		case QDMA_CALLBACK_RX_PACKET:
			gpQdmaPriv->devCfg.bmRecvCallbackFunction = NULL ;	
			break ;
	
		case QDMA_CALLBACK_EVENT_HANDLER:
			gpQdmaPriv->devCfg.bmEventCallbackFunction = NULL ;	
			break ;
			
		case QDMA_CALLBACK_PTM_HANDLER :
			qdmaDisableInt(base, INT_MASK_PTM) ;
			gpQdmaPriv->devCfg.bmPtmIntHandler = NULL ;
			break ;
			
		case QDMA_CALLBACK_SAR_HANDLER :
			qdmaDisableInt(base, INT_MASK_ATM_SAR) ;
			gpQdmaPriv->devCfg.bmSarIntHandler = NULL ;
			break ;
			
		case QDMA_CALLBACK_GPON_MAC_HANDLER :
			qdmaDisableInt(base, INT_MASK_GPON_MAC) ;
			gpQdmaPriv->devCfg.bmGponMacIntHandler = NULL ;
			break ;
			
		case QDMA_CALLBACK_EPON_MAC_HANDLER :
			qdmaDisableInt(base, INT_MASK_EPON_MAC) ;
			gpQdmaPriv->devCfg.bmEponMacIntHandler = NULL ;
			break ;
			
		case QDMA_CALLBACK_XPON_PHY_HANDLER :
			qdmaDisableInt(base, INT_MASK_XPON_PHY) ;
			gpQdmaPriv->devCfg.bmXponPhyIntHandler = NULL ;
			break ;
			
		default:
			return -EFAULT ;	
	}
	return 0 ;
}
EXPORT_SYMBOL(qdma_unregister_callback_function) ;

/******************************************************************************
 Descriptor:	It's used to check there is any unused RX DSCP.
 Input Args:	No
 Ret Value:		0: no free RX DSCP, 1: at least one RX DSCP
******************************************************************************/
int qdma_has_free_rxdscp(void)
{
	return (gpQdmaPriv->rxEndPtr->next != gpQdmaPriv->rxStartPtr) ;
}
EXPORT_SYMBOL(qdma_has_free_rxdscp) ;


/******************************************************************************
 Description:	
 Input Args:	
 Ret Value:		
******************************************************************************/
int qdma_hook_receive_buffer(void *pMsg, uint msgLen, struct sk_buff *skb)
{
	return qdma_bm_hook_receive_buffer(pMsg, msgLen, skb) ;
}
EXPORT_SYMBOL(qdma_hook_receive_buffer) ;

/******************************************************************************
 Description:	
 Input Args:	
 Ret Value:		
******************************************************************************/
int qdma_recycle_receive_buffer(void)
{
	return qdma_bm_recycle_receive_buffer() ;
}
EXPORT_SYMBOL(qdma_recycle_receive_buffer) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
void qdma_enable_rxpkt_int(void)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	
	qdmaEnableInt(base, (INT_MASK_NO_RX_CPU_DSCP|INT_MASK_RX_DONE)) ;
	gpQdmaPriv->devCfg.flags.isRxPolling = QDMA_DISABLE ;
}
EXPORT_SYMBOL(qdma_enable_rxpkt_int) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
void qdma_disable_rxpkt_int(void)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	
	qdmaDisableInt(base, (INT_MASK_NO_RX_CPU_DSCP|INT_MASK_RX_DONE)) ;
	gpQdmaPriv->devCfg.flags.isRxPolling = QDMA_ENABLE ;
}
EXPORT_SYMBOL(qdma_disable_rxpkt_int) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packet_mode(QDMA_RecvMode_t rxMode)
{
	uint base = gpQdmaPriv->csrBaseAddr ;
	
	if(rxMode == QDMA_RX_POLLING) {
		qdmaDisableInt(base, (INT_MASK_NO_RX_CPU_DSCP|INT_MASK_RX_DONE)) ;
		gpQdmaPriv->devCfg.flags.isRxPolling = QDMA_ENABLE ;
		gpQdmaPriv->devCfg.flags.isRxNapi = QDMA_DISABLE ;
	} else {
		qdmaEnableInt(base, (INT_MASK_NO_RX_CPU_DSCP|INT_MASK_RX_DONE)) ;
		gpQdmaPriv->devCfg.flags.isRxPolling = QDMA_DISABLE ;
	
		if(rxMode == QDMA_RX_NAPI) {
			gpQdmaPriv->devCfg.flags.isRxNapi = QDMA_ENABLE ;
		} else {
			gpQdmaPriv->devCfg.flags.isRxNapi = QDMA_DISABLE ;
		}
	}
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_receive_packet_mode) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packets(uint maxPkts)
{
	if(gpQdmaPriv->devCfg.flags.isRxPolling==QDMA_DISABLE && gpQdmaPriv->devCfg.flags.isRxNapi==QDMA_DISABLE) {
		return -EFAULT ;
	} 
	
	return qdma_bm_receive_packets(maxPkts) ;
}
EXPORT_SYMBOL(qdma_receive_packets) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_transmit_packet(void *pMsg, uint msgLen, struct sk_buff *skb)
{
	return qdma_bm_transmit_packet(pMsg, msgLen, skb) ;
}
EXPORT_SYMBOL(qdma_transmit_packet) ;

/******************************************************************************
 Descriptor:	It's used to configure the tx DSCP recycle mode. The upper MAC
 				driver should set the tx DSCP recycle in polling mode or 
 				interrupt mode.
 Input Args:	arg1: tx DSCP recycle mode (QDMA_TX_POLLING/QDMA_TX_INTERRUPT)
 				arg2: tx DSCP threshold for recycling in polling mode. if the tx
 					  recycle mode is interrupt, this arguement is unused.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
#ifdef CONFIG_TX_POLLING_BY_MAC
int qdma_txdscp_recycle_mode(QDMA_RecycleMode_t txMode)
{
	return qdma_bm_tx_polling_mode((txMode==QDMA_TX_POLLING)?QDMA_ENABLE:QDMA_DISABLE, 0) ; /* the second argument is don't care */
}
#else
int qdma_txdscp_recycle_mode(QDMA_RecycleMode_t txMode, unchar txThreshold)
{
	if(txMode == QDMA_TX_POLLING) {
		if(txThreshold <= 0) {
			return -EINVAL ;
		}
		
		return qdma_bm_tx_polling_mode(QDMA_ENABLE, txThreshold) ;
	} else {
		return qdma_bm_tx_polling_mode(QDMA_DISABLE, 0) ;
	}
}
#endif /* CONFIG_TX_POLLING_BY_MAC */
EXPORT_SYMBOL(qdma_txdscp_recycle_mode) ;

#ifdef CONFIG_TX_POLLING_BY_MAC
/******************************************************************************
 Descriptor:	It's used to recycle the tx dscp by upper MAC driver.
 Input Args:	arg1: tx DSCP count. 0 means recycle all of the tx dscp.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_txdscp_recycle(uint count)
{
	if(gpQdmaPriv->devCfg.flags.isTxPolling != QDMA_ENABLE) {
		return -EFAULT ;	
	}
	
	return qdma_bm_transmit_done(count) ;
}
EXPORT_SYMBOL(qdma_txdscp_recycle) ;
#endif /* CONFIG_TX_POLLING_BY_MAC */

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_recycle_transmit_buffer(void)
{
	return qdma_bm_recycle_transmit_buffer() ;
}
EXPORT_SYMBOL(qdma_recycle_transmit_buffer) ;

#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
/******************************************************************************
 Descriptor:	It's used to configure the TXQOS weight type and scale.
 Input Args:	arg1: setting the WRR weighting value is base on packet or byte
 					  (QDMA_TXQOS_WEIGHT_BY_PACKET/QDMA_TXQOS_WEIGHT_BY_BYTE)
 				arg2: setting the byte weighting scale(QDMA_TXQOS_WEIGHT_SCALE_64B
 				      /QDMA_TXQOS_WEIGHT_SCALE_16B). when the weigthing value 
 				      is base on packet, these argument is don't care.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_qos_weight(QDMA_TxQosWeightType_t weightBase, QDMA_TxQosWeightScale_t weightScale)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(weightBase == QDMA_TXQOS_WEIGHT_BY_PACKET) {
		qdmaSetTxQosWeightByPacket(base) ;
	} else if(weightBase == QDMA_TXQOS_WEIGHT_BY_BYTE) {
		qdmaSetTxQosWeightByByte(base) ;
		if(weightScale == QDMA_TXQOS_WEIGHT_SCALE_64B) {
			qdmaSetTxQosWeightScale64(base) ;
		} else if(weightScale == QDMA_TXQOS_WEIGHT_SCALE_16B) {
			qdmaSetTxQosWeightScale16(base) ;
		} else {
			return -EINVAL ;
		}
	} else {
		return -EINVAL ;
	}
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_set_tx_qos_weight) ;

/******************************************************************************
 Descriptor:	It's used to get the TXQOS weight type and scale.
 Input Args:	arg1: the pointer of the weight base value 
 				      (QDMA_TXQOS_WEIGHT_BY_PACKET/QDMA_TXQOS_WEIGHT_BY_BYTE)
 				arg2: the pointer of the weight scale value
 					  (QDMA_TXQOS_WEIGHT_SCALE_64B/QDMA_TXQOS_WEIGHT_SCALE_16B)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_qos_weight(QDMA_TxQosWeightType_t *pWeightBase, QDMA_TxQosWeightScale_t *pWeightScale)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(qdmaIsTxQosWeightByByte(base)) {
		*pWeightBase = QDMA_TXQOS_WEIGHT_BY_BYTE ;
	} else {
		*pWeightBase = QDMA_TXQOS_WEIGHT_BY_PACKET ;
	}
	
	if(qdmaIsTxQosWeightScale16(base)) {
		*pWeightScale = QDMA_TXQOS_WEIGHT_SCALE_16B ;
	} else {
		*pWeightScale = QDMA_TXQOS_WEIGHT_SCALE_64B ;
	}
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_get_tx_qos_weight) ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

/******************************************************************************
 Descriptor:	It's used to configure the tx queue scheduler and queue weigth
 				for specific channel.
 Input Args:	The pointer of the tx qos scheduler struct. It includes:
 				- channel: specific the channel ID (0~15)
 				- qosType: The QoS type is define in QDMA_TxQosType_t enum.
 				- weight: The unit of WRR weight is packets (0~100, 255 means don't care).
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_qos(QDMA_TxQosScheduler_T *pTxQos) 
{
	int i ;
	unchar weight[CONFIG_QDMA_QUEUE] ;
	
	if(pTxQos->channel >= CONFIG_QDMA_CHANNEL) {
		return -EINVAL ;
	}
	
	if(pTxQos->qosType >= QDMA_TXQOS_TYPE_NUMS) {
		return -EINVAL ;
	}
	
	for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
		if((pTxQos->queue[i].weight<0 || pTxQos->queue[i].weight>100) && pTxQos->queue[i].weight!=255) {
			return -EINVAL ;
		}
	
		weight[i] = pTxQos->queue[i].weight ;
	}
	
	return qdmaSetTxQosScheduler(pTxQos->channel, (unchar)pTxQos->qosType, weight) ;
}
EXPORT_SYMBOL(qdma_set_tx_qos) ;

/******************************************************************************
 Descriptor:	It's used to set the tx queue scheduler and queue weigth
 				for specific channel.
 Input Args:	The pointer of the tx qos scheduler struct. It includes:
 				- channel: specific the channel ID (0~15)
 				- qosType: The QoS type is define in QDMA_TxQosType_t enum.
 				- weight: The unit of WRR weight is packets.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_qos(QDMA_TxQosScheduler_T *pTxQos)
{
	int ret, i ;
	unchar qosType ;
	unchar weight[CONFIG_QDMA_QUEUE] ;
	
	if(pTxQos->channel >= CONFIG_QDMA_CHANNEL) {
		return -EINVAL ;
	}

	ret = qdmaGetTxQosScheduler(pTxQos->channel, &qosType, weight) ;
	if(ret < 0) {
		return -EFAULT ;
	}
	
	pTxQos->qosType = qosType ;
	
	for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
		pTxQos->queue[i].weight = weight[i] ;
	}
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_get_tx_qos) ;

/******************************************************************************
 Descriptor:	It's used to configure the tx buffer threshold. For the buffer 
 				management, the total available on-chip buffer is 64Kbyte (256 
 				Blocks, 256 bytes per block). It is shared among WAN and LAN 
 				Tx/Rx interface. If the buffer usage exceeds the threshold, the
 				Tx DMA will stop retrieving packets.
 Input Args:	The pointer of the tx buffer control struct. It includes:
 				- mode: Eanble/Disable tx buffer usage control
 				- chnThreshold: Per tx per channel block usage threshold.
 				- totalThreshold: Total tx block usage threshold.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txbuf_threshold(QDMA_TxBufCtrl_T *pTxBufCtrl)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(pTxBufCtrl->mode == QDMA_ENABLE) {
		qdmaEnableTxBufCtrl(base) ;
		
		qdmaTxBufChnnelThreshold(base, pTxBufCtrl->chnThreshold) ;
		qdmaTxBufTotalThreshold(base, pTxBufCtrl->totalThreshold) ;
	} else if(pTxBufCtrl->mode == QDMA_DISABLE) {
		qdmaDisableTxBufCtrl(base) ;
	} else {
		return -EINVAL ;
	}

	return 0 ;
}
EXPORT_SYMBOL(qdma_set_txbuf_threshold) ;

#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
/******************************************************************************
 Descriptor:	It's used to configure the WAN scheduling mode
 Input Args:	arg1: WAN type (QDMA_WAN_TYPE_GPON/QDMA_WAN_TYPE_EPON)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_scheduling_mode(QDMA_WanType_t type)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(type == QDMA_WAN_TYPE_GPON) {
		qdmaGponSchedulingMode(base) ;
	} else if(type == QDMA_WAN_TYPE_EPON) {
		qdmaEponSchedulingMode(base) ;
	} else {
		return -EINVAL ;
	}

	return 0 ;
}
EXPORT_SYMBOL(qdma_set_scheduling_mode) ;

/******************************************************************************
 Descriptor:	It's used to configure the WAN report mode
 Input Args:	arg1: WAN type (QDMA_WAN_TYPE_GPON/QDMA_WAN_TYPE_EPON)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_report_mode(QDMA_WanType_t type)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(type == QDMA_WAN_TYPE_GPON) {
		qdmaGponReportMode(base) ;
	} else if(type == QDMA_WAN_TYPE_EPON) {
		qdmaEponReportMode(base) ;
	} else {
		return -EINVAL ;
	}

	return 0 ;
}
EXPORT_SYMBOL(qdma_set_report_mode) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable the TRTCM report mode.
 Input Args:	arg1: TRTCM report mode (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_trtcm_mode(QDMA_Mode_t mode) 
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(mode == QDMA_DISABLE) {
		qdmaDisableTrtcm(base) ;
	} else {
		qdmaEnableTrtcm(base) ;
	}
}
EXPORT_SYMBOL(qdma_set_trtcm_mode) ;

/******************************************************************************
 Descriptor:	It's used to set the TRTCM parameter scale.
 Input Args:	arg1: parameter scale (0:1byte, 1:2byte, 2:4byte, 3:8byte...)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_trtcm_param_scale(QDMA_TrtcmScale_t scale)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	qdmaSetTrtcmUnit(base, scale) ;
}
EXPORT_SYMBOL(qdma_set_trtcm_param_scale) ;

/******************************************************************************
 Descriptor:	It's used to set the value of TRTCM parameter. It includes
 				CIR, CBS, PIR and PBS.
 Input Args:	arg1: The pointer of the TRTCM parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_trtcm_params(QDMA_TrtcmParam_T *pTrtcmParam) 
{
	int ret ;
	
	if(pTrtcmParam->channel >= 16) {
		return -EINVAL ;
	}
	
	if((ret = qdmaSetTrtcm(pTrtcmParam->channel, QDMA_TRTCM_PARAM_CIR, pTrtcmParam->cir)) < 0) {
		return ret ;
	}
	if((ret = qdmaSetTrtcm(pTrtcmParam->channel, QDMA_TRTCM_PARAM_CBS, pTrtcmParam->cbs)) < 0) {
		return ret ;
	}
	if((ret = qdmaSetTrtcm(pTrtcmParam->channel, QDMA_TRTCM_PARAM_PIR, pTrtcmParam->pir)) < 0) {
		return ret ;
	}
	if((ret = qdmaSetTrtcm(pTrtcmParam->channel, QDMA_TRTCM_PARAM_PBS, pTrtcmParam->pbs)) < 0) {
		return ret ;
	}
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_set_trtcm_params) ;

/******************************************************************************
 Descriptor:	It's used to get the value of TRTCM parameter. It includes
 				CIR, CBS, PIR and PBS.
 Input Args:	arg1: The pointer of the TRTCM parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_trtcm_params(QDMA_TrtcmParam_T *pTrtcmParam) 
{
	int ret ;
	
	if(pTrtcmParam->channel >= 16) {
		return -EINVAL ;
	}
	
	ret = qdmaGetTrtcm(pTrtcmParam->channel, QDMA_TRTCM_PARAM_CIR) ;
	if(ret < 0)
		return ret ;
	pTrtcmParam->cir = ret ;
	
	ret = qdmaGetTrtcm(pTrtcmParam->channel, QDMA_TRTCM_PARAM_CBS) ;
	if(ret < 0)
		return ret ;
	pTrtcmParam->cbs = ret ;
	
	ret = qdmaGetTrtcm(pTrtcmParam->channel, QDMA_TRTCM_PARAM_PIR) ;
	if(ret < 0)
		return ret ;
	pTrtcmParam->pir = ret ;
	
	ret = qdmaGetTrtcm(pTrtcmParam->channel, QDMA_TRTCM_PARAM_PBS) ;
	if(ret < 0)
		return ret ;
	pTrtcmParam->pbs = ret ;
		
	return 0 ;
}
EXPORT_SYMBOL(qdma_get_trtcm_params) ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

/******************************************************************************
 Descriptor:	It's used to set the EPON threshold report mode.
 Input Args:	arg1: channel number (0~7)
 				arg2: EPON threhold report mode (QDMA_EPON_REPORT_ONE_THRESHOLD/
 				QDMA_EPON_REPORT_TWO_THRESHOLD/QDMA_EPON_REPORT_THREE_THRESHOLD)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_epon_report_mode(unchar channel, QDMA_EponReportMode_t mode) 
{
	if(channel >= 8) {
		return -EINVAL ;
	}
	
	if(mode == QDMA_EPON_REPORT_ONE_THRESHOLD) {
		return qdmaSetEponReportConfig(channel, 1) ;
	} else if(mode == QDMA_EPON_REPORT_TWO_THRESHOLD) {
		return qdmaSetEponReportConfig(channel, 2) ;
	} else if(mode == QDMA_EPON_REPORT_THREE_THRESHOLD) {
		return qdmaSetEponReportConfig(channel, 3) ;
	} else {
		return qdmaSetEponReportConfig(channel, 0) ;
	}
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_set_epon_report_mode) ;

/******************************************************************************
 Descriptor:	It's used to get the EPON threshold report mode.
 Input Args:	arg1: channel (0~7)
 Ret Value:		QDMA_EPON_REPORT_ONE_THRESHOLD, QDMA_EPON_REPORT_TWO_THRESHOLD, 
 				QDMA_EPON_REPORT_THREE_THRESHOLD,  <0: failed.
******************************************************************************/
int qdma_get_epon_report_mode(unchar channel) 
{
	unchar mode ;
	int ret = 0 ;
	
	if(channel >= 8) {
		return -EINVAL ;
	}
	
	ret = qdmaGetEponReportConfig(channel, &mode) ;
	if(ret == 0) {
		return QDMA_EPON_REPORT_WO_THRESHOLD ;
	} else if(ret == 1) {
		return QDMA_EPON_REPORT_ONE_THRESHOLD ;
	} else if(ret == 2) {
		return QDMA_EPON_REPORT_TWO_THRESHOLD ;
	} else if(ret == 3) {
		return QDMA_EPON_REPORT_THREE_THRESHOLD ;
	} else {
		return ret ;
	}
}
EXPORT_SYMBOL(qdma_get_epon_report_mode) ;

/******************************************************************************
 Descriptor:	It's used to set the EPON queue threshold for report.
 Input Args:	arg1: The pointer of the EPON queue threshold struct. The struct 
 				includes channel number(0~7), queue number(0~7), threshold index(0~2)
 				and queue threshold value for EPON local queue status report.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_epon_queue_threshold(QDMA_EponQueueThreshold_T *pEponQThr) 
{
	if(pEponQThr->channel>=8 || pEponQThr->queue>=8 || pEponQThr->thrIdx>=3) {
		return -EINVAL ;
	}

	return qdmaSetEponThreshold(pEponQThr->channel, pEponQThr->queue, pEponQThr->thrIdx, pEponQThr->value) ;
}
EXPORT_SYMBOL(qdma_set_epon_queue_threshold) ;

/******************************************************************************
 Descriptor:	It's used to get the EPON queue threshold for report.
 Input Args:	arg1: The pointer of the EPON queue threshold struct. The struct 
 				includes channel number(0~7), queue number(0~7), threshold index(0~2)
 				and queue threshold value for EPON local queue status report.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_epon_queue_threshold(QDMA_EponQueueThreshold_T *pEponQThr) 
{
	int ret ;
	
	if(pEponQThr->channel>=8 || pEponQThr->queue>=8 || pEponQThr->thrIdx>=3) {
		return -EINVAL ;
	}

	ret = qdmaGetEponThreshold(pEponQThr->channel, pEponQThr->queue, pEponQThr->thrIdx) ;
	if(ret >= 0) {
		pEponQThr->value = ret ;
	} else {
		return ret ;
	}
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_get_epon_queue_threshold) ;


#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
/******************************************************************************
 Descriptor:	It's used to retire the DSCP for specific channel.
 Input Args:	arg1: channel number (0~15)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_channel_retire(uint channel)
{
	if(channel >= CONFIG_QDMA_CHANNEL) {
		return -EINVAL ;
	}

	return qdmaSetChannelRetire(channel) ;
}
EXPORT_SYMBOL(qdma_set_channel_retire) ;

/******************************************************************************
 Descriptor:	It's used to set the channel retire mask for specific channel.
 Input Args:	arg1: channel number (0~15)
 				arg2: Enable/disable channel retire mask
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_channel_retire_mask(uint channel, QDMA_Mode_t maskMode)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(channel >= CONFIG_QDMA_CHANNEL) {
		return -EINVAL ;
	}

	if(maskMode == QDMA_ENABLE) {
		qdmaSetTxChannelRetireMask(base, channel) ;
	} else {
		qdmaClearTxChannelRetireMask(base, channel) ;
	}
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_set_tx_channel_retire_mask) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable QDMA pre-fetch function. Since the
                on-chip buffer resource is limited, to prevent from Head-Of-Line
                blocking issue, 'when' to retrieving packet from External DRAM 
                to on-chip buffer will be application dependent.
 Input Args:	arg1: Enable/disable TXDMA pre-fetch function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_prefetch_mode(QDMA_Mode_t prefecthMode)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(prefecthMode == QDMA_ENABLE) {
		qdmaEnableTxDmaPrefetch(base) ;
	} else {
		qdmaDisableTxDmaPrefetch(base) ;
	}
}
EXPORT_SYMBOL(qdma_set_prefetch_mode) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue trTCM coloring function. 
 Input Args:	arg1: Enable/disable tx queue trTCM function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_txqueue_trtcm_mode(QDMA_Mode_t trtcmMode)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(trtcmMode == QDMA_ENABLE) {
		qdmaEnableTrtcmColorMode(base) ;
	} else {
		qdmaDisableTrtcmColorMode(base) ;
	}
}
EXPORT_SYMBOL(qdma_set_txqueue_trtcm_mode) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue trTCM coloring mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue trTCM mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
QDMA_Mode_t qdma_get_txqueue_trtcm_mode(void)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(qdmaIsSetTrtcmColorMode(base)) {
		return QDMA_ENABLE ;
	} else {
		return QDMA_DISABLE ;
	}
}
EXPORT_SYMBOL(qdma_get_txqueue_trtcm_mode) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue DEI dropped function. 
 Input Args:	arg1: Enable/disable tx queue DEI function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_txqueue_dei_mode(QDMA_Mode_t deiDropMode)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(deiDropMode == QDMA_ENABLE) {
		qdmaEnableDeiDropMode(base) ;
	} else {
		qdmaDisableDeiDropMode(base) ;
	}
}
EXPORT_SYMBOL(qdma_set_txqueue_dei_mode) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue DEI dropped mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue DEI mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
QDMA_Mode_t qdma_get_txqueue_dei_mode(void)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(qdmaIsSetDeiDropMode(base)) {
		return QDMA_ENABLE ;
	} else {
		return QDMA_DISABLE ;
	}
}
EXPORT_SYMBOL(qdma_get_txqueue_dei_mode) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue threshold dropped function. 
 Input Args:	arg1: Enable/disable tx queue threshold function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_txqueue_threshold_mode(QDMA_Mode_t thrsldDropMode)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(thrsldDropMode == QDMA_ENABLE) {
		qdmaEnableThresholdMode(base) ;
	} else {
		qdmaDisableThresholdMode(base) ;
	}
}
EXPORT_SYMBOL(qdma_set_txqueue_threshold_mode) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue threshold dropped mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue threshold mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
QDMA_Mode_t qdma_get_txqueue_threshold_mode(void)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(qdmaIsSetThresholdMode(base)) {
		return QDMA_ENABLE ;
	} else {
		return QDMA_DISABLE ;
	}
}
EXPORT_SYMBOL(qdma_get_txqueue_threshold_mode) ;

/******************************************************************************
 Descriptor:	It's used to set the tx queue congestion scale. 
 Input args:	arg1: The pointer of the QDMA_TxQueueCongestScale_T struct.
 				It includes two parameters.
  				1. maxScale: The scale factor for TXQ_MAX_THRESHOLD
 				2. minScale: The scale factor for TXQ_MIN_THRESHOLD
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_congestion_scale(QDMA_TxQueueCongestScale_T *pScale)
{
	if((pScale->maxScale<0 || pScale->maxScale>=QDMA_TXQUEUE_SCALE_ITEMS) ||
	   (pScale->minScale<0 || pScale->minScale>=QDMA_TXQUEUE_SCALE_ITEMS)) {
		return -EINVAL ;
	}

	return qdmaSetTxQueueCngsScale(pScale->maxScale, pScale->minScale) ;
}
EXPORT_SYMBOL(qdma_set_congestion_scale) ;

/******************************************************************************
 Descriptor:	It's used to set the tx queue congestion drop probability. 
 Input args:	arg1: The pointer of the QDMA_TxQueueDropProbability_T struct.
 				It includes two parameters.
 				1. green: Dropped precentage value for green packets(0~100).
 				   0 means never dropped and 100 means always dropped.
 				2. yellow: Dropped precentage value for yellow packets(0~100)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_congest_drop_probability(QDMA_TxQueueDropProbability_T *pProbability) 
{
	if((pProbability->green<0 || pProbability->green>255) || 
	   (pProbability->yellow<0 || pProbability->yellow>255)) {
		return -EINVAL ;
	}
	
	return qdmaSetTxQueueDropProbability(pProbability->green, pProbability->yellow) ;
}
EXPORT_SYMBOL(qdma_set_congest_drop_probability) ;

/******************************************************************************
 Descriptor:	It's used to set the tx queue congestion threshold. 
 Input args:	arg1: The pointer of the QDMA_TxQueueCongestThreshold_T struct.
 				It includes five parameters.
 				1. queueIdx: tx queue index (0~7).
 				2. grnMaxThreshold: The green max threshold for specific tx queue
 				3. grnMinThreshold: The green min threshold for specific tx queue
 				4. ylwMaxThreshold: The yellow max threshold for specific tx queue
 				5. ylwMinThreshold: The yellow min threshold for specific tx queue
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_congest_threshold(QDMA_TxQueueCongestThreshold_T *pThreshold) 
{
	unchar threshold[CONFIG_QDMA_QUEUE] ;
	int ret = 0 ;
	
	if(pThreshold->queueIdx<0 || pThreshold->queueIdx>=CONFIG_QDMA_QUEUE) {
		return -EINVAL ;
	}
	
	if((pThreshold->grnMaxThreshold<0 || pThreshold->grnMaxThreshold>15) ||
	   (pThreshold->grnMinThreshold<0 || pThreshold->grnMinThreshold>15) ||
	   (pThreshold->ylwMaxThreshold<0 || pThreshold->ylwMaxThreshold>15) ||
	   (pThreshold->ylwMinThreshold<0 || pThreshold->ylwMinThreshold>15)) {
		return -EINVAL ;
	}

	if((ret = qdmaGetTxQueueGreenMaxThreshold(threshold)) == 0) {
		threshold[pThreshold->queueIdx] = pThreshold->grnMaxThreshold ;
		ret = qdmaSetTxQueueGreenMaxThreshold(threshold) ;
	} 
	if(ret != 0) {
		return ret ;
	}

	if((ret = qdmaGetTxQueueGreenMinThreshold(threshold)) == 0) {
		threshold[pThreshold->queueIdx] = pThreshold->grnMinThreshold ;
		ret = qdmaSetTxQueueGreenMinThreshold(threshold) ;
	} 
	if(ret != 0) {
		return ret ;
	}

	if((ret = qdmaGetTxQueueYellowMaxThreshold(threshold)) == 0) {
		threshold[pThreshold->queueIdx] = pThreshold->ylwMaxThreshold ;
		ret = qdmaSetTxQueueYellowMaxThreshold(threshold) ;
	} 
	if(ret != 0) {
		return ret ;
	}

	if((ret = qdmaGetTxQueueYellowMinThreshold(threshold)) == 0) {
		threshold[pThreshold->queueIdx] = pThreshold->ylwMinThreshold ;
		ret = qdmaSetTxQueueYellowMinThreshold(threshold) ;
	} 
	if(ret != 0) {
		return ret ;
	}
	
	return ret ;
}
EXPORT_SYMBOL(qdma_set_congest_threshold) ;

/******************************************************************************
 Descriptor:	It's used to get the tx queue congestion configuration. 
 Input args:	arg1: The pointer of the QDMA_TxQueueCongestCfg_T struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_congest_config(QDMA_TxQueueCongestCfg_T *pCongest)
{
	unchar threshold[CONFIG_QDMA_QUEUE] ;
	unchar maxScale, minScale ;
	unchar greenDropProb, yellowDropProb ;
	int i, ret = 0 ;
	
	if((ret = qdmaGetTxQueueCngsScale(&maxScale, &minScale)) != 0) {
		goto out ;
	} else {
		pCongest->maxScale = maxScale ;
		pCongest->minScale = minScale ;
	}
	
	if((ret = qdmaGetTxQueueDropProbability(&greenDropProb, &yellowDropProb)) != 0) {
		goto out ;
	} else {
		pCongest->grnDropProb = greenDropProb ;
		pCongest->ylwDropProb = yellowDropProb ;
	}
	
	if((ret = qdmaGetTxQueueGreenMaxThreshold(threshold)) == 0) {
		for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
			pCongest->queue[i].grnMaxThreshold = threshold[i] ;
		}
	} else {
		goto out ;
	}
	
	if((ret = qdmaGetTxQueueGreenMinThreshold(threshold)) == 0) {
		for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
			pCongest->queue[i].grnMinThreshold = threshold[i] ;
		}
	} else {
		goto out ;
	}
	
	if((ret = qdmaGetTxQueueYellowMaxThreshold(threshold)) == 0) {
		for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
			pCongest->queue[i].ylwMaxThreshold = threshold[i] ;
		}
	} else {
		goto out ;
	}

	if((ret = qdmaGetTxQueueYellowMinThreshold(threshold)) == 0) {
		for(i=0 ; i<CONFIG_QDMA_QUEUE ; i++) {
			pCongest->queue[i].ylwMinThreshold = threshold[i] ;
		}
	} else {
		goto out ;
	}

out:
	return ret ;	
}
EXPORT_SYMBOL(qdma_get_congest_config) ;

/******************************************************************************
 Descriptor:	It's used to set tx queue trTCM coloring scale. Scaled factor for 
                TCONT trTCM coloring parameters's value in the power of 2. The
                final value will be PARA_SCALE * PARA_VALUE.
                The trTCM scalue must be set before setting the parameters.
 Input Args:	arg1: scale value for tx queue trTCM coloring function. The 
 				default trtcm scale is 128 Byte.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txqueue_trtcm_scale(QDMA_TrtcmScale_t trtcmScale) 
{
	if(trtcmScale<0 || trtcmScale>=QDMA_TRTCM_SCALE_MAX_ITEMS) {
		return -EINVAL ;
	}

	gpQdmaPriv->devCfg.txQueueTrtcmScale = trtcmScale ;
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_set_txqueue_trtcm_scale) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue trTCM coloring scale.
 Input Args:	No input arguments
 Ret Value:		The tx queue trTCM scale value.
******************************************************************************/
QDMA_TrtcmScale_t qdma_get_txqueue_trtcm_scale(void) 
{
	return gpQdmaPriv->devCfg.txQueueTrtcmScale ;
}
EXPORT_SYMBOL(qdma_get_txqueue_trtcm_scale) ;

/******************************************************************************
 Descriptor:	It's used to set the value of tx queue TRTCM parameter. 
 				The parameter includes four types, CIR, CBS, PIR and PBS.
 				And total 32 trtcm parameters are configurable.
 Input Args:	arg1: The pointer of the tx queue TRTCM parameter struct.
 				The unit of the CIR, PIR is 64Kbps and the default scale for 
 				PBS, CBS is 128 Byte.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txqueue_trtcm_params(QDMA_TxQueueTrtcm_T *pTxQueueTrtcm)
{
	int ret = 0 ;
	
	if(pTxQueueTrtcm->tsid<0 || pTxQueueTrtcm->tsid>=32) {
		return -EINVAL ;
	}
	if((ret = qdmaSetTxQueueTrtcmConfig(pTxQueueTrtcm->tsid, QDMA_TRTCM_PARAM_CIR, gpQdmaPriv->devCfg.txQueueTrtcmScale, pTxQueueTrtcm->cirParamValue)) < 0) {
		return ret ;
	}
	if((ret = qdmaSetTxQueueTrtcmConfig(pTxQueueTrtcm->tsid, QDMA_TRTCM_PARAM_CBS, gpQdmaPriv->devCfg.txQueueTrtcmScale, pTxQueueTrtcm->cbsParamValue)) < 0) {
		return ret ;
	}
	if((ret = qdmaSetTxQueueTrtcmConfig(pTxQueueTrtcm->tsid, QDMA_TRTCM_PARAM_PIR, gpQdmaPriv->devCfg.txQueueTrtcmScale, pTxQueueTrtcm->pirParamValue)) < 0) {
		return ret ;
	}
	if((ret = qdmaSetTxQueueTrtcmConfig(pTxQueueTrtcm->tsid, QDMA_TRTCM_PARAM_PBS, gpQdmaPriv->devCfg.txQueueTrtcmScale, pTxQueueTrtcm->pbsParamValue)) < 0) {
		return ret ;
	}
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_set_txqueue_trtcm_params) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx queue TRTCM parameter. 
 				The parameter includes four types, CIR, CBS, PIR and PBS.
 				And total 32 trtcm parameters are configurable.
 Input Args:	arg1: The pointer of the tx queue TRTCM parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_txqueue_trtcm_params(QDMA_TxQueueTrtcm_T *pTxQueueTrtcm)
{
	int value ;
	
	if(pTxQueueTrtcm->tsid<0 || pTxQueueTrtcm->tsid>=32) {
		return -EINVAL ;
	}
	
	if((value = qdmaGetTxQueueTrtcmConfig(pTxQueueTrtcm->tsid, gpQdmaPriv->devCfg.txQueueTrtcmScale, QDMA_TRTCM_PARAM_CIR)) < 0) {
		return -EFAULT ;
	}
	pTxQueueTrtcm->cirParamValue = value ;
	
	if((value = qdmaGetTxQueueTrtcmConfig(pTxQueueTrtcm->tsid, gpQdmaPriv->devCfg.txQueueTrtcmScale, QDMA_TRTCM_PARAM_CBS)) < 0) {
		return -EFAULT ;
	}
	pTxQueueTrtcm->cbsParamValue = value ;

	if((value = qdmaGetTxQueueTrtcmConfig(pTxQueueTrtcm->tsid, gpQdmaPriv->devCfg.txQueueTrtcmScale, QDMA_TRTCM_PARAM_PIR)) < 0) {
		return -EFAULT ;
	}
	pTxQueueTrtcm->pirParamValue = value ;

	if((value = qdmaGetTxQueueTrtcmConfig(pTxQueueTrtcm->tsid, gpQdmaPriv->devCfg.txQueueTrtcmScale, QDMA_TRTCM_PARAM_PBS)) < 0) {
		return -EFAULT ;
	}
	pTxQueueTrtcm->pbsParamValue = value ;
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_get_txqueue_trtcm_params) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable GPON trTCM coloring function. 
 Input Args:	arg1: Enable/disable TCONT trTCM coloring function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_gpon_trtcm_mode(QDMA_Mode_t trtcmMode)
{
	uint base = gpQdmaPriv->csrBaseAddr ;

	if(trtcmMode == QDMA_ENABLE) {
		qdmaEnableGponColorMode(base) ;
	} else {
		qdmaDisableGponColorMode(base) ;
	}
}
EXPORT_SYMBOL(qdma_set_gpon_trtcm_mode) ;

/******************************************************************************
 Descriptor:	It's used to set TCONT trTCM coloring scale. Scaled factor for 
                TCONT trTCM coloring parameters's value in the power of 2. The
                final value will be PARA_SCALE * PARA_VALUE.
 Input Args:	arg1: scale value for TCONT trTCM coloring function 
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_gpon_trtcm_scale(QDMA_TrtcmScale_t trtcmScale)
{
	if(trtcmScale<0 || trtcmScale>=QDMA_TRTCM_SCALE_MAX_ITEMS) {
		return -EINVAL ;
	}
	
	gpQdmaPriv->devCfg.gponTrtcmScale = trtcmScale ;
	
	return 0 ;
}
EXPORT_SYMBOL(qdma_set_gpon_trtcm_scale) ;

/******************************************************************************
 Descriptor:	It's used to set TCONT trTCM coloring scale. The trTCM coloring
                parameter configuration command for GPON DBA report. before 
                setting this bit, the desired CHN_IDX and PARAM_TYPE has to be 
                specfiied. The read back trTCM parameter is in trTCM_VALUE 
                register or the to-be-written trTCM parameter has to be set into
                trTCM_VALUE register. 
 Input Args:	arg1: The pointer of the TCONT TRTCM parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_gpon_trtcm_params(QDMA_TcontTrtcm_T *pTcontTrtcm)
{
	int ret = 0 ;
	
	if(pTcontTrtcm->channel<0 || pTcontTrtcm->channel>=16) {
		return -EINVAL ;
	}
	
	if((ret = qdmaSetGponTrtcmConfig(pTcontTrtcm->channel, QDMA_TRTCM_PARAM_CIR, gpQdmaPriv->devCfg.gponTrtcmScale, pTcontTrtcm->cirParamValue)) < 0) {
		goto out ;
	}
	if((ret = qdmaSetGponTrtcmConfig(pTcontTrtcm->channel, QDMA_TRTCM_PARAM_CBS, gpQdmaPriv->devCfg.gponTrtcmScale, pTcontTrtcm->cbsParamValue)) < 0) {
		goto out ;
	}
	if((ret = qdmaSetGponTrtcmConfig(pTcontTrtcm->channel, QDMA_TRTCM_PARAM_PIR, gpQdmaPriv->devCfg.gponTrtcmScale, pTcontTrtcm->pirParamValue)) < 0) {
		goto out ;
	}
	if((ret = qdmaSetGponTrtcmConfig(pTcontTrtcm->channel, QDMA_TRTCM_PARAM_PBS, gpQdmaPriv->devCfg.gponTrtcmScale, pTcontTrtcm->pbsParamValue)) < 0) {
		goto out ;
	}
	
out:
	return ret ;	
}
EXPORT_SYMBOL(qdma_set_gpon_trtcm_params) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx queue TRTCM parameter. 
 				The parameter includes four types, CIR, CBS, PIR and PBS.
 				And total 32 trtcm parameters are configurable.
 Input Args:	arg1: The pointer of the tx queue TRTCM parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_gpon_trtcm_params(QDMA_TcontTrtcm_T *pTcontTrtcm)
{
	int ret = 0 ;
	
	if(pTcontTrtcm->channel<0 || pTcontTrtcm->channel>=16) {
		return -EINVAL ;
	}
	
	if((ret = qdmaGetGponTrtcmConfig(pTcontTrtcm->channel, QDMA_TRTCM_PARAM_CIR)) < 0) {
		goto out ;
	}
	pTcontTrtcm->cirParamValue = ret ;
	
	if((ret = qdmaGetGponTrtcmConfig(pTcontTrtcm->channel, QDMA_TRTCM_PARAM_CBS)) < 0) {
		goto out ;
	}
	pTcontTrtcm->cbsParamValue = ret ;

	if((ret = qdmaGetGponTrtcmConfig(pTcontTrtcm->channel, QDMA_TRTCM_PARAM_PIR)) < 0) {
		goto out ;
	}
	pTcontTrtcm->pirParamValue = ret ;

	if((ret = qdmaGetGponTrtcmConfig(pTcontTrtcm->channel, QDMA_TRTCM_PARAM_PBS)) < 0) {
		goto out ;
	}
	pTcontTrtcm->pbsParamValue = ret ;

out:
	return ret ;
}
EXPORT_SYMBOL(qdma_get_gpon_trtcm_params) ;

/******************************************************************************
 Descriptor:	It's used to set the PSE PCP encoding/decoding mode.
 Input Args:	arg1: The PCP type. (PSE_PCP_TYPE_CDM_TX/PSE_PCP_TYPE_CDM_RX/PSE_PCP_TYPE_GDM_RX)
 				arg2: The PCP encoding/decoding mode. (PSE_PCP_MODE_DISABLE/PSE_PCP_MODE_8B0D/
 				PSE_PCP_MODE_7B1D/PSE_PCP_MODE_6B2D/PSE_PCP_MODE_5B3D)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_pse_set_pcp_config(PSE_PcpType_t pcpType, PSE_PcpMode_t pcpMode)
{
	if(pcpType!=PSE_PCP_TYPE_CDM_TX && pcpType!=PSE_PCP_TYPE_CDM_RX && pcpType!=PSE_PCP_TYPE_GDM_RX) {
		return -EINVAL ;
	}
	
	if(pcpMode!=PSE_PCP_MODE_DISABLE && pcpMode!=PSE_PCP_MODE_8B0D && pcpMode!=PSE_PCP_MODE_7B1D && pcpMode!=PSE_PCP_MODE_6B2D && pcpMode!=PSE_PCP_MODE_5B3D) {
		return -EINVAL ;
	}
	
	return pseSetWanPcpConfig((unchar)pcpType, (unchar)pcpMode) ;
}
EXPORT_SYMBOL(qdma_pse_set_pcp_config) ;

/******************************************************************************
 Descriptor:	It's used to get the PSE PCP encoding/decoding mode.
 Input Args:	arg1: The PCP type. (PSE_PCP_TYPE_CDM_TX/PSE_PCP_TYPE_CDM_RX/PSE_PCP_TYPE_GDM_RX)
 Ret Value:		>=0: the pcp mode, and otherwise failed.
******************************************************************************/
PSE_PcpMode_t qdma_pse_get_pcp_config(PSE_PcpType_t pcpType)
{
	if(pcpType!=PSE_PCP_TYPE_CDM_TX && pcpType!=PSE_PCP_TYPE_CDM_RX && pcpType!=PSE_PCP_TYPE_GDM_RX) {
		return -EINVAL ;
	}
	
	return (PSE_PcpMode_t)pseGetWanPcpConfig((unchar)pcpType) ;
}
EXPORT_SYMBOL(qdma_pse_get_pcp_config) ;


#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */





