/* -*- Mode: C; c-basic-offset: 3 -*-
    Copyright (C) 2002-2005  Thomas Ries <tries@gmx.net>

    This file is part of Siproxd.

    Siproxd is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Siproxd is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Siproxd; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef	HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "log.h"

/*add by xuezhongbo for zte_binary_body on 20090917*/
char bodymessage[1500];
int bodymessagelen = 0;
/*end add*/

static char const ident[]="$Id: siproxd.c,v 1.57 2005/01/08 10:41:46 hb9xar Exp $";

/*added by xuezhongbo for ALG 2007.10.30*/
/*record for the first unregister request when it was the first request in whole scope*/
//struct RecordOffstUnregister ROfU[URLMAP_SIZE];
/*end added*/

/*added by xuezhongbo for ALG 2007.11.2*/
/* mod by weigy on 20100531 for the multi wan route select */
#if MULTI_WAN
struct SockControlTable SCTable[SCTMAX_SIZE]=
{
{0,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL},
{1,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL},
{2,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL},
{3,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL},
{4,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL},
{5,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL},
{6,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL},
{7,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL},
{8,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL},
{ENDINDEX,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL}
};
#else
struct SockControlTable SCTable[SCTMAX_SIZE]=
{
{0,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL},
{1,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL},
{2,UNKNOWNSOCK,ADDPORT1,SockClosed,NULL},
{3,UNKNOWNSOCK,ADDPORT1,SockClosed,NULL},
{4,UNKNOWNSOCK,ADDPORT2,SockClosed,NULL},
{5,UNKNOWNSOCK,ADDPORT2,SockClosed,NULL},
{ENDINDEX,UNKNOWNSOCK,DEFAULTPORT,SockClosed,NULL}
};
#endif
/* mod end */

extern int sip_udp_socket;
/*end added*/

/*added by xuezhongfor ALG v2 2007.11.07:9:20*/
#ifdef IP_PKTINFO
struct in_pktinfo pktp;
#else
{
#if defined(IP_RECVDSTADDR) || defined(IP_RECVIF)
struct in_indexinfo pktp;
#endif
}
#endif
/*end added*/

/* configuration storage */
struct siproxd_config configuration;

/* Global File instance on pw file */
FILE *siproxd_passwordfile;

/* -h help option text */
static const char str_helpmsg[] =
PACKAGE "-" VERSION "-" BUILDSTR " (c) 2002-2005 Thomas Ries\n"
"\nUsage: siproxd [options]\n\n"
"options:\n"
#ifdef	HAVE_GETOPT_LONG
"       -h, --help                 help\n"
"       -d, --debug <pattern>      set debug-pattern\n"
"       -c, --config <cfgfile>     use the specified config file\n"
"       -p, --pid-file <pidfile>   create pid file <pidfile>\n"
#else
"       -h              help\n"
"       -d <pattern>    set debug-pattern\n"
"       -c <cfgfile>    use the specified config file\n"
"       -p <pidfile>    create pid file <pidfile>\n"
#endif
"";



/*
 * module local data
 */
static  int dmalloc_dump=0;
static  int exit_program=0;

/*
 * local prototypes
 */
/*add by xuezhongbo to reboot sip proxy for renew internet info on 20091008*/
static void closeallsipsock();
static int resetinterface();
/*end add*/
static void sighandler(int sig);
void TrunOffSCTableState(); //added by xuezhongbo 2007.11.2

/*add by xuezhongbo for filter stun on20080424*/
#include <netdb.h>

extern char **environ;
int bcmSystem (char *command) {
   int pid = 0, status = 0;
   char *newCommand = NULL;

   if ( command == 0 )
      return 1;

   pid = fork();
   if ( pid == -1 )
      return -1;

   if ( pid == 0 ) {
      char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = command;
      argv[3] = 0;

      execve("/bin/sh", argv, environ);
      exit(127);
   }

   /* wait for child process return */
   do {
      if ( waitpid(pid, &status, 0) == -1 ) {
         if ( errno != EINTR )
            return -1;
      } else
         return status;
   } while ( 1 );

   return status;
}

static char httpaddr[32][32];

/***************************************************************************
// Function Name: BcmNtwk_GetIpv4ByFQDN
// Description  : get ip addr in dot notation for given hostname
// Parameters   : ipinfo - array to save etc/hosts's ip addr in standard of
                           dot notation
//              : fqdn - name of server, for example: "ip.xten.net"
// Returns      : when do get ip addr of given hostname, return -1, else
                  return 0
****************************************************************************/
static int BcmNtwk_GetIpv4ByFQDN(char (*ipinfo)[32],char *fqdn)
{
    char *ptr,**pptr;
    struct hostent *hptr;
    char str[32];

    ptr = fqdn;

    if ( (hptr = gethostbyname(ptr) ) == NULL )
    {
        printf("gethostbyname error for host:%s\n", ptr);
        return -1;
    }

    printf("official hostname:%s\n",hptr->h_name);

    for(pptr = hptr->h_aliases; *pptr != NULL; pptr++)
        printf("  alias:%s\n",*pptr);

    switch(hptr->h_addrtype)
    {
        case AF_INET:
        case AF_INET6:
            pptr=hptr->h_addr_list;

            for(;*pptr!=NULL;pptr++)
            {
                printf("  address:%s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
                printf("  address:%s\n",str);
	            memcpy(*ipinfo,str,strlen(str));
	            ipinfo++;
            }
            break;
        default:
            printf("unknown address type\n");
        break;
    }
    return 0;
}

static void BcmNtwk_FilterIpaddrOnForwardChain(char (*ipinfo)[32])
{
#define IFC_LARGE_LEN           264
     char cmd[IFC_LARGE_LEN];
     while(strlen(*ipinfo)!=0)
     {
        sprintf(cmd, "iptables -I FORWARD  1 -d %s -p udp  -j DROP 2>/dev/null",
             *ipinfo);
        bcmSystem(cmd);
		sprintf(cmd, "iptables -I FORWARD  1 -d %s -p tcp  -j DROP 2>/dev/null",
             *ipinfo);
        bcmSystem(cmd);
        ipinfo++;
     }
     return;
}
/*end add*/

/* add by weigy on 20100531 for the multi wan route select */
#if MULTI_WAN
int wan_name_is_exist(char *name)
{
    int idx;

    if (!name)
        return TRUE;

    for (idx=0; idx<MAX_WAN_NUM; idx++)
    {
        if (configuration.wan_name[idx] != NULL &&
            strcmp(name, configuration.wan_name[idx]) == 0)
            return TRUE;
    }

    return FALSE;
}

int parse_outbound_if()
{
    int idx;

    configuration.numofoutbound = 0;

    for(idx=0; idx<MAX_WAN_NUM; idx++)
    {
        if (configuration.wan_name[idx] != NULL &&
            strlen(configuration.wan_name[idx]) != NULL &&
            (configuration.wan_type[idx] == CTYPE_INTERNET ||
            configuration.wan_type[idx] == CTYPE_VOIP_INTERNET ||
            configuration.wan_type[idx] == CTYPE_TR069_INTERNET ||
            configuration.wan_type[idx] == CTYPE_TR069_VOIP_INTERNET))
        {
            strcpy(configuration.outbound[configuration.numofoutbound].name,
                configuration.wan_name[idx]);
            get_ip_by_ifname(configuration.outbound[configuration.numofoutbound].name,
                &configuration.outbound[configuration.numofoutbound].addr);
            configuration.outbound[configuration.numofoutbound].lan = configuration.wan_lan[idx];
            configuration.numofoutbound++;

            DEBUGC(DBCLASS_SIP, "parse_outbound_if1 name=%s, type=%d, lan=0x%x, num=%d",
                configuration.outbound[configuration.numofoutbound-1].name,
                configuration.wan_type[idx], configuration.outbound[configuration.numofoutbound-1].lan,
                configuration.numofoutbound-1);
        }
    }

    for(idx=0; idx<MAX_WAN_NUM; idx++)
    {
        if (configuration.wan_name[idx] != NULL &&
            strlen(configuration.wan_name[idx]) != NULL &&
            configuration.wan_lan[idx] != 0)
        {
            if (wan_name_is_exist(configuration.wan_name[idx]))
                continue;

            strcpy(configuration.outbound[configuration.numofoutbound].name,
                configuration.wan_name[idx]);
            get_ip_by_ifname(&configuration.outbound[configuration.numofoutbound].name,
                &configuration.outbound[configuration.numofoutbound].addr);
            configuration.outbound[configuration.numofoutbound].lan = configuration.wan_lan[idx];
            configuration.numofoutbound++;

            DEBUGC(DBCLASS_SIP, "parse_outbound_if2 name=%s, type=%d, lan=0x%x, num=%d",
                configuration.outbound[configuration.numofoutbound-1].name,
                configuration.wan_type[idx], configuration.outbound[configuration.numofoutbound-1].lan,
                configuration.numofoutbound-1);
        }
    }

    return configuration.numofoutbound;
}

char *find_outboundif(int lan)
{
    int shift_lan = 1<<(lan+1);
    int idx;

    for (idx=0; idx<configuration.numofoutbound; idx++)
    {
        if (configuration.outbound[idx].lan & shift_lan)
        {
            DEBUGC(DBCLASS_SIP, "find_outboundif1 outboundif[%d] is %s", idx, configuration.outbound[idx].name);
            return configuration.outbound[idx].name;
        }
    }

    DEBUGC(DBCLASS_SIP, "find_outboundif2 outboundif[0] is %s", configuration.outbound[0].name);
    return configuration.outbound[0].name;
}

int find_outboundidx(int lan)
{
    int shift_lan = 1<<(lan+1);
    int idx;

    for (idx=0; idx<configuration.numofoutbound; idx++)
    {
        if (configuration.outbound[idx].lan & shift_lan)
        {
            DEBUGC(DBCLASS_SIP, "find_outboundidx1 idx is %d", idx);
            return idx;
        }
    }

    DEBUGC(DBCLASS_SIP, "find_outboundidx2 idx is 0");
    return 0;
}

char* get_if_from_socket(int socket)
{
    int idx;

    for (idx=0; idx<SCTMAX_SIZE; idx++)
    {
        if (SCTable[idx].sockfd == socket)
        {
            if (idx == 0)
                return configuration.inbound_if;
            else
                return configuration.outbound[idx-1].name;
        }
    }

    return NULL;
}

#else

char* get_if_from_socket(int socket)
{
    int idx;

    for (idx=0; idx<SCTMAX_SIZE; idx++)
    {
        if (SCTable[idx].sockfd == socket)
        {
            if (idx%2 == 0)
                return configuration.inbound_if;
            else
                return configuration.outbound_if;
        }
    }

    return NULL;
}
#endif
/* add end */


#ifdef BUILD_STATIC
int siproxd_main(int argc, char *argv[])
#else
int main (int argc, char *argv[])
#endif
{
   int sts;
   int i;
   int msgend = 0;
   int access;
   char buff [BUFFER_SIZE];
   sip_ticket_t ticket;

   extern char *optarg;		/* Defined in libc getopt and unistd.h */
   int ch1;

   char configfile[64]="siproxd";	/* basename of configfile */
   int  config_search=1;		/* search the config file */
   int  cmdline_debuglevel=0;
   char *pidfilename=NULL;
   struct sigaction act;
   char szRecvDev[64];
   int uiLen;

   log_set_stderr(1);

/*
 * setup signal handlers
 */
   act.sa_handler=sighandler;
   sigemptyset(&act.sa_mask);
   act.sa_flags=SA_RESTART;
   if (sigaction(SIGTERM, &act, NULL)) {
      ERROR("Failed to install SIGTERM handler");
   }
   if (sigaction(SIGINT, &act, NULL)) {
      ERROR("Failed to install SIGINT handler");
   }
   if (sigaction(SIGUSR2, &act, NULL)) {
      ERROR("Failed to install SIGUSR2 handler");
   }


/*
 * prepare default configuration
 */
   make_default_config();

   log_set_pattern(configuration.debuglevel);

/*
 * open a the pwfile instance, so we still have access after
 * we possibly have chroot()ed to somewhere.
 */
   if (configuration.proxy_auth_pwfile) {
      siproxd_passwordfile = fopen(configuration.proxy_auth_pwfile, "r");
   } else {
      siproxd_passwordfile = NULL;
   }

/*
 * parse command line
 */
{
#ifdef	HAVE_GETOPT_LONG
   int option_index = 0;
   static struct option long_options[] = {
      {"help", no_argument, NULL, 'h'},
      {"config", required_argument, NULL, 'c'},
      {"debug", required_argument, NULL, 'd'},
      {"pid-file", required_argument, NULL,'p'},
      {0,0,0,0}
   };

    while ((ch1 = getopt_long(argc, argv, "hc:d:p:",
                  long_options, &option_index)) != -1) {
#else	/* ! HAVE_GETOPT_LONG */
    while ((ch1 = getopt(argc, argv, "hc:d:p:")) != -1) {
#endif
      switch (ch1) {
      case 'h':	/* help */
         DEBUGC(DBCLASS_CONFIG,"option: help");
         fprintf(stderr,str_helpmsg);
         exit(0);
	 break;

      case 'c':	/* load config file */
         DEBUGC(DBCLASS_CONFIG,"option: config file=%s",optarg);
         i=sizeof(configfile)-1;
         strncpy(configfile,optarg,i-1);
	 configfile[i]='\0';
	 config_search=0;
	 break;

      case 'd':	/* set debug level */
         DEBUGC(DBCLASS_CONFIG,"option: set debug level: %s",optarg);
	 cmdline_debuglevel=atoi(optarg);
         log_set_pattern(cmdline_debuglevel);
	 break;

      case 'p':
	 pidfilename = optarg;
	 break;

      default:
         DEBUGC(DBCLASS_CONFIG,"no command line options");
	 break;
      }
   }
}

/*
 * Init stuff
 */
   INFO(PACKAGE"-"VERSION"-"BUILDSTR" "UNAME" starting up");

   /* read the config file */
   if (read_config(configfile, config_search) == STS_FAILURE) exit(1);

   /* mod by weigy on 20100531 for the multi wan route select */
   #if MULTI_WAN
    parse_outbound_if();
    get_ip_by_ifname(configuration.inbound_if,&configuration.inboundaddr);

    for (i=0; i<configuration.numofoutbound+1; i++)
    {
        SCTable[i].udpport = configuration.sip_listen_port;
	}
   #else
   get_ip_by_ifname(configuration.outbound_if,&configuration.outboundaddr);
   get_ip_by_ifname(configuration.inbound_if,&configuration.inboundaddr);
   INFO("===outbound_if %s inbound_if %s total outbound %s sip_listen_port=%d\n",
   	configuration.outbound_if,configuration.inbound_if,configuration.totalinterface);
	SCTable[0].udpport = configuration.sip_listen_port;
	SCTable[1].udpport = configuration.sip_listen_port;
   #endif
   /* mod end */

   /* if a debug level > 0 has been given on the commandline use its
      value and not what is in the config file */
   if (cmdline_debuglevel != 0) {
      configuration.debuglevel=cmdline_debuglevel;
   }

   /* set debug level as desired */
   log_set_pattern(configuration.debuglevel);
   log_set_listen_port(configuration.debugport);

   /* change user and group IDs */
   secure_enviroment();

   /* daemonize if requested to */
   if (configuration.daemonize) {
      DEBUGC(DBCLASS_CONFIG,"daemonizing");
      if (fork()!=0) exit(0);
      setsid();
      if (fork()!=0)
      {    //"ip.xten.net"
      /*add by xuezhongbo for stun on 20080423*/
      INFO("\nLET US DO STUN\n");
          if (BcmNtwk_GetIpv4ByFQDN(httpaddr,"ip.xten.net") == 0)
          {
	          BcmNtwk_FilterIpaddrOnForwardChain(httpaddr);
          }
      /*end add*/
          exit(0);
      }
      /* commit by weigy tempiliary*/
      //log_set_stderr(0);
      INFO("daemonized, pid=%i", getpid());
   }

   /* write PID file of main thread */
   if (pidfilename == NULL) pidfilename = configuration.pid_file;
   if (pidfilename) {
      FILE *pidfile;
      DEBUGC(DBCLASS_CONFIG,"creating PID file [%s]", pidfilename);
      sts=unlink(configuration.pid_file);
      if ((sts==0) ||(errno == ENOENT)) {
         if ((pidfile=fopen(pidfilename, "w"))) {
            fprintf(pidfile,"%i\n",(int)getpid());
            fclose(pidfile);
         } else {
            WARN("couldn't create new PID file: %s", strerror(errno));
         }
      } else {
         WARN("couldn't delete old PID file: %s", strerror(errno));
      }
   }

   /* initialize the RTP proxy */
   sts=rtpproxy_init();
   if (sts != STS_SUCCESS) {
      ERROR("unable to initialize RTP proxy - aborting");
      exit(1);
   }

   /* init the oSIP parser */
   parser_init();

   /*modified by xuezhongbo for ALG 2007.11.2*/
   #if 0
   /* listen for incoming messages */
   sts=sipsock_listen();
   #else
   sts = sipsock_listen_mult();
   #endif
   /*end modified*/
   if (sts == STS_FAILURE) {
      /* failure to allocate SIP socket... */
      ERROR("unable to bind to SIP listening socket - aborting");
      exit(1);
   }

   /* initialize the registration facility */
   register_init();

   /* mod by weigy on 20100531 for the multi wan route select */
   #ifndef MULTI_WAN
   INFO("configuration.outboundaddr.s_addr 0x%x configuration.inboundaddr.s_addr  0x%x\n",
   	      configuration.outboundaddr.s_addr,configuration.inboundaddr.s_addr);
   #endif
   /* mod end */

/*
 * silence the log - if so required...
 */
   log_set_silence(configuration.silence_log);

   INFO(PACKAGE"-"VERSION"-"BUILDSTR" "UNAME" started");
    int xzbi = 0;

   SIP_PORT = (int)DEFAULTPORT;

Mainloop:
/*
 * Main loop
 */
   while (!exit_program) {

      DEBUGC(DBCLASS_BABBLE,"going into sipsock_wait\n");
      /*added by xuezhongbo for ALG 2007.11.2*/
	TrunOffSCTableState();
	/*end added*/

      /*added by xuezhongbo for ALG v2 2007.11.07:9:23*/
      #ifdef IP_PKTINFO
	memset(&pktp,0,sizeof(struct in_pktinfo));
      #else
      	{
	 #if defined(IP_RECVDSTADDR) ||  defined(IP_RECVIF)
        memset(&pktp,0,sizeof(struct in_indexinfo));
        #endif
	 }
       #endif
	/*end added*/

	/*modified by xuezhongbo for ALG 2007.11.2*/
	#if 0
      while (sipsock_wait()<=0)
         /* got no input, here by timeout. do aging */
    #else
    while (sipsock_wait_mult()<=0)
    /* got no input, here by timeout. do aging */
	#endif
	{
	/*end modified*/
	   register_agemap();

         /* TCP log: check for a connection */
         log_tcp_connect();

         /* dump memory stats if requested to do so */
         if (dmalloc_dump) {
            dmalloc_dump=0;
#ifdef DMALLOC
            INFO("SIGUSR2 - DMALLOC statistics is dumped");
            dmalloc_log_stats();
            dmalloc_log_unfreed();
#else
            INFO("SIGUSR2 - DMALLOC support is not compiled in");
#endif
         }

/*add by xuezhongbo to reboot sip proxy for renew internet info on 20091008*/
		if(resetinterface())
		{
			INFO("siproxd goto Mainloop 1\n");
			goto Mainloop;
		}
/*end add*/

        if (exit_program) goto exit_prg;
    }

    /*add by xuezhongbo for ori dst on 20080320*/
    #if 0
    bcmsystem("cat /proc/net/ip_conntrack > /var/siproxd/siproxdconntrack.log");
    #endif
    /*end add*/

/*add by xuezhongbo to reboot sip proxy for renew internet info on 20091008*/
	if(resetinterface())
	{
		INFO("siproxd goto Mainloop 2\n");
		goto Mainloop;
	}
/*end add*/

    /* got input, process */
    for(xzbi=0;xzbi<SCTMAX_SIZE;xzbi++)
    {
    /*added by xuezhongbo for ALG v2.07 2007.11.12:9:19*/
	register_agemap();
	/*end added*/

	if(SCTable[xzbi].sockindex==ENDINDEX)
	{
	  break;
	}
    else if((SCTable[xzbi].sockfd)&&(SCTable[xzbi].state&SockCanRead))
	{
//      	configuration.sip_listen_port = SCTable[xzbi].udpport;
	sip_udp_socket = SCTable[xzbi].sockfd;

	//SIP_PORT=configuration.sip_listen_port;???2007.11.12:22:35
	i=sipsock_read(&buff, sizeof(buff)-1, &ticket.from, &ticket.protocol);
      buff[i]='\0';
	DEBUGC(DBCLASS_BABBLE,"==========read port %d data %d from %s xzbi %d==========\n",sip_udp_socket,i, utils_inet_ntoa(ticket.from.sin_addr), xzbi);

	/* add by weigy on 20100531 for the multi wan route select */
    #if MULTI_WAN
    ticket.sctable_num = xzbi;
    #define SIOCGRECVIF   0x8908
    if (!ticket.sctable_num)
    {
        ioctl(sip_udp_socket, SIOCGRECVIF, szRecvDev);
        uiLen = strlen(szRecvDev);
        if (strstr(szRecvDev, "ath"))
            ticket.lan = 4;
        else
            ticket.lan = 0;
        ticket.lan += atoi(&szRecvDev[uiLen-1]);
        DEBUGC(DBCLASS_SIP, "get packet from lan=%d", ticket.lan);
    }
    #endif
    /* add end */

    /*add by xuezhongbo for zte_binary_body on 20090917*/
    memset(bodymessage,0,1500);
    bodymessagelen= 0;
	msgend = 0;
    /*end add*/

      /*added by xuezhongbo for ALG 2007.11.2*/
	/*cant read ag from the same sock in a time*/
	SCTable[xzbi].state &=~SockCanRead;
	/*end added*/

      /* evaluate the access lists (IP based filter)*/
      access=accesslist_check(ticket.from);
      if (access == 0) {
         DEBUGC(DBCLASS_ACCESS,"access for this packet was denied");
         continue; /* there are no resources to free */
      }

      /* integrity checks */
      sts=security_check_raw(buff, i);
      if (sts != STS_SUCCESS) {
         DEBUGC(DBCLASS_SIP,"security check (raw) failed");
         continue; /* there are no resources to free */
      }

      /* init sip_msg */
      sts=osip_message_init(&ticket.sipmsg);
      ticket.sipmsg->message=NULL;
      if (sts != 0) {
         ERROR("osip_message_init() failed... this is not good");
	     continue; /* skip, there are no resources to free */
      }

      /*
       * RFC 3261, Section 16.3 step 1
       * Proxy Behavior - Request Validation - Reasonable Syntax
       * (parse the received message)
       */
      sts=osip_message_parse(ticket.sipmsg, buff);
      if (sts != 0) {
/*add by xuezhongbo for zte_binary_body on 20090917*/
		 if(sts == -2)
         {
             /*check if revice a binary body*/
			 bodymessagelen = atoi(ticket.sipmsg->content_length->value);
             msgend = i - atoi(ticket.sipmsg->content_length->value);

			 DEBUGC(DBCLASS_BABBLE,"====msgend - 4 %d msgend - 3 %d msgend - 2 %d msgend - 1 %d\n",
			 	buff[msgend - 4],buff[msgend - 3],buff[msgend - 2],buff[msgend - 1]);
			 if((msgend > 4 ) &&( msgend < i))
			 {
			     if((buff[msgend - 4] == 0x0d)&&
				 	(buff[msgend - 3] == 0x0a)&&
				 	(buff[msgend - 2] == 0x0d)&&
				 	(buff[msgend - 1] == 0x0a))
			     {
			         /*ok we find the end of meaage*/
					 DEBUGC(DBCLASS_BABBLE,"ok we find the end of meaage bodymessagelen %d\n",bodymessagelen);
					 memcpy(bodymessage,&buff[msgend],bodymessagelen);
				 }
				 else
				 	bodymessagelen = 0;
			 }
			 else
			 bodymessagelen = 0;
         }
		 else
/*end add*/
		 {
			 ERROR("osip_message_parse() failed... this is not good");
	         DUMP_BUFFER(-1, buff, i);
	         goto end_loop; /* skip and free resources */
		 }
      }

      /* integrity checks - parsed buffer*/
      sts=security_check_sip(&ticket);
      if (sts != STS_SUCCESS) {
         ERROR("security_check_sip() failed... this is not good");
         DUMP_BUFFER(-1, buff, i);
         goto end_loop; /* skip and free resources */
      }

      /*
       * RFC 3261, Section 16.3 step 2
       * Proxy Behavior - Request Validation - URI scheme
       * (check request URI and refuse with 416 if not understood)
       */
      /* NOT IMPLEMENTED */

      /*
       * RFC 3261, Section 16.3 step 3
       * Proxy Behavior - Request Validation - Max-Forwards check
       * (check Max-Forwards header and refuse with 483 if too many hops)
       */
      {
      osip_header_t *max_forwards;
      int forwards_count = DEFAULT_MAXFWD;

      osip_message_get_max_forwards(ticket.sipmsg, 0, &max_forwards);
      if (max_forwards && max_forwards->hvalue) {
         forwards_count = atoi(max_forwards->hvalue);
      }

      DEBUGC(DBCLASS_PROXY,"checking Max-Forwards (=%i)",forwards_count);
      if (forwards_count <= 0) {
         DEBUGC(DBCLASS_SIP, "Forward count reached 0 -> 483 response");
         sip_gen_response(&ticket, 483 /*Too many hops*/);
         goto end_loop; /* skip and free resources */
      }

      }

      /*
       * RFC 3261, Section 16.3 step 4
       * Proxy Behavior - Request Validation - Loop Detection check
       * (check for loop and return 482 if a loop is detected)
       */
      if (check_vialoop(&ticket) == STS_TRUE) {
         /* make sure we don't end up in endless loop when detecting
          * an loop in an "loop detected" message - brrr */
         if (MSG_IS_RESPONSE(ticket.sipmsg) &&
             MSG_TEST_CODE(ticket.sipmsg, 482)) {
            DEBUGC(DBCLASS_SIP,"loop in loop-response detected, ignoring");
         } else {
            DEBUGC(DBCLASS_SIP,"via loop detected, ignoring request");
            sip_gen_response(&ticket, 482 /*Loop detected*/);
         }
         goto end_loop; /* skip and free resources */
      }

      /*
       * RFC 3261, Section 16.3 step 5
       * Proxy Behavior - Request Validation - Proxy-Require check
       * (check Proxy-Require header and return 420 if unsupported option)
       */
      /* NOT IMPLEMENTED */

      /*
       * RFC 3261, Section 16.5
       * Proxy Behavior - Determining Request Targets
       */
      /* NOT IMPLEMENTED */

      DEBUGC(DBCLASS_SIP,"received SIP type %s:%s",
	     (MSG_IS_REQUEST(ticket.sipmsg))? "REQ" : "RES",
             (MSG_IS_REQUEST(ticket.sipmsg) ?
                ((ticket.sipmsg->sip_method)?
                   ticket.sipmsg->sip_method : "NULL") :
                ((ticket.sipmsg->reason_phrase) ?
                   ticket.sipmsg->reason_phrase : "NULL")));

      /*
       * if an REQ REGISTER, check if it is directed to myself,
       * or am I just the outbound proxy but no registrar.
       * - If I'm the registrar, register & generate answer
       * - If I'm just the outbound proxy, register, rewrite & forward
       */
      if (MSG_IS_REGISTER(ticket.sipmsg) &&
          MSG_IS_REQUEST(ticket.sipmsg)) {
         if (access & ACCESSCTL_REG) {
            osip_uri_t *url;
            struct in_addr addr1, addr2, addr3;
            int dest_port;

            url = osip_message_get_uri(ticket.sipmsg);
            /*added by xuezhongbo for ALG v2.07 2007.11.12:22:44*/
            #if 0
            dest_port= (url->port)?atoi(url->port):SIP_PORT;
            #else
            dest_port= (url->port)?atoi(url->port):configuration.sip_listen_port;
            #endif
            /*end added*/
            /* mod by weigy on 20100531 for the multi wan route select */
            #if MULTI_WAN
            if ( (get_ip_by_host(url->host, &addr1) == STS_SUCCESS) &&
                 (get_ip_by_ifname(configuration.inbound_if,&addr2) == STS_SUCCESS) &&
                 (get_ip_by_ifname(ticket.sctable_num ? configuration.outbound[ticket.sctable_num-1].name :
                    find_outboundif(ticket.lan), &addr3) == STS_SUCCESS))
            {
            #else
            if ( (get_ip_by_host(url->host, &addr1) == STS_SUCCESS) &&
                 (get_ip_by_ifname(configuration.inbound_if,&addr2) ==
                  STS_SUCCESS) &&
                 (get_ip_by_ifname(configuration.outbound_if,&addr3) ==
                  STS_SUCCESS)) {
            #endif
            /* mod end */

               if ((configuration.sip_listen_port == dest_port) &&
                   ((memcmp(&addr1, &addr2, sizeof(addr1)) == 0) ||
                    (memcmp(&addr1, &addr3, sizeof(addr1)) == 0))) {
                  /* I'm the registrar, send response myself */
                  sts = register_client(&ticket, 0);
                  sts = register_response(&ticket, sts);
               } else {
                  /* I'm just the outbound proxy */
                  DEBUGC(DBCLASS_SIP,"proxying REGISTER request to:%s",
                         url->host);
                  sts = register_client(&ticket, 1);
		     if(sts == STS_SUCCESS)
                  sts = proxy_request(&ticket);
               }
            } else {
               if (MSG_IS_REQUEST(ticket.sipmsg)) {
                  sip_gen_response(&ticket, 408 /*request timeout*/);
               }
            }
	 } else {
            WARN("non-authorized registration attempt from %s",
	         utils_inet_ntoa(ticket.from.sin_addr));
	 }

      /*
       * check if outbound interface is UP.
       * If not, send back error to UA and
       * skip any proxying attempt
       */
      /* mod by weigy on 20100531 for the multi wan route select */
      #if MULTI_WAN
      } else if (get_ip_by_ifname(ticket.sctable_num ? configuration.outbound[ticket.sctable_num-1].name :
	            find_outboundif(ticket.lan),NULL) != STS_SUCCESS) {
      #else
      } else if (get_ip_by_ifname(configuration.outbound_if,NULL) !=
                 STS_SUCCESS) {
      #endif
      /* mod end */
         DEBUGC(DBCLASS_SIP, "got a %s to proxy, but outbound interface "
                "is down", (MSG_IS_REQUEST(ticket.sipmsg))? "REQ" : "RES");

         if (MSG_IS_REQUEST(ticket.sipmsg))
            sip_gen_response(&ticket, 408 /*request timeout*/);

      /*
       * MSG is a request, add current via entry,
       * do a lookup in the URLMAP table and
       * send to the final destination
       */
      } else if (MSG_IS_REQUEST(ticket.sipmsg)) {
         if (access & ACCESSCTL_SIP) {
            sts = proxy_request(&ticket);
	 } else {
            INFO("non-authorized request received from %s",
	         utils_inet_ntoa(ticket.from.sin_addr));
	 }

      /*
       * MSG is a response, remove current via and
       * send to the next VIA in chain
       */
      } else if (MSG_IS_RESPONSE(ticket.sipmsg)) {
         if (access & ACCESSCTL_SIP) {
            sts = proxy_response(&ticket);
	 } else {
            INFO("non-authorized response received from %s",
	         utils_inet_ntoa(ticket.from.sin_addr));
	 }

      /*
       * unsupported message
       */
      } else {
         ERROR("received unsupported SIP type %s %s",
	       (MSG_IS_REQUEST(ticket.sipmsg))? "REQ" : "RES",
	       ticket.sipmsg->sip_method);
      }


/*
 * free the SIP message buffers
 */
      end_loop:
      osip_message_free(ticket.sipmsg);
	 }
	}
   } /* while TRUE */
   exit_prg:

   /* dump current known SIP registrations */
   register_shut();
   INFO("properly terminating siproxd");

   /* remove PID file */
   if (pidfilename) {
      DEBUGC(DBCLASS_CONFIG,"deleting PID file [%s]", pidfilename);
      sts=unlink(pidfilename);
      if (sts != 0) {
         WARN("couldn't delete old PID file: %s", strerror(errno));
      }
   }

   /* END */
   return 0;
} /* main */

/*
 * Signal handler
 *
 * this one is called asynchronously whevener a registered
 * signal is applied. Just set a flag and don't do any funny
 * things here.
 */
static void sighandler(int sig) {
   if (sig==SIGTERM) exit_program=1;
   if (sig==SIGINT)  exit_program=1;
   if (sig==SIGUSR2) dmalloc_dump=1;
   return;
}


/*added by xuezhongbo for ALG 2007.11.2*/
void TrunOffSCTableState()
{
  int xzbi = 0;
  for(xzbi=0;xzbi<SCTMAX_SIZE;xzbi++)
  {
  if(SCTable[xzbi].sockindex==ENDINDEX)
	break;
  else
     SCTable[xzbi].state = SockOpened;
  }
}
/*end added*/

/*add by xuezhongbo to reboot sip proxy for renew internet info on 20091008*/
void closeallsipsock()
{
  int xzbi = 0;
  for(xzbi=0;xzbi<SCTMAX_SIZE;xzbi++)
  {
	  if(SCTable[xzbi].sockindex==ENDINDEX)
	      break;
	  else
	  {
	      if(SCTable[xzbi].sockfd > 0)
		  {
		      close(SCTable[xzbi].sockfd);
              SCTable[xzbi].state = SockClosed;
		  }
	  }
  }

  return;
}

/* mod by weigy on 20100531 for the multi wan route select */
#if MULTI_WAN
int resetinterface()
{
    int idx;
    struct in_addr outboundaddr;
    int found=FALSE;

    memset(&outboundaddr,0,sizeof(outboundaddr));

    for (idx=0; idx<configuration.numofoutbound; idx++)
    {
    	if(get_ip_by_ifname(configuration.outbound[idx].name,&outboundaddr) == STS_SUCCESS)
        {
           if(outboundaddr.s_addr != configuration.outbound[idx].addr.s_addr)
           {
      		   configuration.outbound[idx].addr.s_addr = outboundaddr.s_addr;
      		   found=TRUE;
    	   }
        }
    }

    if (found)
    {
       /* initialize the registration facility */
       register_init();
       rtp_close_all_active_relay();
       closeallsipsock();
	   sipsock_listen_mult();
	   return 1;
    }
    else
       return 0;
}
#else
int resetinterface()
{
    struct in_addr outboundaddr;
    memset(&outboundaddr,0,sizeof(outboundaddr));

	if(get_ip_by_ifname(configuration.outbound_if,&outboundaddr) == STS_SUCCESS)
    {
       if(outboundaddr.s_addr != configuration.outboundaddr.s_addr)
       {
  		   configuration.outboundaddr.s_addr = outboundaddr.s_addr;
           /* initialize the registration facility */
           register_init();
           rtp_close_all_active_relay();
           closeallsipsock();
		   sipsock_listen_mult();
		   return 1;
	   }
    }
    return 0;
}
#endif
/* mod end */
/*end add*/
