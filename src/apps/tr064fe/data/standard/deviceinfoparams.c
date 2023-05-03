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
//  Filename:       deviceinfoparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "deviceinfoparams.h"


static VarTemplate StateVariables[] = { 
    { "Manufacturer", "", VAR_EVENTED|VAR_STRING },   
    { "ManufacturerOUI", "" ,VAR_EVENTED|VAR_STRING},
    { "ModelName", "", VAR_EVENTED|VAR_STRING},
    { "Description", "", VAR_EVENTED|VAR_STRING},
    { "ProductClass", "", VAR_EVENTED|VAR_STRING},	
    { "SerialNumber", "", VAR_EVENTED|VAR_STRING},		    
    { "SoftwareVersion",  "", VAR_EVENTED|VAR_STRING },
    { "HardwareVersion", "", VAR_EVENTED|VAR_STRING },
    { "SpecVersion", "", VAR_EVENTED|VAR_STRING },
    { "ProvisioningCode", "", VAR_EVENTED|VAR_STRING },
    { "UpTime", "", VAR_EVENTED|VAR_STRING },
    { "DeviceLog", "", VAR_EVENTED|VAR_STRING },
    { NULL }
};



static Action _GetIfno = {
    "GetInfo", GetInfo,
   (Param []) {
       {"NewManufacturer", VAR_Manufacturer, VAR_OUT},
       {"NewManufacturerOUI", VAR_ManufacturerOUI, VAR_OUT},
       {"NewModelName", VAR_ModelName, VAR_OUT},	
       {"NewDescription", VAR_Description, VAR_OUT},	
       {"NewProductClass", VAR_ProductClass, VAR_OUT},
       {"NewSerialNumber", VAR_SerialNumber, VAR_OUT},
       {"NewSoftwareVersion", VAR_SoftwareVersion, VAR_OUT},
       {"NewHardwareVersion", VAR_HardwareVersion, VAR_OUT},
       {"NewSpecVersion", VAR_SpecVersion, VAR_OUT},
       {"NewProvisioningCode", VAR_ProvisioningCode, VAR_OUT},
       {"NewUpTime", VAR_UpTime, VAR_OUT},
       { 0 }
    }
};
static Action _SetProvisioningCode = {
    "SetProvisioningCode", SetProvisioningCode,
   (Param []) {
	   {"NewProvisioningCode", VAR_ProvisioningCode, VAR_IN},
       { 0 }
    }
};

static Action _GetDeviceLog = {
    "GetDeviceLog", GetDeviceLog,
   (Param []) {
	   {"NewDeviceLog", VAR_DeviceLog, VAR_OUT},
       { 0 }
    }
};

static PAction Actions[] = {
    &_GetIfno,
    &_SetProvisioningCode,
    &_GetDeviceLog,
    NULL
};


ServiceTemplate Template_DeviceInfo = {
    "DeviceInfo:1",
    NULL, 
    DeviceInfo_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, 
    StateVariables,
    Actions,
    0,
    "urn:dslforum-org:service:DeviceInfo"
};

