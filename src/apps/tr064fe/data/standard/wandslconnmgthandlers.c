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
//  Filename:       wandslconnmgthandlers.c
//
******************************************************************************/
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"

#include "wandslconnmgtparams.h"



extern PDevice add_device_and_service_to_wan(PDevice parent, int type, int wanObjIndex, ...);
extern PDevice g_wandev;

int GetWANConnectionServiceNumberOfEntries(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   #if 0  //not surpport yujinshi

   InstanceIdStack WANDevice_iidStack = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack WANConnectionDevice_iidStack = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack WANPPPConnection_iidStack = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack WANIPConnection_iidStack = EMPTY_INSTANCE_ID_STACK;

   WanDevObject *wanDevObj=NULL;
   WanConnDeviceObject *wanconObj = NULL;   
   WanIpConnObject  *ipConnObj = NULL;
   WanPppConnObject  *pppConnObj = NULL;

   int errorinfo = 0;
   int cnt = 0;
   static char ServiceNumberOfEntries[32];

   while (cmsObj_getNext(MDMOID_WAN_DEV, &WANDevice_iidStack, (void **)&wanDevObj) == CMSRET_SUCCESS)
   {
      INIT_INSTANCE_ID_STACK(&WANConnectionDevice_iidStack);
      while (cmsObj_getNextInSubTree(MDMOID_WAN_CONN_DEVICE, &WANDevice_iidStack, &WANConnectionDevice_iidStack,(void **)&wanconObj) == CMSRET_SUCCESS)
      {
         //Check the PPP
         INIT_INSTANCE_ID_STACK(&WANPPPConnection_iidStack);
         while (cmsObj_getNextInSubTree(MDMOID_WAN_PPP_CONN, &WANConnectionDevice_iidStack, &WANPPPConnection_iidStack,(void **)&pppConnObj) == CMSRET_SUCCESS)
         {
            cnt++;
            cmsObj_free((void **) &pppConnObj);   
         }
         //Check the IP  
         INIT_INSTANCE_ID_STACK(&WANIPConnection_iidStack);
         while (cmsObj_getNextInSubTree(MDMOID_WAN_IP_CONN, &WANConnectionDevice_iidStack, &WANIPConnection_iidStack,(void **)&ipConnObj) == CMSRET_SUCCESS)
         {
            cnt++;
            cmsObj_free((void **) &ipConnObj);   
         }  
     
         cmsObj_free((void **) &wanconObj);   
      }
      cmsObj_free((void **) &wanDevObj);         
   }

   sprintf(ServiceNumberOfEntries, "%d", cnt);

   errorinfo |= OutputCharValueToAC(ac, VAR_WANConnectionServiceNumberOfEntries, ServiceNumberOfEntries);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   }

   #endif
   
   return TRUE;
}

int GetSpecificConnectionServiceEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   #if 0  //not surpport yujinshi

   MdmPathDescriptor pathDesc;
   CmsRet ret = CMSRET_SUCCESS;
 
   WanIpConnObject  *ipConnObj = NULL;
   WanPppConnObject  *pppConnObj = NULL;

   int  errorinfo = 0;   
   char WANConnectionService[256];   
   static char WANConnectionDevice[256];   
   static char wanName[32];
   
   struct Param *pParams;

   pParams = findActionParamByRelatedVar(ac,VAR_WANConnectionService);
   if (pParams != NULL)
   {
      sprintf(WANConnectionService,"%s.", pParams->value);
      if ((ret = cmsMdm_fullPathToPathDescriptor(WANConnectionService, &pathDesc)) != CMSRET_SUCCESS)
      {
         //cmsLog_error("cmsMdm_fullPathToPathDescriptor returns error. ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }
   
   if( pathDesc.oid == MDMOID_WAN_PPP_CONN )
   {
      if ((ret = cmsObj_get(pathDesc.oid, &pathDesc.iidStack, 0, (void **)&pppConnObj)) == CMSRET_SUCCESS)
      {
         if(pppConnObj->name)
         {
            sprintf(WANConnectionDevice,  "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d",
                                              pathDesc.iidStack.instance[0], pathDesc.iidStack.instance[1]);
            
            strcpy(wanName, pppConnObj->name);
         }
         cmsObj_free((void **)&pppConnObj);
      }
   }
   else if( pathDesc.oid == MDMOID_WAN_IP_CONN )
   {
      if ((ret = cmsObj_get(pathDesc.oid, &pathDesc.iidStack, 0, (void **)&ipConnObj)) == CMSRET_SUCCESS)
      {
         if(ipConnObj->name)
         {
            sprintf(WANConnectionDevice,  "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d",
                                              pathDesc.iidStack.instance[0], pathDesc.iidStack.instance[1]);
            strcpy(wanName, ipConnObj->name);
         }
         cmsObj_free((void **)&ipConnObj);
      }
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }   

   errorinfo |= OutputCharValueToAC(ac, VAR_WANConnectionDevice, WANConnectionDevice);
   errorinfo |= OutputCharValueToAC(ac, VAR_Name, wanName);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   }

   #endif 

   return TRUE;

}

int GetGenericConnectionServiceEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   #if 0  //not surpport yujinshi


   InstanceIdStack WANDevice_iidStack = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack WANConnectionDevice_iidStack = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack WANPPPConnection_iidStack = EMPTY_INSTANCE_ID_STACK;
   InstanceIdStack WANIPConnection_iidStack = EMPTY_INSTANCE_ID_STACK;

   WanDevObject *wanDevObj=NULL;
   WanConnDeviceObject *wanconObj = NULL;   
   WanIpConnObject  *ipConnObj = NULL;
   WanPppConnObject  *pppConnObj = NULL;

   int found = 0;
   int index = 0;
   int cnt = 0;
   int errorinfo = 0;   
   static char WANConnectionService[256];   
   static char WANConnectionDevice[256];   
   static char wanName[32];
   
   struct Param *pParams;

   memset(WANConnectionService, 0, sizeof(WANConnectionService));
   memset(WANConnectionDevice, 0, sizeof(WANConnectionDevice));
   memset(wanName, 0, sizeof(wanName));

   pParams = findActionParamByRelatedVar(ac,VAR_WANConnectionServiceNumberOfEntries);
   if (pParams != NULL)
   {
      index = atoi(pParams->value);
   }
   else
   {
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   while (cmsObj_getNext(MDMOID_WAN_DEV, &WANDevice_iidStack, (void **)&wanDevObj) == CMSRET_SUCCESS)
   {
      INIT_INSTANCE_ID_STACK(&WANConnectionDevice_iidStack);
      while (cmsObj_getNextInSubTree(MDMOID_WAN_CONN_DEVICE, &WANDevice_iidStack, &WANConnectionDevice_iidStack,(void **)&wanconObj) == CMSRET_SUCCESS)
      {
         //Check the PPP
         INIT_INSTANCE_ID_STACK(&WANPPPConnection_iidStack);
         while (cmsObj_getNextInSubTree(MDMOID_WAN_PPP_CONN, &WANConnectionDevice_iidStack, &WANPPPConnection_iidStack,(void **)&pppConnObj) == CMSRET_SUCCESS)
         {
            cnt++;
            if( cnt == index )
            {
               found = 1;
               sprintf(WANConnectionService, "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d.WANPPPConnection.%d",
                                              WANDevice_iidStack.instance[WANDevice_iidStack.currentDepth-1],
                                              WANConnectionDevice_iidStack.instance[WANConnectionDevice_iidStack.currentDepth-1],
                                              WANPPPConnection_iidStack.instance[WANPPPConnection_iidStack.currentDepth-1]);
               sprintf(WANConnectionDevice,  "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d",
                                              WANDevice_iidStack.instance[WANDevice_iidStack.currentDepth-1],
                                              WANConnectionDevice_iidStack.instance[WANConnectionDevice_iidStack.currentDepth-1]);
               if(pppConnObj->name)
               {
                  strcpy(wanName, pppConnObj->name);
               }
            }
            cmsObj_free((void **) &pppConnObj);   
         }
         //Check the IP  
         INIT_INSTANCE_ID_STACK(&WANIPConnection_iidStack);
         while (cmsObj_getNextInSubTree(MDMOID_WAN_IP_CONN, &WANConnectionDevice_iidStack, &WANIPConnection_iidStack,(void **)&ipConnObj) == CMSRET_SUCCESS)
         {
            cnt++;
            if( cnt == index )
            {
               found = 1;
               sprintf(WANConnectionService, "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d.WANIPConnection.%d",
                                              WANDevice_iidStack.instance[WANDevice_iidStack.currentDepth-1],
                                              WANConnectionDevice_iidStack.instance[WANConnectionDevice_iidStack.currentDepth-1],
                                              WANIPConnection_iidStack.instance[WANIPConnection_iidStack.currentDepth-1]);

               sprintf(WANConnectionDevice,  "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d",
                                              WANDevice_iidStack.instance[WANDevice_iidStack.currentDepth-1],
                                              WANConnectionDevice_iidStack.instance[WANConnectionDevice_iidStack.currentDepth-1]);
               if(ipConnObj->name)
               {
                  strcpy(wanName, ipConnObj->name);
               }
            }
            cmsObj_free((void **) &ipConnObj);   
         }  
     
         cmsObj_free((void **) &wanconObj);   
      }
      cmsObj_free((void **) &wanDevObj);         
   }

   if(found == 0)
   {
      soap_error( uclient, SOAP_SPECIFIEDARRAYINDEXINVALID );
      return FALSE;
   }

   errorinfo |= OutputCharValueToAC(ac, VAR_WANConnectionService, WANConnectionService);
   errorinfo |= OutputCharValueToAC(ac, VAR_WANConnectionDevice, WANConnectionDevice);
   errorinfo |= OutputCharValueToAC(ac, VAR_Name, wanName);

   if(errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   }
   #endif

   return TRUE;
}

int AddConnectionDeviceAndService(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   WanDevObject *wanDev = NULL;
   WanDslLinkCfgObject *dslLinkCfg = NULL;

   WanIpConnObject  *ipConnObj = NULL;
   WanPppConnObject  *pppConnObj = NULL;

   CmsRet ret = CMSRET_SUCCESS;

   int  WANDevice_Index = 0;
   struct Param *pParams;
   int errorinfo = 0;

   char LinkType[32];
   char wanName[32];
   char ConnectionType[32];
   char DestinationAddress[32];
   static char WANConnectionDevice[256];   
   static char WANConnectionService[256];   

   WANDevice_Index = psvc->device->instance;

   pParams = findActionParamByRelatedVar(ac,VAR_Name);
   if (pParams != NULL)
   {
       strcpy(wanName, pParams->value);
   }
   else
   {
       errorinfo |= SOAP_ACTIONFAILED;
   }

   pParams = findActionParamByRelatedVar(ac,VAR_LinkType);
   if (pParams != NULL)
   {
       strcpy(LinkType, pParams->value);
   }
   else
   {
       errorinfo |= SOAP_ACTIONFAILED;
   }

   pParams = findActionParamByRelatedVar(ac,VAR_ConnectionType);
   if (pParams != NULL)
   {
       strcpy(ConnectionType, pParams->value);
   }
   else
   {
       errorinfo |= SOAP_ACTIONFAILED;
   }

   if (errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   }

   pParams = findActionParamByRelatedVar(ac,VAR_DestinationAddress);
   if (pParams != NULL)
   {
       strcpy(DestinationAddress, pParams->value);
   }
   else
   {
       errorinfo |= SOAP_ACTIONFAILED;
   }

   if (errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   }

   while (cmsObj_getNext(MDMOID_WAN_DEV, &iidStack, (void **) &wanDev) == CMSRET_SUCCESS)
   {
      if(iidStack.instance[iidStack.currentDepth-1] == WANDevice_Index)
      {
         cmsObj_free((void **) &wanDev);
         break;
      }
      cmsObj_free((void **) &wanDev);
   }      

   /* add new instance of WanConnectionDevice */
   if ((ret = cmsObj_addInstance(MDMOID_WAN_CONN_DEVICE, &iidStack)) != CMSRET_SUCCESS)
   {
      //cmsLog_error("could not create new WanConnectionDevice, ret=%d", ret);
      soap_error( uclient, SOAP_ACTIONFAILED );
      return FALSE;
   }

   /* get the instance of dslLinkConfig in the newly created
    * WanConnectionDevice sub-tree */
   if ((ret = cmsObj_get(MDMOID_WAN_DSL_LINK_CFG, &iidStack, 0, (void **) &dslLinkCfg)) != CMSRET_SUCCESS)
   {
      //cmsLog_error("Failed to get dslLinkCfgObj, ret=%d", ret);
      return ret;
   }

   cmsMem_free(dslLinkCfg->destinationAddress);
   dslLinkCfg->destinationAddress = cmsMem_strdup(DestinationAddress); 
   cmsMem_free(dslLinkCfg->linkType);
   dslLinkCfg->linkType = cmsMem_strdup(LinkType);
   
   /* set and activate WanDslLinkCfgObject */
   ret = cmsObj_set(dslLinkCfg, &iidStack);
   cmsObj_free((void **) &dslLinkCfg);
      
   if( (0==strcmp(LinkType, "EoA(RFC2684B)"))
       || (0==strcmp(LinkType, "IPoA(RFC2684R)")))   
   {
      /* add new instance of WanIpConnDevice */
      if ((ret = cmsObj_addInstance(MDMOID_WAN_IP_CONN, &iidStack)) != CMSRET_SUCCESS)
      {
         //cmsLog_error("could not create new MDMOID_WAN_IP_CONN, ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      } 
   
      if ((ret = cmsObj_get(MDMOID_WAN_IP_CONN, &iidStack, 0, (void **) &ipConnObj)) != CMSRET_SUCCESS)
      {
         //cmsLog_error("Failed to get ipConnObj, ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }   

      cmsMem_free(ipConnObj->name);
      ipConnObj->name = cmsMem_strdup(wanName);

      cmsMem_free(ipConnObj->connectionType);
      ipConnObj->connectionType = cmsMem_strdup(ConnectionType);

      ret = cmsObj_set(ipConnObj, &iidStack);
      cmsObj_free((void **) &ipConnObj);

      if ( ret!= CMSRET_SUCCESS )
      {
         //cmsLog_error("Failed to cmsObj_set MDMOID_WAN_IP_CONN, ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }       
      else
      {
         sprintf(WANConnectionService, "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d.WANIPConnection.%d",
                                              iidStack.instance[0],
                                              iidStack.instance[1],
                                              iidStack.instance[2]);

         sprintf(WANConnectionDevice,  "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d",
                                              iidStack.instance[0],
                                              iidStack.instance[1]);          
      }
   }
   else if( (0==strcmp(LinkType, "PPPoA"))
            || (0==strcmp(LinkType, "PPPoE"))) 
   {
      /* add new instance of WanIpConnDevice */
      if ((ret = cmsObj_addInstance(MDMOID_WAN_PPP_CONN, &iidStack)) != CMSRET_SUCCESS)
      {
         //cmsLog_error("could not create new MDMOID_WAN_PPP_CONN, ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      } 
   
      if ((ret = cmsObj_get(MDMOID_WAN_PPP_CONN, &iidStack, 0, (void **) &pppConnObj)) != CMSRET_SUCCESS)
      {
         //cmsLog_error("Failed to get ipConnObj, ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }   

      cmsMem_free(pppConnObj->name);
      pppConnObj->name = cmsMem_strdup(wanName);
      cmsMem_free(pppConnObj->connectionType);
      pppConnObj->connectionType = cmsMem_strdup(ConnectionType);

      ret = cmsObj_set(pppConnObj, &iidStack);
      cmsObj_free((void **) &pppConnObj);

      if ( ret!= CMSRET_SUCCESS )
      {
         //cmsLog_error("Failed to cmsObj_set MDMOID_WAN_PPP_CONN, ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }       
      else
      {
         sprintf(WANConnectionService, "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d.WANPPPConnection.%d",
                                              iidStack.instance[0],
                                              iidStack.instance[1],
                                              iidStack.instance[2]);

         sprintf(WANConnectionDevice,  "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d",
                                              iidStack.instance[0],
                                              iidStack.instance[1]); 
      }
   } 

   errorinfo |= OutputCharValueToAC(ac, VAR_WANConnectionDevice, WANConnectionDevice);
   errorinfo |= OutputCharValueToAC(ac, VAR_WANConnectionService, WANConnectionService);

   if (errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   }

   cmsMgm_saveConfigToFlash();
   return TRUE; 
}

int AddConnectionService(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
#ifdef LGD_TODO
   InstanceIdStack iidStack = EMPTY_INSTANCE_ID_STACK;
   WanDevObject *wanDev = NULL;
   MdmPathDescriptor pathDesc; 
   CmsRet ret = CMSRET_SUCCESS;

   struct Param *pParams;
   int errorinfo = 0;

   char wanName[32];
   char ConnectionType[32];
   char WANConnectionDevice[256];   
   static char WANConnectionService[256];   

   pParams = findActionParamByRelatedVar(ac,VAR_Name);
   if (pParams != NULL)
   {
       strcpy(wanName, pParams->value);
   }
   else
   {
       errorinfo |= SOAP_ACTIONFAILED;
   }

   pParams = findActionParamByRelatedVar(ac,VAR_WANConnectionDevice);
   if (pParams != NULL)
   {
       strcpy(WANConnectionDevice, pParams->value);
      if ((ret = cmsMdm_fullPathToPathDescriptor(WANConnectionDevice, &pathDesc)) != CMSRET_SUCCESS)
      {
         //cmsLog_error("cmsMdm_fullPathToPathDescriptor returns error. ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }
   }
   else
   {
       errorinfo |= SOAP_ACTIONFAILED;
   }

   pParams = findActionParamByRelatedVar(ac,VAR_ConnectionType);
   if (pParams != NULL)
   {
       strcpy(ConnectionType, pParams->value);
   }
   else
   {
       errorinfo |= SOAP_ACTIONFAILED;
   }

   uint32 WanConnIndexOld = 0;
   sscanf(WANConnectionDevice, "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.%d",&WanConnIndexOld );

   errorinfo |= OutputCharValueToAC(ac, VAR_WANConnectionService, WANConnectionService);

   if (errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   }
#endif
   return TRUE; 
}

int DeleteConnectionService(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
   MdmPathDescriptor pathDesc; 
   CmsRet ret = CMSRET_SUCCESS;

   int errorinfo = 0;
   struct Param *pParams;
   char WANConnectionService[256];   

   pParams = findActionParamByRelatedVar(ac,VAR_WANConnectionService);
   if (pParams != NULL)
   {
      sprintf(WANConnectionService, "%s.", pParams->value);
      if ((ret = cmsMdm_fullPathToPathDescriptor(WANConnectionService, &pathDesc)) != CMSRET_SUCCESS)
      {
         //cmsLog_error("cmsMdm_fullPathToPathDescriptor returns error. ret=%d", ret);
         soap_error( uclient, SOAP_ACTIONFAILED );
         return FALSE;
      }
   }
   else
   {
      errorinfo |= SOAP_ACTIONFAILED;
      soap_error( uclient, errorinfo );
      return FALSE;
   }

   if ((ret = cmsObj_deleteInstance(pathDesc.oid, &pathDesc.iidStack)) != CMSRET_SUCCESS)
   {
       //cmsLog_error("Failed to delete WANConnectionService Object, ret = %d", ret);
       errorinfo = SOAP_ACTIONFAILED;
   }

   if (errorinfo)
   {
      soap_error( uclient, errorinfo );
      return FALSE;
   }
   cmsMgm_saveConfigToFlash();
   return TRUE; 
}


