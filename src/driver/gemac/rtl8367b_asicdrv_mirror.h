#ifndef _RTL8367B_ASICDRV_MIRROR_H_
#define _RTL8367B_ASICDRV_MIRROR_H_

#include "rtl8367b_asicdrv.h"

extern int rtl8367b_setAsicPortMirror(unsigned int source, unsigned int monitor);
extern int rtl8367b_getAsicPortMirror(unsigned int *pSource, unsigned int *pMonitor);
extern int rtl8367b_setAsicPortMirrorRxFunction(unsigned int enabled);
extern int rtl8367b_getAsicPortMirrorRxFunction(unsigned int* pEnabled);
extern int rtl8367b_setAsicPortMirrorTxFunction(unsigned int enabled);
extern int rtl8367b_getAsicPortMirrorTxFunction(unsigned int* pEnabled);
extern int rtl8367b_setAsicPortMirrorIsolation(unsigned int enabled);
extern int rtl8367b_getAsicPortMirrorIsolation(unsigned int* pEnabled);
extern int rtl8367b_setAsicPortMirrorPriority(unsigned int priority);
extern int rtl8367b_getAsicPortMirrorPriority(unsigned int* pPriority);
extern int rtl8367b_setAsicPortMirrorMask(unsigned int SourcePortmask);
extern int rtl8367b_getAsicPortMirrorMask(unsigned int *pSourcePortmask);

#endif /*#ifndef _RTL8367B_ASICDRV_MIRROR_H_*/

