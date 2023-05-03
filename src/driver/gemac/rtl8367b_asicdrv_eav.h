#ifndef _RTL8367B_ASICDRV_EAV_H_
#define _RTL8367B_ASICDRV_EAV_H_

#include "rtl8367b_asicdrv.h"


extern int rtl8367b_setAsicEavEnable(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicEavEnable(unsigned int port, unsigned int *pEnabled);
extern int rtl8367b_setAsicEavPriRemapping(unsigned int srcpriority, unsigned int priority);
extern int rtl8367b_getAsicEavPriRemapping(unsigned int srcpriority, unsigned int *pPriority);
extern int rtl8367b_setAsicEavTimeFreq(unsigned int frequence);
extern int rtl8367b_getAsicEavTimeFreq(unsigned int* pFrequence);
extern int rtl8367b_setAsicEavTimeOffsetSeccond(unsigned int second);
extern int rtl8367b_getAsicEavTimeOffsetSeccond(unsigned int* pSecond);
extern int rtl8367b_setAsicEavTimeOffset512ns(unsigned int ns);
extern int rtl8367b_getAsicEavTimeOffset512ns(unsigned int* pNs);
extern int rtl8367b_setAsicEavOffsetTune(unsigned int enabled);
extern int rtl8367b_getAsicEavSystemTimeTransmit(unsigned int* pTransmit);
extern int rtl8367b_getAsicEavSystemTimeSeccond(unsigned int* pSecond);
extern int rtl8367b_getAsicEavSystemTime512ns(unsigned int* pNs);
extern int rtl8367b_setAsicEavTimeSyncEn(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicEavTimeSyncEn(unsigned int port, unsigned int *pEnabled);
extern int rtl8367b_setAsicEavTimeStampFillEn(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicEavTimeStampFillEn(unsigned int port, unsigned int *pEnabled);
extern int rtl8367b_getAsicEavTimeSyncValid(unsigned int port, unsigned int *pValid);
extern int rtl8367b_getAsicEavEgressTimestampSeccond(unsigned int port, unsigned int* pSecond);
extern int rtl8367b_getAsicEavEgressTimestamp512ns(unsigned int port, unsigned int* pNs);

#endif /*#ifndef _RTL8367B_ASICDRV_EAV_H_*/

