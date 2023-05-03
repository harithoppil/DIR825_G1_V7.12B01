#ifndef _RTL8367B_ASICDRV_VLAN_H_
#define _RTL8367B_ASICDRV_VLAN_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include "rtl8367b_asicdrv.h"

/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define RTL8367B_PROTOVLAN_GIDX_MAX 3
#define RTL8367B_PROTOVLAN_GROUPNO  4


/****************************************************************/
/* Type Definition                                              */
/****************************************************************/
typedef struct  VLANCONFIGSMI
{
#ifdef _LITTLE_ENDIAN
	unsigned short	mbr:8;
	unsigned short  reserved:8;

	unsigned short	fid_msti:4;
	unsigned short  reserved2:12;
	
	unsigned short	vbpen:1;
	unsigned short	vbpri:3;
	unsigned short	envlanpol:1;
	unsigned short	meteridx:5;
	unsigned short	reserved3:6;

	unsigned short	evid:13;
	unsigned short  reserved4:3;
#else
	unsigned short  reserved:8;
	unsigned short	mbr:8;

	unsigned short  reserved2:12;
	unsigned short	fid_msti:4;
	
	unsigned short	reserved3:6;
	unsigned short	meteridx:5;
	unsigned short	envlanpol:1;
	unsigned short	vbpri:3;
	unsigned short	vbpen:1;

	unsigned short  reserved4:3;
	unsigned short	evid:13;
#endif
	
}rtl8367b_vlanconfigsmi;

typedef struct  VLANCONFIGUSER
{
    unsigned short 	evid;
	unsigned short 	mbr;
    unsigned short  fid_msti;
    unsigned short  envlanpol;
    unsigned short  meteridx;
    unsigned short  vbpen;
    unsigned short  vbpri;
}rtl8367b_vlanconfiguser;

typedef struct  VLANTABLE
{
#ifdef _LITTLE_ENDIAN
	unsigned short 	mbr:8;
 	unsigned short 	untag:8;

 	unsigned short 	fid_msti:4;
 	unsigned short 	vbpen:1;
	unsigned short	vbpri:3;
	unsigned short	envlanpol:1;
	unsigned short	meteridx:5;
	unsigned short	ivl_svl:1;	
	unsigned short	reserved:1;	
#else
 	unsigned short 	untag:8;
	unsigned short 	mbr:8;

	unsigned short	reserved:1;
	unsigned short	ivl_svl:1;	
	unsigned short	meteridx:5;
	unsigned short	envlanpol:1;
	unsigned short	vbpri:3;
 	unsigned short 	vbpen:1;
 	unsigned short 	fid_msti:4;

#endif
}rtl8367b_vlan4kentrysmi;

typedef struct  USER_VLANTABLE{

	unsigned short 	vid;
	unsigned short 	mbr;
 	unsigned short 	untag;
    unsigned short  fid_msti;
    unsigned short  envlanpol;
    unsigned short  meteridx;
    unsigned short  vbpen;
    unsigned short  vbpri;
	unsigned short 	ivl_svl;

}rtl8367b_user_vlan4kentry;

typedef enum
{
    FRAME_TYPE_BOTH = 0,
    FRAME_TYPE_TAGGED_ONLY,
    FRAME_TYPE_UNTAGGED_ONLY,
    FRAME_TYPE_MAX_BOUND
} rtl8367b_accframetype;

typedef enum
{
    EG_TAG_MODE_ORI = 0,
    EG_TAG_MODE_KEEP,
    EG_TAG_MODE_PRI_TAG,
    EG_TAG_MODE_REAL_KEEP,    
    EG_TAG_MODE_END
} rtl8367b_egtagmode;

typedef enum
{
    PPVLAN_FRAME_TYPE_ETHERNET = 0,
    PPVLAN_FRAME_TYPE_LLC,
    PPVLAN_FRAME_TYPE_RFC1042,
    PPVLAN_FRAME_TYPE_END
} rtl8367b_provlan_frametype;

enum RTL8367B_STPST
{
	STPST_DISABLED = 0,
	STPST_BLOCKING,
	STPST_LEARNING,
	STPST_FORWARDING
};


typedef struct
{
    rtl8367b_provlan_frametype  frameType;
    unsigned int                      etherType;
} rtl8367b_protocolgdatacfg;

typedef struct
{
    unsigned int valid;
    unsigned int vlan_idx;
    unsigned int priority;
} rtl8367b_protocolvlancfg;


void _rtl8367b_VlanMCStUser2Smi(rtl8367b_vlanconfiguser *pVlanCg, rtl8367b_vlanconfigsmi *pSmiVlanCfg);
void _rtl8367b_VlanMCStSmi2User(rtl8367b_vlanconfigsmi *pSmiVlanCfg, rtl8367b_vlanconfiguser *pVlanCg);
void _rtl8367b_Vlan4kStUser2Smi(rtl8367b_user_vlan4kentry *pUserVlan4kEntry, rtl8367b_vlan4kentrysmi *pSmiVlan4kEntry);
void _rtl8367b_Vlan4kStSmi2User(rtl8367b_vlan4kentrysmi *pSmiVlan4kEntry, rtl8367b_user_vlan4kentry *pUserVlan4kEntry);

extern int rtl8367b_setAsicVlanMemberConfig(unsigned int index, rtl8367b_vlanconfiguser *pVlanCg);
extern int rtl8367b_getAsicVlanMemberConfig(unsigned int index, rtl8367b_vlanconfiguser *pVlanCg);
extern int rtl8367b_setAsicVlan4kEntry(rtl8367b_user_vlan4kentry *pVlan4kEntry );
extern int rtl8367b_getAsicVlan4kEntry(rtl8367b_user_vlan4kentry *pVlan4kEntry );
extern int rtl8367b_setAsicVlanAccpetFrameType(unsigned int port, rtl8367b_accframetype frameType);
extern int rtl8367b_getAsicVlanAccpetFrameType(unsigned int port, rtl8367b_accframetype *pFrameType);
extern int rtl8367b_setAsicVlanIngressFilter(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicVlanIngressFilter(unsigned int port, unsigned int *pEnable);
extern int rtl8367b_setAsicVlanEgressTagMode(unsigned int port, rtl8367b_egtagmode tagMode);
extern int rtl8367b_getAsicVlanEgressTagMode(unsigned int port, rtl8367b_egtagmode *pTagMode);
extern int rtl8367b_setAsicVlanPortBasedVID(unsigned int port, unsigned int index, unsigned int pri);
extern int rtl8367b_getAsicVlanPortBasedVID(unsigned int port, unsigned int *pIndex, unsigned int *pPri);
extern int rtl8367b_setAsicVlanProtocolBasedGroupData(unsigned int index, rtl8367b_protocolgdatacfg *pPbCfg);
extern int rtl8367b_getAsicVlanProtocolBasedGroupData(unsigned int index, rtl8367b_protocolgdatacfg *pPbCfg);
extern int rtl8367b_setAsicVlanPortAndProtocolBased(unsigned int port, unsigned int index, rtl8367b_protocolvlancfg *pPpbCfg);
extern int rtl8367b_getAsicVlanPortAndProtocolBased(unsigned int port, unsigned int index, rtl8367b_protocolvlancfg *pPpbCfg);
extern int rtl8367b_setAsicVlanFilter(unsigned int enabled);
extern int rtl8367b_getAsicVlanFilter(unsigned int* pEnabled);

extern int rtl8367b_setAsicPortBasedFid(unsigned int port, unsigned int fid);
extern int rtl8367b_getAsicPortBasedFid(unsigned int port, unsigned int* pFid);
extern int rtl8367b_setAsicPortBasedFidEn(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicPortBasedFidEn(unsigned int port, unsigned int* pEnabled);
extern int rtl8367b_setAsicSpanningTreeStatus(unsigned int port, unsigned int msti, unsigned int state);
extern int rtl8367b_getAsicSpanningTreeStatus(unsigned int port, unsigned int msti, unsigned int* pState);
extern int rtl8367b_setAsicVlanUntagDscpPriorityEn(unsigned int enabled);
extern int rtl8367b_getAsicVlanUntagDscpPriorityEn(unsigned int* enabled);
extern int rtl8367b_setAsicVlanTransparent(unsigned int enabled);
extern int rtl8367b_getAsicVlanTransparent(unsigned int* pEnabled);
extern int rtl8367b_setAsicVlanEgressKeep(unsigned int port, unsigned int portmask);
extern int rtl8367b_getAsicVlanEgressKeep(unsigned int port, unsigned int* pPortmask);

#endif /*#ifndef _RTL8367B_ASICDRV_VLAN_H_*/

