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
//  Filename:       wanethlinkconfighandlers.c
//
******************************************************************************/
#ifdef INCLUDE_WANETHERNETLINKCONFIG
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"

#include "wanethlinkconfig.h"
#include "tr64defs.h"
#include "syscall.h"

int WANETHLinkConfig_GetVar(struct Service *psvc, int varindex)
{  
   #if 0  //not surpport yujinshi
   void *info;
   uint32 index = 1;
   char EthernetLinkStatus[32];	
	
   struct StateVar *var;
   var = &(psvc->vars[varindex]);

   if ( BcmCfm_stsGet(BCMCFM_OBJ_IFC_ETHERNET, &info, &index) == BcmCfm_Ok ) 
   {
      PBcmCfm_EthIfcSts_t pEth = (PBcmCfm_EthIfcSts_t)info;
	  
      //Status
      if ( pEth->linkState == BcmCfm_LinkUp )
      {
         strcpy(EthernetLinkStatus, "Up");
      }
      else
      {
         strcpy(EthernetLinkStatus, "Disabled");
      }
   
      BcmCfm_stsFree(BCMCFM_OBJ_IFC_ETHERNET, info);
   }
	
   var = &(psvc->vars[varindex]);
   switch (varindex)
   {
      case VAR_EthernetLinkStatus:
      strcpy(var->value, EthernetLinkStatus);	  	
      break;
   } 
   #endif

   return TRUE;
}
#endif
