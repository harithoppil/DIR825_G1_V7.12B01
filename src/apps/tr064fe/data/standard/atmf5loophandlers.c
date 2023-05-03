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
//  Filename:       atmf5loophandlers.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "atmf5loopparams.h"

#include "cms_util.h"
#include "cms_core.h"
#include "cms_msg.h"

int WANATMF5LoopbackDiagnostics_GetVar(struct Service *psvc, int varindex)
{
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   WanAtm5LoopbackDiagObject *obj;
   CmsRet ret = CMSRET_INVALID_ARGUMENTS;

   struct StateVar *var;

   char DiagnosticsState[32];
   unsigned int NumberOfRepetitions;
   unsigned int Timeout;
   unsigned int SuccessCount;
   unsigned int FailureCount;
   unsigned int AverageResponseTime;
   unsigned int MinimumResponseTime;
   unsigned int MaximumResponseTime;

   int instanceOfWANDevice;
   int instanceOfWANConnectionDevice;
   instanceOfWANDevice = psvc->device->parent->instance;
   instanceOfWANConnectionDevice = psvc->device->instance;

   PUSH_INSTANCE_ID(&iidStack, instanceOfWANDevice);
   PUSH_INSTANCE_ID(&iidStack, instanceOfWANConnectionDevice);

   if ((ret = cmsObj_get(MDMOID_WAN_ATM5_LOOPBACK_DIAG, &iidStack, 0, (void **) &obj)) == CMSRET_SUCCESS)
   {  
      if(obj->diagnosticsState)
      {
         strcpy(DiagnosticsState, obj->diagnosticsState);
      }
      NumberOfRepetitions = obj->numberOfRepetitions;
      Timeout = obj->timeout;
      SuccessCount = obj->successCount;
      FailureCount = obj->failureCount;
      AverageResponseTime = obj->averageResponseTime;
      MinimumResponseTime = obj->minimumResponseTime;
      MaximumResponseTime = obj->maximumResponseTime;

      cmsObj_free((void **) &obj);
   }
   else
   {
      return FALSE;
   }

   var = &(psvc->vars[varindex]);
   switch (varindex)
   {
      case VAR_DiagnosticsState:
         strcpy(var->value, DiagnosticsState);        
      break;

      case VAR_NumberOfRepetitions:
         sprintf(var->value, "%d", NumberOfRepetitions);
      break;

      case VAR_Timeout:
         sprintf(var->value, "%d", Timeout);
      break;

      case VAR_SuccessCount:
         sprintf(var->value, "%d", SuccessCount);
      break;

      case VAR_FailureCount:
         sprintf(var->value, "%d", FailureCount);
      break;

      case VAR_AverageResponseTime:
         sprintf(var->value, "%d", AverageResponseTime);
      break;

      case VAR_MinimumResponseTime:
         sprintf(var->value, "%d", MinimumResponseTime);
      break;

      case VAR_MaximumResponseTime:
         sprintf(var->value, "%d", MaximumResponseTime);
      break;

   } 

   return TRUE;
}

int SetDiagnosticsStateATMF5(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   WanAtm5LoopbackDiagObject *obj;
   CmsRet ret = CMSRET_INVALID_ARGUMENTS;

   struct Param *pParams;

   int instanceOfWANDevice;
   int instanceOfWANConnectionDevice;
   instanceOfWANDevice = psvc->device->parent->instance;
   instanceOfWANConnectionDevice = psvc->device->instance;

   PUSH_INSTANCE_ID(&iidStack, instanceOfWANDevice);
   PUSH_INSTANCE_ID(&iidStack, instanceOfWANConnectionDevice);
   
   pParams = findActionParamByRelatedVar(ac,VAR_DiagnosticsState);
   if (pParams == NULL)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   if ((ret = cmsObj_get(MDMOID_WAN_ATM5_LOOPBACK_DIAG, &iidStack, 0, (void **) &obj)) == CMSRET_SUCCESS)
   {   
      cmsMem_free(obj->diagnosticsState);
      obj->diagnosticsState = cmsMem_strdup(pParams->value);
      ret = cmsObj_set(obj, &iidStack);
      cmsObj_free((void **) &obj);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   if(ret!=CMSRET_SUCCESS)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   cmsMgm_saveConfigToFlash();
   return TRUE;
}

int SetNumberOfRepetitionsATMF5(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   WanAtm5LoopbackDiagObject *obj;
   CmsRet ret = CMSRET_INVALID_ARGUMENTS;


   struct Param *pParams;

   int instanceOfWANDevice;
   int instanceOfWANConnectionDevice;
   instanceOfWANDevice = psvc->device->parent->instance;
   instanceOfWANConnectionDevice = psvc->device->instance;

   PUSH_INSTANCE_ID(&iidStack, instanceOfWANDevice);
   PUSH_INSTANCE_ID(&iidStack, instanceOfWANConnectionDevice);
   
   pParams = findActionParamByRelatedVar(ac,VAR_NumberOfRepetitions);
   if (pParams == NULL)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   if ((ret = cmsObj_get(MDMOID_WAN_ATM5_LOOPBACK_DIAG, &iidStack, 0, (void **) &obj)) == CMSRET_SUCCESS)
   {   
      obj->numberOfRepetitions = atoi(pParams->value);
      ret = cmsObj_set(obj, &iidStack);
      cmsObj_free((void **) &obj);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   if(ret!=CMSRET_SUCCESS)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   cmsMgm_saveConfigToFlash();
   return TRUE;
}

int SetTimeoutATMF5(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   WanAtm5LoopbackDiagObject *obj;
   CmsRet ret = CMSRET_INVALID_ARGUMENTS;

   struct Param *pParams;

   int instanceOfWANDevice;
   int instanceOfWANConnectionDevice;
   instanceOfWANDevice = psvc->device->parent->instance;
   instanceOfWANConnectionDevice = psvc->device->instance;

   PUSH_INSTANCE_ID(&iidStack, instanceOfWANDevice);
   PUSH_INSTANCE_ID(&iidStack, instanceOfWANConnectionDevice);
   
   pParams = findActionParamByRelatedVar(ac,VAR_Timeout);
   if (pParams == NULL)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   if ((ret = cmsObj_get(MDMOID_WAN_ATM5_LOOPBACK_DIAG, &iidStack, 0, (void **) &obj)) == CMSRET_SUCCESS)
   {   
      obj->timeout= atoi(pParams->value);
      ret = cmsObj_set(obj, &iidStack);
      cmsObj_free((void **) &obj);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   if(ret!=CMSRET_SUCCESS)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
 
   cmsMgm_saveConfigToFlash();
   return TRUE;
}

