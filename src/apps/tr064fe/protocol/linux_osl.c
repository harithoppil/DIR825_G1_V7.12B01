/*
 * $Copyright Open Broadcom Corporation$
 *
 * $Id: linux_osl.c,v 1.6.20.2 2003/10/31 21:31:36 mthawani Exp $
 */

#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/socket.h>
#include "upnp_osl.h"
#include "upnp_dbg.h"
#include "upnp.h"
#include "autoconf.h"

struct ip_mreqn2
{
	struct in_addr	imr_multiaddr;		/* IP multicast address of group */
	struct in_addr	imr_address;		/* local IP address of interface */
#ifdef CONFIG_IGMP_PASSTHROUGH	
	struct in_addr 	imr_source;	/* local IP address of interface. Be careful, the element must be at the same location as the struct ip_mreq */
#endif
	int		imr_ifindex;		/* Interface index */
};

struct in_addr *osl_ifaddr(const char *ifname, struct in_addr *inaddr)
{
   int sockfd;
   struct ifreq ifreq;

   if ((sockfd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0) 
   {
      perror("socket");
      return NULL;
   }

   strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
   if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0) 
   {
      inaddr = NULL;
   }
   else 
   {
      memcpy(inaddr, &(((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr), sizeof(struct in_addr));
   }
   close(sockfd);
   return inaddr;
}

int osl_join_multicast(struct iface *pif, int fd, ulong ipaddr, ushort port)
{
   struct ip_mreqn2    mcreqn;
   struct ifreq       ifreq;
   struct sockaddr_in mcaddr;
   int success = FALSE;
   int flag;

   do 
   {
      /* make sure this interface is capable of MULTICAST... */
      memset(&ifreq, 0, sizeof(ifreq));
      strcpy(ifreq.ifr_name, pif->ifname);
      
	  
      if (ioctl(fd, SIOCGIFFLAGS, (int) &ifreq)) 
      {
         break;
      }
      if ((ifreq.ifr_flags & IFF_MULTICAST) == 0) 
      {
         break;
      }

      /* bind the socket to an address and port. */
      flag = 1;
      setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &flag, sizeof(flag));

      memset(&mcaddr, 0, sizeof(mcaddr));
      mcaddr.sin_addr.s_addr = htonl( INADDR_ANY );
      mcaddr.sin_family = AF_INET;
      mcaddr.sin_port = htons(port);
      if ( bind(fd, (struct sockaddr *) &mcaddr, sizeof(mcaddr)) ) 
      {
         break;
      }
	  
      /* join the multicast group. */
      memset(&ifreq, 0, sizeof(ifreq));
      strcpy(ifreq.ifr_name, pif->ifname);
      if (ioctl(fd, SIOCGIFINDEX, &ifreq)) 
      {
         break;
      }
      memset(&mcreqn, 0, sizeof(mcreqn));
      mcreqn.imr_multiaddr.s_addr = ipaddr;
      /* if we get to use struct ip_mreqn2, delete the previous line and uncomment the next two */
      mcreqn.imr_address.s_addr = mcaddr.sin_addr.s_addr;
      mcreqn.imr_ifindex = ifreq.ifr_ifindex;
	  
      if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mcreqn, sizeof(mcreqn))) 
      {
         UPNP_ERROR((" IP_ADD_MEMBERSHIP  ifr_ifindex=%d \n", ifreq.ifr_ifindex));
		 perror("upnp");
         break;
      }
      
      /* restrict multicast messages sent on this socket 
         to only go out this interface and no other
         (doesn't say anything about multicast receives.)
      */

      if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, (char*) &pif->inaddr, sizeof(pif->inaddr))) 
      {
         break;
      }
      success = TRUE;
      
   } while (0);

   /* TRUE == success, FALSE otherwise. */
   return success;
}

