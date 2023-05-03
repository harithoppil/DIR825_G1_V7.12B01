/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENZHEN) Co., Ltd.
 �ļ�����: new_conf.h
 �ļ�����: 
 �޶���¼:
        1. ����: lichuang
           ����: 2009-3-23
           ����: �����ļ�
**********************************************************************/

#ifndef __NEW_CONF_H__
#define __NEW_CONF_H__

//#define USE_CONF

/**/
#define INTERFACE_LEN 	16
#define CONF_FILE_LEN 	64

#define IPV4_ENABLE      (1)
#define IPV6_ENABLE      (2)

#ifdef USE_IPV6
#define NUM_OF_PROTO 2
#define IP_LEN 	     64
#else
#define NUM_OF_PROTO 1
#define IP_LEN 	     20
#endif
#define NUM_OF_DNS 	 3

#define PROTO_IPV4 0
#define PROTO_IPV6 1

#define  DEFAULT_CACHE_FILE    "/var/cache/.cache"

typedef struct ST_LAN_INFO
{
	char szName[INTERFACE_LEN];
	struct ST_LAN_INFO *pNext;
    char szWanIf[INTERFACE_LEN];
	char szDnsIp[NUM_OF_PROTO][NUM_OF_DNS][IP_LEN];
	char szCacheFile[CONF_FILE_LEN];
	int ipPro;
}ST_LAN_INFO;

/* �����ļ��ṹ�� */
typedef struct 
{
	ST_LAN_INFO* pLanInfo;
	unsigned char ucDeamon;				/* �Ƿ��黯���� */
	int nPurgeTime;						/* ��յ�ʱ�� */
	char szConfigFile[CONF_FILE_LEN];	/* �����ļ��� */
	char szHostFile[CONF_FILE_LEN];		/* HOST�ļ��� */
}ST_DNS_CONFIG;

extern ST_DNS_CONFIG g_stConfig;

/*************************************************************************
  ��  ��: ��ȡ�����ļ�
  ��  ��: 
  		pConfig �����ļ�·��
  ����ֵ: �ɹ�����0, ʧ�ܷ���-1  
*************************************************************************/
int load_config(const char* pConfig);

#endif /* __NEW_CONF_H__ */

