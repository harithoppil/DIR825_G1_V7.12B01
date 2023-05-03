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
//  Filename:       lanhostsparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "lanhostsparams.h"

static VarTemplate StateVariables[] = {
    { "HostNumberOfEntries", "1", VAR_EVENTED|VAR_ULONG},
    { "IPAddress", "", VAR_EVENTED|VAR_STRING},
    { "AddressSource", "", VAR_EVENTED|VAR_STRING},
    { "LeaseTimeRemaining",  "", VAR_EVENTED|VAR_LONG },
    { "MACAddress", "", VAR_EVENTED|VAR_STRING },
    { "HostName", "", VAR_EVENTED|VAR_STRING },
    { "InterfaceType", "", VAR_EVENTED|VAR_STRING },
    { "Active", "", VAR_EVENTED|VAR_BOOL },
    { NULL }
};

static Action _GetHostNumberOfEntries = {
    "GetHostNumberOfEntries", GetHostNumberOfEntries,
   (Param []) {
       {"NewHostNumberOfEntries", VAR_HostNumberOfEntries, VAR_OUT},
       { 0 }
    }
};

static Action _GetSpecificHostEntry = {
    "GetSpecificHostEntry", GetSpecificHostEntry,
   (Param []) {
       {"NewMACAddress", VAR_MACAddress, VAR_IN},
       {"NewIPAddress", VAR_IPAddress, VAR_OUT},
       {"NewAddressSource", VAR_AddressSource, VAR_OUT},
       {"NewLeaseTimeRemaining", VAR_LeaseTimeRemaining, VAR_OUT},   
       {"NewInterfaceType", VAR_InterfaceType, VAR_OUT},
       {"NewActive", VAR_Active, VAR_OUT},
       {"NewHostName", VAR_HostName, VAR_OUT},
       { 0 }
    }
};

static Action _GetGenericHostEntry = {
    "GetGenericHostEntry", GetGenericHostEntry,
   (Param []) {
       {"NewIndex", VAR_HostNumberOfEntries, VAR_IN},
       {"NewIPAddress", VAR_IPAddress, VAR_OUT},
       {"NewAddressSource", VAR_AddressSource, VAR_OUT},
       {"NewLeaseTimeRemaining", VAR_LeaseTimeRemaining, VAR_OUT},   
       {"NewMACAddress", VAR_MACAddress, VAR_OUT},
       {"NewInterfaceType", VAR_InterfaceType, VAR_OUT},
       {"NewActive", VAR_Active, VAR_OUT},
       {"NewHostName", VAR_HostName, VAR_OUT},
       { 0 }
    }
};
static PAction Actions[] = {
    &_GetHostNumberOfEntries,
    &_GetSpecificHostEntry,
    &_GetGenericHostEntry,
//  &_DeleteHostEntry,
    NULL
};

ServiceTemplate Template_LANHosts = {
    "Hosts:",
    NULL,
    NULL,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, StateVariables,
    Actions,
    0,
    "urn:dslforum-org:serviceId:Hosts"
};

