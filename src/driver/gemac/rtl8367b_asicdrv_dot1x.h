#ifndef _RTL8367B_ASICDRV_DOT1X_H_
#define _RTL8367B_ASICDRV_DOT1X_H_

#include "rtl8367b_asicdrv.h"

enum DOT1X_UNAUTH_BEHAV
{
    DOT1X_UNAUTH_DROP = 0,
    DOT1X_UNAUTH_TRAP,
    DOT1X_UNAUTH_GVLAN,
    DOT1X_UNAUTH_END
};

extern int rtl8367b_setAsic1xPBEnConfig(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsic1xPBEnConfig(unsigned int port, unsigned int *pEnabled);
extern int rtl8367b_setAsic1xPBAuthConfig(unsigned int port, unsigned int auth);
extern int rtl8367b_getAsic1xPBAuthConfig(unsigned int port, unsigned int *pAuth);
extern int rtl8367b_setAsic1xPBOpdirConfig(unsigned int port, unsigned int opdir);
extern int rtl8367b_getAsic1xPBOpdirConfig(unsigned int port, unsigned int *pOpdir);
extern int rtl8367b_setAsic1xMBEnConfig(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsic1xMBEnConfig(unsigned int port, unsigned int *pEnabled);
extern int rtl8367b_setAsic1xMBOpdirConfig(unsigned int opdir);
extern int rtl8367b_getAsic1xMBOpdirConfig(unsigned int *pOpdir);
extern int rtl8367b_setAsic1xProcConfig(unsigned int port, unsigned int proc);
extern int rtl8367b_getAsic1xProcConfig(unsigned int port, unsigned int *pProc);
extern int rtl8367b_setAsic1xGuestVidx(unsigned int index);
extern int rtl8367b_getAsic1xGuestVidx(unsigned int *pIndex);
extern int rtl8367b_setAsic1xGVOpdir(unsigned int enabled);
extern int rtl8367b_getAsic1xGVOpdir(unsigned int *pEnabled);
extern int rtl8367b_setAsic1xTrapPriority(unsigned int priority);
extern int rtl8367b_getAsic1xTrapPriority(unsigned int *pPriority);


#endif /*_RTL8367B_ASICDRV_DOT1X_H_*/

