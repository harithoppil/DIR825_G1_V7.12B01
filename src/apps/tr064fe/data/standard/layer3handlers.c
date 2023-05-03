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
//  Filename:       layer3handlers.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "layer3params.h"
#include "tr64defs.h"
#include "cms.h"
#include "cmmif.h"

static PDevice find_dev_by_udn(const char *udn);
static PService find_svc_by_type(PDevice pdev, char *type);
static PService find_svc_by_name(PDevice pdev, const char *name);

int Layer3Forwarding_Init(PService psvc, service_state_t state)
{
    struct Layer3Forwarding *pdata;
    switch (state)
    {
    case SERVICE_CREATE:
       pdata = (struct Layer3Forwarding *) malloc(sizeof(struct Layer3Forwarding));
       if (pdata) 
       {
          memset(pdata, 0, sizeof(struct Layer3Forwarding));
          pdata->default_svc = NULL;
          psvc->opaque = (void *) pdata;
       }
       break;
    case SERVICE_DESTROY:
       pdata = (struct Layer3Forwarding *) psvc->opaque;
       free(pdata);
       break;
    } 

    return 0;
}

int Layer3_GetVar(struct Service *psvc, int varindex)
{
   struct StateVar *var;
   struct Layer3Forwarding *data = psvc->opaque;

   char szAdslValue[CMM_MAX_NODE_LEN] ;

   var = &(psvc->vars[varindex]);

   switch (varindex)
   {
      case VAR_DefaultConnectionService:
	  	 if(CMM_GetStr(TR064_ROOT_Layer3Forwarding"DefaultConnectionService", szAdslValue, sizeof(szAdslValue), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current ReservedAddresses ");
		      return FALSE;
	     }
         strcpy(var->value, szAdslValue);
      break;
   } /* end-switch */

   return TRUE;
}

int SetDefaultConnectionService(UFILE *uclient, PService psvc, 
                                PAction ac, pvar_entry_t args, int nargs)
{
   int success = TRUE;
   struct Layer3Forwarding *data = psvc->opaque;
   char *type, *udn;
   struct Service *csvc;
   struct Device *pdev;
   
   udn = ac->params[0].value;
   type = strsep(&udn, ",");
    
   if (udn == NULL || (pdev = find_dev_by_udn(udn)) == NULL)
   {
      soap_error(uclient, SOAP_INVALIDDEVICEUUID);
      success = FALSE;
   }
   else if (type == NULL || (csvc = find_svc_by_type(pdev, type)) == NULL) 
   {
      soap_error(uclient, SOAP_INVALIDSERVICEID);
      success = FALSE;
   } 
   else
   {
      data->default_svc = csvc;
      mark_changed(psvc, VAR_DefaultConnectionService);
   }

   return success;
}

static PDevice find_dev_by_udn(const char *udn)
{
   PDevice pdev = NULL;
    
   forall_devices(pdev) 
   {
      if (strcmp(pdev->udn, udn) == 0) 
      {
         break;
      }
   }
    
   return pdev;
}

static PService find_svc_by_type(PDevice pdev, char *type)
{
   char *name = NULL, *p;
   PService psvc = NULL;
   
   p = rindex(type, ':');
   if (p != 0) 
   {
      *p = '\0';
      p = rindex(type, ':');
      if (p != 0)
         name = p+1;
   }

   if (name) 
      psvc = find_svc_by_name(pdev, name);

   return psvc;
}

static PService find_svc_by_name(PDevice pdev, const char *name)
{
   PService psvc;

   forall_services(pdev, psvc) 
   {
      if (strcmp(psvc->template->name, name) == 0) 
      {
         break;
      }
   }

   return psvc;
}

int AddForwardingEntry( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   unsigned long pulInstanceId ;
   if(CMM_AddInstance(TR064_ROOT_RouteInfo,&pulInstanceId )!= CMM_SUCCESS)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   char szDestIPAddr[60]={0};
   char szDestMask[60]={0};
   char szGatewayIPAddr[60]={0};
   char szInterface[60]={0};
   
   char* szLeafNames[4]={szDestIPAddr,szDestMask,szGatewayIPAddr,szInterface};
   sprintf(szLeafNames[0],TR064_ROOT_RouteInfo"%d.DestIPAddress",pulInstanceId);
   sprintf(szLeafNames[1],TR064_ROOT_RouteInfo"%d.DestSubnetMask",pulInstanceId);
   sprintf(szLeafNames[2],TR064_ROOT_RouteInfo"%d.GatewayIPAddress",pulInstanceId);
   sprintf(szLeafNames[3],TR064_ROOT_RouteInfo"%d.Interface",pulInstanceId);
   int relatedVars[4] = {VAR_DestIPAddress,VAR_DestSubnetMask,VAR_GatewayIPAddress,VAR_Interface};
   SetTr64Values(szLeafNames,relatedVars ,4);
}

/*
   Index Support 
    0     Y        {"NewDestIPAddress", VAR_DestIPAddress, VAR_IN},
    1     Y        {"NewDestSubnetMask", VAR_DestSubnetMask, VAR_IN},
    2     N        {"NewSourceIPAddress", VAR_SourceIPAddress, VAR_IN},
    3     N        {"NewSourceSubnetMask", VAR_SourceSubnetMask, VAR_IN},
 */     
int DeleteForwardingEntry( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char addr[BUFLEN_40], mask[BUFLEN_40];
   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac,VAR_DestIPAddress);
   if (pParams != NULL)
   {
      strcpy(addr, pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   pParams = findActionParamByRelatedVar(ac,VAR_DestSubnetMask);
   if (pParams != NULL)
   {
      strcpy(mask, pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;      
   }
  
   char **ppRouteInfoList = CMM_GetInstanceList(TR064_ROOT_RouteInfo);
   if(ppRouteInfoList != NULL)
   {  
       ST_TR064STR_LINK* pStRouteInfoLink = NULL;
	   int i = 0 ;
	   for( ; ppRouteInfoList[i] ;i++)
	   {
	       char* aszLeafName[2]= {"DestIPAddress","DestSubnetMask"};
		   pStRouteInfoLink = CMM_GetStrs(ppRouteInfoList[i],aszLeafName,2);
		   if(pStRouteInfoLink == NULL)
		   {
		       soap_error( uclient, SOAP_ACTIONFAILED );
			   CMM_FreeInstanceList(ppRouteInfoList);
		       return FALSE;
		   }

		   if((strcmp(addr, pStRouteInfoLink->pstrValue) == 0) && (strcmp(mask,pStRouteInfoLink->pNext->pstrValue)== 0))
		   {
		       TR064_DestroyStrLink(pStRouteInfoLink);
		       if( CMM_DelInstance(ppRouteInfoList[i]) == CMM_SUCCESS)
		       { 
				  CMM_FreeInstanceList(ppRouteInfoList);
	              return TRUE;   
		       }
               TR64FE_TRACE("DeleteForwardingEntry failed");
			   break;
			   
		   }else
		   {
		   	TR064_DestroyStrLink(pStRouteInfoLink);
		   }
	   }
   
   	   CMM_FreeInstanceList(ppRouteInfoList);
   }

   soap_error( uclient, SOAP_ACTIONFAILED );
   return FALSE;
}

int GetGenericForwardingEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   printf( "### GetGenericForwardingEntry####");
   int index = 0;
   struct Param *pParams = findActionParamByRelatedVar(ac,VAR_ForwardNumberOfEntries);
   if (pParams != NULL)
   {
      index = atoi(pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   printf( "index  is = %d" ,index);

   char* aszLeafName[4]= {0};
   sprintf(aszLeafName[0],TR064_ROOT_RouteInfo"%d.DestIPAddress" ,index);
   sprintf(aszLeafName[1],TR064_ROOT_RouteInfo"%d.DestSubnetMask" ,index);
   sprintf(aszLeafName[2],TR064_ROOT_RouteInfo"%d.GatewayIPAddress" ,index);
   sprintf(aszLeafName[3],TR064_ROOT_RouteInfo"%d.Interface" ,index);

   ST_TR064STR_LINK* pStRouteInfoLink = NULL;
   pStRouteInfoLink = CMM_FullPathGetStrs(aszLeafName,4);

   if(pStRouteInfoLink == NULL)
   {
       soap_error( uclient, SOAP_SPECIFIEDARRAYINDEXINVALID );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpstIPIfLink = pStRouteInfoLink;
   
   int errorinfo = 0;   
   static char Enable[32];
   static char Status[32];
   static char DestIPAddress[32];
   static char DestSubnetMask[32];
   static char GatewayIPAddress[32];
   static char Interface[32];
   
   strcpy(Enable ,"1");
   
   strcpy(Status ,"Enabled");
   
   strcpy(DestIPAddress ,pTmpstIPIfLink->pstrValue);
   pTmpstIPIfLink = pTmpstIPIfLink->pNext;
   
   strcpy(DestSubnetMask ,pTmpstIPIfLink->pstrValue);
   pTmpstIPIfLink = pTmpstIPIfLink->pNext;
   
   strcpy(GatewayIPAddress ,pTmpstIPIfLink->pstrValue);
   pTmpstIPIfLink = pTmpstIPIfLink->pNext;
   
   strcpy(Interface,pTmpstIPIfLink->pstrValue);
   
   TR064_DestroyStrLink(pStRouteInfoLink);

   errorinfo |= OutputCharValueToAC(ac, VAR_Enable, Enable);
   errorinfo |= OutputCharValueToAC(ac, VAR_Status, Status);
   errorinfo |= OutputCharValueToAC(ac, VAR_DestIPAddress, DestIPAddress);
   errorinfo |= OutputCharValueToAC(ac, VAR_DestSubnetMask, DestSubnetMask);
   errorinfo |= OutputCharValueToAC(ac, VAR_GatewayIPAddress, GatewayIPAddress);
   errorinfo |= OutputCharValueToAC(ac, VAR_Interface, Interface);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   }
   
   return TRUE;
}

int GetSpecificForwardingEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   static char DestIPAddress[32];
   static char DestSubnetMask[32];
   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac,VAR_DestIPAddress);
   if (pParams != NULL)
   {
      strcpy(DestIPAddress, pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   pParams = findActionParamByRelatedVar(ac,VAR_DestSubnetMask);
   if (pParams != NULL)
   {
      strcpy(DestSubnetMask, pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;      
   }
  
   char **ppRouteInfoList = CMM_GetInstanceList(TR064_ROOT_RouteInfo);
   if( ppRouteInfoList != NULL)
   {
       ST_TR064STR_LINK* pStRouteInfoLink = NULL;
	   int i = 0 ;
	   for( ; ppRouteInfoList[i] ;i++)
	   {
	       char* aszLeafName[4]= {"DestIPAddress","DestSubnetMask","GatewayIPAddress","Interface"};
		   pStRouteInfoLink = CMM_GetStrs(ppRouteInfoList[i],aszLeafName,4);
		   if(pStRouteInfoLink == NULL)
		   {
		       soap_error( uclient, SOAP_ACTIONFAILED );
			   CMM_FreeInstanceList(ppRouteInfoList);
		       return FALSE;
		   }

		   if((strcmp(DestIPAddress , pStRouteInfoLink->pstrValue) == 0) 
		   	            && (strcmp(DestSubnetMask,pStRouteInfoLink->pNext->pstrValue)== 0))
		   {
			   static char Enable[32];
			   static char Status[32];
			   static char GatewayIPAddress[32];
			   static char Interface[32];
			   
			   strcpy(Enable ,"1");
			   strcpy(Status ,"Enabled");

			   ST_TR064STR_LINK *pTmpLink = pStRouteInfoLink;

			   strcpy(GatewayIPAddress ,pTmpLink->pstrValue);
			   pTmpLink = pTmpLink->pNext;
			   strcpy(Interface ,pTmpLink->pstrValue);

			   TR064_DestroyStrLink(pStRouteInfoLink);

			   int errorinfo = 0;
			   errorinfo |= OutputCharValueToAC(ac, VAR_Enable, Enable);
			   errorinfo |= OutputCharValueToAC(ac, VAR_Status, Status);
			   errorinfo |= OutputCharValueToAC(ac, VAR_DestIPAddress, DestIPAddress);
			   errorinfo |= OutputCharValueToAC(ac, VAR_DestSubnetMask, DestSubnetMask);
			   errorinfo |= OutputCharValueToAC(ac, VAR_GatewayIPAddress, GatewayIPAddress);
			   errorinfo |= OutputCharValueToAC(ac, VAR_Interface, Interface);

			   if(!errorinfo)
			   {
			      CMM_FreeInstanceList(ppRouteInfoList);
		          return TRUE;
			   }

	           TR64FE_TRACE("failed to GetSpecificForwardingEntry ");
			   break ;
		   }else
		   {
		      TR064_DestroyStrLink(pStRouteInfoLink);
		   }
	   }

	   CMM_FreeInstanceList(ppRouteInfoList);
   }
   
   soap_error( uclient, SOAP_NOSUCHENTRYINARRAY );
   return FALSE;
}


/*
Index Support
0      Y      {"NewDestIPAddress", VAR_DestIPAddress, VAR_IN},
1      Y       {"NewDestSubnetMask", VAR_DestSubnetMask, VAR_IN},
2      N       {"NewSourceIPAddress", VAR_SourceIPAddress, VAR_IN},
3      N       {"NewSourceSubnetMask", VAR_SourceSubnetMask, VAR_IN},  
4      Y      {"NewEnable", VAR_Enable, VAR_IN},
*/
int SetForwardingEntryEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char DestIPAddress[32];
   char DestSubnetMask[32];
   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac,VAR_DestIPAddress);
   if (pParams != NULL)
   {
      strcpy(DestIPAddress, pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   pParams = findActionParamByRelatedVar(ac,VAR_DestSubnetMask);
   if (pParams != NULL)
   {
      strcpy(DestSubnetMask, pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;      
   }
  
   char **ppRouteInfoList = CMM_GetInstanceList(TR064_ROOT_RouteInfo);
   if( ppRouteInfoList != NULL)
   {
       ST_TR064STR_LINK* pStRouteInfoLink = NULL;
	   int i = 0 ;
	   for( ; ppRouteInfoList[i] ;i++)
	   {
	       char* aszLeafName[2]= {"DestIPAddress","DestSubnetMask"};
		   pStRouteInfoLink = CMM_GetStrs(ppRouteInfoList[i],aszLeafName,2);
		   if(pStRouteInfoLink == NULL)
		   {
		       soap_error( uclient, SOAP_ACTIONFAILED );
			   CMM_FreeInstanceList(ppRouteInfoList);
		       return FALSE;
		   }

		   if((strcmp(DestIPAddress , pStRouteInfoLink->pstrValue) == 0) 
		   	            && (strcmp(DestSubnetMask,pStRouteInfoLink->pNext->pstrValue)== 0))
		   {
		       TR064_DestroyStrLink(pStRouteInfoLink);
			   CMM_FreeInstanceList(ppRouteInfoList);
		       return TRUE;          
		   }else
		   {
		      TR064_DestroyStrLink(pStRouteInfoLink);
		   }
	   }

	   CMM_FreeInstanceList(ppRouteInfoList);
   }
   
   soap_error( uclient, SOAP_NOSUCHENTRYINARRAY );
   return FALSE;
}

/*
 * {"NewForwardNumberOfEntries", VAR_ForwardNumberOfEntries, VAR_OUT}, 
 */ 
int GetForwardNumberOfEntries(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)      
{
   static char ForwardNumberOfEntries[32];
   if(CMM_GetStr(TR064_ROOT_Layer3Forwarding"DefaultConnectionService", ForwardNumberOfEntries, sizeof(ForwardNumberOfEntries), NULL, 0) != CMM_SUCCESS)
   {
          TR64FE_TRACE("Could not get current ReservedAddresses ");
	      return FALSE;
   }
   struct Param *pParams = findActionParamByRelatedVar(ac,VAR_ForwardNumberOfEntries);
   if (pParams != NULL)
   {
      if(0 == strcmp("",ForwardNumberOfEntries))
      {
         strcpy(ForwardNumberOfEntries,"0");
      }
      pParams->value = ForwardNumberOfEntries; 
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   return TRUE;
}
