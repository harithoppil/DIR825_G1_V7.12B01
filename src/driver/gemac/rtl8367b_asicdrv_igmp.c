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
 * Feature : IGMP related functions
 *
 */
#include "rtl8367b_asicdrv_igmp.h"
/* Function Name:
 *      rtl8367b_setAsicIgmp
 * Description:
 *      Set IGMP/MLD state
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIgmp(unsigned int enabled)
{
    int retVal;

    /* Enable/Disable H/W IGMP/MLD */
    retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_IGMP_MLD_EN_OFFSET, enabled);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicIgmp
 * Description:
 *      Get IGMP/MLD state
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIgmp(unsigned int *ptr_enabled)
{
    int retVal;

    retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_IGMP_MLD_EN_OFFSET, ptr_enabled);
    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicIpMulticastVlanLeaky
 * Description:
 *      Set IP multicast VLAN Leaky function
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
 *      When enabling this function, 
 *    	if the lookup result(forwarding portmap) of IP Multicast packet is over VLAN boundary, 
 *    	the packet can be forwarded across VLAN
 */
int rtl8367b_setAsicIpMulticastVlanLeaky(unsigned int port, unsigned int enabled)
{
    int  retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_IPMCAST_VLAN_LEAKY, port, enabled);
    
    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicIpMulticastVlanLeaky
 * Description:
 *      Get IP multicast VLAN Leaky function
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
 *      None
 */
int rtl8367b_getAsicIpMulticastVlanLeaky(unsigned int port, unsigned int *ptr_enabled)
{
    int  retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_IPMCAST_VLAN_LEAKY, port, ptr_enabled);

    return retVal;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPTableFullOP
 * Description:
 *      Set Table Full operation
 * Input:
 *      operation   - The operation should be taken when the IGMP table is full.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		    - Success
 *      RT_ERR_SMI  	    - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter is out of range
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPTableFullOP(unsigned int operation)
{
    int  retVal;

    if(operation >= TABLE_FULL_OP_END)
        return RT_ERR_OUT_OF_RANGE;

    /* Table full Operation */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG1, RTL8367B_TABLE_FULL_OP_MASK, operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPTableFullOP
 * Description:
 *      Get Table Full operation
 * Input:
 *      None
 * Output:
 *      poperation  - The operation should be taken when the IGMP table is full.
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPTableFullOP(unsigned int *poperation)
{
    int   retVal;
    unsigned int  value;

    /* Table full Operation */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG1, RTL8367B_TABLE_FULL_OP_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *poperation = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPCRCErrOP
 * Description:
 *      Set the operation when ASIC receive a Checksum error packet
 * Input:
 *      operation   -The operation when ASIC receive a Checksum error packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		    - Success
 *      RT_ERR_SMI  	    - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter is out of range
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPCRCErrOP(unsigned int operation)
{
    int  retVal;

    if(operation >= CRC_ERR_OP_END)
        return RT_ERR_OUT_OF_RANGE;

    /* CRC Error Operation */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_CKS_ERR_OP_MASK, operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPCRCErrOP
 * Description:
 *      Get the operation when ASIC receive a Checksum error packet
 * Input:
 *      None
 * Output:
 *      poperation  - The operation of Checksum error packet
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPCRCErrOP(unsigned int *poperation)
{
    int   retVal;
    unsigned int  value;

    /* CRC Error Operation */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_CKS_ERR_OP_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *poperation = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPFastLeaveEn
 * Description:
 *      Enable/Disable Fast Leave
 * Input:
 *      enabled - 1:enable Fast Leave; 0:disable Fast Leave
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPFastLeaveEn(unsigned int enabled)
{
    int  retVal;

    /* Fast Leave */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_FAST_LEAVE_EN_MASK, (enabled >= 1) ? 1 : 0);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPFastLeaveEn
 * Description:
 *      Get Fast Leave state
 * Input:
 *      None
 * Output:
 *      penabled        - 1:enable Fast Leave; 0:disable Fast Leave
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPFastLeaveEn(unsigned int *penabled)
{
    int   retVal;
    unsigned int  value;

    /* Fast Leave */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_FAST_LEAVE_EN_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *penabled = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPLeaveTimer
 * Description:
 *      Set the Leave timer of IGMP/MLD
 * Input:
 *      leave_timer     - Leave timer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		    - Success
 *      RT_ERR_SMI  	    - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter is out of range
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPLeaveTimer(unsigned int leave_timer)
{
    int  retVal;

    if(leave_timer > RTL8367B_MAX_LEAVE_TIMER)
        return RT_ERR_OUT_OF_RANGE;

    /* Leave timer */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_LEAVE_TIMER_MASK, leave_timer);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPLeaveTimer
 * Description:
 *      Get the Leave timer of IGMP/MLD
 * Input:
 *      None
 * Output:
 *      pleave_timer    - Leave timer
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPLeaveTimer(unsigned int *pleave_timer)
{
    int   retVal;
    unsigned int  value;

    /* Leave timer */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_LEAVE_TIMER_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pleave_timer = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPQueryInterval
 * Description:
 *      Set Query Interval of IGMP/MLD
 * Input:
 *      interval    - Query Interval
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		    - Success
 *      RT_ERR_SMI  	    - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter is out of range
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPQueryInterval(unsigned int interval)
{
    int  retVal;

    if(interval > RTL8367B_MAX_QUERY_INT)
        return RT_ERR_OUT_OF_RANGE;

    /* Query Interval */
    retVal = rtl8367b_setAsicReg(RTL8367B_REG_IGMP_MLD_CFG2, interval);
    if(retVal != RT_ERR_OK)
        return retVal;
        
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPQueryInterval
 * Description:
 *      Get Query Interval of IGMP/MLD
 * Input:
 *      None
 * Output:
 *      pinterval       - Query Interval
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPQueryInterval(unsigned int *pinterval)
{
    int   retVal;
    unsigned int  value;

    /* Query Interval */
    retVal = rtl8367b_getAsicReg(RTL8367B_REG_IGMP_MLD_CFG2, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pinterval = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPRobVar
 * Description:
 *      Set Robustness Variable of IGMP/MLD
 * Input:
 *      rob_var     - Robustness Variable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		    - Success
 *      RT_ERR_SMI  	    - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter is out of range
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPRobVar(unsigned int rob_var)
{
    int  retVal;

    if(rob_var > RTL8367B_MAX_ROB_VAR)
        return RT_ERR_OUT_OF_RANGE;

    /* Bourstness variable */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_ROBURSTNESS_VAR_MASK, rob_var);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPRobVar
 * Description:
 *      Get Robustness Variable of IGMP/MLD
 * Input:
 *      none
 * Output:
 *      prob_var     - Robustness Variable
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPRobVar(unsigned int *prob_var)
{
    int   retVal;
    unsigned int  value;

    /* Bourstness variable */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_ROBURSTNESS_VAR_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *prob_var = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPStaticRouterPort
 * Description:
 *      Set IGMP static router port mask
 * Input:
 *      pmsk 	- Static portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_MASK  	- Invalid port mask
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPStaticRouterPort(unsigned int pmsk)
{
    if(pmsk > RTL8367B_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_STATIC_ROUTER_PORT, RTL8367B_IGMP_STATIC_ROUTER_PORT_MASK, pmsk);
}

/* Function Name:
 *      rtl8367b_getAsicIGMPStaticRouterPort
 * Description:
 *      Get IGMP static router port mask
 * Input:
 *      pmsk 	- Static portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPStaticRouterPort(unsigned int *pmsk)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_STATIC_ROUTER_PORT, RTL8367B_IGMP_STATIC_ROUTER_PORT_MASK, pmsk);
}

/* Function Name:
 *      rtl8367b_getAsicIGMPdynamicRouterPort1
 * Description:
 *      Get 1st dynamic router port and timer
 * Input:
 *      port 	- Physical port number (0~7)
 *      timer 	- router port timer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPdynamicRouterPort1(unsigned int *port, unsigned int *timer)
{
    int   retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_DYNAMIC_ROUTER_PORT, RTL8367B_D_ROUTER_PORT_1_MASK, port);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_DYNAMIC_ROUTER_PORT, RTL8367B_D_ROUTER_PORT_TMR_1_MASK, timer);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPdynamicRouterPort2
 * Description:
 *      Get 2nd dynamic router port and timer
 * Input:
 *      port 	- Physical port number (0~7)
 *      timer 	- router port timer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPdynamicRouterPort2(unsigned int *port, unsigned int *timer)
{
    int   retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_DYNAMIC_ROUTER_PORT, RTL8367B_D_ROUTER_PORT_2_MASK, port);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_DYNAMIC_ROUTER_PORT, RTL8367B_D_ROUTER_PORT_TMR_2_MASK, timer);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPSuppression
 * Description:
 *      Set the suppression function
 * Input:
 *      report_supp_enabled 	- Report suppression, 1:Enable, 0:disable
 *      leave_supp_enabled 		- Leave suppression, 1:Enable, 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPSuppression(unsigned int report_supp_enabled, unsigned int leave_supp_enabled)
{
    int   retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_REPORT_SUPPRESSION_MASK, report_supp_enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_LEAVE_SUPPRESSION_MASK, leave_supp_enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPSuppression
 * Description:
 *      Get the suppression function
 * Input:
 *      report_supp_enabled 	- Report suppression, 1:Enable, 0:disable
 *      leave_supp_enabled 		- Leave suppression, 1:Enable, 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPSuppression(unsigned int *report_supp_enabled, unsigned int *leave_supp_enabled)
{
    int   retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_REPORT_SUPPRESSION_MASK, report_supp_enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_LEAVE_SUPPRESSION_MASK, leave_supp_enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPQueryRX
 * Description:
 *      Set port-based Query packet RX allowance
 * Input:
 *      port            - port number
 *      allow_query     - allowance of Query packet RX, 1:Allow, 0:Drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPQueryRX(unsigned int port, unsigned int allow_query)
{
    int   retVal;

    if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    /* Allow Query */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_ALLOW_QUERY_MASK, allow_query);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPQueryRX
 * Description:
 *      Get port-based Query packet RX allowance
 * Input:
 *      port            - port number
 * Output:
 *      allow_query     - allowance of Query packet RX, 1:Allow, 0:Drop
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPQueryRX(unsigned int port, unsigned int *allow_query)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Query */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_ALLOW_QUERY_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *allow_query = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPReportRX
 * Description:
 *      Set port-based Report packet RX allowance
 * Input:
 *      port            - port number
 *      allow_report    - allowance of Report packet RX, 1:Allow, 0:Drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPReportRX(unsigned int port, unsigned int allow_report)
{
    int   retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Report */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_ALLOW_REPORT_MASK, allow_report);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPReportRX
 * Description:
 *      Get port-based Report packet RX allowance
 * Input:
 *      port            - port number
 * Output:
 *      allow_report    - allowance of Report packet RX, 1:Allow, 0:Drop
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPReportRX(unsigned int port, unsigned int *allow_report)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Report */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_ALLOW_REPORT_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *allow_report = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPLeaveRX
 * Description:
 *      Set port-based Leave packet RX allowance
 * Input:
 *      port            - port number
 *      allow_leave     - allowance of Leave packet RX, 1:Allow, 0:Drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPLeaveRX(unsigned int port, unsigned int allow_leave)
{
    int   retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Leave */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_ALLOW_LEAVE_MASK, allow_leave);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPLeaveRX
 * Description:
 *      Get port-based Leave packet RX allowance
 * Input:
 *      port            - port number
 * Output:
 *      allow_leave     - allowance of Leave packet RX, 1:Allow, 0:Drop
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPLeaveRX(unsigned int port, unsigned int *allow_leave)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Leave */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_ALLOW_LEAVE_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *allow_leave = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPMRPRX
 * Description:
 *      Set port-based Multicast Routing Protocol packet RX allowance
 * Input:
 *      port            - port number
 *      allow_mrp       - allowance of Multicast Routing Protocol packet RX, 1:Allow, 0:Drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPMRPRX(unsigned int port, unsigned int allow_mrp)
{
    int   retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Multicast Routing Protocol */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_ALLOW_MRP_MASK, allow_mrp);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPMRPRX
 * Description:
 *      Get port-based Multicast Routing Protocol packet RX allowance
 * Input:
 *      port            - port number
 * Output:
 *      allow_mrp       - allowance of Multicast Routing Protocol packet RX, 1:Allow, 0:Drop
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPMRPRX(unsigned int port, unsigned int *allow_mrp)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Multicast Routing Protocol */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_ALLOW_MRP_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *allow_mrp = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPMcDataRX
 * Description:
 *      Set port-based Multicast data packet RX allowance
 * Input:
 *      port            - port number
 *      allow_mcdata    - allowance of Multicast data packet RX, 1:Allow, 0:Drop
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPMcDataRX(unsigned int port, unsigned int allow_mcdata)
{
    int   retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Multicast Data */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_ALLOW_MC_DATA_MASK, allow_mcdata);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPMcDataRX
 * Description:
 *      Get port-based Multicast data packet RX allowance
 * Input:
 *      port            - port number
 * Output:
 *      allow_mcdata    - allowance of Multicast data packet RX, 1:Allow, 0:Drop
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPMcDataRX(unsigned int port, unsigned int *allow_mcdata)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Multicast data */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_ALLOW_MC_DATA_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *allow_mcdata = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPv1Opeartion
 * Description:
 *      Set port-based IGMPv1 Control packet action
 * Input:
 *      port            - port number
 *      igmpv1_op       - IGMPv1 control packet action 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPv1Opeartion(unsigned int port, unsigned int igmpv1_op)
{
    int   retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* IGMPv1 operation */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_IGMPV1_OP_MASK, igmpv1_op);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPv1Opeartion
 * Description:
 *      Get port-based IGMPv1 Control packet action
 * Input:
 *      port            - port number
 * Output:
 *      igmpv1_op       - IGMPv1 control packet action 
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPv1Opeartion(unsigned int port, unsigned int *igmpv1_op)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* IGMPv1 operation */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_IGMPV1_OP_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *igmpv1_op = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPv2Opeartion
 * Description:
 *      Set port-based IGMPv2 Control packet action
 * Input:
 *      port            - port number
 *      igmpv2_op       - IGMPv2 control packet action 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPv2Opeartion(unsigned int port, unsigned int igmpv2_op)
{
    int   retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* IGMPv2 operation */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_IGMPV2_OP_MASK, igmpv2_op);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPv2Opeartion
 * Description:
 *      Get port-based IGMPv2 Control packet action
 * Input:
 *      port            - port number
 * Output:
 *      igmpv2_op       - IGMPv2 control packet action 
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPv2Opeartion(unsigned int port, unsigned int *igmpv2_op)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* IGMPv2 operation */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_IGMPV2_OP_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *igmpv2_op = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPv3Opeartion
 * Description:
 *      Set port-based IGMPv3 Control packet action
 * Input:
 *      port            - port number
 *      igmpv3_op       - IGMPv3 control packet action 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPv3Opeartion(unsigned int port, unsigned int igmpv3_op)
{
    int   retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* IGMPv3 operation */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_IGMPV3_OP_MASK, igmpv3_op);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPv3Opeartion
 * Description:
 *      Get port-based IGMPv3 Control packet action
 * Input:
 *      port            - port number
 * Output:
 *      igmpv3_op       - IGMPv3 control packet action
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPv3Opeartion(unsigned int port, unsigned int *igmpv3_op)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* IGMPv3 operation */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_IGMPV3_OP_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *igmpv3_op = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicMLDv1Opeartion
 * Description:
 *      Set port-based MLDv1 Control packet action
 * Input:
 *      port            - port number
 *      mldv1_op        - MLDv1 control packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicMLDv1Opeartion(unsigned int port, unsigned int mldv1_op)
{
    int   retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* MLDv1 operation */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_MLDv1_OP_MASK, mldv1_op);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicMLDv1Opeartion
 * Description:
 *      Get port-based MLDv1 Control packet action
 * Input:
 *      port            - port number
 * Output:
 *      mldv1_op        - MLDv1 control packet action
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicMLDv1Opeartion(unsigned int port, unsigned int *mldv1_op)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* MLDv1 operation */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_MLDv1_OP_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *mldv1_op = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicMLDv2Opeartion
 * Description:
 *      Set port-based MLDv2 Control packet action
 * Input:
 *      port            - port number
 *      mldv2_op        - MLDv2 control packet action
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicMLDv2Opeartion(unsigned int port, unsigned int mldv2_op)
{
    int   retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* MLDv2 operation */
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_MLDv2_OP_MASK, mldv2_op);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicMLDv2Opeartion
 * Description:
 *      Get port-based MLDv2 Control packet action
 * Input:
 *      port            - port number
 * Output:
 *      mldv2_op        - MLDv2 control packet action
 * Return:
 *      RT_ERR_OK 	    - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicMLDv2Opeartion(unsigned int port, unsigned int *mldv2_op)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* MLDv2 operation */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT0_CONTROL + port, RTL8367B_IGMP_PORT0_CONTROL_MLDv2_OP_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *mldv2_op = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPPortMAXGroup
 * Description:
 *      Set per-port Max group number
 * Input:
 *      port 		- Physical port number (0~7)
 *      max_group 	- max IGMP group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPPortMAXGroup(unsigned int port, unsigned int max_group)
{
    if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    if(max_group > RTL8367B_IGMP_MAX_GOUP)
		return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_IGMP_PORT01_MAX_GROUP + (port/2), RTL8367B_PORT0_MAX_GROUP_MASK << (RTL8367B_PORT1_MAX_GROUP_OFFSET * (port%2)), max_group);
}
/* Function Name:
 *      rtl8367b_getAsicIGMPPortMAXGroup
 * Description:
 *      Get per-port Max group number
 * Input:
 *      port 		- Physical port number (0~7)
 *      max_group 	- max IGMP group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPPortMAXGroup(unsigned int port, unsigned int *max_group)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT01_MAX_GROUP + (port/2), RTL8367B_PORT0_MAX_GROUP_MASK << (RTL8367B_PORT1_MAX_GROUP_OFFSET * (port%2)), &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *max_group = value;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicIGMPPortCurrentGroup
 * Description:
 *      Get per-port current group number
 * Input:
 *      port 			- Physical port number (0~7)
 *      current_group 	- current IGMP group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPPortCurrentGroup(unsigned int port, unsigned int *current_group)
{
    int   retVal;
    unsigned int  value;

    if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_IGMP_PORT01_CURRENT_GROUP + (port/2), RTL8367B_PORT0_CURRENT_GROUP_MASK << (RTL8367B_PORT1_CURRENT_GROUP_OFFSET * (port%2)), &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *current_group = value;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicIGMPGroup
 * Description:
 *      Get IGMP group
 * Input:
 *      idx 	- Group index (0~255)
 *      valid 	- valid bit
 *      grp 	- IGMP group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE		- Group index is out of range
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPGroup(unsigned int idx, unsigned int *valid, rtl8367b_igmpgroup *grp)
{
    int   retVal;
    unsigned int  regAddr, regData;
    unsigned short* tableAddr;
    unsigned int  i;

    if(idx > RTL8367B_IGMP_MAX_GOUP)
        return RT_ERR_OUT_OF_RANGE;

    /* Write ACS_ADR register for data bits */
	regAddr = RTL8367B_TABLE_ACCESS_ADDR_REG;
	regData = idx;
	retVal = rtl8367b_setAsicReg(regAddr, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

    /* Write ACS_CMD register */
	regAddr = RTL8367B_TABLE_ACCESS_CTRL_REG;
	regData = RTL8367B_TABLE_ACCESS_REG_DATA(TB_OP_READ, TB_TARGET_IGMP_GROUP);
	retVal = rtl8367b_setAsicRegBits(regAddr, RTL8367B_TABLE_TYPE_MASK | RTL8367B_COMMAND_TYPE_MASK, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

    /* Read Data Bits */
	regAddr = RTL8367B_TABLE_ACCESS_RDDATA_BASE;
	tableAddr = (unsigned short*)grp;
	for(i=0;i<RTL8367B_IGMP_GRP_BLEN;i++)
	{
		retVal = rtl8367b_getAsicReg(regAddr, &regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		*tableAddr = regData;
		
		regAddr ++;
		tableAddr ++;
	}

    /* Valid bit */
    retVal = rtl8367b_getAsicReg(RTL8367B_IGMP_GROUP_USAGE_REG(idx), &regData);
    if(retVal != RT_ERR_OK)
		return retVal;

    *valid = ((regData & (0x0001 << (idx %16))) != 0) ? 1 : 0;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicIpMulticastPortIsoLeaky
 * Description:
 *      Set IP multicast Port Isolation leaky
 * Input:
 *      port 		- Physical port number (0~7)
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID	- Invalid port number
 * Note:
 *      None
 */
int rtl8367b_setAsicIpMulticastPortIsoLeaky(unsigned int port, unsigned int enabled)
{
    int   retVal;

    if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_IPMCAST_PORTISO_LEAKY_REG, (0x0001 << port), enabled);
    if(retVal != RT_ERR_OK)
		return retVal;

	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIpMulticastPortIsoLeaky
 * Description:
 *      Get IP multicast Port Isolation leaky
 * Input:
 *      port 		- Physical port number (0~7)
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIpMulticastPortIsoLeaky(unsigned int port, unsigned int *enabled)
{
    int   retVal;
    unsigned int  regData;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_IPMCAST_PORTISO_LEAKY_REG, (0x0001 << port), &regData);
    if(retVal != RT_ERR_OK)
		return retVal;

    *enabled = regData;
	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPReportFlood
 * Description:
 *      Set IGMPv1/v2 MLD v1 Report flood
 * Input:
 *      flood 	- 1: flooding to all port, 0: forward to router ports only
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPReportFlood(unsigned int flood)
{
    int   retVal;

    retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_REPORT_FORWARD_OFFSET, flood);
    if(retVal != RT_ERR_OK)
		return retVal;

	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPReportFlood
 * Description:
 *      Get IGMPv1/v2 MLD v1 Report flood
 * Input:
 *      None
 * Output:
 *      pflood 	- 1: flooding to all port, 0: forward to router ports only
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPReportFlood(unsigned int *pFlood)
{
    int   retVal;
    unsigned int  regData;

    retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_REPORT_FORWARD_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
		return retVal;

    *pFlood = regData;
	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPDropLeaveZero
 * Description:
 *      Set the function of droppping Leave packet with group IP = 0.0.0.0
 * Input:
 *      drop    - 1: Drop, 0:Bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPDropLeaveZero(unsigned int drop)
{
    int   retVal;

    retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG1, RTL8367B_DROP_LEAVE_ZERO_OFFSET, drop);
    if(retVal != RT_ERR_OK)
		return retVal;

	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPDropLeaveZero
 * Description:
 *      Get the function of droppping Leave packet with group IP = 0.0.0.0
 * Input:
 *      None
 * Output:
 *      pDrop    - 1: Drop, 0:Bypass
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPDropLeaveZero(unsigned int *pDrop)
{
    int   retVal;
    unsigned int  regData;

    retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG1, RTL8367B_DROP_LEAVE_ZERO_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
		return retVal;

    *pDrop = regData;
	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPBypassStormCTRL
 * Description:
 *      Set the function of bypass strom control for IGMP/MLD packet
 * Input:
 *      bypass    - 1: Bypass, 0:not bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPBypassStormCTRL(unsigned int bypass)
{
    int   retVal;

    retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_IGMP_MLD_DISCARD_STORM_FILTER_OFFSET, bypass);
    if(retVal != RT_ERR_OK)
		return retVal;

	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPBypassStormCTRL
 * Description:
 *      Set the function of bypass strom control for IGMP/MLD packet
 * Input:
 *      None
 * Output:
 *      pBypass    - 1: Bypass, 0:not bypass
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPBypassStormCTRL(unsigned int *pBypass)
{
    int   retVal;
    unsigned int  regData;

    retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_IGMP_MLD_DISCARD_STORM_FILTER_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
		return retVal;

    *pBypass = regData;
	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPIsoLeaky
 * Description:
 *      Set Port Isolation leaky for IGMP/MLD packet
 * Input:
 *      leaky    - 1: Leaky, 0:not leaky
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPIsoLeaky(unsigned int leaky)
{
    int   retVal;

    retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_IGMP_MLD_PORTISO_LEAKY_OFFSET, leaky);
    if(retVal != RT_ERR_OK)
		return retVal;

	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPIsoLeaky
 * Description:
 *      Get Port Isolation leaky for IGMP/MLD packet
 * Input:
 *      Noen
 * Output:
 *      pLeaky    - 1: Leaky, 0:not leaky
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPIsoLeaky(unsigned int *pLeaky)
{
    int   retVal;
    unsigned int  regData;

    retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_IGMP_MLD_PORTISO_LEAKY_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
		return retVal;

    *pLeaky = regData;
	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicIGMPVLANLeaky
 * Description:
 *      Set VLAN leaky for IGMP/MLD packet
 * Input:
 *      leaky    - 1: Leaky, 0:not leaky
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_setAsicIGMPVLANLeaky(unsigned int leaky)
{
    int   retVal;

    retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_IGMP_MLD_VLAN_LEAKY_OFFSET, leaky);
    if(retVal != RT_ERR_OK)
		return retVal;

	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicIGMPVLANLeaky
 * Description:
 *      Get VLAN leaky for IGMP/MLD packet
 * Input:
 *      Noen
 * Output:
 *      pLeaky    - 1: Leaky, 0:not leaky
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
int rtl8367b_getAsicIGMPVLANLeaky(unsigned int *pLeaky)
{
    int   retVal;
    unsigned int  regData;

    retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_IGMP_MLD_CFG0, RTL8367B_IGMP_MLD_VLAN_LEAKY_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
		return retVal;

    *pLeaky = regData;
	return RT_ERR_OK;
}
