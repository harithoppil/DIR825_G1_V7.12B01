#ifndef _RTL8367B_ASICDRV_TRUNKING_H_
#define _RTL8367B_ASICDRV_TRUNKING_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_MAX_TRUNK_GID              (1)
#define RTL8367B_TRUNKING_PORTNO       		(4)
#define RTL8367B_TRUNKING_HASHVALUE_MAX     (15)

extern int rtl8367b_setAsicTrunkingGroup(unsigned int group, unsigned int portmask);
extern int rtl8367b_getAsicTrunkingGroup(unsigned int group, unsigned int* pPortmask);
extern int rtl8367b_setAsicTrunkingFlood(unsigned int enabled);
extern int rtl8367b_getAsicTrunkingFlood(unsigned int* pEnabled);
extern int rtl8367b_setAsicTrunkingHashSelect(unsigned int hashsel);
extern int rtl8367b_getAsicTrunkingHashSelect(unsigned int* pHashsel);

extern int rtl8367b_getAsicQeueuEmptyStatus(unsigned int* pPortmask);

extern int rtl8367b_setAsicTrunkingMode(unsigned int mode);
extern int rtl8367b_getAsicTrunkingMode(unsigned int* pMode);
extern int rtl8367b_setAsicTrunkingFc(unsigned int group, unsigned int enabled);
extern int rtl8367b_getAsicTrunkingFc(unsigned int group, unsigned int* pEnabled);
extern int rtl8367b_setAsicTrunkingHashTable(unsigned int hashval, unsigned int portId);
extern int rtl8367b_getAsicTrunkingHashTable(unsigned int hashval, unsigned int* pPortId);

#endif /*_RTL8367B_ASICDRV_TRUNKING_H_*/

