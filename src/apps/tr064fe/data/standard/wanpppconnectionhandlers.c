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
//  Filename:       wanpppconnectionhandlers.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "wanpppconnection.h"
#include "cmmif.h"


#define GetPPPConnValue(szLeafName) \
do{\
    snprintf(szFullPathName, sizeof(szFullPathName), TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.%s" ,psvc->instance,szLeafName);\
  	if(CMM_GetStr(szFullPathName, var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)\
	{\
	      TR64FE_TRACE("Could not get current WANPPPConnection %s ",szLeafName);\
	      return FALSE;\
	}\
	if( 0 ==strcmp(var->value ,""))\
	{\
	  strcpy(var->value ,"0");\
	}\
	return TRUE; \
}while(0)

int WANPPPConnection_GetVar(struct Service *psvc, int varindex)
{
   char szFullPathName[CMM_MAX_NODE_LEN];
   struct StateVar *var;
   var = &(psvc->vars[varindex]);  

   switch (varindex)
   {
      case VAR_Enable:
	  	 GetPPPConnValue("Enable");
         break;
      case VAR_ConnectionType:
	  	 GetPPPConnValue("ConnectionType");
         break;
      case VAR_ConnectionStatus:
	  	 GetPPPConnValue("ConnectionStatus");
         break;
         
      case VAR_Name:
	  	 GetPPPConnValue("Name");
         break;
        
      case VAR_IdleDisconnectTime:
	  	 GetPPPConnValue("IdleDisconnectTime");
         break;
       
      case VAR_NATEnabled:
	  	 GetPPPConnValue("NATEnabled");
         break;
         
      case VAR_ExternalIPAddress:
	  	 GetPPPConnValue("ExternalIPAddress");
         break;
         
      case VAR_RemoteIPAddress:
	  	 GetPPPConnValue("RemoteIPAddress");
         break;
             
      case VAR_Username:
	  	 GetPPPConnValue("Username");
         break;
              
      case VAR_DNSServers:
	  	 GetPPPConnValue("DNSServers");
         break;
          
      case VAR_PPPoEServiceName:
	  	 GetPPPConnValue("PPPoEServiceName");
         break;
                  
      case VAR_BytesSent:
	     GetPPPConnValue("Stats.EthernetBytesSent");
         break;
         
      case VAR_BytesReceived:
	  	 GetPPPConnValue("Stats.EthernetBytesReceived");
         break;
         
      case VAR_PacketsSent:
	  	 GetPPPConnValue("Stats.EthernetPacketsSent");
         break;
         
      case VAR_PacketsReceived:
	  	 GetPPPConnValue("Stats.EthernetPacketsReceived");
         break;
   }
   return TRUE;
}

int ForceTermination(UFILE *uclient, PService psvc, PAction ac,
                     pvar_entry_t args, int nargs)
{
#ifdef LGD_TODO
#endif
   return TRUE;
}

int RequestTermination(UFILE *uclient, PService psvc, PAction ac,
                     pvar_entry_t args, int nargs)
{
#ifdef LGD_TODO
#endif  
    
   return TRUE;
}

int RequestConnection(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
#ifdef LGD_TODO
#endif
   return TRUE;
}

int SetConnectionType(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
    char szNodeName[CMM_MAX_NODE_LEN]={0};
    snprintf(szNodeName ,sizeof(szNodeName),TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.ConnectionType" ,
	 	                                              psvc->instance);
    SetTr64Value(szNodeName,VAR_ConnectionType);   
}

int SetWANPPPConnEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szNodeName ,sizeof(szNodeName), TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.Enable" ,
	 	                                              psvc->instance);
   SetTr64Value(szNodeName,VAR_Enable);
}

int SetUsername(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szNodeName , sizeof(szNodeName),TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.Username" ,
	 	                                              psvc->instance);
   SetTr64Value(szNodeName,VAR_Username);
}

int SetPassword(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szNodeName ,sizeof(szNodeName), TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.Password" ,
	 	                                              psvc->instance);
   SetTr64Value(szNodeName,VAR_Password); 
}

int SetPPPoEService(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szNodeName ,sizeof(szNodeName), TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.PPPoEServiceName" ,
	 	                                               psvc->instance);
   SetTr64Value(szNodeName,VAR_PPPoEServiceName); 
}

int SetConnectionTrigger(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   //not support
   char szNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szNodeName ,sizeof(szNodeName), TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.ConnectionTrigger" ,
	 	                                             psvc->instance);
   SetTr64Value(szNodeName,VAR_ConnectionTrigger);
}

int SetIdleDisconnectTime(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szNodeName ,sizeof(szNodeName), TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.IdleDisconnectTime" ,
	 	                                              psvc->instance);
   SetTr64Value(szNodeName,VAR_IdleDisconnectTime); 
}

int ppp_GetPortMappingNumberOfEntries(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   int  errorinfo = 0;
   char szNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szNodeName ,sizeof(szNodeName), TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.PortMappingNumberOfEntries",
	 	                                    psvc->instance);
   static char NumOfPortMapping[32];
   if(CMM_GetStr(szNodeName , NumOfPortMapping ,sizeof(NumOfPortMapping),NULL,0 ) != CMM_SUCCESS)
   {
	     TR64FE_TRACE("Could not get current PortMappingNumberOfEntries");
	     return FALSE;
   }

   errorinfo = OutputCharValueToAC(ac, VAR_PortMappingNumberOfEntries, NumOfPortMapping);
   if(errorinfo)
   {
      soap_error( uclient, errorinfo);
      return FALSE;
   }

   return TRUE;
}

int ppp_GetGenericPortMappingEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   int  PortMappingIndex =0;
   int  errorinfo = 0;
  
   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac,VAR_PortMappingNumberOfEntries);
   if (pParams != NULL)
   {
      PortMappingIndex = atoi(pParams->value); 
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   } 

   char szWanPPPConnNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szWanPPPConnNodeName , sizeof(szWanPPPConnNodeName),TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.PortMapping.%d" ,
	 	                                             psvc->instance ,PortMappingIndex);
   char* aszLeafName[6]= {"PortMappingEnabled" ,"ExternalPort","InternalPort","PortMappingProtocol",
   	                                       "InternalClient" ,"PortMappingDescription"};
   
   ST_TR064STR_LINK *pStWanPPPConnLst;
   pStWanPPPConnLst = CMM_GetStrs(szWanPPPConnNodeName,aszLeafName,6);
   if(pStWanPPPConnLst == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpLst;
   pTmpLst = pStWanPPPConnLst;
   
   static char PortMappingEnabled[4];
   static char ExternalPort[32];
   static char InternalPort[32];
   static char PortMappingProtocol[32];
   static char InternalClient[32];
   static char PortMappingDescription[256];
   
   strcpy(PortMappingEnabled, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;
   
   strcpy(ExternalPort, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;
   
   strcpy(InternalPort, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;
   
   strcpy(PortMappingProtocol, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;
   
   strcpy(InternalClient, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;
   
   strcpy(PortMappingDescription, pTmpLst->pstrValue);
   
   TR064_DestroyStrLink(pStWanPPPConnLst);

   errorinfo |= OutputCharValueToAC(ac, VAR_ExternalPort, ExternalPort);
   errorinfo |= OutputCharValueToAC(ac, VAR_PortMappingEnabled, PortMappingEnabled);
   errorinfo |= OutputCharValueToAC(ac, VAR_InternalPort, InternalPort);
   errorinfo |= OutputCharValueToAC(ac, VAR_InternalClient, InternalClient);
   errorinfo |= OutputCharValueToAC(ac, VAR_PortMappingProtocol, PortMappingProtocol);
   errorinfo |= OutputCharValueToAC(ac, VAR_PortMappingDescription, PortMappingDescription);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo);
      return FALSE;
   }

   return TRUE;
}

int ppp_GetSpecificPortMappingEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   int  errorinfo = 0;
   char ExternalPort[32];
   char PortMappingProtocol[32];
   int  found = 0;
 
   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac,VAR_ExternalPort);
   if (pParams != NULL)
   {
	  strcpy(ExternalPort, pParams->value);  
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   pParams = findActionParamByRelatedVar(ac,VAR_PortMappingProtocol);
   if (pParams != NULL)
   {
      strcpy(PortMappingProtocol, pParams->value);   
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;      
   } 

   char* aszLeafName[6]= {"ExternalPort","PortMappingProtocol","PortMappingEnabled","InternalPort", 
   	                                       "InternalClient" ,"PortMappingDescription"};

   char szWanPPPConnNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szWanPPPConnNodeName ,sizeof(szWanPPPConnNodeName),TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.PortMapping." ,
	 	                                              psvc->instance);
   char **ppWanPPPConnPortsLst = NULL;
   ppWanPPPConnPortsLst= CMM_GetInstanceList(szWanPPPConnNodeName);
   if(ppWanPPPConnPortsLst != NULL) 
   {  
      int i= 0 ;
      for(; ppWanPPPConnPortsLst[i]; i++)
      {
		   ST_TR064STR_LINK *pStWanPPPConnLst;
		   pStWanPPPConnLst = CMM_GetStrs(ppWanPPPConnPortsLst[i],aszLeafName,6);
		   if(pStWanPPPConnLst == NULL)
		   {
		       soap_error( uclient, SOAP_ACTIONFAILED );
			   CMM_FreeInstanceList(ppWanPPPConnPortsLst);
		       return FALSE;
		   }

           if(( 0 == strcmp(ExternalPort, pStWanPPPConnLst->pstrValue))
                &&(0==strcmp(PortMappingProtocol, pStWanPPPConnLst->pNext->pstrValue)))
           {
                ST_TR064STR_LINK *pTmpLst ;
                pTmpLst = pStWanPPPConnLst;
				
                pTmpLst = pTmpLst->pNext;
				pTmpLst = pTmpLst->pNext;
		   
                static char PortMappingEnabled[4];
			    static char InternalPort[32];
			    static char InternalClient[32];
			    static char PortMappingDescription[256];
			   
			    strcpy(PortMappingEnabled, pTmpLst->pstrValue);
			    pTmpLst = pTmpLst->pNext;
			   
			    strcpy(InternalPort, pTmpLst->pstrValue);
			    pTmpLst = pTmpLst->pNext;

				strcpy(InternalClient, pTmpLst->pstrValue);
			    pTmpLst = pTmpLst->pNext;
			   
			    strcpy(PortMappingDescription, pTmpLst->pstrValue);

				TR064_DestroyStrLink(pStWanPPPConnLst);

                errorinfo |= OutputCharValueToAC(ac, VAR_PortMappingEnabled, PortMappingEnabled);
			    errorinfo |= OutputCharValueToAC(ac, VAR_InternalPort, InternalPort);
			    errorinfo |= OutputCharValueToAC(ac, VAR_InternalClient, InternalClient);
			    errorinfo |= OutputCharValueToAC(ac, VAR_PortMappingDescription, PortMappingDescription);

			    if(!errorinfo)
			    { 
				   CMM_FreeInstanceList(ppWanPPPConnPortsLst);
			       return TRUE;
			    }

				TR64FE_TRACE("failed to ppp_GetSpecificPortMappingEntry ");
				break ;
				
           }else
           {
             TR064_DestroyStrLink(pStWanPPPConnLst);  
           }
      }
	  CMM_FreeInstanceList(ppWanPPPConnPortsLst);
   } 
	      
   soap_error( uclient, SOAP_NOSUCHENTRYINARRAY );
   return FALSE;
}

int ppp_AddPortMappingEntry( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szWanPPPConnNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szWanPPPConnNodeName , sizeof(szWanPPPConnNodeName),TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.PortMapping.",
	 	                                              psvc->instance);
   unsigned long ulInstanceId ;
   if(CMM_AddInstance(szWanPPPConnNodeName,&ulInstanceId )!= CMM_SUCCESS)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   char szExternalPort[CMM_MAX_NODE_LEN]={0} ;
   char szPortMapProtocol[CMM_MAX_NODE_LEN]={0} ;
   char szPortMapEnable[CMM_MAX_NODE_LEN]={0} ;
   char szInternalPort[CMM_MAX_NODE_LEN]={0} ;
   char szInternalClnt[CMM_MAX_NODE_LEN]={0} ;
   char szPortMapDesc[CMM_MAX_NODE_LEN]={0} ;
   
   char* szLeafNames[6]={szExternalPort,szPortMapProtocol,szPortMapEnable,szInternalPort,szInternalClnt,szPortMapDesc};
   sprintf(szLeafNames[0], "%s%d.ExternalPort",szWanPPPConnNodeName,ulInstanceId);
   sprintf(szLeafNames[1], "%s%d.PortMappingProtocol",szWanPPPConnNodeName,ulInstanceId);
   sprintf(szLeafNames[2], "%s%d.PortMappingEnabled",szWanPPPConnNodeName,ulInstanceId);
   sprintf(szLeafNames[3], "%s%d.InternalPort",szWanPPPConnNodeName,ulInstanceId);
   sprintf(szLeafNames[4], "%s%d.InternalClient",szWanPPPConnNodeName,ulInstanceId);
   sprintf(szLeafNames[5], "%s%d.PortMappingDescription",szWanPPPConnNodeName,ulInstanceId);
   int relatedVars[6] = {VAR_ExternalPort,VAR_PortMappingProtocol,VAR_PortMappingEnabled,
   	                     VAR_InternalPort,VAR_InternalClient,VAR_PortMappingDescription};
   SetTr64Values(szLeafNames,relatedVars ,6);
}

int ppp_DeletePortMappingEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char ExternalPort[32];
   char PortMappingProtocol[32];

   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac,VAR_ExternalPort);
   if (pParams != NULL)
   {
	  strcpy(ExternalPort, pParams->value);   
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   pParams = findActionParamByRelatedVar(ac,VAR_PortMappingProtocol);
   if (pParams != NULL)
   {
      strcpy(PortMappingProtocol, pParams->value);   
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;      
   }

   char* aszLeafName[2]= {"ExternalPort","PortMappingProtocol"};

   char szWanPPPConnNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szWanPPPConnNodeName ,sizeof(szWanPPPConnNodeName), TR064_ROOT_WanConnectionDevice"%d.WANPPPConnection.1.PortMapping." ,
	 	                                              psvc->instance);
   char **ppWanPPPConnPortsLst = NULL;
   ppWanPPPConnPortsLst= CMM_GetInstanceList(szWanPPPConnNodeName);
   if(ppWanPPPConnPortsLst != NULL) 
   {  
      int i= 0 ;
      for(; ppWanPPPConnPortsLst[i]; i++)
      {
		   ST_TR064STR_LINK *pStWanPPPConnLink;
		   pStWanPPPConnLink = CMM_GetStrs(ppWanPPPConnPortsLst[i],aszLeafName,2);
		   if(pStWanPPPConnLink == NULL)
		   {
		       soap_error( uclient, SOAP_ACTIONFAILED );
			   CMM_FreeInstanceList(ppWanPPPConnPortsLst);
		       return FALSE;
		   }

           if( 0 == strcmp( ExternalPort, pStWanPPPConnLink->pstrValue) 
               && 0 == strcmp(PortMappingProtocol, pStWanPPPConnLink->pNext->pstrValue))
           {
				TR064_DestroyStrLink(pStWanPPPConnLink);
			    if(CMM_DelInstance(ppWanPPPConnPortsLst[i]) == CMM_SUCCESS)
			    { 
					 CMM_FreeInstanceList(ppWanPPPConnPortsLst);
		             return TRUE; 
			    }
				TR64FE_TRACE("failed to ppp_DeletePortMappingEntry") ;
                break ;
           }else
           {
           	 TR064_DestroyStrLink(pStWanPPPConnLink);
           }
      }
     
	  CMM_FreeInstanceList(ppWanPPPConnPortsLst);
   }

   soap_error( uclient, SOAP_ACTIONFAILED );
   return FALSE;
}

