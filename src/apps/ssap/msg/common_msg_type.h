#ifndef _COMMON_MSG_TYPE_
#define _COMMON_MSG_TYPE_

#include <tbsutil.h>

/****************hook table define and function*******************/

typedef enum
{
    /* SET MSG FUNCTION */
    CHECK_FUNC = 1,                                         /*�ڵ��麯��*/
    CHECK_CONFLICT_FUNC,                             /*�ڵ�ֵ��ͻ��麯��*/

    /* COMMIT MSG FUNCTION */
    APPLY_PRE_FUNC,
    APPLY_FUNC,                                               /*��Ч����*/
    APPLY_EACH_FUNC,                                 /*ÿ���ڵ㶼��Ч����Чһ�麯��*/
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
    ADD_GET_INDEX_FUNC,                                       /**/
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

typedef int (*PreApplyFunc)(void); //APPLY_PRE_FUNC
typedef int (*ApplyFunc)(char *,char*);//APPLY_FUNC
typedef int (*PostApplyFunc)(void);//APPLY_POST_FUNC
#ifdef MODIFY_CFG
typedef int (*ModifyCFGFunc)(char *,char *);//MODIFY_CFG_FUNC
#endif

typedef int (*CheckDeleteFunc)(char *);//DELETE_CHECK_FUNC
typedef int (*DeleteFunc)(char *);//DELETE_FUNC

typedef int(*CheckAddFunc)(char *);//ADD_CHECK_FUNC
typedef int(*GetAddIndex)(char *, unsigned long *);//ADD_CHECK_FUNC
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

/*******************************cache function****************************/
void InitCache(struct list_head * pCacheList);
void AddCacheNode(struct list_head * pCacheList,char *pszPath,char *pszValue);
void SetCacheNode(struct list_head * pCacheList,char *pszPath,char *pszValue);
void DeleteCacheNode(struct list_head * pCacheList,char*pszPath);
char * GetCacheNode(struct list_head * pCacheList,char *pszPath);
void CleanCache(struct list_head * pCacheList);

void SetCleanFlag(struct list_head * pCacheList,char *pszPath);
void CleanTagedNodes(struct list_head * pCacheList);

void DumpCache(struct list_head * pCacheList);

#endif

