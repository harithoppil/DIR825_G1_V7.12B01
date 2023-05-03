/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ�����: New_msg.c
 �ļ�����: ����Ϣ���ȹ��̵ķ�װ����

 �޶���¼:
        1. ����: ��ΰ
           ����: 2008-08-07
           ����: �����ļ�

**********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "hash.h"
#include "new_msg.h"
#include "msg_prv.h"
#include "tbsutil.h"
#include "tbserror.h"


/* ��ǰģ��id, ���ڵ�������ʾ��ǰ���ȵ��ĸ�ģ�� */
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

static unsigned short stModuleList[MAX_MODULE_NUM] = {0};       /*ģ���¼����*/
static unsigned int iModuleNum=0;                                                    /*���õ�����*/

/*************************************************************************
����:   ���ģ�鵽�б���
����:   unsigned short usModuleID ,         ģ��ID
����:   TRUE ,    �ɹ�
               ����,                ʧ��
��ע:
*************************************************************************/
BOOL AddModule2List(unsigned short usModuleID)
{
    if (iModuleNum>=MAX_MODULE_NUM-1)
        return FALSE;

    stModuleList[iModuleNum++] = usModuleID;
    return TRUE;
}

/*************************************************************************
����:   ���ģ���Ƿ����б���
����:   unsigned short usModuleID ,         ģ��ID
����:   TRUE ,     ��
               ����,   ����
��ע:
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
����:   ���б���ɾ��ģ��
����:   unsigned short usModuleID ,         ģ��ID
����:   ��
��ע:
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
����:   ��ȡģ������
����:   ��
����:   ģ������
��ע:
*************************************************************************/
unsigned int GetModuleListCount()
{
    return iModuleNum;
}

/*************************************************************************
����:   ��ȡ��Ӧ��ģ��MID
����:   ��
����:   ģ������
��ע:
*************************************************************************/
unsigned short GetModuleID(unsigned int uiID)
{
    return stModuleList[uiID];
}

/*************************************************************************
����:   ���MID�б�
����:   ��
����:   ģ������
��ע:
*************************************************************************/
void CleanModuleList()
{
    memset(stModuleList,0,MAX_MODULE_NUM*sizeof(short));
    iModuleNum=0;
}

/*
  *     MSG TYPE hash�е�hash�����Ķ���
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
#endif

/*
  *     ���Ӻ��������hash�е�hash�����Ķ���
  */

/* �����hash�㷨���Ӧ��ͬ����Ϣ���ͼ��㲻ͬ��λ�� */
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

/* ͨ����ϣ��ʽ�Ҳ����򷵻�NULL�����ٱ�������hook entry����Ϊ�����ȽϺķ�ʱ�� */
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
  *     �µ���Ϣ������غ����Ķ���
  */

/*************************************************************************
����:   ��ʼ����ؽṹ��
����:   ��
����:   ģ������
��ע:
*************************************************************************/
void MSG_MSG_Init()
{
    memset(g_Module_Table,0,sizeof(g_Module_Table));
}

/*************************************************************************
����:   �����������ӱ��hash
����:   ST_HookEntry *pHookTable                  ��������Ĺ��ӱ�
               hashtable *pHash                                  �����Ĺ��ӱ�
����:   ��
��ע:
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
����:   �����Ϣ�������Ϣ���ȱ�hash����
����:   unsigned short usMID                                                        MID
               ST_MSG_MAPPING *pstMsgMapTable                                ģ�����Ϣ���ȱ�
����:   ��
��ע:
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

        /*ʵ���ã���ʱֻ������ʹ��hash,LIWEI_TEST*/

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
����:   ɾ����Ϣ�������Ϣ���ȱ��hansh
����:   unsigned short usMID                                                        MID
               ST_MSG_MAPPING *pstMsgMapTable                                ģ�����Ϣ���ȱ�
����:   ��
��ע:
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
����:   ������صĽṹ��
����:   ��
����:   ��
��ע:
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
����:   ����Ƿ�ģ����Ϣ���ȱ��Ƿ�Ϊ��
����:   ��
����:   TRUE     Ϊ��
               FALSE   Ϊ�ǿ�
��ע:
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
����:   ���ĳ��ģ���Ƿ�ע��
����:   ��
����:   TRUE     Ϊ��
               FALSE   Ϊ�ǿ�
��ע:
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
����:   ��ȡ��Ϣ���Ⱥ������hash��
����:   unsigned short usMID               ģ��MID
               unsigned short usMsgType       ��Ϣ����
               ST_MSG_DATA **pMsgData      ��Ϣ�뺯��ӳ��Ľṹ
����:   TRUE     �ɹ�
               FALSE   ʧ��
��ע:
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
����:   ��ȡģ���cache
����:   unsigned short usMID               ģ��MID
����:   struct list_head *                      ģ���Ӧ��cache
��ע:
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
����:   ģ��ע����Ϣ�����ĺ���
����:   unsigned short usMID                                ģ��MID
               ST_MSG_MAPPING *pstMsgMapping          ��Ϣ����ӳ���
����:   TBS_SUCCESS             �ɹ�
               ����                        ʧ��
��ע:
*************************************************************************/
MSG_RET MSG_MSG_RegModule(unsigned short usMID, ST_MSG_MAPPING *pstMsgMapping)
{
    unsigned char ucPid = 0;
    MSG_RET ret = MSG_OK;

    ST_MSG_MAPPING *pstMsgMapEntry = pstMsgMapping;


    /* �����PID */
    ucPid = MID2PID(usMID);

    if (!s_ucMsgInited)     /* ��ǰδ��ʼ�� */
    {
        /* ����pid��ʼ�� */
        ret = MSG_Init(ucPid);
        if (MSG_OK != ret)
        {
            MSG_ERR(ret, "MID: %04x", usMID);
            return ret;
        }

        s_ucMsgInited = 1;   /* ȫ��pid���޸��Ѿ��� MSG_Init ���� */
    }
    else
    {
        if (ucPid != s_ucMsgPid)     /* pid ��һ�� */
        {
            /* ���ش��� */
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
����:   ģ��ע����Ϣ�����ĺ���
����:   unsigned short usMID                                ģ��MID
               ST_MSG_MAPPING *pstMsgMapping          ��Ϣ����ӳ���
����:   TBS_SUCCESS             �ɹ�
               ����                        ʧ��
��ע:
*************************************************************************/
MSG_RET MSG_MSG_UnregModule(unsigned short usMID, ST_MSG_MAPPING *pstMsgMapping)
{
    unsigned char ucPid = 0;
    unsigned char ucAllReged = 0;

    /* ���pid�Ƿ���� */
    ucPid = MID2PID(usMID);
    if (ucPid != s_ucMsgPid)     /* pid ��һ�� */
    {
        /* ���ش��� */
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
        /* MSGģ����� */
        s_ucMsgNeedExit = 1;
        (void)MSG_MSG_Uninit();
    }

    RemoveModuleFromList(usMID);

    return MSG_OK;
}

/*************************************************************************
����:   ����Ϣ���ȴ����֣�Ƕ�����ϵ���Ϣ���Ⱥ�����
����:   ST_MSG_NODE *pstMsgNode          ��Ϣ��
����:   ��
��ע:
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

    /* �㲥���鲥*/
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
    else   /* ���� */
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
����:   �����Ӧ��ģ����Ϣӳ���
����:   unsigned short usMID         MID
����:   ��
��ע:
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

