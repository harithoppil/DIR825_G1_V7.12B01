
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
#include <asm/tc3162/TCIfSetQuery_os.h>
#include <gpio.h>
#include "gemac.h"
#ifdef CONFIG_RTL8365
#include "smi.h"
#include "rtl8367b_asicdrv_port.h"
#include "rtk_api_ext.h"
#else
#include "tcswitch.h"
#endif
#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif


#define RX_BUF_LEN 			(2048 - NET_SKB_PAD - 64 - (sizeof(struct skb_shared_info)))		
#define RX_MAX_PKT_LEN 		1536

/************************************************************************
*                          C O N S T A N T S
*************************************************************************/
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
#define LOOPBACK_PKT		(1<<11)
#define LOOPBACK_EXT		(1<<12)

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
*************************************************************************/

#define TC3262_GMAC_NAPI
#define TC3262_GMAC_SKB_RECYCLE

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

extern int (*ra_sw_nat_hook_rx)(struct sk_buff *skb);
extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);

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

/************************************************************************
*                       P R I V A T E   D A T A
*************************************************************************
*/

/* Device data */
struct net_device *switch_dev[SWITCH_PORT_MAX];
struct phy_link_state phystate[SWITCH_PORT_MAX];
/* phy lock */
static spinlock_t phy_lock;
struct macAdapter *mac_p = NULL;
static DEFINE_SPINLOCK(gimr_lock);
static int proc_txring[TX_QUEUE_NUM];
static char proc_txring_name[TX_QUEUE_NUM][32];

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************/

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
	if (i & 0x0f) {
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


#ifndef CONFIG_GPIO_OPERATION
void miiStationWrite(unsigned int PhyAddr, unsigned int PhyReg, unsigned int MiiData)
{
	unsigned int reg;
	int cnt;
	unsigned long flags;
	
	spin_lock_irqsave(&phy_lock, flags);
	for(cnt = 0; cnt < 1000; cnt++) {
		reg=REG32(GSW_CFG_PIAC);
		if(0 == (reg & PHY_ACS_ST)) {
			break;
		}
	}
	reg = PHY_ACS_ST | (MDIO_ST_START << MDIO_ST_SHIFT) | (MDIO_CMD_WRITE<<MDIO_CMD_SHIFT) | (PhyAddr << MDIO_PHY_ADDR_SHIFT) | (PhyReg << MDIO_REG_ADDR_SHIFT) | (MiiData & MDIO_RW_DATA);
	REGWRITE32(GSW_CFG_PIAC, reg);
	for( ; cnt < 2000; cnt++) {
		reg=REG32(GSW_CFG_PIAC);
		if(0 == (reg & PHY_ACS_ST)) {
			break;
		}
	}
	spin_unlock_irqrestore(&phy_lock, flags);
	if (2000 == cnt) { 
		printk("%s: Error, timeout!\n", __func__);
	}
}

unsigned short miiStationRead(unsigned int PhyAddr, unsigned int PhyReg)
{
	unsigned int reg;
	int cnt;
	unsigned short data = 0;
	unsigned long flags;

	spin_lock_irqsave(&phy_lock, flags);
	for(cnt = 0; cnt < 1000; cnt++) {
		reg=REG32(GSW_CFG_PIAC);
		if(0 == (reg & PHY_ACS_ST)) {
			break;
		}
	}
	reg = PHY_ACS_ST | (MDIO_ST_START << MDIO_ST_SHIFT) | (MDIO_CMD_READ<<MDIO_CMD_SHIFT) | (PhyAddr << MDIO_PHY_ADDR_SHIFT) | (PhyReg << MDIO_REG_ADDR_SHIFT);
	REGWRITE32(GSW_CFG_PIAC, reg);
	for( ; cnt < 2000; cnt++) {
		reg=REG32(GSW_CFG_PIAC);
		if(0 == (reg & PHY_ACS_ST)) {
			break;
		}
	}
	data = reg & MDIO_RW_DATA;
	spin_unlock_irqrestore(&phy_lock, flags);

	if (2000 == cnt) {
		printk("%s: Error, timeout!\n", __func__);
	}
	return data;
}

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

#else

void gpio_mdio_preamble(void)
{
	unsigned int i, j;

	gpio_config(GPIO_MDC, GPIO_OUT);
	gpio_config(GPIO_MDIO, GPIO_OUT);
	gpio_write(GPIO_MDC, GPIO_LEVEL_LOW);
	gpio_write(GPIO_MDIO, GPIO_LEVEL_HIGH);
	for(j = 0; j < MDC_CLK_DELAY; j++); /* CLK_DURATION */
	for(i = 0; i < 32; i++) {
		gpio_write(GPIO_MDC, GPIO_LEVEL_HIGH);
		for(j = 0; j < MDC_CLK_DELAY; j++); /* CLK_DURATION */
		gpio_write(GPIO_MDC, GPIO_LEVEL_LOW);
		for(j = 0; j < MDC_CLK_DELAY; j++); /* CLK_DURATION */
	}
}

void gpio_mdio_write(unsigned short data, unsigned int len)
{
	unsigned int i, j;
	
	for(i = len; i > 0; i--) {
		for(j = 0; j < MDC_CLK_DELAY; j++); /* CLK_DURATION */		
        if(data & (1<<(i-1))) {/* prepare data */
			gpio_write(GPIO_MDIO, GPIO_LEVEL_HIGH);
		} else {
			gpio_write(GPIO_MDIO, GPIO_LEVEL_LOW);
		}
		for(j = 0; j < MDC_CLK_DELAY; j++); /* CLK_DURATION */
		gpio_write(GPIO_MDC, GPIO_LEVEL_HIGH);/* clocking */
        for(j = 0; j < MDC_CLK_DELAY; j++); /* CLK_DURATION */
        gpio_write(GPIO_MDC, GPIO_LEVEL_LOW);
    }
}

unsigned int gpio_mdio_read(unsigned int len)
{
	gpio_level mdio_level;
	unsigned int i, j;
	unsigned int data = 0;

	gpio_config(GPIO_MDIO, GPIO_IN);/* change GPIO pin to Input only */
    for(i = len; i > 0; i--) {
        for(j = 0; j < MDC_CLK_DELAY; j++); /* CLK_DURATION */       
        gpio_write(GPIO_MDC, GPIO_LEVEL_HIGH);/* clocking */
        for(j = 0; j < MDC_CLK_DELAY; j++); /* CLK_DURATION */
        mdio_level = gpio_read(GPIO_MDIO);
        gpio_write(GPIO_MDC, GPIO_LEVEL_LOW);
        data |= (mdio_level << (i - 1));
    }
	
	return data;
}

void gpio_mdio_idle(void)
{
	unsigned int j;
	
	for(j = 0; j < MDC_CLK_DELAY; j++); /* CLK_DURATION */
	gpio_write(GPIO_MDIO, GPIO_LEVEL_HIGH);
	gpio_config(GPIO_MDIO, GPIO_IN);
}

void miiStationWrite(unsigned int PhyAddr, unsigned int RegAddr, unsigned int MiiData)
{
	unsigned short data;
	unsigned long flags;

	spin_lock_irqsave(&phy_lock, flags);
	gpio_mdio_preamble();
	data = MDIO_START | MDIO_WRITE | ((PhyAddr & 0x1F) << MDIO_PHYAD_SHIFT) | ((RegAddr & 0x1F) << MDIO_REGAD_SHIFT) | MDIO_WRITE_TA;
	gpio_mdio_write(data, 16);
	data = MiiData & 0xFFFF;
	gpio_mdio_write(data, 16);
	gpio_mdio_idle();
	spin_unlock_irqrestore(&phy_lock, flags);
}

unsigned short miiStationRead(unsigned int PhyAddr, unsigned int RegAddr)
{
	unsigned short data = 0;
	unsigned long flags;

	spin_lock_irqsave(&phy_lock, flags);
	gpio_mdio_preamble();
	data = MDIO_START | MDIO_READ | ((PhyAddr & 0x1F) << MDIO_PHYAD_SHIFT) | ((RegAddr & 0x1F) << MDIO_REGAD_SHIFT);
	gpio_mdio_write(data, 16);
	data = gpio_mdio_read(16);
	etdebug("%s: data=0x%04X!\n", __func__, data);
	gpio_mdio_idle();
	spin_unlock_irqrestore(&phy_lock, flags);
	
	return data;
}
#endif

static int mdio_read(struct net_device *dev, int phy_id, int reg_num)
{
	return miiStationRead(phy_id, reg_num);
}

static void mdio_write(struct net_device *dev, int phy_id, int reg_num, int val)
{
	miiStationWrite(phy_id, reg_num, val);
}

// Assign Tx Rx Descriptor Control Registers
void macSetDMADescrCtrlReg(struct macAdapter *mac_p)
{
	unsigned int txq;

	for (txq = 0; txq < TX_QUEUE_NUM; txq++) { 
  		REGWRITE32(TX_BASE_PTR(txq), K1_TO_PHY(mac_p->txDescrRingBaseVAddr[txq]));
  		REGWRITE32(TX_MAX_CNT(txq), mac_p->txRingSize);
		REGWRITE32(TX_CTX_IDX(txq), 0);
		REGWRITE32(PDMA_RST_IDX, RST_DTX_IDX(txq));
	}
  	REGWRITE32(RX_BASE_PTR(0), K1_TO_PHY(mac_p->rxDescrRingBaseVAddr));
  	REGWRITE32(RX_MAX_CNT(0), mac_p->rxRingSize);
	REGWRITE32(RX_CALC_IDX(0), mac_p->rxRingSize - 1);
	REGWRITE32(PDMA_RST_IDX, RST_DRX_IDX(0));
}

void macSetGSW(void)
{
	unsigned int reg;
	
	reg = (0xff<<MFC_BC_FFP_SHIFT) | (0xff<<MFC_UNM_FFP_SHIFT) | (0xff<<MFC_UNU_FFP_SHIFT) | MFC_CPU_EN	| (6<<MFC_CPU_PORT_SHIFT);
	REGWRITE32(GSW_MFC, reg);/* set cpu port as port 6 */
	reg = REG32(GSW_GMACCR) | (0xC << 2) | 0x2; /* Set MAC max receive length to 1552 bytes */
	REGWRITE32(GSW_GMACCR, reg);
	if (REG32(CR_AHB_HWCONF)&(1<<31)) {	/* check if FPGA */
		reg = REG32(GSW_CFG_PPSC);/* auto polling enable, 2 PHY ports, start PHY addr=6 and end PHY addr=7 */
		reg |= PHY_AP_EN | EMB_AN_EN;
		reg &= ~(PHY_END_ADDR | PHY_ST_ADDR);
		
		if ((miiStationRead(0, 2) == 0x243) && (miiStationRead(0, 3) == 0xc54)) {/* check 6 PHY ports board or 2 PHY port board */
			reg |= (5<<PHY_END_ADDR_SHIFT) | (0<<PHY_ST_ADDR_SHIFT);
		} else {
			reg |= (7<<PHY_END_ADDR_SHIFT) | (6<<PHY_ST_ADDR_SHIFT);
		}
		REGWRITE32(GSW_CFG_PPSC, reg);
	#ifdef CONFIG_RTL8365	
	} else {		
		reg = REG32(GSW_CFG_PPSC);/* auto polling enable, 2 PHY ports, start PHY addr=6 and end PHY addr=7 */
		reg &= ~(PHY_END_ADDR | PHY_ST_ADDR);
		reg |= (7<<PHY_END_ADDR_SHIFT) | (7<<PHY_ST_ADDR_SHIFT) | PHY_AP_EN;
		REGWRITE32(GSW_CFG_PPSC, reg);
	#endif	
	}	
	reg = (REG32(GSW_CFG_GPC) | RX_CLK_MODE);/* set port 5 giga port RX clock phase to degree 0 */
	#ifdef CONFIG_RTL8365
	reg |= (0x3F << 24); /* Disable Internal 5-port EPHY */
	#endif
	REGWRITE32(GSW_CFG_GPC, reg);
	#ifdef CONFIG_TCPHY
	if (isRT63365){/* enable switch abnormal irq, for error handle when abnormal irq occurs */
		if ( (REG32(0xbfb00064) & (0xffff)) == 0x0 ){
			enable_abnormal_irq();
		}
		reg = REG32(GSW_CKGCR);
		reg &= ~((1<<4) | (1<<5));
		REGWRITE32(GSW_CKGCR, reg);
			
	}
	#endif
	#ifdef CONFIG_RTL8365	
	reg = 0x00FF0000;
	REGWRITE32((GSW_BASE + 0x250C), reg);
	REGWRITE32((GSW_BASE + 0x260C), reg);
	reg = REG32(GSW_BASE + 0x250C);
	reg |= (1<<4);/* Disable Source MAC Address Learning */
	REGWRITE32((GSW_BASE + 0x250C), reg);
	reg = REG32(GSW_BASE + 0x260C);
	reg |= (1<<4);/* Disable Source MAC Address Learning */
	REGWRITE32((GSW_BASE + 0x260C), reg);	
	reg = REG32(GSW_BASE + 0x2510);
	reg |= 0xc0;  /* set port as Transparent port */
	reg &= ~(1<<5);/* set port special tag = 0 */
	REGWRITE32((GSW_BASE + 0x2510), reg);	
	reg = REG32(GSW_BASE + 0x2610);
	reg |= 0xc0;  /* set port as Transparent port */
	reg &= ~(1<<5);/* set port special tag = 0 */
	REGWRITE32((GSW_BASE + 0x2610), reg);
	#else
	/* Enable Special Tag function */
	reg = REG32(GSW_BASE + 0x2610);
	reg &= ~0xc0;/* set CPU port 6 as user port */
	reg |= (1<<5);/* set CPU port 6 special tag = 1 */
	REGWRITE32((GSW_BASE + 0x2610), reg);	
	reg = REG32(GDMA1_FWD_CFG);
	reg |= GDM_TCI_81XX;/* Enable GDM_TCI_81XX */
	REGWRITE32(GDMA1_FWD_CFG, reg);
	#endif
	reg = (IPG_CFG_SHORT<<IPG_CFG_PN_SHIFT) | MAC_MODE_PN | FORCE_MODE_PN | MAC_TX_EN_PN 
		| MAC_RX_EN_PN | BKOFF_EN_PN | BACKPR_EN_PN | ENABLE_RX_FC_PN | ENABLE_TX_FC_PN 
		| (PN_SPEED_1000M<<FORCE_SPD_PN_SHIFT) | FORCE_DPX_PN | FORCE_LNK_PN; 
	REGWRITE32(GSW_PMCR(6), reg);/* set port 6 as 1Gbps, FC on */
	#ifdef CONFIG_RTL8365
	/* set port 5 as 1Gbps, FC on RGMII, Force Link UP */
	reg = (IPG_CFG_SHORT<<IPG_CFG_PN_SHIFT) | EXT_PHY_PN | FORCE_MODE_PN | MAC_TX_EN_PN 
		| MAC_RX_EN_PN | BKOFF_EN_PN | BACKPR_EN_PN | ENABLE_RX_FC_PN | ENABLE_TX_FC_PN 
		| (PN_SPEED_1000M<<FORCE_SPD_PN_SHIFT) | FORCE_DPX_PN | FORCE_LNK_PN | RGMII_MODE_PN;
	REGWRITE32(GSW_PMCR(5), reg);
	#endif	
}

void macSetMACCR(void)
{
	unsigned int reg;

	reg = (12<<GDM_JMB_LEN_SHIFT) | GDM_JMB_EN | GDM_STRPCRC | (GDM_P_CPU<<GDM_UFRC_P_SHIFT) | 
		(GDM_P_CPU<<GDM_BFRC_P_SHIFT) | (GDM_P_CPU<<GDM_MFRC_P_SHIFT) | (GDM_P_CPU<<GDM_OFRC_P_SHIFT);
	#ifdef RAETH_CHECKSUM_OFFLOAD
	reg |= GDM_ICS_EN | GDM_TCS_EN | GDM_UCS_EN;
	#endif
	REGWRITE32(GDMA1_FWD_CFG, reg);
	REGWRITE32(GDMA2_FWD_CFG, reg);
	reg = (0x8100<<INS_VLAN_SHIFT);
	#ifdef RAETH_CHECKSUM_OFFLOAD
	reg |= ICS_GEN_EN | UCS_GEN_EN | TCS_GEN_EN;
	#endif
	REGWRITE32(CDMA_CSG_CFG, reg);
	/* set PDMA FC default value */
	REGWRITE32(PDMA_FC_CFG1, 0x0fffffff);
	REGWRITE32(PDMA_FC_CFG2, 0x0fffffff);
	/* check if FPGA */
	if(REG32(CR_AHB_HWCONF)&(1<<31)) {	/* set 1us clock for FPGA */
		reg = REG32(CR_CLK_CFG);
		reg &= ~(0x3f000000);
		reg |= (0x32<<24);
		REGWRITE32(CR_CLK_CFG, reg);
	}
}

void macSetMacReg(unsigned char *addr)
{
	REGWRITE32(GDMA1_MAC_ADRL, addr[2]<<24 | addr[3]<<16 | addr[4]<<8 | addr[5]);
	REGWRITE32(GDMA1_MAC_ADRH, addr[0]<<8  | addr[1]<<0);
}

void macDrvStart(unsigned char enable)
{
	unsigned int reg;

	reg = REG32(PDMA_GLO_CFG);
	if(enable) {
		reg &= ~PDMA_BT_SIZE;
		reg |= TX_WB_DDONE | (PDMA_BT_SIZE_32DW<<PDMA_BT_SIZE_SHIFT) | RX_DMA_EN | TX_DMA_EN;
		#ifdef __BIG_ENDIAN
		reg |= PDMA_BYTE_SWAP;
		#endif
	} else {/* macDrvStop */
		reg &= ~(TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	}
	REGWRITE32(PDMA_GLO_CFG, reg);
}

void macDrvDescripReset(struct macAdapter *mac_p)
{
	macRxDescr_t *pRxDescp;
	macTxDescr_t *pTxDescp;
	struct sk_buff *skb;
	int i;
	unsigned int txq = 0;

	pRxDescp = (macRxDescr_t*) mac_p->rxDescrRingBaseVAddr;
	for (i = 0; i < mac_p->rxRingSize; i++) {
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
		pTxDescp = (macTxDescr_t*) mac_p->txDescrRingBaseVAddr[txq];
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
	macRxDescr_t *pRxDescp;
  	macTxDescr_t *pTxDescp;
  	unsigned int i, txq;
  	struct sk_buff *skb;

	mac_p->rxDescrRingBaseVAddr = (unsigned int) &mac_p->macRxMemPool_p->rxDescpBuf[0];
	for(txq = 0; txq < TX_QUEUE_NUM; txq++) {
		mac_p->txDescrRingBaseVAddr[txq] = (unsigned int) &mac_p->macTxMemPool_p->txDescpBuf[txq][0];
	}
	macDrvDescripReset(mac_p);
	/* init. Rx descriptor, allocate memory for each descriptor */
	pRxDescp = (macRxDescr_t*) mac_p->rxDescrRingBaseVAddr;
	for(i = 0; i< mac_p->rxRingSize; i++, pRxDescp++) {// Init Descriptor
		pRxDescp->rxd_info1.word = 0;
		pRxDescp->rxd_info2.word = 0;
		pRxDescp->rxd_info3.word = 0;
		pRxDescp->rxd_info4.word = 0;		
		pRxDescp->rxd_info2.bits.LS0 = 1;  // Assign flag
		skb = skbmgr_dev_alloc_skb2k();
		if(skb == NULL) {
			printk("tc3262_gmac_descinit init fail.\n");
			return -1;
		}
		dma_cache_inv((unsigned long)(skb->data), RX_MAX_PKT_LEN);
		skb_reserve(skb, NET_IP_ALIGN);
		pRxDescp->rxd_info1.bits.PDP0 = K1_TO_PHY(skb->data);
		#ifdef __LITTLE_ENDIAN
		pRxDescp->rxd_info1.word = cpu_to_le32(pRxDescp->rxd_info1.word);
		pRxDescp->rxd_info2.word = cpu_to_le32(pRxDescp->rxd_info2.word);
		pRxDescp->rxd_info3.word = cpu_to_le32(pRxDescp->rxd_info3.word);
		pRxDescp->rxd_info4.word = cpu_to_le32(pRxDescp->rxd_info4.word);
		#endif
		mac_p->rxskbs[i] = skb;
	}
	for (txq = 0; txq < TX_QUEUE_NUM; txq++) {/* init. tx descriptor, don't allocate memory */
		pTxDescp = (macTxDescr_t*) mac_p->txDescrRingBaseVAddr[txq];
		for (i = 0 ; i < mac_p->txRingSize ; i++, pTxDescp++) {// Init descriptor
			pTxDescp->txd_info1.word = 0;
			pTxDescp->txd_info2.word = 0;
			pTxDescp->txd_info3.word = 0;
			pTxDescp->txd_info4.word = 0;
    		pTxDescp->txd_info2.bits.LS0_bit = 1;
    		pTxDescp->txd_info2.bits.DDONE_bit = 1;
			/* CPU */
    		//pTxDescp->txd_info4.bits.PN = 0;
			/* GDMA1 */
    		pTxDescp->txd_info4.bits.PN = 1;
    		pTxDescp->txd_info4.bits.QN = 3;
			#ifdef __LITTLE_ENDIAN
			pTxDescp->txd_info1.word = cpu_to_le32(pTxDescp->txd_info1.word);
			pTxDescp->txd_info2.word = cpu_to_le32(pTxDescp->txd_info2.word);
			pTxDescp->txd_info3.word = cpu_to_le32(pTxDescp->txd_info3.word);
			pTxDescp->txd_info4.word = cpu_to_le32(pTxDescp->txd_info4.word);
			#endif
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

int macInit(struct net_device *dev)
{
	unsigned int reg;
	int ret_val = -EFAULT;
	
	reg = REG32(CR_RSTCTRL2);
	reg |= (EPHY_RST | ESW_RST | FE_RST);
	REGWRITE32(CR_RSTCTRL2, reg);/* reset ethernet phy, ethernet switch, frame engine */
	mdelay(10);
	reg = REG32(CR_RSTCTRL2);
	reg &= ~(EPHY_RST | ESW_RST | FE_RST);
	REGWRITE32(CR_RSTCTRL2, reg);/* de-assert reset ethernet phy, ethernet switch, frame engine */
	mdelay(50);
	macDrvStart(0);
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
	if (0 != ret_val) {
		goto out;
	}
	macSetMacReg(dev->dev_addr);/* --- setup MAC address --- */
   	reg = RX_COHERENT | RX_DLY_INT | TX_COHERENT | RX_DONE_INT1 | RX_DONE_INT0;
	REGWRITE32(INT_MASK, reg);/* ----- Initialize interrupt mask ----- */    
	#ifndef TC3262_GMAC_NAPI	
	reg = RXDLY_INT_EN | (4<<RXMAX_PINT_SHIFT) | (2<<RXMAX_PTIME_SHIFT);/* 4 packets or 40us timeout to interrupt */
	REGWRITE32(DLY_INT_CFG, reg);
	#endif
	macSetDMADescrCtrlReg(mac_p);
    macSetMACCR();    
    macSetGSW();
	#ifdef CONFIG_RTL8365
	ret_val = rtl8365_switch_init();
	#else
	ret_val = tc2105mj_switch_init();
	#endif    
	
out:
	return ret_val;
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
	static unsigned char macAddr[7];
  
  	memcpy(macAddr, switch_dev[SWITCH_CPU_PORT]->dev_addr, 6);
	macAddr[6] = 0x0;
	return macAddr;
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
		reg = REG32(GSW_PMSR(port));
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
#ifdef CONFIG_RTL8365
static int get_EXT_switch_LinkSt(int len, char *buf)
{
	struct rtk_port_phy_ability_s pAbility;
	int index = len;
	int port;

	rtl8365_get_link_state();
	for(port = 0; port < SWITCH_CPU_PORT; port++) {
		index += sprintf(buf+index, "External Port %d: ", port);
		if(LINK_UP != phystate[port].pLinkStatus) {
			index += sprintf(buf+index, "Down\n");
			continue;
		}
		switch(phystate[port].pSpeed) {
		case PN_SPEED_1000M:
			index += sprintf(buf+index, "1000M/");
			break;
			
		case PN_SPEED_100M:
			index += sprintf(buf+index, "100M/");
			break;
			
		default:
			index += sprintf(buf+index, "10M/");
			break;
		}
		if(FULL_DUPLEX == phystate[port].pDuplex) {
			index += sprintf(buf+index, "Full Duplex");
		} else {
			index += sprintf(buf+index, "Half Duplex");
		}
		rtk_port_phyAutoNegoAbility_get(port, &pAbility);		
		index += sprintf(buf+index, " FC:");
		if(1 == pAbility.FC) {
			index += sprintf(buf+index, " Enabled ");
		} else {
			index += sprintf(buf+index, " Disabled");
		}
		index += sprintf(buf+index, " NWAY:");
		if(1 == pAbility.AutoNegotiation) {
			index += sprintf(buf+index, " Enabled ");
		} else {
			index += sprintf(buf+index, " Disabled");
		}		
		if(1 == pAbility.AsyFC) {
			index += sprintf(buf+index, " AsyFC");
		}
		index += sprintf(buf+index, "\n");
	}
	
	return index;		
}
#endif

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
		index += sprintf(buf+index, "RxGoodPkts      =0x%08x\n", (REG32(GSW_RGPC(port)) & RX_GOOD_CNT) >> RX_GOOD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "RxBadPkts       =0x%08x ", (REG32(GSW_RGPC(port)) & RX_BAD_CNT) >> RX_BAD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "RxGoodBytes     =0x%08x\n", REG32(GSW_RGOC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "RxBadBytes      =0x%08x ", REG32(GSW_RBOC(port)));		
		CHK_BUF();
		index += sprintf(buf+index, "RxCtrlDropPkts  =0x%08x\n", (REG32(GSW_REPC1(port)) & RX_CTRL_DROP_CNT) >> RX_CTRL_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "RxIngDropPkts   =0x%08x ", (REG32(GSW_REPC1(port)) & RX_ING_DROP_CNT) >> RX_ING_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "RxArlDropPkts   =0x%08x\n", (REG32(GSW_REPC2(port)) & RX_ARL_DROP_CNT) >> RX_ARL_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "RxFilterDropPkts=0x%08x ", (REG32(GSW_REPC2(port)) & RX_FILTER_DROP_CNT) >> RX_FILTER_DROP_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "TxGoodPkts      =0x%08x\n", (REG32(GSW_TGPC(port)) & TX_GOOD_CNT) >> TX_GOOD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "TxBadPkts       =0x%08x ", (REG32(GSW_TGPC(port)) & TX_BAD_CNT) >> TX_BAD_CNT_SHIFT);
		CHK_BUF();
		index += sprintf(buf+index, "TxGoodBytes     =0x%08x\n", REG32(GSW_TGOC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxBadBytes      =0x%08x ", REG32(GSW_TBOC(port)));
		CHK_BUF();
		index += sprintf(buf+index, "TxDropPkts      =0x%08x\n", REG32(GSW_TEPC(port)));
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

static int gsw_link_st_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	len = getGSWLinkSt(buf);
	#ifdef CONFIG_RTL8365
	len = get_EXT_switch_LinkSt(len, buf);
	#endif
	
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
	#if 0
	unsigned int reg;
	#endif
	index += sprintf(buf+index, "MDIO_ACCESS     (%08X)=%8X ", MDIO_ACCESS, REG32(MDIO_ACCESS)); 	
	CHK_BUF();
	index += sprintf(buf+index, "MDIO_CFG        (%08X)=%8X\n", MDIO_CFG, REG32(MDIO_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "FE_DMA_GLO_CFG  (%08X)=%8X ", FE_DMA_GLO_CFG, REG32(FE_DMA_GLO_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "FE_RST_GLO      (%08X)=%8X\n", FE_RST_GLO, REG32(FE_RST_GLO)); 	
	CHK_BUF();
	index += sprintf(buf+index, "FE_INT_STATUS   (%08X)=%8X ", FE_INT_STATUS, REG32(FE_INT_STATUS)); 	
	CHK_BUF();
	index += sprintf(buf+index, "FC_DROP_STA     (%08X)=%8X\n", FC_DROP_STA, REG32(FC_DROP_STA)); 	
	CHK_BUF();
	index += sprintf(buf+index, "FOE_TS_T        (%08X)=%8X ", FOE_TS_T, REG32(FOE_TS_T)); 
	CHK_BUF();
	for (i = 0; i < TX_QUEUE_NUM; i++) {
		index += sprintf(buf+index, "TX_BASE_PTR(%d)  (%08X)=%8X\n", i, TX_BASE_PTR(i), REG32(TX_BASE_PTR(i))); 	
		CHK_BUF();
		index += sprintf(buf+index, "TX_MAX_CNT(%d)   (%08X)=%8X ", i, TX_MAX_CNT(i), REG32(TX_MAX_CNT(i))); 	
		CHK_BUF();
		index += sprintf(buf+index, "TX_CTX_IDX(%d)   (%08X)=%8X\n", i, TX_CTX_IDX(i), REG32(TX_CTX_IDX(i))); 	
		CHK_BUF();
		index += sprintf(buf+index, "TX_DTX_IDX(%d)   (%08X)=%8X ", i, TX_DTX_IDX(i), REG32(TX_DTX_IDX(i))); 	
		CHK_BUF();
	}
	for (i = 0; i < RX_QUEUE_NUM; i++) {
		index += sprintf(buf+index, "RX_BASE_PTR(%d)  (%08X)=%8X\n", i, RX_BASE_PTR(i), REG32(RX_BASE_PTR(i)));
		CHK_BUF();
		index += sprintf(buf+index, "RX_MAX_CNT(%d)   (%08X)=%8X ", i, RX_MAX_CNT(i), REG32(RX_MAX_CNT(i))); 	
		CHK_BUF();
		index += sprintf(buf+index, "RX_CALC_IDX(%d)  (%08X)=%8X\n", i, RX_CALC_IDX(i), REG32(RX_CALC_IDX(i))); 	
		CHK_BUF();
		index += sprintf(buf+index, "RX_DRX_IDX(%d)   (%08X)=%8X ", i, RX_DRX_IDX(i), REG32(RX_DRX_IDX(i))); 	
		CHK_BUF();
	}
	index += sprintf(buf+index, "PDMA_INFO       (%08X)=%8X\n", PDMA_INFO, REG32(PDMA_INFO)); 	
	CHK_BUF();
	index += sprintf(buf+index, "PDMA_GLO_CFG    (%08X)=%8X ", PDMA_GLO_CFG, REG32(PDMA_GLO_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "PDMA_RST_IDX    (%08X)=%8X\n", PDMA_RST_IDX, REG32(PDMA_RST_IDX)); 	
	CHK_BUF();
	index += sprintf(buf+index, "DLY_INT_CFG     (%08X)=%8X ", DLY_INT_CFG, REG32(DLY_INT_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "FREEQ_THRES     (%08X)=%8X\n", FREEQ_THRES, REG32(FREEQ_THRES)); 	
	CHK_BUF();
	index += sprintf(buf+index, "INT_STATUS      (%08X)=%8X ", INT_STATUS, REG32(INT_STATUS)); 	
	CHK_BUF();
	index += sprintf(buf+index, "INT_MASK        (%08X)=%8X\n", INT_MASK, REG32(INT_MASK)); 	
	CHK_BUF();
	index += sprintf(buf+index, "SCH_Q01_CFG     (%08X)=%8X ", SCH_Q01_CFG, REG32(SCH_Q01_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "SCH_Q23_CFG     (%08X)=%8X\n", SCH_Q23_CFG, REG32(SCH_Q23_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_FWD_CFG   (%08X)=%8X ", GDMA1_FWD_CFG, REG32(GDMA1_FWD_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_SCH_CFG   (%08X)=%8X\n", GDMA1_SCH_CFG, REG32(GDMA1_SCH_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_SHRP_CFG  (%08X)=%8X ", GDMA1_SHRP_CFG, REG32(GDMA1_SHRP_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_MAC_ADRL  (%08X)=%8X\n", GDMA1_MAC_ADRL, REG32(GDMA1_MAC_ADRL)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_MAC_ADRH  (%08X)=%8X ", GDMA1_MAC_ADRH, REG32(GDMA1_MAC_ADRH)); 	
	CHK_BUF();
	index += sprintf(buf+index, "PSE_FQFC_CFG    (%08X)=%8X\n", PSE_FQFC_CFG, REG32(PSE_FQFC_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "CDMA_FC_CFG     (%08X)=%8X ", CDMA_FC_CFG, REG32(CDMA_FC_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_FC_CFG    (%08X)=%8X\n", GDMA1_FC_CFG, REG32(GDMA1_FC_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_FC_CFG    (%08X)=%8X ", GDMA2_FC_CFG, REG32(GDMA2_FC_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "CDMA_OQ_STA     (%08X)=%8X\n", CDMA_OQ_STA, REG32(CDMA_OQ_STA)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA1_OQ_STA    (%08X)=%8X ", GDMA1_OQ_STA, REG32(GDMA1_OQ_STA)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_OQ_STA    (%08X)=%8X\n", GDMA2_OQ_STA, REG32(GDMA2_OQ_STA)); 	
	CHK_BUF();
	index += sprintf(buf+index, "PSE_IQ_STA      (%08X)=%8X ", PSE_IQ_STA, REG32(PSE_IQ_STA)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_FWD_CFG   (%08X)=%8X\n", GDMA2_FWD_CFG, REG32(GDMA2_FWD_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_SCH_CFG   (%08X)=%8X ", GDMA2_SCH_CFG, REG32(GDMA2_SCH_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_SHRP_CFG  (%08X)=%8X\n", GDMA2_SHRP_CFG, REG32(GDMA2_SHRP_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_MAC_ADRL  (%08X)=%8X ", GDMA2_MAC_ADRL, REG32(GDMA2_MAC_ADRL)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA2_MAC_ADRH  (%08X)=%8X\n", GDMA2_MAC_ADRH, REG32(GDMA2_MAC_ADRH)); 	
	CHK_BUF();
	index += sprintf(buf+index, "CDMA_CSG_CFG    (%08X)=%8X ", CDMA_CSG_CFG, REG32(CDMA_CSG_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "CDMA_SCH_CFG    (%08X)=%8X\n", CDMA_SCH_CFG, REG32(CDMA_SCH_CFG)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_TX_GBCNT1  (%08X)=%8X ", GDMA_TX_GBCNT1, REG32(GDMA_TX_GBCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_TX_GPCNT1  (%08X)=%8X\n", GDMA_TX_GPCNT1, REG32(GDMA_TX_GPCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_TX_SKIPCNT1(%08X)=%8X ", GDMA_TX_SKIPCNT1, REG32(GDMA_TX_SKIPCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_TX_COLCNT1 (%08X)=%8X\n", GDMA_TX_COLCNT1, REG32(GDMA_TX_COLCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_GBCNT1  (%08X)=%8X ", GDMA_RX_GBCNT1, REG32(GDMA_RX_GBCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_GPCNT1  (%08X)=%8X\n", GDMA_RX_GPCNT1, REG32(GDMA_RX_GPCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_OERCNT1 (%08X)=%8X ", GDMA_RX_OERCNT1, REG32(GDMA_RX_OERCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_FERCNT1 (%08X)=%8X\n", GDMA_RX_FERCNT1, REG32(GDMA_RX_FERCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_SERCNT1 (%08X)=%8X ", GDMA_RX_SERCNT1, REG32(GDMA_RX_SERCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_LERCNT1 (%08X)=%8X\n", GDMA_RX_LERCNT1, REG32(GDMA_RX_LERCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_CERCNT1 (%08X)=%8X ", GDMA_RX_CERCNT1, REG32(GDMA_RX_CERCNT1)); 	
	CHK_BUF();
	index += sprintf(buf+index, "GDMA_RX_FCCNT1  (%08X)=%8X\n", GDMA_RX_FCCNT1, REG32(GDMA_RX_FCCNT1)); 	
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
	macRxDescr_t *pRxDescp;
  	macRxDescr_t pRxDescpTmpVal;
  	macRxDescr_t *pRxDescpTmp = &pRxDescpTmpVal;

	pRxDescp = (macRxDescr_t*) mac_p->rxDescrRingBaseVAddr;
	index += sprintf(buf+index, "rx descr ring=%08x\n", (unsigned int) pRxDescp);
	CHK_BUF();
	for(i = 0 ; i< mac_p->rxRingSize ; i++, pRxDescp++) {
		#ifdef __BIG_ENDIAN
		pRxDescpTmp = pRxDescp;
		#else
		pRxDescpTmp->rxd_info1.word = le32_to_cpu(pRxDescp->rxd_info1.word);
		pRxDescpTmp->rxd_info2.word = le32_to_cpu(pRxDescp->rxd_info2.word);
		pRxDescpTmp->rxd_info3.word = le32_to_cpu(pRxDescp->rxd_info3.word);
		pRxDescpTmp->rxd_info4.word = le32_to_cpu(pRxDescp->rxd_info4.word);
		#endif
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
	index += sprintf(buf+index, "RX_CALC_IDX(0)    =%08x\n", REG32(RX_CALC_IDX(0)));
	CHK_BUF();
	index += sprintf(buf+index, "RX_DRX_IDX(0)     =%08x\n", REG32(RX_DRX_IDX(0)));
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
  	macTxDescr_t *pTxDescp;
  	macTxDescr_t pTxDescpTmpVal;
  	macTxDescr_t *pTxDescpTmp = &pTxDescpTmpVal;

	txq = *((int *) data);
	pTxDescp = (macTxDescr_t*) mac_p->txDescrRingBaseVAddr[txq];
	index += sprintf(buf+index, "tx descr ring%d=%08x\n", txq, (unsigned int) pTxDescp);
	CHK_BUF();
	
	for (i = 0 ; i < mac_p->txRingSize ; i++, pTxDescp++) {
		#ifdef __BIG_ENDIAN
		pTxDescpTmp = pTxDescp;
		#else
		pTxDescpTmp->txd_info1.word = le32_to_cpu(pTxDescp->txd_info1.word);
		pTxDescpTmp->txd_info2.word = le32_to_cpu(pTxDescp->txd_info2.word);
		pTxDescpTmp->txd_info3.word = le32_to_cpu(pTxDescp->txd_info3.word);
		pTxDescpTmp->txd_info4.word = le32_to_cpu(pTxDescp->txd_info4.word);
		#endif
		index += sprintf(buf+index, "i= %d descr=%08x\n", i, (unsigned int) pTxDescp);
		CHK_BUF();
		index += sprintf(buf+index, " tdes1=%08x\n", pTxDescpTmp->txd_info1.word);
		CHK_BUF();
		index += sprintf(buf+index, " tdes2=%08x\n", pTxDescpTmp->txd_info2.word);
		CHK_BUF();
		index += sprintf(buf+index, " tdes3=%08x\n", pTxDescpTmp->txd_info3.word);
		CHK_BUF();
		index += sprintf(buf+index, " tdes4=%08x\n", pTxDescpTmp->txd_info4.word);
		CHK_BUF();
		index += sprintf(buf+index, " skb  =%08x\n", (unsigned int) mac_p->txskbs[txq][i]);
		CHK_BUF();
	}
	index += sprintf(buf+index, "txCurrentDescp[%d]    =%d\n", txq, mac_p->txCurrentDescp[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "txUnReleasedDescp[%d] =%d\n", txq, mac_p->txUnReleasedDescp[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "txUnReleasedBufCnt[%d]=%d\n", txq, mac_p->txUnReleasedBufCnt[txq]);
	CHK_BUF();
	index += sprintf(buf+index, "TX_CTX_IDX(%d)        =%08x\n", txq, REG32(TX_CTX_IDX(txq)));
	CHK_BUF();
	index += sprintf(buf+index, "TX_DTX_IDX(%d)        =%08x\n", txq, REG32(TX_DTX_IDX(txq)));
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
		reg = REG32(SCH_Q01_CFG);
		reg &= ~(MAX_WEIGHT1 | MIN_RATE_RATIO1 | MAX_WEIGHT0 | MIN_RATE_RATIO0);
		reg |= (MIN_RATIO0<<MIN_RATE_RATIO1_SHIFT) | (MIN_RATIO0<<MIN_RATE_RATIO0_SHIFT);
		reg |= (MAX_WEIGHT_2047<<MAX_WEIGHT1_SHIFT) | (MAX_WEIGHT_1023<<MAX_WEIGHT0_SHIFT);
		REGWRITE32(SCH_Q01_CFG, reg);

		reg = REG32(SCH_Q23_CFG);
		reg &= ~(MAX_WEIGHT3 | MIN_RATE_RATIO3 | MAX_WEIGHT2 | MIN_RATE_RATIO2);
		reg |= (MIN_RATIO0<<MIN_RATE_RATIO3_SHIFT) | (MIN_RATIO0<<MIN_RATE_RATIO2_SHIFT);
		reg |= (MAX_WEIGHT_8191<<MAX_WEIGHT3_SHIFT) | (MAX_WEIGHT_4095<<MAX_WEIGHT2_SHIFT);
		REGWRITE32(SCH_Q23_CFG, reg);

		/* set GDMA2_SCH_CFG */
		reg = REG32(GDMA2_SCH_CFG);
		reg &= ~GDM_SCH_MOD;
		reg |= GDM_SCH_MOD_SP<<GDM_SCH_MOD_SHIFT;
		REGWRITE32(GDMA2_SCH_CFG, reg);
	}	
	else {  /*WRR*/
		/* Min BW = 0, Max BW = unlimited */
		reg = REG32(SCH_Q01_CFG);
		reg &= ~(MAX_WEIGHT1 | MIN_RATE_RATIO1 | MAX_WEIGHT0 | MIN_RATE_RATIO0);
		reg |= (MIN_RATIO3<<MIN_RATE_RATIO1_SHIFT) | (MIN_RATIO3<<MIN_RATE_RATIO0_SHIFT);
		reg |= (get_qos_weight(qos_wrr_info[3] & 0x0f)<<MAX_WEIGHT1_SHIFT) | (get_qos_weight(qos_wrr_info[4] & 0x0f)<<MAX_WEIGHT0_SHIFT);
		REGWRITE32(SCH_Q01_CFG, reg);

		reg = REG32(SCH_Q23_CFG);
		reg &= ~(MAX_WEIGHT3 | MIN_RATE_RATIO3 | MAX_WEIGHT2 | MIN_RATE_RATIO2);
		reg |= (MIN_RATIO3<<MIN_RATE_RATIO3_SHIFT) | (MIN_RATIO3<<MIN_RATE_RATIO2_SHIFT);
		reg |= (get_qos_weight(qos_wrr_info[1] & 0x0f)<<MAX_WEIGHT3_SHIFT) | (get_qos_weight(qos_wrr_info[2] & 0x0f)<<MAX_WEIGHT2_SHIFT);
		REGWRITE32(SCH_Q23_CFG, reg);

		/* set GDMA2_SCH_CFG */
		reg = REG32(GDMA2_SCH_CFG);
		reg &= ~(GDM_SCH_MOD | GDM_WT_Q3 | GDM_WT_Q2 | GDM_WT_Q1 | GDM_WT_Q0);
		reg |= (GDM_SCH_MOD_WRR<<GDM_SCH_MOD_SHIFT) | 
					(GDM_WT(qos_wrr_info[1] & 0x0f)<<GDM_WT_Q3_SHIFT) | 
					(GDM_WT(qos_wrr_info[2] & 0x0f)<<GDM_WT_Q2_SHIFT) | 
					(GDM_WT(qos_wrr_info[3] & 0x0f)<<GDM_WT_Q1_SHIFT) | 
					(GDM_WT(qos_wrr_info[4] & 0x0f)<<GDM_WT_Q0_SHIFT);
		REGWRITE32(GDMA2_SCH_CFG, reg);

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

macTxDescr_t *macTxRingProc(struct macAdapter *mac_p, unsigned int txq)
{
	volatile macTxDescr_t *pTxDescp;
	volatile macTxDescr_t pTxDescpTmpVal;
	volatile macTxDescr_t *pTxDescpTmp = &pTxDescpTmpVal;
	unsigned long flags;
	struct sk_buff *freeskb;

	spin_lock_irqsave(&mac_p->lock, flags);
	pTxDescp = ((macTxDescr_t*)mac_p->txDescrRingBaseVAddr[txq]) + mac_p->txUnReleasedDescp[txq];

	#ifdef __BIG_ENDIAN
	pTxDescpTmp = pTxDescp;
	#else
	pTxDescpTmp->txd_info1.word = le32_to_cpu(pTxDescp->txd_info1.word);
	pTxDescpTmp->txd_info2.word = le32_to_cpu(pTxDescp->txd_info2.word);
	pTxDescpTmp->txd_info3.word = le32_to_cpu(pTxDescp->txd_info3.word);
	pTxDescpTmp->txd_info4.word = le32_to_cpu(pTxDescp->txd_info4.word);
	#endif
  	while (mac_p->txUnReleasedBufCnt[txq] != 0) {
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
		pTxDescp = ((macTxDescr_t*)mac_p->txDescrRingBaseVAddr[txq]) + mac_p->txUnReleasedDescp[txq];
		#ifdef __BIG_ENDIAN
		pTxDescpTmp = pTxDescp;
		#else
		pTxDescpTmp->txd_info1.word = le32_to_cpu(pTxDescp->txd_info1.word);
		pTxDescpTmp->txd_info2.word = le32_to_cpu(pTxDescp->txd_info2.word);
		pTxDescpTmp->txd_info3.word = le32_to_cpu(pTxDescp->txd_info3.word);
		pTxDescpTmp->txd_info4.word = le32_to_cpu(pTxDescp->txd_info4.word);
		#endif
    	mac_p->macStat.inSilicon.txDeQueueNum++;
	} 
	spin_unlock_irqrestore(&mac_p->lock, flags);

	return (macTxDescr_t*) pTxDescp;
}

__IMEM int tc3262_gmac_tx(struct sk_buff *skb, struct net_device *dev)
{
	volatile macTxDescr_t *currDescrp = NULL;
	volatile macTxDescr_t currDescrpTmpVal;
	volatile macTxDescr_t *currDescrpTmp = &currDescrpTmpVal;
	unsigned int length, data_len;
	unsigned char *bufAddrp;
	unsigned long flags;
	unsigned int txq = 0;
	struct sk_buff *nskb = NULL;
	struct net_device_stats *stats = NULL;
	
	#ifdef TC_CONSOLE_ENABLE
	if(skb->data[12] == 0xaa) {/* isTCConsolePkt */
		skb->mark |= SKBUF_TCCONSOLE;
	}
	#endif
	data_len = skb->len;
	if((NULL != dev) && (dev->base_addr < SWITCH_CPU_PORT)) {
		stats = (struct net_device_stats *)netdev_priv(dev);
		#ifdef CONFIG_ETHERNET_DEBUG
		if(dump_mask & DUMP_TX_UNTAG) {
			dump_skb(skb, "TX_UNTAGGED");
		}
		#endif
		#ifdef CONFIG_RTL8365_CPU_TAG	
		nskb = rtl8365_cpu_tag_insert(skb, dev);
		#elif defined(CONFIG_RTL8365)
		nskb = rtl8365_vlan_tag_insert(skb, dev);
		#else
		nskb = tc2105mj_vlan_tag_insert(skb, dev);
		#endif
		if(NULL == nskb) {
			if(NULL != stats) {
				stats->tx_dropped++;
			}
			goto out;
		} else {
			skb = nskb;
		}
		#ifdef CONFIG_ETHERNET_DEBUG
		if(dump_mask & DUMP_TX_TAG) {
			dump_skb(skb, "TX_TAGGED");
		}
		#endif
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
		if (ra_sw_nat_hook_txq)
			ra_sw_nat_hook_txq(skb, txq);
		if (ra_sw_nat_hook_tx(skb, 1) == 0) {
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
	currDescrp = ((macTxDescr_t *) mac_p->txDescrRingBaseVAddr[txq]) + mac_p->txCurrentDescp[txq];
	#ifdef __BIG_ENDIAN
	currDescrpTmp = currDescrp;
	#else
	currDescrpTmp->txd_info1.word = le32_to_cpu(currDescrp->txd_info1.word);
	currDescrpTmp->txd_info2.word = le32_to_cpu(currDescrp->txd_info2.word);
	currDescrpTmp->txd_info3.word = le32_to_cpu(currDescrp->txd_info3.word);
	currDescrpTmp->txd_info4.word = le32_to_cpu(currDescrp->txd_info4.word);
	#endif
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
  	currDescrpTmp->txd_info4.bits.QN = 3;
	currDescrpTmp->txd_info4.bits.PN = 1;/* GDMA1 */

	#ifdef TCSUPPORT_RA_HWNAT
	if (ra_sw_nat_hook_magic) {
		if (ra_sw_nat_hook_magic(skb, FOE_MAGIC_PPE)) {/* PPE */			
   			currDescrpTmp->txd_info4.bits.PN = 6; 
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
	#if !defined(__BIG_ENDIAN) && !defined(__LITTLE_ENDIAN)
	currDescrp->txd_info1.word = cpu_to_le32(currDescrpTmp->txd_info1.word);
	currDescrp->txd_info2.word = cpu_to_le32(currDescrpTmp->txd_info2.word);
	currDescrp->txd_info3.word = cpu_to_le32(currDescrpTmp->txd_info3.word);
	currDescrp->txd_info4.word = cpu_to_le32(currDescrpTmp->txd_info4.word);
	#endif
	mac_p->txCurrentDescp[txq] = (mac_p->txCurrentDescp[txq] + 1) % mac_p->txRingSize;
	wmb();
	REGWRITE32(TX_CTX_IDX(txq), mac_p->txCurrentDescp[txq]);
	mac_p->txUnReleasedBufCnt[txq]++;
  	mac_p->macStat.inSilicon.txEnQueueNum++;
	spin_unlock_irqrestore(&mac_p->lock, flags);
out:	
	return NETDEV_TX_OK;
}

__IMEM int macRxRingProc(int quota)
{
	volatile macRxDescr_t *rxDescrp;
	volatile macRxDescr_t rxDescrpTmpVal;
	volatile macRxDescr_t *rxDescrpTmp = &rxDescrpTmpVal;
	unsigned int frameSize;
	int npackets = 0;
	struct sk_buff *newskb;
	struct sk_buff *skb;
	
	rxDescrp = ((macRxDescr_t*)mac_p->rxDescrRingBaseVAddr) + mac_p->rxCurrentDescp;
	#ifdef __BIG_ENDIAN
	rxDescrpTmp = rxDescrp;
	#else
	rxDescrpTmp->rxd_info1.word = le32_to_cpu(rxDescrp->rxd_info1.word);
	rxDescrpTmp->rxd_info2.word = le32_to_cpu(rxDescrp->rxd_info2.word);
	rxDescrpTmp->rxd_info3.word = le32_to_cpu(rxDescrp->rxd_info3.word);
	rxDescrpTmp->rxd_info4.word = le32_to_cpu(rxDescrp->rxd_info4.word);
	#endif
  	while((rxDescrpTmp->rxd_info2.bits.DDONE_bit) 
		#ifdef TC3262_GMAC_NAPI
		&& (npackets < quota)
		#endif
		) {
		npackets++;
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
		#ifdef CONFIG_RTL8365_CPU_TAG
		rtl8365_cpu_tag_remove(skb);
		#elif defined(CONFIG_RTL8365)
		rtl8365_vlan_tag_remove(skb);
		#else
		tc2105mj_vlan_tag_remove(skb);
		#endif
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
			if ((rxDescrpTmp->rxd_info4.bits.L4FVLD_bit && (rxDescrpTmp->rxd_info4.bits.L4F == 0)) ||
				(rxDescrpTmp->rxd_info4.bits.IPFVLD_bit && (rxDescrpTmp->rxd_info4.bits.IPF == 0)))
				skb->ip_summed = CHECKSUM_UNNECESSARY;
			else 
			#endif
				skb->ip_summed = CHECKSUM_NONE;
			skb->protocol = eth_type_trans(skb, skb->dev);
			skb->dev->last_rx = jiffies;
			#ifdef CONFIG_ETHERNET_DEBUG
			if(dump_mask & DUMP_RX_TAG) {
				dump_skb(skb, "RX_BEFORE_HWNAT");
			}
			#endif
			#ifdef TCSUPPORT_RA_HWNAT
			if(ra_sw_nat_hook_rxinfo) {
				ra_sw_nat_hook_rxinfo(skb, FOE_MAGIC_GE, (char *)&rxDescrpTmp->rxd_info4, sizeof(PDMA_RXD_INFO4_T));
			}
			#endif
			if((NULL == ra_sw_nat_hook_rx) 
				#ifdef TCSUPPORT_RA_HWNAT
				|| ((NULL != ra_sw_nat_hook_rx) && (0 != ra_sw_nat_hook_rx(skb)))
				#endif
				) {
				#ifdef TC3262_GMAC_NAPI
				netif_receive_skb(skb);
				#else
				netif_rx(skb);
				#endif
			}
		}
DISCARD:
RECVOK:
		rxDescrpTmp->rxd_info1.bits.PDP0 = K1_TO_PHY(newskb->data);
       	mac_p->rxskbs[mac_p->rxCurrentDescp] = newskb;
		rxDescrpTmp->rxd_info2.word = 0;
		rxDescrpTmp->rxd_info2.bits.LS0 = 1;
		#ifdef __LITTLE_ENDIAN
		rxDescrp->rxd_info1.word = le32_to_cpu(rxDescrpTmp->rxd_info1.word);
		rxDescrp->rxd_info2.word = le32_to_cpu(rxDescrpTmp->rxd_info2.word);
		rxDescrp->rxd_info3.word = le32_to_cpu(rxDescrpTmp->rxd_info3.word);
		rxDescrp->rxd_info4.word = le32_to_cpu(rxDescrpTmp->rxd_info4.word);
		#endif
		wmb();
		REGWRITE32(RX_CALC_IDX(0), mac_p->rxCurrentDescp);
		mac_p->rxCurrentDescp = (mac_p->rxCurrentDescp + 1) % mac_p->rxRingSize;/* next descriptor*/
		rxDescrp = ((macRxDescr_t*)mac_p->rxDescrRingBaseVAddr) + mac_p->rxCurrentDescp;
		#ifdef __BIG_ENDIAN
		rxDescrpTmp = rxDescrp;
		#else
		rxDescrpTmp->rxd_info1.word = le32_to_cpu(rxDescrp->rxd_info1.word);
		rxDescrpTmp->rxd_info2.word = le32_to_cpu(rxDescrp->rxd_info2.word);
		rxDescrpTmp->rxd_info3.word = le32_to_cpu(rxDescrp->rxd_info3.word);
		rxDescrpTmp->rxd_info4.word = le32_to_cpu(rxDescrp->rxd_info4.word);
		#endif
	}

	return npackets;
}


static irqreturn_t tc3262_gmac_isr(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;
	unsigned int reg;

#if KERNEL_2_6_36
	struct macAdapter *mac_p = netdev_priv(dev);
#endif
	reg = REG32(INT_STATUS);
	REGWRITE32(INT_STATUS, reg);
	// ----------Packet Received----------------------
	if (reg & (RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0)) {
#ifdef TC3262_GMAC_NAPI
		spin_lock(&gimr_lock);
#if KERNEL_2_6_36
		if(mac_p==NULL)
			return IRQ_HANDLED;
		if (napi_schedule_prep(&mac_p->napi)) {
		REGWRITE32(INT_MASK, REG32(INT_MASK) & ~(RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0));	
			__napi_schedule(&mac_p->napi);
		}
#else
		if (netif_rx_schedule_prep(dev)) {
			REGWRITE32(INT_MASK, REG32(INT_MASK) & ~(RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0));
			__netif_rx_schedule(dev);
		}

#endif
		spin_unlock(&gimr_lock);
#else
    	macRxRingProc(dev->weight);
#endif
	}
	if(reg & (RX_COHERENT | TX_COHERENT)) {
		printk("%s err mac_isr INT_STATUS=%08x\n", dev->name, reg);
	}			

	return IRQ_HANDLED;
}



/* Starting up the ethernet device */
static int tc3262_gmac_open(struct net_device *dev)
{
	int ret_val = -EINVAL;

	ret_val = request_irq(dev->irq, tc3262_gmac_isr, IRQF_DISABLED, dev->name, dev);
	if(ret_val) {
		goto out;
	}
	mac_p->statisticOn = MAC_STATISTIC_ON;
	macDrvStart(1);
	/* MII setup */
	mac_p->mii_if.phy_id = mac_p->enetPhyAddr;
	mac_p->mii_if.full_duplex = 1;
	mac_p->mii_if.phy_id_mask = 0x1f;
	mac_p->mii_if.reg_num_mask = 0x1f;
	mac_p->mii_if.dev = dev;
	mac_p->mii_if.mdio_read = mdio_read;
	mac_p->mii_if.mdio_write = mdio_write;
	mac_p->mii_if.supports_gmii = mii_check_gmii_support(&mac_p->mii_if);
	#if KERNEL_2_6_36
	napi_enable(&mac_p->napi);
	#endif
	netif_start_queue(dev);

out:
  	return ret_val;
}

/* Stopping the ethernet device */
static int tc3262_gmac_close(struct net_device *dev)
{
	macDrvStart(0);
  	netif_stop_queue(dev);
	#if KERNEL_2_6_36
	napi_disable(&mac_p->napi);
	#endif
	free_irq(dev->irq, dev);

  	return 0;
}

/* Setup multicast list */
static void tc3262_gmac_set_multicast_list(struct net_device *dev)
{
	return; /* Do nothing */
}

/* Setting customized mac address */
int tc3262_gmac_set_macaddr(struct net_device *dev, void *p)
{
	struct sockaddr *addr = p;

	/* Check if given address is valid ethernet MAC address */
  	if (!is_valid_ether_addr(addr->sa_data))
    	return(-EIO);

	/* Save the customize mac address */
  	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
	macSetMacReg(addr->sa_data);

	return 0;
}

/* Get the stats information */
static struct net_device_stats *tc3262_gmac_stats(struct net_device *dev)
{
	struct net_device_stats *stats;

	stats = &mac_p->stats;

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

	return stats;
}

/* Handling ioctl call */
static int tc3262_gmac_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int rc = 0;
	gsw_reg reg;

	switch (cmd) {
		case RAETH_GSW_REG_READ:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			reg.val = REG32(GSW_BASE + reg.off);
			copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
			break;
		case RAETH_GSW_REG_WRITE:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			REGWRITE32(GSW_BASE + reg.off, reg.val);
			break;
		case RAETH_REG_READ:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			reg.val = REG32(reg.off);
			copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
			break;
		case RAETH_REG_WRITE:
			copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
			REGWRITE32(reg.off, reg.val);
			break;
		default:
			rc = generic_mii_ioctl(&mac_p->mii_if, if_mii(ifr), cmd, NULL);
			break;
	}

	return rc;
}

#ifdef TC3262_GMAC_NAPI
#if KERNEL_2_6_36
static int tc3262_gmac_poll(struct napi_struct *napi, int budget)
{ 
	int rx_work_limit=0;
	int received = 0;
	int n=0;
	unsigned long flags=0;

	rx_work_limit = min(budget - received, budget);
	n = macRxRingProc(rx_work_limit);
	received += n;
	if(received < budget) {
		spin_lock_irqsave(&gimr_lock, flags);
		__napi_complete(napi);
		REGWRITE32(INT_MASK, REG32(INT_MASK) | (RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0));	
		spin_unlock_irqrestore(&gimr_lock, flags);
	}

	return received;
}
#else

static int tc3262_gmac_poll(struct net_device *dev, int *budget)
{
	int rx_work_limit = min(dev->quota, *budget);
	int received = 0;
	int n, done;
	unsigned long flags;

	n = macRxRingProc(rx_work_limit);
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

	REGWRITE32(INT_MASK, REG32(INT_MASK) | 
		(RX_DLY_INT | RX_DONE_INT1 | RX_DONE_INT0));

	spin_unlock_irqrestore(&gimr_lock, flags);

more_work:
	*budget -= received;
	dev->quota -= received;

	return done ? 0 : 1;
}
#endif

#endif

static int tc3262_gmac_start(struct net_device *dev)
{
	mac_p = netdev_priv(dev);
	spin_lock_init(&mac_p->lock);
	spin_lock_init(&phy_lock);

	#if KERNEL_2_6_36
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

	#endif
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
	.ndo_get_stats			= tc3262_gmac_stats,
	.ndo_set_multicast_list = tc3262_gmac_set_multicast_list,
	.ndo_do_ioctl			= tc3262_gmac_ioctl,
	.ndo_change_mtu			= eth_change_mtu,
	.ndo_set_mac_address 	= tc3262_gmac_set_macaddr,
	.ndo_validate_addr		= eth_validate_addr,
	#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= tc3262_gmac_poll_controller,
	#endif
};

#endif

static int __init tc3262_gmac_init(void)
{
  	struct net_device *dev;
  	int err = -ENOMEM;
	struct proc_dir_entry *eth_proc;
	int txq;

	
	dev = alloc_etherdev(sizeof(struct macAdapter));
	if(NULL == dev) {
		goto out;
	}
	memset(switch_dev, 0x00, sizeof(switch_dev));
	switch_dev[SWITCH_CPU_PORT] = dev;
	dev->base_addr = 50;
	dev->irq = MAC_INT;
	printk("GEMAC 2.00-NAPI 24.Sep.2012, IRQ=%d\n", dev->irq);
	mac_p = netdev_priv(dev);
	#if KERNEL_2_6_36 	
	dev->netdev_ops = &gmac_netdev_ops;/* Hook up with handlers */
	mac_p->napi.weight = MAC_NAPI_WEIGHT;
	netif_napi_add(dev, &mac_p->napi, tc3262_gmac_poll, MAC_NAPI_WEIGHT);
	#else
	dev->init = tc3262_gmac_start;
	#endif
	err = register_netdev(dev);
	if(err) {
		printk("Fail to Register GMAC device!\n");
		goto out;
	}
	err = macInit(dev);
	if(err) {
		printk("Fail to Initial GMAC!\n");
		goto out;
	}
  	/* ethernet related stats */
	eth_proc = create_proc_entry("tc3162/eth_stats", 0, NULL);
	eth_proc->read_proc = eth_stats_read_proc;
	eth_proc->write_proc = eth_stats_write_proc;
	eth_proc = create_proc_entry("tc3162/gsw_stats", 0, NULL);
	eth_proc->read_proc = gsw_stats_read_proc;
	eth_proc->write_proc = gsw_stats_write_proc;
  	create_proc_read_entry("tc3162/link_status", 0, NULL, gsw_link_st_proc, NULL);
  	create_proc_read_entry("tc3162/eth_reg", 0, NULL, eth_reg_dump_proc, NULL);
	for(txq = 0; txq < TX_QUEUE_NUM; txq++) {
		proc_txring[txq] = txq;
		sprintf(proc_txring_name[txq], "tc3162/eth_txring%d", txq);
		create_proc_read_entry(proc_txring_name[txq], 0, NULL, eth_txring_dump_proc, &proc_txring[txq]);
	}
  	create_proc_read_entry("tc3162/eth_rxring", 0, NULL, eth_rxring_dump_proc, NULL);
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
	#ifdef CONFIG_ETHERNET_DEBUG
	eth_proc = create_proc_entry("tc3162/dump", 0, NULL);
	eth_proc->write_proc = dump_level_write_proc;
	#endif
	#ifdef TC_CONSOLE_ENABLE
	create_tcconsole_proc();
	#endif

    /*
     * TBS_TAG:add by pengyao 20130311
     * Desc: ether net interface for tbs_nfp
     */
    dev->priv_flags |= IFF_ETH;
    /*
     * TBS_TAG:END
     */

out:
  	return err;
}

static void __exit tc3262_gmac_exit(void)
{
	int txq;
	unsigned int reg;

	reg = REG32(CR_RSTCTRL2);
	reg |= (EPHY_RST | ESW_RST | FE_RST);
	REGWRITE32(CR_RSTCTRL2, reg);/* reset ethernet phy, ethernet switch, frame engine */	
	reg = REG32(CR_RSTCTRL2);
	reg &= ~(EPHY_RST | ESW_RST | FE_RST);
	REGWRITE32(CR_RSTCTRL2, reg);/* de-assert reset ethernet phy, ethernet switch, frame engine */
	mdelay(100);
	#ifdef CONFIG_RTL8365
	rtl8365_switch_exit();
	#else
	tc2105mj_switch_exit();
	#endif
	remove_proc_entry("tc3162/eth_stats", 0);
	remove_proc_entry("tc3162/gsw_stats", 0);
	remove_proc_entry("tc3162/gsw_link_st", 0);
   	remove_proc_entry("tc3162/eth_link_st", 0);
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
	#ifdef CONFIG_ETHERNET_DEBUG
	remove_proc_entry("tc3162/dump", 0);
	#endif
	#ifdef TC_CONSOLE_ENABLE
	delete_tcconsole_proc();
	#endif
	macDrvDescripReset(mac_p);
	unregister_netdev(switch_dev[SWITCH_CPU_PORT]);
	if (mac_p->macTxMemPool_p) {
	#ifdef CONFIG_TC3162_DMEM
		if (is_sram_addr(mac_p->macTxMemPool_p))
			free_sram(mac_p->macTxMemPool_p, sizeof(macTxMemPool_t));
		else
	#endif
		dma_free_coherent(NULL, sizeof(macTxMemPool_t), mac_p->macTxMemPool_p, mac_p->macTxMemPool_phys_p);
	}
	if (mac_p->macRxMemPool_p) {
	#ifdef CONFIG_TC3162_DMEM
		if (is_sram_addr(mac_p->macRxMemPool_p))
			free_sram(mac_p->macRxMemPool_p, sizeof(macRxMemPool_t));
		else
	#endif
		dma_free_coherent(NULL, sizeof(macRxMemPool_t), mac_p->macRxMemPool_p, mac_p->macRxMemPool_phys_p);
	}
	free_netdev(switch_dev[SWITCH_CPU_PORT]);
}

/* Register startup/shutdown routines */
module_init(tc3262_gmac_init);
module_exit(tc3262_gmac_exit);
MODULE_LICENSE("GPL");


