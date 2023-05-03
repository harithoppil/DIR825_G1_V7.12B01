/*----------------------------------------------------------------------*
<:copyright-broadcom 
 
 Copyright (c) 2006 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
 *----------------------------------------------------------------------*
 * File Name  : main.c 
 *
 * Description:  entry point of CPE tr64 application
 *   
 *   
 *----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>


#include "upnp.h"
#include "tr64defs.h"
#include "upnp_dbg.h"
#include "session.h"
#include "cms.h"
#include "cms_mdm.h"
#include "tr64utils.h"
#include "cmmif.h"

void *msgHandle = NULL;

pSessionInfo pCurrentSession = NULL;
tr64PersistentData *pTr64Data = NULL;

extern DeviceTemplate IGDeviceTemplate;
extern void init_event_queue(int n);
extern ST_TR064STR_LINK* getIGDSvc() ;
extern ST_TR064STR_LINK* getCftTreeSvc();
void init_static_igd_devices(void);

static void reap(int sig)
{
   pid_t pid;

   while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) 
   {
      UPNP_TRACE(("Reaped %d\n", pid));
   }
}

void getCurrentState(pTr64PersistentData *pData)
{
   uint32 len =0;
   *pData = (pTr64PersistentData)malloc(sizeof(tr64PersistentData));
   if (*pData != NULL) 
   {
         memset(*pData,0,(sizeof(tr64PersistentData)));
         (*pData)->passwordState = FACTORY;
         strcpy((*pData)->password,TR64_DSLF_RESET_PWD);
   }
} 

void setCurrentState(pTr64PersistentData pData)
{

}

int tr064_main()
{
   CMM_Init();

   //Ignore SIGPIPE signals
   signal(SIGPIPE, SIG_IGN);
   
   init_event_queue(TR64_NUMBER_EVENT_QUEUE);
   
   signal(SIGCHLD, reap);
   
   getCurrentState(&pTr64Data);

   /* init devices, services, sub-devices and their services */ 
   init_static_igd_devices();

   upnp_main(TR64_LAN_INTF_NAME);

   return 0;
}


int main(int argc, char** argv)
{  
   return tr064_main();
}

