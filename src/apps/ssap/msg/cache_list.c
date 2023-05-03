/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ�����: cache_list.c
 �ļ�����: ��������Ĵ�����

 �޶���¼:
        1. ����: ��ΰ
           ����: 2008-08-07
           ����: �����ļ�

**********************************************************************/
#include "cache_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Free_Clean(a) if(a){free(a);a=NULL;}

/**************************************************************************
����: ��ʼ�������
����: struct list_head *head,                 ��������ͷ;
����: ��
��ע:
***************************************************************************/
void CacheList_Init(struct list_head *head)
{
    INIT_LIST_HEAD(head);
}

void CacheList_PrintAll(struct list_head *head)
{
    struct list_head *ptr;
    ST_Cache_Entry *l;

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Cache_Entry, head);
        if(l)
        {
        printf("path is %s \n", l->pszPath);
        printf("value is %s \n", l->pszValue);
        printf("clean flag is %d\n",l->cCleanFlag);
        printf("previous is %p\n",l->head.prev);
        printf("next is %p\n",l->head.next);
        }
    }
}

/**************************************************************************
����: ��ջ����
����: struct list_head *head,                 ��������ͷ;
����: ��
��ע:
***************************************************************************/
void CacheList_DropAll(struct list_head *head)
{
    ST_Cache_Entry *l = NULL;

    while (!list_empty(head))
    {
        l = list_entry(head->next, ST_Cache_Entry, head);
        Free_Clean(l->pszValue);
        Free_Clean(l->pszPath);
        list_del(&l->head);
        free(l);
    }
}

/**************************************************************************
����: ���뵽��ͷ
����: struct list_head *head,                 ��������ͷ;
             char *pszPath,                              ·�� ;
             char *pszValue                             ֵ
����: ��
��ע:
***************************************************************************/
void CacheList_InsertHead(struct list_head *head, char *pszPath, char *pszValue)
{
    ST_Cache_Entry *l = (ST_Cache_Entry*)malloc(sizeof(ST_Cache_Entry));
    l->pszPath=(char *)malloc(strlen(pszPath)+1);
    l->pszValue=(char *)malloc(strlen(pszValue)+1);
    strcpy(l->pszValue,pszValue);
    strcpy(l->pszPath,pszPath);
    l->cCleanFlag = VALID;
    list_add(&l->head, head);
}

/**************************************************************************
����: ���뵽��β
����: struct list_head *head,                 ��������ͷ;
             char *pszPath,                              ·�� ;
             char *pszValue                             ֵ
����: ��
��ע:
***************************************************************************/
void CacheList_AddTail(struct list_head *head, char *pszPath, char *pszValue)
{
    ST_Cache_Entry *l = (ST_Cache_Entry*)malloc(sizeof(ST_Cache_Entry));
    l->pszPath=(char *)malloc(strlen(pszPath)+1);
    l->pszValue=(char *)malloc(strlen(pszValue)+1);
    strcpy(l->pszValue,pszValue);
    strcpy(l->pszPath,pszPath);
    l->cCleanFlag = VALID;

    list_add_tail(&l->head, head);
}

/**************************************************************************
����: ɾ��һ��ʵ��
����: struct list_head *head,                 ��������ͷ;
             char *pszPath,                              ·�� ;
����: ��
��ע:
***************************************************************************/
void CacheList_DeleteEntry(struct list_head *head, char *pszPath)
{
    ST_Cache_Entry *l = NULL;
    struct list_head *ptr;

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Cache_Entry, head);
        if (!(strcmp(l->pszPath,pszPath)))
        {
        	  Free_Clean(l->pszValue);
        		Free_Clean(l->pszPath);
            list_del(&l->head);
            free(l);
            return;
        }
    }
}

/**************************************************************************
����: ��ȡֵ
����: struct list_head *head,                 ��������ͷ;
             char *pszPath,                              ·�� ;
����: ��
��ע:
***************************************************************************/
char *CacheList_GetValue(struct list_head *head, char *pszPath)
{
    ST_Cache_Entry *l = NULL;
    struct list_head *ptr;

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Cache_Entry, head);
        if (!(strcmp(l->pszPath,pszPath)))
        {
            return l->pszValue;
        }
    }

    return NULL;
}

/**************************************************************************
����: ����ֵ
����: struct list_head *head,                 ��������ͷ;
             char *pszPath,                              ·�� ;
             char *pszValue                             ֵ
����: ��
��ע:
***************************************************************************/
void CacheList_SetEntry(struct list_head *head,char *pszPath,char *pszValue)
{
    ST_Cache_Entry *l = NULL;
    struct list_head *ptr;

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Cache_Entry, head);
        if (!(strcmp(l->pszPath,pszPath)))
        {
            if (strcmp(l->pszValue, pszValue))
            {
                free(l->pszValue);
                l->pszValue=(char *)malloc(strlen(pszValue)+1);
                strcpy(l->pszValue,pszValue);
            }
            return;
        }
    }

    CacheList_AddTail(head,pszPath,pszValue);
}

/**************************************************************************
����: �ж�ĳ��·���Ƿ���cache��
����: struct list_head *head,                 ��������ͷ;
             char *pszPath,                              ·�� ;
����: int
             true  ��cache��
             flase ����cache��
��ע:
***************************************************************************/
int CacheList_IsInList(struct list_head *head, char *pszPath, BOOL bIsFullMatch)
{
    ST_Cache_Entry *l = NULL;
    struct list_head *ptr;

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Cache_Entry, head);
        if ((bIsFullMatch == TRUE && !strcmp(l->pszPath,pszPath)) ||
            (bIsFullMatch == FALSE && strstr(l->pszPath, pszPath)))
        {
            return 1;
        }
    }

    return 0;
}

/**************************************************************************
����: ����Ҫ������
����: struct list_head *head,                 ��������ͷ;
             char *pszPath,                              ·�� ;
����: ��
��ע:
***************************************************************************/
void CacheList_SetCleanFlag(struct list_head *head, char *pszPath)
{
    ST_Cache_Entry *l = NULL;
    struct list_head *ptr;

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Cache_Entry, head);
        if (!(strcmp(l->pszPath,pszPath)))
        {
            l->cCleanFlag = CLEAN;
        }
    }
}

/**************************************************************************
����: ��������ѱ�ǵĽڵ�
����: struct list_head *head,                 ��������ͷ;
             char *pszPath,                              ·�� ;
����: ��
��ע:
***************************************************************************/
void CacheList_DropAllWithCleanFlag(struct list_head *head)
{
    ST_Cache_Entry *l = NULL;
    struct list_head *ptr;

    list_for_each(ptr, head)
    {
        l = list_entry(ptr, ST_Cache_Entry, head);
        CacheList_PrintAll(head);
        if (CLEAN == l->cCleanFlag)
        {
            Free_Clean(l->pszValue);
            Free_Clean(l->pszPath);
            ptr = l->head.prev;
            list_del(&l->head);
            free(l);
            CacheList_PrintAll(head);
        }
    }
}

