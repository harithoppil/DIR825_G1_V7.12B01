/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称: Common_fun.c
 文件描述: 公共消息处理的封装，简化各模块的处理动作

 修订记录:
        1. 作者: 李伟
           日期: 2008-08-07
           内容: 创建文件

**********************************************************************/

#include <assert.h>
#include "cache_list.h"
#include "common.h"
#include "common_func.h"
#include "common_msg_handler.h"
#include "new_msg.h"


extern ST_MODULE_NODE g_Module_Table[];

/*************************************************************************
功能:   解析set命令的路径和值
参数:   const char *pszSetCmd ,         set命令字符串
               char *pszPath ,                        路径
               unsigned int *pnPathLen ,     路径长度
               char *pszValue ,                    值
               unsigned int *pnValueLen ,   值长度
返回:   TBS_SUCCESS ,    成功
               其它,                失败
备注:
*************************************************************************/
int ParseSetCmdSimple(const char *pszSetCmd,char *pszPath,unsigned int *pnPathLen,char *pszValue,unsigned int *pnValueLen)
{
    const char *pcPos = NULL;
    unsigned int nLen = 0;

    /* 保存节点名和节点值的缓冲区不存在*/
    if ( NULL == pszPath || NULL == pszValue||NULL== pszSetCmd||
            NULL==pnPathLen||NULL==pnValueLen)
    {
        COMMON_MSG_TRACE("null pointer,when parse set cmd\n");
        return TBS_NULL_PTR;
    }

    /* 查找等号位置*/
    pcPos = strchr(pszSetCmd, '=');
    if ( NULL == pcPos )
    {
        COMMON_MSG_TRACE("can't find =\n");
        return TR069_ERRNO_INVALID_VAL;
    }

    /* 等号前面的是节点名*/
    nLen = pcPos - pszSetCmd + 1;
    if ( nLen > *pnPathLen )
    {
        COMMON_MSG_TRACE("can't get node name\n");
        return TR069_ERRNO_INVALID_NAME;
    }

    safe_strncpy(pszPath, pszSetCmd, nLen);
    *pnPathLen = nLen;

    /* 等号后面的是节点值*/
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
  *    缓存链表处理相关函数函数
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
  *     已处理函数链表相关数据结构及函数封装
  */

/* 用来记录生效过的函数，确保同一个实例情况下，不会多次调同一函数*/
typedef struct tag_ST_FUNCTION_NODE
{
    void *pFunction;                                                 /*处理函数*/
    int iPathIndex[MAX_INDEX_NUM];                      /*index数组,传入多实例的情况下需要*/
    struct tag_ST_FUNCTION_NODE *pNext;             /*指针*/
}ST_FUNCTION_NODE;

/* 链表头尾指针*/
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
  *     用于冲突检查中，将产生冲突的节点的路径保存到全局数组中
  */
char g_szErrPath[MAX_PATH_LEN] = {0};  /*记录错误路径*/
extern BOOL g_bIsModuleDefined;              /*模块是否已初始化*/

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
  Description: 用于冲突检查失败后获取产生冲突的节点路径
***************************************************************/
static inline const char *GetLastErrPath(void)
{
    return g_szErrPath;
}

/*
  *  消息处理相关函数
  */

/**************************************************************************
功能: 获得处理使用的钩子函数
参数: char *pcNodePath,                 当前路径;
               HookType eHookType,           钩子函数类型;
               ST_MSG_DATA *pstMsgData   当前处理的消息;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
void *GetHookFunc(char *pcNodePath,HookType eHookType,ST_MSG_DATA *pstMsgData)
{
    ST_HookEntry * pHookEntry = NULL;
    hashtable *pHash = NULL;
    ST_PATH_KEY PathKey;
    ST_PATH_DATA *pPathData;

    /*消息为空 ，则返回空函数*/
    if (!pstMsgData)
    {
        COMMON_MSG_TRACE("HOOK TABLE POINTER IS NULL");
        return NULL;
    }

    COMMON_MSG_TRACE("Input Path = %s\n",pcNodePath);


    /*非加速的模块使用的是全匹配的方式*/
    if (pstMsgData->stTableType == ARRAY_TABLE)
    {
        pHookEntry = pstMsgData->pstHookTable;
        /*遍历钩子表，查找处理函数*/
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
    /*需加速的模块使用的是hash的方式*/
    else
    {
        pHash = pstMsgData->pHashModuleFuncTable;

        PathKey.pszPath = pcNodePath;
        PathKey.eHookType = eHookType;

        /*查找hash表*/
        pPathData = FuncHash_GetValue(&PathKey,pHash);
        if (pPathData)
            return pPathData->pHookFunc;
    }

    COMMON_MSG_TRACE("No Match Path.\n");

    return NULL;
}


typedef int (*COMMON_PROCESS)(ST_MSG *,ST_MSG_DATA *);


/**************************************************************************
功能: 根据消息调用对应的处理函数
参数: ST_MSG_DATA *pstMsgData,                消息处理函数表;
             ST_MSG *pstMsg,                                  当前处理的消息;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int Call_Msg_ProcFunc(ST_MSG_DATA *pstMsgData,ST_MSG *pstMsg)
{

    MSG_PROCESS_FUNC pfnProcess = NULL;
//    ST_HookEntry *pstHookTable = NULL;
//    hashtable *pstHash = NULL;

    /*如果有自定义消息处理函数，则调用自定义函数*/
    if (NULL == pstMsgData)
    {
        pfnProcess = NULL;
    }
    else
    {
        pfnProcess = pstMsgData->pfCustomProcess;
    }


    /*不同消息所对应的公共处理函数*/
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

    /*自定义处理函数的调用*/
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

    /*根据对应的消息类型，调用公共处理函数*/
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
功能: 冲突检查的遍历函数
参数: unsigned short usMID,             当前MID;
             char *pszPath,                         当前传入的路径;
             char *pszValue,                        当前传入的节点值;
             ST_MSG_DATA *pstMsgData    消息处理函数表;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int CheckConflictAccessor(unsigned short usMID,char *pszPath,char *pszValue,ST_MSG_DATA *pstMsgData)
{
    int iRet = TBS_SUCCESS;
    int iIndex[MAX_INDEX_NUM]={0};

    CheckConflictFunc pfCheck = (CheckConflictFunc)GetHookFunc(pszPath,CHECK_CONFLICT_FUNC,pstMsgData);

    if (pfCheck)
    {
        COMM_GetPathIndex(pszPath,iIndex);

        /*如果已经检查过，则跳过*/
        if (FunPointerIsInList(pfCheck,iIndex))
            return TBS_SUCCESS;


        /*否则直接调用处理函数*/
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
功能: 公共的缓存遍历函数，每个实例只调用一次
参数: unsigned short usMID,                            当前MID;
             struct list_head *head,                           链表头;
             ST_MSG_DATA *pstMsgData                    消息处理函数;
             CheckConflictAccessorFunc pAccessor  遍历的函数体
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
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
/*遍历缓存，对应函数可调多次*/
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

/* 在Set消息改变参数值后是否需要重启，仅在一次操作中有效 */
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
功能: 公共set消息处理函数
参数: ST_MSG *pstMsg,                                    消息体;
             ST_MSG_DATA *pstMsgData                   消息处理函数表
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
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

    /* 默认情况下不需要重启生效 */
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

        /*由于多次循环,所以每次循环要将内存清零*/
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

            /* 比较，如果两个值不等则放入到缓存 */
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

        /* 准备处理下一个节点 */
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
        /* 保存错误信息 */
        AddErrInfo(&pstErrInfoList, iRet, szPath);

        iRet = RespErrMsg(pstMsg, iRet, pstErrInfoList);

        /* 释放错误信息链表 */
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
功能: 保存cache中的值到CFG中
参数: struct list_head *head,                                    缓存列表;
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
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
功能: 生效的遍历函数
参数: unsigned short usMID,             当前MID;
             char *pszPath,                         当前传入的路径;
             char *pszValue,                        当前传入的节点值;
             ST_MSG_DATA *pstMsgData    消息处理函数表
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
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

        /*如果已经检查过,则不调用*/
        if (FunPointerIsInList(pfApply,iIndex))
        {
            COMMON_MSG_TRACE("Apply function have been called once.\n");
            iRet =  TBS_SUCCESS;
            goto APPLY_RETURN;
        }

        /*否则调用生效函数*/
        COMMON_MSG_TRACE("==Begin call apply function==\n");
        iRet = pfApply(pszPath,pszValue);

        if (!AddFuncPointer(pfApply,iIndex))
            COMMON_MSG_TRACE("FUNCTION LIST IS FULL\n");
    }

    /*判断是否有每个节点都生效的函数,如果有,调用之*/
#ifdef CONFIG_APPS_SSAP_APPLY_EACH_FUNC_HOOK 
    pfApply = (ApplyFunc)GetHookFunc(pszPath,APPLY_EACH_FUNC,pstMsgData);
    if(pfApply)
    {
        COMMON_MSG_TRACE("==Begin call apply each function==\n");
        iRet = pfApply(pszPath,pszValue);
    }
#endif

APPLY_RETURN:

/* 这里去掉了对节点的保存操作, 是因为节点保存
   统一放到一个模块的所有apply结束后进行,
   为避免保存Care的节点, 新增加了一个函数 SaveAccessor ,
   会对所要写的节点进行mid 检查 */
   (void)usMid; (void)iRetMid;
#if 0
    /*保存对应节点到配置树*/
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

        /*如果已经检查过,则不调用*/
        if (FunPointerIsInList(pfApply,iIndex))
        {
            COMMON_MSG_TRACE("Apply function have been called once.\n");
            iRet =  TBS_SUCCESS;
            goto APPLY_RETURN;
        }

        /*否则调用生效函数*/
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
        //pcAccessor = ""; /*付火平修改，远程修改节点也应主动上报*/
    }

    /*保存对应节点到配置树*/
    iRetMid = CFG_GetNodeMIDList(pszPath,NULL,&usMid,NULL,NULL,NULL,NULL);
    if (usMID == usMid )
    {
        iRet = CFG_SetNodeVal(pszPath, pszValue, pcAccessor);
    }
    return iRet;
}

/**************************************************************************
功能: MODIFY_CFG_FUNC对应的遍历函数
参数: unsigned short usMID,             当前MID;
             char *pszPath,                         当前传入的路径;
             char *pszValue,                        当前传入的节点值;
             ST_MSG_DATA *pstMsgData   消息处理函数表
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
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
        /*如果已经检查过*/
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
功能: 公共COMMIT消息处理函数
参数: ST_MSG *pstMsg,                                    消息体;
             ST_MSG_DATA *pstMsgData                   消息处理函数表
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
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

    /*生效*/
    unsigned short usMid = pstMsg->stMsgHead.usDstMID;
    struct list_head *pList = MSG_MSG_GetModuleCache(usMid);

    /*调用PRE APPLY函数*/
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

    /*保存节点值*/
    CacheList_ForEachOnce(usMid,pList,pstMsgData,SaveAccessor);
//    SaveCache2CFG(pList);

    /*POST APPLY 函数会全部被调用  */
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

    /* 释放缓存 */
    CleanCache(pList);

    return TBS_SUCCESS;
}


/**************************************************************************
功能: 公共CANCEL消息处理函数
参数: ST_MSG *pstMsg,                                    消息体;
             ST_MSG_DATA *pstMsgData                   消息处理函数表
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
***************************************************************************/
int CommonCancelMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData)
{
//if (!pstCancelHookTable)
//        return TBS_FAILED;

    COMMON_MSG_TRACE("Cancel all modify.\n");

    unsigned short usMid = pstMsg->stMsgHead.usDstMID;
    struct list_head *pList = MSG_MSG_GetModuleCache(usMid);

    /* 释放缓存 */
    CleanCache(pList);

    /* 暂时没有发现需要在cancel处理的动作，后面有需要可以在这里加入 */

    return TBS_SUCCESS;
}

/**************************************************************************
功能: 公共DELETE消息处理函数
参数: ST_MSG *pstMsg,                                    消息体;
             ST_MSG_DATA *pstMsgData                   消息处理函数表
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
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

    /*检查配置树是否存在对应的节点*/
    iRet = CFG_GetNodeValPtr(pcPos,NULL,NULL);
    if (iRet != CFG_OK)
    {
        iRet = ERR_CFG_PATH_NOT_EXSITED;
        goto DELETE_FAIL;
    }

    /*调用删除检查函数进行检查*/
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
    /* 响应Del消息*/
    iRet = RespDelMsg(pstMsg, 0);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("ERROR: fail to response del message.\n");
        iRet = iRet;

        goto  DELETE_FAIL;
    }

    return TBS_SUCCESS;

DELETE_FAIL:

    /* 添加错误信息*/
    AddErrInfo(&pstErrInfoList, iRet, pcPos);

    /* 响应错误消息*/
    iRet = RespErrMsg(pstMsg, iRet, pstErrInfoList);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response error message.\n");
    }

    /* 释放错误信息链表*/
    FreeAllErrInfo(&pstErrInfoList);
    return iRet;

}

/**************************************************************************
功能: 公共DELETED消息处理函数
参数: ST_MSG *pstMsg,                                    消息体;
             ST_MSG_DATA *pstMsgData                   消息处理函数表
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
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
    /* 响应Del消息*/
    iRet = RespDelMsg(pstMsg, 0);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("ERROR: fail to response del message.\n");
        iRet = iRet;

        goto  DELETE_FAIL;
    }

    return TBS_SUCCESS;

DELETE_FAIL:

    /* 添加错误信息*/
    AddErrInfo(&pstErrInfoList, iRet, pcPos);

    /* 响应错误消息*/
    iRet = RespErrMsg(pstMsg, iRet, pstErrInfoList);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response error message.\n");
    }

    /* 释放错误信息链表*/
    FreeAllErrInfo(&pstErrInfoList);
    return iRet;

}

/**************************************************************************
功能: 公共ADD消息处理函数
参数: ST_MSG *pstMsg,                                    消息体;
             ST_MSG_DATA *pstMsgData                   消息处理函数表
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
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

    /*检查函数*/
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

    /* 添加配置节点 */
    iRet = CFG_AddObjInst(pcPos,&ulIndex);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: Add Cfg Child Node failed.\n");
        goto  ADD_FAIL;
    }

    /*处理函数*/
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

    /* 响应add消息*/
    iRet = RespAddMsg(pstMsg, ulIndex, 0);

    /* 响应失败*/
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response add message.\n");
        iRet = iRet;
        goto  ADD_FAIL;
    }

    return TBS_SUCCESS;

ADD_FAIL:

    /* 添加错误信息*/
    AddErrInfo(&pstErrInfoList, iRet, pcPos);

    /* 响应错误消息*/
    iRet = RespErrMsg(pstMsg, iRet, pstErrInfoList);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response error message.\n");
    }

    /* 释放错误信息链表*/
    FreeAllErrInfo(&pstErrInfoList);
    return TBS_FAILED;

}


/**************************************************************************
功能: 公共ADDED消息处理函数
参数: ST_MSG *pstMsg,                                    消息体;
             ST_MSG_DATA *pstMsgData                   消息处理函数表
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
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

    /*处理函数*/
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
    /* 响应add消息*/
    iRet = RespAddMsg(pstMsg, 0, 0);

    /* 响应失败*/
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response add message.\n");
        iRet = iRet;
        goto  ADDED_FAIL;
    }

    return TBS_SUCCESS;

ADDED_FAIL:

    /* 添加错误信息*/
    AddErrInfo(&pstErrInfoList, iRet, pcPos);

    /* 响应错误消息*/
    iRet = RespErrMsg(pstMsg, iRet, pstErrInfoList);
    if (RET_FAILED(iRet))
    {
        COMMON_MSG_TRACE("Error: fail to response error message.\n");
    }

    /* 释放错误信息链表*/
    FreeAllErrInfo(&pstErrInfoList);
    return TBS_FAILED;

}



/**************************************************************************
功能: 公共UPDATE消息处理函数
参数: ST_MSG *pstMsg,                                    消息体;
             ST_MSG_DATA *pstMsgData                   消息处理函数表
返回: 成功 -- TBS_SUCCESS
             失败 -- TBS_FAILED
备注:
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
        /*处理函数*/
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

    /*没有加速处理的模块,直接遍历*/
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
    else /*否则遍历hash表*/
    {
        if (pstMsgData->pHashModuleFuncTable)
            hash_for_each_do(pstMsgData->pHashModuleFuncTable,CallFuncInHash);
    }

UPDATE_RESULT:
    /* 响应消息*/
    iRet = RespUpdateMsg(pstMsg, 0);
    //COMMON_MSG_TRACE("Response cfg update message \n");

    return iRet;
}

