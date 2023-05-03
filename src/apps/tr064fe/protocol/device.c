/*
    $Copyright Open Broadcom Corporation$   
*/

#include <stdarg.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>

#include "upnp_osl.h"
#include "upnp_dbg.h"
#include "upnp.h"
#include "tr64defs.h"
#include "device.h"
#include "cms.h"
#include "tr64defs.h"
#include "cmmif.h"

extern void uuidstr_create(char *str, int len);

PDevice root_devices = NULL;
extern DeviceTemplate subdevs_wandevice;
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
/** init_device():
 *  Recursive routine to initialize a device, and all its subdevices.
 *  Also all services within a device are intialized by calling their
 *  specific initialization function, if available.  
 */
PDevice init_device(PDevice parent, PDeviceTemplate pdevtmpl,...)
{
    static int indent = 0;
    int i;
    PFDEVINIT func;
    PDevice pdev, subdev;
    PService psvc;
    va_list ap;

    UPNP_TRACE(("%*sInitializing %sdevice \"%s\".\r\n", indent, "", (parent ? "" : "root "), pdevtmpl->type));
    if (pdevtmpl->schema == NULL)
    {
       pdevtmpl->schema = TR64_DSLFORUM_SCHEMA;
    }
	
    pdev = (Device *) malloc(sizeof(Device));
    memset(pdev, 0, sizeof(Device));
    pdev->parent = parent;
    pdev->template = pdevtmpl;

    /* call the device's intialization function, if defined. */
    if ((func = pdevtmpl->devinit) != NULL) 
    {
       va_start( ap, pdevtmpl);
       (*func)(pdev, DEVICE_CREATE,ap);
       va_end( ap);
    }

    /** we do a top down, depth-first traversal of the device heirarchy.
     * sub-devices will be initialized before we complete initialization of the root device.
     */
    for (i = 0; i < pdevtmpl->ndevices; i++)
    {
       indent += 4;
       subdev = init_device(pdev, &(pdevtmpl->devicelist[i]));
       indent -= 4;
       subdev->next = pdev->subdevs;
       pdev->subdevs = subdev;
    }

    /* Initialize each service in this device. */
    for (i = 0; i < pdevtmpl->nservices; i++) 
    {
       psvc = init_service(pdevtmpl->services[i], pdev, 0);
       psvc->next = pdev->services;
       pdev->services = psvc;
    }

    if (ISROOT(pdev)) 
    {
       pdev->next = root_devices;
       root_devices = pdev;
    }
	
    return pdev;
}

PDevice add_device_and_service_to_wan(PDevice parent, int type, int wanObjIndex, ...)
{
   PDevice pdev = NULL;

#ifdef LGD_TODO
   PFDEVINIT func;
   PService psvc = NULL;
   va_list ap;
   void *objValue;

   if ( BcmCfm_objGet(BCMCFM_OBJ_NTWK_INTF, &objValue, &wanObjIndex) == BcmCfm_Ok ) 
   {
      PBcmCfm_NtwkIntfCfg_t pObj = (PBcmCfm_NtwkIntfCfg_t)objValue;

      if ( pObj != NULL && pObj->attachToObjId == BCMCFM_OBJ_IFC_ATM_VCC ) 
      {
         void *vccObjValue;
         if ( BcmCfm_objGet(BCMCFM_OBJ_IFC_ATM_VCC, &vccObjValue, &pObj->attachToIndex ) == BcmCfm_Ok ) 
         {
            PBcmCfm_AtmVccCfg_t vcc = (PBcmCfm_AtmVccCfg_t)vccObjValue;
            /*Here Create one New WANConnectionDevice Template and Add it to the WANDevice*/
            PDeviceTemplate pdevtmpl = NULL;
            pdevtmpl = (struct DeviceTemplate *) malloc(sizeof(struct DeviceTemplate));
            memset(pdevtmpl, 0, sizeof(DeviceTemplate));
            pdevtmpl->type = (char *)malloc(strlen(subdevs_wandevice.type));
            sprintf(pdevtmpl->type,"%s",subdevs_wandevice.type);
            pdevtmpl->udn = subdevs_wandevice.udn;
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
            
            /* Add 1 for \0 */
            pdev->friendlyname = (char *)malloc(strlen("WanConnectionDevice") + strlen(itoa(wanObjIndex)) + 1);
            sprintf(pdev->friendlyname,"WanConnectionDevice.%d", wanObjIndex);

            pdev->parent = parent;
            pdev->template = pdevtmpl;

            pdev->next = parent->subdevs;
            parent->subdevs = pdev;
            /* call the device's intialization function, if defined. */
            if ((func = pdevtmpl->devinit) != NULL) 
            {
               va_start( ap, pdevtmpl);
               (*func)(pdev, DEVICE_CREATE, ap);
               va_end( ap);
            }

            //Generate UUID For The New Device
            char *udn;
            udn = malloc(50);
            strcpy(udn, "uuid:");
            uuidstr_create(udn+5, 50 - 5);
            pdev->udn = udn;

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

            psvc = init_service(pTemplate_WANDslLinkConfig, pdev, wanObjIndex);
            psvc->next = pdev->services;
            pdev->services = psvc;
#endif
            /*ADD ATMF5LOOPBACKDIAGNOSTICS*/
#ifdef INCLUDE_ATMF5LOOPBACKDIAG
            struct ServiceTemplate *pTemplate_WANATMF5LoopbackDiagnostics= NULL;
            pTemplate_WANATMF5LoopbackDiagnostics= (struct ServiceTemplate *) malloc(sizeof(struct ServiceTemplate));

            if (pTemplate_WANATMF5LoopbackDiagnostics == NULL) 
            {
               UPNP_ERROR(("malloc failed in %s\n", __FUNCTION__));
            }
            memset(pTemplate_WANATMF5LoopbackDiagnostics, 0, sizeof(ServiceTemplate));

            pTemplate_WANATMF5LoopbackDiagnostics->name = (char *)malloc(strlen(Template_WANATMF5LoopbackDiagnostics.name));
            sprintf(pTemplate_WANATMF5LoopbackDiagnostics->name,"%s",Template_WANATMF5LoopbackDiagnostics.name);
            pTemplate_WANATMF5LoopbackDiagnostics->svcinit = Template_WANATMF5LoopbackDiagnostics.svcinit;
            pTemplate_WANATMF5LoopbackDiagnostics->getvars = Template_WANATMF5LoopbackDiagnostics.getvars;
            pTemplate_WANATMF5LoopbackDiagnostics->svcxml = Template_WANATMF5LoopbackDiagnostics.svcxml;
            pTemplate_WANATMF5LoopbackDiagnostics->nvariables = Template_WANATMF5LoopbackDiagnostics.nvariables;
            pTemplate_WANATMF5LoopbackDiagnostics->variables = Template_WANATMF5LoopbackDiagnostics.variables;
            pTemplate_WANATMF5LoopbackDiagnostics->actions = Template_WANATMF5LoopbackDiagnostics.actions;
            pTemplate_WANATMF5LoopbackDiagnostics->count = Template_WANATMF5LoopbackDiagnostics.count;
            pTemplate_WANATMF5LoopbackDiagnostics->serviceid = Template_WANATMF5LoopbackDiagnostics.serviceid; 
            pTemplate_WANATMF5LoopbackDiagnostics->schema = Template_WANATMF5LoopbackDiagnostics.schema;

            psvc = init_service(pTemplate_WANATMF5LoopbackDiagnostics, pdev, wanObjIndex);
            psvc->next = pdev->services;
            pdev->services = psvc;
#endif
            /*ADD WANIPCONNECTION or WANPPPCONNECTION service*/
            if ( pObj->protocol == BcmCfm_NtwkIntfProtoPPPoA  || pObj->protocol==BcmCfm_NtwkIntfProtoPPPoE ) 
            {
#ifdef INCLUDE_PPPCONNECTION
               struct ServiceTemplate *pTemplate_WANPPPConnection = NULL;
               pTemplate_WANPPPConnection= (struct ServiceTemplate *) malloc(sizeof(struct ServiceTemplate));

               if (pTemplate_WANPPPConnection == NULL) 
               {
                  UPNP_ERROR(("malloc failed in %s\n", __FUNCTION__));
               }
               memset(pTemplate_WANPPPConnection, 0, sizeof(ServiceTemplate));

               pTemplate_WANPPPConnection->name = (char *)malloc(strlen(Template_WANPPPConnection.name));
               sprintf(pTemplate_WANPPPConnection->name,"%s",Template_WANPPPConnection.name);

               pTemplate_WANPPPConnection->svcinit = Template_WANPPPConnection.svcinit;
               pTemplate_WANPPPConnection->getvars = Template_WANPPPConnection.getvars;
               pTemplate_WANPPPConnection->svcxml = Template_WANPPPConnection.svcxml;
               pTemplate_WANPPPConnection->nvariables = Template_WANPPPConnection.nvariables;
               pTemplate_WANPPPConnection->variables = Template_WANPPPConnection.variables;
               pTemplate_WANPPPConnection->actions = Template_WANPPPConnection.actions;
               pTemplate_WANPPPConnection->count = Template_WANPPPConnection.count;
               pTemplate_WANPPPConnection->serviceid = Template_WANPPPConnection.serviceid; 
               pTemplate_WANPPPConnection->schema = Template_WANPPPConnection.schema;

               psvc = init_service(pTemplate_WANPPPConnection, pdev ,wanObjIndex);
               psvc->next = pdev->services;
               pdev->services = psvc;
#endif
            } 
            else if ( pObj->protocol == BcmCfm_NtwkIntfProtoIPoA 
                      || pObj->protocol == BcmCfm_NtwkIntfProtoMAC 
                      || pObj->protocol == BcmCfm_NtwkIntfProtoIPoWAN ) 
            {
#ifdef INCLUDE_IPCONNECTION
               struct ServiceTemplate *pTemplate_WANIPConnection = NULL;
               pTemplate_WANIPConnection= (struct ServiceTemplate *) malloc(sizeof(struct ServiceTemplate));

               if (pTemplate_WANIPConnection == NULL) 
               {
                  UPNP_ERROR(("malloc failed in %s\n", __FUNCTION__));
               }
               memset(pTemplate_WANIPConnection, 0, sizeof(ServiceTemplate));

               pTemplate_WANIPConnection->name = (char *)malloc(strlen(Template_WANIPConnection.name));
               sprintf(pTemplate_WANIPConnection->name,"%s",Template_WANIPConnection.name);

               pTemplate_WANIPConnection->svcinit = Template_WANIPConnection.svcinit;
               pTemplate_WANIPConnection->getvars = Template_WANIPConnection.getvars;
               pTemplate_WANIPConnection->svcxml = Template_WANIPConnection.svcxml;
               pTemplate_WANIPConnection->nvariables = Template_WANIPConnection.nvariables;
               pTemplate_WANIPConnection->variables = Template_WANIPConnection.variables;
               pTemplate_WANIPConnection->actions = Template_WANIPConnection.actions;
               pTemplate_WANIPConnection->count = Template_WANIPConnection.count;
               pTemplate_WANIPConnection->serviceid = Template_WANIPConnection.serviceid; 
               pTemplate_WANIPConnection->schema = Template_WANIPConnection.schema;

               psvc = init_service(pTemplate_WANIPConnection, pdev ,wanObjIndex);
               psvc->next = pdev->services;
               pdev->services = psvc;
#endif
            } 

            // Free the mem allocated this object by the get API.
            BcmCfm_objFree(BCMCFM_OBJ_IFC_ATM_VCC, (void*)vcc);

         }
      }
      // Free the mem allocated this object by the get API.
      BcmCfm_objFree(BCMCFM_OBJ_NTWK_INTF, objValue);
      wanObjIndex++;
    }
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


void destroy_device(PDevice pdev)
{
    static int indent = 0;
    PFDEVINIT func;
    PDevice psubdev, nextdev, *ppdev;
    PService psvc, nextsvc;

    syslog(LOG_DEBUG,"%*sDestroying %sdevice \"%s\".\r\n", indent, "", (ISROOT(pdev) ? "root " : ""), 
           pdev->template->type);

    /* destroy all subdevices in this device. */
    if (pdev->subdevs) 
    {
       for (psubdev = pdev->subdevs; psubdev; psubdev = nextdev) 
       {
          nextdev = psubdev->next;
          indent += 4;
          destroy_device(psubdev);
          indent -= 4;
       }
    }
    
    /* destroy all service in this device. */
    if (pdev->services) 
    {
       for (psvc = pdev->services; psvc; psvc = nextsvc) 
       {
          nextsvc = psvc->next;
          destroy_service(psvc);
       }
    }

    /* call this device's destroy function, if defined. */
    if ((func = pdev->template->devinit) != NULL) 
    {
       (*func)(pdev, DEVICE_DESTROY, (va_list) NULL );
    }

    /*  remove the device from the root device list */
    if (ISROOT(pdev)) 
    {
       for (ppdev = &root_devices; *ppdev; ppdev = &(*ppdev)->next) 
       {
          if (*ppdev == pdev) 
          {
             *ppdev = (*ppdev)->next;
             break;
          }
       }
    }

    /* finally, free the memory allocated in init_device(); */
    free(pdev);
}


/** Print an XML device description for a device and all its subdevices.
 *  We used to just print the static XML device description from a file, but now that the 
 *  IGD is more dynamic and can adjust to different gateway configurations,
 *  we must dynamically generate the XML.
 */
void device_xml(PDevice pdev, UFILE *up)
{
   PFDEVXML func;
   char *friendlyname;
   char SerialNumber[64];

   /* call the device's xml function, if defined. */
   if ((func = pdev->template->devxml) != NULL) 
   {
      (*func)(pdev, up);
      return;
   }

   if(CMM_GetStr("InternetGatewayDevice.DeviceInfo.SerialNumber", SerialNumber, sizeof(SerialNumber), NULL, 0) != CMM_SUCCESS)
   {       
   	   TR64FE_TRACE("Failed to read SerialNumber!\n");     
       return;    
   }

   if (ISROOT(pdev)) 
   { 
      uprintf(up,"<?xml version=\"1.0\"?>\r\n");      
      uprintf(up,"<root xmlns=\"urn:%s:device-1-0\">\r\n",TR64_DSLFORUM_SCHEMA);
      uprintf(up,"<specVersion>\r\n");
      uprintf(up,"<major>1</major>\r\n");
      uprintf(up,"<minor>0</minor>\r\n");
      uprintf(up,"</specVersion>\r\n");

      uprintf(up,"<URLBase>http://192.168.1.1:%d/</URLBase>\r\n", TR64C_HTTP_CONN_PORT);
   }

   if (pdev->friendlyname)
   {
      friendlyname = pdev->friendlyname;
   }
   else 
   {
      friendlyname = pdev->template->type;
   }

   uprintf(up, "<device>\r\n");
   uprintf(up, "<deviceType>%s</deviceType>\r\n", pdev->template->type);
   if (ISROOT(pdev)) 
   {
	  uprintf(up, "<presentationURL>http://192.168.1.1:80/</presentationURL>\r\n");
   }
   uprintf(up, "<friendlyName>%s</friendlyName>\r\n", friendlyname);
   uprintf(up, "<manufacturer>%s</manufacturer>\r\n", DEV_MFR);
   uprintf(up, "<manufacturerURL>%s</manufacturerURL>\r\n", DEV_MFR_URL);
   uprintf(up, "<modelDescription>%s</modelDescription>\r\n", DEV_MODEL_DESCRIPTION);
   uprintf(up, "<modelName>%s</modelName>\r\n", DEV_MODEL);
   uprintf(up, "<modelNumber>%s</modelNumber>\r\n", DEV_MODEL_NO);
   uprintf(up, "<modelURL>%s</modelURL>\r\n", DEV_MODEL_URL);
   uprintf(up, "<serialNumber>%s</serialNumber>\r\n", SerialNumber);
   uprintf(up, "<UDN>%s</UDN>\r\n", pdev->udn);

   /* generate XML for any services in this device. */
   device_servicelist(pdev, up);

   /* generate XML for any subdevices in this device. */
   device_devicelist(pdev, up);

   uprintf(up, "</device>\r\n");

   if (ISROOT(pdev)) 
   {
      uprintf(up, "</root>\r\n");
   }
}
    
    
void device_devicelist(PDevice pdev, UFILE *up)
{
    PDevice  psubdev;

    /* generate XML for any subdevices in this device. */
    if (pdev->subdevs) 
    {
       uprintf(up, "<deviceList>\r\n");
       for (psubdev = pdev->subdevs; psubdev; psubdev = psubdev->next) 
       {
          device_xml(psubdev, up);
       }
       uprintf(up, "</deviceList>\r\n");
    }
}


void device_servicelist(PDevice pdev, UFILE *up)
{
   char svcurl[200];
   PService psvc;

   /* generate XML for any services in this device. */
   if (pdev->services)
   {
      uprintf(up, "<serviceList>\r\n");
      forall_services(pdev, psvc) {
         snprintf(svcurl, sizeof(svcurl), "/%s/%s", pdev->udn, psvc->template->name);

         uprintf(up, "<service>\r\n");
         uprintf(up, "<serviceType>urn:%s:service:%s</serviceType>\r\n", 
                 psvc->template->schema, psvc->template->name);
         if (psvc->template->serviceid) 
         {
            uprintf(up, "<serviceId>%s%d</serviceId>\r\n", 
                    psvc->template->serviceid, psvc->instance);
         }
         else 
         {
            uprintf(up, "<serviceId>urn:%s:serviceId:%s%d</serviceId>\r\n", 
                    TR64_DSLFORUM_SCHEMA,psvc->template->name, psvc->instance);
         }
         uprintf(up, "<controlURL>/%s/%s</controlURL>\r\n", pdev->udn, psvc->template->name);
         uprintf(up, "<eventSubURL>/%s/%s</eventSubURL>\r\n", pdev->udn, psvc->template->name);
         uprintf(up, "<SCPDURL>/dynsvc/%s.xml</SCPDURL>\r\n", psvc->template->name);
         uprintf(up, "</service>\r\n");
      }
      uprintf(up, "</serviceList>\r\n");
   }
}


/* Given a device pointer, return the root device for that device. */
PDevice rootdev(PDevice pdev)
{
   while (pdev->parent) 
      pdev = pdev->parent;
   return pdev;
}


/* Device iterator used by the forall_devices() macro.
   
   The first call to device_iterator() should have NULL as its argument.  
   Subsequent calls will return the next device in depth first, pre-order.  
*/
PDevice device_iterator(PDevice pdev)
{
   PDevice nextdev;

   if (pdev == NULL) 
   {
      nextdev = root_devices;
   } 
   else 
   {
      if (pdev->subdevs) 
      {
         nextdev = pdev->subdevs;
      } 
      else
      {
         nextdev = pdev;
         while (nextdev)
         {
            if (nextdev->next) 
            {
               nextdev = nextdev->next;
               break;
            } 
            else
            {
               nextdev = nextdev->parent;
            }
         }
      } 
   }
   return nextdev;
}

