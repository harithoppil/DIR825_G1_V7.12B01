/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_bridge.c
* 文件描述 : tbs加速器多播转发子模块
*
* 修订记录 :
*          1 创建 : pengyao
*            日期 : 2012-02-20
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
#include <linux/mm.h>
#include <linux/rtnetlink.h>
#include <linux/etherdevice.h>
#include <linux/notifier.h>


#include "tbs_nfp.h"
#include "tbs_nfp_mcf.h"
#include "tbs_nfp_itf.h"


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/



/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
static struct kmem_cache *tbs_nfp_mc_fdb_cache __read_mostly = NULL;
static struct kmem_cache *tbs_nfp_mcf_cache __read_mostly = NULL;

static unsigned int  tbs_nfp_mc_fdb_num = 0;
static unsigned ing tbs_nfp_mcf_num = 0;
static unsigned int g_mc_fdb_max_size __read_mostly = TBS_NFP_MC_FDB_DEF;
static unsigned int g_mc_mcf_max_size __read_mostly = TBS_NFP_MC_FDB_DEF;

static struct list_head g_mcf_cache_hash[TBS_NFP_MCF_HASH_SIZE];
static struct tbs_nfp_vif *nfp_vif[TBS_NFP_MAXVIFS] = {NULL};

#ifdef TBS_NFP_IGMP_SNOOPING
static struct list_head g_mc_fdb_hash[TBS_NFP_MC_FDB_HASH_SIZE];
#endif /* TBS_NFP_IGMP_SNOOPING */



/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

#ifdef TBS_NFP_IGMP_SNOOPING
/*=========================================================================
 Function:		int tbs_nfp_mc_fdb_add(int family, unsigned int br_port, unsigned char *src,
    unsigned char *group, int filt_mode
#if defined(TBS_NFP_IGMP_VLAN)
    , unsigned short      orig_vlan, unsigned short      new_vlan
#endif)

 Description:		mc fdb规则添加函数
 Data Accessed:     g_mc_fdb_hash
 Data Updated:
 Input:			    unsigned int br_port    桥端口
                    unsigned char *src      组播源地址
                    unsigned char *group    组播组地址
                    int filt_mode           filter模式
                    unsigned short      orig_vlan/new_vlan, 组播源/点播端vlanid
 Output:			无
 Return:			0   成功；其他  失败
 Others:
=========================================================================*/
int tbs_nfp_mc_fdb_add(int family, unsigned int br_port, unsigned char *src,
    unsigned char *group, int filt_mode
#if defined(TBS_NFP_IGMP_VLAN)
    , unsigned short      orig_vlan, unsigned short      new_vlan
#endif  /* TBS_NFP_IGMP_VLAN */
)
{
    ST_TBS_NFP_MC_FDB *new_mc_fdb = NULL;
    ST_TBS_NFP_MC_FDB *old_mc_fdb = NULL;
    unsigned int hash;
    int ip_len;
    TBS_NFP_IF_MAP *if_map = NULL;
    unsigned int addr_len = (family == AF_INET)?4:16;
    int ret = TBS_NFP_ERR;

    //规则总数不能超过 g_fib_max_size
    if(g_mc_fdb_max_size == tbs_nfp_mc_fdb_num)
    {
        TBS_NFP_DEBUG("tbs_nfp_mc_fdb_num(%u) == g_mc_fdb_max_size\n",
            tbs_nfp_mc_fdb_num);

        return TBS_NFP_ERR;
    }

    new_mc_fdb = kmem_cache_alloc(tbs_nfp_mc_fdb_cache, GFP_ATOMIC);
    if(NULL == new_mc_fdb)
    {
        TBS_NFP_ERROR("kmalloc new ST_TBS_NFP_MC_FDB faild \n");
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

    memset(new_mc_fdb, 0, sizeof(ST_TBS_NFP_MC_FDB));
    new_mc_fdb->family = family;
    new_mc_fdb->br_port = br_port;
    new_mc_fdb->filt_mode = filt_mode;
    memcpy(new_mc_fdb->src, src, ip_len);
    memcpy(new_mc_fdb->group, group, ip_len);

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    //检测是否已存在相同mc_fdb
#if defined(TBS_NFP_IGMP_VLAN)
    old_mc_fdb = tbs_nfp_mc_fdb_lookup(family, src, group, orig_vlan);
#else
    old_mc_fdb = tbs_nfp_mc_fdb_lookup(family, src, group);
#endif

    if (old_mc_fdb)
    {
        list_del(&old_mc_fdb->list);

        tbs_nfp_mc_fdb_num--;
        kmem_cache_free(tbs_nfp_mc_fdb_cache, old_entry);
        old_mc_fdb = NULL;
    }

    //br_port检查
    if_map = tbs_nfp_ifmap_get(br_port);
    if (NULL == if_map || !(if_map->flags & TBS_NFP_F_MAP_BRIDGE_PORT))
    {
        TBS_NFP_ERROR("br_port(%d) is not a bridge port\n", br_port);
        kmem_cache_free(tbs_nfp_mc_fdb_cache, new_mc_fdb);

        ret = TBS_NFP_ERR;
        goto end;
    }

#if defined(TBS_NFP_IGMP_VLAN)
    new_mc_fdb->orig_vid = orig_vlan;
    new_mc_fdb->new_vid = new_vlan;
#endif

    hash = tbs_nfp_mc_fdb_hash(family, src, group);
    list_add(&new_mc_fdb->list, &g_mcf_cache_hash[hash]);

end:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}


/*=========================================================================
 Function:		int tbs_nfp_mc_fdb_delete(int family, unsigned int br_port, unsigned char *src,
    unsigned char *group)

 Description:		mc fdb规则删除函数
 Data Accessed:     g_mc_fdb_hash
 Data Updated:
 Input:			    unsigned int br_port    桥端口
                    unsigned char *src      组播源地址
                    unsigned char *group    组播组地址
 Output:			无
 Return:			0   成功；其他  失败
 Others:
=========================================================================*/
int tbs_nfp_mc_fdb_delete(int family, unsigned int br_port, unsigned char *src,
    unsigned char *group
#if defined(TBS_NFP_IGMP_VLAN)
            , unsigned short      orig_vlan, unsigned short      new_vlan
#endif  /* TBS_NFP_IGMP_VLAN */
)
{
    ST_TBS_NFP_MC_FDB *mc_fdb = NULL;
    int ret = TBS_NFP_ERR;

    if(0 == tbs_nfp_mc_fdb_num)
    {
        TBS_NFP_DEBUG("tbs_nfp_mc_fdb_num == 0\n");

        return TBS_NFP_OK;
    }

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

#if defined(TBS_NFP_IGMP_VLAN)
    mc_fdb = tbs_nfp_mc_fdb_lookup(family, src, group, orig_vlan);
#else
    mc_fdb = tbs_nfp_mc_fdb_lookup(family, src, group);
#endif

    if (mc_fdb)
    {
        list_del(&mc_fdb->list);

        tbs_nfp_mc_fdb_num--;
        kmem_cache_free(tbs_nfp_mc_fdb_cache, mc_fdb);
    }

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}


/*=========================================================================
 Function:		TBS_NFP_MC_FDB *tbs_nfp_mc_fdb_lookup(int family, unsigned char *src, unsigned char *group
#if defined(TBS_NFP_IGMP_VLAN)
        , unsigned short      orig_vlan, unsigned short      new_vlan
#endif)

 Description:		mc fdb规则查找函数
 Data Accessed:     g_mc_fdb_hash
 Data Updated:
 Input:			    unsigned char *src      组播源地址
                    unsigned char *group    组播组地址
                    unsigned short      orig_vlan/new_vlan, 组播源/点播端vlanid

 Output:			无
 Return:			mc fdb规则指针
 Others:
=========================================================================*/
ST_TBS_NFP_MC_FDB *tbs_nfp_mc_fdb_lookup(int family, unsigned char *src, unsigned char *group
#if defined(TBS_NFP_IGMP_VLAN)
        , unsigned short      orig_vlan
#endif  /* TBS_NFP_IGMP_VLAN */
    )
{
    ST_TBS_NFP_MC_FDB *mc_fdb = NULL;
    unsigned int hash;
    int ip_len;
    unsigned int addr_len = (family == AF_INET)?4:16;
    int ret = TBS_NFP_ERR;

    //规则总数不能超过 g_fib_max_size
    if(0 == tbs_nfp_mc_fdb_num)
    {
        return NULL;
    }

    if(AF_INET == family) //ipv4
    {
        ip_len = TBS_NFP_MAX_IPV4_ADDR_SIZE;
    }
    else //ipv6
    {
        ip_len = TBS_NFP_MAX_L3_ADDR_SIZE;
    }

    hash = tbs_nfp_mc_fdb_hash(family, src, group);

    list_for_each_entry(mc_fdb, &g_mcf_cache_hash[hash], list)
    {
        if(mc_fdb->family == family &&
            (0 == mc_fdb->src || !memcmp(mc_fdb->src, src, ip_len)) &&
            !memcmp(mc_fdb->group, group, ip_len))
        {
#if defined(TBS_NFP_IGMP_VLAN)
            if(mc_fdb->orig_vid != orig_vlan)
                continue;
#endif
        }
    }

    return mc_fdb;
}


/*=========================================================================
 Function:		void tbs_nfp_mc_fdb_dump(void)

 Description:		mc fdb规则打印
 Data Accessed:     g_mc_fdb_hash
 Data Updated:
 Input:			    无

 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_mc_fdb_dump(void)
{
    int i = 0;
    ST_TBS_NFP_MC_FDB * mc_fdb = NULL;

    printk("mc_fdb_num: %d\nfilt_mode\t sip\t\t port\t group\t\t orig_vlan\t new_vlan\n",
        tbs_nfp_mc_fdb_num);

    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_mc_fdb_hash); i++){

        list_for_each_entry(mc_fdb, &g_mc_fdb_hash[i], list) {
            if(AF_INET == mc_fdb->family)
            {
                printk("%s\t %pI4\t\t %u\t %pI4\t\t %d\t %d\n",
                    (mc_fdb->filt == MCAST_EXCLUDE)?"MCAST_EXCLUDE":"MCAST_INCLUDE",
                    mc_fdb->src, mc_fdb->br_port, mc_fdb->group,
                    mc_fdb->orig_vid, mc_fdb->new_vid);
            }
            else
            {
                printk("%s\t %pI6\t\t %u\t %pI6\t\t %d\t %d\n",
                    (mc_fdb->filt == MCAST_EXCLUDE)?"MCAST_EXCLUDE":"MCAST_INCLUDE",
                    mc_fdb->src, mc_fdb->br_port, mc_fdb->group,
                    mc_fdb->orig_vid, mc_fdb->new_vid);
            }
        }
    }
}


/*=========================================================================
 Function:		void tbs_nfp_mc_fdb_reset(void)

 Description:		mc fdb规则清除
 Data Accessed:     g_mc_fdb_hash
 Data Updated:
 Input:			    无

 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_mc_fdb_reset(void)
{
    ST_TBS_NFP_MC_FDB *mc_fdb, mc_fdb_next = NULL;
    int i, ret = TBS_NFP_ERR;

    if(0 == tbs_nfp_mc_fdb_num)
        return;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
    for(i = 0, i < TBS_NFP_ARRAY_SIZE(g_mcf_cache_hash), i++)
    {
        if(list_empty(&g_mcf_cache_hash[i]))
            continue;

        list_for_each_entry_safe(mc_fdb, mc_fdb_next, &g_mcf_cache_hash[i], list)
        {
            list_del(&mc_fdb->list);
            tbs_nfp_mc_fdb_num--;
        }
    }

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return;
}
#endif /* TBS_NFP_IGMP_SNOOPING */


/*=========================================================================
 Function:		int tbs_nfp_vif_add(int family, int ifindex, unsigned rate_limit,
    unsigned char threshold, unsigned char flags)

 Description:		增加虚接口
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:			    int family                  协议族
                    int ifindex                 dev ifindex
                    unsigned rate_limit         rate_limit
                    unsigned char threshold     ttl
                    unsigned char flags         control flags
 Output:			0   成功；  其他    失败
 Return:			无
 Others:
=========================================================================*/
int tbs_nfp_vif_add(int family, int ifindex, unsigned rate_limit, unsigned char threshold,
    unsigned char flags)
{
}


/*=========================================================================
 Function:		int tbs_nfp_vif_delete(int family, int ifindex)

 Description:		删除虚接口
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:			    int family                  协议族
                    int ifindex                 dev ifindex
 Output:			0   成功；  其他    失败
 Return:			无
 Others:
=========================================================================*/
int tbs_nfp_vif_delete(int family, int ifindex)
{
}


int tbs_nfp_get_vif_index()
{

}

HARDIRQ_MASK
/*=========================================================================
 Function:		int tbs_nfp_mcf_add(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int ivif, ing flags, int vif_count, unsigned char *vif_info[VIF_INFO_MAX][TBS_NFP_MAXVIFS])

 Description:		mcf 规则添加函数
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:			    unsigned char *mc_group     组播组地址
                    unsigned char *mc_origin    组播源地址
                    int ivif                    组播入接口
                    ing flags                   mcf属性
                    int vif_count               组播分发接口数
                    unsigned char *vif_info     组播分发接口信息

 Output:			0   成功；  其他    失败
 Return:			无
 Others:
=========================================================================*/
int tbs_nfp_mcf_add(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int in_vif, ing flags, int vif_count, unsigned char *vif_info[VIF_INFO_MAX][TBS_NFP_MAXVIFS])
{

}


/*=========================================================================
 Function:		int tbs_nfp_mcf_delete(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int ivif)

 Description:		mcf 规则删除函数
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:			    unsigned char *mc_group     组播组地址
                    unsigned char *mc_origin    组播源地址
                    int ivif                    组播入接口

 Output:			0   成功；  其他    失败
 Return:			无
 Others:
=========================================================================*/
int tbs_nfp_mcf_delete(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int ivif)
{

}


/*=========================================================================
 Function:		int tbs_nfp_mcf_delete(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int ivif)

 Description:		mcf 规则解析函数
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:			    unsigned char *mc_group     组播组地址
                    unsigned char *mc_origin    组播源地址
                    int ivif                    组播入接口

 Output:			mcf 规则
 Return:			无
 Others:
=========================================================================*/
ST_TBS_NFP_MCF *tbs_nfp_mcf_lookup(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int ivif)
{

}


/*=========================================================================
 Function:		void tbs_nfp_mcf_dump(void)

 Description:		mcf 规则打印函数
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_mcf_dump(void)
{
    int i, j = 0;
    ST_TBS_NFP_MCF *mcf_cache = NULL;

    printk("mcf_cache_num: %d\norigin_ip\t\t mc_group\t\t in_if\t flags\t wrong_if\t bytes\t "
        "pkt\t out_if\n", tbs_nfp_mcf_num);

    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_mc_fdb_hash); i++){

        list_for_each_entry(mc_fdb, &g_mc_fdb_hash[i], list) {
            if(AF_INET == mc_fdb->family)
            {
                printk("%pI4\t\t %pI4\t\t %d\t 0x%08x\t %d\t %lu\t %lu\t ",
                    mcf_cache->mfc_origin, mcf_cache->mfc_origin, mcf_cache->mfc_parent,
                    mcf_cache->mfc_flags, mcf_cache->wrong_if, mcf_cache->bytes,
                    mcf_cache->pkt);
            }
            else
            {
                printk("%pI6\t\t %pI6\t\t %d\t 0x%08x\t %d\t %lu\t %lu\t ",
                    mcf_cache->mfc_origin, mcf_cache->mfc_origin, mcf_cache->mfc_parent,
                    mcf_cache->mfc_flags, mcf_cache->wrong_if, mcf_cache->bytes,
                    mcf_cache->pkt);
            }

            for(j = 0; j < mcf_cache->vif_num; j++)
            {
                printk("%2d:%-3d", mcf_cache->vif_info[VIF_INDEX][j],
                    mcf_cache->vif_info[VIF_TTL][j]);
            }

            printk("\n");
        }
    }
}


/*=========================================================================
 Function:		void tbs_nfp_mcf_reset(void)

 Description:		mcf 规则清除函数
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/

void tbs_nfp_mcf_reset(void)
{

}


/*=========================================================================
 Function:		void tbs_nfp_mcf_init(void)

 Description:		mcf子模块初始化函数
 Data Accessed:     g_mcf_cache_hash/g_mc_fdb_hash
 Data Updated:
 Input:			    五元组信息
 Output:			无
 Return:			无
 Others:
=========================================================================*/
int tbs_nfp_mcf_init(void)
{
    unsigned int i = 0;
    int ret = 0;

    //内存告诉缓存
    tbs_nfp_mcf_cache = kmem_cache_create("tbs_nfp_mcf_cache",
            sizeof(TBS_NFP_TUPLE_CT),
            0,
            SLAB_HWCACHE_ALIGN, NULL);
    if(!tbs_nfp_mcf_cache)
    {
        TBS_NFP_ERROR("Fail to kmem hash cache tbs_nfp_mcf_cache \n");
        ret = -ENOMEM;
        goto out;
    }

    tbs_nfp_mcf_num = 0;

    //初始化hash表
    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_mcf_cache_hash); i++)
    {
        INIT_LIST_HEAD(&g_mcf_cache_hash[i]);
    }

#ifdef TBS_NFP_IGMP_SNOOPING
    tbs_nfp_mc_fdb_num = 0;

    tbs_nfp_mc_fdb_cache = kmem_cache_create("tbs_nfp_mc_fdb_cache",
            sizeof(TBS_NFP_TUPLE_CT),
            0,
            SLAB_HWCACHE_ALIGN, NULL);
    if(!tbs_nfp_mc_fdb_cache)
    {
        TBS_NFP_ERROR("Fail to kmem hash cache tbs_nfp_mc_fdb_cache \n");
        ret = -ENOMEM;
        goto out;
    }

    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_mc_fdb_hash); i++)
    {
        INIT_LIST_HEAD(&g_mc_fdb_hash[i]);
    }
#endif /* TBS_NFP_IGMP_SNOOPING */

out:
    return ret;
}


/*=========================================================================
 Function:		void tbs_nfp_mcf_exit(void)

 Description:		mcf子模块退出函数
 Data Accessed:     g_mcf_cache_hash/g_mc_fdb_hash
 Data Updated:
 Input:			    五元组信息
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_mcf_exit(void)
{
    tbs_nfp_mcf_reset();

	//释放内存cache
	kmem_cache_destroy(tbs_nfp_mcf_cache);

#ifdef TBS_NFP_IGMP_SNOOPING
    tbs_nfp_fdb_reset();
    kmem_cache_destroy(tbs_nfp_mc_fdb_cache);
#endif
}

