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
//  Filename:        wanpppconnectionparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "wanpppconnection.h"

char *PossibleConnectionTypes_allowedValueList[] = { "Unconfigured",
                                                     "IP_Routed",
                                                     "DHCP_Spoofed",
                                                     "PPPoE_Bridged",
                                                     "PPTP_Relay",
                                                     "L2TP_Relay",
                                                     "PPPoE_Relay",
                                                     NULL };

char *ConnectionStatus_allowedValueList[] = { "Unconfigured",
                                              "Connecting",
                                              "Authenticating",
                                              "Connected",
                                              "PendingDisconnect",
                                              "Disconnecting",
                                              "Disconnected",
                                              NULL };

char *PPPConnectionTrigger_allowedValueList[] = { "OnDemand",
                                              "AlwaysOn",
                                              "Manual",
                                              NULL };

char *LastConnectionError_allowedValueList[] = { "ERROR_NONE", "ERROR_UNKNOWN", NULL };
char *PortMappingProtocol_allowedValueList[] = { "TCP", "UDP", NULL };

static VarTemplate StateVariables[] = {
    { "Enable", "", VAR_BOOL },
    { "ConnectionType", "IP_Routed", VAR_STRING|VAR_LIST,
        (allowedValue) { PossibleConnectionTypes_allowedValueList }  },
    { "PossibleConnectionTypes", "", VAR_EVENTED|VAR_STRING|VAR_LIST,
        (allowedValue) { PossibleConnectionTypes_allowedValueList } },
    { "ConnectionStatus", "", VAR_EVENTED|VAR_STRING|VAR_LIST,
        (allowedValue) { ConnectionStatus_allowedValueList } },
    { "Name", "", VAR_STRING },
    { "IdleDisconnectTime", "", VAR_ULONG },
    { "NATEnabled", "1", VAR_BOOL },
    { "UserName", "", VAR_STRING },
    { "Password", "", VAR_STRING },
    { "ExternalIPAddress", "", VAR_EVENTED|VAR_STRING },
    { "RemoteIPAddress", "", VAR_EVENTED|VAR_STRING },    
    { "DNSServers", "", VAR_STRING },     
    { "PPPoEServiceName", "", VAR_STRING },        
    { "ConnectionTrigger", "", VAR_STRING |VAR_LIST, (allowedValue) { PPPConnectionTrigger_allowedValueList }},  //not      
    { "PortMappingNumberOfEntries", "",VAR_USHORT },
    { "PortMappingEnabled", "", VAR_BOOL },
    { "ExternalPort", "", VAR_USHORT },
    { "InternalPort", "", VAR_USHORT },
    { "PortMappingProtocol", "", VAR_STRING|VAR_LIST,
        (allowedValue) { PortMappingProtocol_allowedValueList } },
    { "InternalClient", "", VAR_STRING },
    { "PortMappingDescription", "", VAR_STRING },
    { "BytesSent", "", VAR_ULONG },
    { "BytesReceived", "", VAR_ULONG },
    { "PacketsSent", "", VAR_ULONG },
    { "PacketsReceived", "", VAR_ULONG },
    { NULL }
};

static Action _SetEnable = {
    "SetEnable", SetWANPPPConnEnable,
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
       {"NewConnectionStatus", VAR_ConnectionStatus, VAR_OUT},       
       {"NewName", VAR_Name, VAR_OUT},       
       {"NewIdleDisconnectTime", VAR_IdleDisconnectTime, VAR_OUT},              
       {"NewNATEnabled", VAR_NATEnabled, VAR_OUT},       
       {"NewUsername", VAR_Username, VAR_OUT},       
       {"NewExternalIPAddress", VAR_ExternalIPAddress, VAR_OUT},  
       {"NewRemoteIPAddress", VAR_ExternalIPAddress, VAR_OUT},  
       {"NewDNSServers", VAR_DNSServers, VAR_OUT},   
       {"NewPPPoEServiceName", VAR_PPPoEServiceName, VAR_OUT},   
       { 0 }
    }
};

static Action _GetConnectionTypeInfo = {
    "GetConnectionTypeInfo", GetConnectionTypeInfo,
   (Param []) {
       {"NewConnectionType", VAR_ConnectionType, VAR_OUT},
       { 0 }
    }
};

static Action _SetConnectionType = {
    "SetConnectionType", SetConnectionType,
   (Param []) {
       {"NewConnectionType", VAR_ConnectionType, VAR_IN},
       { 0 }
    }
};

static Action _SetIdleDisconnectTime = {
    "SetIdleDisconnectTime", SetIdleDisconnectTime,
   (Param []) {
       {"NewIdleDisconnectTime", VAR_IdleDisconnectTime, VAR_IN},
       { 0 }
    }
};

static Action _GetIdleDisconnectTime = {
    "GetIdleDisconnectTime", GetIdleDisconnectTime,
   (Param []) {
       {"NewIdleDisconnectTime", VAR_IdleDisconnectTime, VAR_OUT},
       { 0 }
    }
};

static Action _RequestConnection = {
    "RequestConnection", RequestConnection,
   (Param []) {
       { 0 }
    }
};

static Action _RequestTermination = {
    "RequestTermination", RequestTermination,
   (Param []) {
       { 0 }
    }
};

static Action _ForceTermination = {
    "ForceTermination", ForceTermination,
   (Param []) {
       { 0 }
    }
};

static Action _GetStatusInfo = {
    "GetStatusInfo", GetStatusInfo,
   (Param []) {
       {"NewConnectionStatus", VAR_ConnectionStatus, VAR_OUT},
       { 0 }
    }
};

static Action _GetUsername = {
    "GetUserName", GetUsername,
   (Param []) {
       {"NewUserName", VAR_Username, VAR_OUT},
       { 0 }
    }
};

static Action _SetUsername = {
    "SetUserName", SetUsername,
   (Param []) {
       {"NewUserName", VAR_Username, VAR_IN},
       { 0 }
    }
};

static Action _SetPassword = {
    "SetPassword", SetPassword,
   (Param []) {
       {"NewPassword", VAR_Password, VAR_IN},
       { 0 }
    }
};

static Action _SetPPPoEService = {
    "SetPPPoEService", SetPPPoEService,
   (Param []) {
       {"NewPPPoEServiceName", VAR_PPPoEServiceName, VAR_IN},
       { 0 }
    }
};

static Action _SetConnectionTrigger = {
    "SetConnectionTrigger", SetConnectionTrigger,
    (Param []) {
       {"NewConnectionTrigger", VAR_ConnectionTrigger, VAR_IN},          
       { 0 }
    }
};

static Action _GetPortMappingNumberOfEntries = {
    "GetPortMappingNumberOfEntries", ppp_GetPortMappingNumberOfEntries,
    (Param []) {
       {"NewPortMappingNumberOfEntries", VAR_PortMappingNumberOfEntries, VAR_OUT},          
       { 0 }
    }
};

static Action _GetGenericPortMappingEntry = {
    "GetGenericPortMappingEntry", ppp_GetGenericPortMappingEntry,
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
    "GetSpecificPortMappingEntry", ppp_GetSpecificPortMappingEntry,
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
    "AddPortMapping", ppp_AddPortMappingEntry,
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
    "DeletePortMapping", ppp_DeletePortMappingEntry,
   (Param []) {
       {"NewExternalPort", VAR_ExternalPort, VAR_IN},
       {"NewProtocol", VAR_PortMappingProtocol, VAR_IN},
       { 0 }
    }
};

static Action _GetStatistics = {
    "GetStatistics", GetStatisticsWANPPP,
   (Param []) {
       {"NewBytesSent", VAR_BytesSent, VAR_OUT},
       {"NewBytesReceived", VAR_BytesReceived, VAR_OUT},
       {"NewPacketsSent", VAR_PacketsSent, VAR_OUT},
       {"NewPacketsReceived", VAR_PacketsReceived, VAR_OUT},
       { 0 }
    }
};

static PAction Actions[] = {
    &_GetInfo,
    &_SetEnable,
    &_GetConnectionTypeInfo,
    &_SetConnectionType,
//    &_RequestConnection,
//    &_RequestTermination,
//    &_ForceTermination,
    &_GetStatusInfo,
    &_SetIdleDisconnectTime,
    &_GetIdleDisconnectTime,
    &_GetUsername,
    &_SetUsername,
    &_SetPassword,
    &_SetPPPoEService,
//    &_SetConnectionTrigger,
    &_GetPortMappingNumberOfEntries,
    &_GetGenericPortMappingEntry,
    &_GetSpecificPortMappingEntry,
    &_AddPortMapping,
    &_DeletePortMapping,    
    &_GetStatistics,    
    NULL
};
ServiceTemplate Template_WANPPPConnection = {
    "WANPPPConnection:",
    NULL,    
    WANPPPConnection_GetVar,    
    NULL,           
    ARRAYSIZE(StateVariables)-1, 
    StateVariables,
    Actions, 
    0,
    "urn:dslforum-org:serviceId:WANPPPConnection"
};

