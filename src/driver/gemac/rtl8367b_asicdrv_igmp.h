#ifndef _RTL8367B_ASICDRV_IGMP_H_
#define _RTL8367B_ASICDRV_IGMP_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include "rtl8367b_asicdrv.h"

#define RTL8367B_MAX_LEAVE_TIMER        (7)
#define RTL8367B_MAX_QUERY_INT          (0xFFFF)
#define RTL8367B_MAX_ROB_VAR            (7)

#define RTL8367B_IGMP_GOUP_NO		    (256)
#define RTL8367B_IGMP_MAX_GOUP		    (0xFF)
#define RTL8367B_IGMP_GRP_BLEN	        (2)
#define RTL8367B_ROUTER_PORT_INVALID    (0xF)

enum RTL8367B_IGMPTABLE_FULL_OP
{
    TABLE_FULL_FORWARD = 0,
    TABLE_FULL_DROP,
    TABLE_FULL_TRAP,
    TABLE_FULL_OP_END
};

enum RTL8367B_CRC_ERR_OP
{
    CRC_ERR_DROP = 0,
    CRC_ERR_TRAP,
    CRC_ERR_FORWARD,
    CRC_ERR_OP_END
};

enum RTL8367B_IGMP_MLD_PROTOCOL_OP
{
    PROTOCOL_OP_ASIC = 0,
    PROTOCOL_OP_FLOOD,
    PROTOCOL_OP_TRAP,
    PROTOCOL_OP_DROP,
    PROTOCOL_OP_END
};

typedef struct
{
#ifdef _LITTLE_ENDIAN
    unsigned int p0_timer:3;
    unsigned int p1_timer:3;
    unsigned int p2_timer:3;
    unsigned int p3_timer:3;
    unsigned int p4_timer:3;
    unsigned int p5_timer:3;
    unsigned int p6_timer:3;
    unsigned int p7_timer:3;
    unsigned int report_supp_flag:1;
    unsigned int reserved:7;
#else
    unsigned int reserved:7;
    unsigned int report_supp_flag:1;
    unsigned int p7_timer:3;
    unsigned int p6_timer:3;
    unsigned int p5_timer:3;
    unsigned int p4_timer:3;
    unsigned int p3_timer:3;
    unsigned int p2_timer:3;
    unsigned int p1_timer:3;
    unsigned int p0_timer:3;
#endif

}rtl8367b_igmpgroup;
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

int rtl8367b_setAsicIgmp(unsigned int enabled);
int rtl8367b_getAsicIgmp(unsigned int *pEnabled);
int rtl8367b_setAsicIpMulticastVlanLeaky(unsigned int port, unsigned int enabled );
int rtl8367b_getAsicIpMulticastVlanLeaky(unsigned int port, unsigned int *pEnabled );
int rtl8367b_setAsicIGMPTableFullOP(unsigned int operation);
int rtl8367b_getAsicIGMPTableFullOP(unsigned int *pOperation);
int rtl8367b_setAsicIGMPCRCErrOP(unsigned int operation);
int rtl8367b_getAsicIGMPCRCErrOP(unsigned int *pOperation);
int rtl8367b_setAsicIGMPFastLeaveEn(unsigned int enabled);
int rtl8367b_getAsicIGMPFastLeaveEn(unsigned int *pEnabled);
int rtl8367b_setAsicIGMPLeaveTimer(unsigned int leave_timer);
int rtl8367b_getAsicIGMPLeaveTimer(unsigned int *pLeave_timer);
int rtl8367b_setAsicIGMPQueryInterval(unsigned int interval);
int rtl8367b_getAsicIGMPQueryInterval(unsigned int *pInterval);
int rtl8367b_setAsicIGMPRobVar(unsigned int rob_var);
int rtl8367b_getAsicIGMPRobVar(unsigned int *pRob_var);
int rtl8367b_setAsicIGMPStaticRouterPort(unsigned int pmsk);
int rtl8367b_getAsicIGMPStaticRouterPort(unsigned int *pMsk);
int rtl8367b_getAsicIGMPdynamicRouterPort1(unsigned int *pPort, unsigned int *pTimer);
int rtl8367b_getAsicIGMPdynamicRouterPort2(unsigned int *pPort, unsigned int *pTimer);
int rtl8367b_setAsicIGMPSuppression(unsigned int report_supp_enabled, unsigned int leave_supp_enabled);
int rtl8367b_getAsicIGMPSuppression(unsigned int *pReport_supp_enabled, unsigned int *pLeave_supp_enabled);
int rtl8367b_setAsicIGMPQueryRX(unsigned int port, unsigned int allow_query);
int rtl8367b_getAsicIGMPQueryRX(unsigned int port, unsigned int *pAllow_query);
int rtl8367b_setAsicIGMPReportRX(unsigned int port, unsigned int allow_report);
int rtl8367b_getAsicIGMPReportRX(unsigned int port, unsigned int *pAllow_report);
int rtl8367b_setAsicIGMPLeaveRX(unsigned int port, unsigned int allow_leave);
int rtl8367b_getAsicIGMPLeaveRX(unsigned int port, unsigned int *pAllow_leave);
int rtl8367b_setAsicIGMPMRPRX(unsigned int port, unsigned int allow_mrp);
int rtl8367b_getAsicIGMPMRPRX(unsigned int port, unsigned int *pAllow_mrp);
int rtl8367b_setAsicIGMPMcDataRX(unsigned int port, unsigned int allow_mcdata);
int rtl8367b_getAsicIGMPMcDataRX(unsigned int port, unsigned int *pAllow_mcdata);
int rtl8367b_setAsicIGMPv1Opeartion(unsigned int port, unsigned int igmpv1_op);
int rtl8367b_getAsicIGMPv1Opeartion(unsigned int port, unsigned int *pIgmpv1_op);
int rtl8367b_setAsicIGMPv2Opeartion(unsigned int port, unsigned int igmpv2_op);
int rtl8367b_getAsicIGMPv2Opeartion(unsigned int port, unsigned int *pIgmpv2_op);
int rtl8367b_setAsicIGMPv3Opeartion(unsigned int port, unsigned int igmpv3_op);
int rtl8367b_getAsicIGMPv3Opeartion(unsigned int port, unsigned int *pIgmpv3_op);
int rtl8367b_setAsicMLDv1Opeartion(unsigned int port, unsigned int mldv1_op);
int rtl8367b_getAsicMLDv1Opeartion(unsigned int port, unsigned int *pMldv1_op);
int rtl8367b_setAsicMLDv2Opeartion(unsigned int port, unsigned int mldv2_op);
int rtl8367b_getAsicMLDv2Opeartion(unsigned int port, unsigned int *pMldv2_op);
int rtl8367b_setAsicIGMPPortMAXGroup(unsigned int port, unsigned int max_group);
int rtl8367b_getAsicIGMPPortMAXGroup(unsigned int port, unsigned int *pMax_group);
int rtl8367b_getAsicIGMPPortCurrentGroup(unsigned int port, unsigned int *pCurrent_group);
int rtl8367b_getAsicIGMPGroup(unsigned int idx, unsigned int *pValid, rtl8367b_igmpgroup *pGrp);
int rtl8367b_setAsicIpMulticastPortIsoLeaky(unsigned int port, unsigned int enabled);
int rtl8367b_getAsicIpMulticastPortIsoLeaky(unsigned int port, unsigned int *pEnabled);
int rtl8367b_setAsicIGMPReportFlood(unsigned int flood);
int rtl8367b_getAsicIGMPReportFlood(unsigned int *pFlood);
int rtl8367b_setAsicIGMPDropLeaveZero(unsigned int drop);
int rtl8367b_getAsicIGMPDropLeaveZero(unsigned int *pDrop);
int rtl8367b_setAsicIGMPBypassStormCTRL(unsigned int bypass);
int rtl8367b_getAsicIGMPBypassStormCTRL(unsigned int *pBypass);
int rtl8367b_setAsicIGMPIsoLeaky(unsigned int leaky);
int rtl8367b_getAsicIGMPIsoLeaky(unsigned int *pLeaky);
int rtl8367b_setAsicIGMPVLANLeaky(unsigned int leaky);
int rtl8367b_getAsicIGMPVLANLeaky(unsigned int *pLeaky);

#endif /*#ifndef _RTL8367B_ASICDRV_IGMP_H_*/

