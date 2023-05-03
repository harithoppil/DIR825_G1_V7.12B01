/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_fib.c
* 文件描述 : TBS加速器fib处理模块
*
* 修订记录 :
*          1 创建 : pengyao
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

#include "tbs_nfp_arp.h"
#include "tbs_nfp_fib.h"
#include "tbs_nfp_ct.h"
/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/

/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
struct list_head g_fib_hash[TBS_NFP_FIB_HASH_SIZE];
struct list_head g_fib_hash_by_gtw[TBS_NFP_FIB_HASH_BY_GTW_SIZE];

struct list_head g_fib_inv_hash[TBS_NFP_FIB_INV_HASH_SIZE];
struct list_head g_fib_inv_hash_by_gtw[TBS_NFP_FIB_INV_HASH_BY_GTW_SIZE];

static struct kmem_cache *tbs_nfp_fib_rule_cache __read_mostly;
unsigned int g_fib_max_size __read_mostly = TBS_NFP_FIB_RULE_DEF;
static unsigned int  tbs_nfp_fib_rule_num = 0;


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:		static inline void tbs_nfp_fib_printk(const TBS_NFP_RULE_FIB *fib)

 Description:		打印fib规则
 Data Accessed:     无

 Data Updated:
 Input:			    const TBS_NFP_RULE_FIB *fib fib规则指针
 Output:			无
 Return:			无

 Others:
=========================================================================*/
static inline void tbs_nfp_fib_printk(unsigned char hash_key, const TBS_NFP_RULE_FIB *fib_rule)
{

    TBS_NFP_IF_MAP *oif_map = NULL;

    oif_map = tbs_nfp_ifmap_get(fib_rule->oif);
    if(NULL == oif_map)
    {
        printk("Get if_map fail(oif:%d)\n", fib_rule->oif);
        return;
    }

    if (AF_INET == fib_rule->family)
    {
        printk("%u\t%-16s"
            "%pI4\t%pI4\t%pI4\t"
            "%02x:%02x:%02x:%02x:%02x:%02x\t%u\t%u\t%u\n",
            hash_key, oif_map->name,
            fib_rule->src_l3,fib_rule->dst_l3,fib_rule->def_gtw_l3,
            fib_rule->da[0], fib_rule->da[1],
            fib_rule->da[2], fib_rule->da[3],
            fib_rule->da[4], fib_rule->da[5],
            fib_rule->count,fib_rule->flags,fib_rule->ref);
    }
    else
    {
        printk("%u\t%-16s"
            "%pI6\t\t%pI6\t\t%pI6\t\t"
            "%02x:%02x:%02x:%02x:%02x:%02x\t%u\t%u\t%u\n",
            hash_key, oif_map->name,
            fib_rule->src_l3,fib_rule->dst_l3,fib_rule->def_gtw_l3,
            fib_rule->da[0], fib_rule->da[1],
            fib_rule->da[2], fib_rule->da[3],
            fib_rule->da[4], fib_rule->da[5],
            fib_rule->count,fib_rule->flags,fib_rule->ref);
    }

    return;
}

/*=========================================================================
 Function:		void tbs_nfp_fib_dump(void)

 Description:		打印fib hash表所有规则
 Data Accessed:     g_fib_hash[hash]
                    g_fib_inv_hash[hash]

 Data Updated:      无
 Input:			    无
 Output:			打印所有的fib规则
 Return:			无

 author:    tbs cairong
=========================================================================*/
void tbs_nfp_fib_dump(void)
{
    unsigned int hash = 0;
    TBS_NFP_RULE_FIB *tmp_entry;

    printk("fib_rule_num:%u\n", tbs_nfp_fib_rule_num);
    printk("valid fib:\n");
    printk("hash\toutdev\t\tsip\t\tdip\t\tgtw\t\t"
        "dmac\t\t\tcount\tflags\tref\n");

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);

    //依次遍历并删除规则g_fib_hash，g_fib_hash_by_gtw中规则也一并被删除

    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_fib_hash); hash++)
    {
        list_for_each_entry(tmp_entry, &g_fib_hash[hash], list)
        {
            tbs_nfp_fib_printk(hash, tmp_entry);
        }
    }

    //依次遍历并删除规则g_fib_inv_hash，g_fib_inv_hash_by_gtw中规则也一并被删除
    printk("\ninvalid fib:\n");
    printk("hash\toutdev\t\tsip\t\tdip\t\tgtw\t\t"
        "dmac\t\t\tcount\tflags\tref\n");
    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_fib_inv_hash); hash++)
    {
        list_for_each_entry(tmp_entry, &g_fib_inv_hash[hash], list)
        {
            tbs_nfp_fib_printk(hash, tmp_entry);
        }
    }

    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);
    return ;
}


static inline int tbs_nfp_fib_rule_cmp(int family, const unsigned char *src_l3,
		const unsigned char *dst_l3, TBS_NFP_RULE_FIB *rule)
{
	if ((family == rule->family) &&
			tbs_nfp_l3_addr_eq(family, rule->src_l3, src_l3) &&
			tbs_nfp_l3_addr_eq(family, rule->dst_l3, dst_l3))
		return true;

	return false;
}

inline TBS_NFP_RULE_FIB *tbs_nfp_fib_lookup(int family, const unsigned char *src_l3,
		const unsigned char *dst_l3, const bool valid)
{
	unsigned int hash;
	TBS_NFP_RULE_FIB *rule;
	struct list_head *head = (valid)? g_fib_hash:g_fib_inv_hash;

	hash = tbs_nfp_fib_hash(family, (unsigned int *)src_l3, (unsigned int *)dst_l3, valid);

	list_for_each_entry(rule, &head[hash], list)
    {
		if (tbs_nfp_fib_rule_cmp(family, src_l3, dst_l3, rule))
			return rule;
	}

	return NULL;
}

/*=========================================================================
 Function:		static bool tbs_nfp_fib_rule_exist(int family, unsigned char *nextHopL3)

 Description:		FIB规则是否存在
 Data Accessed:     struct list_head g_fib_hash[TBS_NFP_FIB_HASH_SIZE];
 Data Updated:
 Input:			    int family,               协议类型ipv4/ipv6
                    unsigned char *nextHopL3  下一跳地址
 Output:			无
 Return:			true:   存在，flse: 不存在
 Others:
=========================================================================*/
inline bool tbs_nfp_fib_exist(int family, const unsigned char *src_l3,
		const unsigned char *dst_l3, const bool valid)
{
    bool exist = false;

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);

    if(tbs_nfp_fib_lookup(family, src_l3, dst_l3, valid))
        exist = true;

    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

    return exist;
}

/*=========================================================================
 Function:		inline void tbs_nfp_fib_valid(TBS_NFP_RULE_FIB *fib,
    unsigned int inv_flag)

 Description:		去除fib规则的 inv_flag标志
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   路由缓存规则
                    inv_flag    TBS_NFP_F_FIB_ARP_INV/TBS_NFP_F_FIB_BRIDGE_INV
 Output:
 Return:			无
 Others:            如果更新的fib记录在g_fib_inv_hash中，则将其移到g_fib_hash
=========================================================================*/
inline void tbs_nfp_fib_valid(TBS_NFP_RULE_FIB *fib,
    unsigned int inv_flag)
{
    int hash, hash_by_gtw;

#ifdef TBS_NFP_BRIDGE
    TBS_NFP_RULE_BRIDGE *bridge_rule;
#endif /*TBS_NFP_BRIDGE*/

    TBS_NFP_INTO_FUNC;

    if(TBS_NFP_F_FIB_BRIDGE_INV != inv_flag &&
        TBS_NFP_F_FIB_ARP_INV != inv_flag)
    {
        TBS_NFP_ERROR("invalid fib inv_flag(%d)\n", inv_flag);
        return;
    }

    /*如果是arp更新，则要对lan侧fib出接口为br进行更新*/
    if(TBS_NFP_F_FIB_ARP_INV == inv_flag &&
        (fib->flags & TBS_NFP_F_FIB_BRIDGE_INV))
    {
#ifdef TBS_NFP_BRIDGE
        bridge_rule = tbs_nfp_bridge_rule_lookup(fib->da, fib->sa, fib->oif);
        if(bridge_rule)
        {
            fib->oif = bridge_rule->oif;
            fib->flags &= ~TBS_NFP_F_FIB_BRIDGE_INV;
        }
#endif /*TBS_NFP_BRIDGE*/
    }

    fib->flags &= ~inv_flag;

    if(!(fib->flags & TBS_NFP_F_FIB_INV_MASK))
    {
        hash = tbs_nfp_fib_hash(fib->family, (unsigned int *)fib->src_l3,
            (unsigned int *)fib->dst_l3, true);

        /*move fib to valid hash 表*/
        list_move(&fib->list, &g_fib_hash[hash]);

        hash_by_gtw = tbs_nfp_fib_hash_gtw(fib->family, (unsigned int *)fib->def_gtw_l3, true);
        list_move(&fib->list_by_gtw, &g_fib_hash_by_gtw[hash_by_gtw]);

        /*更新conntrack规则的fib*/
#ifdef TBS_NFP_CT
        tbs_nfp_ct_fib_update(fib);
#endif  /* TBS_NFP_CT */
    }

    return;
}


/*=========================================================================
 Function:	inline void tbs_nfp_fib_invalid(TBS_NFP_RULE_FIB *fib,
    unsigned int inv_flag)

 Description:		设置fib inv_flag标志
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   路由缓存规则
                    inv_flag    TBS_NFP_F_FIB_ARP_INV/TBS_NFP_F_FIB_BRIDGE_INV
 Output:
 Return:			无
 Others:            如果更新的fib记录在g_fib_hash中，则将其移到g_fib_inv_hash
=========================================================================*/
inline void tbs_nfp_fib_invalid(TBS_NFP_RULE_FIB *fib,
    unsigned int inv_flag)
{
    int hash, hash_by_gtw;
    TBS_NFP_IF_MAP *if_map = NULL;

    if(TBS_NFP_F_FIB_BRIDGE_INV != inv_flag &&
        TBS_NFP_F_FIB_ARP_INV != inv_flag)
    {
        TBS_NFP_ERROR("invalid fib inv_flag(%d)\n", inv_flag);
        return;
    }

    if(!(fib->flags & TBS_NFP_F_FIB_INV_MASK))
    {
        if_map = tbs_nfp_ifmap_get(fib->oif);

        /*设置TBS_NFP_F_FIB_BRIDGE_INV*/
        if(if_map && (if_map->flags & TBS_NFP_F_MAP_BRIDGE_PORT))
        {
            fib->oif = if_map->bridge_if;
            fib->flags |= TBS_NFP_F_FIB_BRIDGE_INV;
        }

        /*move fib to invalid hash表中*/
        hash = tbs_nfp_fib_hash(fib->family, (unsigned int *)fib->src_l3,
                        (unsigned int *)fib->dst_l3, false);
        list_move(&fib->list, &g_fib_inv_hash[hash]);

        hash_by_gtw = tbs_nfp_fib_hash_gtw(fib->family, (unsigned int *)fib->def_gtw_l3, false);
        list_move(&fib->list_by_gtw, &g_fib_inv_hash_by_gtw[hash_by_gtw]);

        tbs_nfp_ct_fib_invalid(fib);
    }

    fib->flags |= inv_flag;

    return;
}


#ifdef TBS_NFP_BRIDGE
/*=========================================================================
 Function:		void tbs_nfp_fib_bridge_update(const TBS_NFP_RULE_BRIDGE *bridge_rule)

 Description:		清除路由缓存表中出接口为bridge_rule->oif规则的
                    的TBS_NFP_F_FIB_BRIDGE_INV标志，并设置路由缓存
                    规则出接口为bridge_rule->oif设备
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    const TBS_NFP_RULE_BRIDGE *bridge_rule  到本机的桥规则
 Output:
 Return:			0:成功  其他:失败
 Others:
=========================================================================*/
void tbs_nfp_fib_bridge_update(const TBS_NFP_RULE_BRIDGE *bridge_rule)
{
    int i = 0;
    TBS_NFP_RULE_FIB *next_fib, *fib = NULL;

    //printk("In %s:   br_rule->da:%02x:%02x:%02x:%02x:%02x:%02x, sa:%02x:%02x:%02x:%02x:%02x:%02x, bridge_rule->iif:%d\n",
    //    __func__, bridge_rule->da[0], bridge_rule->da[1], bridge_rule->da[2], bridge_rule->da[3], bridge_rule->da[4], bridge_rule->da[5],
    //    bridge_rule->sa[0], bridge_rule->sa[1], bridge_rule->sa[2], bridge_rule->sa[3], bridge_rule->sa[4], bridge_rule->sa[5], bridge_rule->iif);


    //遍历hash表
    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_inv_hash); i++)
    {
        //遍历链表
        list_for_each_entry_safe(fib, next_fib, &g_fib_inv_hash[i], list)
        {
            if(!(fib->flags & TBS_NFP_F_FIB_ARP_INV) &&
                (fib->flags & TBS_NFP_F_FIB_BRIDGE_INV))
            {
                if(tbs_nfp_mac_addr_eq((void *)fib->da, (void *)bridge_rule->da) &&
                    tbs_nfp_mac_addr_eq((void *)fib->sa, (void *)bridge_rule->sa) &&
                    fib->oif == bridge_rule->iif)
                {
                    tbs_nfp_fib_valid(fib, TBS_NFP_F_FIB_BRIDGE_INV);
                    fib->oif = bridge_rule->oif;
                }
            }
        }
    }

    return;
}


/*=========================================================================
 Function:		void tbs_nfp_fib_bridge_invalid(int br_index, int br_port_index)

 Description:		设置路由缓存中出接口为br_port_index的路由缓存标志为TBS_NFP_F_FIB_BRIDGE_INV
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    int family  协议族ipv4/6
                    const char *sip 源ip
                    const char *dip 目的ip
 Output:
 Return:			0:成功  其他:失败
 Others:
=========================================================================*/
void tbs_nfp_fib_bridge_invalid(int br_index, int br_port_index, unsigned const char *da)
{
    int i = 0;
    TBS_NFP_RULE_FIB *next_fib, *fib = NULL;

    //printk("In %s:%d, br_port:%d, da:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__, __LINE__,
    //    br_port_index, da[0], da[0], da[0], da[0], da[0], da[0]);

    //遍历hash表
    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_hash); i++)
    {
        //遍历链表
        list_for_each_entry_safe(fib, next_fib, &g_fib_hash[i], list){
            if(fib->oif == br_port_index &&
                tbs_nfp_mac_addr_eq((void *)fib->da, (void *)da)) {
                tbs_nfp_fib_invalid(fib, TBS_NFP_F_FIB_BRIDGE_INV);
                fib->oif = br_index;
            }
        }
    }

    return;
}
#endif  /*TBS_NFP_BRIDGE*/


/*=========================================================================
 Function:		inline void tbs_nfp_fib_arp_update(int family, const unsigned char *next_hop, const unsigned char *mac)


 Description:		设置路由缓存中目的ip为dip的路由缓存标志为TBS_NFP_F_FIB_ARP_INV
 Data Accessed:     struct list_head g_fib_hash[]
 Data Updated:
 Input:			    int family  协议族ipv4/6
                    const char *next_hop 下一跳ip
 Output:
 Return:			0:成功  其他:失败
 Others:            如果更新的fib记录在g_fib_hash中，则将其移到g_fib_inv_hash
=========================================================================*/
inline void tbs_nfp_fib_arp_update(int family, const unsigned char *next_hop, const unsigned char *mac)
{
    TBS_NFP_RULE_FIB *fib, *next_fib;
    int hash_by_gtw = tbs_nfp_fib_hash_gtw(family, (const unsigned int *)next_hop, false);

    TBS_NFP_INTO_FUNC;

    TBS_NFP_DEBUG("hash_by_gtw:%d,family:%d,next_hop:%pI4\n",hash_by_gtw,family,(void *)next_hop);
    list_for_each_entry_safe(fib, next_fib, &g_fib_inv_hash_by_gtw[hash_by_gtw], list_by_gtw)
    {
        if(fib->family == family
            && tbs_nfp_l3_addr_eq(family, next_hop, fib->def_gtw_l3))
        {
            memcpy(fib->da, mac, sizeof(fib->da));
            tbs_nfp_fib_valid(fib, TBS_NFP_F_FIB_ARP_INV);
        }
    }

    return;
}


/*=========================================================================
 Function:		inline void tbs_nfp_fib_arp_invalid(int family, const char *dip)

 Description:		设置路由缓存中目的ip为dip的路由缓存标志为TBS_NFP_F_FIB_ARP_INV
 Data Accessed:     struct list_head g_fib_hash[]
 Data Updated:
 Input:			    int family  协议族ipv4/6
                    const char *next_hop 下一跳ip
 Output:
 Return:			0:成功  其他:失败
 Others:            如果更新的fib记录在g_fib_hash中，则将其移到g_fib_inv_hash
=========================================================================*/
inline void tbs_nfp_fib_arp_invalid(int family, const unsigned char *next_hop)
{
    TBS_NFP_RULE_FIB *fib, *fib_next;

    int hash_by_gtw = tbs_nfp_fib_hash_gtw(family, (unsigned int *)next_hop, true);
    int hash_inv_by_gtw = tbs_nfp_fib_hash_gtw(family, (unsigned int *)next_hop, false);

     TBS_NFP_INTO_FUNC;
    /*遍历g_fib_hash*/
    list_for_each_entry_safe(fib, fib_next, &g_fib_hash_by_gtw[hash_by_gtw], list_by_gtw)
    {
        if(tbs_nfp_l3_addr_eq(family, next_hop, fib->def_gtw_l3))
        {
            memset(fib->da, 0, sizeof(fib->da));
            tbs_nfp_fib_invalid(fib, TBS_NFP_F_FIB_ARP_INV);
        }
    }

    /*遍历g_fib_inv_hash*/
    list_for_each_entry_safe(fib, fib_next, &g_fib_inv_hash_by_gtw[hash_inv_by_gtw], list_by_gtw)
    {
        if(tbs_nfp_l3_addr_eq(family, next_hop, fib->def_gtw_l3) &&
            !(fib->flags & TBS_NFP_F_FIB_ARP_INV))
        {
            memset(fib->da, 0, sizeof(fib->da));
            tbs_nfp_fib_invalid(fib, TBS_NFP_F_FIB_ARP_INV);
        }
    }

    return;
}

/*=========================================================================
 Function:		int tbs_nfp_fib_delete(int family, const char *sip,
    const char *dip, const char *gtw, int oif)

 Description:		从hash表中删除指定路由缓存规则
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    int family  协议族ipv4/6
                    const char *sip 源ip
                    const char *dip 目的ip
                    const char *gtw 网关地址
                    int oif 出接口
 Output:
 Return:			0:成功  其他:失败
 Others:
=========================================================================*/
int tbs_nfp_fib_add(int family, const unsigned char *sip, const unsigned char *dip,
    const unsigned char *gtw, int oif)
{
    TBS_NFP_RULE_FIB *new_entry = NULL;
    TBS_NFP_RULE_FIB *old_entry = NULL;
    unsigned int hash;
    unsigned int hash_by_gtw;
	int ip_len;

    TBS_NFP_RULE_ARP *arp_rule = NULL;
    TBS_NFP_IF_MAP *if_map = NULL;
    unsigned int addr_len = (family == AF_INET)?4:16;
    int ret = TBS_NFP_ERR;

#ifdef TBS_NFP_BRIDGE
    TBS_NFP_RULE_BRIDGE *bridge_rule = NULL;
#endif

	//规则总数不能超过 g_fib_max_size
	if(g_fib_max_size == tbs_nfp_fib_rule_num)
	{
        TBS_NFP_DEBUG("tbs_nfp_fib_rule_num(%u) == g_fib_max_size\n",
            tbs_nfp_fib_rule_num);

		return TBS_NFP_ERR;
	}

    //新建并添加规则 ,为了减少锁中代码量，将初始化放到锁前面
    new_entry = kmem_cache_alloc(tbs_nfp_fib_rule_cache, GFP_ATOMIC);
    if(NULL == new_entry)
    {
        TBS_NFP_ERROR("kmalloc new TBS_NFP_RULE_FIB faild \n");
        return TBS_NFP_ERR;
    }

	if(AF_INET == family) //ipv4
	{
		ip_len = TBS_NFP_MAX_IPV4_ADDR_SIZE;
	}
	else //ipv6
	{
		ip_len = TBS_NFP_MAX_L3_ADDR_SIZE;
	}

    memset(new_entry, 0, sizeof(*new_entry));
    new_entry->family = family;
	new_entry->oif    = oif;
    new_entry->flags |= TBS_NFP_F_FIB_ARP_INV;
    new_entry->ref    = 1;
    memcpy(new_entry->src_l3, sip, ip_len);
    memcpy(new_entry->dst_l3, dip, ip_len);
    memcpy(new_entry->def_gtw_l3, gtw, ip_len);

    memcpy(new_entry->src_l3, sip, addr_len);
    memcpy(new_entry->dst_l3, dip, addr_len);
    memcpy(new_entry->def_gtw_l3, gtw, addr_len);

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
    //检测是否已存在规则
    old_entry = tbs_nfp_fib_lookup(family, sip, dip, true);
    if(NULL == old_entry)
    {
        old_entry = tbs_nfp_fib_lookup(family, sip, dip, false);
    }

    //检测是否已存在规则
    if (old_entry)
    {
        //老规则需要删除
        tbs_nfp_ct_fib_invalid(old_entry);

        BUG_ON(old_entry->ref > 1);

        list_del(&old_entry->list);
        list_del(&old_entry->list_by_gtw);

        tbs_nfp_fib_rule_num--;
        kmem_cache_free(tbs_nfp_fib_rule_cache, old_entry);
		old_entry = NULL;
    }

    //检测出接口是否存在，并获取imap
    if_map = tbs_nfp_ifmap_get(oif);
    if (NULL == if_map)
    {
        kmem_cache_free(tbs_nfp_fib_rule_cache, new_entry);

        ret = TBS_NFP_ERR;
        goto end;
    }

    //初始化源mac为出接口的mac
    memcpy(new_entry->sa, if_map->mac, sizeof(new_entry->sa));

    //出接口是dummyport,全部替换为其父设备
#ifdef TBS_NFP_DUMMYPORT
    if((if_map->flags & TBS_NFP_F_MAP_DUMMYPORT))
    {
        if (NULL == if_map->parent_if)
        {
            kmem_cache_free(tbs_nfp_fib_rule_cache, new_entry);
            ret = TBS_NFP_ERR;
            goto end;
        }

#ifdef TBS_NFP_BRIDGE
        if (if_map->parent_if->flags & TBS_NFP_F_MAP_BRIDGE)
        {
            /*fib设置TBS_NFP_F_FIB_BRIDGE_INV, fib->oif = if_map->parent_if->ifidx*/
            new_entry->oif = if_map->parent_if->ifidx;
            new_entry->flags |= TBS_NFP_F_FIB_BRIDGE_INV;
        }
        else
#endif  /*TBS_NFP_BRIDGE*/
        {
            new_entry->oif = if_map->parent_if->ifidx;
        }
    }
    else
#endif  /*TBS_NFP_DUMMYPORT*/

#ifdef TBS_NFP_BRIDGE
	if(if_map->flags & TBS_NFP_F_MAP_BRIDGE)
    {
        //出接口是桥设备
        new_entry->oif = if_map->ifidx;
        new_entry->flags |= TBS_NFP_F_FIB_BRIDGE_INV;
    }
    else
#endif  /*TBS_NFP_BRIDGE*/

#ifdef TBS_NFP_PPP
    //出接口是ppp设备,不查询arp，直接从ifmap中获取对端的MAC地址
    if(if_map->flags & TBS_NFP_F_MAP_PPPOE)
    {
        memcpy(new_entry->da, if_map->remote_mac, sizeof(new_entry->da));
        memcpy(new_entry->sa, if_map->parent_if->mac, sizeof(new_entry->sa));

        goto fib_valid_rule;
    }
#endif  /*TBS_NFP_PPP*/

    arp_rule = tbs_nfp_arp_lookup(family, gtw);
    if (!arp_rule)
    {
        goto fib_invalid_rule;
    }
    else
    {
        memcpy(new_entry->da, arp_rule->da, sizeof(new_entry->da));
        new_entry->flags &= ~TBS_NFP_F_FIB_ARP_INV;

#ifdef TBS_NFP_BRIDGE
         /*如果出接口是桥接口(当前桥出口初始化时为不可用)*/
        if (new_entry->flags & TBS_NFP_F_FIB_BRIDGE_INV)
        {
            bridge_rule = tbs_nfp_bridge_rule_lookup(new_entry->da, if_map->mac, new_entry->oif);
            if (!bridge_rule)
            {
                /*未找到桥转发规则，将fib添加至不可用hash表*/
                 goto fib_invalid_rule;
            }
            else
            {  //找到桥转发规则,更改出接口为桥转发出接口
                new_entry->oif = bridge_rule->oif;
            }
        }
#endif /*TBS_NFP_BRIDGE*/
    }

    /*出接口是1)物理接口
              2)VLAN接口,
              3)桥接口已找到桥转发规则
              4)ppp接口
      将fib添加至可用两个fib hash表中，并更新ct规则
    */
#ifdef TBS_NFP_PPP
fib_valid_rule:
#endif  /*TBS_NFP_PPP*/

    hash = 	tbs_nfp_fib_hash(family, (unsigned int *)sip, (unsigned int *)dip, true);
    new_entry->flags &= ~TBS_NFP_F_FIB_INV_MASK;
    list_add(&new_entry->list,&g_fib_hash[hash]);

    hash_by_gtw = tbs_nfp_fib_hash_gtw(family, (unsigned int *)gtw, true);
    list_add(&new_entry->list_by_gtw, &g_fib_hash_by_gtw[hash_by_gtw]);

    tbs_nfp_fib_rule_num++;
    tbs_nfp_ct_fib_update(new_entry);

    ret = TBS_NFP_OK;
    goto end;

fib_invalid_rule:
    hash = 	tbs_nfp_fib_hash(family,(unsigned int *)sip, (unsigned int *)dip, false);
    list_add(&new_entry->list, &g_fib_inv_hash[hash]);

    hash_by_gtw = tbs_nfp_fib_hash_gtw(family, (unsigned int *)gtw, false);
    list_add(&new_entry->list_by_gtw, &g_fib_inv_hash_by_gtw[hash_by_gtw]);
    tbs_nfp_fib_rule_num++;

    ret = TBS_NFP_OK;

end:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}


/*=========================================================================
 Function:		int tbs_nfp_fib_delete(int family, const char *sip, const char *dip)

 Description:		从hash表中删除指定路由缓存规则
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    int family  协议族ipv4/6
                    const char *sip 源ip
                    const char *dip 目的ip
 Output:
 Return:			0:成功  其他:失败
 Others:
=========================================================================*/
inline int tbs_nfp_fib_delete(int family, const unsigned char *sip, const unsigned char *dip)
{
    TBS_NFP_RULE_FIB *entry = NULL;
    int ret = TBS_NFP_ERR;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
    //检测是否已存在规则

    entry = tbs_nfp_fib_lookup(family, sip, dip, true);
	if(NULL == entry)
		entry = tbs_nfp_fib_lookup(family, sip, dip, false);
	else
        tbs_nfp_ct_fib_invalid(entry);     /* 删除fib规则，先invalid ct 规则 */

    if (entry)
    {
        BUG_ON(entry->ref > 1);

        list_del(&entry->list);
        list_del(&entry->list_by_gtw);
        kmem_cache_free(tbs_nfp_fib_rule_cache, entry);

        ret = TBS_NFP_OK;
        tbs_nfp_fib_rule_num--;
        goto end;
    }

end:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}


/*=========================================================================
 Function:		int tbs_nfp_fib_age(int family, const char *sip, const char *dip)

 Description:		查询路由缓存规则匹配包计数
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    int family  协议族ipv4/6
                    const char *sip 源ip
                    const char *dip 目的ip
 Output:
 Return:			0:成功  其他:失败
 Others:
=========================================================================*/
int tbs_nfp_fib_age(int family, const unsigned char *sip, const unsigned char *dip)
{
    TBS_NFP_RULE_FIB *rule;
    unsigned int count = 0;

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);

    rule = tbs_nfp_fib_lookup(family, sip, dip, true);
    if(NULL == rule)
	{
		rule = tbs_nfp_fib_lookup(family, sip, dip, false);
		if(NULL == rule)
			goto out;
	}
    count = rule->count;
	rule->count = 0;

out:
    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);
    return count ;
}

/*=========================================================================
 Function:		static void tbs_nfp_fib_flush(void)

 Description:		删除所有fib转发规则
 Data Accessed:     g_fib_hash/g_fib_inv_hash
 Data Updated:
 Input:			    无
 Output:			无
 Return:
 Others:
=========================================================================*/
static void tbs_nfp_fib_flush(void)
{
    unsigned long hash = 0;
    TBS_NFP_RULE_FIB *tmp_entry = NULL;
    TBS_NFP_RULE_FIB *next_entry = NULL;

    TBS_NFP_INTO_FUNC;

    //依次遍历并删除规则g_fib_hash，g_fib_hash_by_gtw中规则也一并被删除
    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_fib_hash); hash++)
    {
        if(list_empty(&(g_fib_hash[hash])))
            continue;

        list_for_each_entry_safe(tmp_entry, next_entry, &g_fib_hash[hash], list)
        {
            tbs_nfp_ct_fib_invalid(tmp_entry);
            BUG_ON(tmp_entry->ref > 1);

            list_del(&tmp_entry->list);
            list_del(&tmp_entry->list_by_gtw);
            kmem_cache_free(tbs_nfp_fib_rule_cache, tmp_entry);
            tbs_nfp_fib_rule_num--;
        }
    }

    //依次遍历并删除规则g_fib_inv_hash，g_fib_inv_hash_by_gtw中规则也一并被删除
    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_fib_inv_hash); hash++)
    {
        if(list_empty(&(g_fib_inv_hash[hash])))
            continue;

        list_for_each_entry_safe(tmp_entry, next_entry, &g_fib_inv_hash[hash], list)
        {
            BUG_ON(tmp_entry->ref > 1);

            list_del(&tmp_entry->list);
            list_del(&tmp_entry->list_by_gtw);
            kmem_cache_free(tbs_nfp_fib_rule_cache, tmp_entry);
            tbs_nfp_fib_rule_num--;
        }
    }

    return;
}


/*=========================================================================
 Function:		void tbs_nfp_fib_reset(void)

 Description:		清除所有fib规则
 Data Accessed:     g_fib_hash/g_fib_inv_hash
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_fib_reset(void)
{
    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    tbs_nfp_fib_flush();

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return;
}
/*=========================================================================
 Function:		void tbs_nfp_fib_exit(void)

 Description:		销毁桥转发规则hash表，用于加速器模块退出
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    无
 Output:			无
 Return:			0:成功  其他:失败
 Others:
=========================================================================*/
void tbs_nfp_fib_exit(void)
{
    //销毁转发规则
    tbs_nfp_fib_reset();

    //释放内存cache
    kmem_cache_destroy(tbs_nfp_fib_rule_cache);
    return;
}


/*=========================================================================
 Function:		void tbs_nfp_fib_init(void)

 Description:		初始化fib规则hash表，用于加速器模块加载
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    无
 Output:			无
 Return:			0:成功  其他:失败
 Others:
=========================================================================*/
int tbs_nfp_fib_init(void)
{
    int i = 0;
    int ret = 0;

    //内存预分配缓存区
    tbs_nfp_fib_rule_cache = kmem_cache_create("tbs_nfp_fib_rule_cache",\
                                sizeof(struct tbs_nfp_rule_fib),\
                                0,\
                                SLAB_HWCACHE_ALIGN, NULL);
    if (!tbs_nfp_fib_rule_cache)
    {
        TBS_NFP_ERROR("Fail to kmem hash cache\n");
        ret = -ENOMEM;
        goto out;
    }

    //初始化hash表
    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_hash); i++)
    {
        INIT_LIST_HEAD(&g_fib_hash[i]);
    }

    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_hash_by_gtw); i++)
    {
        INIT_LIST_HEAD(&g_fib_hash_by_gtw[i]);
    }

    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_inv_hash); i++)
    {
        INIT_LIST_HEAD(&g_fib_inv_hash[i]);
    }

    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_inv_hash_by_gtw); i++)
    {
        INIT_LIST_HEAD(&g_fib_inv_hash_by_gtw[i]);
    }

out:
    return ret;

}


