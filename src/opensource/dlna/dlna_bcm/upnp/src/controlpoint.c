/***************************************************************************
*     (c)2008-2009 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
*   
*  Except as expressly set forth in the Authorized License,
*   
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*   
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
*  USE OR PERFORMANCE OF THE SOFTWARE.
*  
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
*  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: controlpoint.c $
 * $brcm_Revision: 11 $
 * $brcm_Date: 9/30/09 4:19p $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: /AppLibs/upnp/src/controlpoint.c $
 * 
 * 11   9/30/09 4:19p dliu
 * SW7405-2482: Add implemention for DMC
 * 
 * 10   9/16/09 10:21a dliu
 * SW7405-2482: Add support for DMC
 * 
 * 9   7/2/09 10:07a dliu
 * PR55615: Add support for maxAge = 'infinite'
* 
***************************************************************************/
#include "upnp_priv.h"
#include "device.h"
#include "bsocket.h"
#include "list.h"
#include "subscription.h"
#include "controlpoint.h"
#include "url.h"
#include "gena.h"
#include "soap.h"
#include "ssdp.h"
#include "debug.h"
#include "service.h"

/* Add by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
#include "cms.h"
#include "cms_util.h"
#include "cms_core.h"
#include "cms_msg.h"
#include "cms_eid.h"
#include "cms_log.h"
#include "cms_log.h"

void BUPnPControlPoint_UpdateProxyDerviceObject(int add, BUPnPDeviceHandle hDevice, char* deviceType);
#endif
/* Add by tuhanyu, end */

typedef struct InvokeAysncParam
{
    BUPnPDeviceHandle hDevice;
    BUPnPServiceHandle hService;
    BUPnPActionHandle hAction;
} InvokeAsyncParam;

struct ControlPoint* g_ctrlPoint;

static BUPnPError BUPnPControlPoint_DownloadDescription(BUPnPDeviceHandle hDevice);
static void BUPnPControlPoint_VerifyDeviceAlive(TimerHandle handle, void* param);
static void BUPnPControlPoint_GetDescription(void* data);
static void BUPnPControlPoint_InvokeFunc(void* data);
static void BUPnPControlPoint_DestroyDevicePriv(BUPnPDeviceHandle hDevice);
static void BUPnPControlPoint_DestroyServicePriv(BUPnPDeviceHandle hDevice, BUPnPServiceHandle hService);
static BUPnPError BUPnPControlPoint_DownloadServiceDescription(BUPnPDeviceHandle hDevice, HttpContext* pContext);
static void BUPnPControlPoint_TriggerCallback(BUPnPDeviceHandle hDevice);

#define MX_TIME 5

BUPnPError BUPnPControlPoint_Initialize(BUPnPControlPointOpenSettings* pOpenSettings)
{
    /* Create the object */
    g_ctrlPoint = (ControlPointHandle)BUPnPCalloc(1, sizeof(*g_ctrlPoint));
    if (!g_ctrlPoint)
    {
        UPNP_DEBUG_ERROR(BUPnPDebugCategory_Discovery, ("Out of memory")); 
        return UPNP_E_OUT_OF_MEMORY;
    }

    g_ctrlPoint->SsdpAddr.sin_family = AF_INET;
    g_ctrlPoint->SsdpAddr.sin_addr.s_addr = inet_addr(SSDP_ADDRESS);
    g_ctrlPoint->SsdpAddr.sin_port = htons(SSDP_PORT);

    /* Initialize device lists */
    BLST_INIT(&g_ctrlPoint->deviceList);
    BLST_INIT(&g_ctrlPoint->subscriptionList);

    g_ctrlPoint->deviceListMutex = bthread_mutex_create();
    g_ctrlPoint->subscriptionListMutex = bthread_mutex_create();

    g_ctrlPoint->deviceAdded = pOpenSettings->deviceAdded;
    g_ctrlPoint->deviceRemoved = pOpenSettings->deviceRemoved;
    g_ctrlPoint->propertyChanged = pOpenSettings->propertyChanged;
    g_ctrlPoint->searchOption = pOpenSettings->searchOption;
    if (pOpenSettings->searchOption == eSpecificDevice)
        g_ctrlPoint->searchType = BUPnPStrdup(pOpenSettings->searchType);
    g_ctrlPoint->userAgent = BUPnPStrdup(pOpenSettings->userAgent);
    g_ctrlPoint->context = pOpenSettings->context;

    return UPNP_E_SUCCESS;
}

void BUPnPControlPoint_Destroy()
{
    BUPnPDeviceHandle hDevice;
    if (g_ctrlPoint)
    {
        bthread_mutex_lock(g_ctrlPoint->deviceListMutex);
        /* We have to manually loop the list because the pointer could be gone */
        while((hDevice=BLST_FIRST(&g_ctrlPoint->deviceList)) != NULL)
        {
            BLST_REMOVE(&g_ctrlPoint->deviceList, hDevice, BUPnPDevice, link);
            BUPnPControlPoint_DestroyDevicePriv(hDevice);
            BUPnPDevice_Destroy(hDevice);
        }
        bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);
        bthread_mutex_destroy(g_ctrlPoint->deviceListMutex);
        BUPnPFree(g_ctrlPoint->searchType);
        BUPnPFree(g_ctrlPoint->userAgent);
        BUPnPFree(g_ctrlPoint);
    }
}

struct sockaddr_in BUPnPControlPoint_GetSsdpAddr()
{
    return g_ctrlPoint->SsdpAddr;
}

BUPnPError BUPnPControlPoint_UnsubscribeDevice(BUPnPRemoteDeviceHandle hDevice)
{
    struct BUPnPService* pService;

    for(pService=BLST_FIRST(&hDevice->device->serviceList); pService; pService=BLST_NEXT(pService, link)) 
    {
        /* it is okay unsubscribe fails */
        Gena_Unsubscribe(pService->subscriptionId);
    }
    return UPNP_E_SUCCESS;
}

BUPnPError BUPnPControlPoint_SubscribeDevice(BUPnPRemoteDeviceHandle hDevice)
{
    struct BUPnPService *pService;
    BUPnPError result;

    if (BUPnPControlPoint_CheckDeviceAlive(hDevice->device) == false)
        return UPNP_E_NO_DEVICE;

    for(pService=BLST_FIRST(&hDevice->device->serviceList); pService; pService=BLST_NEXT(pService, link)) 
    {
        /* Only subscribe if we got a sub url */
        /* TODO: Change this to only subscribe to services we care about because
          * some device have additional service that we don't know about */
        if (pService->serviceInfo->eventSubUrl != NULL)
        {
            UPNP_DEBUG_TRACE(BUPnPDebugCategory_Eventing, ("found service sub url %s\n", pService->serviceInfo->eventSubUrl));
            result = Gena_Subscribe(hDevice->device, pService);

            if (result != UPNP_E_SUCCESS)
                return result;
        }
    }
    return UPNP_E_SUCCESS;
}

void BUPnPControlPoint_SearchDevices()
{
    /* Always send search twice per the spec */
    if (g_ctrlPoint->searchOption == eSpecificDevice)
    {
        UPNP_DEBUG_TRACE( BUPnPDebugCategory_Discovery,  ("Send M-SEARCH request for %s\n", g_ctrlPoint->searchType));

        Ssdp_SendSearchRequest( MX_TIME, g_ctrlPoint->searchType);
        Ssdp_SendSearchRequest( MX_TIME, g_ctrlPoint->searchType);
    }
    else
    {
        UPNP_DEBUG_TRACE( BUPnPDebugCategory_Discovery,  ("Send M-SEARCH request for %s\n", SSDP_ALL));
        Ssdp_SendSearchRequest( MX_TIME, SSDP_ALL);
        Ssdp_SendSearchRequest( MX_TIME, SSDP_ALL);
    }
}

BUPnPError BUPnPControlPoint_GetIconCount(BUPnPRemoteDeviceHandle hDevice, size_t* count)
{
    BUPnPIconHandle hIcon;
    *count = 0;

    if (BUPnPControlPoint_CheckDeviceAlive(hDevice->device))
    {
        if ( !BLST_EMPTY(&hDevice->device->iconList) )
        {
            for(hIcon = BLST_FIRST(&hDevice->device->iconList); hIcon; hIcon = BLST_NEXT(hIcon, link))
                *count++;
        }
    }
    else
        return UPNP_E_NO_DEVICE;

    return UPNP_E_SUCCESS;
}

BUPnPError BUPnPControlPoint_GetDeviceIconInfo(BUPnPRemoteDeviceHandle hDevice, int index, struct BUPnPIconInfo* pIconInfo)
{
    struct BUPnPIcon* pIcon;

    if (BUPnPControlPoint_CheckDeviceAlive(hDevice->device))
    {
        pIcon = BLST_FIRST(&hDevice->device->iconList);
        for(;pIcon; pIcon = BLST_NEXT(pIcon, link))
        {
            if (index-- == 0)
            {
                memcpy(pIconInfo, &pIcon->iconInfo, sizeof(struct BUPnPIconInfo));
                return UPNP_E_SUCCESS;
            }
        }
        return UPNP_E_NO_ICON;
    }
    else
    {
        return UPNP_E_NO_DEVICE;
    }
}

void BUPnPControlPoint_DestroyIcons(void)
{
}

BUPnPError BUPnPControlPoint_GetDeviceInfo(BUPnPRemoteDeviceHandle hDevice, BUPnPDeviceInfo* pDeviceInfo)
{
    if (BUPnPControlPoint_CheckDeviceAlive(hDevice->device) == false)
        return UPNP_E_NO_DEVICE;
    memcpy(pDeviceInfo, hDevice->device->deviceInfo, sizeof(*pDeviceInfo));
    return UPNP_E_SUCCESS;
}

BUPnPError BUPnPControlPoint_DownloadServiceDescription(BUPnPDeviceHandle hDevice, HttpContext* pContext)
{
    int result = 0;
    BUPnPServiceHandle hService;
    HttpStatusCode statusCode;
    char headers[UPNP_MAX_PACKET_SIZE];

    UPNP_DEBUG_TRACE( BUPnPDebugCategory_Description, ("Downloading Description from %s\n", BUPnPDevice_GetLocation(hDevice)));

    hService = BLST_FIRST(&hDevice->serviceList);
    
    while(hService!=NULL)
    {
        if (!Http_IsPersistent(pContext))
        {
            closesocket(pContext->s);
            pContext->s = BSocket_CreateTcpClient(hDevice->url.host, hDevice->url.port);
            if ( pContext->s <= 0 )
                return UPNP_E_SOCKET;
        }
        Http_ClearContext(pContext);

        snprintf(headers, sizeof(headers)-1, 
            "HOST: %s:%d\r\n"
            "CONTENT-LENGTH: 0\r\n"
            "\r\n",
            hDevice->url.host, hDevice->url.port);

        UPNP_DEBUG_TRACE(BUPnPDebugCategory_Description,  ("Request = %s", headers));

        /* send it off */
        if ((result = Http_Invoke(pContext, "GET", hService->serviceInfo->scpdUrl, headers, NULL)) != UPNP_E_SUCCESS)
        {
            if ( result == UPNP_E_SOCKET_CLOSED)
            {
                /* server might have closed the socket without telling us about it */
                closesocket(pContext->s);
                pContext->s = BSocket_CreateTcpClient(hDevice->url.host, hDevice->url.port);
                if ( pContext->s <= 0 )
                {
                    Http_DestroyContext(pContext);
                    return UPNP_E_SOCKET;
                }

                if ((result = Http_Invoke(pContext, "GET", hService->serviceInfo->scpdUrl, headers, NULL)) != UPNP_E_SUCCESS)
                {
                    UPNP_DEBUG_ERROR(BUPnPDebugCategory_Description,  ("Unable to download service description %s", 
                        hService->serviceInfo->scpdUrl));

                    BUPnPControlPoint_DestroyServicePriv(hDevice, hService);
                    closesocket(pContext->s);
                    return UPNP_E_SERVICE_NOT_FOUND;
                }
            }
            else
            {
                UPNP_DEBUG_ERROR(BUPnPDebugCategory_Description,  ("Unable to download service description %s", 
                    hService->serviceInfo->scpdUrl));
                BUPnPControlPoint_DestroyServicePriv(hDevice, hService);
                closesocket(pContext->s);
                return UPNP_E_SERVICE_NOT_FOUND;
            }
        }

        UPNP_DEBUG_TRACE(BUPnPDebugCategory_Description, ("Content = %s", Http_GetBody(pContext)));

        if (Http_GetBody(pContext))
        {
            statusCode = Http_GetStatusCode(pContext);
            if (statusCode != HTTP_OK)
            {
                UPNP_DEBUG_ERROR( BUPnPDebugCategory_Description, ("Unable to retrieve service description")); 
                result = UPNP_E_INVALID_CONTROL_URL;
            }
            else
                result = BUPnPService_ParseDescriptipn(hService, Http_GetBody(pContext));
        }
        else
        {
            UPNP_DEBUG_ERROR( BUPnPDebugCategory_Description, ("Unable to retrieve device description")); 
            result = UPNP_E_INVALID_CONTROL_URL;
        }
        hService = BLST_NEXT(hService, link);
    }
    Http_DestroyContext(pContext);
    UPNP_DEBUG_TRACE(BUPnPDebugCategory_Description, ("Get description from %s complete", BUPnPDevice_GetLocation(hDevice)));
    return result;

    
}

BUPnPError BUPnPControlPoint_DownloadDescription(BUPnPDeviceHandle hDevice)
{
    int urlBaseLen;
    char* pUrlBase;
    BUPnPError result;
    HttpContext context;
    BUPnPDeviceHandle hSubDevice;
    char uri[UPNP_MAX_URI_SIZE], headers[UPNP_MAX_PACKET_SIZE];

    UPNP_DEBUG_TRACE( BUPnPDebugCategory_Description, ("Downloading Description from %s\n", BUPnPDevice_GetLocation(hDevice)));

    result = Http_CreateContext(&context);
    if ( result != UPNP_E_SUCCESS )
        return result;
    
    context.s = BSocket_CreateTcpClient(hDevice->url.host, hDevice->url.port);
    if ( context.s <= 0 )
        return UPNP_E_SOCKET;

    if (hDevice->url.query != NULL)
        sprintf(uri, "%s?%s", hDevice->url.path, hDevice->url.query);
    else
        strcpy(uri, hDevice->url.path);
    
    snprintf(headers, sizeof(headers)-1, 
        "HOST: %s:%d\r\n"
        "CONTENT-LENGTH: 0\r\n"
        "\r\n",
        hDevice->url.host, hDevice->url.port);

    UPNP_DEBUG_TRACE(BUPnPDebugCategory_Description,  ("Request = %s", headers));

    /* send it off */
    if ((result = Http_Invoke(&context, "GET", uri, headers, NULL)) != UPNP_E_SUCCESS)
    {
        Http_DestroyContext(&context);
        return result;
    }

    UPNP_DEBUG_TRACE(BUPnPDebugCategory_Description, ("Content = %s", Http_GetBody(&context)));

    if (Http_GetBody(&context))
    {
        urlBaseLen = strlen(hDevice->url.path);
        if (hDevice->url.path[urlBaseLen-1] != '/')
        {
            pUrlBase = strrchr(hDevice->url.path, '/');
            pUrlBase[1] = '\0';
        }
        hDevice->urlBase = BUPnPStrdup(hDevice->url.path);
        result = BUPnPDevice_ParseDescription(hDevice, Http_GetBody(&context));
        if (result == UPNP_E_SUCCESS)
            result = BUPnPControlPoint_DownloadServiceDescription(hDevice, &context);

        
    }
    else
    {
        UPNP_DEBUG_ERROR( BUPnPDebugCategory_Description, ("Unable to retrieve device description")); 
        result = UPNP_E_INVALID_CONTROL_URL;
    }

    for(hSubDevice=BLST_FIRST(&g_ctrlPoint->deviceList); hSubDevice; hSubDevice=BLST_NEXT(hSubDevice, link)) 
    {
        if (hSubDevice->parent != NULL)
        {
            if(hSubDevice->parent == hDevice)
                result = BUPnPControlPoint_DownloadServiceDescription(hSubDevice, &context);
        }
    }

    closesocket(context.s);
    Http_DestroyContext(&context);

    UPNP_DEBUG_TRACE(BUPnPDebugCategory_Description, ("Get description from %s complete", BUPnPDevice_GetLocation(hDevice)));

    return result;
}

BUPnPError BUPnPControlPoint_GetService(BUPnPRemoteDeviceHandle hDevice, const char* serviceName, struct BUPnPService** ppService)
{
    BUPnPServiceHandle hService;

    if (hDevice == NULL)
        return UPNP_E_NO_DEVICE;

    if (BUPnPControlPoint_CheckDeviceAlive(hDevice->device) == false)
        return UPNP_E_NO_DEVICE;

    hService = BLST_FIRST(&hDevice->device->serviceList);
    while(hService)
    {
        if (MATCH(hService->serviceInfo->serviceId, serviceName))
        {
            *ppService = hService;
            return UPNP_E_SUCCESS;
        }
        hService = BLST_NEXT(hService, link);
    }
    return UPNP_E_SUCCESS;
}

static void BUPnPControlPoint_DestroyServicePriv(BUPnPDeviceHandle hDevice, BUPnPServiceHandle hService)
{
    BUPnPFree((void*)(hService->serviceInfo->serviceId));
    BUPnPFree((void*)(hService->serviceInfo->scpdUrl));
    BUPnPFree((void*)(hService->serviceInfo->controlUrl));
    BUPnPFree((void*)(hService->serviceInfo->eventSubUrl));
    BUPnPFree((void*)(hService->serviceInfo));

    BLST_REMOVE_HEAD(&hDevice->serviceList, link);
    BUPnPService_Destroy(hService);
}

static void BUPnPControlPoint_DestroyDevicePriv(BUPnPDeviceHandle hDevice)
{
    BUPnPServiceHandle hService;
    BUPnPIconHandle hIcon;
    BUPnPActionHandle hAction;
    BUPnPStateVariableHandle hStateVariable;
    size_t i;

    /* we don't close the socket here because it will cause the select to fail */

    hService = BLST_FIRST(&hDevice->serviceList);
    while(hService)
    {
        if ( hService->subscriptionId != NULL)
            Gena_RemoveSubscriptionByService(hService);

        BUPnPFree((void*)(hService->serviceInfo->serviceId));
        BUPnPFree((void*)(hService->serviceInfo->scpdUrl));
        BUPnPFree((void*)(hService->serviceInfo->controlUrl));
        BUPnPFree((void*)(hService->serviceInfo->eventSubUrl));
        BUPnPFree((void*)(hService->serviceInfo));

        while( (hAction = BLST_FIRST(&hService->actions)) != NULL )
        {
            BLST_REMOVE_HEAD(&hService->actions, link);

            for ( i=0; i<hAction->argumentCount; i++ )
            {
                BUPnPFree((void*)hAction->arguments[i].argumentInfo->name);
                BUPnPFree(hAction->arguments[i].argumentInfo);
            }
            BUPnPFree((void*)hAction->actionInfo->name);
            BUPnPFree(hAction->actionInfo);
            BUPnPAction_Destroy(hAction);
        }

        while( (hStateVariable = BLST_FIRST(&hService->stateVariables)) != NULL )
        {
            BUPnPFree((void*)hStateVariable->variableInfo->name);
            BUPnPFree(hStateVariable->variableInfo);
            BLST_REMOVE_HEAD(&hService->stateVariables, link);
            BUPnPStateVariable_Destroy(hStateVariable);
        }
        hService = BLST_NEXT(hService, link);
    }

    while( (hIcon = BLST_FIRST(&hDevice->iconList)) != NULL )
    {
        BLST_REMOVE_HEAD(&hDevice->iconList, link);

        BUPnPFree((void*)(hIcon->iconInfo->mimeType));
        BUPnPFree((void*)(hIcon->iconInfo->url));
        BUPnPFree(hIcon->iconInfo);
        BUPnPFree(hIcon);
    }

    BUPnPFree((void*)(hDevice->deviceInfo->udn));
    BUPnPFree((void*)(hDevice->deviceInfo->friendlyName));
    BUPnPFree((void*)(hDevice->deviceInfo->manufacturer));
    BUPnPFree((void*)(hDevice->deviceInfo->manufacturerUrl));
    BUPnPFree((void*)(hDevice->deviceInfo->modelDescription));
    BUPnPFree((void*)(hDevice->deviceInfo->modelName));
    BUPnPFree((void*)(hDevice->deviceInfo->modelNumber));
    BUPnPFree((void*)(hDevice->deviceInfo->modelUrl));
    BUPnPFree((void*)(hDevice->deviceInfo->serialNumber));
    BUPnPFree((void*)(hDevice->deviceInfo->upc));
    BUPnPFree((void*)(hDevice->deviceInfo->presentationUrl));

/* Add by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
    BUPnPFree((void*)(hDevice->deviceInfo->manufacturerOUI));
    BUPnPFree((void*)(hDevice->deviceInfo->productClass));
    BUPnPFree((void*)(hDevice->deviceInfo->serialNumberDev));
    BUPnPFree((void*)(hDevice->deviceInfo->provisioningCode));
    BUPnPFree((void*)(hDevice->deviceInfo->deviceType));
    BUPnPFree((void*)(hDevice->deviceInfo->softwareVersion));
    BUPnPFree((void*)(hDevice->deviceInfo->hardwareVersion));
#endif
/* Add by tuhanyu, end */
    
    BUPnPFree((void*)(hDevice->urlBase));

    BUPnPFree(hDevice->deviceInfo);
}

static void BUPnPControlPoint_GetDescription(void* data)
{
    int result;
    BUPnPDeviceHandle hDevice = (BUPnPDeviceHandle)data;
    BUPnPDeviceHandle hSubDevice;

    /* device might be removed before we have a chance to get it */
    if (BUPnPControlPoint_CheckDeviceAlive(hDevice) == false)
        return;

    /* cannot lock here because DownloadDescription could take up to 30 seconds */
    if ((result = BUPnPControlPoint_DownloadDescription(hDevice)) != UPNP_E_SUCCESS)
    {
        /* At this point, either the devie is incorrect or download failed so just remove it */
        bthread_mutex_lock(g_ctrlPoint->deviceListMutex); 
        assert(hDevice != NULL);
        BLST_REMOVE(&g_ctrlPoint->deviceList, hDevice, BUPnPDevice, link);
        bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);

        UPNP_DEBUG_ERROR(BUPnPDebugCategory_Description, ("Unabled to download description from %s, error=%d", hDevice->url.path, result));

        /* remove timer */
        if (hDevice->expirationTimer != NULL)
            Timer_Destroy(hDevice->expirationTimer);

        if ( hDevice)
        {
            /* This frees all resources in the device info structure */
            BUPnPControlPoint_DestroyDevicePriv(hDevice);
            BUPnPDevice_Destroy(hDevice);
        }
    }
    else
    {
        BUPnPControlPoint_TriggerCallback(hDevice);

        for(hSubDevice=BLST_FIRST(&g_ctrlPoint->deviceList); hSubDevice; hSubDevice=BLST_NEXT(hSubDevice, link)) 
        {
            if (hSubDevice->parent != NULL)
            {
                if(hSubDevice->parent == hDevice)
                    BUPnPControlPoint_TriggerCallback(hSubDevice);
            }
        }
    }
}

void BUPnPControlPoint_AddSubDevice(BUPnPDeviceHandle hDevice)
{
    bthread_mutex_lock(g_ctrlPoint->deviceListMutex);

    /* it is okay if this fails. Just means it was already added during discovery to prevent duplicated downloads */
    BLST_DICT_ADD(&g_ctrlPoint->deviceList, hDevice, BUPnPDevice, deviceInfo->udn, link, ignore);

ignore:
    bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);
}

void BUPnPControlPoint_TriggerCallback(BUPnPDeviceHandle hDevice)
{
    /* last check to make sure it is the proper device before we inform the application */
    if (g_ctrlPoint->searchType == NULL)
    {
        if (g_ctrlPoint->deviceAdded != NULL)
/* modify by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
            g_ctrlPoint->deviceAdded(g_ctrlPoint->context, hDevice, hDevice->deviceType);
#else
            g_ctrlPoint->deviceAdded(g_ctrlPoint->context, hDevice->deviceInfo->udn, hDevice->deviceType);
#endif
/* modify by tuhanyu, end */
    }
    else
    {
        if (IMATCH_PREFIX(g_ctrlPoint->searchType, hDevice->deviceType))
        {
            if (g_ctrlPoint->deviceAdded != NULL)
            {
/* modify by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
                g_ctrlPoint->deviceAdded(g_ctrlPoint->context, hDevice, hDevice->deviceType);
#else
                g_ctrlPoint->deviceAdded(g_ctrlPoint->context, hDevice->deviceInfo->udn, hDevice->deviceType);
#endif
/* modify by tuhanyu, end */
            }
        }
    }
}

/* modify by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
void BUPnPControlPoint_AddDevice(const char* uuid, int maxAge, const char* location, HttpContextHandle hContext)
#else
void BUPnPControlPoint_AddDevice(const char* uuid, int maxAge, const char* location)
#endif
/* modify by tuhanyu, end */
{
    BUPnPDeviceHandle hDevice;
    BUPnPDeviceInfo *hDeviceInfo;
    TimerSettings timerSettings;
    void* prevUuid;

    bthread_mutex_lock(g_ctrlPoint->deviceListMutex);

    UPNP_DEBUG_TRACE(BUPnPDebugCategory_Discovery, ("Adding Device at %s (%s)", location, uuid));

    /* Make sure it doesn't exist already, and make sure we don't already have the root device */
    for(hDevice=BLST_FIRST(&g_ctrlPoint->deviceList); hDevice; hDevice=BLST_NEXT(hDevice, link)) 
    {
        if (IMATCH(hDevice->deviceInfo->udn, uuid))
            break;

        /* Make sure it not a sub device. If it is, we'll add it when we get description */
        if (MATCH(BUPnPDevice_GetLocation(hDevice), location))
        {
            bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);
            return;
        }
    }

    if (NULL != hDevice)
    {
        bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);

        /* already exists so just renew it */
        if (hDevice->expirationTimer != NULL)
        {
            prevUuid = Timer_GetParam(hDevice->expirationTimer);
            Timer_Destroy(hDevice->expirationTimer);
            if (maxAge != 0) /* maxAge of 0 indicates that it is infinite or not known */
            {
/* modify by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
                timerSettings.period = 1000*(maxAge+5);
#else
                timerSettings.period = 1000*maxAge;
#endif
/* modify by tuhanyu, end */
                timerSettings.callback = BUPnPControlPoint_VerifyDeviceAlive;
                timerSettings.param = prevUuid;
                timerSettings.executeOnce = true;
                hDevice->expirationTimer = Timer_Create(&timerSettings);
            }
        }
/* Add by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
        if(hDevice->url.host)
            BUPnPFree(hDevice->url.host);
        if(hDevice->url.path)
            BUPnPFree(hDevice->url.path);
        if(hDevice->url.query)
            BUPnPFree(hDevice->url.query);
        if(hDevice->url.buf)
            BUPnPFree(hDevice->url.buf);
        Url_Create(&(hDevice->url), location);

        BUPnPControlPoint_TriggerCallback(hDevice);
#endif
/* Add by tuhanyu, end */
        return;
    }

    /* Create a new device */
    hDeviceInfo = (BUPnPDeviceInfo*)BUPnPCalloc(1, sizeof(*hDeviceInfo));
    if ( hDeviceInfo == NULL )
    {
        bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);
        UPNP_DEBUG_ERROR(BUPnPDebugCategory_Description, ("Out of memory:Create device info"));
        return;
    }

/* Add by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
    hDeviceInfo->manufacturerOUI = BUPnPStrdup(Http_GetHeaderValue(hContext, "ManufacturerOUI"));
    hDeviceInfo->productClass = BUPnPStrdup(Http_GetHeaderValue(hContext, "ProductClass"));
    hDeviceInfo->serialNumberDev = BUPnPStrdup(Http_GetHeaderValue(hContext, "SerialNumber"));
    hDeviceInfo->provisioningCode = BUPnPStrdup(Http_GetHeaderValue(hContext, "ProvisioningCode"));
    hDeviceInfo->deviceType = BUPnPStrdup(Http_GetHeaderValue(hContext, "DeviceType"));
    hDeviceInfo->softwareVersion = BUPnPStrdup(Http_GetHeaderValue(hContext, "SoftwareVersion"));
    hDeviceInfo->hardwareVersion = BUPnPStrdup(Http_GetHeaderValue(hContext, "HardwareVersion"));
#endif
/* Add by tuhanyu, end */
    
    hDevice = BUPnPDevice_Create(hDeviceInfo);
    hDevice->deviceInfo->udn = BUPnPStrdup(uuid);
    Url_Create(&(hDevice->url), location);

    /* Keep track of cache-control time */
    if (maxAge != 0) /* maxAge is not infinite */
    {
/* modify by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
        timerSettings.period = 1000*(maxAge+5);
#else
        timerSettings.period = 1000*maxAge;
#endif
/* modify by tuhanyu, end */
        timerSettings.callback = BUPnPControlPoint_VerifyDeviceAlive;
        timerSettings.param = (void*)BUPnPStrdup(uuid);
        timerSettings.executeOnce = true;
        hDevice->expirationTimer = Timer_Create(&timerSettings);
    }

    /* Add it first so we don't call get description twice */
    BLST_DICT_ADD(&g_ctrlPoint->deviceList, hDevice, BUPnPDevice, deviceInfo->udn, link, error);

    bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);

    /* launch worker thread */
    UPNP_DEBUG_TRACE(BUPnPDebugCategory_Discovery, ("Device %s added", location, uuid));
    bthread_create(BUPnPControlPoint_GetDescription, (void*)hDevice); 

error:
    assert(true);
}

void BUPnPControlPoint_RemoveDevice(const char* uuid)
{
    BUPnPDeviceHandle hDevice;

    if (g_ctrlPoint == NULL)
        return;

    UPNP_DEBUG_TRACE(BUPnPDebugCategory_Description, ("Removing %s", uuid));

    bthread_mutex_lock(g_ctrlPoint->deviceListMutex);
    /* callback before we actually delete */
    BLST_DICT_FIND_BY_STR(&g_ctrlPoint->deviceList, hDevice, uuid, deviceInfo->udn, link);
    bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);
    
    if (hDevice)
    {
/* modify by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
        BUPnPControlPoint_UpdateProxyDerviceObject(0, hDevice, NULL);
#else
        
        if (g_ctrlPoint->deviceRemoved != NULL)
            g_ctrlPoint->deviceRemoved(g_ctrlPoint->context, uuid);

        bthread_mutex_lock(g_ctrlPoint->deviceListMutex);
        BLST_REMOVE(&g_ctrlPoint->deviceList, hDevice, BUPnPDevice, link);
        bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);

        /* This frees all resources in the device info structure */
        BUPnPControlPoint_DestroyDevicePriv(hDevice);
        BUPnPDevice_Destroy(hDevice);
#endif
/* modify by tuhanyu, end */
    }
}

/* modify by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
void BUPnPControlPoint_InvokeCallback(struct BUPnPService* pService, const char* variable)
#else
void BUPnPControlPoint_InvokeCallback(const char* serviceName, const char* variable)
#endif
/* modify by tuhanyu, end */
{
    if (g_ctrlPoint->propertyChanged != NULL)
/* modify by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
        g_ctrlPoint->propertyChanged(g_ctrlPoint->context, pService, variable);
#else
        g_ctrlPoint->propertyChanged(g_ctrlPoint->context, serviceName, variable);
#endif
/* modify by tuhanyu, end */
}

BUPnPError BUPnPControlPoint_Invoke(BUPnPRemoteDeviceHandle hDevice, BUPnPServiceHandle pService, BUPnPActionHandle hAction)
{
    //return Soap_Invoke(hDevice->device->url, pService, hAction);
    return Soap_Invoke(hDevice->device, pService, hAction);
}

static void BUPnPControlPoint_InvokeFunc(void* data)
{
    UNUSED_ARGUMENT(data);
    //InvokeAsyncParam* param = (InvokeAsyncParam*)data;
    //Soap_Invoke(param->hDevice->url, param->hService, param->hAction);
    /*g_ctrlPoint->callback(g_ctrlPoint->context, BUPnPControlPointEvent_eActionComplete, (void*)param->hAction); */
}

void BUPnPControlPoint_InvokeAsync(BUPnPDeviceHandle hDevice, BUPnPServiceHandle hService, BUPnPActionHandle hAction)
{
    InvokeAsyncParam* param = (InvokeAsyncParam*)BUPnPCalloc(1, sizeof(InvokeAsyncParam));
    param->hDevice = hDevice;
    param->hService = hService;
    param->hAction = hAction;
    bthread_create(BUPnPControlPoint_InvokeFunc, (void*)param);
}

bthread_mutex_t* BUPnPControlPoint_GetSubscriptionsMutex(void)
{
    return g_ctrlPoint->subscriptionListMutex;
}

struct BUPnPSubscriptionList* BUPnPControlPoint_GetSubscriptionList()
{
    return &g_ctrlPoint->subscriptionList;
}

static void BUPnPControlPoint_VerifyDeviceAlive(TimerHandle handle, void* param)
{
    UNUSED_ARGUMENT(handle);
    BUPnPControlPoint_RemoveDevice((const char*)param);

    BUPnPFree(param);
}

void BUPnPControlPoint_GetDefaultOpenSettings(BUPnPControlPointOpenSettings* pOpenSettings)
{
    memset(pOpenSettings, 0, sizeof(*pOpenSettings));
    
    /* Default to search for all devices */
    pOpenSettings->searchOption = eAllDevices;
}

/* Add by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
extern void *msgHandle;
void BUPnPControlPoint_SyncServiceAvaible(void *context, struct BUPnPService* pService, char* variable)
{
    BUPnPStateVariableHandle hStateVariable;
    BUPnPDeviceInfo *deviceInfo = pService->parent->deviceInfo;
    UINT32 msgDataLen = sizeof(UpnpActionMsgBody);
    char *buf;
    CmsMsgHeader *msg;
    UpnpActionMsgBody *info;
    CmsRet ret;

    if(!(buf = (char *)malloc(sizeof(CmsMsgHeader) + sizeof(UpnpActionMsgBody))))
    {
        printf("BUPnPControlPoint_SyncServiceAvaible: Can not alloc memory.\n");
        return;
    }
    msg = (CmsMsgHeader *) buf;
    info = (UpnpActionMsgBody *) &(buf[sizeof(CmsMsgHeader)]);
    
    hStateVariable = BUPnPService_GetStateVariable(pService, variable);

    msg->type = CMS_MSG_UPNPD_UPDATE_DEVICE;
    msg->src = EID_DMSD;
    msg->dst = EID_SSK;
    msg->flags_request = 1;
    msg->dataLength = msgDataLen;
    msg->wordData = UPNP_PROXY_DEVICE_EVENT;

    sprintf(info->manufacturerOUI, deviceInfo->manufacturerOUI);
    sprintf(info->productClass, deviceInfo->productClass);
    sprintf(info->serialNumber, deviceInfo->serialNumberDev);
    sprintf(info->servicename, pService->serviceInfo->serviceId);
    sprintf(info->actionname, "event");
    sprintf(info->arg[0].name, variable);
    sprintf(info->arg[0].value, hStateVariable->value.bytes?hStateVariable->value.bytes:"");
    
    if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
    {
        cmsLog_error("Could not send out CMS_MSG_UPNPD_UPDATE_DEVICE ret=%d", ret);
    }
    else
    {
        printf("BUPnPControlPoint_SyncServiceAvaible: CMS_MSG_UPNPD_UPDATE_DEVICE msg sent\n");
    }
    free(buf);

    return;
}

int BUPnPControlPoint_GetValue(char *value, UpnpActionMsgBody *body, char *name)
{
        char *buff = body->arg[0].value;
        char *temp, *end;

        if(strncmp(body->arg[0].name, "ParameterValueList", 18))
        {
            return UPNP_E_INVALID_ARG;
        }

        if(temp = strstr(buff, name))
        {
            temp = strstr(temp, "&lt;Value&gt;");
            temp += 13;
            end = strstr(temp, "&lt;/Value&gt;");
            if(!temp || !end)
            {
                return UPNP_E_INVALID_ARG;
            }
            if(0 != end-temp)
            {
                strncpy(value, temp, end-temp);
            }
        }
        else
        {
            return UPNP_E_INVALID_ARG;
        }

        return UPNP_E_SUCCESS;
}

void BUPnPControlPoint_UpdateProxyDerviceObject(int add, BUPnPDeviceHandle hDevice, char* deviceType)
{
        BUPnPDeviceInfo *deviceInfo = hDevice->deviceInfo;
        UpnpActionMsgBody *body = NULL;
        int result = 0;
        CmsRet ret;
        UINT32 msgDataLen = sizeof(UPnPProxyDeviceBody);
        char buf[sizeof(CmsMsgHeader)+sizeof(UPnPProxyDeviceBody)] = {0};
        CmsMsgHeader *msg = (CmsMsgHeader *)buf;
        UPnPProxyDeviceBody *info = (UPnPProxyDeviceBody *)&buf[sizeof(CmsMsgHeader)];
    
        if(hDevice->deviceType && strstr(hDevice->deviceType,"ManageableDevice") == 0)
        {
            UPNP_DEBUG_TRACE( BUPnPDebugCategory_Description, ("not ManageableDevice (%s).\n", hDevice->deviceType));
            return ;
        }
        if(!deviceInfo)
        {
            if(!deviceInfo->udn)
            {
                printf("BUPnPControlPoint_UpdateProxyDerviceObject: invalid hDevice.\n" );
                return ;
            }
        }
        
        if(add)
        {
            if(!(body = (char *)malloc(sizeof(UpnpActionMsgBody))))
            {
                printf("BUPnPControlPoint_SyncServiceAvaible: Can not alloc memory.\n");
                return;
            }
            memset(body, 0, sizeof(UpnpActionMsgBody));
            strcpy(body->deviceUuid, deviceInfo->udn);
            strcpy(body->servicename, "ConfigurationManagement");
            strcpy(body->actionname, "GetValues");
            strcpy(body->arg[0].name, "Parameters");
            strcpy(body->arg[0].value, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            strcat(body->arg[0].value, "<cms:ContentPathList xmlns:cms=\"urn:schemas-upnp-org:dm:cms\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:schemas-upnp-org:dm:cms http://www.upnp.org/schemas/dm/cms.xsd\">");
            strcat(body->arg[0].value, "<ContentPath>/UPnP/DM/DeviceInfo/</ContentPath>");
            strcat(body->arg[0].value, "</cms:ContentPathList>");
            
            if(BUPnP_ProcessAction(body, 1) != UPNP_E_SUCCESS)
            {
                printf("BUPnPControlPoint_UpdateProxyDerviceObject: GetValues</UPnP/DM/DeviceInfo/> Failed.\n");
//                free(body);
//                return;
            }

        
            BUPnPControlPoint_GetValue(info->friendlyName, body, "FriendlyName");
            BUPnPControlPoint_GetValue(info->devicetype, body, "DeviceType");
            BUPnPControlPoint_GetValue(info->provisioningCode, body, "ProvisioningCode");
            BUPnPControlPoint_GetValue(info->softwareVersion, body, "SoftwareVersion");
            BUPnPControlPoint_GetValue(info->softwareDescription, body, "SoftwareDescription");
            BUPnPControlPoint_GetValue(info->upTime, body, "UpTime");
            BUPnPControlPoint_GetValue(info->hardwareVersion, body, "HardwareVersion");
            result = result | BUPnPControlPoint_GetValue(info->manufacturerOUI, body, "ManufacturerOUI");
            result = result | BUPnPControlPoint_GetValue(info->productClass, body, "ProductClass");
            result = result | BUPnPControlPoint_GetValue(info->serialNumber, body, "SerialNumber");
    
            if(!strlen(info->manufacturerOUI) || !strlen(info->productClass) || !strlen(info->serialNumber))
            {
                printf("BUPnPControlPoint_UpdateProxyDerviceObject: Invalid device.\n");
                free(body);
                return;
            }
        }

        msg->type = CMS_MSG_UPNPD_UPDATE_DEVICE;
        msg->src = EID_DMSD;
        msg->dst = EID_SSK;
        msg->flags_request = 1;
        msg->dataLength = msgDataLen;
        msg->wordData = UPNP_PROXY_DEVICE_UPDATE;
        
        if(add)
        {
            BUPnPFree((void*)(deviceInfo->productClass));
            deviceInfo->productClass = BUPnPStrdup(info->productClass);
            BUPnPFree((void*)(deviceInfo->manufacturerOUI));
            deviceInfo->manufacturerOUI = BUPnPStrdup(info->manufacturerOUI);
            BUPnPFree((void*)(deviceInfo->serialNumberDev));
            deviceInfo->serialNumberDev = BUPnPStrdup(info->serialNumber);
            deviceInfo->online = 1;
        }
        else
        {
            strcpy(info->productClass, deviceInfo->productClass);
            strcpy(info->manufacturerOUI, deviceInfo->manufacturerOUI);
            strcpy(info->serialNumber, deviceInfo->serialNumberDev);
            deviceInfo->online = 0;
        }
        
        strcpy(info->deviceuuid,  deviceInfo->udn); 
        strcpy(info->friendyname,  deviceInfo->friendlyName); 
        strcpy(info->description,  deviceInfo->modelDescription); 
        strcpy(info->manufacture,  deviceInfo->manufacturer); 
        strcpy(info->moduleName,   deviceInfo->modelName); 
        strcpy(info->moduleNumber, deviceInfo->modelNumber); 
//        strcpy(info->serialNumber, deviceInfo->serialNumber); 
        if(add)
            strcpy(info->status, "online"); 
        else
            strcpy(info->status, "downline");

	if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
	{
	   cmsLog_error("Could not send out CMS_MSG_DSL_LOOP_DIAG_MONITOR ret=%d", ret);
	}
	else
	{
	   printf("BUPnPControlPoint_UpdateProxyDerviceObject: CMS_MSG_UPNPD_UPDATE_DEVICE msg sent\n");
	}
       if(body)
           free(body);
       return;
}

void BUPnPControlPoint_PrintDMServiceObject(void *context, BUPnPDeviceHandle hDevice, char* deviceType)
{
    BUPnPServiceHandle hService;
    BUPnPStateVariableHandle hStateVariable;
    const char *name, *value;
    size_t index;
    BUPnPActionHandle hAction;
    
    for ( hService = BLST_FIRST(&hDevice->serviceList); hService; hService = BLST_NEXT(hService, link) )
    {
        for(hStateVariable=BLST_FIRST(&hService->stateVariables); hStateVariable; hStateVariable=BLST_NEXT(hStateVariable, link)) 
        {
            name = BUPnPStateVariable_GetName(hStateVariable);
            value = BUPnPStateVariable_GetValue(hStateVariable);
            printf("%s %d ==> name=%s \n" , __func__ , __LINE__ , name); 
            printf("%s %d ==> value=%s \n" , __func__ , __LINE__ , value); 
        }
        printf("%s %d ==> ************************************************************\n" , __func__ , __LINE__); 
        for(hAction=BLST_FIRST(&hService->actions); hAction; hAction=BLST_NEXT(hAction, link)) 
        {
            printf("%s %d	hAction->name =%s \n " , __func__ , __LINE__ , hAction->name);
            for (index=0;index<hAction->argumentCount;index++)
            {
                printf("%s %d   hAction->arguments[%d].name) =%s \n " , __func__ , __LINE__ , index,hAction->arguments[index].name);
            }
        }
    }
}

void BUPnPControlPoint_AddDeviceCallback(void *context, BUPnPDeviceHandle hDevice, char* deviceType)
{
//	BUPnPControlPoint_PrintDMServiceObject( context, hDevice, deviceType);
    
	BUPnPControlPoint_UpdateProxyDerviceObject(1, hDevice, deviceType);
	return ;
}

void BUPnPControlPoint_TWCalllbackOpenSettings(BUPnPControlPointOpenSettings* pOpenSettings) 
{
    pOpenSettings->deviceAdded = BUPnPControlPoint_AddDeviceCallback;
    pOpenSettings->propertyChanged = BUPnPControlPoint_SyncServiceAvaible;
}
#endif
/* Add by tuhanyu, end */

const char* BUPnPControlPoint_GetUserAgent(void)
{
    return g_ctrlPoint->userAgent;
}

bool BUPnPControlPoint_CheckDeviceAlive(BUPnPDeviceHandle selectedDevice)
{
    BUPnPDeviceHandle hDevice;

    if (selectedDevice == NULL)
        return false;

    bthread_mutex_lock(g_ctrlPoint->deviceListMutex);
    /* We have to manually loop the list because the pointer could be gone */
    for(hDevice=BLST_FIRST(&g_ctrlPoint->deviceList); hDevice; hDevice=BLST_NEXT(hDevice, link)) 
    {
        if (selectedDevice == hDevice)
        {
            bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);
            return true;
        }
    }
    bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);
    return false;
}

bool BUPnPControlPoint_IsEnabled(void)
{
    if (g_ctrlPoint != NULL)
        return true;
    else 
        return false;
}
void BUPnPControlPoint_DestroyDevice(BUPnPRemoteDeviceHandle pDevice)
{
    BUPnPFree(pDevice);
    pDevice = NULL;
}

BUPnPRemoteDeviceHandle BUPnPControlPoint_CreateDevice(const char* uuid)
{
    BUPnPDeviceHandle hDevice;
    BUPnPRemoteDevice* hNewDevice = (BUPnPRemoteDevice*)BUPnPCalloc(1, sizeof(BUPnPRemoteDevice));

    bthread_mutex_lock(g_ctrlPoint->deviceListMutex);
    /* We have to manually loop the list because the pointer could be gone */
    for(hDevice=BLST_FIRST(&g_ctrlPoint->deviceList); hDevice; hDevice=BLST_NEXT(hDevice, link)) 
    {
        if (MATCH(hDevice->deviceInfo->udn, uuid))
        {
            hNewDevice->device = hDevice;
            bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);
            return hNewDevice;
        }

    }
    /* cannot find out, maybe it went away already */
    bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);
    return NULL;
}

bool BUPnPControlPoint_CheckActionExists(const char* uuid, const char* serviceName, const char* actionName)
{
    BUPnPDeviceHandle hDevice;
    BUPnPServiceHandle hService;
    BUPnPActionHandle hAction;

    bthread_mutex_lock(g_ctrlPoint->deviceListMutex);

    /* it is okay if this fails. Just means it was already added during discovery to prevent duplicated downloads */
    BLST_DICT_FIND_BY_STR(&g_ctrlPoint->deviceList, hDevice, uuid, deviceInfo->udn, link);

    bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);

    if (hDevice != NULL)
    {
        hService = BLST_FIRST(&hDevice->serviceList);
        while (hService!=NULL)
        {
            if (IMATCH(hService->serviceInfo->serviceId, serviceName))
            {
                hAction = BUPnPService_GetAction(hService, actionName);
                if (hAction != NULL)
                    return true;
            }
            hService = BLST_NEXT(hService, link);
        }
    }
    return false;
}

bool BUPnPControlPoint_CheckServiceExists(const char* uuid, const char* serviceName)
{
    BUPnPDeviceHandle hDevice;
    BUPnPServiceHandle hService;

    bthread_mutex_lock(g_ctrlPoint->deviceListMutex);

    /* it is okay if this fails. Just means it was already added during discovery to prevent duplicated downloads */
    BLST_DICT_FIND_BY_STR(&g_ctrlPoint->deviceList, hDevice, uuid, deviceInfo->udn, link);

    bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);
    if (hDevice != NULL)
    {
        hService = BLST_FIRST(&hDevice->serviceList);
        while (hService!=NULL)
        {
            if (IMATCH(hService->serviceInfo->serviceId, serviceName))
                return true;

            hService = BLST_NEXT(hService, link);
        }
    }
    return false;
}

