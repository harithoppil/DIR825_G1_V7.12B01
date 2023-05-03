/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称: cache_list.c
 文件描述: 缓存链表的处理函数

 修订记录:
        1. 作者: 李伟
           日期: 2008-08-07
           内容: 创建文件

**********************************************************************/
#include "cache_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Free_Clean(a) if(a){free(a);a=NULL;}

/**************************************************************************
功能: 初始化缓存表
参数: struct list_head *head,                 缓存链表头;
返回: 无
备注:
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
功能: 清空缓存表
参数: struct list_head *head,                 缓存链表头;
返回: 无
备注:
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
功能: 插入到表头
参数: struct list_head *head,                 缓存链表头;
             char *pszPath,                              路径 ;
             char *pszValue                             值
返回: 无
备注:
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
功能: 插入到表尾
参数: struct list_head *head,                 缓存链表头;
             char *pszPath,                              路径 ;
             char *pszValue                             值
返回: 无
备注:
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
功能: 删除一个实例
参数: struct list_head *head,                 缓存链表头;
             char *pszPath,                              路径 ;
返回: 无
备注:
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
功能: 获取值
参数: struct list_head *head,                 缓存链表头;
             char *pszPath,                              路径 ;
返回: 无
备注:
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
功能: 设置值
参数: struct list_head *head,                 缓存链表头;
             char *pszPath,                              路径 ;
             char *pszValue                             值
返回: 无
备注:
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
功能: 判断某个路径是否在cache中
参数: struct list_head *head,                 缓存链表头;
             char *pszPath,                              路径 ;
返回: int
             true  在cache中
             flase 不在cache中
备注:
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
功能: 设置要清除标记
参数: struct list_head *head,                 缓存链表头;
             char *pszPath,                              路径 ;
返回: 无
备注:
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
功能: 清除所有已标记的节点
参数: struct list_head *head,                 缓存链表头;
             char *pszPath,                              路径 ;
返回: 无
备注:
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

