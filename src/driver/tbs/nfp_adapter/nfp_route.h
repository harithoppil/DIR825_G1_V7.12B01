/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 �ļ����� : nfp_route.h
 �ļ����� : TBS����������

 �޶���¼ :
          1 ���� : xiachaoren
            ���� : 2011-04-10
            ���� :
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

	struct flowi fl;                    /*��������ָ������*/
	struct interface_entry *oif;        /*output interface*/
	struct interface_entry *phyoif;     /*���oifΪdummyport,phyoif��Ч*/

	int flags;
};

extern int nfp_route_init(void);
extern int nfp_route_exit(void);

#endif
