/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称: New_msg.c
 文件描述: 新消息调度过程的封装函数

 修订记录:
        1. 作者: 李伟
           日期: 2008-08-07
           内容: 创建文件

**********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "hash.h"
#include "new_msg.h"
#include "msg_prv.h"
#include "tbsutil.h"
#include "tbserror.h"


/* 当前模块id, 用于调试中显示当前调度到哪个模块 */
#ifdef _MSG_DEBUG
STATIC unsigned short s_usMsgCurMID = 0;
#define MSG_CUR_MID(usDstMID) (s_usMsgCurMID = (usDstMID))
#else
#define MSG_CUR_MID(usDstMID)  (void)0
#endif

ST_MODULE_NODE g_Module_Table[MAX_MODULE_NUM];

/*
  *     module list define and function
  */

static unsigned short stModuleList[MAX_MODULE_NUM] = {0};       /*模块记录链表*/
static unsigned int iModuleNum=0;                                                    /*已用的数量*/

/*************************************************************************
功能:   添加模块到列表中
参数:   unsigned short usModuleID ,         模块ID
返回:   TRUE ,    成功
               其它,                失败
备注:
*************************************************************************/
BOOL AddModule2List(unsigned short usModuleID)
{
    if (iModuleNum>=MAX_MODULE_NUM-1)
        return FALSE;

    stModuleList[iModuleNum++] = usModuleID;
    return TRUE;
}

/*************************************************************************
功能:   检查模块是否在列表中
参数:   unsigned short usModuleID ,         模块ID
返回:   TRUE ,     在
               其它,   不在
备注:
*************************************************************************/
BOOL ModuleIsInList(unsigned short usModuleID)
{
    unsigned int i = 0;
    for (i=0;i<iModuleNum;i++)
    {
        if ( stModuleList[i] == usModuleID )
            return TRUE;
    }
    return FALSE;
}

/*************************************************************************
功能:   从列表中删除模块
参数:   unsigned short usModuleID ,         模块ID
返回:   无
备注:
*************************************************************************/
void RemoveModuleFromList(unsigned short usModuleID)
{
    unsigned int i = 0;
    for (i=0;i<iModuleNum;i++)
    {
        if ( stModuleList[i] == usModuleID )
        {
            iModuleNum--;
            for (;i<iModuleNum;i++)
                stModuleList[i] = stModuleList[i+1];
            stModuleList[i] = 0;
            break;
        }
    }
}

/*************************************************************************
功能:   获取模块数量
参数:   无
返回:   模块数量
备注:
*************************************************************************/
unsigned int GetModuleListCount()
{
    return iModuleNum;
}

/*************************************************************************
功能:   获取对应的模块MID
参数:   无
返回:   模块数量
备注:
*************************************************************************/
unsigned short GetModuleID(unsigned int uiID)
{
    return stModuleList[uiID];
}

/*************************************************************************
功能:   清空MID列表
参数:   无
返回:   模块数量
备注:
*************************************************************************/
void CleanModuleList()
{
    memset(stModuleList,0,MAX_MODULE_NUM*sizeof(short));
    iModuleNum=0;
}

/*
  *     MSG TYPE hash中的hash函数的定义
  */

#define HASH_MSG_NUM 61
#define MSG_FUNC_HASH_SIZE 13

unsigned int MsgType_Hash(void *pKey)
{
    ST_MSG_KEY *pMsgKey = (ST_MSG_KEY *)pKey;

    return (pMsgKey->usMsgType)%MSG_FUNC_HASH_SIZE;
}

int Equal_MsgType(void *pKey1,void *pKey2)
{
    if (((ST_MSG_KEY *)pKey1)->usMsgType != ((ST_MSG_KEY *)pKey2)->usMsgType)
        return false;

    return true;
}

#if 0
typedef enum
{
    /* SET MSG FUNCTION */
    CHECK_FUNC = 1,                                         /*节点检查函数*/
    CHECK_CONFLICT_FUNC,                             /*节点值冲突检查函数*/

    /* COMMIT MSG FUNCTION */
    APPLY_FUNC,                                               /*生效函数*/
    APPLY_POST_FUNC,                                     /*模块保存信息到配置树后调用的公共函数，主要用来处理模块的公共动作*/
#ifdef MODIFY_CFG
    MODIFY_CFG_FUNC,                                     /*缓存值保存到配置树后调用的函数，主要用来进行配置树的修改*/
#endif

    /* CANCEL MSG FUNCTION ,NO ONE NOW*/

    /* DELETE MSG FUNCTION */
    DELETE_CHECK_FUNC,                                 /*删除检查函数*/
    DELETE_FUNC,                                              /*删除生效函数*/

    /*ADD MSG FUNCTION*/
    ADD_CHECK_FUNC,                                       /**/
    ADD_FUNC,

    /*ADDED MSG FUNCTION*/
    ADDED_FUNC,

    /*DELETED MSG FUNCTION*/
    DELETED_FUNC,

    /*UPDATE MSG FUNCTION*/
    UPDATE_FUNC,

}HookType;
#endif

/*
  *     钩子函数处理表hash中的hash函数的定义
  */

/* 这里的hash算法会对应不同的消息类型计算不同的位置 */
#define PATH_HASH_NUM 61
unsigned int Path_Hash(void *pKey)
{
    ST_PATH_KEY *pPathKey = (ST_PATH_KEY *)pKey;
    char *pPath = pPathKey->pszPath;
    int iLen = strlen(pPath);

        if ((pPathKey->eHookType == CHECK_FUNC)
            ||(pPathKey->eHookType == CHECK_CONFLICT_FUNC)
            ||(pPathKey->eHookType == APPLY_FUNC))
        {
            if (pPath[iLen-1] == '$')
                iLen--;

            if(iLen>=5)
                return (pPath[iLen-1]+pPath[iLen-3]+pPath[iLen-5])%HASH_MSG_NUM;
            else
                return 0;
        }

        else if ((pPathKey->eHookType == ADD_CHECK_FUNC)
            ||(pPathKey->eHookType == ADD_FUNC)
            ||(pPathKey->eHookType == ADDED_FUNC)
            ||(pPathKey->eHookType == DELETE_CHECK_FUNC)
            ||(pPathKey->eHookType == DELETE_FUNC)
            ||(pPathKey->eHookType == DELETED_FUNC))
        {
            if (pPath[iLen-1] == '$')
                iLen--;

            if(pPath[iLen-1] == '.')
                iLen--;

            if(iLen>=5)
			{
				if(pPath[iLen-1] == '\\')
					return HASH_MSG_NUM>>1;
				if((pPath[iLen-1] >= '0')&&(pPath[iLen-1] <= '9'))
					return HASH_MSG_NUM>>1;
				return (pPath[iLen-1]+pPath[iLen-3]+pPath[iLen-5])%HASH_MSG_NUM;
			}
            else
                return 0;
        }

        else
        {
            return 0;
        }
}

int Equal_Path(void *pKey1,void *pKey2)
{
    if (((ST_PATH_KEY *)pKey1)->eHookType != ((ST_PATH_KEY *)pKey2)->eHookType)
        return false;

    if (!strcmp(((ST_PATH_KEY *)pKey1)->pszPath,((ST_PATH_KEY *)pKey2)->pszPath))
        return false;

    return true;
}

void *FuncHash_GetValue(void *key, hashtable *tab)
{
    int i=0;
    struct hashentry *pos;
    unsigned int index = (tab->gethash)(key) % (tab->hashsize -1);

    for (pos = tab->hashlist[index]; NULL != pos; pos = pos->next) {
        if (((ST_PATH_KEY *)key)->eHookType != ((ST_PATH_KEY *)pos->key)->eHookType)
            continue;

        if (tbsMatch(((ST_PATH_KEY *)key)->pszPath,((ST_PATH_KEY *)pos->key)->pszPath))
        {
            return pos->data;
        }
        else
            continue;
    }

/* 通过哈希方式找不到则返回NULL，不再遍历所有hook entry，因为这样比较耗费时间 */
#if 0
    for (i = 0; i < tab->hashsize; i++) {
        for (pos = tab->hashlist[i]; NULL != pos; pos = pos->next ) {
            if (((ST_PATH_KEY *)key)->eHookType != ((ST_PATH_KEY *)pos->key)->eHookType)
                continue;

            if (tbsMatch(((ST_PATH_KEY *)key)->pszPath,((ST_PATH_KEY *)pos->key)->pszPath))
                return pos->data;
            else
                continue;
        }
    }
#endif

    return NULL;
}

/*
  *     新的消息处理相关函数的定义
  */

/*************************************************************************
功能:   初始化相关结构体
参数:   无
返回:   模块数量
备注:
*************************************************************************/
void MSG_MSG_Init()
{
    memset(g_Module_Table,0,sizeof(g_Module_Table));
}

/*************************************************************************
功能:   建立函数钩子表的hash
参数:   ST_HookEntry *pHookTable                  函数处理的钩子表
               hashtable *pHash                                  建立的钩子表
返回:   无
备注:
*************************************************************************/
void CreateFuncHash(ST_HookEntry *pHookTable,hashtable *pHash)
{
    ST_HookEntry *pEntry = pHookTable;
    while ( NULL != pEntry->pNodePath)
    {
        ST_PATH_KEY *pKey = (ST_PATH_KEY *)malloc(sizeof(ST_PATH_KEY));
        if (NULL == pKey)
        {
            return ;
        }
        pKey->pszPath = pEntry->pNodePath;
        pKey->eHookType = pEntry->eHookType;

        ST_PATH_DATA *pData = (ST_PATH_DATA *)malloc(sizeof(ST_PATH_DATA));
        if (NULL == pData)
        {
            free(pKey);
            return;
        }

        pData->pHookFunc = pEntry->pHookFunc;

        hash_insert(pKey,pData,pHash);

        pEntry++;
    }

}

/*************************************************************************
功能:   添加消息处理的消息调度表到hash表中
参数:   unsigned short usMID                                                        MID
               ST_MSG_MAPPING *pstMsgMapTable                                模块的消息调度表
返回:   无
备注:
*************************************************************************/
bool MSG_MSG_AddMsgFunc(unsigned short usMID, ST_MSG_MAPPING *pstMsgMapTable)
{
    int i = MID2INDEX(usMID);
    g_Module_Table[i].pHashModuleFuncTable = hash_create(MsgType_Hash,Equal_MsgType,MSG_FUNC_HASH_SIZE);

    ST_MSG_MAPPING *pstMsgMapEntry = pstMsgMapTable;
    while (pstMsgMapEntry->usMsgType)
    {
        ST_MSG_KEY *pKey = (ST_MSG_KEY *)malloc(sizeof(ST_MSG_KEY));
        if (NULL == pKey)
        {
            return false;
        }
        pKey->usMsgType = pstMsgMapEntry->usMsgType;

        ST_MSG_DATA *pData = (ST_MSG_DATA *)malloc(sizeof(ST_MSG_DATA));
        if (NULL == pData)
        {
            free(pKey);
            return false;
        }
        pData->pfCustomProcess = pstMsgMapEntry->pfCustomProcess;

        /*实验用，暂时只对无线使用hash,LIWEI_TEST*/

        if (usMID == MID_WLAN && pstMsgMapEntry->pfCustomProcess == NULL)
        {
            pData->stTableType = HASH_TABLE;
            pData->pHashModuleFuncTable = hash_create(Path_Hash,Equal_Path,PATH_HASH_NUM);
            CreateFuncHash(pstMsgMapEntry->pstHookTable,pData->pHashModuleFuncTable);
        }
        else
        {
            pData->stTableType = ARRAY_TABLE;
            pData->pstHookTable = pstMsgMapEntry->pstHookTable;
        }

        hash_insert(pKey,pData,g_Module_Table[i].pHashModuleFuncTable);
        pstMsgMapEntry++;
    }

    InitCache(&(g_Module_Table[i].listCache));

    return true;
}

/*************************************************************************
功能:   删除消息处理的消息调度表从hansh
参数:   unsigned short usMID                                                        MID
               ST_MSG_MAPPING *pstMsgMapTable                                模块的消息调度表
返回:   无
备注:
*************************************************************************/
bool MSG_MSG_DelMsgFunc(unsigned short usMID)
{
    int i = MID2INDEX(usMID);

    hash_free(g_Module_Table[i].pHashModuleFuncTable);
    g_Module_Table[i].pHashModuleFuncTable = NULL;

    CleanCache(&(g_Module_Table[i].listCache));

    return true;
}

/*************************************************************************
功能:   清理相关的结构体
参数:   无
返回:   无
备注:
*************************************************************************/
void MSG_MSG_Uninit()
{
    int i=0;

    for (i=0;i<MAX_MODULE_NUM;i++)
    {
        if (g_Module_Table[i].pHashModuleFuncTable)
        {
            hash_free(g_Module_Table[i].pHashModuleFuncTable);
            g_Module_Table[i].pHashModuleFuncTable = NULL;
            CleanCache(&(g_Module_Table[i].listCache));
        }
    }
}

/*************************************************************************
功能:   检查是否模块消息调度表是否为空
参数:   无
返回:   TRUE     为空
               FALSE   为非空
备注:
*************************************************************************/
unsigned char MSG_MSG_IsAllUnreged(void)
{
    int i=0;

    for (i=0;i<MAX_MODULE_NUM;i++)
    {
        if (g_Module_Table[i].pHashModuleFuncTable)
        {
            return false;
        }
    }

    return true;
}

/*************************************************************************
功能:   检查某个模块是否注册
参数:   无
返回:   TRUE     为空
               FALSE   为非空
备注:
*************************************************************************/
bool MSG_MSG_IsMsgReged(unsigned short usMID)
{

    /*modify for update message and cancel message*/
#if 0
    ST_MSG_KEY stKey;
    stKey.usMID = usMID;
    stKey.usMsgType = usMsgType;

    return (bool)hash_value(&stKey,phtMsgFunc);
#endif
    return ModuleIsInList(usMID);

}

/*************************************************************************
功能:   获取消息调度函数表或hash表
参数:   unsigned short usMID               模块MID
               unsigned short usMsgType       消息类型
               ST_MSG_DATA **pMsgData      消息与函数映射的结构
返回:   TRUE     成功
               FALSE   失败
备注:
*************************************************************************/
//bool MSG_MSG_GetProcessFunc(unsigned short usMID,unsigned short usMsgType,MSG_PROCESS_FUNC *pfnProcessFunc,ST_HookEntry **pstHookTable)
bool MSG_MSG_GetProcessFunc(unsigned short usMID,unsigned short usMsgType,ST_MSG_DATA **pMsgData)
{
//    ST_MSG_DATA *pstMsgData;
    int i = MID2INDEX(usMID);
    hashtable *pHash = g_Module_Table[i].pHashModuleFuncTable;

    if (NULL == pHash)
        return false;

    ST_MSG_KEY stKey;
    stKey.usMsgType = usMsgType;

    *pMsgData = (ST_MSG_DATA *)hash_value(&stKey,pHash);

    return true;

#if 0
    /*modify for update message*/
    if (!pstMsgData)
    {
        *pfnProcessFunc = NULL;
        *pstHookTable = NULL;
    }
    else
    {
        *pfnProcessFunc = pstMsgData->pfCustomProcess;
        *pstHookTable = pstMsgData->pstHookTable;
    }
#endif
}

/*************************************************************************
功能:   获取模块的cache
参数:   unsigned short usMID               模块MID
返回:   struct list_head *                      模块对应的cache
备注:
*************************************************************************/
struct list_head * MSG_MSG_GetModuleCache(unsigned short usMID)
{
    int i = MID2INDEX(usMID);
    return &(g_Module_Table[i].listCache);
}

int DumpEntry(void *pKey, void *pData)
{
    if (pKey)
        printf("Msg Type is %d  -------\r\n",((ST_MSG_KEY *)pKey)->usMsgType);

    if (pData)
        printf("process func is %p,hook table is %p\r\n",((ST_MSG_DATA *)pData)->pfCustomProcess,((ST_MSG_DATA *)pData)->pstHookTable);

    return 1;
}

extern unsigned char s_ucMsgInited;
extern unsigned char s_ucMsgPid;
extern unsigned char s_ucMsgNeedExit;

/*************************************************************************
功能:   模块注册消息处理表的函数
参数:   unsigned short usMID                                模块MID
               ST_MSG_MAPPING *pstMsgMapping          消息处理映射表
返回:   TBS_SUCCESS             成功
               其它                        失败
备注:
*************************************************************************/
MSG_RET MSG_MSG_RegModule(unsigned short usMID, ST_MSG_MAPPING *pstMsgMapping)
{
    unsigned char ucPid = 0;
    MSG_RET ret = MSG_OK;

    ST_MSG_MAPPING *pstMsgMapEntry = pstMsgMapping;


    /* 分离出PID */
    ucPid = MID2PID(usMID);

    if (!s_ucMsgInited)     /* 当前未初始化 */
    {
        /* 按照pid初始化 */
        ret = MSG_Init(ucPid);
        if (MSG_OK != ret)
        {
            MSG_ERR(ret, "MID: %04x", usMID);
            return ret;
        }

        s_ucMsgInited = 1;   /* 全局pid的修改已经在 MSG_Init 中了 */
    }
    else
    {
        if (ucPid != s_ucMsgPid)     /* pid 不一致 */
        {
            /* 返回错误 */
            MSG_ERR(ERR_MSG_MID_INVALID, "MID: %04x", usMID);
            return ERR_MSG_MID_INVALID;
        }
    }

    if (NULL != pstMsgMapEntry)
    {
        if (!MSG_MSG_AddMsgFunc(usMID,pstMsgMapping))
        {
            MSG_ERR(ERR_MSG_MALLOC_FAIL, "MID: %04x", usMID);
        }
    }

    // MSG_MSG_DumpHashTable();

    if (!ModuleIsInList(usMID))
        AddModule2List(usMID);

    return ret;
}

/*************************************************************************
功能:   模块注销消息处理表的函数
参数:   unsigned short usMID                                模块MID
               ST_MSG_MAPPING *pstMsgMapping          消息处理映射表
返回:   TBS_SUCCESS             成功
               其它                        失败
备注:
*************************************************************************/
MSG_RET MSG_MSG_UnregModule(unsigned short usMID, ST_MSG_MAPPING *pstMsgMapping)
{
    unsigned char ucPid = 0;
    unsigned char ucAllReged = 0;

    /* 检查pid是否符合 */
    ucPid = MID2PID(usMID);
    if (ucPid != s_ucMsgPid)     /* pid 不一致 */
    {
        /* 返回错误 */
        MSG_ERR(ERR_MSG_MID_INVALID, "MID: %04x", usMID);
        return ERR_MSG_MID_INVALID;
    }

    if (NULL != pstMsgMapping)
    {
        if (MSG_MSG_DelMsgFunc(usMID))
        {
            return ERR_MSG_MALLOC_FAIL;
        }
    }

    ucAllReged = MSG_MSG_IsAllUnreged();
    if (ucAllReged)
    {
        /* MSG模块结束 */
        s_ucMsgNeedExit = 1;
        (void)MSG_MSG_Uninit();
    }

    RemoveModuleFromList(usMID);

    return MSG_OK;
}

/*************************************************************************
功能:   新消息调度处理部分，嵌入在老的消息调度函数中
参数:   ST_MSG_NODE *pstMsgNode          消息体
返回:   无
备注:
*************************************************************************/
void MSG_MSG_Dispatch(ST_MSG_NODE *pstMsgNode)
{
    unsigned short usMID = 0;
    unsigned short usModuleMID = 0;
    unsigned short usMsgType = 0;
    unsigned char i=0;
    unsigned char ucLowMID=0;

//    MSG_PROCESS_FUNC pfnProcess = NULL;
//    ST_HookEntry *pstHookTable = NULL;

    ST_MSG_DATA *pstMsgData = NULL;

    usMsgType = pstMsgNode->stMsg.stMsgHead.usMsgType;
    usMID = pstMsgNode->stMsg.stMsgHead.usDstMID;
    ucLowMID = MID2INDEX(usMID);

    /* 广播与组播*/
    if ((FLG_MID_GRP == ucLowMID)||(ucLowMID >= MID_GRP_BASE))
    {
        for (i = 0; i < GetModuleListCount(); i++)
        {
            MSG_CUR_MID(pstMsgNode->stMsg.stMsgHead.usDstMID);
            usModuleMID = GetModuleID(i);

            if (MSG_MSG_GetProcessFunc(usModuleMID,usMsgType,&pstMsgData))
            {
                if (NULL != pstMsgData)
                {
                     MSG_LOG("[NEW_MSG_Dspt_GRP]: %04x %04x %08lx %04x %lu MID: %04x",
                              pstMsgNode->stMsg.stMsgHead.usSrcMID, pstMsgNode->stMsg.stMsgHead.usDstMID,
                              pstMsgNode->stMsg.stMsgHead.ulMsgID, pstMsgNode->stMsg.stMsgHead.usMsgType,
                              pstMsgNode->stMsg.stMsgHead.ulBodyLength, usModuleMID);
                }


                Call_Msg_ProcFunc(pstMsgData,&(pstMsgNode->stMsg));


                if (NULL != pstMsgData)
                {
                    MSG_LOG("[NEW_MSG_Dspt_GRP]: %04x %04x %08lx %04x %lu MID: %04x ...... ok",
                             pstMsgNode->stMsg.stMsgHead.usSrcMID, pstMsgNode->stMsg.stMsgHead.usDstMID,
                             pstMsgNode->stMsg.stMsgHead.ulMsgID, pstMsgNode->stMsg.stMsgHead.usMsgType,
                             pstMsgNode->stMsg.stMsgHead.ulBodyLength, usModuleMID);
                }
            }
        }

        MSG_LOG("[NEW_MSG_Dspt_GRP] OK!");
    }
    else   /* 单播 */
    {
        MSG_CUR_MID(pstMsgNode->stMsg.stMsgHead.usDstMID);
        if (MSG_MSG_GetProcessFunc(usMID,usMsgType,&pstMsgData))
        {
            MSG_LOG("[NEW_MSG_Dspt]: %04x %04x %08lx %04x %lu",
                     pstMsgNode->stMsg.stMsgHead.usSrcMID, pstMsgNode->stMsg.stMsgHead.usDstMID,
                     pstMsgNode->stMsg.stMsgHead.ulMsgID, pstMsgNode->stMsg.stMsgHead.usMsgType,
                     pstMsgNode->stMsg.stMsgHead.ulBodyLength);

            Call_Msg_ProcFunc(pstMsgData,&(pstMsgNode->stMsg));

            MSG_LOG("[NEW_MSG_Dspt]: %04x %04x %08lx %04x %lu ...... ok",
                     pstMsgNode->stMsg.stMsgHead.usSrcMID, pstMsgNode->stMsg.stMsgHead.usDstMID,
                     pstMsgNode->stMsg.stMsgHead.ulMsgID, pstMsgNode->stMsg.stMsgHead.usMsgType,
                     pstMsgNode->stMsg.stMsgHead.ulBodyLength);
        }

    }

    MSG_CUR_MID((s_ucMsgPid << 8));
}

/*************************************************************************
功能:   输出对应的模块消息映射表
参数:   unsigned short usMID         MID
返回:   无
备注:
*************************************************************************/
void MSG_MSG_DumpHashTable(unsigned short usMID)
{
    int i = MID2INDEX(usMID);
    hashtable *pHash = g_Module_Table[i].pHashModuleFuncTable;

    printf("Module ID is 0x%04x  -------\r\n",usMID);

    hash_for_each_do(pHash,DumpEntry);
}

/*test code */
#if 0
void main()
{
    MSG_PROCESS_FUNC pfnProcessFunc = NULL;
    void *pstHookTable = NULL;

    Msg_MSG_InitMsgFuncHash();

    printf("full test result is %d\n",MSG_MSG_IsAllUnreged());
    printf("find 1,1 result is %d\n",MSG_MSG_IsMsgReged(1,1));

    MSG_MSG_AddMsgFunc(1,1,1,1);
    printf("find 1,1 result is %d\n",MSG_MSG_IsMsgReged(1,1));
    MSG_MSG_DumpHashTable();

    MSG_MSG_AddMsgFunc(2,2,2,2);
    MSG_MSG_DumpHashTable();

    MSG_MSG_DelMsgFunc(1,1);
    MSG_MSG_DumpHashTable();

    printf("delete result is %d\n",MSG_MSG_DelMsgFunc(1,1));
    MSG_MSG_DumpHashTable();

    printf("find 1,1 result is %d\n",MSG_MSG_IsMsgReged(1,1));

    MSG_MSG_GetProcessFunc(2,2,&pfnProcessFunc,&pstHookTable);
    printf("function is %p,hook table is %p\n",pfnProcessFunc,pstHookTable);

    MSG_MSG_UninitMsgFuncHash();

}

#endif

