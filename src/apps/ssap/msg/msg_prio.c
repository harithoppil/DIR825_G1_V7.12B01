#include "msg_prio.h"
#include "tbstype.h"

#define true 1
#define false 0

typedef int bool;

void MSG_Prio_Init()
{
    memset(g_astMsgQueue,0,sizeof(g_astMsgQueue));
}

void MSG_Prio_Uninit()
{
    int i=0;

    ST_MSG_NODE *pstMsgNode = NULL;
    ST_MSG_NODE *pstMsgQueHead=NULL;

    for(i=MSG_PRIO_NUM-1;i>=0;i--)
    {
        pstMsgQueHead = g_astMsgQueue[i].pstMsgQueHead;
        pstMsgNode = pstMsgQueHead;

        if (NULL != pstMsgQueHead)
        {
            pstMsgQueHead = pstMsgQueHead->pstNext;
            free(pstMsgNode);
        }
    }

    memset(g_astMsgQueue,0,sizeof(g_astMsgQueue));
}


int MSG_Is_Queue_Empty()
{
    int i=0;

    for(i=0;i<MSG_PRIO_NUM;i++)
    {
        if(g_astMsgQueue[i].pstMsgQueHead)
            return false;
    }
    return true;
}

