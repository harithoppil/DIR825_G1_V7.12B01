#ifndef _RTL8367B_ASICDRV_PORTISOLATION_H_
#define _RTL8367B_ASICDRV_PORTISOLATION_H_

#include "rtl8367b_asicdrv.h"

extern int rtl8367b_setAsicPortIsolationPermittedPortmask(unsigned int port, unsigned int permitPortmask);
extern int rtl8367b_getAsicPortIsolationPermittedPortmask(unsigned int port, unsigned int *pPermitPortmask);
extern int rtl8367b_setAsicPortIsolationEfid(unsigned int port, unsigned int efid);
extern int rtl8367b_getAsicPortIsolationEfid(unsigned int port, unsigned int *pEfid);

#endif /*_RTL8367B_ASICDRV_PORTISOLATION_H_*/
