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
//  Filename:       lancfgsechandlers.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "lancfgsecparams.h"
#include "tr64defs.h"
#include "cmmif.h"

extern tr64PersistentData *pTr64Data;
extern void setCurrentState(pTr64PersistentData pData);

int SetConfigPassword(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   
   pParams = findActionParamByRelatedVar(ac,VAR_ConfigPassword);
   if (pParams != NULL)
   {
      if(pParams->value!=NULL)
         strcpy(pTr64Data->password, pParams->value);
      else
         strcpy(pTr64Data->password, TR64_DSLF_RESET_PWD); 
      pTr64Data->passwordState = NORMAL;
      setCurrentState(pTr64Data);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   
   return TRUE;
}
