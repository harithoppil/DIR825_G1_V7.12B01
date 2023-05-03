/*
 * $Copyright Open Broadcom Corporation$
 *
 * $Id: wancommon.c,v 1.26.20.2 2003/10/31 21:31:35 mthawani Exp $
 */

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "wancommon.h"

static char *WANAccessType_allowedValueList[] = { "DSL", "POTS", "Cable", "Ethernet", "Other", NULL };
static char *PhysicalLinkStatus_allowedValueList[] = { "Up", "Down", "Initializing", "Unavailable", NULL };

static VarTemplate StateVariables[] = {
    { "WANAccessType", "DSL", VAR_STRING|VAR_LIST,  (allowedValue) { WANAccessType_allowedValueList } },
    { "Layer1UpstreamMaxBitRate", "", VAR_ULONG },
    { "Layer1DownstreamMaxBitRate", "", VAR_ULONG },
    { "PhysicalLinkStatus", "", VAR_EVENTED|VAR_STRING|VAR_LIST,  (allowedValue) { PhysicalLinkStatus_allowedValueList } },
    { "EnabledForInternet", "1", VAR_EVENTED|VAR_BOOL },
    { "TotalBytesSent", "", VAR_ULONG },
    { "TotalBytesReceived", "", VAR_ULONG },
    { "TotalPacketsSent", "", VAR_ULONG },
    { "TotalPacketsReceived", "", VAR_ULONG },
    { NULL }
};

static Action _GetCommonLinkProperties = {
    "GetCommonLinkProperties", GetCommonLinkProperties,
   (Param []) {
       {"NewWANAccessType", VAR_WANAccessType, VAR_OUT},
       {"NewLayer1UpstreamMaxBitRate", VAR_Layer1UpstreamMaxBitRate, VAR_OUT},
       {"NewLayer1DownstreamMaxBitRate", VAR_Layer1DownstreamMaxBitRate, VAR_OUT},
       {"NewPhysicalLinkStatus", VAR_PhysicalLinkStatus, VAR_OUT},
       { 0 }
    }
};

static Action _GetTotalBytesReceived = { 
    "GetTotalBytesReceived", GetTotalBytesReceived,
        (Param []) {
            {"NewTotalBytesReceived", VAR_TotalBytesReceived, VAR_OUT},
            { 0 }
        }
};


static Action _GetTotalBytesSent = { 
    "GetTotalBytesSent", GetTotalBytesSent,
        (Param []) {
            {"NewTotalBytesSent", VAR_TotalBytesSent, VAR_OUT},
            { 0 }
        }
};

static Action _GetTotalPacketsReceived = { 
    "GetTotalPacketsReceived", GetTotalPacketsReceived,
        (Param []) {
            {"NewTotalPacketsReceived", VAR_TotalPacketsReceived, VAR_OUT},
            { 0 }
        }
};

static Action _GetTotalPacketsSent = { 
    "GetTotalPacketsSent", GetTotalPacketsSent,
        (Param []) {
            {"NewTotalPacketsSent", VAR_TotalPacketsSent, VAR_OUT},
            { 0 }
        }
};

static Action _GetEnabledForInternet = { 
    "GetEnabledForInternet", GetEnabledForInternet,
        (Param []) {
            {"NewEnabledForInternet", VAR_EnabledForInternet, VAR_OUT},
            { 0 }
        }
};

static Action _SetEnabledForInternet = { 
    "SetEnabledForInternet", SetEnabledForInternet,
        (Param []) {
            {"NewEnabledForInternet", VAR_EnabledForInternet, VAR_IN},
            { 0 }
        }
};

static PAction Actions[] = {
    &_GetCommonLinkProperties,
    &_GetTotalBytesSent,
    &_GetTotalBytesReceived,
    &_GetTotalPacketsReceived,
    &_GetTotalPacketsSent,
    &_GetEnabledForInternet,
    &_SetEnabledForInternet,
    NULL
};

ServiceTemplate Template_WANCommonInterfaceConfig = {
    "WANCommonInterfaceConfig:1",
    NULL,
    WANCommonInterfaceConfig_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, StateVariables,
    Actions,
    0,
    "urn:dslforum-org:serviceId:WANCommonIFC"
};

