/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 28599 $
 * $Date: 2012-05-07 09:41:37 +0800 (星期一, 07 五月 2012) $
 *
 * Purpose : RTL8367B switch high-level API for RTL8367B
 * Feature : PHY related functions
 *
 */
#include "rtl8367b_asicdrv_phy.h"

/* Function Name:
 *      rtl8367b_setAsicPHYReg
 * Description:
 *      Set PHY registers
 * Input:
 *      phyNo 	- Physical port number (0~4)
 *      phyAddr - PHY address (0~31)
 *      phyData - Writing data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PHY_REG_ID  		- invalid PHY address
 *      RT_ERR_PHY_ID  			- invalid PHY no
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      None
 */
int rtl8367b_setAsicPHYReg( unsigned int phyNo, unsigned int phyAddr, unsigned int value)
{
	unsigned int regAddr;

    if(phyNo > RTL8367B_PHY_INTERNALNOMAX)
        return RT_ERR_PORT_ID;

    if(phyAddr > RTL8367B_PHY_REGNOMAX)
        return RT_ERR_PHY_REG_ID;

    regAddr = 0x2000 + (phyNo << 5) + phyAddr;

    return rtl8367b_setAsicReg(regAddr, value);
}

/* Function Name:
 *      rtl8367b_getAsicPHYReg
 * Description:
 *      Get PHY registers
 * Input:
 *      phyNo 	- Physical port number (0~4)
 *      phyAddr - PHY address (0~31)
 *      pRegData - Writing data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PHY_REG_ID  		- invalid PHY address
 *      RT_ERR_PHY_ID  			- invalid PHY no
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      None
 */
int rtl8367b_getAsicPHYReg( unsigned int phyNo, unsigned int phyAddr, unsigned int *value)
{
	unsigned int regAddr;

    if(phyNo > RTL8367B_PHY_INTERNALNOMAX)
        return RT_ERR_PORT_ID;

    if(phyAddr > RTL8367B_PHY_REGNOMAX)
        return RT_ERR_PHY_REG_ID;

    regAddr = 0x2000 + (phyNo << 5) + phyAddr;

    return rtl8367b_getAsicReg(regAddr, value);
}

