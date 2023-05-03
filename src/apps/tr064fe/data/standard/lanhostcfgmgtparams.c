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
//  Filename:       lanparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "lanhostcfgmgtparams.h"

static VarTemplate LanHostStateVariables[] = {
	{ "DHCPServerConfigurable", "1", VAR_EVENTED|VAR_BOOL}, //not support
    { "DHCPServerEnable", "1", VAR_EVENTED|VAR_BOOL},    
    { "DHCPRelay", "0", VAR_EVENTED|VAR_BOOL},            
    { "MinAddress",  "", VAR_EVENTED|VAR_STRING },
    { "MaxAddress", "", VAR_EVENTED|VAR_STRING },
    { "ReservedAddresses", "", VAR_EVENTED|VAR_STRING },
    { "SubnetMask", "", VAR_EVENTED|VAR_STRING },
    { "DNSServers", "", VAR_EVENTED|VAR_STRING },
    { "DomainName", "", VAR_EVENTED|VAR_STRING },
    { "IPRouters", "", VAR_EVENTED|VAR_STRING },    
    { "DHCPLeaseTime", "", VAR_ULONG },
    { "IPInterfaceNumberOfEntries", "", VAR_LONG },  
    { "Enable", "", VAR_BOOL },    
    { "IPInterfaceIPAddress", "", VAR_STRING },    
    { "IPInterfaceSubnetMask", "", VAR_STRING },    
    { "IPInterfaceAddressingType", "", VAR_STRING },    //not support
    { NULL }
};

static Action _GetInfo = {
    "GetInfo", GetInfo,
   (Param []) {
		    { "NewDHCPServerConfigurable", VAR_DHCPServerConfigurable, VAR_OUT},  // not support
		    { "NewDHCPServerEnable", VAR_DHCPServerEnable, VAR_OUT},    
		    { "NewDHCPRelay", VAR_DHCPRelay, VAR_OUT},            
		    { "NewMinAddress",  VAR_MinAddress, VAR_OUT },
		    { "NewMaxAddress", VAR_MaxAddress, VAR_OUT },
		    { "NewReservedAddresses", VAR_ReservedAddresses, VAR_OUT },
		    { "NewSubnetMask", VAR_SubnetMask, VAR_OUT },
		    { "NewDNSServers", VAR_DNSServers, VAR_OUT },
		    { "NewDomainName", VAR_DomainName, VAR_OUT },
		    { "NewIPRouters", VAR_IPRouters, VAR_OUT },    
		    { "NewDHCPLeaseTime", VAR_DHCPLeaseTime, VAR_OUT },
		    { 0 }
    }
};

static Action _GetDHCPRelay = { 
    "GetDHCPRelay", GetDHCPRelay,
        (Param []) {
            {"NewDHCPRelay", VAR_DHCPRelay, VAR_OUT},
            { 0 }
        }
};

static Action _SetSubnetMask = { 
    "SetSubnetMask", SetSubnetMask,
        (Param []) {
            {"NewSubnetMask", VAR_SubnetMask, VAR_IN},
            { 0 }
        }
};

static Action _GetSubnetMask = { 
    "GetSubnetMask", GetSubnetMask,
        (Param []) {
            {"NewSubnetMask", VAR_SubnetMask, VAR_OUT},
            { 0 }
        }
};

static Action _SetIPRouter = { 
    "SetIPRouter", SetIPRouter,
        (Param []) {
            {"NewIPRouters", VAR_IPRouters, VAR_IN},
            { 0 }
        }
};

static Action _GetIPRoutersList = { 
    "GetIPRoutersList", GetIPRoutersList,
        (Param []) {
            {"NewIPRouters", VAR_IPRouters, VAR_OUT},
            { 0 }
        }
};

static Action _SetDomainName = { 
    "SetDomainName", SetDomainName,
        (Param []) {
            {"NewDomainName", VAR_DomainName, VAR_IN},
            { 0 }
        }
};

static Action _GetDomainName = { 
    "GetDomainName", GetDomainName,
        (Param []) {
            {"NewDomainName", VAR_DomainName, VAR_OUT},
            { 0 }
        }
};

static Action _SetAddressRange = { 
    "SetAddressRange", SetAddressRange,
        (Param []) {
            {"NewMinAddress", VAR_MinAddress, VAR_IN},
            {"NewMaxAddress", VAR_MaxAddress, VAR_IN},                
            { 0 }
        }
};

static Action _GetAddressRange = { 
    "GetAddressRange", GetAddressRange,
        (Param []) {
            {"NewMinAddress", VAR_MinAddress, VAR_OUT},
            {"NewMaxAddress", VAR_MaxAddress, VAR_OUT},    
            { 0 }
        }
};



static Action _SetDNSServer = { 
    "SetDNSServer", SetDNSServer,
        (Param []) {
            {"NewDNSServers", VAR_DNSServers, VAR_IN},
            { 0 }
        }
};

static Action _DeleteDNSServer = { 
    "DeleteDNSServer", DeleteDNSServer,
        (Param []) {
            {"NewDNSServers", VAR_DNSServers, VAR_IN},
            { 0 }
        }
};

static Action _GetDNSServers = { 
    "GetDNSServer", GetDNSServers,
        (Param []) {
            {"NewDNSServers", VAR_DNSServers, VAR_OUT},
            { 0 }
        }
};

static Action _SetDHCPLeaseTime = {
    "SetDHCPLeaseTime", SetDHCPLeaseTime,
   (Param []) {
       {"NewDHCPLeaseTime", VAR_DHCPLeaseTime, VAR_IN},
       { 0 }
    }
};

static Action _SetDHCPServerEnable = {
    "SetDHCPServerEnable", SetDHCPServerEnable,
   (Param []) {
       {"NewDHCPServerEnable", VAR_DHCPServerEnable, VAR_IN},
       { 0 }
    }
};

static Action _GetIPInterfaceNumberOfEntries = {
    "GetIPInterfaceNumberOfEntries", GetIPInterfaceNumberOfEntries,
   (Param []) {
       {"NewIPInterfaceNumberOfEntries", VAR_IPInterfaceNumberOfEntries, VAR_OUT},
       { 0 }
    }
};

static Action _SetIPInterface = {
    "SetIPInterface", SetIPInterface,
   (Param []) {
       {"NewIPInterfaceIndex", VAR_IPInterfaceNumberOfEntries, VAR_IN},
       {"NewEnable", VAR_Enable, VAR_IN},
       {"NewIPAddress", VAR_IPInterfaceIPAddress, VAR_IN},
       {"NewSubnetMask", VAR_IPInterfaceSubnetMask, VAR_IN},
       { 0 }
    }
};

static Action _GetIPInterfaceGenericEntry = {
    "GetIPInterfaceGenericEntry", GetIPInterfaceGenericEntry,
   (Param []) {
       {"NewIPInterfaceIndex", VAR_IPInterfaceNumberOfEntries, VAR_IN},
       {"NewEnable", VAR_Enable, VAR_OUT},
       {"NewIPAddress", VAR_IPInterfaceIPAddress, VAR_OUT},
       {"NewSubnetMask", VAR_IPInterfaceSubnetMask, VAR_OUT},
       { 0 }
    }
};

static PAction Actions[] = {
   &_GetInfo,
   &_GetDHCPRelay,
   &_SetSubnetMask,
   &_GetSubnetMask,
   &_SetIPRouter,
   &_GetIPRoutersList,
   &_SetDomainName,
   &_GetDomainName,
   &_SetAddressRange,
   &_GetAddressRange,
// &_SetReservedAddress,   //not support
// &_DeleteReservedAddress,  //not support
   &_SetDNSServer,
   &_DeleteDNSServer,
   &_GetDNSServers,
   &_SetDHCPLeaseTime,
   &_SetDHCPServerEnable,
   &_GetIPInterfaceNumberOfEntries,
   &_SetIPInterface,
// &_AddIPInterface,  //not support
// &_DeleteIPInterface, //not support
   &_GetIPInterfaceGenericEntry, 
    NULL
};

ServiceTemplate Template_LANHostConfigManagement = {
    "LANHostConfigManagement:",
    NULL,
    LANHostConfigManagement_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(LanHostStateVariables)-1, LanHostStateVariables,
    Actions,
    0,
    "urn:dslforum-org:serviceId:LANHostConfigManagement"
};

