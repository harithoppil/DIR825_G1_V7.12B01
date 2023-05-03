
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
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rt_flash.h>
#include <linux/version.h>
#include <asm/spram.h>
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/TCIfSetQuery_os.h>
#include <flash_layout_kernel.h>
#include <gpio.h>
#include <led.h>
#include "mt751x_femac.h"
#include "tcconsole.h"
#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif

/************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S
 *************************************************************************
 */
static __IMEM int mt751x_mac_tx(struct sk_buff *skb, struct net_device *dev);
/************************************************************************
 *                        P U B L I C   D A T A
 *************************************************************************
 */

#ifdef TCSUPPORT_QOS
static int qos_flag = NULLQOS;
#endif

//#ifdef QOS_REMARKING  /*Rodney_20090724*/
#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
static int qos_wrr_info[5] = {0};
static int max_prio = 3;
static unsigned char qos_wrr_user = 0x00;
#endif

/************************************************************************
 *                      E X T E R N A L   D A T A
 *************************************************************************
 */
extern unsigned int (*ranand_read_byte)(unsigned long long);
extern int mt751x_macSetUpPhy(macAdapter_t *mac_p);
extern int mt751x_creat_phy_dbg_entry(void);
extern void mt751x_remove_phy_dbg_entry(void);
extern u32 tcMiiStationRead(u32 enetPhyAddr, u32 phyReg);
extern void tcMiiStationWrite(u32 enetPhyAddr, u32 phyReg, u32 miiData);
/************************************************************************
 *                       P R I V A T E   D A T A
 *************************************************************************
 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if defined(TCSUPPORT_MT7510_FE)
static int mac_receive_num = MAC_RECV_THLD; //0 means receive packet and no drop
static int mac_receive_threshold = MAC_RXDESCP_NO_DEFAULT; //0 means receive packet and no drop
#endif

/* Device data */
struct net_device *switch_dev[SWITCH_PORT_MAX];
struct phy_link_state phystate[SWITCH_PORT_MAX];
struct macAdapter *mac_p = NULL;
static DEFINE_SPINLOCK(gimr_lock);
static int proc_txring[TX_QUEUE_NUM];
static char proc_txring_name[TX_QUEUE_NUM][32];

static struct proc_dir_entry *proc_mac_dbg = NULL;
static struct proc_dir_entry *entry_mac_ver = NULL;
static struct proc_dir_entry *entry_eth_stats = NULL;
static struct proc_dir_entry *entry_eth_gsw = NULL;
static struct proc_dir_entry *entry_eth_gsw_mib0 = NULL;
static struct proc_dir_entry *entry_eth_gsw_mib1 = NULL;
static struct proc_dir_entry *entry_eth_gsw_mib2 = NULL;
static struct proc_dir_entry *entry_eth_gsw_mib3 = NULL;
static struct proc_dir_entry *entry_eth_gsw_mib4 = NULL;
static struct proc_dir_entry *entry_eth_gsw_mib5 = NULL;
static struct proc_dir_entry *entry_eth_gsw_mib6 = NULL;
static DEFINE_SPINLOCK(pbus_lock);

/************************************************************************
 *        F U N C T I O N   D E F I N I T I O N S
 *************************************************************************/
int tc3262_gmac_tx(struct sk_buff *skb, struct net_device *dev)
{
	return mt751x_mac_tx(skb, dev);
}
EXPORT_SYMBOL(tc3262_gmac_tx);

#ifndef TC_CONSOLE_ENABLE
void macSend(u32 chanId, struct sk_buff *skb)
{
	// only monitor packet in FE ports
	if ((chanId < 0) || (chanId > (SWITCH_PORT_MAX - 2))) {
		chanId = 0;
	}
	mt751x_mac_tx(skb, switch_dev[chanId]);
}
#else
/*
 * FIXME: this inferface is _JUST_ for ethernet driver layer only,
 * when the skb transfer to br0/etc., the skb->dev should change to
 * the corresponding device object. at that time, we cannot using
 * (struct net_device *)dev->name to identify the port number any more.
 *
 * Nov. 12, Camel Luo
 */
int mt751x_ethdev2port(struct net_device *dev)
{
	int i;
	int len;
	const char *dev_name;

	dev_name = dev->name;
	len = strlen(dev_name);

	for (i = 0; i < ARRAY_SIZE(switch_dev); i++) {
		if (strlen(switch_dev[i]->name) != len) {
			continue;
		}

		if (strcmp(dev_name, switch_dev[i]->name)) {
			continue;
		}

		return i;
	}

	return -1;
}
EXPORT_SYMBOL(mt751x_ethdev2port);

extern u8 dmt_NewVersion;
extern atomic_t tcconsole_port_num;
void macSend(u32 chanId, struct sk_buff *skb)
{
	int use_tcconsole;
	int cur_port;
	dmtConsoleData_t *dmt_data;
	struct sk_buff *skb_tmp;

	dmt_data = NULL;
	skb_tmp = NULL;
	use_tcconsole = FALSE;
	if ((dmt_NewVersion) && (chanId != WAN2LAN_CH_ID)) {
		use_tcconsole = TRUE;
	}

	// only monitor packet in FE ports
	if ((chanId < 0) || (chanId > (SWITCH_PORT_MAX - 2))) {
		chanId = 0;
	}

	if (TRUE == use_tcconsole) {
		/*
		 * FIXME: since this function is call back by dmt
		 * adsl_dev_ops->rts_rcv(), in dmt layer, it cannot
		 * figure out which port to send, so we have hack this
		 * func to send this package to the corresponding port.
		 */
		cur_port = atomic_read(&tcconsole_port_num);
		if (cur_port >= 0) {
			chanId = cur_port;
		}
		dmt_data = (dmtConsoleData_t *)skb;
		skb_tmp = dev_alloc_skb(0x700);
		if (skb_tmp != NULL) {
			memcpy(skb_put(skb_tmp, dmt_data->len), dmt_data->data, dmt_data->len);
			mt751x_mac_tx(skb_tmp, switch_dev[chanId]);
		}
	} else {
		mt751x_mac_tx(skb, switch_dev[chanId]);
	}
}
EXPORT_SYMBOL(macSend);
#endif

#ifdef CONFIG_ETHERNET_DEBUG
unsigned long dump_mask = 0;
void dump_skb(struct sk_buff *skb, char *fun)
{
	char tmp[80];
	char *p = skb->data;
	char *t = tmp;
	int i, n = 0;

	etdebug("%s: skb@%08x, data@%08x dev=%s, skblen=%d\n",
			fun, (u32)skb, (u32)skb->data, (skb->dev) ? skb->dev->name : "NULL", skb->len);
	//	for(i = 0; i < skb->len; i++) {
	for(i = 0; i < 0x20; i++) {
		t += sprintf(t, "%02x ", *p++ & 0xff);
		if ((i & 0x0f) == 0x0f) {
			printk("%04x: %s\n", n, tmp);
			n += 16;
			t = tmp;
		}
	}
	if(i & 0x0f) {
		printk("%04x: %s\n", n, tmp);
	}
}

static int dump_level_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char temp[10];
	int len;
	unsigned long level;

	if(count > 10) {
		len = 10;
	} else {
		len = count;
	}
	memset(temp, 0, sizeof(temp));
	if(copy_from_user(temp, buffer, len - 1)) {
		return -EFAULT;
	}
	level = simple_strtoul(temp, NULL, 16);
	if(dump_mask != level) {
		dump_mask = level;
	}

	return len;
}
#endif

void miiStationWrite(unsigned int PhyAddr, unsigned int PhyReg, unsigned int MiiData)
{
	unsigned int reg;
	int cnt;
	unsigned long flags;

	spin_lock_irqsave(&mac_p->lock, flags);
	for(cnt = 0; cnt < 10000; cnt++) {
		reg=reg_read32(GSW_CFG_PIAC);
		if(0 == (reg & PHY_ACS_ST)) {
			break;
		}
	}
	reg = PHY_ACS_ST | (MDIO_ST_START << MDIO_ST_SHIFT) | (MDIO_CMD_WRITE<<MDIO_CMD_SHIFT) | (PhyAddr << MDIO_PHY_ADDR_SHIFT) | (PhyReg << MDIO_REG_ADDR_SHIFT) | (MiiData & MDIO_RW_DATA);
	reg_write32(GSW_CFG_PIAC, reg);
	for( ; cnt < 20000; cnt++) {
		reg=reg_read32(GSW_CFG_PIAC);
		if(0 == (reg & PHY_ACS_ST)) {
			break;
		}
	}
	spin_unlock_irqrestore(&mac_p->lock, flags);
	if(20000 == cnt) {
		printk("%s: Error, timeout!\n", __func__);
	}
}

unsigned short miiStationRead(unsigned int PhyAddr, unsigned int PhyReg)
{
	unsigned int reg;
	int cnt;
	unsigned short data = 0;
	unsigned long flags;

	spin_lock_irqsave(&mac_p->lock, flags);
	for(cnt = 0; cnt < 10000; cnt++) {
		reg=reg_read32(GSW_CFG_PIAC);
		if(0 == (reg & PHY_ACS_ST)) {
			break;
		}
	}
	reg = PHY_ACS_ST | (MDIO_ST_START << MDIO_ST_SHIFT) | (MDIO_CMD_READ<<MDIO_CMD_SHIFT) | (PhyAddr << MDIO_PHY_ADDR_SHIFT) | (PhyReg << MDIO_REG_ADDR_SHIFT);
	reg_write32(GSW_CFG_PIAC, reg);
	for( ; cnt < 20000; cnt++) {
		reg=reg_read32(GSW_CFG_PIAC);
		if(0 == (reg & PHY_ACS_ST)) {
			break;
		}
	}
	data = reg & MDIO_RW_DATA;
	spin_unlock_irqrestore(&mac_p->lock, flags);

	if (20000 == cnt) {
		printk("%s: Error, timeout!\n", __func__);
	}
	return data;
}

unsigned int tcPhyReadReg(unsigned char port_num, unsigned char reg_num)
{
	unsigned int val, val_r31;
	unsigned int phyAddr = mac_p->enetPhyAddr + port_num;

	if((reg_num<16) || (reg_num==31)) {
		val = miiStationRead(phyAddr, reg_num);
	} else {
		val_r31 = miiStationRead(phyAddr, 31); // remember last page
		if(val_r31 != 0x8000) {// set page to L0 if necessary
			miiStationWrite(phyAddr, 31, 0x8000);
		}
		val = miiStationRead(phyAddr, reg_num);// read reg32
		if(val_r31 != 0x8000) {// restore page if necessary
			miiStationWrite(phyAddr, 31, val_r31);
		}
	}

	return val;
}

void tcPhyWriteReg(unsigned char port_num, unsigned char reg_num, unsigned int reg_data)
{
	unsigned int val_r31;
	unsigned int phyAddr = mac_p->enetPhyAddr + port_num;

	val_r31 = miiStationRead(phyAddr, 31);// remember last page
	if(val_r31 != 0x8000) {// set page if necessary
		miiStationWrite(phyAddr, 31, 0x8000); // page to L0
	}
	miiStationWrite(phyAddr, reg_num, reg_data);
	if(val_r31 != 0x8000) {// restore page if necessary
		miiStationWrite(phyAddr, 31, val_r31);
	}
}

struct tcswitch switchtable[] = {
	{0x9400, 0,  1, "LEH2031"},
	{0x9401, 1,  1, "LEM2011MB"},
	{0x9402, 2,  4, "TC2104MC"},
	{0x9403, 3,  4, "TC2104SD"},
	{0x9404, 4,  1, "TC2101ME"}, /* 62UE */
	{0x9405, 5,  2, "TC2102ME"}, /* TC3182 */
	{0x9406, 6,  4, "TC2104ME"}, /* TC2206F */
	{0x9407, 7,  1, "TC2101MF"},
	{0x940B, 11, 5, "TC2105MJ"},/* RT63365/RT63368 */
	{0x940C, 12, 5, "TC2105SK"},/* TC6508 */
	{0x9412, 13, 1, "MT7530E2"},
	{0x940F, 15, 5, "MT7510FE"},
	{0x9421, 16, 1, "MT7510GE"},
	{0x0000, 0,  5, "Unknown"},
};

#if 0
#define MT7510FE_PHY_INIT_GDATA_LEN 6
#define MT7510FE_PHY_INIT_LDATA_LEN 5
#define MT7510FE_PHY_INIT_PERPDATA_LEN 1
#define MT7510FE_PORTNUM 5
struct cfg_data_s {
	unsigned int reg_num;
	unsigned int val;
};

struct mt7510FE_cfg_data_s {
	struct cfg_data_s gdata[MT7510FE_PHY_INIT_GDATA_LEN];
	struct cfg_data_s ldata[MT7510FE_PHY_INIT_LDATA_LEN];
	struct cfg_data_s perpdata[MT7510FE_PHY_INIT_PERPDATA_LEN]; //per port register setting
};

struct mt7510FE_cfg_data_s mt7510FE_cfg = {
	{ {31,0x2000},{22,0x0092},{23,0x007d},{24,0x09b0},{25,0x0f6e},{26,0x0fc4} },
	{ {31,0x9000},{31,0xb000},{17,0x0000} },//disable EEE
	{ {31,0x8000} }
};

/* write Local Reg */
void tcPhyWriteLReg(unsigned char port_num, unsigned char page_num, unsigned char reg_num, unsigned int reg_data)
{
	unsigned int val_r31;
	unsigned int phyAddr = mac_p->enetPhyAddr + port_num;
	unsigned int pageAddr = (page_num<<12)+0x8000;

	val_r31 = miiStationRead(phyAddr, 31);// remember last page
	if (val_r31 != pageAddr) {// set page if necessary
		miiStationWrite(phyAddr, 31, pageAddr);// switch to page Lx
	}
	miiStationWrite(phyAddr, reg_num, reg_data);
	if(val_r31 != pageAddr) {// restore page if necessary
		miiStationWrite(phyAddr, 31, val_r31);
	}
}

/* write Global Reg */
void tcPhyWriteGReg(unsigned char port_num, unsigned char page_num, unsigned char reg_num, unsigned int reg_data)
{
	unsigned int val_r31;
	unsigned int phyAddr = mac_p->enetPhyAddr + port_num;
	unsigned int pageAddr = (page_num << 12);

	val_r31 = miiStationRead(phyAddr, 31);// remember last page
	if (val_r31 != pageAddr) {// set page if necessary
		miiStationWrite(phyAddr, 31, pageAddr);// switch to page Gx
	}
	miiStationWrite(phyAddr, reg_num, reg_data);
	if(val_r31 != pageAddr) {// restore page if necessary
		miiStationWrite(phyAddr, 31, val_r31);
	}
}

void mt7510FECfgLoad(void)
{
	int pn, i;
	const unsigned short mt7510FE_G0R22_Tx10AmpSave_ON  = 0x0264;
	const unsigned short mt7510FE_G0R22_Tx10AmpSave_OFF = 0x006F;
	unsigned short mt7510FE_L2R16[MT7510FE_PORTNUM] = {0x0606, 0x0606, 0x0606, 0x0606, 0x0606};
	unsigned short mt7510FE_L2R17_A1[MT7510FE_PORTNUM] = {0x0002, 0x0002, 0x0002, 0x0002, 0x0002};
	unsigned short mt7510FE_L3R17_A1[MT7510FE_PORTNUM] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
	unsigned short reg_data = mt7510FE_G0R22_Tx10AmpSave_OFF;// disable
	static unsigned char cfg_Tx10AmpSave_flag = 1;
	unsigned short phyAddr = mac_p->enetPhyAddr;

	for(i=0; i < MT7510FE_PHY_INIT_GDATA_LEN; i++) {// global registers
		miiStationWrite(phyAddr, mt7510FE_cfg.gdata[i].reg_num, mt7510FE_cfg.gdata[i].val );
	}
	if(1 == cfg_Tx10AmpSave_flag){ // enable
		reg_data = mt7510FE_G0R22_Tx10AmpSave_ON;
	}
	tcPhyWriteGReg(mac_p->enetPhyAddr, 0, 22, reg_data);/* mt7510FECfgTx10AmpSave */
	for(pn = 0; pn < MT7510FE_PORTNUM; pn++){/* DO_4_PORT of mt7510FELRCfgLoad */
		for(i = 0; i < MT7510FE_PHY_INIT_LDATA_LEN; i++){
			miiStationWrite((pn + mac_p->enetPhyAddr), mt7510FE_cfg.ldata[i].reg_num, mt7510FE_cfg.ldata[i].val);
		}
		tcPhyWriteLReg(pn,2,16,mt7510FE_L2R16[pn]);
		tcPhyWriteLReg(pn,2,17,mt7510FE_L2R17_A1[pn]);// load revision-related settings
		tcPhyWriteLReg(pn,3,17,mt7510FE_L3R17_A1[pn]);
	}
}
#endif

void tcAdmMiiStationWrite(unsigned int admReg, unsigned int miiData)
{
	unsigned int phyaddr;
	unsigned int reg;

	phyaddr = admReg >> 5;
	reg = admReg & 0x1f;

	miiStationWrite(phyaddr, reg, miiData);
}

unsigned int tcAdmMiiStationRead(unsigned int admReg)
{
	unsigned int phyaddr;
	unsigned int reg;

	phyaddr = admReg >> 5;
	reg = admReg & 0x1f;

	return miiStationRead(phyaddr, reg);
}

static int mdio_read(struct net_device *dev, int phy_id, int reg_num)
{
	return miiStationRead(phy_id, reg_num);
}

static void mdio_write(struct net_device *dev, int phy_id, int reg_num, int val)
{
	miiStationWrite(phy_id, reg_num, val);
}


void enable_abnormal_irq(void)
{
	unsigned char port_num;
	unsigned int val;

	for(port_num=0; port_num<=4; port_num++) {
		reg_read32(GSW_PINT_EN(port_num)) = 0xbeff;
		val = reg_read32(GSW_PINT_EN(port_num));
	}
}


/************************************************************************
 *     A D S L   R E L A T E D    F U N C T I O N S
 *************************************************************************
 */

/* ADSL RTS dump function */
void TCConsole(unsigned char mode)
{
}
EXPORT_SYMBOL(TCConsole);

void uartMacPutchar(int ch)
{
}
EXPORT_SYMBOL(uartMacPutchar);

unsigned int GetIpAddr(void)
{
	return 0;
}
EXPORT_SYMBOL(GetIpAddr);

unsigned char *GetMacAddr(void)
{
	return switch_dev[0]->dev_addr;
}
EXPORT_SYMBOL(GetMacAddr);


/************************************************************************
 *     E T H E R N E T    D E V I C E   P R O C  D E F I N I T I O N S
 *************************************************************************/

static int getGSWLinkSt(char *buf)
{
	int index = 0;
	unsigned int reg;
	int port;
	int speed;

	for(port = 0; port <= 6; port++) {
		index += sprintf(buf+index, "Internal Port %d: ", port);
		reg = reg_read32(GSW_PMSR(port));
		if(!(reg & MAC_LINK_STS)) {
			index += sprintf(buf+index, "Down\n");
			continue;
		}
		speed = (reg & MAC_SPD_STS) >> MAC_SPD_STS_SHIFT;
		if(speed == PN_SPEED_1000M) {
			index += sprintf(buf+index, "1000M/");
		} else if (speed == PN_SPEED_100M) {
			index += sprintf(buf+index, "100M/");
		} else {
			index += sprintf(buf+index, "10M/");
		}
		if(reg & MAC_DPX_STS) {
			index += sprintf(buf+index, "Full Duplex");
		} else {
			index += sprintf(buf+index, "Half Duplex");
		}
		if(reg & (TX_FC_STS | RX_FC_STS)) {
			index += sprintf(buf+index, " FC:");
			if(reg & TX_FC_STS) {
				index += sprintf(buf+index, " TX");
			}
			if(reg & RX_FC_STS) {
				index += sprintf(buf+index, " RX");
			}
		}
		if(reg & EEE100_STS) {
			index += sprintf(buf+index, " EEE100");
		}
		if(reg & EEE1G_STS) {
			index += sprintf(buf+index, " EEE1G");
		}
		index += sprintf(buf+index, "\n");
	}
	return index;
}

static int eth_stats_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;

	index += sprintf(buf+index, "inOctets              = 0x%08x, ", mac_p->macStat.MIB_II.inOctets);
	CHK_BUF();
	index += sprintf(buf+index, "inUnicastPkts         = 0x%08x\n", mac_p->macStat.MIB_II.inUnicastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "inMulticastPkts       = 0x%08x, ", mac_p->macStat.MIB_II.inMulticastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "inDiscards            = 0x%08x\n", mac_p->macStat.MIB_II.inDiscards);
	CHK_BUF();
	index += sprintf(buf+index, "inErrors              = 0x%08x, ", mac_p->macStat.MIB_II.inErrors);
	CHK_BUF();
	index += sprintf(buf+index, "outOctets             = 0x%08x\n", mac_p->macStat.MIB_II.outOctets);
	CHK_BUF();
	index += sprintf(buf+index, "outUnicastPkts        = 0x%08x, ", mac_p->macStat.MIB_II.outUnicastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "outMulticastPkts      = 0x%08x\n", mac_p->macStat.MIB_II.outMulticastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "outDiscards           = 0x%08x, ", mac_p->macStat.MIB_II.outDiscards);
	CHK_BUF();
	index += sprintf(buf+index, "outErrors             = 0x%08x\n", mac_p->macStat.MIB_II.outErrors);
	CHK_BUF();
	index += sprintf(buf+index, "\n[ Statistics Display ]\n");
	CHK_BUF();
	index += sprintf(buf+index, "txJabberTimeCnt       = 0x%08x  ", mac_p->macStat.inSilicon.txJabberTimeCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txLossOfCarrierCnt    = 0x%08x\n", mac_p->macStat.inSilicon.txLossOfCarrierCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txNoCarrierCnt        = 0x%08x  ", mac_p->macStat.inSilicon.txNoCarrierCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txLateCollisionCnt    = 0x%08x\n", mac_p->macStat.inSilicon.txLateCollisionCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txExCollisionCnt      = 0x%08x  ", mac_p->macStat.inSilicon.txExCollisionCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txHeartbeatFailCnt    = 0x%08x\n", mac_p->macStat.inSilicon.txHeartbeatFailCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txCollisionCnt        = 0x%08x  ", mac_p->macStat.inSilicon.txCollisionCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txExDeferralCnt       = 0x%08x\n", mac_p->macStat.inSilicon.txExDeferralCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txUnderRunCnt         = 0x%08x  ", mac_p->macStat.inSilicon.txUnderRunCnt);
	CHK_BUF();
	index += sprintf(buf+index, "rxAlignErr            = 0x%08x\n", mac_p->macStat.inSilicon.rxAlignErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxDribblingErr        = 0x%08x  ", mac_p->macStat.inSilicon.rxDribblingErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxSymbolErr           = 0x%08x\n", mac_p->macStat.inSilicon.rxSymbolErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxMiiErr              = 0x%08x  ", mac_p->macStat.inSilicon.rxMiiErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxCollisionErr        = 0x%08x\n", mac_p->macStat.inSilicon.rxCollisionErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxCrcErr              = 0x%08x  ", mac_p->macStat.inSilicon.rxCrcErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxEtherFrameLengthErr = 0x%08x\n", mac_p->macStat.inSilicon.rxEtherFrameLengthErr);
	CHK_BUF();
	index += sprintf(buf+index, "rx802p3FrameLengthErr = 0x%08x  ", mac_p->macStat.inSilicon.rx802p3FrameLengthErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxPktIPChkSumErr      = 0x%08x\n", mac_p->macStat.inSilicon.rxPktIPChkSumErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxRuntErr             = 0x%08x  ", mac_p->macStat.inSilicon.rxRuntErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxLongErr             = 0x%08x\n", mac_p->macStat.inSilicon.rxLongErr);
	CHK_BUF();
	index += sprintf(buf+index, "\n[ Extra information ]\n");
	CHK_BUF();
	index += sprintf(buf+index, "Rx Descriptor idx     = 0x%08x  ", mac_p->rxCurrentDescp);
	CHK_BUF();
	index += sprintf(buf+index, "Tx Descriptor idx     = 0x%08x\n", mac_p->txCurrentDescp[0]);
	CHK_BUF();
	index += sprintf(buf+index, "Rx Enqueued cnt       = 0x%08x  ", mac_p->macStat.inSilicon.rxEnQueueNum);
	CHK_BUF();
	index += sprintf(buf+index, "Tx Enqueued cnt       = 0x%08x\n", mac_p->macStat.inSilicon.txEnQueueNum);
	CHK_BUF();
	index += sprintf(buf+index, "Rx Dequeued cnt       = 0x%08x  ", mac_p->macStat.inSilicon.rxDeQueueNum);
	CHK_BUF();
	index += sprintf(buf+index, "Tx Dequeued cnt       = 0x%08x\n", mac_p->macStat.inSilicon.txDeQueueNum);
	CHK_BUF();
	index += sprintf(buf+index, "Tx Buf UnReleased cnt = 0x%08x  ", mac_p->txUnReleasedBufCnt[0]);
	CHK_BUF();
	index += sprintf(buf+index, "Tx Buf UnReleased idx = 0x%08x\n", mac_p->txUnReleasedDescp[0]);
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

static int eth_stats_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;
	val_string[count] = '\0';
	memset(&mac_p->macStat.MIB_II, 0, sizeof(macMIB_II_t));
	memset(&mac_p->macStat.inSilicon, 0, sizeof(inSiliconStat_t));

	return count;
}

static int gsw_stats_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int port;

	for(port = 0; port <= 6; port++) {
		index += sprintf(buf+index, "[Port %d]: ", port);
		CHK_BUF();
		index += sprintf(buf+index, "TxDrop       =0x%08x ", reg_read32(GSW_TDPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxCRC        =0x%08x\n", reg_read32(GSW_TCRC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxUnicast    =0x%08x ", reg_read32(GSW_TUPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxMulticast  =0x%08x ", reg_read32(GSW_TMPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxBroadcast  =0x%08x ", reg_read32(GSW_TBPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxCollision  =0x%08x\n", reg_read32(GSW_TCEC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxSCollision =0x%08x ", reg_read32(GSW_TSCEC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxMCollision =0x%08x ", reg_read32(GSW_TMCEC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxDeferred   =0x%08x ", reg_read32(GSW_TDEC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxLCollision =0x%08x\n", reg_read32(GSW_TLCEC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxECollision =0x%08x ", reg_read32(GSW_TXCEC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxPause      =0x%08x ", reg_read32(GSW_TPPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxDrop       =0x%08x ", reg_read32(GSW_RDPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxFilter     =0x%08x\n", reg_read32(GSW_RFPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxUnicast    =0x%08x ", reg_read32(GSW_RUPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxMulticast  =0x%08x ", reg_read32(GSW_RMPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxBroadcast  =0x%08x ", reg_read32(GSW_RBPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxAlignError =0x%08x\n", reg_read32(GSW_RAEPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxCRCError   =0x%08x ", reg_read32(GSW_RCEPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxUdrsize    =0x%08x ", reg_read32(GSW_RUSPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxFragError  =0x%08x ", reg_read32(GSW_RFEPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxOverSize   =0x%08x\n", reg_read32(GSW_ROSPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxPause      =0x%08x ", reg_read32(GSW_RPPC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxCtrlDrop   =0x%08x ", reg_read32(GSW_RDPC_CTRL(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxIngDrop    =0x%08x ", reg_read32(GSW_RDPC_ING(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxArlDrop    =0x%08x\n", reg_read32(GSW_RDPC_ARL(port)));
		CHK_BUF();
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

static int gsw_stats_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	return count;
}

static int gsw_link_st_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = getGSWLinkSt(buf);

	if(len <= off+count)
		*eof = 1;
	*start = buf + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
		len = 0;

	return len;
}


static int eth_reg_dump_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int i;

	index += sprintf(buf+index, "FE_GLO_CFG      (%08X)=%8X ", FE_GLO_CFG, reg_read32(FE_GLO_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "FE_RST_GLO      (%08X)=%8X\n", FE_RST_GLO, reg_read32(FE_RST_GLO));
	CHK_BUF();
	index += sprintf(buf+index, "FE_INT_STATUS   (%08X)=%8X ", FE_INT_STATUS, reg_read32(FE_INT_STATUS));
	CHK_BUF();
	index += sprintf(buf+index, "FE_INT_ENABLE   (%08X)=%8X\n", FE_INT_ENABLE, reg_read32(FE_INT_ENABLE));
	CHK_BUF();
	index += sprintf(buf+index, "FE_FOE_TS_T     (%08X)=%8X ", FE_FOE_TS_T, reg_read32(FE_FOE_TS_T));
	CHK_BUF();
	index += sprintf(buf+index, "FE_IPV6_EXT     (%08X)=%8X\n", FE_IPV6_EXT, reg_read32(FE_IPV6_EXT));
	CHK_BUF();
	for(i = 0; i < TX_QUEUE_NUM; i++) {
		index += sprintf(buf+index, "TX_BASE_PTR(%d)  (%08X)=%8X\n", i, TX_BASE_PTR(i), reg_read32(TX_BASE_PTR(i)));
		CHK_BUF();
		index += sprintf(buf+index, "TX_MAX_CNT(%d)   (%08X)=%8X ", i, TX_MAX_CNT(i), reg_read32(TX_MAX_CNT(i)));
		CHK_BUF();
		index += sprintf(buf+index, "TX_CTX_IDX(%d)   (%08X)=%8X\n", i, TX_CTX_IDX(i), reg_read32(TX_CTX_IDX(i)));
		CHK_BUF();
		index += sprintf(buf+index, "TX_DTX_IDX(%d)   (%08X)=%8X ", i, TX_DTX_IDX(i), reg_read32(TX_DTX_IDX(i)));
		CHK_BUF();
	}
	for(i = 0; i < RX_QUEUE_NUM; i++) {
		index += sprintf(buf+index, "RX_BASE_PTR(%d)  (%08X)=%8X\n", i, RX_BASE_PTR(i), reg_read32(RX_BASE_PTR(i)));
		CHK_BUF();
		index += sprintf(buf+index, "RX_MAX_CNT(%d)   (%08X)=%8X ", i, RX_MAX_CNT(i), reg_read32(RX_MAX_CNT(i)));
		CHK_BUF();
		index += sprintf(buf+index, "RX_CALC_IDX(%d)  (%08X)=%8X\n", i, RX_CALC_IDX(i), reg_read32(RX_CALC_IDX(i)));
		CHK_BUF();
		index += sprintf(buf+index, "RX_DRX_IDX(%d)   (%08X)=%8X ", i, RX_DRX_IDX(i), reg_read32(RX_DRX_IDX(i)));
		CHK_BUF();
	}
	index += sprintf(buf+index, "PDMA_INFO       (%08X)=%8X\n", PDMA_INFO, reg_read32(PDMA_INFO));
	CHK_BUF();
	index += sprintf(buf+index, "PDMA_GLO_CFG    (%08X)=%8X ", PDMA_GLO_CFG, reg_read32(PDMA_GLO_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "PDMA_RST_IDX    (%08X)=%8X\n", PDMA_RST_IDX, reg_read32(PDMA_RST_IDX));
	CHK_BUF();
	index += sprintf(buf+index, "DLY_INT_CFG     (%08X)=%8X ", DLY_INT_CFG, reg_read32(DLY_INT_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "FREEQ_THRES     (%08X)=%8X\n", FREEQ_THRES, reg_read32(FREEQ_THRES));
	CHK_BUF();
	index += sprintf(buf+index, "INT_STATUS      (%08X)=%8X ", INT_STATUS, reg_read32(INT_STATUS));
	CHK_BUF();
	index += sprintf(buf+index, "INT_MASK        (%08X)=%8X\n", INT_MASK, reg_read32(INT_MASK));
	CHK_BUF();
	index += sprintf(buf+index, "SCH_Q01_CFG     (%08X)=%8X ", SCH_Q01_CFG, reg_read32(SCH_Q01_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "SCH_Q23_CFG     (%08X)=%8X\n", SCH_Q23_CFG, reg_read32(SCH_Q23_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_FWD_CFG    (%08X)=%8X ", GDM1_FWD_CFG, reg_read32(GDM1_FWD_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_SHRP_CFG   (%08X)=%8X\n", GDM1_SHRP_CFG, reg_read32(GDM1_SHRP_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_MAC_ADRL   (%08X)=%8X ", GDM1_MAC_ADRL, reg_read32(GDM1_MAC_ADRL));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_MAC_ADRH   (%08X)=%8X\n", GDM1_MAC_ADRH, reg_read32(GDM1_MAC_ADRH));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_FQFC_CFG    (%08X)=%8X ", PSE_FQFC_CFG, reg_read32(PSE_FQFC_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_IQ_REV1     (%08X)=%8X\n", PSE_IQ_REV1, reg_read32(PSE_IQ_REV1));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_IQ_REV2     (%08X)=%8X ", PSE_IQ_REV2, reg_read32(PSE_IQ_REV2));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_IQ_STA1     (%08X)=%8X\n", PSE_IQ_STA1, reg_read32(PSE_IQ_STA1));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_IQ_STA2     (%08X)=%8X ", PSE_IQ_STA2, reg_read32(PSE_IQ_STA2));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_OQ_STA1     (%08X)=%8X\n", PSE_OQ_STA1, reg_read32(PSE_OQ_STA1));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_OQ_STA2     (%08X)=%8X ", PSE_OQ_STA2, reg_read32(PSE_OQ_STA2));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_MIR_PORT    (%08X)=%8X\n", PSE_MIR_PORT, reg_read32(PSE_MIR_PORT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_FWD_CFG   (%08X)=%8X ", GDMA2_FWD_CFG, reg_read32(GDMA2_FWD_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_SHRP_CFG  (%08X)=%8X\n", GDMA2_SHRP_CFG, reg_read32(GDMA2_SHRP_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_SHRP_CFG  (%08X)=%8X ", GDMA2_SHRP_CFG, reg_read32(GDMA2_SHRP_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_MAC_ADRL  (%08X)=%8X\n", GDMA2_MAC_ADRL, reg_read32(GDMA2_MAC_ADRL));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_MAC_ADRH  (%08X)=%8X ", GDMA2_MAC_ADRH, reg_read32(GDMA2_MAC_ADRH));
	CHK_BUF();
	index += sprintf(buf+index, "CDMP_VLAN_CTRL  (%08X)=%8X\n", CDMP_VLAN_CTRL, reg_read32(CDMP_VLAN_CTRL));
	CHK_BUF();
	index += sprintf(buf+index, "CDMP_PPP_GEN    (%08X)=%8X ", CDMP_PPP_GEN, reg_read32(CDMP_PPP_GEN));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_RX_GBCNT_L (%08X)=%8X\n", GDM1_RX_GBCNT_L, reg_read32(GDM1_RX_GBCNT_L));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_RX_GBCNT_H (%08X)=%8X ", GDM1_RX_GBCNT_H, reg_read32(GDM1_RX_GBCNT_H));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_RX_GPCNT   (%08X)=%8X\n", GDM1_RX_GPCNT, reg_read32(GDM1_RX_GPCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_RX_OERCNT  (%08X)=%8X ", GDM1_RX_OERCNT, reg_read32(GDM1_RX_OERCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_RX_FERCNT  (%08X)=%8X\n", GDM1_RX_FERCNT, reg_read32(GDM1_RX_FERCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_RX_SERCNT  (%08X)=%8X ", GDM1_RX_SERCNT, reg_read32(GDM1_RX_SERCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_RX_LERCNT  (%08X)=%8X\n", GDM1_RX_LERCNT, reg_read32(GDM1_RX_LERCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_RX_CERCNT  (%08X)=%8X ", GDM1_RX_CERCNT, reg_read32(GDM1_RX_CERCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_RX_FCCNT   (%08X)=%8X\n", GDM1_RX_FCCNT, reg_read32(GDM1_RX_FCCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_TX_SKIPCNT (%08X)=%8X ", GDM1_TX_SKIPCNT, reg_read32(GDM1_TX_SKIPCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_TX_COLCNT  (%08X)=%8X\n", GDM1_TX_COLCNT, reg_read32(GDM1_TX_COLCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_TX_GBCNT_L (%08X)=%8X ", GDM1_TX_GBCNT_L, reg_read32(GDM1_TX_GBCNT_L));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_TX_GBCNT_H (%08X)=%8X\n", GDM1_TX_GBCNT_H, reg_read32(GDM1_TX_GBCNT_H));
	CHK_BUF();
	index += sprintf(buf+index, "GDM1_TX_GPCNT   (%08X)=%8X ", GDM1_TX_GPCNT, reg_read32(GDM1_TX_GPCNT));
	CHK_BUF();
	index += sprintf(buf+index, "[PHY REG] PHYADDR=%d\n", mac_p->enetPhyAddr);
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

static int eth_rxring_dump_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int i;
	struct PDMA_rxdesc *pRxDescp;
	struct PDMA_rxdesc pRxDescpTmpVal;
	struct PDMA_rxdesc *pRxDescpTmp = &pRxDescpTmpVal;

	pRxDescp = (struct PDMA_rxdesc*) mac_p->rxDescrRingBaseVAddr;
	index += sprintf(buf+index, "rx descr ring=%08x\n", (unsigned int) pRxDescp);
	CHK_BUF();
	for(i = 0 ; i< mac_p->rxRingSize ; i++, pRxDescp++) {
		pRxDescpTmp = pRxDescp;
		index += sprintf(buf+index, "i= %d descr=%08x\n", i, (unsigned int) pRxDescp);
		CHK_BUF();
		index += sprintf(buf+index, " rdes1=%08x\n", pRxDescpTmp->rxd_info1.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes2=%08x\n", pRxDescpTmp->rxd_info2.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes3=%08x\n", pRxDescpTmp->rxd_info3.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes4=%08x\n", pRxDescpTmp->rxd_info4.word);
		CHK_BUF();
		index += sprintf(buf+index, " skb  =%08x\n", (unsigned int) mac_p->rxskbs[i]);
		CHK_BUF();
	}
	index += sprintf(buf+index, "rxCurrentDescp    =%d\n", mac_p->rxCurrentDescp);
	CHK_BUF();
	index += sprintf(buf+index, "RX_CALC_IDX(0)    =%08x\n", reg_read32(RX_CALC_IDX(0)));
	CHK_BUF();
	index += sprintf(buf+index, "RX_DRX_IDX(0)     =%08x\n", reg_read32(RX_DRX_IDX(0)));
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

static int eth_txring_dump_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int i, txq;
	struct PDMA_txdesc *pTxDescp;
	struct PDMA_txdesc pTxDescpTmpVal;
	struct PDMA_txdesc *pTxDescpTmp = &pTxDescpTmpVal;

	txq = *((int *) data);
	pTxDescp = (struct PDMA_txdesc*) mac_p->txDescrRingBaseVAddr[txq];
	index += sprintf(buf+index, "tx descr ring%d=%08x\n", txq, (unsigned int) pTxDescp);
	CHK_BUF();
	for(i = 0 ; i < mac_p->txRingSize ; i++, pTxDescp++) {
		pTxDescpTmp = pTxDescp;
		index += sprintf(buf+index, "index=%03d descr=%08x skb=%08x\n", i, (unsigned int)pTxDescp, (unsigned int)mac_p->txskbs[txq][i]);
		CHK_BUF();
		index += sprintf(buf+index, " Tdes1=%08x", pTxDescpTmp->txd_info1.word);
		CHK_BUF();
		index += sprintf(buf+index, " Tdes2=%08x", pTxDescpTmp->txd_info2.word);
		CHK_BUF();
		index += sprintf(buf+index, " Tdes3=%08x", pTxDescpTmp->txd_info3.word);
		CHK_BUF();
		index += sprintf(buf+index, " Tdes4=%08x\n", pTxDescpTmp->txd_info4.word);
		CHK_BUF();

	}
	index += sprintf(buf+index, "txCurrentDescp[%d]    =%d\n", txq, mac_p->txCurrentDescp[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "txUnReleasedDescp[%d] =%d\n", txq, mac_p->txUnReleasedDescp[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "txUnReleasedBufCnt[%d]=%d\n", txq, mac_p->txUnReleasedBufCnt[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "TX_CTX_IDX(%d)        =%08x\n", txq, reg_read32(TX_CTX_IDX(txq)));
	CHK_BUF();
	index += sprintf(buf+index, "TX_DTX_IDX(%d)        =%08x\n", txq, reg_read32(TX_DTX_IDX(txq)));
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

#if defined (QOS_REMARKING) || defined (TCSUPPORT_HW_QOS)

static unsigned char get_qos_weight(unsigned char weight)
{
	if (weight <= 1)
		return MAX_WEIGHT_1023;
	else if ((weight >= 2) && (weight <= 3))
		return MAX_WEIGHT_2047;
	else if ((weight >= 4) && (weight <= 6))
		return MAX_WEIGHT_4095;
	else if (weight >= 7)
		return MAX_WEIGHT_8191;
	return MAX_WEIGHT_8191;
}

static int eth_qoswrr_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	printk("%d %d %d %d %d\n", *qos_wrr_info, *(qos_wrr_info + 1), *(qos_wrr_info + 2), *(qos_wrr_info + 3), *(qos_wrr_info + 4));
	return 0;
}

static int eth_qoswrr_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int len;
	char get_buf[32];
	unsigned int reg;
	int max_wrr_val = 0, i;

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
	if(*qos_wrr_info == 0) { /*strict priority*/
		/* Min BW = Max BW = unlimited */
		reg = reg_read32(SCH_Q01_CFG);
		reg &= ~(MAX_WEIGHT1 | MIN_RATE_RATIO1 | MAX_WEIGHT0 | MIN_RATE_RATIO0);
		reg |= (MIN_RATIO0<<MIN_RATE_RATIO1_SHIFT) | (MIN_RATIO0<<MIN_RATE_RATIO0_SHIFT);
		reg |= (MAX_WEIGHT_2047<<MAX_WEIGHT1_SHIFT) | (MAX_WEIGHT_1023<<MAX_WEIGHT0_SHIFT);
		reg_write32(SCH_Q01_CFG, reg);

		reg = reg_read32(SCH_Q23_CFG);
		reg &= ~(MAX_WEIGHT3 | MIN_RATE_RATIO3 | MAX_WEIGHT2 | MIN_RATE_RATIO2);
		reg |= (MIN_RATIO0<<MIN_RATE_RATIO3_SHIFT) | (MIN_RATIO0<<MIN_RATE_RATIO2_SHIFT);
		reg |= (MAX_WEIGHT_8191<<MAX_WEIGHT3_SHIFT) | (MAX_WEIGHT_4095<<MAX_WEIGHT2_SHIFT);
		reg_write32(SCH_Q23_CFG, reg);

		/* set GDMA2_SHRP_CFG */
		reg = reg_read32(GDMA2_SHRP_CFG);
		reg &= ~GDM_SCH_MOD;
		reg |= GDM_SCH_MOD_SP<<GDM_SCH_MOD_SHIFT;
		reg_write32(GDMA2_SHRP_CFG, reg);
	} else {  /*WRR*/
		/* Min BW = 0, Max BW = unlimited */
		reg = reg_read32(SCH_Q01_CFG);
		reg &= ~(MAX_WEIGHT1 | MIN_RATE_RATIO1 | MAX_WEIGHT0 | MIN_RATE_RATIO0);
		reg |= (MIN_RATIO3<<MIN_RATE_RATIO1_SHIFT) | (MIN_RATIO3<<MIN_RATE_RATIO0_SHIFT);
		reg |= (get_qos_weight(qos_wrr_info[3] & 0x0f)<<MAX_WEIGHT1_SHIFT) | (get_qos_weight(qos_wrr_info[4] & 0x0f)<<MAX_WEIGHT0_SHIFT);
		reg_write32(SCH_Q01_CFG, reg);

		reg = reg_read32(SCH_Q23_CFG);
		reg &= ~(MAX_WEIGHT3 | MIN_RATE_RATIO3 | MAX_WEIGHT2 | MIN_RATE_RATIO2);
		reg |= (MIN_RATIO3<<MIN_RATE_RATIO3_SHIFT) | (MIN_RATIO3<<MIN_RATE_RATIO2_SHIFT);
		reg |= (get_qos_weight(qos_wrr_info[1] & 0x0f)<<MAX_WEIGHT3_SHIFT) | (get_qos_weight(qos_wrr_info[2] & 0x0f)<<MAX_WEIGHT2_SHIFT);
		reg_write32(SCH_Q23_CFG, reg);

		/* set GDMA2_SHRP_CFG */
		reg = reg_read32(GDMA2_SHRP_CFG);
		reg &= ~(GDM_SCH_MOD | GDM_WT_Q3 | GDM_WT_Q2 | GDM_WT_Q1 | GDM_WT_Q0);
		reg |= (GDM_SCH_MOD_WRR<<GDM_SCH_MOD_SHIFT) |
			(GDM_WT(qos_wrr_info[1] & 0x0f)<<GDM_WT_Q3_SHIFT) |
			(GDM_WT(qos_wrr_info[2] & 0x0f)<<GDM_WT_Q2_SHIFT) |
			(GDM_WT(qos_wrr_info[3] & 0x0f)<<GDM_WT_Q1_SHIFT) |
			(GDM_WT(qos_wrr_info[4] & 0x0f)<<GDM_WT_Q0_SHIFT);
		reg_write32(GDMA2_SHRP_CFG, reg);

	}
	return len;
}
#endif

#ifdef TCSUPPORT_QOS
static int eth_tcqos_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{

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

static int eth_tcqos_write_proc(struct file *file, const char *buffer,
		unsigned long count, void *data){

	char qos_disc[10];
	int len;

	if (count > 10) {
		len = 10;
	} else {
		len = count;
	}
	memset(qos_disc, 0, sizeof(qos_disc));
	if(copy_from_user(qos_disc, buffer, len - 1))
		return -EFAULT;
	qos_disc[len] = '\0';
	if (!strcmp(qos_disc, "PQ")) {
		qos_flag = QOS_SW_PQ;
	} else if (!strcmp(qos_disc, "WRR")) {
		qos_flag = QOS_SW_WRR;
	} else if (!strcmp(qos_disc, "CAR")) {
		qos_flag = QOS_SW_CAR;
	} else if (!strcmp(qos_disc, "HWWRR")) {
		qos_flag = QOS_HW_WRR;
	} else if (!strcmp(qos_disc, "HWPQ")) {
		qos_flag = QOS_HW_PQ;
	} else {
		qos_flag = NULLQOS;
	}

	return len;
}
#endif

void PHY_power_ops(unsigned int port, int optcode)
{
	unsigned int value;

	if(port < 5) {
		value = tcPhyReadReg(port, MII_BMCR);
		if(0 == optcode) {
			value |= BMCR_PDOWN; /* Power down the PHY */
		} else {
			value &= ~BMCR_PDOWN;
			value |= BMCR_ANRESTART;/* Restart auto-negotiation process */
		}
		//etdebug("Turn %s PHY power on port %d\n", (optcode) ? "ON" : "DOWN", port);
		tcPhyWriteReg(port, MII_BMCR, value);
	}
}

void PHY_led_ops(unsigned int port, unsigned int optcode)
{
	unsigned int value;
	unsigned int pins;

	//etdebug("Turn %s PHY LED on port %d\n", (LED_BOOT_OFF == optcode) ? "OFF" : "ON", port);
	if(port < 5) {/* GPIO Shared Pin */
		pins = (1 << (port + 9));
		value = reg_read32(CR_GPIO_SHARE);
		if(optcode > led_on_trig) {
			value |= pins;
		} else {
			value &= ~pins;/* Set as GPIO pin */
		}
		reg_write32(CR_GPIO_SHARE, value);
	}
}

// Assign Tx Rx Descriptor Control Registers
void macSetDMADescrCtrlReg(struct macAdapter *mac_p)
{
	unsigned int txq;

	for(txq = 0; txq < TX_QUEUE_NUM; txq++) {
		reg_write32(TX_BASE_PTR(txq), K1_TO_PHY(mac_p->txDescrRingBaseVAddr[txq]));
		reg_write32(TX_MAX_CNT(txq), mac_p->txRingSize);
		reg_write32(TX_CTX_IDX(txq), 0);
		reg_write32(PDMA_RST_IDX, RST_DTX_IDX(txq));
	}
	reg_write32(RX_BASE_PTR(0), K1_TO_PHY(mac_p->rxDescrRingBaseVAddr));
	reg_write32(RX_MAX_CNT(0), mac_p->rxRingSize);
	reg_write32(RX_CALC_IDX(0), mac_p->rxRingSize - 1);
	reg_write32(PDMA_RST_IDX, RST_DRX_IDX(0));
}

void macSetGSW(void)
{
	unsigned int reg;

	/* set port 6 as 1Gbps, FC on */
	reg = (IPG_CFG_SHORT<<IPG_CFG_PN_SHIFT) | MAC_MODE_PN | FORCE_MODE_PN |
		MAC_TX_EN_PN | MAC_RX_EN_PN | BKOFF_EN_PN | BACKPR_EN_PN |
		ENABLE_RX_FC_PN | ENABLE_TX_FC_PN | (PN_SPEED_1000M<<FORCE_SPD_PN_SHIFT) |
		FORCE_DPX_PN | FORCE_LNK_PN;
	reg_write32(GSW_PMCR(6), reg);
	reg = (0xff<<MFC_BC_FFP_SHIFT) | (0xff<<MFC_UNM_FFP_SHIFT) | (0xff<<MFC_UNU_FFP_SHIFT) | MFC_CPU_EN	| (6<<MFC_CPU_PORT_SHIFT);
	reg_write32(GSW_MFC, reg);/* set cpu port as port 6 */
	reg = reg_read32(GSW_GMACCR) | (0xC << 2) | 0x2; /* Set MAC max receive length to 1552 bytes */
	reg_write32(GSW_GMACCR, reg);
#ifdef CONFIG_TCPHY
	if (isRT63365){/* enable switch abnormal irq, for error handle when abnormal irq occurs */
		if ( (reg_read32(0xbfb00064) & (0xffff)) == 0x0 ){
			enable_abnormal_irq();
		}
		reg = reg_read32(GSW_CKGCR);
		reg &= ~((1<<4) | (1<<5));
		reg_write32(GSW_CKGCR, reg);

	}
#endif
	reg = reg_read32(GSW_PVC(6));
	reg &= ~0xc0;/* set CPU port 6 as user port */
	reg |= (1<<5);/* set CPU port 6 special tag = 1 *//* Enable Special Tag function */
	reg_write32(GSW_PVC(6), reg);

	reg = (0x8100 << INS_VLAN_SHIFT);
	reg |= CDM_STAG_EN;/* Enable SPECAIL TAG */
	reg_write32(CDMP_VLAN_CTRL, reg);
	
#if defined(TCSUPPORT_MT7510_FE) && defined(TCSUPPORT_CPU_MT7510)
   //disable switch flow control
    reg=read_reg_word ((GSW_BASE+0x1fe0));
    reg &= (~(1<<31));
		write_reg_word ((GSW_BASE+0x1fe0), reg);

#if !defined(MT7530_SUPPORT)
		 //Setup Switch Flow Control.
    reg=read_reg_word ((GSW_BASE+0x1fe0));
    reg &= ~(0xffffff);//page setup
    reg |= 0xd8418;
    write_reg_word ((GSW_BASE+0x1fe0), reg);
   	write_reg_word ((GSW_BASE+0x1fe4), 0x77777777);
    write_reg_word ((GSW_BASE+0x1ff4), 0x3610);
#endif

#else
#if defined(TCSUPPORT_WAN_ETHER)
	//disable switch flow control
	reg=reg_read32((GSW_BASE+0x1fe0));
	reg &= (~(1<<31));
	reg_write32 ((GSW_BASE+0x1fe0), reg);
#endif
#endif
}

void macSetMACCR(void)
{
	unsigned int reg;

	reg = reg_read32(GDM1_FWD_CFG);
	reg &= ~(0x7777);
	reg |= GDM_STAG_EN | GDM_STRPCRC | (GDM_P_CPU<<GDM_UFRC_P_SHIFT) | (GDM_P_CPU<<GDM_BFRC_P_SHIFT) | (GDM_P_CPU<<GDM_MFRC_P_SHIFT) | (GDM_P_CPU<<GDM_OFRC_P_SHIFT);
#ifdef RAETH_CHECKSUM_OFFLOAD
	reg |= GDM_ICS_EN | GDM_TCS_EN | GDM_UCS_EN;
#endif
	reg_write32(GDM1_FWD_CFG, reg);
	
#if defined(TCSUPPORT_MT7510_FE) && defined(TCSUPPORT_CPU_MT7510)
    //Setup Page
    write_reg_word(PSE_IQ_REV1, 0x304818);
    write_reg_word(PSE_IQ_REV2, 0x4028);
#endif

}

static void macSetMacReg(unsigned char *addr)
{
	reg_write32(GDM1_MAC_ADRL, addr[2]<<24 | addr[3]<<16 | addr[4]<<8 | addr[5]);
	reg_write32(GDM1_MAC_ADRH, addr[0]<<8  | addr[1]<<0);
}

static void macDrvStart(unsigned char enable)
{
	unsigned int reg;
	unsigned int imr;

	imr = reg_read32(GSW_CFG_IMR);
	reg = reg_read32(PDMA_GLO_CFG);
	if(enable) {
		reg |= RX_2BYTE_OFFSET | TX_WB_DDONE | (PDMA_BT_SIZE_32DW<<PDMA_BT_SIZE_SHIFT) | RX_DMA_EN | TX_DMA_EN;
#ifdef __BIG_ENDIAN
		reg |= PDMA_BYTE_SWAP;
#endif
		imr |= 0x7F;/* Set 1 to enable port interrupt */
	} else {/* macDrvStop */
		reg &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
		imr &= ~0x7F;/* Set 0 to disable port interrupt */
	}
	reg_write32(PDMA_GLO_CFG, reg);
	reg_write32(GSW_CFG_IMR, imr);
}

static void macDrvDescripReset(struct macAdapter *mac_p)
{
	struct PDMA_rxdesc *pRxDescp;
	struct PDMA_txdesc *pTxDescp;
	struct sk_buff *skb;
	int i;
	unsigned int txq = 0;

	pRxDescp = (struct PDMA_rxdesc*) mac_p->rxDescrRingBaseVAddr;
	for(i = 0; i < mac_p->rxRingSize; i++) {
		skb = mac_p->rxskbs[i];
		if (skb != NULL) {
			dev_kfree_skb_any(skb);
		}
		pRxDescp->rxd_info1.word = 0;// Init Descriptor
		pRxDescp->rxd_info2.word = 0;
		pRxDescp->rxd_info3.word = 0;
		pRxDescp->rxd_info4.word = 0;
		mac_p->rxskbs[i] = NULL;
		pRxDescp++;
	}

	for(txq = 0; txq < TX_QUEUE_NUM; txq++) {
		pTxDescp = (struct PDMA_txdesc*) mac_p->txDescrRingBaseVAddr[txq];
		for(i = 0 ; i < mac_p->txRingSize; i++) {// free all un-released tx mbuf
			skb = mac_p->txskbs[txq][i];
			if(skb != NULL) {
				dev_kfree_skb_any(skb);
			}
			pTxDescp->txd_info1.word = 0;
			pTxDescp->txd_info2.word = 0;
			pTxDescp->txd_info3.word = 0;
			pTxDescp->txd_info4.word = 0;
			mac_p->txskbs[txq][i] = NULL;
			pTxDescp++;
		}
	}
	mac_p->rxCurrentDescp = 0;
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		mac_p->txCurrentDescp[txq] = 0;
		mac_p->txUnReleasedDescp[txq] = 0;
		mac_p->txUnReleasedBufCnt[txq] = 0;
	}
}

int macDrvDescripInit(struct macAdapter *mac_p)
{
	struct PDMA_rxdesc *pRxDescp;
	struct PDMA_txdesc *pTxDescp;
	unsigned int i, txq;
	struct sk_buff *skb;

	mac_p->rxDescrRingBaseVAddr = (unsigned int) &mac_p->macRxMemPool_p->rxDescpBuf[0];
	for(txq = 0; txq < TX_QUEUE_NUM; txq++) {
		mac_p->txDescrRingBaseVAddr[txq] = (unsigned int) &mac_p->macTxMemPool_p->txDescpBuf[txq][0];
	}
	macDrvDescripReset(mac_p);
	/* init. Rx descriptor, allocate memory for each descriptor */
	pRxDescp = (struct PDMA_rxdesc*) mac_p->rxDescrRingBaseVAddr;
	for(i = 0; i< mac_p->rxRingSize; i++, pRxDescp++) {// Init Descriptor
		pRxDescp->rxd_info1.word = 0;
		pRxDescp->rxd_info2.word = 0;
		pRxDescp->rxd_info3.word = 0;
		pRxDescp->rxd_info4.word = 0;
		pRxDescp->rxd_info2.bits.LS0 = 1;  // Assign flag
		skb = skbmgr_dev_alloc_skb2k();
		if(skb == NULL) {
			printk("mt751x_mac_descinit init fail.\n");
			return -1;
		}
		dma_cache_inv((unsigned long)(skb->data), RX_MAX_PKT_LEN);
		skb_reserve(skb, NET_IP_ALIGN);
		pRxDescp->rxd_info1.bits.PDP0 = K1_TO_PHY(skb->data);
		mac_p->rxskbs[i] = skb;
	}
	for(txq = 0; txq < TX_QUEUE_NUM; txq++) {/* init. tx descriptor, don't allocate memory */
		pTxDescp = (struct PDMA_txdesc*) mac_p->txDescrRingBaseVAddr[txq];
		for (i = 0 ; i < mac_p->txRingSize ; i++, pTxDescp++) {// Init descriptor
			pTxDescp->txd_info1.word = 0;
			pTxDescp->txd_info2.word = 0;
			pTxDescp->txd_info3.word = 0;
			pTxDescp->txd_info4.word = 0;
			pTxDescp->txd_info2.bits.LS0_bit = 1;
			pTxDescp->txd_info2.bits.DDONE_bit = 1;
			//pTxDescp->txd_info4.bits.PN = 0;/* CPU */
			pTxDescp->txd_info4.bits.PN = 1;/* GDMA1 */
			mac_p->txskbs[txq][i] = NULL;
		}
	}
	mac_p->rxCurrentDescp = 0;
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		mac_p->txCurrentDescp[txq] = 0;
		mac_p->txUnReleasedDescp[txq] = 0;
		mac_p->txUnReleasedBufCnt[txq] = 0;
	}

	return 0;
}

static struct tcswitch *macSearchPhyAddr(void)
{
	unsigned short reg = 0;
	struct tcswitch *phy = NULL;
	int i;

#ifdef TC3262
	for(i = 0; i < 10000; i++) {
		if(miiStationRead(0 , 2)!=0xffff){
			break;
		}
	}
#endif
	for(mac_p->enetPhyAddr = 0; mac_p->enetPhyAddr < 32; mac_p->enetPhyAddr++) {
		reg = miiStationRead(mac_p->enetPhyAddr , MII_PHYSID1);
		if(0 == reg) {
			reg = miiStationRead(mac_p->enetPhyAddr, MII_PHYSID2);
			//			etdebug("MII_PHYSID1=0x0000, MII_PHYSID2=[%04X]\n", reg);
		} else {
			//			etdebug("MII_PHYSID1=[%04X]\n", reg);
		}
		if(0x03a2 == reg) {
			reg = miiStationRead(mac_p->enetPhyAddr, MII_PHYSID2);
			//			etdebug("MII_PHYSID1=0x03a2, MII_PHYSID2=[%04X]\n", reg);
			break;
		}
	}
	for(i = 0; ; i++) {
		phy = &switchtable[i];
		if((reg == phy->id) || (0 == phy->id)) {
			break;
		}
	}
	if(32 == mac_p->enetPhyAddr) {
		mac_p->enetPhyAddr = 0;
	}
	printk("Switch: %s, PHY addr:%#x\n", phy->name, mac_p->enetPhyAddr);
	return phy;
}

static struct sk_buff *mt751x_vlan_tag_insert(struct sk_buff *skb, struct net_device *dev)
{
	struct sk_buff *tagskb = NULL;
	struct device_priv *priv = netdev_priv(dev);
	unsigned char *vhdr;
	unsigned char port_mask = BIT(priv->phy_id);
	unsigned short vlan_tci = 1;
#if 0
	unsigned int cpu_special_tag_len = VLAN_HLEN;

	if(skb_headroom(skb) < cpu_special_tag_len) {
		tagskb = skb_realloc_headroom(skb, cpu_special_tag_len);
		dev_kfree_skb(skb);
		if(NULL == tagskb) {
			printk("Failed to realloc headroom!\n");
			goto out;
		}
	} else {
		tagskb = skb_unshare(skb, GFP_ATOMIC);
		if(NULL == tagskb) {
			goto out;
		}
	}
	vhdr = skb_push(tagskb, cpu_special_tag_len);
	vhdr += 2 * VLAN_ETH_ALEN;
	memmove(tagskb->data, tagskb->data + cpu_special_tag_len, 2 * VLAN_ETH_ALEN);/* move MAC to new head of data */
	tagskb->dev = switch_dev[priv->phy_id];
	memset(vhdr, 0x00, cpu_special_tag_len);
	*vhdr = SPEC_TPID;/* Add Special Tag for this skb *//* first, the ethernet type */
	if(4 == priv->phy_id) {
		*(unsigned short *)(vhdr) = BIT(15) | BIT(6) | port_mask;
		*(vhdr + 3) = START_VLAN_VID + 1;
	} else {
		*(unsigned short *)(vhdr) = BIT(15) | port_mask;
		*(vhdr + 3) = START_VLAN_VID;
	}
#else
	if(4 == priv->phy_id) {
		vlan_tci += 1;
	}

	tagskb = __vlan_put_tag(skb, vlan_tci);
	if(NULL == tagskb) {
		goto out;
	}
	vhdr = tagskb->data + (ETH_ALEN << 1);
	*(unsigned short *)(vhdr) = BIT(15) | port_mask;

#endif
out:
	return tagskb;
}

static struct net_device *mt751x_vlan_tag_remove(struct sk_buff *skb)
{
	unsigned char port = 0;
	unsigned char *src;
	unsigned int pull_len = ETH_ALEN * 2;
	unsigned int cpu_special_tag_len = VLAN_HLEN;
	unsigned char *vlan_tag;

	vlan_tag = skb->data + pull_len;
	//if(SPEC_TPID == *(vlan_tag)) {/* VLAN_BASE_ID */
	if (SPEC_TPID == *(vlan_tag) || 0x01 == *(vlan_tag)) {/* VLAN_BASE_ID */
		port = *(vlan_tag + 1);/* Find out which port the packet receive from */
    	if (((*(uint16 *)(skb->data+pull_len)) & 0xfc78) == 0x0000) {
    		if(port < SWITCH_PORT_MAX) {
    			src = skb->data;
    			skb_pull(skb, cpu_special_tag_len);
    			memmove(skb->data, src, pull_len);
    		} else {
    			port = SWITCH_PORT_MAX - 1;
    		}
        }
        else {
            ;/* this packet not from switch */
        }
	}

	return switch_dev[port];
}

int macInit(struct net_device *dev)
{
	unsigned int reg;
	unsigned int tmp;
	int ret_val = -EFAULT;

	tmp = FE_RST;
#if 1
	tmp |= ESW_RST;
	tmp |= EPHY_RST;
#endif
	reg = reg_read32(CR_RSTCTRL2);
	reg |= tmp;
	reg_write32(CR_RSTCTRL2, reg);/* reset ethernet phy, ethernet switch, frame engine */
	mdelay(10);
	reg = reg_read32(CR_RSTCTRL2);
	reg &= ~tmp;
	reg_write32(CR_RSTCTRL2, reg);/* de-assert reset ethernet phy, ethernet switch, frame engine */
	mdelay(50);
	macDrvStart(0); /* stop macDrv */
#ifdef CONFIG_TC3162_DMEM
	mac_p->macRxMemPool_p = (macRxMemPool_t *) alloc_sram(sizeof(macRxMemPool_t));
	if (mac_p->macRxMemPool_p == NULL)
#endif
		mac_p->macRxMemPool_p = (macRxMemPool_t *) dma_alloc_coherent(NULL, sizeof(macRxMemPool_t), &mac_p->macRxMemPool_phys_p, GFP_KERNEL);
	if (mac_p->macRxMemPool_p == NULL) {
		goto out;
	}
#ifdef CONFIG_TC3162_DMEM
	mac_p->macTxMemPool_p = (macTxMemPool_t *) alloc_sram(sizeof(macTxMemPool_t));
	if(mac_p->macTxMemPool_p == NULL)
#endif
		mac_p->macTxMemPool_p = (macTxMemPool_t *) dma_alloc_coherent(NULL, sizeof(macTxMemPool_t), &mac_p->macTxMemPool_phys_p, GFP_KERNEL);
	if (mac_p->macTxMemPool_p == NULL) {
		goto out;
	}
	mac_p->rxDescrSize = MAC_RXDESCP_SIZE;/* ----- Set up the paramters ----- */
	mac_p->txDescrSize = MAC_TXDESCP_SIZE;
	mac_p->rxRingSize  = MAC_RXDESCP_NO;
	mac_p->txRingSize  = MAC_TXDESCP_NO;
	mac_p->macStat.inSilicon.rxEnQueueNum = 0;
	mac_p->macStat.inSilicon.rxDeQueueNum = 0;
	mac_p->macStat.inSilicon.txEnQueueNum = 0;
	mac_p->macStat.inSilicon.txDeQueueNum = 0;
	ret_val = macDrvDescripInit(mac_p); /* ----- Initialize Tx/Rx descriptors ----- */
	if(0 != ret_val) {
		goto out;
	}

	/* ----- Initialize the phy chip ----- */
	if (mt751x_macSetUpPhy(mac_p)) {
		printk(KERN_ERR "failed while macSetUpPhy!!!\n");
		goto out;
	}

	macSetMacReg(dev->dev_addr);/* --- setup MAC address --- */
	reg = RX_COHERENT | RX_DLY_INT | TX_COHERENT | RX_DONE_INT1 | RX_DONE_INT0;
	reg_write32(INT_MASK, reg);/* ----- Initialize interrupt mask ----- */
	macSetDMADescrCtrlReg(mac_p);
	macSetMACCR();
	macSetGSW();
	macSearchPhyAddr();
	//    mt7510FECfgLoad();
out:
	return ret_val;
}

struct PDMA_txdesc *macTxRingProc(struct macAdapter *mac_p, unsigned int txq)
{
	volatile struct PDMA_txdesc *pTxDescp;
	volatile struct PDMA_txdesc pTxDescpTmpVal;
	volatile struct PDMA_txdesc *pTxDescpTmp = &pTxDescpTmpVal;
	unsigned long flags;
	struct sk_buff *freeskb;

	spin_lock_irqsave(&mac_p->lock, flags);
	pTxDescp = ((struct PDMA_txdesc*)mac_p->txDescrRingBaseVAddr[txq]) + mac_p->txUnReleasedDescp[txq];
	pTxDescpTmp = pTxDescp;
	while(mac_p->txUnReleasedBufCnt[txq] != 0) {
		if (!pTxDescpTmp->txd_info2.bits.DDONE_bit) {
			spin_unlock_irqrestore(&mac_p->lock, flags);
			return 0;
		}
		freeskb = mac_p->txskbs[txq][mac_p->txUnReleasedDescp[txq]];
		dev_kfree_skb_any(freeskb);
		mac_p->txskbs[txq][mac_p->txUnReleasedDescp[txq]] = NULL;
		if(mac_p->txUnReleasedDescp[txq] == (mac_p->txRingSize - 1)) {
			mac_p->txUnReleasedDescp[txq] = 0;
		} else {
			mac_p->txUnReleasedDescp[txq]++;
		}
		mac_p->txUnReleasedBufCnt[txq]--;
		pTxDescp = ((struct PDMA_txdesc*)mac_p->txDescrRingBaseVAddr[txq]) + mac_p->txUnReleasedDescp[txq];
		pTxDescpTmp = pTxDescp;
		mac_p->macStat.inSilicon.txDeQueueNum++;
	}
	spin_unlock_irqrestore(&mac_p->lock, flags);

	return (struct PDMA_txdesc*)pTxDescp;
}

__IMEM int macRxRingProc(int quota)
{
	volatile struct PDMA_rxdesc *rxDescrp;
	volatile struct PDMA_rxdesc rxDescrpTmpVal;
	volatile struct PDMA_rxdesc *rxDescrpTmp = &rxDescrpTmpVal;
	unsigned int frameSize;
	int npackets = 0;
	struct sk_buff *newskb;
	struct sk_buff *skb;
	struct device_priv *priv;
	unsigned int rx_dma_index;
	unsigned short rxd_num;

	rxDescrp = ((struct PDMA_rxdesc*)mac_p->rxDescrRingBaseVAddr) + mac_p->rxCurrentDescp;
	rxDescrpTmp = rxDescrp;
	while((rxDescrpTmp->rxd_info2.bits.DDONE_bit) && (npackets < quota)) {
		npackets++;
#if defined(TCSUPPORT_MT7510_FE)
      if((npackets % mac_receive_num) != 0){ //prevent alwaysdrop
             rx_dma_index = read_reg_word(RX_DRX_IDX(0));

             if(mac_p->rxCurrentDescp <= rx_dma_index){
                    rxd_num = rx_dma_index - mac_p->rxCurrentDescp;
              }else{
                     rxd_num = rx_dma_index + MAC_RXDESCP_NO - mac_p->rxCurrentDescp;
              }

             if( rxd_num > mac_receive_threshold){
                      mac_p->macStat.MIB_II.inDiscards++;
                     // Discard this packet & Repost this mbuf
                     newskb = mac_p->rxskbs[mac_p->rxCurrentDescp];
                     goto DISCARD;
             }
     }
#endif

		frameSize = rxDescrpTmp->rxd_info2.bits.PLEN0;
		if(unlikely((frameSize < 20) || (frameSize > RX_MAX_PKT_LEN))){/* lino: make VLAN friendly */
			mac_p->macStat.inSilicon.rxEtherFrameLengthErr++;
			newskb = mac_p->rxskbs[mac_p->rxCurrentDescp];// Discard this packet & Repost this mbuf
			goto DISCARD;
		}
		skb = mac_p->rxskbs[mac_p->rxCurrentDescp];
		newskb = skbmgr_dev_alloc_skb2k();
		if(unlikely(!newskb)) { /* faild to allocate more mbuf -> drop this pkt */
			newskb = skb;
			mac_p->macStat.MIB_II.inDiscards++;
			goto RECVOK;
		}
		dma_cache_inv((unsigned long)(newskb->data), RX_MAX_PKT_LEN);
		skb_reserve(newskb, NET_IP_ALIGN);
		skb_put(skb, frameSize);
#ifdef CONFIG_ETHERNET_DEBUG
		if(dump_mask & DUMP_RX_TAG) {
			dump_skb(skb, "RX_TAGGED");
		}
#endif
		skb->dev = mt751x_vlan_tag_remove(skb);
#ifdef CONFIG_ETHERNET_DEBUG
		if(dump_mask & DUMP_RX_UNTAG) {
			dump_skb(skb, "RX_UNTAGGED");
		}
#endif
		if(mac_p->statisticOn) {// ----- Count the MIB-II -----
			mac_p->macStat.MIB_II.inOctets += frameSize;
			if (*skb->data & 0x01) {
				mac_p->macStat.MIB_II.inMulticastPkts++;
			} else {
				mac_p->macStat.MIB_II.inUnicastPkts++;
			}
		}
#if defined(CONFIG_TC3162_ADSL) && defined(TC_CONSOLE_ENABLE)
		if(tcconsole_proc(skb) != 1)
#endif
		{
#ifdef RAETH_CHECKSUM_OFFLOAD
			if(((rxDescrpTmp->rxd_info4.bits.SPORT != GDM_P_QDMA) && (rxDescrpTmp->rxd_info4.bits.SPORT != GDM_P_CPU)) &&
					(((rxDescrpTmp->rxd_info4.bits.PKT_INFO & IPV4_H ) == IPV4_H) || ((rxDescrpTmp->rxd_info4.bits.PKT_INFO & IPV6_H ) == IPV6_H)) &&
					(((rxDescrpTmp->rxd_info4.bits.PKT_INFO & TU_H_C_INV ) == 0) && ((rxDescrpTmp->rxd_info4.bits.PKT_INFO & IPV4_H_INV ) == 0))) {
				skb->ip_summed = CHECKSUM_UNNECESSARY;
			} else
#endif
				skb->ip_summed = CHECKSUM_NONE;
			skb->protocol = eth_type_trans(skb, skb->dev);
			skb->dev->last_rx = jiffies;
#ifdef CONFIG_ETHERNET_DEBUG
			if(dump_mask & DUMP_RX_TAG) {
				dump_skb(skb, "RX_BEFORE_HWNAT");
			}
#endif
#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
			priv = (struct device_priv *)netdev_priv(skb->dev);
			if(priv && priv->phy_id == SWITCH_PORT4)
			{
				tbs_led_data_blinking(led_internet_green);
				tbs_led_data_blinking(led_lan_5);
			}
#endif
#ifdef TCSUPPORT_RA_HWNAT
			if(ra_sw_nat_hook_rxinfo) {
				ra_sw_nat_hook_rxinfo(skb, FOE_MAGIC_GE, (char *)&rxDescrpTmp->rxd_info4, sizeof(PDMA_RXD_INFO4_T));
			}
#endif
#ifdef CONFIG_TCSUPPORT_RA_HWNAT
			if((NULL == ra_sw_nat_hook_rx)
#ifdef TCSUPPORT_RA_HWNAT
					|| ((NULL != ra_sw_nat_hook_rx) && (0 != ra_sw_nat_hook_rx(skb)))
#endif
			  )
#endif
			{
				netif_receive_skb(skb);
			}
		}
DISCARD:
RECVOK:
		rxDescrpTmp->rxd_info1.bits.PDP0 = K1_TO_PHY(newskb->data);
		mac_p->rxskbs[mac_p->rxCurrentDescp] = newskb;
		rxDescrpTmp->rxd_info2.word = 0;
		rxDescrpTmp->rxd_info2.bits.LS0 = 1;
		rxDescrpTmp->rxd_info2.bits.PLEN0  = 1536;
		wmb();
		reg_write32(RX_CALC_IDX(0), mac_p->rxCurrentDescp);
		mac_p->rxCurrentDescp = (mac_p->rxCurrentDescp + 1) % mac_p->rxRingSize;/* next descriptor*/
		rxDescrp = ((struct PDMA_rxdesc*)mac_p->rxDescrRingBaseVAddr) + mac_p->rxCurrentDescp;
		rxDescrpTmp = rxDescrp;
	}

	return npackets;
}

int mt751x_mac_tx(struct sk_buff *skb, struct net_device *dev)
{
	volatile struct PDMA_txdesc *currDescrp = NULL;
	volatile struct PDMA_txdesc currDescrpTmpVal;
	volatile struct PDMA_txdesc *currDescrpTmp = &currDescrpTmpVal;
	unsigned int length, data_len;
	unsigned char *bufAddrp;
	unsigned long flags;
	unsigned int txq = 0;
	struct sk_buff *nskb = NULL;
	struct device_priv *priv = NULL;
	struct net_device_stats *stats = NULL;
#if defined(TCSUPPORT_RA_HWNAT) && defined(TCSUPPORT_MT7510_FE)
	struct port_info eth_info;
#endif

#ifdef TC_CONSOLE_ENABLE
	if(skb->data[12] == 0xaa) {/* isTCConsolePkt */
		skb->mark |= SKBUF_TCCONSOLE;
	}
#endif
	data_len = skb->len;
	if(NULL != dev) {
		priv = (struct device_priv *)netdev_priv(dev);
		if(priv->phy_id < SWITCH_PORT_MAX) {
			stats = &priv->stat;
#ifdef CONFIG_ETHERNET_DEBUG
			if(dump_mask & DUMP_TX_UNTAG) {
				dump_skb(skb, "TX_UNTAGGED");
			}
#endif
		#ifdef TCSUPPORT_RA_HWNAT
		if (ra_sw_nat_hook_magic) {
			if (ra_sw_nat_hook_magic(skb, FOE_MAGIC_PPE) == 0) {
				nskb = mt751x_vlan_tag_insert(skb, dev);
				if(NULL == nskb) {
					if(NULL != stats) {
						stats->tx_dropped++;
					}
					goto out;
				} else {
					skb = nskb;
				}
			}
		}
		#endif
#ifdef CONFIG_ETHERNET_DEBUG
			if(dump_mask & DUMP_TX_TAG) {
				dump_skb(skb, "TX_TAGGED");
			}
#endif
		}
	}
#ifdef TCSUPPORT_QOS
	switch (qos_flag) {
		case QOS_SW_PQ:/* PQ mode */
			if (txq < 2 && (skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				txq = 2;
			} else if (txq < 1 && (skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY) {
				txq = 1;
			}
			break;

		case QOS_HW_WRR:/* HW WRR mode */
			if((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				txq = 3;
			} else if ((skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY) {
				txq = 2;
			} else if ((skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY) {
				txq = 1;
			} else {
				txq = 0;
			}
			break;

		case QOS_HW_PQ:/* HW PQ mode */
			if (txq < 3 && (skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				txq = 3;
			} else if (txq < 2 && (skb->mark & QOS_FILTER_MARK) == QOS_H_PRIORITY) {
				txq = 2;
			} else if (txq < 1 && (skb->mark & QOS_FILTER_MARK) == QOS_M_PRIORITY) {
				txq = 1;
			}
			break;

		case NULLQOS: /*It's for putting rtp packets to HH priority when qos_flag not be selected as WRR or PQ*/
			if((skb->mark & QOS_FILTER_MARK) == QOS_HH_PRIORITY) {
				txq = 3;
			}
			break;

		default:
			break;
	}
#endif
#ifdef QOS_REMARKING
	if((skb->mark & QOS_REMARKING_FLAG)){
		txq = (unsigned char)((skb->mark & QOS_REMARKING_MASK) >> 1);
	}
#endif
#ifdef TCSUPPORT_RA_HWNAT
	if (ra_sw_nat_hook_tx != NULL) {
#ifdef TCSUPPORT_MT7510_FE
		eth_info.word = 0; //clean value
		eth_info.pdma_eth.txq = (txq & 0xf);

		if(priv && priv->phy_id == SWITCH_PORT4)
			eth_info.pdma_eth.is_wan = 1;
		else
			eth_info.pdma_eth.is_wan = 0;

		//eth_info.pdma_eth.is_wan = (is_wan_packet & 0x1);
		if (ra_sw_nat_hook_tx(skb, &eth_info, FOE_MAGIC_GE) == 0) {
#else
			if (ra_sw_nat_hook_txq)
				ra_sw_nat_hook_txq(skb, txq);
			if (ra_sw_nat_hook_tx(skb, 1) == 0) {
#endif
				dev_kfree_skb_any(skb);
				goto out;
			}
		}
#endif
	bufAddrp = skb->data;
	if(unlikely(skb->len < ETH_ZLEN)) {
		if(skb_padto(skb, ETH_ZLEN)) {
			mac_p->macStat.MIB_II.outDiscards++;
			if(NULL != stats) {
				stats->tx_dropped++;
			}
			goto out;
		}
		skb->len = ETH_ZLEN;
	}
	length = skb->len;
	if(mac_p->txUnReleasedBufCnt[txq] >= TX_BUF_RELEASE_THRESHOLD) {
		macTxRingProc(mac_p, txq);
	}
	//need protect this count read before count add. shnwind .
	spin_lock_irqsave(&mac_p->lock, flags);
	if(mac_p->txUnReleasedBufCnt[txq] == mac_p->txRingSize - 1) {
		mac_p->macStat.MIB_II.outDiscards++;
		if(NULL != stats) {
			stats->tx_dropped++;
		}
		spin_unlock_irqrestore(&mac_p->lock, flags);
		dev_kfree_skb_any(skb);
		goto out;
	}
	/* ----- Count the MIB-II ----- */
	mac_p->macStat.MIB_II.outOctets += length;
	if(NULL != stats) {
		stats->tx_packets++;
		stats->tx_bytes += data_len;
	}
	if (*bufAddrp & 0x01)
		mac_p->macStat.MIB_II.outMulticastPkts++;
	else
		mac_p->macStat.MIB_II.outUnicastPkts++;
	dma_cache_wback_inv((unsigned long)(skb->data), length);
	/* ----- Get the transmit descriptor ----- */
	currDescrp = ((struct PDMA_txdesc *) mac_p->txDescrRingBaseVAddr[txq]) + mac_p->txCurrentDescp[txq];
	currDescrpTmp = currDescrp;
	if(!currDescrpTmp->txd_info2.bits.DDONE_bit) {
		if(NULL != stats) {
			stats->tx_dropped++;
		}
		mac_p->macStat.MIB_II.outDiscards++;
		dev_kfree_skb_any(skb);
		spin_unlock_irqrestore(&mac_p->lock, flags);
		goto out;
	}

	/* tx buffer size */
	currDescrpTmp->txd_info1.bits.SDP0 = K1_TO_PHY(skb->data);
	currDescrpTmp->txd_info2.bits.SDL0 = length;
	currDescrpTmp->txd_info4.word = 0;
	currDescrpTmp->txd_info4.bits.PN = GDM_P_GDMA1;/* GDMA1 */

#ifdef TCSUPPORT_RA_HWNAT
	if (ra_sw_nat_hook_magic) {
		if (ra_sw_nat_hook_magic(skb, FOE_MAGIC_PPE)) {/* PPE */
			currDescrpTmp->txd_info4.bits.PN = 4;//in femac_7510.h is 4 //6;
		}
	}
#endif
#if VLAN_TAG_USED
	if (vlan_tx_tag_present(skb)) {
		unsigned short vlan_tag = cpu_to_be16(vlan_tx_tag_get(skb));
		currDescrpTmp->txd_info4.bits.VIDX = (vlan_tag & 0xfff);
		currDescrpTmp->txd_info4.bits.VPRI = (vlan_tag>>13)&0x7;
		currDescrpTmp->txd_info4.bits.INSV = 1;
	} else {
		currDescrpTmp->txd_info4.bits.INSV = 0;
	}
#endif
#ifdef RAETH_CHECKSUM_OFFLOAD
	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		currDescrpTmp->txd_info4.bits.TCO = 1;
		currDescrpTmp->txd_info4.bits.UCO = 1;
		currDescrpTmp->txd_info4.bits.ICO = 1;
	}
#endif
	mac_p->txskbs[txq][mac_p->txCurrentDescp[txq]] = skb;
	currDescrpTmp->txd_info2.bits.DDONE_bit = 0;
	mac_p->txCurrentDescp[txq] = (mac_p->txCurrentDescp[txq] + 1) % mac_p->txRingSize;
	wmb();
	reg_write32(TX_CTX_IDX(txq), mac_p->txCurrentDescp[txq]);
	mac_p->txUnReleasedBufCnt[txq]++;
	mac_p->macStat.inSilicon.txEnQueueNum++;
	spin_unlock_irqrestore(&mac_p->lock, flags);
out:
#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
	if(priv && priv->phy_id == SWITCH_PORT4)
	{
		tbs_led_data_blinking(led_internet_green);
		tbs_led_data_blinking(led_lan_5);
	}
#endif
	return NETDEV_TX_OK;
}

static irqreturn_t mt751x_esw_isr(int irq, void *dev_id)
{
	unsigned int reg;

	reg = reg_read32(GSW_CFG_ISR);
	reg_write32(GSW_CFG_ISR, reg);
	if(0 != (reg & 0x7F)) {
		tasklet_schedule(&mac_p->lk_task);
	}
	return IRQ_HANDLED;
}

static void mt751x_esw_dsr(unsigned long v)
{
	unsigned long flags;
	unsigned int reg;
	unsigned int link = 0;
	unsigned int phy_id;
	struct net_device *dev = NULL;
	struct device_priv *priv = NULL;

	// spin_lock_irqsave(&(mac_p->lock), flags);
	spin_lock_irqsave(&gimr_lock, flags);
	for(phy_id = 0; phy_id < SWITCH_PORT_MAX; phy_id++) {
		dev = switch_dev[phy_id];
		priv = netdev_priv(dev);
		reg = reg_read32(GSW_PMSR(phy_id));//Interrupt Status Register
		link = reg & IFF_UP;
		if(link != priv->link.LinkStatus) {
			if(link) {/* 0:DOWN, 1:UP  */
				priv->link.LinkStatus = 1;
				priv->link.Duplex = (reg >> 1) & DUPLEX_FULL;/* 0: Half Duplex. 1: Full Duplex. */
				priv->link.Speed = SPEED_10;
				switch((reg >> 2) & 3) {
					case 2:/* 2:1000Mbps */
						priv->link.Speed *= 10;
					case 1:/* 1:100Mbps */
						priv->link.Speed *= 10;
					default:/* 0:10Mbps */
						break;
				}
				printk("Port %d Link UP at %d Mbps %s Duplex!\n", phy_id, priv->link.Speed, (priv->link.Duplex) ? "Full" : "Half");
#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
				if(priv->phy_id == SWITCH_PORT4)
				{
					tbs_led_trigger_set(led_lan_5, led_on_trig);
					tbs_led_trigger_set(led_lan_5, led_blinking_trig);
				}
#endif
				netif_carrier_on(dev);
			} else {
				printk("Port %d Link Down!\n", phy_id);
#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
				if(priv->phy_id == SWITCH_PORT4)
					tbs_led_trigger_set(led_lan_5, led_off_trig);
#endif
				netif_carrier_off(dev);
				memset(&priv->link, 0x00, sizeof(struct phy_link_state));
			}
		}
	}
	// spin_unlock_irqrestore(&(mac_p->lock), flags);
	spin_unlock_irqrestore(&gimr_lock, flags);
}


static irqreturn_t mt751x_mac_isr(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;
	unsigned int reg;

	reg = reg_read32(INT_STATUS);
	reg_write32(INT_STATUS, reg);
	if (reg & (RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0)) {// ----------Packet Received----------------------
		spin_lock(&gimr_lock);
		if (napi_schedule_prep(&mac_p->napi)) {
			reg_write32(INT_MASK, reg_read32(INT_MASK) & ~(RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0));
			__napi_schedule(&mac_p->napi);
		}
		spin_unlock(&gimr_lock);
	}
	if(reg & (RX_COHERENT | TX_COHERENT)) {
		printk("%s err mac_isr INT_STATUS=%08x\n", dev->name, reg);
	}

	return IRQ_HANDLED;
}

/* Starting up the ethernet device */
static int mt751x_mac_open(struct net_device *dev)
{
	struct device_priv *priv = netdev_priv(dev);

	netif_carrier_off(dev);
	PHY_power_ops(priv->phy_id, led_on_trig);
	PHY_led_ops(priv->phy_id, led_blinking_trig);
	netif_start_queue(dev);

	return 0;
}

/* Stopping the ethernet device */
static int mt751x_mac_close(struct net_device *dev)
{
	struct device_priv *priv = netdev_priv(dev);

	netif_stop_queue(dev);
	PHY_power_ops(priv->phy_id, led_off_trig);

	return 0;
}

/* Setting customized mac address */
int mt751x_mac_set_macaddr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	/* Check if given address is valid ethernet MAC address */
	if(!is_valid_ether_addr(addr->sa_data)) {
		return(-EIO);
	}
	/* Save the customize mac address */
	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
	macSetMacReg(addr->sa_data);

	return 0;
}

/* Get the stats information */
static struct net_device_stats *mt751x_mac_stats(struct net_device *dev)
{
	struct net_device_stats *stats;
	struct device_priv *priv = netdev_priv(dev);

	stats = &priv->stat;

	stats->rx_packets = mac_p->macStat.MIB_II.inUnicastPkts + mac_p->macStat.MIB_II.inMulticastPkts;
	stats->tx_packets = mac_p->macStat.MIB_II.outUnicastPkts + mac_p->macStat.MIB_II.outMulticastPkts;
	stats->rx_bytes = mac_p->macStat.MIB_II.inOctets;
	stats->tx_bytes = mac_p->macStat.MIB_II.outOctets;
	stats->rx_dropped = mac_p->macStat.MIB_II.inDiscards;
	stats->tx_dropped = mac_p->macStat.MIB_II.outDiscards;
	stats->multicast = mac_p->macStat.MIB_II.inMulticastPkts;
	stats->rx_errors = mac_p->macStat.MIB_II.inErrors;
	stats->tx_errors = mac_p->macStat.MIB_II.outErrors;
	stats->collisions = mac_p->macStat.inSilicon.txExCollisionCnt + mac_p->macStat.inSilicon.txCollisionCnt + mac_p->macStat.inSilicon.rxCollisionErr;

	return stats;
}

/* Handling ioctl call */
static int mt751x_mac_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int rc = 0;
	gsw_reg reg;

	switch (cmd) {
		case RAETH_GSW_REG_READ:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			reg.val = reg_read32(CR_GSW_BASE + reg.off);
			copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
			break;
		case RAETH_GSW_REG_WRITE:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			reg_write32(CR_GSW_BASE + reg.off, reg.val);
			break;
		case RAETH_REG_READ:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			reg.val = reg_read32(reg.off);
			copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
			break;
		case RAETH_REG_WRITE:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			reg_write32(reg.off, reg.val);
			break;
		default:
			rc = generic_mii_ioctl(&mac_p->mii_if, if_mii(ifr), cmd, NULL);
			break;
	}

	return rc;
}

static int mt751x_mac_poll(struct napi_struct *napi, int budget)
{
	int rx_work_limit=0;
	int received = 0;
	int n=0;
	unsigned int reg;
	unsigned long flags=0;

	rx_work_limit = min(budget - received, budget);
	n = macRxRingProc(rx_work_limit);
	received += n;
	if(received < budget) {
		spin_lock_irqsave(&gimr_lock, flags);
		__napi_complete(napi);
		reg = reg_read32(INT_MASK);
		reg |= (RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0);
		reg_write32(INT_MASK, reg);
		spin_unlock_irqrestore(&gimr_lock, flags);
	}

	return received;
}

static int mt751x_mac_start(struct net_device *dev)
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31))
#else
#if VLAN_TAG_USED
	dev->features |= NETIF_F_HW_VLAN_TX;
#endif
#ifdef RAETH_CHECKSUM_OFFLOAD
	dev->features |= NETIF_F_IP_CSUM;
#endif
#endif
	/*
	   dev->tx_queue_len = MAC_TXDESCP_NO;
	   dev->flags &= ~IFF_MULTICAST;
	   dev->flags |= IFF_DEBUG;
	   */

	return 0;
}

static const struct net_device_ops gmac_netdev_ops = {
	.ndo_init				= mt751x_mac_start,
	.ndo_open				= mt751x_mac_open,
	.ndo_stop 				= mt751x_mac_close,
	.ndo_start_xmit			= mt751x_mac_tx,
	.ndo_get_stats			= mt751x_mac_stats,
	.ndo_do_ioctl			= mt751x_mac_ioctl,
	.ndo_change_mtu			= eth_change_mtu,
	.ndo_set_mac_address 	= mt751x_mac_set_macaddr,
	.ndo_validate_addr		= eth_validate_addr,
};

static void mt751x_vlan_vid_cfg(unsigned char index, unsigned short vid, unsigned char portmap)
{
	unsigned int value = 0;
	unsigned int reg;

	reg = reg_read32(CR_GSW_BASE + 0x100 + ((index >> 1) << 2));
	if(index % 2) {
		reg &= 0xFFF;
		reg |= (vid << 12);
	} else {
		reg &= 0xFFF000;
		reg |= vid;
	}
	reg_write32((CR_GSW_BASE + 0x100 + ((index >> 1) << 2)), reg);
	value = ((portmap | (1 << 6)) << 16);/* VLAN member including port 6(CPU port) */
	value |= BIT(30);/* Independent VLAN Learning */
	value |= BIT(0);/* Make this entry active */
	reg_write32(GSW_VAWDI, value);/* write VLAN config data */
	reg = BIT(31) | (0x01 << 12) | vid; /* Write VID entry */
	reg_write32(GSW_VTCR, reg);
	do {
		reg = reg_read32(GSW_VTCR);
	} while(reg & BIT(31));/* make sure VLAN table is no longer busy */
	reg = 0x8002;
	reg_write32(GSW_ATC, reg);/* Clear address table */
	mdelay(5);
}

static void mt751x_vlan_init(void)
{
	unsigned int reg;
	int i;

	for(i = 0; i < SWITCH_PORT4; i++) {/* Set Pvid */
		reg_write32(GSW_PPBV1(i), 0x10001);
	}
	reg_write32(GSW_PPBV1(4), 0x10002);
	reg_write32(GSW_PPBV1(6), 0x20001);
	reg = 0x8002;
	reg_write32(GSW_ATC, reg);/* Clear address table */
	mdelay(5);
	mt751x_vlan_vid_cfg(0, 1, 0xF);
	mt751x_vlan_vid_cfg(1, 2, BIT(4));
	for(i = 0; i < SWITCH_PORT_MAX; i++) {
		reg = reg_read32(GSW_PCR(i));
		reg &= ~(0x03 << 28);
		reg |= 0x03;
		reg_write32(GSW_PCR(i), reg);
	}
	reg = reg_read32(GSW_PCR(6));
	reg &= ~(0x03 << 28);
	reg |= (0x02 << 28);
	reg |= 0x03;
	reg_write32(GSW_PCR(6), reg);
}

static u32 gswPbusRead(u32 pbus_addr)
{
	u32 pbus_data;
	u32 phyaddr;
	u32 reg;
	u32 value;

	spin_lock_bh(&pbus_lock);

	phyaddr = 31;
	// 1. write high-bit page address
	reg = 31;
	value = (pbus_addr >> 6);
	tcMiiStationWrite(phyaddr, reg, value);
	/* mdelay(5); */
	/*
	 * DBG_PRINTF("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */
	/*
	 * printk("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */

	// 2. read low DWord
	reg = (pbus_addr>>2) & 0x000f;
	value = tcMiiStationRead(phyaddr, reg);
	/* mdelay(5); */
	/*
	 * DBG_PRINTF("2. miir phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */
	/*
	 * printk("2. miir phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */
	pbus_data = value;

	// 3. read high DWord
	reg = 16;
	value = tcMiiStationRead(phyaddr, reg);
	/* mdelay(5); */
	/*
	 * DBG_PRINTF("3. miir phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */
	/*
	 * printk("3. miir phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */

	pbus_data = (pbus_data) | (value<<16);

	/*
	 * DBG_PRINTF("# pbus read addr=0x%04x data=0x%04x_%04x\r\n",
	 *         (pbus_addr&0xfffc), (pbus_data>>16), (pbus_data&0xffff));
	 */
	/*
	 * printk("# pbus read addr=0x%04x data=0x%04x_%04x\r\n",
	 *         (pbus_addr&0xfffc), (pbus_data>>16), (pbus_data&0xffff));
	 */
	/* printk("gswPbusRead read data:\n"); */

	/* printk("pbus_data: %x\n", pbus_data); */

	spin_unlock_bh(&pbus_lock);
	return pbus_data;
} /* end frank modify for rt62806 */

/* frank modify for rt62806 */
#if 0
static int gswPbusWrite(u32 pbus_addr, u32 pbus_data)
{
	u32 phyaddr;
	u32 reg;
	u32 value;

	spin_lock_bh(&pbus_lock);

	phyaddr = 31;

	// 1. write high-bit page address
	reg = 31;
	value = (pbus_addr >> 6);
	tcMiiStationWrite(phyaddr, reg, value);
	//mdelay(5);

	/*
	 * printk("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */
	/*
	 * DBG_PRINTF("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */

	// 2. write low DWord
	reg = (pbus_addr>>2) & 0x000f;
	value = pbus_data & 0xffff;
	tcMiiStationWrite(phyaddr, reg, value);
	/* mdelay(5); */
	/*
	 * printk("2. miiw phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */
	/*
	 * DBG_PRINTF("2. miiw phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */

	// 3. write high DWord
	reg = 16;
	value = (pbus_data>>16) & 0xffff;
	tcMiiStationWrite(phyaddr, reg, value);
	/* mdelay(5); */
	/*
	 * printk("3. miiw phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */
	/*
	 * DBG_PRINTF("3. miiw phyaddr=%2d reg=%2d value=%04x\r\n",
	 *         phyaddr, reg, value);
	 */

	/*
	 * DBG_PRINTF("# pbus write addr=0x%04x data=0x%04x_%04x\r\n",
	 *         (pbus_addr&0xfffc), (pbus_data>>16),(pbus_data&0xffff));
	 */
	spin_unlock_bh(&pbus_lock);

	return 0;
} /* end frank modify for rt62806 */

//PHY2 read/write
static u32 gswPmiRead(u32 phy_addr, u32 phy_reg)
{
	u32 pbus_addr;
	u32 pbus_data;
	u32 phy_data;
	u32 phy_acs_st;
	/* u32 max_wait_cnt = 1000; */

	pbus_addr = 0x701c;
	/*
	 * b31	- phy access 1:start&busy, 0:complete&idle
	 * b29:25 - mdio phy reg addr
	 * b24:20 - mdio phy addr
	 * b19:18 - 2'b01: write, 2'b10: read
	 * b17:16 - start field, always 2'b01
	 * b15:0	- data
	 */

	phy_addr = phy_addr & 0x1f;
	phy_reg  = phy_reg & 0x1f;

	// 1. write phy_addr & phy_reg
	pbus_data = 0x80090000; // read
	pbus_data = pbus_data | (phy_addr<<20);
	pbus_data = pbus_data | (phy_reg<<25);

	gswPbusWrite(pbus_addr,pbus_data);

	// 2. check phy_acs_st
	phy_acs_st = 1;
	while (phy_acs_st) {
		pbus_data = gswPbusRead(pbus_addr);
		phy_acs_st = (pbus_data>>31) & 0x1;
	}

	// 3. return data
	phy_data = pbus_data & 0xffff;
	return phy_data;
} /* end frank modify for rt62806 */

static u32 gswPmiWrite(u32 phy_addr, u32 phy_reg, u32 phy_data)
{
	u32 pbus_addr;
	u32 pbus_data;
	/* u32 phy_acs_st; */

	pbus_addr = 0x701c;
	/*
	 * b31    - phy access 1:start&busy, 0:complete&idle
	 * b29:25 - mdio phy reg addr
	 * b24:20 - mdio phy addr
	 * b19:18 - 2'b01: write, 2'b10: read
	 * b17:16 - start field, always 2'b01
	 * b15:0  - data
	 */

	phy_addr = phy_addr & 0x1f;
	phy_reg  = phy_reg & 0x1f;
	phy_data = phy_data & 0xffff;

	// 1. write phy_addr & phy_reg & phy_data
	pbus_data = 0x80050000; // write
	pbus_data = pbus_data | (phy_addr<<20);
	pbus_data = pbus_data | (phy_reg<<25);
	pbus_data = pbus_data | (phy_data);

	gswPbusWrite(pbus_addr,pbus_data);
	/*
	 * DBG_PRINTF(" pbus write addr=0x%04x data=0x%08x\r\n",
	 *         pbus_addr, pbus_data);
	 */

	return 0;
} /* end frank modify for rt62806 */
#endif

static int gsw_mibN_read_proc(char *buf, char **start, off_t off, int count,
		int *eof, void *data, uint portId)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int port;

	/*
	 * this interface is _ONLY_ called by MIB proc entry for now,
	 * and MIB proc entry is create after mac init, so we
	 * _JUST_ remove this condition
	 * --Camel Luo
	 */
#if 0
	if (!macInitialized) {
		*eof = 1;
		return 0;
	}
#endif

	port = portId;
#ifdef TCSUPPORT_MT7510_FE
	index += sprintf(buf+index, "[ Port %d ]\n", port);
	CHK_BUF();
	if (isMT7520G || isMT7525G) {
		index += sprintf(buf+index, "Rx Unicase Pkts        = 0x%08x, ",
				gswPbusRead(EXT_GSW_RX_UNIC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Multicast Pkts      = 0x%08x\n",
				gswPbusRead(EXT_GSW_RX_MULC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Broadcast Pkts      = 0x%08x, ",
				gswPbusRead(EXT_GSW_RX_BROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Align Error         = 0x%08x\n",
				gswPbusRead(EXT_GSW_RX_ALIGE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx CRC Error           = 0x%08x, ",
				gswPbusRead(EXT_GSW_RX_CRC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Under Size Pkts     = 0x%08x\n",
				gswPbusRead(EXT_GSW_RX_RUNT(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Fragment Error      = 0x%08x, ",
				gswPbusRead(EXT_GSW_RX_FRGE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Over Size Pkts      = 0x%08x\n",
				gswPbusRead(EXT_GSW_RX_LONG(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Jabber Error        = 0x%08x, ",
				gswPbusRead(EXT_GSW_RX_JABE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Pause Pkts          = 0x%08x\n",
				gswPbusRead(EXT_GSW_RX_PAUC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Drop Pkts           = 0x%08x, ",
				gswPbusRead(EXT_GSW_RX_DROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx ING Drop Pkts       = 0x%08x\n",
				gswPbusRead(EXT_GSW_RX_INGC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx ARL Drop Pkts       = 0x%08x, ",
				gswPbusRead(EXT_GSW_RX_ARLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx FILTER Drop Pkts    = 0x%08x\n",
				gswPbusRead(EXT_GSW_RX_FILC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Unicase Pkts        = 0x%08x, ",
				(u32)read_reg_word(GSW_TX_UNIC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Multicast Pkts      = 0x%08x\n",
				gswPbusRead(EXT_GSW_TX_MULC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Broadcast Pkts      = 0x%08x, ",
				gswPbusRead(EXT_GSW_TX_BROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Collision           = 0x%08x\n",
				gswPbusRead(EXT_GSW_TX_COLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Single Collision    = 0x%08x, ",
				gswPbusRead(EXT_GSW_TX_SCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Multiple Collision  = 0x%08x\n",
				gswPbusRead(EXT_GSW_TX_MCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Defer               = 0x%08x, ",
				gswPbusRead(EXT_GSW_TX_DEFC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Late Collision      = 0x%08x\n",
				gswPbusRead(EXT_GSW_TX_LCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx eXcessive Collision = 0x%08x, ",
				gswPbusRead(EXT_GSW_TX_ECOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Pause Pkts          = 0x%08x\n",
				gswPbusRead(EXT_GSW_TX_PAUC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Drop Pkts           = 0x%08x\n",
				gswPbusRead(EXT_GSW_TX_DROC(port)));
		CHK_BUF();
	} else {
		index += sprintf(buf+index, "Rx Unicase Pkts        = 0x%08x, ",
				(u32)read_reg_word(GSW_RX_UNIC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Multicast Pkts      = 0x%08x\n",
				(u32)read_reg_word(GSW_RX_MULC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Broadcast Pkts      = 0x%08x, ",
				(u32)read_reg_word(GSW_RX_BROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Align Error         = 0x%08x\n",
				(u32)read_reg_word(GSW_RX_ALIGE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx CRC Error           = 0x%08x, ",
				(u32)read_reg_word(GSW_RX_CRC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Under Size Pkts     = 0x%08x\n",
				(u32)read_reg_word(GSW_RX_RUNT(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Fragment Error      = 0x%08x, ",
				(u32)read_reg_word(GSW_RX_FRGE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Over Size PPkts     = 0x%08x\n",
				(u32)read_reg_word(GSW_RX_LONG(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Jabber Error        = 0x%08x, ",
				(u32)read_reg_word(GSW_RX_JABE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Pause Pkts          = 0x%08x\n",
				(u32)read_reg_word((u32)GSW_RX_PAUC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Drop Pkts           = 0x%08x, ",
				(u32)read_reg_word(GSW_RX_DROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx ING Drop Pkts       = 0x%08x\n",
				(u32)read_reg_word(GSW_RX_INGC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx ARL Drop Pkts       = 0x%08x, ",
				(u32)read_reg_word(GSW_RX_ARLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx FILTER Drop Pkts    = 0x%08x\n",
				(u32)read_reg_word(GSW_RX_FILC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Unicase Pkts        = 0x%08x, ",
				(u32)read_reg_word(GSW_TX_UNIC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Multicast Pkts      = 0x%08x\n",
				(u32)read_reg_word(GSW_TX_MULC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Broadcast Pkts      = 0x%08x, ",
				(u32)read_reg_word(GSW_TX_BROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Collision           = 0x%08x\n",
				(u32)read_reg_word(GSW_TX_COLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Single Collision    = 0x%08x, ",
				(u32)read_reg_word(GSW_TX_SCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Multiple Collision  = 0x%08x\n",
				(u32)read_reg_word(GSW_TX_MCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Deffer              = 0x%08x, ",
				(u32)read_reg_word(GSW_TX_DEFC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Late Collision      = 0x%08x\n",
				(u32)read_reg_word(GSW_TX_LCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx eXcessive Collision = 0x%08x, ",
				(u32)read_reg_word(GSW_TX_ECOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Pause Pkts          = 0x%08x\n",
				(u32)read_reg_word(GSW_TX_PAUC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Drop Pkts           = 0x%08x\n",
				(u32)read_reg_word(GSW_TX_DROC(port)));
		CHK_BUF();
	}
#endif

	index += sprintf(buf+index, "------\n");
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

static int gsw_mib0_read_proc(char *buf, char **start, off_t off, int count,
		int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 0);
}

static int gsw_mib1_read_proc(char *buf, char **start, off_t off, int count,
		int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 1);
}

static int gsw_mib2_read_proc(char *buf, char **start, off_t off, int count,
		int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 2);
}

static int gsw_mib3_read_proc(char *buf, char **start, off_t off, int count,
		int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 3);
}

static int gsw_mib4_read_proc(char *buf, char **start, off_t off, int count,
		int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 4);
}

static int gsw_mib5_read_proc(char *buf, char **start, off_t off, int count,
		int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 5);
}

static int gsw_mib6_read_proc(char *buf, char **start, off_t off, int count,
		int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 6);
}

static int mt75x0_mac_get_version(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	int len;

	len = 0;
	len += sprintf(page, "version: %s\n", MT751X_MAC_DEBUG_VER);
	len += sprintf(page+len, "buid time: %s, %s\n", __DATE__, __TIME__);
	len += sprintf(page+len, "for adapt MTK SDK debug Only");

	*start = page + off;
	len -= off;
	if (len > count) {
		len = count;
	}
	if (len < 0) {
		len = 0;
	}
	*eof = 1;

	return len;
}

static int creat_mtk_debug_proc(void)
{
	u8 tmp[32];

	sprintf(tmp, "driver/%s", MT751X_MAC_MOD_NAME);
	proc_mac_dbg = proc_mkdir(tmp, NULL);
	if (!proc_mac_dbg) {
		printk(KERN_ERR "failed while create proc dir \"%s\"\n", tmp);
		return -1;
	}

	//2 1. version
	entry_mac_ver = create_proc_entry("version", 0644, proc_mac_dbg);
	if (!entry_mac_ver) {
		printk(KERN_ERR "failed while create version entry \"%s\"\n", tmp);
		goto ERR_CREAT_MAC_DBG_VER;
	}
	entry_mac_ver->read_proc = mt75x0_mac_get_version;
	entry_mac_ver->write_proc = NULL;
	entry_mac_ver->mode = S_IFREG | S_IRUGO;
	entry_mac_ver->uid = 0;
	entry_mac_ver->gid = 0;
	entry_mac_ver->size = 0;
	printk(KERN_INFO "MAC debug version proc created\n");

	//2 2. eth stats
	sprintf(tmp, "tc3162/eth_stats");
	entry_eth_stats = create_proc_entry(tmp, 0644, NULL);
	if (!entry_eth_stats) {
		printk(KERN_ERR "failed while create Eth stats entry \"%s\"\n", tmp);
		goto ERR_CREAT_MAC_ETH_STATS;
	}
	entry_eth_stats->read_proc = eth_stats_read_proc;
	entry_eth_stats->write_proc = eth_stats_write_proc;
	printk(KERN_INFO "MAC ethernet stats proc created\n");

	//2 3. gsw stats
	sprintf(tmp, "tc3162/gsw_stats");
	entry_eth_gsw = create_proc_entry(tmp, 0644, NULL);
	if (!entry_eth_gsw) {
		printk(KERN_ERR "failed while create version entry \"%s\"\n", tmp);
		goto ERR_CREAT_MAC_GSW_STATS;
	}
	entry_eth_gsw->read_proc = gsw_stats_read_proc;
	entry_eth_gsw->write_proc = gsw_stats_write_proc;
	printk(KERN_INFO "MAC gsw stats proc created\n");

	//2 4. MIB stats
	sprintf(tmp, "tc3162/gsw_mib0");
	entry_eth_gsw_mib0= create_proc_entry(tmp, 0644, NULL);
	if (!entry_eth_gsw_mib0) {
		printk(KERN_ERR "failed while create gsw_mib0 entry \"%s\"\n", tmp);
		goto ERR_CREAT_MAC_GSW_MIB0;
	}
	entry_eth_gsw_mib0->read_proc = gsw_mib0_read_proc;
	entry_eth_gsw_mib0->write_proc = gsw_stats_write_proc;

	sprintf(tmp, "tc3162/gsw_mib1");
	entry_eth_gsw_mib1= create_proc_entry(tmp, 0644, NULL);
	if (!entry_eth_gsw_mib1) {
		printk(KERN_ERR "failed while create gsw_mib1 entry \"%s\"\n", tmp);
		goto ERR_CREAT_MAC_GSW_MIB1;
	}
	entry_eth_gsw_mib1->read_proc = gsw_mib1_read_proc;
	entry_eth_gsw_mib1->write_proc = gsw_stats_write_proc;

	sprintf(tmp, "tc3162/gsw_mib2");
	entry_eth_gsw_mib2= create_proc_entry(tmp, 0644, NULL);
	if (!entry_eth_gsw_mib2) {
		printk(KERN_ERR "failed while create gsw_mib2 entry \"%s\"\n", tmp);
		goto ERR_CREAT_MAC_GSW_MIB2;
	}
	entry_eth_gsw_mib2->read_proc = gsw_mib2_read_proc;
	entry_eth_gsw_mib2->write_proc = gsw_stats_write_proc;

	sprintf(tmp, "tc3162/gsw_mib3");
	entry_eth_gsw_mib3= create_proc_entry(tmp, 0644, NULL);
	if (!entry_eth_gsw_mib3) {
		printk(KERN_ERR "failed while create gsw_mib3 entry \"%s\"\n", tmp);
		goto ERR_CREAT_MAC_GSW_MIB3;
	}
	entry_eth_gsw_mib3->read_proc = gsw_mib3_read_proc;
	entry_eth_gsw_mib3->write_proc = gsw_stats_write_proc;

	sprintf(tmp, "tc3162/gsw_mib4");
	entry_eth_gsw_mib4= create_proc_entry(tmp, 0644, NULL);
	if (!entry_eth_gsw_mib4) {
		printk(KERN_ERR "failed while create gsw_mib4 entry \"%s\"\n", tmp);
		goto ERR_CREAT_MAC_GSW_MIB4;
	}
	entry_eth_gsw_mib4->read_proc = gsw_mib4_read_proc;
	entry_eth_gsw_mib4->write_proc = gsw_stats_write_proc;

	sprintf(tmp, "tc3162/gsw_mib5");
	entry_eth_gsw_mib5= create_proc_entry(tmp, 0644, NULL);
	if (!entry_eth_gsw_mib5) {
		printk(KERN_ERR "failed while create gsw_mib5 entry \"%s\"\n", tmp);
		goto ERR_CREAT_MAC_GSW_MIB5;
	}
	entry_eth_gsw_mib5->read_proc = gsw_mib5_read_proc;
	entry_eth_gsw_mib5->write_proc = gsw_stats_write_proc;

	sprintf(tmp, "tc3162/gsw_mib6");
	entry_eth_gsw_mib6= create_proc_entry(tmp, 0644, NULL);
	if (!entry_eth_gsw_mib6) {
		printk(KERN_ERR "failed while create gsw_mib6 entry \"%s\"\n", tmp);
		goto ERR_CREAT_MAC_GSW_MIB6;
	}
	entry_eth_gsw_mib6->read_proc = gsw_mib6_read_proc;
	entry_eth_gsw_mib6->write_proc = gsw_stats_write_proc;

	return 0;

ERR_CREAT_MAC_GSW_MIB6:
	sprintf(tmp, "tc3162/gsw_mib5");
	remove_proc_entry(tmp, NULL);
	entry_eth_gsw_mib5 = NULL;

ERR_CREAT_MAC_GSW_MIB5:
	sprintf(tmp, "tc3162/gsw_mib4");
	remove_proc_entry(tmp, NULL);
	entry_eth_gsw_mib4 = NULL;

ERR_CREAT_MAC_GSW_MIB4:
	sprintf(tmp, "tc3162/gsw_mib3");
	remove_proc_entry(tmp, NULL);
	entry_eth_gsw_mib3 = NULL;

ERR_CREAT_MAC_GSW_MIB3:
	sprintf(tmp, "tc3162/gsw_mib2");
	remove_proc_entry(tmp, NULL);
	entry_eth_gsw_mib2 = NULL;

ERR_CREAT_MAC_GSW_MIB2:
	sprintf(tmp, "tc3162/gsw_mib1");
	remove_proc_entry(tmp, NULL);
	entry_eth_gsw_mib1 = NULL;

ERR_CREAT_MAC_GSW_MIB1:
	sprintf(tmp, "tc3162/gsw_mib0");
	remove_proc_entry(tmp, NULL);
	entry_eth_gsw_mib0= NULL;

ERR_CREAT_MAC_GSW_MIB0:
	sprintf(tmp, "tc3162/gsw_stats");
	remove_proc_entry(tmp, NULL);
	entry_eth_gsw = NULL;

ERR_CREAT_MAC_GSW_STATS:
	sprintf(tmp, "tc3162/eth_stats");
	remove_proc_entry(tmp, NULL);
	entry_eth_stats = NULL;

ERR_CREAT_MAC_ETH_STATS:
	remove_proc_entry("version", entry_mac_ver);
	entry_mac_ver = NULL;

ERR_CREAT_MAC_DBG_VER:
	sprintf(tmp, "driver/%s", MT751X_MAC_MOD_NAME);
	remove_proc_entry(tmp, NULL);

	return -1;
}

static void remove_mtk_debug_proc(void)
{
	u8 tmp[32];

	if (entry_eth_gsw_mib6) {
		sprintf(tmp, "tc3162/gsw_mib6");
		remove_proc_entry(tmp, NULL);
		entry_eth_gsw_mib6 = NULL;
	}

	if (entry_eth_gsw_mib5) {
		sprintf(tmp, "tc3162/gsw_mib5");
		remove_proc_entry(tmp, NULL);
		entry_eth_gsw_mib5 = NULL;
	}

	if (entry_eth_gsw_mib4) {
		sprintf(tmp, "tc3162/gsw_mib4");
		remove_proc_entry(tmp, NULL);
		entry_eth_gsw_mib4 = NULL;
	}

	if (entry_eth_gsw_mib3) {
		sprintf(tmp, "tc3162/gsw_mib3");
		remove_proc_entry(tmp, NULL);
		entry_eth_gsw_mib3 = NULL;
	}

	if (entry_eth_gsw_mib2) {
		sprintf(tmp, "tc3162/gsw_mib2");
		remove_proc_entry(tmp, NULL);
		entry_eth_gsw_mib2 = NULL;
	}

	if (entry_eth_gsw_mib1) {
		sprintf(tmp, "tc3162/gsw_mib1");
		remove_proc_entry(tmp, NULL);
		entry_eth_gsw_mib1 = NULL;
	}

	if (entry_eth_gsw_mib0) {
		sprintf(tmp, "tc3162/gsw_mib0");
		remove_proc_entry(tmp, NULL);
		entry_eth_gsw_mib0= NULL;
	}

	if (entry_eth_gsw ) {
		sprintf(tmp, "tc3162/gsw_stats");
		remove_proc_entry(tmp, NULL);
		entry_eth_gsw = NULL;
	}

	if (entry_eth_stats) {
		sprintf(tmp, "tc3162/eth_stats");
		remove_proc_entry(tmp, NULL);
		entry_eth_stats = NULL;
	}

	if (entry_mac_ver) {
		remove_proc_entry("version", proc_mac_dbg);
		entry_mac_ver = NULL;
	}

	if (proc_mac_dbg) {
		sprintf(tmp, "driver/%s", MT751X_MAC_MOD_NAME);
		remove_proc_entry(tmp, NULL);
		proc_mac_dbg = NULL;
	}
}
#if  defined(TCSUPPORT_MT7510_FE)
static int eth_rx_receive_num_read_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
        int index = 0;
        off_t pos = 0;
        off_t begin = 0;

        index += sprintf(buf+index, "mac_receive_num %d mac_receive_threshold %d\n", mac_receive_num, mac_receive_threshold);
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

static int eth_rx_receive_num_write_proc(struct file *file, const char *buffer,
        unsigned long count, void *data){
        char tmp[16];
        int len;

        if (count > sizeof(tmp)) {
               len = sizeof(tmp);
        }
        else {
               len = count;
       }

        if(copy_from_user(tmp, buffer, len))
                return -EFAULT;

        /* zero terminate get_buf */
        tmp[len]='\0';


       if ((2 == sscanf(tmp, "%d %d", &mac_receive_num, &mac_receive_threshold)) ){
               printk("mac_receive_num %d mac_receive_threshold %d\n", mac_receive_num, mac_receive_threshold );
        }else{
                printk("Invalid input %s\n", tmp);
        }
        return count;

}
#endif

static int __init mt751x_mac_init(void)
{
	struct net_device *dev = NULL;
	struct device_priv *priv = NULL;
	struct proc_dir_entry *parent = init_net.proc_net;
	struct proc_dir_entry *eth_proc;
	struct sockaddr addr;
	unsigned char mac[ETH_ALEN] = {0x00, 0x1E, 0xE3, 0x00, 0x00, 0x00};
	int ret_val = -ENOMEM;
	int i;
	int txq;

	mac_p = kzalloc(sizeof(struct macAdapter), GFP_KERNEL);
	if(NULL == mac_p) {
		goto out;
	}
	spin_lock_init(&mac_p->lock);
	for(i = 0; i < SWITCH_PORT_MAX; i++) {
		dev = alloc_etherdev(sizeof(struct macAdapter));
		if(NULL == dev) {
			printk("%s: Fail to allocate etherdev %d\n", __func__, i);
			if(NULL != mac_p) {
				kfree(mac_p);
			}
			for( ; i >= 0; i--) {
				dev = switch_dev[i];
				free_netdev(dev);
			}
			goto out;
		} else {
			switch_dev[i] = dev;
			sprintf(dev->name, "eth%d", i);
			dev->irq = MAC_INT;
			dev->addr_len = ETH_ALEN;
			dev->base_addr = CR_GSW_BASE;
			dev->netdev_ops = &gmac_netdev_ops;/* Hook up with handlers */
			dev->watchdog_timeo = 5 * HZ;
#ifdef CONFIG_RAETH_CHECKSUM_OFFLOAD
			dev->features |= NETIF_F_IP_CSUM; /* Can checksum TCP/UDP over IPv4 */
#ifdef CONFIG_RAETH_TSOV6
			dev->features |= NETIF_F_TSO6;
			dev->features |= NETIF_F_IPV6_CSUM; /* Can checksum TCP/UDP over IPv6 */
#endif
#else // Checksum offload disabled
			dev->features &= ~NETIF_F_IP_CSUM; /* disable checksum TCP/UDP over IPv4 */
#endif
			dev->priv_flags |= IFF_ETH;
			priv = netdev_priv(dev);
			memset(priv, 0x00, sizeof(struct device_priv));
			priv->phy_id = i;
			ret_val = register_netdev(dev);/* Register net device for the driver */
			if(ret_val) {
				printk("%s: register netdev %s failed\n", __func__, dev->name);
			}
		}
	}
	mac_p->napi.weight = MAC_NAPI_WEIGHT;
	netif_napi_add(dev, &mac_p->napi, mt751x_mac_poll, MAC_NAPI_WEIGHT);
	napi_enable(&mac_p->napi);
	tasklet_init(&mac_p->lk_task, mt751x_esw_dsr, (unsigned long)dev);
	ret_val = macInit(dev);
	if(ret_val) {
		printk("Fail to Initial GMAC!\n");
		goto out;
	}
	mt751x_vlan_init();
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
	ret_val = request_irq(dev->irq, mt751x_mac_isr, IRQF_DISABLED, dev->name, dev);
	if(ret_val) {
		goto out;
	}
	ret_val = request_irq(MAC1_INT, mt751x_esw_isr, IRQF_DISABLED, "Link", dev);
	if(ret_val) {
		printk("%s: Fail to open IRQ %d\n", __func__, MAC1_INT);
		goto out;
	}
	printk("GEMAC 2.00-NAPI 24.Sep.2012, IRQ=%d\n", dev->irq);
	mac_p->mii_if.phy_id = mac_p->enetPhyAddr;/* MII setup */
	mac_p->mii_if.full_duplex = 1;
	mac_p->mii_if.phy_id_mask = 0x1f;
	mac_p->mii_if.reg_num_mask = 0x1f;
	mac_p->mii_if.dev = dev;
	mac_p->mii_if.mdio_read = mdio_read;
	mac_p->mii_if.mdio_write = mdio_write;
	mac_p->mii_if.supports_gmii = mii_check_gmii_support(&mac_p->mii_if);
	/* ethernet related stats */
	eth_proc = create_proc_entry("eth_stats", 0, parent);
	eth_proc->read_proc = eth_stats_read_proc;
	eth_proc->write_proc = eth_stats_write_proc;
	eth_proc = create_proc_entry("gsw_stats", 0, parent);
	eth_proc->read_proc = gsw_stats_read_proc;
	eth_proc->write_proc = gsw_stats_write_proc;
	create_proc_read_entry("link_stat", 0, parent, gsw_link_st_proc, NULL);
	create_proc_read_entry("eth_reg", 0, parent, eth_reg_dump_proc, NULL);
	for(txq = 0; txq < TX_QUEUE_NUM; txq++) {
		proc_txring[txq] = txq;
		sprintf(proc_txring_name[txq], "eth_txring%d", txq);
		create_proc_read_entry(proc_txring_name[txq], 0, parent, eth_txring_dump_proc, &proc_txring[txq]);
	}
	create_proc_read_entry("eth_rxring", 0, parent, eth_rxring_dump_proc, NULL);
#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
	eth_proc = create_proc_entry("eth1_qoswrr", 0, parent);
	eth_proc->read_proc = eth_qoswrr_read_proc;
	eth_proc->write_proc = eth_qoswrr_write_proc;
#endif
#ifdef TCSUPPORT_QOS
	eth_proc = create_proc_entry("eth1_tcqos_disc", 0, parent);
	eth_proc->read_proc = eth_tcqos_read_proc;
	eth_proc->write_proc = eth_tcqos_write_proc;
#endif
#ifdef CONFIG_ETHERNET_DEBUG
	eth_proc = create_proc_entry("dump", 0, parent);
	eth_proc->write_proc = dump_level_write_proc;
#endif

#if defined(TCSUPPORT_MT7510_FE) || defined(MT7530_SUPPORT)
	eth_proc = create_proc_entry("tc3162/eth_rx_packet_number", 0, NULL);
	if(eth_proc){
		eth_proc->read_proc = eth_rx_receive_num_read_proc;
		eth_proc->write_proc = eth_rx_receive_num_write_proc;
	}
#endif

#ifdef TC_CONSOLE_ENABLE
	create_tcconsole_proc();
	rcu_assign_pointer(send_uart_msg, uart_msg_to_tcconsole);
#endif
	/* TBS_TAG:add by pengyao 20130311: ether net interface for tbs_nfp */
	dev->priv_flags |= IFF_ETH;
	/* TBS_TAG:END */
	mac_p->statisticOn = MAC_STATISTIC_ON;
	macDrvStart(1);

	mt751x_creat_phy_dbg_entry();
	creat_mtk_debug_proc();

out:
	return ret_val;
}

static void __exit mt751x_mac_exit(void)
{
	int txq;
	int i;
	unsigned int reg;
	struct net_device *dev;
	struct proc_dir_entry *parent = init_net.proc_net;

	napi_disable(&mac_p->napi);
	for(i = 0; i < SWITCH_PORT_MAX; i++) {
		dev = switch_dev[i];
		etdebug("Free ei_local and unregister netdev %d\n", i);
		unregister_netdev(dev);
		free_netdev(dev);
	}
	free_irq(dev->irq, dev);
	macDrvStart(0);
	reg = reg_read32(CR_RSTCTRL2);
	reg |= (ESW_RST | FE_RST);
	reg_write32(CR_RSTCTRL2, reg);/* reset ethernet phy, ethernet switch, frame engine */
	reg = reg_read32(CR_RSTCTRL2);
	reg &= ~(ESW_RST | FE_RST);
	reg_write32(CR_RSTCTRL2, reg);/* de-assert reset ethernet phy, ethernet switch, frame engine */
	mdelay(100);
	remove_proc_entry("eth_stats", parent);
	remove_proc_entry("gsw_stats", parent);
	remove_proc_entry("link_stat", parent);
	remove_proc_entry("eth_reg", parent);
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		remove_proc_entry(proc_txring_name[txq], parent);
	}
	remove_proc_entry("eth_rxring", parent);
#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
	remove_proc_entry("eth1_qoswrr", parent);
#endif
#ifdef TCSUPPORT_QOS
	remove_proc_entry("eth1_tcqos_disc", parent);
#endif
#ifdef CONFIG_ETHERNET_DEBUG
	remove_proc_entry("dump", parent);
#endif
#if defined(TCSUPPORT_MT7510_FE) || defined(MT7530_SUPPORT)
	remove_proc_entry("tc3162/eth_rx_packet_number", 0);
#endif
#ifdef TC_CONSOLE_ENABLE
	delete_tcconsole_proc();
	rcu_assign_pointer(send_uart_msg, NULL);
#endif
	macDrvDescripReset(mac_p);
	if(mac_p->macTxMemPool_p) {
#ifdef CONFIG_TC3162_DMEM
		if(is_sram_addr(mac_p->macTxMemPool_p)) {
			free_sram(mac_p->macTxMemPool_p, sizeof(macTxMemPool_t));
		} else
#endif
		{
			dma_free_coherent(NULL, sizeof(macTxMemPool_t), mac_p->macTxMemPool_p, mac_p->macTxMemPool_phys_p);
		}
	}
	if(mac_p->macRxMemPool_p) {
#ifdef CONFIG_TC3162_DMEM
		if(is_sram_addr(mac_p->macRxMemPool_p)) {
			free_sram(mac_p->macRxMemPool_p, sizeof(macRxMemPool_t));
		} else
#endif
		{
			dma_free_coherent(NULL, sizeof(macRxMemPool_t), mac_p->macRxMemPool_p, mac_p->macRxMemPool_phys_p);
		}
	}
	if(mac_p) {
		kfree(mac_p);
		mac_p = NULL;
	}

	mt751x_remove_phy_dbg_entry();
	remove_mtk_debug_proc();
}
EXPORT_SYMBOL_GPL(PHY_led_ops);
/* Register startup/shutdown routines */
module_init(mt751x_mac_init);
module_exit(mt751x_mac_exit);

MODULE_LICENSE("GPL");
