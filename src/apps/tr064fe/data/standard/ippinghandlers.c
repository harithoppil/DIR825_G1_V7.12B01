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
//  Filename:       ippinghandlers.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "ippingparams.h"

#include "tr64defs.h"
#include "session.h"

#include "cms_core.h"
#include "cms_dal.h"
#include "cms_util.h"

int IPPing_GetVar(struct Service *psvc, int varindex)
{
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   IPPingDiagObject *pingDiag;
   CmsRet ret;

   struct StateVar *var;
   char DiagnosticsState[32];
   char Interface[256];
   char Host[256];
   unsigned int NumberOfRepetitions;
   unsigned int Timeout;
   unsigned int DataBlockSize;
   unsigned int DSCP;
   unsigned int SuccessCount;
   unsigned int FailureCount;
   unsigned int AverageResponseTime;
   unsigned int MinimumResponseTime;
   unsigned int MaximumResponseTime;

   if ((ret = cmsObj_get(MDMOID_IP_PING_DIAG, &iidStack, 0, (void **) &pingDiag)) == CMSRET_SUCCESS)
   {
      if(pingDiag->diagnosticsState)
      {
         strcpy(DiagnosticsState, pingDiag->diagnosticsState);
      }

      if(pingDiag->interface)
      {
         strcpy(Interface, pingDiag->interface);
      }

      if(pingDiag->host)
      {
         strcpy(Host, pingDiag->host);
      }

      NumberOfRepetitions = pingDiag->numberOfRepetitions;
      Timeout = pingDiag->timeout;
      DataBlockSize = pingDiag->dataBlockSize;
      DSCP = pingDiag->DSCP;
      SuccessCount = pingDiag->successCount;
      FailureCount = pingDiag->failureCount;
      AverageResponseTime = pingDiag->averageResponseTime;
      MinimumResponseTime = pingDiag->minimumResponseTime;
      MaximumResponseTime = pingDiag->maximumResponseTime;

      cmsObj_free((void **) &pingDiag);
   }
   else
   {
      //cmsLog_error("cmsObj_get MDMOID_IP_PING_DIAG failed ret=%d", ret);
      return FALSE;
   } 

   var = &(psvc->vars[varindex]);
   switch (varindex)
   {
      case VAR_DiagnosticsState:
         strcpy(var->value, DiagnosticsState);        
      break;

      case VAR_Interface:
         strcpy(var->value, Interface);        
      break;

      case VAR_Host:
         strcpy(var->value, Host);        
      break;

      case VAR_NumberOfRepetitions:
         sprintf(var->value, "%d", NumberOfRepetitions);
      break;

      case VAR_Timeout:
         sprintf(var->value, "%d", Timeout);
      break;

      case VAR_DataBlockSize:
         sprintf(var->value, "%d", DataBlockSize);
      break;

      case VAR_DSCP:
         sprintf(var->value, "%d", DSCP);
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

int SetDiagnosticsStateIPPing(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   
   pParams = findActionParamByRelatedVar(ac,VAR_DiagnosticsState);
   if (pParams != NULL)
   {
      InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
      IPPingDiagObject *pingDiag;
      CmsRet ret;

      if (strcmp(pParams->value,MDMVS_REQUESTED) != 0)
      {
         soap_error( uclient, SOAP_ACTIONFAILED );
         //cmsLog_error(" input parameter value failed");
         return FALSE;
      }

      if ((ret = cmsObj_get(MDMOID_IP_PING_DIAG, &iidStack, 0, (void **) &pingDiag)) == CMSRET_SUCCESS)
      {     
         free(pingDiag->diagnosticsState);
         pingDiag->diagnosticsState = cmsMem_strdup(MDMVS_REQUESTED);

         ret = cmsObj_set(pingDiag, &iidStack);

         cmsObj_free((void **) &pingDiag);

         if(ret != CMSRET_SUCCESS)
         {
            //cmsLog_error("cmsObj_set MDMOID_IP_PING_DIAG failed ret=%d", ret);
            soap_error( uclient, SOAP_ACTIONFAILED );
            return FALSE;
         }
      }
      else
      {
         ////cmsLog_error("cmsObj_get MDMOID_IP_PING_DIAG failed ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }
   }
   else
   {
      ////cmsLog_error("input pParam->value is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   cmsMgm_saveConfigToFlash();   
   return TRUE;
}

int SetInterfaceIPPing(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   
   pParams = findActionParamByRelatedVar(ac,VAR_Interface);
   if (pParams != NULL)
   {
      InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
      IPPingDiagObject *pingDiag;
      CmsRet ret;

      if ((ret = cmsObj_get(MDMOID_IP_PING_DIAG, &iidStack, 0, (void **) &pingDiag)) == CMSRET_SUCCESS)
      {     
         cmsMem_free(pingDiag->interface);
         pingDiag->interface= cmsMem_strdup(pParams->value);

         ret = cmsObj_set(pingDiag, &iidStack);

         cmsObj_free((void **) &pingDiag);

         if(ret != CMSRET_SUCCESS)
         {
            //cmsLog_error("cmsObj_set MDMOID_IP_PING_DIAG failed ret=%d", ret);
            soap_error( uclient, SOAP_ACTIONFAILED );
            return FALSE;
         }
      }
      else
      {
         //cmsLog_error("cmsObj_get MDMOID_IP_PING_DIAG failed ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }
   }
   else
   {
      //cmsLog_error("input pParam->value is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   cmsMgm_saveConfigToFlash();
   return TRUE;

}

int SetHostIPPing(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   
   pParams = findActionParamByRelatedVar(ac,VAR_Host);
   if (pParams != NULL)
   {
      InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
      IPPingDiagObject *pingDiag;
      CmsRet ret;

      if ((ret = cmsObj_get(MDMOID_IP_PING_DIAG, &iidStack, 0, (void **) &pingDiag)) == CMSRET_SUCCESS)
      {     
         cmsMem_free(pingDiag->host);
         pingDiag->host= cmsMem_strdup(pParams->value);

         ret = cmsObj_set(pingDiag, &iidStack);

         cmsObj_free((void **) &pingDiag);

         if(ret != CMSRET_SUCCESS)
         {
            //cmsLog_error("cmsObj_set MDMOID_IP_PING_DIAG failed ret=%d", ret);
            soap_error( uclient, SOAP_ACTIONFAILED );
            return FALSE;
         }
      }
      else
      {
         //cmsLog_error("cmsObj_get MDMOID_IP_PING_DIAG failed ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }
   }
   else
   {
      //cmsLog_error("input pParam is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   cmsMgm_saveConfigToFlash();
   return TRUE;
}

int NumberOfRepetitionsIPPing(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   
   pParams = findActionParamByRelatedVar(ac,VAR_NumberOfRepetitions);
   if (pParams != NULL)
   {
      InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
      IPPingDiagObject *pingDiag;
      CmsRet ret;

      if ((ret = cmsObj_get(MDMOID_IP_PING_DIAG, &iidStack, 0, (void **) &pingDiag)) == CMSRET_SUCCESS)
      {     
         pingDiag->numberOfRepetitions = atoi(pParams->value);

         ret = cmsObj_set(pingDiag, &iidStack);
         cmsObj_free((void **) &pingDiag);
         if(ret != CMSRET_SUCCESS)
         {
            //cmsLog_error("cmsObj_set MDMOID_IP_PING_DIAG failed ret=%d", ret);
            soap_error( uclient, SOAP_ACTIONFAILED );
            return FALSE;
         }
      }
      else
      {
         //cmsLog_error("cmsObj_get MDMOID_IP_PING_DIAG failed ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }
   }
   else
   {
      //cmsLog_error("input pParam is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   cmsMgm_saveConfigToFlash();
   return TRUE;
}

int SetTimeoutIPPing(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   
   pParams = findActionParamByRelatedVar(ac,VAR_Timeout);
   if (pParams != NULL)
   {
      InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
      IPPingDiagObject *pingDiag;
      CmsRet ret;

      if ((ret = cmsObj_get(MDMOID_IP_PING_DIAG, &iidStack, 0, (void **) &pingDiag)) == CMSRET_SUCCESS)
      {     
         pingDiag->timeout = atoi(pParams->value);

         ret = cmsObj_set(pingDiag, &iidStack);
         cmsObj_free((void **) &pingDiag);
         if(ret != CMSRET_SUCCESS)
         {
            //cmsLog_error("cmsObj_set MDMOID_IP_PING_DIAG failed ret=%d", ret);
            soap_error( uclient, SOAP_ACTIONFAILED );
            return FALSE;
         }
      }
      else
      {
         //cmsLog_error("cmsObj_get MDMOID_IP_PING_DIAG failed ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }
   }
   else
   {
      //cmsLog_error("input pParams is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   cmsMgm_saveConfigToFlash();
   return TRUE;
}

int SetDataBlockSizeIPPing(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   
   pParams = findActionParamByRelatedVar(ac,VAR_DataBlockSize);
   if (pParams != NULL)
   {
      InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
      IPPingDiagObject *pingDiag;
      CmsRet ret;

      if ((ret = cmsObj_get(MDMOID_IP_PING_DIAG, &iidStack, 0, (void **) &pingDiag)) == CMSRET_SUCCESS)
      {     
         pingDiag->dataBlockSize = atoi(pParams->value);

         ret = cmsObj_set(pingDiag, &iidStack);
         cmsObj_free((void **) &pingDiag);
         if(ret != CMSRET_SUCCESS)
         {
            //cmsLog_error("cmsObj_set MDMOID_IP_PING_DIAG failed ret=%d", ret);
            soap_error( uclient, SOAP_ACTIONFAILED );
            return FALSE;
         }
      }
      else
      {
         //cmsLog_error("cmsObj_get MDMOID_IP_PING_DIAG failed ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }
   }
   else
   {
      //cmsLog_error("input pParams is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   cmsMgm_saveConfigToFlash();
   return TRUE;
}

int SetDSCPIPPing(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   
   pParams = findActionParamByRelatedVar(ac,VAR_DSCP);
   if (pParams != NULL)
   {
      InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
      IPPingDiagObject *pingDiag;
      CmsRet ret;

      if ((ret = cmsObj_get(MDMOID_IP_PING_DIAG, &iidStack, 0, (void **) &pingDiag)) == CMSRET_SUCCESS)
      {     
         pingDiag->DSCP = atoi(pParams->value);

         ret = cmsObj_set(pingDiag, &iidStack);
         cmsObj_free((void **) &pingDiag);
         if(ret != CMSRET_SUCCESS)
         {
            //cmsLog_error("cmsObj_set MDMOID_IP_PING_DIAG failed ret=%d", ret);
            soap_error( uclient, SOAP_ACTIONFAILED );
            return FALSE;
         }
      }
      else
      {
         //cmsLog_error("cmsObj_get MDMOID_IP_PING_DIAG failed ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }
   }
   else
   {
      //cmsLog_error("input pParams is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   cmsMgm_saveConfigToFlash();
   return TRUE;
}
