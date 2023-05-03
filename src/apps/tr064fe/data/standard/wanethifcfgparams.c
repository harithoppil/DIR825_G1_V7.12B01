
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
//  Filename:       wanethifcfgparams.c
//
******************************************************************************/
#ifdef INCLUDE_WANETHERNETCONFIG
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "wanethifcfgparams.h"

char *wanethMaxBitRate_allowedValueList[] = { "10",
                                              "100",
                                              "1000",
                                              "Auto", 
                                              NULL };

static VarTemplate StateVariables[] = { 
   { "Enable", "1", VAR_BOOL }, 
   { "Status", "", VAR_STRING }, 
   { "MACAddress", "", VAR_STRING|VAR_LIST,(allowedValue) { wanethMaxBitRate_allowedValueList } }, 
   { "MaxBitRate", "", VAR_STRING },    
   { "Bytes-Sent", "", VAR_ULONG },    
   { "Bytes-Received", "", VAR_ULONG },    
   { "Packets-Sent", "", VAR_ULONG },    
   { "Packets-Received", "", VAR_ULONG },    
   { 0 } 
};


static Action _SetEnable = {
    "SetEnable", SetETHInterfaceEnable,
   (Param []) {
       {"NewEnable", VAR_Enable, VAR_IN},
       { 0 }
    }
};

static Action _GetInfo = {
    "GetInfo", GetInfo,
   (Param []) {
       {"NewEnable", VAR_Enable, VAR_OUT},
       {"NewStatus", VAR_Status, VAR_OUT},
       {"NewMACAddress", VAR_MACAddress, VAR_OUT},   
       {"NewMaxBitRate", VAR_MaxBitRate, VAR_OUT},   
                                  
       { 0 }
    }
};


static Action _SetMaxBitRate = {
    "SetMaxBitRate", SetMaxBitRate,
   (Param []) {
       {"NewMaxBitRate", VAR_MaxBitRate, VAR_IN},
       { 0 }
    }
};

static Action _GetStatistics = {
    "GetStatistics", GetStatisticsWANETH,
   (Param []) {
       {"NewBytes-Sent", VAR_BytesSent, VAR_OUT},
       {"NewBytes-Received", VAR_BytesReceived, VAR_OUT},
       {"NewPackets-Sent", VAR_PacketsSent, VAR_OUT},
       {"NewPackets-Received", VAR_PacketsReceived, VAR_OUT},
       { 0 }
    }
};

static PAction Actions[] = {
   &_SetEnable,
   &_GetInfo,
   &_SetMaxBitRate,
   &_GetStatistics,
   NULL
};

ServiceTemplate Template_WANETHInterfaceConfig = {
   "WANETHInterfaceConfig:1",
   NULL,
   WANETHInterfaceConfig_GetVar,    /* state variable handler */
   NULL,  /* SVCXML */
   ARRAYSIZE(StateVariables)-1, 
   StateVariables,
   Actions,
   0,
   "urn:dslforum-org:serviceId:WANETHInterfaceConfig"
};
#endif

