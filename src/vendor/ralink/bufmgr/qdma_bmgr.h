#ifndef _QDMA_BMGR_H_
#define _QDMA_BMGR_H_

#include <linux/netdevice.h>
#include <linux/types.h>
#include <linux/string.h>
#include "qdma_dev.h"
#include "qdma_api.h"

#define CHK_BUF() 		pos = begin + index ; \
						if(pos < off) { \
							index = 0 ; \
							begin = pos ; \
						} \
						if(pos > off + count) \
							goto done ;
			
typedef enum {
	DBG_ERR ,
	DBG_ST ,
	DBG_WARN ,
	DEG_MSG 
} QDMA_DebugLevel_t ;

#ifdef CONFIG_DEBUG
	#define QDMA_MSG(level, F, B...)	{ \
											if(gpQdmaPriv->devCfg.dbgLevel >= level) 	\
												printk("%s [%d]: " F, strrchr(__FILE__, '/')+1, __LINE__, ##B) ; \
										}
	#define QDMA_ERR(F, B...)			printk("%s [%d]: " F, strrchr(__FILE__, '/')+1, __LINE__, ##B)
	#define QDMA_LOG(F, B...)			printk("%s [%d]: " F, strrchr(__FILE__, '/')+1, __LINE__, ##B)
#else
	#define QDMA_MSG(level, F, B...)			
	#define QDMA_ERR(F,B...)			printk(F, ##B) ; 
	#define QDMA_LOG(F,B...)			printk(F, ##B) ; 
#endif


#define CONFIG_IRQ_DEF_VALUE			(0xFFFFFFFF)


#ifdef MT7510_DMA_DSCP_CACHE
	#define K0_TO_K1(x)				(((uint)(x)) | 0xa0000000)
	#define K1_TO_PHY(x)			(((uint)(x)) & 0x1fffffff)
	
	#define TXDSP_PADDR_START_BASE	0xbfb0087c
	#define TXDSP_VADDR_START_BASE	(TXDSP_PADDR_START_BASE+0x4)
	#define TXDSP_VADDR_END_BASE	(TXDSP_PADDR_START_BASE+0x8)
	#define RXDSP_PADDR_START_BASE	(TXDSP_PADDR_START_BASE+0xc)
	#define RXDSP_VADDR_START_BASE	(TXDSP_PADDR_START_BASE+0x10)
	#define RXDSP_VADDR_END_BASE	(TXDSP_PADDR_START_BASE+0x14)
	
	#define CACHE_LINE_VADDR_OFFSET 0x8000000 //128M
	
	#define TO_CACHE_LINE_VADDR(x)	((x) & (~(0x1 << 31)))  //Use virtual address > 128M, and clear the 31 bit
	#define	CACHE_LINE_SIZE			(32)
#endif /* MT7510_DMA_DSCP_CACHE */


/***************************************
 struct definition
***************************************/
struct QDMA_DscpInfo_S {
	QDMA_DMA_DSCP_T				*dscpPtr ;
	uint						dscpIdx ;
	void						*msgPtr ;
	struct sk_buff				*skb ;
	struct QDMA_DscpInfo_S		*next ;
} ;

typedef struct {
	struct {
		unchar		isTxPolling			:1 ;
		unchar		isRxPolling			:1 ;
		unchar		isRxNapi			:1 ;
		unchar		isIsrRequest		:1 ;
		unchar 		resv1				:4 ;
	} flags ;
	unchar		txRecycleThreshold ;
#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
	unchar		txQueueTrtcmScale ;
	unchar		gponTrtcmScale ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
#ifdef CONFIG_DEBUG
	unchar		dbgLevel ;
#endif /* CONFIG_DEBUG */
#ifdef CONFIG_SUPPORT_SELF_TEST
	unchar		txDscpDoneBit ;
	unchar		rxDscpDoneBit ;
	ushort		waitTxMaxNums ;
	ushort		countDown ;
	unchar		rxMsgLens ;
#endif /* CONFIG_SUPPORT_SELF_TEST */	
	int (*bmXmitCallbackFunction)(void *, struct sk_buff *) ;
	int (*bmRecvCallbackFunction)(void *, uint, struct sk_buff *, uint) ;
	int (*bmEventCallbackFunction)(QDMA_EventType_t) ;
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	int (*bmXmitMsgCallbackFunction)(void *, struct sk_buff *) ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
	void (*bmPtmIntHandler)(void) ;
	void (*bmSarIntHandler)(void) ;
	void (*bmGponMacIntHandler)(void) ;
	void (*bmEponMacIntHandler)(void) ;
	void (*bmXponPhyIntHandler)(void) ;
} BM_DevConfig_T ;


typedef struct {
	uint 	txCounts ;
	uint	txRecycleCounts ;
	uint	txQdmaDropCounts ;
	uint 	rxCounts ;
	ushort 	IrqQueueAsynchronous ;
	ushort 	txIrqQueueIdxErrs ;
	ushort 	txDscpIncorrect ;
	ushort 	txRecycleErrs ;
	ushort 	txDscpDoneErrs ;
	ushort 	rxDscpIncorrect ;
	ushort	rxPktErrs ;
	ushort 	noTxCbErrs ;
	ushort	noTxDscps ;
	ushort 	noRxCbErrs ;
	ushort	noRxDscps ;
	ushort	intRxCoherent ;
	ushort	intTxCoherent ;
	ushort	intIrqFull ;
	ushort	intNoLinkDscp ;
	ushort	intRxBuffOverrun ;
	ushort	intTxBuffUnderrun ;
	ushort	intNoRxBuff ;
	ushort	intNoTxBuff ;
	ushort	intNoRxDscp ;
	ushort	intNoTxDscp ;
	uint	intRxDone ;
	uint	intTxDone ;
} BM_Counters_T ;


typedef struct {
	uint						csrBaseAddr ;
	ushort						txDscpNum ;				/* Total TX DSCP number */
	ushort						rxDscpNum ;				/* Total RX DSCP number */
	ushort						hwFwdDscpNum ;
	ushort						irqDepth ;				/* Max depth for IRQ queue */
	uint						dscpInfoAddr ; 			/* Start pointer for DSCP information node */
	uint						irqQueueAddr ;			/* IRQ queue address */
	uint						hwFwdBaseAddr ;			/* Base address of the hardware forwarding */
	struct QDMA_DscpInfo_S		*txHeadPtr ;			/* Head node for unused tx desc. */
	struct QDMA_DscpInfo_S		*txTailPtr ;			/* Tail node for unused tx desc. */
	struct QDMA_DscpInfo_S		*txUsingPtr ;			/* TXDMA using DSCP node. */
	struct QDMA_DscpInfo_S		*rxStartPtr ;			/* Start using node for rx desc. */
	struct QDMA_DscpInfo_S		*rxEndPtr ;				/* End using node for rx desc. */
	struct QDMA_DscpInfo_S		*rxUsingPtr ;			/* RXDMA using DSCP node. */
	spinlock_t          		txLock ;				/* spin lock for Tx */
	spinlock_t          		rxLock ;				/* spin lock for Rx */
	spinlock_t					irqLock ;				/* spin lock for IRQ */
	struct tasklet_struct 		task ;	
	BM_DevConfig_T				devCfg ;
	BM_Counters_T				counters ;
} QDMA_Private_T ;

/***************************************
 function prototype definition
***************************************/
int qdma_bm_receive_packets(uint maxPkts) ;
int qdma_bm_hook_receive_buffer(void *pMsg, uint msgLen, struct sk_buff *skb) ;
int qdma_bm_recycle_receive_buffer(void) ;
int qdma_bm_transmit_packet(void *pMsg, uint msgLen, struct sk_buff *skb) ;
int qdma_bm_transmit_done(int amount) ;
int qdma_bm_recycle_transmit_buffer(void) ;
int qdma_bm_tx_polling_mode(QDMA_Mode_t txMode, unchar txThreshold) ;

/***************************************
***************************************/
extern QDMA_Private_T *gpQdmaPriv ;


#endif /* _QDMA_BMGR_H_ */

