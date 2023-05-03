#ifndef _RTL8367B_ASICDRV_LED_H_
#define _RTL8367B_ASICDRV_LED_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_LEDGROUPNO					3
#define RTL8367B_LEDGROUPMASK               0x7
#define RTL8367B_LED_FORCE_MODE_BASE        RTL8367B_REG_CPU_FORCE_LED0_CFG0
#define RTL8367B_LED_FORCE_CTRL             RTL8367B_REG_CPU_FORCE_LED_CFG

enum RTL8367B_LEDOP{

    LEDOP_SCAN0=0,
    LEDOP_SCAN1,
    LEDOP_PARALLEL,
    LEDOP_SERIAL,
    LEDOP_END,
};

enum RTL8367B_LEDSERACT{

    LEDSERACT_HIGH=0,
    LEDSERACT_LOW,
    LEDSERACT_MAX,
};

enum RTL8367B_LEDSER{

    LEDSER_16G=0,
    LEDSER_8G,
    LEDSER_MAX,
};

enum RTL8367B_LEDCONF{

    LEDCONF_LEDOFF=0,
    LEDCONF_DUPCOL,
    LEDCONF_LINK_ACT,
    LEDCONF_SPD1000,
    LEDCONF_SPD100,
    LEDCONF_SPD10,
    LEDCONF_SPD1000ACT,
    LEDCONF_SPD100ACT,
    LEDCONF_SPD10ACT,
    LEDCONF_SPD10010ACT,
    LEDCONF_LOOPDETECT,
    LEDCONF_EEE,
    LEDCONF_LINKRX,
    LEDCONF_LINKTX,
    LEDCONF_MASTER,
    LEDCONF_ACT,
    LEDCONF_END
};

enum RTL8367B_LEDBLINKRATE{

	LEDBLINKRATE_32MS=0,
	LEDBLINKRATE_64MS,
	LEDBLINKRATE_128MS,
	LEDBLINKRATE_256MS,
	LEDBLINKRATE_512MS,
	LEDBLINKRATE_1024MS,
	LEDBLINKRATE_48MS,
	LEDBLINKRATE_96MS,
	LEDBLINKRATE_END,
};

enum RTL8367B_LEDFORCEMODE{

    LEDFORCEMODE_NORMAL=0,
    LEDFORCEMODE_BLINK,
    LEDFORCEMODE_OFF,
    LEDFORCEMODE_ON,
    LEDFORCEMODE_END,
};

enum RTL8367B_LEDFORCERATE{

    LEDFORCERATE_512MS=0,
    LEDFORCERATE_1024MS,
    LEDFORCERATE_2048MS,
    LEDFORCERATE_NORMAL,
    LEDFORCERATE_END,

};

enum RTL8367B_LEDMODE
{
    RTL8367B_LED_MODE_0 = 0,
    RTL8367B_LED_MODE_1,
    RTL8367B_LED_MODE_2,
    RTL8367B_LED_MODE_3,
    RTL8367B_LED_MODE_END
};

extern int rtl8367b_setAsicLedIndicateInfoConfig(unsigned int ledno, unsigned int config);
extern int rtl8367b_getAsicLedIndicateInfoConfig(unsigned int ledno, unsigned int* pConfig);
extern int rtl8367b_setAsicForceLed(unsigned int port, unsigned int group, unsigned int mode);
extern int rtl8367b_getAsicForceLed(unsigned int port, unsigned int group, unsigned int* pMode);
extern int rtl8367b_setAsicForceGroupLed(unsigned int groupmask, unsigned int mode);
extern int rtl8367b_getAsicForceGroupLed(unsigned int* groupmask, unsigned int* pMode);
extern int rtl8367b_setAsicLedBlinkRate(unsigned int blinkRate);
extern int rtl8367b_getAsicLedBlinkRate(unsigned int* pBlinkRate);
extern int rtl8367b_setAsicLedForceBlinkRate(unsigned int blinkRate);
extern int rtl8367b_getAsicLedForceBlinkRate(unsigned int* pBlinkRate);
extern int rtl8367b_setAsicLedGroupMode(unsigned int mode);
extern int rtl8367b_getAsicLedGroupMode(unsigned int* pMode);
extern int rtl8367b_setAsicLedGroupEnable(unsigned int group, unsigned int portmask);
extern int rtl8367b_getAsicLedGroupEnable(unsigned int group, unsigned int *portmask);
extern int rtl8367b_setAsicLedOperationMode(unsigned int mode);
extern int rtl8367b_getAsicLedOperationMode(unsigned int *mode);
extern int rtl8367b_setAsicLedSerialModeConfig(unsigned int active, unsigned int serimode);
extern int rtl8367b_getAsicLedSerialModeConfig(unsigned int *active, unsigned int *serimode);
#endif /*#ifndef _RTL8367B_ASICDRV_LED_H_*/

