#ifndef _RTL8367B_ASICDRV_STORM_H_
#define _RTL8367B_ASICDRV_STORM_H_

#include "rtl8367b_asicdrv.h"

extern int rtl8367b_setAsicStormFilterBroadcastEnable(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicStormFilterBroadcastEnable(unsigned int port, unsigned int *pEnabled);
extern int rtl8367b_setAsicStormFilterBroadcastMeter(unsigned int port, unsigned int meter);
extern int rtl8367b_getAsicStormFilterBroadcastMeter(unsigned int port, unsigned int *pMeter);
extern int rtl8367b_setAsicStormFilterMulticastEnable(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicStormFilterMulticastEnable(unsigned int port, unsigned int *pEnabled);
extern int rtl8367b_setAsicStormFilterMulticastMeter(unsigned int port, unsigned int meter);
extern int rtl8367b_getAsicStormFilterMulticastMeter(unsigned int port, unsigned int *pMeter);
extern int rtl8367b_setAsicStormFilterUnknownMulticastEnable(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicStormFilterUnknownMulticastEnable(unsigned int port, unsigned int *pEnabled);
extern int rtl8367b_setAsicStormFilterUnknownMulticastMeter(unsigned int port, unsigned int meter);
extern int rtl8367b_getAsicStormFilterUnknownMulticastMeter(unsigned int port, unsigned int *pMeter);
extern int rtl8367b_setAsicStormFilterUnknownUnicastEnable(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicStormFilterUnknownUnicastEnable(unsigned int port, unsigned int *pEnabled);
extern int rtl8367b_setAsicStormFilterUnknownUnicastMeter(unsigned int port, unsigned int meter);
extern int rtl8367b_getAsicStormFilterUnknownUnicastMeter(unsigned int port, unsigned int *pMeter);

#endif /*_RTL8367B_ASICDRV_STORM_H_*/


