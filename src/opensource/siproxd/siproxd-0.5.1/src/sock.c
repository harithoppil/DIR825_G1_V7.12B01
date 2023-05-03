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

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <linux/sockios.h>

#include <osipparser2/osip_parser.h>

#include "siproxd.h"
#include "log.h"

/**/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/if.h>
/**/
static char const ident[]="$Id: sock.c,v 1.29 2005/01/08 10:05:13 hb9xar Exp $";

/* configuration storage */
extern struct siproxd_config configuration;

/* socket used for sending SIP datagrams */
int sip_udp_socket=0;

extern struct SockControlTable SCTable[SCTMAX_SIZE]; //added by xuezhongbo 2007.11.2

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


/*added by xuezhongbo for ALG v2 2007.11.07:9:24*/
int recvfrom_index(int  s, void *buf, size_t len, int *flags, struct sockaddr *from, socklen_t *fromlen)
{
struct msghdr msg;
struct iovec iov[1];
int n;
struct cmsghdr *cmptr;
union {
	struct cmsghdr cm;
	char control[3*CMSG_SPACE(sizeof(struct in_addr))];
	}control_un;
int opt;

msg.msg_control = control_un.control;
msg.msg_controllen = sizeof(control_un.control);
msg.msg_flags = 0;
msg.msg_name = from;
msg.msg_namelen = *fromlen;
iov[0].iov_base = buf;
iov[0].iov_len = len;
msg.msg_iov = iov;
msg.msg_iovlen = 1;

#ifdef IP_PKTINFO
        /* Set the IP_PKTINFO option (Linux). */
        opt = 1;
        setsockopt(s, SOL_IP, IP_PKTINFO, &opt, sizeof(opt));
#endif

#ifdef IP_RECVDSTADDR
        /* Set the IP_RECVDSTADDR option (BSD). */
        opt = 1;
        setsockopt(s, IPPROTO_IP, IP_RECVDSTADDR, &opt, sizeof(opt));
#endif

#ifdef IP_RECVIF
        /* Set the IP_RECVDSTADDR option (BSD). */
        opt = 1;
        setsockopt(s, IPPROTO_IP, IP_RECVDSTADDR, &opt, sizeof(opt));
#endif

bzero(&pktp,sizeof(pktp));

if((n = recvmsg(s,&msg,*flags))<0)
 return n;


*flags=msg.msg_flags;
if(msg.msg_controllen<sizeof(struct cmsghdr) || (msg.msg_flags&MSG_CTRUNC))
	return n;

for(cmptr = CMSG_FIRSTHDR(&msg);cmptr!=NULL;cmptr=CMSG_NXTHDR(&msg, cmptr))
{

#ifdef IP_RECVDSTADDR
  if(cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVDSTADDR)
  {
   memcpy(&pktp.ipi_addr,CMSG_DATA(cmptr),sizeof(struct in_addr));
   continue;
  }
#endif

#ifdef IP_RECVIF
  if(cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_RECVIF)
  {
   struct sockaddr_dl *sdl;
   sdl = (struct sockaddr_dl *)CMSG_DATA(cmptr);
   pktp.ipi_ifindex = sdl->sdl_index;
   continue;
  }
#endif

#ifdef IP_PKTINFO
 if (cmptr->cmsg_level == SOL_IP && cmptr->cmsg_type == IP_PKTINFO) {
     struct in_pktinfo *i =(struct in_pktinfo *)CMSG_DATA(cmptr);
       /*get the index */
	 pktp.ipi_ifindex = i->ipi_ifindex;

	 /*get the dst ip addr*/
       memcpy(&pktp.ipi_addr,&i->ipi_addr,sizeof(struct in_addr)); 
       break;
                }
#endif
}
return n;
}
/*end added*/


/*added by xuezhongbo for ALG 2007.11.2*/
int sipsock_listen_mult (void) {
struct in_addr ipaddr;
   memset(&ipaddr, 0, sizeof(ipaddr));
   int i = 0;
/*modified  by xuezhongbo for ALG 2007.10.31*/   
#if 0     
sip_udp_socket=sockbind(ipaddr, configuration.sip_listen_port, 1);   
if (sip_udp_socket == 0) return STS_FAILURE; 
/* failure*/   
#else    
for(i=0;i<SCTMAX_SIZE;i++)   
{    
	if(SCTable[i].sockindex == ENDINDEX) 		
		break;    
	else 	
	{	
		SCTable[i].sockfd = creatsock();  
		/*add by xuezhongbo for wan interface on 20081028*/
        /* mod by weigy on 20100531 for the multi wan route select */
        #if MULTI_WAN
		if(i != 0)
		{
		    if (configuration.outbound[i-1].name[0] == 0)
		    {
		        SCTable[i].sockindex = ENDINDEX;
		        break;
		    }
		    ipaddr.s_addr = configuration.outbound[i-1].addr.s_addr;
		    dobind(SCTable[i].sockfd,ipaddr,SCTable[i].udpport);	
            BindSockToInterface(SCTable[i].sockfd,configuration.outbound[i-1].name);
            DEBUGC(DBCLASS_NET,"sipsock_listen_mult1 SCTable[%d].sockfd=%d, udpport=%d, outbound=%s", i, SCTable[i].sockfd, SCTable[i].udpport, configuration.outbound[i-1].name);
		}
		else
		{
		    ipaddr.s_addr = configuration.inboundaddr.s_addr;
		    dobind(SCTable[i].sockfd,ipaddr,SCTable[i].udpport);
			BindSockToInterface(SCTable[i].sockfd,configuration.inbound_if);
            DEBUGC(DBCLASS_NET,"sipsock_listen_mult2 SCTable[%d].sockfd=%d, udpport=%d, inbound_if=%s", i, SCTable[i].sockfd, SCTable[i].udpport, configuration.inbound_if);
		}
        #else
		if(i%2 != 0)
		{
		    ipaddr.s_addr = configuration.outboundaddr.s_addr;
		    dobind(SCTable[i].sockfd,ipaddr,SCTable[i].udpport);	
            BindSockToInterface(SCTable[i].sockfd,configuration.outbound_if);
		}
		else
		{
		    ipaddr.s_addr = configuration.inboundaddr.s_addr;
		    dobind(SCTable[i].sockfd,ipaddr,SCTable[i].udpport);
			BindSockToInterface(SCTable[i].sockfd,configuration.inbound_if);
		}
        #endif
        /* mod end */
		/*end add*/
		if(SCTable[i].sockfd) 
		   SCTable[i].state |= SockOpened;  
		else 
		   return STS_FAILURE;
	}   
}   
#endif   
/*end modified*/

   INFO("bound to port %i", configuration.sip_listen_port);
   DEBUGC(DBCLASS_NET,"bound socket %i",sip_udp_socket);
   return STS_SUCCESS;
}

/*end added*/

/*
 * binds to SIP UDP socket for listening to incoming packets
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int sipsock_listen (void) {
   struct in_addr ipaddr;
   memset(&ipaddr, 0, sizeof(ipaddr));
   int i = 0;
  
   sip_udp_socket=sockbind(ipaddr, configuration.sip_listen_port, 1);   
   if (sip_udp_socket == 0) return STS_FAILURE; 
   /* failure*/   


   INFO("bound to port %i", configuration.sip_listen_port);
   DEBUGC(DBCLASS_NET,"bound socket %i",sip_udp_socket);
   return STS_SUCCESS;
}

/*added by xuezhongbo for ALG 2007.11.2*/
int sipsock_wait_mult (void)
{
   int sts;
   fd_set fdset;
   struct timeval timeout;
   int i=0; //added by xuezhongbo for ALG 2007.11.2
   int maxfd=-1; //added by xuezhongbo for ALG 2007.11.2
   timeout.tv_sec=1;
   timeout.tv_usec=0;
   
   FD_ZERO(&fdset);

/*modified by xuezhongbo for ALG 2007.11.2*/  
#if 0
FD_SET (sip_udp_socket, &fdset);   
sts=select (sip_udp_socket+1, &fdset, NULL, NULL, &timeout); 
#else  
    for(i=0;i<SCTMAX_SIZE;i++) 
	{   
	if(SCTable[i].sockindex==ENDINDEX)    	
		break;   
	else {   	
		if(SCTable[i].state&SockOpened)
			{		
			SCTable[i].state &= ~SockCanRead;		
			SCTable[i].state &= ~SockCanWrite;		
			FD_SET (SCTable[i].sockfd, &fdset);		
			maxfd = maxfd>SCTable[i].sockfd ? maxfd:SCTable[i].sockfd;	
			}   	
		}	  
	}  

    sts=select (maxfd+1, &fdset, NULL, NULL, &timeout);  
#endif  /*end modified*/

   /* WARN on failures */
   if (sts<0) {
      /* WARN on failure, except if it is an "interrupted system call"
         as it will result by SIGINT, SIGTERM */
      if (errno != 4) {
         WARN("select() returned error [%i:%s]",errno, strerror(errno));
      } else {
         DEBUGC(DBCLASS_NET,"select() returned error [%i:%s]",
                errno, strerror(errno));
      }
   }

  /*added by xuezhongbo for ALG 2007.10.31*/  
  if(sts>0)  {  	
  	for(i=0;i<SCTMAX_SIZE;i++)  	
		{  	      
		if(SCTable[i].sockindex==ENDINDEX)    	            
			break;            
		else {   	        
			if(SCTable[i].state&SockOpened) 
				{		     
				if(FD_ISSET(SCTable[i].sockfd, &fdset)) 
					{                     
					SCTable[i].state |=SockCanRead;			  
					SCTable[i].state |=SockCanWrite;			  
                    DEBUGC(DBCLASS_NET,"sipsock_wait_mult1 SCTable[%d].state=%d", i, SCTable[i].state);
					}		   
				else 			
					{                     
					SCTable[i].state &=~SockCanRead;			  
					SCTable[i].state &=~SockCanWrite;				  
                    DEBUGC(DBCLASS_NET,"sipsock_wait_mult2 SCTable[%d].state=%d", i, SCTable[i].state);
					}		  
				}   	      
			}	  	
		}  
	} 
  /*end added*/  
  
   return sts;
}

/*end added*/

/*
 * Wait for incoming SIP message. After a 2 sec timeout
 * this function returns with sts=0
 *
 * RETURNS >0 if data received, =0 if nothing received /T/O), -1 on error
 */
int sipsock_wait(void) {
   int sts;
   fd_set fdset;
   struct timeval timeout;
   int i=0; //added by xuezhongbo for ALG 2007.11.2
   int maxfd=-1; //added by xuezhongbo for ALG 2007.11.2
   timeout.tv_sec=2;
   timeout.tv_usec=0;
   
   FD_ZERO(&fdset);

  FD_SET (sip_udp_socket, &fdset);   
  sts=select (sip_udp_socket+1, &fdset, NULL, NULL, &timeout); 


   /* WARN on failures */
   if (sts<0) {
      /* WARN on failure, except if it is an "interrupted system call"
         as it will result by SIGINT, SIGTERM */
      if (errno != 4) {
         WARN("select() returned error [%i:%s]",errno, strerror(errno));
      } else {
         DEBUGC(DBCLASS_NET,"select() returned error [%i:%s]",
                errno, strerror(errno));
      }
   } 
   return sts;
}

/*
 * read a message from SIP listen socket (UDP datagram)
 *
 * RETURNS number of bytes read
 *         from is modified to return the sockaddr_in of the sender
 */
int sipsock_read(void *buf, size_t bufsize,
                 struct sockaddr_in *from, int *protocol) 
{
   int count;
   socklen_t fromlen;
   int flag = 0;
   fromlen=sizeof(struct sockaddr_in);
   *protocol = PROTO_UDP; /* up to now, unly UDP */
   /* add by weigy on 20100531 for the multi wan route select */
   char *ptr;
   /* add end */

   /*modified by xuezhongbo for ALG v2 2007.11.07:13:27*/
   #if 0
   count=recvfrom(sip_udp_socket, buf, bufsize, 0,
                  (struct sockaddr *)from, &fromlen);
   #else
   count= recvfrom_index(sip_udp_socket,buf,bufsize,&flag,(struct sockaddr *)from, &fromlen);
   #endif
   /*end modified*/
   if (count<0) {
      WARN("recvfrom() returned error [%s]",strerror(errno));
      *protocol = PROTO_UNKN;
   }

   /* mod by weigy on 20100531 for the multi wan route select */
   ptr = get_if_from_socket(sip_udp_socket);
   DEBUGC(DBCLASS_NET,"interface %s received UDP packet from %s, count=%i",
          ptr ? ptr : "NULL", utils_inet_ntoa(from->sin_addr), count);
   /* mod end */
   DUMP_BUFFER(DBCLASS_NETTRAF, buf, count);
   return count;
}


/*
 * sends an UDP datagram to the specified destination
 *
 * RETURNS
 *	STS_SUCCESS on success
 *	STS_FAILURE on error
 */
int sipsock_send(struct in_addr addr, int port, int protocol,
                 char *buffer, int size) {
   struct sockaddr_in dst_addr;
   int sts;

   /* first time: allocate a socket for sending */
   if (sip_udp_socket == 0) {
      ERROR("SIP socket not allocated");
      return STS_FAILURE;
   }

   if (buffer == NULL) {
      ERROR("sipsock_send got NULL buffer");
      return STS_FAILURE;
   }

   if (protocol != PROTO_UDP) {
      ERROR("sipsock_send: only UDP supported by now");
      return STS_FAILURE;
   }

   dst_addr.sin_family = AF_INET;
   memcpy(&dst_addr.sin_addr.s_addr, &addr, sizeof(struct in_addr));
   dst_addr.sin_port= htons(port);

   DEBUGC(DBCLASS_NET,"send UDP packet to %s: %i", utils_inet_ntoa(addr),port);
   DUMP_BUFFER(DBCLASS_NETTRAF, buffer, size);

   sts = sendto(sip_udp_socket, buffer, size, 0,
                (const struct sockaddr *)&dst_addr,
                (socklen_t)sizeof(dst_addr));
   
   if (sts == -1) {
      if (errno != ECONNREFUSED) {
         ERROR("sendto() [%s:%i size=%i] call failed: %s",
               utils_inet_ntoa(addr),
               port, size, strerror(errno));
         return STS_FAILURE;
      }
      DEBUGC(DBCLASS_BABBLE,"sendto() [%s:%i] call failed: %s",
             utils_inet_ntoa(addr), port, strerror(errno));
   }

   return STS_SUCCESS;
}

/*add by xuezhongbo for wan interface on 20081028*/
int creatsock()
{
    int sock;
    sock=socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
       ERROR("socket() call failed: %s",strerror(errno));
       return 0;
    }	
	return sock;
}

int dobind(int sock,struct in_addr ipaddr, int localport)
{
   struct sockaddr_in my_addr;
   int sts;
   int flags;

   memset(&my_addr, 0, sizeof(my_addr));
   my_addr.sin_family = AF_INET;
   memcpy(&my_addr.sin_addr.s_addr, &ipaddr, sizeof(struct in_addr));       

   my_addr.sin_port = htons(localport);
	
   sts=bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr));
   if (sts != 0) {
      close(sock);
      return 0;
   }

   /*
    * It has been seen on linux 2.2.x systems that for some
    * reason (bug?) inside the RTP relay, select()
    * claims that a certain file descriptor has data available to
    * read, a subsequent call to read() or recv() then does block!!
    * So lets make the FD's we are going to use non-blocking, so
    * we will at least survive and not run into a deadlock.
    *
    * There is a way to (more or less) reproduce this effect:
    * Make a local UA to local UA call and then very quickly do
    * HOLD/unHOLD, several times.
    */
   flags = fcntl(sock, F_GETFL);
   if (flags < 0) {
      ERROR("fcntl(F_SETFL) failed: %s",strerror(errno));
      close(sock);
      return 0;
   }
   if (fcntl(sock, F_SETFL, (long) flags | O_NONBLOCK) < 0) {
      ERROR("fcntl(F_SETFL) failed: %s",strerror(errno));
      close(sock);
      return 0;
   }

   return sock;
}

int sipsock_send_to(int sock,struct in_addr addr, int port, int protocol,
                 char *buffer, int size)
{
   struct sockaddr_in dst_addr;
   int sts;
   /* add by weigy on 20100531 for the multi wan route select */
   char *ptr;
   /* add end */

   /* first time: allocate a socket for sending */
   if (sock == 0) {
      ERROR("SIP socket not allocated");
      return STS_FAILURE;
   }

   if (buffer == NULL) {
      ERROR("sipsock_send got NULL buffer");
      return STS_FAILURE;
   }

   if (protocol != PROTO_UDP) {
      ERROR("sipsock_send: only UDP supported by now");
      return STS_FAILURE;
   }

   dst_addr.sin_family = AF_INET;
   memcpy(&dst_addr.sin_addr.s_addr, &addr, sizeof(struct in_addr));
   dst_addr.sin_port= htons(port);

   /* mod by weigy on 20100531 for the multi wan route select */
   ptr = get_if_from_socket(sock);
   DEBUGC(DBCLASS_NET,"interface %s send UDP packet to %s: %i", 
        ptr ? ptr : "NULL", utils_inet_ntoa(addr),port);
   /* mod end */
   DUMP_BUFFER(DBCLASS_NETTRAF, buffer, size);

   sts = sendto(sock, buffer, size, 0,
                (const struct sockaddr *)&dst_addr,
                (socklen_t)sizeof(dst_addr));
   
   if (sts == -1) {
      if (errno != ECONNREFUSED) {
         ERROR("sendto() [%s:%i size=%i] call failed: %s",
               utils_inet_ntoa(addr),
               port, size, strerror(errno));
         return STS_FAILURE;
      }
      DEBUGC(DBCLASS_BABBLE,"sendto() [%s:%i] call failed: %s",
             utils_inet_ntoa(addr), port, strerror(errno));
   }

   return STS_SUCCESS;
}
/*end add*/

/*
 * generic routine to allocate and bind a socket to a specified
 * local address and port (UDP)
 * errflg !=0 log errors, ==0 don't
 *
 * RETURNS socket number on success, zero on failure
 */
int sockbind(struct in_addr ipaddr, int localport, int errflg) {
   struct sockaddr_in my_addr;
   int sts;
   int sock;
   int flags;

   memset(&my_addr, 0, sizeof(my_addr));

   my_addr.sin_family = AF_INET;
   memcpy(&my_addr.sin_addr.s_addr, &ipaddr, sizeof(struct in_addr));
   my_addr.sin_port = htons(localport);

   sock=socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (sock < 0) {
      ERROR("socket() call failed: %s",strerror(errno));
      return 0;
   }

   sts=bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr));

   if (sts != 0) {
      if (errflg) ERROR("bind failed: %s",strerror(errno));
      close(sock);
      return 0;
   }

   /*
    * It has been seen on linux 2.2.x systems that for some
    * reason (bug?) inside the RTP relay, select()
    * claims that a certain file descriptor has data available to
    * read, a subsequent call to read() or recv() then does block!!
    * So lets make the FD's we are going to use non-blocking, so
    * we will at least survive and not run into a deadlock.
    *
    * There is a way to (more or less) reproduce this effect:
    * Make a local UA to local UA call and then very quickly do
    * HOLD/unHOLD, several times.
    */
   flags = fcntl(sock, F_GETFL);
   if (flags < 0) {
      ERROR("fcntl(F_SETFL) failed: %s",strerror(errno));
      close(sock);
      return 0;
   }
   if (fcntl(sock, F_SETFL, (long) flags | O_NONBLOCK) < 0) {
      ERROR("fcntl(F_SETFL) failed: %s",strerror(errno));
      close(sock);
      return 0;
   }

   return sock;
}

/*add by xuezhongbo for wan interface on 20081028*/
int BindSockToInterface(int sockfd, char *ifname)
{
   struct ifreq sipinterface;
   int length = 0;

   if(sockfd < 0)
   {
       return -1;   
   }

   if(ifname == NULL)
   {
       memcpy(sipinterface.ifr_ifrn.ifrn_name,"",IFNAMSIZ);        
       /* add by weigy on 20100531 for the multi wan route select */
       #if MULTI_WAN
       DEBUGC(DBCLASS_NET,"BindSockToInterface 1 sockfd=%d, ifname=%s", 
            sockfd, "NULL");
       #endif
       /* add end */
       return 0;
   }
   else
   {
       strncpy(sipinterface.ifr_ifrn.ifrn_name,ifname,IFNAMSIZ);      
       /* add by weigy on 20100531 for the multi wan route select */
       #if MULTI_WAN
       DEBUGC(DBCLASS_NET,"BindSockToInterface 2 sockfd=%d, ifname=%s", 
            sockfd, ifname);
       #endif
       /* add end */
   }

   /*mod by lixb on 20091218 for return-err */
   #if 1
    int retval=0;
   	length = strlen(sipinterface.ifr_ifrn.ifrn_name);
    /* mod by weigy on 20100531 for the multi wan route select */
   	#if MULTI_WAN
   	if( (retval=setsockopt( sockfd, SOL_SOCKET, SO_BINDDEVICE, sipinterface.ifr_ifrn.ifrn_name, length)) != 0 )
   	#else
   	if( (retval=setsockopt( sockfd, SOL_SOCKET, SO_BINDTODEVICE, sipinterface.ifr_ifrn.ifrn_name, length)) != 0 )
	#endif
   #else
   length = sizeof(sipinterface);
   if( setsockopt( sockfd, SOL_SOCKET, SO_BINDTODEVICE, &sipinterface, length) != 0 )
   #endif
   /*mod end*/
   {
       printf("#### setting socket options ERROR !!! with sockfd=%d ifname=%s len=%d retval=%d error=%d errstr=%s\n",sockfd,ifname, length, retval,errno,strerror(errno));
	   return -1;
   }   

   return 0;
}
/*end add*/
