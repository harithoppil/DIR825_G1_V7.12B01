#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"

#include "lanhostsparams.h"
#include "cmmif.h"


int GetHostNumberOfEntries(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)      
{
   static char HostNumberOfEntries[32];
   if(CMM_GetStr(TR064_ROOT_LanHostCfg"HostNumberOfEntries", HostNumberOfEntries,sizeof(HostNumberOfEntries), NULL, 0) != CMM_SUCCESS)
   {
	      TR64FE_TRACE("Could not get current LANHostCfg HostNumberOfEntries");
	      return FALSE;
   }

   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_HostNumberOfEntries, HostNumberOfEntries);
   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   }
 
   return TRUE;
}

int GetGenericHostEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   int index = 0 ;

   pParams = findActionParamByRelatedVar(ac,VAR_HostNumberOfEntries);
   if (pParams != NULL)
   {
       index = atoi(pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   char* aszLeafName[5]= {"IPAddress" ,"AddressSource","LeaseTimeRemaining","MACAddress","HostName"};

   char  szPathName[60]= {0};
   snprintf(szPathName, sizeof(szPathName), TR064_ROOT_LanHostCfg"Host.%d.", index);
   ST_TR064STR_LINK *pStLanHostCfgLink;
   pStLanHostCfgLink = CMM_GetStrs(szPathName,aszLeafName,5);
   if(pStLanHostCfgLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   static char IPAddress[32];
   static char AddressSource[32];
   static char LeaseTimeRemaining[32];
   static char MACAddress[32];
   static char InterfaceType[32]={0}; //""
   static char Active[32] ={0}; //"1" ;
   static char HostName[32];
   ST_TR064STR_LINK *pTmpStLanHostCfgLink = pStLanHostCfgLink ;

   strcpy(IPAddress, pTmpStLanHostCfgLink->pstrValue);
   pTmpStLanHostCfgLink = pTmpStLanHostCfgLink->pNext;
   
   strcpy(AddressSource, pTmpStLanHostCfgLink->pstrValue);
   pTmpStLanHostCfgLink = pTmpStLanHostCfgLink->pNext;

   strcpy(LeaseTimeRemaining, pTmpStLanHostCfgLink->pstrValue);
   pTmpStLanHostCfgLink = pTmpStLanHostCfgLink->pNext;

   strcpy(MACAddress, pTmpStLanHostCfgLink->pstrValue);
   pTmpStLanHostCfgLink = pTmpStLanHostCfgLink->pNext;

   strcpy(HostName, pTmpStLanHostCfgLink->pstrValue);

   TR064_DestroyStrLink(pStLanHostCfgLink);

   strcpy(InterfaceType, " ");
   strcpy(Active, "1");

   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_IPAddress, IPAddress);
   errorinfo |= OutputCharValueToAC(ac, VAR_AddressSource, AddressSource);
   errorinfo |= OutputCharValueToAC(ac, VAR_LeaseTimeRemaining, LeaseTimeRemaining);
   errorinfo |= OutputCharValueToAC(ac, VAR_MACAddress, MACAddress);
   errorinfo |= OutputCharValueToAC(ac, VAR_InterfaceType, InterfaceType);
   errorinfo |= OutputCharValueToAC(ac, VAR_Active, Active);
   errorinfo |= OutputCharValueToAC(ac, VAR_HostName, HostName);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}

int GetSpecificHostEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   static char MACAddress[32];
   struct Param* pParams = findActionParamByRelatedVar(ac,VAR_MACAddress);
   if (pParams != NULL)
   {
      strcpy(MACAddress, pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   char **ppLanHostList = NULL;
   ppLanHostList= CMM_GetInstanceList(TR064_ROOT_LanHostCfg"Host.");

   if(ppLanHostList != NULL) 
   {  
      char* aszLeafName[5]= {"MACAddress","IPAddress","AddressSource","LeaseTimeRemaining","HostName"};
	  char szPathName[60]= {0};
      int i= 0 ;
      for(; ppLanHostList[i]; i++)
      {
          int nLanHostInstanceId = atoi(ppLanHostList[i]+sizeof(TR064_ROOT_LanHostCfg"Host.") - 1); //Get the instance NO.
		  memset(szPathName,0,sizeof(szPathName));
		  snprintf(szPathName, sizeof(szPathName), TR064_ROOT_LanHostCfg"Host.%d.", nLanHostInstanceId);
		  ST_TR064STR_LINK *pStLanHostCfgLink;
		  pStLanHostCfgLink = CMM_GetStrs(szPathName,aszLeafName,5);
		  if(pStLanHostCfgLink == NULL)
		  {
		       soap_error(uclient, SOAP_ACTIONFAILED );
		       return FALSE;
		  }

		  if( 0 == strcmp(MACAddress, pStLanHostCfgLink->pstrValue))
	      {
	           static char IPAddress[32];
			   static char AddressSource[32];
			   static char LeaseTimeRemaining[32];
			   static char InterfaceType[32]={0}; //""
			   static char Active[32] ={0}; //"1" ;
			   static char HostName[32];
			   ST_TR064STR_LINK *pTmpStLanHostCfgLink = pStLanHostCfgLink ;
			   pTmpStLanHostCfgLink = pTmpStLanHostCfgLink->pNext;

			   strcpy(IPAddress, pTmpStLanHostCfgLink->pstrValue);
			   pTmpStLanHostCfgLink = pTmpStLanHostCfgLink->pNext;
			   
			   strcpy(AddressSource, pTmpStLanHostCfgLink->pstrValue);
			   pTmpStLanHostCfgLink = pTmpStLanHostCfgLink->pNext;

			   strcpy(LeaseTimeRemaining, pTmpStLanHostCfgLink->pstrValue);
			   pTmpStLanHostCfgLink = pTmpStLanHostCfgLink->pNext;

			   strcpy(HostName, pTmpStLanHostCfgLink->pstrValue);

			   TR064_DestroyStrLink(pStLanHostCfgLink);

			   strcpy(InterfaceType, " ");
			   strcpy(Active, "1");

			   int errorinfo = 0;
			   errorinfo |= OutputCharValueToAC(ac, VAR_IPAddress, IPAddress);
			   errorinfo |= OutputCharValueToAC(ac, VAR_AddressSource, AddressSource);
			   errorinfo |= OutputCharValueToAC(ac, VAR_LeaseTimeRemaining, LeaseTimeRemaining);
			   errorinfo |= OutputCharValueToAC(ac, VAR_MACAddress, MACAddress);
			   errorinfo |= OutputCharValueToAC(ac, VAR_InterfaceType, InterfaceType);
			   errorinfo |= OutputCharValueToAC(ac, VAR_Active, Active);
			   errorinfo |= OutputCharValueToAC(ac, VAR_HostName, HostName);

			   if(errorinfo)
			   {
			      soap_error( uclient, errorinfo );
			      return FALSE;
			   } 
			   return TRUE;
	      }
	  }
   }

   soap_error(uclient, SOAP_NOSUCHENTRYINARRAY);
   return FALSE;
}

