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

#include "7510ptm.h"
#include "../bufmgr/qdma_api.h"
#include "../fe_api/fe_api.h"


#define RX_BUF_LEN 	(2048 - NET_SKB_PAD - 64 - (sizeof(struct skb_shared_info)))
#define PKT_PATTERN_NUM	6


#define CHK_BUF() pos = begin + index; if (pos < off) { index = 0; begin = pos; }; if (pos > off + count) goto done;


#define INC_PKTTYPE			0
#define RAN_PKTTYPE			1
#define ZERO_PKTTYPE		2
#define FF_PKTTYPE			3
#define FA_PKTTYPE			4
#define AF_PKTTYPE			5

#define LOOP_SLEEP_TIME		500
#define TEST_SLEEP_TIME		5000
#define TX_HOLD_TIME		msecs_to_jiffies(1)

/* in case QDMA IRQ (default is 512) is full, just put
 * (TX_QUEUE_LEN-4) packets in each Txq */
#define TX_BURST_LEN		(TX_QUEUE_LEN-4) 

#define MAX_PKT_SIZE		1650

#define BACP_CHECK_STAGE	29

extern uint8 defMacAddr[];
extern struct net_device *mt7510PtmDev[];
extern int mt7510_ptm_tx(struct sk_buff *skb, struct net_device *dev);
extern void dump_data(char *p, int len);
#ifdef PTM_DEBUG
extern int ptmDbgLevel;
#endif
#ifdef TCSUPPORT_BONDING
extern unsigned int pcieVirBaseAddr;
#endif

static int loopbackPoint = 0;
static int testType = -1, nPackets = 0;
static int dataPathRxPkts = 0;
static uint8 qdmaDmaAvailable = 0;
static uint8 qdmaWaitingCnt = 0;
static int testPktCnt = 0;
static int curTxq;
static int wrrQueuePhase, wrrQueueStart;
static uint8 txqWrrVal[TX_QUEUE_NUM] = {1,2,3,4,5,6,7,8};
static uint32 txqWrrTotalVal;
static int totalTxPkts;
static int rxPreemptionPkts;
atomic_t isTxHold = ATOMIC_INIT(0);
static int bacpCheckStage = 0;
static int isAllTestsOnGoing = 0;
static int minPktSize = 60, maxPktSize = 1514;
static int startPath = 0, endPath = PATH_NUM;
static int isCrcCheckTest = 0;
static int isWrrFirstTest[PATH_NUM] = {1,1,1,1,1,1,1,1};

uint8 dstMacAddr[] = {0x00, 0xaa, 0xbb, 0x5a, 0x5a, 0x5a};
unsigned int bacpHeader[] = {0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000};
int stopTesting = 0;
int ptmPathId = 0, isPtmPathIdSetEnable = 0;


static int ptm_pathId_write_proc(
		struct file *file, const char *buffer,
		unsigned long count, void *data
)
{
	char valString[8];

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d %d", &isPtmPathIdSetEnable, &ptmPathId);

	if (isPtmPathIdSetEnable == 1)
		printk("\nPTM PATH ID: %d\n\n", ptmPathId);
	else
		printk("\nPathIdSet disabled!\n\n");

	return count;
}

/* "echo M N > /proc/tc3162/ptm_loopback_pathRangeSet"
 * to allow only (M) ~ (N-1) channel packets can be
 * sent. */
static int ptm_loopbackPathRangeSet_write_proc(
		struct file *file, const char *buffer, 
		unsigned long count, void *data
)
{
	char valString[32];
	int value1, value2;

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d %d", &value1, &value2);

	if (value1 > value2)
		printk("\nWrong: the first value: %d should be smaller than the second value:%d\n", value1, value2);
	else
	{
		startPath = value1;
		endPath = value2;

		printk("\npath range is %d~%d\n", startPath, endPath-1);
	}

	return count;
}


/* Fix packet szie between minPktSize ~ maxPktSize */
static int ptm_loopbackSetPktSize_write_proc(
		struct file *file, const char *buffer, 
		unsigned long count, void *data
)
{
	char valString[32];
	int value1, value2;

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d %d", &value1, &value2);

	if (value1 > value2)
		printk("\nWrong: the first value: %d should be smaller than the second value:%d\n", value1, value2);
	else if ((value1 < 60) || (value2 > 1514))
		printk("\nWrong: The correct packet size should be 60~1514\n");
	else
	{
		minPktSize = value1;
		maxPktSize = value2;

		printk("\nminPktSize: %d\nmaxPktSize: %d\n", minPktSize, maxPktSize);
	}

	return count;
}

/* Decide what is going to be filled in Tx Message 
 * by Byte18~19 of packet's content,
 * which are test-type dependent */
int loopback_tx_xmit_prepare(
		struct sk_buff *skb, int *line, uint8 *bearer, 
		int *preemption, int *txq, int *needHwVlanTag, 
		uint32 *vlanTag, uint8 *ipCsIns
)
{
	uint8 *txData;
	uint16 ethType;
	
	txData = skb->data;
	ethType = *(uint16*)(&txData[12]);

	if (ethType == ETHER_TYPE_IP)
	{
		switch (txData[18])	//test type
		{
			case SINGLE_PKT_TESTTYPE:
			case VLAN_UNTAG_TESTTYPE:
			case VLAN_DBL_TAG_TESTTYPE:
			case VLAN_DBL_UNTAG_TESTTYPE:
			case IPCS_CHECK_TESTTYPE:
			case BACP_PKT_TESTTYPE:
			case RUNT_PKT_TESTTYPE:
			case LONG_PKT_TESTTYPE:
			case CRCERR_PKT_TESTTYPE:
				break;
			case PATH_NO_TESTTYPE:
			case DATA_PATH_TESTTYPE:
			case DATA_BURST_TESTTYPE:
			case MULTI_CHANNEL_TESTTYPE:
			case PREEMPTION_TESTTYPE:
			case SP_PRIORITY_TESTTYPE:
			case WRR_PRIORITY_TESTTYPE:
			case SP_WRR_TESTTYPE:
				*txq = (txData[19] >> 4) & 0xf;
				*needHwVlanTag = 0;
				break;
			case VLAN_TAG_TESTTYPE:
				if ((txData[19]>>4) & 0x1)
				{
					*needHwVlanTag = 1;
					*vlanTag = (ETHER_TYPE_8021Q << 16) | 
							((*((uint16*)(&txData[20]))) & 0x1fff);
				}
				break;
			case IPCS_INSERT_TESTTYPE:
				if ((txData[19]>>4)&0x1)
					*ipCsIns = 1;
				break;
				
			default:
				printk("\nWrong loopback test type:%d "
						"for ETHER_TYPE_IP\n", txData[18]);
				return -1;
		}
	}
	else if (ethType == ETHER_TYPE_8021Q)
	{
		txData += 4;
		
		switch (txData[18])
		{
			case VLAN_UNTAG_TESTTYPE:
				*needHwVlanTag = 0;
				break;
			case VLAN_DBL_TAG_TESTTYPE:
				*needHwVlanTag = 1;
				if (((txData[19]>>4)&0x3) == 1)
					*vlanTag = (ETHER_TYPE_8021AD << 16) | 
							((*((uint16*)(&txData[20]))) & 0x1fff);
				else if (((txData[19]>>4)&0x3) == 3)
					*vlanTag = (ETHER_TYPE_QINQ << 16) |
							((*((uint16*)(&txData[20]))) & 0x1fff);					break;
			default:
				printk("\nWrong loopback test type:%d "
						"for ETHER_TYPE_8021Q\n", txData[18]);
				return -1;
		}
	}
	else if ((ethType == ETHER_TYPE_8021AD) || 
			(ethType == ETHER_TYPE_QINQ))
	{
		txData += 8;
		*needHwVlanTag = 0;
	}
	else if (ethType == ETHER_TYPE_SLOW)
	{
		txData += 10;
	}
	else
	{
		printk("\nWrong ethType:%04x loopback packet, drop it!\n", ethType);
			return -1;
	}

	*line = (txData[19] >> PATH_LINE_SHIFT) & 0x1;
	*bearer = (txData[19] >> PATH_BEARER_SHIFT) & 0x1;
	*preemption =(txData[19] >> PATH_PREEMPT_SHIFT) & 0x1;

	return 0;
}

#ifdef EXTERNAL_LOOPBACK
/* exchange packet's src & dst MAC address
 * and then send it back to DSLM */
int ptm_externalLoopbackRx(
	ptmRxMsg_t *curRxMsg, ptmAdapter_t *ptmApt,
	struct sk_buff *skb, struct sk_buff **skbPtr,
	struct net_device *dev, uint frameSize
)
{
	uint8 *skbData;
	uint8 macAddr[6];
	uint32 offset;


	skbData = (uint8*) skb->data;
	if (skbData[0] == defMacAddr[0] && 
		skbData[1] == defMacAddr[1] && 
		skbData[2] == defMacAddr[2] && 
		skbData[3] == defMacAddr[3] && 
		skbData[4] == defMacAddr[4] && 
		skbData[5] == defMacAddr[5] &&
		skbData[6] == 0x00 && skbData[7] == 0xAA && 
		skbData[8] == 0xBB && skbData[9] == 0x22 && 
		skbData[10] == 0x22 && skbData[11] == 0x22)
	{
		if ((curRxMsg->rxMsgW0.word & PTM_RX_W0_ERR_BITS) 
		/*|| (curRxMsg->rxMsgW1.word & PTM_RX_W1_ERR_BITS)*/
		)
		{
			*skbPtr = skb;
			return 1;
		}

		memcpy(macAddr, &skbData[0], 6);
		memcpy(&skbData[0], &skbData[6], 6);
		memcpy(&skbData[6], macAddr, 6);

		skb->len = frameSize;

		*skbPtr = skbmgr_dev_alloc_skb2k();
		if (*skbPtr)
		{
			//shift to 4-byte alignment
			offset = ((uint32)((*skbPtr)->tail)) & 0x3;
			if (offset)
				skb_reserve(*skbPtr, 4 - offset);

			mt7510_ptm_tx(skb, dev);
			
			return 0;
		}
		else
		{
			printk("\nFAILED: alloc skb for external loopback\n");
			*skbPtr = skb;
			return 1;
		}
	}
	else  //drop packets that aren't for external loopback
	{
		*skbPtr = skb;
		return 1;			
	}
}
#endif

static void bacpHeader_write(unsigned int h0, unsigned int h1, unsigned int h2, unsigned int h3)
{
	bacpHeader[0] = h0;
	bacpHeader[1] = h1;
	bacpHeader[2] = h2;
	bacpHeader[3] = h3;
}

static void bacpReg_write(unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3, unsigned int r4)
{
	write_reg_word(PTM_BACP_FIELD_0, r0);
	write_reg_word(PTM_BACP_FIELD_1, r1);
	write_reg_word(PTM_BACP_FIELD_2, r2);
	write_reg_word(PTM_BACP_FIELD_3, r3);
	write_reg_word(PTM_BACP_FIELD_EN, r4);
}

static void bacpHeader_read(void)
{
	PTM_DBG(DBG_L1, "\nbacpHeader[0]:0x%.8x\n"
			 "bacpHeader[1]:0x%.8x\n"
			 "bacpHeader[2]:0x%.8x\n"
			 "bacpHeader[3]:0x%.8x\n\n",
			 bacpHeader[0], bacpHeader[1], 
			 bacpHeader[2], bacpHeader[3]);
}

static void bacpReg_read(void)
{
	PTM_DBG(DBG_L1, "\nbacpReg0:0x%.8x\n"
			 "bacpReg1:0x%.8x\n"
			 "bacpReg2:0x%.8x\n"
			 "bacpReg3:0x%.8x\n"
			 "bacpRegEn:0x%.8x\n\n",
			 (unsigned int)read_reg_word(PTM_BACP_FIELD_0), 
			 (unsigned int)read_reg_word(PTM_BACP_FIELD_1), 
			 (unsigned int)read_reg_word(PTM_BACP_FIELD_2), 
			 (unsigned int)read_reg_word(PTM_BACP_FIELD_3), 
			(unsigned int) read_reg_word(PTM_BACP_FIELD_EN));
}

static int ptm_loopbackBacpHdrSet_write_proc(
		struct file *file, const char *buffer, 
		unsigned long count, void *data
)
{
	char valString[64];
	

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%x %x %x %x", &bacpHeader[0], &bacpHeader[1], &bacpHeader[2], &bacpHeader[3]);


	return count;
}

static int ptm_loopbackBacpRegSet_write_proc(
		struct file *file, const char *buffer, 
		unsigned long count, void *data
)
{
	char valString[64];
	unsigned int bacpReg[5];
	

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%x %x %x %x %x", &bacpReg[0], &bacpReg[1], &bacpReg[2], &bacpReg[3], &bacpReg[4]);

	bacpReg_write(bacpReg[0], bacpReg[1], 
			 bacpReg[2], bacpReg[3], bacpReg[4]);

	bacpCheckStage = 88;

	return count;
}


static int ptm_loopbackStopTest_write_proc(
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

	sscanf(valString, "%d", &stopTesting);

	return count;
}

void ptmTcLoopbackPoint_set(void)
{
#ifndef TCSUPPORT_BONDING
    uint32 reg;
#endif

    //set for PTM-TC loopback for line0
    write_reg_word(0xbf900030, 2);

#ifdef TCSUPPORT_BONDING
    //set for PTM-TC loopback for line1
    write_reg_word(pcieVirBaseAddr+0x900030, 2);
#else
    //no dummy R/W for dmt
    reg = read_reg_word(TPSTC_RX_CFG);
    reg |= TPSTC_REDUNDANT_MODE;
    write_reg_word(TPSTC_RX_CFG, reg);
#endif

    printk("\nSetting for loopback at PTM-TC\n");
    printk("\nReminder: rmmod dmt.ko when doing loopback!\n\n");

    return;
}


/* "echo N > /proc/tc3162/ptm_loopback_point" 
 * to set loopback point:
 * when N=1, loopback at QDMA,
 * when N=2: loopback at GDM (U-MAC),
 * when N=3: loopback at PTM-TC */
static int ptm_loopbackPoint_write_proc(
		struct file *file, const char *buffer, 
		unsigned long count, void *data
)
{
	char valString[4];
	uint32 reg;

	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;

	valString[count] = '\0';

	sscanf(valString, "%d", &loopbackPoint);


	switch (loopbackPoint)
	{
		case 1:
			//QDMA loopback
			qdma_loopback_mode(QDMA_LOOPBACK_QDMA);
			printk("\nloopback at QDMA\n\n");
			break;
		case 2:
			//GDM (Upper MAC) loopback
			qdma_loopback_mode(QDMA_LOOPBACK_UMAC);
			/* disable CRC stripping in GDM, 
			 * otherwise RX packet size will be lack of 4 */
			reg = fe_reg_read(GDM2_FWD_CFG_OFF);
			reg &= ~(1<<16);
			fe_reg_write(GDM2_FWD_CFG_OFF, reg);
			printk("\nloopback at GDM (U-MAC)\n\n");
			break;
		case 3:
			ptmTcLoopbackPoint_set();		
			break;
		default:
			printk("\nUsage: echo N > /proc/tc3162/ptm_loopback_point, where:\n"
					"N==1: loopback at QDMA\n"
					"N==2: loopback at GDM (U-MAC)\n"
					"N==3: loopback at PTM-TC\n\n");
			break;
	}

	return count;
}


static int ptm_loopback_registersCheck(int tryCnt)
{
	unsigned int reg = PTM_REG_BASE;
	int i, j;

	printk("\nStart checking PTM MAC registers for %d times\n", tryCnt);

	for (j = 0; j < tryCnt; j++)
	{
		if ((read_reg_word(reg) & 0x3) != 0x0)
		{
			printk("\nWrong default value for 0x%08x:\n"
					"read_reg_word(reg):0x%08lx\n"
					"Expected default value: 0x0\n",
					reg, read_reg_word(reg));
			return -1;
		}

		//after nas8/nas9 up, PTM_CTRL_REG has become 0x02010101
		if (read_reg_word(PTM_CTRL_REG) != 0x02010101)
		{
			printk("\nWrong default value for PTM_CTRL_REG(0x%08x):\n"
					"read_reg_word(PTM_CTRL_REG):0x%08lx\n"
					"Expected default value: 0x02010101\n",
					PTM_CTRL_REG, read_reg_word(PTM_CTRL_REG));
			return -1;
		}
	
		reg = 0xbfb62010;
	
		for (i = 0; i < 5; i++)
		{
			if (read_reg_word(reg+(i<<2)) != 0x0)
			{
				printk("\nWrong default value for 0x%08x:\n"
						"read_reg_word(reg):0x%08lx\n"
						"Expected default value: 0x0\n",
						reg+(i<<2), read_reg_word(reg+(i<<2)));
				return -1;
			}
		}
	
		reg = 0xbfb62080;
	
		for (i = 0; i < 7; i++)
		{
			if (read_reg_word(reg+(i<<2)) != 0x0)
			{
				printk("\nWrong default value for 0x%08x:\n"
						"read_reg_word(reg):0x%08lx\n"
						"Expected default value: 0x0\n",
						reg+(i<<2), read_reg_word(reg+(i<<2)));
				return -1;
			}
		}
	
		reg = 0xbfb62100;
	
		for (i = 0; i < 9; i++)
		{
			if (read_reg_word(reg+(i<<2)) != 0x0)
			{
				printk("\nWrong default value for 0x%08x:\n"
						"read_reg_word(reg):0x%08lx\n"
						"Expected default value: 0x0\n",
						reg+(i<<2), read_reg_word(reg+(i<<2)));
				return -1;
			}
		}
	
		reg = 0xbfb62130;
	
		for (i = 0; i < 9; i++)
		{
			if (read_reg_word(reg+(i<<2)) != 0x0)
			{
				printk("\nWrong default value for 0x%08x:\n"
						"read_reg_word(reg):0x%08lx\n"
						"Expected default value: 0x0\n",
						reg+(i<<2), read_reg_word(reg+(i<<2)));
				return -1;
			}
		}
	
		reg = 0xbfb62160;
	
		for (i = 0; i < 9; i++)
		{
			if (read_reg_word(reg+(i<<2)) != 0x0)
			{
				printk("\nWrong default value for 0x%08x:\n"
						"read_reg_word(reg):0x%08lx\n"
						"Expected default value: 0x0\n",
						reg+(i<<2), read_reg_word(reg+(i<<2)));
				return -1;
			}
		}
	
		reg = 0xbfb62190;
	
		for (i = 0; i < 9; i++)
		{
			if (read_reg_word(reg+(i<<2)) != 0x0)
			{
				printk("\nWrong default value for 0x%08x:\n"
						"read_reg_word(reg):0x%08lx\n"
						"Expected default value: 0x0\n",
						reg+(i<<2), read_reg_word(reg+(i<<2)));
				return -1;
			}
		}
	
		reg = 0xbfb621c0;
	
		for (i = 0; i < 9; i++)
		{
			if (read_reg_word(reg+(i<<2)) != 0x0)
			{
				printk("\nWrong default value for 0x%08x:\n"
						"read_reg_word(reg):0x%08lx\n"
						"Expected default value: 0x0\n",
						reg+(i<<2), read_reg_word(reg+(i<<2)));
				return -1;
			}
		}
	
		/* 0xbfb621e4~e8 are probe signal registers, don't care.*/
	}
	
	printk("\nregisters check OK for %d times\n", tryCnt);
		
	return 0;
}


static unsigned short in_csum(unsigned short *ptr, int nbytes)
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


/* 
 * Content format:
 * -> Byte0~13: Dest MAC Addr, Src MAC Addr, Ethernet Type
 * -> Byte14[bit0~3]: packet payload type which will decide
 *            the content after 20th byte.
 * -> Byte14[bit4~7]: IP version.
 * -> Byte15: base value for content after 20th byte.
 * -> Byte16~17: packet length.
 * -> Byte18: test type.
 * -> Byte19: test-type dependent value.
 * -> Byte20~ : packet payload.
 */
static int txCommonContent_fill(
		struct sk_buff **pSkb, uint8 **pTxData,
		int pkt, int type, uint16 etherType, 
		int offset, uint32 pktLen
)
{
	uint32 k, txLen, tagId = 0;
	uint8 baseVal;
	uint8 *txData;
	struct sk_buff *skb;
	int pktType = type & 0xf;
	uint8 *dstMacPtr = NULL;
	uint8 *bacpHdr = NULL;


	skb = dev_alloc_skb(RX_BUF_LEN);
	if (skb == NULL)
	{
		printk("\nFAILED: skb allocation:\n");
		return -1;
	}

	//when 0, pkt len is decided by packet payload type
	if (!pktLen)
	{
		if (pktType == RAN_PKTTYPE)
			txLen = random32();
		else
			txLen = pkt;

		//txLen is 60~1514 by default.(CRC not included)
		if ( (txLen < minPktSize) || (txLen > maxPktSize) )
		{
			txLen %= (maxPktSize - minPktSize + 1);
			txLen += minPktSize;
		}
	}
	else
		txLen = pktLen;

	txData = skb_put(skb, txLen);

	/* destination mac should be changed, because
	 * BACP has it own destination mac */
	if ((etherType == ETHER_TYPE_SLOW) && (pkt & 1))
		dstMacPtr = (uint8 *)&bacpHeader[0];
	else
		dstMacPtr = &dstMacAddr[0];

	//destination mac
	for (k = 0; k < 6; k++)
		txData[k] = dstMacPtr[k];

	//source mac
	for (k = 6; k < 12; k++)
		txData[k] = defMacAddr[k-6];


	/* byte12~13 are for ethernet type, but in VLAN
	 * & BACP test cases, the rule may be chanegd. */
	*((uint16*)(&txData[12])) = etherType;


	//for VLAN_UNTAG_TESTTYPE
	if (etherType == ETHER_TYPE_8021Q) 
	{
		if (pkt & 1) //vlan packets
		{
			tagId = random32() & 0x1fff; //13-bit tagId
			*((uint16*)(&txData[14])) = tagId;
			*((uint16*)(&txData[16])) = ETHER_TYPE_IP;
		
			txData += 4; //shift 4 bytes
			txLen -= 4;
		
			*((uint16*)(&txData[20])) = tagId;
		}
		else //normal packets
			*((uint16*)(&txData[12])) = ETHER_TYPE_IP;	
	}
	//for VLAN_DBL_TAG_TESTTYPE
	else if (etherType == ETHER_TYPE_8021AD) 
	{
		if (((pkt & 3) == 1) || ((pkt & 3) == 3))
		{
			*((uint16*)(&txData[12])) = ETHER_TYPE_8021Q;
			
			if ((pkt & 3) == 1)
				*((uint16*)(&txData[14])) = 0x1234;
			else
				*((uint16*)(&txData[14])) = 0x1567;
				
			*((uint16*)(&txData[16])) = ETHER_TYPE_IP;

			txData += 4; //shift 4 bytes
			txLen -= 4;	

			*((uint16*)(&txData[20])) = (uint16) (random32() & 0x1fff);
		}
		else
			*((uint16*)(&txData[12])) = ETHER_TYPE_IP;
	}
	//for VLAN_DBL_UNTAG_TESTTYPE
	else if (etherType == ETHER_TYPE_QINQ) 
	{
		if (((pkt & 3) == 1) || ((pkt & 3) == 3))
		{
			if ((pkt & 3) == 1)
				*((uint16*)(&txData[12])) = ETHER_TYPE_8021AD;
				
			tagId = random32() & 0x1fff; //13-bit tagId
			*((uint16*)(&txData[14])) = tagId;				

			*((uint16*)(&txData[16])) = ETHER_TYPE_8021Q;
			
			if ((pkt & 3) == 1)
				*((uint16*)(&txData[18])) = 0x189a;
			else
				*((uint16*)(&txData[18])) = 0x1bcd;
				
			*((uint16*)(&txData[20])) = ETHER_TYPE_IP;

			txData += 8; //shift 8 bytes
			txLen -= 8;	

			*((uint16*)(&txData[20])) = tagId;
		}
		else
			*((uint16*)(&txData[12])) = ETHER_TYPE_IP;
	}
	//for BACP_PKT_TESTTYPE
	else if (etherType == ETHER_TYPE_SLOW)
	{
		if (pkt & 1) //bacp packets
		{
			bacpHdr = (uint8 *)&bacpHeader[2];
		
			txData[14] = bacpHdr[0]; //subtype
			txData[15] = bacpHdr[1]; //ITU OUI
			txData[16] = bacpHdr[2]; 
			txData[17] = bacpHdr[3];
			txData[18] = bacpHdr[4]; //ITU subtype
			txData[19] = bacpHdr[5]; //BACP version
			*((uint32*)(&txData[20])) = 0x0; //BACP timestamp

			txData += 10; //shift 10 bytes
			txLen -= 10;
		}
		else //normanl packets
			*((uint16*)(&txData[12])) = ETHER_TYPE_IP;
	}

	//ip version4 & pkt payload type
	txData[14] = ((4 << 4) | pktType);

	//base value for payload
	switch (pktType)
	{
		case INC_PKTTYPE:
			baseVal = (uint8) (pkt & 0xff);
			break;
		case RAN_PKTTYPE:
			baseVal = (uint8) (random32() & 0xff);
			break;
		case ZERO_PKTTYPE:
			baseVal = 0;
			break;
		case FF_PKTTYPE:
			baseVal = 0xff;
			break;
		case FA_PKTTYPE:
			baseVal = 0x5a;
			break;
		case AF_PKTTYPE:
			baseVal = 0xa5;
			break;
		default:
			printk("\nWrong PKT pattern:\n"
					"type:%d, pkt:%d\n",
					pktType, pkt);
			return -1;
	}	
	txData[15] = baseVal;
	

	//packet length
	*((uint16*)(&txData[16])) = (uint16) (txLen & 0xffff);
	

	/* Fill packet payload:
	 * If payload type is INC or RAN, payload value will
	 * be incremental starting from baseVal, otherwise 
	 * payload is baseVal. 
	 * Packet payload is starting from byte20, except
	 * VLAN tests. */
	for (k = offset; k < txLen; k++)
	{
		txData[k] = baseVal;
		if (pktType == INC_PKTTYPE || pktType == RAN_PKTTYPE)
		{
			if (baseVal == 0xff)
				baseVal = 0;
			else
				baseVal++;
		}
	}

	*pSkb = skb;
	*pTxData = txData;

	return 0;
}


/* After sending a packet, wait until the packet
 * is received by loopback Rx, before a new
 * packet can be sent */
struct task_struct *txProcess = NULL;
static void loopback_pkt_transmit(
		struct sk_buff *skb, uint8 bearer
)
{
	atomic_set(&isTxHold, 1);
	txProcess = NULL;
	
	mt7510_ptm_tx(skb, mt7510PtmDev[bearer]);
		
	while (atomic_read(&isTxHold))
	{
	#if 0
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(1); //jiffies
		set_current_state(TASK_RUNNING);
	#endif
	#if 1
		txProcess = current;
		if (atomic_read(&isTxHold))
			schedule();
	#endif
	}
}

static void rxWakeUpTx(void)
{
	atomic_set(&isTxHold, 0);
	if (txProcess)
		wake_up_process(txProcess);
}

/* for debug only, not included in real tests */
static int ptm_loopback_singlePkt_tx(uint32 pktLen)
{
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int offset = 20;
	

	printk("\nStart %s test\n", __FUNCTION__);

	if (txCommonContent_fill(&skb, &txData, 0, RAN_PKTTYPE, ETHER_TYPE_IP, offset, pktLen))
		return -1;

	
	txData[18] = SINGLE_PKT_TESTTYPE; //single packet test type
	txData[19] = 0;	//not used


	if (!stopTesting)
	{
		mt7510_ptm_tx(skb, mt7510PtmDev[PTM_BEARER_0]);
	}
	else
	{
		printk("\nStop Testing (single packet):\n");
		return -1;
	}


	printk("\nSinglePkt TX OK with pktLen:%d\n", (int)pktLen);
	return 0;
}


static int dataPath_tx(
		int path, int txq, int type, int pkt
)
{
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	uint8 bearer = 0;
	int offset = 20;


	if (txCommonContent_fill(&skb, &txData, pkt, type, ETHER_TYPE_IP, offset, 0))
		return -1;
	
	
	txData[18] =DATA_PATH_TESTTYPE ;	//data path test type
	txData[19] = (path & 0xf) | ((txq & 0xf) << 4);	//path & txq


	if (!stopTesting)
	{
		totalTxPkts++;
		bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
		if ((totalTxPkts & 0xffff) != 0xffff)
		{
			PTM_DBG(DBG_L1, "%s: path:%d, txq:%d, type:%d, pkt:%d\n", __FUNCTION__, path, txq, type, pkt);
		}
		else
			printk("%s: path:%d, txq:%d, type:%d, pkt:%d\n", 
				__FUNCTION__, path, txq, type, pkt);
				
		loopback_pkt_transmit(skb, bearer);
	}
	else
	{
		printk("\nStop Testing (data path):\n"
				"path:%d, txq:%d, type:%d, pkt:%d, totalTxPkts:%d\n",
				path, txq, type, pkt, totalTxPkts);
		return -1;
	}

	return 0;
}

/* send a packet, wait until the packet is received
 * before next packet can be sent */
static int ptm_loopback_dataPath_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	int pkt, type = 0, path = 0, txq = 0;
	

	printk("\nStart %s test\n", __FUNCTION__);

	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;


data_path_loop:

	for (path = startPath; path < endPath; path++)
	{
		for (txq = 0; txq < TX_QUEUE_NUM; txq++)
		{
  			for (type = 0; type < PKT_PATTERN_NUM; type++)
  			{
				for (pkt = 0; pkt < nPkts; pkt++)
				{
					if (dataPath_tx(path, txq, type, pkt))
						return -1;
				}
  			}
		}
	}


	if (loop)
	{
		printk("\nData Path TX Loop %d OK\n", loop);
		loop++;
		goto data_path_loop;
	}

	printk("\nData Path TX OK with %d pkts!\n", totalTxPkts);
	return 0;
}


static int dataBurst_tx(
		int path, int txq, int type, int pkt
)
{
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	uint8 bearer = 0;
	int offset = 20;


	if (txCommonContent_fill(&skb, &txData, pkt, type, ETHER_TYPE_IP, offset, 0))
		return -1;
	
	
	txData[18] =DATA_BURST_TESTTYPE ;	//data burst test type
	txData[19] = (path & 0xf) | ((txq & 0xf) << 4);	//path & txq


	totalTxPkts++;
	bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
	PTM_DBG(DBG_L1, "%s: path:%d, txq:%d, pkt:%d\n", 
				__FUNCTION__, path, txq, pkt);

	mt7510_ptm_tx(skb, mt7510PtmDev[bearer]);

	return 0;
}

/* For each path test: 
 * 1. hold QDMA
 * 2. send (TX_QUEUE_NUM*TX_BURST_LEN) packets to PSE buffer
 * 3. turn on QDMA
 * 4. wait until all loopback packets are receved
 */
static int ptm_loopback_dataBurst_tx(uint32 pktNum)
{
	int loop = 0;
	int pkt, path = 0, txq = 0;
	

	printk("\nStart %s test\n", __FUNCTION__);

	if (pktNum == 0)
		if (!isAllTestsOnGoing)
			loop = 1;


data_burst_loop:
	
	/* The number of PTM RX messages should be as many as 
	 * (TX_QUEUE_NUM*TX_QUEUE_LEN), otherwise QDMA may
	 * be out of RX descriptors when receiving RX packets,
	 * because the process of receiving a packet is slow! 
	 */
  	for (path = startPath; path < endPath; path++)
  	{
		qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);
	
		for (txq = 0; txq < TX_QUEUE_NUM; txq++)
		{
			for (pkt = 0; pkt < TX_BURST_LEN; pkt++)
			{
				if (dataBurst_tx(path, txq, RAN_PKTTYPE, pkt))
					return -1;
			}
  		}

		testPktCnt = 1; //init testPktCnt
		dataPathRxPkts = 0;
		qdmaDmaAvailable = 0;
		qdmaWaitingCnt = 0;
		qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_DISABLE); //noHwFwd
		
		while (!qdmaDmaAvailable)
		{
			if (++qdmaWaitingCnt == 3)
			{
				printk("\nWaiting for too long! (%d pkts sent, %d pkts received)\n", totalTxPkts, dataPathRxPkts);
				return -1;
			}
			msleep(LOOP_SLEEP_TIME);
		}

		if (stopTesting)
		{
			printk("\nStop Testing (data burst):\n"
					"path:%d, totalTxPkts:%d\n",
					path, totalTxPkts);
			return -1;
		}
	}


	if (loop)
	{
		printk("\nData Burst TX Loop %d OK\n", loop);
		loop++;
		goto data_burst_loop;
	}

	printk("\nData Burst TX OK with %d pkts!\n", totalTxPkts);
	return 0;
}


static int multiChannel_tx(
		int path, int txq, int type, int pkt
)
{
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	uint8 bearer = 0;
	int offset = 20;


	if (txCommonContent_fill(&skb, &txData, pkt, type, ETHER_TYPE_IP, offset, 0))
		return -1;
	
	
	txData[18] = MULTI_CHANNEL_TESTTYPE; //multi channel test type
	txData[19] = (path & 0xf) | ((txq & 0xf) << 4);	//path & txq


	totalTxPkts++;
	bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
	PTM_DBG(DBG_L1, "%s: path:%d, txq:%d, pkt:%d\n", 
				__FUNCTION__, path, txq, pkt);

	mt7510_ptm_tx(skb, mt7510PtmDev[bearer]);

	return 0;
}

/* For every 2 paths, do the following for 1500 times: 
 * 1. hold QDMA
 * 2. send a packet for path(N) and another packet
 *    for path(N+1) to PSE buffer
 * 3. turn on QDMA
 * 4. wait until the 2 loopback packets are receved
 */
static int ptm_loopback_multiChannel_tx(uint32 pktNum)
{
	int loop = 0;
	int pkt, path = 0, txq = 0, path2 = 0;
	

	printk("\nStart %s test\n", __FUNCTION__);

	if (pktNum == 0)
		if (!isAllTestsOnGoing)
			loop = 1;


multi_channel_loop:

		
  	for (path = startPath; path < endPath; path++)
  	{	
		for (pkt = 0; pkt < 24; pkt++)
		{
			txq = pkt & 0x7;
		
			qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);

			if (multiChannel_tx(path, txq, RAN_PKTTYPE, pkt))
				return -1;

			if ((path+1) == endPath)
				path2 = startPath;
			else
				path2 = path+1;

			if (multiChannel_tx(path2, txq, RAN_PKTTYPE, pkt))
				return -1;

			testPktCnt = 1; //init testPktCnt
			qdmaDmaAvailable = 0;
			qdmaWaitingCnt = 0;
			qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_DISABLE); //noHwFwd
		
			while (!qdmaDmaAvailable)
			{
				if (++qdmaWaitingCnt == 3)
				{
					printk("\nWaiting for too long! (%d pkts sent, %d pkts received)\n", totalTxPkts, dataPathRxPkts);
					return -1;
				}
				msleep(LOOP_SLEEP_TIME);
			}

			if (stopTesting)
			{
				printk("\nStop Testing (multi channel):\n"
						"path:%d, totalTxPkts:%d\n",
						path, totalTxPkts);
				return -1;
			}
		}
	}


	if (loop)
	{
		printk("\nMulti Channel TX Loop %d OK\n", loop);
		loop++;
		goto multi_channel_loop;
	}

	printk("\nMulti Channel TX OK with %d pkts!\n", totalTxPkts);
	return 0;
}


static int ptm_loopback_pathNo_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	uint8 bearer;
	int pkt, path;
	int offset = 20;
	

	printk("\nStart %s test\n", __FUNCTION__);

	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;


path_no_loop:

	for (pkt = 0; pkt < nPkts; pkt++)
	{
		for (path = startPath; path < endPath; path++)
		{

			if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_IP, offset, 0))
				return -1;
		
	
			txData[18] = PATH_NO_TESTTYPE;	//path_no test type
			txData[19] = path & 0xf; //path


			if (!stopTesting)
			{
				totalTxPkts++;
				bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
				PTM_DBG(DBG_L1, "%s: pkt:%d, path:%d\n", 
				__FUNCTION__, pkt, path);

				loopback_pkt_transmit(skb, bearer);
			}
			else
			{
				printk("\nStop Testing (path_no):\n"
						"pkt:%d, path:%d, totalTxPkts:%d\n",
						pkt, path, totalTxPkts);
				return -1;
			}
  		}
	}	


	if (loop)
	{
		printk("\nPath_No TX Loop %d OK\n", loop);
		loop++;
		goto path_no_loop;
	}

	printk("\nPath_No TX OK with %d pkts!\n", totalTxPkts);
	return 0;
}


static int preemption_tx(
		int path, int txq, int pkt
)
{
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int offset = 20;
	int bearer;


	if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_IP, offset, 1500))
		return -1;
			
	
	txData[18] = PREEMPTION_TESTTYPE;	//preemption test type
	
	txData[19] = (path & 0xf) | ((txq & 0xf) << 4); //path & txq


	totalTxPkts++;
	bearer = (path>>PATH_BEARER_SHIFT) & 0x1;

	PTM_DBG(DBG_L1, "%s: path:%d, txq:%d, pkt:%d\n", 
				__FUNCTION__, path, txq, pkt);
				
	mt7510_ptm_tx(skb, mt7510PtmDev[bearer]);

	return 0;
}


/* For a normal path and a preemption path: 
 * 1. hold QDMA
 * 2. send 60 packets for the normal path and 4
 *    packets for preemption path to PSE buffer
 * 3. turn on QDMA
 * 4. wait until the 4 preemption packets are all
 *    receved before all 60 normal packets are
 *    received. */
static int ptm_loopback_preemption_tx(uint32 pktNum)
{
	int loop = 0;
	int pkt, txq;
	int path, pathNo;
	

	printk("\nStart ptm_loopback_preemption test\n");

	if (pktNum == 0)
		if (!isAllTestsOnGoing)
			loop = 1;


preemption_loop:

	/* PTM MAC can support only 2 paths to move at
	 * the same time for 30MHz POF*/

	for (txq = 0; txq < TX_QUEUE_NUM; txq++)
	{
	for (path = startPath; path < endPath; path++)
	{
		rxPreemptionPkts = 0;
		dataPathRxPkts = 0;
		PTM_DBG(DBG_L1, "\nPreemption Tx with path%d txq%d\n", path, txq);
	
		qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);

		/* compare normal & preemptive packets in the same line & bearer.
		 * that is, path0 compares with path1, path2 with path3, path4 
		 * with path5, path6 with path7. */

		if (path & 1)
		{
			//normal
			pathNo = path - 1;
			for (pkt = 0; pkt < 60; pkt++)
			{
				if (preemption_tx(pathNo, txq, pkt))
					return -1;
			}

			//preemption
			pathNo = path;
			for (pkt = 0; pkt < 4; pkt++)
			{
				if (preemption_tx(pathNo, txq, pkt))
					return -1;
			}
		}
		else
		{
			//normal
			pathNo = path;
			for (pkt = 0; pkt < 60; pkt++)
			{
				if (preemption_tx(pathNo, txq, pkt))
					return -1;
			}
			
			//preemption
			pathNo = path + 1;
			if (pathNo == endPath && startPath != (endPath-1) )
				pathNo = startPath;

			for (pkt = 0; pkt < 4; pkt++)
			{
				if (preemption_tx(pathNo, txq, pkt))
					return -1;
			}			
		}
	
		qdmaDmaAvailable = 0;
		qdmaWaitingCnt = 0;
	
		qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_DISABLE);

		while (!qdmaDmaAvailable)
		{
			if (++qdmaWaitingCnt == 3)
			{
				printk("\nWaiting for too long! (%d pkts sent, %d normal & %d preemption pkts received)\n", totalTxPkts, dataPathRxPkts, rxPreemptionPkts);
				return -1;
			}
			msleep(LOOP_SLEEP_TIME);
		}

		if (stopTesting)
		{
			printk("\nStop Testing (preemption):\n"
					"3 paths from path%d, txq:%d, totalTxPkts:%d\n",
					path, txq, totalTxPkts);
			return -1;
		}
		
	}
	}
	

	if (loop)
	{
		printk("\nPreemption TX Loop %d OK\n", loop);
		loop++;
		
		goto preemption_loop;
	}

	printk("\nPreemption TX OK with %d pkts!\n", totalTxPkts);
	return 0;
}


static int spPriority_tx(
		int path, int txq, int pkt
)
{
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	uint8 bearer;
	int offset = 20;


	if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_IP, offset, 0))
		return -1;

	
	txData[18] = SP_PRIORITY_TESTTYPE;	//SP test type

	txData[19] = (path & 0xf) | ((txq & 0xf) << 4); //path & txq


	totalTxPkts++;
	bearer = (path >> (PATH_BEARER_SHIFT)) & 0x1;
	PTM_DBG(DBG_L1, "%s: path:%d, txq:%d, pkt:%d\n", 
			__FUNCTION__, path, txq, pkt);
	mt7510_ptm_tx(skb, mt7510PtmDev[bearer]);

	return 0;
}

/* For each path test:
 * 1. set QDMA as Strict Priority mode
 * 2. enable QDMA TX buffer usage but set threshold to 0
 * 3. send low priority and then high priority packets
 *    (total (TX_QUEUE_NUM*TX_BURST_LEN) packets) to 
 *    QDMA link manager.
 * 4. hold QDMA.
 * 5. release QDMA TX buffer usage
 * 6. turn on QDMA
 * 7. wait until all loopback packets are receved
 *    according to their priority.
 */
static int ptm_loopback_spPriority_tx(uint32 pktNum)
{
	int loop = 0;
	int pkt, path;
	int txq;
	QDMA_TxQosScheduler_T txQos;
	QDMA_TxBufCtrl_T txBufUsage;
	

	printk("\nStart %s test\n", __FUNCTION__);

	if (pktNum == 0)
		if (!isAllTestsOnGoing)
			loop = 1;


sp_priority_loop:
	
	/* The number of PTM RX messages should be as many as 
	 * (TX_QUEUE_NUM*TX_QUEUE_LEN), otherwise QDMA may
	 * be out of RX descriptors when receiving RX packets,
	 * because the process of receiving a packet is slow! 
	 */
  	for (path = startPath; path < endPath; path++)
  	{
		/* set QDMA as Strip Priority Mode,
	 	 * so that p7> ... >p0 for each path */
		memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
		txQos.channel = path;
		txQos.qosType = QDMA_TXQOS_TYPE_SP;
		if (qdma_set_tx_qos(&txQos))
		{
			printk("\nERROR(%s): qdma_set_tx_qos for path%d\n", __FUNCTION__, path);
			return -1;
		}
	
		//init parameters for RX
		curTxq = 7;

		/* enable TX buffer usage but set threshold to 0,
		 * then packets will be queued in QDMA link manager
		 * in priority but won't be sent */
		memset(&txBufUsage, 0, sizeof(QDMA_TxBufCtrl_T));
		txBufUsage.mode = QDMA_ENABLE;
		if (qdma_set_txbuf_threshold (&txBufUsage))
		{
			printk("\nFAILED(%s): qdma enable txBufUsage\n" , __FUNCTION__);
			return -1;
		}

		//low priority packets go into QDMA first!
		for (txq = 0; txq < TX_QUEUE_NUM; txq++)
		{
			for (pkt = 0; pkt < TX_BURST_LEN; pkt++)
			{
				if (spPriority_tx(path, txq, pkt))
					return -1;
			}
  		}

		qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);

		testPktCnt = 1; //initialize testPktCnt
		dataPathRxPkts = 0;
		qdmaDmaAvailable = 0;
		qdmaWaitingCnt = 0;

		//release TX buffer usage
		memset(&txBufUsage, 0, sizeof(QDMA_TxBufCtrl_T));
		if (qdma_set_txbuf_threshold (&txBufUsage))
		{
			printk("\nFAILED(%s): qdma disable txBufUsage\n" , __FUNCTION__);
			return -1;
		}

		qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_DISABLE); //noHwFwd

		while (!qdmaDmaAvailable)
		{
			if (++qdmaWaitingCnt == 3)
			{
				printk("\nWaiting for too long! (%d pkts sent, %d pkts received)\n", totalTxPkts, dataPathRxPkts);
				return -1;
			}
			msleep(LOOP_SLEEP_TIME);
		}
		
		if (stopTesting)
		{
			printk("\nStop Testing (SP priority):\n"
					"path:%d, totalTxPkts:%d\n", 
					path, totalTxPkts);
			return -1;
		}		
	}

	if (loop)
	{
		printk("\nSP priority TX Loop %d OK\n", loop);
		loop++;
		
		goto sp_priority_loop;
	}

	printk("\nSP priority TX OK with %d pkts!\n", totalTxPkts);
	return 0;
}


static int wrrPriority_tx(
		int path, int txq, int pkt
)
{
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	uint8 bearer;
	int offset = 20;


	if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_IP, offset, 0))
		return -1;

	
	txData[18] = WRR_PRIORITY_TESTTYPE;	//WRR test type

	txData[19] = (path & 0xf) | ((txq & 0xf) << 4); //path & txq


	totalTxPkts++;
	bearer = (path >> (PATH_BEARER_SHIFT)) & 0x1;
	PTM_DBG(DBG_L1, "%s: path:%d, txq:%d, pkt:%d, totalTxPkts:%d\n", __FUNCTION__, path, txq, pkt, totalTxPkts);
	mt7510_ptm_tx(skb, mt7510PtmDev[bearer]);

	return 0;
}

/* For each path test:
 * 1. set QDMA as Weighted Round Robin mode
 * 2. send a packet for every queue of the path.
 * 2. enable QDMA TX buffer usage but set threshold to 0
 * 3. send multiple of txqWrrVal[txqN] packets for txqN
 *    to QDMA link manager.
 * 4. hold QDMA.
 * 5. release QDMA TX buffer usage
 * 6. turn on QDMA
 * 7. wait until all loopback packets are receved
 *    according to their priority. */
static int ptm_loopback_wrrPriority_tx(uint32 pktNum)
{
	int loop = 0;
	int pkt, path;
	int txq, i;
	QDMA_TxQosScheduler_T txQos;
	QDMA_TxBufCtrl_T txBufUsage;
	

	printk("\nStart %s test\n", __FUNCTION__);

	if (pktNum == 0)
		if (!isAllTestsOnGoing)
			loop = 1;

	txqWrrTotalVal = 0;
	for (i = 0; i < TX_QUEUE_NUM; i++)
		txqWrrTotalVal += txqWrrVal[i];

wrr_priority_loop:

  	for (path = startPath; path < endPath; path++)
  	{

		/* set QDMA's WRR priority registers,
	 	 * so that p7: ... :p0 == 8: ... :1 
		 */
		memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
		txQos.channel = path;
		txQos.qosType = QDMA_TXQOS_TYPE_WRR;
		for(i = 0 ; i < TX_QUEUE_NUM ; i++)
			txQos.queue[i].weight = txqWrrVal[i];
					
		if (qdma_set_tx_qos(&txQos))
		{
			printk("\nERROR(%s): qdma_set_tx_qos for path%d\n"
					, __FUNCTION__, path);
			return -1;
		}
		else //success
		{
			memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
			txQos.channel = path;
			qdma_get_tx_qos(&txQos) ;
			printk("\nWRR setting OK for Path:%d, Type:%d\n"
					"Q0:%d, Q1:%d, Q2:%d, Q3:%d, Q4:%d, Q5:%d, Q6:%d, Q7:%d\n\n",
					path, txQos.qosType, 
					txQos.queue[0].weight,
					txQos.queue[1].weight,
					txQos.queue[2].weight,
					txQos.queue[3].weight,
					txQos.queue[4].weight,
					txQos.queue[5].weight,
					txQos.queue[6].weight,
					txQos.queue[7].weight);	
		}


		/* Due to HW's design, HW WRR counters won't be accurate
		 * until every queue (per path) sends a packet */
		if (isWrrFirstTest[path])
		{
			qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);
			
			for (txq = 0; txq < TX_QUEUE_NUM; txq++)
			{
				if (wrrPriority_tx(path, txq, 0))
					return -1;
			}

			dataPathRxPkts = 0;
			qdmaDmaAvailable = 0;
			qdmaWaitingCnt = 0;
	
			qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_DISABLE); //noHwFwd
			
			while (!qdmaDmaAvailable)
			{
				if (++qdmaWaitingCnt == 3)
				{
					printk("\nWaiting for too long for WRR dirst test for path%d\n", path);
					return -1;
				}
				msleep(LOOP_SLEEP_TIME);
			}

			isWrrFirstTest[path] = 0;
		}


		/* begin real WRR test */

		/* enable TX buffer usage and set threshold to 0,
		 * then packets will be queued in QDMA link manager
		 * in priority but won't be sent */
		memset(&txBufUsage, 0, sizeof(QDMA_TxBufCtrl_T));
		txBufUsage.mode = QDMA_ENABLE;
		if (qdma_set_txbuf_threshold (&txBufUsage))
		{
			printk("\nFAILED(%s): qdma enable txBufUsage\n" , __FUNCTION__);
			return -1;
		}

		/* due to HW's design, put multiple txqWrrVal[txqN]
		 * packets for txqN ! So we put 8, 16, 24, 32
		 * 40, 48, 56, 64 packets in txq0, 1, 2 ,3, 4,
		 * 5, 6, 7 respectively */
		for (txq = 0; txq < TX_QUEUE_NUM; txq++)
		{
			for (pkt = 0; pkt < (txqWrrVal[txq]<<3); pkt++)
			{
				if (wrrPriority_tx(path, txq, pkt))
					return -1;
			}
		}

		qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);
		
		
		//init paremeters
		wrrQueuePhase = 0;
		wrrQueueStart = 0;
		qdmaDmaAvailable = 0;
		qdmaWaitingCnt = 0;
		dataPathRxPkts = 0;

		//release TX buffer usage
		memset(&txBufUsage, 0, sizeof(QDMA_TxBufCtrl_T));
		if (qdma_set_txbuf_threshold (&txBufUsage))
		{
			printk("\nFAILED(%s): qdma disable txBufUsage\n" , __FUNCTION__);
			return -1;
		}
	
		qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_DISABLE); //noHwFwd

		while (!qdmaDmaAvailable)
		{
			if (++qdmaWaitingCnt == 3)
			{
				printk("\nWaiting for too long! (%d pkts sent, %d pkts received)\n", totalTxPkts, dataPathRxPkts);
				return -1;
			}
			msleep(LOOP_SLEEP_TIME);
		}

		if (stopTesting)
		{
			printk("\nStop Testing (WRR priority):\n"
					"path:%d, totalTxPkts:%d\n", 
					path, totalTxPkts);
			return -1;
		}
	}


	/* WRR counters will be reset 
	 * when not looping in the same Path. */
	if (endPath > startPath+1)
	{
		for (path = startPath; path < endPath; path++)
			isWrrFirstTest[path] = 1;
	}

	if (loop)
	{
		printk("\nWRR priority TX Loop %d OK\n", loop);
		loop++;
		
		goto wrr_priority_loop;
	}


	printk("\nWRR priority TX OK with %d pkts!\n", totalTxPkts);


	/* set QDMA QOS registers back to default values
	 * which is QDMA_TXQOS_TYPE_SP */
	for (i = startPath; i < endPath; i++)
	{
		memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
		txQos.channel = i;
		txQos.qosType = QDMA_TXQOS_TYPE_SP;
		if (qdma_set_tx_qos(&txQos))
		{
			printk("\nERROR(%s): qdma_set_tx_qos for path%d\n", __FUNCTION__,i);
			return -1;
		}
	}
	
	return 0;
}


static int spWrr_tx(
		int path, int txq, int pkt
)
{
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	uint8 bearer;
	int offset = 20;


	if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_IP, offset, 0))
		return -1;
			
	
	txData[18] = SP_WRR_TESTTYPE;	//SP+WRR test type

	txData[19] = (path & 0xf) | ((txq & 0xf) << 4); //path & txq


	totalTxPkts++;
	bearer = (path >> (PATH_BEARER_SHIFT)) & 0x1;
	PTM_DBG(DBG_L1, "%s: path:%d, txq:%d, pkt:%d\n", 
			__FUNCTION__, path, txq, pkt);
	mt7510_ptm_tx(skb, mt7510PtmDev[bearer]);

	return 0;
}


static int spWrr_tx_put(int path)
{
	int txq, pkt;

	/* Here we focus on q4:q3:q2:q1:q0 == 5:4:3:2:1 */

	/* due to HW's design, put multiple of txqWrrVal[txqN]
	 * packets for txqN ! So we put 12, 24, 36, 48
	 * 60 packets in txq0, 1, 2 ,3, 4 respectively */
	for (txq = 0; txq < 5; txq++)
	{
		for (pkt = 0; pkt < (txqWrrVal[txq]*12); pkt++)
		{
			if (spWrr_tx(path, txq, pkt))
				return -1;
		}
	}
		
	// init variables
	wrrQueuePhase = 0;
	wrrQueueStart = 0;
	dataPathRxPkts = 0;


	/* Here we focus on q7>q6>q5> q4,q3,q2,q1,q0 */

	for (txq = 5; txq < TX_QUEUE_NUM; txq++)
	{
		/* in case QDMA IRQ is full, just put TX_BURST_LEN
		 * packets in each Txq */
		for (pkt = 0; pkt < TX_BURST_LEN; pkt++)
		{
			if (spWrr_tx(path, txq, pkt))
				return -1;
		}
  	}

	//init parameters for RX
	curTxq = 7;
	testPktCnt = 1; //initialize testPktCnt

	return 0;
}

/* For each path test:
 * 1. set QDMA as SP+WRR mode
 * 2. send a packet for every queue of the path.
 * 2. enable QDMA TX buffer usage but set threshold to 0
 * 3. send multiple of txqWrrVal[txqN] packets for txq0~4
 *    and then (3*TX_BURST_LEN) packets for txq5~7 
 *    to QDMA link manager.
 * 4. hold QDMA.
 * 5. release QDMA TX buffer usage
 * 6. turn on QDMA
 * 7. wait until all loopback packets are receved
 *    according to their priority. */
static int ptm_loopback_spWrr_tx(uint32 pktNum)
{
	int loop = 0;
	int i, path, txq;
	QDMA_TxQosScheduler_T txQos;
	QDMA_TxBufCtrl_T txBufUsage;
	

	printk("\nStart %s test\n", __FUNCTION__);

	if (pktNum == 0)
		if (!isAllTestsOnGoing)
			loop = 1;

	txqWrrTotalVal = 0;
	for (i = 0; i < 5; i++)
		txqWrrTotalVal += txqWrrVal[i];

spWrr_loop:

  	for (path = startPath; path < endPath; path++)
  	{
		/* set QDMA's SP+WRR priority registers,
	 	 * so that p7>p6>p5> p4:p3:p2:p1:p0 == 5:4:3:2:1
		 */
		memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
		txQos.channel = path;
		txQos.qosType = QDMA_TXQOS_TYPE_SPWRR5;
		for(i = 0 ; i < 5; i++)
			txQos.queue[i].weight = txqWrrVal[i];
					
		if (qdma_set_tx_qos(&txQos))
		{
			printk("\nERROR(%s): qdma_set_tx_qos for path%d\n"
					, __FUNCTION__, path);
			return -1;
		}
		else //success
		{
			memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
			txQos.channel = path;
			qdma_get_tx_qos(&txQos) ;
			printk("\nSP+WRR setting OK for Path:%d, Type:%d\n"
					"Q7> Q6> Q5> Q4:%d, Q3:%d, Q2:%d, Q1:%d, Q0:%d\n\n",
					path, txQos.qosType, 
					txQos.queue[4].weight,
					txQos.queue[3].weight,
					txQos.queue[2].weight,
					txQos.queue[1].weight,
					txQos.queue[0].weight);
		}


		/* Due to HW's design, HW WRR counters will be
		* accurate after every queue (per path) sends a packet */
		if (isWrrFirstTest[path])
		{
			qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);
			
			for (txq = 0; txq < TX_QUEUE_NUM; txq++)
			{
				if (spWrr_tx(path, txq, 0))
					return -1;
			}

			dataPathRxPkts = 0;
			qdmaDmaAvailable = 0;
			qdmaWaitingCnt = 0;
	
			qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_DISABLE); //noHwFwd
			
			while (!qdmaDmaAvailable)
			{
				if (++qdmaWaitingCnt == 3)
				{
					printk("\nWaiting for too long for WRR dirst test for path%d\n", path);
					return -1;
				}
				msleep(LOOP_SLEEP_TIME);
			}
			
			isWrrFirstTest[path] = 0;
		}


		/* begine real SP_WRR test */

		//init variables
		qdmaDmaAvailable = 0;
		qdmaWaitingCnt = 0;			

		/* enable TX buffer usage and set threshold to 0,
		 * then packets will be queued in QDMA link manager
		 * in priority but won't be sent */
		memset(&txBufUsage, 0, sizeof(QDMA_TxBufCtrl_T));
		txBufUsage.mode = QDMA_ENABLE;
		if (qdma_set_txbuf_threshold (&txBufUsage))
		{
			printk("\nFAILED(%s): qdma enable txBufUsage\n" , __FUNCTION__);
			return -1;
		}		 

		if (spWrr_tx_put(path))
			return -1;

		qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);

		//release TX buffer usage
		memset(&txBufUsage, 0, sizeof(QDMA_TxBufCtrl_T));
		if (qdma_set_txbuf_threshold (&txBufUsage))
		{
			printk("\nFAILED(%s): qdma disable txBufUsage\n" , __FUNCTION__);
			return -1;
		}

		qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_DISABLE); //noHwFwd


		while (!qdmaDmaAvailable)
		{
			if (++qdmaWaitingCnt == 3)
			{
				printk("\nWaiting for too long! (%d pkts sent)\n", totalTxPkts);
				return -1;
			}
			msleep(LOOP_SLEEP_TIME);
		}

		if (stopTesting)
		{
			printk("\nStop Testing (SP+WRR priority):\n"
					"path:%d, totalTxPkts:%d\n", 
					path, totalTxPkts);
			return -1;
		}
	}


	/* WRR counters will be reset 
	 * when not looping in the same Path. */
	if (endPath > startPath+1)
	{
		for (path = startPath; path < endPath; path++)
			isWrrFirstTest[path] = 1;
	}

	if (loop)
	{
		printk("\nSP+WRR priority TX Loop %d OK\n", loop);
		loop++;
		
		goto spWrr_loop;
	}


	printk("\nSP+WRR priority TX OK with %d pkts!\n", totalTxPkts);


	/* set QDMA QOS registers back to default values
	 * which is QDMA_TXQOS_TYPE_SP */
	for (i = startPath; i < endPath; i++)
	{
		memset(&txQos, 0, sizeof(QDMA_TxQosScheduler_T));
		txQos.channel = i;
		txQos.qosType = QDMA_TXQOS_TYPE_SP;
		if (qdma_set_tx_qos(&txQos))
		{
			printk("\nERROR(%s): qdma_set_tx_qos for path%d\n", __FUNCTION__,i);
			return -1;
		}
	}

	return 0;
}

/* HW to tag. SW to check if the tagging is correct */
static int ptm_loopback_vlanTag_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int pkt, path, bearer;
	int offset = 22;
	uint32 reg = 0;
	

	printk("\nStart %s test\n", __FUNCTION__);


	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;


	/* disable "Extract outer VLAN tag"
	 * in order to use SW to un-tag */
	reg = fe_reg_read(CDM_VLAN_GE_OFF);
	reg &= ~(0x2);
	fe_reg_write(CDM_VLAN_GE_OFF, reg);


vlan_tag_loop:

	for (path = startPath; path < endPath; path++)
	{
	for (pkt = 0; pkt < nPkts; pkt++)
	{
		if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_IP, offset, 0))
			return -1;

	
		txData[18] = VLAN_TAG_TESTTYPE;	//vlan_tag test type
		
		//even pkt: HW don't tag, odd pkt: HW tag.
		txData[19] = (path&0xf) | ((pkt&1)<<4);

		if ((txData[19]>>4) & 0x1)
		{
			//13-bit vlan tag
			*((uint16*)(&txData[20])) = (uint16) (random32() & 0x1fff);
			//Rx will receive 4 more bytes
			*((uint16*)(&txData[16])) += 4; 
		}

		if (!stopTesting)
		{
			totalTxPkts++;
			bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
			PTM_DBG(DBG_L1, "%s: path:%d, pkt:%d\n", 
			__FUNCTION__, path, pkt);

			loopback_pkt_transmit(skb, bearer);
		}
		else
		{
			printk("\nStop Testing (vlanTag):\n"
					"path:%d, pkt:%d, totalTxPkts:%d\n",
					path, pkt, totalTxPkts);
			return -1;
		}
	}
	}


	if (loop)
	{
		printk("\nVlan_Tag TX Loop %d OK\n", loop);
		loop++;
		goto vlan_tag_loop;
	}

	printk("\nVlan_Tag TX OK with %d pkts!\n", totalTxPkts);
	return 0;
	
}

/* SW to tag. HW to un-tag for SW to analysize  */
static int ptm_loopback_vlanUnTag_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int pkt = 0, path, bearer;
	int offset = 22;
	uint32 reg = 0;
	

	printk("\nStart %s test\n", __FUNCTION__);


	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;



	/* enable "Extract outer VLAN tag" 
	 * in order to use HW to un-tag */
	reg = fe_reg_read(CDM_VLAN_GE_OFF);
	reg |= 0x2;
	fe_reg_write(CDM_VLAN_GE_OFF, reg);

	
vlan_untag_loop:

	for (path = startPath; path < endPath; path++)
	{
	for (pkt = 0; pkt < nPkts; pkt++)
	{
		if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_8021Q, offset, 0))
			return -1;

	
		txData[18] = VLAN_UNTAG_TESTTYPE;	//vlan_untag test type
		
		//even pkt: SW don't tag, odd pkt: SW tag.
		txData[19] = (path&0xf) | ((pkt&0x1)<<4);


		if (!stopTesting)
		{
			totalTxPkts++;
			bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
			PTM_DBG(DBG_L1, "%s: path:%d, pkt:%d\n", 
			__FUNCTION__, path, pkt);

			loopback_pkt_transmit(skb, bearer);
		}
		else
		{
			printk("\nStop Testing (vlanUnTag):\n"
					"path:%d, pkt:%d, totalTxPkts:%d\n",
					path, pkt, totalTxPkts);
			return -1;
		}
	}
	}


	if (loop)
	{
		printk("\nVlan_UnTag TX Loop %d OK\n", loop);
		loop++;
		goto vlan_untag_loop;
	}

	printk("\nVlan_UnTag TX OK with %d pkts!\n", totalTxPkts);

	/* set it back to default value */
	reg = fe_reg_read(CDM_VLAN_GE_OFF);
	reg &= ~(0x2);
	fe_reg_write(CDM_VLAN_GE_OFF, reg);
	
	return 0;
}

/* HW double tag, SW checkk if the double tagging is correct. */
static int ptm_loopback_vlanDblTag_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int pkt, path, bearer;
	int offset = 22;
	uint32 reg = 0;
	

	printk("\nStart %s test\n", __FUNCTION__);


	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;


	/* disable "Extract outer VLAN tag" for SW to un-tag,
	 * and then set CDMQ_TPID for user-defined VLAN tag */
	reg = fe_reg_read(CDM_VLAN_GE_OFF);
	reg &= ~((0x2) | (0xffff << 16));
	reg |= (ETHER_TYPE_QINQ << 16);
	fe_reg_write(CDM_VLAN_GE_OFF, reg);
	

vlan_dbl_tag_loop:

	for (path = startPath; path < endPath; path++)
	{
	for (pkt = 0; pkt < nPkts; pkt++)
	{
		if (txCommonContent_fill(&skb, &txData, pkt, 1, ETHER_TYPE_8021AD, offset, 0))
			return -1;

		//vlan_double_tag test type
		txData[18] = VLAN_DBL_TAG_TESTTYPE;
		
		/* pkt0,2: HW don't tag, 
		 * pkt1: HW tag 0x88a8, 
		 * pkt3: HW tag 0x9100.*/
		txData[19] = (path&0xf) | ((pkt&0x3)<<4);

		if ((((txData[19]>>4)&0x3) == 1) || (((txData[19]>>4)&0x3) == 3))
			*((uint16*)(&txData[16])) += 8; //Rx will receive 4 more bytes for vlanTag & add 4 for prevous minus 4.
		

		if (!stopTesting)
		{
			totalTxPkts++;
			bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
			PTM_DBG(DBG_L1, "%s: path:%d, pkt:%d\n", 
			__FUNCTION__, path, pkt);

			loopback_pkt_transmit(skb, bearer);
		}
		else
		{
			printk("\nStop Testing (vlanDblTag):\n"
					"path:%d, pkt:%d, totalTxPkts:%d\n",
					path, pkt, totalTxPkts);
			return -1;
		}
	}
	}


	if (loop)
	{
		printk("\nVlan_Dbl_Tag TX Loop %d OK\n", loop);
		loop++;
		goto vlan_dbl_tag_loop;
	}

	printk("\nVlan_Dbl_Tag TX OK with %d pkts!\n", totalTxPkts);

	//set it back to default value
	fe_reg_write(CDM_VLAN_GE_OFF, 0);
	
	return 0;
}

/* SW double tag, HW only un-tag the outer tag, which is
 * 8021AD or QinQ, for SW to analysize. */
static int ptm_loopback_vlanDblUnTag_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int pkt, path, bearer;
	int offset = 22;
	uint32 reg = 0;
	

	printk("\nStart %s test\n", __FUNCTION__);


	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;



	/* enable "Extract outer VLAN tag" for HW to un-tag */
	reg = fe_reg_read(CDM_VLAN_GE_OFF);
	reg |= 0x2;
	fe_reg_write(CDM_VLAN_GE_OFF, reg);

	/* Extented VLAN TPID for 0x9100 for RX */
	reg = fe_reg_read(FE_GLO_CFG_OFF);
	reg &= ~(0xffff << 16);
	reg |= (ETHER_TYPE_QINQ << 16);
	fe_reg_write(FE_GLO_CFG_OFF, reg);
	

vlan_dbl_untag_loop:

	for (path = startPath; path < endPath; path++)
	{
	for (pkt = 0; pkt < nPkts; pkt++)
	{
		if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_QINQ, offset, 0))
			return -1;

		//vlan_bdl_untag test type
		txData[18] = VLAN_DBL_UNTAG_TESTTYPE;

		/* pkt0,2: SW don't tag, 
		 * pkt1: SW tags 8021AD and 8021Q, 
		 * pkt3: SW tags QinQ and 8021Q.*/
		txData[19] = (path&0xf) | ((pkt&0x3)<<4);

		if ((((txData[19]>>4)&0x3) == 1) || (((txData[19]>>4)&0x3) == 3))
			*((uint16*)(&txData[16])) += 4; //add 4 for prevous minus 8.


		if (!stopTesting)
		{
			totalTxPkts++;
			bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
			PTM_DBG(DBG_L1, "%s: path:%d, pkt:%d\n", 
			__FUNCTION__, path, pkt);

			loopback_pkt_transmit(skb, bearer);
		}
		else
		{
			printk("\nStop Testing (vlanDblUnTag):\n"
					"path:%d, pkt:%d, totalTxPkts:%d\n",
					path, pkt, totalTxPkts);
			return -1;
		}
	}
	}


	if (loop)
	{
		printk("\nVlanDblUnTag TX Loop %d OK\n", loop);
		loop++;
		goto vlan_dbl_untag_loop;
	}

	printk("\nVlanDblUnTag TX OK with %d pkts!\n", totalTxPkts);


	/* set it back to default values */
	fe_reg_write(CDM_VLAN_GE_OFF, 0);
	reg = fe_reg_read(FE_GLO_CFG_OFF);
	reg &= ~(0xffff << 16);
	reg |= (0x8100 << 16);
	fe_reg_write(FE_GLO_CFG_OFF, reg);
	
	return 0;
}

/* HW insert IP chksum, SW check if the IP chksum is correct */
static int ptm_loopback_ipCsInsert_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int pkt, path, bearer;
	int offset = 26;
	

	printk("\nStart %s test\n", __FUNCTION__);


	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;


ipcs_insert_loop:

	for (path = startPath; path < endPath; path++)
	{
	for (pkt = 0; pkt < nPkts; pkt++)
	{
		if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_IP, offset, 0))
			return -1;

		//HW needs correct ip version & header length to do ip chksum
		txData[14] = (4 << 4) | (5);

		txData[18] = IPCS_INSERT_TESTTYPE;	//ipcs_insert test type
		
		//even pkt: HW don't ipcs insert, odd pkt: HW ipcs insert.
		txData[19] = (path&0xf) | ((pkt&0x1)<<4);

		txData[20] = 0x12;
		txData[21] = 0x34;
		txData[22] = 0x56;
		txData[23] = 0x78;
		//Byte24~25 are left for HW to insert ip chksum, needed to be reset before calculating and inserting.
		txData[24] = 0;
		txData[25] = 0;


		if (!stopTesting)
		{
			totalTxPkts++;
			bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
			PTM_DBG(DBG_L1, "%s: path:%d, pkt:%d\n", 
			__FUNCTION__, path, pkt);

			loopback_pkt_transmit(skb, bearer);
		}
		else
		{
			printk("\nStop Testing (ipCsInsert):\n"
					"path:%d, pkt:%d, totalTxPkts:%d\n",
					path, pkt, totalTxPkts);
			return -1;
		}
	}
	}


	if (loop)
	{
		printk("\nipCsInsert TX Loop %d OK\n", loop);
		loop++;
		goto ipcs_insert_loop;
	}

	printk("\nipCsInsert TX OK with %d pkts!\n", totalTxPkts);
	return 0;
}

/* SW insert IP chksum, HW indicate if IP chksum is correct */
static int ptm_loopback_ipCsCheck_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int pkt, path, bearer;
	int offset = 26;
	uint32 reg;
	

	printk("\nStart %s test\n", __FUNCTION__);


	/* don't drop packets with ipv4 checksum error 
	 * but indicate those packets in RX message */
	reg = fe_reg_read(GDM2_FWD_CFG_OFF);
	reg &= ~(1<<22);
	fe_reg_write(GDM2_FWD_CFG_OFF, reg);


	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;


ipcs_check_loop:

	for (path = startPath; path < endPath; path++)
	{
	for (pkt = 0; pkt < nPkts; pkt++)
	{
		if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_IP, offset, 0))
			return -1;

		//HW needs connect ip version & header length to do ip chksum
		txData[14] = (4 << 4) | (5);
	
		txData[18] = IPCS_CHECK_TESTTYPE;	//ipcs_check test type
		
		txData[19] = (path&0xf) | ((pkt&0x1)<<4);

		txData[20] = 0xab;
		txData[21] = 0x34;
		txData[22] = 0x56;
		txData[23] = 0x78;
		//Byte24~25 are left for ip chksum calculated by SW, needed to be reset before calculating.
		txData[24] = 0;
		txData[25] = 0;

		//even pkt: SW insert wrong ip chksum, odd pkt: SW insert correct ip chksum.
		if ((txData[19]>>4)&0x1)
			*((uint16*)(&txData[24])) = in_csum((unsigned short *) &txData[14], 20);
		else
			*((uint16*)(&txData[24])) = (1 + in_csum((unsigned short *) &txData[14], 20));


		if (!stopTesting)
		{
			totalTxPkts++;
			bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
			PTM_DBG(DBG_L1, "%s: path:%d, pkt:%d\n", 
			__FUNCTION__, path, pkt);

			loopback_pkt_transmit(skb, bearer);
		}
		else
		{
			printk("\nStop Testing (ipCsCheck):\n"
					"path:%d, pkt:%d, totalTxPkts:%d\n",
					path, pkt, totalTxPkts);
			return -1;
		}
	}
	}


	if (loop)
	{
		printk("\nipCsCheck TX Loop %d OK\n", loop);
		loop++;
		goto ipcs_check_loop;
	}

	printk("\nipCsCheck TX OK with %d pkts!\n", totalTxPkts);

	
	return 0;
}

/* case 0~11 will be tested by ptm_loopback_bacpCheck,
 * case 88 will be tested by ptm_loopbackBacpRegSet */
static int bacpStageChange(void)
{
	switch(bacpCheckStage)
	{
		case 0:
			PTM_DBG(DBG_L1, "bacp test for default\n");
			bacpHeader_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000, 0x3f);
			break;
		case 1:
			PTM_DBG(DBG_L1, "bacp test for Dst MAC case 1\n");
			bacpHeader_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000, 0x1);
			break;
		case 2:
			PTM_DBG(DBG_L1, "bacp test for Dst MAC case 2\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000);
			break;
		case 3:
			PTM_DBG(DBG_L1, "bacp test for Org SubType case 1\n");
			bacpHeader_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000, 0x4);
			break;
		case 4:
			PTM_DBG(DBG_L1, "bacp test for Org SubType case 2\n");
			bacpHeader_write(0x0180c200, 0x00028809, 0x0b0019a7, 0x01010000);
			break;
		case 5:
			PTM_DBG(DBG_L1, "bacp test for ITU OUI case 1\n");
			bacpHeader_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000, 0x8);
			break;
		case 6:
			PTM_DBG(DBG_L1, "bacp test for ITU OUI case 2\n");
			bacpHeader_write(0x0180c200, 0x00028809, 0x0a0019a8, 0x01010000);
			break;
		case 7:
			PTM_DBG(DBG_L1, "bacp test for ITU SubType case 1\n");
			bacpHeader_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000, 0x10);
			break;
		case 8:
			PTM_DBG(DBG_L1, "bacp test for ITU SubType case 2\n");
			bacpHeader_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x02010000);
			break;
		case 9:
			PTM_DBG(DBG_L1, "bacp test for BACP version case 1\n");
			bacpHeader_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000, 0x20);
			break;
		case 10:
			PTM_DBG(DBG_L1, "bacp test for BACP version case 2\n");
			bacpHeader_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01020000);
			break;
		case 11:
			PTM_DBG(DBG_L1, "for Dst MAC & BACP version case 1\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000, 0x21);
			break;
		case 12:
			PTM_DBG(DBG_L1, "for Dst MAC & BACP version case 2\n");
			bacpHeader_write(0x0180c201, 0x10028809, 0x0a0019a7, 0x01110000);
			break;
		case 13:
			PTM_DBG(DBG_L1, "for Dst MAC & ITU SubType case 1\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000, 0x11);
			break;
		case 14:
			PTM_DBG(DBG_L1, "for Dst MAC & ITU SubType case 2\n");
			bacpHeader_write(0x0180c201, 0x00038809, 0x0a0019a7, 0x11010000);
			break;
		case 15:
			PTM_DBG(DBG_L1, "for Dst MAC & ITU OUI case 1\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000, 0x9);
			break;
		case 16:
			PTM_DBG(DBG_L1, "for Dst MAC & ITU OUI case 2\n");
			bacpHeader_write(0x0180c201, 0x10028809, 0x0a0019b7, 0x01010000);
			break;
		case 17:
			PTM_DBG(DBG_L1, "for Dst MAC &  Org SubType case 1\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000, 0x5);
			break;
		case 18:
			PTM_DBG(DBG_L1, "for Dst MAC &  Org SubType case 2\n");
			bacpHeader_write(0x0180d201, 0x10028809, 0x1a0019a7, 0x01010000);
			break;
		case 19:
			PTM_DBG(DBG_L1, "for Dst MAC &  Org SubType & ITU OUI case 1\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000, 0xd);
			break;
		case 20:
			PTM_DBG(DBG_L1, "for Dst MAC &  Org SubType & ITU OUI case 2\n");
			bacpHeader_write(0x0190c301, 0x05228809, 0x3c1119a7, 0x01010000);
			break;
		case 21:
			PTM_DBG(DBG_L1, "for Org SubType & ITU OUI & ITU subType case 1\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000, 0x1c);
			break;
		case 22:
			PTM_DBG(DBG_L1, "for Org SubType & ITU OUI & ITU subType case 2\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x11aa1aa7, 0xaa010000);
			break;
		case 23:
			PTM_DBG(DBG_L1, "for ITU OUI & ITU subType & BACP version case 1\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000, 0x38);
			break;
		case 24:
			PTM_DBG(DBG_L1, "for ITU OUI & ITU subType & BACP version case 2\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x0a441177, 0x98760000);
			break;
		case 25:
			PTM_DBG(DBG_L1, "for Org SubType & ITU OUI & ITU subType & BACP version case 1\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000, 0x3c);
			break;
		case 26:
			PTM_DBG(DBG_L1, "for Org SubType & ITU OUI & ITU subType & BACP version case 2\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x33333333, 0x33330000);
			break;
		case 27:
			PTM_DBG(DBG_L1, "for Dst MAC & ITU OUI & ITU subType & BACP version case 1\n");
			bacpHeader_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000);
			bacpReg_write(0x0180c201, 0x00028809, 0x0a0019a7, 0x01010000, 0x39);
			break;
		case 28:
			PTM_DBG(DBG_L1, "for Dst MAC & ITU OUI & ITU subType & BACP version case 2\n");
			bacpHeader_write(0x05555555, 0x55558809, 0x0a555555, 0x55550000);
			break;
		case BACP_CHECK_STAGE:
			PTM_DBG(DBG_L1, "bacp test for default case 2\n");
			bacpHeader_write(0x01eeeeee, 0xeeee8809, 0xeeeeeeee, 0xeeee0000);
			bacpReg_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000, 0x3f);
			break;			
		case 88:
			PTM_DBG(DBG_L1, "bacp test by CI command\n");
			break;
		default:
			printk("\nunknown bacp test case!\n");
			return -1;
	}

	bacpHeader_read();
	bacpReg_read();

	return 0;
}

/* For case 0~11 in bacpStageChange(), 
 * SW insert BACP header and set registers, 
 * HW indicate if the packet is for BACP */
static int ptm_loopback_bacpCheck_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int pkt, path, bearer;
	int offset = 20;
	

	printk("\nStart %s test\n", __FUNCTION__);


	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;

	bacpCheckStage = 0;

bacp_check_loop:

	if (bacpStageChange())
		return -1;
	
	for (path = startPath; path < endPath; path++)
	{
	for (pkt = 0; pkt < nPkts; pkt++)
	{
		if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_SLOW, offset, 0))
			return -1;

	
		txData[18] = BACP_PKT_TESTTYPE;	//bacp_check test type
		
		//even pkt: data pkt, odd pkt: bacp pkt.(fill bacp header in txCommonContent_fill)
		txData[19] = (path&0xf) | ((pkt&0x1)<<4);

		if ((txData[19]>>4)&0x1)
			*((uint16*)(&txData[16])) += 10; //for previous minus 10.

		if (!stopTesting)
		{
			totalTxPkts++;
			bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
			//PTM_DBG(DBG_L1, "%s: path:%d, pkt:%d\n", __FUNCTION__, path, pkt);

			loopback_pkt_transmit(skb, bearer);
		}
		else
		{
			printk("\nStop Testing (bacpCheck):\n"
					"path:%d, pkt:%d, totalTxPkts:%d\n",
					path, pkt, totalTxPkts);
			return -1;
		}
	}
	}

	if (0 <= bacpCheckStage && bacpCheckStage < BACP_CHECK_STAGE)
	{
		bacpCheckStage++;
		goto bacp_check_loop;
	}

	if (loop)
	{
		printk("\nbacpCheck TX Loop %d OK\n", loop);
		loop++;
		bacpCheckStage = 0;
		goto bacp_check_loop;
	}

	printk("\nbacpCheck TX OK with %d pkts!\n", totalTxPkts);

	//set back to default values
	bacpHeader_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000);
	bacpReg_write(0x0180c200, 0x00028809, 0x0a0019a7, 0x01010000, 0x3f);
	bacpCheckStage = 0;
	
	return 0;
}

/* SW send runt packets, HW indicate if the packet is runt */
static int ptm_loopback_runtCheck_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int pkt, path, bearer;
	uint32 txLen;
	int offset = 20;
	uint32 reg;
	

	printk("\nStart %s test\n", __FUNCTION__);


	/* don't drop packets with len < 60 bytes (not including CRC) 
	 * but indicate those packets in RX message */
	reg = fe_reg_read(GDM2_FWD_CFG_OFF);
	reg &= ~(1<<24);
	fe_reg_write(GDM2_FWD_CFG_OFF, reg);


	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;


runt_check_loop:

	for (path = startPath; path < endPath; path++)
	{
	for (pkt = 0; pkt < nPkts; pkt++)
	{
		if (pkt & 1)
		{
			//txLen: 21~59 bytes for Runt packets
			txLen = random32();
		
			if ((txLen < 21) || (txLen > 59))
				txLen %= 39;
			txLen += 21;
		}
		else
			txLen = 0;
	
		if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_IP, offset, txLen))
			return -1;

	
		txData[18] = RUNT_PKT_TESTTYPE;	//runt_check test type
		
		txData[19] = (path&0xf) | ((pkt&0x1)<<4); //even pkt: normal pkt, odd pkt: runt pkt.


		if (!stopTesting)
		{
			totalTxPkts++;
			bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
			PTM_DBG(DBG_L1, "%s: path:%d, pkt:%d, txLen:%d\n", 
			__FUNCTION__, path, pkt, (int)*((uint16*)(&txData[16])));

			loopback_pkt_transmit(skb, bearer);
		}
		else
		{
			printk("\nStop Testing (runtCheck):\n"
					"path:%d, pkt:%d, totalTxPkts:%d\n",
					path, pkt, totalTxPkts);
			return -1;
		}
	}
	}


	if (loop)
	{
		printk("\nruntCheck TX Loop %d OK\n", loop);
		loop++;
		goto runt_check_loop;
	}

	printk("\nruntCheck TX OK with %d pkts!\n", totalTxPkts);

	
	return 0;
}

/* SW send long packets, HW indicate if the packet is long */
static int ptm_loopback_longCheck_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int pkt, path, bearer;
	uint32 txLen;
	int offset = 20;
	uint32 reg;
	

	printk("\nStart %s test\n", __FUNCTION__);


	/* don't drop long packets 
	 * but indicate those packets in RX message */
	reg = fe_reg_read(GDM2_FWD_CFG_OFF);
	reg &= ~(1<<25);
	fe_reg_write(GDM2_FWD_CFG_OFF, reg);
	/* configure packets with len > 1514 bytes 
	 * (not including 4-byte CRC) as long packets */
	reg = fe_reg_read(GDM2_LEN_CFG_OFF);
	reg &= ~(0xffff0000);
	reg |= 0x5ea << 16;
	fe_reg_write(GDM2_LEN_CFG_OFF, reg);

	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;


long_check_loop:

	for (path = startPath; path < endPath; path++)
	{
	for (pkt = 0; pkt < nPkts; pkt++)
	{
		if (pkt & 1)
		{
			//txLen: 1515~MAX_PKT_SZIE bytes for Long packets
			txLen = random32();
		
			if ((txLen < 1515) || (txLen > MAX_PKT_SIZE))
			{
				txLen %= (MAX_PKT_SIZE - 1515 + 1);
				txLen += 1515;
			}
		}
		else
			txLen = 0;
	
		if (txCommonContent_fill(&skb, &txData, pkt, 1, ETHER_TYPE_IP, offset, txLen))
			return -1;

	
		txData[18] = LONG_PKT_TESTTYPE;	//long_check test type
		
		txData[19] = (path&0xf) | ((pkt&0x1)<<4); //even pkt: normal pkt, odd pkt: long pkt.

		if (!stopTesting)
		{
			totalTxPkts++;
			bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
			PTM_DBG(DBG_L1, "%s: path:%d, len:%d\n", 
			__FUNCTION__, path, *((uint16*)(&txData[16])));

			loopback_pkt_transmit(skb, bearer);
		}
		else
		{
			printk("\nStop Testing (longCheck):\n"
					"path:%d, pkt:%d, totalTxPkts:%d\n",
					path, pkt, totalTxPkts);
			return -1;
		}
	}
	}


	if (loop)
	{
		printk("\nlongCheck TX Loop %d OK\n", loop);
		loop++;
		goto long_check_loop;
	}

	printk("\nlongCheck TX OK with %d pkts!\n", totalTxPkts);

	//set it back to default values
	reg = fe_reg_read(GDM2_LEN_CFG_OFF);
	reg &= ~(0xffff0000);
	reg |= 0x7d0 << 16;
	fe_reg_write(GDM2_LEN_CFG_OFF, reg);

	return 0;
}

/* HW generate CRC packets and then indacate if the
 * packet is CRC-error. 
 * Please note: during this test, MAC Rx crcErr counters
 * should count! */
static int ptm_loopback_crcCheck_tx(uint32 pktNum)
{
	int nPkts, loop = 0;
	struct sk_buff *skb = NULL;
	uint8 *txData = NULL;
	int pkt, path, bearer;
	int offset = 20;
	uint32 reg;
	

	printk("\nStart %s test\n", __FUNCTION__);
	isCrcCheckTest = 1;

	if (pktNum == 0)
	{
		nPkts = 1500;
		if (!isAllTestsOnGoing)
			loop = 1;
	}
	else
		nPkts = pktNum;

	reg = read_reg_word(PTM_CTRL_REG);

crc_check_loop:

	for (path = startPath; path < endPath; path++)
	{
	for (pkt = 0; pkt < nPkts; pkt++)
	{
		if (txCommonContent_fill(&skb, &txData, pkt, RAN_PKTTYPE, ETHER_TYPE_IP, offset, 0))
			return -1;

	
		txData[18] = CRCERR_PKT_TESTTYPE;	//crc_check test type
		
		txData[19] = (path&0xf) | ((pkt&0x1)<<4); //even pkt: normal pkt, odd pkt: crc pkt.
		
		if (pkt &0x1)
		{
	 		/* enable "HW wrong CRC generation" */
			reg |= (0x1<<18);
			write_reg_word(PTM_CTRL_REG, reg);
		}
		else
		{
			/* disable "HW wrong CRC generation" */
			reg &= ~(0x1<<18);
			write_reg_word(PTM_CTRL_REG, reg);			
		}


		if (!stopTesting)
		{
			totalTxPkts++;
			bearer = (path >> PATH_BEARER_SHIFT) & 0x1;
			PTM_DBG(DBG_L1, "%s: path:%d, pkt:%d\n", 
					__FUNCTION__, path, pkt);

			loopback_pkt_transmit(skb, bearer);
		}
		else
		{
			printk("\nStop Testing (crcCheck):\n"
					"path:%d, pkt:%d, totalTxPkts:%d\n",
					path, pkt, totalTxPkts);
			return -1;
		}
	}
	}


	if (loop)
	{
		printk("\ncrcCheck TX Loop %d OK\n", loop);
		loop++;
		goto crc_check_loop;
	}

	printk("\ncrcCheck TX OK with %d pkts!\n", totalTxPkts);

	//set it back to defualt value
	reg = read_reg_word(PTM_CTRL_REG);
	reg &= ~(0x1<<18);
	write_reg_word(PTM_CTRL_REG, reg);

	return 0;
}


static int ptm_loopback_allTests_tx(uint32 pktNum)
{
	int nPackets, loop = 0;
	

	printk("\nStart %s test\n", __FUNCTION__);
	isAllTestsOnGoing = 1;

	if (pktNum == 0)
	{
		nPackets = 1500;
		loop = 1;
	}
	else
		nPackets = pktNum;


all_tests_loop:

	if (ptm_loopback_dataPath_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);

	if (ptm_loopback_pathNo_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);

#if !(defined(TCSUPPORT_BONDING) && defined(FPGA_STAGE))
/* Boding's clock in FPGA is slow, so burst mode
 * based tests can't work, so don't care them! */
 
	if (ptm_loopback_preemption_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_spPriority_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_wrrPriority_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_spWrr_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
#endif

	if (ptm_loopback_vlanTag_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_vlanUnTag_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_vlanDblTag_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_vlanDblUnTag_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_ipCsInsert_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_ipCsCheck_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_bacpCheck_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_runtCheck_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_longCheck_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
	
	if (ptm_loopback_crcCheck_tx(nPackets))
		goto allTests_fail;
	else
	{
		msleep(TEST_SLEEP_TIME);
		isCrcCheckTest = 0;
	}

#if !(defined(TCSUPPORT_BONDING) && defined(FPGA_STAGE))
/* Boding's clock in FPGA is slow, so burst mode
 * based tests can't work, so don't care them! */

	if (ptm_loopback_dataBurst_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);

	if (ptm_loopback_multiChannel_tx(nPackets))
		goto allTests_fail;
	else
		msleep(TEST_SLEEP_TIME);
#endif

	if (loop)
	{
		printk("\nallTests TX Loop %d OK\n", loop);
		loop++;
		goto all_tests_loop;
	}

	printk("\nallTests TX OK\n");
	return 0;

allTests_fail:
	return -1;
}

/* "echo N M > /proc/tc3162/ptm_loopback_test &" to 
 * start loopback test.
 * Note: N is test case, M is number of test packets */
int ptm_loopbackTest_write_proc(
		struct file *file, const char *buffer, 
		unsigned long count, void *data
)
{
	char valString[10];
	uint32 reg;


	if (count > sizeof(valString) - 1)
		return -EINVAL;

	if (copy_from_user(valString, buffer, count))
		return -EFAULT;


	valString[count] = '\0';
	
	if(sscanf(valString, "%d %d", &testType, &nPackets) != 2)
		return count;

	//init variables
	stopTesting = 0;
	dataPathRxPkts = 0;
	totalTxPkts = 0;
	isAllTestsOnGoing = 0;
	isCrcCheckTest = 0;

	/* don't do error drop for udp, tcp, ipv4 checksum
	 * for loopback because loopback packet content 
	 * isn't well designed */
	reg = fe_reg_read(GDM2_FWD_CFG_OFF);
	reg &= ~((1<<20) | (1<<21) | (1<<22));
	fe_reg_write(GDM2_FWD_CFG_OFF, reg);

	switch (testType)
	{
		case REGISTER_TESTTYPE:
			ptm_loopback_registersCheck(nPackets);
			break;
		case SINGLE_PKT_TESTTYPE:
			ptm_loopback_singlePkt_tx(nPackets);
			break;
		case DATA_PATH_TESTTYPE:
			ptm_loopback_dataPath_tx(nPackets);
			break;
		case DATA_BURST_TESTTYPE:
			ptm_loopback_dataBurst_tx(nPackets);
			break;
		case MULTI_CHANNEL_TESTTYPE:
			ptm_loopback_multiChannel_tx(nPackets);
			break;
		case PATH_NO_TESTTYPE:
			ptm_loopback_pathNo_tx(nPackets);
			break;
		case PREEMPTION_TESTTYPE:
			ptm_loopback_preemption_tx(nPackets);
			break;
		case SP_PRIORITY_TESTTYPE:
			ptm_loopback_spPriority_tx(nPackets);
			break;
		case WRR_PRIORITY_TESTTYPE:
			ptm_loopback_wrrPriority_tx(nPackets);
			break;
		case SP_WRR_TESTTYPE:
			ptm_loopback_spWrr_tx(nPackets);
			break;
		case VLAN_TAG_TESTTYPE:
			ptm_loopback_vlanTag_tx(nPackets);
			break;
		case VLAN_UNTAG_TESTTYPE:
			ptm_loopback_vlanUnTag_tx(nPackets);
			break;
		case VLAN_DBL_TAG_TESTTYPE:
			ptm_loopback_vlanDblTag_tx(nPackets);
			break;
		case VLAN_DBL_UNTAG_TESTTYPE:
			ptm_loopback_vlanDblUnTag_tx(nPackets);
			break;
		case IPCS_INSERT_TESTTYPE:
			ptm_loopback_ipCsInsert_tx(nPackets);
			break;
		case IPCS_CHECK_TESTTYPE:
			ptm_loopback_ipCsCheck_tx(nPackets);
			break;
		case BACP_PKT_TESTTYPE:
			ptm_loopback_bacpCheck_tx(nPackets);
			break;
		case RUNT_PKT_TESTTYPE:
			ptm_loopback_runtCheck_tx(nPackets);
			break;
		case LONG_PKT_TESTTYPE:
			ptm_loopback_longCheck_tx(nPackets);
			break;
		case CRCERR_PKT_TESTTYPE:
			ptm_loopback_crcCheck_tx(nPackets);
			break;
		case ALL_LOOPBACK_TESTTYPE:
			ptm_loopback_allTests_tx(nPackets);
			break;
			
		default:
			printk("\nWrong loopback test type: %d\n", testType);
			break;
	}

	
	return count;
}

/* "cat /proc/tc3162/ptm_loopback_test &" to 
 * see what loopback tests do we have. */
int ptm_loopbackTest_read_proc(
		char *buf, char **start, off_t off, 
		int count, int *eof, void *data
)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;


	index += sprintf(buf+index, "\nTest Type IDs:\n");
	CHK_BUF();
	index += sprintf(buf+index, "REGISTER_TESTTYPE:       %d\n", 
				REGISTER_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "DATA_PATH_TESTTYPE:      %d\n",
				DATA_PATH_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "PATH_NO_TESTTYPE:        %d\n",
				PATH_NO_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "PREEMPTION_TESTTYPE:     %d\n",
				PREEMPTION_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "SP_PRIORITY_TESTTYPE:    %d\n",
				SP_PRIORITY_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "WRR_PRIORITY_TESTTYPE:   %d\n",
				WRR_PRIORITY_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "SP_WRR_TESTTYPE:         %d\n",
				SP_WRR_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "VLAN_TAG_TESTTYPE:       %d\n",
				VLAN_TAG_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "VLAN_UNTAG_TESTTYPE:     %d\n",
				VLAN_UNTAG_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "VLAN_DBL_TAG_TESTTYPE:   %d\n",
				VLAN_DBL_TAG_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "VLAN_DBL_UNTAG_TESTTYPE: %d\n",
				VLAN_DBL_UNTAG_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "IPCS_INSERT_TESTTYPE:    %d\n",
				IPCS_INSERT_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "IPCS_CHECK_TESTTYPE:     %d\n",
				IPCS_CHECK_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "BACP_PKT_TESTTYPE:       %d\n",
				BACP_PKT_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "RUNT_PKT_TESTTYPE:       %d\n",
				RUNT_PKT_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "LONG_PKT_TESTTYPE:       %d\n",
				LONG_PKT_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "CRCERR_PKT_TESTTYPE:     %d\n",
				CRCERR_PKT_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "DATA_BURST_TESTTYPE:     %d\n",
				DATA_BURST_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "MULTI_CHANNEL_TESTTYPE:  %d\n",
				MULTI_CHANNEL_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "SINGLE_PKT_TESTTYPE:     %d\n",
				SINGLE_PKT_TESTTYPE);
	CHK_BUF();
	index += sprintf(buf+index, "ALL_LOOPBACK_TESTTYPE:   %d\n\n",
				ALL_LOOPBACK_TESTTYPE);
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

int loopback_proc_init(void)
{	
	struct proc_dir_entry *ptmProc;

	ptmProc = create_proc_entry("tc3162/ptm_loopback_point", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_loopback_point\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_loopbackPoint_write_proc;
	
	ptmProc = create_proc_entry("tc3162/ptm_loopback_test", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_loopback_test\n");
		return -ENOMEM;
	}
	ptmProc->read_proc = ptm_loopbackTest_read_proc;
	ptmProc->write_proc = ptm_loopbackTest_write_proc;

	ptmProc = create_proc_entry("tc3162/ptm_loopback_stopTest", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_loopback_stopTes\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_loopbackStopTest_write_proc;

	ptmProc = create_proc_entry("tc3162/ptm_loopback_setPktSize", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_loopback_setPktSize\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_loopbackSetPktSize_write_proc;

	ptmProc = create_proc_entry("tc3162/ptm_loopback_bacpHdrSet", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_loopback_bacpHdrSet\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_loopbackBacpHdrSet_write_proc;

	ptmProc = create_proc_entry("tc3162/ptm_loopback_bacpRegSet", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_loopback_bacpRegSet\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_loopbackBacpRegSet_write_proc;


	ptmProc = create_proc_entry("tc3162/ptm_loopback_pathRangeSet", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_loopback_pathRangeSet\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_loopbackPathRangeSet_write_proc;

	ptmProc = create_proc_entry("tc3162/ptm_pathIdSet", 0, NULL);
	if (!ptmProc)
	{
		printk("\nFAILED: create proc for ptm_pathIdSet\n");
		return -ENOMEM;
	}
	ptmProc->write_proc = ptm_pathId_write_proc;


	return 0;
}

void loopback_proc_remove(void)
{
	remove_proc_entry("tc3162/ptm_loopback_point", 0);
	remove_proc_entry("tc3162/ptm_loopback_test", 0);
	remove_proc_entry("tc3162/ptm_loopback_stopTest", 0);
	remove_proc_entry("tc3162/ptm_loopback_setPktSize", 0);
	remove_proc_entry("tc3162/ptm_loopback_bacpHdrSet", 0);
	remove_proc_entry("tc3162/ptm_loopback_bacpRegSet", 0);
	remove_proc_entry("tc3162/ptm_loopback_pathRangeSet", 0);
	remove_proc_entry("tc3162/ptm_pathIdSet", 0);

	return;
}


/* check if packet payload is correct */
static int rxContent_check(
		uint8 *txData, int offset, uint16 txLen,
		uint8 baseVal, uint8 type
)
{
	int k;
	int pktType = type & 0xf;

	for (k = offset; k < txLen; k++)
	{
		if (txData[k] != baseVal)
		{
			printk("\nFAILED: (pkt:%d)\n"
					"txData[%d]:%02X != baseVal:%02X\n",
					dataPathRxPkts, k, txData[k], baseVal);
			return -1;
		}

		if (pktType == 0 || pktType == 1)
		{
			if (baseVal == 0xff)
				baseVal = 0;
			else
				baseVal++;
		}
	}

	return 0;
}


static int ptm_loopback_singlePkt_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;

	
	txData = skb->data;
	type = txData[14];
	baseVal = txData[15];
	txLen = *(uint16*) (&txData[16]);

	if (rxContent_check(txData, 20, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	printk("\nsinglePkt RX OK with pktLen:%d\n", (int)txLen);
	return 0;
}


static int ptm_loopback_dataPath_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	int path;
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;


	path = rxMsg->rxMsgW0.bits.pathNo & 0xf;
	txData = skb->data;
	type = txData[14];
	baseVal = txData[15];
	
	txLen = *(uint16*) (&txData[16]);

	if (txData[18] != DATA_PATH_TESTTYPE)
	{
		printk("\nFAILED: %s: (path:%d)\n"
				"txData[18]:%02X != DATA_PATH_TESTTYPE\n",
				__FUNCTION__, path, txData[18]);
		return -1;
	}

	if (rxContent_check(txData, 20, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	if ((dataPathRxPkts & 0xffff) != 0xffff)
	{
		PTM_DBG(DBG_L1, "%s: %d pkts recvd\n\n", __FUNCTION__, dataPathRxPkts);
	}
	else
		printk("%s: %d pkts recvd\n\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}


static int ptm_loopback_dataBurst_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	int path;
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;


	path = rxMsg->rxMsgW0.bits.pathNo & 0xf;
	txData = skb->data;
	type = txData[14];
	baseVal = txData[15];
	
	txLen = *(uint16*) (&txData[16]);

	if (txData[18] != DATA_BURST_TESTTYPE)
	{
		printk("\nFAILED: %s: (path:%d)\n"
				"txData[18]:%02X != DATA_BURST_TESTTYPE\n",
				__FUNCTION__, path, txData[18]);
		return -1;
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19] & 0xf):%d != path:%d\n",
				__FUNCTION__, (int)(txData[19] & 0xf), path);
		return -1;
	}

	if (rxContent_check(txData, 20, txLen, baseVal, type))
		return -1;


	testPktCnt++;
	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n\n", __FUNCTION__, dataPathRxPkts);

	//turn on QDMA when all packets are received. 
	if ((testPktCnt-1) == (TX_QUEUE_NUM * TX_BURST_LEN))
	{
		printk("\ndataBurst test for path%d is finished\n\n", path);
		qdmaDmaAvailable = 1;
	}

	return 0;
}


static int ptm_loopback_multiChannel_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	int path;
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;


	path = rxMsg->rxMsgW0.bits.pathNo & 0xf;
	txData = skb->data;
	type = txData[14];
	baseVal = txData[15];
	
	txLen = *(uint16*) (&txData[16]);

	if (txData[18] != MULTI_CHANNEL_TESTTYPE)
	{
		printk("\nFAILED: %s: (path:%d)\n"
				"txData[18]:%02X != MULTI_CHANNEL_TESTTYPE\n",
				__FUNCTION__, path, txData[18]);
		return -1;
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19] & 0xf):%d != path:%d\n",
				__FUNCTION__, (int)(txData[19] & 0xf), path);
		return -1;
	}

	if (rxContent_check(txData, 20, txLen, baseVal, type))
		return -1;


	testPktCnt++;
	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n\n", __FUNCTION__, dataPathRxPkts);

	if ((testPktCnt-1) == 2)
		qdmaDmaAvailable = 1;

	return 0;
}


static int ptm_loopback_pathNo_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	int path;
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;


	path = rxMsg->rxMsgW0.bits.pathNo;
	txData = skb->data;
	type = txData[14];
	baseVal = txData[15];
	txLen = *(uint16*) (&txData[16]);

	if (txData[18] != PATH_NO_TESTTYPE)
	{
		printk("\nFAILED: %s: (path:%d)\n"
				"txData[18]:%02X != PATH_NO_TESTTYPE\n",
				__FUNCTION__, path, txData[18]);
		return -1;
	}

	if (path != (txData[19] & 0xf))
	{
		printk("\nFAILED: %s:\n"
				"path:%d != (txData[19] & 0xf):%d\n",
				__FUNCTION__, path, (int)(txData[19] & 0xf));
		return -1;
	}

	if (rxContent_check(txData, 20, txLen, baseVal, type))
		return -1;

		
	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}

/* check if all preemption packets are received
 * before all normal packets are comming */
static int ptm_loopback_preemption_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	int path;
	int preemption;
	uint8 *txData;
	uint8 baseVal;
	uint16 txLen;


	path = rxMsg->rxMsgW0.bits.pathNo;
	preemption = (path & 0x1);
	txData = skb->data;
	baseVal = txData[15];
	txLen = *(uint16*) (&txData[16]);

	if (txData[18] != PREEMPTION_TESTTYPE)
	{
		printk("\nFAILED: %s: (path:%d)\n"
				"txData[18]:%02X != PREEMPTION_TESTTYPE\n",
				__FUNCTION__, path, txData[18]);
		return -1;
	}

	if (path != (txData[19] & 0xf))
	{
		printk("\nFAILED: %s:\n"
				" path:%d != (txData[19] & 0xf):%d\n",
				__FUNCTION__, path, (int)(txData[19] & 0xf));
		return -1;
	}

	if (rxContent_check(txData, 20, txLen, baseVal, 1))
		return -1;

	if (preemption)
	{
		rxPreemptionPkts++;
		PTM_DBG(DBG_L1, "\nPreemption pkt:%d recevied\n",
						rxPreemptionPkts);
	}
	else
	{
		dataPathRxPkts++; //normal packets
		PTM_DBG(DBG_L1, "\nNormal pkt:%d recevied\n",
						dataPathRxPkts);
	}

	if (rxPreemptionPkts == 4)
	{
		rxPreemptionPkts++; //prevent to come here again.
	
		if (dataPathRxPkts >= 60)
		{
			printk("\nFailed: Preemption test\n");
			return -1;
		}
	}

	if ((rxPreemptionPkts + dataPathRxPkts) == 65)
	{
		PTM_DBG(DBG_L1, "\nPreemption Rx OK\n");
		qdmaDmaAvailable = 1;
	}

	return 0;
}

/* check if packet comming sequence is 
 * (TX_BURST_LEN packets for txq7) ->
 * (TX_BURST_LEN packets for txq6) ->
 *  .............................
 * (TX_BURST_LEN packets for txq0)  */
static int ptm_loopback_spPriority_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	int path, txq;
	uint8 *txData;
	uint8 baseVal;
	uint16 txLen;


	path = rxMsg->rxMsgW0.bits.pathNo;
	txData = skb->data;
	baseVal = txData[15];
	txLen = *(uint16*) (&txData[16]);
	txq = ((txData[19] >> 4) & 0xf);

	if (txData[18] != SP_PRIORITY_TESTTYPE)
	{
		printk("\nFAILED: %s: (path:%d)\n"
				"txData[18]:%02X != SP_PRIORITY_TESTTYPE\n",
				__FUNCTION__, path, txData[18]);
		return -1;
	}

	if (path != (txData[19] & 0xf))
	{
		printk("\nFAILED: %s:\n"
				" path:%d != (txData[19] & 0xf):%d\n",
				__FUNCTION__, path, (int)(txData[19] & 0xf));
		return -1;
	}


	/* check if rx packets' priority is p7>p6>...>p0 */

	//txq7 pkts expected
	if (testPktCnt <= (TX_BURST_LEN * 1))
	{
		if (txq != 7)
		{
			printk("\nFAILED: pkt:%d should be txq7 pkt, not txq%d pkt\n", testPktCnt, txq);
			return -1;
		}
		
		if (testPktCnt == (TX_BURST_LEN * 1))
			curTxq -= 1;
	}
	//txq6 pkts expected
	else if (((TX_BURST_LEN * 1) < testPktCnt) &&
		(testPktCnt <= (TX_BURST_LEN << 1)))
	{
		if (txq != 6)
		{
			printk("\nFAILED: pkt:%d should be txq6 pkt, not txq%d pkt\n", testPktCnt, txq);
			return -1;
		}

		if (testPktCnt == (TX_BURST_LEN << 1))
			curTxq -= 1;
	}
	//txq5 pkts expected
	else if (((TX_BURST_LEN << 1) < testPktCnt) &&
		(testPktCnt <= (TX_BURST_LEN * 3)))
	{
		if (txq != 5)
		{
			printk("\nFAILED: pkt:%d should be txq5 pkt, not txq%d pkt\n", testPktCnt, txq);
			return -1;
		}

		if (testPktCnt == (TX_BURST_LEN * 3))
			curTxq -= 1;
	}
	//txq4 pkts expected
	else if (((TX_BURST_LEN * 3) < testPktCnt) &&
		(testPktCnt <= (TX_BURST_LEN << 2)))
	{
		if (txq != 4)
		{
			printk("\nFAILED: pkt:%d should be txq4 pkt, not txq%d pkt\n", testPktCnt, txq);
			return -1;
		}

		if (testPktCnt == (TX_BURST_LEN << 2))
			curTxq -= 1;
	}
	//txq3 pkts expected
	else if (((TX_BURST_LEN << 2) < testPktCnt) &&
		(testPktCnt <= (TX_BURST_LEN * 5)))
	{
		if (txq != 3)
		{
			printk("\nFAILED: pkt:%d should be txq3 pkt, not txq%d pkt\n", testPktCnt, txq);
			return -1;
		}

		if (testPktCnt == (TX_BURST_LEN * 5))
			curTxq -= 1;
	}
	//txq2 pkts expected
	else if (((TX_BURST_LEN * 5) < testPktCnt) &&
		(testPktCnt <= (TX_BURST_LEN * 6)))
	{
		if (txq != 2)
		{
			printk("\nFAILED: pkt:%d should be txq2 pkt, not txq%d pkt\n", testPktCnt, txq);
			return -1;
		}

		if (testPktCnt == (TX_BURST_LEN * 6))
			curTxq -= 1;
	}
	//txq1 pkts expected  
	else if (((TX_BURST_LEN * 6) < testPktCnt) &&
		(testPktCnt <= (TX_BURST_LEN * 7)))
	{
		if (txq != 1)
		{
			printk("\nFAILED: pkt:%d should be txq1 pkt, not txq%d pkt\n", testPktCnt, txq);
			return -1;
		}

		if (testPktCnt == (TX_BURST_LEN * 7))
			curTxq -= 1;
	}
	//txq0 pkts expected
	else if (((TX_BURST_LEN * 7) < testPktCnt) &&
		(testPktCnt <= (TX_BURST_LEN << 3)))	
	{
		if (txq != 0)
		{
			printk("\nFAILED: pkt:%d should be txq0 pkt, not txq%d pkt\n", testPktCnt, txq);
			return -1;
		}

		if (testPktCnt == (TX_BURST_LEN << 3))
			curTxq -= 1;
	}
	//unknown pkts
	else
	{
		printk("\nFAILED: pkt:%d is not known pkt (txq:%d)\n", testPktCnt, txq);
		return -1;
	}


	if (rxContent_check(txData, 20, txLen, baseVal, 1))
		return -1;

		
	testPktCnt++;
	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts (txq%d) recvd\n", __FUNCTION__, dataPathRxPkts, txq);

	if (testPktCnt == (TX_QUEUE_NUM * TX_BURST_LEN))
	{
		printk("\nspPriority test for path%d is finished\n\n", path);
		qdmaDmaAvailable = 1;
	}

	return 0;
}

/* check if packet comming sequence is
 * txq0 -> txq1 -> txq2 -> .... -> txq7 ->
 * txq1 -> txq2 -> .... -> txq7 ->
 * txq2 -> txq3 -> .... -> txq7 ->
 *  ...........................
 * txq7 ->
 * txq0 -> txq1 -> txq2 -> .... -> txq7 ->
 * ............................ */
static int ptm_loopback_wrrPriority_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	int path, txq;
	uint8 *txData;
	uint8 baseVal;
	uint16 txLen;



	path = rxMsg->rxMsgW0.bits.pathNo;
	txData = skb->data;
	baseVal = txData[15];
	txLen = *(uint16*) (&txData[16]);
	txq = ((txData[19] >> 4) & 0xf);

	if (txData[18] != WRR_PRIORITY_TESTTYPE)
	{
		printk("\nFAILED: %s: (path:%d)\n"
				"txData[18]:%02X != WRR_PRIORITY_TESTTYPE\n",
				__FUNCTION__, path, txData[18]);
		return -1;
	}

	if (path != (txData[19] & 0xf))
	{
		printk("\nFAILED: %s:\n"
				" path:%d != (txData[19] & 0xf):%d\n",
				__FUNCTION__, path, (int)(txData[19] & 0xf));
		return -1;
	}

	if (isWrrFirstTest[path])
	{
		dataPathRxPkts++;
		PTM_DBG(DBG_L1, "%d pkts (path%d txq%d) recvd\n", dataPathRxPkts, path, txq);

		if (dataPathRxPkts == TX_QUEUE_NUM)
		{
			printk("\nWRR first Test for path%d is finished\n\n", path);
			qdmaDmaAvailable = 1;
		}

		return 0;
	}


	switch (wrrQueuePhase) 
	{
		case 7: //txq7 pkts
		case 6: //txq6 pkts
		case 5: //txq5 pkts
		case 4: //txq4 pkts
		case 3: //txq3 pkts
		case 2: //txq2 pkts
		case 1: //txq1 pkts
		case 0: //txq0 pkts		
			if (wrrQueuePhase != txq)
			{
				printk("\nFAILED: pkt:%d should be txq%d pkt, not txq%d pkt\n", 
				dataPathRxPkts, wrrQueuePhase, txq);
				return 0;
			}

			wrrQueuePhase++;

			if (wrrQueuePhase == TX_QUEUE_NUM)
			{
				wrrQueueStart++;
				if (wrrQueueStart == TX_QUEUE_NUM)
					wrrQueueStart = 0;
				
				wrrQueuePhase = wrrQueueStart;
			}			

			break;
		default: //unKnown pkts
			printk("\nFAILED: pkt:%d is not known pkt\n", dataPathRxPkts);
			return -1;
	}


	if (rxContent_check(txData, 20, txLen, baseVal, 1))
		return -1;


	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts (txq%d) recvd\n", __FUNCTION__, dataPathRxPkts, txq);

	if (dataPathRxPkts == (txqWrrTotalVal<<3))
	{
		printk("\nWRR test for path%d is finished\n\n", path);
		qdmaDmaAvailable = 1;
	}

	return 0;
}

/* check if packet comming sequence is 
 * (TX_BURST_LEN packets for txq7) ->
 * (TX_BURST_LEN packets for txq6) ->
 * (TX_BURST_LEN packets for txq5) ->
 * txq0 -> txq1 -> txq2 -> txq3 -> txq4 ->
 * txq1 -> txq2 -> txq3 -> txq4 ->
 * txq2 -> txq3 -> txq4 ->
 * txq3 -> txq4 ->
 * txq4 ->
 * txq0 -> txq1 -> txq2 -> txq3 -> txq4 ->
 * ............................ */
static int ptm_loopback_spWrr_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	int path, txq;
	uint8 *txData;
	uint8 baseVal;
	uint16 txLen;



	path = rxMsg->rxMsgW0.bits.pathNo;
	txData = skb->data;
	baseVal = txData[15];
	txLen = *(uint16*) (&txData[16]);
	txq = ((txData[19] >> 4) & 0xf);

	if (txData[18] != SP_WRR_TESTTYPE)
	{
		printk("\nFAILED: %s: (path:%d)\n"
				"txData[18]:%02X != SP_WRR_TESTTYPE\n",
				__FUNCTION__, path, txData[18]);
		return -1;
	}

	if (path != (txData[19] & 0xf))
	{
		printk("\nFAILED: %s:\n"
				" path:%d != (txData[19] & 0xf):%d\n",
				__FUNCTION__, path, (int)(txData[19] & 0xf));
		return -1;
	}


	if (isWrrFirstTest[path])
	{
		dataPathRxPkts++;
		PTM_DBG(DBG_L1, "%d pkts (path%d txq%d) recvd\n", dataPathRxPkts, path, txq);

		if (dataPathRxPkts == TX_QUEUE_NUM)
		{
			printk("\nSP_WRR first Test for WRR for path%d is finished\n\n", path);
			qdmaDmaAvailable = 1;
		}

		return 0;
	}
	

	//txq7 pkts expected
	if (testPktCnt <= (TX_BURST_LEN * 1))
	{
		if (txq != 7)
		{
			printk("\nFAILED: pkt:%d should be txq7 pkt,"
					" not txq%d pkt\n", testPktCnt, txq);
			return -1;
		}

		if (testPktCnt == (TX_BURST_LEN * 1))
				curTxq -= 1;
	}
	//txq6 pkts expected
	else if (((TX_BURST_LEN * 1) < testPktCnt) && 
			(testPktCnt <= (TX_BURST_LEN << 1)))
	{
		if (txq != 6)
		{
			printk("\nFAILED: pkt:%d should be txq6 pkt,"
					" not txq%d pkt\n", testPktCnt, txq);
			return -1;
		}
		
		if (testPktCnt == (TX_BURST_LEN << 1))
			curTxq -= 1;
	}
	//txq5 pkts expected
	else if (((TX_BURST_LEN << 1) < testPktCnt) && 
			(testPktCnt <= (TX_BURST_LEN * 3)))
	{
		if (txq != 5)
		{
			printk("\nFAILED: pkt:%d should be txq5 pkt,"
					" not txq%d pkt\n", testPktCnt, txq);
			return -1;
		}

		if (testPktCnt == (TX_BURST_LEN * 3))
			curTxq -= 1;
	}
	//txq4~txq0 pkts expected for WRR
	else
	{
		switch (wrrQueuePhase) 
		{
			case 4: //txq4 pkts
			case 3: //txq3 pkts
			case 2: //txq2 pkts
			case 1: //txq1 pkts
			case 0: //txq0 pkts
				if (txq != wrrQueuePhase)
				{
					printk("\nFAILED: pkt:%d should be txq%d pkt,"
							"not txq%d pkt\n", 
							dataPathRxPkts, wrrQueuePhase, txq);
					return 0;
				}

				wrrQueuePhase++;

				if (wrrQueuePhase == 5)
				{
					wrrQueueStart++;
					if (wrrQueueStart == 5)
						wrrQueueStart = 0;
				
					wrrQueuePhase = wrrQueueStart;
				}
			
				break;
			
			default: //not known pkts
				printk("\nFAILED: pkt:%d is not known pkt\n",
						dataPathRxPkts);
				return -1;
		}
	}

	if (rxContent_check(txData, 20, txLen, baseVal, 1))
		return -1;
		
	testPktCnt++;
	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts (txq%d) recvd\n", __FUNCTION__, dataPathRxPkts, txq);

	if (dataPathRxPkts == ((txqWrrTotalVal*12)+(3*TX_BURST_LEN)))
	{
		printk("\nSP_WRR RX for path%d is finished\n\n", path);
		qdmaDmaAvailable = 1;
	}

	return 0;
}


static int ptm_loopback_vlanTag_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen, vlanTag = 0;
	int path;


	txData = skb->data;
	path = rxMsg->rxMsgW0.bits.pathNo;

	if (*(uint16*)(&txData[12]) == ETHER_TYPE_8021Q)
	{
		vlanTag = (*(uint16*) (&txData[14])) & 0x1fff;
		
		txData += 4; //vlanTag shifts 4 bytes!
		(*(uint16*) (&txData[16])) -= 4;

		if (((txData[19]>>4) & 0x1) == 0)
		{
			printk("\nFAILED: %s: HW shouldn't insert VLAN tag!\n",
					__FUNCTION__);
			return -1;
		}
	}
	else //ETHER_TYPE_IP
	{
		if (((txData[19]>>4) & 0x1))
		{
			printk("\nFAILED: %s: HW should insert VLAN tag!\n",
					__FUNCTION__);
			return -1;
		}
	}
	
	type = txData[14];
	baseVal = txData[15];
	txLen = (*(uint16*) (&txData[16]));

	if (txData[18] != VLAN_TAG_TESTTYPE)
	{
		printk("\nFAILED: %s:\n"
				"txData[18]:%02X != VLAN_TAG_TESTTYPE\n",
				__FUNCTION__, txData[18]);
		return -1;
	}

	if (((txData[19]>>4) & 0x1))
	{
		if (vlanTag != (*(uint16*) (&txData[20])))
		{
			printk("\nFAILED: %s:\n"
			"vlanTag:%d != (*(uint16*) (&txData[20])):%d\n", 
			__FUNCTION__, vlanTag, (*(uint16*) (&txData[20])));
			return -1;		
		}
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19]&0xf):%02X != path:%d\n",
				__FUNCTION__, txData[19]&0xf, path);
		return -1;		
	}

	if (rxContent_check(txData, 22, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}


static int ptm_loopback_vlanUnTag_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;
	uint16 vlanTag = 0;
	int path;


	txData = skb->data;
	type = txData[14];
	baseVal = txData[15];
	txLen = *(uint16*) (&txData[16]);
	path = rxMsg->rxMsgW0.bits.pathNo;


	if (txData[18] != VLAN_UNTAG_TESTTYPE)
	{
		printk("\nFAILED: %s:\n"
				"txData[18]:%02X != VLAN_UNTAG_TESTTYPE\n",
				__FUNCTION__, txData[18]);
		return -1;
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19]&0xf):%02X != path:%d\n",
				__FUNCTION__, txData[19]&0xf, path);
		return -1;		
	}

	if (((txData[19]>>4) & 0x1))
	{
		if (!rxMsg->rxMsgW2.bits.vlanHit)
		{
			printk("\nFAILED: %s: vlanHit bit should be set!\n",
					__FUNCTION__);
			return -1;
		}

		if (rxMsg->rxMsgW2.bits.vlanTpID != 0)
		{
			printk("\nFAILED: %s: vlanTpId:%d should be 0 (for 0x8100)!\n",
					__FUNCTION__, rxMsg->rxMsgW2.bits.vlanTpID);
			return -1;
		}

		vlanTag = (*(uint16*) (&txData[20])) & 0x1fff;
		
		if ((rxMsg->rxMsgW2.bits.vlanTag & 0x1fff) != vlanTag)
		{
			printk("\nFAILED: %s\n"
					"rxMsg->rxMsgW2.vlanTag:%.8x != vlanTag:%.8x",
					__FUNCTION__, rxMsg->rxMsgW2.bits.vlanTag & 0x1fff, 
					vlanTag);
			return -1;
		}
	}
	else
	{
		if (rxMsg->rxMsgW2.bits.vlanHit)
		{
			printk("\nFAILED: %s: vlanHit bit shouldn't be set!\n",
					__FUNCTION__);
			return -1;
		}
	}

	if (rxContent_check(txData, 22, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}


static int ptm_loopback_vlanDblTag_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{	/* only packets with ETHER_TYPE_8021AD, ETHER_TYPE_QINQ, 
 	* or ETHER_TYPE_IP can go into this funation */
	 
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen, vlanTag = 0;
	int path;


	txData = skb->data;
	path = rxMsg->rxMsgW0.bits.pathNo;

	if (*(uint16*)(&txData[12]) == ETHER_TYPE_8021AD)
	{
		vlanTag = (*(uint16*) (&txData[14])) & 0x1fff;

		if ((*(uint16*)(&txData[16]) != ETHER_TYPE_8021Q) ||
			(*(uint16*)(&txData[18]) != 0x1234))
		{
			printk("\nFAILED: %s: 8021Q tagId should be 0x1234!\n",
					__FUNCTION__);
			return -1;
		}
		
		txData += 8; //vlanTag shifts 8 bytes!
		(*(uint16*) (&txData[16])) -= 8;

		if ((((txData[19]>>4)&0x3) != 1))
		{
			printk("\nFAILED: %s: HW skhould insert 8021AD tag!\n",
					__FUNCTION__);
			return -1;
		}
	}
	else if (*(uint16*)(&txData[12]) == ETHER_TYPE_QINQ)
	{
		vlanTag = (*(uint16*) (&txData[14])) & 0x1fff;

		if ((*(uint16*)(&txData[16]) != ETHER_TYPE_8021Q) ||
			(*(uint16*)(&txData[18]) != 0x1567))
		{
			printk("\nFAILED: %s: 8021Q tagId should be 0x1567!\n",
					__FUNCTION__);
			return -1;
		}
		
		txData += 8; //vlanTag shifts 8 bytes!
		(*(uint16*) (&txData[16])) -= 8;

		if ((((txData[19]>>4)&0x3) != 3))
		{
			printk("\nFAILED: %s: HW should insert QinQ tag!\n",
					__FUNCTION__);
			return -1;
		}
	}
	else //ETHER_TYPE_IP
	{
		if ((((txData[19]>>4)&0x3) != 0) && (((txData[19]>>4)&0x3) != 2))
		{
			printk("\nFAILED: %s: HW should insert DBL VLAN tag!\n",
					__FUNCTION__);
			return -1;
		}
	}


	type = txData[14];
	baseVal = txData[15];
	txLen = (*(uint16*) (&txData[16]));

	if (txData[18] != VLAN_DBL_TAG_TESTTYPE)
	{
		printk("\nFAILED: %s:\n"
				"txData[18]:%02X != VLAN_DBL_TAG_TESTTYPE\n",
				__FUNCTION__, txData[18]);
		return -1;
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19]&0xf):%02X != path:%d\n",
				__FUNCTION__, txData[19]&0xf, path);
		return -1;		
	}

	if ((((txData[19]>>4)&0x3) == 1) || (((txData[19]>>4)&0x3) == 3))
	{
		if (vlanTag != (*(uint16*) (&txData[20])))
		{
			printk("\nFAILED: %s:\n"
			"vlanTag:%d != (*(uint16*) (&txData[20])):%d\n", 
			__FUNCTION__, vlanTag, (*(uint16*) (&txData[20])));
			return -1;		
		}
	}

	if (rxContent_check(txData, 22, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}


static int ptm_loopback_vlanDblUnTag_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{	/* only packets with ETHER_TYPE_8021Q 
 	* or ETHER_TYPE_IP can go into this funation */
	
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;
	uint16 vTag = 0;
	uint16 vlanTag = 0;
	int path;


	txData = skb->data;
	path = rxMsg->rxMsgW0.bits.pathNo;

	if (*(uint16*)(&txData[12]) == ETHER_TYPE_8021Q)
	{
		vTag = *(uint16*)(&txData[14]);

		txData += 4;
	}


	type = txData[14];
	baseVal = txData[15];
	txLen = *(uint16*) (&txData[16]);


	if (txData[18] != VLAN_DBL_UNTAG_TESTTYPE)
	{
		printk("\nFAILED: %s:\n"
				"txData[18]:%02X != VLAN_DBL_UNTAG_TESTTYPE\n",
				__FUNCTION__, txData[18]);
		return -1;
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19]&0xf):%02X != path:%d\n",
				__FUNCTION__, txData[19]&0xf, path);
		return -1;		
	}

	if (((txData[19]>>4)&0x3) == 1)
	{
		if (vTag != 0x189a)
		{
			printk("\nFAILED: %s: 8021Q tagId should be 0x189a!\n",
					__FUNCTION__);
			return -1;
		}

		if (!rxMsg->rxMsgW2.bits.vlanHit)
		{
			printk("\nFAILED (TPID_0x88a8): %s: vlanHit bit should be set!\n",
					__FUNCTION__);
			return -1;
		}

		if (rxMsg->rxMsgW2.bits.vlanTpID != 1)
		{
			printk("\nFAILED: %s: vlanTpId should be 1!\n",
					__FUNCTION__);
			return -1;
		}

		vlanTag = (*(uint16*) (&txData[20])) & 0x1fff;
		
		if ((rxMsg->rxMsgW2.bits.vlanTag & 0x1fff) != vlanTag)
		{
			printk("\nFAILED (TPID_0x88a8): %s\n"
					"rxMsg->rxMsgW2.vlanTag:%.8x != vlanTag:%.8x",
					__FUNCTION__, rxMsg->rxMsgW2.bits.vlanTag & 0x1fff, 
					vlanTag);
			return -1;
		}

		txLen -= 4;
	}
	else if (((txData[19]>>4)&0x3) == 3)
	{
		if ((((txData[19]>>4)&0x3) == 3) && (vTag != 0x1bcd))
		{
			printk("\nFAILED: %s: 8021Q tagId should be 0x1bcd!\n",
					__FUNCTION__);
			return -1;
		}
		
		if (!rxMsg->rxMsgW2.bits.vlanHit)
		{
			printk("\nFAILED (TPID_0x9100): %s: vlanHit bit should be set!\n",
					__FUNCTION__);
			return -1;
		}

		if (rxMsg->rxMsgW2.bits.vlanTpID != 2)
		{
			printk("\nFAILED: %s: vlanTpId should be 2!\n",
					__FUNCTION__);
			return -1;
		}

		vlanTag = (*(uint16*) (&txData[20])) & 0x1fff;
		
		if ((rxMsg->rxMsgW2.bits.vlanTag & 0x1fff) != vlanTag)
		{
			printk("\nFAILED (TPID_0x9100): %s\n"
					"rxMsg->rxMsgW2.vlanTag:%.8x != vlanTag:%.8x",
					__FUNCTION__, rxMsg->rxMsgW2.bits.vlanTag & 0x1fff, 
					vlanTag);
			return -1;
		}

		txLen -= 4;
	}
	else
	{
		if (rxMsg->rxMsgW2.bits.vlanHit)
		{
			printk("\nFAILED: %s: vlanHit bit shouldn't be set!\n",
					__FUNCTION__);
			return -1;
		}
	}

	if (rxContent_check(txData, 22, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}


static int ptm_loopback_ipCsInsert_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen, ipCs = 0;
	unsigned short chksum;
	int path;


	txData = skb->data;
	path = rxMsg->rxMsgW0.bits.pathNo;

	//txData[14] is changed in TX, so don't use.
	type = RAN_PKTTYPE;
	
	baseVal = txData[15];
	txLen = (*(uint16*) (&txData[16]));

	if (txData[18] != IPCS_INSERT_TESTTYPE)
	{
		printk("\nFAILED: %s:\n"
				"txData[18]:%02X != IPCS_INSERT_TESTTYPE\n",
				__FUNCTION__, txData[18]);
		return -1;
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19]&0xf):%02X != path:%d\n",
				__FUNCTION__, txData[19]&0xf, path);
		return -1;		
	}

	if ((txData[19]>>4)&0x1)
	{
		ipCs = (*(uint16*) (&txData[24]));
		(*(uint16*) (&txData[24])) = 0;
		chksum = in_csum((unsigned short *) &txData[14], 20);
		if (ipCs != chksum)
		{
			printk("\nFAILED: %s:\n"
					"ipCs:%.4x != chksum:%.4x\n",
					__FUNCTION__, ipCs, chksum);
			return -1;			
		}
	}
	else
	{
		if ((*(uint16*) (&txData[24])) != 0)
		{
			printk("\nFAILED: %s:\n"
					"txData[24-25]:%.4x should be 0\n",
					__FUNCTION__, *(uint16*) (&txData[24]));
			return -1;
		}
	}

	if (rxContent_check(txData, 26, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}


static int ptm_loopback_ipCsCheck_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;
	int path;


	txData = skb->data;
	path = rxMsg->rxMsgW0.bits.pathNo;

	//txData[14] is changed in ipCsCheck_tx, so don't use.
	type = RAN_PKTTYPE;
	
	baseVal = txData[15];
	txLen = (*(uint16*) (&txData[16]));

	if (txData[18] != IPCS_CHECK_TESTTYPE)
	{
		printk("\nFAILED: %s:\n"
				"txData[18]:%02X != IPCS_CHECK_TESTTYPE\n",
				__FUNCTION__, txData[18]);
		return -1;
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19]&0xf):%02X != path:%d\n",
				__FUNCTION__, txData[19]&0xf, path);
		return -1;		
	}

	if ((txData[19]>>4)&0x1)
	{
		if (rxMsg->rxMsgW1.bits.ip4CsErr)
		{
			printk("\nFAILED: %s:\n"
					"ipCsErr bit shouldn't be set\n",
					__FUNCTION__);
			return -1;
		}
	}
	else
	{
		if (!rxMsg->rxMsgW1.bits.ip4CsErr)
		{
			printk("\nFAILED: %s:\n"
					"ipCsErr bit should be set\n",
					__FUNCTION__);
			return -1;
		}
	}

	if (rxContent_check(txData, 26, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}

/* There are five parts for BACP header, which are
 * DST_MAC, ORG_SUB, ITU_OUI, ITU_SUB, VERSION.
 * Only if the BACP header matchs the enable parts
 * in BACP registers, the packet is for BACP. 
 * Note: for disable parts, we don't care, so we
 * suoopse they are Hits */
static int ptm_loopback_bacpCheck_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;
	int path;
	uint8 *bacpHdr = NULL, *regP = NULL;
	uint32 reg0, reg1, reg2, reg3, regEn;
	int macHit=0, orgHit=0, ituoHit=0, itusHit=0, bacpHit=0;


	txData = skb->data;
	path = rxMsg->rxMsgW0.bits.pathNo;

	reg0 = read_reg_word(PTM_BACP_FIELD_0);
	reg1 = read_reg_word(PTM_BACP_FIELD_1);
	reg2 = read_reg_word(PTM_BACP_FIELD_2);
	reg3 = read_reg_word(PTM_BACP_FIELD_3);
	regEn = read_reg_word(PTM_BACP_FIELD_EN);


	if ((!rxMsg->rxMsgW0.bits.isBacp && rxMsg->rxMsgW0.bits.isRaw) ||
		(rxMsg->rxMsgW0.bits.isBacp && !rxMsg->rxMsgW0.bits.isRaw))
	{
		printk("\nFAILED: isBacp bit & isRaw bit should both hit or un-hit at the same time\n");
		return -1;
	}

	if (regEn & BACP_DST_MAC_EN) //Dest MAC check Enable
	{
		bacpHdr = (uint8 *)&bacpHeader[1];
		regP = (uint8 *)&reg1;

		if (bacpHeader[0] == reg0 && bacpHdr[0] == regP[0] && bacpHdr[1] == regP[1])
		macHit = 1;
	}
	else
		macHit = 1; //don't care

	if (regEn & BACP_ORG_SUB_EN) //Org SubType check Enable
	{
		bacpHdr = (uint8 *)&bacpHeader[2];
		regP = (uint8 *)&reg2;

		if (bacpHdr[0] == regP[0])
			orgHit = 1;
	}
	else
		orgHit = 1; //don't care

	if (regEn & BACP_ITU_OUI_EN) //ITU OUI check Enable
	{
		bacpHdr = (uint8 *)&bacpHeader[2];
		regP = (uint8 *)&reg2;

		if (bacpHdr[1] == regP[1] && bacpHdr[2] == regP[2] && bacpHdr[3] == regP[3])
		ituoHit = 1;
	}
	else
		ituoHit = 1; //dont care

	if (regEn & BACP_ITU_SUB_EN) //ITU SubType check Enable
	{
		bacpHdr = (uint8 *)&bacpHeader[3];
		regP = (uint8 *)&reg3;

		if (bacpHdr[0] == regP[0])
			itusHit = 1;
	}
	else
		itusHit = 1; //dont care

	if (regEn & BACP_VERSION_EN) //BACP version check Enable
	{
		bacpHdr = (uint8 *)&bacpHeader[3];
		regP = (uint8 *)&reg3;

		if (bacpHdr[1] == regP[1])
			bacpHit = 1;
	}
	else
		bacpHit = 1; //dont care



	if (*(uint16*)(&txData[12]) == ETHER_TYPE_SLOW)
	{
		/* Only if the BACP header matchs the enable parts
		 * in BACP registers, the packet is for BACP */
		if (macHit && orgHit && ituoHit && itusHit && bacpHit)
		{
			if (!rxMsg->rxMsgW0.bits.isBacp)
			{
				printk("\nFAILED: isBacp bit should be set\n");
				return -1;
			}
		}
		else
		{
			if (rxMsg->rxMsgW0.bits.isBacp)
			{
				printk("\nFAILED: isBacp bit shouldn't be set\n"
						"macHit:%d, orgHit:%d, ituoHit:%d, itusHit:%d, bacpHit:%d\n", macHit, orgHit, ituoHit, itusHit, bacpHit);
				return -1;
			}
		}


		txData += 10; //shift 10 bytes
		(*(uint16*) (&txData[16])) -= 10;

		if (!((txData[19]>>4)&0x1))
		{
			printk("\nFAILED: %s:\n"
					"This shouldn't be a BACP packet!\n",
					__FUNCTION__);
		}	
	}
	else //ETHER_TYPE_IP
	{
		if (rxMsg->rxMsgW0.bits.isBacp)
		{
			regP = (uint8 *)&reg3;
			/* normal packets' path_no is in the position 
			 * of bacp version, so bacp hit may be 
			 * triggered, so double check. */
			if (path != regP[1])
			{
				printk("\nFAILED: %s: isBacp bit shouldn't be set for normal packet!\n", __FUNCTION__);
				return -1;
			}
		}

		if ((txData[19]>>4)&0x1)
		{
			printk("\nFAILED: %s:\n"
					"This should be a BACP packet!\n",
					__FUNCTION__);
		}
	}
	
	type = txData[14];
	baseVal = txData[15];
	txLen = (*(uint16*) (&txData[16]));

	if (txData[18] != BACP_PKT_TESTTYPE)
	{
		printk("\nFAILED: %s:\n"
				"txData[18]:%02X != BACP_PKT_TESTTYPE\n",
				__FUNCTION__, txData[18]);
		return -1;
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19]&0xf):%02X != path:%d\n",
				__FUNCTION__, txData[19]&0xf, path);
		return -1;		
	}

	if (rxContent_check(txData, 20, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	//PTM_DBG(DBG_L1, "%s: %d pkts recvd\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}


static int ptm_loopback_runtCheck_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;
	int path;


	txData = skb->data;
	path = rxMsg->rxMsgW0.bits.pathNo;

	type = txData[14];
	baseVal = txData[15];
	txLen = (*(uint16*) (&txData[16]));

	if (txData[18] != RUNT_PKT_TESTTYPE)
	{
		printk("\nFAILED: %s:\n"
				"txData[18]:%02X != RUNT_PKT_TESTTYPE\n",
				__FUNCTION__, txData[18]);
		return -1;
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19]&0xf):%02X != path:%d\n",
				__FUNCTION__, txData[19]&0xf, path);
		return -1;		
	}

	if ((txData[19]>>4)&0x1)
	{
		if (!rxMsg->rxMsgW0.bits.isRunt)
		{
			printk("\nFAILED: %s: isRunt bit should be set!\n",
					__FUNCTION__);
			return -1;
		}

		if (txLen > 59)
		{
			printk("\nFAILED: %s: txLen:%d is too long!\n",
					__FUNCTION__, txLen);
			return -1;
		}
	}
	else
	{
		if (rxMsg->rxMsgW0.bits.isRunt)
		{
			printk("\nFAILED: %s: isRunt bit shouldn't be set!\n",
					__FUNCTION__);
			return -1;
		}

		if (txLen < 60)
		{
			printk("\nFAILED: %s: txLen:%d is too short!\n",
					__FUNCTION__, txLen);
			return -1;
		}
	}

	if (rxContent_check(txData, 20, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}


static int ptm_loopback_longCheck_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;
	int path;


	txData = skb->data;
	path = rxMsg->rxMsgW0.bits.pathNo;

	type = txData[14];
	baseVal = txData[15];
	txLen = (*(uint16*) (&txData[16]));

	if (txData[18] != LONG_PKT_TESTTYPE)
	{
		printk("\nFAILED: %s:\n"
				"txData[18]:%02X != LONG_PKT_TESTTYPE\n",
				__FUNCTION__, txData[18]);
		return -1;
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19]&0xf):%02X != path:%d\n",
				__FUNCTION__, txData[19]&0xf, path);
		return -1;		
	}

	if ((txData[19]>>4)&0x1)
	{
		if (!rxMsg->rxMsgW0.bits.isLong)
		{
			printk("\nFAILED: %s: isLong bit should be set!\n",
					__FUNCTION__);
			return -1;
		}

		if (!( (1515 <= txLen) && (txLen <= MAX_PKT_SIZE) ))
		{
			printk("\nFAILED: %s: txLen:%d is out of expectation!\n",
					__FUNCTION__, txLen);
			return -1;
		}
	}
	else
	{
		if (rxMsg->rxMsgW0.bits.isLong)
		{
			printk("\nFAILED: %s: isLong bit shouldn't be set!\n",
					__FUNCTION__);
			return -1;
		}

		if (txLen > 1514)
		{
			printk("\nFAILED: %s: txLen:%d is too long!\n",
					__FUNCTION__, txLen);
			return -1;
		}
	}

	if (rxContent_check(txData, 20, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}


static int ptm_loopback_crcCheck_rx(
		ptmRxMsg_t *rxMsg, struct sk_buff *skb
)
{
	uint8 *txData;
	uint8 type;
	uint8 baseVal;
	uint16 txLen;
	int path;


	txData = skb->data;
	path = rxMsg->rxMsgW0.bits.pathNo;

	type = txData[14];
	baseVal = txData[15];
	txLen = (*(uint16*) (&txData[16]));

	if (txData[18] != CRCERR_PKT_TESTTYPE)
	{
		printk("\nFAILED: %s:\n"
				"txData[18]:%02X != CRCERR_PKT_TESTTYPE\n",
				__FUNCTION__, txData[18]);
		return -1;
	}

	if ((txData[19] & 0xf) != path)
	{
		printk("\nFAILED: %s:\n"
				"(txData[19]&0xf):%02X != path:%d\n",
				__FUNCTION__, txData[19]&0xf, path);
		return -1;		
	}

	if ((txData[19]>>4)&0x1)
	{
		if (!rxMsg->rxMsgW0.bits.crcErr)
		{
			printk("\nFAILED: %s: crcErr bit should be set!\n",
					__FUNCTION__);
			return -1;
		}
	}
	else
	{
		if (rxMsg->rxMsgW0.bits.crcErr)
		{
			printk("\nFAILED: %s: crcErr bit shouldn't be set!\n",
					__FUNCTION__);
			return -1;
		}
	}

	if (rxContent_check(txData, 20, txLen, baseVal, type))
		return -1;


	dataPathRxPkts++;
	PTM_DBG(DBG_L1, "%s: %d pkts recvd\n", __FUNCTION__, dataPathRxPkts);

	/* when the packet is received, un-lock txHold 
	 * and then wake up Tx to send next packet */
	rxWakeUpTx();

	return 0;
}

/* the Rx entrance for loopback packets */
int ptm_loopback_rx(
		ptmRxMsg_t *rxMsg, uint32 rxMsgLen, 
		struct sk_buff *skb, uint32 frameSize
)
{
	uint8 *txData, *macAddr = &dstMacAddr[0];
	int k;
	int lenOffset;

	if (rxMsg == NULL)
	{
		printk("\nERROR: rxMsg is NULL at %s\n", __FUNCTION__);
		goto stop_testing;
	}
	if (skb == NULL)
	{
		printk("\nERROR: skb is NULL at %s\n", __FUNCTION__);
		goto stop_testing;
	}

	txData = skb->data;

	//check rx msg len
	if (rxMsgLen != sizeof(ptmRxMsg_t))
	{
		printk("\nWrong rxMsgLen:%d\n"
				"sizeof(ptmRxMsg_t):%d\n"
				, (int)rxMsgLen, sizeof(ptmRxMsg_t));
		goto stop_testing;
	}


	//check rx CRC msg error
	if (!isCrcCheckTest && rxMsg->rxMsgW0.bits.crcErr)
	{
		printk("\nFAILED: CRC ERROR\n");
		goto stop_testing;
	}


	//check destion mac
	if (*(uint16*)(&txData[12]) == ETHER_TYPE_SLOW)
		macAddr = (uint8 *)&bacpHeader[0];

	for (k = 0; k < 6; k++)
	{
		if (txData[k] != macAddr[k])
		{
			printk("\nWrong Dst MAC:\n");
			printk("txData[0~5]: ");
			for (k = 0; k < 6; k++)
				printk("%02X ", txData[k]);
			printk("\ndstMacAddr: ");
			for (k = 0; k < 6; k++)
				printk("%02X ", macAddr[k]);
			printk("\n");

			goto stop_testing;
		}
	}


	//check source mac
	for (k = 6; k < 12; k++)
	{
		if (txData[k] != defMacAddr[k-6])
		{
			printk("\nWrong Src MAC:\n");
			printk("txData[6~11]: ");
			for (k = 6; k < 12; k++)
				printk("%02X ", txData[k]);
			printk("\nsrcMacAddr: ");
			for (k = 6; k < 12; k++)
				printk("%02X ", defMacAddr[k-6]);
			printk("\n");

			goto stop_testing;
		}
	}


	//decide offset for checking frame size
	if (*(uint16*)(&txData[12]) == ETHER_TYPE_IP)
		lenOffset = 16;
	else if (*(uint16*)(&txData[12]) == ETHER_TYPE_8021Q)
		lenOffset = 20;
	else if ((*(uint16*)(&txData[12]) == ETHER_TYPE_8021AD) ||
			(*(uint16*)(&txData[12]) == ETHER_TYPE_QINQ))
		lenOffset = 24;
	else if (*(uint16*)(&txData[12]) == ETHER_TYPE_SLOW)
		lenOffset = 26;
	else
	{
		printk("\nWrong Ether type:%d\n", txData[12]);
		goto stop_testing;
	}

	//BACP is raw packet, so FE won't strip its 4-byte CRC
	if (rxMsg->rxMsgW0.bits.isRaw)
		frameSize -= 4;

	//check frame size
	if ((*(uint16*)(&txData[lenOffset])) != frameSize) 
	{
		printk("\nWrong frame size:\n"
				"txData[%d-%d]:%d != frameSize:%d\n",
				lenOffset, lenOffset+1, 
				*(uint16*)(&txData[lenOffset]), (int)frameSize);

		goto stop_testing;
	}
	
	//decide what kind of test is for this packet. 
	if (*(uint16*)(&txData[12]) == ETHER_TYPE_IP)
	{
		switch (txData[18]) //test type
		{
			case SINGLE_PKT_TESTTYPE:
				if (ptm_loopback_singlePkt_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case DATA_PATH_TESTTYPE:
				if (ptm_loopback_dataPath_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case DATA_BURST_TESTTYPE:
				if (ptm_loopback_dataBurst_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case MULTI_CHANNEL_TESTTYPE:
				if (ptm_loopback_multiChannel_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case PATH_NO_TESTTYPE:
				if (ptm_loopback_pathNo_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case PREEMPTION_TESTTYPE:
				if (ptm_loopback_preemption_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case SP_PRIORITY_TESTTYPE:
				if (ptm_loopback_spPriority_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case WRR_PRIORITY_TESTTYPE:
				if (ptm_loopback_wrrPriority_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case SP_WRR_TESTTYPE:
				if (ptm_loopback_spWrr_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case VLAN_TAG_TESTTYPE:
				if (ptm_loopback_vlanTag_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case VLAN_UNTAG_TESTTYPE:
				if (ptm_loopback_vlanUnTag_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case VLAN_DBL_TAG_TESTTYPE:
				if (ptm_loopback_vlanDblTag_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case VLAN_DBL_UNTAG_TESTTYPE:
				if (ptm_loopback_vlanDblUnTag_rx(rxMsg, skb))
					goto stop_testing;
				break;	
			case IPCS_INSERT_TESTTYPE:
				if (ptm_loopback_ipCsInsert_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case IPCS_CHECK_TESTTYPE:
				if (ptm_loopback_ipCsCheck_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case BACP_PKT_TESTTYPE:
				if (ptm_loopback_bacpCheck_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case RUNT_PKT_TESTTYPE:
				if (ptm_loopback_runtCheck_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case LONG_PKT_TESTTYPE:
				if (ptm_loopback_longCheck_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case CRCERR_PKT_TESTTYPE:
				if (ptm_loopback_crcCheck_rx(rxMsg, skb))
					goto stop_testing;
				break;
				
			default:
				printk("\nWrong test type:%d for ETHER_TYPE_IP\n", 
				txData[18]);
				goto stop_testing;
		}
	}
	else if (*(uint16*)(&txData[12]) == ETHER_TYPE_8021Q)
	{
		txData += 4; //shift 4 bytes
	
		switch (txData[18]) //test type
		{
			case VLAN_TAG_TESTTYPE:
				if (ptm_loopback_vlanTag_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case VLAN_UNTAG_TESTTYPE:
				printk("\nFAILED: HW didn't un-tag correctly\n");
				goto stop_testing;
			case VLAN_DBL_TAG_TESTTYPE:
				printk("\nFAILED: HW didn't double tag correctly\n");
				goto stop_testing;
			case VLAN_DBL_UNTAG_TESTTYPE:
				if (ptm_loopback_vlanDblUnTag_rx(rxMsg, skb))
					goto stop_testing;
				break;
				
			default:
				printk("\nWrong test type:%d for ETHER_TYPE_8021Q\n", 
				txData[18]);
				goto stop_testing;
		}
	
	}
	else if ((*(uint16*)(&txData[12]) == ETHER_TYPE_8021AD) ||
			(*(uint16*)(&txData[12]) == ETHER_TYPE_QINQ))
	{
		txData += 8; //shift 8 bytes

		switch (txData[18]) //test type
		{
			case VLAN_DBL_TAG_TESTTYPE:
				if (ptm_loopback_vlanDblTag_rx(rxMsg, skb))
					goto stop_testing;
				break;
			case VLAN_DBL_UNTAG_TESTTYPE:
				printk("\nFAILED: HW didn't double untag correctly\n");
				goto stop_testing;
			
			default:
				printk("\nWrong test type:%d for 8021AD/QINQ\n", 
				txData[18]);
				goto stop_testing;
		}

	}
	else if (*(uint16*)(&txData[12]) == ETHER_TYPE_SLOW)
	{
		if (ptm_loopback_bacpCheck_rx(rxMsg, skb))
			goto stop_testing;
	}
	else
	{
		printk("\nWrong ether type:%d\n", txData[12]);
		goto stop_testing;
	}

	return 0;


stop_testing:

	stopTesting = 1;

	printk("\n%d pkts have received OK\n", dataPathRxPkts);
	printk("\nDump RX Message:\n");
	dump_data((char*)rxMsg, rxMsgLen);
	printk("\nDump skb->data:\n");
	dump_data(skb->data, frameSize);
	
	return -1;
}

