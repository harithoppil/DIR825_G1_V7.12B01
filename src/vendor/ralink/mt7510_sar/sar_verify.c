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
#include <asm/tc3162/cmdparse.h>	
#include <linux/dma-mapping.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/atmbr2684.h>

#include "../bufmgr/qdma_api.h"
#include "tsarm.h"
#include "tsarm_verify.h"
/*Global variable*/
extern uint8 oamAisCell[];
extern uint8 oamRdiCell[];
extern uint8 oamLoopBackReqCell[];
extern uint8 oamContinuityCheckReqCell[];
extern uint8 oamF5AisCell[];
extern uint8 oamF5RdiCell[];
extern uint8 oamF5LoopBackReqCell[];
extern uint8 oamF5ContinuityCheckReqCell[];

extern uint32 treg_tslr;	//0x00010001		default value
extern atmConfig_t atmCfgTable;
extern uint32 treg_tstbr;

extern 
int atmTxQueSize[];

// frank used
extern int txcount1, txcount2;
extern int rxcount1, rxcount2;

extern uint16 volatile atmTxPriVcCnt[ATM_TX_PRIORITY_MAX][ATM_VC_MAX+1];
extern uint32 volatile atmTxVcTotalCnt;
extern uint16 volatile atmTxVcCnt[ATM_VC_MAX+1];


extern atmCtrl_t *atm_p;
/*end global variable*/
#ifdef CONFIG_MIPS_TC3262
static spinlock_t tsarm_lock;
#endif
//static uint8 sarVerifyDbg=0;
uint8 sarVerifyDbg=0;
static uint32 test_fixed_size=0;
static uint8 test_traffic=0;
static uint8 stop_loop=0;	
uint16 test_flag=0;
static uint8 ax4k_lpbk=0;
static uint8 wrrRule[ATM_TX_PRIORITY_MAX];
/*Used to set atm qos*/
#define MAX_TSW	4095
#define WRRBASE	720000
/*end  set atm qos*/
/*Used to VLAN Test*/
#define VLAN_LEN	4
#define VLAN_INDEX	12
static int vlan_id=0;
static uint8 vlanTag[VLAN_LEN]={0x81, 0x00, 0x00, 0x00};
/*end vlan test*/

static atmOamCell_t cc_cont;

/*End global variable*/
#define	MAX_TOS			255
#define	MAX_VPI			255
#define	MAX_VCI			65535
#define	MIN_VCI			1
#define	TEST_MIN_VCI	5

#define MIN_SKB_SIZE	64

void
cmpCell_tasklet(unsigned long data);
static struct tasklet_struct tsarm_tasklet;
static uint8 tasklet_flag=0;

spinlock_t dequeuLock = SPIN_LOCK_UNLOCKED;
spinlock_t txCCLock = SPIN_LOCK_UNLOCKED;
static struct sk_buff_head rx_queue;
static atomic_t sarRxMDone;
static atomic_t sarRxDDone;

#if 1
uint32
atmOamF4F5DataReqVerify(
	uint8 vpi,
	uint16 vci,
	uint8 f5,
	uint8 endToEnd,
	uint8 funcType
);
#endif
/*Used tc3162l2sar function*/
extern void 
mt7510AtmRateCalCulate(uint16 tb, uint32 cellRate,	uint32 *dec_p, uint32 *init_p);

extern void 
initMPOA(uint32 vc, qosProfile_t *qos_p);

extern int
atmRegDump(uint16 vc);

//extern void
//atmTxDescrFree(uint8 vc, uint8 free_any);

extern void atmInit(void);
extern void atmReset(void);

extern uint8 
atmDataReq(struct sk_buff *skb, uint8 vc, uint8 priority);

extern struct sk_buff  *
atmAal5DataInd(struct sk_buff *skb, uint8 vc, uint32 len, atmRxMsgBuf_t *pMsg);

extern uint8
atmOamDataReq(uint8 *data_p, uint8 pti, uint8 vc);

extern uint8
atmCcDataReq(uint8 *data_p);

extern uint32
mt7510AtmMbsCalCulate(uint16 pcr, uint16 scr, uint16 mbs);

extern void
delay1ms(int ms);

extern void
atmCounterDisplay(void);

extern uint32
atmOamF4F5DataReq(uint8 vpi,uint16 vci, uint8 f5, uint8 endToEnd, uint8 funcType);

extern uint8 
atmCcHandler(atmOamCell_t *oamCellp);

extern uint8 
atmOamHandler(atmOamCellPayload_t * oamCellPayloadp, uint8 pti, uint8 vc);

extern uint8
atmVcNumberGet(uint8 vpi,uint16 vci);

/*end Used tc3162l2sar function*/
extern int 
cmd_register(cmds_t *cmds_p);

extern int 
subcmd(const cmds_t tab[], int argc, char *argv[], void *p);

extern uint32 rand(void);

static int 
mt7510_atm_verify_send(struct atm_vcc *vcc, struct sk_buff *skb);

void 
verifySarInit(void);

struct sk_buff* 
patternGen(uint32 seq, uint8 priority);

void 
openAtmVc(uint8 vc,uint8 vpi, uint16 vci, uint8 phy);

void 
allCellTest(int delay, int loopTimes);

int  
doTsarm(int argc, char *argv[], void *p);

static int 
doTsarmReset(int argc, char *argv[], void *p);

static int 
doTsarmRegDump(int argc, char *argv[], void *p);

static int 
doTsarmRegCheck(int argc, char *argv[], void *p);

static int 
doTsarmLineRate(int argc, char *argv[], void *p);

static int
doTsarmRate(int argc, char *argv[], void *p);

static int 
doTsarmDMAWRR(int argc, char *argv[], void *p);

static int 
doTsarmDMAWRRSwitch(int argc, char *argv[], void *p);

static int 
doTsarmDMAWRRSet(int argc, char *argv[], void *p);

static int 
doTsarmDMAWRRRule(int argc, char *argv[], void *p);

static int 
doTsarmHwMpoa(int argc, char *argv[], void *p);

static int 
doTsarmHwVlanUnTagSwitch(int argc, char *argv[], void *p);

static int 
doTsarmLpbk(int argc, char *argv[], void *p);

static int 
doTsarmAX4klpbk(int argc, char *argv[], void *p);

static int 
doTsarmMode(int argc, char *argv[], void *p);

static int 
doTsarmDbg(int argc, char *argv[], void *p);

void
tsarmReset(void);

int
tsarmRegDefCheck(void);

int
tsarmRegRWTest(uint32 parttern);

void
sarLpbkInit(void);

static int 
doTsarmLpbkInit(int argc, char *argv[], void *p);

static int 
doTsarmLpbkPatternMode(int argc, char *argv[], void *p);

static int 
doTsarmLpbkFixedPattern(int argc, char *argv[], void *p);

static int 
doTsarmLpbkCellSize(int argc, char *argv[], void *p);

static int 
doTsarmLpbkSingle(int argc, char *argv[], void *p);

static int 
doTsarmLpbkTraffic(int argc, char *argv[], void *p);

static int 
doTsarmLpbkAll(int argc, char *argv[], void *p);

static int 
doTsarmMaxRate(int argc, char *argv[], void *p);

static int 
doTsarmLpbkData(int argc, char *argv[], void *p);

static int 
doTsarmLpbkOam(int argc, char *argv[], void *p);

static int 
doTsarmLpbkCc(int argc, char *argv[], void *p);

static int 
doTsarmLpbkCcCont(int argc, char *argv[], void *p);

static int 
doTsarmLpbkVlan(int argc, char *argv[], void *p);

reg_check_t sar_reg[]=
{
	/*Register name , Type, Address, Default Value, Mask*/
	{"TSARM_RAI", RW, (TSARM_REGISTER_BASE + 0x0000), 0x00000006, 0x00000001},
	{"TSARM_GFR", RW, (TSARM_REGISTER_BASE + 0x0004), 0xc0000000, 0x000076fb},
	{"TSARM_TSTBR", RW, (TSARM_REGISTER_BASE + 0x0008), 0x003fffff, 0x003fffff},
	{"TSARM_RMPLR", RW, (TSARM_REGISTER_BASE + 0x000c), 0x00000fff, 0x00000fff},
	{"TSARM_TXSLRC", RW, (TSARM_REGISTER_BASE + 0x0034), 0x00010001, 0x007f0fff},
	{"TSARM_CCFILTER", RW, (TSARM_REGISTER_BASE + 0x003c), 0x00000000, 0x000007ff},
	{"TSARM_VCCR", (RW|VC_TYPE), (TSARM_REGISTER_BASE + 0x0100), 0x00000000, 0x0ffffff7},
	{"TSARM_CCCR", RW, (TSARM_VCCR_BASE + 0x0040), 0x00000000, 0x00000001},
	{"TSARM_PCR", (RW|VC_TYPE), (TSARM_REGISTER_BASE + 0x0400), 0x00000000, 0x007f0fff},
	{"TSARM_SCR", (RW|VC_TYPE), (TSARM_REGISTER_BASE + 0x0440), 0x00000000, 0x007f0fff},
	{"TSARM_MBSTP", (RW|VC_TYPE), (TSARM_REGISTER_BASE + 0x0480), 0x00000000, 0x3fffffff},
	{"TSARM_MAX_FRAME_SIZE", (RW|VC_TYPE), (TSARM_REGISTER_BASE + 0x04c0), 0x00000000, 0x0000007f},
	{"TSARM_TRAFFIC_SHAPER_WEIGHT", (RW|VC_TYPE), (TSARM_REGISTER_BASE + 0x0500), 0x00000000, 0x0fff0fff},
	{"TSARM_TDCNT", (RW|VC_TYPE), (TSARM_REGISTER_BASE + 0x0600), 0x00000000, 0x00000000},
	{"TSARM_TDCNTCC", RW, (TSARM_TDCNT_BASE + 0x0040), 0x00000000, 0x00000000},
	{"TSARM_RDCNT", (RW|VC_TYPE), (TSARM_REGISTER_BASE + 0x0700), 0x00000000, 0x00000000},
	{"TSARM_RDCNTCC", RW, (TSARM_RDCNT_BASE + 0x0040), 0x00000000, 0x00000000},
	{"TSARM_MISCNT", RW, (TSARM_RDCNT_BASE + 0x0044), 0x00000000, 0x00000000},
	{"TSARM_MPOA_GCR", RW, (TSARM_REGISTER_BASE + 0x0800), 0x00000000, 0x03ff03ff},
	{"TSARM_VC_MPOA_CTRL", (RW|VC_TYPE), (TSARM_REGISTER_BASE + 0x0810), 0x00000000, 0x000007ff},
	{"TSARM_MPOA_HFIV11", RW, (TSARM_REGISTER_BASE + 0x0850), 0x00000000, 0x00ffffff},
	{"TSARM_MPOA_HFIV12", RW, (TSARM_REGISTER_BASE + 0x0854), 0x00000000, 0x00ffffff},
	{"TSARM_MPOA_HFIV13", RW, (TSARM_REGISTER_BASE + 0x0858), 0x00000000, 0x00ffffff},
	{"TSARM_MPOA_HFIV21", RW, (TSARM_REGISTER_BASE + 0x0860), 0x00000000, 0x00ffffff},
	{"TSARM_MPOA_HFIV22", RW, (TSARM_REGISTER_BASE + 0x0864), 0x00000000, 0x00ffffff},
	{"TSARM_MPOA_HFIV23", RW, (TSARM_REGISTER_BASE + 0x0868), 0x00000000, 0x00ffffff},
	{"TSARM_MPOA_HFIV31", RW, (TSARM_REGISTER_BASE + 0x0870), 0x00000000, 0x0000ffff},	
	{"TSARM_MPOA_HFIV32", RW, (TSARM_REGISTER_BASE + 0x0874), 0x00000000, 0x0000ffff},
	{"TSARM_MPOA_HFIV33", RW, (TSARM_REGISTER_BASE + 0x0878), 0x00000000, 0x0000ffff},
	{"TSARM_MPOA_HFIV41", RW, (TSARM_REGISTER_BASE + 0x0880), 0x00000000, 0x0000ffff},
	{"TSARM_MPOA_HFIV42", RW, (TSARM_REGISTER_BASE + 0x0884), 0x00000000, 0x0000ffff},
	{"TSARM_MPOA_HFIV43", RW, (TSARM_REGISTER_BASE + 0x0888), 0x00000000, 0x0000ffff},
	{NULL, 0x0, 0x0, 0x0, 0x0}
};

struct irq_event_s 
irq_event_table[]=
{
	{LIRQ_TX_U_DONE,		"LIRQ_TX_U_DONE"},
	{LIRQ_TX_M_DONE,		"LIRQ_TX_M_DONE"},
	{LIRQ_RX_U_DONE,		"LIRQ_RX_U_DONE"},
	{LIRQ_RX_M_DONE,		"LIRQ_RX_M_DONE"},
	{LIRQ_TX_BUF_DONE,	"LIRQ_TX_BUF_DONE"},
	{LIRQ_TX_U_BD_UF,	"LIRQ_TX_U_BD_UF"},
	{LIRQ_TX_M_BD_UF,	"LIRQ_TX_M_BD_UF"},
	{LIRQ_TX_SW_DIS,		"LIRQ_TX_SW_DIS"},
	{LIRQ_RX_U_BD_OV,	"LIRQ_RX_U_BD_OV"},
	{LIRQ_RX_M_BD_OV,	"LIRQ_RX_M_BD_OV"},
	{LIRQ_RX_MAXLENE,	"LIRQ_RX_MAXLENE"},
	{LIRQ_RX_U_BOV,		"LIRQ_RX_U_BOV"},
	{LIRQ_RX_M_BOV,		"LIRQ_RX_M_BOV"},
	{LIRQ_RX_CRC10E,		"LIRQ_RX_CRC10E"},
	{LIRQ_RX_CRC32E,		"LIRQ_RX_CRC32E"},
	{LIRQ_RX_LENE,		"LIRQ_RX_LENE"},
	{LIRQ_RX_INACT_VC,	"LIRQ_RX_INACT_VC"},
	{LIRQ_IRQ_Q_FULL,		"LIRQ_IRQ_Q_FULL"},
	{0,	NULL}
};


#define BURST_TRAFFIC	0
#define SYNC_TRAFFIC	1

#define PATTERN_FF		0
#define PATTERN_00		1
#define PATTERN_5A		2
#define PATTERN_A5		3

#if 1
uint8 dataCell[1024];
#endif

#ifdef TCSUPPORT_AUTOBENCH
int autobench_sar_lpbk_flag = 0;
#endif

/*CI-CMD Warring message*/
#define SAR_MODE_WRRMSG	"Usage:mode <np|tsa_wrr> <on|off>\r\n"
/*End CI-CMD Warring message*/

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%	TSARM CI Commend
%%____________________________________________________________________________*/

/*____________________________________________________________________________*/
static const cmds_t tsarmRootCmd[] = {
	{"regdump",			doTsarmRegDump,			0x02,	1,	"vc reg <vc>"},		/* ok */
	{"reset",			doTsarmReset,			0x02,	0,	NULL},				/* ok */
	{"reg_check",		doTsarmRegCheck,		0x02,	0,	NULL},				/* ok */
	{"lineRate",		doTsarmLineRate,		0x02,	1,	"lineRate <line-rate>"},	/* ok */
	{"rate",			doTsarmRate,			0x02,	7,	"rate <vc> <C|U|Q|V|G> <pcr> <scr> <mbs> <clp0> <clp1>"}, /* ok */
	{"dma_wrr",		doTsarmDMAWRR,	0X02,	0,	NULL},	/* ok */
	{"hw_mpoa",		doTsarmHwMpoa,	0x02,	5,	"hw_mpoa <vc:0~9> <vpi:0~255> <vci:1~65535> <muxType:1:LLC, 2:VC> <encapType:1:POE,2:1483-R,3:POA,4:1483-B>"}, /* ok */
 //  frankliao modified 20100713
 //	{"hw_vlan_untag",	doTsarmHwVlanUnTagSwitch,	0x02,	1,	"hw_vlan_untag <on|off>"},
	{"hw_vlan_untag",	doTsarmHwVlanUnTagSwitch,	0x02,	2,	"hw_vlan_untag <vc:0~9> <on|off>"}, 
	{"lpbk",			doTsarmLpbk,				0x02,	0,	NULL}, /* wait modify */
	{"ax4k_lpbk",		doTsarmAX4klpbk,		0x02,	0,	"ax4k_lpbk <on|off>"}, /* ok */
	{"mode",			doTsarmMode,		0x02,	2,	"mode <np|tsa_wrr> <on|off>"}, /* ok */
	{"dbg",			doTsarmDbg,		0x02,	1,	"dbg <on|off>"},					/* ok */
	{"maxRate",		doTsarmMaxRate,	0x02,	1,	"maxRate <on|off>"}, 				/* ok */
    {NULL,	NULL,	0x10,	0,	NULL},	
};

static const cmds_t tsarmWrrCmd[] = {
	{"switch", doTsarmDMAWRRSwitch,0x02, 1,	"switch <on|off>"},										/* ok */
	{"set",	doTsarmDMAWRRSet, 0x02,	8, "set PRI0 PRI1 PRI2 PRI3 PRI4 PRI5 PRI6 PRI7"},				/* ok */
	{"rule", doTsarmDMAWRRRule,	0x02, 8, "rule Rule0 Rule1 Rule2 Rule3 Rule4 Rule5 Rule6 Rule7"}, 	/* ok */
	{NULL,	NULL, 0x10,	0, NULL},
};

static const cmds_t tsarmLpbkCmd[] = {
	{"init",	doTsarmLpbkInit,			0x02,	0,	"tsarm lpbk init"},				/* ok */
	{"pattern", doTsarmLpbkPatternMode, 0x2, 1, "pattern <mode:0~1, 0:fixed pattern, 1:random pattern>"},
	{"fixed_pattern", doTsarmLpbkFixedPattern, 0x02, 1, "fixed pattern <type:0~3: 0:0xff, 1:0x00, 2:0x5a, 3:0xa5>"},
	{"cell_size", doTsarmLpbkCellSize, 0x02, 1, "cell size <size:64~1024>"},
	{"single", doTsarmLpbkSingle, 0x02,	0, NULL},
	{"test_traffic", doTsarmLpbkTraffic, 0x02, 1, "test traffic <traffic:0~1, 0:burst traffic, 1:sync traffic>"},
	{"all",	doTsarmLpbkAll, 0x02, 4, "all <raw:0|1> <mpoa:0|1> <delay> <count>"},
	{NULL, NULL, 0x10, 0, NULL},
};

static const cmds_t tsarmLpbkSingleCmd[] = {
	{"data",	doTsarmLpbkData,		0x02,	5,	"data <vc:0~9> <vpi:0~255> <vci:1~65535> <priority:0~3> <phy:0~1>"}, /* ok */
	{"oam",	doTsarmLpbkOam,		0x02,	6,	"oam <vc:0~9> <vpi:0~255> <vci:1~65535> <pti:3~4> <type:0~3> <phy:0~1>"}, /* ok */
	{"cc",	doTsarmLpbkCc,			0x02,	4,	"cc <vc:0~9> <vpi:0~255> <vci:3~4> <type:0~3> <phy:0~1>"},	/* ok */
	{"cc_cont",	doTsarmLpbkCcCont,			0x02,	4,	"cc_cont <vpi:0~2> <vci:0~16> <pti:0~6> <clp:0~1>"}, /* ok */
	{"vlan",	doTsarmLpbkVlan,		0x02,	5,	"vlan <vc:0~9> <vpi:0~255> <vci:1~65535> <priority:0~3> <vlanId>"},		/* ok */
	{NULL,	NULL,					0x10,	0,	NULL},
};

/*_____________________________________________________________________________
**      function name: putVal
**      descriptions:
**        Put a value in host order into a char array in network order
**             
**      parameters:
**           cp: Put the specify the buffer address.
**           x:Specify the value.
**           len: 2, 4 bytes
**             
**      global:
**            None
**             
**      return:
**           Buffer address.
**	     
**      call:
**           None
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
uint8 *
putVal(
	uint8	*cp,
	uint32	x,
	uint8 len
)
{	
	switch(len){
		case 4:
			*cp++ = x >> 24;
			*cp++ = x >> 16;
			/* FALL THROUGH */
		case 2:
			*cp++ = x >> 8;
			*cp++ = x;
			break;
		default:
			break;
	}
	return cp;
}/*end putVal*/

/*_____________________________________________________________________________
**      function name: get16
**      descriptions:
**        Get a 2bytes value of the buffer address.
**             
**      parameters:
**           cp: Put the specify the buffer address.
**             
**      global:
**            None
**             
**      return:
**           Value
**	     
**      call:
**           None
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
uint16
get16(
	uint8	*cp
)
{
	register uint16 x;

	x = *cp++;
	x <<= 8;
	x |= *cp;
	return x;
}/*end get16*/

/*_____________________________________________________________________________
**      function name: dumpIRQEvent
**      descriptions:
**        Dump irq event message. 
**             
**      parameters:
**           vc: Current number is used for display vc information
**           irq_report: IRQ Event 
**             
**      global:
**            irq_event_table
**             
**      return:
**           Value
**	     
**      call:
**           None
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
void 
dumpIRQEvent(uint8 vc,uint32 irq_report){
	int i=0;
	if(sarVerifyDbg){
		printk("IRQ Report(vc:%02x value:0x%08lx):", vc, irq_report);
		for(i=0; irq_event_table[i].name!=NULL; i++){
			if(irq_report & irq_event_table[i].id){
				printk("%s ",irq_event_table[i].name);
			}
		}
		printk("\r\n");
	}
}/*end dumpIRQEvent*/

/*_____________________________________________________________________________
**      function name: patternGen
**      descriptions:
**        Generate pattern for atm data cell is used for local loop back test. 
**             
**      parameters:
**           seq: Sequence number.
**           priority: 
**             
**      global:
**            test_flag
**             
**      return:
**           Value
**	     
**      call:
**           None
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
struct sk_buff* 
patternGen(uint32 seq, uint8 priority){

/*
		Pattern Format
|Offset(8	)	|Seq(24)				|
|Seq(8)		| len(16)		|sed(8)	|
|Pri(8)		| Data.....				|
|			........				|
*/
	struct sk_buff* skb=NULL;
	uint16 size = 0;
	uint8 seed;
	uint16 i;
	uint8 *ptr;
	uint8 offset=0;

	skb=skbmgr_dev_alloc_skb2k();
	if(!skb){
		printk("patternGen: Alloc skb fail\r\n");
		return NULL;
	}
	else{

		/*Fill the offset value*/
		*(skb->data + offset) = offset;
		/*Fill the seq value*/
		/*put32(skb->data + offset+1, seq);*/
		putVal(skb->data + offset+1, seq, 4);

		/*Limit max data length to 1023 bytes*/
		if (test_flag & RAND_DCELL_FG){
			size = (rand() & 0x3ff) + 16;
		}


		if (test_flag & FIX_DCELL_FG){
			size = test_fixed_size;
		}

		/*
		**If user active RAW_CELL_FG, it's mean to test raw cell,
		**raw cell size must be multiple of 48.
		*/
		if(test_flag & RAW_CELL_FG){
			size = RAW_CELL_SIZE;
			putVal(skb->data + offset+OFFSET_LEN +SEQ_LEN, RAW_CELL_SIZE, 2);
		}
		else{
			/* Fill the length of the data cell */
			putVal(skb->data + offset+OFFSET_LEN +SEQ_LEN, (size -offset) , 2);
		}

		if (test_flag & FIX_DCELL_FG){
			memcpy(skb->data, dataCell, size+offset);
		} else {
			/* Generate seed */
			seed = (rand() & 0xff);
			if(sarVerifyDbg){
				printk("seed :%02x size:%0x offset:%0x\r\n", seed, size, offset);
			}
			/*Fill in seed and priority*/
			*(skb->data +OFFSET_LEN+SEQ_LEN+PATTERN_LEN+ offset) = seed;
			*(skb->data +OFFSET_LEN+SEQ_LEN+PATTERN_LEN+PRIO_LEN + offset) = priority;

			/*Fill the testing content of data cell*/
			ptr = skb->data + TOTAL_PAT_LEN+ offset;
			for(i = TOTAL_PAT_LEN + offset; i < (size+offset); i++){
				*ptr++ = (seed++) & 0xff;
			}
			/*Offset the skb data pointer*/
			skb->data+=offset;
		}	
	
		/*Count the skb data length*/
		if(test_flag & RAW_CELL_FG){
			skb_put(skb, RAW_CELL_SIZE);
		}
		else{
			skb_put(skb, (size-offset));
		}
	}
	return skb;
}

/*_____________________________________________________________________________
**      function name: patternCheck
**      descriptions:
**        	Check the data cell content is fit with transmit data pattern. 
**             
**      parameters:
**           skb: Sequence number.
**           len: Length of atm data cell.
**             
**      global:
**            None
**             
**      return:
**           Pattern is match : 0
**           Pattern is mismatch:1
**	     
**      call:
**           get16
**           dbg_plinel_1
**           dumpCell
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
uint8 
patternCheck(uint8* data_p, int len)
{
	uint16 size;
	uint8 seed=0;
	uint16 i;
	
	size = get16(data_p +OFFSET_LEN+SEQ_LEN);
	if (size != len){
		printk("\r\n size:%02x Real length:%02x", size, len);
		dumpCell(data_p, len);
		return 1;
	}
	
	/*Get the seed value from header*/
	seed = *(data_p + OFFSET_LEN+SEQ_LEN+PATTERN_LEN);
	/*Compare the data cell content*/
 	for ( i = TOTAL_PAT_LEN; i < size; i++){
		if(data_p[i]!=(uint8)((seed++)& 0xff)){
			printk("\r\n i=:%02x data_p=%02x seed=%02x", i, data_p[i], (seed & 0xff));
			dumpCell(data_p, len);
			return 1;
		}
 	}
	return 0;
}
/*_____________________________________________________________________________
**      function name: ccCellCheck
**      descriptions:
**        	Check the cc cell content is fit with transmit data pattern. 
**             
**      parameters:
**           data_p: CC cell data content pointer.
**           len: Length of atm cc cell.
**             
**      global:
**            None
**             
**      return:
**           Pattern is match : 0
**           Pattern is mismatch:1
**	     
**      call:
**           dumpCell
**      
**      revision:
**      1. Here 2008/12/19
**____________________________________________________________________________
*/
uint8 
ccCellCheck(uint8* data_p, int len)
{
	uint8 seed=0;
	uint16 i;

	/*Fix header format*/
	for(i=0;i<3;i++){
		if(data_p[i]!=0){
			printk("CC cell header is wrong\r\n");
			return 1;
		}
	}
	/*The vci value is 3 or 4*/
	if((data_p[3]==0x30) || (data_p[3]==0x40)){
		/*Get the seed value from header*/
		seed =data_p[4];
		/*Compare the data cell content*/
//	 	for ( i = 5; i < len; i++){
	 	for ( i = 5; i < (len-2); i++){
			if(data_p[i]!=(uint8)((seed++)& 0xff)){
				printk("\r\n i=:%02x data_p=%02x seed=%02x", i, data_p[i], (seed & 0xff));
				dumpCell(data_p, len);
				return 1;
			}
	 	}
	}
	else{
		printk("cc cell vci value is wrong, value:%02x\r\n", data_p[2]);
		return 1;
	}
	
	return 0;
}

/*_____________________________________________________________________________
**      function name: doTsarm
**      descriptions:
**            Root of tsarm ci-cmd.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
int 
doTsarm(int argc, char *argv[], void *p)
{
	return subcmd(tsarmRootCmd, argc, argv, p);
}/*end doTsarm*/

static int 
doTsarmRegDump(int argc, char *argv[], void *p)
{
	uint16 vc;
	vc=simple_strtoul(argv[1], NULL, 10);
	atmRegDump(vc);
	return 0;
}/*end doTsarmRegDump*/


static int 
doTsarmReset(int argc, char *argv[], void *p)
{
	atmInit();
	return 0;
}/*end doTsarmRegCheck*/



/*_____________________________________________________________________________
**      function name: doTsarmRegCheck
**      descriptions:
**           Verify registers of SAR Modules, It's inclued default value and read/write test. 
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	tsarmReset
**      	doTsarmRegCheck
**      	tsarmRegRWTest
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmRegCheck(int argc, char *argv[], void *p)
{
	if(argc <= 1){
		tsarmReset();
		if(tsarmRegDefCheck() == -1){
			printk("SAR Register default value verification is failure!!\r\n");
		}
		else{
			printk("SAR Register default value verification is ok!!\r\n");
		}
	}else if(argc == 2){
		tsarmReset();
		if(tsarmRegRWTest((unsigned long)simple_strtoul(argv[1], NULL, 16))==-1){
			printk("SAR Register  Read/Write verification is failure!!\r\n");
		}
		else{
			printk("SAR Register Read/Write verification is ok!!\r\n");
		}
		tsarmReset();
	}
	else{
		printk("Usage:tsarm reg_check <partten>\n");
	}
	return 0;
}/*end doTsarmRegCheck*/

/*_____________________________________________________________________________
**      function name: doTsarmLineRate
**      descriptions:
**           To limit line-rate.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            treg_tslr
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	mt7510AtmRateCalCulate
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmLineRate(int argc, char *argv[], void *p)
{
	int linerate;
	uint32 lr_dec;
	uint32 lr_init;

	linerate=simple_strtoul(argv[1], NULL, 10);
	mt7510AtmRateCalCulate((uint16)treg_tstbr, (uint16)linerate, &lr_dec, &lr_init);
	TSARM_TXSLRC = (lr_dec << 16) | lr_init;
	printk("TSARM_TXSLRC:0x%08lx\n", (uint32)TSARM_TXSLRC);
	return 0;
}/*end doTsarmLineRate*/

/*_____________________________________________________________________________
**      function name: doTsarmDMAWRR
**      descriptions:
**            Root ci-cmd for local loop back test.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            tsarmLpbkCmd
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	subcmd
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmDMAWRR(int argc, char *argv[], void *p)
{
	return subcmd(tsarmWrrCmd, argc, argv, p);
}/*end doTsarmLpbkTest*/

/*_____________________________________________________________________________
**      function name: doTsarmDMAWRRSwitch
**      descriptions:
**           Switch hardware WRR funcionality to enable or disable.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmDMAWRRSwitch(int argc, char *argv[], void *p)
{
	if(argc == 2){
		if( strcmp(argv[1], "on") == 0){
			TSARM_GFR |= (1 << 15);
			printk("dma_wrr on!\r\n");
		}
		if( strcmp(argv[1], "off") == 0){
			TSARM_GFR &= ~(1<<15);
			test_flag &=~HW_WRR_RULE_FG;
			printk("dma_wrr off!\r\n");			
		}		
	}else{
		printk("usage:tsarm dma_wrr <on|off>\r\n");
	}
	return 0;
}/*end doTsarmDMAWRRSwitch*/

/*_____________________________________________________________________________
**      function name: doTsarmDMAWRRSet
**      descriptions:
**           Set the 4 wrr priority queue length.The queue value is between 0 and 15.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmDMAWRRSet(int argc, char *argv[], void *p)
{
	QDMA_TxQosScheduler_T txQos ;
	int wrr_pri[8];
	int i=0, j=0;
		
	for(i=0; i<8; i++){
		wrr_pri[i]=simple_strtoul(argv[i+1], NULL, 10);
		if((wrr_pri[i] > 15) || (wrr_pri[i] < 0) ){
			printk("PRI0 PRI1 PRI2 PRI3 PRI4 PRI5 PRI6 PRI7 range is (0 ~ 15)\r\n");
			return -1;
		}
	}

	
	for(i=0 ; i<8 ; i++) {
		txQos.channel = i ;
		txQos.qosType = 0;
		for(j=0 ; j<8; j++) {
			txQos.queue[j].weight = wrr_pri[j];
		}
		qdma_set_tx_qos(&txQos) ;
	}
			
	for(i=0 ; i<8; i++) {
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
	
	return 0;
}/*end doTsarmDMAWRRSet*/

/*_____________________________________________________________________________
**      function name: doTsarmDMAWRRRule
**      descriptions:
**           According the tos filed of IP header to enqueue atm cell into different priority queue, this ci-cmd
**          is used to set the tos filed to fit the enqueue rule.
**             
**      0parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            wrrRule
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      
**      revision:
**      1. Here 2008/09/01
**____________________________________________________________________________
*/
static int 
doTsarmDMAWRRRule(int argc, char *argv[], void *p)
{	
	int param[ATM_TX_PRIORITY_MAX];
	int i=0;
	
	for(i=0; i<8; i++){
		param[i]=simple_strtoul(argv[i+1], NULL, 16);
		if((param[i]>MAX_TOS) || (param[i]<0)){
			printk("Rule0 Rule1 Rule2 Rule3 Rule4 Rule5 Rule6 Rule7 range is (0 ~255)\r\n");
			return -1;
		}
	}

	test_flag|=HW_WRR_RULE_FG;
	
	for(i=0; i<8; i++){
		wrrRule[i]=(uint8)param[i];
		printk("Rule%x: is 0x%x\t",i, wrrRule[i]);
	}
	printk("\r\n");
	return 0;
}

/*_____________________________________________________________________________
**      function name: doTsarmHwMpoa
**      descriptions:
**              Set up the VC with basic parametes and enable hardware mpoa funcationality that  is used
**              to test hardware mpoa funcationality 
**              
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            test_flag
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	openAtmVc
**      	initMPOA
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmHwMpoa(int argc, char *argv[], void *p)
{
	int param[5];/*0:VC, 1:VPI, 2:VCI,3:LLC or VC Type, 4:Encap Type*/
	qosProfile_t qos;
	int i=0;
	struct sk_buff* skb=NULL;
		
	for(i=0; i<5; i++){
		param[i]=simple_strtoul(argv[i+1], NULL, 10);
	}
	
	if((param[0]>=ATM_VC_MAX)
	||(param[1]>MAX_VPI)
	||((param[2]>MAX_VCI) ||(param[2]<MIN_VCI) )	
	||((param[3]>MUX_VC) ||(param[3]<=MUX_NONE) )
	||(param[4] > (ENCAP_MER-1)) ||(param[4] < (ENCAP_POE-1))){
		printk("Usage:vc <vc:0~9> <vpi:0~255> <vci:1~65535> \
<muxType:1:LLC, 2:VC> <encapType:1:POE,2:1483-R,3:POA,4:1483-B>\r\n");
		return -1;
	}
	
	test_flag |= HW_MPOA_FG;
	openAtmVc(param[0], param[1], param[2], VCCFGR_ATM_PHY0);
//	openAtmVc(param[0], param[1], param[2]);
	/*Set VC MPOA Parameters*/
	qos.muxType=param[3];
	switch(param[4]){
		case 1:/*POE*/
		case 4:/*1483 Bridged Mode*/
			qos.mode=p_bridged;
			qos.encapType=0;
			break;
		case 2: /*1483 Routed Mode*/
			qos.mode=p_routed;
			qos.encapType=0;
			break;
		case 3:/*POA*/
			qos.mode=p_routed;
			qos.encapType=1;
			break;
		default:
			break;
	}
	
	initMPOA(param[0],&qos);
	/*Send data cell*/
	skb=skbmgr_dev_alloc_skb2k();

	if(skb){
		memcpy(skb->data, dataCell, sizeof(dataCell));
		skb_put(skb, sizeof(dataCell));
		atmAal5DataReqVerify(skb, param[0], 3);	
	}
	else{
		printk("Allocate skb failure!\r\n");
	}
	/*end send data cell*/
	return 0;
}/*end doTsarmHwMpoa*/

/*_____________________________________________________________________________
**      function name: doTsarmHwVlanUnTagSwitch
**      descriptions:
**            Switch hardware VLAN un-tag funcionality to enable or disable.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	
**      
**      revision:
**      1. Here 2008/08/15
**      2. frankliao 2010/07/13
**         Chip: TC3182 TC3162
**
**         Question: While doing hareware vlan untag test, using this command cannot
**                   turn on/off hareware vlan untag
**
**         Root cause: The control register setting for hardware vlan untag is not done
**                     in this function. Otherwise, the setting for turning on hareware
**                     vlan untag is done in initMPOA function, and the setting for
**                     for turning off hareware vlan untag is not supported.
**
**         Solution: add a parameter for vc number and add the control register setting
**                   (on/off) for hardware vlan untag in this function
**____________________________________________________________________________
*/
static int 
doTsarmHwVlanUnTagSwitch(int argc, char *argv[], void *p)
{
    int vc;
	if(argc == 3){

        vc = simple_strtoul(argv[1], NULL, 10);
        if (vc<0 || vc>9) {
             printk("Usage: tsarm hw_vlan_untag [vc:0~9] [on|off]\r\n");
             return -1;
        }

		if( strcmp(argv[2], "on") == 0){
			test_flag|= HW_VLAN_UNTAG_FG;
            // frankliao added 20100713
            TSARM_VC_MPOA_CTRL(vc) |= HW_VLAN_UNTAG_FG;
			printk("Hardware VLAN un-tag Switch On!\n");
		}
		else if( strcmp(argv[2], "off") == 0){
			test_flag &= ~HW_VLAN_UNTAG_FG;	
            // frankliao added 20100713
            TSARM_VC_MPOA_CTRL(vc) &= ~HW_VLAN_UNTAG_FG;
			printk("Hardware VLAN un-tag Switch Off!\n");	
		}	
        else {
            printk("Usage: tsarm hw_vlan_untag [vc:0~9] [on|off]\r\n");
            return -1;
        }	
	}
	else{
        printk("Usage: tsarm hw_vlan_untag [vc:0~9] [on|off]\r\n");
        return -1;
	}
		
	return 0;
}/*end doTsarmHwVlanUnTagSwitch*/

/*_____________________________________________________________________________
**      function name: doTsarmLpbk
**      descriptions:
**            Root ci-cmd for local loop back test.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            tsarmLpbkCmd
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	subcmd
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmLpbk(int argc, char *argv[], void *p)
{
	return subcmd(tsarmLpbkCmd, argc, argv, p);
}/*end doTsarmLpbk*/

/*_____________________________________________________________________________
**      function name: doTsarmLpkInit
**      descriptions:
**            Set General Configuration Register to enable local loop.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	subcmd
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmLpbkInit(int argc, char *argv[], void *p)
{
	txcount1=0;
	txcount2=0;
	rxcount1=0;
	rxcount2=0;
	test_flag=0;
	stop_loop=0;

	sarLpbkInit();
	return 0;
}/*end doTsarmLpkInit*/

static int 
doTsarmLpbkPatternMode(int argc, char *argv[], void *p)
{
	int param;/*1: pattern mode*/

	param=simple_strtoul(argv[1], NULL, 10);

	switch(param){/*Loopback Pattern Type*/
		/* Fixed Pattern */
		case 0:
			test_flag &= (~RAND_DCELL_FG);
			test_flag |= FIX_DCELL_FG;
			break;
		/* Random Pattern */
		case 1:
			test_flag &= (~FIX_DCELL_FG);
			test_flag |= RAND_DCELL_FG;
			break;
		/* Fixed Random Pattern */
		case 2:
			test_flag |= RAND_DCELL_FG;
			test_flag |= FIX_DCELL_FG;
			break;
		default:
			printk("Usage: pattern <mode:0~1, 0:fixed pattern, 1:random pattern>\r\n");
			return -1;
	}

	return 0;
}/*end doTsarmLpbkPatternMode*/

static int 
doTsarmLpbkCellSize(int argc, char *argv[], void *p)
{
	uint size;
 
	size=simple_strtoul(argv[1], NULL, 10);

	if (size<MIN_DATA_SIZE || size>1024){
		printk("Usage: cell size <size: %x~1024>\r\n", MIN_DATA_SIZE);
		return -1;
	}	

	test_fixed_size = size;

	return 0;
}/*end doTsarmLpbkFixedPattern*/


/*_____________________________________________________________________________
**      function name: doTsarmLpbkFixedPattern
**      descriptions:
**            Set General Configuration Register to enable local loop.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	subcmd
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmLpbkFixedPattern(int argc, char *argv[], void *p)
{
	int param;/*0: pattern ,1: lenght*/

	param=simple_strtoul(argv[1], NULL, 10);

	switch(param){/*Loopback Pattern Type*/
		/* PATTERN_FF */
		case 0:
			dataCell[OFFSET_LEN+SEQ_LEN+PATTERN_LEN] = 0x0;
			memset(&dataCell[TOTAL_PAT_LEN], 0xff, sizeof(dataCell)-TOTAL_PAT_LEN);
			break;
		/* PATTERN_00 */
		case 1:
			dataCell[OFFSET_LEN+SEQ_LEN+PATTERN_LEN] = 0x1;
			memset(&dataCell[TOTAL_PAT_LEN], 0x00, sizeof(dataCell)-TOTAL_PAT_LEN);
			break;
		/* PATTERN_5a */
		case 2:
			dataCell[OFFSET_LEN+SEQ_LEN+PATTERN_LEN] = 0x2;
			memset(&dataCell[TOTAL_PAT_LEN], 0x5a, sizeof(dataCell)-TOTAL_PAT_LEN);
			break;
		/* PATTERN_a5 */
		case 3:
			dataCell[OFFSET_LEN+SEQ_LEN+PATTERN_LEN] = 0x3;
			memset(&dataCell[TOTAL_PAT_LEN], 0xa5, sizeof(dataCell)-TOTAL_PAT_LEN);
			break;
		default:
			printk("Usage: fixed pattern <type:0~3, 0:0xff, 1:0x00, 2:0x5a, 3:0xa5>\r\n");
			return -1;
	}

	return 0;
}/*end doTsarmLpbkFixedPattern*/

/*_____________________________________________________________________________
**      function name: doTsarmLpkSingle
**      descriptions:
**           CI-command for test local loop back at single VC.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmLpbkSingle(int argc, char *argv[], void *p)
{
	return subcmd(tsarmLpbkSingleCmd, argc, argv, p);
}/*end doTsarmLpkSingle*/


static int 
doTsarmLpbkTraffic(int argc, char *argv[], void *p)
{
	int param;	
	param=simple_strtoul(argv[1], NULL, 10);

	if (param == BURST_TRAFFIC){	
		test_traffic = 0;
	} else {
		test_traffic = 1;
	}

	return 0;
}/*end doTsarmLpkSingle*/


/*_____________________________________________________________________________
**      function name: doTsarmLpkAll
**      descriptions:
**           CI-command for test local loop back at all VC.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmLpbkAll(int argc, char *argv[], void *p)
{
	int param[4];/*0:Raw Cell Test, 1:Mpoa Test,2:Delay Time,3:count*/
	int i=0;

	for(i=0; i<4; i++){
		param[i]=simple_strtoul(argv[i+1], NULL, 10);
	}

	/*Check the value of user input is validly or not*/
	for(i=0; i<2; i++){
		if(param[i]>1){
			printk("Usage:all <raw:0|1> <mpoa:0|1> <delay> <count>\n");
			return -1;
		}
	}

	if (param[3]<=0){
		printk("Loopback test count must larger than 0\n");
	}

	//reset test_flag. shnwind add 20100609. 
//	test_flag = 0; 
	for(i=0; i<2; i++){
		if(param[i]==1){
			test_flag |=(RAW_CELL_FG<<i);
		}
	}

//	test_flag |= RAND_DCELL_FG|OAM_CELL_FG|CC_CELL_FG;
	test_flag |= OAM_CELL_FG|CC_CELL_FG;
	printk("test_flag:%02x\r\n", test_flag);
//	sarLpbkInit();
	allCellTest(param[2],param[3]);

	return 0;
}/*end doTsarmLpkAll*/


/*_____________________________________________________________________________
**      function name: doTsarmMaxRate
**      descriptions:
**           Set all VCs with CBR and let the hardware to send full speed atm cell.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            None
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	atmReset
**      
**      revision:
**      1. Here 2008/09/25
**____________________________________________________________________________
*/
static int 
doTsarmMaxRate(int argc, char *argv[], void *p)
{
	int vc=0;
	if(!strcmp(argv[1],"on")){
		/*Qos Parameters*/
		TSARM_TXSLRC= (1 << 16) | 1;
		/*end qos parameters*/
		for(vc=0;vc<ATM_VC_MAX; vc++){
			/*Set QoS Parameters*/
			TSARM_MBSTP(vc) = (3 << 28) | (0 << 26) | (1 << 2) | TSARM_QOS_CBR;
			TSARM_PCR(vc) = (1 << 16) | 1;
			TSARM_SCR(vc) = (1 << 16) | 1;
			/*end Set QoS Parameters*/
		}
	}
	else{
//		atmReset();
		printk("Software Reset ATM SAR\r\n");
	}
	return 0;
}/*end doTsarmMaxRate*/
/*_____________________________________________________________________________
**      function name: doTsarmLpbkData
**      descriptions:
**           CI-command for test atm data cell on local loop back.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	dataCellTest
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
static int 
doTsarmLpbkData(int argc, char *argv[], void *p)
{
	int param[5];/*0:VC, 1:VPI, 2:VCI, 3: Priority*/
	int i=0;
	struct sk_buff* skb=NULL;
	
	for(i=0; i<5; i++){
		param[i]=simple_strtoul(argv[i+1], NULL, 10);
	}
	
	printk("<vc:%d> <vpi:%d> <vci:%d> <priority:%d> <phy:%d>\n", param[0], param[1], param[2], param[3], param[4]);

	if((param[0]>=ATM_VC_MAX)
	||(param[1]>MAX_VPI)
	||((param[2]>MAX_VCI) || (param[2]<MIN_VCI))
	||(param[3] >= ATM_TX_PRIORITY_MAX)
	||((param[4] < VCCFGR_ATM_PHY0) || (param[4] > VCCFGR_ATM_PHY1))){
		printk("Usage: tsarm lpbk single data <vc:0~9> <vpi:0~255> <vci:1~65535> <priority:0~3> <phy:0~1>\r\n");
		return -1;
	}
	
	test_flag |= FIX_DCELL_FG;

	openAtmVc(param[0],param[1],param[2], param[4]);
	skb=skbmgr_dev_alloc_skb2k();

	if(skb){
//		memcpy(skb->data, dataCell, sizeof(dataCell));
//		skb_put(skb, sizeof(dataCell));
		if ((test_fixed_size < MIN_DATA_SIZE) || (test_fixed_size > 1024)){
			printk("Test packet size %ld setting error\n", test_fixed_size);
		} else {
			memcpy(skb->data, dataCell, test_fixed_size);
			skb_put(skb, test_fixed_size);
			atmAal5DataReqVerify(skb, param[0], param[3]);	
		}
	} else {
		printk("Allocate skb failure!\r\n");
	}

	delay1ms(1000);
	atmAal5VcClose(param[1], param[2]);
	return 0;
}/*end doTsarmLpbkData*/

/*_____________________________________________________________________________
**      function name: doTsarmLpbkOam
**      descriptions:
**           CI-command for test atm oam cell on local loop back.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	atmOamF4F5DataReq
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
static int 
doTsarmLpbkOam(int argc, char *argv[], void *p)
{
	int param[6];/*0:vc, 1:vpi, 2:vci,  3:pti, 4:type*/
	int i=0;
	
	for(i=0; i<6; i++){
		param[i]=simple_strtoul(argv[i+1], NULL, 10);
	}
	
	if( (param[0]>=ATM_VC_MAX )
	||(param[1]>MAX_VPI)
	||((param[2]>MAX_VCI) ||(param[2]<MIN_VCI))
	||((param[3] > 4) ||(param[3] < 3))
	||(param[4] > 3)
	||((param[5] < VCCFGR_ATM_PHY0) || (param[5] > VCCFGR_ATM_PHY1))){
		printk("Usage: oam < vc:0~9> <vpi:0~255> <vci:1~65535> <pti:3~4> <type:0~3> <phy:0~1>\r\n");
		return -1;
	}
	
	test_flag|=OAM_CELL_FG;
	openAtmVc((uint8)param[0], (uint8)param[1], (uint16)param[2], (uint8)param[5]);

#if 1
	if(atmOamF4F5DataReqVerify(param[1], param[2], 1, (param[3]-3), param[4])==1){
		printk("Send ATM OAM Cell failure!\r\n");
	}
#endif
	
	delay1ms(100);
	atmAal5VcClose((uint8)param[1], (uint16)param[2]);
	return 0;
}/*end doTsarmLpbkOam*/

/*_____________________________________________________________________________
**      function name: doTsarmLpbkCc
**      descriptions:
**           CI-command for test atm cc cell on local loop back.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	dataCellTest
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
static int 
doTsarmLpbkCc(int argc, char *argv[], void *p)
{
	int param[5];/*0:VC, 1:VPI, 2:VCI, 3:type 4:phy*/
	int i=0;
	
	for(i=0; i<5; i++){
		param[i]=simple_strtoul(argv[i+1], NULL, 10);
	}

	if((param[0]>=ATM_VC_MAX )
	||(param[1]>MAX_VPI)
	||((param[2] > 4) ||(param[2] < 3) )
	||(param[3] > 3)
	||((param[4] < VCCFGR_ATM_PHY0) || (param[4] > VCCFGR_ATM_PHY1))){
		printk("Usage: cc <vc:0~9> <vpi:0~255> <vci:3~4> <type:0~3> <phy:0~1>\r\n");
		return -1;
	}

	test_flag|=CC_CELL_FG;
#if 1
	if(atmOamF4F5DataReqVerify(param[1], param[2], 0, (param[2]-3), param[3])==1){
		printk("Send ATM CC Cell failure!\r\n");
	}
#endif
	return 0;
}/*end doTsarmLpbkCc*/
/*_____________________________________________________________________________
**      function name: doTsarmLpbkCcCont
**      descriptions:
**           CI-command for test atm cc filter to filed the cc content.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	dataCellTest
**      
**      revision:
**      1. Here 2009/07/28
**____________________________________________________________________________
*/
static int
doTsarmLpbkCcCont(int argc, char *argv[], void *p){
	int param[4];/*0:VPI, 1:VCI, 2:PTI , 3:CLP*/
	int i=0;
	for(i=0; i<4; i++){
		param[i]=simple_strtoul(argv[i+1], NULL, 10);
	}

	if((param[0]>2 )
	||(param[1]>16)
	||(param[2] > 6)
	||(param[3] > 1)){
		printk("Usage: cc_cont <vpi:0~2> <vci:0~16> <pti:0~6> <clp:0~1>\r\n");
		return -1;
	}

	test_flag|=CC_CONT_FG;
	
	cc_cont.vpi=param[0];
	cc_cont.vci=param[1];
	cc_cont.pti=param[2];
	cc_cont.clp=param[3];

	printk("vpi:%d vci:%d pti:%d clp:%d\r\n",cc_cont.vpi,cc_cont.vci, cc_cont.pti, cc_cont.clp);
	return 0;
}


/*_____________________________________________________________________________
**      function name: doTsarmLpbkVlan
**      descriptions:
**           CI-command for test atm data cell with vlan tag on local loop back.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	dataCellTest
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
static int 
doTsarmLpbkVlan(int argc, char *argv[], void *p){
	int param[5];/*0:vc,1:vpi, 2:vci,3:priority,4:vlanId*/
	
	struct sk_buff *skb = NULL;
	int i=0;
	
	for(i=0; i<5; i++){
		param[i]=simple_strtoul(argv[i+1], NULL, 10);
	}
	
	if((param[0] >= ATM_VC_MAX)
	||(param[1]>MAX_VPI)
	||((param[2]>MAX_VCI) || (param[2]<MIN_VCI))
	||(param[3]>=ATM_TX_PRIORITY_MAX)){
		printk("Usage:vlan <vc:0~9> <vpi:0~255> <vci:1~65535> <priority:0~3> <vlanId>\r\n");
		return -1;
	}

	vlan_id = param[4];
	test_flag |= VLAN_TAG_FG;/*set up the flag*/
	/*set the sequence number into the vlan tag*/
	putVal(&vlanTag[2], vlan_id, 2);

//	openAtmVc(param[0], param[1], param[2]);
	openAtmVc(param[0], param[1], param[2], VCCFGR_ATM_PHY0);
	
	skb=skbmgr_dev_alloc_skb2k();

	if(skb != NULL){
//		memcpy(skb->data, dataCell, sizeof(dataCell));
//		skb_put(skb, sizeof(dataCell));
		memcpy(skb->data, dataCell, test_fixed_size);
		skb_put(skb, test_fixed_size);
	}
	else{
		printk("skbmgr_dev_alloc_skb2k failure!\r\n");
		return -1;
	}

	if(atmAal5DataReqVerify(skb, param[0], param[3])!=0){
		printk("Send atm data cell with vlan tag failure!\r\n");
	}
	
	/*Need to delay wait for transmit atm cell is finished*/
	delay1ms(100);
	atmAal5VcClose(param[1], param[2]);
	return 0;
}

/*_____________________________________________________________________________
**      function name: doTsarmAX4klpbk
**      descriptions:
**           CI-command for test loop back via AX4000.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmAX4klpbk(int argc, char *argv[], void *p)
{
	if(argc == 2){
		if(!strcmp(argv[1],"on")){
			ax4k_lpbk = AX4K_LBPK_EN;/*set up flag to do driver loopback*/
//			atmReset();
			printk("AX4000 loop back on\r\n");
		}
		else if(!strcmp(argv[1],"off")){
			ax4k_lpbk = AX4K_LBPK_DN;
			printk("AX4000 loop back off\r\n");
		}
	}
	else{
		printk("usage tsarm ax4k_lpbk <on|off>\n");
	}
	
	return 0;
}/*end doTsarmAX4klpbk*/
/*_____________________________________________________________________________
**      function name: doTsarmMode
**      descriptions:
**           CI-command for enable hardware funcationality.Current support to enable np only(For UTOPIA)
**           mode and tsa_wrr mode used to test ATM QoS.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmMode(int argc, char *argv[], void *p)
{
	int onOff=0;
	
	if(argc==3){
		if(strcmp(argv[2], "on")==0){
			onOff=1;
		}
		else if(strcmp(argv[2], "off")==0){
			onOff=0;
		}
		else{
			printk(SAR_MODE_WRRMSG);
			return -1;
		}
		
		if(strcmp(argv[1],"np")==0){
			if(onOff==1){
				VPint(CR_AHB_SSR) |=NP_ONLY_MODE;	
				printk("np mode on\r\n");
			}
			else{
				VPint(CR_AHB_SSR) &= ~NP_ONLY_MODE;
				printk("np mode off\r\n");
			}
		}
		else if(strcmp(argv[1],"tsa_wrr")==0){
			if(onOff==1){
				TSARM_GFR |=GFR_TSA_WRR_EN;
				printk("tsa_wrr mode on\r\n");
			}
			else{
				TSARM_GFR &= ~GFR_TSA_WRR_EN;
				printk("tsa_wrr mode off\r\n");
			}
		}
		else{
			printk(SAR_MODE_WRRMSG);
			return -1;
		}
	}
	return 0;
}/*end doTsarmMode*/
/*_____________________________________________________________________________
**      function name: doTsarmDbg
**      descriptions:
**           CI-command for enable/disable debug flag.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	
**      
**      revision:
**      1. Here 2008/09/09
**____________________________________________________________________________
*/
static int 
doTsarmDbg(int argc, char *argv[], void *p){
	if(argc == 2){
		if(!strcmp(argv[1],"on")){
			sarVerifyDbg= 1;
			printk("Verify debug flag is on\r\n");
		}
		else if(!strcmp(argv[1],"off")){
			sarVerifyDbg = 0;
			printk("Verify debug flag is off\r\n");
		}
	}
	else{
		printk("usage: dbg <on|off>\n");
	}
	return 0;
}
/*_____________________________________________________________________________
**      function name: doTsarmRate
**      descriptions:
**           CI-command used to set atm qos for specify vc.
**             
**      parameters:
**            argc:            argument counter
**            argv:            argument array strings
**            p:
**             
**      global:
**            
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**      	
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
static int 
doTsarmRate(int argc, char *argv[], void *p)
{
	int vc;
	int clp0, clp1, LP_TSW, HP_TSW;
	int pcr, scr, mbs;
	uint32 cal_mbs;
	char argv2;
	uint32 pcr_dec;
	uint32 scr_dec;
	uint32 pcr_init;
	uint32 scr_init;	

	sscanf(argv[1], "%d", &vc);
	sscanf(argv[3], "%d", &pcr);
	sscanf(argv[4], "%d", &scr);
	sscanf(argv[5], "%d", &mbs);
	sscanf(argv[6], "%d", &clp0);
	sscanf(argv[7], "%d", &clp1);
	
	argv2 = *(argv[2]);
	switch(argv2){/*ATM QoS Type*/
		case 'C':/*CBR*/
		case 'U':/*UBR*/
			printk("argc is : %d\r\n",argc);
			printk("pcr is : %d\r\n",pcr);
			printk("wrrbase is :%d\r\n",WRRBASE);
			if(pcr == 0){	//to prevent pcr = 0
				pcr = 1;
				HP_TSW = (WRRBASE/pcr);
				LP_TSW = (WRRBASE/pcr);
				pcr = 0;
			}
			else{
				HP_TSW = (WRRBASE/pcr);
				LP_TSW = (WRRBASE/pcr);
			}
			
			if( HP_TSW > MAX_TSW ){
				HP_TSW = MAX_TSW;
				LP_TSW = MAX_TSW;
			}
			
			TSARM_TRAFFIC_SHAPER_WEIGHT(vc) =((LP_TSW & 0xfff) << 16) | ((HP_TSW & 0xfff) << 0);
			mt7510AtmRateCalCulate((uint16)treg_tstbr, (uint16)pcr, &pcr_dec, &pcr_init);
			mbs = 1; // CBR or UBR, mbs must be 1
			TSARM_PCR(vc) = (pcr_dec << 16) | pcr_init;
			TSARM_SCR(vc) = (pcr_dec << 16) | pcr_init;
			if(argv2=='C'){
				TSARM_MBSTP(vc) = ((clp1 & 0x3) << 28) | ((clp0 & 0x3) << 26) | (mbs << 2) | TSARM_QOS_CBR;
			}
			else{
				TSARM_MBSTP(vc) = ((clp1 & 0x3) << 28) | ((clp0 & 0x3) << 26) | (mbs << 2) | TSARM_QOS_UBR;	
			}		
			break;
		case 'Q':/*UBR+*/
		case 'V':/*VBR*/
			printk("argc is : %d\r\n",argc);
			printk("scr is : %d\r\n",scr);
			printk("wrrbase is :%d\r\n",WRRBASE);
			if(scr == 0){ //to prevent scr = 0
				scr = 1;
				HP_TSW = (WRRBASE/scr);
				LP_TSW = (WRRBASE/scr);
				scr = 0;
			}
			else{
				HP_TSW = (WRRBASE/scr);
				LP_TSW = (WRRBASE/scr);
			}
			
			if( HP_TSW > MAX_TSW ){
				HP_TSW = MAX_TSW;
				LP_TSW = MAX_TSW;
			}
			TSARM_TRAFFIC_SHAPER_WEIGHT(vc) =((LP_TSW & 0xfff) << 16) | ((HP_TSW & 0xfff) << 0);
			mt7510AtmRateCalCulate((uint16)treg_tstbr, (uint16)pcr, &pcr_dec, &pcr_init);
			mt7510AtmRateCalCulate((uint16)treg_tstbr, (uint16)scr, &scr_dec, &scr_init);
			if( pcr == 0 ){
	                        printk("PCR is ZERO!!\r\n");
	                        return 0;
	                }
	
			cal_mbs=mt7510AtmMbsCalCulate(pcr,scr,mbs);
			if(scr == 0){
				scr_dec = 0;
				scr_init = 1;
			}
			TSARM_PCR(vc) = (pcr_dec << 16) | pcr_init;
			TSARM_SCR(vc) = (scr_dec << 16) | scr_init;
			if(argv2=='Q'){/*UBR+*/
				TSARM_MBSTP(vc) = ((clp1 & 0x3) << 28) | ((clp0 & 0x3) << 26) | (mbs << 2) | TSARM_QOS_UBR;
			}
			else{/*VBR*/
				TSARM_MBSTP(vc) = ((clp1 & 0x3) << 28) | ((clp0 & 0x3) << 26) | (cal_mbs << 2) | TSARM_QOS_rtVBR;
			}
			break;
		case 'G':/*GFR*/
			LP_TSW = (WRRBASE/pcr );
			if(scr == 0){ //to prevent scr = 0
				scr = 1;
				HP_TSW = (WRRBASE/scr);
				scr = 0;
			}
			else{  
				HP_TSW = (WRRBASE/scr);
			}

			if( HP_TSW>MAX_TSW){
				HP_TSW = MAX_TSW;
			}

			if( LP_TSW > MAX_TSW){
				LP_TSW = MAX_TSW;
			}
			
			/*mfs means maximum frame size, and ethernet maximum frame size was 1514, so it set to 35 (35*48)*/
			TSARM_MAX_FRAME_SIZE(vc) = ATM_QOS_GFR_MFS;
			TSARM_TRAFFIC_SHAPER_WEIGHT(vc) =((LP_TSW & 0xfff) << 16) | ((HP_TSW & 0xfff) << 0);
		
			mt7510AtmRateCalCulate((uint16)treg_tstbr, (uint16)pcr, &pcr_dec, &pcr_init);
			mt7510AtmRateCalCulate((uint16)treg_tstbr, (uint16)scr, &scr_dec, &scr_init);
			if( pcr == 0 ){
				printk("PCR is ZERO!!\r\n");
				return 0;
			}
			cal_mbs=mt7510AtmMbsCalCulate(pcr,scr,mbs);
			if(scr == 0){
				scr_dec = 0;
				scr_init = 1;
			}
		
			if(scr == pcr){
				/*
				**ATM QoS GFR Type, it will be accroding the mbs size to  send atm cell with high priority or not.
				**GFQ Type is inclued with packet concept, if the capacity(mbs) is more than 1 ethernet packet size
				**, it will be sent by high priority, otherwise it always send by lower priortiy.Maximum ethernet frame
				**is 1518 bytes, 1518%48=32, so the mbs must be large than 32, the etherent packet could be use
				** high priority to send.
				*/
				cal_mbs +=  50;
			}
			TSARM_PCR(vc) = (pcr_dec << 16) | pcr_init;
			TSARM_SCR(vc) = (scr_dec << 16) | scr_init;
			TSARM_MBSTP(vc) = ((clp1 & 0x3) << 28) | ((clp0 & 0x3) << 26) | (cal_mbs << 2) | TSARM_QOS_GFR;
			break;
		default:
			break;
	}
	printk("\r\n vc:%d", vc);
	printk("\r\n TSARM_PCR:0x%08lx", (uint32)TSARM_PCR(vc));
	printk("\r\n TSARM_SCR:0x%08lx", (uint32)TSARM_SCR(vc));
	printk("\r\n TSARM_MBSTP:0x%08lx", (uint32)TSARM_MBSTP(vc));
	if(argv2 == 'G')
		printk("\r\n TSARM_MAX_FRAME_SIZE:0x%08lx\r\n", (uint32)TSARM_MAX_FRAME_SIZE(vc));
	return 0;
}

/*_____________________________________________________________________________
**      function name: tsarmReset
**      descriptions:
**           Software reset hardware SAR modules.
**             
**      parameters:
**           None
**             
**      global:
**            None
**             
**      return:
**            None
**	     
**      call:
**      	delay1ms
**      	RAI_RESET_ENB
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
void
tsarmReset(void)
{
	delay1ms(50);
	TSARM_RAI = RAI_RESET_ENB(1);
	delay1ms(5);
	TSARM_RAI = RAI_RESET_ENB(0);
}/*end tsarmReset*/

/*_____________________________________________________________________________
**      function name: tsarmRegDefCheck
**      descriptions:
**          Read the value of SAR register and to compare the default value is fit in with data 
**         sheet. 
**             
**      parameters:
**           None
**             
**      global:
**            sar_reg
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**           None
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
int
tsarmRegDefCheck(void)
{
	int i=0,j=0;
	uint32 reg_addr=0;
	uint8 loop=0;
	int retval=0;

	for(i=0; sar_reg[i].name!=NULL; i++){
		reg_addr = sar_reg[i].addr;
		if(sar_reg[i].type & NO_DEF){
			/*If the register is no default value, we skip this register.*/
			continue;
		}
		else if(sar_reg[i].type & VC_TYPE){
			loop=ATM_VC_MAX;
		}
		else{
			loop=1;
		}

		for(j=0; j<loop; j++){
			if(VPint(reg_addr + (j << 2)) != sar_reg[i].def_value){
				printk("Error(j:%d):%s is error, Default: 0x%08lx Real:0x%08lx\n", j, sar_reg[i].name, sar_reg[i].def_value, (uint32) VPint(reg_addr));
				retval=-1;
			}
		}
	}
	return retval;
}/*end tsarmRegDefCheck*/

/*_____________________________________________________________________________
**      function name: tsarmRegDefCheck
**      descriptions:
**         SAR registers read/write test.
**         Steps as flows:
**         1. Write the test pattern into SAR register. 
**         2. Read the value of SAR register. 
**         3. Compare the value of SAR register is fit in with test pattern. 
**             
**      parameters:
**           pattern: Test pattern.
**             
**      global:
**            sar_reg
**             
**      return:
**            Success:        0
**            Otherwise:     -1
**	     
**      call:
**           delay1ms
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
int
tsarmRegRWTest(uint32 pattern)
{
	int i=0, j=0;
	uint32 befVal=0;
	uint8 loop=0;
	uint32 reg_addr=0x0;
	int retval=0;
	int err=0;
	
	for(i=1; sar_reg[i].name!=NULL; i++){/*Skip Software reset register(TSARM_RAI)*/
		if(sar_reg[i].type & VC_TYPE){
			delay1ms(100);
			loop=ATM_VC_MAX;
		}else{
			loop=1;
		}
		
		for(j=0; j<loop; j++){
			err=0;
			reg_addr= sar_reg[i].addr+(j<<2);
			befVal= (uint32)VPint(reg_addr);
		
			if(sar_reg[i].type & RO){
				VPint(reg_addr)=~(VPint(reg_addr));
				if(VPint(reg_addr) != befVal){
					retval=-1;
					err=1;
				}
			}
			else if(sar_reg[i].type & WO){
				VPint(reg_addr) = pattern;
				if(VPint(reg_addr)!=0x00000000){
					retval=-1;
					err=1;
				}
			}
			else if(sar_reg[i].type & RW){
				VPint(reg_addr)=pattern;
				if((VPint(reg_addr) & sar_reg[i].mask) != (pattern & sar_reg[i].mask)){
					retval=-1;
					err=1;
				}
			}
			
			if(err==1){
				printk("Error(j:%d): %s is error, Pattern: 0x%08lx before:0x%08lx After:0x%08lx\n", \
					j, sar_reg[i].name, (pattern & sar_reg[i].mask), befVal, (uint32) VPint(reg_addr));	
			}
		}
	}
	return retval;
}/*end tsarmRegRWTest*/

/*_____________________________________________________________________________
**      function name: sarLpbkInit
**      descriptions:
**         Reset  sar driver and enable local loop back functionality. 
**             
**      parameters:
**           None
**             
**      global:
**            sar_reg
**             
**      return:
**           None
**	     
**      call:
**           atmReset
**           delay1ms
**      
**      revision:
**      1. Here 2008/08/15
**____________________________________________________________________________
*/
void
sarLpbkInit(void){

	int val;

//	atmReset();
	/*Set General Configuration Register to enable Local loop functionality.*/
//	delay1ms(500);
	TSARM_GFR =  0;
	delay1ms(2000);
	TSARM_GFR |=  GFR_LOCAL_LPK;
	delay1ms(2000);
	TSARM_GFR |= GFR_TXENB | GFR_RXENB ;
	delay1ms(2000);
}/*end sarLpbkInit*/

/*_____________________________________________________________________________
**      function name: dumpCell
**      descriptions:
**         Dump the atm cell content information. 
**             
**      parameters:
**           src: Start address of atm cell content.
**           len: Length of atm cell.
**             
**      global:
**            None
**             
**      return:
**           None
**	     
**      call:
**           None
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
void 
dumpCell(uint8* src, int len){
	int i=0;

	printk("\r\nCell content(len:%d) as flow:\r\n", len);
	for(i=0; i<len; i++){
		if((i&7)==0){
			printk("\r\n");
		}
		printk(" %02x", src[i]);
	}
	printk("\r\n");
}/*end dumpCell*/

/*_____________________________________________________________________________
**      function name: atmAal5DataReqVerify
**      descriptions:
**        Send ATM Data cell function that is copy from atmAal5RealDataReq function.
**             
**      parameters:
**           skb: Start address of atm cell content.
**           vc: Specify the vc number (0~9)
**           priority: Specify the priority queue(0~3)
**             
**      global:
**            atmTxQueSize
**            atmTxVcCnt
**            atmTxPriVcCnt
**            atm_p
**             
**      return:
**           0: Sucess
**           1: Failure 
**	     
**      call:
**           atmTxDescrFree
**           atmDataReq
**           K1_TO_K0
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
uint8
atmAal5DataReqVerify(struct sk_buff *skb,uint8 vc,uint8 priority)
{
	uint8 atmTxVcDescrNummax;
	
	atmTxVcDescrNummax = atmTxQueSize[0] + atmTxQueSize[1] + atmTxQueSize[2] + atmTxQueSize[3];

#ifdef CONFIG_TX_POLLING_BY_MAC
	if (atmTxVcCnt[vc] >= 4){
		qdma_txdscp_recycle(0);
	}
#endif

#ifdef CONFIG_TX_POLLING_BY_MAC
	if (atmTxPriVcCnt[priority][vc] >= 16){
		qdma_txdscp_recycle(0);
	}
#endif

	if (atmTxVcCnt[vc] >= atmTxVcDescrNummax || atmTxPriVcCnt[priority][vc] >= 16){ 
		if(sarVerifyDbg){
			printk("atmTxVcCnt >atmTxVcDescrNummax occurs to free skb\r\n");
		}

#ifdef CONFIG_TX_POLLING_BY_MAC
		qdma_txdscp_recycle(0);
#endif
		atm_p->MIB_II.outSoftwareDiscards++;
		atm_p->MIB_II.outSoftDropVcNum[vc]++;
		atm_p->MIB_II.outDiscards++;
		dev_kfree_skb_any(skb); 
		return 1;
	}

	atmDataReq(skb, vc, priority);
	return 0;
}/*end atmAal5DataReqVerify*/

#if 1
/*______________________________________________________________________________
**  atmOamF4F5DataReqVerify
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
atmOamF4F5DataReqVerify(
	uint8 vpi,
	uint16 vci,
	uint8 f5,
	uint8 endToEnd,
	uint8 funcType
)
{
	atmOamCell_t *oamCellp=NULL;
	atmOamCellPayload_t *oamCellPayloadp;
	uint8 vc;
	uint8 pti;
	uint8 tmpCell[52];/*Testing cell*/
	uint8 tx_seed=0;
	int i=0;
	
	atmCell_t atmCellBuf;

	memset(&atmCellBuf, 0, sizeof(atmCell_t));

	if ( f5 ) { /* F5: by PTI */
		/* Move to non-cacheable region */
		switch ( funcType ) {
		case 0: /* AIS */
			oamCellPayloadp = (atmOamCellPayload_t *)(oamF5AisCell);
			break;
		case 1: /* RDI */
			oamCellPayloadp = (atmOamCellPayload_t *)(oamF5RdiCell);
			break;
		case 2: /* Loopback */
			oamCellPayloadp = (atmOamCellPayload_t *)(oamF5LoopBackReqCell);
			break;
		case 3: /* Continuity check */
			oamCellPayloadp = (atmOamCellPayload_t *)(oamF5ContinuityCheckReqCell);
			break;
		default:
			return 1;
		}
		vc = atmVcNumberGet(vpi, vci);
		if (  vc == ATM_DUMMY_VC ) {
			/* This is not a opened VC */
			return 1;
		}
		if ( endToEnd ) {
			pti = 5;
		}
		else {
			pti = 4;
		}

		memcpy(atmCellBuf.word, (uint8 *)oamCellPayloadp, 48);
		if(!atmOamDataReq((uint8 *)atmCellBuf.word, pti, vc)){
			atm_p->MIB_II.outF5Pkts++;
			atm_p->MIB_II.outPkts++;
		} else {
			atm_p->MIB_II.outDiscards++;
			printk("atmOamDataReq fail\r\n");
		}
	}
	else { /* F4: by VCI */
		memset(tmpCell, 0, 52);
		tx_seed=(uint8)random32();
		tmpCell[4]=tx_seed;
		
		oamCellp=(atmOamCell_t *)(tmpCell);
		/*Modify VCI value with CC cell header*/
		if ( endToEnd ) {
			oamCellp->vci = 4;
		}
		else {
			oamCellp->vci = 3;
		}
		/*Test CC Cell filter*/
		if(test_flag & CC_CONT_FG){
			oamCellp->vpi=cc_cont.vpi;
			oamCellp->vci=cc_cont.vci;
			oamCellp->pti=cc_cont.pti;
			oamCellp->clp=cc_cont.clp;
		}
		/*end cce cell*/
		/*Fill the cc cell content*/
		for(i=0;i<45;i++){
			oamCellp->payload[i]=(uint8) (tx_seed++ & 0xff);
		}
		
		memcpy(atmCellBuf.word, (uint8 *)oamCellp, 52);
		if(!atmCcDataReq((uint8 *)atmCellBuf.word)){
			atm_p->MIB_II.outF4Pkts++;
			atm_p->MIB_II.outPkts++;
		}
		else {
			atm_p->MIB_II.outDiscards++;
			printk("atmCcDataReq fail\r\n");
		}
	}
	return 0;
}
#endif

/*_____________________________________________________________________________
**      function name: openAtmVc
**      descriptions:
**         Open a new VC.
**             
**      parameters:
**           vc: The number of VC.
**           vpi: VC's vpi.
**           vci: VC's vci.
**             
**      global:
**            atmCfgTable
**             
**      return:
**           None
**	     
**      call:
**           None
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
void 
openAtmVc(uint8 vc,uint8 vpi, uint16 vci, uint8 phy)
{	
	TSARM_VCCR(vc) = VCCFGR_VPI(vpi) | VCCFGR_VCI(vci) |
			VCCFGR_PORT(phy) | VCCFGR_VALID;

	if(test_flag & RAW_CELL_FG){
		TSARM_VCCR(vc) |= VCCFGR_RXRAW;
	}
	
	/* below three qos parameters must be set or it wouln't work.*/
	TSARM_PCR(vc) = 0x00010010;
	TSARM_SCR(vc) = 0x00010010;
	TSARM_MBSTP(vc) = 0x00000004;

	TSARM_RXDBCSR |= 1 << vc;
	TSARM_RXMBCSR |= 1 <<vc;
	TSARM_RXMBCSR |= 1 << 16;

	atmCfgTable.openFlag[vc]=1;
	atmCfgTable.vpi[vc] = vpi;
	atmCfgTable.vci[vc] = vci;
	atmCfgTable.vcNumber++;
}/*end openAtmVc*/

/*_____________________________________________________________________________
**      function name: allCellTest
**      descriptions:
**         Random to open VCs and 
**             
**      parameters:
**           delay: Delay a time to send atm cell
**           loopTimes: Loop time for all VCs test,0:loop forever.
**             
**      global:
**            atmCfgTable
**             
**      return:
**           None
**	     
**      call:
**          rand
**          patternGen
**          atmAal5DataReqVerify
**          delay1ms
**          atmOamF4F5DataReq
**          atmAal5VcClose
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
void 
allCellTest(int delay, int loopTimes){
	struct sk_buff* skb=NULL;
	uint8 vc=0;
	uint8 vpi=0;
	uint16 vci=0;
	int nPriQue[ATM_TX_PRIORITY_MAX];
	uint8 i=0;
	int j=0;
	int k=0;
	uint32 seq=0;
	uint32 flags=0;
	uint8 e2e[2];/*0:CC, 1:OAM*/
	uint8 type[2];/*0:CC, 1:OAM*/
	uint8 nMCell[2];/*0:CC, 1:OAM*/
	uint32 count=0;
	int wait_times;//prevent loop forever. shnwind add.
	atomic_set(&sarRxMDone, 1);
	atomic_set(&sarRxDDone, 1);
	
	if(test_flag & MPOA_ON_FG){
		TSARM_VC_MPOA_CTRL(0) = TSARM_VC_MPOA_CTRL(5) = 0x0389;/*RFC1483-R LLC*/
		TSARM_VC_MPOA_CTRL(1) = TSARM_VC_MPOA_CTRL(6) = 0x0365;/*RFC1483-B LLC*/
		TSARM_VC_MPOA_CTRL(2) = TSARM_VC_MPOA_CTRL(7) = 0x03f0;/*PPPoA LLC*/
		TSARM_VC_MPOA_CTRL(3) = TSARM_VC_MPOA_CTRL(8) = 0x0340;/*RFC1483-B VC*/
		TSARM_VC_MPOA_CTRL(4) = TSARM_VC_MPOA_CTRL(9) = 0x0;
	}
	
	/*Qos Parameters*/
	TSARM_TXSLRC= (1 << 16) | 1;
	/*end qos parameters*/
	
	while(stop_loop==0){
		for(i=0; i<ATM_TX_PRIORITY_MAX; i++){
			if (test_traffic == BURST_TRAFFIC){
				// Burst Traffic
				nPriQue[i] = (rand() & 0xf);
			} else {
				// Sync Traffic
				nPriQue[i] = 0xff; 
			}
		}
		#if 1
//		vc = (rand() & 0xf) % ATM_VC_MAX;
		vc =  ATM_VC_MAX;
//		vc =  1;
		vci = (rand()&0xfff0);
		for(i=0; i<vc; i++){
			vpi = (rand()&0xff);
			/*
			**If vci is equal with 3 or 4 that is caused rx side compare data cell error. 
			**Beacuse the cc cell will be received on atm data channel.
			*/
			if(vci < TEST_MIN_VCI){
				vci+=TEST_MIN_VCI;
			}
			printk("vc:%02x vpi:%02x vci:%02x\r\n", i, vpi, vci+i);
			openAtmVc(i, vpi,vci+i, VCCFGR_ATM_PHY0);
			TSARM_MBSTP(i) = (3 << 28) | (0 << 26) | (1 << 2) | TSARM_QOS_CBR;
			TSARM_PCR(i) = (1 << 16) | 1;
			TSARM_SCR(i) = (1 << 16) | 1;

			/*end Set QoS Parameters*/
		}
		#endif
// frank mark 
#if 1
		for(i=0; i<vc; i++){/*Loop for VC*/
			/*Data Cell*/
			#if 1
			for(j=0; j<ATM_TX_PRIORITY_MAX; j++){ /*Loop for 4 priority queue*/
				for(k=0; k<nPriQue[j]; k++){/*Loop for send ATM data cell*/
					skb=patternGen(seq, (0x0a+k));
					if(skb){
						if (test_traffic == SYNC_TRAFFIC){
		                    wait_times = 0;
							while(1){
								if(atomic_read(&sarRxDDone)==1){
									atomic_set(&sarRxDDone, 0);
									break;
								}
								if(wait_times == 1000){
									printk("no receive data packet\n");
									goto end;
								}
                		        wait_times++;
								mdelay(1);
							}
						}
						spin_lock_irqsave(&tsarm_lock, flags);
						if(atmAal5DataReqVerify(skb, i, j)!=0){
							printk("Fail:atmAal5DataReqVerify\r\n");
							spin_unlock_irqrestore(&tsarm_lock, flags);
							continue;
							#if 0
							goto end;
							#endif
						}
						spin_unlock_irqrestore(&tsarm_lock, flags);
						if(delay){
							mdelay(delay);
						}
					}
					else{
						printk("Allocate skb failure!\r\n");
						goto end;
					}

					#if 1
					/*Go out loop*/
					if(stop_loop!=0){
						goto end;
					}
					#endif
				}/*end for(k=0; k<nPriQ...*/
			}/*end for(j=0; j<ATM_TX_...*/
			#endif

			#if 1
			/*OAM/CC Cell*/
			for(j=0; j<2; j++){
				e2e[j] = ((rand()&0x03)%2);
				type[j] = (rand()&0x03);
				nMCell[j]=(rand() & 0x3);
				for(k=0; k<nMCell[j]; k++){
					/*
					**CC/OAM Cell must be wait for rx recevied the cell then to start send cc/oam cell.
					**Because the manager cell size is too small, it's will cause IRQ queue full situation.
					*/
					if (test_traffic == SYNC_TRAFFIC){
	                    wait_times = 0;
						while(1){
							if(atomic_read(&sarRxMDone)==1){
								atomic_set(&sarRxMDone, 0);
								break;
							}
							if(wait_times == 1000){
								printk("no receive oam packet\n");
								goto end;
							}
                	        wait_times++;
							mdelay(1);
						}
					}

					spin_lock_irqsave(&tsarm_lock, flags);
					if(atmOamF4F5DataReqVerify(atmCfgTable.vpi[i], atmCfgTable.vci[i], j, e2e[j], type[j])==1){
						if(j==0){
							printk("Send CC cell error\r\n");
						}
						else{
							printk("Send OAM cell error\r\n");
						}
					}
					spin_unlock_irqrestore(&tsarm_lock, flags);
				}/*end for(k=0; k<nMCell[j]...*/
				if(delay){
					mdelay(delay);
				}
			}/*end for(j=0; j<2...*/
			#endif
			seq++;
		}/*end for(i=0; i<vc...*/
#endif
#if 1
		#if 1
		/*Waiting for atm cell is received by rx side*/
		if (test_traffic == BURST_TRAFFIC){
			mdelay(300);

			/*Close the all vcs that is created for test*/
			for(i=0; i<vc; i++){
				atmAal5VcClose(atmCfgTable.vpi[i], atmCfgTable.vci[i]);
			}	
			// wait for qdma retire finish
			mdelay(300);
		}
		#endif
		if(loopTimes>0){
			if(count==loopTimes){
				goto end;
			}
		}

		if(count%60==0){
			printk("\r\n");
		}
		else{
			printk(".");
		}

		count++;
#endif
	}/*end while(stop_loop==0)*/
end:

	#if 1  /*To closed the pvc but fixed the vpi/vci and pvcs number*/
	/*Waiting for atm cell is received by rx side*/
	if ((test_traffic == SYNC_TRAFFIC) || (stop_loop == 1)){
		mdelay(150);
		/*Close the all vcs that is created for test*/
		for(i=0; i<vc; i++){
			atmAal5VcClose(atmCfgTable.vpi[i], atmCfgTable.vci[i]);
		}
		// wait for qdma retire finish
		mdelay(300);
	}
	#endif

	printk("\r\nstop_loop:%d count:0x%08lx\r\n",stop_loop, count);	
	atmCounterDisplay();

}/*end allCellTest*/
/*_____________________________________________________________________________
**      function name: cmpContent
**      descriptions:
**        	Compare the data content.
**             
**      parameters:
**           src: Source cell content start address.
**           target: Target cell content start address.
**           len: Specify the compare length. 
**             
**      global:
**          None
**             
**      return:
**          None
**	     
**      call:
**           dumpCell
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
int 
cmpContent(uint8* src, uint8* target, int len)
{
	int i=0, test_index=0;
	uint8 seed;
	uint8 pattern;

	if(sarVerifyDbg){
		printk("Receive packet content\n");
		dumpCell(src, len);
	}

	/*Get the seed value from header*/
	seed = *(src + OFFSET_LEN + SEQ_LEN + PATTERN_LEN);

	switch(seed){/*Loopback Pattern Type*/
		/* PATTERN_FF */
		case 0:
			pattern = 0xff;
			break;
		/* PATTERN_00 */
		case 1:
			pattern = 0x00;
			break;
		/* PATTERN_5a */
		case 2:
			pattern = 0x5a;
			break;
		/* PATTERN_a5 */
		case 3:
			pattern = 0xa5;
			break;
		default:
			printk("Error Data Pattern\r\n");
			return -1;
	}
	
	if (test_flag & VLAN_TAG_FG){
		test_index = 16; 
	} else {
		test_index = TOTAL_PAT_LEN;
	}

	for(i=test_index; i<len; i++){
		if(src[i] != pattern){
			printk("Mismatch,index:%d src:%2x target:%2x\n", i, src[i], pattern);
			dumpCell(src, len);
			stop_loop=1;
			return -1;
		}
	}
	return 0;
}/*end cmpContent*/

int 
cmpVlanTag(uint8* src, uint8* target, int len)
{
	int i=0;

	for(i=0; i<len; i++){
		if(src[i] != target[i]){
			printk("Vlan Tag Mismatch\n");
			printk("Packet Vlan Tag: %x%x%x%x\n", src[0], src[1], src[2], src[3]);
			printk("Real  Vlan Tag: %x%x%x%x\n", target[0], target[1], target[2], target[3]);
			stop_loop=1;
			return -1;
		}
	}

	return 0;
}/*end cmpContent*/


int
oamCellCheck(uint8* src, uint8* target, int len)
{
	int i=0;
//	for(i=0; i<len; i++){
	for(i=0; i<(len-2); i++){
		if(src[i] != target[i]){
			printk("Mismatch,i:%d src:%2x target:%2x\n", i, src[i], target[i]);
			dumpCell(src, len);
			stop_loop=1;
			return -1;
		}
	}
	return 0;
}/*end oamCellCheck */


/*_____________________________________________________________________________
**      function name: cmpAtmCell
**      descriptions:
**        	Compare the ATM cell content is fit with in transmit cell content.
**             
**      parameters:
**           type:1: DATA_CELL
**          		2: OAM_CELL
**          		3: CC_CELL
**           src: Recevied ATM cell content start address.
**           len: Length of ATM Cell size
**             
**      global:
**            test_flag
**            stop_loop
**            dataCell
**            vlanTag
**            oamF5RdiCell
**            oamRdiCell
**            oamF5LoopBackReqCell
**            oamLoopBackReqCell
**            oamF5ContinuityCheckReqCell
**            oamContinuityCheckReqCell
**             
**      return:
**          None
**	     
**      call:
**           patternCheck
**           dumpCell
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
void
cmpAtmCell(uint8 type, uint8* src, int len){
	uint8* target=NULL;
	uint8 key=0;
	if(sarVerifyDbg){
		printk("ATM Cell(type:%x) addr:0x%08lx\r\n", type, (uint32)src);
	}
	
	if(type & DATA_CELL){/*Check the data cell content*/
		/*Check the data cell content with random pattern*/
		if(test_flag & RAND_DCELL_FG){
			if(patternCheck(src, len)!=0){
				printk("Random ATM Data Cell is error\r\n");
				stop_loop=1;
			}
			else{
				if(sarVerifyDbg)
					printk("Random ATM Data Cell is match\r\n");
			}
		} 

		/*Check the data cell content with fix pattern*/
		if (test_flag & FIX_DCELL_FG){
			if(cmpContent(src, dataCell, len)==0){
				if(sarVerifyDbg){ 
					printk("Fixed ATM Data Cell Payload is match\n");
				}
			}
			else{
				printk("Fixed ATM Data Cell Payload is error\r\n");
				#ifdef TCSUPPORT_AUTOBENCH
				autobench_sar_lpbk_flag = 1;
				#endif
			}			
		}

		/*check vlan flag*/
		if (test_flag & VLAN_TAG_FG){
			if(cmpVlanTag(&src[VLAN_INDEX], vlanTag, VLAN_LEN)==0){
				printk("VLAN Tag is match\n");
			}
		}
		
		if((test_flag & HW_VLAN_UNTAG_FG) || (test_flag & HW_MPOA_FG)){
			dumpCell(src, len);
		}
	}
	else{/*Check the manager cell content*/
		/* frank add for test */
//		printk("dump oam/cc cell content\n");
//		dumpCell(src, len);
		if(type & CC_CELL){
			if(ccCellCheck(src, CC_CELL_SIZE)==0){
				if(sarVerifyDbg)
					printk("CC Cell is match\r\n");
			}
			else{
				printk("CC Cell is mis-match\n");
			}
		}
		else{
			/*OAM Cell*/
			switch(src[0]){
				case AIS_TYPE:
					target=oamF5AisCell;
					break;
				case RDI_TYPE:
					target=oamF5RdiCell;
					break;
				case LPBKREQ_TYPE:
					target=oamF5LoopBackReqCell;
					break;
				case CONTCHKREQ_TYPE:
					target=oamF5ContinuityCheckReqCell;
					break;
				default:
					printk("MCell is mismatch:key=%2x\r\n", key);
					stop_loop=1;
					break;
			}
		
			if(target!=NULL){
				if(oamCellCheck(src, target, len)==0){
					if(sarVerifyDbg)
						printk("Oam Cell is match\n");
				}
				else{
					printk("Oam Cell is mis-match\n");
				}
			}
		}
	}
}/*end cmpAtmCell*/

/*_____________________________________________________________________________
**      function name: atmDataInd
**      descriptions:
**        	If the ax4k loop back test is enabled, cpe was received the atm cell then it'll be 
**        	replaied to ax4k directly.
**             
**      parameters:
**           skb: Specify the socket buffer that you want to replay to ax4k.
**           vc: The number of vc.
**           len: Length of data content.
**             
**      global:
**            atm_p
**            atmCfgTable
**            ax4k_lpbk
**             
**      return:
**         	Socket buffer address
**	     
**      call:
**           mt7510_atm_verify_send
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
struct sk_buff  *
atmDataInd(
	struct sk_buff *skb,
	uint8 vc,
	uint32 len
)
{
	struct sk_buff *freeSkb = NULL;
	struct atm_vcc *vcc;
	
	atm_p->MIB_II.inPkts++;
	atm_p->MIB_II.inDataPkts++;
	freeSkb=skbmgr_dev_alloc_skb2k();
	if (freeSkb){
		dma_cache_inv((unsigned long)freeSkb->data, RX_BUF_LEN);
		vcc = atmCfgTable.vcc[vc];
		if (unlikely(vcc == NULL)) {
			dev_kfree_skb_any(skb);
			atm_p->MIB_II.inDiscards++;
			return freeSkb;
		}
		ATM_SKB(skb)->vcc = vcc;
		skb_put(skb, len);
		mt7510_atm_verify_send(vcc, skb);
	}
	else {
		if(ax4k_lpbk == AX4K_LBPK_EN){
			atm_p->MIB_II.inDiscards++;
		}
		freeSkb = skb;
	}

	return freeSkb;
}/*end atmDataInd*/
/*_____________________________________________________________________________
**      function name: atmCcLpbkHandler
**      descriptions:
**        	If the ax4k loop back test is enabled, cpe was received the atm CC cell then it'll be 
**        	replaied to ax4k directly, otherwise to check cc cell content is fit with transmit side.
**             
**      parameters:
**           cellp: Atm cc cell content start address.
**             
**      global:
**            ax4k_lpbk
**            atm_p
**             
**      return:
**         	None
**	     
**      call:
**           CACHE_TO_NONCACHE
**           atmCcDataReq
**           cmpAtmCell
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
void
atmCcLpbkHandler(atmOamCell_t *oamCellp){
	uint8 isF4 = 0;

	struct sk_buff *skb = NULL;
	atomic_set(&sarRxMDone, 1);

	if((test_flag&CC_CELL_FG)  || (ax4k_lpbk == AX4K_LBPK_EN)){
		if(ax4k_lpbk == AX4K_LBPK_EN){
			if((oamCellp->vci == 3) || (oamCellp->vci == 4)){
				atm_p->MIB_II.inF4Pkts++;
				atm_p->MIB_II.inPkts++;
				isF4 = 1;
			}
			if(!atmCcDataReq((uint8 *)oamCellp)){
				if(isF4){
					atm_p->MIB_II.outF4Pkts++;
				}
				atm_p->MIB_II.outPkts++;
			}
		}
		else{
			atm_p->MIB_II.inF4Pkts++;
			atm_p->MIB_II.inPkts++;
			/*To check tasklet is run or not, if it's not run we wake up it.*/
			if(tasklet_flag==0){
				tasklet_schedule(&tsarm_tasklet);
			}
			/*We must to limit the skb enqueue size, otherwise the system will not enough memory space*/
			if(skb_queue_len(&rx_queue) < MAX_SKB_Q_SIZE){
				skb=dev_alloc_skb(MIN_SKB_SIZE);
				if(skb){
					dma_cache_inv((unsigned long)(skb)->data, MIN_SKB_SIZE);
					skb->cb[0]=CC_CELL;
					memcpy(skb->data, (uint8 *)oamCellp, CC_CELL_SIZE);
					skb_put(skb,CC_CELL_SIZE);
					skb_queue_tail(&rx_queue, skb);
				}
				else{
					stop_loop=1;
					printk("dev_alloc_skb alloc fail\r\n");
				}
			}
			else{
				atm_p->MIB_II.inDiscards++;
			}

		}
	}
	else{/*Orginal Path*/
		atmCcHandler(oamCellp);
	}
}/*end atmCcLpbkHandler*/

/*_____________________________________________________________________________
**      function name: atmOamLpbkHandler
**      descriptions:
**        	If the ax4k loop back test is enabled, cpe was received the atm OAM cell then it'll be 
**        	replaied to ax4k directly, otherwise to check oam cell content is fit with transmit side.
**             
**      parameters:
**           atmRxCcDescrp: Atm cc cell content start address.
**           vc: Specify the vc channel
**             
**      global:
**            ax4k_lpbk
**            atm_p
**             
**      return:
**         	None
**	     
**      call:
**           CACHE_TO_NONCACHE
**           atmOamDataReq
**           cmpAtmCell
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
void
atmOamLpbkHandler(atmOamCellPayload_t * oamCellPayloadp, uint8 pti, uint8 vc){
	uint8 isF4 = 0;
	struct sk_buff *skb = NULL;
	atomic_set(&sarRxMDone, 1);

	if(test_flag&OAM_CELL_FG || (ax4k_lpbk == AX4K_LBPK_EN)){
		
		if(ax4k_lpbk == AX4K_LBPK_EN){
			if((pti == 4) || (pti == 5)){
				atm_p->MIB_II.inF5Pkts++;
				atm_p->MIB_II.inPkts++;
				isF4 = 0;
			}
			if(!atmOamDataReq((uint8 *)oamCellPayloadp, pti, vc)) {
				if(!isF4){
					atm_p->MIB_II.outF5Pkts++;
				}
				atm_p->MIB_II.outPkts++;
			}
		}
		else{
			atm_p->MIB_II.inF5Pkts++;
			atm_p->MIB_II.inPkts++;
			/*To check tasklet is run or not, if it's not run we wake up it.*/
			if(tasklet_flag==0){
				tasklet_schedule(&tsarm_tasklet);
			}
			/*We must to limit the skb enqueue size, otherwise the system will not enough memory space*/
			if(skb_queue_len(&rx_queue) < MAX_SKB_Q_SIZE){
				skb=dev_alloc_skb(MIN_SKB_SIZE);
				if(skb){
					dma_cache_inv((unsigned long)(skb)->data, MIN_SKB_SIZE);
					memcpy(skb->data, (uint8*)oamCellPayloadp, OAM_CELL_SIZE);
					skb_put(skb,OAM_CELL_SIZE);
					skb->cb[0]=OAM_CELL;
					skb_queue_tail(&rx_queue, skb);		
				}
				else{
					printk("dev_alloc_skb alloc fail\r\n");
				}
			}
			else{
				atm_p->MIB_II.inDiscards++;
			}
		}
	}
	else{/*Orginal Path*/
		atmOamHandler(oamCellPayloadp, pti, vc);
	}
}/*end atmOamLpbkHandler*/

/*_____________________________________________________________________________
**      function name: atmDataLpbkHandler
**      descriptions:
**        	If the ax4k loop back test is enabled, cpe was received the atm OAM cell then it'll be 
**        	replaied to ax4k directly, otherwise to check oam cell content is fit with transmit side.
**             
**      parameters:
**           atmRxCcDescrp: Atm cc cell content start address.
**           vc: Specify the vc channel
**             
**      global:
**            ax4k_lpbk
**            atm_p
**             
**      return:
**         	None
**	     
**      call:
**           CACHE_TO_NONCACHE
**           atmOamDataReq
**           cmpAtmCell
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
struct sk_buff *
atmDataLpbkHandler(struct sk_buff *skb, uint8 vc, uint32 len, atmRxMsgBuf_t *pMsg){
	struct sk_buff *freeSkb = NULL;
	unsigned int offset;

	atomic_set(&sarRxDDone, 1);

	if(ax4k_lpbk==AX4K_LBPK_EN){
		skb = atmDataInd(skb, vc, len);/*driver loopback for data cell*/
	}
	else{
		/* Pass to upper layer */					
		if (len && (len <= RX_BUF_LEN)){
			if (test_flag==0 || test_flag & HW_WRR_RULE_FG ){/*Orginal path*/

				if (sarVerifyDbg){		
					dumpCell(skb->data, len);
				}

				skb = atmAal5DataInd(skb, vc, len, pMsg);
			}
			else{
				/*To check tasklet is run or not, if it's not run we wake up it.*/
				if(tasklet_flag==0){
					tasklet_schedule(&tsarm_tasklet);
				}
				/*We must to limit the skb enqueue size, otherwise the system will not enough memory space*/
				if(skb_queue_len(&rx_queue) < MAX_SKB_Q_SIZE){
					atm_p->MIB_II.inPkts++;
					atm_p->MIB_II.inDataPkts++;
					skb_put(skb, len);
					skb->cb[0]=DATA_CELL;
					skb_queue_tail(&rx_queue, skb);
					freeSkb = dev_alloc_skb(RX_BUF_LEN);
					if(freeSkb){
						dma_cache_inv((unsigned long)freeSkb->data, RX_BUF_LEN);
						skb=freeSkb;/*Zero copy*/
						//frank add 20120828 shift to 4-byte alignment
						offset = ((unsigned int)(skb->tail) & 3);
						if(offset)
							skb_reserve(skb, (4 - offset));
					}
					else{
						atm_p->MIB_II.inDiscards++;
					}
				}
			}
		}
	}
	return skb;
}/*end atmDataLpbkHandler*/


/*_____________________________________________________________________________
**      function name: getSkbPriority
**      descriptions:
**        	Use the tos filed of ip header to classfiy skb priority.
**             
**      parameters:
**           tos: Value of tos filed 
**             
**      global:
**            wrrRule
**             
**      return:
**         	Prority queue value.
**	     
**      call:
**           None
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
uint8 
getSkbPriority(uint8 tos){
	uint8 i=0;
	uint8 priority=3;

	for(i=0; i<ATM_TX_PRIORITY_MAX; i++){
		if(tos==wrrRule[i]){
			priority=i;
			break;
		}
	}
	
	if(sarVerifyDbg){
		printk("tos:%02x rule[0]:%02x rule[1]:%02x rule[2]:%02x rule[3]:%02x i:%02x pri:%02x\r\n", \
			tos, wrrRule[0], wrrRule[1],wrrRule[2],wrrRule[3],i,priority);
	}
	return priority;
}/*end getSkbPriority*/

/*_____________________________________________________________________________
**      function name: sarVerifyInit
**      descriptions:
**        	Register SAR verification ci-cmds.
**             
**      parameters:
**           None
**             
**      global:
**           None
**             
**      return:
**         	None
**	     
**      call:
**           cmd_register
**      
**      revision:
**      1. Here 2008/08/24
**____________________________________________________________________________
*/
void 
sarVerifyInit(void)
{
	cmds_t	tsarm_cmd;
	/*Register tsarm ci-cmd*/
	tsarm_cmd.name="tsarm";
	tsarm_cmd.func=doTsarm;
	tsarm_cmd.flags=0x12;
	tsarm_cmd.argcmin=0;
	tsarm_cmd.argc_errmsg=NULL;
	cmd_register(&tsarm_cmd);
#ifdef CONFIG_MIPS_TC3262
	spin_lock_init(&tsarm_lock);
#endif
	tasklet_init(&tsarm_tasklet, cmpCell_tasklet, 0);
	skb_queue_head_init(&rx_queue);
}/*end sarVerifyInit*/
/*_____________________________________________________________________________
**      function name: sarVerifyExit
**      descriptions:
**        	To kill the sar tasklet task.
**             
**      parameters:
**           None
**             
**      global:
**           None
**             
**      return:
**         	None
**	     
**      call:
**           None
**      
**      revision:
**      1. Here 2008/11/15
**____________________________________________________________________________
*/
void 
sarVerifyExit(void){
	tasklet_kill(&tsarm_tasklet);
}/*end sarVerifyExit*/

/*_____________________________________________________________________________
**      function name: isWrrPriorityPkt
**      descriptions:
**          If user active HW_WRR_RULE_FG flag, we need to check the tos of ip header that is fit with 
**          wrr_rule, it's used to enqueue differnet priority queue.This case only work on bridge mode,
**          otherwise it's not work.
**             
**      parameters:
**           skb: socket buffer address
**           priority: The value of priority queue.
**             
**      global:
**           test_flag
**             
**      return:
**         	None
**	     
**      call:
**           getSkbPriority
**      
**      revision:
**      1. Here 2008/09/08
**____________________________________________________________________________
*/
uint8
isWrrPriorityPkt(struct sk_buff* skb,uint8 priority)
{
	/*
	**If user set HW_WRR_RULE_FG, we must got tos of ip header to enqueue 
	**different priority queue according the user settings.This case only work on
	**bridge mode, otherwise it's not work.
	*/
	if(test_flag & HW_WRR_RULE_FG){
		if(skb->len > (IP_TOS_INDEX+1)){
			priority=getSkbPriority(skb->data[IP_TOS_INDEX]);
		}
		/*
		dumpCell(skb->data, skb->len);
		printk("skb->len:%d priority:0x%x\r\n", skb->len, priority);
		*/
	}
	return priority;
}/*end isWrrPriorityPkt*/

/*_____________________________________________________________________________
**      function name: setSarVerifyDataTxMsg
**      descriptions:
**          Set raw/vlan tag of the TX data descriptor.
**             
**      parameters:
**           atmTxDescrp: tx data descriptor
**             
**      global:
**           test_flag
**             
**      return:
**         	None
**	     
**      call:
**           getSkbPriority
**      
**      revision:
**      1. Here 2008/09/08
**____________________________________________________________________________
*/
void 
setSarVerifyDataTxMsg(atmTxMsgBuf_t *pTxMsg)
{
#if 1
	if(test_flag & RAW_CELL_FG){
		pTxMsg->txMsgW0.raw |= TSARM_TX_RAW_EN;	/*Enable HW RAW Cell Func*/
	}
	if(test_flag & VLAN_TAG_FG){
		pTxMsg->txMsgW1.vlanEn = 1;
		pTxMsg->txMsgW1.vlanTag = vlan_id;
	}
#endif
}/*end sarVerifyDataTxMsg*/

/*_____________________________________________________________________________
**      function name: enHwVlanUntag
**      descriptions:
**         If user enable HW_VLAN_UNTAG_FG then nable hardware untag vlan functionality, otherwise 
**         do nothing.
**             
**      parameters:
**           vc: Specify the vc number(0~9)
**             
**      global:
**           test_flag
**             
**      return:
**         	None
**	     
**      call:
**           getSkbPriority
**      
**      revision:
**      1. Here 2008/09/08
**____________________________________________________________________________
*/
void 
enHwVlanUntag(uint32 vc)
{
	if(test_flag & HW_VLAN_UNTAG_FG){
		/*hardware vlan un-tag*/
		TSARM_VC_MPOA_CTRL(vc) |= 0x0700;	
	}
}/*end enHwVlanUntag*/

/*_____________________________________________________________________________
**      function name: dumpDataCell
**      descriptions:
**         If user open the debug flag then to dump data cell information.
**             
**      parameters:
**           skb:	socket buffer address
**           priority: Queue priority
**             
**      global:
**           sarVerifyDbg
**           test_flag
**             
**      return:
**         	None
**	     
**      call:
**           dumpCell
**      
**      revision:
**      1. Here 2008/09/08
**____________________________________________________________________________
*/
void 
dumpDataCell(struct sk_buff* skb, uint8 priority)
{
	if(sarVerifyDbg){
		printk("Tx data Cell......priority:%x len:%08x addr:%08lx\r\n", priority, skb->len, (uint32)skb->data);
		dumpCell(skb->data, skb->len);
	}
}/*end dumpDataCell*/

/*_____________________________________________________________________________
**      function name: dumpMCellAddr
**      descriptions:
**         If user open the debug flag then to dump data cell information.
**             
**      parameters:
**           data_p: Data cell pointer
**           type: OAM_CELL or CC_CELL
**             
**      global:
**           sarVerifyDbg
**           test_flag
**             
**      return:
**         	None
**	     
**      call:
**           None
**      
**      revision:
**      1. Here 2008/09/08
**____________________________________________________________________________
*/
void 
dumpMCellAddr(uint8* data_p, uint8 type)
{
	if(sarVerifyDbg){
		if(type==OAM_CELL){
			printk("Tx OAM cell address:%08lx......\r\n", (uint32)data_p);
		}
		else{
			printk("Tx CC cell address:%08lx......\r\n", (uint32)data_p);
		}
	}
}/*end dumpMCellAddr*/

/*_____________________________________________________________________________
**      function name: mt7510_atm_verify_send
**      descriptions:
**         This function is copy from tc3162_atm_send function.We just want to separate orginal sar driver
**         and verify code independently.
**             
**      parameters:
**           vcc: Atm vcc datasture pionter.
**           skb: Socket buffer address.
**             
**      global:
**         	None
**             
**      return:
**         	None
**	     
**      call:
**           atmAal5DataReq
**      
**      revision:
**      1. Here 2008/09/08
**____________________________________________________________________________
*/
static int 
mt7510_atm_verify_send(struct atm_vcc *vcc, struct sk_buff *skb)
{
	int ret;
	uint32 flags;
	spin_lock_irqsave(&tsarm_lock, flags);
	ATM_SKB(skb)->vcc = vcc;
	ret = atmAal5DataReq(skb, vcc->vpi, vcc->vci);
	if (!ret)
		atomic_inc(&vcc->stats->tx);
	spin_unlock_irqrestore(&tsarm_lock, flags);
	return ret;
}/*end tc3162_atm_verify_send*/
/*_____________________________________________________________________________
**      function name: cmpCell_tasklet
**      descriptions:
**         Create a tasklet to handle compare atm cell conntent that is to weast the cpu time.
**             
**      parameters:
**           data: tasklet data, not used.
**             
**      global:
**         	tasklet_flag
**             
**      return:
**         	None
**	     
**      call:
**           cmpAtmCell
**      
**      revision:
**      1. Here 2008/12/19
**____________________________________________________________________________
*/
void
cmpCell_tasklet(unsigned long data){
	uint32 flags;
	struct sk_buff *skb=NULL;
	spin_lock_irqsave(&dequeuLock, flags);
	tasklet_flag=1;
	spin_unlock_irqrestore(&dequeuLock, flags);
    	while ((skb = skb_dequeue(&rx_queue)) != NULL){
			cmpAtmCell(skb->cb[0], skb->data,  skb->len);
			dev_kfree_skb(skb);/*Free the skb*/
//			rxcount1++;
	}
	spin_lock_irqsave(&dequeuLock, flags);
	tasklet_flag=0;
	spin_unlock_irqrestore(&dequeuLock, flags);	
}
#ifdef TCSUPPORT_AUTOBENCH
int autobench_sar_lpbk(void){
	int param[4];/*0:VC, 1:VPI, 2:VCI, 3: Priority*/
	int i=0;
	struct sk_buff* skb=NULL;

	test_flag=0;
	stop_loop=0;

	sarLpbkInit();

	test_flag|=FIX_DCELL_FG;

//	openAtmVc(0,0,55);
	openAtmVc(0,0,55, VCCFGR_ATM_PHY0);
	skb=skbmgr_dev_alloc_skb2k();
	if(skb){
		memcpy(skb->data, dataCell, sizeof(dataCell));
		skb_put(skb, sizeof(dataCell));
		atmAal5DataReqVerify(skb, 0, 3);
	}
	else{
		printk("Allocate skb failure!\r\n");
	}

	delay1ms(1000);
	atmAal5VcClose(0, 55);

	return autobench_sar_lpbk_flag;
}

EXPORT_SYMBOL(autobench_sar_lpbk);
#endif
