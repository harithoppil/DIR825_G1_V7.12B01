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
//  Filename:       lanwlancfghandlers.c
//
******************************************************************************/
#ifdef WIRELESS
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "lanwlancfgparams.h"
#include "cmmif.h"


#define GetWlanValue(szLeafName ,szValue ,ulValueLen ,VAR_Name) \
do{\
	char szFullPathName[80] ={0};\
    snprintf(szFullPathName, sizeof(szFullPathName), TR064_ROOT_WLanCfg"%d.%s" , psvc->instance,szLeafName);\
  	if(CMM_GetStr(szFullPathName, szValue,ulValueLen, NULL, 0) != CMM_SUCCESS)\
	{\
	    TR64FE_TRACE("Could not get current Wlan %s ",szLeafName);\
	    soap_error( uclient, SOAP_ACTIONFAILED );\
	    return FALSE;\
	}\
    if(strcmp(szValue ,""))\
    {\
       strcpy(szValue ,"0");\
	}\
    int errorinfo = 0;\
    errorinfo |= OutputCharValueToAC(ac, VAR_Name, szValue);\
    if(errorinfo)\
    {\
      soap_error( uclient, errorinfo );\
      return FALSE;\
    }\
	return TRUE; \
}while(0)

int GetWlanInfo(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   char* aszLeafName[11]= {"Enable" ,"Status","MaxBitRate","Channel","SSID","BeaconType",
   	                               "MACAddressControlEnabled","Standard","BSSID","BasicEncryptionModes","BasicAuthenticationMode"};

   char szPathName[80]= {0};
   snprintf(szPathName, sizeof(szPathName), TR064_ROOT_WLanCfg"%d." , psvc->instance);
   ST_TR064STR_LINK *pStWlanCfgLink;
   pStWlanCfgLink = CMM_GetStrs(szPathName,aszLeafName,11);
   if(pStWlanCfgLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpWlanCfgLink = pStWlanCfgLink ;

   static char Enable[4];
   static char Status[8];
   static char MaxBitRate[4];
   static char Channel[4];
   static char SSID[32];
   static char BeaconType[32];
   static char MACAddressControlEnabled[4];
   static char Standard[32];
   static char BSSID[32];
   static char BasicEncryptionModes[32];
   static char BasicAuthenticationMode[32];

   strcpy(Enable, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;
   
   strcpy(Status, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;

   strcpy(MaxBitRate, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;

   strcpy(Channel, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;

   strcpy(SSID, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;

   strcpy(BeaconType, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;

   strcpy(MACAddressControlEnabled, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;

   strcpy(Standard, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;

   strcpy(BSSID, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;

   strcpy(BasicEncryptionModes, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;

   strcpy(BasicAuthenticationMode, pTmpWlanCfgLink->pstrValue);

   TR064_DestroyStrLink(pStWlanCfgLink);

   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_Enable, Enable);  
   errorinfo |= OutputCharValueToAC(ac, VAR_Status, Status);
   errorinfo |= OutputCharValueToAC(ac, VAR_MaxBitRate, MaxBitRate);
   errorinfo |= OutputCharValueToAC(ac, VAR_Channel, Channel);
   errorinfo |= OutputCharValueToAC(ac, VAR_SSID, SSID);
   errorinfo |= OutputCharValueToAC(ac, VAR_BeaconType, BeaconType);
   errorinfo |= OutputCharValueToAC(ac, VAR_MACAddressControlEnabled, MACAddressControlEnabled);
   errorinfo |= OutputCharValueToAC(ac, VAR_Standard, Standard);
   errorinfo |= OutputCharValueToAC(ac, VAR_BSSID, BSSID);
   errorinfo |= OutputCharValueToAC(ac, VAR_BasicEncryptionModes, BasicEncryptionModes);
   errorinfo |= OutputCharValueToAC(ac, VAR_BasicAuthenticationMode, BasicAuthenticationMode);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}

int GetWlanStatistics(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   char* aszLeafName[4]= {"TotalBytesSent" ,"TotalBytesReceived","TotalPacketsSent","TotalPacketsReceived"};

   char  szPathName[80]= {0};
   snprintf(szPathName, sizeof(szPathName), TR064_ROOT_WLanCfg"%d." , psvc->instance);
   
   ST_TR064STR_LINK *pStWlanCfgLink;
   pStWlanCfgLink = CMM_GetStrs(szPathName,aszLeafName,4);
   if(pStWlanCfgLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpWlanCfgLink = pStWlanCfgLink ;
   static char TotalBytesSent[32];
   static char TotalBytesReceived[32];
   static char TotalPacketsSent[32];
   static char TotalPacketsReceived[32];

   strcpy(TotalBytesSent, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;
   
   strcpy(TotalBytesReceived, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;

   strcpy(TotalPacketsSent, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;

   strcpy(TotalPacketsReceived, pTmpWlanCfgLink->pstrValue);
   
   TR064_DestroyStrLink(pStWlanCfgLink);
   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_TotalBytesSent, TotalBytesSent);  
   errorinfo |= OutputCharValueToAC(ac, VAR_TotalBytesReceived, TotalBytesReceived);
   errorinfo |= OutputCharValueToAC(ac, VAR_TotalPacketsSent, TotalPacketsSent);
   errorinfo |= OutputCharValueToAC(ac, VAR_TotalPacketsReceived, TotalPacketsReceived);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}

int GetWlanByteStatistics(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   char* aszLeafName[2]= {"TotalBytesSent" ,"TotalBytesReceived"};
   
   char szPathName[80]= {0};
   snprintf(szPathName, sizeof(szPathName), TR064_ROOT_WLanCfg"%d." , psvc->instance);
   
   ST_TR064STR_LINK *pStWlanCfgLink;
   pStWlanCfgLink = CMM_GetStrs(szPathName,aszLeafName,2);
   if(pStWlanCfgLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpWlanCfgLink = pStWlanCfgLink ;
   static char TotalBytesSent[32];
   static char TotalBytesReceived[32];
  

   strcpy(TotalBytesSent, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;
   
   strcpy(TotalBytesReceived, pTmpWlanCfgLink->pstrValue);
  
   TR064_DestroyStrLink(pStWlanCfgLink);
   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_TotalBytesSent, TotalBytesSent);  
   errorinfo |= OutputCharValueToAC(ac, VAR_TotalBytesReceived, TotalBytesReceived);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}

int GetWlanPacketStatistics(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)       
{
   char* aszLeafName[2]= {"TotalPacketsSent" ,"TotalPacketsReceived"};
   
   char szPathName[80]= {0};
   snprintf(szPathName, sizeof(szPathName), TR064_ROOT_WLanCfg"%d." , psvc->instance);
   
   ST_TR064STR_LINK *pStWlanCfgLink;
   pStWlanCfgLink = CMM_GetStrs(szPathName,aszLeafName,2);
   if(pStWlanCfgLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpWlanCfgLink = pStWlanCfgLink ;
   static char TotalPacketsSent[32];
   static char TotalPacketsReceived[32];
  
   strcpy(TotalPacketsSent, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;
   
   strcpy(TotalPacketsReceived, pTmpWlanCfgLink->pstrValue);
  
   TR064_DestroyStrLink(pStWlanCfgLink);
   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_TotalPacketsSent, TotalPacketsSent);
   errorinfo |= OutputCharValueToAC(ac, VAR_TotalPacketsReceived, TotalPacketsReceived);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}

int SetEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   SetTr64Value(TR064_ROOT_WLanCfg"Enable",VAR_Enable);
}

int GetEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   static char szEnable[32]={0};
   GetWlanValue("Enable" ,szEnable ,32 ,VAR_Enable);
}

int SetSSID(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[80]= {0};
   snprintf(szNodeName ,sizeof(szNodeName),TR064_ROOT_WLanCfg"%d.SSID",psvc->instance);
   SetTr64Value(szNodeName,VAR_SSID);
}

int GetSSID(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   static char szSSID[32];
   GetWlanValue("SSID" ,szSSID ,32 ,VAR_SSID);
}

int GetBSSID(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   static char szBSSID[32];
   GetWlanValue("BSSID" ,szBSSID ,32 ,VAR_BSSID);
}

int SetChannel(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[80]= {0};
   snprintf(szNodeName ,sizeof(szNodeName),TR064_ROOT_WLanCfg"%d.Channel",psvc->instance);
   SetTr64Value(szNodeName,VAR_Channel);
}

int GetChannelInfo(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char* aszLeafName[2]= {"Channel" ,"PossibleChannels"};
   char szPathName[80]= {0};
   snprintf(szPathName, sizeof(szPathName), TR064_ROOT_WLanCfg"%d." , psvc->instance);
   
   ST_TR064STR_LINK *pStWlanChannelLink;
   pStWlanChannelLink = CMM_GetStrs(szPathName,aszLeafName,2);
   if(pStWlanChannelLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpLink = pStWlanChannelLink ;
   static char szChannel[32];
   static char szPossibleChannels[32];
  
   strcpy(szChannel, pTmpLink->pstrValue);
   pTmpLink = pTmpLink->pNext;
   
   strcpy(szPossibleChannels, pTmpLink->pstrValue);
  
   TR064_DestroyStrLink(pStWlanChannelLink);
   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_Channel, szChannel);
   errorinfo |= OutputCharValueToAC(ac, VAR_PossibleChannels, szPossibleChannels);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}

int GetBeaconType(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   static char szBeaconType[32];
   GetWlanValue("BeaconType" ,szBeaconType ,32 ,VAR_BeaconType);
}

int SetRadioMode(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[80]= {0};
   snprintf(szNodeName ,sizeof(szNodeName),TR064_ROOT_WLanCfg"%d.RadioEnabled",psvc->instance);
   SetTr64Value(szNodeName,VAR_RadioEnabled);
}

int GetRadioMode(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   static char szRadioEnable[32];
   GetWlanValue("RadioEnabled" ,szRadioEnable ,32 ,VAR_RadioEnabled);
}

int SetConfig(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szMaxBitRate[80]= {0};
   snprintf(szMaxBitRate ,sizeof(szMaxBitRate),TR064_ROOT_WLanCfg"%d.MaxBitRate" ,psvc->instance);
   
   char szChannel[80];
   snprintf(szChannel ,sizeof(szChannel),TR064_ROOT_WLanCfg"%d.Channel",psvc->instance);
   
   char szSSID[80];
   snprintf(szSSID ,sizeof(szSSID),TR064_ROOT_WLanCfg"%d.SSID",psvc->instance);
   
   char szBeaconType[80];
   snprintf(szBeaconType ,sizeof(szBeaconType),TR064_ROOT_WLanCfg"%d.BeaconType",psvc->instance);

   char szBasicEncryModes[80];
   snprintf(szBasicEncryModes ,sizeof(szBasicEncryModes),TR064_ROOT_WLanCfg"%d.BasicEncryptionModes",psvc->instance);

   char szBasicAuthMode[80];
   snprintf(szBasicAuthMode ,sizeof(szBasicAuthMode),TR064_ROOT_WLanCfg"%d.BasicAuthenticationMode",psvc->instance);
	 
   char* szLeafNames[6]={szMaxBitRate,szChannel,szSSID,szBeaconType,szBasicEncryModes,szBasicAuthMode};
   int  relatedVars[6] = {VAR_MaxBitRate,VAR_Channel, VAR_SSID,VAR_BeaconType,VAR_BasicEncryptionModes,VAR_BasicAuthenticationMode};
   SetTr64Values(szLeafNames,relatedVars ,6);
}

int SetBeaconType(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[80]= {0};
   snprintf(szNodeName ,sizeof(szNodeName),TR064_ROOT_WLanCfg"%d.BeaconType",psvc->instance);
   SetTr64Value(szNodeName,VAR_BeaconType);
}

int GetDefaultWEPKeyIndex(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   static char szDefaultWEPKeyIndex[32];
   GetWlanValue("WEPKeyIndex",szDefaultWEPKeyIndex,32,VAR_WEPKeyIndex);
}

int SetDefaultWEPKeyIndex(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szNodeName[80]= {0};
   snprintf(szNodeName ,sizeof(szNodeName),TR064_ROOT_WLanCfg"%d.WEPKeyIndex",psvc->instance);
   SetTr64Value(szNodeName,VAR_WEPKeyIndex);
}

int GetTotalAssociations(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   static char szTotalAssociations[32];
   GetWlanValue("TotalAssociations",szTotalAssociations,32,VAR_TotalAssociations);
}
              
int GetGenericAssociatedDeviceInfo(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
    //not support
    soap_error( uclient, SOAP_ACTIONFAILED);
    return FALSE;
}

int GetSpecificAssociatedDeviceInfo(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   //not support
    soap_error( uclient, SOAP_ACTIONFAILED);
    return FALSE;
}

int GetBasBeaconSecurityProperties(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char* aszLeafName[2]= {"BasicEncryptionModes" ,"BasicAuthenticationMode"};
   
   char szPathName[80]= {0};
   snprintf(szPathName, sizeof(szPathName), TR064_ROOT_WLanCfg"%d." , psvc->instance);
   
   ST_TR064STR_LINK *pStWlanCfgLink;
   pStWlanCfgLink = CMM_GetStrs(szPathName,aszLeafName,2);
   if(pStWlanCfgLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpWlanCfgLink = pStWlanCfgLink ;
   static char BasicEncryptionModes[16];
   static char BasicAuthenticationMode[24];
  
   strcpy(BasicEncryptionModes, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;
   
   strcpy(BasicAuthenticationMode, pTmpWlanCfgLink->pstrValue);
  
   TR064_DestroyStrLink(pStWlanCfgLink);
   
   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_BasicEncryptionModes, BasicEncryptionModes);
   errorinfo |= OutputCharValueToAC(ac, VAR_BasicAuthenticationMode, BasicAuthenticationMode);
   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}

int SetBasBeaconSecurityProperties(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szBasicEncryModes[80]= {0};
   snprintf(szBasicEncryModes ,sizeof(szBasicEncryModes),TR064_ROOT_WLanCfg"%d.BasicEncryptionModes" ,psvc->instance);
   
   char szBasicAuthMode[80]= {0};
   snprintf(szBasicAuthMode ,sizeof(szBasicAuthMode),TR064_ROOT_WLanCfg"%d.BasicAuthenticationMode",psvc->instance);
   
   char* szLeafNames[2]={szBasicEncryModes,szBasicAuthMode};
   int  relatedVars[2] = {VAR_BasicEncryptionModes,VAR_BasicAuthenticationMode};
   SetTr64Values(szLeafNames,relatedVars ,2);
}

int GetWPABeaconSecurityProperties(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char* aszLeafName[2]= {"WPAEncryptionModes" ,"WPAAuthenticationMode"};
   
   char szPathName[80]= {0};
   snprintf(szPathName, sizeof(szPathName), TR064_ROOT_WLanCfg"%d." , psvc->instance);
   
   ST_TR064STR_LINK *pStWlanCfgLink;
   pStWlanCfgLink = CMM_GetStrs(szPathName,aszLeafName,2);
   if(pStWlanCfgLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpWlanCfgLink = pStWlanCfgLink ;
   static char WPAEncryptionModes[32];
   static char WPAAuthenticationMode[24];  
  
   strcpy(WPAEncryptionModes, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;
   
   strcpy(WPAAuthenticationMode, pTmpWlanCfgLink->pstrValue);
  
   TR064_DestroyStrLink(pStWlanCfgLink);

   if(strcmp(WPAEncryptionModes ,"") == 0 )
   {
     strcpy(WPAEncryptionModes ,"0");
   }

   if(strcmp(WPAEncryptionModes ,"") == 0 )
   {
      strcpy(WPAEncryptionModes ,"0");
   }
   
   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_WPAEncryptionModes, WPAEncryptionModes);
   errorinfo |= OutputCharValueToAC(ac, VAR_WPAAuthenticationMode, WPAAuthenticationMode);
   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}

int SetWPABeaconSecurityProperties(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   char szWPAEncryModes[80]= {0};
   snprintf(szWPAEncryModes ,sizeof(szWPAEncryModes),TR064_ROOT_WLanCfg"%d.WPAEncryptionModes" ,psvc->instance);
   
   char szWPAAuthMode[80]= {0};
   snprintf(szWPAAuthMode ,sizeof(szWPAAuthMode),TR064_ROOT_WLanCfg"%d.WPAAuthenticationMode",psvc->instance);
   
   char* szLeafNames[2]={szWPAEncryModes,szWPAAuthMode};
   int  relatedVars[2] = {VAR_WPAEncryptionModes,VAR_WPAAuthenticationMode};
   SetTr64Values(szLeafNames,relatedVars ,2);
}

int GetSecurityKeys(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
#ifdef LGD_TODO

#endif      
   return TRUE;
}

int SetSecurityKeys(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
#ifdef LGD_TODO

#endif
   return TRUE;
}

int GetPreSharedKey(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   struct Param *pParams;
   pParams = findActionParamByRelatedVar(ac, VAR_PreSharedKeyIndex);
   if (pParams == NULL)
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

    

   char* aszLeafName[3]= {"PreSharedKey" ,"KeyPassphrase","AssociatedDeviceMACAddress"};
   char szPathName[80]= {0};
   snprintf(szPathName, sizeof(szPathName), TR064_ROOT_WLanCfg"%d.PreSharedKey.%s." , psvc->instance ,pParams->value);
   
   ST_TR064STR_LINK *pStWlanCfgLink;
   pStWlanCfgLink = CMM_GetStrs(szPathName,aszLeafName,3);
   if(pStWlanCfgLink == NULL)
   {
       soap_error( uclient, SOAP_ACTIONFAILED );
       return FALSE;
   }

   ST_TR064STR_LINK *pTmpWlanCfgLink = pStWlanCfgLink ;
  
   static char szPreSharedKey[32] ;
   static char szKeyPassphrase[32];
   static char szAssDeviceMACAddr[32];
  
   strcpy(szPreSharedKey, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;
   
   strcpy(szKeyPassphrase, pTmpWlanCfgLink->pstrValue);
   pTmpWlanCfgLink = pTmpWlanCfgLink->pNext;
   
   strcpy(szAssDeviceMACAddr, pTmpWlanCfgLink->pstrValue);
   
   TR064_DestroyStrLink(pStWlanCfgLink);

   if(strcmp(szPreSharedKey ,"") == 0 )
   {
     strcpy(szPreSharedKey ,"0");
   }

   if(strcmp(szKeyPassphrase ,"") == 0 )
   {
      strcpy(szKeyPassphrase ,"0");
   }

   if(strcmp(szAssDeviceMACAddr ,"") == 0 )
   {
      strcpy(szAssDeviceMACAddr ,"0");
   }
   
   int errorinfo = 0;
   errorinfo |= OutputCharValueToAC(ac, VAR_PreSharedKey, szPreSharedKey);
   errorinfo |= OutputCharValueToAC(ac, VAR_KeyPassphrase, szKeyPassphrase);
   errorinfo |= OutputCharValueToAC(ac, VAR_AssociatedDeviceMACAddress, szAssDeviceMACAddr);
   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
   return TRUE;
}

int SetPreSharedKey(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
#ifdef LGD_TODO
#endif
   return TRUE;
}
#endif
