/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : common_func.h
 �ļ����� : ���ģ�鴦��Ĺ���������װ

 �޶���¼ :
          1 ���� : ��ΰ
            ���� : 2008-9-1
            ���� : �����÷��뿴lan.c
**********************************************************************/

#ifndef _COMMON_FUNC_
#define _COMMON_FUNC_

#include <autoconf.h>
#include <tbsmsg.h>
#include <sys/time.h>
#include <unistd.h>

typedef enum
{
    FROM_CACHE,
    FROM_CFG,
    CACHE_CFG,
}ValuePostionType;

/*�ڵ�ֵ��Ŀ*/
typedef struct {
    char *pszNodeName;
    char *pszNodeValue;
} ST_SET_NODE_VAL_ENTRY;

/******************************************************************************
                                                        common macro
******************************************************************************/
#define NONE          "\033[m"
#define LIGHT_RED "\033[0;31m"
#define GREEN         "\033[0;32m"

/* �ն���ʹ�õ���ɫ�������� */
#define ECHO_NONE           "\033[0m"

#define ECHO_RED            "\033[0;31m"
#define ECHO_GREEN          "\033[0;32m"
#define ECHO_YELLOW         "\033[0;33m"
#define ECHO_BLUE           "\033[0;34m"
#define ECHO_PURPLE         "\033[0;35m"

/* ��˸ */
#define ECHO_FLASH_RED      "\033[5;31m"
#define ECHO_FLASH_GREEN    "\033[5;32m"
#define ECHO_FLASH_YELLOW   "\033[5;33m"
#define ECHO_FLASH_BLUE     "\033[5;34m"
#define ECHO_FLASH_PURPLE   "\033[5;35m"

#define ATTR_STAT            "Stat"
#define ATTR_CACHE           "Cache"
#define ATTR_WRITABLE        "Writable"
#define FM_PATH				 "InternetGatewayDevice.DeviceInfo.X_TWSZ-COM_FactoryMode"


//typedef  int BOOL;

#define RET_GOTO(module,Ret,strError,gotoTag)         \
      {\
        if ( RET_FAILED(Ret) )    \
        {   \
            COMMON_TRACE(module,strError); \
            goto gotoTag; \
        }\
      }
#define POINTER_GOTO(module, Pointer, gotoTag, strError, args...)         \
    {\
        if ( !Pointer)    \
        {   \
            COMMON_TRACE(module, strError, ##args); \
            goto gotoTag; \
        }\
     }



#define RET_FALSE(module,Ret,strError,args...)         \
    {\
        if ( RET_FAILED(Ret) )    \
        {   \
            COMMON_TRACE(module, strError, ##args); \
            return false; \
        }\
     }

#define RET_RETURN(module,Ret,strError,args...)         \
    {\
        if ( RET_FAILED(Ret) )    \
        {   \
            COMMON_TRACE(module, strError, ##args); \
            return Ret; \
        }\
    }
#define RET_LOG(module,Ret,strError,args...)         \
    {\
        if ( RET_FAILED(Ret) )    \
        {   \
            COMMON_TRACE(module, strError, ##args); \
        }\
    }

#define PTR_RETURN(module,Pointer,Ret,strError,args...)         \
    {\
        if ( !Pointer)    \
        {   \
            COMMON_TRACE(module, strError, ##args); \
            return Ret; \
        }\
     }

#define PTR_FALSE(module,Pointer,strError,args...)         \
    {\
        if ( !Pointer)    \
        {   \
            COMMON_TRACE(module, strError, ##args); \
            return FALSE; \
        }\
    }
#define PTR_LOG(module,Pointer,strError,args...)         \
    {\
        if ( !Pointer)    \
        {   \
            COMMON_TRACE(module, strError, ##args); \
        }\
    }


#define POINTER_RETURN(module,Pointer,strError,args...)         \
    {\
        if ( !Pointer)    \
        {   \
            COMMON_TRACE(module, strError, ##args); \
            return TBS_NULL_PTR; \
        }\
     }

#define POINTER_FALSE(module,Pointer,strError,args...)         \
    {\
        if ( !Pointer)    \
        {   \
            COMMON_TRACE(module, strError, ##args); \
            return FALSE; \
        }\
    }
#define POINTER_LOG(module,Pointer,strError,args...)         \
    {\
        if ( !Pointer)    \
        {   \
            COMMON_TRACE(module, strError, ##args); \
        }\
    }

#define RET_VAL_IF_FAIL(module, con, ret)   \
    {\
        if ( !(con) ) \
        {\
            COMMON_TRACE(module, "%s failed\n", #con); \
            return ret; \
        }\
    }

#define RET_IF_FAIL(module, con)   \
    {\
        if ( !(con) ) \
        {\
            COMMON_TRACE(module, "%s failed\n", #con); \
            return; \
        }\
    }

#define LOG_IF_FAIL(module, con)   \
    {\
        if ( !(con) ) \
        {\
            COMMON_TRACE(module, "%s failed\n", #con); \
        }\
    }

#define CALL_FUCTION_FAILED     "Call function \"%s\" failed\n"


/******************************************************************************
                                                        common function
******************************************************************************/
int COMM_NullFunc(char *pszPath);


/*
ȥ��·������'.'
ע��:�˺������޸Ĵ����ָ������
*/
void COMM_RemovePathLastDot(char *pszPath);


/*
����·��,����ʱȥ��·������'.'
*/
void COMM_CopyPathNoLastDot(char *pszDestPath, const char *pszSrcPath);


/*************************************************************************
Function:    char *  COMM_GetPathNode(char *pszCurPath,int iPos)
Description:   ��ȡ·���еĵ�n ���ڵ�,
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iPos,                                        ��ȡ�ڼ����ڵ�
Output:        ��
Return:        NULL ,       ʧ��;
                    ����,     �ɹ���ֵ��ָ��
Others:         Ϊ�˼򻯽ӿڣ�����ʹ����ȫ�ֱ����洢����ֵ��
                    ���Բ�֧�ֶ��̵߳Ĳ�������
*************************************************************************/
const char * COMM_GetPathNode(char *pszCurPath,int iPos);

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
int  COMM_GetPathIndex(char *pszCurPath,int *iIndex);

/*************************************************************************
Function:      int GetPathFromCurrentPath(char *pszCurPath,int iChangeLevel,char *pszExtraPath,char *pszDestPath,unsigned long *pulPathLen)
Description:   ͨ�����е�·��ƴװ��·��
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iChangeLevel,                         �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
                   char *pszExtraPath                     ���� ��·����ΪNULL���ʾ�����
Output:        ��
Return:        NULL ,       ʧ��;
                    ����,     �ɹ���ֵ��ָ��
Others:
*************************************************************************/
int COMM_GetPathEx(const char *pszCurPath,int iChangeLevel,
        const char *pszExtraPath,char *pszDestPath,unsigned long *pulPathLen);



/*************************************************************************
Function:      char * COMM_GetNodeValue(char *pszPath,ValuePostionType *eGetMode)
Description:   ͳһ�Ļ�ȡֵ�ĺ����������ӻ����ȡ������������ȡ���ȴӻ��������������ַ�ʽ
Input:         char *pszPath ,                       ��ǰ�����·��
                   ValuePostionType eGetMode  ��ȡֵ�÷�ʽ
                                                                   (    FROM_CACHE �ӻ���, FROM_CFG ��������,  CACHE_CFG �Ȼ����������)
Output:        ValuePostionType eGetMode ��ʲô�ط���ȡ����ֵ
Return:        NULL ,       ʧ��;
                    ����,     �ɹ���ֵ��ָ��
Others:
*************************************************************************/
const char * COMM_GetNodeValue(char *pszPath,ValuePostionType eGetMode);
ValuePostionType COMM_GetLastPos();/*�������һ��COMM_GetNodeValue������ȡֵ��λ��*/


/*************************************************************************
Function:    char * COMM_GetNodeValueEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath,ValuePostionType eGetMode)
Description:  ��ȡֵ�ķ�װ�ӿڣ�֧��:��·������ͨ��������·��ƴװ���
                    1.�����ȡֵ��·������ͨ��������·��ƴװ���
                    2.֧�ִӻ��棬�����������Ȼ�����������ȼ��ַ�ʽ��ýڵ�ֵ
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iChangeLevel,                         �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
                   char *pszExtraPath                     ���� ��·����ΪNULL���ʾ�����
                   ValuePostionType eGetMode        ��ȡֵ�ķ�ʽ
                                                                         (    FROM_CACHE �ӻ���, FROM_CFG ��������,  CACHE_CFG �Ȼ����������)
Output:        ��
Return:        NULL ,       ʧ��;
                    ����,     �ɹ���ֵ��ָ��
Others:
*************************************************************************/
const char * COMM_GetNodeValueEx(const char *pszCurPath,int iChangeLevel,
                const char *pszExtraPath,ValuePostionType eGetMode);


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
    ValuePostionType eGetMode, unsigned short usNodeCount, ...);



/*
ͨ������ڵ�����������ȡ����ڵ�ֵ��֧�ֶ�·��ƴװ
1.�����ȡֵ��·������ͨ��������·��ƴװ���
2.֧�ִӻ��棬�����������Ȼ�����������ȼ��ַ�ʽ��ýڵ�ֵ
*/
int COMM_GetMultiNodeValueByStrArrEx(const char *pszCurPath, int iChangeLevel,
        ValuePostionType eGetMode, unsigned short usNodeCount,
        const char *apcNodeArr[], void *pvNodeValArr);
/*
ͨ������ڵ�������ͽڵ�ֵ������ͬʱ���ö���ڵ�ֵ, ֧�ֶ�·��ƴװ
1.�����ȡֵ��·������ͨ��������·��ƴװ���
*/
int COMM_SetMultiNodeValueByStrArrEx(const char *pszCurPath, int iChangeLevel,
        unsigned short usNodeCount, const char *apcNodeArr[], void *pvNodeValArr);


/*************************************************************************
Function:    char * COMM_GetAndSynValueEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath,ValuePostionType eGetMode)
Description:  ��ȡֵ������ͬ����ǣ�ͬ����Ǳ�ʾ��ӻ�����ȥ������commit�����ȵ���
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iChangeLevel,                         �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
                   char *pszExtraPath                     ���� ��·����ΪNULL���ʾ�����
                   ValuePostionType eGetMode        ��ȡֵ�ķ�ʽ
                                                                        (    FROM_CACHE �ӻ���, FROM_CFG ��������,  CACHE_CFG �Ȼ����������)
Output:        ��
Return:        NULL ,       ʧ��;
                    ����,     �ɹ���ֵ��ָ��
Others:
*************************************************************************/
const char * COMM_GetAndSynValueEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath,ValuePostionType eGetMode);

/*************************************************************************
Function:    int  COMM_SetValueEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath,char *pszValue)
Description:  ����ֵ�ķ�װ�ӿڣ�֧��:��·������ͨ��������·��ƴװ���
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   int iChangeLevel,                         �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
                   char *pszExtraPath                     ���� ��·����ΪNULL���ʾ�����
                   char *pszValue                            ���õ�ֵ
Output:        ��
Return:        TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:
*************************************************************************/
int COMM_SetValueEx(const char *pszCurPath,int iChangeLevel,char *pszExtraPath, const char *pszValue);


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
int COMM_SetMultiNodeValueEx(const char *pszCurPath, int iChangeLevel, unsigned short usNodeCount, ...);

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
                              int iChangeLevel, unsigned short usNodeCount, ...);

/*************************************************************************
Function:       COMM_GetMultiCacheValueEx
Description:    ��ȡ����ڵ㻺��ֵ�ķ�װ�ӿ�
                1.�����ȡ����ֵ��·������ͨ��������·��ƴװ���
                2.֧�ִӻ���,��������,�Ȼ�����������ȼ��ַ�ʽ��ýڵ�ֵ

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
                              int iChangeLevel, unsigned short usNodeCount, ...);

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
                           int iChangeLevel, unsigned short usNodeCount, ...);

/*************************************************************************
Function:    int COMM_SetCacheValueEx(unsigned short usMID,char *pszCurPath,int iChangeLevel,char *pszExtraPath,char *pszValue)
Description:  ���û���ֵ�ķ�װ�ӿڣ�֧��:��������ֵ��·������ͨ��������·��ƴװ���
Calls:         ��
Data Accessed:
Data Updated:
Input:     unsigned short            ����ģ���MID
               char *pszCurPath,       ��ǰ�����·��
               int iChangeLevel,        �ı��·������(0��ֵ����ʾ��·���ұ�ȥ���Ľڵ���)
               char *pszExtraPath    ���ӵ�·����ΪNULL���ʾ�����
               char *pszValue           ���õ�ֵ
Output:        ��
Return:        TBS_SUCCESS ,      �ɹ�;
               ����,     ʧ��
Others:
*************************************************************************/
int COMM_SetCacheValueEx(unsigned short usMID,const char *pszCurPath,int iChangeLevel,
                         char *pszExtraPath, char *pszValue);

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
                                 int iChangeLevel, char *pszExtraPath);

/*************************************************************************
Function: void COMM_SynCache2CFG(unsigned short usMID)

Description:  ��ģ�黺��ֵ���浽��������

Input:     usMID,   ģ��MID

Return:   ��

Others:
*************************************************************************/
void COMM_SynCache2CFG(unsigned short usMID);
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
/* ���ָ��·��������ʵ���Ƿ��нڵ�ֵ��ͻ(���ڳ�ͻ��麯��) */
int COMM_CheckConflictNodes(const char *pszMatchPath, const char *pszCurPath,
        int iChangeLevel, int iNodeCnt, char * const *apcNodeArr);

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
int COMM_AddObjectEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath);

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
int COMM_DeleteObjectEx(char *pszCurPath,int iChangeLevel,char *pszExtraPath);

/* �Է���ֵ�Ĵ���ʽ */
#define CFG_FAILED_EXIT     0
#define CFG_SUCCESS_EXIT    1
#define CFG_DONOT_EXIT      2
/*************************************************************************
Function:    int CFG_CheckForEach(char *pszCurPath,char *pszCurVal,int iChangeLevel,CheckEachFunc pfnCheck)
Description:  ��ĳ��·�����б���
Input:         char *pszCurPath ,                       ��ǰ�����·�� (a.b.1.c)
                   int iChangeLevel,                         ������·���Ĳ�Σ���ʾ���ұ�ȥ���Ľڵ���
                   CheckEachFunc pfnCheck,           ��麯��
                   void *pData,                                 �����ֵ����ģ�����
                   BOOL bRet,                                    ��ʾ����ʲô״̬���أ�
                                                                        true��ʾ������һ��TBS_SUCCESS���أ�false������һ����TBS_SUCCESS����
Output:        ��
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:
*************************************************************************/
typedef int (*CheckEachFunc)(char *pszNewNodePath, void *pData, char *pszEachNodePath);
int CFG_CheckForEach(char *pszCurPath, int iChangeLevel, CheckEachFunc pfnCheck, void *pData, int bRet);

/*************************************************************************
Function:    int CFG_ForEachEx(char *pszCurPath, ForEachFunc pfnForEach, void *pData)
Description:  ��ĳ��·�����б���
Calls:         ��
Data Accessed:
Data Updated:
Input:         char *pszCurPath ,                       ��ǰ�����·��
                   ForEachFunc pfnForEach            ����ʹ�õĺ���
                   void *pData,                                �����˽������
Output:        ��
Return:       true ,      �ɹ�;
                   false,     ʧ��
Others:
*************************************************************************/
typedef int (*ForEachFunc)(char *pszNewNodePath, void *pData);
int CFG_ForEachEx(const char *pszCurPath, ForEachFunc pfnForEach, void *pData,int bRet);

/*************************************************************************
Function:    int COMM_SynNode2Cache(char *pszPath)
Description:  ��ֵͬ����������
Input:         char *pszPath ,                       ��ǰ�����·��
Output:        ��
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:
*************************************************************************/
int COMM_SynNode2Cache(unsigned short usMID,char *pszPath);

/*************************************************************************
Function:    int COMM_ApplyAllNodes(unsigned short usMID)
Description:  ��Ч��ģ�����еĽڵ�
Input:         unsigned short usMID ,                       ��ǰ��MID
Output:        ��
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:
*************************************************************************/
int COMM_ApplyAllNodes(unsigned short usMID);

/*************************************************************************
Function:    int COMM_ApplyAllNodesEx(unsigned short usMID,const char * pszPath)
Description:  ��Ч��ģ�����·�������еĽڵ�
Input:         unsigned short usMID ,                       ��ǰ��MID
                   const char * pszPath                          ��Ҫ��Ч��·���������� .����
Output:        ��
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:
*************************************************************************/
int COMM_ApplyAllNodesEx(unsigned short usMID,const char * pszPath);

/*************************************************************************
Function:    int COMM_ParseCustomMsg(ST_MSG *pstMsg,char *pszFormat, ...)
Description:  �����Զ�����Ϣ��
Calls:         ��
Data Accessed:
Data Updated:
Input:         ST_MSG *pstMsg       ��Ϣ��
                  char *pszFormat       ��Ҫ�Ľ���name�����Կո��������:"name1 name2"
Output:        ��Σ���ѽ�����ֵ���뵽�����
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:        ��������Ϣ������ name1=value1...���͵�
*************************************************************************/
int COMM_ParseCustomMsg(const ST_MSG *pstMsg, char *pszFormat, ...);

/* ��ȡmsgid */
unsigned long GetLastMsgID();

/*************************************************************************
Function:    int COMM_MakeCustomMsg(ST_MSG *pstMsg,unsigned short usMsgType,unsigned short usSrcMID,unsigned short usDstMID,unsigned short usNodeNum, ...);
Description:  �����Զ�����Ϣ��
Calls:         ��
Data Accessed:
Data Updated:
Input:         ST_MSG *pstMsg                  ��Ϣ��
                  unsigned short usMsgType  ��Ϣ����
                  unsigned short usSrcMID     ԴMID
                  unsigned short usDstMID     Ŀ��MID
                  unsigned short usNodeNum ������Ϣ�Ľڵ���
                  ���                                   �����õ��ַ���
Output:
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:       �������Ϣ������ name1=value1...���͵�
*************************************************************************/
int COMM_MakeCustomMsg(ST_MSG **ppstMsg,unsigned short usMsgType,unsigned short usSrcMID,unsigned short usDstMID,unsigned short usNodeNum, ...);

/*************************************************************************
Function:    int COMM_MakeAndSendCustomMsg(unsigned short usMsgType,unsigned short usSrcMID,unsigned short usDstMID,unsigned short usNodeNum, ...);
Description:  ���첢�����Զ�����Ϣ��
Calls:         ��
Data Accessed:
Data Updated:
Input:        unsigned short usMsgType  ��Ϣ����
                  unsigned short usSrcMID     ԴMID
                  unsigned short usDstMID     Ŀ��MID
                  unsigned short usNodeNum ������Ϣ�Ľڵ���
                  ���                                  �����õ��ַ���
Output:
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:       �������Ϣ������ name1=value1...���͵�
*************************************************************************/
int COMM_MakeAndSendCustomMsg(unsigned short usMsgType,unsigned short usSrcMID,unsigned short usDstMID,unsigned short usNodeNum, ...);

/*************************************************************************
Function:    int COMM_ResponseCustomMsg(ST_MSG *pstMsg,unsigned short usMsgType,unsigned short usNodeNum, ...)
Description:  ���첢�����Զ�����Ϣ��Ļظ���Ϣ
Calls:         ��
Data Accessed:
Data Updated:
Input:        ST_MSG *pstMsg                   ���յ�����Ϣ��
                  unsigned short usMsgType  �ظ�����Ϣ����
                  unsigned short usNodeNum ������Ϣ�Ľڵ���
                  ���                                  �����õ��ַ���
Output:
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:       �������Ϣ������ name1=value1...���͵�
*************************************************************************/
int COMM_ResponseCustomMsg(ST_MSG *pstMsg,unsigned short usMsgType,unsigned short usNodeNum, ...);

/*************************************************************************
Function:    int COMM_SendAndParseResponseMsg(ST_MSG *pstMsg,char *pszFormat, ...)
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
int COMM_SendAndParseResponseMsg(int iNeedResponse, ST_MSG *pstMsg, char *pszFormat, ...);


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
ST_MSG *COMM_MakeBinMsg(unsigned short usMsgType,unsigned short usSrcMID,unsigned short usDstMID,unsigned short usMsgLen, void *pcMsgBody);


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
Return:         TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:         �������Ϣ�������������͵Ķ����Ƹ�ʽ
*************************************************************************/
int COMM_MakeAndSendBinMsg(unsigned short usMsgType,unsigned short usSrcMID,
    unsigned short usDstMID,unsigned short usMsgLen, void *pcMsgBody);


/*************************************************************************
Function:    COMM_SetCacheValue(char *pszPath,char *pszValue);
Description:  ���û���ֵ
Calls:         ��
Data Accessed:
Data Updated:
Input:        unsigned short usMID  MID
                  char *pszPath              ·��
                  char *pszValue            ���õ�ֵ
Output:
Return:       TBS_SUCCESS ,      �ɹ�;
                    ����,     ʧ��
Others:
*************************************************************************/
int COMM_SetCacheValue(unsigned short usMID,char *pszPath,char *pszValue);

/*************************************************************************
Function:    const char *COMM_GetCacheValue(unsigned short usMID, char *pszPath)
Description:  ��ȡ�ڵ�cacheֵ

Input:      usMID      ģ���MID
            pszPath    �ڵ�·��

Return:       �ڵ㻺��ֵָ�� ,      �ɹ�;
              NULL,                 ʧ��
Others:
*************************************************************************/
const char *COMM_GetCacheValue(unsigned short usMID, char *pszPath);

/*************************************************************************
Function:    int COMM_DelCacheNode(unsigned short usMID, char *pszPath)
Description: ɾ������pszPathָ���·���ڵ�Ļ���

Input:      usMID      ģ���MID
            pszPath    �ڵ�·��

Return:       TBS_SUCCESS,      �ɹ�;
              ����,             ʧ��
Others:
*************************************************************************/
int COMM_DelCacheNode(unsigned short usMID, char *pszPath);

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
int COMM_CleanCache(unsigned short usMID);


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
unsigned short COMM_GetCacheBuildMID(unsigned short usCacheMID);

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
int COMM_SetCacheBuildMID(unsigned short usCacheMID, unsigned short usCacheBuildMID);

/**********************************************************************
         ����set ack��Ϣ�д���·������ģ���ڳ�ͻ���ʱ����
**********************************************************************/
void COMM_SetLastErrPath(char *pszPath);


/************************************************************
Function:      int COMM_SystemEx(const char *pszCmd,char * pszResult,int iCount)
Description:   ��ȡshell����ı���Ϣ��ĳЩ�е�ĳЩ�ֶΣ��÷�������strtok
Input:         const char *pszCmd,              Ҫִ�е�����
                   char * pszResult,                    ִ�е�������
                   int iCount                                ��ǰbuffer�ĳ���
Output:
Return:        false ,       ʧ��;
                    true,     �ɹ�;
Others:
************************************************************/
int COMM_SystemEx(const char *pszCmd,char * pszResult,int iCount);

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
int COMM_StrtokEx(char *szBuf,char *szDelim,int iMinLine,int iMaxLine,int iParaNum,...);

BOOL COMM_IsPathInCache(unsigned short usMID, char *pszPath);

int COMM_ParserPairValue(char *szBuf,char *szDelim,char *szName,char **pszValue);

int COMM_ParserPairValueEx(char *szBuf,char *szFirst,char *szEnd,char **pszValue);
void COMM_PrintCurTime();

void COMM_SetModuleToApply(unsigned short usMID);
int COMM_IsModuleToApply(unsigned short usMID);

/* ��ȡ��ǰ��Factroy Mode*/
const char * COMM_GetConfigSetting(ValuePostionType eGetMode);
#ifdef CONFIG_LAST_SAVE_CFG
int COMM_SaveLastConfig();
int COMM_RecoverLastConfig();
int COMM_SaveConfigInfo(unsigned long ulTime, const char * pszVersion);
int COMM_GetConfigInfo(unsigned long * ulTime,  char * pszVersion);
#endif

#endif
