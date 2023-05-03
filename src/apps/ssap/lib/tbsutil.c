/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ�����: tbsutil.c
 �ļ�����: ���������ķ�װ���ṩ����ģ��ʹ��

 �޶���¼:
        1. ����: all
           ����: 2008-08-07
           ����: �����ļ�

**********************************************************************/

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <asm/types.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <regex.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/sysinfo.h> /* wuyun, 2010-01-21, for sysinfo() */
//wulihua 2010-1-20 add

#include "tbsutil.h"
#include "tbsipv6.h"
#include "tbserror.h"
#include "common.h"
#include "autoconf.h"

#define MAX_INPUT_LENGTH 64
#define MAX_KEY_LENGTH 32
#define MAX_BLOCK_LENGTH 16
// The following lookup tables and functions are for internal use only!
unsigned char AES_Sbox[] = {99,124,119,123,242,107,111,197,48,1,103,43,254,215,171,
  118,202,130,201,125,250,89,71,240,173,212,162,175,156,164,114,192,183,253,
  147,38,54,63,247,204,52,165,229,241,113,216,49,21,4,199,35,195,24,150,5,154,
  7,18,128,226,235,39,178,117,9,131,44,26,27,110,90,160,82,59,214,179,41,227,
  47,132,83,209,0,237,32,252,177,91,106,203,190,57,74,76,88,207,208,239,170,
  251,67,77,51,133,69,249,2,127,80,60,159,168,81,163,64,143,146,157,56,245,
  188,182,218,33,16,255,243,210,205,12,19,236,95,151,68,23,196,167,126,61,
  100,93,25,115,96,129,79,220,34,42,144,136,70,238,184,20,222,94,11,219,224,
  50,58,10,73,6,36,92,194,211,172,98,145,149,228,121,231,200,55,109,141,213,
  78,169,108,86,244,234,101,122,174,8,186,120,37,46,28,166,180,198,232,221,
  116,31,75,189,139,138,112,62,181,102,72,3,246,14,97,53,87,185,134,193,29,
  158,225,248,152,17,105,217,142,148,155,30,135,233,206,85,40,223,140,161,
  137,13,191,230,66,104,65,153,45,15,176,84,187,22};
 
unsigned char AES_ShiftRowTab[] = {0,5,10,15,4,9,14,3,8,13,2,7,12,1,6,11};
 
unsigned char AES_Sbox_Inv[256];
unsigned char AES_ShiftRowTab_Inv[16];
unsigned char AES_xtime[256];

static int s_itbsSystemAsyn = 0;      /* �첽����system */
static int s_itbsSystemMID = 0;       /* �첽����system��MID */
static BOOL g_bFactoryMode = FALSE;


/******************************************************************************
 *                                 �ַ�������                                 *
 ******************************************************************************/

/**
 * strlcpy - Copy a %NUL terminated string into a sized buffer
 * @dst: Where to copy the string to
 * @src: Where to copy the string from
 * @siz: size of destination buffer
 *
 * Compatible with *BSD: the result is always a valid
 * NUL-terminated string that fits in the buffer (unless,
 * of course, the buffer size is zero). It does not pad
 * out the result like strncpy() does.
 */
 /*********************************************************************
*  ����: ����/���õ�ǰ���̱���ʱ����core�ļ�
*  ����: iFlag  1 - ����, 0 - ����
*  ����: TBS_SUCCESS/TBS_FAILED
*  ����: ������ / 2010-08-17
*********************************************************************/
int tbsEnableCoreDump(int iFlag)
{
    int iRes = RLIMIT_CORE;
    struct rlimit stRlim;

    /* ��������core�ļ� */
    stRlim.rlim_cur = stRlim.rlim_max = iFlag ? RLIM_INFINITY : 0;
    if (0 != setrlimit(iRes, &stRlim))
    {
        printf("Error: setrlimit failed, %s\n", strerror(errno));
        return TBS_FAILED;
    }
    else
    {
        /* ����core�ļ����ɵ�·�� */
        system("echo /var/core > /proc/sys/kernel/core_pattern");
        printf("Set coredump file size to %lu, path = /var/core\n", stRlim.rlim_cur);
        return TBS_SUCCESS;
    }

    return TBS_FAILED;
}

size_t strlcpy(char *dst, const char *src, size_t siz)
{
    size_t ret = strlen(src);

    if (src == 0 || dst == 0)
        return 0;

    if (siz) {
        size_t len = (ret >= siz) ? siz - 1 : ret;
        memcpy(dst, src, len);
        dst[len] = '\0';
    }
    return ret;
}


/**
 * strlcat - Append a length-limited, %NUL-terminated string to another
 * @dst: The string to be appended to
 * @src: The string to append to it
 * @siz: The size of the destination buffer.
 */
size_t strlcat(char *dst, const char *src, size_t siz)
{
	size_t dsize = strlen(dst);
	size_t len = strlen(src);
	size_t res = dsize + len;

    if (src == 0 || dst == 0)
        return 0;

	/* This would be a bug */
	if (dsize >= siz)
	    return 0;

	dst += dsize;
	siz -= dsize;
	if (len >= siz)
		len = siz-1;
	memcpy(dst, src, len);
	dst[len] = 0;
	return res;
}


/*=========================================================================*/
/*  ��������: tbsMatch                                                     */
/*  ��������: �ַ���ƥ�亯��                                               */
/*  ��  ��  : pszString Ҫƥ����ַ���                                     */
/*            pszPattern   ƥ�����                                        */
/*  ��  ��  : BOOL   TRUE:ƥ��ɹ���FALSE����ƥ��                          */
/*  ��  ��  : tbs / 2008-04-19                                             */
/*=========================================================================*/
BOOL tbsMatch(const char *pszString, const char *pszPattern)
{
    int status;
    regex_t re;

    if (NULL == pszString || NULL == pszPattern)
    {
        return FALSE;
    }

    if (regcomp(&re, pszPattern, REG_EXTENDED|REG_NOSUB) != 0)
    {
        return FALSE; /* report error */
    }

    status = regexec(&re, pszString, (size_t) 0, NULL, 0);
    regfree(&re);

    if (status != 0)
    {
        return FALSE; /* report error */
    }

    return TRUE; /* match */
}

#ifdef _TBS_ENV_PC
#define execv(sh, cmd) \
{ \
    if (!printFlag) \
    { \
        printf("execv: %s\n", cmd[2]); \
    } \
    if (0 == strncmp(cmd[2], "kill ", 5) \
        || 0 == strncmp(cmd[2], "killall ", 8)) \
    { \
        system(cmd[2]); \
    } \
    else \
    { \
        execl("execl_null", cmd[2], NULL); \
    } \
}
#endif

/*************************************************************************
����: ���ص�ǰ�Ƿ�Ϊfactory mode
����: ��
����:
		true -- �ǹ���ģʽ
        false -- �ǹ���ģʽ
��ע:
**************************************************************************/
BOOL tbsFactoryMode()
{
    FILE *fp;
    char buf[80]={0};

    if(fp = fopen("/var/factorymode.txt","r"))
    {
        if (fgets(buf, sizeof(buf), fp))
        {
            if(strstr(buf, FACTORY_MODE) || strstr(buf, "1"))
            {
                /* ����ģʽ��ֹ����*/
                printf("FACTORY_MODE\n");
                fclose(fp);
                return TRUE;
            }
        }

        fclose(fp);
    }

    FALSE;
}

#ifdef CONFIG_APPS_CMDDAEMON
/*************************************************************************
����: ��ȡ��ǰ����ģʽ����ȫ�ֱ���g_bFactoryMode������ģʽ������cmddaemon
����: ��
����:
		��
��ע:
**************************************************************************/
void tbsConfigModeInit(void)
{
    g_bFactoryMode = tbsFactoryMode();

    /*Ŀǰֻ��logic�����л���cmddaemon����*/
    if(g_bFactoryMode)
        tbsSystem("/usr/bin/cmddaemon &", TBS_PRINT_CMD);
}


/*************************************************************************
����: ����tbsSystemΪ�첽ģʽ��ͨ��cmddaemon����ִ������
����: ��
����:
		��
��ע:
**************************************************************************/
void tbsSetSystemAsyn(unsigned short usMID)
{
    /*ֻ�Թ���ģʽ��Ч*/
    if(!g_bFactoryMode)
        return;

    s_itbsSystemAsyn = 1;
    s_itbsSystemMID = usMID;
    return;
}


/*************************************************************************
����: ����tbsSystemΪͬ��ģʽ����ͨ��cmddaemon����ִ������
����: ��
����:
		��
��ע:
**************************************************************************/
void tbsSystemAsynComplete(void)
{
    /*ֻ�Թ���ģʽ��Ч*/
    if(!g_bFactoryMode)
        return;

    s_itbsSystemAsyn = 0;
    s_itbsSystemMID = 0;
    return;
}
#endif

/*=========================================================================*/
/*  ��������: tbsSystem                                                    */
/*  ��������: ϵͳ����system�����                                         */
/*  ��  ��  : char* ���������                                             */
/*            int   �����ӡ��־��0������ӡ����������ӡ                    */
/*  ��  ��  : int   0:ִ�гɹ�������������                                 */
/*  ��  ��  : tbs / 2007-12-14                                             */
/*=========================================================================*/
int tbsSystem(char *command, int printFlag)
{
	int pid = 0, status = 0;

    if( !command )
    {
        printf("tbsSystem: Null Command, Error!");
        return -1;
    }

#ifdef CONFIG_APPS_CMDDAEMON
    /* �첽ģʽ */
    if (s_itbsSystemAsyn)
    {
        return COMM_MakeAndSendCustomMsg(MSG_CMDDAEMON_APPLYCMD_REQUEST,
                                         s_itbsSystemMID, MID_CMDDAEMON,
                                         1, "Command", command);
    }
#endif

	pid = fork();
  	if ( pid == -1 )
  	{
		return -1;
	}

  	if ( pid == 0 )
  	{
        char *argv[4];
    	argv[0] = "sh";
    	argv[1] = "-c";
    	argv[2] = command;
    	argv[3] = 0;
    	if (printFlag)
    	{
	        printf("[system]: %s\r\n", command);
            //syslog(LOG_DEBUG, command);
        }
    	execv("/bin/sh", argv);
    	exit(127);
	}

        errno = 0;
  	/* wait for child process return */
  	do
  	{
	  	if ( waitpid(pid, &status, 0) == -1 )
    	{
	    	if ( errno != EINTR )
    		{
            	return -1;
      	    }
	    }
    	else
    	{
	    	return status;
		}
	} while ( 1 );

	return status;
}

int tbsCheckIpRangeSize(const char *pStartIp, const char *pEndIp)
{
	unsigned long ipStart[4], ipEnd[4];
    int ret, i = 0;

	ret = sscanf(pStartIp, "%lu.%lu.%lu.%lu",
           &ipStart[0], &ipStart[1], &ipStart[2], &ipStart[3]);
    if(ret == 0 || ret < 4)
    {
    	return 0;
    }

	ret = sscanf(pEndIp, "%lu.%lu.%lu.%lu",
           &ipEnd[0], &ipEnd[1], &ipEnd[2], &ipEnd[3]);
    if(ret == 0 || ret < 4)
    {
    	return 0;
    }
	
	if(ipStart[0] > ipEnd[0])
	{
		return ERR_IP_INVALID_RANGE;
	}
	else if(ipStart[0] == ipEnd[0])
	{
		if(ipStart[1] > ipEnd[1])
		{
			return ERR_IP_INVALID_RANGE;
		}
		else if(ipStart[1] == ipEnd[1])
		{
			if(ipStart[2] > ipEnd[2])
			{
				return ERR_IP_INVALID_RANGE;
			}
			else if(ipStart[2] == ipEnd[2])
			{
				if(ipStart[3] > ipEnd[3])
				{
					return ERR_IP_INVALID_RANGE;
				}				
			}
		}
	}

	return TBS_SUCCESS;
}


/*
 * tbsCheckIpRange: ������������IP��ַ�Ƿ���һ����Χ֮��
 * pStartIp: 		��ʼIP��ַ
 * pEndIp: 			����IP��ַ
 * ����ֵ:			���������ͬһ������֮��,����TBS_SUCCESS, ���򷵻�TBS_ERROR
 * ��ע:			�ú�������IP��ַ����Ч�������, �ڵ���֮ǰ���ʹ��
 					tbsCheckIpEx�������м��
*/
int tbsCheckIpRange(const char *pStartIp, const char *pEndIp)
{
	in_addr_t stStartIp, stEndIp;

	stStartIp = inet_addr(pStartIp);
	stEndIp = inet_addr(pEndIp);

	return (stEndIp >= stStartIp) ? TBS_SUCCESS : ERR_IP_INVALID_RANGE;
}

int isBigEndian()
{
    union {
        short s;
        char c[sizeof(short)];
    } un;
    un.s = 0x0102;
    if(sizeof(short)==2) 
	{
        if(un.c[0]==1 && un.c[1] == 2)
        {
            //printf("big-endian\n");
			return 1;
        }
        else if (un.c[0] == 2 && un.c[1] == 1)
		{
            //printf("little-endian\n");
			return 0;
        }
        else
        {
            //printf("unknown\n");
			return -1;
        }
    } 
	else
    {
            printf("isBigEndian() unknown ,sizeof(short)=%d\n",sizeof(short));
			return -1;
    }
	
}


/**************************************************************************
����: ��IP�ַ����н���ip
����: const char *pcIp,                 IP�ַ���;
����: ip��ַ
��ע:
***************************************************************************/
unsigned long tbsCnvtIpFromStr(const char *pcIp)
{
    unsigned long ip[4];
    unsigned long retIp;
    int ret;
#ifndef _TBS_ENV_PC
    unsigned char pos[] = {24, 16, 8, 0};
#else
    unsigned char pos[] = {0, 8, 16, 24};
#endif

    ret = sscanf(pcIp, "%lu.%lu.%lu.%lu",
           &ip[0], &ip[1], &ip[2], &ip[3]);
    if(ret == 0 || ret < 4)
    {
    		return 0;
    }
    retIp = (ip[0] << pos[0]) | (ip[1] << pos[1]) | (ip[2] << pos[2]) | (ip[3] << pos[3]);

    return retIp;
}

/**************************************************************************
����: ��ipתΪ�ַ���
����:    unsigned long ulIp      IP��ַ
                char *pszIp                 IP�ַ���;
����: ��
��ע:
***************************************************************************/
void tbsCnvtIpToStr(unsigned long ulIp, char *pszIp)
{
	char szSectIp[4][4];
	int  iSectIp, i;
	unsigned long ulTempIp = ulIp;

	for(i=0; i<4; i++)
	{
		iSectIp = ulTempIp%256;
		snprintf(szSectIp[i], 4, "%d", iSectIp);

		ulTempIp -= iSectIp;
		ulTempIp /= 256;
	}

	snprintf(pszIp, 20, "%s.%s.%s.%s", szSectIp[3], szSectIp[2], szSectIp[1], szSectIp[0]);

}


/*=========================================================================*/
/*  ��������: tbsSplitString                                               */
/*  ��������: ����ָ���ַ��ָ��ַ���                                       */
/*  ��  ��  : str ��Ҫ�ָ���ַ���ָ��                                     */
/*            substr ���ָ��ָ�õ�Ԫ��ָ��                              */
/*            delimit �ָ���                                               */
/*            max_count ���Ԫ�ظ���                                       */
/*  ��  ��  : void  ��                                                     */
/*  ��  ��  : tbs / 2007-12-14                                             */
/*  ��  ��  : zoudeming /2007-12-27                                        */
/*            ��˫��ѭ����Ϊ��ѭ������������ĳЩ�β�                       */
/*            zoudeming /2008-1-3                                          */
/*            ����ع�                                                     */
/*=========================================================================*/
int tbsSplitString(char *str, char *substr[], const char delimit, const int max_count)
{
	int count = 0;

	if ( str == NULL || substr == NULL )
		return 0;

	if ( 0 < max_count )
			substr[0] = str;

	count = 1;
	while ( *str )
	{

		while ( *str != delimit && *str != '\0' )
			str++;

		if ( *str == delimit )
		{
			*str++ = '\0';
			if(count < max_count)
				substr[count] = str;
			count++;
		}
	}

	return count;
}


/*************************************************************************
����: �����ƶ��ַ��ָ��б��ַ���
      (����: xx,yy,zz, �ָ�֮����������{xx, yy, zz})
����: strList               ԭ�б��ַ���]
      delimit               �ָ��ַ�
      apcStrArray           ��ŷָ�õ�Ԫ��ָ��
      nMaxCnt               ���Ԫ�ظ���
����: �����б���ʵ��Ԫ�ظ���
��ע:
**************************************************************************/
int tbsStrListSplit(char *strList, char delimit, char *apcStrArray[], int nMaxCnt)
{
	int cnt = 0;

	if ( strList == NULL || apcStrArray == NULL )
		return 0;

	if (nMaxCnt > 0)
		apcStrArray[0] = strList;

	cnt = 1;
	while (*strList)
	{
		while ( *strList != delimit && *strList != '\0' )
			strList++;

		if ( *strList == delimit )
		{
			*strList++ = '\0';
			if(cnt < nMaxCnt)
				apcStrArray[cnt] = strList;
			cnt++;
		}
	}

	return cnt;
}
int tbsCheckIpInIpRange(const char *psIp, const char *pStartIp, const char *pEndIp)
{
	in_addr_t stIp, stStartIp, stEndIp;

	stIp = inet_addr(psIp);
	stStartIp = inet_addr(pStartIp);
	stEndIp = inet_addr(pEndIp);

	if(stIp >= stStartIp && stIp<= stEndIp)
	{
		return TBS_SUCCESS;
	}
	
	return ERR_IP_INVALID_RANGE;
}

int tbsCheckIpInIplistRange(char *pszInIp, char *pszIpList)
{
	char *psIpRange = NULL;
	char *pszIPRange[8] = {NULL};	
    char *pszIP[2] = {NULL};
	int i = 0, iCount = 0, ipCount = 0;

	psIpRange = malloc(strlen(pszIpList)*sizeof(char));
	if(NULL == psIpRange)
	{
		printf("malloc fail\n");
		return ERR_MALLOC_FAILED;
	}

	safe_strncpy(psIpRange, pszIpList, strlen(pszIpList));

	
	if(0 != safe_strstr(psIpRange, ","))//ip1-ip2,ip3,ip4-ip5
	{
		iCount = tbsSplitString(psIpRange, pszIPRange, ',', 8);

		if ( iCount > 8 )
		{
			printf("Error: ip_addr range count error\n");
			free(psIpRange);
			return ERR_IPLIST_TOO_MANY;
		}
		for (i = 0; i < iCount; i++)
		{
			if ( !pszIPRange[i] )
			{
				free(psIpRange);
				return ERR_IPLIST_INVALID_FORMAT;
			}
			if(0 != strstr(pszIPRange[i], "-"))
			{
				ipCount = tbsSplitString(pszIPRange[i], pszIP, '-', 2);
				if ( ipCount > 2 )
			    {
					free(psIpRange);
			        return ERR_IPLIST_INVALID_FORMAT;
			    }
				
				printf("ip1=%s, ip2=%s\n", pszIP[0], pszIP[1]);
				
				if ( !pszIP[0] || !tbsCheckIp(pszIP[0])
					|| !pszIP[1] || !tbsCheckIp(pszIP[1]))
		        {
					free(psIpRange);
		            return ERR_IP_INVALID_RANGE;
		        }
				
				if(TBS_SUCCESS != tbsCheckIpInIpRange(pszInIp, pszIP[0], pszIP[1]))
				{
					free(psIpRange);
					return ERR_IP_NOT_INRANGE;
				}
			}
			else
			{
				if( !pszIPRange[i] || !tbsCheckIp(pszIPRange[i]))
				{
					free(psIpRange);
		            return ERR_IP_INVALID_RANGE;
		        }
				
				if(TBS_SUCCESS != tbsCheckIpInIpRange(pszInIp, pszIPRange[i], pszIPRange[i]))
				{
					free(psIpRange);
					return ERR_IP_NOT_INRANGE;
				}
				
			}
		}	

	}
	else//ip1-ip2 or ip3
	{
		if(0 != safe_strstr(psIpRange, "-"))
		{
			ipCount = tbsSplitString(pszIPRange[i], pszIP, '-', 2);
			if ( ipCount > 2 )
		    {
				free(psIpRange);
		        return ERR_IPLIST_INVALID_FORMAT;
		    }
			
			printf("ip1=%s, ip2=%s\n", pszIP[0], pszIP[1]);
			
			if ( !pszIP[0] || !tbsCheckIp(pszIP[0])
				|| !pszIP[1] || !tbsCheckIp(pszIP[1]))
	        {
				free(psIpRange);
	            return ERR_IP_INVALID_RANGE;
	        }

			if(TBS_SUCCESS != tbsCheckIpInIpRange(pszInIp, pszIP[0], pszIP[1]))
			{
				free(psIpRange);
				return ERR_IP_NOT_INRANGE;
			}
		}
		else
		{
			if( !pszIPRange[i] || !tbsCheckIp(pszIPRange[i]))
			{
				free(psIpRange);
	            return ERR_IP_INVALID_RANGE;
	        }
			
			if(TBS_SUCCESS != tbsCheckIpInIpRange(pszInIp, pszIPRange[i], pszIPRange[i]))
			{
				free(psIpRange);
				return ERR_IP_NOT_INRANGE;
			}
		}
	}
	
	free(psIpRange);

	return TBS_SUCCESS;
}

int tbsCheckPortInPortlistRange(char *pszInPort, char *pszPortList)
{
	char *psPortRange = NULL;
	char *pszPortRange[8] = {NULL};	
    char *pszPort[2] = {NULL};
	int i = 0, iCount = 0, ipCount = 0;

	psPortRange = malloc(strlen(pszPortList)*sizeof(char));
	if(NULL == psPortRange)
	{
		printf("malloc fail\n");
		return ERR_MALLOC_FAILED;
	}

	safe_strncpy(psPortRange, pszPortList, strlen(pszPortList));
	
	if(0 != safe_strstr(psPortRange, ","))
	{
		iCount = tbsSplitString(psPortRange, pszPortRange, ',', 8);

		if ( iCount > 8 )
		{
			printf("Error: ip_addr range count error\n");
			free(psPortRange);
			return ERR_PORT_RANGE_INVALID;
		}
		for (i = 0; i < iCount; i++)
		{
			if ( !pszPortRange[i] )
			{
				free(psPortRange);
				return ERR_PORT_RANGE_INVALID;
			}
			if(0 != strstr(pszPortRange[i], "-"))
			{
				ipCount = tbsSplitString(pszPortRange[i], pszPort, '-', 2);
				if ( ipCount > 2 )
			    {
					free(psPortRange);
			        return ERR_PORT_RANGE_INVALID;
			    }
				
				printf("port1=%s, port2=%s\n", pszPort[0], pszPort[1]);
				
				if ( !pszPort[0] || !tbsCheckPort(pszPort[0])
					|| !pszPort[1] || !tbsCheckPort(pszPort[1]))
		        {
					free(psPortRange);
		            return ERR_PORT_RANGE_INVALID;
		        }

				if(atoi(pszInPort) <= atoi(pszPort[0]) && atoi(pszInPort) >= atoi(pszPort[1]))
				{					
					free(psPortRange);
					return ERR_PORT_NOT_INRANGE;
					
				}
				
			}
			else
			{
				if( !pszPortRange[i] || !tbsCheckPort(pszPortRange[i]))
				{
					free(psPortRange);
		            return ERR_PORT_RANGE_INVALID;
		        }
				
				if(atoi(pszInPort) != atoi(pszPortRange[i]))
				{					
					free(psPortRange);
					return ERR_PORT_NOT_INRANGE;
					
				}
			}
		}	

	}
	else
	{
		if(0 != safe_strstr(pszPortRange, "-"))
		{
			ipCount = tbsSplitString(pszPortRange[i], pszPort, '-', 2);
			if ( ipCount > 2 )
		    {
				free(psPortRange);
		        return ERR_PORT_RANGE_INVALID;
		    }
			
			printf("port1=%s, port2=%s\n", pszPort[0], pszPort[1]);
			
			if ( !pszPort[0] || !tbsCheckPort(pszPort[0])
				|| !pszPort[1] || !tbsCheckPort(pszPort[1]))
	        {
				free(psPortRange);
	            return ERR_PORT_RANGE_INVALID;
	        }

			if(atoi(pszInPort) <= atoi(pszPort[0]) && atoi(pszInPort) >= atoi(pszPort[1]))
			{					
				free(psPortRange);
				return ERR_PORT_NOT_INRANGE;
				
			}
		}
		else
		{
			if( !pszPortRange[i] || !tbsCheckPort(pszPortRange[i]))
			{
				free(psPortRange);
	            return ERR_PORT_RANGE_INVALID;
	        }
			if(atoi(pszInPort) != atoi(pszPortRange[i]))
			{					
				free(psPortRange);
				return ERR_PORT_NOT_INRANGE;
				
			}
		}
	}
	
	free(psPortRange);

	return TBS_SUCCESS;
}

int tbsCheckPortNotInPortlistRange(char *pszInPort, char *pszPortList)
{
	char *psPortRange = NULL;
	char *pszPortRange[8] = {NULL};	
    char *pszPort[2] = {NULL};
	int i = 0, iCount = 0, ipCount = 0;

	psPortRange = malloc(strlen(pszPortList)*sizeof(char));
	if(NULL == psPortRange)
	{
		printf("malloc fail\n");
		return ERR_MALLOC_FAILED;
	}

	safe_strncpy(psPortRange, pszPortList, strlen(pszPortList));
	
	if(0 != safe_strstr(psPortRange, ","))
	{
		iCount = tbsSplitString(psPortRange, pszPortRange, ',', 8);

		if ( iCount > 8 )
		{
			printf("Error: ip_addr range count error\n");
			free(psPortRange);
			return ERR_PORT_RANGE_INVALID;
		}
		for (i = 0; i < iCount; i++)
		{
			if ( !pszPortRange[i] )
			{
				free(psPortRange);
				return ERR_PORT_RANGE_INVALID;
			}
			if(0 != strstr(pszPortRange[i], "-"))
			{
				ipCount = tbsSplitString(pszPortRange[i], pszPort, '-', 2);
				if ( ipCount > 2 )
			    {
					free(psPortRange);
			        return ERR_PORT_RANGE_INVALID;
			    }
				
				printf("port1=%s, port2=%s\n", pszPort[0], pszPort[1]);
				
				if ( !pszPort[0] || !tbsCheckPort(pszPort[0])
					|| !pszPort[1] || !tbsCheckPort(pszPort[1]))
		        {
					free(psPortRange);
		            return ERR_PORT_RANGE_INVALID;
		        }

				if(atoi(pszInPort) >= atoi(pszPort[0]) && atoi(pszInPort) <= atoi(pszPort[1]))
				{					
					free(psPortRange);
					return ERR_PORT_NOT_INRANGE;
					
				}
				
			}
			else
			{
				if( !pszPortRange[i] || !tbsCheckPort(pszPortRange[i]))
				{
					free(psPortRange);
		            return ERR_PORT_RANGE_INVALID;
		        }
				
				if(atoi(pszInPort) == atoi(pszPortRange[i]))
				{					
					free(psPortRange);
					return ERR_PORT_NOT_INRANGE;
					
				}
			}
		}	

	}
	else
	{
		if(0 != safe_strstr(pszPortRange, "-"))
		{
			ipCount = tbsSplitString(pszPortRange[i], pszPort, '-', 2);
			if ( ipCount > 2 )
		    {
				free(psPortRange);
		        return ERR_PORT_RANGE_INVALID;
		    }
			
			printf("port1=%s, port2=%s\n", pszPort[0], pszPort[1]);
			
			if ( !pszPort[0] || !tbsCheckPort(pszPort[0])
				|| !pszPort[1] || !tbsCheckPort(pszPort[1]))
	        {
				free(psPortRange);
	            return ERR_PORT_RANGE_INVALID;
	        }

			if(atoi(pszInPort) >= atoi(pszPort[0]) && atoi(pszInPort) <= atoi(pszPort[1]))
			{					
				free(psPortRange);
				return ERR_PORT_NOT_INRANGE;
				
			}
		}
		else
		{
			if( !pszPortRange[i] || !tbsCheckPort(pszPortRange[i]))
			{
				free(psPortRange);
	            return ERR_PORT_RANGE_INVALID;
	        }
			if(atoi(pszInPort) == atoi(pszPortRange[i]))
			{					
				free(psPortRange);
				return ERR_PORT_NOT_INRANGE;
				
			}
		}
	}
	
	free(psPortRange);

	return TBS_SUCCESS;
}

/*************************************************************************
����: ��һ���б��ַ����в�������һ��Ԫ��
      (����: xx,yy,zz, ����yy)
����: szList                ԭ�б��ַ���]
      delimit               �ָ��ַ�
      szDel                 Ҫ���ҵ�Ԫ��
����: �ҵ��ⷵ���ַ�ָ�룬���򷵻�NULL
��ע:
**************************************************************************/
int tbsStrListFind(const char *szList, char delimit, const char *szEntry)
{
    char *p1, *p2;

    p2 = p1 = (char*)szList;
    while (p2)
    {
        /* ��ͬ��·��֮��ʹ��'�ָ��� */
        p2 = strchr(p1, delimit);
        if (p2)
        {
            if (!strncmp(szEntry, p1, p2 - p1))
            {
                return 1;
            }
            p2++;
            p1 = p2;
        }
        else
        {
            if (!strcmp(szEntry, p1))
            {
                return 1;
            }
        }
    }

    return 0;
}


/*************************************************************************
����: ��һ���б��ַ��������һ��Ԫ��
      (����: xx,yy, ���zz, ���� xx,yy,zz)
����: szList                ԭ�б��ַ���]
      delimit               �ָ��ַ�
      szDel                 Ҫ��ӵ�Ԫ��
����: true/false
��ע:
**************************************************************************/
int tbsStrListAdd(char *szList, char delimit, const char *szEntry)
{
    if (strlen(szList) > 0)
    {
        if (strlen(szEntry))
        {
            sprintf(szList+strlen(szList), "%c%s", delimit, szEntry);
        }
    }
    else
    {
        sprintf(szList, "%s", szEntry);
    }

    return 1;
}


/*************************************************************************
����: ��һ���б��ַ�����ɾ������һ��Ԫ��
      (����: xx,yy,zz, ɾ��yy, ���� xx,zz)
����: szList                ԭ�б��ַ���]
      delimit               �ָ��ַ�
      szDel                 Ҫɾ����Ԫ��
����: true/false
��ע:
**************************************************************************/
int tbsStrListDel(char *szList, char delimit, const char *szEntry)
{
  	char *p1, *p2 = NULL;
    int isFound = 0;

	p2 = p1 = szList;
	while (p2)
	{
	    /* ��ͬ��·��֮��ʹ��','�ָ��� */
	    p2 = strchr(p1, delimit);
	    if (p2)
	    {
	        if (0 == strncmp(szEntry, p1, p2 - p1))
	        {
	            memmove(p1, p2+1, strlen(p2));
                isFound = 1;
	            break;
	        }
	        p2++;
	        p1 = p2;
	    }
	    else
	    {
	        if (0 == strcmp(szEntry, p1))
	        {
				*p1 = '\0';

	            /* ȥ����β��',' */
				if (p1 > szList)
					*(p1 -1 ) = '\0';

                isFound = 1;
	        }
	    }
	}

    return isFound;
}


BOOL tbsStrToBool(const char *pcValue)
{
	if ( pcValue == NULL || strlen(pcValue) == 0)
	{
		return FALSE;
	}

	if ( strcasecmp(pcValue, "1") == 0 || strcasecmp(pcValue, "true") == 0 )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/**************************************************************************
����: ����������1�ĸ���,�Ƿ������򷵻ظ���
����:    const char *pcMask      �����ַ���
����: ������1�ĸ���
��ע:
***************************************************************************/
int tbsGetBitsByMask(const char *pcMask)
{
    int i, iBits = 32, iSet = 0;
    unsigned long ulMask;

    if (NULL == pcMask)
    {
        return TBS_PARAM_ERR;
    }

    if (!inet_aton(pcMask, (struct in_addr *)((void *)&ulMask)))
    {
        return TBS_PARAM_ERR;
    }

    ulMask = ntohl(ulMask);

    for (i = 0; i < 32; i++)
    {
        if (ulMask & (1 << i))
        {
            iSet = 1;
        }
        else
        {
            if (iSet)
            {
                return TBS_PARAM_ERR;;
            }

            iBits--;
        }
    }

    return iBits;
}




/****************************************************
*
*   ��麯��
*
****************************************************/

/* ���IP�Ϸ��� */
BOOL tbsCheckIp(const char *pcip)
{
	struct in_addr addr;

	if (!tbsMatch(pcip, "^([0-9]{1,3}\\.){3}[0-9]{1,3}$"))
        return FALSE;

	if(!inet_aton(pcip, &addr))
		return FALSE;

	return TRUE;
}

BOOL tbsCheckPort(const char *pszString)
{
    char *pszPattern = "^[1-9][0-9]\{0,4}$";

    if (TRUE != tbsMatch(pszString, pszPattern))
        return FALSE;

    if (atoi(pszString) < 1 || atoi(pszString) > 65535)
        return FALSE;

    return TRUE;
}

BOOL tbsCheckPriority(const char *pszString)
{
	if (TRUE != tbsMatch(pszString, "^[0-7]$"))
    {
		return FALSE;
    }

    return TRUE;
}


BOOL tbsCheckMask(const char *pcValue)
{
	unsigned long ulMask = 0;

	if ( pcValue == NULL || strlen(pcValue) == 0 )
	{
		return FALSE;
	}

	if(FALSE == tbsCheckIp(pcValue))
	{
		return FALSE;
	}

	ulMask = tbsCnvtIpFromStr(pcValue);
	if ( ulMask == 0 )
	{
		return FALSE;
	}

#ifdef _TBS_ENV_PC
    ulMask = htonl(ulMask);
#endif

	while ( ulMask & 0x80000000 )
	{
		ulMask <<= 1;
	}

	ulMask &= 0xffffffff;
	if ( ulMask == 0 )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



BOOL tbsCheckEnable(const char *pszValue)
{
    if ( NULL == pszValue )
    {
        return FALSE;
    }

	if ( 0 == strcmp(pszValue, "1") || 0 == strcmp(pszValue, "true") ||
			0 == strcmp(pszValue, "0") || 0 == strcmp(pszValue, "false") )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL tbsCheckInt(const char *pszValue)
{
	if ( NULL == pszValue )
	{
		return FALSE;
	}

	return tbsMatch(pszValue, "^(0|-?[1-9][0-9]{0,9})$");
}

BOOL tbsCheckUInt(const char *pszValue)
{
	if ( NULL == pszValue )
	{
		return FALSE;
	}

    return tbsMatch(pszValue, "^(0|[1-9][0-9]{0,9})$");
}

BOOL tbsCheckMac(const char *pcMac)
{
    int arr[6], index;
    const char *pattern = "^[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}$";

  if(pcMac == NULL)
    return FALSE;

  if(FALSE == tbsMatch(pcMac, pattern))
  {
  	return FALSE;
  }


  if(sscanf(pcMac,"%x:%x:%x:%x:%x:%x",\
           &arr[0],&arr[1],&arr[2],&arr[3],&arr[4],&arr[5]) != 6 )
    return FALSE;

#if 0
    /* ���MAC��ַ�Ƿ�Ϊ�㲥����ȫ��MAC */
    if (0 == strcasecmp(pcMac, "00:00:00:00:00:00")
       || 0 == strcasecmp(pcMac, "FF:FF:FF:FF:FF:FF")
    )
    {
        return FALSE;
    }
  /*���MAC��ַ�Ƿ�Ϊ�ಥMAC��ַ*/
  if( arr[0]%2 > 0 )
    return FALSE;
#endif

  /*���MAC��ַ�Ƿ�Ϸ�*/
  for(index = 1; index < 6; index ++)
  {
  	if(arr[index] > 0xff)
  	{
  		return FALSE;
  	}
  }
  return TRUE;
}




BOOL tbsCheckIpList(const char *pszIpList, int iMaxCount)
{
    return RET_SUCCEED(tbsCheckIpListEx(pszIpList, iMaxCount));
}

BOOL tbsCheckMacList(const char *pszMacList, int iMaxCount)
{
    return RET_SUCCEED(tbsCheckMacListEx(pszMacList, iMaxCount));
}


/*
����: tbsCheckRegex
����: ʹ�ô����������ʽƥ��Ҫ����ֵ,��ƥ���򷵻�ָ���Ĵ�����
����: pszValue      Ҫ����ֵ
      pszPattern    ������ʽ
      iErrNo        ������
����: �ɹ�,TBS_SUCCESS
      ʧ��,������
*/
int tbsCheckRegex(const char *pszValue, const char *pszPattern, int iErrNo)
{
    if ( !tbsMatch(pszValue, pszPattern) )
        return iErrNo;
    return TBS_SUCCESS;
}

/*
���IP�Ϸ��ԣ�������������ip
*/
int tbsCheckIpEx(const char *pszValue)
{
    if ( NULL == pszValue || 0 == strlen(pszValue))
    {
        return ERR_CAN_NOT_EMPTY;
    }

    if ( !tbsCheckIp(pszValue) )
    {
        printf("Invalid ip");
        return ERR_IP_INVALID_FORMAT;
    }

    return TBS_SUCCESS;
}


/*
���IP�Ϸ��ԣ�������Ϊ�鲥�㲥�������ַ
*/
int tbsCheckHostIpEx(const char *pszValue)
{
    unsigned long ulIp = 0;

    if ( NULL == pszValue || 0 == strlen(pszValue))
    {
        return ERR_CAN_NOT_EMPTY;
    }

    if ( !tbsCheckIp(pszValue) )
    {
        printf("Invalid ip\n");
        return ERR_IP_INVALID_FORMAT;
    }

    ulIp = tbsCnvtIpFromStr(pszValue);
    /*
      IP��Χ: 1.*.*.*   -  127.*.*.*,
              128.*.*.* -  224.*.*.*
    */
    if ((ulIp > 0x01000000 && ulIp < 0x7f000000)
        || (ulIp > 0x80000000 && ulIp < 0xe0000000) )
    {
        /* ����Ϊ*.*.*.0 ���� *.*.*.255 */
        if ( 0 != (ulIp&0x000000ff)
            && 0xff != (ulIp&0x000000ff) )
            return TBS_SUCCESS;
        else
            return ERR_IP_IS_CAST_OR_NET_ADDR;
    }
    else
        return ERR_IP_IS_WRONG_NET_SECTION;
}

/*
���DNS IP�Ϸ��ԣ�������Ϊ�鲥�㲥�������ַ,����0.0.0.0��IP����
*/
int tbsCheckDNSHostIpEx(const char *pszValue)
{
    unsigned long ulIp = 0;

    if ( NULL == pszValue || 0 == strlen(pszValue))
    {
        return ERR_CAN_NOT_EMPTY;
    }

    if (!safe_strcmp(pszValue, "0.0.0.0"))
    {
        return TBS_SUCCESS;
    }

    if ( !tbsCheckIp(pszValue) )
    {
        printf("Invalid ip");
        return ERR_IP_INVALID_FORMAT;
    }

    ulIp = tbsCnvtIpFromStr(pszValue);
    /*
      IP��Χ: 1.*.*.*   -  127.*.*.*,
              128.*.*.* -  224.*.*.*
    */
    if ((ulIp > 0x01000000 && ulIp < 0x7f000000)
        || (ulIp > 0x80000000 && ulIp < 0xe0000000) )
    {
        /* ����Ϊ*.*.*.0 ���� *.*.*.255 */
        if ( 0 != (ulIp&0x000000ff)
            && 0xff != (ulIp&0x000000ff) )
            return TBS_SUCCESS;
        else
            return ERR_IP_IS_CAST_OR_NET_ADDR;
    }
    else
        return ERR_IP_IS_WRONG_NET_SECTION;
}


int tbsCheckPortEx(const char *pszString)
{
        char *pszPattern = "^[1-9][0-9]\{0,4}$";

    if (TRUE != tbsMatch(pszString, pszPattern))
    {
    	return ERR_UINT_INVALID_VALUE;
    }

    if (atoi(pszString) < 1 || atoi(pszString) > 65535)
    {
    	return ERR_OUT_OF_RANGE;
    }

    return TBS_SUCCESS;
}

int  tbsCheckPriorityEx(const char *pszString)
{
	if (tbsCheckIntRangeEx(pszString, 1, 8))
		return ERR_PRIORITY_INVALID_VALUE;

	return TBS_SUCCESS;
}


int tbsCheckMaskEx(const char *pcValue)
{
	unsigned long ulMask = 0;

	if ( pcValue == NULL || strlen(pcValue) == 0 )
	{
		return ERR_CAN_NOT_EMPTY;
	}

	if(FALSE == tbsCheckIp(pcValue))
	{
		return ERR_MASK_INVALID_FORMAT;
	}

	ulMask = tbsCnvtIpFromStr(pcValue);
	if ( ulMask == 0 )
	{
		return ERR_MAKS_ALL_ZERO;
	}

#ifdef _TBS_ENV_PC
    ulMask = htonl(ulMask);
#endif

	while ( ulMask & 0x80000000 )
	{
		ulMask <<= 1;
	}

	ulMask &= 0xffffffff;
	if ( ulMask == 0 )
	{
		return TBS_SUCCESS;
	}
	else
	{
		return ERR_MASK_INVALID_VALUE;
	}
}



int tbsCheckEnableEx(const char *pszValue)
{
    if ( NULL == pszValue )
    {
        return ERR_ENABLE_EMPTY;
    }

    if ( 0 == strcasecmp(pszValue, "1") || 0 == strcasecmp(pszValue, "true") ||
          0 == strcasecmp(pszValue, "0") || 0 == strcasecmp(pszValue, "false") )
    {
        return TBS_SUCCESS;
    }
    else
    {
        return ERR_ENABLE_INVALID_VALUE;
    }
}



/*********************************************************************
*  ����: ���Э�������Ƿ�Ϸ�
*  ����: pszValue, Э���ַ���
*  ����: const char *
*  ��ע: �Ϸ�ֵΪIPv4, IPv6, IPv4&6
*********************************************************************/
 int tbsCheckProtoType(const char * pszValue)
{
    if (NULL == pszValue)
    {
        return TBS_NULL_PTR;
    }

    if (!strcasecmp(pszValue, STR_PROTO_TYPE_IPV4) ||
        !strcasecmp(pszValue, STR_PROTO_TYPE_IPV6) ||
        !strcasecmp(pszValue, STR_PROTO_TYPE_IPV4_6))
    {
        return TBS_SUCCESS;
    }

    return ERR_INVALID_PROTO_TYPE;
}

int tbsCheckIntEx(const char *pszValue)
{
    int iValue = 0;

    if ( NULL == pszValue )
    {
        return ERR_INT_INVALID_VALUE;
    }

    if(1 == sscanf(pszValue, "%d", &iValue))
        return TBS_SUCCESS;
    else
        return ERR_INT_INVALID_VALUE;
}


int tbsCheckUIntEx(const char *pszValue)
{
	if ( NULL == pszValue )
	{
		return ERR_UINT_INVALID_VALUE;
	}

    if (!tbsMatch(pszValue, "^(0|[1-9][0-9]{0,9})$"))
    {
        return ERR_UINT_INVALID_VALUE;
    }

    return TBS_SUCCESS;
}


int tbsCheckIntRangeEx(const char *pszValue, int nMin, int nMax)
{
    int iValue = 0;

    if ( RET_FAILED(tbsCheckIntEx(pszValue)) )
        return ERR_INT_INVALID_VALUE;

    sscanf(pszValue, "%d", &iValue);
    if ( iValue > nMax || iValue <nMin )
        return ERR_OUT_OF_RANGE;
    else
        return TBS_SUCCESS;
}

int tbsCheckUIntRangeEx(const char *pszValue, unsigned int nMin, unsigned int nMax)
{
    unsigned int nValue = 0;

	if (RET_FAILED(tbsCheckUIntEx(pszValue)) )
		return ERR_UINT_INVALID_VALUE;

	sscanf(pszValue, "%u", &nValue);
	if ( nValue > nMax || nValue <nMin )
		return ERR_OUT_OF_RANGE;
	else
		return TBS_SUCCESS;
}



int  tbsCheckMacEx(const char *pcMac)
{
    int arr[6], index;
    const char *pattern = "^[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}$";

    if(pcMac == NULL)
    return ERR_MAC_INVALID_VALUE;

    if(FALSE == tbsMatch(pcMac, pattern))
    {
    	return ERR_MAC_INVALID_VALUE;
    }

    if(sscanf(pcMac,"%x:%x:%x:%x:%x:%x",\
           &arr[0],&arr[1],&arr[2],&arr[3],&arr[4],&arr[5]) != 6 )
    return ERR_MAC_INVALID_VALUE;

    /* ���MAC��ַ�Ƿ�Ϊ�㲥����ȫ��MAC */

   if (0 == strcasecmp(pcMac, "00:00:00:00:00:00")
         || 0 == strcasecmp(pcMac, "FF:FF:FF:FF:FF:FF")
    )
    {
        return ERR_MAC_INVALID_VALUE;
    }

    /*���MAC��ַ�Ƿ�Ϊ�ಥMAC��ַ*/
    if( arr[0]%2 > 0 )
        return ERR_MAC_INVALID_VALUE;

    /*���MAC��ַ�Ƿ�Ϸ�*/
    for(index = 1; index < 6; index ++)
    {
    	if(arr[index] > 0xff)
    	{
    		return ERR_MAC_INVALID_VALUE;
    	}
    }
    return TBS_SUCCESS;
}


/*
 ip�б��ʽ:ip1,ip2
 ip�б��в������ظ�ip
*/
int tbsCheckIpListEx(const char *pszIpList, int iMaxCount)
{
    char szIPAddrs[MAX_IP_LIST_LEN] = {0};
    char *pszIP[MAX_IP_COUNT] = {NULL};
    int iCount = 0;
    int i = 0;
    int j = 0;

    if ( NULL == pszIpList || 0 == strlen(pszIpList))
    {
        return ERR_CAN_NOT_EMPTY;
    }

    safe_strncpy(szIPAddrs, pszIpList, MAX_IP_LIST_LEN);
    iCount = tbsSplitString(szIPAddrs, pszIP, ',', iMaxCount);
    if ( iCount > iMaxCount )
    {
        printf("Error: ip_addr count error\n");
        return ERR_IPLIST_TOO_MANY;
    }

    for (i = 0; i < iCount; i++)
    {
        if ( !pszIP[i] || !tbsCheckIp(pszIP[i]) )
        {
            return ERR_IPLIST_INVALID_FORMAT;
        }

        /* ����ظ� */
        for ( j = i+1; j < iCount; j++ )
        {
            if ( strcmp(pszIP[i], pszIP[j]) == 0 )
                return ERR_LIST_HAS_REPEAT;
        }
    }

    return TBS_SUCCESS;
}

/*
 ip�б��ʽ:ip1,ip2
 ip�б��е�ip������hostip(����Ϊ�㲥��ַ,�鲥��ַ�������Ƿ�ip)
 ip�б��в������ظ�ip
*/

int tbsCheckHostIpListEx(const char *pszIpList, int iMaxCount)
{
    char szIPAddrs[MAX_IP_LIST_LEN] = {0};
    char *pszIP[MAX_IP_COUNT] = {NULL};
    int iCount = 0;
    int i = 0;
    int j = 0;

    if ( NULL == pszIpList || 0 == strlen(pszIpList))
    {
        return ERR_CAN_NOT_EMPTY;
    }

    safe_strncpy(szIPAddrs, pszIpList, MAX_IP_LIST_LEN);
    iCount = tbsSplitString(szIPAddrs, pszIP, ',', iMaxCount);
    if ( iCount > iMaxCount )
    {
        printf("Error: ip_addr count error\n");
        return ERR_IPLIST_TOO_MANY;
    }

    for (i = 0; i < iCount; i++)
    {
        if ( !pszIP[i]
            || RET_FAILED(tbsCheckHostIpEx(pszIP[i])) )
        {
            return ERR_IPLIST_INVALID_FORMAT;
        }

        /* ����ظ� */
        for ( j = i+1; j < iCount; j++ )
        {
            if ( strcmp(pszIP[i], pszIP[j]) == 0 )
                return ERR_LIST_HAS_REPEAT;
        }
    }

    return TBS_SUCCESS;
}

/*
 ip�б��ʽ:ip1,ip2
 ip�б��е�ip������hostip(����Ϊ�㲥��ַ,�鲥��ַ�������Ƿ�ip)
 ip�б��в������ظ�ip,�����ж�0.0.0.0��IP,��������IP����
*/
int tbsCheckDNSHostIpListEx(const char *pszIpList, int iMaxCount)
{
    char szIPAddrs[MAX_IP_LIST_LEN] = {0};
    char *pszIP[MAX_IP_COUNT] = {NULL};
    int iCount = 0;
    int i = 0;
    int j = 0;

    if ( NULL == pszIpList || 0 == strlen(pszIpList))
    {
        return ERR_CAN_NOT_EMPTY;
    }

    safe_strncpy(szIPAddrs, pszIpList, MAX_IP_LIST_LEN);
    iCount = tbsSplitString(szIPAddrs, pszIP, ',', iMaxCount);
    if ( iCount > iMaxCount )
    {
        printf("Error: ip_addr count error\n");
        return ERR_IPLIST_TOO_MANY;
    }

    for (i = 0; i < iCount; i++)
    {
        if ( !pszIP[i]
            || RET_FAILED(tbsCheckDNSHostIpEx(pszIP[i])) )
        {
            return ERR_IPLIST_INVALID_FORMAT;
        }

        /* ����ظ� */
        for ( j = i+1; j < iCount; j++ )
        {
            if ( strcmp(pszIP[i], pszIP[j]) == 0 )
                return ERR_LIST_HAS_REPEAT;
        }
    }

    return TBS_SUCCESS;
}

int Check2IpIsInSameNet(const char *pcIp1,const char *pcIp2,const char *mask)
{
	if((tbsCnvtIpFromStr(pcIp1) & tbsCnvtIpFromStr(mask))!=(tbsCnvtIpFromStr(pcIp2) & tbsCnvtIpFromStr(mask)))
		return 0;
	else
		return 1;
}

/******************************************************************************
*
* Function    : tbsSyncNetIDWithLanIP
* Description : ?oldIPStr??????????lanIPStr???????,
*               ?????????
* Parameters  : newIPStr: ?????IP???????
* Return      :
* Author      : lixiaoguo/ 2011-08-04
* History     :
*******************************************************************************/
void tbsSyncNetIDWithLanIP(char *lanIPStr, char *netMaskStr, char *oldIPStr,
char *newIPStr)
{
	unsigned long lanIP, netMask, oldIP, newIP, netID, hostID;

	lanIP = tbsCnvtIpFromStr(lanIPStr);
	netMask = tbsCnvtIpFromStr(netMaskStr);
	oldIP = tbsCnvtIpFromStr(oldIPStr);

	netID = lanIP & netMask & 0xFFFFFFFF;
	hostID = ((~netMask) & oldIP) & 0xFFFFFFFF;

	newIP = (netID | hostID) & 0xFFFFFFFF;

	tbsCnvtIpToStr(newIP, newIPStr);
}

int tbsCheckMacListEx(const char *pszMacList, int iMaxCount)
{
    char szMacList[MAX_MAC_LIST_LEN] = {0};
    char *apszMac[MAX_MAC_COUNT] = {NULL};
    int iCount = 0;
    int i = 0;
    int j = 0;

    if ( NULL == pszMacList )
    {
        return ERR_MACLIST_INVALID_FORMAT;
    }

    if ( 0 == strlen(pszMacList) )
    {
        return ERR_MACLIST_INVALID_FORMAT;
    }

    safe_strncpy(szMacList, pszMacList, MAX_MAC_LIST_LEN);
    iCount = tbsSplitString(szMacList, apszMac, ',', iMaxCount);
    if ( iCount > iMaxCount )
    {
        printf("Error: ip_addr count error\n");
        return ERR_MACLIST_TOO_MANY;
    }

    for (i = 0; i < iCount; i++)
    {
        if ( !apszMac[i] || !tbsCheckMac(apszMac[i]) )
        {
            return ERR_MACLIST_INVALID_FORMAT;
        }

        /* ����ظ� */
        for ( j = i+1; j < iCount; j++ )
        {
            if ( strcmp(apszMac[i], apszMac[j]) == 0 )
                return ERR_LIST_HAS_REPEAT;
        }
    }

    return TBS_SUCCESS;
}

/* ����û���/���� */
int tbsCheckUserEx(const char *pszValue)
{
    char pattern[MAX_USER_LEN] = { 0 };
    char bufferValue[MAX_USER_LEN] = {'\0'};
	char* tmp = NULL;
	
	if (pszValue == NULL)
    {
        return ERR_USERNAME;
    }
	
	strncpy( bufferValue , pszValue, MAX_USER_LEN );
	
	while( (tmp = strstr( bufferValue , "%" )) != NULL )
	{
		*tmp = '\0'; 
		tmp++;
		if ( *tmp != '\0' )
		    strcat(bufferValue, tmp);
	}
		
    sprintf(pattern, "^[0-9a-zA-Z\\.@_$!#-]{1,%d}", MAX_USER_LEN);
   
    if (FALSE == tbsMatch(bufferValue, pattern))
    {
        return ERR_USERNAME;
    }

    return TBS_SUCCESS;
}

int tbsCheckPasswdEx(const char *pszValue)
{
    if (pszValue == NULL)
    {
        return ERR_PASSWORD;
    }

    if (tbsCheckUserEx(pszValue) != TBS_SUCCESS)
    {
        return ERR_PASSWORD;
    }

    return TBS_SUCCESS;
}

int tbsCheckDomainName(const char *pszValue)
{
    if (pszValue == NULL)
    {
        return TBS_NULL_PTR;
    }

	/* ����Ϊ�� */
	if (0 == strlen(pszValue))
	{
		return TBS_SUCCESS;
	}
	
    if (!tbsMatch(pszValue, "^([[:alnum:]]?)([[:alnum:]-]+[.]?){0,5}[[:alpha:]]+$")
        || strlen(pszValue) > 256 )
    {
        return ERR_INVALID_DOMAIN_NAME;
    }

    return TBS_SUCCESS;
}
int tbsCheckHostName(const char *pszValue)
{
    if (pszValue == NULL)
    {
        return ERR_INVALID_HOSTNAME_NAME;
    }
	/* ����Ϊ�� */
	if (0 == strlen(pszValue))
	{
		return ERR_INVALID_HOSTNAME_NAME;
	}
	/* �����������������ַ�*/
	 if (!tbsMatch(pszValue, "^([[:alnum:]-]+)$")|| strlen(pszValue) > 67)
	//if ( strlen(pszValue) > 67)
	{
		return ERR_INVALID_HOSTNAME_NAME;
	}
	

    return TBS_SUCCESS;
}

/* �����������IP�ĺϷ��� */
int tbsCheckHost(const char *pszValue)
{
    BOOL bIsIP = TRUE;

    if (pszValue == NULL)
    {
        return TBS_NULL_PTR;
    }

    /*
      ���ܽ��Կ�ͷ�Ƿ����������ж��Ƿ�ΪIP����Ӧ
	 ��ȫ�������ֺ�'.'��ɣ���Ϊ��IP��ַ����ֹ����
	 163.com֮���������������Ϊ��IP��ַ��
	*/
    if (!tbsMatch(pszValue, "^([0-9]{1,3}\\.){3}[0-9]{1,3}$"))
        bIsIP = FALSE;

    /* ��ip��ַ */
	if ( bIsIP )
    {
		return tbsCheckHostIpEx(pszValue);
	}
    else
    {
		return tbsCheckDomainName(pszValue);
	}
}

void TW_printf(const void *buf, int len)
{
    const int LINE_WIDTH = 16;
    int i = 0, j = 0;
    int tail = 0;
    const unsigned char *pucBuf = NULL;

    if(len == 0 || NULL == buf) {
        return;
    }
    pucBuf = buf;
    for(; i<len; i++) {
        if(i>0 && i%LINE_WIDTH == 0) {
            j = i - LINE_WIDTH;
            printf("; ");
            for(; j < i; j++) {
                if((pucBuf[j] > 31) && (pucBuf[j] < 127)) {
	                printf("%c", pucBuf[j]);
                } else {
                    printf("%c", '.');
                }
            }
            printf("\x0a\x0d");
        }
        printf("%02X ", pucBuf[i]);
    }
    tail = len%LINE_WIDTH == 0 ? len-LINE_WIDTH:(len/LINE_WIDTH)*LINE_WIDTH;
    if(tail != len-LINE_WIDTH) {
        for(i=0; i<48-(len-tail)*3; i++) {
            printf("%c", ' ');
        }
    }
    printf("; ");
    for(i=tail; i<len; i++) {
        if((pucBuf[j] > 31) && (pucBuf[j] < 127)) {
            printf("%c", pucBuf[i]);
        } else {
            printf("%c", '.');
        }
    }
    printf("\x0a\x0d");
}

/*=========================================================================
*
*    Mem Pool function
*
*=========================================================================*/
/* �ڴ�����¼ */
typedef struct __ST_Mem_Rec
{
    struct list_head head;
    void *pAddr;
    unsigned int nSize;
    const char *szFile;
    const char *szFunc;
    unsigned long ulLine;
    unsigned int bPrinted;
}ST_Mem_Rec;

/* ȡ���ڴ�⺯�����ض��� */
#ifdef malloc
#undef malloc
#endif

#ifdef free
#undef free
#endif

#ifdef calloc
#undef calloc
#endif

#ifdef realloc
#undef realloc
#endif


#ifdef strdup
#undef strdup
#endif



/* ��ȫ�ͷ��ڴ�*/
#define safe_mem_free(pointer) \
    { \
        if ( pointer ) \
        { \
            free(pointer); \
            pointer = NULL; \
        } \
    }

static void MemList_AddTail(struct list_head *head, ST_Mem_Rec *pstMem)
{
    ST_Mem_Rec *l = (ST_Mem_Rec*)malloc(sizeof(ST_Mem_Rec));
    memcpy(l, pstMem, sizeof(ST_Mem_Rec));

    list_add_tail(&l->head, head);
}


void MemList_SetEntry(struct list_head *head, void *pAddr, ST_Mem_Rec *pstMem)
{
    ST_Mem_Rec *l = NULL;
    struct list_head *ptr;

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Mem_Rec, head);
        if ( l->pAddr == pAddr )
        {
            l->pAddr = pstMem->pAddr;
            l->nSize = pstMem->nSize;
            l->szFile = pstMem->szFile;
            l->szFunc = pstMem->szFunc;
            l->ulLine = pstMem->ulLine;
            l->bPrinted= FALSE;
            return;
        }
    }

    MemList_AddTail(head, pstMem);
}


#define MEM_MALLOC_MAGIC    0x30    /* 0 */
#define MEM_FREE_MAGIC      0x39    /* 9 */
#define MEM_EXCCED_MAGIC    0x7a    /* z */
#define MEM_EXCCED_LEN      16


#define MEM_EXCCED_CHECK(l) \
{ \
    int i = 0; \
    for (i = 0; i < MEM_EXCCED_LEN; i++) \
    { \
        if (MEM_EXCCED_MAGIC != *((unsigned char *)(l->pAddr + l->nSize + i))) \
        { \
            printf("\n_   _ \n" \
                     "@   @ \n" \
                     "  )   \n" \
                     "  o  !!!!!!==== Mem Exceed at %s %lu ====!!!!!!\n", \
                     l->szFile, l->ulLine); \
            exit(-1); \
        } \
    } \
}

static BOOL MemList_DeleteEntry(struct list_head *head, void *pAddr)
{
    ST_Mem_Rec *l = NULL;
    struct list_head *ptr;

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Mem_Rec, head);
        if ( l->pAddr == pAddr )
        {
            MEM_EXCCED_CHECK(l);
            memset(l->pAddr, MEM_FREE_MAGIC, l->nSize);
            safe_mem_free(l->pAddr);
            list_del(&l->head);
            free(l);
            return TRUE;
        }
    }
    return FALSE;
}

static void MemList_DropAll(struct list_head *head)
{
    ST_Mem_Rec  *l = NULL;

    while (!list_empty(head))
    {
        l = list_entry(head->next, ST_Mem_Rec , head);
        safe_mem_free(l->pAddr);
        list_del(&l->head);
        free(l);
    }
}

static void MemList_PrintAll(struct list_head *head, BOOL bOnlyPrintNew)
{
    struct list_head *ptr;
    ST_Mem_Rec *l;
    int total_mem_count = 0;
    int total_size = 0;

    printf("===========================================================================\n");
    printf("Memory Pool Status\n");
    printf("\n");

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Mem_Rec, head);
        if(l)
        {
            total_mem_count++;
            total_size += l->nSize;

            MEM_EXCCED_CHECK(l);

            /* �Ƿ�ֻ��ӡ�·�������� */
            if ( bOnlyPrintNew )
            {
                if ( !(l->bPrinted) )
                {
                    printf("Mem Rec %4d/%p: Address=%p, Size=%u, Malloc at %s:%s:%4lu  \n",
                        total_mem_count, l, l->pAddr, l->nSize, l->szFile, l->szFunc, l->ulLine);
                    l->bPrinted = TRUE;
                }
            }
            else
                printf("Mem Rec %4d/%p:: Address=%p, Size=%u, Malloc at %s:%s:%4lu  \n",
                    total_mem_count, l, l->pAddr, l->nSize, l->szFile, l->szFunc, l->ulLine);
        }
    }
    printf("===========================================================================\n");
    printf("Total memory blocks = %d, Total memory size = %d bytes. \n", total_mem_count, total_size);
}


/* �ڴ������ͷ */
static struct list_head g_stMemPool = LIST_HEAD_INIT(g_stMemPool);


/*********************************************************************
*  ����: Mem_Malloc
*  ����: �����ڴ�,����¼
*  ����: nSize          : Ҫ�����ڴ�Ĵ�С
*        szFile         : �ļ���
*        szFunc         : ������
*        szLine         : �к�
*  ����: �ɹ�,������ڴ��ָ��
*        ʧ��,NULL
*  ����: ������ / 2008-10-15
*********************************************************************/
void *Mem_Malloc(size_t nSize, const char *szFile, const char *szFunc, unsigned long ulLine)
{
    void *pAddr = NULL;
    ST_Mem_Rec stMem;

    pAddr = malloc(nSize + MEM_EXCCED_LEN);
    if ( NULL == pAddr )
    {
        Mem_PrintStatus();
        return NULL;
    }

    /* ����ħ������ */
    memset(pAddr, MEM_MALLOC_MAGIC, nSize);
    memset(pAddr + nSize, MEM_EXCCED_MAGIC, MEM_EXCCED_LEN);

    memset(&stMem, 0, sizeof(ST_Mem_Rec));
    stMem.pAddr = pAddr;
    stMem.nSize = nSize;
    stMem.szFile = szFile;
    stMem.szFunc = szFunc;
    stMem.ulLine = ulLine;
    stMem.bPrinted= FALSE;

    MemList_AddTail(&g_stMemPool, &stMem);

    return pAddr;
}

char *Mem_Strdup(const char *pcStr, const char *szFile, const char *szFunc, unsigned long ulLine)
{
    size_t nSize = 0;
    char *pcNew = NULL;

    if (NULL == pcStr)
    {
        return NULL;
    }

    nSize = strlen(pcStr) + 1;
    pcNew = Mem_Malloc(nSize, szFile, szFunc, ulLine);
    if (NULL != pcNew)
    {
        strcpy(pcNew, pcStr);
    }

    return pcNew;
}

extern size_t strnlen (const char *str, size_t len);

char *Mem_Strndup(const char *pcStr, size_t nSize, const char *szFile, const char *szFunc, unsigned long ulLine)
{
    char *pcNew = NULL;

    if (NULL == pcStr)
    {
        return NULL;
    }

    nSize = strnlen(pcStr, nSize) + 1;
    pcNew = Mem_Malloc(nSize, szFile, szFunc, ulLine);
    if (NULL != pcNew)
    {
        strlcpy(pcNew, pcStr, nSize);
    }

    return pcNew;
}



/*********************************************************************
*  ����: Mem_Calloc
*  ����: Ϊ��������ڴ�(���������ڴ���0),����¼
*  ����: nMemb          : Ҫ�����ڴ�Ŀ���
*        nSize          : Ҫ�����ÿ���ڴ�Ĵ�С
*        szFile         : �ļ���
*        szFunc         : ������
*        szLine         : �к�
*  ����: �ɹ�,������ڴ��ָ��
*        ʧ��,NULL
*  ����: ������ / 2008-10-15
*********************************************************************/
void *Mem_Calloc(size_t nMemb, size_t nSize, const char *szFile, const char *szFunc, unsigned long ulLine)
{
    void *pAddr = NULL;
    ST_Mem_Rec stMem;
    unsigned long ulLen = nMemb * nSize;

    pAddr = malloc(ulLen + MEM_EXCCED_LEN);
    if ( NULL == pAddr )
    {
        Mem_PrintStatus();
        return NULL;
    }
    memset(pAddr, 0, ulLen);

    /* ����ħ������ */
    memset(pAddr + ulLen, MEM_EXCCED_MAGIC, MEM_EXCCED_LEN);

    memset(&stMem, 0, sizeof(ST_Mem_Rec));
    stMem.pAddr = pAddr;
    stMem.nSize = nMemb * nSize;
    stMem.szFile = szFile;
    stMem.szFunc = szFunc;
    stMem.ulLine = ulLine;

    MemList_AddTail(&g_stMemPool, &stMem);

    return pAddr;
}


/*********************************************************************
*  ����: Mem_Realloc
*  ����: ���·����ڴ�( �ͷ�/��չԭ��������ڴ�),����¼
*  ����: pAddr          : ԭ�����ڴ�ָ��
*        nSize          : �·�����ڴ�Ĵ�С
*        szFile         : �ļ���
*        szFunc         : ������
*        szLine         : �к�
*  ����: �ɹ�,������ڴ��ָ��
*        ʧ��,NULL
*  ����: ������ / 2008-10-15
*********************************************************************/
void *Mem_Realloc(void *pAddr, size_t nSize, const char *szFile, const char *szFunc, unsigned long ulLine)
{
    void *pNewAddr = NULL;
    ST_Mem_Rec stMem;

    if (pAddr == NULL)
    {
        pNewAddr = Mem_Malloc(nSize, szFile, szFunc, ulLine);
        return pNewAddr;
    }

    if ( nSize == 0 )
    {
        Mem_Free(pAddr, szFile, szFunc, ulLine);
        return NULL;
    }

    pNewAddr = realloc(pAddr, nSize + MEM_EXCCED_LEN);
    if ( NULL == pNewAddr )
    {
        Mem_PrintStatus();
        return NULL;
    }

    /* ����ħ������ */
    //memset(pNewAddr, MEM_MALLOC_MAGIC, nSize);
    memset(pNewAddr + nSize, MEM_EXCCED_MAGIC, MEM_EXCCED_LEN);

    memset(&stMem, 0, sizeof(ST_Mem_Rec));
    stMem.pAddr = pNewAddr;
    stMem.nSize = nSize;
    stMem.szFile = szFile;
    stMem.szFunc = szFunc;
    stMem.ulLine = ulLine;
    MemList_SetEntry(&g_stMemPool, pAddr, &stMem);

    return pNewAddr;
}

/*********************************************************************
*  ����: Mem_Free
*  ����: �ͷ��ڴ�,��ɾ����¼
*  ����: pAddr          : Ҫ�ͷŵ��ڴ�ָ��
*        szFile         : �ļ���
*        szFunc         : ������
*        szLine         : �к�
*  ����: void
*  ����: ������ / 2008-10-15
*********************************************************************/
void Mem_Free(void *pAddr, const char *szFile, const char *szFunc, unsigned long ulLine)
{
    if ( NULL != pAddr)
    {
        if( !MemList_DeleteEntry(&g_stMemPool, pAddr) )
        {
            printf("[MEM]Free wrong memory: Address=%p, Free at %s:%s:%4lu \n",
                pAddr, szFile, szFunc, ulLine);
            free(pAddr);
        }
    }
}

/*********************************************************************
*  ����: Mem_FreeAll
*  ����: �ͷ������ڴ�, ��ɾ�����м�¼
*  ����: void
*  ����: void
*  ����: ������ / 2008-10-15
*********************************************************************/
void Mem_FreeAll(void)
{
    MemList_PrintAll(&g_stMemPool, FALSE );
    MemList_DropAll(&g_stMemPool );
}

/*********************************************************************
*  ����: Mem_PrintStatus
*  ����: ��ʾ����֮����δ�ͷŵ��ڴ����
*  ����: void
*  ����: void
*  ����: ������ / 2008-10-15
*********************************************************************/
void Mem_PrintStatus(void)
{
#ifdef CONFIG_TBS_APPS_DEBUG_MALLOC
    MemList_PrintAll(&g_stMemPool, FALSE);
#endif
}

/*********************************************************************
*  ����: Mem_PrintStatusOnce
*  ����: ��ʾ����֮����δ�ͷŵ��ڴ����(�Ѿ����ô˺�����ʾ�����ڴ�鲻����ʾ)
*  ����: void
*  ����: void
*  ����: ������ / 2008-10-15
*********************************************************************/
void Mem_PrintStatusOnce(void)
{
#ifdef CONFIG_TBS_APPS_DEBUG_MALLOC
    MemList_PrintAll(&g_stMemPool, TRUE );
#endif
}

/*********************************************************************
*  ����: tbsAddMac

*  ����: ����mac��ַ��֧��LAN��WAN�豸���޸�

*  ����: pIfNameΪ�豸��,��eth0
		 iInterfaceTypeΪMAC_LAN_DEVICE����MAC_WAN_DEVICE
		 szMacVal  ���ظ������ߵ�mac��ַ, �ڴ��ɵ���������

*  ����: TBS_SUCCESS/TBS_FAILED

*  ����: zhujianwen / 2008-12-25
*********************************************************************/
int tbsAddMac(const char *pIfName, int iInterfaceType, char *szMacVal)
{
    char szMacLow[MAX_MAC_ADDR_LEN] = {0}, szMacHigh[MAX_MAC_ADDR_LEN] = {0};
    char szCmd[SYS_CMD_LEN] = {0};
    unsigned long ulMacLow;
    int iMacOffSet;
    FILE *fp;
    int i;

    strcpy(szMacLow, "0x");
    /* ��ȡflash��mac��ַ */
    if((fp = fopen(PATH_MAC_ADDRESS, "r")) == NULL)
    {
        printf("Error: Fail to open %s \r\n", PATH_MAC_ADDRESS);
        return TBS_FAILED;
    }
    /* ƫ��2����Ϊ����0x */
    if(fgets(szMacHigh, MAX_MAC_ADDR_LEN, fp) == 0)
    {
        printf("Error: Fail to get data from FILE \r\n");
        fclose(fp);
        return TBS_FAILED;
    }
    fclose(fp);

    /* ���ݽӿ�����ѡ��ͬ��ƫ���� */
    if(iInterfaceType == MAC_LAN_DEVICE)
    {
        iMacOffSet = CONFIG_LAN_MAC_ADDRESS_OFFSET;
    }
    else if(iInterfaceType == MAC_WAN_DEVICE)
    {
        iMacOffSet = CONFIG_WAN_MAC_ADDRESS_OFFSET;
    }
    else if(iInterfaceType == MAC_WLAN_DEVICE)
    {
        iMacOffSet = CONFIG_WLAN_MAC_ADDRESS_OFFSET;
    }
    else if(iInterfaceType == MAC_USB_DEVICE)
    {
        iMacOffSet = CONFIG_USB_MAC_ADDRESS_OFFSET;
    }
    else
    {
        printf("Error: parament pass wrong: iInterfaceType\r\n");
        return TBS_FAILED;
    }

    /* ����mac��ַ��Ӧ��ʮ������������,�ֿ��ߵ������� */
    strcpy(szMacLow+2, szMacHigh+6);
    memset(szMacHigh+6, 0, MAX_MAC_ADDR_LEN-6);

    /* �ֱ�ȡ�ߵ�������,ת��Ϊʮ�������� */
    ulMacLow = strtol(szMacLow, NULL, 16);

    /* �������ƫ����������Χ, ��λ���� */
    if((ulMacLow+iMacOffSet) > MAX_VALUE_OF_HEX)
    {
            ulMacLow = ulMacLow + iMacOffSet - CYCLE_VALUE_OF_HEX;
    }
    else
    {
            ulMacLow += iMacOffSet;
    }

    /* ��' '��Ϊ0 */
    memset(szMacLow, 0, MAX_MAC_ADDR_LEN);
    sprintf(szMacLow, "%6lx", ulMacLow);
    for(i=0; i<6; i++)
    {
        if(szMacLow[i] == ' ')
            szMacLow[i] = '0';
    }

	memset(szMacVal, 0, MAX_MAC_ADDR_LEN);
	snprintf(szMacVal, MAX_MAC_ADDR_LEN, "%s%s", szMacHigh, szMacLow);
    /* дmac��ַ�������� */
    snprintf(szCmd, SYS_CMD_LEN,
             "ifconfig %s down;ifconfig %s hw ether %s%s;ifconfig %s up",
             pIfName, pIfName, szMacHigh, szMacLow, pIfName);

    return tbsSystem(szCmd, TBS_PRINT_CMD);
}

/* DNS������ص�API���ṹ��,��Ķ��� */
#define REG_MATCH_LEN  5

/*********************************************************************
 *  ����: tbsSplitUrl

 *  ����: �������������, �����������Ҫ�����Ĳ����Լ��˿ں�, �����
 IP��ַ��ֱ�ӷ���

 *  ����:
    pHostName:	��Ҫ����������
    pPort:		�˿ں�
    pPath:		·��

 *  ����:
    SPLIT_SUCCESS: �����ɹ�, ��ʱpHostName��ŵ��ǽ������������ַ
    pPort��ŵ��Ƕ˿ں�
    SPLIT_ERROR:	����ʧ��
    SPLIT_IP:		�����URL��IP��ַ, ֱ�ӷ���

 *  ����: lichuang (2009-04-01)
 *********************************************************************/
int tbsSplitUrl(const char *pUrl, char *pHostname, char *pPort, char *pPath)
{
	size_t          nLen = 0;
	regex_t         stRegex;             /* �洢����õ�������ʽ��������ʽ��ʹ��֮ǰҪ�������� */
	regmatch_t      szRegMatch [REG_MATCH_LEN];     /* �洢ƥ�䵽���ַ���λ�� */
	int             nResult = 0;
    char            szHost[256] = {0};

    /* szUrlPattern�ַ��� */
	char szUrlPattern[] = "^((http|https)://)?([^:/]*)(:[0-9]+)?(.*)";

#ifdef CONFIG_TBS_SUPPORT_IPV6
    char szIp6Pattern[] = "^((http|https)://)?\\[([^]]*)\\](:[0-9]+)?(.*)";
    int isIPv6 = 0;

    /* ipv6 ��ʽ��url��ַ */
    if (strstr(pUrl, "http://[")
        || strstr(pUrl, "https://[")
        || (pUrl[0] == '[' && strchr(pUrl, ']'))
    )
    {
        isIPv6 = 1;
        /* ����������ʽ */
    	if (0 != regcomp(&stRegex, szIp6Pattern, REG_EXTENDED))
    	{
    		return SPLIT_ERROR;
    	}
    }
    else
#endif
    {
    	/* ����������ʽ */
    	if (0 != regcomp(&stRegex, szUrlPattern, REG_EXTENDED))
    	{
    		return SPLIT_ERROR;
    	}
    }

	/* ִ��ģʽƥ�� */
	if (0 != regexec(&stRegex, pUrl, (size_t) REG_MATCH_LEN, szRegMatch, 0))
	{
		regfree(&stRegex);
		return SPLIT_ERROR;
	}
	nResult = SPLIT_SUCCESS;

	/* ���ȵõ��˿ں�, ����szUrlPattern�еĵ�4���ӱ��ʽ, ���������4��ʼ */
	if (-1 != szRegMatch[4].rm_so)
	{
		nLen = szRegMatch[4].rm_eo - szRegMatch[4].rm_so;
		if (nLen > MAX_PORT_LEN)
		{
			regfree(&stRegex);
			return SPLIT_ERROR;
		}

        if (pPort)
		{
		    strncpy(pPort, pUrl + szRegMatch[4].rm_so + 1, nLen - 1);
		}
	}
    else
    {
        if (strstr(pUrl, "https://"))
        {
            if (pPort)
                strcpy(pPort, "443");
        }
        else
        {
            if (pPort)
                strcpy(pPort, "80");
        }
    }

	/* ��εõ�����, ����szUrlPattern�еĵ�3���ӱ��ʽ, ���������3��ʼ */
	if (-1 != szRegMatch[3].rm_so)
	{
		nLen = szRegMatch[3].rm_eo - szRegMatch[3].rm_so;
	    strncpy(szHost, pUrl + szRegMatch[3].rm_so, nLen);
	    szHost[nLen] = '\0';

#ifdef CONFIG_TBS_SUPPORT_IPV6
        /* ����ipv6 url��ʽ������ǲ��ǺϷ���ip��ַ */
        if (isIPv6)
        {
            if (TBS_SUCCESS == tbsCheckIpv6Addr(szHost))
                nResult = SPLIT_IP6;
            else
                nResult = SPLIT_ERROR;
        }
        else
#endif
        {
            /* �Ƚ����ǲ���IPv4��ַ��ʽ */
    		if (tbsCheckIp(szHost))
    			nResult = SPLIT_IP;
            else
                nResult = SPLIT_SUCCESS;
        }

        if (pHostname)
        {
            strcpy(pHostname, szHost);
        }
	}

    /* ���õ�·��,��5�����ʽ */
    if (-1 != szRegMatch[5].rm_so)
	{
        if (pPath)
        {
    		nLen = szRegMatch[5].rm_eo - szRegMatch[5].rm_so;
    	    strncpy(pPath, pUrl + szRegMatch[5].rm_so, nLen);
    	    pPath[nLen] = '\0';
        }
	}

	regfree(&stRegex);   /* �����˱������ͷ� */
	return nResult;
}

/*********************************************************************
 *  ����: SetDefaultDns

 *  ����: ����DNS����֮ǰָ�������õ�DNS��������ַ,��������øú���,
 ��ʹ��ϵͳĬ�ϵ�DNS������

 *  ����:
pServer: DNS��������ַ

 *  ����:

 *  ����: lichuang (2009-04-01)
 *********************************************************************/
static int SetDefaultDns(const char *pszServer)
{
    struct in_addr stAddr;

    char szCmd[256] = {0};
    system("cp /var/resolv.conf /var/resolv.conf_old");
    snprintf(szCmd, sizeof(szCmd), "echo nameserver %s>/etc/resolv.conf", pszServer);
    system(szCmd);

	if (inet_pton(AF_INET, pszServer, &stAddr) > 0)
    {
		_res.nscount = 1;
		_res.nsaddr_list[0].sin_addr = stAddr;

        return AF_INET;
	}

#ifdef CONFIG_TBS_SUPPORT_IPV6
    struct in6_addr stAddr6;
    if (inet_pton(AF_INET6, pszServer, &stAddr6) > 0)
    {
        struct sockaddr_in6 *sin6;

        sin6 = malloc(sizeof(struct sockaddr_in6));
        memset(sin6, 0, sizeof(struct sockaddr_in6));
        sin6->sin6_family = AF_INET6;
        sin6->sin6_port = htons(53);
        sin6->sin6_flowinfo = 0;
        sin6->sin6_addr = stAddr6;

        //_res.options |= RES_USE_INET6;
        _res._u._ext.nscount = 1;
        _res._u._ext.nsaddrs[0] = sin6;

        return AF_INET6;
    }
#endif

    return AF_UNSPEC;
}


static void RestoreOldDns(void)
{
    system("cp /var/resolv.conf_old /var/resolv.conf");
}


/*********************************************************************
 *  ����: __ResolveIpByName

 *  ����: �Դ�����������н���, �����������ST_DNS_INFO������

 *  ����:
pHostName: ��Ҫ����������

 *  ����: 0/-1

 *  ����: lichuang (2009-04-01)
 *********************************************************************/
static int __ResolveIpByName(const char *pszUrl, int iFamily)
{
	struct addrinfo *pResult = NULL;
	int nResult;
	struct addrinfo stAddr;
	char szHostName[256] = {'\0'};
	char szPort[MAX_PORT_LEN] = {'\0'};

    nResult = tbsSplitUrl(pszUrl, szHostName, szPort, NULL);
	if (SPLIT_ERROR == nResult)
	{
		return -1;
	}
	else if (SPLIT_IP == nResult || SPLIT_IP6 == nResult)
	{
		char szTemp[20];
		snprintf(szTemp, sizeof(szTemp), "/var/%d.cache", getpid());

		FILE *pFile = fopen(szTemp, "w");
		if (!pFile)
		{
			return -1;
		}

		fprintf(pFile, "%s\n", szPort);
		fprintf(pFile, "%s\n", szHostName);
		fclose(pFile);
		return 0;
	}

	memset(&stAddr, 0 , sizeof(stAddr));
	stAddr.ai_socktype = SOCK_STREAM;
	stAddr.ai_family = iFamily;
	//stAddr.ai_flags = AI_CANONNAME;

	nResult = getaddrinfo(szHostName, NULL, &stAddr, &pResult);
	if (!nResult)
	{
		char szTemp[20];
		snprintf(szTemp, sizeof(szTemp), "/var/%d.cache", getpid());

		FILE *pFile = fopen(szTemp, "w");
		if (!pFile)
		{
			return -1;
		}

		fprintf(pFile, "%s\n", szPort);

        char szAddr[MAX_IP6_LEN];
		struct addrinfo *pCurr = pResult;
		for (; pCurr; pCurr = pCurr->ai_next)
		{
			if(pCurr->ai_family == AF_INET)
			{
                inet_ntop(pCurr->ai_family, &(((struct sockaddr_in *)(pCurr->ai_addr))->sin_addr), szAddr, MAX_IP_LEN);
			}
#ifdef CONFIG_TBS_SUPPORT_IPV6
			else if(pCurr->ai_family == AF_INET6)
			{
				inet_ntop(pCurr->ai_family, &(((struct sockaddr_in6 *)(pCurr->ai_addr))->sin6_addr), szAddr, MAX_IP6_LEN);
			}
#endif
            fprintf(pFile, "%s\n", szAddr);
		}
		fclose(pFile);
	}

	freeaddrinfo(pResult);

	return (nResult != 0);
}

/*********************************************************************
*  ����: tbsResolveIpByName
*  ����: �Դ�����������н���, �����������ST_DNS_INFO������, ����dns�����ͷֱ����IPV4/IPV6��ַ
*  ����:
		pszUrl:     ��Ҫ����������
		ppDnsInfo:  ��Ž������������Ԫ��ָ��, ʹ�����֮��ʹ����Ҫ
                    ʹ��tbsFreeDnsInfo�����ͷ�
		pPort:      ��Ž���֮��Ķ˿ں�
		nMsecond:	��ʱ�õĺ�����
		pServer:    ����DNS��server��ַ, ����Ϊ��, Ϊ����ʹ��ϵͳ�ĵ�ַ

*  ����: TBS_SUCCESS/TBS_FAILED
*  ����: lichuang (2009-04-01)
*  ��ע:
         1) ����ú������óɹ�, ��Ҫ�������Լ�ȥ�ͷ�ppDnsInfo���ڴ�
            �����������ʧ��, ����Ҫ����Щ,�ñ�mallocʧ��֮����ڴ治��Ҫfreeһ��

         2) ���URL��û�ж˿ں�,��ô����Э������Ĭ�϶˿ڣ���: http->80, https->443
*********************************************************************/
int tbsResolveIpByName(const char *pszUrl, ST_DNS_INFO **ppDnsInfo,
					char* pPort, int nMsecond, const char* pServer)
{
    int iRet = 0;
    *ppDnsInfo = NULL;
	pid_t nPid = fork();

	if (0 > nPid)
	{
		return TBS_FAILED;
	}
	else if (0 == nPid)
	{
		// child
        int af_family = AF_UNSPEC;

		if (pServer)
		{
			af_family = SetDefaultDns(pServer);
		}

        res_init();

        iRet = __ResolveIpByName(pszUrl, af_family);

        if (pServer)
		{
			RestoreOldDns();
		}

		if (iRet < 0)
		{
			exit(-1);
		}
		else
		{
			exit(0);
		}
	}
	else
	{
		// parent
		int nStatus = 0, nRet = 0;

		nMsecond *= 1000;

		/* ÿ��ѭ��usleep100΢�� */
		while (nMsecond > 0)
		{
			nMsecond -= 10000;
			nRet = waitpid(nPid, &nStatus, WNOHANG);
			if (nRet == nPid)
			{
				break;
			}
			usleep(10000);
		}

		if (!nRet)
		{
			char szTemp[32];
			snprintf(szTemp, sizeof(szTemp), "kill -9 %d", nPid);
			system(szTemp);
            do {
                nRet = waitpid(nPid, &nStatus, WNOHANG);
            } while (nRet != nPid);
			return TBS_FAILED;
		}
		else
		{
			if (-1 == nStatus)
			{
				return TBS_FAILED;
			}
			else if (0 == nStatus)
			{
				char szTemp[50];
				int nFlag = 0, nLen;

				snprintf(szTemp, sizeof(szTemp), "/var/%d.cache", nPid);
				FILE *pFile = fopen(szTemp, "r");
				if (!pFile)
				{
            		unlink(szTemp);
					return TBS_FAILED;
				}

				ST_DNS_INFO *pDnsInfo;
				memset(szTemp, 0, sizeof(szTemp));

				while (fgets(szTemp, sizeof(szTemp), pFile))
				{
					nLen = strlen(szTemp);
					if (!nLen)
						break;
					szTemp[nLen - 1] = '\0';

					if (!nFlag)
					{
						nFlag = 1;

                        if(pPort)
						    strncpy(pPort, szTemp, MAX_PORT_LEN);
					}
					else
					{
						pDnsInfo = (ST_DNS_INFO*)malloc(sizeof(ST_DNS_INFO));
						if (!pDnsInfo)
						{
							tbsFreeDnsInfo(*ppDnsInfo);
                            *ppDnsInfo = NULL;
							fclose(pFile);

                            snprintf(szTemp, sizeof(szTemp), "/var/%d.cache", nPid);
            				unlink(szTemp);

							return -1;
						}
						if (!*ppDnsInfo)
						{
							*ppDnsInfo = pDnsInfo;
							pDnsInfo->pNext = NULL;
						}
						else
						{
							pDnsInfo->pNext = *ppDnsInfo;
							*ppDnsInfo = pDnsInfo;
						}
						strcpy(pDnsInfo->szIp, szTemp);
					}
				}

				fclose(pFile);
				snprintf(szTemp, sizeof(szTemp), "/var/%d.cache", nPid);
				unlink(szTemp);

				return TBS_SUCCESS;
			}
		}
	}

	return TBS_FAILED;
}

/*********************************************************************
 *  ����: tbsFreeDnsInfo

 *  ����: ����ST_DNS_INFO����, �ͷ����������Ԫ��

 *  ����:
pDnsInfo:  ��Ҫ�ͷŵ�����Ԫ��ָ��

 *  ����:

 *  ����: lichuang (2009-04-01)
 *********************************************************************/
void	tbsFreeDnsInfo(ST_DNS_INFO* pDnsInfo)
{
	ST_DNS_INFO* pTemp;
	while (pDnsInfo)
	{
		pTemp = pDnsInfo->pNext;
		free(pDnsInfo);
		pDnsInfo = pTemp;
	}
}

#define MAX_DNS_COUNT   		3
#define DNS_DEFAULT_TIMEOUT   	300 /* ms */
/*********************************************************************
*  ����: tbsResolveURL
*  ����: �Դ��������ʹ��ָ���Ľӿں�dns���н���, ������������
*  ����:
		pszUrl:         ��Ҫ����������
		pszIfName:      �������ݰ��ĳ����豸
		pszGateway:     �����豸��Ĭ������
		pszDnsServers:  ��������ʹ�õ�dns�������б�(�ö��ŷָ�)
		pcIpAddr:       ��Ž���������IP��ַ(out)
		pcPort:         ��Ž��������Ķ˿�(out)

*  ����: TBS_SUCCESS/TBS_FAILED
*  ����: kuangsuwen (2010-05-25)
*  ��ע:
         1) �������ĳ����豸��Ĭ�����ء�dns�������б�Ϊ�գ���ʹ��ϵͳĬ��·�ɺ�dnsȥ����
         2) ���URL��û�ж˿ں�,��ô����Э�鷵��Ĭ�϶˿ڣ���: http->80, https->443
*********************************************************************/
int tbsResolveURL(const char *pszUrl, const char *pszIfName, const char *pszGateway, const char *pszDnsServers,
    char *pcIpAddr, char *pcPort)
{
    int iRet = TBS_SUCCESS;

    int i = 0;
    int nCnt  = 0;
    char acDNSServers[256] = {0};
    char *apcNameServer[3] = {0};
    ST_DNS_INFO *pDNSInfo = NULL;

    char acIP[MAX_IP6_LEN] = {0};
    char acPort[MAX_PORT_LEN] = {0};
    char acCmd[MAX_CMD_LEN] = {0};

    int bIsSuccess =  0;
    int bIsIPv6 =  0;

    /* ͨ��ָ���Ľӿں�dnsȥ���Խ������� */
    if (safe_strlen(pszDnsServers) && safe_strlen(pszGateway) && safe_strlen(pszGateway))
    {

        #ifdef CONFIG_TBS_SUPPORT_IPV6
        /* �ж��Ƿ�ʹ��ipv6�������� */
        bIsIPv6 = (TBS_SUCCESS == tbsCheckIpv6Addr(pszGateway));
        #endif

        /* �ֽ�DNSServers */
        strcpy(acDNSServers, pszDnsServers);
        nCnt = tbsStrListSplit(acDNSServers, ',', apcNameServer, MAX_DNS_COUNT);
        if (nCnt > 3)
        {
            printf("Error: ip_addr count error\n");
            return TBS_FAILED;
        }

        /* �ֱ�ʹ�ò�ͬ��dns��������ȥ���������URL */
        for(i = 0; i < nCnt; i++)
        {
            if (strlen(apcNameServer[i]) == 0)
            {
                continue;
            }
            printf("Address = %s, apcNameServer = %s\n", pszUrl, apcNameServer[i]);

            sprintf(acCmd, "ip %s route add %s via %s dev %s", bIsIPv6 ? "-6" : "-4", apcNameServer[i], pszGateway, pszIfName);
            tbsSystem(acCmd, TBS_PRINT_CMD);

            iRet = tbsResolveIpByName(pszUrl, &pDNSInfo, acPort, DNS_DEFAULT_TIMEOUT, apcNameServer[i]);

            sprintf(acCmd, "ip %s route del %s via %s dev %s", bIsIPv6 ? "-6" : "-4", apcNameServer[i], pszGateway, pszIfName);
            (void)tbsSystem(acCmd, TBS_PRINT_CMD);

            if (RET_SUCCEED(iRet))
            {
                printf("Dns server is %s, Output IP is %s, Port is %s\n", apcNameServer[i], pDNSInfo->szIp, acPort);
                strcpy(acIP, pDNSInfo->szIp);
                tbsFreeDnsInfo(pDNSInfo);
                bIsSuccess = 1;
                break;
            }
            else
            {
                printf("Error: Resolve failed! NameServer is \"%s\"\n", apcNameServer[i]);
            }
        }
    }
    /* ʹ��ϵͳĬ�Ͻӿں�dnsȥ�������� */
    else
    {
        iRet = tbsResolveIpByName(pszUrl, &pDNSInfo, acPort, DNS_DEFAULT_TIMEOUT, NULL);
        if (RET_SUCCEED(iRet))
        {
            printf("Resolve by default Dns server, Output IP is %s, Port is %s\n", pDNSInfo->szIp, acPort);
            strcpy(acIP, pDNSInfo->szIp);
            tbsFreeDnsInfo(pDNSInfo);
            bIsSuccess = 1;
        }
    }

    /* �ж��Ƿ�����ɹ� */
    if (!bIsSuccess)
    {
        printf("Resolve failed by all Dns server.\n");
        return TBS_FAILED;
    }

    /* ���ؽ����ɹ���IP�Ͷ˿� */
    if(pcIpAddr != NULL)
    {
       strcpy(pcIpAddr, acIP);
    }
    if(pcPort != NULL)
    {
       strcpy(pcPort, acPort);
    }

    return TBS_SUCCESS;
}


/*********************************************************************
*  ����: Strtok_r

*  ����: �̰߳�ȫ��strtok������ʵ��, ��������net-snmp-5.4.1��Ŀ

*  ����:
	pString: ��Ҫ�������ַ���, ��һ�ε��õ�ʱ����Ҫ������Ҫ�������ַ���
			 ��������NULL����
	pDelim:  �ָ��ַ���
	ppLast:  ������һ�ν��������ָ��

*  ����:
			�����NULL���Ҳ������ָ����ַ�����, �������Ѿ��ָ��õ��ַ���

*  ����: lichuang (2009-04-01)
*********************************************************************/
char* Strtok_r(char *pString, const char *pDelim, char **pLast)
{
	const char *pSpanp;
	int c, sc;
	char *pTok;

	if (pString == NULL && (pString = *pLast) == NULL)
		return (NULL);

	/*
	 * ��pString�в��ҵ�һ���Ƿָ�����Ԫ�ص�λ��
	*/
cont:
	c = *pString++;
	for (pSpanp = pDelim; (sc = *pSpanp++) != 0;)
	{
		if (c == sc)
			goto cont;
	}

	/*
	 * ���û�ҵ�, ֱ�ӷ���
	*/
	if (c == 0)
	{
		*pLast = NULL;
		return (NULL);
	}

	/*
	 * ����һ��λ��, ��λ���Ǻ���Ҫ���ص�ָ��
	*/
	pTok = pString - 1;

	/*
	 * �ӵ�ǰλ�ÿ�ʼ���ҵ�һ���ָ�����Ԫ�ص�λ��, �����λ����0Ȼ�󷵻�
	*/
	while (1)
	{
		c = *pString++;
		pSpanp = pDelim;
		do
		{
			if ((sc = *pSpanp++) == c)
			{
				if (c == 0)
					pString = NULL;
				else
					pString[-1] = 0;
				*pLast = pString;
				return (pTok);
			}
		} while (sc != 0);
	}
}

/**************************************************************************
��  ��: ����ethernet WAN�ӿ����ƻ�ȡ�ӿڵ�Э������(��λMbps)

��  ��: pszDevName -- ethernet WAN�ӿ�����(��eth0)

����ֵ: ethernet WAN�ӿڵ�Э������

��  ע: created by XuXiaodong 2009-3-18
***************************************************************************/
int tbsGetEthWanIfLinkSpeed(const char *pszDevName)
{
    struct ifreq ifr;
	struct ethtool_cmd ecmd;
	int fd, err;

	if (pszDevName == NULL)
	{
		return TBS_FAILED;
	}

	/* Setup control structures. */
	memset(&ecmd, 0, sizeof(ecmd));
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, pszDevName);

	/* Open control socket. */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Cannot get control socket");
		return TBS_FAILED;
	}

	ecmd.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t)&ecmd;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err == 0)
	{
		close(fd);
		return (int)ecmd.speed;
	}
	else
	{
		//printf("Call function \"ioctl\" failed\n");
		close(fd);
		return TBS_FAILED;
	}
}

/**************************************************************************
��  ��: ����ppp�ӿ���(pppoe)��ȡ���ppp�ӿ����������̫���ӿ���
��  ��: pszPppDevName -- ppp�ӿ���
        pszPhyDevName -- ����ʱ�����ppp�ӿ������������ӿ���
        iLen -- pszPhyDevNameָ��Ļ�������С
����ֵ: �ɹ� -- TBS_SUCCESS
        ʧ�� -- ����
��  ע: created by XuXiaodong 2010-4-14
***************************************************************************/
#define PPPIOCGPHYDEV  _IOR('t', 54, int)	/* get ppp physical device name */
int tbsGetPppPhyDev(const char *pszPppDevName, char *pszPhyDevName, int iLen)
{
	int iFd = 0;
	char szDevName[MAX_IF_NAME_LEN] = {0};
	int iRet = TBS_SUCCESS;

	if (!pszPppDevName || !strstr(pszPppDevName, "ppp") || !pszPhyDevName)
	{
		return ERR_INVALID_VAL;
	}

	safe_strncpy(szDevName, pszPppDevName, sizeof(szDevName));
	iFd = open("/dev/ppp", O_RDWR);
	if (iFd < 0)
	{
		return ERR_FILE_OPEN;
	}

	iRet = ioctl(iFd, PPPIOCGPHYDEV, (int)szDevName);
	if (iRet < 0)
	{
	    close(iFd);
		return TBS_FAILED;
	}

	safe_strncpy(pszPhyDevName, szDevName, iLen);
	close(iFd);
	return TBS_SUCCESS;
}

/*************************************************************************
��  ��: url���
��  ��: pValue  URL���
����ֵ: TBS_SUCCESS/����������
��  ע:
*************************************************************************/
int tbsCheckUrl(const char *pValue)
{
    char *pattern ="^(http://|https://)?(([0-9a-zA-Z_!~*'().&=+$%-]+: )?[0-9a-zA-Z_!~*'().&=+$%-]+@)?(([0-9]{1,3}\\.){3}[0-9]{1,3}|([0-9a-zA-Z][0-9a-zA-Z-]{0,61})(\\.[a-zA-Z0-9][-a-zA-Z0-9]{0,61})*)((:[0-9]{1,4})|(:[1-5][0-9]{1,4})|(:6[0-4][0-9]{3})|(:65[0-4][0-9]{2})|(:655[0-2][0-9])|(:6553[0-5]))?((/?)|(/[0-9a-zA-Z_!~*'().;?:@&=+$,%#-]+)+/?)$";

    /* URL����Ϊ�� */
    if(safe_strlen(pValue) == 0)
    {
        return ERR_CAN_NOT_EMPTY;
    }

    /* URL���ܹ��� */
    if(strlen(pValue) > URL_STR_URL_MAX_LEN)
    {
        return ERR_URLFILTER_URL_LENGTH_OVER;
    }

    if(!tbsMatch(pValue, pattern))
    {
        return ERR_URLFILTER_URL_ERROR;
    }

    return TBS_SUCCESS;
}


int tbsCheckUserExWithLength(const char *pszValue, const int iLen)
{
    char pattern[MAX_USER_LEN] = { 0 };
    sprintf(pattern, "^[0-9a-zA-Z\\.@_-]{1,%d}$", iLen);

    if (pszValue == NULL)
    {
        return ERR_USERNAME;
    }

    if (FALSE == tbsMatch(pszValue, pattern))
    {
        return ERR_USERNAME;
    }

    return TBS_SUCCESS;
}


int tbsCheckPasswdExWithLength(const char *pszValue, const int iLen)
{
    if (pszValue == NULL)
    {
        return ERR_PASSWORD;
    }

    if (tbsCheckUserExWithLength(pszValue, iLen) != TBS_SUCCESS)
    {
        return ERR_PASSWORD;
    }

    return TBS_SUCCESS;
}

#ifdef CONFIG_VX180
typedef struct tag_ST_APFEATURE_MEMBER_LIST {
    struct tag_ST_APFEATURE_MEMBER_LIST *pstNext;
    unsigned short ucMID;
} ST_APFEATURE_MEMBER_LIST;

int tbsAddApfeaturMember(ST_APFEATURE_MEMBER_LIST **ppstApMemberList, unsigned short ucMID)
{
    ST_APFEATURE_MEMBER_LIST *pstNewMember, *pstMember;

    pstMember = *ppstApMemberList;  /* ppstApMemberList ����Ϊ NULL */
    while(pstMember)
    {
        if (pstMember->ucMID == ucMID)
            break;
        pstMember = pstMember->pstNext;
    }

    /* �Ѿ���¼��ģ����� apfeature ���� */
    if (pstMember)
        return 0;

    /* ��¼ģ����� apfeature ���� */
    pstNewMember = (ST_APFEATURE_MEMBER_LIST *)malloc(sizeof(ST_APFEATURE_MEMBER_LIST));
    if (NULL == pstNewMember)
    {
        fprintf(stderr, "ERROR: failed to record apfeature member, Out of memory\n");
        return -1;
    }

    memset(pstNewMember, 0, sizeof(ST_APFEATURE_MEMBER_LIST));
    pstNewMember->ucMID = ucMID;

    /* �����б� */
    pstMember = *ppstApMemberList;
    if (!pstMember)
    {
        *ppstApMemberList = pstNewMember;
    }
    else
    {
        while(pstMember)
        {
            if (!pstMember->pstNext)
            {
                pstMember->pstNext = pstNewMember;
                break;
            }

            pstMember = pstMember->pstNext;
        }
    }

    return 0;
}

int tbsRemoveApfeaturMember(ST_APFEATURE_MEMBER_LIST **ppstApMemberList, unsigned short ucMID)
{
    ST_APFEATURE_MEMBER_LIST *pstMemberPrev = NULL, *pstMember;

    pstMember = *ppstApMemberList;  /* ppstApMemberList ����Ϊ NULL */
    while(pstMember)
    {
        if (pstMember->ucMID == ucMID)
            break;

        pstMemberPrev = pstMember;
        pstMember = pstMember->pstNext;
    }

    /* û��ģ����� apfeature ���ܵļ�¼ */
    if (!pstMember)
        return 0;

    /* ���б���ɾ�� */
    if (!pstMemberPrev)
    {
        *ppstApMemberList = pstMember->pstNext;
    }
    else
    {
        pstMemberPrev->pstNext = pstMember->pstNext;
    }

    free(pstMember);

    return 0;
}

int tbsCountApfeatureMembers(ST_APFEATURE_MEMBER_LIST **ppstApMemberList)
{
    int iCount = 0;
    ST_APFEATURE_MEMBER_LIST *pstMember;

    pstMember = *ppstApMemberList;  /* ppstApMemberList ����Ϊ NULL */
    while(pstMember)
    {
        iCount++;
        pstMember = pstMember->pstNext;
    }

    return iCount;
}

int tbsApfeatureControl(unsigned short ucMID, int iEnable)
{
    static ST_APFEATURE_MEMBER_LIST *pstApMemberList = NULL;
    int iModuleCount = 0; /* ���� AP ���ٹ���ģ��ļ��� */
    char szCommand[100];

    /* ����/ɾ�������� apfeature ���ܵ�ģ���¼ */
    if (iEnable)
        tbsRemoveApfeaturMember(&pstApMemberList, ucMID);
    else
    {
        if (0 != tbsAddApfeaturMember(&pstApMemberList, ucMID))
            return -1;
    }

    iModuleCount = tbsCountApfeatureMembers(&pstApMemberList);

    fprintf(stderr, "INFO: MID: (%u), iEnable: (%d), apfeature %s\n",
            ucMID, iEnable, iModuleCount ? "disable" : "enable");

    sprintf(szCommand, "sysutil apfeature ALL 6 %s", iModuleCount ? "disable" : "enable");
    tbsSystem(szCommand, TBS_PRINT_CMD);

    sprintf(szCommand, "sysutil apfeature ALL 5 %s", iModuleCount ? "disable" : "enable");
    tbsSystem(szCommand, TBS_PRINT_CMD);

    sprintf(szCommand, "sysutil apfeature ALL VLANBRIDGE %s", iModuleCount ? "disable" : "enable");
    tbsSystem(szCommand, TBS_PRINT_CMD);

    return 0;
}

/* ���� IKANOS AP ���ٹ��� */
int tbsApfeatureEnable(unsigned short ucMID)
{
    return tbsApfeatureControl(ucMID, 1);
}

/* ���� IKANOS AP ���ٹ��� */
int tbsApfeatureDisable(unsigned short ucMID)
{
    return tbsApfeatureControl(ucMID, 0);
}
#endif

void tbsClearRamForUpgrade(int iUpgradeFlag)
{
	/* kill������Ҫ�Ľ��̣����﷢��16�źŸ�PC */
	tbsSystemMute("killall -16 pc");

     //sleep(1);

	/* ���iptables��ebtales�����й��� */
	tbsSystemMute("iptables -Z");
	tbsSystemMute("iptables -F");
	tbsSystemMute("iptables -X");
	tbsSystemMute("iptables -t nat -Z");
	tbsSystemMute("iptables -t nat -F");
	tbsSystemMute("iptables -t nat -X");
	tbsSystemMute("iptables -t mangle -Z");
	tbsSystemMute("iptables -t mangle -F");
	tbsSystemMute("iptables -t mangle -X");

	tbsSystemMute("ebtables -Z");
	tbsSystemMute("ebtables -F");
	tbsSystemMute("ebtables -X");
	tbsSystemMute("ebtables -t nat -Z");
	tbsSystemMute("ebtables -t nat -F");
	tbsSystemMute("ebtables -t nat -X");
	tbsSystemMute("ebtables -t broute -Z");
	tbsSystemMute("ebtables -t broute -F");
	tbsSystemMute("ebtables -t broute -X");

     //tbsSystemMute("flush_conntrack");
#if 0
	if(iUpgradeFlag == TR069_UPGRADE)
	{
		/* ��tr069������down��LAN�˽ӿ� */
		tbsSystemMute("ifconfig br1 down");
		tbsSystemMute("ifconfig eth0 down");
		tbsSystemMute("brctl delif br1 eth0");
		tbsSystemMute("brctl delbr br1");
		tbsSystemMute("ifconfig lo down");
	}
	else
	{
		tbsSystemMute("ifconfig lo down");
		tbsSystemMute("killall pc");
	    tbsSystemMute("killall dsl_cpe_control");
	}
#endif

    /* FIXME: only for LAN port upgrade */
    //tbsSystemMute("killall pc");
    //tbsSystemMute("killall dsl_cpe_control");
    //sleep(2);

    tbsSystemMute("echo 3 > proc/sys/vm/drop_caches");

    //sleep(1);
}

/*********************************************************************
*  ����: �ý��Ǳ�Ϊ�ػ�����
*  ����: closefd    �Ƿ�ر������ļ�������
*  ����: void
*  ����: ������ / 2009-7-22
*********************************************************************/
int tbsDaemonize(int closefd)
{
	int pid = 0;
    int i   = 0;

	switch(pid = fork())
	{
    	/* fork error */
    	case -1:
    		perror("fork()");
    		exit(1);

    	/* child process */
    	case 0:
    		/* obtain a new process group */
    		if( (pid = setsid()) < 0)
    		{
    			perror("setsid()");
    			exit(1);
    		}

            if (closefd)
            {
        		/* close all descriptors */
        		for (i=getdtablesize();i>=0;--i)
        		{
                    close(i);
        		}

                /* open stdin */
        		i = open("/dev/console",O_RDWR);

                /* stdout */
        		dup(i);

                /* stderr */
        		dup(i);
            }

    		umask(027);
    		chdir("/"); /* chdir to /tmp ? */

    		return pid;

    	/* parent process */
    	default:
    		exit(0);
	}

	return pid;
}

/* ԭ�ַ���תΪ16�����ַ�������"0!k" תΪ "30216b"
	ʹ��ʱע�⣬szDecHex�������2����i2Num */
int tbsSting2Hex(char *szSrcString, char *szDecHex, unsigned int i2Num)
{
	int i = i2Num;
	char *srcPrt = szSrcString;
	char *decPrt = szDecHex;
	char tmp;

	if((szSrcString==NULL) || (szDecHex==NULL))
	{
		return TBS_FAILED;
	}

	while(i--)
	{
		tmp = (*srcPrt&0xf0) >> 4;
		if(tmp <= 9)
		{
			*(decPrt++) = tmp + 0x30;
		}
		else
		{
			*(decPrt++) = tmp + 0x37;
		}

		tmp = *srcPrt&0x0f;
		if(tmp <= 9)
		{
			*(decPrt++) = tmp + 0x30;
		}
		else
		{
			*(decPrt++) = tmp + 0x37;
		}

		srcPrt++;
	}

	*decPrt = '\0';

	return TBS_SUCCESS;
}
/*********************************************************************
*  ����: �ַ����滻���ܺ���
*  ����:
*  ����: const char *
*  ����: huangjidong
*  ��ע: ���ص��ַ�����Ҫ�û������ͷ�
*********************************************************************/
char* tbsStringReplaceWithMalloc(const char *pcSrc, const char *pcOldStr, const char *pcNewStr)
{
    char *pcRetBuf = NULL;
    char *pcBuf = NULL;
    char *pcTmp = NULL;
    char *pcStart = NULL;
    char *pcEnd = NULL;
    int iCount = 0;
    int iOldStrLen = 0;
    int iRetLen = 0;

    if(NULL == pcSrc || NULL == pcOldStr || NULL == pcNewStr)
        return NULL;

    pcTmp = pcBuf = strdup(pcSrc);
    /*if equal, return without replace*/
    if(0 == strcmp(pcOldStr, pcNewStr))
        return pcBuf;

    iOldStrLen = strlen(pcOldStr);
    while(NULL != (pcTmp = strstr(pcTmp, pcOldStr)))
    {
        iCount++;
        pcTmp += iOldStrLen;
    }

    /*calculate memory size add modify it*/
    iRetLen = strlen(pcSrc) + (strlen(pcNewStr)-strlen(pcOldStr))*iCount + 1;
    pcRetBuf = malloc(iRetLen);
    if(NULL == pcRetBuf)
    {
        free(pcBuf);
        return NULL;
    }
    memset(pcRetBuf, 0, iRetLen);

    pcStart = pcBuf;
    pcTmp = pcRetBuf;
    /*replace content in pcTmp to pcRetBuf*/
    while(NULL != (pcEnd = strstr(pcStart, pcOldStr)))
    {
        strncpy(pcTmp, pcStart, pcEnd-pcStart);
        strcat(pcTmp, pcNewStr);
        pcTmp+=strlen(pcTmp);
        pcStart = pcEnd + iOldStrLen;
    }
    strcat(pcTmp, pcStart);

    free(pcBuf);
    return pcRetBuf;
}

/*********************************************************************
*  ����: �ַ����滻���ܺ���(�滻ԭ���ַ���)
*  ����:
*  ����: const char *
*  ����: huangjidong
*  ��ע: ��Ҫ��֤ԭ���ַ����Ŀռ�������滻�ַ�֮����ַ�������
*********************************************************************/
char* tbsStringReplace(char *pcSrc, const char *pcOldStr, const char *pcNewStr)
{
    char *pcTmp = NULL;
    int iSrcLen    = 0;
    int iOldStrLen = 0;
    int iNewStrLen = 0;

    iSrcLen    = safe_strlen(pcSrc);
    iOldStrLen = safe_strlen(pcOldStr);
    iNewStrLen = safe_strlen(pcNewStr);

    /* check valid */
    if(!iSrcLen || !iOldStrLen || NULL == pcNewStr)
        return NULL;

    /* if equal, return without replace */
    if(0 == strcmp(pcOldStr, pcNewStr))
        return pcSrc;

    pcTmp = pcSrc;
    while(NULL != (pcTmp = strstr(pcTmp, pcOldStr)))
    {
        memmove(pcTmp+iNewStrLen, pcTmp+iOldStrLen, strlen(pcTmp) - iOldStrLen + 1);
        memcpy(pcTmp, pcNewStr, iNewStrLen);
        pcTmp += iNewStrLen;
    }

    return pcSrc;
}


/**************************************************************************
����: ͨ���ӿ�����ȡ�ӿڶ�Ӧ��MAC��ַ
����: pszIfName  ��Žӿ����Ļ�����ָ��(����)
      pszValue   ���MAC��ַ�Ļ�����ָ��(���)
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע: ���øú����л���������ķ���,������Ӧ��ΪpszValueָ��Ļ���������
      �㹻�Ŀռ������MAC��ַ
***************************************************************************/
int tbsGetMacAddr(const char *pszIfName, char *pszMac)
{
    struct ifreq stIfr;
    int iSockfd;

    strcpy(stIfr.ifr_name, pszIfName);

    /* �����׽���ʧ��*/
    if ((iSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return TR069_ERRNO_CPE_ERR;
    }

    if (ioctl(iSockfd, SIOCGIFHWADDR, &stIfr) < 0)
    {
        close(iSockfd);
        return TR069_ERRNO_CPE_ERR;
    }

    /* ��ȡMAC��ַ*/
    sprintf(pszMac, "%02x:%02x:%02x:%02x:%02x:%02x",
                        (unsigned char)stIfr.ifr_hwaddr.sa_data[0],
                        (unsigned char)stIfr.ifr_hwaddr.sa_data[1],
                        (unsigned char)stIfr.ifr_hwaddr.sa_data[2],
                        (unsigned char)stIfr.ifr_hwaddr.sa_data[3],
                        (unsigned char)stIfr.ifr_hwaddr.sa_data[4],
                        (unsigned char)stIfr.ifr_hwaddr.sa_data[5]);
    close(iSockfd);

    return TBS_SUCCESS;
}


/*
��ָ�����ļ��в����ַ���
*/
int tbsFindStrInFile(const char *pszFileName, const char *pszStr)
{
    FILE *fp = NULL;
    char result[512]={0};
    int iRet = TBS_FAILED;

    fp = fopen(pszFileName, "r");
    if(NULL == fp)
    {
    	return iRet;
    }

    while (1)
    {
        if (fgets(result, sizeof(result), fp) == NULL)
        {
            break;
        }
        if (strstr(result, pszStr))
        {
            iRet = TBS_SUCCESS;
            break;
        }
    }
    fclose(fp);

    return iRet;
}


/*
���ַ���д�뵽ָ�����ַ�����
*/
int tbsWriteStrToFile(const char *pszFileName, const char *pszStr)
{
    FILE *fp = fopen(pszFileName, "w");
    if(NULL == fp)
    {
    	return TBS_FAILED;
    }

    fprintf(fp, "%s", pszStr);
    fclose(fp);

    return TBS_SUCCESS;
}


/*******************************************************************************
* Function    : GetSysUpTime
* Description : ȡ��ϵͳ����������ʱ�䣬��λΪ��
* Parameters  :
* Return      : ��
* Author      : wuyun / 2010-01-21
* History     :
*******************************************************************************/
inline long GetSysUpTime()
{
    struct sysinfo info;
    sysinfo(&info);
    return info.uptime;
}

/******************************************************************************
 *       BASE64 encode/decode
 ******************************************************************************/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";//
/* cb64 cd64 */
/* encode 3 8-bit binary bytes as 4 '6-bit' characters */
static inline void ILibencodeblock( unsigned char in[3], unsigned char out[4], int len )
{
	out[0] = cb64[ in[0] >> 2 ];
	out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
	out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/* Decode 4 '6-bit' characters into 3 8-bit binary bytes */
static inline void ILibdecodeblock( unsigned char in[4], unsigned char out[3] )
{
	out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
	out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
	out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}


/******************************************************************
����: ����64λ���ܷ�ʽ���ַ������ܺ���
����: *value:�����ַ���
        vlen:���ܳ���
����: result: ���ܺ���ַ���
��ע: ���ô˺�����ע�����ⲿfree����ָ��
******************************************************************/
char *tbsBase64Encode(const unsigned char *value, int vlen)
{
	unsigned char oval = 0 ;
	char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *result = (char *)malloc((vlen * 4) / 3 + 5) ;
    char *out = result;
	while (vlen >= 3) {
        	*out++ = basis_64[value[0] >> 2];
        	*out++ = basis_64[((value[0] << 4) & 0x30) | (value[1] >> 4)];
        	*out++ = basis_64[((value[1] << 2) & 0x3C) | (value[2] >> 6)];
        	*out++ = basis_64[value[2] & 0x3F];
        	value += 3;
       		vlen -= 3;
    	}
	if (vlen > 0) {
	        *out++ = basis_64[value[0] >> 2];
	        oval = (value[0] << 4) & 0x30 ;
	        if (vlen > 1) oval |= value[1] >> 4;
	        *out++ = basis_64[oval];
	        *out++ = (vlen < 2) ? '=' : basis_64[(value[1] << 2) & 0x3C];
	        *out++ = '=';
	}
	*out = '\0';
    /* ��ע�⣬��ָ����Ҫ���ⲿfree */
	return result;
}

/******************************************************************
����: ����64λ���ܷ�ʽ���ַ������ܺ���
����: *value:�����ַ���
        vlen:���ܺ󳤶�
����: result: ���ܺ���ַ���
��ע: ���ô˺�����ע�����ⲿfree����ָ��
******************************************************************/
unsigned char *tbsBase64Decode(const char *value, int *rlen)
{
	int c1, c2, c3, c4;
	int vlen = strlen(value);
	unsigned char *result =(unsigned char *)malloc((vlen * 3) / 4 + 1);
	unsigned char *out = result;
	static signed char index_64[128] = {
	    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
	    52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
	    -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
	    15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
	    -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
	    41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
	} ;
#define CHAR64(c)  (((c) < 0 || (c) > 127) ? -1 : index_64[(c)])

	*rlen = 0;

	while (1) {
		if (value[0]==0) {
			*out = '\0' ;
			return result;
		}
		c1 = value[0];
		if (CHAR64(c1) == -1) goto base64_decode_error;
		c2 = value[1];
		if (CHAR64(c2) == -1) goto base64_decode_error;
		c3 = value[2];
		if ((c3 != '=') && (CHAR64(c3) == -1)) goto base64_decode_error;
		c4 = value[3];
		if ((c4 != '=') && (CHAR64(c4) == -1)) goto base64_decode_error;
		value += 4;
		*out++ = (CHAR64(c1) << 2) | (CHAR64(c2) >> 4);
		*rlen += 1;
		if (c3 != '=') {
			*out++ = ((CHAR64(c2) << 4) & 0xf0) | (CHAR64(c3) >> 2);
			*rlen += 1;
			if (c4 != '=') {
				*out++ = ((CHAR64(c3) << 6) & 0xc0) | CHAR64(c4);
				*rlen += 1;
			}
		}
	}
base64_decode_error:
	*result = 0;
	*rlen = 0;
	return result;
}
/*! \fn Base64_Encode_Safe(unsigned char* input, const int inputlen)
	\brief Decode a base64 encoded stream discarding padding, line breaks and noise
	\para
	\b Note: The decoded stream must be freed
	\param input The stream to decode
	\param inputlen The length of \a input
	\returns output The decoded stream
*/
char *Base64_Encode_Safe(const char* input, const int inputlen)
{
	unsigned char* out;
	unsigned char* in;
    unsigned char* inptr;
    char *result;

	if (input == NULL || inputlen == 0)
	{
		return NULL;
	}

    result = malloc(((inputlen * 4) / 3) + 5);
	out = (unsigned char*)result;
	in  = (unsigned char*)input;
	inptr  = (unsigned char*)input;

	while ((in+3) <= (inptr + inputlen))
	{
		ILibencodeblock(in, out, 3);
		in += 3;
		out += 4;
	}
    
	if ((inptr + inputlen)-in == 1)
	{
		ILibencodeblock(in, out, 1);
		out += 4;
	}
	else if ((inptr + inputlen)-in == 2)
	{
		ILibencodeblock(in, out, 2);
		out += 4;
	}
	*out = 0;

	return result;
}

/*! \fn Base64_Decode(unsigned char* input, const int inputlen)
	\brief Decode a base64 encoded stream discarding padding, line breaks and noise
	\para
	\b Note: The decoded stream must be freed
	\param input The stream to decode
	\param inputlen The length of \a input
	\returns output The decoded stream
*/
char *Base64_Decode_Safe(const char* input, const int inputlen)
{
    unsigned char* out;
	unsigned char* in;
    unsigned char* inptr;
    char *result;
	unsigned char v;
	unsigned char inarr[4];
	int i, len;

	if (input == NULL || inputlen == 0)
	{
		return NULL;
	}

	result = (unsigned char*)malloc(((inputlen * 3) / 4) + 4);
    out = (unsigned char*)result;
	in  = (unsigned char*)input;
	inptr  = (unsigned char*)input;

	while( in <= (inptr+inputlen) )
	{
		for( len = 0, i = 0; i < 4 && in <= (inptr+inputlen); i++ )
		{
			v = 0;
			while( in <= (inptr+inputlen) && v == 0 ) {
				v = (unsigned char) *in;
				in++;
				v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
				if( v ) {
					v = (unsigned char) ((v == '$') ? 0 : v - 61);
				}
			}
			if( in <= (inptr+inputlen) ) {
				len++;
				if( v ) {
					inarr[ i ] = (unsigned char) (v - 1);
				}
			}
			else {
				inarr[i] = 0;
			}
		}
		if( len )
		{
			ILibdecodeblock( inarr, out );
			out += len-1;
		}
	}
	*out = 0;
    
	return result;
}

/*************************************************************************
����: ���ַ���ת����Base64���룬���ⲻ֧�ֵ��ֿⱣ�浽�����У����������쳣
����: pStrIn : �����ַ���,���ֶ��ڴ�ռ�Ҫ���ǿ�д�ģ���Ԥ����Ŀռ��㹻���
			 ת������ַ�(ԭ4/3����)��
	 pStrOut : ����ַ���
����: �ɹ�TBS_SUCCESS��ʧ��TBS_FAILED
��ע:
*************************************************************************/
int Safe_Base64Encode(char *pStrIn, char *pStrOut, int size)
{
	int iLen = 0;
	char *pBuf = NULL;
	if (NULL == pStrIn || strlen(pStrIn) == 0)
	{
		return TBS_FAILED;
	}

	iLen = strlen(pStrIn);
	pBuf = Base64_Encode_Safe(pStrIn, iLen);
	
	if (NULL == pBuf)
	{
		return TBS_FAILED;
	}

	memset(pStrOut, 0, sizeof(pStrOut));
	safe_strncpy(pStrOut, pBuf, size);
	safe_free(pBuf);
	
	return TBS_SUCCESS;
}

/*************************************************************************
����: ���ַ�����Base64����ת������
����: pStrIn : �����ַ��������ֶ��ڴ�ռ�Ҫ���ǿ�д�ġ�ת����ĳ��Ȳ����ԭ������
	 pStrOut : ����ַ���
����: �ɹ�TBS_SUCCESS��ʧ��TBS_FAILED
��ע:
*************************************************************************/
int Safe_Base64Decode(char *pStrIn, char *pStrOut)
{
	int iLen = 0;
	char *pBuf = NULL;
	if (NULL == pStrIn || strlen(pStrIn) == 0)
	{
		return TBS_FAILED;
	}
	
	iLen = strlen(pStrIn);
	pBuf = Base64_Decode_Safe(pStrIn, iLen);
	
	if (NULL == pBuf)
	{
		return TBS_FAILED;
	}

	memset(pStrOut, 0, sizeof(pStrOut));
	strcpy(pStrOut, pBuf);
	safe_free(pBuf);
	
	return TBS_SUCCESS;
}
/******************************************************************************
 *      AES128 encode/decode
 ******************************************************************************/
static void AES_SubBytes(unsigned char state[], unsigned char sbox[]) {
  int i;
  for(i = 0; i < 16; i++)
    state[i] = sbox[state[i]];
}
 
static void AES_AddRoundKey(unsigned char state[], unsigned char rkey[]) {
  int i;
  for(i = 0; i < 16; i++)
    state[i] ^= rkey[i];
}
 
static void AES_ShiftRows(unsigned char state[], unsigned char shifttab[]) {
  unsigned char h[16];
  memcpy(h, state, 16);
  int i;
  for(i = 0; i < 16; i++)
    state[i] = h[shifttab[i]];
}
 
static void AES_MixColumns(unsigned char state[]) {
  int i;
  for(i = 0; i < 16; i += 4) {
    unsigned char s0 = state[i + 0], s1 = state[i + 1];
    unsigned char s2 = state[i + 2], s3 = state[i + 3];
    unsigned char h = s0 ^ s1 ^ s2 ^ s3;
    state[i + 0] ^= h ^ AES_xtime[s0 ^ s1];
    state[i + 1] ^= h ^ AES_xtime[s1 ^ s2];
    state[i + 2] ^= h ^ AES_xtime[s2 ^ s3];
    state[i + 3] ^= h ^ AES_xtime[s3 ^ s0];
  }
}
 
static void AES_MixColumns_Inv(unsigned char state[]) {
  int i;
  for(i = 0; i < 16; i += 4) {
    unsigned char s0 = state[i + 0], s1 = state[i + 1];
    unsigned char s2 = state[i + 2], s3 = state[i + 3];
    unsigned char h = s0 ^ s1 ^ s2 ^ s3;
    unsigned char xh = AES_xtime[h];
    unsigned char h1 = AES_xtime[AES_xtime[xh ^ s0 ^ s2]] ^ h;
    unsigned char h2 = AES_xtime[AES_xtime[xh ^ s1 ^ s3]] ^ h;
    state[i + 0] ^= h1 ^ AES_xtime[s0 ^ s1];
    state[i + 1] ^= h2 ^ AES_xtime[s1 ^ s2];
    state[i + 2] ^= h1 ^ AES_xtime[s2 ^ s3];
    state[i + 3] ^= h2 ^ AES_xtime[s3 ^ s0];
  }
}
 
// AES_Init: initialize the tables needed at runtime. 
// Call this function before the (first) key expansion.
static void AES_Init() {
  int i;
  for(i = 0; i < 256; i++)
    AES_Sbox_Inv[AES_Sbox[i]] = i;
   
  for(i = 0; i < 16; i++)
    AES_ShiftRowTab_Inv[AES_ShiftRowTab[i]] = i;
 
  for(i = 0; i < 128; i++) {
    AES_xtime[i] = i << 1;
    AES_xtime[128 + i] = (i << 1) ^ 0x1b;
  }
}
 
// AES_Done: release memory reserved by AES_Init. 
// Call this function after the last encryption/decryption operation.
static void AES_Done() {}
 
/* AES_ExpandKey: expand a cipher key. Depending on the desired encryption 
   strength of 128, 192 or 256 bits 'key' has to be a byte array of length 
   16, 24 or 32, respectively. The key expansion is done "in place", meaning 
   that the array 'key' is modified.
*/  
static int AES_ExpandKey(unsigned char key[], int keyLen) {
  int kl = keyLen, ks, Rcon = 1, i, j;
  unsigned char temp[4], temp2[4];
  switch (kl) {
    case 16: ks = 16 * (10 + 1); break;
    case 24: ks = 16 * (12 + 1); break;
    case 32: ks = 16 * (14 + 1); break;
    default: 
      printf("AES_ExpandKey: Only key lengths of 16, 24 or 32 bytes allowed!");
  }
  for(i = kl; i < ks; i += 4) {
    memcpy(temp, &key[i-4], 4);
    if (i % kl == 0) {
      temp2[0] = AES_Sbox[temp[1]] ^ Rcon;
      temp2[1] = AES_Sbox[temp[2]];
      temp2[2] = AES_Sbox[temp[3]];
      temp2[3] = AES_Sbox[temp[0]];
      memcpy(temp, temp2, 4);
      if ((Rcon <<= 1) >= 256)
        Rcon ^= 0x11b;
    }
    else if ((kl > 24) && (i % kl == 16)) {
      temp2[0] = AES_Sbox[temp[0]];
      temp2[1] = AES_Sbox[temp[1]];
      temp2[2] = AES_Sbox[temp[2]];
      temp2[3] = AES_Sbox[temp[3]];
      memcpy(temp, temp2, 4);
    }
    for(j = 0; j < 4; j++)
      key[i + j] = key[i + j - kl] ^ temp[j];
  }
  return ks;
}
 
// AES_Encrypt: encrypt the 16 byte array 'block' with the previously expanded key 'key'.
static void AES_Encrypt(unsigned char block[], unsigned char key[], int keyLen) {
  int l = keyLen, i;
//  printBytes(block, 16);
  AES_AddRoundKey(block, &key[0]);
  for(i = 16; i < l - 16; i += 16) {
    AES_SubBytes(block, AES_Sbox);
    AES_ShiftRows(block, AES_ShiftRowTab);
    AES_MixColumns(block);
    AES_AddRoundKey(block, &key[i]);
  }
  AES_SubBytes(block, AES_Sbox);
  AES_ShiftRows(block, AES_ShiftRowTab);
  AES_AddRoundKey(block, &key[i]);
}
 
// AES_Decrypt: decrypt the 16 byte array 'block' with the previously expanded key 'key'.
static void AES_Decrypt(unsigned char block[], unsigned char key[], int keyLen) {
  int l = keyLen, i;
  AES_AddRoundKey(block, &key[l - 16]);
  AES_ShiftRows(block, AES_ShiftRowTab_Inv);
  AES_SubBytes(block, AES_Sbox_Inv);
  for(i = l - 32; i >= 16; i -= 16) {
    AES_AddRoundKey(block, &key[i]);
    AES_MixColumns_Inv(block);
    AES_ShiftRows(block, AES_ShiftRowTab_Inv);
    AES_SubBytes(block, AES_Sbox_Inv);
  }
  AES_AddRoundKey(block, &key[0]);
}


static void print_bytes(char *str,unsigned char *input, int len)
{
	int i=0;
	printf ("%s\n",str);
	for (i=0;i<len;i++)
	{
//		if (i%2 == 0) printf (" ");
		printf ("%02x",input[i]);
	}
	printf ("\n");
	return ;
}

static unsigned int hexstr2array (char *input, unsigned char *output)
{
	unsigned int length = strlen(input);
	unsigned int i=0;
	char tmp[3];

	if ((length % 2) != 0)
	{
		fprintf (stderr, "[ERROR] %s:%u: input length error:%u\n", __FILE__,__LINE__,length);
		return 0;
	}

	for (i=0;i<length;i++)
	{
		char c = input[i];
		if (c < '0' ||
			((c > '9') && (c < 'A')) ||
			((c > 'F') && (c < 'a')) ||
			(c > 'f'))
			{
			fprintf (stderr, "[ERROR] %s:%u: input string error: %s\n", __FILE__,__LINE__, input);
			return 0;
			}
	}

	for (i=0;i<length;i+=2)
	{
		tmp[0] = input[i];
		tmp[1] = input[i+1];
		tmp[2] = '\0';
		output[i/2] = strtol (tmp, NULL, 16);
	}

	return length/2;
}

static unsigned int ary2hexstring (unsigned char *input, char *output, unsigned int bytelength)
{
	unsigned int i=0;
	char tmp[3];
	output[0] = '\0';
	static char hex_list[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	for (i=0;i<bytelength;i++)
	{
		//sprintf (tmp, "%02x", input[i]); /* need to increase speed */
		unsigned char c = input[i];
		tmp[0] = hex_list[(c >>4)];
		tmp[1] = hex_list[(c & 0xf)];
		tmp[2] = '\0';

		strcat(output,tmp);
	}
	return 0;
}


int encrypt_aes(unsigned char *key_hex_string, unsigned char *data, unsigned int data_len, unsigned char *output)
{
	unsigned char block[MAX_INPUT_LENGTH] = {0};
	unsigned char key[MAX_KEY_LENGTH] = {0};

	int i;

	if(hexstr2array(key_hex_string, key) != (MAX_KEY_LENGTH/2)){
		printf("Key error : Must be a %d-bytes-Hex-String\n",MAX_KEY_LENGTH);
		return -1;
	}

        if(data_len > MAX_INPUT_LENGTH){
                printf("Data error : Max size is %d bytes\n",MAX_INPUT_LENGTH);
                return -1;
        }
	memcpy((void *)block, (void *)data, data_len);

	AES_Init();	
	
	for (i=0;i<MAX_INPUT_LENGTH/MAX_BLOCK_LENGTH;i++)
	{
		AES_Encrypt(block+i*MAX_BLOCK_LENGTH, key, MAX_KEY_LENGTH);
	}

	ary2hexstring(block, output, MAX_INPUT_LENGTH);
	
	AES_Done();

	return MAX_INPUT_LENGTH * 2;
}

int decrypt_aes(unsigned char *key_hex_string, unsigned char *ciphertext_hex_string, unsigned char *output)
{
	unsigned char block[MAX_INPUT_LENGTH] = {0};
	unsigned char key[MAX_KEY_LENGTH] = {0};
	unsigned int  output_len;

	int i;

        if(hexstr2array(ciphertext_hex_string, block) != MAX_INPUT_LENGTH){
                printf("Ciphertext error : Must be a %d-bytes-Hex-String\n",MAX_INPUT_LENGTH*2);
                return -1;
        }

        if(hexstr2array(key_hex_string, key) != (MAX_KEY_LENGTH/2)){
                printf("Key error : Must be a %d-bytes-Hex-String\n",MAX_KEY_LENGTH);
                return -1;
        }

	AES_Init();

	for (i=0;i<MAX_INPUT_LENGTH/MAX_BLOCK_LENGTH;i++)
	{
		AES_Decrypt(block+i*MAX_BLOCK_LENGTH, key, MAX_KEY_LENGTH);
	}

	output_len = strlen(block);

	if(output_len >= MAX_INPUT_LENGTH)
		memcpy(output, block, MAX_INPUT_LENGTH);		
	else
		memcpy(output, block, output_len);
	
	AES_Done();

	return output_len;
}

void generate_random_array (unsigned char * ary, unsigned int length)
{
	unsigned int i = 0;
	
	for (i=0;i<length;i++)
	{
		ary[i] = rand() & 0xff;
	}
	return ;
}


/*******************************************************************************
* Function    : tbsGetSysUpTime
* Description : ȡ��ϵͳ����������ʱ�䣬��λΪ��
* Parameters  :
* Return      : ��
* Author      : wuyun / 2010-01-21
* History     :
*******************************************************************************/
long tbsGetSysUpTime()
{
    struct sysinfo info;
    sysinfo(&info);
    return info.uptime;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// Local variables:
// tab-width: 4
// c-basic-offset: 4
// End:
// vim:tw=78
// vim600: sw=4 ts=4 fdm=marker
// vim<600: sw=4 ts=4
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

