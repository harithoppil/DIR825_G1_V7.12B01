/*****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
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
//  Filename:       upnp.c (mthawani; modified for TR64)
//
******************************************************************************/

#include <signal.h>
#include <stdarg.h>
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "linux_timer.h"
#include "tr64defs.h"

extern void send_advertisements(ssdp_t);
extern void alarm_handler(int i);
extern void gena_subscription_reaper(timer_t, void *arg);
extern void uuidstr_create(char *, int);
extern struct net_connection *make_http_socket(struct iface *);
extern struct net_connection *make_ssdp_socket(struct iface *);
extern void delete_all_subscriptions(void);
extern void delete_all_connections(void);
extern PDevice init_device(PDevice, PDeviceTemplate, ...);
extern void destroy_device(PDevice);
extern void schedule_advertisements(void);
extern PDevice root_devices;

extern int tr064_main();

void interrupt_handler(int i);
static struct iface *create_lan_interface(char *);
static int fd_net_set(fd_set *fds);
static void process_receive_fds(fd_set *fds);
static void connection_reaper(timer_t, void *arg);
static void block_alarm(void);
static void unblock_alarm(void);
static void generate_uuids(void);

struct net_connection *net_connections = NULL;

int global_exit_now = FALSE;
int global_reboot = FALSE;

struct iface *global_lans = NULL;

Error UPNPErrors[] = {
    { SOAP_INVALIDACTION, "Invalid Action" },
    { SOAP_INVALIDARGS, "Invalid Args" },
    { SOAP_INVALIDVAR, "Invalid Var" },
    { SOAP_ACTIONFAILED, "Action Failed" },
    { SOAP_INVALIDDEVICEUUID, "InvalidDeviceUUID" },
    { SOAP_INVALIDSERVICEID, "InvalidServiceID" },
    { SOAP_ACTIONNOTAUTHORIZED, "Unauthorized Action" },
    { 0, NULL }
};


#ifdef BCMDBG
int upnp_msg_level = 0xffffffff;
#endif

extern void setTr064IfChanged(timer_t t, void *arg);

extern pTr64PersistentData pTr64Data;


int upnp_main(char *ifname) 
{
   fd_set rfds;
   int n;
   struct  itimerspec  timer;
   struct iface *lanif;
   timer_t td1, td2,td3;

   UPNP_TRACE(("Entered upnp_main ifname=0x%x \"%s\"\n", 
               (uint) ifname, (ifname ? ifname : "NULL")));

   soap_register_errors(UPNPErrors);

   if ((lanif = create_lan_interface(ifname)) == NULL) 
   {
      UPNP_ERROR(("cannot use LAN interface \"%s\"\n", ifname));
      exit(1);
   }
   else
   {
      lanif->next = global_lans;
      global_lans = lanif;
   }

   UPNP_TRACE(("Generating UUIDs.\n"));
   generate_uuids();
   UPNP_TRACE(("Finished generating UUIDs.\n"));
   fflush(stdout);

   block_alarm();

   signal(SIGINT, interrupt_handler);
   signal(SIGTERM, interrupt_handler);

   memset(&timer, 0, sizeof(timer));
   timer.it_interval.tv_sec = 30;
   timer.it_value.tv_sec = 30;
   td1 = enqueue_event(&timer, (event_callback_t)connection_reaper, NULL);

   memset(&timer, 0, sizeof(timer));
   timer.it_interval.tv_sec = 30;
   timer.it_value.tv_sec = 30;
   td2 = enqueue_event(&timer, (event_callback_t)gena_subscription_reaper, NULL);

   memset(&timer, 0, sizeof(timer));
   timer.it_interval.tv_sec = 30;
   timer.it_value.tv_sec = 20;
   td3 = enqueue_event(&timer, (event_callback_t)setTr064IfChanged, NULL);

   UPNP_TRACE(("Sending initial advertisements.\n"));
   send_advertisements(SSDP_BYEBYE);
   send_advertisements(SSDP_BYEBYE);
   send_advertisements(SSDP_BYEBYE);
   send_advertisements(SSDP_ALIVE);
   send_advertisements(SSDP_ALIVE);
   send_advertisements(SSDP_ALIVE);

   schedule_advertisements();

   /* enter the top of the event loop. */
   UPNP_TRACE(("Entering upnp loop\n"));

   global_exit_now = FALSE;
   global_reboot = FALSE;
   while (global_exit_now == FALSE) 
   {
      FD_ZERO(&rfds);

      n = fd_net_set(&rfds);

      unblock_alarm();
      n = select(n, &rfds, NULL, NULL, NULL);
      block_alarm();

      if (global_exit_now)
         break;
      if (n > 0) 
         process_receive_fds(&rfds);
   }

   UPNP_TRACE(("Exiting upnp loop\n"));
  
   /* cancel timers */
   timer_delete(td1);
   timer_delete(td2);
   timer_delete(td3);

   delete_all_subscriptions();
   delete_all_connections();

   destroy_device(root_devices);

   if (pTr64Data)
   {
      free(pTr64Data);
   }

   if(global_reboot)
   {
      UPNP_TRACE(( "########## enter into reboot tr064_main()")) ;
      tr064_main();
   }
   return 0;
}


void interrupt_handler(int i)
{
   /*    signal(SIGINT, SIG_DFL); */
   send_advertisements(SSDP_BYEBYE);
   global_exit_now = TRUE;
}


/* 
   Strip characters in 'reject' from start and end of the string 'str'.
   Return a pointer to the start of the modified string.
*/
char *strip_chars(char *str, char *reject)
{
   char *end;

   /* calculate len of str with consisting of entirely "reject" */
   str += strspn(str, reject);
   /* mark the end position, store in end */
   end = &str[strlen(str)-1];
   /* starting from the end of the string, search for reject char and wipe out */
   while (end > str && strpbrk(end, reject))
      *end-- = '\0';
	
   return str;
}

static struct iface *create_lan_interface(char *ifname)
{
   struct net_connection *nc;
   struct iface *pif = NULL;

   for (pif = (struct iface *)malloc(sizeof(struct iface));
        pif != NULL;
        free(pif), pif = NULL) 
   {
      memset(pif, 0, sizeof(*pif));
      pif->ifname = ifname;

      if ( !osl_ifaddr(ifname, &pif->inaddr) ) 
      {
         UPNP_ERROR(("osl_ifaddr failed.\n"));
         continue;
      }

      if ((nc = make_http_socket(pif)) == NULL)
      {
         continue;
      }

      if ((nc = make_ssdp_socket(pif)) == NULL) 
      {
         /* need to destroy the http connection here... */
         continue;
      }
      pif->ssdp_connection = nc;

      break;
   }

   return pif;
}


void reinit_lan_interface(struct iface *pif)
{
   struct net_connection *nc;

   do 
   {
      delete_all_connections();
	
      if ( !osl_ifaddr(pif->ifname, &pif->inaddr) ) 
      {
         UPNP_ERROR(("osl_ifaddr failed.\n"));
         continue;
      }
	
      if ((nc = make_http_socket(pif)) == NULL) 
      {
         continue;
      }
      
      if ((nc = make_ssdp_socket(pif)) == NULL) 
      {
         /* need to destroy the http connection here... */
         continue;
      }
      pif->ssdp_connection = nc;
   } while (0);
}

/** Construct the bitmask of file descriptors that will be passed to select(2).
 *  For each network connection that we are maintaining, set the
 *  appropriate bit in a mask that will be used with a BSD-style select
 *  call.
 */
static int fd_net_set(fd_set *fds)
{
   struct net_connection *net;
   int max = 0;

   for (net = net_connections; net; net = net->next) 
   {
      if (net->fd > max)
         max = net->fd;
      FD_SET(net->fd, fds);
   }

   return (max + 1);
}

static void process_receive_fds(fd_set *fds)
{
   struct net_connection *net, *next;
   
   for (net = net_connections; net; net = next) 
   {
      next = net->next;
      if (FD_ISSET(net->fd, fds)) 
      {
         assert(net->func);
         (*(net->func))(CONNECTION_RECV, net, net->arg);
      }
   }
}

/* Big hammer to delete all existing connections. 
   Typically this should only be used in preparation for shutting down the UPNP server. 
*/
void delete_all_connections()
{
   struct net_connection **pnet, *net;

   pnet = &net_connections; 
   while (*pnet) 
   {
      net = *pnet;
      *pnet = (*pnet)->next;
      assert(net->func);

      (*(net->func))(CONNECTION_DELETE, net, net->arg);
   }
}

void remove_net_connection(int fd)
{
   struct net_connection *net, **pnet;

   pnet = &net_connections;
   while (*pnet) 
   {
      if ((*pnet)->fd == fd) 
      {
         net = *pnet;
         *pnet = (*pnet)->next;
         assert(net->func);
         (*(net->func))(CONNECTION_DELETE, net, net->arg);
         break;
      }
      pnet = &((*pnet)->next);
   }
}

static void connection_reaper(timer_t t, void *arg)
{
   struct net_connection **pnet, *net;
   time_t now;
   int i = 0;

   now = time(NULL);
   pnet = &net_connections;
   while (*pnet) 
   {
      i++;

      if ((*pnet)->expires && (*pnet)->expires < now) 
      {
         net = *pnet;
         *pnet = (*pnet)->next;
         assert(net->func);
         (*(net->func))(CONNECTION_DELETE, net, net->arg);
         continue;
      }
      pnet = &((*pnet)->next);
   }
}

timer_t enqueue_event(struct itimerspec *value, event_callback_t func, void *arg)
{
   timer_t          td;

   timer_create(CLOCK_REALTIME, NULL, &td);
   timer_connect(td, (void*)func, (int) arg);
   timer_settime(td, 0, value, NULL);

   return(td);
}


static void block_alarm(void)
{
   sigset_t set;

   sigemptyset(&set);
   sigaddset(&set, SIGALRM);
   sigprocmask(SIG_BLOCK, &set, NULL);
}

static void unblock_alarm(void)
{
   sigset_t set;

   sigemptyset(&set);
   sigaddset(&set, SIGALRM);
   sigprocmask(SIG_UNBLOCK, &set, NULL);
}


// Create the unique device identifiers for each device.
static void generate_uuids(void)
{
   char *udn;
   PDevice pdev;

   forall_devices(pdev) {
      udn = malloc(50);
      strcpy(udn, "uuid:");
      uuidstr_create(udn+5, 50 - 5);
    
      /** the device definition contains a unique string that should be substituted whenever 
       * the http server sends out the XML description document.
       */
      pdev->udn = udn;
   }
}

static void delayed_call_helper(timer_t t, void *arg)
{
   voidfp_t f = (voidfp_t) arg;

   timer_delete(t);
   (*f)();
}

void delayed_call(uint seconds, voidfp_t f)
{
   struct  itimerspec  timer;

   memset(&timer, 0, sizeof(timer));
   timer.it_interval.tv_sec = 0;
   timer.it_value.tv_sec = seconds;
   enqueue_event(&timer, (event_callback_t)delayed_call_helper, f);
}
