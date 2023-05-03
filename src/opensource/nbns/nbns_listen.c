/**************************************************************************
aim:listen client pc nbns registration (via udp port 137)
author:zym
date:2012-10-15
description: when client pc register it name by broadcast nbns packets, our ap record and send it's ip to logic dhcps module
**************************************************************************/
/*
链路层数据结构：
----dest----|---src--------|------|-------ip-hdr---|----udp-hdr--|----nbns------
0 1 2 3 4 5 |6 7 8 9 10 11 |12 13 |14 ----------33 |34 -------41 |42----------
链路层数据发送的地址是sockaddr_ll，定义在<netpacket/packet.h>

*/
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
//#include <linux/udp.h>
#include <netinet/ip.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include "nbns_listen.h"
#include <stdarg.h>
#include "autoconf.h"

//#include <linux/if_packet.h> 

int dns_sock_raw;
int llmnr_sock_raw;

#define MAX_LEN 2048
#define DATAMAX 128
#define UDP_PORT 137
#define FLUSHTIME 120

#define USE_IPV6

static char ipcache[2048]={0};
char match_str[DATAMAX] ={0};
char match_str_local[DATAMAX] ={0};
char match_str_mac[DATAMAX] ={0};
char match_str_mac_local[DATAMAX] ={0};

char manageip[16]={0};
char manageipBak[16]={0};

char manageipv6[48]={0};

char raw_buffer[MAX_LEN] = { 0 };//rece buf
char raw6_buffer[MAX_LEN] = { 0 };//rece buf

unsigned char srcmac[6]={0};
unsigned char *mac=NULL;
struct ifreq ifr;

typedef unsigned char __u8;
typedef unsigned short __u16;
struct udphdr {
	__u16	source;
	__u16	dest;
	__u16	len;
	__u16	check;
};

//#define DEBUG 1
#ifdef DEBUG
#define DEBUG_PIRNT(fmt, args...)  printf("%s:%d "fmt, __func__, __LINE__, ##args)
#else
#define DEBUG_PIRNT(fmt, args...)
#endif
#define __LITTLE_ENDIAN_BITFIELD 1
struct ipv6hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
        __u8                    priority:4,
                                version:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
        __u8                    version:4,
                                priority:4;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
        __u8                    flow_lbl[3];

        __u16                   payload_len;
        __u8                    nexthdr;
        __u8                    hop_limit;

        struct  in6_addr        saddr;
        struct  in6_addr        daddr;
};


void nbns_Log(char *szFmt, ...)
{
    static char buff[1024];
    long nLen;
    memset(buff,0, 1024);
    vsnprintf(buff, sizeof(buff), szFmt, (va_list)(&szFmt + 1));
    nLen = strlen(buff);
    if (nLen < 0)
        strcpy(buff, "Overflow");
    else if (nLen > (sizeof(buff) - 4))
        buff[sizeof(buff) - 1] = 0;

    FILE *fp = fopen("/var/nbns.txt", "a");
    fprintf(fp, "%d:%s",getpid(), buff);
    fclose(fp);
}

void printf_data(char * data)
{
    int i;
    DEBUG_PIRNT("\n===========================\n");
    for( i=0; i<16; i++)
        printf("%02x ", data[i]);
    DEBUG_PIRNT("\n===========================\n");
}

#if defined( CONFIG_APPS_HTML_WEB_STYLE_DLINK1155)||defined(CONFIG_APPS_HTML_WEB_STYLE_DLINK1160)
struct arpMsg 
{
    struct ethhdr ethhdr;	 	/* Ethernet header */
    u_short htype;				/* hardware type (must be ARPHRD_ETHER) */
	u_short ptype;				/* protocol type (must be ETH_P_IP) */
    u_char  hlen;				/* hardware address length (must be 6) */
    u_char  plen;				/* protocol address length (must be 4) */
    u_short operation;			/* ARP opcode */
    u_char  sHaddr[6];			/* sender's hardware address */
    u_char  sInaddr[4];			/* sender's IP address */
    u_char  tHaddr[6];			/* target's hardware address */	
    u_char  tInaddr[4];			/* target's IP address */
    u_char  pad[18];			/* pad for min. Ethernet payload (60 bytes) */
};


int arpping(u_int32_t tsaddr, u_int32_t ip, const char *mac, unsigned char *dstmac, char *interface)
{

	int	timeout = 0;
	int 	optval = 1;
	int	s;			/* socket */
	int	rv = -1;			/* return value */
	struct sockaddr addr;		/* for interface name */
	struct arpMsg	arp;
	fd_set		fdset;
	struct timeval	tm;
	//time_t		prevTime;
    int ret = -1;
	
	char chBrdMac[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	if ((s = socket (PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) == -1) {
		printf("%s %d : Could not open raw socket\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1) {
		printf("%s %d : Could not setsocketopt on raw socket\n", __FUNCTION__, __LINE__);
		close(s);
		return -1;
	}

	/* send arp request */
	memset(&arp, 0, sizeof(struct arpMsg));
	memcpy(arp.ethhdr.h_dest, chBrdMac, 6);	/* MAC DA */
	memcpy(arp.ethhdr.h_source, mac, 6);		/* MAC SA */
	arp.ethhdr.h_proto = htons(ETH_P_ARP);		/* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);		/* hardware type */
	arp.ptype = htons(ETH_P_IP);			/* protocol type (ARP message) */
	arp.hlen = 6;					/* hardware address length */
	arp.plen = 4;					/* protocol address length */
	arp.operation = htons(ARPOP_REQUEST);		/* ARP op code */
	memcpy(arp.sInaddr, &ip, sizeof(ip));		/* source IP address */
	memcpy(arp.sHaddr, mac, 6);			/* source hardware address */
	memcpy(arp.tInaddr, &tsaddr, sizeof(tsaddr));	/* target IP address */

	/*DEBUG_PIRNT("sm=%x:%x:%x:%x:%x:%x,si=%x.%x.%x.%x,tm=%x:%x:%x:%x:%x:%x,ti=%x.%x.%x.%x\n", 
		arp.sHaddr[0], arp.sHaddr[1], arp.sHaddr[2], arp.sHaddr[3], arp.sHaddr[4], arp.sHaddr[5],
		arp.sInaddr[0], arp.sInaddr[1], arp.sInaddr[2], arp.sInaddr[3], 
		arp.tHaddr[0], arp.tHaddr[1], arp.tHaddr[2], arp.tHaddr[3], arp.tHaddr[4], arp.tHaddr[5],
		arp.tInaddr[0], arp.tInaddr[1], arp.tInaddr[2], arp.tInaddr[3]);*/
	memset(&addr, 0, sizeof(addr));
	strcpy(addr.sa_data, interface);
	if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0)
		rv = 0;
	
	/* wait arp reply, and check it */
	tm.tv_usec = 250000;
	FD_ZERO(&fdset);
	FD_SET(s, &fdset);
	tm.tv_sec = timeout;
    ret = select(s + 1, &fdset, (fd_set *) NULL, (fd_set *) NULL, &tm);
	if (ret < 0) 
    {
		DEBUG_PIRNT("%s %d : Error on ARPING request\n", __FUNCTION__, __LINE__);
        rv = -1;
	} 
    else if(ret == 0)
    {
        DEBUG_PIRNT("%s %d : Error on ARPING request timeout\n", __FUNCTION__, __LINE__);
    }
    else
    {
        DEBUG_PIRNT("%s %d : ARPING request reply\n", __FUNCTION__, __LINE__);
        if (FD_ISSET(s, &fdset)) 
        {
        	memset(&arp, 0, sizeof(struct arpMsg));
			if (recv(s, &arp, sizeof(arp), 0) < 0 )
            {
                rv = -1;
            }
			if (arp.operation == htons(ARPOP_REPLY) && 
			    bcmp(arp.tHaddr, mac, 6) == 0) 
			{
				memcpy(dstmac, arp.sHaddr, 6);
				/*DEBUG_PIRNT("sm=%x:%x:%x:%x:%x:%x,si=%x.%x.%x.%x,tm=%x:%x:%x:%x:%x:%x,ti=%x.%x.%x.%x\n", 
				arp.sHaddr[0], arp.sHaddr[1], arp.sHaddr[2], arp.sHaddr[3], arp.sHaddr[4], arp.sHaddr[5],
				arp.sInaddr[0], arp.sInaddr[1], arp.sInaddr[2], arp.sInaddr[3], 
				arp.tHaddr[0], arp.tHaddr[1], arp.tHaddr[2], arp.tHaddr[3], arp.tHaddr[4], arp.tHaddr[5],
				arp.tInaddr[0], arp.tInaddr[1], arp.tInaddr[2], arp.tInaddr[3]);*/
				DEBUG_PIRNT("%s %d : Valid arp reply receved for this address\n", __FUNCTION__, __LINE__);
				rv = 0;
			}
		}
     }

	close(s);

	return rv;
}  
#endif
//根据nbname_request可以得知，得到查询的question_name[]位置，其中question_name[]第一字节固定是0x20，接下来才是经编码的数据
//如果解析,编码方式如下，比如数据为a：
//					1：a---->A  2：(A>>4)&0x0f---->buf[i*2], 3:A&0x0f--->buf[2*i+1]   4:将buf数据依次存入question_name[]中.
int DecodeName(unsigned char *in, unsigned char *out)
{
		unsigned char tmpbuf[34]={0};
		unsigned char i=0;
		unsigned char p,q;
		for(i=0;i<17;i++)
		{	
			p = *(in+2*i);
			q = *(in+2*i+1);
			if((p>='A')&&(q>='A'))
				{
					tmpbuf[i]= (((p-'A')&0x0f)<<4)+((q-'A')&0x0f);
				}
				else
					break;
		}
		memcpy(out,tmpbuf,strlen(tmpbuf));
		return 0;	
}

int get_options( int argc, char ** argv )
{
		int c = 0;
		 while( (c = getopt( argc, argv, "s:dhi:k:")) != -1 ) {
        switch(c) {
        	case 's':
        			memcpy(match_str,optarg,strlen(optarg));
        			DEBUG_PIRNT("--------------------------------the string is:%s\n",optarg);
        			break;
        	case 'i':
        			memcpy(manageip,optarg,strlen(optarg));

        			memcpy(manageipBak,optarg,strlen(optarg));
					
        			DEBUG_PIRNT("--------------------------------the string is:%s\n",manageip);
        			break;
			case 'k':
        			memcpy(manageipv6,optarg,strlen(optarg));
        			DEBUG_PIRNT("--------------------------------the string is:%s\n",manageipv6);
        			break;		
        	default:                
              break;
        }
    }
		return 0;
}
unsigned short check_sum(unsigned short *addr,int len) 
{ 
		register int nleft=len; 
		register int sum=0; 
		unsigned char *w=addr; 
		short answer=0; 
		unsigned short ii;
		while(nleft>1) 
		{ 								
				ii= (unsigned short)(*w<<8)+*(w+1);
				sum +=ii;			
				w = w+2;
				nleft-=2; 
		}		
		if(nleft==1) 
		{ 
				*(unsigned char *)(&answer)=*(unsigned char *)w; 
				sum+=answer; 
			} 

			sum=(sum>>16)+(sum&0xffff); 
			sum+=(sum>>16); 
			answer=~sum; 
			return ntohs(answer); 
} 
	
void  toUpper( char   * s) {
     if ( ! s) {
       return ;
    }
     while ( * s) {
        if  ( ( * s >= 'a' )  &&  ( * s <= 'z' ) ) {
         * s += 'A' - 'a' ;
       }
        ++ s;
    }
}
int Compare_DnsName(unsigned char *srcdata, unsigned char *desdata)
{
    int i;
    
    for(i = 0; i < strlen(srcdata); i++)
    {
        DEBUG_PIRNT("%d:%02x %02x\n", i, *(srcdata+i), *(desdata+i+1));
        if( (*(srcdata+i) != *(desdata+i+1))
            && ('.' != *(srcdata+i)) )
        {
            return -1;
        }
    }

    return 0;

}
int get_ipaddr(char * interface_name,char *buf)
{
    int s;
    char *szptr =NULL;
    if((s = socket(PF_INET,SOCK_STREAM,0)) < 0)
    {
       perror("Socket");
       return 0;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name,interface_name);

    if(ioctl(s,SIOCGIFADDR,&ifr) < 0)
    {
       perror("ioctl");
       close(s);
        return -1;
    }
    szptr = (char*)inet_ntoa(((struct sockaddr_in*) (&ifr.ifr_addr))->sin_addr);  
    strcpy(buf,szptr);
    close(s);
    return 0;
}


int get_mac(char * interface_name,char *buf)
{  
    if(strcmp(interface_name,"lo") == 0)
    {
        return 0;
    }

    int s;
   
    if((s = socket(PF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("Socket");
        return -1;
    }

    struct ifreq ifr;
  
    strcpy(ifr.ifr_name,interface_name);

    if(ioctl(s,SIOCGIFHWADDR,&ifr) != 0)
    {
        perror("ioctl");
        close(s);
        return 0;
    }

    u_char * ptr;
    ptr =(u_char *)&ifr.ifr_ifru.ifru_hwaddr.sa_data[0];
   sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",*ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5));
  	close(s);
    return 0;
}


int  get_netmask(char * interface_name,char *buf)
{
    int s;
    char *szptr =NULL;
  
    if((s = socket(PF_INET,SOCK_STREAM,0)) < 0)
    {
       perror("Socket");
       return -1;
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name,interface_name);

    if(ioctl(s,SIOCGIFNETMASK,&ifr) < 0)
    {
       perror("ioctl");
       close(s);
       return -1;
    }
   szptr = (char*)inet_ntoa(((struct sockaddr_in*) (&ifr.ifr_addr))->sin_addr);  
    strcpy(buf,szptr);
 		close(s);
    return 0;
}

int LlmnrPacketProcess(const struct iphdr * ipheader)
{
	struct llmnr_req *req;
	struct llmnr_ans *ans;
	struct udphdr * udpheader = NULL;
	unsigned int i;
	char br0_1ip[16]={0};
	char br0ip[16]={0};
	//char manageip[16]={0};
	char namebuf[17]={0};
	char *address =NULL;
	char br0netmask[16] ={0};
	char br00netmask[16]={0};
	struct ether_header *nbns_ether;
	struct ether_header *ether_tmp;
	struct iphdr * nbns_iph;           //ip head
	struct udphdr * nbns_udpheader;
	struct ifreq ifr1;
	struct ifconf ifconf;
	struct sockaddr_ll sll;
	char *usrdata = NULL;
	unsigned char buf[512];
	unsigned int ipnum[4];
	unsigned char sendbuf[256]={0};
	char *szIpAddr = NULL;
	char IPbuf[16]= {0}, dataSentbuf[256] = { 0 };
	int dns_any_sock;
#if defined( CONFIG_APPS_HTML_WEB_STYLE_DLINK1155)||defined(CONFIG_APPS_HTML_WEB_STYLE_DLINK1160)
	unsigned char dstmac[6]={0};
	int arppingNum = 3;
#endif
	int iRet;
	struct sockaddr_in *addr = NULL;
    struct in6_addr ipaddr;

    DEBUG_PIRNT("-----%s----%d---\n",__FUNCTION__,__LINE__);
	usrdata =(char *) (raw_buffer +sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr)); 
	if((!ipheader)||(!usrdata))
	{
		return -1;
	}    
	szIpAddr = (char *) (raw_buffer +sizeof(struct ether_header)+12);
	sprintf(IPbuf,"%d.%d.%d.%d",(unsigned char) szIpAddr[0],(unsigned char) szIpAddr[1],(unsigned char) szIpAddr[2],(unsigned char) szIpAddr[3]);                                                            
	req = (struct llmnr_req*)usrdata;
	//DecodeName(req->queries.name,namebuf);
	strncpy(namebuf, usrdata+13, req->queries.len);
	req->queries.type = ntohs(*(unsigned short *)(usrdata+14+req->queries.len));
	req->queries.ClassType = ntohs(*(unsigned short *)(usrdata+16+req->queries.len));
	DEBUG_PIRNT("ClassType:%x, type=%x\n",req->queries.ClassType, req->queries.type);
	toUpper(namebuf);
	DEBUG_PIRNT("name:%s\n",namebuf);
	toUpper(match_str);
	toUpper(match_str_local);
	toUpper(match_str_mac);
	toUpper(match_str_mac_local);
	if(((strlen(match_str)) &&
	   (0 == strncmp(namebuf, match_str, strlen(match_str)) ||
	   0 == strncmp(&namebuf[4], match_str, strlen(match_str))))||
	   ((strlen(match_str_local)) &&
	   (0 == strncmp(namebuf, match_str_local, strlen(match_str_local)) ||
	   0 == strncmp(&namebuf[4], match_str_local, strlen(match_str_local))))||
	   ((strlen(match_str_mac)) &&
	   (0 == strncmp(namebuf, match_str_mac, strlen(match_str_mac)) ||
	   0 == strncmp(&namebuf[4], match_str_mac, strlen(match_str_mac))))||
	   ((strlen(match_str_mac_local)) &&
	   (0 == strncmp(namebuf, match_str_mac_local, strlen(match_str_mac_local)) ||
	   0 == strncmp(&namebuf[4], match_str_mac_local, strlen(match_str_mac_local)))))
	{
		memset(br0_1ip,0,sizeof(br0_1ip));
		memset(br0ip,0,sizeof(br0ip)); 
		//memset(manageip,0,sizeof(manageip)); 
		nbns_ether =malloc(sizeof(struct ether_header));
		nbns_iph = malloc(sizeof(struct iphdr));
		nbns_udpheader = malloc(sizeof(struct udphdr));
		ans = malloc(sizeof(struct llmnr_ans));
		if((ans==NULL)||(NULL==nbns_ether)||(NULL==nbns_iph)||(NULL==nbns_udpheader))
		{
			if(NULL != ans)
				free(ans);
			if(NULL != nbns_udpheader)
				free(nbns_udpheader);
			if(NULL != nbns_iph)
				free(nbns_iph);
			if(NULL != nbns_ether)
				free(nbns_ether);
			return -1;
		}

		bzero(&sll, sizeof(sll));
		sll.sll_family = AF_PACKET;
		sll.sll_halen = 6;
		memcpy(sll.sll_addr,raw_buffer+6,6);
		sll.sll_ifindex = ifr.ifr_ifindex; 
		//get_netmask("br0",br0netmask);	
		//get_netmask("br0:0",br00netmask);
		ether_tmp = (struct ether_header *)raw_buffer;

		dns_any_sock = socket(AF_INET, SOCK_DGRAM, 0);//用于获取MAC，IP等地址信息
		if(dns_any_sock<0)
		{
			DEBUG_PIRNT("Could not create dns socket");
			if(NULL != ans)
				free(ans);
			if(NULL != nbns_udpheader)
				free(nbns_udpheader);
			if(NULL != nbns_iph)
				free(nbns_iph);
			if(NULL != nbns_ether)
				free(nbns_ether);
			return -1;
		}
		#if 0
		ifconf.ifc_len = sizeof(buf);
		ifconf.ifc_buf = buf;
		ioctl(dns_any_sock, SIOCGIFCONF, &ifconf);    //获取所有接口信息
		ifr1 = (struct ifreq*)buf;  

		for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
		{
			if(0 == strcmp(ifr1->ifr_name, "br0"))			
			{
				if(strlen(br0netmask)==0)  
					strcpy(br0netmask,"255.255.255.0");
				address = inet_ntoa(((struct sockaddr_in*)&(ifr1->ifr_addr))->sin_addr);                                                                        
				DEBUG_PIRNT("br0======%s=========IP:%s,netmask:%s===========\n",ifr1->ifr_name, address, br0netmask);
				if((inet_addr(address) & inet_addr(br0netmask)) ==  (inet_addr(IPbuf) & inet_addr(br0netmask)))
				{
					DEBUG_PIRNT("find br0 ip\n");
					memcpy(manageip,address,strlen(address));
					break;
				}
				//memcpy(br0_1ip,address,strlen(address));
			}
			else if(0 == strcmp(ifr1->ifr_name, "br0:0"))
			{
				if(strlen(br00netmask)==0)  
					strcpy(br00netmask,"255.255.0.0");
				address = inet_ntoa(((struct sockaddr_in*)&(ifr1->ifr_addr))->sin_addr);                                                                
				DEBUG_PIRNT("br0:0======%s=========IP:%s,netmask:%s===========\n",ifr1->ifr_name, address, br00netmask);
				if((inet_addr(address) & inet_addr(br00netmask)) ==  (inet_addr(IPbuf) & inet_addr(br00netmask)))
				{
					DEBUG_PIRNT("find br0:0 ip\n");
					memcpy(manageip,address,strlen(address));
					break;
				}
				//memcpy(br0ip,address,strlen(address));
			}
			DEBUG_PIRNT("name = [%s]\n", ifr1->ifr_name);
			DEBUG_PIRNT("local addr = [%s]\n", 
			inet_ntoa(((struct sockaddr_in*)&(ifr1->ifr_addr))->sin_addr));          					
			ifr1++;
		}
		#else
		if(req->queries.type == 0x1c)//ipv6 AAA
		{
			 if ( inet_pton ( AF_INET6, manageipv6, &ipaddr ) <= 0 )
		    {
		        goto out;
			}
			printf("04x%04x%04x%04x%04x\n", htonl(ipaddr.s6_addr32[0]),htonl(ipaddr.s6_addr32[1]),htonl(ipaddr.s6_addr32[2]),htonl(ipaddr.s6_addr32[3]));
		}
		else//ipv4 A
		{
			memset(&ifr1, 0, sizeof(struct ifreq)); 
			strcpy(ifr1.ifr_name, "br0");
			addr = (struct sockaddr_in *)&ifr1.ifr_addr;
			addr->sin_family = AF_INET;
			
			iRet = ioctl(dns_any_sock, SIOCGIFADDR, (char *)&ifr1);
			if (iRet < 0) 
			{
				DEBUG_PIRNT("Couldn't get the address of interface br0, err=%d\n", iRet);
				strcpy(manageip, "192.168.0.1");
			}
			else
			{
				memcpy(manageip, inet_ntoa(addr->sin_addr), sizeof(manageip));
			}
		}
		#endif
		if(dns_any_sock > 0)
		{
			close(dns_any_sock);
		}

		if(0 == strlen(manageip))
		{
			DEBUG_PIRNT("manageip not find in same submast\n");
			if(NULL != ans)
				free(ans);
			if(NULL != nbns_udpheader)
				free(nbns_udpheader);
			if(NULL != nbns_iph)
				free(nbns_iph);
			if(NULL != nbns_ether)
				free(nbns_ether);
			return -1;
		}
		
		DEBUG_PIRNT("********manageip:%s,%d\n",manageip,strlen(manageip));
		sscanf(manageip,"%d.%d.%d.%d",&ipnum[0],&ipnum[1],&ipnum[2],&ipnum[3]);	         	

		//设置用户数据包层
		ans->id 			= req->id;
		ans->flags 		= htons(0x8000);//应答包
		ans->question = htons(0x01);	//询问包个数
		ans->answer		= htons(0x01);//应答包个数
		ans->auth			= htons(0x0);//无认证
		ans->add			= htons(0x0);//附加信息
		memcpy(dataSentbuf, ans, 12);
		dataSentbuf[12] = req->queries.len;
		memcpy(&dataSentbuf[13], namebuf, req->queries.len);
		//dataSentbuf[13+req->queries.len] = req->queries.type;
		memcpy(&dataSentbuf[14+req->queries.len], (usrdata+14+req->queries.len), 2);
		//dataSentbuf[15+req->queries.len] = req->queries.ClassType;
		memcpy(&dataSentbuf[16+req->queries.len], (usrdata+16+req->queries.len), 2);
		dataSentbuf[18+req->queries.len] = req->queries.len;
		memcpy(&dataSentbuf[19+req->queries.len], namebuf, req->queries.len);
		//dataSentbuf[18+req->queries.len*2] = req->queries.type;
		memcpy(&dataSentbuf[20+req->queries.len*2], (usrdata+14+req->queries.len), 2);
		//dataSentbuf[20+req->queries.len*2] = req->queries.ClassType;
		memcpy(&dataSentbuf[22+req->queries.len*2], (usrdata+16+req->queries.len), 2);
		dataSentbuf[24+req->queries.len*2] = 0x0;
		dataSentbuf[25+req->queries.len*2] = 0x0;
		dataSentbuf[26+req->queries.len*2] = 0x0;
		dataSentbuf[27+req->queries.len*2] = 0x1e;
		
		if(req->queries.type == 0x1c)//ipv6 AAA
		{
			dataSentbuf[28+req->queries.len*2] = 0x0;
			dataSentbuf[29+req->queries.len*2] = 0x10;
			dataSentbuf[30+req->queries.len*2] = 0xfe;
			dataSentbuf[31+req->queries.len*2] = 0x80;
			dataSentbuf[32+req->queries.len*2] = 0x00;
			dataSentbuf[33+req->queries.len*2] = 0x00;
			dataSentbuf[34+req->queries.len*2] = 0x00;
			dataSentbuf[35+req->queries.len*2] = 0x00;
			dataSentbuf[36+req->queries.len*2] = 0x00;
			dataSentbuf[37+req->queries.len*2] = 0x00;
			dataSentbuf[38+req->queries.len*2] = 0xc2;
			dataSentbuf[39+req->queries.len*2] = 0xa0;
			dataSentbuf[40+req->queries.len*2] = 0xbb;
			dataSentbuf[41+req->queries.len*2] = 0xff;
			dataSentbuf[42+req->queries.len*2] = 0xfe;
			dataSentbuf[43+req->queries.len*2] = 0xfd;
			dataSentbuf[44+req->queries.len*2] = 0x8d;
			dataSentbuf[45+req->queries.len*2] = 0x2c;
		}
		else
		{
			dataSentbuf[28+req->queries.len*2] = 0x0;
			dataSentbuf[29+req->queries.len*2] = 0x04;
			dataSentbuf[30+req->queries.len*2] = (unsigned char)ipnum[0];
			dataSentbuf[31+req->queries.len*2] = (unsigned char)ipnum[1];
			dataSentbuf[32+req->queries.len*2] = (unsigned char)ipnum[2];
			dataSentbuf[33+req->queries.len*2] = (unsigned char)ipnum[3];
		}
		
		//memcpy(ans->answers.name,usrdata,req->queries.len));
		//ans->answers.type = req->queries.type;
		//ans->answers.ClassType = req->queries.ClassType;
		//ans->answers.ttl	= htonl(0x0000001e);
		//ans->answers.datalen = htons(0x04);
		//ans->answers.flags = htons(0x4000);
		//ans->answers.ip[0] = (unsigned char)ipnum[0];
		//ans->answers.ip[1] = (unsigned char)ipnum[1];
		//ans->answers.ip[2] = (unsigned char)ipnum[2];
		//ans->answers.ip[3] = (unsigned char)ipnum[3];

		//设置UDP
		udpheader = (struct udphdr *)(raw_buffer +sizeof(struct ether_header) + sizeof(struct iphdr));
		nbns_udpheader->dest = udpheader->source;
		nbns_udpheader->source = udpheader->dest;
		nbns_udpheader->len		= htons(sizeof(struct udphdr)+(34+req->queries.len*2));
		nbns_udpheader->check = 0;

		//设置IP
		nbns_iph->version = ipheader->version;
		nbns_iph->ihl = sizeof(struct iphdr)>>2;
		nbns_iph->tos = ipheader->tos;
		nbns_iph->tot_len = htons(sizeof(struct iphdr)+sizeof(struct udphdr)+(34+req->queries.len*2));
		nbns_iph->id = ipheader->id;
		nbns_iph->frag_off = htons(0x4000);
		nbns_iph->ttl = 64;
		nbns_iph->protocol=IPPROTO_UDP;
		nbns_iph->check=0;
		memcpy(&(nbns_iph->daddr),(unsigned char *)(raw_buffer +sizeof(struct ether_header)+12),4);//这个位置比较怪异，只能copy内存，不能赋值															
		nbns_iph->saddr = inet_addr(manageip);						
		nbns_iph->check = check_sum((unsigned short *)nbns_iph,20); 

		//设置MAC层
#if defined( CONFIG_APPS_HTML_WEB_STYLE_DLINK1155)||defined(CONFIG_APPS_HTML_WEB_STYLE_DLINK1160)
		while(arppingNum--)
		{
			if(0 == arpping(inet_addr(IPbuf), inet_addr(manageip), srcmac, dstmac, "br0"))
			{
				DEBUG_PIRNT("IPbuf=%s,dstmac=%x:%x:%x:%x:%x:%x\n", IPbuf, dstmac[0], dstmac[1], dstmac[2], dstmac[3], dstmac[4], dstmac[5]);
				memcpy(nbns_ether->ether_dhost, dstmac, 6);
				break;
			}
		}
		if( strlen(dstmac) == 0 )
#endif
		memcpy(nbns_ether->ether_dhost,ether_tmp->ether_shost,6);
		memcpy(nbns_ether->ether_shost,srcmac,6);
		nbns_ether->ether_type = htons(0x0800);

		memcpy(sendbuf,nbns_ether,sizeof(struct ether_header));
		memcpy(sendbuf+sizeof(struct ether_header),nbns_iph,sizeof(struct iphdr));
		memcpy(sendbuf+sizeof(struct ether_header)+sizeof(struct iphdr),nbns_udpheader,sizeof(struct udphdr));
		memcpy(sendbuf+sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr),dataSentbuf,(34+req->queries.len*2));
        DEBUG_PIRNT("-----%s----%d---\n",__FUNCTION__,__LINE__);
		sendto(dns_sock_raw,sendbuf,sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr)+(34+req->queries.len*2),0,(struct sockaddr*)&sll, sizeof(sll));
out:
		if(NULL != ans)
			free(ans);
		if(NULL != nbns_udpheader)
			free(nbns_udpheader);
		if(NULL != nbns_iph)
			free(nbns_iph);
		if(NULL != nbns_ether)
			free(nbns_ether);
	}
	DEBUG_PIRNT("-----%s----%d---\n",__FUNCTION__,__LINE__);
	return 1;
}
int ipV6_LlmnrPacketProcess(const struct ipv6hdr * ip6header)

{
	struct llmnr_req *req;
	struct llmnr_ans *ans;
	struct udphdr * udpheader = NULL;
	struct udphdr * udpheaderNew = NULL;
	unsigned int i;
	char br0_1ip[16]={0};
	char br0ip[16]={0};
	//char manageip[16]={0};
	char namebuf[17]={0};
	char namebufBak[17]={0};
	char *address =NULL;
	char br0netmask[16] ={0};
	char br00netmask[16]={0};
	struct ether_header *nbns_ether;
	struct ether_header *ether_tmp;
	struct iphdr * nbns_iph;           //ip head
	struct udphdr * nbns_udpheader;
	struct ifreq ifr1;
	struct ifconf ifconf;
	struct sockaddr_ll sll;
	char *usrdata = NULL;
	unsigned char buf[512];
	unsigned int ipnum[4];
	unsigned char sendbuf[256]={0};
	char *szIpAddr = NULL;
	char IPbuf[16]= {0}, dataSentbuf[256] = { 0 };
	char IP6buf[48]= {0};
	 //struct sockaddr_in6 my_addr, their_addr;
	int dns_any_sock;
#ifdef CONFIG_APPS_HTML_WEB_STYLE_DLINK1155
	unsigned char dstmac[6]={0};
	int arppingNum = 3;
#endif 
    struct ipv6hdr ip6new;
    struct udphdr *udp;
    struct sockaddr_in6 saddr;
	struct sockaddr_in6 daddr;
	unsigned char *temp = NULL;
    int iret  = 0;
	int queryType = AAA;
	int iRet;
	struct sockaddr_in *addr = NULL;
	//strcpy(manageipv6,"fe80::acbd:1aff:fe2f:2645");
	/*br0 linklocal IPV6 字符串地址转换16进制格式*/
    if ( inet_pton ( AF_INET6, manageipv6, &ip6new.saddr ) <= 0 )
    {
        return -1;
	}
	DEBUG_PIRNT("ip6new->ip6_src=%02x%02x%02x%02x\n",ip6new.saddr.in6_u.u6_addr8[0],ip6new.saddr.in6_u.u6_addr8[1],ip6new.saddr.in6_u.u6_addr8[2],ip6new.saddr.in6_u.u6_addr8[3]);
	DEBUG_PIRNT("ip6new->ip6_src=%02x%02x%02x%02x\n",ip6new.saddr.in6_u.u6_addr8[4],ip6new.saddr.in6_u.u6_addr8[5],ip6new.saddr.in6_u.u6_addr8[6],ip6new.saddr.in6_u.u6_addr8[7]);
	DEBUG_PIRNT("ip6new->ip6_src=%02x%02x%02x%02x\n",ip6new.saddr.in6_u.u6_addr8[8],ip6new.saddr.in6_u.u6_addr8[9],ip6new.saddr.in6_u.u6_addr8[10],ip6new.saddr.in6_u.u6_addr8[11]);
	DEBUG_PIRNT("ip6new->ip6_src=%02x%02x%02x%02x\n",ip6new.saddr.in6_u.u6_addr8[12],ip6new.saddr.in6_u.u6_addr8[13],ip6new.saddr.in6_u.u6_addr8[14],ip6new.saddr.in6_u.u6_addr8[15]);
	
    DEBUG_PIRNT("------------ipV6_LlmnrPacketProcess---------------\n");
    
	if(!ip6header)
	{
		return -1;
	}    
	//szIpAddr = (char *) (raw_buffer +sizeof(struct ether_header)+12);
	/*IPV6  16进制格式转换字符串地址*/
	sprintf(IP6buf,"%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[0],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[1],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[2],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[3],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[4],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[5],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[6],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[7],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[8],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[9],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[10],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[11],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[12],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[13],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[14],
	                (unsigned char) ip6header->saddr.in6_u.u6_addr8[15]);  
	DEBUG_PIRNT("IP6buf=%s\n",IP6buf);
	
	/*目的IPV6 字符串地址转换16进制格式*/
    if ( inet_pton ( AF_INET6, IP6buf, &ip6new.daddr ) <= 0 )
    {
        return -1;
	}
	
    /*填充IPV6 头部*/
	ip6new.version     = ip6header->version;
	ip6new.priority    = ip6header->priority;
	ip6new.flow_lbl[0]    = ip6header->flow_lbl[0];
	ip6new.flow_lbl[1]    = ip6header->flow_lbl[1];
	ip6new.flow_lbl[2]    = ip6header->flow_lbl[2];
	ip6new.payload_len = ip6header->payload_len;
	ip6new.nexthdr     = ip6header->nexthdr;
	ip6new.hop_limit   = ip6header->hop_limit;

    temp = (unsigned char *)(&ip6new);
	DEBUG_PIRNT("new ipv6 packet\n");
	DEBUG_PIRNT("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5],temp[6],temp[7]);
	DEBUG_PIRNT("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[8],temp[9],temp[10],temp[11],temp[12],temp[13],temp[14],temp[15]);
	DEBUG_PIRNT("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[16],temp[17],temp[18],temp[19],temp[20],temp[21],temp[22],temp[23]);
	DEBUG_PIRNT("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[24],temp[25],temp[26],temp[27],temp[28],temp[29],temp[30],temp[31]);
	DEBUG_PIRNT("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[32],temp[33],temp[34],temp[35],temp[36],temp[37],temp[38],temp[39]);
	//printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[40],temp[41],temp[42],temp[43],temp[44],temp[45],temp[46],temp[47]);
	DEBUG_PIRNT("new ipv6 packet end\n");
	
	usrdata =(char *) (raw6_buffer +sizeof(struct ether_header) + sizeof(struct ipv6hdr) + sizeof(struct udphdr)); 

    if(!usrdata)
	{
		return -1;
	} 
	
	req = (struct llmnr_req*)usrdata;
	
	//DecodeName(req->queries.name,namebuf);
	//printf("req->queries.len=%d\n",req->queries.len);
	strncpy(namebuf, usrdata+13, req->queries.len);
	
    req->queries.type = ntohs(*(unsigned short *)(usrdata+14+req->queries.len));
	
	req->queries.ClassType = ntohs(*(unsigned short *)(usrdata+16+req->queries.len));
	
	DEBUG_PIRNT("ClassType:%x, type=%x\n",req->queries.ClassType, req->queries.type);
	//printf("req->queries.type=%02x %02x\n",*(unsigned char *)(usrdata+14+req->queries.len),*(unsigned char *)(usrdata+15+req->queries.len));
    /*判断llmnr 请求IPV4 or IPV6 地址*/
	if(0x1 == req->queries.type)
	{
        queryType = A;
	}
	else if(0x1c == req->queries.type)
	{
        queryType = AAA;
	}
	else
	{
        printf("queries type=%d is invalied!!!\n",*(unsigned short *)(usrdata+14+req->queries.len));
		return -1;
	}
    DEBUG_PIRNT("queryType=0x%x !!!\n",queryType);

	/*BACK UP namebuf
	   because after toUpper(namebuf); change to Upper
         so do this to keep the original string 
	*/	
	memset(namebufBak,0,sizeof(namebufBak));
    strncpy(namebufBak,namebuf,strlen(namebuf));
	DEBUG_PIRNT("namebufBak name:%s\n",namebufBak);
	
	toUpper(namebuf);
	DEBUG_PIRNT("query name:%s\n",namebuf);
	// ok return 0; 
	toUpper(match_str);
	toUpper(match_str_local);
	toUpper(match_str_mac);
	toUpper(match_str_mac_local);
	if(((strlen(match_str)) &&
	   (0 == strncmp(namebuf, match_str, strlen(match_str)) ||
	   0 == strncmp(&namebuf[4], match_str, strlen(match_str))))||
	   ((strlen(match_str_local)) &&
	   (0 == strncmp(namebuf, match_str_local, strlen(match_str_local)) ||
	   0 == strncmp(&namebuf[4], match_str_local, strlen(match_str_local))))||
	   ((strlen(match_str_mac)) &&
	   (0 == strncmp(namebuf, match_str_mac, strlen(match_str_mac)) ||
	   0 == strncmp(&namebuf[4], match_str_mac, strlen(match_str_mac))))||
	   ((strlen(match_str_mac_local)) &&
	   (0 == strncmp(namebuf, match_str_mac_local, strlen(match_str_mac_local)) ||
	   0 == strncmp(&namebuf[4], match_str_mac_local, strlen(match_str_mac_local)))))
	{
        DEBUG_PIRNT("query name match_str:%s\n",match_str);

		bzero(&sll, sizeof(sll));
		sll.sll_family = AF_PACKET;
		sll.sll_halen = 6;
		memcpy(sll.sll_addr,raw6_buffer+6,6);
		sll.sll_ifindex = ifr.ifr_ifindex; 
		
        memset(br0_1ip,0,sizeof(br0_1ip));
		memset(br0ip,0,sizeof(br0ip)); 
		//memset(manageip,0,sizeof(manageip)); 
		nbns_ether =malloc(sizeof(struct ether_header));
		nbns_iph = malloc(sizeof(struct iphdr));
		nbns_udpheader = malloc(sizeof(struct udphdr));
		ans = malloc(sizeof(struct llmnr_ans));
		if((ans==NULL)||(NULL==nbns_ether)||(NULL==nbns_iph)||(NULL==nbns_udpheader))
		{
			if(NULL != ans)
				free(ans);
			if(NULL != nbns_udpheader)
				free(nbns_udpheader);
			if(NULL != nbns_iph)
				free(nbns_iph);
			if(NULL != nbns_ether)
				free(nbns_ether);
			return -1;
		}
		//printf("manageipBak=%s\n",manageipBak);
		//sscanf(manageipBak,"%d.%d.%d.%d\n",&ipnum[0],&ipnum[1],&ipnum[2],&ipnum[3]);
	#if 1	

		//get_netmask("br0",br0netmask);	
		//get_netmask("br0:0",br00netmask);
		ether_tmp = (struct ether_header *)raw6_buffer;

		dns_any_sock = socket(AF_INET, SOCK_DGRAM, 0);//用于获取MAC，IP等地址信息
		if(dns_any_sock<0)
		{
			DEBUG_PIRNT("Could not create dns socket");
			if(NULL != ans)
				free(ans);
			if(NULL != nbns_udpheader)
				free(nbns_udpheader);
			if(NULL != nbns_iph)
				free(nbns_iph);
			if(NULL != nbns_ether)
				free(nbns_ether);
			return -1;
		}
		#if 0
		ifconf.ifc_len = sizeof(buf);
		ifconf.ifc_buf = buf;
		ioctl(dns_any_sock, SIOCGIFCONF, &ifconf);    //获取所有接口信息
		ifr1 = (struct ifreq*)buf;  

		for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
		{
			if(0 == strcmp(ifr1->ifr_name, "br0"))			
			{
				if(strlen(br0netmask)==0)  
					strcpy(br0netmask,"255.255.255.0");
				address = inet_ntoa(((struct sockaddr_in*)&(ifr1->ifr_addr))->sin_addr);                                                                        
				DEBUG_PIRNT("br0======%s=========IP:%s,netmask:%s===========\n",ifr1->ifr_name, address, br0netmask);
				//if((inet_addr(address) & inet_addr(br0netmask)) ==  (inet_addr(IPbuf) & inet_addr(br0netmask)))
				//{
					DEBUG_PIRNT("find br0 ip\n");
				    memset(manageipBak,0,sizeof(manageipBak));
					memcpy(manageipBak,address,strlen(address));
					break;
				//}
				//memcpy(br0_1ip,address,strlen(address));
			}
			/*只取br0 地址
			else if(0 == strcmp(ifr1->ifr_name, "br0:0"))
			{
				if(strlen(br00netmask)==0)  
					strcpy(br00netmask,"255.255.0.0");
				address = inet_ntoa(((struct sockaddr_in*)&(ifr1->ifr_addr))->sin_addr);                                                                
				DEBUG_PIRNT("br0:0======%s=========IP:%s,netmask:%s===========\n",ifr1->ifr_name, address, br00netmask);
				if((inet_addr(address) & inet_addr(br00netmask)) ==  (inet_addr(IPbuf) & inet_addr(br00netmask)))
				{
					DEBUG_PIRNT("find br0:0 ip\n");
					memcpy(manageip,address,strlen(address));
					break;
				}
				//memcpy(br0ip,address,strlen(address));
			}
			*/
			DEBUG_PIRNT("name = [%s]\n", ifr1->ifr_name);
			DEBUG_PIRNT("local addr = [%s]\n", 
			inet_ntoa(((struct sockaddr_in*)&(ifr1->ifr_addr))->sin_addr));          					
			ifr1++;
		}
		#else
		#if 1
		memset(&ifr1, 0, sizeof(struct ifreq)); 
		sprintf(ifr1.ifr_name, "br0");
		addr = (struct sockaddr_in *)&ifr1.ifr_addr;
		addr->sin_family = AF_INET;
		
		iRet = ioctl(dns_any_sock, SIOCGIFADDR, (char *)&ifr1);
		
		memset(manageipBak,0,sizeof(manageipBak));
		if (iRet < 0) 
		{
			DEBUG_PIRNT("Couldn't get the address of interface br0");
			strcpy(manageipBak, "192.168.0.1");
		}
		else
		{
			memcpy(manageipBak, inet_ntoa(addr->sin_addr), sizeof(manageipBak));
		}
		#endif
		#endif
		if(dns_any_sock > 0)
		{
			close(dns_any_sock);
		}

		if(0 == strlen(manageipBak))
		{
			DEBUG_PIRNT("manageip not find in same submast\n");
			if(NULL != ans)
				free(ans);
			if(NULL != nbns_udpheader)
				free(nbns_udpheader);
			if(NULL != nbns_iph)
				free(nbns_iph);
			if(NULL != nbns_ether)
				free(nbns_ether);
			return -1;
		}
		
		DEBUG_PIRNT("********manageip:%s,%d\n",manageipBak,strlen(manageipBak));
		sscanf(manageipBak,"%d.%d.%d.%d",&ipnum[0],&ipnum[1],&ipnum[2],&ipnum[3]);	         	
#endif
		//设置用户数据包层
        /*query header set start*/
        ans->id 			= req->id;
		ans->flags 		= htons(0x8000);//应答包
		ans->question = htons(0x01);	//询问包个数
		ans->answer		= htons(0x01);//应答包个数
		ans->auth			= htons(0x0);//无认证
		ans->add			= htons(0x0);//附加信息
		memcpy(dataSentbuf, ans, 12);
        /*query header set end*/
		
		/*query data set start*/
		//query host name
		dataSentbuf[12] = req->queries.len;
		memcpy(&dataSentbuf[13], namebufBak, req->queries.len);

		//query type (A type means ipv4,value is 0x0001;AAAA type means ipv6,value is 0x001c)
		memcpy(&dataSentbuf[14+req->queries.len], (usrdata+14+req->queries.len), 2);

		//query class
		memcpy(&dataSentbuf[16+req->queries.len], (usrdata+16+req->queries.len), 2);
		/*query data set end*/

		/*Answer data set start*/
		//Answer host name
		dataSentbuf[18+req->queries.len] = req->queries.len;
		memcpy(&dataSentbuf[19+req->queries.len], namebufBak, req->queries.len);

		//Answer type(A type means ipv4,value is 0x0001;AAAA type means ipv6,value is 0x001c)
		memcpy(&dataSentbuf[20+req->queries.len*2], (usrdata+14+req->queries.len), 2);

		//Answer class
		memcpy(&dataSentbuf[22+req->queries.len*2], (usrdata+16+req->queries.len), 2);
        //Answer Time to Live (4bytes ,default 30s=0x0000001e)
		dataSentbuf[24+req->queries.len*2] = 0x0;
		dataSentbuf[25+req->queries.len*2] = 0x0;
		dataSentbuf[26+req->queries.len*2] = 0x0;
		dataSentbuf[27+req->queries.len*2] = 0x1e;
		if(0x1 == req->queries.type)
		{
            //Answer Data Len(2 bytes ipv4 Data Len is 4=0x0004 ,ipv6 is 16=0x0010 )
			dataSentbuf[28+req->queries.len*2] = 0x0;
			dataSentbuf[29+req->queries.len*2] = 0x04; 
			//Answer Data
		    dataSentbuf[30+req->queries.len*2] = (unsigned char)ipnum[0];
		    dataSentbuf[31+req->queries.len*2] = (unsigned char)ipnum[1];
		    dataSentbuf[32+req->queries.len*2] = (unsigned char)ipnum[2];
		    dataSentbuf[33+req->queries.len*2] = (unsigned char)ipnum[3];
		}
		else if(0x1c == req->queries.type)
		{
			//Answer Data Len(2 bytes ipv4 Data Len is 4=0x0004 ,ipv6 is 16=0x0010 )
			dataSentbuf[28+req->queries.len*2] = 0x0;
			dataSentbuf[29+req->queries.len*2] = 0x10;
			//Answer Data
			if ( inet_pton ( AF_INET6, manageipv6, &(dataSentbuf[30+req->queries.len*2]) ) <= 0 )
	    	{
	        	goto out;
			}
		}
        else
		{
	        printf("queries type=%d is invalied!!!\n",*(unsigned short *)(usrdata+14+req->queries.len));
			return -1;
		}

		//设置UDP
		udpheader = (struct udphdr *)(raw6_buffer +sizeof(struct ether_header) + sizeof(struct ipv6hdr));
		nbns_udpheader->dest = udpheader->source;
		nbns_udpheader->source = udpheader->dest;
		
		if(0x1 == req->queries.type)
			nbns_udpheader->len		= htons(sizeof(struct udphdr)+(34+req->queries.len*2));
		else if(0x1c == req->queries.type)
		    nbns_udpheader->len		= htons(sizeof(struct udphdr)+(34+12+req->queries.len*2));
        else
		{
	        printf("queries type=%d is invalied!!!\n",*(unsigned short *)(usrdata+14+req->queries.len));
			goto out;
		}
		DEBUG_PIRNT("nbns_udpheader->len=0x%x\n",nbns_udpheader->len);
		nbns_udpheader->check = 0;
#if 0
		//设置IP
		nbns_iph->version = ip6header->version;
		nbns_iph->ihl = sizeof(struct iphdr)>>2;
		//nbns_iph->tos = ipheader->tos;
		nbns_iph->tot_len = htons(sizeof(struct iphdr)+sizeof(struct udphdr)+(34+req->queries.len*2));
		//nbns_iph->id = ipheader->id;
		nbns_iph->frag_off = htons(0x4000);
		nbns_iph->ttl = 64;
		nbns_iph->protocol=IPPROTO_UDP;
		nbns_iph->check=0;
		memcpy(&(nbns_iph->daddr),(unsigned char *)(raw_buffer +sizeof(struct ether_header)+12),4);//这个位置比较怪异，只能copy内存，不能赋值															
		nbns_iph->saddr = inet_addr(manageip);						
		nbns_iph->check = check_sum((unsigned short *)nbns_iph,20); 
#endif
		//设置MAC层
#ifdef CONFIG_APPS_HTML_WEB_STYLE_DLINK1155
		while(arppingNum--)
		{
			if(0 == arpping(inet_addr(IPbuf), inet_addr(manageip), srcmac, dstmac, "br0"))
			{
				DEBUG_PIRNT("IPbuf=%s,dstmac=%x:%x:%x:%x:%x:%x\n", IPbuf, dstmac[0], dstmac[1], dstmac[2], dstmac[3], dstmac[4], dstmac[5]);
				memcpy(nbns_ether->ether_dhost, dstmac, 6);
				break;
			}
		}
		if( strlen(dstmac) == 0 )
#endif
		memcpy(nbns_ether->ether_dhost,ether_tmp->ether_shost,6);
		memcpy(nbns_ether->ether_shost,srcmac,6);
		nbns_ether->ether_type = htons(0x0800);

		memcpy(sendbuf,nbns_ether,sizeof(struct ether_header));
		ip6new.payload_len = htons(sizeof(struct udphdr)+(34+req->queries.len*2));
		memcpy(sendbuf+sizeof(struct ether_header),&ip6new,sizeof(struct ipv6hdr));
		
		DEBUG_PIRNT("req->queries.len*2=%d\n",req->queries.len*2);
		//xx return 0;
		memcpy(sendbuf+sizeof(struct ether_header)+sizeof(struct ipv6hdr),nbns_udpheader,sizeof(struct udphdr));
		memcpy(sendbuf+sizeof(struct ether_header)+sizeof(struct ipv6hdr)+sizeof(struct udphdr),dataSentbuf,(34+req->queries.len*2));

		DEBUG_PIRNT("sendto ipv6 packet\n");
		temp = (unsigned char *)(sendbuf);
	    DEBUG_PIRNT("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5],temp[6],temp[7]);
		DEBUG_PIRNT("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[8],temp[9],temp[10],temp[11],temp[12],temp[13],temp[14],temp[15]);
		DEBUG_PIRNT("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[16],temp[17],temp[18],temp[19],temp[20],temp[21],temp[22],temp[23]);
		DEBUG_PIRNT("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[24],temp[25],temp[26],temp[27],temp[28],temp[29],temp[30],temp[31]);
		DEBUG_PIRNT("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[32],temp[33],temp[34],temp[35],temp[36],temp[37],temp[38],temp[39]);
#if 0
		printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[40],temp[41],temp[42],temp[43],temp[44],temp[45],temp[46],temp[47]);
		printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[48],temp[49],temp[50],temp[51],temp[52],temp[53],temp[54],temp[55]);
		printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[56],temp[57],temp[58],temp[59],temp[60],temp[61],temp[62],temp[63]);
		printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[64],temp[65],temp[66],temp[67],temp[68],temp[69],temp[70],temp[71]);
		printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[72],temp[73],temp[74],temp[75],temp[76],temp[77],temp[78],temp[79]);
		printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[80],temp[81],temp[82],temp[83],temp[84],temp[85],temp[86],temp[87]);
		printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[88],temp[89],temp[90],temp[91],temp[92],temp[93],temp[94],temp[95]);
		printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[96],temp[97],temp[98],temp[99],temp[100],temp[101],temp[102],temp[103]);
#endif
		DEBUG_PIRNT("sendto ipv6 packet end\n");
		if(0x1 == req->queries.type)
		{
            udpheaderNew = (struct udphdr *)(sendbuf +sizeof(struct ether_header) + sizeof(struct ipv6hdr));
			udpheaderNew->check= check_sum((unsigned short *)sendbuf,(34+req->queries.len*2)); 
			iret = sendto(llmnr_sock_raw,sendbuf,sizeof(struct ether_header)+sizeof(struct ipv6hdr)+sizeof(struct udphdr)+(34+req->queries.len*2),0,(struct sockaddr*)&sll, sizeof(sll));
		}
		else if(0x1c == req->queries.type)
		{   /*IPv6的特点10高效包头：IPv6包头比IPv4数据包头简单。
		      IPv4包头的6个字段在IPv6包头中被去掉了。
		      IPv4包头算上选项和填充字段有14个字段，
		      IPv6包头有8个字段。基本的IPv6包头大小是40字节，
		      IPv4包头不带选项和填充字段是20个字节。
		      基本的IPv6包头长度固定，
		      IPv4包头在使用选项字段时是变长的。
		      较少的IPv6包头字段和固定的长度意味着路由器转发IPv6数据包耗费较少的CPU周期，
		      直接有益于网络性能。在IPv4中，一个16比特字段用来验证包头的完整性。
		      IPv6包头中这个字段被简单去掉，以提高路由选择效率。
		      实际上，路径上的所有路由器在转发处理期间不必重新计算校验和。
		      错误检测由数据链路层技术第2层和传输层第4层端到端连接的校验和来处理。
		      在第2层和第4层所做的校验和足够强壮，从而不必考虑对第3层校验和的需要。
		      对IPv6而言，TCP和UDP传输协议都必须需要校验和。IPv4中UDP校验和是可选的。*/
		    udpheaderNew = (struct udphdr *)(sendbuf +sizeof(struct ether_header) + sizeof(struct ipv6hdr));
			udpheaderNew->check= check_sum((unsigned short *)sendbuf,(34+12+req->queries.len*2)); 
		    iret = sendto(llmnr_sock_raw,sendbuf,sizeof(struct ether_header)+sizeof(struct ipv6hdr)+sizeof(struct udphdr)+(34+12+req->queries.len*2),0,(struct sockaddr*)&sll, sizeof(sll));
		}
        else
		{
	        DEBUG_PIRNT("queries type=%d is invalied!!!\n",*(unsigned short *)(usrdata+14+req->queries.len));
			return -1;
		}
out:
		if(NULL != ans)
			free(ans);
		if(NULL != nbns_udpheader)
			free(nbns_udpheader);
		if(NULL != nbns_iph)
			free(nbns_iph);
		if(NULL != nbns_ether)
			free(nbns_ether);
	}
	DEBUG_PIRNT("-----%s----%d---\n",__FUNCTION__,__LINE__);
	return 1;
}

int NbnsPacketProcess(const struct iphdr * ipheader)
{
	struct nbns_req *req;
	struct nbns_ans *ans;
	struct udphdr * udpheader = NULL;
	unsigned int i;
	char br0_1ip[16]={0};
	char br0ip[16]={0};
	//char manageip[16]={0};
	char namebuf[17]={0};
	char *address =NULL;
	char br0netmask[16] ={0};
	char br00netmask[16]={0};
	struct ether_header *nbns_ether;
	struct ether_header *ether_tmp;
	struct iphdr * nbns_iph;           //ip head
	struct udphdr * nbns_udpheader;
	struct ifreq ifr1;
	struct ifconf ifconf;
	struct sockaddr_ll sll;
	char *usrdata = NULL;
	unsigned char buf[512];
	unsigned int ipnum[4];
	unsigned char sendbuf[256]={0};
	char *szIpAddr = NULL;
	char IPbuf[16]= {0};
	int dns_any_sock;
#if defined( CONFIG_APPS_HTML_WEB_STYLE_DLINK1155)||defined(CONFIG_APPS_HTML_WEB_STYLE_DLINK1160)
	unsigned char dstmac[6]={0};
	int arppingNum = 3;
#endif
	int iRet;
	struct sockaddr_in *addr = NULL;

    DEBUG_PIRNT("-----%s----%d---\n",__FUNCTION__,__LINE__);
	usrdata =(char *) (raw_buffer +sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr)); 
	if((!ipheader)||(!usrdata))
	{
		return -1;
	}
	//printf_data(raw_buffer);
	szIpAddr = (char *) (raw_buffer +sizeof(struct ether_header)+12);
	sprintf(IPbuf,"%d.%d.%d.%d",(unsigned char) szIpAddr[0],(unsigned char) szIpAddr[1],(unsigned char) szIpAddr[2],(unsigned char) szIpAddr[3]);                                                            
	req = (struct nbns_req*)usrdata;
	DecodeName(req->queries.name,namebuf);
	toUpper(match_str);
	toUpper(match_str_local);
	toUpper(match_str_mac);
	toUpper(match_str_mac_local);

	DEBUG_PIRNT("name:%s\n",namebuf);
	DEBUG_PIRNT("match_str:%s\n",match_str);	
	if( ((strlen(match_str)) &&
	   (0 == strncmp(namebuf, match_str, strlen(match_str)) ||
	   0 == strncmp(&namebuf[4], match_str, strlen(match_str))))||
	   ((strlen(match_str_local)) &&
	   (0 == strncmp(namebuf, match_str_local, strlen(match_str_local)) ||
	   0 == strncmp(&namebuf[4], match_str_local, strlen(match_str_local))))||
	   ((strlen(match_str_mac)) &&
	   (0 == strncmp(namebuf, match_str_mac, strlen(match_str_mac)) ||
	   0 == strncmp(&namebuf[4], match_str_mac, strlen(match_str_mac))))||
	   ((strlen(match_str_mac_local)) &&
	   (0 == strncmp(namebuf, match_str_mac_local, strlen(match_str_mac_local)) ||
	   0 == strncmp(&namebuf[4], match_str_mac_local, strlen(match_str_mac_local)))))
	{
	    DEBUG_PIRNT("match_str:%s = namebuf=%s\n",match_str,namebuf);
		memset(br0_1ip,0,sizeof(br0_1ip));
		memset(br0ip,0,sizeof(br0ip)); 
		//memset(manageip,0,sizeof(manageip)); 
		nbns_ether =malloc(sizeof(struct ether_header));
		nbns_iph = malloc(sizeof(struct iphdr));
		nbns_udpheader = malloc(sizeof(struct udphdr));
		ans = malloc(sizeof(struct nbns_ans));
		if((ans==NULL)||(NULL==nbns_ether)||(NULL==nbns_iph)||(NULL==nbns_udpheader))
		{
			if(NULL != ans)
				free(ans);
			if(NULL != nbns_udpheader)
				free(nbns_udpheader);
			if(NULL != nbns_iph)
				free(nbns_iph);
			if(NULL != nbns_ether)
				free(nbns_ether);
			return -1;
		}

		bzero(&sll, sizeof(sll));
		sll.sll_family = AF_PACKET;
		sll.sll_halen = 6;
		memcpy(sll.sll_addr,raw_buffer+6,6);
		sll.sll_ifindex = ifr.ifr_ifindex; 
		//get_netmask("br0",br0netmask);
		//get_netmask("br0:0",br00netmask);
		ether_tmp = (struct ether_header *)raw_buffer;

		dns_any_sock = socket(AF_INET, SOCK_DGRAM, 0);//用于获取MAC，IP等地址信息
		if(dns_any_sock<0)
		{
			DEBUG_PIRNT("Could not create dns socket");
			if(NULL != ans)
				free(ans);
			if(NULL != nbns_udpheader)
				free(nbns_udpheader);
			if(NULL != nbns_iph)
				free(nbns_iph);
			if(NULL != nbns_ether)
				free(nbns_ether);
			return -1;
		}
		#if 0
		ifconf.ifc_len = sizeof(buf);
		ifconf.ifc_buf = buf;
		ioctl(dns_any_sock, SIOCGIFCONF, &ifconf);    //获取所有接口信息
		ifr1 = (struct ifreq*)buf;  

		for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
		{
			if(0 == strcmp(ifr1->ifr_name, "br0"))
			{
				if(strlen(br0netmask)==0)  
					strcpy(br0netmask,"255.255.255.0");
				address = inet_ntoa(((struct sockaddr_in*)&(ifr1->ifr_addr))->sin_addr);                                                                        
				DEBUG_PIRNT("br0======%s=========IP:%s,netmask:%s===========\n",ifr1->ifr_name, address, br0netmask);
				if((inet_addr(address) & inet_addr(br0netmask)) ==  (inet_addr(IPbuf) & inet_addr(br0netmask)))
				{
					DEBUG_PIRNT("find br0 ip\n");
					memcpy(manageip,address,strlen(address));
					break;
				}
				//memcpy(br0_1ip,address,strlen(address));
			}
			else if(0 == strcmp(ifr1->ifr_name, "br0:0"))
			{
				if(strlen(br00netmask)==0)  
					strcpy(br00netmask,"255.255.0.0");
				address = inet_ntoa(((struct sockaddr_in*)&(ifr1->ifr_addr))->sin_addr);                                                                
				DEBUG_PIRNT("br0:0======%s=========IP:%s,netmask:%s===========\n",ifr1->ifr_name, address, br00netmask);
				if((inet_addr(address) & inet_addr(br00netmask)) ==  (inet_addr(IPbuf) & inet_addr(br00netmask)))
				{
					DEBUG_PIRNT("find br0:0 ip\n");
					memcpy(manageip,address,strlen(address));
					break;
				}
				//memcpy(br0ip,address,strlen(address));
			}
			DEBUG_PIRNT("name = [%s]\n", ifr1->ifr_name);
			DEBUG_PIRNT("local addr = [%s]\n", 
			inet_ntoa(((struct sockaddr_in*)&(ifr1->ifr_addr))->sin_addr));          					
			ifr1++;
		}
		#else
		#if 1
   		memset(&ifr1, 0, sizeof(struct ifreq));	
		sprintf(ifr1.ifr_name, "br0");
		addr = (struct sockaddr_in *)&ifr1.ifr_addr;
   		addr->sin_family = AF_INET;
		
		iRet = ioctl(dns_any_sock, SIOCGIFADDR, (char *)&ifr1);
		if (iRet < 0) 
		{
			DEBUG_PIRNT("Couldn't get the address of interface br0");
			strcpy(manageip, "192.168.0.1");
		}
		else
		{
			memcpy(manageip, inet_ntoa(addr->sin_addr), sizeof(manageip));
		}
		#endif
		#endif
				
		if(dns_any_sock > 0)
		{
			close(dns_any_sock);
		}

		if(0 == strlen(manageip))
		{
			DEBUG_PIRNT("manageip not find in same submast\n");
			if(NULL != ans)
				free(ans);
			if(NULL != nbns_udpheader)
				free(nbns_udpheader);
			if(NULL != nbns_iph)
				free(nbns_iph);
			if(NULL != nbns_ether)
				free(nbns_ether);
			return -1;
		}
		
		DEBUG_PIRNT("********manageip:%s,%d\n",manageip,strlen(manageip));
		sscanf(manageip,"%d.%d.%d.%d\n",&ipnum[0],&ipnum[1],&ipnum[2],&ipnum[3]);	         	

		//设置用户数据包层
		ans->id 			= req->id;
		ans->flags 		= htons(0x8400);//应答包
		ans->question = htons(0x0);	//询问包个数
		ans->answer		= htons(0x01);//应答包个数
		ans->auth			= htons(0x0);//无认证
		ans->add			= htons(0x0);//附加信息
		ans->answers.blank = req->queries.blank;
		memcpy(ans->answers.name,req->queries.name,sizeof(req->queries.name));
		ans->answers.type = req->queries.type;
		ans->answers.ClassType = req->queries.ClassType;
		ans->answers.ttl	= htonl(0x00000e10);
		ans->answers.datalen = htons(0x06);
		ans->answers.flags = htons(0x4000);
		ans->answers.ip[0] = (unsigned char)ipnum[0];
		ans->answers.ip[1] = (unsigned char)ipnum[1];
		ans->answers.ip[2] = (unsigned char)ipnum[2];
		ans->answers.ip[3] = (unsigned char)ipnum[3];

		//设置UDP
		nbns_udpheader->dest = htons(UDP_PORT);
		nbns_udpheader->source = htons(UDP_PORT);
		nbns_udpheader->len		= htons(sizeof(struct udphdr)+sizeof(struct nbns_ans));
		nbns_udpheader->check = 0;
		//设置IP
		nbns_iph->version = ipheader->version;
		nbns_iph->ihl = sizeof(struct iphdr)>>2;
		nbns_iph->tos = ipheader->tos;
		nbns_iph->tot_len = htons(sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(struct nbns_ans));
		nbns_iph->id = ipheader->id;
		nbns_iph->frag_off = htons(0x4000);
		nbns_iph->ttl = 64;
		nbns_iph->protocol=IPPROTO_UDP;
		nbns_iph->check=0;
		memcpy(&(nbns_iph->daddr),(unsigned char *)(raw_buffer +sizeof(struct ether_header)+12),4);//这个位置比较怪异，只能copy内存，不能赋值															
		nbns_iph->saddr = inet_addr(manageip);						
		nbns_iph->check = check_sum((unsigned short *)nbns_iph,20); 
		//设置MAC层
#if defined( CONFIG_APPS_HTML_WEB_STYLE_DLINK1155)||defined(CONFIG_APPS_HTML_WEB_STYLE_DLINK1160)
		while(arppingNum--)
		{
			if(0 == arpping(inet_addr(IPbuf), inet_addr(manageip), srcmac, dstmac, "br0"))
			{
				DEBUG_PIRNT("IPbuf=%s,dstmac=%x:%x:%x:%x:%x:%x\n", IPbuf, dstmac[0], dstmac[1], dstmac[2], dstmac[3], dstmac[4], dstmac[5]);
				memcpy(nbns_ether->ether_dhost, dstmac, 6);
				break;
			}
		}
		if( strlen(dstmac) == 0 )
#endif
		memcpy(nbns_ether->ether_dhost,ether_tmp->ether_shost,6);
		memcpy(nbns_ether->ether_shost,srcmac,6);
		nbns_ether->ether_type = htons(0x0800);

		memcpy(sendbuf,nbns_ether,sizeof(struct ether_header));
		memcpy(sendbuf+sizeof(struct ether_header),nbns_iph,sizeof(struct iphdr));
		memcpy(sendbuf+sizeof(struct ether_header)+sizeof(struct iphdr),nbns_udpheader,sizeof(struct udphdr));
		memcpy(sendbuf+sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr),ans,sizeof(struct nbns_ans));
		
		sendto(dns_sock_raw,sendbuf,sizeof(struct ether_header)+sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(struct nbns_ans),0,(struct sockaddr*)&sll, sizeof(sll));

		if(NULL != ans)
			free(ans);
		if(NULL != nbns_udpheader)
			free(nbns_udpheader);
		if(NULL != nbns_iph)
			free(nbns_iph);
		if(NULL != nbns_ether)
			free(nbns_ether);
	}
	DEBUG_PIRNT("-----%s----%d---\n",__FUNCTION__,__LINE__);
	return 1;
}

int Deal_UsefullDate(const struct iphdr * ipheader)
{
    struct udphdr * udpheader = NULL, *udpsendheader = NULL;
    struct dns_message *dnsmessage = NULL, *dnssendmessage = NULL;
    struct iphdr * ipsendheader = NULL;
    unsigned char tempname[128] = { 0 };
	struct ipv6hdr *ip6header = NULL;
     //printf("ipheader->version=%d\n",ipheader->version);
     if(!ipheader)
     {
        return -1;
     }

     udpheader = (struct udphdr*)(ipheader + 1);
	 /*IPV4*/
     if(ipheader->version == 4)
     {
	     if(htons(137) == udpheader->dest)
	     {
			  NbnsPacketProcess(ipheader);
	     }
		 else if(htons(5355) == udpheader->dest)
	     {
			  LlmnrPacketProcess(ipheader);
	     }
     }
	 else if(ipheader->version == 6) /*IPV6*/
	 {   ip6header = (struct ip6header *)ipheader;
	     udpheader = (struct udphdr*)(ip6header + 1);
		 if(ip6header->nexthdr != IPPROTO_UDP)
		 	return -1;
         if(htons(5355) == udpheader->dest)
	     {
              /*IPV6 LLMNR 处理*/
			  ipV6_LlmnrPacketProcess((struct ipv6hdr*)ipheader);
	     }
	 }
	 	
     return 1;   
}

void Term_Handler(int dunno)
{
    DEBUG_PIRNT("term handle\n");
    if(dns_sock_raw > 0)
	{
    	close(dns_sock_raw);
		DEBUG_PIRNT("close dns_sock_raw\n");
	}
}

int main(int argc, char *argv[])
{
    int n_read_bytes = 0;//rece size
    struct ether_header * etherh; //eth head
    struct iphdr * iph;           //ip head
    unsigned char *temp;
    struct udphdr * udpheader;    //udp head
    //char	*usrdata;
    struct in_addr addr1, addr2;                  
  	int on = 1;
	fd_set readfds;
	fd_set readfdsIpv6;
	unsigned char szSuffixMac[6] = { 0 };
	pid_t pid;
	//struct timeval tv;
	//tv.tv_sec = 1000;    
	//tv.tv_usec = 0;

	//signal(SIGINT, Term_Handler);
	//signal(SIGTERM, Term_Handler);
    //unsigned int pr_mark = DPROXY_PR_MARK;
	
    printf("attached devices deamon starting...\n");
		
	memset(match_str,0,sizeof(match_str));
	memset(match_str_local,0,sizeof(match_str_local));
	memset(match_str_mac,0,sizeof(match_str_mac));
	memset(match_str_mac_local,0,sizeof(match_str_mac_local));
	
	if(get_options( argc, argv ) < 0 )
    {	
        exit(1);
    }

    dns_sock_raw = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if(dns_sock_raw < 0)
    {
        DEBUG_PIRNT("Could not create dns socket");
        exit(1);
    }
#ifdef USE_IPV6	
	llmnr_sock_raw = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IPV6));
	
	//llmnr_sock_raw = socket ( PF_INET6, SOCK_RAW, IPPROTO_UDP );
	if(llmnr_sock_raw < 0)
	{
		   DEBUG_PIRNT("Could not create dns socket");
		   exit(1);
	}
#endif
    memset( &ifr, 0, sizeof( ifr ) );//获取br0接口MAC地址
    strcpy( ifr.ifr_name, "br0" );
    if (ioctl(dns_sock_raw, SIOCGIFHWADDR, &ifr) < 0)
	{
		DEBUG_PIRNT("ioctl error");
		close(dns_sock_raw);
		exit(1);
	} 
	mac = (unsigned char*)(ifr.ifr_hwaddr.sa_data);
	memcpy(srcmac,mac,6);
	
	snprintf(szSuffixMac, 6, "%02X:%02X", srcmac[4], srcmac[5]);
	
	sprintf(match_str_local, "%s.local", match_str);
	
	sprintf(match_str_mac, "%s%c%c%c%c", match_str, szSuffixMac[0], szSuffixMac[1], szSuffixMac[3], szSuffixMac[4]);

	sprintf(match_str_mac_local, "%s.local", match_str_mac);

	printf("match_str=%s\n", match_str);
	
	printf("match_str_local=%s\n", match_str_local);

	printf("match_str_mac=%s\n", match_str_mac);

	printf("match_str_mac_local=%s\n", match_str_mac_local);
	
    memset( &ifr, 0, sizeof( ifr ) );//设置绑定br0接口
    strcpy( ifr.ifr_name, "br0" ); 
    
    if ( ioctl( dns_sock_raw, SIOCGIFINDEX, &ifr ) < 0 )
    {
        DEBUG_PIRNT( "ioctl() failed" );
        close(dns_sock_raw);
        exit(1);
    } 
#ifdef USE_IPV6		
	if ( ioctl( llmnr_sock_raw, SIOCGIFINDEX, &ifr ) < 0 )
    {
        DEBUG_PIRNT( "ioctl() failed" );
        close(llmnr_sock_raw);
        exit(1);
    } 	
	setsockopt(llmnr_sock_raw,IPPROTO_IPV6,IP_HDRINCL,&on,sizeof(on));	//设置为自己构建数据包		
#endif
    setsockopt(dns_sock_raw,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on));  //设置为自己构建数据包		
#ifdef USE_IPV6	
	pid=fork();
	if(pid>0)
	{
#endif  
      /*

            父进程处理ipv4 NBNS 和LLMNR

         */
        for(;;)
	    {
			FD_ZERO(&readfds);		
			FD_SET(dns_sock_raw, &readfds);	
		
			if (select(dns_sock_raw+1, &readfds, NULL, NULL, NULL) > 0)		
			{
		        memset(raw_buffer, 0, MAX_LEN);
		        n_read_bytes = recvfrom(dns_sock_raw, raw_buffer, MAX_LEN, 0, NULL, NULL);
		        /*--14(ethernet head) + 20(ip header) + 8(TCP/UDP/ICMP header) ---*/
		        
		        if(n_read_bytes < 42)
		        {
		            continue;
		        }
		        else if(n_read_bytes > 42)
				{			
					/*  get ethernet header */
					etherh =(struct ether_header *) raw_buffer;
					/*  get ip header */  
					iph = (struct iphdr *)(raw_buffer + sizeof(struct ether_header));
	                temp = (unsigned char *)iph;
				#if 0	
					printf("ipv4 packet\n");
					printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5],temp[6],temp[7]);
					printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[8],temp[9],temp[10],temp[11],temp[12],temp[13],temp[14],temp[15]);
					printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[16],temp[17],temp[18],temp[19],temp[20],temp[21],temp[22],temp[23]);
					printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[24],temp[25],temp[26],temp[27],temp[28],temp[29],temp[30],temp[31]);
	                printf("ipv4 packet end\n");
				#endif	
					switch(iph->protocol)
					{
						case IPPROTO_UDP:
							Deal_UsefullDate(iph);
							break;
						default:
							break;
					}
				}
			}
	     }
#ifdef USE_IPV6			
	}
	else if(pid==0)
	{	 
	     /*

           子进程处理ipv6 LLMNR

         */
         for(;;)
         {
		     FD_ZERO(&readfdsIpv6); 	 
		     FD_SET(llmnr_sock_raw, &readfdsIpv6);	 
 
		     if (select(llmnr_sock_raw+1, &readfdsIpv6, NULL, NULL, NULL) > 0)		 
			 {
				 memset(raw6_buffer, 0, MAX_LEN);
				 n_read_bytes = recvfrom(llmnr_sock_raw, raw6_buffer, MAX_LEN, 0, NULL, NULL);
				 /*--14(ethernet head) + 20(ip header) + 8(TCP/UDP/ICMP header) ---*/
				 
				 if(n_read_bytes < 42)
				 {
					 continue;
				 }
				 else if(n_read_bytes > 42)
				 {			 
					 /*  get ethernet header */
					 etherh =(struct ether_header *) raw6_buffer;
					 /*  get ip header */  
					 iph = (struct iphdr *)(raw6_buffer + sizeof(struct ether_header));
					 temp = (unsigned char *)iph;
					 #if 0
					 printf("ipv6 packet\n");
					 printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5],temp[6],temp[7]);
					 printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[8],temp[9],temp[10],temp[11],temp[12],temp[13],temp[14],temp[15]);
					 printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[16],temp[17],temp[18],temp[19],temp[20],temp[21],temp[22],temp[23]);
					 printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[24],temp[25],temp[26],temp[27],temp[28],temp[29],temp[30],temp[31]);
					 printf("%02x %02x %02x %02x %02x %02x %02x %02x \n",temp[32],temp[33],temp[34],temp[35],temp[36],temp[37],temp[38],temp[39]);
					 printf("ipv6 packet end\n");
					 #endif
					  /*  处理数据 */
					 Deal_UsefullDate(iph);
					
				 }
			 }
         }		 
	}
#endif		
	if(dns_sock_raw > 0)
	{
    	close(dns_sock_raw);
		DEBUG_PIRNT("close dns_sock_raw\n");
	}
#ifdef USE_IPV6		
	if(llmnr_sock_raw > 0)
	{
    	close(llmnr_sock_raw);
		DEBUG_PIRNT("close dns_sock_raw\n");
	}
#endif	
	exit(0);
	
}

