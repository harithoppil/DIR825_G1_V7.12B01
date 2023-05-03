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
#include <linux/inet.h>
#include <linux/netfilter.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <linux/list_nulls.h>
#ifdef CONFIG_NF_NAT_NEEDED
#include <net/netfilter/nf_nat_core.h>
#include <net/netfilter/nf_nat_protocol.h>
#endif
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

#include "nfp_conntrack.h"
#include "nfp_adapter.h"


//DEFINE_RWLOCK(nfp_conntrack_lock);
static spinlock_t nfp_conntrack_splock;
//DEFINE_RWLOCK(nfp_ct_filter_lock);
static spinlock_t nfp_filter_splock;
static spinlock_t nfp_h245_splock;

struct hlist_nulls_head filter_head;
struct list_head h245_ct_list = LIST_HEAD_INIT(h245_ct_list);
/*=========================================================================
缓存中conntrack规则计数器
=========================================================================*/
static unsigned int conntrack_entry_num = 0;

static struct list_head delayed_work_head = LIST_HEAD_INIT(delayed_work_head);
static spinlock_t ct_delayed_list_lock;
static atomic_t module_exit;
#define DELAY_TIME  3

static struct kmem_cache *nfp_ct_cache __read_mostly;
static int nfp_ct_delete(conntrack_entry_tab *ct);
static int nfp_ct_add(conntrack_entry_tab *ct);


/*=========================================================================
 Function:		static inline void nfp_ct_dump(conntrack_entry_tab *ct)
 Description:		打印节点信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
static inline void nfp_ct_dump(conntrack_entry_tab *entry)
{
#if defined(CONFIG_NFP_ADAPTER_DEBUG)
    u_int16_t family;
    const char *formart = NULL;
    char addr_dst_ori[NFP_INET_ADDRSTRLEN] = {0};
    char addr_src_ori[NFP_INET_ADDRSTRLEN] = {0};
    char addr_dst_rep[NFP_INET_ADDRSTRLEN] = {0};
    char addr_src_rep[NFP_INET_ADDRSTRLEN] = {0};
    family = entry->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;

    if(entry->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num == AF_INET)
        formart = "%pI4";
    else
        formart = "%pI6";

    sprintf(addr_dst_ori, formart, entry->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all);
    sprintf(addr_src_ori, formart, entry->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all);
    sprintf(addr_dst_rep, formart, entry->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.all);
    sprintf(addr_src_rep, formart, entry->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.all);

    printk("family\tsaddr\t\tdaddr\t\tsport\tdport\tproto\trep_saddr\t"
        "rep_daddr\tr_sport\tr_dport\n");
    printk("%s\t%-16s%-16s%hu\t%hu\t0x%x\t"
                "%-16s%-16s%hu\t%hu\n",
                ((family == AF_INET6)?"AF_INET6":"AF_INET"),
                addr_src_ori, addr_dst_ori,
                ntohs(entry->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all),
                ntohs(entry->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all),
                entry->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum,
                addr_src_rep, addr_dst_rep,
                ntohs(entry->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all),
                ntohs(entry->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all));
#endif

    return;
}


#ifdef CONFIG_TBS_NFP_CT
extern int (*tbs_nfp_hook_ct_nat_age)(struct nf_conn *ct);
#endif

struct conntrack_entry_tab * __nfp_ct_h245_filter_find(struct nf_conn *ct)
{
	struct conntrack_entry_tab *h;
    int family = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;

    if(list_empty(&h245_ct_list))
        return NULL;

	list_for_each_entry(h, &h245_ct_list, list)
	{
        if(AF_INET == family)
        {
            if (h && h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip ==
                            ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip &&
                  (h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all ==
                      ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all) &&
                  (h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num ==
                      ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num) &&
                  h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip ==
                         ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip &&
                   (h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all ==
                      ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all))
            {
                return h;
            }
        }
        else
        {
    		if (h && !memcmp(h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all,
    			            ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all,
    			            sizeof(h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all)) &&
    			  (h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all ==
    			      ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all) &&
    			  (h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num ==
    			      ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num) &&
    			  !memcmp(h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all,
    			         ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all,
    			         sizeof(h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all)) &&
    			   (h->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all ==
    			      ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all))
    		{
    			return h;
    		}
        }
	}

    return NULL;
}

int nfp_ct_h245_filter_add(struct nf_conn *ct)
{
    int iret = -1;
    conntrack_entry_tab *h245_ct = NULL;

    //return 0;

    spin_lock(&nfp_h245_splock);

    h245_ct = __nfp_ct_h245_filter_find(ct);

    if(h245_ct) {
        NFP_ADAPTER_DEBUG("h245 ct filter exist\n");
        //nfp_ct_dump(h245_ct);
        goto out;
    }

    h245_ct = kmalloc(sizeof(conntrack_entry_tab), GFP_ATOMIC);
    if(!h245_ct)
        goto out;

    memset(h245_ct, 0, sizeof(conntrack_entry_tab));

    h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num  =  ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;
    h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
    h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all  =  ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
    h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all  =  ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
    memcpy(h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all,
           ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all, sizeof(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all));
    memcpy(h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all,
           ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all, sizeof(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all));

    h245_ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.l3num  =  ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.l3num;
    h245_ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.protonum = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.protonum;
    h245_ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all  =  ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all;
    h245_ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all  =  ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all;
    memcpy(h245_ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.all,
           ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.all, sizeof(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.all));
    memcpy(h245_ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.all,
           ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.all, sizeof(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.all));

    list_add(&h245_ct->list,&h245_ct_list);

    NFP_ADAPTER_DEBUG("h245 ct filter add\n");
    //nfp_ct_dump(h245_ct);

    iret = 0;
out:

    spin_unlock(&nfp_h245_splock);

    return 0;
}

int nfp_ct_h245_filter_delete(struct nf_conn *ct)
{
    int iret = -1;
    conntrack_entry_tab *h245_ct = NULL;

    spin_lock(&nfp_h245_splock);
    h245_ct = __nfp_ct_h245_filter_find(ct);

    if(h245_ct) {
        NFP_ADAPTER_DEBUG("h245 ct filter delete\n");
        //nfp_ct_dump(h245_ct);
        list_del(&h245_ct->list);
        kfree((void *)h245_ct);

        iret = 0;
    }
    spin_unlock(&nfp_h245_splock);

    return iret;
}

static bool nfp_ct_is_h245_filter_entry(struct nf_conn *ct)
{
    bool exist = false;

    spin_lock(&nfp_h245_splock);
    if(__nfp_ct_h245_filter_find(ct)){
        exist = true;
    }
    spin_unlock(&nfp_h245_splock);

    return exist;
}

static void nfp_ct_h245_filter_init(void)
{
    spin_lock_init(&nfp_h245_splock);
    nf_ct_h245_filter_hook = nfp_ct_h245_filter_add;
}

static void nfp_ct_h245_filter_exit(void)
{
    struct conntrack_entry_tab *h245_ct, *next;

    nf_ct_h245_filter_hook = NULL;

    spin_lock(&nfp_h245_splock);
    /*遍历hash_table[hash]链表，从链表中移除节点，并释放空间*/
    list_for_each_entry_safe(h245_ct, next, &h245_ct_list, list) {
        list_del(&h245_ct->list);
        kfree((void *)h245_ct);
    }
    spin_unlock(&nfp_h245_splock);
}

static inline int nfp_ct_option(unsigned short option, conntrack_entry_tab *ct)
{
    int ret = NFP_UNABLE_PARSER;

    if(nfp_cmd_parser){
        struct nfp_adapter_cmd cmd_entry = {option, (void *)ct};
        ret = nfp_cmd_parser(NFP_CMD_CT, &cmd_entry);
    }
    else if(NFP_OPT_STAT == option)
        ret = NFP_RULE_TIMEOUT;

    return ret;
}

inline static void nfp_ct_delay_work_list_init(void)
{
    INIT_LIST_HEAD(&delayed_work_head);
    spin_lock_init(&ct_delayed_list_lock);
}


static void nfp_ct_delay_work_list_clean(void)
{
    struct delay_conntrack_event_t *ct_delay, *next;

    spin_lock_bh(&ct_delayed_list_lock);

    /*遍历hash_table[hash]链表，从链表中移除节点，并释放空间*/
    list_for_each_entry_safe(ct_delay, next, &delayed_work_head, list) {

        //nfp_ct_dump(&ct_delay->ct);
        cancel_delayed_work_sync(&ct_delay->work);
        list_del(&ct_delay->list);
        kfree((void *)ct_delay);
    }

    spin_unlock_bh(&ct_delayed_list_lock);
}

static void nfp_ct_delay_worker_func(struct work_struct *work)
{
    int ret = 0;
    unsigned long events = 0;
    struct delay_conntrack_event_t *ct_event_entry;
    conntrack_entry_tab *nfp_ct = NULL;
    struct delayed_work *delay_work = NULL;

    NFP_ADAPTER_INTO_FUNC;

    if(NULL == work)
        return;

    spin_lock_bh(&ct_delayed_list_lock);

    delay_work = container_of(work, struct delayed_work, work);
    ct_event_entry = container_of(delay_work, struct delay_conntrack_event_t, work);

    /*模块退出，直接返回*/
    if(atomic_read(&module_exit)){
        spin_unlock_bh(&ct_delayed_list_lock);
        return;
    }

    /*从延后任务链表中移除*/
    list_del(&ct_event_entry->list);
    spin_unlock_bh(&ct_delayed_list_lock);

    nfp_ct = &ct_event_entry->ct;
    events = ct_event_entry->event;

    NFP_ADAPTER_DEBUG("delayed working:\n");
    //nfp_ct_dump(nfp_ct);

    if (events & 1UL<<IPCT_DESTROY){
        ret = nfp_ct_delete(nfp_ct);
    }
    else if (events & (1UL<<IPCT_NEW | 1UL<<IPCT_RELATED)){
        ret = nfp_ct_add(nfp_ct);
    }
    else  if (events & (1UL<<IPCT_ASSURED | 1UL<<IPCT_PROTOINFO)) {
        ret = nfp_ct_add(nfp_ct);
    }
    else
        goto out;

    if(ret)
        NFP_ADAPTER_DEBUG("event: %lu, return %d", events, ret);

out:
    /*释放延后工作队列任务节点*/
    kfree((void *)ct_event_entry);

    return;
}

#ifdef CONFIG_NF_CONNTRACK_EVENTS

/*
 * TBS_TAG: add by baiyonghui 2011-12-1
 * Description:
 */
/*=========================================================================
 Function:		static int nfp_ct_nat_age(struct nf_conn *ct)
 Description:		根据目的IP取hash
 Data Accessed:
 Data Updated:
 Input:			dst_ip: 目的IP
 Output:			无
 Return:			-1:失败

 Others:
=========================================================================*/
static inline int nfp_ct_nat_age(struct nf_conn *ct)
{
	conntrack_entry_tab ct_entry_tab;

    memset(&ct_entry_tab, 0, sizeof(ct_entry_tab));
	memcpy(ct_entry_tab.tuplehash, ct->tuplehash, sizeof(ct_entry_tab.tuplehash));

	/*加速器中规则老化查询*/
	if(NFP_RULE_TIMEOUT == nfp_ct_option(NFP_OPT_STAT, &ct_entry_tab))
		return 0;

	return NFP_RULE_ACTIVE;
}
/*
 * TBS_END_TAG
 */

inline static int nfp_conntrack_ip_commpare(int family, const u_int32_t *saddr,
                                                 const u_int32_t *daddr)
{
    NFP_ASSERT(saddr && daddr);

    if(AF_INET == family)
    {
        return !(*saddr == *daddr);
    }
    else if(AF_INET6 == family)
    {
        return !(saddr[0] == daddr[0] &&
            saddr[1] == daddr[1] &&
            saddr[2] == daddr[2] &&
            saddr[3] == daddr[3]);
    }
    else
        return -1;
}

/*=========================================================================
 Function:		static int nfp_ct_add(const struct nf_conn *ct)
 Description:		加速器规则NFP添加
 Data Accessed:
 Data Updated:
 Input:			ct: 要添加的新规则
 Output:			无
 Return:			0:成功
				-1:规则已经存在,添加失败或其它
 Others:
=========================================================================*/
static int nfp_ct_add(conntrack_entry_tab *ct)
{
	int iRet = 0;

	NFP_ADAPTER_INTO_FUNC;
	NFP_ASSERT(ct);

	/* 生效规则到加速器 */
	iRet = nfp_ct_option(NFP_OPT_ADD, ct);
	if(NFP_UNABLE_PARSER != iRet && NFP_NO_ERR != iRet)
	{
		//nfp_ct_dump(ct);
	//	NFP_ADAPTER_ERROR("call nfp_ct_option(NFP_OPT_ADD, ct) error: %d\n", iRet);
	}

	return iRet;
}

#if 0
static int nfp_ct_update(conntrack_entry_tab *ct)
{
    return 0;
}
#endif

/*=========================================================================
 Function:		static int nfp_ct_delete(const struct nf_conn *ct)
 Description:		加速器规则NFP删除
 Data Accessed:
 Data Updated:
 Input:			ct: 要删除的新规则
 Output:			无
 Return:			0:成功
				-1或其它:删除失败
 Others:
=========================================================================*/
static int nfp_ct_delete(conntrack_entry_tab *ct)
{
	int iRet = 0;

	NFP_ADAPTER_INTO_FUNC;
	NFP_ASSERT(ct);

	/* 删除加速器相应规则 */
	iRet = nfp_ct_option(NFP_OPT_DELETE, ct);
	if(NFP_UNABLE_PARSER != iRet && NFP_NO_ERR != iRet)
	{

		//nfp_ct_dump(ct);
		NFP_ADAPTER_ERROR("call nfp_ct_option(NFP_OPT_DELETE, ct) error: %d\n", iRet);
	}

	return iRet;
}



/*=========================================================================
 Function:		int nfp_conntrack_event(int event, const struct nf_conn *ct)
 Description:		定义hook响应conntrack中通知链事件
 Data Accessed:
 Data Updated:
 Input:			event: 通知链事件
 					ct: 要操作的规则
 Output:			无
 Return:			0:成功
				-1或其它:失败
 Others:
=========================================================================*/
int nfp_conntrack_event(unsigned long events, struct nf_conn *ct)
{
	int ret = 0;
	u_int16_t family;
	conntrack_entry_tab nfp_ct;

	memset(&nfp_ct, 0, sizeof(nfp_ct));

	nfp_ct.tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num  =  ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;
	nfp_ct.tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
	nfp_ct.tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all  =  ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
	nfp_ct.tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all  =  ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
    memcpy(nfp_ct.tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all,
		   ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all, sizeof(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all));
	memcpy(nfp_ct.tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all,
		   ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all, sizeof(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all));

	nfp_ct.tuplehash[IP_CT_DIR_REPLY].tuple.src.l3num  =  ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.l3num;
	nfp_ct.tuplehash[IP_CT_DIR_REPLY].tuple.dst.protonum = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.protonum;
	nfp_ct.tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all  =  ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all;
	nfp_ct.tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all  =  ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all;
    memcpy(nfp_ct.tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.all,
		   ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.all, sizeof(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.all));
	memcpy(nfp_ct.tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.all,
		   ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.all, sizeof(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.all));
    /* 当不是LAN_WAN同时做NAT时，需要过滤掉发往本机连接 */
	family = nfp_ct.tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;

	if ((ct->status & IPS_NAT_MASK) != IPS_NAT_MASK )
	{
		//NFP_ADAPTER_DEBUG(" the status isn't IPS_NAT_DONE_MASK\n");
		if (family == AF_INET)
		{
			if ( (inet_addr_type(&init_net,nfp_ct.tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip) == RTN_LOCAL &&
				  inet_addr_type(&init_net,nfp_ct.tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip) == RTN_LOCAL) ||
				 (inet_addr_type(&init_net,nfp_ct.tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip) == RTN_LOCAL &&
				  inet_addr_type(&init_net,nfp_ct.tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip) == RTN_LOCAL) )
			{
				return ret;
			}
		}
	}

    /*http流量延后处理，解决tr069认证无效，url过滤无效的问题*/
    if(SOL_TCP == nfp_ct.tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum &&
        (nfp_ct.tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all == htons(80) ||
        nfp_ct.tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all == htons(80)))
    {
        struct delay_conntrack_event_t *http_delay_event = NULL;

        http_delay_event = (void *)kmalloc(sizeof(struct delay_conntrack_event_t), GFP_ATOMIC);
        if(!http_delay_event)
        {
            NFP_ADAPTER_ERROR("kmalloc buf fail!\n");

            return -1;
        }

        memset(http_delay_event, 0, sizeof(struct delay_conntrack_event_t));
        memcpy(&http_delay_event->ct, &nfp_ct, sizeof(struct conntrack_entry_tab));
        http_delay_event->event = events;

        INIT_DELAYED_WORK_DEFERRABLE(&http_delay_event->work, nfp_ct_delay_worker_func);

        NFP_ADAPTER_DEBUG("add delayed(%ds) work:\n", DELAY_TIME);
        //nfp_ct_dump(&http_delay_event->ct);

        /*延后工作队列链表加锁，并添加节点*/
        spin_lock_bh(&ct_delayed_list_lock);
        list_add(&http_delay_event->list, &delayed_work_head);
        spin_unlock_bh(&ct_delayed_list_lock);

        schedule_delayed_work(&http_delay_event->work, DELAY_TIME*HZ);

        return 0;
    }

#if defined(CONFIG_TBS_NFP_MARVELL_ADAPTER) || defined(CONFIG_TBS_NFP_TBS_ADAPTER)
    if (events & 1UL<<IPCT_DESTROY){
        ret = nfp_ct_delete(&nfp_ct);
#ifdef CONFIG_TBS_NFP_CT
		ct->tbs_nfp = false;
#endif
    }
    else if (events & (1UL<<IPCT_NEW | 1UL<<IPCT_RELATED)){
        ret = nfp_ct_add(&nfp_ct);
#ifdef CONFIG_TBS_NFP_CT
		ct->tbs_nfp = true;
#endif
    }
    else  if (events & (1UL<<IPCT_ASSURED| 1UL<<IPCT_PROTOINFO)) {
        ret = nfp_ct_add(&nfp_ct);
#ifdef CONFIG_TBS_NFP_CT
		ct->tbs_nfp = true;
#endif
    }
    else
        return ret;

#endif
	    return ret;
}

#endif

MODULE_ALIAS("nfp_adapter");
MODULE_ALIAS_NFNL_SUBSYS(NFNL_SUBSYS_ctnfp);
MODULE_ALIAS_NFNL_SUBSYS(NFNL_SUBSYS_ctnfp_EXP);


/*=========================================================================
 Function:		static int display_nfp_filter_entry(char *buf, char **start, off_t offset, int count, int *eof, void *data)
 Description:		显示nfp加速器规则表
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/

static int display_nfp_filter(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	struct nf_conntrack_tuple_hash *pos;
	struct hlist_nulls_node *n;
    conntrack_entry_tab *h245_ct = NULL;
	u_int8_t family;
	const u_int32_t *ip_addr;
	char addr_dst[NFP_INET_ADDRSTRLEN] = {0};
	char addr_src[NFP_INET_ADDRSTRLEN] = {0};
	char addr_dst_rep[NFP_INET_ADDRSTRLEN] = {0};
	char addr_src_rep[NFP_INET_ADDRSTRLEN] = {0};

    char *out = buf;
    int len = 0;

    //out += sprintf(out, "family\tsaddr\t\tdaddr\t\tsport\tdport\tproto\n");
    printk("family\tsaddr\t\tdaddr\t\tsport\tdport\tproto\n");

	//printk("####list the filter table####\n");
	spin_lock_bh(&nfp_filter_splock);
    hlist_nulls_for_each_entry(pos,n,&filter_head, hnnode)
    {
        family = pos->tuple.src.l3num;

        ip_addr = pos->tuple.dst.u3.all;
        NFP_INET_NTOP(family, ip_addr, addr_dst, NFP_INET_ADDRSTRLEN);
        ip_addr = pos->tuple.src.u3.all;
        NFP_INET_NTOP(family, ip_addr, addr_src, NFP_INET_ADDRSTRLEN);
        printk("%s\t%-16s%-16s%hu\t%hu\t0x%x\n",
                ((family == AF_INET6)?"AF_INET6":"AF_INET"),
                addr_src, addr_dst,
                ntohs(pos->tuple.src.u.all),
                ntohs(pos->tuple.dst.u.all),
                pos->tuple.dst.protonum);


    }
    spin_unlock_bh(&nfp_filter_splock);

    printk("h245 ct filter:\n");
    printk("family\tsaddr\t\tdaddr\t\tsport\tdport\tproto\trep_saddr\t"
        "rep_daddr\tr_sport\tr_dport\n");

	spin_lock(&nfp_h245_splock);
    list_for_each_entry(h245_ct, &h245_ct_list, list)
    {
        family = h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;

        ip_addr = h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all;
        NFP_INET_NTOP(family, ip_addr, addr_dst, NFP_INET_ADDRSTRLEN);
        ip_addr = h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all;
        NFP_INET_NTOP(family, ip_addr, addr_src, NFP_INET_ADDRSTRLEN);
        ip_addr = h245_ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.all;
        NFP_INET_NTOP(family, ip_addr, addr_dst_rep, NFP_INET_ADDRSTRLEN);
        ip_addr = h245_ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.all;
        NFP_INET_NTOP(family, ip_addr, addr_src_rep, NFP_INET_ADDRSTRLEN);

        printk("%s\t%-16s%-16s%hu\t%hu\t0x%x\t"
                    "%-16s%-16s%hu\t%hu\n",
                    ((family == AF_INET6)?"AF_INET6":"AF_INET"),
                    addr_src, addr_dst,
                    ntohs(h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all),
                    ntohs(h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all),
                    h245_ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum,
                    addr_src_rep, addr_dst_rep,
                    ntohs(h245_ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all),
                    ntohs(h245_ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all));
    }
    spin_unlock(&nfp_h245_splock);

    len = out - buf;
    len -= offset;
    if (len < count){
        *eof = 1;
        if (len <= 0)
            return 0;
    }
    else
        len = count;
    *start = buf + offset;

    return len;
}


/*=========================================================================
 Function:		static int nfp_filter_list_destory()

 Description:		删除filter_list链表中所有节点并且释放内存
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int nfp_filter_list_destory(void)
{
	struct nf_conntrack_tuple_hash *pos;
	struct hlist_nulls_node *n;

    hlist_nulls_for_each_entry(pos,n,&filter_head, hnnode)
	{
		hlist_nulls_del(&(pos->hnnode));
		kfree(pos);
	}

	return 0;
}


/*=========================================================================
 Function:		static int write_nfp_filter(char *buf, char **start, off_t offset, int count, int *eof, void *data)
 Description:		写入值到proc文件系统节点,用户空间写该文件
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/

static int write_nfp_filter( struct file *filp, const char __user *buf,unsigned long len, void *data )
{
	int ret = 0;
    char str_buf[512] = {0};
	char src_ip[16] = {0}, dst_ip[16] = {0};
	u_int16_t src_port = 0, dst_port = 0;
	char protonum[5] = {0};
	struct nf_conntrack_tuple_hash *input_filter;

	NFP_ADAPTER_INTO_FUNC;
	/* 限定一条输入长度不大于512 */
	if(len > 512)
	{
	    NFP_ADAPTER_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

		/* 当仅输入字符0时，清除链表中所有内容，并释放节点内存 */
		if (str_buf[0] == '0' && len < 5)
		{
			NFP_ADAPTER_DEBUG("flush the nfp filter table\n");
			nfp_filter_list_destory();
			goto out;
		}

        ret = sscanf(str_buf, "%s %hu %s %hu %s ", src_ip,
					 	 						 &src_port,
                         						 dst_ip,
                         						 &dst_port,
                         						 protonum);
        if(ret != 5)
		{
            printk("Write_proc: Need 5 parameters to write in\n");
            goto out;
        }
    }

	input_filter = (void *)kmalloc(sizeof(struct nf_conntrack_tuple_hash), GFP_ATOMIC);
	if (!input_filter)
	{
		NFP_ADAPTER_ERROR("kmalloc input_filter err\n");
	    goto out;
	}

	input_filter->tuple.src.l3num = AF_INET;

	if (*src_ip == '*')
		input_filter->tuple.src.u3.ip = 0;
	else
		input_filter->tuple.src.u3.ip = in_aton(src_ip);

	if (*dst_ip == '*')
		input_filter->tuple.dst.u3.ip = 0;
	else
		input_filter->tuple.dst.u3.ip = in_aton(dst_ip);

	input_filter->tuple.src.u.all = htons(src_port);
	input_filter->tuple.dst.u.all = htons(dst_port);

	if (!memcmp(protonum, "tcp", 3) || !memcmp(protonum, "TCP", 3))
	{
        input_filter->tuple.dst.protonum = IPPROTO_TCP;
	}
	else if (!memcmp(protonum, "udp", 3) || !memcmp(protonum, "UDP", 3))
	{
       input_filter->tuple.dst.protonum = IPPROTO_UDP;
	}
	else
	{
        printk("the protonum is invalid \n");
        goto out;
	}

    spin_lock_bh(&nfp_filter_splock);
	hlist_nulls_add_head(&input_filter->hnnode,&filter_head);
    spin_unlock_bh(&nfp_filter_splock);
	NFP_ADAPTER_OUT_FUNC;

out:

    return len;
}

/*=========================================================================
 Function:		static int nfp_proc_filter_init( void )
 Description:		加速器过滤proc接口初始化
 Data Accessed:

 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功

 Others: 			建立/proc/fpp_filter文件,只读
=========================================================================*/

static int nfp_proc_filter_init( void )
{
    struct proc_dir_entry *proc_filter = NULL;

    proc_filter = create_proc_entry( "nfp_filter", 0666, nfp_adapter_proc );
	if (proc_filter)
	{
	    proc_filter->read_proc = &display_nfp_filter;
	    proc_filter->write_proc= &write_nfp_filter;
	}

	return 0;
}

#ifdef CONFIG_NF_CONNTRACK_EVENTS


/*=========================================================================
 Function:		static int nfp_ct_event_handle(struct notifier_block *this,
				     unsigned long events, void *ptr)
 Description:		事件勾子处理函数
 Data Accessed:

 Data Updated:
 Input:			events    添加/删除事件
 Output:			无
 Return:			0:成功

 Others:
=========================================================================*/
//int (*fcn)(unsigned int events, struct nf_ct_event *item);
static int nfp_ct_event_handle(unsigned int events, struct nf_ct_event *item)
{

	struct nf_conn *ct = NULL;
	struct nf_conntrack_tuple_hash *pos;
	struct hlist_nulls_node *n;
    u_int8_t protonum = 0;
    const struct nf_conn_help *help;

	ct =item->ct;
    //NFP_ADAPTER_DEBUG("events = 0x%08x, return NOTIFY_DONE\n", events);

	/* ignore our fake conntrack entry */
	if (ct == &nf_conntrack_untracked)
		return NOTIFY_DONE;
    help = nfct_help(ct);

    //alg 信令不加速
    if (help && help->helper){
        return NOTIFY_DONE;
    }

    if(conntrack_entry_num > CONNTRACK_ENTRYNUM_MAX) {
        return NOTIFY_DONE;
    }

    if (!(events &
        (1UL<<IPCT_DESTROY | 1UL<<IPCT_NEW | 1UL<<IPCT_RELATED | \
        1UL<<IPCT_ASSURED | 1UL<<IPCT_PROTOINFO))){

        //NFP_ADAPTER_DEBUG("events = 0x%08x, return NOTIFY_DONE\n", events);
        return NOTIFY_DONE;
    }

    /*只处理TCP/UDP流量*/
    if(SOL_TCP != ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum &&
        SOL_UDP != ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum)
    {
        return NOTIFY_DONE;
    }

    /*过滤多播、广播地址*/
    if(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num == AF_INET &&
        (ipv4_is_multicast(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip) ||
        ipv4_is_multicast(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip) ||
        ipv4_is_lbcast(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip) ||
        ipv4_is_lbcast(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip)))
    {
        return NOTIFY_DONE;
    }

    protonum = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
    if(SOL_TCP == protonum)
    {
        u_int8_t tcp_stat = ct->proto.tcp.state;
        if(!(events & 1UL<<IPCT_DESTROY) &&
            TCP_CONNTRACK_ESTABLISHED != tcp_stat)
        {
            return NOTIFY_OK;
        }

        /*h245业务过滤*/
        if(events & 1UL<<IPCT_DESTROY)
        {
            nfp_ct_h245_filter_delete(ct);
        }
        else
        {
            if(nfp_ct_is_h245_filter_entry(ct)){
                return NOTIFY_OK;
            }
        }
    }

	/* 过滤掉指定不走加速业务 */
	//list_for_each_entry(pos, &filter_head, list)
    hlist_nulls_for_each_entry(pos,n,&filter_head, hnnode)
	{
		if (pos->tuple.dst.protonum != 0 &&
			pos->tuple.dst.protonum != ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum)
			continue;

		if (pos->tuple.src.l3num == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num)
		{
			if (pos->tuple.src.u3.ip != 0 &&
				(nfp_conntrack_ip_commpare(pos->tuple.src.l3num,
									  pos->tuple.src.u3.all,
									  ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.all) &&
                nfp_conntrack_ip_commpare(pos->tuple.src.l3num,
									  pos->tuple.src.u3.all,
									  ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.all)))
				continue;

			if (pos->tuple.dst.u3.ip != 0 &&
				(nfp_conntrack_ip_commpare(pos->tuple.src.l3num,
									  pos->tuple.dst.u3.all,
									  ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.all) &&
				nfp_conntrack_ip_commpare(pos->tuple.src.l3num,
									  pos->tuple.dst.u3.all,
									  ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.all)))
				continue;
		}
        else
            continue;

		if (pos->tuple.src.u.all != 0 &&
			(pos->tuple.src.u.all != ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all &&
			pos->tuple.src.u.all != ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all))
			continue;

		if (pos->tuple.dst.u.all != 0 &&
			(pos->tuple.dst.u.all != ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all &&
			pos->tuple.dst.u.all != ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all))
			continue;

        return NOTIFY_OK;
	}

	nfp_conntrack_event(events, ct);

	return NOTIFY_OK;
}

/* 定义勾子 */
static struct nf_ct_event_notifier nfp_ct_notifier = {
	.fcn = nfp_ct_event_handle,
};

#endif



/*=========================================================================
 Function:		static void nfp_ct_proc_exit( void )
 Description:		加速器状态查询proc接口删除
 Data Accessed:

 Data Updated:
 Input:			无
 Output:			无
 Return:			无

 Others: 			删除/proc/nfp_conntrack文件
=========================================================================*/

static void nfp_ct_proc_exit( void )
{
    remove_proc_entry("nfp_conntrack", nfp_adapter_proc);
}

/*=========================================================================
 Function:		static void nfp_filter_proc_exit( void )
 Description:		加速器过滤规则proc接口删除
 Data Accessed:

 Data Updated:
 Input:			无
 Output:			无
 Return:			无

 Others: 			删除/proc/fpp/nfp_filter文件
=========================================================================*/

static void nfp_filter_proc_exit( void )
{
    remove_proc_entry("nfp_filter", nfp_adapter_proc);
}

/*=========================================================================
 Function:		int nfp_adapter_init( void )
 Description:		加速器规则路由适配初始化
 Data Accessed:     nfp_conntrack_hash:加速器规则在NFP的缓存哈希表,初始化为全0

 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功

 Others:
=========================================================================*/
int nfp_conntrack_init(void)
{
	int ret = 0;
    NFP_ADAPTER_INTO_FUNC;

    atomic_set(&module_exit, 0);
    nfp_ct_h245_filter_init();

    spin_lock_init(&nfp_conntrack_splock);
    spin_lock_init(&nfp_filter_splock);
    nfp_ct_cache = kmem_cache_create("nfp_ct_cache",
                     sizeof(struct conntrack_entry_tab),
                     0,
                     SLAB_HWCACHE_ALIGN, NULL);
    if (!nfp_ct_cache){
        NFP_ADAPTER_ERROR("Fail to kmem hash cache\n");
        ret = -ENOMEM;
    }

    /*延后处理任务链表初始化*/
    nfp_ct_delay_work_list_init();

/* 注册勾子处理函数 */
#ifdef CONFIG_NF_CONNTRACK_EVENTS
    INIT_HLIST_NULLS_HEAD(&filter_head, NULL);

    nf_conntrack_register_notifier_nfp(&nfp_ct_notifier);
	if (0 != ret) {
		NFP_ADAPTER_ERROR("ctnfp_init: register nf_conntrack notifier faild\n");
		NFP_ADAPTER_ERROR("ret is %d",ret);
		goto err1;
	}

#endif

	/* 初始化特殊业务过滤proc文件 */
	ret = nfp_proc_filter_init();
    if(ret){
        goto err2;
    }

#ifdef CONFIG_TBS_NFP_CT
	tbs_nfp_hook_ct_nat_age = nfp_ct_nat_age;
#endif

    NFP_ADAPTER_OUT_FUNC;

	return ret;

err2:
    nf_conntrack_unregister_notifier_nfp(&nfp_ct_notifier);
err1:
    kmem_cache_destroy(nfp_ct_cache);
    return ret;
}

void nfp_conntrack_exit(void)
{

#ifdef CONFIG_TBS_NFP_CT
    tbs_nfp_hook_ct_nat_age = NULL;
#endif

    atomic_set(&module_exit, 1);
#ifdef CONFIG_NF_CONNTRACK_EVENTS
	//nf_conntrack_expect_unregister_notifier(&nfp_ct_notifier_exp);
	nf_conntrack_unregister_notifier_nfp(&nfp_ct_notifier);
#endif

    /*清除延后工作队列中的未执行任务*/
    nfp_ct_delay_work_list_clean();

	nfp_ct_proc_exit();

	nfp_filter_proc_exit();

	nfp_filter_list_destory();

    nfp_ct_h245_filter_exit();

    kmem_cache_destroy(nfp_ct_cache);

	return;
}

