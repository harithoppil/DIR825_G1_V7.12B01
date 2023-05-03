#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "layer3params.h"


static VarTemplate StateVariables[] = { 
    { "DefaultConnectionService", "", VAR_EVENTED|VAR_STRING }, 
    { "ForwardNumerOfEntries", "", VAR_USHORT}, 
    { "Enable", "1", VAR_EVENTED|VAR_BOOL},
    { "Status", "Disabled", VAR_EVENTED|VAR_STRING},    
    { "Type", "", VAR_EVENTED|VAR_STRING},            
    { "DestIPAddress",  "", VAR_EVENTED|VAR_STRING },
    { "DestSubnetMask", "", VAR_EVENTED|VAR_STRING },
    { "SourceIPAddress", "", VAR_EVENTED|VAR_STRING },
    { "SourceSubnetMask", "", VAR_EVENTED|VAR_STRING },
    { "GatewayIPAddress", "", VAR_EVENTED|VAR_STRING },
    { "Interface", "", VAR_EVENTED|VAR_STRING },
    { "ForwardingMetric", "", VAR_EVENTED|VAR_LONG },    
    { "MTU", "", VAR_USHORT },
    { NULL }
};

static Action _GetDefaultConnectionService = { 
   "GetDefaultConnectionService", GetDefaultConnectionService,
   (Param []) {
      {"NewDefaultConnectionService", VAR_DefaultConnectionService, VAR_OUT},
      { 0 }
   }
};

static Action _SetDefaultConnectionService = { 
   "SetDefaultConnectionService", SetDefaultConnectionService,
   (Param []) {
      {"NewDefaultConnectionService", VAR_DefaultConnectionService, VAR_IN},
      { 0 }
   }
};
static Action _GetForwardNumberOfEntries = {
    "GetForwardNumberOfEntries", GetForwardNumberOfEntries,
   (Param []) {
       {"NewForwardNumberOfEntries", VAR_ForwardNumberOfEntries, VAR_OUT},
       { 0 }
    }
};

static Action _AddForwardingEntry = {
    "AddForwardingEntry", AddForwardingEntry,
        
   (Param []) {
//       {"NewType", VAR_Type, VAR_IN},
       {"NewDestIPAddress", VAR_DestIPAddress, VAR_IN},
       {"NewDestSubnetMask", VAR_DestSubnetMask, VAR_IN},
//       {"NewSourceIPAddress", VAR_SourceIPAddress, VAR_IN},
//       {"NewSourceSubnetMask", VAR_SourceSubnetMask, VAR_IN},
       {"NewGatewayIPAddress", VAR_GatewayIPAddress, VAR_IN},
       {"NewInterface", VAR_Interface, VAR_IN},
//       {"NewForwardingMetric", VAR_ForwardingMetric, VAR_IN},
//       {"NewMTU", VAR_MTU, VAR_IN},
       { 0 }
    }
};

static Action _DeleteForwardingEntry = {
    "DeleteForwardingEntry", DeleteForwardingEntry,
   (Param []) {
       {"NewDestIPAddress", VAR_DestIPAddress, VAR_IN},
       {"NewDestSubnetMask", VAR_DestSubnetMask, VAR_IN},
//     {"NewSourceIPAddress", VAR_SourceIPAddress, VAR_IN},
//     {"NewSourceSubnetMask", VAR_SourceSubnetMask, VAR_IN},       
       { 0 }
    }
};

static Action _GetSpecificForwardingEntry = {
    "GetSpecificForwardingEntry", GetSpecificForwardingEntry,
   (Param []) {
       {"NewDestIPAddress", VAR_DestIPAddress, VAR_IN},
       {"NewDestSubnetMask", VAR_DestSubnetMask, VAR_IN},
//       {"NewSourceIPAddress", VAR_SourceIPAddress, VAR_IN},
//       {"NewSourceSubnetMask", VAR_SourceSubnetMask, VAR_IN},   
       {"NewGatewayIPAddress", VAR_GatewayIPAddress, VAR_OUT},
       {"NewEnable", VAR_Enable, VAR_OUT},
       {"NewStatus", VAR_Status, VAR_OUT},    
//       {"NewType", VAR_Type, VAR_OUT},
       {"NewInterface", VAR_Interface, VAR_OUT}, 
//       {"NewForwardingMetric", VAR_ForwardingMetric, VAR_OUT},
//       {"NewMTU", VAR_MTU, VAR_OUT},       
       { 0 }
    }
};

static Action _GetGenericForwardingEntry = {
    "GetGenericForwardingEntry", GetGenericForwardingEntry,
   (Param []) {
       {"NewForwardNumberOfEntries", VAR_ForwardNumberOfEntries, VAR_IN},
       {"NewEnable", VAR_Enable, VAR_OUT},
       {"NewStatus", VAR_Status, VAR_OUT},    
  //     {"NewType", VAR_Type, VAR_OUT},
       {"NewDestIPAddress", VAR_DestIPAddress, VAR_OUT},
       {"NewDestSubnetMask", VAR_DestSubnetMask, VAR_OUT},
//       {"NewSourceIPAddress", VAR_SourceIPAddress, VAR_OUT},
//       {"NewSourceSubnetMask", VAR_SourceSubnetMask, VAR_OUT},
       {"NewGatewayIPAddress", VAR_GatewayIPAddress, VAR_OUT},
       {"NewInterface", VAR_Interface, VAR_OUT}, 
//       {"NewForwardingMetric", VAR_ForwardingMetric, VAR_OUT},
//       {"NewMTU", VAR_MTU, VAR_OUT},       
       { 0 }
    }
};

static Action _SetForwardingEntryEnable = {
    "SetForwardingEntryEnable", SetForwardingEntryEnable,
   (Param []) {
       {"NewDestIPAddress", VAR_DestIPAddress, VAR_IN},
       {"NewDestSubnetMask", VAR_DestSubnetMask, VAR_IN},
//       {"NewSourceIPAddress", VAR_SourceIPAddress, VAR_IN},
//       {"NewSourceSubnetMask", VAR_SourceSubnetMask, VAR_IN},   
       {"NewEnable", VAR_Enable, VAR_IN},  
       { 0 }
    }
};

static PAction Actions[] = {
    &_GetDefaultConnectionService,
    &_SetDefaultConnectionService,
    &_GetForwardNumberOfEntries,
    &_AddForwardingEntry,
    &_DeleteForwardingEntry,
    &_GetSpecificForwardingEntry,
    //&_GetGenericForwardingEntry,
    &_SetForwardingEntryEnable,
    NULL
};


ServiceTemplate Template_Layer3Forwarding = {
    "Layer3Forwarding:1",
    Layer3Forwarding_Init, 
    Layer3_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, 
    StateVariables,
    Actions
};



