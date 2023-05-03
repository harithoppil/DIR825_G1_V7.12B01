/*
** $Id: //BBN_Linux/Branch/Branch_for_MT75xx_ASIC_20130518/tclinux_phoenix/modules/private/raeth/femac.c#21 $
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
** $Log: femac.c,v $
** Revision 1.7  2011/10/21 06:14:05  lino
** add RT63365 ASIC support: hwnat
**
** Revision 1.6  2011/10/12 08:44:19  lino
** add RT63365 ASIC support: hwnat & qos support for ethernet WAN
**
** Revision 1.5  2011/10/12 05:29:01  shnwind
** RT63365 ether wan compiler
**
** Revision 1.4  2011/10/06 07:39:24  shnwind
** RT63365 etherwan support
**
** Revision 1.3  2011/10/06 07:27:55  shnwind
** RT63365 etherwan support
**
** Revision 1.2  2011/10/06 04:29:35  shnwind
** RT63365 etherwan support
**
** Revision 1.1  2011/09/23 02:07:55  shnwind
** Add rt63365 support
**
** Revision 1.1.2.11  2011/08/02 06:10:01  lino
** add RT63365 support: set port 5 giga port RX clock to degree 0
**
** Revision 1.1.2.10  2011/06/16 05:07:48  lino
** add RT63365 support: rewrite mac reset function
**
** Revision 1.1.2.9  2011/06/16 03:21:24  lino
** add RT63365 support
**
** Revision 1.1.2.8  2011/06/14 11:47:40  lino
** add RT63365 support: add 6 port phy board support
**
** Revision 1.1.2.7  2011/05/26 08:51:44  lino
** add RT63365 support: special tag
**
** Revision 1.1.2.6  2011/04/28 04:42:11  lino
** add RT63365 support
**
** Revision 1.1.2.5  2011/04/22 02:55:22  lino
** add RT63365 support
**
** Revision 1.1.2.4  2011/04/21 12:51:03  lino
** add RT63365 support
**
** Revision 1.1.2.3  2011/04/20 10:53:33  lino
** add RT63365 support
**
** Revision 1.1.2.2  2011/04/20 09:58:41  lino
** add RT63365 support
**
** Revision 1.1.2.1  2011/04/20 02:39:15  lino
** add RT63365 support
**
 */

#define TC3262_GMAC_NAPI

#define TC3262_GMAC_SKB_RECYCLE

#define DRV_NAME	"femac"
#ifdef TC3262_GMAC_NAPI
#define DRV_VERSION	"1.00-NAPI"
#else
#define DRV_VERSION	"1.00"
#endif
#define DRV_RELDATE	"29.Mar.2011"

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
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rt_flash.h>
#include <linux/version.h>
#include <asm/spram.h>
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/ledcetrl.h>
#include <asm/tc3162/TCIfSetQuery_os.h>

#ifdef TCPHY_SUPPORT
#include <asm/tc3162/cmdparse.h>
#include "../tcphy/tcetherphy.h"
#include "../tcphy/tcswitch.h"
#endif
#ifdef TCSUPPORT_AUTOBENCH
#include "../auto_bench/autobench.h"
#endif

#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif

#include "../tcphy/tcconsole.h"
#include "femac.h"

#ifdef TCSUPPORT_MT7530_SWITCH_API
#include "../tcphy/mtkswitch_api.h"
#include "../tcphy/mtkswitch_api_def.h"
//extern int macMT7530gswAPIDispatch(struct ifreq *ifr);
#endif
#ifdef MT7510_DMA_DSCP_CACHE
#include <asm/r4kcache.h>
#endif

#ifdef LOOPBACK_SUPPORT
#include "fe_verify.h"
#endif

#ifdef TCSUPPORT_MT7510_FE
#include "fe_api.h"
#undef CONFIG_TC3162_DMEM
#endif

#include <led.h>

#ifdef __BIG_ENDIAN
#define FE_BYTE_SWAP
#endif

#define RAETH_CHECKSUM_OFFLOAD

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#if defined(WAN2LAN) || defined(CONFIG_TC3162_ADSL)
#define VLAN_TAG_USED 0
#else
#define VLAN_TAG_USED 1
#endif
#else
#define VLAN_TAG_USED 0
#endif

#ifdef TCSUPPORT_MAX_PACKET_2000
#define RX_MAX_PKT_LEN 		2048
#else
#define RX_MAX_PKT_LEN 		1536
#endif

//#undef CONFIG_TC3162_DMEM

#define GEN_1588_PKT_7530_VERIFY	//MTK20120829_MT7530_1588pkt_generation
#ifdef MT7530_SUPPORT
#define switch_reg_read(reg) 		 gswPbusRead((reg-GSW_BASE))
#define switch_reg_write(reg, wdata) gswPbusWrite((reg-GSW_BASE), wdata)
#else
#define switch_reg_read(reg) 		 read_reg_word(reg)
#define switch_reg_write(reg, wdata) write_reg_word(reg, wdata)
#endif

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

#define LAN_ST_100MB        0x01
#define LAN_ST_FULL_DUPLEX	0x02
#define LAN_ST_LINK_UP      0x04
#define LAN_ST_1000MB       0x08


/* ADMTEK6996M register */
#define ADM_PORT0_BASIC		0x01
#define ADM_PORT1_BASIC		0x03
#define ADM_PORT2_BASIC		0x05
#define ADM_PORT3_BASIC		0x07
#define ADM_PORT4_BASIC		0x08
#define ADM_PORT5_BASIC		0x09

#define ADM_CHIP_ID0		0xa0
#define ADM_CHIP_ID1		0xa1

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

/************************************************************************
*                            M A C R O S
*************************************************************************
*/
#define CONFIG_8021P_REMARK 1
#if defined(TCSUPPORT_WAN_ETHER) && defined(CONFIG_8021P_REMARK)
#define QOS_8021p_MARK			0x0F00 	/* 8~11 bits used for 802.1p */
#define QOS_8021P_0_MARK		0x08	/* default mark is zero */
#define VLAN_HLEN				4
#define VLAN_ETH_ALEN			6
#endif

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
uint32 gswPbusRead(uint32 pbus_addr);
int gswPbusWrite(uint32 pbus_addr, uint32 pbus_data);

// frank add for generate 1588 protocol packet command
extern void getnstimeofday(struct timespec *tv);	//MTK20120829_MT7530_1588pkt_generation

static irqreturn_t tc3262_gmac_isr(int irq, void *dev_id);
#if defined(TCSUPPORT_WAN_ETHER)
static int eth_wan_link_st_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data);
static int tc3262_gmac_wan_start(struct net_device *dev);
static struct net_device_stats *tc3262_gmac_wan_stats(struct net_device *dev);
static int tc3262_gmac_wan_open(struct net_device *dev);
static int tc3262_gmac_wan_close(struct net_device *dev);
static void tc3262_gmac_wan_set_multicast_list(struct net_device *dev);
static int tc3262_gmac_wan_set_macaddr(struct net_device *dev, void *p);
static int tc3262_gmac_wan_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
int tc3262_gmac_wan_tx(struct sk_buff *skb, struct net_device *dev);
#endif

#if defined(TCPHY_SUPPORT)
extern int gsw_check_read_proc(char *buf, char **start, off_t off, int count,
							int *eof, void *data);

extern int gsw_check_write_proc(struct file *file, const char *buffer,
							  unsigned long count, void *data);
#endif				

extern int ethernet_portmap_read_proc(char *buf, char **start, off_t off, int count,
					                  int *eof, void *data);
extern int stag_to_vtag_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
extern int stag_to_vtag_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);
//extern void tcephydbgcmd(void);
/* for port reverse */
extern int port_reverse_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
extern int port_reverse_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);
#if defined(SQA_VERIFY)
//MTK20120829_MT7530_1588pkt_generation, Start[
static int gen_1588_pkt_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);
//MTK20120829_MT7530_1588pkt_generation, ]End
#endif

#ifdef TCPHY_DEBUG
extern void tcPhyChkVal(void);
extern int tcPhyErrMonitor(void);
#endif

/************************************************************************
*                        P U B L I C   D A T A
*************************************************************************
*/

#ifdef TCSUPPORT_QOS
int qos_flag = NULLQOS;
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
#ifdef TCPHY_SUPPORT
extern uint8 getTcPhyFlag(void);
extern int tcPhySwPatch(void);
extern int tcPhyVerLookUp(macAdapter_t *mac_p);
extern int tcPhySwPatch(void);
extern uint32 getTcPhyStatusReg(macAdapter_t * mac_p);
extern int tcPhyInit(macAdapter_t * mac_p);
extern int tcPhyPortNumGet(void);
#if !defined(TCSUPPORT_CT)
extern int eth_port_stat_read_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data);
#endif
#endif
extern int wan_port_id;
/* frankliao added 20101215 */
//extern unsigned long flash_base;
extern unsigned int (*ranand_read_byte)(unsigned long long);

/************************************************************************
*                       P R I V A T E   D A T A
*************************************************************************
*/

/* Device data */
struct net_device *tc3262_gmac_dev;
#if defined(TCSUPPORT_WAN_ETHER)
struct net_device *tc3262_gmac_wan_dev = NULL;
macAdapter_t *mac_wan_p = NULL;
#endif
/* Timer */
static struct timer_list eth_timer;
static struct timer_list eth_poll_timer;
static struct timer_list eth_wan_timer;
/* phy lock */
static spinlock_t phy_lock;

macAdapter_t *mac_p = NULL;
static macPhyLinkProfile_t enetPhyLinkProfile;

static phyDeviceList_t phyDeviceList[] = {
	{ 0x0013, "LXT972" },
	{ 0x0022, "AC101L" },
	{ 0x0243, "IP101" },
	{ 0x8201, "RTL8201" },
	{ 0x001c, "RTL8212" },
	{ 0x03a2, "TC2031" },
//	{ 0x0007, "MT7530" },    // for E1
	{ 0x03a2, "MT7530" },
};

static uint8 macInitialized = 0;	

uint8 def_mac_addr[] = {0x00, 0x00, 0xaa, 0xbb, 0xcc, 0xff};

uint8 swicVendor = 0;

static DEFINE_SPINLOCK(gimr_lock);

#ifdef WAN2LAN
/* ------ xyzhu_091105:special tag relation data start ---------- */
extern uint8 macSTagFlag;
/* ------ xyzhu_091105:special tag relation data end ----------- */
int masko_on_off = 0;
EXPORT_SYMBOL(masko_on_off);

#endif
/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#define pause(x)					mdelay(x)

#ifdef MT7510_DMA_DSCP_CACHE
static char *rx_dscp_alloc = NULL;
static char *tx_dscp_alloc = NULL;
#endif

static inline struct sk_buff *gmac_alloc_skb2k(void)
{
	return skbmgr_dev_alloc_skb2k();
	//return dev_alloc_skb(RX_BUF_LEN);
}

#ifdef TCSUPPORT_MAX_PACKET_2000
static inline struct sk_buff *gmac_alloc_skb4k(void)
{
	return skbmgr_dev_alloc_skb4k();
	//return dev_alloc_skb(RX_BUF_LEN);
}
#endif


void tcMiiStationWrite(uint32 enetPhyAddr, uint32 phyReg, uint32 miiData)
{
	uint32 reg;
	uint32 cnt=10000;
	u32 GSW_CFG_PIAC_addr;

//	GSW_CFG_PIAC_addr = (isMT7530 || isMT7530ext) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	GSW_CFG_PIAC_addr = (isMT7530) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;

	spin_lock_bh(&phy_lock);
	do {
		reg=read_reg_word (GSW_CFG_PIAC_addr);
		cnt--;
	} while ((reg & PHY_ACS_ST) && (cnt != 0));

	reg = PHY_ACS_ST | (MDIO_ST_START << MDIO_ST_SHIFT) | (MDIO_CMD_WRITE<<MDIO_CMD_SHIFT) | 
		(enetPhyAddr << MDIO_PHY_ADDR_SHIFT) | (phyReg << MDIO_REG_ADDR_SHIFT) | 
		(miiData & MDIO_RW_DATA);
	write_reg_word (GSW_CFG_PIAC_addr, reg);

	cnt = 10000;
	do {
		//pause(1);
		reg=read_reg_word (GSW_CFG_PIAC_addr);
		cnt--;
	} while ((reg & PHY_ACS_ST) && (cnt != 0));
	spin_unlock_bh(&phy_lock);
	if (cnt==0) 
		printk("EER: tcMiiStationWrite timeout!\n");
	#ifdef TCPHY_4PORT
	/*TC2206 switch IC can't be direct to do PHY reset, we must 
	 * avoid ESD software patch be trigger.
	 */
	refillPhyDefVal(enetPhyAddr, phyReg, miiData);	
	#endif
}
EXPORT_SYMBOL(tcMiiStationWrite);

uint32 tcMiiStationRead(uint32 enetPhyAddr, uint32 phyReg)
{
	uint32 reg;
	uint32 cnt=10000;
	u32 GSW_CFG_PIAC_addr;

//	GSW_CFG_PIAC_addr = (isMT7530 || isMT7530ext) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	GSW_CFG_PIAC_addr = (isMT7530) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;

	spin_lock_bh(&phy_lock);
	do {
		reg=read_reg_word (GSW_CFG_PIAC_addr);
		cnt--;
	} while ((reg & PHY_ACS_ST) && (cnt != 0));

	reg = PHY_ACS_ST | (MDIO_ST_START << MDIO_ST_SHIFT) | (MDIO_CMD_READ<<MDIO_CMD_SHIFT) | 
		(enetPhyAddr << MDIO_PHY_ADDR_SHIFT) | (phyReg << MDIO_REG_ADDR_SHIFT);
	write_reg_word (GSW_CFG_PIAC_addr, reg);

	cnt = 10000;
	do {
		//pause(1);
		reg=read_reg_word (GSW_CFG_PIAC_addr);
		cnt--;
	} while ((reg & PHY_ACS_ST) && (cnt != 0));
	reg = reg & MDIO_RW_DATA;
	spin_unlock_bh(&phy_lock);

	if (cnt == 0) 
		printk("EER: tcMiiStationRead timeout!\n");

	return reg;
}
EXPORT_SYMBOL(tcMiiStationRead);

void miiStationWrite(macAdapter_t *mac_p, uint32 phyReg, uint32 miiData)
{
	tcMiiStationWrite(mac_p->enetPhyAddr, phyReg, miiData);
}

uint32 miiStationRead(macAdapter_t *mac_p, uint32 phyReg)
{
	return tcMiiStationRead(mac_p->enetPhyAddr, phyReg);
}

void tcAdmMiiStationWrite(uint32 admReg, uint32 miiData)
{
	uint32 phyaddr;
	uint32 reg;

	phyaddr = admReg >> 5;
	reg = admReg & 0x1f;

	tcMiiStationWrite(phyaddr, reg, miiData);
}

uint32 tcAdmMiiStationRead(uint32 admReg)
{
	uint32 phyaddr;
	uint32 reg;
	
	phyaddr = admReg >> 5;
	reg = admReg & 0x1f;

	return tcMiiStationRead(phyaddr, reg);
}
#if 1
//#ifdef TCSUPPORT_MT7510_FE
//#define SUPPORT_CLAUSE45_API
#ifdef SUPPORT_CLAUSE45_API
/*_____________________________________________________________________________
**      function name: tcMiiExtStationFillAddr
**      descriptions:
**         Fill the address to prepare aceess the phy register.
**      parameters:
**            portAddr : Port Address
**            devAddr : Device Address
**            regAddr : Register Address
**
**      global:
**            None
**
**      return:
**            Success:        0
**            Otherwise:     -1
**
**      call:
**      write_reg_word
**      read_reg_word
**      DEBUGMSG
**
**      revision:
**      	Here_20090625
**____________________________________________________________________________
*/
int tcMiiExtStationFillAddr(uint32 portAddr, uint32 devAddr, uint32 regAddr)
{
	uint32 value, cnt;
	u32 GSW_CFG_PIAC_addr;

//	GSW_CFG_PIAC_addr = (isMT7530 || isMT7530ext) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	GSW_CFG_PIAC_addr = (isMT7530) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	portAddr &= 0x1F;
	devAddr &= 0x1F;

	spin_lock_bh(&phy_lock);
	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));

	value = PHY_ACS_ST | (MDIO_CL45_ST_START << MDIO_ST_SHIFT) | (MDIO_CL45_CMD_ADDR << MDIO_CMD_SHIFT) |
		(portAddr << MDIO_PHY_ADDR_SHIFT) | (devAddr << MDIO_REG_ADDR_SHIFT) |
		(regAddr & MDIO_RW_DATA);
	write_reg_word (GSW_CFG_PIAC_addr, value);

	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));
	spin_unlock_bh(&phy_lock);

	if (cnt==0){
		printk("EER: tcMiiExtStationFillAddr timeout!\n");
		return -1;
	}
	return 0;
}/*end tcMiiExtStationFillAddr*/
EXPORT_SYMBOL(tcMiiExtStationFillAddr);

/*_____________________________________________________________________________
**      function name: tcMiiExtStationWrite
**      descriptions:
**        Used 45.3 method to write the phy register
**      parameters:
**            portAddr : Port Address
**            devAddr : Device Address
**            regAddr : Register Address
**            miiData : Write Data
**
**      global:
**            None
**
**      return:
**            Success:        0
**            Otherwise:     -1
**
**      call:
**      write_reg_word
**      read_reg_word
**      DEBUGMSG
**
**      revision:
**      	Here_20090625
**____________________________________________________________________________
*/
void tcMiiExtStationWrite(uint32 portAddr, uint32 devAddr, uint32 regAddr, uint32 miiData)
{
	uint32 value, cnt;
	u32 GSW_CFG_PIAC_addr;

//	GSW_CFG_PIAC_addr = (isMT7530 || isMT7530ext) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	GSW_CFG_PIAC_addr = (isMT7530) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	portAddr &= 0x1F;
	devAddr &= 0x1F;

	tcMiiExtStationFillAddr(devAddr, portAddr, regAddr);

	spin_lock_bh(&phy_lock);
	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));

	value = PHY_ACS_ST | (MDIO_CL45_ST_START << MDIO_ST_SHIFT) | (MDIO_CL45_CMD_WRITE << MDIO_CMD_SHIFT) |
		(portAddr << MDIO_PHY_ADDR_SHIFT) | (devAddr << MDIO_REG_ADDR_SHIFT) |
		(miiData & MDIO_RW_DATA);
	write_reg_word(GSW_CFG_PIAC_addr, value);

	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));
	spin_unlock_bh(&phy_lock);

	if (cnt==0){
		printk("EER: tcMiiStationWrite timeout!\n");
	}
}
EXPORT_SYMBOL(tcMiiExtStationWrite);

/*_____________________________________________________________________________
**      function name: tcMiiExtStationRead
**      descriptions:
**        Used 45.3 method to read the phy register
**      parameters:
**            portAddr : Port Address
**            devAddr : Device Address
**            regAddr : Register Address
**            op : 0:Normal read
**			 1:Post read
**
**
**      global:
**            None
**
**      return:
**            Success:        0
**            Otherwise:     -1
**
**      call:
**      write_reg_word
**      read_reg_word
**      DEBUGMSG
**
**      revision:
**      	Here_20090625
**____________________________________________________________________________
*/
uint32 tcMiiExtStationRead(uint32 portAddr, uint32 devAddr, uint32 regAddr, uint8 op)
{
	uint32 value, cnt;
	u32 GSW_CFG_PIAC_addr;

//	GSW_CFG_PIAC_addr = (isMT7530 || isMT7530ext) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	GSW_CFG_PIAC_addr = (isMT7530) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	portAddr &= 0x1F;
	devAddr &= 0x1F;

	if (op != POST_READ){
		/*POST READ command only to fill the address once, the address will be
		increased once automatically.*/
		tcMiiExtStationFillAddr(devAddr, portAddr, regAddr);
	}

	spin_lock_bh(&phy_lock);
	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));

	if(op==POST_READ){
		value = PHY_ACS_ST | (MDIO_CL45_ST_START << MDIO_ST_SHIFT) | (MDIO_CL45_CMD_READ << MDIO_CMD_SHIFT) |
			(portAddr << MDIO_PHY_ADDR_SHIFT) | (devAddr << MDIO_REG_ADDR_SHIFT);

	}
	else{/*Normal Read*/
		value = PHY_ACS_ST | (MDIO_CL45_ST_START << MDIO_ST_SHIFT) | (MDIO_CL45_CMD_READ << MDIO_CMD_SHIFT) |
			(portAddr << MDIO_PHY_ADDR_SHIFT) | (devAddr << MDIO_REG_ADDR_SHIFT);
	}
	write_reg_word (GSW_CFG_PIAC_addr, value);

	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));
	spin_unlock_bh(&phy_lock);

	if (cnt==0){
		printk("EER: tcMiiExtStationRead timeout!\n");
	}

	return (value & MDIO_RW_DATA);
}/*end tcMiiExtStationRead*/
EXPORT_SYMBOL(tcMiiExtStationRead);

void tcMiiStationCL45Address(uint32 portAddr, uint32 devAddr, uint32 regAddr)
{
	uint32 value, cnt;
	u32 GSW_CFG_PIAC_addr;

//	GSW_CFG_PIAC_addr = (isMT7530 || isMT7530ext) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	GSW_CFG_PIAC_addr = (isMT7530) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	portAddr &= 0x1F;
	devAddr &= 0x1F;

	spin_lock_bh(&phy_lock);
	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));

	value = PHY_ACS_ST | (MDIO_CL45_ST_START << MDIO_ST_SHIFT) | (MDIO_CL45_CMD_ADDR << MDIO_CMD_SHIFT) |
		(portAddr << MDIO_PHY_ADDR_SHIFT) | (devAddr << MDIO_REG_ADDR_SHIFT) |
		(regAddr & MDIO_RW_DATA);
	write_reg_word (GSW_CFG_PIAC_addr, value);

	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));
	spin_unlock_bh(&phy_lock);
	if (cnt == 0) printk("ERR: %s() timeout!\n", __FUNCTION__);
	#if (0)
	#ifdef TCPHY_4PORT
	/*TC2206 switch IC can't be direct to do PHY reset, we must
	 * avoid ESD software patch be trigger.
	 */
	refillPhyDefVal(enetPhyAddr, phyReg, miiData);
	#endif
	#endif
}
EXPORT_SYMBOL(tcMiiStationCL45Address);

void tcMiiStationCL45Write(uint32 portAddr, uint32 devAddr, uint32 miiData)
{
	uint32 value, cnt;
	u32 GSW_CFG_PIAC_addr;

//	GSW_CFG_PIAC_addr = (isMT7530 || isMT7530ext) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	GSW_CFG_PIAC_addr = (isMT7530) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	portAddr &= 0x1F;
	devAddr &= 0x1F;

	spin_lock_bh(&phy_lock);
	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));

	value = PHY_ACS_ST | (MDIO_CL45_ST_START << MDIO_ST_SHIFT) | (MDIO_CL45_CMD_WRITE << MDIO_CMD_SHIFT) |
		(portAddr << MDIO_PHY_ADDR_SHIFT) | (devAddr << MDIO_REG_ADDR_SHIFT) |
		(miiData & MDIO_RW_DATA);
	write_reg_word(GSW_CFG_PIAC_addr, value);

	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt--;
	} while ((value & PHY_ACS_ST) && (cnt != 0));
	spin_unlock_bh(&phy_lock);
	if (cnt == 0) printk("ERR: %s() timeout!\n", __FUNCTION__);
	#if (0)
	#ifdef TCPHY_4PORT
	/*TC2206 switch IC can't be direct to do PHY reset, we must
	 * avoid ESD software patch be trigger.
	 */
	refillPhyDefVal(enetPhyAddr, phyReg, miiData);
	#endif
	#endif
}
EXPORT_SYMBOL(tcMiiStationCL45Write);

uint32 tcMiiStationCL45Read(uint32 portAddr, uint32 devAddr)
{
	uint32 value, cnt;
	u32 GSW_CFG_PIAC_addr;

//	GSW_CFG_PIAC_addr = (isMT7530 || isMT7530ext) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	GSW_CFG_PIAC_addr = (isMT7530) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	portAddr &= 0x1F;
	devAddr &= 0x1F;

	spin_lock_bh(&phy_lock);
	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));

	value = PHY_ACS_ST | (MDIO_CL45_ST_START << MDIO_ST_SHIFT) | (MDIO_CL45_CMD_READ << MDIO_CMD_SHIFT) |
		(portAddr << MDIO_PHY_ADDR_SHIFT) | (devAddr << MDIO_REG_ADDR_SHIFT);
	write_reg_word(GSW_CFG_PIAC_addr, value);

	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));
	spin_unlock_bh(&phy_lock);
	if (cnt == 0) printk("ERR: %s() timeout!\n", __FUNCTION__);

	return (value & MDIO_RW_DATA);
}
EXPORT_SYMBOL(tcMiiStationCL45Read);

uint32 tcMiiStationCL45PostReadIncAddr(uint32 portAddr, uint32 devAddr)
{
	uint32 value, cnt;
	u32 GSW_CFG_PIAC_addr;

//	GSW_CFG_PIAC_addr = (isMT7530 || isMT7530ext) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	GSW_CFG_PIAC_addr = (isMT7530) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;
	portAddr &= 0x1F;
	devAddr &= 0x1F;

	spin_lock_bh(&phy_lock);
	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));

	value = PHY_ACS_ST | (MDIO_CL45_ST_START << MDIO_ST_SHIFT) | (MDIO_CL45_CMD_POSTREAD_INCADDR << MDIO_CMD_SHIFT) |
		(portAddr << MDIO_PHY_ADDR_SHIFT) | (devAddr << MDIO_REG_ADDR_SHIFT);
	write_reg_word(GSW_CFG_PIAC_addr, value);

	cnt = 10000;
	do {
		//pause(1);
		value = read_reg_word(GSW_CFG_PIAC_addr);
		cnt --;
	} while ((value & PHY_ACS_ST) && (cnt != 0));
	spin_unlock_bh(&phy_lock);
	if (cnt == 0) printk("ERR: %s() timeout!\n", __FUNCTION__);

	return (value & MDIO_RW_DATA);
}
EXPORT_SYMBOL(tcMiiStationCL45PostReadIncAddr);

void miiExtStationFillAddr(macAdapter_t *mac_p, uint32 devAddr, uint32 regAddr)
{
	tcMiiExtStationFillAddr(mac_p->enetPhyAddr, devAddr, regAddr);
}

void miiExtStationWrite(macAdapter_t *mac_p, uint32 devAddr, uint32 regAddr, uint32 miiData)
{
	tcMiiExtStationWrite(mac_p->enetPhyAddr, devAddr, regAddr, miiData);
}

uint32 miiExtStationRead(macAdapter_t *mac_p, uint32 devAddr, uint32 regAddr, uint8 op)
{
	return tcMiiExtStationRead(mac_p->enetPhyAddr, devAddr, regAddr, op);
}
#endif 	//#ifdef SUPPORT_CLAUSE45_API

//#ifdef SUPPORT_CLAUSE45_API
//static spinlock_t phy_lock_clause45;

int tcMiiExtStationFillAddr_ext(uint32 portAddr, uint32 devAddr, uint32 regAddr)
{
	return (0);
}
EXPORT_SYMBOL(tcMiiExtStationFillAddr_ext);

void tcMiiExtStationWrite_ext(uint32 portAddr, uint32 devAddr, uint32 regAddr, uint32 miiData)
{
}
EXPORT_SYMBOL(tcMiiExtStationWrite_ext);

uint32 tcMiiExtStationRead_ext(uint32 portAddr, uint32 devAddr, uint32 regAddr, uint8 op)
{
	return 0;
}
EXPORT_SYMBOL(tcMiiExtStationRead_ext);

void tcMiiStationWrite_ext(uint32 enetPhyAddr, uint32 phyReg, uint32 miiData)
{
}
EXPORT_SYMBOL(tcMiiStationWrite_ext);

uint32 tcMiiStationRead_ext(uint32 enetPhyAddr, uint32 phyReg)
{
	return 0;
}
EXPORT_SYMBOL(tcMiiStationRead_ext);

//#endif //#ifdef SUPPORT_CLAUSE45_API

#endif  //#ifdef TCSUPPORT_MT7510_FE


static int mdio_read(struct net_device *dev, int phy_id, int reg_num)
{
	return tcMiiStationRead(phy_id, reg_num);
}

static void mdio_write(struct net_device *dev, int phy_id, int reg_num, int val)
{
	tcMiiStationWrite(phy_id, reg_num, val);
}

//** MTK120625 start,
static spinlock_t pbus_lock;
/* frank modify for rt62806 */
uint32
gswPbusRead(uint32 pbus_addr)
{
	uint32 pbus_data;

	uint32 phyaddr;
	uint32 reg;
	uint32 value;

	spin_lock_bh(&pbus_lock);

	phyaddr = 31;
	// 1. write high-bit page address
	reg = 31;
	value = (pbus_addr >> 6);
  	tcMiiStationWrite(phyaddr, reg, value);
	//mdelay(5);
	//DBG_PRINTF("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//printk("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);

	// 2. read low DWord
	reg = (pbus_addr>>2) & 0x000f;
	value = tcMiiStationRead(phyaddr, reg);
	//mdelay(5);
	//DBG_PRINTF("2. miir phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//printk("2. miir phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	pbus_data = value;

	// 3. read high DWord
	reg = 16;
		value = tcMiiStationRead(phyaddr, reg);
	//mdelay(5);
	//DBG_PRINTF("3. miir phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//printk("3. miir phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);

	pbus_data = (pbus_data) | (value<<16);

	//DBG_PRINTF("# pbus read addr=0x%04x data=0x%04x_%04x\r\n", (pbus_addr&0xfffc), (pbus_data>>16), (pbus_data&0xffff));
	//printk("# pbus read addr=0x%04x data=0x%04x_%04x\r\n", (pbus_addr&0xfffc), (pbus_data>>16), (pbus_data&0xffff));
//	printk("gswPbusRead read data:\n");

//	printk("pbus_data: %x\n", pbus_data);

	spin_unlock_bh(&pbus_lock);
	return pbus_data;
} /* end frank modify for rt62806 */

/* frank modify for rt62806 */
int
gswPbusWrite(uint32 pbus_addr, uint32 pbus_data)
{
	uint32 phyaddr;
	uint32 reg;
	uint32 value;

	spin_lock_bh(&pbus_lock);

	phyaddr = 31;

	// 1. write high-bit page address
	reg = 31;
	value = (pbus_addr >> 6);
	tcMiiStationWrite(phyaddr, reg, value);
	//mdelay(5);

	//printk("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//DBG_PRINTF("1. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);

	// 2. write low DWord
	reg = (pbus_addr>>2) & 0x000f;
	value = pbus_data & 0xffff;
	tcMiiStationWrite(phyaddr, reg, value);
	//mdelay(5);
	//printk("2. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//DBG_PRINTF("2. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);

	// 3. write high DWord
	reg = 16;
	value = (pbus_data>>16) & 0xffff;
	tcMiiStationWrite(phyaddr, reg, value);
	//mdelay(5);
	//printk("3. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);
	//DBG_PRINTF("3. miiw phyaddr=%2d reg=%2d value=%04x\r\n", phyaddr, reg, value);

	//DBG_PRINTF("# pbus write addr=0x%04x data=0x%04x_%04x\r\n", (pbus_addr&0xfffc), (pbus_data>>16),(pbus_data&0xffff));
	spin_unlock_bh(&pbus_lock);
  	return 0;
} /* end frank modify for rt62806 */

//PHY2 read/write
uint32 gswPmiRead(uint32 phy_addr, uint32 phy_reg)
{
	uint32 pbus_addr;
	uint32 pbus_data;
	uint32 phy_data;
	uint32 phy_acs_st;
//	uint32 max_wait_cnt = 1000;

	pbus_addr = 0x701c;
	// b31	- phy access 1:start&busy, 0:complete&idle
	// b29:25 - mdio phy reg addr
	// b24:20 - mdio phy addr
	// b19:18 - 2'b01: write, 2'b10: read
	// b17:16 - start field, always 2'b01
	// b15:0	- data

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

uint32 gswPmiWrite(uint32 phy_addr, uint32 phy_reg, uint32 phy_data)
{
	uint32 pbus_addr;
	uint32 pbus_data;
//	uint32 phy_acs_st;

	pbus_addr = 0x701c;
	// b31    - phy access 1:start&busy, 0:complete&idle
	// b29:25 - mdio phy reg addr
	// b24:20 - mdio phy addr
	// b19:18 - 2'b01: write, 2'b10: read
	// b17:16 - start field, always 2'b01
	// b15:0  - data

	phy_addr = phy_addr & 0x1f;
	phy_reg  = phy_reg & 0x1f;
	phy_data = phy_data & 0xffff;

	// 1. write phy_addr & phy_reg & phy_data
	pbus_data = 0x80050000; // write
	pbus_data = pbus_data | (phy_addr<<20);
	pbus_data = pbus_data | (phy_reg<<25);
	pbus_data = pbus_data | (phy_data);

	gswPbusWrite(pbus_addr,pbus_data);
//  DBG_PRINTF(" pbus write addr=0x%04x data=0x%08x\r\n", pbus_addr, pbus_data);

	return 0;
} /* end frank modify for rt62806 */

void macIMRMask(void)
{
	uint32 reg = 0;

	//reg |= RX_COHERENT | RX_DLY_INT | TX_COHERENT | TX_DLY_INT | 
	//		RX_DONE_INT1 | RX_DONE_INT0 |
	//		TX_DONE_INT3 | TX_DONE_INT2 | TX_DONE_INT1 | TX_DONE_INT0;
	reg |= RX_COHERENT | RX_DLY_INT | TX_COHERENT |
			RX_DONE_INT1 | RX_DONE_INT0;
	write_reg_word(INT_MASK, reg);
}

extern uint8 use_ext_switch;
extern void mt7530_ext_reset();
void macReset(void)
{
	uint32 reg;

#if defined(MT7530_SUPPORT)
	/* ----- Hardware reset Ehernet phy chip, this address is defined by h/w engineer ----- */
    mt7530_ext_reset();
#endif

	/* reset ethernet phy, ethernet switch, frame engine */
	reg = read_reg_word(CR_RSTCTRL2);
	reg |= (EPHY_RST | ESW_RST | FE_RST);
	write_reg_word(CR_RSTCTRL2, reg);

	/* de-assert reset ethernet phy, ethernet switch, frame engine */
	reg = read_reg_word(CR_RSTCTRL2);
	reg &= ~(EPHY_RST | ESW_RST | FE_RST);
	write_reg_word(CR_RSTCTRL2, reg);
	if(isMT7520G || isMT7525G)
	{
		write_reg_word(0xbfb00358, 0x55550001);
		mdelay(10);
		write_reg_word(0xbfb00358, 0x55551001);
		mdelay(10);
	}
}

void macSetIntConf(void)
{
#ifndef TC3262_GMAC_NAPI
	uint32 reg;

	/* 4 packets or 40us timeout to interrupt */
	reg = RXDLY_INT_EN | (4<<RXMAX_PINT_SHIFT) | (2<<RXMAX_PTIME_SHIFT);
	write_reg_word(DLY_INT_CFG, reg);
#endif
}

// Assign Tx Rx Descriptor Control Registers
void macSetDMADescrCtrlReg(macAdapter_t *mac_p)
{
	uint32 txq;

	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		#ifdef MT7510_DMA_DSCP_CACHE
		write_reg_word(TX_BASE_PTR(txq), mac_p->macTxMemPool_phys_p + txq*(MAC_TXDESCP_NO*sizeof(macTxDescr_t)/2));
		#else
  		write_reg_word(TX_BASE_PTR(txq), K1_TO_PHY(mac_p->txDescrRingBaseVAddr[txq]));
		#endif
		
  		write_reg_word(TX_MAX_CNT(txq), mac_p->txRingSize);
		write_reg_word(TX_CTX_IDX(txq), 0);
		write_reg_word(PDMA_RST_IDX, RST_DTX_IDX(txq));
	}
	#ifdef MT7510_DMA_DSCP_CACHE
	write_reg_word(RX_BASE_PTR(0), mac_p->macRxMemPool_phys_p);
	#else
  	write_reg_word(RX_BASE_PTR(0), K1_TO_PHY(mac_p->rxDescrRingBaseVAddr));
	#endif
  	write_reg_word(RX_MAX_CNT(0), mac_p->rxRingSize);
	write_reg_word(RX_CALC_IDX(0), mac_p->rxRingSize - 1);
	write_reg_word(PDMA_RST_IDX, RST_DRX_IDX(0));
}

void macSetGSW(macAdapter_t *mac_p)
{
	uint32 reg;

	/* set port 6 as 1Gbps, FC on */
	reg = (IPG_CFG_SHORT<<IPG_CFG_PN_SHIFT) | MAC_MODE_PN | FORCE_MODE_PN |
		MAC_TX_EN_PN | MAC_RX_EN_PN | BKOFF_EN_PN | BACKPR_EN_PN |
		ENABLE_RX_FC_PN | ENABLE_TX_FC_PN | (PN_SPEED_1000M<<FORCE_SPD_PN_SHIFT) |
		FORCE_DPX_PN | FORCE_LNK_PN;
	write_reg_word(GSW_PMCR(6), reg);

	/* set cpu port as port 6 */
	reg = (0xff<<MFC_BC_FFP_SHIFT) | (0xff<<MFC_UNM_FFP_SHIFT) | (0xff<<MFC_UNU_FFP_SHIFT) |
			MFC_CPU_EN	| (6<<MFC_CPU_PORT_SHIFT);
	switch_reg_write(GSW_MFC, reg);

	
	/* check if FPGA */
	if (isFPGA) {
#ifdef TCSUPPORT_MT7510_FE
		/*decrease mdc/mdio clock*/
		reg = read_reg_word(GSW_CFG_PPSC);
		reg &= ~((1<<6) | (1<<7));
		write_reg_word(GSW_CFG_PPSC, reg);

		/* auto polling enable */
		reg = read_reg_word(GSW_CFG_PPSC);
		//reg |= 0x3F << 24;	// for FPGA external PHY, always use auto polling
		reg &= ~(0x7F << 24);	// for FPGA external PHY, always use auto polling
		reg &= ~(PHY_END_ADDR | PHY_ST_ADDR);
		if ((tcMiiStationRead(0, 2) == 0x4d) && (tcMiiStationRead(0, 3) == 0xd072)){	// MT7510 FPGA mainboard built-in 2 port
			reg |= 0x3F << 24;	// for FPGA external PHY, always use auto polling
			reg |= (6<<PHY_END_ADDR_SHIFT) | (0<<PHY_ST_ADDR_SHIFT);
			write_reg_word(GSW_CFG_PPSC, reg);
		}
		else if ((tcMiiStationRead(1, 2) == 0x4d) && (tcMiiStationRead(1, 3) == 0xd072)){	// MT7510 FPGA mainboard built-in 2 port
			reg |= 0x3 << 24;	// for FPGA external PHY, always use auto polling
			reg |= (2<<PHY_END_ADDR_SHIFT) | (1<<PHY_ST_ADDR_SHIFT);
			write_reg_word(GSW_CFG_PPSC, reg);
	#if (0)	// newer MT7520 FPGA no longer need // #ifdef TCSUPPORT_CPU_MT7520
			// MT7520 FPGA to external MT7530 can only connect with 10Mbps
			reg = tcMiiStationRead(1, MII_BMCR);
			reg &= ~(BMCR_ANENABLE | BMCR_SPEED100 | BMCR_SPEED1000 | BMCR_FULLDPLX);
			reg |= BMCR_ANRESTART + BMCR_FULLDPLX;
			tcMiiStationWrite(1, MII_BMCR, reg);
	#endif
		}
		else if ((tcMiiStationRead(4, 2) == 0xf) && (tcMiiStationRead(4, 3) == 0xc6c2)){	// MT7510 FPGA daughter board extra 4 port
			reg |= 0x3F << 24;	// for FPGA external PHY, always use auto polling
			reg |= (7<<PHY_END_ADDR_SHIFT) | (2<<PHY_ST_ADDR_SHIFT);
			write_reg_word(GSW_CFG_PPSC, reg);
		}
#else
		/* auto polling enable, 2 PHY ports, start PHY addr=6 and end PHY addr=7 */
		reg = read_reg_word(GSW_CFG_PPSC);
		reg |= PHY_AP_EN | EMB_AN_EN;
		reg &= ~(PHY_END_ADDR | PHY_ST_ADDR);
		/* check 6 PHY ports board or 2 PHY port board */
		if ((tcMiiStationRead(0, 2) == 0x243) && (tcMiiStationRead(0, 3) == 0xc54)) 
			reg |= (5<<PHY_END_ADDR_SHIFT) | (0<<PHY_ST_ADDR_SHIFT);
		else
			reg |= (7<<PHY_END_ADDR_SHIFT) | (6<<PHY_ST_ADDR_SHIFT);
		write_reg_word(GSW_CFG_PPSC, reg);
#endif
	}else{
#if !defined(MT7530_SUPPORT) && !defined(TCSUPPORT_MT7510_FE)
		/* auto polling enable, 2 PHY ports, start PHY addr=6 and end PHY addr=7 */
		reg = read_reg_word(GSW_CFG_PPSC);
		reg |= PHY_AP_EN;
		reg &= ~(PHY_END_ADDR | PHY_ST_ADDR);
		reg |= (7<<PHY_END_ADDR_SHIFT) | (7<<PHY_ST_ADDR_SHIFT);
		write_reg_word(GSW_CFG_PPSC, reg);
#endif
	}
#ifdef TCSUPPORT_MT7510_FE
		//tag-pri-Q  mapping, setup for QoS
		//PEM1: UP1-> Q1  UP0-> Q0
		reg = switch_reg_read(GSW_BASE+0x48);
		reg &= ~(0x7<<8); //UP0 -> Queue0
		reg &= ~(0x7<<24); //UP1 -> Queue1
		reg |= (0x1<<24); 
		switch_reg_write ((GSW_BASE+0x48), reg);	

		//PEM1: UP3-> Q3  UP2-> Q2
		reg = switch_reg_read(GSW_BASE+0x4c);
		reg &= ~(0x7<<8); //UP2 -> Queue2
		reg |= (0x2<<8);
		reg &= ~(0x7<<24); //UP3 -> Queue3 
		reg |= (0x3<<24); 
		switch_reg_write ((GSW_BASE+0x4c), reg);

		//PEM1: UP5-> Q5  UP4-> Q4
		reg = switch_reg_read(GSW_BASE+0x50);
		reg &= ~(0x7<<8); //UP4 -> Queue4
		reg |= (0x4<<8);
		reg &= ~(0x7<<24); //UP5 -> Queue5 
		reg |= (0x5<<24); 
		switch_reg_write ((GSW_BASE+0x50), reg);

		//PEM1: UP7-> Q7  UP6-> Q6
		reg = switch_reg_read(GSW_BASE+0x54);
		reg &= ~(0x7<<8); //UP6 -> Queue6
		reg |= (0x6<<8);
		reg |= (0x7<<24); //UP7 -> Queue7 
		switch_reg_write ((GSW_BASE+0x54), reg);
#else
	/* set port 5 giga port RX clock phase to degree 0 */
	reg = read_reg_word(GSW_CFG_GPC);
#if defined(MT7530_SUPPORT)
	reg |= RX_CLK_MODE | (0x3f<<24); //Disable all phy
#else
	reg |= RX_CLK_MODE;
#endif
	write_reg_word(GSW_CFG_GPC, reg);
#endif
#if defined(TCPHY_SUPPORT)
	/* enable switch abnormal irq, for error handle when abnormal irq occurs */
	if (isRT63365){
		if ( (read_reg_word(0xbfb00064) & (0xffff)) == 0x0 ){
			enable_abnormal_irq();
		}

		reg = read_reg_word(GSW_CKGCR);
		reg &= ~((1<<4) | (1<<5));
		write_reg_word(GSW_CKGCR, reg);
	}
#if defined(TCSUPPORT_WAN_ETHER) && !defined(TCSUPPORT_MT7510_FE)
	//disable switch flow control
	reg=read_reg_word ((GSW_BASE+0x1fe0));
	reg &= (~(1<<31));
	write_reg_word ((GSW_BASE+0x1fe0), reg);
#endif
#endif

#if defined(MT7530_SUPPORT)
	if (isMT7530ext)
	{
		//disable ext switch flow control
		reg = gswPbusRead (0x1fe0);
		reg &= (~(1<<31));
		gswPbusWrite (0x1fe0, reg);
		
		// temp workaround for avoiding MT7530 7 seconds auto-powerdown
		gswPbusWrite(0x30F0, 0x00001E02);   // CKGCR : clear bit 0

		//Deduce MT7530 P6 Tx Driving Strength
#ifdef TCSUPPORT_MT7510_FE
        #if 0
		gswPbusWrite(0x7a54, 0x99);
		gswPbusWrite(0x7a5c, 0x99);
		gswPbusWrite(0x7a64, 0x99);
		gswPbusWrite(0x7a6c, 0x99);
		gswPbusWrite(0x7a74, 0x99);
		#else
    	gswPbusWrite(0x7a54, 0x44);
		gswPbusWrite(0x7a5c, 0x44);
		gswPbusWrite(0x7a64, 0x44);
		gswPbusWrite(0x7a6c, 0x44);
		gswPbusWrite(0x7a74, 0x44);
		gswPbusWrite(0x7a7c, 0xff);
		write_reg_word(GSW_BASE+0X7a54, 0x44);
		write_reg_word(GSW_BASE+0x7a5c, 0x44);
		write_reg_word(GSW_BASE+0x7a64, 0x44);
		write_reg_word(GSW_BASE+0x7a6c, 0x44);
		write_reg_word(GSW_BASE+0x7a74, 0x44);
		write_reg_word(GSW_BASE+0x7a7c, 0xff);
		#endif
#else
		//63368
		gswPbusWrite(0x7a54, 0x44);
		gswPbusWrite(0x7a5c, 0x44);
		gswPbusWrite(0x7a64, 0x44);
		gswPbusWrite(0x7a6c, 0x44);
		gswPbusWrite(0x7a74, 0x44);
#endif
        gswPbusWrite(0x7a40, 0x50020000);
        gswPbusWrite(0x7a40, 0x40020000);
        gswPbusWrite(0x7a00, 0x80020000);
        gswPbusWrite(0x7a00, 0x00020000);

        write_reg_word(GSW_BASE+0X7a40, 0x50020000);
        write_reg_word(GSW_BASE+0X7a40, 0x40020000);
        write_reg_word(GSW_BASE+0X7a00, 0x80020000);
        write_reg_word(GSW_BASE+0X7a00, 0x00020000);

		//Disable Port 5/Port 6 SA Learning
		write_reg_word(GSW_PSC(5), 0xfff10);
		write_reg_word(GSW_PSC(6), 0xfff10);

		/* set port 5 as 1000Mbps, FC on */
		reg = (IPG_CFG_SHORT<<IPG_CFG_PN_SHIFT) | MAC_MODE_PN | FORCE_MODE_PN | 
			MAC_TX_EN_PN | MAC_RX_EN_PN | BKOFF_EN_PN | BACKPR_EN_PN | 
			(PN_SPEED_1000M<<FORCE_SPD_PN_SHIFT) | FORCE_DPX_PN | FORCE_LNK_PN;

		write_reg_word(GSW_PMCR(5), reg);
		gswPbusWrite(0x3600, reg);

	}
#endif
}

void macSetMACCR(macAdapter_t *mac_p)
{
	uint32 reg;
	int i;
		
#if defined(TCSUPPORT_MT7510_FE)
	reg = read_reg_word(GDMA1_FWD_CFG);
	reg &= ~(0x7777);
	reg |= GDM_STRPCRC | 
		(GDM_P_CPU<<GDM_UFRC_P_SHIFT) | (GDM_P_CPU<<GDM_BFRC_P_SHIFT) |
		(GDM_P_CPU<<GDM_MFRC_P_SHIFT) | (GDM_P_CPU<<GDM_OFRC_P_SHIFT);
#ifdef RAETH_CHECKSUM_OFFLOAD
	reg |= GDM_ICS_EN | GDM_TCS_EN | GDM_UCS_EN;
#endif
	write_reg_word(GDMA1_FWD_CFG, reg);

	reg = (0x8100<<INS_VLAN_SHIFT);
	write_reg_word(CDMA_CSG_CFG, reg);
	/* check if FPGA */
	if (isFPGA) {
		/* set 1us clock for FPGA */
		reg = read_reg_word(CR_CLK_CFG);
		reg &= ~(0x3f000000);
		reg |= (0x31<<24);
		write_reg_word(CR_CLK_CFG, reg);
	}
#else

	reg = (12<<GDM_JMB_LEN_SHIFT) | GDM_STRPCRC |
		(GDM_P_CPU<<GDM_UFRC_P_SHIFT) | (GDM_P_CPU<<GDM_BFRC_P_SHIFT) | 
		(GDM_P_CPU<<GDM_MFRC_P_SHIFT) | (GDM_P_CPU<<GDM_OFRC_P_SHIFT);
#ifdef RAETH_CHECKSUM_OFFLOAD
	reg |= GDM_ICS_EN | GDM_TCS_EN | GDM_UCS_EN;
#endif
	write_reg_word(GDMA1_FWD_CFG, reg);
#if defined(TCSUPPORT_WAN_ETHER)
	write_reg_word(GDMA2_FWD_CFG, reg);
#endif

	reg = (0x8100<<INS_VLAN_SHIFT);
#ifdef RAETH_CHECKSUM_OFFLOAD
	reg |= ICS_GEN_EN | UCS_GEN_EN | TCS_GEN_EN;
#endif
	write_reg_word(CDMA_CSG_CFG, reg);

	/* set PDMA FC default value */
	write_reg_word(PDMA_FC_CFG1, 0x0fffffff);
	write_reg_word(PDMA_FC_CFG2, 0x0fffffff);

	/* check if FPGA */
	if (isFPGA) {
		/* set 1us clock for FPGA */
		reg = read_reg_word(CR_CLK_CFG);
		reg &= ~(0x3f000000);
		reg |= (0x32<<24);
		write_reg_word(CR_CLK_CFG, reg);
	}

	/* set VLAN ID */
	for (i = 0; i < 8; i++) {
		reg = ((2*i+1)<<VLAN_ID1_SHIFT) | ((2*i)<<VLAN_ID0_SHIFT);
		write_reg_word(FE_VLAN_ID(i), reg);
	}
#endif
}

void macSetMacReg(macAdapter_t *mac_p)
{
	write_reg_word(GDMA1_MAC_ADRL, mac_p->macAddr[2]<<24 | mac_p->macAddr[3]<<16 | \
                               mac_p->macAddr[4]<<8  | mac_p->macAddr[5]<<0);
	write_reg_word(GDMA1_MAC_ADRH, mac_p->macAddr[0]<<8  | mac_p->macAddr[1]<<0);

#if defined(TCSUPPORT_WAN_ETHER) || defined(TCSUPPORT_MT7510_FE)
	write_reg_word(GDMA2_MAC_ADRL, mac_p->macAddr[2]<<24 | mac_p->macAddr[3]<<16 | \
                               mac_p->macAddr[4]<<8  | mac_p->macAddr[5]<<0);
	write_reg_word(GDMA2_MAC_ADRH, mac_p->macAddr[0]<<8  | mac_p->macAddr[1]<<0);
#endif

	if ((swicVendor==SWIC_RT63365) || (swicVendor==SWIC_MT7530)) {
		write_reg_word(GSW_SMACCR0, mac_p->macAddr[2]<<24 | mac_p->macAddr[3]<<16 | \
                               mac_p->macAddr[4]<<8  | mac_p->macAddr[5]<<0);
		write_reg_word(GSW_SMACCR1, mac_p->macAddr[0]<<8  | mac_p->macAddr[1]<<0);
	}
}

macTxDescr_t *macTxRingProc(macAdapter_t *mac_p, uint32 txq)
{
	volatile macTxDescr_t *pTxDescp;
	volatile macTxDescr_t pTxDescpTmpVal;
	volatile macTxDescr_t *pTxDescpTmp = &pTxDescpTmpVal;
	unsigned long flags;
	struct sk_buff *freeskb;

	spin_lock_irqsave(&mac_p->lock, flags);
	pTxDescp = ((macTxDescr_t*)mac_p->txDescrRingBaseVAddr[txq]) + mac_p->txUnReleasedDescp[txq];

#if defined(FE_BYTE_SWAP) || defined(__LITTLE_ENDIAN)
	pTxDescpTmp = pTxDescp;
#else
	pTxDescpTmp->txd_info1.word = le32_to_cpu(pTxDescp->txd_info1.word);
	pTxDescpTmp->txd_info2.word = le32_to_cpu(pTxDescp->txd_info2.word);
	pTxDescpTmp->txd_info3.word = le32_to_cpu(pTxDescp->txd_info3.word);
	pTxDescpTmp->txd_info4.word = le32_to_cpu(pTxDescp->txd_info4.word);
#endif
#ifdef MT7510_DMA_DSCP_CACHE
	protected_cache_op(Hit_Invalidate_D, ((unsigned long)(pTxDescpTmp)));
#endif

  	while (mac_p->txUnReleasedBufCnt[txq] != 0) {
  		if (!pTxDescpTmp->txd_info2.bits.DDONE_bit) { 
			spin_unlock_irqrestore(&mac_p->lock, flags);
			return 0; 
		}

		freeskb = mac_p->txskbs[txq][mac_p->txUnReleasedDescp[txq]];
#ifdef LOOPBACK_SUPPORT
		if (LOOPBACK_MODE(macLoopback))
			dev_kfree_skb(freeskb);
		else
#endif
			dev_kfree_skb_any(freeskb);

		//pTxDescp->tdes2.txbuf_addr = 0;
		mac_p->txskbs[txq][mac_p->txUnReleasedDescp[txq]] = NULL;

		if (mac_p->txUnReleasedDescp[txq] == (mac_p->txRingSize - 1))
			mac_p->txUnReleasedDescp[txq] = 0;
		else
			mac_p->txUnReleasedDescp[txq]++;
		mac_p->txUnReleasedBufCnt[txq]--;

		pTxDescp = ((macTxDescr_t*)mac_p->txDescrRingBaseVAddr[txq]) + mac_p->txUnReleasedDescp[txq];

#if defined(FE_BYTE_SWAP) || defined(__LITTLE_ENDIAN)
		pTxDescpTmp = pTxDescp;
#else
		pTxDescpTmp->txd_info1.word = le32_to_cpu(pTxDescp->txd_info1.word);
		pTxDescpTmp->txd_info2.word = le32_to_cpu(pTxDescp->txd_info2.word);
		pTxDescpTmp->txd_info3.word = le32_to_cpu(pTxDescp->txd_info3.word);
		pTxDescpTmp->txd_info4.word = le32_to_cpu(pTxDescp->txd_info4.word);
#endif
#ifdef MT7510_DMA_DSCP_CACHE
		protected_cache_op(Hit_Invalidate_D, ((unsigned long)(pTxDescpTmp)));
#endif
    	mac_p->macStat.inSilicon.txDeQueueNum++;
	} 
	spin_unlock_irqrestore(&mac_p->lock, flags);

	return (macTxDescr_t*) pTxDescp;
}

#if defined(TCSUPPORT_WAN_ETHER) && defined(CONFIG_8021P_REMARK)
static inline struct sk_buff* vlanPriRemark(struct sk_buff *skb)
{
	char * vlan_p = NULL, *ether_type_ptr = NULL;
	unsigned char ucprio = 0;
	unsigned char uc802prio = 0;
	uint16 vid=0;
	int copy_len = 0;

	if ( skb->mark & QOS_8021p_MARK ) {
		ether_type_ptr = skb->data + 12;
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
				#if 0
				struct sk_buff *sk_tmp = skb;
				skb = skb_realloc_headroom(sk_tmp, VLAN_HLEN);
				
				if ( ATM_SKB(sk_tmp)->vcc->pop ) {
					ATM_SKB(sk_tmp)->vcc->pop(ATM_SKB(sk_tmp)->vcc, sk_tmp);
				}
				else {
					dev_kfree_skb_any(sk_tmp);
				}
				#endif
				struct sk_buff *skb2 = skb_realloc_headroom(skb, VLAN_HLEN);
				dev_kfree_skb(skb);
				if (skb2 == NULL) {
					printk(KERN_ERR, "Vlan:failed to realloc headroom\n");
					return NULL;
				}
				skb = skb2;
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
		
			copy_len = 2*VLAN_ETH_ALEN;
			/*move the mac address to the beginning of new header*/
			memmove(skb->data, skb->data+VLAN_HLEN, copy_len);
			skb->network_header -= VLAN_HLEN;
			skb->mac_header -= VLAN_HLEN;
		}
	
		vlan_p = skb->data + 12;
		*(unsigned short *)vlan_p = 0x8100;
		
		vlan_p += 2;
		*(unsigned short *)vlan_p = 0;
		/*3 bits priority and vid vlaue*/
		*(unsigned short*)vlan_p |= (((uc802prio & 0x7) << 13)|vid) ;
		//skb->network_header -= VLAN_HLEN;
		//skb->mac_header -= VLAN_HLEN;
	}
	return skb;
}
#endif

#ifdef TCSUPPORT_AUTOBENCH
int tc3262_gmac_tx(struct sk_buff *skb, struct net_device *dev)
{
	if (skb != NULL){
		dev_kfree_skb_any(skb);
	}
	return NETDEV_TX_OK;
}
#endif

#ifdef TCSUPPORT_AUTOBENCH
int tc3262_gmac_tx_autobench(struct sk_buff *skb, struct net_device *dev)
#else
int tc3262_gmac_tx(struct sk_buff *skb, struct net_device *dev)
#endif
{
	volatile macTxDescr_t *currDescrp = NULL;
	volatile macTxDescr_t currDescrpTmpVal;
	volatile macTxDescr_t *currDescrpTmp = &currDescrpTmpVal;
	uint32 length;
	uint8 *bufAddrp;
	unsigned long flags;
	uint32 txq = 0;
	char is_wan_packet = 0;
#if defined(TCSUPPORT_RA_HWNAT) && defined(TCSUPPORT_MT7510_FE) 
	struct port_info eth_info;	
#endif


#if defined(TCSUPPORT_WAN_ETHER)
	if(dev == tc3262_gmac_wan_dev){
		//note send to wan port
		setComeFromWan(skb,1);
		is_wan_packet = 1;
#ifdef CONFIG_8021P_REMARK
		skb=vlanPriRemark(skb);
		if(skb==NULL){
			printk("802.1p remark failure\r\n");
			return NETDEV_TX_OK;
		}
#endif
	}else{
		setComeFromWan(skb,0);
		is_wan_packet = 0;
	}
#endif
	
#ifdef LOOPBACK_SUPPORT
	if (macLoopback & LOOPBACK_PKT) {
		printk("TX: BEFORE ");
		dump_skb(skb);
	}
#endif

#ifdef CONFIG_TC3162_ADSL
	if(isTCConsolePkt(skb)){
		if((swicVendor==SWIC_RT63365) || (swicVendor==SWIC_MT7530)){
			if(swicVendor==SWIC_MT7530)
			{
				if(isMT7520 || isMT7520G || isMT7525 || isMT7525G)
					skb = macMT7520STagInsert(skb);
				else
					skb = macMT7510STagInsert(skb);
			}	
			else if(swicVendor==SWIC_RT63365)
			{
#if defined(MT7530_SUPPORT)
				skb = macRT63368ExtSTagInsert(skb);
#else			
				skb = macRT63365STagInsert(skb);
#endif
			}
			if (skb == NULL) {
#if defined(TCSUPPORT_WAN_ETHER)
				if(is_wan_packet == 1){
					mac_wan_p->macStat.MIB_II.outDiscards++;
				}else
#endif							
				mac_p->macStat.MIB_II.outDiscards++;
				return NETDEV_TX_OK;
			}
		}
	}
#endif
#ifdef WAN2LAN
	if(((swicVendor==SWIC_RT63365) || (swicVendor==SWIC_MT7530))&&macSTagFlag && !(skb->mark & SKBUF_TCCONSOLE)){
#ifdef TCSUPPORT_RA_HWNAT
		if (ra_sw_nat_hook_magic) {
			if (ra_sw_nat_hook_magic(skb, FOE_MAGIC_PPE) == 0) {
				if(swicVendor==SWIC_MT7530)
				{
					if(isMT7520 || isMT7520G || isMT7525 || isMT7525G)
						skb = macMT7520STagInsert(skb);
					else
						skb = macMT7510STagInsert(skb);
				}	
				else if(swicVendor==SWIC_RT63365)
				{
#if defined(MT7530_SUPPORT)
					skb = macRT63368ExtSTagInsert(skb);
#else			
					skb = macRT63365STagInsert(skb);
#endif
				}	

				if (skb == NULL) {
#if defined(TCSUPPORT_WAN_ETHER)
					if(is_wan_packet == 1){
						mac_wan_p->macStat.MIB_II.outDiscards++;
					}else
#endif
					mac_p->macStat.MIB_II.outDiscards++;
					return NETDEV_TX_OK;
				}
			}
		}else{
			if(swicVendor==SWIC_MT7530)
			{
				if(isMT7520 || isMT7520G || isMT7525 || isMT7525G)
					skb = macMT7520STagInsert(skb);
				else
					skb = macMT7510STagInsert(skb);
			}
			else if(swicVendor==SWIC_RT63365)
			{
#if defined(MT7530_SUPPORT)
					skb = macRT63368ExtSTagInsert(skb);
#else			
					skb = macRT63365STagInsert(skb);
#endif
			}
			if (skb == NULL) {
#if defined(TCSUPPORT_WAN_ETHER)
				if(is_wan_packet == 1){
					mac_wan_p->macStat.MIB_II.outDiscards++;
				}else
#endif
				mac_p->macStat.MIB_II.outDiscards++;
				return NETDEV_TX_OK;
			}
		}	
#else	
		if(swicVendor==SWIC_MT7530)
		{
			if(isMT7520 || isMT7520G || isMT7525 || isMT7525G)
				skb = macMT7520STagInsert(skb);
			else
				skb = macMT7510STagInsert(skb);
		}	
		else if(swicVendor==SWIC_RT63365)
		{
#if defined(MT7530_SUPPORT)
				skb = macRT63368ExtSTagInsert(skb);
#else			
				skb = macRT63365STagInsert(skb);
#endif
		}
		if (skb == NULL) {
#if defined(TCSUPPORT_WAN_ETHER)
			if(is_wan_packet == 1){
				mac_wan_p->macStat.MIB_II.outDiscards++;
			}else
#endif
			mac_p->macStat.MIB_II.outDiscards++;
			return NETDEV_TX_OK;
		}
#endif		
	}
#endif
#if defined(TCSUPPORT_WAN_ETHER) || defined(TCSUPPORT_MT7510_FE) 
	txq = 0;
#else
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
	#endif
#endif

#ifdef TCSUPPORT_RA_HWNAT
	if (ra_sw_nat_hook_tx != NULL) {
#ifdef TCSUPPORT_MT7510_FE
		eth_info.word = 0; //clean value
		eth_info.pdma_eth.txq = (txq & 0xf);
		eth_info.pdma_eth.is_wan = (is_wan_packet & 0x1);
		if (ra_sw_nat_hook_tx(skb, &eth_info, FOE_MAGIC_GE) == 0) {
#else
		if (ra_sw_nat_hook_txq)
			ra_sw_nat_hook_txq(skb, txq);
		if (ra_sw_nat_hook_tx(skb, 1) == 0) {
#endif			
			dev_kfree_skb_any(skb);
			return NETDEV_TX_OK;
		}
	}
#endif

	bufAddrp = skb->data;
	length = skb->len;
	if (unlikely(skb->len < (ETH_ZLEN + 4))) {// need increase 4 byte to save stag
		if (skb_padto(skb, (ETH_ZLEN + 4))) {
#if defined(TCSUPPORT_WAN_ETHER)
			if(is_wan_packet == 1){
				mac_wan_p->macStat.MIB_II.outDiscards++;
			}else
#endif			
			mac_p->macStat.MIB_II.outDiscards++;
			return NETDEV_TX_OK;
		}
		length = (ETH_ZLEN + 4);
	}

#ifdef LOOPBACK_SUPPORT
	if (macLoopback & LOOPBACK_PKT) {
		printk("TX: ");
		dump_skb(skb);
	}

	if (macLoopback & LOOPBACK_TX_RANDOM)
		txq = random32() & GMAC_PRIORITY_MASK;
	else
		txq = skb->priority & GMAC_PRIORITY_MASK;
#endif

	if (mac_p->txUnReleasedBufCnt[txq] >= TX_BUF_RELEASE_THRESHOLD)
    	macTxRingProc(mac_p, txq);

	//need protect this count read before count add. shnwind .
	spin_lock_irqsave(&mac_p->lock, flags);
	if (mac_p->txUnReleasedBufCnt[txq] == mac_p->txRingSize - 1) {
#if defined(TCSUPPORT_WAN_ETHER)
		if(is_wan_packet == 1){
			mac_wan_p->macStat.MIB_II.outDiscards++;
		}else
#endif		
		mac_p->macStat.MIB_II.outDiscards++;
		spin_unlock_irqrestore(&mac_p->lock, flags);
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	/* ----- Count the MIB-II ----- */
#if defined(TCSUPPORT_WAN_ETHER)
	if(is_wan_packet == 1){
		mac_wan_p->macStat.MIB_II.outOctets += length;
		if (*bufAddrp & 0x01)
			mac_wan_p->macStat.MIB_II.outMulticastPkts++;
		else
			mac_wan_p->macStat.MIB_II.outUnicastPkts++;
	}else		
#endif	
	{
	mac_p->macStat.MIB_II.outOctets += length;

	if (*bufAddrp & 0x01)
		mac_p->macStat.MIB_II.outMulticastPkts++;
	else
		mac_p->macStat.MIB_II.outUnicastPkts++;

	}
	#if 0
  	ledTurnOn(LED_ETHER_ACT_STATUS);
	if (mac_p->macPhyLinkProfile_p->enetMode & LAN_ST_100MB)
		ledTurnOn(LED_ETHER_100M_ACT_STATUS);
	else
		ledTurnOn(LED_ETHER_10M_ACT_STATUS);
	#endif

	dma_cache_wback_inv((unsigned long)(skb->data), length);

	//spin_lock_irqsave(&mac_p->lock, flags);

	/* ----- Get the transmit descriptor ----- */
	currDescrp = ((macTxDescr_t *) mac_p->txDescrRingBaseVAddr[txq]) + mac_p->txCurrentDescp[txq];

#if defined(FE_BYTE_SWAP) || defined(__LITTLE_ENDIAN)
	currDescrpTmp = currDescrp;
#else
	currDescrpTmp->txd_info1.word = le32_to_cpu(currDescrp->txd_info1.word);
	currDescrpTmp->txd_info2.word = le32_to_cpu(currDescrp->txd_info2.word);
	currDescrpTmp->txd_info3.word = le32_to_cpu(currDescrp->txd_info3.word);
	currDescrpTmp->txd_info4.word = le32_to_cpu(currDescrp->txd_info4.word);
#endif

	#ifdef MT7510_DMA_DSCP_CACHE
	protected_cache_op(Hit_Invalidate_D, ((unsigned long)(currDescrpTmp)));
	#endif

  	if (!currDescrpTmp->txd_info2.bits.DDONE_bit) {
#if defined(TCSUPPORT_WAN_ETHER)
		if(is_wan_packet == 1){
			mac_wan_p->macStat.MIB_II.outDiscards++;
		}else
#endif		
		mac_p->macStat.MIB_II.outDiscards++;
		dev_kfree_skb_any(skb);
		spin_unlock_irqrestore(&mac_p->lock, flags);
		return NETDEV_TX_OK;
	}

	/* tx buffer size */
 	currDescrpTmp->txd_info1.bits.SDP0 = K1_TO_PHY(skb->data);
  	currDescrpTmp->txd_info2.bits.SDL0 = length;

  	currDescrpTmp->txd_info4.word = 0;
#ifndef TCSUPPORT_MT7510_FE
  	currDescrpTmp->txd_info4.bits.QN = 3;
#endif
#ifdef LOOPBACK_SUPPORT
	if (LOOPBACK_MODE(macLoopback) && (LOOPBACK_MODE(macLoopback) <= LOOPBACK_RX_CHK)) {
		/* check if external loopback */
		if (macLoopback & LOOPBACK_EXT) {
			/* GDMA1 */
   			currDescrpTmp->txd_info4.bits.PN = DPORT_GDMA1;
		} else {
			/* CPU */
   			currDescrpTmp->txd_info4.bits.PN = DPORT_CPU;
		}
	} else
#endif
	{
#if defined(TCSUPPORT_WAN_ETHER) && !defined(TCSUPPORT_MT7510_FE)
		if (is_wan_packet == 1)
			/* GDMA2 */
   			currDescrpTmp->txd_info4.bits.PN = DPORT_GDMA2;
		else
#endif

			/* GDMA1 */
   			currDescrpTmp->txd_info4.bits.PN = DPORT_GDMA1;
	}

#ifdef TCSUPPORT_RA_HWNAT
	if (ra_sw_nat_hook_magic) {
		if (ra_sw_nat_hook_magic(skb, FOE_MAGIC_PPE)) {
			/* PPE */
   			currDescrpTmp->txd_info4.bits.PN = DPORT_PPE;
		}
	}
#endif

#if VLAN_TAG_USED
	if (vlan_tx_tag_present(skb)) {
		uint16 vlan_tag = cpu_to_be16(vlan_tx_tag_get(skb));

   		currDescrpTmp->txd_info4.bits.VIDX = (vlan_tag & 0xfff);
   		currDescrpTmp->txd_info4.bits.VPRI = (vlan_tag>>13)&0x7;
   		currDescrpTmp->txd_info4.bits.INSV = 1;
#if defined(TCSUPPORT_MT7510_FE)
		currDescrpTmp->txd_info4.bits.CFI = (vlan_tag>>12)&0x1;
#endif
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

#if !defined(FE_BYTE_SWAP) && !defined(__LITTLE_ENDIAN)
	currDescrp->txd_info1.word = cpu_to_le32(currDescrpTmp->txd_info1.word);
	currDescrp->txd_info2.word = cpu_to_le32(currDescrpTmp->txd_info2.word);
	currDescrp->txd_info3.word = cpu_to_le32(currDescrpTmp->txd_info3.word);
	currDescrp->txd_info4.word = cpu_to_le32(currDescrpTmp->txd_info4.word);
#endif

#ifdef MT7510_DMA_DSCP_CACHE
	protected_cache_op(Hit_Writeback_Inv_D, ((unsigned long)(currDescrpTmp)));
#endif

#ifdef LOOPBACK_SUPPORT
#ifdef TCSUPPORT_MT7510_FE
	if (macLoopback & LOOPBACK_TX_VLAN){
		currDescrpTmp->txd_info4.bits.VIDX = 0x99;
   		currDescrpTmp->txd_info4.bits.VPRI = 0x2;
		currDescrpTmp->txd_info4.bits.CFI  = 1;
   		currDescrpTmp->txd_info4.bits.INSV = 1;
	}
#endif
	if (macLoopback & LOOPBACK_MSG) {
		printk("tx txq=%ld curr=%ld\n", txq, mac_p->txCurrentDescp[txq]);
		printk(" tdes1=%08lx\n", currDescrpTmp->txd_info1.word);
		printk(" tdes2=%08lx\n", currDescrpTmp->txd_info2.word);
		printk(" tdes3=%08lx\n", currDescrpTmp->txd_info3.word);
		printk(" tdes4=%08lx\n", currDescrpTmp->txd_info4.word);
	}
#endif

	mac_p->txCurrentDescp[txq] = (mac_p->txCurrentDescp[txq] + 1) % mac_p->txRingSize;

	wmb();

	write_reg_word(TX_CTX_IDX(txq), mac_p->txCurrentDescp[txq]);

	mac_p->txUnReleasedBufCnt[txq]++;

  	mac_p->macStat.inSilicon.txEnQueueNum++;
	spin_unlock_irqrestore(&mac_p->lock, flags);

#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
	if(skb->dev && skb->dev->name && (strncmp(skb->dev->name, "eth4", 4) == 0))
	{
		tbs_led_data_blinking(led_internet_green);
		tbs_led_data_blinking(led_lan_5);
	}
#endif

	return NETDEV_TX_OK;
}


void macDefaultParaSet(macAdapter_t *mac_p)
{
	mac_p->rxDescrSize = MAC_RXDESCP_SIZE;
	mac_p->txDescrSize = MAC_TXDESCP_SIZE;
	mac_p->rxRingSize  = MAC_RXDESCP_NO;
	mac_p->txRingSize  = MAC_TXDESCP_NO;

  	mac_p->macStat.inSilicon.rxEnQueueNum = 0;
	mac_p->macStat.inSilicon.rxDeQueueNum = 0;
	mac_p->macStat.inSilicon.txEnQueueNum = 0;
	mac_p->macStat.inSilicon.txDeQueueNum = 0;
}

int macDrvRegInit(macAdapter_t *mac_p)
{
    macIMRMask();
    
    macSetIntConf();

    macSetDMADescrCtrlReg(mac_p);

    macSetMACCR(mac_p);
    
    // --- setup MAC address ---
    macSetMacReg(mac_p);      

    macSetGSW(mac_p);

    return 0;
}

void macDrvDescripReset(macAdapter_t *mac_p)
{
	macRxDescr_t *pRxDescp;
	macTxDescr_t *pTxDescp;
	struct sk_buff *skb;
	int i;
	uint32 txq = 0;

	pRxDescp = (macRxDescr_t*) mac_p->rxDescrRingBaseVAddr;
	for (i = 0; i < mac_p->rxRingSize; i++) {
		skb = mac_p->rxskbs[i];
		if (skb != NULL)
			dev_kfree_skb_any(skb);
		// Init Descriptor
		pRxDescp->rxd_info1.word = 0;
		pRxDescp->rxd_info2.word = 0;
		pRxDescp->rxd_info3.word = 0;
		pRxDescp->rxd_info4.word = 0;
		mac_p->rxskbs[i] = NULL;
		pRxDescp++;
	}
#ifdef MT7510_DMA_DSCP_CACHE
	/*Write back DSCP*/
	dma_cache_wback_inv((unsigned long)mac_p->macRxMemPool_p, sizeof(macRxMemPool_t));
#endif

	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		pTxDescp = (macTxDescr_t*) mac_p->txDescrRingBaseVAddr[txq];
		// free all un-released tx mbuf
		for (i = 0 ; i < mac_p->txRingSize ; i++) {
			skb = mac_p->txskbs[txq][i];
			if (skb != NULL)
				dev_kfree_skb_any(skb);
			pTxDescp->txd_info1.word = 0;
			pTxDescp->txd_info2.word = 0;
			pTxDescp->txd_info3.word = 0;
			pTxDescp->txd_info4.word = 0;
			mac_p->txskbs[txq][i] = NULL;
			pTxDescp++;
		}
	}
	
#ifdef MT7510_DMA_DSCP_CACHE
	dma_cache_wback_inv((unsigned long)mac_p->macTxMemPool_p, sizeof(macTxMemPool_t));
#endif

	mac_p->rxCurrentDescp = 0;
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		mac_p->txCurrentDescp[txq] = 0;
		mac_p->txUnReleasedDescp[txq] = 0;
		mac_p->txUnReleasedBufCnt[txq] = 0;
	}
}

uint8 macDrvDescripInit(macAdapter_t *mac_p)
{
	macRxDescr_t *pRxDescp;
  	macTxDescr_t *pTxDescp;
  	uint32 i, txq;
  	struct sk_buff *skb;

	mac_p->rxDescrRingBaseVAddr = (uint32) &mac_p->macRxMemPool_p->rxDescpBuf[0];
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		mac_p->txDescrRingBaseVAddr[txq] = (uint32) &mac_p->macTxMemPool_p->txDescpBuf[txq][0];
	}

	if (macInitialized)
		macDrvDescripReset(mac_p);

	/* init. Rx descriptor, allocate memory for each descriptor */
	pRxDescp = (macRxDescr_t*) mac_p->rxDescrRingBaseVAddr;
	for (i = 0 ; i< mac_p->rxRingSize ; i++, pRxDescp++) {
		// Init Descriptor
		pRxDescp->rxd_info1.word = 0;
		pRxDescp->rxd_info2.word = 0;
		pRxDescp->rxd_info3.word = 0;
		pRxDescp->rxd_info4.word = 0;

		// Assign flag
		pRxDescp->rxd_info2.bits.LS0 = 1;  

#ifdef TCSUPPORT_MAX_PACKET_2000
		skb = gmac_alloc_skb4k();
#else
		skb = gmac_alloc_skb2k();
#endif
		if (skb == NULL) {
			printk("tc3262_gmac_descinit init fail.\n");
			return 1;
		}
		dma_cache_inv((unsigned long)(skb->data), RX_MAX_PKT_LEN);
		skb_reserve(skb, NET_IP_ALIGN);

		pRxDescp->rxd_info1.bits.PDP0 = K1_TO_PHY(skb->data);
#ifdef TCSUPPORT_MT7510_FE
		pRxDescp->rxd_info2.bits.PLEN0  = RX_MAX_PKT_LEN;
#endif
#if !defined(FE_BYTE_SWAP) && !defined(__LITTLE_ENDIAN)
		pRxDescp->rxd_info1.word = cpu_to_le32(pRxDescp->rxd_info1.word);
		pRxDescp->rxd_info2.word = cpu_to_le32(pRxDescp->rxd_info2.word);
		pRxDescp->rxd_info3.word = cpu_to_le32(pRxDescp->rxd_info3.word);
		pRxDescp->rxd_info4.word = cpu_to_le32(pRxDescp->rxd_info4.word);
#endif

		mac_p->rxskbs[i] = skb;
	}
	#ifdef MT7510_DMA_DSCP_CACHE
	/*Write back DSCP*/
	dma_cache_wback_inv((unsigned long)mac_p->macRxMemPool_p, sizeof(macRxMemPool_t));
	#endif

	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		/* init. tx descriptor, don't allocate memory */
		pTxDescp = (macTxDescr_t*) mac_p->txDescrRingBaseVAddr[txq];
		for (i = 0 ; i < mac_p->txRingSize ; i++, pTxDescp++) {
    		// Init descriptor
			pTxDescp->txd_info1.word = 0;
			pTxDescp->txd_info2.word = 0;
			pTxDescp->txd_info3.word = 0;
			pTxDescp->txd_info4.word = 0;

    		pTxDescp->txd_info2.bits.LS0_bit = 1;
    		pTxDescp->txd_info2.bits.DDONE_bit = 1;

			/* CPU */
    		//pTxDescp->txd_info4.bits.PN = 0;
			/* GDMA1 */
    		pTxDescp->txd_info4.bits.PN = DPORT_GDMA1;
#ifndef TCSUPPORT_MT7510_FE
    		pTxDescp->txd_info4.bits.QN = 3;
#endif

#if !defined(FE_BYTE_SWAP) && !defined(__LITTLE_ENDIAN)
			pTxDescp->txd_info1.word = cpu_to_le32(pTxDescp->txd_info1.word);
			pTxDescp->txd_info2.word = cpu_to_le32(pTxDescp->txd_info2.word);
			pTxDescp->txd_info3.word = cpu_to_le32(pTxDescp->txd_info3.word);
			pTxDescp->txd_info4.word = cpu_to_le32(pTxDescp->txd_info4.word);
#endif

			mac_p->txskbs[txq][i] = NULL;
		}
	}
	#ifdef MT7510_DMA_DSCP_CACHE
	dma_cache_wback_inv((unsigned long)mac_p->macTxMemPool_p, sizeof(macTxMemPool_t));
	#endif

	mac_p->rxCurrentDescp = 0;
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		mac_p->txCurrentDescp[txq] = 0;
		mac_p->txUnReleasedDescp[txq] = 0;
		mac_p->txUnReleasedBufCnt[txq] = 0;
	}

	return 0;
}

void macDrvStart(void)
{
	uint32 reg;

	reg = read_reg_word(PDMA_GLO_CFG);
	reg &= ~PDMA_BT_SIZE;
	reg |= TX_WB_DDONE | (PDMA_BT_SIZE_32DW<<PDMA_BT_SIZE_SHIFT) | RX_DMA_EN | TX_DMA_EN;
#ifdef FE_BYTE_SWAP
	reg |= PDMA_BYTE_SWAP;
#endif
#ifdef TCSUPPORT_MT7510_FE
	reg |= RX_2BYTE_OFFSET;
#endif
	write_reg_word(PDMA_GLO_CFG, reg);
}

void macPhyReset(void)
{
	#ifdef TCPHY_SUPPORT
	if(getTcPhyFlag()){
		tcPhyInit(mac_p);
	}
	else{  
	#endif	
		miiStationWrite(mac_p, MII_BMCR, BMCR_RESET);
		pause(10);
		miiStationWrite(mac_p, MII_BMCR, BMCR_ANRESTART | BMCR_ANENABLE);
	#ifdef TCPHY_SUPPORT
	}
	#endif
}

static inline int macRxRingChk(void)
{
	macRxDescr_t *rxDescrp;
	volatile macRxDescr_t rxDescrpTmpVal;
	volatile macRxDescr_t *rxDescrpTmp = &rxDescrpTmpVal;

	rxDescrp = ((macRxDescr_t*)mac_p->rxDescrRingBaseVAddr) + mac_p->rxCurrentDescp;

#if defined(FE_BYTE_SWAP) || defined(__LITTLE_ENDIAN)
	rxDescrpTmp = rxDescrp;
#else
	rxDescrpTmp->rxd_info2.word = le32_to_cpu(rxDescrp->rxd_info2.word);
#endif

 	if (rxDescrpTmp->rxd_info2.bits.DDONE_bit) 
		return 1;

	return -1;
}
#ifdef TCSUPPORT_AUTOBENCH
extern int autobench_mac_lpbk_flag;
extern int autobench_mac_lpbk_cnt;
extern int autobench_mac_lpbk1_flag;
extern unsigned char LoopbackData[];
int check_data_fail = 0;
#endif

#ifdef TCSUPPORT_MT7510_FE
#define READ_CNT 3
int readRxDespDoneBit(macRxDescr_t *rxDescrp){
	uint32 cnt = READ_CNT;
	volatile macRxDescr_t *rxDescrpTmp = rxDescrp;

	while(cnt){
		if(rxDescrpTmp->rxd_info2.bits.DDONE_bit){
			return 1;
		}
		cnt--;
	}
	return 0;
}
#endif

int macRxRingProc(struct net_device *dev, int quota)
{
	volatile macRxDescr_t *rxDescrp;
	volatile macRxDescr_t rxDescrpTmpVal;
	volatile macRxDescr_t *rxDescrpTmp = &rxDescrpTmpVal;
	uint32 frameSize;
	struct sk_buff *newskb, *skb;
	int npackets = 0;
	uint32 pattern;
	
	
	#ifdef TCPHY_4PORT
	uint16 maxPktSize=0;
	#endif

	rxDescrp = ((macRxDescr_t*)mac_p->rxDescrRingBaseVAddr) + mac_p->rxCurrentDescp;

#if defined(FE_BYTE_SWAP) || defined(__LITTLE_ENDIAN)
	rxDescrpTmp = rxDescrp;
#else
	rxDescrpTmp->rxd_info1.word = le32_to_cpu(rxDescrp->rxd_info1.word);
	rxDescrpTmp->rxd_info2.word = le32_to_cpu(rxDescrp->rxd_info2.word);
	rxDescrpTmp->rxd_info3.word = le32_to_cpu(rxDescrp->rxd_info3.word);
	rxDescrpTmp->rxd_info4.word = le32_to_cpu(rxDescrp->rxd_info4.word);
#endif

#ifdef MT7510_DMA_DSCP_CACHE
	protected_cache_op(Hit_Invalidate_D, ((unsigned long)(rxDescrpTmp)));
#endif

#ifdef TC3262_GMAC_NAPI
#if KERNEL_2_6_36
#ifdef TCSUPPORT_MT7510_FE
	while ((npackets < quota) && readRxDespDoneBit(rxDescrpTmp)) {
#else
  	while ((npackets < quota) && (rxDescrpTmp->rxd_info2.bits.DDONE_bit)) {
#endif
#else
	while ((npackets <= quota) && (rxDescrpTmp->rxd_info2.bits.DDONE_bit)) {
#endif
#else
  	while (rxDescrpTmp->rxd_info2.bits.DDONE_bit) {
#endif

		npackets++;
#ifdef LOOPBACK_SUPPORT
		if (macLoopback & LOOPBACK_MSG) {
			printk("\t\trx curr=%ld rx=%08lx\n", mac_p->rxCurrentDescp, (uint32) rxDescrp);
			printk("\t\t rdes1=%08lx\n", rxDescrpTmp->rxd_info1.word);
			printk("\t\t rdes2=%08lx\n", rxDescrpTmp->rxd_info2.word);
			printk("\t\t rdes3=%08lx\n", rxDescrpTmp->rxd_info3.word);
			printk("\t\t rdes4=%08lx\n", rxDescrpTmp->rxd_info4.word);
#ifdef TCSUPPORT_MT7510_FE
			if((rxDescrpTmp->rxd_info4.bits.SPORT != GDM_P_QDMA) || (rxDescrpTmp->rxd_info4.bits.SPORT != GDM_P_PDMA)){
				if((rxDescrpTmp->rxd_info4.bits.PKT_INFO & TU_H ) == TU_H){
					printk("Packet has TCP/UDP header and TCP/UDP checksum %svalid\n",((rxDescrpTmp->rxd_info4.bits.PKT_INFO & TU_H_C_INV) ==  TU_H_C_INV)? "in":"");
				}else{
					printk("Packet has no TCP/UDP header\n");
				}
				if((rxDescrpTmp->rxd_info4.bits.PKT_INFO & IPV4_H ) == IPV4_H){
					printk("IPv4 packet and IP header checksum %svalid\n", ((rxDescrpTmp->rxd_info4.bits.PKT_INFO & IPV4_H_INV ) == IPV4_H_INV)? "in" : "");
					
				}else{
					printk("Not IPv4 Packet\n");
				}
				if((rxDescrpTmp->rxd_info4.bits.PKT_INFO & IPV6_H ) == IPV6_H){
					printk("IPv6 packet\n");
				}else{
					printk("Not IPv6 Packet\n");
				}

			}
#endif
		}
#endif
		if (1) {
 	 		frameSize = rxDescrpTmp->rxd_info2.bits.PLEN0;

		#ifdef TCPHY_4PORT
	      	switch(swicVendor){
			case SWIC_TC2206:
				/*TC2206 Special Tag Length=8bytes, vlan header 4 bytes, max ethernet length=1518; 1518+8+4=1530*/
#ifdef TCSUPPORT_MAX_PACKET_2000
				maxPktSize=2012;
#else
				maxPktSize=1530;
#endif
				break;
			default:
#ifdef TCSUPPORT_MAX_PACKET_2000
				maxPktSize=2004;
#else
				maxPktSize=1522;
#endif
				break;
			}
	      	if (unlikely((frameSize < 60) || (frameSize > maxPktSize))){
		#else
		/* lino: make VLAN friendly */
		if (unlikely((frameSize < 60) || (frameSize > 1522))){
		#endif
#ifdef LOOPBACK_SUPPORT
				printk("\t\tERR length rx curr=%ld rx=%08lx\n", mac_p->rxCurrentDescp, (uint32) rxDescrp);
				printk("\t\t rdes1=%08lx\n", rxDescrpTmp->rxd_info1.word);
				printk("\t\t rdes2=%08lx\n", rxDescrpTmp->rxd_info2.word);
				printk("\t\t rdes3=%08lx\n", rxDescrpTmp->rxd_info3.word);
				printk("\t\t rdes4=%08lx\n", rxDescrpTmp->rxd_info4.word);

	       		skb = mac_p->rxskbs[mac_p->rxCurrentDescp];
				skb->len = rxDescrpTmp->rxd_info2.bits.PLEN0;
				dump_skb(skb);
				skb->len = 0;
#endif

	        	mac_p->macStat.inSilicon.rxEtherFrameLengthErr++;

	        	// Discard this packet & Repost this mbuf
	        	newskb = mac_p->rxskbs[mac_p->rxCurrentDescp];
	        	goto DISCARD;
	      	}

       		skb = mac_p->rxskbs[mac_p->rxCurrentDescp];
#ifdef TCSUPPORT_MAX_PACKET_2000			
			newskb = gmac_alloc_skb4k();
#else
			newskb = gmac_alloc_skb2k();
#endif
        	if (unlikely(!newskb)) { /* faild to allocate more mbuf -> drop this pkt */
        		newskb = skb;
          		mac_p->macStat.MIB_II.inDiscards++;
          		goto RECVOK;
      		}

			dma_cache_inv((unsigned long)(newskb->data), RX_MAX_PKT_LEN);
			skb_reserve(newskb, NET_IP_ALIGN);

			skb_put(skb, frameSize);


#ifdef LOOPBACK_SUPPORT
			if (macLoopback & LOOPBACK_PKT) {
				printk("RX: FIRST ");
				dump_skb(skb);
			}
#endif

	      	// ----- Count the MIB-II -----
		    if (mac_p->statisticOn) {
	  	    	mac_p->macStat.MIB_II.inOctets += frameSize;
	
				if (*skb->data & 0x01)
			    	mac_p->macStat.MIB_II.inMulticastPkts++;
				else
		        	mac_p->macStat.MIB_II.inUnicastPkts++;
		    }

			#ifdef WAN2LAN
			/*add specail tag function by xyzhu*/
			if (macSTagFlag) {
				switch (swicVendor) {
					case SWIC_RT63365:
#if defined(MT7530_SUPPORT)
						macRT63368ExtSTagRemove(skb);
#else			
						macRT63365STagRemove(skb);
#endif						
						break;
					case SWIC_MT7530:
						if(isMT7520 || isMT7520G || isMT7525 || isMT7525G)
						{
							if(macMT7520STagRemove(skb) == -1)
							{
								dev_kfree_skb_any(skb);
								goto RECVOK;
							}
						}
						else
						{
							macMT7510STagRemove(skb);
						}
						
						break;							
					default:
						break;
				}
			}
#if defined(TCSUPPORT_WAN_ETHER)
			if(isComeFromWan(skb) == 1){
				mac_wan_p->macStat.MIB_II.inOctets += frameSize;
	
				if (*skb->data & 0x01)
					mac_wan_p->macStat.MIB_II.inMulticastPkts++;
				else
		        		mac_wan_p->macStat.MIB_II.inUnicastPkts++;
			}

        		if(masko_on_off && (isComeFromWan(skb) == 1)){
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
			#endif
#ifdef TCSUPPORT_AUTOBENCH
			if (autobench_mac_lpbk_flag){
				int number = 0;
				if( memcmp(skb->data + 20 , LoopbackData + 20, 44) == 0 ){
					pattern = skb->data[64];
					for (number=64; number<MAC_LPBK_DATA_LEN; number++){
						if (skb->data[16] == 0x1){
							if (skb->data[number] != ((pattern + number - 64) & 0xff))
								break;
						} else if (skb->data[number] != skb->data[16]){
							break;
						}
					}
				}

				if (number == MAC_LPBK_DATA_LEN){
					autobench_mac_lpbk_cnt++;
				}
				else {
					if (!check_data_fail){
						int i;
						for( i=0; i<MAC_LPBK_DATA_LEN; i++){
							printk("%02x ", (unsigned char)skb->data[i]);
							if( (i+1)%8 == 0 )
								printk("\n");
						}


						for( i=64; i<MAC_LPBK_DATA_LEN; i++){
							if (skb->data[16] == 0x1){
								if (skb->data[i] != ((pattern + i - 64) & 0xff)){
									break;
								}
							}else if (skb->data[16] != skb->data[i]){
								break;
							}
						}

						printk("\n\nPacket %d data check error\n", autobench_mac_lpbk_cnt);
						printk("The first errer data at %d bytes\n", i);
						printk("error data: %02x\n", skb->data[i]);
						if (skb->data[16] == 0x1)
							printk("correct data: %02x\n", ((pattern + i - 64) & 0xff));
						else
							printk("correct data: %02x\n", skb->data[16]);
							
						check_data_fail = 1;
						printk("\n");
					}
				}	

				dev_kfree_skb_any(skb);
				goto RECVOK;
			}
			if( autobench_mac_lpbk1_flag ){
				autobench_mac_lpbk_cnt++;
				dev_kfree_skb_any(skb);
				goto RECVOK;
			}
#endif
#ifdef LOOPBACK_SUPPORT
			if (skb_shinfo(skb)->nr_frags) {
				int i;
				printk("GMAC nr_frags=%ld %lx\n", (uint32) skb_shinfo(skb)->nr_frags, (uint32) skb_shinfo(skb)->nr_frags);
				for (i = 0; i < 16; i++)
					printk("page%d=%lx\n", i, (uint32) skb_shinfo(skb)->frags[i].page);
				printk("ERR skb=%08lx data=%08lx len=%d\n", (uint32) skb, (uint32) skb->data, skb->len);
				printk("\t\tERR rx curr=%ld rx=%08lx\n", mac_p->rxCurrentDescp, (uint32) rxDescrp);
				printk("\t\t rdes1=%08lx\n", rxDescrpTmp->rxd_info1.word);
				printk("\t\t rdes2=%08lx\n", rxDescrpTmp->rxd_info2.word);
				printk("\t\t rdes3=%08lx\n", rxDescrpTmp->rxd_info3.word);
				printk("\t\t rdes4=%08lx\n", rxDescrpTmp->rxd_info4.word);
				dump_data(skb->data, 2048);
				dump_data(UNCAC_ADDR(skb->data), 2048);
			}

			if (macLoopback & LOOPBACK_PKT) {
				printk("RX: ");
				dump_skb(skb);
			}

			if (LOOPBACK_MODE(macLoopback) == LOOPBACK_TX) {
				tc3262_gmac_tx(skb, dev);
			} else if (LOOPBACK_MODE(macLoopback) == LOOPBACK_RX_DROP) {
				dev_kfree_skb_any(skb);
			} else if (LOOPBACK_MODE(macLoopback) == LOOPBACK_RX_CHK) {
				tc3262_gmac_loopback_chk(skb, dev);
			} else
#endif
			{
#ifdef CONFIG_TC3162_ADSL
			if(tcconsole_proc(skb)==1){
				;
			} else {
#else
			{
#endif
#if defined(TCSUPPORT_WAN_ETHER)
					if((tc3262_gmac_wan_dev != NULL) && (isComeFromWan(skb) == 1)){
						skb->dev = tc3262_gmac_wan_dev;
					}else
#endif
					{
					skb->dev = dev;
					}

					
#ifdef RAETH_CHECKSUM_OFFLOAD
#ifdef TCSUPPORT_MT7510_FE
					if (((rxDescrpTmp->rxd_info4.bits.SPORT != GDM_P_QDMA) && (rxDescrpTmp->rxd_info4.bits.SPORT != GDM_P_PDMA)) &&
							(((rxDescrpTmp->rxd_info4.bits.PKT_INFO & IPV4_H ) == IPV4_H) || ((rxDescrpTmp->rxd_info4.bits.PKT_INFO & IPV6_H ) == IPV6_H)) && 
							(((rxDescrpTmp->rxd_info4.bits.PKT_INFO & TU_H_C_INV ) == 0) && ((rxDescrpTmp->rxd_info4.bits.PKT_INFO & IPV4_H_INV ) == 0))){
						skb->ip_summed = CHECKSUM_UNNECESSARY;
					}else
#else
					if ((rxDescrpTmp->rxd_info4.bits.L4FVLD_bit && (rxDescrpTmp->rxd_info4.bits.L4F == 0)) ||
						(rxDescrpTmp->rxd_info4.bits.IPFVLD_bit && (rxDescrpTmp->rxd_info4.bits.IPF == 0))){
						skb->ip_summed = CHECKSUM_UNNECESSARY;
					}else
#endif
#endif
					{
						skb->ip_summed = CHECKSUM_NONE;
					}
					skb->protocol = eth_type_trans(skb, skb->dev);
					dev->last_rx = jiffies;

#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
			if(skb->dev && skb->dev->name && (strncmp(skb->dev->name, "eth4", 4) == 0))
			{
				tbs_led_data_blinking(led_internet_green);
				tbs_led_data_blinking(led_lan_5);
			}
#endif

#ifdef TCSUPPORT_RA_HWNAT
					if (ra_sw_nat_hook_rxinfo)
						ra_sw_nat_hook_rxinfo(skb, FOE_MAGIC_GE, (char *)&rxDescrpTmp->rxd_info4, sizeof(PDMA_RXD_INFO4_T));
								
					if (ra_sw_nat_hook_rx != NULL) {
						if (ra_sw_nat_hook_rx(skb)) {
#ifdef TC3262_GMAC_NAPI
							netif_receive_skb(skb);
#else
							netif_rx(skb);
#endif
					   	}
				   	} else 
#endif
					{
#ifdef TC3262_GMAC_NAPI
						netif_receive_skb(skb);
#else
						netif_rx(skb);
#endif
				   	}
				}
			}
DISCARD:

RECVOK:

			rxDescrpTmp->rxd_info1.bits.PDP0 = K1_TO_PHY(newskb->data);
	       	mac_p->rxskbs[mac_p->rxCurrentDescp] = newskb;
		} else { /* Update Error Counter and Drop it */
#ifdef LOOPBACK_SUPPORT
			printk("\t\tERR rx curr=%ld rx=%08lx\n", mac_p->rxCurrentDescp, (uint32) rxDescrp);
			printk("\t\t rdes1=%08lx\n", rxDescrpTmp->rxd_info1.word);
			printk("\t\t rdes2=%08lx\n", rxDescrpTmp->rxd_info2.word);
			printk("\t\t rdes3=%08lx\n", rxDescrpTmp->rxd_info3.word);
			printk("\t\t rdes4=%08lx\n", rxDescrpTmp->rxd_info4.word);

	       	skb = mac_p->rxskbs[mac_p->rxCurrentDescp];
			skb->len = rxDescrpTmp->rxd_info2.bits.PLEN0;
			dump_skb(skb);
			skb->len = 0;
#endif

		}
	
		rxDescrpTmp->rxd_info2.word = 0;  
		rxDescrpTmp->rxd_info2.bits.LS0 = 1;  
#ifdef TCSUPPORT_MT7510_FE
		rxDescrpTmp->rxd_info2.bits.PLEN0  = RX_MAX_PKT_LEN;
#endif
	
#if !defined(FE_BYTE_SWAP) && !defined(__LITTLE_ENDIAN)
		rxDescrp->rxd_info1.word = le32_to_cpu(rxDescrpTmp->rxd_info1.word);
		rxDescrp->rxd_info2.word = le32_to_cpu(rxDescrpTmp->rxd_info2.word);
		rxDescrp->rxd_info3.word = le32_to_cpu(rxDescrpTmp->rxd_info3.word);
		rxDescrp->rxd_info4.word = le32_to_cpu(rxDescrpTmp->rxd_info4.word);
#endif
		#ifdef MT7510_DMA_DSCP_CACHE
		protected_cache_op(Hit_Writeback_Inv_D, ((unsigned long)(rxDescrpTmp)));
		#endif

		wmb();	

		write_reg_word(RX_CALC_IDX(0), mac_p->rxCurrentDescp);

 		/* next descriptor*/
		mac_p->rxCurrentDescp = (mac_p->rxCurrentDescp + 1) % mac_p->rxRingSize;

		rxDescrp = ((macRxDescr_t*)mac_p->rxDescrRingBaseVAddr) + mac_p->rxCurrentDescp;

#if defined(FE_BYTE_SWAP) || defined(__LITTLE_ENDIAN)
		rxDescrpTmp = rxDescrp;
#else
		rxDescrpTmp->rxd_info1.word = le32_to_cpu(rxDescrp->rxd_info1.word);
		rxDescrpTmp->rxd_info2.word = le32_to_cpu(rxDescrp->rxd_info2.word);
		rxDescrpTmp->rxd_info3.word = le32_to_cpu(rxDescrp->rxd_info3.word);
		rxDescrpTmp->rxd_info4.word = le32_to_cpu(rxDescrp->rxd_info4.word);
#endif

		#ifdef MT7510_DMA_DSCP_CACHE
		//dma_cache_inv((unsigned long)rxDescrpTmp, CACHE_LINE_SIZE);
		protected_cache_op(Hit_Invalidate_D, ((unsigned long)(rxDescrpTmp)));
		#endif
	}

#ifdef LOOPBACK_SUPPORT
	if (macLoopback & LOOPBACK_MSG) 
		printk("npackets=%d\n", npackets);
#endif

	return npackets;
}

void macDrvStop(void)
{
	uint32 reg;

	reg = read_reg_word(PDMA_GLO_CFG);
	reg &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	write_reg_word(PDMA_GLO_CFG, reg);
}

macAdapter_t * macGetAdapterByChanID(void)
{
	return mac_p;
}

void macGetMacAddr(macAdapter_t *mac_p)
{
	uint32 i;

	for ( i = 0; i < 6; i++ )
		mac_p->macAddr[i] = def_mac_addr[i];
}

int32 macPhyLookUp(macAdapter_t *mac_p, uint32 companyId)
{
	uint32 i;
	uint32 phyTypeSupport;

	phyTypeSupport = sizeof(phyDeviceList) / sizeof(phyDeviceList_t);
	for ( i = 0; i < phyTypeSupport; i++ ) {
		if ( companyId == phyDeviceList[i].companyId ) {
			mac_p->enetPhyId = i;
			#ifdef TCPHY_SUPPORT
			if ((companyId == 0x03a2) || (companyId == 0x0007)){	// add MTK chip ID
				tcPhyVerLookUp(mac_p);
			}
			#endif
			return 1;
		}
	}
	return 0;
}

int32 macSearchPhyAddr(macAdapter_t *mac_p)
{
	uint32 miiReg = 0;

#if 0
	mac_p->enetPhyAddr = 0;
	for ( mac_p->enetPhyAddr = 0; mac_p->enetPhyAddr < 32; mac_p->enetPhyAddr++ ) {
#endif
		miiReg = miiStationRead(mac_p, MII_PHYSID1);
		if (miiReg == 0)
			miiReg = miiStationRead(mac_p, MII_PHYSID2);
		if (macPhyLookUp(mac_p, miiReg)) {
			return 0;
		}
#if 0
	}
	mac_p->enetPhyAddr = 0x00000000;
#endif
	return -1;
}

int macSetUpPhy(macAdapter_t *mac_p)
{
#if 0
	/* ----- Hardware reset Ehernet phy chip, this address is defined by h/w engineer ----- */
	ledTurnOn(LED_LAN_RESET);
	pause(100);
	/* ----- Wait for hardware reset completed ----- */
	ledTurnOff(LED_LAN_RESET);
	pause(600);
#endif

	/*OSBNB00031891: Add for Multi EPHY setting*/
	mac_p->enetPhyAddr = 0;
	for ( mac_p->enetPhyAddr = 0; mac_p->enetPhyAddr < 32; mac_p->enetPhyAddr++ ) {
		if(!macSearchPhyAddr(mac_p)){
			macPhyReset();
			mac_p->enetPhyAddr+= (tcPhyPortNumGet()-1);
		}
	}
	mac_p->enetPhyAddr = 0;
	/*OSBNB00031891: Add for Multi EPHY setting over*/
	pause(100);

	/* Detect 4-port switch or single port switch */
	/* detect if ADM6996M */
	if (((tcAdmMiiStationRead(ADM_CHIP_ID0) & 0xfff0) == 0x1020) &&
		(tcAdmMiiStationRead(ADM_CHIP_ID1) == 0x7)) {
		swicVendor = SWIC_ADM6996M;		    
		/* enable crossover auto detect */
		tcAdmMiiStationWrite(ADM_PORT0_BASIC, tcAdmMiiStationRead(ADM_PORT0_BASIC)|0x8000);
		tcAdmMiiStationWrite(ADM_PORT1_BASIC, tcAdmMiiStationRead(ADM_PORT1_BASIC)|0x8000);
		tcAdmMiiStationWrite(ADM_PORT2_BASIC, tcAdmMiiStationRead(ADM_PORT2_BASIC)|0x8000);
		tcAdmMiiStationWrite(ADM_PORT3_BASIC, tcAdmMiiStationRead(ADM_PORT3_BASIC)|0x8000);
	}
	/* detect if IP175C */
	if ((tcMiiStationRead(4, 2) == 0x243) && (tcMiiStationRead(4, 3) == 0xd80)) {
		swicVendor = SWIC_IP175C;
	}
	#ifdef TCPHY_4PORT
	/*Switch Model Number Register (SMNR), Addr.: 16'h02FE*/
	if (tcMiiStationRead(31, 31) == 0x2206){
	    swicVendor = SWIC_TC2206;	
	    filedSwicDefVal();
	    printk("TC2206, ");
	}
	#endif
	
	/* detect if RTL8305 */	
	if ((tcMiiStationRead(4, 2) == 0x1c) && (tcMiiStationRead(4, 3) == 0xc852)) {
		if ((tcMiiStationRead(6, 2) == 0x1c) && (tcMiiStationRead(6, 3) == 0xc852) ){
			swicVendor = SWIC_RTL8306SD;
			/*Let CPU Port Link up*/
			tcMiiStationWrite(5, 22,tcMiiStationRead(5,22)|0x8000);
			tcMiiStationWrite(6, 22,tcMiiStationRead(6,22)|0x8000);
		}
		else{
			swicVendor = SWIC_RTL8305;
		}
	}

	/* detect if MARVEL 88E6060 */	
	if (((tcMiiStationRead(4, 2) == 0x141) || (tcMiiStationRead(20, 2) == 0x141)) && 
	    ((tcMiiStationRead(4, 3) == 0xc87) || (tcMiiStationRead(20, 3) == 0xc87))) {
		swicVendor = SWIC_MARVEL6060;
	}

	if (isRT63365)
		swicVendor = SWIC_RT63365;		    

	if (!(isRT63368)){
		/* detect if RT62806, frank added 20110628*/
		if (isRT62806) {
			swicVendor = SWIC_RT62806;
		//	printk("swicVendor=SWIC_RT62806\n");
		} /* end frank added 20110628*/

		/* detect if MT7530, wplin added 20120712*/
		if (isMT7530 || isMT7530ext) {
			swicVendor = SWIC_MT7530;
		//	printk("swicVendor=SWIC_MT7530\n");
		} /* end wplin added 20120712*/
	}
	return 0;
}

#ifdef MT7510_DMA_DSCP_CACHE
void macSetDMADescrCacheReg(macAdapter_t *mac_p){
	uint32 txdscp_vaddr_start, txdscp_vaddr_end; 
	uint32 rxdscp_vaddr_start, rxdscp_vaddr_end;


	txdscp_vaddr_start = (uint32)mac_p->macTxMemPool_p;
	txdscp_vaddr_end = (uint32)(mac_p->macTxMemPool_p) + sizeof(macTxMemPool_t) - 4;
	
	rxdscp_vaddr_start = (uint32)mac_p->macRxMemPool_p;
	rxdscp_vaddr_end = (uint32)(mac_p->macRxMemPool_p) + sizeof(macRxMemPool_t) - 4;

	txdscp_vaddr_start = TO_CACHE_LINE_VADDR(txdscp_vaddr_start);
	txdscp_vaddr_end = TO_CACHE_LINE_VADDR(txdscp_vaddr_end);
	rxdscp_vaddr_start = TO_CACHE_LINE_VADDR(rxdscp_vaddr_start);
	rxdscp_vaddr_end = TO_CACHE_LINE_VADDR(rxdscp_vaddr_end);

	//printk("txdscp_vaddr_start=%x\n",txdscp_vaddr_start);
	//printk("txdscp_vaddr_end=%x\n",txdscp_vaddr_end);
	//printk("txdscp_paddr_start=%x\n",mac_p->macTxMemPool_phys_p);
	//printk("rxdscp_vaddr_start=%x\n",rxdscp_vaddr_start);
	//printk("rxdscp_vaddr_end=%x\n",rxdscp_vaddr_end);
	//printk("rxdscp_paddr_start=%x\n",mac_p->macRxMemPool_phys_p);
	pause(10);
	
	/* init dscp cache line register */
	write_reg_word((TXDSP_PADDR_START_BASE), mac_p->macTxMemPool_phys_p);
	write_reg_word((TXDSP_VADDR_START_BASE), txdscp_vaddr_start);
	write_reg_word((TXDSP_VADDR_END_BASE), txdscp_vaddr_end);
	write_reg_word((RXDSP_PADDR_START_BASE), mac_p->macRxMemPool_phys_p);
	write_reg_word((RXDSP_VADDR_START_BASE), rxdscp_vaddr_start);
	write_reg_word((RXDSP_VADDR_END_BASE), rxdscp_vaddr_end);
}

void macClearDMADescrCacheReg(void){
	write_reg_word((TXDSP_PADDR_START_BASE), 0xffffffff);
	write_reg_word((TXDSP_VADDR_START_BASE), 0xffffffff);
	write_reg_word((TXDSP_VADDR_END_BASE), 0xffffffff);
	write_reg_word((RXDSP_PADDR_START_BASE), 0xffffffff);
	write_reg_word((RXDSP_VADDR_START_BASE), 0xffffffff);
	write_reg_word((RXDSP_VADDR_END_BASE), 0xffffffff);
	
}
#endif

int macInit(void)
{
#ifdef MT7510_DMA_DSCP_CACHE
	char *temp_mem_p = NULL;
#endif
	if (macInitialized)
		return 0;

	macReset(); 

	macDrvStop();

	/* ----- Get Mac Adapter from dummy data or NDIS control block ----- */
	mac_p = macGetAdapterByChanID();

	/* ----- Assign reserved data pointer ----- */
	mac_p->macPhyLinkProfile_p = &enetPhyLinkProfile;
#ifdef CONFIG_TC3162_DMEM
	mac_p->macRxMemPool_p = (macRxMemPool_t *) alloc_sram(sizeof(macRxMemPool_t));
	if (mac_p->macRxMemPool_p == NULL)
#endif
#ifdef MT7510_DMA_DSCP_CACHE
	{
		/*Allocate 3*space, 1st space for hardware DMA, 2nd and 3rd for virutal space mapping*/
		temp_mem_p = (char *) dma_alloc_noncoherent(NULL, sizeof(macRxMemPool_t)/2*3, &mac_p->macRxMemPool_phys_p, GFP_KERNEL);
		if (temp_mem_p == NULL) {
			printk("unable to kmalloc macRxMemPool structure.\n");
			return -1;
		}
		rx_dscp_alloc = temp_mem_p;
		dma_cache_wback_inv((unsigned long)temp_mem_p, sizeof(macRxMemPool_t)/2*3);
		mac_p->macRxMemPool_p = (macRxMemPool_t *)(temp_mem_p + sizeof(macRxMemPool_t)/2);
	}

#else
	mac_p->macRxMemPool_p = (macRxMemPool_t *) dma_alloc_coherent(NULL, sizeof(macRxMemPool_t), &mac_p->macRxMemPool_phys_p, GFP_KERNEL);
	if (mac_p->macRxMemPool_p == NULL) {
		printk("unable to kmalloc macRxMemPool structure.\n");
		return -1;
	}
#endif
#ifdef CONFIG_TC3162_DMEM
	mac_p->macTxMemPool_p = (macTxMemPool_t *) alloc_sram(sizeof(macTxMemPool_t));
	if (mac_p->macTxMemPool_p == NULL)
#endif
#ifdef MT7510_DMA_DSCP_CACHE
	{
		/*Allocate 3*space, 1st space for hardware DMA, 2nd and 3rd for virutal space mapping*/
		temp_mem_p = (char *) dma_alloc_noncoherent(NULL, sizeof(macTxMemPool_t)/2*3, &mac_p->macTxMemPool_phys_p, GFP_KERNEL);

		if (temp_mem_p == NULL) {
			printk("unable to kmalloc macTxMemPool structure.\n");
			return -1;
		}
		tx_dscp_alloc = temp_mem_p;
		dma_cache_wback_inv((unsigned long)temp_mem_p, sizeof(macTxMemPool_t)/2*3);
		mac_p->macTxMemPool_p = (macTxMemPool_t *)(temp_mem_p + sizeof(macTxMemPool_t)/2);
	}
#else
	mac_p->macTxMemPool_p = (macTxMemPool_t *) dma_alloc_coherent(NULL, sizeof(macTxMemPool_t), &mac_p->macTxMemPool_phys_p, GFP_KERNEL);
	if (mac_p->macTxMemPool_p == NULL) {
		printk("unable to kmalloc macTxMemPool structure.\n");
		return -1;
	}
#endif
#ifdef MT7510_DMA_DSCP_CACHE
	/*Init DSCP cache register*/
	macSetDMADescrCacheReg(mac_p);
#endif

	/* ----- Set up the paramters ----- */
	macDefaultParaSet(mac_p);

	/* ----- Get the Mac address ----- */
	macGetMacAddr(mac_p);

	/* ----- Initialize Tx/Rx descriptors ----- */
	if (macDrvDescripInit(mac_p) != 0)
		return -1;

	/* ----- Initialize the phy chip ----- */
	if (macSetUpPhy(mac_p)) 
		return -1;

	/* ----- Initialize Registers ----- */
	macDrvRegInit(mac_p);

	mac_p->statisticOn = MAC_STATISTIC_ON;

	macInitialized = 1;
	macDrvStart();
	#ifdef TCPHY_SUPPORT
	//tcephydbgcmd();
	#ifdef TC2031_DEBUG
	tcethercmd(); 
	#endif
	#endif
	#if defined(WAN2LAN) || defined(CONFIG_TC3162_ADSL)
	#ifndef TC2031_SUPPORT 
	/* ------ xyzhu_nj_091104:Enable special Tag default ----- */
	macRxPortEnable(0);
	#endif
	#endif

	init_ethernet_port_map();
	
	return 0;
}

int macSwReset(macAdapter_t *mac_p)
{
	return 0;
}

/************************************************************************
*     A D S L   R E L A T E D    F U N C T I O N S
*************************************************************************
*/

static void tc3262_gmac_poll_func(unsigned long data)
{
#ifdef CONFIG_TC3162_ADSL
	uint8 modemst;

	if (adsl_dev_ops == NULL)
		goto down_proc;

	adsl_dev_ops->query(ADSL_QUERY_STATUS, &modemst, NULL);		  
	switch (modemst) {
		case ADSL_MODEM_STATE_DOWN:
down_proc:
			mod_timer(&eth_poll_timer, jiffies + msecs_to_jiffies(500));
			break;
		case ADSL_MODEM_STATE_WAIT_INIT:
			mod_timer(&eth_poll_timer, jiffies + msecs_to_jiffies(100));
			break;
		case ADSL_MODEM_STATE_INIT:
			mod_timer(&eth_poll_timer, jiffies + msecs_to_jiffies(100));
			break;
		case ADSL_MODEM_STATE_UP:
			mod_timer(&eth_poll_timer, jiffies + msecs_to_jiffies(500));
			break;
	}
#endif
}

/* ADSL RTS dump function */

void TCConsole(uint8 mode)
{
}
EXPORT_SYMBOL(TCConsole);

void uartMacPutchar(int ch)
{
}
EXPORT_SYMBOL(uartMacPutchar);

uint32 GetIpAddr(void)
{
	return 0;
}
EXPORT_SYMBOL(GetIpAddr);

uint8 *GetMacAddr(void)
{
	static uint8 macAddr[7];
  
  	memcpy(macAddr, def_mac_addr, 6);
	macAddr[6] = 0x0;
	return macAddr;
}
EXPORT_SYMBOL(GetMacAddr);

/************************************************************************
*     E T H E R N E T    D E V I C E   P R O C  D E F I N I T I O N S
*************************************************************************
*/

#define CHK_BUF() pos = begin + index; if (pos < off) { index = 0; begin = pos; }; if (pos > off + count) goto done;

static int getETHLinkSt(char *buf)
{
	uint16 index = 0;

	if (!macInitialized) {
		index += sprintf(buf+index, "Down\n");
		return index;
	}

	if ((mac_p->macPhyLinkProfile_p->enetMode & LAN_ST_LINK_UP) == 0) {
		index += sprintf(buf+index, "Down\n");
		return index;
	}

	if (mac_p->macPhyLinkProfile_p->enetMode & LAN_ST_1000MB)
		index += sprintf(buf+index, "1000M/");
	else if (mac_p->macPhyLinkProfile_p->enetMode & LAN_ST_100MB)
		index += sprintf(buf+index, "100M/");
	else 
		index += sprintf(buf+index, "10M/");
	if (mac_p->macPhyLinkProfile_p->enetMode & LAN_ST_FULL_DUPLEX)
		index += sprintf(buf+index, "Full Duplex\n");
	else 
		index += sprintf(buf+index, "Half Duplex\n");
	return index;
}

static int getGSWLinkSt(char *buf)
{
	uint16 index = 0;
	uint32 reg;
	int port;
	int speed;

	if (!macInitialized) {
		return index;
	}

	for (port = 0; port <= 6; port++) {
		index += sprintf(buf+index, "Internal Port %d: ", port);
		if (isMT7520G || isMT7525G){
			reg = gswPbusRead(GSW_PMSR(port)-GSW_BASE);
		}else{
		reg = switch_reg_read(GSW_PMSR(port));
		}
		if (!(reg & MAC_LINK_STS)) {
			index += sprintf(buf+index, "Down\n");
			continue;
		}

		speed = (reg & MAC_SPD_STS) >> MAC_SPD_STS_SHIFT;
		if (speed == 3)
			index += sprintf(buf+index, "invalid(10G)/");
		else if (speed == PN_SPEED_1000M)
			index += sprintf(buf+index, "1000M/");
		else if (speed == PN_SPEED_100M)
			index += sprintf(buf+index, "100M/");
		else 
			index += sprintf(buf+index, "10M/");

		if (reg & MAC_DPX_STS)
			index += sprintf(buf+index, "Full Duplex");
		else 
			index += sprintf(buf+index, "Half Duplex");

		if (reg & (TX_FC_STS | RX_FC_STS)) {
			index += sprintf(buf+index, " FC:");
			if (reg & TX_FC_STS) 
				index += sprintf(buf+index, " TX");
			if (reg & RX_FC_STS) 
				index += sprintf(buf+index, " RX");
		}

		if (reg & EEE100_STS) 
			index += sprintf(buf+index, " EEE100");
		if (reg & EEE1G_STS) 
			index += sprintf(buf+index, " EEE1G");

		index += sprintf(buf+index, "\n");
	}
	return index;
}

/******************************************************************************
******************************************************************************/

#ifdef TCSUPPORT_FREE_BOOTBASE
#define FLAG_ADDR		0x80001fff;
#else
#define FLAG_ADDR		0x8001ffff;
#endif

uint16 lan_port_tpid[MT7530_SWIC_PORTNUM];
uint16 ether_wan_tpid = 0;
uint16 cpu_port_tpid = 0;
uint8 use_ext_switch = 0;//0:inner only 1:ext switch

extern int wan_port_id;
extern char lan_port_map[ RT63365_SWIC_PORTNUM ];
extern char switch_port_map[ RT63365_SWIC_PORTNUM ];

static int special_tpid_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int i = 0,index = 0;
	off_t pos=0, begin=0 ;
	
	index += sprintf(buf+index, "Switch Port TPID:\nPort   TPID\n");
	for (i = 0; i < MT7530_SWIC_PORTNUM; i++) 
	{
		if(use_ext_switch == 1)
			index += sprintf(buf+index, "%d:    0x%x\n", i,((gswPbusRead(0x2010 + (i * 0x100)) & 0xFFFF0000) >> 16));
		else
			index += sprintf(buf+index, "%d:    0x%x\n", i,((read_reg_word(GSW_BASE + 0x2010 + (i * 0x100)) & 0xFFFF0000) >> 16));
		CHK_BUF();
	}
	index += sprintf(buf+index, "Lan Port TPID:\nPort   TPID\n");
	for (i = 0; i < MT7530_SWIC_PORTNUM; i++) 
	{
		index += sprintf(buf+index, "%d:    0x%x\n", i, lan_port_tpid[i]);
		CHK_BUF();
	}
	if (wan_port_id >= 0 && wan_port_id<MT7530_SWIC_PORTNUM-1)
		index += sprintf(buf+index, "EtherWanPort: 0x%x\n", ether_wan_tpid);

	index += sprintf(buf+index, "Cpu Port: 0x%x\n", cpu_port_tpid);

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

static int special_tpid_write_proc(struct file *file, const char *buffer,unsigned long count, void *data)
{
	char val_string[32];
	uint16 addr = 0,tpid = 0;
	uint32 value = 0;
	int i = 0;
	char tmp = 0;
	int port, port_tpid;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	if ((1 != sscanf(val_string, "%d:%x", &port, &port_tpid)) ||
		(port < 0) || (port > MT7530_SWIC_PORTNUM-1)){
		printk("Invalid param, must be like: portId[0~%d]:Tpid\n", MT7530_SWIC_PORTNUM-1);
		return -1;
	}
	tpid = port_tpid;

	/* update lan port tpid from reg & lan_port_map */
	if ((switch_port_map[port] >=0) && (switch_port_map[port]<MT7530_SWIC_PORTNUM))
		lan_port_tpid[switch_port_map[port]] = tpid;

	if (port == wan_port_id)
		ether_wan_tpid = tpid;
	else if (port == 6)
		cpu_port_tpid = tpid;

	addr = port * 0x100 + 0x2010;

	if(use_ext_switch == 1)
	{
		value = gswPbusRead(addr);
		value &= 0x0000FFFF;
		value |= (tpid << 16);
		gswPbusWrite(addr,value);
	}
	else
	{
		value = read_reg_word(GSW_BASE + addr);
		value &= 0x0000FFFF;
		value |= (tpid << 16);
		write_reg_word(GSW_BASE + addr,value);	
	}
	return count;
}

static int gpon_bootflag_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	off_t pos=0, begin=0 ;

	char *boot_flag = FLAG_ADDR;

	if (!macInitialized) {
		*eof = 1;
		return 0;
	}

	index += sprintf(buf+ index, "%d", *boot_flag) ;
	CHK_BUF() ;

	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0)		index = 0 ;
	if(index>count)		index = count ;
	return index ;
}

uint32 calcnt(uint32 basereg){
	int i;
	uint32 ret_val = 0;

	for(i=0; i<6;i++){
		if(i == wan_port_id)
			continue;
		ret_val += switch_reg_read(basereg + i*0x100);
	}

	return ret_val;
}
	
uint32 ext_cal_cnt(uint32 basereg){
	int i;
	uint32 ret_val = 0;
	for(i=0; i<6;i++){
		if(i == wan_port_id)
			continue;
		ret_val += gswPbusRead(basereg + i*0x100);
	}
	return ret_val;
}
static int eth_stats_read_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;

	if (!macInitialized) {
		*eof = 1;
		return 0;
	}

#ifdef TCSUPPORT_MT7510_FE //use switch to save count to webpage
	if(isMT7520G || isMT7525G)
	{
		index += sprintf(buf+index, "inOctets              = 0x%08lx, ", ext_cal_cnt(EXT_GSW_RX_OCL(0)));
		CHK_BUF();

		index += sprintf(buf+index, "inUnicastPkts         = 0x%08lx\n", ext_cal_cnt(EXT_GSW_RX_UNIC(0)));
		CHK_BUF();

		index += sprintf(buf+index, "inMulticastPkts       = 0x%08lx, ", ext_cal_cnt(EXT_GSW_RX_MULC(0)));
		CHK_BUF();

		index += sprintf(buf+index, "inDiscards            = 0x%08lx\n", ext_cal_cnt(EXT_GSW_RX_DROC(0)));
		CHK_BUF();

		index += sprintf(buf+index, "inErrors              = 0x%08lx, ", ext_cal_cnt(EXT_GSW_RX_ALIGE(0)) + ext_cal_cnt(EXT_GSW_RX_CRC(0)) + ext_cal_cnt(EXT_GSW_RX_RUNT(0)) + ext_cal_cnt(EXT_GSW_RX_FRGE(0)) + ext_cal_cnt(EXT_GSW_RX_LONG(0)));
		CHK_BUF();

		index += sprintf(buf+index, "outOctets             = 0x%08lx\n", ext_cal_cnt(EXT_GSW_TX_OCL(0)));
		CHK_BUF();

		index += sprintf(buf+index, "outUnicastPkts        = 0x%08lx, ", ext_cal_cnt(EXT_GSW_TX_UNIC(0)));
		CHK_BUF();
		
		index += sprintf(buf+index, "outMulticastPkts      = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_MULC(6))); //use port 6 to count multicast, include etherwan.
		CHK_BUF();
		
		index += sprintf(buf+index, "outDiscards           = 0x%08lx, ", ext_cal_cnt(EXT_GSW_TX_DROC(0)));
		CHK_BUF();
		
		index += sprintf(buf+index, "outErrors             = 0x%08lx\n", ext_cal_cnt(EXT_GSW_TX_COLC(0)));
		CHK_BUF();
	}
	else
	{
	index += sprintf(buf+index, "inOctets              = 0x%08lx, ", calcnt(GSW_RX_OCL(0)));
	CHK_BUF();
	
	index += sprintf(buf+index, "inUnicastPkts         = 0x%08lx\n", calcnt(GSW_RX_UNIC(0)));
	CHK_BUF();

	index += sprintf(buf+index, "inMulticastPkts       = 0x%08lx, ", calcnt(GSW_RX_MULC(0)));
	CHK_BUF();
	index += sprintf(buf+index, "inDiscards            = 0x%08lx\n", calcnt(GSW_RX_DROC(0)));
	CHK_BUF();

	index += sprintf(buf+index, "inErrors              = 0x%08lx, ", calcnt(GSW_RX_ALIGE(0)) + calcnt(GSW_RX_CRC(0)) + calcnt(GSW_RX_RUNT(0)) + calcnt(GSW_RX_FRGE(0)) + calcnt(GSW_RX_LONG(0)));
	CHK_BUF();
	index += sprintf(buf+index, "outOctets             = 0x%08lx\n", calcnt(GSW_TX_OCL(0)));
	CHK_BUF();

	index += sprintf(buf+index, "outUnicastPkts        = 0x%08lx, ", calcnt(GSW_TX_UNIC(0)));
	CHK_BUF();
		index += sprintf(buf+index, "outMulticastPkts      = 0x%08lx\n", switch_reg_read(GSW_RX_MULC(6))); //use port 6 to count multicast, include etherwan.
	CHK_BUF();

	index += sprintf(buf+index, "outDiscards           = 0x%08lx, ", calcnt(GSW_TX_DROC(0)));
	CHK_BUF();
	index += sprintf(buf+index, "outErrors             = 0x%08lx\n", calcnt(GSW_TX_COLC(0)));
	CHK_BUF();
	}

	index += sprintf(buf+index, "PDMA inOctets         = 0x%08lx, ", mac_p->macStat.MIB_II.inOctets);
	CHK_BUF();
	index += sprintf(buf+index, "PDMA inUnicastPkts    = 0x%08lx\n", mac_p->macStat.MIB_II.inUnicastPkts);
	CHK_BUF();

	index += sprintf(buf+index, "PDMA inMulticastPkts  = 0x%08lx, ", mac_p->macStat.MIB_II.inMulticastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "PDMA inDiscards       = 0x%08lx\n", mac_p->macStat.MIB_II.inDiscards);
	CHK_BUF();

	index += sprintf(buf+index, "PDMA inErrors         = 0x%08lx, ", mac_p->macStat.MIB_II.inErrors);
	CHK_BUF();
	index += sprintf(buf+index, "PDMA outOctets        = 0x%08lx\n", mac_p->macStat.MIB_II.outOctets);
	CHK_BUF();

	index += sprintf(buf+index, "PDMA outUnicastPkts   = 0x%08lx, ", mac_p->macStat.MIB_II.outUnicastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "PDMA outMulticastPkts = 0x%08lx\n", mac_p->macStat.MIB_II.outMulticastPkts);
	CHK_BUF();

	index += sprintf(buf+index, "PDMA outDiscards      = 0x%08lx, ", mac_p->macStat.MIB_II.outDiscards);
	CHK_BUF();
	index += sprintf(buf+index, "PDMA outErrors        = 0x%08lx\n", mac_p->macStat.MIB_II.outErrors);
	CHK_BUF();
#else
	index += sprintf(buf+index, "inOctets              = 0x%08lx, ", mac_p->macStat.MIB_II.inOctets);
	CHK_BUF();
	index += sprintf(buf+index, "inUnicastPkts         = 0x%08lx\n", mac_p->macStat.MIB_II.inUnicastPkts);
	CHK_BUF();

	index += sprintf(buf+index, "inMulticastPkts       = 0x%08lx, ", mac_p->macStat.MIB_II.inMulticastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "inDiscards            = 0x%08lx\n", mac_p->macStat.MIB_II.inDiscards);
	CHK_BUF();

	index += sprintf(buf+index, "inErrors              = 0x%08lx, ", mac_p->macStat.MIB_II.inErrors);
	CHK_BUF();
	index += sprintf(buf+index, "outOctets             = 0x%08lx\n", mac_p->macStat.MIB_II.outOctets);
	CHK_BUF();

	index += sprintf(buf+index, "outUnicastPkts        = 0x%08lx, ", mac_p->macStat.MIB_II.outUnicastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "outMulticastPkts      = 0x%08lx\n", mac_p->macStat.MIB_II.outMulticastPkts);
	CHK_BUF();

	index += sprintf(buf+index, "outDiscards           = 0x%08lx, ", mac_p->macStat.MIB_II.outDiscards);
	CHK_BUF();
	index += sprintf(buf+index, "outErrors             = 0x%08lx\n", mac_p->macStat.MIB_II.outErrors);
	CHK_BUF();
#endif
	index += sprintf(buf+index, "\n[ Statistics Display ]\n");
	CHK_BUF();
	index += sprintf(buf+index, "txJabberTimeCnt       = 0x%08lx  ", mac_p->macStat.inSilicon.txJabberTimeCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txLossOfCarrierCnt    = 0x%08lx\n", mac_p->macStat.inSilicon.txLossOfCarrierCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txNoCarrierCnt        = 0x%08lx  ", mac_p->macStat.inSilicon.txNoCarrierCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txLateCollisionCnt    = 0x%08lx\n", mac_p->macStat.inSilicon.txLateCollisionCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txExCollisionCnt      = 0x%08lx  ", mac_p->macStat.inSilicon.txExCollisionCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txHeartbeatFailCnt    = 0x%08lx\n", mac_p->macStat.inSilicon.txHeartbeatFailCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txCollisionCnt        = 0x%08lx  ", mac_p->macStat.inSilicon.txCollisionCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txExDeferralCnt       = 0x%08lx\n", mac_p->macStat.inSilicon.txExDeferralCnt);
	CHK_BUF();
	index += sprintf(buf+index, "txUnderRunCnt         = 0x%08lx  ", mac_p->macStat.inSilicon.txUnderRunCnt);
	CHK_BUF();

	index += sprintf(buf+index, "rxAlignErr            = 0x%08lx\n", mac_p->macStat.inSilicon.rxAlignErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxDribblingErr        = 0x%08lx  ", mac_p->macStat.inSilicon.rxDribblingErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxSymbolErr           = 0x%08lx\n", mac_p->macStat.inSilicon.rxSymbolErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxMiiErr              = 0x%08lx  ", mac_p->macStat.inSilicon.rxMiiErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxCollisionErr        = 0x%08lx\n", mac_p->macStat.inSilicon.rxCollisionErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxCrcErr              = 0x%08lx  ", mac_p->macStat.inSilicon.rxCrcErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxEtherFrameLengthErr = 0x%08lx\n", mac_p->macStat.inSilicon.rxEtherFrameLengthErr);
	CHK_BUF();
	index += sprintf(buf+index, "rx802p3FrameLengthErr = 0x%08lx  ", mac_p->macStat.inSilicon.rx802p3FrameLengthErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxPktIPChkSumErr      = 0x%08lx\n", mac_p->macStat.inSilicon.rxPktIPChkSumErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxRuntErr             = 0x%08lx  ", mac_p->macStat.inSilicon.rxRuntErr);
	CHK_BUF();
	index += sprintf(buf+index, "rxLongErr             = 0x%08lx\n", mac_p->macStat.inSilicon.rxLongErr);
	CHK_BUF();

	index += sprintf(buf+index, "\n[ Extra information ]\n");
	CHK_BUF();
	index += sprintf(buf+index, "Rx Descriptor idx     = 0x%08lx  ", mac_p->rxCurrentDescp);
	CHK_BUF();
	index += sprintf(buf+index, "Tx Descriptor idx     = 0x%08lx\n", mac_p->txCurrentDescp[0]);
	CHK_BUF();
 	index += sprintf(buf+index, "Rx Enqueued cnt       = 0x%08lx  ", mac_p->macStat.inSilicon.rxEnQueueNum);
	CHK_BUF();
  	index += sprintf(buf+index, "Tx Enqueued cnt       = 0x%08lx\n", mac_p->macStat.inSilicon.txEnQueueNum);		
	CHK_BUF();
  	index += sprintf(buf+index, "Rx Dequeued cnt       = 0x%08lx  ", mac_p->macStat.inSilicon.rxDeQueueNum);
	CHK_BUF();
  	index += sprintf(buf+index, "Tx Dequeued cnt       = 0x%08lx\n", mac_p->macStat.inSilicon.txDeQueueNum);
	CHK_BUF();
  	index += sprintf(buf+index, "Tx Buf UnReleased cnt = 0x%08lx  ", mac_p->txUnReleasedBufCnt[0]);
	CHK_BUF();
  	index += sprintf(buf+index, "Tx Buf UnReleased idx = 0x%08lx\n", mac_p->txUnReleasedDescp[0]);
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

static int eth_stats_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	if (!macInitialized) {
		return 0;
	}

	memset(&mac_p->macStat.MIB_II, 0, sizeof(macMIB_II_t));
	memset(&mac_p->macStat.inSilicon, 0, sizeof(inSiliconStat_t));

	return count;
}

#ifdef TCSUPPORT_WAN_ETHER

static int eth1_stats_read_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;

	if (!macInitialized) {
		*eof = 1;
		return 0;
	}

#ifdef TCSUPPORT_MT7510_FE //use switch to save count to webpage

	index += sprintf(buf+index, "inOctets              = 0x%08lx, ", switch_reg_read(GSW_RX_OCL(wan_port_id)));
	CHK_BUF();
	
	index += sprintf(buf+index, "inUnicastPkts         = 0x%08lx\n", switch_reg_read(GSW_RX_UNIC(wan_port_id)));
	CHK_BUF();

	index += sprintf(buf+index, "inMulticastPkts       = 0x%08lx, ", switch_reg_read(GSW_RX_MULC(wan_port_id)));
	CHK_BUF();
	index += sprintf(buf+index, "inDiscards            = 0x%08lx\n", switch_reg_read(GSW_RX_DROC(wan_port_id)));
	CHK_BUF();

	index += sprintf(buf+index, "inErrors              = 0x%08lx, ", switch_reg_read(GSW_RX_ALIGE(wan_port_id)) + switch_reg_read(GSW_RX_CRC(wan_port_id))
														+ switch_reg_read(GSW_RX_RUNT(wan_port_id)) + switch_reg_read(GSW_RX_FRGE(wan_port_id)) + switch_reg_read(GSW_RX_LONG(wan_port_id)));
	CHK_BUF();
	index += sprintf(buf+index, "outOctets             = 0x%08lx\n", switch_reg_read(GSW_TX_OCL(wan_port_id)));
	CHK_BUF();

	index += sprintf(buf+index, "outUnicastPkts        = 0x%08lx, ", switch_reg_read(GSW_TX_UNIC(wan_port_id)));
	CHK_BUF();
	index += sprintf(buf+index, "outMulticastPkts      = 0x%08lx\n", switch_reg_read(GSW_TX_MULC(wan_port_id))); 

	index += sprintf(buf+index, "outDiscards           = 0x%08lx, ", switch_reg_read(GSW_TX_DROC(wan_port_id)));
	CHK_BUF();
	index += sprintf(buf+index, "outErrors             = 0x%08lx\n", switch_reg_read(GSW_TX_COLC(wan_port_id)));
	CHK_BUF();

	index += sprintf(buf+index, "PDMA inOctets         = 0x%08lx, ", mac_wan_p->macStat.MIB_II.inOctets);
	CHK_BUF();
	index += sprintf(buf+index, "PDMA inUnicastPkts    = 0x%08lx\n", mac_wan_p->macStat.MIB_II.inUnicastPkts);
	CHK_BUF();

	index += sprintf(buf+index, "PDMA inMulticastPkts  = 0x%08lx, ", mac_wan_p->macStat.MIB_II.inMulticastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "PDMA inDiscards       = 0x%08lx\n", mac_wan_p->macStat.MIB_II.inDiscards);
	CHK_BUF();

	index += sprintf(buf+index, "PDMA inErrors         = 0x%08lx, ", mac_wan_p->macStat.MIB_II.inErrors);
	CHK_BUF();
	index += sprintf(buf+index, "PDMA outOctets        = 0x%08lx\n", mac_wan_p->macStat.MIB_II.outOctets);
	CHK_BUF();

	index += sprintf(buf+index, "PDMA outUnicastPkts   = 0x%08lx, ", mac_wan_p->macStat.MIB_II.outUnicastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "PDMA outMulticastPkts = 0x%08lx\n", mac_wan_p->macStat.MIB_II.outMulticastPkts);
	CHK_BUF();

	index += sprintf(buf+index, "PDMA outDiscards      = 0x%08lx, ", mac_wan_p->macStat.MIB_II.outDiscards);
	CHK_BUF();
	index += sprintf(buf+index, "PDMA outErrors        = 0x%08lx\n", mac_wan_p->macStat.MIB_II.outErrors);
	CHK_BUF();
#else
	index += sprintf(buf+index, "inOctets              = 0x%08lx, ", mac_wan_p->macStat.MIB_II.inOctets);
	CHK_BUF();
	index += sprintf(buf+index, "inUnicastPkts         = 0x%08lx\n", mac_wan_p->macStat.MIB_II.inUnicastPkts);
	CHK_BUF();

	index += sprintf(buf+index, "inMulticastPkts       = 0x%08lx, ", mac_wan_p->macStat.MIB_II.inMulticastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "inDiscards            = 0x%08lx\n", mac_wan_p->macStat.MIB_II.inDiscards);
	CHK_BUF();

	index += sprintf(buf+index, "inErrors              = 0x%08lx, ", mac_wan_p->macStat.MIB_II.inErrors);
	CHK_BUF();
	index += sprintf(buf+index, "outOctets             = 0x%08lx\n", mac_wan_p->macStat.MIB_II.outOctets);
	CHK_BUF();

	index += sprintf(buf+index, "outUnicastPkts        = 0x%08lx, ", mac_wan_p->macStat.MIB_II.outUnicastPkts);
	CHK_BUF();
	index += sprintf(buf+index, "outMulticastPkts      = 0x%08lx\n", mac_wan_p->macStat.MIB_II.outMulticastPkts);
	CHK_BUF();

	index += sprintf(buf+index, "outDiscards           = 0x%08lx, ", mac_wan_p->macStat.MIB_II.outDiscards);
	CHK_BUF();
	index += sprintf(buf+index, "outErrors             = 0x%08lx\n", mac_wan_p->macStat.MIB_II.outErrors);
	CHK_BUF();
#endif
	
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

static int eth1_stats_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	if (!macInitialized) {
		return 0;
	}

	memset(&mac_wan_p->macStat.MIB_II, 0, sizeof(macMIB_II_t));
	memset(&mac_wan_p->macStat.inSilicon, 0, sizeof(inSiliconStat_t));

	return count;
}
#endif
static int gsw_stats_read_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int port;

	if (!macInitialized) {
		*eof = 1;
		return 0;
	}

	for (port = 0; port <= 6; port++) {
		index += sprintf(buf+index, "[ Port %d ]\n", port);
		CHK_BUF();
#if defined(TCSUPPORT_MT7510_FE) || defined(MT7530_SUPPORT)
		if (isMT7520G || isMT7525G){
			index += sprintf(buf+index, "Rx Unicase Pkts        = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_UNIC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Multicast Pkts      = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_MULC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Broadcast Pkts      = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_BROC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Align Error         = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_ALIGE(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx CRC Error           = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_CRC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Under Size Pkts     = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_RUNT(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Fragment Error      = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_FRGE(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Over Size Pkts      = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_LONG(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Jabber Error        = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_JABE(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Pause Pkts          = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_PAUC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Drop Pkts           = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_DROC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx ING Drop Pkts       = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_INGC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx ARL Drop Pkts       = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_ARLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx FILTER Drop Pkts    = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_FILC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Unicase Pkts        = 0x%08lx, ", read_reg_word(GSW_TX_UNIC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Multicast Pkts      = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_MULC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Broadcast Pkts      = 0x%08lx, ", gswPbusRead(EXT_GSW_TX_BROC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Collision           = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_COLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Single Collision    = 0x%08lx, ", gswPbusRead(EXT_GSW_TX_SCOLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Multiple Collision  = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_MCOLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Defer               = 0x%08lx, ", gswPbusRead(EXT_GSW_TX_DEFC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Late Collision      = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_LCOLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx eXcessive Collision = 0x%08lx, ", gswPbusRead(EXT_GSW_TX_ECOLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Pause Pkts          = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_PAUC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Drop Pkts           = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_DROC(port)));
			CHK_BUF();
		}else{
		index += sprintf(buf+index, "Rx Unicase Pkts        = 0x%08lx, ", switch_reg_read(GSW_RX_UNIC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Multicast Pkts      = 0x%08lx\n", switch_reg_read(GSW_RX_MULC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Broadcast Pkts      = 0x%08lx, ", switch_reg_read(GSW_RX_BROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Align Error         = 0x%08lx\n", switch_reg_read(GSW_RX_ALIGE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx CRC Error           = 0x%08lx, ", switch_reg_read(GSW_RX_CRC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Under Size Pkts     = 0x%08lx\n", switch_reg_read(GSW_RX_RUNT(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Fragment Error      = 0x%08lx, ", switch_reg_read(GSW_RX_FRGE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Over Size Pkts      = 0x%08lx\n", switch_reg_read(GSW_RX_LONG(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Jabber Error        = 0x%08lx, ", switch_reg_read(GSW_RX_JABE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Pause Pkts          = 0x%08lx\n", switch_reg_read(GSW_RX_PAUC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Drop Pkts           = 0x%08lx, ", switch_reg_read(GSW_RX_DROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx ING Drop Pkts       = 0x%08lx\n", switch_reg_read(GSW_RX_INGC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx ARL Drop Pkts       = 0x%08lx, ", switch_reg_read(GSW_RX_ARLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx FILTER Drop Pkts    = 0x%08lx\n", switch_reg_read(GSW_RX_FILC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Unicase Pkts        = 0x%08lx, ", switch_reg_read(GSW_TX_UNIC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Multicast Pkts      = 0x%08lx\n", switch_reg_read(GSW_TX_MULC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Broadcast Pkts      = 0x%08lx, ", switch_reg_read(GSW_TX_BROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Collision           = 0x%08lx\n", switch_reg_read(GSW_TX_COLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Single Collision    = 0x%08lx, ", switch_reg_read(GSW_TX_SCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Multiple Collision  = 0x%08lx\n", switch_reg_read(GSW_TX_MCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Defer               = 0x%08lx, ", switch_reg_read(GSW_TX_DEFC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Late Collision      = 0x%08lx\n", switch_reg_read(GSW_TX_LCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx eXcessive Collision = 0x%08lx, ", switch_reg_read(GSW_TX_ECOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Pause Pkts          = 0x%08lx\n", switch_reg_read(GSW_TX_PAUC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Drop Pkts           = 0x%08lx\n", switch_reg_read(GSW_TX_DROC(port)));
		CHK_BUF();
		}
#else
		index += sprintf(buf+index, "Rx Good Pkts          = 0x%08lx, ", (read_reg_word(GSW_RGPC(port)) & RX_GOOD_CNT) >> RX_GOOD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Rx Bad Pkts           = 0x%08lx\n", (read_reg_word(GSW_RGPC(port)) & RX_BAD_CNT) >> RX_BAD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Rx Good Bytes         = 0x%08lx, ", read_reg_word(GSW_RGOC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Bad Bytes          = 0x%08lx\n", read_reg_word(GSW_RBOC(port)));		
		CHK_BUF();
		index += sprintf(buf+index, "Rx CTRL Drop Pkts     = 0x%08lx, ", (read_reg_word(GSW_REPC1(port)) & RX_CTRL_DROP_CNT) >> RX_CTRL_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Rx ING Drop Pkts      = 0x%08lx\n", (read_reg_word(GSW_REPC1(port)) & RX_ING_DROP_CNT) >> RX_ING_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Rx ARL Drop Pkts      = 0x%08lx, ", (read_reg_word(GSW_REPC2(port)) & RX_ARL_DROP_CNT) >> RX_ARL_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Rx FILTER Drop Pkts   = 0x%08lx\n", (read_reg_word(GSW_REPC2(port)) & RX_FILTER_DROP_CNT) >> RX_FILTER_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Tx Good Pkts          = 0x%08lx, ", (read_reg_word(GSW_TGPC(port)) & TX_GOOD_CNT) >> TX_GOOD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Tx Bad Pkts           = 0x%08lx\n", (read_reg_word(GSW_TGPC(port)) & TX_BAD_CNT) >> TX_BAD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Tx Good Bytes         = 0x%08lx, ", read_reg_word(GSW_TGOC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Bad Bytes          = 0x%08lx\n", read_reg_word(GSW_TBOC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Drop Pkts          = 0x%08lx\n", read_reg_word(GSW_TEPC(port)));
		CHK_BUF();
#endif
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
#if defined(MT7530_SUPPORT)
static int int_gsw_stats_read_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int port;

	if (!macInitialized) {
		*eof = 1;
		return 0;
	}

	for (port = 5; port <= 6; port++) {
		index += sprintf(buf+index, "[ Port %d ]\n", port);
		CHK_BUF();
#if defined(TCSUPPORT_MT7510_FE)
		index += sprintf(buf+index, "Rx Unicase Pkts        = 0x%08lx, ", read_reg_word(GSW_RX_UNIC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Multicast Pkts      = 0x%08lx\n", read_reg_word(GSW_RX_MULC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Broadcast Pkts      = 0x%08lx, ", read_reg_word(GSW_RX_BROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Align Error         = 0x%08lx\n", read_reg_word(GSW_RX_ALIGE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx CRC Error           = 0x%08lx, ", read_reg_word(GSW_RX_CRC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Under Size Pkts     = 0x%08lx\n", read_reg_word(GSW_RX_RUNT(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Fragment Error      = 0x%08lx, ", read_reg_word(GSW_RX_FRGE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Over Size Pkts      = 0x%08lx\n", read_reg_word(GSW_RX_LONG(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Jabber Error        = 0x%08lx, ", read_reg_word(GSW_RX_JABE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Pause Pkts          = 0x%08lx\n", read_reg_word(GSW_RX_PAUC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Drop Pkts           = 0x%08lx, ", read_reg_word(GSW_RX_DROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx ING Drop Pkts       = 0x%08lx\n", read_reg_word(GSW_RX_INGC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx ARL Drop Pkts       = 0x%08lx, ", read_reg_word(GSW_RX_ARLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx FILTER Drop Pkts    = 0x%08lx\n", read_reg_word(GSW_RX_FILC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Unicase Pkts        = 0x%08lx, ", read_reg_word(GSW_TX_UNIC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Multicast Pkts      = 0x%08lx\n", read_reg_word(GSW_TX_MULC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Broadcast Pkts      = 0x%08lx, ", read_reg_word(GSW_TX_BROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Collision           = 0x%08lx\n", read_reg_word(GSW_TX_COLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Single Collision    = 0x%08lx, ", read_reg_word(GSW_TX_SCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Multiple Collision  = 0x%08lx\n", read_reg_word(GSW_TX_MCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Defer               = 0x%08lx, ", read_reg_word(GSW_TX_DEFC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Late Collision      = 0x%08lx\n", read_reg_word(GSW_TX_LCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx eXcessive Collision = 0x%08lx, ", read_reg_word(GSW_TX_ECOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Pause Pkts          = 0x%08lx\n", read_reg_word(GSW_TX_PAUC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Drop Pkts           = 0x%08lx\n", read_reg_word(GSW_TX_DROC(port)));
		CHK_BUF();;

#else
		index += sprintf(buf+index, "Rx Good Pkts          = 0x%08lx, ", (read_reg_word(GSW_RGPC(port)) & RX_GOOD_CNT) >> RX_GOOD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Rx Bad Pkts           = 0x%08lx\n", (read_reg_word(GSW_RGPC(port)) & RX_BAD_CNT) >> RX_BAD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Rx Good Bytes         = 0x%08lx, ", read_reg_word(GSW_RGOC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Bad Bytes          = 0x%08lx\n", read_reg_word(GSW_RBOC(port)));		
		CHK_BUF();
		index += sprintf(buf+index, "Rx CTRL Drop Pkts     = 0x%08lx, ", (read_reg_word(GSW_REPC1(port)) & RX_CTRL_DROP_CNT) >> RX_CTRL_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Rx ING Drop Pkts      = 0x%08lx\n", (read_reg_word(GSW_REPC1(port)) & RX_ING_DROP_CNT) >> RX_ING_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Rx ARL Drop Pkts      = 0x%08lx, ", (read_reg_word(GSW_REPC2(port)) & RX_ARL_DROP_CNT) >> RX_ARL_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Rx FILTER Drop Pkts   = 0x%08lx\n", (read_reg_word(GSW_REPC2(port)) & RX_FILTER_DROP_CNT) >> RX_FILTER_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Tx Good Pkts          = 0x%08lx, ", (read_reg_word(GSW_TGPC(port)) & TX_GOOD_CNT) >> TX_GOOD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Tx Bad Pkts           = 0x%08lx\n", (read_reg_word(GSW_TGPC(port)) & TX_BAD_CNT) >> TX_BAD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "Tx Good Bytes         = 0x%08lx, ", read_reg_word(GSW_TGOC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Bad Bytes          = 0x%08lx\n", read_reg_word(GSW_TBOC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Drop Pkts          = 0x%08lx\n", read_reg_word(GSW_TEPC(port)));
		CHK_BUF();

#endif
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
#endif

//#if defined(GEN_1588_PKT_7530_VERIFY)
//MTK20120829_MT7530_1588pkt_generation, Start[
unsigned char sync_pkt[100] = {
  0x01, 0x00, 0x5e, 0x00, 0x01, 0x81, 0x00, 0xaa, 0xbb, 0x01, 0x23, 0x45, 0x08, 0x00, 0x45, 0x00,
  0x00, 0x48, 0x00, 0x00, 0x40, 0x00, 0x01, 0x11, 0xd6, 0x7a, 0xc0, 0xa8, 0x01, 0x01, 0xe0, 0x00,
  0x01, 0x81, 0x01, 0x3f, 0x01, 0x3f, 0x00, 0x34, 0x5f, 0x00, 0x80, 0x02, 0x00, 0x2c, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa,
  0xbb, 0xff, 0xfe, 0x01, 0x23, 0x45, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x30,
  0x8a, 0xca, 0x1f, 0xdd, 0xa0, 0xe4
};

unsigned char follow_pkt[100] = {
  0x01, 0x00, 0x5e, 0x00, 0x01, 0x81, 0x00, 0xaa, 0xbb, 0x01, 0x23, 0x45, 0x08, 0x00, 0x45, 0x00,
  0x00, 0x48, 0x00, 0x00, 0x40, 0x00, 0x01, 0x11, 0xd6, 0x6f, 0xc0, 0xa8, 0x01, 0x0c, 0xe0, 0x00,
  0x01, 0x81, 0x01, 0x40, 0x01, 0x40, 0x00, 0x34, 0xbb, 0x61, 0x88, 0x02, 0x00, 0x2c, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
  0x3d, 0xff, 0xfe, 0x81, 0x9f, 0x00, 0x00, 0x01, 0x00, 0x26, 0x02, 0x00, 0x00, 0x00, 0x4f, 0x30,
  0x8a, 0xf5, 0x39, 0x12, 0x23, 0x50
};

unsigned char delay_req_pkt[100] = {
  0x01, 0x00, 0x5e, 0x00, 0x00, 0x6b, 0x00, 0xaa, 0xbb, 0x01, 0x23, 0x45, 0x08, 0x00, 0x45, 0x00,
  0x00, 0x52, 0x00, 0x00, 0x40, 0x00, 0x01, 0x11, 0xd7, 0x86, 0xc0, 0xa8, 0x01, 0x01, 0xe0, 0x00,
  0x00, 0x6b, 0x01, 0x3f, 0x01, 0x3f, 0x00, 0x3e, 0xa4, 0x31, 0x82, 0x02, 0x00, 0x36, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa,
  0xbb, 0xff, 0xfe, 0x01, 0x23, 0x45, 0x00, 0x01, 0x00, 0x16, 0x05, 0x7f, 0x00, 0x00, 0x4f, 0x30,
  0x8a, 0xf8, 0x1f, 0x53, 0x55, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


unsigned int udp_checksum(const struct sk_buff *skb)
{
	unsigned int checksum = 0;
	unsigned int src_ip1, src_ip2, dest_ip1, dest_ip2;
	unsigned int src_port, dest_port;
	unsigned int udp_length;
	unsigned int udp_protocol;
	unsigned int data;
	unsigned int index;

	udp_protocol = (unsigned int)(*((unsigned char*)(&skb->data[23])));
	src_ip1 = (unsigned int)(*((unsigned short*)(&skb->data[26])));
	src_ip2 = (unsigned int)(*((unsigned short*)(&skb->data[28])));
	dest_ip1 = (unsigned int)(*((unsigned short*)(&skb->data[30])));
	dest_ip2 = (unsigned int)(*((unsigned short*)(&skb->data[32])));
	src_port = (unsigned int)(*((unsigned short*)(&skb->data[34])));
	dest_port = (unsigned int)(*((unsigned short*)(&skb->data[36])));
	udp_length = (unsigned int)(*((unsigned short*)(&skb->data[38])));

//	printk("src ip1: %x, src ip2: %x\n", src_ip1, src_ip2);
//	printk("dest ip1: %x, dest ip2: %x\n", dest_ip1, dest_ip2);
//	printk("src port: %x, dest port: %x\n", src_port, dest_port);
//	printk("udp_length: %x\n", udp_length);
//	printk("udp_protocol: %x\n", udp_protocol);
	data = 0;
	for (index=42; index<skb->len; index+=2){
		data += (unsigned int)(*((unsigned short*)(&skb->data[index])));
	}
//	printk("data: %x\n", data);

	checksum = src_ip1 + src_ip2 + dest_ip1 + dest_ip2 + src_port + dest_port + (2*udp_length) + data + udp_protocol;
//	printk("checksum1: %x\n", checksum);
	checksum += (checksum >> 16);
//	printk("checksum2: %x\n", checksum);
	checksum &= 0xffff;
//	printk("checksum3: %x\n", checksum);
	checksum = (~checksum);
//	printk("checksum4: %x\n", checksum);
	checksum &= 0xffff;
//	printk("checksum5: %x\n", checksum);

	return checksum;
}

int MT7530_gen_1588_packet(char flag)
{
//	int k;
	struct sk_buff *skb;
	int tx_len;
	int tx_priority;
	unsigned long checksum;
//	unsigned long sec, nsec;
	uint8 *tx_data;
	static short id=0;

	struct timespec cur_time;

	getnstimeofday(&cur_time);

	skb = dev_alloc_skb(RX_BUF_LEN);
	if (skb == NULL){
		printk("cannot allocate skb\n");
		return 0;
	}

	tx_len = 100;
	tx_data = skb_put(skb, tx_len);

	if (flag == 0)
		memcpy(tx_data, sync_pkt, 100);
	else if (flag == 1)
		memcpy(tx_data, follow_pkt, 100);
	else if (flag == 2)
		memcpy(tx_data, delay_req_pkt, 100);
	else {
		printk("error: 1588 packet type input\n");
		return 0;
	}

	tx_priority = 3;
	skb->priority = tx_priority;

	skb->data[76] = (unsigned char)(0x0);
	skb->data[77] = (unsigned char)(0x0);
	skb->data[78] = (unsigned char)((cur_time.tv_sec>>24) & 0xff);
	skb->data[79] = (unsigned char)((cur_time.tv_sec>>16) & 0xff);
	skb->data[80] = (unsigned char)((cur_time.tv_sec>>8) & 0xff);
	skb->data[81] = (unsigned char)((cur_time.tv_sec>>0) & 0xff);

	skb->data[82] = (unsigned char)((cur_time.tv_nsec>>24) & 0xff);
	skb->data[83] = (unsigned char)((cur_time.tv_nsec>>16) & 0xff);
	skb->data[84] = (unsigned char)((cur_time.tv_nsec>>8) & 0xff);
	skb->data[85] = (unsigned char)((cur_time.tv_nsec>>0) & 0xff);

	skb->data[72] = (unsigned char)((id >> 8) & 0xff);
	skb->data[73] = (unsigned char)(id & 0xff);
	id++;

	checksum = udp_checksum(skb);
	skb->data[40] = (checksum >> 8);
	skb->data[41] = (checksum & 0xff);

	tc3262_gmac_tx(skb, tc3262_gmac_dev);

	mdelay(100);

	return 0;
}

static int gen_1588_pkt_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];
	int flag;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	flag = simple_strtoul(val_string, NULL, 16);

	MT7530_gen_1588_packet((char)(flag));

	return count;
}
//MTK20120829_MT7530_1588pkt_generation, ]End
//#endif //GEN_1588_PKT_7530_VERIFY

static int gsw_stats_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	return count;
}

/* wplin addded 20120703 */
static int gsw_mibN_read_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data, uint portId)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int port;

	if (!macInitialized) {
		*eof = 1;
		return 0;
	}


	port = portId;
	{
#ifdef TCSUPPORT_MT7510_FE
		index += sprintf(buf+index, "[ Port %d ]\n", port);
		CHK_BUF();
		if (isMT7520G || isMT7525G) {
			index += sprintf(buf+index, "Rx Unicase Pkts        = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_UNIC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Multicast Pkts      = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_MULC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Broadcast Pkts      = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_BROC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Align Error         = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_ALIGE(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx CRC Error           = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_CRC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Under Size Pkts     = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_RUNT(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Fragment Error      = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_FRGE(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Over Size Pkts      = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_LONG(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Jabber Error        = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_JABE(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Pause Pkts          = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_PAUC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx Drop Pkts           = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_DROC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx ING Drop Pkts       = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_INGC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx ARL Drop Pkts       = 0x%08lx, ", gswPbusRead(EXT_GSW_RX_ARLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Rx FILTER Drop Pkts    = 0x%08lx\n", gswPbusRead(EXT_GSW_RX_FILC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Unicase Pkts        = 0x%08lx, ", read_reg_word(GSW_TX_UNIC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Multicast Pkts      = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_MULC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Broadcast Pkts      = 0x%08lx, ", gswPbusRead(EXT_GSW_TX_BROC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Collision           = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_COLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Single Collision    = 0x%08lx, ", gswPbusRead(EXT_GSW_TX_SCOLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Multiple Collision  = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_MCOLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Defer               = 0x%08lx, ", gswPbusRead(EXT_GSW_TX_DEFC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Late Collision      = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_LCOLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx eXcessive Collision = 0x%08lx, ", gswPbusRead(EXT_GSW_TX_ECOLC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Pause Pkts          = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_PAUC(port)));
			CHK_BUF();
			index += sprintf(buf+index, "Tx Drop Pkts           = 0x%08lx\n", gswPbusRead(EXT_GSW_TX_DROC(port)));
			CHK_BUF();			
		}else{
		index += sprintf(buf+index, "Rx Unicase Pkts        = 0x%08lx, ", read_reg_word(GSW_RX_UNIC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Multicast Pkts      = 0x%08lx\n", read_reg_word(GSW_RX_MULC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Broadcast Pkts      = 0x%08lx, ", read_reg_word(GSW_RX_BROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Align Error         = 0x%08lx\n", read_reg_word(GSW_RX_ALIGE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx CRC Error           = 0x%08lx, ", read_reg_word(GSW_RX_CRC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Under Size Pkts     = 0x%08lx\n", read_reg_word(GSW_RX_RUNT(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Fragment Error      = 0x%08lx, ", read_reg_word(GSW_RX_FRGE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Over Size PPkts     = 0x%08lx\n", read_reg_word(GSW_RX_LONG(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Jabber Error        = 0x%08lx, ", read_reg_word(GSW_RX_JABE(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Pause Pkts          = 0x%08lx\n", read_reg_word(GSW_RX_PAUC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx Drop Pkts           = 0x%08lx, ", read_reg_word(GSW_RX_DROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx ING Drop Pkts       = 0x%08lx\n", read_reg_word(GSW_RX_INGC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx ARL Drop Pkts       = 0x%08lx, ", read_reg_word(GSW_RX_ARLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Rx FILTER Drop Pkts    = 0x%08lx\n", read_reg_word(GSW_RX_FILC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Unicase Pkts        = 0x%08lx, ", read_reg_word(GSW_TX_UNIC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Multicast Pkts      = 0x%08lx\n", read_reg_word(GSW_TX_MULC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Broadcast Pkts      = 0x%08lx, ", read_reg_word(GSW_TX_BROC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Collision           = 0x%08lx\n", read_reg_word(GSW_TX_COLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Single Collision    = 0x%08lx, ", read_reg_word(GSW_TX_SCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Multiple Collision  = 0x%08lx\n", read_reg_word(GSW_TX_MCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Deffer              = 0x%08lx, ", read_reg_word(GSW_TX_DEFC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Late Collision      = 0x%08lx\n", read_reg_word(GSW_TX_LCOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx eXcessive Collision = 0x%08lx, ", read_reg_word(GSW_TX_ECOLC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Pause Pkts          = 0x%08lx\n", read_reg_word(GSW_TX_PAUC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "Tx Drop Pkts           = 0x%08lx\n", read_reg_word(GSW_TX_DROC(port)));
		CHK_BUF();
		}
#endif

		index += sprintf(buf+index, "------\n");
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

static int gsw_mib0_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 0);
}

static int gsw_mib1_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 1);
}
static int gsw_mib2_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 2);
}
static int gsw_mib3_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 3);
}
static int gsw_mib4_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 4);
}
static int gsw_mib5_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 5);
}
static int gsw_mib6_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return gsw_mibN_read_proc(buf, start, off, count, eof, data, 6);
}
/* end wplin added 20120703 */

static int gsw_link_st_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int len = getGSWLinkSt(buf);
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


static int eth_link_st_proc(char *buf, char **start, off_t off, int count,
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

static int eth_reg_dump_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int i;
	uint32 reg;

	if (!macInitialized) {
		*eof = 1;
		return 0;
	}

	index += sprintf(buf+index, "FE_DMA_GLO_CFG   (0x%08x) = 0x%08lx\n", FE_DMA_GLO_CFG, read_reg_word(FE_DMA_GLO_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "FE_RST_GLO       (0x%08x) = 0x%08lx\n", FE_RST_GLO, read_reg_word(FE_RST_GLO)); 	
	CHK_BUF();
	index += sprintf(buf+index, "FE_INT_STATUS    (0x%08x) = 0x%08lx\n", FE_INT_STATUS, read_reg_word(FE_INT_STATUS)); 	
	CHK_BUF();
#ifdef TCSUPPORT_MT7510_FE
	index += sprintf(buf+index, "FE_INT_Enable    (0x%08x) = 0x%08lx\n", FE_INT_ENABLE, read_reg_word(FE_INT_ENABLE));
	CHK_BUF();
#else
	index += sprintf(buf+index, "FC_DROP_STA      (0x%08x) = 0x%08lx\n", FC_DROP_STA, read_reg_word(FC_DROP_STA));
	CHK_BUF();
#endif
	index += sprintf(buf+index, "FOE_TS_T         (0x%08x) = 0x%08lx\n", FOE_TS_T, read_reg_word(FOE_TS_T));
	CHK_BUF();

	for (i = 0; i < TX_QUEUE_NUM; i++) {
		index += sprintf(buf+index, "TX_BASE_PTR(%d)   (0x%08x) = 0x%08lx\n", i, TX_BASE_PTR(i), read_reg_word(TX_BASE_PTR(i))); 	
		CHK_BUF();
		index += sprintf(buf+index, "TX_MAX_CNT(%d)    (0x%08x) = 0x%08lx\n", i, TX_MAX_CNT(i), read_reg_word(TX_MAX_CNT(i))); 	
		CHK_BUF();
		index += sprintf(buf+index, "TX_CTX_IDX(%d)    (0x%08x) = 0x%08lx\n", i, TX_CTX_IDX(i), read_reg_word(TX_CTX_IDX(i))); 	
		CHK_BUF();
		index += sprintf(buf+index, "TX_DTX_IDX(%d)    (0x%08x) = 0x%08lx\n", i, TX_DTX_IDX(i), read_reg_word(TX_DTX_IDX(i))); 	
		CHK_BUF();
	}

	for (i = 0; i < RX_QUEUE_NUM; i++) {
		index += sprintf(buf+index, "RX_BASE_PTR(%d)   (0x%08x) = 0x%08lx\n", i, RX_BASE_PTR(i), read_reg_word(RX_BASE_PTR(i))); 	
		CHK_BUF();
		index += sprintf(buf+index, "RX_MAX_CNT(%d)    (0x%08x) = 0x%08lx\n", i, RX_MAX_CNT(i), read_reg_word(RX_MAX_CNT(i))); 	
		CHK_BUF();
		index += sprintf(buf+index, "RX_CALC_IDX(%d)   (0x%08x) = 0x%08lx\n", i, RX_CALC_IDX(i), read_reg_word(RX_CALC_IDX(i))); 	
		CHK_BUF();
		index += sprintf(buf+index, "RX_DRX_IDX(%d)    (0x%08x) = 0x%08lx\n", i, RX_DRX_IDX(i), read_reg_word(RX_DRX_IDX(i))); 	
		CHK_BUF();
	}

	index += sprintf(buf+index, "PDMA_INFO        (0x%08x) = 0x%08lx\n", PDMA_INFO, read_reg_word(PDMA_INFO)); 	
	CHK_BUF();
	index += sprintf(buf+index, "PDMA_GLO_CFG     (0x%08x) = 0x%08lx\n", PDMA_GLO_CFG, read_reg_word(PDMA_GLO_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "PDMA_RST_IDX     (0x%08x) = 0x%08lx\n", PDMA_RST_IDX, read_reg_word(PDMA_RST_IDX)); 	
	CHK_BUF();
	index += sprintf(buf+index, "DLY_INT_CFG      (0x%08x) = 0x%08lx\n", DLY_INT_CFG, read_reg_word(DLY_INT_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "FREEQ_THRES      (0x%08x) = 0x%08lx\n", FREEQ_THRES, read_reg_word(FREEQ_THRES)); 	
	CHK_BUF();
	index += sprintf(buf+index, "INT_STATUS       (0x%08x) = 0x%08lx\n", INT_STATUS, read_reg_word(INT_STATUS)); 	
	CHK_BUF();
	index += sprintf(buf+index, "INT_MASK         (0x%08x) = 0x%08lx\n", INT_MASK, read_reg_word(INT_MASK)); 	
	CHK_BUF();
	index += sprintf(buf+index, "SCH_Q01_CFG      (0x%08x) = 0x%08lx\n", SCH_Q01_CFG, read_reg_word(SCH_Q01_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "SCH_Q23_CFG      (0x%08x) = 0x%08lx\n", SCH_Q23_CFG, read_reg_word(SCH_Q23_CFG)); 	
	CHK_BUF();
#ifdef TCSUPPORT_MT7510_FE
	// CDMA1 Reg
	index += sprintf(buf+index, "CDMA_CSG_CFG         (0x%08x) = 0x%08lx\n", CDMA_CSG_CFG, read_reg_word(CDMA_CSG_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "CDMA_PPP_GEN         (0x%08x) = 0x%08lx\n", CDMA_PPP_GEN, read_reg_word(CDMA_PPP_GEN));
	CHK_BUF();
	//GDMA1 Reg
	index += sprintf(buf+index, "GDMA1_FWD_CFG        (0x%08x) = 0x%08lx\n", GDMA1_FWD_CFG, read_reg_word(GDMA1_FWD_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_SHRP_CFG       (0x%08x) = 0x%08lx\n", GDMA1_SHRP_CFG, read_reg_word(GDMA1_SHRP_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_MAC_ADRL       (0x%08x) = 0x%08lx\n", GDMA1_MAC_ADRL, read_reg_word(GDMA1_MAC_ADRL));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_MAC_ADRH       (0x%08x) = 0x%08lx\n", GDMA1_MAC_ADRH, read_reg_word(GDMA1_MAC_ADRH));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_VLAN_GEN       (0x%08x) = 0x%08lx\n", GDMA1_VLAN_GEN, read_reg_word(GDMA1_VLAN_GEN));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_FQFC_CFG         (0x%08x) = 0x%08lx\n", PSE_FQFC_CFG, read_reg_word(PSE_FQFC_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_IQ_REV1          (0x%08x) = 0x%08lx\n", PSE_IQ_REV1, read_reg_word(PSE_IQ_REV1));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_IQ_REV2          (0x%08x) = 0x%08lx\n", PSE_IQ_REV2, read_reg_word(PSE_IQ_REV2));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_IQ_STA1          (0x%08x) = 0x%08lx\n", PSE_IQ_STA1, read_reg_word(PSE_IQ_STA1));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_IQ_STA2          (0x%08x) = 0x%08lx\n", PSE_IQ_STA2, read_reg_word(PSE_IQ_STA2));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_IQ PER PORT 0:0x%02lx 1:0x%02lx 2:0x%02lx 5:0x%02lx\n", (read_reg_word(PSE_IQ_STA1) & 0xff), ((read_reg_word(PSE_IQ_STA1)>>8) & 0xff), ((read_reg_word(PSE_IQ_STA1)>>16) & 0xff), ((read_reg_word(PSE_IQ_STA2)>>8) & 0xff));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_OQ_STA1          (0x%08x) = 0x%08lx\n", PSE_OQ_STA1, read_reg_word(PSE_OQ_STA1));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_OQ_STA2          (0x%08x) = 0x%08lx\n", PSE_OQ_STA2, read_reg_word(PSE_OQ_STA2));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_OQ PER PORT 0:0x%02lx 1:0x%02lx 2:0x%02lx 4:0x%02lx 5:0x%02lx 6:0x%02lx\n", (read_reg_word(PSE_OQ_STA1) & 0xff), ((read_reg_word(PSE_OQ_STA1)>>8) & 0xff), ((read_reg_word(PSE_OQ_STA1)>>16) & 0xff), (read_reg_word(PSE_OQ_STA2) & 0xff), ((read_reg_word(PSE_OQ_STA2)>>8) & 0xff), ((read_reg_word(PSE_OQ_STA2)>>16) & 0xff));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_DROP_COUNT_0     (0x%08x) = 0x%08lx\n", PSE_DROP_COUNT_0, read_reg_word(PSE_DROP_COUNT_0));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_DROP_COUNT_1     (0x%08x) = 0x%08lx\n", PSE_DROP_COUNT_1, read_reg_word(PSE_DROP_COUNT_1));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_DROP_COUNT_2     (0x%08x) = 0x%08lx\n", PSE_DROP_COUNT_2, read_reg_word(PSE_DROP_COUNT_2));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_DROP_COUNT_4     (0x%08x) = 0x%08lx\n", PSE_DROP_COUNT_4, read_reg_word(PSE_DROP_COUNT_4));
	CHK_BUF();
	index += sprintf(buf+index, "PSE_DROP_COUNT_5     (0x%08x) = 0x%08lx\n", PSE_DROP_COUNT_5, read_reg_word(PSE_DROP_COUNT_5));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_FWD_CFG        (0x%08x) = 0x%08lx\n", GDMA2_FWD_CFG, read_reg_word(GDMA2_FWD_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_SHRP_CFG       (0x%08x) = 0x%08lx\n", GDMA2_SHRP_CFG, read_reg_word(GDMA2_SHRP_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_MAC_ADRL       (0x%08x) = 0x%08lx\n", GDMA2_MAC_ADRL, read_reg_word(GDMA2_MAC_ADRL));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_MAC_ADRH       (0x%08x) = 0x%08lx\n", GDMA2_MAC_ADRH, read_reg_word(GDMA2_MAC_ADRH));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_VLAN_CHECK     (0x%08x) = 0x%08lx\n", GDMA2_VLAN_CHECK, read_reg_word(GDMA2_VLAN_CHECK));
	CHK_BUF();
	//GDMA1 count display
	index += sprintf(buf+index, "GDMA1_TX_BYTECNT_H   (0x%08x) = 0x%08lx\n", GDMA_TX_BYTECNT_H, read_reg_word(GDMA_TX_BYTECNT_H));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_TX_BYTECNT_L   (0x%08x) = 0x%08lx\n", GDMA_TX_BYTECNT_L, read_reg_word(GDMA_TX_BYTECNT_L));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_TX_PACKETCNT   (0x%08x) = 0x%08lx\n", GDMA_TX_PKTCNT, read_reg_word(GDMA_TX_PKTCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_TX_ABORTCNT    (0x%08x) = 0x%08lx\n", GDMA_TX_ABORTCNT, read_reg_word(GDMA_TX_ABORTCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_TX_COLCNT      (0x%08x) = 0x%08lx\n", GDMA_TX_COLCNT , read_reg_word(GDMA_TX_COLCNT ));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_RX_BYTECNT_H   (0x%08x) = 0x%08lx\n", GDMA_RX_BYTECNT_H, read_reg_word(GDMA_RX_BYTECNT_H));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_RX_BYTECNT_L   (0x%08x) = 0x%08lx\n", GDMA_RX_BYTECNT_L, read_reg_word(GDMA_RX_BYTECNT_L));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_RX_PacketCNT   (0x%08x) = 0x%08lx\n", GDMA_RX_PKTCNT, read_reg_word(GDMA_RX_PKTCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_RX_OVERFLOW_CNT(0x%08x) = 0x%08lx\n", GDMA_RX_OERCNT, read_reg_word(GDMA_RX_OERCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_RX_FCS_CNT     (0x%08x) = 0x%08lx\n", GDMA_RX_FCSCNT, read_reg_word(GDMA_RX_FCSCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_RX_RUNT_CNT    (0x%08x) = 0x%08lx\n", GDMA_RX_RUNTCNT, read_reg_word(GDMA_RX_RUNTCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_RX_LONG_CNT    (0x%08x) = 0x%08lx\n", GDMA_RX_LONGCNT, read_reg_word(GDMA_RX_LONGCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_RX_ITUC_CNT    (0x%08x) = 0x%08lx\n", GDMA_RX_ITUCCNT, read_reg_word(GDMA_RX_ITUCCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_RX_FC_CNT      (0x%08x) = 0x%08lx\n", GDMA_RX_FCCNT, read_reg_word(GDMA_RX_FCCNT));
	CHK_BUF();
	//GDMA2 count display
	index += sprintf(buf+index, "GDMA2_TX_GETCNT      (0x%08x) = 0x%08lx\n", GDMA2_TX_GETCNT, read_reg_word(GDMA2_TX_GETCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_TX_GOKCNT      (0x%08x) = 0x%08lx\n", GDMA2_TX_OKCNT, read_reg_word(GDMA2_TX_OKCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_TX_DROPCNT     (0x%08x) = 0x%08lx\n", GDMA2_TX_DROPCNT, read_reg_word(GDMA2_TX_DROPCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_RX_OKCNT       (0x%08x) = 0x%08lx\n", GDMA2_RX_OKCNT, read_reg_word(GDMA2_RX_OKCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_RX_OVDROPCNT   (0x%08x) = 0x%08lx\n", GDMA2_RX_OVDROPCNT, read_reg_word(GDMA2_RX_OVDROPCNT));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_RX_ERRDROPCNT  (0x%08x) = 0x%08lx\n", GDMA2_RX_ERRDROPCNT, read_reg_word(GDMA2_RX_ERRDROPCNT));
	CHK_BUF();
#else
	index += sprintf(buf+index, "GDMA1_FWD_CFG    (0x%08x) = 0x%08lx\n", GDMA1_FWD_CFG, read_reg_word(GDMA1_FWD_CFG));
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_SCH_CFG    (0x%08x) = 0x%08lx\n", GDMA1_SCH_CFG, read_reg_word(GDMA1_SCH_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_SHRP_CFG   (0x%08x) = 0x%08lx\n", GDMA1_SHRP_CFG, read_reg_word(GDMA1_SHRP_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_MAC_ADRL   (0x%08x) = 0x%08lx\n", GDMA1_MAC_ADRL, read_reg_word(GDMA1_MAC_ADRL)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_MAC_ADRH   (0x%08x) = 0x%08lx\n", GDMA1_MAC_ADRH, read_reg_word(GDMA1_MAC_ADRH)); 	
	CHK_BUF();
	index += sprintf(buf+index, "PSE_FQFC_CFG     (0x%08x) = 0x%08lx\n", PSE_FQFC_CFG, read_reg_word(PSE_FQFC_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "CDMA_FC_CFG      (0x%08x) = 0x%08lx\n", CDMA_FC_CFG, read_reg_word(CDMA_FC_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_FC_CFG     (0x%08x) = 0x%08lx\n", GDMA1_FC_CFG, read_reg_word(GDMA1_FC_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_FC_CFG     (0x%08x) = 0x%08lx\n", GDMA2_FC_CFG, read_reg_word(GDMA2_FC_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "CDMA_OQ_STA      (0x%08x) = 0x%08lx\n", CDMA_OQ_STA, read_reg_word(CDMA_OQ_STA)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_OQ_STA     (0x%08x) = 0x%08lx\n", GDMA1_OQ_STA, read_reg_word(GDMA1_OQ_STA)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_OQ_STA     (0x%08x) = 0x%08lx\n", GDMA2_OQ_STA, read_reg_word(GDMA2_OQ_STA)); 	
	CHK_BUF();
	index += sprintf(buf+index, "PSE_IQ_STA       (0x%08x) = 0x%08lx\n", PSE_IQ_STA, read_reg_word(PSE_IQ_STA)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_FWD_CFG    (0x%08x) = 0x%08lx\n", GDMA2_FWD_CFG, read_reg_word(GDMA2_FWD_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_SCH_CFG    (0x%08x) = 0x%08lx\n", GDMA2_SCH_CFG, read_reg_word(GDMA2_SCH_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_SHRP_CFG   (0x%08x) = 0x%08lx\n", GDMA2_SHRP_CFG, read_reg_word(GDMA2_SHRP_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_MAC_ADRL   (0x%08x) = 0x%08lx\n", GDMA2_MAC_ADRL, read_reg_word(GDMA2_MAC_ADRL)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_MAC_ADRH   (0x%08x) = 0x%08lx\n", GDMA2_MAC_ADRH, read_reg_word(GDMA2_MAC_ADRH)); 	
	CHK_BUF();
	index += sprintf(buf+index, "CDMA_CSG_CFG     (0x%08x) = 0x%08lx\n", CDMA_CSG_CFG, read_reg_word(CDMA_CSG_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "CDMA_SCH_CFG     (0x%08x) = 0x%08lx\n", CDMA_SCH_CFG, read_reg_word(CDMA_SCH_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_TX_GBCNT1   (0x%08x) = 0x%08lx\n", GDMA_TX_GBCNT1, read_reg_word(GDMA_TX_GBCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_TX_GPCNT1   (0x%08x) = 0x%08lx\n", GDMA_TX_GPCNT1, read_reg_word(GDMA_TX_GPCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_TX_SKIPCNT1 (0x%08x) = 0x%08lx\n", GDMA_TX_SKIPCNT1, read_reg_word(GDMA_TX_SKIPCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_TX_COLCNT1  (0x%08x) = 0x%08lx\n", GDMA_TX_COLCNT1, read_reg_word(GDMA_TX_COLCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_GBCNT1   (0x%08x) = 0x%08lx\n", GDMA_RX_GBCNT1, read_reg_word(GDMA_RX_GBCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_GPCNT1   (0x%08x) = 0x%08lx\n", GDMA_RX_GPCNT1, read_reg_word(GDMA_RX_GPCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_OERCNT1  (0x%08x) = 0x%08lx\n", GDMA_RX_OERCNT1, read_reg_word(GDMA_RX_OERCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_FERCNT1  (0x%08x) = 0x%08lx\n", GDMA_RX_FERCNT1, read_reg_word(GDMA_RX_FERCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_SERCNT1  (0x%08x) = 0x%08lx\n", GDMA_RX_SERCNT1, read_reg_word(GDMA_RX_SERCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_LERCNT1  (0x%08x) = 0x%08lx\n", GDMA_RX_LERCNT1, read_reg_word(GDMA_RX_LERCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_CERCNT1  (0x%08x) = 0x%08lx\n", GDMA_RX_CERCNT1, read_reg_word(GDMA_RX_CERCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_FCCNT1   (0x%08x) = 0x%08lx\n", GDMA_RX_FCCNT1, read_reg_word(GDMA_RX_FCCNT1)); 	
	CHK_BUF();

	index += sprintf(buf+index, "[PHY REG] PHYADDR=%ld\n", mac_p->enetPhyAddr);
	CHK_BUF();
	for (i = 0; i <= 10; i++) {
   		reg = miiStationRead(mac_p, i);
		index += sprintf(buf+index, "PHY reg%d=0x%08lx\n", i, reg);
		CHK_BUF();
	}

	for (i = 15; i <= 20; i++) {
   		reg = miiStationRead(mac_p, i);
		index += sprintf(buf+index, "PHY reg%d=0x%08lx\n", i, reg);
		CHK_BUF();
	}

   	reg = miiStationRead(mac_p, 30);
	index += sprintf(buf+index, "PHY reg%d=0x%08lx\n", 30, reg);
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

static int eth_rxring_dump_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int i;
	macRxDescr_t *pRxDescp;
  	macRxDescr_t pRxDescpTmpVal;
  	macRxDescr_t *pRxDescpTmp = &pRxDescpTmpVal;

	if (!macInitialized) {
		*eof = 1;
		return 0;
	}

	pRxDescp = (macRxDescr_t*) mac_p->rxDescrRingBaseVAddr;
	index += sprintf(buf+index, "rx descr ring=%08lx\n", (uint32) pRxDescp);
	CHK_BUF();

	for (i = 0 ; i< mac_p->rxRingSize ; i++, pRxDescp++) {
#if defined(FE_BYTE_SWAP) || defined(__LITTLE_ENDIAN)
		pRxDescpTmp = pRxDescp;
#else
		pRxDescpTmp->rxd_info1.word = le32_to_cpu(pRxDescp->rxd_info1.word);
		pRxDescpTmp->rxd_info2.word = le32_to_cpu(pRxDescp->rxd_info2.word);
		pRxDescpTmp->rxd_info3.word = le32_to_cpu(pRxDescp->rxd_info3.word);
		pRxDescpTmp->rxd_info4.word = le32_to_cpu(pRxDescp->rxd_info4.word);
#endif
		#ifdef MT7510_DMA_DSCP_CACHE
		dma_cache_inv((unsigned long)pRxDescpTmp, CACHE_LINE_SIZE);
		#endif

		index += sprintf(buf+index, "i= %d descr=%08lx\n", i, (uint32) pRxDescp);
		CHK_BUF();
		index += sprintf(buf+index, " rdes1=%08lx\n", pRxDescpTmp->rxd_info1.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes2=%08lx\n", pRxDescpTmp->rxd_info2.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes3=%08lx\n", pRxDescpTmp->rxd_info3.word);
		CHK_BUF();
		index += sprintf(buf+index, " rdes4=%08lx\n", pRxDescpTmp->rxd_info4.word);
		CHK_BUF();
		index += sprintf(buf+index, " skb  =%08lx\n", (uint32) mac_p->rxskbs[i]);
		CHK_BUF();
	}

	index += sprintf(buf+index, "rxCurrentDescp    =%ld\n", mac_p->rxCurrentDescp);
	CHK_BUF();
	index += sprintf(buf+index, "RX_CALC_IDX(0)    =%08lx\n", read_reg_word(RX_CALC_IDX(0)));
	CHK_BUF();
	index += sprintf(buf+index, "RX_DRX_IDX(0)     =%08lx\n", read_reg_word(RX_DRX_IDX(0)));
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

static int eth_txring_dump_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;
	int i, txq;
  	macTxDescr_t *pTxDescp;
  	macTxDescr_t pTxDescpTmpVal;
  	macTxDescr_t *pTxDescpTmp = &pTxDescpTmpVal;

	if (!macInitialized) {
		*eof = 1;
		return 0;
	}

	txq = *((int *) data);

	pTxDescp = (macTxDescr_t*) mac_p->txDescrRingBaseVAddr[txq];

	index += sprintf(buf+index, "tx descr ring%d=%08x\n", txq, (unsigned int) pTxDescp);
	CHK_BUF();
	
	for (i = 0 ; i < mac_p->txRingSize ; i++, pTxDescp++) {
#if defined(FE_BYTE_SWAP) || defined(__LITTLE_ENDIAN)
		pTxDescpTmp = pTxDescp;
#else
		pTxDescpTmp->txd_info1.word = le32_to_cpu(pTxDescp->txd_info1.word);
		pTxDescpTmp->txd_info2.word = le32_to_cpu(pTxDescp->txd_info2.word);
		pTxDescpTmp->txd_info3.word = le32_to_cpu(pTxDescp->txd_info3.word);
		pTxDescpTmp->txd_info4.word = le32_to_cpu(pTxDescp->txd_info4.word);
#endif

		#ifdef MT7510_DMA_DSCP_CACHE
		dma_cache_inv((unsigned long)pTxDescpTmp, CACHE_LINE_SIZE);
		#endif

		index += sprintf(buf+index, "i= %d descr=%08lx\n", i, (uint32) pTxDescp);
		CHK_BUF();
		index += sprintf(buf+index, " tdes1=%08lx\n", pTxDescpTmp->txd_info1.word);
		CHK_BUF();
		index += sprintf(buf+index, " tdes2=%08lx\n", pTxDescpTmp->txd_info2.word);
		CHK_BUF();
		index += sprintf(buf+index, " tdes3=%08lx\n", pTxDescpTmp->txd_info3.word);
		CHK_BUF();
		index += sprintf(buf+index, " tdes4=%08lx\n", pTxDescpTmp->txd_info4.word);
		CHK_BUF();
		index += sprintf(buf+index, " skb  =%08lx\n", (uint32) mac_p->txskbs[txq][i]);
		CHK_BUF();
	}

	index += sprintf(buf+index, "txCurrentDescp[%d]    =%ld\n", txq, mac_p->txCurrentDescp[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "txUnReleasedDescp[%d] =%ld\n", txq, mac_p->txUnReleasedDescp[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "txUnReleasedBufCnt[%d]=%ld\n", txq, mac_p->txUnReleasedBufCnt[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "TX_CTX_IDX(%d)        =%08lx\n", txq, read_reg_word(TX_CTX_IDX(txq)));
	CHK_BUF();
	index += sprintf(buf+index, "TX_DTX_IDX(%d)        =%08lx\n", txq, read_reg_word(TX_DTX_IDX(txq)));
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

static uint8 get_qos_weight(uint8 weight)
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

static int eth_qoswrr_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data){

	printk("%d %d %d %d %d\n", *qos_wrr_info, *(qos_wrr_info + 1), *(qos_wrr_info + 2), *(qos_wrr_info + 3), *(qos_wrr_info + 4));
	return 0;
}

static int eth_qoswrr_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data){
	int len;
	char get_buf[32];
	uint32 reg;
	int max_wrr_val = 0, i;
#if defined(TCSUPPORT_WAN_ETHER) || defined(TCSUPPORT_MT7510_FE)
	u8 port, queue;
	u32 value;
	u32 switch_reg; 
#endif
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
#if defined(TCSUPPORT_WAN_ETHER) || defined(TCSUPPORT_MT7510_FE)
		//SP mode
		for( queue = 0; queue < 4;queue++){		
		//Port n Queue x min.rate control enable		
			for( port = 0; port < 7;port++){
				switch_reg = 0x1000 + queue*0x8 + port * 0x100;
				value = switch_reg_read(GSW_BASE+switch_reg);
				value|= (1<<15);
				value&=(0xfffff080);
				switch_reg_write((GSW_BASE+switch_reg), value);
				//printf("reg :%X, val: %X\r\n",reg, value);
			}

		//enable MAX_SP_WFQ_Qx_Pn with SP mode		
			for( port = 0; port < 7;port++){
				switch_reg = 0x1000 + queue*0x8 + 0x4+ port * 0x100;
				value=switch_reg_read(GSW_BASE+switch_reg);
				value|= (1<<31);
				switch_reg_write((GSW_BASE+switch_reg), value);
				//printf("reg :%X, val: %X\r\n",reg, value);
			}
		}

#ifndef TCSUPPORT_MT7510_FE
		//tag-pri-Q  mapping
		//PEM4: tag7-> pri7-> Q3  tag6-> pri6-> Q2
		value = 0x38C03080;
		switch_reg = 0x54;
		switch_reg_write((GSW_BASE+switch_reg), value);
		//printf("reg :%X, val: %X\r\n",reg, value);	

		//PEM3: tag5-> pri5-> Q1  tag4-> pri4-> Q0
		value = 0x28402000;
		switch_reg = 0x50;
		switch_reg_write((GSW_BASE+switch_reg), value);
		//printf("reg :%X, val: %X\r\n",reg, value);
#endif		
#else
		/* Min BW = Max BW = unlimited */
		reg = read_reg_word(SCH_Q01_CFG);
		reg &= ~(MAX_WEIGHT1 | MIN_RATE_RATIO1 | MAX_WEIGHT0 | MIN_RATE_RATIO0);
		reg |= (MIN_RATIO0<<MIN_RATE_RATIO1_SHIFT) | (MIN_RATIO0<<MIN_RATE_RATIO0_SHIFT);
		reg |= (MAX_WEIGHT_2047<<MAX_WEIGHT1_SHIFT) | (MAX_WEIGHT_1023<<MAX_WEIGHT0_SHIFT);
		write_reg_word(SCH_Q01_CFG, reg);

		reg = read_reg_word(SCH_Q23_CFG);
		reg &= ~(MAX_WEIGHT3 | MIN_RATE_RATIO3 | MAX_WEIGHT2 | MIN_RATE_RATIO2);
		reg |= (MIN_RATIO0<<MIN_RATE_RATIO3_SHIFT) | (MIN_RATIO0<<MIN_RATE_RATIO2_SHIFT);
		reg |= (MAX_WEIGHT_8191<<MAX_WEIGHT3_SHIFT) | (MAX_WEIGHT_4095<<MAX_WEIGHT2_SHIFT);
		write_reg_word(SCH_Q23_CFG, reg);

		/* set GDMA2_SCH_CFG */
		reg = read_reg_word(GDMA2_SCH_CFG);
		reg &= ~GDM_SCH_MOD;
		reg |= GDM_SCH_MOD_SP<<GDM_SCH_MOD_SHIFT;
		write_reg_word(GDMA2_SCH_CFG, reg);
#endif
	}	
	else {  /*WRR*/
#if defined(TCSUPPORT_WAN_ETHER) || defined(TCSUPPORT_MT7510_FE)
		//WRR mode
		for (queue=0; queue<4; queue++){
			
			//Port n Queue x min.rate control enable		
			for (port=0; port<7; port++){
				switch_reg = 0x1000 + queue*0x8 + port * 0x100;
				value = switch_reg_read(GSW_BASE+switch_reg);
				value &= (0xfffff080);
				value |= (1<<15);
				switch_reg_write((GSW_BASE+switch_reg), value);
				//printf("reg :%X, val: %X\r\n",reg, value);	
			}

			//enable MAX_SP_WFQ_Qx_Pn with RR mode		
			for (port=0; port<7; port++){
				switch_reg = 0x1000 + queue*0x8 + 0x4+ port * 0x100;///*		
				value = switch_reg_read(GSW_BASE+switch_reg);
				value &= (~(1<<31));
				switch_reg_write((GSW_BASE+switch_reg), value);
				//printf("reg :%X, val: %X\r\n",reg, value);	
			}
		}

		//weight
		for( queue = 0; queue < 4;queue++){
			for( port = 0; port < 7;port++){
				switch_reg = 0x1000 + queue*0x8 + 0x4+ port * 0x100;///*
				value = switch_reg_read(GSW_BASE+switch_reg);
				value &= (~(0xf<<24));
				value|= (((*(qos_wrr_info+4-queue)-1)&0xf)<<24);
				//printf("reg :%X, val: %X\r\n",reg, value);	
				switch_reg_write((GSW_BASE+switch_reg), value);
			}
		}
#ifndef TCSUPPORT_MT7510_FE

		//tag-pri-Q  mapping
		//PEM4: tag7-> pri7-> Q3  tag6-> pri6-> Q2
		value = 0x38C03080;
		switch_reg = 0x54;
		switch_reg_write((GSW_BASE+switch_reg), value);
		//printf("reg :%X, val: %X\r\n",reg, value);	

		//PEM3: tag5-> pri5-> Q1  tag4-> pri4-> Q0
		value = 0x28402000;
		switch_reg = 0x50;
		switch_reg_write((GSW_BASE+switch_reg), value);
		//printf("reg :%X, val: %X\r\n",reg, value);
#endif
#else			

		/* Min BW = 0, Max BW = unlimited */
		reg = read_reg_word(SCH_Q01_CFG);
		reg &= ~(MAX_WEIGHT1 | MIN_RATE_RATIO1 | MAX_WEIGHT0 | MIN_RATE_RATIO0);
		reg |= (MIN_RATIO3<<MIN_RATE_RATIO1_SHIFT) | (MIN_RATIO3<<MIN_RATE_RATIO0_SHIFT);
		reg |= (get_qos_weight(qos_wrr_info[3] & 0x0f)<<MAX_WEIGHT1_SHIFT) | (get_qos_weight(qos_wrr_info[4] & 0x0f)<<MAX_WEIGHT0_SHIFT);
		write_reg_word(SCH_Q01_CFG, reg);

		reg = read_reg_word(SCH_Q23_CFG);
		reg &= ~(MAX_WEIGHT3 | MIN_RATE_RATIO3 | MAX_WEIGHT2 | MIN_RATE_RATIO2);
		reg |= (MIN_RATIO3<<MIN_RATE_RATIO3_SHIFT) | (MIN_RATIO3<<MIN_RATE_RATIO2_SHIFT);
		reg |= (get_qos_weight(qos_wrr_info[1] & 0x0f)<<MAX_WEIGHT3_SHIFT) | (get_qos_weight(qos_wrr_info[2] & 0x0f)<<MAX_WEIGHT2_SHIFT);
		write_reg_word(SCH_Q23_CFG, reg);

		/* set GDMA2_SCH_CFG */
		reg = read_reg_word(GDMA2_SCH_CFG);
		reg &= ~(GDM_SCH_MOD | GDM_WT_Q3 | GDM_WT_Q2 | GDM_WT_Q1 | GDM_WT_Q0);
		reg |= (GDM_SCH_MOD_WRR<<GDM_SCH_MOD_SHIFT) | 
					(GDM_WT(qos_wrr_info[1] & 0x0f)<<GDM_WT_Q3_SHIFT) | 
					(GDM_WT(qos_wrr_info[2] & 0x0f)<<GDM_WT_Q2_SHIFT) | 
					(GDM_WT(qos_wrr_info[3] & 0x0f)<<GDM_WT_Q1_SHIFT) | 
					(GDM_WT(qos_wrr_info[4] & 0x0f)<<GDM_WT_Q0_SHIFT);
		write_reg_word(GDMA2_SCH_CFG, reg);
#endif
	}
	return len;
}
#endif

#ifdef TCSUPPORT_QOS
static int eth_tcqos_read_proc(char *page, char **start, off_t off,
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

static int eth_tcqos_write_proc(struct file *file, const char *buffer,
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

#ifdef TCPHY_SUPPORT
#if defined(TCSUPPORT_AGEOUT_MAC)
static uint8 portstatus[4]={0};
/*_____________________________________________________________________________
**      function name: monitorLinkSt
**      descriptions:
**      	check whether lan port is down.if lan port change status from up to down,
**			clean the mac address entry with the specific port
**   
**      parameters:
**         None
**             
**      global:
**         None
**             
**      return:
**             None
**	     
**      call:
**   	      
**      revision:
**      1. Shelven.Lu 2011/02/08
**____________________________________________________________________________
*/

void 
monitorLinkSt(void){
	uint32 regValue = 0;
	int i = 0;
	uint32 value = 0; 

	if(swicVendor!=0){
		for (i=0; i<4; i++){
			regValue=tcMiiStationRead(i, MII_BMSR);
			/*If the ethernet Link is changed, system must be hook the bhalWakeupMonitorTask function.*/
			if (((regValue&BMSR_LSTATUS)>>2) != portstatus[i]) {
				if(portstatus[i]==1){
					value=0;/*value =0 for it will be the value of register GSW_BASE+0x80 */
					value |= (1<<i);
					write_reg_word (GSW_ATA1_REG,value);
					value=read_reg_word (GSW_ATC_REG);
					value |=0x8c02;//write enable and clean all mac address entry with specific port
					write_reg_word (GSW_ATC_REG,value);
				}	
				portstatus[i] = ((regValue&BMSR_LSTATUS)>>2);
			}
		}
	}
}/*end monitorLinkSt*/
#endif

/*_____________________________________________________________________________
**      function name: periodChk
**      descriptions:
**      	The periodic check  TC phy or switch register, and make sure
**      the ethernet can be work.
**   
**      parameters:
**         None
**             
**      global:
**         None
**             
**      return:
**             None
**	     
**      call:
**   	     getTcPhyFlag
**   	     esdSwPatch
**   	     updatingUpnpGid
**   	     tcPhyChkVal
**   	     tcPhyErrMonitor
** 
**      revision:
**      1. Here 2010/05/11
**____________________________________________________________________________
*/
void
periodChk(void){
	if (getTcPhyFlag()){
#if defined(TCSUPPORT_AGEOUT_MAC)
		uint32 value=0;
#endif
		tcPhySwPatch();
		#ifdef TCPHY_4PORT
		esdSwPatch();
		/*Workaroud to fix hardware igmpsnoop function actived then 
		the upnp multicast packet will be drop.(239.255.255.250)*/
		updatingUpnpGid();
		#endif
#if defined(TCPHY_SUPPORT)
		if (isRT63365){
			if ( (read_reg_word(0xbfb00064) & (0xffff)) == 0x0 ){
				duplex_sw_patch();
				polling_abnormal_irq();
			}	
#if defined(TCSUPPORT_AGEOUT_MAC)
			value=read_reg_word(GSW_VLAN_REG);
			if((value & (1<<30))!=0){//IVL ENABLE
				monitorLinkSt();
			}
#endif
		}	
#endif		
		#ifdef TCPHY_DEBUG
		tcPhyChkVal();
		tcPhyErrMonitor();
		#endif
	}
}/*end periodChk*/
#endif
/* Monitor the status of link, re-do link initialization if necessary. */
static void tc3262_gmac_link_status(unsigned long data)
{
	uint32 bmsr;
	uint32 bmcr;
	uint32 loopback = 0;
	uint32 advert, lpa;
	uint32 advert2, lpa2;
	uint32 enetMode;

	if (mac_p == NULL)
		return;
	
#ifdef TCPHY_SUPPORT
	periodChk();
#endif	

#ifdef  TC_CONSOLE_ENABLE
    tcconsole_chk();
#endif

#ifdef TC3262_GMAC_NAPI
//#ifdef LOOPBACK_SUPPORT
//	if (!LOOPBACK_MODE(macLoopback)) 
//#endif
	{
		int txq;

		for (txq = 0; txq < TX_QUEUE_NUM; txq++) 
   			macTxRingProc(mac_p, txq);
	}
#endif

	/* save the original enet mode */
	enetMode = mac_p->macPhyLinkProfile_p->enetMode;

	if (swicVendor) {
		mac_p->macPhyLinkProfile_p->enetMode =  LAN_ST_100MB | LAN_ST_FULL_DUPLEX | LAN_ST_LINK_UP;
		mac_p->macPhyLinkProfile_p->enetMode &= ~LAN_ST_100MB;
		mac_p->macPhyLinkProfile_p->enetMode |= LAN_ST_1000MB;
	} else {
		mac_p->macPhyLinkProfile_p->enetMode = 0;
		bmcr = miiStationRead(mac_p, MII_BMCR);
		loopback = bmcr & BMCR_LOOPBACK;

		/* ----- Check reset bit & power down bit ----- */
		if (bmcr & (BMCR_RESET | BMCR_PDOWN)) 
			return;
		#ifdef TCPHY_SUPPORT
		/*Updating by phy register*/
		if (getTcPhyFlag())
			bmsr = getTcPhyStatusReg(mac_p);
		else
		#endif	
		bmsr = miiStationRead(mac_p, MII_BMSR);

		if (bmcr & BMCR_ANENABLE) {
			if (bmsr & BMSR_ANEGCOMPLETE) {
				advert = miiStationRead(mac_p, MII_ADVERTISE);
				lpa = miiStationRead(mac_p, MII_LPA);

				advert2 = miiStationRead(mac_p, MII_CTRL1000);
				lpa2 = miiStationRead(mac_p, MII_STAT1000);

				if ((advert2 & (lpa2 >> 2)) & ADVERTISE_1000FULL)
					mac_p->macPhyLinkProfile_p->enetMode |= (LAN_ST_FULL_DUPLEX | LAN_ST_1000MB);
				else if ((advert2 & (lpa2 >> 2)) & ADVERTISE_1000HALF)
					mac_p->macPhyLinkProfile_p->enetMode |= (LAN_ST_1000MB);
				if ((advert & lpa) & ADVERTISE_100FULL) 
					mac_p->macPhyLinkProfile_p->enetMode |= (LAN_ST_FULL_DUPLEX | LAN_ST_100MB);
				else if ((advert & lpa) & ADVERTISE_100HALF) 
					mac_p->macPhyLinkProfile_p->enetMode |= (LAN_ST_100MB);
				else if ((advert & lpa) & ADVERTISE_10FULL) 
					mac_p->macPhyLinkProfile_p->enetMode |= (LAN_ST_FULL_DUPLEX);
				else if ((advert & lpa) & ADVERTISE_10HALF)
					mac_p->macPhyLinkProfile_p->enetMode |= 0;
			}
		} else {
			if ((bmcr & BMCR_SPEED1000) && (bmcr & BMCR_SPEED100) == 0)
				mac_p->macPhyLinkProfile_p->enetMode |= (LAN_ST_1000MB);
			else if (bmcr & BMCR_SPEED100) 
				mac_p->macPhyLinkProfile_p->enetMode |= (LAN_ST_100MB);

			if (bmcr & BMCR_FULLDPLX) 
				mac_p->macPhyLinkProfile_p->enetMode |= (LAN_ST_FULL_DUPLEX);
		}

		if (bmsr & BMSR_LSTATUS) 
			mac_p->macPhyLinkProfile_p->enetMode |= LAN_ST_LINK_UP;
		else
			mac_p->macPhyLinkProfile_p->enetMode &= ~LAN_ST_LINK_UP;
	}

	if (mac_p->macPhyLinkProfile_p->enetMode & LAN_ST_LINK_UP) {
		if (!loopback) {
			if (enetMode != mac_p->macPhyLinkProfile_p->enetMode) {
				/* According to enet phy link mode to adjust mac full duplex mode */
			}
		}
	} else {
	}

	/* Schedule for the next time */
	eth_timer.expires = jiffies + msecs_to_jiffies(250);
  	add_timer(&eth_timer);
}

static void tc3262_gmac_wan_link_status(unsigned long data)
{
    uint32 reg;
    static unsigned int linkstatus = 0xffffffff;
    static unsigned int duplex = 0;
    static unsigned int speed = 0;

    reg = switch_reg_read(GSW_PMSR(4));
    if(linkstatus != (reg & MAC_LINK_STS))
    {
        linkstatus = (reg & MAC_LINK_STS);
        if (!(reg & MAC_LINK_STS))
        {
#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
			tbs_led_trigger_set(led_lan_5, led_off_trig);
#endif
            printk("Port 4 Link Down!\n");
        }
        else
        {
            speed = (reg & MAC_SPD_STS) >> MAC_SPD_STS_SHIFT;
            if (speed == 3)
                speed = 10000;
            else if (speed == PN_SPEED_1000M)
                speed = 1000;
            else if (speed == PN_SPEED_100M)
                speed = 100;
            else 
                speed = 10;

            if (reg & MAC_DPX_STS)
                duplex = 1;
            else 
                duplex = 0;

#if defined(CONFIG_NEW_LED) || defined(CONFIG_TBSMOD_LED)
			tbs_led_trigger_set(led_lan_5, led_on_trig);
			tbs_led_trigger_set(led_lan_5, led_blinking_trig);
#endif
            printk("Port 4 Link UP at %d Mbps %s Duplex!\n", speed, duplex ? "Full" : "Half");
        }
    }

    /* Schedule for the next time */
    eth_wan_timer.expires = jiffies + msecs_to_jiffies(250);
    add_timer(&eth_wan_timer);
}

extern void mtEMiiRegWrite(uint32 port_num, uint32 dev_num, uint32 reg_num, uint32 reg_data);

static int mt7530_switch_init()
{
	int i = 0;
	
	if(use_ext_switch == 0)
	{
		/* get tpid from map lan port */
		for (i = 0; i < RT63365_SWIC_PORTNUM-1; i++){
			if ((lan_port_map[i] >= 0) && (lan_port_map[i] < RT63365_SWIC_PORTNUM-1))
				lan_port_tpid[i] = read_reg_word(GSW_BASE+ GSW_PVC(lan_port_map[i]))>>16;
			else
				lan_port_tpid[i] = DEFAULT_TPID;
		}
		if ((wan_port_id >= 0) && (wan_port_id < RT63365_SWIC_PORTNUM-1))
			ether_wan_tpid = read_reg_word(GSW_BASE+ GSW_PVC(wan_port_id))>>16;
		else 
			ether_wan_tpid = DEFAULT_TPID;
		cpu_port_tpid = read_reg_word(GSW_BASE+ GSW_PVC(6))>>16;

		/* set vlan table: add all vids to table */
		for(i = 0; i < 4096; i++)
		{
			write_reg_word(GSW_BASE + 0x0094,0x607f0001);
			write_reg_word(GSW_BASE + 0x0090,(0x80001000 + i));
		}
		/* global mac control: set Rx Jumbo to 9K Bytes */
		write_reg_word(GSW_BASE + 0x30e0,0x3f27);

		//set TRGMII
		write_reg_word(GSW_BASE + 0x7808,0);
		write_reg_word(GSW_BASE + 0x7804,0x01017e8f);
		write_reg_word(GSW_BASE + 0x7808,1);

		mtEMiiRegWrite(12,0x1f,0x103,0x0020);
		//mtEMiiRegWrite(12,0x1f,0x104,0x0608);
		mtEMiiRegWrite(12,0x1f,0x104,0x2608);

		mtEMiiRegWrite(12,0x1f,0x404,0x1900); // 200MHz
		mtEMiiRegWrite(12,0x1f,0x409,0x0057);
		mtEMiiRegWrite(12,0x1f,0x40a,0x0057);

		mtEMiiRegWrite(12,0x1f,0x403,0x1800);
		mtEMiiRegWrite(12,0x1f,0x403,0x1c00);
		mtEMiiRegWrite(12,0x1f,0x401,0xc020);
		mtEMiiRegWrite(12,0x1f,0x406,0xa030);
		mtEMiiRegWrite(12,0x1f,0x410,0x0003);

		for(i = 0; i < 5; i++)
		{
			write_reg_word(GSW_BASE + 0x7a10 + (i * 8),1);
		}
			write_reg_word(GSW_BASE + 0x1fe0,0xa0087864); //open flow control for throughput test

		/* port 6: enable flowcontrol and force to 1G full */
		write_reg_word(GSW_PMCR(6), 0x5e33b);
		//write_reg_word(GSW_BASE + 0x3600,0x5e30b);
		write_reg_word(GSW_BASE + 0x7830,1);
	}
	else
	{
		/* get tpid from map lan port */
		for (i = 0; i < RT63365_SWIC_PORTNUM-1; i++){
			if ((lan_port_map[i] >= 0) && (lan_port_map[i] < RT63365_SWIC_PORTNUM-1))
				lan_port_tpid[i] = gswPbusRead( GSW_PVC(lan_port_map[i]))>>16;
			else
				lan_port_tpid[i] = DEFAULT_TPID; 
		}
		if ((wan_port_id >= 0) && (wan_port_id < RT63365_SWIC_PORTNUM-1))
			ether_wan_tpid = gswPbusRead( GSW_PVC(wan_port_id))>>16;
		else 
			ether_wan_tpid = DEFAULT_TPID; 
		cpu_port_tpid = gswPbusRead( GSW_PVC(6))>>16;
		
		//set VLAN table
		for(i = 0; i < 4096; i++)
		{
			gswPbusWrite(0x0094,0x607f0001);
			gswPbusWrite(0x0090,(0x80001000 + i));
		}
		//large pkt support
		gswPbusWrite(0x30e0,0x3f27);
		write_reg_word(GSW_BASE + 0x30e0,0x3f27);
		
		//set TRGMII
		write_reg_word(GSW_BASE + 0x7808,0);
		write_reg_word(GSW_BASE + 0x7804,0x01017e8f);
		write_reg_word(GSW_BASE + 0x7808,1);
		gswPbusWrite(0x7808,0);
		gswPbusWrite(0x7804,0x01017e8f);
		gswPbusWrite(0x7808,1);

		//PLL reset for E2
		mtEMiiRegWrite(0,0x1f,0x103,0x0020);
		mtEMiiRegWrite(0,0x1f,0x104,0x0608);
		mtEMiiRegWrite(0,0x1f,0x104,0x2608);
		mtEMiiRegWrite(12,0x1f,0x104,0x2608);

		//PLL modify to 362.5MHz
		mtEMiiRegWrite(0,0x1f,0x404,0x1900); // for 200MHz
		mtEMiiRegWrite(0,0x1f,0x409,0x0057);
		mtEMiiRegWrite(0,0x1f,0x40a,0x0057);

		//for internal
		mtEMiiRegWrite(12,0x1f,0x404,0x1900); // for 200MHz
		mtEMiiRegWrite(12,0x1f,0x409,0x0057);
		mtEMiiRegWrite(12,0x1f,0x40a,0x0057);

		//PLL bias en
		mtEMiiRegWrite(0,0x1f,0x403,0x1800);
		//Bias LPF en
		mtEMiiRegWrite(0,0x1f,0x403,0x1c00);
		//sys PLL en
		mtEMiiRegWrite(0,0x1f,0x401,0xc020);
		//LCDDDS PWDB
		mtEMiiRegWrite(0,0x1f,0x406,0xa030);
		//turn on gsw_2x_clk
		mtEMiiRegWrite(0,0x1f,0x410,0x0003);

		//for internal
		mtEMiiRegWrite(12,0x1f,0x403,0x1800);
		mtEMiiRegWrite(12,0x1f,0x403,0x1c00);
		mtEMiiRegWrite(12,0x1f,0x401,0xc020);
		mtEMiiRegWrite(12,0x1f,0x406,0xa030);
		mtEMiiRegWrite(12,0x1f,0x410,0x0003);

		//setup RX delay
		for(i = 0; i < 5; i++)
		{
			write_reg_word(GSW_BASE + 0x7a10 + (i * 8),1);
			gswPbusWrite(0x7a10 + (i * 8),1);
		}

		//close flow control
			write_reg_word(GSW_BASE + 0x1fe0,0xa0087864);  //open flow control for throughput test
		gswPbusWrite(0x1fe0,0x20087864);

		//set port to giga mode
		gswPbusWrite(0x3600,0x5e30b);
		//enable TRGMII
		gswPbusWrite(0x7830,1);
		//set port to giga mode
		write_reg_word(GSW_BASE + 0x3500,0x5e30b);
		write_reg_word(GSW_BASE + 0x3600,0x5e33b); //open flow control for throughtput test
		//enable TRGMII
		write_reg_word(GSW_BASE + 0x7830,1);
	}
	return 0;
}

/* Starting up the ethernet device */
static int tc3262_gmac_open(struct net_device *dev)
{
	int err;
	int i = 0;
	uint32 value = 0;

  	printk(KERN_INFO "%s: starting interface.\n", dev->name);

	err = request_irq(dev->irq, tc3262_gmac_isr, 0, dev->name, dev);
	if (err)
		return err;

  	macInit();

	/* MT7530 for 7520 or 7525 init */	
	if(isMT7520 || isMT7520G || isMT7525 || isMT7525G){
		mt7530_switch_init();
	}

	/* MII setup */
	mac_p->mii_if.phy_id = mac_p->enetPhyAddr;
	mac_p->mii_if.full_duplex = 1;
	mac_p->mii_if.phy_id_mask = 0x1f;
	mac_p->mii_if.reg_num_mask = 0x1f;
	mac_p->mii_if.dev = dev;
	mac_p->mii_if.mdio_read = mdio_read;
	mac_p->mii_if.mdio_write = mdio_write;
	mac_p->mii_if.supports_gmii = mii_check_gmii_support(&mac_p->mii_if);

  	/* Schedule timer for monitoring link status */
  	init_timer(&eth_timer);
	eth_timer.expires = jiffies + msecs_to_jiffies(250);
  	eth_timer.function = tc3262_gmac_link_status;
  	eth_timer.data = (unsigned long)dev;
  	add_timer(&eth_timer);

  	/* Schedule timer for monitoring link status */
  	init_timer(&eth_poll_timer);
  	eth_poll_timer.expires = jiffies + msecs_to_jiffies(500);
  	eth_poll_timer.function = tc3262_gmac_poll_func;
  	eth_poll_timer.data = (unsigned long)dev;
  	add_timer(&eth_poll_timer);

#if KERNEL_2_6_36
	napi_enable(&mac_p->napi);
#endif
  	netif_start_queue(dev);

  	return 0;
}

/* Stopping the ethernet device */
static int tc3262_gmac_close(struct net_device *dev)
{
	printk(KERN_INFO "tc3262_gmac_close\n");

  	netif_stop_queue(dev);

#if KERNEL_2_6_36
	napi_disable(&mac_p->napi);
#endif
	free_irq(dev->irq, dev);

  	/* Kill timer */
  	del_timer_sync(&eth_timer);
  	del_timer_sync(&eth_poll_timer);

  	return 0;
}

/* Setup multicast list */
static void tc3262_gmac_set_multicast_list(struct net_device *dev)
{
	return; /* Do nothing */
}
#ifdef TCSUPPORT_MAX_PACKET_2000
int my_eth_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu < 68 || new_mtu > 2000)
		return -EINVAL;
	dev->mtu = new_mtu;
	return 0;
}
#endif

/* Setting customized mac address */
static int tc3262_gmac_set_macaddr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	/* Check if given address is valid ethernet MAC address */
  	if (!is_valid_ether_addr(addr->sa_data))
    	return(-EIO);

	/* Save the customize mac address */
  	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
  	memcpy(def_mac_addr, addr->sa_data, dev->addr_len);

	macGetMacAddr(mac_p);
  	macSetMacReg(mac_p);

	return 0;
}

/* Get the stats information */
static struct net_device_stats *tc3262_gmac_stats(struct net_device *dev)
{
	struct net_device_stats *stats;

	stats = &mac_p->stats;

#ifdef TCSUPPORT_MT7510_FE //use switch to save count to webpage
	stats->rx_packets = calcnt(GSW_RX_UNIC(0)) + calcnt(GSW_RX_MULC(0));
	stats->tx_packets = calcnt(GSW_TX_UNIC(0)) + read_reg_word(GSW_RX_MULC(6));
	stats->rx_bytes = calcnt(GSW_RX_OCL(0));
	stats->tx_bytes = read_reg_word(GSW_TX_OCL(wan_port_id));
	stats->rx_dropped = calcnt(GSW_RX_DROC(0));
	stats->tx_dropped = calcnt(GSW_TX_DROC(0));
	stats->multicast = calcnt(GSW_RX_MULC(0));
	stats->rx_errors = calcnt(GSW_RX_ALIGE(0)) + calcnt(GSW_RX_CRC(0)) + calcnt(GSW_RX_RUNT(0)) 
					+ calcnt(GSW_RX_FRGE(0)) + calcnt(GSW_RX_LONG(0));
	stats->tx_errors = calcnt(GSW_TX_COLC(0));
	stats->collisions = calcnt(GSW_TX_COLC(0));
#else
	stats->rx_packets = mac_p->macStat.MIB_II.inUnicastPkts +
		mac_p->macStat.MIB_II.inMulticastPkts;
	stats->tx_packets = mac_p->macStat.MIB_II.outUnicastPkts +
		mac_p->macStat.MIB_II.outMulticastPkts;
	stats->rx_bytes = mac_p->macStat.MIB_II.inOctets;
	stats->tx_bytes = mac_p->macStat.MIB_II.outOctets;
	stats->rx_dropped = mac_p->macStat.MIB_II.inDiscards;
	stats->tx_dropped = mac_p->macStat.MIB_II.outDiscards;
	stats->multicast = mac_p->macStat.MIB_II.inMulticastPkts;
	stats->rx_errors = mac_p->macStat.MIB_II.inErrors;
	stats->tx_errors = mac_p->macStat.MIB_II.outErrors;
	stats->collisions = mac_p->macStat.inSilicon.txExCollisionCnt + 
		mac_p->macStat.inSilicon.txCollisionCnt +
		mac_p->macStat.inSilicon.rxCollisionErr;
#endif
	return stats;
}

/* Handling ioctl call */
static int tc3262_gmac_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int rc = 0;
	gsw_reg reg;
	struct mii_ioctl_data mii;

	if (macInitialized) {
		switch (cmd) {
			case RAETH_REG_READ:
				copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
				reg.val = read_reg_word(reg.off);
				copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
				break;

			case RAETH_REG_WRITE:
				copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
				write_reg_word(reg.off, reg.val);
				break;

			case RAETH_GSW_REG_READ:
				copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
				reg.val = read_reg_word(GSW_BASE + reg.off);
				copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
				break;
			case RAETH_GSW_REG_WRITE:
				copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
				write_reg_word(GSW_BASE + reg.off, reg.val);
				break;

			case RAETH_GSWEXT_REG_READ:  //MTK120625 ///YM
				copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
				reg.val = gswPbusRead(reg.off);
				copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
				break;

			case RAETH_GSWEXT_REG_WRITE:  //MTK120625 ///YM
				copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
				gswPbusWrite(reg.off, reg.val);
				break;

			case RAETH_GSW_PHY_READ:
				copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
				mii.val_out = tcMiiStationRead(mii.phy_id, mii.reg_num);
				copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
				break;

			case RAETH_GSW_PHY_WRITE:
				copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
				tcMiiStationWrite(mii.phy_id, mii.reg_num, mii.val_in);
				break;

			case RAETH_GSWEXT_PHY_READ:  //MTK120625 ///YM
				copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
				mii.val_out = gswPmiRead(mii.phy_id, mii.reg_num);
				copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
				break;

			case RAETH_GSWEXT_PHY_WRITE:  //MTK120625 ///YM
				copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
				gswPmiWrite(mii.phy_id, mii.reg_num, mii.val_in);
				break;

		#ifdef TCSUPPORT_MT7530_SWITCH_API
			case RAETH_GSW_CTLAPI:
//				printk("RAETH_GSW_CTLAPI\n");
//				if (isRT62806 || isMT7530 || isMT7530ext)
//				if (isMT7530)
				if ((swicVendor == SWIC_MT7530) || (swicVendor == SWIC_RT62806))	// MT7530 switch
				{
					macMT7530gswAPIDispatch(ifr);
				}
				break;
		#endif

			default:
				copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
				rc = generic_mii_ioctl(&mac_p->mii_if, &mii, cmd, NULL);
//				rc = generic_mii_ioctl(&mac_p->mii_if, if_mii(ifr), cmd, NULL);
				copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
				break;
		}
	} else {
		rc = -EOPNOTSUPP;
	}

	return rc;
}

static irqreturn_t tc3262_gmac_isr(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;
	uint32 reg;


#if KERNEL_2_6_36
	macAdapter_t *mac_p = NULL;
	mac_p = netdev_priv(dev);
#endif

	reg = read_reg_word(INT_STATUS);
	write_reg_word(INT_STATUS, reg);
#ifdef LOOPBACK_SUPPORT
	if (macLoopback & LOOPBACK_ISR_TEST){
		printk("ISR Status %x\n",reg);
	}

	if (reg & TX_DONE_INT0){
		macTxRingProc(mac_p, 0);
	}
	if (reg & TX_DONE_INT1){
		macTxRingProc(mac_p, 1);
	}
	if (reg & TX_DONE_INT2){
		macTxRingProc(mac_p, 2);
	}
	if (reg & TX_DONE_INT3){
		macTxRingProc(mac_p, 3);
	}
#endif
	//write_reg_word(INT_STATUS, reg & read_reg_word(INT_MASK));

	// ----------Packet Received----------------------
	if (reg & (RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0)) {
#if 0
    	ledTurnOn(LED_ETHER_ACT_STATUS);
		if (mac_p->macPhyLinkProfile_p->enetMode & LAN_ST_100MB)
			ledTurnOn(LED_ETHER_100M_ACT_STATUS);
		else
			ledTurnOn(LED_ETHER_10M_ACT_STATUS);
#endif
#ifdef TC3262_GMAC_NAPI
		spin_lock(&gimr_lock);

#if KERNEL_2_6_36
		if(mac_p==NULL)
			return IRQ_HANDLED;
		if (napi_schedule_prep(&mac_p->napi)) {
		write_reg_word(INT_MASK, read_reg_word(INT_MASK) & 
					~(RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0));	
			__napi_schedule(&mac_p->napi);
		}
#else
		if (netif_rx_schedule_prep(dev)) {
			write_reg_word(INT_MASK, read_reg_word(INT_MASK) & 
					~(RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0));
			__netif_rx_schedule(dev);
		}

#endif
		spin_unlock(&gimr_lock);
#else
    	macRxRingProc(dev, dev->weight);
#endif
	} 

	if (reg & (RX_COHERENT | TX_COHERENT)) {
		printk("%s err mac_isr INT_STATUS=%08lx\n", dev->name, reg);
	}			

	return IRQ_HANDLED;
}

#ifdef TC3262_GMAC_NAPI
#if KERNEL_2_6_36
static int tc3262_gmac_poll(struct napi_struct *napi, int budget)
{
	macAdapter_t *mac_p = container_of(napi, macAdapter_t, napi);
	struct net_device *dev = mac_p->dev;
	int received = 0;
	unsigned long flags=0;

	received = macRxRingProc(dev, budget); 

	if (received < budget) {

		spin_lock_irqsave(&gimr_lock, flags);

		__napi_complete(napi);

		write_reg_word(INT_MASK, read_reg_word(INT_MASK) | 
			(RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0));
	
		spin_unlock_irqrestore(&gimr_lock, flags);
	}

	return received;
}
#else //KERNEL_2_6_36

static int tc3262_gmac_poll(struct net_device *dev, int *budget)
{
	int rx_work_limit = min(dev->quota, *budget);
	int received = 0;
	int n, done;
	unsigned long flags;

	n = macRxRingProc(dev, rx_work_limit);
	if (n) {
		received += n;
		rx_work_limit -= n;
		if (rx_work_limit <= 0) {
			done = 0;
			goto more_work;
		}
	}

	done = 1;

	spin_lock_irqsave(&gimr_lock, flags);

	__netif_rx_complete(dev);

	write_reg_word(INT_MASK, read_reg_word(INT_MASK) | 
		(RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0));

	spin_unlock_irqrestore(&gimr_lock, flags);

more_work:
	*budget -= received;
	dev->quota -= received;

	return done ? 0 : 1;
}
#endif

#endif //KERNEL_2_6_36
static int tc3262_gmac_start(struct net_device *dev)
{
	int i;

	/* frankliao modify 20101216 */
//	uint8 *flash_mac_addr = (uint8 *)0xbfc0ff48;
	char flash_mac_addr[6];

	for (i=0; i<6; i++) {
		flash_mac_addr[i] = i*2; //READ_FLASH_BYTE(flash_base + 0xff48 + i);
	}


	if( (flash_mac_addr[0] == 0) && (flash_mac_addr[1] == 0) && (flash_mac_addr[2] == 0) &&
	    (flash_mac_addr[3] == 0) && (flash_mac_addr[4] == 0) && (flash_mac_addr[5] == 0) )
		printk(KERN_INFO "The MAC address in flash is null!\n");	    
	else    
  		memcpy(def_mac_addr, flash_mac_addr, 6);  	

	mac_p = netdev_priv(dev);

	for (i = 0; i < 6; i++) {
		dev->dev_addr[i] = def_mac_addr[i];
	}

	spin_lock_init(&mac_p->lock);
	spin_lock_init(&phy_lock);

#if KERNEL_2_6_36
	mac_p->dev = dev;
#else
  	/* Hook up with handlers */
  	dev->get_stats 			= tc3262_gmac_stats;
  	dev->hard_start_xmit 	= tc3262_gmac_tx;
  	dev->open 				= tc3262_gmac_open;
  	dev->stop 				= tc3262_gmac_close;
  	dev->set_multicast_list = tc3262_gmac_set_multicast_list;
  	dev->do_ioctl 			= tc3262_gmac_ioctl;
  	dev->set_mac_address 	= tc3262_gmac_set_macaddr;
#ifdef TC3262_GMAC_NAPI
	dev->poll 				= tc3262_gmac_poll;
#endif 
	//dev->weight 			= MAC_RXDESCP_NO>>1;
	dev->weight 			= MAC_NAPI_WEIGHT;  

#if VLAN_TAG_USED
	dev->features |= NETIF_F_HW_VLAN_TX;
#endif

#ifdef RAETH_CHECKSUM_OFFLOAD
	dev->features |= NETIF_F_IP_CSUM;
#endif

#endif //KERNEL_2_6_36
	printk(KERN_INFO
	       "%s: FE MAC Ethernet address: %02X:%02X:%02X:%02X:%02X:%02X\n",
	       dev->name, 
	       dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
		   dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

	/*
  	dev->tx_queue_len = MAC_TXDESCP_NO; 
  	dev->flags &= ~IFF_MULTICAST;
  	dev->flags |= IFF_DEBUG;
	*/

	return 0;
}

#if KERNEL_2_6_36
static const struct net_device_ops gmac_netdev_ops = {
	.ndo_init				= tc3262_gmac_start,
	.ndo_open				= tc3262_gmac_open,
	.ndo_stop 				= tc3262_gmac_close,
	.ndo_start_xmit			= tc3262_gmac_tx,
//	.ndo_tx_timeout			= pcnet32_tx_timeout,
	.ndo_get_stats			= tc3262_gmac_stats,
	.ndo_set_multicast_list = tc3262_gmac_set_multicast_list,
	.ndo_do_ioctl			= tc3262_gmac_ioctl,
#ifdef TCSUPPORT_MAX_PACKET_2000
	.ndo_change_mtu			= my_eth_change_mtu,
#else
	.ndo_change_mtu			= eth_change_mtu,
#endif
	.ndo_set_mac_address 	= tc3262_gmac_set_macaddr,
	.ndo_validate_addr		= eth_validate_addr,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= tc3262_gmac_poll_controller,
#endif
};
#ifdef TCSUPPORT_WAN_ETHER
static const struct net_device_ops gmac_wan_netdev_ops = {
	.ndo_init				= tc3262_gmac_wan_start,
	.ndo_open				= tc3262_gmac_wan_open,
	.ndo_stop 				= tc3262_gmac_wan_close,
	.ndo_start_xmit			= tc3262_gmac_wan_tx,
	.ndo_get_stats			= tc3262_gmac_wan_stats,
	.ndo_set_multicast_list = tc3262_gmac_wan_set_multicast_list,
	.ndo_do_ioctl			= tc3262_gmac_wan_ioctl,
	.ndo_change_mtu			= eth_change_mtu,
	.ndo_set_mac_address 	= tc3262_gmac_wan_set_macaddr,
};
#endif
#endif //KERNEL_2_6_36
static int proc_txring[TX_QUEUE_NUM];
static char proc_txring_name[TX_QUEUE_NUM][32];

static int __init tc3262_gmac_init(void)
{
  	struct net_device *dev;
  	int err = 0;
	struct proc_dir_entry *eth_proc;
	struct proc_dir_entry *parent = init_net.proc_net;
	int txq;

	printk(KERN_INFO "%s", version);

	dev = alloc_etherdev(sizeof(macAdapter_t));
	if (!dev)
		return -ENOMEM;

	tc3262_gmac_dev = dev;
	
	dev->irq = MAC_INT;

	mac_p = netdev_priv(dev);

#if KERNEL_2_6_36
 	/* Hook up with handlers */
	dev->netdev_ops = &gmac_netdev_ops;

	mac_p->napi.weight = MAC_NAPI_WEIGHT;
	strcpy(dev->name, "ethsw");

	netif_napi_add(dev, &mac_p->napi, tc3262_gmac_poll, MAC_NAPI_WEIGHT);
#else
  	dev->init = tc3262_gmac_start;
#endif
	err = register_netdev(dev);
	if (err)
		return err;

#if defined(TCSUPPORT_WAN_ETHER)		
//register ether wan
	dev = alloc_netdev(sizeof(macAdapter_t), "eth4", ether_setup);
	if (!dev)
		return -ENOMEM;

	tc3262_gmac_wan_dev = dev;
	
	mac_wan_p = netdev_priv(dev);
#if KERNEL_2_6_36
 	/* Hook up with handlers */
	dev->netdev_ops = &gmac_wan_netdev_ops;

	mac_wan_p->napi.weight = MAC_NAPI_WEIGHT;
	
	netif_napi_add(dev, &mac_wan_p->napi, NULL/*dev->poll function*/, MAC_NAPI_WEIGHT);
#else
  	dev->init = tc3262_gmac_wan_start;

#endif	
  	
	err = register_netdev(dev);
	if (err)
		return err;
#endif
#ifdef TCSUPPORT_HEC_6906
	amc_verify_init();
#endif
	if(isMT7520 || isMT7520G || isMT7525 || isMT7525G)
	{
#if defined(JUDGE_SWITCH_SCENARIO_BY_751020_SUBMODEL)
		if (DefaultUseExtMT7530)
			use_ext_switch = 1;
		else
			use_ext_switch = 0;
#elif defined(DEFAULT_USE_EXT_SWIC)
			use_ext_switch = 1;
#else
			use_ext_switch = 0;
#endif
		eth_proc = create_proc_entry("tc3162/gsw_sp_tpid", 0, NULL);
		eth_proc->read_proc = special_tpid_read_proc;
		eth_proc->write_proc = special_tpid_write_proc;
		
		eth_proc = create_proc_entry("tc3162/gpon_bootflag", 0, NULL);
		eth_proc->read_proc = gpon_bootflag_read_proc;
	}

  	/* ethernet related stats */
	eth_proc = create_proc_entry("tc3162/eth_stats", 0, NULL);
	eth_proc->read_proc = eth_stats_read_proc;
	eth_proc->write_proc = eth_stats_write_proc;

#ifdef TCSUPPORT_WAN_ETHER
	eth_proc = create_proc_entry("tc3162/eth1_stats", 0, NULL);
	eth_proc->read_proc = eth1_stats_read_proc;
	eth_proc->write_proc = eth1_stats_write_proc;
#endif

	eth_proc = create_proc_entry("tc3162/gsw_stats", 0, NULL);
	eth_proc->read_proc = gsw_stats_read_proc;
	eth_proc->write_proc = gsw_stats_write_proc;
#if defined(MT7530_SUPPORT)
	create_proc_read_entry("tc3162/gsw_int_stats", 0, NULL, int_gsw_stats_read_proc, NULL);	
#endif

	/* wplin added 20120703 */
	#if (1)
	eth_proc = create_proc_entry("tc3162/gsw_mib0", 0, NULL);
	eth_proc->read_proc = gsw_mib0_read_proc;
	eth_proc->write_proc = gsw_stats_write_proc;
	eth_proc = create_proc_entry("tc3162/gsw_mib1", 0, NULL);
	eth_proc->read_proc = gsw_mib1_read_proc;
	eth_proc->write_proc = gsw_stats_write_proc;
	eth_proc = create_proc_entry("tc3162/gsw_mib2", 0, NULL);
	eth_proc->read_proc = gsw_mib2_read_proc;
	eth_proc->write_proc = gsw_stats_write_proc;
	eth_proc = create_proc_entry("tc3162/gsw_mib3", 0, NULL);
	eth_proc->read_proc = gsw_mib3_read_proc;
	eth_proc->write_proc = gsw_stats_write_proc;
	eth_proc = create_proc_entry("tc3162/gsw_mib4", 0, NULL);
	eth_proc->read_proc = gsw_mib4_read_proc;
	eth_proc->write_proc = gsw_stats_write_proc;
	eth_proc = create_proc_entry("tc3162/gsw_mib5", 0, NULL);
	eth_proc->read_proc = gsw_mib5_read_proc;
	eth_proc->write_proc = gsw_stats_write_proc;
	eth_proc = create_proc_entry("tc3162/gsw_mib6", 0, NULL);
	eth_proc->read_proc = gsw_mib6_read_proc;
	eth_proc->write_proc = gsw_stats_write_proc;
	#endif

//#if defined(GEN_1588_PKT_7530_VERIFY)
//MTK20120829_MT7530_1588pkt_generation, Start[
	eth_proc = create_proc_entry("tc3162/gen_1588_pkt", 0, NULL);
	eth_proc->write_proc = gen_1588_pkt_write_proc;
//MTK20120829_MT7530_1588pkt_generation, ]End
//#endif //GEN_1588_PKT_7530_VERIFY

  	//create_proc_read_entry("tc3162/gsw_link_st", 0, NULL, gsw_link_st_proc, NULL);
	create_proc_read_entry("link_stat", 0, parent, gsw_link_st_proc, NULL);

#ifdef LOOPBACK_SUPPORT
	eth_proc = create_proc_entry("tc3162/eth_loopback", 0, NULL);
	eth_proc->read_proc = eth_loopback_read_proc;
	eth_proc->write_proc = eth_loopback_write_proc;

	eth_proc = create_proc_entry("tc3162/eth_loopback_test", 0, NULL);
	eth_proc->read_proc = eth_loopback_test_read_proc;
	eth_proc->write_proc = eth_loopback_test_write_proc;
#endif

  	create_proc_read_entry("tc3162/eth_link_st", 0, NULL, eth_link_st_proc, NULL);
  	create_proc_read_entry("tc3162/eth_reg", 0, NULL, eth_reg_dump_proc, NULL);
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		proc_txring[txq] = txq;
		sprintf(proc_txring_name[txq], "tc3162/eth_txring%d", txq);
		create_proc_read_entry(proc_txring_name[txq], 0, NULL, eth_txring_dump_proc, &proc_txring[txq]);
	}
  	create_proc_read_entry("tc3162/eth_rxring", 0, NULL, eth_rxring_dump_proc, NULL);

#if defined(TCSUPPORT_WAN_ETHER)	
	create_proc_read_entry("tc3162/eth1_link_st", 0, NULL, eth_wan_link_st_proc, NULL);
#endif	

#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
	eth_proc = create_proc_entry("tc3162/eth1_qoswrr", 0, NULL);
	eth_proc->read_proc = eth_qoswrr_read_proc;
	eth_proc->write_proc = eth_qoswrr_write_proc;
#endif

#ifdef TCSUPPORT_QOS
	eth_proc = create_proc_entry("tc3162/eth1_tcqos_disc", 0, NULL);
	eth_proc->read_proc = eth_tcqos_read_proc;
	eth_proc->write_proc = eth_tcqos_write_proc;
#endif

#if defined(TCPHY_SUPPORT)
	/* GSW patch */
	eth_proc = create_proc_entry("tc3162/mac_esd_check", 0, NULL);
	eth_proc->read_proc = gsw_check_read_proc;
	eth_proc->write_proc = gsw_check_write_proc;
#endif			

	eth_proc = create_proc_entry("tc3162/port_reverse", 0, NULL);
	eth_proc->read_proc = port_reverse_read_proc;
	eth_proc->write_proc = port_reverse_write_proc;
	
	eth_proc = create_proc_entry("tc3162/stag_to_vtag", 0, NULL);
	eth_proc->read_proc = stag_to_vtag_read_proc;
	eth_proc->write_proc = stag_to_vtag_write_proc;
#if defined(TCSUPPORT_MT7510_FE) || defined(MT7530_SUPPORT)
	/*vport enable/disable control*/
	eth_proc = create_proc_entry("tc3162/vport_enable", 0, NULL);
	if(eth_proc){
		eth_proc->read_proc = vport_enable_read_proc;
		eth_proc->write_proc = vport_enable_write_proc;
	}	
#endif

	#ifdef TCPHY_SUPPORT
#if !defined(TCSUPPORT_CT)
  	create_proc_read_entry("tc3162/eth_port_status", 0, NULL, eth_port_stat_read_proc, NULL);
#endif
	//tcephydbgcmd();
	#endif
#ifdef TC_CONSOLE_ENABLE 
	create_tcconsole_proc();
	rcu_assign_pointer(send_uart_msg, uart_msg_to_tcconsole);
#endif
	
	eth_proc = create_proc_entry("tc3162/eth_portmap", 0, NULL);
	eth_proc->read_proc = ethernet_portmap_read_proc;

  	return 0;
}

static void __exit tc3262_gmac_exit(void)
{
	int txq;

	macReset();
	
	if(isMT7520 || isMT7520G || isMT7525 || isMT7525G){
		remove_proc_entry("tc3162/gsw_sp_tpid", 0);
		remove_proc_entry("tc3162/gpon_bootflag", 0);
	}
#if defined(TCSUPPORT_MT7510_FE) || defined(MT7530_SUPPORT)
	remove_proc_entry("tc3162/vport_enable", 0);
#endif

	remove_proc_entry("tc3162/eth_stats", 0);
#ifdef TCSUPPORT_WAN_ETHER
	remove_proc_entry("tc3162/eth1_stats", 0);
#endif
	remove_proc_entry("tc3162/gsw_stats", 0);
#if defined(MT7530_SUPPORT)
	remove_proc_entry("tc3162/gsw_int_stats", 0);
#endif
	remove_proc_entry("tc3162/gsw_link_st", 0);
#ifdef LOOPBACK_SUPPORT
	remove_proc_entry("tc3162/eth_loopback", 0);
	remove_proc_entry("tc3162/eth_loopback_test", 0);
#endif
   	remove_proc_entry("tc3162/eth_link_st", 0);
#if defined(TCSUPPORT_WAN_ETHER)
	remove_proc_entry("tc3162/eth1_link_st", 0);
#endif	
   	remove_proc_entry("tc3162/eth_reg", 0);
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {
		remove_proc_entry(proc_txring_name[txq], NULL);
	}
   	remove_proc_entry("tc3162/eth_rxring", 0);

#if defined(QOS_REMARKING) || defined(TCSUPPORT_HW_QOS)
	remove_proc_entry("tc3162/eth1_qoswrr", 0);
#endif

#ifdef TCSUPPORT_QOS
	remove_proc_entry("tc3162/eth1_tcqos_disc", 0);
#endif
	remove_proc_entry("tc3162/stag_to_vtag", 0);
	remove_proc_entry("tc3162/port_reverse", 0);

	macDrvDescripReset(mac_p);

#if defined(TCSUPPORT_WAN_ETHER)
	unregister_netdev(tc3262_gmac_wan_dev);
#endif	
	unregister_netdev(tc3262_gmac_dev);
#ifdef MT7510_DMA_DSCP_CACHE
	macClearDMADescrCacheReg();
#endif
	if (mac_p->macTxMemPool_p) {
#ifdef CONFIG_TC3162_DMEM
		if (is_sram_addr(mac_p->macTxMemPool_p))
			free_sram(mac_p->macTxMemPool_p, sizeof(macTxMemPool_t));
		else
#endif
#ifdef MT7510_DMA_DSCP_CACHE
		dma_free_noncoherent(NULL, sizeof(macTxMemPool_t)/2*3, tx_dscp_alloc, mac_p->macTxMemPool_phys_p);
#else
		dma_free_coherent(NULL, sizeof(macTxMemPool_t), mac_p->macTxMemPool_p, mac_p->macTxMemPool_phys_p);
#endif
	}
	if (mac_p->macRxMemPool_p) {
#ifdef CONFIG_TC3162_DMEM
		if (is_sram_addr(mac_p->macRxMemPool_p))
			free_sram(mac_p->macRxMemPool_p, sizeof(macRxMemPool_t));
		else
#endif
#ifdef MT7510_DMA_DSCP_CACHE
		dma_free_noncoherent(NULL, sizeof(macRxMemPool_t)/2*3, rx_dscp_alloc, mac_p->macRxMemPool_phys_p);
#else
		dma_free_coherent(NULL, sizeof(macRxMemPool_t), mac_p->macRxMemPool_p, mac_p->macRxMemPool_phys_p);
#endif
	}
	free_netdev(tc3262_gmac_dev);
	#ifdef TCPHY_SUPPORT
#if !defined(TCSUPPORT_CT)
	remove_proc_entry("tc3162/eth_port_status", 0);
#endif
	/*unregister ci command */
	//cmd_unregister("tcephydbg");
	#endif
#ifdef TC_CONSOLE_ENABLE 
	delete_tcconsole_proc();
	rcu_assign_pointer(send_uart_msg, NULL);
	#endif
}

#if defined(TCSUPPORT_WAN_ETHER)

static int get_wan_port_stat(char *buf){
	uint16 index = 0;
	uint32 reg;
	int port;
	int speed;

	if (!macInitialized) {
		return index;
	}

	port = wan_port_id;
	
	reg = switch_reg_read(GSW_PMSR(port));

	if (!(reg & MAC_LINK_STS)) {
		index += sprintf(buf+index, "Down\n");
		return index;
	}

	speed = (reg & MAC_SPD_STS) >> MAC_SPD_STS_SHIFT;
	if (speed == PN_SPEED_1000M)
		index += sprintf(buf+index, "1000M/");
	else if (speed == PN_SPEED_100M)
		index += sprintf(buf+index, "100M/");
	else 
		index += sprintf(buf+index, "10M/");

	if (reg & MAC_DPX_STS)
		index += sprintf(buf+index, "Full Duplex");
	else 
		index += sprintf(buf+index, "Half Duplex");

	if (reg & (TX_FC_STS | RX_FC_STS)) {
		index += sprintf(buf+index, " FC:");
		if (reg & TX_FC_STS) 
			index += sprintf(buf+index, " TX");
		if (reg & RX_FC_STS) 
			index += sprintf(buf+index, " RX");
	}

	if (reg & EEE100_STS) 
		index += sprintf(buf+index, " EEE100");
	if (reg & EEE1G_STS) 
		index += sprintf(buf+index, " EEE1G");

	index += sprintf(buf+index, "\n");
	
	return index;
}

static int eth_wan_link_st_proc(char *buf, char **start, off_t off, int count,
                 int *eof, void *data)
{
	int len = get_wan_port_stat(buf);
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

static int tc3262_gmac_wan_start(struct net_device *dev)
{
	int i;
	char flash_mac_addr[6];

	for (i=0; i<6; i++) {
		flash_mac_addr[i] = i*4;//READ_FLASH_BYTE(flash_base + 0xff48 + i);
	}

	if( (flash_mac_addr[0] == 0) && (flash_mac_addr[1] == 0) && (flash_mac_addr[2] == 0) &&
	    (flash_mac_addr[3] == 0) && (flash_mac_addr[4] == 0) && (flash_mac_addr[5] == 0) )
		printk(KERN_INFO "The MAC address in flash is null!\n");	    
	else    
  		memcpy(def_mac_addr, flash_mac_addr, 6);  	

	for (i = 0; i < 6; i++) {
		dev->dev_addr[i] = def_mac_addr[i];
	}

	
#if KERNEL_2_6_36
	mac_wan_p = netdev_priv(dev);
	mac_wan_p->dev = dev;
#else
  	/* Hook up with handlers */
  	dev->get_stats		= tc3262_gmac_wan_stats;
  	dev->hard_start_xmit	= tc3262_gmac_wan_tx;
  	dev->open		= tc3262_gmac_wan_open;
  	dev->stop		= tc3262_gmac_wan_close;
  	dev->set_multicast_list = tc3262_gmac_wan_set_multicast_list;
  	dev->do_ioctl		= tc3262_gmac_wan_ioctl;
  	dev->set_mac_address 	= tc3262_gmac_wan_set_macaddr;

#if VLAN_TAG_USED
	dev->features |= NETIF_F_HW_VLAN_TX;
#endif

#ifdef RAETH_CHECKSUM_OFFLOAD
	dev->features |= NETIF_F_IP_CSUM;
#endif

	
#endif//endof kernel_2_6_36
	return 0;
}

int tc3262_gmac_wan_tx(struct sk_buff *skb, struct net_device *dev)
{
//   	printk("tc3262_gmac_wan_tx\n");
#ifdef WAN2LAN
        if(masko_on_off){
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
	
	return tc3262_gmac_tx(skb, dev);
}

static struct net_device_stats *tc3262_gmac_wan_stats(struct net_device *dev)
{
	struct net_device_stats *stats;

	stats = &mac_wan_p->stats;

#ifdef TCSUPPORT_MT7510_FE //use switch to save count to webpage
	stats->rx_packets = read_reg_word(GSW_RX_UNIC(wan_port_id)) + read_reg_word(GSW_RX_MULC(wan_port_id));
	stats->tx_packets = read_reg_word(GSW_TX_UNIC(wan_port_id)) + read_reg_word(GSW_TX_MULC(wan_port_id));
	stats->rx_bytes = read_reg_word(GSW_RX_OCL(wan_port_id));
	stats->tx_bytes = read_reg_word(GSW_TX_OCL(wan_port_id));
	stats->rx_dropped = read_reg_word(GSW_RX_DROC(wan_port_id));
	stats->tx_dropped = read_reg_word(GSW_TX_DROC(wan_port_id));
	stats->multicast = read_reg_word(GSW_RX_MULC(wan_port_id));
	stats->rx_errors = read_reg_word(GSW_RX_ALIGE(wan_port_id)) + read_reg_word(GSW_RX_CRC(wan_port_id)) + read_reg_word(GSW_RX_RUNT(wan_port_id)) + read_reg_word(GSW_RX_FRGE(wan_port_id)) + read_reg_word(GSW_RX_LONG(wan_port_id));
	stats->tx_errors = read_reg_word(GSW_TX_COLC(wan_port_id));
	stats->collisions = read_reg_word(GSW_TX_COLC(wan_port_id));
#else
	stats->rx_packets = mac_wan_p->macStat.MIB_II.inUnicastPkts +
		mac_wan_p->macStat.MIB_II.inMulticastPkts;
	stats->tx_packets = mac_wan_p->macStat.MIB_II.outUnicastPkts +
		mac_wan_p->macStat.MIB_II.outMulticastPkts;
	stats->rx_bytes = mac_wan_p->macStat.MIB_II.inOctets;
	stats->tx_bytes = mac_wan_p->macStat.MIB_II.outOctets;
	stats->rx_dropped = mac_wan_p->macStat.MIB_II.inDiscards;
	stats->tx_dropped = mac_wan_p->macStat.MIB_II.outDiscards;
	stats->multicast = mac_wan_p->macStat.MIB_II.inMulticastPkts;
	stats->rx_errors = mac_wan_p->macStat.MIB_II.inErrors;
	stats->tx_errors = mac_wan_p->macStat.MIB_II.outErrors;
	stats->collisions = mac_wan_p->macStat.inSilicon.txExCollisionCnt + 
		mac_wan_p->macStat.inSilicon.txCollisionCnt +
		mac_wan_p->macStat.inSilicon.rxCollisionErr;
#endif
	return stats;
}
/* Starting up the ethernet device */
static int tc3262_gmac_wan_open(struct net_device *dev)
{
// 	printk("tc3262_gmac_wan_open\n");
	#if KERNEL_2_6_36
	napi_enable(&mac_wan_p->napi);
	#endif
	netif_start_queue(dev);

  	/* Schedule timer for monitoring link status */
  	init_timer(&eth_wan_timer);
	eth_wan_timer.expires = jiffies + msecs_to_jiffies(250);
  	eth_wan_timer.function = tc3262_gmac_wan_link_status;
  	eth_wan_timer.data = (unsigned long)dev;
  	add_timer(&eth_wan_timer);

  	return 0; 
}
static int tc3262_gmac_wan_close(struct net_device *dev)
{
// 	printk(KERN_INFO "tc3262_gmac_wan_close\n");
	#if KERNEL_2_6_36
	napi_disable(&mac_wan_p->napi);
	#endif

  	netif_stop_queue(dev);

  	/* Kill timer */
  	del_timer_sync(&eth_wan_timer);

  	return 0;
}
/* Setup multicast list */
static void tc3262_gmac_wan_set_multicast_list(struct net_device *dev)
{
	return; /* Do nothing */
}

/* Setting customized mac address */
static int tc3262_gmac_wan_set_macaddr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	/* Check if given address is valid ethernet MAC address */
  	if (!is_valid_ether_addr(addr->sa_data))
    		return(-EIO);

	/* Save the customize mac address */
  	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
#ifndef TCSUPPORT_MT7510_FE
	write_reg_word(GDMA2_MAC_ADRL, addr->sa_data[2]<<24 | addr->sa_data[3]<<16 | \
                               addr->sa_data[4]<<8  | addr->sa_data[5]<<0);
	write_reg_word(GDMA2_MAC_ADRH, addr->sa_data[0]<<8  | addr->sa_data[1]<<0);
#endif
	return 0; /* Do nothing */
}
/* Handling ioctl call */
static int tc3262_gmac_wan_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int rc;

	if (macInitialized) {
		rc = generic_mii_ioctl(&mac_p->mii_if, if_mii(ifr), cmd, NULL);
	} else {
		rc = -EOPNOTSUPP;
	}

	return rc;
}
#endif


/* Register startup/shutdown routines */
module_init(tc3262_gmac_init);
module_exit(tc3262_gmac_exit);
MODULE_LICENSE("GPL");
