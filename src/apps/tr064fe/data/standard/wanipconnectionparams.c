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
//  Filename:        wanipconnectionparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "wanipconnection.h"

char *IPPossibleConnectionTypes_allowedValueList[] = { "Unconfigured",
                                                       "IP_Routed",
                                                       "IP_Bridged",
                                                       NULL };

char *IPConnectionStatus_allowedValueList[] = { "Unconfigured",
                                              "Connecting",
                                              "Authenticating",
                                              "Connected",
                                              "PendingDisconnect",
                                              "Disconnecting",
                                              "Disconnected",
                                              NULL };

char *IPConnectionTrigger_allowedValueList[] = { "OnDemand",
                                              "AlwaysOn",
                                              "Manual",
                                              NULL };

char *IPLastConnectionError_allowedValueList[] = { "ERROR_NONE", "ERROR_UNKNOWN", NULL };
char *IPPortMappingProtocol_allowedValueList[] = { "TCP", "UDP", NULL };
char *IPAddressingType_allowedValueList[] = { "DHCP", "Static", NULL };

static VarTemplate StateVariables[] = {
    { "Enable", "1", VAR_BOOL },
    { "ConnectionType", "IP_Routed", VAR_STRING|VAR_LIST,
        (allowedValue) { IPPossibleConnectionTypes_allowedValueList }  },      
    { "PossibleConnectionTypes", "", VAR_EVENTED|VAR_STRING|VAR_LIST,   //not support
        (allowedValue) { IPPossibleConnectionTypes_allowedValueList } },
    { "ConnectionStatus", "", VAR_EVENTED|VAR_STRING|VAR_LIST,
        (allowedValue) { IPConnectionStatus_allowedValueList } },        
    { "Name", "", VAR_STRING },
    { "Uptime", "", VAR_ULONG },  // not support
    { "NATEnabled", "1", VAR_BOOL },
    { "AddressingType", "", VAR_STRING|VAR_LIST, (allowedValue) { IPAddressingType_allowedValueList } },
    { "ExternalIPAddress", "", VAR_EVENTED|VAR_STRING },
    { "SubnetMask", "", VAR_STRING },
    { "DefaultGateway", "", VAR_STRING },    
    { "DNSEnabled", "", VAR_BOOL },
    { "DNSServers", "", VAR_STRING },
    { "ConnectionTrigger", "", VAR_STRING|VAR_LIST, (allowedValue) { IPConnectionTrigger_allowedValueList } }, //not support
    { "PortMappingNumberOfEntries", "", VAR_EVENTED|VAR_USHORT },
    { "PortMappingEnabled", "", VAR_BOOL }, //not support
    { "ExternalPort", "", VAR_USHORT },
    { "InternalPort", "", VAR_USHORT },
    { "PortMappingProtocol", "", VAR_STRING|VAR_LIST,
        (allowedValue) { IPPortMappingProtocol_allowedValueList } },
    { "InternalClient", "", VAR_STRING },
    { "PortMappingDescription", "", VAR_STRING },
    { "BytesSent", "", VAR_ULONG },
    { "BytesReceived", "", VAR_ULONG },
    { "PacketsSent", "", VAR_ULONG },
    { "PacketsReceived", "", VAR_ULONG },
    { NULL }
};

static Action _SetEnable = {
    "SetEnable", SetWANIPConnEnable,
    (Param []) {
       {"NewEnable", VAR_Enable, VAR_IN},
       { 0 }
    }
};
static Action _GetInfo = {
    "GetInfo", GetInfo,
   (Param []) {
       {"NewEnable", VAR_Enable, VAR_OUT},   
       {"NewConnectionType", VAR_ConnectionType, VAR_OUT},
       {"NewPossibleConnectionTypes", VAR_PossibleConnectionTypes, VAR_OUT},
       {"NewConnectionStatus", VAR_ConnectionStatus, VAR_OUT},       
       {"NewName", VAR_Name, VAR_OUT},       
       {"NewUptime", VAR_Uptime, VAR_OUT},              
       {"NewNATEnabled", VAR_NATEnabled, VAR_OUT},       
       {"NewExternalIPAddress", VAR_ExternalIPAddress, VAR_OUT},  
       {"NewSubnetMask", VAR_SubnetMask, VAR_OUT},  
       {"NewDefaultGateway", VAR_DefaultGateway, VAR_OUT},  
  //     {"NewDNSEnabled", VAR_DNSEnabled, VAR_OUT},        
       {"NewDNSServers", VAR_DNSServers, VAR_OUT},   
       {"NewConnectionTrigger", VAR_ConnectionTrigger, VAR_OUT},   
       { 0 }
    }
};

static Action _GetConnectionTypeInfo = {
    "GetConnectionTypeInfo", GetConnectionTypeInfo,
   (Param []) {
       {"NewConnectionType", VAR_ConnectionType, VAR_OUT},
       {"NewPossibleConnectionTypes", VAR_PossibleConnectionTypes, VAR_OUT},
       { 0 }
    }
};

static Action _SetIPConnectionType = {
    "SetConnectionType", SetIPConnectionType,
    (Param []) {
       {"NewConnectionType", VAR_ConnectionType, VAR_IN},
       { 0 }
    }
};

static Action _RequestConnection = {
    "RequestConnection", RequestConnectionTR64,
   (Param []) {
       { 0 }
    }
};

static Action _RequestTermination = {
    "RequestTermination", RequestTerminationTR64,
   (Param []) {
       { 0 }
    }
};

static Action _ForceTermination = {
    "ForceTermination", ForceTerminationTR64,
   (Param []) {
       { 0 }
    }
};

static Action _GetStatusInfo = {
    "GetStatusInfo", GetStatusInfo,
   (Param []) {
       {"NewConnectionStatus", VAR_ConnectionStatus, VAR_OUT},
       {"NewUptime", VAR_Uptime, VAR_OUT},
       { 0 }
    }
};

static Action _GetExternalIPAddress = {
    "GetExternalIPAddress", GetExternalIPAddress,
   (Param []) {
       {"NewExternalIPAddress", VAR_ExternalIPAddress, VAR_OUT},
       { 0 }
    }
};

static Action _SetIPInterfaceInfo = {
    "SetIPInterfaceInfo", SetIPInterfaceInfo,
    (Param []) {
       {"NewAddressingType", VAR_AddressingType, VAR_IN},
       {"NewExternalIPAddress", VAR_ExternalIPAddress, VAR_IN},           
       {"NewSubnetMask", VAR_SubnetMask, VAR_IN},       
       {"NewDefaultGateway", VAR_DefaultGateway, VAR_IN},              
       { 0 }
    }
};

static Action _SetConnectionTrigger = {
    "SetConnectionTrigger", SetIPConnectionTrigger,
    (Param []) {
       {"NewConnectionTrigger", VAR_ConnectionTrigger, VAR_IN},          
       { 0 }
    }
};

static Action _GetPortMappingNumberOfEntries = {
    "GetPortMappingNumberOfEntries", GetPortMappingNumberOfEntries,
    (Param []) {
       {"NewPortMappingNumberOfEntries", VAR_PortMappingNumberOfEntries, VAR_OUT},          
       { 0 }
    }
};
              
static Action _GetGenericPortMappingEntry = {
    "GetGenericPortMappingEntry", GetGenericPortMappingEntry,
   (Param []) {
       {"NewPortMappingIndex", VAR_PortMappingNumberOfEntries, VAR_IN},
       {"NewExternalPort", VAR_ExternalPort, VAR_OUT},
       {"NewProtocol", VAR_PortMappingProtocol, VAR_OUT},
       {"NewInternalPort", VAR_InternalPort, VAR_OUT},
       {"NewInternalClient", VAR_InternalClient, VAR_OUT},
       {"NewEnabled", VAR_PortMappingEnabled, VAR_OUT},
       {"NewPortMappingDescription", VAR_PortMappingDescription, VAR_OUT},
       { 0 }
    }
};

static Action _GetSpecificPortMappingEntry = {
    "GetSpecificPortMappingEntry", GetSpecificPortMappingEntry,
   (Param []) {
       {"NewExternalPort", VAR_ExternalPort, VAR_IN},
       {"NewProtocol", VAR_PortMappingProtocol, VAR_IN},
       {"NewInternalPort", VAR_InternalPort, VAR_OUT},
       {"NewInternalClient", VAR_InternalClient, VAR_OUT},
       {"NewEnabled", VAR_PortMappingEnabled, VAR_OUT},
       {"NewPortMappingDescription", VAR_PortMappingDescription, VAR_OUT},
       { 0 }
    }
};

static Action _AddPortMapping = {
    "AddPortMapping", AddPortMappingEntry,
   (Param []) {
       {"NewExternalPort", VAR_ExternalPort, VAR_IN},
       {"NewProtocol", VAR_PortMappingProtocol, VAR_IN},
       {"NewInternalPort", VAR_InternalPort, VAR_IN},
       {"NewInternalClient", VAR_InternalClient, VAR_IN},
       {"NewEnabled", VAR_PortMappingEnabled, VAR_IN},
       {"NewPortMappingDescription", VAR_PortMappingDescription, VAR_IN},
       { 0 }
    }
};
static Action _DeletePortMapping = {
    "DeletePortMapping", DeletePortMappingEntry,
   (Param []) {
       {"NewExternalPort", VAR_ExternalPort, VAR_IN},
       {"NewProtocol", VAR_PortMappingProtocol, VAR_IN},
       { 0 }
    }
};

static Action _GetStatistics = {
    "GetStatistics", GetStatisticsWANIP,
   (Param []) {
       {"NewBytesSent", VAR_BytesSent, VAR_OUT},
       {"NewBytesReceived", VAR_BytesReceived, VAR_OUT},
       {"NewPacketsSent", VAR_PacketsSent, VAR_OUT},
       {"NewPacketsReceived", VAR_PacketsReceived, VAR_OUT},
       { 0 }
    }
};

static PAction Actions[] = {
   &_SetEnable,
   &_GetInfo,
   &_GetConnectionTypeInfo,
   &_GetStatusInfo,
   &_GetExternalIPAddress,
//   &_RequestConnection,
 //  &_RequestTermination,
//   &_ForceTermination,
   &_SetIPInterfaceInfo,
 //  &_SetConnectionTrigger,
   &_SetIPConnectionType,
   &_GetPortMappingNumberOfEntries,
   &_GetGenericPortMappingEntry,
   &_GetSpecificPortMappingEntry,
   &_AddPortMapping,
   &_DeletePortMapping,
   &_GetStatistics,
    NULL
};

ServiceTemplate Template_WANIPConnection = {
    "WANIPConnection:",
    NULL,    /* service initialization */
    WANIPConnection_GetVar,    /* state variable handler */
    NULL,            /* xml generator */
    ARRAYSIZE(StateVariables)-1, 
    StateVariables,
    Actions, 0,
    "urn:dslforum-org:serviceId:WANIPConnection"
};

