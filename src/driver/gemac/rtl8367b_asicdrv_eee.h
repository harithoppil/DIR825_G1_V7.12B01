#ifndef _RTL8367B_ASICDRV_EEE_H_
#define _RTL8367B_ASICDRV_EEE_H_

#include "rtl8367b_asicdrv.h"

extern int rtl8367b_setAsicEee100M(unsigned int port, unsigned int enable);
extern int rtl8367b_getAsicEee100M(unsigned int port, unsigned int *enable);
extern int rtl8367b_setAsicEeeGiga(unsigned int port, unsigned int enable);
extern int rtl8367b_getAsicEeeGiga(unsigned int port, unsigned int *enable);


#endif /*_RTL8367B_ASICDRV_EEE_H_*/
