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
//  Filename:       lanhandlers.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "lanhostcfgmgtparams.h"
#include "tr64defs.h"
#include "cmmif.h"

int LANHostConfigManagement_GetVar(struct Service *psvc, int varindex)
{
   struct StateVar *var;
   var = &(psvc->vars[varindex]);
   char szAdslValue[CMM_MAX_NODE_LEN] ;
   
   switch (varindex) 
   {
      case VAR_DHCPServerEnable:
	  	 if(CMM_GetStr( TR064_ROOT_LanHostMgmt"DHCPServerEnable", var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)
		 {
		      TR64FE_TRACE("Could not get current DHCPServerEnable");
		      return FALSE;
		 }

         break;

      case VAR_DHCPRelay:
         if(CMM_GetStr( TR064_ROOT_LanHostMgmt"DHCPRelay", var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)
		 {
		      TR64FE_TRACE("Could not get current DHCPRelay");
		      return FALSE;
		 }
		
      break;
      
      case VAR_MinAddress:
         if(CMM_GetStr(TR064_ROOT_LanHostMgmt"MinAddress", var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current MinAddress ");
		      return FALSE;
	     }
        
      break;

      case VAR_MaxAddress:
	  	 if(CMM_GetStr(TR064_ROOT_LanHostMgmt"MaxAddress", var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current MaxAddress ");
		      return FALSE;
	     }
      break;

      case VAR_ReservedAddresses:
	  	 if(CMM_GetStr(TR064_ROOT_LanHostMgmt"X_TWSZ-COM_DHCPFilter.WhiteList.FilterIP", var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current ReservedAddresses ");
		      return FALSE;
	     }
         
      break;

      case VAR_SubnetMask:
         if(CMM_GetStr(TR064_ROOT_LanHostMgmt"SubnetMask", var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current ReservedAddresses ");
		      return FALSE;
	     }
         
      break;
      
      case VAR_DHCPLeaseTime:
	  	 if(CMM_GetStr( TR064_ROOT_LanHostMgmt"DHCPLeaseTime", var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)
		 {
		      TR64FE_TRACE("Could not get current DHCPLeaseTime");
		      return FALSE;
		 }
      break;
   
      case VAR_DNSServers:
         if(CMM_GetStr(TR064_ROOT_LanHostMgmt"DNSServers", var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current DNSServers ");
		      return FALSE;
	     }
      break;
   
      case VAR_DomainName:
         if(CMM_GetStr(TR064_ROOT_LanHostMgmt"DomainName", var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current DomainName ");
		      return FALSE;
	     }
      break;
      
      case VAR_IPRouters:
         if(CMM_GetStr(TR064_ROOT_LanHostMgmt"IPRouters", var->value,sizeof(var->value), NULL, 0) != CMM_SUCCESS)
	     {
              TR64FE_TRACE("Could not get current IPRouters ");
		      return FALSE;
	     }
       break;   
    }

	if( 0 == strcmp(var->value ,""))
	{
	   strcpy(var->value,"0");
	}
	
    return TRUE;
}

int SetDHCPLeaseTime(UFILE *uclient, PService psvc, PAction ac,pvar_entry_t args, int nargs)
{
   SetTr64Value(TR064_ROOT_LanHostMgmt"DHCPLeaseTime",VAR_DHCPLeaseTime);
}

/* not support */
int SetDHCPServerConfigurable(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   return TRUE;
}

int SetIPRouter(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   SetTr64Value(TR064_ROOT_LanHostMgmt"IPRouters",VAR_IPRouters);
}

int SetDomainName(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   SetTr64Value(TR064_ROOT_LanHostMgmt"DomainName",VAR_DomainName);
}

int SetAddressRange(UFILE *uclient, PService psvc, PAction ac,pvar_entry_t args, int nargs)
{
   char* szLeafNames[2]={TR064_ROOT_LanHostMgmt"MaxAddress",TR064_ROOT_LanHostMgmt"MinAddress"};
   int  relatedVars[2] = {VAR_MaxAddress,VAR_MinAddress} ;
   SetTr64Values(szLeafNames,relatedVars ,2) ;
}

int GetAddressRange(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char* aszLeafName[2]= {"MinAddress" ,"MaxAddress"};
   
   ST_TR064STR_LINK *pStIPIfLink;
   pStIPIfLink = CMM_GetStrs(TR064_ROOT_LanHostMgmt,aszLeafName,2);
   if(pStIPIfLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   static char szMinAddress[32];
   static char szMaxAddress[32];
   strcpy(szMinAddress, pStIPIfLink->pstrValue);
   strcpy(szMaxAddress, pStIPIfLink->pNext->pstrValue);
   TR064_DestroyStrLink(pStIPIfLink);

   struct Param *pParams ;
   pParams = findActionParamByRelatedVar(ac, VAR_MinAddress);
   if(pParams != NULL)
   {
      pParams->value = szMinAddress ;
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   pParams = findActionParamByRelatedVar(ac, VAR_MaxAddress);
   if (pParams != NULL)
   {
      pParams->value = szMaxAddress;
   }
   else
   { 
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }   
   
   return TRUE;
}

int SetDNSServer(UFILE *uclient, PService psvc, PAction ac,pvar_entry_t args, int nargs)
{
   SetTr64Value(TR064_ROOT_LanHostMgmt"DNSServers",VAR_DNSServers);
}

int DeleteDNSServer(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
#ifdef LGD_TODO
#endif
   soap_error( uclient, SOAP_ACTIONFAILED );
   return FALSE;
}

int SetDHCPServerEnable(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   SetTr64Value(TR064_ROOT_LanHostMgmt"DHCPServerEnable",VAR_DHCPServerEnable);
}

int SetSubnetMask(UFILE *uclient, PService psvc, PAction ac,
                      pvar_entry_t args, int nargs)
{
   SetTr64Value(TR064_ROOT_LanHostMgmt"SubnetMask",VAR_SubnetMask);
}

int GetIPInterfaceNumberOfEntries(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   static char szIndex[16] ;
   if(CMM_GetStr( TR064_ROOT_LanHostMgmt"IPInterfaceNumberOfEntries", szIndex,sizeof(szIndex), NULL, 0) != CMM_SUCCESS)	
   {
   	  soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac, VAR_IPInterfaceNumberOfEntries);
   if (pParams != NULL)
   {
      pParams->value = szIndex;
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   return TRUE;
}

int GetIPInterfaceGenericEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   int index = 0;
   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac, VAR_IPInterfaceNumberOfEntries);
   if (pParams != NULL)
   {
      index = atoi(pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   char szEnable[CMM_MAX_LEAF_LEN]={0};
   char szIPIfIPAdd[CMM_MAX_LEAF_LEN]={0};
   char szIPIfSubnetMask[CMM_MAX_LEAF_LEN]={0};
   char* aszLeafName[3]= {szEnable ,szIPIfIPAdd, szIPIfSubnetMask };
   sprintf(aszLeafName[0],"IPInterface.%d.Enable" ,index);
   sprintf(aszLeafName[1],"IPInterface.%d.IPInterfaceIPAddress" ,index);
   sprintf(aszLeafName[2],"IPInterface.%d.IPInterfaceSubnetMask" ,index);

   ST_TR064STR_LINK* pStIPIfLink = NULL;
   pStIPIfLink = CMM_GetStrs(TR064_ROOT_LanHostMgmt,aszLeafName,3);
   
   static char Enable[4];
   static char IPAddress[32];
   static char SubnetMask[32];
   if(pStIPIfLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   strcpy(Enable , pStIPIfLink->pstrValue);
   strcpy(IPAddress ,pStIPIfLink->pNext->pstrValue);
   strcpy(SubnetMask ,pStIPIfLink->pNext->pNext->pstrValue);
   TR064_DestroyStrLink(pStIPIfLink);

   pParams = findActionParamByRelatedVar(ac, VAR_Enable);
   if(pParams != NULL)
   {
      pParams->value = Enable;
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   pParams = findActionParamByRelatedVar(ac, VAR_IPInterfaceIPAddress);
   if (pParams != NULL)
   {
      pParams->value = IPAddress;
   }
   else
   { 
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   
   pParams = findActionParamByRelatedVar(ac, VAR_IPInterfaceSubnetMask);
   if (pParams != NULL)
   {
      pParams->value = SubnetMask;
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }     

   return TRUE;
}

int SetIPInterface(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac, VAR_IPInterfaceNumberOfEntries);
   if (pParams == NULL)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   char szEnable[120] = {0};
   char szIPIfAddr[120] = {0};
   char szIPIfMask[120] = {0};
   char* szLeafNames[120]={ szEnable , szIPIfAddr,szIPIfMask };
   //enable 
   sprintf(szLeafNames[0], TR064_ROOT_LanHostMgmt"IPInterface.%s.Enable" ,pParams->value);
   //ip address
   sprintf(szLeafNames[1], TR064_ROOT_LanHostMgmt"IPInterface.%s.IPInterfaceIPAddress" ,pParams->value);
   //subnetaddress
   sprintf(szLeafNames[2], TR064_ROOT_LanHostMgmt"IPInterface.%s.IPInterfaceSubnetMask" ,pParams->value);
  
   int relatedVars[3] = {VAR_Enable,VAR_IPInterfaceIPAddress,VAR_IPInterfaceSubnetMask};
   
   SetTr64Values(szLeafNames,relatedVars ,3);
}
