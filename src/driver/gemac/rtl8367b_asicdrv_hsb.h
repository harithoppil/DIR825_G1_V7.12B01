#ifndef _RTL8367B_ASICDRV__HSB_H_
#define _RTL8367B_ASICDRV__HSB_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_FIELDSEL_FORMAT_NUMBER      (16)
#define RTL8367B_FIELDSEL_MAX_OFFSET         (255)

enum FIELDSEL_FORMAT_FORMAT
{
    FIELDSEL_FORMAT_DEFAULT = 0,
    FIELDSEL_FORMAT_RAW,
	FIELDSEL_FORMAT_LLC,
	FIELDSEL_FORMAT_IPV4,
	FIELDSEL_FORMAT_ARP,
	FIELDSEL_FORMAT_IPV6,
	FIELDSEL_FORMAT_IPPAYLOAD,
	FIELDSEL_FORMAT_L4PAYLOAD,
    FIELDSEL_FORMAT_END
};

extern int rtl8367b_setAsicFieldSelector(unsigned int index, unsigned int format, unsigned int offset);
extern int rtl8367b_getAsicFieldSelector(unsigned int index, unsigned int* pFormat, unsigned int* pOffset);

#endif /*_RTL8367B_ASICDRV__HSB_H_*/

