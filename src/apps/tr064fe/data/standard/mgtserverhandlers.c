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
//  Filename:       mgtserverhandlers.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "mgtserverparams.h"
#include "tr64defs.h"
#include <time.h>
#include <sys/time.h>
#include "cmmif.h"


int MgtServer_GetVar(struct Service *psvc, int varindex)
{
   struct StateVar *var;
   var = &(psvc->vars[varindex]);

   switch (varindex) 
   {
      case VAR_URL:
	  	 if(CMM_GetStr(TR064_ROOT_ManagementServer"URL", var->value, sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current ReservedAddresses ");
		      return FALSE;
	     }
      break;

      case VAR_PeriodicInformEnable:
         if(CMM_GetStr(TR064_ROOT_ManagementServer"PeriodicInformEnable", var->value, sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current PeriodicInformEnable ");
		      return FALSE;
	     }
      break;

      case VAR_PeriodicInformInterval:
         if(CMM_GetStr(TR064_ROOT_ManagementServer"PeriodicInformInterval", var->value, sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current PeriodicInformInterval ");
		      return FALSE;
	     }
      break;

      case VAR_PeriodicInformTime:
         if(CMM_GetStr(TR064_ROOT_ManagementServer"PeriodicInformTime", var->value, sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current PeriodicInformTime ");
		      return FALSE;
	     }
      break;
      
      case VAR_ParameterKey:
         if(CMM_GetStr(TR064_ROOT_ManagementServer"ParameterKey", var->value, sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current ParameterKey ");
		      return FALSE;
	     }
		 
      break;
      
      case VAR_ParameterHash:
         /*
		 if(CMM_GetStr(TR064_ROOT_ManagementServer"ParameterHash", var->value, sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current ParameterHash ");
		      return FALSE;
	     }
	     */
	      strcpy(var->value, "");
      break;   

      case VAR_ConnectionRequestURL:
         if(CMM_GetStr(TR064_ROOT_ManagementServer"ConnectionRequestURL", var->value, sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current ReservedAddresses ");
		      return FALSE;
	     }
      break;   

      case VAR_ConnectionRequestUsername:
         if(CMM_GetStr(TR064_ROOT_ManagementServer"ConnectionRequestUsername", var->value, sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current ConnectionRequestUsername ");
		      return FALSE;
	     }
      break;      

      case VAR_UpgradesManaged:
         strcpy(var->value, "");
      break;         
    }

    if(0 == strcmp(var->value,""))
    {
       strcpy(var->value, "0");
    }

    return TRUE;
}

int SetUpgradesManagement(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   return TRUE;
}

int SetManagementServerURL(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   SetTr64Value(TR064_ROOT_ManagementServer"URL",VAR_URL);
}

int SetManagementServerPassword(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{   
   SetTr64Value(TR064_ROOT_ManagementServer"Password",VAR_Password);
}

int SetPeriodicInform(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szInformInterval[80];
   snprintf(szInformInterval ,sizeof(szInformInterval),TR064_ROOT_ManagementServer"PeriodicInformInterval");
   
   char szInformTime[80];
   snprintf(szInformTime ,sizeof(szInformTime),TR064_ROOT_ManagementServer"PeriodicInformTime");

   char szInformEnable[80];
   snprintf(szInformEnable ,sizeof(szInformEnable),TR064_ROOT_ManagementServer"PeriodicInformEnable");

   char* szLeafNames[3]={szInformInterval,szInformTime,szInformEnable};
   int  relatedVars[3] = {VAR_PeriodicInformInterval,VAR_PeriodicInformTime, VAR_PeriodicInformEnable};
   SetTr64Values(szLeafNames,relatedVars ,3);
}

int SetConnectionRequestAuthentication(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szConnRequestUsr[80];
   snprintf(szConnRequestUsr ,sizeof(szConnRequestUsr),TR064_ROOT_ManagementServer"ConnectionRequestUsername");
   
   char szConnRequestPwd[80];
   snprintf(szConnRequestPwd ,sizeof(szConnRequestPwd),TR064_ROOT_ManagementServer"ConnectionRequestPassword");

   char* szLeafNames[2]={szConnRequestUsr,szConnRequestPwd};
   int  relatedVars[2] = {VAR_ConnectionRequestUsername, VAR_ConnectionRequestPassword};
   SetTr64Values(szLeafNames,relatedVars ,2);

   return TRUE;
}
