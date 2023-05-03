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
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <osipparser2/osip_parser.h>
#include <osipparser2/sdp_message.h>

#include "siproxd.h"
#include "log.h"

static char const ident[]="$Id: proxy.c,v 1.80 2005/01/24 19:12:40 hb9xar Exp $";

/* configuration storage */
extern struct siproxd_config configuration;	/* defined in siproxd.c */
extern struct SockControlTable SCTable[SCTMAX_SIZE];
extern int bodymessagelen;
extern char bodymessage[1500];
extern struct urlmap_s urlmap[];		/* URL mapping table     */
extern struct lcl_if_s local_addresses;

/*added by xuezhong for ALG v2 2007.11.07:9:20*/
#ifdef IP_PKTINFO
extern struct in_pktinfo pktp;
#else
{
#if defined(IP_RECVDSTADDR) || defined(IP_RECVIF) 
extern struct in_indexinfo pktp;
#endif
}
#endif
/*end added*/ 
extern int sip_udp_socket;
/*added by xuezhongbo for ALG v2 2007.11.07:13:53*/
/*if sucess return 1
if failure return -1
if the request flag = 1
if the response flag = 0*/
/* mod by weigy on 20100531 for the multi wan route select */
#if MULTI_WAN
static int getdirctiontypebyipaddr(int *type,int flag, sip_ticket_t *ticket)
#else
static int getdirctiontypebyipaddr(int *type,int flag)
#endif
/* mod end */
{
   struct in_addr addr2;
   struct in_addr addr3;
   
#ifdef IP_PKTINFO
   struct in_pktinfo zeropktp;
   memset(&zeropktp,0,sizeof(struct in_pktinfo));
#else
{
#if defined(IP_RECVDSTADDR) || defined(IP_RECVIF) 
   struct in_indexinfo zeropktp;
   memset(&zeropktp,0,sizeof(struct in_indexinfo));
#endif
}
#endif

if(type==NULL)
	return -1;

   memset(&zeropktp,0,sizeof(zeropktp));
   
   if(memcmp(&pktp.ipi_addr,&zeropktp.ipi_addr,sizeof(struct in_addr))==0)
   	return -1;
   	
   if(get_ip_by_ifname(configuration.inbound_if,&addr2)!=STS_SUCCESS)
   	{
   	return -1;
   	}
   else
   	{
   	if(memcmp(&pktp.ipi_addr,&addr2,sizeof(struct in_addr))==0)
		{
		if(flag == 1) {
		      *type = REQTYP_OUTGOING;
		       return 1;
		}
             if(flag == 0){
		      *type = RESTYP_OUTGOING;	
		      return 1;
		}		
             return -1;
		}
   	}
   /* mod by weigy on 20100531 for the multi wan route select */
   #if MULTI_WAN
   if(get_ip_by_ifname(ticket->sctable_num ? configuration.outbound[ticket->sctable_num-1].name : 
	    find_outboundif(ticket->lan),&addr3)!=STS_SUCCESS)
   #else
   if(get_ip_by_ifname(configuration.outbound_if,&addr3)!=STS_SUCCESS)
   #endif
   /* mod end */
   	{
   	return -1;
   	}
   else
   	{
   	if(memcmp(&pktp.ipi_addr,&addr3,sizeof(struct in_addr))==0)
		{
		if(flag == 1) {
		      *type = REQTYP_INCOMING;
		      return 1;
		}
             if(flag == 0){
		      *type = RESTYP_INCOMING;	
		      return 1;
		}		
		return -1;
		}   	
   	}
   return -1;
}
/*end added*/

/*added by xuezhongbo for ALG v2 2007.11.07:13:53*/
/*if sucess return 1
if failure return -1
if the request flag = 1
if the response flag = 0*/
/* mod by weigy on 20100531 for the multi wan route select */
#if MULTI_WAN
static int getdirctiontypebyindex(int *type,int flag, sip_ticket_t *ticket)
#else
static int getdirctiontypebyindex(int *type,int flag)
#endif
/* mod end */
{
   struct in_addr addr2;
   struct in_addr addr3;
   int index=0;
   
#ifdef IP_PKTINFO
   struct in_pktinfo zeropktp;
   memset(&zeropktp,0,sizeof(struct in_pktinfo));
#else
{
#if defined(IP_RECVDSTADDR) || defined(IP_RECVIF) 
   struct in_indexinfo zeropktp;
   memset(&zeropktp,0,sizeof(struct in_indexinfo));
#endif
}
#endif

   if(pktp.ipi_ifindex==0)
   	return -1;
   
   if(get_ip_by_ifname(configuration.inbound_if,&addr2)!=STS_SUCCESS)
   	{
   	return -1;
   	}
   else
   	{
   	if(get_index_by_addr(&index,&addr2)<0)
		return -1;
   	if(index==pktp.ipi_ifindex)
		{
             if(flag == 1) {
		          *type = REQTYP_OUTGOING;
		          return 1;
		          }
             if(flag == 0){
		          *type = RESTYP_OUTGOING;	
		          return 1;
		          }
		return -1;
   	      }
   	}

   /* mod by weigy on 20100531 for the multi wan route select */
   #if MULTI_WAN
   if(get_ip_by_ifname(ticket->sctable_num ? configuration.outbound[ticket->sctable_num-1].name : 
	    find_outboundif(ticket->lan),&addr3)!=STS_SUCCESS)
   #else
   if(get_ip_by_ifname(configuration.outbound_if,&addr3)!=STS_SUCCESS)
   #endif
   /* mod end */
   	{
   	return -1;
   	}
   else
   	{
   	if(get_index_by_addr(&index,&addr3)<0)
		return -1;
   	if(index==pktp.ipi_ifindex)
		{
		if(flag == 1) {
		         *type = REQTYP_INCOMING;
		         return 1;
		         }
             if(flag == 0){
		         *type = RESTYP_INCOMING;	
		         return 1;
		         }	
		return -1;
		}
   	}
   return -1;
}
/*end added*/

/*
 * PROXY_REQUEST
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 *
 * RFC3261
 *    Section 16.3: Proxy Behavior - Request Validation
 *    1. Reasonable Syntax
 *    2. URI scheme
 *    3. Max-Forwards
 *    4. (Optional) Loop Detection
 *    5. Proxy-Require
 *    6. Proxy-Authorization
 *
 *    Section 16.6: Proxy Behavior - Request Forwarding
 *    1.  Make a copy of the received request
 *    2.  Update the Request-URI
 *    3.  Update the Max-Forwards header field
 *    4.  Optionally add a Record-route header field value
 *    5.  Optionally add additional header fields
 *    6.  Postprocess routing information
 *    7.  Determine the next-hop address, port, and transport
 *    8.  Add a Via header field value
 *    9.  Add a Content-Length header field if necessary
 *    10. Forward the new request
 *    11. Set timer C
 */
int proxy_request (sip_ticket_t *ticket) {
   int i;
   int sts;
   int type;
   struct in_addr sendto_addr;
   osip_uri_t *url;
   int port;
   char *buffer;
   char *endp = NULL;
   char *startp = NULL;
   osip_message_t *request;
   struct sockaddr_in *from;
   struct in_addr local_ip;
   char bufmsg[1500];
   int sockfd = 0;
   DEBUGC(DBCLASS_PROXY,"proxy_request");
   char contentbuf[20];
   if (ticket==NULL) {
      ERROR("proxy_request: called with NULL ticket");
      return STS_FAILURE;
   }

   request=ticket->sipmsg;
   from=&ticket->from;

   /*
    * RFC 3261, Section 16.4
    * Proxy Behavior - Route Information Preprocessing
    * (process Route header)
    */
   /*mod by xuezhongbo for only del route on 20080319*/
   #if 0
   route_preprocess(ticket);
   #else
   route_del(ticket);
   #endif
   /*end mod*/
   /*
    * figure out whether this is an incoming or outgoing request
    * by doing a lookup in the registration table.
    */
#define _OLD_DIRECTION_EVALUATION 0
#if _OLD_DIRECTION_EVALUATION
   type = 0;
   for (i=0; i<URLMAP_SIZE; i++) {
      if (urlmap[i].active == 0) continue;

      /* incoming request ('to' == 'masq') || (('to' == 'reg') && !REGISTER)*/
      if ((compare_url(request->to->url, urlmap[i].masq_url)==STS_SUCCESS) ||
          (!MSG_IS_REGISTER(request) &&
           (compare_url(request->to->url, urlmap[i].reg_url)==STS_SUCCESS))) {
         type=REQTYP_INCOMING;
         DEBUGC(DBCLASS_PROXY,"incoming request from %s@%s from outbound",
	   request->from->url->username? request->from->url->username:"*NULL*",
           request->from->url->host? request->from->url->host: "*NULL*");
	 break;
      }

      /* outgoing request ('from' == 'reg') */
      if (compare_url(request->from->url, urlmap[i].reg_url)==STS_SUCCESS) {
         type=REQTYP_OUTGOING;
         DEBUGC(DBCLASS_PROXY,"outgoing request from %s@%s from inbound",
	   request->from->url->username? request->from->url->username:"*NULL*",
           request->from->url->host? request->from->url->host: "*NULL*");
	 break;
      }
   }
#else
   type = 0;
   /*
    * did I receive the telegram from a REGISTERED host?
    * -> it must be an OUTGOING request
    */

   /*add by xuezhongbo on 20071226 for sipalg*/
   if(MSG_IS_REGISTER(request))
   {
       type=REQTYP_OUTGOING;
       DEBUGC(DBCLASS_PROXY, "proxy_request: MSG_IS_REGISTER let it OUTGOING");
   }
   /*end add*/
   else
   {
       for (i=0; i<URLMAP_SIZE; i++) {
          struct in_addr tmp_addr;

          if (urlmap[i].active == 0) 
              continue;

          if ((urlmap[i].true_url == NULL)||(get_ip_by_host(urlmap[i].true_url->host, &tmp_addr) == STS_FAILURE)) 
          {
              DEBUGC(DBCLASS_PROXY, "proxy_request: cannot resolve host [%s]",
                    urlmap[i].true_url==NULL?"*NULL":(urlmap[i].true_url->host==NULL?"*NULL*":urlmap[i].true_url->host));
          } 
          else 
          {
              DEBUGC(DBCLASS_PROXY, "proxy_request: reghost:%s ip:%s",urlmap[i].true_url->host,utils_inet_ntoa(from->sin_addr));
              if (memcmp(&tmp_addr, &from->sin_addr, sizeof(tmp_addr)) == 0) 
              {
                 type=REQTYP_OUTGOING;
                 DEBUGC(DBCLASS_PROXY, "proxy_request: REQTYP_OUTGOING i=%d",i);
                 /*added by xuezhongbo for ALG v2.07 2007.11.12:15:56*/
                 if(check_integrality_of_via(ticket)==0) 
                 {
                     //not integrality
                     if(urlmap[i].true_url->port!=NULL)
                        sip_add_myviaport (ticket,urlmap[i].true_url->port);
                     else 
                     { 
                       /*added the default port 5060*/ 
                       sip_add_myviaport (ticket,DEFAULTSIPPORT);             
                     }
                 }      
                 /*end added*/
	          break;
             }
         }
       }
   }
   DEBUGC(DBCLASS_PROXY,"proxy_urlmap sheet %d\n",i);
   /*
    * is the telegram directed to an internally registered host?
    * -> it must be an INCOMING request
    */
   if (type == 0) {
      for (i=0; i<URLMAP_SIZE; i++) {
         if (urlmap[i].active == 0) continue;
         /* RFC3261:
          * 'To' contains a display name (Bob) and a SIP or SIPS URI
          * (sip:bob@biloxi.com) towards which the request was originally
          * directed.  Display names are described in RFC 2822 [3].
          */

         /* So this means, that we must check the SIP URI supplied with the
          * INVITE method, as this points to the real wanted target.
          * Q: does there exist a situation where the SIP URI itself does
          *    point to "somewhere" but the To: points to the correct UA?
          * So for now, we just look at both of them (SIP URI and To: header)
          */

         /* incoming request (SIP URI == 'masq') || ((SIP URI == 'reg') && !REGISTER)*/
         if ((compare_url(request->req_uri, urlmap[i].masq_url)==STS_SUCCESS) ||
             (!MSG_IS_REGISTER(request) &&
              (compare_url(request->req_uri, urlmap[i].reg_url)==STS_SUCCESS))) {
            type=REQTYP_INCOMING;
            DEBUGC(DBCLASS_PROXY, "proxy_request: REQTYP_INCOMING i=%d",i);
	        break;
         }
         DEBUGC(DBCLASS_PROXY,"proxy_urlmap sheet2 %d\n",i);        
         /* incoming request ('to' == 'masq') || (('to' == 'reg') && !REGISTER)*/
         if ((compare_url(request->to->url, urlmap[i].masq_url)==STS_SUCCESS) ||
             (!MSG_IS_REGISTER(request) &&
              (compare_url(request->to->url, urlmap[i].reg_url)==STS_SUCCESS))) {
            DEBUGC(DBCLASS_PROXY, "proxy_request: REQTYP_INCOMING2 i=%d",i);
            type=REQTYP_INCOMING;
	        break;
         }
      }
   }
#endif
   ticket->direction=type;
   DEBUGC(DBCLASS_PROXY,"proxy_urlmap sheet3 %d\n",i);
   /*
    * logging of passing calls
    */
   if (configuration.log_calls) {
      osip_uri_t *cont_url = NULL;
      if (!osip_list_eol(request->contacts, 0))
         cont_url = ((osip_contact_t*)(request->contacts->node->element))->url;
      
      /* INVITE */
      if (MSG_IS_INVITE(request)) {
         if (cont_url) {
            INFO("%s Call from: %s@%s",
                 (type==REQTYP_INCOMING) ? "Incoming":"Outgoing",
                 cont_url->username ? cont_url->username:"*NULL*",
                 cont_url->host ? cont_url->host : "*NULL*");
         } else {
            INFO("%s Call (w/o contact header) from: %s@%s",
                 (type==REQTYP_INCOMING) ? "Incoming":"Outgoing",
	         request->from->url->username ? 
                    request->from->url->username:"*NULL*",
	         request->from->url->host ? 
                    request->from->url->host : "*NULL*");
         }
      /* BYE / CANCEL */
      } else if (MSG_IS_BYE(request) || MSG_IS_CANCEL(request)) {
         if (cont_url) {
            INFO("Ending Call from: %s@%s",
                 cont_url->username ? cont_url->username:"*NULL*",
                 cont_url->host ? cont_url->host : "*NULL*");
         } else {
            INFO("Ending Call (w/o contact header) from: %s@%s",
	         request->from->url->username ? 
                    request->from->url->username:"*NULL*",
	         request->from->url->host ? 
                    request->from->url->host : "*NULL*");
         }
      }
   } /* log_calls */


   /*
    * RFC 3261, Section 16.6 step 1
    * Proxy Behavior - Request Forwarding - Make a copy
    */
   /* nothing to do here, copy is ready in 'request'*/

   /* get destination address */
   url=osip_message_get_uri(request);

   /* Determine local address to use based on the To: address */
/*del by xuezhongbo for better way choose right ip addr on  20081024*/
   #if 0
   get_local_ip(request->to->url->host, &local_ip);
   DEBUGC(DBCLASS_PROXY, "local IP: %s", inet_ntoa(local_ip));
   #endif
/*end del*/
   
   switch (type) {
  /*
   * from an external host to the internal masqueraded host
   */
   case REQTYP_INCOMING:

/*del by xuezhongbo for better way choose right ip addr on  20081024*/
       {
           struct sockaddr_in localaddr;
           if(get_ip_by_ifname(configuration.inbound_if,&localaddr.sin_addr) == STS_SUCCESS)      
           {
               local_ip = localaddr.sin_addr;
           }
		   else
		   {
		       get_local_ip(request->to->url->host, &local_ip);
		   }
		   DEBUGC(DBCLASS_PROXY, "local IP: %s", inet_ntoa(local_ip));
       }
/*end del*/
      DEBUGC(DBCLASS_PROXY,"incoming request from %s@%s from outbound",
	         request->from->url->username? request->from->url->username:"*NULL*",
             request->from->url->host? request->from->url->host: "*NULL*");

      /*
       * RFC 3261, Section 16.6 step 2
       * Proxy Behavior - Request Forwarding - Request-URI
       * (rewrite request URI to point to the real host)
       */
      /* 'i' still holds the valid index into the URLMAP table */
      if (check_rewrite_rq_uri(request) == STS_TRUE) {
         proxy_rewrite_request_uri(request, i);
      }

      /* if this is CANCEL/BYE request, stop RTP proxying */
      if (MSG_IS_BYE(request) || MSG_IS_CANCEL(request)) {
         /* stop the RTP proxying stream(s) */
         rtp_stop_fwd(osip_message_get_call_id(request), DIR_INCOMING);
         rtp_stop_fwd(osip_message_get_call_id(request), DIR_OUTGOING);

      /* check for incoming request */
      } else if ((MSG_IS_INVITE(request))|| (MSG_IS_UPDATE(request))) {
         /* rewrite the body */
         if (configuration.rtp_proxy_enable == 1) {
            /* mod by weigy on 20100531 for the multi wan route select */
            #if MULTI_WAN
            sts = proxy_rewrite_invitation_body(request, DIR_INCOMING, NULL, ticket);
            #else
            sts = proxy_rewrite_invitation_body(request, DIR_INCOMING, NULL);
            #endif
            /* mod end */
         }
      } else if (MSG_IS_ACK(request)) {
         /* rewrite the body */
         /* mod by weigy on 20100531 for the multi wan route select */
         #if MULTI_WAN
         sts = proxy_rewrite_invitation_body(request, DIR_INCOMING, NULL, ticket);
         #else
         sts = proxy_rewrite_invitation_body(request, DIR_INCOMING, NULL);
         #endif
         /* mod end */
      }
      break;
   
  /*
   * from the internal masqueraded host to an external host
   */
   case REQTYP_OUTGOING:
      DEBUGC(DBCLASS_PROXY,"outgoing request from %s@%s from inbound",
	         request->from->url->username? request->from->url->username:"*NULL*",
             request->from->url->host? request->from->url->host: "*NULL*");

      /*
       * RFC 3261, Section 16.6 step 2
       * Proxy Behavior - Request Forwarding - Request-URI
       */
      /* nothing to do for an outgoing request */


      /* if it is addressed to myself, then it must be some request
       * method that I as a proxy do not support. Reject */
#if 0
/* careful - an internal UA might send an request to another internal UA.
   This would be caught here, so don't do this. This situation should be
   caught in the default part of the CASE statement below */
      if (is_sipuri_local(ticket) == STS_TRUE) {
         WARN("unsupported request [%s] directed to proxy from %s@%s -> %s@%s",
	    request->sip_method? request->sip_method:"*NULL*",
	    request->from->url->username? request->from->url->username:"*NULL*",
	    request->from->url->host? request->from->url->host : "*NULL*",
	    url->username? url->username : "*NULL*",
	    url->host? url->host : "*NULL*");

         sip_gen_response(ticket, 403 /*forbidden*/);

         return STS_FAILURE;
      }
#endif

/*del by xuezhongbo for better way choose right ip addr on  20081024*/
       {
           struct sockaddr_in localaddr;
           /* mod by weigy on 20100531 for the multi wan route select */
           #if MULTI_WAN
           if(get_ip_by_ifname(ticket->sctable_num ? configuration.outbound[ticket->sctable_num-1].name : 
	            find_outboundif(ticket->lan),&localaddr.sin_addr) == STS_SUCCESS)      
	       #else
           if(get_ip_by_ifname(configuration.outbound_if,&localaddr.sin_addr) == STS_SUCCESS)      
	       #endif
	       /* mod end */
           {
               local_ip = localaddr.sin_addr;
           }
		   else
		   {
		       get_local_ip(request->to->url->host, &local_ip);
		   }
		   DEBUGC(DBCLASS_PROXY, "local IP: %s", inet_ntoa(local_ip));
       }
/*end del*/

      /* rewrite Contact header to represent the masqued address */
      sip_rewrite_contact(ticket, DIR_OUTGOING, &local_ip);

      /* if an INVITE, rewrite body */
      if ((MSG_IS_INVITE(request))|| (MSG_IS_UPDATE(request))) {
         /* mod by weigy on 20100531 for the multi wan route select */
         #if MULTI_WAN
         sts = proxy_rewrite_invitation_body(request, DIR_OUTGOING, &local_ip, ticket);
         #else
         sts = proxy_rewrite_invitation_body(request, DIR_OUTGOING, &local_ip);
         #endif
        /* mod end */
      } else if (MSG_IS_ACK(request)) {
         /* mod by weigy on 20100531 for the multi wan route select */
         #if MULTI_WAN
         sts = proxy_rewrite_invitation_body(request, DIR_OUTGOING, &local_ip, ticket);
         #else
         sts = proxy_rewrite_invitation_body(request, DIR_OUTGOING, &local_ip);
         #endif
         /* mod end */
      }

      /* if this is CANCEL/BYE request, stop RTP proxying */
      if (MSG_IS_BYE(request) || MSG_IS_CANCEL(request)) {
         /* stop the RTP proxying stream(s) */
         rtp_stop_fwd(osip_message_get_call_id(request), DIR_INCOMING);
         rtp_stop_fwd(osip_message_get_call_id(request), DIR_OUTGOING);
      }

      break;
   
   default:
      DEBUGC(DBCLASS_PROXY, "request [%s] from/to unregistered UA "
           "(RQ: %s@%s -> %s@%s)",
           request->sip_method? request->sip_method:"*NULL*",
	   request->from->url->username? request->from->url->username:"*NULL*",
	   request->from->url->host? request->from->url->host : "*NULL*",
	   url->username? url->username : "*NULL*",
	   url->host? url->host : "*NULL*");

/*
 * we may end up here for two reasons:
 *  1) An incomming request (from outbound) that is directed to
 *     an unknown (not registered) local UA
 *  2) an outgoing request from a local UA that is not registered.
 *
 * Case 1) we should probably answer with "404 Not Found",
 * case 2) more likely a "403 Forbidden"
 * 
 * How about "408 Request Timeout" ?
 *
 */
      DEBUGC(DBCLASS_PROXY,"proxy_request ticket, 408 /* Request Timeout */ \n");
      sip_gen_response(ticket, 408 /* Request Timeout */);

      return STS_FAILURE;
   }


   /*
    * RFC 3261, Section 16.6 step 3
    * Proxy Behavior - Request Forwarding - Max-Forwards
    * (if Max-Forwards header exists, decrement by one, if it does not
    * exist, add a new one with value SHOULD be 70)
    */
   {
   osip_header_t *max_forwards;
   int forwards_count = DEFAULT_MAXFWD;
   char mfwd[8];

   osip_message_get_max_forwards(request, 0, &max_forwards);
   if (max_forwards == NULL) {
      sprintf(mfwd, "%i", forwards_count);
      osip_message_set_max_forwards(request, mfwd);
   } else {
      if (max_forwards->hvalue) {
         forwards_count = atoi(max_forwards->hvalue);
         forwards_count -=1;
         osip_free (max_forwards->hvalue);
      }

      sprintf(mfwd, "%i", forwards_count);
      max_forwards->hvalue = osip_strdup(mfwd);
   }

   DEBUGC(DBCLASS_PROXY,"setting Max-Forwards=%s",mfwd);
   }

   /*
    * RFC 3261, Section 16.6 step 4
    * Proxy Behavior - Request Forwarding - Add a Record-route header
    */

   /*
    * for ALL incoming requests, include my Record-Route header.
    * The local UA will probably send its answer to the topmost 
    * Route Header (8.1.2 of RFC3261)
    */
   if (type == REQTYP_INCOMING) {
      DEBUGC(DBCLASS_PROXY,"Adding my Record-Route");
      /*del by xuezhongbo for conflict get ori dst on 20080411*/
      #if 0
      route_add_recordroute(ticket);
      #endif
      /*end del*/
   } else {
      /*
       * outgoing packets must not have my record route header, as
       * this likely will contain a private IP address (my inbound).
       */
      DEBUGC(DBCLASS_PROXY,"Purging Record-Routes (outgoing packet)");
      route_purge_recordroute(ticket);
   }

   /*
    * RFC 3261, Section 16.6 step 5
    * Proxy Behavior - Request Forwarding - Add Additional Header Fields
    */
   /* NOT IMPLEMENTED (optional) */


   /*
    * RFC 3261, Section 16.6 step 6
    * Proxy Behavior - Request Forwarding - Postprocess routing information
    *
    * If the copy contains a Route header field, the proxy MUST
    * inspect the URI in its first value.  If that URI does not
    * contain an lr parameter, the proxy MUST modify the copy as
    * follows:
    *
    * -  The proxy MUST place the Request-URI into the Route header
    *    field as the last value.
    *
    * -  The proxy MUST then place the first Route header field value
    *    into the Request-URI and remove that value from the Route
    *    header field.
    */
#if 0
   route_postprocess(ticket);
#endif

   /*
    * RFC 3261, Section 16.6 step 7
    * Proxy Behavior - Determine Next-Hop Address
    */
/*&&&& priority probably should be:
 * 1) Route header
 * 2) fixed outbound proxy
 * 3) SIP URI
 */
   /*
    * fixed or domain outbound proxy defined ?
    */
   #if 0
   if ((type == REQTYP_OUTGOING) &&
       (sip_find_outbound_proxy(ticket, &sendto_addr, &port) == STS_SUCCESS)) {
      DEBUGC(DBCLASS_PROXY, "proxy_request: have outbound proxy %s:%i",
             utils_inet_ntoa(sendto_addr), port);
   /*
    * Route present?
    * If so, fetch address from topmost Route: header and remove it.
    */
   } else if ((type == REQTYP_OUTGOING) && 
              (request->routes && !osip_list_eol(request->routes, 0))) {
      sts=route_determine_nexthop(ticket, &sendto_addr, &port);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "proxy_request: route_determine_nexthop failed");
         return STS_FAILURE;
      }
      DEBUGC(DBCLASS_PROXY, "proxy_request: have Route header to %s:%i",
             utils_inet_ntoa(sendto_addr), port);
   /*
    * destination from SIP URI
    */
   } else {
      /* get the destination from the SIP URI */
      sts = get_ip_by_host(url->host, &sendto_addr);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "proxy_request: cannot resolve URI [%s]",
                url->host);
         return STS_FAILURE;
      }
/*added by xuezhongbo for ALG v1 2007.11.5:16:09*/
#if 0
      if (url->port) {
         port=atoi(url->port);
      } else {
         port=SIP_PORT;
      }
#else
      if (url->port) {
         port=atoi(url->port);
      } else {
         port=configuration.sip_listen_port;
      }
#endif
/*end added*/	  
      DEBUGC(DBCLASS_PROXY, "proxy_request: have SIP URI to %s:%i",
             url->host, port);
   }
   #endif
   /*add by xuezhongbo for OUTGOING dst*/
   /*the is sent to the original dst on 20080322*/
   if (type == REQTYP_OUTGOING)
   {
       struct sockpair pair;

       pair.srcsock.sin_port = htons(configuration.sip_listen_port);
       pair.dstsock.sin_addr.s_addr = ticket->from.sin_addr.s_addr;
       pair.dstsock.sin_port = ticket->from.sin_port;       
       if((get_ip_by_ifname(configuration.inbound_if,&pair.srcsock.sin_addr) == STS_SUCCESS)&&
          (sip_find_outbound_dst(sip_udp_socket,pair,&sendto_addr)))
       {                         
#if 1
          if (url->port) {
              port=atoi(url->port);
          } else {
    	       port=SIP_PORT;
	           DEBUGC(DBCLASS_PROXY, "proxy_request: get default port=%d", port);
          }           
#else
           port = configuration.sip_listen_port;
#endif
       }
       else
       {      
          /*use the normal way to get dst on 20080322*/
          /* get the destination from the SIP URI 
           * we should add code for dealing with ROUTE head. do it next day
           */
          sts = get_ip_by_host(url->host, &sendto_addr);
          if (sts == STS_FAILURE) {
              DEBUGC(DBCLASS_PROXY, "proxy_request: cannot resolve URI [%s]",url->host);
              return STS_FAILURE;
          }   
          if (url->port) {
              port=atoi(url->port);
          } else {
              port=SIP_PORT;
          }           
       }
   }
   /*for INCOMING dst*/
   else
   {
      /* get the destination from the SIP URI */
      sts = get_ip_by_host(url->host, &sendto_addr);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "proxy_request: cannot resolve URI [%s]",
                url->host);
         return STS_FAILURE;
      }   
      if (url->port) {
         port=atoi(url->port);
      } else {
         /*which port should we use? 5060,contact:port ??*/
         port=SIP_PORT;
      }      
   }
   /*end add*/

   /*
    * RFC 3261, Section 16.6 step 8
    * Proxy Behavior - Add a Via header field value
    */
   /* add my Via header line (outbound interface)*/
   if (type == REQTYP_INCOMING) 
   {
   	    
      /*del by xuezhongbo for conflict get ori dst on 20080411*/
      #if 0
      sts = sip_add_myvia(ticket, IF_INBOUND, NULL);
      if (sts == STS_FAILURE) {
         ERROR("adding my inbound via failed!");
      }
      #endif
      /*end del*/
/*add by xuezhongbo for wan interface on 20081028*/
		  int i ;
		  for(i = 0; (i < SCTMAX_SIZE)&&(SCTable[i].sockindex!=ENDINDEX); i++)
		  {
		      /* mod by weigy on 20100531 for the multi wan route select */
              #if MULTI_WAN
		      if(SCTable[i].sockfd == sip_udp_socket)
		      {
	              sockfd = SCTable[0].sockfd ;
				  break;
		      }
		      #else
		      if(SCTable[i].sockfd == sip_udp_socket)
		      {
		          if(i%2 != 0)
		          {
		             sockfd =  SCTable[i-1].sockfd ;
		          }
				  else
				  {
				     sockfd =  SCTable[i+1].sockfd ;
				  }
				  break;
		      }
		      #endif
		      /* mod end */
		  }
		  if((i >= SCTMAX_SIZE)||(SCTable[i].sockindex == ENDINDEX))
		  {
	         ERROR("adding my outbound via failed!");
	         return STS_FAILURE;		  
		  }
/*end add*/	  
      } 
      else 
      {     
	      sts = sip_add_myvia(ticket, IF_OUTBOUND, &local_ip);
	      if (sts == STS_FAILURE) 
		  {
	         ERROR("adding my outbound via failed!");
	         return STS_FAILURE;
	      }
/*add by xuezhongbo for wan interface on 20081028*/
		  int i ;
		  for(i = 0; (i < SCTMAX_SIZE)&&(SCTable[i].sockindex!=ENDINDEX); i++)
		  {
		      /* mod by weigy on 20100531 for the multi wan route select */
              #if MULTI_WAN
		      if(SCTable[i].sockfd == sip_udp_socket)
		      {
		          sockfd = SCTable[find_outboundidx(ticket->lan)+1].sockfd;
				  break;
		      }
		      #else
		      if(SCTable[i].sockfd == sip_udp_socket)
		      {
		          if(i%2 != 0)
		          {
		             sockfd =  SCTable[i-1].sockfd ;
		          }
				  else
				  {
				     sockfd =  SCTable[i+1].sockfd ;
				  }
				  break;
		      }
		      #endif
		      /* mod end */
		  }
		  if((i >= SCTMAX_SIZE)||(SCTable[i].sockindex == ENDINDEX))
		  {
	         ERROR("adding my outbound via failed!");
	         return STS_FAILURE;		  
		  }
/*end add*/		  
   }
  /*
   * RFC 3261, Section 16.6 step 9
   * Proxy Behavior - Add a Content-Length header field if necessary
   */
  /* not necessary, already in message and we do not support TCP */

  /*
   * RFC 3261, Section 16.6 step 10
   * Proxy Behavior - Forward the new request
   */
   sts = osip_message_to_str(request, &buffer);
   if (sts != 0) {
      ERROR("proxy_request: osip_message_to_str failed");
      return STS_FAILURE;
   }

/*add by xuezhongbo for zte_binary_body on 20090917*/
   if(bodymessagelen != 0)
   {
	   
	   memset(contentbuf,0,sizeof(contentbuf));
	   sprintf(contentbuf,"%d",bodymessagelen);
       
       startp = strstr(buffer,"Content-Length:"); 	
       
	   startp += strlen("Content-Length:");
       *startp = ' ';
	   startp++;
	   sprintf(startp,"%d\r\n\r\n",bodymessagelen);
          
	   memset(bufmsg,0,sizeof(bufmsg));
	   strcpy(bufmsg,buffer);   

	   memcpy(&bufmsg[strlen(buffer)],bodymessage,bodymessagelen);
	   
       sipsock_send_to(sockfd,sendto_addr, port, ticket->protocol, bufmsg, strlen(buffer)+bodymessagelen);


   }
   else
/*end add*/   	
      sipsock_send_to(sockfd,sendto_addr, port, ticket->protocol,buffer, strlen(buffer));


   
   osip_free (buffer);

  /*
   * RFC 3261, Section 16.6 step 11
   * Proxy Behavior - Set timer C
   */
  /* NOT IMPLEMENTED - does this really apply for stateless proxies? */

   return STS_SUCCESS;
}

/*
 * PROXY_RESPONSE
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 * RFC3261
 *    Section 16.7: Proxy Behavior - Response Processing
 *    1.  Find the appropriate response context
 *    2.  Update timer C for provisional responses
 *    3.  Remove the topmost Via
 *    4.  Add the response to the response context
 *    5.  Check to see if this response should be forwarded immediately
 *    6.  When necessary, choose the best final response from the
 *        response context
 *    7.  Aggregate authorization header field values if necessary
 *    8.  Optionally rewrite Record-Route header field values
 *    9.  Forward the response
 *    10. Generate any necessary CANCEL requests 
 *
 */
int proxy_response (sip_ticket_t *ticket) {
   int i=0;
   int sts;
   int type;
   struct in_addr sendto_addr, local_ip;
   osip_via_t *via;
   int port;
   char *buffer;
   osip_message_t *response;
   struct sockaddr_in *from;
   int sockfd = 0;
   DEBUGC(DBCLASS_PROXY,"proxy_response");

   if (ticket==NULL) {
      ERROR("proxy_response: called with NULL ticket");
      return STS_FAILURE;
   }
   
   /*add by xuezhongbo for only del route on 20080319*/
   route_del(ticket);
   /*end add*/

   response=ticket->sipmsg;
   from=&ticket->from;

/*del by xuezhongbo for better way choose right ip addr on  20081024*/
#if 0
   /* Determine local address to use based on the To: address */
   get_local_ip(response->to->url->host, &local_ip);
   DEBUGC(DBCLASS_PROXY, "local IP: %s", inet_ntoa(local_ip));
#endif
/*end del*/

/*del by xuezhongbo for conflict on 20080411*/
#if 0
   /*
    * RFC 3261, Section 16.7 step 3
    * Proxy Behavior - Response Processing - Remove my Via header field value
    */
   /* remove my Via header line */
   sts = sip_del_myvia(ticket, &local_ip);
   if (sts == STS_FAILURE) {
      DEBUGC(DBCLASS_PROXY,"not addressed to my VIA, ignoring response");
      return STS_FAILURE;
   }
#endif
/*end del*/

   /*
    * figure out if this is an request coming from the outside
    * world to one of our registered clients
    */

   /* Ahhrghh...... a response seems to have NO contact information... 
    * so let's take FROM instead...
    * the TO and FROM headers are EQUAL to the request - that means 
    * they are swapped in their meaning for a response...
    */

#if _OLD_DIRECTION_EVALUATION
   type = 0;
   for (i=0; i<URLMAP_SIZE; i++) {
      if (urlmap[i].active == 0) continue;

      /* incoming response ('from' == 'masq') || ('from' == 'reg') */
      if ((compare_url(response->from->url, urlmap[i].reg_url)==STS_SUCCESS) ||
          (compare_url(response->from->url, urlmap[i].masq_url)==STS_SUCCESS)) {
         type=RESTYP_INCOMING;
         DEBUGC(DBCLASS_PROXY,"incoming response for %s@%s from outbound",
	   response->from->url->username? response->from->url->username:"*NULL*",
	   response->from->url->host? response->from->url->host : "*NULL*");
	 break;
      }

      /* outgoing response ('to' == 'reg') || ('to' == 'masq' ) */
      if ((compare_url(response->to->url, urlmap[i].masq_url)==STS_SUCCESS) ||
          (compare_url(response->to->url, urlmap[i].reg_url)==STS_SUCCESS)){
         type=RESTYP_OUTGOING;
         DEBUGC(DBCLASS_PROXY,"outgoing response for %s@%s from inbound",
	        response->from->url->username ?
                   response->from->url->username : "*NULL*",
	        response->from->url->host ? 
                   response->from->url->host : "*NULL*");
	 break;
      }
   }
#else
   type = 0;
   /*
    * did I receive the telegram from a REGISTERED host?
    * -> it must be an OUTGOING response
    */
   for (i=0; i<URLMAP_SIZE; i++) {
      struct in_addr tmp_addr;
      if (urlmap[i].active == 0) continue;

      if ((urlmap[i].true_url == NULL)||(get_ip_by_host(urlmap[i].true_url->host, &tmp_addr) == STS_FAILURE)) {
         DEBUGC(DBCLASS_PROXY, "proxy_response: cannot resolve host [%s]",
             urlmap[i].true_url==NULL?"*NULL":(urlmap[i].true_url->host==NULL?"*NULL*":urlmap[i].true_url->host));
      } else {
         DEBUGC(DBCLASS_PROXY, "proxy_response: reghost:%s ip:%s",
                urlmap[i].true_url->host, utils_inet_ntoa(from->sin_addr));
         if (memcmp(&tmp_addr, &from->sin_addr, sizeof(tmp_addr)) == 0) {
            type=RESTYP_OUTGOING;
	    break;
         }
      }
   }
   if(type != RESTYP_OUTGOING)
   {
       type = RESTYP_INCOMING;
       /*add by xuezhongbo for conflit on 20080411*/       
       /*
        * RFC 3261, Section 16.7 step 3
        * Proxy Behavior - Response Processing - Remove my Via header field value
        */
/*del by xuezhongbo for better way choose right ip addr on  20081024*/
       {
           struct sockaddr_in localaddr;
           /* mod by weigy on 20100531 for the multi wan route select */
           #if MULTI_WAN
           if(get_ip_by_ifname(ticket->sctable_num ? configuration.outbound[ticket->sctable_num-1].name : 
	            find_outboundif(ticket->lan),&localaddr.sin_addr) == STS_SUCCESS)      
           #else
           if(get_ip_by_ifname(configuration.outbound_if,&localaddr.sin_addr) == STS_SUCCESS)      
           #endif
           /* mod end */
           {
               local_ip = localaddr.sin_addr;
           }
		   else
		   {
		       get_local_ip(response->to->url->host, &local_ip);
		   }
		   DEBUGC(DBCLASS_PROXY, "local IP: %s", inet_ntoa(local_ip));
       }
/*end del*/	   
       /* remove my Via header line */
       sts = sip_del_myvia(ticket, &local_ip);
       if (sts == STS_FAILURE) 
       {
          DEBUGC(DBCLASS_PROXY,"not addressed to my VIA, ignoring response");
          return STS_FAILURE;
       }         
       /*end add*/
   }

   #if 0
   /*
    * is the telegram directed to an internal registered host?
    * -> it must be an INCOMING response
    */
   if (type == 0) {
      for (i=0; i<URLMAP_SIZE; i++) {
         if (urlmap[i].active == 0) continue;
         /* incoming response ('from' == 'masq') || ('from' == 'reg') */
         if ((compare_url(response->from->url, urlmap[i].reg_url)==STS_SUCCESS) ||
             (compare_url(response->from->url, urlmap[i].masq_url)==STS_SUCCESS)) {
            type=RESTYP_INCOMING;
	    break;
         }
      }
   }
/* &&&& Open Issue &&&&
   it has been seen with cross-provider calls that the FROM may be 'garbled'
   (e.g 1393xxx@proxy01.sipphone.com for calls made sipphone -> FWD)
   How can we deal with this? Should I take into consideration the 'Via'
   headers? This is the only clue I have, pointing to the *real* UA.
   Maybe I should put in a 'siproxd' ftag value to recognize it a header
   put in by myself
*/
   if ((type == 0) && (!osip_list_eol(response->vias, 0))) {
      osip_via_t *via;
      struct in_addr addr_via, addr_myself;
      int port_via, port_ua;

      /* get the via address */
      via = (osip_via_t *) osip_list_get (response->vias, 0);
      DEBUGC(DBCLASS_PROXY, "proxy_response: check via [%s] for "
             "registered UA",via->host);
      sts=get_ip_by_host(via->host, &addr_via);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_DNS, "proxy_response: cannot resolve VIA [%s]",
                via->host);
      } else {

         for (i=0; i<URLMAP_SIZE; i++) {
            if (urlmap[i].active == 0) continue;
            /* incoming response (1st via in list points to a registered UA) */
            sts=get_ip_by_host(urlmap[i].true_url->host, &addr_myself);
            if (sts == STS_FAILURE) {
               DEBUGC(DBCLASS_DNS, "proxy_response: cannot resolve "
                      "true_url [%s]", via->host);
               continue;
            }

            port_via=0;
            if (via->port) port_via=atoi(via->port);
            if (port_via <= 0) port_via=SIP_PORT;

            port_ua=0;
            if (urlmap[i].true_url->port)
               port_ua=atoi(urlmap[i].true_url->port);
            if (port_ua <= 0) port_ua=SIP_PORT;

            DEBUGC(DBCLASS_BABBLE, "proxy_response: checking for registered "
                   "host [%s:%i] <-> [%s:%i]",
                   urlmap[i].true_url->host, port_ua,
                   via->host, port_via);

            if ((memcmp(&addr_myself, &addr_via, sizeof(addr_myself))==0) &&
                (port_via == port_ua)) {
               type=RESTYP_INCOMING;
	       break;
            }
         }
      }
   }
#endif
#endif
   ticket->direction=type;

/*
 * ok, we got a response that we are allowed to process.
 */
   switch (type) {
  /*
   * from an external host to the internal masqueraded host
   */
   case RESTYP_INCOMING:
      DEBUGC(DBCLASS_PROXY,"incoming response for %s@%s from outbound",
	         response->from->url->username? response->from->url->username:"*NULL*",
	         response->from->url->host? response->from->url->host : "*NULL*");

      /*
       * Response for INVITE - deal with RTP data in body and
       *                       start RTP proxy stream(s). In case
       *                       of a negative answer, stop RTP stream
       */
      if (MSG_IS_RESPONSE_FOR(response,"INVITE")) {
         /* positive response, start RTP stream */
         if ((MSG_IS_STATUS_1XX(response)) || 
              (MSG_IS_STATUS_2XX(response))) {
            if (configuration.rtp_proxy_enable == 1) {
               DEBUGC(DBCLASS_PROXY,"here");
               /* mod by weigy on 20100531 for the multi wan route select */
               #if MULTI_WAN
               sts = proxy_rewrite_invitation_body(response, DIR_INCOMING, NULL, ticket);
               #else
               sts = proxy_rewrite_invitation_body(response, DIR_INCOMING, NULL);
               #endif
               /* mod end */
            }
         /* negative - stop a possibly started RTP stream */
         } else if ((MSG_IS_STATUS_4XX(response))  ||
                     (MSG_IS_STATUS_5XX(response)) ||
                     (MSG_IS_STATUS_6XX(response))) {
            rtp_stop_fwd(osip_message_get_call_id(response), DIR_INCOMING);
            rtp_stop_fwd(osip_message_get_call_id(response), DIR_OUTGOING);
         }
      } /* if INVITE */

      /*
       * Response for REGISTER - special handling of Contact header
       */
      if (MSG_IS_RESPONSE_FOR(response,"REGISTER")) {
         /*
          * REGISTER returns *my* Contact header information.
          * Rewrite Contact header back to represent the true address.
          * Other responses do return the Contact header of the sender.
          */
       DEBUGC(DBCLASS_PROXY,"get status_code %d\n",response->status_code);
       if(response->status_code==200)
       {
        {
         time_t time_now;
         osip_header_t *expires_hdr;
         osip_uri_param_t *expires_param=NULL;
         int expires = 0;
         osip_message_t *sip_msg=ticket->sipmsg;
	     osip_contact_t *contact;        
         time(&time_now);
         
	     DEBUGC(DBCLASS_BABBLE,"RECV 200 for REGISTER time %d\n",time_now);

	     if (sip_msg == NULL) 
		      return STS_FAILURE;
		    
	     if(osip_message_get_expires(ticket->sipmsg, 0, &expires_hdr)==-1)
	     {
	         DEBUGC(DBCLASS_PROXY,"osip_message_get_expires error\n");	     
	     }
         else
         {
             if (expires_hdr && expires_hdr->hvalue) 
             {
	             /* get expires from expires Header */
	              expires=atoi(expires_hdr->hvalue);
	              DEBUGC(DBCLASS_PROXY,"get expires from expires Heade expires %d\n",expires);	              
	         }
         }

         if(osip_message_get_contact(sip_msg, 0, &contact)==-1)
         {
	           DEBUGC(DBCLASS_PROXY,"osip_message_get_contact error\n");
         }
         else 
         {
		     /*
		      * look for an Contact expires parameter - in case of REGISTER
		      * these two are equal. The Contact expires has higher priority!
		      */
		     DEBUGC(DBCLASS_PROXY,"look for an Contact expires paramete\n");
		     if (ticket->sipmsg->contacts && ticket->sipmsg->contacts->node &&
		         ticket->sipmsg->contacts->node->element) 
		     {
		         DEBUGC(DBCLASS_PROXY,"look for an Contact expires paramete 2\n");	       
		                osip_contact_param_get_byname((osip_contact_t*) ticket->sipmsg->contacts->node->element,
		                EXPIRES, 
		                &expires_param);
	         }
	             if (expires_param && expires_param->gvalue) 
	             {
	                 /* get expires from contact Header */
	                 expires=atoi(expires_param->gvalue);
	                 DEBUGC(DBCLASS_PROXY,"get expires from contact Header expires %d\n",expires);	                 
	             }	
	      } 

             for (i=0;i<URLMAP_SIZE;i++){
                if (urlmap[i].active == 0) 
                    continue;
                if (compare_url(response->to->url, urlmap[i].reg_url)==STS_SUCCESS) 
                {
                   	break;
                }
             }

            if(i<URLMAP_SIZE)
            { 
               if(expires == 0 )
                {
                  urlmap[i].expires = 0;
                  DEBUGC(DBCLASS_PROXY,"---turn off urlmap--- %d\n",i);
                }
                else
                {
                  urlmap[i].expires = time_now +expires+30;
                  DEBUGC(DBCLASS_PROXY,"---turn on urlmap---  urlmap[%d].expires and nowtime %d %d\n",i,urlmap[i].expires,time_now);                 
                }
            }
           }
         }
         sip_rewrite_contact(ticket, DIR_INCOMING, NULL);
      }

      /* 
       * Response for SUBSCRIBE
       *
       * HACK for Grandstream SIP phones (with newer firmware like 1.0.4.40):
       *   They send a SUBSCRIBE request to the registration server. In
       *   case of beeing registering directly to siproxd, this request of
       *   course will eventually be forwarded back to the same UA.
       *   Grandstream then does reply with an '202' response (A 202
       *   response merely indicates that the subscription has been
       *   understood, and that authorization may or may not have been
       *   granted), which then of course is forwarded back to the phone.
       *   Ans it seems that the Grandstream can *not* *handle* this
       *   response, as it immediately sends another SUBSCRIBE request.
       *   And this games goes on and on and on...
       *
       *   As a workaround we will transform any 202 response to a
       *   '404 unknown destination'
       *   
       */
{
      osip_header_t *ua_hdr=NULL;
      osip_message_get_user_agent(response, 0, &ua_hdr);
      if (ua_hdr && ua_hdr->hvalue &&
          (osip_strncasecmp(ua_hdr->hvalue,"grandstream", 11)==0) &&
          (MSG_IS_RESPONSE_FOR(response,"SUBSCRIBE")) &&
          (MSG_TEST_CODE(response, 202))) {
         DEBUGC(DBCLASS_PROXY, "proxy_response: Grandstream hack 202->404");
         response->status_code=404;
      }
}
      break;
   
  /*
   * from the internal masqueraded host to an external host
   */
   case RESTYP_OUTGOING:
      DEBUGC(DBCLASS_PROXY,"outgoing response for %s@%s from inbound",
	     response->from->url->username ?
                response->from->url->username : "*NULL*",
	     response->from->url->host ? 
                response->from->url->host : "*NULL*");

      /* rewrite Contact header to represent the masqued address */
      sip_rewrite_contact(ticket, DIR_OUTGOING, NULL);

      /*
       * If an 2xx OK or 1xx response, answer to an INVITE request,
       * rewrite body
       *
       * In case of a negative answer, stop RTP stream
       */
/*mod by xuezhongbo for zte on 20090923*/
#if 0
if (MSG_IS_RESPONSE_FOR(response,"INVITE"))
#else
if ((MSG_IS_RESPONSE_FOR(response,"INVITE"))||(MSG_IS_RESPONSE_FOR(response,"UPDATE")))
#endif
/*end mod*/
       {
         /* positive response, start RTP stream */
         if ((MSG_IS_STATUS_1XX(response)) || 
              (MSG_IS_STATUS_2XX(response))) {
            /* This is an outgoing response, therefore an outgoing stream */
            /* mod by weigy on 20100531 for the multi wan route select */
            #if MULTI_WAN
            sts = proxy_rewrite_invitation_body(response, DIR_OUTGOING, NULL, ticket);
            #else
            sts = proxy_rewrite_invitation_body(response, DIR_OUTGOING, NULL);
            #endif
            /* mod end */
         /* megative - stop a possibly started RTP stream */
         } else if ((MSG_IS_STATUS_4XX(response))  ||
                     (MSG_IS_STATUS_5XX(response)) ||
                     (MSG_IS_STATUS_6XX(response))) {
            rtp_stop_fwd(osip_message_get_call_id(response), DIR_INCOMING);
            rtp_stop_fwd(osip_message_get_call_id(response), DIR_OUTGOING);
         }
      } /* if INVITE */

      break;
   
   default:
      DEBUGC(DBCLASS_PROXY, "response from/to unregistered UA (%s@%s)",
	   response->from->url->username? response->from->url->username:"*NULL*",
	   response->from->url->host? response->from->url->host : "*NULL*");
      return STS_FAILURE;
   }

   /*
    * for ALL incoming response include my Record-Route header.
    * The local UA will probably send its answer to the topmost 
    * Route Header (8.1.2 of RFC3261)
    */
    if (type == RESTYP_INCOMING) {
       DEBUGC(DBCLASS_PROXY,"Adding my Record-Route");
       /*mod by xuezhongbo for conflict get ori dst on 20080411*/
       #if 0
       route_add_recordroute(ticket);
       #endif
       /*end mod*/
/*add by xuezhongbo for wan interface on 20081028*/
		  int i ;
		  for(i = 0; (i < SCTMAX_SIZE)&&(SCTable[i].sockindex!=ENDINDEX); i++)
		  {
              /* mod by weigy on 20100531 for the multi wan route select */
              #if MULTI_WAN
		      if(SCTable[i].sockfd == sip_udp_socket)
		      {
		          sockfd = SCTable[0].sockfd;
              	  break;
			  }
			  #else
		      if(SCTable[i].sockfd == sip_udp_socket)
		      {
		          if(i%2 != 0)
		          {
		             sockfd =  SCTable[i-1].sockfd ;
		          }
				  else
				  {
				     sockfd =  SCTable[i+1].sockfd ;
				  }
              	  break;
			  }
			  #endif
			  /* mod end */
		  }
		  if((i >= SCTMAX_SIZE)||(SCTable[i].sockindex == ENDINDEX))
		  {
	         ERROR("adding my outbound via failed!");
	         return STS_FAILURE;		  
		  }
/*end add*/	   
    } else {
       /*
        * outgoing packets must not have my record route header, as
        * this likely will contain a private IP address (my inbound).
        */
       DEBUGC(DBCLASS_PROXY,"Purging Record-Routes (outgoing packet)");
       route_purge_recordroute(ticket);
/*add by xuezhongbo for wan interface on 20081028*/
		  int i ;
		  for(i = 0; (i < SCTMAX_SIZE)&&(SCTable[i].sockindex!=ENDINDEX); i++)
		  {
              /* mod by weigy on 20100531 for the multi wan route select */
              #if MULTI_WAN
		      if(SCTable[i].sockfd == sip_udp_socket)
		      {
		          sockfd = SCTable[find_outboundidx(ticket->lan)+1].sockfd;
				  break;
		      }
		      #else
		      if(SCTable[i].sockfd == sip_udp_socket)
		      {
		          if(i%2 != 0)
		          {
		             sockfd =  SCTable[i-1].sockfd ;
		          }
				  else
				  {
				     sockfd =  SCTable[i+1].sockfd ;
				  }
				  break;
		      }
		      #endif
		      /* mod end */
		  }
		  if((i >= SCTMAX_SIZE)||(SCTable[i].sockindex == ENDINDEX))
		  {
	         ERROR("adding my outbound via failed!");
	         return STS_FAILURE;		  
		  }
/*end add*/	   
    }

   /*
    * Determine Next-Hop Address
    */
/*&&&& priority probably should be:
 * 1) Route header
 * 2) fixed outbound proxy
 * 3) SIP URI
 */
   /*
    * check if we need to send to an outbound proxy
    */
   #if 0
   if ((type == RESTYP_OUTGOING) &&
       (sip_find_outbound_proxy(ticket, &sendto_addr, &port) == STS_SUCCESS)) {
      DEBUGC(DBCLASS_PROXY, "proxy_response: have outbound proxy %s:%i",
             utils_inet_ntoa(sendto_addr), port);
   /*
    * Route present?
    * If so, fetch address from topmost Route: header and remove it.
    */
   } else if ((type == RESTYP_OUTGOING) && 
              (response->routes && !osip_list_eol(response->routes, 0))) {
      sts=route_determine_nexthop(ticket, &sendto_addr, &port);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "proxy_response: route_determine_nexthop failed");
         return STS_FAILURE;
      }
      DEBUGC(DBCLASS_PROXY, "proxy_response: have Route header to %s:%i",
             utils_inet_ntoa(sendto_addr), port);
   } else {
      /* get target address and port from VIA header */
      via = (osip_via_t *) osip_list_get (response->vias, 0);
      if (via == NULL) {
         ERROR("proxy_response: list_get via failed");
         return STS_FAILURE;
      }

      sts = get_ip_by_host(via->host, &sendto_addr);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "proxy_response: cannot resolve VIA [%s]",
                via->host);
         return STS_FAILURE;
      }

      if (via->port) {
         port=atoi(via->port);
      } else {
         port=SIP_PORT;
      }
   }
   #endif
   
      /* get target address and port from VIA header */
      via = (osip_via_t *) osip_list_get (response->vias, 0);
      if (via == NULL) {
         ERROR("proxy_response: list_get via failed");
         return STS_FAILURE;
      }
      
      sts = get_ip_by_host(via->host, &sendto_addr);
      if (sts == STS_FAILURE) {
         DEBUGC(DBCLASS_PROXY, "proxy_response: cannot resolve VIA [%s]",
                via->host);
         return STS_FAILURE;
      }

      if (via->port) {
         port=atoi(via->port);
      } else {
         port=SIP_PORT;
      }

   sts = osip_message_to_str(response, &buffer);
   if (sts != 0) {
      ERROR("proxy_response: osip_message_to_str failed");
      return STS_FAILURE;
   }

/*mod by xuezhongbo for wan interface on 20081028*/
#if 0
   sipsock_send(sendto_addr, port, ticket->protocol,
                buffer, strlen(buffer)); 
#else

   sipsock_send_to(sockfd,sendto_addr, port, ticket->protocol,buffer, strlen(buffer));
#endif
/*end mod*/

   osip_free (buffer);
   return STS_SUCCESS;
}


/*
 * PROXY_REWRITE_INVITATION_BODY
 *
 * rewrites the outgoing INVITATION request or response packet
 * 
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
/* mod by weigy on 20100531 for the multi wan route select */
#if MULTI_WAN
int proxy_rewrite_invitation_body(osip_message_t *mymsg, int direction,
				  struct in_addr *local_ip, sip_ticket_t *ticket){
#else
int proxy_rewrite_invitation_body(osip_message_t *mymsg, int direction,
				  struct in_addr *local_ip){
#endif
/* mod end */
   osip_body_t *body;
   sdp_message_t  *sdp;
   struct in_addr map_addr, addr_sess, addr_media, outside_addr, inside_addr;
   int sts;
   char *bodybuff;
   char clen[8]; /* content length: probably never more than 7 digits !*/
   int map_port, msg_port;
   int media_stream_no;
   sdp_connection_t *sdp_conn;
   sdp_media_t *sdp_med;
   int rtp_direction=0;
   int have_c_media=0;
#ifndef LIXB_LAN2LAN_BUG
    int msg_type = NONE_MSG;
#endif

   if (configuration.rtp_proxy_enable == 0) return STS_SUCCESS;

   /*
    * get SDP structure
    */
   sts = osip_message_get_body(mymsg, 0, &body);
   if (sts != 0) {
      if ((MSG_IS_RESPONSE_FOR(mymsg,"INVITE")) &&
          (MSG_IS_STATUS_1XX(mymsg))) {
         /* 1xx responses *MAY* contain SDP data */
         DEBUGC(DBCLASS_PROXY, "rewrite_invitation_body: "
                "no body found in message");
         return STS_SUCCESS;
      } else {
         /* INVITE request and 200 response *MUST* contain SDP data */
         ERROR("rewrite_invitation_body: no body found in message");
         return STS_FAILURE;
      }
   }

   sts = osip_body_to_str(body, &bodybuff);
   if (sts != 0) {
      ERROR("rewrite_invitation_body: unable to sip_body_to_str");
   }
   sts = sdp_message_init(&sdp);
   sts = sdp_message_parse (sdp, bodybuff);

   osip_free(bodybuff);
   if (sts != 0) {
      ERROR("rewrite_invitation_body: unable to sdp_message_parse body");
      sdp_message_free(sdp);
      return STS_FAILURE;
   }


if (configuration.debuglevel)
{ /* just dump the buffer */
   char *tmp, *tmp2;
   sts = osip_message_get_body(mymsg, 0, &body);
   sts = osip_body_to_str(body, &tmp);
   osip_content_length_to_str(mymsg->content_length, &tmp2);
   DEBUG("Body before rewrite (clen=%s, strlen=%i):\n%s\n----",
         tmp2, strlen(tmp), tmp);
   osip_free(tmp);
   osip_free(tmp2);
}

   /*
    * RTP proxy: get ready and start forwarding
    * start forwarding for each media stream ('m=' item in SIP message)
    */

   /* get outbound address */
   /* mod by weigy on 20100531 for the multi wan route select */
   #if MULTI_WAN
   if (get_ip_by_ifname(ticket->sctable_num ? configuration.outbound[ticket->sctable_num-1].name : 
        find_outboundif(ticket->lan), &outside_addr) != 
       STS_SUCCESS) {
      ERROR("can't find outbound interface %s - configuration error?",
            ticket->sctable_num ? configuration.outbound[ticket->sctable_num-1].name : 
            find_outboundif(ticket->lan));
      sdp_message_free(sdp);
      return STS_FAILURE;
   }
   #else
   if (get_ip_by_ifname(configuration.outbound_if, &outside_addr) != 
       STS_SUCCESS) {
      ERROR("can't find outbound interface %s - configuration error?",
            configuration.outbound_if);
      sdp_message_free(sdp);
      return STS_FAILURE;
   }
   #endif
   /* mod end */

   if (local_ip == NULL) {
      /* get inbound address */
      if (get_ip_by_ifname(configuration.inbound_if, &inside_addr) !=
          STS_SUCCESS) {
         ERROR("can't find inbound interface %s - configuration error?",
               configuration.inbound_if);
         sdp_message_free(sdp);
         return STS_FAILURE;
      }
   } else {
      outside_addr = *local_ip;
   }

   /* figure out what address to use for RTP masquerading */
   if (MSG_IS_REQUEST(mymsg)) {
      if (direction == DIR_INCOMING) {
         memcpy(&map_addr, &inside_addr, sizeof (map_addr));
         rtp_direction = DIR_OUTGOING;
      } else {
         memcpy(&map_addr, &outside_addr, sizeof (map_addr));
         rtp_direction = DIR_INCOMING;
      }
#ifndef LIXB_LAN2LAN_BUG
		msg_type = REQUEST_MSG;
#endif
   } else /* MSG_IS_REPONSE(mymsg) */ {
      if (direction == DIR_INCOMING) {
         memcpy(&map_addr, &inside_addr, sizeof (map_addr));
         rtp_direction = DIR_OUTGOING;
      } else {
         memcpy(&map_addr, &outside_addr, sizeof (map_addr));
         rtp_direction = DIR_INCOMING;
      }
#ifndef LIXB_LAN2LAN_BUG
		msg_type = RESPONSE_MSG;
#endif
   }

   DEBUGC(DBCLASS_PROXY, "proxy_rewrite_invitation_body: SIP[%s %s] RTP[%s %s]",
          MSG_IS_REQUEST(mymsg)? "RQ" : "RS",
          (direction==DIR_INCOMING)? "IN" : "OUT",
          (rtp_direction==DIR_INCOMING)? "IN" : "OUT",
          utils_inet_ntoa(map_addr));


   /*
    * first, check presence of a 'c=' item on session level
    */
   if (sdp->c_connection==NULL || sdp->c_connection->c_addr==NULL) {
      /*
       * No 'c=' on session level, search on media level now
       *
       * According to RFC2327, ALL media description must
       * include a 'c=' item now:
       */
      media_stream_no=0;
      while (!sdp_message_endof_media(sdp, media_stream_no)) {
         /* check if n'th media stream is present */
         if (sdp_message_c_addr_get(sdp, media_stream_no, 0) == NULL) {
            ERROR("SDP: have no 'c=' on session level and neither "
                  "on media level (media=%i)",media_stream_no);
            sdp_message_free(sdp);
            return STS_FAILURE;
         }
         media_stream_no++;
      } /* while */
   }

   /* Required 'c=' items ARE present */


   /*
    * rewrite 'c=' item on session level if present and not yet done.
    * remember the original address in addr_sess
    */
   memset(&addr_sess, 0, sizeof(addr_sess));
   if (sdp->c_connection && sdp->c_connection->c_addr) {
      sts = get_ip_by_host(sdp->c_connection->c_addr, &addr_sess);
      if (sts == STS_FAILURE) {
         ERROR("SDP: cannot resolve session 'c=' host [%s]",
               sdp->c_connection->c_addr);
         sdp_message_free(sdp);
         return STS_FAILURE;
      }
      /*
       * Rewrite
       * an IP address of 0.0.0.0 means *MUTE*, don't rewrite such
       */
      /*&&&& should use gethostbyname here */
      if (strcmp(sdp->c_connection->c_addr, "0.0.0.0") != 0) {
         osip_free(sdp->c_connection->c_addr);
         sdp->c_connection->c_addr=osip_malloc(HOSTNAME_SIZE);
         sprintf(sdp->c_connection->c_addr, "%s", utils_inet_ntoa(map_addr));
      } else {
         /* 0.0.0.0 - don't rewrite */
         DEBUGC(DBCLASS_PROXY, "proxy_rewrite_invitation_body: "
                "got a MUTE c= record (on session level - legal?)");
      }
   }


   /*
    * rewrite 'o=' item (originator) on session level if present.
    */
   if (sdp->o_addrtype && sdp->o_addr) {
      if (strcmp(sdp->o_addrtype, "IP4") != 0) {
         ERROR("got IP6 in SDP originator - not yet suported by siproxd");
         sdp_message_free(sdp);
         return STS_FAILURE;
      }

      osip_free(sdp->o_addr);
      sdp->o_addr=osip_malloc(HOSTNAME_SIZE);
      sprintf(sdp->o_addr, "%s", utils_inet_ntoa(map_addr));
   }

/*add by xuezhongbo for change private a attribute. such as: a=alt: 1 1 xxxxx zzzzz 192.168.1.12 44456 on 20081029*/
{
    int maxsize;
    int i = osip_list_size (sdp->m_medias);
	maxsize = osip_list_size (sdp->m_medias);
	for(i = 0;i < maxsize;i++)
	{
	    sdp_message_a_attribute_del(sdp,i,"alt");
	}
}
/*end add*/
    DEBUGC(DBCLASS_PROXY, "complet peel off alt private attribute");
   /*
    * loop through all media descritions,
    * start RTP proxy and rewrite them
    */
   for (media_stream_no=0;;media_stream_no++) {
      /* check if n'th media stream is present */
      if (sdp_message_m_port_get(sdp, media_stream_no) == NULL) break;

      /*
       * check if a 'c=' item is present in this media description,
       * if so -> rewrite it
       */
      memset(&addr_media, 0, sizeof(addr_media));
      have_c_media=0;
      sdp_conn=sdp_message_connection_get(sdp, media_stream_no, 0);
      if (sdp_conn && sdp_conn->c_addr) {
         /*&&&& should use gethostbyname here as well */
         if (strcmp(sdp_conn->c_addr, "0.0.0.0") != 0) {
            sts = get_ip_by_host(sdp_conn->c_addr, &addr_media);
            have_c_media=1;
            /* have a valid address */
            osip_free(sdp_conn->c_addr);
            sdp_conn->c_addr=osip_malloc(HOSTNAME_SIZE);
            sprintf(sdp_conn->c_addr, "%s", utils_inet_ntoa(map_addr));
         } else {
            /* 0.0.0.0 - don't rewrite */
            DEBUGC(DBCLASS_PROXY, "proxy_rewrite_invitation_body: got a "
                   "MUTE c= record (media level)");
         }
      }

      /* start an RTP proxying stream */
      if (sdp_message_m_port_get(sdp, media_stream_no)) {
         msg_port=atoi(sdp_message_m_port_get(sdp, media_stream_no));

         if (msg_port > 0) {
            osip_uri_t *cont_url = NULL;
            char *client_id=NULL;
            /* try to get some additional UA specific unique ID.
             * Try:
             * 1) User part of Contact header
             * 2) Host part of Contact header (will be different
             *    between internal UA and external UA)
             */
            if (!osip_list_eol(mymsg->contacts, 0))
               cont_url = ((osip_contact_t*)(mymsg->contacts->node->element))->url;
            if (cont_url) {
               client_id=cont_url->username;
               if (client_id == NULL) client_id=cont_url->host;
            }


            /*
             * do we have a 'c=' item on media level?
             * if not, use the same as on session level
             */
            if (have_c_media == 0) {
               memcpy(&addr_media, &addr_sess, sizeof(addr_sess));
            }

            sts = rtp_start_fwd(osip_message_get_call_id(mymsg),
                                client_id,
                                rtp_direction,
#ifndef LIXB_LAN2LAN_BUG
								msg_type,
#endif
                                media_stream_no,
                                map_addr, &map_port,
                                addr_media, msg_port);

            if (sts == STS_SUCCESS) {
               /* and rewrite the port */
               sdp_med=osip_list_get(sdp->m_medias, media_stream_no);
               if (sdp_med && sdp_med->m_port) {
                  osip_free(sdp_med->m_port);
                  sdp_med->m_port=osip_malloc(8); /* 5 digits, \0 + align */
                  sprintf(sdp_med->m_port, "%i", map_port);
                  DEBUGC(DBCLASS_PROXY, "proxy_rewrite_invitation_body: "
                         "m= rewrote port to [%i]",map_port);
               } else {
                  ERROR("rewriting port in m= failed sdp_med=%p, "
                        "m_number_of_port=%p", sdp_med, sdp_med->m_port);
               }
            } /* sts == success */
         } /* if msg_port > 0 */
      } else {
         /* no port defined - skip entry */
         WARN("no port defined in m=(media) stream_no=%i", media_stream_no);
         continue;
      }
   } /* for media_stream_no */

   /* remove old body */
   sts = osip_list_remove(mymsg->bodies, 0);
   osip_body_free(body);

   /* dump new body */
   sdp_message_to_str(sdp, &bodybuff);

   /* free sdp structure */
   sdp_message_free(sdp);

   /* include new body */
   osip_message_set_body(mymsg, bodybuff);

   /* free content length resource and include new one*/
   osip_content_length_free(mymsg->content_length);
   mymsg->content_length=NULL;
   sprintf(clen,"%i",strlen(bodybuff));
   sts = osip_message_set_content_length(mymsg, clen);

   /* free old body */
   osip_free(bodybuff);

if (configuration.debuglevel)
{ /* just dump the buffer */

   char *tmp, *tmp2;
 
   sts = osip_message_get_body(mymsg, 0, &body);
    
   sts = osip_body_to_str(body, &tmp);
   osip_content_length_to_str(mymsg->content_length, &tmp2);
   DEBUG("Body after rewrite (clen=%s, strlen=%i):\n%s\n----",
         tmp2, strlen(tmp), tmp);

   osip_free(tmp);

   osip_free(tmp2);

}
   return STS_SUCCESS;
}


/*
 * PROXY_REWRITE_REQUEST_URI
 *
 * rewrites the incoming Request URI
 * 
 * RETURNS
 *	STS_SUCCESS on success
 */
int proxy_rewrite_request_uri(osip_message_t *mymsg, int idx){
   char *host;
   char *port;
   osip_uri_t *url;

   if ((idx >= URLMAP_SIZE) || (idx < 0)) {
      WARN("proxy_rewrite_request_uri: called with invalid index");
      return STS_FAILURE;
   }

   DEBUGC(DBCLASS_PROXY,"rewriting incoming Request URI");
   url=osip_message_get_uri(mymsg);

   /* set the true host */
   if (url->host) osip_free(url->host);url->host=NULL;
   if (urlmap[idx].true_url&&urlmap[idx].true_url->host) {
      DEBUGC(DBCLASS_BABBLE,"proxy_rewrite_request_uri: host=%s",
             urlmap[idx].true_url->host);
      host = (char *)malloc(strlen(urlmap[idx].true_url->host)+1);
      memcpy(host, urlmap[idx].true_url->host, strlen(urlmap[idx].true_url->host));
      host[strlen(urlmap[idx].true_url->host)]='\0';
      osip_uri_set_host(url, host);
   }

   /* set the true port */
   if (url->port) osip_free(url->port);url->port=NULL;
   if (urlmap[idx].true_url&&urlmap[idx].true_url->port) {
      DEBUGC(DBCLASS_BABBLE,"proxy_rewrite_request_uri: port=%s",
             urlmap[idx].true_url->port);
      port = (char *)malloc(strlen(urlmap[idx].true_url->port)+1);
      memcpy(port, urlmap[idx].true_url->port, strlen(urlmap[idx].true_url->port));
      port[strlen(urlmap[idx].true_url->port)]='\0';
      osip_uri_set_port(url, port);
   }
   return STS_SUCCESS;
}
