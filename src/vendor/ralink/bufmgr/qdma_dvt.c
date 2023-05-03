#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/jiffies.h>

#include "qdma_api.h"
#include "qdma_bmgr.h"
#include "qdma_dev.h"
#include "qdma_dvt.h"

#ifdef CONFIG_SUPPORT_SELF_TEST

QDMA_DbgCounters_T counters[CONFIG_QDMA_CHANNEL][CONFIG_QDMA_QUEUE] ;
/******************************************************************************
******************************************************************************/
static void __st_dump_skb(struct sk_buff *skb) 
{
	unchar n=0, *p = (unchar *)skb->data ;
	int i ;

	//printk("Raw data for send packet. skb:0x%08x, data:0x%08x, len:%d", (uint)skb, (uint)skb->data, skb->len) ;
	for(i=0 ; i<skb->len ; i++) {
		n = i & 0x0f ;
		
		if(n == 0x00) 		printk("%.4x: ", i) ;
		else if(n == 0x08) 	printk(" ") ;

		printk("%.2x ", *p++) ; 
		
		if(n == 0x0F)	printk("\n") ;
	}
	
	if(n != 0x0F) 	printk("\n") ;
}

/******************************************************************************
******************************************************************************/
static ushort __st_in_csum(unsigned short *ptr, int nbytes)
{
	register int			sum;		/* assumes long == 32 bits */
	unsigned short			oddbyte;
	register unsigned short	answer; 	/* assumes u_short == 16 bits */

	/*
	 * Our algorithm is simple, using a 32-bit accumulator (sum),
	 * we add sequential 16-bit words to it, and at the end, fold back
	 * all the carry bits from the top 16 bits into the lower 16 bits.
	 */

	sum = 0;

	while (nbytes > 1)	{
		sum += *ptr++;
		nbytes -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nbytes == 1) {
		oddbyte = 0;		/* make sure top half is zero */
		*((unsigned char *) &oddbyte) = *(unsigned char *)ptr;   /* one byte only */
		sum += oddbyte;
	}

	/*
	 * Add back carry outs from top 16 bits to low 16 bits.
	 */

	sum  = (sum >> 16) + (sum & 0xffff);	/* add high-16 to low-16 */
	sum += (sum >> 16); 		/* add carry */
	answer = ~sum;		/* ones-complement, then truncate to 16 bits */
	return(answer);
}

/*****************************************************************************
******************************************************************************/
static int __st_prepare_rx_buff(QDMA_RxMsg_T *pRxMsg, struct sk_buff *skb) 
{
	int offset, ret = 0 ;

	/* allocate QDMA message memory */
	if(pRxMsg == NULL) {
		pRxMsg = (QDMA_RxMsg_T *)kmalloc(sizeof(QDMA_RxMsg_T), GFP_KERNEL) ;
		if(pRxMsg == NULL) {
			ret = -ENOMEM ;
			goto err ;
		}
	}
	memset(pRxMsg, 0, sizeof(QDMA_RxMsg_T *)) ;
		
	/* allocate the packet buffer */
	if(skb == NULL) {
		skb = dev_alloc_skb(CONFIG_MAX_PKT_LENS) ;
		if(skb == NULL) {
			ret = -ENOMEM ;
			goto err ;
		}
		
		/* Shift to 4 byte alignment */
		offset = ((uint)(skb->tail) & 3) ;
		if(offset) {
			skb_reserve(skb, (4 - offset)) ;
		}
	}
	
	/* call qdma hook receive buffer API to prepare for rx packets */
	/* It's used the gpQdmaPriv->devCfg.rxMsgLens that is because the rx message len 
	   for upper mac loopback and qdma loopback is different */
	ret = qdma_hook_receive_buffer(pRxMsg, gpQdmaPriv->devCfg.rxMsgLens, skb) ;
	if(ret != 0) {
		ret = -ENODEV ;
		goto err ;
	}

	return ret ;
	
err:
	if(pRxMsg)		kfree(pRxMsg) ;
	if(skb) 		dev_kfree_skb(skb) ;
	return ret ;
}


/******************************************************************************
******************************************************************************/
static void __st_print_counters(void)
{
	int i, j ;
	uint tx=0, rx=0, rx_err=0 ;
	
	msleep(2000) ;
	
	printk(" C   Q     Tx Frames    Rx OK Frames    Rx Err Frames \n") ;
	for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
		for(j=0 ; j<CONFIG_QDMA_QUEUE ; j++) {
			tx += counters[i][j].tx_frames ;
			rx += counters[i][j].rx_frames ;
			rx_err += counters[i][j].rx_err_frames ;
			
			if(counters[i][j].tx_frames) {
				printk(" %2d  %d   %12d    %12d    %12d\n", i, j, counters[i][j].tx_frames, 
															counters[i][j].rx_frames, 
															counters[i][j].rx_err_frames) ;
			}
		}
	}
	printk(" Total   %12d    %12d    %12d\n", tx, rx, rx_err) ;
	printk("\n") ;
}

/******************************************************************************
******************************************************************************/
static int __st_check_counters(void)
{
	int i, j ;
	
	for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
		for(j=0 ; j<CONFIG_QDMA_QUEUE ; j++) {
			if(counters[i][j].rx_err_frames!=0 || counters[i][j].tx_frames!=counters[i][j].rx_frames) {
				return -1 ;
			}
		}
	}

	return 0 ;
}

/******************************************************************************
******************************************************************************/
static int qdma_st_packet_gen(uint channel, uint queue, uint npackets, uint lens, unchar pattern)
{
	int i, k;
	struct sk_buff *skb;
	int tx_len;
	unchar *tx_data;
	unchar tx_seed;
	int offset = 0;
	ushort chksum;
	QDMA_TxMsg_T *pTxMsg ;
	int times ;

	QDMA_MSG(DBG_ST, "Loopback test packets=%d, lens=%d, jiffies:%d \n", npackets, lens, (uint)jiffies);

	for (i=0 ; i<npackets ; i++) {
		times = 2 ;
		pTxMsg = (QDMA_TxMsg_T *)kmalloc(sizeof(QDMA_TxMsg_T), GFP_KERNEL);
		if(pTxMsg == NULL) {
			continue ;
		} else {
			memset(pTxMsg, 0, sizeof(QDMA_TxMsg_T)) ;
			pTxMsg->channel = (channel == CONFIG_QDMA_CHANNEL)?(random32() % CONFIG_QDMA_CHANNEL):channel;
			pTxMsg->queue = (queue == CONFIG_QDMA_QUEUE)?(random32() % CONFIG_QDMA_QUEUE):queue;
			pTxMsg->fport = 0x2 ;
		}
		
		skb = dev_alloc_skb(CONFIG_MAX_PKT_LENS+2);
		if (skb == NULL)
			continue;

		if (random32() & 0x1)
			offset = 2;
		else 
			offset = 0;

		skb_reserve(skb, offset);

		if (lens == 0) {
			tx_len = random32() % (CONFIG_MAX_PKT_LENS+1);
		} else if(lens == 1) {
			tx_len = (48+i) % (CONFIG_MAX_PKT_LENS+1);
		} else {
			tx_len = lens % (CONFIG_MAX_PKT_LENS+1);
		}
		tx_len = (tx_len<48)?48:tx_len ;
			
		tx_data = skb_put(skb, tx_len);
		for (k = 0; k < 6; k++)
			tx_data[k] = 0x11;
		for (k = 6; k < 12; k++)
			tx_data[k] = 0x22;

		tx_data[12] = 0x08;
		tx_data[13] = 0x00;

		tx_data[14] = pTxMsg->channel;
		tx_data[15] = pTxMsg->queue;
		
		tx_data[16] = (tx_len)>>8;
		tx_data[17] = (tx_len);

		tx_data[18] = (i+1)>>24;
		tx_data[19] = (i+1)>>16;
		tx_data[20] = (i+1)>>8;
		tx_data[21] = (i+1);

		tx_data[22] = 0;
		tx_data[23] = 0;

		if(pattern == 0x01) {
			tx_data[24] = 0x01 ;	
			tx_seed = (unchar) random32() ;
			tx_data[25] = tx_seed;

			for(k=26 ; k<tx_len ; k++) {
				tx_seed++;
				tx_data[k] = (unchar) (tx_seed & 0xff);
			}
		} else {
			tx_data[24] = 0x00;
			tx_data[25] = pattern;

			for(k=26 ; k<tx_len ; k++) {
				tx_data[k] = pattern;
			}
		}

		chksum = __st_in_csum((unsigned short *) (skb->data), tx_len-4);
		tx_data[22] = (chksum >> 8) & 0xff;
		tx_data[23] = chksum & 0xff;

		do {
			if(i%10000 == 9999) {
				QDMA_MSG(DBG_ST, "TX PKT: there are %d packets has been sent.\n", i+1) ;
				msleep(0) ;
			}
#ifdef CONFIG_DEBUG
//				printk("Raw data for send packet, %d\n", tx_len) ;
//				__st_dump_skb(skb);
#endif /* CONFIG_DEBUG */
			if(qdma_transmit_packet(pTxMsg, sizeof(QDMA_TxMsg_T), skb) == 0) {
				break;
			}	
			msleep(200);
		} while(times--);
		if(times <= 0) {
			kfree(pTxMsg);
			dev_kfree_skb(skb);
			break ;
		}
	}
	QDMA_MSG(DBG_ST, "TX PKT: there are %d packets has been sent.\n", i);

	return 0;
}

/******************************************************************************
******************************************************************************/
static int qdma_st_cb_rx_packet(void *msg_p, uint msg_len, struct sk_buff *skb, uint rx_len)
{
	int k;
	unchar *rx_data;
	unchar rx_seed, pattern;
	int idx, channel=0, queue=0, lens ;
	QDMA_RxMsg_T *pRxMsg = (QDMA_RxMsg_T *)msg_p ;
	ushort chksum;

	skb_put(skb, rx_len) ;
	rx_data = skb->data;
	
	for (k = 0; k < 6; k++) {
		if (rx_data[k] != 0x11) {
			printk("loopback fail: dst mac unmatch\n");
			goto err;
		}
	}
	for (k = 6; k < 12; k++) {
		if (rx_data[k] != 0x22) {
			printk("loopback fail: src mac unmatch\n");
			goto err;
		}
	}

	if ((rx_data[12] != 0x08) || (rx_data[13] != 0x00)) {
		printk("loopback fail: ether type unmatch\n");
		goto err;
	}

//	if (rx_data[14] != pRxMsg->channel) {
//		printk("loopback fail: channel number unmatch\n");
//		goto err;	
//	}

	channel = rx_data[14]&0xf;
	queue = rx_data[15]&0x7;
	lens = (rx_data[16]<<8) | rx_data[17];
	idx = (rx_data[18]<<24) | (rx_data[19]<<16) | (rx_data[20]<<8) | rx_data[21];
	chksum = (rx_data[22]<<8) | rx_data[23];

	if(rx_data[24] == 0x01) {
		rx_seed = rx_data[25];
		for(k=26 ; k<rx_len ; k++) {
			rx_seed++;
			if (rx_data[k] != (unchar) (rx_seed & 0xff)) {
				printk("loopback fail: random payload unmatch pos=%04x\n", k);
				goto err;
			} 
		}
	} else {
		pattern = rx_data[25] ;
		for(k=26 ; k<rx_len ; k++) {
			if (rx_data[k] != pattern) {
				printk("loopback fail: payload unmatch pos=%04x, pattern=%.2x\n", k, pattern);
				goto err;
			} 
		}		
	}
	
	if (__st_in_csum((unsigned short *) (skb->data), lens-4) != 0) {
		printk("loopback fail: ip checksum unmatch\n");
		goto err;
	}

	counters[channel][queue].rx_frames++ ;

#ifdef CONFIG_DEBUG
//	printk("\nRaw data for receive packet, len:%d\n", skb->len) ;
//	__st_dump_skb(skb);
#endif /* CONFIG_DEBUG */
	dev_kfree_skb(skb);
	__st_prepare_rx_buff(pRxMsg, NULL) ;
	
	if((idx%10000) == 0) {
		QDMA_MSG(DBG_ST, "RX PKT: the %d packet has been received. jiffies:%d\n", idx, (uint)jiffies);
	}
	
	return 0;

err:
	printk("Raw data for receive packet, len:%d\n", skb->len) ;
	counters[channel][queue].rx_err_frames++ ;
	__st_dump_skb(skb);
	dev_kfree_skb(skb);
	__st_prepare_rx_buff(pRxMsg, NULL) ;
	return -1;
}


/*****************************************************************************
******************************************************************************/
static int qdma_st_cb_tx_finished(void *msg_p, struct sk_buff *skb)
{
	QDMA_TxMsg_T *pTxMsg = (QDMA_TxMsg_T *)msg_p ;
	uint channel, queue ;
	
	channel = pTxMsg->channel ;
	queue = pTxMsg->queue ;

	counters[channel][queue].tx_frames++;
	
	kfree(pTxMsg) ;
	dev_kfree_skb(skb) ;

	return 0 ;
}

#ifndef CONFIG_TX_POLLING_BY_MAC
/******************************************************************************
******************************************************************************/
static int qdma_st_rx_polling(void) 
{
	while(gpQdmaPriv->devCfg.flags.isRxPolling == QDMA_ENABLE) {
		qdma_receive_packets(128) ;
		msleep(random32()%10) ;
	}
	
	return 0 ;
}
#endif /* CONFIG_TX_POLLING_BY_MAC */

/****************************************************************************
 to verify the TxQoS CSR configuration
*****************************************************************************/
static void qdma_st_csr_txqos(uint times)
{
	int t, i, j ;
	QDMA_TxQosScheduler_T setTxQos[CONFIG_QDMA_CHANNEL] ;
	QDMA_TxQosScheduler_T getTxQos ;
	int result ;
	
	for(t=0 ; t<times ; t++) {
		for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
			setTxQos[i].channel = i ;
			setTxQos[i].qosType = random32()%QDMA_TXQOS_TYPE_NUMS ;
		
			for(j=0 ; j<CONFIG_QDMA_QUEUE ; j++) {
				setTxQos[i].queue[j].weight = (random32() % 254) + 1 ;
			}
		
			if(qdma_set_tx_qos(&setTxQos[i]) < 0) {
				printk("failed to set the GPON scheduler.\n") ;
				goto ret ;
			}
		} 
		
		for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
			getTxQos.channel = i ;
			if(qdma_get_tx_qos(&getTxQos) < 0) {
				printk("failed to get the GPON scheduler.\n") ;
				goto ret ;
			}
			
			result = 0 ;
			if(setTxQos[i].channel!=getTxQos.channel || setTxQos[i].qosType!=getTxQos.qosType) {
				result = 1 ;
			} else {
				for(j=0 ; j<CONFIG_QDMA_QUEUE ; j++) {
					if(setTxQos[i].queue[j].weight!=getTxQos.queue[j].weight) {
						result = 1 ;
					}
				}
			}
			
			if(result == 1) {
				printk("failed to get the GPON scheduler.\n") ;
				printk("Set: channel:%d, type:%d, weight:%d, %d, %d, %d, %d, %d, %d, %d\n", 
												setTxQos[i].channel, setTxQos[i].qosType, 
												setTxQos[i].queue[0].weight, 
												setTxQos[i].queue[1].weight, 
												setTxQos[i].queue[2].weight, 
												setTxQos[i].queue[3].weight, 
												setTxQos[i].queue[4].weight, 
												setTxQos[i].queue[5].weight, 
												setTxQos[i].queue[6].weight, 
												setTxQos[i].queue[7].weight) ;
				printk("Get: channel:%d, type:%d, weight:%d, %d, %d, %d, %d, %d, %d, %d\n", 
												getTxQos.channel, getTxQos.qosType, 
												getTxQos.queue[0].weight, 
												getTxQos.queue[1].weight, 
												getTxQos.queue[2].weight, 
												getTxQos.queue[3].weight, 
												getTxQos.queue[4].weight, 
												getTxQos.queue[5].weight, 
												getTxQos.queue[6].weight, 
												getTxQos.queue[7].weight) ;
				goto ret ;
			}
		}
		
		if((t%100) == 99) {
			printk("repeat to verify the GPON scheduler %d times.\n", t+1) ;
			msleep(0) ;
		}
	}

	printk("successful to verify the GPON scheduler configuration %d times.\n", times) ;
	
ret:
	return ;	
}

#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
/****************************************************************************
 to verify the tx queue trTCM CSR configuration
*****************************************************************************/
static void qdma_st_csr_txq_trtcm(uint times)
{
	int t, i ;
	QDMA_TxQueueTrtcm_T setTxqTrtcm[32] ;
	QDMA_TxQueueTrtcm_T getTxqTrtcm ;

	if(qdma_set_txqueue_trtcm_scale(QDMA_TRTCM_SCALE_256BYTE)) {
		printk("failed to set the TxQ trTCM scale.\n") ;
		goto ret ;
	}

	
	for(t=0 ; t<times ; t++) {
		for(i=0 ; i<32 ; i++) {
			setTxqTrtcm[i].tsid = i ;
			setTxqTrtcm[i].cirParamValue = (random32() & 0xFFFF) ;
			setTxqTrtcm[i].cbsParamValue = (random32() & 0xFFFF) ;
			setTxqTrtcm[i].pirParamValue = (random32() & 0xFFFF) ;
			setTxqTrtcm[i].pbsParamValue = (random32() & 0xFFFF) ;
		
			if(qdma_set_txqueue_trtcm_params(&setTxqTrtcm[i]) < 0) {
				printk("failed to set the TxQ trTCM parameters.\n") ;
				goto ret ;
			}
		} 
		
		for(i=0 ; i<32 ; i++) {
			getTxqTrtcm.tsid = i ;
			if(qdma_get_txqueue_trtcm_params(&getTxqTrtcm) < 0) {
				printk("failed to get the TxQ trTCM parameters.\n") ;
				goto ret ;
			}
			
			if((setTxqTrtcm[i].tsid!=getTxqTrtcm.tsid) || 
			   (setTxqTrtcm[i].cirParamValue!=getTxqTrtcm.cirParamValue) ||
			   (setTxqTrtcm[i].cbsParamValue!=getTxqTrtcm.cbsParamValue) ||
			   (setTxqTrtcm[i].pirParamValue!=getTxqTrtcm.pirParamValue) ||
			   (setTxqTrtcm[i].pbsParamValue!=getTxqTrtcm.pbsParamValue)) {
				printk("failed to verify the TxQ trTCM parameters.\n") ;
				goto ret ;
			}
		}
		
		if((t%100) == 99) {
			printk("repeat to verify the TxQ trTCM parameters %d times.\n", t+1) ;
			msleep(0) ;
		}
	}

	printk("successful to verify the TxQ trTCM parameters  %d times.\n", times) ;
	
ret:
	return ;	
}

/****************************************************************************
 to verify the GPON trTCM CSR configuration
*****************************************************************************/
static void qdma_st_csr_gpon_trtcm(uint times)
{
	int t, i ;
	QDMA_TcontTrtcm_T setGponTrtcm[CONFIG_QDMA_CHANNEL] ;
	QDMA_TcontTrtcm_T getGponTrtcm ;

	if(qdma_set_gpon_trtcm_scale(QDMA_TRTCM_SCALE_256BYTE)) {
		printk("failed to set the GPON trTCM scale.\n") ;
		goto ret ;
	}

	
	for(t=0 ; t<times ; t++) {
		for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
			setGponTrtcm[i].channel = i ;
			setGponTrtcm[i].cirParamValue = (random32() & 0xFFFF) ;
			setGponTrtcm[i].cbsParamValue = (random32() & 0xFFFF) ;
			setGponTrtcm[i].pirParamValue = (random32() & 0xFFFF) ;
			setGponTrtcm[i].pbsParamValue = (random32() & 0xFFFF) ;
		
			if(qdma_set_gpon_trtcm_params(&setGponTrtcm[i]) < 0) {
				printk("failed to set the GPON trTCM parameters.\n") ;
				goto ret ;
			}
		} 
		
		for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
			getGponTrtcm.channel = i ;
			if(qdma_get_gpon_trtcm_params(&getGponTrtcm) < 0) {
				printk("failed to get the GPON trTCM parameters.\n") ;
				goto ret ;
			}
			
			if((setGponTrtcm[i].channel!=getGponTrtcm.channel) || 
			   (setGponTrtcm[i].cirParamValue!=getGponTrtcm.cirParamValue) ||
			   (setGponTrtcm[i].cbsParamValue!=getGponTrtcm.cbsParamValue) ||
			   (setGponTrtcm[i].pirParamValue!=getGponTrtcm.pirParamValue) ||
			   (setGponTrtcm[i].pbsParamValue!=getGponTrtcm.pbsParamValue)) {
				printk("failed to verify the GPON trTCM parameters.\n") ;
				goto ret ;
			}
		}
		
		if((t%100) == 99) {
			printk("repeat to verify the GPON trTCM parameters %d times.\n", t+1) ;
			msleep(0) ;
		}
	}

	printk("successful to verify the GPON trTCM parameters %d times.\n", times) ;
	
ret:
	return ;	
}
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

/****************************************************************************
 to verify the EPON Threshold CSR configuration
*****************************************************************************/
static void qdma_st_csr_eponthr(uint times)
{
	int t, i, j, k ;
	ushort eponQueueThrValue[8][8][3] ;
	QDMA_EponQueueThreshold_T cfgEntry ;
	
	for(t=0 ; t<times ; t++) {
		for(i=0 ; i<8 ; i++) {
			for(j=0 ; j<8 ; j++) {
				for(k=0 ; k<3 ; k++) {
					eponQueueThrValue[i][j][k] = random32() & 0xFFFF ;
					
					cfgEntry.channel = i ;
					cfgEntry.queue = j ;
					cfgEntry.thrIdx = k ;
					cfgEntry.value = eponQueueThrValue[i][j][k] ;
					
					if(qdma_set_epon_queue_threshold(&cfgEntry) < 0) {
						printk("Set the EPON queue threshold failed, %d, %d, %d, %d\n", i, j, k, eponQueueThrValue[i][j][k]) ;
						goto ret ;
					}
				}
			}
		}
		
		for(i=0 ; i<8 ; i++) {
			for(j=0 ; j<8 ; j++) {
				for(k=0 ; k<3 ; k++) {
					cfgEntry.channel = i ;
					cfgEntry.queue = j ;
					cfgEntry.thrIdx = k ;

					if(qdma_get_epon_queue_threshold(&cfgEntry) < 0) {
						printk("Get the EPON queue threshold failed, %d, %d, %d\n", i, j, k) ;
						goto ret ;
					}

					if(cfgEntry.value != eponQueueThrValue[i][j][k]) {
						printk("Get wrong EPON queue threshold value, %d, %d, %d, %d, %d\n", i, j, k, eponQueueThrValue[i][j][k], cfgEntry.value) ;
						goto ret ;
					}
				}
			}
		}

		if((t%10) == 9) {
			printk("repeat to verify the EPON queue threshold %d times.\n", i+1) ;
		}
	}

	printk("successful to verify the EPON queue threshold configuration %d times.\n", times) ;
	
ret:
	return ;	
}

/****************************************************************************
 to verify read/write int mask register
*****************************************************************************/
static void qdma_st_csr_intmask(uint times)
{
	uint value = 0xA215A ;
	uint reg ;
	uint i = 0 ;
	uint base = gpQdmaPriv->csrBaseAddr ;
	
	while(times--) {
		iowrite32(value, (void __iomem *)(QDMA_CSR_INT_ENABLE(base))) ;
		
		reg = ioread32((void __iomem *)(QDMA_CSR_INT_ENABLE(base))) ;
		if(reg != value) {
			printk("========>Get the INT_ENABLE register error: Write:%.8x, Read:%.8x\n", value, reg) ;
			i++ ;
		}
	}
	
	if(i) {
		printk("Failed to verify the INT_ENABLE register read/write, %d\n", i) ;
	} else {
		printk("Successful to verify the INT_ENABLE register read/write\n") ;
	}
	
	return ;	
}

/******************************************************************************
******************************************************************************/
static int qdma_st_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int i, j ;
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;

	index += sprintf(buf+index, " C   Q     Tx Frames    Rx OK Frames    Rx Err Frames \n");
	CHK_BUF();
	for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
		for(j=0 ; j<CONFIG_QDMA_QUEUE ; j++) {
			if(counters[i][j].tx_frames) {
				index += sprintf(buf+index, " %2d  %d   %12d    %12d    %12d\n", i, j, counters[i][j].tx_frames, 
																						counters[i][j].rx_frames, 
																						counters[i][j].rx_err_frames) ;
				CHK_BUF();
			}
		}
	}

	*eof = 1;

done:
	*start = buf + (off - begin);
	index -= (off - begin);
	if (index<0) 
		index = 0;
	if (index>count) 
		index = count;
	return index;
}

/******************************************************************************
 Description:	Input a self test command and start the QDMA self testing. 
 				This test process is generated packets by QDMA, send to 
 				TXDMA, receive from RXDMA and check the receive contents.
 Proc Command:	"init": 			Init the QDMA TX/RX DSCP and driver
 				"csr1 pkts lens":	Verification for NO_RX_CPU_DSCP_INT and NO_TX_CPU_DSCP_INT
 				"csr2 pkts lens":	Verification for NO_LINK_DSCP interrupt
 				"csr3":				Verification for CHCCK_DONE bit enable/disable
 				"csr4":				Verification for for RX_DONE_INT and RX_DLY_INT
 				"csr5":				Verification for for TX_DONE_INT and TX_DLY_INT
 				"tc1 pkts lens":	Process the test case 1
 				"tc2":				Process the test case 2
 				"tc3 pkts lens":	Process the test case 3
 				"tc4 pkts lens":	Process the test case 4
 				"tc5 pkts lens":	Process the test case 5
 				"tc6 pkts lens":	Process the test case 6
 				"tcall pkts lens":	Process all test case in sequential. (tc6, tc1, tc2~tc5)
******************************************************************************/
static int qdma_st_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[64], cmd[32], subcmd[32] ;
	uint value ;
	uint packets, lens ;
	uint base = gpQdmaPriv->csrBaseAddr ;
	uint i, j, k ;
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	sscanf(val_string, "%s %s %d", cmd, subcmd, &value) ;
	packets = simple_strtol(subcmd, NULL, 0) ; 
	lens = value ;
	
	if(!strcmp(cmd, "init")) {
		uint dbg=0 ;
		QDMA_InitCfg_t initCfg ;
		
		memset(&initCfg, 0, sizeof(QDMA_InitCfg_t)) ;
		initCfg.txRecycleMode = QDMA_TX_INTERRUPT ;
		initCfg.rxRecvMode = QDMA_RX_INTERRUPT ;
		initCfg.txIrqThreshold = 20 ;
		initCfg.txIrqPTime = 100 ;
		initCfg.rxDelayInt = 0 ;
		initCfg.rxDelayPTime = 0 ;
		initCfg.cbXmitFinish = qdma_st_cb_tx_finished ;
		initCfg.cbRecvPkts = qdma_st_cb_rx_packet ;

		qdma_init(&initCfg) ;

		gpQdmaPriv->devCfg.rxMsgLens = 16 ;
#ifndef CONFIG_NEW_QDMA_CSR_OFFSET
		qdmaSetRxMsgLens(base, VAL_MSG_LEN_4_WORD) ;
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */

		do {
			if(__st_prepare_rx_buff(NULL, NULL) != 0) {
				break ;
			}
			dbg++ ;
		} while(qdma_has_free_rxdscp()) ;
		
		if(!strcmp(subcmd, "umac")) {
			qdma_loopback_mode(QDMA_LOOPBACK_UMAC) ;
			IO_SMASK(0xBFB51500, 0x1F<<20, 20, 0) ;
		} else {
			qdma_loopback_mode(QDMA_LOOPBACK_QDMA) ;
		}

		qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_ENABLE) ;

		gpQdmaPriv->devCfg.dbgLevel = 1 ;
		
	    QDMA_MSG(DBG_ST, "Prepare %d receive packet buffers in QDMA init stage.\n", dbg) ;
		QDMA_MSG(DBG_ST, "CSR info: INFO:%.8x, GLG:%.8x, INT_MASK:%.8x, TX_DSCP_BASE:%.8x, RX_DSCP_BASE:%.8x\n", 
																qdmaGetQdmaInfo(base), 
																qdmaGetGlbCfg(base), 
																qdmaGetIntMask(base),
																qdmaGetTxDscpBase(base), 
																qdmaGetRxDscpBase(base)) ;
		QDMA_MSG(DBG_ST, "CSR info: RX_CPU_IDX:%d, RX_DMA_IDX:%d, TX_CPU_IDX:%d, TX_DMA_IDX:%d\n", 
																qdmaGetRxCpuIdx(base), 
																qdmaGetRxDmaIdx(base), 
																qdmaGetTxCpuIdx(base), 
																qdmaGetTxDmaIdx(base)) ;
	} 
//	else if(!strcmp(cmd, "deinit")) {
//		uint dbg=0 ;
//		QDMA_InitCfg_t initCfg ;
//
//		qdma_loopback_mode(QDMA_LOOPBACK_DISABLE) ;		
//		qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE) ;
//		qdma_recycle_receive_buffer() ;	
//		qdma_recycle_transmit_buffer() ;
//		qdma_deinit() ;
//	} 
	/* for NO_RX_CPU_DSCP_INT and NO_TX_CPU_DSCP_INT verification */
	else if(!strcmp(cmd, "csr1")) {
		uint oldInt = qdmaGetIntMask(base) ; 
		uint channel = random32() % CONFIG_QDMA_CHANNEL ;
		uint queue = random32() % CONFIG_QDMA_QUEUE ;
		
		qdmaSetIntMask(base, (INT_STATUS_TX_DONE|INT_STATUS_NO_RX_CPU_DSCP|INT_STATUS_NO_TX_CPU_DSCP)) ;
	
		memset(counters, 0, sizeof(QDMA_DbgCounters_T)*CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE) ;
		gpQdmaPriv->counters.intNoRxDscp = 0 ;
		gpQdmaPriv->counters.intNoTxDscp = 0 ;
		gpQdmaPriv->devCfg.waitTxMaxNums = 0 ;
		gpQdmaPriv->devCfg.countDown = (random32()%(gpQdmaPriv->txDscpNum-1))+1 ;
		qdma_st_packet_gen(channel, queue, packets, lens, 0x01) ;

		//in order to set the TX_CPU_PTR to HW register
		gpQdmaPriv->devCfg.countDown = 1 ;
		gpQdmaPriv->devCfg.waitTxMaxNums = 1 ;
		qdma_st_packet_gen(channel, queue, 1, lens, 0x01) ;
		qdma_bm_receive_packets(0) ;
		
		printk("INT_CSR:%.8x, Packets:%d, NO_TX_DSCP_INT Count:%d, NO_RX_DSCP_INT Count:%d\n\n", qdmaGetIntStatus(base), packets, gpQdmaPriv->counters.intNoTxDscp, gpQdmaPriv->counters.intNoRxDscp) ;
		__st_print_counters() ;

		qdmaSetIntMask(base, oldInt) ;
	}
	/* for NO_LINK_DSCP verification */
	else if(!strcmp(cmd, "csr2")) {
		uint oldInt = qdmaGetIntMask(base) ; 
		uint channel = random32() % CONFIG_QDMA_CHANNEL ;
		uint queue = random32() % CONFIG_QDMA_QUEUE ;

		if(gpQdmaPriv->txDscpNum < 1024) {
			printk("Failed: the number of TX DSCP must larger than 1024.\n") ;
			return count ;
		}
		
		qdmaSetIntMask(base, (INT_STATUS_TX_DONE|INT_STATUS_NO_LINK_DSCP)) ;
	
		memset(counters, 0, sizeof(QDMA_DbgCounters_T)*CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE) ;
		gpQdmaPriv->counters.intNoLinkDscp = 0 ;
		qdma_st_packet_gen(channel, queue, packets, lens, 0x01) ;
		qdma_bm_receive_packets(0) ;
		printk("INT_CSR:%.8x, Packets:%d, NO_LINK_DSCP_INT Count:%d\n\n", qdmaGetIntStatus(base), packets, gpQdmaPriv->counters.intNoLinkDscp) ;
		__st_print_counters() ;

		qdmaSetIntMask(base, oldInt) ;
	}
	/* CHCCK_DONE bit enable/disable verification  */
	else if(!strcmp(cmd, "csr3")) {
		uint oldInt = qdmaGetIntMask(base) ; 
		uint oldGlb = qdmaGetGlbCfg(base) ;
		uint channel = random32() % CONFIG_QDMA_CHANNEL ;
		uint queue = random32() % CONFIG_QDMA_QUEUE ;
		uint csr[2] = {(oldGlb&=~GLB_CFG_CHECK_DONE), (oldGlb|GLB_CFG_CHECK_DONE)} ;

		qdmaSetIntMask(base, (INT_STATUS_TX_DONE|INT_MASK_RX_COHERENT|INT_MASK_TX_COHERENT)) ;
		qdmaSetGlbCfg(base, (oldGlb&=~GLB_CFG_CHECK_DONE)) ;

		memset(counters, 0, sizeof(QDMA_DbgCounters_T)*CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE) ;
		
		//set all DSCP done bit to 1
		for(i=0 ; i<(gpQdmaPriv->txDscpNum+gpQdmaPriv->rxDscpNum) ; i++) {
			((struct QDMA_DscpInfo_S *)gpQdmaPriv->dscpInfoAddr + i)->dscpPtr->ctrl.done = 1 ;
		}
		gpQdmaPriv->devCfg.rxDscpDoneBit = 1 ;
		gpQdmaPriv->devCfg.txDscpDoneBit = 1 ;
		
		for(i=0 ; i<2 ; i++) {
			qdmaSetGlbCfg(base, csr[i]) ;
			
			gpQdmaPriv->counters.intRxCoherent = 0 ;
			gpQdmaPriv->counters.intTxCoherent = 0 ;
			for(j=0 ; j<packets/500 ; j++) {
				qdma_st_packet_gen(channel, queue, 500, 0, 0x01) ;
				qdma_bm_receive_packets(500) ;
			}
			printk("CHECK_DONE:%d, GLB_CSR:%.8x, INT_CSR:%.8x, RX_COHERENT_INT Count:%d, TX_COHERENT_INT Count:%d\n\n", i, qdmaGetGlbCfg(base), qdmaGetIntStatus(base), gpQdmaPriv->counters.intRxCoherent, gpQdmaPriv->counters.intTxCoherent) ;
		}
		__st_print_counters() ;

		//clear all DSCP done bit to 0
		for(i=0 ; i<(gpQdmaPriv->txDscpNum+gpQdmaPriv->rxDscpNum) ; i++) {
			((struct QDMA_DscpInfo_S *)gpQdmaPriv->dscpInfoAddr + i)->dscpPtr->ctrl.done = 0 ;
		}
		gpQdmaPriv->devCfg.rxDscpDoneBit = 0 ;
		gpQdmaPriv->devCfg.txDscpDoneBit = 0 ;
		
		qdmaClearIntStatus(base, 0xFFFFFFFF) ;
		qdmaSetGlbCfg(base, oldGlb) ;
		qdmaSetIntMask(base, oldInt) ;
	} 
	/* for RX_DONE_INT and RX_DLY_INT verification */
	else if(!strcmp(cmd, "csr4")) {
		uint oldDly = qdmaGetDelayIntCfg(base) ;
		uint channel = random32() % CONFIG_QDMA_CHANNEL ;
		uint queue = random32() % CONFIG_QDMA_QUEUE ;
		uint csr[5] = {0, (DLY_INT_RXDLY_INT_EN | (25<<DLY_INT_RXMAX_PINT_SHIFT) | (0<<DLY_INT_RXMAX_PTIME_SHIFT)), 
						  (DLY_INT_RXDLY_INT_EN | (50<<DLY_INT_RXMAX_PINT_SHIFT) | (0<<DLY_INT_RXMAX_PTIME_SHIFT)), 
						  (DLY_INT_RXDLY_INT_EN | (50<<DLY_INT_RXMAX_PINT_SHIFT) | (80<<DLY_INT_RXMAX_PTIME_SHIFT)), 
						  (DLY_INT_RXDLY_INT_EN | (50<<DLY_INT_RXMAX_PINT_SHIFT) | (200<<DLY_INT_RXMAX_PTIME_SHIFT))} ;
		
		for(i=0 ; i<5 ; i++) {
			qdmaSetDelayIntCfg(base, csr[i]) ;
			gpQdmaPriv->counters.intRxDone = 0 ;
			qdma_st_packet_gen(channel, queue, 1000, 0, 0x01) ;
			printk("Rx Delay Pkt:%d, Time:%d*20us, DlyCSR:%.8x, Packets:%d, RX_INT Count:%d\n\n", 
										((qdmaGetDelayIntCfg(base)&DLY_INT_RXMAX_PINT_MASK)>>DLY_INT_RXMAX_PINT_SHIFT) ,
										((qdmaGetDelayIntCfg(base)&DLY_INT_RXMAX_PTIME_MASK)>>DLY_INT_RXMAX_PTIME_SHIFT),
										qdmaGetDelayIntCfg(base), packets, gpQdmaPriv->counters.intRxDone) ;
			msleep(2000) ;
		}
		qdmaSetDelayIntCfg(base, oldDly) ;
	}
	/* for TX_DONE_INT and TX_DLY_INT verification */
	else if(!strcmp(cmd, "csr5")) {
		uint oldGlb = qdmaGetGlbCfg(base) ;
		uint oldDly = qdmaGetDelayIntCfg(base) ;
		uint channel = random32() % CONFIG_QDMA_CHANNEL ;
		uint queue = random32() % CONFIG_QDMA_QUEUE ;
		uint csr[5] = {0, (DLY_INT_TXDLY_INT_EN | (25<<DLY_INT_TXMAX_PINT_SHIFT) | (0<<DLY_INT_TXMAX_PTIME_SHIFT)), 
						  (DLY_INT_TXDLY_INT_EN | (50<<DLY_INT_TXMAX_PINT_SHIFT) | (0<<DLY_INT_TXMAX_PTIME_SHIFT)), 
						  (DLY_INT_TXDLY_INT_EN | (50<<DLY_INT_TXMAX_PINT_SHIFT) | (80<<DLY_INT_TXMAX_PTIME_SHIFT)), 
						  (DLY_INT_TXDLY_INT_EN | (50<<DLY_INT_TXMAX_PINT_SHIFT) | (200<<DLY_INT_TXMAX_PTIME_SHIFT))} ;
		
		qdmaSetGlbCfg(base, (oldGlb&=~GLB_CFG_IRQ_EN)) ;
		for(i=0 ; i<5 ; i++) {
			qdmaSetDelayIntCfg(base, csr[i]) ;
			gpQdmaPriv->counters.intTxDone = 0 ;
			qdma_st_packet_gen(channel, queue, 200, 0, 0x01) ;
			printk("Tx Delay Pkt:%d, Time:%d*20us, DlyCSR:%.8x, Packets:%d, TX_INT Count:%d\n\n", 
										((qdmaGetDelayIntCfg(base)&DLY_INT_TXMAX_PINT_MASK)>>DLY_INT_TXMAX_PINT_SHIFT) ,
										((qdmaGetDelayIntCfg(base)&DLY_INT_TXMAX_PTIME_MASK)>>DLY_INT_TXMAX_PTIME_SHIFT),
										qdmaGetDelayIntCfg(base), packets, gpQdmaPriv->counters.intTxDone) ;
			msleep(2000) ;
		}
		qdmaSetGlbCfg(base, oldGlb) ;
		qdmaSetDelayIntCfg(base, oldDly) ;
		printk("Please reboot the system to verify another test case\n") ;
	} 
	/* for other csr verification */
	else if(!strcmp(cmd, "csr")) {
		if(!strcmp(subcmd, "txqos")) {
			qdma_st_csr_txqos(value) ;
		} else
#ifdef CONFIG_NEW_QDMA_CSR_OFFSET
		if(!strcmp(subcmd, "txqtrtcm")) {
			qdma_st_csr_txq_trtcm(value) ;
		} else if(!strcmp(subcmd, "gpontrtcm")) {
			qdma_st_csr_gpon_trtcm(value) ;
		} else
#endif /* CONFIG_NEW_QDMA_CSR_OFFSET */
		if(!strcmp(subcmd, "eponthr")) {
			qdma_st_csr_eponthr(value) ;
		} else if(!strcmp(subcmd, "intmask")) {
			qdma_st_csr_intmask(value) ;	
		}
	}
	else if(!strcmp(cmd, "tc0")) {
		uint channel = random32() % CONFIG_QDMA_CHANNEL ;
		uint queue = random32() % CONFIG_QDMA_QUEUE ;

		memset(counters, 0, sizeof(QDMA_DbgCounters_T)*CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE) ;
		qdma_st_packet_gen(channel, queue, packets, lens, 0x01) ;
		__st_print_counters() ;
	} 
	else {
		if(!strcmp(cmd, "tc6") || !strcmp(cmd, "tcall")) {
			uint grade[3] = {128, 512, 1024} ;
			uint oldInt, depth ;
		
			oldInt = qdmaGetIntMask(base) ;
			depth = qdmaGetIrqDepth(base) ;		
			qdmaSetIntMask(base, (INT_MASK_RX_DONE|INT_MASK_IRQ_FULL)) ;
	
			memset(counters, 0, sizeof(QDMA_DbgCounters_T)*CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE) ;
			for(i=0 ; i<3 ; i++) {
				if(grade[i] >= gpQdmaPriv->txDscpNum)
					continue ;
				
				qdmaSetIrqDepth(base, grade[i]) ;
				printk("Case 6-%d: IRQ Queue Depth:%d, Pattern:RAND, Lens:RAND, Packets:%d\n", i+1, grade[i], packets) ;
				qdma_st_packet_gen(CONFIG_QDMA_CHANNEL, CONFIG_QDMA_QUEUE, packets/3, 0, 0x01) ;
				qdma_bm_transmit_done(0) ;
				msleep(1000) ;
				if(__st_check_counters() == 0) {
					printk("Grade Nums:%d, Packets:%d, Test Result:Pass\n", grade[i], packets/3) ;
				} else {
					printk("Grade Nums:%d, Packets:%d, Test Result:Failed\n", grade[i], packets/3) ;
				}
			}
			__st_print_counters() ;
			
			qdmaSetIrqDepth(base, depth) ;
			qdmaSetIntMask(base, oldInt) ;
		}  
#ifndef CONFIG_TX_POLLING_BY_MAC
		if(!strcmp(cmd, "tc1") || !strcmp(cmd, "tcall")) {
			uint oldInt ;
		
			//oldInt = qdmaGetIntMask(base) ;
			//qdmaSetIntMask(base, 0) ;
			
			qdma_txdscp_recycle_mode(QDMA_TX_POLLING, 32) ;		
			qdma_receive_packet_mode(QDMA_RX_POLLING) ;
		
			printk("Case 1: Polling Mode for TX/RX, Pattern:RAND, Lens:RAND, Packets:%d\n", packets) ;
			kernel_thread((int (*)(void *))qdma_st_rx_polling, NULL, 0) ;
			memset(counters, 0, sizeof(QDMA_DbgCounters_T)*CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE) ;
			qdma_st_packet_gen(CONFIG_QDMA_CHANNEL, CONFIG_QDMA_QUEUE, packets, lens, 0x01) ;
			msleep(3000) ;
			__st_print_counters() ;
		
			qdma_txdscp_recycle_mode(QDMA_TX_INTERRUPT, 0) ;		
			qdma_receive_packet_mode(QDMA_RX_INTERRUPT) ;
		//	qdmaSetIntMask(base, oldInt) ;
		}  
#endif /* CONFIG_TX_POLLING_BY_MAC */
		if(!strcmp(cmd, "tc2") || !strcmp(cmd, "tcall")) {
			unchar pattern[5] = {0x00, 0xFF, 0x5A, 0xA5, 0x01} ;
	
			printk("Case 2: (Channel, Queue, Pattern, Lens) Any Combination, Packets:%d\n", 100000*16*9*5) ;
			for(i=0 ; i<CONFIG_QDMA_CHANNEL ; i++) {
				for(j=0 ; j<CONFIG_QDMA_QUEUE ; j++) {
					for(k=0 ; k<5 ; k++) {
						memset(counters, 0, sizeof(QDMA_DbgCounters_T)*CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE) ;
						qdma_st_packet_gen(i, j, 100000, 1, pattern[k]) ;
						msleep(1000) ;
						if(counters[i][j].tx_frames == counters[i][j].rx_frames) {
							printk("Channel:%d, Queue:%d, Pattern:%.2x, Lens:48~2000, Packets:%d, Test Result:Pass\n\n", i, j, pattern[k], counters[i][j].tx_frames) ;
						} else {
							printk("Channel:%d, Queue:%d, Pattern:%.2x, Lens:48~2000, Packets:%d, Test Result:Failed\n\n", i, j, pattern[k], counters[i][j].tx_frames) ;
						}
					}
				}
			}		
		}
		if(!strcmp(cmd, "tc3") || !strcmp(cmd, "tcall")) {
			unchar pattern[5] = {0x00, 0xFF, 0x5A, 0xA5, 0x01} ;
			
			memset(counters, 0, sizeof(QDMA_DbgCounters_T)*CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE) ;
			printk("Case 3: RX_2B_Offset:%s, Channel:RAND, Queue:RAND, Lens:Sequential, Pattern:0x00, 0xFF, 0x5A, 0xA5, RAND, Packets:%d\n", (qdmaIsSetRx2bOffset(base))?"Enable":"Disable", packets) ;
			for(i=0 ; i<5 ; i++) {
				qdma_st_packet_gen(CONFIG_QDMA_CHANNEL, CONFIG_QDMA_QUEUE, (packets/5), 1, pattern[i]) ;
				msleep(1000) ;
				if(__st_check_counters() == 0) {
					printk("Pattern:%.2x, Packets:%d, Test Result:Pass\n\n", pattern[i], packets/5) ;
				} else {
					printk("Pattern:%.2x, Packets:%d, Test Result:Failed\n\n", pattern[i], packets/5) ;
				}
			}
			__st_print_counters() ;
		}
		if(!strcmp(cmd, "tc4") || !strcmp(cmd, "tcall")) {
			unchar bs[4] = {VAL_BST_4_DWORD, VAL_BST_8_DWORD, VAL_BST_16_DWARD, VAL_BST_32_DWARD} ;
			
			memset(counters, 0, sizeof(QDMA_DbgCounters_T)*CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE) ;
			printk("Case 4: Pattern:RAND, Lens:RAND, Burst Size:4, 8, 16, 32, Packets:%d\n", packets) ;
			for(i=0 ; i<4 ; i++) {
				qdmaSetBurstSize(base, bs[i])	;
				msleep(1000) ;
				
				qdma_st_packet_gen(CONFIG_QDMA_CHANNEL, CONFIG_QDMA_QUEUE, packets/4, 0, 0x01) ;
				msleep(1000) ;
				if(__st_check_counters() == 0) {
					printk("Burst Size:%d, Packets:%d, Test Result:Pass\n\n", qdmaGetBurstSize(base), packets/4) ;
				} else {
					printk("Burst Size:%d, Packets:%d, Test Result:Failed\n\n", qdmaGetBurstSize(base), packets/4) ;
				}
			}
			__st_print_counters() ;
		}
		if(!strcmp(cmd, "tc5") || !strcmp(cmd, "tcall")) {
			uint grade[5] = {100, 200, 400, 1000, 0} ;
			
			memset(counters, 0, sizeof(QDMA_DbgCounters_T)*CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE) ;
			for(i=0 ; i<5 ; i++) {
				if(grade[i] >= gpQdmaPriv->txDscpNum)
					continue ;
					 
				gpQdmaPriv->devCfg.waitTxMaxNums = (grade[i]>=gpQdmaPriv->txDscpNum) ? (gpQdmaPriv->txDscpNum-1) : (grade[i]) ;
				gpQdmaPriv->devCfg.countDown = (gpQdmaPriv->devCfg.waitTxMaxNums) ? (gpQdmaPriv->devCfg.waitTxMaxNums) : ((random32()%(gpQdmaPriv->txDscpNum-1))+1) ;
				printk("Case 5-%d: TX_DSCP Nums:%d, Pattern:RAND, Lens:RAND, Packets:%d, waitTxMaxNums:%d, countDown:%d\n", i+1, grade[i], packets, gpQdmaPriv->devCfg.waitTxMaxNums, gpQdmaPriv->devCfg.countDown) ;
				qdma_st_packet_gen(CONFIG_QDMA_CHANNEL, CONFIG_QDMA_QUEUE, packets/5, 0, 0x01) ;
	
				//in order to set the TX_CPU_PTR to HW register
				gpQdmaPriv->devCfg.countDown = 1 ;
				gpQdmaPriv->devCfg.waitTxMaxNums = 1 ;
				qdma_st_packet_gen(CONFIG_QDMA_CHANNEL, CONFIG_QDMA_QUEUE, 1, 0, 0x01) ;
				msleep(1000) ;
				if(__st_check_counters() == 0) {
					printk("Grade Nums:%d, Packets:%d, Test Result:Pass\n\n", grade[i], packets/5) ;
				} else {
					printk("Grade Nums:%d, Packets:%d, Test Result:Failed\n\n", grade[i], packets/5) ;
				}
			}
			__st_print_counters() ;
			
			gpQdmaPriv->devCfg.countDown = 1 ;
			gpQdmaPriv->devCfg.waitTxMaxNums = 1 ;
		}
	}

	return count ;
}

#endif /* CONFIG_SUPPORT_SELF_TEST */


/******************************************************************************
******************************************************************************/
int qdma_dvt_init(void) 
{
	struct proc_dir_entry *qdma_proc;

#ifdef CONFIG_SUPPORT_SELF_TEST
	/* create proc node */
	qdma_proc = create_proc_entry("qdma/self_test", 0, NULL);
	if(qdma_proc) {
		qdma_proc->read_proc = qdma_st_read_proc;
		qdma_proc->write_proc = qdma_st_write_proc;
	}
#endif /* CONFIG_SUPPORT_SELF_TEST */

	return 0 ;
}


/******************************************************************************
******************************************************************************/
int qdma_dvt_deinit(void) 
{
#ifdef CONFIG_SUPPORT_SELF_TEST
	remove_proc_entry("qdma/self_test", NULL);
#endif /* CONFIG_SUPPORT_SELF_TEST */

	return 0 ;
}
