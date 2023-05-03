#ifndef _MT751X_PHY_H_
#define _MT751X_PHY_H_

#include <linux/version.h>
#include <linux/mii.h>

#include "mt751x_femac.h"

#define read_reg_word(reg)    regRead32(reg)
#define write_reg_word(reg, wdata)     regWrite32(reg, wdata)
#define pause(x)    mdelay(x)
#define stricmp(x,y) strcmp(x,y)

/*Lions porting to linux relate funtion mapping.*/
#if  defined(CONFIG_MIPS_TC3182) ||  defined(CONFIG_MIPS_TC3262)
/*MII interface MACRO*/
#define PHY_CONTROL_REG		MII_BMCR
#define PHY_RESET		BMCR_RESET
#define MIIDR_AUTO_NEGOTIATE	(BMCR_ANRESTART | BMCR_ANENABLE)
#define PHY_STATUS_REG		MII_BMSR
#define PHY_REMOTE_CAP_REG		MII_LPA
#endif

/*end Lions porting to linux relate funtion mapping.*/

#if defined(TC2206_SUPPORT)|| defined(TC2031_SUPPORT) || defined(RT63365_SUPPORT)
#define TCPHY_SUPPORT	1 // All TC PHY initial & patch
#endif

#ifdef TC2206_SUPPORT
#define TCPHY_4PORT 1    //   TC2206
#endif

#if defined(TC2031_SUPPORT) /* UE model */
#define TCPHY_1PORT 1    //  LEH/LEM
//#define TCPHY_TESTCHIP 1 //   TC2104SD
#endif

#if defined(RT63365_SUPPORT)
#define TCPHY_5PORT 1
#endif

#if defined(MT7510FE_SUPPORT)
#define TCPHY_4PORT 1
#endif


#if defined(TCPHY_DEBUG) && defined(TCPHY_SUPPORT)
#define TCPHY_DEBUG		1 // All TC PHY debug (OFF if code-size is limited)
#define TCPHY_DEBUG_DISP_LEVEL 2 // Level=0..6 (default:OFF)
#endif

// 2. DBUGE : advanced debug function
#if (!defined(PURE_BRIDGE) && defined(TCPHY_SUPPORT)) && defined(TCPHY_DEBUG)
#define TCPHY_DEBUG		1 // All TC PHY debug (OFF if code-size is limited)
#define TCPHY_DEBUG_DISP_LEVEL 2 // Level=0..6 (default:OFF)
#endif

#define SW_MT7530_SUPPORT
#define MT7510Ge_SUPPORT
#define PHYPART_DEBUG  //for check FW hang in phy or others
//#define PHYPART_DEBUG_SW_PATCH  //for check FW hang in phy or others

// 1. SUPPORT : basic function
#if defined( SW_MT7530_SUPPORT)
#define MTPHY_SUPPORT	1
#define MTPHY_DEBUG		1 // All TC PHY debug (OFF if code-size is limited)
//#define MTPHY_DEBUG_DISP_LEVEL 2 // Level=0..6 (default:OFF) //CML_20130312
#define CL45_CMD_SUPPORT  1
#define TCPHY_DEBUG_DISP_LEVEL 2 // Level=0..6 (default:OFF) //CML_20130312
#endif

#define TCPHY_DEBUG		1
#define MTPHY_DEBUG		1


#if defined( MT7510Ge_SUPPORT)
#define MTPHY_SUPPORT	1
#define MTPHY_DEBUG		1 // All TC PHY debug (OFF if code-size is limited)
//#define MTPHY_DEBUG_DISP_LEVEL 2 // Level=0..6 (default:OFF) //CML_20130312
#define CL45_CMD_SUPPORT  1
#define TCPHY_DEBUG_DISP_LEVEL 2 // Level=0..6 (default:OFF) //CML_20130312
#endif

#if (defined(TC2105SG_SUPPORT) || defined(TC2105MJ_SUPPORT) || defined(TC2105SK_SUPPORT))
#define TCPHY_PORTNUM 5
#else
#if (defined(TC2104MC_SUPPORT) || defined(TC2104SD_SUPPORT) || defined(TC2104ME_SUPPORT) || defined(MT7510FE_SUPPORT))
#define TCPHY_PORTNUM 4
#else
#define TCPHY_PORTNUM 1
#endif
#endif
#ifdef SW_MT7530_SUPPORT
#define MTPHY_PORTNUM 5
#endif

#define MODEL_V0	(1<<0)	/*TC2206A1*/
#define MODEL_V1	(1<<1)	/*TC2206B2*/
#define MODEL_V2	(1<<2)	/*TC2206F*/

#define RT63365_SWIC_PORTNUM	7
#define MT7530_SWIC_PORTNUM	7

/* auto detect 4 port switch ic */
#define SWIC_ADM6996M		1
#define SWIC_IP175C			2
#define SWIC_MARVEL6060     3
#define SWIC_RTL8305        4
#define SWIC_RTL8306SD		5
#define SWIC_TC2206	7
#define SWIC_RT63365		8
#define SWIC_RT62806		9
#define SWIC_MT7530		10
#define HW_IGMPSNOOP	(1<<0)

#define RX_STAG_LEN 8
#define TX_STAG_LEN 6
#define DEF_STAG_VALUE 0x8901
#define IPPROTOL	0x0800

#define tcPhyVer_2031    0 // 9400, LEH
#define tcPhyVer_2101mb  1 // 9401, LEM
#define tcPhyVer_2104mc  2 // 9402, tc2206
#define tcPhyVer_2104sd  3 // 9403, tc2104sd
#define tcPhyVer_2101me  4 // 9404, 62UE
#define tcPhyVer_2102me  5 // 9405, tc3182
#define tcPhyVer_2104me  6 // 9406, tc2206F
#define tcPhyVer_2101mf  7 // 9407, FPGA(MF)
#define tcPhyVer_2105sg  8 // 9408, TC6501
#define tcPhyVer_2101mi  10 // 940a, RT63260
#define tcPhyVer_2105mj  11 // 940b, RT63365
#define tcPhyVer_2105sk  12 // 940c, TC6508
#define mtPhyVer_7530    13 //MT7530
#define tcPhyVer_mt7510FE 15 //MT7510,FE(Fast Ethernet)
#define mtPhyVer_7510Ge  16 //MT7510Ge

#define MT7530_PORTNUM 5
#define MT7530_PHY_INIT_LDATA_LEN 0
#define MT7530_PHY_INIT_PERPDATA_LEN 0
#define MT7530_PHY_INIT_SET_NUM 1

#define MT7530_PHY_INIT_CL45_GDATA_LEN 0
#define MT7530_PHY_INIT_CL45_LDATA_LEN 0
#define MT7530_PHY_INIT_CL45_PERPDATA_LEN 0
#define MT7530_PHY_INIT_CL45_SET_NUM 1

#define MT7530_PHY_INIT_TR_LDATA_LEN 0
#define MT7530_PHY_INIT_TR_PERPDATA_LEN 0
#define MT7530_PHY_INIT_TR_SET_NUM 1

#define MT7510FE_PHY_INIT_GDATA_LEN 6
#define MT7510FE_PHY_INIT_LDATA_LEN 5
#define MT7510FE_PHY_INIT_PERPDATA_LEN 1
#define MT7510FE_PHY_INIT_SET_NUM 1
#define MT7510FE_PORTNUM 4

#define MT7510Ge_PORTNUM 1
#define MT7510Ge_PHY_INIT_LDATA_LEN 0
#define MT7510Ge_PHY_INIT_PERPDATA_LEN 0
#define MT7510Ge_INIT_SET_NUM 1
#define MT7510Ge_PHY_INIT_SET_NUM 1

#define MT7510Ge_PHY_INIT_CL45_GDATA_LEN 2
#define MT7510Ge_PHY_INIT_CL45_LDATA_LEN 4
#define MT7510Ge_PHY_INIT_CL45_PERPDATA_LEN 0
#define MT7510Ge_PHY_INIT_CL45_SET_NUM 1

#define MT7510Ge_PHY_INIT_TR_LDATA_LEN 0
#define MT7510Ge_PHY_INIT_TR_PERPDATA_LEN 0
#define MT7510Ge_PHY_INIT_TR_SET_NUM 1

// 5. Port number
// #define TCPHY_PORTNUM 4
#define TCPHY_4PORT 1

#if (defined(TC2104MC_SUPPORT) || defined(TC2104SD_SUPPORT)|| defined(TC2104ME_SUPPORT) || defined(TC2105MJ_SUPPORT)|| defined(SW_MT7530_SUPPORT)||defined(MT7510Ge_SUPPORT))
#define DO_PER_PORT 1
#define DO_4_PORT 0
#define DO_5_PORT 2
#endif
#if 1
#define NORMAL_READ (1<<0)
#define POST_READ (1<<1)
#endif

/*MII interface MACRO*/
#define PHY_CONTROL_REG		MII_BMCR
#define PHY_RESET		BMCR_RESET
#define MIIDR_AUTO_NEGOTIATE	(BMCR_ANRESTART | BMCR_ANENABLE)
#define PHY_STATUS_REG		MII_BMSR
#define PHY_REMOTE_CAP_REG		MII_LPA

#define ST_LINK_DOWN 0
#define ST_LINK_DOWN2UP 1
#define ST_LINK_UP 2
#define ST_LINK_UP2DOWN 3

// type for register settings
typedef struct cfg_data_s{
	u32 reg_num;
	u32 val;
}cfg_data_t;

typedef struct cfg_cL45data_s{
	u32 dev_num;
	u32 reg_num;
	u32 val;
}cfg_cl45data_t;

typedef struct cfg_trdata_s{
	u8 reg_typ[10];
	u32 reg_num;
	u32 val;
}cfg_trdata_t;

typedef struct phyDeviceList_s {
	u16 companyId;
	char vendorName[30];
} phyDeviceList_t;

typedef struct mt7530_cfg_data_s{
	char name[10];
	cfg_data_t ldata[MT7530_PHY_INIT_LDATA_LEN];
	cfg_data_t perpdata[MT7530_PHY_INIT_PERPDATA_LEN]; //per port register setting
}mt7530_cfg_data_t;

typedef struct mt7530_cfg_cl45data_s{
	cfg_cl45data_t gdata[MT7530_PHY_INIT_CL45_GDATA_LEN];
	cfg_cl45data_t ldata[MT7530_PHY_INIT_CL45_LDATA_LEN];
	cfg_cl45data_t perpdata[MT7530_PHY_INIT_CL45_PERPDATA_LEN]; //per port register setting
}mt7530_cfg_cl45data_t;

typedef struct mt7530_cfg_trdata_s{
	cfg_trdata_t ldata[MT7530_PHY_INIT_TR_LDATA_LEN];
	cfg_trdata_t perpdata[MT7530_PHY_INIT_PERPDATA_LEN]; //per port register setting
}mt7530_cfg_trdata_t;

typedef struct mt7510FE_cfg_data_s{
	char name[10];
	cfg_data_t gdata[MT7510FE_PHY_INIT_GDATA_LEN];
	cfg_data_t ldata[MT7510FE_PHY_INIT_LDATA_LEN];
	cfg_data_t perpdata[MT7510FE_PHY_INIT_PERPDATA_LEN]; //per port register setting
}mt7510FE_cfg_data_t;

typedef struct mt7510Ge_cfg_data_s{
	char name[10];
	cfg_data_t ldata[MT7510Ge_PHY_INIT_LDATA_LEN];
	cfg_data_t perpdata[MT7510Ge_PHY_INIT_PERPDATA_LEN]; //per port register setting
}mt7510Ge_cfg_data_t;

typedef struct mt7510Ge_cfg_cl45data_s{
	cfg_cl45data_t gdata[MT7510Ge_PHY_INIT_CL45_GDATA_LEN];
	cfg_cl45data_t ldata[MT7510Ge_PHY_INIT_CL45_LDATA_LEN];
	cfg_cl45data_t perpdata[MT7510Ge_PHY_INIT_CL45_PERPDATA_LEN]; //per port register setting
}mt7510Ge_cfg_cl45data_t;

typedef struct mt7510Ge_cfg_trdata_s{
	cfg_trdata_t ldata[MT7510Ge_PHY_INIT_TR_LDATA_LEN];
	cfg_trdata_t perpdata[MT7510Ge_PHY_INIT_PERPDATA_LEN]; //per port register setting
}mt7510Ge_cfg_trdata_t;

typedef struct {
	u8 main_reset; // 15
	u8 force_speed; // 13
	u8 autoneg_enable; // 12
	u8 powerdown; // 11
	u8 force_duplex; // 8
} tcphy_mr0_reg_t;

typedef struct {
	u16 value; // 15:0
	//u8 autoneg_complete; // 5
	bool link_status; // 2
	bool link_status_prev;
} tcphy_mr1_reg_t;

typedef struct {
	u8 selector_field; //4:0
	u8 able100F; // 8
	u8 able100H; // 7
	u8 able10F; // 6
	u8 able10H; // 5
} tcphy_mr4_reg_t; // use for mr4 & mr5

typedef struct {
	u8 selector_field; //4:0
	u8 able100F; // 8
	u8 able100H; // 7
	u8 able10F; // 6
	u8 able10H; // 5
	u8 LPNextAble;//15
} tcphy_mr5_reg_t; // use for mr4 & mr5


typedef struct {
	//u8 parallel_detect_fault; // 4
	u8 lp_np_able; // 3
	//u8 np_able; // 2
	//u8 lch_page_rx; // 1
	u8 lp_autoneg_able; // 0
} tcphy_mr6_reg_t; // use for mr6

typedef struct {
	//u8  slicer_err_thd; // 15:11
	u16 err_over_cnt;   // 10:0
	u16 err_over_cnt_prev; // 10:0
} tcphy_l0r25_reg_t;

typedef struct {
	//u8  slicer_err_thd; // 15:11
	u16 err_over_cnt;   // 10:0
	u16 err_over_cnt_prev; // 10:0
} mtphy_errovcnt_reg_t;

typedef struct {
	u8 lch_sig_detect;
	u8 lch_rx_linkpulse;
	u8 lch_linkup_100;
	u8 lch_linkup_10;
	u8 lch_linkup_mdi;
	u8 lch_linkup_mdix;
	u8 lch_descr_lock;
	u8 mdix_status;
	u8 tx_amp_save;
	u8 final_duplex;
	u8 final_speed;
	u8 final_link;
} tcphy_l0r28_reg_t;

typedef struct {
	u8 lch_SignalDetect;//15
	u8 lch_LinkPulse;//14
	u8 lch_DescramblerLock1000;//13
	u8 lch_DescramblerLock100;//12
	u8 lch_LinkStatus1000_OK;//11
	u8 lch_LinkStatus100_OK;//10
	u8 lch_LinkStatus10_OK;//9
	u8 lch_MrPageRx;//8
	u8 lch_MrAutonegComplete;//7
	u8 da_mdix;//6
	u8 FullDuplexEnable;//5
	u8 MSConfig1000;// 4
	u8 final_speed_1000;// 3
	u8 final_speed_100;// 2
	u8 final_speed_10;// 1
} tcphy_1ErA2_reg_t;

typedef struct {
	u8 lp_eee_10g;
	u8 lp_eee_1000;
	u8 lp_eee_100;
} tcphy_l3r18_reg_t;

typedef struct swic_reg{
	u16 model_id;
	u16 phyaddr;
	u16 regaddr;
	u16 value;
}swic_reg_t;

/* for miir/miiw debug proc entry */
#undef PHY_PROC_DEBUG
/* #define PHY_PROC_DEBUG 1 */
#if PHY_PROC_DEBUG
#define PHY_DEBUG(format, arg...) \
	printk(KERN_INFO "%s()|%d: " format "\n", __func__, __LINE__, ##arg);
#else
#define PHY_DEBUG(n, arg...)
#endif

#define MT751X_PHY_MOD_NAME    "mt751x_phy"
#define MT751X_PHY_DEBUG_VER   "T&W mt751X debug PHY v0.1 (Camel)"

#define DEFAULT_PAGE_REG_NUM   31

u32 tcMiiStationRead(u32 enetPhyAddr, u32 phyReg);
void tcMiiStationWrite(u32 enetPhyAddr, u32 phyReg, u32 miiData);

#endif
