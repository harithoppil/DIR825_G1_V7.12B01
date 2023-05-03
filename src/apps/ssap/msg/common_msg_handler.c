/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ�����: Common_fun.c
 �ļ�����: ������Ϣ����ķ�װ���򻯸�ģ��Ĵ�����

 �޶���¼:
        1. ����: ��ΰ
           ����: 2008-08-07
           ����: �����ļ�

**********************************************************************/

#include <assert.h>
#include "cache_list.h"
#include "common.h"
#include "common_func.h"
#include "common_msg_handler.h"
#include "new_msg.h"


extern ST_MODULE_NODE g_Module_Table[];

/*************************************************************************
����:   ����set�����·����ֵ
����:   const char *pszSetCmd ,         set�����ַ���
               char *pszPath ,                        ·��
               unsigned int *pnPathLen ,     ·������
               char *pszValue ,                    ֵ
               unsigned int *pnValueLen ,   ֵ����
����:   TBS_SUCCESS ,    �ɹ�
               ����,                ʧ��
��ע:
*************************************************************************/
int ParseSetCmdSimple(const char *pszSetCmd,char *pszPath,unsigned int *pnPathLen,char *pszValue,unsigned int *pnValueLen)
{
    const char *pcPos = NULL;
    unsigned int nLen = 0;

    /* ����ڵ����ͽڵ�ֵ�Ļ�����������*/
    if ( NULL == pszPath || NULL == pszValue||NULL== pszSetCmd||
            NULL==pnPathLen||NULL==pnValueLen)
    {
        COMMON_MSG_TRACE("null pointer,when parse set cmd\n");
        return TBS_NULL_PTR;
    }

    /* ���ҵȺ�λ��*/
    pcPos = strchr(pszSetCmd, '=');
    if ( NULL == pcPos )
    {
        COMMON_MSG_TRACE("can't find =\n");
        return TR069_ERRNO_INVALID_VAL;
    }

    /* �Ⱥ�ǰ����ǽڵ���*/
    nLen = pcPos - pszSetCmd + 1;
    if ( nLen > *pnPathLen )
    {
        COMMON_MSG_TRACE("can't get node name\n");
        return TR069_ERRNO_INVALID_NAME;
    }

    safe_strncpy(pszPath, pszSetCmd, nLen);
    *pnPathLen = nLen;

    /* �Ⱥź�����ǽڵ�ֵ*/
    pcPos++;
    nLen = strlen(pcPos) + 1;
    if ( nLen > *pnValueLen )
    {
        COMMON_MSG_TRACE("can't get node value\n");
        return TR069_ERRNO_INVALID_VAL;
    }

    safe_strncpy(pszValue, pcPos, nLen);
    *pnValueLen = nLen;

    return TBS_SUCCESS;
}


/*
  *    ������������غ�������
  */

void InitCache(struct list_head * pCacheList)
{
    CacheList_Init(pCacheList);
}

void AddCacheNode(struct list_head * pCacheList,char *pszPath,char *pszValue)
{
    CacheList_AddTail(pCacheList,pszPath,pszValue);
}

void SetCacheNode(struct list_head * pCacheList,char *pszPath,char *pszValue)
{
    CacheList_SetEntry(pCacheList,pszPath,pszValue);
}
void DeleteCacheNode(struct list_head * pCacheList,char*pszPath)
{
    CacheList_DeleteEntry(pCacheList,pszPath);
}
char * GetCacheNode(struct list_head * pCacheList,char *pszPath)
{
    return CacheList_GetValue(pCacheList, pszPath);
}
void CleanCache(struct list_head * pCacheList)
{
    CacheList_DropAll(pCacheList);
}

void SetCleanFlag(struct list_head * pCacheList,char *pszPath)
{
    CacheList_SetCleanFlag(pCacheList,pszPath);
}

void CleanTagedNodes(struct list_head * pCacheList)
{
    CacheList_DropAllWithCleanFlag(pCacheList);
}

void DumpCache(struct list_head * pCacheList)
{
    CacheList_PrintAll(pCacheList);
}

char * GetCacheNodeEx(char *pszPath)
{
    int i = 0;
    char *pValue = NULL;

    for (i=0;i< MAX_MODULE_NUM;i++)
    {
        if (g_Module_Table[i].pHashModuleFuncTable)
        {
            pValue = GetCacheNode(&(g_Module_Table[i].listCache),pszPath);
            if (pValue)
            {
                break;
            }
        }
    }

    return pValue;
}


/*
  *     �Ѵ���������������ݽṹ��������װ
  */

/* ������¼��Ч���ĺ�����ȷ��ͬһ��ʵ������£������ε�ͬһ����*/
typedef struct tag_ST_FUNCTION_NODE
{
    void *pFunction;                                                 /*������*/
    int iPathIndex[MAX_INDEX_NUM];                      /*index����,�����ʵ�����������Ҫ*/
    struct tag_ST_FUNCTION_NODE *pNext;             /*ָ��*/
}ST_FUNCTION_NODE;

/* ����ͷβָ��*/
ST_FUNCTION_NODE *pFuncHead = NULL,*pFuncTail = NULL;

BOOL AddFuncPointer(void *pFuncPointer,int iIndex[MAX_INDEX_NUM])
{
    ST_FUNCTION_NODE *pTempFunc =  (ST_FUNCTION_NODE*)malloc(sizeof(ST_FUNCTION_NODE));
    if (!pTempFunc)
    {
        COMMON_MSG_TRACE("malloc failed\n");
        return false;
    }

    bzero(pTempFunc, sizeof(ST_FUNCTION_NODE));

    pTempFunc->pFunction = pFuncPointer;
    memcpy(pTempFunc->iPathIndex,iIndex,sizeof(int)*MAX_INDEX_NUM);

    pTempFunc->pNext = NULL;

    if (pFuncTail)
    {
        pFuncTail->pNext = pTempFunc;
    }
    pFuncTail = pTempFunc;

    if (!pFuncHead)
    {
        pFuncHead = pTempFunc;
    }

    return true;
}

BOOL FunPointerIsInList(void * pFuncPointer,int iIndex[MAX_INDEX_NUM])
{
    int i=0;

    ST_FUNCTION_NODE *pTempFunc = pFuncHead;
    for (;pTempFunc;pTempFunc = pTempFunc->pNext)
    {
        if (pFuncPointer == pTempFunc->pFunction)
        {
            for (i=0;i<MAX_INDEX_NUM;i++)
            {
                if (iIndex[i] != pTempFunc->iPathIndex[i])
                    break;
            }

            if (MAX_INDEX_NUM == i)
                return true;
        }
    }

    return false;
}

void CleanFunPointerList()
{
    ST_FUNCTION_NODE *pTempFunc = pFuncHead;
    ST_FUNCTION_NODE *pTempNext = NULL;
    while (pTempFunc)
    {
        pTempNext = pTempFunc->pNext;
        free(pTempFunc);
        pTempFunc = pTempNext;
    }

    pFuncHead=NULL;
    pFuncTail=NULL;
}

/*
  *     ���ڳ�ͻ����У���������ͻ�Ľڵ��·�����浽ȫ��������
  */
char g_szErrPath[MAX_PATH_LEN] = {0};  /*��¼����·��*/
extern BOOL g_bIsModuleDefined;              /*ģ���Ƿ��ѳ�ʼ��*/

static inline void SetLastErrPath(char *pszPath)
{
    if (g_bIsModuleDefined == TRUE)
    {
        g_bIsModuleDefined = FALSE;
    }
    else
    {
        safe_strncpy(g_szErrPath, pszPath, sizeof(g_szErrPath));
    }

    return;
}

/***************************************************************
  Function:   const char *COMM_GetLastErrPath()
  Description: ���ڳ�ͻ���ʧ�ܺ��ȡ������ͻ�Ľڵ�·��
***************************************************************/
static inline const char *GetLastErrPath(void)
{
    return g_szErrPath;
}

/*
  *  ��Ϣ������غ���
  */

/**************************************************************************
����: ��ô���ʹ�õĹ��Ӻ���
����: char *pcNodePath,                 ��ǰ·��;
               HookType eHookType,           ���Ӻ�������;
               ST_MSG_DATA *pstMsgData   ��ǰ�������Ϣ;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
void *GetHookFunc(char *pcNodePath,HookType eHookType,ST_MSG_DATA *pstMsgData)
{
    ST_HookEntry * pHookEntry = NULL;
    hashtable *pHash = NULL;
    ST_PATH_KEY PathKey;
    ST_PATH_DATA *pPathData;

    /*��ϢΪ�� ���򷵻ؿպ���*/
    if (!pstMsgData)
    {
        COMMON_MSG_TRACE("HOOK TABLE POINTER IS NULL");
        return NULL;
    }

    COMMON_MSG_TRACE("Input Path = %s\n",pcNodePath);


    /*�Ǽ��ٵ�ģ��ʹ�õ���ȫƥ��ķ�ʽ*/
    if (pstMsgData->stTableType == ARRAY_TABLE)
    {
        pHookEntry = pstMsgData->pstHookTable;
        /*�������ӱ����Ҵ�����*/
        while (pHookEntry->pNodePath)
        {
            if ((pHookEntry->eHookType == eHookType)&&(tbsMatch(pcNodePath,pHookEntry->pNodePath)))
            {
                COMMON_MSG_TRACE("Match Path = %s \n", pHookEntry->pNodePath);
                return pHookEntry->pHookFunc;
            }
            else
                pHookEntry++;
        }
    }
    /*����ٵ�ģ��ʹ�õ���hash�ķ�ʽ*/
    else
    {
        pHash = pstMsgData->pHashModuleFuncTable;

        PathKey.pszPath = pcNodePath;
        PathKey.eHookType = eHookType;

        /*����hash��*/
        pPathData = FuncHash_GetValue(&PathKey,pHash);
        if (pPathData)
            return pPathData->pHookFunc;
    }

    COMMON_MSG_TRACE("No Match Path.\n");

    return NULL;
}


typedef int (*COMMON_PROCESS)(ST_MSG *,ST_MSG_DATA *);


/**************************************************************************
����: ������Ϣ���ö�Ӧ�Ĵ�����
����: ST_MSG_DATA *pstMsgData,                ��Ϣ��������;
             ST_MSG *pstMsg,                                  ��ǰ�������Ϣ;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int Call_Msg_ProcFunc(ST_MSG_DATA *pstMsgData,ST_MSG *pstMsg)
{

    MSG_PROCESS_FUNC pfnProcess = NULL;
//    ST_HookEntry *pstHookTable = NULL;
//    hashtable *pstHash = NULL;

    /*������Զ�����Ϣ��������������Զ��庯��*/
    if (NULL == pstMsgData)
    {
        pfnProcess = NULL;
    }
    else
    {
        pfnProcess = pstMsgData->pfCustomProcess;
    }


    /*��ͬ��Ϣ����Ӧ�Ĺ���������*/
    struct ST_MSG_COMMFUNC_MAP {
        unsigned short usMsgType;
        COMMON_PROCESS pfnCommProcess;
    }
    stMsgCommFuncMap[] =
    {
        {MSG_CMM_SET_VAL,CommonSetMsgHandler},
        {MSG_CMM_COMMIT,CommonCommitMsgHandler},
        {MSG_CMM_CANCEL,CommonCancelMsgHandler},
        {MSG_CMM_DEL_NODE,CommonDeleteMsgHandler},
        {MSG_CMM_INST_DELED,CommonDeletedMsgHandler},
        {MSG_CMM_ADD_NODE,CommonAddMsgHandler},
        {MSG_CMM_INST_ADDED,CommonAddedMsgHandler},
        {MSG_CMM_UPDATE,CommonUpdateMsgHandler},
    };
    unsigned char ucMapNum = sizeof(stMsgCommFuncMap)/sizeof(struct ST_MSG_COMMFUNC_MAP);

    int i=0;
    unsigned short usMsgType = 0;
    unsigned short usDstMid = 0;

    if (!pstMsg)
    {
        COMMON_LOG_ERROR(0,"msg is null!\n");
        return false;
    }

    usDstMid = pstMsg->stMsgHead.usDstMID;
    usMsgType = pstMsg->stMsgHead.usMsgType;

//    COMM_PrintCurTime();

    /*�Զ��崦�����ĵ���*/
    if (pfnProcess)
    {
        COMMON_MSG_TRACE("Call Custom MsgHandler, SrcMID:%04x,DstMID:%04x, MsgID:0x%08lx, MsgType:%04x\n",
                         pstMsg->stMsgHead.usSrcMID,
                         pstMsg->stMsgHead.usDstMID,
                         pstMsg->stMsgHead.ulMsgID,
                         pstMsg->stMsgHead.usMsgType);

        pfnProcess(pstMsg);

        /* Show Memory Status */
        if ( usMsgType != MSG_CMM_UPDATE )
        {
            /* Mem_PrintStatusOnce() */ ;
        }

        return true;
    }

    /* now ,all cmm message give default process */
#if 0
    if (!pstHookTable && ((usMsgType != MSG_CMM_UPDATE) &&(usMsgType != MSG_CMM_CANCEL)))
    {
        COMMON_MSG_TRACE("mid %x -- msg %x is not registered!\n",usDstMid,usMsgType);
        return false;
    }
#endif

    /*���ݶ�Ӧ����Ϣ���ͣ����ù���������*/
    for (i=0;i<ucMapNum;i++)
    {
        if (usMsgType == stMsgCommFuncMap[i].usMsgType)
        {
            /* don't show update msg */
            if ( usMsgType != MSG_CMM_UPDATE )
            {
                COMMON_MSG_TRACE("Call Common MsgHandler, SrcMID:%04x,DstMID:%04x, MsgID:0x%08lx, MsgType:%04x\n",
                                 pstMsg->stMsgHead.usSrcMID,
                                 pstMsg->stMsgHead.usDstMID,
                                 pstMsg->stMsgHead.ulMsgID,
                                 pstMsg->stMsgHead.usMsgType);
            }

            stMsgCommFuncMap[i].pfnCommProcess(pstMsg,pstMsgData);

            /* Show Memory Status */
            if ( usMsgType != MSG_CMM_UPDATE )
            {
                /* Mem_PrintStatusOnce() */;
            }

            return true;
        }
    }
#ifdef COMMON_MSG_DEBUG_SWITCH
    COMMON_LOG_ERROR(0, "Not Found MsgHandler, SrcMID:%04x,DstMID:%04x, MsgID:0x%08lx, MsgType:%04x\n",
                     pstMsg->stMsgHead.usSrcMID,
                     pstMsg->stMsgHead.usDstMID,
                     pstMsg->stMsgHead.ulMsgID,
                     pstMsg->stMsgHead.usMsgType);
#endif
    return false;
}

/**************************************************************************
����: ��ͻ���ı�������
����: unsigned short usMID,             ��ǰMID;
             char *pszPath,                         ��ǰ�����·��;
             char *pszValue,                        ��ǰ����Ľڵ�ֵ;
             ST_MSG_DATA *pstMsgData    ��Ϣ��������;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int CheckConflictAccessor(unsigned short usMID,char *pszPath,char *pszValue,ST_MSG_DATA *pstMsgData)
{
    int iRet = TBS_SUCCESS;
    int iIndex[MAX_INDEX_NUM]={0};

    CheckConflictFunc pfCheck = (CheckConflictFunc)GetHookFunc(pszPath,CHECK_CONFLICT_FUNC,pstMsgData);

    if (pfCheck)
    {
        COMM_GetPathIndex(pszPath,iIndex);

        /*����Ѿ�������������*/
        if (FunPointerIsInList(pfCheck,iIndex))
            return TBS_SUCCESS;


        /*����ֱ�ӵ��ô�����*/
        iRet = pfCheck(pszPath,pszValue);
        if (RET_FAILED(iRet))
        {
            SetLastErrPath(pszPath);
            return iRet;
        }
        else
        {
            if (!AddFuncPointer((void *)pfCheck,iIndex))
                COMMON_MSG_TRACE("FUNCTION LIST IS FULL\n");
        }
    }

    return iRet;
}

/**************************************************************************
����: �����Ļ������������ÿ��ʵ��ֻ����һ��
����: unsigned short usMID,                            ��ǰMID;
             struct list_head *head,                           ����ͷ;
             ST_MSG_DATA *pstMsgData                    ��Ϣ������;
             CheckConflictAccessorFunc pAccessor  �����ĺ�����
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int CacheList_ForEachOnce(unsigned short usMID,struct list_head *head,ST_MSG_DATA *pstMsgData, CheckConflictAccessorFunc pAccessor)
{
    ST_Cache_Entry *l = NULL;
    struct list_head *ptr;
    int iRet = TBS_SUCCESS;

    CleanFunPointerList();

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Cache_Entry, head);

        if (!l)
        {
            return ERR_INNER_CACHE_INVALID;
        }
        else
        {
            COMMON_MSG_TRACE("current path = %s,value = %s\n",l->pszPath,l->pszValue);

            iRet = pAccessor(usMID,l->pszPath,l->pszValue,pstMsgData);
            if (RET_FAILED(iRet) )
            {
                return iRet;
            }
        }
    }

    CleanFunPointerList();

    return TBS_SUCCESS;
}


#if 0
/*�������棬��Ӧ�����ɵ����*/
int CacheList_ForEach(struct list_head *head,ST_HookEntry * pstSetHookTable, CheckConflictAccessorFunc pAccessor)
{
    ST_Cache_Entry *l = NULL;
    struct list_head *ptr;
    int iRet = TBS_SUCCESS;

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Cache_Entry, head);

        if (!l)
        {
            return ERR_INNER_CACHE_INVALID;
        }
        else
        {
            COMMON_MSG_TRACE("current path = %s,value = %s\n",l->pszPath,l->pszValue);
            if (l->cCleanFlag == VALID)
            {
                iRet = pAccessor(l->pszPath,l->pszValue,pstSetHookTable);
                if (RET_FAILED(iRet) )
                {
                    return iRet;
                }
            }
        }
    }

    return TBS_SUCCESS;
}
#endif

static ST_MSG *g_pstCurMsg = NULL;

/* ��Set��Ϣ�ı����ֵ���Ƿ���Ҫ����������һ�β�������Ч */
static int g_iNeedReboot = 0;
void CommonSetNeedReboot(void)
{
  g_iNeedReboot = 1;
}

ST_MSG * COMM_GetCurMsg(void)
{
    return g_pstCurMsg;
}

/**************************************************************************
����: ����set��Ϣ������
����: ST_MSG *pstMsg,                                    ��Ϣ��;
             ST_MSG_DATA *pstMsgData                   ��Ϣ��������
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int CommonSetMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData)
{
    char *pcPos = NULL;
    const char *pOldValue = NULL;
    const char *pcCache = NULL;

    unsigned long ulCount = 0;

    int i=0;
    int iRet = TBS_SUCCESS;

    char szPath[MAX_PATH_LEN] = {0};
    char *pszValue = NULL;
    unsigned int uPathLen = MAX_PATH_LEN;
    unsigned int uValueLen = pstMsg->stMsgHead.ulBodyLength;

    CheckFunc pfCheck = NULL;

    /* Ĭ������²���Ҫ������Ч */
    g_iNeedReboot = 0;

    ST_ErrInfo *pstErrInfoList = NULL;

    /*pstMsg not null*/
    if ( NULL == pstMsg )
    {
        COMMON_MSG_TRACE("set msg pointer is NULL!\n");
        return TBS_FAILED;
    }

    unsigned short usMid = pstMsg->stMsgHead.usDstMID;
    struct list_head *pList = MSG_MSG_GetModuleCache(usMid);

    g_pstCurMsg = pstMsg;

    if (NULL == pstMsgData)
    {
        COMMON_MSG_TRACE("not suppport set message!\n");
        iRet = ERR_NOT_SUPPORT_FUNCTION;
        goto SET_RESULT;
    }

    pszValue = (char *)malloc(uValueLen);
    if(NULL == pszValue)
    {
        COMMON_MSG_TRACE("malloc failed!\n");
        iRet = ERR_MALLOC_FAILED;
        goto SET_RESULT;
    }

    COMMON_MSG_TRACE("===========CHECK_FUNC==============\n");

    /*msg decode and process set node */
    pcPos = pstMsg->szMsgBody;

    GET_ULONG(pcPos, ulCount);
    for ( i = 0; i < ulCount; i++ )
    {
        /* get path and value */
        uPathLen = MAX_PATH_LEN;
        uValueLen = pstMsg->stMsgHead.ulBodyLength;
        szPath[0] = '\0';

        /*���ڶ��ѭ��,����ÿ��ѭ��Ҫ���ڴ�����*/
        memset(pszValue, 0, uValueLen);

        iRet = ParseSetCmdSimple(pcPos, szPath, &uPathLen, pszValue, &uValueLen);
        if ( RET_FAILED(iRet) )
        {
            /*return right path*/
            if (strlen(szPath) == 0)
                safe_strncpy(szPath,pcPos,MAX_PATH_LEN);

            COMMON_MSG_TRACE("parse set cmd %s is error!\n", pcPos);
            goto SET_RESULT;
        }

        /* Check path format: Suffix is no point  */
        if ( szPath[uPathLen-1] == '.' )
        {
            iRet = TR069_ERRNO_INVALID_NAME;
            COMMON_MSG_TRACE("Path invalid! Path:%s,Value:%s\n",szPath,pszValue);
            goto SET_RESULT;
        }

        /* get old node value, if not exists, goto error */
        iRet = CFG_GetNodeValPtr(szPath, &pOldValue, NULL);
        if ( RET_FAILED(iRet))
        {
            goto  SET_RESULT;
        }

        /* check each node in msg */
        pfCheck = (CheckFunc)GetHookFunc(szPath,CHECK_FUNC,pstMsgData);
        if (pfCheck)
        {
            iRet= pfCheck(pszValue);
            if (RET_FAILED(iRet))
            {
                COMMON_MSG_TRACE("check set value error! Path:%s,Value:%s\n",szPath,pszValue);
                goto SET_RESULT;
            }
        }
        else
        {
            iRet = CFG_CheckNodeVal(szPath, pszValue);
            if (RET_FAILED(iRet))
            {
                COMMON_MSG_TRACE("check set value error! Path:%s,Value:%s\n",szPath,pszValue);
                goto SET_RESULT;
            }
        }

		iRet = CFG_GetNodeAttrValPtr(szPath, ATTR_CACHE, &pcCache);
        if ( RET_FAILED(iRet))
        {
            goto  SET_RESULT;
        }

        if (NULL != pcCache && '1' == pcCache[0])
        {
            AddCacheNode(pList,szPath,pszValue);
        }
        else
        {
            /* get old node value, if not exists, goto error */
            iRet = CFG_GetNodeValPtr(szPath, &pOldValue, NULL);
            if ( RET_FAILED(iRet))
            {
                goto  SET_RESULT;
            }

            /* �Ƚϣ��������ֵ��������뵽���� */
            if (strcmp(pOldValue,pszValue))
            {
                AddCacheNode(pList,szPath,pszValue);
            }
            else
            {
                pcPos += strlen(pcPos) + 1;
                continue;
            }
        }

        /* ׼��������һ���ڵ� */
        pcPos += strlen(pcPos) + 1;
    }


    /*check conflict,use accessor pattern  to ignore two  cache diffirence*/

    COMMON_MSG_TRACE("===========CHECK_CONFLICT==============\n");
    iRet = CacheList_ForEachOnce(pstMsg->stMsgHead.usDstMID,pList,pstMsgData,CheckConflictAccessor);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Value is conflict,please check it\n");
        safe_strncpy(szPath, GetLastErrPath(), sizeof(szPath));
        goto SET_RESULT;
    }

SET_RESULT:
    safe_free(pszValue);
    if ( RET_FAILED(iRet) )
    {
        /* ���������Ϣ */
        AddErrInfo(&pstErrInfoList, iRet, szPath);

        iRet = RespErrMsg(pstMsg, iRet, pstErrInfoList);

        /* �ͷŴ�����Ϣ���� */
        FreeAllErrInfo(&pstErrInfoList);

        CleanCache(pList);

        return iRet;
    }
    else
    {
        RespSetMsg(pstMsg, g_iNeedReboot);
        return iRet;
    }
}



/**************************************************************************
����: ����cache�е�ֵ��CFG��
����: struct list_head *head,                                    �����б�;
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
void SaveCache2CFG(struct list_head *head)
{
    int iRet;

    ST_Cache_Entry *l = NULL;
    struct list_head *ptr;

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Cache_Entry,head);

        iRet = CFG_SetNodeVal(l->pszPath, l->pszValue, NULL);
        if ( RET_FAILED(iRet) )
        {
            COMMON_MSG_TRACE("save cache to cfg is failed,path is %s,value is %s\n",l->pszPath,l->pszValue);
        }
    }

    return;
}

/**************************************************************************
����: ��Ч�ı�������
����: unsigned short usMID,             ��ǰMID;
             char *pszPath,                         ��ǰ�����·��;
             char *pszValue,                        ��ǰ����Ľڵ�ֵ;
             ST_MSG_DATA *pstMsgData    ��Ϣ��������
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int ApplyAccessor(unsigned short usMID,char *pszPath,char *pszValue,ST_MSG_DATA *pstMsgData)
{
    int iRet = TBS_SUCCESS;
    int iRetMid = TBS_SUCCESS;
    int iIndex[MAX_INDEX_NUM]={0};

    unsigned short usMid = 0;
    ApplyFunc pfApply = NULL;


    if (pszPath==NULL || pstMsgData==NULL)
        return TBS_SUCCESS;

    pfApply = (ApplyFunc)GetHookFunc(pszPath,APPLY_FUNC,pstMsgData);
    if (pfApply)
    {
        COMM_GetPathIndex(pszPath,iIndex);

        /*����Ѿ�����,�򲻵���*/
        if (FunPointerIsInList(pfApply,iIndex))
        {
            COMMON_MSG_TRACE("Apply function have been called once.\n");
            iRet =  TBS_SUCCESS;
            goto APPLY_RETURN;
        }

        /*���������Ч����*/
        COMMON_MSG_TRACE("==Begin call apply function==\n");
        iRet = pfApply(pszPath,pszValue);

        if (!AddFuncPointer(pfApply,iIndex))
            COMMON_MSG_TRACE("FUNCTION LIST IS FULL\n");
    }

    /*�ж��Ƿ���ÿ���ڵ㶼��Ч�ĺ���,�����,����֮*/
#ifdef CONFIG_APPS_SSAP_APPLY_EACH_FUNC_HOOK 
    pfApply = (ApplyFunc)GetHookFunc(pszPath,APPLY_EACH_FUNC,pstMsgData);
    if(pfApply)
    {
        COMMON_MSG_TRACE("==Begin call apply each function==\n");
        iRet = pfApply(pszPath,pszValue);
    }
#endif

APPLY_RETURN:

/* ����ȥ���˶Խڵ�ı������, ����Ϊ�ڵ㱣��
   ͳһ�ŵ�һ��ģ�������apply���������,
   Ϊ���Ᵽ��Care�Ľڵ�, ��������һ������ SaveAccessor ,
   �����Ҫд�Ľڵ����mid ��� */
   (void)usMid; (void)iRetMid;
#if 0
    /*�����Ӧ�ڵ㵽������*/
    iRetMid = CFG_GetNodeMIDList(pszPath,NULL,&usMid,NULL,NULL,NULL,NULL);
    if (usMID == usMid )
    {
//        iRet = CFG_SetNodeVal(pszPath, pszValue, NULL);
    }
#endif
    return iRet;
}


int SaveAccessor(unsigned short usMID,char *pszPath,char *pszValue,ST_MSG_DATA *pstMsgData)
{
    int iRet = TBS_SUCCESS;
    int iRetMid = TBS_SUCCESS;
    int iIndex[MAX_INDEX_NUM]={0};

    unsigned short usMid = 0;
    ApplyFunc pfApply = NULL;
    const char *pcAccessor = NULL;


    if (pszPath==NULL || pstMsgData==NULL)
        return TBS_SUCCESS;
#if 0
    pfApply = (ApplyFunc)GetHookFunc(pszPath,APPLY_FUNC,pstMsgData);
    if (pfApply)
    {
        COMM_GetPathIndex(pszPath,iIndex);

        /*����Ѿ�����,�򲻵���*/
        if (FunPointerIsInList(pfApply,iIndex))
        {
            COMMON_MSG_TRACE("Apply function have been called once.\n");
            iRet =  TBS_SUCCESS;
            goto APPLY_RETURN;
        }

        /*���������Ч����*/
        COMMON_MSG_TRACE("==Begin call apply function==\n");
        iRet = pfApply(pszPath,pszValue);

        if (!AddFuncPointer(pfApply,iIndex))
            COMMON_MSG_TRACE("FUNCTION LIST IS FULL\n");
    }
APPLY_RETURN:
#endif
    (void)iIndex; (void)pfApply;

    if (MID_TR069FE == g_pstCurMsg->stMsgHead.usFirstMID
        || MID_CTMDW == g_pstCurMsg->stMsgHead.usFirstMID)
    {
        //pcAccessor = ""; /*����ƽ�޸ģ�Զ���޸Ľڵ�ҲӦ�����ϱ�*/
    }

    /*�����Ӧ�ڵ㵽������*/
    iRetMid = CFG_GetNodeMIDList(pszPath,NULL,&usMid,NULL,NULL,NULL,NULL);
    if (usMID == usMid )
    {
        iRet = CFG_SetNodeVal(pszPath, pszValue, pcAccessor);
    }
    return iRet;
}

/**************************************************************************
����: MODIFY_CFG_FUNC��Ӧ�ı�������
����: unsigned short usMID,             ��ǰMID;
             char *pszPath,                         ��ǰ�����·��;
             char *pszValue,                        ��ǰ����Ľڵ�ֵ;
             ST_MSG_DATA *pstMsgData   ��Ϣ��������
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
#ifdef MODIFY_CFG
int ModifyCFGAccessor(char *pszPath,char *pszValue,ST_MSG_DATA *pstMsgData)
{
    int iRet = TBS_SUCCESS;
    ModifyCFGFunc pfModifyCFG = NULL;

    if (pszPath==NULL || pstMsgData==NULL)
        return TBS_SUCCESS;

    pfModifyCFG = (ApplyFunc)GetHookFunc(pszPath,MODIFY_CFG_FUNC,pstMsgData);
    if (pfModifyCFG)
    {
        /*����Ѿ�����*/
        if (FunPointerIsInList(pfModifyCFG))
        {
            COMMON_MSG_TRACE("Modify function have been called once.\n");
            return TBS_SUCCESS;
        }
        iRet = pfModifyCFG(pszPath,pszValue);
        if (RET_FAILED(iRet))
        {
            return iRet;
        }
        else
        {
            if (!AddFuncPointer((void *)pfModifyCFG))
                COMMON_MSG_TRACE("FUNCTION LIST IS FULL\n");
        }
    }

    return TBS_SUCCESS;
}
#endif

int CallFuncInHash(void *key,void *data)
{
    ST_PATH_DATA * pPathData = (ST_PATH_DATA *)data;
    return ((ProcessFunc)(pPathData->pHookFunc))();
}

int CallPostApplyFuncInHash(void *key,void *data)
{
    ST_PATH_KEY *pPathKey = (ST_PATH_KEY *)key;
    ST_PATH_DATA * pPathData = (ST_PATH_DATA *)data;
    if (pPathKey->eHookType == APPLY_POST_FUNC)
        return ((ProcessFunc)(pPathData->pHookFunc))();

    return TBS_SUCCESS;
}

/**************************************************************************
����: ����COMMIT��Ϣ������
����: ST_MSG *pstMsg,                                    ��Ϣ��;
             ST_MSG_DATA *pstMsgData                   ��Ϣ��������
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int CommonCommitMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData)
{
    int iRet = TBS_SUCCESS;
    PreApplyFunc  pPreApply = NULL;
    PostApplyFunc pPostApply = NULL;
    ST_HookEntry * pHookEntry = NULL;

    g_pstCurMsg = pstMsg;
    if (!pstMsgData)
        return TBS_FAILED;

    COMMON_MSG_TRACE("=============CommonCommitMsgHandler==========\n");

    /*��Ч*/
    unsigned short usMid = pstMsg->stMsgHead.usDstMID;
    struct list_head *pList = MSG_MSG_GetModuleCache(usMid);

    /*����PRE APPLY����*/
    if (pstMsgData->stTableType == ARRAY_TABLE)
    {
        pHookEntry = pstMsgData->pstHookTable;
        while (pHookEntry->pHookFunc)
        {
            if (APPLY_PRE_FUNC == pHookEntry->eHookType)
            {
                pPreApply = (PreApplyFunc)pHookEntry->pHookFunc;
                if (!pPreApply)
                {
                    pHookEntry++;
                    continue;
                }

                iRet = pPreApply();
                if (RET_FAILED(iRet))
                {
                    COMMON_MSG_TRACE("Error: PreApply failed - func:0x%p\n", pPreApply);
                }
            }

            pHookEntry++;
        }
    }

    CacheList_ForEachOnce(usMid,pList,pstMsgData,ApplyAccessor);

    /*����ڵ�ֵ*/
    CacheList_ForEachOnce(usMid,pList,pstMsgData,SaveAccessor);
//    SaveCache2CFG(pList);

    /*POST APPLY ������ȫ��������  */
    if (pstMsgData->stTableType == ARRAY_TABLE)
    {
        pHookEntry = pstMsgData->pstHookTable;
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
        if (pstMsgData->pHashModuleFuncTable)
            hash_for_each_do(pstMsgData->pHashModuleFuncTable,CallPostApplyFuncInHash);
    }

    /* �ͷŻ��� */
    CleanCache(pList);

    return TBS_SUCCESS;
}


/**************************************************************************
����: ����CANCEL��Ϣ������
����: ST_MSG *pstMsg,                                    ��Ϣ��;
             ST_MSG_DATA *pstMsgData                   ��Ϣ��������
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int CommonCancelMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData)
{
//if (!pstCancelHookTable)
//        return TBS_FAILED;

    COMMON_MSG_TRACE("Cancel all modify.\n");

    unsigned short usMid = pstMsg->stMsgHead.usDstMID;
    struct list_head *pList = MSG_MSG_GetModuleCache(usMid);

    /* �ͷŻ��� */
    CleanCache(pList);

    /* ��ʱû�з�����Ҫ��cancel����Ķ�������������Ҫ������������� */

    return TBS_SUCCESS;
}

/**************************************************************************
����: ����DELETE��Ϣ������
����: ST_MSG *pstMsg,                                    ��Ϣ��;
             ST_MSG_DATA *pstMsgData                   ��Ϣ��������
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int CommonDeleteMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData)
{
    int iRet = TBS_SUCCESS;

    DeleteFunc pfDelete = NULL;
    CheckDeleteFunc pfCheckDelete = NULL;

    char *pcPos = pstMsg->szMsgBody;
    ST_ErrInfo * pstErrInfoList = NULL;

    /*pstMsg not null*/
    if ( NULL == pstMsg )
    {
        COMMON_MSG_TRACE("ERROR:set msg pointer is NULL!\n");
        return TBS_FAILED;
    }

    if (!pstMsgData)
    {
        iRet = TBS_SUCCESS;
        goto DELETE_SUCCESS;
    }

    /*����������Ƿ���ڶ�Ӧ�Ľڵ�*/
    iRet = CFG_GetNodeValPtr(pcPos,NULL,NULL);
    if (iRet != CFG_OK)
    {
        iRet = ERR_CFG_PATH_NOT_EXSITED;
        goto DELETE_FAIL;
    }

    /*����ɾ����麯�����м��*/
    pfCheckDelete = (CheckDeleteFunc)GetHookFunc(pcPos,DELETE_CHECK_FUNC,pstMsgData);
    if (pfCheckDelete)
    {
        iRet = pfCheckDelete(pcPos);
        if (RET_FAILED(iRet))
        {
            COMMON_MSG_TRACE("ERROR:check delete value error!PATH:%s\n",pcPos);
            goto DELETE_FAIL;
        }
    }
#if 0
    else
    {
        iRet = TR069_ERRNO_INVALID_NAME;
        COMMON_MSG_TRACE("ERROR:not find this path!PATH:%s\n",pcPos);
        goto DELETE_FAIL;
    }
#endif

    /*process delete msg */
    pfDelete = (DeleteFunc)GetHookFunc(pcPos,DELETE_FUNC,pstMsgData);
    if (pfDelete)
    {
        iRet = pfDelete(pcPos);
        if (RET_FAILED(iRet))
        {
            COMMON_MSG_TRACE("ERROR:delete msg process error!PATH:%s\n",pcPos);
            goto DELETE_FAIL;
        }
    }


DELETE_SUCCESS:
    /* ��ӦDel��Ϣ*/
    iRet = RespDelMsg(pstMsg, 0);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("ERROR: fail to response del message.\n");
        iRet = iRet;

        goto  DELETE_FAIL;
    }

    return TBS_SUCCESS;

DELETE_FAIL:

    /* ��Ӵ�����Ϣ*/
    AddErrInfo(&pstErrInfoList, iRet, pcPos);

    /* ��Ӧ������Ϣ*/
    iRet = RespErrMsg(pstMsg, iRet, pstErrInfoList);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response error message.\n");
    }

    /* �ͷŴ�����Ϣ����*/
    FreeAllErrInfo(&pstErrInfoList);
    return iRet;

}

/**************************************************************************
����: ����DELETED��Ϣ������
����: ST_MSG *pstMsg,                                    ��Ϣ��;
             ST_MSG_DATA *pstMsgData                   ��Ϣ��������
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int CommonDeletedMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData)
{
    int iRet = TBS_SUCCESS;

    DeletedFunc pfDeleted = NULL;

    char *pcPos = pstMsg->szMsgBody;
    ST_ErrInfo * pstErrInfoList = NULL;

    /*pstMsg not null*/
    if ( NULL == pstMsg )
    {
        COMMON_MSG_TRACE("ERROR:set msg pointer is NULL!\n");
        return TBS_FAILED;
    }

    if (!pstMsgData)
    {
        iRet = TBS_SUCCESS;
        goto DELETE_SUCCESS;
    }

    /*process delete msg */
    pfDeleted = (DeleteFunc)GetHookFunc(pcPos,DELETED_FUNC,pstMsgData);
    if (pfDeleted)
    {
        iRet = pfDeleted(pcPos);
        if (RET_FAILED(iRet))
        {
            COMMON_MSG_TRACE("ERROR:delete msg process error!PATH:%s\n",pcPos);
            goto DELETE_FAIL;
        }
    }

DELETE_SUCCESS:
    /* ��ӦDel��Ϣ*/
    iRet = RespDelMsg(pstMsg, 0);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("ERROR: fail to response del message.\n");
        iRet = iRet;

        goto  DELETE_FAIL;
    }

    return TBS_SUCCESS;

DELETE_FAIL:

    /* ��Ӵ�����Ϣ*/
    AddErrInfo(&pstErrInfoList, iRet, pcPos);

    /* ��Ӧ������Ϣ*/
    iRet = RespErrMsg(pstMsg, iRet, pstErrInfoList);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response error message.\n");
    }

    /* �ͷŴ�����Ϣ����*/
    FreeAllErrInfo(&pstErrInfoList);
    return iRet;

}

/**************************************************************************
����: ����ADD��Ϣ������
����: ST_MSG *pstMsg,                                    ��Ϣ��;
             ST_MSG_DATA *pstMsgData                   ��Ϣ��������
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int CommonAddMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData)
{
    int iRet = TBS_SUCCESS;
    unsigned long ulIndex = 0;

    CheckAddFunc pfAddCheck = NULL;
    AddFunc pfAdd=NULL;
    char *pcPos = pstMsg->szMsgBody;
    ST_ErrInfo * pstErrInfoList = NULL;

//    char szObjPath[MAX_PATH_LEN] = {0};

    /*pstMsg not null*/
    if ( NULL == pstMsg )
    {
        COMMON_MSG_TRACE("Error: set msg pointer is NULL!\n");
        return TBS_FAILED;
    }

    /*��麯��*/
    if (pstMsgData)
    {
        GetAddIndex pfnGetIndex = NULL;

        /*process add msg */
        pfAddCheck = (CheckAddFunc)GetHookFunc(pcPos,ADD_CHECK_FUNC,pstMsgData);
        if (pfAddCheck)
        {
            iRet = pfAddCheck(pcPos);
            if (RET_FAILED(iRet))
            {
                COMMON_MSG_TRACE("Error: check set value error!PATH:%s\n",pcPos);
                goto ADD_FAIL;
            }
        }
#if 0
        else
        {
            iRet = TR069_ERRNO_INVALID_NAME;
            COMMON_MSG_TRACE("ERROR:not find this path!PATH:%s\n",pcPos);
            goto ADD_FAIL;
        }
#endif

        pfnGetIndex = (GetAddIndex)GetHookFunc(pcPos, ADD_GET_INDEX_FUNC, pstMsgData);
        if (pfnGetIndex)
        {
            iRet = pfnGetIndex(pcPos, &ulIndex);
            if (RET_FAILED(iRet))
            {
                COMMON_MSG_TRACE("Error: check set value error!PATH:%s\n",pcPos);
                goto ADD_FAIL;
            }
        }
    }

    /* ������ýڵ� */
    iRet = CFG_AddObjInst(pcPos,&ulIndex);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: Add Cfg Child Node failed.\n");
        goto  ADD_FAIL;
    }

    /*������*/
    if (pstMsgData)
    {
        /*process add msg */
        pfAdd = (AddFunc)GetHookFunc(pcPos,ADD_FUNC,pstMsgData);
        if (pfAdd)
        {
            iRet = pfAdd(pcPos,ulIndex);
            if (RET_FAILED(iRet))
            {
                iRet = TBS_FAILED;
                COMMON_MSG_TRACE("Error: check set value error!PATH:%s\n",pcPos);
                goto ADD_FAIL;
            }
        }
    }

    /* ��Ӧadd��Ϣ*/
    iRet = RespAddMsg(pstMsg, ulIndex, 0);

    /* ��Ӧʧ��*/
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response add message.\n");
        iRet = iRet;
        goto  ADD_FAIL;
    }

    return TBS_SUCCESS;

ADD_FAIL:

    /* ��Ӵ�����Ϣ*/
    AddErrInfo(&pstErrInfoList, iRet, pcPos);

    /* ��Ӧ������Ϣ*/
    iRet = RespErrMsg(pstMsg, iRet, pstErrInfoList);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response error message.\n");
    }

    /* �ͷŴ�����Ϣ����*/
    FreeAllErrInfo(&pstErrInfoList);
    return TBS_FAILED;

}


/**************************************************************************
����: ����ADDED��Ϣ������
����: ST_MSG *pstMsg,                                    ��Ϣ��;
             ST_MSG_DATA *pstMsgData                   ��Ϣ��������
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int CommonAddedMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData)
{
    int iRet = TBS_SUCCESS;

    AddedFunc pfAdded=NULL;
    char *pcPos = pstMsg->szMsgBody;
    ST_ErrInfo * pstErrInfoList = NULL;

//    char szObjPath[MAX_PATH_LEN] = {0};

    /*pstMsg not null*/
    if ( NULL == pstMsg )
    {
        COMMON_MSG_TRACE("Error: set msg pointer is NULL!\n");
        return TBS_FAILED;
    }

    if (!pstMsgData)
    {
        iRet = TBS_SUCCESS;
        goto ADDED_SUCCESS;
    }

    /*������*/
    if (pstMsgData)
    {
        /*process add msg */
        pfAdded = (AddedFunc)GetHookFunc(pcPos,ADDED_FUNC,pstMsgData);
        if (pfAdded)
        {
            iRet = pfAdded(pcPos);
            if (RET_FAILED(iRet))
            {
                iRet = TBS_FAILED;
                COMMON_MSG_TRACE("Error: check set value error!PATH:%s\n",pcPos);
                goto ADDED_FAIL;
            }
        }
    }

ADDED_SUCCESS:
    /* ��Ӧadd��Ϣ*/
    iRet = RespAddMsg(pstMsg, 0, 0);

    /* ��Ӧʧ��*/
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response add message.\n");
        iRet = iRet;
        goto  ADDED_FAIL;
    }

    return TBS_SUCCESS;

ADDED_FAIL:

    /* ��Ӵ�����Ϣ*/
    AddErrInfo(&pstErrInfoList, iRet, pcPos);

    /* ��Ӧ������Ϣ*/
    iRet = RespErrMsg(pstMsg, iRet, pstErrInfoList);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response error message.\n");
    }

    /* �ͷŴ�����Ϣ����*/
    FreeAllErrInfo(&pstErrInfoList);
    return TBS_FAILED;

}



/**************************************************************************
����: ����UPDATE��Ϣ������
����: ST_MSG *pstMsg,                                    ��Ϣ��;
             ST_MSG_DATA *pstMsgData                   ��Ϣ��������
����: �ɹ� -- TBS_SUCCESS
             ʧ�� -- TBS_FAILED
��ע:
***************************************************************************/
int CommonUpdateMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData)
{
    int iRet=TBS_SUCCESS;
    UpdateFunc pfUpdateFunc = NULL;
//    char *pcPos = pstMsg->szMsgBody;

    /*pstMsg not null*/
    if ( NULL == pstMsg )
    {
        COMMON_MSG_TRACE("Error: set msg pointer is NULL! file:%s-line:%d\n",__FILE__,__LINE__);
        return TBS_FAILED;
    }

    if (!pstMsgData)
    {
        iRet = TBS_SUCCESS;
        goto UPDATE_RESULT;
    }

    /*now,update message has no body,must wait for new update message*/
#if 0
    GET_ULONG(pcPos, ulCount);
    for ( i = 0; i < ulCount; i++ )
    {
        /*������*/
        if (pstUpdateHookTable)
        {
            /*process update msg */
            pfUpdateFunc = (UpdateFunc)GetHookFunc(pcPos,UPDATE_FUNC,pstUpdateHookTable);
            if (pfUpdateFunc)
            {
                iRet = pfUpdateFunc(pcPos);
                if ( RET_FAILED(iRet) )
                {
                    COMMON_MSG_TRACE("Error: update value error!PATH:%s",pcPos);
                    goto UPDATE_RESULT;
                }
            }
        }
        pcPos += strlen(pcPos) + 1;
    }
#endif

    /*û�м��ٴ����ģ��,ֱ�ӱ���*/
    if (pstMsgData->stTableType == ARRAY_TABLE)
    {
        ST_HookEntry * pHookEntry = pstMsgData->pstHookTable;
		
        while (pHookEntry->pHookFunc)
        {
			
            pfUpdateFunc = (UpdateFunc)pHookEntry->pHookFunc;

            iRet = pfUpdateFunc();
            if (RET_FAILED(iRet))
            {
                COMMON_MSG_TRACE("Error: Update failed - msg:0x%04x,func:0x%p,path:%s\n",
                                 pHookEntry->eHookType,pHookEntry->pHookFunc,pHookEntry->pNodePath);
            }

            pHookEntry++;
        }
    }
    else /*�������hash��*/
    {
        if (pstMsgData->pHashModuleFuncTable)
            hash_for_each_do(pstMsgData->pHashModuleFuncTable,CallFuncInHash);
    }

UPDATE_RESULT:
    /* ��Ӧ��Ϣ*/
    iRet = RespUpdateMsg(pstMsg, 0);
    //COMMON_MSG_TRACE("Response cfg update message \n");

    return iRet;
}

