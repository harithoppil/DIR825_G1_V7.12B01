#ifndef _RTL8367B_ASICDRV_INBWCTRL_H_
#define _RTL8367B_ASICDRV_INBWCTRL_H_

#include "rtl8367b_asicdrv.h"

extern int rtl8367b_setAsicPortIngressBandwidth(unsigned int port, unsigned int bandwidth, unsigned int preifg, unsigned int enableFC);
extern int rtl8367b_getAsicPortIngressBandwidth(unsigned int port, unsigned int* pBandwidth, unsigned int* pPreifg, unsigned int* pEnableFC );
extern int rtl8367b_setAsicPortIngressBandwidthBypass(unsigned int enabled);
extern int rtl8367b_getAsicPortIngressBandwidthBypass(unsigned int* pEnabled);


#endif /*_RTL8367B_ASICDRV_INBWCTRL_H_*/

