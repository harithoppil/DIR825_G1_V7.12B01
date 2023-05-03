/*
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

/* $Id: siproxd.h,v 1.48 2005/01/08 10:05:13 hb9xar Exp $ */

#ifdef DMALLOC
 #include <dmalloc.h>
#endif

/* add by weigy on 20100531 for the multi wan route select */
#include "config.h"
/* add end */

/*
 * table to hold the client registrations
 */
struct urlmap_s {
   int  active;
   int  expires;
   osip_uri_t *true_url;	// true URL of UA  (inbound URL)
   osip_uri_t *masq_url;	// masqueraded URL (outbound URL)
   osip_uri_t *reg_url;		// registered URL  (masq URL as wished by UA)
};
/*
 * the difference between masq_url and reg_url is, 
 * the reg URL *always* holds the url registered by the UA.
 * the masq_url may contain a different URL due to an additional
 * masquerading feature (mask_host, masked_host config options)
 */


/*
 * Array of strings - used within configuration store
 */
#define CFG_STRARR_SIZE		128	/* max 128 entries in array */
typedef struct {
   int  used;
   char *string[CFG_STRARR_SIZE];
} stringa_t;

/* add by weigy on 20100531 for the multi wan route select */
#if MULTI_WAN
#define MAX_WAN_NUM		    8
#define MAX_WAN_NAME_LEN    24

#define TRUE                1
#define FALSE               0

enum {
    CTYPE_NULL=0,
    CTYPE_INTERNET,
    CTYPE_VOIP,
    CTYPE_TR069,
    CTYPE_VOIP_INTERNET,
    CTYPE_TR069_INTERNET,
    CTYPE_VOIP_TR069,
    CTYPE_TR069_VOIP_INTERNET
};

typedef struct {
    char name[MAX_WAN_NAME_LEN];
    struct in_addr addr;
    int lan;
}outbound_info;
#endif
/* add end */

/*
 * configuration option table
 */
struct siproxd_config {
   int debuglevel;
   int debugport;
   char *inbound_if;
   /* mod by weigy on 20100531 for the multi wan route select */
   #if MULTI_WAN
   outbound_info outbound[MAX_WAN_NUM];
   int numofoutbound;
   #else
   char *outbound_if;
   #endif
   /* mod end */
   int sip_listen_port;
   int daemonize;
   int silence_log;
   int rtp_port_low;
   int rtp_port_high;
   int rtp_timeout;
   int rtp_proxy_enable;
   char *user;
   char *chrootjail;
   char *hosts_allow_reg;
   char *hosts_allow_sip;
   char *hosts_deny_sip;
   char *proxy_auth_realm;
   char *proxy_auth_passwd;
   char *proxy_auth_pwfile;
   stringa_t mask_host;
   stringa_t masked_host;
   char *outbound_proxy_host;
   int  outbound_proxy_port;
   stringa_t outbound_proxy_domain_name;
   stringa_t outbound_proxy_domain_host;
   stringa_t outbound_proxy_domain_port;
   char *registrationfile;
   int  log_calls;
   char *pid_file;
   int  default_expires;
/*add by xuezhong for interface choose on 20091009*/   
   char *totalinterface;
/*end add*/
   /* mod by weigy on 20100531 for the multi wan route select */
   #ifndef MULTI_WAN
   struct in_addr outboundaddr;
   #endif
   /* mod end */
   struct in_addr inboundaddr;
   /* add by weigy on 20100531 for the multi wan route select */
   #if MULTI_WAN
   char *wan_name[MAX_WAN_NUM];
   int  wan_type[MAX_WAN_NUM];
   int  wan_lan[MAX_WAN_NUM];
   #endif
   /* add end */
};

/*
 * SIP ticket
 */
typedef struct {
   osip_message_t *sipmsg;	/* SIP */
   struct sockaddr_in from;	/* received from */
#define PROTO_UNKN -1
#define PROTO_UDP  1
#define PROTO_TCP  2
   int protocol;		/* received by protocol */
#define REQTYP_INCOMING		1
#define REQTYP_OUTGOING		2
#define RESTYP_INCOMING		3
#define RESTYP_OUTGOING		4
   int direction;		/* direction as determined by proxy */
   /* add by weigy on 20100531 for the multi wan route select */
   #if MULTI_WAN
   int sctable_num;
   int lan;
   #endif
   /* add end */
} sip_ticket_t;

#if defined(IP_RECVDSTADDR) ||  defined(IP_RECVIF) 
/*added by xuezhongbo for ALG v2 2007.11.07:9:29*/
struct in_indexinfo
{
 struct in_addr ipi_addr;
 int ipi_ifindex;
};
#endif
/*end added*/

/*add by xuezhongbo for get ori dst 20080322*/
struct sockpair
{
struct sockaddr_in srcsock;
struct sockaddr_in dstsock;
};
#define SO_ORIGINAL_DST 80
/*end add*/


/*
 * Function prototypes
 */

/*				function returns STS_* status values     vvv */

/* sock.c */
int sipsock_listen(void);						/*X*/
/*added by xuezhongbo for ALG 2007.11.2*/
int sipsock_listen_mult (void);
/*end added*/
int sipsock_wait(void);
/*added by xuezhongbo for ALG 2007.11.2*/
int sipsock_wait_mult (void);
/*end added*/
int sipsock_read(void *buf, size_t bufsize,
                 struct sockaddr_in *from, int *protocol);
int sipsock_send(struct in_addr addr, int port,	int protocol,			/*X*/
                 char *buffer, int size);
int sockbind(struct in_addr ipaddr, int localport, int errflg);
/*add by xuezhongbo for wan interface on 20081028*/
int BindSockToInterface(int sockfd, char *ifname);
int creatsock();
int dobind(int sock,struct in_addr ipaddr, int localport);
int sipsock_send_to(int sock,struct in_addr addr, int port, int protocol,char *buffer, int size);
/*end add*/

/* register.c */
void register_init(void);
void register_shut(void);
int  register_client(sip_ticket_t *ticket, int force_lcl_masq);		/*X*/
void register_agemap(void);
int  register_response(sip_ticket_t *ticket, int flag);			/*X*/

/* proxy.c */
int proxy_request (sip_ticket_t *ticket);				/*X*/
int proxy_response (sip_ticket_t *ticket);				/*X*/
/* mod by weigy on 20100531 for the multi wan route select */
#if MULTI_WAN
int proxy_rewrite_invitation_body(osip_message_t *m, int direction,
				  struct in_addr *local_ip, sip_ticket_t *ticket);            /*X*/
#else
int proxy_rewrite_invitation_body(osip_message_t *m, int direction,
				  struct in_addr *local_ip);            /*X*/
#endif
/* mod end */
int proxy_rewrite_request_uri(osip_message_t *mymsg, int idx);		/*X*/

/* route_preprocessing.c */
int route_preprocess(sip_ticket_t *ticket);				/*X disable by xuezhongbo*/
int route_add_recordroute(sip_ticket_t *ticket);			/*X*/
int route_purge_recordroute(sip_ticket_t *ticket);			/*X*/
int route_postprocess(sip_ticket_t *ticket);				/*X disable by xuezhongbo*/
int route_determine_nexthop(sip_ticket_t *ticket,
                            struct in_addr *dest, int *port);		/*X disable by xuezhongbo*/
/*add by xuezhongbo for only del own route*/
int route_del(sip_ticket_t *ticket);
/*end add*/

/* utils.c */
int  get_ip_by_host(char *hostname, struct in_addr *addr);		/*X*/
void secure_enviroment (void);
int  get_ip_by_ifname(char *ifname, struct in_addr *retaddr);		/*X*/
char *utils_inet_ntoa(struct in_addr in);
int  utils_inet_aton(const char *cp, struct in_addr *inp);
void get_local_ip(char *ip, struct in_addr *local_ip);
/*added by xuezhongbo for ALG v2 2007.11.06:10:48*/
int get_netmask_by_addr(char *netmask,struct in_addr* addr);
/*end added*/
/*added by xuezhongbo for ALG v2 2007.11.07:9:05*/
int get_index_by_addr(int *index,struct in_addr* addr);
/*end added*/
/*add by xuezhongbo for get ori dst on 20080322*/
int bcmsystem (char *command);
/*end add*/



/* sip_utils.c */
osip_message_t * msg_make_template_reply (sip_ticket_t *ticket, int code);
int  check_vialoop (sip_ticket_t *ticket);				/*X*/
/* mod by weigy on 20100531 for the multi wan route select */
#if MULTI_WAN
int  is_via_local (osip_via_t *via, struct in_addr *local_ip, sip_ticket_t *ticket);		/*X*/
#else
int  is_via_local (osip_via_t *via, struct in_addr *local_ip);		/*X*/
#endif
/* mod end */
int  compare_url(osip_uri_t *url1, osip_uri_t *url2);			/*X*/
int  compare_callid(osip_call_id_t *cid1, osip_call_id_t *cid2);	/*X*/
int  is_sipuri_local (sip_ticket_t *ticket);				/*X*/
int  check_rewrite_rq_uri (osip_message_t *sip);			/*X*/
int  sip_gen_response(sip_ticket_t *ticket, int code);			/*X*/
#define IF_OUTBOUND 0
#define IF_INBOUND  1
int  sip_add_myvia (sip_ticket_t *ticket, int interface,
		    struct in_addr *local_ip);		                /*X*/
int  sip_del_myvia (sip_ticket_t *ticket, struct in_addr *local_ip);/*X*/
int  sip_rewrite_contact (sip_ticket_t *ticket, int direction,
			  struct in_addr *local_ip);	                /*X*/
int  sip_calculate_branch_id (sip_ticket_t *ticket, char *id);		/*X*/
int  sip_find_outbound_proxy(sip_ticket_t *ticket, struct in_addr *addr,
                             int *port);				/*X*/
/*add by xuezhongbo for outbound on 20080320*/                             
int sip_find_outbound_dst(int sockfd,struct sockpair pair,struct in_addr *originaldst);
/*end add*/
/*added by xuezhongbo for ALG v2.07 2007.11.12:15:56*/
int check_integrality_of_via (sip_ticket_t *ticket) ;
int sip_add_myviaport(sip_ticket_t *ticket, char *port);
/*end added*/

/* readconf.c */

int read_config(char *name, int search);				/*X*/

int make_default_config(void);						/*X*/

/* rtpproxy.c */
int  rtpproxy_init( void );						/*X*/
int  rtp_start_fwd (osip_call_id_t *callid, char *client_id,            /*X*/
                    int direction, 
#ifndef LIXB_LAN2LAN_BUG
					int msg_type,
#endif
                    int media_stream_no,
		    struct in_addr outbound_ipaddr, int *outboundport,
                    struct in_addr lcl_client_ipaddr, int lcl_clientport);
int  rtp_stop_fwd (osip_call_id_t *callid, int direction);		/*X*/
void rtpproxy_kill( void );						/*X*/

/* accessctl.c */
int  accesslist_check(struct sockaddr_in from);

/* security.c */
int  security_check_raw(char *sip_buffer, int size);			/*X*/
int  security_check_sip(sip_ticket_t *ticket);				/*X*/

/* auth.c */
int  authenticate_proxy(sip_ticket_t *ticket);				/*X*/
int  auth_include_authrq(sip_ticket_t *ticket);				/*X*/
void CvtHex(char *hash, char *hashstring);

/* fwapi.c */
int fwapi_start_rtp(int rtp_direction,
                    struct in_addr local_ipaddr, int local_port,
                    struct in_addr remote_ipaddr, int remote_port);
int fwapi_stop_rtp(int rtp_direction,
                   struct in_addr local_ipaddr, int local_port,
                   struct in_addr remote_ipaddr, int remote_port);


/*
 * some constant definitions
 */
/*added by xuezhongbo for ALG v2 2007.11.5:17:00*/
#if 0
#define SIP_PORT	5060	/* default port to listen */
#else
int SIP_PORT;
#endif
/*end added*/
#define DEFAULT_MAXFWD	70	/* default Max-Forward count */
#define DEFAULT_EXPIRES	3600	/* default Expires timeout */

/* mod by weigy on 20100531 for the multi wan route select */
#if MULTI_WAN
#define URLMAP_SIZE	9	/* number of URL mapping table entries	*/
#else
#define URLMAP_SIZE	4	/* number of URL mapping table entries	*/
#endif
/* mod end */
#define RTPPROXY_SIZE	8	/* number of rtp proxy entries		*/

#define BUFFER_SIZE	8196	/* input buffer for read from socket	*/
#define RTP_BUFFER_SIZE	1300 /* max size of an RTP frame		*/
#define URL_STRING_SIZE	128	/* max size of an URL/URI string	*/
#define STATUSCODE_SIZE 5	/* size of string representation of status */
#define DNS_CACHE_SIZE  32	/* number of entries in internal DNS cache */
#define DNS_MAX_AGE	60	/* maximum age of an cache entry (sec)	*/
#define IFADR_CACHE_SIZE 32	/* number of entries in internal IFADR cache */
#define IFADR_MAX_AGE	5	/* max. age of the IF address cache (sec) */
#define IFNAME_SIZE	16	/* max string length of a interface name */
#define HOSTNAME_SIZE	64	/* max string length of a hostname	*/
#define USERNAME_SIZE	64	/* max string length of a username (auth) */
#define PASSWORD_SIZE	64	/* max string length of a password (auth) */
#define VIA_BRANCH_SIZE 64	/* max string length for via branch param */
				/* scratch buffer for gethostbyname_r() */
#if defined(PR_NETDB_BUF_SIZE)
   #define GETHOSTBYNAME_BUFLEN PR_NETDB_BUF_SIZE 
#else
   #define GETHOSTBYNAME_BUFLEN 1024
#endif

/* constants for security testing */
#define SEC_MINLEN	16	/* minimum received length */
#define SEC_MAXLINELEN	1024	/* maximum acceptable length of one line
				   in the SIP telegram (security check)
                                   Careful: Proxy-Authorization lines may
                                   get quite long */

/* symbols for access control */
#define ACCESSCTL_SIP	1	/* for access control - SIP allowed	*/
#define ACCESSCTL_REG	2	/* --"--              - registr. allowed */

/* symbolic return stati */
#define STS_SUCCESS	0	/* SUCCESS				*/
#define STS_TRUE	0	/* TRUE					*/
#define STS_FAILURE	1	/* FAILURE				*/
#define STS_FALSE	1	/* FALSE				*/
#define STS_NEED_AUTH	1001	/* need authentication			*/


/* symbolic direction of data */
#define DIR_INCOMING	1
#define DIR_OUTGOING	2

#ifndef LIXB_LAN2LAN_BUG
#define NONE_MSG    	0
#define REQUEST_MSG    1
#define RESPONSE_MSG    2
#endif

/* various */
#ifndef satoi
#define satoi atoi  /* used in libosips MSG_TEST_CODE macro ... */
#endif


/*this struct used for record the sock state */
/*added by xuezhongbo for ALG 2007.11.2*/
struct SockControlTable
{
int sockindex;
int sockfd;
int udpport;
int state;      /*state for open,close,canread,canwrite*/
char *sockname;
};

#define SCTMAX_SIZE URLMAP_SIZE 

/*the end of sockindex*/
#define ENDINDEX -1 

/*sockfd*/
#define UNKNOWNSOCK 0

/*udpport*/
#define DEFAULTPORT 5060
#define ADDPORT1 5064
#define ADDPORT2 5061
/*#define ADDPORTx zzzzadd as you like ,but add the value in sockcontrol.c in the same time*/

/*sock state*/
#define SockClosed 0x0000
#define SockOpened 0x0001
#define SockCanRead 0x0002
#define SockCanWrite 0x0004

/*sockname*/
/*not used*/
/*end added*/

/*added by xuezhongbo for ALG v2.07 2007.11.12:16:23*/
#define DEFAULTSIPPORT "5060"
/*end added*/

/*add by zengw on 20100317 for tos */
/*
 * Macro that limits the frequency of this particular code
 * block to no faster than every 'a' seconds. Used for logging
 */
#define LIMIT_LOG_RATE(a) \
        static time_t last=0; \
        time_t now; \
        int dolog=0; \
        time(&now); \
        if ((last+(a)) <= now) {last=now; dolog=1;} \
        if (dolog)
/*add end*/
        
