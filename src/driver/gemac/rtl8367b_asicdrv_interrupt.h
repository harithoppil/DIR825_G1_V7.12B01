#ifndef _RTL8367B_ASICDRV_INTERRUPT_H_
#define _RTL8367B_ASICDRV_INTERRUPT_H_

#include "rtl8367b_asicdrv.h"

typedef enum RTL8367B_INTR_IMRS_E
{
    IMRS_LINK_CHANGE,
    IMRS_METER_EXCEED,
    IMRS_L2_LEARN,
    IMRS_SPEED_CHANGE,
    IMRS_SPECIAL_CONGESTION,
    IMRS_GREEN_FEATURE,
    IMRS_LOOP_DETECTION,
    IMRS_8051,
    IMRS_CABLE_DIAG,
    IMRS_ACL,
    IMRS_UPS,
    IMRS_SLIENT,
    IMRS_END,    
}RTL8367B_INTR_IMRS;

typedef enum RTL8367B_INTR_INDICATOR_E
{
	INTRST_L2_LEARN = 0,
	INTRST_SPEED_CHANGE,
	INTRST_SPECIAL_CONGESTION,
	INTRST_PORT_LINKDOWN,
	INTRST_PORT_LINKUP,
	INTRST_METER0_15,
	INTRST_METER16_31,
	INTRST_RLDP_LOOPED,
	INTRST_RLDP_RELEASED,
	INTRST_END,
}RTL8367B_INTR_INDICATOR;

extern int rtl8367b_setAsicInterruptPolarity(unsigned int polarity);
extern int rtl8367b_getAsicInterruptPolarity(unsigned int* pPolarity);
extern int rtl8367b_setAsicInterruptMask(unsigned int imr);
extern int rtl8367b_getAsicInterruptMask(unsigned int* pImr);
extern int rtl8367b_setAsicInterruptStatus(unsigned int ims);
extern int rtl8367b_getAsicInterruptStatus(unsigned int* pIms);
extern int rtl8367b_setAsicInterruptRelatedStatus(unsigned int type, unsigned int status);
extern int rtl8367b_getAsicInterruptRelatedStatus(unsigned int type, unsigned int* pStatus);


#endif /*#ifndef _RTL8367B_ASICDRV_INTERRUPT_H_*/

