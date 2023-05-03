

#include "tbserror.h"
#include <stdarg.h>
#include "common.h"
#include "msg_prv.h"
#include "new_msg.h"
#include "msg_prio.h"


#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif


#ifdef _MSG_DEBUG

    /* ��ǰģ��id, ���ڵ�������ʾ��ǰ���ȵ��ĸ�ģ�� */
    STATIC unsigned short s_usMsgCurMID = 0;
    #define MSG_CUR_MID(usDstMID) (s_usMsgCurMID = (usDstMID))
#else
    #define MSG_CUR_MID(usDstMID)  (void)0
#endif



/* �����̵��׽��־�� */
int s_iMsgSocket = -1;

/* ģ���ʼ����־ */
unsigned char s_ucMsgInited = 0;

/* �����̵�pid */
unsigned char s_ucMsgPid = 0;

/* ��־�Ƿ���Ҫ�˳� */
unsigned char s_ucMsgNeedExit = 0;


/* �Ƿ�����ģ���ʼ����� */
STATIC unsigned char s_ucMsgAllModStarted = 0;


/* ��Ϣ���Ͷ��� */
STATIC ST_MSG_NODE *s_pstMsgSendQueHead = NULL;
STATIC ST_MSG_NODE *s_pstMsgSendQueTail = NULL;



/* MID�������� */
STATIC ST_MSG_PROC_NODE **s_ppstMsgProcTable = NULL;

/* ��Ϣ���� */
STATIC ST_MSG_NODE *s_pstMsgQueHead = NULL;
STATIC ST_MSG_NODE *s_pstMsgQueTail = NULL;

/* ��ϢƬ������ */
STATIC ST_MSG_PART *s_pstMsgPartList = NULL;

/* �鲥�ⲿmid�Ķ�ά���� */
STATIC unsigned short **s_ppusMsgGroupMIDList = NULL;


MSG_RET MSG_Init(unsigned char ucPid)
{
    struct sockaddr_un stSockAddr;
    MSG_RET ret = MSG_OK;

    /* ���������׽��� */
    s_iMsgSocket = socket(PF_UNIX, (int)SOCK_DGRAM, 0);
    if (-1 == s_iMsgSocket)
    {
        /*����ʧ��*/
        MSG_ERR(ERR_MSG_SOCKET);
        return ERR_MSG_SOCKET;
    }

    stSockAddr.sun_family = AF_UNIX;
    sprintf(stSockAddr.sun_path, MSG_SOCKET_FORMAT, ucPid);
    ret = bind(s_iMsgSocket, (struct sockaddr *)(void *)&stSockAddr,
                                                sizeof(stSockAddr));
    if (ret != 0)
    {
        /*
        Ŀ�����ڲ���һ�������ظ������׽���
        */
        /* connect */
        ret = connect(s_iMsgSocket, (struct sockaddr *)(void *)&stSockAddr,
                                                       sizeof(stSockAddr));
        if (0 == ret) /* ע��, ����0��ʾ���ӳɹ�, ��ģ����ʹ��,��˲��ܳ�ʼ�� */
        {
            close(s_iMsgSocket);
            s_iMsgSocket = -1;
            MSG_ERR(ERR_MSG_SOCKET);
            return ERR_MSG_SOCKET;
        }

        /* unlink */
        ret = unlink(stSockAddr.sun_path);
        if (0 != ret)
        {
            close(s_iMsgSocket);
            s_iMsgSocket = -1;
            MSG_ERR(ERR_MSG_SOCKET);
            return ERR_MSG_SOCKET;
        }

        /* bind */
        ret = bind(s_iMsgSocket, (struct sockaddr *)(void *)&stSockAddr,
                                                sizeof(stSockAddr));
        if (0 != ret)
        {
            close(s_iMsgSocket);
            s_iMsgSocket = -1;
            MSG_ERR(ERR_MSG_SOCKET);
            return ERR_MSG_SOCKET;
        }
    }

    s_ucMsgPid = ucPid;

    /* ����socket����Ȩ�� */
    (void)chmod(stSockAddr.sun_path, 0777);

    /* ������ڴ� */
    s_ppstMsgProcTable = malloc(sizeof(ST_MSG_PROC_NODE *) * MSG_MAX_MID_NUM);
    if (NULL == s_ppstMsgProcTable)
    {
        unlink(stSockAddr.sun_path);
        close(s_iMsgSocket);
        s_iMsgSocket = -1;
        s_ucMsgPid = 0;
        MSG_ERR(ERR_MSG_MALLOC_FAIL);
        return ERR_MSG_MALLOC_FAIL;
    }
    memset(s_ppstMsgProcTable, 0, sizeof(ST_MSG_PROC_NODE *) * MSG_MAX_MID_NUM);

    s_pstMsgPartList = malloc(sizeof(ST_MSG_PART) * MSG_MAX_PART_GROUP);

    if (NULL == s_pstMsgPartList)
    {
        unlink(stSockAddr.sun_path);
        close(s_iMsgSocket);
        s_iMsgSocket = -1;
        s_ucMsgPid = 0;
        free(s_ppstMsgProcTable);
        s_ppstMsgProcTable = NULL;
        MSG_ERR(ERR_MSG_MALLOC_FAIL);
        return ERR_MSG_MALLOC_FAIL;
    }
    memset(s_pstMsgPartList, 0, sizeof(ST_MSG_PART) * MSG_MAX_PART_GROUP);

    MSG_Prio_Init();

    /*add by kideli for new msg process */
    MSG_MSG_Init();
    /******************************/
    return MSG_OK;
}

MSG_RET MSG_Uninit(void)
{
    char acSocket[MSG_SOCKET_LEN];
    ST_MSG_NODE *pstMsgNode = NULL;
    ST_MSG_NODE *pstNext = NULL;

    /* ɾ���׽��� */
    if (-1 != s_iMsgSocket)
    {
        close(s_iMsgSocket);

        sprintf(acSocket, MSG_SOCKET_FORMAT, s_ucMsgPid);
        (void)unlink(acSocket);

        s_iMsgSocket = -1;
        s_ucMsgPid = 0;
    }

    /* �ͷ�������Ϣ */
    pstMsgNode = s_pstMsgQueHead;
    while (NULL != pstMsgNode)
    {
        pstNext = pstMsgNode->pstNext;
        free(pstMsgNode);
        pstMsgNode = pstNext;
    }
    s_pstMsgQueHead = NULL;
    s_pstMsgQueTail = NULL;
    MSG_Prio_Uninit();

    /* �ͷű�Ĵ������Լ����ڴ� */
    if (NULL != s_ppstMsgProcTable)
    {
        unsigned i = 0;
        ST_MSG_PROC_NODE *pstProcNext = NULL;

        for (i = 0; i < MSG_MAX_MID_NUM; i++)
        {
            while (NULL != s_ppstMsgProcTable[i])
            {
                pstProcNext = s_ppstMsgProcTable[i]->pstNext;
                free(s_ppstMsgProcTable[i]);
                s_ppstMsgProcTable[i] = pstProcNext;
            }
        }

        free(s_ppstMsgProcTable);
        s_ppstMsgProcTable= NULL;
    }
    if (NULL != s_pstMsgPartList)
    {
        free(s_pstMsgPartList);
        s_pstMsgPartList = NULL;
    }

    s_ucMsgInited = 0;
    s_ucMsgNeedExit = 0;

    /*add by kideli for new msg process */
    MSG_MSG_Uninit();
    /******************************/

    return MSG_OK;
}

MSG_RET MSG_AddMIDProcFunc(unsigned short usMID, FUN_RECEIVE funReceive)
{
    unsigned char i = 0;
    ST_MSG_PROC_NODE *pstProcNode = NULL;

    pstProcNode = malloc(sizeof(ST_MSG_PROC_NODE));
    if (NULL == pstProcNode)
    {
        MSG_ERR(ERR_MSG_MALLOC_FAIL);
        return ERR_MSG_MALLOC_FAIL;
    }
    pstProcNode->pfnFunc = funReceive;

    i = MID2INDEX(usMID);
    pstProcNode->pstNext = s_ppstMsgProcTable[i];
    s_ppstMsgProcTable[i] = pstProcNode;

    return MSG_OK;
}

MSG_RET MSG_DelMIDProcFunc(unsigned short usMID, FUN_RECEIVE funReceive)
{
    unsigned char i = 0;
    ST_MSG_PROC_NODE *pstProcNode = NULL;
    ST_MSG_PROC_NODE *pstProcPrev = NULL;

    i = MID2INDEX(usMID);
    pstProcNode = s_ppstMsgProcTable[i];

    while (NULL != pstProcNode)
    {
        if (funReceive == pstProcNode->pfnFunc)
        {
            /* ɾ���ڵ� */
            if (NULL != pstProcPrev)
            {
                pstProcPrev->pstNext = pstProcNode->pstNext;
            }
            else
            {
                s_ppstMsgProcTable[i] = pstProcNode->pstNext;
            }

            /* �ͷ��ڴ� */
            free(pstProcNode);

            return MSG_OK;
        }
        pstProcPrev = pstProcNode;
        pstProcNode = pstProcNode->pstNext;
    }

    return ERR_MSG_PROC_NOT_FOUND;
}

unsigned char MSG_IsAllUnreged(void)
{
    unsigned char i = 0;

    for (i = 0; i < MSG_MAX_MID_NUM; i++)
    {
        if (NULL != s_ppstMsgProcTable[i])
        {
            return false;
        }
    }
    return MSG_MSG_IsAllUnreged();
}

unsigned char MSG_IsMIDReged(unsigned usMID)
{
    unsigned char i = MID2INDEX(usMID);

    if (NULL == s_ppstMsgProcTable[i])
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

MSG_RET MSG_SendOutMsg(ST_MSG_NODE *pstMsgNode, unsigned char ucPid)
{
    struct sockaddr_un stToAddr;
    unsigned int uiToLen = 0;
    ST_MSG *pstMsg = NULL;
    ST_MSG *pstNewMsg = NULL;
    unsigned long ulCount = 0;
    unsigned long ulRestLen = 0;
    int flag = MSG_DONTWAIT;

    ST_MSG_NODE *pstMsgNodeWork = pstMsgNode;

    pstMsg = &pstMsgNode->stMsg;
    pstNewMsg = pstMsg;
#ifdef CONFIG_REPLACE_NODE_PREFIX
    if ((MID_TR069FE == pstMsg->stMsgHead.usDstMID || MID_CTMDW == pstMsg->stMsgHead.usDstMID)
        && PID_TM != ucPid
        && (MID_CMM == pstMsg->stMsgHead.usSrcMID
            || MID_APPMON == pstMsg->stMsgHead.usSrcMID))
    {
        pstNewMsg = MSG_ReplaceNodePrefix(pstMsg, CPE_PREFIX, CONFIG_PRODUCT_PREFIX);
        if (NULL == pstNewMsg)
        {
            return -1;
        }

        pstMsgNodeWork = MSG_GET_NODE_ADDR(pstNewMsg);
    }

    if ((MID_TR069FE == pstMsg->stMsgHead.usSrcMID || MID_CTMDW == pstMsg->stMsgHead.usSrcMID)
        && PID_TM != ucPid
        && (MID_CMM == pstMsg->stMsgHead.usDstMID))
    {
        pstNewMsg = MSG_ReplaceNodePrefix(pstMsg, CONFIG_PRODUCT_PREFIX, CPE_PREFIX);
        if (NULL == pstNewMsg)
        {
            return -1;
        }

        pstMsgNodeWork = MSG_GET_NODE_ADDR(pstNewMsg);
    }
#endif
    uiToLen = sizeof(stToAddr);
    stToAddr.sun_family = AF_UNIX;
    sprintf(stToAddr.sun_path, MSG_SOCKET_FORMAT, ucPid);

    ulRestLen = pstNewMsg->stMsgHead.ulBodyLength;
    if (ulRestLen > MSG_BODY_MAX_LEN)
    {
        pstMsgNodeWork->ucPartFlag = MSG_PART_FLAG;
    }

    if(ulRestLen>(MSG_BODY_MAX_LEN*4))
    {                   
        flag = 0;
    }

    do
    {
        if (ulRestLen > MSG_BODY_MAX_LEN)
        {
            ulCount = MSG_BODY_MAX_LEN;
        }
        else
        {
            ulCount = ulRestLen;
        }
        pstMsgNodeWork->usMsgLen = ulCount + sizeof(ST_MSGHEAD);
        ulCount = (unsigned long)sendto(s_iMsgSocket, (char *)pstMsgNodeWork,
                            MSG_SEND_EXT_LEN + ulCount , flag,
                            (struct sockaddr *)(void *)&stToAddr, uiToLen);
        if ((long)ulCount <= 0)
        {
#ifndef _MSG_SUPPORT_TM

            MSG_ERR(ERR_MSG_SEND_FAIL, "%s %lu %lu", stToAddr.sun_path,
                                 pstNewMsg->stMsgHead.ulBodyLength, ulRestLen);
#endif
            return ERR_MSG_SEND_FAIL;
        }

        ulCount -= MSG_SEND_EXT_LEN;

        ulRestLen -= ulCount;
        if (ulRestLen > 0)
        {
            memcpy((char *)pstMsgNodeWork + ulCount, (char *)pstMsgNodeWork, MSG_SEND_EXT_LEN);
            pstMsgNodeWork = (ST_MSG_NODE *)((char *)pstMsgNodeWork + ulCount);
        }
    } while (ulRestLen > 0);

    if(pstNewMsg != pstMsg)
    {
        safe_free_msg(pstNewMsg);
    }

    return MSG_OK;
}


void MSG_AppendQueMsg(ST_MSG_NODE *pstMsgNode)
{
    pstMsgNode->pstNext = NULL;

    if (NULL == s_pstMsgQueHead)
    {
        s_pstMsgQueHead = pstMsgNode;
        s_pstMsgQueTail = s_pstMsgQueHead;
    }
    else
    {
        s_pstMsgQueTail->pstNext = pstMsgNode;
        s_pstMsgQueTail = s_pstMsgQueTail->pstNext;
    }
    s_pstMsgQueTail->pstNext = NULL;
}

ST_MSG_NODE *MSG_GetQueMsg(void)
{
    ST_MSG_NODE *pstMsgNode = NULL;

    pstMsgNode = s_pstMsgQueHead;
    if (NULL != s_pstMsgQueHead)
    {
        s_pstMsgQueHead = s_pstMsgQueHead->pstNext;
        if (NULL == s_pstMsgQueHead)
        {
            s_pstMsgQueTail = NULL;
        }
    }

    return pstMsgNode;
}

MSG_RET MSG_AppendMsgPart(const ST_MSG_NODE *pstMsgNode, unsigned long ulLen)
{
    unsigned char i = 0;
    ST_MSG_PART *pstMsgPart = NULL;
    ST_MSG *pstMsg = NULL;

    for(i = 0; i < MSG_MAX_PART_GROUP; i++)
    {
        if (NULL != s_pstMsgPartList[i].pstMsgNode
            && s_pstMsgPartList[i].pstMsgNode->stMsg.stMsgHead.ulMsgID
                 == pstMsgNode->stMsg.stMsgHead.ulMsgID)
        {
            pstMsgPart = &s_pstMsgPartList[i];
            break;
        }
    }

    if (NULL == pstMsgPart)
    {
        for (i = 0; i < MSG_MAX_PART_GROUP; i++)
        {
            if (NULL == s_pstMsgPartList[i].pstMsgNode)
            {
                pstMsgPart = &s_pstMsgPartList[i];
                break;
            }
        }
        if (NULL == pstMsgPart)
        {
            MSG_ERR(ERR_MSG_PART_LIST_FULL);
            return ERR_MSG_PART_LIST_FULL;
        }

        pstMsg = MSG_CreateMessage(pstMsgNode->stMsg.stMsgHead.ulBodyLength);
        if (NULL == pstMsg)
        {
            MSG_ERR(ERR_MSG_MALLOC_FAIL);
            return ERR_MSG_MALLOC_FAIL;
        }
        pstMsg->stMsgHead.usSrcMID = pstMsgNode->stMsg.stMsgHead.usSrcMID;
        pstMsg->stMsgHead.usDstMID = pstMsgNode->stMsg.stMsgHead.usDstMID;
        pstMsg->stMsgHead.ulMsgID = pstMsgNode->stMsg.stMsgHead.ulMsgID;
        pstMsg->stMsgHead.usMsgType = pstMsgNode->stMsg.stMsgHead.usMsgType;

        pstMsgPart->pstMsgNode = MSG_GET_NODE_ADDR(pstMsg);
        pstMsgPart->pcCurPart = pstMsg->szMsgBody;
        pstMsgPart->ulRestLen = pstMsg->stMsgHead.ulBodyLength;
    }

    if (pstMsgPart->ulRestLen < ulLen)
    {
        MSG_ERR(ERR_MSG_PART_INVALID);
        return ERR_MSG_PART_INVALID;
    }

    memcpy(pstMsgPart->pcCurPart, pstMsgNode->stMsg.szMsgBody, ulLen);
    pstMsgPart->pcCurPart += ulLen;
    pstMsgPart->ulRestLen -= ulLen;

    /* �Ѿ����� */
    if (0 == pstMsgPart->ulRestLen)
    {
        MSG_AppendQueMsg(pstMsgPart->pstMsgNode);
        pstMsgPart->pstMsgNode = NULL;
    }

    return MSG_OK;
}


MSG_RET MSG_ProcMsgPart(ST_MSG_NODE *pstMsgNode, unsigned long ulCount)
{
    ST_MSG *pstMsg = &pstMsgNode->stMsg;
    unsigned long ulLen = ulCount - MSG_SEND_EXT_LEN;
    MSG_RET ret = MSG_OK;

    /*�����Ϣ��������*/
    if (ulCount < MSG_SEND_EXT_LEN
        || ulCount < pstMsgNode->usMsgLen + MSG_CTRL_LEN)
    {
        free(pstMsgNode);

        /*��Ϣͷ���ݲ�ȫ*/
        MSG_ERR(ERR_MSG_NOT_FULL);
        return MSG_OK;
    }

    if (MSG_PART_FLAG != pstMsgNode->ucPartFlag)  /* ������Ϣ */
    {
        if (pstMsg->stMsgHead.ulBodyLength != ulLen)
        {
            free(pstMsgNode);

            /* ��Ϣ�����ݲ�ȫ, ֱ�Ӷ��� */
            MSG_ERR(ERR_MSG_NOT_FULL);
            return MSG_OK;
        }

        /* ֱ�ӷ������ */
        MSG_AppendQueMsg(pstMsgNode);
    }
    else
    {
        if (pstMsg->stMsgHead.ulBodyLength == ulLen)   /* �������� */
        {
            /* ������� */
            MSG_ERR(ERR_MSG_PART_NOEFFECT);

            /* ֱ�ӷ������ */
            MSG_AppendQueMsg(pstMsgNode);
            return MSG_OK;
        }

        ret = MSG_AppendMsgPart(pstMsgNode, ulLen);
        free(pstMsgNode);
        if (MSG_OK != ret)
        {
            MSG_ERR(ret);
            return ret;
        }
    }

    return MSG_OK;
}


unsigned char MSG_IsAllMsgFull(void)
{
    unsigned char i = 0;
    unsigned char ucAllFull = 1;

    for (i = 0; i < MSG_MAX_PART_GROUP; i++)
    {
        if (NULL != s_pstMsgPartList[i].pstMsgNode)
        {
            ucAllFull = 0;
            break;
        }
    }

    return ucAllFull;
}

void MSG_FreeAllPart(void)
{
    unsigned char i = 0;

    for (i = 0; i < MSG_MAX_PART_GROUP; i++)
    {
        free(s_pstMsgPartList[i].pstMsgNode);
        s_pstMsgPartList[i].pstMsgNode = NULL;
    }
}

MSG_RET MSG_ReceiveOutMsg(unsigned long lTimeout)
{
    int iSelect = 0;
    unsigned long ulCount = 0;          /*�����ַ�����*/
    struct sockaddr_un stFrmAddr;       /*�Է�Э���ַ*/
    int iFrmLen = 0;                    /*�Է�Э���ַ����*/
    ST_MSG *pstMsg = NULL;              /*��Ϣ������*/
    ST_MSG_NODE *pstMsgNode = NULL;
    fd_set fds;
    struct timeval tv;
    MSG_RET ret = MSG_OK;
    unsigned char usIsAllFull = 0;

    while (!usIsAllFull)
    {
        /*���ս�������Ϣ��*/
        FD_ZERO(&fds);
        FD_SET((unsigned long)s_iMsgSocket, &fds);
        tv.tv_sec = (long)lTimeout;    /*�����ʱ*/
        tv.tv_usec = 0;

#ifdef _MSG_SUPPORT_TM
        if (lTimeout >= 100000)
        {
            tv.tv_sec = 0;
            tv.tv_usec = (long)lTimeout;
        }
#endif
        iSelect = select(s_iMsgSocket + 1, &fds, NULL, NULL, &tv);
        if (iSelect < 0)
        {
            MSG_FreeAllPart();

            MSG_ERR(ERR_MSG_SOCKET);
            return ERR_MSG_SOCKET;
        }
        else if(iSelect == 0)
        {
            MSG_FreeAllPart();
            /*��ʱ, û����Ϣ��*/
            return ERR_MSG_TIMEOUT;
        }
        else
        {
            ;
        }
        /*��������, ���Խ���...*/

        if (ioctl(s_iMsgSocket, FIONREAD, &ulCount) != 0)
        {
            MSG_FreeAllPart();

            MSG_ERR(ERR_MSG_SOCKET);
            return ERR_MSG_SOCKET;
        }

        /*��ʼ������Ϣ��*/
        pstMsg = MSG_CreateMessage(ulCount - MSG_SEND_EXT_LEN); /*Ҫ���û�����MSG_ReleaseMessage�����ͷ�*/
        if (NULL == pstMsg)
        {
            MSG_FreeAllPart();

            /* �ڴ����ʧ�� */
            MSG_ERR(ERR_MSG_MALLOC_FAIL);
            return ERR_MSG_MALLOC_FAIL;
        }
        pstMsgNode = MSG_GET_NODE_ADDR(pstMsg);
        iFrmLen = sizeof(stFrmAddr);
        ulCount = (unsigned long)recvfrom(s_iMsgSocket, (void *)pstMsgNode, ulCount, 0,
             (struct sockaddr *)(void *)&stFrmAddr, (unsigned int *)&iFrmLen);
        if ((long)ulCount <= 0)
        {
            /*ϵͳ����, ��Sockekt�������ر�*/
            free(pstMsgNode);
            MSG_FreeAllPart();

            MSG_ERR(ERR_MSG_SOCKET);
            return ERR_MSG_SOCKET;
        }

        ret = MSG_ProcMsgPart(pstMsgNode, ulCount);
        if (MSG_OK != ret)
        {
            MSG_FreeAllPart();

            MSG_ERR(ret);
            return ret;
        }

        /* �����Ϣ�Ƿ������� */
        usIsAllFull = MSG_IsAllMsgFull();
    }

    return MSG_OK;
}


void MSG_Dispatch(ST_MSG_NODE *pstMsgNode)
{
    unsigned char i = 0;
    unsigned char j = 0;
    unsigned char k = 0;
    ST_MSG_PROC_NODE *pstProcNode = NULL;

    i = MID2INDEX(pstMsgNode->stMsg.stMsgHead.usDstMID);
    k = i;

    if (FLG_MID_GRP == i)
    {
        for (i = 0; i < MSG_MAX_MID_NUM; i++)
        {
            MSG_CUR_MID(pstMsgNode->stMsg.stMsgHead.usDstMID);
            pstProcNode = s_ppstMsgProcTable[i];
            while (NULL != pstProcNode)
            {
                MSG_LOG("[MSG_Dspt_GRP]: %04x %04x %08lx %04x %lu",
                    pstMsgNode->stMsg.stMsgHead.usSrcMID, pstMsgNode->stMsg.stMsgHead.usDstMID,
                    pstMsgNode->stMsg.stMsgHead.ulMsgID, pstMsgNode->stMsg.stMsgHead.usMsgType,
                    pstMsgNode->stMsg.stMsgHead.ulBodyLength);

                pstProcNode->pfnFunc(&pstMsgNode->stMsg);
                MSG_LOG("[MSG_Dspt_GRP]: %04x %04x %08lx %04x %lu ...... ok",
                    pstMsgNode->stMsg.stMsgHead.usSrcMID, pstMsgNode->stMsg.stMsgHead.usDstMID,
                    pstMsgNode->stMsg.stMsgHead.ulMsgID, pstMsgNode->stMsg.stMsgHead.usMsgType,
                    pstMsgNode->stMsg.stMsgHead.ulBodyLength);
                pstProcNode = pstProcNode->pstNext;

                j++;
            }
        }
    }
    else
    {
        MSG_CUR_MID(pstMsgNode->stMsg.stMsgHead.usDstMID);
        pstProcNode = s_ppstMsgProcTable[i];
        while (NULL != pstProcNode)
        {
            MSG_LOG("[MSG_Dspt]: %04x %04x %08lx %04x %lu",
                pstMsgNode->stMsg.stMsgHead.usSrcMID, pstMsgNode->stMsg.stMsgHead.usDstMID,
                pstMsgNode->stMsg.stMsgHead.ulMsgID, pstMsgNode->stMsg.stMsgHead.usMsgType,
                pstMsgNode->stMsg.stMsgHead.ulBodyLength);

            pstProcNode->pfnFunc(&pstMsgNode->stMsg);

            MSG_LOG("[MSG_Dspt]: %04x %04x %08lx %04x %lu ...... ok",
                pstMsgNode->stMsg.stMsgHead.usSrcMID, pstMsgNode->stMsg.stMsgHead.usDstMID,
                pstMsgNode->stMsg.stMsgHead.ulMsgID, pstMsgNode->stMsg.stMsgHead.usMsgType,
                pstMsgNode->stMsg.stMsgHead.ulBodyLength);
            pstProcNode = pstProcNode->pstNext;

            j++;
        }
    }

/*�򲿷�ģ���ڲ�δע�������飬���������ݲ���ע���鲥�����ַ��鲥��Ϣ*/
#if 0
    if (i >= MID_GRP_BASE)
    {
        i -= MID_GRP_BASE;
        if (NULL != s_ppusMsgGroupMIDList && NULL != s_ppusMsgGroupMIDList[i])
        {
            unsigned char k = 0;
            for (k = 0; k < MSG_EXT_MID_COUNT
                        && 0 != s_ppusMsgGroupMIDList[i][k]; k++)
            {
                unsigned char ucPid = MID2PID(s_ppusMsgGroupMIDList[i][k]);

                if (ucPid == s_ucMsgPid)
                {
                    ST_MSG_DATA *pstMsgData = NULL;

                    if (MSG_MSG_GetProcessFunc(s_ppusMsgGroupMIDList[i][k],
                           pstMsgNode->stMsg.stMsgHead.usMsgType, &pstMsgData))
                    {
                        MSG_LOG("[MSG_Dspt]: %04x %04x %08lx %04x %lu MID: %04x",
                                 pstMsgNode->stMsg.stMsgHead.usSrcMID, pstMsgNode->stMsg.stMsgHead.usDstMID,
                                 pstMsgNode->stMsg.stMsgHead.ulMsgID, pstMsgNode->stMsg.stMsgHead.usMsgType,
                                 pstMsgNode->stMsg.stMsgHead.ulBodyLength,
                                 s_ppusMsgGroupMIDList[i][k]);

                        Call_Msg_ProcFunc(pstMsgData,&(pstMsgNode->stMsg));

                        MSG_LOG("[MSG_Dspt]: %04x %04x %08lx %04x %lu MID: %04x ...... ok",
                                 pstMsgNode->stMsg.stMsgHead.usSrcMID, pstMsgNode->stMsg.stMsgHead.usDstMID,
                                 pstMsgNode->stMsg.stMsgHead.ulMsgID, pstMsgNode->stMsg.stMsgHead.usMsgType,
                                 pstMsgNode->stMsg.stMsgHead.ulBodyLength,
                                 s_ppusMsgGroupMIDList[i][k]);
                    }
                }
                else
                {
                    (void)MSG_SendOutMsg(pstMsgNode,
                         MID2PID(s_ppusMsgGroupMIDList[i][k]));
                }
            }
        }
    }
#endif

    MSG_CUR_MID((s_ucMsgPid << 8));

    if(!j || k >= MID_GRP_BASE)
        MSG_MSG_Dispatch(pstMsgNode);

}


void MSG_AppendSendQueMsg(ST_MSG_NODE *pstMsgNode)
{
    pstMsgNode->pstNext = NULL;

    if (NULL == s_pstMsgSendQueHead)
    {
        s_pstMsgSendQueHead = pstMsgNode;
        s_pstMsgSendQueTail = s_pstMsgSendQueHead;
    }
    else
    {
        s_pstMsgSendQueTail->pstNext = pstMsgNode;
        s_pstMsgSendQueTail = s_pstMsgSendQueTail->pstNext;
    }
    s_pstMsgSendQueTail->pstNext = NULL;
}


MSG_RET MSG_SendAllSendMsg(void)
{
    ST_MSG_NODE *pstMsgNode = s_pstMsgSendQueHead;
    ST_MSG_NODE *pstNext = NULL;

    while (MSG_OK == MSG_ReceiveOutMsg(0))
    {
        ;
    }

    while (NULL != pstMsgNode)
    {
        pstNext = pstMsgNode->pstNext;

        (void)MSG_SendMessage(&pstMsgNode->stMsg);
        MSG_ReleaseMessage(&pstMsgNode->stMsg);
        while (MSG_OK == MSG_ReceiveOutMsg(0))
        {
            ;
        }
        pstMsgNode = pstNext;
    }

    s_pstMsgSendQueHead = NULL;
    s_pstMsgSendQueTail = NULL;

    return MSG_OK;
}

void MSG_PollToIdle(void)
{
    ST_MSG_NODE *pstMsgNode = NULL;
    ST_MSG *pstMsg = NULL;
    MSG_RET ret = MSG_OK;

    while (NULL != s_pstMsgQueHead)
    {
        /* ������Ϣ */
        ret = MSG_ReceiveMessage(&pstMsg, MSG_POLL_ONCE_TIME);
        MSG_ASSERT(0 == ret);

        pstMsgNode = MSG_GET_NODE_ADDR(pstMsg);

        /* ������Ϣ */
        MSG_Dispatch(pstMsgNode);

        /* �ͷ���Ϣ */
        MSG_ReleaseMessage(pstMsg);
    }
}

#define MSG_FUNC_OUT

/*ģ��ע��, ע���ӿ�*/
MSG_RET MSG_RegModule(unsigned short usMID, FUN_RECEIVE funReceive)
{
    unsigned char ucPid = 0;
    MSG_RET ret = MSG_OK;

    MSG_CUR_MID(usMID);

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

    if (NULL != funReceive)
    {
        /* �������ҵ������� */
        ret = MSG_AddMIDProcFunc(usMID, funReceive);
        if (MSG_OK != ret)
        {
            MSG_ERR(ret, "MID: %04x", usMID);
        }
    }

    return ret;
}


void MSG_AllModStartOK(void)
{
    if (!s_ucMsgAllModStarted)
    {
        s_ucMsgAllModStarted = 1;
        (void)MSG_SendAllSendMsg();
    }
}


MSG_RET MSG_UnregModule(unsigned short usMID, FUN_RECEIVE funReceive)
{
    unsigned char ucPid = 0;
    MSG_RET ret = MSG_OK;
    unsigned char ucAllReged = 0;

    /* ���pid�Ƿ���� */
    ucPid = MID2PID(usMID);
    if (ucPid != s_ucMsgPid)     /* pid ��һ�� */
    {
        /* ���ش��� */
        MSG_ERR(ERR_MSG_MID_INVALID, "MID: %04x", usMID);
        return ERR_MSG_MID_INVALID;
    }

    if (NULL != funReceive)
    {
        ret = MSG_DelMIDProcFunc(usMID, funReceive);
        if (MSG_OK != ret)
        {
            MSG_ERR(ret, "MID: %04x", usMID);
            return ret;
        }
    }

    ucAllReged = MSG_IsAllUnreged();
    if (ucAllReged)
    {
        /* MSGģ����� */
        s_ucMsgNeedExit = 1;
        (void)MSG_Uninit();
    }

    return MSG_OK;
}

/*lint -save -e429*/
/*��Ϣ�������ӿ�*/
ST_MSG *MSG_CreateMessage(unsigned long ulSize)
{
    ST_MSG_NODE *pstMsgNode = NULL;
    unsigned long ulTotalLen = sizeof(ST_MSG) + ulSize + MSG_CTRL_LEN;

    /*���Ԥ������Ϣ������*/
    if (ulSize > MSG_CREATE_MAX_LEN)
    {
        MSG_ERR(ERR_MSG_PARA_INVALID, "size: %08x", (unsigned int)ulSize);
        return NULL;
    }

    /*Ϊ������Ϣ�������ڴ�*/
    pstMsgNode = (ST_MSG_NODE *)malloc(ulTotalLen);
    if (NULL == pstMsgNode)
    {
        MSG_ERR(ERR_MSG_MALLOC_FAIL);
        return NULL;
    }

    /*�����Ϣ��*/
    memset(pstMsgNode, 0, ulTotalLen);

    /*������Ϣ�峤��*/
    pstMsgNode->stMsg.stMsgHead.ulBodyLength = ulSize;

    /* ��ʼ����Ϣ�ڵ��еĿ��Ʋ��� */
    pstMsgNode->cRefCount = 1;
    pstMsgNode->pstNext = NULL;

    /*���ر���������Ϣ��ָ��*/
    return &pstMsgNode->stMsg;
}
/*lint -restore*/

/*��Ϣ���ͷŽӿ�*/
void MSG_ReleaseMessage(ST_MSG *pstMsg)
{
    ST_MSG_NODE *pstMsgNode = NULL;

    if (NULL == pstMsg)
    {
        MSG_ERR(ERR_MSG_PARA_INVALID);
        return;
    }

    pstMsgNode = MSG_GET_NODE_ADDR(pstMsg);
    if (pstMsgNode->cRefCount <= 1)
    {
        free(pstMsgNode);
    }
    else
    {
        pstMsgNode->cRefCount--;
    }
}

/*��Ϣ���ͣ����սӿ�*/
MSG_RET MSG_SendMessage(ST_MSG *pstMsg)
{
    ST_MSG_NODE *pstMsgNode = NULL;
    unsigned char ucPid = 0;
    MSG_RET ret = MSG_OK;
    unsigned short usDstMID = 0;
    unsigned short usMsgType = 0;

    if (NULL == pstMsg)
    {
        /* ���ش��� */
        MSG_ERR(ERR_MSG_PARA_INVALID);
        return ERR_MSG_PARA_INVALID;
    }

    pstMsgNode = MSG_GET_NODE_ADDR(pstMsg);

    if (!s_ucMsgAllModStarted)
    {
        pstMsgNode->cRefCount++;
        MSG_AppendSendQueMsg(pstMsgNode);
        return MSG_OK;
    }

    MSG_LOG("[MSG_Send]: %04x %04x %08lx %04x %lu",
        pstMsg->stMsgHead.usSrcMID, pstMsg->stMsgHead.usDstMID,
        pstMsg->stMsgHead.ulMsgID, pstMsg->stMsgHead.usMsgType,
        pstMsg->stMsgHead.ulBodyLength);

    usDstMID = pstMsg->stMsgHead.usDstMID;

#ifdef _MSG_SUPPORT_TM

    if (s_ucMsgPid != PID_TM
        && MSG_BODY_MAX_LEN >= pstMsg->stMsgHead.ulBodyLength)
    {
        (void)MSG_SendOutMsg(pstMsgNode, PID_TM);
    }

#endif

    ucPid = MID2PID(usDstMID);
    if (s_ucMsgPid != ucPid)
    {
        /* ֱ�ӷ��ͳ�ȥ */
        ret = MSG_SendOutMsg(pstMsgNode, ucPid);

#ifdef _MSG_SUPPORT_TM
        return MSG_OK;
#endif
        if (MSG_OK != ret)
        {
            MSG_ERR(ret);
            return ret;
        }
    }
    else
    {
        if (!MSG_IS_GRP_MID(usDstMID))  /* ������Ϣ */
        {
            unsigned char ucMIDReged = 0;
            usMsgType = pstMsg->stMsgHead.usMsgType;

            ucMIDReged = MSG_IsMIDReged(usDstMID);

            /****************add by kideli for new msg process**************/
            ucMIDReged |= MSG_MSG_IsMsgReged(usDstMID);
            /**********************************************************/

            if (!ucMIDReged)       /* Ŀ��ģ��û��ע�� */
            {
#ifdef _MSG_SUPPORT_TM
                return MSG_OK;
#endif
                MSG_ERR(ERR_MSG_DSTMID_UNREGED, "%04x", usDstMID);
                return ERR_MSG_DSTMID_UNREGED;
            }
        }

        /* ������Ϣ���� */
        pstMsgNode->cRefCount++;

        /* ��ӵ����� */
        MSG_AppendQueMsg(pstMsgNode);
    }

    return MSG_OK;
}

MSG_RET MSG_ReceiveMessage(ST_MSG **ppstMsg, unsigned long lSecond)
{
    ST_MSG_NODE *pstMsgNode = NULL;

    pstMsgNode = MSG_GetQueMsg();

    if (NULL == pstMsgNode)
    {
        /* ���ս�������Ϣ */
        (void)MSG_ReceiveOutMsg(lSecond);

        /* ������ȥȡ���� */
        pstMsgNode = MSG_GetQueMsg();
        if (NULL == pstMsgNode)
        {
            return ERR_MSG_TIMEOUT;
        }
        MSG_LOG("[MSG_Recv]: %04x %04x %08lx %04x %lu",
            pstMsgNode->stMsg.stMsgHead.usSrcMID, pstMsgNode->stMsg.stMsgHead.usDstMID,
            pstMsgNode->stMsg.stMsgHead.ulMsgID, pstMsgNode->stMsg.stMsgHead.usMsgType,
            pstMsgNode->stMsg.stMsgHead.ulBodyLength);
    }

    *ppstMsg = &pstMsgNode->stMsg;

    return MSG_OK;
}

void MSG_Poll(void)
{
    ST_MSG_NODE *pstMsgNode = NULL;
    ST_MSG *pstMsg = NULL;
    MSG_RET ret = MSG_OK;


    while (!s_ucMsgNeedExit)
    {
        /* ������Ϣ */
        ret = MSG_ReceiveMessage(&pstMsg, MSG_POLL_ONCE_TIME);
        if (MSG_OK != ret)
        {
            continue;
        }
        pstMsgNode = MSG_GET_NODE_ADDR(pstMsg);

        /* ������Ϣ */
        MSG_Dispatch(pstMsgNode);

        /* �ͷ���Ϣ */
        MSG_ReleaseMessage(pstMsg);
    }

    /* ģ����� */
    (void)MSG_Uninit();

    return;
}

static MSG_RET MSG_RegGroupModule(unsigned short usGroupID, unsigned short usMID)
{
    unsigned long ulSize = sizeof(unsigned short *) *
                                    (1 + FLG_MID_GRP - MID_GRP_BASE);
    unsigned char ucIndex = MID2INDEX(usGroupID);
    unsigned char i = 0;

    if (NULL == s_ppusMsgGroupMIDList)
    {
        s_ppusMsgGroupMIDList = malloc(ulSize);
        if (NULL == s_ppusMsgGroupMIDList)
        {
            MSG_ERR(ERR_MSG_MALLOC_FAIL);
            return ERR_MSG_MALLOC_FAIL;
        }

        memset(s_ppusMsgGroupMIDList, 0, ulSize);
    }

    if (ucIndex < MID_GRP_BASE || MID2PID(usGroupID) != s_ucMsgPid)
    {
        MSG_ERR(ERR_MSG_PARA_INVALID, "%u %u\n", usGroupID, usMID);
        return ERR_MSG_PARA_INVALID;
    }
    ucIndex -= MID_GRP_BASE;
    if (NULL == s_ppusMsgGroupMIDList[ucIndex])
    {
        ulSize = sizeof(unsigned short) * MSG_EXT_MID_COUNT;
        s_ppusMsgGroupMIDList[ucIndex] = malloc(ulSize);
        if (NULL == s_ppusMsgGroupMIDList[ucIndex])
        {
            MSG_ERR(ERR_MSG_MALLOC_FAIL);
            return ERR_MSG_MALLOC_FAIL;
        }
        memset(s_ppusMsgGroupMIDList[ucIndex], 0, ulSize);
    }
    for (i = 0; i < MSG_EXT_MID_COUNT; i++)
    {
        if (usMID == s_ppusMsgGroupMIDList[ucIndex][i])
        {
            return MSG_OK;
        }

        if (0 == s_ppusMsgGroupMIDList[ucIndex][i])
        {
            s_ppusMsgGroupMIDList[ucIndex][i] = usMID;
            return MSG_OK;
        }
    }

    return ERR_MSG_GROUP_MID_LIST_FULL;
}

/* ע��ģ������Ҫ������鲥���б�:
   �鲥��idΪunsigned short ��, �����ɱ�,
   ����0����.�� :
   MSG_RegModuleGroup(MID_XXX, MID_GROUP_1, MID_GROUP_2, 0);
*/
MSG_RET MSG_RegModuleGroup(unsigned short usMID, ...)
{
    va_list arg_gid;
    unsigned short usGroupID = 0;
    MSG_RET ret = MSG_OK;

    va_start(arg_gid, usMID);
    while (1)
    {
        usGroupID = (unsigned short)va_arg(arg_gid, unsigned long);
        if (0 == usGroupID)
        {
            break;
        }
        ret = MSG_RegGroupModule(usGroupID, usMID);
        if (MSG_OK != ret)
        {
            MSG_ERR(ret, "%04x %04x", usGroupID, usMID);
            break;
        }
    }
    va_end(arg_gid);

    return ret;
}


#ifdef CONFIG_REPLACE_NODE_PREFIX
#define MSG_DbgPrint(fmt, args...) (void)0

ST_MSG *MSG_ReplaceNodePrefix(ST_MSG *pstOldMsg, char *szOldPrefix, char *szNewPrefix)
{
    ST_MSG  *pstNewMsg = NULL;
    int     nOldPrfxLen;
    int     nNewPrfxLen;
    u_long  ulNewMsgBodyLen;
    u_long  ulItemNum = 0;
    u_long  ulIdx;
    u_long  ulMatchNum = 0;
    char    *pOldList = NULL;
    char    *pOldPstn = NULL;
    char    *pNewPstn = NULL;
    char    *pOldPrfx = NULL;
    unsigned long ulResult = 0;

    nOldPrfxLen = strlen(szOldPrefix);
    nNewPrfxLen = strlen(szNewPrefix);

    switch (pstOldMsg->stMsgHead.usMsgType)
    {
        /*
        ˵��: ������Ҫ��Ӧ����Ϣ��ͳһ����,
        ��Ӧ����Ҫ������, һ���ǲ�����ȷʱ����Ҫ�����滻��������Ϣ:
            MSG_CMM_SET_VAL_ACK:
            MSG_CMM_ADD_NODE_ACK:
            MSG_CMM_DEL_NODE_ACK:
            MSG_CMM_SET_ATTR_ACK:
        ��������Ϣ������������ڴ���Ӧ��Ļ�, ��Ҫ����ͳһ�Ĵ�����Ϣ��ʽ�����滻.

        ��һ������������ȷ���Ǵ�����Ҫ���нڵ��滻����Ϣ:
            MSG_CMM_GET_VAL_ACK:
            MSG_CMM_GET_ATTR_ACK:
            MSG_CMM_GET_NOTI_ACK:
            MSG_CMM_GET_NAME_ACK:
            MSG_TR069_GET_MON_EVENT_ACK:
        ����, �����������, �ȶԵ�һ����Ϣ���н�����, �����ȷ, ���˳�;
        ���ʧ��, ��������¸� case ��֮����ͳһ�Ĵ�����Ϣ����.

        */

        case MSG_CMM_SET_VAL_ACK:
        case MSG_CMM_ADD_NODE_ACK:
        case MSG_CMM_DEL_NODE_ACK:
        case MSG_CMM_SET_ATTR_ACK:
        {
            ulResult = *(unsigned long *)(pstOldMsg->szMsgBody);
            if (0 == ulResult)
            {
                pstNewMsg = pstOldMsg;
                pstOldMsg = NULL;
                goto quit;
            }
        } /* ����֮����û��break, ��ο�ǰ���ע�� */

        case MSG_CMM_GET_VAL_ACK:
        case MSG_CMM_GET_ATTR_ACK:
        case MSG_CMM_GET_NOTI_ACK:
        case MSG_CMM_GET_NAME_ACK:
        {
            ulResult = *(unsigned long *)(pstOldMsg->szMsgBody);
            if (0 == ulResult)
            {
                ulItemNum = *(u_long *)(pstOldMsg->szMsgBody +  sizeof(u_long));
                pOldList = pstOldMsg->szMsgBody + 2 * (sizeof(u_long));
            }
            else
            {
                ulItemNum = *(u_long *)(pstOldMsg->szMsgBody + 2 * sizeof(u_long));
                pOldList = pstOldMsg->szMsgBody + 3 * (sizeof(u_long));
            }
            if (0 == ulItemNum)
            {
                pstNewMsg = pstOldMsg;
                pstOldMsg = NULL;
                goto quit;
            }
        }   break;

        case MSG_CMM_GET_VAL:
        case MSG_CMM_SET_VAL:
        case MSG_CMM_GET_ATTR:
        case MSG_CMM_SET_ATTR:
        {
            ulItemNum = *(u_long *)(pstOldMsg->szMsgBody);
            pOldList = pstOldMsg->szMsgBody + sizeof(u_long);
        }   break;

        case MSG_CMM_ADD_NODE:
        case MSG_CMM_DEL_NODE:
        {
            ulItemNum = 1;
            pOldList = pstOldMsg->szMsgBody;
        }   break;

        case MSG_CMM_GET_NAME:
        {
            ulItemNum = 1;
            pOldList = pstOldMsg->szMsgBody + sizeof(u_char) ;
        }   break;

        default:
        {
            pstNewMsg = pstOldMsg;
            pstOldMsg = NULL;
            goto quit;
        }
    }

    /* ͳ��ƥ��ǰ׺�ĸ�������ȷ���Ƿ���Ҫ��������Ϣ���Լ�����Ϣ����Ϣ�峤�� */
    for (ulIdx = 0, pOldPstn = pOldList;
         ulIdx < ulItemNum;
         ulIdx ++)
    {
        while (1)
        {
            pOldPrfx = strstr(pOldPstn, szOldPrefix);
            if (NULL == pOldPrfx)
            {
                pOldPstn += strlen(pOldPstn) + 1;
                break;
            }
            else
            {
                ulMatchNum ++;
                pOldPstn = pOldPrfx + nOldPrfxLen;
            }
        }
    }

    MSG_DbgPrint("ulMatchNum = %lu", ulMatchNum);

    if (0 == ulMatchNum)
    {
        pstNewMsg = pstOldMsg;
        pstOldMsg = NULL;
        goto quit;
    }

    /* ��������Ϣ����Ϣ�峤�� */
    ulNewMsgBodyLen = pstOldMsg->stMsgHead.ulBodyLength + (nNewPrfxLen - nOldPrfxLen) * ulMatchNum;
    MSG_DbgPrint("Calc body len for new msg: %lu + (%d - %d) * %lu = %lu",
               pstOldMsg->stMsgHead.ulBodyLength,
               nNewPrfxLen, nOldPrfxLen, ulMatchNum, ulNewMsgBodyLen);

    pOldPstn = pOldList;
    if (ulNewMsgBodyLen == pstOldMsg->stMsgHead.ulBodyLength)
    {
        for (ulIdx = 0; ulIdx < ulItemNum; ulIdx++)
        {
            while (pOldPrfx = strstr(pOldPstn, szOldPrefix),
                   NULL != pOldPrfx)
            {
                memcpy(pOldPrfx, szNewPrefix, nNewPrfxLen);
                pOldPstn = pOldPrfx + nNewPrfxLen;
            }
            pOldPstn += strlen(pOldPstn) + 1;
        }

        pstNewMsg = pstOldMsg;
        pstOldMsg = NULL;
      goto quit;
    }

    /* ��������Ϣ */
    if (NULL == (pstNewMsg = MSG_CreateMessage(ulNewMsgBodyLen)))
    {
        goto quit;
    }

    /* �������Ϣ */
    memcpy((void *)pstNewMsg, (void *)pstOldMsg, pOldList - (char *)pstOldMsg);
    pstNewMsg->stMsgHead.ulBodyLength = ulNewMsgBodyLen;

    for (ulIdx = 0, pOldPstn = pOldList, pNewPstn = (char *)pstNewMsg + (pOldList - (char *)pstOldMsg);
         ulIdx < ulItemNum;
         ulIdx ++)
    {
        while (1)
        {
            pOldPrfx = strstr(pOldPstn, szOldPrefix);
            if (NULL == pOldPrfx)
            {
                pNewPstn += sprintf(pNewPstn, "%s", pOldPstn) + 1;
                pOldPstn += strlen(pOldPstn) + 1;
                break;
            }
            else
            {
                memcpy(pNewPstn, pOldPstn, pOldPrfx - pOldPstn);
                pNewPstn += pOldPrfx - pOldPstn;
                pNewPstn += sprintf(pNewPstn, "%s", szNewPrefix);

                pOldPstn = pOldPrfx + nOldPrfxLen;
            }
        }
    }

quit:
    return pstNewMsg;
}
#endif




#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif


