/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称 : nfp_route.h
 文件描述 : TBS加速器适配

 修订记录 :
          1 创建 : xiachaoren
            日期 : 2011-04-10
            描述 :
**********************************************************************/

#ifndef _NFP_ROUTE_H_
#define _NFP_ROUTE_H_

#include <linux/timer.h>
#include <asm/atomic.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/jhash.h>
#include <linux/err.h>
#include <linux/sysctl.h>

#include <net/flow.h>
#include "nfp_adapter.h"

#if defined(CONFIG_NFP_ADAPTER_DEBUG)
    #define NFP_ROUTE_DEBUG(fmt, args...) COMMON_TRACE(fmt, ##args)
#else
    #define NFP_ROUTE_DEBUG(fmt, args...)  do { ; } while(0)
#endif

#define NFP_FL_INPUT    0X1
#define NFP_FL_OUTPUT   0X2

#define NFP_ROUTE_TIMEOUT     0X1
#define NFP_ROUTE_UPDATE      0X2


/* rt_entry structure */
struct rt_entry {
    unsigned short family;              /*ipv4/ipv6*/
    unsigned mtu;
    __be32 gateway;

	struct flowi fl;                    /*后续换成指针类型*/
	struct interface_entry *oif;        /*output interface*/
	struct interface_entry *phyoif;     /*如果oif为dummyport,phyoif有效*/

	int flags;
};

extern int nfp_route_init(void);
extern int nfp_route_exit(void);

#endif
