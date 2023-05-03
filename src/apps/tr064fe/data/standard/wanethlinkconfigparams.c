
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
//  Filename:       wanethlinkconfigparams.c
//
******************************************************************************/
#ifdef INCLUDE_WANETHERNETLINKCONFIG
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "wanethlinkconfig.h"


static VarTemplate StateVariables[] = {
   { "EthernetLinkStatus", "", VAR_STRING }, 
   { 0 } 
};

static Action _GetEthernetLinkStatus = { 
   "GetEthernetLinkStatus", GetEthernetLinkStatus,
   (Param [])    {
      {"NewEthernetLinkStatus", VAR_EthernetLinkStatus, VAR_OUT},
      { 0 }
   }
};

static PAction Actions[] = {
   &_GetEthernetLinkStatus,
   NULL
};


ServiceTemplate Template_WANETHLinkConfig = {
   "WANETHLinkConfig:1",
   NULL,
   WANETHLinkConfig_GetVar,    /* state variable handler */
   NULL,  /* SVCXML */
   ARRAYSIZE(StateVariables)-1, 
   StateVariables,
   Actions,
   0,
   "urn:dslforum-org:serviceId:WANETHLinkConfig"
};
#endif
