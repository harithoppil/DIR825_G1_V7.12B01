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
//  Filename:     lanethifcfghandlers.c  
//
******************************************************************************/

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "lanethifcfgparams.h"
#include "cmmif.h"

#define GetLANEthIFCfgValue(szLeafName) \
do{\
    snprintf(szFullPathName, sizeof(szFullPathName), TR064_ROOT_LanEthIf"%d.%s" ,psvc->instance,szLeafName);\
  	if(CMM_GetStr(szFullPathName, var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)\
	{\
	      TR64FE_TRACE("Could not get current LANEthIFCfg %s ",szLeafName);\
	      return FALSE;\
	}\
	if( 0 == strcmp(var->value,""))\
   	{\
   	  strcpy(var->value ,"0");\
   	}\
	return TRUE; \
}while(0)

int LANEthIFCfg_GetVar(struct Service *psvc, int varindex)
{
   char szFullPathName[CMM_MAX_NODE_LEN];
   struct StateVar *var;
   var = &(psvc->vars[varindex]);
   switch (varindex)
   {
      case VAR_Enable:
         GetLANEthIFCfgValue("Enable");
         break;
      case VAR_Status:
         GetLANEthIFCfgValue("Status");
         break;
      case VAR_MACAddress:
         GetLANEthIFCfgValue("MACAddress");
         break;
      case VAR_MACAddressControlEnabled:
         GetLANEthIFCfgValue("MACAddressControlEnabled");
		 strcpy(var->value ,"1");
         break;
      case VAR_MaxBitRate:
         GetLANEthIFCfgValue("MaxBitRate");
         break;
      case VAR_DuplexMode:
         GetLANEthIFCfgValue("DuplexMode");
         break;
      case VAR_BytesSent:
         GetLANEthIFCfgValue("Stats.BytesSent");
         break;
      case VAR_BytesReceived:
         GetLANEthIFCfgValue("Stats.BytesReceived");
         break;
      case VAR_PacketsSent:
         GetLANEthIFCfgValue("Stats.PacketsSent");
         break;
      case VAR_PacketsReceived:
         GetLANEthIFCfgValue("Stats.PacketsReceived");
         break;
   } 
   
   return TRUE;
}

int SetEnableLANETH(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szNodeName, sizeof(szNodeName), TR064_ROOT_LanEthIf"%d.Enable" ,psvc->instance);
   SetTr64Value(szNodeName,VAR_Enable);
}

int SetMaxBitRateLANETH(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szNodeName, sizeof(szNodeName), TR064_ROOT_LanEthIf"%d.MaxBitRate" ,psvc->instance);
   SetTr64Value(szNodeName,VAR_MaxBitRate);
}
