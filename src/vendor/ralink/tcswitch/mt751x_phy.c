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
#include "mt751x_phy.h"

static struct macAdapter g_mac;
static struct macAdapter *mac_p;

static void miiStationWrite(macAdapter_t *, u32, u32);
static u32 miiStationRead(macAdapter_t *, u32);
static void mtEMiiRegWrite(u32, u32, u32, u32);
static u32 mtEMiiRegRead(u32, u32, u32);
static void mtMiiRegWrite(u32, u32, u32);
static u32 mtMiiRegRead(u8,u8);
static int tcPhyPortInit(u8);
static void refillPhyDefVal(u32 enetPhyAddr, u32 phyReg, u32 miiData);

static u8 current_idx = 0; // default 0, Do NOT change

static u8 cfg_Tx10AmpSave_flag = 1; // default enable


static const u32 mt7530_page_sel_addr=31;

static const u32 mt7510FE_page_sel_addr=31;
static const u16 mt7510FE_G0R22_Tx10AmpSave_ON  = 0x0264;
static const u16 mt7510FE_G0R22_Tx10AmpSave_OFF = 0x006F;

static const u32 mt7510Ge_page_sel_addr=31;

static const u16 mt7510FE_L2R16[MT7510FE_PORTNUM]
		= {0x0606, 0x0606, 0x0606, 0x0606};
static const u16 mt7510FE_L2R17_A1[MT7510FE_PORTNUM]
		= {0x0002, 0x0002, 0x0002, 0x0002};
static const u16 mt7510FE_L3R17_A1[MT7510FE_PORTNUM]
		= {0x0000, 0x0000, 0x0000, 0x0000};

static phyDeviceList_t phyDeviceList[] = {
	{0x0013, "LXT972"},
	{0x0022, "AC101L"},
	{0x0243, "IP101"},
	{0x8201, "RTL8201"},
	{0x001c, "RTL8212"},
	{0x03a2, "TC2031"},
	/* {0x0007, "MT7530"},    // for E1 */
	{0x03a2, "MT7530"},
};

static u8 tcPhyFlag = 0;
static u8 fgMT7530_INT = 0;
static u8 fgMT7510Ge_INT = 0;
static u8 tcPhyVer = 99; // default: unknown PhyVer
static u8 tcPhyPortNum = 1; // set value in tcPhyVerLookUp
static u8 swicVendor = 0;
static int eco_rev = 0x00;

static u8 tcphy_link_state[TCPHY_PORTNUM];

static DEFINE_SPINLOCK(phy_lock);

static mt7530_cfg_data_t mt7530_cfg[MT7530_PHY_INIT_SET_NUM]={
	{
		{"E3.0"},
		{
		},
		{
		}
	},
};

static mt7530_cfg_cl45data_t mt7530_cfg_cl45[MT7530_PHY_INIT_CL45_SET_NUM]={
	{
		{
		},
		{
		},
		{
		}
	},
};

static mt7530_cfg_trdata_t mt7530_cfg_tr[MT7530_PHY_INIT_TR_SET_NUM]={
	{
		{
		},
		{
		}
	},
};

static mt7510FE_cfg_data_t mt7510FE_cfg[MT7510FE_PHY_INIT_SET_NUM]={
	{
		{"7510FE_2.0"},
		{
			{31,0x2000},{22,0x0092},{23,0x007d},{24,0x09b0},{25,0x0f6e},
			{26,0x0fc4}

		},
		{
			{31,0x9000},{31,0xb000},{17,0x0000}//disable EEE
		},
		{
			{31,0x8000}
		}
	},
};

static mt7510Ge_cfg_data_t mt7510Ge_cfg[MT7510Ge_PHY_INIT_SET_NUM]={

	{{"Ge-1.0"},
		{
		},
		{
		}
	},
};

static mt7510Ge_cfg_cl45data_t mt7510Ge_cfg_cl45[MT7510Ge_PHY_INIT_CL45_SET_NUM]={
	{//globle data
		{
			{0x1f, 0x0416, 0x0047}, // mcc_acc_ref_th setting //CML_20130327
			{0x1f, 0x027c, 0x0c0c} // for 10BASE-T amplitude //CML_20130327
		},
		{
			{0x7,0x003c,0x0000},  //disable EEE mode
			{0x1e,0x0012,0x7210},
			{0x1e,0x0045,0x0202},
			{0x1e,0x0046,0x0404}// for 100BASE-T TX //CML_20130401

		},
		//per-port data
		{
		}
	},
};

static mt7510Ge_cfg_trdata_t mt7510Ge_cfg_tr[MT7510Ge_PHY_INIT_TR_SET_NUM]={
	{//local data
		{
		},
		{
		}
	},
};

static void tcMiiExtStationWrite_ext(u32 portAddr, u32 devAddr,
		u32 regAddr, u32 miiData)
{
}

static u32 tcMiiExtStationRead_ext(u32 portAddr, u32 devAddr,
		u32 regAddr, u32 op)
{
	return 0;
}

static void tcMiiStationWrite_ext(u32 enetPhyAddr, u32 phyReg,
		u32 miiData)
{
}

static u32 tcMiiStationRead_ext(u32 enetPhyAddr, u32 phyReg)
{
	return 0;
}

u32 tcMiiStationRead(u32 enetPhyAddr, u32 phyReg)
{
	u32 reg;
	u32 cnt=10000;
	u32 GSW_CFG_PIAC_addr;

	GSW_CFG_PIAC_addr = (isMT7530) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;

	spin_lock_bh(&phy_lock);
	do {
		reg=read_reg_word (GSW_CFG_PIAC_addr);
		cnt--;
	} while ((reg & PHY_ACS_ST) && (cnt != 0));

	reg = PHY_ACS_ST
		| (MDIO_ST_START << MDIO_ST_SHIFT)
		| (MDIO_CMD_READ<<MDIO_CMD_SHIFT)
		| (enetPhyAddr << MDIO_PHY_ADDR_SHIFT)
		| (phyReg << MDIO_REG_ADDR_SHIFT);
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

void tcMiiStationWrite(u32 enetPhyAddr, u32 phyReg, u32 miiData)
{
	u32 reg;
	u32 cnt=10000;
	u32 GSW_CFG_PIAC_addr;

	GSW_CFG_PIAC_addr = (isMT7530) ? (GSW_CFG_BASE + 0x001C) : GSW_CFG_PIAC;

	spin_lock_bh(&phy_lock);
	do {
		reg=read_reg_word (GSW_CFG_PIAC_addr);
		cnt--;
	} while ((reg & PHY_ACS_ST) && (cnt != 0));

	reg = PHY_ACS_ST
		| (MDIO_ST_START << MDIO_ST_SHIFT)
		| (MDIO_CMD_WRITE<<MDIO_CMD_SHIFT)
		| (enetPhyAddr << MDIO_PHY_ADDR_SHIFT)
		| (phyReg << MDIO_REG_ADDR_SHIFT)
		| (miiData & MDIO_RW_DATA);
	write_reg_word (GSW_CFG_PIAC_addr, reg);

	cnt = 10000;
	do {
		//pause(1);
		reg = read_reg_word(GSW_CFG_PIAC_addr);
		cnt--;
	} while ((reg & PHY_ACS_ST) && (cnt != 0));
	spin_unlock_bh(&phy_lock);
	if (cnt==0)
		printk("EER: tcMiiStationWrite timeout!\n");
	/*TC2206 switch IC can't be direct to do PHY reset, we must
	 * avoid ESD software patch be trigger.
	 */
	refillPhyDefVal(enetPhyAddr, phyReg, miiData);
}

static u32 tcMiiExtStationRead_CL22(u32 port_num, u32 dev_num, u32 reg_num)
{
	const u16 MMD_Control_register=0xD;
	const u16 MMD_addr_data_register=0xE;
	const u16 page_reg=31;

	u32 value = 0;

	mtMiiRegWrite(port_num, page_reg, 0x00); //switch to main page
	mtMiiRegWrite(port_num, MMD_Control_register, (0<<14)+dev_num);
	mtMiiRegWrite(port_num, MMD_addr_data_register, reg_num);
	mtMiiRegWrite(port_num, MMD_Control_register, (1<<14)+dev_num);
	value = mtMiiRegRead(port_num, MMD_addr_data_register);

	return value;
}

static void tcMiiExtStationWrite_CL22(u32 port_num, u32 dev_num, u32 reg_num,
		u32 reg_data)
{
	const u16 MMD_Control_register=0xD;
	const u16 MMD_addr_data_register=0xE;
	const u16 page_reg=31;

	mtMiiRegWrite(port_num, page_reg, 0x00); //switch to main page
	mtMiiRegWrite(port_num, MMD_Control_register, (0<<14)+dev_num);
	mtMiiRegWrite(port_num, MMD_addr_data_register, reg_num);
	mtMiiRegWrite(port_num, MMD_Control_register, (1<<14)+dev_num);
	mtMiiRegWrite(port_num, MMD_addr_data_register, reg_data);
}

static u32 mtEMiiRegRead(u32 port_num, u32 dev_num, u32 reg_num)
{
	if((tcPhyVer == mtPhyVer_7530)&&(fgMT7530_INT==0))
		return (tcMiiExtStationRead_ext(port_num, dev_num, reg_num,
					NORMAL_READ));
	else
		return (tcMiiExtStationRead_CL22( port_num, dev_num, reg_num));
}


static void mtEMiiRegWrite(u32 port_num, u32 dev_num, u32 reg_num, u32 reg_data)
{
	if((tcPhyVer == mtPhyVer_7530)&&(fgMT7530_INT==0))
		tcMiiExtStationWrite_ext( port_num, dev_num, reg_num, reg_data);
	else
		tcMiiExtStationWrite_CL22( port_num, dev_num, reg_num, reg_data);
}



static void miiStationWrite(macAdapter_t *mac_p, u32 phyReg, u32 miiData)
{
	tcMiiStationWrite(mac_p->enetPhyAddr, phyReg, miiData);
}

static u32 miiStationRead(macAdapter_t *mac_p, u32 phyReg)
{
	return tcMiiStationRead(mac_p->enetPhyAddr, phyReg);
}

static void mtMiiRegWrite(u32 port_num, u32 reg_num, u32 reg_data)
{
	if((tcPhyVer == mtPhyVer_7530)&&(fgMT7530_INT==0))
		tcMiiStationWrite_ext(port_num,reg_num, reg_data);
	else
		tcMiiStationWrite(port_num, reg_num, reg_data);
}

static u32 mtMiiRegRead(u8 port_num,u8 reg_num)
{
	if((tcPhyVer == mtPhyVer_7530)&&(fgMT7530_INT==0))
		return(tcMiiStationRead_ext( port_num,reg_num));
	else
		return(tcMiiStationRead(port_num, reg_num));
}


// write Local Reg
static void tcPhyWriteLReg(u8 port_num,u8 page_num,u8 reg_num,u32 reg_data)
{
	u32 val_r31;
	u32 phyAddr = mac_p->enetPhyAddr + port_num;
	u32 pageAddr = (page_num<<12)+0x8000;

	val_r31 = tcMiiStationRead(phyAddr, 31);  // remember last page
	// set page if necessary
	if (val_r31 != pageAddr) {
		tcMiiStationWrite(phyAddr, 31, pageAddr); // switch to page Lx
	}
	tcMiiStationWrite(phyAddr, reg_num, reg_data);
	// restore page if necessary
	if (val_r31 != pageAddr) {
		tcMiiStationWrite(phyAddr, 31, val_r31);
	}
}

static int mtPhyMiiWrite_TrDbg(u8 phyaddr, char *type, u32 data_addr,
		u32 value, u8 ch_num)
{
	const u16 page_reg=31;
	const u32 Token_Ring_debug_reg=0x52B5;
	const u32 Token_Ring_Control_reg=0x10;
	const u32 Token_Ring_Low_data_reg=0x11;
	const u32 Token_Ring_High_data_reg=0x12;

	u16 ch_addr=0;
	u32 node_addr=0;
	u32 value_high=0;
	u32 value_low=0;

	if (stricmp(type, "DSPF") == 0){// DSP Filter Debug Node
		ch_addr=0x02;
		node_addr=0x0D;
	}
	else if (stricmp(type, "PMA") == 0){ // PMA Debug Node
		ch_addr=0x01;
		node_addr=0x0F;
	}
	else if (stricmp(type, "TR") == 0){ // Timing Recovery	Debug Node
		ch_addr=0x01;
		node_addr=0x0D;
	}
	else if (stricmp(type, "PCS") == 0){ // R1000PCS Debug Node
		ch_addr=0x02;
		node_addr=0x0F;
	}
	else if (stricmp(type, "FFE") == 0){ // FFE Debug Node
		ch_addr=ch_num;
		node_addr=0x04;
	}
	else if (stricmp(type, "EC") == 0){ // ECC Debug Node
		ch_addr=ch_num;
		node_addr=0x00;
	}
	else if (stricmp(type, "ECT") == 0){ // EC/Tail Debug Node
		ch_addr=ch_num;
		node_addr=0x01;
	}
	else if (stricmp(type, "NC") == 0){ // EC/NC Debug Node
		ch_addr=ch_num;
		node_addr=0x01;
	}
	else if (stricmp(type, "DFEDC") == 0){ // DFETail/DC Debug Node
		ch_addr=ch_num;
		node_addr=0x05;
	}
	else if (stricmp(type, "DEC") == 0){ // R1000DEC Debug Node
		ch_addr=0x00; node_addr=0x07;
	}
	else if (stricmp(type, "CRC") == 0){ // R1000CRC Debug Node
		ch_addr=ch_num;
		node_addr=0x06;
	}
	else if (stricmp(type, "AN") == 0){ // Autoneg Debug Node
		ch_addr=0x00; node_addr=0x0F;
	}
	else if (stricmp(type, "CMI") == 0){ // CMI Debug Node
		ch_addr=0x03; node_addr=0x0F;
	}
	else if (stricmp(type, "SUPV") == 0){ // SUPV PHY  Debug Node
		ch_addr=0x00; node_addr=0x0D;
	}

	data_addr=data_addr&0x3F;
	value_high=(0x00FF0000&value)>>16;
	value_low=(0x0000FFFF&value);

	mtMiiRegWrite(phyaddr, page_reg, Token_Ring_debug_reg);
	mtMiiRegWrite(phyaddr, Token_Ring_Low_data_reg, value_low);
	mtMiiRegWrite(phyaddr, Token_Ring_High_data_reg, value_high);
	mtMiiRegWrite(phyaddr, Token_Ring_Control_reg,
			(1<<15)|(0<<13)|(ch_addr<<11)|(node_addr<<7)|(data_addr<<1));
	mtMiiRegWrite(phyaddr, page_reg, 0x00);//V1.11
	return 0;
}

// write Global Reg
static void tcPhyWriteGReg(u8 port_num,u8 page_num,u8 reg_num,u32 reg_data)
{
	u32 val_r31;
	u32 phyAddr = mac_p->enetPhyAddr + port_num;
	u32 pageAddr = (page_num<<12);

	val_r31 = tcMiiStationRead(phyAddr, 31);  // remember last page
	// set page if necessary
	if (val_r31 != pageAddr) {
		tcMiiStationWrite(phyAddr, 31, pageAddr); // switch to page Gx
	}
	tcMiiStationWrite(phyAddr, reg_num, reg_data);
	// restore page if necessary
	if (val_r31 != pageAddr) {
		tcMiiStationWrite(phyAddr, 31, val_r31);
	}
}

static void mt7530GePhyCfgLoad(u8 idx, u8 doPerPort_flag, u8 port_num)
{
	int pn, i;
	u16 phyAddr;
	current_idx = idx;

	// global registers
	phyAddr = port_num; //mac_p->enetPhyAddr;
	for (pn = 0; pn < MTPHY_PORTNUM; pn++) {
		phyAddr = pn; //mac_p->enetPhyAddr + pn;

		for(i = 0; i < MT7530_PHY_INIT_CL45_GDATA_LEN; i++) {
			mtEMiiRegWrite(phyAddr,
					mt7530_cfg_cl45[current_idx].gdata[i].dev_num,
					mt7530_cfg_cl45[current_idx].gdata[i].reg_num,
					mt7530_cfg_cl45[current_idx].gdata[i].val);
		}
		for( i=0; i<MT7530_PHY_INIT_CL45_LDATA_LEN; i++ ){
			mtEMiiRegWrite(phyAddr,
					mt7530_cfg_cl45[current_idx].ldata[i].dev_num,
					mt7530_cfg_cl45[current_idx].ldata[i].reg_num,
					mt7530_cfg_cl45[current_idx].ldata[i].val);
		}
		for(i=0; i<MT7530_PHY_INIT_LDATA_LEN; i++) {
			mtMiiRegWrite(phyAddr, mt7530_cfg[current_idx].ldata[i].reg_num,
					mt7530_cfg[current_idx].ldata[i].val);
		}
		for(i = 0; i < MT7530_PHY_INIT_TR_LDATA_LEN; i++) {
			mtPhyMiiWrite_TrDbg(phyAddr,
					mt7530_cfg_tr[current_idx].ldata[i].reg_typ,
					mt7530_cfg_tr[current_idx].ldata[i].reg_num,
					mt7530_cfg_tr[current_idx].ldata[i].val,0);
		}
	}
}

static void mt7510FECfgTx10AmpSave(void)
{
	u16 phyAddr = mac_p->enetPhyAddr;

	if (cfg_Tx10AmpSave_flag==1){ // enable
		tcPhyWriteGReg(phyAddr,0,22,mt7510FE_G0R22_Tx10AmpSave_ON);
	}
	else { // disable
		tcPhyWriteGReg(phyAddr,0,22,mt7510FE_G0R22_Tx10AmpSave_OFF);
	}
}

static void mt7510FELRCfgLoad(u8 idx, u8 port_num)
{
	int i;
	current_idx = idx;
	for( i=0; i<MT7510FE_PHY_INIT_LDATA_LEN; i++ ){
		tcMiiStationWrite((port_num + mac_p->enetPhyAddr),
				mt7510FE_cfg[current_idx].ldata[i].reg_num,
				mt7510FE_cfg[current_idx].ldata[i].val );
	}

	tcPhyWriteLReg(port_num,2,16,mt7510FE_L2R16[port_num]);
	tcPhyWriteLReg(port_num,2,17,mt7510FE_L2R17_A1[port_num]);
	tcPhyWriteLReg(port_num,3,17,mt7510FE_L3R17_A1[port_num]);
}

// 1. loading default register setting in tcPhyInit()
// 2. changing register setting
static void mt7510FECfgLoad(u8 idx, u8 doPerPort_flag, u8 port_num)
{
	int pn,i;
	u16 phyAddr;
	current_idx = idx;

	// global registers
	phyAddr = mac_p->enetPhyAddr;

	for( i=0; i<MT7510FE_PHY_INIT_GDATA_LEN; i++ ){
		tcMiiStationWrite(phyAddr, mt7510FE_cfg[current_idx].gdata[i].reg_num,
				mt7510FE_cfg[current_idx].gdata[i].val );
	}
	//
	mt7510FECfgTx10AmpSave();

	// local registers
	if (doPerPort_flag == DO_PER_PORT) {
		mt7510FELRCfgLoad(current_idx, port_num);
	} else {
		for( pn=0; pn<MT7510FE_PORTNUM; pn++){
			phyAddr = pn; //mac_p->enetPhyAddr + pn;
			mt7510FELRCfgLoad(current_idx, phyAddr);
		}
	}
	printk("tcphy: CfgLoad %s\r\n",  mt7510FE_cfg[current_idx].name);
}

static void mt7510GePhyCfgLoad(u8 idx)
{
	int pn, i;
	u16 phyAddr;
	current_idx = idx;

	// global registers
	phyAddr = mac_p->enetPhyAddr;//port_num
	for( pn=0; pn<tcPhyPortNum; pn++){

		//pork:phyAddr = pn; //mac_p->enetPhyAddr + pn;
		phyAddr = mac_p->enetPhyAddr + pn;
		for( i=0; i<MT7510Ge_PHY_INIT_CL45_GDATA_LEN; i++ ) {
			mtEMiiRegWrite(phyAddr,
					mt7510Ge_cfg_cl45[current_idx].gdata[i].dev_num,
					mt7510Ge_cfg_cl45[current_idx].gdata[i].reg_num,
					mt7510Ge_cfg_cl45[current_idx].gdata[i].val);
		}
		for( i=0; i<MT7510Ge_PHY_INIT_CL45_LDATA_LEN; i++ ){
			mtEMiiRegWrite(phyAddr,
					mt7510Ge_cfg_cl45[current_idx].ldata[i].dev_num,
					mt7510Ge_cfg_cl45[current_idx].ldata[i].reg_num,
					mt7510Ge_cfg_cl45[current_idx].ldata[i].val);
		}
		for( i=0; i<MT7510Ge_PHY_INIT_LDATA_LEN; i++ ){
			mtMiiRegWrite(phyAddr,
					mt7510Ge_cfg[current_idx].ldata[i].reg_num,
					mt7510Ge_cfg[current_idx].ldata[i].val);
		}
		for( i=0; i<MT7510Ge_PHY_INIT_TR_LDATA_LEN; i++ ){
			mtPhyMiiWrite_TrDbg(phyAddr,
					mt7510Ge_cfg_tr[current_idx].ldata[i].reg_typ,
					mt7510Ge_cfg_tr[current_idx].ldata[i].reg_num,
					mt7510Ge_cfg_tr[current_idx].ldata[i].val,0);
		}

	}

}

static void refillPhyDefVal(u32 enetPhyAddr, u32 phyReg, u32 miiData)
{
	/*
	 * TC2206 switch IC can't be direct to do PHY reset, we must
	 * avoid ESD software patch be trigger.
	 */
	if((phyReg==MII_BMCR) && (miiData& BMCR_RESET)&&(swicVendor==SWIC_TC2206)
			&&((enetPhyAddr==0)||(enetPhyAddr==1)||(enetPhyAddr==2)
				||(enetPhyAddr==3))){
		tcPhyPortInit(enetPhyAddr);
	}
}/*end refillPhyDefVal*/

// tcPhy initial: reset, load default register setting, restat AN
static int tcPhyPortInit(u8 port_num)
{
	u32 reg_value; //CML_20130226_1

	tcMiiStationWrite(port_num, PHY_CONTROL_REG, PHY_RESET);

	switch (tcPhyVer) {
		case tcPhyVer_mt7510FE: //mt7510FE
			mt7510FECfgLoad(0, DO_PER_PORT, port_num);
			break;
		case mtPhyVer_7510Ge: // MT7510Ge
			mt7510GePhyCfgLoad(0);
		default:
			printk("unsupport Phy[%d] now..\n", tcPhyVer);
	}

	reg_value = tcMiiStationRead(port_num, PHY_CONTROL_REG);
	reg_value |= MIIDR_AUTO_NEGOTIATE;
	tcMiiStationWrite(port_num, PHY_CONTROL_REG, reg_value );

	// tcphy_link_state init.
	tcphy_link_state[port_num]=ST_LINK_DOWN;

	return 0;
}

static int tcPhyVerLookUp(macAdapter_t *mac_p)
{
	u32 phyAddr;
	u32 rval;

	phyAddr = 0;
	tcPhyFlag = 1;

	rval = miiStationRead(mac_p, 3); // phy revision id
	if(rval == 0xffff) {
		rval = tcMiiStationRead_ext(phyAddr, 3); // phy revision id
	} else {
		fgMT7530_INT = 1;
	}

	rval &= 0xffff;

	if (rval == 0x940F) {
		tcPhyVer = tcPhyVer_mt7510FE;
		tcPhyPortNum = 4;
		printk("MT7510FE, ");
	} else if (rval == 0x9412) { // CML_20130219, E2 PHY ID
		tcPhyVer = mtPhyVer_7530;
		tcPhyPortNum = 1;
		printk("MT7530E2,Internal check flag: fgMT7530_INT=0x%x \n",
				fgMT7530_INT);
	} else if (rval == 0x9421) {
		tcPhyVer = mtPhyVer_7510Ge;
		tcPhyPortNum = 1;
		printk("MT7510Ge,Internal check flag: fgMT7510Ge_INT=0x%x \n",
				fgMT7510Ge_INT);
	} else {
		printk(KERN_ERR "unknown PHYID: %x, ", rval);
	}

	if (tcPhyVer == mtPhyVer_7530){ //CML_20130226_2
		eco_rev = (mtEMiiRegRead(phyAddr, 0x1f, 0) >> 4);
	} else {
		eco_rev = tcMiiStationRead(phyAddr, 31);
	}
	eco_rev &= (0x0f);

	return 0;
}

static int macPhyLookUp(macAdapter_t *mac_p, u32 companyId)
{
	u32 i;
	u32 phyTypeSupport;

	phyTypeSupport = sizeof(phyDeviceList) / sizeof(phyDeviceList_t);
	for ( i = 0; i < phyTypeSupport; i++ ) {
		if (companyId == phyDeviceList[i].companyId ) {
			if ((companyId == 0x03a2) || (companyId == 0x0007)){
				tcPhyVerLookUp(mac_p);
			}
			return 1;
		}
	}
	return 0;
}

static int macSearchPhyAddr(macAdapter_t *mac_p)
{
	u32 miiReg = 0;

	miiReg = miiStationRead(mac_p, MII_PHYSID1);
	if (miiReg == 0) {
		miiReg = miiStationRead(mac_p, MII_PHYSID2);
	}
	if (macPhyLookUp(mac_p, miiReg)) {
		return 0;
	}

	return -1;
}

static u8 getTcPhyFlag(void)
{
	return tcPhyFlag;
}

static int tcPhyPortNumGet(void)
{
	if(tcPhyVer != 99)
		return tcPhyPortNum;
	else
		return 0;
}

static int tcPhyInit(macAdapter_t * mac_p){
	int i=0;
	u16 start_addr= mac_p->enetPhyAddr;
	u32 reg_value; //CML_20130226_1

	for( i=start_addr; i<start_addr+tcPhyPortNum; i++ ){
		mtMiiRegWrite(i, PHY_CONTROL_REG, PHY_RESET );
	}

	printk(KERN_INFO "--> tcPhyVer = %d\n", tcPhyVer);

	switch (tcPhyVer) {
		case tcPhyVer_mt7510FE: // MT7510FE
			mt7510FECfgLoad(0, DO_4_PORT, 0);
			break;
		case mtPhyVer_7510Ge: // MT7510Ge
			mt7510GePhyCfgLoad(0);
			break;
			//default:
	}

	for( i=start_addr; i<start_addr+tcPhyPortNum; i++ ){
		reg_value = mtMiiRegRead(i, PHY_CONTROL_REG);
		reg_value |= MIIDR_AUTO_NEGOTIATE;
		mtMiiRegWrite(i, PHY_CONTROL_REG, reg_value );
	}

	for(i=0;i<TCPHY_PORTNUM;i++){
		tcphy_link_state[i]=ST_LINK_DOWN;
	}

	return 0;
}

static void macPhyReset(void)
{
	if (getTcPhyFlag()) {
		tcPhyInit(mac_p);
	} else {
		miiStationWrite(mac_p, MII_BMCR, BMCR_RESET);
		pause(10);
		miiStationWrite(mac_p, MII_BMCR, BMCR_ANRESTART | BMCR_ANENABLE);
	}
}

int mt751x_macSetUpPhy(macAdapter_t *pmac)
{
	/*OSBNB00031891: Add for Multi EPHY setting*/
	u32 phy_addr_tmp;

	g_mac = *pmac;
	mac_p = &g_mac;
	phy_addr_tmp = mac_p->enetPhyAddr;

	mac_p->enetPhyAddr = 0;
	for (; mac_p->enetPhyAddr < 32; mac_p->enetPhyAddr++) {
		if(!macSearchPhyAddr(mac_p)) {
			macPhyReset();
			mac_p->enetPhyAddr += (tcPhyPortNumGet()-1);
		}
	}
	mac_p->enetPhyAddr = 0;
	pmac->enetPhyAddr = 0;
	/*OSBNB00031891: Add for Multi EPHY setting over*/
	mdelay(10);

	return 0;
}

// static int g_is_phy_proc_created = -1;
static struct proc_dir_entry *proc_phy_dbg = NULL;
static struct proc_dir_entry *entry_phy_ver = NULL;
static struct proc_dir_entry *entry_phy_sw_ver = NULL;
static struct proc_dir_entry *entry_phy_miir = NULL;
static struct proc_dir_entry *entry_phy_miiw = NULL;

static u16 checked_atoi(char *val)
{
	if (val[0]<'0' || val[0]>'9')
		return (0xffff);
	return (simple_strtoul(val, NULL,10));
}

static u16 getMiiPage(char *page)
{
    // g0,g1,g2,g3 = 0x0000~0x3000
    // l0,l1,l2,l3 = 0x8000~0xb000
    if (stricmp(page, "g0") == 0) {
        return (0x0000);
    } else if (stricmp(page, "g1") == 0) {
        return (0x1000);
    } else if (stricmp(page, "g2") == 0) {
        return (0x2000);
    } else if (stricmp(page, "g3") == 0) {
        return (0x3000);
    } else if (stricmp(page, "g4") == 0) {
        return (0x4000);
    } else if (stricmp(page, "l0") == 0) {
        return (0x8000);
    } else if (stricmp(page, "l1") == 0) {
        return (0x9000);
    } else if (stricmp(page, "l2") == 0) {
        return (0xa000);
    } else if (stricmp(page, "l3") == 0) {
        return (0xb000);
    } else {
        printk(KERN_INFO "Wrong PageNo(%s).\n", page);
        return (0xFFFF);
    }
}

static int mt7610e_phy_get_version(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	int len;

	len = 0;
	len += sprintf(page, "version: %s\n", MT751X_PHY_DEBUG_VER);
	len += sprintf(page+len, "buid time: %s, %s\n", __DATE__, __TIME__);

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

static int mt7610e_phy_get_sw_version(char *page, char **start,
		off_t off, int count, int *eof, void *data)
{
	int len;

	len = 0;
	len += sprintf(page, "SW Ver : ");

	switch (tcPhyVer) {
		case mtPhyVer_7530:
			len += sprintf(page+len, "%s\n", mt7530_cfg[0].name);
			len += sprintf(page+len, "mtPhyVer_7530, tcPhyVer=0x%08X\n",
					tcPhyVer);
			break;
		case tcPhyVer_mt7510FE: // mt7510FE
			len += sprintf(page+len, "%s\n", mt7510FE_cfg[0].name);
			len += sprintf(page+len, "tcPhyVer_mt7510FE, tcPhyVer=0x%08X\n",
					tcPhyVer);
			break;
		case mtPhyVer_7510Ge: // MT7510Ge
			len += sprintf(page+len, "mt7510Ge_cfg_cl45(Nill)\n");
			len += sprintf(page+len, "mt7510Ge_cfg_cl45, tcPhyVer=0x%08X\n",
					tcPhyVer);
			break;
		default:
			len += sprintf(page+len, "Unknown PHY, tcPhyVer=0x%08X\n",
					tcPhyVer);
	}
	/* len += sprintf(page, "\n2013-04-01\n"); */

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

static int mt7610e_phy_miir_help(char *page, char **start,
		 off_t off, int count, int *eof, void *data)
{
	int len;

	len = 0;

	len += sprintf(page+len, "Usage: miir all <PhyAddr> [PageNo]\n");
	len += sprintf(page+len, "		miir <PhyAddr> <RegAddr>\n");
	len += sprintf(page+len, "		miir <PhyAddr> <PageNo> <RegAddr>\n");

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

 /*
  * usage:
  *     argc: 2
  *         all <PhyAddr>
  *         <PhyAddr> <RegAddr>
  *     argc: 3
  *         all <PhyAddr> <PageNo>
  *         <PhyAddr> <PageNo> <RegAddr>
  */
static int mt7610e_phy_miir(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
	int i;
	/* int ret; */
	int argc;
	u32 value;
	u32 phy_addr;
	u32 reg_addr;
	u32 page_no;
	u8 buf[128];
	u8 phyaddr_str[16];
	u8 regaddr_str[16];
	u8 pageno_str[16];

	phy_addr = 0;
	reg_addr = 0;
	page_no = 0;

	memset(buf, 0, sizeof(buf));
	if (copy_from_user(buf, buffer, count)) {
		return -EFAULT;
	}
	PHY_DEBUG("cmdline: %s", buf);

	//2 parse
	argc = sscanf(buf, "%s %s %s", phyaddr_str, pageno_str, regaddr_str);
	if (2 == argc) {
		if (stricmp(phyaddr_str, "all") == 0){ // all <PhyAddr>
			phy_addr = checked_atoi(pageno_str);
			PHY_DEBUG("argc[%d], phyaddr_str:%s(str), phy_addr:%0d",
					argc, phyaddr_str, phy_addr);
		} else { // <PhyAddr> <RegAddr>
			phy_addr = checked_atoi(phyaddr_str);
			reg_addr = checked_atoi(pageno_str);
			PHY_DEBUG("argc[%d], phy_addr:%0d(digit), reg_addr: %0d",
					argc, phy_addr, reg_addr);
		}
	} else if (3 == argc) {
		if (stricmp(phyaddr_str, "all") == 0){ // all <PhyAddr> <PageNo>
			phy_addr = checked_atoi(pageno_str);
			page_no = getMiiPage(regaddr_str);
			PHY_DEBUG("argc[%d], phyaddr_str:%s(str), phy_addr:%0d,"
					" page_no:%0d",
					argc, phyaddr_str, phy_addr, page_no);
		} else { // <PhyAddr> <PageNo> <RegAddr>
			phy_addr = checked_atoi(phyaddr_str);
			page_no = getMiiPage(pageno_str);
			reg_addr = checked_atoi(regaddr_str);
			PHY_DEBUG("argc[%d], phy_addr:%0d(digit), page_no:%0d,"
					" reg_addr:%0d",
					argc, phy_addr, page_no, phy_addr);
		}
	} else {
		printk(KERN_ERR "invalid parameters number: %d\n", argc);
		return -EFAULT;
	}

	if(reg_addr != 31) {
		mtMiiRegWrite(phy_addr, DEFAULT_PAGE_REG_NUM, 0);
	}

	if ((phy_addr > 31) || (reg_addr > 31) || (0xffff == page_no)) {
		printk(KERN_ERR "invalid parameters, the PhyAdd & RegAddr must less"
			" than 32, and page number must less than 0xFFFF\n");
	}

	printk(KERN_INFO "PhyAddr=%0d, PageNo=%0d, RegAddr=%0d\n",
			phy_addr, page_no, reg_addr);

	//2 set page
	if (argc == 3) {
		printk(KERN_INFO "PageNo=%0d\n", page_no);
		mtMiiRegWrite(phy_addr, DEFAULT_PAGE_REG_NUM, page_no);
	}
	//2 read data
	if (stricmp(phyaddr_str, "all") == 0) { // multiple read
		for(i = 0; i < 32; i++) {
			value = mtMiiRegRead(phy_addr, i);
			printk(KERN_INFO "[reg=%02d val=%08X]", i, value);
			if((i + 1) % 4 == 0 )
				printk(KERN_INFO "\n");
		}
	} else {
		value = mtMiiRegRead(phy_addr, reg_addr);
		printk(KERN_INFO "PhyAddr=%d RegAddr=%02d value=%08X\n",
				phy_addr, reg_addr, value);
	}

	return count;
}

static int mt7610e_phy_miiw_help(char *page, char **start,
		 off_t off, int count, int *eof, void *data)
{
	int len;

	len = 0;

	len += sprintf(page+len, "Usage: w <PhyAddr> <RegAddr> <RegVal>\n");
	len += sprintf(page+len, "	   w <PhyAddr> <PageNo> <RegAddr> <RegVal>\n");
	len += sprintf(page+len, "	   b <PhyAddr> <RegAddr> <STBit> <BFLen>"
			" <BFVal>\n");
	len += sprintf(page+len, "	   b <PhyAddr> <PageNo> <RegAddr> <STBit>"
			" <BFLen> <BFVal>\n");

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

/*
 * usage:
 *     w <PhyAddr> <RegAddr> <Value>
 *     w <PhyAddr> <PageNo> <RegAddr> <Value>
 *     b <PhyAddr> <RegAddr> <STBit> <BFLen> <BFVal>
 *     b <PhyAddr> <PageNo> <RegAddr> <STBit> <BFLen> <BFVal>
 */
static int mt7610e_phy_miiw(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
	int i;
	/* int ret; */
	int argc;
	u16 st_bit;
	u16 bf_len;
	u16 bf;
	u16 bf_mask;
	u32 op_mode;
	u32 phy_addr;
	u32 reg_addr;
	u32 page_no;
	u32 reg_val;
	u32 r_value;
	u8 buf[128];
	u8 op_mode_str[16];
	u8 phyaddr_str[16];
	u8 regaddr_str[16];
	u8 pageno_str[16];
	/* u8 regval_str[16]; */
	u8 stbit_str[16];
	u8 bflen_str[16];
	u8 bval_str[16];

	st_bit = 0;
	bf_len = 0;
	bf = 0;
	bf_mask = 0;
	phy_addr = 0;
	reg_addr = 0;
	page_no = 0;
	r_value = 0;

	memset(buf, 0, sizeof(buf));
	if (copy_from_user(buf, buffer, count)) {
		return -EFAULT;
	}
	PHY_DEBUG("cmdline: %s", buf);

	//2 parse
	argc = sscanf(buf, "%s %s %s %s %s %s %s",
			op_mode_str, phyaddr_str, pageno_str, regaddr_str, stbit_str,
			bflen_str, bval_str);
	if (4 == argc) { // w <PhyAddr> <RegAddr> <Value>
		if (stricmp(op_mode_str, "w") != 0) {
			PHY_DEBUG("invalid opmode: %s\n", op_mode_str);
			return -EFAULT;
		}
		op_mode = 1;
		phy_addr = checked_atoi(phyaddr_str);
		reg_addr = checked_atoi(pageno_str);
		// reg_val = checked_atoi(regaddr_str);
		sscanf(regaddr_str, "%x", &reg_val);
		PHY_DEBUG("argc[%d], op(%d), phy_addr:%0d, reg_addr:%0d,"
				" reg_val:0x%08X",
				argc, op_mode, phy_addr, reg_addr, reg_val);
	} else if (5 == argc) { //  w <PhyAddr> <PageNo> <RegAddr> <Value>
		if (stricmp(op_mode_str, "w") != 0) {
			PHY_DEBUG("invalid opmode: %s\n", op_mode_str);
			return -EFAULT;
		}
		op_mode = 1;
		phy_addr = checked_atoi(phyaddr_str);
		page_no = getMiiPage(pageno_str);
		reg_addr = checked_atoi(regaddr_str);
		// reg_val = checked_atoi(stbit_str);
		sscanf(stbit_str, "%x", &reg_val);
		PHY_DEBUG("argc[%d], op(%d), phy_addr:%0d, page_no:%0d,"
				" phy_addr:%0d, reg_val:0x%08X",
				argc, op_mode, phy_addr, page_no, phy_addr, reg_val);
	} else if (6 == argc) { // b <PhyAddr> <RegAddr> <STBit> <BFLen> <BFVal>
		if (stricmp(op_mode_str, "b") != 0) {
			PHY_DEBUG("invalid opmode: %s\n", op_mode_str);
			return -EFAULT;
		}
		op_mode = 0;
		phy_addr = checked_atoi(phyaddr_str);
		reg_addr = checked_atoi(pageno_str);
		st_bit = checked_atoi(regaddr_str);
		bf_len = checked_atoi(stbit_str);
		// reg_val = checked_atoi(bflen_str);
		sscanf(bflen_str, "%x", &reg_val);
		PHY_DEBUG("argc[%d], op(%d), phy_addr:%0d, reg_addr:%0d,"
				" st_bit:%0d, bf_len:%0d, reg_val:0x%08X",
				argc, op_mode, phy_addr, reg_addr, st_bit, bf_len, reg_val);
	} else if (7 == argc) {
		// b <PhyAddr> <PageNo> <RegAddr> <STBit> <BFLen> <BFVal>
		if (stricmp(op_mode_str, "b") != 0) {
			PHY_DEBUG("invalid opmode: %s\n", op_mode_str);
			return -EFAULT;
		}
		op_mode = 0;
		phy_addr = checked_atoi(phyaddr_str);
		page_no = getMiiPage(pageno_str);
		reg_addr = checked_atoi(regaddr_str);
		st_bit = checked_atoi(stbit_str);
		bf_len = checked_atoi(bflen_str);
		// reg_val = checked_atoi(bval_str);
		sscanf(bval_str, "%x", &reg_val);
		PHY_DEBUG("argc[%d], op(%d), phy_addr:%0d, page_no:%0d, "
				"reg_addr:%0d, st_bit:%0d, bf_len:%0d, reg_val:%0d",
				argc, op_mode, phy_addr, page_no, reg_addr, st_bit, bf_len,
				reg_val);
	} else {
		printk(KERN_ERR "invalid parameters number: %d\n", argc);
		return -EFAULT;
	}

	if(reg_addr != 31) {
		mtMiiRegWrite(phy_addr, DEFAULT_PAGE_REG_NUM, 0);
	}

	//2 set page
	if ((phy_addr > 31) || (reg_addr > 31) || (0xffff == page_no)) {
		printk(KERN_ERR "invalid parameters, the PhyAdd & RegAddr must less"
			" than 32, and page number must less than 0xFFFF\n");
	}

	//2 write data
	if (1 == op_mode) {
		// set page
		if (5 == argc) {
			printk(KERN_INFO "PageNo=%0d\n", page_no);
			mtMiiRegWrite(phy_addr, DEFAULT_PAGE_REG_NUM, page_no);
		}
		// write data
		printk(KERN_INFO "Phyaddr=%0d, RegAddr=%0d, value=%08X\n",
				phy_addr, reg_addr, reg_val);
		mtMiiRegWrite(phy_addr, reg_addr, reg_val);
	} else if (0 == op_mode) {
		if (argc == 7) {
			printk(KERN_INFO "PageNo=%0d\n", page_no);
			mtMiiRegWrite(phy_addr, DEFAULT_PAGE_REG_NUM, page_no);
		}

		r_value = mtMiiRegRead(phy_addr, reg_addr);
		for(i = 0; i < bf_len; i++) {
			bf = 1;
			bf = bf << (st_bit + i);
			bf_mask = bf_mask | bf;
		}
		bf_mask = ~bf_mask;
		reg_val = (r_value & bf_mask) | (reg_val << bf_mask);
		printk(KERN_INFO "Phyaddr=%d, RegAddr=%0d, OrgValue=0x%08X,"
				" Modified value=0x%08X\n",
				phy_addr, reg_addr, r_value, reg_val);

		printk(KERN_INFO "OrgValue=0x%08X,", r_value);
		mtMiiRegWrite(phy_addr, reg_addr, reg_val);
		r_value = mtMiiRegRead(phy_addr, reg_addr);
		printk(KERN_INFO " ModValue=0x%08X\n", r_value);
	}

	return count;
}

int mt751x_creat_phy_dbg_entry(void)
{
	u8 tmp[32];

	sprintf(tmp, "driver/%s", MT751X_PHY_MOD_NAME);
	proc_phy_dbg = proc_mkdir(tmp, NULL);
	if (!proc_phy_dbg) {
		printk(KERN_ERR "failed while create proc dir \"%s\"\n", tmp);

		return -1;
	}

	entry_phy_ver = create_proc_entry("version", 0644, proc_phy_dbg);
	if (!entry_phy_ver) {
		printk(KERN_ERR "failed while create version entry \"%s\"\n", tmp);

		goto ERR_CREAT_PHY_DBG_VER;
	}
	entry_phy_ver->read_proc = mt7610e_phy_get_version;
	entry_phy_ver->write_proc = NULL;
	entry_phy_ver->mode = S_IFREG | S_IRUGO;
	entry_phy_ver->uid = 0;
	entry_phy_ver->gid = 0;
	entry_phy_ver->size = 37;
	printk(KERN_INFO "PHY driver version proc created\n");

	entry_phy_sw_ver = create_proc_entry("Ver", 0644, proc_phy_dbg);
	if (!entry_phy_sw_ver) {
		printk(KERN_ERR "failed while create s/w version entry \"%s\"\n", tmp);

		goto ERR_CREAT_PHY_SW_VER;
	}
	entry_phy_sw_ver->read_proc = mt7610e_phy_get_sw_version;
	entry_phy_sw_ver->write_proc = NULL;
	entry_phy_sw_ver->mode = S_IFREG | S_IRUGO;
	entry_phy_sw_ver->uid = 0;
	entry_phy_sw_ver->gid = 0;
	entry_phy_sw_ver->size = 37;
	printk(KERN_INFO "PHY s/w version proc created\n");

	entry_phy_miir = create_proc_entry("miir", 0644, proc_phy_dbg);
	if (!entry_phy_miir) {
		printk(KERN_ERR "failed while create PHY MII Read entry\n");
		goto ERR_CREAT_PHY_MIIR_PROC;
	}
	entry_phy_miir->read_proc = mt7610e_phy_miir_help;
	entry_phy_miir->write_proc = mt7610e_phy_miir;
	entry_phy_miir->mode = S_IFREG | S_IRUGO;
	entry_phy_miir->uid = 0;
	entry_phy_miir->gid = 0;
	entry_phy_miir->size = 37;
	printk(KERN_INFO "PHY MII Read proc created\n");

	entry_phy_miiw = create_proc_entry("miiw", 0644, proc_phy_dbg);
	if (!entry_phy_miiw) {
		printk(KERN_ERR "failed while create PHY MII Write entry\n");
		goto ERR_CREAT_PHY_MIIW_PROC;
	}
	entry_phy_miiw->read_proc = mt7610e_phy_miiw_help;
	entry_phy_miiw->write_proc = mt7610e_phy_miiw;
	entry_phy_miiw->mode = S_IFREG | S_IRUGO;
	entry_phy_miiw->uid = 0;
	entry_phy_miiw->gid = 0;
	entry_phy_miiw->size = 37;
	printk(KERN_INFO "PHY MII Write proc created\n");

	return 0;

ERR_CREAT_PHY_MIIW_PROC:
	remove_proc_entry("miir", proc_phy_dbg);

ERR_CREAT_PHY_MIIR_PROC:
	remove_proc_entry("Ver", proc_phy_dbg);

ERR_CREAT_PHY_SW_VER:
	remove_proc_entry("version", entry_phy_ver);

ERR_CREAT_PHY_DBG_VER:
	sprintf(tmp, "driver/%s", MT751X_PHY_MOD_NAME);
	remove_proc_entry(tmp, NULL);

	return -1;
}

void mt751x_remove_phy_dbg_entry(void)
{
	u8 tmp[32];

	if (entry_phy_miiw) {
		remove_proc_entry("miiw", proc_phy_dbg);
		entry_phy_miiw = NULL;
	}

	if (entry_phy_miir) {
		remove_proc_entry("miir", proc_phy_dbg);
		entry_phy_miir = NULL;
	}

	if (entry_phy_sw_ver) {
		remove_proc_entry("Ver", proc_phy_dbg);
		entry_phy_sw_ver = NULL;
	}

	if (entry_phy_ver) {
		remove_proc_entry("version", proc_phy_dbg);
		entry_phy_ver = NULL;
	}

	if (proc_phy_dbg) {
		sprintf(tmp, "driver/%s", MT751X_PHY_MOD_NAME);
		remove_proc_entry(tmp, NULL);
		proc_phy_dbg = NULL;
	}
}
