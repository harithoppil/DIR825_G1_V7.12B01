/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称: nfp_conntrack.c

 文件描述: FPP适配子模块

 函数列表:


 修订记录:
           1 作者 : 王亚波
             日期 : 2011-04-07
             描述 : 创建

**********************************************************************/

#ifndef _NFP_CONNTRACK_H_
#define _NFP_CONNTRACK_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/notifier.h>
#include <linux/jhash.h>

#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/route.h>
#ifdef CONFIG_NF_NAT_NEEDED
#include <net/netfilter/nf_nat_core.h>
#include <net/netfilter/nf_nat_protocol.h>
#endif

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>
#include <asm/atomic.h>
#include "nfp_route.h"

/*=========================================================================
conntrack连接条目,最大数量不能超过CONNTRACK_ENTRYNUM_MAX
=========================================================================*/
#define CONNTRACK_ENTRYNUM_MAX      1024
#define NFP_CT_TCP_TIMEOUT	(150 * HZ)
#define NFP_CT_UDP_TIMEOUT	(59 * HZ)


#define NFP_INET_ADDRSTRLEN     40

#define ORIGINAL				0
#define REPLY					1

typedef struct conntrack_entry_tab {

	struct nf_conntrack_tuple_hash tuplehash[IP_CT_DIR_MAX]; /* 五元组 */
    struct list_head list;              /* h245 5元组过滤链表节点*/

#if 0
    struct rt_entry *route_orig;        /* 路由缓存 */
    struct rt_entry *route_reply;

	unsigned short state;
	atomic_t refcnt;                    /*引用计数*/
#endif
#if 0
	struct list_head list_orig;			/* origsrc 与 replysrc哈希表头  */
	struct list_head list_reply;        /* replysrc 与 roigsrc哈希表头 */
#endif
}conntrack_entry_tab;

union nfp_route_address {
u_int32_t all[NF_CT_TUPLE_L3SIZE];
__be32 ip;
__be32 ip6[4];
};

/*
neighbour规则缓存状态，NB_INVALID状态代表以及无效，
但是被其他模块引用，只待释放而已
*/
enum CON_STAT{
	CON_INVALID 		= 0x0,
    CON_ACTIVE       = 0x1,
};

struct delay_conntrack_event_t {
    struct list_head list;
	struct conntrack_entry_tab ct;
    unsigned long event;
    struct delayed_work work;
};


typedef int (*nfp_ct_parser_fun)(unsigned short, const struct conntrack_entry_tab *);

extern int nfp_ct_parser_register(nfp_ct_parser_fun new);
extern void nfp_ct_parser_unregister(nfp_ct_parser_fun new);
void nfp_conntrack_reset(void);

void nfp_conntrack_exit(void);
int nfp_conntrack_init(void);

/*********************************************************************
 *                              DEBUG                                *
 *********************************************************************/

#define NFP_CONNTRACK_ASSERT(i)   BUG_ON(!(i))

#endif

