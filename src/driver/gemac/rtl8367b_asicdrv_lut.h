#ifndef _RTL8367B_ASICDRV_LUT_H_
#define _RTL8367B_ASICDRV_LUT_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_LUT_AGETIMERMAX        (7)
#define RTL8367B_LUT_AGESPEEDMAX        (3)
#define RTL8367B_LUT_LEARNLIMITMAX      (0x840)
#define RTL8367B_LUT_ADDRMAX            (0x083F)
#define RTL8367B_LUT_IPMCGRP_TABLE_MAX  (0x3F)
#define	RTL8367B_LUT_ENTRY_SIZE			(6)
#define	RTL8367B_LUT_BUSY_CHECK_NO		(10)

enum RTL8367B_LUTHASHMETHOD{

	LUTHASHMETHOD_SVL=0,
	LUTHASHMETHOD_IVL,
	LUTHASHMETHOD_END,
};


enum RTL8367B_LRNOVERACT{

	LRNOVERACT_FORWARD=0,
	LRNOVERACT_DROP,
	LRNOVERACT_TRAP,
	LRNOVERACT_END,
};

enum RTL8367B_LUTREADMETHOD{

	LUTREADMETHOD_MAC =0,
	LUTREADMETHOD_ADDRESS,
	LUTREADMETHOD_NEXT_ADDRESS,
	LUTREADMETHOD_NEXT_L2UC,
	LUTREADMETHOD_NEXT_L2MC,
	LUTREADMETHOD_NEXT_L3MC,
	LUTREADMETHOD_NEXT_L2L3MC,
	LUTREADMETHOD_NEXT_L2UCSPA,
};

enum RTL8367B_FLUSHMODE
{
	FLUSHMDOE_PORT = 0,
	FLUSHMDOE_VID,
	FLUSHMDOE_FID,
	FLUSHMDOE_END,
};

enum RTL8367B_FLUSHTYPE
{
	FLUSHTYPE_DYNAMIC = 0,
	FLUSHTYPE_BOTH,
	FLUSHTYPE_END,
};


typedef struct LUTTABLE{

	unsigned int sip;
	unsigned int dip;
	ether_addr_t mac;
	unsigned short ivl_svl:1;
	unsigned short cvid_fid:12;
	unsigned short fid:4;
	unsigned short efid:3;

	unsigned short nosalearn:1;
	unsigned short da_block:1;
	unsigned short sa_block:1;
	unsigned short auth:1;
	unsigned short lut_pri:3;
	unsigned short sa_en:1;
	unsigned short fwd_en:1;
	unsigned short mbr:8;
	unsigned short spa:4;
	unsigned short age:3;
	unsigned short l3lookup:1;
	unsigned short igmp_asic:1;
	unsigned short igmpidx:8;

	unsigned short lookup_hit:1;
	unsigned short lookup_busy:1;
	unsigned short address:12;

    unsigned short wait_time;

}rtl8367b_luttb;

struct fdb_maclearn_st{

#ifdef _LITTLE_ENDIAN
	unsigned short mac5:8;
	unsigned short mac4:8;

	unsigned short mac3:8;
	unsigned short mac2:8;

	unsigned short mac1:8;
	unsigned short mac0:8;

	unsigned short cvid_fid:12;
	unsigned short l3lookup:1;
	unsigned short ivl_svl:1;
    unsigned short reserved:2;

	unsigned short efid:3;
	unsigned short fid:4;
	unsigned short sa_en:1;
	unsigned short spa:3;
	unsigned short age:3;
	unsigned short auth:1;
	unsigned short sa_block:1;

	unsigned short da_block:1;
	unsigned short lut_pri:3;
	unsigned short fwd_en:1;
	unsigned short nosalearn:1;
    unsigned short reserved2:10;
#else
	unsigned short mac4:8;
	unsigned short mac5:8;

	unsigned short mac2:8;
	unsigned short mac3:8;

	unsigned short mac0:8;
	unsigned short mac1:8;

    unsigned short reserved:2;
	unsigned short ivl_svl:1;
	unsigned short l3lookup:1;
	unsigned short cvid_fid:12;

	unsigned short sa_block:1;
	unsigned short auth:1;
	unsigned short age:3;
	unsigned short spa:3;
	unsigned short sa_en:1;
	unsigned short fid:4;
	unsigned short efid:3;

    unsigned short reserved2:10;
	unsigned short nosalearn:1;
	unsigned short fwd_en:1;
	unsigned short lut_pri:3;
	unsigned short da_block:1;
#endif
};

struct fdb_l2multicast_st{

#ifdef _LITTLE_ENDIAN
	unsigned short mac5:8;
	unsigned short mac4:8;

	unsigned short mac3:8;
	unsigned short mac2:8;

	unsigned short mac1:8;
	unsigned short mac0:8;

	unsigned short cvid_fid:12;
	unsigned short l3lookup:1;
	unsigned short ivl_svl:1;
    unsigned short reserved:2;

	unsigned short mbr:8;
	unsigned short igmpidx:8;

	unsigned short igmp_asic:1;
	unsigned short lut_pri:3;
	unsigned short fwd_en:1;
	unsigned short nosalearn:1;
	unsigned short valid:1;
    unsigned short reserved2:9;
#else
	unsigned short mac4:8;
	unsigned short mac5:8;

	unsigned short mac2:8;
	unsigned short mac3:8;

	unsigned short mac0:8;
	unsigned short mac1:8;

    unsigned short reserved:2;
	unsigned short ivl_svl:1;
	unsigned short l3lookup:1;
	unsigned short cvid_fid:12;

	unsigned short igmpidx:8;
	unsigned short mbr:8;

    unsigned short reserved2:9;
	unsigned short valid:1;
	unsigned short nosalearn:1;
	unsigned short fwd_en:1;
	unsigned short lut_pri:3;
	unsigned short igmp_asic:1;
#endif
};

struct fdb_ipmulticast_st{

#ifdef _LITTLE_ENDIAN
    unsigned short sip0:8;
	unsigned short sip1:8;

	unsigned short sip2:8;
	unsigned short sip3:8;

	unsigned short dip0:8;
	unsigned short dip1:8;

	unsigned short dip2:8;
	unsigned short dip3:4;
	unsigned short l3lookup:1;
    unsigned short reserved:3;


	unsigned short mbr:8;
	unsigned short igmpidx:8;

	unsigned short igmp_asic:1;
	unsigned short lut_pri:3;
	unsigned short fwd_en:1;
	unsigned short nosalearn:1;
	unsigned short valid:1;
    unsigned short reserved2:9;
#else
	unsigned short sip1:8;
    unsigned short sip0:8;

	unsigned short sip3:8;
	unsigned short sip2:8;

	unsigned short dip1:8;
	unsigned short dip0:8;

    unsigned short reserved:3;
	unsigned short l3lookup:1;
	unsigned short dip3:4;
	unsigned short dip2:8;

	unsigned short igmpidx:8;
	unsigned short mbr:8;

    unsigned short reserved2:9;
	unsigned short valid:1;
	unsigned short nosalearn:1;
	unsigned short fwd_en:1;
	unsigned short lut_pri:3;
	unsigned short igmp_asic:1;

#endif
};

typedef union FDBSMITABLE{

	struct fdb_ipmulticast_st	smi_ipmul;
	struct fdb_l2multicast_st   smi_l2mul;
	struct fdb_maclearn_st		smi_auto;

}rtl8367b_fdbtb;


extern int rtl8367b_setAsicLutIpMulticastLookup(unsigned int enabled);
extern int rtl8367b_getAsicLutIpMulticastLookup(unsigned int* pEnabled);
extern int rtl8367b_setAsicLutAgeTimerSpeed(unsigned int timer, unsigned int speed);
extern int rtl8367b_getAsicLutAgeTimerSpeed(unsigned int* pTimer, unsigned int* pSpeed);
extern int rtl8367b_setAsicLutCamTbUsage(unsigned int enabled);
extern int rtl8367b_getAsicLutCamTbUsage(unsigned int* pEnabled);
extern int rtl8367b_getAsicLutCamType(unsigned int* pType);
extern int rtl8367b_setAsicLutLearnLimitNo(unsigned int port, unsigned int number);
extern int rtl8367b_getAsicLutLearnLimitNo(unsigned int port, unsigned int* pNumber);
extern int rtl8367b_setAsicLutLearnOverAct(unsigned int action);
extern int rtl8367b_getAsicLutLearnOverAct(unsigned int* pAction);
extern int rtl8367b_setAsicL2LookupTb(rtl8367b_luttb *pL2Table);
extern int rtl8367b_getAsicL2LookupTb(unsigned int method, rtl8367b_luttb *pL2Table);
extern int rtl8367b_getAsicLutLearnNo(unsigned int port, unsigned int* pNumber);
extern int rtl8367b_setAsicLutIpLookupMethod(unsigned int type);
extern int rtl8367b_getAsicLutIpLookupMethod(unsigned int* pType);
extern int rtl8367b_setAsicLutForceFlush(unsigned int portmask);
extern int rtl8367b_getAsicLutForceFlushStatus(unsigned int *pPortmask);
extern int rtl8367b_setAsicLutFlushMode(unsigned int mode);
extern int rtl8367b_getAsicLutFlushMode(unsigned int* pMode);
extern int rtl8367b_setAsicLutFlushType(unsigned int type);
extern int rtl8367b_getAsicLutFlushType(unsigned int* pType);
extern int rtl8367b_setAsicLutFlushVid(unsigned int vid);
extern int rtl8367b_getAsicLutFlushVid(unsigned int* pVid);
extern int rtl8367b_setAsicLutFlushFid(unsigned int fid);
extern int rtl8367b_getAsicLutFlushFid(unsigned int* pFid);
extern int rtl8367b_setAsicLutDisableAging(unsigned int port, unsigned int disabled);
extern int rtl8367b_getAsicLutDisableAging(unsigned int port, unsigned int *pDisabled);
extern int rtl8367b_setAsicLutIPMCGroup(unsigned int index, unsigned int group_addr);
extern int rtl8367b_getAsicLutIPMCGroup(unsigned int index, unsigned int *pGroup_addr);
extern int rtl8367b_setAsicLutLinkDownForceAging(unsigned int enable);
extern int rtl8367b_getAsicLutLinkDownForceAging(unsigned int *pEnable);

#endif /*_RTL8367B_ASICDRV_LUT_H_*/

