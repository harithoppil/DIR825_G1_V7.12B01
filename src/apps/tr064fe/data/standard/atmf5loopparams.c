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
//  Filename:       atmf5loopparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "atmf5loopparams.h"

static VarTemplate StateVariables[] = {
    { "DiagnosticsState", "", VAR_STRING},
    { "NumberOfRepetitions", "", VAR_ULONG},    
    { "Timeout", "", VAR_ULONG},            
    { "SuccessCount",  "", VAR_ULONG },
    { "FailureCount", "", VAR_ULONG },
    { "AverageResponseTime", "", VAR_ULONG },
    { "MinimumResponseTime", "", VAR_ULONG },
    { "MaximumResponseTime", "", VAR_ULONG },
    { NULL }
};

static Action _GetInfo = {
    "GetInfo", GetInfo,
   (Param []) {
        { "NewDiagnosticsState", VAR_DiagnosticsState, VAR_OUT},
        { "NewNumberOfRepetitions", VAR_NumberOfRepetitions, VAR_OUT},    
        { "NewTimeout", VAR_Timeout, VAR_OUT},            
        { "NewSuccessCount", VAR_SuccessCount, VAR_OUT },
        { "NewFailureCount", VAR_FailureCount, VAR_OUT },
        { "NewAverageResponseTime", VAR_AverageResponseTime, VAR_OUT },
        { "NewMinimumResponseTime", VAR_MinimumResponseTime, VAR_OUT },
        { "NewMaximumResponseTime", VAR_MaximumResponseTime, VAR_OUT },
        { 0 }
    }
};

static Action _SetDiagnosticsState = { 
    "SetDiagnosticsState", SetDiagnosticsStateATMF5,
        (Param []) {
            {"NewDiagnosticsState", VAR_DiagnosticsState, VAR_IN},
            { 0 }
        }
};


static Action _SetNumberOfRepetitions = { 
    "SetNumberOfRepetitions", SetNumberOfRepetitionsATMF5,
        (Param []) {
            {"NewNumberOfRepetitions", VAR_NumberOfRepetitions, VAR_IN},
            { 0 }
        }
};

static Action _SetTimeout = { 
    "SetTimeout", SetTimeoutATMF5,
        (Param []) {
            {"NewTimeout", VAR_Timeout, VAR_IN},
            { 0 }
        }
};

static PAction Actions[] = {
   &_GetInfo,
   &_SetDiagnosticsState,
   &_SetNumberOfRepetitions,
   &_SetTimeout,
    NULL
};

ServiceTemplate Template_WANATMF5LoopbackDiagnostics = {
    "WANATMF5LoopbackDiagnostics:1",
    NULL,
    WANATMF5LoopbackDiagnostics_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, StateVariables,
    Actions,
    0,
    "urn:dslforum-org:service:WANATMF5LoopbackDiagnostics"
};

