#ifndef _NEW_MSG_H_
#define _NEW_MSG_H_

#include "common_msg_type.h"
//#include "common_msg_handler.h"
#include "hash.h"
#include "msg_prv.h"
//#include "normal_list.h"

#define true 1
#define false 0
typedef int bool;

#define MAX_MODULE_NUM 0xFF

/* ��־��ӡ�� */
//#define COMMON_MSG_TRACE(fmt, args...)  COMMON_TRACE(MID_CCP, fmt, ##args)

extern hashtable *phtMsgFunc;

typedef struct tag_ST_MSG_FUNC_KEY
{
	unsigned short usMsgType;
}ST_MSG_KEY;

typedef int (*MSG_PROCESS_FUNC)(ST_MSG *);

typedef enum HOOK_TABLE_TYPE
{
    ARRAY_TABLE=1,
    HASH_TABLE,
}HookTableType;

typedef struct tag_ST_MSG_FUNC_DATA
{
	MSG_PROCESS_FUNC  pfCustomProcess;
	ST_HookEntry * pstHookTable;
	/*Ϊ�˽��hook��̫���ٶȻ�Ƚ��������⣬��hook��ʹ��hash*/
        HookTableType stTableType;
        hashtable *pHashModuleFuncTable;
}ST_MSG_DATA;

typedef struct tag_ST_PATH_KEY
{
	char *pszPath;
        HookType eHookType;
}ST_PATH_KEY;

typedef struct tag_ST_PATH_DATA
{
    void *pHookFunc;
}ST_PATH_DATA;

typedef struct tag_ST_MSG_MAPPING
{
    unsigned short usMsgType;
    MSG_PROCESS_FUNC  pfCustomProcess;
    ST_HookEntry * pstHookTable;
}ST_MSG_MAPPING;

#define MFLG_BIT_TO_APPLY       1 << 0
//TO EXTEND: ����չ

typedef struct tag_ST_MODULE_NODE
{
    hashtable *pHashModuleFuncTable;
    struct list_head listCache;
    unsigned short  usCacheBuildMID; /*��ʾģ��Ļ������������ĸ�ģ�����õ�,Ĭ��Ϊ0(����ģ��)*/
    unsigned char ucModuleFLag;      /* ģ��İ�λ��ʶ */
}ST_MODULE_NODE;

void *FuncHash_GetValue(void *key, hashtable *tab);

void Msg_MSG_InitMsgFuncHash();
void MSG_MSG_UninitMsgFuncHash();

void MSG_MSG_Init();
void MSG_MSG_Uninit();

MSG_RET MSG_MSG_RegModule(unsigned short usMID, ST_MSG_MAPPING *pstMsgMapping);
MSG_RET MSG_MSG_UnregModule(unsigned short usMID, ST_MSG_MAPPING *pstMsgMapping);

int Call_Msg_ProcFunc(ST_MSG_DATA *pstMsgData,ST_MSG *pstMsg);
void MSG_MSG_Dispatch(ST_MSG_NODE *pstMsgNode);

bool MSG_MSG_AddMsgFunc(unsigned short usMID, ST_MSG_MAPPING *pstMsgMapTable);
bool MSG_MSG_DelMsgFunc(unsigned short usMID);

unsigned char MSG_MSG_IsAllUnreged(void);
bool MSG_MSG_IsMsgReged(unsigned short usMID);

bool MSG_MSG_GetProcessFunc(unsigned short usMID,unsigned short usMsgType,ST_MSG_DATA **pMsgData);
void MSG_MSG_DumpHashTable(unsigned short usMID);
ST_MSG *MSG_ReplaceNodePrefix(ST_MSG *pstOldMsg, char *szOldPrefix, char *szNewPrefix);

struct list_head * MSG_MSG_GetModuleCache(unsigned short usMID);

unsigned int GetModuleListCount();
unsigned short GetModuleID(unsigned int uiID);

#endif
