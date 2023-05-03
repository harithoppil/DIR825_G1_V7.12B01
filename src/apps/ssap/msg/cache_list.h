/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : wanpath_list.h
 �ļ����� : �豸������������Ӧ�Ķ���ͷ�ļ���ʹ��normal_list.h��ͨ�������װ


 �����б� :

 �޶���¼ :
          1 ���� : ��ΰ
            ʱ�� : 2008-8-27
            ���� :

**********************************************************************/
#ifndef _WANPATH_LIST_H_
#define _WANPATH_LIST_H_

#include "tbsutil.h"

typedef enum tagEN_CACHE_CLEAN_FLAG
{
    VALID = 0,
    CLEAN
}EN_ACL_INST_TYPE;

typedef struct tag_ST_CONNPATH_DEVNAME{
	struct list_head head;
	char	*pszPath;
	char *pszValue;
	char cCleanFlag;
}ST_Cache_Entry;


/* ����ʹ�ú����������÷���devname_list.c������ */
void CacheList_Init(struct list_head *head);
void CacheList_PrintAll(struct list_head *head);
void CacheList_DropAll(struct list_head *head);
void CacheList_InsertHead(struct list_head *head, char *pszPath, char *pszValue);
void CacheList_AddTail(struct list_head *head, char *pszPath, char *pszValue);
void CacheList_DeleteEntry(struct list_head *head, char *pszValue);
char *CacheList_GetValue(struct list_head *head, char *pszPath);
int CacheList_IsInList(struct list_head *head, char *pszPath, BOOL bIsFullMatch);
void CacheList_SetEntry(struct list_head *head,char *pszPath,char *pszValue);

void CacheList_SetCleanFlag(struct list_head *head, char *pszPath);
void CacheList_DropAllWithCleanFlag(struct list_head *head);

#endif

