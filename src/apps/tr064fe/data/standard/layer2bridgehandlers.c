#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "layer2bridgeparams.h"
#include "tr64defs.h"
#include "cmmif.h"

int GetLayer2Bridge_Info(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   char* aszLeafName[7]= {"MaxBridgeEntries" ,"MaxFilterEntries","MaxMarkingEntries","BridgeNumberOfEntries",
   	        "FilterNumberOfEntries","MarkingNumberOfEntries","AvailableInterfaceNumberOfEntries"};
   
   ST_TR064STR_LINK *pStLayer2BridgeLink;
   
   pStLayer2BridgeLink = CMM_GetStrs(TR064_ROOT_L2BrdgCfg,aszLeafName,7);
   if(pStLayer2BridgeLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   static char MaxBridgeEntries[4];
   static char MaxFilterEntries[4];
   static char MaxMarkingEntries[4];   
   static char BridgeNumberOfEntries[4];   
   static char FilterNumberOfEntries[4];   
   static char MarkingNumberOfEntries[4];   
   static char AvailableInterfaceNumberOfEntries[4];

   ST_TR064STR_LINK *pTmpLayer2BridgeLink = pStLayer2BridgeLink ;

   strcpy(MaxBridgeEntries, pTmpLayer2BridgeLink->pstrValue);
   pTmpLayer2BridgeLink = pTmpLayer2BridgeLink->pNext;

   strcpy(MaxFilterEntries, pTmpLayer2BridgeLink->pstrValue);
   pTmpLayer2BridgeLink = pTmpLayer2BridgeLink->pNext;

   strcpy(MaxMarkingEntries, pTmpLayer2BridgeLink->pstrValue);
   pTmpLayer2BridgeLink = pTmpLayer2BridgeLink->pNext;

   strcpy(BridgeNumberOfEntries, pTmpLayer2BridgeLink->pstrValue);
   pTmpLayer2BridgeLink = pTmpLayer2BridgeLink->pNext;

   strcpy(FilterNumberOfEntries, pTmpLayer2BridgeLink->pstrValue);
   pTmpLayer2BridgeLink = pTmpLayer2BridgeLink->pNext;

   strcpy(MarkingNumberOfEntries, pTmpLayer2BridgeLink->pstrValue);
   pTmpLayer2BridgeLink = pTmpLayer2BridgeLink->pNext;

   strcpy(AvailableInterfaceNumberOfEntries, pTmpLayer2BridgeLink->pstrValue);
   
   TR064_DestroyStrLink(pStLayer2BridgeLink);
   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_MaxBridgeEntries, MaxBridgeEntries);  
   errorinfo |= OutputCharValueToAC(ac, VAR_MaxFilterEntries, MaxFilterEntries);
   errorinfo |= OutputCharValueToAC(ac, VAR_MaxMarkingEntries, MaxMarkingEntries);
   errorinfo |= OutputCharValueToAC(ac, VAR_BridgeNumberOfEntries, BridgeNumberOfEntries);
   errorinfo |= OutputCharValueToAC(ac, VAR_FilterNumberOfEntries, FilterNumberOfEntries);
   errorinfo |= OutputCharValueToAC(ac, VAR_MarkingNumberOfEntries, MarkingNumberOfEntries);
   errorinfo |= OutputCharValueToAC(ac, VAR_AvailableInterfaceNumberOfEntries, AvailableInterfaceNumberOfEntries);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}

int AddBridgeEntry( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   unsigned long ulInstanceId ;
   if(CMM_AddInstance(TR064_ROOT_L2BrdgCfg"Bridge.",&ulInstanceId )!= CMM_SUCCESS)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   char szBridgeEnable[80]={0};
   char szBridgeName[80]={0};
   char szVLANID[80]={0};
 
   char* szLeafNames[3] = {szBridgeEnable ,szBridgeName,szVLANID};
   sprintf(szLeafNames[0], TR064_ROOT_L2BrdgCfg"Bridge.%d.BridgeEnable",ulInstanceId);
   sprintf(szLeafNames[1], TR064_ROOT_L2BrdgCfg"Bridge.%d.BridgeName",ulInstanceId);
   sprintf(szLeafNames[2], TR064_ROOT_L2BrdgCfg"Bridge.%d.VLANID",ulInstanceId);
   
   int relatedVars[3] = {VAR_BridgeEnable,VAR_BridgeName,VAR_VLANID};

   char* szLeafValues[3] ={0};
   struct Param *pParams;
   int i = 0 ; 
   for( ; i< 3 ;i++) 
   {
      pParams = findActionParamByRelatedVar(ac,relatedVars[i]);
	  if (pParams != NULL)
	  {
	      szLeafValues[i] = pParams->value ;
	  } 
	  else 
	  { 
	     soap_error( uclient, SOAP_ACTIONFAILED ); 
	     return FALSE; 
	  } 
   }

   int nRet = CMM_SetStrs(szLeafNames,szLeafValues, 3) ; 
   if(nRet != CMM_SUCCESS)
   { 
      TR64FE_TRACE("set of %s failed", szLeafNames[0] ); 
      soap_error( uclient, SOAP_ACTIONFAILED ); 
      return FALSE; 
   }
   
   
   char szBridgeKey[80]={0};
   static char szBridgeKeyValue[32];
   sprintf(szBridgeKey, TR064_ROOT_L2BrdgCfg"Bridge.%d.BridgeKey",ulInstanceId);
    
   if(CMM_GetStr(szBridgeKey,szBridgeKeyValue, sizeof(szBridgeKeyValue), NULL, 0)!= CMM_SUCCESS)
   { 
      TR64FE_TRACE("Get of %s failed", szBridgeKey ); 
      soap_error( uclient, SOAP_ACTIONFAILED ); 
      return FALSE; 
   }
   int errorinfo = 0 ;
   errorinfo |= OutputCharValueToAC(ac, VAR_BridgeKey, szBridgeKeyValue);

   if (errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   }
   
   return TRUE;
}

int DeleteBridgeEntry( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   int errorinfo = 0;
   struct Param *pParams;

   char BridgeKey[32]; 
   pParams = findActionParamByRelatedVar(ac,VAR_BridgeKey);
   if (pParams != NULL)
   {
      strcpy(BridgeKey, pParams->value);
   }
   else
   {
      TR64FE_TRACE("input pParams is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }    
  
   char **ppLBridgeLst = NULL;
   ppLBridgeLst= CMM_GetInstanceList(TR064_ROOT_L2BrdgCfg"Bridge.");
   if(ppLBridgeLst != NULL) 
   {  
      char szFindBridgeKey[32];
	  char szFullPathName[60]={0};
      int i= 0 ;
      for(; ppLBridgeLst[i]; i++)
      {
           snprintf(szFullPathName, sizeof(szFullPathName), "%sBridgeKey",ppLBridgeLst[i]);
		   if(CMM_GetStr(szFullPathName, szFindBridgeKey ,sizeof(szFindBridgeKey), NULL, 0) != CMM_SUCCESS)
		   {
			    TR64FE_TRACE("Could not get current BridgeKey ");
				soap_error( uclient, SOAP_ACTIONFAILED );
				CMM_FreeInstanceList(ppLBridgeLst);
			    return FALSE;
		   }

           if( 0 == strcmp(szFindBridgeKey, BridgeKey))           
           {
			    if(CMM_DelInstance(ppLBridgeLst[i]) == CMM_SUCCESS)
			    { 
					  CMM_FreeInstanceList(ppLBridgeLst);
		              return TRUE;  
			    }

				TR64FE_TRACE("CMM_DelInstance failed");
				break ;
           } 
      }
	  CMM_FreeInstanceList(ppLBridgeLst);
   }

   soap_error( uclient, SOAP_ACTIONFAILED );
   return FALSE;
}
      
int GetSpecificBridgeEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{ 
   int  errorinfo = 0;
   char BridgeKey[32];
   Param *pParams = findActionParamByRelatedVar(ac,VAR_BridgeKey);
   if (pParams != NULL)
   {
      strcpy(BridgeKey, pParams->value);
   }
   else
   {
      TR64FE_TRACE("input pParams is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }     

   int  found = 0;
   char* aszLeafName[4]= {"BridgeKey","BridgeEnable","BridgeStatus","VLANID"};

   char **ppL2BridgeLst = NULL;
   ppL2BridgeLst= CMM_GetInstanceList(TR064_ROOT_L2BrdgCfg"Bridge.");
   if(ppL2BridgeLst != NULL) 
   {  
      char szFullPathName[60]={0}; 
      char szfFindBridgeKey[32];  
      int i= 0 ;
      for(; ppL2BridgeLst[i]; i++)
      {
		   ST_TR064STR_LINK *pStL2BridgeLink;
		   pStL2BridgeLink = CMM_GetStrs(ppL2BridgeLst[i],aszLeafName,4);
		   if(szFullPathName == NULL)
		   {
		       soap_error( uclient, SOAP_ACTIONFAILED );
			   CMM_FreeInstanceList(ppL2BridgeLst);
		       return FALSE;
		   }

           if( 0 == strcmp(pStL2BridgeLink->pstrValue, BridgeKey)) 
           {
		        found = 1;
                ST_TR064STR_LINK *pTmpLst ;
                pTmpLst = pStL2BridgeLink;
                pTmpLst = pTmpLst->pNext;
		   
                static char BridgeEnable[4];
			    static char BridgeStatus[16];   
			    static char VLANID[4];   
			   
			    strcpy(BridgeEnable, pTmpLst->pstrValue);
			    pTmpLst = pTmpLst->pNext;
			   
			    strcpy(BridgeStatus, pTmpLst->pstrValue);
			    pTmpLst = pTmpLst->pNext;

				strcpy(VLANID, pTmpLst->pstrValue);

				TR064_DestroyStrLink(pStL2BridgeLink);
				
                errorinfo |= OutputCharValueToAC(ac, VAR_BridgeEnable, BridgeEnable);
			    errorinfo |= OutputCharValueToAC(ac, VAR_BridgeStatus, BridgeStatus);
			    errorinfo |= OutputCharValueToAC(ac, VAR_VLANID, VLANID);

			    if(!errorinfo)
			    {
				   CMM_FreeInstanceList(ppL2BridgeLst);
			       return TRUE;
			    }

				TR64FE_TRACE("GetSpecificBridgeEntry failed");
				break ;
           }else
           {
           	  TR064_DestroyStrLink(pStL2BridgeLink);
           }
      }
     
	  CMM_FreeInstanceList(ppL2BridgeLst);
   } 
 
   soap_error( uclient, errorinfo);
   return FALSE;
}
     
int GetGenericBridgeEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   int  errorinfo = 0;
  
   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac,VAR_BridgeNumberOfEntries);
   if (pParams == NULL)
   {
      TR64FE_TRACE("input pParams is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }     

   char szL2BrdgCfg[CMM_MAX_NODE_LEN]={0};
   snprintf(szL2BrdgCfg , sizeof(szL2BrdgCfg),TR064_ROOT_L2BrdgCfg"Bridge.%s" ,pParams->value);
   char* aszLeafName[4]= {"BridgeKey" ,"BridgeEnable","BridgeStatus","VLANID"};
   
   ST_TR064STR_LINK *pStL2BridgeLink;
   pStL2BridgeLink = CMM_GetStrs(szL2BrdgCfg,aszLeafName,4);
   if(pStL2BridgeLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpLst;
   pTmpLst = pStL2BridgeLink;
   
   static char BridgeKey[32];
   static char BridgeEnable[4];
   static char BridgeStatus[16];   
   static char VLANID[4]; 
   
   strcpy(BridgeKey, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;
   
   strcpy(BridgeEnable, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;
   
   strcpy(BridgeStatus, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;
   
   strcpy(VLANID, pTmpLst->pstrValue);
  
   TR064_DestroyStrLink(pStL2BridgeLink);

   errorinfo |= OutputCharValueToAC(ac, VAR_BridgeKey, BridgeKey);
   errorinfo |= OutputCharValueToAC(ac, VAR_BridgeEnable, BridgeEnable);
   errorinfo |= OutputCharValueToAC(ac, VAR_BridgeStatus, BridgeStatus);
   errorinfo |= OutputCharValueToAC(ac, VAR_VLANID, VLANID);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo);
      return FALSE;
   }

   return TRUE;
}

int SetBridgeEntryEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   int  errorinfo = 0;
   char BridgeKey[32];
   Param *pParams = findActionParamByRelatedVar(ac,VAR_BridgeKey);
   if (pParams != NULL)
   {
      strcpy(BridgeKey, pParams->value);
   }
   else
   {
      //cmsLog_error("input pParams is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }     

   int  found = 0;
   char* aszLeafName[4]= {"BridgeKey","BridgeEnable"};

   int nPrefixLen = strlen(TR064_ROOT_L2BrdgCfg"Bridge.");
   char **ppL2BriageLst = NULL;
   ppL2BriageLst= CMM_GetInstanceList(TR064_ROOT_L2BrdgCfg"Bridge.");
   if(ppL2BriageLst != NULL) 
   {  
      int i= 0 ;
      for(; ppL2BriageLst[i]; i++)
      {
		   ST_TR064STR_LINK *pStL2BridgeLink;
		   pStL2BridgeLink = CMM_GetStrs(ppL2BriageLst[i],aszLeafName,2);
		   if(pStL2BridgeLink == NULL)
		   {
		       soap_error( uclient, SOAP_ACTIONFAILED );
			   CMM_FreeInstanceList(ppL2BriageLst);
		       return FALSE;
		   }

           if( 0 == strcmp(pStL2BridgeLink->pstrValue, BridgeKey)) 
           {   
		        char szNodeName[CMM_MAX_NODE_LEN]={0};
                snprintf(szNodeName ,sizeof(szNodeName),"%sBridgeEnable",ppL2BriageLst[i]);
                TR064_DestroyStrLink(pStL2BridgeLink);
				CMM_FreeInstanceList(ppL2BriageLst);
				 
                SetTr64Value(szNodeName,VAR_BridgeEnable);
           } 
		   
		   TR064_DestroyStrLink(pStL2BridgeLink); 
      }
     
	  CMM_FreeInstanceList(ppL2BriageLst);
   } 
	      
   soap_error( uclient, SOAP_NOSUCHENTRYINARRAY );
   return FALSE;
}

int GetSpecificAvailableInterfaceEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   char AvailableInterfaceKey[32]; 

   Param *pParams = findActionParamByRelatedVar(ac,VAR_AvailableInterfaceKey);
   if (pParams != NULL)
   {
      strcpy(AvailableInterfaceKey, pParams->value);
   }
   else
   {
      TR64FE_TRACE("input pParams is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }     
   
   int  found = 0;
   char* aszLeafName[3]= {"AvailableInterfaceKey","InterfaceType","InterfaceReference"};

   int nPrefixLen = strlen(TR064_ROOT_L2BragIfCfg);
   char **ppL2BrgIfLst = NULL;
   ppL2BrgIfLst= CMM_GetInstanceList(TR064_ROOT_L2BragIfCfg);
   if(ppL2BrgIfLst != NULL) 
   {  
      int i= 0 ;
      for(; ppL2BrgIfLst[i]; i++)
      {
		   ST_TR064STR_LINK *pStL2BridgeLink;
		   pStL2BridgeLink = CMM_GetStrs(ppL2BrgIfLst[i],aszLeafName,3);
		   if(pStL2BridgeLink == NULL)
		   {
		       soap_error( uclient, SOAP_ACTIONFAILED );
			   CMM_FreeInstanceList(ppL2BrgIfLst);
		       return FALSE;
		   }

           if( 0 == strcmp(pStL2BridgeLink->pstrValue, AvailableInterfaceKey)) 
           {
		        found = 1;
                ST_TR064STR_LINK *pTmpLst ;
                pTmpLst = pStL2BridgeLink;
                pTmpLst = pTmpLst->pNext;
		   
                static char InterfaceType[32];
                static char InterfaceReference[256]; 
			   
			    strcpy(InterfaceType, pTmpLst->pstrValue);
			    pTmpLst = pTmpLst->pNext;
			   
			    strcpy(InterfaceReference, pTmpLst->pstrValue);
		
				int  errorinfo = 0;
                errorinfo |= OutputCharValueToAC(ac, VAR_InterfaceType, InterfaceType);
                errorinfo |= OutputCharValueToAC(ac, VAR_InterfaceReference, InterfaceReference);
				
			    if(errorinfo)
			    {
			       TR064_DestroyStrLink(pStL2BridgeLink);
				   CMM_FreeInstanceList(ppL2BrgIfLst);
			       soap_error( uclient, errorinfo);
			       return FALSE;
			    }
           } 
		   
		   TR064_DestroyStrLink(pStL2BridgeLink);

           if( 1==found )
           {
              break ;
           }
      }
     
	  CMM_FreeInstanceList(ppL2BrgIfLst);
   } 
	      
   if(found ==0)
   {
      soap_error( uclient, SOAP_NOSUCHENTRYINARRAY );
      return FALSE;
   }

   return TRUE;
}


int GetGenericAvailableInterfaceEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   int  errorinfo = 0;
   int index = 0;
   struct Param *pParams;

   pParams = findActionParamByRelatedVar(ac,VAR_AvailableInterfaceNumberOfEntries);
   if (pParams != NULL)
   {
      index = atoi(pParams->value);
   }
   else
   {
      //cmsLog_error("input pParams is NULL");
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   } 
   
   char szL2BragIfCfg[CMM_MAX_NODE_LEN]={0};
   snprintf(szL2BragIfCfg , sizeof(szL2BragIfCfg),TR064_ROOT_L2BragIfCfg"%d.",index);
   char* aszLeafName[3]= {"AvailableInterfaceKey","InterfaceType","InterfaceReference"};
   
   ST_TR064STR_LINK *pStL2BriageLst;
   pStL2BriageLst = CMM_GetStrs(szL2BragIfCfg,aszLeafName,3);
   if(pStL2BriageLst == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpLst;
   pTmpLst = pStL2BriageLst;
   
   static char AvailableInterfaceKey[32];
   static char InterfaceType[32];
   static char InterfaceReference[256];  
   
   strcpy(AvailableInterfaceKey, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;
   
   strcpy(InterfaceType, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;
   
   strcpy(InterfaceReference, pTmpLst->pstrValue);
   
   TR064_DestroyStrLink(pStL2BriageLst);

   errorinfo |= OutputCharValueToAC(ac, VAR_AvailableInterfaceKey, AvailableInterfaceKey);
   errorinfo |= OutputCharValueToAC(ac, VAR_InterfaceType, InterfaceType);
   errorinfo |= OutputCharValueToAC(ac, VAR_InterfaceReference, InterfaceReference);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo);
      return FALSE;
   }

   return TRUE;
}

