#ifndef _MSG_PRIO_H_
#define _MSG_PRIO_H_

#include <msg_prv.h>

#define MSG_PRIO_NUM 5
#define MAX_MSG_NUM 200

typedef struct
{
    ST_MSG_NODE *pstMsgQueHead,*pstMsgQueTail;
} ST_MSG_QUEUE;

ST_MSG_QUEUE g_astMsgQueue[MSG_PRIO_NUM];

void MSG_Prio_Init();
void MSG_Prio_Uninit();
int MSG_Is_Queue_Empty();

#endif
