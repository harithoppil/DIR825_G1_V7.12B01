/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ�����: Common_fun.c
 �ļ�����: ���������ķ�װ���ṩ����ģ��ʹ��

 �޶���¼:
        1. ����: ��ΰ
           ����: 2008-08-07
           ����: �����ļ�

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
Description:   �պ�����do noting
Input:         char *pszPath                       ��ǰ�����·��
Output:        ��
Return:        ��
Others:
*************************************************************************/
int COMM_NullFunc(char *pszPath)
{
    return TBS_SUCCESS;
}


/*************************************************************************
Function:      void COMM_RemovePathLastDot(char *pszPath)
Description:   ȥ��·������'.'
Input:         char *pszPath                       ��ǰ�����·��
Output:        ��
Return:        ��
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
Description:   ����·��,����ʱȥ��·������'.'
Input:         const char *pszSrcPath,                       ��ǰ�����·��
                   char *pszDestPath,                         ������Ŀ��·��
Output:        ��
Return:        ��
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
Description:   ͨ�����е�·��ƴװ��·��
Calls:         ��
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iChangeLevel,                         �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
                   char *pszExtraPath                     ���� ��·����ΪNULL���ʾ�����(����û��.)
Output:        ��
Return:        NULL ,       ʧ��;
                    ����,     �ɹ���ֵ��ָ��
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

    /* ʵ��������cd ../../xx,�����ȴ���../.. */
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

    /* ���� ..���·��*/
    if (pszExtraPath)
    {
        sprintf(pszDestPath,"%s%s",pszDestPath,pszExtraPath);
    }
    else
    {
        /* pszExtraPathΪNULL,��ȥ������'.' */
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
Description:   ��ȡ·���еĵ�n ���ڵ�
Calls:         ��
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iPos,                                        ��ȡ�ڼ����ڵ�
Output:        ��
Return:        NULL ,       ʧ��;
                    ����,     �ɹ���ֵ��ָ��
Others:         Ϊ�˼򻯽ӿڣ�����ʹ����ȫ�ֱ����洢����ֵ��
                    ���Բ�֧�ֶ��̵߳Ĳ�������
*************************************************************************/
char szTempPathNode[MAX_PATH_LEN] = {0};
const char * COMM_GetPathNode(char *pszCurPath,int iPos)
{
    unsigned char ulCount=0;
    char *pReturnVal = NULL;

    char **papcAttrList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );

    safe_strncpy(szTempPathNode,pszCurPath,MAX_PATH_LEN);

    /* ȫ������������.�ָ��·��*/
    papcAttrList[ulCount] = strtok(szTempPathNode, ".");
    while (papcAttrList[ulCount])
    {
        ulCount++;
        papcAttrList[ulCount] = strtok(NULL, ".");
    }

    /* ����λ�ò�������������ֵ*/
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
Description:   ��ȡ·���е�index
Calls:         ��
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       ��ǰ�����·��
Output:        int *iIndex
Return:        ��ǰindex������
Others:         Ϊ�˼򻯽ӿڣ�����ʹ����ȫ�ֱ����洢����ֵ��
                    ���Բ�֧�ֶ��̵߳Ĳ�������
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
            /*�������е�����*/
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
Description:   ͳһ�Ļ�ȡֵ�ĺ����������ӻ����ȡ������������ȡ���ȴӻ��������������ַ�ʽ
Calls:         ��
Data Accessed:
Data Updated:
Input:         char *pszPath ,                       ��ǰ�����·��
                   ValuePostionType eGetMode  ��ȡֵ�÷�ʽ
Output:        ��
Return:        NULL ,       ʧ��;
                    ����,     �ɹ���ֵ��ָ��
Others:
*************************************************************************/
static ValuePostionType g_enValuePos = FROM_CACHE;

ValuePostionType COMM_GetLastPos()/*�������һ��COMM_GetNodeValue������ȡֵ��λ��*/
{
    return g_enValuePos;
}

const char * COMM_GetNodeValue(char *pszPath,ValuePostionType eGetMode)
{
    const char *pValue = NULL;
    int iRet = TBS_SUCCESS;


    if (eGetMode == FROM_CACHE)
    {
        /*Ϊ��Ӱ��ӿ���ʱʹ�ã����ں���ͳһ�޸�*/
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
        /*Ϊ��Ӱ��ӿ���ʱʹ�ã����ں���ͳһ�޸�*/
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
Description:  ��ȡֵ�ķ�װ�ӿڣ�֧��:
                    1.�����ȡֵ��·������ͨ��������·��ƴװ���
                    2.֧�ִӻ��棬�����������Ȼ�����������ȼ��ַ�ʽ��ýڵ�ֵ
Calls:         ��
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iChangeLevel,                         �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
                   char *pszExtraPath                     ���� ��·����ΪNULL���ʾ�����
                   ValuePostionType eGetMode        ��ȡֵ�ķ�ʽ(    FROM_CACHE �ӻ���, FROM_CFG ��������,  CACHE_CFG �Ȼ����������)
Output:        ��
Return:        NULL ,       ʧ��;
                    ����,     �ɹ���ֵ��ָ��
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
Description:  ��ȡ����ڵ�ֵ�ķ�װ�ӿڣ�֧��:��·������ͨ��������·��ƴװ���
                    1.�����ȡֵ��·������ͨ��������·��ƴװ���
                    2.֧�ִӻ��棬�����������Ȼ�����������ȼ��ַ�ʽ��ýڵ�ֵ
Input:         char *pszCurPath ,               ��ǰ�����·��
               int iChangeLevel,                �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
               ValuePostionType eGetMode        ��ȡֵ�ķ�ʽ  (    FROM_CACHE �ӻ���, FROM_CFG ��������,  CACHE_CFG �Ȼ����������)
               unsigned short usNodeCount       �ڵ����
               ...                              ָ����,�������ջ�ȡ�Ľڵ�ֵ,
                                                ��: char *pcNodeName, char **ppcNodeValue
Output:        ��
Return:        �ɹ�:TBS_SUCCESS
               ʧ��:������
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

    /* ƴ��·��ǰ׺,���治��Ҫ�ظ�ƴ�� */
    iRet = COMM_GetPathEx(pszCurPath, iChangeLevel, "", szTempPath, NULL);
    if ( RET_FAILED(iRet) )
    {
        return iRet;
    }

    pszPos += strlen(szTempPath);
    ulBufLen = MAX_PATH_LEN - (pszPos - szTempPath);

    /* ����ڵ��ȡֵ */
    va_start(arg_ptr, usNodeCount);
    for (i = 0; i < usNodeCount; i++)
    {
        pszNodeName = va_arg(arg_ptr,char *);
        ppszNodeValue = va_arg(arg_ptr,char **);

        /* ����Ƿ������Ĳ��� */
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
Description:  ͨ������ڵ�����������ȡ����ڵ�ֵ
              ֧��:��·������ͨ��������·��ƴװ���
                1.�����ȡֵ��·������ͨ��������·��ƴװ���
                2.֧�ִӻ��棬�����������Ȼ�����������ȼ��ַ�ʽ��ýڵ�ֵ
Input:         char *pszCurPath ,               ��ǰ�����·��
               int iChangeLevel,                �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
               ValuePostionType eGetMode        ��ȡֵ�ķ�ʽ  (FROM_CACHE �ӻ���, FROM_CFG ��������,  CACHE_CFG �Ȼ����������)
               unsigned short usNodeCount       �ڵ����
               const char * apcNodeArr[]        �ڵ����飬�ڵ�������Ϊ�յĽڵ����
               void *pvNodeValArr               �������սڵ�ֵ�Ľṹ������ַ�����
Output:        ��
Return:        �ɹ�:TBS_SUCCESS
               ʧ��:������
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

    /* ƴ��·��ǰ׺,���治��Ҫ�ظ�ƴ�� */
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
            /* ���Կսڵ� */
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
Description:  ͨ������ڵ�������ͽڵ�ֵ������ͬʱ���ö���ڵ�ֵ
              ֧��: ��·������ͨ��������·��ƴװ���
Input:        char *pszCurPath ,               ��ǰ�����·��
              int iChangeLevel,                �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
              unsigned short usNodeCount       �ڵ����
              const char * apcNodeArr[]        �ڵ����飬�ڵ�������Ϊ�յĽڵ����
              void *pvNodeValArr               �������õĽڵ�ֵ�ַ�������߽ṹ��(���г�Ա�����ַ�ָ��)
Output:       ��
Return:       �ɹ�:TBS_SUCCESS
              ʧ��:������
Others:
*************************************************************************/
int COMM_SetMultiNodeValueByStrArrEx(const char *pszCurPath, int iChangeLevel,
        unsigned short usNodeCount, const char *apcNodeArr[], void *pvNodeValArr)
{
    int iRet = TBS_SUCCESS;
    char szTempPath[MAX_PATH_LEN] = {0};

    /* ƴ��·��ǰ׺,���治��Ҫ�ظ�ƴ�� */
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
Description:  ����ֵ�ķ�װ�ӿڣ�֧��:��������ֵ��·������ͨ��������·��ƴװ���
Calls:         ��
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iChangeLevel,                         �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
                   char *pszExtraPath                     ���� ��·����ΪNULL���ʾ�����
                   char *pszValue                            ���õ�ֵ
Output:        ��
Return:        TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
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
Description:    ���ö���ڵ�ֵ�ķ�װ�ӿڣ�֧��:��������ֵ��·������ͨ��������·��ƴװ���
Calls:          ��
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,               ��ǰ�����·��
               int iChangeLevel,                �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
               unsigned short usNodeCount       ���ýڵ�ĸ���
               ...                              ָ����,��������Ҫ���õĽڵ����ƺͽڵ�ֵ,
                                                ��: char *pcNodeName, char *pcNodeValue
Output:        ��
Return:        TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
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

    /* ƴ��·��ǰ׺,���治��Ҫ�ظ�ƴ�� */
    iRet = COMM_GetPathEx(pszCurPath,iChangeLevel,"",szTempPath, NULL);
    if ( RET_FAILED(iRet) )
    {
        return iRet;
    }

    pszPos += strlen(szTempPath);
    ulBufLen = MAX_PATH_LEN - (pszPos - szTempPath);

    /* ����ڵ�����ֵ */
    va_start(arg_ptr, usNodeCount);
    for (i = 0; i < usNodeCount; i++)
    {
        pszNodeName = va_arg(arg_ptr,char *);
        pszNodeValue = va_arg(arg_ptr,char *);

        /* ����Ƿ������Ĳ��� */
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
Description:    ���ö���ڵ㻺��ķ�װ�ӿ�

Input:         usMID                    �ڵ�ֵ������MID
               char *pszCurPath ,       ��ǰ�����·��
               int iChangeLevel,        �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
               usNodeCount              ���ýڵ�ĸ���,Ϊ0ʱ��ʾʹ�õı��ΪST_SET_NODE_VAL_ENTRY
                                        ���顣���ڵ���1ʱ��ʾʹ�õı��Ϊ�ڵ����ƺ�ֵ��ָ�롣
               ...                      (usNodeCount����0ʱ)ָ��ST_SET_NODE_VAL_ENTRY�����ָ��,
                                        ������������Ԫ�ر���ΪNULLָ���Ա�ʾ����Ľ�β��
                                        (usNodeCount>0ʱ)ָ����,��ʽ
                                        ��: char *pcNodeName, char *pcNodeValue
Return:        TBS_SUCCESS ,      �ɹ�;
               ����,              ʧ��
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

    /*ƴ��·��ǰ׺*/
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

            /* ����Ƿ������Ĳ��� */
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
Description:    ��ȡ����ڵ㻺��ֵ�ķ�װ�ӿ�
                1.�����ȡ����ֵ��·������ͨ��������·��ƴװ���

Input:   usMID                       �ڵ�������MID
         char *pszCurPath ,          ��ǰ�����·��
         int iChangeLevel,           �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
         unsigned short usNodeCount  ���ýڵ�ĸ���
         ...                         ָ����,��������Ҫ���õĽڵ����ƺͽڵ�ֵ,
                                     ��: char *pcNodeName, char **ppcNodeValue
Return:        TBS_SUCCESS , �ɹ�;
               ����,         ʧ��
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

    /*ƴ��·��ǰ׺*/
    iRet = COMM_GetPathEx(pszCurPath, iChangeLevel, "", szTempPath, NULL);
    if (RET_FAILED(iRet))
    {
        return iRet;
    }

    pszPos += strlen(szTempPath);
    ulBufLen = MAX_PATH_LEN - (pszPos - szTempPath);

    /* ����ڵ��ȡֵ */
    va_start(arg_ptr, usNodeCount);
    for (i = 0; i < usNodeCount; i++)
    {
        pszNodeName = va_arg(arg_ptr,char *);
        ppszNodeValue = va_arg(arg_ptr,const char **);

        /* ����Ƿ������Ĳ��� */
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
Description:    ɾ������ڵ㻺��ֵ�ķ�װ�ӿ�
                1.����ɾ������ֵ��·������ͨ��������·��ƴװ���

Input:   usMID                       �ڵ�������MID
         char *pszCurPath ,          ��ǰ�����·��
         int iChangeLevel,           �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
         unsigned short usNodeCount  ���ýڵ�ĸ���
         ...                         ָ����, ָ��Ҫɾ������Ľڵ�����
Return:        TBS_SUCCESS , �ɹ�;
               ����,         ʧ��
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

    /*ƴ��·��ǰ׺*/
    iRet = COMM_GetPathEx(pszCurPath, iChangeLevel, "", szTempPath, NULL);
    if (RET_FAILED(iRet))
    {
        return iRet;
    }

    pszPos += strlen(szTempPath);
    ulBufLen = MAX_PATH_LEN - (pszPos - szTempPath);

    /* ���ɾ���ڵ㻺�� */
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
Description:  ���û���ֵ�ķ�װ�ӿڣ�֧��:��������ֵ��·������ͨ��������·��ƴװ���
Calls:         ��
Data Accessed:
Data Updated:
Input:     unsigned short usMD   ����ģ��MID
               char *pszCurPath,       ��ǰ�����·��
               int iChangeLevel,         �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
               char *pszExtraPath     ���ӵ�·����ΪNULL���ʾ�����
               char *pszValue            ���õ�ֵ
Output:        ��
Return:        TBS_SUCCESS ,      �ɹ�;
               ����,     ʧ��
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

Description:  ��ȡ����ֵ�ķ�װ�ӿڣ�֧��:��������ֵ��·������ͨ��������·��ƴװ���

Input:     usMID,          ����ģ��MID
           pszCurPath,     ��ǰ�����·��
           iChangeLevel,   �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
           pszExtraPath    ���ӵ�·����ΪNULL���ʾ�����

Return:    ����ֵָ��,   �ɹ�
           NULL,         ʧ��
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

Description:  ��ģ�黺��ֵ���浽��������

Input:     usMID,   ģ��MID

Return:   ��

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
����: ���ָ��·����ʵ��֮���Ƿ��нڵ�ֵ��ͻ(����ڵ�)
����: char *pszMatchPath           ��Ҫ����ͨ��·��(�������NULL����ݵ�ǰ·���Զ�����)
      char *pszCurPath             ��Ҫ���ĵ�ǰ·��
      int iChangeLevel             �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
      int  iNodeCnt                �ڵ����
      char **apcNodeArr            �����Ľڵ�����
����: �ɹ� -- TBS_SUCCESS
      ʧ�� -- ����������
��ע:
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
      �ӻ����л�ȡ��ǰʵ���ڵ�ֵ
      InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.1.Name
      InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANIPConnection.1.
    */
    iRet = COMM_GetMultiNodeValueByStrArrEx(pszCurPath, iChangeLevel, CACHE_CFG,
            iNodeCnt, (const char **)apcNodeArr, apcCurNodeValArr);
    RET_RETURN(MID_CCP, iRet, "Get instance sub node failed, Path = [%s]\n", pszCurPath);

    if (NULL == pszMatchPath)
    {
        /*
          ƴ��·�����õ�ʵ���ĸ�·��
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
      ��ȡ·��������ʵ��
      InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.
    */
    iRet = CFG_GetMatchNodeName(szMatchPath, 1, &pstParaList, NULL);
    RET_RETURN(MID_CCP, iRet, "get node list error!\n");

    /* ��������ʵ�������αȽ� */
    for(pstParaTmp = pstParaList; NULL != pstParaTmp; pstParaTmp = pstParaTmp->pstNext)
    {
        /*
          ��������
          pszCurPath/pstParaBuf->pcName:
          InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.1.Name
          InternetGatewayDevice.WANDevice.{i}.WANConnectionDevice.{i}.WANIPConnection.1.
         */
        if (safe_strstr(pszCurPath, pstParaTmp->pcName))
        {
            continue;
        }

        /* �������л�ȡʵ���ڵ�ֵ */
        iRet = COMM_GetMultiNodeValueByStrArrEx(pstParaTmp->pcName, 0, FROM_CFG,
                iNodeCnt, (const char **)apcNodeArr, apcTmpNodeValArr);
        if (RET_FAILED(iRet))
        {
            COMMON_TRACE(MID_CCP, "Get instance sub node failed, Path = [%s]\n", pstParaTmp->pcName);
            break;
        }

        iConflict = 1;

        /* �����ڵ�����, �Ƚ����нڵ�ֵ */
        for (i = 0; i < iNodeCnt; i++)
        {
            /* ֻҪ��һ���ڵ�ֵ���ȣ��Ͳ��ᷢ����ͻ */
            if (0 != safe_strcmp(apcCurNodeValArr[i], apcTmpNodeValArr[i]))
            {
                iConflict = 0;
                break;
            }
        }

        if (iConflict)
        {
            /* �Ƚ���ϣ����нڵ�ֵ��ȣ�˵��������ͻ */
            iRet = TBS_FAILED;
            break;
        }
    }

    /* �ͷ��ڴ� */
    CFG_MemFree(pstParaList);

    return iRet;
}

/*************************************************************************
Function:    int COMM_AddObjectEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath)
Description:  ���object�ķ�װ�ӿڣ�֧��:��������ֵ��·������ͨ��������·��ƴװ���
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iChangeLevel,                         �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
                   char *pszExtraPath                     ���� ��·����ΪNULL���ʾ�����
Output:        ��
Return:        TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
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
Description:  ɾ��object�ķ�װ�ӿڣ�֧��:��������ֵ��·������ͨ��������·��ƴװ���
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iChangeLevel,                         �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
                   char *pszExtraPath                     ���� ��·����ΪNULL���ʾ�����
Output:        ��
Return:        TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
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
Description:  ��ȡֵ������ͬ����ǣ�ͬ����Ǳ�ʾ��ӻ�����ȥ������commit�����ȵ���
Calls:         ��
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iChangeLevel,                         �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
                   char *pszExtraPath                     ���� ��·����ΪNULL���ʾ�����
                   ValuePostionType eGetMode        ��ȡֵ�ķ�ʽ(    FROM_CACHE �ӻ���, FROM_CFG ��������,  CACHE_CFG �Ȼ����������)
Output:        ��
Return:        NULL ,       ʧ��;
                    ����,     �ɹ���ֵ��ָ��
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
Description:  ��ĳ��·�����б���
Calls:         ��
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   char *pszCurVal,                          ��ǰֵ
                   int iChangeLevel,                         ������·���Ĳ�Σ���ʾ���ұ�ȥ���Ľڵ���
                   CheckEachFunc pfnCheck            ��麯��
Output:        ��
Return:       true ,      �ɹ�;
                   false,     ʧ��
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
Description:  ��ĳ��·�����б���
Calls:         ��
Data Accessed:
Data Updated:
Input:     char *pszCurPath ,                       ��ǰ�����·��
           ForEachFunc pfnForEach            ����ʹ�õĺ���
           void *pData,                                �����˽������
           int bRet                                     ѭ���˳�������
                        0----ѭ��������һ����TBS_SUCCESS�����˳�
                        1-----ѭ��������һ��TBS_SUCCESS�����˳�
                        2-----ѭ����ϲ��˳�
Output:        ��
Return:       true ,      �ɹ�;
                   false,     ʧ��
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


    /*����path��������*/
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
        if ('%' != *(papcAttrList[i])) /*����%,���ʾ��㲻��ѭ��*/
        {
            strcat(szObjPath,papcAttrList[i]);
            strcat(szObjPath,".");
            i++;
        }
        else  /*����%, ��Ҫ����ѭ��*/
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

            /*�Ա������ѭ������*/
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

                /*�ݹ���ã��Խ���������*/
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

    /*·����û��%*/
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
Description:  ��ֵͬ����������
Calls:         ��
Data Accessed:
Data Updated:
Input:         char *pszPath ,                       ��ǰ�����·��
Output:        ��
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
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
Description:  ��Ч��ģ�����еĽڵ�
Calls:         ��
Data Accessed:
Data Updated:
Input:         unsigned short usMID ,                       ��ǰ��MID
Output:        ��
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
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

     /*ѭ��ȡ��ÿ��ʵ���е�ֵ�����뵽cache��*/
    while (ret = CFG_ListMIDNextInstNodes(&pstListHead),
            CFG_OK == ret && NULL != pstListHead)
    {
        for (pstList = pstListHead; NULL != pstList; pstList = pstList->pstNext)
        {
             /*Ϊ����ͳ�ƽڵ������Ч���������ٶȣ���������Stat=1�Ľڵ㲻����cache*/
            ret = CFG_GetNodeAttrValPtr(pstList->pcName,ATTR_STAT,(const char**)&pszStat);
            if ((ret == CFG_OK && pszStat[0] != '1') || (ret != CFG_OK))
            {
                /* ���ڲ���д�Ľڵ㲻д��cache */
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

        /*��cache�нڵ������Ч*/
    if (MSG_MSG_GetProcessFunc(usMID,MSG_CMM_COMMIT,&pMsgData))
    {
        if (pMsgData)
        {
            struct list_head *pList = MSG_MSG_GetModuleCache(usMID);
            iRet = CacheList_ForEachOnce(usMID,pList,pMsgData,ApplyAccessor);

            /*POST APPLY ������ȫ��������  */
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
Description:  ��Ч��ģ��ĳ·�������еĽڵ㣬������COMM_ApplyAllNodes
Calls:         ��
Data Accessed:
Data Updated:
Input:         unsigned short usMID ,                       ��ǰ��MID
                   const char * pszPath,                        ��ǰ��·��
Output:        ��
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
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
                /* ���ڲ���д�Ľڵ㲻����cache */
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
Description:  �����Զ�����Ϣ��
Calls:         ��
Data Accessed:
Data Updated:
Input:         ST_MSG *pstMsg       ��Ϣ��
                  char *pszFormat       ��Ҫ�Ľ���name�����Կո��������:"name3 name2"
Output:        ���                      ��ѽ�����ֵ���뵽�����
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
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


    /*�������*/
    if (NULL == pstMsg)
    {
        iRet = TBS_PARAM_ERR;
        return iRet;
    }

    /*����format*/
    papcAttrList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );

    safe_strncpy(szTempFormat,pszFormat,MAX_PATH_LEN);

    papcAttrList[ulCount] = strtok(szTempFormat, " ");
    while (papcAttrList[ulCount])
    {
        ulCount++;
        papcAttrList[ulCount] = strtok(NULL, " ");
    }


    /*���������б�*/
    papcParaList = (char **)malloc(ulCount * sizeof(char *) );

    va_start(arg_ptr, pszFormat);
    for (i = 0; i < ulCount; i++)
    {
        ptr = va_arg(arg_ptr,char *);
        papcParaList[i] = ptr;
    }
    va_end(arg_ptr);


    /*������֪ͨ��Ϣ�ĸ����ֶ�*/
    pcPos = pstMsg->szMsgBody;
    GET_ULONG(pcPos, ulNodeCount);


    for (i = 0; i < ulNodeCount; i++)
    {
        /* ��ȡһ���������� */
        GET_STR(pcPos, pszMsgNode);

        /* �������������ȡ�ڵ������ڵ�ֵ��Index��� */
        iRet = ParseSetCmd(pszMsgNode, szTempPath, szTempValue, NULL, NULL);
        if (RET_FAILED(iRet))
        {
            COMMON_TRACE(MID_CCP,"Parse msg failed.");
            goto EXIT;
        }

        /* ȡ���ڵ�·�� */
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
Description: ������Ϣ��������Ӧ
Calls:       ��
Data Accessed:
Data Updated:
Input:       int iNeedResponse      �Ƿ���Ҫ������Ӧ: 0 ������, 1 ���ղ�����
             ST_MSG *pstMsg         ��Ϣ��
             char *pszFormat        ��Ҫ�Ľ���name�����Կո��������:"name3 name2"
Output:      ���                   ��ѽ�����ֵ���뵽�����
Return:      TBS_SUCCESS:�ɹ�       ����:ʧ��
Others:      ��COMM_MakeCustomMsg���ʹ��,��Ҫ���ڽ��̼�ͨ��
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

    /*�������*/
    if (NULL == pstMsg)
    {
        iRet = TBS_PARAM_ERR;
        return iRet;
    }

    /*���Ͳ�������Ϣ*/
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

    /*����format*/
    papcAttrList = (char **)malloc(MAX_PATH_DEPTH * sizeof(char *) );

    safe_strncpy(szTempFormat,pszFormat,MAX_PATH_LEN);

    papcAttrList[ulCount] = strtok(szTempFormat, " ");
    while (papcAttrList[ulCount])
    {
        ulCount++;
        papcAttrList[ulCount] = strtok(NULL, " ");
    }


    /*���������б�*/
    papcParaList = (char **)malloc(ulCount * sizeof(char *) );

    va_start(arg_ptr, pszFormat);
    for (i = 0; i < ulCount; i++)
    {
        ptr = va_arg(arg_ptr,char *);
        papcParaList[i] = ptr;
    }
    va_end(arg_ptr);


    /*������֪ͨ��Ϣ�ĸ����ֶ�*/
    pcPos = pstMsg->szMsgBody;
    GET_ULONG(pcPos, ulNodeCount);


    for (i = 0; i < ulNodeCount; i++)
    {
        /* ��ȡһ���������� */
        GET_STR(pcPos, pszMsgNode);

        /* �������������ȡ�ڵ������ڵ�ֵ��Index��� */
        iRet = ParseSetCmd(pszMsgNode, szTempPath, szTempValue, NULL, NULL);
        if (RET_FAILED(iRet))
        {
            COMMON_TRACE(MID_CCP,"Parse msg failed.");
            goto EXIT;
        }

        /* ȡ���ڵ�·�� */
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
Description:    �����Զ�����Ϣ��
Calls:          ��
Data Accessed:
Data Updated:
Input:        ST_MSG *pstMsg              ��Ϣ��
              unsigned short usMsgType    ��Ϣ����
              unsigned short usSrcMID     ԴMID
              unsigned short usDstMID     Ŀ��MID
              unsigned short usNodeNum    ������Ϣ�Ľڵ���
Output:       ��Σ���ѽ�����ֵ���뵽�����
Return:       TBS_SUCCESS ,      �ɹ�;
			  ����,              ʧ��
Others:       �������Ϣ������ name1=value1...���͵�
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

    /* ������Ϣ����*/
    pcPos = pstMsg->szMsgBody;
    SET_ULONG(pcPos, usNodeNum);

    for (i=0;i<usNodeNum;i++)
    {
        memset(szTempBuf, 0, sizeof(szTempBuf));
        sprintf(szTempBuf, "%s=%s", papcNameList[i],papcAttrList[i]);
        SET_STR(pcPos, szTempBuf);
    }

    /* ������Ϣͷ*/
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
Description:    ���첢�����Զ�����Ϣ��
Calls:          ��
Data Accessed:
Data Updated:
Input:          unsigned short usMsgType    ��Ϣ����
                unsigned short usSrcMID     ԴMID
                unsigned short usDstMID     Ŀ��MID
                unsigned short usNodeNum    ������Ϣ�Ľڵ���
                ���                        �����õ��ַ���
Output:
Return:         TBS_SUCCESS ,               �ɹ�
                ����,                       ʧ��
Others:         �������Ϣ������ name1=value1...���͵�
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

    /*�������б��*/
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

    /* ������Ϣ����*/
    pcPos = pstMsg->szMsgBody;
    SET_ULONG(pcPos, usNodeNum);

    for (i=0;i<usNodeNum;i++)
    {
        memset(szTempBuf, 0, sizeof(szTempBuf));
        sprintf(szTempBuf, "%s=%s", papcNameList[i],papcAttrList[i]);

        SET_STR(pcPos, szTempBuf);
    }

    /* ������Ϣͷ*/
    pstMsg->stMsgHead.ulMsgID = GenMsgID();
    pstMsg->stMsgHead.usSrcMID = usSrcMID;
    pstMsg->stMsgHead.usDstMID = usDstMID;
    pstMsg->stMsgHead.usMsgType = usMsgType;

    pstMsg->stMsgHead.ulBodyLength = pcPos-pstMsg->szMsgBody;

    iRet = MSG_SendMessage(pstMsg);

    /* ����ʧ��*/
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
Description:    ���첢�����Զ�����Ϣ��Ļظ���Ϣ
Calls:          ��
Data Accessed:
Data Updated:
Input:          ST_MSG *pstMsg              ���յ�����Ϣ��
                unsigned short usMsgType    �ظ�����Ϣ����
                unsigned short usNodeNum    ������Ϣ�Ľڵ���
                ���                        �����õ��ַ���
Output:
Return:         TBS_SUCCESS ,               �ɹ�
                ����,                       ʧ��
Others:         �������Ϣ������ name1=value1...���͵�
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

    /*�������б��*/
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

    /* ������Ϣ����*/
    pcPos = pstResponseMsg->szMsgBody;
    SET_ULONG(pcPos, usNodeNum);

    for (i=0;i<usNodeNum;i++)
    {
        memset(szTempBuf, 0, sizeof(szTempBuf));
        sprintf(szTempBuf, "%s=%s", papcNameList[i],papcAttrList[i]);

        SET_STR(pcPos, szTempBuf);
    }

    /* ������Ϣͷ*/
    pstResponseMsg->stMsgHead.ulMsgID = pstMsg->stMsgHead.ulMsgID;
    pstResponseMsg->stMsgHead.usSrcMID = pstMsg->stMsgHead.usDstMID;
    pstResponseMsg->stMsgHead.usDstMID = pstMsg->stMsgHead.usSrcMID;
    pstResponseMsg->stMsgHead.usMsgType = usMsgType;

    pstResponseMsg->stMsgHead.ulBodyLength = pcPos-pstResponseMsg->szMsgBody;

    iRet = MSG_SendMessage(pstResponseMsg);

    /* ����ʧ��*/
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
Description:    �����Զ�����Ϣ��,�����Ƹ�ʽ
Calls:          ��
Data Accessed:
Data Updated:
Input:          unsigned short usMsgType    ��Ϣ����
                unsigned short usSrcMID     ԴMID
                unsigned short usDstMID     Ŀ��MID
                unsigned short usMsgLen     ��Ϣ�峤��
                void *pcMsgBody             ��Ϣ������
Output:
Return:         ST_MSG* ,                   �ɹ�
                NULL,                       ʧ��
Others:         �������Ϣ�������������͵Ķ����Ƹ�ʽ
*************************************************************************/
ST_MSG *COMM_MakeBinMsg(unsigned short usMsgType,unsigned short usSrcMID,
    unsigned short usDstMID,unsigned short usMsgLen, void *pcMsgBody)
{
    char *pcPos = NULL;
    ST_MSG *pstMsg = NULL;

    /* ������Ϣ */
    pstMsg = MSG_CreateMessage(usMsgLen);
    if (NULL == pstMsg)
    {
        COMMON_LOG_ERROR(MID_CCP, "Create Notify Message Failed.\n");
        goto EXIT;
    }

    /* ������Ϣ����*/
    pcPos = pstMsg->szMsgBody;
    if (usMsgLen > 0)
    {
        memcpy(pcPos, pcMsgBody, usMsgLen);
        pcPos += usMsgLen;
    }

    /* ������Ϣͷ*/
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
Description:    ���첢�����Զ�����Ϣ��,�����Ƹ�ʽ
Calls:          ��
Data Accessed:
Data Updated:
Input:          unsigned short usMsgType    ��Ϣ����
                unsigned short usSrcMID     ԴMID
                unsigned short usDstMID     Ŀ��MID
                unsigned short usMsgLen     ��Ϣ�峤��
                char *pcMsgBody             ��Ϣ������
Output:
Return:         TBS_SUCCESS ,               �ɹ�
                ����,                       ʧ��
Others:         �������Ϣ�������������͵Ķ����Ƹ�ʽ
*************************************************************************/
int COMM_MakeAndSendBinMsg(unsigned short usMsgType,unsigned short usSrcMID,
    unsigned short usDstMID,unsigned short usMsgLen, void *pcMsgBody)
{
    int iRet = 0;

    char *pcPos = NULL;
    ST_MSG *pstMsg = NULL;

    /* ������Ϣ */
    pstMsg = MSG_CreateMessage(usMsgLen);
    if (NULL == pstMsg)
    {
        iRet = TBS_OUT_OF_MEM;
        COMMON_LOG_ERROR(MID_CCP,"Create Notify Message Failed.\n");
        goto EXIT;
    }

    /* ������Ϣ����*/
    pcPos = pstMsg->szMsgBody;
    if (usMsgLen > 0)
    {
        memcpy(pcPos, pcMsgBody, usMsgLen);
        pcPos += usMsgLen;
    }

    /* ������Ϣͷ*/
    pstMsg->stMsgHead.ulMsgID = GenMsgID();
    pstMsg->stMsgHead.usSrcMID = usSrcMID;
    pstMsg->stMsgHead.usDstMID = usDstMID;
    pstMsg->stMsgHead.usMsgType = usMsgType;

    pstMsg->stMsgHead.ulBodyLength = pcPos-pstMsg->szMsgBody;

    iRet = MSG_SendMessage(pstMsg);

    /* ����ʧ��*/
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
Description:  ����cacheֵ
Calls:         ��
Data Accessed:
Data Updated:
Input:        unsigned short usMID      ģ���MID
                  char *pszPath                  ���õ�·��
                  char *pszValue                ���õ�ֵ
Output:
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:       �������Ϣ������ name1=value1...���͵�
*************************************************************************/
int COMM_SetCacheValue(unsigned short usMID,char *pszPath,char *pszValue)
{
    const char *pszCfgVal = NULL, *pszCacheVal = NULL;

    if ((pszPath==NULL)||(pszValue == NULL))
    {
        return TBS_FAILED;
    }

    int i = MID2INDEX(usMID);

    /*���ȶ�ȡ����ֵ,����ڵ�ֵ�Ѿ��ڻ�����������������һ��*/
    pszCacheVal = GetCacheNode(&(g_Module_Table[i].listCache), pszPath);
    if (pszCacheVal)
    {
        SetCacheNode(&(g_Module_Table[i].listCache),pszPath,pszValue);
    }
    else
    {
        /*���������ж�ȡ�ڵ�ֵ,����ڵ�ֵ����������ֵ��ͬ,�Ž�����
        ����Ľڵ�ֵ���õ�������*/
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
Description:  ��ȡ�ڵ�cacheֵ

Input:      usMID      ģ���MID
            pszPath    �ڵ�·��

Return:       �ڵ㻺��ֵָ�� ,      �ɹ�;
              NULL,                 ʧ��
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
Description: ɾ������pszPathָ���·���ڵ�Ļ���

Input:      usMID      ģ���MID
            pszPath    �ڵ�·��

Return:       TBS_SUCCESS,      �ɹ�;
              ����,             ʧ��
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
Description:  ���cache
Calls:         ��
Data Accessed:
Data Updated:
Input:        unsigned short usMID      ģ���MID
Output:
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
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
Description:  ��ȡģ��ID,��ģ���������ɲ��������ģ��Ļ���,
              һ�������ģ��Ļ��涼���ɹ���ģ����������

Input:        usCacheMID,     ����������ģ���MID
Output:
Return:       ������usCacheMID�����ģ��MID ,      �ɹ�;
              ����,                                ʧ��
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

Description:  ���ô��������ģ��ID, usCacheMID��ʾ����������ģ��ID,
              usCacheBuildMID��ʾ���������ģ��ID

Input:        usCacheMID,       ��������ģ���MID
              usCacheBuildMID,  ������ģ��usCacheMID�����MID
Output:
Return:       TBS_SUCCESS,      �ɹ�;
              ����,             ʧ��
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
Description:  ����ģ����Ҫ��¼��ǰ�����ʵ��ֵ֮���ͻʱ���������ĸ�·������
Calls:        ��
Data Accessed:
Data Updated:
Input:        char *pszPath      ��ǰ�����·��
Output:
Return:       ��
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
Description:   ��ȡshell����ı���Ϣ��ĳЩ�е�ĳЩ�ֶΣ��÷�������strtok
Input:         char *szBuf,                             ������ַ�buf
                   char *szDelim,                         �ָ���ַ���������ָ�ÿ���õ��ַ�����
                   int iMinLine, int iMaxLine,        ��Ҫ��ȡ��Ϣ��������>=iMinLine,<=iMaxLine,��0��ʼ��ţ�
                                                                    ��iMinLineΪ0ʱ��ʾ��������һ�п�ʼ��iMaxLineΪ-1��ʾ����û������
                   int iParaNum,                           ��ȡ��������������м���
                   ...                                              ��Σ���Ҫ��ȡ���кͶ�Ӧֵ��ָ�룬��:2,pCol2,4,pCol4 (ע��col����������������)
Output:        ...                                              ��ż������λ��ǰһ����ֵ��ָ�����
Return:        false,       ʧ��;
                    true,     �ɹ�
Others:       �÷�������strtok����һ������ΪNULL,��ʾ����ѭ�����ǿձ�ʾ��ʼѭ������
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

    /*�������У��������*/
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

    /*��Ը��н��зֲ�*/
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
Description:   ��ȡshell���������Ϣ
Input:         const char *pszCmd,              Ҫִ�е�����
                   char * pszResult,                    ִ�е�������
                   int iCount                                ��ǰbuffer�ĳ���
Output:
Return:        false ,       ʧ��;
                    true,     �ɹ�;
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
Description:   ������Ĵ��н�����Ҫ����Ϣ���ʺϽ���������ifconfig�����Ϣ�͵�����
Input:         char *szBuf,                            �����buf
                   char *szDelim,                       �ָ���ַ���
                   char *szName                         ��Ҫ���ҵ�����
Output:       char **pszValue                      �������Ľ��
Return:        false ,       ʧ��;
                    true,     �ɹ�;
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
Description:   ����ʱ��ӡʱ���ʹ��
Input:         char *pszInfo,                         ��Ҫ��ӡ��������Ϣ
Return:        ��
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
����: ���ص�ǰfactory mode
����: ValuePostionType eGetMode  ��ȡֵ�÷�ʽ      
����: 
		�ɹ� -- ����X_TWSZ-COM_FactoryMode��ֵ
    ʧ�� -- NULL
��ע:
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
    
/***************�ֳ�����**************/
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
/***************�������**************/

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




