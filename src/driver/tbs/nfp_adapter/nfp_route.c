/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称 : nfp_route.c
 文件描述 : TBS加速器适配路由子模块

 修订记录 :
          1 创建 : 伍国祥
            日期 : 2011-04-15
            描述 :
**********************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/notifier.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include <net/route.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/list.h>

#include "nfp_interface.h"
#include "nfp_neighbour.h"
#include "nfp_conntrack.h"
#include "nfp_route.h"
#include "nfp_adapter.h"


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/

/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/

/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/
extern int neigh_lookup_for_nfp(int family, const void *pkey,
			       int ifindex, unsigned char *mac_addr, int addrlen);

#ifdef CONFIG_TBS_NFP_FIB
extern int (*tbs_nfp_hook_fib_age)(int, u_int32_t*, u_int32_t *, int, int);
#endif


#if defined(CONFIG_NFP_ADAPTER_DEBUG)
/*=========================================================================
 Function:		static inline void nfp_route_dump(const struct rt_entry *rt_entry)

 Description:		调试输出函数
 Data Accessed:

 Data Updated:
 Input:			    struct rt_entry *rt_entry
 Output:			无
 Return:			无

 Others:
=========================================================================*/
static inline void nfp_route_dump(struct rt_entry *rt_entry)
{

    char addr_buf[NFP_INET_ADDRSTRLEN] = {0};
    NFP_ASSERT(rt_entry);

    NFP_INET_NTOP(rt_entry->family, &rt_entry->fl.fl4_src, addr_buf, sizeof(addr_buf));

    printk("%s\t%s\t",
           (rt_entry->family == AF_INET6) ? "AF_INET6" : "AF_INET", addr_buf);

    NFP_INET_NTOP(rt_entry->family, &rt_entry->fl.fl4_dst, addr_buf, sizeof(addr_buf));

    printk("%s\t", addr_buf);

    NFP_INET_NTOP(rt_entry->family, &rt_entry->gateway, addr_buf, sizeof(addr_buf));

    printk("%s\t%d\t%d\t%d\n",
           addr_buf, rt_entry->fl.iif,
           rt_entry->oif ? rt_entry->oif->ifindex : 0,
           rt_entry->phyoif ? rt_entry->phyoif->ifindex : 0);
    return;
}
#endif

static inline int nfp_route_option(unsigned short option, struct rt_entry *route)
{
    int ret = NFP_UNABLE_PARSER;

    if(nfp_cmd_parser){
        struct nfp_adapter_cmd cmd_entry = {option, (void *)route};
        ret = nfp_cmd_parser(NFP_CMD_FIB, &cmd_entry);
    }
    else if(NFP_OPT_STAT == option)
        ret = NFP_RULE_TIMEOUT;

    return ret;
}

/*=========================================================================
 Function:		static int nfp_get_fl_type(struct flowi *fl)

 Description:		获取flow的类型，分为INPUT和OUTPUT类型
 Data Accessed:

 Data Updated:
 Input:			    struct flowi *fl
 Output:			无
 Return:			0:成功

 Others:
=========================================================================*/
static inline int nfp_get_fl_type(struct flowi *fl)
{
    if (0 == fl->oif)
        return NFP_FL_INPUT;
    else
        return NFP_FL_OUTPUT;
}

static int nfp_route_add(struct rt_entry *rt_entry)
{
	int ret = -1;

	NFP_ADAPTER_INTO_FUNC;
	NFP_ASSERT(rt_entry);

	NFP_ROUTE_DEBUG("NFP ROUTE ADD:\n");

	/* 添加加速器中规则 */
	ret = nfp_route_option(NFP_OPT_ADD, rt_entry);
	if(ret != NFP_UNABLE_PARSER && NFP_NO_ERR != ret) {
		NFP_ROUTE_DEBUG("call nfp_route_option(NFP_OPT_ADD, route) error: %d\n", ret);
#if defined(CONFIG_NFP_ADAPTER_DEBUG)
		nfp_route_dump(rt_entry);
#endif

		goto out;
	}

out:
	return ret;
}


static int nfp_route_del(struct rt_entry *rt_entry)
{
	int ret = 0;

	NFP_ADAPTER_INTO_FUNC;
	NFP_ASSERT(rt_entry);

	NFP_ROUTE_DEBUG("NFP ROUTE DELETE:\n");

	/* 删除加速器中规则 */
	ret = nfp_route_option(NFP_OPT_DELETE, rt_entry);
	if(ret != NFP_UNABLE_PARSER && NFP_NO_ERR != ret) {
		NFP_ROUTE_DEBUG("call nfp_route_option(NFP_OPT_DELETE, route) error: %d\n", ret);

		goto out;
	}

out:
	return ret;
}


/*
 * TBS_TAG: add by baiyonghui 2011-9-10
 * Description:
 */
#ifdef CONFIG_TBS_NFP_FIB
static inline int nfp_route_age(int family, u_int32_t *src_l3, u_int32_t *dst_l3, int iif, int oif)
{
    struct rt_entry fib_entry;

    if(!nfp_interface_exist(iif) || !nfp_interface_exist(oif))
        return 0;

    memset(&fib_entry, 0, sizeof(fib_entry));
    fib_entry.family = family;

    if(AF_INET == family)
    {
        fib_entry.fl.fl4_src = *src_l3;
        fib_entry.fl.fl4_dst = *dst_l3;
    }
    else if(AF_INET6 == family)
    {
        memcpy(&(fib_entry.fl.fl6_src), src_l3, 16);
        memcpy(&(fib_entry.fl.fl6_dst), dst_l3, 16);
    }
    else
        return -1;

	if(NFP_RULE_TIMEOUT == nfp_route_option(NFP_OPT_STAT, &fib_entry))
		return 0;
	return NFP_RULE_ACTIVE;
}
#endif
/*
 * TBS_END_TAG
 */


/*=========================================================================
 Function:		static void nfp_route_proc_exit( void )
 Description:		route规则状态查询proc接口删除
 Data Accessed:

 Data Updated:
 Input:			无
 Output:			无
 Return:			无

 Others: 			删除/proc/nfp_route_entry文件
=========================================================================*/

static void nfp_route_proc_exit( void )
{
    remove_proc_entry("nfp_route_entry", nfp_adapter_proc);
}

/*=========================================================================
 Function:		    struct interface_entry * nfp_route_get_interface(int ifidx)
 Description:		获取设备索引对应的interface
 Data Accessed:

 Data Updated:
 Input:			    逻辑设备的ifindex
 Output:			无
 Return:			设备索引对应的interface

 Others:
=========================================================================*/
struct interface_entry * nfp_route_get_interface(int ifidx)
{
    struct interface_entry * if_entry = NULL;

    NFP_ADAPTER_INTO_FUNC;

    if_entry = nfp_interface_query(ifidx);  /* 该接口由interface子模块提供*/
    if (NULL == if_entry)
        NFP_ROUTE_DEBUG("Interface query failed, itf_index = %d\n", ifidx);

    return if_entry;
}

#ifndef CONFIG_TBS_NFP_FIB
/*=========================================================================
 Function:		    static struct interface_entry * nfp_route_get_phyif(struct interface_entry *oif)
 Description:		获取设备对应的phy interface
 Data Accessed:

 Data Updated:
 Input:			    逻辑设备的interface
 Output:			无
 Return:			设备对应的phy interface

 Others:
=========================================================================*/
static struct interface_entry * nfp_route_get_phyif(struct rt_entry *route)
{
    int ret = 0;
    struct interface_entry * oif = NULL;
    struct interface_entry * phyif = NULL;

    NFP_ASSERT(route);
    NFP_ADAPTER_INTO_FUNC;

    oif = route->oif;
    if (NULL == oif)
        return NULL;

    //lan侧dummyport则要找出对应的lan接口
    if (((oif->itf_flags & ITF_DUMMYPORT) && (oif->itf_flags & ITF_TYPE_LAN))
        || (oif->itf_flags & ITF_BRIDGE))
    {
#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE)
        unsigned char mac_addr[6] = {0};
        unsigned char *paddr = mac_addr;

        struct net_bridge_fdb_entry *fdb = NULL;
        struct net_bridge *br = NULL;
        struct net_device *br_dev = NULL;

        ret = neigh_lookup_for_nfp(route->family, &route->gateway,
                               route->oif->ifindex, mac_addr, sizeof(mac_addr));
        if(ret || is_zero_ether_addr(mac_addr))
        {
            NFP_ROUTE_DEBUG("neigh lookup faild by gtw:0x%08x failed,out_dev = %s\n",
                route->gateway, oif->ifname);

            return NULL;
        }

        if(oif->itf_flags & ITF_BRIDGE){
            br_dev = dev_get_by_index(&init_net,oif->ifindex);
        }
        else{
            br_dev = dev_get_by_name(&init_net,"br1");
        }

        if(NULL == br_dev){
            NFP_ROUTE_DEBUG("get bridge dev faild:%s\n", oif->ifname);
            goto put;
        }

        br = netdev_priv(br_dev);

        rcu_read_lock();
        fdb = __br_fdb_get(br, paddr);
        if(!fdb){
            NFP_ROUTE_DEBUG("get fdb faild:%02x:%02x:%02x:%02x:%02x:%02x\n",
                paddr[0], paddr[1], paddr[2],
                paddr[3], paddr[4], paddr[5]);

            goto put;
        }

        NFP_ROUTE_DEBUG("fdb->dst->dev->name:%s\n", fdb->dst->dev->name);
        phyif = nfp_route_get_interface(fdb->dst->dev->ifindex);

put:
        rcu_read_unlock();
        if(br_dev)
            dev_put(br_dev);

#endif
    }
    else
        phyif = nfp_route_get_interface(oif->phys_ifindex);
#if 1
    if (NULL == phyif)
        NFP_ROUTE_DEBUG("Get phy interface failed.\n");
#endif
    return phyif;
}
#endif


static int nfp_route_setup_entry(struct rt_entry *route, struct rtable *rt,
    unsigned long events)
{
    NFP_ASSERT(route);
    NFP_ASSERT(rt);

    NFP_ADAPTER_INTO_FUNC;

    memcpy(&route->fl, &rt->fl, sizeof(struct flowi));
    if(RT_CACHE_NEW == events)
    {
        route->oif = nfp_route_get_interface(rt->idev->dev->ifindex);
        if (NULL == route->oif) {
            NFP_ROUTE_DEBUG("Get output interface failed..\n");
            goto err;
        }

#if defined(CONFIG_TBSMOD_DUMMYPORT)
        /*出接口为dummyport，且未绑定物理接口，不做处理*/
        if((route->oif->itf_flags & ITF_DUMMYPORT) &&
            (((route->oif->itf_flags & (ITF_BIND_PHY_ITF | ITF_NFP_ENABLE)) !=
            (ITF_BIND_PHY_ITF | ITF_NFP_ENABLE))))
        {
            NFP_ROUTE_DEBUG("out_device: %s, dummyport unbind physics device\n", route->oif->ifname);
            goto err;
        }
#endif
    }

    NFP_ROUTE_DEBUG("\n\nroute %s, rt = 0x%p, lastuse: %lu(%lus before), expires:%lu, jiffies:%lu, rt_flags: 0x%x\n\n",
        (RT_CACHE_NEW == events)?"add":"delete", rt,
        rt->dst.lastuse, (jiffies - rt->dst.lastuse)/HZ,
        rt->dst.expires, jiffies, rt->rt_flags);

    route->gateway = rt->rt_gateway;
    route->mtu = rt->idev->dev->mtu;

    /*tbs_nfp不需要获取br_port, 此处可能会获取失败*/
#ifndef CONFIG_TBS_NFP_FIB
    route->phyoif = nfp_route_get_phyif(route);
#endif

    NFP_ADAPTER_OUT_FUNC;

    return 0;

err:
    if(route->oif)
        nfp_interface_put(route->oif);
    if(route->phyoif)
        nfp_interface_put(route->phyoif);

    NFP_ADAPTER_OUT_FUNC;

    return -1;
}


/*=========================================================================
 Function:		    static int nfp_ipv4_route_event( void )
 Description:		路由缓存通知链事件处理函数
 Data Accessed:

 Data Updated:
 Input:			    无
 Output:			struct notifier_block *this
                    unsigned long events
                    void *ptr
 Return:			无

 Others:
=========================================================================*/
static int nfp_ipv4_route_event(struct notifier_block *this,
				     unsigned long events, void *ptr)
{
    int ret = 0;
    struct rtable *rt = (struct rtable *)ptr;

    struct rt_entry route;
    int fl_type = 0;
    char saddr_buf[NFP_INET_ADDRSTRLEN] = {0};
    char daddr_buf[NFP_INET_ADDRSTRLEN] = {0};

    NFP_ADAPTER_INTO_FUNC;
    NFP_ASSERT(rt);

    memset(&route, 0, sizeof(route));
    NFP_INET_NTOP(rt->dst.ops->family, &(rt->fl.fl4_src), saddr_buf, sizeof(saddr_buf));
    NFP_INET_NTOP(rt->dst.ops->family, &(rt->fl.fl4_dst), daddr_buf, sizeof(daddr_buf));

    //NFP_ROUTE_DEBUG("route event: %lu, saddr_buf:%s, daddr_buf:%s\n", events, saddr_buf, daddr_buf);

    /*过滤本地发出的路由缓存和单臂的路由缓存*/
    if(rt->fl.iif == 0 || NULL == rt->idev ||
        rt->fl.iif == rt->idev->dev->ifindex)
    {
        NFP_ROUTE_DEBUG("rt->fl.iif: %d\n", rt->fl.iif);
        return NOTIFY_DONE;
    }

    /*不关注的接口，不做处理*/
    if(!(nfp_interface_exist(rt->fl.iif) && nfp_interface_exist(rt->idev->dev->ifindex)))
    {
        NFP_ROUTE_DEBUG("nfp_interface_exist,fl.iif:%d, rt->idev->dev->ifindex:%d\n",
            rt->fl.iif, rt->idev->dev->ifindex);
        return NOTIFY_DONE;
    }

    route.family = rt->dst.ops->family;

    /* 只处理IPv4路由添加删除事件 */
    if (route.family == AF_INET) {

        /* 只关注转发路由 */
        fl_type = nfp_get_fl_type(&rt->fl);
        if (NFP_FL_INPUT != fl_type)
        {
            NFP_ROUTE_DEBUG("Not forward route\n");

            return NOTIFY_DONE;
        }

        /*只处理单播路由*/
        if(rt->fl.fl4_src == 0 ||
            inet_addr_type(&init_net,rt->fl.fl4_dst) != RTN_UNICAST) {
             NFP_ROUTE_DEBUG("!RTN_UNICAST\n");
            return NOTIFY_DONE;
        }

        /*源和目的IP相同的路由缓存规则不处理*/
        if(!((rt->fl.fl4_src) ^ (rt->fl.fl4_dst)))
        {
            return NOTIFY_DONE;
        }

        ret = nfp_route_setup_entry(&route, rt, events);
        if (ret < 0)
            goto out;
    }
    else
        goto out;

    switch(events) {
        case RT_CACHE_NEW:
#if (defined(CONFIG_TBSMOD_DUMMYPORT) && !defined(CONFIG_TBS_NFP))
            if((((route.oif)->itf_flags & (ITF_DUMMYPORT|ITF_TYPE_WAN)) ==
                (ITF_DUMMYPORT|ITF_TYPE_WAN)) && (NULL == route.phyoif)) {
                NFP_ROUTE_DEBUG("Get phyoif failed, oif: %s\n", (route.oif)->ifname);
                break;
            }
#endif

            ret = nfp_route_add(&route);
			/* TBS_TAG: add by baiyonghui 2011-11-29 for */
#ifdef CONFIG_TBS_NFP_FIB
            if(NFP_NO_ERR == ret)
			    rt->tbs_nfp = true;
#endif
            break;
        case RT_CACHE_DEL:
            nfp_route_del(&route);

#ifdef CONFIG_TBS_NFP_FIB
			rt->tbs_nfp = false;
#endif
            break;
        default:
            NFP_ROUTE_DEBUG("route event: %lu, nothing to do\n", events);
    }

out:

    /*释放interface、neigh引用计数*/
    if(route.oif)
        nfp_interface_put(route.oif);
    if(route.phyoif)
        nfp_interface_put(route.phyoif);

    NFP_ADAPTER_OUT_FUNC;

    return NOTIFY_DONE;
}

static struct notifier_block nfp_ipv4_route_notifier = {
	.notifier_call	= nfp_ipv4_route_event,
};

/*=========================================================================
 Function:		    int nfp_route_init(void)

 Description:		route子模块初始化,创建hash表并初始化
 Data Accessed:

 Data Updated:
 Input:			    无
 Output:			无
 Return:			0:成功

 Others:
=========================================================================*/
int nfp_route_init(void)
{
	int ret = 0;

    NFP_ADAPTER_INTO_FUNC;

    /* 注册route通知链 */
    ret = ipv4_route_register_notifier(&nfp_ipv4_route_notifier);
	if (ret < 0) {
		NFP_ADAPTER_ERROR("nfp_route_init: cannot register notifier.\n");
		return ret;
	}

#ifdef CONFIG_TBS_NFP_FIB
	tbs_nfp_hook_fib_age = nfp_route_age;
#endif

    NFP_ADAPTER_OUT_FUNC;

	return ret;

}

/*=========================================================================
 Function:		    int nfp_route_exit(void)

 Description:		route子模块退出，释放内存资源
 Data Accessed:

 Data Updated:
 Input:			    无
 Output:			无
 Return:			0:成功

 Others:
=========================================================================*/

int nfp_route_exit(void)
{

#ifdef CONFIG_TBS_NFP_FIB
    tbs_nfp_hook_fib_age = NULL;
#endif

    /*注销ipv4 route通知链*/
    if(ipv4_route_unregister_notifier(&nfp_ipv4_route_notifier))
        NFP_ADAPTER_ERROR("nfp_route_exit: ipv4_route_unregister_notifier fail.\n");

    nfp_route_proc_exit();

    return 0;
}

