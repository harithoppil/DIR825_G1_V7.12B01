/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 �ļ�����: tbsipv6.c
 �ļ�����: ipv6��صĹ����ӿ�

 �޶���¼:
        1. ����: XuXiaodong
           ����: 2009-12-24
           ����: �����ļ�
**********************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/if.h>
#include <stdlib.h>
#include "tbserror.h"
#include "common.h"
#include "tbsipv6.h"

/**************************************************************************
����: ͨ����������ʽ��IPV6��ַ��ȡIPV6��ַ����
����: pstIp6Addr, ���IPV6�����Ƶ�ַ�Ľṹָ��
����: �ɹ� -- ���ص�ַ����
      ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int tbsGetIp6AddrType(const struct in6_addr *pstIp6Addr)
{
	unsigned long ulVal;

    if (pstIp6Addr == NULL)
    {
        return TBS_FAILED;
    }

	ulVal = ntohl(pstIp6Addr->s6_addr32[0]);

    if ((ulVal & 0xFFC00000) == 0xFE800000)
    {
        return IP6_ADDR_LINKLOCAL;
    }

    if ((ulVal & 0xFE000000) == 0xFC000000)
    {
        return IP6_ADDR_UNIQUELOCAL;
    }

    if ((ulVal & 0xE0000000) == 0x20000000)
    {
        return IP6_ADDR_GLOBALUNIQUE;
    }

    if ((ulVal & 0xFF000000) == 0xFF000000)
    {
        return IP6_ADDR_MULTICAST;
    }

    if (pstIp6Addr->s6_addr32[0] == 0 &&
        pstIp6Addr->s6_addr32[1] == 0 &&
        pstIp6Addr->s6_addr32[2] == 0)
    {
        if (ntohl(pstIp6Addr->s6_addr32[3]) == 0x00000001)
        {
            return IP6_ADDR_LOOPBACK;
        }

        if (pstIp6Addr->s6_addr32[3] == 0)
        {
            return IP6_ADDR_UNSPECIFIED;
        }
    }

    return IP6_ADDR_UNKNOWNTYPE;
}

/**************************************************************************
����: ͨ���ַ�����ʽ��IPV6��ַ��ȡIPV6��ַ����
����: pszIp6Addr, IPV6��ַ�ַ���ָ��
����: �ɹ� -- ���ص�ַ����
      ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int tbsGetIp6AddrTypeByStr(const char *pszIp6Addr)
{
    struct in6_addr pstAddr;
    int iRet = TBS_SUCCESS;

    if (pszIp6Addr == NULL)
    {
        return TBS_FAILED;
    }

    iRet = inet_pton(AF_INET6, pszIp6Addr, (void *)&pstAddr);
    if (iRet <= 0)
    {
        return TBS_FAILED;
    }

    return tbsGetIp6AddrType(&pstAddr);
}

/**************************************************************************
����: �ӿڵ�IPV6���û���ÿ���
����: pszDevName, �ӿ�����
      iHow, 1��ʾ����, 0��ʾ����
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
***************************************************************************/
int tbsIfIpv6Switch(const char *pszDevName, int iHow)
{
#ifdef _TBS_ENV_PC
    return TBS_SUCCESS;
#else
    #define SIOCSIFIPV6     0x89D1
    int iSockFd = 0;
    int iRet = TBS_SUCCESS;
    struct ifreq stIfr;

    if (pszDevName == NULL)
    {
        return TBS_NULL_PTR;
    }

    if (pszDevName[0] == '\0')
    {
        return TBS_FAILED;
    }

    iSockFd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (iSockFd <= 0)
    {
        return TBS_FAILED;
    }

    safe_strncpy(stIfr.ifr_name, pszDevName, MAX_IF_NAME_LEN);
    stIfr.ifr_ipv6 = iHow;
    iRet = ioctl(iSockFd, SIOCSIFIPV6, &stIfr);

    close(iSockFd);
    return iRet;
#endif
}

/**************************************************************************
����: ���ýӿڵ�IPV6����
����: pszDevName, �ӿ�����
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
***************************************************************************/
int tbsEnableIfIpv6(const char *pszDevName)
{
    return tbsIfIpv6Switch(pszDevName, 1);
}

/**************************************************************************
����: ���ýӿڵ�IPV6����
����: pszDevName, �ӿ�����
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
***************************************************************************/
int tbsDisableIfIpv6(const char *pszDevName)
{
    return tbsIfIpv6Switch(pszDevName, 0);
}

/**************************************************************************
����: ����EUI64������ͨ��MAC��ַ��ȡInterface ID, MAC��ַ�����Ƹ�ʽ
����: pszMacStr, MAC��ַ������ָ��
      pucIfId, ���Interface ID�Ļ�����ָ��
      ulLen, pucIfIdָ��Ļ������Ĵ�С,����Ϊ8�ֽ�
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
***************************************************************************/
int tbsGetEui64IfIdByMac(const unsigned char *pucMacAddr, unsigned char *pucIfId,
                         unsigned int uiLen)
{
    if (pucMacAddr == NULL ||
        pucIfId == NULL)
    {
        return TBS_NULL_PTR;
    }

    if (uiLen < 8)
    {
        return TBS_FAILED;
    }

    memcpy(pucIfId, pucMacAddr, 3);
    memcpy(pucIfId+5, pucMacAddr+3, 3);

    pucIfId[3] = 0xFF;
    pucIfId[4] = 0xFE;
    pucIfId[0] ^= 2;

    return TBS_SUCCESS;
}

/**************************************************************************
����: ����EUI64������ͨ��MAC��ַ��ȡInterface ID, MAC��ַ���ַ����ĸ�ʽ
����: pszMacStr, MAC��ַ�ַ���ָ��
      pucIfId, ���Interface ID�Ļ�����ָ��
      ulLen, pucIfIdָ��Ļ������Ĵ�С,����Ϊ8�ֽ�
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
***************************************************************************/
int tbsGetEui64IfIdByMacStr(const char *pszMacStr, unsigned char *pucIfId, unsigned int uiLen)
{
    unsigned char szMacAddr[6] = {0};
    int iRet = TBS_SUCCESS;

    if (pszMacStr == NULL ||
        pucIfId == NULL)
    {
        return TBS_NULL_PTR;
    }

    if (!tbsCheckMac(pszMacStr))
    {
        return TBS_FAILED;
    }

    if (uiLen < 8)
    {
        return TBS_FAILED;
    }

    iRet = sscanf(pszMacStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                  &szMacAddr[0], &szMacAddr[1], &szMacAddr[2],
                  &szMacAddr[3], &szMacAddr[4], &szMacAddr[5]);
    if (iRet != 6)
    {
        return TBS_FAILED;
    }

    return tbsGetEui64IfIdByMac(szMacAddr, pucIfId, uiLen);
}

/**************************************************************************
����: ��ȡ�����Interface ID
����: pucIfId, ���Interface ID�Ļ�����ָ��
      ulLen, pucIfIdָ��Ļ������Ĵ�С,����Ϊ8�ֽ�
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
***************************************************************************/
int tbsGetRandomIfId(unsigned char *pucIfId, unsigned int uiLen)
{
    unsigned long *pulVal = NULL;

    if (pucIfId == NULL)
    {
        return TBS_NULL_PTR;
    }

    if (uiLen < 8)
    {
        return TBS_FAILED;
    }

    pulVal = (unsigned long *)pucIfId;
    pulVal[0] = random();
    pulVal[1] = random();
    pucIfId[0] &= ~2;

    return TBS_SUCCESS;
}

/**************************************************************************
����: ��IPV6��ַǰ׺��Interface ID����ƴ��,����IPV6��ַ
����: pstIp6Prefix, ���IPV6�����Ƹ�ʽǰ׺
      pucIfId, ���Interface ID�Ļ�����ָ��
      ulLen, pucIfIdָ��Ļ������Ĵ�С,����Ϊ8�ֽ�
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
***************************************************************************/
int tbsGenerateIp6Addr(struct in6_addr *pstIp6Prefix, unsigned char *pucIfId,
                       unsigned int uiLen)
{
    unsigned long *pulVal;
    int i = 0;

    if (pstIp6Prefix == NULL ||
        pucIfId == NULL)
    {
        return TBS_NULL_PTR;
    }

    if (pstIp6Prefix->s6_addr32[0] == 0)
    {
        return TBS_FAILED;
    }

    if (uiLen < 8)
    {
        return TBS_FAILED;
    }

    pulVal = (unsigned long *)pucIfId;
    if (pulVal[0] == 0 && pulVal[1] == 0)
    {
        return TBS_FAILED;
    }

    for (i = 0; i < 8; i++)
    {
        pstIp6Prefix->s6_addr[i+8] = pucIfId[i];
    }

    return TBS_SUCCESS;
}
/**************************************************************************
����: ����IPv6��Link-local��ַ
����: pstIp6Prefix, ���IPV6�����Ƹ�ʽǰ׺
      pucIfId, ���Interface ID�Ļ�����ָ��
      ulLen, pucIfIdָ��Ļ������Ĵ�С,����Ϊ8�ֽ�
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע: huyu 2010/10/28
***************************************************************************/
int tbsGetLLIp6Addr(char *DevMac,char *IP6Addr)
{
    int iRet = 0;
    unsigned char pucIfId[9];
    const unsigned char *IP6Addr_pre = "fe80::";
    struct in6_addr pstIp6Addr;

    memset(&pstIp6Addr, 0, sizeof(pstIp6Addr));
	iRet = tbsIp6AddrStr2Num(&pstIp6Addr, IP6Addr_pre);
	if(iRet != 0)
	{
	    return TBS_SUCCESS;
	}

    if(DevMac == NULL)
    {
        iRet = tbsGetRandomIfId(pucIfId, 8);
        if(iRet != 0)
        {
                printf("222222222222222222\n");
                return TBS_FAILED;
        }
    }
    else
    {
        iRet = tbsGetEui64IfIdByMacStr(DevMac, pucIfId, 8);
    	if(iRet != 0)
    	{
            printf("3333333333333333\n");
    	    return TBS_FAILED;
    	}
	}

    iRet = tbsGenerateIp6Addr(&pstIp6Addr, pucIfId, 8);
	if(iRet != 0)
	{
	    return TBS_SUCCESS;
	}

    iRet = tbsIp6AddrNum2Str(&pstIp6Addr, IP6Addr, 40);
	if(iRet != 0)
	{
	    return TBS_SUCCESS;
	}

	return iRet;

}

/**************************************************************************
����: ��IPV6��ַ���ַ�����ʽת���ɶ����Ƹ�ʽ
����: pstIp6Addr, ��Ŷ����Ƹ�ʽIPV6��ַ�Ľṹָ��
      pcIp6Addr,  ����ַ�����ʽIPV6��ַ�Ļ�����ָ��
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
***************************************************************************/
int tbsIp6AddrStr2Num(struct in6_addr *pstIp6Addr, const char *pcIp6Addr)
{
    int iRet = TBS_SUCCESS;

    if (pstIp6Addr == NULL ||
        pcIp6Addr == NULL)
    {
        return TBS_NULL_PTR;
    }

    iRet = inet_pton(AF_INET6, pcIp6Addr, (void *)pstIp6Addr);
    if (iRet <= 0)
    {
        return TBS_FAILED;
    }

    return TBS_SUCCESS;
}

/**************************************************************************
����: ��IPV6��ַ�Ӷ����Ƹ�ʽת�����ַ�����ʽ
����: pstIp6Addr, ��Ŷ����Ƹ�ʽIPV6��ַ�Ľṹָ��
      pcIp6Addr,  ����ַ�����ʽIPV6��ַ�Ļ�����ָ��
      uiLen, pcIp6Addrָ��Ļ�������С
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
***************************************************************************/
int tbsIp6AddrNum2Str(struct in6_addr *pstIp6Addr, char *pcIp6Addr, unsigned int uiLen)
{
    if (pstIp6Addr == NULL ||
        pcIp6Addr == NULL)
    {
        return TBS_NULL_PTR;
    }

    if (!inet_ntop(AF_INET6, (void *)pstIp6Addr, pcIp6Addr, uiLen))
    {
        return TBS_FAILED;
    }

    return TBS_SUCCESS;
}

/**************************************************************************
����: ���Unique Local��ַ�е�GlocalID���ִ��Ƿ�Ϸ�
����: pszGlobalID, ð�ŷָ���ʽ��Global ID
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע: Global ID�ĸ�ʽΪXX:XX:XX:XX:XX,����X��ʾʮ�������ַ�
***************************************************************************/
int tbsCheckGlobalID(char *pszGlobalID)
{
    const char *pszPatten = "^([[:xdigit:]]{2}:){4}[[:xdigit:]]{2}$";

    if (!pszGlobalID)
    {
        return TBS_NULL_PTR;
    }

    if (!tbsMatch(pszGlobalID, pszPatten))
    {
        return TBS_FAILED;
    }

    return TBS_SUCCESS;
}

/*************************************************************************
����: IPV6��ַ��麯��
����: const char *pszValue                �����ֵ
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
**************************************************************************/
int tbsCheckIpv6Addr(const char *pszIpv6Addr)
{
    struct in6_addr pstAddr;
    int iRet = TBS_SUCCESS;

    if (pszIpv6Addr == NULL)
    {
        return TBS_NULL_PTR;
    }

    iRet = inet_pton(AF_INET6, pszIpv6Addr, (void *)&pstAddr);
    if (iRet <= 0)
    {
        return ERR_IPV6_INVALID_FORMAT;
    }

    return TBS_SUCCESS;
}
/*************************************************************************
����: IPV6�����麯��
����: const char *pszValue                �����ֵ
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
**************************************************************************/
int tbsCheckIpv6Net(const char *pszIpv6Net)
{
    struct in6_addr pstAddr;
    int iRet = TBS_SUCCESS;
	int n = 0;
	char *ip6Addr;
	char *ip6Mask;

    if (pszIpv6Net == NULL)
    {
        return TBS_NULL_PTR;
    }

	char Ipv6Net[MAX_IP6_LEN] = {0};
	n = strlen(pszIpv6Net) < sizeof(MAX_IP6_LEN)? strlen(pszIpv6Net): sizeof(Ipv6Net) -1;
	strncpy(Ipv6Net, pszIpv6Net, n);
	ip6Addr = Ipv6Net;
	ip6Mask = strchr(Ipv6Net, '/');

	if(!ip6Mask)
		ip6Mask = "128";
	else
	{
		*ip6Mask = 0;
		ip6Mask++;
	}

	if(atoi(ip6Mask) >128 ||atoi(ip6Mask) < 1)
	{
		return TBS_FAILED;
	}

	iRet = inet_pton(AF_INET6, ip6Addr, (void *)&pstAddr);
	if (iRet <= 0)
	{
	return ERR_IPV6_INVALID_FORMAT;
	}

    return TBS_SUCCESS;
}


/*
return: 0, ��ͬ, 1, ���ڣ�-1�� С��, ��������

*/
int tbsIPv6AddrCmp(const char * pszAddr1, const char * pszAddr2)
{
    struct in6_addr stAddr1;
    struct in6_addr stAddr2;
  	int i, iRet=0;


    if (NULL ==pszAddr1 || NULL ==  pszAddr2)
    {
        return TBS_NULL_PTR;
    }

    iRet = inet_pton(AF_INET6, pszAddr1, (void *)&stAddr1);
    if (iRet <= 0)
    {
        return ERR_IPV6_INVALID_FORMAT;
    }

    iRet = inet_pton(AF_INET6, pszAddr2, (void *)&stAddr2);
    if (iRet <= 0)
    {
        return ERR_IPV6_INVALID_FORMAT;
    }

  	for (i = 0; i < 16; i++) {
  		if (stAddr1.s6_addr[i] != stAddr2.s6_addr[i]) {
          	if (stAddr1.s6_addr[i] > stAddr2.s6_addr[i])
  				return CMP_LOGIC_BIG;
  			else
  				return CMP_LOGIC_SMALL;
  		}
  	}

  	return CMP_LOGIC_EQUAL;
}

int tbsCheckIPv6GlobalAddr(const char * pszValue)
{
    int iRet = TBS_SUCCESS;

    iRet = tbsGetIp6AddrTypeByStr(pszValue);
    /*��̬IPV6��ַ����ΪGUA����*/
    if (iRet != IP6_ADDR_GLOBALUNIQUE)
    {
        printf("Invalid Static ipv6 address: %s\n", pszValue);
        return ERR_V6CONN_INVALID_STATIC_ADDR;
    }

    return TBS_SUCCESS;
}

/*************************************************************************
����: IPV6��ַ�б��麯��
����: pszIp6AddrList��   �����ֵ
      iMaxCount��        ��ַ�б��е�ַ��������
      CheckFunc��        �ص�����,���ڶԵ�ַ������ļ��

����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע: ��ַ�б��еĵ�ַ��,�Ž��зָ�
**************************************************************************/
#define MAX_IPV6_ADDR_LIST_LEN  6*MAX_IP6_ADDR_LEN
int tbsCheckIpv6AddrList(const char *pszIp6AddrList, int iMaxCount,
                         int (*CheckFunc)(const char *))
{
    char szIp6Addrs[MAX_IPV6_ADDR_LIST_LEN] = {0};
    char *apszIp6Addr[5] = {NULL};
    struct in6_addr stIp6Addr, stIp6AddrTmp;
    int iCount = 0;
    int i = 0, j = 0;
    int iRet = TBS_SUCCESS;

    if (!pszIp6AddrList ||
        strlen(pszIp6AddrList) == 0 ||
        strlen(pszIp6AddrList) >= MAX_IPV6_ADDR_LIST_LEN ||
        iMaxCount > 6)
    {
        return ERR_INVALID_VAL;
    }

    memset(&stIp6Addr, 0, sizeof(struct in6_addr));
    memset(&stIp6AddrTmp, 0, sizeof(struct in6_addr));

	safe_strncpy(szIp6Addrs, pszIp6AddrList, MAX_IPV6_ADDR_LIST_LEN);
	iCount = tbsSplitString(szIp6Addrs, apszIp6Addr, ',', iMaxCount);
	for (i = 0; i < iCount; i++)
	{
        iRet = tbsCheckIpv6Addr(apszIp6Addr[i]);
        if (RET_FAILED(iRet))
        {
            return iRet;
        }

        /*���ûص�������IPV6��ַ���ж���ļ��*/
        if (CheckFunc)
        {
            iRet = CheckFunc(apszIp6Addr[i]);
            if (RET_FAILED(iRet))
            {
                return iRet;
            }
        }
	}

	/*����ַ�ظ�*/
	for (i = 0; i < iCount; i++)
	{
		for (j = i+1; j < iCount; j++)
		{
            tbsIp6AddrStr2Num(&stIp6Addr, apszIp6Addr[i]);
            tbsIp6AddrStr2Num(&stIp6AddrTmp, apszIp6Addr[j]);

	        if (!memcmp(&stIp6Addr, &stIp6AddrTmp, sizeof(struct in6_addr)))
	        {
                return ERR_IPV6_ADDR_REPEAT;
	        }
		}
	}

	return TBS_SUCCESS;
}

/*************************************************************************
����: IPV6��ַǰ׺���ȼ�麯��
����: const char *pszValue                �����ֵ
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
**************************************************************************/
int tbsCheckIpv6PrefixLen(const char *pszIpv6PrefixLen)
{
    int iLen = 0;

    if (pszIpv6PrefixLen == NULL)
    {
        return TBS_NULL_PTR;
    }

    iLen = safe_atoi(pszIpv6PrefixLen);
    if(iLen>128 || iLen<=0)
    {
        return ERR_IPV6_INVALID_PREFIX_LEN;
    }

    return TBS_SUCCESS;
}

/**************************************************************************
����: ���dhcpv6�е�DUID�ַ����Ƿ�Ϸ�
����: pszDUID, ð�ŷָ���ʽ��DUID
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע: DUID�ĸ�ʽΪXX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX:XX,����X��ʾʮ�������ַ�
***************************************************************************/
int tbsCheckDUID(char *pszDUID)
{
    char *pszPatten = "^([[:xdigit:]]{2}:){13}[[:xdigit:]]{2}$";

    if (!tbsMatch(pszDUID, pszPatten))
    {
        return ERR_IPV6_INVALID_DUID;
    }

    return TBS_SUCCESS;
}

