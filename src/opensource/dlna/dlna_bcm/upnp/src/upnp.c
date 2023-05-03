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
 * $brcm_Workfile: upnp.c $
 * $brcm_Revision: 6 $
 * $brcm_Date: 9/30/09 4:21p $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: /AppLibs/upnp/src/upnp.c $
 * 
 * 6   9/30/09 4:21p dliu
 * SW7405-2482: Fix bug in removing expire timer
 * 
 * 5   9/2/09 10:56a dliu
 * SW7405-2482: Add interface to UPnP Settings
* 
***************************************************************************/
#include "upnp_priv.h"
#include "device.h"
#include "debug.h"
#include "bsocket.h"
#include "service.h"
#include "device.h"
#include "http.h"
#include "ssdp.h"
#include "list.h"
#include "time.h"
#include "heap.h"

/* Add by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
#include "controlpoint.h"
#include "cms_msg.h"
#include "cms.h"
#include "cms_log.h"
#include "cms_util.h"
#include "cms_core.h"

#define UPNP_DM_ARG_HEADER "<?xml version=\"1.0\" encoding=\"UTF-8\"?><cms:%s xmlns:cms=\"urn:schemas-upnp-org:dm:cms\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:schemas-upnp-org:dm:cms http://www.upnp.org/schemas/dm/cms.xsd\">"
#endif
/* Add by tuhanyu, end */

#define UPNP_STOP_TIMEOUT              5
#define UPNP_CONNECTION_TIMEOUT         30

typedef struct BUPnPThread
{
    bthread* threadId;
    BLST_ENTRY(BUPnPThread) link;
}BUPnPThread;

typedef struct BUPnPModule
{
    bthread* hThread;

    int32_t exitFlag;

    char hostInfo[UPNP_MAX_LINE_SIZE];

    Url url;

    struct BUPnPDeviceList deviceList;
    BLST_HEAD(Connections, BUPnPConnection) connections;
    BLST_HEAD(ThreadList, BUPnPThread) threads;
}BUPnPModule;

static void BUPnP_AdvertiseTimerCallback(TimerHandle handle, void *arg);
static bool BUPnP_NewConnectionHandler(BUPnPConnectionHandle hConnection, void* args);
static int BUPnP_GetDescriptors(fd_set *fds);
static void BUPnP_ProcessReceive(fd_set *fds);
static void BUPnP_RemoveExpiredConnections(void);
static void BUPnP_ThreadProc(void *pParam);
static void BUPnP_RemoveExpiredThread(void);

static struct BUPnPModule* g_upnpModule = NULL;

extern bool Http_RequestHandler(BUPnPConnectionHandle hConnection, void* args);
extern bool Ssdp_RequestHandler(BUPnPConnectionHandle hConnection, void* args);

/* make this into structure with ControlpointHandle */
BUPnPError BUPnP_Initialize(BUPnPOpenSettings* pSettings)
{
    char host[32];
    struct utsname uts;

    assert(g_upnpModule == NULL);
    BUPnPHeapInitialize();

    g_upnpModule = (struct BUPnPModule*) BUPnPCalloc(1, sizeof(struct BUPnPModule));
    if ( g_upnpModule == NULL )
        return UPNP_E_OUT_OF_MEMORY;

    if ( pSettings->ipAddress == NULL )
    {
        host_getipaddress(pSettings->networkInterface, host, 31);
        g_upnpModule->url.host = BUPnPStrndup(host, 31);
    }
    else 
        g_upnpModule->url.host = BUPnPStrdup(pSettings->ipAddress);
    if (g_upnpModule->url.host == NULL)
        return UPNP_E_OUT_OF_MEMORY;

    g_upnpModule->url.port = pSettings->portNumber;

    uname(&uts);
    snprintf(g_upnpModule->hostInfo, UPNP_MAX_LINE_SIZE-1, 
                "%s/%s UPnP/%s Broadcom UPNP/%s", uts.sysname, uts.version, UPNP_VERSION, BLIB_VERSION); 

    Timer_Initialize();

    return UPNP_E_SUCCESS;
}

void BUPnP_Close()
{
    BUPnPDeviceHandle hDevice;

    if (!g_upnpModule->exitFlag)
        BUPnP_Stop();


    while( (hDevice = BLST_FIRST(&g_upnpModule->deviceList)) != NULL )
    {
        BLST_REMOVE_HEAD(&g_upnpModule->deviceList, link);
        BUPnPDevice_Destroy(hDevice);
    }

    Timer_Close();
    BUPnP_RemoveExpiredThread();

    BUPnPFree(g_upnpModule->url.host);
    BUPnPFree(g_upnpModule);

    BUPnPHeapClose();

    g_upnpModule = NULL;
}

BUPnPError BUPnP_Start()
{
    g_upnpModule->exitFlag = false;

    g_upnpModule->hThread = bthread_create(BUPnP_ThreadProc, NULL);
    if ( g_upnpModule->hThread == NULL )
        return UPNP_E_THREAD;

    return UPNP_E_SUCCESS;
}

BUPnPError BUPnP_Stop()
{
    g_upnpModule->exitFlag = true;

    Ssdp_SendAdvertisement(SsdpAdvertisementType_ByeBye, NULL);

    /* Remove the http listener.  This will interrupt the and force an exit */
    BUPnP_RemoveConnection( BLST_FIRST(&g_upnpModule->connections) );

    /* can't call bthread_join in Linux because select function won't get 
     * interruped by socket close */
    /* result = bthread_join(g_upnpModule->hThread, UPNP_STOP_TIMEOUT*1000);
    bthread_destroy(g_upnpModule->hThread);
    return (result == 0) ? UPNP_E_SUCCESS : UPNP_E_THREAD;
    */
    return UPNP_E_SUCCESS;
}

BUPnPDeviceHandle BUPnP_RegisterDevice(struct BUPnPDeviceInfo* pBUPnPDeviceInfo)
{
    BUPnPDeviceHandle hDevice;
    
    hDevice = BUPnPDevice_Create(pBUPnPDeviceInfo);
    if ( hDevice == NULL )
        return NULL;

    BLST_INSERT_HEAD(&g_upnpModule->deviceList, hDevice, link);

    /* server is already running send alive just for this device */
    if (g_upnpModule->hThread != NULL)
    {
        Ssdp_SendAdvertisement(SsdpAdvertisementType_Alive, hDevice);
        bsleep(200);
        Ssdp_SendAdvertisement(SsdpAdvertisementType_Alive, hDevice);
        bsleep(200);
        Ssdp_SendAdvertisement(SsdpAdvertisementType_Alive, hDevice);
    }
    return hDevice;
}

void BUPnP_UnregisterDevice(BUPnPDeviceHandle hDevice)
{
     /* If server is still runnning send byebye for just this device here */
    if (g_upnpModule->exitFlag == false)
    {
        Ssdp_SendAdvertisement(SsdpAdvertisementType_ByeBye, hDevice);
        bsleep(200);
        Ssdp_SendAdvertisement(SsdpAdvertisementType_ByeBye, hDevice);
    }

    BLST_REMOVE(&g_upnpModule->deviceList, hDevice, BUPnPDevice, link);
    BUPnPDevice_Destroy(hDevice);
}

void BUPnP_RegisterThread(bthread* hThread)
{
    BUPnPThread* thread = BUPnPCalloc(1, sizeof(BUPnPThread));
    thread->threadId = hThread;
    BLST_INSERT_HEAD(&g_upnpModule->threads, thread, link);
}

const char* BUPnP_GetIPAddress()
{
    return g_upnpModule->url.host;
}

uint32_t BUPnP_GetPortNumber()
{
    return g_upnpModule->url.port;
}

const char* BUPnP_GetDescription()
{
    return g_upnpModule->hostInfo;
}

BUPnPDeviceListHandle BUPnP_GetDeviceList()
{
    return &g_upnpModule->deviceList;
}

BUPnPConnectionHandle BUPnP_AddConnection( SOCKET s, int32_t timeout, BUPnPReceiveCallback callback, void* args )
{
    BUPnPConnectionHandle pConnection;

    pConnection = (BUPnPConnectionHandle)BUPnPCalloc(1, sizeof(*pConnection));
    if ( pConnection == NULL )
        return NULL;

    Http_CreateContext(&pConnection->context);
        
    pConnection->socket = s;
    pConnection->context.s = s;
    pConnection->callback = callback;
    pConnection->args = args;

    if ( timeout > 0 )
    {
        pConnection->timeout = timeout;
        pConnection->expireTime = time(NULL) + timeout;
    }

    BLST_INSERT_HEAD(&g_upnpModule->connections, pConnection, link);

    return pConnection;
}

void BUPnP_RemoveConnection(BUPnPConnectionHandle hConnection)
{
    if (hConnection == NULL)
        return;

    if (!BLST_EMPTY(&g_upnpModule->connections))
    BLST_REMOVE(&g_upnpModule->connections, hConnection, BUPnPConnection, link);

    if ( hConnection->socket > 0 )
        closesocket(hConnection->socket);


    Http_DestroyContext(&hConnection->context);

    BUPnPFree(hConnection);
    hConnection = NULL;
}

const char* BUPnP_GetRfc1123Date(char *buffer, size_t length)
{
    time_t now;
    now = time(NULL);
    strftime( buffer, length, "%a, %d %b %Y %H:%M:%S GMT", (const struct tm*)gmtime( &now ) );
    return buffer;
}

static int BUPnP_GetDescriptors(fd_set *fds)
{
    BUPnPConnectionHandle pConnection;
    SOCKET max = 0;
            
    FD_ZERO(fds);
    for ( pConnection = BLST_FIRST(&g_upnpModule->connections); pConnection; pConnection = BLST_NEXT(pConnection, link) )
    {
        FD_SET(pConnection->socket, fds);
        if ( max < pConnection->socket)
            max = pConnection->socket;
    }

    return (max + 1);
}

static void BUPnP_RemoveExpiredConnections(void)
{
    BUPnPConnectionHandle hConnection, hNextConnection;
    time_t now = time(NULL);

    for ( hConnection = BLST_FIRST(&g_upnpModule->connections); hConnection; )
    {
        hNextConnection = BLST_NEXT(hConnection, link);
        if ((hConnection->expireTime) && (hConnection->expireTime < now))
        {
            BUPnP_RemoveConnection(hConnection);
        }

        hConnection = hNextConnection;
    }
}

static void BUPnP_RemoveExpiredThread(void)
{
    int count = 0;
    BUPnPThread *thread, *nextThread;
    thread = BLST_FIRST(&g_upnpModule->threads);
   
    while(thread)
    {
        nextThread = BLST_NEXT(thread, link);
        if (thread->threadId->state == bthreadState_eStopped)
        {
            bthread_join(thread->threadId, 0);
            BLST_REMOVE(&g_upnpModule->threads, thread, BUPnPThread, link);
            BUPnPFree(thread->threadId);
            BUPnPFree(thread);
        }
        else
            count++;
        thread = nextThread;
    }

}

static void BUPnP_ProcessReceive(fd_set *fds)
{
    bool keepAlive;
    BUPnPConnectionHandle pConnection;

    for ( pConnection = BLST_FIRST(&g_upnpModule->connections); pConnection; pConnection = BLST_NEXT(pConnection, link) )
    {
        if (FD_ISSET(pConnection->socket, fds))
        {
            keepAlive = (*(pConnection->callback))(pConnection, pConnection->args);
            if ( keepAlive == false )
            {
                BUPnP_RemoveConnection(pConnection);
                break;
            }

            if ( pConnection->expireTime != 0 ) /* reset the expiration counter each time we receive some data. */
                pConnection->expireTime = time(NULL) + pConnection->timeout;
        }
    }
}

static bool BUPnP_NewConnectionHandler(BUPnPConnectionHandle hConnection, void* args)
{
    SOCKET s;
    UNUSED_ARGUMENT(args);

    s = accept(hConnection->socket, NULL, NULL);
    if (s != -1)
    {
        if ( BUPnP_AddConnection(s, UPNP_CONNECTION_TIMEOUT, Http_RequestHandler, NULL) == NULL )
            closesocket(s);
    }
    else
        UPNP_DEBUG_ERROR(BUPnPDebugCategory_Protocol, ("BUPnP - Socket Accept Error %d", errno));

    return true;
}

static void BUPnP_AdvertiseTimerCallback(TimerHandle hTimer, void *arg)
{
    UNUSED_ARGUMENT(hTimer);
    UNUSED_ARGUMENT(arg);

    /* Send out NOTIFY if we have devices registered */
    if (!BLST_EMPTY(&g_upnpModule->deviceList))
        Ssdp_SendAdvertisement(SsdpAdvertisementType_Alive, NULL);
}

/* Add by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
extern void *msgHandle;

int BUPnP_PraseArgument(char *arg, int num, BUPnPActionHandle hAction)
{
    int i;
    int ret = 0;
    char *name, *value;
    char *temp;
    BUPnPArgumentHandle hArg;

    for ( i=0; i<hAction->argumentCount; i++ )
    {
        hArg = &hAction->arguments[i];
        if (hArg->argumentInfo->attributes & BUPnPAttribute_In)
        {
            name = strstr(arg, hArg->name);
            if(!name)
            {
                printf("Can not found Argument name: %s\n", hArg->name);
                return UPNP_E_INVALID_ARG;
            }
            value = name + strlen(hArg->name);
            if(strncmp(value, "={", 2))
            {
                printf("Can not found Argument value: %s\n", hArg->name);
                return UPNP_E_INVALID_ARG;
            }
            value += 2;
            name = strchr(value, '}');
            if(!name)
            {
                printf("Invalid Argument value: %s\n", hArg->name);
                return UPNP_E_INVALID_ARG;
            }
            *name = 0;
            CString_SetText(&hArg->value, value) ;
            *name = '}';
        }
    }

    return UPNP_E_SUCCESS;
}

int BUPnP_ProcessAction(UpnpActionMsgBody *body, int numOfArg)
{
    int i;
    BUPnPDeviceHandle hDevice;
    BUPnPServiceHandle hService;
    BUPnPActionHandle hAction;
    BUPnPArgumentHandle hArg;
    int ret = 0;
    int argNum = 0;
    char paramType[64];
    char *escape;

printf( "BUPnP_ProcessAction: manufacturerOUI=%s\n", body->manufacturerOUI);
printf( "BUPnP_ProcessAction: productClass=%s\n", body->productClass);
printf( "BUPnP_ProcessAction: serialNumber=%s\n", body->serialNumber);
printf( "BUPnP_ProcessAction: uuid=%s\n", body->deviceUuid);
    
    bthread_mutex_lock(g_ctrlPoint->deviceListMutex);
    /* it is okay if this fails. Just means it was not added into deviceList . */
    if(strlen(body->deviceUuid))
    {
        BLST_DICT_FIND_BY_STR(&g_ctrlPoint->deviceList, hDevice, body->deviceUuid , deviceInfo->udn, link);
    }
    else
    {
        BLST_DICT_FIND_BY_PARAM(&g_ctrlPoint->deviceList, hDevice, body , link);
    }
    bthread_mutex_unlock(g_ctrlPoint->deviceListMutex);

    if(!hDevice)
    {
        printf("BUPnP_ProcessAction: Invalid Device.\n");
        return UPNP_E_NO_DEVICE;
    }
    printf("BUPnP_ProcessAction: find device\n");

    for(hService = BLST_FIRST(&hDevice->serviceList); hService; hService = BLST_NEXT(hService, link))
    {
        if(!strcmp(body->servicename, hService->serviceInfo->serviceId))
        {
            break;
        }
    }

    if(!hService)
    {
        printf("BUPnP_ProcessAction: Invalid Service: %s\n", body->servicename);
        return UPNP_E_SERVICE_NOT_FOUND;
    }

    printf("BUPnP_ProcessAction: find service\n");
    for(hAction = BLST_FIRST(&hService->actions); hAction; hAction = BLST_NEXT(hAction, link))
    {
        if(!strcmp(body->actionname, hAction->name))
        {
            printf("BUPnP_ProcessAction: actname = %s vs %s \n", body->actionname, hAction->name);
            break;
        }
    }

    if(!hAction)
    {
        printf("BUPnP_ProcessAction: Invalid Action: %s\n", body->actionname);
        return UPNP_E_INVALID_ACTION;
    }

    if(numOfArg <= MAX_ARG_NUM)
    {
        for(i=0; i<MAX_ARG_NUM; i++)
        {
            if(body->arg[i].name && strlen(body->arg[i].name) && (hArg = BUPnPAction_GetArgumentByName(hAction, body->arg[i].name)))
            {
                CString_SetText(&hArg->value, body->arg[i].value) ;
            }
        }
    }
    else
    {
        if(BUPnP_PraseArgument((char *)&body->arg[0], numOfArg, hAction) != UPNP_E_SUCCESS)
        {
            return UPNP_E_INVALID_ARG;
        }
    }

    if((ret = Soap_Invoke( hDevice,hService, hAction)) != UPNP_E_SUCCESS)
    {
         printf("BUPnP_ProcessAction: Invoke Action Failed. ret=%d\n", ret);
         return ret;
    }

    memset(&(body->arg[0]), 0, sizeof(UpnpActionArg)*MAX_ARG_NUM);
    char *arg = (char *)&body->arg[0];
    for ( i=0, argNum=0; i<hAction->argumentCount; i++ )
    {
        hArg = &hAction->arguments[i];
        if (hArg->argumentInfo->attributes & BUPnPAttribute_Out)
        {
            strcpy(body->arg[argNum].name, hArg->name);
            
            if(hArg->value.bytes && (strlen(hArg->value.bytes) >= BUFLEN_1024*5))
                strncpy(body->arg[argNum].value, hArg->value.bytes, BUFLEN_1024*5-1);
            else
                strcpy(body->arg[argNum].value, (hArg->value.bytes ? hArg->value.bytes : ""));
            
            if(!strcmp(hAction->name, "GetValues"))
            {
                FILE *fp = NULL;
                if((fp = fopen("/var/UpnpGetValuesResponse", "w+")) != NULL)
                {
                    fputs((hArg->value.bytes ? hArg->value.bytes : ""), fp);
                    fclose(fp);
                }
                else
                {
                    return UPNP_E_ARGUMENT_VALUE_INVALID;
                }
            }
            argNum++;
        }
    }

    return UPNP_E_SUCCESS;
}

static void BUPnP_ProcessMsg(CmsMsgHeader *msg)
{

   UpnpActionMsgBody *UpnpBody = (UpnpActionMsgBody *) (msg+1);
   int ret;
   char *replybuf;
   CmsMsgHeader *replymsg;
   UpnpActionMsgBody *replyinfo;

   if(!(replybuf = (char *)malloc(sizeof(CmsMsgHeader) + sizeof(UpnpActionMsgBody))))
   {
       printf("BUPnPControlPoint_SyncServiceAvaible: Can not alloc memory.\n");
       return;
   }
   memset(replybuf, 0, sizeof(CmsMsgHeader) + sizeof(UpnpActionMsgBody));
   replymsg = (CmsMsgHeader *) replybuf;
   replyinfo = (UpnpActionMsgBody *) &(replybuf[sizeof(CmsMsgHeader)]);
    
   ret = BUPnP_ProcessAction(UpnpBody, msg->wordData);

   replymsg->dst = msg->src;
   replymsg->src = msg->dst;
   replymsg->type = msg->type;
   replymsg->flags_bounceIfNotRunning = msg->flags_bounceIfNotRunning;
   replymsg->flags_request = 0;
   replymsg->flags_response = 1;
   replymsg->sequenceNumber = msg->sequenceNumber;
   replymsg->dataLength = sizeof(UpnpActionMsgBody);
   
   if(UPNP_E_SUCCESS == ret)
   {
         printf("BUPnP_ProcessMsg: UPnP invoke action success.\n");
         memcpy(replyinfo, UpnpBody, sizeof(UpnpActionMsgBody));
   }
   else
   {
         printf("BUPnP_ProcessMsg: UPnP invoke action failed.\n");
         strcpy(replyinfo->deviceUuid, UpnpBody->deviceUuid);
         strcpy(replyinfo->manufacturerOUI, UpnpBody->manufacturerOUI);
         strcpy(replyinfo->productClass, UpnpBody->productClass);
         strcpy(replyinfo->serialNumber, UpnpBody->serialNumber);
         strcpy(replyinfo->servicename, UpnpBody->servicename);
         strcpy(replyinfo->actionname, UpnpBody->actionname);
         strcpy(replyinfo->arg[0].name, "errorCode");
         sprintf(replyinfo->arg[0].value, "%d", ret);
    }

    if ((ret = cmsMsg_send(msgHandle, replymsg)) != CMSRET_SUCCESS)
    {
        cmsLog_error("send response for msg 0x%x failed, ret=%d", replymsg->type, ret);
    }

    free(replybuf);
    return;
}

static void BUPnP_InitProxyDeviceInfo()
{
    int ret;
    char buf[sizeof(CmsMsgHeader)]={0};
    CmsMsgHeader *msg = (CmsMsgHeader *) buf;
 
    msg->type = CMS_MSG_UPNPD_UPDATE_DEVICE;
    msg->src = EID_DMSD;
    msg->dst = EID_SSK;
    msg->flags_request = 1;
    msg->dataLength = 0;
    msg->wordData = UPNP_PROXY_DEVICE_INIT;
   
    if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
    {
        cmsLog_error("send  msg to ssk 0x%x failed, ret=%d", msg->type, ret);
    }
}

void BUPnP_MsgHandle(void)
{
	CmsMsgHeader *msg=NULL;
	CmsRet ret;

	// Only the control Point can receive the msg
	if ( BUPnPControlPoint_IsEnabled() == true )
	{
		if ((ret = cmsMsg_receiveWithTimeout(msgHandle, &msg, 0)) == CMSRET_SUCCESS)
		{
			switch(msg->type)
			{
				case CMS_MSG_UPNP_DEVICE_MONITOR:
				   BUPnP_ProcessMsg(msg);
				   break;
				default:
				   break;
			}
			CMSMEM_FREE_BUF_AND_NULL_PTR(msg);
		}
	}

}
#endif
/* Add by tuhanyu, end */

static void BUPnP_ThreadProc(void *pParam)
{
    SOCKET s;
    int result, n=0;
    fd_set rfds;
    struct timeval timeout;
    TimerHandle hTimer;
    TimerSettings timerSettings;
    BUPnPConnectionHandle hConnection;
    UNUSED_ARGUMENT(pParam);

    UPNP_DEBUG_TRACE( BUPnPDebugCategory_All, ("Starting BUPnP Host '%s'...", BUPnP_GetDescription() ));

    s = BSocket_CreateTcpListener(g_upnpModule->url.host, g_upnpModule->url.port);
    if ( s <= 0 )
        perror("Socket Error");
    BUPnP_AddConnection(s, 0, BUPnP_NewConnectionHandler, NULL);

    s = BSocket_CreateUdpListener(SSDP_ADDRESS, SSDP_PORT);
    if ( s <= 0 )
        perror("Socket Error");
    BUPnP_AddConnection(s, 0, Ssdp_RequestHandler, NULL);

    timerSettings.period = ((SSDP_REFRESH * 2)/3)*1000;
    timerSettings.callback = BUPnP_AdvertiseTimerCallback;
    timerSettings.executeOnce = false;
    hTimer = Timer_Create(&timerSettings);
    if ( hTimer ==  NULL )
        perror("Out Of Memory");

/* Modify by tuhanyu, for upnp device management of separate, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
    BUPnP_InitProxyDeviceInfo();

    if ( BUPnPControlPoint_IsEnabled() == true )
    {
        timerSettings.period = 30*1000;
        timerSettings.callback = BUPnPControlPoint_SearchDevices;
        timerSettings.executeOnce = true;
        hTimer = Timer_Create(&timerSettings);
        if ( hTimer ==  NULL )
            perror("Out Of Memory");
    }
#else
    if ( BUPnPControlPoint_IsEnabled() == true )
        BUPnPControlPoint_SearchDevices();
#endif
/* Modify by tuhanyu, end */

    if (!BLST_EMPTY(&g_upnpModule->deviceList))
    {
        if ((result = Ssdp_SendStartupAdvertisement()) != UPNP_E_SUCCESS)
        {
            UPNP_DEBUG_TRACE( BUPnPDebugCategory_All, ("SSDP initial advertisment failed. Error = %d", result));
            return;
        }
    }

    timeout.tv_sec = UPNP_CONNECTION_TIMEOUT;
    timeout.tv_usec = 0;

    g_upnpModule->exitFlag = false;
    while ( !g_upnpModule->exitFlag )
    {
        n = BUPnP_GetDescriptors(&rfds);

/* Add by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
		int  comm_fd  ;

        if(msgHandle == NULL)
        {
              printf("BUPnP_ThreadProc: msgHandle=NULL.\n");
        }
        else
        {
	      cmsMsg_getEventHandle(msgHandle, &comm_fd);
 	      n = (n>(comm_fd))?n:(comm_fd+1);
              FD_SET(comm_fd,&rfds);
        }

#endif
/* Add by tuhanyu, end */

        /* Work around problem in Linux where timeout gets reset to 0 after select */
        timeout.tv_sec = UPNP_CONNECTION_TIMEOUT;

        result = select(n, &rfds, NULL, NULL, &timeout);
/* Add by tuhanyu, for upnp device management of separate ap, 2011/04/15, start */
#ifdef DMP_X_CT_COM_UPNP_DM_CP_1
        if ((result > 0) && (FD_ISSET(comm_fd, &rfds)) ) {
            BUPnP_MsgHandle();
        }
        else
#endif
/* Add by tuhanyu, end */
        if ( !g_upnpModule->exitFlag )
        {
            Timer_BlockEvents();

            if (result > 0)
            {
                BUPnP_ProcessReceive(&rfds);
            }
            else if (result < 0)
            {
                perror("select failed\n"); /* change to DEBUG_ERROR */
                assert(0);
            }
            BUPnP_RemoveExpiredThread();
            BUPnP_RemoveExpiredConnections();

            Timer_UnblockEvents();
        }
    }

    Timer_Destroy(hTimer);

    /* Remove all connections */
    while( (hConnection = BLST_FIRST(&g_upnpModule->connections)) != NULL )
    {
        BLST_REMOVE_HEAD(&g_upnpModule->connections, link);
        if ( hConnection->socket > 0 )
            closesocket(hConnection->socket);
        Http_DestroyContext(&hConnection->context);
        BUPnPFree(hConnection);
    }

    UPNP_DEBUG_TRACE( BUPnPDebugCategory_All, ("BUPnP Host '%s' Stopped.", BUPnP_GetDescription() ));
}
