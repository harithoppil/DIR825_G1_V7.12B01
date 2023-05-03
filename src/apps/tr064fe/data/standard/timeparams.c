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
//  Filename:       timeparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "timeparams.h"


static VarTemplate StateVariables[] = { 
   { "NTPServer1", "", VAR_STRING }, 
   { "NTPServer2", "", VAR_STRING }, 
   { "NTPServer3", "", VAR_STRING }, 
   { "NTPServer4", "", VAR_STRING }, 
   { "NTPServer5", "", VAR_STRING }, 
   { "CurrentLocalTime", "", VAR_STRING }, 
   { "LocalTimeZone", "", VAR_STRING }, 
   { "LocalTimeZoneName", "", VAR_STRING }, 
   { "DaylightSavingsUsed", "", VAR_BOOL }, 
   { "DaylightSavingsStart", "", VAR_STRING }, 
   { "DaylightSavingsEnd", "", VAR_STRING }, 
   { NULL }
};

static Action _GetIfno = {
    "GetInfo", GetInfo,
   (Param []) {
       {"NewNTPServer1", VAR_NTPServer1, VAR_OUT},
       {"NewNTPServer2", VAR_NTPServer2, VAR_OUT},
       {"NewNTPServer3", VAR_NTPServer3, VAR_OUT},
       {"NewNTPServer4", VAR_NTPServer4, VAR_OUT},
       {"NewNTPServer5", VAR_NTPServer5, VAR_OUT},
       {"NewCurrentLocalTime", VAR_CurrentLocalTime, VAR_OUT},
       {"NewLocalTimeZone", VAR_LocalTimeZone, VAR_OUT},    
       {"NewLocalTimeZoneName", VAR_LocalTimeZoneName, VAR_OUT},
       {"NewDaylightSavingsUsed", VAR_DaylightSavingsUsed, VAR_OUT},
       {"NewDaylightSavingStart", VAR_DaylightSavingsStart, VAR_OUT},
       {"NewDaylightSavingEnd", VAR_DaylightSavingsEnd, VAR_OUT},
       { 0 }
    }
};

static Action _SetNTPServers = { 
   "SetNTPServers", SetNTPServers,
   (Param []) {
      {"NewNTPServer1", VAR_NTPServer1, VAR_IN},
      {"NewNTPServer2", VAR_NTPServer2, VAR_IN},
      {"NewNTPServer3", VAR_NTPServer3, VAR_IN},
      {"NewNTPServer4", VAR_NTPServer4, VAR_IN},
      {"NewNTPServer5", VAR_NTPServer5, VAR_IN},

      { 0 }
   }
};

static Action _SetLocalTimeZone = { 
   "SetLocalTimeZone", SetLocalTimeZone,
   (Param []) {
      {"NewLocalTimeZone", VAR_LocalTimeZone, VAR_IN},
      {"NewLocalTimeZoneName", VAR_LocalTimeZoneName, VAR_IN},
      {"NewDaylightSavingsUsed", VAR_DaylightSavingsUsed, VAR_IN},
      {"NewDaylightSavingsStart", VAR_DaylightSavingsStart, VAR_IN},
      {"NewDaylightSavingsEnd", VAR_DaylightSavingsEnd, VAR_IN},
      { 0 }
   }
};

static PAction Actions[] = {
    &_GetIfno,
    &_SetNTPServers,
    &_SetLocalTimeZone,
    NULL
};

ServiceTemplate Template_TimeServer = {
    "Time:1",
    NULL, 
    TimeServer_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, 
    StateVariables,
    Actions
};
