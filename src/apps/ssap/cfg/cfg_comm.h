
/**********************************************************************
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : cfg_comm.h
 文件描述 : cfg模块内部全局公用部分的头文件
 修订记录 :
          1 创建 : 陈跃东
            日期 : 2009-07-27
            描述 :

**********************************************************************/

#include "cfg_api.h"
#include <errno.h>

#ifdef _cplusplus
    #if _cplusplus
        extern "C" {
    #endif
#endif


#define STATIC static


/*********************** 错误报告 ************************/

#ifdef _CFG_DEBUG


#define CFG_ERR(ErrCode, arg...) \
    printf("\n[CFG]: %s %d errno: %04u%04u %s", __FILE__, __LINE__, \
        TBS_ERR_INTERNAL(ErrCode), TBS_ERR_STAND(ErrCode), strerror(errno)); \
    printf(" para: " arg); \
    printf("\n")

#define CFG_LOG(arg...) \
    printf("\n[CFG]: %s %d ",  __FILE__, __LINE__); \
    printf(" log: " arg); \
    printf("\n")

#define CFG_ASSERT(exp) \
{ \
    if (!(exp)) \
    { \
        CFG_ERR(CFG_FAIL, "ASSERT FAILED"); \
        exit(-1); \
    } \
}

#else

#define CFG_ERR(ErrCode, arg...) (void)0

#define CFG_LOG(arg...) (void)0

#define CFG_ASSERT(exp) (void)0

#endif



#define CFG_COMM_ERR_PROC(err, arg...) \
if (CFG_OK != ret) \
{ \
    CFG_ERR(ret, "" arg); \
    return ret; \
}

#define CFG_GOTO_ERR_PROC(ret, tag, arg...) \
if (CFG_OK != ret) \
{ \
    CFG_ERR(ret, "" arg); \
    goto tag; \
}



#ifdef _cplusplus
    #if _cplusplus
        }
    #endif
#endif


