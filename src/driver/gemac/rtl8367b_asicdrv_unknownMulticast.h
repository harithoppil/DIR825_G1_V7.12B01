#ifndef _RTL8367B_ASICDRV_UNKNOWNMULTICAST_H_
#define _RTL8367B_ASICDRV_UNKNOWNMULTICAST_H_

#include "rtl8367b_asicdrv.h"

enum L2_UNKOWN_MULTICAST_BEHAVE
{
    L2_UNKOWN_MULTICAST_FLOODING = 0,
    L2_UNKOWN_MULTICAST_DROP,
    L2_UNKOWN_MULTICAST_TRAP,
    L2_UNKOWN_MULTICAST_DROP_EXCLUDE_RMA,
    L2_UNKOWN_MULTICAST_END
};

enum L3_UNKOWN_MULTICAST_BEHAVE
{
    L3_UNKOWN_MULTICAST_FLOODING = 0,
    L3_UNKOWN_MULTICAST_DROP,
    L3_UNKOWN_MULTICAST_TRAP,
    L3_UNKOWN_MULTICAST_ROUTER,
    L3_UNKOWN_MULTICAST_END
};

enum MULTICASTTYPE{
	MULTICAST_TYPE_IPV4 = 0,
	MULTICAST_TYPE_IPV6,
	MULTICAST_TYPE_L2,
	MULTICAST_TYPE_END
};

extern int rtl8367b_setAsicUnknownL2MulticastBehavior(unsigned int port, unsigned int behave);
extern int rtl8367b_getAsicUnknownL2MulticastBehavior(unsigned int port, unsigned int *pBehave);
extern int rtl8367b_setAsicUnknownIPv4MulticastBehavior(unsigned int port, unsigned int behave);
extern int rtl8367b_getAsicUnknownIPv4MulticastBehavior(unsigned int port, unsigned int *pBehave);
extern int rtl8367b_setAsicUnknownIPv6MulticastBehavior(unsigned int port, unsigned int behave);
extern int rtl8367b_getAsicUnknownIPv6MulticastBehavior(unsigned int port, unsigned int *pBehave);
extern int rtl8367b_setAsicUnknownMulticastTrapPriority(unsigned int priority);
extern int rtl8367b_getAsicUnknownMulticastTrapPriority(unsigned int *pPriority);

#endif /*_RTL8367B_ASICDRV_UNKNOWNMULTICAST_H_*/


