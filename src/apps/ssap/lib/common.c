/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称: Common_fun.c
 文件描述: 公共函数的封装，提供给各模块使用

 修订记录:
        1. 作者: all
           日期: 2007-08-07
           内容: 创建文件

**********************************************************************/
#include <stdarg.h>
#include <ctype.h>
#include "common.h"

#if 0
#define COMM_TRACE(fmt, args...)    COMMON_TRACE(MID_CCP, fmt, ##args)
#else
#define COMM_TRACE(fmt, args...)    do { ; } while(0)
#endif

extern int vsscanf(const char *, const char *, va_list);


/**************************************************************************
功能: 将节点的值保存到配置缓存中
参数: void *pInstance,               当前实例;
               const char *pszName,    节点名;
               const char * pszValue,   节点值
               const char *pszPath       路径
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int SetToConfBuf(void *pInstance, const char *pszName, const char * pszValue, const char *pszPath)
{
    int iRet = TBS_SUCCESS;

    ST_Instance *pstInstance = NULL;
    char **ppcIdxTable = NULL;
    ST_NodeInfo **ppstNodeInfoList = NULL;
    const ST_NodeInfo *pstNodeInfo = NULL;

    char *pszValueBuf = NULL;
    char szOldValue[MAX_NODE_VALUE_LEN] = { 0 };
    unsigned long ulValLen = 0;

    int iOffset = 0;
    unsigned int nNodeCount = 0;
    unsigned int i = 0;

    if ( NULL == pInstance || NULL == pszName || NULL == pszValue )
    {
        COMM_TRACE(INFO_ERROR);
        return TBS_NULL_PTR;
    }

    COMM_TRACE("pszName = %s, pszValue = %s\n", pszName, pszValue);

    pstInstance = (ST_Instance *)pInstance;

    /* 获取节点信息列表*/
    nNodeCount = pstInstance->nNodeCount;
    ppstNodeInfoList = pstInstance->ppstNodeInfoList;
    if ( NULL == ppstNodeInfoList )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    /* 查找节点信息*/
    pstNodeInfo = FindNodeInfo(ppstNodeInfoList, pszName);
    if ( NULL == pstNodeInfo )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_NAME;
    }

    /* 越界*/
    iOffset = pstNodeInfo - pstInstance->pstNodeInfoArray;
    if ( iOffset < 0 || iOffset >= nNodeCount )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_INDEX_OVERFLOW;
    }

    /* 判断节点是否可写*/
    if ( pstNodeInfo->bReadOnly )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_WRITE_FORBID;
    }

    /* 检查值的合法性*/
    if ( NULL != pstNodeInfo->pCheckFunc )
    {
        if ( ! (pstNodeInfo->pCheckFunc(pszValue)) )
        {
            COMM_TRACE(INFO_ERROR"\n");
            return TR069_ERRNO_INVALID_VAL;
        }
    }

    /* 获取配置缓存索引表*/
    ppcIdxTable = pstInstance->ppcIdxTable;

    /* 检查新节点值是否和配置树中的相同*/
    if ( NULL != pszPath )
    {
        COMM_TRACE("pszPath = %s\n", pszPath);

        /* 从配置树中获取旧节点值*/
        ulValLen = MAX_NODE_VALUE_LEN;
        iRet =  CFG_GetLeafValAndType(pszPath, szOldValue, &ulValLen, NULL, NULL);
        if ( RET_SUCCEED(iRet) && 0 == safe_strcmp(szOldValue, pszValue) )
        {
            if ( NULL != ppcIdxTable )
            {
                COMM_TRACE("\n");

                /* 释放节点值缓存*/
                safe_free(ppcIdxTable[iOffset]);

                /* 检查配置缓存中是否有值*/
                for ( i = 0; i < nNodeCount; i++ )
                {
                    if ( NULL != ppcIdxTable[i] )
                    {
                        break;
                    }
                }

                /* 如果配置缓存中没有值，释放掉配置缓存索引表*/
                if ( i == nNodeCount )
                {
                    COMM_TRACE("\n");
                    safe_free(pstInstance->ppcIdxTable);
                }
            }

            COMM_TRACE(INFO_SUCCESS"\n");
            return TBS_SUCCESS;
        }
    }

    /* 配置缓存索引表不存在*/
    if ( NULL == ppcIdxTable )
    {
        COMM_TRACE("\n");

        /* 动态分配配置缓存索引表*/
        ppcIdxTable = (char **)malloc(nNodeCount  * sizeof(char *));
        if ( NULL == ppcIdxTable )
        {
            COMM_TRACE(INFO_ERROR"\n");
            return TBS_OUT_OF_MEM;
        }

        /* 初始化*/
        memset(ppcIdxTable, 0, nNodeCount  * sizeof(char *));
    }
    else
    {
        /* 检查新节点值是否和配置缓存中的相同*/
        if ( 0 == safe_strcmp(ppcIdxTable[iOffset], pszValue) )
        {
            COMM_TRACE(INFO_SUCCESS"\n");
            return TBS_SUCCESS;
        }
        else
        {
            /* 释放掉以前的配置缓存*/
            COMM_TRACE("\n");
            safe_free(ppcIdxTable[iOffset]);
        }
    }

    /* 为新的节点值分配空间*/
    pszValueBuf = (char *)malloc(strlen(pszValue) + 1);
    if ( NULL == pszValueBuf )
    {
        if ( NULL == pstInstance->ppcIdxTable )
        {
            safe_free(ppcIdxTable);
        }

        COMM_TRACE(INFO_ERROR"\n");
        return TBS_OUT_OF_MEM;
    }

    /* 将节点值配置缓存添加到配置缓存索引表中*/
    strcpy(pszValueBuf, pszValue);
    ppcIdxTable[iOffset] = pszValueBuf;

    /* 将配置索引表加入到实例中*/
    pstInstance->ppcIdxTable = ppcIdxTable;

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
功能: 从配置缓存中获取节点的值
参数: void *pInstance,               当前实例;
               const char *pszName,    节点名;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
const char * GetFromConfBuf(void * pInstance, const char *pszName)
{
    ST_Instance * pstInstance = NULL;
    char **ppcIdxTable = NULL;
    ST_NodeInfo **ppstNodeInfoList = NULL;
    const ST_NodeInfo *pstNodeInfo = NULL;
    int iOffset = 0;

    if ( NULL == pInstance || NULL == pszName )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return NULL;
    }

    COMM_TRACE("pszName = %s\n", pszName);

    pstInstance = (ST_Instance *)pInstance;

    /* 获取配置缓存索引表*/
    ppcIdxTable = pstInstance->ppcIdxTable;
    if ( NULL == ppcIdxTable )
    {
        COMM_TRACE("\n");
        return NULL;
    }

    /* 获取节点信息列表*/
    ppstNodeInfoList = pstInstance->ppstNodeInfoList;
    if ( NULL == ppstNodeInfoList )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return NULL;
    }

    /* 查找节点信息*/
    pstNodeInfo = FindNodeInfo(ppstNodeInfoList, pszName);
    if ( NULL == pstNodeInfo )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return NULL;
    }

    /* 越界*/
    iOffset = pstNodeInfo - pstInstance->pstNodeInfoArray;
    if ( iOffset < 0 || iOffset >= pstInstance->nNodeCount )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return NULL;
    }

    if ( NULL == ppcIdxTable[iOffset] )
    {
        COMM_TRACE("\n");
        return NULL;
    }
    else
    {
        COMM_TRACE(INFO_SUCCESS"\n");
        return ppcIdxTable[iOffset];
    }
}

/**************************************************************************
功能: 释放实例的配置缓存
参数: void *pInstance,               当前实例;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int FreeConfBuf(void *pInstance)
{
    ST_Instance *pstInstance = NULL;
    char **ppcIdxTable = NULL;
    unsigned int nNodeCount = 0;
    unsigned int i = 0;

    if ( NULL == pInstance )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    pstInstance = (ST_Instance *)pInstance;

    /* 配置缓存索引表*/
    ppcIdxTable = pstInstance->ppcIdxTable;
    if ( ppcIdxTable != NULL )
    {
        /* 获取节点个数*/
        nNodeCount = pstInstance->nNodeCount;

        /* 释放所有节点配置缓存*/
        for ( i = 0; i < nNodeCount; i++ )
        {
            safe_free(ppcIdxTable[i]);
        }

        /* 释放配置缓存索引表*/
        safe_free(pstInstance->ppcIdxTable);
    }

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
功能: 释放模块的所有实例的配置缓存
参数: void *pInstanceList,               当前实例;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int FreeAllConfBuf(void *pInstanceList)
{
    ST_Instance *pstInstanceList = NULL;
    ST_Instance *pstInstance = NULL;
    char **ppcIdxTable = NULL;
    unsigned int nNodeCount = 0;
    unsigned int i = 0;

    pstInstanceList = (ST_Instance *)pInstanceList;

    /* 遍历所有实例*/
    for ( pstInstance = pstInstanceList; pstInstance; pstInstance = pstInstance->pstNext )
    {
        /* 配置缓存索引表*/
        ppcIdxTable = pstInstance->ppcIdxTable;
        if ( ppcIdxTable != NULL )
        {
            /* 获取节点个数*/
            nNodeCount = pstInstance->nNodeCount;

            /* 释放所有节点配置缓存*/
            for ( i = 0; i < nNodeCount; i++ )
            {
                safe_free(ppcIdxTable[i]);
            }

            /* 释放配置缓存索引表*/
            safe_free(pstInstance->ppcIdxTable);
        }
    }

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
功能: 应用所有实例的最新配置
参数: void *pInstanceList,                               当前实例;
             int (* pApplyFunc)(void * pInstance)   配置生效函数;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int ApplyAllConfig(void *pInstanceList, int (* pApplyFunc)(void * pInstance))
{
    ST_Instance *pstInstanceList = NULL;
    ST_Instance *pstInstance = NULL;
    int iRet = TBS_SUCCESS;

    if ( NULL == pApplyFunc )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    pstInstanceList = (ST_Instance *)pInstanceList;

    /* 遍历所有实例*/
    for ( pstInstance = pstInstanceList; pstInstance; pstInstance = pstInstance->pstNext )
    {
        /* 修改了配置*/
        if ( NULL != pstInstance->ppcIdxTable )
        {
            iRet = (*pApplyFunc)(pstInstance);
            if ( RET_FAILED(iRet) )
            {
                COMM_TRACE(INFO_ERROR"\n");
                return iRet;
            }
        }
    }

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
功能: 根据节点名找到对应的 NodeInfo
参数: ST_NodeInfo **ppstNodeInfoList,                              节点链表;
             const char * pszName   配置生效函数;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
const ST_NodeInfo * FindNodeInfo(ST_NodeInfo **ppstNodeInfoList, const char * pszName)
{
    char ch = 0;
    int iHash = 0;
    const ST_NodeInfo * pstNodeInfo = NULL;

    if ( NULL == ppstNodeInfoList || NULL == pszName )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return NULL;
    }

    /* 根据节点名的第一个字母生成hash 值*/
    ch = pszName[0];
    if ( ch >= 'a' && ch <= 'z' )
    {
        iHash = ch - 'a';
    }
    else if ( ch >= 'A' && ch <= 'Z' )
    {
        iHash = ch - 'A';
    }
    else
    {
        COMM_TRACE(INFO_ERROR"\n");
        return NULL;
    }

    /* 根据节点名在节点信息hash 表中查找*/
    for ( pstNodeInfo = ppstNodeInfoList[iHash]; pstNodeInfo; pstNodeInfo = pstNodeInfo->pstNext )
    {
        if ( 0 == safe_strcmp(pszName, pstNodeInfo->pszName) )
        {
            COMM_TRACE(INFO_SUCCESS"\n");
            return pstNodeInfo;
        }
    }

    COMM_TRACE(INFO_ERROR"\n");
    return NULL;
}

/**************************************************************************
功能: 将节点信息添加到节点信息列表中
参数: ST_NodeInfo **ppstNodeInfoList,                              节点链表;
             const char * pszName                                                 配置生效函数;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int AddNodeInfo(ST_NodeInfo **ppstNodeInfoList, ST_NodeInfo * pstNodeInfo)
{
    char ch = 0;
    int iHash = 0;
    const char *pszName = NULL;
    ST_NodeInfo * pstTempNodeInfo = NULL;

    if ( NULL == ppstNodeInfoList || NULL == pstNodeInfo )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    pszName = pstNodeInfo->pszName;
    if ( NULL == pszName )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    /* 根据节点名的第一个字母生成hash 值*/
    ch = pszName[0];
    if ( ch >= 'a' && ch <= 'z' )
    {
        iHash = ch - 'a';
    }
    else if ( ch >= 'A' && ch <= 'Z' )
    {
        iHash = ch - 'A';
    }
    else
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_FAILED;
    }

    /* 根据节点名在节点信息hash 表中查找*/
    for ( pstTempNodeInfo = ppstNodeInfoList[iHash]; pstTempNodeInfo; pstTempNodeInfo = pstTempNodeInfo->pstNext )
    {
        if ( 0 == safe_strcmp(pstTempNodeInfo->pszName, pszName) )
        {
            COMM_TRACE(INFO_ERROR"\n");
            return TBS_FAILED;
        }
    }

    /* 将节点信息添加到节点信息hash 表中*/
    pstNodeInfo->pstNext = ppstNodeInfoList[iHash];
    ppstNodeInfoList[iHash] = pstNodeInfo;

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}


/**************************************************************************
功能: 添加子结点
参数: const char *pszPath,                                                   当前路径
             const ST_TreeNode astTreeNodeList[],                      叶子节点列表;
             unsigned int nNodeCount                                             叶子节点数;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int AddChildNode(const char *pszPath, const ST_TreeNode astTreeNodeList[], unsigned int nNodeCount)
{
    int iRet = 0;
    unsigned int i = 0;
    char szPath[MAX_PATH_LEN] = {0};

    if ( NULL == pszPath || NULL == astTreeNodeList )
    {
        return TBS_NULL_PTR;
    }

    for (i = 0; i < nNodeCount; i++)
    {
        if ( NULL == astTreeNodeList[i].pszName || NULL == astTreeNodeList[i].pszType || NULL == astTreeNodeList[i].pszValue )
        {
            COMM_TRACE(INFO_ERROR"\n");
            return TBS_NULL_PTR;
        }

        /* 节点路径*/
        sprintf(szPath, "%s.%s", pszPath, astTreeNodeList[i].pszName);

        /* 添加节点*/
        iRet = CFG_AddNode(szPath, astTreeNodeList[i].pszType, astTreeNodeList[i].pszValue, astTreeNodeList[i].ucIsStandard,
            astTreeNodeList[i].ucNeedSave, astTreeNodeList[i].ucWriteable);
        if(RET_FAILED(iRet))
        {
            COMM_TRACE(INFO_ERROR"\n");
            return iRet;
        }
    }

    return TBS_SUCCESS;
}


/**************************************************************************
功能: 从SET 命令中获取节点路径
参数: const char *pszCmd,                                                  输入的SET字符串;
             char *pszPath                                                             解析出的路径;
             unsigned long ulLen                                                    path数组的总长度;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int GetPathFromCmd(const char *pszCmd, char *pszPath, unsigned long ulLen)
{
    char *pcPos = NULL;
    unsigned long ulPathLen = 0;

    if ( NULL == pszCmd || NULL == pszPath )
    {
        return TBS_NULL_PTR;
    }

    /* 查找'='号的位置*/
    pcPos = strchr(pszCmd, '=');
    if ( NULL != pcPos )
    {
        ulPathLen = pcPos - pszCmd + 1;

        if ( ulLen > ulPathLen )
        {
            ulLen = ulPathLen;
        }
    }

    /* 将Path保存*/
    safe_strncpy(pszPath, pszCmd, ulLen);

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
功能: 从SET命令中解析出节点名、节点值和实例号
参数: const char * pszSetCmd                                                输入的SET字符串;
             char * pszName                                                             解析出的路径;
             char * pszValue                                                             解析出的值;
             unsigned long aulIdxList[]                                            获取的index列表
             int *piIdxCount                                                              获取的index数量
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int ParseSetCmd(const char * pszSetCmd, char * pszName, char * pszValue, unsigned long aulIdxList[], int *piIdxCount)
{
    int iRet = 0;

    char szSetCmdBuf[MAX_PATH_LEN] = { 0 };
    char * apszSubStr[MAX_PATH_NODE_COUNT] = { NULL };
    char * pcPos = NULL;

    int iSubCount = 0;
    int iIdxCount = 0;
    int i = 0;

    unsigned long ulIdx = 0;

    if ( NULL == pszSetCmd )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    /* 复制SET 命令到缓冲区中*/
    safe_strncpy(szSetCmdBuf, pszSetCmd, MAX_PATH_LEN);

    /* 等号后面的是节点值*/
    pcPos = strchr(szSetCmdBuf, '=');

    if ( NULL == pcPos )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_VAL;
    }

    /* 等号前面的是节点路径*/
    *pcPos = '\0';
    pcPos++;

    /* 节点值*/
    if ( NULL != pszValue )
    {
        strcpy(pszValue, pcPos);
    }
    else
    {
        return TR069_ERRNO_INVALID_VAL;
    }

    /* 将节点路径用点切分成若干子串*/
    iSubCount = tbsSplitString(szSetCmdBuf, apszSubStr, '.', MAX_PATH_NODE_COUNT);
    if ( iSubCount > MAX_PATH_NODE_COUNT )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_NAME;
    }

    if ( NULL != aulIdxList && NULL != piIdxCount )
    {
        /* 实例号最大个数*/
        iIdxCount = *piIdxCount;

        /* 从最后一个子串开始遍历*/
        for ( i = iSubCount - 1; i >= 0; i-- )
        {
            iRet = sscanf(apszSubStr[i], "%lu", &ulIdx);

            /* 子串是实例号*/
            if ( 1 == iRet )
            {
                /* 路径中的实例号个数大于需要的个数*/
                if ( iIdxCount < 1 )
                {
                    COMM_TRACE(INFO_ERROR"\n");
                    return TR069_ERRNO_INVALID_NAME;
                }
                else
                {
                    aulIdxList[--iIdxCount] = ulIdx;
                }
            }
        }

        /* 实际实例号个数*/
        *piIdxCount -= iIdxCount;

        /* 如果实际实例号个数小于指定的最大实例号个数*/
        if ( iIdxCount > 0 )
        {
            /* 从数组anIdxList 的第一项开始保存实例号*/
            memcpy(aulIdxList, aulIdxList + iIdxCount, (*piIdxCount) * sizeof(unsigned long));
            memset(aulIdxList + *piIdxCount, 0, iIdxCount * sizeof(unsigned long));
        }
    }

    /* 最后一个子串是节点名*/
    if ( iSubCount > 0 && NULL != pszName )
    {
        strcpy(pszName, apszSubStr[iSubCount - 1]);
    }

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}


/**************************************************************************
功能: 从SET命令中解析出节点名、节点值和实例号
参数: const char * pszSetCmd                                                输入的SET字符串;
             unsigned int nNameLen                                                 解析的节点名长度;
             unsigned int nValueLen                                                 解析出的值长度;
             const char *pszFmt                                                       格式列表
             ...                                                                                    解析出的结构
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int ParseSetCmdExact(const char *pszSetCmd, unsigned int nNameLen, unsigned int nValueLen, const char *pszFmt, ...)
{
    const char *pcPos = NULL;
    char *pszName = NULL;
    char *pszValue = NULL;
    unsigned long *pulIdx = 0;
    char szIdx[MAX_ULONG_LEN] = { 0 };
    unsigned int nLen = 0;
    va_list argp;

    if ( NULL == pszSetCmd || NULL == pszFmt )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    /* 检查路径是否正确，并解析出实例号*/
    va_start(argp, pszFmt);
    for ( ; *pszFmt && *pszSetCmd; pszFmt++, pszSetCmd++ )
    {
        /* 判断是实例号还是路径字符串*/
        if ( '%' == *pszFmt )
        {
            /* 查找实例号字符串*/
            pulIdx = va_arg(argp, unsigned long *);
            for ( pcPos = pszSetCmd; *pszSetCmd; pszSetCmd++ )
            {
                if ( *pszSetCmd < '0' || *pszSetCmd > '9' )
                {
                    break;
                }
            }

            /* 检查实例号字符串是否超长*/
            nLen = pszSetCmd - pcPos;
            if ( 0 == nLen || nLen > MAX_ULONG_LEN - 1 )
            {
                COMM_TRACE(INFO_ERROR"\n");
                return TR069_ERRNO_INVALID_NAME;
            }

            /* 解析实例号*/
            safe_strncpy(szIdx, pcPos, nLen + 1);
            sscanf(szIdx, "%lu", pulIdx);

            /* 继续往下处理*/
            pszFmt++;
            continue;
        }
        else
        {
            if ( *pszSetCmd != *pszFmt )
            {
                COMM_TRACE(INFO_ERROR"\n");
                return TR069_ERRNO_INVALID_NAME;
            }
        }
    }
    pszName = va_arg(argp, char *);
    pszValue = va_arg(argp, char *);
    va_end(argp);

    /* 路径是否完全匹配*/
    if ( '\0' != *pszFmt )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_NAME;
    }

    /* 保存节点名和节点值的缓冲区不存在*/
    if ( NULL == pszName || NULL == pszValue )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    /* 查找等号位置*/
    pcPos = strchr(pszSetCmd, '=');
    if ( NULL == pcPos )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_VAL;
    }

    /* 等号前面的是节点名*/
    nLen = pcPos - pszSetCmd + 1;
    if ( nLen > nNameLen )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_NAME;
    }

    safe_strncpy(pszName, pszSetCmd, nLen);

    /* 等号后面的是节点值*/
    pcPos++;
    nLen = strlen(pcPos) + 1;
    if ( nLen > nValueLen )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_VAL;
    }

    safe_strncpy(pszValue, pcPos, nLen);

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
功能: 添加字符串到字符串链表中
参数: ST_Str ** ppstStrList                                             字符串链表;
             char *pszStr                                                           添加的字符串;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
static int AddStr(ST_Str ** ppstStrList, char *pszStr)
{
    ST_Str *pstStr = NULL;

    if (NULL == ppstStrList || NULL == pszStr)
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    /* 创建结构*/
    pstStr = (ST_ErrInfo *)malloc(sizeof(ST_ErrInfo));

    /* 分配空间失败*/
    if (NULL == pstStr)
    {
        COMM_TRACE(INFO_ERROR"Memery allocate failed\n");
        return TBS_OUT_OF_MEM;
    }

    pstStr->pszStr = pszStr;

    /* 在链表首插入新错误信息*/
    pstStr->pstNext = *ppstStrList;
    *ppstStrList = pstStr;

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
功能: 添加错误信息到错误信息链表中
参数: ST_Str ** ppstStrList                                             字符串链表;
             int iErrCode,                                                           错误码
             char *pszPath                                                           添加的字符串;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int AddErrInfo(ST_ErrInfo **ppstErrInfoList, int iErrCode, const char *pszPath)
{
    char *pszErrDescBuf = NULL;
    int iLen = 0;

    if (NULL == ppstErrInfoList || NULL == pszPath)
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    iLen = strlen(pszPath) + 1 + MAX_INT_LEN + 1 + 1;

    /* 为错误描述分配空间*/
    pszErrDescBuf = (char *)malloc(iLen);

    /* 分配空间失败*/
    if (NULL == pszErrDescBuf)
    {
        COMM_TRACE(INFO_ERROR"Memery allocate failed\n");
        return TBS_OUT_OF_MEM;
    }

    /* 生成错误描述信息*/
    sprintf(pszErrDescBuf, "%s=%04u%04u", pszPath,
                 TBS_ERR_INTERNAL(iErrCode), TBS_ERR_STAND(iErrCode));

    COMM_TRACE("pszErrDescBuf = %s\n", pszErrDescBuf);
    return AddStr(ppstErrInfoList, pszErrDescBuf);
}

/**************************************************************************
功能: 添加结果信息到结果信息链表中
参数: ST_ResultInfo **ppstResultInfoList                                           结果信息链表;
             const char *pszPath                                                                    路径
             const char *pszType                                                                   类型;
             const char *pszValue                                                                  值
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int AddResultInfo(ST_ResultInfo **ppstResultInfoList, const char *pszPath, const char *pszType, const char *pszValue)
{
    char *pszResultBuf = NULL;
    int iLen = 0;

    if (NULL == ppstResultInfoList || NULL == pszPath || NULL == pszType || NULL == pszValue)
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    iLen = strlen(pszType) + 1 + strlen(pszPath) + 1 + strlen(pszValue) + 1;

    /* 为错误描述分配空间*/
    pszResultBuf = (char *)malloc(iLen);

    /* 分配空间失败*/
    if (NULL == pszResultBuf)
    {
        COMM_TRACE(INFO_ERROR"Memery allocate failed\n");
        return TBS_OUT_OF_MEM;
    }

    /* 生成错误描述信息*/
    sprintf(pszResultBuf, "%s %s=%s", pszType, pszPath, pszValue);

    COMM_TRACE("pszResultBuf = %s\n", pszResultBuf);
    return AddStr(ppstResultInfoList, pszResultBuf);
}

/**************************************************************************
功能: 释放字符串链表
参数: ST_Str **ppstStrList                                           字符串链表;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int FreeAllStr(ST_Str **ppstStrList)
{
    ST_Str *pstStr = NULL;

    while ( NULL != *ppstStrList )
    {
        pstStr = *ppstStrList;
        *ppstStrList = pstStr->pstNext;

        safe_free(pstStr->pszStr);
        safe_free(pstStr);
    }

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
功能: 回复错误消息
参数: const ST_MSG *pstOrgMsg                                           原消息;
             unsigned long ulErrNo                                                 错误码;
             ST_ErrInfo *pstErrInfoList                                          错误信息列表;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int RespErrMsg(const ST_MSG *pstOrgMsg, unsigned long ulErrNo, ST_ErrInfo *pstErrInfoList)
{
    int  iRet = TBS_SUCCESS;
    ST_MSG *pstRespMsg = NULL;
    unsigned long ulMsgBodyLen = 0;
    ST_ErrInfo *pstErrInfo = NULL;
    unsigned long ulCount = 0;
    char *pcPos = NULL;

    if (NULL == pstOrgMsg )
    {
        COMM_TRACE(INFO_ERROR"Invalid function prameters\n");
        return TBS_NULL_PTR;
    }

    /* 结果和个数*/
    ulMsgBodyLen = sizeof(unsigned long) * 3;

    /* 计算消息长度*/
    for ( pstErrInfo = pstErrInfoList; pstErrInfo; pstErrInfo = pstErrInfo->pstNext )
    {
        if ( NULL != pstErrInfo->pszStr )
        {
            ulMsgBodyLen += strlen(pstErrInfo->pszStr) + 1;
            ulCount++;
        }
    }

    /* 创建响应消息*/
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"Create response message failed \n");
        return TBS_OUT_OF_MEM;
    }

    /* 填充消息头部*/
    pstRespMsg->stMsgHead.usSrcMID = pstOrgMsg->stMsgHead.usDstMID;
    pstRespMsg->stMsgHead.usDstMID = pstOrgMsg->stMsgHead.usSrcMID;
    pstRespMsg->stMsgHead.ulMsgID = pstOrgMsg->stMsgHead.ulMsgID;

    /* 消息类型*/
    switch ( pstOrgMsg->stMsgHead.usMsgType )
    {
        case MSG_CMM_ADD_NODE:
            pstRespMsg->stMsgHead.usMsgType = MSG_CMM_ADD_NODE_ACK;
            break;

        case MSG_CMM_DEL_NODE:
            pstRespMsg->stMsgHead.usMsgType = MSG_CMM_DEL_NODE_ACK;
            break;

        case MSG_CMM_GET_VAL:
            pstRespMsg->stMsgHead.usMsgType = MSG_CMM_GET_VAL_ACK;
            break;

        case MSG_CMM_SET_VAL:
            pstRespMsg->stMsgHead.usMsgType = MSG_CMM_SET_VAL_ACK;
            break;
        case MSG_CMM_INST_ADDED:
            pstRespMsg->stMsgHead.usMsgType = MSG_CMM_INST_ADDED_ACK;
            break;
        case MSG_CMM_INST_DELED:
            pstRespMsg->stMsgHead.usMsgType = MSG_CMM_INST_DELED_ACK;
            break;
        default:
            safe_free_msg(pstRespMsg);

            COMM_TRACE(INFO_ERROR"\n");
            return TBS_FAILED;
    }

    pstRespMsg->stMsgHead.ulBodyLength = ulMsgBodyLen;

    pcPos = pstRespMsg->szMsgBody;

    /* TR069标准定义的错误码*/
    SET_ULONG(pcPos, ulErrNo);

    /* 内部错误码*/
    SET_ULONG(pcPos, ulErrNo);

    /* 错误项个数*/
    SET_ULONG(pcPos, ulCount);

    /* 错误项*/
    for ( pstErrInfo = pstErrInfoList; pstErrInfo; pstErrInfo = pstErrInfo->pstNext )
    {
        if ( NULL != pstErrInfo->pszStr )
        {
            COMM_TRACE("pszStr = %s\n", pstErrInfo->pszStr);
            SET_STR(pcPos, pstErrInfo->pszStr);
        }
    }

    /* 发送响应消息 */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}

/* 响应添加实例消息 */
int RespAddMsg(const ST_MSG *pstOrgMsg, unsigned long ulObjNo, unsigned long ulStatus)
{
    int  iRet = TBS_SUCCESS;
    ST_MSG *pstRespMsg = NULL;
    unsigned long ulMsgBodyLen = 0;
    char *pcPos = NULL;

    if ( NULL == pstOrgMsg )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    ulMsgBodyLen = sizeof(unsigned long) * 3;

    /* 申请回应消息的内存 */
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_OUT_OF_MEM;
    }

    /* 设置响应消息 */
    pstRespMsg->stMsgHead.ulMsgID = pstOrgMsg->stMsgHead.ulMsgID;
    pstRespMsg->stMsgHead.usSrcMID = pstOrgMsg->stMsgHead.usDstMID;
    pstRespMsg->stMsgHead.usDstMID = pstOrgMsg->stMsgHead.usSrcMID;
    switch (pstOrgMsg->stMsgHead.usMsgType)
    {
        case MSG_CMM_ADD_NODE:
            pstRespMsg->stMsgHead.usMsgType = MSG_CMM_ADD_NODE_ACK;
            break;
        case MSG_CMM_INST_ADDED:
            pstRespMsg->stMsgHead.usMsgType = MSG_CMM_INST_ADDED_ACK;
            break;
        default:
            safe_free_msg(pstRespMsg);
            COMM_TRACE(INFO_ERROR"\n");
            return TBS_FAILED;
    }

    /* 消息体长度 */
    pstRespMsg->stMsgHead.ulBodyLength = ulMsgBodyLen;

    pcPos = pstRespMsg->szMsgBody;

    /* 添加结果 */
    SET_ULONG(pcPos, 0);

    /* 添加的Index */
    SET_ULONG(pcPos, ulObjNo);

    /* 实例状态 */
    SET_ULONG(pcPos, ulStatus);

    /* 发送响应消息 */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}

/**************************************************************************
功能: 响应删除实例消息
参数: const ST_MSG *pstOrgMsg                                           原消息;
             unsigned long ulStatus                                                 状态;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int RespDelMsg(const ST_MSG *pstOrgMsg, unsigned long ulStatus)
{
    int  iRet = TBS_SUCCESS;
    ST_MSG *pstRespMsg = NULL;
    unsigned long ulMsgBodyLen = 0;
    char *pcPos = NULL;

    if ( NULL == pstOrgMsg )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    ulMsgBodyLen = sizeof(unsigned long) * 2;

    /* 申请回应消息的内存 */
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_OUT_OF_MEM;
    }

    /* 设置响应消息 */
    pstRespMsg->stMsgHead.ulMsgID = pstOrgMsg->stMsgHead.ulMsgID;
    pstRespMsg->stMsgHead.usSrcMID = pstOrgMsg->stMsgHead.usDstMID;
    pstRespMsg->stMsgHead.usDstMID = pstOrgMsg->stMsgHead.usSrcMID;
    switch (pstOrgMsg->stMsgHead.usMsgType)
    {
        case MSG_CMM_DEL_NODE:
            pstRespMsg->stMsgHead.usMsgType = MSG_CMM_DEL_NODE_ACK;
            break;
        case MSG_CMM_INST_DELED:
            pstRespMsg->stMsgHead.usMsgType = MSG_CMM_INST_DELED_ACK;
            break;
        default:
            safe_free_msg(pstRespMsg);
            COMM_TRACE(INFO_ERROR"\n");
            return TBS_FAILED;
    }

    /* 消息体长度 */
    pstRespMsg->stMsgHead.ulBodyLength = ulMsgBodyLen;

    pcPos = pstRespMsg->szMsgBody;

    /* 删除结果 */
    SET_ULONG(pcPos, 0);

    /* 删除状态 */
    SET_ULONG(pcPos, ulStatus);

    /* 发送响应消息 */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}


/**************************************************************************
功能: 响应设置消息
参数: const ST_MSG *pstOrgMsg                                           原消息;
             unsigned long ulStatus                                                 状态;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int RespSetMsg(const ST_MSG *pstOrgMsg, unsigned long ulStatus)
{
    int  iRet = TBS_SUCCESS;
    ST_MSG *pstRespMsg = NULL;
    unsigned long ulMsgBodyLen = 0;
    char *pcPos = NULL;

    if (NULL == pstOrgMsg )
    {
        COMM_TRACE(INFO_ERROR"Invalide function prameters\n");
        return TBS_NULL_PTR;
    }

    ulMsgBodyLen = sizeof(unsigned long) * 2;

    /* 创建响应消息*/
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"Create response message failed\n");
        return TBS_OUT_OF_MEM;
    }

    /* 填充消息头部*/
    pstRespMsg->stMsgHead.usSrcMID = pstOrgMsg->stMsgHead.usDstMID;
    pstRespMsg->stMsgHead.usDstMID = pstOrgMsg->stMsgHead.usSrcMID;
    pstRespMsg->stMsgHead.ulMsgID = pstOrgMsg->stMsgHead.ulMsgID;
    pstRespMsg->stMsgHead.usMsgType = MSG_CMM_SET_VAL_ACK;

    /* 计算消息体长度*/
    pstRespMsg->stMsgHead.ulBodyLength = ulMsgBodyLen;

    pcPos = pstRespMsg->szMsgBody;

    /* 填充Set结果*/
    SET_ULONG(pcPos, 0);

    /* 填充状态*/
    SET_ULONG(pcPos, ulStatus);

    /* 发送响应消息 */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}

/**************************************************************************
功能: 响应查询消息
参数: const ST_MSG *pstOrgMsg                                           原消息;
             unsigned long ulStatus                                                 状态;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int RespGetMsg(const ST_MSG *pstOrgMsg, ST_ResultInfo *pstResultInfoList)
{
    int  iRet = TBS_SUCCESS;
    ST_MSG *pstRespMsg = NULL;
    unsigned long ulMsgBodyLen = 0;
    ST_ResultInfo *pstResultInfo = NULL;
    unsigned long ulCount = 0;
    char *pcPos = NULL;

    if (NULL == pstOrgMsg )
    {
        COMM_TRACE(INFO_ERROR"Invalid function prameters\n");
        return TBS_NULL_PTR;
    }

    /* 结果和个数*/
    ulMsgBodyLen = sizeof(unsigned long) * 2;

    /* 计算消息长度*/
    for ( pstResultInfo = pstResultInfoList; pstResultInfo; pstResultInfo = pstResultInfo->pstNext )
    {
        if ( NULL != pstResultInfo->pszStr )
        {
            ulMsgBodyLen += strlen(pstResultInfo->pszStr) + 1;
            ulCount++;
        }
    }

    /* 创建响应消息*/
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"Create response message failed \n");
        return TBS_OUT_OF_MEM;
    }

    /* 填充消息头部*/
    pstRespMsg->stMsgHead.usSrcMID     = pstOrgMsg->stMsgHead.usDstMID;
    pstRespMsg->stMsgHead.usDstMID     = pstOrgMsg->stMsgHead.usSrcMID;
    pstRespMsg->stMsgHead.ulMsgID      = pstOrgMsg->stMsgHead.ulMsgID;
    pstRespMsg->stMsgHead.usMsgType    = MSG_CMM_GET_VAL_ACK;

    pstRespMsg->stMsgHead.ulBodyLength = ulMsgBodyLen;

    pcPos = pstRespMsg->szMsgBody;

    /* 填充Get结果*/
    SET_ULONG(pcPos, 0);

    /* 结果个数*/
    SET_ULONG(pcPos, ulCount);

    /* 填充结果*/
    for ( pstResultInfo = pstResultInfoList; pstResultInfo; pstResultInfo = pstResultInfo->pstNext )
    {
        if ( NULL != pstResultInfo->pszStr )
        {
            SET_STR(pcPos, pstResultInfo->pszStr);
        }
    }

    /* 发送响应消息 */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}

/**************************************************************************
功能: 响应配置更新消息
参数: const ST_MSG *pstOrgMsg                                           原消息;
             unsigned long ulStatus                                                 结果;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int RespUpdateMsg(const ST_MSG *pstOrgMsg, unsigned long ulResult)
{
    int  iRet = TBS_SUCCESS;
    ST_MSG *pstRespMsg = NULL;
    unsigned long ulMsgBodyLen = 0;
    char *pcPos = NULL;

    if ( NULL == pstOrgMsg )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    ulMsgBodyLen = sizeof(unsigned long);

    /* 申请回应消息的内存 */
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_OUT_OF_MEM;
    }

    /* 设置响应消息 */
    pstRespMsg->stMsgHead.ulMsgID = pstOrgMsg->stMsgHead.ulMsgID;
    pstRespMsg->stMsgHead.usSrcMID = pstOrgMsg->stMsgHead.usDstMID;
    pstRespMsg->stMsgHead.usDstMID = pstOrgMsg->stMsgHead.usSrcMID;
    pstRespMsg->stMsgHead.usMsgType = MSG_CMM_UPDATE_ACK;

    /* 消息体长度 */
    pstRespMsg->stMsgHead.ulBodyLength = ulMsgBodyLen;

    pcPos = pstRespMsg->szMsgBody;

    /* 更新结果 */
    SET_ULONG(pcPos, ulResult);

    /* 发送响应消息 */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}


/**************************************************************************
功能: 通用获取配置值消息处理函数
参数: ST_MSG *pstMsg                                          原消息;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int GetMsgHandler(ST_MSG *pstMsg)
{
    int iRet = TBS_SUCCESS;

    unsigned long i = 0;
    unsigned long ulCount = 0;

    unsigned long ulTypeLen = 0;
    unsigned long ulValueLen = 0;

    char szNodeType[MAX_NODE_TYPE_LEN] = {0};
    char szNodeValue[MAX_NODE_VALUE_LEN] = {0};

    char *pcPos = NULL;

    ST_ResultInfo *pstResultList = NULL;
    ST_ErrInfo *pstErrList = NULL;

    if ( NULL == pstMsg )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    pcPos = pstMsg->szMsgBody;

    /* 获取查询项数目*/
    GET_ULONG(pcPos, ulCount);

    for (i = 0; i < ulCount; i++)
    {
        /* 读取配置获取节点值和类型*/
        ulTypeLen = MAX_NODE_TYPE_LEN;
        ulValueLen = MAX_NODE_VALUE_LEN;
        iRet = CFG_GetLeafValAndType(pcPos, szNodeValue, &ulValueLen, szNodeType, &ulTypeLen);
        if ( RET_SUCCEED(iRet) )
        {
            /* 保存查询结果到链表中*/
            AddResultInfo(&pstResultList, pcPos, szNodeType, szNodeValue);
        }
        else
        {
            AddErrInfo(&pstErrList, iRet, pcPos);
            COMM_TRACE(INFO_ERROR"\n");
            break;
        }

        /* 获取一条查询命令*/
        pcPos += strlen(pcPos) + 1;
    }

    if ( RET_SUCCEED(iRet) )
    {
        /* 响应查询消息*/
        iRet = RespGetMsg(pstMsg, pstResultList);
    }
    else
    {
        /* 发送错误消息 */
        iRet = RespErrMsg(pstMsg, iRet, pstErrList);
    }

    /* 释放字符串链表*/
    FreeAllResultInfo(&pstResultList);

    /* 释放错误信息链表 */
    FreeAllErrInfo(&pstErrList);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}


/*设置进程优先级*/
static int setvalue(char *pid, char *file, char *value)
{
    char path[64] = {0};
    char cmd[128] = {0};

    snprintf(path, sizeof(path), "/proc/%s/%s", pid, file);
    snprintf(cmd, sizeof(cmd), "echo %s > %s", value, path);

    return system(cmd);
}


/************************************************************
Function:      int pid_poll_set(char *pName, char *pFile, char *pValue)
Description:   设置进程优先级
Input:         char *pName, char *pFile, char *pValue
Return:        无
Others:        pid_poll_set("/usr/bin/pc", "oom_adj", "-17");
************************************************************/
int pid_poll_set(char *pName, char *pFile, char *pValue)
{
    FILE *fp = NULL;
    char readline[128] = {0};

    char cmd[64] = {0};
    char pid[16] = {9};

    int getflag = 0;

    snprintf(cmd, sizeof(cmd), "ps -A|grep %s|grep -v grep", pName);

    if (NULL != (fp = popen(cmd, "r")))
    {
            while (NULL != fgets(readline, sizeof(readline), fp))
            {
                    sscanf(readline, "%s", pid);
                    setvalue(pid, pFile, pValue);

                    memset(readline, 0, sizeof(readline));

                    getflag = 1;
            }

            pclose(fp);
    }

    return getflag;
}

/************************************************************
Function:      bool ModuleExist(const char *pszName)
Description:   判断kenel module是否已经加载
Input:         const char *pszName      模块名称
Return:        无
Others:        ModuleExist("NetUsb");
************************************************************/
int ModuleExist(const char *pszName)
{
    FILE *fp = NULL;
    char readline[256] = {0};
    char szModuleName[128] = {9};

    int iflag = 0;

    if (NULL != (fp = fopen("/proc/modules", "r")))
    {
            while (NULL != fgets(readline, sizeof(readline), fp))
            {
                sscanf(readline, "%s", szModuleName);

                if(0 == safe_strcmp(szModuleName, pszName))
                {
                    iflag = 1;
                    break;
                }

                memset(readline, 0, sizeof(readline));
            }

            fclose(fp);
    }

    return iflag;
}


