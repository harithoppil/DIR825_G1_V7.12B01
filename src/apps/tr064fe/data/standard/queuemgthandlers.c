#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "queuemgtparams.h"
#include "tr64defs.h"
#include "cmmif.h"

#define GetQueueMngValue(szLeafName,szValue,valueLen) \
do{\
	char szFullPathName[60]={0};\
    snprintf(szFullPathName, sizeof(szFullPathName), TR064_ROOT_QueueMngCfg".%s" ,szLeafName);\
  	if(CMM_GetStr(szFullPathName, szValue,valueLen, NULL, 0) != CMM_SUCCESS)\
	{\
	      TR64FE_TRACE("Could not get current QueueMgt %s ",szLeafName);\
	      return FALSE;\
	}\
	return TRUE; \
}while(0)

int GetQueueMgt_Info(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   char* aszLeafName[13]= {"Enable" ,"MaxQueues","MaxClassificationEntries","MaxAppEntries",
   	        "MaxFlowEntries","MaxPolicerEntries","MaxQueueEntries","DefaultForwardingPolicy","DefaultPolicer",
   	        "DefaultQueue","DefaultDSCPMark","DefaultEthernetPriorityMark","AvailableAppList"};
   
   ST_TR064STR_LINK *pStQueueMgtLink;
   pStQueueMgtLink = CMM_GetStrs(TR064_ROOT_QueueMngCfg,aszLeafName,13);
   if(pStQueueMgtLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   static char Enable[4];
   static char MaxQueues[4];
   static char MaxClassificationEntries[4];
   static char MaxAppEntries[4];   
   static char MaxFlowEntries[4];   
   static char MaxPolicerEntries[4];   
   static char MaxQueueEntries[4];   
   static char DefaultForwardingPolicy[4];  
   static char DefaultPolicer[4];   
   static char DefaultQueue[4];   
   static char DefaultDSCPMark[4];   
   static char DefaultEthernetPriorityMark[4];   
   static char AvailableAppList[4];   

   ST_TR064STR_LINK *pTmpQueueMgtLink = pStQueueMgtLink ;

   strcpy(Enable, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(MaxQueues, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(MaxClassificationEntries, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(MaxAppEntries, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(MaxFlowEntries, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(MaxPolicerEntries, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(MaxQueueEntries, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(DefaultForwardingPolicy, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(DefaultPolicer, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(DefaultQueue, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(DefaultDSCPMark, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(DefaultEthernetPriorityMark, pTmpQueueMgtLink->pstrValue);
   pTmpQueueMgtLink = pTmpQueueMgtLink->pNext;

   strcpy(AvailableAppList, pTmpQueueMgtLink->pstrValue);
   
   TR064_DestroyStrLink(pStQueueMgtLink);
   int errorinfo = 0;

   errorinfo |= OutputCharValueToAC(ac, VAR_Enable, Enable);  
   errorinfo |= OutputCharValueToAC(ac, VAR_MaxQueues, MaxQueues);
   errorinfo |= OutputCharValueToAC(ac, VAR_MaxClassificationEntries, MaxClassificationEntries);
   errorinfo |= OutputCharValueToAC(ac, VAR_MaxAppEntries, MaxAppEntries);
   errorinfo |= OutputCharValueToAC(ac, VAR_MaxFlowEntries, MaxFlowEntries);
   errorinfo |= OutputCharValueToAC(ac, VAR_MaxPolicerEntries, MaxPolicerEntries);
   errorinfo |= OutputCharValueToAC(ac, VAR_MaxQueueEntries, MaxQueueEntries);
   errorinfo |= OutputCharValueToAC(ac, VAR_DefaultForwardingPolicy, DefaultForwardingPolicy);
   errorinfo |= OutputCharValueToAC(ac, VAR_DefaultPolicer, DefaultPolicer);
   errorinfo |= OutputCharValueToAC(ac, VAR_DefaultQueue, DefaultQueue);
   errorinfo |= OutputCharValueToAC(ac, VAR_DefaultDSCPMark, DefaultDSCPMark);
   errorinfo |= OutputCharValueToAC(ac, VAR_DefaultEthernetPriorityMark, DefaultEthernetPriorityMark);
   errorinfo |= OutputCharValueToAC(ac, VAR_AvailableAppList, AvailableAppList);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}
 
int SetQueueMGTEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   SetTr64Value(TR064_ROOT_QueueMngCfg"Enable",VAR_Enable);
}

int SetDefaultBehavior(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   char* szLeafNames[2]={TR064_ROOT_QueueMngCfg"DefaultDSCPMark",TR064_ROOT_QueueMngCfg"VAR_DefaultQueue"};
   int  relatedVars[2] = {VAR_DefaultDSCPMark, VAR_DefaultQueue} ;
   SetTr64Values(szLeafNames,relatedVars ,2) ;
}

int AddClassificationEntry( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   unsigned long ulInstanceId ;
   if(CMM_AddInstance(TR064_ROOT_QueueMngCfg"Classification.",&ulInstanceId )!= CMM_SUCCESS)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   char szClsOrder[60];
   char szClassInterface[60];
   char szDestIP[60];
   char szDestMask[60];
   char szDestIPExclude[60];
   char szSourceIP[60];
   char szSourceMask[60];
   char szSourceIPExclude[60];
   char szProtocol[60];
   char szProtocolExclude[60];
   char szDestPort[60];
   char szDestPortRangeMax[60];
   char szDestPortExclude[60];
   char szSourcePort[60];
   char szSourcePortRangeMax[60];
   char szSourcePortExclude[60];
   char szSourceMACAddress[60];
   char szSourceMACMask[60];
   char szSourceMACExclude[60];
   char szDestMACAddress[60];
   char szDestMACMask[60];
   char szDestMACExclude[60];
   char szEthertype[60];
   char szEthertypeExclude[60];
   char szSourceVendorClassID[60];
   char szSourceVendorClassIDExclude[60];
   char szSourceUserClassID[60];
   char szSourceUserClassIDExclude[60];
   char szDSCPCheck[60];
   char szDSCPExclude[60];
   char szDSCPMark[60];
   char szEthernetPriorityCheck[60];
   char szEthernetPriorityExclude[60];
   char szEthernetPriorityMark[60];
   char szVLANIDCheck[60];
   char szVLANIDExclude[60];
   char szForwardingPolicy[60];
   char szClassPolicer[60];
   char szClassQueue[60];
   char szClassApp[60];
   
   char* szLeafNames[40]={
   	szClsOrder,           szClassInterface,     szDestIP,           szDestMask,         szDestIPExclude,
    szSourceIP,           szSourceMask,         szSourceIPExclude,  szProtocol,         szProtocolExclude,
    szDestPort,           szDestPortRangeMax,   szDestPortExclude,  szSourcePort,       szSourcePortRangeMax,
    szSourcePortExclude,  szSourceMACAddress,   szSourceMACMask,    szSourceMACExclude, szDestMACAddress,     
    szDestMACMask,        szDestMACExclude,   szEthertype,   szEthertypeExclude,   szSourceVendorClassID,
    szSourceVendorClassIDExclude,szSourceUserClassID,szSourceUserClassIDExclude,szDSCPCheck,szDSCPExclude,
    szDSCPMark,szEthernetPriorityCheck,szEthernetPriorityExclude,szEthernetPriorityMark,szVLANIDCheck,
    szVLANIDExclude,szForwardingPolicy,szClassPolicer,szClassQueue,szClassApp};

   sprintf(szLeafNames[0], TR064_ROOT_QueueMngCfg".%d.ClassificationOrder",ulInstanceId);
   sprintf(szLeafNames[1], TR064_ROOT_QueueMngCfg".%d.ClassInterface",ulInstanceId);
   sprintf(szLeafNames[2], TR064_ROOT_QueueMngCfg".%d.DestIP",ulInstanceId);
   sprintf(szLeafNames[3], TR064_ROOT_QueueMngCfg".%d.DestMask",ulInstanceId);
   sprintf(szLeafNames[4], TR064_ROOT_QueueMngCfg".%d.DestIPExclude",ulInstanceId);
   
   sprintf(szLeafNames[5], TR064_ROOT_QueueMngCfg".%d.SourceIP",ulInstanceId);
   sprintf(szLeafNames[6], TR064_ROOT_QueueMngCfg".%d.SourceMask",ulInstanceId);
   sprintf(szLeafNames[7], TR064_ROOT_QueueMngCfg".%d.SourceIPExclude",ulInstanceId);
   sprintf(szLeafNames[8], TR064_ROOT_QueueMngCfg".%d.Protocol",ulInstanceId);
   sprintf(szLeafNames[9], TR064_ROOT_QueueMngCfg".%d.ProtocolExclude",ulInstanceId);
   
   sprintf(szLeafNames[10], TR064_ROOT_QueueMngCfg".%d.DestPort",ulInstanceId);
   sprintf(szLeafNames[11], TR064_ROOT_QueueMngCfg".%d.DestPortRangeMax",ulInstanceId);
   sprintf(szLeafNames[12], TR064_ROOT_QueueMngCfg".%d.DestPortExclude",ulInstanceId);
   sprintf(szLeafNames[13], TR064_ROOT_QueueMngCfg".%d.SourcePort",ulInstanceId);
   sprintf(szLeafNames[14], TR064_ROOT_QueueMngCfg".%d.SourcePortRangeMax",ulInstanceId);
   
   sprintf(szLeafNames[15], TR064_ROOT_QueueMngCfg".%d.SourcePortExclude",ulInstanceId);
   sprintf(szLeafNames[16], TR064_ROOT_QueueMngCfg".%d.SourceMACAddress",ulInstanceId);
   sprintf(szLeafNames[17], TR064_ROOT_QueueMngCfg".%d.SourceMACMask",ulInstanceId);
   sprintf(szLeafNames[18], TR064_ROOT_QueueMngCfg".%d.SourceMACExclude",ulInstanceId);
   sprintf(szLeafNames[19], TR064_ROOT_QueueMngCfg".%d.DestMACAddress",ulInstanceId);
   
   sprintf(szLeafNames[20], TR064_ROOT_QueueMngCfg".%d.DestMACMask",ulInstanceId);
   sprintf(szLeafNames[21], TR064_ROOT_QueueMngCfg".%d.DestMACExclude",ulInstanceId);
   sprintf(szLeafNames[22], TR064_ROOT_QueueMngCfg".%d.Ethertype",ulInstanceId);
   sprintf(szLeafNames[23], TR064_ROOT_QueueMngCfg".%d.EthertypeExclude",ulInstanceId);
   sprintf(szLeafNames[24], TR064_ROOT_QueueMngCfg".%d.SourceVendorClassID",ulInstanceId);
   
   
   sprintf(szLeafNames[25], TR064_ROOT_QueueMngCfg".%d.SourceVendorClassIDExclude",ulInstanceId);
   sprintf(szLeafNames[26], TR064_ROOT_QueueMngCfg".%d.SourceUserClassID",ulInstanceId);
   sprintf(szLeafNames[27], TR064_ROOT_QueueMngCfg".%d.SourceUserClassIDExclude",ulInstanceId);
   sprintf(szLeafNames[28], TR064_ROOT_QueueMngCfg".%d.DSCPCheck",ulInstanceId);
   sprintf(szLeafNames[29], TR064_ROOT_QueueMngCfg".%d.DSCPExclude",ulInstanceId);
   
   sprintf(szLeafNames[30], TR064_ROOT_QueueMngCfg".%d.DSCPMark",ulInstanceId);
   sprintf(szLeafNames[31], TR064_ROOT_QueueMngCfg".%d.EthernetPriorityCheck",ulInstanceId);
   sprintf(szLeafNames[32], TR064_ROOT_QueueMngCfg".%d.EthernetPriorityExclude",ulInstanceId);
   sprintf(szLeafNames[33], TR064_ROOT_QueueMngCfg".%d.EthernetPriorityMark",ulInstanceId);
   sprintf(szLeafNames[34], TR064_ROOT_QueueMngCfg".%d.VLANIDCheck",ulInstanceId);
   
   sprintf(szLeafNames[35], TR064_ROOT_QueueMngCfg".%d.VLANIDExclude",ulInstanceId);
   sprintf(szLeafNames[36], TR064_ROOT_QueueMngCfg".%d.ForwardingPolicy",ulInstanceId);
   sprintf(szLeafNames[37], TR064_ROOT_QueueMngCfg".%d.ClassPolicer",ulInstanceId);
   sprintf(szLeafNames[38], TR064_ROOT_QueueMngCfg".%d.ClassQueue",ulInstanceId);
   sprintf(szLeafNames[39], TR064_ROOT_QueueMngCfg".%d.ClassApp",ulInstanceId);
   
   int relatedVars[40]=
   	{VAR_ClassificationOrder, VAR_ClassInterface,VAR_DestIP,VAR_DestMask,VAR_DestIPExclude,
   	VAR_SourceIP,VAR_SourceMask,VAR_SourceIPExclude,VAR_Protocol, VAR_ProtocolExclude,
   	VAR_DestPort,VAR_DestPortRangeMax,VAR_DestPortExclude,VAR_SourcePort,VAR_SourcePortRangeMax,
   	VAR_SourcePortExclude,VAR_SourceMACAddress,VAR_SourceMACMask,VAR_SourceMACExclude,VAR_DestMACAddress,
    VAR_DestMACMask,VAR_DestMACExclude,VAR_Ethertype,VAR_EthertypeExclude,VAR_SourceVendorClassID,
    VAR_SourceVendorClassIDExclude,VAR_SourceUserClassID,VAR_SourceUserClassIDExclude,VAR_DSCPCheck, VAR_DSCPExclude,
    VAR_DSCPMark,VAR_EthernetPriorityCheck,VAR_EthernetPriorityExclude,VAR_EthernetPriorityMark,VAR_VLANIDCheck,
    VAR_VLANIDExclude,VAR_ForwardingPolicy,VAR_ClassPolicer,VAR_ClassQueue,VAR_ClassApp};
   
   SetTr64Values(szLeafNames,relatedVars ,40);
}

int DeleteClassificationEntry( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   char ClassificationKey[32]; 

   pParams = findActionParamByRelatedVar(ac,VAR_ClassificationKey);
   if (pParams != NULL)
   {
      strcpy(ClassificationKey, pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }  

   char **ppClsEntryLst = NULL;
   ppClsEntryLst= CMM_GetInstanceList(TR064_ROOT_QueueMngCfg);
   if(ppClsEntryLst != NULL) 
   {  
      char szClassificationKey[32];  
	  char szNodeName[60]={0};
      int i= 0 ;
      for(; ppClsEntryLst[i]; i++)
      {
           memset(szNodeName ,0 ,sizeof(szNodeName) );
		   snprintf(szNodeName,sizeof(szNodeName),"%s.ClassificationKey",ppClsEntryLst[i]);
           if(CMM_GetStr(szNodeName, szClassificationKey ,sizeof(szClassificationKey), NULL, 0) != CMM_SUCCESS)
		   {
		          soap_error( uclient, SOAP_ACTIONFAILED );
				  CMM_FreeInstanceList(ppClsEntryLst);
			      TR64FE_TRACE("Could not get current ClassificationKey");
			      return FALSE;
		   }
          
           if( 0 == strcmp( szClassificationKey, ClassificationKey))           
           {
			    if(CMM_DelInstance(ppClsEntryLst[i]) == CMM_SUCCESS)
			    { 
					  CMM_FreeInstanceList(ppClsEntryLst);
		              return TRUE;   
			    }
                TR64FE_TRACE("failed to DeleteClassificationEntry");
				break ;
           } 
      }
	  CMM_FreeInstanceList(ppClsEntryLst);
   }

   soap_error( uclient, SOAP_ACTIONFAILED );
   return FALSE;
}

static int FillClsEntry(char* pcFullPath,PAction ac)
{
    char* szLeafNames[42]={ 
		"ClassificationKey","ClassificationEnable","ClassificationOrder", "ClassInterface","DestIP","DestMask",
		"DestIPExclude","SourceIP","SourceMask","SourceIPExclude","Protocol",
		"ProtocolExclude","DestPort","DestPortRangeMax","DestPortExclude","SourcePort",
		"SourcePortRangeMax","SourcePortExclude","SourceMACAddress","SourceMACMask","SourceMACExclude",
		"DestMACAddress","DestMACMask","DestMACExclude","Ethertype","EthertypeExclude",
		"SourceVendorClassID","SourceVendorClassIDExclude","SourceUserClassID","SourceUserClassIDExclude","DSCPCheck",
        "DSCPExclude","DSCPMark","EthernetPriorityCheck","EthernetPriorityExclude","EthernetPriorityMark",
		"ClassQueue","VLANIDCheck","VLANIDExclude","ForwardingPolicy", "ClassPolicer","ClassApp"};

    ST_TR064STR_LINK *pStQueueMgtClsLink;
    pStQueueMgtClsLink = CMM_GetStrs(pcFullPath,szLeafNames,42);
    if(pStQueueMgtClsLink == NULL)
    {
       TR64FE_TRACE("CMM_GetStrs is NULL " );
       return FALSE;
    }

    static char ClassificationKey[32]; 
	static char ClsEnable[4];
    static char ClsOrder[4];
    static char ClassInterface[256];
    static char DestIP[24];
    static char DestMask[24];
	
    static char DestIPExclude[4];
    static char SourceIP[24];
    static char SourceMask[24];
    static char SourceIPExclude[4];
    static char Protocol[24];
	
    static char ProtocolExclude[4];
    static char DestPort[24];
    static char DestPortRangeMax[24];
    static char DestPortExclude[4];
    static char SourcePort[24];
	
    static char SourcePortRangeMax[24];
    static char SourcePortExclude[4];
    static char SourceMACAddress[32];
    static char SourceMACMask[32];
    static char SourceMACExclude[4];
	
    static char DestMACAddress[32];
    static char DestMACMask[32];
    static char DestMACExclude[4];
    static char Ethertype[32];
    static char EthertypeExclude[4];
	
    static char SourceVendorClassID[256];
    static char SourceVendorClassIDExclude[4];
    static char SourceUserClassID[256];
    static char SourceUserClassIDExclude[4];
    static char DSCPCheck[24];
		
    static char DSCPExclude[4];
    static char DSCPMark[24];
    static char EthernetPriorityCheck[24];
    static char EthernetPriorityExclude[4];
    static char EthernetPriorityMark[24];
	
    static char ClassQueue[24]; 
    static char VLANIDCheck[32];
    static char VLANIDExclude[4];
    static char ForwardingPolicy[32];
    static char ClassPolicer[32];
    static char ClassApp[32];

	ST_TR064STR_LINK *pTmpLst ;
    pTmpLst = pStQueueMgtClsLink;

    strcpy(ClassificationKey, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
	
    strcpy(ClsEnable, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
   
    strcpy(ClsOrder, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(ClassInterface, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(DestIP, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
   
    strcpy(DestMask, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
	
    strcpy(DestIPExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(SourceIP, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(SourceMask, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
   
    strcpy(SourceIPExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
	
	strcpy(Protocol, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
	
	strcpy(ProtocolExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
   
    strcpy(DestPort, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(DestPortRangeMax, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(DestPortExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
   
    strcpy(SourcePort, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
	
    strcpy(SourcePortRangeMax, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(SourcePortExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
   
    strcpy(SourceMACAddress, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(SourceMACMask, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(SourceMACExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
	
	strcpy(DestMACAddress, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(DestMACMask, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(DestMACExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
   
    strcpy(Ethertype, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(EthertypeExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
	
	strcpy(SourceVendorClassID, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
   
    strcpy(SourceVendorClassIDExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(SourceUserClassID, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(SourceUserClassIDExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
   
    strcpy(DSCPCheck, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
		
	strcpy(DSCPExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

    strcpy(DSCPMark, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
   
    strcpy(EthernetPriorityCheck, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(EthernetPriorityExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(EthernetPriorityMark, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
	
	strcpy(ClassQueue, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(VLANIDCheck, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
	
	strcpy(VLANIDExclude, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;

	strcpy(ForwardingPolicy, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
	
	strcpy(ClassPolicer, pTmpLst->pstrValue);
    pTmpLst = pTmpLst->pNext;
	
	strcpy(ClassApp, pTmpLst->pstrValue);

	TR064_DestroyStrLink(pStQueueMgtClsLink);

    int errorinfo = 0;
	errorinfo |= OutputCharValueToAC(ac, VAR_ClassificationKey, ClassificationKey);
    errorinfo |= OutputCharValueToAC(ac, VAR_ClassificationEnable, ClsEnable);
    errorinfo |= OutputCharValueToAC(ac, VAR_ClassificationOrder, ClsOrder);
    errorinfo |= OutputCharValueToAC(ac, VAR_ClassInterface, ClassInterface);
    errorinfo |= OutputCharValueToAC(ac, VAR_DestIP, DestIP);
    errorinfo |= OutputCharValueToAC(ac, VAR_DestMask, DestMask);
    errorinfo |= OutputCharValueToAC(ac, VAR_DestIPExclude, DestIPExclude);
    errorinfo |= OutputCharValueToAC(ac, VAR_SourceIP, SourceIP);
    errorinfo |= OutputCharValueToAC(ac, VAR_SourceMask, SourceMask);
    errorinfo |= OutputCharValueToAC(ac, VAR_SourceIPExclude, SourceIPExclude);
    errorinfo |= OutputCharValueToAC(ac, VAR_Protocol, Protocol);
    errorinfo |= OutputCharValueToAC(ac, VAR_ProtocolExclude, ProtocolExclude);
    errorinfo |= OutputCharValueToAC(ac, VAR_DestPort, DestPort);   
    errorinfo |= OutputCharValueToAC(ac, VAR_DestPortRangeMax, DestPortRangeMax);   
    errorinfo |= OutputCharValueToAC(ac, VAR_DestPortExclude, DestPortExclude);   
    errorinfo |= OutputCharValueToAC(ac, VAR_SourcePort, SourcePort);   
    errorinfo |= OutputCharValueToAC(ac, VAR_SourcePortRangeMax, SourcePortRangeMax);   
    errorinfo |= OutputCharValueToAC(ac, VAR_SourcePortExclude, SourcePortExclude); 

    errorinfo |= OutputCharValueToAC(ac, VAR_SourceMACAddress, SourceMACAddress); 
    errorinfo |= OutputCharValueToAC(ac, VAR_SourceMACMask, SourceMACMask); 
    errorinfo |= OutputCharValueToAC(ac, VAR_SourceMACExclude, SourceMACExclude); 
    errorinfo |= OutputCharValueToAC(ac, VAR_DestMACAddress, DestMACAddress); 
    errorinfo |= OutputCharValueToAC(ac, VAR_DestMACMask, DestMACMask); 
    errorinfo |= OutputCharValueToAC(ac, VAR_DestMACExclude, DestMACExclude); 

    errorinfo |= OutputCharValueToAC(ac, VAR_Ethertype, Ethertype); 
    errorinfo |= OutputCharValueToAC(ac, VAR_EthertypeExclude, EthertypeExclude);  

    errorinfo |= OutputCharValueToAC(ac, VAR_SourceVendorClassID, SourceVendorClassID); 
    errorinfo |= OutputCharValueToAC(ac, VAR_SourceVendorClassIDExclude, SourceVendorClassIDExclude); 

    errorinfo |= OutputCharValueToAC(ac, VAR_SourceUserClassID, SourceUserClassID); 
    errorinfo |= OutputCharValueToAC(ac, VAR_SourceUserClassIDExclude, SourceUserClassIDExclude); 

    errorinfo |= OutputCharValueToAC(ac, VAR_DSCPCheck, DSCPCheck);      
    errorinfo |= OutputCharValueToAC(ac, VAR_DSCPExclude, DSCPExclude);      
    errorinfo |= OutputCharValueToAC(ac, VAR_DSCPMark, DSCPMark);      
    errorinfo |= OutputCharValueToAC(ac, VAR_EthernetPriorityCheck, EthernetPriorityCheck);      
    errorinfo |= OutputCharValueToAC(ac, VAR_EthernetPriorityExclude, EthernetPriorityExclude);      
    errorinfo |= OutputCharValueToAC(ac, VAR_EthernetPriorityMark, EthernetPriorityMark);      
    errorinfo |= OutputCharValueToAC(ac, VAR_ClassQueue, ClassQueue);      
   
    errorinfo |= OutputCharValueToAC(ac, VAR_VLANIDCheck, VLANIDCheck); 
    errorinfo |= OutputCharValueToAC(ac, VAR_VLANIDExclude, VLANIDExclude); 
    errorinfo |= OutputCharValueToAC(ac, VAR_ForwardingPolicy, ForwardingPolicy); 
    errorinfo |= OutputCharValueToAC(ac, VAR_ClassPolicer, ClassPolicer); 
    errorinfo |= OutputCharValueToAC(ac, VAR_ClassApp, ClassApp); 

	if(errorinfo)
    { 
       return FALSE;
    }
	
	return TRUE ; 
}
      
int GetSpecificClassificationEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
    struct Param *pParams;
    char ClassificationKey[32]; 

    pParams = findActionParamByRelatedVar(ac,VAR_ClassificationKey);
    if (pParams != NULL)
    {
       strcpy(ClassificationKey, pParams->value);
    }
    else
    {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
    }  

	char **ppClsEntryLst = NULL;
    ppClsEntryLst= CMM_GetInstanceList(TR064_ROOT_QueueMngCfg"Classification.");
    if(ppClsEntryLst != NULL) 
    {  
      char szGetClsKey[32];  
	  char szNodeName[60]={0};
      int i= 0 ;
      for(; ppClsEntryLst[i]; i++)
      {
           printf( "######  %s  \n "  , ppClsEntryLst[i] ) ;    
           memset(szNodeName ,0 ,sizeof(szNodeName) );
		   snprintf(szNodeName,sizeof(szNodeName),"%s.ClassificationKey",ppClsEntryLst[i]);
           if(CMM_GetStr(szNodeName, szGetClsKey ,sizeof(szGetClsKey), NULL, 0) != CMM_SUCCESS)
		   {
		          soap_error( uclient, SOAP_ACTIONFAILED );
				  CMM_FreeInstanceList(ppClsEntryLst);
			      TR64FE_TRACE("Could not get current Specific Classification Entry ClassificationKey.");
			      return FALSE;
		   }
          
           if( 0 == strcmp( szGetClsKey, ClassificationKey))           
           {
              if( FillClsEntry(ppClsEntryLst[i] ,ac))
              {   
                  CMM_FreeInstanceList(ppClsEntryLst);
                  return TRUE ;
              }
			  TR64FE_TRACE("failed to GetSpecificClassificationEntry");
			  break ;
           } 
      }
     
	  CMM_FreeInstanceList(ppClsEntryLst);
   } 
	      
   soap_error( uclient, SOAP_NOSUCHENTRYINARRAY );
   return FALSE;
}
     
int GetGenericClassificationEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   uint32 index = 0;
   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac,VAR_ClassificationNumberOfEntries);
   if (pParams != NULL)
   {
      index = atoi(pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   } 

   char szClsFullPath[60] = {0};
   snprintf(szClsFullPath,sizeof(szClsFullPath) ,TR064_ROOT_QueueMngCfg"Classification.%d",index);
   
   if( FillClsEntry(szClsFullPath ,ac))
   {   
      return TRUE ;
   }
}

int SetClassificationEntryEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
    struct Param *pParams;
    char ClassificationKey[32]; 

    pParams = findActionParamByRelatedVar(ac,VAR_ClassificationKey);
    if (pParams != NULL)
    {
       strcpy(ClassificationKey, pParams->value);
    }
    else
    {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
    }  

	char **ppClsEntryLst = NULL;
    ppClsEntryLst= CMM_GetInstanceList(TR064_ROOT_QueueMngCfg"Classification.");
    if(ppClsEntryLst != NULL) 
    {  
      char szGetClsKey[32];  
	  char szNodeName[60]={0};
      int i= 0 ;
      for(; ppClsEntryLst[i]; i++)
      {
           memset(szNodeName ,0 ,sizeof(szNodeName));
		   snprintf(szNodeName,sizeof(szNodeName),"%s.ClassificationKey",ppClsEntryLst[i]);
           if(CMM_GetStr(szNodeName, szGetClsKey ,sizeof(szGetClsKey), NULL, 0) != CMM_SUCCESS)
		   {
		          soap_error( uclient, SOAP_ACTIONFAILED );
				  CMM_FreeInstanceList(ppClsEntryLst);
			      TR64FE_TRACE("Could not get current Specific Classification Entry ClassificationKey.");
			      return FALSE;
		   }
          
           if( 0 == strcmp( szGetClsKey, ClassificationKey))           
           {
              char szNodeName[60] = {0} ;
			  snprintf(szNodeName ,sizeof(szNodeName),"%s.ClassificationEnable",ppClsEntryLst[i]);
			  CMM_FreeInstanceList(ppClsEntryLst);
              SetTr64Value(szNodeName,VAR_ClassificationEnable); 
           } 
      }
     
	  CMM_FreeInstanceList(ppClsEntryLst);
   } 
	      
   soap_error( uclient, SOAP_NOSUCHENTRYINARRAY );
   return FALSE;
}
      
int SetClassificationEntryOrder(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
    struct Param *pParams;
    char ClassificationKey[32]; 

    pParams = findActionParamByRelatedVar(ac,VAR_ClassificationKey);
    if (pParams != NULL)
    {
       strcpy(ClassificationKey, pParams->value);
    }
    else
    {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
    }  

	char **ppClsEntryLst = NULL;
    ppClsEntryLst= CMM_GetInstanceList(TR064_ROOT_QueueMngCfg"Classification.");
    if(ppClsEntryLst != NULL) 
    {  
      char szGetClsKey[32];  
	  char szNodeName[60]={0};
      int i= 0 ;
      for(; ppClsEntryLst[i]; i++)
      {
           memset(szNodeName ,0 ,sizeof(szNodeName));
		   snprintf(szNodeName,sizeof(szNodeName),"%s.ClassificationKey",ppClsEntryLst[i]);
           if(CMM_GetStr(szNodeName, szGetClsKey ,sizeof(szGetClsKey), NULL, 0) != CMM_SUCCESS)
		   {
		        soap_error( uclient, SOAP_ACTIONFAILED );
				CMM_FreeInstanceList(ppClsEntryLst);
			    TR64FE_TRACE("Could not get current Specific Classification Entry ClassificationKey.");
			    return FALSE;
		   }
          
           if( 0 == strcmp( szGetClsKey, ClassificationKey))           
           {
               char szNodeName[60] = {0} ;
			   snprintf(szNodeName ,sizeof(szNodeName),"%s.ClassificationEnable",ppClsEntryLst[i]);
			   CMM_FreeInstanceList(ppClsEntryLst);
               SetTr64Value(szNodeName,VAR_ClassificationOrder); 
           } 
      }
     
	  CMM_FreeInstanceList(ppClsEntryLst);
   } 
	      
   soap_error( uclient, SOAP_NOSUCHENTRYINARRAY );
   return FALSE;
}
      
int AddQueueEntry( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   unsigned long ulInstanceId ;
   if(CMM_AddInstance(TR064_ROOT_QueueMngCfg"Queue.",&ulInstanceId )!= CMM_SUCCESS)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   char QueueEnable[4];
   char QueueInterface[256];
   char QueuePrecedence[8];
   char QueueKey[8];
   
   char* szLeafNames[4]={QueueEnable,QueueInterface, QueuePrecedence ,QueueKey};

   sprintf(szLeafNames[0], TR064_ROOT_QueueMngCfg"Queue.%d",ulInstanceId);
   sprintf(szLeafNames[1], TR064_ROOT_QueueMngCfg"Queue.%d",ulInstanceId);
   sprintf(szLeafNames[2], TR064_ROOT_QueueMngCfg"Queue.%d",ulInstanceId);
   sprintf(szLeafNames[3], TR064_ROOT_QueueMngCfg"Queue.%d",ulInstanceId);
   int relatedVars[4] = {VAR_QueueEnable,VAR_QueueInterface,VAR_QueuePrecedence,VAR_QueueKey};
   SetTr64Values(szLeafNames,relatedVars ,4);
}

int DeleteQueueEntry( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   char QueueKey[32]; 

   pParams = findActionParamByRelatedVar(ac,VAR_QueueKey);
   if (pParams != NULL)
   {
      strcpy(QueueKey, pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }    

   char **ppQueueEntryLst = NULL;
   ppQueueEntryLst= CMM_GetInstanceList(TR064_ROOT_QueueMngCfg"Queue.");
   if(ppQueueEntryLst != NULL) 
   {  
      char szQueueKey[32];  
	  char szNodeName[60]={0};
      int i= 0 ;
      for(; ppQueueEntryLst[i]; i++)
      {
           memset(szNodeName ,0 ,sizeof(szNodeName) );
		   snprintf(szNodeName,sizeof(szNodeName),"%s.QueueKey",ppQueueEntryLst[i]);
           if(CMM_GetStr(szNodeName, szQueueKey ,sizeof(szQueueKey), NULL, 0) != CMM_SUCCESS)
		   {
		          soap_error( uclient, SOAP_ACTIONFAILED );
				  CMM_FreeInstanceList(ppQueueEntryLst);
			      TR64FE_TRACE("Could not get current QueueKey");
			      return FALSE;
		   }
          
           if( 0 == strcmp( szQueueKey, QueueKey))           
           {
			    if(CMM_DelInstance(ppQueueEntryLst[i]) == CMM_SUCCESS)
			    { 
					  CMM_FreeInstanceList(ppQueueEntryLst);
		              return TRUE;  
			    }
				TR64FE_TRACE("failed to DeleteQueueEntry");
				break ;
           } 
      }
	  CMM_FreeInstanceList(ppQueueEntryLst);
   }

   soap_error( uclient, SOAP_ACTIONFAILED );
   return FALSE; 
}

int SetQueueEntryEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
    struct Param *pParams;
     char QueueKey[32]; 

    pParams = findActionParamByRelatedVar(ac,VAR_QueueKey);
    if (pParams != NULL)
    {
       strcpy(QueueKey, pParams->value);
    }
    else
    {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
    }     

	char **ppQueueEntryLst = NULL;
    ppQueueEntryLst= CMM_GetInstanceList(TR064_ROOT_QueueMngCfg"Queue.");
    if(ppQueueEntryLst != NULL) 
    {  
      char szGetQueueKey[32];  
	  char szNodeName[60]={0};
      int i= 0 ;
      for(; ppQueueEntryLst[i]; i++)
      {
           memset(szNodeName ,0 ,sizeof(szNodeName));
		   snprintf(szNodeName,sizeof(szNodeName),"%s.QueueKey",ppQueueEntryLst[i]);
           if(CMM_GetStr(szNodeName, szGetQueueKey ,sizeof(szGetQueueKey), NULL, 0) != CMM_SUCCESS)
		   {
		          soap_error( uclient, SOAP_ACTIONFAILED );
				  CMM_FreeInstanceList(ppQueueEntryLst);
			      TR64FE_TRACE("Could not get current Specific Classification Entry ClassificationKey.");
			      return FALSE;
		   }
          
           if( 0 == strcmp( QueueKey, szGetQueueKey))           
           {
              char szNodeName[60] = {0} ;
			  snprintf(szNodeName ,sizeof(szNodeName),"%s.QueueEnable",ppQueueEntryLst[i]);
			  CMM_FreeInstanceList(ppQueueEntryLst);
              SetTr64Value(szNodeName,VAR_ClassificationEnable); 
           } 
      }
     
	  CMM_FreeInstanceList(ppQueueEntryLst);
   }
}

int GetSpecificQueueEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   char QueueKey[8];
   Param *pParams = findActionParamByRelatedVar(ac,VAR_QueueKey);
   if (pParams != NULL)
   {
      strcpy(QueueKey, pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }     

   char* aszLeafName[5]= {"QueueKey","QueueEnable","QueueInterface","QueueStatus","QueuePrecedence"};

   char **ppQueueEntryLst = NULL;
   ppQueueEntryLst= CMM_GetInstanceList(TR064_ROOT_QueueMngCfg"Queue.");
   if(ppQueueEntryLst != NULL) 
   {  
      char szGetQueueKey[32];  
	  char szNodeName[60]={0};
      int i= 0 ;
      for(; ppQueueEntryLst[i]; i++)
      {
           ST_TR064STR_LINK *pStQueueEntryLst;
		   pStQueueEntryLst = CMM_GetStrs(ppQueueEntryLst[i],aszLeafName,5);
		   if(pStQueueEntryLst == NULL)
		   {
		       soap_error( uclient, SOAP_ACTIONFAILED );
			   CMM_FreeInstanceList(ppQueueEntryLst);
		       return FALSE;
		   }
          
           if( 0 == strcmp( QueueKey, pStQueueEntryLst->pstrValue))           
           {
                ST_TR064STR_LINK *pTmpLst ;
                pTmpLst = pStQueueEntryLst;
				pTmpLst = pTmpLst->pNext;

				static char QueueEnable[4];
				static char QueueInterface[256];
				static char QueueStatus[16];
				static char QueuePrecedence[8];

			    strcpy(QueueEnable, pTmpLst->pstrValue);
			    pTmpLst = pTmpLst->pNext;
			   
			    strcpy(QueueInterface, pTmpLst->pstrValue);
			    pTmpLst = pTmpLst->pNext;

				strcpy(QueueStatus, pTmpLst->pstrValue);
			    pTmpLst = pTmpLst->pNext;
			   
			    strcpy(QueuePrecedence, pTmpLst->pstrValue);

				TR064_DestroyStrLink(pStQueueEntryLst);

				int errorinfo = 0;
                errorinfo |= OutputCharValueToAC(ac, VAR_QueueEnable, QueueEnable);
				errorinfo |= OutputCharValueToAC(ac, VAR_QueueStatus, QueueStatus);
				errorinfo |= OutputCharValueToAC(ac, VAR_QueueInterface, QueueInterface);
				errorinfo |= OutputCharValueToAC(ac, VAR_QueuePrecedence, QueuePrecedence);
			    if(!errorinfo)
			    {  
				   CMM_FreeInstanceList(ppQueueEntryLst);
			       return TRUE;
			    }
				
                TR64FE_TRACE("failed to GetSpecificQueueEntry ") ;
				break ;	
           }else
           {
              TR064_DestroyStrLink(pStQueueEntryLst);
           }
      }
     
	  CMM_FreeInstanceList(ppQueueEntryLst);
   } 
	      
   soap_error( uclient, SOAP_NOSUCHENTRYINARRAY );
   return FALSE;
}

int GetGenericQueueEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   int index = 0;
   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac,VAR_QueueNumberOfEntries);
   if (pParams != NULL)
   {
      index = atoi(pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   } 

   char szQueueNodeName[60] = {0};
   snprintf(szQueueNodeName ,sizeof(szQueueNodeName) ,TR064_ROOT_QueueMngCfg"Queue.%d",index);

   char* aszLeafName[13]= {"QueueKey","QueueEnable","QueueInterface","QueueStatus","QueuePrecedence" ,
   	                      "QueueBufferLength","QueueWeight","REDThreshold","REDPercentage",
   	                      "DropAlgorithm","SchedulerAlgorithm","ShapingRate","ShapingBurstSize"};

   ST_TR064STR_LINK *pStQueueEntryLink;
   pStQueueEntryLink = CMM_GetStrs(szQueueNodeName,aszLeafName,13);
   if(pStQueueEntryLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpLst ;
   pTmpLst = pStQueueEntryLink;
  
   static char QueueKey[8];
   static char QueueEnable[4];
   static char QueueInterface[256];
   static char QueueStatus[16];
   static char QueuePrecedence[8];
   
   static char QueueBufferLength[16];
   static char QueueWeight[16];
   static char REDThreshold[16];
   static char REDPercentage[16];

   static char DropAlgorithm[16];
   static char SchedulerAlgorithm[16];
   static char ShapingRate[16];
   static char ShapingBurstSize[16];

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(QueueKey, "0");
   }else
      strcpy(QueueKey, pTmpLst->pstrValue); 
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(QueueEnable, "0");
   }else
      strcpy(QueueEnable, pTmpLst->pstrValue); 
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(QueueInterface, "0");  
   }else
      strcpy(QueueInterface, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(QueueStatus, "0");
   }else
      strcpy(QueueStatus, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(QueuePrecedence, "0");
   }else
      strcpy(QueuePrecedence, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(QueueBufferLength, "0");
   }else
      strcpy(QueueBufferLength, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(QueueWeight, "0");
   }else
      strcpy(QueueWeight, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(REDThreshold, "0");
   }else
      strcpy(REDThreshold, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(REDPercentage, "0");
   }else
      strcpy(REDPercentage, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(DropAlgorithm, "0");
   }else
      strcpy(DropAlgorithm, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(SchedulerAlgorithm, "0");
   }else
      strcpy(SchedulerAlgorithm, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(ShapingRate, "0");
   }else
      strcpy(ShapingRate, pTmpLst->pstrValue);
   pTmpLst = pTmpLst->pNext;

   if( 0 == strcmp(pTmpLst->pstrValue ,""))
   {
      strcpy(ShapingBurstSize, "0");
   }else
     strcpy(ShapingBurstSize, pTmpLst->pstrValue);
   
   TR064_DestroyStrLink(pStQueueEntryLink);

   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_QueueKey, QueueKey); 
   errorinfo |= OutputCharValueToAC(ac, VAR_QueueEnable, QueueEnable);
   errorinfo |= OutputCharValueToAC(ac, VAR_QueueStatus, QueueStatus);
   errorinfo |= OutputCharValueToAC(ac, VAR_QueueInterface, QueueInterface);
   errorinfo |= OutputCharValueToAC(ac, VAR_QueuePrecedence, QueuePrecedence);

   errorinfo |= OutputCharValueToAC(ac, VAR_QueueBufferLength, QueueBufferLength); 
   errorinfo |= OutputCharValueToAC(ac, VAR_QueueWeight, QueueWeight);
   errorinfo |= OutputCharValueToAC(ac, VAR_REDThreshold, REDThreshold);
   errorinfo |= OutputCharValueToAC(ac, VAR_REDPercentage, REDPercentage);

   errorinfo |= OutputCharValueToAC(ac, VAR_DropAlgorithm, DropAlgorithm); 
   errorinfo |= OutputCharValueToAC(ac, VAR_SchedulerAlgorithm, SchedulerAlgorithm);
   errorinfo |= OutputCharValueToAC(ac, VAR_ShapingRate, ShapingRate);
   errorinfo |= OutputCharValueToAC(ac, VAR_ShapingBurstSize, ShapingBurstSize);
   
   if(errorinfo)
   { 
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   return TRUE;
}
