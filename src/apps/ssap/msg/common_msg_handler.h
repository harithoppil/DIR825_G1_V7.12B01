#ifndef _COMMON_MSG_HANDLER_
#define _COMMON_MSG_HANDLER_
#include "common.h"
#include "new_msg.h"
#include <warnlog.h>
#include <common_msg_type.h>

#define MAX_FUNCTION_NUM 100
#define MAX_INDEX_NUM 5


/* ��־��ӡ�� */
#ifdef COMMON_MSG_DEBUG_SWITCH
#define COMMON_MSG_TRACE(fmt, args...)     COMMON_TRACE(0, fmt, ##args)
#else
#define COMMON_MSG_TRACE(fmt, args...)
#endif

#if 0
/****************hook table define and function*******************/

typedef enum
{
    /* SET MSG FUNCTION */
    CHECK_FUNC = 1,                                         /*�ڵ��麯��*/
    CHECK_CONFLICT_FUNC,                             /*�ڵ�ֵ��ͻ��麯��*/

    /* COMMIT MSG FUNCTION */
    APPLY_FUNC,                                               /*��Ч����*/
    APPLY_POST_FUNC,                                     /*ģ�鱣����Ϣ������������õĹ�����������Ҫ��������ģ��Ĺ�������*/
#ifdef MODIFY_CFG
    MODIFY_CFG_FUNC,                                     /*����ֵ���浽����������õĺ�������Ҫ�����������������޸�*/
#endif

    /* CANCEL MSG FUNCTION ,NO ONE NOW*/

    /* DELETE MSG FUNCTION */
    DELETE_CHECK_FUNC,                                 /*ɾ����麯��*/
    DELETE_FUNC,                                              /*ɾ����Ч����*/

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

typedef int (* CheckFunc)(char *); //CHECK_FUNC
typedef int (*CheckConflictFunc)(char *,char *);//CHECK_CONFLICT_FUNC

typedef int (*ApplyFunc)(char *,char*);//APPLY_FUNC
typedef int (*PostApplyFunc)(void);//APPLY_POST_FUNC
#ifdef MODIFY_CFG
typedef int (*ModifyCFGFunc)(char *,char *);//MODIFY_CFG_FUNC
#endif

typedef int (*CheckDeleteFunc)(char *);//DELETE_CHECK_FUNC
typedef int (*DeleteFunc)(char *);//DELETE_FUNC

typedef int(*CheckAddFunc)(char *);//ADD_CHECK_FUNC
typedef int (*AddFunc)(char *,unsigned long);//ADD_FUNC

typedef int (*UpdateFunc)(void);//UPDATE_FUNC

typedef int (*AddedFunc)(char *);//ADDED_FUNC

typedef int (*DeletedFunc)(char *);//DELETED_FUNC

typedef struct tag_ST_HookEntry
{
    char *pNodePath;
    HookType eHookType;
    void *pHookFunc;
}ST_HookEntry;
#endif

typedef int (*ProcessFunc)(void);


void CommonSetNeedReboot(void);
/*******************msg common handler function**************************/

int CommonSetMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData);
int CommonCommitMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData);
int CommonCancelMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData);
int CommonDeleteMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData);
int CommonAddMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData);
int CommonUpdateMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData);
int CommonDeletedMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData);
int CommonAddedMsgHandler(ST_MSG *pstMsg,ST_MSG_DATA *pstMsgData);

/*****************************extern variable******************************/
extern struct list_head g_cache_list;

void *GetHookFunc(char *pcNodePath,HookType eHookType,ST_MSG_DATA *pstMsgData);

char * GetCacheNodeEx(char *pszPath);

int ApplyAccessor(unsigned short usMID,char *pszPath,char *pszValue,ST_MSG_DATA *pstMsgData);
void SaveCache2CFG(struct list_head *head);

typedef int (*CheckConflictAccessorFunc)(unsigned short usMID,char *pszPath,char *pszValue,ST_MSG_DATA *pstMsgData);
//int CacheList_ForEachOnce(unsigned short usMid,struct list_head *head,ST_HookEntry * pstSetHookTable, CheckConflictAccessorFunc pAccessor);
int CacheList_ForEachOnce(unsigned short usMID,struct list_head *head,ST_MSG_DATA *pstMsgData, CheckConflictAccessorFunc pAccessor);

int ParseSetCmdSimple(const char *pszSetCmd,char *pszPath,unsigned int *pnPathLen,char *pszValue,unsigned int *pnValueLen);

int CallPostApplyFuncInHash(void *key,void *data);

#endif
