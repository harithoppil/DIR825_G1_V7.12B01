/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称: cmddaemon.h
 文件描述: cmddaemon模块的头文件

 修订记录:
 
        1. 作者: WuGuoxiang
        
           日期: 2012-09-05
           
           内容: 创建文件
**********************************************************************/
#ifndef _CMDDAEMON_H_
#define _CMDDAEMON_H_

#ifdef CMDDAEMON_DEBUG
    #define CMDDAEMON_TRACE(fmt, args...) COMMON_TRACE(MID_CMDDAEMON, fmt, ##args)
    #define CMDDAEMON_LOG_ERR(fmt, args...) COMMON_TRACE(MID_CMDDAEMON, fmt, ##args)
#else
    #define CMDDAEMON_TRACE(fmt, args...)
    #define CMDDAEMON_LOG_ERR(fmt, args...) COMMON_LOG_ERROR(MID_CMDDAEMON, fmt, ##args)
#endif

#define CMDDAEMON_TRACE_INTO_FUNC CMDDAEMON_TRACE("##In## %s\n", __func__)

#define CMDDAEMON_RET_GOTO(Ret, gotoTag, fmt, args...) \
        if ( RET_FAILED(Ret) )  \
        {\
            CMDDAEMON_TRACE(fmt, ##args); \
            goto gotoTag; \
        }

#define CMDDAEMON_RET_RETURN(Ret, fmt, args...)  \
        RET_RETURN(MID_CMDDAEMON, Ret, fmt, ##args)

#define CMDDAEMON_PTR_GOTO(Pointer,gotoTag, fmt, args...)  \
        POINTER_GOTO(MID_CMDDAEMON, Pointer, gotoTag, fmt, ##args)

#define CMDDAEMON_PTR_RETURN(Pointer, Ret, fmt, args...) \
        if (!Pointer)    \
        { \
            CMDDAEMON_TRACE(fmt, ##args); \
            return Ret; \
        }

#define CMDDAEMON_ASSERT(expr) \
    do{ \
        if(!(expr))\
        {\
            CMDDAEMON_TRACE("Assert \"%s\" failed\n", #expr); \
            exit(-1); \
        } \
    }while(0)

#define CALL_FUCTION_FAILED     "Call function \"%s\" failed\n"

#define SIGF_TERM 0x1
#define SIGF_HUP  0x2

typedef struct tag_ST_CMDDAEMON_CMDINFO {
    char *pszCmd;    
    unsigned int nCmdLen ;
    TBS_ST_LIST_HEAD lhCmdList;
} ST_CMDDAEMON_CMDINFO;

#endif /*_CMDDAEMON_H_*/

