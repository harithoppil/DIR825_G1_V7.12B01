/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 �ļ�����: nfp_conntrack.c

 �ļ�����: FPP������ģ��

 �����б�:


 �޶���¼:
           1 ���� : ���ǲ�
             ���� : 2011-04-07
             ���� : ����

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
conntrack������Ŀ,����������ܳ���CONNTRACK_ENTRYNUM_MAX
=========================================================================*/
#define CONNTRACK_ENTRYNUM_MAX      1024
#define NFP_CT_TCP_TIMEOUT	(150 * HZ)
#define NFP_CT_UDP_TIMEOUT	(59 * HZ)


#define NFP_INET_ADDRSTRLEN     40

#define ORIGINAL				0
#define REPLY					1

typedef struct conntrack_entry_tab {

	struct nf_conntrack_tuple_hash tuplehash[IP_CT_DIR_MAX]; /* ��Ԫ�� */
    struct list_head list;              /* h245 5Ԫ���������ڵ�*/

#if 0
    struct rt_entry *route_orig;        /* ·�ɻ��� */
    struct rt_entry *route_reply;

	unsigned short state;
	atomic_t refcnt;                    /*���ü���*/
#endif
#if 0
	struct list_head list_orig;			/* origsrc �� replysrc��ϣ��ͷ  */
	struct list_head list_reply;        /* replysrc �� roigsrc��ϣ��ͷ */
#endif
}conntrack_entry_tab;

union nfp_route_address {
u_int32_t all[NF_CT_TUPLE_L3SIZE];
__be32 ip;
__be32 ip6[4];
};

/*
neighbour���򻺴�״̬��NB_INVALID״̬�����Լ���Ч��
���Ǳ�����ģ�����ã�ֻ���ͷŶ���
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

