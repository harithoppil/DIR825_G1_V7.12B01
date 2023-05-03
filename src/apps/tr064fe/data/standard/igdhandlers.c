/*
 *
 * $Copyright Open Broadcom Corporation$
 *
 * $Id: igd.c,v 1.12.20.2 2003/10/31 21:31:35 mthawani Exp $
 */

#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "tr64defs.h"
#include <signal.h>

#include "cms.h"
#include "tr64utils.h"
#include "commhandle.h"
#include "cmmif.h"

int igd_config_generation = 0;

extern PDevice root_devices ;

extern DeviceTemplate LANDeviceTemplate;
extern DeviceTemplate WANDeviceTemplate;
extern DeviceTemplate IGDeviceTemplate;
extern DeviceTemplate subdevs_wandevice;

extern PServiceTemplate svcs_wanconnection;

#define TR064_CHANGE_FILE  "/var/tmp/tr064_change"

#ifdef INCLUDE_DSLLINKCONFIG
extern ServiceTemplate Template_WANDSLLinkConfig;
#endif
#ifdef INCLUDE_IPCONNECTION
extern ServiceTemplate Template_WANIPConnection;
#endif
#ifdef INCLUDE_PPPCONNECTION
extern ServiceTemplate Template_WANPPPConnection;
#endif
#ifdef INCLUDE_ATMF5LOOPBACKDIAG
extern ServiceTemplate Template_WANATMF5LoopbackDiagnostics;
#endif

#ifdef INCLUDE_LANHOSTCONFIGMGMT
extern ServiceTemplate Template_LANHostConfigManagement;
#endif
#ifdef INCLUDE_LANETHINTERFACECONFIG
extern ServiceTemplate Template_LANEthernetInterfaceConfig;
#endif
#ifdef INCLUDE_WLANCONFIG
extern ServiceTemplate Template_WLANConfig;
#endif
#ifdef INCLUDE_LANUSBINTERFACECONFIG
extern ServiceTemplate Template_LANUSBInterfaceConfig;
#endif
#ifdef INCLUDE_LANHOSTS
extern ServiceTemplate Template_LANHosts;
#endif

extern PService init_service(PServiceTemplate svctmpl, PDevice pdev, unsigned int dynsvcidx);
extern PDevice init_device(PDevice parent, PDeviceTemplate, ...);
extern int global_exit_now ;
extern int global_reboot ;

PDevice init_wan_device_and_service(PDevice parent, int type, ...);
PDevice init_lan_device_and_service(PDevice parent, int instanceOfLANDevice);

Error IGDErrors[] = {
   { SOAP_DISCONNECTINPROGRESS, "DisconnectInProgress" },
   { SOAP_INVALIDCONNECTIONTYPE, "InvalidConnectionType" },
   { SOAP_CONNECTIONALREADYTERMNATED, "ConnectionAlreadyTerminated" },
   { SOAP_CONNECTIONNOTCONFIGURED, "ConnectionNotConfigured" },
   { SOAP_SPECIFIEDARRAYINDEXINVALID, "SpecifiedArrayIndexInvalid" },
   { SOAP_NOSUCHENTRYINARRAY, "NoSuchEntryInArray" },
   { SOAP_CONFLICTINMAPPINGENTRY, "ConflictInMappingEntry" },
   { SOAP_ONLYPERMANENTLEASESSUPPORTED, "OnlyPermanentLeasesSupported" },
   { 0, NULL }
};

static ST_TR064STR_LINK* getCftTreeSvc();
static ST_TR064STR_LINK* getIGDSvc();
static int compareLink(ST_TR064STR_LINK* const pSrcLink ,  ST_TR064STR_LINK* const pDstLink);

void init_static_igd_devices(void)
{  
   /* recursively init all the devices in local memory */
   init_device(NULL, &IGDeviceTemplate);
}

static ST_TR064STR_LINK* getIGDSvc()
{
   ST_TR064STR_LINK *pStrLink = NULL;
   PDevice  pdev ;
   PService psvc ;
   forall_devices(pdev)
   {
        forall_services(pdev, psvc)
		{
		 	char* svcName = psvc->template->name;
		    if( strstr( svcName, Template_WANPPPConnection.name) != NULL || strstr(svcName, Template_WANIPConnection.name))
			{
		        TR064_AddStrToLink(&pStrLink, svcName);	
			}
	    }
   }

   return pStrLink ; 
}

static ST_TR064STR_LINK* getCftTreeSvc()
{
     ST_TR064STR_LINK *pStrLink = NULL;
     char **ppWanConnWanList = CMM_GetInstanceList(TR064_ROOT_WanConnectionDevice);
     if( ppWanConnWanList == NULL  )
     {
        return pStrLink ;
     }
     int i ;
	 char szBuffer[CMM_MAX_NODE_LEN] ;		 
#ifdef INCLUDE_PPPCONNECTION
     char **ppPPPConnWanList = ppWanConnWanList;
	 i= 0 ;
	 
     for(; ppPPPConnWanList[i]; i++)
     {
         memset(szBuffer ,0 ,sizeof(szBuffer));
		 snprintf(szBuffer ,sizeof(szBuffer) , "%sWANPPPConnectionNumberOfEntries" ,ppPPPConnWanList[i]);
		 
		 unsigned long nPPPConnNum = 0 ;
         if((CMM_GetUlong(szBuffer, &nPPPConnNum, NULL, 0) == CMM_SUCCESS) && nPPPConnNum == 1)
         {  
             int nConnWanInstanceId  = atoi(ppPPPConnWanList[i]+sizeof(TR064_ROOT_WanConnectionDevice) - 1);
        	 memset(szBuffer ,0 ,sizeof(szBuffer));
	         sprintf(szBuffer,"%s%d",Template_WANPPPConnection.name,nConnWanInstanceId);
			 TR064_AddStrToLink(&pStrLink, szBuffer);
         }
      }
#endif

#ifdef INCLUDE_IPCONNECTION
      char **ppIPConnWanList = ppWanConnWanList;
      i= 0 ;
	  for(; ppIPConnWanList[i]; i++)
	  {
		   memset(szBuffer ,0 ,sizeof(szBuffer));
		   snprintf(szBuffer ,sizeof(szBuffer) , "%sWANIPConnectionNumberOfEntries" ,ppIPConnWanList[i]);
		   unsigned long nIPConnNum = 0 ;
			 
	       if((CMM_GetUlong(szBuffer, &nIPConnNum, NULL, 0) == CMM_SUCCESS) && nIPConnNum == 1)
	       {    
	            int nConnWanInstanceId  = atoi(ppIPConnWanList[i]+sizeof(TR064_ROOT_WanConnectionDevice) - 1);
		        memset(szBuffer ,0 ,sizeof(szBuffer));
		        sprintf(szBuffer,"%s%d",Template_WANIPConnection.name,nConnWanInstanceId);
		        TR064_AddStrToLink(&pStrLink, szBuffer);
	       }
	  }  
#endif

      if(ppWanConnWanList!= NULL) 
	  {  
		 CMM_FreeInstanceList(ppWanConnWanList);
	  }

	  return pStrLink ;
}

/** Initialize IGD, LAN device, WAN device under IGD
 */
int IGDevice_Init(PDevice igdev, device_state_t state,va_list ap)
{
   PDevice subdev;
   PDevice landev;
   PDevice wandev;
   
   switch (state)
   {
	   case DEVICE_CREATE:
	      soap_register_errors(IGDErrors);
		  igdev->friendlyname = TR64_ROOT_FRIENDLY_NAME;
	      
		  char **ppLanInfoList = NULL;
	      ppLanInfoList= CMM_GetInstanceList(TR064_ROOT_LanDevice);
		  if(ppLanInfoList != NULL) 
		  {
		      int i= 0 ;
		     for(; ppLanInfoList[i]; i++)
		      {           
		         char tmpBuf[128];
		         memset(tmpBuf, 0, sizeof(tmpBuf));

				 int nLanInstanceId  = atoi(ppLanInfoList[i]+sizeof(TR064_ROOT_LanDevice) - 1); //Get the instance NO.
		         landev = init_device(igdev, &LANDeviceTemplate);
		         sprintf(tmpBuf, "LANDevice.%d", nLanInstanceId);
		         landev->friendlyname = strdup(tmpBuf);
		         landev->next = igdev->subdevs;
		         landev->instance = nLanInstanceId;
		         igdev->subdevs = landev;

		         subdev = init_lan_device_and_service(landev,landev->instance);
		      } 

			  CMM_FreeInstanceList(ppLanInfoList);
		  }

		  char **ppWanInfoList = NULL;
		  ppWanInfoList= CMM_GetInstanceList(TR064_ROOT_WanDevice);
		  if(ppWanInfoList != NULL) 
	      {  
	         int i= 0 ;
		     for(; ppLanInfoList[i]; i++)
		     {
		         char tmpBuf[128];
		         memset(tmpBuf, 0, sizeof(tmpBuf));
				 int nWanInstanceId  = atoi(ppLanInfoList[i]+sizeof(TR064_ROOT_WanDevice) - 1); //Get the instance NO.
		         wandev = init_device(igdev, &WANDeviceTemplate);
		         sprintf(tmpBuf, "WANDevice.%d", nWanInstanceId);
		         wandev->friendlyname = strdup(tmpBuf);
		         wandev->next = igdev->subdevs;
		         wandev->instance = nWanInstanceId;
		         igdev->subdevs = wandev;

		         subdev = init_wan_device_and_service(wandev,1);
		     }
	         
			 CMM_FreeInstanceList(ppWanInfoList);
	      } 

	      break;

	   case DEVICE_DESTROY:
	      break;
   }
   
   return TRUE;
}

const char *itoa(int i)
{
   static char buf[256];
   sprintf(buf, "%d", i);
   return buf;
}

static int compareLink(ST_TR064STR_LINK* const pSrcLink ,  ST_TR064STR_LINK* const pDstLink)
{
    //printf(" enter into compareLink \n");
    ST_TR064STR_LINK* pTmpSrcLink;
	ST_TR064STR_LINK* pTmpDstLink = pDstLink ;

    char* pcDstName ;
	while( pTmpDstLink != NULL )
	{
	   pcDstName = pTmpDstLink->pstrValue;
	   pTmpSrcLink = pSrcLink ;
	   while(pTmpSrcLink != NULL)
	   {
	      if(strcmp(pcDstName , pTmpSrcLink->pstrValue) == 0 )
		  {
		      break ;
		  }

		  pTmpSrcLink  = pTmpSrcLink->pNext;
	   }

	   if(pTmpSrcLink == NULL )
	   {
	      TR64FE_TRACE("can't match item is  %s" ,pcDstName);
          return 1;
	   }

	   pTmpDstLink = pTmpDstLink->pNext;
	}
   
   	return 0;
}

void setTr064IfChanged(timer_t t, void *arg)
{
    int iChange = 0 ;
	FILE *fp_dev ;
	if( access( TR064_CHANGE_FILE, F_OK ) == 0 )
    {
        fp_dev = fopen( TR064_CHANGE_FILE, "r" );
        if( fp_dev != NULL )
        {
            fscanf( fp_dev, "%d", &iChange );
            fclose( fp_dev );
        }
        unlink(TR064_CHANGE_FILE);
    }
	
	if(iChange == 0)
	{
	   return ;
	}
	
	TR64FE_TRACE("########## Begin FUNCTION  setTr064IfChanged #############\n");
	
    ST_TR064STR_LINK* pCfgLink = getCftTreeSvc();
    ST_TR064STR_LINK* pIGDLink = getIGDSvc();

    int bChange = compareLink(pCfgLink ,pIGDLink) || compareLink(pIGDLink ,pCfgLink);
	TR064_DestroyStrLink(pCfgLink);
	TR064_DestroyStrLink(pIGDLink);
	
	if( bChange )
	{
	   TR64FE_TRACE("########## setTr064IfChanged set global_exit_now is TRUE ############# \n");
	   send_advertisements(SSDP_BYEBYE);
	   send_advertisements(SSDP_BYEBYE);
       global_exit_now = TRUE;
       global_reboot = TRUE ;
	}

	TR64FE_TRACE("########## end FUNCTION  setTr064IfChanged #############\n");
}

PDevice init_lan_device_and_service(PDevice parent, int instanceOfLANDevice)
{
   PDevice pdev = parent;
   PService psvc = NULL;

   /*ADD InternetGatewayDevice.LANDevice.{i}.LANHostConfigManagement.*/
 #ifdef INCLUDE_LANHOSTCONFIGMGMT
        struct ServiceTemplate *pTemplate_LANHostCfg = NULL;
	    pTemplate_LANHostCfg= (struct ServiceTemplate *) malloc(sizeof(struct ServiceTemplate));
	    if (pTemplate_LANHostCfg == NULL) 
	    {
	       TR64FE_TRACE("malloc failed");
	    }
	    memset(pTemplate_LANHostCfg, 0, sizeof(ServiceTemplate));
	    pTemplate_LANHostCfg->name = (char *)malloc(strlen(Template_LANHostConfigManagement.name)+ strlen(itoa(1)) + 1);
	    sprintf(pTemplate_LANHostCfg->name,"%s%d",Template_LANHostConfigManagement.name, 1);
	    pTemplate_LANHostCfg->svcinit = Template_LANHostConfigManagement.svcinit;
	    pTemplate_LANHostCfg->getvars = Template_LANHostConfigManagement.getvars;
	    pTemplate_LANHostCfg->svcxml = Template_LANHostConfigManagement.svcxml;
	    pTemplate_LANHostCfg->nvariables = Template_LANHostConfigManagement.nvariables;
	    pTemplate_LANHostCfg->variables = Template_LANHostConfigManagement.variables;
	    pTemplate_LANHostCfg->actions = Template_LANHostConfigManagement.actions;
	    pTemplate_LANHostCfg->count = Template_LANHostConfigManagement.count;
	    pTemplate_LANHostCfg->serviceid = Template_LANHostConfigManagement.serviceid; 
	    pTemplate_LANHostCfg->schema = Template_LANHostConfigManagement.schema;

	    psvc = init_service(pTemplate_LANHostCfg, pdev, 1);

	    psvc->next = pdev->services;
	    pdev->services = psvc;	
 #endif
         /*ADD InternetGatewayDevice.LANDevice.{i}.LANEthernetInterfaceConfig.{i}.*/
#ifdef INCLUDE_LANETHINTERFACECONFIG
         char **ppLanEthIfCfgList = NULL;
		 ppLanEthIfCfgList= CMM_GetInstanceList(TR064_ROOT_LanEthIf);
		 if(ppLanEthIfCfgList != NULL) 
	     { 
	         int i= 0 ;
		     for(; ppLanEthIfCfgList[i]; i++)
		     {
		        int nLanEthIfInstanceId  = atoi(ppLanEthIfCfgList[i]+sizeof(TR064_ROOT_LanEthIf) - 1); //Get the instance NO.
	            struct ServiceTemplate *pTemplate_LANEthIntfCfg = NULL;
	            pTemplate_LANEthIntfCfg= (struct ServiceTemplate *) malloc(sizeof(struct ServiceTemplate));

	            if (pTemplate_LANEthIntfCfg == NULL) 
	            {
	               TR64FE_TRACE("malloc failed");
	            }
	            memset(pTemplate_LANEthIntfCfg, 0, sizeof(ServiceTemplate));

	            pTemplate_LANEthIntfCfg->name = (char *)malloc(strlen(Template_LANEthernetInterfaceConfig.name)+ strlen(itoa(nLanEthIfInstanceId)) + 1);
	            sprintf(pTemplate_LANEthIntfCfg->name,"%s%d",Template_LANEthernetInterfaceConfig.name,nLanEthIfInstanceId );
	            pTemplate_LANEthIntfCfg->svcinit = Template_LANEthernetInterfaceConfig.svcinit;
	            pTemplate_LANEthIntfCfg->getvars = Template_LANEthernetInterfaceConfig.getvars;
	            pTemplate_LANEthIntfCfg->svcxml = Template_LANEthernetInterfaceConfig.svcxml;
	            pTemplate_LANEthIntfCfg->nvariables = Template_LANEthernetInterfaceConfig.nvariables;
	            pTemplate_LANEthIntfCfg->variables = Template_LANEthernetInterfaceConfig.variables;
	            pTemplate_LANEthIntfCfg->actions = Template_LANEthernetInterfaceConfig.actions;
	            pTemplate_LANEthIntfCfg->count = Template_LANEthernetInterfaceConfig.count;
	            pTemplate_LANEthIntfCfg->serviceid = Template_LANEthernetInterfaceConfig.serviceid; 
	            pTemplate_LANEthIntfCfg->schema = Template_LANEthernetInterfaceConfig.schema;

	            psvc = init_service(pTemplate_LANEthIntfCfg, pdev, nLanEthIfInstanceId);
	            psvc->next = pdev->services;
	            pdev->services = psvc;
		     }
		 }   
#endif
         /*ADD InternetGatewayDevice.LANDevice.{i}.WLANConfiguration.{i}.*/
#ifdef INCLUDE_WLANCONFIG
#ifdef WIRELESS
         char **ppWLanCfgList = NULL;
		 ppWLanCfgList= CMM_GetInstanceList(TR064_ROOT_WLanCfg);
		 if(ppWLanCfgList != NULL) 
	     { 
	         int i= 0 ;
		     for(; ppWLanCfgList[i]; i++)
		     {
		        int nWLanCfgInstanceId  = atoi(ppWLanCfgList[i]+sizeof(TR064_ROOT_WLanCfg) - 1); //Get the instance NO.
                struct ServiceTemplate *pTemplate_LANWlan = NULL;
	            pTemplate_LANWlan= (struct ServiceTemplate *) malloc(sizeof(struct ServiceTemplate));

	            if (pTemplate_LANWlan == NULL) 
	            {
	               TR64FE_TRACE("malloc failed");
	            }
	            memset(pTemplate_LANWlan, 0, sizeof(ServiceTemplate));

	            pTemplate_LANWlan->name = (char *)malloc(strlen(Template_WLANConfig.name)+ strlen(itoa(nWLanCfgInstanceId)) + 1);
	            sprintf(pTemplate_LANWlan->name,"%s%d",Template_WLANConfig.name, nWLanCfgInstanceId);
	            pTemplate_LANWlan->svcinit = Template_WLANConfig.svcinit;
	            pTemplate_LANWlan->getvars = Template_WLANConfig.getvars;
	            pTemplate_LANWlan->svcxml = Template_WLANConfig.svcxml;
	            pTemplate_LANWlan->nvariables = Template_WLANConfig.nvariables;
	            pTemplate_LANWlan->variables = Template_WLANConfig.variables;
	            pTemplate_LANWlan->actions = Template_WLANConfig.actions;
	            pTemplate_LANWlan->count = Template_WLANConfig.count;
	            pTemplate_LANWlan->serviceid = Template_WLANConfig.serviceid; 
	            pTemplate_LANWlan->schema = Template_WLANConfig.schema;

	            psvc = init_service(pTemplate_LANWlan, pdev, nWLanCfgInstanceId);
	            psvc->next = pdev->services;
	            pdev->services = psvc;
		     }
		 }        
#endif
#endif
         /* ADD InternetGatewayDevice.LANDevice.{i}.LANUSBInterfaceConfig.{i}. */
#ifdef INCLUDE_LANUSBINTERFACECONFIG
#ifdef USB
         char **ppLanUSBCfgList = NULL;
		 ppLanUSBCfgList= CMM_GetInstanceList(TR064_ROOT_LanUSBIfCfg);
		 if(ppLanUSBCfgList != NULL) 
	     { 
	         int i= 0 ;
		     for(; ppLanUSBCfgList[i]; i++)
		     {
		        int nLanUSBIfInstanceId  = atoi(ppLanUSBCfgList[i]+sizeof(TR064_ROOT_LanUSBIfCfg) - 1); //Get the instance NO.
                struct ServiceTemplate *pTemplate_LANUSB = NULL;
	            pTemplate_LANUSB= (struct ServiceTemplate *) malloc(sizeof(struct ServiceTemplate));
	 
	            if (pTemplate_LANUSB == NULL) 
	            {
	               TR64FE_TRACE("malloc failed");
	            }
	            memset(pTemplate_LANUSB, 0, sizeof(ServiceTemplate));

	            pTemplate_LANUSB->name = (char *)malloc(strlen(Template_LANUSBInterfaceConfig.name)+ strlen(itoa(nLanUSBIfInstanceId)) + 1);
	            sprintf(pTemplate_LANUSB->name,"%s%d",Template_LANUSBInterfaceConfig.name, nLanUSBIfInstanceId);
	            pTemplate_LANUSB->svcinit = Template_LANUSBInterfaceConfig.svcinit;
	            pTemplate_LANUSB->getvars = Template_LANUSBInterfaceConfig.getvars;
	            pTemplate_LANUSB->svcxml = Template_LANUSBInterfaceConfig.svcxml;
	            pTemplate_LANUSB->nvariables = Template_LANUSBInterfaceConfig.nvariables;
	            pTemplate_LANUSB->variables = Template_LANUSBInterfaceConfig.variables;
	            pTemplate_LANUSB->actions = Template_LANUSBInterfaceConfig.actions;
	            pTemplate_LANUSB->count = Template_LANUSBInterfaceConfig.count;
	            pTemplate_LANUSB->serviceid = Template_LANUSBInterfaceConfig.serviceid; 
	            pTemplate_LANUSB->schema = Template_LANUSBInterfaceConfig.schema;

	            psvc = init_service(pTemplate_LANUSB, pdev, nLanUSBIfInstanceId);
	            psvc->next = pdev->services;
	            pdev->services = psvc;
 
			 }
		 }
#endif
#endif
         /* ADD InternetGatewayDevice.LANDevice.{i}.Hosts.*/
#ifdef INCLUDE_LANHOSTS
         struct ServiceTemplate *pTemplate_LANHosts = NULL;
         pTemplate_LANHosts= (struct ServiceTemplate *) malloc(sizeof(struct ServiceTemplate));

         if(pTemplate_LANHosts == NULL) 
         {
            TR64FE_TRACE("malloc failed");
         }
         memset(pTemplate_LANHosts, 0, sizeof(ServiceTemplate));

         pTemplate_LANHosts->name = (char *)malloc(strlen(Template_LANHosts.name)+ strlen(itoa(1)) + 1);
         sprintf(pTemplate_LANHosts->name,"%s%d",Template_LANHosts.name, 1);
         pTemplate_LANHosts->svcinit = Template_LANHosts.svcinit;
         pTemplate_LANHosts->getvars = Template_LANHosts.getvars;
         pTemplate_LANHosts->svcxml = Template_LANHosts.svcxml;
         pTemplate_LANHosts->nvariables = Template_LANHosts.nvariables;
         pTemplate_LANHosts->variables = Template_LANHosts.variables;
         pTemplate_LANHosts->actions = Template_LANHosts.actions;
         pTemplate_LANHosts->count = Template_LANHosts.count;
         pTemplate_LANHosts->serviceid = Template_LANHosts.serviceid; 
         pTemplate_LANHosts->schema = Template_LANHosts.schema;

         psvc = init_service(pTemplate_LANHosts, pdev, 1);
         psvc->next = pdev->services;
         pdev->services = psvc;
#endif
    if(pdev!=NULL)
    {
       if (ISROOT(pdev)) 
       {
          pdev->next = root_devices;
          root_devices = pdev;
       }
    }
    return pdev;
}

PDevice init_wan_device_and_service(PDevice parent, int type, ...)
{
   PFDEVINIT func;
   PDevice pdev = NULL;
   PService psvc = NULL;
   va_list ap;

   char **ppWanInfoList = NULL;
   ppWanInfoList= CMM_GetInstanceList(TR064_ROOT_WanDevice);
   if(ppWanInfoList == NULL)
   	  return pdev ;
   int i= 0 ;
   for(; ppWanInfoList[i]; i++)
   {
	  /*Here Create one New WANConnectionDevice Template and Add it to the WANDevice*/
      int nInstanceId = atoi(ppWanInfoList[i]+sizeof(TR064_ROOT_WanDevice) - 1); //Get the instance NO.
      PDeviceTemplate pdevtmpl = NULL;
      pdevtmpl = (struct DeviceTemplate *) malloc(sizeof(struct DeviceTemplate));
      memset(pdevtmpl, 0, sizeof(DeviceTemplate));
      pdevtmpl->type = (char *)malloc(strlen(subdevs_wandevice.type) + strlen(itoa(nInstanceId)) + 1 );
      sprintf(pdevtmpl->type,"%s%d",subdevs_wandevice.type, nInstanceId);

      pdevtmpl->udn = (char *)malloc(strlen(subdevs_wandevice.udn) + strlen(itoa(nInstanceId)) + 1);
      sprintf(pdevtmpl->udn,"%s%d",subdevs_wandevice.udn, nInstanceId);

      pdevtmpl->devinit = subdevs_wandevice.devinit;
      pdevtmpl->devxml = subdevs_wandevice.devxml;
      pdevtmpl->nservices = subdevs_wandevice.nservices;
      pdevtmpl->services = subdevs_wandevice.services;
      pdevtmpl->ndevices = subdevs_wandevice.ndevices;
      pdevtmpl->devicelist = subdevs_wandevice.devicelist;
      pdevtmpl->schema = subdevs_wandevice.schema;
      if (pdevtmpl->schema == NULL)
      {
         pdevtmpl->schema = TR64_DSLFORUM_SCHEMA;
      }
      pdev = (Device *) malloc(sizeof(Device));
      memset(pdev, 0, sizeof(Device));

      pdev->instance = nInstanceId;            
      /* Add 1 for \0 */
      pdev->friendlyname = (char *)malloc(strlen("WanConnectionDevice") + strlen(itoa(nInstanceId)) + 1);
      sprintf(pdev->friendlyname,"WanConnectionDevice.%d", nInstanceId);

      pdev->parent = parent;
      pdev->template = pdevtmpl;

      pdev->next = parent->subdevs;
      parent->subdevs = pdev;
      /* call the device's intialization function, if defined. */
      if ((func = pdevtmpl->devinit) != NULL) 
      {
         (*func)(pdev, DEVICE_CREATE, ap);
      }

      /*ADD WANDSLLINKCONFIG*/
#ifdef INCLUDE_DSLLINKCONFIG
      struct ServiceTemplate *pTemplate_WANDslLinkConfig= NULL;
      pTemplate_WANDslLinkConfig= (struct ServiceTemplate *) malloc(sizeof(struct ServiceTemplate));

      if (pTemplate_WANDslLinkConfig == NULL) 
      {
         UPNP_ERROR(("malloc failed in %s\n", __FUNCTION__));
      }
      memset(pTemplate_WANDslLinkConfig, 0, sizeof(ServiceTemplate));

      pTemplate_WANDslLinkConfig->name = (char *)malloc(strlen(Template_WANDSLLinkConfig.name));
      sprintf(pTemplate_WANDslLinkConfig->name,"%s",Template_WANDSLLinkConfig.name);
      pTemplate_WANDslLinkConfig->svcinit = Template_WANDSLLinkConfig.svcinit;
      pTemplate_WANDslLinkConfig->getvars = Template_WANDSLLinkConfig.getvars;
      pTemplate_WANDslLinkConfig->svcxml = Template_WANDSLLinkConfig.svcxml;
      pTemplate_WANDslLinkConfig->nvariables = Template_WANDSLLinkConfig.nvariables;
      pTemplate_WANDslLinkConfig->variables = Template_WANDSLLinkConfig.variables;
      pTemplate_WANDslLinkConfig->actions = Template_WANDSLLinkConfig.actions;
      pTemplate_WANDslLinkConfig->count = Template_WANDSLLinkConfig.count;
      pTemplate_WANDslLinkConfig->serviceid = Template_WANDSLLinkConfig.serviceid; 
      pTemplate_WANDslLinkConfig->schema = Template_WANDSLLinkConfig.schema;

      psvc = init_service(pTemplate_WANDslLinkConfig, pdev, 0);
      psvc->next = pdev->services;
      pdev->services = psvc;
#endif

      /*ADD WANIPCONNECTION or WANPPPCONNECTION service*/
#ifdef INCLUDE_PPPCONNECTION
       char **ppPPPConnWanList = NULL;
	   ppPPPConnWanList= CMM_GetInstanceList(TR064_ROOT_WanConnectionDevice);

	   if(ppPPPConnWanList!= NULL) 
	   {  
	        int i= 0 ;
			char szNodeName[CMM_MAX_NODE_LEN]={0};
		    for(; ppPPPConnWanList[i]; i++)
		    {
                 int nConnWanInstanceId  = atoi(ppPPPConnWanList[i]+sizeof(TR064_ROOT_WanConnectionDevice) - 1); //Get the instance NO.
               
				 memset(szNodeName ,0 ,sizeof(szNodeName));
				 snprintf(szNodeName ,sizeof(szNodeName) , "%sWANPPPConnectionNumberOfEntries" ,ppPPPConnWanList[i]);
				 unsigned long nPPPConnNum = 0 ;
				 
		         if((CMM_GetUlong(szNodeName, &nPPPConnNum, NULL, 0) == CMM_SUCCESS) && nPPPConnNum == 1)
		         {   
		        	 struct ServiceTemplate *pTemplate_WANPPPConnection = NULL;
			         pTemplate_WANPPPConnection= (struct ServiceTemplate *) malloc(sizeof(struct ServiceTemplate));

			         if(pTemplate_WANPPPConnection == NULL) 
			         {
			            UPNP_ERROR(("malloc failed in %s\n", __FUNCTION__));
			         }
			         memset(pTemplate_WANPPPConnection, 0, sizeof(ServiceTemplate));

			         pTemplate_WANPPPConnection->name = (char *)malloc(strlen(Template_WANPPPConnection.name));
			         sprintf(pTemplate_WANPPPConnection->name,"%s%d",Template_WANPPPConnection.name,nConnWanInstanceId);

			         pTemplate_WANPPPConnection->svcinit = Template_WANPPPConnection.svcinit;
			         pTemplate_WANPPPConnection->getvars = Template_WANPPPConnection.getvars;
			         pTemplate_WANPPPConnection->svcxml = Template_WANPPPConnection.svcxml;
			         pTemplate_WANPPPConnection->nvariables = Template_WANPPPConnection.nvariables;
			         pTemplate_WANPPPConnection->variables = Template_WANPPPConnection.variables;
			         pTemplate_WANPPPConnection->actions = Template_WANPPPConnection.actions;
			         pTemplate_WANPPPConnection->count = Template_WANPPPConnection.count;
			         pTemplate_WANPPPConnection->serviceid = Template_WANPPPConnection.serviceid; 
			         pTemplate_WANPPPConnection->schema = Template_WANPPPConnection.schema;

			         psvc = init_service(pTemplate_WANPPPConnection, pdev ,nConnWanInstanceId);
			         psvc->next = pdev->services;
			         pdev->services = psvc;  
		         }
		    }
			CMM_FreeInstanceList(ppPPPConnWanList);
		}
#endif

#ifdef INCLUDE_IPCONNECTION
      /* get the related wanIpConn obj */
      char **ppIPConnWanList = NULL;
	  ppIPConnWanList= CMM_GetInstanceList(TR064_ROOT_WanConnectionDevice);
	  if(ppIPConnWanList!= NULL) 
	  {  
	      int i= 0 ;
		  char szNodeName[CMM_MAX_NODE_LEN]={0};
		  for(; ppIPConnWanList[i]; i++)
		  {
               int nConnWanInstanceId  = atoi(ppIPConnWanList[i]+sizeof(TR064_ROOT_WanConnectionDevice) - 1); //Get the instance NO.
		       memset(szNodeName ,0 ,sizeof(szNodeName));
			   snprintf(szNodeName ,sizeof(szNodeName) , "%sWANIPConnectionNumberOfEntries" ,ppIPConnWanList[i]);
			   unsigned long nIPConnNum = 0 ;
				 
		       if((CMM_GetUlong(szNodeName, &nIPConnNum, NULL, 0) == CMM_SUCCESS) && nIPConnNum == 1)
		       { 
			         struct ServiceTemplate *pTemplate_WANIPConnection = NULL;
			         pTemplate_WANIPConnection= (struct ServiceTemplate *) malloc(sizeof(struct ServiceTemplate));

			         if(pTemplate_WANIPConnection == NULL) 
			         {
			            UPNP_ERROR(("malloc failed in %s\n", __FUNCTION__));
			         }
			         memset(pTemplate_WANIPConnection, 0, sizeof(ServiceTemplate));

			         pTemplate_WANIPConnection->name = (char *)malloc(strlen(Template_WANIPConnection.name));
			         sprintf(pTemplate_WANIPConnection->name,"%s%d",Template_WANIPConnection.name,nConnWanInstanceId);

			         pTemplate_WANIPConnection->svcinit = Template_WANIPConnection.svcinit;
			         pTemplate_WANIPConnection->getvars = Template_WANIPConnection.getvars;
			         pTemplate_WANIPConnection->svcxml = Template_WANIPConnection.svcxml;
			         pTemplate_WANIPConnection->nvariables = Template_WANIPConnection.nvariables;
			         pTemplate_WANIPConnection->variables = Template_WANIPConnection.variables;
			         pTemplate_WANIPConnection->actions = Template_WANIPConnection.actions;
			         pTemplate_WANIPConnection->count = Template_WANIPConnection.count;
			         pTemplate_WANIPConnection->serviceid = Template_WANIPConnection.serviceid; 
			         pTemplate_WANIPConnection->schema = Template_WANIPConnection.schema;

			         psvc = init_service(pTemplate_WANIPConnection, pdev ,nConnWanInstanceId);
			         psvc->next = pdev->services;
			         pdev->services = psvc;
		        }
		  }
		  
		 CMM_FreeInstanceList(ppIPConnWanList);
	  }
#endif
    }

    if(pdev!=NULL)
    {
       if (ISROOT(pdev)) 
       {
          pdev->next = root_devices;
          root_devices = pdev;
       }
    }
    return pdev;
}

int WANDevice_Init(PDevice pdev, device_state_t state, va_list ap)
{
   PWANDevicePrivateData pdata;

   switch (state) 
   {
   case DEVICE_CREATE:
      pdata = (PWANDevicePrivateData) malloc(sizeof(WANDevicePrivateData));
      /* we only have one WAN device */
      if (pdata)
      {
         strcpy(pdata->ifname, "WAN DEVICE");
         pdev->opaque = (void *) pdata;
      }
      break;
       
    case DEVICE_DESTROY:
       free(pdev->opaque);
       pdev->opaque = NULL;
       break;
    }

    return TRUE;
}


int LANDevice_Init(PDevice pdev, device_state_t state, va_list ap)
{
    PLANDevicePrivateData pdata;

    switch (state) 
    {
    case DEVICE_CREATE:
       pdata = (PLANDevicePrivateData) malloc(sizeof(LANDevicePrivateData));
       /* we only have one LAN device */
       if (pdata)
       {
          strcpy(pdata->ifname, "LAN DEVICE");
          pdev->opaque = (void *) pdata;
       }
       break;

    case DEVICE_DESTROY:
       free(pdev->opaque);
       pdev->opaque = NULL;
       break;
    }

    return TRUE;
}

/** findActionParamByRelatedVar
 *  input parameter: Action pointer, and relatedVar is the var index such as VAR_Interface.
 *  output: returns pointer to struct Param of this variable. NULL if not found.
 */
struct Param *findActionParamByRelatedVar(PAction ac, int relatedVar)
{
   struct Param *ptr;
   int index = 0;

   while ((ptr = &ac->params[index]) != NULL)
   {
      if (ptr->related == relatedVar)
      {
         return ptr;
      }
      index++;
   } /* while */
   return ((struct Param*)NULL);
}

int OutputCharValueToAC(PAction ac, int varIndex, char *value)
{
   int errorinfo = 0;
   struct Param *pParams;

   pParams = findActionParamByRelatedVar(ac,varIndex);
   if (pParams != NULL)
   {
      if(0 == strcmp(value,""))
      {
        strcpy(value,"0"); 
      }
      pParams->value = value;
   }
   else
   {
      errorinfo = SOAP_ACTIONFAILED;
   }
   return errorinfo;
}

