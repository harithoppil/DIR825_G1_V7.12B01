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
//  Filename:       wanipconnectionhandlers.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "wanipconnection.h"
#include "cmmif.h"

#define GetIPConnValue(szLeafName) \
do{\
    snprintf(szFullPathName, sizeof(szFullPathName), TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.%s" ,psvc->instance,szLeafName);\
  	if(CMM_GetStr(szFullPathName, var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)\
	{\
	      TR64FE_TRACE("Could not get current WANIPConnection %s ",szLeafName);\
	      return FALSE;\
	}\
	if( 0 ==strcmp(var->value ,""))\
	{\
	  strcpy(var->value ,"0");\ 
	}\
	return TRUE; \
}while(0)

int WANIPConnection_GetVar(struct Service *psvc, int varindex)
{
   char szFullPathName[CMM_MAX_NODE_LEN];
   struct StateVar *var;
   var = &(psvc->vars[varindex]);
   
   switch (varindex)
   {
      case VAR_Enable:
         GetIPConnValue("Enable");
         break;
      case VAR_ConnectionType:
         GetIPConnValue("ConnectionType");
         break;
      case VAR_PossibleConnectionTypes:
         GetIPConnValue("PossibleConnectionTypes"); //not support
         break;
      case VAR_ConnectionStatus:
         GetIPConnValue("ConnectionStatus");
         break;
      case VAR_Name:
         GetIPConnValue("Name");
         break;
      case VAR_Uptime:
        // GetIPConnValue("Uptime");
        strcpy(var->value ,"0");\ 
         break;
      case VAR_NATEnabled:
         GetIPConnValue("NATEnabled");
         break;
      case VAR_ExternalIPAddress:
         GetIPConnValue("ExternalIPAddress");
         break;
      case VAR_AddressingType:
         GetIPConnValue("AddressingType");
         break;       
      case VAR_SubnetMask:
         GetIPConnValue("SubnetMask");
         break;
      case VAR_DefaultGateway:
         GetIPConnValue("DefaultGateway");
         break;   
      case VAR_DNSServers:
         GetIPConnValue("DNSServers");
         break;   
      case VAR_ConnectionTrigger:
         //GetIPConnValue("ConnectionTrigger");
         strcpy(var->value ,"0");
         break;            
      case VAR_BytesSent:
         GetIPConnValue("Stats.EthernetBytesSent");
         break;
      case VAR_BytesReceived:
         GetIPConnValue("Stats.EthernetBytesReceived");
         break;
      case VAR_PacketsSent:
         GetIPConnValue("Stats.EthernetPacketsSent");
         break;
      case VAR_PacketsReceived:
         GetIPConnValue("Stats.EthernetPacketsReceived");
         break;
   }
   return TRUE;
}

int SetWANIPConnEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
    char szNodeName[CMM_MAX_NODE_LEN]={0};
    snprintf(szNodeName ,sizeof(szNodeName),TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.Enable" ,
	 	                                              psvc->instance);
    SetTr64Value(szNodeName,VAR_Enable);
}

int SetIPConnectionType(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
    char szNodeName[CMM_MAX_NODE_LEN]={0};
    snprintf(szNodeName ,sizeof(szNodeName),TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.ConnectionType" ,
	 	                                             psvc->instance);
    SetTr64Value(szNodeName,VAR_ConnectionType);
}
       
int SetIPInterfaceInfo(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szAddressingType[CMM_MAX_NODE_LEN];
   snprintf(szAddressingType ,sizeof(szAddressingType),TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.AddressingType" ,
	 	                                             psvc->instance);
   char szExtIPAddress[CMM_MAX_NODE_LEN];
   snprintf(szExtIPAddress ,sizeof(szExtIPAddress),TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.ExternalIPAddress" ,
	 	                                             psvc->instance);

   char szSubnetMask[CMM_MAX_NODE_LEN];
   snprintf(szSubnetMask ,sizeof(szSubnetMask),TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.SubnetMask" ,
	 	                                             psvc->instance);
   char szDefaultGateway[CMM_MAX_NODE_LEN];
   snprintf(szDefaultGateway ,sizeof(szDefaultGateway),TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.DefaultGateway" ,
	 	                                             psvc->instance);
   
   char* szLeafNames[4]={szAddressingType,szExtIPAddress,szSubnetMask,szDefaultGateway};
   int  relatedVars[4] = {VAR_AddressingType,VAR_ExternalIPAddress, VAR_SubnetMask,VAR_DefaultGateway};
   SetTr64Values(szLeafNames,relatedVars ,4);
}

int SetIPConnectionTrigger(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
    //not support
    soap_error( uclient, SOAP_ACTIONFAILED );
    return FALSE;

}

int ForceTerminationTR64(UFILE *uclient, PService psvc, PAction ac,
                     pvar_entry_t args, int nargs)
{
#ifdef LGD_TODO
#endif    
   return TRUE;
}

int RequestTerminationTR64(UFILE *uclient, PService psvc, PAction ac,
                     pvar_entry_t args, int nargs)
{
#ifdef LGD_TODO
#endif    
   return TRUE;
}

int RequestConnectionTR64(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
#ifdef LGD_TODO
#endif
   return TRUE;
}

int GetPortMappingNumberOfEntries(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   int  errorinfo = 0;
   char szNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szNodeName ,sizeof(szNodeName), TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.PortMappingNumberOfEntries",
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

int GetGenericPortMappingEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
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

   char szWanIPConnNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szWanIPConnNodeName , sizeof(szWanIPConnNodeName),TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.PortMapping.%d" ,
	 	                                              psvc->instance ,PortMappingIndex);
   char* aszLeafName[6]= {"PortMappingEnabled" ,"ExternalPort","InternalPort","PortMappingProtocol",
   	                                       "InternalClient" ,"PortMappingDescription"};
   
   ST_TR064STR_LINK *pStWanIPConnLst;
   pStWanIPConnLst = CMM_GetStrs(szWanIPConnNodeName,aszLeafName,6);
   if(pStWanIPConnLst == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpLst;
   pTmpLst = pStWanIPConnLst;
   
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
   
   TR064_DestroyStrLink(pStWanIPConnLst);

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

int GetSpecificPortMappingEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   int  errorinfo = 0;
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

   char* aszLeafName[6]= {"ExternalPort","PortMappingProtocol","PortMappingEnabled","InternalPort", 
   	                                       "InternalClient" ,"PortMappingDescription"};

   char szWanIPConnNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szWanIPConnNodeName ,sizeof(szWanIPConnNodeName),TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.PortMapping." ,
	 	                                              psvc->instance);
   char **ppWanIPConnPortsLst = NULL;
   ppWanIPConnPortsLst= CMM_GetInstanceList(szWanIPConnNodeName);
   if(ppWanIPConnPortsLst != NULL) 
   {  
      int i= 0 ;
      for(; ppWanIPConnPortsLst[i]; i++)
      {
		   ST_TR064STR_LINK *pStWanIPConnLst;
		   pStWanIPConnLst = CMM_GetStrs(ppWanIPConnPortsLst[i],aszLeafName,6);
		   if(pStWanIPConnLst == NULL)
		   {
		       soap_error( uclient, SOAP_ACTIONFAILED );
			   CMM_FreeInstanceList(ppWanIPConnPortsLst);
		       return FALSE;
		   }

           if(( 0 == strcmp(ExternalPort, pStWanIPConnLst->pstrValue))
                &&(0==strcmp(PortMappingProtocol, pStWanIPConnLst->pNext->pstrValue)))
           {
                ST_TR064STR_LINK *pTmpLst ;
                pTmpLst = pStWanIPConnLst;
				
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

				TR064_DestroyStrLink(pStWanIPConnLst);

                errorinfo |= OutputCharValueToAC(ac, VAR_PortMappingEnabled, PortMappingEnabled);
			    errorinfo |= OutputCharValueToAC(ac, VAR_InternalPort, InternalPort);
			    errorinfo |= OutputCharValueToAC(ac, VAR_InternalClient, InternalClient);
			    errorinfo |= OutputCharValueToAC(ac, VAR_PortMappingDescription, PortMappingDescription);
			    if(!errorinfo)
			    {  
				   CMM_FreeInstanceList(ppWanIPConnPortsLst);
			       return TRUE;
			    }
				
                TR64FE_TRACE("failed to GetSpecificPortMappingEntry ") ;
				break ;	
           }else
           {
              TR064_DestroyStrLink(pStWanIPConnLst);
           }
      }
     
	  CMM_FreeInstanceList(ppWanIPConnPortsLst);
   } 
	      
   soap_error( uclient, SOAP_NOSUCHENTRYINARRAY );
   return FALSE;
}

int AddPortMappingEntry( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szWanIPConnNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szWanIPConnNodeName , sizeof(szWanIPConnNodeName),TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.PortMapping.",
	 	                                              psvc->instance);
   unsigned long ulInstanceId ;
   if(CMM_AddInstance(szWanIPConnNodeName,&ulInstanceId )!= CMM_SUCCESS)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   char szExternalPort[CMM_MAX_NODE_LEN] = {0};
   char szPortMappingProtocol[CMM_MAX_NODE_LEN] = {0};
   char szPortMappingEnabled[CMM_MAX_NODE_LEN] = {0};
   char szInternalPort[CMM_MAX_NODE_LEN] = {0};
   char szInternalClient[CMM_MAX_NODE_LEN] = {0};
   char szPortMappingDesc[CMM_MAX_NODE_LEN] = {0};
   
   char* szLeafNames[6]={szExternalPort,szPortMappingProtocol, szPortMappingEnabled ,
   	                     szInternalPort,szInternalClient, szPortMappingDesc};

   sprintf(szLeafNames[0], "%s%d.ExternalPort",szWanIPConnNodeName,ulInstanceId);
   sprintf(szLeafNames[1], "%s%d.PortMappingProtocol",szWanIPConnNodeName,ulInstanceId);
   sprintf(szLeafNames[2], "%s%d.PortMappingEnabled",szWanIPConnNodeName,ulInstanceId);
   sprintf(szLeafNames[3], "%s%d.InternalPort",szWanIPConnNodeName,ulInstanceId);
   sprintf(szLeafNames[4], "%s%d.InternalClient",szWanIPConnNodeName,ulInstanceId);
   sprintf(szLeafNames[5], "%s%d.PortMappingDescription",szWanIPConnNodeName,ulInstanceId);
   int relatedVars[6] = {VAR_ExternalPort,VAR_PortMappingProtocol,VAR_PortMappingEnabled,
   	                     VAR_InternalPort,VAR_InternalClient,VAR_PortMappingDescription};
   SetTr64Values(szLeafNames,relatedVars ,6);

}

int DeletePortMappingEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
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

   char szWanIPConnNodeName[CMM_MAX_NODE_LEN]={0};
   snprintf(szWanIPConnNodeName ,sizeof(szWanIPConnNodeName), TR064_ROOT_WanConnectionDevice"%d.WANIPConnection.1.PortMapping." ,
	 	                                              psvc->instance);
   char **ppWanIPConnPortsLst = NULL;
   ppWanIPConnPortsLst= CMM_GetInstanceList(szWanIPConnNodeName);
   if(ppWanIPConnPortsLst != NULL) 
   {  
      int i= 0 ;
      for(; ppWanIPConnPortsLst[i]; i++)
      {
		   ST_TR064STR_LINK *pStWanIPConnLst;
		   pStWanIPConnLst = CMM_GetStrs(ppWanIPConnPortsLst[i],aszLeafName,2);
		   if(pStWanIPConnLst == NULL)
		   {
		       soap_error( uclient, SOAP_ACTIONFAILED );
			   CMM_FreeInstanceList(ppWanIPConnPortsLst);
		       return FALSE;
		   }

           if( 0 == strcmp( ExternalPort, pStWanIPConnLst->pstrValue) 
               && 0 == strcmp(PortMappingProtocol, pStWanIPConnLst->pNext->pstrValue))
           {
				TR064_DestroyStrLink(pStWanIPConnLst);
			    if(CMM_DelInstance(ppWanIPConnPortsLst[i]) == CMM_SUCCESS)
			    { 
					  CMM_FreeInstanceList(ppWanIPConnPortsLst);
		              return TRUE;   
			    }
                TR64FE_TRACE("failed to DeletePortMappingEntry");
				break ;
				
           }else
           {
             TR064_DestroyStrLink(pStWanIPConnLst);
           }   
      }
     
	  CMM_FreeInstanceList(ppWanIPConnPortsLst);
   }

   soap_error( uclient, SOAP_ACTIONFAILED );
   return FALSE;
}

