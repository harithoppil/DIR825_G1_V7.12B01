#ifndef _QDMA_API_H_
#define _QDMA_API_H_

#include "qdma_glb.h"

/***************************************
 enum definition
***************************************/
typedef enum {
	QDMA_CALLBACK_TX_FINISHED,
	QDMA_CALLBACK_RX_PACKET, 
	QDMA_CALLBACK_EVENT_HANDLER,
	QDMA_CALLBACK_TX_MSG, 
	QDMA_CALLBACK_PTM_HANDLER,
	QDMA_CALLBACK_SAR_HANDLER,
	QDMA_CALLBACK_GPON_MAC_HANDLER,
	QDMA_CALLBACK_EPON_MAC_HANDLER,
	QDMA_CALLBACK_XPON_PHY_HANDLER,
} QDMA_CbType_t ;

typedef enum {
	QDMA_EVENT_RECV_PKTS = 0 ,
	QDMA_EVENT_NO_RX_BUFFER ,
	QDMA_EVENT_TX_CROWDED
} QDMA_EventType_t ;

typedef enum {
	QDMA_LOOPBACK_DISABLE = 0 ,
	QDMA_LOOPBACK_QDMA ,
	QDMA_LOOPBACK_UMAC
} QDMA_LoopbackMode_t ;

typedef enum {
	QDMA_TX_POLLING = 0 ,
	QDMA_TX_INTERRUPT ,
} QDMA_RecycleMode_t ;

typedef enum {
	QDMA_RX_POLLING = 0 ,
	QDMA_RX_INTERRUPT ,
	QDMA_RX_NAPI
} QDMA_RecvMode_t ;

typedef enum {
	QDMA_DISABLE = 0 ,
	QDMA_ENABLE
} QDMA_Mode_t ;

typedef enum {
	QDMA_WAN_TYPE_GPON = 0,
	QDMA_WAN_TYPE_EPON,
	QDMA_WAN_TYPE_PTM,
	QDMA_WAN_TYPE_SAR
} QDMA_WanType_t ;

typedef enum {
	QDMA_TXQOS_WEIGHT_BY_PACKET = 0,
	QDMA_TXQOS_WEIGHT_BY_BYTE,
} QDMA_TxQosWeightType_t ;

typedef enum {
	QDMA_TXQOS_WEIGHT_SCALE_64B = 0,
	QDMA_TXQOS_WEIGHT_SCALE_16B
} QDMA_TxQosWeightScale_t ;

typedef enum {
	QDMA_TXQOS_TYPE_WRR = 0,
	QDMA_TXQOS_TYPE_SP,
	QDMA_TXQOS_TYPE_SPWRR7, 
	QDMA_TXQOS_TYPE_SPWRR6, 
	QDMA_TXQOS_TYPE_SPWRR5, 
	QDMA_TXQOS_TYPE_SPWRR4, 
	QDMA_TXQOS_TYPE_SPWRR3, 
	QDMA_TXQOS_TYPE_SPWRR2, 
	QDMA_TXQOS_TYPE_NUMS
} QDMA_TxQosType_t ;

typedef enum {
	QDMA_TRTCM_SCALE_1BYTE = 0,
	QDMA_TRTCM_SCALE_2BYTE,
	QDMA_TRTCM_SCALE_4BYTE, 
	QDMA_TRTCM_SCALE_8BYTE,
	QDMA_TRTCM_SCALE_16BYTE,
	QDMA_TRTCM_SCALE_32BYTE,
	QDMA_TRTCM_SCALE_64BYTE,
	QDMA_TRTCM_SCALE_128BYTE,
	QDMA_TRTCM_SCALE_256BYTE,
	QDMA_TRTCM_SCALE_512BYTE,
	QDMA_TRTCM_SCALE_1KBYTE,
	QDMA_TRTCM_SCALE_2KBYTE,
	QDMA_TRTCM_SCALE_4KBYTE,
	QDMA_TRTCM_SCALE_8KBYTE,
	QDMA_TRTCM_SCALE_16KBYTE,
	QDMA_TRTCM_SCALE_32KBYTE,
	QDMA_TRTCM_SCALE_MAX_ITEMS
} QDMA_TrtcmScale_t ;

typedef enum {
	QDMA_TRTCM_PARAM_CIR = 0,
	QDMA_TRTCM_PARAM_CBS,
	QDMA_TRTCM_PARAM_PIR,
	QDMA_TRTCM_PARAM_PBS
} QDMA_TrtcmParamType_t ;

typedef enum {
	QDMA_EPON_REPORT_WO_THRESHOLD = 0,
	QDMA_EPON_REPORT_ONE_THRESHOLD,
	QDMA_EPON_REPORT_TWO_THRESHOLD,
	QDMA_EPON_REPORT_THREE_THRESHOLD
} QDMA_EponReportMode_t ;

typedef enum {
	QDMA_TXQUEUE_SCALE_2_DSCP = 0,
	QDMA_TXQUEUE_SCALE_4_DSCP,
	QDMA_TXQUEUE_SCALE_8_DSCP,
	QDMA_TXQUEUE_SCALE_16_DSCP,
	QDMA_TXQUEUE_SCALE_ITEMS
} QDMA_TxQueueThresholdScale_t ;

typedef enum {
	PSE_PCP_TYPE_CDM_TX = 0,
	PSE_PCP_TYPE_CDM_RX, 
	PSE_PCP_TYPE_GDM_RX
} PSE_PcpType_t ;

typedef enum {
	PSE_PCP_MODE_DISABLE = 0,
	PSE_PCP_MODE_8B0D = 1,
	PSE_PCP_MODE_7B1D = 2, 
	PSE_PCP_MODE_6B2D = 4,
	PSE_PCP_MODE_5B3D = 8
} PSE_PcpMode_t ;

/***************************************
 structure definition
***************************************/
typedef int (*qdma_callback_xmit_finish_t)(void *, struct sk_buff *) ;
typedef int (*qdma_callback_recv_packet_t)(void *, uint, struct sk_buff *, uint) ;
typedef int (*qdma_callback_event_handler_t)(QDMA_EventType_t) ;
typedef void (*qdma_callback_int_handler_t)(void) ;
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	typedef int (*qdma_callback_xmit_msg_t)(void *, struct sk_buff *) ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */

typedef struct {
	QDMA_RecvMode_t					rxRecvMode ;
	QDMA_RecycleMode_t				txRecycleMode ;
	unchar							txRecycleThreshold ;
	unchar							txIrqThreshold ;
	ushort							txIrqPTime ;
	unchar							rxDelayInt ;
	unchar							rxDelayPTime ;
	qdma_callback_xmit_finish_t		cbXmitFinish ;
	qdma_callback_recv_packet_t		cbRecvPkts ;
	qdma_callback_event_handler_t	cbEventHandler ;
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	qdma_callback_xmit_msg_t		cbXmitMsg ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
	qdma_callback_int_handler_t		cbPtmHandler ;
	qdma_callback_int_handler_t		cbSarHandler ;
	qdma_callback_int_handler_t		cbGponMacHandler ;
	qdma_callback_int_handler_t		cbEponMacHandler ;
	qdma_callback_int_handler_t		cbXponPhyHandler ;
} QDMA_InitCfg_t ;


typedef struct {
	QDMA_Mode_t	mode ;
	unchar		chnThreshold ;
	unchar		totalThreshold ;
} QDMA_TxBufCtrl_T ;

typedef struct {
	unchar					channel ;
	QDMA_TxQosType_t		qosType ;
	struct {
		unchar				weight ;		//0 for don't care
	} queue[CONFIG_QDMA_QUEUE] ;
} QDMA_TxQosScheduler_T ;

typedef struct {
	unchar 			channel ;
	unchar			queue ;
	unchar			thrIdx ;
	ushort			value ;
} QDMA_EponQueueThreshold_T ;

typedef struct {
	unchar	channel ;
	ushort	cir ;
	ushort	cbs ;
	ushort	pir ;
	ushort	pbs ;
} QDMA_TrtcmParam_T ;

typedef struct {
	QDMA_TxQueueThresholdScale_t	maxScale ;
	QDMA_TxQueueThresholdScale_t	minScale ;
} QDMA_TxQueueCongestScale_T ;

typedef struct {
	unchar				green ;
	unchar				yellow ;
} QDMA_TxQueueDropProbability_T ;

typedef struct {
	unchar				queueIdx ;
	unchar				grnMaxThreshold ;
	unchar				grnMinThreshold ;
	unchar				ylwMaxThreshold ;
	unchar				ylwMinThreshold ;
} QDMA_TxQueueCongestThreshold_T ;

typedef struct {
	QDMA_TxQueueThresholdScale_t	maxScale ; //The scale factor for TXQ_MAX_THRESHOLD
	QDMA_TxQueueThresholdScale_t	minScale ; // The scale factor for TXQ_MIN_THRESHOLD
	unchar							grnDropProb ; //Dropped precentage value for green packets(0~100)
	unchar							ylwDropProb ; //Dropped precentage value for yellow packets(0~100)
	struct {
		unchar			grnMaxThreshold ;
		unchar			grnMinThreshold ;
		unchar			ylwMaxThreshold ;
		unchar			ylwMinThreshold ;
	} queue[CONFIG_QDMA_QUEUE] ;
} QDMA_TxQueueCongestCfg_T ;

typedef struct {
	unchar				tsid ;
	ushort				cirParamValue ; //The unit of CIR, PIR is 64Kbps
	ushort				cbsParamValue ; //The default trtcm scale of CBS,PBS is 128 Byte
	ushort				pirParamValue ; 
	ushort				pbsParamValue ;
} QDMA_TxQueueTrtcm_T ;

typedef struct {
	unchar				channel ;
	ushort				cirParamValue ;
	ushort				cbsParamValue ;
	ushort				pirParamValue ;
	ushort				pbsParamValue ;
} QDMA_TcontTrtcm_T ;

/******************************************************************************
 Descriptor:	It's used to init the QDMA software driver and hardware device.
 				This function must be called if the upper layer application wanna
 				use the QDMA to send/receive packets.
 Input Args:	The pointer of the QDMA_InitCfg_t
 Ret Value:		0: init successful otherwise failed.
******************************************************************************/
int qdma_init(QDMA_InitCfg_t *pInitCfg) ;

/******************************************************************************
 Descriptor:	It's used to deinit the QDMA software driver and hardware device.
 				This function must be called if the upper layer application wanna
 				transfer to another application.
 Input Args:	None
 Ret Value:		0: init successful otherwise failed.
******************************************************************************/
int qdma_deinit(void) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable the TXDMA, RXDMA and HWFWD mode
 Input Args:	arg1: TX DMA mode (QDMA_ENABLE/QDMA_DISABLE)
 				arg2: RX DMA mode (QDMA_ENABLE/QDMA_DISABLE)
 				arg3: Hardware forwarding mode (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value: 	No
******************************************************************************/
void qdma_dma_mode(QDMA_Mode_t txMode, QDMA_Mode_t rxMode, QDMA_Mode_t hwFwdMode) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable the QDMA loopback mode
 Input Args:	arg1: loopback mode (QDMA_LOOPBACK_DISABLE/QDMA_LOOPBACK_QDMA/QDMA_LOOPBACK_UMAC)
 Ret Value: 	No
******************************************************************************/
void qdma_loopback_mode(QDMA_LoopbackMode_t lbMode) ;

/******************************************************************************
 Descriptor:	It's used to configure the TX interrupt delay parameters.
 Input Args:	arg1: irq queue threshold (0~irqDepth). When valid irq entry 
 				len > irq queue threshold, the hardware will generate an interrupt. 
 				arg2: irq pending time (0~65535). This argument is specified max 
 				pending for the irq queue interrupt. The uint of the pending time 
 				is 20us.
 Ret Value:		0: setting successful otherwise failed.
******************************************************************************/
int qdma_set_tx_delay(unchar txIrqThreshold, ushort txIrqPtime) ;

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
int qdma_set_rx_delay(unchar rxMaxInt, unchar rxMaxPtime) ;

/******************************************************************************
 Descriptor:	It's used to register the QDMA callback functions. The QDMA 
 				driver support several callback function type that is define 
 				in QDMA_CbType_t enum.
 Input Args:	arg1: callback function type that is define in QDMA_CbType_t enum.
 				arg2: the pointer of the callback function.
 Ret Value:		0: register successful otherwise failed.
******************************************************************************/
int qdma_register_callback_function(QDMA_CbType_t type, void *pCbFun) ;

/******************************************************************************
 Description:	It's used to unregister the QDMA callback functions.
 Input Args:	arg1: callback function type that is define in QDMA_CbType_t enum.
 Ret Value:		0: unregister successful otherwise failed.
******************************************************************************/
int qdma_unregister_callback_function(QDMA_CbType_t type) ;

/******************************************************************************
 Descriptor:	It's used to check there is any unused RX DSCP.
 Input Args:	No
 Ret Value:		0: no free RX DSCP, 1: at least one RX DSCP
******************************************************************************/
int qdma_has_free_rxdscp(void) ;


/******************************************************************************
 Description:	
 Input Args:	
 Ret Value:		
******************************************************************************/
int qdma_hook_receive_buffer(void *pMsg, uint msgLen, struct sk_buff *skb) ;

/******************************************************************************
 Description:	
 Input Args:	
 Ret Value:		
******************************************************************************/
int qdma_recycle_receive_buffer(void) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
void qdma_enable_rxpkt_int(void) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
void qdma_disable_rxpkt_int(void) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packet_mode(QDMA_RecvMode_t rxMode) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packets(uint maxPkts) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_transmit_packet(void *pMsg, uint msgLen, struct sk_buff *skb) ;

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
	int qdma_txdscp_recycle_mode(QDMA_RecycleMode_t txMode) ;
#else
	int qdma_txdscp_recycle_mode(QDMA_RecycleMode_t txMode, unchar txThreshold) ;
#endif /* CONFIG_TX_POLLING_BY_MAC */

/******************************************************************************
 Descriptor:	It's used to recycle the tx dscp by upper MAC driver.
 Input Args:	arg1: tx DSCP count. 0 means recycle all of the tx dscp.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
#ifdef CONFIG_TX_POLLING_BY_MAC
	int qdma_txdscp_recycle(uint count) ;
#endif /* CONFIG_TX_POLLING_BY_MAC */

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_recycle_transmit_buffer(void) ;

/******************************************************************************
 Descriptor:	It's used to configure the TXQOS weight type and scale.
 Input Args:	arg1: setting the WRR weighting value is base on packet or byte
 					  (QDMA_TXQOS_WEIGHT_BY_PACKET/QDMA_TXQOS_WEIGHT_BY_BYTE)
 				arg2: setting the byte weighting scale(QDMA_TXQOS_WEIGHT_SCALE_64B
 				      /QDMA_TXQOS_WEIGHT_SCALE_16B). when the weigthing value 
 				      is base on packet, these argument is don't care.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_qos_weight(QDMA_TxQosWeightType_t weightBase, QDMA_TxQosWeightScale_t weightScale) ;

/******************************************************************************
 Descriptor:	It's used to get the TXQOS weight type and scale.
 Input Args:	arg1: the pointer of the weight base value 
 				      (QDMA_TXQOS_WEIGHT_BY_PACKET/QDMA_TXQOS_WEIGHT_BY_BYTE)
 				arg2: the pointer of the weight scale value
 					  (QDMA_TXQOS_WEIGHT_SCALE_64B/QDMA_TXQOS_WEIGHT_SCALE_16B)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_qos_weight(QDMA_TxQosWeightType_t *pWeightBase, QDMA_TxQosWeightScale_t *pWeightScale) ;

/******************************************************************************
 Descriptor:	It's used to configure the tx queue scheduler and queue weigth
 				for specific channel.
 Input Args:	The pointer of the tx qos scheduler struct. It includes:
 				- channel: specific the channel ID (0~15)
 				- qosType: The QoS type is define in QDMA_TxQosType_t enum.
 				- weight: The unit of WRR weight is packets.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_qos(QDMA_TxQosScheduler_T *pTxQos) ;

/******************************************************************************
 Descriptor:	It's used to set the tx queue scheduler and queue weigth
 				for specific channel.
 Input Args:	The pointer of the tx qos scheduler struct. It includes:
 				- channel: specific the channel ID (0~15)
 				- qosType: The QoS type is define in QDMA_TxQosType_t enum.
 				- weight: The unit of WRR weight is packets.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_qos(QDMA_TxQosScheduler_T *pTxQos) ;

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
int qdma_set_txbuf_threshold(QDMA_TxBufCtrl_T *pTxBufCtrl) ;

/******************************************************************************
 Descriptor:	It's used to set the EPON threshold report mode.
 Input Args:	arg1: channel number (0~7)
 				arg2: EPON threhold report mode (QDMA_EPON_REPORT_ONE_THRESHOLD/
 				QDMA_EPON_REPORT_TWO_THRESHOLD/QDMA_EPON_REPORT_THREE_THRESHOLD)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_epon_report_mode(unchar channel, QDMA_EponReportMode_t mode) ;

/******************************************************************************
 Descriptor:	It's used to get the EPON threshold report mode.
 Input Args:	arg1: channel (0~7)
 Ret Value:		QDMA_EPON_REPORT_ONE_THRESHOLD, QDMA_EPON_REPORT_TWO_THRESHOLD, 
 				QDMA_EPON_REPORT_THREE_THRESHOLD,  <0: failed.
******************************************************************************/
int qdma_get_epon_report_mode(unchar channel) ;

/******************************************************************************
 Descriptor:	It's used to set the EPON queue threshold for report.
 Input Args:	arg1: The pointer of the EPON queue threshold struct. The struct 
 				includes channel number(0~7), queue number(0~7), threshold index(0~2)
 				and queue threshold value for EPON local queue status report.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_epon_queue_threshold(QDMA_EponQueueThreshold_T *pEponQThr) ;

/******************************************************************************
 Descriptor:	It's used to get the EPON queue threshold for report.
 Input Args:	arg1: The pointer of the EPON queue threshold struct. The struct 
 				includes channel number(0~7), queue number(0~7), threshold index(0~2)
 				and queue threshold value for EPON local queue status report.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_epon_queue_threshold(QDMA_EponQueueThreshold_T *pEponQThr) ;


/******************************************************************************
 Descriptor:	It's used to retire the DSCP for specific channel.
 Input Args:	arg1: channel number (0~15)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_channel_retire(uint channel) ;

/******************************************************************************
 Descriptor:	It's used to set the channel retire mask for specific channel.
 Input Args:	arg1: channel number (0~15)
 				arg2: Enable/disable channel retire mask
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_channel_retire_mask(uint channel, QDMA_Mode_t maskMode) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable QDMA pre-fetch function. Since the
                on-chip buffer resource is limited, to prevent from Head-Of-Line
                blocking issue, 'when' to retrieving packet from External DRAM 
                to on-chip buffer will be application dependent.
 Input Args:	arg1: Enable/disable TXDMA pre-fetch function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_prefetch_mode(QDMA_Mode_t prefecthMode) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue trTCM coloring function. 
 Input Args:	arg1: Enable/disable tx queue trTCM function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_txqueue_trtcm_mode(QDMA_Mode_t trtcmMode) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue trTCM coloring mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue trTCM mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
QDMA_Mode_t qdma_get_txqueue_trtcm_mode(void) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue DEI dropped function. 
 Input Args:	arg1: Enable/disable tx queue trTCM function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_txqueue_dei_mode(QDMA_Mode_t deiDropMode) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue DEI dropped mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue DEI mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
QDMA_Mode_t qdma_get_txqueue_dei_mode(void) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue threshold dropped function. 
 Input Args:	arg1: Enable/disable tx queue threshold function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_txqueue_threshold_mode(QDMA_Mode_t thrsldDropMode) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue threshold dropped mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue threshold mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
QDMA_Mode_t qdma_get_txqueue_threshold_mode(void) ;

/******************************************************************************
 Descriptor:	It's used to set the tx queue congestion scale. 
 Input args:	arg1: The pointer of the QDMA_TxQueueCongestScale_T struct.
 				It includes two parameters.
  				1. maxScale: The scale factor for TXQ_MAX_THRESHOLD
 				2. minScale: The scale factor for TXQ_MIN_THRESHOLD
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_congestion_scale(QDMA_TxQueueCongestScale_T *pScale) ;

/******************************************************************************
 Descriptor:	It's used to set the tx queue congestion drop probability. 
 Input args:	arg1: The pointer of the QDMA_TxQueueDropProbability_T struct.
 				It includes two parameters.
 				1. green: Dropped precentage value for green packets(0~100).
 				   0 means never dropped and 100 means always dropped.
 				2. yellow: Dropped precentage value for yellow packets(0~100)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_congest_drop_probability(QDMA_TxQueueDropProbability_T *pProbability) ;

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
int qdma_set_congest_threshold(QDMA_TxQueueCongestThreshold_T *pThreshold) ;

/******************************************************************************
 Descriptor:	It's used to get the tx queue congestion configuration. 
 Input args:	arg1: The pointer of the QDMA_TxQueueCongestCfg_T struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_congest_config(QDMA_TxQueueCongestCfg_T *pCongest) ;

/******************************************************************************
 Descriptor:	It's used to set tx queue trTCM coloring scale. Scaled factor for 
                TCONT trTCM coloring parameters's value in the power of 2. The
                final value will be PARA_SCALE * PARA_VALUE.
                The trTCM scalue must be set before setting the parameters.
 Input Args:	arg1: scale value for tx queue trTCM coloring function. The 
 				default trtcm scale is 128 Byte.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txqueue_trtcm_scale(QDMA_TrtcmScale_t trtcmScale) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue trTCM coloring scale.
 Input Args:	No input arguments
 Ret Value:		The tx queue trTCM scale value.
******************************************************************************/
QDMA_TrtcmScale_t qdma_get_txqueue_trtcm_scale(void) ;

/******************************************************************************
 Descriptor:	It's used to set the value of tx queue TRTCM parameter. 
 				The parameter includes four types, CIR, CBS, PIR and PBS.
 				And total 32 trtcm parameters are configurable.
 Input Args:	arg1: The pointer of the tx queue TRTCM parameter struct.
 				The unit of the CIR, PIR is 64Kbps and the default scale for 
 				PBS, CBS is 128 Byte.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txqueue_trtcm_params(QDMA_TxQueueTrtcm_T *pTxQueueTrtcm) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx queue TRTCM parameter. 
 				The parameter includes four types, CIR, CBS, PIR and PBS.
 				And total 32 trtcm parameters are configurable.
 Input Args:	arg1: The pointer of the tx queue TRTCM parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_txqueue_trtcm_params(QDMA_TxQueueTrtcm_T *pTxQueueTrtcm) ;


/******************************************************************************
 Descriptor:	It's used to enable/disable GPON trTCM coloring function. 
 Input Args:	arg1: Enable/disable TCONT trTCM coloring function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_gpon_trtcm_mode(QDMA_Mode_t trtcmMode) ;

/******************************************************************************
 Descriptor:	It's used to set TCONT trTCM coloring scale. Scaled factor for 
                TCONT trTCM coloring parameters's value in the power of 2. The
                final value will be PARA_SCALE * PARA_VALUE.
                The trTCM scalue must be set before setting the parameters.
 Input Args:	arg1: scale value for TCONT trTCM coloring function 
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_gpon_trtcm_scale(QDMA_TrtcmScale_t trtcmScale) ;

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
int qdma_set_gpon_trtcm_params(QDMA_TcontTrtcm_T *pTcontTrtcm) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx queue TRTCM parameter. 
 				The parameter includes four types, CIR, CBS, PIR and PBS.
 				And total 32 trtcm parameters are configurable.
 Input Args:	arg1: The pointer of the tx queue TRTCM parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_gpon_trtcm_params(QDMA_TcontTrtcm_T *pTcontTrtcm) ;

/******************************************************************************
 Descriptor:	It's used to set the PSE PCP encoding/decoding.
 Input Args:	arg1: The PCP type. (PSE_PCP_TYPE_CDM_TX/PSE_PCP_TYPE_CDM_RX/PSE_PCP_TYPE_GDM_RX)
 				arg2: The PCP encoding/decoding mode. (PSE_PCP_MODE_DISABLE/PSE_PCP_MODE_8B0D/
 				PSE_PCP_MODE_7B1D/PSE_PCP_MODE_6B2D/PSE_PCP_MODE_5B3D)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_pse_set_pcp_config(PSE_PcpType_t pcpType, PSE_PcpMode_t pcpMode) ;

/******************************************************************************
 Descriptor:	It's used to get the PSE PCP encoding/decoding mode.
 Input Args:	arg1: The PCP type. (PSE_PCP_TYPE_CDM_TX/PSE_PCP_TYPE_CDM_RX/PSE_PCP_TYPE_GDM_RX)
 Ret Value:		>=0: the pcp mode, and otherwise failed.
******************************************************************************/
PSE_PcpMode_t qdma_pse_get_pcp_config(PSE_PcpType_t pcpType) ;


#endif /* _QDMA_API_H_ */
