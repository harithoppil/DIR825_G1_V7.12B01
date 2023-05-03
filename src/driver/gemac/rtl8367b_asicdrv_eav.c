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
 * Feature : Ethernet AV related functions
 *
 */

#include "rtl8367b_asicdrv_eav.h"
/* Function Name:
 *      rtl8367b_setAsicEavEnable
 * Description:
 *      Set per-port EAV function enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      If EAV function is enabled, PTP event messgae packet will be attached PTP timestamp for trapping
 */
int rtl8367b_setAsicEavEnable(unsigned int port, unsigned int enabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;
	
    return rtl8367b_setAsicRegBit(RTL8367B_REG_EAV_CTRL0, port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicEavEnable
 * Description:
 *      Get per-port EAV function enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicEavEnable(unsigned int port, unsigned int *pEnabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_REG_EAV_CTRL0, port, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicEavPriRemapping
 * Description:
 *      Set non-EAV streaming priority remapping
 * Input:
 *      srcpriority - Priority value
 *      priority 	- Absolute priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY  	- Invalid priority
 * Note:
 *      None
 */
int rtl8367b_setAsicEavPriRemapping(unsigned int srcpriority, unsigned int priority)
{
	if(srcpriority > RTL8367B_PRIMAX || priority > RTL8367B_PRIMAX)
		return RT_ERR_QOS_INT_PRIORITY; 

	return rtl8367b_setAsicRegBits(RTL8367B_EAV_PRIORITY_REMAPPING_REG(srcpriority), RTL8367B_EAV_PRIORITY_REMAPPING_MASK(srcpriority),priority);		
}
/* Function Name:
 *      rtl8367b_getAsicEavPriRemapping
 * Description:
 *      Get non-EAV streaming priority remapping
 * Input:
 *      srcpriority - Priority value
 *      pPriority 	- Absolute priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY  	- Invalid priority
 * Note:
 *      None
 */
int rtl8367b_getAsicEavPriRemapping(unsigned int srcpriority, unsigned int *pPriority)
{
	if(srcpriority > RTL8367B_PRIMAX )
		return RT_ERR_QOS_INT_PRIORITY; 

	return rtl8367b_getAsicRegBits(RTL8367B_EAV_PRIORITY_REMAPPING_REG(srcpriority), RTL8367B_EAV_PRIORITY_REMAPPING_MASK(srcpriority),pPriority);		
}
/* Function Name:
 *      rtl8367b_setAsicEavTimeFreq
 * Description:
 *      Set EAV timing frequence
 * Input:
 *      frequence - frequence
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicEavTimeFreq(unsigned int frequence)
{
    return rtl8367b_setAsicReg(RTL8367B_REG_SYS_TIME_FREQ, frequence);
}
/* Function Name:
 *      rtl8367b_getAsicEavTimeFreq
 * Description:
 *      Get EAV timing frequence
 * Input:
 *      pFrequence - frequence
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicEavTimeFreq(unsigned int* pFrequence)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_SYS_TIME_FREQ, pFrequence);
}
/* Function Name:
 *      rtl8367b_setAsicEavTimeOffsetSeccond
 * Description:
 *      Set EAV timing second offset
 * Input:
 *      second - seconds
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      Ethernet AV second offset of timer for tuning
 */
int rtl8367b_setAsicEavTimeOffsetSeccond(unsigned int second)
{
	int  retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_REG_SYS_TIME_OFFSET_L, (second & 0xFFFF));
	if(retVal != RT_ERR_OK)
		return retVal;
	
    return rtl8367b_setAsicReg(RTL8367B_REG_SYS_TIME_OFFSET_H, (second >> 16));
}
/* Function Name:
 *      rtl8367b_getAsicEavTimeOffsetSeccond
 * Description:
 *      Get EAV timing second offset
 * Input:
 *      pSecond - seconds
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicEavTimeOffsetSeccond(unsigned int* pSecond)
{
	int  retVal;
	unsigned int 	regData, regData2;

	retVal = rtl8367b_getAsicReg(RTL8367B_REG_SYS_TIME_OFFSET_L, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	retVal = rtl8367b_getAsicReg(RTL8367B_REG_SYS_TIME_OFFSET_H, &regData2);
	if(retVal != RT_ERR_OK)
		return retVal;

	*pSecond = (regData2 << 16) | regData;
	
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicEavTimeOffset512ns
 * Description:
 *      Set EAV timing 512 nano-second offset
 * Input:
 *      ns - 512 nano-second
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicEavTimeOffset512ns(unsigned int ns)
{
	int  retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_REG_SYS_TIME_OFFSET_512NS_L, (ns & 0xFFFF));
	if(retVal != RT_ERR_OK)
		return retVal;
	
    return rtl8367b_setAsicRegBits(RTL8367B_REG_SYS_TIME_OFFSET_512NS_H, RTL8367B_SYS_TIME_OFFSET_512NS_H_SYS_TIME_OFFSET_512NS_MASK, (ns >> 16));
}
/* Function Name:
 *      rtl8367b_getAsicEavTimeOffset512ns
 * Description:
 *      Get EAV timing 512 nano-second offset
 * Input:
 *      pNs - 512 nano-second
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicEavTimeOffset512ns(unsigned int* pNs)
{
	int   retVal;
	unsigned int 	regData, regData2;

	retVal = rtl8367b_getAsicReg(RTL8367B_REG_SYS_TIME_OFFSET_512NS_L, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_SYS_TIME_OFFSET_512NS_H, RTL8367B_SYS_TIME_OFFSET_512NS_H_SYS_TIME_OFFSET_512NS_MASK, &regData2);
	if(retVal != RT_ERR_OK)
		return retVal;

	*pNs = (regData2 << 16) | regData;
	
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicEavOffsetTune
 * Description:
 *      Set EAV timer offset tune
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicEavOffsetTune(unsigned int enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_SYS_TIME_OFFSET_512NS_H, RTL8367B_SYS_TIME_OFFSET_TUNE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicEavSystemTimeTransmit
 * Description:
 *      Get EAV system timer transmit
 * Input:
 *      pTransmit - transmit value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      If this transmit is set/clear which means there is 512NS carry event
 */
int rtl8367b_getAsicEavSystemTimeTransmit(unsigned int* pTransmit)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_SYS_TIME_SEC_TRANSIT, pTransmit);
}
/* Function Name:
 *      rtl8367b_getAsicEavSystemTimeSeccond
 * Description:
 *      Get EAV system second time
 * Input:
 *      pSecond - seconds
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicEavSystemTimeSeccond(unsigned int* pSecond)
{
	int   retVal;
	unsigned int 	regData, regData2;

	retVal    = rtl8367b_getAsicReg(RTL8367B_REG_SYS_TIME_SEC_HIGH_L, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	retVal    = rtl8367b_getAsicReg(RTL8367B_REG_SYS_TIME_SEC_HIGH_H, &regData2);
	if(retVal != RT_ERR_OK)
		return retVal;

	*pSecond = (regData2 << 16) | regData;
	
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicEavSystemTime512ns
 * Description:
 *      Get EAV system 512 nano-second time
 * Input:
 *      pNs 	- 512 nano-second
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicEavSystemTime512ns(unsigned int* pNs)
{
	int   retVal;
	unsigned int 	regData, regData2;

	retVal    = rtl8367b_getAsicReg(RTL8367B_REG_SYS_TIME_512NS_L, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	retVal    = rtl8367b_getAsicRegBits(RTL8367B_REG_SYS_TIME_512NS_H, RTL8367B_SYS_TIME_512NS_H_MASK, &regData2);
	if(retVal != RT_ERR_OK)
		return retVal;

	*pNs = (regData2 << 16) | regData;
	
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicEavTimeSyncEn
 * Description:
 *      Set per-port EAV timing synchronization enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      If EAV EAV timing synchronization enabled, switch will add timestamp info for ingress and egress PTP message
 */
int rtl8367b_setAsicEavTimeSyncEn(unsigned int port, unsigned int enabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;
	
    return rtl8367b_setAsicRegBit(RTL8367B_REG_PTP_PORT0_CFG1 + (port * 0x20), RTL8367B_PTP_PORT0_CFG1_TIME_SYNC_EN_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicEavTimeSyncEn
 * Description:
 *      Get per-port EAV timing synchronization enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicEavTimeSyncEn(unsigned int port, unsigned int *pEnabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_REG_PTP_PORT0_CFG1 + (port * 0x20), RTL8367B_PTP_PORT0_CFG1_TIME_SYNC_EN_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367b_setAsicEavTimeStampFillEn
 * Description:
 *      Set per-port EAV time stamp filling to PTP message enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      If EAV EAV timing synchronization enabled, switch will add timestamp info for ingress and egress PTP message
 */
int rtl8367b_setAsicEavTimeStampFillEn(unsigned int port, unsigned int enabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;
	
    return rtl8367b_setAsicRegBit(RTL8367B_REG_PTP_PORT0_CFG1 + (port * 0x20), RTL8367B_PTP_PORT0_CFG1_TIME_SYNC_TX_TS_FILL_EN_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicEavTimeStampFillEn
 * Description:
 *      Get per-port EAV time stamp filling to PTP message enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicEavTimeStampFillEn(unsigned int port, unsigned int *pEnabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_REG_PTP_PORT0_CFG1 + (port * 0x20), RTL8367B_PTP_PORT0_CFG1_TIME_SYNC_TX_TS_FILL_EN_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_getAsicEavTimeSyncValid
 * Description:
 *      Get per-port EAV timing synchronization valid state
 * Input:
 *      port 		- Physical port number (0~7)
 *      pValid		- 1: valid, 0: invalid
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicEavTimeSyncValid(unsigned int port, unsigned int *pValid)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_REG_PTP_PORT0_CFG1 + (port * 0x20), RTL8367B_PTP_PORT0_CFG1_EGRESS_TS_VALID_OFFSET, pValid);
}
/* Function Name:
 *      rtl8367b_getAsicEavEgressTimestamp512ns
 * Description:
 *      Get EAV egress time stamp 512 nano-second
 * Input:
 *      port 		- Physical port number (0~7)
 *      ns 			- 512 nano-second
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicEavEgressTimestamp512ns(unsigned int port, unsigned int* pNs)
{
	int   retVal;
	unsigned int 	regData, regData2;

	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	retVal = rtl8367b_getAsicReg(RTL8367B_REG_PTP_PORT0_CFG0 + (port * 0x20), &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_PTP_PORT0_CFG1 + (port * 0x20), RTL8367B_PTP_PORT0_CFG1_EGRESS_TS_512NS_MASK, &regData2);
	if(retVal != RT_ERR_OK)
		return retVal;

	*pNs = (regData2 << 16) | regData;
	
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicEavEgressTimestampSeccond
 * Description:
 *      Get EAV system timer transmit
 * Input:
 *      port 		- Physical port number (0~7)
 *      pSecond 	- second LSB 8-bits value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      If this transmit is set/clear which means there is 512NS carry event
 */
int rtl8367b_getAsicEavEgressTimestampSeccond(unsigned int port, unsigned int* pSecond)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_REG_PTP_PORT0_CFG1 + (port * 0x20), RTL8367B_PTP_PORT0_CFG1_TIME_TS_SECOND_MASK, pSecond 	);
}

