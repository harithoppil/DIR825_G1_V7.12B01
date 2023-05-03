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
//  Filename:       mgtserverparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "mgtserverparams.h"


static VarTemplate StateVariables[] = { 
    { "URL", "", VAR_EVENTED|VAR_STRING },   
    { "Password", "" ,VAR_EVENTED|VAR_STRING},
    { "PeriodicInformaEnable", "", VAR_EVENTED|VAR_BOOL},
    { "PeriodicInformInterval", "", VAR_EVENTED|VAR_LONG},    
    { "PeriodicInformTime", "", VAR_EVENTED|VAR_STRING},            
    { "ParameterKey",  "", VAR_EVENTED|VAR_STRING },
    { "ParameterHash", "", VAR_EVENTED|VAR_STRING },
    { "ConnectionRequestURL", "", VAR_EVENTED|VAR_STRING },
    { "ConnectionRequestUsername", "", VAR_EVENTED|VAR_STRING },
    { "ConnectionRequestPassword", "", VAR_EVENTED|VAR_STRING },
    { "UpgradesManaged", "", VAR_EVENTED|VAR_BOOL },
    { "KickURL", "", VAR_EVENTED|VAR_STRING },
    { "DownloadProgressURL", "", VAR_EVENTED|VAR_STRING },
    { NULL }
};



static Action _GetIfno = {
    "GetInfo", GetInfo,
   (Param []) {
       {"NewURL", VAR_URL, VAR_OUT},
       {"NewPeriodicInformEnable", VAR_PeriodicInformEnable, VAR_OUT},
       {"NewPeriodicInformInterval", VAR_PeriodicInformInterval, VAR_OUT},    
       {"NewPeriodicInformTime", VAR_PeriodicInformTime, VAR_OUT},
       {"NewParameterKey", VAR_ParameterKey, VAR_OUT},
       {"NewParameterHash", VAR_ParameterHash, VAR_OUT},
       {"NewConnectionRequestURL", VAR_ConnectionRequestURL, VAR_OUT},
       {"NewConnectionRequestUsername", VAR_ConnectionRequestUsername, VAR_OUT},
       {"NewUpgradesManaged", VAR_UpgradesManaged, VAR_OUT},
       { 0 }
    }
};

static Action _SetManagementServerURL = {
    "SetManagementServerURL", SetManagementServerURL,
   (Param []) {
       {"NewURL", VAR_URL, VAR_IN},
       { 0 }
    }
};

static Action _SetUpgradesManagement = {
    "SetUpgradesManagement", SetUpgradesManagement,
   (Param []) {
       {"NewUpgradesManaged", VAR_UpgradesManaged, VAR_IN},
       { 0 }
    }
};

static Action _SetManagementServerPassword = {
    "SetManagementServerPassword", SetManagementServerPassword,
   (Param []) {
       {"NewPassword", VAR_Password, VAR_IN},
       { 0 }
    }
};

static Action _SetPeriodicInform = {
    "SetPeriodicInform", SetPeriodicInform,
   (Param []) {
       {"NewPeriodicInformEnable", VAR_PeriodicInformEnable, VAR_IN},
       {"NewPeriodicInformInterval", VAR_PeriodicInformInterval, VAR_IN},
       {"NewPeriodicInformTime", VAR_PeriodicInformTime, VAR_IN},
       { 0 }
    }
};

static Action _SetConnectionRequestAuthentication = {
    "SetConnectionRequestAuthentication", SetConnectionRequestAuthentication,
   (Param []) {
       {"NewConnectionRequestUsername", VAR_ConnectionRequestUsername, VAR_IN},
       {"NewConnectionRequestPassword", VAR_ConnectionRequestPassword, VAR_IN},
       { 0 }
    }
};

static PAction Actions[] = {
    &_GetIfno,
    &_SetManagementServerURL,    
    &_SetManagementServerPassword,
    //&_SetUpgradesManagement,
    &_SetPeriodicInform,
    &_SetConnectionRequestAuthentication,
    NULL
};


ServiceTemplate Template_MgtServer = {
    "ManagementServer:1",
    NULL, 
    MgtServer_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, 
    StateVariables,
    Actions,
    0,
    "urn:dslforum-org:service:ManagementServer"
};

