/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ�����: Common_fun.c
 �ļ�����: ���������ķ�װ���ṩ����ģ��ʹ��

 �޶���¼:
        1. ����: all
           ����: 2007-08-07
           ����: �����ļ�

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
����: ���ڵ��ֵ���浽���û�����
����: void *pInstance,               ��ǰʵ��;
               const char *pszName,    �ڵ���;
               const char * pszValue,   �ڵ�ֵ
               const char *pszPath       ·��
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ��ȡ�ڵ���Ϣ�б�*/
    nNodeCount = pstInstance->nNodeCount;
    ppstNodeInfoList = pstInstance->ppstNodeInfoList;
    if ( NULL == ppstNodeInfoList )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    /* ���ҽڵ���Ϣ*/
    pstNodeInfo = FindNodeInfo(ppstNodeInfoList, pszName);
    if ( NULL == pstNodeInfo )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_NAME;
    }

    /* Խ��*/
    iOffset = pstNodeInfo - pstInstance->pstNodeInfoArray;
    if ( iOffset < 0 || iOffset >= nNodeCount )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_INDEX_OVERFLOW;
    }

    /* �жϽڵ��Ƿ��д*/
    if ( pstNodeInfo->bReadOnly )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_WRITE_FORBID;
    }

    /* ���ֵ�ĺϷ���*/
    if ( NULL != pstNodeInfo->pCheckFunc )
    {
        if ( ! (pstNodeInfo->pCheckFunc(pszValue)) )
        {
            COMM_TRACE(INFO_ERROR"\n");
            return TR069_ERRNO_INVALID_VAL;
        }
    }

    /* ��ȡ���û���������*/
    ppcIdxTable = pstInstance->ppcIdxTable;

    /* ����½ڵ�ֵ�Ƿ���������е���ͬ*/
    if ( NULL != pszPath )
    {
        COMM_TRACE("pszPath = %s\n", pszPath);

        /* ���������л�ȡ�ɽڵ�ֵ*/
        ulValLen = MAX_NODE_VALUE_LEN;
        iRet =  CFG_GetLeafValAndType(pszPath, szOldValue, &ulValLen, NULL, NULL);
        if ( RET_SUCCEED(iRet) && 0 == safe_strcmp(szOldValue, pszValue) )
        {
            if ( NULL != ppcIdxTable )
            {
                COMM_TRACE("\n");

                /* �ͷŽڵ�ֵ����*/
                safe_free(ppcIdxTable[iOffset]);

                /* ������û������Ƿ���ֵ*/
                for ( i = 0; i < nNodeCount; i++ )
                {
                    if ( NULL != ppcIdxTable[i] )
                    {
                        break;
                    }
                }

                /* ������û�����û��ֵ���ͷŵ����û���������*/
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

    /* ���û�������������*/
    if ( NULL == ppcIdxTable )
    {
        COMM_TRACE("\n");

        /* ��̬�������û���������*/
        ppcIdxTable = (char **)malloc(nNodeCount  * sizeof(char *));
        if ( NULL == ppcIdxTable )
        {
            COMM_TRACE(INFO_ERROR"\n");
            return TBS_OUT_OF_MEM;
        }

        /* ��ʼ��*/
        memset(ppcIdxTable, 0, nNodeCount  * sizeof(char *));
    }
    else
    {
        /* ����½ڵ�ֵ�Ƿ�����û����е���ͬ*/
        if ( 0 == safe_strcmp(ppcIdxTable[iOffset], pszValue) )
        {
            COMM_TRACE(INFO_SUCCESS"\n");
            return TBS_SUCCESS;
        }
        else
        {
            /* �ͷŵ���ǰ�����û���*/
            COMM_TRACE("\n");
            safe_free(ppcIdxTable[iOffset]);
        }
    }

    /* Ϊ�µĽڵ�ֵ����ռ�*/
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

    /* ���ڵ�ֵ���û�����ӵ����û�����������*/
    strcpy(pszValueBuf, pszValue);
    ppcIdxTable[iOffset] = pszValueBuf;

    /* ��������������뵽ʵ����*/
    pstInstance->ppcIdxTable = ppcIdxTable;

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
����: �����û����л�ȡ�ڵ��ֵ
����: void *pInstance,               ��ǰʵ��;
               const char *pszName,    �ڵ���;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ��ȡ���û���������*/
    ppcIdxTable = pstInstance->ppcIdxTable;
    if ( NULL == ppcIdxTable )
    {
        COMM_TRACE("\n");
        return NULL;
    }

    /* ��ȡ�ڵ���Ϣ�б�*/
    ppstNodeInfoList = pstInstance->ppstNodeInfoList;
    if ( NULL == ppstNodeInfoList )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return NULL;
    }

    /* ���ҽڵ���Ϣ*/
    pstNodeInfo = FindNodeInfo(ppstNodeInfoList, pszName);
    if ( NULL == pstNodeInfo )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return NULL;
    }

    /* Խ��*/
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
����: �ͷ�ʵ�������û���
����: void *pInstance,               ��ǰʵ��;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ���û���������*/
    ppcIdxTable = pstInstance->ppcIdxTable;
    if ( ppcIdxTable != NULL )
    {
        /* ��ȡ�ڵ����*/
        nNodeCount = pstInstance->nNodeCount;

        /* �ͷ����нڵ����û���*/
        for ( i = 0; i < nNodeCount; i++ )
        {
            safe_free(ppcIdxTable[i]);
        }

        /* �ͷ����û���������*/
        safe_free(pstInstance->ppcIdxTable);
    }

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
����: �ͷ�ģ�������ʵ�������û���
����: void *pInstanceList,               ��ǰʵ��;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int FreeAllConfBuf(void *pInstanceList)
{
    ST_Instance *pstInstanceList = NULL;
    ST_Instance *pstInstance = NULL;
    char **ppcIdxTable = NULL;
    unsigned int nNodeCount = 0;
    unsigned int i = 0;

    pstInstanceList = (ST_Instance *)pInstanceList;

    /* ��������ʵ��*/
    for ( pstInstance = pstInstanceList; pstInstance; pstInstance = pstInstance->pstNext )
    {
        /* ���û���������*/
        ppcIdxTable = pstInstance->ppcIdxTable;
        if ( ppcIdxTable != NULL )
        {
            /* ��ȡ�ڵ����*/
            nNodeCount = pstInstance->nNodeCount;

            /* �ͷ����нڵ����û���*/
            for ( i = 0; i < nNodeCount; i++ )
            {
                safe_free(ppcIdxTable[i]);
            }

            /* �ͷ����û���������*/
            safe_free(pstInstance->ppcIdxTable);
        }
    }

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
����: Ӧ������ʵ������������
����: void *pInstanceList,                               ��ǰʵ��;
             int (* pApplyFunc)(void * pInstance)   ������Ч����;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ��������ʵ��*/
    for ( pstInstance = pstInstanceList; pstInstance; pstInstance = pstInstance->pstNext )
    {
        /* �޸�������*/
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
����: ���ݽڵ����ҵ���Ӧ�� NodeInfo
����: ST_NodeInfo **ppstNodeInfoList,                              �ڵ�����;
             const char * pszName   ������Ч����;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ���ݽڵ����ĵ�һ����ĸ����hash ֵ*/
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

    /* ���ݽڵ����ڽڵ���Ϣhash ���в���*/
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
����: ���ڵ���Ϣ��ӵ��ڵ���Ϣ�б���
����: ST_NodeInfo **ppstNodeInfoList,                              �ڵ�����;
             const char * pszName                                                 ������Ч����;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ���ݽڵ����ĵ�һ����ĸ����hash ֵ*/
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

    /* ���ݽڵ����ڽڵ���Ϣhash ���в���*/
    for ( pstTempNodeInfo = ppstNodeInfoList[iHash]; pstTempNodeInfo; pstTempNodeInfo = pstTempNodeInfo->pstNext )
    {
        if ( 0 == safe_strcmp(pstTempNodeInfo->pszName, pszName) )
        {
            COMM_TRACE(INFO_ERROR"\n");
            return TBS_FAILED;
        }
    }

    /* ���ڵ���Ϣ��ӵ��ڵ���Ϣhash ����*/
    pstNodeInfo->pstNext = ppstNodeInfoList[iHash];
    ppstNodeInfoList[iHash] = pstNodeInfo;

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}


/**************************************************************************
����: ����ӽ��
����: const char *pszPath,                                                   ��ǰ·��
             const ST_TreeNode astTreeNodeList[],                      Ҷ�ӽڵ��б�;
             unsigned int nNodeCount                                             Ҷ�ӽڵ���;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

        /* �ڵ�·��*/
        sprintf(szPath, "%s.%s", pszPath, astTreeNodeList[i].pszName);

        /* ��ӽڵ�*/
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
����: ��SET �����л�ȡ�ڵ�·��
����: const char *pszCmd,                                                  �����SET�ַ���;
             char *pszPath                                                             ��������·��;
             unsigned long ulLen                                                    path������ܳ���;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int GetPathFromCmd(const char *pszCmd, char *pszPath, unsigned long ulLen)
{
    char *pcPos = NULL;
    unsigned long ulPathLen = 0;

    if ( NULL == pszCmd || NULL == pszPath )
    {
        return TBS_NULL_PTR;
    }

    /* ����'='�ŵ�λ��*/
    pcPos = strchr(pszCmd, '=');
    if ( NULL != pcPos )
    {
        ulPathLen = pcPos - pszCmd + 1;

        if ( ulLen > ulPathLen )
        {
            ulLen = ulPathLen;
        }
    }

    /* ��Path����*/
    safe_strncpy(pszPath, pszCmd, ulLen);

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
����: ��SET�����н������ڵ������ڵ�ֵ��ʵ����
����: const char * pszSetCmd                                                �����SET�ַ���;
             char * pszName                                                             ��������·��;
             char * pszValue                                                             ��������ֵ;
             unsigned long aulIdxList[]                                            ��ȡ��index�б�
             int *piIdxCount                                                              ��ȡ��index����
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ����SET �����������*/
    safe_strncpy(szSetCmdBuf, pszSetCmd, MAX_PATH_LEN);

    /* �Ⱥź�����ǽڵ�ֵ*/
    pcPos = strchr(szSetCmdBuf, '=');

    if ( NULL == pcPos )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_VAL;
    }

    /* �Ⱥ�ǰ����ǽڵ�·��*/
    *pcPos = '\0';
    pcPos++;

    /* �ڵ�ֵ*/
    if ( NULL != pszValue )
    {
        strcpy(pszValue, pcPos);
    }
    else
    {
        return TR069_ERRNO_INVALID_VAL;
    }

    /* ���ڵ�·���õ��зֳ������Ӵ�*/
    iSubCount = tbsSplitString(szSetCmdBuf, apszSubStr, '.', MAX_PATH_NODE_COUNT);
    if ( iSubCount > MAX_PATH_NODE_COUNT )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_NAME;
    }

    if ( NULL != aulIdxList && NULL != piIdxCount )
    {
        /* ʵ����������*/
        iIdxCount = *piIdxCount;

        /* �����һ���Ӵ���ʼ����*/
        for ( i = iSubCount - 1; i >= 0; i-- )
        {
            iRet = sscanf(apszSubStr[i], "%lu", &ulIdx);

            /* �Ӵ���ʵ����*/
            if ( 1 == iRet )
            {
                /* ·���е�ʵ���Ÿ���������Ҫ�ĸ���*/
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

        /* ʵ��ʵ���Ÿ���*/
        *piIdxCount -= iIdxCount;

        /* ���ʵ��ʵ���Ÿ���С��ָ�������ʵ���Ÿ���*/
        if ( iIdxCount > 0 )
        {
            /* ������anIdxList �ĵ�һ�ʼ����ʵ����*/
            memcpy(aulIdxList, aulIdxList + iIdxCount, (*piIdxCount) * sizeof(unsigned long));
            memset(aulIdxList + *piIdxCount, 0, iIdxCount * sizeof(unsigned long));
        }
    }

    /* ���һ���Ӵ��ǽڵ���*/
    if ( iSubCount > 0 && NULL != pszName )
    {
        strcpy(pszName, apszSubStr[iSubCount - 1]);
    }

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}


/**************************************************************************
����: ��SET�����н������ڵ������ڵ�ֵ��ʵ����
����: const char * pszSetCmd                                                �����SET�ַ���;
             unsigned int nNameLen                                                 �����Ľڵ�������;
             unsigned int nValueLen                                                 ��������ֵ����;
             const char *pszFmt                                                       ��ʽ�б�
             ...                                                                                    �������Ľṹ
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ���·���Ƿ���ȷ����������ʵ����*/
    va_start(argp, pszFmt);
    for ( ; *pszFmt && *pszSetCmd; pszFmt++, pszSetCmd++ )
    {
        /* �ж���ʵ���Ż���·���ַ���*/
        if ( '%' == *pszFmt )
        {
            /* ����ʵ�����ַ���*/
            pulIdx = va_arg(argp, unsigned long *);
            for ( pcPos = pszSetCmd; *pszSetCmd; pszSetCmd++ )
            {
                if ( *pszSetCmd < '0' || *pszSetCmd > '9' )
                {
                    break;
                }
            }

            /* ���ʵ�����ַ����Ƿ񳬳�*/
            nLen = pszSetCmd - pcPos;
            if ( 0 == nLen || nLen > MAX_ULONG_LEN - 1 )
            {
                COMM_TRACE(INFO_ERROR"\n");
                return TR069_ERRNO_INVALID_NAME;
            }

            /* ����ʵ����*/
            safe_strncpy(szIdx, pcPos, nLen + 1);
            sscanf(szIdx, "%lu", pulIdx);

            /* �������´���*/
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

    /* ·���Ƿ���ȫƥ��*/
    if ( '\0' != *pszFmt )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_NAME;
    }

    /* ����ڵ����ͽڵ�ֵ�Ļ�����������*/
    if ( NULL == pszName || NULL == pszValue )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    /* ���ҵȺ�λ��*/
    pcPos = strchr(pszSetCmd, '=');
    if ( NULL == pcPos )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_VAL;
    }

    /* �Ⱥ�ǰ����ǽڵ���*/
    nLen = pcPos - pszSetCmd + 1;
    if ( nLen > nNameLen )
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TR069_ERRNO_INVALID_NAME;
    }

    safe_strncpy(pszName, pszSetCmd, nLen);

    /* �Ⱥź�����ǽڵ�ֵ*/
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
����: ����ַ������ַ���������
����: ST_Str ** ppstStrList                                             �ַ�������;
             char *pszStr                                                           ��ӵ��ַ���;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
static int AddStr(ST_Str ** ppstStrList, char *pszStr)
{
    ST_Str *pstStr = NULL;

    if (NULL == ppstStrList || NULL == pszStr)
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_NULL_PTR;
    }

    /* �����ṹ*/
    pstStr = (ST_ErrInfo *)malloc(sizeof(ST_ErrInfo));

    /* ����ռ�ʧ��*/
    if (NULL == pstStr)
    {
        COMM_TRACE(INFO_ERROR"Memery allocate failed\n");
        return TBS_OUT_OF_MEM;
    }

    pstStr->pszStr = pszStr;

    /* �������ײ����´�����Ϣ*/
    pstStr->pstNext = *ppstStrList;
    *ppstStrList = pstStr;

    COMM_TRACE(INFO_SUCCESS"\n");
    return TBS_SUCCESS;
}

/**************************************************************************
����: ��Ӵ�����Ϣ��������Ϣ������
����: ST_Str ** ppstStrList                                             �ַ�������;
             int iErrCode,                                                           ������
             char *pszPath                                                           ��ӵ��ַ���;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* Ϊ������������ռ�*/
    pszErrDescBuf = (char *)malloc(iLen);

    /* ����ռ�ʧ��*/
    if (NULL == pszErrDescBuf)
    {
        COMM_TRACE(INFO_ERROR"Memery allocate failed\n");
        return TBS_OUT_OF_MEM;
    }

    /* ���ɴ���������Ϣ*/
    sprintf(pszErrDescBuf, "%s=%04u%04u", pszPath,
                 TBS_ERR_INTERNAL(iErrCode), TBS_ERR_STAND(iErrCode));

    COMM_TRACE("pszErrDescBuf = %s\n", pszErrDescBuf);
    return AddStr(ppstErrInfoList, pszErrDescBuf);
}

/**************************************************************************
����: ��ӽ����Ϣ�������Ϣ������
����: ST_ResultInfo **ppstResultInfoList                                           �����Ϣ����;
             const char *pszPath                                                                    ·��
             const char *pszType                                                                   ����;
             const char *pszValue                                                                  ֵ
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* Ϊ������������ռ�*/
    pszResultBuf = (char *)malloc(iLen);

    /* ����ռ�ʧ��*/
    if (NULL == pszResultBuf)
    {
        COMM_TRACE(INFO_ERROR"Memery allocate failed\n");
        return TBS_OUT_OF_MEM;
    }

    /* ���ɴ���������Ϣ*/
    sprintf(pszResultBuf, "%s %s=%s", pszType, pszPath, pszValue);

    COMM_TRACE("pszResultBuf = %s\n", pszResultBuf);
    return AddStr(ppstResultInfoList, pszResultBuf);
}

/**************************************************************************
����: �ͷ��ַ�������
����: ST_Str **ppstStrList                                           �ַ�������;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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
����: �ظ�������Ϣ
����: const ST_MSG *pstOrgMsg                                           ԭ��Ϣ;
             unsigned long ulErrNo                                                 ������;
             ST_ErrInfo *pstErrInfoList                                          ������Ϣ�б�;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ����͸���*/
    ulMsgBodyLen = sizeof(unsigned long) * 3;

    /* ������Ϣ����*/
    for ( pstErrInfo = pstErrInfoList; pstErrInfo; pstErrInfo = pstErrInfo->pstNext )
    {
        if ( NULL != pstErrInfo->pszStr )
        {
            ulMsgBodyLen += strlen(pstErrInfo->pszStr) + 1;
            ulCount++;
        }
    }

    /* ������Ӧ��Ϣ*/
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"Create response message failed \n");
        return TBS_OUT_OF_MEM;
    }

    /* �����Ϣͷ��*/
    pstRespMsg->stMsgHead.usSrcMID = pstOrgMsg->stMsgHead.usDstMID;
    pstRespMsg->stMsgHead.usDstMID = pstOrgMsg->stMsgHead.usSrcMID;
    pstRespMsg->stMsgHead.ulMsgID = pstOrgMsg->stMsgHead.ulMsgID;

    /* ��Ϣ����*/
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

    /* TR069��׼����Ĵ�����*/
    SET_ULONG(pcPos, ulErrNo);

    /* �ڲ�������*/
    SET_ULONG(pcPos, ulErrNo);

    /* ���������*/
    SET_ULONG(pcPos, ulCount);

    /* ������*/
    for ( pstErrInfo = pstErrInfoList; pstErrInfo; pstErrInfo = pstErrInfo->pstNext )
    {
        if ( NULL != pstErrInfo->pszStr )
        {
            COMM_TRACE("pszStr = %s\n", pstErrInfo->pszStr);
            SET_STR(pcPos, pstErrInfo->pszStr);
        }
    }

    /* ������Ӧ��Ϣ */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}

/* ��Ӧ���ʵ����Ϣ */
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

    /* �����Ӧ��Ϣ���ڴ� */
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_OUT_OF_MEM;
    }

    /* ������Ӧ��Ϣ */
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

    /* ��Ϣ�峤�� */
    pstRespMsg->stMsgHead.ulBodyLength = ulMsgBodyLen;

    pcPos = pstRespMsg->szMsgBody;

    /* ��ӽ�� */
    SET_ULONG(pcPos, 0);

    /* ��ӵ�Index */
    SET_ULONG(pcPos, ulObjNo);

    /* ʵ��״̬ */
    SET_ULONG(pcPos, ulStatus);

    /* ������Ӧ��Ϣ */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}

/**************************************************************************
����: ��Ӧɾ��ʵ����Ϣ
����: const ST_MSG *pstOrgMsg                                           ԭ��Ϣ;
             unsigned long ulStatus                                                 ״̬;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* �����Ӧ��Ϣ���ڴ� */
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_OUT_OF_MEM;
    }

    /* ������Ӧ��Ϣ */
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

    /* ��Ϣ�峤�� */
    pstRespMsg->stMsgHead.ulBodyLength = ulMsgBodyLen;

    pcPos = pstRespMsg->szMsgBody;

    /* ɾ����� */
    SET_ULONG(pcPos, 0);

    /* ɾ��״̬ */
    SET_ULONG(pcPos, ulStatus);

    /* ������Ӧ��Ϣ */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}


/**************************************************************************
����: ��Ӧ������Ϣ
����: const ST_MSG *pstOrgMsg                                           ԭ��Ϣ;
             unsigned long ulStatus                                                 ״̬;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ������Ӧ��Ϣ*/
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"Create response message failed\n");
        return TBS_OUT_OF_MEM;
    }

    /* �����Ϣͷ��*/
    pstRespMsg->stMsgHead.usSrcMID = pstOrgMsg->stMsgHead.usDstMID;
    pstRespMsg->stMsgHead.usDstMID = pstOrgMsg->stMsgHead.usSrcMID;
    pstRespMsg->stMsgHead.ulMsgID = pstOrgMsg->stMsgHead.ulMsgID;
    pstRespMsg->stMsgHead.usMsgType = MSG_CMM_SET_VAL_ACK;

    /* ������Ϣ�峤��*/
    pstRespMsg->stMsgHead.ulBodyLength = ulMsgBodyLen;

    pcPos = pstRespMsg->szMsgBody;

    /* ���Set���*/
    SET_ULONG(pcPos, 0);

    /* ���״̬*/
    SET_ULONG(pcPos, ulStatus);

    /* ������Ӧ��Ϣ */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}

/**************************************************************************
����: ��Ӧ��ѯ��Ϣ
����: const ST_MSG *pstOrgMsg                                           ԭ��Ϣ;
             unsigned long ulStatus                                                 ״̬;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ����͸���*/
    ulMsgBodyLen = sizeof(unsigned long) * 2;

    /* ������Ϣ����*/
    for ( pstResultInfo = pstResultInfoList; pstResultInfo; pstResultInfo = pstResultInfo->pstNext )
    {
        if ( NULL != pstResultInfo->pszStr )
        {
            ulMsgBodyLen += strlen(pstResultInfo->pszStr) + 1;
            ulCount++;
        }
    }

    /* ������Ӧ��Ϣ*/
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"Create response message failed \n");
        return TBS_OUT_OF_MEM;
    }

    /* �����Ϣͷ��*/
    pstRespMsg->stMsgHead.usSrcMID     = pstOrgMsg->stMsgHead.usDstMID;
    pstRespMsg->stMsgHead.usDstMID     = pstOrgMsg->stMsgHead.usSrcMID;
    pstRespMsg->stMsgHead.ulMsgID      = pstOrgMsg->stMsgHead.ulMsgID;
    pstRespMsg->stMsgHead.usMsgType    = MSG_CMM_GET_VAL_ACK;

    pstRespMsg->stMsgHead.ulBodyLength = ulMsgBodyLen;

    pcPos = pstRespMsg->szMsgBody;

    /* ���Get���*/
    SET_ULONG(pcPos, 0);

    /* �������*/
    SET_ULONG(pcPos, ulCount);

    /* �����*/
    for ( pstResultInfo = pstResultInfoList; pstResultInfo; pstResultInfo = pstResultInfo->pstNext )
    {
        if ( NULL != pstResultInfo->pszStr )
        {
            SET_STR(pcPos, pstResultInfo->pszStr);
        }
    }

    /* ������Ӧ��Ϣ */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}

/**************************************************************************
����: ��Ӧ���ø�����Ϣ
����: const ST_MSG *pstOrgMsg                                           ԭ��Ϣ;
             unsigned long ulStatus                                                 ���;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* �����Ӧ��Ϣ���ڴ� */
    pstRespMsg = (ST_MSG*)MSG_CreateMessage(sizeof(ST_MSGHEAD) + ulMsgBodyLen);
    if (NULL == pstRespMsg)
    {
        COMM_TRACE(INFO_ERROR"\n");
        return TBS_OUT_OF_MEM;
    }

    /* ������Ӧ��Ϣ */
    pstRespMsg->stMsgHead.ulMsgID = pstOrgMsg->stMsgHead.ulMsgID;
    pstRespMsg->stMsgHead.usSrcMID = pstOrgMsg->stMsgHead.usDstMID;
    pstRespMsg->stMsgHead.usDstMID = pstOrgMsg->stMsgHead.usSrcMID;
    pstRespMsg->stMsgHead.usMsgType = MSG_CMM_UPDATE_ACK;

    /* ��Ϣ�峤�� */
    pstRespMsg->stMsgHead.ulBodyLength = ulMsgBodyLen;

    pcPos = pstRespMsg->szMsgBody;

    /* ���½�� */
    SET_ULONG(pcPos, ulResult);

    /* ������Ӧ��Ϣ */
    iRet = MSG_SendMessage(pstRespMsg);
    safe_free_msg(pstRespMsg);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}


/**************************************************************************
����: ͨ�û�ȡ����ֵ��Ϣ������
����: ST_MSG *pstMsg                                          ԭ��Ϣ;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
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

    /* ��ȡ��ѯ����Ŀ*/
    GET_ULONG(pcPos, ulCount);

    for (i = 0; i < ulCount; i++)
    {
        /* ��ȡ���û�ȡ�ڵ�ֵ������*/
        ulTypeLen = MAX_NODE_TYPE_LEN;
        ulValueLen = MAX_NODE_VALUE_LEN;
        iRet = CFG_GetLeafValAndType(pcPos, szNodeValue, &ulValueLen, szNodeType, &ulTypeLen);
        if ( RET_SUCCEED(iRet) )
        {
            /* �����ѯ�����������*/
            AddResultInfo(&pstResultList, pcPos, szNodeType, szNodeValue);
        }
        else
        {
            AddErrInfo(&pstErrList, iRet, pcPos);
            COMM_TRACE(INFO_ERROR"\n");
            break;
        }

        /* ��ȡһ����ѯ����*/
        pcPos += strlen(pcPos) + 1;
    }

    if ( RET_SUCCEED(iRet) )
    {
        /* ��Ӧ��ѯ��Ϣ*/
        iRet = RespGetMsg(pstMsg, pstResultList);
    }
    else
    {
        /* ���ʹ�����Ϣ */
        iRet = RespErrMsg(pstMsg, iRet, pstErrList);
    }

    /* �ͷ��ַ�������*/
    FreeAllResultInfo(&pstResultList);

    /* �ͷŴ�����Ϣ���� */
    FreeAllErrInfo(&pstErrList);

    COMM_TRACE(INFO_SUCCESS"\n");
    return iRet;
}


/*���ý������ȼ�*/
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
Description:   ���ý������ȼ�
Input:         char *pName, char *pFile, char *pValue
Return:        ��
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
Description:   �ж�kenel module�Ƿ��Ѿ�����
Input:         const char *pszName      ģ������
Return:        ��
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


