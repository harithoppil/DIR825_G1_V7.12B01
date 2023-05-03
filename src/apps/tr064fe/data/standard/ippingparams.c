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
//  Filename:       ippingparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "ippingparams.h"

static VarTemplate StateVariables[] = {
    { "DiagnosticsState", "", VAR_STRING},
    { "Interface", "", VAR_STRING},
    { "Host", "", VAR_STRING},
    { "NumberOfRepetitions", "", VAR_ULONG},
    { "Timeout", "", VAR_ULONG},
    { "DataBlockSize", "", VAR_ULONG},
    { "DSCP", "", VAR_ULONG},    
    { "SuccessCount", "", VAR_ULONG },    
    { "FailureCount",  "", VAR_ULONG },
    { "AverageResponseTime", "", VAR_ULONG},
    { "MinimumResponseTime", "", VAR_ULONG},    
    { "MaximumResponseTime", "", VAR_ULONG},
    { NULL }
};

static Action _GetInfo = {
    "GetInfo", GetIPPingInfo,
   (Param []) {
        { "NewDiagnosticsState", VAR_DiagnosticsState, VAR_OUT},
        { "NewInterface", VAR_Interface, VAR_OUT},    
        { "NewHost", VAR_Host, VAR_OUT},            
        { "NewNumberOfRepetitions",  VAR_NumberOfRepetitions, VAR_OUT },
        { "NewTimeout", VAR_Timeout, VAR_OUT },
        { "NewDataBlockSize", VAR_DataBlockSize, VAR_OUT },
        { "NewDSCP", VAR_DSCP, VAR_OUT },        
        { "NewSuccessCount", VAR_SuccessCount, VAR_OUT },        
        { "NewFailureCount", VAR_FailureCount, VAR_OUT },
        { "NewAverageResponseTime", VAR_AverageResponseTime, VAR_OUT },
        { "NewMinimumResponseTime", VAR_MinimumResponseTime, VAR_OUT },
        { "NewMaximumResponseTime", VAR_MaximumResponseTime, VAR_OUT },
        { 0 }
    }
};

static Action _SetDiagnosticsState = {
    "SetDiagnosticsState", SetDiagnosticsStateIPPing,
   (Param []) { 
       {"NewDiagnosticsState", VAR_DiagnosticsState, VAR_IN},  
       { 0 }
    }
};

static Action _SetInterface = {
    "SetInterface", SetInterfaceIPPing,
   (Param []) { 
       {"NewInterface", VAR_Interface, VAR_IN},       
       { 0 }
    }
};

static Action _SetHost = {
    "SetHost", SetHostIPPing,
   (Param []) { 
       {"NewHost", VAR_Host, VAR_IN}, 
       { 0 }
    }
};

static Action _NumberOfRepetitions = {
    "NumberOfRepetitions", NumberOfRepetitionsIPPing,
   (Param []) { 
       {"NewNumberOfRepetitions", VAR_NumberOfRepetitions, VAR_IN}, 
       { 0 }
    }
};

static Action _SetTimeout = {
    "SetTimeout", SetTimeoutIPPing,
   (Param []) { 
       {"NewTimeout", VAR_Timeout, VAR_IN}, 
       { 0 }
    }
};

static Action _SetDataBlockSize = {
    "SetDataBlockSize", SetDataBlockSizeIPPing,
   (Param []) { 
       {"NewDataBlockSize", VAR_DataBlockSize, VAR_IN}, 
       { 0 }
    }
};


static Action _SetDSCP = {
    "SetDSCP", SetDSCPIPPing,
   (Param []) { 
       {"NewDSCP", VAR_DSCP, VAR_IN}, 
       { 0 }
    }
};

static PAction Actions[] = {
    &_GetInfo,
    &_SetDiagnosticsState,
    &_SetInterface,
    &_SetHost,
    &_NumberOfRepetitions,
    &_SetTimeout,
    &_SetDataBlockSize,
    &_SetDSCP,
    NULL
};

ServiceTemplate Template_IPPingConfig = {
    "IPPingDiagnostics:1",
    NULL,
    IPPing_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, StateVariables,
    Actions,
    0,
    "urn:dslforum-org:serviceId:IPPingDiagnostics"
};

