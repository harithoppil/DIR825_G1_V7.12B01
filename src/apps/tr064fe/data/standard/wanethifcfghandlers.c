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
//  Filename:       wanethifcfghandler.c
//
******************************************************************************/
#ifdef INCLUDE_WANETHERNETCONFIG
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "wanethifcfgparams.h"
#include "tr64defs.h"
#include "bcmcfm.h"
#include "syscall.h"


int WANETHInterfaceConfig_GetVar(struct Service *psvc, int varindex)
{
   void *info;
   uint32 index = 1;
	
   struct StateVar *var;
   var = &(psvc->vars[varindex]);

   int    Enable = 0;
   char   Status[16];
   char   MACAddress[24];
   char   MaxBitRate[8];
   if ( BcmCfm_objGet(BCMCFM_OBJ_IFC_ETHERNET, &info, &index) == BcmCfm_Ok ) 
   {
      PBcmCfm_EthIfcCfg_t pEth = (PBcmCfm_EthIfcCfg_t)info;
	  
      //Enable
      if ( pEth->status == BcmCfm_CfgEnabled )
      {
         Enable = 1;
      }
      else
      {
         Enable = 0;
      }
	  
      //MACAddress
      strcpy(MACAddress, writeMac(pEth->mac));
      BcmCfm_objFree(BCMCFM_OBJ_IFC_ETHERNET, info);
   }
   
   if ( BcmCfm_stsGet(BCMCFM_OBJ_IFC_ETHERNET, &info, &index) == BcmCfm_Ok ) 
   {
      PBcmCfm_EthIfcSts_t pEth = (PBcmCfm_EthIfcSts_t)info;
	  
      //Status
      if ( pEth->linkState == BcmCfm_LinkUp )
      {
         strcpy(Status, "Up");
      }
      else
      {
         strcpy(Status, "Disabled");
      }
	  
      //MaxBitRate
      if(pEth->autoNegState== BcmCfm_CfgEnabled)
      {
         strcpy(MaxBitRate, "Auto");
      }
      else
      {
         if(pEth->speed == BcmCfm_EthSpeed100)
             strcpy(MaxBitRate, "100");
         else if(pEth->speed == BcmCfm_EthSpeed10)
             strcpy(MaxBitRate, "10");
         else
            strcpy(MaxBitRate, "Auto");
      }
   
      BcmCfm_stsFree(BCMCFM_OBJ_IFC_ETHERNET, info);
   }		        
    switch (varindex) 
	{
	    case VAR_Enable:			
			sprintf(var->value, "%u", Enable);
		break;

	    case VAR_Status:
			
			strcpy(var->value, Status);
		break;

		case VAR_MACAddress:
			strcpy(var->value, MACAddress);
		break;

		case VAR_MaxBitRate:
			strcpy(var->value, MaxBitRate);
		break;
						
    }

    return TRUE;
}


int SetETHInterfaceEnable(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)

{
    uint32 index = 1;
    void *info;
    uint32 Enable ;
    struct Param *pParams;
    pParams = findActionParamByRelatedVar(ac,VAR_Enable);
    if (pParams != NULL)
    {
       if(strlen(pParams->value)==0)
       {
          soap_error( uclient, SOAP_ACTIONFAILED );
          return FALSE;
       }
	    Enable = atoi(pParams->value);
    }
    else
    {
	    soap_error( uclient, SOAP_ACTIONFAILED );
	    return FALSE;
    }

    if ( BcmCfm_objGet(BCMCFM_OBJ_IFC_ETHERNET, &info, &index) == BcmCfm_Ok ) 
    {
      PBcmCfm_EthIfcCfg_t pEth = (PBcmCfm_EthIfcCfg_t)info;
      //Enable
      if(Enable)
	  	 pEth->status = BcmCfm_CfgEnabled;
	  else
	  	pEth->status = BcmCfm_CfgDisabled;

      BcmCfm_objSet(BCMCFM_OBJ_IFC_ETHERNET, info, index);
    }
 
    BcmPsi_flush();
    return TRUE;

}

int SetMaxBitRate(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)

{
    uint32 index = 1;
    void *info;
    char MaxBitRate[8] ;
    struct Param *pParams;
    pParams = findActionParamByRelatedVar(ac,VAR_MaxBitRate);
    if (pParams != NULL)
    {
       if(strlen(pParams->value)==0)
       {
          soap_error( uclient, SOAP_ACTIONFAILED );
          return FALSE;
       }
	    strcpy(MaxBitRate, pParams->value);
    }
    else
    {
	    soap_error( uclient, SOAP_ACTIONFAILED );
	    return FALSE;
    }

    if ( BcmCfm_objGet(BCMCFM_OBJ_IFC_ETHERNET, &info, &index) == BcmCfm_Ok ) 
    {
      PBcmCfm_EthIfcCfg_t pEth = (PBcmCfm_EthIfcCfg_t)info;
      if ( strcmp(MaxBitRate, "Auto") == 0 )
         pEth->autoNeg = BcmCfm_CfgEnabled;
	  else 
	  {
		 pEth->autoNeg = BcmCfm_CfgDisabled;
		 if ( strcmp(MaxBitRate, "100") == 0 )
		    pEth->speed = BcmCfm_EthSpeed100;
		 else if( strcmp(MaxBitRate, "10") == 0 )
			pEth->speed = BcmCfm_EthSpeed10;
	 	 else
			pEth->autoNeg = BcmCfm_CfgEnabled;
	  }
      BcmCfm_objSet(BCMCFM_OBJ_IFC_ETHERNET, info, index);
    }
    BcmPsi_flush(); 
    return TRUE;

}
 
int getLANDeviceLANInterfaceConfigStatsTR64(char *value, uint32 index, BcmCfm_Stats statsType)
{
	void *obj;
	if ( BcmCfm_stsGet(BCMCFM_OBJ_NTWK_INTF, &obj, &index) == BcmCfm_Ok ) 
   {
		PBcmCfm_NtwkIntfSts_t pObj = (PBcmCfm_NtwkIntfSts_t)obj;
		if ( pObj != NULL ) 
      {
			char buf[64];
			uint32 *statsPtr = NULL;
			switch (statsType) 
         {
				case BcmCfm_StatsTxBytes:
					statsPtr =  &(pObj->txBytes);
					break;
				case BcmCfm_StatsRxBytes:
					statsPtr =  &(pObj->rxBytes);
					break;
				case BcmCfm_StatsTxPkts:
					statsPtr =  &(pObj->txPkts);
					break;
				case BcmCfm_StatsRxPkts:
					statsPtr =  &(pObj->rxPkts);
					break;
				default:
					break;
			}																			
			snprintf(buf, sizeof(buf), "%d", *statsPtr);
         strcpy(value, buf);
			BcmCfm_stsFree(BCMCFM_OBJ_NTWK_INTF, obj);
			return 0;
		}
	}
	return -1;	
}

int GetStatisticsWANETH(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)

{
   int errorinfo = 0;

   static char BytesSent[32];
   static char BytesReceived[32];
   static char PacketsSent[32];
   static char PacketsReceived[32];
   
   getLANDeviceLANInterfaceConfigStatsTR64(BytesSent, IFC_ENET_ID, BcmCfm_StatsTxBytes);                    
   getLANDeviceLANInterfaceConfigStatsTR64(BytesReceived, IFC_ENET_ID, BcmCfm_StatsRxBytes);                    
   getLANDeviceLANInterfaceConfigStatsTR64(PacketsSent, IFC_ENET_ID, BcmCfm_StatsTxPkts);                    
   getLANDeviceLANInterfaceConfigStatsTR64(PacketsReceived, IFC_ENET_ID, BcmCfm_StatsRxPkts);                    
   
   errorinfo |= OutputCharValueToAC(ac, VAR_BytesSent, BytesSent);
   errorinfo |= OutputCharValueToAC(ac, VAR_BytesReceived, BytesReceived);
   errorinfo |= OutputCharValueToAC(ac, VAR_PacketsSent, PacketsSent);
   errorinfo |= OutputCharValueToAC(ac, VAR_PacketsReceived, PacketsReceived);
   
   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   } 
	return TRUE;

}
#endif

