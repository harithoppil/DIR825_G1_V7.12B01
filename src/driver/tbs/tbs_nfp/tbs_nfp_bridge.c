/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_bridge.c
* 文件描述 : tbs加速器bridge模块
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
#include "tbs_nfp_bridge.h"
#include "tbs_nfp_itf.h"

#ifdef TBS_NFP_FIB
#include "tbs_nfp_fib.h"
#endif


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];


/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
static struct kmem_cache *tbs_nfp_bridge_rule_cache __read_mostly;
static unsigned int  tbs_nfp_bridge_rule_num = 0;
unsigned int g_bridge_max_size __read_mostly = TBS_NFP_BRIDGE_RULE_DEF;


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/


/*=========================================================================
 Function:		int tbs_nfp_bridge_rule_flush(void)

 Description:		对外提供删除所有转发规则的接口
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    无
 Output:			无
 Return:
 Others:
=========================================================================*/
void tbs_nfp_bridge_rule_flush(void)
{
    unsigned long hash = 0;
    TBS_NFP_RULE_BRIDGE *tmp_entry = NULL;
    TBS_NFP_RULE_BRIDGE *next_entry = NULL;

    TBS_NFP_INTO_FUNC;

    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_bridge_hash); hash++)
    {
        if(list_empty(&(g_bridge_hash[hash])))
            continue;

        list_for_each_entry_safe(tmp_entry, next_entry, &g_bridge_hash[hash], list)
        {
            list_del(&tmp_entry->list);
            kmem_cache_free(tbs_nfp_bridge_rule_cache, tmp_entry);
            tbs_nfp_bridge_rule_num--;
        }
    }
    return;
}



/*=========================================================================
 Function:		void tbs_nfp_bridge_rule_clear_by_ifindex(int ifindex)

 Description:		按桥接口ifindex清除桥转发规则，用于br_port删除
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    int ifindex     br_port ifindex
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_bridge_rule_clear_by_ifindex(int ifindex)
{
    unsigned long hash = 0;
    TBS_NFP_RULE_BRIDGE *tmp_entry = NULL;
    TBS_NFP_RULE_BRIDGE *next_entry = NULL;

    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_bridge_hash); hash++)
    {
        if(list_empty(&(g_bridge_hash[hash])))
            continue;

        list_for_each_entry_safe(tmp_entry, next_entry, &g_bridge_hash[hash], list)
        {
            if (tmp_entry->iif==ifindex
                    || tmp_entry->oif==ifindex)
            {
                list_del(&tmp_entry->list);
                kmem_cache_free(tbs_nfp_bridge_rule_cache, tmp_entry);
                tbs_nfp_bridge_rule_num--;
            }
        }
    }
}


/*=========================================================================
 Function:		void tbs_nfp_bridge_rule_print(int hash_key, const TBS_NFP_RULE_BRIDGE * br_rule)

 Description:		打印指定桥转发规则，用于调试诊断
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    NFP_RULE_BRIDGE * br_rule   要打印的规则
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_bridge_rule_print(int hash_key, const TBS_NFP_RULE_BRIDGE * br_rule)
{
    TBS_NFP_IF_MAP *iif_map, *oif_map = NULL;

    iif_map = tbs_nfp_ifmap_get(br_rule->iif);
    oif_map = tbs_nfp_ifmap_get(br_rule->oif);

    if(NULL == iif_map || NULL == oif_map)
    {
        printk("Get if_map fail(iif:%d, oif:%d)\n", br_rule->iif, br_rule->oif);
        return;
    }

    printk("%u\t\t%s(%u)\t\t%s(%u)\t\t%02x:%02x:%02x:%02x:%02x:%02x\t"
        "%02x:%02x:%02x:%02x:%02x:%02x\t%u\n",
        hash_key, iif_map->name, iif_map->ifidx,
        oif_map->name, oif_map->ifidx,
        br_rule->sa[0], br_rule->sa[1],
        br_rule->sa[2], br_rule->sa[3],
        br_rule->sa[4], br_rule->sa[5],
        br_rule->da[0], br_rule->da[1],
        br_rule->da[2], br_rule->da[3],
        br_rule->da[4], br_rule->da[5], br_rule->count);

    return;
}


/*=========================================================================
 Function:		void tbs_nfp_bridge_rule_dump(void)

 Description:		打印所有桥转发规则，用于调试诊断
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_bridge_rule_dump(void)
{
    unsigned long hash = 0;
    TBS_NFP_RULE_BRIDGE *tmp_entry = NULL;

    printk("hash_key\tin_ifname\tout_ifname\tsrc_mac\t\t\tdst_mac\t\t\tcount\n");
    TBS_NFP_READ_LOCK(&tbs_nfp_lock);
    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_bridge_hash); hash++)
    {
        if(list_empty(&(g_bridge_hash[hash])))
            continue;

        list_for_each_entry(tmp_entry, &g_bridge_hash[hash], list)
        {
            tbs_nfp_bridge_rule_print(hash, tmp_entry);
        }
    }
    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

    return;
}



/*=========================================================================
 Function:		INLINE int tbs_nfp_bridge_rule_cmp(const unsigned char *da, const unsigned char *sa,
    int iif, int oif, TBS_NFP_RULE_BRIDGE *rule)

 Description:		桥转发规则比较
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    u8 *sa      源MAC地址
                    u8 *da      目的MAC地址
                    int iif     入接口ifindex
 Output:			无
 Return:			TBS_NFP_TRUE: 规则相同；TBS_NFP_FALSE:不相同
 Others:
=========================================================================*/
inline int tbs_nfp_bridge_rule_cmp(const unsigned char *da, const unsigned char *sa,
                                   int iif, TBS_NFP_RULE_BRIDGE *rule)
{
    if ((iif == rule->iif) &&
            (*(unsigned short *)(rule->da)) == (*(unsigned short *)(da))	 &&
            (*(unsigned int *)(rule->da + 2)) == (*(unsigned int *)(da + 2)) &&
            (*(unsigned int *)(rule->sa)) == (*(unsigned int *)(sa))	 &&
            (*(unsigned short *)(rule->sa + 4)) == (*(unsigned short *)(sa + 4)))
        return true;

    return false;
}

/*=========================================================================
 Function:		static INLINE TBS_NFP_RULE_BRIDGE *tbs_nfp_bridge_rule_lookup(const unsigned char *da,
            const unsigned char *sa, int iif, int oif)

 Description:		桥转发规则解析
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    u8 *sa      源MAC地址
                    u8 *da      目的MAC地址
                    int iif     入接口ifindex
 Output:			无
 Return:			NULL: 规则不存在；其他:返回bridge rule
 Others:
=========================================================================*/
inline TBS_NFP_RULE_BRIDGE *tbs_nfp_bridge_rule_lookup(const unsigned char *da,
        const unsigned char *sa, int iif)
{
    unsigned int hash;
    TBS_NFP_RULE_BRIDGE *rule;

    hash = 	tbs_nfp_bridge_hash(da, sa, iif);
    if(list_empty(&g_bridge_hash[hash]))
        return NULL;

    list_for_each_entry(rule, &g_bridge_hash[hash], list) {
        if (tbs_nfp_bridge_rule_cmp(da, sa, iif, rule))
            return rule;
    }

    return NULL;
}


/*=========================================================================
 Function:		static bool tbs_nfp_bridge_rule_get(const unsigned char *da,
            const unsigned char *sa, int iif, int oif)

 Description:		桥转发规则是否存在
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    u8 *sa      源MAC地址
                    u8 *da      目的MAC地址
                    int iif     入接口ifindex
 Output:			无
 Return:			true:   存在，flse: 不存在
 Others:
=========================================================================*/
inline bool tbs_nfp_bridge_rule_exist(const unsigned char *da,
        const unsigned char *sa, int iif)
{
    bool exist = false;

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);

    if(tbs_nfp_bridge_rule_lookup(da, sa, iif))
        exist = true;

    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

    return exist;
}


/*=========================================================================
 Function:		int tbs_nfp_bridge_rule_add(const u8 *sa, const u8 *da, int iif, int oif)

 Description:		添加桥转发规则
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    u8 *sa      源MAC地址
                    u8 *da      目的MAC地址
                    int iif     入接口ifindex
                    int oif     出接口ifindex
 Output:			无
 Return:			TBS_NFP_OK: 成功；其他:失败
 Others:
=========================================================================*/
inline int tbs_nfp_bridge_rule_add(const u8 *sa, const u8 *da, int iif, int oif)
{
    unsigned int hash;
    TBS_NFP_RULE_BRIDGE *new_entry = NULL;
#ifdef TBS_NFP_FIB
    TBS_NFP_IF_MAP *ifmap = NULL;
#endif

    //当前规则是否已经存在
    if(tbs_nfp_bridge_rule_exist(da, sa, iif))
        return TBS_NFP_OK;

    //规则总数不能超过 g_bridge_max_size
    if (g_bridge_max_size == tbs_nfp_bridge_rule_num)
    {
        TBS_NFP_DEBUG("tbs_nfp_bridge_rule_num(%u) == g_bridge_max_size\n",
            tbs_nfp_bridge_rule_num);

        return TBS_NFP_ERR;
    }

    new_entry = kmem_cache_alloc(tbs_nfp_bridge_rule_cache, GFP_ATOMIC);
    if(NULL == new_entry)
    {
        TBS_NFP_ERROR("kmalloc new TBS_NFP_RULE_BRIDGE faild \n");
        return TBS_NFP_ERR;
    }

    hash = 	tbs_nfp_bridge_hash(da, sa, iif);
    memset(new_entry, 0, sizeof(*new_entry));
    memcpy(new_entry->da, da, sizeof(new_entry->da));
    memcpy(new_entry->sa, sa, sizeof(new_entry->sa));
    new_entry->iif = iif;
    new_entry->oif = oif;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

#ifdef TBS_NFP_FIB
        ifmap = tbs_nfp_ifmap_get(new_entry->iif);

        if(ifmap && (ifmap->flags & TBS_NFP_F_MAP_BRIDGE))
            tbs_nfp_fib_bridge_update(new_entry);
#endif

    tbs_nfp_bridge_rule_num++;
    list_add(&new_entry->list,&g_bridge_hash[hash]);

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return TBS_NFP_OK;
}


/*=========================================================================
 Function:		int tbs_nfp_bridge_rule_delete(const u8 *sa, const u8 *da, int iif)

 Description:		删除桥转发规则
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    u8 *sa      源MAC地址
                    u8 *da      目的MAC地址
                    int iif     入接口ifindex
 Output:			无
 Return:			TBS_NFP_OK: 成功；其他:失败
 Others:
=========================================================================*/
inline int tbs_nfp_bridge_rule_delete(const u8 *sa, const u8 *da, int iif)
{
    int ret = TBS_NFP_OK;
    TBS_NFP_RULE_BRIDGE *rule = NULL;
#ifdef TBS_NFP_FIB
    TBS_NFP_IF_MAP *ifmap = NULL;
#endif

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    rule = tbs_nfp_bridge_rule_lookup(da, sa, iif);
    if(NULL == rule)
    {
        ret = TBS_NFP_ERR;
        goto out;
    }

#ifdef TBS_NFP_FIB
    ifmap = tbs_nfp_ifmap_get(rule->iif);

    if(ifmap && (ifmap->flags & TBS_NFP_F_MAP_BRIDGE))
        tbs_nfp_fib_bridge_invalid(rule->iif, rule->oif, rule->da);
#endif

    list_del(&rule->list);
    kmem_cache_free(tbs_nfp_bridge_rule_cache, rule);
    tbs_nfp_bridge_rule_num--;

out:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return ret;
}


/*=========================================================================
 Function:		int tbs_nfp_bridge_rule_age(const u8 *sa, const u8 *da)

 Description:		桥转发规则老化查询
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    u8 *sa      源MAC地址
                    u8 *da      目的MAC地址
                    int iif     入接口ifindex
 Output:			无
 Return:			0: 已经老化；其他:未老化
 Others:
=========================================================================*/
inline int tbs_nfp_bridge_rule_age(const u8 *sa, const u8 *da, int iif)
{
    TBS_NFP_RULE_BRIDGE *rule = NULL;
    unsigned int count= 0;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    rule = tbs_nfp_bridge_rule_lookup(da, sa, iif);
    if(NULL == rule)
    {
        goto out;
    }

    count = rule->count;
	rule->count=0;

out:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return count;
}


/*=========================================================================
 Function:		void tbs_nfp_bridge_reset(void)

 Description:		清除桥转发规则
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_bridge_reset(void)
{
    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
    tbs_nfp_bridge_rule_flush();
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
}


/*=========================================================================
 Function:		void tbs_nfp_bridge_reset(void)

 Description:		bridge子模块退出
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_bridge_rule_exit(void)
{
    //销毁桥转发规则
    TBS_NFP_INTO_FUNC;
    tbs_nfp_bridge_rule_flush();

    //释放内存cache
    kmem_cache_destroy(tbs_nfp_bridge_rule_cache);
    return;
}


/*=========================================================================
 Function:		void tbs_nfp_bridge_rule_init(void)

 Description:		初始化桥转发规则hash表，用于加速器模块加载
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
int tbs_nfp_bridge_rule_init(void)
{
    int ret = 0;
    int i = 0;

    //内存预分配缓存区
    tbs_nfp_bridge_rule_cache = kmem_cache_create("tbs_nfp_bridge_rule_cache",\
                                sizeof(struct nfp_rule_bridge),\
                                0,\
                                SLAB_HWCACHE_ALIGN, NULL);
    if (!tbs_nfp_bridge_rule_cache)
    {
        TBS_NFP_ERROR("Fail to kmem hash cache\n");
        ret = -ENOMEM;
        goto out;
    }


    //初始化hash表
    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_bridge_hash); i++)
    {
        INIT_LIST_HEAD(&g_bridge_hash[i]);
    }

out:
    return ret;
}

