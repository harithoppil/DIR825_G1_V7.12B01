/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : cfgfile.c
 文件描述 : 实现文件读写配置保存恢复等功能
 函数列表 :

 修订记录 :
          1 创建 : 陈跃东
            日期 : 2007-10-15
            描述 :

**********************************************************************/

#include <sys/stat.h>
#include <zlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "cfg_api.h"
#include "cfg_file.h"
#include "flash_layout.h"
#include "cfg_comm.h"
#include "autoconf.h"
#include "common_func.h"
#include "tbstype.h"
#include "tbsutil.h"

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif

#if 0
//#ifdef _TBS_ENV_PC

/*lint -save -e652*/
int pc_app_item_get( void *data , char *item_name ,unsigned short *len );
int pc_app_item_save( void *data , char *item_name ,unsigned short len );

#define app_item_save pc_app_item_save
#define app_item_get  pc_app_item_get
/*lint -restore*/
#endif


/*
  还原出厂默认配置之后，不再允许写flash
  只能重启后再配置 add by pengyao 20121015
*/
int g_bResetedFlag = 0;


/* 存放配置区名称 */
STATIC char s_acCfgFileName[CFG_FILE_NAME_LEN] = TBS_APP_CFG;
STATIC char s_acCfgItemName[CFG_FILE_NAME_LEN]  = TBS_APP_CFG;
char s_acCfgConfigFile[CFG_FILE_NAME_LEN]       = CFG_DEF_FILE;
char s_acCfgFullConfigFile[CFG_FILE_NAME_LEN]   = CFG_FULL_FILE;

#ifdef _CFG_BYTE_NEED_CVT
#define LONG_CVT(long)  long = htonl(long)
#else
#define LONG_CVT(long) (void)0
#endif

/*************************************************************************
Function:      cfg_item_save(const char *data , const char *item_name, unsigned long len)
Description:   保存配置item的封装接口
Calls:         无
Data Accessed:
Data Updated:
Input:         data, 要写入的数据
               item_name, item 的名称
               len, data的长度
Output:
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET cfg_item_save(const char *data , const char *item_name, unsigned long len)
{
    CFG_RET ret = CFG_OK;

#ifndef _CFG_UML
    ret = (CFG_RET)app_item_save((void *)data, (char *)item_name, (unsigned short)len);
    if (CFG_OK != ret)
    {
        CFG_ERR(ERR_CFG_ITEM_OPT_FAIL, "%lu %s %lu", ret, item_name, len);
        return ERR_CFG_ITEM_OPT_FAIL;
    }
#endif
    return ret;
}

/*************************************************************************
Function:      CFG_RET cfg_item_get(char *data , const char *item_name, unsigned long *len)
Description:   读取配置item的封装接口
Calls:         无
Data Accessed:
Data Updated:
Input:         item_name, item 的名称
               len, data的可写长度
Output:        data, 输出读取到的数据
               len , 实际读取到的长度
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET cfg_item_get(char *data , const char *item_name, unsigned long *len)
{
    unsigned short usLen = (unsigned long)(*len);
    CFG_RET ret = CFG_OK;

    ret =  (CFG_RET)app_item_get((void *)data, (char *)item_name, &usLen);
    *len = usLen;
    if (CFG_OK != ret)
    {
        CFG_ERR(ERR_CFG_ITEM_OPT_FAIL, "%lu %u", ret, usLen);
        return ERR_CFG_ITEM_OPT_FAIL;
    }

    return CFG_OK;
}

/*************************************************************************
Function:      CFG_RET CFG_ReadItem(const char *item_name, char **ppcBuf, unsigned long *pulLen)
Description:   读取配置item的封装接口, 内存由函数内分配
Calls:         无
Data Accessed:
Data Updated:
Input:         item_name, item 的名称
Output:        ppcBuf, 输出实际分配到的内存地址
               pulLen , 实际读取到的长度
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_ReadItem(const char *item_name, char **ppcBuf, unsigned long *pulLen)
{
    unsigned long ulLen = CFG_COMM_MTD_LEN;
    char *pcBuf = NULL;
    CFG_RET ret = CFG_OK;

    /* 分配长度 */
    pcBuf = malloc(ulLen);
    if (NULL == pcBuf)
    {
        CFG_ERR(ERR_CFG_MALLOC_FAIL);
        return ERR_CFG_MALLOC_FAIL;
    }

    /* 读取 */
    ret = cfg_item_get(pcBuf, item_name, &ulLen);
    if (CFG_OK != ret)    /* 失败 */
    {
        free(pcBuf);

        /* 重新分配 */
        pcBuf = malloc(ulLen);
        if (NULL == pcBuf)
        {
            CFG_ERR(ERR_CFG_MALLOC_FAIL);
            return ERR_CFG_MALLOC_FAIL;
        }

        /* 读取 */
        ret = cfg_item_get(pcBuf, item_name, &ulLen);
        if (CFG_OK != ret) /* 失败 */
        {
            free(pcBuf);
            CFG_ERR(ret, "cfg_item_get(%s) fail\n", item_name);

            return ret;
        }
    }

    *ppcBuf = pcBuf;
    *pulLen = ulLen;

    return CFG_OK;
}

#define CFG_ITEM_WRITE(pcFile, pcBuf, ulLen) \
{ \
    if (pcFile == s_acCfgFileName \
        || 0 == strcmp(TBS_APP_KEY_TREE, pcFile) \
        || 0 == strcmp(TBS_APP_KEY_CONN, pcFile)) \
    { \
        CFG_RET ret = CFG_OK; \
 \
        ret = cfg_item_save(pcBuf, pcFile, (unsigned long)ulLen); \
        if (CFG_OK != ret) \
        { \
            CFG_ERR(ret, "%lu", ret); \
        } \
        return ret; \
    } \
}

#define CFG_ITEM_READ(pcFile, ppcBuf, pulLen) \
{ \
    if (pcFile == s_acCfgFileName \
        || 0 == strcmp(TBS_APP_KEY_TREE, pcFile) \
        || 0 == strcmp(TBS_APP_KEY_CONN, pcFile)) \
    { \
        return CFG_ReadItem(pcFile, ppcBuf, pulLen); \
    } \
}


/*************************************************************************
Function:      long CFG_ReadFile(const char *pcFile, char *pcBuf,
                                             unsigned long *pulen)
Description:   从文件中读出所有内容
Calls:         无
Data Accessed:
Data Updated:
Input:         pcFile, 要读取的文件名
Output:        ppcBuf, 输出读取的内容, 如果 * ppcBuf == NULL, 则表示内存由函数内部分配
               pulen, 输出读取出的长度, 初始输入 pcBuf 中可写入的长度
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_ReadFile(const char *pcFile, char **ppcBuf,
                     unsigned long ulPrevSpace, unsigned long ulPostSpace,
                     unsigned long *pulLen)
{
    long lFd = 0;
    struct stat stStat;
    unsigned long ulLen = 0;
    char *pcBuf = NULL;
    char *pcBufBegin = NULL;

    CFG_ITEM_READ(pcFile, ppcBuf, pulLen);

    stStat.st_size = 0;
    lFd = open(pcFile, O_RDONLY);
    if (lFd < 0)
    {
        CFG_ERR(ERR_CFG_FILE_OPEN);
        return ERR_CFG_FILE_OPEN;
    }

    /* 表示需要函数里面分配 */
    if (NULL == *ppcBuf)
    {
        (void)fstat(lFd, &stStat);
        ulLen = (unsigned long)(stStat.st_size);
        pcBufBegin = malloc(ulLen + ulPrevSpace + ulPostSpace);
        if (NULL == pcBufBegin)
        {
            close(lFd);

            CFG_ERR(ERR_CFG_MALLOC_FAIL);
            return ERR_CFG_MALLOC_FAIL;
        }
        pcBuf = pcBufBegin + ulPrevSpace;
    }
    else
    {
        pcBuf = *ppcBuf + ulPrevSpace;
        ulLen = *pulLen;
    }

    ulLen = (unsigned long)read(lFd, pcBuf, ulLen);
    close(lFd);
    if ((unsigned long)(-1) == ulLen)
    {
        free(pcBufBegin);

        CFG_ERR(ERR_CFG_FILE_READ);
        return ERR_CFG_FILE_READ;
    }

    if (NULL == *ppcBuf)
    {
        if (ulLen != (unsigned long)(stStat.st_size))
        {
            free(pcBufBegin);

            CFG_ERR(ERR_CFG_FILE_READ);
            return ERR_CFG_FILE_READ;
        }

        *ppcBuf = pcBufBegin;
    }
    *pulLen = ulLen;

    return CFG_OK;

}


/*************************************************************************
Function:      long CFG_WriteFile(const char *pcFile, const char *pcBuf,
                                                    unsigned long ulLen)
Description:   写将指定内容写入文件
Calls:         无
Data Accessed:
Data Updated:
Input:         pcFile, 要写入的文件名
               pcBuf, 要写入的内容
               ulLen, 要写入的长度
Output:
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_WriteFile(const char *pcFile, const char *pcBuf, unsigned long ulLen)
{
    long lFd = 0;
    long lOffset = 0;
    CFG_RET ret = CFG_OK;

    CFG_ITEM_WRITE(pcFile, pcBuf, ulLen);

    lFd = open(pcFile, O_WRONLY | O_CREAT | O_TRUNC, CFG_WRITE_MOD);
    if ( 0 > lFd )
    {
        int err = errno;
        if (EACCES == err)
        {
            ret = ERR_FILE_NOT_ALLOWED;
        }
        else if ( ENOSPC == err)
        {
            ret = ERR_FILE_WTITE_NOSPACE;
        }
        else
        {
            ret = ERR_FILE_OPEN_UNKOWN;
        }

        CFG_ERR(ret, "open(%s) fail\n", pcFile);

        //CFG_ERR(ret);
        return ret;
    }

    while (ulLen > 0)
    {
        lOffset = write(lFd, pcBuf, ulLen);
        if (lOffset == -1 )
        {
            int err = errno;
            if (EACCES == err)
            {
                ret = ERR_FILE_NOT_ALLOWED;
            }
            else if ( ENOSPC == err)
            {
                ret = ERR_FILE_WTITE_NOSPACE;
            }
            else
            {
                ret = ERR_FILE_WRITE_UNKOWN;
            }

            close(lFd);

            CFG_ERR(ret);
            return ret;
        }
        else if (lOffset < (long)ulLen )
        {
            pcBuf += lOffset;
        }
        else
        {
            ;
        }
        ulLen -= lOffset;
    }

    close(lFd);
    return CFG_OK;
}


/* 读取当前配置 */
/*************************************************************************
Function:      long CFG_ReadCurCfg(char *pcCfg, unsigned long *pulLen)
Description:   读取当前配置
Calls:         无
Data Accessed:
Data Updated:
Input:         无
Output:        pcCfg, 输出缓冲区,
               pulLen, 输出配置实际长度, 初始输入 pcCfg 中可写的长度
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/

#define XML_CTL_HEAD "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"
#define XML_CTL_HEAD_LEN (strlen("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"))


CFG_RET CFG_ReadCurCfg(const char *pcItem, char **ppcBuf, unsigned long ulPrevSpace,
                     unsigned long ulPostSpace, unsigned long *pulLen)
{
    ST_CFG_FILE *pstCfgFile = NULL;
    unsigned long ulLen = 0;
    CFG_RET ret = CFG_OK;
    unsigned long ulCalCrc = 0;
    char *pcCfg = NULL;
    char *pcBufBegin = NULL;

    if (NULL == pcItem)
    {
#ifdef CONFIG_APPS_LOGIC_WANSELECT

        unsigned long ulBufLen = CFG_FILE_NAME_LEN;

        ret = cfg_item_get(s_acCfgFileName, TBS_USERCFG_ITEM, &ulBufLen);
        if (CFG_OK != ret)
        {
            sprintf(s_acCfgFileName, TBS_USERCFG_PREFIX"%d", 1);
        }

        //printf("Reading config from %s area...\n", s_acCfgFileName);
#endif
        pcItem = s_acCfgFileName;
    }

    /* 从配置区中读取文件信息 */
    ret = CFG_ReadFile(pcItem, (char **)(void *)&pstCfgFile, 0, 0, &ulLen);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
        return ret;
    }

    LONG_CVT(pstCfgFile->ulMagicNum);
    LONG_CVT(pstCfgFile->ulUncompLen);
    LONG_CVT(pstCfgFile->ulCompLen);
    LONG_CVT(pstCfgFile->ulCrc);

    /* 检测数据是否有效 */
    if (MAGICNUM != pstCfgFile->ulMagicNum)
    {
        free(pstCfgFile);
        return CFG_FAIL;
    }

	/* 如果数据大于64K则认为非法文件 */
    if (pstCfgFile->ulCompLen > 65536)
    {
        free(pstCfgFile);
        return CFG_FAIL;
    }

    /* CRC 校验 */
    ulCalCrc= crc32(0, (unsigned char *)pstCfgFile->acData, pstCfgFile->ulCompLen);
    if (pstCfgFile->ulCrc != ulCalCrc)
    {
        free(pstCfgFile);

        CFG_ERR(ERR_CFG_FILE_CHKSUM);
        return ERR_CFG_FILE_CHKSUM;
    }

    /* 分配内存 */
    ulLen = pstCfgFile->ulUncompLen;
    pcBufBegin = malloc(ulLen + ulPrevSpace + ulPostSpace);
    if (NULL == pcBufBegin)
    {
        free(pstCfgFile);

        CFG_ERR(ERR_CFG_MALLOC_FAIL);
        return ERR_CFG_MALLOC_FAIL;
    }
    pcCfg = pcBufBegin + ulPrevSpace;

    //add encodeing control header
    sprintf(pcCfg, "%s", XML_CTL_HEAD );
    pcCfg += XML_CTL_HEAD_LEN;


    /* 解压缩 */
    ret = uncompress(pcCfg, &ulLen, pstCfgFile->acData, pstCfgFile->ulCompLen);
    free(pstCfgFile);

    if (CFG_OK != ret /*|| ulLen != pstCfgFile->ulUncompLen */)
    {
        free(pcBufBegin);
        CFG_ERR(ret);
        return ret;
    }

    *ppcBuf = pcBufBegin;
    *pulLen = ulLen + XML_CTL_HEAD_LEN;

#ifdef _CFG_DEBUG
    (void)CFG_WriteFile(CFG_DEBUG_FILE, pcCfg, ulLen);
#endif

    return ret;
}

CFG_RET CFG_HoldKeyValWhenRecover(void);
CFG_RET CFG_LocalRecover(void);
CFG_RET CFG_RemoteRecover(void);

STATIC FUNC_CFG_SPEC_RECOVER s_pfnCfgSpecRecover = NULL;

/* 恢复出厂配置 */
/*************************************************************************
Function:      long CFG_RecoverFactCfg(void)
Description:   恢复出厂配置
Calls:         CFG_ReadFile
               CFG_SaveCurCfg
Data Accessed:
Data Updated:
Input:         无
Output:        配置取的内容被回复到出厂默认配置的内容
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_RecoverFactCfg(const char *pcAccessor)
{
    CFG_RET ret = CFG_OK;

#ifdef CONFIG_APPS_LOGIC_WANSELECT

        unsigned long ulBufLen = CFG_FILE_NAME_LEN;

        ret = cfg_item_get(s_acCfgFileName, TBS_USERCFG_ITEM, &ulBufLen);
        if (CFG_OK != ret)
        {
            sprintf(s_acCfgFileName, TBS_USERCFG_PREFIX"%d", 1);
        }

        //printf("Clearing config at %s area...\n", s_acCfgFileName);
#endif

    /* 清除配置 */
    ret = CFG_ClearCurCfg();
	
	/* 设置首页跳到恢复出厂页面 add by hujian 2016-3-4*/
   ret = (int)app_item_save("1", TBS_WiZARD_FLAG, 2);
   if (CFG_OK != ret)
	   printf("Write wizard item failed !\n");
#ifdef CONFIG_APPS_SSAP_HOLD_KEY_PARA
    if (NULL != pcAccessor && '\0' == pcAccessor[0])
    {
        s_pfnCfgSpecRecover = CFG_HoldKeyValWhenRecover;
    }
    else
    {
        s_pfnCfgSpecRecover = CFG_LocalRecover;
    }
#endif

    /*清除sitecom 自动升级配置条目*/
#ifdef CONFIG_APPS_LOGIC_AUTOUPGRADE
    char szLanIp[] = {"192.168.0.1"};
    char szLanIp6Ip[] = {"fe80::1"};
    char szFwOnServer[] = {"\0"};
    char szFwVersion[] = {"\0"};
    char szFwAction[] = {"0"};

    app_item_save((void*)szLanIp, SITCOM_LAN_IP, strlen(szLanIp));
    app_item_save((void*)szLanIp6Ip, SITCOM_LAN_IPV6, strlen(szLanIp6Ip));
    app_item_save((void*)szFwOnServer, SITCOM_FW_ONSERVER, 1);
    app_item_save((void*)szFwVersion, SITCOM_FWVER, 1);
    app_item_save((void*)szFwAction, SITECOM_FWACTION, strlen(szFwAction));
#endif
    if (NULL != s_pfnCfgSpecRecover) /* 保留关键参数回调接口不为空 */
    {
        /* 调用之 */
        ret = s_pfnCfgSpecRecover();
    }
    else
    {
        /* 清除配置 */
        ret = CFG_ClearCurCfg();
    }

    /*
      Set Reset flag = 1,不允许再写配置
     只能重启后再配置 add by pengyao 20121015
    */
    //printf("=======================in %s:%d, set g_bResetedFlag = 1\n", __FILE__, __LINE__);
    g_bResetedFlag = 1;

    return ret;
}


/* 保存当前配置 */
/*************************************************************************
Function:      long CFG_SaveCurCfg(char *pcCfg, unsigned long ulLen)
Description:   保存当前配置
Calls:         CFG_WriteFile
Data Accessed:
Data Updated:
Input:         pcCfg, 配置文件内容
               ulLen, 配置文件的长度
Output:
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_SaveCurCfg(const char *pcItem, char *pcCfg, unsigned long ulLen)
{
    ST_CFG_FILE *pstCfgFile = NULL;
    unsigned long ulFileLen = CFG_MAX_MTD_LEN;
    CFG_RET ret = CFG_OK;
    unsigned long ulComLen = 0;

    //printf("=======================in %s:%d, CFG_SaveCurCfg, g_bResetedFlag = %d, pcItem=%s\n",
    //    __FILE__, __LINE__, g_bResetedFlag, pcItem);

    /*
     已经还原成出厂默认设置，则不再允许保存配置
     只能重启后再配置 add by pengyao 20121015
    */
    if(g_bResetedFlag
        && (NULL == pcItem || 0 == strncmp(pcItem, s_acCfgFileName, sizeof(s_acCfgFileName))
        || 0 == strncmp(pcItem, s_acCfgItemName, sizeof(s_acCfgItemName))))
    {
        printf("~~~~~Reseted unable save Cfg~~~~\n");
        return TBS_SUCCESS;
    }

    /* 分配内存 */
    pstCfgFile = malloc(ulFileLen);
    if (NULL == pstCfgFile)
    {
        CFG_ERR(ERR_CFG_MALLOC_FAIL);
        return ERR_CFG_MALLOC_FAIL;
    }
    pstCfgFile->ulCompLen = (ulFileLen - sizeof(ST_CFG_FILE));

    /* 压缩 */
    ret = compress(pstCfgFile->acData, &pstCfgFile->ulCompLen, pcCfg, ulLen);
    if (CFG_OK != ret)
    {
        free(pstCfgFile);

        CFG_ERR(ret);
        return ret;
    }

    /* 计算crc */
    pstCfgFile->ulCrc = crc32(0, pstCfgFile->acData, pstCfgFile->ulCompLen);

    /* 按文件格式填充数据 */
    pstCfgFile->ulMagicNum = MAGICNUM;
    pstCfgFile->ulUncompLen = ulLen;

    /* 字节序转换之前先保存 */
    ulComLen = pstCfgFile->ulCompLen;

    LONG_CVT(pstCfgFile->ulMagicNum);
    LONG_CVT(pstCfgFile->ulUncompLen);
    LONG_CVT(pstCfgFile->ulCompLen);
    LONG_CVT(pstCfgFile->ulCrc);

    if (NULL == pcItem)
    {
#ifdef CONFIG_APPS_LOGIC_WANSELECT
        char *pszPath = "InternetGatewayDevice.Services."\
                        "X_TWSZ-COM_WANSelect.DefaultCfg";
        const char *pszValue = NULL;

        /* 获取默认配置索引 */
        ret = CFG_GetNodeValPtr(pszPath, &pszValue, NULL);
        if (CFG_OK == ret && strlen(pszValue) != 0)
        {
            sprintf(s_acCfgFileName, TBS_USERCFG_PREFIX"%s", pszValue);
        }
        else
        {
            /* 默认写在配置区tbs_app_cfg_1 */
            sprintf(s_acCfgFileName, TBS_USERCFG_PREFIX"%d", 1);
        }

        //printf("Saving config to %s area...\n", s_acCfgFileName);
#endif
        pcItem = s_acCfgFileName;
    }

    /* 写入配置区 */
    ret = CFG_WriteFile(pcItem, (char *)pstCfgFile,
                         sizeof(ST_CFG_FILE) + ulComLen);
    //printf("=======================in %s:%d, CFG_SaveCurCfg, ulLen = %d, pcItem=%s,struct=%d,total=%d\n",
    //    __FILE__, __LINE__, ulLen, pcItem, sizeof(ST_CFG_FILE), sizeof(ST_CFG_FILE)+ulComLen);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    free(pstCfgFile);

#ifdef _CFG_DEBUG
    if (CFG_OK == ret)
    {
        (void)CFG_WriteFile(CFG_DEBUG_FILE, pcCfg, ulLen);
    }
#endif

    return ret;
}


/* 清除当前配置 */
/***********************************************************
接口:   清除当前配置
参数:   无
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_ClearCurCfg(void)
{
    CFG_RET ret = CFG_OK;
    ST_CFG_FILE stHead;
    //stHead.ulMagicNum = 0;

    memset(&stHead,0xff,32);
      ret = CFG_WriteFile(s_acCfgFileName, (const char *)&stHead, 32);
    if (CFG_OK != ret)
    {
        CFG_ERR(ret);
    }

    return ret;
}

#ifdef CONFIG_APPS_LOGIC_WANSELECT

#define MAX_NUM_OF_CFG  4

/* 清除所有配置 */
/***********************************************************
接口:   清除所有配置
参数:   无
返回值: 0:成功
        其它:失败
***********************************************************/
CFG_RET CFG_ClearAllTheCfg(void)
{
    CFG_RET ret = CFG_OK;
    ST_CFG_FILE stHead;
    int i = 0;

    //stHead.ulMagicNum = 0;
    memset(&stHead,0xff,32);
    for (i = 0; i < MAX_NUM_OF_CFG; i++)
    {
        sprintf(s_acCfgFileName, TBS_USERCFG_PREFIX"%d", i);
        //printf("Clearing config at %s area...\n", s_acCfgFileName);

        ret = CFG_WriteFile(s_acCfgFileName, (const char *)&stHead, sizeof(ST_CFG_FILE));
        if (CFG_OK != ret)
        {
            CFG_ERR(ret);
        }
    }

    return ret;
}
#endif

/*************************************************************************
Function:      CFG_ReadDefaultCfg(char **ppcBuf, unsigned long ulPrevSpace,
                     unsigned long ulPostSpace, unsigned long *pulLen)
Description:   读取缺省配置
Calls:         无
Data Accessed:
Data Updated:
Input:         ppcBuf, 输出读取的内容, 如果 * ppcBuf == NULL, 则表示内存由函数内部分配
               ulPrevSpace, 读取的内容前端需要空出多少字节
               ulPostSpace, 读取的内容后端需要空出多少字节
               pulen, 输出读取出的长度, 初始输入 pcBuf 中可写入的长度
Output:
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_ReadDefaultCfg(char **ppcBuf, unsigned long ulPrevSpace,
                     unsigned long ulPostSpace, unsigned long *pulLen)
{
#ifdef CONFIG_APPS_LOGIC_WANSELECT

    char szFile[CFG_FILE_NAME_LEN] = {0};
    CFG_RET ret = CFG_OK;
    unsigned long ulLen = CFG_FILE_NAME_LEN;

    ret = cfg_item_get(szFile, TBS_DEFCFG_PATH, &ulLen);
    if (CFG_OK != ret)
    {
        sprintf(szFile, "%s", CFG_DEF_FILE);
    }

    CFG_LOG("Reading default config: %s\n", szFile);
    return CFG_ReadFile(szFile, ppcBuf, ulPrevSpace, ulPostSpace, pulLen);
#else
    return CFG_ReadFile(CFG_DEF_FILE, ppcBuf, ulPrevSpace, ulPostSpace, pulLen);
#endif
}


/*************************************************************************
Function:      CFG_ReadFullCfg(char **ppcBuf, unsigned long ulPrevSpace,
                     unsigned long ulPostSpace, unsigned long *pulLen)
Description:   读取逻辑树配置
Calls:         无
Data Accessed:
Data Updated:
Input:         ppcBuf, 输出读取的内容, 如果 * ppcBuf == NULL, 则表示内存由函数内部分配
               ulPrevSpace, 读取的内容前端需要空出多少字节
               ulPostSpace, 读取的内容后端需要空出多少字节
               pulen, 输出读取出的长度, 初始输入 pcBuf 中可写入的长度
Output:
Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_ReadFullCfg(char **ppcBuf, unsigned long ulPrevSpace,
                     unsigned long ulPostSpace, unsigned long *pulLen)
{
    return CFG_ReadFile(CFG_FULL_FILE, ppcBuf, ulPrevSpace, ulPostSpace, pulLen);
}

#ifdef  CONFIG_APPS_LOGIC_DEVCONFIG

#define CFG_DEVCONFIG_NODE_CONFIGFILE    "InternetGatewayDevice.DeviceConfig.ConfigFile"

static void inline myitoa(char *tmp, int num)
{
	switch(num)
	{
		case 0:
			*tmp = '0';
			break;
		case 1:
			*tmp = '1';
			break;
		case 2:
			*tmp = '2';
			break;
		case 3:
			*tmp = '3';
			break;
		case 4:
			*tmp = '4';
			break;
		case 5:
			*tmp = '5';
			break;
		case 6:
			*tmp = '6';
			break;
		case 7:
			*tmp = '7';
			break;
		case 8:
			*tmp = '8';
			break;
		case 9:
			*tmp = '9';
			break;
		case 10:
			*tmp = 'a';
			break;
		case 11:
			*tmp = 'b';
			break;
		case 12:
			*tmp = 'c';
			break;
		case 13:
			*tmp = 'd';
			break;
		case 14:
			*tmp = 'e';
			break;
		case 15:
			*tmp = 'f';
			break;
		default:
			*tmp = 'f';
			break ;
	}
}

static int inline myatoi(char *pcNum, const char *pcFirst, const char *pcSec )
{
     int iCount = 0;
     unsigned char iFirst, iSec;
     //printf("pcFirst = %c pcSec = %c\n", *pcFirst, *pcSec);
     if((*pcSec >= '0') && (*pcSec <= '9') )
     {
           iSec = *pcSec - '0';
	    iCount ++;
     }else if(*pcSec >= 'a' && *pcSec <= 'f'){
           iSec = *pcSec - 'a' + 10;
	    iCount ++ ;
     }else{
           return -1;
     }

     if((*pcFirst >= '0') && (*pcFirst <= '9') )
     {
           iFirst = *pcFirst - '0';
	    iCount ++ ;
     }else if((*pcFirst >= 'a') && (*pcFirst <= 'f')){
           iFirst = *pcFirst - 'a' + 10;
           iCount ++ ;
     }else{
           return -1;
     }
     *pcNum = iFirst  * 16 + iSec;
     //printf("pcNum=%x \n", *pcNum);
     return iCount;
}

CFG_RET CFG_CheckConfigFile(const char *pszValue)
{
   CFG_RET ret = CFG_OK;
#if 0
   ST_CFG_FILE *pstCfgFile = NULL;
   unsigned char *pstCfgTmp = NULL;
   unsigned long ulCalCrc = 0;
   const char *pstCfg;
   int rlen;

   if(safe_strlen(pszValue)>0)
   {

	    pstCfg=(char *)pszValue;
		pstCfgTmp = tbsBase64Decode(pstCfg, &rlen);
		pstCfgFile = (ST_CFG_FILE*)pstCfgTmp;

	    LONG_CVT(pstCfgFile->ulMagicNum);
	    LONG_CVT(pstCfgFile->ulUncompLen);
	    LONG_CVT(pstCfgFile->ulCompLen);
	    LONG_CVT(pstCfgFile->ulCrc);

    	 /* 检测数据是否有效 */
    	 if (MAGICNUM != pstCfgFile->ulMagicNum)
    	 {
    	       CFG_ERR(CFG_FAIL);
    		ret = CFG_FAIL;
		goto __out;
    	 }

   	   /* CRC 校验 */
   	   ulCalCrc= crc32(0, pstCfgFile->acData, pstCfgFile->ulCompLen);
   	   if (pstCfgFile->ulCrc != ulCalCrc)
   	   {
   	       CFG_ERR(ERR_CFG_FILE_CHKSUM);
   		   ret = ERR_CFG_FILE_CHKSUM;
		goto __out;
   	    }
   }
   else
   {
      CFG_ERR(CFG_FAIL);
	  ret = CFG_FAIL;
	  return ret;
   }

__out:

      free(pstCfgTmp);
#endif
      return ret;
}

/* 保存配置到ConfigFile节点 */
/*************************************************************************
Function:      CFG_RET CFG_SaveCurrentCfgToConfigNode()
Description:   备份配置
Calls:         无
Data Accessed:
Data Updated:
Input:
Output:        无

Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_SaveCurrentCfgToConfigNodeFromWeb()
{
	CFG_RET ret = CFG_OK;
	ST_CFG_FILE *pstCfgFile = NULL;
	const unsigned char *pstCfgTmp = NULL;
	unsigned long ulLen = 0;
	char *pstValue =NULL;

#ifdef CONFIG_APPS_LOGIC_WANSELECT
	unsigned long ulBufLen = CFG_FILE_NAME_LEN;

    ret = cfg_item_get(s_acCfgFileName, TBS_USERCFG_ITEM, &ulBufLen);
    if (CFG_OK != ret)
    {
        sprintf(s_acCfgFileName, TBS_USERCFG_PREFIX"%d", 1);
    }

    printf("Read config from %s area...\n", s_acCfgFileName);
#endif

    ret = CFG_ReadItem(s_acCfgFileName, (char **)(void *)&pstCfgFile, &ulLen);
    if (CFG_OK != ret)
    {

      	 CFG_ERR(ret);
      	 return ret;
    }

	pstCfgTmp = (unsigned char *)pstCfgFile;
	pstValue = tbsBase64Encode(pstCfgTmp, ulLen);
    ret = CFG_SetNodeVal(CFG_DEVCONFIG_NODE_CONFIGFILE, pstValue, NULL);

    if(ret!=TBS_SUCCESS)
       printf("Set Node Fail!\n");

	free(pstValue);
	free(pstCfgFile);

	return ret;
}

#if 0
/* 保存TR069传配置到ConfigFile节点 */
/*************************************************************************
Function:      CFG_RET CFG_SaveCurrentCfgToConfigNode(char *pszValue)
Description:   导入配置
Calls:         无
Data Accessed:
Data Updated:
Input:         pszValue,当前配置内容
Output:        无

Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_SaveCurrentCfgToConfigNode(char *pszValue)
{
   CFG_RET iRet = CFG_OK;
   ST_CFG_FILE *pstCfgFile = NULL;
   unsigned long ulCalCrc = 0;
   const char *pstCfg;
   int *rlen;

  //tr069 传过来的值
   if(safe_strlen(pszValue)>0)
   {
	    pstCfg=(char *)pszValue;

		pstCfgFile = tbsBase64Decode(pstCfg, rlen);

	    LONG_CVT(pstCfgFile->ulMagicNum);
	    LONG_CVT(pstCfgFile->ulUncompLen);
	    LONG_CVT(pstCfgFile->ulCompLen);
	    LONG_CVT(pstCfgFile->ulCrc);

    	 /* 检测数据是否有效 */
    	 if (MAGICNUM != pstCfgFile->ulMagicNum)
    	 {
    	      CFG_ERR(CFG_FAIL);
    		  iRet = CFG_FAIL;
		goto __out;
    	 }

   	 /* CRC 校验 */
   	 ulCalCrc= crc32(0, pstCfgFile->acData, pstCfgFile->ulCompLen);
   	 if (pstCfgFile->ulCrc != ulCalCrc)
   	 {
   	       CFG_ERR(ERR_CFG_FILE_CHKSUM);
   		ret = ERR_CFG_FILE_CHKSUM;
		goto __out;
   	  }

               /* 写入配置区 */
	iRet = CFG_SetNodeVal(CFG_DEVCONFIG_NODE_CONFIGFILE, pszValue, NULL);

__out:
	free(pstCfgFile);

       }

    return iRet;
}
#endif

/* TR069远程恢复配置 */
/*************************************************************************
Function:      CFG_RET CFG_EffectDeviceConfigNode(char *pszValue)
Description:   导入配置
Calls:         无
Data Accessed:
Data Updated:
Input:         pszValue,当前配置加密内容
Output:        无

Return:        0,成功;
               其它, 失败
Others:
*************************************************************************/
CFG_RET CFG_EffectDeviceConfigNode(char *pszValue)
{
   CFG_RET ret = CFG_OK;
   ST_CFG_FILE *pstCfgFile = NULL;
   unsigned char *pstCfgTmp = NULL;
   unsigned long ulCalCrc = 0;
   const char *pstCfg;
   int rlen;

   //tr069 传过来的值
   if(safe_strlen(pszValue)>0)
   {
		printf("enter the CFG_EffectDeviceConfigNode \n");
	    pstCfg=(char *)pszValue;
		pstCfgTmp = tbsBase64Decode(pstCfg, &rlen);
		pstCfgFile = (ST_CFG_FILE*)pstCfgTmp;

	 	printf("1. MagicNum=0x%08x, 0x%08x\n", (unsigned int)pstCfgFile->ulMagicNum, (unsigned int)MAGICNUM);
	    //LONG_CVT(pstCfgFile->ulMagicNum);
	    //LONG_CVT(pstCfgFile->ulUncompLen);
	    //LONG_CVT(pstCfgFile->ulCompLen);
	    //LONG_CVT(pstCfgFile->ulCrc);
		//printf("2. MagicNum=0x%08x, 0x%08x\n", (unsigned int)pstCfgFile->ulMagicNum, (unsigned int)MAGICNUM);

		/* 检测数据是否有效 */
		if (htonl(MAGICNUM) != pstCfgFile->ulMagicNum)
		{
			CFG_ERR(CFG_FAIL);
			ret = CFG_FAIL;
			goto __out;
		}

		/* CRC 校验 */
		ulCalCrc= crc32(0, pstCfgFile->acData, pstCfgFile->ulCompLen);
		if (pstCfgFile->ulCrc != htonl(ulCalCrc))
		{
			CFG_ERR(ERR_CFG_FILE_CHKSUM);
			ret = ERR_CFG_FILE_CHKSUM;
			goto __out;
		}

#ifdef CONFIG_APPS_LOGIC_WANSELECT
        unsigned long ulBufLen = CFG_FILE_NAME_LEN;

	    ret = cfg_item_get(s_acCfgFileName, TBS_USERCFG_ITEM, &ulBufLen);
	    if (CFG_OK != ret)
	    {
	        sprintf(s_acCfgFileName, TBS_USERCFG_PREFIX"%d", 1);
	    }

	    printf("Read config from %s area...\n", s_acCfgFileName);
#endif

	   /* 写入配置区 */
	   ret = CFG_WriteFile(s_acCfgFileName, (char *)pstCfgFile, rlen);
   }

__out:

	free(pstCfgTmp);
	if (CFG_OK != ret)
	{
	   CFG_ERR(ret);
	}

   	printf("CFG_EffectDeviceConfigNode success \n");
    return ret;
}
#endif
#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif


