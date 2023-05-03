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
//  Filename:       lanusbifcfghandlers.c
//
******************************************************************************/
#ifdef USB
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "lanusbifcfgparams.h"
#include "cmmif.h"

#define GetLANUSBIFCfgValue(szLeafName) \
do{\
    snprintf(szFullPathName, sizeof(szFullPathName), TR064_ROOT_LanUSBIfCfg"%d.%s" ,psvc->instance,szLeafName);\
  	if(CMM_GetStr(szFullPathName, var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)\
	{\
	      TR64FE_TRACE("Could not get current LANUSBIFCfg %s ",szLeafName);\
	      return FALSE;\
	}\
	if(0 == strcmp(var->value,""))\
	{\
	    strcpy(var->value,"0");\
	}\
	return TRUE; \
}while(0)


int LANUSBIFCfg_GetVar(struct Service *psvc, int varindex)
{
   char szFullPathName[90]={0} ;
   struct StateVar *var;
   var = &(psvc->vars[varindex]);
   switch (varindex)
   {
      case VAR_Enable:
         GetLANUSBIFCfgValue("Enable");
         break;
      case VAR_Status:
         GetLANUSBIFCfgValue("Status");
         break;
      case VAR_MACAddress:
         GetLANUSBIFCfgValue("MACAddress");
         break;
      case VAR_MACAddressControlEnabled:
         GetLANUSBIFCfgValue("MACAddressControlEnabled");
         break;
      case VAR_Standard:
         GetLANUSBIFCfgValue("Standard");
         break;
      case VAR_Type:
         GetLANUSBIFCfgValue("Type");
         break;
      case VAR_Rate:
         GetLANUSBIFCfgValue("Rate");
         break;
      case VAR_BytesSent:
         GetLANUSBIFCfgValue("Stats.BytesSent");
         break;
      case VAR_BytesReceived:
         GetLANUSBIFCfgValue("Stats.BytesReceived");
         break;
      case VAR_PacketsSent:
         GetLANUSBIFCfgValue("Stats.CellsSent");
         break;
      case VAR_PacketsReceived:
         GetLANUSBIFCfgValue("Stats.CellsReceived");
         break;
   }      
    return TRUE;
}

int SetUSBEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szNodeName, sizeof(szNodeName), TR064_ROOT_LanUSBIfCfg"%d.Enable" ,psvc->instance);
   SetTr64Value(szNodeName,VAR_Enable);
}
#endif

