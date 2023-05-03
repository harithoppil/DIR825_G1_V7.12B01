#ifndef _RTL8367B_ASICDRV_METER_H_
#define _RTL8367B_ASICDRV_METER_H_

#include "rtl8367b_asicdrv.h"


extern int rtl8367b_setAsicShareMeter(unsigned int index, unsigned int rate, unsigned int ifg);
extern int rtl8367b_getAsicShareMeter(unsigned int index, unsigned int *pRate, unsigned int *pIfg);
extern int rtl8367b_setAsicShareMeterBucketSize(unsigned int index, unsigned int lbThreshold);
extern int rtl8367b_getAsicShareMeterBucketSize(unsigned int index, unsigned int *pLbThreshold);
extern int rtl8367b_setAsicMeterExceedStatus(unsigned int index);
extern int rtl8367b_getAsicMeterExceedStatus(unsigned int index, unsigned int* pStatus);

#endif /*_RTL8367B_ASICDRV_FC_H_*/

