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
//  Filename:       timehandlers.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "timeparams.h"
#include "tr64defs.h"
#include "cmmif.h"

#define GetTimeValue(szLeafName) \
do{\
	char szFullPathName[60] ={0};\
    snprintf(szFullPathName, sizeof(szFullPathName), TR064_ROOT_TimeCfg"%s" ,szLeafName);\
  	if(CMM_GetStr(szFullPathName, var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)\
	{\
	      TR64FE_TRACE("Could not get current time %s ",szLeafName);\
	      return FALSE;\
	}\
	return TRUE; \
}while(0)

int TimeServer_GetVar(struct Service *psvc, int varindex)
{
   printf("enter into TimeServer_GetVar ()");
  
   struct StateVar *var;
   var = &(psvc->vars[varindex]);
   
   switch (varindex)
   {
      case VAR_NTPServer1:
	  	 GetTimeValue("NTPServer1");     
      break;

      case VAR_NTPServer2:
         GetTimeValue("NTPServer2");
      break;

      case VAR_CurrentLocalTime:
	  	 GetTimeValue("CurrentLocalTime");
      break;

      case VAR_LocalTimeZone:
	  	 GetTimeValue("LocalTimeZone");
      break;

      case VAR_LocalTimeZoneName:
	  	 GetTimeValue("LocalTimeZoneName");
      break;

      case VAR_DaylightSavingsUsed:
         GetTimeValue("DaylightSavingsUsed");
      break;

      case VAR_DaylightSavingsStart:
         GetTimeValue("DaylightSavingStart");
      break;

      case VAR_DaylightSavingsEnd:
         GetTimeValue("DaylightSavingsEnd");
      break;
   } 

   return TRUE;
}

int SetNTPServers(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNTPServer1[60]= {TR064_ROOT_TimeCfg"NTPServer1"} ;
   char szNTPServer2[60]= {TR064_ROOT_TimeCfg"NTPServer2"} ;
   
   char* szLeafNames[2]={szNTPServer1,szNTPServer2};
   
   int  relatedVars[6] = {VAR_NTPServer1,VAR_NTPServer2};
   SetTr64Values(szLeafNames,relatedVars ,2);
}

int SetLocalTimeZone(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szLocalTimeZone[60]={TR064_ROOT_TimeCfg"LocalTimeZone"};
   char szLocalTimeZoneName[60] ={TR064_ROOT_TimeCfg"LocalTimeZoneName"};
   char szDaylightSavingsUsed[60]={TR064_ROOT_TimeCfg"DaylightSavingsStart"};
   char szDaylightSavingsStart[60]={TR064_ROOT_TimeCfg"LocalTimeZoneName"};
   char szDaylightSavingsEnd[60]={TR064_ROOT_TimeCfg"DaylightSavingsEnd"};

   char* szLeafNames[5]={szLocalTimeZone,szLocalTimeZoneName,szDaylightSavingsUsed,szDaylightSavingsStart,szDaylightSavingsEnd};
   int  relatedVars[5] = {VAR_LocalTimeZone,VAR_LocalTimeZoneName,VAR_DaylightSavingsUsed,VAR_DaylightSavingsStart,VAR_DaylightSavingsEnd};
   SetTr64Values(szLeafNames,relatedVars ,5);
  
}
