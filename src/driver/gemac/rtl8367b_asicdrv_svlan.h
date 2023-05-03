#ifndef _RTL8367B_ASICDRV_SVLAN_H_
#define _RTL8367B_ASICDRV_SVLAN_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_C2SIDXNO               128
#define RTL8367B_C2SIDXMAX              (RTL8367B_C2SIDXNO-1)
#define RTL8367B_MC2SIDXNO              32
#define RTL8367B_MC2SIDXMAX             (RTL8367B_MC2SIDXNO-1)
#define RTL8367B_SP2CIDXNO              128
#define RTL8367B_SP2CMAX        		(RTL8367B_SP2CIDXNO-1)


enum RTL8367B_SPRISEL
{
    SPRISEL_INTERNALPRI =  0,
    SPRISEL_CTAGPRI,
    SPRISEL_VSPRI,
    SPRISEL_PBPRI,
    SPRISEL_END
};

enum RTL8367B_SUNACCEPT
{
    SUNACCEPT_DROP =  0,
    SUNACCEPT_TRAP,
    SUNACCEPT_SVLAN,
    SUNACCEPT_END
};

enum RTL8367B_SVLAN_MC2S_MODE
{
    SVLAN_MC2S_MODE_MAC =  0,
    SVLAN_MC2S_MODE_IP,
    SVLAN_MC2S_MODE_END
};


typedef struct  rtl8367b_svlan_memconf_s{

    unsigned short vs_member:8;
    unsigned short vs_untag:8;

    unsigned short vs_fid_msti:4;
    unsigned short vs_priority:3;
    unsigned short vs_force_fid:1;
    unsigned short reserved:8;

    unsigned short vs_svid:12;
    unsigned short vs_efiden:1;
    unsigned short vs_efid:3;


}rtl8367b_svlan_memconf_t;

typedef struct  rtl8367b_svlan_memconf_smi_s{
#ifdef _LITTLE_ENDIAN

    unsigned short vs_member:8;
	unsigned short vs_untag:8;

    unsigned short vs_fid_msti:4;
    unsigned short vs_priority:3;
    unsigned short vs_force_fid:1;
    unsigned short reserved:8;

    unsigned short vs_svid:12;
    unsigned short vs_efiden:1;
    unsigned short vs_efid:3;

#else
	unsigned short vs_untag:8;
    unsigned short vs_member:8;

    unsigned short reserved:8;
    unsigned short vs_force_fid:1;
    unsigned short vs_priority:3;
    unsigned short vs_fid_msti:4;

    unsigned short vs_efid:3;
    unsigned short vs_efiden:1;
    unsigned short vs_svid:12;

#endif
}rtl8367b_svlan_memconf_smi_t;


typedef struct  rtl8367b_svlan_c2s_smi_s{

#ifdef _LITTLE_ENDIAN

    unsigned short svidx:6;
    unsigned short reserved:10;

    unsigned short c2senPmsk:8;
    unsigned short reserved2:8;

    unsigned short evid:13;
    unsigned short reserved3:3;

#else

    unsigned short reserved:10;
    unsigned short svidx:6;

    unsigned short reserved2:8;
    unsigned short c2senPmsk:8;

    unsigned short reserved3:3;
    unsigned short evid:13;

#endif
}rtl8367b_svlan_c2s_smi_t;


typedef struct  rtl8367b_svlan_mc2s_s{

    unsigned short valid:1;
    unsigned short format:1;
    unsigned short svidx:6;
    unsigned int sdata;
    unsigned int smask;
}rtl8367b_svlan_mc2s_t;

typedef struct  rtl8367b_svlan_mc2s_smi_s{

#ifdef _LITTLE_ENDIAN

    unsigned short svidx:6;
    unsigned short format:1;
    unsigned short valid:1;
    unsigned short reserved:8;

    unsigned short mask0:8;
    unsigned short mask1:8;

    unsigned short mask2:8;
    unsigned short mask3:8;

    unsigned short data0:8;
    unsigned short data1:8;

    unsigned short data2:8;
    unsigned short data3:8;

#else
    unsigned short reserved:8;
    unsigned short valid:1;
    unsigned short format:1;
    unsigned short svidx:6;

    unsigned short mask1:8;
    unsigned short mask0:8;

    unsigned short mask3:8;
    unsigned short mask2:8;

    unsigned short data1:8;
    unsigned short data0:8;

    unsigned short data3:8;
    unsigned short data2:8;

#endif
}rtl8367b_svlan_mc2s_smi_t;

typedef struct  rtl8367b_svlan_s2c_s{

	unsigned short valid:1;
    unsigned short svidx:6;
    unsigned short dstport:3;
    unsigned int vid:12;
}rtl8367b_svlan_s2c_t;

typedef struct  rtl8367b_svlan_s2c_smi_s{

#ifdef _LITTLE_ENDIAN

    unsigned short dstport:3;
    unsigned short svidx:6;
	unsigned short reserved_1:7;

    unsigned short vid:12;
	unsigned short valid:1;
    unsigned short reserved_2:3;
#else
	unsigned short reserved_1:7;
    unsigned short svidx:6;
    unsigned short dstport:3;

    unsigned short reserved_2:3;
	unsigned short valid:1;
    unsigned short vid:12;
#endif
}rtl8367b_svlan_s2c_smi_t;

extern int rtl8367b_setAsicSvlanIngressUntag(unsigned int mode);
extern int rtl8367b_getAsicSvlanIngressUntag(unsigned int* pMode);
extern int rtl8367b_setAsicSvlanIngressUnmatch(unsigned int mode);
extern int rtl8367b_getAsicSvlanIngressUnmatch(unsigned int* pMode);
extern int rtl8367b_setAsicSvlanTrapPriority(unsigned int priority);
extern int rtl8367b_getAsicSvlanTrapPriority(unsigned int* pPriority);
extern int rtl8367b_setAsicSvlanDefaultVlan(unsigned int port, unsigned int index);
extern int rtl8367b_getAsicSvlanDefaultVlan(unsigned int port, unsigned int* pIndex);

extern int rtl8367b_setAsicSvlanMemberConfiguration(unsigned int index,rtl8367b_svlan_memconf_t* pSvlanMemCfg);
extern int rtl8367b_getAsicSvlanMemberConfiguration(unsigned int index,rtl8367b_svlan_memconf_t* pSvlanMemCfg);

extern int rtl8367b_setAsicSvlanPrioritySel(unsigned int priSel);
extern int rtl8367b_getAsicSvlanPrioritySel(unsigned int* pPriSel);
extern int rtl8367b_setAsicSvlanTpid(unsigned int protocolType);
extern int rtl8367b_getAsicSvlanTpid(unsigned int* pProtocolType);
extern int rtl8367b_setAsicSvlanUplinkPortMask(unsigned int portMask);
extern int rtl8367b_getAsicSvlanUplinkPortMask(unsigned int* pPortmask);
extern int rtl8367b_setAsicSvlanEgressUnassign(unsigned int enabled);
extern int rtl8367b_getAsicSvlanEgressUnassign(unsigned int* pEnabled);
extern int rtl8367b_setAsicSvlanC2SConf(unsigned int index, unsigned int evid, unsigned int portmask, unsigned int svidx);
extern int rtl8367b_getAsicSvlanC2SConf(unsigned int index, unsigned int* pEvid, unsigned int* pPortmask, unsigned int* pSvidx);
extern int rtl8367b_setAsicSvlanMC2SConf(unsigned int index,rtl8367b_svlan_mc2s_t* pSvlanMc2sCfg);
extern int rtl8367b_getAsicSvlanMC2SConf(unsigned int index,rtl8367b_svlan_mc2s_t* pSvlanMc2sCfg);
extern int rtl8367b_setAsicSvlanSP2CConf(unsigned int index,rtl8367b_svlan_s2c_t* pSvlanSp2cCfg);
extern int rtl8367b_getAsicSvlanSP2CConf(unsigned int index,rtl8367b_svlan_s2c_t* pSvlanSp2cCfg);
extern int rtl8367b_setAsicSvlanDmacCvidSel(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicSvlanDmacCvidSel(unsigned int port, unsigned int* pEnabled);
extern int rtl8367b_setAsicSvlanUntagVlan(unsigned int index);
extern int rtl8367b_getAsicSvlanUntagVlan(unsigned int* pIndex);
extern int rtl8367b_setAsicSvlanUnmatchVlan(unsigned int index);
extern int rtl8367b_getAsicSvlanUnmatchVlan(unsigned int* pIndex);

#endif /*#ifndef _RTL8367B_ASICDRV_SVLAN_H_*/

