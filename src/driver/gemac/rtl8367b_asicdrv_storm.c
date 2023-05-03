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
 * Feature : Storm control filtering related functions
 *
 */

#include "rtl8367b_asicdrv_storm.h"
/* Function Name:
 *      rtl8367b_setAsicStormFilterBroadcastEnable
 * Description:
 *      Set per-port broadcast storm filter enable/disable
 * Input:
 *      port 	- Physical port number (0~7)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_setAsicStormFilterBroadcastEnable(unsigned int port, unsigned int enabled)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;
	
    return rtl8367b_setAsicRegBit(RTL8367B_STORM_BCAST_REG, port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicStormFilterBroadcastEnable
 * Description:
 *      Get per-port broadcast storm filter enable/disable
 * Input:
 *      port 	- Physical port number (0~7)
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicStormFilterBroadcastEnable(unsigned int port, unsigned int *pEnabled)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_STORM_BCAST_REG, port, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicStormFilterBroadcastMeter
 * Description:
 *      Set per-port broadcast storm filter meter
 * Input:
 *      port 	- Physical port number (0~7)
 *      meter 	- meter index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  		- Invalid port number
 *      RT_ERR_FILTER_METER_ID  - Invalid meter index
 * Note:
 *      None
 */
int rtl8367b_setAsicStormFilterBroadcastMeter(unsigned int port, unsigned int meter)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    if(meter > RTL8367B_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8367b_setAsicRegBits(RTL8367B_STORM_BCAST_METER_CTRL_REG(port), RTL8367B_STORM_BCAST_METER_CTRL_MASK(port), meter);
}
/* Function Name:
 *      rtl8367b_getAsicStormFilterBroadcastMeter
 * Description:
 *      Get per-port broadcast storm filter meter
 * Input:
 *      port 	- Physical port number (0~7)
 *      pMeter 	- meter index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicStormFilterBroadcastMeter(unsigned int port, unsigned int *pMeter)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_STORM_BCAST_METER_CTRL_REG(port), RTL8367B_STORM_BCAST_METER_CTRL_MASK(port), pMeter);
}
/* Function Name:
 *      rtl8367b_setAsicStormFilterMulticastEnable
 * Description:
 *		Set per-port multicast storm filter enable/disable
 * Input:
 *      port 	- Physical port number (0~7)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_setAsicStormFilterMulticastEnable(unsigned int port, unsigned int enabled)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_setAsicRegBit(RTL8367B_STORM_MCAST_REG, port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicStormFilterMulticastEnable
 * Description:
 *		Get per-port multicast storm filter enable/disable
 * Input:
 *      port 	- Physical port number (0~7)
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicStormFilterMulticastEnable(unsigned int port, unsigned int *pEnabled)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_STORM_MCAST_REG, port, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicStormFilterMulticastMeter
 * Description:
 *      Set per-port multicast storm filter meter
 * Input:
 *      port 	- Physical port number (0~7)
 *      meter 	- meter index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  		- Invalid port number
 *      RT_ERR_FILTER_METER_ID  - Invalid meter index
 * Note:
 *      None
 */
int rtl8367b_setAsicStormFilterMulticastMeter(unsigned int port, unsigned int meter)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    if(meter > RTL8367B_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8367b_setAsicRegBits(RTL8367B_STORM_MCAST_METER_CTRL_REG(port), RTL8367B_STORM_MCAST_METER_CTRL_MASK(port), meter);
}
/* Function Name:
 *      rtl8367b_getAsicStormFilterMulticastMeter
 * Description:
 *      Get per-port multicast storm filter meter
 * Input:
 *      port 	- Physical port number (0~7)
 *      pMeter 	- meter index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicStormFilterMulticastMeter(unsigned int port, unsigned int *pMeter)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_STORM_MCAST_METER_CTRL_REG(port), RTL8367B_STORM_MCAST_METER_CTRL_MASK(port), pMeter);
}
/* Function Name:
 *      rtl8367b_setAsicStormFilterUnknownMulticastEnable
 * Description:
 *      Set per-port unknown multicast storm filter enable/disable
 * Input:
 *      port 	- Physical port number (0~7)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_setAsicStormFilterUnknownMulticastEnable(unsigned int port, unsigned int enabled)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;
	
    return rtl8367b_setAsicRegBit(RTL8367B_STORM_UNKNOWN_MCAST_REG, port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicStormFilterUnknownMulticastEnable
 * Description:
 *      Get per-port unknown multicast storm filter enable/disable
 * Input:
 *      port 	- Physical port number (0~7)
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicStormFilterUnknownMulticastEnable(unsigned int port, unsigned int *pEnabled)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_STORM_UNKNOWN_MCAST_REG, port, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicStormFilterUnknownMulticastMeter
 * Description:
 *      Set per-port unknown multicast storm filter meter
 * Input:
 *      port 	- Physical port number (0~7)
 *      meter 	- meter index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  		- Invalid port number
 *      RT_ERR_FILTER_METER_ID  - Invalid meter index
 * Note:
 *      None
 */
int rtl8367b_setAsicStormFilterUnknownMulticastMeter(unsigned int port, unsigned int meter)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    if(meter > RTL8367B_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8367b_setAsicRegBits(RTL8367B_STORM_UNMC_METER_CTRL_REG(port), RTL8367B_STORM_UNMC_METER_CTRL_MASK(port), meter);
}
/* Function Name:
 *      rtl8367b_getAsicStormFilterUnknownMulticastMeter
 * Description:
 *      Get per-port unknown multicast storm filter meter
 * Input:
 *      port 	- Physical port number (0~7)
 *      pMeter 	- meter index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicStormFilterUnknownMulticastMeter(unsigned int port, unsigned int *pMeter)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_STORM_UNMC_METER_CTRL_REG(port), RTL8367B_STORM_UNMC_METER_CTRL_MASK(port), pMeter);
}
/* Function Name:
 *      rtl8367b_setAsicStormFilterUnknownUnicastEnable
 * Description:
 *      Set per-port unknown unicast storm filter enable/disable
 * Input:
 *      port 	- Physical port number (0~7)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_setAsicStormFilterUnknownUnicastEnable(unsigned int port, unsigned int enabled)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_setAsicRegBit(RTL8367B_STORM_UNKNOWN_UCAST_REG, port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicStormFilterUnknownUnicastEnable
 * Description:
 *      get per-port unknown unicast storm filter enable/disable
 * Input:
 *      port 	- Physical port number (0~7)
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicStormFilterUnknownUnicastEnable(unsigned int port, unsigned int *pEnabled)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_STORM_UNKNOWN_UCAST_REG, port, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicStormFilterUnknownUnicastMeter
 * Description:
 *      Set per-port unknown unicast storm filter meter
 * Input:
 *      port 	- Physical port number (0~7)
 *      meter 	- meter index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  		- Invalid port number
 *      RT_ERR_FILTER_METER_ID  - Invalid meter index
 * Note:
 *      None
 */
int rtl8367b_setAsicStormFilterUnknownUnicastMeter(unsigned int port, unsigned int meter)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    if(meter > RTL8367B_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8367b_setAsicRegBits(RTL8367B_STORM_UNDA_METER_CTRL_REG(port), RTL8367B_STORM_UNDA_METER_CTRL_MASK(port), meter);
}
/* Function Name:
 *      rtl8367b_getAsicStormFilterUnknownUnicastMeter
 * Description:
 *      Get per-port unknown unicast storm filter meter
 * Input:
 *      port 	- Physical port number (0~7)
 *      pMeter 	- meter index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicStormFilterUnknownUnicastMeter(unsigned int port, unsigned int *pMeter)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_STORM_UNDA_METER_CTRL_REG(port), RTL8367B_STORM_UNDA_METER_CTRL_MASK(port), pMeter);
}

