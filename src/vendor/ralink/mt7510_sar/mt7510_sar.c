/*
** $Id: //BBN_Linux/Branch/Branch_for_MT7520_20120528/tclinux_phoenix/modules/private/tc3162l2hp2h/sar_tc3162l2.c#1 $
*/
/************************************************************************
 *
 *	Copyright (C) 2006 Trendchip Technologies, Corp.
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
#include <linux/kernel.h>
#include <linux/atmdev.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/TCIfSetQuery_os.h>
#include <asm/tc3162/ledcetrl.h>
#include "../include/tcversion.h"
#include "../tcphy/tcswitch.h"
#include "tsarm.h"
#include "../bufmgr/qdma_api.h"
#include "../fe_api/fe_api.h"

#ifdef SAR_VERIFY
#include "tsarm_verify.h"
#endif

#include <linux/mtd/rt_flash.h>
#include <linux/atmbr2684.h>
#include <linux/if_arp.h>

#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/ppp_channel.h>
#include <linux/atmppp.h>
#include <led.h>

// extern unsigned long flash_base;
#ifdef L2_AUTOPVC
#include "autoPVC.h"
#include "aal5.h"
extern uint32 g_manageFlag;
#endif


#ifdef CWMP
uint32 g_f5loopback_rxtime = 0;
uint32 g_f5loopback_txtime = 0;
uint32 maxtimeindex = 0xFFFFFFFF;
static uint32 cwmpSavedInF5Pkts = 0;
static uint32 cwmpSavedOutF5Pkts = 0;
uint8 internal_error = 0;
uint32 cwmpflag = 0;
//static uint32 cwmpSavedInF4Pkts = 0;
#endif


#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif

//#define MT7510_FPGA

#ifdef MT7510_FPGA
//#define SAR_CLK     (FE_CLK)/(4.0)
static float sar_clk = 7.5;
#else
static float sar_clk = 50;
#endif

static unsigned int recycle_tx = 0;
static unsigned int recycle_rx = 0;

#define BR2684_ETHERTYPE_LEN	2
#define BR2684_PAD_LEN			2

// frank
#define KERNEL_2_6_36			1

#ifndef TCSUPPORT_QOS
#define QOS_REMARKING 1
#endif
#define TCSUPPORT_HW_QOS

#ifdef QOS_REMARKING  /*Rodney_20090724*/
#define QOS_REMARKING_MASK    0x00000007
#define QOS_REMARKING_FLAG    0x00000001
//#define QOS_DMAWRR_USERDEFINE  0x01
#endif

#define LLC				0xaa, 0xaa, 0x03
#define SNAP_BRIDGED	0x00, 0x80, 0xc2
#define SNAP_ROUTED		0x00, 0x00, 0x00
#define PID_ETHERNET	0x00, 0x07
#define ETHERTYPE_IPV4	0x08, 0x00
#define ETHERTYPE_IPV6	0x86, 0xdd

#define PPP_PID_IPV4	0x00, 0x21
#define PPP_PID_IPV6	0x00, 0x57


#define PAD_BRIDGED		0x00, 0x00
#define MIN_PKT_SIZE    60

uint8 defMacAddr[] = {0x00, 0x00, 0xaa, 0xbb, 0xcc, 0xff};

static unsigned char ethertype_ipv4[] =
	{ ETHERTYPE_IPV4 };
static unsigned char ethertype_ipv6[] =
	{ ETHERTYPE_IPV6 };

static unsigned char ppp_pid_ipv4[] =
	{ PPP_PID_IPV4 };
static unsigned char ppp_pid_ipv6[] =
	{ PPP_PID_IPV6 };

enum br2684_encaps {
	e_br2684_vc  = BR2684_ENCAPS_VC,
	e_br2684_llc = BR2684_ENCAPS_LLC,
};

struct br2684_dev {
	struct net_device *net_dev;
	struct list_head br2684_devs;
	int number;
	struct list_head brvccs;    /* one device <=> one vcc (before xmas) */
	int mac_was_set;
	enum br2684_payload payload;
};

struct br2684_vcc {
	struct atm_vcc  *atmvcc;
	struct net_device *device;
	/* keep old push,pop functions for chaining */
	void (*old_push)(struct atm_vcc *vcc,struct sk_buff *skb);
	void (*old_pop)(struct atm_vcc *vcc,struct sk_buff *skb); 
	enum br2684_encaps encaps;
	struct list_head brvccs;
#ifdef CONFIG_ATM_BR2684_IPFILTER
	struct br2684_filter filter;
#endif /* CONFIG_ATM_BR2684_IPFILTER */
	unsigned copies_needed, copies_failed;
};

enum pppoatm_encaps {
	e_autodetect = PPPOATM_ENCAPS_AUTODETECT,
	e_pppoa_vc = PPPOATM_ENCAPS_VC,
	e_pppoa_llc = PPPOATM_ENCAPS_LLC,
};

struct pppoatm_vcc {
	struct atm_vcc	*atmvcc;	/* VCC descriptor */
	void (*old_push)(struct atm_vcc *, struct sk_buff *);
	void (*old_pop)(struct atm_vcc *, struct sk_buff *);
					/* keep old push/pop for detaching */
	enum pppoatm_encaps encaps;
	int flags;			/* SC_COMP_PROT - compress protocol */
	struct ppp_channel chan;	/* interface to generic ppp layer */
	struct tasklet_struct wakeup_tasklet;
};

/*
 * Header used for LLC Encapsulated PPP (4 bytes) followed by the LCP protocol
 * ID (0xC021) used in autodetection
 */
static const unsigned char pppllc[6] = { 0xFE, 0xFE, 0x03, 0xCF, 0xC0, 0x21 };


#if defined(WAN2LAN)
#define TX_STAG_LEN 6
#endif

#define LLC_LEN		(4)

/************************************************************************
*				E X T E R N A L   R E F E R E N C E S
*************************************************************************
*/

static DEFINE_RWLOCK(devs_lock);
static DEFINE_RWLOCK(devs_lock2);
static LIST_HEAD(br2684_devs);

#if defined(WAN2LAN)
extern void macSend(uint32 chanId, struct sk_buff *skb);
// extern int masko_on_off;
#endif

#ifdef SAR_VERIFY
extern uint16 test_flag;
#endif

#ifdef WAN2LAN
extern int masko; //use this flag control if open wan2lan function
#endif

#ifdef TR068_LED
extern int internet_led_on;
extern int internet_trying_led_on;
#if defined(TCSUPPORT_CPU_MT7510) ||  defined(TCSUPPORT_CPU_MT7520)
extern int internet_hwnat_pktnum;
extern int internet_hwnat_timer_switch;
#endif
#endif

#define DEBUG
#ifdef DEBUG
#define DPRINTK(format, args...) printk(KERN_DEBUG "br2684: " format, ##args)
#else
#define DPRINTK(format, args...)
#endif

#define SKB_DEBUG
#ifdef SKB_DEBUG
static void skb_debug(const struct sk_buff *skb)
{
#define NUM2PRINT 50
	char buf[NUM2PRINT * 3 + 1];	/* 3 chars per byte */
	int i = 0;
	for (i = 0; i < skb->len && i < NUM2PRINT; i++) {
		sprintf(buf + i * 3, "%2.2x ", 0xff & skb->data[i]);
	}
	printk(KERN_DEBUG "br2684: skb: %s\n", buf);
}
#else
#define skb_debug(skb)	do {} while (0)
#endif


/************************************************************************
*				V E R S I O N   C O N T R O L
*************************************************************************
*/


/************************************************************************
*							C O N S T A N T S
*************************************************************************
*/
#ifdef ALIGN32
#define ALIGN32BYTESBASE	32
#endif

#ifdef WAN2LAN
#define SKBUF_COPYTOLAN (1<<26) //xyzhu_nj_091105:borrow this bit to show if the packet is used for wan2lan
#endif
/************************************************************************
*							D A T A   T Y P E S
*************************************************************************
*/
#ifdef TR068_LED
	struct sar_stats {
		unsigned long	rx_pkts;		/* total pkts received	*/
		unsigned long	tx_pkts;		/* total pkts transmitted	*/
	};
#endif

/************************************************************************
*							M A C R O S
*************************************************************************
*/
#define dbg_pline_1(x)		printk("%s", x)
#define dbg_plineb_1(x, y)	printk("%s%02x", x, (uint8) y)
#define dbg_plinew_1(x, y)	printk("%s%04x", x, (uint16) y)
#define dbg_plinel_1(x, y)	printk("%s%08lx", x, (uint32) y)

#define puts(s)				printk("%s", s)
#define printf				printk

#define WORDSWAP(a) a = ((((a)>>24)&0xff) | (((a)>>8)&0xff00) | (((a)<<8)&0xff0000) | (((a)<<24)&0xff000000))
/************************************************************************
*				F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
int mt7510_atm_xmit_callback(void *pMsg, struct sk_buff *skb);
int mt7510_atm_recv_callback(void *pMsg, uint msg_len, struct sk_buff *skb, uint pkt_len);
int mt7510_atm_qdmaEventHandler(QDMA_EventType_t qdmaEventType);

uint8 atmCcHandler(atmOamCell_t *oamCellp);
uint8 atmOamHandler(atmOamCellPayload_t * oamCellPayloadp, uint8 pti, uint8 vc);

uint8 atmOamDataReq(uint8 *data_p, uint8 pti, uint8 vc);
uint8 atmVcNumberGet(uint8 vpi, uint16 vci);
struct sk_buff  *atmAal5DataInd(struct sk_buff *skb, uint8 vc, uint32 len, atmRxMsgBuf_t *pMsg);
uint8 atmDataReq(struct sk_buff *skb, uint8 vc, uint8 priority);
int atmOamOpen(void);
void atmIrqStSaved(uint32 irqst, uint8 vc);
int getATMState(char *stateATM);

static int atm_msg_buffer_init(uint txMsgNum, uint rxMsgNum);
static int setAtmQosRed(qosProfile_t *qos_p,	uint32 vc);
static inline int atm_push_tx_msg(struct atmMsgInfo_s *pMsg);
static inline struct atmMsgInfo_s *atm_pop_tx_msg(void);
static inline int atm_add_rx_msg(void);
static inline void atm_remove_rx_msg(void);

extern int getXdslSpeed(void);
extern uint8 getXdslModeType(void);
extern int ppp_channel_index(struct ppp_channel *);
extern int ppp_unit_number(struct ppp_channel *);
extern int eth_change_mtu(struct net_device *dev, int new_mtu);
extern int eth_validate_addr(struct net_device *dev);

int atm_init_wrappter(void);
int atm_exit_wrappter(void);
extern int br2684_init(void);
extern void br2684_exit(void);
extern void (*br2684_config_hook)(int linkMode, int linkType);
extern int (*br2684_init_hook)(struct atm_vcc *atmvcc, int encaps);
extern int (*br2684_push_hook)(struct atm_vcc *atmvcc, struct sk_buff *skb);
extern int (*br2684_xmit_hook)(struct sk_buff *skb, struct net_device *dev, struct br2684_vcc *brvcc);

extern int pppoatm_init(void);
extern void pppoatm_exit(void);
extern void (*pppoatm_config_hook)(int linkMode, int linkType);
extern int (*pppoatm_init_hook)(struct atm_vcc *atmvcc, int encaps);
extern int (*pppoatm_push_hook)(struct atm_vcc *atmvcc, struct sk_buff *skb);

extern netdev_tx_t br2684_start_xmit(struct sk_buff *skb, struct net_device *dev);

extern int napi_en;

#ifdef SAR_VERIFY
struct sk_buff *
atmDataLpbkHandler(struct sk_buff *skb, uint8 vc, uint32 len, atmRxMsgBuf_t *pMsg);
#endif

/************************************************************************
*						P U B L I C   D A T A
*************************************************************************
*/
atmCtrl_t atmCtrl;
atmCtrl_t *atm_p = &atmCtrl;

#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
#define QOS_DMAWRR_USERDEFINE  0x01
static int qos_wrr_info[5] = {0};
static int max_prio = 3;
static uint8 qos_wrr_user = 0x00;
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
static int qos_flag = NULLQOS;
#endif

#define DEF_PRIORITY_PKT_CHK_LEN         100

int priPktChkLen = DEF_PRIORITY_PKT_CHK_LEN;

#ifdef CONFIG_QOS
#ifdef TCSUPPORT_SBTHROUGHPUT_ENHANCE
extern int tc_qos_switch;
#endif
#endif

module_param(priPktChkLen, int, 0);

#ifdef PURE_BRIDGE
int priPktChk = 0;
#else
int priPktChk = 1;
#endif

module_param(priPktChk, int, 0);
/************************************************************************
*						E X T E R N A L   D A T A
*************************************************************************
*/
#ifdef SAR_VERIFY
extern uint8 sarVerifyDbg;
#endif
static uint8 sarDebugFlag = 0;
static uint8 sarSoftReset = 0;
#ifdef CONFIG_MIPS_TC3262
static spinlock_t sarLock;
static spinlock_t msgLock;

#define ATM_NAPI_MODE

#if defined(ATM_NAPI_MODE)
static spinlock_t napiLock;
#endif
#endif


#if defined(ATM_NAPI_MODE)
struct net_device *mt7510AtmDev = NULL;
struct napi_struct  mt7510Napi;
#endif
/************************************************************************
*						P R I V A T E   D A T A
*************************************************************************
*/
#ifdef TR068_LED
	static struct sar_stats sarStats;
        static struct timer_list mt7510sar_timer;
#endif

QDMA_InitCfg_t initCfg;

atmRxDescr_t *atmRxVcDescrp[ATM_VC_MAX];

uint8 atmTxCcCnt;
atmTxDescr_t *atmTxVcFreeDescrp[ATM_TX_PRIORITY_MAX][ATM_VC_MAX];
atmTxDescr_t *atmTxVcBusyDescrp[ATM_TX_PRIORITY_MAX][ATM_VC_MAX];

// frank used

int txcount1=0, txcount2=0;
int rxcount1=0, rxcount2=0;
uint16 volatile atmTxPriVcCnt[ATM_TX_PRIORITY_MAX+1][ATM_VC_MAX+1];
uint32 volatile atmTxVcTotalCnt;
uint16 volatile atmTxVcCnt[ATM_VC_MAX+1];

atmRxCcDescr_t *atmRxCcBusyDescrp;

atmConfig_t atmCfgTable = { 0 };
atmConfig_t atmCfgBackup = { 0 };
qosProfile_t qosRecord[ATM_VC_MAX];

#define CON_BR2684										0
#define CON_PPPOA										1
static int gLinkType = 0;
static int gLinkMode = 0;
static int gLinkPOE = 0;

atmMsg_t* gpMsg = NULL;
static int msg_cnt = 0;

int atmTxQueSize[4] = {	ATM_TX_VC_DESCR_P0_NUMMAX,
						ATM_TX_VC_DESCR_P1_NUMMAX,
						ATM_TX_VC_DESCR_P2_NUMMAX,
						ATM_TX_VC_DESCR_P3_NUMMAX};

module_param_array(atmTxQueSize, int, NULL, 0);

static uint8 RFC1483_B_LLC_HDR[] = {
	0xaa, 0xaa, 0x03, 0x00, 0x80, 0xc2, 0x00, 0x07, 0x00, 0x00
};
static uint8 RFC1483_R_LLC_HDR[] = {
	0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00
};
static uint8 RFC1483_B_LLC_FCS_HDR[] = {
	0xaa, 0xaa, 0x03, 0x00, 0x80, 0xc2, 0x00, 0x01, 0x00, 0x00
};
static uint8 RFC1483_B_VC_HDR[] = {
	0x00, 0x00
};
static uint8 PPPOA_LLC_HDR[] = {
	0xfe, 0xfe, 0x03, 0xcf, 0x00, 0x21
};
static uint8 PPPOA_VC_HDR[] = {
	0x00, 0x21
};

/* priority packet check parameters */

uint8 oamAisCell[52] = {
	/* ATM Header, without HEC */
	0x00, 0x00, 0x00, 0x00,
	/* 48 octets payload */
	0x10, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x03, 0xB9
};

uint8 oamRdiCell[52] = {
	/* ATM Header, without HEC */
	0x00, 0x00, 0x00, 0x00,
	/* 48 octets payload */
	0x11, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x00, 0xAF
};

uint8 oamLoopBackReqCell[52] = {
	/* ATM Header, without HEC */
	0x00, 0x00, 0x00, 0x00,
	/* 48 octets payload */
	0x18, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x03, 0x93
};

uint8 oamContinuityCheckReqCell[52] = {
	/* ATM Header, without HEC */
	0x00, 0x00, 0x00, 0x00,
	/* 48 octets payload */
	0x14, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x03, 0x87
};

uint8 oamF5AisCell[48] = {
	/* 48 octets payload */
	0x10, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x03, 0xB9
};

uint8 oamF5RdiCell[48] = {
	/* 48 octets payload */
	0x11, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x00, 0xAF
};

uint8 oamF5LoopBackReqCell[48] = {
	/* 48 octets payload */
	0x18, 0x01, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x03, 0x93
};

uint8 oamF5ContinuityCheckReqCell[48] = {
	/* 48 octets payload */
	0x14, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A,
	0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x6A, 0x03, 0x87
};

typedef  struct QosParm_s{
	uint32 cellRate;
	uint32  init;
	uint32 dec;
} QosParm_t;

#define MAX_ATMQOS_CNT        17  /* 1+8*2 = 17; 1:dslupRate;8 PVCs, 2:pcr and scr */
QosParm_t ATMQosParam[MAX_ATMQOS_CNT] = {{0}};

#ifdef MT7510_FPGA
int32 treg_tstbr = 9;	   //for FPGA
#else
uint32 treg_tstbr = 10172;	   //for ASIC
#endif
uint32 treg_tslr = 65537;	//0x00010001		default value

/************************************************************************
*			    L I N U X   S P E C I F I C     S E C T I O N
*************************************************************************
*/
static int mt7510_atm_open(struct atm_vcc *vcc);
static void mt7510_atm_close(struct atm_vcc *vcc);
static int mt7510_atm_ioctl(struct atm_dev *dev, unsigned int cmd, void *arg);
static int mt7510_atm_send(struct atm_vcc *vcc, struct sk_buff *skb);
static int mt7510_atm_change_qos (struct atm_vcc *vcc, struct atm_qos *qos,int flags);

static struct atm_dev *mt7510_atm_dev;
static struct atmdev_ops mt7510_atm_ops = {
	.open =			mt7510_atm_open,
	.close =     	mt7510_atm_close,
	.ioctl =      	mt7510_atm_ioctl,
	.send =       	mt7510_atm_send,
	.change_qos = 	mt7510_atm_change_qos,
	.owner = 		THIS_MODULE
};

/************************************************************************
*				F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#define sarRegRead(x)		regRead32(x)
#define sarRegWrite(x,y)	regWrite32(x, y)

/*_____________________________________________________________________________
**      function name: align32Byte
**      descriptions:
**          If you allocate memory with non-cache area and it is close neighbor cache area, If this not
**
**      parameters:
**           addr: Specify the address that you want to 32 bytes alignment.
**
**      global:
**           None
**
**      return:
**          32 bytes alignment address.
**
**      call:
**           None
**
**      revision:
**      1. Here 2008/09/08
**____________________________________________________________________________
*/
uint32
align32Byte(uint32 addr)
{
	#ifdef ALIGN32
	return (addr+31)&0xFFFFFFE0;
	#else
	return addr;
	#endif
}/*end align32Byte*/

static uint8 isPriorityPkt(uint8 *cp, uint8 vc, uint8 *priority, unsigned int len)
{
	uint16 etherType;
	uint8 ipVerLen;
	uint8 ipProtocol;
	uint8 tcpFlags;
	uint16 pppProtocol;
	uint8 ipv6_protocol, ipv6_type;

	if (atmCfgTable.qos[vc].mode == p_bridged)
		goto eth_header;
	else
		goto ipv4_header;

eth_header:
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
	/*20100921_serena_modify: check whether arp packet*/	
	} else if (etherType == 0x0806) {
		*priority = 3;
		return 1;	
	} 
	else if (etherType == 0x86dd) {
		cp += 6;
		ipv6_protocol = *(uint8*)cp;
		if (ipv6_protocol == 0x3a) {
			cp += 34;
			ipv6_type = *(uint8*)cp;
			if (ipv6_type == 0x87) {
				//printk("check ipv6 icmp packet.jiffies is %u\n", jiffies);
				*priority = 3;
				return 1;
			}
		}
	}
	else {
		/* check if ip packet */
		if (etherType != 0x0800)
			return 0;
	}

ipv4_header:

	// frank add 20121220 for pppoa mode
	pppProtocol = *(uint16 *) cp;
	/* check if LCP protocol, for pppoa control packet */
	if (pppProtocol == 0xc021) {
		*priority = 3;
		return 1;
	} else if(pppProtocol == 0x0021) {
		cp += 2; 					/* 6: PPPoE header 2: PPP protocol */
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
	
	/* check if TCP syn/ack/psh ack/fin ack */ /*20100921_serena_modify*/
	if ((((tcpFlags & 0x10) == 0x10) || (tcpFlags == 0x02)) && (len < priPktChkLen)) {
		*priority = 3;
		return 1;
	}

	return 0;
}

void delay1ms(int ms)
{
	volatile uint32 timer_now, timer_last;
	volatile uint32 tick_acc;
	uint32 one_tick_unit = 1 * SYS_HCLK * 1000 / 2;
	volatile uint32 tick_wait = ms * one_tick_unit;
	volatile uint32 timer1_ldv = sarRegRead(CR_TIMER1_LDV);

	tick_acc = 0;
 	timer_last = sarRegRead(CR_TIMER1_VLR);
	do {
   		timer_now = sarRegRead(CR_TIMER1_VLR);
       	if (timer_last >= timer_now)
       		tick_acc += timer_last - timer_now;
      	else
       		tick_acc += timer1_ldv - timer_now + timer_last;
     	timer_last = timer_now;
	} while (tick_acc < tick_wait);
}

void atmQosParmInit(void){
	 // 7510 FPGA
	 if(sar_clk < 10){ /*It's mean FAGA Clock rate, HCLK=25Mhz, SAR_CLK=6.125Mhz*/
		printk("7510 FPGA Qos Param Init\n");
	 	treg_tstbr=9;
	 }
	 // 7510 ASIC
	 else{
		printk("7510 ASIC Qos Param Init\n");
		treg_tstbr = 10172;
	 }

	 printk("ATM SAR Time Base: %d\n", treg_tstbr);
}

/*______________________________________________________________________________
**  atmInit
**
**  descriptions: the intialization function of ATM SAR module
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
void
atmInit(
	void
)
{
	uint8 i;
	uint32 flags, reg;

	char mac_addr[6];
#if 0
	for (i=0; i<6; i++) {
		mac_addr[i] = READ_FLASH_BYTE(flash_base + 0xff48 + i);
	}
#else
	for (i=0; i<6; i++) {
		mac_addr[i] = (0x6 + i);
	}
#endif

	printk("atmInit 1.1 Version\n");

	spin_lock_irqsave(&sarLock, flags);
	//disable Tx/Rx first to prevent error
	TSARM_GFR = 0x0;
	//reset SAR Module
	delay1ms(50); //delay1ms(50);	//check

	printk("ATM SAR Reset\n");

	reg = sarRegRead(0xbfb00834);
	reg |= (1<<7);
	sarRegWrite(0xbfb00834, reg);

//	TSARM_RAI = RAI_RESET_ENB(1);
//	delay1ms(1);//delay1ms(5);		//check
	delay1ms(50);//delay1ms(5);		//check
//	TSARM_RAI = RAI_RESET_ENB(0);
	reg &= (~(1<<7));
	sarRegWrite(0xbfb00834, reg);
	delay1ms(50);//delay1ms(5);		//check

	// enable gdm2 channel 
	fe_reg_modify_bits(0x152c, 0xffff, 16, 16);
	reg = ((RX_BUF_LEN << 16) | (0x0004 << 0));
	// set femac packet size limit 
	fe_reg_modify_bits(0x1524, reg, 0, 32);

	reg = ( ((uint32)(mac_addr[0]&0xff) << 24) | ((uint32)(mac_addr[1]&0xff) << 16) 
			| ((uint32)(mac_addr[2]&0xff) << 8)  | (mac_addr[3]&0xff) );
	sarRegWrite(0xbfb60890, reg);

	reg = ( ((uint32)(mac_addr[4]&0xff) << 24) | ((uint32)(mac_addr[5]&0xff) << 16) );
	sarRegWrite(0xbfb60894, reg);

	TSARM_RMPLR = 0xfa0;		// sar rx max packet length setting
	atmQosParmInit();
	TSARM_TSTBR = treg_tstbr;
	TSARM_TXSLRC = treg_tslr;

	/* Clear ATM SAR registers */
	//there are 10 VCs inside 3162L2
	for (i = 0; i < 10; i++)
	{
		// VC configuration
		TSARM_VCCR(i) = 0;
		// Traffic Scheduler - PCR
		TSARM_PCR(i) = 0;
		// Traffic Scheduler - SCR
		TSARM_SCR(i) = 0;
		// Traffic Scheduler - MBSTP
		TSARM_MBSTP(i) = 0;
		TSARM_VC_MPOA_CTRL(i) = 0;
	}
	//mpoa registers
	TSARM_MPOA_GCR = 0x0;
	//MPOA header insertion field values
	TSARM_MPOA_HFIV11 = 0xaaaa03;
	TSARM_MPOA_HFIV12 = 0xfefe03;
	TSARM_MPOA_HFIV13 = 0x0;
	TSARM_MPOA_HFIV21 = 0x0080c2;
	TSARM_MPOA_HFIV22 = 0x000000;
	TSARM_MPOA_HFIV23 = 0x0;
	TSARM_MPOA_HFIV31 = 0x0001;
	TSARM_MPOA_HFIV32 = 0x0007;
	TSARM_MPOA_HFIV33 = 0xfefe;
	TSARM_MPOA_HFIV41 = 0x0000;
	TSARM_MPOA_HFIV42 = 0x0800;
	TSARM_MPOA_HFIV43 = 0x03cf;

	TSARM_CCCR = 0x0;

	// cc channel
	TSARM_CCCR = VCCFGR_VPI(0) | VCCFGR_VCI(0) |
		VCCFGR_PORT(VCCFGR_ATM_PHY0) | VCCFGR_VALID;

//	atmOamOpen();
	if( sarSoftReset == 0) {
		atm_p = &atmCtrl;
		/* clear atm counters */
		memset((char *)&(atm_p->MIB_II), 0, sizeof(atmMIB_II_t));
		memset( (uint8*)&atmCfgTable, 0, sizeof(atmConfig_t));
	}

	TSARM_GFR = (GFR_TXENB | GFR_RXENB | (RAW_CELL_SIZE<<16));

	spin_unlock_irqrestore(&sarLock, flags);
}

typedef struct tsarm_ioctl{
	unsigned short int active;
	unsigned int vid;
	unsigned char vpi;
	unsigned char vci;
} tsarm_ioctl_t;

tsarm_ioctl_t vlan_vcc[ATM_VC_MAX] = {{0}};

#define RFC1483_B_LLC 			(0)
#define RFC1483_R_LLC 			1
#define RFC1483_B_VC  			2
#define PPPoA_LLC				3
#define PPPoA_VC      			4
#define RFC1483_R_VC			5


/*______________________________________________________________________________
**  atmCounterClear
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
void atmCounterClear(void)
{
	memset((char *)&(atm_p->MIB_II), 0, sizeof(atmMIB_II_t));
}

/*______________________________________________________________________________
**  atmPktsClear
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
void
atmPktsClear(
	void
)
{	
	int i;
	atm_p->MIB_II.inPkts = 0;
	atm_p->MIB_II.inF4Pkts = 0;
	atm_p->MIB_II.inF5Pkts = 0;
	atm_p->MIB_II.inDataPkts = 0;
	atm_p->MIB_II.outPkts = 0;
	atm_p->MIB_II.outF4Pkts = 0;
	atm_p->MIB_II.outF5Pkts = 0;
	atm_p->MIB_II.outDataPkts = 0;
	atm_p->MIB_II.inMpoaErr = 0;
	atm_p->MIB_II.inCrcErr = 0;
	atm_p->MIB_II.inCrc32Err = 0;
	atm_p->MIB_II.inCrc10Err = 0;
	atm_p->MIB_II.inActErr = 0;

	for (i=0; i<ATM_VC_MAX; i++){
		atm_p->MIB_II.outVcNum[i] = 0;
		atm_p->MIB_II.outSoftDropVcNum[i] = 0;
		atm_p->MIB_II.outHardDropVcNum[i] = 0;
	}
}

/*______________________________________________________________________________
**  atmAal5RealDataReq
**
**  descriptions: API for sending an AAL5 frame
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
uint8
atmAal5RealDataReq(
	struct sk_buff *skb, 
	uint8 vc
)
{
	uint8 priority = 0;
	uint8 new_priority;

#ifdef CONFIG_TX_POLLING_BY_MAC
	if (atmTxVcCnt[vc] >= 4){
		qdma_txdscp_recycle(0);
	}
#endif

#ifdef TCSUPPORT_QOS
	switch (qos_flag) {
		case QOS_SW_PQ:
			/* PQ mode */
			if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				priority = 2;
			}
			else if ((skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY) {
				priority = 1;
			}
			break;
		case QOS_HW_WRR:
			/* HW WRR mode */
			if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				//printk("lalala to first queue.\n");
				priority = 3;
			}
			else if ((skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY) {
				priority = 2;
			}
			else if ((skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY) {
				priority = 1;
			}
			else {
				priority = 0;
			}
			break;
		case QOS_HW_PQ:
			/* HW PQ mode */
			if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				priority = 3;
			}
			else if ((skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY) {
				priority = 2;
			}
			else if ((skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY) {
				priority = 1;
			}
			break;
		case NULLQOS:/*It's for putting rtp packets to HH priority when qos_flag not be selected as WRR or PQ*/
			if ((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				priority = 3;
			}
			break;

		default:
			break;
	}
#endif

	#ifdef QOS_REMARKING  /*Rodney_20090724*/
	if((skb->mark & QOS_REMARKING_FLAG)){
		priority = (uint8)((skb->mark & QOS_REMARKING_MASK) >> 1);
	}
	else{
		if (priPktChk && (skb->len < priPktChkLen)) {
			/*20100921_serena_modify: add inupu arg "len" to check tcp packet*/	
			if (isPriorityPkt(skb->data, vc, &new_priority, skb->len)) {
			#ifdef TCSUPPORT_QOS
				if (qos_flag == QOS_HW_WRR) {
					/* hw wrr mode, to handle special packet */
					priority = max_prio;
				}
				else {
					if (new_priority > priority) {
						priority = new_priority;
					}
				}
			#else
				priority = new_priority;
			#endif
			}
		}
	}
	#else
	if (priPktChk && (skb->len < priPktChkLen)) {
		/*20100921_serena_modify: add inupu arg "len" to check tcp packet*/	
		if (isPriorityPkt(skb->data, vc, &new_priority, skb->len)) {
		#ifdef TCSUPPORT_QOS
			if (qos_flag == QOS_HW_WRR) {
				/* hw wrr mode, to handle special packet */
				priority = max_prio;
			}
			else {
				if (new_priority > priority) {
					priority = new_priority;
				}
			}
		#else
			priority = new_priority;
		#endif
		}
	}
	#endif

	#ifdef SAR_VERIFY
	priority=isWrrPriorityPkt(skb, priority);
	#endif

#ifdef CONFIG_TX_POLLING_BY_MAC
	if (atmTxPriVcCnt[priority][vc] >= 16){
		qdma_txdscp_recycle(0);
	}
#endif

	if (atmTxVcCnt[vc] >= 64 || atmTxPriVcCnt[priority][vc] >= 16){
		atm_p->MIB_II.outDiscards++;
		atm_p->MIB_II.outSoftwareDiscards++;
		atm_p->MIB_II.outSoftDropVcNum[vc]++;
		if (ATM_SKB(skb)->vcc->pop) {
			ATM_SKB(skb)->vcc->pop (ATM_SKB(skb)->vcc, skb);
		} else {
			dev_kfree_skb_any(skb);
		}

#ifdef CONFIG_TX_POLLING_BY_MAC
		qdma_txdscp_recycle(0);
#endif
		return 1;
	}

	return ( atmDataReq(skb, vc, priority) );
}

uint8 atmDataReq(struct sk_buff *skb, uint8 vc, uint8 priority)
{
	int ret,pkt_len;
	struct atmMsgInfo_s *pMsg;
	atmTxMsgBuf_t *pTxMsg;
#ifdef TCSUPPORT_RA_HWNAT
	struct port_info atm_info;	
#endif	

#ifdef TCSUPPORT_RA_HWNAT
	if (ra_sw_nat_hook_tx != NULL) {
		atm_info.word = 0;
		atm_info.qatm.txq = (priority & 0xf);
		atm_info.qatm.vcnum = vc;
		atm_info.qatm.xoa = 1;
		if(atmCfgTable.qos[vc].mode == p_routed){
			if(atmCfgTable.qos[vc].encapType == CON_PPPOA)
				atm_info.qatm.pppoa = 1;
			else
				atm_info.qatm.ipoa = 1;
		}
	if (ra_sw_nat_hook_tx(skb, &atm_info, FOE_MAGIC_ATM) == 0) {			
			dev_kfree_skb_any(skb);
			return NETDEV_TX_OK;
		}
	}
#endif

	pMsg = atm_pop_tx_msg();
	pTxMsg = (atmTxMsgBuf_t *)(pMsg);

	if (pMsg == NULL){
		printk("atmDataReq: No Free Tx Message Buffer\n");
		dev_kfree_skb_any(skb);
		atm_p->MIB_II.outDiscards++;
		atm_p->MIB_II.outSoftDropVcNum[vc]++;
		return 1;
	}

	#ifdef SAR_VERIFY
	if(sarVerifyDbg){
		dumpDataCell(skb, priority);
	}
	#endif
	
	#ifdef SAR_VERIFY	
	setSarVerifyDataTxMsg(pTxMsg);
	#endif

	// fill priority setting
	pTxMsg->txMsgW0.queue = (priority & 0xf);

	// fill vc number setting
	pTxMsg->txMsgW0.vcNo = (vc & 0xf);

	// fill cell type setting (data packet)
	pTxMsg->txMsgW0.oam = 0;
	
	pTxMsg->txMsgW1.fPort = 2;

	pkt_len = skb->len;

//	printk("pTxMsg W0: %x\n", pTxMsg->txMsgW0);
//	printk("pTxMsg W1: %x\n", pTxMsg->txMsgW1);

	ret = qdma_transmit_packet(pTxMsg, sizeof(atmTxMsgBuf_t), skb);

	if (!ret){
		atmTxPriVcCnt[priority][vc]++;
		atmTxVcCnt[vc]++;
		atm_p->MIB_II.outPkts++;
		atm_p->MIB_II.outDataPkts++;
		atm_p->MIB_II.outVcNum[vc]++;
		atm_p->MIB_II.outBytes += pkt_len;
		
		// ledTurnOn(LED_DSL_ACT_STATUS);
		#ifdef TR068_LED
		/*for interner traffic led*/
		if(internet_led_on) {//IP connected and IP traffic is passing
			ledTurnOn(LED_INTERNET_STATUS);
			ledTurnOn(LED_INTERNET_ACT_STATUS);
		} 
		else {
			if(!internet_trying_led_on) {
				ledTurnOff(LED_INTERNET_STATUS);
				ledTurnOff(LED_INTERNET_TRYING_STATUS);
			}
		}

		#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
		tbs_led_data_blinking(led_internet_green);
		#endif
        
	#endif	
	} else {
		atm_p->MIB_II.outDiscards++;
		atm_p->MIB_II.outHardDropVcNum[vc]++;
		#ifdef SAR_VERIFY
		if(ATM_SKB(skb)->vcc ==NULL){
			dev_kfree_skb_any(skb);
			return 0;
		}
		#endif
		if (ATM_SKB(skb)->vcc->pop) {
			ATM_SKB(skb)->vcc->pop (ATM_SKB(skb)->vcc, skb);
		} else {
			dev_kfree_skb_any(skb);
		}
		memset(pTxMsg, 0, sizeof(atmTxMsgBuf_t));
		pMsg->next = NULL;
		atm_push_tx_msg(pMsg);
	}

	return ret;
}

#define CONFIG_VLAN_ATM
#ifdef CONFIG_VLAN_ATM
#define QOS_8021p_MARK			0x0F00 	/* 8~11 bits used for 802.1p */
#define QOS_8021P_0_MARK		0x08	/* default mark is zero */
#define QOS_8021P_TRANSPARENT	0x09
#define VLAN_HLEN				4
#define VLAN_ETH_ALEN			6

#define SET_ATM_VLAN			0x1111
#define DEL_ATM_PVC			    0x2222

uint8 getIndexVlanVcc( uint8 vpi, uint8 vci ) 
{
	uint8 vc;
	for ( vc = 0; vc < ATM_VC_MAX; vc++ ) {
		if ( vlan_vcc[vc].vpi == vpi && vlan_vcc[vc].vci == vci ) {
			return vc;
		}
	} 
	return ATM_DUMMY_VC;
}

int getEncapMode(uint8 *data)
{
	/*RFC1483 LLC Brided Mode*/
	if ( memcmp(data, RFC1483_B_LLC_HDR, 7 ) == 0 ) {
		return RFC1483_B_LLC;
	}
	/*RFC1483 LLC Routing Mode*/
	else if ( memcmp(data, RFC1483_R_LLC_HDR, sizeof(RFC1483_R_LLC_HDR)) == 0 ){
		return RFC1483_R_LLC;
	}
	/*PPPoA LLC Routing Mode*/
	else if ( memcmp(data, PPPOA_LLC_HDR, sizeof(PPPOA_LLC_HDR)) == 0 ){
		return PPPoA_LLC;
	}
	/*PPPoA VCMux Routing Mode*/
	else if ( memcmp(data, PPPOA_VC_HDR, sizeof(PPPOA_VC_HDR)) == 0 ){
		return PPPoA_VC;
	}
	/*RFC1483 VCMux Bridged Mode*/
	else if ( memcmp(data, RFC1483_B_VC_HDR, sizeof(RFC1483_B_VC_HDR)) == 0 ) {
		return RFC1483_B_VC;
	}
	else {
		return RFC1483_R_VC;
	}
}

int atmEncapLen[] = { 
	sizeof(RFC1483_B_LLC_HDR), 
	sizeof(RFC1483_R_LLC_HDR),
	sizeof(PPPOA_LLC_HDR),
	sizeof(PPPOA_VC_HDR),
	sizeof(RFC1483_B_VC_HDR)
};


static struct sk_buff * insert_vtag(struct sk_buff *skb, uint8 vc)
{
	uint8 encap_mode = 0, encap_len = 0;
	char * vlan_p = NULL, *ether_type_ptr = NULL;
	unsigned char ucprio = 0;
	unsigned char uc802prio = 0;
	
	int copy_len = 0;
	
	if ( skb->mark & QOS_8021p_MARK ) {
		/*vlan tagging*/
		encap_mode = getEncapMode((uint8*)skb->data);
		/*Note Ethernet Header*/
		if ( (encap_mode == RFC1483_B_VC)
			|| (encap_mode == PPPoA_LLC)
			|| (encap_mode == PPPoA_VC) ) {
				/*Nono ethernet header to do nothings*/
				return skb;	
			}
		
		encap_len = atmEncapLen[encap_mode];
		ether_type_ptr = skb->data + encap_len + 12;
		ucprio = (skb->mark & QOS_8021p_MARK) >> 8;
		
		if ( (ucprio < QOS_8021P_0_MARK) && (ucprio >= 0) ) { //0~7 remark
			uc802prio = ucprio;
		}
		else if ( QOS_8021P_0_MARK == ucprio ) {	//zero mark
			uc802prio = 0;
		}
		else if ( QOS_8021P_TRANSPARENT == ucprio ) {//pass through
			/*do nothing*/
			return skb;
		}
				
		if ( skb_headroom(skb) < VLAN_HLEN ) {
			struct sk_buff *sk_tmp = skb;
			skb = skb_realloc_headroom(sk_tmp, VLAN_HLEN);
			
			if ( ATM_SKB(sk_tmp)->vcc->pop ) {
				ATM_SKB(sk_tmp)->vcc->pop(ATM_SKB(sk_tmp)->vcc, sk_tmp);
			}
			else {
				dev_kfree_skb_any(sk_tmp);
			}
			
			if ( !skb ) {
				printk(KERN_ERR, "Vlan:failed to realloc headroom\n");
				return NULL;
			}
		}
		else {
			skb = skb_unshare(skb, GFP_ATOMIC);
			if ( !skb ) {
		//		printk(KERN_ERR, "Vlan: failed to unshare skbuff\n");
				return NULL;
			}
		}
		
		/*offset 4 bytes*/

		skb_push(skb, VLAN_HLEN);
		copy_len = encap_len + 2*VLAN_ETH_ALEN;
		
		/*move the mac address to the beginning of new header*/
		memmove(skb->data, skb->data+VLAN_HLEN, copy_len);
#if 0	
		printk("mac is %02x:%02x:%02x:%02x:%02x:%02x\n",*(skb->data+encap_len),\
				*(skb->data+encap_len+1),*(skb->data+encap_len+2),*(skb->data+encap_len+3),\
				*(skb->data+encap_len+4),*(skb->data+encap_len+5));
#endif

		vlan_p = skb->data + encap_len + 12;
		*(unsigned short *)vlan_p = 0x8100;
		
		vlan_p += 2;
		
		*(unsigned short *)vlan_p = 0;

		/*3 bits priority*/
		*(unsigned short*)vlan_p |= ((uc802prio & 0x7) << 13);
		
		/*12 bits vlan id*/
		*(unsigned short *)vlan_p |= (vlan_vcc[vc].vid & 0x0fff);
		//skb->mac.raw -= VLAN_HLEN;
		//skb->nh.raw -= VLAN_HLEN;
		skb->network_header -= VLAN_HLEN;
		skb->mac_header -= VLAN_HLEN;
	}
	
	return skb;
}

int resetVlanVcc(uint8 vpi, uint8 vci)
{
	uint8 vc = getIndexVlanVcc( vpi, vci );
	if ( (vc != ATM_DUMMY_VC) && (vc < ATM_VC_MAX) ) {
		memset( &vlan_vcc[vc], 0, sizeof(tsarm_ioctl_t) );
	}
	else {
		return -1;
	}
	return 0;
}

int setATMVlan( tsarm_ioctl_t *sar_ioctl )
{
	uint8 vc;
	/* reset this entry first */
	resetVlanVcc( sar_ioctl->vpi, sar_ioctl->vci );
	
	for( vc = 0; vc < ATM_VC_MAX; vc++ ) {
		if(vlan_vcc[vc].vpi == 0 && vlan_vcc[vc].vci == 0 ){
			vlan_vcc[vc].vpi = sar_ioctl->vpi;
			vlan_vcc[vc].vci = sar_ioctl->vci;
			vlan_vcc[vc].active = sar_ioctl->active;
			vlan_vcc[vc].vid = sar_ioctl->vid;
			return 0;
		}
	}
	return -1;
}
#endif

/*______________________________________________________________________________
**  atmAal5DataReq
**
**  descriptions: API for sending an AAL5 frame
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
uint8
atmAal5DataReq(
	struct sk_buff *skb,
	uint8 vpi,
	uint16 vci
)
{
	uint8 vc;

	vc = atmVcNumberGet(vpi, vci);
	if ( vc == ATM_DUMMY_VC ) {
		if (ATM_SKB(skb)->vcc->pop) {
			ATM_SKB(skb)->vcc->pop (ATM_SKB(skb)->vcc, skb);
		} else {
			dev_kfree_skb_any(skb);
		}
		return 1;
	}

 	return atmAal5RealDataReq(skb, vc);
}

/*______________________________________________________________________________
**  atmCcDataReq
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
uint8
atmCcDataReq(
	uint8 *data_p
)
{
	int ret;
	struct atmMsgInfo_s *pMsg;
	struct sk_buff *skb;
	atmTxMsgBuf_t *pTxMsg;
	unsigned int offset;

	skb = alloc_skb(52, GFP_ATOMIC | __GFP_NOWARN);
	pMsg = atm_pop_tx_msg();
	pTxMsg = (atmTxMsgBuf_t *)pMsg;

	if (skb == NULL){
		printk("atmCcDataReq: Alloc Skb Fail\n");
		return 1;
	}

	if (pMsg == NULL){
		printk("atmCcDataReq: No Free Tx Message Buffer\n");
		return 1;
	}

	if ( skb ) {
		dma_cache_inv((unsigned long)skb->data, 48);
		// four byte alignment
		offset = (uint)(skb->tail) & 3;
		if (offset){
			skb_reserve(skb, (4 - offset));
		}
	}

	memcpy(skb->data, data_p, 52);
	skb->len = 52;
	
	// fill vc number setting
	pTxMsg->txMsgW0.vcNo = 0xa;

	// fill cell type setting (data packet)
	pTxMsg->txMsgW0.oam = 1;

	pTxMsg->txMsgW1.fPort = 2;

	ret = qdma_transmit_packet(pTxMsg, sizeof(atmTxMsgBuf_t), skb);

	if (!ret){
		txcount1++;
		atmTxCcCnt++;
		// 4th queue, 9th vc channel
		atmTxPriVcCnt[4][ATM_VC_MAX]++;
		// cc channel
		atmTxVcCnt[ATM_VC_MAX]++;
		TSARM_TXMBCSR |= 1 << 16;
	} else {
		dev_kfree_skb_any(skb);
		memset(pTxMsg, 0, sizeof(atmTxMsgBuf_t));
		pMsg->next = NULL;	
		atm_push_tx_msg(pMsg);
	}

	return ret;
}


/*______________________________________________________________________________
**  atmOamDataReq
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
uint8
atmOamDataReq(
	uint8 *data_p,
	uint8 pti,
	uint8 vc
)
{
	int ret;
	struct atmMsgInfo_s *pMsg;
	struct sk_buff *skb;
	atmTxMsgBuf_t *pTxMsg;
	unsigned int offset;

	skb = alloc_skb(48, GFP_ATOMIC | __GFP_NOWARN);

	pMsg = atm_pop_tx_msg();
	pTxMsg = (atmTxMsgBuf_t *)pMsg;

	if (skb == NULL){
		printk("atmOamDataReq: Alloc Skb Fail\n");
		return 1;
	}

	if (pMsg == NULL){
		printk("atmOamDataReq: No Free Tx Message Buffer\n");
		return 1;
	}

	if (skb){
		dma_cache_inv((unsigned long)skb->data, 48);
		// four byte alignment
		offset = (uint)(skb->tail) & 3;
		if (offset){
			skb_reserve(skb, (4 - offset));
		}
	}

	memcpy(skb->data, data_p, 48);
	skb->len = 48;
	
	// fill vc number setting
	pTxMsg->txMsgW0.vcNo = (vc & 0xf);

	// fill payload type setting (oam packet)
	pTxMsg->txMsgW0.pti	= (pti & 0x7);

	// fill cell type setting (oam packet)
	pTxMsg->txMsgW0.oam = 1;

	pTxMsg->txMsgW1.fPort = 2;

	ret = qdma_transmit_packet(pTxMsg, sizeof(atmTxMsgBuf_t), skb);

	if (!ret){
		atmTxCcCnt++;
		txcount1++;
		// oam/cc using 4th queue
		atmTxPriVcCnt[4][vc]++;
		atmTxVcCnt[vc]++;
		TSARM_TXMBCSR |= 1 << vc;
	} else {
		dev_kfree_skb_any(skb);
		memset(pTxMsg, 0, sizeof(atmTxMsgBuf_t));
		pMsg->next = NULL;
		atm_push_tx_msg(pMsg);
	}

	return ret;
}

#if defined(CWMP)
/*______________________________________________________________________________
**  atmOamF4F5DataReq
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
uint32
atmOamF4F5DataReq(
	uint8 vpi,
	uint16 vci,
	uint8 f5,
	uint8 endToEnd,
	uint8 funcType
)
{
	atmOamCell_t *oamCellp;
	atmOamCellPayload_t *oamCellPayloadp;
	uint8 vc;
	uint8 pti;

	if (f5){ /* F5: by PTI */
		/* Move to non-cacheable region */
		printk("F5 \n");
		switch (funcType){
		case 0: /* AIS */
			printk("AIS Cell\n");
			oamCellPayloadp = (atmOamCellPayload_t *)(oamF5AisCell);
			break;
		case 1: /* RDI */
			printk("RDI Cell\n");
			oamCellPayloadp = (atmOamCellPayload_t *)(oamF5RdiCell);
			break;
		case 2: /* Loopback */
			printk("Loopback Cell\n");
			oamCellPayloadp = (atmOamCellPayload_t *)(oamF5LoopBackReqCell);
			break;
		case 3: /* Continuity check */
			printk("Continuity Check Cell\n");
			oamCellPayloadp = (atmOamCellPayload_t *)(oamF5ContinuityCheckReqCell);
			break;
		default:
			return 1;
		}
		vc = atmVcNumberGet(vpi, vci);
		if (vc == ATM_DUMMY_VC){
			/* This is not a opened VC */
			return 1;
		}
		if (endToEnd){
			pti = 5;
		}
		else {
			pti = 4;
		}
		if (!atmOamDataReq((uint8 *)oamCellPayloadp, pti, vc)){
			atm_p->MIB_II.outF5Pkts++;
			atm_p->MIB_II.outPkts++;
			//add by brian
			#ifdef CWMP
			if(cwmpflag)
			{
				cwmpSavedOutF5Pkts++;
			}
			#endif
		}
	}
	else { /* F4: by VCI */
		printk("F4\n");
		/* Move to non-cacheable region */
		switch (funcType){
		case 0: /* AIS */
			printk("AIS Cell\n");
			oamCellp = (atmOamCell_t *)(oamAisCell);
			break;
		case 1: /* RDI */
			printk("RDI Cell\n");
			oamCellp = (atmOamCell_t *)(oamRdiCell);
			break;
		case 2: /* Loopback */
			printk("Loopback Cell\n");
			oamCellp = (atmOamCell_t *)(oamLoopBackReqCell);
			break;
		case 3: /* Continuity check */
			printk("Continuity Check Cell\n");
			oamCellp = (atmOamCell_t *)(oamContinuityCheckReqCell);
			break;
		default:
			return 1;
		}
		oamCellp->vpi = vpi;
		if (endToEnd){
			oamCellp->vci = 4;
		}
		else {
			oamCellp->vci = 3;
		}
		if (!atmCcDataReq((uint8 *)oamCellp)){
			atm_p->MIB_II.outF4Pkts++;
			atm_p->MIB_II.outPkts++;
		}
	}
	return 0;
}
#endif


void initMPOA(uint32 vc, qosProfile_t *qos_p)
{
	printk("Init VC %ld MPOA Function\n", vc);

	//re-init reg
	TSARM_MPOA_GCR &= ~(1<<(vc+16) | (1<<vc));
	TSARM_VC_MPOA_CTRL(vc) = 0x0;

	//MPOA global control register
	if(qos_p->mode == p_routed){				//router mode
		printk("MPOA Mode: MODE_ROUTER\n");
		if(qos_p->encapType == CON_PPPOA){
			printk("encapType: CON_PPPOA\n");
				TSARM_MPOA_GCR |= 1 << (vc+16) | 1 << vc;
		} else {
			printk("encapType: Not PPPOA\n");
				TSARM_MPOA_GCR |= 0 << (vc+16) | 1 << vc;
		}
	}else if(qos_p->mode == p_bridged){			//bridge mode
		printk("MPOA Mode: MODE_BRIDGE\n");
		TSARM_MPOA_GCR |= 0 << (vc+16) | 0 << vc;
	} else {
		printk("MPOA Mode Error\n");
	}

	//VC 0 ~ 9 control register
	if (qos_p->muxType == MUX_VC){
		printk("MuxType: MUX_VC\n");
		if (qos_p->mode == p_routed){
			printk("Mode: MODE_ROUTER\n");
			if (qos_p->encapType == CON_PPPOA)
				TSARM_VC_MPOA_CTRL(vc) |= 0xc200;
			else
				TSARM_VC_MPOA_CTRL(vc) |= 0x8200;
		} else {
			printk("Mode: MODE_BRIDGE\n");
			TSARM_VC_MPOA_CTRL(vc) |= 0x0340;
		}
	} else if(qos_p->muxType == MUX_LLC){			//MUX_LLC
		printk("MuxType: MUX_LLC\n");
		if (qos_p->mode == p_routed){
			printk("Mode: MODE_ROUTER\n");
			if (qos_p->encapType == CON_PPPOA)
				TSARM_VC_MPOA_CTRL(vc) |= 0xe3f0;
			else
				TSARM_VC_MPOA_CTRL(vc) |= 0xa309;
		} else {
			printk("Mode: MODE_BRIDGE\n");
			TSARM_VC_MPOA_CTRL(vc) |= 0x2365;
		}
	}
}

uint32
mt7510AtmMbsCalCulate(
	uint32 pcr,
	uint32 scr,
	uint32 mbs
)
{
	uint32 cal_mbs;
	cal_mbs = (uint32)(mbs*(1-(scr * (1 /(float)pcr))));
	if(cal_mbs == 0)
		cal_mbs = 1;
	return cal_mbs;
}

void
mt7510AtmRateCalCulate(
	uint16 tb,
	uint32 cellRate,
	uint32 *dec_p,
	uint32 *init_p
)
{
	uint32 dec, init;
	uint32 calCellRate;
	uint32 diffSquare = 0xffffffff;
	uint32 closesetSquare = 0xff;

	float calRateParam=0;
	float calRateParam_f=0;
	int i = 0, j = 0;

	/* find ATM Qos parameter in table */
	for(i = 0; i < MAX_ATMQOS_CNT; i++){
		if((ATMQosParam[i].dec == 0)&&(ATMQosParam[i].init == 0))
			continue;
		else
		{
			if(ATMQosParam[i].cellRate == cellRate){
				*dec_p = ATMQosParam[i].dec;
				*init_p = ATMQosParam[i].init;
				return;
			}
		}
	}

	//calRateParam=(float)SAR_CLK*1000000;
	calRateParam_f=(float)sar_clk*1000000*(1/(float)(tb+1));
	
	for ( dec = 1; dec <= 0x07f; dec++ ) {    /* PCR_dec or SCR_dec are both 7 bits long */
		diffSquare = 0xffffffff;
		calRateParam=calRateParam_f*(float)dec;
		for ( init = dec; init <= 0x0fff; init++ ) {  /* PCR_init or SCR_init are both 12 bits long */
       		//calCellRate= calRateParam * dec * (1/(float)(tb+1)) * (1/(float)init);
			calCellRate= calRateParam*(1/(float)init); 
			diffSquare = calCellRate - cellRate;
			if ( diffSquare < closesetSquare ) {
				closesetSquare = diffSquare;
				*dec_p = dec;
				*init_p = init;
				if ( diffSquare == 0 ) {
					/* Perfectly match */
					goto Store; /*store the parameter to table and you can find next time when cellRate is the same*/
					//return;
				}
			}
		}
	}
Store:
	for(j = 0; j < MAX_ATMQOS_CNT; j++){
		if((ATMQosParam[j].dec == 0)&&(ATMQosParam[j].init == 0)){
			ATMQosParam[j].cellRate = cellRate;
			ATMQosParam[j].dec = *dec_p;
			ATMQosParam[j].init = *init_p;
			break;
		}	
	}

	if(j == MAX_ATMQOS_CNT){ /*The table is full and modify from the last one*/
		ATMQosParam[j-1].cellRate = cellRate;
		ATMQosParam[j-1].dec = *dec_p;
		ATMQosParam[j-1].init = *init_p;
	}
	return;
}

/*______________________________________________________________________________
**  setAtmQosRed
**
**  descriptions:Used to set Atm Qos register
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:atmAal5VcOpen,update_qosred
**  call:
**____________________________________________________________________________*/
static int
setAtmQosRed(
	qosProfile_t *qos_p,	
	uint32 vc
)
{
	uint32 linkMode = 0;		//ADSL
	uint32 dslUpRate;
	uint32 pcr_dec;
	uint32 scr_dec;
	uint32 lr_dec;
	uint32 pcr_init;
	uint32 scr_init;
	uint32 lr_init;
	uint32 cal_mbs;

	switch (qos_p->type)
	{
	    case CBR	: qos_p->type = TSARM_QOS_CBR;		break;
	    case UBR	: qos_p->type = TSARM_QOS_UBR;		break;
	    case VBR	: qos_p->type = TSARM_QOS_rtVBR;	break;
	    case nrtVBR	: qos_p->type = TSARM_QOS_nrtVBR;	break;
	    default     : qos_p->type = TSARM_QOS_UBR;		break;
	}

	dslUpRate = getXdslSpeed(); /* Kbps */

	if (dslUpRate <= 0)
	{
		#ifdef MT7510_FPGA
		printk("FPGA dslUpRate Setting\n");
		dslUpRate = 1100;
		#else
		printk("ASIC dslUpRate Setting\n");
		dslUpRate = 1024;
		#endif
	}
	
	if (linkMode == 1){
		printk("ATM over VDSL mode init\n");
		dslUpRate = (dslUpRate * 1000 * 55) / (53 * 8);	/* cell rate */
	} else {
		printk("ATM over ADSL mode init\n");
		dslUpRate = (dslUpRate * 1000) / (53 * 8);	/* cell rate */
	}

	if(dslUpRate % 53 !=0){
		dslUpRate=dslUpRate+1;
	}

	printk("dslUpRate: %d\n", dslUpRate);

	if ((qos_p->pcr == 0) || (qos_p->pcr > dslUpRate))
		qos_p->pcr = dslUpRate;
	if ((qos_p->scr == 0) || (qos_p->scr > qos_p->pcr))
		qos_p->scr = qos_p->pcr;
	if (qos_p->mbs == 0)
		qos_p->mbs = 1;

	if (linkMode == 1)
		sarRegWrite(0xbfb60b80, 0x1);
	else 
		sarRegWrite(0xbfb60b80, 0x0);

	//init line rate
	mt7510AtmRateCalCulate((uint16)treg_tstbr, dslUpRate, &lr_dec, &lr_init);
	if (linkMode == 1){
		sarRegWrite(0xbfb60b84, lr_init);
		sarRegWrite(0xbfb60b88, lr_dec);
	} else {
		TSARM_TXSLRC = (lr_dec << 16) | lr_init;
	}

	/* ----- Set up vpi, vci, and QoS parameters of the vc ----- */
	//calculate rate.......
	if (( qos_p->type == TSARM_QOS_CBR) || (qos_p->type == TSARM_QOS_UBR) ) {
		mt7510AtmRateCalCulate((uint16)treg_tstbr, qos_p->pcr, &scr_dec, &scr_init);
		if(getXdslModeType() == ME_CMD_ADSL_ANNEXM){
			qos_p->mbs = 2; 
		}
		else{
			qos_p->mbs = 1; 								// if CBR, mbs must be 1
		}
		
		if (linkMode == 1){
			sarRegWrite(0xbfb60c80, scr_init);
			sarRegWrite(0xbfb60cc0, scr_dec);
			sarRegWrite(0xbfb60c00, scr_init);
			sarRegWrite(0xbfb60c40, scr_dec);
		} else {
			TSARM_SCR(vc) = (scr_dec << 16) | scr_init;
			TSARM_PCR(vc) = (scr_dec << 16) | scr_init;
		}

		if(qos_p->type == TSARM_QOS_CBR)
			TSARM_MBSTP(vc) = (1 << 28) |(0 << 26) |(qos_p->mbs << 2) | qos_p->type;
		else
			TSARM_MBSTP(vc) = (1 << 28) |(3 << 26) |(qos_p->mbs << 2) | qos_p->type;
	} else {
		mt7510AtmRateCalCulate((uint16)treg_tstbr, qos_p->pcr, &pcr_dec, &pcr_init);
		mt7510AtmRateCalCulate((uint16)treg_tstbr, qos_p->scr, &scr_dec, &scr_init);	

		if (linkMode == 1){
			sarRegWrite(0xbfb60c00, pcr_init);
			sarRegWrite(0xbfb60c40, pcr_dec);
			sarRegWrite(0xbfb60c80, scr_init);
			sarRegWrite(0xbfb60cc0, scr_dec);
		} else {
			TSARM_PCR(vc) = (pcr_dec << 16) | pcr_init;
			TSARM_SCR(vc) = (scr_dec << 16) | scr_init;
		}

		cal_mbs = mt7510AtmMbsCalCulate(qos_p->pcr, qos_p->scr, qos_p->mbs);
		if(qos_p->type == TSARM_QOS_rtVBR){
			TSARM_MBSTP(vc) = (1 << 28) |(1 << 26) | (cal_mbs << 2) | qos_p->type;    //for new L2 SAR
			sarRegWrite(0xbfb60d00, cal_mbs);
		} else {
			TSARM_MBSTP(vc) = (1 << 28) |(2 << 26) | (cal_mbs << 2) | (2 << 0);
			sarRegWrite(0xbfb60d00, cal_mbs);
		}
	}

	if (sarDebugFlag){
		dbg_plinel_1("\r\n vc:", vc);
		dbg_plinel_1(" PCR:", TSARM_PCR(vc));
		dbg_plinel_1(" SCR:", TSARM_SCR(vc));
		dbg_plinel_1(" MBSTP:", TSARM_MBSTP(vc));
		dbg_plinel_1("\r\n TSARM_MPOA_GCR:", TSARM_MPOA_GCR);
		dbg_plinel_1(" TSARM_VC_MPOA_CTRL:", TSARM_VC_MPOA_CTRL(vc));
		dbg_pline_1("\r\n");
	}
	return 0;
}


int findVcChan(int vpi, int vci)
{
	int i;
	for (i=0; i<ATM_VC_MAX; i++){
		if (atmCfgTable.openFlag[i] == 1){
			if ((atmCfgTable.vpi[i]==vpi) && (atmCfgTable.vci[i]==vci))
				break;
		}
	}

	if ((i>=0) && (i<ATM_VC_MAX))
		return i;
	else
		return -1;
}


int getFreeVcChan(void)
{
	int i, vc = 0;

	if ( atmCfgTable.vcNumber >= ATM_VC_MAX ) {
		return -1;
	}

	for ( i = 0; i < ATM_VC_MAX; i++ ){
		if ( atmCfgTable.openFlag[i] == 0 ){
			atmCfgTable.openFlag[i] = 1;
			vc = i;
			atmCfgTable.vcNumber++;
			break;
		}
	}

	if ((vc>=0) && (vc<ATM_VC_MAX))
		return vc;
	else
		return -1;
}


/*______________________________________________________________________________
**  atmAal5VcOpen
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
int
atmAal5VcOpen(
	uint8 vpi,
	uint16 vci,
	qosProfile_t *qos_p,
	struct atm_vcc *vcc
)
{
	uint32 vc = ATM_VC_MAX;
	uint32 temp;

	printk("mt7510_atm_open\n");
	printk("vpi: %d\n", vpi);
	printk("vci: %d\n", vci);

	printk("atmAal5VcOpen vc %ld:\n", vc);

	setAtmQosRed(qos_p,vc);

	// read to clear Receive Counter
	temp = TSARM_TDCNT(vc);
	temp = TSARM_RDCNT(vc);

	// set up VC Configuration as valid =1, raw =0(AAL5 mode), port =0, VPI, VCI
	TSARM_VCCR(vc) =
		VCCFGR_VPI(vpi) | VCCFGR_VCI(vci) |
		VCCFGR_PORT(VCCFGR_ATM_PHY0) | VCCFGR_VALID;

	return 0;
}

/*______________________________________________________________________________
**  atmAal5VcClose
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
uint8
atmAal5VcClose(
	uint8 vpi,
	uint16 vci
)
{
	uint8 vc, i;

	vc = atmVcNumberGet(vpi, vci);
	if ( vc == ATM_DUMMY_VC )
		return 1;

	// VC configuration
	TSARM_VCCR(vc) = 0;
	// Traffic Scheduler - PCR
	TSARM_PCR(vc) = 0;
	// Traffic Scheduler - SCR
	TSARM_SCR(vc) = 0;
	// Traffic Scheduler - MBSTP
	TSARM_MBSTP(vc) = 0;

	atmCfgTable.openFlag[vc] = 0;
	atmCfgTable.vpi[vc] = atmCfgTable.vci[vc] = 0;
	atmCfgTable.vcc[vc] = NULL;
	atmCfgTable.vcNumber--;

	qdma_set_channel_retire(vc);

	// workaround for abnormal irq report
	for ( i = 0; i < ATM_VC_MAX; i++ ) {
		if ( atmCfgTable.openFlag[i] == 1 ) {
			TSARM_VCCR(10) =
				VCCFGR_VPI(atmCfgTable.vpi[i]) | VCCFGR_VCI(atmCfgTable.vci[i]) |
				VCCFGR_PORT(VCCFGR_ATM_PHY0) | VCCFGR_VALID;
			break;
		}
	}
	if (i == ATM_VC_MAX)
		TSARM_VCCR(10) = VCCFGR_VPI(255) | VCCFGR_VCI(15) |
			VCCFGR_PORT(VCCFGR_ATM_PHY0) | VCCFGR_VALID;

#ifdef CONFIG_VLAN_ATM
	resetVlanVcc(vpi, vci);
#endif

	return 0;
}

/*______________________________________________________________________________
**  atmOamOpen
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
int
atmOamOpen(
	void
)
{
	TSARM_CCCR = VCCFGR_VALID;

	return 0;
}


/*______________________________________________________________________________
**  atmRegDump
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
int
atmRegDump(uint16 vc)
{
	uint16	index=0;

	char buf[1000];

	memset(buf, 0x0, sizeof(buf));

	printk("Dump SAR register\n");
	index += sprintf( buf+index, "\n\r***** ATM SAR Module: Common Register Dump *****");
	index += sprintf( buf+index, "\n\rReset&Identify reg   = 0x%08lx, General CFG reg         = 0x%08lx",
		TSARM_RAI, TSARM_GFR);
	index += sprintf( buf+index, "\n\rTraffic Sched. TB reg= 0x%08lx, RX Max Length reg       = 0x%08lx",
		TSARM_TSTBR, TSARM_RMPLR);
	index += sprintf( buf+index, "\n\rP23 Data c /s reg    = 0x%08lx, TX Traffic Sched. LR reg= 0x%08lx",
		TSARM_TXDBCSR_P23, TSARM_TXSLRC);
	index += sprintf( buf+index, "\n\rP01 Data c /s reg    = 0x%08lx, TX OAM ctrl/stat reg    = 0x%08lx",
		TSARM_TXDBCSR_P01, TSARM_TXMBCSR);
	index += sprintf( buf+index, "\n\rRX Data ctrl/stat reg= 0x%08lx, RX OAM ctrl/stat reg    = 0x%08lx",
		TSARM_RXDBCSR, TSARM_RXMBCSR);
	index += sprintf( buf+index, "\n\r");
	index += sprintf( buf+index, "\n\r***** ATM SAR Module: VC(%02d) Register Dump *****\n\r", vc);
	index += sprintf( buf+index, "\n\rTX Traffic PCR       = 0x%08lx, TX Traffic SCR          = 0x%08lx",
		TSARM_PCR(vc), TSARM_SCR(vc));
	index += sprintf( buf+index, "\n\rTX Traffic MBS/Type  = 0x%08lx",
		TSARM_MBSTP(vc));
	index += sprintf( buf+index, "\n\rTX Total Data Count  = 0x%08lx, RX Total Data Count     = 0x%08lx",
		TSARM_TDCNT(vc), TSARM_RDCNT(vc));
	index += sprintf( buf+index, "\n\rTX CC Total Count    = 0x%08lx, RX CC Total Count       = 0x%08lx",
		TSARM_TDCNTCC, TSARM_RDCNTCC);
	index += sprintf( buf+index, "\n\rRX Mis-insert Cnt.   = 0x%08lx, TX AAL5 Count           = 0x%08x",
		TSARM_MISCNT, atmTxVcCnt[vc]);
	index += sprintf( buf+index, "\n\r");

	buf[index] = '\0';

	printk("%s\n", buf);
	return index;
}

/*______________________________________________________________________________
**  atmCounterDisplay
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
void
atmCounterDisplay(
	void
)
{
	int		index = 0;
	char *stateATM;

	stateATM = (char *)kzalloc(2048, GFP_KERNEL);
	index = getATMState( stateATM );

	printf("%s", stateATM);
	kfree(stateATM); 
}


int
getATMState(char *stateATM)
{
	uint16	index=0;

	index += sprintf( stateATM+index, "\n[ SAR Counters ]\n");
	index += sprintf( stateATM+index, "inPkts         = 0x%08lx, inDataPkts     = 0x%08lx\n",
		atm_p->MIB_II.inPkts, atm_p->MIB_II.inDataPkts);
	index += sprintf( stateATM+index, "inF4Pkts       = 0x%08lx, inF5Pkts       = 0x%08lx\n",
		atm_p->MIB_II.inF4Pkts, atm_p->MIB_II.inF5Pkts);
	index += sprintf( stateATM+index, "inCrcErr       = 0x%08lx\n", atm_p->MIB_II.inCrcErr);
	index += sprintf( stateATM+index, "inDiscards     = 0x%08lx\n", atm_p->MIB_II.inDiscards);
	index += sprintf( stateATM+index, "outPkts        = 0x%08lx, outDataPkts    = 0x%08lx\n",
		atm_p->MIB_II.outPkts, atm_p->MIB_II.outDataPkts);

	index += sprintf( stateATM+index, "outVc0Pkts         = 0x%08lx, outVc1Pkts            = 0x%08lx\n",
		atm_p->MIB_II.outVcNum[0], atm_p->MIB_II.outVcNum[1]);
	index += sprintf( stateATM+index, "outVc2Pkts         = 0x%08lx, outVc3Pkts            = 0x%08lx\n",
		atm_p->MIB_II.outVcNum[2], atm_p->MIB_II.outVcNum[3]);
	index += sprintf( stateATM+index, "outVc4Pkts         = 0x%08lx, outVc5Pkts            = 0x%08lx\n",
		atm_p->MIB_II.outVcNum[4], atm_p->MIB_II.outVcNum[5]);
	index += sprintf( stateATM+index, "outVc6Pkts         = 0x%08lx, outVc7Pkts            = 0x%08lx\n",
		atm_p->MIB_II.outVcNum[6], atm_p->MIB_II.outVcNum[7]);
	index += sprintf( stateATM+index, "outVc8Pkts         = 0x%08lx, outVc9Pkts            = 0x%08lx\n",
		atm_p->MIB_II.outVcNum[8], atm_p->MIB_II.outVcNum[9]);

	index += sprintf( stateATM+index, "outSoftDropVc0Pkts = 0x%08lx, outSoftDropVc1Pkts    = 0x%08lx\n",
		atm_p->MIB_II.outSoftDropVcNum[0], atm_p->MIB_II.outSoftDropVcNum[1]);
	index += sprintf( stateATM+index, "outSoftDropVc2Pkts = 0x%08lx, outSoftDropVc3Pkts    = 0x%08lx\n",
		atm_p->MIB_II.outSoftDropVcNum[2], atm_p->MIB_II.outSoftDropVcNum[3]);
	index += sprintf( stateATM+index, "outSoftDropVc4Pkts = 0x%08lx, outSoftDropVc5Pkts    = 0x%08lx\n",
		atm_p->MIB_II.outSoftDropVcNum[4], atm_p->MIB_II.outSoftDropVcNum[5]);
	index += sprintf( stateATM+index, "outSoftDropVc6Pkts = 0x%08lx, outSoftDropVc7Pkts    = 0x%08lx\n",
		atm_p->MIB_II.outSoftDropVcNum[6], atm_p->MIB_II.outSoftDropVcNum[7]);
	index += sprintf( stateATM+index, "outSoftDropVc8Pkts = 0x%08lx, outSoftDropVc9Pkts    = 0x%08lx\n",
		atm_p->MIB_II.outSoftDropVcNum[8], atm_p->MIB_II.outSoftDropVcNum[9]);

	index += sprintf( stateATM+index, "outHardDropVc0Pkts = 0x%08lx, outHardDropVc1Pkts    = 0x%08lx\n",
		atm_p->MIB_II.outHardDropVcNum[0], atm_p->MIB_II.outHardDropVcNum[1]);
	index += sprintf( stateATM+index, "outHardDropVc2Pkts = 0x%08lx, outHardDropVc3Pkts    = 0x%08lx\n",
		atm_p->MIB_II.outHardDropVcNum[2], atm_p->MIB_II.outHardDropVcNum[3]);
	index += sprintf( stateATM+index, "outHardDropVc4Pkts = 0x%08lx, outHardDropVc5Pkts    = 0x%08lx\n",
		atm_p->MIB_II.outHardDropVcNum[4], atm_p->MIB_II.outHardDropVcNum[5]);
	index += sprintf( stateATM+index, "outHardDropVc6Pkts = 0x%08lx, outHardDropVc7Pkts    = 0x%08lx\n",
		atm_p->MIB_II.outHardDropVcNum[6], atm_p->MIB_II.outHardDropVcNum[7]);
	index += sprintf( stateATM+index, "outHardDropVc8Pkts = 0x%08lx, outHardDropVc9Pkts    = 0x%08lx\n",
		atm_p->MIB_II.outHardDropVcNum[8], atm_p->MIB_II.outHardDropVcNum[9]);


	index += sprintf( stateATM+index, "outF4Pkts      = 0x%08lx, outF5Pkts      = 0x%08lx\n",
		atm_p->MIB_II.outF4Pkts, atm_p->MIB_II.outF5Pkts);
	index += sprintf( stateATM+index, "softRstCnt     = 0x%08lx\n", atm_p->MIB_II.softRstCnt);
	index += sprintf( stateATM+index, "outDiscards    = 0x%08lx\n", atm_p->MIB_II.outDiscards);
	index += sprintf( stateATM+index, "outSoftDrops   = 0x%08lx\n", atm_p->MIB_II.outSoftwareDiscards);
	index += sprintf( stateATM+index, "inCrc32Err     = 0x%08lx\n", atm_p->MIB_II.inCrc32Err);
	index += sprintf( stateATM+index, "inCrc10Err     = 0x%08lx\n", atm_p->MIB_II.inCrc10Err);
	index += sprintf( stateATM+index, "inL4CsErr      = 0x%08lx\n", atm_p->MIB_II.inL4CsErr);
	index += sprintf( stateATM+index, "inIp4CsErr     = 0x%08lx\n", atm_p->MIB_II.inIp4CsErr);
	index += sprintf( stateATM+index, "inActErr       = 0x%08lx\n", atm_p->MIB_II.inActErr);
	index += sprintf( stateATM+index, "inBufLenErr    = 0x%08lx\n", atm_p->MIB_II.inBufLenErr);
	index += sprintf( stateATM+index, "inMpoaErr      = 0x%08lx, inVlanHit      = 0x%08lx\n",
		atm_p->MIB_II.inMpoaErr, atm_p->MIB_II.inVlanHit);
	index += sprintf( stateATM+index, "inBytes        = 0x%08lx, outBytes       = 0x%08lx\n",
		atm_p->MIB_II.inBytes, atm_p->MIB_II.outBytes);//yzwang_20091125
	return index;
}


/*______________________________________________________________________________
**  atmCcHandler
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
uint8
atmCcHandler(
	atmOamCell_t *oamCellp
)
{
	uint8 ret = 0;
	uint8 isF4 = 0;
	uint32 i;

	if ( oamCellp->oamCellType != OAM_FAULT_MANAGEMENT ) {
		ret = 1;
	}
	if ((oamCellp->vci == 3) || (oamCellp->vci == 4)) {
		isF4 = 1;
		atm_p->MIB_II.inF4Pkts++;
	}

	atm_p->MIB_II.inPkts++;

	switch ( oamCellp->oamFuncType ) {
	case OAM_LOOPBACK:
		/* Loopback Indication field:
		** 1 - source point, 0 - loopback point
		*/
		if ( oamCellp->payload[0] == 0x00 ) {
			ret = 1;
		}
		else {
			oamCellp->payload[0] = 0x00;
		}
		break;
	case OAM_AIS:
		oamCellp->oamFuncType = OAM_RDI;
		break;
	default:
		ret = 1;
		break;
	}
	if ( !ret ) {
		if(!atmCcDataReq((uint8 *)oamCellp)){
			if(isF4){
				atm_p->MIB_II.outF4Pkts++;
			}
			atm_p->MIB_II.outPkts++;
		}
	}

	return ret;
}

/*______________________________________________________________________________
**  atmOamHandler
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
uint8
atmOamHandler(
	atmOamCellPayload_t * oamCellPayloadp,
	uint8 pti,
	uint8 vc
)
{
	uint8 ret = 0;
	uint8 isF4 = 0;
	uint32 i;

	if (oamCellPayloadp->oamCellType != OAM_FAULT_MANAGEMENT){
		ret = 1;
	}

	if ((pti == 4) || (pti == 5)) {
		isF4 = 0;
		atm_p->MIB_II.inF5Pkts++;
	}

	atm_p->MIB_II.inPkts++;

	//add by brian
	#ifdef CWMP
	if(cwmpflag)
	{
		g_f5loopback_rxtime = jiffies;
		cwmpSavedInF5Pkts++;
	}
	#endif

	switch (oamCellPayloadp->oamFuncType){
	case OAM_LOOPBACK:
		/* Loopback Indication field:
		** 1 - source point, 0 - loopback point
		*/
		if (oamCellPayloadp->payload[0] == 0x00){
			ret = 1;
		}
		else {
			oamCellPayloadp->payload[0] = 0x00;
		}
		break;
	case OAM_AIS:
		oamCellPayloadp->oamFuncType = OAM_RDI;
		break;
	default:
		ret = 1;
		break;
	}

	if ( !ret ) {
		if(!atmOamDataReq((uint8 *)oamCellPayloadp, pti, vc)) {
			if(!isF4){
				atm_p->MIB_II.outF5Pkts++;
			}
			atm_p->MIB_II.outPkts++;
		}
	}

	return ret;
}

/*______________________________________________________________________________
**  atmVcNumberGet
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
uint8
atmVcNumberGet(
	uint8 vpi,
	uint16 vci
)
{
	uint8 vc;

	for ( vc = 0; vc < ATM_VC_MAX; vc++ ) {
		if ( atmCfgTable.openFlag[vc] &&
			 (atmCfgTable.vpi[vc] == vpi) &&
		     (atmCfgTable.vci[vc] == vci) ) {
			return vc;
		}
	}

	return ATM_DUMMY_VC;
}

/*______________________________________________________________________________
**  atm
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
void
atmReset(
	void
)
{
	unsigned long flags;

	if (sarDebugFlag) {
		dbg_pline_1("\r\n atmReset");
	}

	spin_lock_irqsave(&sarLock, flags);

	// disable rx
	TSARM_GFR = 0x0;
	delay1ms(1);

	atmInit();						/* reset sar & init descriptor */
#if 0
	for( i = 0; i < atmCfgBackup.vcNumber; i++ ){	/* open vc according to backup table*/
		atmAal5VcOpen( atmCfgBackup.vpi[i], atmCfgBackup.vci[i], &qosRecord[i], atmCfgBackup.vcc[i]);
	}
#endif
	spin_unlock_irqrestore(&sarLock, flags);
}


void dump_skb(struct sk_buff *skb)
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


/*______________________________________________________________________________
**  atmAal5DataInd
**
**  descriptions:
**  execution sequence:
**  commands process:
**  parameters:
**  local:
**  global:
**  return:
**  called by:
**  call:
**____________________________________________________________________________*/
struct sk_buff  *
atmAal5DataInd(
	struct sk_buff *skb,
	uint8 vc,
	uint32 len,
	atmRxMsgBuf_t *pMsg
)
{
	struct sk_buff *freeSkb = NULL;
	struct atm_vcc *vcc;
	unsigned int offset;	
	uint8 raw = 0;
	int i;
	uint16 etherType = 0;
	uint16 protocol = 0;

	atm_p->MIB_II.inPkts++;
	atm_p->MIB_II.inDataPkts++;
	atm_p->MIB_II.inBytes += len;//yzwang_20091125
	freeSkb = skbmgr_dev_alloc_skb2k();

	if (freeSkb){
		// four byte alignment
		offset = (uint)(freeSkb->tail) & 3;
		if (offset){
			skb_reserve(freeSkb, (4 - offset));
		}

		vcc = atmCfgTable.vcc[vc];
		if (unlikely(vcc == NULL)) {
			dev_kfree_skb_any(skb);
			atm_p->MIB_II.inDiscards++;
			return freeSkb;
		}

		ATM_SKB(skb)->vcc = vcc;
		skb_put(skb, len);

// Frank
#if 0
		protocol = *(uint16 *)(&skb->data[20]);
		etherType = *(uint16 *) (&skb->data[12]);

#if 0
		if ((etherType == 0x0800) || (protocol == 0x0021)){
			printk("ATM SAR 1.3\n");
			dump_skb(skb);
			printk("\n\n");
		}
#endif
#endif

#if defined(WAN2LAN)
/* do wan2lan after skb_put,
 * because skb->len starts to have value from it */

		if(masko)
		{
			struct sk_buff *skb2 = NULL;
			uint32 port_id = 0;

			//Check the skb headroom is enough or not. shnwind 20100121.
			if (skb_headroom(skb) < TX_STAG_LEN)
				skb2 = skb_copy_expand(skb, TX_STAG_LEN, skb_tailroom(skb) , GFP_ATOMIC);
			else
				skb2 = skb_copy(skb, GFP_ATOMIC);

			if(skb2 == NULL)
				printk("\nFAILED: wan2lan skb2 allocation in atm rx direction.\n");
			else
			{
#if 0
				if ((skb2->data[0]==0x00) && (skb2->data[1]==0x88) && (skb2->data[2]==0x77)
                 && (skb2->data[3]==0x87) && (skb2->data[4]==0x87) && (skb2->data[5]==0x87)){
					// Frank Test Wan2Lan
					skb2->data[0] = 0x00;
					skb2->data[1] = 0x99;
					skb2->data[2] = 0x99;
					skb2->data[3] = 0x99;
					skb2->data[4] = 0x99;
					skb2->data[5] = 0x99;
				}
#endif
				skb2->mark |= SKBUF_COPYTOLAN;
				port_id = (masko>>24) - 1;
				macSend(port_id,skb2); //tc3162_mac_tx
			}
		}
#endif

		if (atm_charge(vcc, skb->truesize) == 0){
			dev_kfree_skb_any(skb);
			atm_p->MIB_II.inDiscards++;
			return freeSkb;
		}

#ifdef TCSUPPORT_RA_HWNAT
		if (ra_sw_nat_hook_rxinfo)
			ra_sw_nat_hook_rxinfo(skb, FOE_MAGIC_ATM, &pMsg->rxMsgW1, sizeof(rxMsgWord1_t));
				
		if (ra_sw_nat_hook_rx != NULL){
			if (ra_sw_nat_hook_rx(skb)){
				/*pass it up to kernel networking layer and update stats*/
				
				if (atmCfgTable.qos[vc].mode == p_routed){
					raw = pMsg->rxMsgW0.raw;
					if (!raw){
						skb_pull(skb, 12);
					}
				}

				vcc->push(vcc, skb);
			} 
		} else
#endif
		{

			if (atmCfgTable.qos[vc].mode == p_routed){
				raw = pMsg->rxMsgW0.raw;
				if (!raw){
					skb_pull(skb, 12);
				} 
			}

			/*pass it up to kernel networking layer and update stats*/
			vcc->push(vcc, skb);
		}
		atomic_inc(&vcc->stats->rx);
		#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
		tbs_led_data_blinking(led_internet_green);
		#endif        
	}
	else {
		atm_p->MIB_II.inDiscards++;	
		freeSkb = skb;
	}

	return freeSkb;
}

/************************************************************************
*		     A T M    D E V I C E   O P S  D E F I N I T I O N S
*************************************************************************
*/

/******************************************************************************
 Packet Transmit
******************************************************************************/
/******************************************************************************
******************************************************************************/
static inline int atm_push_tx_msg(struct atmMsgInfo_s *pMsg)
{
	unsigned long flags;

	spin_lock_irqsave(&msgLock, flags);

	if (!pMsg){
		printk("atm_push_tx_msg: Error insert tx message\n");
		return -1;
	}

	if (pMsg->next != NULL) {
		printk("The TX Message Buffer is not return from tx used pool\n") ;
		return -1 ;
	}
	
	if(!gpMsg->txHeadPtr) {
		gpMsg->txHeadPtr = pMsg;
		gpMsg->txTailPtr = pMsg;
	} else {
		if (!gpMsg->txTailPtr){
            printk("atm_push_tx_msg: tail pointer error\n");
		} else {
			gpMsg->txTailPtr->next = pMsg;
			gpMsg->txTailPtr = gpMsg->txTailPtr->next ;
		}
	}
	msg_cnt++;

	spin_unlock_irqrestore(&msgLock, flags);
	return 0 ;
}

/******************************************************************************
******************************************************************************/
static inline struct atmMsgInfo_s *atm_pop_tx_msg()
{
	unsigned long flags;
	struct atmMsgInfo_s *pMsg;
	
	spin_lock_irqsave(&msgLock, flags);

	pMsg = gpMsg->txHeadPtr;
	if(gpMsg->txHeadPtr == gpMsg->txTailPtr) {
		gpMsg->txHeadPtr = NULL;
		gpMsg->txTailPtr = NULL;
	} else {
		gpMsg->txHeadPtr = gpMsg->txHeadPtr->next;
	}
	msg_cnt--;
	if(pMsg != NULL) {
		pMsg->next = NULL;
	}
	
	//clear Tx message before using
	memset(&pMsg->txMsg,0,sizeof(atmTxMsgBuf_t));
	
	spin_unlock_irqrestore(&msgLock, flags);
	return pMsg;
}


/******************************************************************************
 Packet Receive
******************************************************************************/
/******************************************************************************
******************************************************************************/
static inline int atm_add_rx_msg(void) 
{
	gpMsg->rxEndIdx++;

	if (gpMsg->rxEndIdx == gpMsg->rxMsgNum){
		gpMsg->rxEndIdx = 0;
	}

	if (gpMsg->rxEndIdx == gpMsg->rxMsgNum){
		printk("ATM SAR RX Message Buffer is Full\n");
		gpMsg->rxEndIdx--;
		if (gpMsg->rxEndIdx < 0){
			gpMsg->rxEndIdx = gpMsg->rxMsgNum;
		}
		return -1;
	}
	
	return (gpMsg->rxEndIdx);
}


static inline void atm_remove_rx_msg(void)
{
	if (gpMsg->rxEndIdx == gpMsg->rxStartIdx){
		printk(" ATM SAR RX Message Buffer is Empty\n");
	} else {
		gpMsg->rxEndIdx--;
		if (gpMsg->rxEndIdx < 0){
			gpMsg->rxEndIdx = gpMsg->rxMsgNum;
		}
	}
}

/******************************************************************************
******************************************************************************/
static int atm_msg_buffer_init(uint txMsgNum, uint rxMsgNum)
{
	struct atmMsgInfo_s *txMsgBaseAddr;
	dma_addr_t dmaTxAddr, dmaRxAddr;
	uint32 rxMsgBaseAddr, i;
	uint8 vc, priority;

	printk("atm_msg_buffer_init\n");

	if((txMsgNum>4095 || txMsgNum<=0) || (rxMsgNum>2048 || rxMsgNum<=0)){
		printk("ATM SAR: initial parameters are invalid.\n") ;
		return -EFAULT ;
	}
	gpMsg->txMsgNum = txMsgNum;
	gpMsg->rxMsgNum = rxMsgNum;
	
	/******************************************
	* Allocate descriptor DMA memory          *
	*******************************************/

	#ifdef MSG_CACHED		// cache address
	txMsgBaseAddr = (struct atmMsgInfo_s*)dma_alloc_noncoherent(NULL, sizeof(struct atmMsgInfo_s)*(txMsgNum), &dmaTxAddr, GFP_KERNEL);
	#else					// uncache address
	txMsgBaseAddr = (struct atmMsgInfo_s*)dma_alloc_coherent(NULL, sizeof(struct atmMsgInfo_s)*(txMsgNum), &dmaTxAddr, GFP_KERNEL);
	#endif

	if(!txMsgBaseAddr){
		printk("Allocate memory for TX Message Buffer failed.\n"); 
		return -ENOMEM ;
	}

	gpMsg->txMsgBaseAddr = txMsgBaseAddr;
	memset((struct atmMsgInfo_s*)txMsgBaseAddr, 0, (txMsgNum) * sizeof(struct atmMsgInfo_s));

	for (i=0; i<txMsgNum; i++){	
		txMsgBaseAddr[i].next = NULL;
		atm_push_tx_msg(&txMsgBaseAddr[i]);
	}

	#ifdef MSG_CACHED		// cache address
	rxMsgBaseAddr = (uint)dma_alloc_noncoherent(NULL, sizeof(atmRxMsgBuf_t)*(rxMsgNum), &dmaRxAddr, GFP_KERNEL);
	#else					// uncache address
	rxMsgBaseAddr = (uint)dma_alloc_coherent(NULL, sizeof(atmRxMsgBuf_t)*(rxMsgNum), &dmaRxAddr, GFP_KERNEL);
	#endif	

	if(!rxMsgBaseAddr){
		printk("Allocate memory for RX Message Buffer failed.\n"); 
		return -ENOMEM ;
	}

	gpMsg->rxMsgBaseAddr = rxMsgBaseAddr;

	for(vc=0; vc < ATM_VC_MAX; vc++){
		for(priority = 0; priority < ATM_TX_PRIORITY_MAX; priority++){
			atmTxPriVcCnt[priority][vc] = 0;
		}
	}

	atmTxVcCnt[vc] = 0;

	return 0 ;
}


static int atm_msg_buffer_free(int txMsgNum, int rxMsgNum)
{
	dma_addr_t dmaRxAddr, dmaTxAddr;
	struct atmMsgInfo_s *pMsg;

	printk("atm_msg_buffer_free\n");

#ifdef MSG_CACHED		// cache address
	// note
	dma_free_noncoherent(NULL, sizeof(atmRxMsgBuf_t)*(rxMsgNum), (atmRxMsgBuf_t*)gpMsg->rxMsgBaseAddr, &dmaRxAddr);
#else
	dma_free_coherent(NULL, sizeof(atmRxMsgBuf_t)*(rxMsgNum), (atmRxMsgBuf_t*)gpMsg->rxMsgBaseAddr, &dmaRxAddr);
#endif

	while ((pMsg = atm_pop_tx_msg()) != NULL);

#ifdef MSG_CACHED		// cache address
	dma_free_noncoherent(NULL, sizeof(struct atmMsgInfo_s)*(txMsgNum), (struct atmMsgInfo_s*)gpMsg->txMsgBaseAddr, &dmaTxAddr);
#else
	dma_free_coherent(NULL, sizeof(struct atmMsgInfo_s)*(txMsgNum), (struct atmMsgInfo_s*)gpMsg->txMsgBaseAddr, &dmaTxAddr);
#endif

	return 0;
}


#if defined(ATM_NAPI_MODE)
int mt7510_atm_qdmaEventHandler(QDMA_EventType_t qdmaEventType)
{
	unsigned long flags;

	if (qdmaEventType == QDMA_EVENT_RECV_PKTS)
	{
		//isr can't be preempted in our kernel, so this
		//spin_lock can be removed ? --Trey
		spin_lock_irqsave(&napiLock, flags);
		#if KERNEL_2_6_36
		if (napi_schedule_prep(&mt7510Napi)) {
			qdma_disable_rxpkt_int();
			__napi_schedule(&mt7510Napi);
		}
		#else
		//because no dev info available, we always use nas8's poll function.
		if (netif_rx_schedule_prep(mt7510AtmDev))
		{
			qdma_disable_rxpkt_int();
			__netif_rx_schedule(mt7510AtmDev);
		}
		#endif
		spin_unlock_irqrestore(&napiLock, flags);
	}
	else if (qdmaEventType == QDMA_EVENT_NO_RX_BUFFER)
	{
		printk("\nQDMA_EVENT_NO_RX_BUFFER\n");
	}
	else if (qdmaEventType == QDMA_EVENT_TX_CROWDED)
	{
		printk("\nQDMA_EVENT_TX_CROWDED\n");
	}
	else
	{
		printk("\nWrong QDMA Event Type: %d\n", qdmaEventType);
	}

	return 0;
}


#if KERNEL_2_6_36
int br2684_napiPoll(struct napi_struct *napi, int budget)
{
	int rx_work_limit=0;
	int received = 0;
	int n=0, done=0;
	unsigned long flags=0;

	rx_work_limit = min(budget - received, budget);

	received = qdma_receive_packets(rx_work_limit);

	if (received < budget) {

		spin_lock_irqsave(&napiLock, flags);

		__napi_complete(napi);

		qdma_enable_rxpkt_int();

		spin_unlock_irqrestore(&napiLock, flags);
	}

	return received;
}
#else
int br2684_napiPoll(struct net_device *dev, int *budget)
/* Because qdmaEventHandler has no dev info, 
 * we always use nas8's poll function.
 */
	int rx_work_limit = min(dev->quota, *budget);
	int received = 0;
	int done;
	unsigned long flags;

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

	return done ? 0 : 1;
}
#endif
#endif

int mt7510_atm_xmit_callback(void *pMsg, struct sk_buff *skb)
{
	uint32 type, vc, priority;
	dma_addr_t dmaTxAddr;
	atmTxMsgBuf_t *pTxMsg = (atmTxMsgBuf_t *)(pMsg);
	struct atmMsgInfo_s *pTxMsgInfo = ( struct atmMsgInfo_s *)(pMsg);

	if (recycle_tx){
		if (skb != NULL){
			dev_kfree_skb_any(skb);
		}

		if (pTxMsgInfo != NULL){
			memset(pTxMsg, 0, sizeof(atmTxMsgBuf_t));
			pTxMsgInfo->next = NULL;
			atm_push_tx_msg(pTxMsgInfo);
		}

		return 0;
	}	
    if (skb == NULL)
    {
        printk("\nERROR: skb is NULL at mt7510_atm_xmit_callback\n");
        return 0;
    }

    if (pTxMsg == NULL)
    {
		printk("\nERROR: txMsg is NULL at mt7510_atm_xmit_callback\n");
        return 0;
    }

	type = pTxMsg->txMsgW0.oam;

	vc = pTxMsg->txMsgW0.vcNo;

	if (type){
		priority = 4;
	} else {
		priority = pTxMsg->txMsgW0.queue;
	}

	atmTxPriVcCnt[priority][vc]--;
	atmTxVcCnt[vc]--;

	memset(pTxMsg, 0, sizeof(atmTxMsgBuf_t));
	pTxMsgInfo->next = NULL;
	atm_push_tx_msg(pTxMsgInfo);

	if (type){
		kfree_skb(skb);
	} else {
		dev_kfree_skb_any(skb);
	}

	return 0;
}


int mt7510_atm_recv_callback(void *pMsg, uint msg_len, struct sk_buff *skb, uint pkt_len)
{
	uint8 type, vc, pti, crcErr, crc10Err, crc32Err, len_err, inact_err;
	uint8 mpoaErr, l4CsErr, l4Valid, ip4CsErr, ipv6, ipv4;
	atmRxMsgBuf_t *pRxMsg;
	dma_addr_t dmaRxAddr;
	struct sk_buff *newSkb;
	unsigned int offset;
	int ret = 0;
	int i = 0;

	if (recycle_rx){
		if (skb != NULL){
			dev_kfree_skb_any(skb);
		}
		return 0;
	}	

	if (pMsg == NULL || skb == NULL){
		printk("\nERROR: rxMsg&skb is NULL at mt7510_atm_recv_callback\n");
		return 0;
	}

	pRxMsg = (atmRxMsgBuf_t*)pMsg;

//	printk("RxMsg W0: %x\n", pRxMsg->rxMsgW0);
//	printk("RxMsg W1: %x\n", pRxMsg->rxMsgW1);
//	printk("RxMsg W2: %x\n", pRxMsg->rxMsgW2);
//	printk("RxMsg W3: %x\n", pRxMsg->rxMsgW3);

	vc = pRxMsg->rxMsgW0.vcNo;
	type = pRxMsg->rxMsgW0.oam;

	ipv4 = pRxMsg->rxMsgW1.ipv4;
	ipv6 = pRxMsg->rxMsgW1.ipv6;
	l4Valid = pRxMsg->rxMsgW1.l4Valid;

	if (pRxMsg->rxMsgW1.l4CsErr)
		atm_p->MIB_II.inL4CsErr++;

	if (pRxMsg->rxMsgW1.ip4CsErr)
		atm_p->MIB_II.inIp4CsErr++;

	if (pRxMsg->rxMsgW0.lenErr)
		atm_p->MIB_II.inBufLenErr++;

	if (pRxMsg->rxMsgW0.inactErr)
		atm_p->MIB_II.inActErr++;

	if (pRxMsg->rxMsgW0.crcErr)
		atm_p->MIB_II.inCrcErr++;
		
	if (pRxMsg->rxMsgW2.crc10Err)
		atm_p->MIB_II.inCrc10Err++;

	if (pRxMsg->rxMsgW2.crc32Err)
		atm_p->MIB_II.inCrc32Err++;

	if (pRxMsg->rxMsgW0.mpoaErr)
		atm_p->MIB_II.inMpoaErr++;
	
	if (pRxMsg->rxMsgW1.l4CsErr | pRxMsg->rxMsgW1.ip4CsErr | pRxMsg->rxMsgW0.lenErr 
      | pRxMsg->rxMsgW0.inactErr | pRxMsg->rxMsgW0.crcErr | pRxMsg->rxMsgW2.crc10Err 
	  | pRxMsg->rxMsgW2.crc32Err | pRxMsg->rxMsgW0.mpoaErr){
		newSkb = skb;
		goto recv_ok;
//		dev_kfree_skb_any(skb);
		return 0;
	}

	// oam cell
	if (type){
		pti = pRxMsg->rxMsgW0.pti;
		if (vc == 0x0a) {		/* CC ...for L2-> vc = 0x0a means oam or cc cell. */
			#ifndef SAR_VERIFY
			atmCcHandler((atmOamCell_t *)(skb->data));
			#else
			atmCcLpbkHandler((atmOamCell_t *)(skb->data));
			#endif
		} else {				/* OAM ...for L2-> vc = 0x0a means oam or cc cell. */
			#ifndef SAR_VERIFY
			atmOamHandler((atmOamCellPayload_t *)(skb->data), pti, vc);
			#else
			atmOamLpbkHandler((atmOamCellPayload_t *)(skb->data), pti, vc);
			#endif
		}

		// memory may be not enough
		newSkb = skb;
		dma_cache_inv((unsigned long)newSkb->data, RX_BUF_LEN);
		offset = (uint)(newSkb->tail) & 3;
		if (offset){
			skb_reserve(newSkb, (4 - offset));
		}
	// data cell
	} else {
		#ifdef  SAR_VERIFY
		newSkb=atmDataLpbkHandler(skb, vc, pkt_len, pRxMsg);
		#else
		if (pkt_len && (pkt_len <= RX_BUF_LEN)) {
			newSkb = atmAal5DataInd(skb, vc, pkt_len, pRxMsg);
		} else {
			atm_p->MIB_II.inDiscards++;
			newSkb = skbmgr_dev_alloc_skb2k();
			if (newSkb){
				dev_kfree_skb_any(skb);
				// four byte alignment
				offset = (uint)(newSkb->tail) & 3;
				if (offset){
					skb_reserve(newSkb, (4 - offset));
				}
			} else {
				newSkb = skb;
				offset = (uint)(newSkb->tail) & 3;
				if (offset){
					skb_reserve(newSkb, (4 - offset));
				}
			}
		}
		#endif
	}

recv_ok:

	memset(pRxMsg, 0, sizeof(atmRxMsgBuf_t));

	if (qdma_has_free_rxdscp()){
		ret = qdma_hook_receive_buffer(pRxMsg, sizeof(atmRxMsgBuf_t), newSkb);
	} else {
		printk("\nSAR Driver RX Error: no available QDMA RX descritor\n");
		dev_kfree_skb_any(newSkb);
	}

	if (ret){
		printk("qdma hook receive buffer fail!\n");
	}


	return 0;
}

static int mt7510_atm_open(struct atm_vcc *vcc)
{
	uint32 i, vc = ATM_VC_MAX, vci, vpi;
	uint32 temp;

	set_bit(ATM_VF_ADDR, &vcc->flags);	/* claim address	*/
	vcc->itf = vcc->dev->number;		/* interface number */
	vpi = vcc->vpi;
	vci = vcc->vci;

	printk("mt7510_atm_open\n");
	printk("vpi: %d\n", vcc->vpi);
	printk("vci: %d\n", vcc->vci);

	/* ----- Check the number of total opened vc ----- */
	if ( atmCfgTable.vcNumber >= ATM_VC_MAX ) {
		return -EBUSY;
	}

	if(gLinkPOE == 0){ /* PPPoA style should reset these two member */
	    gLinkMode = 0;
	    gLinkType = 0;
	}
	else if(gLinkPOE == 1){
	    gLinkPOE = 0; /* after used should reset it */
	}

	for ( i = 0; i < ATM_VC_MAX; i++ ) {
		if (atmCfgTable.openFlag[i]==0){
			atmCfgTable.openFlag[i] = 1;
			vc = i;
			atmCfgTable.vpi[vc] = vpi;
			atmCfgTable.vci[vc] = vci;
			atmCfgTable.vcc[vc] = vcc;
			atmCfgTable.qos[vc].mode = gLinkMode;
			atmCfgTable.qos[vc].encapType = gLinkType;
			atmCfgTable.vcNumber++;
			break;
		}
	}

	if (vc == ATM_VC_MAX)
		return -EBUSY;

	printk("mt7510_atm_open vc %ld:\n", vc);

	atmCfgTable.qos[vc].pcr = 0;
	atmCfgTable.qos[vc].scr = 0;
	atmCfgTable.qos[vc].mbs = 0;
	atmCfgTable.qos[vc].type = UBR;

	switch (vcc->qos.txtp.traffic_class) {
	case ATM_CBR:
		atmCfgTable.qos[vc].type = CBR;
		atmCfgTable.qos[vc].pcr = vcc->qos.txtp.pcr;
		break;
  	case ATM_UBR:
		atmCfgTable.qos[vc].type = UBR;
		atmCfgTable.qos[vc].pcr = vcc->qos.txtp.pcr;
		break;
  	case ATM_VBR:
		atmCfgTable.qos[vc].type = VBR;
		atmCfgTable.qos[vc].pcr = vcc->qos.txtp.pcr;
		atmCfgTable.qos[vc].scr = vcc->qos.txtp.scr;
		atmCfgTable.qos[vc].mbs = vcc->qos.txtp.mbs;
		break;
	case ATM_nrtVBR:
		atmCfgTable.qos[vc].type = nrtVBR;
		atmCfgTable.qos[vc].pcr = vcc->qos.txtp.pcr;
		atmCfgTable.qos[vc].scr = vcc->qos.txtp.scr;
		atmCfgTable.qos[vc].mbs = vcc->qos.txtp.mbs;
		break;
	default:
		/* default is UBR */
		break;
	}

	setAtmQosRed(&atmCfgTable.qos[vc],vc);

	// read to clear Receive Counter
	temp = TSARM_TDCNT(vc);
	temp = TSARM_RDCNT(vc);

	// set up VC Configuration as valid =1, raw =0(AAL5 mode), port =0, VPI, VCI

	TSARM_VCCR(vc) =
		VCCFGR_VPI(vpi) | VCCFGR_VCI(vci) |
		VCCFGR_PORT(VCCFGR_ATM_PHY0) | VCCFGR_VALID;

	TSARM_VCCR(vc) |= (1<<28);

	vcc->sk.sk_sndbuf = 0x7fffffff;
 	vcc->sk.sk_rcvbuf = 0x7fffffff;

	set_bit(ATM_VF_READY, &vcc->flags);

	return 0;
}


// need free allocated memory in atm_open function
static void mt7510_atm_close(struct atm_vcc *vcc)
{
	clear_bit(ATM_VF_ADDR, &vcc->flags);
	atmAal5VcClose(vcc->vpi, vcc->vci);
	clear_bit(ATM_VF_READY, &vcc->flags);

	printk("tc3162_atm_close done\n");
}

static int mt7510_atm_ioctl(struct atm_dev *dev, unsigned int cmd, void *arg)
{
#ifdef CONFIG_VLAN_ATM
	tsarm_ioctl_t sar_ioctl;
	switch(cmd) {
		case SET_ATM_VLAN:
			if ( copy_from_user(&sar_ioctl, (tsarm_ioctl_t*)arg, sizeof(tsarm_ioctl_t)) ) {
				return -EFAULT;
			}
			if ( setATMVlan(&sar_ioctl) == -1 ) {
				return -EFAULT;
			}
			break;
		case DEL_ATM_PVC:
		default:
			break;
	}
#endif
	return 0;
}

#define CONFIG_8021P_REMARK 1
#ifdef CONFIG_8021P_REMARK
static inline struct sk_buff* vlanPriRemark(struct sk_buff *skb)
{
	uint8 encap_mode = 0, encap_len = 0;
	char * vlan_p = NULL, *ether_type_ptr = NULL;
	unsigned char ucprio = 0;
	unsigned char uc802prio = 0;
	uint16 vid=0;
	int copy_len = 0;

	if ( skb->mark & QOS_8021p_MARK ) {
		/*vlan tagging*/
		encap_mode = getEncapMode((uint8*)skb->data);
		/*Note Ethernet Header*/
		if ( (encap_mode == RFC1483_B_VC)
			|| (encap_mode == PPPoA_LLC)
			|| (encap_mode == PPPoA_VC) ) {
				/*Nono ethernet header to do nothings*/
				return skb;	
			}
		
		encap_len = atmEncapLen[encap_mode];
		ether_type_ptr = skb->data + encap_len + 12;
		ucprio = (skb->mark & QOS_8021p_MARK) >> 8;
		//printk("\r\n[vlanPriRemark]skb->mark=%x;ucprio=%d",skb->mark,ucprio);
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
				
				if ( ATM_SKB(sk_tmp)->vcc->pop ) {
					ATM_SKB(sk_tmp)->vcc->pop(ATM_SKB(sk_tmp)->vcc, sk_tmp);
				}
				else {
					dev_kfree_skb_any(sk_tmp);
				}
				
				if ( !skb ) {
					printk(KERN_ERR, "Vlan:failed to realloc headroom\n");
					return NULL;
				}
			}
			else {
				skb = skb_unshare(skb, GFP_ATOMIC);
				if ( !skb ) {
			//		printk(KERN_ERR, "Vlan: failed to unshare skbuff\n");
					return NULL;
				}
			}
		
			/*offset 4 bytes*/
			skb_push(skb, VLAN_HLEN);
		
			copy_len = encap_len + 2*VLAN_ETH_ALEN;
			/*move the mac address to the beginning of new header*/
			memmove(skb->data, skb->data+VLAN_HLEN, copy_len);
		}
	
		vlan_p = skb->data + encap_len + 12;
		*(unsigned short *)vlan_p = 0x8100;
		
		vlan_p += 2;
		*(unsigned short *)vlan_p = 0;
		/*3 bits priority and vid vlaue*/
		*(unsigned short*)vlan_p |= (((uc802prio & 0x7) << 13)|vid) ;
		skb->network_header -= VLAN_HLEN;
		skb->mac_header -= VLAN_HLEN;
		//skb_dump(skb);
	}
	return skb;
}
#endif /*CONFIG_8021P_REMARK*/

static int mt7510_atm_send(struct atm_vcc *vcc, struct sk_buff *skb)
{
	int ret;
	uint32 flags;

	if (skb == NULL)
	{
		printk("\nERROR: skb is NULL at mt7510_atm_send\n");
		return 1;
	}

	if (recycle_tx){
		printk("\nNow is Recycle TX Skb, Doesn't send packet\n");
		return 1;
	}

#ifdef CONFIG_VLAN_ATM
	int vlan_vc;
	vlan_vc = getIndexVlanVcc( vcc->vpi, vcc->vci );
	if ( vlan_vc != ATM_DUMMY_VC ) {
		if ( vlan_vcc[vlan_vc].active == 1 ) {
			//printk("insert vlan tag here.\n");
			skb = insert_vtag( skb, vlan_vc );
			if ( skb == NULL ) {
				printk("just return for skb is NULL.\n");
				return 1;
			}
		}
	}
#ifdef CONFIG_8021P_REMARK
	else{
		skb=vlanPriRemark(skb);
		if(skb==NULL){
			printk("802.1p remark failure\r\n");
			return 1;
		}
	}
#endif	
#endif

#if defined(WAN2LAN)
	if (masko)
	{
		struct sk_buff *skb2 = NULL;
		uint32 port_id = 0;

		//Check the skb headroom is enough or not. shnwind 20100121.
		if (skb_headroom(skb) < TX_STAG_LEN)
			skb2 = skb_copy_expand(skb, TX_STAG_LEN, skb_tailroom(skb) , GFP_ATOMIC);
		else
			skb2 = skb_copy(skb, GFP_ATOMIC);

		if (skb2 == NULL)
			printk("\nFAIL(%s): wan2lan allocation\n", __FUNCTION__);
		else
		{
			skb2->mark |= SKBUF_COPYTOLAN;
			port_id = (masko>>24) - 1;
			macSend(port_id, skb2); //tc3162_mac_tx
		}
	}
#endif

	spin_lock_irqsave(&sarLock, flags);
	ATM_SKB(skb)->vcc = vcc;
	ret = atmAal5DataReq(skb, vcc->vpi, vcc->vci);
	if (!ret)
		atomic_inc(&vcc->stats->tx);

	spin_unlock_irqrestore(&sarLock, flags);
	return ret;
}

static int mt7510_atm_change_qos (struct atm_vcc *vcc, struct atm_qos *qos,int flags)
{
	printk("tc3162_atm_change_qos\n");
	return 0;
}


int proc_vc[ATM_VC_MAX];
char proc_vc_name[ATM_VC_MAX][32];

static int tsarm_pktsclear_read_proc(char *page, char **start, off_t off,
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

static int tsarm_pktsclear_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data){
	char val_string[8];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

//	atmReset();
	atmPktsClear();
	return count;
}

#if defined (QOS_REMARKING) || defined (TCSUPPORT_HW_QOS)
static int tsarm_qoswrr_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data){

	int len = sprintf(page, "%d %d %d %d %d\r\n",*qos_wrr_info, *(qos_wrr_info + 1), *(qos_wrr_info + 2), *(qos_wrr_info + 3), *(qos_wrr_info + 4));
	return len;
}
static int tsarm_qoswrr_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data){
	int len;
	char get_buf[32];
	uint32 priority;
	int max_wrr_val = 0, i, j;
	QDMA_TxQosScheduler_T txQos;
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
			max_prio = i;
		}
	}

	printk("qos_wrr_info: %d %d %d %d %d\n", qos_wrr_info[0], qos_wrr_info[1], qos_wrr_info[2], qos_wrr_info[3], qos_wrr_info[4]);

	qos_wrr_user = QOS_DMAWRR_USERDEFINE;
	if(*qos_wrr_info == 0){  /*strict priority*/
	//	TSARM_GFR &= ~(GFR_DMT_WRR_EN);

		printk("QDMA QoS: HW SP\n");

	#if 1
		for(i=0 ; i<ATM_VC_MAX ; i++){
			txQos.channel = i ;
			txQos.qosType = 1 ;	// Strict Priority
			/* QDMA has 8 queue */
			for(j=0 ; j<8 ; j++){
				txQos.queue[j].weight = 0;
			}
			qdma_set_tx_qos(&txQos) ;
		}

		for(i=0 ; i<ATM_VC_MAX ; i++){
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
	#endif
	} else{  /*WRR*/

		printk("QDMA QoS: HW WRR\n");

		for(i=0 ; i<ATM_VC_MAX ; i++){
			txQos.channel = i ;
			txQos.qosType = 0 ;
			/* QDMA has 8 queue */
			for(j=0 ; j<4 ; j++){
				txQos.queue[j].weight = qos_wrr_info[4-j];
			}

			for(j=4 ; j<8 ; j++){
				txQos.queue[j].weight = 0;
			}

			qdma_set_tx_qos(&txQos) ;
		}

		for(i=0 ; i<ATM_VC_MAX ; i++){
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
	return len;
}
#endif


#ifdef TCSUPPORT_QOS
static int tsarm_tcqos_read_proc(char *page, char **start, off_t off,
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

static int tsarm_tcqos_write_proc(struct file *file, const char *buffer,
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


#if 0
static int tsarm_proc_reg(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int len;
	int *vc;

	vc = (int *) data;
	len = atmRegDump(buf, *vc);
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
#endif

static int tsarm_stats_read_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int len = getATMState(buf);
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

static int tsarm_stats_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	memset((char *)&(atm_p->MIB_II), 0, sizeof(atmMIB_II_t));

	return count;
}

static uint32 savedInF5Pkts = 0;
static uint32 savedInF4Pkts = 0;
#ifdef CMD_API
static uint32 oam_success_cnt, oam_fail_cnt;
static uint32 res_time_sum = 0, res_time_min = 0xFFFFFFFF, res_time_max = 0;
static uint32 rep_num, f5;
volatile uint8 recved;
#endif

static int oam_ping_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;
	int result;

#ifdef CMD_API
	if(f5){
		oam_success_cnt = atm_p->MIB_II.inF5Pkts - savedInF5Pkts;
		oam_fail_cnt = rep_num - atm_p->MIB_II.inF5Pkts + savedInF5Pkts;
	}
	else{
		oam_success_cnt = atm_p->MIB_II.inF4Pkts - savedInF4Pkts;
		oam_fail_cnt = rep_num - atm_p->MIB_II.inF4Pkts + savedInF4Pkts;
	}
	savedInF5Pkts = atm_p->MIB_II.inF5Pkts;
	savedInF4Pkts = atm_p->MIB_II.inF4Pkts;
	if(oam_success_cnt)
		do_div(res_time_sum, oam_success_cnt);
	else
		res_time_sum = 0;
	len = sprintf(page, "OAM- Test Results\r\nSuccessCount:%ld FailureCount:%ld AverageResponseTime:%lu MinimumResponseTime:%lu MaximumResponseTime:%lu\r\n", oam_success_cnt, oam_fail_cnt, res_time_sum, res_time_min, res_time_max);
	res_time_sum = 0;
	res_time_min = 0xFFFFFFFF;
	res_time_max = 0;
	rep_num = 0;
#else
	result = (savedInF5Pkts != atm_p->MIB_II.inF5Pkts) || (savedInF4Pkts != atm_p->MIB_II.inF4Pkts);
	if (result) {
		savedInF5Pkts = atm_p->MIB_II.inF5Pkts;
		savedInF4Pkts = atm_p->MIB_II.inF4Pkts;
	}

	len = sprintf(page, "%d\n", result);

#endif
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

static int oam_ping_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];
	uint32 vpi;
	uint32 vci;
#ifndef CMD_API
	uint32 f5;
#endif
	uint32 endToEnd;
	uint32 funcType;
#ifdef CMD_API
	uint32 ms, timeout;
	uint32 i;
	volatile uint32 timer_now, timer_last;
	volatile uint32 tick_acc;
	uint32 one_tick_unit = 1 * SYS_HCLK * 1000 / 2;
	volatile uint32 timer1_ldv = VPint(CR_TIMER1_LDV);
	volatile uint32 tick_wait;
#endif

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

#ifdef CMD_API
	if (sscanf(val_string, "%lu %lu %lu %lu %lu %lu", &vpi, &vci, &f5, &endToEnd, &funcType, &timeout) != 6) {
	    printk("usage: <vpi> <vci> <0(f4) 1(f5)> <0(segment) 1(end-to-end)> <type:0(AIS) 1(RDI) 2(Loopback)> <timeout>\n");
	    return count;
	}
	rep_num++;
	tick_acc = 0;
	recved = 0;
	timer_last = VPint(CR_TIMER1_VLR);
#else
	if (sscanf(val_string, "%lu %lu %lu %lu %lu", &vpi, &vci, &f5, &endToEnd, &funcType) != 5) {
		printk("usage: <vpi> <vci> <f5> <end-to-end> <type:0(AIS) 1(RDI) 2(Loopback)>\n");
		return count;
	}

	savedInF5Pkts = atm_p->MIB_II.inF5Pkts;
	savedInF4Pkts = atm_p->MIB_II.inF4Pkts;

#endif

	printk("oam_ping\n");
	printk("%lu %lu %lu %lu %lu %lu\n", vpi, vci, f5, endToEnd, funcType, timeout);

	atmOamF4F5DataReq((uint8 )vpi, (uint16 )vci, f5, endToEnd, funcType);
#ifdef CMD_API
	tick_wait=timeout*one_tick_unit;
	while (!recved){
	//	msleep(10);
		timer_now = VPint(CR_TIMER1_VLR);
		if (timer_last >= timer_now)
			tick_acc += timer_last - timer_now;
		else
	    	tick_acc += timer1_ldv - timer_now + timer_last;
		timer_last = timer_now;
		ms = tick_acc;
	//	do_div(ms, one_tick_unit);
		if((ms >= tick_wait)||recved)
			break;
	}
	ms = tick_acc;
   	do_div(ms, one_tick_unit);
   	res_time_sum += ms;
	if(res_time_min > ms)
   		res_time_min = ms;
	if(res_time_max < ms)
		res_time_max = ms;
#endif

	return count;
}

//add by brian for atm f5 loopback diagnostic
#ifdef CWMP
static int loopback_diagnostic_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int index = 0;
	char *loopbackbuf = NULL;
	uint32 diagnostictime;
	char result[][32] = {"Complete","Error_internal","InProgress","Error_other"};
	enum result_index
	{
		Complete = 0,
		Internal,
		InProgress,
		Other
	};

	loopbackbuf = page;
	if(internal_error)
	{
		index += sprintf(loopbackbuf+index, "DiagnosticState=%s\n", result[Internal]);
		index += sprintf(loopbackbuf+index, "DiagnosticTime=0\n");
		internal_error = 0;
	}
	else
	{
		if((cwmpSavedInF5Pkts != 0) && (cwmpSavedOutF5Pkts <= cwmpSavedInF5Pkts))//diagnostic ok
		{
			if(g_f5loopback_rxtime > g_f5loopback_txtime)
				diagnostictime = (g_f5loopback_rxtime - g_f5loopback_txtime)*1000 / HZ;
			else
				diagnostictime = (maxtimeindex + g_f5loopback_rxtime - g_f5loopback_txtime)*1000 / HZ;

			index += sprintf(loopbackbuf+index, "DiagnosticState=%s\n", result[Complete]);
			index += sprintf(loopbackbuf+index, "DiagnosticTime=%lu\n", diagnostictime);
		}
		else
		{
			index += sprintf(loopbackbuf+index, "DiagnosticState=%s\n", result[InProgress]);
			index += sprintf(loopbackbuf+index, "DiagnosticTime=0\n");
		}
	}

	index -= off;
	*start = page + off;

	if (index > count)
		index = count;
	else
		*eof = 1;

	if (index < 0)
		index = 0;

	return index;

}

static int loopback_diagnostic_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	uint32 f5;
	uint32 endToEnd;
	uint32 funcType;
	uint32 flag;
	uint32 vpi = 0;
	uint32 vci = 0;
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';
	if (sscanf(val_string, "%lu %lu %lu %lu %lu %lu", &vpi, &vci, &f5, &endToEnd, &funcType,&flag) != 6) {
		printk("usage: <wanindex> <f5> <end-to-end> <type:0(AIS) 1(RDI) 2(Loopback)> <flag>\n");
		return count;
	}

	printk("\r\nthe value is %lu,%lu,%lu,%lu,%lu,%lu",vpi,vci,f5,endToEnd,funcType,flag);
	//the glag used for cwmp diagnostic
	cwmpflag = flag;

	//compute the time
	g_f5loopback_rxtime = 0;
	g_f5loopback_txtime = jiffies;

	//reset the pkt number
	cwmpSavedInF5Pkts = 0;
	cwmpSavedOutF5Pkts = 0;

	//do loopback diagnostic
	if(atmOamF4F5DataReq((uint8 )vpi, (uint16 )vci, f5, endToEnd, funcType))
	{
		//internal error,so wu just set the variable
		internal_error = 1;
	}

	return count;
}

static int loopback_diagnostic_reset_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	return 0;
}

static int loopback_diagnostic_reset_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	int resetflag = 0;
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';
	if (sscanf(val_string, "%d", &resetflag) != 1) {
		printk("usage: <flag>\n");
		return count;
	}

	if(resetflag)
	{
		cwmpSavedInF5Pkts = 0;
		cwmpSavedOutF5Pkts = 0;
		g_f5loopback_rxtime = 0;
		g_f5loopback_txtime = 0;
		cwmpflag = 0;//reset the cwmpflag,so no need to compute the in or out pkt number
		internal_error = 0;
		resetflag = 0;
	}

	return count;
}
#endif


static int (*eth_mac_addr)(struct net_device *, void *);

static int br2684_mac_addr(struct net_device *dev, void *p)
{
	struct br2684_dev *br2684Dev;

	int err = eth_mac_addr(dev, p);

	br2684Dev = (struct br2684_dev *)netdev_priv(dev);

	if (!err)
		br2684Dev->mac_was_set = 1;

	return err;

}

void mt7510_config(int linkMode, int linkType)
{
	printk("mt7510_config\n");
	gLinkType = linkType;
	gLinkMode = linkMode;
	gLinkPOE = 1;
}


int mt7510_init_MPOA(struct atm_vcc *atmvcc, int encaps)
{
	int err, vc, i;

	printk("mt7510_init_MPOA\n");

	vc = -1;
	err = 0;
	for (i=0; i<ATM_VC_MAX; i++){
		if (atmCfgTable.openFlag[i] == 1){
			if ((atmCfgTable.vpi[i]==atmvcc->vpi) && (atmCfgTable.vci[i]==atmvcc->vci)){
				vc = i;
				break;
			}
		}
	}

	if (vc != -1){ 
		if (encaps == BR2684_ENCAPS_VC){
			atmCfgTable.qos[vc].muxType = MUX_VC;
			qosRecord[vc].muxType = MUX_VC;
			initMPOA(vc, &atmCfgTable.qos[vc]);
		} else if (encaps == BR2684_ENCAPS_LLC){
			atmCfgTable.qos[vc].muxType = MUX_LLC;
			qosRecord[vc].muxType = MUX_LLC;
			initMPOA(vc, &atmCfgTable.qos[vc]);
		} else {
			printk("br2684 encapsulation error\n");
			err = -EINVAL;
		}
	} else {
		printk("br2684_regvcc: cannot get free vc channel\n");
		err = -EINVAL;
	}
	return err;
}


/*
 * Return Value:
 *		Error: 	-1
 *      Drop:  	-2
 *      Exit:  	-3
 *      Success: 0
 */
 
// add by camel for fix 'implicit declaretion' error
extern 	__IMEM __be16 eth_type_trans(struct sk_buff *skb, struct net_device *dev);

int mt7510_br2684_push(struct atm_vcc *atmvcc, struct sk_buff *skb)
{
	struct br2684_vcc *brvcc = (struct br2684_vcc *)(atmvcc->user_back);
	struct net_device *net_dev = brvcc->device;
	struct br2684_dev *brdev = (struct br2684_dev *)netdev_priv(net_dev);

	if (brdev->payload == p_bridged){
		skb->protocol = eth_type_trans(skb, net_dev);
	} else if (brdev->payload == p_routed){
		if (memcmp(skb->data, ethertype_ipv6, sizeof(ethertype_ipv6)) == 0){
			skb->protocol = htons(ETH_P_IPV6);
			skb_pull(skb, sizeof(ethertype_ipv6));
		} else if (memcmp(skb->data, ethertype_ipv4, sizeof(ethertype_ipv4)) == 0){
			skb->protocol = htons(ETH_P_IP);
			skb_pull(skb, sizeof(ethertype_ipv4));
		} else {
			printk("mt7510_br2684_push: payload data error\n");
			return -1;
		}
		skb_reset_network_header(skb);
		skb->pkt_type = PACKET_HOST;
	}
	return 0;
}


/*
 * Return Value:
 *		Error: 	-1
 *      Drop:  	-2
 *      Exit:  	-3
 *      Success: 0
 */
int mt7510_br2684_xmit(struct sk_buff *skb, struct net_device *dev, struct br2684_vcc *brvcc)
{
	struct br2684_dev *brdev =(struct br2684_dev *)netdev_priv(dev);

	if (brvcc->encaps == e_br2684_llc){
		if (brdev->payload == p_routed){
			unsigned short prot = ntohs(skb->protocol);

			skb_push(skb, sizeof(ethertype_ipv4));
			switch (prot){
				case ETH_P_IP:
					skb_copy_to_linear_data(skb, ethertype_ipv4, sizeof(ethertype_ipv4));
					break;
				case ETH_P_IPV6:
					skb_copy_to_linear_data(skb, ethertype_ipv6, sizeof(ethertype_ipv6));
					break;
				default:
					dev_kfree_skb(skb);
					return 1;
			}
		}
	} 
	return 0;
}


/*
 * Return Value:
 *		Error: 	-1
 *      Exit:  	-2
 *      Success: 0
 */
/* Called when an AAL5 PDU comes in */
int mt7510_pppoatm_push(struct atm_vcc *atmvcc, struct sk_buff *skb)
{
	if (memcmp(skb->data, ethertype_ipv6, sizeof(ethertype_ipv6)) == 0){
		memcpy(skb->data, ppp_pid_ipv6, sizeof(ppp_pid_ipv6));
	} else if (memcmp(skb->data, ethertype_ipv4, sizeof(ethertype_ipv4)) == 0){
		memcpy(skb->data, ppp_pid_ipv4, sizeof(ppp_pid_ipv4));
	}
	return 0;
}


int atm_init_wrappter()
{
	printk("atm_init_wrappter\n");
	br2684_config_hook = mt7510_config;
	br2684_init_hook = mt7510_init_MPOA;
	br2684_push_hook = mt7510_br2684_push;
	br2684_xmit_hook = mt7510_br2684_xmit;

	printk("br2684_init\n");
	br2684_init();

	pppoatm_config_hook = mt7510_config;
	pppoatm_init_hook = mt7510_init_MPOA;
	pppoatm_push_hook = mt7510_pppoatm_push;

	printk("pppoatm_init\n");
	pppoatm_init();

#if defined(ATM_NAPI_MODE)
	napi_en = 1;
#endif

	return 0;
}


int atm_exit_wrappter()
{
	printk("atm_exit_wrappter\n");
	br2684_config_hook = NULL;
	br2684_init_hook = NULL;
	br2684_push_hook = NULL;
	br2684_xmit_hook = NULL;

	printk("br2684_exit\n");
	br2684_exit();

	pppoatm_config_hook = NULL;
	pppoatm_init_hook = NULL;
	pppoatm_push_hook = NULL;

	printk("pppoatm_exit\n");
	pppoatm_exit();

#if defined(ATM_NAPI_MODE)
	napi_en = 0;
#endif

	return 0;
}


#if defined(ATM_NAPI_MODE)
/* Setting customized mac address */
static int mt7510_netdev_set_macAddr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	printk(" mt7510_netdev_set_macAddr\n");

	/* Save the customize mac address */
  	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

	return 0;
}

#ifdef TR068_LED
static void mt7510_sar_timer(unsigned long data)
{
	unsigned long rx_pkts,tx_pkts;		
	struct psepkt_stats pf_stats;
	unsigned long rx_pkts_diff ,tx_pkts_diff;	

#ifdef TCSUPPORT_RA_HWNAT
	if (internet_hwnat_timer_switch && ra_sw_nat_hook_pse_stats) {
		rx_pkts = sarStats.rx_pkts;		
		tx_pkts = sarStats.tx_pkts;		
		
		sarStats.rx_pkts = atm_p->MIB_II.inPkts;
		sarStats.tx_pkts = atm_p->MIB_II.outPkts;

		ra_sw_nat_hook_pse_stats(&pf_stats, 2);

		sarStats.rx_pkts += pf_stats.rx_pkts;
		sarStats.tx_pkts += pf_stats.tx_pkts;

		rx_pkts_diff = sarStats.rx_pkts- rx_pkts;
		tx_pkts_diff = sarStats.tx_pkts- tx_pkts;
#if 0
		if ((rx_pkts_diff > internet_hwnat_pktnum) ||
			(tx_pkts_diff > internet_hwnat_pktnum)) {
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
	mt7510sar_timer.expires = jiffies + msecs_to_jiffies(250);
  	add_timer(&mt7510sar_timer);
}
#endif

static int mt7510_netdev_open(struct net_device *dev)
{
	printk("mt7510_netdev_open\n");

	printk("%s: starting interface.\n", dev->name);
#ifdef TR068_LED
	/* Schedule timer */
  	init_timer(&mt7510sar_timer);
	mt7510sar_timer.expires = jiffies + msecs_to_jiffies(250);
  	mt7510sar_timer.function = mt7510_sar_timer;
  	mt7510sar_timer.data = (unsigned long)dev;
  	add_timer(&mt7510sar_timer);
#endif
#if KERNEL_2_6_36
	// frank
	napi_enable(&mt7510Napi);
#endif

	netif_start_queue(dev);

  	return 0;
}


static int mt7510_netdev_close(struct net_device *dev)
{
	printk("mt7510_netdev_close\n");

	printk("%s: stoping interface.\n", dev->name);

#ifdef TR068_LED
	/* Kill timer */
  	del_timer_sync(&mt7510sar_timer);
#endif
	netif_stop_queue(dev);

#if KERNEL_2_6_36
	// frank
	napi_disable(&mt7510Napi);
#endif

	return 0;
}


static int mt7510_netdev_set_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	printk("\n%s: Not implemented yet\n", __FUNCTION__);
  	return 0;
}


static int mt7510_netdev_start(struct net_device *dev)
{
	uint8 *flashMacAddr = (uint8 *) (0xbc00ff48);

	printk(" mt7510_netdev_start \n");
#if 0
	if (
		flashMacAddr[0] == 0 && flashMacAddr[1] == 0 && flashMacAddr[2] == 0 &&
		flashMacAddr[3] == 0 && flashMacAddr[4] == 0 && flashMacAddr[5] == 0
	)
		printk(KERN_INFO "\nThe MAC address in flash is null!\n");	    
	else    
		memcpy(defMacAddr, flashMacAddr, 6);  	
#endif
	memcpy(dev->dev_addr, defMacAddr, 6);
	dev->addr_len = 6;

	printk(KERN_INFO
	       "%s: MT7510 ATM Ethernet address: %02X:%02X:%02X:%02X:%02X:%02X\n",
	       dev->name, 
	       dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
		   dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
	return 0;
}


#ifdef KERNEL_2_6_36
static const struct net_device_ops atm_netdev_ops = {

	.ndo_init               = mt7510_netdev_start,
	.ndo_open               = mt7510_netdev_open,
	.ndo_stop               = mt7510_netdev_close,
	.ndo_set_mac_address    = mt7510_netdev_set_macAddr,
	.ndo_do_ioctl           = mt7510_netdev_set_ioctl,
	.ndo_start_xmit         = br2684_start_xmit,
	.ndo_change_mtu         = eth_change_mtu,
	.ndo_validate_addr      = eth_validate_addr,
};
#endif
#endif

static int __init mt7510_atm_init(void)
{
	struct proc_dir_entry *tsarm_proc;
	uint32 offset;
	int idx, err=0, reg, i, j, ret=0;
	atmRxMsgBuf_t *pMsg;
	struct sk_buff *skb;
	QDMA_TxQosScheduler_T txQos;

	printk("TSARM: MT7510 ATM SAR driver 1.0 init\n");

	spin_lock_init(&sarLock);
	spin_lock_init(&msgLock);
	qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);

	if ((mt7510_atm_dev = atm_dev_register("TSARM", &mt7510_atm_ops, -1, NULL)) == NULL) {
		printk("atm dev register fail.\n");
  	    err = -ENODEV;
		goto failed;
	}

	mt7510_atm_dev->ci_range.vpi_bits = ATM_CI_MAX;        /* atm VPI supports 8 bits */
	mt7510_atm_dev->ci_range.vci_bits = ATM_CI_MAX;        /* atm VCI supports 16 bits */

	/* init tsarm hardware module */
	printk("atmInit\n");
	atmInit();
	printk("==============================================\n");
	
	printk("atm_init_wrappter\n");
	atm_init_wrappter();
	printk("==============================================\n");

#if 1
	/* Initial device private data */
	gpMsg = (atmMsg_t *)kmalloc(sizeof(atmMsg_t), GFP_KERNEL);
	if (gpMsg == NULL){
		printk("mt7510_atm_init: alloc momory fail\n");
		err = (-ENOMEM);
		goto failed;
	}

	gpMsg->txHeadPtr = NULL;
	gpMsg->txTailPtr = NULL;

	memset(&initCfg, 0, sizeof(QDMA_InitCfg_t));

#ifdef CONFIG_TX_POLLING_BY_MAC
	initCfg.txRecycleMode = QDMA_TX_POLLING;	//check if polling
	initCfg.txRecycleThreshold = 4;
#else
	initCfg.txRecycleMode = QDMA_TX_INTERRUPT;	//check if polling
	initCfg.txIrqThreshold = 4;
#endif

	initCfg.rxRecvMode = QDMA_RX_INTERRUPT;
	initCfg.cbXmitFinish = mt7510_atm_xmit_callback;
	initCfg.cbRecvPkts = mt7510_atm_recv_callback;

#if defined(ATM_NAPI_MODE)
	printk("ATM SAR RX NAPI Mode\n");

	initCfg.rxRecvMode = QDMA_RX_NAPI;
	initCfg.cbEventHandler = mt7510_atm_qdmaEventHandler;

	mt7510AtmDev = alloc_netdev(sizeof(struct br2684_dev),"atm_dev", ether_setup);

	if (!mt7510AtmDev){
		printk("mt7510_atm_init: atm dev alloc memory fail\n");
		kfree(gpMsg);
		return -ENOMEM;
	}

#if KERNEL_2_6_36
/* Hook up with handlers */
	mt7510AtmDev->netdev_ops = &atm_netdev_ops;
	mt7510Napi.weight = (RX_QUEUE_LEN >> 1);
	netif_napi_add(mt7510AtmDev, &mt7510Napi, br2684_napiPoll, (RX_QUEUE_LEN>>1));
#else
	mt7510AtmDev->poll = mt7510_atm_napiPoll;
	mt7510AtmDev->weight = RX_QUEUE_LEN >> 1;
	mt7510AtmDev->init = mt7510_netdev_start;
#endif

	err = register_netdev(mt7510AtmDev);
	if (err < 0) {
		printk(" mt7510_atm_init: register_netdev fail\n");
		kfree(gpMsg);
		free_netdev(mt7510AtmDev);
		goto failed;
	} else {
		printk(" mt7510_atm_init: register_netdev success\n");
	}

	printk("mt7510_atm_init: open genenral device\n");
	
	netif_start_queue(mt7510AtmDev);

	set_bit(__LINK_STATE_START, &mt7510AtmDev->state);

#ifndef KERNEL_2_6_36
	if (mt7510AtmDev->open){
		ret = mt7510AtmDev->open(mt7510AtmDev);
		if (ret)
			clear_bit(__LINK_STATE_START, &mt7510AtmDev->state);
	}

	if (!ret)
		mt7510AtmDev->flags |= IFF_UP;
#else
	if (mt7510AtmDev->netdev_ops->ndo_open){
		ret = mt7510AtmDev->netdev_ops->ndo_open(mt7510AtmDev);
		if (ret)
			clear_bit(__LINK_STATE_START, &mt7510AtmDev->state);
	}
#endif

	printk("general device status: %x\n", mt7510AtmDev->state);
#else
	printk("ATM SAR RX INT Mode\n");
	initCfg.rxRecvMode = QDMA_RX_INTERRUPT;
#endif

	// disable GDM RX CRC Stripping
	// SAR is interface without FCS
	fe_reg_modify_bits(0x1500, 0x0, 16, 1);
	fe_reg_modify_bits(0x1500, 0x1, 31, 1);

	qdma_init(&initCfg);
		
	err = atm_msg_buffer_init(((4+1)*16*(ATM_VC_MAX+1)), 512);
	if (err) {
		printk("SAR Driver: message buffer init fail!\n");	
		kfree(gpMsg);
#if defined(ATM_NAPI_MODE)
		unregister_netdev(mt7510AtmDev);
		free_netdev(mt7510AtmDev);
#endif
		goto failed;
	}

	memset((char*)gpMsg->rxMsgBaseAddr, 0, gpMsg->rxMsgNum * sizeof(atmRxMsgBuf_t));
	pMsg = (atmRxMsgBuf_t *)gpMsg->rxMsgBaseAddr;

	printk("txMsgNum: %d\n", gpMsg->txMsgNum);
	printk("rxMsgNum: %d\n", gpMsg->rxMsgNum);

	//Enable TXBUF_CTRL
	//SET TXBUF Channel Threshold-> 16
	fe_reg_modify_bits(0x1890, 0x800010c0, 0, 32);
	
	// Enable Green Drop
	fe_reg_modify_bits(0x18a0, 0x1, 29, 1);
	fe_reg_modify_bits(0x18a4, 0x88888888, 0, 32);

	// reserve 1 rx descp to hardware
	for (idx=0; idx<(gpMsg->rxMsgNum-1); idx++){
		skb = skbmgr_dev_alloc_skb2k();
		//frank : may need to add the free action
		if (skb == NULL){
			printk("mt7510_atm_open: memory allocate fail\n");

			kfree(gpMsg);
#if defined(ATM_NAPI_MODE)
			unregister_netdev(mt7510AtmDev);
			free_netdev(mt7510AtmDev);
#endif
			err = (-ENOMEM);
			goto failed;
		}
		// four byte alignment
		offset = (uint)(skb->tail) & 3;
		if (offset){
			skb_reserve(skb, (4 - offset));
		}

		if (qdma_has_free_rxdscp()){
			err = qdma_hook_receive_buffer(&pMsg[idx], sizeof(atmRxMsgBuf_t), skb);
		} else {
			printk("\nError: no available QDMA RX descritor\n");

			kfree(gpMsg);
#if defined(ATM_NAPI_MODE)
			unregister_netdev(mt7510AtmDev);
			free_netdev(mt7510AtmDev);
#endif
			err = (-ENOMEM);
			goto failed;
		}

		if (err){
			printk("qdma hook receive buffer fail!\n");

			kfree(gpMsg);
#if defined(ATM_NAPI_MODE)
			unregister_netdev(mt7510AtmDev);
			free_netdev(mt7510AtmDev);
#endif
			goto failed;
		}
	}
#endif

	/* atm related stats */
	tsarm_proc = create_proc_entry("tc3162/tsarm_stats", 0, NULL);
	if (tsarm_proc){
		tsarm_proc->read_proc = tsarm_stats_read_proc;
		tsarm_proc->write_proc = tsarm_stats_write_proc;
	} else {
		printk("proc_entry tc3162/tsarm_stats alloc fail\n");
	}

	tsarm_proc = create_proc_entry("tc3162/tsarm_pktsclear", 0, NULL);

	if (tsarm_proc){
		tsarm_proc->read_proc = tsarm_pktsclear_read_proc;
		tsarm_proc->write_proc = tsarm_pktsclear_write_proc;
	} else {
		printk("proc_entry tc3162/tsarm_pktsclear alloc fail\n");
	}

	#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
	tsarm_proc = create_proc_entry("tc3162/tsarm_qoswrr", 0, NULL);

	if (tsarm_proc){
		tsarm_proc->read_proc = tsarm_qoswrr_read_proc;
		tsarm_proc->write_proc = tsarm_qoswrr_write_proc;
	} else {
		printk("proc_entry tc3162/tsarm_qoswrr alloc fail\n");
	}
	#endif

	#ifdef TCSUPPORT_QOS
	tsarm_proc = create_proc_entry("tc3162/tcqos_disc", 0, NULL);

	if (tsarm_proc){
		tsarm_proc->read_proc = tsarm_tcqos_read_proc;
		tsarm_proc->write_proc = tsarm_tcqos_write_proc;
	} else {
		printk("proc_entry tc3162/tcqos_disc alloc fail\n");
	}
	#endif

#if 1
	/* atm oam loopback */
	tsarm_proc = create_proc_entry("tc3162/oam_ping", 0, NULL);
	tsarm_proc->read_proc = oam_ping_read_proc;
	tsarm_proc->write_proc = oam_ping_write_proc;
    //add by brian for atm f5 loopback diagnostic
	#ifdef CWMP
	tsarm_proc = create_proc_entry("tc3162/atm_f5_loopback_diagnostic", 0, NULL);
	tsarm_proc->read_proc = loopback_diagnostic_read_proc;
	tsarm_proc->write_proc = loopback_diagnostic_write_proc;

	tsarm_proc = create_proc_entry("tc3162/atm_f5_loopback_diagnostic_reset", 0, NULL);
	tsarm_proc->read_proc = loopback_diagnostic_reset_read_proc;
	tsarm_proc->write_proc = loopback_diagnostic_reset_write_proc;
	#endif
#endif
#if 0
	for (vc = 0; vc < ATM_VC_MAX; vc++) {
		proc_vc[vc] = vc;
		sprintf(proc_vc_name[vc], "tc3162/tsarm_vc%d", vc);
		create_proc_read_entry(proc_vc_name[vc], 0, NULL, tsarm_proc_reg, &proc_vc[vc]);
	}
#endif

	#ifdef SAR_VERIFY
	/*Register ci-cmd*/
	sarVerifyInit();
	#endif

#if 1
	for(i=0 ; i<ATM_VC_MAX ; i++){
		txQos.channel = i ;
		txQos.qosType = 1 ;	// Strict Priority
		/* QDMA has 8 queue */
		for(j=0 ; j<8 ; j++){
			txQos.queue[j].weight = 0;
		}
		qdma_set_tx_qos(&txQos) ;
	}

	for(i=0 ; i<ATM_VC_MAX ; i++){
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
#endif
#ifdef TCSUPPORT_RA_HWNAT
	qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_ENABLE); 
#else
	qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_DISABLE);
#endif
	return 0;

failed:
#if 1
	if (mt7510_atm_dev)
		atm_dev_deregister(mt7510_atm_dev);
	mt7510_atm_dev = NULL;
#endif
    return err;
}

static void __exit mt7510_atm_exit(void)
{
	int vc;

	qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_DISABLE);

	// disable tx/rx
	TSARM_GFR = 0x0;
	delay1ms(50); //delay1ms(50);	//check

	// GDM2 Channel disable
	fe_reg_modify_bits(0x152c, 0x0000, 16, 16);

#if defined(ATM_NAPI_MODE)
	printk("mt7510_atm_exit: ATM SAR RX NAPI Mode\n");

	if (mt7510AtmDev){
#ifndef KERNEL_2_6_36
			if (mt7510AtmDev->stop){
				mt7510AtmDev->stop(mt7510AtmDev);
			}
#else
			if (mt7510AtmDev->netdev_ops->ndo_stop){
				mt7510AtmDev->netdev_ops->ndo_stop(mt7510AtmDev);
			}
#endif
		clear_bit(__LINK_STATE_START, &mt7510AtmDev->state);

		netif_stop_queue(mt7510AtmDev);

		unregister_netdev(mt7510AtmDev);

		free_netdev(mt7510AtmDev);
	}
#endif

	printk("atm_exit_wrappter\n");
	atm_exit_wrappter();

	if (mt7510_atm_dev){
		atm_dev_deregister(mt7510_atm_dev);
		mt7510_atm_dev = NULL;
	}

	printk("qdma_set_channel_retire\n");
	for (vc=0; vc<ATM_VC_MAX; vc++){
		qdma_set_channel_retire(vc);
	}

	printk("qdma_txdscp_recycle_mode\n");
	qdma_txdscp_recycle_mode(QDMA_TX_INTERRUPT);

	recycle_rx = 1;
	printk("qdma_recycle_receive_buffer\n");
	qdma_recycle_receive_buffer();

	recycle_tx = 1;
	printk("qdma_recycle_transmit_buffer\n");
	qdma_recycle_transmit_buffer();

	printk("atm_msg_buffer_free\n");
	atm_msg_buffer_free(gpMsg->txMsgNum, gpMsg->rxMsgNum);

	printk("qdma_deinit\n");
	qdma_deinit();

	remove_proc_entry("tc3162/tsarm_stats", NULL);

	remove_proc_entry("tc3162/tsarm_pktsclear", NULL);

	#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
	remove_proc_entry("tc3162/tsarm_qoswrr", NULL);
	#endif

	#ifdef TCSUPPORT_QOS
	remove_proc_entry("tc3162/tcqos_disc", NULL);
	#endif

#if 1
	remove_proc_entry("tc3162/oam_ping", NULL);
	#ifdef CWMP
	remove_proc_entry("tc3162/atm_f5_loopback_diagnostic", NULL);
	remove_proc_entry("tc3162/atm_f5_loopback_diagnostic_reset", NULL);
	#endif
#endif
}

module_init (mt7510_atm_init);
module_exit (mt7510_atm_exit);
