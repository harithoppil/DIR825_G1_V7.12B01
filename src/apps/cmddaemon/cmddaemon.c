/**********************************************************************

 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称: cmddaemon.c

 文件描述:

 修订记录:

        1. 作者: WuGuoxiang

           日期: 2012-09-05

           内容: 创建

**********************************************************************/
#include <sys/time.h>
#include <sys/resource.h>

#include "new_msg.h"
#include "common_func.h"
#include "common_msg_handler.h"
#include "cmddaemon.h"

static int s_iSigFlag = 0;
static int s_iStopFlag = 0;
static TBS_ST_LIST_HEAD s_lhCmdList;

static void CMDDAEMON_PrintCmdList(void)
{
    TBS_ST_LIST_HEAD *plhCmd = NULL;
    ST_CMDDAEMON_CMDINFO *pstCmdInst = NULL;

    /*遍历命令链表*/
    TBS_list_for_each(plhCmd, &s_lhCmdList)
    {
        pstCmdInst = TBS_list_entry(plhCmd, ST_CMDDAEMON_CMDINFO, lhCmdList);
        printf("========================================================================\n");
        printf("CMDDAEMON Command(%p): %s, Length: %d\n",
                pstCmdInst, pstCmdInst->pszCmd, pstCmdInst->nCmdLen);
    }

    printf("========================================================================\n");
    return;
}

/*从Command list获取一个Command实例*/
static ST_CMDDAEMON_CMDINFO *CMDDAEMON_GetCmdInst(void)
{
    TBS_ST_LIST_HEAD *plhCmd = NULL;
    ST_CMDDAEMON_CMDINFO *pstCmdInst = NULL;

    CMDDAEMON_TRACE_INTO_FUNC;

    plhCmd = s_lhCmdList.next;

    /*Command list不为空*/
    if (plhCmd != &s_lhCmdList)
    {
        pstCmdInst = TBS_list_entry(plhCmd, ST_CMDDAEMON_CMDINFO, lhCmdList);
    }
    /*Command list为空*/
    else
    {
        pstCmdInst = NULL;
    }

    return pstCmdInst;
}


/*添加新的Command实例*/
static int CMDDAEMON_AddCmdInst(const char *pszCmd)
{
    ST_CMDDAEMON_CMDINFO *pstCmdInst = NULL;
    int iLen = 0;

    CMDDAEMON_TRACE_INTO_FUNC;

    /*分配命令实例空间*/
    pstCmdInst = (ST_CMDDAEMON_CMDINFO *)malloc(sizeof(ST_CMDDAEMON_CMDINFO));
    if (!pstCmdInst)
    {
        CMDDAEMON_TRACE(CALL_FUCTION_FAILED, "malloc");
        return TBS_FAILED;
    }

    memset(pstCmdInst, 0, sizeof(ST_CMDDAEMON_CMDINFO));

    /*设置Command配置信息*/
    iLen = strlen(pszCmd) + 1;
    pstCmdInst->pszCmd = (char *)malloc(iLen);
    strncpy(pstCmdInst->pszCmd, pszCmd, iLen);
    pstCmdInst->nCmdLen = iLen;

    //printf("cmddaemon_add:%s\n", pstCmdInst->pszCmd);
    /*将结构加入链表*/
    TBS_list_add_tail(&pstCmdInst->lhCmdList, &s_lhCmdList);
    return TBS_SUCCESS;
}

/*删除Command实例*/
static int CMDDAEMON_DelCmdInst(ST_CMDDAEMON_CMDINFO *pstCmdInst)
{
    CMDDAEMON_TRACE_INTO_FUNC;
    if (pstCmdInst)
    {
        TBS_list_del(&pstCmdInst->lhCmdList);
        safe_free(pstCmdInst->pszCmd);
        safe_free(pstCmdInst);
    }

    return TBS_SUCCESS;
}

/*删除所有Command实例*/
static void CMMDAEMON_DestroyCmdList(void)
{
    ST_CMDDAEMON_CMDINFO *pstCmdInst = NULL;

    CMDDAEMON_TRACE_INTO_FUNC;

    while((pstCmdInst = CMDDAEMON_GetCmdInst()))
        CMDDAEMON_DelCmdInst(pstCmdInst);

    return;
}


/*MSG_CMDDAEMON_APPLYCMD_REQUEST消息处理函数*/
static int CMDDAEMON_MsgRequestHandler(ST_MSG *pstMsg)
{
    char szCmd[MAX_CMD_LEN] = {0};
    int iRet = TBS_SUCCESS;

    CMDDAEMON_TRACE_INTO_FUNC;

    /*解析消息内容*/
    iRet = COMM_ParseCustomMsg(pstMsg, "Command", szCmd);
    CMDDAEMON_ASSERT(iRet == TBS_SUCCESS);

    /*见命令加入链表*/
    iRet = CMDDAEMON_AddCmdInst(szCmd);
    CMDDAEMON_RET_RETURN(iRet, "Add Command inst failed...\n");

    return iRet;
}


/*消息处理函数*/
static void CMDDAEMON_MsgProcess(ST_MSG *pstMsg)
{
    CMDDAEMON_TRACE_INTO_FUNC;
    if (pstMsg == NULL)
    {
        CMDDAEMON_TRACE("Null message\n");
        return;
    }

    CMDDAEMON_TRACE("Receive message, SrcMID: %hu, MsgType: %hu\n",
                  pstMsg->stMsgHead.usSrcMID, pstMsg->stMsgHead.usMsgType);

    switch (pstMsg->stMsgHead.usMsgType)
    {
        case MSG_CMDDAEMON_APPLYCMD_REQUEST:
            if (RET_FAILED(CMDDAEMON_MsgRequestHandler(pstMsg)))
            {
                CMDDAEMON_TRACE(CALL_FUCTION_FAILED, "CMDDAEMON_MsgRequestHandler");
            }
            break;

        default:
            CMDDAEMON_TRACE("Unknown message type\n");
            break;
    }

    return;
}

/*捕获信号后的处理函数*/
static void CMDDAEMON_SignalUSR1(int iSignal)
{
    /*打印当前命令链表*/
    CMDDAEMON_PrintCmdList();
    return;
}

/*捕获信号后的处理函数*/
static void CMDDAEMON_SignalHunt(int iSignal)
{
    CMDDAEMON_TRACE_INTO_FUNC;
    switch (iSignal)
    {
        case SIGTERM:
            s_iSigFlag |= SIGF_TERM;
            break;
        case SIGHUP:
            s_iSigFlag |= SIGF_HUP;
            break;
        default:
            CMDDAEMON_TRACE("Unknown signal type: %d\n", iSignal);
            break;
    }

    return;
}

/*正式的信号处理函数*/
static void CMDDAEMON_ProcessSignal(void)
{
    CMDDAEMON_TRACE_INTO_FUNC;
    s_iStopFlag = 1;
}


/*模块初始化*/
static int CMDDAEMON_ModuleInit(void)
{
    int iRet = TBS_SUCCESS;

    CMDDAEMON_TRACE_INTO_FUNC;

    /*注册模块*/
    iRet = MSG_RegModule(MID_CMDDAEMON, (FUN_RECEIVE)CMDDAEMON_MsgProcess);
    CMDDAEMON_RET_RETURN(iRet, CALL_FUCTION_FAILED, "MSG_RegModule");

    MSG_AllModStartOK();
    CMDDAEMON_TRACE("Register module success.\n");

    /*初始化session链表头*/
    TBS_INIT_LIST_HEAD(&s_lhCmdList);

    /*注册信号处理程序*/
    if (signal(SIGTERM, CMDDAEMON_SignalHunt) == SIG_ERR)
    {
        CMDDAEMON_TRACE(CALL_FUCTION_FAILED, "signal");
        return TBS_FAILED;
    }

    if (signal(SIGHUP, CMDDAEMON_SignalHunt) == SIG_ERR)
    {
        CMDDAEMON_TRACE(CALL_FUCTION_FAILED, "signal");
        return TBS_FAILED;
    }

    if (signal(SIGUSR1, CMDDAEMON_SignalUSR1) == SIG_ERR)
    {
        CMDDAEMON_TRACE(CALL_FUCTION_FAILED, "signal");
        return TBS_FAILED;
    }

    return TBS_SUCCESS;
}

/*模块销毁函数*/
static int CMDDAEMON_ModuleDestroy(void)
{
    int iRet = TBS_SUCCESS;

    /*注销模块*/
    iRet = MSG_UnregModule(MID_CMDDAEMON, CMDDAEMON_MsgProcess);
    CMDDAEMON_RET_RETURN(iRet, CALL_FUCTION_FAILED, "MSG_UnregModule");

    CMMDAEMON_DestroyCmdList();

    return TBS_SUCCESS;
}

/*从命令链表中获取一条命令生效*/
static int CMDDAEMON_ApplyCmd(void)
{
    int iRet = TBS_SUCCESS;
    ST_CMDDAEMON_CMDINFO *pstCmdInst = NULL;

    CMDDAEMON_TRACE_INTO_FUNC;

    while ((pstCmdInst = CMDDAEMON_GetCmdInst()))
    {
        //printf("cmddaemon:%s\n", pstCmdInst->pszCmd);
        tbsSystem(pstCmdInst->pszCmd, TBS_PRINT_CMD);
        iRet = CMDDAEMON_DelCmdInst(pstCmdInst);
        CMDDAEMON_RET_RETURN(iRet, "Delete command inst failed...\n");
    }

    return TBS_SUCCESS;
}


/*模块主循环*/
static int CMDDAEMON_MainLoop(void)
{
    ST_MSG *pstMsg = NULL;

    while (!s_iStopFlag)
    {
        /*处理信号*/
        if (s_iSigFlag)
        {
            CMDDAEMON_ProcessSignal();
        }

        /* 等待消息,如果有则立即接受处理 */
        if (MSG_OK != MSG_ReceiveMessage(&pstMsg, 1))
        {
            continue;
        }

        /* 处理接收到的消息，忽略返回 */
        CMDDAEMON_MsgProcess(pstMsg);
        safe_free_msg(pstMsg);

        CMDDAEMON_ApplyCmd();
    }

    return TBS_SUCCESS;
}


/*主函数*/
int main(int argc, char **argv)
{
    int iRet = TBS_SUCCESS;
    int iPrio = 0;

    CMDDAEMON_TRACE_INTO_FUNC;

    iPrio = getpriority(PRIO_PROCESS, 0);

    /*设置优先级比logic低*/
    setpriority(PRIO_PROCESS, 0, iPrio + 1);

    /*模块初始化*/
    iRet = CMDDAEMON_ModuleInit();
    CMDDAEMON_RET_GOTO(iRet, OUT, CALL_FUCTION_FAILED, "CMDDAEMON_ModuleInit");

    /*主循环*/
    iRet = CMDDAEMON_MainLoop();
    CMDDAEMON_RET_GOTO(iRet, OUT, CALL_FUCTION_FAILED, "CMDDAEMON_MainLoop");

OUT:
    /*模块退出前的清理操作*/
    iRet = CMDDAEMON_ModuleDestroy();
    CMDDAEMON_TRACE("Module exit status: %s\n", RET_FAILED(iRet) ? "Failed" : "Success");
    return iRet;
}

/*
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 */
 /*
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 */
  /*
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 */
  /*
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 */
 /*
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 * this is a excess file
 */


