/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_ct.h
* 文件描述 : TBS加速器conntrack处理模块
*
* 修订记录 :
*          1 创建 :
*            日期 : 2011-11-10
*            描述 :
*
*******************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/notifier.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/rtnetlink.h>
#include <linux/etherdevice.h>
#include <linux/notifier.h>
#include <net/neighbour.h>

#include "tbs_nfp.h"
#include "tbs_nfp_ct.h"


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/

/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
static struct kmem_cache *tbs_nfp_ct_rule_cache __read_mostly;
unsigned int g_ct_max_size __read_mostly = TBS_NFP_CT_RULE_DEF;
static unsigned int  tbs_nfp_ct_rule_num = 0;

#ifdef TBS_NFP_CT
struct list_head g_ct_hash[TBS_NFP_CT_HASH_SIZE];
struct list_head g_ct_hash_by_fib[TBS_NFP_CT_HASH_BY_FIB_SIZE];

struct list_head g_ct_inv_hash[TBS_NFP_CT_INV_HASH_SIZE];
struct list_head g_ct_inv_hash_by_fib[TBS_NFP_CT_INV_HASH_BY_FIB_SIZE];
#endif /* TBS_NFP_CT */


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:		static inline void tbs_nfp_ct_print(const TBS_NFP_RULE_CT *ct)

 Description:		答应conntrack
 Data Accessed:     无
 Data Updated:
 Input:			    const TBS_NFP_RULE_CT *ct   输出的conntrack
 Output:			无
 Return:			无
 Others:
=========================================================================*/
static inline void tbs_nfp_ct_print(const TBS_NFP_TUPLE_CT *tuple_ct)
{
	/*show the orignal rule*/
	if(NULL == tuple_ct)
		return;

	printk("[orignal] family=%d proto=%hu ",
			tuple_ct->family, tuple_ct->proto);

	if(AF_INET == tuple_ct->family)
		printk("src=%pI4 dst=%pI4 ",
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_ORIGINAL].src_l3,
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_ORIGINAL].dst_l3);
	else
		printk("src=%pI6 dst=%pI6 ",
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_ORIGINAL].src_l3,
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_ORIGINAL].dst_l3);

	printk("sport=%hu dport=%hu count=%u flags=0x%02x %s\n",
			ntohs(((unsigned short *)&tuple_ct->tuplehash[CT_DIR_ORIGINAL].ports)[0]),
			ntohs(((unsigned short *)&tuple_ct->tuplehash[CT_DIR_ORIGINAL].ports)[1]),
			tuple_ct->count, tuple_ct->tuplehash[CT_DIR_ORIGINAL].flags,
			(tuple_ct->tuplehash[CT_DIR_ORIGINAL].fib) ? "valid":"invalid");

	/*show the reply rule*/
	printk("[reply]   family=%d proto=%hu ",
			tuple_ct->family, tuple_ct->proto);

	if(AF_INET == tuple_ct->family)
		printk("src=%pI4 dst=%pI4 ",
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_REPLY].src_l3,
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_REPLY].dst_l3);
	else
		printk("src=%pI6 dst=%pI6 ",
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_REPLY].src_l3,
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_REPLY].dst_l3);

	printk("sport=%hu dport=%hu count=%u flags=0x%02x %s\n\n",
			ntohs(((unsigned short *)&tuple_ct->tuplehash[CT_DIR_REPLY].ports)[0]),
			ntohs(((unsigned short *)&tuple_ct->tuplehash[CT_DIR_REPLY].ports)[1]),
			tuple_ct->count, tuple_ct->tuplehash[CT_DIR_REPLY].flags,
			(tuple_ct->tuplehash[CT_DIR_REPLY].fib) ? "valid":"invalid");

	return;
}


/*=========================================================================
 Function:		void tbs_nfp_ct_dump(void)

 Description:		打印全部conntrack
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_ct_dump(void)
{
	ST_TUPLE_HASH *tmp_entry = NULL;
	unsigned int hash;

	printk("ct_rule_num:%u\n", tbs_nfp_ct_rule_num);

	TBS_NFP_READ_LOCK(&tbs_nfp_lock);

	for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_ct_hash); hash++)
	{
		list_for_each_entry(tmp_entry, &g_ct_hash[hash], list)
		{
			if(tmp_entry->dir == CT_DIR_ORIGINAL)
				tbs_nfp_ct_print(GET_TUPLE_CT(tmp_entry));
		}
	}

	for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_ct_inv_hash); hash++)
	{
		list_for_each_entry(tmp_entry, &g_ct_inv_hash[hash], list)
		{
			if(tmp_entry->dir == CT_DIR_ORIGINAL)
				tbs_nfp_ct_print(GET_TUPLE_CT(tmp_entry));
		}
	}
	TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

    return;
}


/*=========================================================================
 Function:		TBS_NFP_RULE_CT *tbs_nfp_ct_lookup_by_tuple(int family,
    const unsigned char *sip, const unsigned char *dip, unsigned int ports,
    unsigned short proto, bool valid)

 Description:		通过五元组从指定hash表中查找conntrack
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    int family              协议族
                    const unsigned char *src_l3 源IP地址
                    const unsigned char *dst_l3 目的IP地址
                    unsigned int ports      源、目的端口号
                    unsigned short proto    协议号
                    const bool valid        hash表g_ct_hash or g_ct_inv_hash
 Output:			无
 Return:			NULL:不存在；其他:TBS_NFP_RULE_CT 指针
 Others:
=========================================================================*/
ST_TUPLE_HASH *tbs_nfp_ct_lookup_by_tuple(int family,
    const unsigned char *sip, const unsigned char *dip, unsigned int ports,
    unsigned char proto, bool valid)
{
    unsigned int hash;
	TBS_NFP_TUPLE_CT *tuple_ct;
	ST_TUPLE_HASH *ct;
    const struct list_head *head = (valid)? g_ct_hash:g_ct_inv_hash;

    hash = tbs_nfp_ct_hash(family, sip, dip,
		((unsigned short *)&ports)[0], ((unsigned short *)&ports)[1],
        proto, valid);

    if(list_empty(&head[hash]))
        return NULL;

	list_for_each_entry(ct, &head[hash], list) {
		tuple_ct = GET_TUPLE_CT(ct);
		if ((tuple_ct->family == family) &&
				tbs_nfp_l3_addr_eq(tuple_ct->family, ct->src_l3, sip) &&
				tbs_nfp_l3_addr_eq(tuple_ct->family, ct->dst_l3, dip) &&
				(ct->ports == ports) && (tuple_ct->proto == proto))
			return ct;
	}

    return NULL;
}


/*=========================================================================
 Function:		static inline void tbs_nfp_ct_valid(TBS_NFP_RULE_CT *ct,
            TBS_NFP_RULE_FIB *fib)

 Description:		将ct从g_ct_inv_hash表移到g_ct_hash表中
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   路由缓存规则
 Output:			无
 Return:			无
 Others:
=========================================================================*/
static inline void tbs_nfp_ct_valid(ST_TUPLE_HASH *ct, TBS_NFP_RULE_FIB *fib, unsigned char proto)
{
    unsigned int hash , hash_by_fib;
    TBS_NFP_INTO_FUNC;

    ct->flags &= ~TBS_NFP_F_CT_FIB_INV;
    if(!(ct->flags & TBS_NFP_F_CT_INV_MASK))
    {
		hash = tbs_nfp_ct_hash(fib->family, ct->src_l3,
				ct->dst_l3, ((unsigned short *)&ct->ports)[0],
				((unsigned short *)&ct->ports)[1], proto, true);

		hash_by_fib = tbs_nfp_ct_hash_by_fib(fib->family, (unsigned int *)fib->src_l3,
				(unsigned int *)fib->dst_l3, true);

        /*fib 增加引用计数*/
        ct->fib = fib;
        fib->ref++;

        /*move ct to valid hash表*/
        list_move(&ct->list, &g_ct_hash[hash]);
        list_move(&ct->list_by_fib, &g_ct_hash_by_fib[hash_by_fib]);
    }

    return;
}


/*=========================================================================
 Function:		static inline void tbs_nfp_ct_invalid(TBS_NFP_RULE_CT *ct)

 Description:		将ct从g_ct_inv_hash表中移到g_ct_hash表中
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   路由缓存规则
 Output:			无
 Return:			无
 Others:
=========================================================================*/
static inline void tbs_nfp_ct_invalid(ST_TUPLE_HASH *ct, int family, unsigned char proto)
{
	unsigned int hash , hash_by_fib;

	ct->flags |= TBS_NFP_F_CT_FIB_INV;

	/*释放fib引用计数*/
	if(ct->fib)
	{
		ct->fib->ref--;
		ct->fib = NULL;
	}

	hash = tbs_nfp_ct_hash(family, ct->src_l3,
			ct->dst_l3, *(unsigned short *)&ct->ports,
			*((unsigned short *)&ct->ports + 1), proto, false);

	hash_by_fib = tbs_nfp_ct_hash_by_fib(family, (unsigned int *)ct->src_l3,
			(unsigned int *)(GET_NEXT_DIR_TUPLE(ct)->src_l3), false);

	/*move ct to invalid hash表*/
	list_move(&ct->list, &g_ct_inv_hash[hash]);
	list_move(&ct->list_by_fib, &g_ct_inv_hash_by_fib[hash_by_fib]);

	return;
}


/*=========================================================================
 Function:		void tbs_nfp_ct_fib_update(const TBS_NFP_RULE_FIB *fib)

 Description:		将g_ct_inv_hash表中与fib有关联的ct移到g_ct_hash表中
 Data Accessed:     g_ct_hash_by_fib/g_ct_inv_hash
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   路由缓存规则
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_ct_fib_invalid(const TBS_NFP_RULE_FIB *fib)
{
	ST_TUPLE_HASH*ct, *next;
	TBS_NFP_TUPLE_CT *tuple_ct;
	unsigned char *pdst_l3;

	unsigned int hash_by_fib = tbs_nfp_ct_hash_by_fib(fib->family, (unsigned int *)fib->src_l3,
			(unsigned int *)fib->dst_l3, true);

	list_for_each_entry_safe(ct, next, &g_ct_hash_by_fib[hash_by_fib], list_by_fib)
	{
		tuple_ct = GET_TUPLE_CT(ct);
		if(ct->flags & TBS_NFP_F_CT_DNAT)
			pdst_l3 = tuple_ct->tuplehash[CT_NEXT_DIR(ct->dir)].src_l3;
		else
			pdst_l3 = tuple_ct->tuplehash[ct->dir].dst_l3;

		if(fib->family == tuple_ct->family &&
				tbs_nfp_l3_addr_eq(fib->family, fib->src_l3, tuple_ct->tuplehash[ct->dir].src_l3) &&
				tbs_nfp_l3_addr_eq(fib->family, fib->dst_l3, pdst_l3))
			tbs_nfp_ct_invalid(ct, tuple_ct->family, tuple_ct->proto);
	}

	return;
}


/*=========================================================================
 Function:		void tbs_nfp_ct_fib_update(TBS_NFP_RULE_FIB *fib)

 Description:		将g_ct_inv_hash表中与fib有关联的ct移到g_ct_hash表中
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   路由缓存规则
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_ct_fib_update(TBS_NFP_RULE_FIB *fib)
{
	ST_TUPLE_HASH*ct, *next;
	TBS_NFP_TUPLE_CT *tuple_ct;
	unsigned char *pdst_l3;

	unsigned int hash_by_fib = tbs_nfp_ct_hash_by_fib(fib->family, (unsigned int *)fib->src_l3,
			(unsigned int *)fib->dst_l3, false);

	list_for_each_entry_safe(ct, next, &(g_ct_inv_hash_by_fib[hash_by_fib]), list_by_fib)
	{
		tuple_ct = GET_TUPLE_CT(ct);
		if(ct->flags & TBS_NFP_F_CT_DNAT)
			pdst_l3 = tuple_ct->tuplehash[CT_NEXT_DIR(ct->dir)].src_l3;
		else
			pdst_l3 = ct->dst_l3;

		if(tbs_nfp_l3_addr_eq(fib->family, fib->src_l3, ct->src_l3) &&
				tbs_nfp_l3_addr_eq(fib->family, fib->dst_l3, pdst_l3))
		{
			tbs_nfp_ct_valid(ct, fib, tuple_ct->proto);
        }
	}

	return;
}


/*=========================================================================
 Function:		static inline void __tbs_nfp_ct_clean(void)
 Description:		清除所有指定hash表中的规则
 Data Accessed:     g_ct_hash or g_ct_inv_hash
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
static inline void __tbs_nfp_ct_clean(bool valid)
{
	unsigned int hash = 0;
	struct list_head *head;
	unsigned int hash_num;
	ST_TUPLE_HASH *tmp_entry = NULL;
	TBS_NFP_TUPLE_CT *tuple_ct;

	head = valid ? g_ct_hash:g_ct_inv_hash;
	hash_num = valid ?TBS_NFP_ARRAY_SIZE(g_ct_hash):TBS_NFP_ARRAY_SIZE(g_ct_inv_hash);

	TBS_NFP_INTO_FUNC;

	for (hash = 0; hash < hash_num; hash++)
	{
        /*
        这里可能会出现双向五元组在同一个hash桶中，所以不能用
        list_for_each_entry_safe()遍历删除
        */
        while(!list_empty(&(head[hash])))
        {
            tmp_entry = list_entry((head[hash]).next, ST_TUPLE_HASH, list);
			tuple_ct = GET_TUPLE_CT(tmp_entry);

			/* delete CT_DIR_ORIGINAL direction ct rule */
			if(tuple_ct->tuplehash[CT_DIR_ORIGINAL].fib)
				tuple_ct->tuplehash[CT_DIR_ORIGINAL].fib->ref --;

			list_del(&tuple_ct->tuplehash[CT_DIR_ORIGINAL].list);
			list_del(&tuple_ct->tuplehash[CT_DIR_ORIGINAL].list_by_fib);

			/* delete CT_DIR_REPLY direction ct rule */
			if(tuple_ct->tuplehash[CT_DIR_REPLY].fib)
				tuple_ct->tuplehash[CT_DIR_REPLY].fib->ref --;

			list_del(&tuple_ct->tuplehash[CT_DIR_REPLY].list);
			list_del(&tuple_ct->tuplehash[CT_DIR_REPLY].list_by_fib);

			tbs_nfp_ct_rule_num --;

			kmem_cache_free(tbs_nfp_ct_rule_cache, tuple_ct);
        }
	}
}
/*=========================================================================
 Function:		static inline void tbs_nfp_ct_clean(void)
 Description:		清除所有ct规则
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
static inline void tbs_nfp_ct_clean(void)
{
	__tbs_nfp_ct_clean(true);
	__tbs_nfp_ct_clean(false);
	return;
}


/*=========================================================================
 Function:		static inline int tbs_nfp_ct_reset(int family,
                const unsigned char *sip, const unsigned char *dip,
                unsigned int ports, unsigned char proto)

 Description:		清除所有ct规则
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_ct_reset(void)
{
    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
    tbs_nfp_ct_clean();
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return;
}



/*=========================================================================
 Function:      static inline bool tbs_nfp_ct_exist(int family, const unsigned char *sip,
				const unsigned char *dip, unsigned short sport, unsigned short dport,
				unsigned char proto)

 Description:		查找五元组信息是否已经存在hash缓存中
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    五元组信息
                    family: IP version
					sip: source ip address
					dip: dest ip address
					sport: source port
					dport: dest port
					proto: protocols number
 Output:			无
 Return:			0:  成功，其他: 失败
 Others:
=========================================================================*/
static inline bool tbs_nfp_ct_exist(int family, const unsigned char *sip,
    const unsigned char *dip, unsigned int ports, unsigned char proto)
{
	bool exist = false;
	TBS_NFP_READ_LOCK(&tbs_nfp_lock);
	if(tbs_nfp_ct_lookup_by_tuple(family, sip, dip, ports, proto, true)||
			tbs_nfp_ct_lookup_by_tuple(family, sip, dip, ports, proto, false))
		exist = true;
	TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

	return exist;
}


/*=========================================================================
 Function:		int tbs_nfp_ct_add(int family, const unsigned char *sip, const unsigned char *dip,
		unsigned short sport, unsigned short dport, unsigned char proto,
		const unsigned char *reply_sip, const unsigned char *reply_dip,
		unsigned short reply_sport, unsigned short reply_dport)

 Description:		往g_ct_inv_hash/g_ct_hash表中添加指定的5元组信息
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    双向五元组信息
 Output:			无
 Return:			0:  成功，其他: 失败
 Others:
=========================================================================*/
int tbs_nfp_ct_add(int family, const unsigned char *sip, const unsigned char *dip,
		unsigned short sport, unsigned short dport, unsigned char proto,
		const unsigned char *reply_sip, const unsigned char *reply_dip,
		unsigned short reply_sport, unsigned short reply_dport)
{
	TBS_NFP_TUPLE_CT * new_tupe_ct;
	TBS_NFP_RULE_FIB * fib_entry;
	unsigned int hash;
	unsigned int ports;
	int ip_len;
#ifdef TBS_NFP_NAT
	int original_flags = 0;
	int reply_flags = 0;
#endif

	((unsigned short* )&ports)[0] = sport;
	((unsigned short* )&ports)[1] = dport;

	//当前规则是否已经存在
	if(tbs_nfp_ct_exist(family, sip, dip, ports, proto))
		return TBS_NFP_OK;

	//规则总数不能超过 g_ct_max_size
	if(g_ct_max_size == tbs_nfp_ct_rule_num)
	{
        TBS_NFP_DEBUG("tbs_nfp_ct_rule_num(%u) == g_ct_max_size\n",
            tbs_nfp_ct_rule_num);

		return TBS_NFP_ERR;
	}

	new_tupe_ct = kmem_cache_alloc(tbs_nfp_ct_rule_cache, GFP_ATOMIC);
	if(NULL == new_tupe_ct )
		return TBS_NFP_ERR;

	memset(new_tupe_ct, 0, sizeof(*new_tupe_ct));
	/* 5 tuple key */
	new_tupe_ct->family = family;
	new_tupe_ct->proto = proto;

	if(AF_INET == family) //ipv4
	{
		ip_len = TBS_NFP_MAX_IPV4_ADDR_SIZE;
	}
	else //ipv6
	{
		ip_len = TBS_NFP_MAX_L3_ADDR_SIZE;
	}

#ifdef TBS_NFP_NAT
//tbs_nfp_l3_addr_eq(family, reply_sip, sip)
	if(((!tbs_nfp_l3_addr_eq(family, reply_dip, sip))|| reply_dport != sport) &&
			(tbs_nfp_l3_addr_eq(family, reply_sip, dip) && reply_sport == dport))
	{
		/*snat*/
		original_flags = TBS_NFP_F_CT_SNAT;
		reply_flags = TBS_NFP_F_CT_DNAT;
	}
//	else if((reply_dip ==
//				sip && reply_dport == sport) && (
//					reply_sip != dip || reply_sport != dport))
	else if(((tbs_nfp_l3_addr_eq(family, reply_dip, sip) && reply_dport == sport ))&&
			((!tbs_nfp_l3_addr_eq(family, reply_sip, dip)) || reply_sport != dport))

	{
		/*dnat*/
		original_flags = TBS_NFP_F_CT_DNAT;
		reply_flags = TBS_NFP_F_CT_SNAT;
	}
	else if((!tbs_nfp_l3_addr_eq(family, reply_dip, sip)||reply_dport != sport) &&
			(!tbs_nfp_l3_addr_eq(family, reply_dip, dip) ||  reply_sport != dport))
	//else if((reply_dip !=
	//			sip || reply_dport != sport) && (
	//				reply_sip != dip || reply_sport != dport))
	{
		/*dnat & snat*/
		original_flags = TBS_NFP_F_CT_SNAT | TBS_NFP_F_CT_DNAT;
		reply_flags = TBS_NFP_F_CT_DNAT| TBS_NFP_F_CT_SNAT;
	}
	else
	{
		/*no nat*/
	}
#endif /*TBS_NFP_NAT*/

	((unsigned short* )&ports)[0] = sport;
	((unsigned short* )&ports)[1] = dport;
	/* original direction initial */
	memcpy(new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].src_l3, sip, ip_len);
	memcpy(new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].dst_l3, dip, ip_len);
	new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].ports = ports;
#ifdef TBS_NFP_NAT
	new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].flags = original_flags;
#endif
	new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].dir = CT_DIR_ORIGINAL;

	/* reply direction initial */
	((unsigned short* )&ports)[0] = reply_sport;
	((unsigned short* )&ports)[1] = reply_dport;
	memcpy(new_tupe_ct->tuplehash[CT_DIR_REPLY].src_l3, reply_sip, ip_len);
	memcpy(new_tupe_ct->tuplehash[CT_DIR_REPLY].dst_l3, reply_dip, ip_len);
	new_tupe_ct->tuplehash[CT_DIR_REPLY].ports = ports;
#ifdef TBS_NFP_NAT
	new_tupe_ct->tuplehash[CT_DIR_REPLY].flags = reply_flags;
#endif
	new_tupe_ct->tuplehash[CT_DIR_REPLY].dir = CT_DIR_REPLY;

	TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

	/*
	 * 查找fib规则中是否存在相应的规则
	 */
	fib_entry = tbs_nfp_fib_lookup(family, sip, reply_sip, true);
	new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].fib = fib_entry;

	/* fib规则中不存在相应的规则 */
	if(NULL == fib_entry)
	{
		new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].flags |= TBS_NFP_F_CT_FIB_INV;

		hash = tbs_nfp_ct_hash(family, sip, dip,
				sport, dport, proto, false);
		list_add(&new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].list, &g_ct_inv_hash[hash]);

		hash = tbs_nfp_ct_hash_by_fib(family, (unsigned int*)sip, (unsigned int*)reply_sip, false);

		list_add(&new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].list_by_fib, &g_ct_inv_hash_by_fib[hash]);

	}
	else /* fib规则中存在相应的规则 */
	{
		new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].fib->ref ++;
		hash = tbs_nfp_ct_hash(family, sip, dip,
				sport, dport, proto, true);

		list_add(&new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].list, &g_ct_hash[hash]);

		hash = tbs_nfp_ct_hash_by_fib(family, (unsigned int*)sip, (unsigned int*)reply_sip, true);
		list_add(&new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].list_by_fib, &g_ct_hash_by_fib[hash]);
	}

	//reply
	fib_entry = tbs_nfp_fib_lookup(family, reply_sip, sip, true);
	new_tupe_ct->tuplehash[CT_DIR_REPLY].fib = fib_entry;

	/* fib规则中不存在相应的规则 */
	if(NULL == fib_entry)
	{

		new_tupe_ct->tuplehash[CT_DIR_REPLY].flags |= TBS_NFP_F_CT_FIB_INV;

		hash = tbs_nfp_ct_hash(family, reply_sip, reply_dip,
				dport, sport, proto, false);
		list_add(&new_tupe_ct->tuplehash[CT_DIR_REPLY].list, &(g_ct_inv_hash[hash]));

		hash = tbs_nfp_ct_hash_by_fib(family, (unsigned int*)reply_sip, (unsigned int*)sip, false);

		list_add(&(new_tupe_ct->tuplehash[CT_DIR_REPLY].list_by_fib), &(g_ct_inv_hash_by_fib[hash]));

	}
	else /* fib规则中存在相应的规则 */
	{
		new_tupe_ct->tuplehash[CT_DIR_REPLY].fib->ref ++;
		hash = tbs_nfp_ct_hash(family, reply_sip, reply_dip,
				dport, sport, proto, true);

		list_add(&new_tupe_ct->tuplehash[CT_DIR_REPLY].list, &g_ct_hash[hash]);

		hash = tbs_nfp_ct_hash_by_fib(family, (unsigned int*)reply_sip, (unsigned int*)sip, true);
		list_add(&new_tupe_ct->tuplehash[CT_DIR_REPLY].list_by_fib, &g_ct_hash_by_fib[hash]);
	}
	//new_ct_entry->count = 10;
	++tbs_nfp_ct_rule_num;
	TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

	return TBS_NFP_OK;
}

/*=========================================================================
 Function:		int tbs_nfp_ct_delete(int family, const unsigned char *sip,
                const unsigned char *dip, unsigned short sport,
                unsigned short dport, unsigned char proco)

 Description:		从g_ct_inv_hash/g_ct_hash表中移除指定的5元组信息
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    五元组信息
 Output:			无
 Return:			0:  成功，其他: 失败
 Others:
=========================================================================*/
int tbs_nfp_ct_delete(int family, const unsigned char *sip, const unsigned char *dip,
               unsigned short sport, unsigned short dport, unsigned char proto)
{
	TBS_NFP_TUPLE_CT *ct_tuple;
	ST_TUPLE_HASH * ct_dir_tuple;
	int ret = TBS_NFP_OK;
	unsigned int ports;

	((unsigned short* )&ports)[0] = sport;
	((unsigned short* )&ports)[1] = dport;
	TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
	ct_dir_tuple = tbs_nfp_ct_lookup_by_tuple(family, sip, dip,
			ports, proto, true);
	if(NULL == ct_dir_tuple)
	{
		ct_dir_tuple = tbs_nfp_ct_lookup_by_tuple(family, sip, dip,
				ports, proto, false);
		if(NULL == ct_dir_tuple)
		{
			ret = TBS_NFP_ERR;
			goto out1;
		}
	}

	ct_tuple = GET_TUPLE_CT(ct_dir_tuple);

	/* delete CT_DIR_ORIGINAL direction ct rule */
	if(ct_tuple->tuplehash[CT_DIR_ORIGINAL].fib)
		ct_tuple->tuplehash[CT_DIR_ORIGINAL].fib->ref--;
	list_del(&ct_tuple->tuplehash[CT_DIR_ORIGINAL].list);
	list_del(&ct_tuple->tuplehash[CT_DIR_ORIGINAL].list_by_fib);

	/* delete CT_DIR_REPLY direction ct rule */
	if(ct_tuple->tuplehash[CT_DIR_REPLY].fib)
		ct_tuple->tuplehash[CT_DIR_REPLY].fib->ref--;
	list_del(&ct_tuple->tuplehash[CT_DIR_REPLY].list);
	list_del(&ct_tuple->tuplehash[CT_DIR_REPLY].list_by_fib);

	kmem_cache_free(tbs_nfp_ct_rule_cache, ct_tuple);
	tbs_nfp_ct_rule_num--;

out1:
	TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
	return ret;
}


/*=========================================================================
 Function:		int tbs_nfp_ct_age(int family, const unsigned char *sip,
                const unsigned char *dip, unsigned short sport,
                unsigned short dport, unsigned char proco)

 Description:		从g_ct_inv_hash/g_ct_hash表中查找指定五元组记录的包计数
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    五元组信息
 Output:			无
 Return:			0:  老化，-1: 失败，其他:   包计数
 Others:
=========================================================================*/
unsigned int tbs_nfp_ct_age(int family, const unsigned char *sip, const unsigned char *dip,
               unsigned short sport, unsigned short dport, unsigned char proto)
{
	ST_TUPLE_HASH *ct_dir_tuple= NULL;
	TBS_NFP_TUPLE_CT *ct_tuple= NULL;
	unsigned int count = 0;
	unsigned int ports;
	((unsigned short* )&ports)[0] = sport;
	((unsigned short* )&ports)[1] = dport;

	TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

	ct_dir_tuple = tbs_nfp_ct_lookup_by_tuple(family, sip, dip, ports, proto, true);
	if(NULL == ct_dir_tuple)
	{
		ct_dir_tuple = tbs_nfp_ct_lookup_by_tuple(family, sip, dip, ports, proto, false);
		if(NULL == ct_dir_tuple)
			goto out;
	}
	ct_tuple = GET_TUPLE_CT(ct_dir_tuple);
	count = ct_tuple->count;
	ct_tuple->count = 0;

out:
	TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
	return count;
}


/*=========================================================================
 Function:		void tbs_nfp_ct_exit(void)

 Description:		conntrack子模块退出函数
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    五元组信息
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_ct_exit(void)
{
	//清除所有ct规则
	tbs_nfp_ct_reset();

	//释放内存cache
	kmem_cache_destroy(tbs_nfp_ct_rule_cache);
	return;
}


/*=========================================================================
 Function:		int tbs_nfp_ct_init(void)

 Description:		conntrack子模块初始化函数
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    五元组信息
 Output:			无
 Return:			0:  老化，其他: 失败
 Others:
=========================================================================*/
int tbs_nfp_ct_init(void)
{
	unsigned int i = 0;
	int ret = 0;

	//内存预分配缓存区
	tbs_nfp_ct_rule_cache = kmem_cache_create("tbs_nfp_ct_rule_cache",
			sizeof(TBS_NFP_TUPLE_CT),
			0,
			SLAB_HWCACHE_ALIGN, NULL);
	if(!tbs_nfp_ct_rule_cache)
	{
		TBS_NFP_ERROR("Fail to kmem hash cache tbs_nfp_ct_rule_cache \n");
		ret = -ENOMEM;
		goto out;
	}
	tbs_nfp_ct_rule_num = 0;

    //初始化hash表
	for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_ct_hash); i++)
	{
		INIT_LIST_HEAD(&g_ct_hash[i]);
	}

	for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_ct_hash_by_fib); i++)
	{
		INIT_LIST_HEAD(&g_ct_hash_by_fib[i]);
	}

	for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_ct_inv_hash); i++)
	{
		INIT_LIST_HEAD(&g_ct_inv_hash[i]);
	}
	for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_ct_inv_hash_by_fib); i++)
	{
		INIT_LIST_HEAD(&g_ct_inv_hash_by_fib[i]);
	}

out:
	return ret;
}

