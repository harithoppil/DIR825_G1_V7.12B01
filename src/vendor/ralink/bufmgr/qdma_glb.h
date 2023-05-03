#ifndef _QDMA_GLB_H_
#define _QDMA_GLB_H_

#define CONFIG_QDMA_CHANNEL					16
#define CONFIG_QDMA_QUEUE					8

#define CONFIG_TX_DSCP_NUM					(1024)
#define CONFIG_RX_DSCP_NUM					(512)
#define CONFIG_HWFWD_DSCP_NUM				(1024)
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	#define CONFIG_HWFWD_MSG_LENS			(0)		//0 words
#else
	#define CONFIG_HWFWD_MSG_LENS			(8)		//2 words
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
#define CONFIG_IRQ_DEPTH					(512)
#define CONFIG_MAX_PKT_LENS					(2000)

#endif /* _QDMA_GLB_H_ */

