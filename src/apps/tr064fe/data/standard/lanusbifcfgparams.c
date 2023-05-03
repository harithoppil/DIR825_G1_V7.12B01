/*****************************************************************************
//
//  Copyright (c) 2005  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
******************************************************************************
//
//  Filename:       lanusbifcfgparams.c
//
******************************************************************************/
#ifdef USB
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "lanusbifcfgparams.h"

static VarTemplate StateVariables[] = {
    { "Enable", "1", VAR_EVENTED|VAR_BOOL},
    { "Status", "", VAR_EVENTED|VAR_STRING},	
    { "MACAddress", "", VAR_EVENTED|VAR_STRING},	
    { "MACAddressControlEnabled", "", VAR_EVENTED|VAR_BOOL },    
    { "Standard",  "", VAR_EVENTED|VAR_STRING },
    { "BytesSent", "", VAR_EVENTED|VAR_ULONG},
    { "Type",  "", VAR_EVENTED|VAR_STRING },
    { "Rate", "", VAR_EVENTED|VAR_STRING},    
    { "BytesReceived", "", VAR_EVENTED|VAR_ULONG},
    { "PacketsSent", "", VAR_EVENTED|VAR_ULONG},
    { "PacketsReceived", "", VAR_EVENTED|VAR_ULONG},
    { NULL }
};

static Action _GetInfo = {
    "GetInfo", GetInfo,
   (Param []) {
        { "NewEnable", VAR_Enable, VAR_OUT},
        { "NewStatus", VAR_Status, VAR_OUT},	
        { "NewMACAddress", VAR_MACAddress, VAR_OUT},
        { "NewMACAddressControlEnabled",  VAR_MACAddressControlEnabled, VAR_OUT },
        { "NewStandard", VAR_Standard, VAR_OUT },
        { 0 }
    }
};

static Action _GetStatistics = {
    "GetStatistics", GetStatistics,
   (Param []) {
        { "NewBytesSent", VAR_BytesSent, VAR_OUT},
        { "NewBytesReceived", VAR_BytesReceived, VAR_OUT},
        { "NewPacketsSent", VAR_PacketsSent, VAR_OUT},
        { "NewPacketsReceived",  VAR_PacketsReceived, VAR_OUT },
        { 0 }
    }
};

static Action _SetEnable = {
    "SetEnable", SetUSBEnable,
   (Param []) {
        { "NewEnable", VAR_Enable, VAR_IN},
        { 0 }
    }
};

static PAction Actions[] = {
    &_GetInfo,
    &_GetStatistics,
    &_SetEnable,
    NULL
};

ServiceTemplate Template_LANUSBInterfaceConfig = {
    "LANUSBInterfaceConfig:",
    NULL,
    LANUSBIFCfg_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, StateVariables,
    Actions,
    0,
    "urn:dslforum-org:serviceId:LANUSBInterfaceConfig"
};
#endif

