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
//  Filename:       lancfgsecparams.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "lancfgsecparams.h"


static VarTemplate StateVariables[] = { 
    { "ConfigPassword", "", VAR_STRING },   

    { NULL }
};





static Action _SetConfigPassword = {
    "SetConfigPassword", SetConfigPassword,
   (Param []) {
       {"NewConfigPassword", VAR_ConfigPassword, VAR_IN},
       { 0 }
    }
};

static PAction Actions[] = {
    &_SetConfigPassword,
    NULL
};


ServiceTemplate Template_LANConfigSecurity = {
    "LANConfigSecurity:1",
    NULL, 
    NULL,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, 
    StateVariables,
    Actions,
    0,
    "LANConfigSecurity"
};
