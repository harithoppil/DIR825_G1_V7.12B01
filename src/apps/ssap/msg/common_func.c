/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称: Common_fun.c
 文件描述: 公共函数的封装，提供给各模块使用

 修订记录:
        1. 作者: 李伟
           日期: 2008-08-07
           内容: 创建文件

**********************************************************************/

#include "warnlog.h"
#include "common.h"
#include "new_msg.h"
#include "cache_list.h"

#include "common_func.h"
#include "common_msg_handler.h"

#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 *          extern define
 */

extern ST_MODULE_NODE g_Module_Table[];


/**************************************************************************/


/*************************************************************************
Function:      int COMM_NullFunc(char *pszPath)
Description:   空函数，do noting
Input:         char *pszPath                       当前处理的路径
Output:        无
Return:        无
Others:
*************************************************************************/
int COMM_NullFunc(char *pszPath)
{
    return TBS_SUCCESS;
}


/*************************************************************************
Function:      void COMM_RemovePathLastDot(char *pszPath)
Description:   去掉路径最后的'.'
Input:         char *pszPath                       当前处理的路径
Output:        无
Return:        无
Others:
*************************************************************************/
void COMM_RemovePathLastDot(char *pszPath)
{
    unsigned int nLen = 0;
    if ( pszPath == NULL)
        return;

    nLen = strlen(pszPath);
    if (pszPath[nLen-1] == '.')
        pszPath[nLen-1] = '\0';
}


/*************************************************************************
Function:      void COMM_CopyPathNoLastDot(char *pszDestPath, const char *pszSrcPath)
Description:   复制路径,复制时去掉路径最后的'.'
Input:         const char *pszSrcPath,                       当前处理的路径
                   char *pszDestPath,                         拷贝的目标路径
Output:        无
Return:        无
Others:
*************************************************************************/
void COMM_CopyPathNoLastDot(char *pszDestPath, const char *pszSrcPath)
{
    unsigned int nLen = 0;
    if ( pszSrcPath == NULL || pszDestPath == NULL)
        return;

    strcpy(pszDestPath, pszSrcPath);
    nLen = strlen(pszDestPath);
    if (pszDestPath[nLen-1] == '.')
        pszDestPath[nLen-1] = '\0';
}



/*************************************************************************
Function:      int COMM_GetPathEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath,char *pszDestPath,unsigned long *pulPathLen)
Description:   通过现有的路径拼装新路径
Calls:         无
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       当前处理的路径
                   int iChangeLevel,                         改变的路径层数(0或负值，表示从路径右边去掉的节点数)
                   char *pszExtraPath                     附加 的路径，为NULL则表示不添加(后面没有.)
Output:        无
Return:        NULL ,       失败;
                    其它,     成功，值的指针
Others:
*************************************************************************/
#define MAX_PATH_DEPTH 128
int COMM_GetPathEx(const char *pszCurPath,int iChangeLevel, const char *pszExtraPath,
                   char *pszDestPath,unsigned long *pulPathLen)
{
    int i=0;
    int iRet = TBS_SUCCESS;

    char szTempPath[MAX_PATH_LEN] = {0};

    char **papcAttrList = NULL;
    unsigned char ulCount=0;
    int cCurCount = 0;
    //unsigned short ulTotalSize = 0;

    /* 实现类似于cd ../../xx,这里先处理../.. */
    if (iChangeLevel>0)
        return TBS_PARAM_ERR;
    else if (iChangeLevel < 0)
    {
        papcAttrList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );
        safe_strncpy(szTempPath,pszCurPath,MAX_PATH_LEN);

        papcAttrList[ulCount] = strtok(szTempPath, ".");
        while (papcAttrList[ulCount])
        {
            ulCount++;
            papcAttrList[ulCount] = strtok(NULL, ".");
        }

        cCurCount = ulCount + iChangeLevel;
        if (cCurCount<0)
        {
            iRet = TBS_PARAM_ERR;
            goto EXIT;
        }

        pszDestPath[0] = '\0';
        for (i=0;i<cCurCount;i++)
        {
            if (papcAttrList[i])
                sprintf(pszDestPath,"%s%s.",pszDestPath,papcAttrList[i]);
        }
    }
    else
    {
        safe_strncpy(szTempPath,pszCurPath,MAX_PATH_LEN);
        pszDestPath[0] = '\0';
        if (szTempPath[strlen(szTempPath)-1] != '.')
        {
            sprintf(pszDestPath, "%s.", szTempPath);
        }
        else
            strcpy(pszDestPath, szTempPath);
    }

    /* 加入 ..后的路径*/
    if (pszExtraPath)
    {
        sprintf(pszDestPath,"%s%s",pszDestPath,pszExtraPath);
    }
    else
    {
        /* pszExtraPath为NULL,则去掉最后的'.' */
        if (strlen(pszDestPath))
            pszDestPath[strlen(pszDestPath)-1] = '\0';
    }

    if (pulPathLen != NULL)
    {
        *pulPathLen = strlen(pszDestPath);
    }

    iRet = TBS_SUCCESS;

EXIT:
    safe_free(papcAttrList);
    return iRet;
}

/*************************************************************************
Function:    char *  COMM_GetPathNode(char *pszCurPath,int iPos)
Description:   获取路径中的第n 个节点
Calls:         无
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       当前处理的路径
                   int iPos,                                        获取第几个节点
Output:        无
Return:        NULL ,       失败;
                    其它,     成功，值的指针
Others:         为了简化接口，这里使用了全局变量存储返回值，
                    所以不支持多线程的并发调用
*************************************************************************/
char szTempPathNode[MAX_PATH_LEN] = {0};
const char * COMM_GetPathNode(char *pszCurPath,int iPos)
{
    unsigned char ulCount=0;
    char *pReturnVal = NULL;

    char **papcAttrList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );

    safe_strncpy(szTempPathNode,pszCurPath,MAX_PATH_LEN);

    /* 全部解析出所有.分割的路径*/
    papcAttrList[ulCount] = strtok(szTempPathNode, ".");
    while (papcAttrList[ulCount])
    {
        ulCount++;
        papcAttrList[ulCount] = strtok(NULL, ".");
    }

    /* 根据位置参数，传出返回值*/
    if (iPos>0&&iPos<=ulCount)
        pReturnVal = papcAttrList[iPos-1];
    else if (iPos<0&&iPos>=-ulCount)
        pReturnVal = papcAttrList[ulCount+iPos];
    else
        pReturnVal = NULL;

    free(papcAttrList);
    return pReturnVal;

}


/*************************************************************************
Function:    int  COMM_GetPathIndex(char *pszCurPath,int *iIndex)
Description:   获取路径中的index
Calls:         无
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       当前处理的路径
Output:        int *iIndex
Return:        当前index的数量
Others:         为了简化接口，这里使用了全局变量存储返回值，
                    所以不支持多线程的并发调用
*************************************************************************/
int  COMM_GetPathIndex(char *pszCurPath,int *iIndex)
{
    int iTempIndex = 0;
    unsigned char ucCount=0;
    char *pTemp = NULL;

    char szTempPathNode[MAX_PATH_LEN] = {0};
    safe_strncpy(szTempPathNode,pszCurPath,MAX_PATH_LEN);

    pTemp = strtok(szTempPathNode, ".");
    while (pTemp)
    {
        iTempIndex = 0;
        while (*pTemp!='\0')
        {
            /*记下所有的数字*/
            if ((*pTemp<'0')||(*pTemp>'9'))
                break;
            else
                iTempIndex = iTempIndex*10+(*pTemp-'0');

            pTemp++;
        }

        if (*pTemp=='\0')
        {
            iIndex[ucCount++] = iTempIndex;
        }

        pTemp = strtok(NULL, ".");
    }

    return ucCount;
}

/*************************************************************************
Function:      char * COMM_GetNodeValue(char *pszPath,ValuePostionType *eGetMode)
Description:   统一的获取值的函数，包括从缓存获取，从配置树获取和先从缓存后从配置树三种方式
Calls:         无
Data Accessed:
Data Updated:
Input:         char *pszPath ,                       当前处理的路径
                   ValuePostionType eGetMode  获取值得方式
Output:        无
Return:        NULL ,       失败;
                    其它,     成功，值的指针
Others:
*************************************************************************/
static ValuePostionType g_enValuePos = FROM_CACHE;

ValuePostionType COMM_GetLastPos()/*返回最近一次COMM_GetNodeValue函数获取值的位置*/
{
    return g_enValuePos;
}

const char * COMM_GetNodeValue(char *pszPath,ValuePostionType eGetMode)
{
    const char *pValue = NULL;
    int iRet = TBS_SUCCESS;


    if (eGetMode == FROM_CACHE)
    {
        /*为不影响接口临时使用，会在后面统一修改*/
        pValue = GetCacheNodeEx(pszPath);
        if (pValue)
        {
            g_enValuePos = FROM_CACHE;
        }
    }
    else if (eGetMode == FROM_CFG)
    {
        iRet = CFG_GetNodeValPtr(pszPath, &pValue, NULL);
        if ( RET_FAILED(iRet) )
        {
            pValue = NULL;
        }
        g_enValuePos = FROM_CFG;
    }
    else if (eGetMode == CACHE_CFG)
    {
        /*为不影响接口临时使用，会在后面统一修改*/
        pValue = GetCacheNodeEx(pszPath);
        if (pValue)
        {
            g_enValuePos = FROM_CACHE;
        }

        if (!pValue)
        {
            iRet = CFG_GetNodeValPtr(pszPath, &pValue, NULL);
            if ( RET_FAILED(iRet) )
            {
                pValue = NULL;
            }
            else
            {
                g_enValuePos = FROM_CFG;
            }
        }
        else
        {
            g_enValuePos = FROM_CACHE;
        }
    }

    return (char *)pValue;
}

/*************************************************************************
Function:    char * COMM_GetNodeValueEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath,ValuePostionType eGetMode)
Description:  获取值的封装接口，支持:
                    1.对想获取值的路径可以通过从现有路径拼装获得
                    2.支持从缓存，从配置树，先缓存后配置树等几种方式获得节点值
Calls:         无
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       当前处理的路径
                   int iChangeLevel,                         改变的路径层数(0或负值，表示从路径右边去掉的节点数)
                   char *pszExtraPath                     附加 的路径，为NULL则表示不添加
                   ValuePostionType eGetMode        获取值的方式(    FROM_CACHE 从缓存, FROM_CFG 从配置树,  CACHE_CFG 先缓存后配置树)
Output:        无
Return:        NULL ,       失败;
                    其它,     成功，值的指针
Others:
*************************************************************************/
const char * COMM_GetNodeValueEx(const char *pszCurPath,int iChangeLevel,
                    const char *pszExtraPath,ValuePostionType eGetMode)
{
    char szTempPath[MAX_PATH_LEN] = {'\0'};
    unsigned long ulCount = MAX_PATH_LEN;

    const char *pVal = NULL;
    int iRet =TBS_SUCCESS;

    iRet = COMM_GetPathEx(pszCurPath,iChangeLevel,pszExtraPath,szTempPath,&ulCount);
    if (iRet != TBS_SUCCESS)
    {
        pVal = NULL;
    }

    pVal = COMM_GetNodeValue(szTempPath,eGetMode);

    return pVal;
}


/*************************************************************************
Function:    COMM_GetMultiNodeValueEx
Description:  获取多个节点值的封装接口，支持:对路径可以通过从现有路径拼装获得
                    1.对想获取值的路径可以通过从现有路径拼装获得
                    2.支持从缓存，从配置树，先缓存后配置树等几种方式获得节点值
Input:         char *pszCurPath ,               当前处理的路径
               int iChangeLevel,                改变的路径层数(0或负值，表示从路径右边去掉的节点数)
               ValuePostionType eGetMode        获取值的方式  (    FROM_CACHE 从缓存, FROM_CFG 从配置树,  CACHE_CFG 先缓存后配置树)
               unsigned short usNodeCount       节点个数
               ...                              指针变参,用来接收获取的节点值,
                                                如: char *pcNodeName, char **ppcNodeValue
Output:        无
Return:        成功:TBS_SUCCESS
               失败:错误码
Others:
*************************************************************************/
int COMM_GetMultiNodeValueEx(const char *pszCurPath, int iChangeLevel,
                             ValuePostionType eGetMode, unsigned short usNodeCount, ...)
{
    int iRet = TBS_SUCCESS, i = 0;
    va_list arg_ptr;
    char *pszNodeName = NULL;
    char **ppszNodeValue = NULL;
    char szTempPath[MAX_PATH_LEN] = {0};
    char *pszPos = szTempPath;
    unsigned long ulBufLen = 0;

    /* 拼接路径前缀,下面不需要重复拼接 */
    iRet = COMM_GetPathEx(pszCurPath, iChangeLevel, "", szTempPath, NULL);
    if ( RET_FAILED(iRet) )
    {
        return iRet;
    }

    pszPos += strlen(szTempPath);
    ulBufLen = MAX_PATH_LEN - (pszPos - szTempPath);

    /* 逐个节点获取值 */
    va_start(arg_ptr, usNodeCount);
    for (i = 0; i < usNodeCount; i++)
    {
        pszNodeName = va_arg(arg_ptr,char *);
        ppszNodeValue = va_arg(arg_ptr,char **);

        /* 检测是否传入错误的参数 */
        if ((NULL == ppszNodeValue) || (NULL == pszNodeName))
        {
            COMMON_TRACE(MID_CCP, "Error:  NodeName or  NodeValue Point to NULL.\n");
            return TBS_NULL_PTR;
        }

        snprintf(pszPos, ulBufLen, "%s", pszNodeName);
        (*ppszNodeValue) = (char *)COMM_GetNodeValue(szTempPath, eGetMode);
        if (NULL == (*ppszNodeValue))
        {
            COMMON_TRACE(MID_CCP, "Get Node value failed, Node Path = %s.\n", szTempPath);
            return  ERR_GET_NODE_VALUE_FAIL;
        }
    }

    va_end(arg_ptr);
    return TBS_SUCCESS;
}

#if 1

/*************************************************************************
Function:    COMM_GetMultiNodeValueByStrArrEx
Description:  通过传入节点名数组来获取多个节点值
              支持:对路径可以通过从现有路径拼装获得
                1.对想获取值的路径可以通过从现有路径拼装获得
                2.支持从缓存，从配置树，先缓存后配置树等几种方式获得节点值
Input:         char *pszCurPath ,               当前处理的路径
               int iChangeLevel,                改变的路径层数(0或负值，表示从路径右边去掉的节点数)
               ValuePostionType eGetMode        获取值的方式  (FROM_CACHE 从缓存, FROM_CFG 从配置树,  CACHE_CFG 先缓存后配置树)
               unsigned short usNodeCount       节点个数
               const char * apcNodeArr[]        节点数组，节点数组中为空的节点忽略
               void *pvNodeValArr               用来接收节点值的结构体或者字符数组
Output:        无
Return:        成功:TBS_SUCCESS
               失败:错误码
Others:
*************************************************************************/
int COMM_GetMultiNodeValueByStrArrEx(const char *pszCurPath, int iChangeLevel,
        ValuePostionType eGetMode, unsigned short usNodeCount,
        const char *apcNodeArr[], void *pvNodeValArr)
{
    int iRet = TBS_SUCCESS, i = 0;
    char szTempPath[MAX_PATH_LEN] = {0};
    char *pszPos = szTempPath;
    unsigned long ulBufLen = 0;
    char **ppOut = pvNodeValArr;

    /* 拼接路径前缀,下面不需要重复拼接 */
    iRet = COMM_GetPathEx(pszCurPath, iChangeLevel, "", szTempPath, NULL);
    if ( RET_FAILED(iRet) )
    {
        return iRet;
    }

    pszPos += strlen(szTempPath);
    ulBufLen = MAX_PATH_LEN - (pszPos - szTempPath);

    if (eGetMode == FROM_CFG)
    {
        iRet = CFG_GetValToStrArr(szTempPath, pvNodeValArr, (char * const *)apcNodeArr, usNodeCount);
    }
    else if (eGetMode == FROM_CACHE || eGetMode == CACHE_CFG)
    {
        for (i = 0; i < usNodeCount; i++)
        {
            /* 忽略空节点 */
            if ('\0' == apcNodeArr[i][0])
            {
                *(ppOut + i) = NULL;
                continue;
            }

            snprintf(pszPos, ulBufLen, "%s", apcNodeArr[i]);
            *(ppOut + i) = (char *)COMM_GetNodeValue(szTempPath, eGetMode);
            if (NULL == *(ppOut + i))
            {
                COMMON_TRACE(MID_CCP, "Get Node value failed, Node Path = %s.\n", szTempPath);
                return  ERR_GET_NODE_VALUE_FAIL;
            }
        }
    }

    return TBS_SUCCESS;
}



/*************************************************************************
Function:     COMM_SetMultiNodeValueByStrArrEx
Description:  通过传入节点名数组和节点值数组来同时设置多个节点值
              支持: 对路径可以通过从现有路径拼装获得
Input:        char *pszCurPath ,               当前处理的路径
              int iChangeLevel,                改变的路径层数(0或负值，表示从路径右边去掉的节点数)
              unsigned short usNodeCount       节点个数
              const char * apcNodeArr[]        节点数组，节点数组中为空的节点忽略
              void *pvNodeValArr               用来设置的节点值字符数组或者结构体(所有成员都是字符指针)
Output:       无
Return:       成功:TBS_SUCCESS
              失败:错误码
Others:
*************************************************************************/
int COMM_SetMultiNodeValueByStrArrEx(const char *pszCurPath, int iChangeLevel,
        unsigned short usNodeCount, const char *apcNodeArr[], void *pvNodeValArr)
{
    int iRet = TBS_SUCCESS;
    char szTempPath[MAX_PATH_LEN] = {0};

    /* 拼接路径前缀,下面不需要重复拼接 */
    iRet = COMM_GetPathEx(pszCurPath, iChangeLevel, "", szTempPath, NULL);
    if ( RET_FAILED(iRet) )
    {
        return iRet;
    }

    iRet = CFG_SetValFromStrArr(szTempPath, pvNodeValArr, (char * const *)apcNodeArr, usNodeCount);
    if (RET_FAILED(iRet))
    {
        COMMON_TRACE(MID_CCP,"Set Node value failed, Node Path = %s.\n", szTempPath);
        return  ERR_SET_NODE_VALUE_FAIL;
    }

    return TBS_SUCCESS;
}

#endif


/*************************************************************************
Function:    int  COMM_SetValueEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath,char *pszValue)
Description:  设置值的封装接口，支持:对想设置值的路径可以通过从现有路径拼装获得
Calls:         无
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       当前处理的路径
                   int iChangeLevel,                         改变的路径层数(0或负值，表示从路径右边去掉的节点数)
                   char *pszExtraPath                     附加 的路径，为NULL则表示不添加
                   char *pszValue                            设置的值
Output:        无
Return:        TBS_SUCCESS ,      成功;
                    其它,     失败
Others:
*************************************************************************/
int COMM_SetValueEx(const char *pszCurPath,int iChangeLevel,char *pszExtraPath, const char *pszValue)
{
    unsigned long ulCount = MAX_PATH_LEN;
    char szTempPath[MAX_PATH_LEN] = {'\0'};

    int iRet =TBS_SUCCESS;

    iRet = COMM_GetPathEx(pszCurPath,iChangeLevel,pszExtraPath,szTempPath,&ulCount);
    if (iRet != TBS_SUCCESS)
    {
        return  iRet;
    }

    iRet = CFG_SetNodeVal(szTempPath,pszValue,NULL);
    return iRet;
}


/*************************************************************************
Function:       COMM_SetMultiNodeValueEx
Description:    设置多个节点值的封装接口，支持:对想设置值的路径可以通过从现有路径拼装获得
Calls:          无
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,               当前处理的路径
               int iChangeLevel,                改变的路径层数(0或负值，表示从路径右边去掉的节点数)
               unsigned short usNodeCount       设置节点的个数
               ...                              指针变参,用来接收要设置的节点名称和节点值,
                                                如: char *pcNodeName, char *pcNodeValue
Output:        无
Return:        TBS_SUCCESS ,      成功;
                    其它,     失败
Others:
*************************************************************************/
int COMM_SetMultiNodeValueEx(const char *pszCurPath, int iChangeLevel, unsigned short usNodeCount, ...)
{
    int iRet = TBS_SUCCESS, i = 0;
    va_list arg_ptr;
    char *pszNodeName = NULL;
    char *pszNodeValue = NULL;
    char szTempPath[MAX_PATH_LEN] = {0};
    char *pszPos = szTempPath;
    unsigned long ulBufLen = 0;

    /* 拼接路径前缀,下面不需要重复拼接 */
    iRet = COMM_GetPathEx(pszCurPath,iChangeLevel,"",szTempPath, NULL);
    if ( RET_FAILED(iRet) )
    {
        return iRet;
    }

    pszPos += strlen(szTempPath);
    ulBufLen = MAX_PATH_LEN - (pszPos - szTempPath);

    /* 逐个节点设置值 */
    va_start(arg_ptr, usNodeCount);
    for (i = 0; i < usNodeCount; i++)
    {
        pszNodeName = va_arg(arg_ptr,char *);
        pszNodeValue = va_arg(arg_ptr,char *);

        /* 检测是否传入错误的参数 */
        if ((NULL == pszNodeValue) || (NULL == pszNodeName))
        {
            COMMON_TRACE(MID_CCP, "Error:  NodeName or  NodeValue Point to NULL.\n");
            return TBS_NULL_PTR;
        }

        snprintf(pszPos, ulBufLen, "%s", pszNodeName);
        iRet = CFG_SetNodeVal(szTempPath, pszNodeValue, NULL);
        if ( RET_FAILED(iRet) )
        {
            COMMON_TRACE(MID_CCP,"Set Node value failed, Node Path = %s.\n", szTempPath);
            return  ERR_SET_NODE_VALUE_FAIL;
        }
    }

    va_end(arg_ptr);
    return TBS_SUCCESS;
}

/*************************************************************************
Function:       COMM_SetMultiCacheValueEx
Description:    设置多个节点缓存的封装接口

Input:         usMID                    节点值所属的MID
               char *pszCurPath ,       当前处理的路径
               int iChangeLevel,        改变的路径层数(0或负值，表示从路径右边去掉的节点数)
               usNodeCount              设置节点的个数,为0时表示使用的变参为ST_SET_NODE_VAL_ENTRY
                                        数组。大于等于1时表示使用的变参为节点名称和值的指针。
               ...                      (usNodeCount等于0时)指向ST_SET_NODE_VAL_ENTRY数组的指针,
                                        该数组中最后的元素必须为NULL指针以表示数组的结尾。
                                        (usNodeCount>0时)指针变参,形式
                                        如: char *pcNodeName, char *pcNodeValue
Return:        TBS_SUCCESS ,      成功;
               其它,              失败
Others:
*************************************************************************/
int COMM_SetMultiCacheValueEx(unsigned short usMID, const char *pszCurPath,
                              int iChangeLevel, unsigned short usNodeCount, ...)
{
    int iRet;
    va_list arg_ptr;
    char szTempPath[MAX_PATH_LEN] = {0};
    char *pszPos = szTempPath;
    unsigned long ulBufLen = 0;

    /*拼接路径前缀*/
    iRet = COMM_GetPathEx(pszCurPath, iChangeLevel, "", szTempPath, NULL);
    if (RET_FAILED(iRet))
    {
        return iRet;
    }

    pszPos += strlen(szTempPath);
    ulBufLen = MAX_PATH_LEN - (pszPos - szTempPath);

    va_start(arg_ptr, usNodeCount);

    if (usNodeCount == 0)
    {
        ST_SET_NODE_VAL_ENTRY *pstNodeValueTab = va_arg(arg_ptr, ST_SET_NODE_VAL_ENTRY *);
        if (pstNodeValueTab == NULL)
        {
            return TBS_FAILED;
        }

        for (; pstNodeValueTab->pszNodeName != NULL; pstNodeValueTab++)
        {
            if (pstNodeValueTab->pszNodeValue == NULL)
            {
                continue;
            }

            snprintf(pszPos, ulBufLen, "%s", pstNodeValueTab->pszNodeName);
            iRet = COMM_SetCacheValue(usMID, szTempPath, pstNodeValueTab->pszNodeValue);
            if (RET_FAILED(iRet))
            {
                COMMON_TRACE(MID_CCP,"Set Node cache value failed, Node Path = %s.\n", szTempPath);
                return iRet;
            }
        }
    }
    else
    {
        char *pszNodeName = NULL;
        char *pszNodeValue = NULL;
        int i = 0;
        for ( ; i < usNodeCount; i++)
        {
            pszNodeName = va_arg(arg_ptr,char *);
            pszNodeValue = va_arg(arg_ptr,char *);

            /* 检测是否传入错误的参数 */
            if ((NULL == pszNodeValue) || (NULL == pszNodeName))
            {
                COMMON_TRACE(MID_CCP, "Error:  NodeName or  NodeValue Point to NULL.\n");
                return TBS_NULL_PTR;
            }

            snprintf(pszPos, ulBufLen, "%s", pszNodeName);
            iRet = COMM_SetCacheValue(usMID, szTempPath, pszNodeValue);
            if (RET_FAILED(iRet))
            {
                COMMON_TRACE(MID_CCP,"Set Node cache value failed, Node Path = %s.\n", szTempPath);
                return iRet;
            }
        }
    }

    va_end(arg_ptr);
    return TBS_SUCCESS;
}

/*************************************************************************
Function:       COMM_GetMultiCacheValueEx
Description:    获取多个节点缓存值的封装接口
                1.对想获取缓存值的路径可以通过从现有路径拼装获得

Input:   usMID                       节点所属的MID
         char *pszCurPath ,          当前处理的路径
         int iChangeLevel,           改变的路径层数(0或负值，表示从路径右边去掉的节点数)
         unsigned short usNodeCount  设置节点的个数
         ...                         指针变参,用来接收要设置的节点名称和节点值,
                                     如: char *pcNodeName, char **ppcNodeValue
Return:        TBS_SUCCESS , 成功;
               其它,         失败
Others:
*************************************************************************/
int COMM_GetMultiCacheValueEx(unsigned short usMID, const char *pszCurPath,
                              int iChangeLevel, unsigned short usNodeCount, ...)
{
    int iRet = TBS_SUCCESS, i = 0;
    char szTempPath[MAX_PATH_LEN] = {0};
    char *pszPos = szTempPath;
    va_list arg_ptr;
    char *pszNodeName = NULL;
    const char **ppszNodeValue = NULL;
    unsigned long ulBufLen = 0;

    /*拼接路径前缀*/
    iRet = COMM_GetPathEx(pszCurPath, iChangeLevel, "", szTempPath, NULL);
    if (RET_FAILED(iRet))
    {
        return iRet;
    }

    pszPos += strlen(szTempPath);
    ulBufLen = MAX_PATH_LEN - (pszPos - szTempPath);

    /* 逐个节点获取值 */
    va_start(arg_ptr, usNodeCount);
    for (i = 0; i < usNodeCount; i++)
    {
        pszNodeName = va_arg(arg_ptr,char *);
        ppszNodeValue = va_arg(arg_ptr,const char **);

        /* 检测是否传入错误的参数 */
        if (NULL == ppszNodeValue ||
            NULL == pszNodeName)
        {
            COMMON_TRACE(MID_CCP, "Error:  NodeName or  NodeValue Point to NULL.\n");
            return TBS_NULL_PTR;
        }

        snprintf(pszPos, ulBufLen, "%s", pszNodeName);
        (*ppszNodeValue) = COMM_GetCacheValue(usMID, szTempPath);
    }
    va_end(arg_ptr);

    return TBS_SUCCESS;
}

/*************************************************************************
Function:       COMM_DelMultiCacheNode
Description:    删除多个节点缓存值的封装接口
                1.对想删除缓存值的路径可以通过从现有路径拼装获得

Input:   usMID                       节点所属的MID
         char *pszCurPath ,          当前处理的路径
         int iChangeLevel,           改变的路径层数(0或负值，表示从路径右边去掉的节点数)
         unsigned short usNodeCount  设置节点的个数
         ...                         指针变参, 指向要删除缓存的节点名称
Return:        TBS_SUCCESS , 成功;
               其它,         失败
Others:
*************************************************************************/
int COMM_DelMultiCacheNode(unsigned short usMID, const char *pszCurPath,
                           int iChangeLevel, unsigned short usNodeCount, ...)
{
    int iRet = TBS_SUCCESS, i = 0;
    char szTempPath[MAX_PATH_LEN] = {0};
    char *pszPos = szTempPath;
    va_list arg_ptr;
    char *pszNodeName = NULL;
    unsigned long ulBufLen = 0;

    /*拼接路径前缀*/
    iRet = COMM_GetPathEx(pszCurPath, iChangeLevel, "", szTempPath, NULL);
    if (RET_FAILED(iRet))
    {
        return iRet;
    }

    pszPos += strlen(szTempPath);
    ulBufLen = MAX_PATH_LEN - (pszPos - szTempPath);

    /* 逐个删除节点缓存 */
    va_start(arg_ptr, usNodeCount);
    for (i = 0; i < usNodeCount; i++)
    {
        pszNodeName = va_arg(arg_ptr,char *);
        if (pszNodeName == NULL)
        {
            COMMON_TRACE(MID_CCP, "Error:  NodeName Point to NULL.\n");
            return  TBS_NULL_PTR;
        }

        snprintf(pszPos, ulBufLen, "%s", pszNodeName);
        COMM_DelCacheNode(usMID, szTempPath);
    }

    va_end(arg_ptr);

    return TBS_SUCCESS;
}

/*************************************************************************
Function:    int  COMM_SetCacheValueEx(char *pszCurPath,int iChangeLevel,
                                       char *pszExtraPath,char *pszValue)
Description:  设置缓存值的封装接口，支持:对想设置值的路径可以通过从现有路径拼装获得
Calls:         无
Data Accessed:
Data Updated:
Input:     unsigned short usMD   调用模块MID
               char *pszCurPath,       当前处理的路径
               int iChangeLevel,         改变的路径层数(0或负值，表示从路径右边去掉的节点数)
               char *pszExtraPath     附加的路径，为NULL则表示不添加
               char *pszValue            设置的值
Output:        无
Return:        TBS_SUCCESS ,      成功;
               其它,     失败
Others:
*************************************************************************/
int COMM_SetCacheValueEx(unsigned short usMID,const char *pszCurPath,int iChangeLevel,
                         char *pszExtraPath,char *pszValue)
{
    unsigned long ulCount = MAX_PATH_LEN;
    char szTempPath[MAX_PATH_LEN] = {'\0'};

    int iRet =TBS_SUCCESS;

    iRet = COMM_GetPathEx(pszCurPath,iChangeLevel,pszExtraPath,szTempPath,&ulCount);
    if (iRet != TBS_SUCCESS)
    {
        return  iRet;
    }

    iRet = COMM_SetCacheValue(usMID,szTempPath,pszValue);
    return iRet;
}

/*************************************************************************
Function:
     const char *COMM_GetCacheValueEx(unsigned short usMID, const char *pszCurPath,
                                      int iChangeLevel, char *pszExtraPath)

Description:  获取缓存值的封装接口，支持:对想设置值的路径可以通过从现有路径拼装获得

Input:     usMID,          调用模块MID
           pszCurPath,     当前处理的路径
           iChangeLevel,   改变的路径层数(0或负值，表示从路径右边去掉的节点数)
           pszExtraPath    附加的路径，为NULL则表示不添加

Return:    缓存值指针,   成功
           NULL,         失败
Others:
*************************************************************************/
const char *COMM_GetCacheValueEx(unsigned short usMID, const char *pszCurPath,
                                int iChangeLevel, char *pszExtraPath)
{
    char szTempPath[MAX_PATH_LEN] = {0};
    int iRet =TBS_SUCCESS;

    iRet = COMM_GetPathEx(pszCurPath,iChangeLevel,pszExtraPath,szTempPath, NULL);
    if (iRet != TBS_SUCCESS)
    {
        return  NULL;
    }

    return COMM_GetCacheValue(usMID, szTempPath);
}

/*************************************************************************
Function: void COMM_SynCache2CFG(unsigned short usMID)

Description:  将模块缓存值保存到配置树中

Input:     usMID,   模块MID

Return:   无

Others:
*************************************************************************/
void COMM_SynCache2CFG(unsigned short usMID)
{
    int i = MID2INDEX(usMID);

    SaveCache2CFG(&(g_Module_Table[i].listCache));
    return;
}

/************************************************************************/
BOOL COMM_IsPathInCache(unsigned short usMID, char *pszPath)
{
    int i = MID2INDEX(usMID);

    return CacheList_IsInList(&g_Module_Table[i].listCache, pszPath, FALSE);
}
#define MAX_NODE_CNT 32
/*************************************************************************
功能: 检查指定路径下实例之间是否有节点值冲突(多个节点)
参数: char *pszMatchPath           需要检查的通配路径(如果传入NULL则根据当前路径自动计算)
      char *pszCurPath             需要检查的当前路径
      int iChangeLevel             改变的路径层数(0或负值，表示从路径右边去掉的节点数)
      int  iNodeCnt                节点个数
      char **apcNodeArr            待检查的节点数组
返回: 成功 -- TBS_SUCCESS
      失败 -- 其它错误码
备注:
**************************************************************************/
int COMM_CheckConflictNodes(const char *pszMatchPath, const char *pszCurPath,
                            int iChangeLevel, int iNodeCnt, char * const *apcNodeArr)
{
    int iRet = TBS_SUCCESS;
    char szMatchPath[MAX_PATH_LEN] = {0};
    ST_PARA_VAL *pstParaTmp = NULL;
    ST_PARA_VAL *pstParaList = NULL;
    int i = 0;
    int iConflict = 0;

    char *apcCurNodeValArr[MAX_NODE_CNT] = {0};
    char *apcTmpNodeValArr[MAX_NODE_CNT] = {0};

    if (iNodeCnt > MAX_NODE_CNT)
    {
        COMMON_TRACE(MID_CCP, "too much node in node array\n");
        return TBS_FAILED;
    }

    /*
      从缓存中获取当前实例节点值
      InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.1.Name
      InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.1.
    */
    iRet = COMM_GetMultiNodeValueByStrArrEx(pszCurPath, iChangeLevel, CACHE_CFG,
            iNodeCnt, (const char **)apcNodeArr, apcCurNodeValArr);
    RET_RETURN(MID_CCP, iRet, "Get instance sub node failed, Path = [%s]\n", pszCurPath);

    if (NULL == pszMatchPath)
    {
        /*
          拼接路径，得到实例的根路径
          InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.1.Name
          InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.
        */
        iRet = COMM_GetPathEx(pszCurPath, iChangeLevel - 1, "", szMatchPath, NULL);
        if (RET_FAILED(iRet))
        {
            return iRet;
        }
    }
    else
    {
        safe_strncpy(szMatchPath, pszMatchPath, MAX_PATH_LEN);
    }

    /*
      获取路径下所有实例
      InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.
    */
    iRet = CFG_GetMatchNodeName(szMatchPath, 1, &pstParaList, NULL);
    RET_RETURN(MID_CCP, iRet, "get node list error!\n");

    /* 遍历所有实例，依次比较 */
    for(pstParaTmp = pstParaList; NULL != pstParaTmp; pstParaTmp = pstParaTmp->pstNext)
    {
        /*
          跳过自身
          pszCurPath/pstParaBuf->pcName:
          InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.1.Name
          InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.1.
         */
        if (safe_strstr(pszCurPath, pstParaTmp->pcName))
        {
            continue;
        }

        /* 从配置中获取实例节点值 */
        iRet = COMM_GetMultiNodeValueByStrArrEx(pstParaTmp->pcName, 0, FROM_CFG,
                iNodeCnt, (const char **)apcNodeArr, apcTmpNodeValArr);
        if (RET_FAILED(iRet))
        {
            COMMON_TRACE(MID_CCP, "Get instance sub node failed, Path = [%s]\n", pstParaTmp->pcName);
            break;
        }

        iConflict = 1;

        /* 遍历节点数组, 比较所有节点值 */
        for (i = 0; i < iNodeCnt; i++)
        {
            /* 只要有一个节点值不等，就不会发生冲突 */
            if (0 != safe_strcmp(apcCurNodeValArr[i], apcTmpNodeValArr[i]))
            {
                iConflict = 0;
                break;
            }
        }

        if (iConflict)
        {
            /* 比较完毕，所有节点值相等，说明发生冲突 */
            iRet = TBS_FAILED;
            break;
        }
    }

    /* 释放内存 */
    CFG_MemFree(pstParaList);

    return iRet;
}

/*************************************************************************
Function:    int COMM_AddObjectEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath)
Description:  添加object的封装接口，支持:对想设置值的路径可以通过从现有路径拼装获得
Input:         char *pszCurPath ,                       当前处理的路径
                   int iChangeLevel,                         改变的路径层数(0或负值，表示从路径右边去掉的节点数)
                   char *pszExtraPath                     附加 的路径，为NULL则表示不添加
Output:        无
Return:        TBS_SUCCESS ,      成功;
                    其它,     失败
Others:
*************************************************************************/
int COMM_AddObjectEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath)
{
    unsigned long ulCount = MAX_PATH_LEN;
    char szTempPath[MAX_PATH_LEN] = {'\0'};
    unsigned long ulIndex = 0;

    int iRet =TBS_SUCCESS;

    iRet = COMM_GetPathEx(pszCurPath,iChangeLevel,pszExtraPath,szTempPath,&ulCount);
    if (iRet != TBS_SUCCESS)
    {
        return  iRet;
    }

    iRet = CFG_AddObjInst(szTempPath,&ulIndex);
    return iRet;
}

/*************************************************************************
Function:    int COMM_DeleteObjectEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath)
Description:  删除object的封装接口，支持:对想设置值的路径可以通过从现有路径拼装获得
Input:         char *pszCurPath ,                       当前处理的路径
                   int iChangeLevel,                         改变的路径层数(0或负值，表示从路径右边去掉的节点数)
                   char *pszExtraPath                     附加 的路径，为NULL则表示不添加
Output:        无
Return:        TBS_SUCCESS ,      成功;
                    其它,     失败
Others:
*************************************************************************/
int COMM_DeleteObjectEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath)
{
    unsigned long ulCount = MAX_PATH_LEN;
    char szTempPath[MAX_PATH_LEN] = {'\0'};

    int iRet =TBS_SUCCESS;

    iRet = COMM_GetPathEx(pszCurPath,iChangeLevel,pszExtraPath,szTempPath,&ulCount);
    if (iRet != TBS_SUCCESS)
    {
        return  iRet;
    }

    iRet = CFG_DelNode(szTempPath);
    return iRet;
}


/*************************************************************************
Function:    char * COMM_GetAndSynValueEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath,ValuePostionType eGetMode)
Description:  获取值并设置同步标记，同步标记表示会从缓存中去掉，供commit函数等调用
Calls:         无
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       当前处理的路径
                   int iChangeLevel,                         改变的路径层数(0或负值，表示从路径右边去掉的节点数)
                   char *pszExtraPath                     附加 的路径，为NULL则表示不添加
                   ValuePostionType eGetMode        获取值的方式(    FROM_CACHE 从缓存, FROM_CFG 从配置树,  CACHE_CFG 先缓存后配置树)
Output:        无
Return:        NULL ,       失败;
                    其它,     成功，值的指针
Others:
*************************************************************************/
const char * COMM_GetAndSynValueEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath,ValuePostionType eGetMode)
{
    const char *pVal;
    char szTempPath[MAX_PATH_LEN] = {'\0'};
    unsigned long ulCount = MAX_PATH_LEN;

    int iRet =TBS_SUCCESS;

    iRet = COMM_GetPathEx(pszCurPath,iChangeLevel,pszExtraPath,szTempPath,&ulCount);
    if (iRet != TBS_SUCCESS)
    {
        pVal =  NULL;
    }

    pVal = COMM_GetNodeValue(szTempPath,eGetMode);
#if 0
    if (pVal)
    {

        iRet = CFG_SetNodeVal(szTempPath, pVal, NULL);
        if (iRet != TBS_SUCCESS)
        {
            pVal =  NULL;
        }

        SetCleanFlag(szTempPath);
    }
#endif
    return pVal;


}

/*************************************************************************
Function:    int CFG_CheckForEach(char *pszCurPath,char *pszCurVal,int iChangeLevel,CheckEachFunc pfnCheck)
Description:  对某层路径进行遍历
Calls:         无
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       当前处理的路径
                   char *pszCurVal,                          当前值
                   int iChangeLevel,                         遍历的路径的层次，表示从右边去掉的节点数
                   CheckEachFunc pfnCheck            检查函数
Output:        无
Return:       true ,      成功;
                   false,     失败
Others:
*************************************************************************/
int CFG_CheckForEach(char *pszCurPath, int iChangeLevel, CheckEachFunc pfnCheck, void *pData, int bRet)
{
    unsigned long ulRet = 0;
    unsigned long ulLen = 0;

    char szObjPath[MAX_PATH_LEN] = {0};
    ST_PARA_VAL *pstPathPara = NULL;
    ST_PARA_VAL *pstParaBuf= {NULL};

    ulRet = COMM_GetPathEx(pszCurPath,iChangeLevel,NULL,szObjPath,&ulLen);
    if (ulRet != TBS_SUCCESS)
    {
        return ulRet;
    }

    strcat(szObjPath,".");
    ulRet = CFG_GetNodeName(szObjPath, 1, &pstPathPara, NULL);
    if (CFG_RET_FAILED(ulRet))
    {
        return ulRet;
    }

    for (pstParaBuf = pstPathPara; pstParaBuf != NULL; pstParaBuf = pstParaBuf->pstNext)
    {
        ulRet = pfnCheck(pszCurPath,pData,pstParaBuf->pcName);
        if (bRet == CFG_FAILED_EXIT)
        {
            if (TBS_SUCCESS != ulRet)
                goto EXIT;
        }
        else if (bRet == CFG_SUCCESS_EXIT)
        {
            if (TBS_SUCCESS == ulRet)
                goto EXIT;
        }
    }

EXIT:
    safe_free_cfg(pstPathPara);

    return ulRet;
}


/*************************************************************************
Function:    int CFG_ForEachEx(char *pszCurPath, ForEachFunc pfnForEach, void *pData)
Description:  对某层路径进行遍历
Calls:         无
Data Accessed:
Data Updated:
Input:     char *pszCurPath ,                       当前处理的路径
           ForEachFunc pfnForEach            遍历使用的函数
           void *pData,                                传入的私有数据
           int bRet                                     循环退出条件，
                        0----循环遇到第一个非TBS_SUCCESS返回退出
                        1-----循环遇到第一个TBS_SUCCESS返回退出
                        2-----循环完毕才退出
Output:        无
Return:       true ,      成功;
                   false,     失败
Others:
*************************************************************************/
int CFG_ForEachEx(const char *pszCurPath, ForEachFunc pfnForEach, void *pData,int bRet)
{
    int i=0,j=0;

    unsigned long ulRet = TBS_SUCCESS;
    unsigned long ulCount=0;

    char szObjPath[MAX_PATH_LEN] = {0};
    char szTempPath[MAX_PATH_LEN] = {0};

    ST_PARA_VAL *pstPathPara = NULL;
    ST_PARA_VAL *pstParaBuf= {NULL};


    /*解析path到链表中*/
    char **papcAttrList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );

    safe_strncpy(szTempPath,pszCurPath,MAX_PATH_LEN);

    papcAttrList[ulCount] = strtok(szTempPath, ".");
    while (papcAttrList[ulCount])
    {
        ulCount++;
        papcAttrList[ulCount] = strtok(NULL, ".");
    }

    while (i<ulCount)
    {
        if ('%' != *(papcAttrList[i])) /*不是%,则表示这层不用循环*/
        {
            strcat(szObjPath,papcAttrList[i]);
            strcat(szObjPath,".");
            i++;
        }
        else  /*等于%, 需要进行循环*/
        {
            ulRet = CFG_GetNodeName(szObjPath, 1, &pstPathPara, NULL);
            if (CFG_RET_FAILED(ulRet) || pstPathPara == NULL)
            {
                if (bRet == CFG_FAILED_EXIT)
                    ulRet = TBS_SUCCESS;
                else if (bRet == CFG_SUCCESS_EXIT)
                    ulRet = TBS_FAILED;

                goto EXIT;
            }

            /*对本层进行循环处理*/
            for (pstParaBuf = pstPathPara; pstParaBuf != NULL; pstParaBuf = pstParaBuf->pstNext)
            {
                safe_strncpy(szObjPath,pstParaBuf->pcName,MAX_PATH_LEN);

                j=i+1;
                while (j<ulCount)
                {
                    strcat(szObjPath,papcAttrList[j++]);
                    strcat(szObjPath,".");
                }
                szObjPath[strlen(szObjPath)-1] = 0;

                /*递归调用，以解决多层的情况*/
                ulRet = CFG_ForEachEx(szObjPath,pfnForEach,pData,bRet);
                if (bRet == CFG_FAILED_EXIT)
                {
                    if (TBS_SUCCESS != ulRet)
                        goto EXIT;
                }
                else if (bRet == CFG_SUCCESS_EXIT)
                {
                    if (TBS_SUCCESS == ulRet)
                        goto EXIT;
                }
            }

            goto EXIT;
        }
    }

    /*路径中没有%*/
    if (i == ulCount)
    {
        szObjPath[strlen(szObjPath)-1] = 0;
        ulRet = pfnForEach(szObjPath, pData);
        if (bRet == CFG_FAILED_EXIT)
        {
            if (TBS_SUCCESS != ulRet)
                goto EXIT;
        }
        else if (bRet == CFG_SUCCESS_EXIT)
        {
            if (TBS_SUCCESS == ulRet)
                goto EXIT;
        }
    }

EXIT:
    safe_free_cfg(pstPathPara);
    free(papcAttrList);

    return ulRet;
}

/*************************************************************************
Function:    int COMM_SynNode2Cache(char *pszPath)
Description:  把值同步到缓存中
Calls:         无
Data Accessed:
Data Updated:
Input:         char *pszPath ,                       当前处理的路径
Output:        无
Return:       TBS_SUCCESS ,      成功;
                    其它,     失败
Others:
*************************************************************************/
int COMM_SynNode2Cache(unsigned short usMID,char *pszPath)
{
    char *pVal = (char *)COMM_GetNodeValue(pszPath,FROM_CFG);
    if (pVal)
    {
        int i = MID2INDEX(usMID);

        AddCacheNode(&(g_Module_Table[i].listCache),pszPath,pVal);
        return TBS_SUCCESS;
    }

    return TBS_NULL_PTR;
}

/*************************************************************************
Function:    int COMM_ApplyAllNodes(unsigned short usMID)
Description:  生效该模块所有的节点
Calls:         无
Data Accessed:
Data Updated:
Input:         unsigned short usMID ,                       当前的MID
Output:        无
Return:       TBS_SUCCESS ,      成功;
                    其它,     失败
Others:
*************************************************************************/
int COMM_ApplyAllNodes(unsigned short usMID)
{
    int iRet = TBS_SUCCESS;
//    MSG_PROCESS_FUNC pfnProcess = NULL;
//    ST_HookEntry *pstHookTable = NULL;
    ST_MSG_DATA *pMsgData = NULL;

    ST_PARA_VAL *pstListHead = NULL;
    ST_PARA_VAL *pstList = NULL;

    char *pszStat = NULL;
    char *pszWritable = NULL;
    ST_HookEntry * pHookEntry = NULL;
    PostApplyFunc pPostApply = NULL;

    CFG_RET ret = 0;

    CFG_MIDInstListBegin(usMID);

     /*循环取出每个实例中的值，放入到cache中*/
    while (ret = CFG_ListMIDNextInstNodes(&pstListHead),
            CFG_OK == ret && NULL != pstListHead)
    {
        for (pstList = pstListHead; NULL != pstList; pstList = pstList->pstNext)
        {
             /*为避免统计节点遍历生效降低启动速度，对于属性Stat=1的节点不放入cache*/
            ret = CFG_GetNodeAttrValPtr(pstList->pcName,ATTR_STAT,(const char**)&pszStat);
            if ((ret == CFG_OK && pszStat[0] != '1') || (ret != CFG_OK))
            {
                /* 对于不可写的节点不写入cache */
                ret = CFG_GetNodeAttrValPtr(pstList->pcName,ATTR_WRITABLE,(const char**)&pszWritable);
                if ((ret == CFG_OK && pszWritable[0] != '0') || (ret != CFG_OK))
                {
                    COMM_SynNode2Cache(usMID,pstList->pcName);
                }
            }
        }

        (void)CFG_MemFree(pstListHead);
        pstListHead = NULL;

    }

        /*对cache中节点进行生效*/
    if (MSG_MSG_GetProcessFunc(usMID,MSG_CMM_COMMIT,&pMsgData))
    {
        if (pMsgData)
        {
            struct list_head *pList = MSG_MSG_GetModuleCache(usMID);
            iRet = CacheList_ForEachOnce(usMID,pList,pMsgData,ApplyAccessor);

            /*POST APPLY 函数会全部被调用  */
            if (pMsgData->stTableType == ARRAY_TABLE)
            {
                pHookEntry = pMsgData->pstHookTable;
                while (pHookEntry->pHookFunc)
                {
                    if (APPLY_POST_FUNC == pHookEntry->eHookType)
                    {
                        pPostApply = (PostApplyFunc)pHookEntry->pHookFunc;

                        iRet = pPostApply();
                        if (RET_FAILED(iRet))
                        {
                            COMMON_MSG_TRACE("Error: Update failed - msg:0x%04x,func:0x%p,path:%s\n",
                                             pHookEntry->eHookType,pHookEntry->pHookFunc,pHookEntry->pNodePath);
                        }
                    }

                    pHookEntry++;
                }
            }
            else
            {
                if (pMsgData->pHashModuleFuncTable)
                    hash_for_each_do(pMsgData->pHashModuleFuncTable,CallPostApplyFuncInHash);
            }

            CleanCache(pList);
        }
    }
    else
    {
        return TBS_FAILED;
    }
    return TBS_SUCCESS;
}


/*************************************************************************
Function:    int COMM_ApplyAllNodesEx(unsigned short usMID,const char * pszPath)
Description:  生效该模块某路径下所有的节点，类似于COMM_ApplyAllNodes
Calls:         无
Data Accessed:
Data Updated:
Input:         unsigned short usMID ,                       当前的MID
                   const char * pszPath,                        当前的路径
Output:        无
Return:       TBS_SUCCESS ,      成功;
                    其它,     失败
Others:
*************************************************************************/
int COMM_ApplyAllNodesEx(unsigned short usMID,const char * pszPath)
{
    int iRet = TBS_SUCCESS;
//    MSG_PROCESS_FUNC pfnProcess = NULL;
//    ST_HookEntry *pstHookTable = NULL;
    ST_MSG_DATA *pMsgData = NULL;

    ST_PARA_VAL *pstListHead = NULL;
    ST_PARA_VAL *pstList = NULL;

    char *pszStat = NULL;
    char *pszWritable = NULL;

    CFG_RET ret = 0;

    CFG_MIDInstListBeginEx(usMID,pszPath);

    while (ret = CFG_ListMIDNextInstNodes(&pstListHead),
            CFG_OK == ret && NULL != pstListHead)
    {
        for (pstList = pstListHead; NULL != pstList; pstList = pstList->pstNext)
        {
            ret = CFG_GetNodeAttrValPtr(pstList->pcName,ATTR_STAT,(const char**)&pszStat);
            if ((ret == CFG_OK && pszStat[0] != '1') || (ret != CFG_OK))
            {
                /* 对于不可写的节点不放入cache */
                ret = CFG_GetNodeAttrValPtr(pstList->pcName,ATTR_WRITABLE,(const char**)&pszWritable);
                if ((ret == CFG_OK && pszWritable[0] != '0') || (ret != CFG_OK))
                {
                    COMM_SynNode2Cache(usMID,pstList->pcName);
                }

            }
        }

        (void)CFG_MemFree(pstListHead);
        pstListHead = NULL;
    }

    if (MSG_MSG_GetProcessFunc(usMID,MSG_CMM_COMMIT,&pMsgData))
    {
        if (pMsgData)
        {
            struct list_head *pList = MSG_MSG_GetModuleCache(usMID);
            iRet = CacheList_ForEachOnce(usMID,pList,pMsgData,ApplyAccessor);

            CleanCache(pList);
        }
    }
    else
    {
        return TBS_FAILED;
    }

    return TBS_SUCCESS;
}

/*************************************************************************
Function:    int COMM_ParseCustomMsg(ST_MSG *pstMsg,char *pszFormat, ...)
Description:  解析自定义消息体
Calls:         无
Data Accessed:
Data Updated:
Input:         ST_MSG *pstMsg       消息体
                  char *pszFormat       需要的解析name串，以空格隔开。如:"name3 name2"
Output:        变参                      会把解析的值传入到变参中
Return:       TBS_SUCCESS ,      成功;
                    其它,     失败
Others:
*************************************************************************/
int COMM_ParseCustomMsg(const ST_MSG *pstMsg,char *pszFormat, ...)
{
    int i,j;
    int iRet;

    unsigned int iBitFlag=0;

    va_list arg_ptr;

    char *ptr = NULL;
    const char *pcPos = NULL;
    char *pszMsgNode= NULL;

    char **papcAttrList = NULL;
    char **papcParaList = NULL;

    unsigned long ulCount = 0;
    unsigned long ulNodeCount  = 0;
    unsigned long ulMatchCount =  0;

    char szTempPath[MAX_PATH_LEN] = {0};
    char szTempFormat[MAX_PATH_LEN] = {0};
    char szTempValue[MAX_NODE_VALUE_LEN] = {0};


    /*参数检查*/
    if (NULL == pstMsg)
    {
        iRet = TBS_PARAM_ERR;
        return iRet;
    }

    /*解析format*/
    papcAttrList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );

    safe_strncpy(szTempFormat,pszFormat,MAX_PATH_LEN);

    papcAttrList[ulCount] = strtok(szTempFormat, " ");
    while (papcAttrList[ulCount])
    {
        ulCount++;
        papcAttrList[ulCount] = strtok(NULL, " ");
    }


    /*解析参数列表*/
    papcParaList = (char **)malloc(ulCount * sizeof(char *) );

    va_start(arg_ptr, pszFormat);
    for (i = 0; i < ulCount; i++)
    {
        ptr = va_arg(arg_ptr,char *);
        papcParaList[i] = ptr;
    }
    va_end(arg_ptr);


    /*解析出通知消息的各个字段*/
    pcPos = pstMsg->szMsgBody;
    GET_ULONG(pcPos, ulNodeCount);


    for (i = 0; i < ulNodeCount; i++)
    {
        /* 获取一条设置命令 */
        GET_STR(pcPos, pszMsgNode);

        /* 解析设置命令，获取节点名，节点值和Index序号 */
        iRet = ParseSetCmd(pszMsgNode, szTempPath, szTempValue, NULL, NULL);
        if (RET_FAILED(iRet))
        {
            COMMON_TRACE(MID_CCP,"Parse msg failed.");
            goto EXIT;
        }

        /* 取出节点路径 */
        for (j=0;j<ulCount;j++)
        {
            if (!strcasecmp(szTempPath, papcAttrList[j]))
            {

                if (!(iBitFlag&(1<<j)))
                {
                    strcpy(papcParaList[j] , szTempValue);
                    ulMatchCount++;
                    iBitFlag |= (1<<j);
                    break;
                }
                else
                {
                    COMMON_LOG_ERROR(MID_CCP,"have same name in msg:%s",szTempPath);
                    iRet = ERR_INNER_MSG_REPEATED_NAME;
                    goto EXIT;
                }
                COMMON_TRACE(MID_CCP, "parse Path = (%s) Value = (%s)\n", szTempPath,szTempValue);
            }
        }

    }

EXIT:
    free(papcAttrList);
    free(papcParaList);

    if (ulMatchCount != ulCount)
        return TBS_NO_INSTANCE;
    else
        return TBS_SUCCESS;
}

/*************************************************************************
Function:    int COMM_SendAndParseResponseMsg(int iNeedResponse, ST_MSG *pstMsg, char *pszFormat, ...)
Description: 发送消息并解析响应
Calls:       无
Data Accessed:
Data Updated:
Input:       int iNeedResponse      是否需要接收响应: 0 不接收, 1 接收并解析
             ST_MSG *pstMsg         消息体
             char *pszFormat        需要的解析name串，以空格隔开。如:"name3 name2"
Output:      变参                   会把解析的值传入到变参中
Return:      TBS_SUCCESS:成功       其它:失败
Others:      与COMM_MakeCustomMsg配合使用,主要用于进程间通信
*************************************************************************/
int COMM_SendAndParseResponseMsg(int iNeedResponse, ST_MSG *pstMsg, char *pszFormat, ...)
{
    int i,j;
    int iRet;

    unsigned int iBitFlag=0;

    va_list arg_ptr;

    char *ptr = NULL;
    char *pcPos = NULL;
    char *pszMsgNode= NULL;

    char **papcAttrList = NULL;
    char **papcParaList = NULL;

    unsigned long ulCount = 0;
    unsigned long ulNodeCount  = 0;
    unsigned long ulMatchCount =  0;

    char szTempPath[MAX_PATH_LEN] = {0};
    char szTempFormat[MAX_PATH_LEN] = {0};
    char szTempValue[MAX_NODE_VALUE_LEN] = {0};

    /*参数检查*/
    if (NULL == pstMsg)
    {
        iRet = TBS_PARAM_ERR;
        return iRet;
    }

    /*发送并接收消息*/
    iRet = MSG_SendMessage(pstMsg);
    MSG_ReleaseMessage(pstMsg);
    if (0 != iRet)
    {
        COMMON_TRACE(MID_CCP,"Send message failed.");
        return iRet;
    }

    if(iNeedResponse)
    {
        iRet = MSG_ReceiveMessage(&(pstMsg), 5);
        if (0 != iRet)
        {
            COMMON_TRACE(MID_CCP,"Receive message failed.");
            return iRet;
        }
    }
    else
    {
        return TBS_SUCCESS;
    }

    /*解析format*/
    papcAttrList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );

    safe_strncpy(szTempFormat,pszFormat,MAX_PATH_LEN);

    papcAttrList[ulCount] = strtok(szTempFormat, " ");
    while (papcAttrList[ulCount])
    {
        ulCount++;
        papcAttrList[ulCount] = strtok(NULL, " ");
    }


    /*解析参数列表*/
    papcParaList = (char **)malloc(ulCount * sizeof(char *) );

    va_start(arg_ptr, pszFormat);
    for (i = 0; i < ulCount; i++)
    {
        ptr = va_arg(arg_ptr,char *);
        papcParaList[i] = ptr;
    }
    va_end(arg_ptr);


    /*解析出通知消息的各个字段*/
    pcPos = pstMsg->szMsgBody;
    GET_ULONG(pcPos, ulNodeCount);


    for (i = 0; i < ulNodeCount; i++)
    {
        /* 获取一条设置命令 */
        GET_STR(pcPos, pszMsgNode);

        /* 解析设置命令，获取节点名，节点值和Index序号 */
        iRet = ParseSetCmd(pszMsgNode, szTempPath, szTempValue, NULL, NULL);
        if (RET_FAILED(iRet))
        {
            COMMON_TRACE(MID_CCP,"Parse msg failed.");
            goto EXIT;
        }

        /* 取出节点路径 */
        for (j=0;j<ulCount;j++)
        {
            if (!strcasecmp(szTempPath, papcAttrList[j]))
            {

                if (!(iBitFlag&(1<<j)))
                {
                    strcpy(papcParaList[j] , szTempValue);
                    ulMatchCount++;
                    iBitFlag |= (1<<j);
                    break;
                }
                else
                {
                    COMMON_LOG_ERROR(MID_CCP,"have same name in msg:%s",szTempPath);
                    iRet = ERR_INNER_MSG_REPEATED_NAME;
                    goto EXIT;
                }
                COMMON_TRACE(MID_CCP, "parse Path = (%s) Value = (%s)\n", szTempPath,szTempValue);
            }
        }

    }

EXIT:
    free(papcAttrList);
    free(papcParaList);
    MSG_ReleaseMessage(pstMsg);

    if (ulMatchCount != ulCount)
        return TBS_NO_INSTANCE;
    else
        return TBS_SUCCESS;
}

static unsigned long s_ulMsgID = 0;
unsigned long GenMsgID()
{
    return s_ulMsgID++;
}

unsigned long GetLastMsgID()
{
    return s_ulMsgID;
}


/*************************************************************************
Function:       COMM_MakeCustomMsg
Description:    解析自定义消息体
Calls:          无
Data Accessed:
Data Updated:
Input:        ST_MSG *pstMsg              消息体
              unsigned short usMsgType    消息类型
              unsigned short usSrcMID     源MID
              unsigned short usDstMID     目的MID
              unsigned short usNodeNum    发送消息的节点数
Output:       变参，会把解析的值传入到变参中
Return:       TBS_SUCCESS ,      成功;
			  其它,              失败
Others:       构造的消息必须是 name1=value1...类型的
*************************************************************************/
int COMM_MakeCustomMsg(ST_MSG **ppstMsg,unsigned short usMsgType,unsigned short usSrcMID,unsigned short usDstMID,unsigned short usNodeNum, ...)
{
    int i=0;
    int iRet = 0;

    unsigned short usNameIdx = 0;
    unsigned short usValueIdx = 0;
    unsigned short usMsgLen = 0;

    char **papcNameList = NULL;
    char **papcAttrList = NULL;

    char *ptr = NULL;
    char *pcPos = NULL;

    char szTempBuf[2*MAX_PATH_LEN] = {'\0'};

    va_list arg_ptr;

    ST_MSG *pstMsg = NULL;

    papcNameList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );
    papcAttrList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );

    usMsgLen = sizeof(unsigned long);

    va_start(arg_ptr, usNodeNum);
    for (i = 0; i < usNodeNum; i++)
    {
        ptr = va_arg(arg_ptr,char *);
        papcNameList[usNameIdx++] = ptr;
        usMsgLen += strlen(ptr)+1;

        ptr = va_arg(arg_ptr,char *);
        papcAttrList[usValueIdx++] = ptr;
        usMsgLen += strlen(ptr)+1;
    }
    va_end(arg_ptr);

    *ppstMsg = MSG_CreateMessage(usMsgLen);
    pstMsg = *ppstMsg;


    if (NULL == pstMsg)
    {
        iRet = TBS_OUT_OF_MEM;
        COMMON_LOG_ERROR(MID_CCP,"Create Notify Message Failed.\n");
        goto EXIT;
    }

    /* 负载信息项数*/
    pcPos = pstMsg->szMsgBody;
    SET_ULONG(pcPos, usNodeNum);

    for (i=0;i<usNodeNum;i++)
    {
        memset(szTempBuf, 0, sizeof(szTempBuf));
        sprintf(szTempBuf, "%s=%s", papcNameList[i],papcAttrList[i]);
        SET_STR(pcPos, szTempBuf);
    }

    /* 设置消息头*/
    pstMsg->stMsgHead.ulMsgID = GenMsgID();
    pstMsg->stMsgHead.usSrcMID = usSrcMID;
    pstMsg->stMsgHead.usDstMID = usDstMID;
    pstMsg->stMsgHead.usMsgType = usMsgType;
    pstMsg->stMsgHead.ulBodyLength = pcPos-pstMsg->szMsgBody;

    iRet = TBS_SUCCESS;

EXIT:
    free(papcNameList);
    free(papcAttrList);

    return iRet;
}


/*************************************************************************
Function:       COMM_MakeAndSendCustomMsg
Description:    构造并发送自定义消息体
Calls:          无
Data Accessed:
Data Updated:
Input:          unsigned short usMsgType    消息类型
                unsigned short usSrcMID     源MID
                unsigned short usDstMID     目的MID
                unsigned short usNodeNum    发送消息的节点数
                变参                        构造用的字符串
Output:
Return:         TBS_SUCCESS ,               成功
                其它,                       失败
Others:         构造的消息必须是 name1=value1...类型的
*************************************************************************/
int COMM_MakeAndSendCustomMsg(unsigned short usMsgType,unsigned short usSrcMID,
    unsigned short usDstMID,unsigned short usNodeNum, ...)
{
    int i=0;
    int iRet = 0;

    unsigned short usNameIdx = 0;
    unsigned short usValueIdx = 0;
    unsigned short usMsgLen = 0;

    char **papcNameList = NULL;
    char **papcAttrList = NULL;

    char *ptr = NULL;
    char *pcPos = NULL;

    char szTempBuf[2*MAX_PATH_LEN] = {'\0'};

    va_list arg_ptr;

    ST_MSG *pstMsg = NULL;

    papcNameList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );
    papcAttrList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );

    usMsgLen = sizeof(unsigned long);

    /*解析所有变参*/
    va_start(arg_ptr, usNodeNum);
    for (i = 0; i < usNodeNum; i++)
    {
        ptr = va_arg(arg_ptr,char *);
        papcNameList[usNameIdx++] = ptr;
        usMsgLen += strlen(ptr)+1;


        ptr = va_arg(arg_ptr,char *);
        papcAttrList[usValueIdx++] = ptr;
        usMsgLen += strlen(ptr)+1;

    }
    va_end(arg_ptr);

    pstMsg = MSG_CreateMessage(usMsgLen);

    if (NULL == pstMsg)
    {
        iRet = TBS_OUT_OF_MEM;
        COMMON_LOG_ERROR(MID_CCP,"Create Notify Message Failed.\n");
        goto EXIT;
    }

    /* 负载信息项数*/
    pcPos = pstMsg->szMsgBody;
    SET_ULONG(pcPos, usNodeNum);

    for (i=0;i<usNodeNum;i++)
    {
        memset(szTempBuf, 0, sizeof(szTempBuf));
        sprintf(szTempBuf, "%s=%s", papcNameList[i],papcAttrList[i]);

        SET_STR(pcPos, szTempBuf);
    }

    /* 设置消息头*/
    pstMsg->stMsgHead.ulMsgID = GenMsgID();
    pstMsg->stMsgHead.usSrcMID = usSrcMID;
    pstMsg->stMsgHead.usDstMID = usDstMID;
    pstMsg->stMsgHead.usMsgType = usMsgType;

    pstMsg->stMsgHead.ulBodyLength = pcPos-pstMsg->szMsgBody;

    iRet = MSG_SendMessage(pstMsg);

    /* 发送失败*/
    if (RET_FAILED(iRet))
    {
        COMMON_LOG_ERROR(MID_CCP,"Send message failed\n");
    }

    MSG_ReleaseMessage(pstMsg);

    iRet = TBS_SUCCESS;

EXIT:
    free(papcNameList);
    free(papcAttrList);

    return iRet;
}


/*************************************************************************
Function:       COMM_ResponseCustomMsg
Description:    构造并发送自定义消息体的回复消息
Calls:          无
Data Accessed:
Data Updated:
Input:          ST_MSG *pstMsg              接收到的消息体
                unsigned short usMsgType    回复的消息类型
                unsigned short usNodeNum    发送消息的节点数
                变参                        构造用的字符串
Output:
Return:         TBS_SUCCESS ,               成功
                其它,                       失败
Others:         构造的消息必须是 name1=value1...类型的
*************************************************************************/
int COMM_ResponseCustomMsg(ST_MSG *pstMsg,unsigned short usMsgType,
    unsigned short usNodeNum, ...)
{
    int i=0;
    int iRet = 0;

    unsigned short usNameIdx = 0;
    unsigned short usValueIdx = 0;
    unsigned short usMsgLen = 0;

    char **papcNameList = NULL;
    char **papcAttrList = NULL;

    char *ptr = NULL;
    char *pcPos = NULL;

    char szTempBuf[2*MAX_PATH_LEN] = {'\0'};

    va_list arg_ptr;

    ST_MSG *pstResponseMsg = NULL;

    papcNameList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );
    papcAttrList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );

    usMsgLen = sizeof(unsigned long);

    /*解析所有变参*/
    va_start(arg_ptr, usNodeNum);
    for (i = 0; i < usNodeNum; i++)
    {
        ptr = va_arg(arg_ptr,char *);
        papcNameList[usNameIdx++] = ptr;
        usMsgLen += strlen(ptr)+1;


        ptr = va_arg(arg_ptr,char *);
        papcAttrList[usValueIdx++] = ptr;
        usMsgLen += strlen(ptr)+1;

    }
    va_end(arg_ptr);

    pstResponseMsg = MSG_CreateMessage(usMsgLen);

    if (NULL == pstResponseMsg)
    {
        iRet = TBS_OUT_OF_MEM;
        COMMON_LOG_ERROR(MID_CCP,"Create Notify Message Failed.\n");
        goto EXIT;
    }

    /* 负载信息项数*/
    pcPos = pstResponseMsg->szMsgBody;
    SET_ULONG(pcPos, usNodeNum);

    for (i=0;i<usNodeNum;i++)
    {
        memset(szTempBuf, 0, sizeof(szTempBuf));
        sprintf(szTempBuf, "%s=%s", papcNameList[i],papcAttrList[i]);

        SET_STR(pcPos, szTempBuf);
    }

    /* 设置消息头*/
    pstResponseMsg->stMsgHead.ulMsgID = pstMsg->stMsgHead.ulMsgID;
    pstResponseMsg->stMsgHead.usSrcMID = pstMsg->stMsgHead.usDstMID;
    pstResponseMsg->stMsgHead.usDstMID = pstMsg->stMsgHead.usSrcMID;
    pstResponseMsg->stMsgHead.usMsgType = usMsgType;

    pstResponseMsg->stMsgHead.ulBodyLength = pcPos-pstResponseMsg->szMsgBody;

    iRet = MSG_SendMessage(pstResponseMsg);

    /* 发送失败*/
    if (RET_FAILED(iRet))
    {
        COMMON_LOG_ERROR(MID_CCP,"Send message failed\n");
    }

    MSG_ReleaseMessage(pstResponseMsg);

    iRet = TBS_SUCCESS;

EXIT:
    free(papcNameList);
    free(papcAttrList);

    return iRet;
}

/*************************************************************************
Function:       COMM_MakeBinMsg
Description:    构造自定义消息体,二进制格式
Calls:          无
Data Accessed:
Data Updated:
Input:          unsigned short usMsgType    消息类型
                unsigned short usSrcMID     源MID
                unsigned short usDstMID     目的MID
                unsigned short usMsgLen     消息体长度
                void *pcMsgBody             消息体数据
Output:
Return:         ST_MSG* ,                   成功
                NULL,                       失败
Others:         构造的消息可以是任意类型的二进制格式
*************************************************************************/
ST_MSG *COMM_MakeBinMsg(unsigned short usMsgType,unsigned short usSrcMID,
    unsigned short usDstMID,unsigned short usMsgLen, void *pcMsgBody)
{
    char *pcPos = NULL;
    ST_MSG *pstMsg = NULL;

    /* 创建消息 */
    pstMsg = MSG_CreateMessage(usMsgLen);
    if (NULL == pstMsg)
    {
        COMMON_LOG_ERROR(MID_CCP, "Create Notify Message Failed.\n");
        goto EXIT;
    }

    /* 负载信息项数*/
    pcPos = pstMsg->szMsgBody;
    if (usMsgLen > 0)
    {
        memcpy(pcPos, pcMsgBody, usMsgLen);
        pcPos += usMsgLen;
    }

    /* 设置消息头*/
    pstMsg->stMsgHead.ulMsgID = GenMsgID();
    pstMsg->stMsgHead.usSrcMID = usSrcMID;
    pstMsg->stMsgHead.usDstMID = usDstMID;
    pstMsg->stMsgHead.usMsgType = usMsgType;
    pstMsg->stMsgHead.ulBodyLength = pcPos-pstMsg->szMsgBody;

    return pstMsg;

EXIT:
    return NULL;
}


/*************************************************************************
Function:       COMM_MakeAndSendBinMsg
Description:    构造并发送自定义消息体,二进制格式
Calls:          无
Data Accessed:
Data Updated:
Input:          unsigned short usMsgType    消息类型
                unsigned short usSrcMID     源MID
                unsigned short usDstMID     目的MID
                unsigned short usMsgLen     消息体长度
                char *pcMsgBody             消息体数据
Output:
Return:         TBS_SUCCESS ,               成功
                其它,                       失败
Others:         构造的消息可以是任意类型的二进制格式
*************************************************************************/
int COMM_MakeAndSendBinMsg(unsigned short usMsgType,unsigned short usSrcMID,
    unsigned short usDstMID,unsigned short usMsgLen, void *pcMsgBody)
{
    int iRet = 0;

    char *pcPos = NULL;
    ST_MSG *pstMsg = NULL;

    /* 创建消息 */
    pstMsg = MSG_CreateMessage(usMsgLen);
    if (NULL == pstMsg)
    {
        iRet = TBS_OUT_OF_MEM;
        COMMON_LOG_ERROR(MID_CCP,"Create Notify Message Failed.\n");
        goto EXIT;
    }

    /* 负载信息项数*/
    pcPos = pstMsg->szMsgBody;
    if (usMsgLen > 0)
    {
        memcpy(pcPos, pcMsgBody, usMsgLen);
        pcPos += usMsgLen;
    }

    /* 设置消息头*/
    pstMsg->stMsgHead.ulMsgID = GenMsgID();
    pstMsg->stMsgHead.usSrcMID = usSrcMID;
    pstMsg->stMsgHead.usDstMID = usDstMID;
    pstMsg->stMsgHead.usMsgType = usMsgType;

    pstMsg->stMsgHead.ulBodyLength = pcPos-pstMsg->szMsgBody;

    iRet = MSG_SendMessage(pstMsg);

    /* 发送失败*/
    if (RET_FAILED(iRet))
    {
        COMMON_LOG_ERROR(MID_CCP,"Send message failed\n");
    }

    MSG_ReleaseMessage(pstMsg);

    iRet = TBS_SUCCESS;

EXIT:

    return iRet;
}

/*************************************************************************
Function:    int COMM_SetCacheValue(char *pszPath,char *pszValue)
Description:  设置cache值
Calls:         无
Data Accessed:
Data Updated:
Input:        unsigned short usMID      模块的MID
                  char *pszPath                  设置的路径
                  char *pszValue                设置的值
Output:
Return:       TBS_SUCCESS ,      成功;
                    其它,     失败
Others:       构造的消息必须是 name1=value1...类型的
*************************************************************************/
int COMM_SetCacheValue(unsigned short usMID,char *pszPath,char *pszValue)
{
    const char *pszCfgVal = NULL, *pszCacheVal = NULL;

    if ((pszPath==NULL)||(pszValue == NULL))
    {
        return TBS_FAILED;
    }

    int i = MID2INDEX(usMID);

    /*首先读取缓存值,如果节点值已经在缓存中了则重新设置一次*/
    pszCacheVal = GetCacheNode(&(g_Module_Table[i].listCache), pszPath);
    if (pszCacheVal)
    {
        SetCacheNode(&(g_Module_Table[i].listCache),pszPath,pszValue);
    }
    else
    {
        /*从配置树中读取节点值,如果节点值与参数传入的值不同,才将参数
        传入的节点值设置到缓存中*/
        pszCfgVal = COMM_GetNodeValue(pszPath, FROM_CFG);
        if(!pszCfgVal) return TBS_FAILED;
		if (strcmp(pszCfgVal, pszValue))
        {
            SetCacheNode(&(g_Module_Table[i].listCache),pszPath,pszValue);
        }
    }

    return TBS_SUCCESS;
}

/*************************************************************************
Function:    const char *COMM_GetCacheValue(unsigned short usMID, char *pszPath)
Description:  获取节点cache值

Input:      usMID      模块的MID
            pszPath    节点路径

Return:       节点缓存值指针 ,      成功;
              NULL,                 失败
Others:
*************************************************************************/
const char *COMM_GetCacheValue(unsigned short usMID, char *pszPath)
{
    if (pszPath == NULL)
        return NULL;

    int i = MID2INDEX(usMID);

    return GetCacheNode(&g_Module_Table[i].listCache, pszPath);
}

/*************************************************************************
Function:    int COMM_DelCacheNode(unsigned short usMID, char *pszPath)
Description: 删除参数pszPath指向的路径节点的缓存

Input:      usMID      模块的MID
            pszPath    节点路径

Return:       TBS_SUCCESS,      成功;
              其它,             失败
Others:
*************************************************************************/
int COMM_DelCacheNode(unsigned short usMID, char *pszPath)
{
    if (pszPath == NULL)
        return TBS_FAILED;

    int i = MID2INDEX(usMID);

    DeleteCacheNode(&g_Module_Table[i].listCache, pszPath);
    return TBS_SUCCESS;
}

/*************************************************************************
Function:    int COMM_CleanCache(unsigned short usMID)
Description:  清除cache
Calls:         无
Data Accessed:
Data Updated:
Input:        unsigned short usMID      模块的MID
Output:
Return:       TBS_SUCCESS ,      成功;
                    其它,     失败
Others:
*************************************************************************/
int COMM_CleanCache(unsigned short usMID)
{
    int i = MID2INDEX(usMID);

    CleanCache(&(g_Module_Table[i].listCache));
    return TBS_SUCCESS;
}

/*************************************************************************
Function:   unsigned short COMM_GetCacheBuildMID(unsigned short usCacheMID)
Description:  获取模块ID,该模块设置了由参数传入的模块的缓存,
              一般情况下模块的缓存都是由公共模块来创建的

Input:        usCacheMID,     缓存所属的模块的MID
Output:
Return:       设置了usCacheMID缓存的模块MID ,      成功;
              其它,                                失败
Others:
*************************************************************************/
unsigned short COMM_GetCacheBuildMID(unsigned short usCacheMID)
{
    int i = MID2INDEX(usCacheMID);

    return g_Module_Table[i].usCacheBuildMID;
}

/*************************************************************************
Function:
    int COMM_SetCacheBuildMID(unsigned short usCacheMID, unsigned short usCacheBuildMID)

Description:  设置创建缓存的模块ID, usCacheMID表示缓存所属的模块ID,
              usCacheBuildMID表示创建缓存的模块ID

Input:        usCacheMID,       缓存所属模块的MID
              usCacheBuildMID,  创建了模块usCacheMID缓存的MID
Output:
Return:       TBS_SUCCESS,      成功;
              其它,             失败
Others:
*************************************************************************/
int COMM_SetCacheBuildMID(unsigned short usCacheMID, unsigned short usCacheBuildMID)
{
    int i = MID2INDEX(usCacheMID);

    g_Module_Table[i].usCacheBuildMID = usCacheBuildMID;
    return TBS_SUCCESS;
}

/*************************************************************************
Function:
              void COMM_SetLastErrPath(char *pszPath)
Description:  部分模块需要记录当前处理的实例值之间冲突时，具体是哪个路径错误
Calls:        无
Data Accessed:
Data Updated:
Input:        char *pszPath      当前出错的路径
Output:
Return:       无
Others:
*************************************************************************/
extern char g_szErrPath[];
BOOL   g_bIsModuleDefined = FALSE;
void COMM_SetLastErrPath(char *pszPath)
{
    if (!pszPath)
        return;

    safe_strncpy(g_szErrPath, pszPath, MAX_PATH_LEN);
    g_bIsModuleDefined = TRUE;
    return;
}

/************************************************************
Function:      int COMM_StrtokEx(char *szBuf,int iMinLine,int iMaxLine,int iParaNum,...)
Description:   获取shell输出的表信息中某些行的某些字段，用法类似于strtok
Input:         char *szBuf,                             输出的字符buf
                   char *szDelim,                         分割符字符串，如果分割每列用的字符集合
                   int iMinLine, int iMaxLine,        需要获取信息的行数，>=iMinLine,<=iMaxLine,从0开始编号，
                                                                    当iMinLine为0时表示从最上面一行开始，iMaxLine为-1表示上限没有限制
                   int iParaNum,                           获取的列数，即变参有几对
                   ...                                              变参，需要获取的列和对应值的指针，如:2,pCol2,4,pCol4 (注意col数必须以升序排列)
Output:        ...                                              第偶数个变参会把前一个列值的指针输出
Return:        false,       失败;
                    true,     成功
Others:       用法类似于strtok，第一个参数为NULL,表示继续循环，非空表示开始循环遍历
************************************************************/
#define MAX_LINE_NUM 100
//#define MAX_CHAR_NUM 1024
int COMM_StrtokEx(char *szBuf,char *szDelim,int iMinLine,int iMaxLine,int iParaNum,...)
{
    int j=0;
    char *pszWord;

    int iMatched = 1;
    int iCurPara = 0;

    int iColIndex = 0;
    char ** pColStr = NULL;

    static char *pszLine[MAX_LINE_NUM];
    //static char pszBuf[MAX_CHAR_NUM];
    static char *pszBuf = NULL;
    static int iCurLine=0;
    static int iCount=0;

    va_list arg_ptr;

    /*解析各行，放入队列*/
    if (szBuf)
    {
        if (!strlen(szBuf))
            return 0;

        //strncpy(pszBuf,szBuf,MAX_CHAR_NUM);
        pszBuf = szBuf;
        iCount = 0;
        iCurLine = (0>iMinLine?0:iMinLine);
        memset(pszLine,0,MAX_LINE_NUM * sizeof(pszLine[0]));
        pszLine[iCount] = strtok(pszBuf, "\n");
        while (pszLine[iCount] && iCount < MAX_LINE_NUM - 1)
        {
            pszLine[++iCount] = strtok(NULL,"\n");
        }
    }
    else
    {
        iCurLine++;
        if (iCurLine >= iCount || ((iCurLine > iMaxLine)&&(iMaxLine > 0)))
            return 0;
    }

    va_start(arg_ptr, iParaNum);

    /*针对各行进行分拆*/
    pszWord = strtok(pszLine[iCurLine], szDelim);

    while (pszWord)
    {
        if (iMatched)
        {
            iColIndex = va_arg(arg_ptr,int);
            pColStr = va_arg(arg_ptr,char **);
            iCurPara++;
        }


        if (j == iColIndex)
        {
            iMatched = 1;
            *pColStr = pszWord;
        }
        else
        {
            iMatched = 0;
        }

        if ((iCurPara == iParaNum) && iMatched)
        {
            return 1;
        }

        pszWord = strtok(NULL,szDelim);
        j++;
    }

    if ((iCurPara == iParaNum) && iMatched)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}


/************************************************************
Function:      int COMM_SystemEx(const char *pszCmd,char * pszResult,int iCount)
Description:   获取shell命令输出信息
Input:         const char *pszCmd,              要执行的命令
                   char * pszResult,                    执行的输出结果
                   int iCount                                当前buffer的长度
Output:
Return:        false ,       失败;
                    true,     成功;
Others:
************************************************************/
int COMM_SystemEx(const char *pszCmd,char * pszResult,int iCount)
{
    FILE   *stream;

    //memset( pszResult, '\0', sizeof(pszResult) );
    memset( pszResult, '\0', iCount );
    stream = popen( pszCmd, "r" );
    if (!stream)
    {
        pclose( stream );
        return 0;
    }

    iCount = fread( pszResult, sizeof(char), iCount, stream);
    if (!(iCount))
    {
        pclose( stream );
        return 0;
    }

    pclose( stream );
    return 1;
}

/************************************************************
Function:      int COMM_ParserPairValue(char *szBuf,char *szDelim,char *szName,char **pszValue)
Description:   从输出的串中解析需要的信息，适合解析类似于ifconfig输出信息型的数据
Input:         char *szBuf,                            传入的buf
                   char *szDelim,                       分割的字符集
                   char *szName                         需要查找的名字
Output:       char **pszValue                      解析出的结果
Return:        false ,       失败;
                    true,     成功;
Others:
************************************************************/
int COMM_ParserPairValue(char *szBuf,char *szDelim,char *szName,char **pszValue)
{
    char *pszWord = NULL;

    static char *pszLine[200];
    static int iCurLine=0;
    static int iCount=0;

    if (szBuf)
    {
        if (!strlen(szBuf))
            return 0;

        iCount = 0;
        memset(pszLine,0,200);
        pszLine[iCount] = strtok(szBuf, "\n");
        while (pszLine[iCount])
        {
            pszLine[++iCount] = strtok(NULL,"\n");
        }

    }


    while (iCurLine<iCount)
    {
        pszWord = strtok(pszLine[iCurLine], szDelim);
        while (pszWord)
        {
            if (strcmp(pszWord,szName) == 0)
            {
                *pszValue = strtok(NULL,szDelim);
                return 1;
            }
            pszWord = strtok(NULL,szDelim);
        }
        iCurLine++;
    }

    return 0;
}


/* 0:TBS_SUCCESS, 1:TBS_FAILED */
int COMM_ParserPairValueEx(char *szBuf,char *szFirst,char *szEnd,char **pszValue)
{
    char *pFirst = NULL, *pEnd = NULL;

    char *pszLine[200];
    int iCurLine=0;
    int iCount=0;

    if (szBuf)
    {
        if (!strlen(szBuf))
            return 1;

        iCount = 0;
        memset(pszLine,0,200);
        pszLine[iCount] = strtok(szBuf, "\n");
        while (pszLine[iCount])
        {
            pszLine[++iCount] = strtok(NULL,"\n");
        }

    }
	else
	{
		return 1;
	}

    while (iCurLine<iCount)
    {
        pFirst = strstr(pszLine[iCurLine], szFirst);
		if(pFirst)
		{
			if(szEnd)
			{
				pEnd = strstr(pszLine[iCurLine], szEnd);
				if(pEnd)
				{
					while(*(--pEnd) == ' ');
					*pszValue = pFirst + strlen(szFirst);
					*(pEnd + 1) = 0;
					return 0;
				}
				else
				{
					printf("error ,usage not true\n\r");
					return 1;
				}
			}
			else
			{
				*pszValue = pFirst + strlen(szFirst);
				pEnd = *pszValue + strlen(*pszValue);
				while(*(--pEnd) == ' ');
				*(pEnd + 1) = 0;
				return 0;
			}
		}
		iCurLine++;
    }

    return 1;
}

/************************************************************
Function:      void COMM_PrintCurTime(char *pszInfo)
Description:   调试时打印时间戳使用
Input:         char *pszInfo,                         需要打印的描述信息
Return:        无
Others:
************************************************************/
void COMM_PrintCurTime(char *pszInfo)
{
    struct timeval tv;
    struct timezone tz;

    gettimeofday (&tv , &tz);
    printf("%s,Current time is %lu%lu\r\n", pszInfo,(unsigned long)tv.tv_sec,(unsigned long)tv.tv_usec);

}

void COMM_SetModuleToApply(unsigned short usMID)
{
    int i = MID2INDEX(usMID);

    SET_BIT(g_Module_Table[i].ucModuleFLag, MFLG_BIT_TO_APPLY);
    return;
}

int COMM_IsModuleToApply(unsigned short usMID)
{
    int i = MID2INDEX(usMID);

    return TEST_BIT(g_Module_Table[i].ucModuleFLag, MFLG_BIT_TO_APPLY);
}

/*************************************************************************
功能: 返回当前factory mode
参数: ValuePostionType eGetMode  获取值得方式      
返回: 
		成功 -- 返回X_TWSZ-COM_FactoryMode的值
    失败 -- NULL
备注:
**************************************************************************/
const char * COMM_GetConfigSetting(ValuePostionType eGetMode)
{
#define FACTORY_MODE_PATH   "InternetGatewayDevice.DeviceInfo.X_TWSZ-COM_FactoryMode"
    const char * pszConfigSetting = NULL;

    pszConfigSetting = COMM_GetNodeValue(FACTORY_MODE_PATH,eGetMode);
    if(!pszConfigSetting)
    {
    	COMMON_TRACE("Get %s Return Null!\n",FACTORY_MODE_PATH);
    }
    return pszConfigSetting;
}

#ifdef CONFIG_LAST_SAVE_CFG

#define LAST_CONFIG_PATH                                                     "/usr/local/ct/last_config"
#define LAST_TMP_CONFIG_PATH                                          "/usr/local/ct/.last_config"
#define LAST_CONFIG_INFO                                                      "/usr/local/ct/config_info"

int COMM_SaveLastConfig()
{
#define PATH_NOTE_LASTSAVETIME                          "InternetGatewayDevice.DeviceConfig.X_TWSZ-COM_LastSaveTime"
#define PATH_NOTE_LASTSAVEVERSION                 "InternetGatewayDevice.DeviceConfig.X_TWSZ-COM_LastSaveVersion"
#define PATH_NOTE_SOFTWAREVERSION               "InternetGatewayDevice.DeviceInfo.SoftwareVersion"
    int iRet=TBS_SUCCESS;
    char szTime[32]={0};
    char szVersion[128]={0};
    char szBuf[256]={0};
    unsigned long  pulLen;
    unsigned long ulCurTime = 0;
    unsigned long ulOldTime = 0;
    const char * pszOldTime = NULL;
    const char * pszCurVersion = NULL;
    const char * pszOldVersion = NULL;
    
/***************现场保护**************/
    pszOldTime = COMM_GetNodeValue(PATH_NOTE_LASTSAVETIME , CACHE_CFG);
    if(!pszOldTime)
    {
        COMMON_TRACE(MID_CCP, "Get %s faild!\n",  PATH_NOTE_LASTSAVETIME);
        return ERR_GET_NODE_VALUE_FAIL;
    }
    ulOldTime = atoi(pszOldTime);
    ulCurTime = time(NULL);
    sprintf(szTime, "%ld", ulCurTime);

    pszOldVersion = COMM_GetNodeValue(PATH_NOTE_LASTSAVEVERSION , CACHE_CFG);
    if(!pszOldVersion)
    {
        COMMON_TRACE(MID_CCP, "Get %s faild!\n",  PATH_NOTE_LASTSAVEVERSION);
        return ERR_GET_NODE_VALUE_FAIL;
    }
    pszCurVersion = COMM_GetNodeValue(PATH_NOTE_SOFTWAREVERSION, CACHE_CFG);
    if(!pszCurVersion)
    {
        COMMON_TRACE(MID_CCP, "Get %s faild!\n",  PATH_NOTE_SOFTWAREVERSION);
        return ERR_GET_NODE_VALUE_FAIL;
    }
    strcpy(szVersion, pszOldVersion);
/***************保护完毕**************/

    if(CFG_SetNodeVal( PATH_NOTE_LASTSAVETIME, szTime, NULL) != CFG_OK)
    {
        COMMON_TRACE(MID_CCP, "Set %s faild!\n",  PATH_NOTE_LASTSAVETIME);
        return ERR_SET_NODE_VALUE_FAIL;
    }

    if(safe_strcmp(pszCurVersion, pszOldVersion))
    {
        if(CFG_SetNodeVal( PATH_NOTE_LASTSAVEVERSION, pszCurVersion, NULL) != CFG_OK)
        {
            COMMON_TRACE(MID_CCP, "Set %s faild!\n",  PATH_NOTE_LASTSAVEVERSION);
            return ERR_SET_NODE_VALUE_FAIL;
        }
    }

    iRet = CFG_GetCfgFile(LAST_TMP_CONFIG_PATH, &pulLen, 1, 1, NULL);
    if(iRet!=CFG_OK)
    {
        sprintf(szTime, "%ld", ulOldTime);
        CFG_SetNodeVal( PATH_NOTE_LASTSAVETIME, szTime, NULL);

        if(safe_strcmp(pszCurVersion, szVersion))
        {
            CFG_SetNodeVal( PATH_NOTE_LASTSAVEVERSION, szVersion, NULL);
        }
        if(iRet == ERR_FILE_NOT_ALLOWED||iRet == ERR_FILE_WTITE_NOSPACE || iRet == ERR_FILE_OPEN_UNKOWN)
        {
            return ERR_JFS_NOT_OPEN;
        }
        return ERR_SAVE_CFG; 
    }

    COMM_SaveConfigInfo(ulCurTime, pszCurVersion);
    
    sprintf(szBuf,"mv %s %s", LAST_TMP_CONFIG_PATH, LAST_CONFIG_PATH);
    tbsSystem(szBuf,1);
    return TBS_SUCCESS;
}
int COMM_RecoverLastConfig()
{
    int iRet=TBS_SUCCESS;
    iRet = CFG_SetCompressCfgFile(LAST_CONFIG_PATH, 0);

    if(iRet!=CFG_OK)
    {
         if(iRet == ERR_CFG_FILE_OPEN)
         {
            return ERR_JFS_NOT_OPEN;
         }
         return ERR_RECOVER_CFG; 
    }

    return iRet;
}

int COMM_SaveConfigInfo(unsigned long ulTime, const char * pszVersion)
{
    int fd = 0;
    int iRet = 0;
    char str[256] = {0};

    COMMON_TRACE(MID_CCP, "Enter COMM_SaveConfigInfo!\n");
    if(!pszVersion) return TBS_FAILED;

    fd = open(LAST_CONFIG_INFO, O_WRONLY|O_CREAT);
    if(fd<0)
    {
        COMMON_TRACE(MID_CCP, "Open %s fail,error is:%s!\n",  LAST_CONFIG_INFO, strerror(errno));
        return TBS_FAILED;
    }

    sprintf(str, "Time\t:%ld\nVersion\t:%.128s", ulTime, pszVersion);

    iRet = write(fd, str, strlen(str));
    if(iRet<strlen(str))
    {
        COMMON_TRACE(MID_CCP, "Write %s to %s fail,error is:%s!\n", str, LAST_CONFIG_INFO, strerror(errno));
        iRet  = TBS_FAILED;
    }
    else
        iRet = TBS_SUCCESS;

    close(fd);
   return iRet;
}
int COMM_GetConfigInfo(unsigned long * ulTime,  char * pszVersion)
{
    char str[256] = {0};
    char szTime[32] = {0};
    int iRet = 0;
    int fd = 0;

    COMMON_TRACE(MID_CCP, "Enter COMM_GetConfigInfo!\n");
    if(!ulTime || !pszVersion) return TBS_FAILED;
    fd = open(LAST_CONFIG_INFO, O_RDONLY);
    if(fd<0)
    {
        COMMON_TRACE(MID_CCP, "Open %s fail,error is:%s!\n",  LAST_CONFIG_INFO, strerror(errno));
        return TBS_FAILED;
    }

    iRet = read(fd, str, sizeof(str)-1);
    if(iRet<0)
    {
        iRet = TBS_FAILED;
        COMMON_TRACE(MID_CCP, "Read %s fail,error is:%s!\n",  LAST_CONFIG_INFO, strerror(errno));
    }
    else
    {
        iRet = TBS_SUCCESS;
    }
    close(fd);
    if(iRet == TBS_SUCCESS)
    {
        COMMON_TRACE(MID_CCP, "str:%s\n",str);
        if(sscanf(str,"Time\t:%s\nVersion\t:%s",szTime, pszVersion)!= 2)
        {
            return TBS_FAILED;
        }
    }
    *ulTime = atoi(szTime);
    return TBS_SUCCESS;
}

#endif




