/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称: tbsutil.c
 文件描述: 公共函数的封装，提供给各模块使用

 修订记录:
        1. 作者: all
           日期: 2008-08-07
           内容: 创建文件

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

static int s_itbsSystemAsyn = 0;      /* 异步调用system */
static int s_itbsSystemMID = 0;       /* 异步调用system的MID */
static BOOL g_bFactoryMode = FALSE;


/******************************************************************************
 *                                 字符串处理                                 *
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
*  功能: 允许/禁用当前进程崩溃时生成core文件
*  参数: iFlag  1 - 允许, 0 - 禁用
*  返回: TBS_SUCCESS/TBS_FAILED
*  创建: 匡素文 / 2010-08-17
*********************************************************************/
int tbsEnableCoreDump(int iFlag)
{
    int iRes = RLIMIT_CORE;
    struct rlimit stRlim;

    /* 允许生成core文件 */
    stRlim.rlim_cur = stRlim.rlim_max = iFlag ? RLIM_INFINITY : 0;
    if (0 != setrlimit(iRes, &stRlim))
    {
        printf("Error: setrlimit failed, %s\n", strerror(errno));
        return TBS_FAILED;
    }
    else
    {
        /* 设置core文件生成的路径 */
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
/*  函数名称: tbsMatch                                                     */
/*  函数功能: 字符串匹配函数                                               */
/*  输  入  : pszString 要匹配的字符串                                     */
/*            pszPattern   匹配规则                                        */
/*  输  出  : BOOL   TRUE:匹配成功，FALSE：不匹配                          */
/*  创  建  : tbs / 2008-04-19                                             */
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
功能: 返回当前是否为factory mode
参数: 无
返回:
		true -- 是工厂模式
        false -- 非工厂模式
备注:
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
                /* 工厂模式禁止按键*/
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
功能: 获取当前配置模式，到全局变量g_bFactoryMode，工厂模式则拉起cmddaemon
参数: 无
返回:
		无
备注:
**************************************************************************/
void tbsConfigModeInit(void)
{
    g_bFactoryMode = tbsFactoryMode();

    /*目前只有logic进程中会拉cmddaemon进程*/
    if(g_bFactoryMode)
        tbsSystem("/usr/bin/cmddaemon &", TBS_PRINT_CMD);
}


/*************************************************************************
功能: 设置tbsSystem为异步模式，通过cmddaemon进程执行命令
参数: 无
返回:
		无
备注:
**************************************************************************/
void tbsSetSystemAsyn(unsigned short usMID)
{
    /*只对工厂模式生效*/
    if(!g_bFactoryMode)
        return;

    s_itbsSystemAsyn = 1;
    s_itbsSystemMID = usMID;
    return;
}


/*************************************************************************
功能: 设置tbsSystem为同步模式，不通过cmddaemon进程执行命令
参数: 无
返回:
		无
备注:
**************************************************************************/
void tbsSystemAsynComplete(void)
{
    /*只对工厂模式生效*/
    if(!g_bFactoryMode)
        return;

    s_itbsSystemAsyn = 0;
    s_itbsSystemMID = 0;
    return;
}
#endif

/*=========================================================================*/
/*  函数名称: tbsSystem                                                    */
/*  函数功能: 系统调用system的替代                                         */
/*  输  入  : char* 命令行语句                                             */
/*            int   命令打印标志，0：不打印，其它：打印                    */
/*  输  出  : int   0:执行成功，其它：错误                                 */
/*  创  建  : tbs / 2007-12-14                                             */
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
    /* 异步模式 */
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
 * tbsCheckIpRange: 检查输入的两个IP地址是否在一个范围之内
 * pStartIp: 		起始IP地址
 * pEndIp: 			结束IP地址
 * 返回值:			如果两者在同一个返回之内,返回TBS_SUCCESS, 否则返回TBS_ERROR
 * 备注:			该函数不对IP地址的有效性做检查, 在调用之前最好使用
 					tbsCheckIpEx函数进行检查
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
功能: 从IP字符串中解析ip
参数: const char *pcIp,                 IP字符串;
返回: ip地址
备注:
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
功能: 把ip转为字符串
参数:    unsigned long ulIp      IP地址
                char *pszIp                 IP字符串;
返回: 无
备注:
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
/*  函数名称: tbsSplitString                                               */
/*  函数功能: 根据指定字符分割字符串                                       */
/*  输  入  : str 需要分割的字符串指针                                     */
/*            substr 存放指向分割好的元素指针                              */
/*            delimit 分隔符                                               */
/*            max_count 最大元素个数                                       */
/*  输  出  : void  无                                                     */
/*  创  建  : tbs / 2007-12-14                                             */
/*  修  订  : zoudeming /2007-12-27                                        */
/*            把双重循环改为单循环，重命名了某些形参                       */
/*            zoudeming /2008-1-3                                          */
/*            代码回滚                                                     */
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
功能: 根据制定字符分割列表字符串
      (形如: xx,yy,zz, 分割之后生成数组{xx, yy, zz})
参数: strList               原列表字符串]
      delimit               分割字符
      apcStrArray           存放分割好的元素指针
      nMaxCnt               最大元素个数
返回: 返回列表中实际元素个数
备注:
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
功能: 从一个列表字符串中查找其中一个元素
      (形如: xx,yy,zz, 查找yy)
参数: szList                原列表字符串]
      delimit               分割字符
      szDel                 要查找的元素
返回: 找到这返回字符指针，否则返回NULL
备注:
**************************************************************************/
int tbsStrListFind(const char *szList, char delimit, const char *szEntry)
{
    char *p1, *p2;

    p2 = p1 = (char*)szList;
    while (p2)
    {
        /* 不同的路径之间使用'分隔符 */
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
功能: 往一个列表字符串中添加一个元素
      (形如: xx,yy, 添加zz, 则变成 xx,yy,zz)
参数: szList                原列表字符串]
      delimit               分割字符
      szDel                 要添加的元素
返回: true/false
备注:
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
功能: 从一个列表字符串中删除其中一个元素
      (形如: xx,yy,zz, 删除yy, 则变成 xx,zz)
参数: szList                原列表字符串]
      delimit               分割字符
      szDel                 要删除的元素
返回: true/false
备注:
**************************************************************************/
int tbsStrListDel(char *szList, char delimit, const char *szEntry)
{
  	char *p1, *p2 = NULL;
    int isFound = 0;

	p2 = p1 = szList;
	while (p2)
	{
	    /* 不同的路径之间使用','分隔符 */
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

	            /* 去除结尾的',' */
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
功能: 计算掩码中1的个数,非法掩码则返回负数
参数:    const char *pcMask      掩码字符串
返回: 掩码中1的个数
备注:
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
*   检查函数
*
****************************************************/

/* 检查IP合法性 */
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
    /* 检查MAC地址是否为广播或者全零MAC */
    if (0 == strcasecmp(pcMac, "00:00:00:00:00:00")
       || 0 == strcasecmp(pcMac, "FF:FF:FF:FF:FF:FF")
    )
    {
        return FALSE;
    }
  /*检查MAC地址是否为多播MAC地址*/
  if( arr[0]%2 > 0 )
    return FALSE;
#endif

  /*检查MAC地址是否合法*/
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
函数: tbsCheckRegex
功能: 使用传入的正则表达式匹配要检查的值,不匹配则返回指定的错误码
参数: pszValue      要检查的值
      pszPattern    正则表达式
      iErrNo        错误码
返回: 成功,TBS_SUCCESS
      失败,错误码
*/
int tbsCheckRegex(const char *pszValue, const char *pszPattern, int iErrNo)
{
    if ( !tbsMatch(pszValue, pszPattern) )
        return iErrNo;
    return TBS_SUCCESS;
}

/*
检查IP合法性，允许所有类型ip
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
检查IP合法性，不允许为组播广播等特殊地址
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
      IP范围: 1.*.*.*   -  127.*.*.*,
              128.*.*.* -  224.*.*.*
    */
    if ((ulIp > 0x01000000 && ulIp < 0x7f000000)
        || (ulIp > 0x80000000 && ulIp < 0xe0000000) )
    {
        /* 不能为*.*.*.0 或者 *.*.*.255 */
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
检查DNS IP合法性，不允许为组播广播等特殊地址,允许0.0.0.0的IP存在
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
      IP范围: 1.*.*.*   -  127.*.*.*,
              128.*.*.* -  224.*.*.*
    */
    if ((ulIp > 0x01000000 && ulIp < 0x7f000000)
        || (ulIp > 0x80000000 && ulIp < 0xe0000000) )
    {
        /* 不能为*.*.*.0 或者 *.*.*.255 */
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
*  功能: 检查协议类型是否合法
*  参数: pszValue, 协议字符串
*  返回: const char *
*  备注: 合法值为IPv4, IPv6, IPv4&6
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

    /* 检查MAC地址是否为广播或者全零MAC */

   if (0 == strcasecmp(pcMac, "00:00:00:00:00:00")
         || 0 == strcasecmp(pcMac, "FF:FF:FF:FF:FF:FF")
    )
    {
        return ERR_MAC_INVALID_VALUE;
    }

    /*检查MAC地址是否为多播MAC地址*/
    if( arr[0]%2 > 0 )
        return ERR_MAC_INVALID_VALUE;

    /*检查MAC地址是否合法*/
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
 ip列表格式:ip1,ip2
 ip列表中不能有重复ip
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

        /* 检查重复 */
        for ( j = i+1; j < iCount; j++ )
        {
            if ( strcmp(pszIP[i], pszIP[j]) == 0 )
                return ERR_LIST_HAS_REPEAT;
        }
    }

    return TBS_SUCCESS;
}

/*
 ip列表格式:ip1,ip2
 ip列表中的ip必须是hostip(不能为广播地址,组播地址等其他非法ip)
 ip列表中不能有重复ip
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

        /* 检查重复 */
        for ( j = i+1; j < iCount; j++ )
        {
            if ( strcmp(pszIP[i], pszIP[j]) == 0 )
                return ERR_LIST_HAS_REPEAT;
        }
    }

    return TBS_SUCCESS;
}

/*
 ip列表格式:ip1,ip2
 ip列表中的ip必须是hostip(不能为广播地址,组播地址等其他非法ip)
 ip列表中不能有重复ip,加入判断0.0.0.0的IP,允许这种IP存在
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

        /* 检查重复 */
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

        /* 检查重复 */
        for ( j = i+1; j < iCount; j++ )
        {
            if ( strcmp(apszMac[i], apszMac[j]) == 0 )
                return ERR_LIST_HAS_REPEAT;
        }
    }

    return TBS_SUCCESS;
}

/* 检查用户名/密码 */
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

	/* 允许为空 */
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
	/* 允许为空 */
	if (0 == strlen(pszValue))
	{
		return ERR_INVALID_HOSTNAME_NAME;
	}
	/* 允许主机名用特殊字符*/
	 if (!tbsMatch(pszValue, "^([[:alnum:]-]+)$")|| strlen(pszValue) > 67)
	//if ( strlen(pszValue) > 67)
	{
		return ERR_INVALID_HOSTNAME_NAME;
	}
	

    return TBS_SUCCESS;
}

/* 检查域名或者IP的合法性 */
int tbsCheckHost(const char *pszValue)
{
    BOOL bIsIP = TRUE;

    if (pszValue == NULL)
    {
        return TBS_NULL_PTR;
    }

    /*
      不能仅以开头是否是数字来判断是否为IP，而应
	 当全部由数字和'.'组成，认为是IP地址，防止类似
	 163.com之类的主机名被错认为是IP地址。
	*/
    if (!tbsMatch(pszValue, "^([0-9]{1,3}\\.){3}[0-9]{1,3}$"))
        bIsIP = FALSE;

    /* 是ip地址 */
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
/* 内存分配记录 */
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

/* 取消内存库函数的重定义 */
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



/* 安全释放内存*/
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

            /* 是否只打印新分配的数据 */
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


/* 内存池链表头 */
static struct list_head g_stMemPool = LIST_HEAD_INIT(g_stMemPool);


/*********************************************************************
*  名称: Mem_Malloc
*  功能: 分配内存,并记录
*  参数: nSize          : 要分配内存的大小
*        szFile         : 文件名
*        szFunc         : 函数名
*        szLine         : 行号
*  返回: 成功,分配的内存的指针
*        失败,NULL
*  创建: 匡素文 / 2008-10-15
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

    /* 设置魔术数字 */
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
*  名称: Mem_Calloc
*  功能: 为数组分配内存(会给分配的内存置0),并记录
*  参数: nMemb          : 要分配内存的块数
*        nSize          : 要分配的每块内存的大小
*        szFile         : 文件名
*        szFunc         : 函数名
*        szLine         : 行号
*  返回: 成功,分配的内存的指针
*        失败,NULL
*  创建: 匡素文 / 2008-10-15
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

    /* 设置魔术数字 */
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
*  名称: Mem_Realloc
*  功能: 重新分配内存( 释放/扩展原来分配的内存),并记录
*  参数: pAddr          : 原来的内存指针
*        nSize          : 新分配的内存的大小
*        szFile         : 文件名
*        szFunc         : 函数名
*        szLine         : 行号
*  返回: 成功,分配的内存的指针
*        失败,NULL
*  创建: 匡素文 / 2008-10-15
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

    /* 设置魔术数字 */
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
*  名称: Mem_Free
*  功能: 释放内存,并删除记录
*  参数: pAddr          : 要释放的内存指针
*        szFile         : 文件名
*        szFunc         : 函数名
*        szLine         : 行号
*  返回: void
*  创建: 匡素文 / 2008-10-15
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
*  名称: Mem_FreeAll
*  功能: 释放所有内存, 并删除所有记录
*  参数: void
*  返回: void
*  创建: 匡素文 / 2008-10-15
*********************************************************************/
void Mem_FreeAll(void)
{
    MemList_PrintAll(&g_stMemPool, FALSE );
    MemList_DropAll(&g_stMemPool );
}

/*********************************************************************
*  名称: Mem_PrintStatus
*  功能: 显示分配之后尚未释放的内存情况
*  参数: void
*  返回: void
*  创建: 匡素文 / 2008-10-15
*********************************************************************/
void Mem_PrintStatus(void)
{
#ifdef CONFIG_TBS_APPS_DEBUG_MALLOC
    MemList_PrintAll(&g_stMemPool, FALSE);
#endif
}

/*********************************************************************
*  名称: Mem_PrintStatusOnce
*  功能: 显示分配之后尚未释放的内存情况(已经调用此函数显示过的内存块不再显示)
*  参数: void
*  返回: void
*  创建: 匡素文 / 2008-10-15
*********************************************************************/
void Mem_PrintStatusOnce(void)
{
#ifdef CONFIG_TBS_APPS_DEBUG_MALLOC
    MemList_PrintAll(&g_stMemPool, TRUE );
#endif
}

/*********************************************************************
*  名称: tbsAddMac

*  功能: 增加mac地址，支持LAN和WAN设备的修改

*  参数: pIfName为设备名,如eth0
		 iInterfaceType为MAC_LAN_DEVICE或者MAC_WAN_DEVICE
		 szMacVal  返回给调用者的mac地址, 内存由调用者申请

*  返回: TBS_SUCCESS/TBS_FAILED

*  创建: zhujianwen / 2008-12-25
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
    /* 读取flash的mac地址 */
    if((fp = fopen(PATH_MAC_ADDRESS, "r")) == NULL)
    {
        printf("Error: Fail to open %s \r\n", PATH_MAC_ADDRESS);
        return TBS_FAILED;
    }
    /* 偏移2是因为保留0x */
    if(fgets(szMacHigh, MAX_MAC_ADDR_LEN, fp) == 0)
    {
        printf("Error: Fail to get data from FILE \r\n");
        fclose(fp);
        return TBS_FAILED;
    }
    fclose(fp);

    /* 根据接口类型选择不同的偏移量 */
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

    /* 由于mac地址对应的十六进制数过大,分开高低两部分 */
    strcpy(szMacLow+2, szMacHigh+6);
    memset(szMacHigh+6, 0, MAX_MAC_ADDR_LEN-6);

    /* 分别取高低两部分,转化为十六进制数 */
    ulMacLow = strtol(szMacLow, NULL, 16);

    /* 如果加上偏移量超出范围, 进位处理 */
    if((ulMacLow+iMacOffSet) > MAX_VALUE_OF_HEX)
    {
            ulMacLow = ulMacLow + iMacOffSet - CYCLE_VALUE_OF_HEX;
    }
    else
    {
            ulMacLow += iMacOffSet;
    }

    /* 将' '变为0 */
    memset(szMacLow, 0, MAX_MAC_ADDR_LEN);
    sprintf(szMacLow, "%6lx", ulMacLow);
    for(i=0; i<6; i++)
    {
        if(szMacLow[i] == ' ')
            szMacLow[i] = '0';
    }

	memset(szMacVal, 0, MAX_MAC_ADDR_LEN);
	snprintf(szMacVal, MAX_MAC_ADDR_LEN, "%s%s", szMacHigh, szMacLow);
    /* 写mac地址到驱动里 */
    snprintf(szCmd, SYS_CMD_LEN,
             "ifconfig %s down;ifconfig %s hw ether %s%s;ifconfig %s up",
             pIfName, pIfName, szMacHigh, szMacLow, pIfName);

    return tbsSystem(szCmd, TBS_PRINT_CMD);
}

/* DNS解析相关的API及结构体,宏的定义 */
#define REG_MATCH_LEN  5

/*********************************************************************
 *  名称: tbsSplitUrl

 *  功能: 解析传入的域名, 分离出真正需要解析的部分以及端口号, 如果是
 IP地址则直接返回

 *  参数:
    pHostName:	需要解析的域名
    pPort:		端口号
    pPath:		路径

 *  返回:
    SPLIT_SUCCESS: 解析成功, 此时pHostName存放的是解析后的域名地址
    pPort存放的是端口号
    SPLIT_ERROR:	解析失败
    SPLIT_IP:		传入的URL是IP地址, 直接返回

 *  创建: lichuang (2009-04-01)
 *********************************************************************/
int tbsSplitUrl(const char *pUrl, char *pHostname, char *pPort, char *pPath)
{
	size_t          nLen = 0;
	regex_t         stRegex;             /* 存储编译好的正则表达式，正则表达式在使用之前要经过编译 */
	regmatch_t      szRegMatch [REG_MATCH_LEN];     /* 存储匹配到的字符串位置 */
	int             nResult = 0;
    char            szHost[256] = {0};

    /* szUrlPattern字符串 */
	char szUrlPattern[] = "^((http|https)://)?([^:/]*)(:[0-9]+)?(.*)";

#ifdef CONFIG_TBS_SUPPORT_IPV6
    char szIp6Pattern[] = "^((http|https)://)?\\[([^]]*)\\](:[0-9]+)?(.*)";
    int isIPv6 = 0;

    /* ipv6 形式的url地址 */
    if (strstr(pUrl, "http://[")
        || strstr(pUrl, "https://[")
        || (pUrl[0] == '[' && strchr(pUrl, ']'))
    )
    {
        isIPv6 = 1;
        /* 编译正则表达式 */
    	if (0 != regcomp(&stRegex, szIp6Pattern, REG_EXTENDED))
    	{
    		return SPLIT_ERROR;
    	}
    }
    else
#endif
    {
    	/* 编译正则表达式 */
    	if (0 != regcomp(&stRegex, szUrlPattern, REG_EXTENDED))
    	{
    		return SPLIT_ERROR;
    	}
    }

	/* 执行模式匹配 */
	if (0 != regexec(&stRegex, pUrl, (size_t) REG_MATCH_LEN, szRegMatch, 0))
	{
		regfree(&stRegex);
		return SPLIT_ERROR;
	}
	nResult = SPLIT_SUCCESS;

	/* 首先得到端口号, 它在szUrlPattern中的第4个子表达式, 因此索引从4开始 */
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

	/* 其次得到域名, 它在szUrlPattern中的第3个子表达式, 因此索引从3开始 */
	if (-1 != szRegMatch[3].rm_so)
	{
		nLen = szRegMatch[3].rm_eo - szRegMatch[3].rm_so;
	    strncpy(szHost, pUrl + szRegMatch[3].rm_so, nLen);
	    szHost[nLen] = '\0';

#ifdef CONFIG_TBS_SUPPORT_IPV6
        /* 符合ipv6 url格式，检测是不是合法的ip地址 */
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
            /* 比较它是不是IPv4地址格式 */
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

    /* 最后得到路径,第5个表达式 */
    if (-1 != szRegMatch[5].rm_so)
	{
        if (pPath)
        {
    		nLen = szRegMatch[5].rm_eo - szRegMatch[5].rm_so;
    	    strncpy(pPath, pUrl + szRegMatch[5].rm_so, nLen);
    	    pPath[nLen] = '\0';
        }
	}

	regfree(&stRegex);   /* 用完了别忘了释放 */
	return nResult;
}

/*********************************************************************
 *  名称: SetDefaultDns

 *  功能: 进行DNS解析之前指定解析用的DNS服务器地址,如果不调用该函数,
 将使用系统默认的DNS服务器

 *  参数:
pServer: DNS服务器地址

 *  返回:

 *  创建: lichuang (2009-04-01)
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
 *  名称: __ResolveIpByName

 *  功能: 对传入的域名进行解析, 解析结果放在ST_DNS_INFO链表中

 *  参数:
pHostName: 需要解析的域名

 *  返回: 0/-1

 *  创建: lichuang (2009-04-01)
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
*  名称: tbsResolveIpByName
*  功能: 对传入的域名进行解析, 解析结果放在ST_DNS_INFO链表中, 根据dns的类型分别解析IPV4/IPV6地址
*  参数:
		pszUrl:     需要解析的域名
		ppDnsInfo:  存放解析结果的链表元素指针, 使用完毕之后使用者要
                    使用tbsFreeDnsInfo函数释放
		pPort:      存放解析之后的端口号
		nMsecond:	超时用的毫秒数
		pServer:    解析DNS的server地址, 可以为空, 为空则使用系统的地址

*  返回: TBS_SUCCESS/TBS_FAILED
*  创建: lichuang (2009-04-01)
*  备注:
         1) 如果该函数调用成功, 需要调用者自己去释放ppDnsInfo的内存
            但是如果调用失败, 则不需要做这些,好比malloc失败之后的内存不需要free一样

         2) 如果URL中没有端口号,那么根据协议设置默认端口，如: http->80, https->443
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

		/* 每次循环usleep100微妙 */
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
 *  名称: tbsFreeDnsInfo

 *  功能: 遍历ST_DNS_INFO链表, 释放里面的所有元素

 *  参数:
pDnsInfo:  需要释放的链表元素指针

 *  返回:

 *  创建: lichuang (2009-04-01)
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
*  名称: tbsResolveURL
*  功能: 对传入的域名使用指定的接口和dns进行解析, 并输出解析结果
*  参数:
		pszUrl:         需要解析的域名
		pszIfName:      解析数据包的出口设备
		pszGateway:     出口设备的默认网关
		pszDnsServers:  解析域名使用的dns服务器列表(用逗号分隔)
		pcIpAddr:       存放解析出来的IP地址(out)
		pcPort:         存放解析出来的端口(out)

*  返回: TBS_SUCCESS/TBS_FAILED
*  创建: kuangsuwen (2010-05-25)
*  备注:
         1) 如果传入的出口设备、默认网关、dns服务器列表为空，则使用系统默认路由和dns去解析
         2) 如果URL中没有端口号,那么根据协议返回默认端口，如: http->80, https->443
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

    /* 通过指定的接口和dns去尝试解析域名 */
    if (safe_strlen(pszDnsServers) && safe_strlen(pszGateway) && safe_strlen(pszGateway))
    {

        #ifdef CONFIG_TBS_SUPPORT_IPV6
        /* 判断是否使用ipv6解析域名 */
        bIsIPv6 = (TBS_SUCCESS == tbsCheckIpv6Addr(pszGateway));
        #endif

        /* 分解DNSServers */
        strcpy(acDNSServers, pszDnsServers);
        nCnt = tbsStrListSplit(acDNSServers, ',', apcNameServer, MAX_DNS_COUNT);
        if (nCnt > 3)
        {
            printf("Error: ip_addr count error\n");
            return TBS_FAILED;
        }

        /* 分别使用不同的dns服务器，去解析传入的URL */
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
    /* 使用系统默认接口和dns去解析域名 */
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

    /* 判断是否解析成功 */
    if (!bIsSuccess)
    {
        printf("Resolve failed by all Dns server.\n");
        return TBS_FAILED;
    }

    /* 返回解析成功的IP和端口 */
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
*  名称: Strtok_r

*  功能: 线程安全的strtok函数的实现, 代码来自net-snmp-5.4.1项目

*  参数:
	pString: 需要分析的字符串, 第一次调用的时候需要传入需要解析的字符串
			 后续传入NULL即可
	pDelim:  分隔字符串
	ppLast:  保存上一次解析结果的指针

*  返回:
			如果是NULL则找不到所分隔的字符串了, 否则是已经分隔好的字符串

*  创建: lichuang (2009-04-01)
*********************************************************************/
char* Strtok_r(char *pString, const char *pDelim, char **pLast)
{
	const char *pSpanp;
	int c, sc;
	char *pTok;

	if (pString == NULL && (pString = *pLast) == NULL)
		return (NULL);

	/*
	 * 在pString中查找第一个非分隔符中元素的位置
	*/
cont:
	c = *pString++;
	for (pSpanp = pDelim; (sc = *pSpanp++) != 0;)
	{
		if (c == sc)
			goto cont;
	}

	/*
	 * 如果没找到, 直接返回
	*/
	if (c == 0)
	{
		*pLast = NULL;
		return (NULL);
	}

	/*
	 * 回退一个位置, 该位置是后面要返回的指针
	*/
	pTok = pString - 1;

	/*
	 * 从当前位置开始查找第一个分隔符中元素的位置, 将这个位置置0然后返回
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
功  能: 根据ethernet WAN接口名称获取接口的协商速率(单位Mbps)

参  数: pszDevName -- ethernet WAN接口名称(如eth0)

返回值: ethernet WAN接口的协商速率

备  注: created by XuXiaodong 2009-3-18
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
功  能: 根据ppp接口名(pppoe)获取与该ppp接口相关联的以太网接口名
参  数: pszPppDevName -- ppp接口名
        pszPhyDevName -- 返回时存放与ppp接口相关联的物理接口名
        iLen -- pszPhyDevName指向的缓冲区大小
返回值: 成功 -- TBS_SUCCESS
        失败 -- 其他
备  注: created by XuXiaodong 2010-4-14
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
功  能: url检查
参  数: pValue  URL检查
返回值: TBS_SUCCESS/其他错误码
备  注:
*************************************************************************/
int tbsCheckUrl(const char *pValue)
{
    char *pattern ="^(http://|https://)?(([0-9a-zA-Z_!~*'().&=+$%-]+: )?[0-9a-zA-Z_!~*'().&=+$%-]+@)?(([0-9]{1,3}\\.){3}[0-9]{1,3}|([0-9a-zA-Z][0-9a-zA-Z-]{0,61})(\\.[a-zA-Z0-9][-a-zA-Z0-9]{0,61})*)((:[0-9]{1,4})|(:[1-5][0-9]{1,4})|(:6[0-4][0-9]{3})|(:65[0-4][0-9]{2})|(:655[0-2][0-9])|(:6553[0-5]))?((/?)|(/[0-9a-zA-Z_!~*'().;?:@&=+$,%#-]+)+/?)$";

    /* URL不能为空 */
    if(safe_strlen(pValue) == 0)
    {
        return ERR_CAN_NOT_EMPTY;
    }

    /* URL不能过长 */
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

    pstMember = *ppstApMemberList;  /* ppstApMemberList 不能为 NULL */
    while(pstMember)
    {
        if (pstMember->ucMID == ucMID)
            break;
        pstMember = pstMember->pstNext;
    }

    /* 已经记录了模块禁用 apfeature 功能 */
    if (pstMember)
        return 0;

    /* 记录模块禁用 apfeature 功能 */
    pstNewMember = (ST_APFEATURE_MEMBER_LIST *)malloc(sizeof(ST_APFEATURE_MEMBER_LIST));
    if (NULL == pstNewMember)
    {
        fprintf(stderr, "ERROR: failed to record apfeature member, Out of memory\n");
        return -1;
    }

    memset(pstNewMember, 0, sizeof(ST_APFEATURE_MEMBER_LIST));
    pstNewMember->ucMID = ucMID;

    /* 加入列表 */
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

    pstMember = *ppstApMemberList;  /* ppstApMemberList 不能为 NULL */
    while(pstMember)
    {
        if (pstMember->ucMID == ucMID)
            break;

        pstMemberPrev = pstMember;
        pstMember = pstMember->pstNext;
    }

    /* 没有模块禁用 apfeature 功能的记录 */
    if (!pstMember)
        return 0;

    /* 从列表中删除 */
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

    pstMember = *ppstApMemberList;  /* ppstApMemberList 不能为 NULL */
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
    int iModuleCount = 0; /* 禁用 AP 加速功能模块的计数 */
    char szCommand[100];

    /* 增加/删除禁用了 apfeature 功能的模块记录 */
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

/* 启用 IKANOS AP 加速功能 */
int tbsApfeatureEnable(unsigned short ucMID)
{
    return tbsApfeatureControl(ucMID, 1);
}

/* 禁用 IKANOS AP 加速功能 */
int tbsApfeatureDisable(unsigned short ucMID)
{
    return tbsApfeatureControl(ucMID, 0);
}
#endif

void tbsClearRamForUpgrade(int iUpgradeFlag)
{
	/* kill掉不必要的进程，这里发送16信号给PC */
	tbsSystemMute("killall -16 pc");

     //sleep(1);

	/* 清空iptables和ebtales的所有规则 */
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
		/* 对tr069，可以down掉LAN端接口 */
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
*  功能: 让进城变为守护进程
*  参数: closefd    是否关闭所有文件描述符
*  返回: void
*  创建: 匡素文 / 2009-7-22
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

/* 原字符串转为16进制字符串，如"0!k" 转为 "30216b"
	使用时注意，szDecHex必须大于2倍的i2Num */
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
*  功能: 字符串替换功能函数
*  参数:
*  返回: const char *
*  创建: huangjidong
*  备注: 返回的字符串需要用户自行释放
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
*  功能: 字符串替换功能函数(替换原有字符串)
*  参数:
*  返回: const char *
*  创建: huangjidong
*  备注: 需要保证原有字符串的空间大于在替换字符之后的字符串长度
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
功能: 通过接口名获取接口对应的MAC地址
参数: pszIfName  存放接口名的缓冲区指针(输入)
      pszValue   存放MAC地址的缓冲区指针(输出)
返回: 成功 -- TBS_SUCCESS
      失败 -- 其他错误码
备注: 调用该函数有缓冲区溢出的风险,调用者应该为pszValue指向的缓冲区分配
      足够的空间来存放MAC地址
***************************************************************************/
int tbsGetMacAddr(const char *pszIfName, char *pszMac)
{
    struct ifreq stIfr;
    int iSockfd;

    strcpy(stIfr.ifr_name, pszIfName);

    /* 创建套接字失败*/
    if ((iSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return TR069_ERRNO_CPE_ERR;
    }

    if (ioctl(iSockfd, SIOCGIFHWADDR, &stIfr) < 0)
    {
        close(iSockfd);
        return TR069_ERRNO_CPE_ERR;
    }

    /* 获取MAC地址*/
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
从指定的文件中查找字符串
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
将字符串写入到指定的字符串中
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
* Description : 取得系统启动以来的时间，单位为秒
* Parameters  :
* Return      : 无
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
功能: 基于64位加密方式的字符串加密函数
参数: *value:加密字符串
        vlen:加密长度
返回: result: 加密后的字符串
备注: 调用此函数请注意在外部free返回指针
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
    /* 请注意，此指针需要在外部free */
	return result;
}

/******************************************************************
功能: 基于64位加密方式的字符串解密函数
参数: *value:加密字符串
        vlen:解密后长度
返回: result: 解密后的字符串
备注: 调用此函数请注意在外部free返回指针
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
功能: 把字符串转换成Base64编码，以免不支持的字库保存到配置中，导致配置异常
参数: pStrIn : 输入字符串,该字段内存空间要求是可写的，且预分配的空间足够存放
			 转换后的字符(原4/3长度)。
	 pStrOut : 输出字符串
返回: 成功TBS_SUCCESS，失败TBS_FAILED
备注:
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
功能: 把字符串从Base64编码转换回来
参数: pStrIn : 输入字符串，该字段内存空间要求是可写的。转换后的长度不会比原串长。
	 pStrOut : 输出字符串
返回: 成功TBS_SUCCESS，失败TBS_FAILED
备注:
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
* Description : 取得系统启动以来的时间，单位为秒
* Parameters  :
* Return      : 无
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

