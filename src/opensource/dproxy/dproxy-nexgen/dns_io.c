#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "dns_io.h"
#include "dns_decode.h"
#include "string.h"
#include <include/tbs_ioctl.h>

extern ST_LAN_INFO g_defaultLanInfo;

/*****************************************************************************/


int dns_read_packet(int sock, dns_request_t *m, int protocol, unsigned char ucFlag)
{
    unsockaddr unsa;
    socklen_t salen;
    char szRecvDev[64] = {0};
    ST_LAN_INFO *pLanInfo = NULL;
    /* Read in the actual packet */
    salen = sizeof(unsa);
    char addr[64] = {0};
    int ret;

    m->numread = recvfrom(sock, m->original_buf, sizeof(m->original_buf), 0,
		                  (struct sockaddr *)&unsa, &salen);
    if ( m->numread < 0) {
        debug_perror("dns_read_packet: recvfrom error\n");
        return -1;
    }

    /* TODO: check source addr against list of allowed hosts */
    /* record where it came from */
    if (protocol == PROTO_IPV4) {
        inet_ntop(AF_INET, &unsa.sa_in.sin_addr, addr, sizeof(addr));
        memcpy(&m->src_addr.in_addr, &unsa.sa_in.sin_addr, sizeof(struct in_addr));
        m->src_port = ntohs(unsa.sa_in.sin_port);
        m->protocol = PROTO_IPV4;
	        /*记录lan侧报文是ipv4还是ipv6*/
        if (ucFlag)
        {
           m->srcisipv4 = 1;
        }
    }
    else
    {
        inet_ntop(AF_INET6, &unsa.sa_in6.sin6_addr, addr, sizeof(addr));
        memcpy(&m->src_addr.in6_addr, &unsa.sa_in6.sin6_addr, sizeof(struct in6_addr));
        m->src_port = ntohs(unsa.sa_in6.sin6_port);
        m->protocol = PROTO_IPV6;
	        /*记录lan侧报文是ipv4还是ipv6*/
        if (ucFlag)
        {
           m->srcisipv4 = 0;
        }
    }

    debug("recv packet from %s\n", addr);
    debug("flag = %d\n", ucFlag);
  	if (ucFlag)
  	{
	  	ret = ioctl(sock, SIOCGRECVIF, szRecvDev);
	  	debug("ret=%d\n", ret);
	  	if (ret < 0) {
            debug_perror("call ioctl failed: %s\n", strerror(errno));

            /*规避接口不可用的情况*/
            if(0 == strlen(szRecvDev))
            {
                strcpy(szRecvDev, "eth0.1");
            }
	  	}
        else
        {
    	  	debug("--------------%s----------, len = %d\n", szRecvDev, strlen(szRecvDev));
    		for (pLanInfo = g_stConfig.pLanInfo; pLanInfo; pLanInfo = pLanInfo->pNext)
    		{
    			if (!strncmp(pLanInfo->szName, szRecvDev, strlen(pLanInfo->szName)))
    			{
    				debug("find %s\n", szRecvDev);
    				break;
    			}
    		}
        }

		if (!pLanInfo) /*没有找到对应的接口*/
		{
	  		debug("can not find %s\n", szRecvDev);
#ifdef DEBUG_ERROR_LOG
			FILE *fp = fopen(ERROR_LOG, "w+");
			if (fp)
			{
				fprintf(fp, "from %s, dev %s\n", addr, szRecvDev);
				fclose(fp);
			}
#endif
			// FILE *fp_config = NULL;
			FILE *fp_resolv = NULL;

			char resv_buff[256] = {0};
			char resv_dns[64] = {0};
			char buffer[64] = {0};
			int ret = -1;
			int i = 0, j = 0;

            /*分配新的ST_LAN_INFO结构*/
			pLanInfo = (ST_LAN_INFO*)malloc(sizeof(ST_LAN_INFO));
			if (!pLanInfo) {
				return -2;
			}
			memset(pLanInfo, 0, sizeof(ST_LAN_INFO));
			/*加入链表*/
			pLanInfo->pNext = g_stConfig.pLanInfo;
			g_stConfig.pLanInfo = pLanInfo;

			strncpy(pLanInfo->szName, szRecvDev, sizeof(pLanInfo->szName)-1);
			snprintf(pLanInfo->szCacheFile, sizeof(pLanInfo->szCacheFile),
			         "/var/cache/%s.cache", szRecvDev);

			// fp_config = fopen(g_stConfig.szConfigFile, "a+");
			// if (!fp_config)
			// 	return -2;

			fp_resolv = fopen("/etc/resolv.conf", "r+");
			if(fp_resolv != NULL) {
				while(fgets(resv_buff, sizeof(resv_buff), fp_resolv)) {
					memset(resv_dns, 0, sizeof(resv_dns));
					ret = sscanf(resv_buff, "nameserver %s", resv_dns);
					if(1 == ret) {
						snprintf(buffer, sizeof(buffer), "%s %s\n", szRecvDev, resv_dns);
						// fwrite(buffer, sizeof(char), strlen(buffer), fp_config);
						debug("default dns = %s\n", resv_dns);
						if (strchr(resv_dns, '.')) {
                            /*IPv4*/
                            if (i < NUM_OF_DNS)
						        strcpy(pLanInfo->szDnsIp[PROTO_IPV4][i++], resv_dns);
                            if(strcmp(resv_dns, "0.0.0.0"))
                            {
                              pLanInfo->ipPro |= IPV4_ENABLE ;
                            }
                        }
						else
						{
						    if (j < NUM_OF_DNS)
                                strcpy(pLanInfo->szDnsIp[PROTO_IPV6][j++], resv_dns);

                            pLanInfo->ipPro |= IPV6_ENABLE ;
						}

						if(i >= NUM_OF_DNS && j >= NUM_OF_DNS) {
							break;
						}
					}
				}
			}
			// fclose(fp_config);
			fclose(fp_resolv);
		}
		m->pNode = pLanInfo;

        /*检查dns是否可用,不可用，切换 */
        if(m->protocol == PROTO_IPV4)
        {
            if(pLanInfo &&(!(pLanInfo->ipPro & IPV4_ENABLE)))
            {
                m->protocol = PROTO_IPV6;
            }
        }
        else
        {
            if(pLanInfo &&(!(pLanInfo->ipPro & IPV6_ENABLE)))
            {
                m->protocol = PROTO_IPV4;
            }
        }

		m->ucIndex = 0;
  	}

    /* check that the message is long enough */
    if( m->numread < sizeof(m->message.header) ) {
        debug("dns_read_packet: packet from '%s' to short to be dns packet\n", addr);
        return -1;
    }

    /* reset cache flag */
    m->cache=0;

    /* pass on for full decode */
    dns_decode_request( m );
    return 0;
}

/*****************************************************************************/


int dns_write_packet(int sock, unaddr *addr, int port, dns_request_t *m, int request)
{
    unsockaddr unsa;
	struct ifreq ifr;
    int retval;

    if (request)
    {
        /* Adding setsockopt to set a mark value for policy route. */
        strcpy(ifr.ifr_name, m->pNode->szWanIf);
        if (setsockopt(sock, SOL_SOCKET, SO_BINDDEVICE, &ifr, sizeof(ifr)) < 0)
        {
            debug("dns_write_packet: Unable to bind device %s, Error=%d\n", ifr.ifr_name, errno);
        }
    }

    /* Zero it out */
    memset((void *)&unsa, 0, sizeof(unsa));

    /* Fill in the information */
    if (m->protocol == PROTO_IPV4)
    {
        memcpy(&unsa.sa_in.sin_addr, &addr->in_addr, sizeof(struct in_addr));
        unsa.sa_in.sin_port = htons(port);
        unsa.sa_in.sin_family = AF_INET;
    }
    else
    {
        memcpy(&unsa.sa_in6.sin6_addr, &addr->in6_addr, sizeof(struct in6_addr));
        unsa.sa_in6.sin6_port = htons(port);
        unsa.sa_in6.sin6_family = AF_INET6;
    }

    retval = sendto(sock, m->original_buf, m->numread, 0,
		            (struct sockaddr *)&unsa.sa, sizeof(unsa));
    if( retval < 0 ) {
        debug_perror("dns_write_packet: sendto\n");
    }
    return retval;
}

