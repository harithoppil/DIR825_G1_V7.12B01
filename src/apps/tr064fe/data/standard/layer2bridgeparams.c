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
//  Filename:       layer2bridgeparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "layer2bridgeparams.h"

static VarTemplate StateVariables[] = { 
    { "MaxBridgeEntries", "", VAR_USHORT }, 
    { "MaxFilterEntries", "" ,VAR_USHORT},
    { "MaxMarkingEntries", "", VAR_USHORT},
    { "BridgeNumberOfEntries", "", VAR_USHORT},    
    { "BridgeKey", "", VAR_USHORT},        
    { "BridgeEnable", "", VAR_BOOL},
    { "BridgeStatus", "", VAR_STRING},            
    { "BridgeName", "", VAR_STRING},            
    { "VLANID", "", VAR_USHORT},                
    { "FilterNumberOfEntries", "", VAR_USHORT},
    { "FilterKey", "", VAR_USHORT},
    { "FilterEnable", "", VAR_USHORT},
    { "FilterStatus", "", VAR_BOOL},
    { "FilterBridgeReference", "", VAR_SHORT},                
    { "ExclusivityOrder", "", VAR_USHORT},
    { "FilterInterface", "", VAR_STRING},                
    { "VLANIDFilter", "", VAR_SHORT},
    { "AdmitOnlyVLANTagged", "", VAR_BOOL},
    { "EthertypeFilterList", "", VAR_STRING},                
    { "EthertypeFilterExclude", "", VAR_BOOL},
    { "SourceMACAddressFilterList", "", VAR_STRING},                
    { "SourceMACAddressFilterExclude", "", VAR_BOOL},
    { "DestMACAddressFilterList", "", VAR_STRING},                
    { "DestMACAddressFilterExclude", "", VAR_BOOL},
    { "SourceMACFromVendorClassIDFilter", "", VAR_STRING},                
    { "SourceMACFromVendorClassIDFilterExclude", "", VAR_BOOL},                    
    { "DestMACFromVendorClassIDFilter", "", VAR_STRING},                
    { "DestMACFromVendorClassIDFilterExclude", "", VAR_BOOL},                    
    { "SourceMACFromClientIDFilter", "", VAR_STRING},                
    { "SourceMACFromClientIDFilterExclude", "", VAR_BOOL},                    
    { "DestMACFromClientIDFilter", "", VAR_STRING},                
    { "DestMACFromClientIDFilterExclude", "", VAR_BOOL},                    
    { "SourceMACFromUserClassIDFilter", "", VAR_STRING},                
    { "SourceMACFromUserClassIDFilterExclude", "", VAR_BOOL},                    
    { "DestMACFromUserClassIDFilter", "", VAR_STRING},                
    { "DestMACFromUserClassIDFilterExclude", "", VAR_BOOL},                    
    { "MarkingNumberOfEntries", "", VAR_USHORT},                    
    { "MarkingKey", "", VAR_USHORT},                        
    { "MarkingEnable", "", VAR_BOOL},
    { "MarkingStatus", "", VAR_STRING},                        
    { "MarkingBridgeReference", "", VAR_SHORT},                        
    { "MarkingInterface", "", VAR_STRING},    
    { "VLANIDUntag", "", VAR_BOOL},                        
    { "VLANIDMark", "", VAR_SHORT},                            
    { "EthernetPriorityMark", "", VAR_SHORT},                            
    { "EthernetPriorityOverride", "", VAR_BOOL},
    { "AvailableInterfaceNumberOfEntries", "", VAR_USHORT},                            
    { "AvailableInterfaceKey", "", VAR_USHORT},                            
    { "InterfaceType", "", VAR_STRING},
    { "InterfaceReference", "", VAR_STRING},                            
    { NULL }
};

static Action _GetInfo = { 
   "GetInfo", GetLayer2Bridge_Info,
   (Param []) {
      {"NewMaxBridgeEntries", VAR_MaxBridgeEntries, VAR_OUT},
      {"NewMaxFilterEntries", VAR_MaxFilterEntries, VAR_OUT},
      {"NewMaxMarkingEntries", VAR_MaxMarkingEntries, VAR_OUT},      
      {"NewBridgeNumberOfEntries", VAR_BridgeNumberOfEntries, VAR_OUT},      
      {"NewFilterNumberOfEntries", VAR_FilterNumberOfEntries, VAR_OUT},      
      {"NewMarkingNumberOfEntries", VAR_MarkingNumberOfEntries, VAR_OUT},      
      {"NewAvailableInterfaceNumberOfEntries", VAR_AvailableInterfaceNumberOfEntries, VAR_OUT},      
      { 0 }
   }
};

static Action _AddBridgeEntry = { 
   "AddBridgeEntry", AddBridgeEntry,
   (Param []) {
      {"NewBridgeEnable", VAR_BridgeEnable, VAR_IN},
      {"NewBridgeName", VAR_BridgeName, VAR_IN},
      {"NewVLANID", VAR_VLANID, VAR_IN},      
      {"NewBridgeKey", VAR_BridgeKey, VAR_OUT},
      { 0 }
   }
};

static Action _DeleteBridgeEntry = { 
   "DeleteBridgeEntry", DeleteBridgeEntry,
   (Param []) {
      {"NewBridgeKey", VAR_BridgeKey, VAR_IN},
      { 0 }
   }
};

static Action _GetSpecificBridgeEntry = { 
   "GetSpecificBridgeEntry", GetSpecificBridgeEntry,
   (Param []) {
      {"NewBridgeKey", VAR_BridgeKey, VAR_IN},
      {"NewBridgeEnable", VAR_BridgeEnable, VAR_OUT},
      {"NewBridgeStatus", VAR_BridgeStatus, VAR_OUT},      
      {"NewVLANID", VAR_VLANID, VAR_OUT},      
      { 0 }
   }
};


static Action _GetGenericBridgeEntry = { 
   "GetGenericBridgeEntry", GetGenericBridgeEntry,
   (Param []) {
      {"NewBridgeIndex", VAR_BridgeNumberOfEntries, VAR_IN},   
      {"NewBridgeKey", VAR_BridgeKey, VAR_OUT},
      {"NewBridgeEnable", VAR_BridgeEnable, VAR_OUT},
      {"NewBridgeStatus", VAR_BridgeStatus, VAR_OUT},      
      {"NewVLANID", VAR_VLANID, VAR_OUT},      
      { 0 }
   }
};

static Action _SetBridgeEntryEnable = { 
   "SetBridgeEntryEnable", SetBridgeEntryEnable,
   (Param []) {
      {"NewBridgeKey", VAR_BridgeKey, VAR_IN},
      {"NewBridgeEnable", VAR_BridgeEnable, VAR_IN},
      { 0 }
   }
};

static Action _GetSpecificAvailableInterfaceEntry = { 
   "GetSpecificAvailableInterfaceEntry", GetSpecificAvailableInterfaceEntry,
   (Param []) {
      {"NewAvailableInterfaceKey", VAR_AvailableInterfaceKey, VAR_IN},
      {"NewInterfaceType", VAR_InterfaceType, VAR_OUT},
      {"NewInterfaceReference", VAR_InterfaceReference, VAR_OUT},
      { 0 }
   }
};

static Action _GetGenericAvailableInterfaceEntry = { 
   "GetGenericAvailableInterfaceEntry", GetGenericAvailableInterfaceEntry,
   (Param []) {
      {"NewAvailableInterfaceIndex", VAR_AvailableInterfaceNumberOfEntries, VAR_IN},   
      {"NewAvailableInterfaceKey", VAR_AvailableInterfaceKey, VAR_OUT},
      {"NewInterfaceType", VAR_InterfaceType, VAR_OUT},
      {"NewInterfaceReference", VAR_InterfaceReference, VAR_OUT},
      { 0 }
   }
};

static PAction Actions[] = {
    &_GetInfo,
    &_AddBridgeEntry,
    &_DeleteBridgeEntry,
    &_GetSpecificBridgeEntry,    
    &_GetGenericBridgeEntry,
    &_SetBridgeEntryEnable,
    &_GetSpecificAvailableInterfaceEntry,
    &_GetGenericAvailableInterfaceEntry,    
    NULL
};


ServiceTemplate Template_Layer2Bridging = {
    "Layer2Bridging:1",
    NULL, 
    NULL,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, 
    StateVariables,
    Actions
};

