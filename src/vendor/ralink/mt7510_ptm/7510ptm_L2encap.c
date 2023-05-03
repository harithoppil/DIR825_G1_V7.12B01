/*
 * ptm_L2encap.c
 *
 *  Created on: Nov 5, 2012
 *      Author: mtk04880
 */
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
#include <asm/spram.h>
#include <asm/io.h>
#include <linux/version.h>
#include <asm/tc3162/tc3162.h>
#include "../../version/tcversion.h"
#include <asm/tc3162/ledcetrl.h>
#include "7510ptm.h"


/***************************************************/
/*												   */
/****************Macro Definition	 ***************/
/*												   */
/***************************************************/
#ifdef LOOPBACK_SUPPORT

/* define loopback mode test */
#define LOOPBACK_TX			0x01
#define LOOPBACK_RX_DROP	0x02
#define LOOPBACK_RX_CHK		0x03
#define LOOPBACK_TX_QOS		0x04
#define LOOPBACK_MODE_MASK	0x0f
#define LOOPBACK_MODE(x)	((x) & 0x0f)
#define LOOPBACK_TX_IPCS	(1<<4)
#define LOOPBACK_TX_VLAN	(1<<5)
#define LOOPBACK_TX_RANDOM	(1<<6)
#define LOOPBACK_MSG		(1<<7)
#define LOOPBACK_TX_B0		(1<<8)
#define LOOPBACK_TX_B1		(1<<9)
#define LOOPBACK_TX_NO_MSG	(1<<10)
#define LOOPBACK_PKT		(1<<11)
#define LOOPBACK_TX_RANDOM2	(1<<12)
#define LOOPBACK_TX_F_P0	(1<<13)
#define LOOPBACK_TX_F_P7	(1<<14)
#define DEF_PRIORITY_PKT_CHK_LEN		100

#endif

/***************************************************/
/*												   */
/****************Structure Definition***************/
/*												   */
/***************************************************/

typedef struct L2Param_s{
	uint8 coMacAddr[6];
	uint16 dataLen;
	uint16 lineId;
	uint16 synSymbCnt;
	uint8 segCode;
	uint8 *chanData;
	uint8 reserved[3];
}L2Param_t;

typedef struct L2BakupParam_s{
	uint16 dataLen;
	uint8 *chanData;
	uint8 reserved[2];
}L2BakupParam_t;

/***************************************************/
/*												   */
/****************Global Variable********************/
/*												   */
/***************************************************/
#ifdef LOOPBACK_SUPPORT
extern uint16 ptmLoopback;
extern atomic_t ptmRxLoopback;
#endif
static int pktDbgEn = 0;
extern struct net_device *mt7510PtmDev[BEARER_NUM];



/***************************************************/
/*												   */
/****************Function Body********************/
/*												   */
/***************************************************/
int tc3262_ptm_L2encap_gen(L2Param_t* L2ParamO,char* selfFormData,int selfFormDataLen);

#ifdef LOOPBACK_SUPPORT
static int ptm_L2param_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "\n");
	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

static int ptm_L2param_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];
	int i = 0;
	int tmpId,tmpSymCnt,tmpCode,tmpLen;
	L2Param_t L2ParamNode;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';
	sscanf(val_string, "%d %d %d %d",&tmpId,&tmpSymCnt,&tmpCode,&tmpLen);

	L2ParamNode.dataLen = (uint16)tmpLen;
	L2ParamNode.lineId = (uint16)tmpId;
	L2ParamNode.synSymbCnt = (uint16)tmpSymCnt;
	L2ParamNode.segCode= (uint8)tmpCode;
	L2ParamNode.coMacAddr[0] = 0x0;
	L2ParamNode.coMacAddr[1] = 0x12;
	L2ParamNode.coMacAddr[2] = 0x34;
	L2ParamNode.coMacAddr[3] = 0xaa;
	L2ParamNode.coMacAddr[4] = 0x32;
	L2ParamNode.coMacAddr[5] = 0x46;

	L2ParamNode.chanData = kmalloc(L2ParamNode.dataLen,GFP_KERNEL);
	if(L2ParamNode.chanData){
		for(i = 0 ;i< L2ParamNode.dataLen;i++)
			L2ParamNode.chanData[i] = i + 13;
		tc3262_ptm_L2encap_gen(&L2ParamNode,NULL,0);
	}
	kfree(L2ParamNode.chanData);
	return count;
}
#endif
static int ptm_L2paramDbg_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "\n");
	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

static int ptm_L2paramDbg_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	pktDbgEn = simple_strtoul(val_string, NULL, 10) ? 1 : 0;
	return count;
}

void dump_pkt(struct sk_buff *skb,int cnt){
	int i = 0;
	char *p = skb->data;

	printk("====%dth packet====",cnt+1);
	for (i = 0; i < skb->len; i++) {
		if(i < 6){
			if(i == 0)
				printk("\n---DST MAC---\n");
		}
		else if(i < 12){
			if(i == 6)
				printk("\n---SRC MAC---\n");
		}
		else if(i< 14){
			if(i == 12)
				printk("\n---Length Field---\n");
		}
		else if(i< 17){
			if(i == 14)
				printk("\n---LLC Header---\n");
		}
		else if(i< 20){
			if(i == 17)
				printk("\n---ITU-T OUI---\n");
		}
		else if(i< 22){
			if(i == 20)
				printk("\n---PROTOCOL ID---\n");
		}
		else if(i< 24){
			if(i == 22)
				printk("\n---LINE ID---\n");
		}
		else if(i< 26){
			if(i == 24)
				printk("\n---SYNC SYMBL CNT---\n");
		}
		else if(i< 27){
			if(i == 26)
				printk("\n---SEG CODE---\n");
		}
		else{
			if(i == 27)
				printk("\n---CHAN Data---\n");
		}
		printk("%02x ",*p& 0xff);
		p++;
	}
	printk("\n");
}
static void dump_bakupPkt(struct sk_buff *skb)
{
	char tmp[80];
	char *p = skb->data;
	char *t = tmp;
	int i, n = 0;

	printk("skb=%08lx data=%08lx len=%d\n", (uint32) skb, (uint32) skb->data, skb->len);
	for (i = 0; i < skb->len; i++) {
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

int tc3262_ptm_L2encap_gen(L2Param_t* L2ParamO,char* selfFormData,int selfFormDataLen){
	int i = 0, k, rxpackets;
	struct sk_buff *skb;
	int tx_len;
	uint8 *tx_data;
	int tx_priority;
	int bearer;
	int rxq;
	static uint32 pktCnt;

	uint32 outDiscards;
#ifdef LOOPBACK_SUPPORT

	int retry = 0;
	uint32 loopback_pkt_seq[BEARER_NUM][RX_QUEUE_NUM];
	uint16 loopback_pkt_len[BEARER_NUM][RX_QUEUE_NUM];
	uint16 loopback_pkt_last_byte[BEARER_NUM][RX_QUEUE_NUM];

	printk("ptm_L2encap_gen test: seg code:%d Line ID:%d syn SymbCnt:%d dataLen:%d \n",\
			L2ParamO->segCode,L2ParamO->lineId,L2ParamO->synSymbCnt,\
			L2ParamO->dataLen);
	ptmLoopback |= LOOPBACK_TX_NO_MSG;
	for (bearer = PTM_B0; bearer <= PTM_B1; bearer += PTM_B1) {
		for (rxq = 0; rxq < RX_QUEUE_NUM; rxq++) {
			loopback_pkt_seq[bearer][rxq] = 0;
			loopback_pkt_len[bearer][rxq] = 0;
			loopback_pkt_last_byte[bearer][rxq] = 0;
		}
	}
	atomic_set(&ptmRxLoopback, 0);
	while (i < 10) {
#endif
		if(selfFormData && selfFormDataLen){
			skb = dev_alloc_skb(selfFormDataLen);
			if (skb == NULL)
#ifdef LOOPBACK_SUPPORT
				continue;
#else
				return 0;
#endif

#ifdef LOOPBACK_SUPPORT
			if ((i % 4096) == 0)
				printk("Gen %d packets.\n", i);
#endif
			tx_data = skb_put(skb,selfFormDataLen);
			memcpy(tx_data,selfFormData,sizeof(selfFormDataLen));
			if(pktDbgEn)
				dump_bakupPkt(skb);
		}
		else{
			skb = dev_alloc_skb(L2ParamO->dataLen+27);
			if (skb == NULL)
#ifdef LOOPBACK_SUPPORT
				continue;
#else
				return 0;
#endif

#ifdef LOOPBACK_SUPPORT
			if ((i % 4096) == 0)
				printk("Gen %d packets.\n", i);
#endif
			tx_len = L2ParamO->dataLen;
			/*DST MAC ADDR*/
			for (k = 0; k < 6; k++)
				tx_data[k] = L2ParamO->coMacAddr[k];

			/*SRC MAC ADDR*/
			tx_data[6] = 0x0;
			tx_data[7] = 0xaa;
			tx_data[8] = 0xbb;
			tx_data[9] = 0x1;
			tx_data[10] = 0x23;
			tx_data[11] = 0x45;

			/*Data Length--Line ID(2)+SyncSymCnt(2)+SegCode(1)*/
			put16(&tx_data[12],L2ParamO->dataLen+5);
			/*LLC PDU header*/
			tx_data[14] = 0xaa;
			tx_data[15] = 0xaa;
			tx_data[16] = 0x03;

			/*ITU-T OUI*/
			tx_data[17] = 0x0;
			tx_data[18] = 0x19;
			tx_data[19] = 0xa7;

			/*Protocol ID*/
			tx_data[20] = 0x0;
			tx_data[21] = 0x03;

			/*Line ID*/
			put16(&tx_data[22],L2ParamO->lineId);

			/*Sync Symbol Count*/
			put16(&tx_data[24],L2ParamO->synSymbCnt);

			/*Segment Code*/
			tx_data[26] = L2ParamO->segCode;

			/*Data Content*/
			for(k = 0;k < tx_len;k++){
				tx_data[k+27] = L2ParamO->chanData[k];
			}
			if(pktDbgEn)
				dump_pkt(skb,pktCnt++);
		}
		tx_data[18] =DATA_PATH_TESTTYPE ;	//data path test type
		tx_data[19] = (0 & 0xf) | ((0 & 0xf) << 4);	//path & txq
		mt7510_ptm_tx(skb, mt7510PtmDev[0]);

#ifdef LOOPBACK_SUPPORT
		i++;
	}
	printk("Gen %d packets done.\n", i);
#endif
	return 0;
}

int l2encapProcInit(struct proc_dir_entry *ptm_proc){
	if(ptm_proc){
#ifdef LOOPBACK_SUPPORT
		ptm_proc = create_proc_entry("tc3162/ptm_l2encap_lbk", 0, NULL);
		ptm_proc->read_proc = ptm_L2param_read_proc;
		ptm_proc->write_proc = ptm_L2param_write_proc;
#endif
		ptm_proc = create_proc_entry("tc3162/ptm_l2encap_dbgEn", 0, NULL);
		ptm_proc->read_proc = ptm_L2paramDbg_read_proc;
		ptm_proc->write_proc = ptm_L2paramDbg_write_proc;
		return 0;
	}
	else{
		return -1;
	}
}

EXPORT_SYMBOL(tc3262_ptm_L2encap_gen);

