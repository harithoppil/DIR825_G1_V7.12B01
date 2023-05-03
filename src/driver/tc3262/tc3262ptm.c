/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/tc3262/tc3262ptm.c#2 $
*/
/************************************************************************
 *
 *	Copyright (C) 2008 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
/*
** $Log: tc3262ptm.c,v $
** Revision 1.18  2011/10/12 13:36:29  frankliao_hc
** bug 11425
**
** Revision 1.17  2011/07/29 13:01:08  frankliao_hc
** bug 10703  ptm qos setting
**
** Revision 1.16  2011/07/15 01:13:41  lino
** do reset when tx/rx disabled
**
** Revision 1.15  2011/07/09 02:09:17  treychen_hc
** bug#10858 --Trey
**
** Revision 1.14  2011/06/30 12:07:13  lino
** hwnat enhance: IPv6 and QinQ support
**
** Revision 1.13  2011/06/30 09:06:55  serenahuang_hc
** remove compile option WITHVOIP
**
** Revision 1.12  2011/06/23 08:44:47  lino
** when repeating close/open PTM device and traffic is from PTM, it has chance the PTM interrupt is disabled
**
** Revision 1.11  2011/06/23 04:50:23  serenahuang_hc
** Put rtp packets to HH priority when qos_flag not be selected as WRR or PQ
**
** Revision 1.10  2011/06/16 09:14:10  lino
** add RT65168 support: turn off hwnat when disabled hwnat
**
** Revision 1.9  2011/06/15 07:19:55  lino
** add RT65168 support: internet led support
**
** Revision 1.8  2011/06/10 07:54:42  lino
** add RT65168 support
**
** Revision 1.7  2011/06/09 08:17:26  lino
** add RT65168 support
**
** Revision 1.6  2011/06/08 19:20:55  frankliao_hc
** add rt65168 support
**
** Revision 1.5  2011/06/08 15:05:56  treychen_hc
** ptm webpage --Trey
**
** Revision 1.4  2011/06/08 10:02:37  lino
** add RT65168 support
**
** Revision 1.3  2011/06/03 10:35:51  lino
** add RT65168 support
**
** Revision 1.2  2011/06/03 02:33:10  lino
** add RT65168 support
**
** Revision 1.1.1.1.6.3  2011/04/01 09:54:53  lino
** add RT65168 support
**
** Revision 1.1.1.1.6.2  2011/04/01 08:30:24  lino
** add RT65168 support
**
** Revision 1.1.1.1.6.1  2011/01/19 06:25:11  lino
** add RT65168 support
**
** Revision 1.1.1.1  2010/09/30 21:14:54  josephxu
** modules/public, private
**
** Revision 1.1.1.1  2010/04/09 09:34:42  feiyan
** New TC Linux Make Flow Trunk
**
** Revision 1.1.1.1  2009/12/17 01:47:37  josephxu
** 20091217, from Hinchu ,with VoIP
**
 */

#define TC3262_PTM_NAPI

#define TC3262_PTM_SKB_RECYCLE

#define DRV_NAME	"tc3262ptm"
#ifdef TC3262_PTM_NAPI
#define DRV_VERSION	"1.00-NAPI"
#else
#define DRV_VERSION	"1.00"
#endif
#define DRV_RELDATE	"16.Sep.2008"

static const char *const version =
    DRV_NAME ".c:v" DRV_VERSION " " DRV_RELDATE "\n";

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
#include "../tcphy/tcconsole.h"
#include <linux/skbuff.h>
#if defined(TCSUPPORT_HWNAT)
#include <linux/pktflow.h>
#endif

#include "tc3262ptm.h"

#define RX_BUF_LEN 			(2048 - NET_SKB_PAD - 64 - (sizeof(struct skb_shared_info)))
#define RX_MAX_PKT_LEN 		1536
#define TR068_LED  //add by xiaobodong


//#undef CONFIG_TC3162_DMEM

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/
#if defined(TCSUPPORT_AUTOBENCH)
#define LOOPBACK_SUPPORT
#endif
//#define LOOPBACK_SUPPORT

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

#ifndef TCSUPPORT_QOS
#define QOS_REMARKING  1
#endif
#define TCSUPPORT_HW_QOS

#ifdef QOS_REMARKING
#define QOS_REMARKING_MASK    0x00000007
#define QOS_REMARKING_FLAG    0x00000001
//#define QOS_DMAWRR_USERDEFINE  0x01
#endif
#if defined (QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
#define QOS_DMAWRR_USERDEFINE 0x1
#define PTQCR_WRR_EN			(1<<4)
#define PTQCR_WRR_SELECT 	 	(1<<6)
#endif

#ifdef TCSUPPORT_QOS
#define		QOS_FILTER_MARK		0xf0
#define 	QOS_HH_PRIORITY		0x10
#define 	QOS_H_PRIORITY		0x20
#define 	QOS_M_PRIORITY		0x30
#define		QOS_L_PRIORITY		0x40

#define		NULLQOS				-1
#define 	QOS_SW_PQ			0	//will use hw at the same time
#define		QOS_SW_WRR			1
#define		QOS_SW_CAR			2
#define 	QOS_HW_WRR			3
#define		QOS_HW_PQ			4

#endif

#ifdef WAN2LAN
#define SKBUF_COPYTOLAN (1<<26) //xyzhu_nj_091105:borrow this bit to show if the packet is used for wan2lan
#define TX_STAG_LEN 6
#endif

/************************************************************************
*				E X T E R N A L   R E F E R E N C E S
*************************************************************************
*/

#ifdef TR068_LED
extern int internet_led_on;
extern int internet_trying_led_on;
#endif

/************************************************************************
*                            M A C R O S
*************************************************************************
*/

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/
struct ringinfo {
	int bearer;
	int queue;
};

struct ptm_stats {
	unsigned long	rx_bytes;		/* total bytes received 	*/
	unsigned long	tx_bytes;		/* total bytes transmitted	*/
};

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static irqreturn_t tc3262_ptm_isr(int irq, void *dev_id);
static int tc3262_ptm_tx(struct sk_buff *skb, struct net_device *dev);
static void ptmDrvStart(int bearer);
static void ptmDrvStop(int bearer);
#if (defined(TCSUPPORT_WAN_ETHER) || defined(TCSUPPORT_WAN_PTM)) && defined(TCSUPPORT_MULTISERVICE_ON_WAN)
static void ptmTxDrvStop(int bearer);
static void ptmTxDrvStart(int bearer);
#endif
#ifdef LOOPBACK_SUPPORT
static void dump_skb(struct sk_buff *skb);
#endif

/************************************************************************
*                        P U B L I C   D A T A
*************************************************************************
*/

/* priority packet check parameters */

int priPktChkLen = DEF_PRIORITY_PKT_CHK_LEN;
module_param(priPktChkLen, int, 0);
int priPktChk = 0;
module_param(priPktChk, int, 0);

#ifdef TCSUPPORT_QOS
static int qos_flag = NULLQOS;
#endif

//#ifdef QOS_REMARKING  /*Rodney_20090724*/
#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
static int qos_wrr_info[5] = {0};
static int max_prio = 3;
static uint8 qos_wrr_user = 0x00;
#endif
/************************************************************************
*                      E X T E R N A L   D A T A
*************************************************************************
*/
#ifdef WAN2LAN
extern unsigned char masko; //use this flag control if open wan2lan function
#endif

/************************************************************************
*                       P R I V A T E   D A T A
*************************************************************************
*/

/* Device data */
static struct net_device *tc3262_ptm_dev[PTM_BEARER_NUM];

static ptmAdapter_t *ptm_p[PTM_BEARER_NUM] = {NULL, NULL};

static uint8 ptmInitialized[PTM_BEARER_NUM] = {0, 0};

static struct ptm_stats ptmStats[PTM_BEARER_NUM];

#ifdef LOOPBACK_SUPPORT
static uint16 ptmLoopback = 0;
static atomic_t ptmRxLoopback;
#endif

static uint8 def_mac_addr[] = {0x00, 0x00, 0xaa, 0xbb, 0xcc, 0xff};

static DEFINE_SPINLOCK(pimr_lock);

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
#define read_reg_word(reg) 			VPint(reg)
#define write_reg_word(reg, wdata) 	VPint(reg)=wdata

#define pause(x)					mdelay(x)

static inline struct sk_buff *ptm_alloc_skb2k(void)
{
#ifdef TC3262_PTM_SKB_RECYCLE
	return skbmgr_dev_alloc_skb2k();
#else
	return dev_alloc_skb(RX_BUF_LEN);
#endif
}

static inline struct sk_buff *ptm_alloc_skb128(void)
{
#ifdef TC3262_PTM_SKB_RECYCLE
	return skbmgr_dev_alloc_skb128();
#else
	return dev_alloc_skb(SG_MAX_PKT_LEN);
#endif
}

void ptmIMRMask(int bearer)
{
	uint32 reg;

	reg = PIMR_CH0_RX_DONE | PIMR_CH0_RX_FIFO_OVR | PIMR_CH0_DMA_ERR |
		PIMR_CH1_RX_DONE | PIMR_CH1_RX_FIFO_OVR | PIMR_CH1_DMA_ERR |
		PIMR_CH0_TX_OWNER_ERR | PIMR_CH0_RX_OWNER_ERR | PIMR_CH0_RX_RING_FULL |
		PIMR_CH1_TX_OWNER_ERR | PIMR_CH1_RX_OWNER_ERR | PIMR_CH1_RX_RING_FULL;
	write_reg_word(CR_PTM_PIMR(bearer), reg);
}

void ptmReset(int bearer)
{
	uint32 reg;

	/* Disable DMA Enable */
	reg = 0;
	write_reg_word (CR_PTM_PRCR(bearer), reg);
	pause(2);

	/* Reset PTM */
	reg = PRCT_HW_RESET;
	write_reg_word (CR_PTM_PRCR(bearer), reg);
	pause(10);
}

void ptmSetIntConf(int bearer)
{
	uint32 reg = 0;

#ifdef TC3262_PTM_NAPI
	reg = (1<<PPRCTR_RX_PKT_DONE_SHIFT) | 4;
#else
	reg = (4<<PPRCTR_RX_PKT_DONE_SHIFT) | 4;
#endif
	write_reg_word(CR_PTM_PPRCTR(bearer), reg);

	/* TX 128us timeout to interrupt */
	reg = 0xc80;
	write_reg_word(CR_PTM_PTIETR(bearer), reg);

	/* RX 32us timeout to interrupt */
	reg = 0x320;
	write_reg_word(CR_PTM_PRIETR0(bearer), reg);

	/* RX 32us timeout to interrupt */
	reg = 0x320;
	write_reg_word(CR_PTM_PRIETR1(bearer), reg);
}

// Assign Tx Rx Descriptor Control Registers
void ptmSetDMADescrCtrlReg(ptmAdapter_t *ptm_p)
{
	int txq;
	int bearer = ptm_p->bearer;

  	write_reg_word(CR_PTM_PTRBR(bearer), K1_TO_PHY(ptm_p->txDescrRingBaseVAddr[0]));
  	write_reg_word(CR_PTM_PRRBR(bearer), K1_TO_PHY(ptm_p->rxDescrRingBaseVAddr[0]));

  	write_reg_word(CR_PTM_PTRS01R(bearer), (ptm_p->txRingSize<<16)|ptm_p->txRingSize);
  	write_reg_word(CR_PTM_PTRS23R(bearer), (ptm_p->txRingSize<<16)|ptm_p->txRingSize);
  	write_reg_word(CR_PTM_PTRS45R(bearer), (ptm_p->txRingSize<<16)|ptm_p->txRingSize);
  	write_reg_word(CR_PTM_PTRS67R(bearer), (ptm_p->txRingSize<<16)|ptm_p->txRingSize);
  	write_reg_word(CR_PTM_PRRSR(bearer), (ptm_p->rxRingSize<<16)|ptm_p->rxRingSize);

	/* write tx/rx descriptor size in unit of word */
	write_reg_word(CR_PTM_PRDLR(bearer), ptm_p->rxDescrSize>>2);
	write_reg_word(CR_PTM_PTDLR(bearer), ptm_p->txDescrSize>>2);

	write_reg_word(CR_PTM_PRTPR(bearer, 0), 0x8000);
	write_reg_word(CR_PTM_PRTPR(bearer, 1), 0x8000);
	for (txq = 0; txq < TX_QUEUE_NUM; txq++)
		write_reg_word(CR_PTM_PTTPR(bearer, txq), 0);
}

void ptmSetPTMCR(int bearer)
{
	uint32 reg;

	reg = PFSR_RX_LONG | PFSR_RX_ALL | PFSR_RX_BCAST |
		PFSR_RX_MCAST;
#if !defined(TCSUPPORT_HWNAT)
	reg |= PFSR_RX_CRC_DIS | PFSR_RX_RUNT;
#endif
	write_reg_word(CR_PTM_PFSR(bearer), reg);

	/* set TX under run register to 1.5KB */
	write_reg_word(CR_PTM_PTLWR(bearer), 0x180);

	/* set RX max/min length */
	write_reg_word(CR_PTM_PRXLR(bearer), 0x40 | ((RX_MAX_PKT_LEN-4) << 16));

	/* set WRR weight and unit is packet */
	write_reg_word(CR_PTM_PWWR0(bearer), 0x1);
	write_reg_word(CR_PTM_PWWR1(bearer), 0x2);
	write_reg_word(CR_PTM_PWWR2(bearer), 0x4);
	write_reg_word(CR_PTM_PWWR3(bearer), 0x8);

	write_reg_word(CR_PTM_PTQCR(bearer), 0x50);
#if defined (QOS_REMARKING) || defined (TCSUPPORT_HW_QOS)
	qos_wrr_user = QOS_DMAWRR_USERDEFINE;
	if(*qos_wrr_info == 0) { /*strict priority*/
		reg = read_reg_word(CR_PTM_PTQCR(bearer));
		write_reg_word(CR_PTM_PTQCR(bearer), reg & ~(PTQCR_WRR_EN));
	}
	else {  /*WRR*/
		write_reg_word(CR_PTM_PWWR3(bearer), ((uint32)(qos_wrr_info[1] & 0x0f)));
		write_reg_word(CR_PTM_PWWR2(bearer), ((uint32)(qos_wrr_info[2] & 0x0f)));
		write_reg_word(CR_PTM_PWWR1(bearer), ((uint32)(qos_wrr_info[3] & 0x0f)));
		write_reg_word(CR_PTM_PWWR0(bearer), ((uint32)(qos_wrr_info[4] & 0x0f)));

		reg = read_reg_word(CR_PTM_PTQCR(bearer));
		reg |= (PTQCR_WRR_EN | PTQCR_WRR_SELECT);
		write_reg_word(CR_PTM_PTQCR(bearer), reg );
	}
#endif

	write_reg_word(CR_PTM_PRBSR(bearer), RX_MAX_PKT_LEN);
#ifdef TC3262_PTM_SG_MODE
	write_reg_word(CR_PTM_PSGCR(bearer), PSGCR_SG_EN | (SG_MAX_PKT_LEN & PSGCR_SG_PKT_LEN_MASK));
#endif
}

void ptmSetMacReg(ptmAdapter_t *ptm_p)
{
	int bearer = ptm_p->bearer;

	write_reg_word(CR_PTM_PMAR(bearer), ptm_p->macAddr[0]<<24 |
			ptm_p->macAddr[1]<<16 | ptm_p->macAddr[2]<<8  | ptm_p->macAddr[3]<<0);
	write_reg_word(CR_PTM_PMAR1(bearer), ptm_p->macAddr[4]<<8 |
			ptm_p->macAddr[5]<<0);
}

static uint8 isPriorityPkt(uint8 *cp, int *priority)
{
	uint16 etherType;
	uint8 ipVerLen;
	uint8 ipProtocol;
	uint8 tcpFlags;
	uint16 pppProtocol;

	/* skip DA and SA mac address */
	cp += 12;
	/* get ether type */
	etherType = *(uint16 *) cp;
	/* skip ether type */
	cp += 2;

	/*parse if vlan exists*/
	if (etherType == 0x8100) {
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
		if (etherType != 0x0800)
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

#define CONFIG_8021P_REMARK 1

#ifdef CONFIG_8021P_REMARK
#define QOS_8021p_MARK			0x0F00 	/* 8~11 bits used for 802.1p */
#define QOS_8021P_0_MARK		0x08	/* default mark is zero */
#define VLAN_HLEN			4
#define VLAN_ETH_ALEN			6


static inline struct sk_buff* vlanPriRemark(struct sk_buff *skb)
{
	uint8 encap_mode = 0, encap_len = 0;
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
		else if ( QOS_8021P_0_MARK == ucprio ) {	//zero mark
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
					printk(KERN_ERR, "Vlan:failed to realloc headroom\n");
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
			copy_len = 2*VLAN_ETH_ALEN;	// frank mark
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


ptmTxDescr_t *ptmTxRingProc(ptmAdapter_t *ptm_p, int txq, int force)
{
	volatile ptmTxDescr_t *pTxDescp;
	unsigned long flags;
	struct sk_buff *freeskb;

	spin_lock_irqsave(&ptm_p->lock, flags);
	pTxDescp = ((ptmTxDescr_t*)ptm_p->txDescrRingBaseVAddr[txq]) + ptm_p->txUnReleasedDescp[txq];

  	while (ptm_p->txUnReleasedBufCnt[txq] != 0) {
	  	if ((force == 0)&& pTxDescp->tdes0.bits.owner) {
			spin_unlock_irqrestore(&ptm_p->lock, flags);
			return 0; // owned by PTM engine, something wrong here!
		}

	  	if (pTxDescp->tdes0.bits.ur_abort || pTxDescp->tdes0.bits.txpkt_ur) {
			printk("ERR TX tx curr=%ld tx=%08lx\n", ptm_p->txUnReleasedDescp[txq], (uint32) pTxDescp);
			printk(" tdes0=%08lx\n", pTxDescp->tdes0.word);
			printk(" tdes1=%08lx\n", pTxDescp->tdes1.word);
			printk(" tdes2=%08lx\n", pTxDescp->tdes2.word);
			printk(" tdes3=%08lx\n", pTxDescp->tdes3.word);
		}

		if (ptm_p->statisticOn) {
			if ((pTxDescp->tdes0.word) & (1<<3|1<<2))
				ptm_p->ptmStat.inSilicon.txUnderRunCnt++;
		}

		freeskb = ptm_p->txskbs[txq][ptm_p->txUnReleasedDescp[txq]];

#ifdef LOOPBACK_SUPPORT
		if (LOOPBACK_MODE(ptmLoopback))
			dev_kfree_skb(freeskb);
		else
#endif
			dev_kfree_skb_any(freeskb);

		pTxDescp->tdes2.txbuf_addr = 0;
		ptm_p->txskbs[txq][ptm_p->txUnReleasedDescp[txq]] = NULL;

		if (ptm_p->txUnReleasedDescp[txq] == (ptm_p->txRingSize - 1))
			ptm_p->txUnReleasedDescp[txq] = 0;
		else
			ptm_p->txUnReleasedDescp[txq]++;
		ptm_p->txUnReleasedBufCnt[txq]--;

		pTxDescp = ((ptmTxDescr_t*)ptm_p->txDescrRingBaseVAddr[txq]) + ptm_p->txUnReleasedDescp[txq];
	}
	spin_unlock_irqrestore(&ptm_p->lock, flags);

	return (ptmTxDescr_t*) pTxDescp;
}

int tc3262_ptm_tx(struct sk_buff *skb, struct net_device *dev)
{
	ptmAdapter_t *ptm_p = netdev_priv(dev);
	volatile ptmTxDescr_t *currDescrp = NULL;
	uint32 length = skb->len;
	uint8 *bufAddrp = skb->data;
	unsigned long flags;
	uint32 reg;
	int txq = 0;
	int new_txq;

	#ifdef CONFIG_8021P_REMARK
	skb=vlanPriRemark(skb);
	if(skb==NULL){
		printk("802.1p remark failure\r\n");
		return 1;
	}
	#endif

#ifdef WAN2LAN
        if(masko){
            struct sk_buff *skb2 = NULL;

            //Check the skb headroom is enough or not. shnwind 20100121.
            if(skb_headroom(skb) < TX_STAG_LEN){
                skb2 = skb_copy_expand(skb, TX_STAG_LEN, skb_tailroom(skb) , GFP_ATOMIC);;
            }else{
                skb2 = skb_copy(skb,GFP_ATOMIC);
            }

            if(skb2 == NULL){
                printk("wan2lan failure in sar rx direction for skb2 allocate failure.\n");
            }
            else{
                skb2->mark |= SKBUF_COPYTOLAN;
                macSend(WAN2LAN_CH_ID,skb2);
            }
        }
#endif

#ifdef LOOPBACK_SUPPORT
	txq = skb->priority & PTM_PRIORITY_MASK;
#endif

	if (unlikely(skb->len < ETH_ZLEN)) {
		if (skb_padto(skb, ETH_ZLEN)) {
			ptm_p->ptmStat.MIB_II.outDiscards++;
			return NETDEV_TX_OK;
		}
		length = ETH_ZLEN;
	}

#ifdef LOOPBACK_SUPPORT
	if (ptmLoopback & LOOPBACK_PKT) {
		printk("TX: ");
		dump_skb(skb);
	}

	if (ptmLoopback & LOOPBACK_TX_RANDOM2) {
		txq = random32() & PTM_PRIORITY_MASK;
		/*
		if (ptmLoopback & LOOPBACK_TX_B0)
			bearer = PTM_B0;
		else if (ptmLoopback & LOOPBACK_TX_B1)
			bearer = PTM_B1;
		else
			bearer = random32() & PTM_BEARER_MASK;
		*/
	}
#endif

#ifdef TCSUPPORT_QOS
	switch (qos_flag) {
		case QOS_SW_PQ:
			/* PQ mode */
			if (txq < 2 && (skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				txq = 2;
			}
			else if (txq < 1 && (skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY) {
				txq = 1;
			}
			break;
	#if 1//def TCSUPPORT_HW_QOS
		case QOS_HW_WRR:
			/* HW WRR mode */
			if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				txq = 3;
			}
			else if ((skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY) {
				txq = 2;
			}
			else if ((skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY) {
				txq = 1;
			}
			else {
				txq = 0;
			}
			break;
		case QOS_HW_PQ:
			/* HW PQ mode */
			if (txq < 3 && (skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				txq = 3;
			}
			else if (txq < 2 && (skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY) {
				txq = 2;
			}
			else if (txq < 1 && (skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY) {
				txq = 1;
			}
			break;
		case NULLQOS: /*It's for putting rtp packets to HH priority when qos_flag not be selected as WRR or PQ*/
			if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				txq = 3;
			}
			break;
	#endif
		default:
			break;
	}
#endif

	#ifdef QOS_REMARKING
	if((skb->mark & QOS_REMARKING_FLAG)){
		txq = (uint8)((skb->mark & QOS_REMARKING_MASK) >> 1);
	}
	else
	#endif
	if (priPktChk && (skb->len < priPktChkLen)) {
		if (isPriorityPkt(skb->data, &new_txq)) {
			#ifdef TCSUPPORT_QOS
			if (qos_flag == QOS_HW_WRR) {
				/* hw wrr mode, to handle special packet */
				txq = max_prio;
			}
			else {
				if (new_txq > txq) {
					txq = new_txq;
				}
			}
			#else
			txq = new_txq;
			#endif
		}
	}

#if defined(TCSUPPORT_HWNAT)
	if (pktflow_tx_hook)
		pktflow_tx_hook(skb, 2 + ptm_p->bearer);
#endif

	if (ptm_p->txUnReleasedBufCnt[txq] >= TX_BUF_RELEASE_THRESHOLD)
    	ptmTxRingProc(ptm_p, txq, 0);

	if (ptm_p->txUnReleasedBufCnt[txq] == ptm_p->txRingSize) {
#ifdef LOOPBACK_SUPPORT
		//if (!(ptmLoopback & LOOPBACK_TX_NO_MSG))
		//	printk("b%d drop tx packet, ring is full PTTPR(%d)=%08lx PTHPR(%d)=%08lx\n", ptm_p->bearer, txq, VPint(CR_PTM_PTTPR(ptm_p->bearer, txq)), txq, VPint(CR_PTM_PTHPR(ptm_p->bearer, txq)));
#endif
		ptm_p->ptmStat.MIB_II.outDiscards++;
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	/* ----- Count the MIB-II ----- */
	ptm_p->ptmStat.MIB_II.outOctets += length;

	if (*bufAddrp & 0x01)
		ptm_p->ptmStat.MIB_II.outMulticastPkts++;
	else
		ptm_p->ptmStat.MIB_II.outUnicastPkts++;
	dma_cache_wback_inv((unsigned long)(skb->data), length);
	spin_lock_irqsave(&ptm_p->lock, flags);
	/* ----- Get the transmit descriptor ----- */
	currDescrp = ((ptmTxDescr_t*) ptm_p->txDescrRingBaseVAddr[txq]) + ptm_p->txCurrentDescp[txq];

	if (currDescrp->tdes0.word & (1<<31)) {
#ifdef LOOPBACK_SUPPORT
		if (!(ptmLoopback & LOOPBACK_TX_NO_MSG))
			printk("b%d drop tx packet, ring is full PTTPR(%d)=%08lx PTHPR(%d)=%08lx\n", ptm_p->bearer, txq, VPint(CR_PTM_PTTPR(ptm_p->bearer, txq)), txq, VPint(CR_PTM_PTHPR(ptm_p->bearer, txq)));
#endif
		ptm_p->ptmStat.MIB_II.outDiscards++;
		dev_kfree_skb_any(skb);
		spin_unlock_irqrestore(&ptm_p->lock, flags);
		return 0;
	}

	/* tx buffer size */
	currDescrp->tdes0.word = 0;
  	currDescrp->tdes1.word = (length & 0x000007ff);
#ifdef LOOPBACK_SUPPORT
	/* ip checksum calculation */
	if (ptmLoopback & LOOPBACK_TX_IPCS)
  		currDescrp->tdes1.bits.ipcs_ins = 1;
	/* vlan tag insertion */
	if (ptmLoopback & LOOPBACK_TX_VLAN) {
  		currDescrp->tdes1.bits.vlan_ins = 1;
  		currDescrp->tdes1.bits.vlan_tag = txq;
	}
#endif
  	currDescrp->tdes2.txbuf_addr = K1_TO_PHY(skb->data);
	ptm_p->txskbs[txq][ptm_p->txCurrentDescp[txq]] = skb;

	currDescrp->tdes0.word |= (1<<31);
	wmb();

	reg = read_reg_word(CR_PTM_PTTPR(ptm_p->bearer, txq));
	if (ptm_p->txCurrentDescp[txq] == (ptm_p->txRingSize - 1)) {
		ptm_p->txCurrentDescp[txq] = 0;
		reg = ((reg ^ 0x8000) & 0x8000) | ptm_p->txCurrentDescp[txq];
	} else {
		ptm_p->txCurrentDescp[txq]++;
		reg = (reg & 0x8000) | ptm_p->txCurrentDescp[txq];
	}
	write_reg_word(CR_PTM_PTTPR(ptm_p->bearer, txq), reg);

	ptm_p->txUnReleasedBufCnt[txq]++;

	spin_unlock_irqrestore(&ptm_p->lock, flags);

	return NETDEV_TX_OK;
}

#ifdef LOOPBACK_SUPPORT

static void dump_skb(struct sk_buff *skb)
{
	char tmp[80];
	char *p = skb->data;
	char *t = tmp;
	int i, n = 0;

	printk("ERR skb=%08lx data=%08lx len=%d\n", (uint32) skb, (uint32) skb->data, skb->len);
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

static void dump_data(char *p, int len)
{
	char tmp[80];
	char *t = tmp;
	int i, n = 0;

	printk("ERR data=%08lx len=%d\n", (uint32) p, len);
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

static uint32 get32(uint8 *cp)
{
	uint32 rval;

	rval = *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp;

	return rval;
}

static uint16 get16(uint8 *cp)
{
	uint16 x;

	x = *cp++;
	x <<= 8;
	x |= *cp;
	return x;
}

static uint8 *put32(uint8 *cp, uint32 x)
{
	*cp++ = x >> 24;
	*cp++ = x >> 16;
	*cp++ = x >> 8;
	*cp++ = x;
	return cp;
}

static uint8 * put16(uint8 *cp, uint16 x)
{
	*cp++ = x >> 8;
	*cp++ = x;

	return cp;
}

unsigned short in_csum(unsigned short *ptr, int nbytes)
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

#ifndef TCSUPPORT_AUTOBENCH
static
#endif
int tc3262_ptm_loopback_gen(struct net_device *dev, int npackets, int txlen)
{
	int i = 0, k, rxpackets;
	struct sk_buff *skb;
	int tx_len;
	uint8 *tx_data;
	uint8 tx_seed;
	int offset = 0;
	int tx_priority;
	int bearer;
	int rxq;
	unsigned short chksum;
	int ip_len = 5;
	int retry = 0;
	uint32 outDiscards;
	uint32 loopback_pkt_seq[PTM_BEARER_NUM][RX_QUEUE_NUM];
	uint16 loopback_pkt_len[PTM_BEARER_NUM][RX_QUEUE_NUM];
	uint16 loopback_pkt_last_byte[PTM_BEARER_NUM][RX_QUEUE_NUM];

	printk("Loopback test packets=%d txlen=%d\n", npackets, txlen);

	ptmLoopback |= LOOPBACK_TX_NO_MSG;
	priPktChk = 0;

	for (bearer = PTM_B0; bearer <= PTM_B1; bearer += PTM_B1) {
		for (rxq = 0; rxq < RX_QUEUE_NUM; rxq++) {
			loopback_pkt_seq[bearer][rxq] = 0;
			loopback_pkt_len[bearer][rxq] = 0;
			loopback_pkt_last_byte[bearer][rxq] = 0;
		}
	}

	atomic_set(&ptmRxLoopback, 0);
	while (i < npackets) {
		skb = dev_alloc_skb(RX_BUF_LEN);
		if (skb == NULL)
			continue;

#ifndef TCSUPPORT_AUTOBENCH
		if ((i % 4096) == 0)
			printk("Gen %d packets.\n", i);

#endif
		offset =  random32();
		if (offset & 0x1)
			offset = 2;
		else
			offset = 0;

		skb_reserve(skb, offset);

		if (txlen == 0) {
			tx_len = random32();
			tx_len = tx_len % 1515;
			if (tx_len > 1514)
				tx_len = 1514;
			/*
			if (tx_len < 34)
				tx_len = 34;
			*/
			if (tx_len < 60)
				tx_len = 60;
		} else {
			tx_len = txlen;
		}

		tx_data = skb_put(skb, tx_len);

		if (ptmLoopback & LOOPBACK_TX_RANDOM) {
			tx_priority = random32() & PTM_PRIORITY_MASK;

			if (ptmLoopback & LOOPBACK_TX_F_P0)
				tx_priority = 0;
			else if (ptmLoopback & LOOPBACK_TX_F_P7)
				tx_priority = 7;

			if (ptmLoopback & LOOPBACK_TX_B0)
				bearer = PTM_B0;
			else if (ptmLoopback & LOOPBACK_TX_B1)
				bearer = PTM_B1;
			else
				bearer = random32() & PTM_BEARER_MASK;
		} else {
			tx_priority = 0;
			bearer = 0;
		}

		rxq = (VPint(CR_PTM_TQER(bearer)) & (1<<tx_priority)) ? PTM_CH1 : PTM_CH0;

		for (k = 0; k < 6; k++)
			if (k & 0x1)
				tx_data[k] = 0x11;
			else
				tx_data[k] = 0x22;
		for (k = 6; k < 12; k++)
			if (k & 0x1)
				tx_data[k] = 0x11;
			else
				tx_data[k] = 0x22;

		put16(&tx_data[2], loopback_pkt_len[bearer][rxq]);
		put32(&tx_data[4], tx_len);
		put32(&tx_data[8], i);

		tx_data[12] = 0x08;
		tx_data[13] = 0x00;
		tx_seed = (uint8) random32();

		skb->priority = tx_priority | (bearer << PTM_BEARER_SHIFT);

		if (tx_len < (ip_len * 4 + 14))
			ip_len = (tx_len - 14) >> 2;

		tx_data[14] = 0x40 | ip_len;
		tx_data[15] = loopback_pkt_last_byte[bearer][rxq];
		tx_data[16] = bearer;
		tx_data[17] = rxq;
		tx_data[18] = tx_priority;

		for (k = 19; k < tx_len; k++)
			tx_data[k] = (uint8) (tx_seed++ & 0xff);

		tx_data[24] = 0;
		tx_data[25] = 0;

		chksum = in_csum((unsigned short *) (skb->data + 14), ip_len << 2);
		tx_data[24] = (chksum >> 8) & 0xff;
		tx_data[25] = chksum & 0xff;

		ip_len++;
		if (ip_len > 15)
			ip_len = 5;

		outDiscards = ptm_p[bearer]->ptmStat.MIB_II.outDiscards;
		tc3262_ptm_tx(skb, tc3262_ptm_dev[bearer]);
		if (outDiscards == ptm_p[bearer]->ptmStat.MIB_II.outDiscards) {
			loopback_pkt_seq[bearer][rxq]++;
			loopback_pkt_len[bearer][rxq] = (uint16) tx_len;
			tx_seed--;
			loopback_pkt_last_byte[bearer][rxq] = tx_seed;
			i++;
			retry = 0;
		} else {
			mdelay(1);
			retry++;
		}
#ifdef TCSUPPORT_AUTOBENCH
		mdelay(3);
#endif
		if (retry > 5000)
			ptmLoopback &= ~LOOPBACK_TX_NO_MSG;
		if (retry > 5010)
			break;
		/*
		if (retry > 50000)
			ptmLoopback &= ~LOOPBACK_TX_NO_MSG;
		if (retry > 50100)
			break;
		*/
	}
#ifdef TCSUPPORT_AUTOBENCH
	printk("Gen %d packets done.\n", i);
	mdelay(2000);
	i = atomic_read(&ptmRxLoopback);
	rxpackets = i;
#else
	printk("Gen %d packets done.\n", i);
	mdelay(5000);

	i = atomic_read(&ptmRxLoopback);
	rxpackets = i;
	k = 0;
	while (1) {
		mdelay(1000);
		k++;
		if (k > 30)
			break;
		if (rxpackets != atomic_read(&ptmRxLoopback)) {
			rxpackets = atomic_read(&ptmRxLoopback);
			k = 0;
		}
		if (i == atomic_read(&ptmRxLoopback))
			break;
	}

#endif
	printk("Chk %d packets done.\n", atomic_read(&ptmRxLoopback));

	if (atomic_read(&ptmRxLoopback) != npackets){
		printk("ERR TX/RX packet number mismatch.\n");

#ifdef TCSUPPORT_AUTOBENCH
		return -1;
#endif
	}
	return 0;
}

#ifdef TCSUPPORT_AUTOBENCH

int set_loopback_mode(int mode){
	ptmLoopback = mode;
}
EXPORT_SYMBOL(tc3262_ptm_loopback_gen);
EXPORT_SYMBOL(set_loopback_mode);
#endif
static int tc3262_ptm_loopback_chk(struct sk_buff *skb, struct net_device *dev, int bearer, int rxq)
{
	int k;
	uint8 *tx_data;
	uint8 tx_seed;
	int ip_len;
	uint32 pkt_seq;
	uint32 pkt_len;
	uint16 pre_pkt_len;
	int pkt_bearer, pkt_rxq;

	atomic_add(1, &ptmRxLoopback);

	tx_data = skb->data;

	pre_pkt_len = get16(&tx_data[2]);
	pkt_len = get32(&tx_data[4]);
	pkt_seq = get32(&tx_data[8]);

	if (pkt_len != skb->len) {
		printk("loopback fail: B%d RXQ%d len unmatch len=%d pkt_len=%d\n", bearer, rxq, skb->len, (int) pkt_len);
		goto err;
	}

	pkt_bearer = tx_data[16];
	pkt_rxq = tx_data[17];

	if (pkt_bearer != bearer) {
		printk("loopback fail: B%d RXQ%d bearer unmatch bearer=%d chk_bearer=%d\n", bearer, rxq, pkt_bearer, bearer);
		goto err;
	}

	if (pkt_rxq != rxq) {
		printk("loopback fail: B%d RXQ%d rxq unmatch rxq=%d chk_rxq=%d\n", bearer, rxq, pkt_rxq, rxq);
		goto err;
	}

	if ((tx_data[12] != 0x08) || (tx_data[13] != 0x00)) {
		printk("loopback fail: B%d RXQ%d ether type unmatch\n", bearer, rxq);
		goto err;
	}

	if ((tx_data[14] & 0xf0) != 0x40) {
		printk("loopback fail: B%d RXQ%d ip header unmatch\n", bearer, rxq);
		goto err;
	}
	ip_len = (tx_data[14] & 0x0f) << 2;
	tx_seed = tx_data[19];

	for (k = 19; k < skb->len; k++) {
		if ((k != 24) && (k != 25)) {
			if (tx_data[k] != (uint8) (tx_seed++ & 0xff)) {
				printk("loopback fail: B%d RXQ%d payload unmatch pos=%04x\n", bearer, rxq, k);
				goto err;
			}
		} else {
			tx_seed++;
		}
	}

	if (in_csum((unsigned short *) (skb->data + 14), ip_len) != 0) {
		printk("loopback fail: B%d RXQ%d ip checksum unmatch\n", bearer, rxq);
		goto err;
	}

	dev_kfree_skb(skb);
	return 0;

err:
	printk("%d packet\n", atomic_read(&ptmRxLoopback));
	dump_skb(skb);
	dev_kfree_skb(skb);
	return 1;
}

static int tc3262_ptm_qos_gen(struct net_device *dev)
{
	ptmAdapter_t *ptm_p = netdev_priv(dev);
	int bearer = ptm_p->bearer;
	int i, k;
	struct sk_buff *skb;
	int tx_len;
	uint8 *tx_data;
	uint8 tx_seed;
	uint8 tx_queue;
	int txq;

	ptmDrvStop(bearer);

	for (txq = TX_QUEUE_NUM - 1; txq >= 0; txq--) {
		for (i = 0; i < PTM_TXDESCP_NO; i++) {
			skb = dev_alloc_skb(RX_BUF_LEN);
			if (skb == NULL)
				continue;

			skb->priority = txq;

			//tx_len = random32();
			tx_len = 60;
			tx_len = tx_len % 1514;
			if (tx_len > 1514)
				tx_len = 1514;
			if (tx_len < 60)
				tx_len = 60;

			tx_data = skb_put(skb, tx_len);
			for (k = 0; k < 6; k++) {
				if (k != 5)
					tx_data[k] = 0x10;
				else
					tx_data[k] = 0x10 + txq;
			}
			for (k = 6; k < 12; k++)
				tx_data[k] = 0x22;

			tx_data[12] = 0x08;
			tx_data[13] = 0x01;

			tx_seed = (uint8) random32();
			tx_queue = (uint8) random32();

			tx_data[14] = tx_seed;
			tx_data[15] = tx_queue;

			for (k = 16; k < tx_len; k++)
				tx_data[k] = (uint8) (tx_seed++ & 0xff);

			tc3262_ptm_tx(skb, dev);
		}
	}

	ptmDrvStart(bearer);

	return 0;
}

#endif

void ptmDefaultParaSet(ptmAdapter_t *ptm_p)
{
	ptm_p->rxDescrSize = PTM_RXDESCP_SIZE;
	ptm_p->txDescrSize = PTM_TXDESCP_SIZE;
	ptm_p->rxRingSize  = PTM_RXDESCP_NO;
	ptm_p->txRingSize  = PTM_TXDESCP_NO;
}

int ptmDrvRegInit(ptmAdapter_t *ptm_p)
{
	int bearer = ptm_p->bearer;

	ptmReset(bearer);

    ptmIMRMask(bearer);

    ptmSetIntConf(bearer);

    ptmSetDMADescrCtrlReg(ptm_p);

    ptmSetPTMCR(bearer);

    // --- setup MAC address ---
    ptmSetMacReg(ptm_p);

#if defined(TCSUPPORT_HWNAT)
	/* enable hwnat */
	if (isRT65168) {
		VPint(CR_PTM_PFSR(bearer)) |= PFSR_TXPAD_EN | PFSR_HIGH_SPD_DROP | PFSR_NAT_EN;
	}
#endif

    return 0;
}

void ptmDrvDescripReset(ptmAdapter_t *ptm_p)
{
	ptmRxDescr_t *pRxDescp;
	ptmTxDescr_t *pTxDescp;
	struct sk_buff *skb;
#ifdef TC3262_PTM_SG_MODE
	struct sk_buff *sg_skb;
#endif
	int i;
	int txq, rxq;

	for (rxq = 0; rxq < RX_QUEUE_NUM; rxq++) {
		pRxDescp = (ptmRxDescr_t*) ptm_p->rxDescrRingBaseVAddr[rxq];
		for (i = 0; i < ptm_p->rxRingSize; i++) {
			skb = ptm_p->rxskbs[rxq][i];
			if (skb != NULL)
				dev_kfree_skb_any(skb);
#ifdef TC3262_PTM_SG_MODE
			sg_skb = ptm_p->sg_rxskbs[rxq][i];
			if (sg_skb != NULL)
				dev_kfree_skb_any(sg_skb);
#endif
			// Init Descriptor
			pRxDescp->rdes0.word = 0;
			pRxDescp->rdes1.word = 0;
			pRxDescp->rdes2.word = 0;
			pRxDescp->rdes3.word = 0;
			pRxDescp->rdes4.word = 0;
			pRxDescp->rdes5.word = 0;
			pRxDescp->rdes6.word = 0;
			pRxDescp->rdes7.word = 0;
			ptm_p->rxskbs[rxq][i] = NULL;
#ifdef TC3262_PTM_SG_MODE
			ptm_p->sg_rxskbs[rxq][i] = NULL;
#endif
			pRxDescp++;
		}
	}

	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		pTxDescp = (ptmTxDescr_t*) ptm_p->txDescrRingBaseVAddr[txq];
		// free all un-released tx mbuf
		for (i = 0 ; i < ptm_p->txRingSize ; i++) {
			skb = ptm_p->txskbs[txq][i];
			if (skb != NULL)
				dev_kfree_skb_any(skb);
			pTxDescp->tdes0.word = 0;
			pTxDescp->tdes1.word = 0;
			pTxDescp->tdes2.word = 0;
			pTxDescp->tdes3.word = 0;
			ptm_p->txskbs[txq][i] = NULL;
			pTxDescp++;
		}
	}

	for (rxq = 0; rxq < RX_QUEUE_NUM; rxq++)
		ptm_p->rxCurrentDescp[rxq] = 0;
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		ptm_p->txCurrentDescp[txq] = 0;
		ptm_p->txUnReleasedDescp[txq] = 0;
		ptm_p->txUnReleasedBufCnt[txq] = 0;
	}
}

uint8 ptmDrvDescripInit(ptmAdapter_t *ptm_p)
{
	ptmRxDescr_t *pRxDescp;
  	ptmTxDescr_t *pTxDescp;
  	int i, txq, rxq;
  	struct sk_buff *skb;
#ifdef TC3262_PTM_SG_MODE
  	struct sk_buff *sg_skb;
#endif

	for (rxq = 0; rxq < RX_QUEUE_NUM; rxq++) {
		ptm_p->rxDescrRingBaseVAddr[rxq] = (uint32) &ptm_p->ptmRxMemPool_p->rxDescpBuf[rxq][0];
	}
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		ptm_p->txDescrRingBaseVAddr[txq] = (uint32) &ptm_p->ptmTxMemPool_p->txDescpBuf[txq][0];
	}

	if (ptmInitialized[ptm_p->bearer])
		ptmDrvDescripReset(ptm_p);

	/* init. Rx descriptor, allocate memory for each descriptor */
	for (rxq = 0; rxq < RX_QUEUE_NUM; rxq++) {
		pRxDescp = (ptmRxDescr_t*) ptm_p->rxDescrRingBaseVAddr[rxq];
		for (i = 0 ; i< ptm_p->rxRingSize ; i++, pRxDescp++) {
			// Init Descriptor
			pRxDescp->rdes0.word = 0;
			pRxDescp->rdes1.word = 0;
			pRxDescp->rdes2.word = 0;
			pRxDescp->rdes3.word = 0;
			pRxDescp->rdes4.word = 0;
			pRxDescp->rdes5.word = 0;
			pRxDescp->rdes6.word = 0;
			pRxDescp->rdes7.word = 0;

			// Assign flag
			pRxDescp->rdes0.bits.owner = 1;  /* owned by DMA */
			pRxDescp->rdes1.bits.rx_buf_size = RX_MAX_PKT_LEN;

			skb = ptm_alloc_skb2k();
			if (skb == NULL) {
				printk("tc3262_ptm_descinit init fail.\n");
				return 1;
			}
			dma_cache_inv((unsigned long)(skb->data), RX_MAX_PKT_LEN);
			skb_reserve(skb, NET_IP_ALIGN);

			pRxDescp->rdes2.rxbuf_addr = K1_TO_PHY(skb->data);
			ptm_p->rxskbs[rxq][i] = skb;

#ifdef TC3262_PTM_SG_MODE
			sg_skb = ptm_alloc_skb128();
			if (sg_skb == NULL) {
				printk("tc3262_ptm_descinit init fail.\n");
				return 1;
			}
			dma_cache_inv((unsigned long)(sg_skb->data), SG_MAX_PKT_LEN);
			skb_reserve(sg_skb, NET_IP_ALIGN);

			pRxDescp->rdes3.rxhdr_addr = K1_TO_PHY(sg_skb->data);
			ptm_p->sg_rxskbs[rxq][i] = sg_skb;
#endif
		}
	}

	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		/* init. tx descriptor, don't allocate memory */
		pTxDescp = (ptmTxDescr_t*) ptm_p->txDescrRingBaseVAddr[txq];
		for (i = 0 ; i < ptm_p->txRingSize ; i++, pTxDescp++) {
    		// Init descriptor
    		pTxDescp->tdes0.word = 0;
    		pTxDescp->tdes1.word = 0;
    		pTxDescp->tdes2.word = 0;
    		pTxDescp->tdes3.word = 0;
			ptm_p->txskbs[txq][i] = NULL;
		}
	}

	for (rxq = 0; rxq < RX_QUEUE_NUM; rxq++)
		ptm_p->rxCurrentDescp[rxq] = 0;
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		ptm_p->txCurrentDescp[txq] = 0;
		ptm_p->txUnReleasedDescp[txq] = 0;
		ptm_p->txUnReleasedBufCnt[txq] = 0;
	}

	return 0;
}

void ptmDrvStart(int bearer)
{
	uint32 reg;

	reg = read_reg_word(CR_PTM_PDER(bearer));
	/* enable DMA tx& rx */
	reg |= PDER_CH0_RX_DMA_EN;
	reg |= PDER_CH0_TX_DMA_EN;
	reg |= PDER_CH1_RX_DMA_EN;
	reg |= PDER_CH1_TX_DMA_EN;
	write_reg_word(CR_PTM_PDER(bearer), reg);
}

#if (defined(TCSUPPORT_WAN_ETHER) || defined(TCSUPPORT_WAN_PTM)) && defined(TCSUPPORT_MULTISERVICE_ON_WAN)
void ptmTxDrvStart(int bearer)
{
	uint32 reg;

	reg = read_reg_word(CR_PTM_PDER(bearer));
	/* enable DMA tx */
	reg |= PDER_CH0_TX_DMA_EN;
	reg |= PDER_CH1_TX_DMA_EN;
	write_reg_word(CR_PTM_PDER(bearer), reg);
}
#endif

static inline int ptmRxRingChk(int bearer)
{
	ptmRxDescr_t *rxDescrp;

	/* check high priority queue first */
	rxDescrp = ((ptmRxDescr_t *)ptm_p[bearer]->rxDescrRingBaseVAddr[PTM_CH1]) + ptm_p[bearer]->rxCurrentDescp[PTM_CH1];
  	if ((rxDescrp->rdes0.bits.owner == 0))
		return PTM_CH1;

	/* then check low priority queue */
	rxDescrp = ((ptmRxDescr_t *)ptm_p[bearer]->rxDescrRingBaseVAddr[PTM_CH0]) + ptm_p[bearer]->rxCurrentDescp[PTM_CH0];
  	if ((rxDescrp->rdes0.bits.owner == 0))
		return PTM_CH0;

	return -1;
}

int ptmRxRingProc(struct net_device *dev, int quota)
{
	ptmAdapter_t *ptm_p = netdev_priv(dev);
	int bearer = ptm_p->bearer;
	volatile ptmRxDescr_t *rxDescrp;
	uint32 frameSize;
	struct sk_buff *newskb, *skb;
	uint32 rdes0_error_check, rdes1_error_check;
	uint32 reg;
	int npackets = 0;
	int rxq;

#ifdef LOOPBACK_SUPPORT
	if (ptmLoopback & LOOPBACK_TX_IPCS)
		rdes0_error_check = (1<<27|1<<22|1<<21|1<<20|1<<19|1<<18);
	else
#endif
		rdes0_error_check = (1<<22|1<<21|1<<20|1<<19|1<<18);
  	rdes1_error_check = (1<<28|1<<27);

#ifdef TC3262_PTM_NAPI
  	while (npackets <= quota) {
#else
  	while (1) {
#endif
		rxq = ptmRxRingChk(bearer);
		if (rxq < 0)
			break;

		rxDescrp = ((ptmRxDescr_t*)ptm_p->rxDescrRingBaseVAddr[rxq]) + ptm_p->rxCurrentDescp[rxq];

		npackets++;
#ifdef LOOPBACK_SUPPORT
		if (ptmLoopback & LOOPBACK_MSG) {
			printk("\t\trx b%d rxq=%d curr=%ld rx=%08lx\n", bearer, rxq, ptm_p->rxCurrentDescp[rxq], (uint32) rxDescrp);
			printk("\t\t rdes0=%08lx\n", rxDescrp->rdes0.word);
			printk("\t\t rdes1=%08lx\n", rxDescrp->rdes1.word);
			printk("\t\t rdes2=%08lx\n", rxDescrp->rdes2.word);
			printk("\t\t rdes3=%08lx\n", rxDescrp->rdes3.word);
			printk("\t\t rdes4=%08lx\n", rxDescrp->rdes4.word);
			printk("\t\t rdes5=%08lx\n", rxDescrp->rdes5.word);
			printk("\t\t rdes6=%08lx\n", rxDescrp->rdes6.word);
			printk("\t\t rdes7=%08lx\n", rxDescrp->rdes7.word);
		}
#endif
	    if ((rxDescrp->rdes0.word & rdes0_error_check) == 0 &&
        	(rxDescrp->rdes1.word & rdes1_error_check) == 0) {
	    	// For removing Rx VLAN tag, we need to check if the remain length
	    	// is not smaller than minimun packet length.
	    	// rxDescrp->rdes0.bits.vlan_hit
	    	if (rxDescrp->rdes0.word & (1<<30)) {
	      		frameSize = rxDescrp->rdes0.bits.rx_length;
	      		frameSize = (frameSize > 60) ? frameSize : 60;
	      	} else {
 	 			frameSize = rxDescrp->rdes0.bits.rx_length;
			}

	      	if (unlikely((frameSize < 34) || (frameSize > 1522))) {
	        	ptm_p->ptmStat.inSilicon.rxEtherFrameLengthErr++;

	        	// Discard this packet & Repost this mbuf
#ifdef TC3262_PTM_SG_MODE
				if (frameSize <= SG_MAX_PKT_LEN)
					newskb = ptm_p->sg_rxskbs[rxq][ptm_p->rxCurrentDescp[rxq]];
				else
					newskb = ptm_p->rxskbs[rxq][ptm_p->rxCurrentDescp[rxq]];
#else
				newskb = ptm_p->rxskbs[rxq][ptm_p->rxCurrentDescp[rxq]];
#endif
	        	goto DISCARD;
	      	}

	      	// ----- Count the MIB-II -----
		    if (ptm_p->statisticOn) {
	  	    	ptm_p->ptmStat.MIB_II.inOctets += frameSize;

			    if ((rxDescrp->rdes0.word) & (1<<16))
			    	ptm_p->ptmStat.MIB_II.inMulticastPkts++;
			    else
		        	ptm_p->ptmStat.MIB_II.inUnicastPkts++;
		    }

#ifdef TC3262_PTM_SG_MODE
			if (frameSize <= SG_MAX_PKT_LEN) {
				skb = ptm_p->sg_rxskbs[rxq][ptm_p->rxCurrentDescp[rxq]];

				newskb = ptm_alloc_skb128();
	        	if (unlikely(!newskb)) { /* faild to allocate more mbuf -> drop this pkt */
	        		newskb = skb;
	          		ptm_p->ptmStat.MIB_II.inDiscards++;
	          		goto RECVOK;
	      		}
				dma_cache_inv((unsigned long)(newskb->data), SG_MAX_PKT_LEN);
				skb_reserve(newskb, NET_IP_ALIGN);
			} else {
				skb = ptm_p->rxskbs[rxq][ptm_p->rxCurrentDescp[rxq]];

				newskb = ptm_alloc_skb2k();
	        	if (unlikely(!newskb)) { /* faild to allocate more mbuf -> drop this pkt */
	        		newskb = skb;
	          		ptm_p->ptmStat.MIB_II.inDiscards++;
	          		goto RECVOK;
	      		}
				dma_cache_inv((unsigned long)(newskb->data), RX_MAX_PKT_LEN);
				skb_reserve(newskb, NET_IP_ALIGN);
			}
#else
			skb = ptm_p->rxskbs[rxq][ptm_p->rxCurrentDescp[rxq]];

			newskb = ptm_alloc_skb2k();
        	if (unlikely(!newskb)) { /* faild to allocate more mbuf -> drop this pkt */
        		newskb = skb;
          		ptm_p->ptmStat.MIB_II.inDiscards++;
          		goto RECVOK;
      		}

			dma_cache_inv((unsigned long)(newskb->data), RX_MAX_PKT_LEN);
			skb_reserve(newskb, NET_IP_ALIGN);
#endif

			skb_put(skb, frameSize);
#ifdef WAN2LAN
        if(masko){
            struct sk_buff *skb2 = NULL;

            //Check the skb headroom is enough or not. shnwind 20100121.
            if(skb_headroom(skb) < TX_STAG_LEN){
                skb2 = skb_copy_expand(skb, TX_STAG_LEN, skb_tailroom(skb) , GFP_ATOMIC);;
            }else{
                skb2 = skb_copy(skb,GFP_ATOMIC);
            }

            if(skb2 == NULL){
                printk("wan2lan failure in sar rx direction for skb2 allocate failure.\n");
            }
            else{
                skb2->mark |= SKBUF_COPYTOLAN;
                macSend(WAN2LAN_CH_ID,skb2);
            }
        }
#endif

#ifdef LOOPBACK_SUPPORT
			if (skb_shinfo(skb)->nr_frags) {
				int i;

				printk("PTM nr_frags=%ld %lx\n", (uint32) skb_shinfo(skb)->nr_frags, (uint32) skb_shinfo(skb)->nr_frags);
				for (i = 0; i < 16; i++)
					printk("page%d=%lx\n", i, (uint32) skb_shinfo(skb)->frags[i].page);
				printk("ERR skb=%08lx data=%08lx len=%d\n", (uint32) skb, (uint32) skb->data, skb->len);
				printk("\t\tERR rx B%d rxq=%d curr=%ld rx=%08lx\n", bearer, rxq, ptm_p->rxCurrentDescp[rxq], (uint32) rxDescrp);
				printk("\t\t rdes0=%08lx\n", rxDescrp->rdes0.word);
				printk("\t\t rdes1=%08lx\n", rxDescrp->rdes1.word);
				printk("\t\t rdes2=%08lx\n", rxDescrp->rdes2.word);
				printk("\t\t rdes3=%08lx\n", rxDescrp->rdes3.word);
				printk("\t\t rdes4=%08lx\n", rxDescrp->rdes4.word);
				printk("\t\t rdes5=%08lx\n", rxDescrp->rdes5.word);
				printk("\t\t rdes6=%08lx\n", rxDescrp->rdes6.word);
				printk("\t\t rdes7=%08lx\n", rxDescrp->rdes7.word);
				dump_data(skb->data, 2048);
				dump_data(UNCAC_ADDR(skb->data), 2048);
			}

			if (ptmLoopback & LOOPBACK_PKT) {
				printk("RX: ");
				dump_skb(skb);
			}

			if (LOOPBACK_MODE(ptmLoopback) == LOOPBACK_TX) {
				tc3262_ptm_tx(skb, dev);
			} else if (LOOPBACK_MODE(ptmLoopback) == LOOPBACK_RX_DROP) {
				dev_kfree_skb_any(skb);
			} else if (LOOPBACK_MODE(ptmLoopback) == LOOPBACK_RX_CHK) {
				tc3262_ptm_loopback_chk(skb, dev, bearer, rxq);
			} else
#endif
			{
				skb->dev = dev;

			        /* TBS_TAG: by WuGuoxiang 2012-7-11 Desc: Added for QoS */
			        skb->mark = 0x100000UL;

#if defined(TCSUPPORT_HWNAT)
				if (pktflow_rx_hook) {
					if (pktflow_rx_hook(skb, 2 + bearer, &rxDescrp->rdes3.word))
						goto RECVOK;
				}
#endif

				skb->ip_summed = CHECKSUM_NONE;
				skb->protocol = eth_type_trans(skb, dev);
				dev->last_rx = jiffies;
#ifdef TC3262_PTM_NAPI
				netif_receive_skb(skb);
#else
				netif_rx(skb);
#endif
			}
DISCARD:

RECVOK:

#ifdef TC3262_PTM_SG_MODE
			if (frameSize <= SG_MAX_PKT_LEN) {
	      		rxDescrp->rdes3.rxhdr_addr = K1_TO_PHY(newskb->data);
				ptm_p->sg_rxskbs[rxq][ptm_p->rxCurrentDescp[rxq]] = newskb;
			} else {
	      		rxDescrp->rdes2.rxbuf_addr = K1_TO_PHY(newskb->data);
				ptm_p->rxskbs[rxq][ptm_p->rxCurrentDescp[rxq]] = newskb;
			}
#else
	      	rxDescrp->rdes2.rxbuf_addr = K1_TO_PHY(newskb->data);
			ptm_p->rxskbs[rxq][ptm_p->rxCurrentDescp[rxq]] = newskb;
#endif
		} else { /* Update Error Counter and Drop it */
#ifdef LOOPBACK_SUPPORT
			printk("\t\tERR rx B%d rxq=%d curr=%ld rx=%08lx\n", bearer, rxq, ptm_p->rxCurrentDescp[rxq], (uint32) rxDescrp);
			printk("\t\t rdes0=%08lx\n", rxDescrp->rdes0.word);
			printk("\t\t rdes1=%08lx\n", rxDescrp->rdes1.word);
			printk("\t\t rdes2=%08lx\n", rxDescrp->rdes2.word);
			printk("\t\t rdes3=%08lx\n", rxDescrp->rdes3.word);
			printk("\t\t rdes4=%08lx\n", rxDescrp->rdes4.word);
			printk("\t\t rdes5=%08lx\n", rxDescrp->rdes5.word);
			printk("\t\t rdes6=%08lx\n", rxDescrp->rdes6.word);
			printk("\t\t rdes7=%08lx\n", rxDescrp->rdes7.word);
			printk("\t\t CR_PTM_PRCEPC(0)=%08lx\n", VPint(CR_PTM_PRCEPC(0)));
			printk("\t\t CR_PTM_PRCEPC(1)=%08lx\n", VPint(CR_PTM_PRCEPC(1)));

			skb = ptm_p->rxskbs[rxq][ptm_p->rxCurrentDescp[rxq]];
			skb->len = rxDescrp->rdes0.bits.rx_length;
			dump_skb(skb);
			skb->len = 0;
#endif

	      	if (ptm_p->statisticOn) {
		  		if (rxDescrp->rdes1.word & (1<<28)) {
					ptm_p->ptmStat.inSilicon.rx802p3FrameLengthErr++;
					ptm_p->ptmStat.MIB_II.inErrors++;
				}
		  		if (rxDescrp->rdes1.word & (1<<27)) {
					ptm_p->ptmStat.inSilicon.rxAlignErr++;
					ptm_p->ptmStat.MIB_II.inErrors++;
				}
				if (rxDescrp->rdes0.word & (1<<27)) {
					ptm_p->ptmStat.inSilicon.rxPktIPChkSumErr++;
					ptm_p->ptmStat.MIB_II.inErrors++;
				}
	        	if (rxDescrp->rdes0.word & (1<<22)) {
		        	ptm_p->ptmStat.inSilicon.rxCollisionErr++;
		          	ptm_p->ptmStat.MIB_II.inErrors++;
		        }
		        if (rxDescrp->rdes0.word & (1<<21)) {
					ptm_p->ptmStat.inSilicon.rxRuntErr++;
					ptm_p->ptmStat.MIB_II.inErrors++;
				}
		        if (rxDescrp->rdes0.word & (1<<20)) {
					ptm_p->ptmStat.inSilicon.rxLongErr++;
					ptm_p->ptmStat.MIB_II.inErrors++;
				}
		        if (rxDescrp->rdes0.word & (1<<19)) {
					ptm_p->ptmStat.inSilicon.rxCrcErr++;
					ptm_p->ptmStat.MIB_II.inErrors++;
				}
		        if (rxDescrp->rdes0.word & (1<<18)) {
					ptm_p->ptmStat.inSilicon.rxSymbolErr++;
					ptm_p->ptmStat.MIB_II.inErrors++;
				}
			}
		}

	    rxDescrp->rdes0.word |= (1<<31);
		wmb();

 		/* next descriptor*/
		reg = read_reg_word(CR_PTM_PRTPR(bearer, rxq));
		if ( ptm_p->rxCurrentDescp[rxq] == (ptm_p->rxRingSize - 1)) {
			ptm_p->rxCurrentDescp[rxq] = 0;
			reg = ((reg ^ 0x8000) & 0x8000) | ptm_p->rxCurrentDescp[rxq];
		} else {
			ptm_p->rxCurrentDescp[rxq]++;
			reg = (reg & 0x8000) | ptm_p->rxCurrentDescp[rxq];
		}
		write_reg_word(CR_PTM_PRTPR(bearer, rxq), reg);
		/* process one packet */
		write_reg_word(CR_PTM_PRPPCR(bearer, rxq), 1);

		rxDescrp = ((ptmRxDescr_t*)ptm_p->rxDescrRingBaseVAddr[rxq]) + ptm_p->rxCurrentDescp[rxq];
	}

#ifdef LOOPBACK_SUPPORT
	if (ptmLoopback & LOOPBACK_MSG)
		printk("npackets=%d\n", npackets);
#endif

	return npackets;
}

void ptmDrvStop(int bearer)
{
	uint32 reg;

	reg = read_reg_word(CR_PTM_PDER(bearer));
	/* disable DMA tx& rx */
	reg &= ~PDER_CH0_RX_DMA_EN;
	reg &= ~PDER_CH0_TX_DMA_EN;
	reg &= ~PDER_CH1_RX_DMA_EN;
	reg &= ~PDER_CH1_TX_DMA_EN;
	write_reg_word(CR_PTM_PDER(bearer), reg);
}

#if (defined(TCSUPPORT_WAN_ETHER) || defined(TCSUPPORT_WAN_PTM)) && defined(TCSUPPORT_MULTISERVICE_ON_WAN)
void ptmTxDrvStop(int bearer)
{
	uint32 reg;

	reg = read_reg_word(CR_PTM_PDER(bearer));
	/* disable DMA tx& rx */
	reg &= ~PDER_CH0_TX_DMA_EN;
	reg &= ~PDER_CH1_TX_DMA_EN;
	write_reg_word(CR_PTM_PDER(bearer), reg);
}
#endif

void ptmGetMacAddr(ptmAdapter_t *ptm_p)
{
	uint32 i;

	for ( i = 0; i < 6; i++ )
		ptm_p->macAddr[i] = def_mac_addr[i];
}

int ptmInit(int bearer)
{
	if (ptmInitialized[bearer])
		return 0;

	ptmDrvStop(bearer);

	/* ----- Assign reserved data pointer ----- */
#ifdef CONFIG_TC3162_DMEM
	ptm_p[bearer]->ptmRxMemPool_p = (ptmRxMemPool_t *) alloc_sram(sizeof(ptmRxMemPool_t));
	if (ptm_p[bearer]->ptmRxMemPool_p == NULL)
#endif
	ptm_p[bearer]->ptmRxMemPool_p = (ptmRxMemPool_t *) dma_alloc_coherent(NULL, sizeof(ptmRxMemPool_t), &ptm_p[bearer]->ptmRxMemPool_phys_p, GFP_KERNEL);
	if (ptm_p[bearer]->ptmRxMemPool_p == NULL) {
		printk("unable to kmalloc ptmRxMemPool structure.\n");
		return -1;
	}

#ifdef CONFIG_TC3162_DMEM
	ptm_p[bearer]->ptmTxMemPool_p = (ptmTxMemPool_t *) alloc_sram(sizeof(ptmTxMemPool_t));
	if (ptm_p[bearer]->ptmTxMemPool_p == NULL)
#endif
	ptm_p[bearer]->ptmTxMemPool_p = (ptmTxMemPool_t *) dma_alloc_coherent(NULL, sizeof(ptmTxMemPool_t), &ptm_p[bearer]->ptmTxMemPool_phys_p, GFP_KERNEL);
	if (ptm_p[bearer]->ptmTxMemPool_p == NULL) {
		printk("unable to kmalloc ptmTxMemPool structure.\n");
		return -1;
	}
//	}

	/* ----- Set up the paramters ----- */
	ptmDefaultParaSet(ptm_p[bearer]);

	/* ----- Get the Mac address ----- */
	ptmGetMacAddr(ptm_p[bearer]);

	/* ----- Initialize Tx/Rx descriptors ----- */
	if (ptmDrvDescripInit(ptm_p[bearer]) != 0)
		return -1;

	/* ----- Initialize Registers ----- */
	ptmDrvRegInit(ptm_p[bearer]);

	ptmDrvStart(bearer);

	ptm_p[bearer]->statisticOn = PTM_STATISTIC_ON;
	ptmInitialized[bearer] = 1;

	return 0;
}

int ptmSwReset(int bearer, ptmAdapter_t *ptm_p)
{
	ptmRxDescr_t *pRxDescp;
	ptmTxDescr_t *pTxDescp;
	struct sk_buff *skb;
#ifdef TC3262_PTM_SG_MODE
	struct sk_buff *sg_skb;
#endif
	uint32 reg;
	uint32 i, txq, rxq;

	if (!ptmInitialized[bearer])
		return 0;
#if 1
	if (netif_running(tc3262_ptm_dev[bearer])) {
		/* wait here for poll to complete */
		#if KERNEL_2_6_36
		napi_disable(&ptm_p->napi);
		#else
		netif_poll_disable(tc3262_ptm_dev[bearer]);
		#endif
		netif_tx_disable(tc3262_ptm_dev[bearer]);
	}
#endif
	ptmDrvStop(bearer);

#if defined(TCSUPPORT_QOS)
	if(*qos_wrr_info == 0) { /*strict priority*/
		reg = read_reg_word(CR_PTM_PTQCR(bearer));
		write_reg_word(CR_PTM_PTQCR(bearer), reg & ~(PTQCR_WRR_EN));
	}
	else {  /*WRR*/
		write_reg_word(CR_PTM_PWWR3(bearer), ((uint32)(qos_wrr_info[1] & 0x0f)));
		write_reg_word(CR_PTM_PWWR2(bearer), ((uint32)(qos_wrr_info[2] & 0x0f)));
		write_reg_word(CR_PTM_PWWR1(bearer), ((uint32)(qos_wrr_info[3] & 0x0f)));
		write_reg_word(CR_PTM_PWWR0(bearer), ((uint32)(qos_wrr_info[4] & 0x0f)));

		reg = read_reg_word(CR_PTM_PTQCR(bearer));
		reg |= (PTQCR_WRR_EN | PTQCR_WRR_SELECT);
		write_reg_word(CR_PTM_PTQCR(bearer), reg );
	}
#endif

	/* init. Rx descriptor, allocate memory for each descriptor */
	for (rxq = 0; rxq < RX_QUEUE_NUM; rxq++) {
		pRxDescp = (ptmRxDescr_t*) ptm_p->rxDescrRingBaseVAddr[rxq];
		for (i = 0; i < ptm_p->rxRingSize; i++, pRxDescp++) {
			// Assign flag
			pRxDescp->rdes0.bits.owner = 1;  /* owned by DMA */
			pRxDescp->rdes1.bits.rx_buf_size = RX_MAX_PKT_LEN;
		}
	}

	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		pTxDescp = (ptmTxDescr_t*) ptm_p->txDescrRingBaseVAddr[txq];
		// free all un-released tx mbuf
		for (i = 0 ; i < ptm_p->txRingSize ; i++) {
			skb = ptm_p->txskbs[txq][i];
			if (skb != NULL)
				dev_kfree_skb_any(skb);
			pTxDescp->tdes0.word = 0;
			pTxDescp->tdes1.word = 0;
			pTxDescp->tdes2.word = 0;
			pTxDescp->tdes3.word = 0;
			ptm_p->txskbs[txq][i] = NULL;
			pTxDescp++;
		}
	}

	for (rxq = 0; rxq < RX_QUEUE_NUM; rxq++)
		ptm_p->rxCurrentDescp[rxq] = 0;
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		ptm_p->txCurrentDescp[txq] = 0;
		ptm_p->txUnReleasedDescp[txq] = 0;
		ptm_p->txUnReleasedBufCnt[txq] = 0;
	}

	write_reg_word(CR_PTM_PRTPR(bearer, 0), 0x8000);
	write_reg_word(CR_PTM_PRTPR(bearer, 1), 0x8000);
	for (txq = 0; txq < TX_QUEUE_NUM; txq++)
		write_reg_word(CR_PTM_PTTPR(bearer, txq), 0);

	/* Software reset Mac */
	reg = read_reg_word (CR_PTM_PRCR(bearer));
	reg |= PRCT_SW_RESET;
	write_reg_word (CR_PTM_PRCR(bearer), reg);
	pause(10);

	ptmDrvStart(bearer);
#if 1
	if (netif_running(tc3262_ptm_dev[bearer])) {
		netif_wake_queue(tc3262_ptm_dev[bearer]);
		#if KERNEL_2_6_36
		napi_enable(&ptm_p->napi);
		#else
		netif_poll_enable(tc3262_ptm_dev[bearer]);
		#endif
	}
#endif
	return 0;
}

/************************************************************************
*     E T H E R N E T    D E V I C E   P R O C  D E F I N I T I O N S
*************************************************************************
*/

#define CHK_BUF() pos = begin + index; if (pos < off) { index = 0; begin = pos; }; if (pos > off + count) goto done;

static int getETHLinkSt(char *buf)
{
	uint16 index = 0;

	return index;
}
//#ifdef QOS_REMARKING  /*Rodney_20090724*/
#if defined (QOS_REMARKING) || defined (TCSUPPORT_HW_QOS)
static int ptm_qoswrr_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data){

	printk("%d %d %d %d %d\n", *qos_wrr_info, *(qos_wrr_info + 1), *(qos_wrr_info + 2), *(qos_wrr_info + 3), *(qos_wrr_info + 4));
	return 0;
}

static int ptm_qoswrr_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data){
	int len;
	char get_buf[32];
	uint32 priority, reg;
	int max_wrr_val = 0, i, bearer;

	/* do a range checking, don't overflow buffers in kernel modules */
	if(count > 32)
		len = 32;
	else
		len = count;
	/* use the copy_from_user function to copy buffer data to our get_buf */
	if(copy_from_user(get_buf, buffer, len))
		return -EFAULT;
	/* zero terminate get_buf */
	get_buf[len]='\0';

	if(sscanf(get_buf, "%d %d %d %d %d", qos_wrr_info, (qos_wrr_info+1), (qos_wrr_info+2), (qos_wrr_info+3), (qos_wrr_info+4)) != 5)
		return count;

	/* find max qos wrr weight */
	for (i = 0; i < 4; i++) {
		if (max_wrr_val < qos_wrr_info[i + 1]) {
			max_wrr_val = qos_wrr_info[i + 1];
			max_prio = 3 - i;
		}
	}

	qos_wrr_user = QOS_DMAWRR_USERDEFINE;

	ptmSwReset(PTM_B0, ptm_p[PTM_B0]);
	ptmSwReset(PTM_B1, ptm_p[PTM_B1]);
	for (bearer = PTM_B0; bearer <= PTM_B1; bearer += PTM_B1) {
		if(*qos_wrr_info == 0) { /*strict priority*/
			reg = read_reg_word(CR_PTM_PTQCR(bearer));
			write_reg_word(CR_PTM_PTQCR(bearer), reg & ~(PTQCR_WRR_EN));
		}
		else {  /*WRR*/
			write_reg_word(CR_PTM_PWWR3(bearer), ((uint32)(qos_wrr_info[1] & 0x0f)));
			write_reg_word(CR_PTM_PWWR2(bearer), ((uint32)(qos_wrr_info[2] & 0x0f)));
			write_reg_word(CR_PTM_PWWR1(bearer), ((uint32)(qos_wrr_info[3] & 0x0f)));
			write_reg_word(CR_PTM_PWWR0(bearer), ((uint32)(qos_wrr_info[4] & 0x0f)));

			reg = read_reg_word(CR_PTM_PTQCR(bearer));
			reg |= (PTQCR_WRR_EN | PTQCR_WRR_SELECT);
			write_reg_word(CR_PTM_PTQCR(bearer), reg );
		}
	}
	return len;
}
#endif

#ifdef TCSUPPORT_QOS
static int ptm_tcqos_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data){

	if (qos_flag == QOS_SW_PQ) {
		printk("qos discipline is PQ.\n");
	}
	else if (qos_flag == QOS_SW_WRR) {
		printk("qos discipline is WRR.\n");
	}
	else if (qos_flag == QOS_SW_CAR) {
		printk("qos discipline is CAR.\n");
	}
	else if (qos_flag == QOS_HW_WRR) {
		printk("qos discipline is HW WRR.\n");
	}
	else if (qos_flag == QOS_HW_PQ) {
		printk("qos discipline is HW PQ.\n");
	}
	else {
		printk("qos is disabled.\n");
	}
	return 0;
}

static int ptm_tcqos_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data){

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
	if (!strcmp(qos_disc, "PQ")) {
		qos_flag = QOS_SW_PQ;
	}
	else if (!strcmp(qos_disc, "WRR")) {
		qos_flag = QOS_SW_WRR;
	}
	else if (!strcmp(qos_disc, "CAR")) {
		qos_flag = QOS_SW_CAR;
	}
	else if (!strcmp(qos_disc, "HWWRR")) {
		qos_flag = QOS_HW_WRR;
	}
	else if (!strcmp(qos_disc, "HWPQ")) {
		qos_flag = QOS_HW_PQ;
	}
	else {
		qos_flag = NULLQOS;
	}

	return len;
}
#endif

static int ptm_reset_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "0\n");
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

static int ptm_reset_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];
	int bearer = *((int *) data);

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	ptmSwReset(bearer, ptm_p[bearer]);

	return count;
}

static int ptm_stats_read_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int bearer = *((int *) data);
	uint32 value;
#if defined(TCSUPPORT_HWNAT)
	struct pktflow_stats pf_stats;
#endif

	if (!ptmInitialized[bearer]) {
		*eof = 1;
		return 0;
	}

#if defined(TCSUPPORT_HWNAT)
	if (pktflow_get_stats_hook)
		pktflow_get_stats_hook(&pf_stats, 2 + bearer);
	else
		memset(&pf_stats, 0, sizeof(pf_stats));
#endif

	value = ptm_p[bearer]->ptmStat.MIB_II.inOctets;
#if defined(TCSUPPORT_HWNAT)
	value += pf_stats.rx_bytes;
#endif
	index += sprintf(buf+index, "inOctets              = 0x%08lx, ", value);
	CHK_BUF();
	value = ptm_p[bearer]->ptmStat.MIB_II.inUnicastPkts;
#if defined(TCSUPPORT_HWNAT)
	value += pf_stats.rx_packets;
#endif
	index += sprintf(buf+index, "inUnicastPkts         = 0x%08lx\n", value);
	CHK_BUF();

	index += sprintf(buf+index, "inMulticastPkts       = 0x%08lx, ", ptm_p[bearer]->ptmStat.MIB_II.inMulticastPkts);
	CHK_BUF();
	value = ptm_p[bearer]->ptmStat.MIB_II.inDiscards;
#if defined(TCSUPPORT_HWNAT)
	value += pf_stats.rx_dropped;
#endif
	index += sprintf(buf+index, "inDiscards            = 0x%08lx\n", value);
	CHK_BUF();

	index += sprintf(buf+index, "inErrors              = 0x%08lx, ", ptm_p[bearer]->ptmStat.MIB_II.inErrors);
	CHK_BUF();
	value = ptm_p[bearer]->ptmStat.MIB_II.outOctets;
#if defined(TCSUPPORT_HWNAT)
	value += pf_stats.tx_bytes;
#endif
	index += sprintf(buf+index, "outOctets             = 0x%08lx\n", ptm_p[bearer]->ptmStat.MIB_II.outOctets);
	CHK_BUF();

	value = ptm_p[bearer]->ptmStat.MIB_II.outUnicastPkts;
#if defined(TCSUPPORT_HWNAT)
	value += pf_stats.tx_packets;
#endif
	index += sprintf(buf+index, "outUnicastPkts        = 0x%08lx, ", ptm_p[bearer]->ptmStat.MIB_II.outUnicastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "outMulticastPkts      = 0x%08lx\n", ptm_p[bearer]->ptmStat.MIB_II.outMulticastPkts);
	CHK_BUF();

	value = ptm_p[bearer]->ptmStat.MIB_II.outDiscards;
#if defined(TCSUPPORT_HWNAT)
	value += pf_stats.tx_dropped;
#endif
	index += sprintf(buf+index, "outDiscards           = 0x%08lx, ", value);
	CHK_BUF();
	index += sprintf(buf+index, "outErrors             = 0x%08lx\n", ptm_p[bearer]->ptmStat.MIB_II.outErrors);
	CHK_BUF();

	index += sprintf(buf+index, "\n[ Statistics Display ]\n");
	CHK_BUF();
	index += sprintf(buf+index, "txJabberTimeCnt       = 0x%08lx  ", ptm_p[bearer]->ptmStat.inSilicon.txJabberTimeCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txLossOfCarrierCnt    = 0x%08lx\n", ptm_p[bearer]->ptmStat.inSilicon.txLossOfCarrierCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txNoCarrierCnt        = 0x%08lx  ", ptm_p[bearer]->ptmStat.inSilicon.txNoCarrierCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txLateCollisionCnt    = 0x%08lx\n", ptm_p[bearer]->ptmStat.inSilicon.txLateCollisionCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txExCollisionCnt      = 0x%08lx  ", ptm_p[bearer]->ptmStat.inSilicon.txExCollisionCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txHeartbeatFailCnt    = 0x%08lx\n", ptm_p[bearer]->ptmStat.inSilicon.txHeartbeatFailCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txCollisionCnt        = 0x%08lx  ", ptm_p[bearer]->ptmStat.inSilicon.txCollisionCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txExDeferralCnt       = 0x%08lx\n", ptm_p[bearer]->ptmStat.inSilicon.txExDeferralCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txUnderRunCnt         = 0x%08lx  ", ptm_p[bearer]->ptmStat.inSilicon.txUnderRunCnt);
	CHK_BUF();

	index += sprintf(buf+index, "rxAlignErr            = 0x%08lx\n", ptm_p[bearer]->ptmStat.inSilicon.rxAlignErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxDribblingErr        = 0x%08lx  ", ptm_p[bearer]->ptmStat.inSilicon.rxDribblingErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxSymbolErr           = 0x%08lx\n", ptm_p[bearer]->ptmStat.inSilicon.rxSymbolErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxMiiErr              = 0x%08lx  ", ptm_p[bearer]->ptmStat.inSilicon.rxMiiErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxCollisionErr        = 0x%08lx\n", ptm_p[bearer]->ptmStat.inSilicon.rxCollisionErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxCrcErr              = 0x%08lx  ", ptm_p[bearer]->ptmStat.inSilicon.rxCrcErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxEtherFrameLengthErr = 0x%08lx\n", ptm_p[bearer]->ptmStat.inSilicon.rxEtherFrameLengthErr);
	CHK_BUF();
	index += sprintf(buf+index, "rx802p3FrameLengthErr = 0x%08lx  ", ptm_p[bearer]->ptmStat.inSilicon.rx802p3FrameLengthErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxPktIPChkSumErr      = 0x%08lx\n", ptm_p[bearer]->ptmStat.inSilicon.rxPktIPChkSumErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxRuntErr             = 0x%08lx  ", ptm_p[bearer]->ptmStat.inSilicon.rxRuntErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxLongErr             = 0x%08lx\n", ptm_p[bearer]->ptmStat.inSilicon.rxLongErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxOverrunInt          = 0x%08lx\n", ptm_p[bearer]->ptmStat.inSilicon.rxOverrunInt);
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

static int ptm_stats_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];
	int bearer = *((int *) data);

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	if (!ptmInitialized[bearer]) {
		return 0;
	}

	memset(&ptm_p[bearer]->ptmStat.MIB_II, 0, sizeof(ptmMIB_II_t));
	memset(&ptm_p[bearer]->ptmStat.inSilicon, 0, sizeof(inSiliconStat_t));

#if defined(TCSUPPORT_HWNAT)
	if (pktflow_clear_stats_hook)
		pktflow_clear_stats_hook(2 + bearer);
#endif

	return count;
}

int ptm_pripktchk_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%d\n", priPktChk);
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

int ptm_pripktchk_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	priPktChk = simple_strtoul(val_string, NULL, 10) ? 1 : 0;

	return count;
}

#ifdef LOOPBACK_SUPPORT

static int ptm_loopback_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%04x\n", ptmLoopback);
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

static int ptm_loopback_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	ptmLoopback = simple_strtoul(val_string, NULL, 16);
	if (LOOPBACK_MODE(ptmLoopback) == LOOPBACK_TX_QOS) {
		tc3262_ptm_qos_gen(tc3262_ptm_dev[PTM_B0]);
		tc3262_ptm_qos_gen(tc3262_ptm_dev[PTM_B1]);
	}

	return count;
}

static int ptm_loopback_test_read_proc(char *page, char **start, off_t off,
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

static int ptm_loopback_test_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];
	int npackets, txlen;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	sscanf(val_string, "%d %d", &npackets, &txlen);
	tc3262_ptm_loopback_gen(NULL, npackets, txlen);

	return count;
}

#endif

int ptm_link_st_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int len = getETHLinkSt(buf);
	if (len <= off+count)
		*eof = 1;
	*start = buf + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
		len = 0;
	return len;
}

int ptm_reg_dump_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int i;
	int bearer = *((int *) data);

	if (!ptmInitialized[bearer]) {
		*eof = 1;
		return 0;
	}

	index += sprintf(buf+index, "[Bearer%d]\n", bearer);
	CHK_BUF();

	index += sprintf(buf+index, "CR_PTM_PRCR        (0x%08x) = 0x%08lx\n",
				CR_PTM_PRCR(bearer), VPint(CR_PTM_PRCR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PIMR        (0x%08x) = 0x%08lx\n",
				CR_PTM_PIMR(bearer), VPint(CR_PTM_PIMR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PISR        (0x%08x) = 0x%08lx\n",
				CR_PTM_PISR(bearer), VPint(CR_PTM_PISR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_TQER        (0x%08x) = 0x%08lx\n",
				CR_PTM_TQER(bearer), VPint(CR_PTM_TQER(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PFSR        (0x%08x) = 0x%08lx\n",
				CR_PTM_PFSR(bearer), VPint(CR_PTM_PFSR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PMAR        (0x%08x) = 0x%08lx\n",
				CR_PTM_PMAR(bearer), VPint(CR_PTM_PMAR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PMAR1       (0x%08x) = 0x%08lx\n",
				CR_PTM_PMAR1(bearer), VPint(CR_PTM_PMAR1(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PDPSR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PDPSR(bearer), VPint(CR_PTM_PDPSR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PPCRR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PPCRR(bearer), VPint(CR_PTM_PPCRR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PVPMR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PVPMR(bearer), VPint(CR_PTM_PVPMR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTPR0       (0x%08x) = 0x%08lx\n",
				CR_PTM_PTPR0(bearer), VPint(CR_PTM_PTPR0(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTPR1       (0x%08x) = 0x%08lx\n",
				CR_PTM_PTPR1(bearer), VPint(CR_PTM_PTPR1(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTPR2       (0x%08x) = 0x%08lx\n",
				CR_PTM_PTPR2(bearer), VPint(CR_PTM_PTPR2(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTPR3       (0x%08x) = 0x%08lx\n",
				CR_PTM_PTPR3(bearer), VPint(CR_PTM_PTPR3(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTCPR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PTCPR(bearer), VPint(CR_PTM_PTCPR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRXLR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PRXLR(bearer), VPint(CR_PTM_PRXLR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PDER        (0x%08x) = 0x%08lx\n",
				CR_PTM_PDER(bearer), VPint(CR_PTM_PDER(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRRBR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PRRBR(bearer), VPint(CR_PTM_PRRBR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTRBR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PTRBR(bearer), VPint(CR_PTM_PTRBR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRRSR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PRRSR(bearer), VPint(CR_PTM_PRRSR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTRS01R     (0x%08x) = 0x%08lx\n",
				CR_PTM_PTRS01R(bearer), VPint(CR_PTM_PTRS01R(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTRS23R     (0x%08x) = 0x%08lx\n",
				CR_PTM_PTRS23R(bearer), VPint(CR_PTM_PTRS23R(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTRS45R     (0x%08x) = 0x%08lx\n",
				CR_PTM_PTRS45R(bearer), VPint(CR_PTM_PTRS45R(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTRS67R     (0x%08x) = 0x%08lx\n",
				CR_PTM_PTRS67R(bearer), VPint(CR_PTM_PTRS67R(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRHPR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PRHPR(bearer), VPint(CR_PTM_PRHPR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PSGCR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PSGCR(bearer), VPint(CR_PTM_PSGCR(bearer)));
	CHK_BUF();
	for (i = 0; i < TX_QUEUE_NUM; i++) {
		index += sprintf(buf+index, "CR_PTM_PTHPR(%d)    (0x%08x) = 0x%08lx\n",
					i, CR_PTM_PTHPR(bearer, i), VPint(CR_PTM_PTHPR(bearer, i)));
		CHK_BUF();
	}
	for (i = 0; i < TX_QUEUE_NUM; i++) {
		index += sprintf(buf+index, "CR_PTM_PTTPR(%d)    (0x%08x) = 0x%08lx\n",
					i, CR_PTM_PTTPR(bearer, i), VPint(CR_PTM_PTTPR(bearer, i)));
		CHK_BUF();
	}
	index += sprintf(buf+index, "CR_PTM_PTLWR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PTLWR(bearer), VPint(CR_PTM_PTLWR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTUSR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PTUSR(bearer), VPint(CR_PTM_PTUSR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTIETR      (0x%08x) = 0x%08lx\n",
				CR_PTM_PTIETR(bearer), VPint(CR_PTM_PTIETR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PPRCTR      (0x%08x) = 0x%08lx\n",
				CR_PTM_PPRCTR(bearer), VPint(CR_PTM_PPRCTR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRDLR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PRDLR(bearer), VPint(CR_PTM_PRDLR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTDLR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PTDLR(bearer), VPint(CR_PTM_PTDLR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTQCR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PTQCR(bearer), VPint(CR_PTM_PTQCR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTSLR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PTSLR(bearer), VPint(CR_PTM_PTSLR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PSBCR0      (0x%08x) = 0x%08lx\n",
				CR_PTM_PSBCR0(bearer), VPint(CR_PTM_PSBCR0(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PSBCR1      (0x%08x) = 0x%08lx\n",
				CR_PTM_PSBCR1(bearer), VPint(CR_PTM_PSBCR1(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PSBCR2      (0x%08x) = 0x%08lx\n",
				CR_PTM_PSBCR2(bearer), VPint(CR_PTM_PSBCR2(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PSBCR3      (0x%08x) = 0x%08lx\n",
				CR_PTM_PSBCR3(bearer), VPint(CR_PTM_PSBCR3(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PWWR0       (0x%08x) = 0x%08lx\n",
				CR_PTM_PWWR0(bearer), VPint(CR_PTM_PWWR0(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PWWR1       (0x%08x) = 0x%08lx\n",
				CR_PTM_PWWR1(bearer), VPint(CR_PTM_PWWR1(bearer)));
	CHK_BUF() ;
	index += sprintf(buf+index, "CR_PTM_PWWR2       (0x%08x) = 0x%08lx\n",
				CR_PTM_PWWR2(bearer), VPint(CR_PTM_PWWR2(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PWWR3       (0x%08x) = 0x%08lx\n",
				CR_PTM_PWWR3(bearer), VPint(CR_PTM_PWWR3(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRTPR0      (0x%08x) = 0x%08lx\n",
				CR_PTM_PRTPR(bearer, 0), VPint(CR_PTM_PRTPR(bearer, 0)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRTPR1      (0x%08x) = 0x%08lx\n",
				CR_PTM_PRTPR(bearer, 1), VPint(CR_PTM_PRTPR(bearer, 1)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRXCR0      (0x%08x) = 0x%08lx\n",
				CR_PTM_PRXCR0(bearer), VPint(CR_PTM_PRXCR0(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRXCR1      (0x%08x) = 0x%08lx\n",
				CR_PTM_PRXCR1(bearer), VPint(CR_PTM_PRXCR1(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRIETR0     (0x%08x) = 0x%08lx\n",
				CR_PTM_PRIETR0(bearer), VPint(CR_PTM_PRIETR0(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRIETR1     (0x%08x) = 0x%08lx\n",
				CR_PTM_PRIETR1(bearer), VPint(CR_PTM_PRIETR1(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTPC        (0x%08x) = 0x%08lx\n",
				CR_PTM_PTPC(bearer), VPint(CR_PTM_PTPC(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTBC        (0x%08x) = 0x%08lx\n",
				CR_PTM_PTBC(bearer), VPint(CR_PTM_PTBC(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRPC        (0x%08x) = 0x%08lx\n",
				CR_PTM_PRPC(bearer), VPint(CR_PTM_PRPC(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRBC        (0x%08x) = 0x%08lx\n",
				CR_PTM_PRBC(bearer), VPint(CR_PTM_PRBC(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRBPC       (0x%08x) = 0x%08lx\n",
				CR_PTM_PRBPC(bearer), VPint(CR_PTM_PRBPC(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRMPC       (0x%08x) = 0x%08lx\n",
				CR_PTM_PRMPC(bearer), VPint(CR_PTM_PRMPC(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRRPC       (0x%08x) = 0x%08lx\n",
				CR_PTM_PRRPC(bearer), VPint(CR_PTM_PRRPC(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRLPC       (0x%08x) = 0x%08lx\n",
				CR_PTM_PRLPC(bearer), VPint(CR_PTM_PRLPC(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRCEPC      (0x%08x) = 0x%08lx\n",
				CR_PTM_PRCEPC(bearer), VPint(CR_PTM_PRCEPC(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTFC        (0x%08x) = 0x%08lx\n",
				CR_PTM_PTFC(bearer), VPint(CR_PTM_PTFC(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRBSR       (0x%08x) = 0x%08lx\n",
				CR_PTM_PRBSR(bearer), VPint(CR_PTM_PRBSR(bearer)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PDRPCR0     (0x%08x) = 0x%08lx\n",
				CR_PTM_PDRPCR(bearer, 0), VPint(CR_PTM_PDRPCR(bearer, 0)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PDRPCR1     (0x%08x) = 0x%08lx\n",
				CR_PTM_PDRPCR(bearer, 1), VPint(CR_PTM_PDRPCR(bearer, 1)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRPPCR0     (0x%08x) = 0x%08lx\n",
				CR_PTM_PRPPCR(bearer, 0), VPint(CR_PTM_PRPPCR(bearer, 0)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRPPCR1     (0x%08x) = 0x%08lx\n",
				CR_PTM_PRPPCR(bearer, 1), VPint(CR_PTM_PRPPCR(bearer, 1)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PESR        (0x%08x) = 0x%08lx\n",
				CR_PTM_PESR(bearer), VPint(CR_PTM_PESR(bearer)));
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

int ptm_rxring_dump_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int i, rxq, bearer;
	struct ringinfo *ringinfo;
	ptmRxDescr_t *pRxDescp;

	ringinfo = (struct ringinfo *) data;
	bearer = ringinfo->bearer;
	rxq = ringinfo->queue;

	if (!ptmInitialized[bearer]) {
		*eof = 1;
		return 0;
	}

	pRxDescp = (ptmRxDescr_t*) ptm_p[bearer]->rxDescrRingBaseVAddr[rxq];
	index += sprintf(buf+index, "bearer%d rx descr ring%d=%08lx\n", bearer, rxq, (uint32) pRxDescp);
	CHK_BUF();

	for (i = 0 ; i< ptm_p[bearer]->rxRingSize ; i++, pRxDescp++) {
		index += sprintf(buf+index, "i= %d descr=%08lx\n", i, (uint32) pRxDescp);
		CHK_BUF();
		index += sprintf(buf+index, " rdes0=%08lx\n", pRxDescp->rdes0.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes1=%08lx\n", pRxDescp->rdes1.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes2=%08lx\n", pRxDescp->rdes2.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes3=%08lx\n", pRxDescp->rdes3.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes4=%08lx\n", pRxDescp->rdes4.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes5=%08lx\n", pRxDescp->rdes5.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes6=%08lx\n", pRxDescp->rdes6.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes7=%08lx\n", pRxDescp->rdes7.word);
		CHK_BUF();
		index += sprintf(buf+index, " skb  =%08lx\n", (uint32) ptm_p[bearer]->rxskbs[rxq][i]);
		CHK_BUF();
	}

	index += sprintf(buf+index, "rxCurrentDescp    =%ld\n", ptm_p[bearer]->rxCurrentDescp[rxq]);
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRTPR%d     =%08lx\n", rxq, VPint(CR_PTM_PRTPR(bearer, rxq)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PRHPR      =%08lx\n", VPint(CR_PTM_PRHPR(bearer)));
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

int ptm_txring_dump_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int i, txq, bearer;
	struct ringinfo *ringinfo;
  	ptmTxDescr_t *pTxDescp;

	ringinfo = (struct ringinfo *) data;
	bearer = ringinfo->bearer;
	txq = ringinfo->queue;

	if (!ptmInitialized[bearer]) {
		*eof = 1;
		return 0;
	}

	pTxDescp = (ptmTxDescr_t*) ptm_p[bearer]->txDescrRingBaseVAddr[txq];

	index += sprintf(buf+index, "bearer%d tx descr ring%d=%08lx\n", bearer, txq, (uint32) pTxDescp);
	CHK_BUF();

	for (i = 0 ; i < ptm_p[bearer]->txRingSize ; i++, pTxDescp++) {
		index += sprintf(buf+index, "i= %d descr=%08lx\n", i, (uint32) pTxDescp);
		CHK_BUF();
		index += sprintf(buf+index, " tdes0=%08lx\n", pTxDescp->tdes0.word);
		CHK_BUF();
		index += sprintf(buf+index, " tdes1=%08lx\n", pTxDescp->tdes1.word);
		CHK_BUF();
		index += sprintf(buf+index, " tdes2=%08lx\n", pTxDescp->tdes2.word);
		CHK_BUF();
		index += sprintf(buf+index, " tdes3=%08lx\n", pTxDescp->tdes3.word);
		CHK_BUF();
		index += sprintf(buf+index, " skb  =%08lx\n", (uint32) ptm_p[bearer]->txskbs[txq][i]);
		CHK_BUF();
	}

	index += sprintf(buf+index, "txCurrentDescp[%d]    =%ld\n", txq, ptm_p[bearer]->txCurrentDescp[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "txUnReleasedDescp[%d] =%ld\n", txq, ptm_p[bearer]->txUnReleasedDescp[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "txUnReleasedBufCnt[%d]=%ld\n", txq, ptm_p[bearer]->txUnReleasedBufCnt[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTTPR(%d)   =%08lx\n", txq, VPint(CR_PTM_PTTPR(bearer, txq)));
	CHK_BUF();
	index += sprintf(buf+index, "CR_PTM_PTHPR(%d)   =%08lx\n", txq, VPint(CR_PTM_PTHPR(bearer, txq)));
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

static void tc3262_ptm_timer(unsigned long data)
{
	struct net_device *dev = (struct net_device *) data;
	ptmAdapter_t *ptm_p = netdev_priv(dev);
#ifdef TC3262_PTM_NAPI
	int txq;
#endif

#ifdef LOOPBACK_SUPPORT
	if (LOOPBACK_MODE(ptmLoopback))
		return;
#endif

#ifdef TC3262_PTM_NAPI
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
   		ptmTxRingProc(ptm_p, txq, 0);
	}
#endif

#if defined(TCSUPPORT_HWNAT)
{
	unsigned long rx_bytes = ptmStats[ptm_p->bearer].rx_bytes;
	unsigned long tx_bytes = ptmStats[ptm_p->bearer].tx_bytes;

	ptmStats[ptm_p->bearer].rx_bytes = ptm_p->ptmStat.MIB_II.inOctets;
	ptmStats[ptm_p->bearer].tx_bytes = ptm_p->ptmStat.MIB_II.outOctets;

	if (pktflow_get_stats_hook) {
		struct pktflow_stats pf_stats;
		pktflow_get_stats_hook(&pf_stats, 2 + ptm_p->bearer);
		ptmStats[ptm_p->bearer].rx_bytes += pf_stats.rx_bytes;
		ptmStats[ptm_p->bearer].tx_bytes += pf_stats.tx_bytes;
	}
}
#endif

	/* Schedule for the next time */
	ptm_p->ptm_timer.expires = jiffies + msecs_to_jiffies(250);
  	add_timer(&ptm_p->ptm_timer);
}

/* Starting up the ethernet device */
static int tc3262_ptm_open(struct net_device *dev)
{
	int err = 0;
	ptmAdapter_t *ptm_p = netdev_priv(dev);

  	printk(KERN_INFO "%s: starting interface %s\n", __func__, dev->name);
	err = request_irq(dev->irq, tc3262_ptm_isr, 0, dev->name, dev);
	if(err) {
		goto out;
	}
  	ptmInit(ptm_p->bearer);
  	/* Schedule timer */
  	setup_timer(&ptm_p->ptm_timer, tc3262_ptm_timer, (unsigned long)dev);
  	mod_timer(&ptm_p->ptm_timer, jiffies + msecs_to_jiffies(250));
#if KERNEL_2_6_36
	napi_enable(&ptm_p->napi);
#endif
  	netif_start_queue(dev);
out:
  	return err;
}

/* Stopping the ethernet device */
static int tc3262_ptm_close(struct net_device *dev)
{
	ptmAdapter_t *ptm_p = netdev_priv(dev);
#if (defined(TCSUPPORT_WAN_ETHER) || defined(TCSUPPORT_WAN_PTM)) && defined(TCSUPPORT_MULTISERVICE_ON_WAN)
	int txq;
#endif

	printk(KERN_INFO "%s: stop interface %s\n", __func__, dev->name);
  	netif_stop_queue(dev);
	#if KERNEL_2_6_36
	napi_disable(&ptm_p->napi);
	#endif
	free_irq(dev->irq, dev);
  	/* Kill timer */
  	del_timer_sync(&ptm_p->ptm_timer);
	#if (defined(TCSUPPORT_WAN_ETHER) || defined(TCSUPPORT_WAN_PTM)) && defined(TCSUPPORT_MULTISERVICE_ON_WAN)
	ptmTxDrvStop(ptm_p->bearer);
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
   		ptmTxRingProc(ptm_p, txq, 1);
	}
	ptmTxDrvStart(ptm_p->bearer);
	#endif
  	return 0;
}

/* Setup multicast list */
static void tc3262_ptm_set_multicast_list(struct net_device *dev)
{
	return; /* Do nothing */
}

/* Setting customized mac address */
static int tc3262_ptm_set_macaddr(struct net_device *dev, void *p)
{
	ptmAdapter_t *ptm_p = netdev_priv(dev);
	struct sockaddr *addr = p;

	/* Check if given address is valid ethernet MAC address */
  	if (!is_valid_ether_addr(addr->sa_data))
    	return(-EIO);

	/* Save the customize mac address */
  	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
  	memcpy(def_mac_addr, addr->sa_data, dev->addr_len);

	ptmGetMacAddr(ptm_p);
  	ptmSetMacReg(ptm_p);

	return 0;
}

/* Get the stats information */
static struct net_device_stats *tc3262_ptm_stats(struct net_device *dev)
{
	ptmAdapter_t *ptm_p = netdev_priv(dev);
	struct net_device_stats *stats;

	stats = &ptm_p->stats;

	stats->rx_packets = ptm_p->ptmStat.MIB_II.inUnicastPkts +
		ptm_p->ptmStat.MIB_II.inMulticastPkts;
	stats->tx_packets = ptm_p->ptmStat.MIB_II.outUnicastPkts +
		ptm_p->ptmStat.MIB_II.outMulticastPkts;
	stats->rx_bytes = ptm_p->ptmStat.MIB_II.inOctets;
	stats->tx_bytes = ptm_p->ptmStat.MIB_II.outOctets;
	stats->rx_dropped = ptm_p->ptmStat.MIB_II.inDiscards;
	stats->tx_dropped = ptm_p->ptmStat.MIB_II.outDiscards;
	stats->multicast = ptm_p->ptmStat.MIB_II.inMulticastPkts;
	stats->rx_errors = ptm_p->ptmStat.MIB_II.inErrors;
	stats->tx_errors = ptm_p->ptmStat.MIB_II.outErrors;
	stats->collisions = ptm_p->ptmStat.inSilicon.txExCollisionCnt +
		ptm_p->ptmStat.inSilicon.txCollisionCnt +
		ptm_p->ptmStat.inSilicon.rxCollisionErr;

#if defined(TCSUPPORT_HWNAT)
	if (pktflow_get_stats_hook) {
		struct pktflow_stats pf_stats;

		pktflow_get_stats_hook(&pf_stats, 2 + ptm_p->bearer);

		stats->rx_packets += pf_stats.rx_packets;
		stats->tx_packets += pf_stats.tx_packets;
		stats->rx_bytes += pf_stats.rx_bytes;
		stats->tx_bytes += pf_stats.tx_bytes;
		stats->rx_dropped += pf_stats.rx_dropped;
		stats->tx_dropped += pf_stats.tx_dropped;
	}
#endif

	return stats;
}

/* Handling ioctl call */
static int tc3262_ptm_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	/* Not implemented yet */
  	return 0;
}

static irqreturn_t tc3262_ptm_isr(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;
	ptmAdapter_t *ptm_p = netdev_priv(dev);
	uint32 reg;
	int bearer = (irq == PTM_B0_INT) ? PTM_B0 : PTM_B1;
	unsigned long flags;

	reg = read_reg_word(CR_PTM_PISR(bearer));
	write_reg_word(CR_PTM_PISR(bearer), reg & read_reg_word(CR_PTM_PIMR(bearer)));

	//printk("ptm_isr PISR=%08lx bearer=%d\n", reg, bearer);

	// ----------Packet Received----------------------
	if (reg & (PIMR_CH1_RX_DONE | PIMR_CH1_RX_RING_FULL | PIMR_CH1_RX_FIFO_OVR |
			   PIMR_CH0_RX_DONE | PIMR_CH0_RX_RING_FULL | PIMR_CH0_RX_FIFO_OVR)) {
#ifdef TC3262_PTM_NAPI
		spin_lock_irqsave(&pimr_lock, flags);

#if KERNEL_2_6_36
		if (napi_schedule_prep(&ptm_p->napi)) {
			write_reg_word(CR_PTM_PIMR(bearer), read_reg_word(CR_PTM_PIMR(bearer)) &
			 	~(PIMR_CH1_RX_DONE | PIMR_CH1_RX_RING_FULL | PIMR_CH1_RX_FIFO_OVR |
			   	  PIMR_CH0_RX_DONE | PIMR_CH0_RX_RING_FULL | PIMR_CH0_RX_FIFO_OVR));

			__napi_schedule(&ptm_p->napi);
		}
#else
		if (netif_rx_schedule_prep(dev)) {
			write_reg_word(CR_PTM_PIMR(bearer), read_reg_word(CR_PTM_PIMR(bearer)) &
			 	~(PIMR_CH1_RX_DONE | PIMR_CH1_RX_RING_FULL | PIMR_CH1_RX_FIFO_OVR |
			   	  PIMR_CH0_RX_DONE | PIMR_CH0_RX_RING_FULL | PIMR_CH0_RX_FIFO_OVR));

			__netif_rx_schedule(dev);
		}
#endif

		spin_unlock_irqrestore(&pimr_lock, flags);
#else
    	ptmRxRingProc(dev, dev->weight);
#endif
		if (reg & PIMR_CH0_RX_RING_FULL) {
	        ptm_p->ptmStat.MIB_II.inDiscards++;
#ifdef LOOPBACK_SUPPORT
			printk("%s B%d CH%d RX_RING_FULL PISR=%08lx PRXCR0=%08lx PRXCR1=%08lx\n",
					dev->name, bearer, (reg & PIMR_CH0_RX_RING_FULL) ? 0 : 1, reg,
					VPint(CR_PTM_PRXCR0(bearer)), VPint(CR_PTM_PRXCR1(bearer)));
#endif
		}
		if (reg & PIMR_CH1_RX_RING_FULL) {
	        ptm_p->ptmStat.MIB_II.inDiscards++;
#ifdef LOOPBACK_SUPPORT
			printk("%s B%d CH%d RX_RING_FULL PISR=%08lx PRXCR0=%08lx PRXCR1=%08lx\n",
					dev->name, bearer, (reg & PIMR_CH0_RX_RING_FULL) ? 0 : 1, reg,
					VPint(CR_PTM_PRXCR0(bearer)), VPint(CR_PTM_PRXCR1(bearer)));
#endif
		}
		if (reg & PIMR_CH0_RX_FIFO_OVR) {
	        ptm_p->ptmStat.MIB_II.inDiscards++;
	        ptm_p->ptmStat.inSilicon.rxOverrunInt++;
#ifdef LOOPBACK_SUPPORT
			printk("%s B%d CH%d RX_FIFO_OVR PISR=%08lx PRXCR0=%08lx PRXCR1=%08lx\n",
					dev->name, bearer, (reg & PIMR_CH0_RX_FIFO_OVR) ? 0 : 1, reg,
					VPint(CR_PTM_PRXCR0(bearer)), VPint(CR_PTM_PRXCR1(bearer)));
#endif
		}
		if (reg & PIMR_CH1_RX_FIFO_OVR) {
	        ptm_p->ptmStat.MIB_II.inDiscards++;
	        ptm_p->ptmStat.inSilicon.rxOverrunInt++;
#ifdef LOOPBACK_SUPPORT
			printk("%s B%d CH%d RX_FIFO_OVR PISR=%08lx PRXCR0=%08lx PRXCR1=%08lx\n",
					dev->name, bearer, (reg & PIMR_CH0_RX_FIFO_OVR) ? 0 : 1, reg,
					VPint(CR_PTM_PRXCR0(bearer)), VPint(CR_PTM_PRXCR1(bearer)));
#endif
		}
	}

	if (reg & ~(PIMR_CH1_RX_DONE | PIMR_CH1_RX_RING_FULL | PIMR_CH1_RX_FIFO_OVR |
			    PIMR_CH0_RX_DONE | PIMR_CH0_RX_RING_FULL | PIMR_CH0_RX_FIFO_OVR |
				PIMR_CH0_TX_DONE | PIMR_CH1_TX_DONE)) {
		printk("%s err ptm_isr PISR=%08lx\n", dev->name, reg);
	}

	return IRQ_HANDLED;
}

#ifdef TC3262_PTM_NAPI
#if KERNEL_2_6_36
static int tc3262_ptm_poll(struct napi_struct *napi, int budget)
{
	ptmAdapter_t *ptm_p = container_of(napi, ptmAdapter_t, napi);
	int bearer = ptm_p->bearer;
	struct net_device *dev = ptm_p->dev;
	int rx_work_limit;
	int received = 0;
	int n, done;
	unsigned long flags;

	rx_work_limit = min(budget - received, budget);

	n = ptmRxRingProc(dev, rx_work_limit);
	received += n;

	if (received < budget) {

		spin_lock_irqsave(&pimr_lock, flags);

		__napi_complete(napi);

		write_reg_word(CR_PTM_PIMR(bearer), read_reg_word(CR_PTM_PIMR(bearer)) |
			  (PIMR_CH1_RX_DONE | PIMR_CH1_RX_RING_FULL | PIMR_CH1_RX_FIFO_OVR |
			   PIMR_CH0_RX_DONE | PIMR_CH0_RX_RING_FULL | PIMR_CH0_RX_FIFO_OVR));

		spin_unlock_irqrestore(&pimr_lock, flags);
	}

	return received;
}
#else
static int tc3262_ptm_poll(struct net_device *dev, int *budget)
{
	ptmAdapter_t *ptm_p = netdev_priv(dev);
	int bearer = ptm_p->bearer;
	int rx_work_limit = min(dev->quota, *budget);
	int received = 0;
	int n, done;
	unsigned long flags;

	n = ptmRxRingProc(dev, rx_work_limit);
	if (n) {
		received += n;
		rx_work_limit -= n;
		if (rx_work_limit <= 0) {
			done = 0;
			goto more_work;
		}
	}

	done = 1;

	spin_lock_irqsave(&pimr_lock, flags);

	__netif_rx_complete(dev);

	write_reg_word(CR_PTM_PIMR(bearer), read_reg_word(CR_PTM_PIMR(bearer)) |
			  (PIMR_CH1_RX_DONE | PIMR_CH1_RX_RING_FULL | PIMR_CH1_RX_FIFO_OVR |
			   PIMR_CH0_RX_DONE | PIMR_CH0_RX_RING_FULL | PIMR_CH0_RX_FIFO_OVR));

	spin_unlock_irqrestore(&pimr_lock, flags);

more_work:

	*budget -= received;
	dev->quota -= received;

	return done ? 0 : 1;
}
#endif
#endif

static int tc3262_ptm_start(struct net_device *dev)
{
	int i;
	ptmAdapter_t *ptm_p = netdev_priv(dev);

	uint8 *flash_mac_addr = (uint8 *)0xbfc0ff48;

	if( (flash_mac_addr[0] == 0) && (flash_mac_addr[1] == 0) && (flash_mac_addr[2] == 0) &&
	    (flash_mac_addr[3] == 0) && (flash_mac_addr[4] == 0) && (flash_mac_addr[5] == 0) )
		printk(KERN_INFO "The MAC address in flash is null!\n");
	else
  		memcpy(def_mac_addr, flash_mac_addr, 6);

	for (i = 0; i < 6; i++) {
		dev->dev_addr[i] = def_mac_addr[i];
	}

	spin_lock_init(&ptm_p->lock);

#if KERNEL_2_6_36
	ptm_p->dev = dev;
#else
  	/* Hook up with handlers */
  	dev->get_stats 			= tc3262_ptm_stats;
  	dev->hard_start_xmit 	= tc3262_ptm_tx;
  	dev->open 				= tc3262_ptm_open;
  	dev->stop 				= tc3262_ptm_close;
  	dev->set_multicast_list = tc3262_ptm_set_multicast_list;
  	dev->do_ioctl 			= tc3262_ptm_ioctl;
  	dev->set_mac_address 	= tc3262_ptm_set_macaddr;
#ifdef TC3262_PTM_NAPI
	dev->poll 				= tc3262_ptm_poll;
#endif
	dev->weight 			= PTM_RXDESCP_NO>>1;
#endif
	printk(KERN_INFO
	       "%s: TC3262 PTM Ethernet address: %02X:%02X:%02X:%02X:%02X:%02X\n",
	       dev->name,
	       dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
		   dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

	/*
  	dev->tx_queue_len = PTM_TXDESCP_NO;
  	dev->flags &= ~IFF_MULTICAST;
  	dev->flags |= IFF_DEBUG;
	*/

	return 0;
}

#if KERNEL_2_6_36
static const struct net_device_ops ptm_netdev_ops = {
	.ndo_init				= tc3262_ptm_start,
	.ndo_open				= tc3262_ptm_open,
	.ndo_stop 				= tc3262_ptm_close,
	.ndo_start_xmit			= tc3262_ptm_tx,
//	.ndo_tx_timeout			= pcnet32_tx_timeout,
	.ndo_get_stats			= tc3262_ptm_stats,
	.ndo_set_multicast_list = tc3262_ptm_set_multicast_list,
	.ndo_do_ioctl			= tc3262_ptm_ioctl,
	.ndo_change_mtu			= eth_change_mtu,
	.ndo_set_mac_address 	= tc3262_ptm_set_macaddr,
	.ndo_validate_addr		= eth_validate_addr,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= tc3262_ptm_poll_controller,
#endif
};
#endif

struct ringinfo proc_txring[PTM_BEARER_NUM][TX_QUEUE_NUM];
char proc_txring_name[PTM_BEARER_NUM][TX_QUEUE_NUM][32];
struct ringinfo proc_rxring[PTM_BEARER_NUM][RX_QUEUE_NUM];
char proc_rxring_name[PTM_BEARER_NUM][RX_QUEUE_NUM][32];
int proc_bearer[PTM_BEARER_NUM];

static int __init tc3262_ptm_init(void)
{
  	struct net_device *dev;
  	int err = 0;
	struct proc_dir_entry *ptm_proc;
	int bearer;
	int txq, rxq;
	char proc_name[32];

	printk(KERN_INFO "%s", version);
	printk("\r\n%s\n", MODULE_VERSION_PTM);
	for(bearer = PTM_B0; bearer <= PTM_B1; bearer += PTM_B1) {
		if (bearer == PTM_B0)
			dev = alloc_netdev(sizeof(ptmAdapter_t), "nas8", ether_setup);
		else
			dev = alloc_netdev(sizeof(ptmAdapter_t), "nas9", ether_setup);
		if (!dev)
			return -ENOMEM;

        /*
         * TBS_TAG:add by pengyao 20130311
         * Desc: define PTM interface for tbs_nfp
         */
        dev->priv_flags |= IFF_PTM;
        /*
         * TBS_TAG:END
         */

		tc3262_ptm_dev[bearer] = dev;

		dev->irq = (bearer == PTM_B0) ? PTM_B0_INT : PTM_B1_INT;

		ptm_p[bearer] = netdev_priv(dev);
		ptm_p[bearer]->bearer = bearer;
		ptm_p[bearer]->ptm_intr = (bearer == PTM_B0) ? PTM_B0_INT : PTM_B1_INT;

		#if KERNEL_2_6_36
		/* Hook up with handlers */
		dev->netdev_ops = &ptm_netdev_ops;
		ptm_p[bearer]->napi.weight = PTM_NAPI_WEIGHT;
		netif_napi_add(dev, &ptm_p[bearer]->napi, tc3262_ptm_poll, PTM_NAPI_WEIGHT);
		#else
		dev->init = tc3262_ptm_start;
		#endif
		err = register_netdev(dev);
		if(err) {
			goto out;
		}
		proc_bearer[bearer] = bearer;
		sprintf(proc_name, "tc3162/ptm_b%d_reset", bearer);
		ptm_proc = create_proc_entry(proc_name, 0, NULL);
		ptm_proc->read_proc = ptm_reset_read_proc;
		ptm_proc->write_proc = ptm_reset_write_proc;
		ptm_proc->data = &proc_bearer[bearer];
		/* ethernet related stats */
		sprintf(proc_name, "tc3162/ptm_b%d_stats", bearer);
		ptm_proc = create_proc_entry(proc_name, 0, NULL);
		ptm_proc->read_proc = ptm_stats_read_proc;
		ptm_proc->write_proc = ptm_stats_write_proc;
		ptm_proc->data = &proc_bearer[bearer];
		sprintf(proc_name, "tc3162/ptm_b%d_link_st", bearer);
		create_proc_read_entry(proc_name, 0, NULL, ptm_link_st_proc, &proc_bearer[bearer]);
		sprintf(proc_name, "tc3162/ptm_b%d_reg", bearer);
		create_proc_read_entry(proc_name, 0, NULL, ptm_reg_dump_proc, &proc_bearer[bearer]);
		for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
			proc_txring[bearer][txq].bearer = bearer;
			proc_txring[bearer][txq].queue  = txq;
			sprintf(proc_txring_name[bearer][txq], "tc3162/ptm_b%d_txring%d", bearer, txq);
			create_proc_read_entry(proc_txring_name[bearer][txq], 0, NULL, ptm_txring_dump_proc, &proc_txring[bearer][txq]);
		}
		for (rxq = 0; rxq < RX_QUEUE_NUM; rxq++) {
			proc_rxring[bearer][rxq].bearer = bearer;
			proc_rxring[bearer][rxq].queue  = rxq;
			sprintf(proc_rxring_name[bearer][rxq], "tc3162/ptm_b%d_rxring%d", bearer, rxq);
			create_proc_read_entry(proc_rxring_name[bearer][rxq], 0, NULL, ptm_rxring_dump_proc, &proc_rxring[bearer][rxq]);
		}
	}
	ptm_proc = create_proc_entry("tc3162/ptm_pripktchk", 0, NULL);
	ptm_proc->read_proc = ptm_pripktchk_read_proc;
	ptm_proc->write_proc = ptm_pripktchk_write_proc;
#ifdef LOOPBACK_SUPPORT
	ptm_proc = create_proc_entry("tc3162/ptm_loopback", 0, NULL);
	ptm_proc->read_proc = ptm_loopback_read_proc;
	ptm_proc->write_proc = ptm_loopback_write_proc;
	ptm_proc = create_proc_entry("tc3162/ptm_loopback_test", 0, NULL);
	ptm_proc->read_proc = ptm_loopback_test_read_proc;
	ptm_proc->write_proc = ptm_loopback_test_write_proc;
#endif

#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
	ptm_proc = create_proc_entry("tc3162/ptm_qoswrr", 0, NULL);
	ptm_proc->read_proc = ptm_qoswrr_read_proc;
	ptm_proc->write_proc = ptm_qoswrr_write_proc;
#endif

#ifdef TCSUPPORT_QOS
	ptm_proc = create_proc_entry("tc3162/ptm_tcqos_disc", 0, NULL);
	ptm_proc->read_proc = ptm_tcqos_read_proc;
	ptm_proc->write_proc = ptm_tcqos_write_proc;
#endif
out:
  	return err;
}

static void __exit tc3262_ptm_exit(void)
{
	int bearer;
	int txq, rxq;
	char proc_name[32];

	for (bearer = PTM_B0; bearer <= PTM_B1; bearer += PTM_B1) {
		ptmReset(bearer);

		sprintf(proc_name, "tc3162/ptm_b%d_reset", bearer);
   		remove_proc_entry(proc_name, 0);
		sprintf(proc_name, "tc3162/ptm_b%d_stats", bearer);
   		remove_proc_entry(proc_name, 0);
		sprintf(proc_name, "tc3162/ptm_b%d_link_st", bearer);
   		remove_proc_entry(proc_name, 0);
		sprintf(proc_name, "tc3162/ptm_b%d_reg", bearer);
   		remove_proc_entry(proc_name, 0);

		for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
			remove_proc_entry(proc_txring_name[bearer][txq], NULL);
		}
		for (rxq = 0; rxq < RX_QUEUE_NUM; rxq++) {
			remove_proc_entry(proc_rxring_name[bearer][rxq], NULL);
		}
	}
	remove_proc_entry("tc3162/ptm_pripktchk", 0);
#ifdef LOOPBACK_SUPPORT
	remove_proc_entry("tc3162/ptm_loopback", 0);
	remove_proc_entry("tc3162/ptm_loopback_test", 0);
#endif

	for (bearer = PTM_B0; bearer <= PTM_B1; bearer += PTM_B1) {
		ptmDrvDescripReset(ptm_p[bearer]);

		unregister_netdev(tc3262_ptm_dev[bearer]);

		if (ptm_p[bearer]->ptmRxMemPool_p != NULL)
			dma_free_coherent(NULL, sizeof(ptmRxMemPool_t), ptm_p[bearer]->ptmRxMemPool_p, ptm_p[bearer]->ptmRxMemPool_phys_p);
		if (ptm_p[bearer]->ptmTxMemPool_p != NULL)
			dma_free_coherent(NULL, sizeof(ptmTxMemPool_t), ptm_p[bearer]->ptmTxMemPool_p, ptm_p[bearer]->ptmTxMemPool_phys_p);
	}

	for (bearer = PTM_B0; bearer <= PTM_B1; bearer += PTM_B1)
		free_netdev(tc3262_ptm_dev[bearer]);
}

/* Register startup/shutdown routines */
module_init(tc3262_ptm_init);
module_exit(tc3262_ptm_exit);
MODULE_LICENSE("GPL");
