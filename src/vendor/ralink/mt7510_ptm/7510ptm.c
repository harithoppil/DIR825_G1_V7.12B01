

#define DRV_NAME	"mt7510ptm"
#define DRV_VERSION	"2.00-NAPI"
#define DRV_RELDATE	"19.Jun.2012"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/version.h>
#include <asm/spram.h>
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/ledcetrl.h>
#include "../include/tcversion.h"
#include <linux/skbuff.h>
#include <linux/mtd/rt_flash.h>

#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif
#include <asm/tc3162/TCIfSetQuery_os.h>
#include "7510ptm.h"
#include "../bufmgr/qdma_api.h"
#include "../fe_api/fe_api.h"
#include <led.h>


/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

#define CONFIG_8021P_REMARK

#ifdef CONFIG_8021P_REMARK
#define QOS_8021p_MARK			0x0F00 	/* 8~11 bits used for 802.1p */
#define QOS_8021P_0_MARK		0x08	/* default mark is zero */
#define VLAN_HLEN			    4
#define VLAN_ETH_ALEN			6
#endif

#define DEF_PRIORITY_PKT_CHK_LEN	100

#ifndef TCSUPPORT_QOS
#define QOS_REMARKING  1  
#endif

#define TCSUPPORT_HW_QOS

#ifdef QOS_REMARKING  
#define QOS_REMARKING_MASK    	0x00000007
#define QOS_REMARKING_FLAG    	0x00000001
#endif


#ifdef TCSUPPORT_QOS
#define 	QOS_HH_PRIORITY		0x10
#define 	QOS_H_PRIORITY		0x20
#define 	QOS_M_PRIORITY		0x30
#define		QOS_L_PRIORITY		0x40

#define		NULLQOS				-1
#define 	QOS_HW_WRR			3
#define		QOS_HW_PQ			4
#endif


#if defined(WAN2LAN)
#define TX_STAG_LEN 6
#endif

/************************************************************************
*				E X T E R N A L   R E F E R E N C E S
*************************************************************************
*/
// extern unsigned long flash_base;

#if defined(WAN2LAN)
extern void macSend(uint32 chanId, struct sk_buff *skb);
extern int masko;
#endif

#ifdef TR068_LED
extern int internet_led_on;
extern int internet_trying_led_on;
#if defined(TCSUPPORT_CPU_MT7510) ||  defined(TCSUPPORT_CPU_MT7520)
extern int internet_hwnat_pktnum;
extern int internet_hwnat_timer_switch;
#endif
#endif

#ifdef LOOPBACK_SUPPORT
extern int loopback_proc_init(void);
extern void loopback_proc_remove(void);
extern int ptm_loopback_rx(
		ptmRxMsg_t *rxMsg, uint32 rxMsgLen, 
		struct sk_buff *skb, uint32 frameSize
);
extern int loopback_tx_xmit_prepare(
		struct sk_buff *skb, int *line, uint8 *bearer, 
		int *preemption, int *txq, int *needHwVlanTag, 
		uint32 *vlanTag, uint8 *ipCsIns
);

extern int stopTesting;
extern int ptmPathId, isPtmPathIdSetEnable;
#if KERNEL_2_6_36
extern uint8 dstMacAddr[];
extern unsigned int bacpHeader[];
#endif
#endif


#ifdef EXTERNAL_LOOPBACK
extern int ptm_externalLoopbackRx(
		ptmRxMsg_t *curRxMsg, ptmAdapter_t *ptmApt,
		struct sk_buff *skb, struct sk_buff **skbPtr,
		struct net_device *dev, uint frameSize
);

int mt7510_ptm_tx(struct sk_buff *, struct net_device *);
#endif

#ifdef TCSUPPORT_BONDING
extern unsigned long pcie_virBaseAddr_get(void);
#endif

#ifdef LINE_BONDING
extern uint8 maxFragSizeEn;
extern uint32 maxFragSizeLine0;
extern uint32 maxFragSizeLine1;

extern int lineBonding_init(void);
extern int lineBonding_proc_init(void);
extern void lineBonding_proc_remove(void);
extern void maxFragSize_set(void);
#endif

extern adsldev_ops* adsl_dev_ops_get(int lineId);

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/
#ifdef TR068_LED
struct ptm_stats {
	unsigned long	rx_pkts;		/* total pkts received 	*/
	unsigned long	tx_pkts;		/* total pkts transmitted	*/
};
#endif

/************************************************************************
*                        P U B L I C   D A T A
*************************************************************************
*/
uint8 defMacAddr[] = {0x00, 0x00, 0xaa, 0xbb, 0xcc, 0xff};
struct net_device *mt7510PtmDev[BEARER_NUM] = {NULL, NULL};

uint32 priPktChkLen = DEF_PRIORITY_PKT_CHK_LEN;
int priPktChk = 1;
#ifdef PTM_DEBUG
int ptmDbgLevel = 0;
#endif

#ifdef TCSUPPORT_QOS
static char qosFlag = NULLQOS;
#endif

#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
static int qos_wrr_info[5] = {0};
static uint8 maxPrio = 3;
#endif

/************************************************************************
*                       P R I V A T E   D A T A
*************************************************************************
*/
static uint8 ptmInitialized = 0;
static uint8 ptmIfaceUp[BEARER_NUM] = {0 , 0};
static uint8 ptmMsgBufqdmaCfgInitialized = 0;
static void *txMsgRingBasePtr, *rxMsgRingBasePtr;
static dma_addr_t txMsgRingPhyAddr, rxMsgRingPhyAddr;
static DEFINE_SPINLOCK(txRingLock);
static DEFINE_SPINLOCK(napiLock);
static ptmTxMsgBuf_t *txAvaiMsgHead, *txAvaiMsgTail;
#ifdef EXTERNAL_LOOPBACK
int externalLoopbackEn = 0;
module_param(externalLoopbackEn, int, 0);
#endif
#ifdef LOOPBACK_SUPPORT
int internalLoopbackEn = 0;
module_param(internalLoopbackEn, int, 0);
#endif
static int trtcmEnable = 0, trtcmTsid = 0;
static uint32 qdmaRecycleRxBuf, qdmaRecycleTxBuf;
static uint32 txMsgBufNum = BEARER_NUM * TX_QUEUE_NUM * TX_QUEUE_LEN;
static uint32 rxMsgBufNum = BEARER_NUM * RX_QUEUE_NUM * RX_QUEUE_LEN;
#ifdef TCSUPPORT_BONDING
static uint32 linesSwitchingMode = 0x7;
unsigned int pcieVirBaseAddr = 0;
unsigned int slaveBondingBaseAddr;
unsigned int slaveScuBaseAddr;
unsigned int slaveTpstcBaseAddr;
static int bondingCheckRegsFail = 0;
#endif
#ifdef TR068_LED
static struct ptm_stats ptmStats;
#endif
/************************************************************************
*        P U B L I C   F U N C T I O N
*************************************************************************
*/

void ptm_open(void);
void ptm_reset(void);
void ptm_close(void);
void bonding_recovery(void);


/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#define pause(x)					mdelay(x)

#define CHK_BUF() pos = begin + index; if (pos < off) { index = 0; begin = pos; }; if (pos > off + count) goto done;

#define WORDSWAP(a)	a = ((((a)>>24)&0xff) | (((a)>>8)&0xff00) | (((a)<<8)&0xff0000) | (((a)<<24)&0xff000000))


void dump_data(char *p, int len)
{
	char tmp[80];
	char *t = tmp;
	int i, n = 0;

	if (p == NULL)
	{
		printk("\nERROR: p is NULL at dump_data\n");
		return;
	}

	printk("data=%08lx len=%d\n", (uint32) p, len);
	for (i = 0; i < len; i++) {
		t += sprintf(t, "%02x ", *p++ & 0xff);
		if ((i & 0x0f) == 0x0f) {
			printk("%04x: %s\n", n, tmp);
			n += 16;
			t = tmp;
		}
	}
	if (i & 0x0f)
		printk("%04x: %s\n", n, tmp);
}

#ifdef TCSUPPORT_BONDING
unsigned int sBonding_reg_read(
		unsigned int reg_offset
)
{
	return read_reg_word (slaveBondingBaseAddr + reg_offset);
}

void sBonding_reg_write(
		unsigned int reg_offset, unsigned int value
)
{
	write_reg_word (slaveBondingBaseAddr + reg_offset, value);
	return;
}

static unsigned int sSCU_reg_read(
		unsigned int reg_offset
)
{
	return read_reg_word (slaveScuBaseAddr + reg_offset);
}

static void sSCU_reg_write(
		unsigned int reg_offset, unsigned int value
)
{
	write_reg_word (slaveScuBaseAddr + reg_offset, value);
	return;
}
#endif

/* "echo onOff tsid cir pir greenDropRate yellowDropRate > 
 * /proc/tc3162/ptm_ptm_trafficShapingSet" to set TRTCM.
 * onOff: 0:disable, 1:enable trtcm
 * tsid: 0~15
 * cir, pir: unit is 64Kbps
 * greenDropRate, yellowDropRate: has to be devided by 255
 */
static int ptm_trafficShapingSet_write_proc(
		struct file *file, const char *buffer, 
		unsigned long count, void *data
)
{
	char valString[64];
	int cir, pir, greenDropRate, yellowDropRate, i;
	QDMA_TxQueueTrtcm_T txqTrtcm;
	QDMA_TxQueueDropProbability_T txqDropRate;
	QDMA_TxQueueCongestScale_T txqScale;
	QDMA_TxQueueCongestThreshold_T txqThreshold;
	QDMA_Mode_t qdmaMode = QDMA_ENABLE;
	

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d %d %d %d %d %d", &trtcmEnable, &trtcmTsid, &cir, &pir, &greenDropRate, &yellowDropRate);

	if (!trtcmEnable)
	{
		qdmaMode = QDMA_DISABLE;
		qdma_set_txqueue_trtcm_mode(qdmaMode);
		printk("\ntrtcm coloring is disabled!\n");
		printk("\nUsage: echo 1 (tsid) (cir)[*64Kbps] (pir)[*64Kbps] (greenDropRate)[/255] (yellowDropRate)[/255] > /proc/tc3162/ptm_ptm_trafficShapingSet\n\n");
		return count;
	}


	/* Setting for CIR, PIR, CBS, PBS*/
	
	
	if (isFPGA)
	    write_reg_word(0xbfb0082c, 0x31000000); //let cir, pir more precise.
	
	memset(&txqTrtcm, 0, sizeof(QDMA_TxQueueTrtcm_T));
	txqTrtcm.tsid = trtcmTsid;
	//the uint of cir, pir is 64Kpbs
	txqTrtcm.cirParamValue = cir;
	txqTrtcm.pirParamValue = pir;
	//160*128=20480 byte (the largetest size that PTM can transmit in a burst)
	//Note: 128 is set by QDMA driver in initialization.
	//Note: 20480 must be at least 2~3 larger than cir, pir
	txqTrtcm.cbsParamValue = 160;
	txqTrtcm.pbsParamValue = 160;

	qdma_set_txqueue_trtcm_params(&txqTrtcm);
	qdma_set_txqueue_trtcm_mode(qdmaMode);


	/* Setting for green, yellow drop probability*/
	
	memset(&txqDropRate, 0, sizeof(QDMA_TxQueueDropProbability_T));
	txqDropRate.green = greenDropRate;
	txqDropRate.yellow = yellowDropRate;

	qdma_set_congest_drop_probability(&txqDropRate);


	/* Setting for green, yellow congestion threshold (descriptors based).
	 * 1. smaller than minThreshold, packets always pass
	 *    w/o caring about drop probility.
	 * 2. between minThreshold and maxThreshold, packets
	 *    are dropped by drop probability.
	 * 3. larger than maxThreshold, packets are all drop 
	 *    w/o caring about drop probility. */
	memset(&txqScale, 0, sizeof(QDMA_TxQueueCongestScale_T));
	txqScale.maxScale = QDMA_TXQUEUE_SCALE_16_DSCP;
	qdma_set_congestion_scale(&txqScale);

	/* we let each queue of each channel have the same
	 * threshold for grn/ylw packets (which is 16*8=128 dscps),
	 * so packets are dropped by drop probability. */
	memset(&txqThreshold, 0, sizeof(QDMA_TxQueueCongestThreshold_T));
	for (i = 0; i < TX_QUEUE_NUM; i++)
	{
		txqThreshold.queueIdx = i;
		txqThreshold.grnMaxThreshold = 5;
		txqThreshold.ylwMaxThreshold = 5;
		qdma_set_congest_threshold(&txqThreshold);
	}

	qdma_set_txqueue_threshold_mode(qdmaMode);

	printk("\ntrtcmEnable:%d\ntrtcmTsid:%d\n"
			"CIR:%d[*64kbps]\nPIR:%d[*64kbps]\n"
			"greenDropRate:%d[%%]\nyellowDropRate:%d[%%]\n\n",
			trtcmEnable, trtcmTsid, 
			cir, pir, 
			(greenDropRate*100)/255, 
			(yellowDropRate*100)/255);

	return count;
}

static int ptm_trafficShapingSet_read_proc(
		char *buf, char **start, off_t off, int count,
        int *eof, void *data
)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	QDMA_TxQueueTrtcm_T txqTrtcm;
	QDMA_TxQueueCongestCfg_T txqCongestCfg;
	QDMA_Mode_t qdmaMode;
	int i;


	qdmaMode = qdma_get_txqueue_threshold_mode();
	if (qdmaMode)
	{
		index += sprintf(buf+index, 
				"\nQDMA threshold drop is enabled\n");
		CHK_BUF();	
	}
	else
	{
		index += sprintf(buf+index, 
				"\nQDMA threshold drop is disabled\n");
		CHK_BUF();	
	}
	
	qdmaMode = qdma_get_txqueue_trtcm_mode();
	if (qdmaMode)
	{
		index += sprintf(buf+index, 
				"\nQDMA trtcm coloring is enabled\n");
		CHK_BUF();	
	}
	else
	{
		index += sprintf(buf+index, 
				"\nQDMA trtcm coloring is disabled\n");
		CHK_BUF();	
	}
	
	if (!trtcmEnable)
	{
		*eof = 1;
		goto done;
	}

	memset(&txqTrtcm, 0, sizeof(QDMA_TxQueueTrtcm_T));

	for (i = 0; i < 32; i++)
	{
		txqTrtcm.tsid = i;
		if (!qdma_get_txqueue_trtcm_params(&txqTrtcm))
		{
			if (txqTrtcm.pirParamValue == 0)
				continue;
		
			index += sprintf(buf+index, "\nTSID: %d, CIR: %d [*64Kbps], PIR: %d [*64Kbps], CBS: %d [*128Byte], PBS: %d [*128Byte]\n", txqTrtcm.tsid, txqTrtcm.cirParamValue, txqTrtcm.pirParamValue, txqTrtcm.cbsParamValue, txqTrtcm.pbsParamValue);
			CHK_BUF();
		}
	}

	memset(&txqCongestCfg, 0, sizeof(QDMA_TxQueueCongestCfg_T));

	if (!qdma_get_congest_config(&txqCongestCfg))
	{
		index += sprintf(buf+index, "\nmaxScale: %d, minScale: %d\n", 0x2<<(int)txqCongestCfg.maxScale, 0x2<<(int)txqCongestCfg.minScale);
		CHK_BUF();
		index += sprintf(buf+index, "greenDropRate: %d(%%), yellowDropRate: %d(%%)\n", ((int)txqCongestCfg.grnDropProb*100)/255, ((int)txqCongestCfg.ylwDropProb*100)/255);
		CHK_BUF();
		for (i = 0; i < 8; i++)
		{
			index += sprintf(buf+index, "TxQueue[%d]:\n", i);
			CHK_BUF();			
			index += sprintf(buf+index, "grnMaxThld: %d, grnMinThld: %d, ylwMaxThld: %d, ylwMinThld: %d\n", (int)txqCongestCfg.queue[i].grnMaxThreshold, (int)txqCongestCfg.queue[i].grnMinThreshold, (int)txqCongestCfg.queue[i].ylwMaxThreshold, (int)txqCongestCfg.queue[i].ylwMinThreshold);
			CHK_BUF();
		}
	}

	*eof = 1;

done:
	*start = buf + (off - begin);
	index -= (off - begin);
	if (index < 0) 
		index = 0;
	if (index > count) 
		index = count;

	return index;
}


#ifdef TCSUPPORT_BONDING
void bonding_lines_switching(int toWhere)
{
    switch(toWhere)
    {
        case TO_LINE0:
            linesSwitchingMode = 0x6;
            printk("\nbonding switching to line0\n");
            break;
        case TO_LINE1:
            linesSwitchingMode = 0x5;
            printk("\nbonding switching to line1\n");
            break;
        case TO_BOTH_LINES:
            linesSwitchingMode = 0x7;
            printk("\nbonding switching to both lines\n");
            break;
        case TO_NO_BONDING:
            linesSwitchingMode = 0;
            printk("\nbonding switching to No bonding\n");
            break;
        default:
            printk("\nUnknown bonding switching type:%d\n", toWhere);
            return;
    }

	//close PTM/Bonding Tx/Rx by sequence
	ptm_close();
    
	/* after reset, PTM/Bonding registers will become 
	 * default values and bonding module is disabled 
	 * by default. */
	ptm_reset();
    
#ifdef LINE_BONDING
    if (toWhere == TO_BOTH_LINES)
        maxFragSize_set();
#endif

	//config bonding settings
	if (toWhere != TO_NO_BONDING)
	    bonding_recovery();
    
	//enable PTM/Bonding Tx/Rx by sequence
	ptm_open();

	return;    
}

#ifdef FPGA_STAGE
/* "echo N > /proc/tc3162/bonding_lines_switching"
 * to do line switching.
 * N==0: switching to line0 only
 * N==1: switching to line1 only
 * N==2: switching to both line0 & line1 */
static int bonding_lines_switching_write_proc(
		struct file *file, const char *buffer, 
		unsigned long count, void *data
)
{
	char valString[8];
	int mode;
	

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d", &mode);

	if (mode == 0)
		bonding_lines_switching(TO_LINE0);
	else if (mode == 1)
		bonding_lines_switching(TO_LINE1);
	else if (mode == 2)
		bonding_lines_switching(TO_BOTH_LINES);
	else
		printk("\n[Usage]: 0: line0, 1: line1, 2: 2 lines\n");

	return count;
}
#endif
#endif


#ifdef CONFIG_8021P_REMARK
/* 
 * Fill vlanTag's PCP (Priority Code Point) by skb->mark's bit8~11.
 * If packet's byte12-13 (vlanTag's TPID) is 0x8100, just change
 * the packet's vlanTag's PCP.
 * If packet's byte12-13 (vlanTag's TPID) isn't 0x8100, 
 * insert a VLAN tag with TPID=0x8100, PCP=mark, and VID=0.
 */
static inline struct sk_buff* vlanPriRemark(struct sk_buff *skb)
{
    char * vlan_p = NULL, *ether_type_ptr = NULL;
    unsigned char ucprio = 0;
    unsigned char uc802prio = 0;
    uint16 vid=0;
    int copy_len = 0;

    if ( skb->mark & QOS_8021p_MARK ) {
#if 1
        /*vlan tagging*/
        ether_type_ptr = skb->data + 12;
#endif		
        ucprio = (skb->mark & QOS_8021p_MARK) >> 8;
        if ( (ucprio < QOS_8021P_0_MARK) && (ucprio >= 0) ) { //0~7 remark
            uc802prio = ucprio;
        }
        else if ( QOS_8021P_0_MARK == ucprio ) {    //zero mark
            uc802prio = 0;
        }
        else{//pass through
            /*do nothing*/
            return skb;
        }
        if(*(unsigned short *)ether_type_ptr == 0x8100){
            vid=(*(unsigned short *)(ether_type_ptr+2) & 0xfff);
        }
        else{
            /*Insert a vlan tag with vid =0*/
            vid=0;
            if ( skb_headroom(skb) < VLAN_HLEN ) {
                struct sk_buff *sk_tmp = skb;
                skb = skb_realloc_headroom(sk_tmp, VLAN_HLEN);
                dev_kfree_skb_any(sk_tmp);
                if ( !skb ) {
                    printk("Vlan:failed to realloc headroom\n");
                    return NULL;
                }
            }
            else {
                skb = skb_unshare(skb, GFP_ATOMIC);
                if ( !skb ) {
                    return NULL;
                }
            }
        
            /*offset 4 bytes*/
            skb_push(skb, VLAN_HLEN);
            copy_len = 2*VLAN_ETH_ALEN; // frank mark
            /*move the mac address to the beginning of new header*/
            memmove(skb->data, skb->data+VLAN_HLEN, copy_len);
        }
    
        vlan_p = skb->data + 12;
        *(unsigned short *)vlan_p = 0x8100;
        
        vlan_p += 2;
        /*3 bits priority and vid vlaue*/
        *(unsigned short*)vlan_p = (((uc802prio & 0x7) << 13)|vid) ;
        skb->network_header -= VLAN_HLEN;
        skb->mac_header -= VLAN_HLEN;
    }
    return skb;
}
#endif /*CONFIG_8021P_REMARK*/

/*
 * If the returned value is 0, we don't care (*priority).
 * If the returned value is 1, we will use (*priority) 
 * as new priority 
 */
static uint8 isPriorityPkt(uint8 *cp, int *priority)
{
	uint16 etherType;
	uint8 ipVerLen;
	uint8 ipProtocol;
	uint8 tcpFlags;
	uint16 pppProtocol;


	if (cp == NULL)
	{
		printk("\nERROR: cp is NULL at isPriorityPkt\n");
		return 0;
	}
	if (priority == NULL)
	{
		printk("\nERROR: priority is NULL at isPriorityPkt\n");
		return 0;
	}

	/* skip DA and SA mac address */
	cp += 12;
	/* get ether type */
	etherType = *(uint16 *) cp;
	/* skip ether type */
	cp += 2;

	/*parse if vlan exists*/
	if (etherType == ETHER_TYPE_8021Q) {
		/*skip 802.1q tag field*/
		cp += 2;
		/*re-parse ether type*/
		etherType = *(uint16 *) cp;
		/* skip ether type */
		cp += 2;
	}

	/*check whether PPP packets*/ 
	if (etherType == 0x8864) {
		/* skip pppoe head */
		cp += 6; 					/* 6: PPPoE header 2: PPP protocol */
		/* get ppp protocol */
		pppProtocol = *(uint16 *) cp;
		/* check if LCP protocol */
		if (pppProtocol == 0xc021) {
			*priority = 3;
			return 1;
		/* check if IP protocol */
		} else if (pppProtocol != 0x0021) {
			return 0;
		}
		/* skip ppp protocol */
		cp += 2; 					/* 6: PPPoE header 2: PPP protocol */
	} else if (etherType == 0x8863) {
		*priority = 3;
		return 1;
	} else {
		/* check if ip packet */
		if (etherType != ETHER_TYPE_IP)
			return 0;
	}

	/* check if it is a ipv4 packet */
	ipVerLen = *cp;
	if ((ipVerLen & 0xf0) != 0x40) 
		return 0;

	/* get ip protocol */
	ipProtocol = *(cp + 9);
	/* check if ICMP/IGMP protocol */
	if ((ipProtocol == 1) || (ipProtocol == 2)) {
		*priority = 3;
		return 1;
	}
	/* check if TCP protocol */
	if (ipProtocol != 6)
		return 0;
		
	/* align to TCP header */
	cp += (ipVerLen & 0x0f) << 2;
	/* get TCP flags */
	tcpFlags = *(cp + 13);
	/* check if TCP syn/ack/psh ack/fin ack */
	if (((tcpFlags & 0x10) == 0x10) || (tcpFlags == 0x02)) {
		*priority = 3;
		return 1;
	}
		
	return 0;
}

/* disable PTM MAC Tx */
static void ptm_tx_stop(void)
{
	uint32 reg;

	reg = read_reg_word(PTM_CTRL_REG);
	reg &= ~(CTRL_TX_EN);
	write_reg_word(PTM_CTRL_REG, reg);
}

/* disable PTM MAC Rx */
static void ptm_rx_stop(void)
{
	uint32 reg;

	reg = read_reg_word(PTM_CTRL_REG);
	reg &= ~(CTRL_RX_EN);
	write_reg_word(PTM_CTRL_REG, reg);
}

/* enable PTM MAC Tx */
static void ptm_tx_en(void)
{
	uint32 reg;

	reg = read_reg_word(PTM_CTRL_REG);
	reg |= (CTRL_TX_EN);
	write_reg_word(PTM_CTRL_REG, reg);
}

/* enable PTM MAC Rx */
static void ptm_rx_en(void)
{
	uint32 reg;

	reg = read_reg_word(PTM_CTRL_REG);
	reg |= (CTRL_RX_EN);
	write_reg_word(PTM_CTRL_REG, reg);
}

#ifndef TCSUPPORT_BONDING
//removes PTM MAC dummy RW to speed up
static void ptm_noDummyRW(void)
{
	uint32 reg;

	reg = read_reg_word(PTM_CTRL_REG);
	reg &= ~(U_CELL_BASE_MODE);
	write_reg_word (PTM_CTRL_REG, reg);
}

#define VDSL2_SET_REDUNDANT_MODE_SET 0x3002
static void dmt_noDummyRW (void)
{
	adsldev_ops *ops = NULL;
    unsigned char value = 1;

#ifdef LOOPBACK_SUPPORT
    if (internalLoopbackEn)
        return;
#endif
	ops = adsl_dev_ops_get(1); //Line0

    if (ops == NULL)
    {
        printk("\nFAILED(%s): adsl_dev_ops_get\n", __FUNCTION__);
        return;
    }

    ops->set(VDSL2_SET_REDUNDANT_MODE_SET, &value, NULL);

    return;
}
#endif

static void tpstc_txRxReset(void)
{
    uint32 reg1, reg2;

	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);

    reg1 = read_reg_word(TPSTC_TX_CFG);
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);
    reg2 = read_reg_word(TPSTC_RX_CFG);
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);

    if (((reg1 & TPSTC_MODE_MASK) == TPSTC_PTM_MODE) &&
        ((reg2 & TPSTC_MODE_MASK) == TPSTC_PTM_MODE))
    {
		printk("=========%s %d===========\n", __FUNCTION__, __LINE__);
        reg1 &= ~(TPSTC_MODE_MASK);
        write_reg_word(TPSTC_TX_CFG, reg1);

        reg2 &= ~(TPSTC_MODE_MASK);
        write_reg_word(TPSTC_RX_CFG, reg2);

        reg1 = read_reg_word(TPSTC_TX_CFG);
        reg1 |= (TPSTC_PTM_MODE);
        write_reg_word(TPSTC_TX_CFG, reg1);

    	reg2 = read_reg_word(TPSTC_RX_CFG);
        reg2 |= (TPSTC_PTM_MODE);
        write_reg_word(TPSTC_RX_CFG, reg2);

        printk("\ntpstc_txRxReset\n");
    }
    else
        printk("\nTPSTC is ATM\n");
    return;
}

#ifdef TCSUPPORT_BONDING
/* disable line0 & line1 utopia rx, which are
 * between Bonding & PTM-TC*/
static void bonding_rx_stop(void)
{
	uint32 reg;

	//disable line0 utopia rx (in master chip)
	reg = read_reg_word(BONDING_COMMON1);
	reg &= ~(COMMON1_UTOPIA_RX_EN);
	write_reg_word(BONDING_COMMON1, reg);

	//disable line1 utopia rx (in slave chip)
	reg = sBonding_reg_read(S_BONDING_COMMON1_OFF);
	reg &= ~(COMMON1_UTOPIA_RX_EN);
	sBonding_reg_write(S_BONDING_COMMON1_OFF, reg);
}

/* enable line0 & line1 utopia rx, which are
 * between Bonding & PTM-TC*/
static void bonding_rx_en(void)
{
	uint32 reg;

	if (linesSwitchingMode & (0x1<<1))
	{
		//enable line0 utopia rx (in master chip)
		reg = read_reg_word(BONDING_COMMON1);
		reg |= (COMMON1_UTOPIA_RX_EN);
		write_reg_word(BONDING_COMMON1, reg);
	}
	
	if (linesSwitchingMode & (0x1))
	{
		//enable line1 utopia rx (in slave chip)
		reg = sBonding_reg_read(S_BONDING_COMMON1_OFF);
		reg |= (COMMON1_UTOPIA_RX_EN);
		sBonding_reg_write(S_BONDING_COMMON1_OFF, reg);
	}
}
#endif

/* After SW reset, register configuration still exist */
static void ptm_sw_reset(void)
{
	uint32 reg, reg2, i;


	/* make sure PTM MAC is tx/rx idle before
	 * doing reset*/
    pause(2);
    reg = read_reg_word(PTM_RST_REG);
    if (!((reg & 0x8) && (reg & 0x4)))
    {
        printk("\nptm_sw_reset fail because PTM MAC tx/rx isn't in idle state\n");
        return;
    }


	/* close PTM bus timeout before doing
	 * PTM (SW) reset */

	reg = read_reg_word(PTM_RST_REG);
	reg |= RST_SW_RESET;
	//hold PTM MAC reset
	write_reg_word (PTM_RST_REG, reg);
	reg &= ~(RST_SW_RESET);
	
	//close PTM bus timeout
	reg2 = read_reg_word(0xbfb0092c);
	reg2 &= ~(1<<5); //bit5 is for PTM bus timeout
	write_reg_word (0xbfb0092c, reg2);

	//channel retire between hold & release MAC reset
	for (i = 0; i < PATH_NUM; i++){
		qdma_set_channel_retire(i);
	}

    tpstc_txRxReset();
	
	//release PTM MAC reset
	write_reg_word (PTM_RST_REG, reg);

	//enable PTM bus timeout
	reg2 |= (1<<5); //bit5 is for PTM bus timeout
	write_reg_word (0xbfb0092c, reg2);

	pause(1);
}

#ifdef TCSUPPORT_BONDING
void bonding_recovery(void)
{
	printk("Bonding recovery\n");

	/* config Slave Bonding */
	
	//Rx FIFO status mode for line1
	sBonding_reg_write(S_BONDING_RBUS_STATUS0_OFF, 0x4000000);
	sBonding_reg_write(S_BONDING_RBUS_CFG_OFF, 0x1200214);
	//Tx buffer is only for bearer0 of line1
	sBonding_reg_write(S_BONDING_TX_MEM_CFG_OFF, 0x1000000);
	/* set rx_channel_en to 0x33 instead of 0x11 to 
	 * let path1 error packets go into MAC and dropped,
	 * so that path1 error packets won't occupy Rx buffer */
	sBonding_reg_write(S_BONDING_COMMON1_OFF, 0x3300);
	//slave bonding enable
	sBonding_reg_write(S_BONDING_REG_BASE_OFF, 0x10001);


	/* config Master Bonding */
	
	//Tx cell count mode for line1
	write_reg_word(BONDING_RBUS_STATUS0, 0x4000000);
	write_reg_word(BONDING_RBUS_CFG, 0x200250);
	//Tx buffer is only for bearer0 of line0
	write_reg_word(BONDING_TX_MEM_CFG, 0x1000000);
	/* set rx_channel_en to 0x33 instead of 0x11 to 
	 * let path1 error packets go into MAC and dropped,
	 * so that path1 error packets won't occupy Rx buffer */  
	write_reg_word(BONDING_COMMON1, 0x3300);
#ifdef LINE_BONDING
	if (maxFragSizeEn)
	{
		maxFragSizeEn = 0;
		write_reg_word(BONDING_TXPKTCFG, 0);
		write_reg_word(BONDING_LINE0_B0_MAX, maxFragSizeLine0);
		write_reg_word(BONDING_LINE1_B0_MAX, maxFragSizeLine1);
		write_reg_word(BONDING_TXPKTCFG, 1);
	}
#endif
	//master bonding enable
	write_reg_word(BONDING_REG_BASE, 0x10001);

	return;
}
#endif

/* PTM MAC/ Bonding reset.
 * After reset, register configuration will become default */
void ptm_reset(void)
{
	uint32 reg, i;
#ifdef TCSUPPORT_BONDING
	uint32 regM, regS;
#endif


	/* make sure PTM MAC is tx/rx idle before
	 * doing reset*/
	pause(2);
	reg = read_reg_word(PTM_RST_REG);
	if (!((reg & 0x8) && (reg & 0x4)))
	{
		printk("\nptm_reset fail because PTM MAC tx/rx isn't in idle state\n");
		return;
	}


	/* use SCU_RESET_REG (0xbfb00834) instead of
	 * PTM_RST_REG to reset PTM MAC and Bonding 
	 * to prevent bus timeout from happening */

	reg = read_reg_word(SCU_RESET_REG);
	reg |= (1<<5);
	//hold PTM MAC reset
	write_reg_word(SCU_RESET_REG, reg);
	reg &= ~(1<<5);
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);

	//channel retire between hold & release MAC reset
	for (i = 0; i < PATH_NUM; i++){
		qdma_set_channel_retire(i);
	}
	
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);
#ifdef TCSUPPORT_BONDING
	printk("Bonding reset\n");

	//hold bonding reset
	regS = sSCU_reg_read(S_SCU_RESET_REG_OFF);
	regM = read_reg_word(SCU_RESET_REG);
	regS |= (1<<10);
	regM |= (1<<10);
	sSCU_reg_write(S_SCU_RESET_REG_OFF, regS);
	write_reg_word(SCU_RESET_REG, regM);
#endif
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);

    tpstc_txRxReset();
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);

	//release PTM MAC reset
	write_reg_word(SCU_RESET_REG, reg);
	pause(1);

#ifdef TCSUPPORT_BONDING
	//release bonding reset
	regS = sSCU_reg_read(S_SCU_RESET_REG_OFF);
	regM = read_reg_word(SCU_RESET_REG);
	regS &= ~(1<<10);
	regM &= ~(1<<10);
	sSCU_reg_write(S_SCU_RESET_REG_OFF, regS);
	write_reg_word(SCU_RESET_REG, regM);
	pause(1);
#endif

#ifndef TCSUPPORT_BONDING
	/* PTM removes dummy RW to speed up. Bonding can try in ASIC*/
	ptm_noDummyRW();
#endif
}


static int qdma_reg_init(void)
{
	int i;
	QDMA_TxQueueCongestThreshold_T txqThreshold;
	QDMA_TxQueueCongestScale_T txqScale;
	QDMA_Mode_t qdmaMode = QDMA_ENABLE;
	QDMA_TxBufCtrl_T txBufUsage;
	
#if defined (QOS_REMARKING) || defined (TCSUPPORT_HW_QOS)
	/* do QDMA congestion configuration, otherwise low
	 * priority packets may use all tx descriptors. 
	 * We suppose each queue (for all channels) can 
	 * use 16*5==80 dscps at most! */
	memset(&txqScale, 0, sizeof(QDMA_TxQueueCongestScale_T));
	txqScale.maxScale = QDMA_TXQUEUE_SCALE_16_DSCP;
	qdma_set_congestion_scale(&txqScale);
	
	memset(&txqThreshold, 0, sizeof(QDMA_TxQueueCongestThreshold_T));
	for (i = 0; i < TX_QUEUE_NUM; i++)
	{
		txqThreshold.queueIdx = i;
		txqThreshold.grnMaxThreshold = 5;
		qdma_set_congest_threshold(&txqThreshold);
	}

	qdma_set_txqueue_threshold_mode(qdmaMode);


	/* limite PSE buffer usage for each channel, then
	 * low priority packes won't block high priority
	 * packets (in PSE buffer) for too long, in order
	 * to prevent highest priority packets from dropping */
	memset(&txBufUsage, 0, sizeof(QDMA_TxBufCtrl_T));
	txBufUsage.mode = QDMA_ENABLE;
	txBufUsage.chnThreshold = 0x20; //PSE blocks
	txBufUsage.totalThreshold = 0x20;
	if (qdma_set_txbuf_threshold (&txBufUsage))
	{
		printk("\nFAILED(%s): qdma setting for txBufUsage\n" , __FUNCTION__);
		return -1;
	}
#endif

	return 0;
}

/* "echo 1 > /proc/tc3162/ptm_swReset" to do PTM SW reset */
static int ptm_swReset(void)
{
	uint8 bearer;
#if KERNEL_2_6_36
	ptmAdapter_t *ptmApt;
#endif

	if (!ptmInitialized)
		return 0;

	//stop interface to kernel
	for (bearer = 0; bearer < BEARER_NUM; bearer++)
	{
		if (ptmIfaceUp[bearer] && netif_running(mt7510PtmDev[bearer]))
		{
		#if KERNEL_2_6_36
			ptmApt = netdev_priv(mt7510PtmDev[bearer]);
			napi_disable(&ptmApt->napi);
		#else
			netif_poll_disable(mt7510PtmDev[bearer]);
		#endif

			netif_tx_disable(mt7510PtmDev[bearer]);
		}
	}

    ptm_close();


	//open interface to kernel
	for (bearer = 0; bearer < BEARER_NUM; bearer++)
	{
		if (ptmIfaceUp[bearer] && netif_running(mt7510PtmDev[bearer]))
		{
			netif_wake_queue(mt7510PtmDev[bearer]);
		#if KERNEL_2_6_36
			ptmApt = netdev_priv(mt7510PtmDev[bearer]);
			napi_enable(&ptmApt->napi);
		#else
			netif_poll_enable(mt7510PtmDev[bearer]);
		#endif
		}
	}

	//reset PTM MAC except registers
	ptm_sw_reset();

    ptm_open();

	return 1;
}

static int ptm_swReset_write_proc(struct file *file, const char *buffer, 
								unsigned long count, void *data)
{
	char valString[4];
	int val = 0;
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EINVAL;

	valString[count] = '\0';

	sscanf(valString, "%d", &val);

	if (val)
	{
		if (ptm_swReset())
			printk("\nPTM SW reset done!\n");
	}

	return count;
}

static int ptm_doResetSequence_write_proc(struct file *file, const char *buffer, 
								unsigned long count, void *data)
{
	char valString[4];
	int val = 0;
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EINVAL;

	valString[count] = '\0';

	sscanf(valString, "%d", &val);

	if (val)
	{
	#if defined(TCSUPPORT_BONDING)
        bonding_lines_switching(TO_BOTH_LINES);
    #else
        ptm_close();
    	ptm_reset();
        ptm_open();
    #endif
		printk("\nPTM Do Reset Sequence!\n");
	}

	return count;
}


/* "echo 1 > /proc/tc3162/ptm_reg_dump" to reset
 * PTM MAC TX_OK, RX_OK, RX_CRC counters */
static int ptm_reg_clear_proc(struct file *file, const char *buffer, 
								unsigned long count, void *data)
{
	char valString[4];
	int val = 0;
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EINVAL;

	valString[count] = '\0';

	sscanf(valString, "%d", &val);

	if (val)
	{
		write_reg_word(TMAC_PKT_CNT_CLR, 0xff);
		write_reg_word(TMAC_PKT_CNT_CLR, 0);
		write_reg_word(RMAC_PKT_CNT_CLR, 0xff);
		write_reg_word(RMAC_PKT_CNT_CLR, 0);
		write_reg_word(RMAC_CRCE_CNT_CLR, 0xff);
		write_reg_word(RMAC_CRCE_CNT_CLR, 0);
		
		printk("\nPTM MAC counters cleared!\n\n");
	}

	return count;
}


static int ptm_reg_dump_proc(char *buf, char **start, off_t off, int count,
                 			int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;


	if (!ptmInitialized)
	{
		*eof = 1;
		return 0;
	}


	index += sprintf(buf+index, "PTM_CTRL_REG         = 0x%08lx\n\n",
			read_reg_word(PTM_CTRL_REG)); 	
	CHK_BUF();
	
	index += sprintf(buf+index, "PTM_TX_UBUF_WR_CNT_L0 = 0x%08lx\t", 
			read_reg_word(PTM_TX_UBUF_WR_CNT_L0));
	CHK_BUF();
	index += sprintf(buf+index, "PTM_TX_UBUF_RD_CNT_L0 = 0x%08lx\n", 
			read_reg_word(PTM_TX_UBUF_RD_CNT_L0));
	CHK_BUF();
	index += sprintf(buf+index, "PTM_TX_UBUF_WR_CNT_L1 = 0x%08lx\t", 
			read_reg_word(PTM_TX_UBUF_WR_CNT_L1));
	CHK_BUF();
	index += sprintf(buf+index, "PTM_TX_UBUF_RD_CNT_L1 = 0x%08lx\n", 
			read_reg_word(PTM_TX_UBUF_RD_CNT_L1));
	CHK_BUF();

	index += sprintf(buf+index, "PTM_RX_UBUF_CNT      = 0x%08lx\n\n", 
			read_reg_word(PTM_RX_UBUF_CNT));
	CHK_BUF();
	
	index += sprintf(buf+index, "TMAC_PKT_CNT_P0      = 0x%08lx\t", 
			read_reg_word(TMAC_PKT_CNT_P0));
	CHK_BUF();
	index += sprintf(buf+index, "TMAC_PKT_CNT_P1      = 0x%08lx\n", 
			read_reg_word(TMAC_PKT_CNT_P1));
	CHK_BUF();
	index += sprintf(buf+index, "TMAC_PKT_CNT_P2      = 0x%08lx\t", 
			read_reg_word(TMAC_PKT_CNT_P2));
	CHK_BUF();
	index += sprintf(buf+index, "TMAC_PKT_CNT_P3      = 0x%08lx\n", 
			read_reg_word(TMAC_PKT_CNT_P3));
	CHK_BUF();
	index += sprintf(buf+index, "TMAC_PKT_CNT_P4      = 0x%08lx\t", 
			read_reg_word(TMAC_PKT_CNT_P4));
	CHK_BUF();
	index += sprintf(buf+index, "TMAC_PKT_CNT_P5      = 0x%08lx\n", 
			read_reg_word(TMAC_PKT_CNT_P5));
	CHK_BUF();
	index += sprintf(buf+index, "TMAC_PKT_CNT_P6      = 0x%08lx\t", 
			read_reg_word(TMAC_PKT_CNT_P6));
	CHK_BUF();
	index += sprintf(buf+index, "TMAC_PKT_CNT_P7      = 0x%08lx\n\n", 
			read_reg_word(TMAC_PKT_CNT_P7));
	CHK_BUF();

	index += sprintf(buf+index, "RMAC_PKT_CNT_P0      = 0x%08lx\t", 
			read_reg_word(RMAC_PKT_CNT_P0));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_PKT_CNT_P1      = 0x%08lx\n", 
			read_reg_word(RMAC_PKT_CNT_P1));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_PKT_CNT_P2      = 0x%08lx\t", 
			read_reg_word(RMAC_PKT_CNT_P2));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_PKT_CNT_P3      = 0x%08lx\n", 
			read_reg_word(RMAC_PKT_CNT_P3));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_PKT_CNT_P4      = 0x%08lx\t", 
			read_reg_word(RMAC_PKT_CNT_P4));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_PKT_CNT_P5      = 0x%08lx\n", 
			read_reg_word(RMAC_PKT_CNT_P5));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_PKT_CNT_P6      = 0x%08lx\t", 
			read_reg_word(RMAC_PKT_CNT_P6));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_PKT_CNT_P7      = 0x%08lx\n\n", 
			read_reg_word(RMAC_PKT_CNT_P7));
	CHK_BUF();

	index += sprintf(buf+index, "RMAC_CRCE_CNT_P0     = 0x%08lx\t", 
			read_reg_word(RMAC_CRCE_CNT_P0));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_CRCE_CNT_P1     = 0x%08lx\n", 
			read_reg_word(RMAC_CRCE_CNT_P1));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_CRCE_CNT_P2     = 0x%08lx\t", 
			read_reg_word(RMAC_CRCE_CNT_P2));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_CRCE_CNT_P3     = 0x%08lx\n", 
			read_reg_word(RMAC_CRCE_CNT_P3));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_CRCE_CNT_P4     = 0x%08lx\t", 
			read_reg_word(RMAC_CRCE_CNT_P4));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_CRCE_CNT_P5     = 0x%08lx\n", 
			read_reg_word(RMAC_CRCE_CNT_P5));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_CRCE_CNT_P6     = 0x%08lx\t", 
			read_reg_word(RMAC_CRCE_CNT_P6));
	CHK_BUF();
	index += sprintf(buf+index, "RMAC_CRCE_CNT_P7     = 0x%08lx\n\n", 
			read_reg_word(RMAC_CRCE_CNT_P7));
	CHK_BUF();

#if 1
	index += sprintf(buf+index, "QDMA_TX_PKT_SUM      = 0x%08lx\n", 
			read_reg_word(QDMA_TX_PKT_SUM));
	CHK_BUF();
	index += sprintf(buf+index, "GDM2_TX_GET_CNT      = 0x%08x\n", 
			fe_reg_read(GDM2_TX_GET_CNT_OFF));
	CHK_BUF();
	index += sprintf(buf+index, "GDM2_TX_OK_CNT       = 0x%08x\n", 
			fe_reg_read(GDM2_TX_OK_CNT_OFF));
	CHK_BUF();
	index += sprintf(buf+index, "QDMA_RX_PKT_SUM      = 0x%08lx\n",
			read_reg_word(QDMA_RX_PKT_SUM));
	CHK_BUF();
	index += sprintf(buf+index, "GDM2_RX_OK_CNT       = 0x%08x\n\n", 
			fe_reg_read(GDM2_RX_OK_CNT_OFF));
	CHK_BUF();	
	index += sprintf(buf+index, "GDM2_TX_DROP_CNT     = 0x%08x\n", 
			fe_reg_read(GDM2_TX_DROP_CNT_OFF));
	CHK_BUF();
	index += sprintf(buf+index, "GDM2_RX_OVER_DROP_CNT= 0x%08x\n", 
			fe_reg_read(GDM2_RX_OVER_DROP_CNT_OFF));
	CHK_BUF();
	index += sprintf(buf+index, "GDM2_RX_ERR_DROP_CNT = 0x%08x\n", 
			fe_reg_read(GDM2_RX_ERROR_DROP_CNT_OFF));
	CHK_BUF();
#endif

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

#ifdef TCSUPPORT_BONDING
static unsigned int slave_tpstc_reg_read(uint32 offset)
{
	return 	read_reg_word(slaveTpstcBaseAddr+offset);
}
#endif

static int tpstc_reg_dump_proc(char *buf, char **start, off_t off, int count,
                 			int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;


	if (!ptmInitialized)
	{
		*eof = 1;
		return 0;
	}


	index += sprintf(buf+index, "\n== Master TPSTC Counters ==\n\n"); 	
	CHK_BUF();
	
	index += sprintf(buf+index, "TPSTC_TX_B0_OK            = 0x%08lx\n",
			read_reg_word(TPSTC_TX_B0_OK)); 	
	CHK_BUF();
	
	index += sprintf(buf+index, "TPSTC_TX_B1_OK            = 0x%08lx\n",
			read_reg_word(TPSTC_TX_B1_OK)); 	
	CHK_BUF();
	
	index += sprintf(buf+index, "TPSTC_TX_B0_FIFO_UNDERRUN = 0x%08lx\n", 
			read_reg_word(TPSTC_TX_B0_FIFO_UNDERRUN));
	CHK_BUF();

	index += sprintf(buf+index, "TPSTC_TX_B1_FIFO_UNDERRUN = 0x%08lx\n\n", 
			read_reg_word(TPSTC_TX_B1_FIFO_UNDERRUN));
	CHK_BUF();	

	index += sprintf(buf+index, "TPSTC_RX_B0_OK            = 0x%08lx\n",
			read_reg_word(TPSTC_RX_B0_OK)); 	
	CHK_BUF();
	
	index += sprintf(buf+index, "TPSTC_RX_B1_OK            = 0x%08lx\n",
			read_reg_word(TPSTC_RX_B1_OK)); 	
	CHK_BUF();

	index += sprintf(buf+index, "TPSTC_RX_FIFO_FULL        = 0x%08lx\n",
			read_reg_word(TPSTC_RX_FIFO_FULL)); 	
	CHK_BUF();
	
	index += sprintf(buf+index, "TPSTC_RX_CRC_ERR          = 0x%08lx\n",
			read_reg_word(TPSTC_RX_CRC_ERR)); 	
	CHK_BUF();

#ifdef TCSUPPORT_BONDING
	index += sprintf(buf+index, "\n== Slave TPSTC Counters (0x%08x) ==\n\n",
			slaveTpstcBaseAddr); 	
	CHK_BUF();
	
	index += sprintf(buf+index, "TPSTC_TX_B0_OK            = 0x%08x\n",
			slave_tpstc_reg_read(TPSTC_TX_B0_OK_OFF)); 	
	CHK_BUF();
	
	index += sprintf(buf+index, "TPSTC_TX_B1_OK            = 0x%08x\n",
			slave_tpstc_reg_read(TPSTC_TX_B1_OK_OFF)); 	
	CHK_BUF();
	
	index += sprintf(buf+index, "TPSTC_TX_B0_FIFO_UNDERRUN = 0x%08x\n", 
			slave_tpstc_reg_read(TPSTC_TX_B0_FIFO_UNDERRUN_OFF));
	CHK_BUF();

	index += sprintf(buf+index, "TPSTC_TX_B1_FIFO_UNDERRUN = 0x%08x\n\n", 
			slave_tpstc_reg_read(TPSTC_TX_B1_FIFO_UNDERRUN_OFF));
	CHK_BUF();	

	index += sprintf(buf+index, "TPSTC_RX_B0_OK            = 0x%08x\n",
			slave_tpstc_reg_read(TPSTC_RX_B0_OK_OFF)); 	
	CHK_BUF();
	
	index += sprintf(buf+index, "TPSTC_RX_B1_OK            = 0x%08x\n",
			slave_tpstc_reg_read(TPSTC_RX_B1_OK_OFF)); 	
	CHK_BUF();

	index += sprintf(buf+index, "TPSTC_RX_FIFO_FULL        = 0x%08x\n",
			slave_tpstc_reg_read(TPSTC_RX_FIFO_FULL_OFF)); 	
	CHK_BUF();
	
	index += sprintf(buf+index, "TPSTC_RX_CRC_ERR          = 0x%08x\n",
			slave_tpstc_reg_read(TPSTC_RX_CRC_ERR_OFF)); 	
	CHK_BUF();
#endif

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

#ifdef TCSUPPORT_BONDING
static int bonding_reg_dump_proc(char *buf, char **start, off_t off, int count,
                 			int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;


	if (!ptmInitialized)
	{
		*eof = 1;
		return 0;
	}


	index += sprintf(buf+index, "\nBONDING_TXFRAG0   = 0x%08lx\t", 
			read_reg_word(BONDING_TXFRAG0));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_TXFRAG1   = 0x%08lx\n", 
			read_reg_word(BONDING_TXFRAG1));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_TXFRAG2   = 0x%08lx\t", 
			read_reg_word(BONDING_TXFRAG2));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_TXFRAG3   = 0x%08lx\n", 
			read_reg_word(BONDING_TXFRAG3));
	CHK_BUF();

	index += sprintf(buf+index, "BONDING_TXFRAG4   = 0x%08lx\t", 
			read_reg_word(BONDING_TXFRAG4));
	CHK_BUF();
	
	index += sprintf(buf+index, "BONDING_TXFRAG5   = 0x%08lx\n", 
			read_reg_word(BONDING_TXFRAG5));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_TXFRAG6   = 0x%08lx\t", 
			read_reg_word(BONDING_TXFRAG6));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_TXFRAG7   = 0x%08lx\n\n", 
			read_reg_word(BONDING_TXFRAG7));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXFRAG0   = 0x%08lx\t", 
			read_reg_word(BONDING_RXFRAG0));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXFRAG1   = 0x%08lx\n", 
			read_reg_word(BONDING_RXFRAG1));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXFRAG2   = 0x%08lx\t", 
			read_reg_word(BONDING_RXFRAG2));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXFRAG3   = 0x%08lx\n", 
			read_reg_word(BONDING_RXFRAG3));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXFRAG4   = 0x%08lx\t", 
			read_reg_word(BONDING_RXFRAG4));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXFRAG5   = 0x%08lx\n", 
			read_reg_word(BONDING_RXFRAG5));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXFRAG6   = 0x%08lx\t", 
			read_reg_word(BONDING_RXFRAG6));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXFRAG7   = 0x%08lx\n\n", 
			read_reg_word(BONDING_RXFRAG7));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXPKT0    = 0x%08lx\t", 
			read_reg_word(BONDING_RXPKT0));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXPKT1    = 0x%08lx\n", 
			read_reg_word(BONDING_RXPKT1));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXPKT2    = 0x%08lx\t", 
			read_reg_word(BONDING_RXPKT2));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXPKT3    = 0x%08lx\n", 
			read_reg_word(BONDING_RXPKT3));
	CHK_BUF();

	index += sprintf(buf+index, "BONDING_RXPKT4    = 0x%08lx\t", 
			read_reg_word(BONDING_RXPKT4));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXPKT5    = 0x%08lx\n", 
			read_reg_word(BONDING_RXPKT5));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXPKT6    = 0x%08lx\t", 
			read_reg_word(BONDING_RXPKT6));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXPKT7    = 0x%08lx\n\n", 
			read_reg_word(BONDING_RXPKT7));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXERRPKT0 = 0x%08lx\t", 
			read_reg_word(BONDING_RXERRPKT0));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXERRPKT1 = 0x%08lx\n", 
			read_reg_word(BONDING_RXERRPKT1));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXERRPKT2 = 0x%08lx\t", 
			read_reg_word(BONDING_RXERRPKT2));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXERRPKT3 = 0x%08lx\n", 
			read_reg_word(BONDING_RXERRPKT3));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXERRPKT4 = 0x%08lx\t", 
			read_reg_word(BONDING_RXERRPKT4));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXERRPKT5 = 0x%08lx\n", 
			read_reg_word(BONDING_RXERRPKT5));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXERRPKT6 = 0x%08lx\t", 
			read_reg_word(BONDING_RXERRPKT6));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_RXERRPKT7 = 0x%08lx\n\n", 
			read_reg_word(BONDING_RXERRPKT7));
	CHK_BUF();

	index += sprintf(buf+index, "BONDING_U2R_TX(M) = 0x%08lx\t", 
			read_reg_word(BONDING_U2R_TX));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_R2U_TX(S) = 0x%08x\n", 
			sBonding_reg_read(S_BONDING_R2U_TX_OFF));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_U2R_RX(S) = 0x%08x\t", 
			sBonding_reg_read(S_BONDING_U2R_RX_OFF));
	CHK_BUF();
	index += sprintf(buf+index, "BONDING_R2U_RX(M) = 0x%08lx\n\n", 
			read_reg_word(BONDING_R2U_RX));
	CHK_BUF();
	

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

static void bonding_check_fail(
		unsigned int reg, unsigned int regVal, int i
)
{
	printk("\nWrong value for 0x%08x:\n"
			"read_reg_word(reg):0x%08lx\n"
			"Expected value: 0x%08x\n",
			reg+(i<<2), read_reg_word(reg+(i<<2)), regVal);

	bondingCheckRegsFail = 1;
	return;
}

static int bonding_check_reg_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	unsigned int reg = BONDING_REG_BASE, regVal;
	char valString[8];
	int i = 0, j, tryCnt;


	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EINVAL;

	valString[count] = '\0';

	sscanf(valString, "%d", &tryCnt);

	printk("\nStart checking bonding registers for %d times\n", tryCnt);	

	bondingCheckRegsFail = 0;

	for (j = 0; j < tryCnt; j++)
	{

		//configured in driver
		reg = BONDING_REG_BASE;
		regVal = 0x10001;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
		
		//configured in driver
		reg = S_BONDING_REG_BASE_OFF;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
		

		//configured in driver
		reg = BONDING_COMMON1;
		regVal = 0x1013300;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
		
		//configured in driver
		reg = S_BONDING_COMMON1_OFF;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);


		//configured in driver
		reg = BONDING_TXPAF_CFG0;
		regVal = 0x7;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);

		reg = S_BONDING_TXPAF_CFG0_OFF;
		regVal = 0x0;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);


		reg = BONDING_TXPAF_CFG1;
		regVal = 0x0;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);

		reg = S_BONDING_TXPAF_CFG1_OFF;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
			

		//configured in driver
		reg = BONDING_RXPAF_CFG0;
		regVal = 0x307;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);

		reg = S_BONDING_RXPAF_CFG0_OFF;
		regVal = 0x0;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
			

		reg = BONDING_RXPAF_CFG1;
		regVal = 0x0;
		if (read_reg_word(reg) != 0x0)
			bonding_check_fail(reg, regVal, i);
	
		reg = S_BONDING_RXPAF_CFG1_OFF;
		if (sBonding_reg_read(reg) != 0x0)
			bonding_check_fail(reg, regVal, i);
	
	
		//configured in driver
		reg = BONDING_LINE0_B0_MAX;
		regVal = 0x2000200;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
		reg = S_BONDING_LINE0_B0_MAX_OFF;
		regVal = 0x2000200;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
	
		reg = BONDING_LINE0_B1_MAX;
		regVal = 0x2000200;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
		reg = S_BONDING_LINE0_B1_MAX_OFF;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	

		//configured in driver
		reg = BONDING_LINE1_B0_MAX;
		regVal = 0x2000200;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
		reg = S_BONDING_LINE1_B0_MAX_OFF;
		regVal = 0x2000200;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
	
		reg = BONDING_LINE1_B1_MAX;
		regVal = 0x2000200;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
		reg = S_BONDING_LINE1_B1_MAX_OFF;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
		

		reg = BONDING_LINE0_B0_MIN;
		regVal = 0x400040;
		for (i = 0; i < 4; i++)
		{
			if (read_reg_word(reg+(i<<2)) != regVal)
				bonding_check_fail(reg, regVal, i);
		}
	
		reg = S_BONDING_LINE0_B0_MIN_OFF;
		for (i = 0; i < 4; i++)
		{
			if (sBonding_reg_read(reg+(i<<2)) != regVal)
				bonding_check_fail(reg, regVal, i);
		}
	
		i = 0;
		
	
		//configured in driver
		reg = BONDING_RX_CFG;
		regVal = 0xf;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
		//configured in driver
		reg = S_BONDING_RX_CFG_OFF;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
	
		reg = BONDING_TXFRAG0;
		regVal = 0x0;
		for (i = 0; i < 32; i++)
		{
			if (read_reg_word(reg+(i<<2)) != regVal)
				bonding_check_fail(reg, regVal, i);
		}
	
		reg = S_BONDING_TXFRAG0_OFF;
		for (i = 0; i < 32; i++)
		{
			if (sBonding_reg_read(reg+(i<<2)) != regVal)
				bonding_check_fail(reg, regVal, i);
		}
	
		i = 0;
			
		//configured in driver
		reg = BONDING_TXPKTCFG;
		regVal = 0x0;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
		reg = S_BONDING_TXPKTCFG_OFF;
		regVal = 0x0;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
		
	
		reg = BONDING_MASTER_ADR;
		regVal = 0x20a020a0;
		for (i = 0; i < 2; i++)
		{
			if (read_reg_word(reg+(i<<2)) != regVal)
				bonding_check_fail(reg, regVal, i);
		}
	
		reg = S_BONDING_MASTER_ADR_OFF;
		regVal = 0x20a020a0;
		for (i = 0; i < 2; i++)
		{
			if (sBonding_reg_read(reg+(i<<2)) != regVal)
				bonding_check_fail(reg, regVal, i);
		}
	
		i = 0;
		
		//configured in driver
		reg = BONDING_RBUS_CFG;
		regVal = 0x200250;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
		//configured in driver
		reg = S_BONDING_RBUS_CFG_OFF;
		regVal = 0x1200214;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
	
		//configured in driver
		reg = BONDING_TX_MEM_CFG;
		regVal = 0x1000000;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
		//configured in driver
		reg = S_BONDING_TX_MEM_CFG_OFF;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
		
	
		reg = BONDING_DEBUG_CTL1;
		regVal = 0x3e0;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
		reg = S_BONDING_DEBUG_CTL1_OFF;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	

		reg = BONDING_RBUS_STATUS0;
		regVal = 0x4000f0f;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
		reg = S_BONDING_RBUS_STATUS0_OFF;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
	
		reg = BONDING_MEM_CFG;
		regVal = 0x1000155;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);

		reg = S_BONDING_MEM_CFG_OFF;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	

		reg = BONDING_DEBUG_CTL;
		regVal = 0x0;
		if (read_reg_word(reg) != regVal)
			bonding_check_fail(reg, regVal, i);
	
		reg = S_BONDING_DEBUG_CTL_OFF;
		if (sBonding_reg_read(reg) != regVal)
			bonding_check_fail(reg, regVal, i);

		if (bondingCheckRegsFail)
			return count;
	}
	
	printk("\nMaster & Slave Bonding registers check OK for %d times\n", tryCnt);

	return count;
}
#endif


/* file /proc/tc3162/ptm_b0_stats will be parsed in web.c 
 * and be shown in statitics webpage, so don't change 
 * the words below, otherwise, web.c can't parse correctly! */
static int ptm_stats_read_proc(
		char *buf, char **start, off_t off, int count,
        int *eof, void *data
)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	uint8 bearer = *((uint8 *) data);
	uint32 value;
    ptmAdapter_t *ptmApt = netdev_priv(mt7510PtmDev[bearer]);

	if (!ptmInitialized || !ptmIfaceUp[bearer]) {
		*eof = 1;
		return 0;
	}


	value = fe_reg_read(GDM2_RX_OK_BYTE_CNT_OFF);
	index += sprintf(buf+index, "inOctets       = 0x%08lx, ", value);
	CHK_BUF();

	value = fe_reg_read(GDM2_RX_OK_CNT_OFF);
	index += sprintf(buf+index, "inUnicastPkts  = 0x%08lx\n", value);
	CHK_BUF();

	value = fe_reg_read(GDM2_RX_OVER_DROP_CNT_OFF) +
			ptmApt->ptmStats.ptmMIB.rxDiscards +
			fe_reg_read(GDM2_RX_ERROR_DROP_CNT_OFF) +
			ptmApt->ptmStats.ptmMIB.rxErrors;
	index += sprintf(buf+index, "inDiscards     = 0x%08lx\n", value);
	CHK_BUF();

	value = fe_reg_read(GDM2_TX_OK_BYTE_CNT_OFF);
	index += sprintf(buf+index, "outOctets      = 0x%08lx, ", value);
	CHK_BUF();

	value = fe_reg_read(GDM2_TX_OK_CNT_OFF);
	index += sprintf(buf+index, "outUnicastPkts = 0x%08lx\n", value);
	CHK_BUF();

	value = fe_reg_read(GDM2_TX_DROP_CNT_OFF) + 
			ptmApt->ptmStats.ptmMIB.txDiscards + 
			read_reg_word(QDMA_RCDROP_FWD_GREEN) + 
			read_reg_word(QDMA_RCDROP_CPU_GREEN);
	index += sprintf(buf+index, "outDiscards    = 0x%08lx\n\n", value);
	CHK_BUF();


	value = ptmApt->ptmStats.ptmMIB.rxErrCrc;
	index += sprintf(buf+index, "rxErrCrc       = 0x%08lx, ", value);
	CHK_BUF();
	value = ptmApt->ptmStats.ptmMIB.rxErrLong;
	index += sprintf(buf+index, "rxErrLong      = 0x%08lx\n", value);
	CHK_BUF();
	value = ptmApt->ptmStats.ptmMIB.rxErrRunt;
	index += sprintf(buf+index, "rxErrRunt      = 0x%08lx, ", value);
	CHK_BUF();
	value = ptmApt->ptmStats.ptmMIB.rxErrIp4Cs;
	index += sprintf(buf+index, "rxErrIp4Cs     = 0x%08lx\n", value);
	CHK_BUF();
	value = ptmApt->ptmStats.ptmMIB.rxErrL4Cs;
	index += sprintf(buf+index, "rxErrL4Cs      = 0x%08lx\n\n", value);
	CHK_BUF();


	*eof = 1;

done:
	*start = buf + (off - begin);
	index -= (off - begin);
	if (index < 0) 
		index = 0;
	if (index > count) 
		index = count;

	return index;
}

#ifdef PTM_DEBUG
static int ptm_txCntLimiter_dump_proc(
		char *buf, char **start, off_t off, int count,
        int *eof, void *data
)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int value;
	int i;
    ptmAdapter_t *ptmApt;


    ptmApt = netdev_priv(mt7510PtmDev[0]);
	if (!ptmApt) {
		*eof = 1;
		return 0;
	}

	for (i = 0; i < TX_QUEUE_NUM; i++)
	{
		value = ptmApt->txCntLimiter[i];
		index += sprintf(buf+index, "B0_txCntLimiter[%d] = %d", i, value);
		if ((i & 0x1) == 0)
			index +=  sprintf(buf+index, "\t");
		else
			index +=  sprintf(buf+index, "\n");
			
		CHK_BUF();
	}

	index +=  sprintf(buf+index, "\n");


    ptmApt = netdev_priv(mt7510PtmDev[1]);
	if (!ptmApt) {
		goto done;
	}

	for (i = 0; i < TX_QUEUE_NUM; i++)
	{
		value = ptmApt->txCntLimiter[i];
		index += sprintf(buf+index, "B1_txCntLimiter[%d] = %d", i, value);
		if ((i & 0x1) == 0)
			index +=  sprintf(buf+index, "\t");
		else
			index +=  sprintf(buf+index, "\n");
		
		CHK_BUF();
	}

	index +=  sprintf(buf+index, "\n");


done:

	*eof = 1;

	*start = buf + (off - begin);
	index -= (off - begin);
	if (index < 0) 
		index = 0;
	if (index > count) 
		index = count;

	return index;
}
#endif


// clear ptm SW statistics
static int ptm_stats_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	uint8 bearer = *((uint8 *)data);
	char valString[4];
	int val = 0;
    ptmAdapter_t *ptmApt = netdev_priv(mt7510PtmDev[bearer]);
	
	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EINVAL;

	valString[count] = '\0';

	if (!ptmInitialized || !ptmIfaceUp[bearer])
		return count;

	sscanf(valString, "%d", &val);

	if (val)
	{
		memset(&ptmApt->ptmStats, 0, sizeof(ptmStats_t));
		printk("\nPTM bearer %d statistics is clear\n", bearer);
	}


	return count;
}


static int ptm_priPktChk_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	char valString[4];

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d", &priPktChk);

	if (priPktChk)
		printk("\npriority packet check is enabled and priPktChkLen is %d\n", (int)priPktChkLen);
	else
		printk("\npriority packet check is disbled\n");

	return count;
}

#ifdef PTM_DEBUG
static int ptm_dbgLevel_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	char valString[4];

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d", &ptmDbgLevel);

	printk("\nPTM DEBUG LEVEL: %d\n\n", ptmDbgLevel);

	return count;
}
#endif


#if defined (QOS_REMARKING) || defined (TCSUPPORT_HW_QOS)
static int ptm_qoswrr_read_proc(
		char *page, char **start, off_t off,
		int count, int *eof, void *data
)
{
	/* qos_wrr_info[0]: 0:SP, 1:WRR 
	 * qos_wrr_info[1]: txq3 weight for WRR
	 * qos_wrr_info[0]: txq2 weight for WRR
	 * qos_wrr_info[0]: txq1 weight for WRR
	 * qos_wrr_info[0]: txq0 weight for WRR 
	 */
	printk("%d %d %d %d %d\n", *qos_wrr_info, 
		*(qos_wrr_info + 1), *(qos_wrr_info + 2), 
		*(qos_wrr_info + 3), *(qos_wrr_info + 4));
		
	return 0;
}


/* when setting QoS via webpage, cfg_manager will
 * "echo ..... > /proc/tc3162/ptm_qoswrr" to do
 * HW QDMA QoS config */
static int ptm_qoswrr_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	int len, i;
	char get_buf[32];
	int max_wrr_val = 0;
	int path, txq;
	QDMA_TxQosScheduler_T txQos;


	if(count > 32)
		len = 32;
	else
		len = count;
		
	/* use the copy_from_user function to copy 
	 * buffer data to our get_buf */
	if(copy_from_user(get_buf, buffer, len))
		return -EFAULT;
		
	/* zero terminate get_buf */
	get_buf[len]='\0';


	/* qos_wrr_info[0]: 0:SP, 1:WRR 
	 * qos_wrr_info[1]: txq3 weight for WRR
	 * qos_wrr_info[2]: txq2 weight for WRR
	 * qos_wrr_info[3]: txq1 weight for WRR
	 * qos_wrr_info[4]: txq0 weight for WRR */
	if(sscanf(get_buf, "%d %d %d %d %d", qos_wrr_info, 
			(qos_wrr_info+1), (qos_wrr_info+2), 
			(qos_wrr_info+3), (qos_wrr_info+4)) != 5)
		return count;


	/* find the txq with max qos wrr weight */
	for (i = 0; i < 4; i++) {
		if (max_wrr_val < qos_wrr_info[i + 1]) {
			max_wrr_val = qos_wrr_info[i + 1];
			maxPrio = 3 - i;
		}
	}


	/* Strict Priority */
	if(*qos_wrr_info == 0) 
	{
		/* set QDMA as Strict Priority Mode,
	 	 * so that p7> ... >p0 for all paths */
		for (path = 0; path < PATH_NUM; path++)
		{
			memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
			txQos.channel = path;
			txQos.qosType = QDMA_TXQOS_TYPE_SP;
			if (qdma_set_tx_qos(&txQos))
			{
				printk("\nFAILED(%s): qdma strict priority setting for path%d\n", __FUNCTION__, path);
				return -1;
			}
		}

		printk("\nSP setting for txq0~3 is done\n\n");

	}
	/* Weighted Round Robin */
	else 
	{
		/* set QDMA's SP & WRR priority registers,
	 	 * so that p7>p6>p5>p4> p3:p2:p1:p0 for path 0~7 && 
		 * p3:p2:p1:p0 == qos_wrr_info[1]:qos_wrr_info[2]:
		 * 				  qos_wrr_info[3]:qos_wrr_info[4] */
		for (path = 0; path < PATH_NUM; path++)
		{
			memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
			txQos.channel = path;
			txQos.qosType = QDMA_TXQOS_TYPE_SPWRR4;
			for(txq = 0 ; txq < 4; txq++)
				txQos.queue[txq].weight = qos_wrr_info[4-txq];
					
			if (qdma_set_tx_qos(&txQos))
			{
				printk("\nFAILED(%s): qdma wrr setting for path%d\n" , __FUNCTION__, path);
				return -1;
			}
		}

		//take path0's WRR setting as an example to show
		memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
		txQos.channel = 0;
		qdma_get_tx_qos(&txQos) ;
		printk("\nWRR setting for txq0~3 is done:\n" 
		"(Type:%d) Q3:%d, Q2:%d, Q1:%d, Q0:%d\n\n",
					txQos.qosType, 
					txQos.queue[3].weight,
					txQos.queue[2].weight,
					txQos.queue[1].weight,
					txQos.queue[0].weight);
	}
		
	return len;
}
#endif


#ifdef TCSUPPORT_QOS
/* when setting QoS via webpage, cfg_manager will
 * "echo ..... > /proc/tc3162/ptm_tcqos_disc" to do
 * tcqos_disc config */
static int ptm_tcqos_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	char qos_disc[10];
	int len;

	if (count > 10) {
		len = 10;
	}
	else {
		len = count;
	}
	memset(qos_disc, 0, sizeof(qos_disc));
	
	if(copy_from_user(qos_disc, buffer, len - 1))
		return -EFAULT;

	qos_disc[len] = '\0';
	
	if (!strcmp(qos_disc, "HWWRR")) {
		qosFlag = QOS_HW_WRR;
		printk("\nqos discipline is HW WRR.\n\n");
	}
	else if (!strcmp(qos_disc, "HWPQ")) {
		qosFlag = QOS_HW_PQ;
		printk("\nqos discipline is HW PQ.\n\n");
	}
	else {
		qosFlag = NULLQOS;
		printk("\nqos discipline is disabled.\n\n");
	}

	return len;
}
#endif


#if defined(LOOPBACK_SUPPORT) || defined(EXTERNAL_LOOPBACK)
#define SCU_WAN_REG				0xbfb00070
#define WAN_PTM_MODE			0x2
#define WAN_MODE_MASK			0x3
//tell QDMA that now is ATM or PTM MAC enabled
static void ptm_mac_select(void)
{
	uint32 reg;

	reg = read_reg_word(SCU_WAN_REG);
	reg &= ~(WAN_MODE_MASK);
	reg |= WAN_PTM_MODE;
	write_reg_word (SCU_WAN_REG, reg);
}
#endif

static int ptm_reg_init(void)
{
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);
	/* PTM/Bonding registers will have default values after reset */
	ptm_reset();
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);

#ifndef TCSUPPORT_BONDING
    /* DMT removes dummy RW to cooperate with MAC. Do this just
     * in the begining because DMT will store it in global variable */
    dmt_noDummyRW();
#endif
#ifdef TCSUPPORT_BONDING
	//recovery bonding settings
	bonding_recovery();
#endif

#if defined(LOOPBACK_SUPPORT) || defined(EXTERNAL_LOOPBACK)
	// ptm_mac_select is done in CC also.
	if (internalLoopbackEn)
	ptm_mac_select();
#endif
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);

	return 0;
}


static void frameEngineReg_init(void)
{
	uint32 reg;


	//set the "strip CRC" bit because SAR driver may clear this bit!
	reg = fe_reg_read(GDM2_FWD_CFG_OFF);
	reg |= (1<<16);
	fe_reg_write(GDM2_FWD_CFG_OFF, reg);

	//clear the bit31 (for SAR to use) because SAR driver may set this bit!
	reg = fe_reg_read(GDM2_FWD_CFG_OFF);
	reg &= ~(1<<31);
	fe_reg_write(GDM2_FWD_CFG_OFF, reg);
	
	//set short packet as 60 bytes and long packet as 1600 because SAR driver may change it.
	fe_reg_write(GDM2_LEN_CFG_OFF, 0x640003c);	

	//set GDM2_CHN_EN to its default value because rmmod SAR will change this value.
	fe_reg_write(GDM2_CHN_EN_OFF, 0xffffffff);

	//disable VLAN untag
	reg = fe_reg_read(CDMP_VLAN_CT_OFF);
	reg &= ~(1<<1);
	fe_reg_write(CDMP_VLAN_CT_OFF, reg);	
}


static int mt7510_ptm_recycle_tx(void *txMsg, struct sk_buff *skb)
{	//in qdma isr

	ptmTxMsg_t *curTxMsg;
	ptmTxMsgBuf_t *curTxMsgBuf;
	uint8 bearer;
	int txq;
	ptmAdapter_t *ptmApt;


	/* When PTM driver is removed, QDMA TX skb 
	 * buffers should be recycled  */
	if (qdmaRecycleTxBuf){
		if (skb != NULL){
			dev_kfree_skb_any(skb);
		}
		return 0;
	}


	if (skb == NULL)
	{
		printk("\nERROR(%s): skb is NULL!\n\n", __FUNCTION__);
		return -1;
	}
	if (txMsg == NULL)
	{
		printk("\nERROR(%s): txMsg is NULL!\n\n", __FUNCTION__);
		dev_kfree_skb_any(skb);
		return -1;
	}

	curTxMsg = (ptmTxMsg_t*)txMsg;
	
	curTxMsgBuf = (ptmTxMsgBuf_t*)txMsg;
	bearer = (curTxMsg->txMsgW0.bits.pathNo >> PATH_BEARER_SHIFT) & 0x1;
	txq = curTxMsg->txMsgW0.bits.queue & TX_QUEUE_MASK;
	ptmApt = netdev_priv(mt7510PtmDev[bearer]);

	//free skb
	dev_kfree_skb_any(skb);

	/* hang txMsg back to txMsgQueue */
	
	spin_lock(&txRingLock);
	
	txAvaiMsgTail->nextAvaiMsg = curTxMsgBuf;
	txAvaiMsgTail = curTxMsgBuf;
		ptmApt->txCntLimiter[txq]--;

	spin_unlock(&txRingLock);


	return 0;
}


static int mt7510_ptm_rx(void *rxMsg, uint rxMsgLen, struct sk_buff *skb, uint frameSize)
{
	ptmRxMsg_t *curRxMsg;
	uint8 bearer;
	struct net_device *dev;
	ptmAdapter_t *ptmApt;
	struct sk_buff *newSkb;
	uint32 offset;
#ifdef EXTERNAL_LOOPBACK
	struct sk_buff *tmpSkb;
#endif


	/* QDMA RX skb buffers should be recycled when PTM driver
	 * is removed */
	if (qdmaRecycleRxBuf){
		if (skb != NULL){
			dev_kfree_skb_any(skb);
		}
		return 0;
	}

	if (skb == NULL)
	{
		printk("\nERROR(%s): skb is NULL!\n\n", __FUNCTION__);
		return -1;
	}
	if (rxMsg == NULL)
	{
		printk("\nERROR(%s): rxMsg is NULL!\n\n", __FUNCTION__);
		dev_kfree_skb_any(skb);
		return -1;
	}
	
	curRxMsg = (ptmRxMsg_t*) rxMsg;


	bearer = (curRxMsg->rxMsgW0.bits.pathNo >> PATH_BEARER_SHIFT) & 0x1;
	dev = mt7510PtmDev[bearer];
	ptmApt = netdev_priv(mt7510PtmDev[bearer]);

	if (!ptmInitialized)
	{
		newSkb = skb;
		ptmApt->ptmStats.ptmMIB.rxDiscards++;
        PTM_DBG(DBG_L2, "\nError(%s): ptmInitialized!\n", __FUNCTION__);
		goto recv_ok;
	}

#ifdef PTM_DEBUG
	if (ptmDbgLevel >= DBG_L4)
	{
		printk("\nDump RX Message:\n");
		dump_data((char*)curRxMsg, rxMsgLen);
		printk("\nDump skb->data:\n");
		dump_data(skb->data, frameSize);
	}
#endif

#ifdef EXTERNAL_LOOPBACK
	if (externalLoopbackEn)
	{
		if (ptm_externalLoopbackRx (curRxMsg, ptmApt,
						skb, &tmpSkb, dev, frameSize))
			ptmApt->ptmStats.ptmMIB.rxDiscards++;

		newSkb = tmpSkb;
		goto recv_ok;
	}
#endif

#ifdef LOOPBACK_SUPPORT
	if (internalLoopbackEn)
	{
		ptm_loopback_rx(curRxMsg, rxMsgLen, skb, frameSize);
	
		newSkb = skb;
		goto recv_ok;
	}
#endif

	// Comment by Camel, Will FIX Soon
	// ledTurnOn(LED_DSL_ACT_STATUS);
#if defined(TR068_LED)
	/*for interner traffic led*/
	if(internet_led_on) //IP connected and IP traffic is passing
		ledTurnOn(LED_INTERNET_ACT_STATUS);
	else
	{
		if(!internet_trying_led_on) 
		{
			ledTurnOff(LED_INTERNET_STATUS);
			ledTurnOff(LED_INTERNET_TRYING_STATUS);
		}
	}	
#endif

	#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
	tbs_led_data_blinking(led_internet_green);
	#endif


	if ((curRxMsg->rxMsgW0.word & PTM_RX_W0_ERR_BITS) ||
		(curRxMsg->rxMsgW1.word & PTM_RX_W1_ERR_BITS))
	{
		newSkb = skb;
		ptmApt->ptmStats.ptmMIB.rxErrors++;

		if (curRxMsg->rxMsgW0.bits.crcErr)
			ptmApt->ptmStats.ptmMIB.rxErrCrc++;
		if (curRxMsg->rxMsgW0.bits.isLong)
			ptmApt->ptmStats.ptmMIB.rxErrLong++;
		if (curRxMsg->rxMsgW0.bits.isRunt)
			ptmApt->ptmStats.ptmMIB.rxErrRunt++;
		if (curRxMsg->rxMsgW1.bits.ip4CsErr)
			ptmApt->ptmStats.ptmMIB.rxErrIp4Cs++;
		if (curRxMsg->rxMsgW1.bits.l4CsErr)
			ptmApt->ptmStats.ptmMIB.rxErrL4Cs++;
	#ifdef PTM_DEBUG
		if (ptmDbgLevel >= DBG_L1)
		{
			printk("\nDump RX ERROR Message:\n");
			dump_data((char*)curRxMsg, rxMsgLen);
			printk("\nDump skb->data:\n");
			dump_data(skb->data, frameSize);
		}
	#endif
		goto recv_ok;
	}

	newSkb = skbmgr_dev_alloc_skb2k();
	if (unlikely(!newSkb)) 
	{	/* faild to allocate more mbuf -> drop this pkt */
		newSkb = skb;
		ptmApt->ptmStats.ptmMIB.rxDiscards++;
        PTM_DBG(DBG_L2, "\nError(%s): skbmgr_dev_alloc_skb2k!\n", __FUNCTION__);
		goto recv_ok;
	}
	
	//shift to 4-byte alignment
	offset = ((uint32)(newSkb->tail)) & 0x3;
	if (offset)
		skb_reserve(newSkb, 4 - offset);


	skb_put(skb, frameSize);

#if defined(WAN2LAN)
/* do wan2lan after skb_put, 
 * because skb->len starts to have value from it */

	if(masko)
	{
		struct sk_buff *skb2 = NULL;
		uint32 port_id = 0;

		//Check the skb headroom is enough or not. shnwind 20100121.
		if(skb_headroom(skb) < TX_STAG_LEN)
			skb2 = skb_copy_expand(skb, TX_STAG_LEN, skb_tailroom(skb) , GFP_ATOMIC);
		else
			skb2 = skb_copy(skb, GFP_ATOMIC);
            
		if(skb2 == NULL)
			printk("\nFAILED: wan2lan skb2 allocation in ptm rx direction.\n");
		else
		{
			skb2->mark |= SKBUF_COPYTOLAN;
			port_id = (masko>>24) - 1;
			macSend(port_id,skb2); //tc3162_mac_tx
		}
	}
#endif


	// ----- Count the MIB-II -----
	ptmApt->ptmStats.ptmMIB.rxBytes += frameSize;
	ptmApt->ptmStats.ptmMIB.rxPkts++;
	

	skb->dev = dev;
	skb->ip_summed = CHECKSUM_NONE;
	skb->protocol = eth_type_trans(skb, dev); //extract ethnet header
	dev->last_rx = jiffies;

#ifdef TCSUPPORT_RA_HWNAT
	if (ra_sw_nat_hook_rxinfo)
		ra_sw_nat_hook_rxinfo(skb, FOE_MAGIC_PTM, (char*)&curRxMsg->rxMsgW1, sizeof(rxMsgWord1_t));
		
	if (ra_sw_nat_hook_rx != NULL) {
		if (ra_sw_nat_hook_rx(skb)) {
			netif_receive_skb(skb);
		}
	} else
#endif
	{
		netif_receive_skb(skb); //to kernel
	}


recv_ok:

	/* hook rxMsg back to qdma rx dscp ring */
	
	//using word access to clear rxMsg is more efficient.
	curRxMsg->rxMsgW0.word = 0;
	curRxMsg->rxMsgW1.word = 0;
	curRxMsg->rxMsgW2.word = 0;

	if (qdma_has_free_rxdscp())
    {   
		if (qdma_hook_receive_buffer(curRxMsg, sizeof(ptmRxMsg_t), newSkb))
        {      
     		dev_kfree_skb_any(newSkb);
    		printk("\nFAILED(%s): hook_receive_buffer\n", __FUNCTION__);
            return -1;       
        }
    }
	else
	{
		dev_kfree_skb_any(newSkb);
		printk("\nError: no available QDMA RX descritor\n");
        return -1;
	}

	return 0;
}


/* when packets are comming:
 * 1. check if ptm napi is in kernel napi queue
 * 2. disable QDMA Rx interrupt 
 * 3. put ptm napi in kernel napi queue 
 * (then, mt7510_ptm_napiPoll will be called) */
static int mt7510_ptm_qdmaEventHandler(QDMA_EventType_t qdmaEventType)
{	//in qdma isr

	unsigned long flags;
#if KERNEL_2_6_36
	ptmAdapter_t *ptmApt = netdev_priv(mt7510PtmDev[PTM_BEARER_0]);
#endif
	

	if ((qdmaEventType == QDMA_EVENT_RECV_PKTS) ||
        (qdmaEventType == QDMA_EVENT_NO_RX_BUFFER))
	{
        if (qdmaEventType == QDMA_EVENT_NO_RX_BUFFER)
            printk("\nQDMA_EVENT_NO_RX_BUFFER\n");
    
		spin_lock_irqsave(&napiLock, flags);

		/* because no dev info available, we always
		 * use nas8's poll function. */
	#if KERNEL_2_6_36
		if (napi_schedule_prep(&ptmApt->napi))
		{
			qdma_disable_rxpkt_int(); //disable RX interrupt.
			__napi_schedule(&ptmApt->napi);
		}
	#else
		if (netif_rx_schedule_prep(mt7510PtmDev[PTM_BEARER_0]))
		{
			qdma_disable_rxpkt_int(); //disable RX interrupt
			__netif_rx_schedule(mt7510PtmDev[PTM_BEARER_0]);
		}
	#endif
		
		spin_unlock_irqrestore(&napiLock, flags);
	}
	else if (qdmaEventType == QDMA_EVENT_TX_CROWDED)
	{
		printk("\nQDMA_EVENT_TX_CROWDED\n");
	}
	else
		printk("\nWrong QDMA Event Type: %d\n", qdmaEventType);

	return 0;
}


static int ptmMsgBuf_qdmaCfg_init(void)
{
	QDMA_InitCfg_t qdmaInitCfg;
	ptmTxMsgBuf_t *txAvaiMsg = NULL, *preTxAvaiMsg = NULL;
	ptmRxMsgBuf_t *rxAvaiMsg = NULL;
	struct sk_buff *skb;
	uint32 *allocSkbPtr;
	unsigned int offset, allocSkbNum = 0;
	int i; 


#ifdef LOOPBACK_SUPPORT
	/* The number of PTM RX messages should be as many as 
	 * (TX_QUEUE_NUM*TX_QUEUE_LEN) when in QoS loopback test, 
	 * otherwise QDMA may be out of RX descriptors when 
	 * receiving RX packets, because CPU process of 
	 * receiving a packet is slow! */
	if (internalLoopbackEn)
		rxMsgBufNum = (TX_QUEUE_NUM * TX_QUEUE_LEN) - 1; //minus 1 to escape warnning message!
#endif

	if (ptmMsgBufqdmaCfgInitialized) {
		printk(KERN_INFO "ptmMsgBufqdmaCfg has been initialited!\n");

		return 0;
	}

	qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);

	/* HW QDMA QoS init */
	qdma_reg_init();

	/* register QDMA callback functions */
	memset(&qdmaInitCfg, 0, sizeof(QDMA_InitCfg_t));
	qdmaInitCfg.txRecycleMode = QDMA_TX_POLLING;
	qdmaInitCfg.rxRecvMode = QDMA_RX_NAPI;
	qdmaInitCfg.cbXmitFinish = mt7510_ptm_recycle_tx;
	qdmaInitCfg.cbRecvPkts = mt7510_ptm_rx;
	qdmaInitCfg.cbEventHandler = mt7510_ptm_qdmaEventHandler;
	qdma_init(&qdmaInitCfg);


	/* allocate TX messages */

#ifdef MSG_CACHED
	txMsgRingBasePtr = dma_alloc_noncoherent(NULL, txMsgBufNum * sizeof(ptmTxMsgBuf_t), &txMsgRingPhyAddr, GFP_KERNEL);
#else
	txMsgRingBasePtr = dma_alloc_coherent(NULL, txMsgBufNum * sizeof(ptmTxMsgBuf_t), &txMsgRingPhyAddr, GFP_KERNEL);
#endif

	if (txMsgRingBasePtr == NULL)
	{
		printk("\nFAILED: allocate TX message buffers!\n");
		return -ENOMEM;
	}

	memset(txMsgRingBasePtr, 0, txMsgBufNum * sizeof(ptmTxMsgBuf_t));

	txAvaiMsg = (ptmTxMsgBuf_t*) txMsgRingBasePtr;
	txAvaiMsgHead = txAvaiMsg;
	txAvaiMsgTail = txAvaiMsg + (txMsgBufNum-1);

	//organize Tx messages as a ring
	for (i = 0; i < txMsgBufNum; i++, txAvaiMsg++)
	{
		txAvaiMsg->nextAvaiMsg = NULL;
		
		if (i > 0)
			preTxAvaiMsg->nextAvaiMsg = txAvaiMsg;

		preTxAvaiMsg = txAvaiMsg;
	}


	/* allocate Rx messages & skb buffers, 
	 * 			and hook them with QDMA descriptors */

#ifdef MSG_CACHED
	rxMsgRingBasePtr = dma_alloc_noncoherent(NULL, rxMsgBufNum * sizeof(ptmRxMsgBuf_t), &rxMsgRingPhyAddr, GFP_KERNEL);
#else
	rxMsgRingBasePtr = dma_alloc_coherent(NULL, rxMsgBufNum * sizeof(ptmRxMsgBuf_t), &rxMsgRingPhyAddr, GFP_KERNEL);
#endif

	if (rxMsgRingBasePtr == NULL)
	{
		printk("\nFAILED: allocate RX message buffers!\n");
		goto freeTxMsgRing;
	}
	
	memset(rxMsgRingBasePtr, 0, rxMsgBufNum * sizeof(ptmRxMsgBuf_t));
	rxAvaiMsg = (ptmRxMsgBuf_t*) rxMsgRingBasePtr;

	//for storing skb addresses temporarily
	allocSkbPtr = (uint32*) kmalloc(rxMsgBufNum * sizeof(uint32), GFP_KERNEL);
	if (allocSkbPtr == NULL)
	{
		printk("\nFAILED: allocate RX skb addresses!\n");
		goto freeRxMsgRing;
	}
	
	memset(allocSkbPtr, 0, rxMsgBufNum * sizeof(uint32));


	for (i = 0; i < rxMsgBufNum; i++, rxAvaiMsg++)
	{
		skb = skbmgr_dev_alloc_skb2k();
		if (skb == NULL)
		{
			printk("\nFAILED: allocate %dnd SKB buffer!\n", allocSkbNum);
			if (allocSkbNum == 0)
				goto freeAllocSkb;
			else
				goto freeSKBs;			
		}

		allocSkbNum++;
		allocSkbPtr[i] = (uint32) skb;

		//shift to 4-byte alignment
		offset = ((unsigned int)(skb->tail) & 3);
		if(offset) 
			skb_reserve(skb, (4 - offset));

		//hook Rx messages and skb buffers to QDMA dscps
		if (qdma_has_free_rxdscp())
			qdma_hook_receive_buffer(rxAvaiMsg, sizeof(ptmRxMsg_t), skb);
		else
		{
			dev_kfree_skb_any(skb);
			printk("\nWARNING: QDMA has no available RX Descriptors for %dth skb\n", allocSkbNum);
			break;
		}
	}

	kfree(allocSkbPtr); //no need any more


	/* QDMA is ready to TX/RX packets */
	qdmaRecycleRxBuf = 0;
	qdmaRecycleTxBuf = 0;
	ptmMsgBufqdmaCfgInitialized = 1;

#ifdef TCSUPPORT_RA_HWNAT
	qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_ENABLE); 
#else
	qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_DISABLE); //noHwFwd
#endif

	return 0;


freeSKBs:
	for (i = 0; i < allocSkbNum; i++)
		if (allocSkbPtr[i] != 0)
			dev_kfree_skb_any((struct sk_buff*)allocSkbPtr[i]);

freeAllocSkb:
	kfree(allocSkbPtr);

freeRxMsgRing:
#ifdef MSG_CACHED
	dma_free_noncoherent(NULL, rxMsgBufNum * sizeof(ptmRxMsgBuf_t), rxMsgRingBasePtr, rxMsgRingPhyAddr);
#else
	dma_free_coherent(NULL, rxMsgBufNum * sizeof(ptmRxMsgBuf_t), rxMsgRingBasePtr, rxMsgRingPhyAddr);
#endif

freeTxMsgRing:
#ifdef MSG_CACHED
	dma_free_noncoherent(NULL, txMsgBufNum * sizeof(ptmTxMsgBuf_t), txMsgRingBasePtr, txMsgRingPhyAddr);
#else
	dma_free_coherent(NULL, txMsgBufNum * sizeof(ptmTxMsgBuf_t), txMsgRingBasePtr, txMsgRingPhyAddr);
#endif

	return -ENOMEM;
}


static void ptmMsgBuf_qdmaCfg_free(void)
{
	int path;

	if (!ptmMsgBufqdmaCfgInitialized)
		return;

	qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);

	/* disable GDM2 TXRX channels */
	fe_reg_write(GDM2_CHN_EN_OFF, 0);

	/* QDMA channels retire for PTM */
	for (path = 0; path < PATH_NUM; path++){
		qdma_set_channel_retire(path);
	}

	qdma_txdscp_recycle_mode(QDMA_TX_INTERRUPT);
	/* mt7510_ptm_rx will be called to recycle rxSkbBuf */
	qdmaRecycleRxBuf = 1;
	qdma_recycle_receive_buffer();
	/* mt7510_ptm_recycle_tx will be called to recycle txSkbBuf */
	qdmaRecycleTxBuf = 1;
	qdma_recycle_transmit_buffer();

	/* release PTM TX/RX messages */
#ifdef MSG_CACHED
	dma_free_noncoherent(NULL, rxMsgBufNum * sizeof(ptmRxMsgBuf_t), rxMsgRingBasePtr, rxMsgRingPhyAddr);
	dma_free_noncoherent(NULL, txMsgBufNum * sizeof(ptmTxMsgBuf_t), txMsgRingBasePtr, txMsgRingPhyAddr);
#else
	dma_free_coherent(NULL, rxMsgBufNum * sizeof(ptmRxMsgBuf_t), rxMsgRingBasePtr, rxMsgRingPhyAddr);
	dma_free_coherent(NULL, txMsgBufNum * sizeof(ptmTxMsgBuf_t), txMsgRingBasePtr, txMsgRingPhyAddr);	
#endif

	/* unregister QDMA callback functions */
	qdma_deinit();

	ptmMsgBufqdmaCfgInitialized = 0;

	return;
}


static int mt7510_ptm_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{

	printk("\n%s: Not implemented yet\n", __FUNCTION__);
  	return 0;
}


/* Get device stats info from CI command "ifconfig" */
static struct net_device_stats *mt7510_ptm_stats(struct net_device *dev)
{
	ptmAdapter_t *ptmApt = netdev_priv(dev);
	struct net_device_stats *stats;


	stats = &ptmApt->stats;

	stats->tx_packets	= fe_reg_read(GDM2_TX_OK_CNT_OFF);
	stats->tx_bytes		= fe_reg_read(GDM2_TX_OK_BYTE_CNT_OFF);
	stats->tx_dropped	= (fe_reg_read(GDM2_TX_DROP_CNT_OFF)+ 
						(ptmApt->ptmStats.ptmMIB.txDiscards)+ 
						read_reg_word(QDMA_RCDROP_FWD_GREEN)+ 
						read_reg_word(QDMA_RCDROP_CPU_GREEN));

	stats->rx_packets	= fe_reg_read(GDM2_RX_OK_CNT_OFF);
	stats->rx_bytes		= fe_reg_read(GDM2_RX_OK_BYTE_CNT_OFF);
	stats->rx_dropped	= (fe_reg_read(GDM2_RX_OVER_DROP_CNT_OFF)+
						(ptmApt->ptmStats.ptmMIB.rxDiscards));
	stats->rx_errors	= (fe_reg_read(GDM2_RX_ERROR_DROP_CNT_OFF)+
						(ptmApt->ptmStats.ptmMIB.rxErrors));

	return stats;
}


/* Setup multicast list */
static void mt7510_ptm_set_multicast_list(struct net_device *dev)
{
	return;
}

/* Setting customized mac address */
static int mt7510_ptm_set_macAddr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	/* Check if given address is valid ethernet MAC address */
  	if (!is_valid_ether_addr(addr->sa_data))
	{
		printk("\nERROR: wrong MAC address\n");
    	return(-EIO);
	}

	/* Save the customize mac address */
  	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);


	return 0;
}


/* enable PTM/Bonding Tx/Rx by sequence */
void ptm_open(void)
{
#ifdef TCSUPPORT_BONDING
	uint32 regM, regS;
#endif

	ptm_rx_en();

#ifdef TCSUPPORT_BONDING
	printk("enable bonding TX/RX by sequence\n");

	//enable master rxpaf (RX)
	/* Enable RxPaf forwarding mode for path1 (0x300) to 
	 * let path1 error packets go into MAC and dropped,
	 * so that path1 error packets won't occupy Rx buffer */
	write_reg_word(BONDING_RXPAF_CFG0, (linesSwitchingMode | 0x300));

	//enable slave rbus_master (RX)
	regS = sBonding_reg_read(S_BONDING_COMMON1_OFF);
	regS |= (1<<24);
	sBonding_reg_write(S_BONDING_COMMON1_OFF, regS);

	//enable master rbus_master (TX)
	regM = read_reg_word(BONDING_COMMON1);
	regM |= (1<<24);
	write_reg_word(BONDING_COMMON1, regM);

	//enable master txpaf (TX)
	write_reg_word(BONDING_TXPAF_CFG0, linesSwitchingMode);

	/* master & slave uropia RX can't be enabled until
	 * PTM MAC is ready to receive packets, otherwise,
	 * slave wifi will hang! */
	bonding_rx_en();
#endif

	ptm_tx_en();

	return;
}

#ifdef TR068_LED	
static void mt7510_ptm_timer(unsigned long data)
{
	struct net_device *dev = (struct net_device *) data;
	ptmAdapter_t *ptm_p = netdev_priv(dev);
	struct psepkt_stats pf_stats;
	unsigned long rx_pkts_diff ,tx_pkts_diff;
	unsigned long rx_pkts,tx_pkts;

#ifdef TCSUPPORT_RA_HWNAT
	if (internet_hwnat_timer_switch && ra_sw_nat_hook_pse_stats) {		
		rx_pkts = ptmStats.rx_pkts; 	
		tx_pkts = ptmStats.tx_pkts; 		
		ptmStats.rx_pkts = ptm_p->ptmStats.ptmMIB.rxPkts;
		ptmStats.tx_pkts = ptm_p->ptmStats.ptmMIB.txPkts;

		ra_sw_nat_hook_pse_stats(&pf_stats, 2);

		ptmStats.rx_pkts += pf_stats.rx_pkts;
		ptmStats.tx_pkts += pf_stats.tx_pkts;

		rx_pkts_diff = ptmStats.rx_pkts- rx_pkts;
		tx_pkts_diff = ptmStats.tx_pkts- tx_pkts;

		if ((rx_pkts_diff > internet_hwnat_pktnum) ||
			(tx_pkts_diff > internet_hwnat_pktnum)) {
// comment by camel, will Fix Soon
#if 0
			//printk("\r\nmt7510_ptm_timer light led,internet_hwnat_pktnum=%d",internet_hwnat_pktnum);
			ledTurnOn(LED_DSL_ACT_STATUS);
			/*for interner traffic led*/
			if(internet_led_on) {//IP connected and IP traffic is passing
				ledTurnOn(LED_INTERNET_ACT_STATUS);
			} 
			else {
				if(!internet_trying_led_on) {
					ledTurnOff(LED_INTERNET_STATUS);
					ledTurnOff(LED_INTERNET_TRYING_STATUS);
				}
			}		
		}
#endif
	}
#endif

	/* Schedule for the next time */
	ptm_p->ptm_timer.expires = jiffies + msecs_to_jiffies(250);
  	add_timer(&ptm_p->ptm_timer);
}
#endif
static int mt7510_ptm_open(struct net_device *dev)
{
	ptmAdapter_t *ptmApt = netdev_priv(dev);
	uint8 bearer = ptmApt->bearer;
    unsigned long flags;
#if KERNEL_2_6_36
    ptmAdapter_t *ptmApter = netdev_priv(mt7510PtmDev[PTM_BEARER_0]);
#endif


#ifdef LINE_BONDING
    lineBonding_init();
#endif

	if (ptmIfaceUp[bearer])
		return 0;

#ifdef TR068_LED	
	/* Schedule timer */
  	init_timer(&ptmApt->ptm_timer);
	ptmApt->ptm_timer.expires = jiffies + msecs_to_jiffies(250);
  	ptmApt->ptm_timer.function = mt7510_ptm_timer;
  	ptmApt->ptm_timer.data = (unsigned long) dev;
  	add_timer(&ptmApt->ptm_timer);
#endif

	memset(&ptmApt->ptmStats, 0, sizeof(ptmStats_t));
	memset(&ptmApt->stats, 0, sizeof(struct net_device_stats));

#if KERNEL_2_6_36
	napi_enable(&ptmApt->napi);
#endif

	netif_start_queue(dev);
	set_bit(__LINK_STATE_START, &dev->state);

	printk(KERN_INFO "%s: starting interface.\n", dev->name);
    ptmIfaceUp[bearer] = 1;


    /* when vdsl is from down to up and nas8 is not ready yet,
     * Because HW path is up, qdma will receive packets and issue
     * Rx Done interrupts to CPU. Beucase nas8 isn't ready, SW
     * won't do napi polling, so qdma Rx buffer will be full.
     * when nas8 is ready, no Rx Done or Rx Desp Full interrupts
     * will be raised to notify SW to do napi polling. 
     * Therefore, do napi polling here. */

	spin_lock_irqsave(&napiLock, flags);

	/* because no dev info available, we always
	 * use nas8's poll function. */
#if KERNEL_2_6_36
	if (napi_schedule_prep(&ptmApter->napi))
	{
		qdma_disable_rxpkt_int(); //disable RX interrupt.
		__napi_schedule(&ptmApter->napi);
	}
#else
	if (netif_rx_schedule_prep(mt7510PtmDev[PTM_BEARER_0]))
	{
		qdma_disable_rxpkt_int(); //disable RX interrupt
		__netif_rx_schedule(mt7510PtmDev[PTM_BEARER_0]);
	}
#endif
	
	spin_unlock_irqrestore(&napiLock, flags);


  	return 0;
}


/* close PTM/Bonding Tx/Rx by sequence */
void ptm_close(void)
{
#ifdef TCSUPPORT_BONDING
	uint32 regM, regS;
#endif

	ptm_tx_stop();

#ifdef TCSUPPORT_BONDING
	printk("close bonding TX/RX by sequence\n");

	//close master & slave utopia_rx (RX)
	bonding_rx_stop();

	//close master txpaf (TX)
	write_reg_word(BONDING_TXPAF_CFG0, 0);
	
	//close slave rbus_master (RX)
	regS = sBonding_reg_read(S_BONDING_COMMON1_OFF);
	regS &= ~(1<<24);
	sBonding_reg_write(S_BONDING_COMMON1_OFF, regS);

	//close master rbus_master (TX)
	regM = read_reg_word(BONDING_COMMON1);
	regM &= ~(1<<24);
	write_reg_word(BONDING_COMMON1, regM);

	/* wait for a while after rbus_master disable 
	 * and before slave bonding reset */
	pause(100);

	//close master rxpaf (RX)
	write_reg_word(BONDING_RXPAF_CFG0, 0);
#endif

	ptm_rx_stop();

	return;
}


static int mt7510_ptm_close(struct net_device *dev)
{
	ptmAdapter_t *ptmApt = netdev_priv(dev);
	uint8 bearer = ptmApt->bearer;


	if (!ptmIfaceUp[bearer])
		return 0;

	/* Kill timer */
#ifdef TR068_LED
  	del_timer_sync(&ptmApt->ptm_timer);
#endif
	clear_bit(__LINK_STATE_START, &dev->state);
	netif_stop_queue(dev);

#if KERNEL_2_6_36
	napi_disable(&ptmApt->napi);
#endif

	printk(KERN_INFO "%s: stoping interface.\n", dev->name);
	ptmIfaceUp[bearer] = 0;

	return 0;
}


#if defined(LOOPBACK_SUPPORT) || defined(EXTERNAL_LOOPBACK)
int dummy_ptm_tx(struct sk_buff *skb, struct net_device *dev)
{
	//drop packets from Kernel when doing internal loopback
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}
#endif


int mt7510_ptm_tx(struct sk_buff *skb, struct net_device *dev)
{
	ptmAdapter_t *ptmApt;
	uint8 bearer = 0;
	uint32 vlanTag = 0;
	int needHwVlanTag = 0;
	int txq = 0;
	int newTxq;
	ptmTxMsg_t *curTxMsg;
	ptmTxMsgBuf_t *curTxMsgBuf;
	int error;
	unsigned long flags;
	int line = 0, preemption = 0;
	uint8 ipCsIns = 0;
#ifdef TCSUPPORT_RA_HWNAT
	struct port_info ptm_info;	
#endif

	if (skb == NULL)
	{
		printk("\nERROR(%s): skb is NULL!\n\n", __FUNCTION__);
		return NETDEV_TX_OK;
	}
    if (unlikely(skb->data == NULL))
    {
        printk("\n\nError(%s): skb->data is NULL\n\n", __FUNCTION__);
        return NETDEV_TX_OK;
    }
	if ((dev == NULL) || (netdev_priv(dev) == NULL))
	{
		printk("\nERROR(%s): dev is NULL!\n\n", __FUNCTION__);
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	ptmApt = netdev_priv(dev);
	bearer = ptmApt->bearer;


	if (!ptmInitialized)
	{
		dev_kfree_skb_any(skb);
		ptmApt->ptmStats.ptmMIB.txDiscards++;
        PTM_DBG(DBG_L2, "\nError(%s): ptmInitialized!\n", __FUNCTION__);
		return NETDEV_TX_OK;
	}


#ifdef EXTERNAL_LOOPBACK
	if (externalLoopbackEn)
		goto external_loopback_tx;
#endif

#ifdef LOOPBACK_SUPPORT
	if (internalLoopbackEn)
		goto loopback_tx_xmit;
#endif


#ifdef CONFIG_8021P_REMARK
	skb=vlanPriRemark(skb);
	if(skb==NULL){
		printk("(%s)802.1p remark failure\r\n", __FUNCTION__);
		return NETDEV_TX_OK;
	}
#endif


#if defined(WAN2LAN)
	if(masko)
	{
		struct sk_buff *skb2 = NULL;
		uint32 port_id = 0;

		//Check the skb headroom is enough or not. shnwind 20100121.
		if(skb_headroom(skb) < TX_STAG_LEN)
			skb2 = skb_copy_expand(skb, TX_STAG_LEN, skb_tailroom(skb) , GFP_ATOMIC);
		else
			skb2 = skb_copy(skb, GFP_ATOMIC);
            
		if(skb2 == NULL)
			printk("\nFAIL(%s): wan2lan allocation\n", __FUNCTION__);
		else
		{
			skb2->mark |= SKBUF_COPYTOLAN;
			port_id = (masko>>24) - 1;
			macSend(port_id, skb2); //tc3162_mac_tx
		}
	}
#endif


	/* the min len to PTM-MAC is 60 bytes, because PTM-MAC 
	 * will append 4-byte CRC in TX and FrameEngine will 
	 * strip 4-byte CRC in RX */
	if (unlikely(skb->len < ETH_ZLEN))
	{
		if (skb_padto(skb, ETH_ZLEN))
		{	//when error, skb_padto() will free skb.
			ptmApt->ptmStats.ptmMIB.txDiscards++;
            PTM_DBG(DBG_L2, "\nError(%s): skb_padto()!\n", __FUNCTION__);
			return NETDEV_TX_OK;
		}
		
		skb->len = ETH_ZLEN;
	}


#ifdef TCSUPPORT_QOS
	switch (qosFlag)
	{
		case QOS_HW_WRR:
			/* HW WRR mode */
			if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY)
				txq = 3;
			else if ((skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY)
				txq = 2;
			else if ((skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY)
				txq = 1;
			break;
		case QOS_HW_PQ:
			/* HW PQ mode */
			if (txq < 3 && (skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY)
				txq = 3;
			else if (txq < 2 && (skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY)
				txq = 2;
			else if (txq < 1 && (skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY)
				txq = 1;
			break;
		case NULLQOS: 
			/* for putting rtp packets to HH priority 
			 * when qosFlag is not selected as WRR or PQ*/
			if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY)
				txq = 3;
			break;
		default:
			break;
	}
#endif

#ifdef QOS_REMARKING
	if((skb->mark & QOS_REMARKING_FLAG))
		txq = (uint8)((skb->mark & QOS_REMARKING_MASK) >> 1);
	else
#endif
	if (priPktChk && (skb->len < priPktChkLen))
	{
		if (isPriorityPkt(skb->data, &newTxq))
		{
		#ifdef TCSUPPORT_QOS
			if (qosFlag == QOS_HW_WRR)
				txq = maxPrio; //maxPrio is the txq with max weight
			else
			{
				if (newTxq > txq)
					txq = newTxq;
			}
		#else
			txq = newTxq;
		#endif
		}
	}

#ifdef TCSUPPORT_RA_HWNAT
	if (ra_sw_nat_hook_tx != NULL) {
		ptm_info.word = 0;
		ptm_info.qptm.txq = (txq & 0xf);
		ptm_info.qptm.channel = ((line & 0x1) << PATH_LINE_SHIFT) | ((bearer & 0x1) << PATH_BEARER_SHIFT) | ((preemption & 0x1) << PATH_PREEMPT_SHIFT);		
		
		//for TRTCM test --Trey
		if (trtcmEnable)
		{
			ptm_info.qptm.tse = 1;
			ptm_info.qptm.tsid = trtcmTsid;
		}

		if (ra_sw_nat_hook_tx(skb, &ptm_info, FOE_MAGIC_PTM) == 0) {
			dev_kfree_skb_any(skb);
			ptmApt->ptmStats.ptmMIB.txDiscards++;
            PTM_DBG(DBG_L2, "\nError(%s): ra_sw_nat_hook_tx()!\n", __FUNCTION__);
			return NETDEV_TX_OK;
		}
	}
#endif


#ifdef LOOPBACK_SUPPORT
loopback_tx_xmit:

	if (internalLoopbackEn)
	{
		/* Decide what to filled in Tx Message by packet's content */
		if (loopback_tx_xmit_prepare(
				skb, &line, &bearer, 
				&preemption, &txq, &needHwVlanTag, &vlanTag,
				&ipCsIns)
		)
		{
			dev_kfree_skb_any(skb);
			ptmApt->ptmStats.ptmMIB.txDiscards++;
			stopTesting = 1;
			return NETDEV_TX_OK;
		}
	}
#endif

#ifdef EXTERNAL_LOOPBACK
external_loopback_tx:
#endif


	/* CONFIG_TX_POLLING_BY_MAC should be defined in
	 * QDMA driver first */
	if (ptmApt->txCntLimiter[txq] > 4)
	{
		if (qdma_txdscp_recycle(0))
			printk("\nFAILED(%s): qdma_txdscp_recycle\n", __FUNCTION__);
	}
	
	
	spin_lock_irqsave(&txRingLock, flags);

	if (
		(ptmApt->txCntLimiter[txq] >= TX_QUEUE_LEN) || 
		(txAvaiMsgHead == txAvaiMsgTail)
	)
	{
		ptmApt->ptmStats.ptmMIB.txDiscards++;
		spin_unlock_irqrestore(&txRingLock, flags);
		dev_kfree_skb_any(skb);
	#ifdef LOOPBACK_SUPPORT
		if (internalLoopbackEn)
			stopTesting = 1;
	#endif
		PTM_DBG(DBG_L2, "\ntxCntLimiter:%d (for txq%d) is full!\n", ptmApt->txCntLimiter[txq], txq);
		return NETDEV_TX_OK;		
	}

	//get a Tx message
	curTxMsgBuf = txAvaiMsgHead;
	txAvaiMsgHead = curTxMsgBuf->nextAvaiMsg;
	ptmApt->txCntLimiter[txq]++;
	
	spin_unlock_irqrestore(&txRingLock, flags);
	
	//clear Tx message before using
	curTxMsgBuf->nextAvaiMsg = NULL;
	curTxMsg = &curTxMsgBuf->ptmTxMsg;
	curTxMsg->txMsgW0.word = 0;
	curTxMsg->txMsgW1.word = 0;


	/* start filling tx message */

	curTxMsg->txMsgW0.bits.pathNo = ((line & 0x1) << PATH_LINE_SHIFT) | ((bearer & 0x1) << PATH_BEARER_SHIFT) | ((preemption & 0x1) << PATH_PREEMPT_SHIFT);

#ifdef LOOPBACK_SUPPORT
	if (internalLoopbackEn && isPtmPathIdSetEnable)
		curTxMsg->txMsgW0.bits.pathNo = ptmPathId;
#endif
		
	curTxMsg->txMsgW0.bits.queue = txq & TX_QUEUE_MASK;

	if (trtcmEnable)
	{
		curTxMsg->txMsgW0.bits.tse = 1;
		curTxMsg->txMsgW0.bits.tsid = trtcmTsid;
	}

	curTxMsg->txMsgW1.bits.fPort = 2; //2: for Low MAC
	
	if (needHwVlanTag)
	{
		curTxMsg->txMsgW1.bits.vlanEn = 1;
		
		if (((vlanTag >> 16) & 0xffff) == ETHER_TYPE_8021Q)
			curTxMsg->txMsgW1.bits.vlanTpID = 0;
		else if (((vlanTag >> 16) & 0xffff) == ETHER_TYPE_8021AD)
			curTxMsg->txMsgW1.bits.vlanTpID = 1;
		else if (((vlanTag >> 16) & 0xffff) == ETHER_TYPE_QINQ)
			curTxMsg->txMsgW1.bits.vlanTpID = 2;
		else
		{
			printk("\nFAILED: unknown vlanTPID:%.4x\n", 
					(uint16)((vlanTag >> 16) & 0xffff));
			dev_kfree_skb_any(skb);
			ptmApt->ptmStats.ptmMIB.txDiscards++;
		#ifdef LOOPBACK_SUPPORT
			if (internalLoopbackEn)
				stopTesting = 1;
		#endif
    		spin_lock_irqsave(&txRingLock, flags);
    		txAvaiMsgTail->nextAvaiMsg = curTxMsgBuf;
    		txAvaiMsgTail = curTxMsgBuf;
			ptmApt->txCntLimiter[txq]--;
    		spin_unlock_irqrestore(&txRingLock, flags);
			return NETDEV_TX_OK;
		}
			
		curTxMsg->txMsgW1.bits.vlanTag = (uint16)(vlanTag & 0xffff);
	}

	if (ipCsIns)
		curTxMsg->txMsgW1.bits.ipCsIns = 1;
	
#ifdef PTM_DEBUG
	if (ptmDbgLevel >= DBG_L3)
	{
		printk("\nDump TX Message:\n");
		dump_data((char*)curTxMsg, sizeof(ptmTxMsg_t));
		printk("\nDump skb->data:\n");
		dump_data(skb->data, skb->len);
	}
#endif

	error = qdma_transmit_packet(curTxMsg, sizeof(ptmTxMsg_t), skb);

	if (error)
	{
		spin_lock_irqsave(&txRingLock, flags);
		
		txAvaiMsgTail->nextAvaiMsg = curTxMsgBuf;
		txAvaiMsgTail = curTxMsgBuf;
		ptmApt->txCntLimiter[txq]--;

		spin_unlock_irqrestore(&txRingLock, flags);
		
		ptmApt->ptmStats.ptmMIB.txDiscards++;
		printk("\nFAILED: qdma_transmit_packet for "
				"line:%d, bearer:%d, preemption:%d, txq:%d\n", 
				line, bearer, preemption, txq);
		dev_kfree_skb_any(skb);
	#ifdef LOOPBACK_SUPPORT
		if (internalLoopbackEn)
			stopTesting = 1;
	#endif
		return NETDEV_TX_OK;		
	}


	/* ----- Count the MIB ----- */
	ptmApt->ptmStats.ptmMIB.txBytes += skb->len;
	ptmApt->ptmStats.ptmMIB.txPkts++;

	// Comment by camel, will Fix Soon
    // ledTurnOn(LED_DSL_ACT_STATUS);
#if defined(TR068_LED)
	/*for interner traffic led*/
	if(internet_led_on)	//IP connected and IP traffic is passing
		ledTurnOn(LED_INTERNET_ACT_STATUS);
	else
	{
		if(!internet_trying_led_on)
		{
			ledTurnOff(LED_INTERNET_STATUS);
			ledTurnOff(LED_INTERNET_TRYING_STATUS);
		}
	}	
#endif

	#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
	tbs_led_data_blinking(led_internet_green);
	#endif


	return NETDEV_TX_OK;
}


/* 1. call mt7510_ptm_rx once to receive just a packet
 *    until all packets are received. 
 * 2. If there are still packets left, put ptm napi 
 *    in kernel napi queue again. 
 * 3. If packets are all received, finish napi polling
 *     and enable QDMA Rx interrupt */
#if KERNEL_2_6_36
static int mt7510_ptm_napiPoll(struct napi_struct *napi, int budget)
{
	int n;
	unsigned long flags;

	/* call mt7510_ptm_rx to receive a packet 
	 * until all packets are received */
	n = qdma_receive_packets(budget);

	if (n < budget)
	{
		spin_lock_irqsave(&napiLock, flags);
	
		__napi_complete(napi);
		qdma_enable_rxpkt_int();

		spin_unlock_irqrestore(&napiLock, flags);
	}
	
	return n;
}
#else
static int mt7510_ptm_napiPoll(struct net_device *dev, int *budget)
{
/* Because qdmaEventHandler has no dev info, 
 * we always use nas8's poll function.
 */
	int rx_work_limit = min(dev->quota, *budget);
	int received = 0;
	int done;
	unsigned long flags;

	/* call mt7510_ptm_rx to receive a packet 
	 * until all packets are received */
	received = qdma_receive_packets(rx_work_limit);
	if (received >= rx_work_limit)
	{
		done = 0;
		goto more_work;
	}

	done = 1;

	spin_lock_irqsave(&napiLock, flags);
	
	__netif_rx_complete(dev);
	qdma_enable_rxpkt_int();

	spin_unlock_irqrestore(&napiLock, flags);


more_work:
	*budget -= received;
	dev->quota -= received;

	/* if 1 (done==0) is returned, poll function 
	 * will be called again. */
	return done ? 0 : 1;
}
#endif


#define MAC_BASE_MAGIC 
static int mt7510_ptm_start(struct net_device *dev)
{
	uint8 flashMacAddr[6];
    uint8 i;

#if KERNEL_2_6_36
	ptmAdapter_t *ptmApt = netdev_priv(dev);
#endif

#if 1
	//memcpy(defMacAddr, flashMacAddr, 6);
#else
	int ret_val = -ENOMEM;
	struct sockaddr addr;

	ret_val = tbs_read_mac(LAN_MAC, 0, addr.sa_data);
	net_srandom(jiffies);
	for(i = 0; i < SWITCH_PORT_MAX; i++) {
		if(SWITCH_PORT_MAX - 1 == i) {
			memset(addr.sa_data, 0x00, ETH_ALEN - 1);
			addr.sa_data[5] = net_random() & 0xFF;
		} else if(0 != ret_val) {/* generate a mac address */
			memcpy(addr.sa_data, mac, ETH_ALEN);
			addr.sa_data[5] = net_random() & 0xFF;
		}
		mt751x_mac_set_macaddr(switch_dev[i], &addr);
	}
#endif

	memcpy(dev->dev_addr, defMacAddr, 6);
	dev->addr_len = 6;

#if KERNEL_2_6_36
	ptmApt->dev = dev;
	
#else
  	/* Hook up with handlers */
	dev->do_ioctl 			= mt7510_ptm_ioctl;
  	dev->get_stats 			= mt7510_ptm_stats;
	dev->set_mac_address 	= mt7510_ptm_set_macAddr;
  	dev->open 				= mt7510_ptm_open;
  	dev->stop 				= mt7510_ptm_close;
	dev->set_multicast_list = mt7510_ptm_set_multicast_list;
	dev->hard_start_xmit 	= mt7510_ptm_tx;
	dev->poll 				= mt7510_ptm_napiPoll;
	dev->weight 			= RX_QUEUE_LEN >> 1;
#ifdef LOOPBACK_SUPPORT
	if (internalLoopbackEn)
		dev->hard_start_xmit 	= dummy_ptm_tx;
#endif
#ifdef EXTERNAL_LOOPBACK
	if (externalLoopbackEn)
		dev->hard_start_xmit 	= dummy_ptm_tx;
#endif

#endif

	printk(KERN_INFO
	       "%s: MT7510 PTM Ethernet address: %02X:%02X:%02X:%02X:%02X:%02X\n",
	       dev->name, 
	       dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
		   dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);


	return 0;
}

#if KERNEL_2_6_36
static struct net_device_ops ptm_netdev_ops = 
{
	.ndo_init				= mt7510_ptm_start,
	.ndo_open				= mt7510_ptm_open,
	.ndo_stop				= mt7510_ptm_close,
	.ndo_start_xmit			= mt7510_ptm_tx,
	.ndo_get_stats			= mt7510_ptm_stats,
	.ndo_set_multicast_list	= mt7510_ptm_set_multicast_list,
	.ndo_do_ioctl 			= mt7510_ptm_ioctl,
	//.ndo_change_mtu		= eth_change_mtu,
	//.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= mt7510_ptm_set_macAddr,
#ifdef CONFIG_NET_POLL_CONTROLLER
	//.ndo_poll_controller	= mt7510_ptm_poll_controller,
#endif
};
#endif

static int __init mt7510_ptm_init(void)
{
	struct net_device *dev;
	uint8 bearer;
	int error;
	char procName[32];
	char devName[8];
	struct proc_dir_entry *ptmProc;
    ptmAdapter_t *ptmApt;
#ifdef LOOPBACK_SUPPORT
    uint32 reg;
#endif

	printk(DRV_NAME " " DRV_VERSION " " DRV_RELDATE "\n");

#ifdef TCSUPPORT_BONDING
	//get slave Bonding and SCU base address.
	if (!pcie_virBaseAddr_get())
    {
        printk("\nError(%s): Can't get pcie virtual base address!\n", __FUNCTION__);
        return -EINVAL;
    }
	pcieVirBaseAddr = pcie_virBaseAddr_get();
	slaveBondingBaseAddr = pcieVirBaseAddr + (BONDING_REG_BASE & 0xffffff);
	slaveScuBaseAddr = pcieVirBaseAddr + 0xb00000;
	slaveTpstcBaseAddr = pcieVirBaseAddr + (TPSTC_REG_BASE & 0xffffff);
	printk("pcieVirBase:0x%.8x\tsBondingBase:0x%.8x\nsTpstcBase:0x%.8x\tsScuBase:0x%.8x\n",
		pcieVirBaseAddr, slaveBondingBaseAddr, slaveTpstcBaseAddr, slaveScuBaseAddr);
#endif

#ifdef LOOPBACK_SUPPORT
    if (internalLoopbackEn)
    {
    //release master dmt reset
    reg = read_reg_word(0xbfb00084);
    reg &= ~(1);
    write_reg_word(0xbfb00084, reg);
#ifdef TCSUPPORT_BONDING
    //release slave dmt reset
    reg = read_reg_word(pcieVirBaseAddr+0xb00084);
    reg &= ~(1);
    write_reg_word(pcieVirBaseAddr+0xb00084, reg);
#endif
    }
#endif
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);

	ptmMsgBuf_qdmaCfg_init();
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);

	if (!ptmMsgBufqdmaCfgInitialized)
	{
		printk("\nFAILED: ptmMsgBuf/qdmaCfg initialization!\n");
		return -EINVAL;
	}
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);


	/* PTM/Bonding will do reset before doing ptm reg init */
	ptm_reg_init();
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);

	frameEngineReg_init();
	printk("=========%s %d===========\n", __FUNCTION__, __LINE__);

	for (bearer = PTM_BEARER_0; bearer < BEARER_NUM; bearer++)
	{
		sprintf(devName, "nas%d", bearer + 8);
	
		//allocate net device
		dev = alloc_netdev(sizeof(ptmAdapter_t), devName, ether_setup);
		if (!dev)
		{
			printk("\n\nFAILED: PTM Bearer %d net_device allocation\n\n", bearer);
			for (bearer = PTM_BEARER_0; bearer < (BEARER_NUM - 1); bearer++)
				if (mt7510PtmDev[bearer])
					free_netdev(mt7510PtmDev[bearer]);

			return -ENOMEM;
		}

		mt7510PtmDev[bearer] = dev;
        ptmApt = netdev_priv(dev);
        memset(ptmApt, 0, sizeof(ptmAdapter_t));
		ptmApt->bearer = bearer;


#if KERNEL_2_6_36
	#ifdef LOOPBACK_SUPPORT
		if (internalLoopbackEn)
			ptm_netdev_ops.ndo_start_xmit = dummy_ptm_tx;
	#endif
	#ifdef EXTERNAL_LOOPBACK
		if (externalLoopbackEn)
			ptm_netdev_ops.ndo_start_xmit = dummy_ptm_tx;
	#endif
		dev->netdev_ops = &ptm_netdev_ops;
		ptmApt->napi.weight = PTM_NAPI_WEIGHT;
		netif_napi_add(dev, &ptmApt->napi, mt7510_ptm_napiPoll, PTM_NAPI_WEIGHT);

#else
		dev->init = mt7510_ptm_start;
#endif

		//register net device (mt7510_ptm_start will be done inside)
		error = register_netdev(dev);
		if (error)
		{
			printk("\n\nFAILED: PTM Bearer %d net_device register\n\n", bearer);
			for (bearer = PTM_BEARER_0; bearer < BEARER_NUM - 1; bearer++)
				if (mt7510PtmDev[bearer])
				{
					unregister_netdev(mt7510PtmDev[bearer]);
					free_netdev(mt7510PtmDev[bearer]);
				}
			return error;
		}


		sprintf(procName, "tc3162/ptm_b%d_stats", bearer);
		ptmProc = create_proc_entry(procName, 0, NULL);
		if (!ptmProc)
		{
			printk("\nFAILED: create proc for ptm_b%d_stats\n", bearer);
			return -ENOMEM;
		}
		ptmProc->read_proc = ptm_stats_read_proc;
		ptmProc->write_proc = ptm_stats_write_proc;
		ptmProc->data = &(ptmApt->bearer);
	}

	/* enable PTM/Bonding Tx/Rx by sequence */
	ptm_open();
    ptmInitialized = 1;

	ptmProc = create_proc_entry("tc3162/ptm_swReset", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_swReset\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_swReset_write_proc;

	ptmProc = create_proc_entry("tc3162/ptm_do_reset_sequence", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_do_reset_sequence\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_doResetSequence_write_proc;

	ptmProc = create_proc_entry("tc3162/ptm_reg_dump", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_reg_dump\n");
		return -ENOMEM;
	}
	ptmProc->read_proc = ptm_reg_dump_proc;
	ptmProc->write_proc = ptm_reg_clear_proc;

	ptmProc = create_proc_entry("tc3162/tpstc_reg_dump", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for tpstc_reg_dump\n");
		return -ENOMEM;
	}
	ptmProc->read_proc = tpstc_reg_dump_proc;

	ptmProc = create_proc_entry("tc3162/ptm_priPktChk", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_priPktChk\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_priPktChk_write_proc;
    
#ifdef PTM_DEBUG
	ptmProc = create_proc_entry("tc3162/ptm_dbgLevel", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_dbgLevel\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_dbgLevel_write_proc;

	ptmProc = create_proc_entry("tc3162/ptm_txCntLimiter_dump", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_txCntLimiter_dump\n");
		return -ENOMEM;
	}
	ptmProc->read_proc = ptm_txCntLimiter_dump_proc;
#endif

	ptmProc = create_proc_entry("tc3162/ptm_trafficShapingSet", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_trafficShapingSet\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_trafficShapingSet_write_proc;
	ptmProc->read_proc = ptm_trafficShapingSet_read_proc;

#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
	ptmProc = create_proc_entry("tc3162/ptm_qoswrr", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_qoswrr\n");
		return -ENOMEM;
	}
	ptmProc->read_proc = ptm_qoswrr_read_proc;
	ptmProc->write_proc = ptm_qoswrr_write_proc;
#endif

#ifdef TCSUPPORT_QOS
	ptmProc = create_proc_entry("tc3162/ptm_tcqos_disc", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_tcqos_disc\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_tcqos_write_proc;
#endif

#ifdef TCSUPPORT_BONDING
	ptmProc = create_proc_entry("tc3162/bonding_reg_dump", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for bonding_reg_dump\n");
		return -ENOMEM;
	}
	ptmProc->read_proc = bonding_reg_dump_proc;
	
	ptmProc = create_proc_entry("tc3162/bonding_check_reg", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for bonding_check_reg\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = bonding_check_reg_write_proc;

#ifdef FPGA_STAGE
	ptmProc = create_proc_entry("tc3162/bonding_lines_switching", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for bonding_lines_switching\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = bonding_lines_switching_write_proc;
#endif
#endif

#ifdef LOOPBACK_SUPPORT
	if (internalLoopbackEn)
		if (loopback_proc_init())
			return -ENOMEM;
#endif

#ifdef LINE_BONDING
	if (lineBonding_proc_init())
		return -ENOMEM;
#endif

#ifdef UNH_L2ENCAP
	l2encapProcInit(ptmProc);
#endif


	return 0;
}


static void mt7510_ptm_exit(void)
{
	uint8 bearer;
	char procName[32];


	/* close PTM/Bonding Tx/Rx by sequence */
	ptm_close();
	
	ptmMsgBuf_qdmaCfg_free(); 
	if (ptmMsgBufqdmaCfgInitialized)
	{
		printk("\nFAILED: free ptmMsgBuf/qdmaCfg!\n");
		return;
	}
    
    ptmInitialized = 0;


	for (bearer = PTM_BEARER_0; bearer < BEARER_NUM; bearer++)
	{
		unregister_netdev(mt7510PtmDev[bearer]);
		free_netdev(mt7510PtmDev[bearer]);
		mt7510PtmDev[bearer] = NULL;	
		sprintf(procName, "tc3162/ptm_b%d_stats", bearer);
		remove_proc_entry(procName, 0);
	}


	remove_proc_entry("tc3162/ptm_swReset", 0);
    remove_proc_entry("tc3162/ptm_do_reset_sequence", 0);
	remove_proc_entry("tc3162/ptm_reg_dump", 0);
	remove_proc_entry("tc3162/tpstc_reg_dump", 0);
	remove_proc_entry("tc3162/ptm_priPktChk", 0);
	remove_proc_entry("tc3162/ptm_dbgLevel", 0);
	remove_proc_entry("tc3162/ptm_txCntLimiter_dump", 0);
	remove_proc_entry("tc3162/ptm_trafficShapingSet", 0);

#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
	remove_proc_entry("tc3162/ptm_qoswrr", 0);
#endif

#ifdef TCSUPPORT_QOS
	remove_proc_entry("tc3162/ptm_tcqos_disc", 0);
#endif

#ifdef TCSUPPORT_BONDING
	remove_proc_entry("tc3162/bonding_reg_dump", 0);
	remove_proc_entry("tc3162/bonding_check_reg", 0);
#ifdef FPGA_STAGE
	remove_proc_entry("tc3162/bonding_lines_switching", 0);
#endif
#endif

#ifdef LOOPBACK_SUPPORT
	if (internalLoopbackEn)
		loopback_proc_remove();
#endif

#ifdef LINE_BONDING
	lineBonding_proc_remove();
#endif

	return;
}


/* Register startup/shutdown routines */
module_init(mt7510_ptm_init);
module_exit(mt7510_ptm_exit);
MODULE_LICENSE("Proprietary");










