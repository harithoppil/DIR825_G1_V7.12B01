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
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/etherdevice.h>
#include <linux/mii.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/ledcetrl.h>
#include <asm/tc3162/TCIfSetQuery_os.h>
#include <gpio.h>
#include <led.h>
#include "gemac.h"
#include "tcswitch.h"
#include <autoconf.h>

#define TC2105MJ_PHY_INIT_GDATA_LEN 15
#define TC2105MJ_PHY_INIT_LDATA_LEN 1
#define TC2105MJ_PHY_INIT_PERPDATA_LEN 1
#define TC2105MJ_PHY_INIT_SET_NUM 1
#define TC2105MJ_PORTNUM 5

extern struct net_device *switch_dev[SWITCH_PORT_MAX];
extern struct phy_link_state phystate[SWITCH_PORT_MAX];
extern struct macAdapter *mac_p;
#ifdef WAN2LAN
extern unsigned char masko;
#endif
/* Timer */
static struct timer_list link_state_timer;
static struct tasklet_struct link_dsr_tasklet;
static spinlock_t smi_lock;



void smi_init(void)
{
	spin_lock_init(&smi_lock);
}

unsigned int tcPhyReadReg(unsigned char port_num, unsigned char reg_num)
{
	unsigned int val, val_r31;
	unsigned int phyAddr = port_num;
	unsigned long flags;
	
	spin_lock_irqsave(&smi_lock, flags);
	val_r31 = miiStationRead(phyAddr, 31);// page to L0
	miiStationWrite(phyAddr, 31, 0x8000);
	val = miiStationRead(phyAddr, reg_num);// read reg
	if(val_r31 != 0x8000) {// restore page if necessary
		miiStationWrite(phyAddr, 31, val_r31);
	}
	spin_unlock_irqrestore(&smi_lock, flags);

	return val;
}

// read Global Reg
unsigned int tcPhyReadGReg(unsigned char port_num,unsigned char page_num,unsigned char reg_num)
{
	unsigned int val, val_r31;
	unsigned int phyAddr = port_num;
	unsigned int pageAddr = (page_num<<12);
	unsigned long flags;
	
	spin_lock_irqsave(&smi_lock, flags);
	val_r31 = miiStationRead(phyAddr, 31);// page to L0
	miiStationWrite(phyAddr, 31, pageAddr);
	val = miiStationRead(phyAddr, reg_num);
	if(val_r31 != pageAddr) {// restore page if necessary
		miiStationWrite(phyAddr, 31, val_r31);
	}
	spin_unlock_irqrestore(&smi_lock, flags);

	return val;
}

void tcPhyWriteGReg(unsigned char port_num,unsigned char page_num,unsigned char reg_num,unsigned int reg_data)
{
	unsigned int val_r31;
	unsigned int phyAddr = port_num;
	unsigned int pageAddr = (page_num<<12);
	unsigned long flags;
	
	spin_lock_irqsave(&smi_lock, flags);
	val_r31 = miiStationRead(phyAddr, 31);// page to L0
	miiStationWrite(phyAddr, 31, pageAddr);
	miiStationWrite(phyAddr, reg_num, reg_data);
	if(val_r31 != pageAddr) {// restore page if necessary
		miiStationWrite(phyAddr, 31, val_r31);
	}
	spin_unlock_irqrestore(&smi_lock, flags);
}

// write Local Reg
void tcPhyWriteLReg(unsigned char port_num, unsigned char page_num, unsigned char reg_num, unsigned int reg_data)
{
	unsigned int val_r31;
	unsigned int phyAddr = port_num;
	unsigned int pageAddr = (page_num<<12)+0x8000;
	unsigned long flags;
	
	spin_lock_irqsave(&smi_lock, flags);
	val_r31 = miiStationRead(phyAddr, 31);// page to L0
	miiStationWrite(phyAddr, 31, pageAddr);
	miiStationWrite(phyAddr, reg_num, reg_data);
	if(val_r31 != pageAddr) {// restore page if necessary
		miiStationWrite(phyAddr, 31, val_r31);
	}
	spin_unlock_irqrestore(&smi_lock, flags);
}

struct swic_reg tc2206_swic_reg[]= {
	{3, 22,  0, 0x06f9},/*P4MECR,Enable RGMII Mode*/
	{3, 22,  1, 0x07f8},/*P5MECR*/
	{3, 21,  5, 0x068c},/*HIGMPSCR*/
	{3, 20, 13, 0x0020},/*ATCR*/
	{3, 21, 17, 0x010d},/*SCR1*/
	{3, 21, 18, 0x07fc},/*P0MCR*/
	{3, 21, 19, 0x07fc},/*P1MCR*/
	{3, 21, 20, 0x07fc},/*P2MCR*/
	{3, 21, 21, 0x07fc},/*P3MCR*/
	{3, 21, 22, 0x0ffc},/*P4MCR*/
	{7, 21, 23, 0x5ffc},/*P5MCR,Let CPU Port (Port 5) fixed on Link UP & 100 Full-Duplex Speed*/   
	{7, 21, 29, 0x6858},/*FLOW CONTROL THRESHOLD. DECREASE RLEASE THRESHOLD*/
	{3, 20,  2, 0xf20f},/*P0CR*/
	{3, 20,  3, 0xf20f},/*P1CR*/
	{3, 20,  4, 0xf20f},/*P2CR*/
	{3, 20,  5, 0xf20f},/*P3CR*/
	{3, 20,  6, 0xf20f},/*P4CR*/
	{3, 20,  7, 0xf20f},/*P5CR*/
	#if defined(TCSUPPORT_ETH4_WAN_PORT)
	{4, 20, 17, 0x4001}, 
	{4, 20, 18, 0x4002},
	{4, 20, 19, 0x4003},
	{4, 20, 20, 0x4004},
	#endif
	{3, 21, 31, 0x0040},/*MCMBTCR*/
	{7, 24, 22, 0x4000},/*Enable the secondary L2 mac table to workaround thatSA=0.0.0.0 will be used entry 2049 entry*/
	{3, 30,  8, 0x7777},/*RXCDC, must to be set up*/
	{3, 30, 13, 0x0000},/*WDTCR,Disable the watchdog timer*/
	{7, 21,  4, 0x000d},/*MLDPCR, MLD(IPv6 packet) forward to port_dft_mcas*/
	{7, 21, 16, 0x0010},/*SCR0,Disable illegal DA/SA check operation*/
	{0,  0,  0, 0x0000}
};

struct cfg_data_s {
    unsigned int reg_num;
    unsigned int val;
};

struct tc2105mj_cfg_data_s {
    struct cfg_data_s gdata[TC2105MJ_PHY_INIT_GDATA_LEN];
    struct cfg_data_s ldata[TC2105MJ_PHY_INIT_LDATA_LEN];
	struct cfg_data_s perpdata[TC2105MJ_PHY_INIT_PERPDATA_LEN]; //per port register setting
};

static const struct tc2105mj_cfg_data_s tc2105mj_cfg = {
	{	{31,0x4000}, {16,0xD4CC}, {17,0x7444}, {19,0x0112}, {22,0x10cf}, 
    	{26,0x0777}, {31,0x2000}, {21,0x0655}, {22,0x0fd3}, {23,0x003d}, 
    	{24,0x096e}, {25,0x0fed}, {26,0x0fc4}, {31,0x1000}, {17,0xe7f8}
    },
	{	{31,0x9000}},
	{	{31,0x8000}}
};

void tc2105mjLRCfgLoad(unsigned char port_num)
{
	int i;
	unsigned short tc2105mj_L2R16[TC2105MJ_PORTNUM] = {0x0e0e, 0x0c0c, 0x0f0f, 0x1010, 0x0909};

	
	for(i = 0; i < TC2105MJ_PHY_INIT_LDATA_LEN; i++){
		miiStationWrite(port_num, tc2105mj_cfg.ldata[i].reg_num, tc2105mj_cfg.ldata[i].val);
	}
	tcPhyWriteLReg(port_num, 2, 16, tc2105mj_L2R16[port_num]);
	tcPhyWriteLReg(port_num, 2, 17, 0);// load revision-related settings, tc2105mj_L2R17_A1/A2, 
}

static void tc2105mjCfgTx10AmpSave(void)
{
	const unsigned short tc2105mj_G0R22_Tx10AmpSave_ON  = 0x0264;
	const unsigned short tc2105mj_G0R22_Tx10AmpSave_OFF = 0x006F;
	unsigned short phyAddr = mac_p->enetPhyAddr;
	unsigned char cfg_Tx10AmpSave_flag = 1;
	
	if(cfg_Tx10AmpSave_flag == 1){ // enable
		tcPhyWriteGReg(phyAddr,0,22,tc2105mj_G0R22_Tx10AmpSave_ON);
	} else { // disable
		tcPhyWriteGReg(phyAddr,0,22,tc2105mj_G0R22_Tx10AmpSave_OFF);
	}
}

void tc2105mjCfgLoad(void)
{
	int pn,i;
	unsigned short phyAddr, rev;
	const unsigned int tc2105mj_G4R21 = 0x7160;
	const unsigned int tc2105mj_G4R25_A1 = 0x0102;
	const unsigned int tc2105mj_G4R29_A1 = 0x8641;
	const unsigned int tc2105mj_G4R25_A2 = 0x0212;
	const unsigned int tc2105mj_G4R29_A2 = 0x4640;
	
	phyAddr = mac_p->enetPhyAddr;
	for(i = 0; i < TC2105MJ_PHY_INIT_GDATA_LEN; i++){// global registers
		miiStationWrite(phyAddr, tc2105mj_cfg.gdata[i].reg_num, tc2105mj_cfg.gdata[i].val);
	}
	tc2105mjCfgTx10AmpSave(); // local registers 
	for(pn = 0; pn < TC2105MJ_PORTNUM; pn++) {
		tc2105mjLRCfgLoad(pn);
	}
	tcPhyWriteGReg(phyAddr, 4, 21,tc2105mj_G4R21);
	rev = miiStationRead(0, 31) & 0x0F;
	if(rev == 0x00) {// load revision-related settings //rev=A1		
		tcPhyWriteGReg(phyAddr, 4, 25, tc2105mj_G4R25_A1);
		tcPhyWriteGReg(phyAddr, 4, 29, tc2105mj_G4R29_A1);
	} else {//rev=A2
		tcPhyWriteGReg(phyAddr, 4, 25, tc2105mj_G4R25_A2);
		tcPhyWriteGReg(phyAddr, 4, 29, tc2105mj_G4R29_A2);
	}
}

void filedSwicDefVal(void)
{
	int i=0;
	unsigned short ver=0;
	unsigned short val=0;
	
	val = miiStationRead(31, 30);
	ver=(unsigned short)(1 << val);/*Get the TC2206 switch IC model id*/
	/*Let IP175C PING compatible, so let those register to reset default value*/
	for(i = 0; tc2206_swic_reg[i].phyaddr != 0; i++) {
		if(ver & tc2206_swic_reg[i].model_id) {
			miiStationWrite(tc2206_swic_reg[i].phyaddr, tc2206_swic_reg[i].regaddr, tc2206_swic_reg[i].value);
		}
	}
	/*The L2P2/L3P3/L4P4 CPU is not supported Turbo MII mode, we need to disabled it*/
	#if !defined(TCSUPPORT_CT) 
	/*Do not use isTC3162L2P2 definition. It will be true in 3182 or 3262 chip. Enable TMII in TC3162U and TC3182. shnwind 20100221.*/
	val = miiStationRead(22, 1);
	if(isTC3162U || isTC3182 || isRT65168 || isRT63260) {
		val |= 0x2;
		miiStationWrite(22, 1, val);
	}else if( isTC3162L3P3 || isTC3162L4P4) {
		val &= 0xfffd;
	}
	miiStationWrite(22, 1, val);
	#endif
}

int tc2105mj_hw_init(void)
{
	unsigned int pn;
	
	local_irq_disable();
	mac_p->enetPhyAddr = 0;
	for(pn = 0; pn < TC2105MJ_PORTNUM; pn++) {/* Reset All PHYs */
		miiStationWrite(pn, MII_BMCR, BMCR_RESET);
	}
	tc2105mjCfgLoad();
	for(pn = 0; pn < TC2105MJ_PORTNUM; pn++) {
		miiStationWrite(pn, MII_BMCR, (BMCR_ANRESTART | BMCR_ANENABLE));
	}
	mdelay(100);
	if(0x2206 == miiStationRead(31, 31)) {
		filedSwicDefVal();
		printk("%s: TC2206\n", __func__);
	}
	local_irq_enable();
	
	return 0;
}

__IMEM struct sk_buff *tc2105mj_vlan_tag_insert(struct sk_buff *skb, struct net_device *dev)
{
	struct sk_buff *tagskb = NULL;
	unsigned char *vhdr = NULL;
	unsigned int cpu_special_tag_len = VLAN_HLEN;
	unsigned char port_mask = 0;

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
	tagskb->dev = switch_dev[SWITCH_CPU_PORT];
	memset(vhdr, 0x00, VLAN_HLEN);
	*vhdr = htons(VLAN_TPID);/* Add Special Tag for this skb *//* first, the ethernet type */
	#ifdef WAN2LAN
	if(skb->mark & SKBUF_COPYTOLAN) {
		port_mask = masko;
	} else
	#endif
	{
		port_mask = (1 << dev->base_addr);
	}
	*(vhdr + 1) = htons(port_mask);/* now, the TCI *//* VLAN_BASE_ID */

out:
	return tagskb;
}

__IMEM int tc2105mj_vlan_tag_remove(struct sk_buff *skb)
{
	int ret_val = -EINVAL;
	unsigned char port = 0;
	unsigned char *src;
	unsigned int pull_len = ETH_ALEN * 2;
	unsigned int cpu_special_tag_len = VLAN_HLEN;
	unsigned char *vlan_tag;
	struct net_device_stats *stats;
	#ifdef CONFIG_TCSUPPORT_ETH4_WAN_PORT
	unsigned char port_max = SWITCH_PORT4;
	#else
	unsigned char port_max = SWITCH_PORT3;
	#endif

	vlan_tag = skb->data + pull_len;
	if(VLAN_TPID == __constant_ntohs(*vlan_tag)) {/* VLAN_BASE_ID */
		port = (__constant_ntohs(*(vlan_tag + 1)));/* Find out which port the packet receive from */
		if(port <= port_max) {
			skb->dev = switch_dev[port];
			stats = (struct net_device_stats *)netdev_priv(skb->dev);
			stats->rx_packets++;
			stats->rx_bytes += skb->len;
			src = skb->data;
			skb_pull(skb, cpu_special_tag_len);
			memmove(skb->data, src, pull_len);
		} else {
//			etdebug("Unknown port %d, using default!\n", port);
			skb->dev = switch_dev[SWITCH_CPU_PORT];
		}
		ret_val = 0;
	} else {
//		etdebug("VLAN_ID=0x%04X\n", __constant_ntohs(*(vlan_tag + 1)));
		skb->dev = switch_dev[SWITCH_CPU_PORT];
	}
	
	return ret_val;
}


void PHY_power_ops(unsigned int port, int optcode)
{
	unsigned int value;
	
	if(port < SWITCH_CPU_PORT) {
		value = tcPhyReadGReg(port, 0, 0);
		if(0 == optcode) {
			value |= BMCR_PDOWN; /* Power down the PHY */
		} else {
			value &= ~BMCR_PDOWN;
			value |= BMCR_ANRESTART; /* Restart auto-negotiation process */
		}
		etdebug("Turn %s PHY power on port %d\n", (optcode)? "ON" : "DOWN", port + 1);
		tcPhyWriteGReg(port, 0, 0, value);
		if(0 != optcode) { /* Waiting for the link to be ready */
			mdelay(100);
		}
	}
}


void PHY_led_ops(unsigned int port, unsigned int optcode)
{
	unsigned int value;
	unsigned int pins;

	etdebug("Turn %s PHY LED on port %d\n", (LED_BOOT_OFF == optcode) ? "OFF" : "ON", port + 1);
	if(port < 5) {/* GPIO Shared Pin */
		pins = (1 << (port + 9));
		value = REG32(CR_GPIO_SHARE);
		if(optcode > LED_BOOT_ON) {
			value |= pins;
		} else {
			value &= ~pins;/* Set as GPIO pin */
		}
		REGWRITE32(CR_GPIO_SHARE, value);
	}
}
#if 0
int tc2105mj_port_mode_control(unsigned int port, unsigned int mode)
{
	unsigned int value;
	const unsigned int modemask = 0xE000;/* Bit15~Bit13 */
	int ret_val = -EINVAL;

	if((port < SWITCH_CPU_PORT) && (mode < 5)) {
		rtl8367b_getAsicPHYReg(port, GIGA_BASET_CTRL_REG, &value);
		value &= ~(modemask);
		value |= (mode << 13);
		rtl8367b_setAsicPHYReg(port, GIGA_BASET_CTRL_REG, value);
		ret_val = 0;
	} else if(9 == mode) {/* Reset switch */
		del_timer_sync(&link_state_timer);
		disable_irq(GPIO_INT);
		tc2105mj_switch_hw_reset_kernel();
		ret_val = tc2105mj_chip_init_kernel();
		if(0 != ret_val) {
			printk("%s: Fail to init chip!\n", __func__);
		}		
		ret_val = tc2105mj_hw_init();
		if(0 == ret_val) {
			ret_val = tc2105mj_interrupt_init();
			if(0 != ret_val) {
				ret_val = mod_timer(&link_state_timer, jiffies + msecs_to_jiffies(500));
			}
		}
		enable_irq(GPIO_INT);
		for(port = 0; port < SWITCH_CPU_PORT; port++) {
			PHY_power_ops(port, ENABLED);
		}
	} else if(8 == mode) {
		ret_val = del_timer_sync(&link_state_timer);
		if(0 != port) {/* Restart link state timer */
			ret_val = mod_timer(&link_state_timer, jiffies + msecs_to_jiffies(500));
		}
	}
	return ret_val;
}
#endif



/*=========================================================================
 Function Description: Register operation for switch
 Data Accessed:
 Data Updated:
 Input:				reg -r reg_addr length
                    reg -w reg_addr data
 Output:			register data
 Return:			always return 0
 Others:
=========================================================================*/
ssize_t switch_register_operation( struct file *filp, const char __user *buff, unsigned long len, void *data)
{
	char temp[64] = {'\0'};
	char *addrp = &temp[0];
	char *valp;
	const unsigned short page_reg = 31;
	unsigned int addr = 0;
	unsigned int value = 0;
	int i;

	if(len > sizeof(temp)) {
		goto err1;
	}
	if(0 != copy_from_user(temp, buff, len)) {
		goto err1;
	}
	if(0 == memcmp(temp, "reg -w", 6)) {
		addrp += 7;
		addr = simple_strtoul(addrp, &valp, 16);
		if((0 == addr) || (0 != (addr % 4)) || (valp == addrp)) {
			goto err1;
		}
		value = simple_strtoul(valp + 1, NULL, 16);
		REGWRITE32(addr, value);
		printk("Addr:%#x  Value:%08X\n", addr, REG32(addr));
	} else if(0 == memcmp(temp, "reg -r", 6)) {
		addrp += 7;
		addr = simple_strtoul(addrp, &valp, 16);/* PageNo. */
		if((addr > 0xB000) || (valp == addrp)) {
			goto err1;
		}
		value = simple_strtoul(valp + 1, NULL, 10);/* PHYaddr */
		if(value > 31) {
			goto err1;
		}
		local_bh_disable();
		miiStationWrite(value, page_reg, addr);/* Select PageNo. first */
		printk("PageNo. = 0x%04X, PHYaddr = %d", addr, value);
		for(i = 0; i < 32; i++) {
			if(0 == (i % 4)) {
				printk("\n");
			}
			printk("[reg=%02d val=%04X]", i, miiStationRead(value, i));
		}
		local_bh_enable();
		printk("\n");		
	} else {
		goto err1;
	}
	goto err0;

err1:	
	printk("Input Error!\n");
err0:
	return len;
}


int tc2105mj_interrupt_init(void)
{
	int ret_val = -EOPNOTSUPP;
	unsigned int mask;

	mask = REG32(GSW_CFG_IMR);
	mask &= ~(0x7F); /* Set 0 to enable port interrupt */
	REGWRITE32(GSW_CFG_IMR, mask);
	#if 0
	mask = REG32(GSW_GPINT_EN);
	mask |= 0x0F;
	REGWRITE32(GSW_GPINT_EN, mask);
	#endif
	ret_val = 0;
	
	return ret_val;
}

void tc2105mj_get_link_state(void)
{
	struct phy_link_state st;
	int port;
	unsigned int reg;

	for(port = 0; port < SWITCH_CPU_PORT; port++) {
		reg = REG32(GSW_PMSR(port));
		st.pLinkStatus = (reg & (1 << 0)); /* 0:DOWN, 1:UP  */
		if(phystate[port].pLinkStatus != st.pLinkStatus) {
			if(1 == st.pLinkStatus) {
				netif_carrier_on(switch_dev[port]);
				printk("Port %d Link UP!\n", port + 1);
			} else {
				netif_carrier_off(switch_dev[port]);
				printk("Port %d Link DOWN!\n", port + 1);
			}
		}
		st.pDuplex = (reg & (1 << 1)); /* 0:Half Duplex, 1:Full Duplex */
		st.pSpeed = (reg & (3 << 2)); /* 0:10M, 1:100M, 2:1000M, 3:Invalid */
		phystate[port] = st;
	}
}


static void tc2105mj_link_state_timer_handler(unsigned long data)
{	
	tc2105mj_get_link_state();
	mod_timer(&link_state_timer, jiffies + msecs_to_jiffies(500));
}

void tc2105mj_interrupt_dsr_link(unsigned long v)
{
	tc2105mj_get_link_state();
}

static irqreturn_t tc2105mj_isr(int irq, void *dev_id)
{
	unsigned int ims;

	ims = REG32(GSW_CFG_ISR);
	REGWRITE32(GSW_CFG_ISR, ims);
	if(0 != (ims & 0x7F)) {
		tasklet_schedule(&link_dsr_tasklet);
	}
	#if 0
	ims = REG32(GSW_GPINT_STS);
	REGWRITE32(GSW_GPINT_STS, ims);
	#endif
	
	return IRQ_HANDLED;
}
static int tc2105mj_open(struct net_device *dev)/* Starting up the ethernet device */
{
	int ret_val = 0;

	netif_carrier_off(dev);
	PHY_power_ops(dev->base_addr, 1);
	PHY_led_ops(dev->base_addr, 3);
	netif_start_queue(dev);

  	return ret_val;
}


static int tc2105mj_close(struct net_device *dev)/* Stopping the ethernet device */
{
  	netif_stop_queue(dev);
	PHY_power_ops(dev->base_addr, 0);
  	return 0;
}

__IMEM netdev_tx_t tc2105mj_tx(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats;
	struct net_device *rootdev = switch_dev[SWITCH_CPU_PORT];
	
	netdev_tx_t ret_val = NETDEV_TX_OK;

	stats = (struct net_device_stats *)netdev_priv(dev);
	
    if(unlikely(netif_queue_stopped(rootdev))) {
		dev_kfree_skb_any(skb);
		stats->tx_dropped++;
		printk("%s: Root device is stopped!\n", __func__);
		goto out;
   	}
	ret_val = tc3262_gmac_tx(skb, dev);

out:	
	return ret_val;
}


static int tc2105mj_set_macaddr(struct net_device *dev, void *p)/* Setting customized mac address */
{
	struct sockaddr *addr = p;
	int ret_val = -EIO;
	#ifdef CONFIG_RTL8365_CPU_TAG
	struct rtk_mac_s *pMac;
	struct rtk_l2_ucastAddr_s pL2_data;
	#endif

	/* Check if given address is valid ethernet MAC address */
  	if(!is_valid_ether_addr(addr->sa_data)) {
    	goto out;
	}
	#ifdef CONFIG_RTL8365_CPU_TAG
	if(is_valid_ether_addr(switch_dev[SWITCH_CPU_PORT]->dev_addr)) {
		pMac = (struct rtk_mac_s *)(switch_dev[SWITCH_CPU_PORT]->dev_addr);
		memset(&pL2_data, 0x00, sizeof(struct rtk_l2_ucastAddr_s));
		memcpy(pL2_data.mac.octet, pMac->octet, ETHER_ADDR_LEN);
		pL2_data.port = 6;
		pL2_data.is_static = 1;
		ret_val = rtk_l2_addr_del(pMac, &pL2_data);
	}
	#endif
  	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);/* Save the customize mac address */
	tc3262_gmac_set_macaddr(switch_dev[SWITCH_CPU_PORT], addr);
	#ifdef CONFIG_RTL8365_CPU_TAG
	pMac = (struct rtk_mac_s *)(dev->dev_addr);	
	memset(&pL2_data, 0x00, sizeof(struct rtk_l2_ucastAddr_s));
	memcpy(pL2_data.mac.octet, pMac->octet, ETHER_ADDR_LEN);
	pL2_data.port = 6;
	pL2_data.is_static = 1;
	ret_val = rtk_l2_addr_add(pMac, &pL2_data);
	if(0 != ret_val) {
		printk("Fail to add new MAC entry!\n");
		goto out;
	}
	#endif
out:
	return ret_val;
}

static struct net_device_stats *tc2105mj_get_stats(struct net_device *dev)
{
	return (struct net_device_stats *)netdev_priv(dev);
}

static const struct net_device_ops switch_netdev_ops = {
	.ndo_open				= tc2105mj_open,
	.ndo_stop 				= tc2105mj_close,
	.ndo_start_xmit			= tc2105mj_tx,
	.ndo_get_stats			= tc2105mj_get_stats,
	.ndo_set_mac_address 	= tc2105mj_set_macaddr,
};



int __init tc2105mj_switch_init(void)
{
	int i, port;
	struct net_device *dev = NULL;
	struct proc_dir_entry *entry;
	int ret_val = -ENOMEM;

	smi_init();
	tasklet_init(&link_dsr_tasklet, tc2105mj_interrupt_dsr_link, (unsigned long)switch_dev[SWITCH_CPU_PORT]);
  	setup_timer(&link_state_timer, tc2105mj_link_state_timer_handler, (unsigned long)switch_dev[SWITCH_CPU_PORT]);

	ret_val = tc2105mj_hw_init();
	if(0 != ret_val) {
		goto out;
	}
	for(port = 0; port < SWITCH_CPU_PORT; port++) {
        dev = alloc_etherdev(sizeof(struct net_device_stats));
		if(NULL == dev) {
            printk("%s: No memory!\n", __func__);
            for(i = port; i > 0; i--) {
				unregister_netdev(switch_dev[i]);
				free_netdev(switch_dev[i]);
			}
            break;
        }
        memset(netdev_priv(dev), 0x00, sizeof(struct net_device_stats));
        sprintf(dev->name, "eth0.%d", port + 1);
        dev->netdev_ops = &switch_netdev_ops;
        dev->base_addr = port;
		dev->irq = MAC1_INT;
        memcpy(dev->dev_addr, switch_dev[SWITCH_CPU_PORT]->dev_addr, ETH_ALEN);
        switch_dev[port] = dev;
        ret_val = register_netdev(dev);
		if(0 != ret_val) {
			printk("Fail to register device %s\n", dev->name);
            free_netdev(dev);
            for(i = port; i > 0; i--) {
				unregister_netdev(switch_dev[i]);
				free_netdev(switch_dev[i]);
			}
            break;
        }

        /*
         * TBS_TAG:add by pengyao 20130311
         * Desc: ether net interface for tbs_nfp
         */
        dev->priv_flags |= IFF_ETH;
        /*
         * TBS_TAG:END
         */

   	}
	memset(phystate, 0x00, sizeof(phystate));
	#if 0
	create_proc_read_entry(LOOKUP_PROC_ENTRY, 0, NULL, lookup_table_entry_read, NULL);
	entry = create_proc_entry(MIB_COUNTER_ENTRY, 0644, NULL);
	if(NULL != entry) {
		entry->read_proc = mib_entry_read;
		entry->write_proc = mib_entry_write;
	}
	entry = create_proc_entry(SWITCH_MODE_ENTRY, 0644, NULL);
	if(NULL != entry) {
		entry->read_proc = tc2105mj_mode_proc_read;
		entry->write_proc = tc2105mj_mode_proc_write;
	}
	#endif
	entry = create_proc_entry(SWITCH_REG_ENTRY, 0644, NULL);
	if(NULL != entry) {
		entry->write_proc = switch_register_operation;
	}
	ret_val = tc2105mj_interrupt_init();
	printk("Switch interrupt ");
	if(0 != ret_val) {/* Schedule timer for monitoring link status */
		printk("NOT support!\n");
		ret_val = mod_timer(&link_state_timer, jiffies + msecs_to_jiffies(500));
	} else {
		ret_val = request_irq(dev->irq, tc2105mj_isr, IRQF_DISABLED, "Switch", dev);
		if(0 != ret_val) {
			printk("%s: Fail to register irq %d, ret=%d!\n", __func__, dev->irq, ret_val);
			goto out;
		} else {
			printk("Enabled, IRQ=%d\n", dev->irq);
		}
	}	

out:
	return ret_val;
}

void __exit tc2105mj_switch_exit(void)
{
	int port;

	synchronize_net();
  	/* Kill timer */
  	del_timer_sync(&link_state_timer);
	tasklet_kill(&link_dsr_tasklet);
	free_irq(MAC1_INT, NULL);
	#if 0	
	remove_proc_entry(LOOKUP_PROC_ENTRY, 0);
	remove_proc_entry(MIB_COUNTER_ENTRY, 0);
	remove_proc_entry(SWITCH_MODE_ENTRY, 0);
	#endif	
	remove_proc_entry(SWITCH_REG_ENTRY, 0);

	for(port = 0; port < SWITCH_CPU_PORT; port++) {
		if(NULL != switch_dev[port]) {
            unregister_netdev(switch_dev[port]);
            free_netdev(switch_dev[port]);
            switch_dev[port] = NULL;
        }
    }
}

EXPORT_SYMBOL_GPL(PHY_led_ops);



