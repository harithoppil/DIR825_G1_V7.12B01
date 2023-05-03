
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
//  Filename:       wandslconnmgtparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "wandslconnmgtparams.h"

char *PossibleLinkTypesDslConnMgmt_allowedValueList[] = { "EoA(RFC2684B)",
                                               "IPoA(RFC2684R)",
                                               "PPPoA",
                                               "PPPoE",
                                               "CIP",
                                               "Unconfigured",
                                               NULL };

char *PossibleConnectionTypesDslConnMgmt_allowedValueList[] = { "IP_Routed",
                                               "IP_Bridged",
                                               "Unconfigured",
                                               NULL };


static VarTemplate StateVariables[] = {
   { "WANConnectionDevice", "", VAR_STRING },
   { "WANConnectionService", "", VAR_STRING },       
   { "DestinationAddress", "", VAR_STRING }, 
   { "LinkType", "", VAR_STRING|VAR_LIST, (allowedValue) { PossibleLinkTypesDslConnMgmt_allowedValueList } }, 
   { "ConnectionType", "", VAR_STRING|VAR_LIST ,(allowedValue) { PossibleConnectionTypesDslConnMgmt_allowedValueList }}, 
   { "Name", "", VAR_STRING }, 
   { "WANConnectionServiceNumberOfEntries", "", VAR_STRING }, 
   { 0 } 
};

static Action _GetWANConnectionServiceNumberOfEntries = { 
   "GetWANConnectionServiceNumberOfEntries", GetWANConnectionServiceNumberOfEntries,
   (Param [])    {
      {"NewWANConnectionServiceNumberOfEntries", VAR_WANConnectionServiceNumberOfEntries, VAR_OUT},
      { 0 }
   }
};

static Action _GetGenericConnectionServiceEntry = { 
   "GetGenericConnectionServiceEntry", GetGenericConnectionServiceEntry,
   (Param [])    {
      {"NewConnectionServiceIndex", VAR_WANConnectionServiceNumberOfEntries, VAR_IN},
      {"NewWANConnectionDevice", VAR_WANConnectionDevice, VAR_OUT},
      {"NewWANConnectionService", VAR_WANConnectionService, VAR_OUT},
      {"NewName", VAR_Name, VAR_OUT},
      { 0 }
   }
};

static Action _GetSpecificConnectionServiceEntry = { 
   "GetSpecificConnectionServiceEntry", GetSpecificConnectionServiceEntry,
   (Param [])    {
      {"NewWANConnectionService", VAR_WANConnectionService, VAR_IN},
      {"NewWANConnectionDevice", VAR_WANConnectionDevice, VAR_OUT},
      {"NewName", VAR_Name, VAR_OUT},
      { 0 }
   }
};


static Action _AddConnectionDeviceAndService = { 
   "AddConnectionDeviceAndService", AddConnectionDeviceAndService,
   (Param [])    {
      {"NewDestinationAddress", VAR_DestinationAddress, VAR_IN},
      {"NewLinkType", VAR_LinkType, VAR_IN},
      {"NewConnectionType", VAR_ConnectionType, VAR_IN},
      {"NewName", VAR_Name, VAR_IN},
      {"NewWANConnectionDevice", VAR_WANConnectionDevice, VAR_OUT},
      {"NewWANConnectionService", VAR_WANConnectionService, VAR_OUT},
      { 0 }
   }
};

static Action _AddConnectionService = { 
   "AddConnectionService", AddConnectionService,
   (Param [])    {
      {"NewWANConnectionDevice", VAR_WANConnectionDevice, VAR_IN},
      {"NewConnectionType", VAR_ConnectionType, VAR_IN},
      {"NewName", VAR_Name, VAR_IN},
      {"NewWANConnectionService", VAR_WANConnectionService, VAR_OUT},
      { 0 }
   }
};

static Action _DeleteConnectionService = { 
   "DeleteConnectionService", DeleteConnectionService,
   (Param [])    {
      {"NewWANConnectionService", VAR_WANConnectionService, VAR_IN},
      { 0 }
   }
};

static PAction Actions[] = {
   &_GetWANConnectionServiceNumberOfEntries,
   &_GetGenericConnectionServiceEntry,
   &_GetSpecificConnectionServiceEntry,
   &_AddConnectionDeviceAndService,
   &_AddConnectionService,
   &_DeleteConnectionService,
   NULL
};


ServiceTemplate Template_WANDSLConnMgt = {
   "WANDSLConnectionManagement:1",
   NULL,
   NULL,    /* state variable handler */
   NULL,  /* SVCXML */
   ARRAYSIZE(StateVariables)-1, 
   StateVariables,
   Actions,
   0,
   "urn:dslforum-org:service:WANDSLConnectionManagement"
};
