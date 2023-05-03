#ifndef _RTL8367B_ASICDRV_GREEN_H_
#define _RTL8367B_ASICDRV_GREEN_H_

#include "rtl8367b_asicdrv.h"
#include "rtl8367b_asicdrv_phy.h"

#define PHY_POWERSAVING_REG                         21
#define PHY_POWERSAVING_OFFSET                      12
#define PHY_POWERSAVING_MASK                        0x1000

extern int rtl8367b_setAsicGreenTrafficType(unsigned int priority, unsigned int traffictype);
extern int rtl8367b_getAsicGreenTrafficType(unsigned int priority, unsigned int* pTraffictype);
extern int rtl8367b_getAsicGreenPortPage(unsigned int port, unsigned int* pPage);
extern int rtl8367b_getAsicGreenHighPriorityTraffic(unsigned int port, unsigned int* pIndicator);
extern int rtl8367b_setAsicGreenHighPriorityTraffic(unsigned int port);
extern int rtl8367b_setAsicGreenEthernet(unsigned int green);
extern int rtl8367b_getAsicGreenEthernet(unsigned int* green);
extern int rtl8367b_setAsicPowerSaving(unsigned int phy, unsigned int enable);
extern int rtl8367b_getAsicPowerSaving(unsigned int phy, unsigned int* enable);
#endif /*#ifndef _RTL8367B_ASICDRV_GREEN_H_*/

