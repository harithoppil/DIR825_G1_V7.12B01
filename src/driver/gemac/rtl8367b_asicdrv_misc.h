#ifndef _RTL8367B_ASICDRV_MISC_H_
#define _RTL8367B_ASICDRV_MISC_H_

#include "rtl8367b_asicdrv.h"

extern int rtl8367b_setAsicMacAddress(ether_addr_t mac);
extern int rtl8367b_getAsicMacAddress(ether_addr_t *pMac);
extern int rtl8367b_getAsicDebugInfo(unsigned int port, unsigned int *pDebugifo);
extern int rtl8367b_setAsicPortJamMode(unsigned int mode);
extern int rtl8367b_getAsicPortJamMode(unsigned int* pMode);
extern int rtl8367b_setAsicMaxLengthInRx(unsigned int maxLength);
extern int rtl8367b_getAsicMaxLengthInRx(unsigned int* pMaxLength);
extern int rtl8367b_setAsicMaxLengthAltTxRx(unsigned int maxLength, unsigned int pmskGiga, unsigned int pmask100M);
extern int rtl8367b_getAsicMaxLengthAltTxRx(unsigned int* pMaxLength, unsigned int* pPmskGiga, unsigned int* pPmask100M);

#endif /*_RTL8367B_ASICDRV_MISC_H_*/

