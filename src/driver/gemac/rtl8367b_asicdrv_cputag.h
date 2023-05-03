#ifndef _RTL8367B_ASICDRV_CPUTAG_H_
#define _RTL8367B_ASICDRV_CPUTAG_H_

#include "rtl8367b_asicdrv.h"

enum CPUTAG_INSERT_MODE
{
    CPUTAG_INSERT_TO_ALL = 0,
    CPUTAG_INSERT_TO_TRAPPING,
    CPUTAG_INSERT_TO_NO,
    CPUTAG_INSERT_END
};

extern int rtl8367b_setAsicCputagEnable(unsigned int enabled);
extern int rtl8367b_getAsicCputagEnable(unsigned int *pEnabled);
extern int rtl8367b_setAsicCputagTrapPort(unsigned int port);
extern int rtl8367b_getAsicCputagTrapPort(unsigned int *pPort);
extern int rtl8367b_setAsicCputagPortmask(unsigned int portmask);
extern int rtl8367b_getAsicCputagPortmask(unsigned int *pPmsk);
extern int rtl8367b_setAsicCputagInsertMode(unsigned int mode);
extern int rtl8367b_getAsicCputagInsertMode(unsigned int *pMode);
extern int rtl8367b_setAsicCputagPriorityRemapping(unsigned int srcPri, unsigned int newPri);
extern int rtl8367b_getAsicCputagPriorityRemapping(unsigned int srcPri, unsigned int *pNewPri);
extern int rtl8367b_setAsicCputagPosition(unsigned int postion);
extern int rtl8367b_getAsicCputagPosition(unsigned int* pPostion);
extern int rtl8367b_setAsicCputagMode(unsigned int mode);
extern int rtl8367b_getAsicCputagMode(unsigned int *pMode);
extern int rtl8367b_setAsicCputagRxMinLength(unsigned int mode);
extern int rtl8367b_getAsicCputagRxMinLength(unsigned int *pMode);

#endif /*#ifndef _RTL8367B_ASICDRV_CPUTAG_H_*/

