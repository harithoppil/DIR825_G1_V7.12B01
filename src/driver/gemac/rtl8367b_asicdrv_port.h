#ifndef _RTL8367B_ASICDRV_PORTSECURITY_H_
#define _RTL8367B_ASICDRV_PORTSECURITY_H_

#include "rtl8367b_asicdrv.h"
#include "rtl8367b_asicdrv_unknownMulticast.h"
#include "rtl8367b_asicdrv_phy.h"

/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

#define	RTL8367B_MAC7		7
#define RTL8367B_EXTNO       3

#define RTL8367B_RTCT_PAGE          (11)
#define RTL8367B_RTCT_RESULT_A_REG  (27)
#define RTL8367B_RTCT_RESULT_B_REG  (28)
#define RTL8367B_RTCT_RESULT_C_REG  (29)
#define RTL8367B_RTCT_RESULT_D_REG  (30)
#define RTL8367B_RTCT_STATUS_REG    (26)

enum L2_SECURITY_BEHAVE
{
    L2_BEHAVE_FLOODING = 0,
    L2_BEHAVE_DROP,
    L2_BEHAVE_TRAP,
    L2_BEHAVE_END
};
enum L2_SECURITY_SA_BEHAVE
{
    L2_BEHAVE_SA_FLOODING = 0,
    L2_BEHAVE_SA_DROP,
    L2_BEHAVE_SA_TRAP,
    L2_BEHAVE_SA_COPY28051,
    L2_BEHAVE_SA_END
};

/* enum for port current link speed */
enum SPEEDMODE
{
	SPD_10M = 0,
	SPD_100M,
	SPD_1000M
};

/* enum for mac link mode */
enum LINKMODE
{
	MAC_NORMAL = 0,
	MAC_FORCE,
};

/* enum for port current link duplex mode */
enum DUPLEXMODE
{
	HALF_DUPLEX = 0,
	FULL_DUPLEX
};

/* enum for port current MST mode */
enum MSTMODE
{
	SLAVE_MODE= 0,
	MASTER_MODE
};

enum LINKSTATUS {
	LINK_DOWN = 0,
	LINK_UP
};

enum EXTMODE
{
    EXT_DISABLE = 0,
    EXT_RGMII,
    EXT_MII_MAC,
    EXT_MII_PHY,
    EXT_TMII_MAC,
    EXT_TMII_PHY,
    EXT_GMII,
    EXT_RMII_MAC,
    EXT_RMII_PHY,
    EXT_END
};

enum DOSTYPE
{
	DOS_DAEQSA = 0,
	DOS_LANDATTACKS,
	DOS_BLATATTACKS,
	DOS_SYNFINSCAN,
	DOS_XMASCAN,
	DOS_NULLSCAN,
	DOS_SYN1024,
	DOS_TCPSHORTHDR,
	DOS_TCPFRAGERROR,
	DOS_ICMPFRAGMENT,
	DOS_END,

};

typedef struct  rtl8367b_port_ability_s{
#ifdef _LITTLE_ENDIAN
    unsigned short speed:2;
    unsigned short duplex:1;
    unsigned short reserve1:1;
    unsigned short link:1;
    unsigned short rxpause:1;
    unsigned short txpause:1;
    unsigned short nway:1;
    unsigned short mstmode:1;
    unsigned short mstfault:1;
    unsigned short reserve2:2;
    unsigned short forcemode:1;
    unsigned short reserve3:3;
#else
    unsigned short reserve3:3;
    unsigned short forcemode:1;
    unsigned short reserve2:2;
    unsigned short mstfault:1;
    unsigned short mstmode:1;
    unsigned short nway:1;
    unsigned short txpause:1;
    unsigned short rxpause:1;
    unsigned short link:1;
    unsigned short reserve1:1;
    unsigned short duplex:1;
    unsigned short speed:2;

#endif
}rtl8367b_port_ability_t;

typedef struct  rtl8367b_port_status_s{
#ifdef _LITTLE_ENDIAN
    unsigned short speed:2;
    unsigned short duplex:1;
    unsigned short reserve1:1;
    unsigned short link:1;
    unsigned short rxpause:1;
    unsigned short txpause:1;
    unsigned short nway:1;
    unsigned short mstmode:1;
    unsigned short mstfault:1;
    unsigned short lpi100:1;
    unsigned short lpi1000:1;
    unsigned short reserve2:4;
#else
    unsigned short reserve2:4;
    unsigned short lpi1000:1;
    unsigned short lpi100:1;
    unsigned short mstfault:1;
    unsigned short mstmode:1;
    unsigned short nway:1;
    unsigned short txpause:1;
    unsigned short rxpause:1;
    unsigned short link:1;
    unsigned short reserve1:1;
    unsigned short duplex:1;
    unsigned short speed:2;

#endif
}rtl8367b_port_status_t;

typedef struct rtct_result_s
{
    unsigned int      channelAShort;
    unsigned int      channelBShort;
    unsigned int      channelCShort;
    unsigned int      channelDShort;

    unsigned int      channelAOpen;
    unsigned int      channelBOpen;
    unsigned int      channelCOpen;
    unsigned int      channelDOpen;

    unsigned int      channelAMismatch;
    unsigned int      channelBMismatch;
    unsigned int      channelCMismatch;
    unsigned int      channelDMismatch;

    unsigned int      channelALinedriver;
    unsigned int      channelBLinedriver;
    unsigned int      channelCLinedriver;
    unsigned int      channelDLinedriver;

    unsigned int      channelALen;
    unsigned int      channelBLen;
    unsigned int      channelCLen;
    unsigned int      channelDLen;
} rtl8367b_port_rtct_result_t;


/****************************************************************/
/* Driver Proto Type Definition                                 */
/****************************************************************/
extern int rtl8367b_setAsicPortUnknownDaBehavior(unsigned int behavior);
extern int rtl8367b_getAsicPortUnknownDaBehavior(unsigned int *pBehavior);
extern int rtl8367b_setAsicPortUnknownSaBehavior(unsigned int behavior);
extern int rtl8367b_getAsicPortUnknownSaBehavior(unsigned int *pBehavior);
extern int rtl8367b_setAsicPortUnmatchedSaBehavior(unsigned int behavior);
extern int rtl8367b_getAsicPortUnmatchedSaBehavior(unsigned int *pBehavior);
extern int rtl8367b_setAsicPortUnknownDaFloodingPortmask(unsigned int portmask);
extern int rtl8367b_getAsicPortUnknownDaFloodingPortmask(unsigned int *pPortmask);
extern int rtl8367b_setAsicPortUnknownMulticastFloodingPortmask(unsigned int portmask);
extern int rtl8367b_getAsicPortUnknownMulticastFloodingPortmask(unsigned int *pPortmask);
extern int rtl8367b_setAsicPortBcastFloodingPortmask(unsigned int portmask);
extern int rtl8367b_getAsicPortBcastFloodingPortmask(unsigned int *pPortmask);
extern int rtl8367b_setAsicPortBlockSpa(unsigned int port, unsigned int block);
extern int rtl8367b_getAsicPortBlockSpa(unsigned int port, unsigned int *pBlock);
extern int rtl8367b_setAsicPortForceLink(unsigned int port, rtl8367b_port_ability_t *pPortAbility);
extern int rtl8367b_getAsicPortForceLink(unsigned int port, rtl8367b_port_ability_t *pPortAbility);
extern int rtl8367b_getAsicPortStatus(unsigned int port, rtl8367b_port_status_t *pPortStatus);
extern int rtl8367b_setAsicPortForceLinkExt(unsigned int id, rtl8367b_port_ability_t *pPortAbility);
extern int rtl8367b_getAsicPortForceLinkExt(unsigned int id, rtl8367b_port_ability_t *pPortAbility);
extern int rtl8367b_setAsicPortExtMode(unsigned int id, unsigned int mode);
extern int rtl8367b_getAsicPortExtMode(unsigned int id, unsigned int *pMode);
extern int rtl8367b_setAsicPortDos(unsigned int type, unsigned int drop);
extern int rtl8367b_getAsicPortDos(unsigned int type, unsigned int* pDrop);
extern int rtl8367b_setAsicPortEnableAll(unsigned int enable);
extern int rtl8367b_getAsicPortEnableAll(unsigned int *pEnable);
extern int rtl8367b_setAsicPortSmallIpg(unsigned int port, unsigned int enable);
extern int rtl8367b_getAsicPortSmallIpg(unsigned int port, unsigned int* pEnable);
extern int rtl8367b_setAsicPortLoopback(unsigned int port, unsigned int enable);
extern int rtl8367b_getAsicPortLoopback(unsigned int port, unsigned int *pEnable);
extern int rtl8367b_setAsicPortRTCT(unsigned int portmask);
extern int rtl8367b_getAsicPortRTCTResult(unsigned int port, rtl8367b_port_rtct_result_t *pResult);

#endif /*_RTL8367B_ASICDRV_PORTSECURITY_H_*/

