/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_mng.c
* 文件描述 : tbs加速器管理接口
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

#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE)
/*in kernel dir*/
#include <linux/../../net/bridge/br_private.h>
#endif

/*in vendor dir*/
#include "../nfp_adapter/nfp_neighbour.h"
#include "../nfp_adapter/nfp_interface.h"
#include "../nfp_adapter/nfp_route.h"
#include "../nfp_adapter/nfp_conntrack.h"
#include "../nfp_adapter/nfp_adapter.h"
#include "../nfp_adapter/nfp_bridge.h"
#include "autoconf.h"

#include "tbs_nfp.h"
#include "tbs_nfp_itf.h"

#ifdef TBS_NFP_BRIDGE
#include "tbs_nfp_bridge.h"
#endif

#ifdef TBS_NFP_FIB
#include "tbs_nfp_fib.h"
#include "tbs_nfp_arp.h"
#endif

#ifdef TBS_NFP_CT
#include "tbs_nfp_ct.h"
#endif


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/


/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/


#ifdef TBS_NFP_BRIDGE
/*=========================================================================
 Function:static int fpp_bridge_parser(unsigned short fcode, PL2Bridge_entry pEntry, unsigned short *rlen, unsigned short *rdata)

 Description:       加速器规则状态添加/删除/获取
 Data Accessed:

 Data Updated:
 Input:         fcode
                pEntry
 Output:        rlen
                rdata
 Return:        0:成功
                -1或其它:失败

 Others:
=========================================================================*/
static int tbs_nfp_bridge_parser(unsigned short fcode, const struct tL2Bridge_entry *pEntry)
{
    int ret = 0;

    switch(fcode){
        case NFP_OPT_STAT:
            ret = tbs_nfp_bridge_rule_age((u8 *)&pEntry->sa, (u8 *)&pEntry->da, pEntry->input_ifindex);
            if(ret < 0){
                TBS_NFP_ERROR("Call nfp_bridge_rule_age faild\n"
                    "\tsrc_mac:%02x:%02x:%02x:%02x:%02x:%02x dst_mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
                    pEntry->sa[0], pEntry->sa[1], pEntry->sa[2], pEntry->sa[3], pEntry->sa[4], pEntry->sa[5],
                    pEntry->da[0], pEntry->da[1], pEntry->da[2], pEntry->da[3], pEntry->da[4], pEntry->da[5]);
            }
            if(ret > 0)
                ret = NFP_RULE_ACTIVE;
            else
                ret = NFP_RULE_TIMEOUT;

            break;

        case NFP_OPT_ADD:
            ret = tbs_nfp_bridge_rule_add((u8 *)&pEntry->sa, (u8 *)&pEntry->da, pEntry->input_ifindex, pEntry->output_ifindex);
            if(ret){
                TBS_NFP_ERROR("Call nfp_bridge_rule_add faild:\n"
                    "\tsrc_mac:%02x:%02x:%02x:%02x:%02x:%02x dst_mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
                    pEntry->sa[0], pEntry->sa[1], pEntry->sa[2], pEntry->sa[3], pEntry->sa[4], pEntry->sa[5],
                    pEntry->da[0], pEntry->da[1], pEntry->da[2], pEntry->da[3], pEntry->da[4], pEntry->da[5]);            }
            break;

        case NFP_OPT_DELETE:
            ret = tbs_nfp_bridge_rule_delete((u8 *)&pEntry->sa, (u8 *)&pEntry->da, pEntry->input_ifindex);
            if(ret){
                TBS_NFP_ERROR("Call nfp_bridge_rule_del faild\n"
                    "\tsrc_mac:%02x:%02x:%02x:%02x:%02x:%02x dst_mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
                    pEntry->sa[0], pEntry->sa[1], pEntry->sa[2], pEntry->sa[3], pEntry->sa[4], pEntry->sa[5],
                    pEntry->da[0], pEntry->da[1], pEntry->da[2], pEntry->da[3], pEntry->da[4], pEntry->da[5]);
            }
            break;

        default:
            break;
    }

    return ret;
}
#endif /*TBS_NFP_BRIDGE*/


#ifdef TBS_NFP_FIB
/*=========================================================================
 Function:          static int tbs_nfp_ipv4_arp_process(unsigned short cmd, const struct neighbour_entry *nb)

 Description:       ipv4 arp 规则增加/删除接口
 Data Accessed:

 Data Updated:
 Input:             无
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_ipv4_arp_process(unsigned short cmd, const struct neighbour_entry *nb)
{
    int ret = 0;
    struct interface_entry * itf = NULL;

    TBS_NFP_INTO_FUNC;

    switch (cmd)
    {
        case NFP_OPT_ADD:
            tbs_nfp_arp_add(nb->family,(u8 *)(nb->ip_addr),(u8 *)nb->mac_addr);
            break;

        case NFP_OPT_DELETE:
            tbs_nfp_arp_delete(nb->family,(u8 *)(nb->ip_addr));
            break;

        case NFP_OPT_STAT:
            ret = tbs_nfp_arp_age(nb->family, (u8 *)(nb->ip_addr));
            if(ret > 0)
                ret = NFP_RULE_ACTIVE;
            else
                ret = NFP_RULE_TIMEOUT;

            break;

        default:
            TBS_NFP_DEBUG("Unknown cmd[%d]\n", cmd);
            ret = ERR_NFP_UNKNOWN_OPTION;

            break;
    }

    if(itf)
        nfp_interface_put(itf);

    return ret;
}

/*=========================================================================
 Function:          static int tbs_nfp_ipv6_ndisc_process(const struct neighbour_entry *nb)

 Description:       ipv6 neighbourdisc 规则配置接口
 Data Accessed:

 Data Updated:
 Input:             无
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_ipv6_ndisc_process(unsigned short cmd, const struct neighbour_entry *nb)
{
    int ret = 0;

    TBS_NFP_INTO_FUNC;

    if(!nb) {
        printk("error: assert(neighbour_entry *nb)\n");
        return -1;
    }

    switch (cmd)
    {
        case NFP_OPT_ADD:

            if (ret){
                TBS_NFP_DEBUG("Error %d while add ipv4 neigh faild\n", ret);
            }

            break;

        case NFP_OPT_DELETE:

            break;

        case NFP_OPT_STAT:
            ret = NFP_RULE_ACTIVE;
            break;

        default:
            ret = ERR_NFP_UNKNOWN_OPTION;

            break;
    }

    return ret;
}


/*=========================================================================
 Function:          int fpp_neigbour_parser(unsigned short cmd, const struct neighbour_entry *nb)

 Description:       neighbour 加速器规则配置接口
 Data Accessed:

 Data Updated:
 Input:             无
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_neigbour_parser(unsigned short cmd, const struct neighbour_entry *nb)
{
    int ret = 0;

    if(!nb) {
        printk("error: assert(neighbour_entry *nb)\n");
        return -1;
    }
    TBS_NFP_INTO_FUNC;

    switch (nb->family)
    {
        case AF_INET:
            ret = tbs_nfp_ipv4_arp_process(cmd, nb);

            break;

        case AF_INET6:
            ret = tbs_nfp_ipv6_ndisc_process(cmd, nb);

            break;

        default:
            TBS_NFP_DEBUG("Unknown family[%d]\n", nb->family);
            break;
    }

    return ret;
}


/*=========================================================================
 Function:          static int tbs_nfp_ipv4_route_process(unsigned short cmd, const struct rt_entry *nb)

 Description:       ipv4 route 规则增加/删除接口
 Data Accessed:

 Data Updated:
 Input:             无
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_ipv4_route_process(unsigned short cmd, const struct rt_entry *rt)
{
    int ret = -1;
    int ifindex = 0;

    TBS_NFP_INTO_FUNC;

    TBS_NFP_DEBUG("cmd:%u, family:%u, sip:%pI4, dip:%pI4, gtw:%pI4\n",\
                  cmd, rt->family, &rt->fl.fl4_src,  &rt->fl.fl4_dst, &rt->gateway);

    switch (cmd)
    {
        case NFP_OPT_ADD:
            if(NULL == rt->oif)
            {
                TBS_NFP_ERROR("route output interface is NULL\n");
                goto err;
            }

            ifindex = rt->oif->ifindex;
            TBS_NFP_DEBUG("ifindex = %d\n", ifindex);
            ret = tbs_nfp_fib_add(rt->family, (u8 *)&(rt->fl.fl4_src),  (u8 *)&(rt->fl.fl4_dst), (u8 *)&(rt->gateway), ifindex);
            if (ret){
                TBS_NFP_DEBUG("Error %d while add fib faild, dip: 0x%08x, oif: %d\n", ret, rt->fl.fl4_dst, ifindex);
            }

            break;

        case NFP_OPT_DELETE:
            ret = tbs_nfp_fib_delete(rt->family, (u8 *)&(rt->fl.fl4_src),  (u8 *)&(rt->fl.fl4_dst));
            if (ret){
                TBS_NFP_DEBUG("Error %d while delete fib faild\n", ret);
                goto err;
            }
            break;

        case NFP_OPT_STAT:
            ret = tbs_nfp_fib_age(rt->family, (u8 *)&(rt->fl.fl4_src),  (u8 *)&(rt->fl.fl4_dst));
            if(ret > 0)
                ret = NFP_RULE_ACTIVE;
            else
                ret = NFP_RULE_TIMEOUT;

            TBS_NFP_DEBUG("tbs_nfp_fib_stat: %s\n",
                (NFP_RULE_ACTIVE == ret)?"NFP_RULE_ACTIVE":"NFP_RULE_TIMEOUT");

            break;

        default:
            TBS_NFP_DEBUG("Unknown cmd[%d]\n", cmd);
            ret = ERR_NFP_UNKNOWN_OPTION;

            break;
    }

err:

    return ret;
}


/*=========================================================================
 Function:          static int fpp_ipv6_route_process(unsigned short cmd, const struct rt_entry *nb)

 Description:       ipv6 route 规则增加/删除接口
 Data Accessed:

 Data Updated:
 Input:             无
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_ipv6_route_process(unsigned short cmd, const struct rt_entry *rt)
{
    int ret = -1;

    /*
    网关地址没有
    从interface模块中获取接口名称
    */
    switch (cmd)
    {
        case NFP_OPT_ADD:
            break;

        case NFP_OPT_DELETE:
            break;

        case NFP_OPT_STAT:
            ret = NFP_RULE_ACTIVE;
            break;

        default:
            TBS_NFP_DEBUG("Unknown cmd[%d]\n", cmd);
            ret = ERR_NFP_UNKNOWN_OPTION;

            break;
    }

    return ret;
}


/*=========================================================================
 Function:          int tbs_nfp_route_parser(unsigned short cmd, const struct neighbour_entry *nb)

 Description:       route cache 加速器规则配置接口
 Data Accessed:

 Data Updated:
 Input:             无
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_route_parser(unsigned short cmd, const struct rt_entry *rt)
{
    int ret = -1;

    if(!rt) {
        printk("error: assert(rt_entry *rt)\n");
        return ret;
    }

    TBS_NFP_INTO_FUNC;

    switch (rt->family)
    {
        case AF_INET:
            ret = tbs_nfp_ipv4_route_process(cmd, rt);

            break;

        case AF_INET6:
            ret = tbs_nfp_ipv6_route_process(cmd, rt);

            break;

        default:
            TBS_NFP_DEBUG("Unknown family[%d]\n", rt->family);
            break;
    }

    return ret;
}
#endif /*TBS_NFP_FIB*/


#ifdef TBS_NFP_CT

/*=========================================================================
 Function:          static int tbs_nfp_ipv4_ct_process(unsigned short cmd, const struct rt_entry *nb)

 Description:       ipv6 route 规则增加/删除接口
 Data Accessed:

 Data Updated:
 Input:             无
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_ipv4_ct_process(unsigned short cmd, const struct conntrack_entry_tab *ct)
{
    int ret = 0;

    u32 original_sip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip;
    u32 original_dip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip;
    u16 original_sport = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
    u16 original_dport = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;

    u32 reply_sip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip;
    u32 reply_dip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip;
    u16 reply_sport = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all;
    u16 reply_dport = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all;

    u8 protocol = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.protonum;

    TBS_NFP_INTO_FUNC;

    switch (cmd)
    {
        case NFP_OPT_ADD:
            ret = tbs_nfp_ct_add(AF_INET, (u8 *)&original_sip, (u8 *)&original_dip, original_sport, original_dport,
                protocol, (u8 *)&reply_sip, (u8 *)&reply_dip, reply_sport , reply_dport);
            if (ret){
                TBS_NFP_DEBUG("Error %d while add contrack faild\n", ret);
            }

            break;

        case NFP_OPT_DELETE:
            tbs_nfp_ct_delete(AF_INET, (u8 *)&original_sip, (u8 *)&original_dip, original_sport, original_dport, protocol);
            break;

        case NFP_OPT_STAT:
            if(tbs_nfp_ct_age(AF_INET, (u8 *)&original_sip, (u8 *)&original_dip, original_sport, original_dport, protocol) > 0)
            {
                ret = NFP_RULE_ACTIVE;
            }
            else
            {
                ret = NFP_RULE_TIMEOUT;
            }

            break;

        default:
            TBS_NFP_DEBUG("Unknown cmd[%d]\n", cmd);
            ret = ERR_NFP_UNKNOWN_OPTION;

            break;
    }

    return ret;
}

/*=========================================================================
 Function:          static int tbs_nfp_ipv6_ct_process(unsigned short cmd, const struct rt_entry *nb)

 Description:       ipv6 route 规则增加/删除接口
 Data Accessed:

 Data Updated:
 Input:             无
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_ipv6_ct_process(unsigned short cmd, const struct conntrack_entry_tab *ct)
{
    return 0;
}


/*=========================================================================
 Function:          static int fpp_conntrack_parser(unsigned short cmd, const struct nfp_entry_tab *nb)

 Description:       conntrack 加速器规则配置接口
 Data Accessed:

 Data Updated:
 Input:             无
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_conntrack_parser(unsigned short cmd, const struct conntrack_entry_tab *ct)
{
    int ret = 0;

    int family = 0;
    if(!ct) {
        printk("error: assert(conntrack_entry_tab *ct)\n");
        return -1;
    }

    TBS_NFP_INTO_FUNC;

    family = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;
    switch (family)
    {
        case AF_INET:
            ret = tbs_nfp_ipv4_ct_process(cmd, ct);

            break;

        case AF_INET6:
            ret = tbs_nfp_ipv6_ct_process(cmd, ct);

            break;

        default:
            TBS_NFP_DEBUG("Unknown family[%d]\n", family);
            break;
    }

    return ret;
}
#endif /*TBS_NFP_CT*/


#ifdef TBS_NFP_BRIDGE
/*=========================================================================
 Function:          static inline int tbs_nfp_brctl_process(unsigned short option, const struct interface_entry *itf)

 Description:       brctl 命令处理函数
 Data Accessed:

 Data Updated:
 Input:             unsigned short option  操作字
                    const struct interface_entry *itf   br_port结构体指针
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static inline int tbs_nfp_brctl_process(unsigned short option, const struct interface_entry *itf)
{
    int ret = NFP_NO_ERR;

    if(NULL == itf)
    {
        TBS_NFP_ERROR("itf is null\n");
        return ERR_NFP_OTHER;
    }

    if(itf->itf_flags & (ITF_BRIDGE | ITF_DUMMYPORT | ITF_PPPOE))
    {
        TBS_NFP_ERROR("interface %s Unsupport brctl option\n", itf->ifname);
        return ERR_NFP_OTHER;
    }

    /* Add/rem port_if interface to bridge */
    switch(option)
    {
        case NFP_OPT_BRPORT_ADD:
            ret = tbs_nfp_if_br_port_add(itf->br_index, itf->ifindex);
            if(ret)
                TBS_NFP_DEBUG("call nfp_if_to_bridge_add faild(%d)\n", ret);

            break;

        case NFP_OPT_BRPORT_DELETE:
            ret = tbs_nfp_if_br_port_remove(itf->ifindex);
            if(ret)
                TBS_NFP_DEBUG("call nfp_if_to_bridge_del faild(%d)\n", ret);

            break;

        default:
            TBS_NFP_DEBUG("unknown brctl option(%d)\n", option);
            ret = ERR_NFP_UNKNOWN_OPTION;
            break;
    }

    return ret;
}
#endif /*TBS_NFP_BRIDGE*/


/*=========================================================================
 Function:          static int tbs_nfp_eth_process(unsigned short cmd, const struct interface_entry *itf)

 Description:       以太网物理接口处理
 Data Accessed:

 Data Updated:
 Input:             unsigned short cmd  命令字
                    const struct interface_entry *itf
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_eth_process(unsigned short cmd, const struct interface_entry *itf)
{
    short ret= 0;

    switch (cmd)
    {
        case NFP_OPT_ADD:
            ret = tbs_nfp_if_phys_add(itf->ifindex, itf->mtu, itf->macaddr, itf->ifname);
            if(ret){
                TBS_NFP_ERROR("Can't add ethernet interface, eth_if=%s(%d)\n",
                itf->ifname, itf->ifindex);
                break;
            }

            break;
        case NFP_OPT_DELETE:
            ret = tbs_nfp_if_delete(itf->ifindex);
            if(ret){
                TBS_NFP_ERROR("Can't delete ethernet interface, eth_if=%s(%d)\n",
                itf->ifname, itf->ifindex);
                break;
            }

            break;

        case NFP_OPT_CHANGE_ADDR:
            ret = tbs_nfp_if_mac_change(itf->ifindex, itf->macaddr);
            break;

        default:
            TBS_NFP_DEBUG("Unknown cmd[%d], eth_if=%s(%d)\n", cmd,
                itf->ifname, itf->ifindex);
            ret = ERR_NFP_UNKNOWN_OPTION;

            break;
    }

    return ret;
}

#ifdef TBS_NFP_WLAN
/*=========================================================================
 Function:          static int tbs_nfp_wlan_process(unsigned short cmd, const struct interface_entry *itf)

 Description:       无线接口处理
 Data Accessed:

 Data Updated:
 Input:             unsigned short cmd  命令字
                    const struct interface_entry *itf
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_wlan_process(unsigned short cmd, const struct interface_entry *itf)
{
    short ret= 0;

    switch (cmd)
    {
        case NFP_OPT_ADD:
            ret = tbs_nfp_if_wlan_add(itf->ifindex, itf->mtu, itf->macaddr, itf->ifname);
            if(ret){
                TBS_NFP_ERROR("Can't add wlan interface, wlan_if=%s(%d)\n",
                itf->ifname, itf->ifindex);
                break;
            }

            break;
        case NFP_OPT_DELETE:
            ret = tbs_nfp_if_delete(itf->ifindex);
            if(ret){
                TBS_NFP_ERROR("Can't delete wlan interface, wlan_if=%s(%d)\n",
                itf->ifname, itf->ifindex);
                break;
            }

            break;

        default:
            TBS_NFP_DEBUG("Unknown cmd[%d], wlan_if=%s(%d)\n", cmd,
                itf->ifname, itf->ifindex);
            ret = ERR_NFP_UNKNOWN_OPTION;

            break;
    }

    return ret;
}
#endif /*TBS_NFP_WLAN*/


#ifdef TBS_NFP_BRIDGE
/*=========================================================================
 Function:          static int tbs_nfp_br_process(unsigned short cmd, const struct interface_entry *itf)

 Description:       vlan interface 配置接口
 Data Accessed:

 Data Updated:
 Input:             unsigned short cmd  命令字
                    const struct interface_entry *itf
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_br_process(unsigned short cmd, const struct interface_entry *itf)
{
    short ret = 0;

    switch(cmd)
    {
        case NFP_OPT_ADD:
            ret = tbs_nfp_if_bridge_add(itf->ifindex, itf->mtu, itf->macaddr, itf->ifname);
            if (ret) {
                TBS_NFP_ERROR("Can't register bridge interface bridge_if=%s(%d)\n",
                        itf->ifname, itf->ifindex);
            }

            break;
        case NFP_OPT_DELETE:
            ret = tbs_nfp_if_delete(itf->ifindex);
            if (ret) {
                TBS_NFP_ERROR("Can't delete bridge interface bridge_if=%s(%d)\n",
                        itf->ifname, itf->ifindex);
                return ret;
            }

            break;

        default:
            TBS_NFP_ERROR("Unknown cmd:%d bridge interface bridge_if=%s(%d)\n",
                                    cmd, itf->ifname, itf->ifindex);
            ret = ERR_NFP_UNKNOWN_OPTION;
            break;
    }

    return ret;
}
#endif /*TBS_NFP_BRIDGE*/


#ifdef TBS_NFP_VLAN
/*=========================================================================
 Function:          static int tbs_nfp_vlan_process(unsigned short cmd, const struct interface_entry *itf)

 Description:       vlan interface 配置接口
 Data Accessed:

 Data Updated:
 Input:             unsigned short cmd  命令字
                    const struct interface_entry *itf
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_vlan_process(unsigned short cmd, const struct interface_entry *itf)
{
    short ret= 0;

    switch (cmd)
    {
        case NFP_OPT_ADD:
            ret = tbs_nfp_if_vlan_add(itf->ifindex, itf->mtu, itf->phys_ifindex,
                itf->vlan_id, itf->macaddr, itf->ifname);
            if(ret){
                TBS_NFP_ERROR("Can't add vlan interface, vlan_if=%s(%d)\n",
                itf->ifname, itf->ifindex);
                break;
            }

            break;
        case NFP_OPT_DELETE:
            ret = tbs_nfp_if_delete(itf->ifindex);
            if(ret){
                TBS_NFP_ERROR("Can't delete vlan interface, vlan_if=%s(%d)\n",
                itf->ifname, itf->ifindex);
                break;
            }

            break;

        default:
            TBS_NFP_DEBUG("Unknown cmd[%d], vlan_if=%s(%d)\n", cmd,
                itf->ifname, itf->ifindex);
            ret = ERR_NFP_UNKNOWN_OPTION;

            break;
    }

    return ret;
}
#endif /*TBS_NFP_VLAN*/


#ifdef TBS_NFP_PPP
/*=========================================================================
 Function:          static int tbs_nfp_pppoe_process(unsigned short cmd, const struct interface_entry *itf)

 Description:       pppoe interface 配置接口
 Data Accessed:

 Data Updated:
 Input:             unsigned short cmd  命令字
                    const struct interface_entry *itf
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_pppoe_process(unsigned short cmd, const struct interface_entry *itf)
{
    int ret = 0;
    int phys_ifindex = 0;

    switch (cmd)
    {
        case NFP_OPT_ADD:
            phys_ifindex = itf->phys_ifindex;

            ret = tbs_nfp_if_pppoe_add(itf->ifindex, itf->phys_ifindex, itf->mtu,
                itf->session_id, itf->dst_macaddr, itf->ifname);
            if(ret){
                TBS_NFP_ERROR("Can't add ppp interface, ppp_if=%s(%d)\n",
                itf->ifname, itf->ifindex);
                break;
            }

            break;
        case NFP_OPT_DELETE:
            ret = tbs_nfp_if_delete(itf->ifindex);

            printk("pppif delete: phys_ifindex is %d, itf->ifindex is %d\n,mac:%x:%x:%x:%x:%x:%x",phys_ifindex,itf->ifindex,\
                itf->dst_macaddr[0],itf->dst_macaddr[1],itf->dst_macaddr[2],itf->dst_macaddr[3],itf->dst_macaddr[4],itf->dst_macaddr[4]);
            if(ret){
                TBS_NFP_ERROR("Can't delete ppp interface, ppp_if=%s(%d)\n",
                itf->ifname, itf->ifindex);
                break;
            }

            break;

        default:
            TBS_NFP_DEBUG("Unknown cmd[%d], ppp_if=%s(%d)\n", cmd,
                itf->ifname, itf->ifindex);
            ret = ERR_NFP_UNKNOWN_OPTION;
            break;
    }

    return ret;
}
#endif /*TBS_NFP_PPP*/


#ifdef TBS_NFP_DUMMYPORT
/*=========================================================================
 Function:          static int tbs_nfp_dummyport_process(unsigned short cmd, const struct interface_entry *itf)

 Description:       dummyport interface 配置接口
 Data Accessed:

 Data Updated:
 Input:             unsigned short cmd  命令字
                    const struct interface_entry *itf
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_dummyport_process(unsigned short cmd, const struct interface_entry *itf)
{
    short ret = 0;

    switch (cmd)
    {
        case NFP_OPT_ADD:
            ret = tbs_nfp_if_dummyport_add(itf->ifindex, itf->mtu, itf->phys_ifindex,
                itf->macaddr, itf->ifname);
            if(ret){
                TBS_NFP_ERROR("Can't add dummyport interface, dummyport_if=%s(%d), phys_if=%d\n",
                    itf->ifname, itf->ifindex, itf->phys_ifindex);
            }
            break;
        case NFP_OPT_DELETE:
            ret = tbs_nfp_if_delete(itf->ifindex);
            if(ret){
                TBS_NFP_ERROR("Can't delete dummyport interface, dummyport_if=%s(%d)\n",
                itf->ifname, itf->ifindex);
            }
            break;

        case NFP_OPT_DUMMYPORT_BIND:
            ret = tbs_nfp_if_dummyport_bind(itf->ifindex, itf->phys_ifindex);
            if(ret){
                TBS_NFP_ERROR("Can't bind dummyport interface, dummyport_if=%s(%d), phys_if=%d\n",
                itf->ifname, itf->ifindex, itf->phys_ifindex);
            }

            break;
        case NFP_OPT_DUMMYPORT_UNBIND:
            ret = tbs_nfp_if_dummyport_unbind(itf->ifindex);
            if(ret){
                TBS_NFP_ERROR("Can't unbind dummyport interface, dummyport_if=%s(%d), phys_if=%d\n",
                itf->ifname, itf->ifindex, itf->phys_ifindex);
            }

            break;

        default:
            TBS_NFP_DEBUG("Unknown cmd[%d]\n", cmd);
            ret = ERR_NFP_UNKNOWN_OPTION;

            break;
    }

    return ret;
}
#endif /*TBS_NFP_DUMMYPORT*/


#if defined(CONFIG_VNET)
/*=========================================================================
 Function:          static int tbs_nfp_vnet_process(unsigned short cmd, const struct interface_entry *itf)

 Description:       vnet interface 配置接口
 Data Accessed:

 Data Updated:
 Input:             unsigned short cmd  命令字
                    const struct interface_entry *itf
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_vnet_process(unsigned short cmd, const struct interface_entry *itf)
{
    short ret = 0;

    switch (cmd)
    {
        case NFP_OPT_ADD:
            ret = tbs_nfp_if_vnet_add(itf->ifindex, itf->mtu, itf->phys_ifindex,
                itf->macaddr, itf->ifname);
            if(ret){
                TBS_NFP_ERROR("Can't add vnet interface, vnet_if=%s(%d), phys_if=%d\n",
                    itf->ifname, itf->ifindex, itf->phys_ifindex);
            }
            break;
        case NFP_OPT_DELETE:
            ret = tbs_nfp_if_delete(itf->ifindex);
            if(ret){
                TBS_NFP_ERROR("Can't delete vnet interface, vnet_if=%s(%d)\n",
                itf->ifname, itf->ifindex);
            }
            break;

        default:
            TBS_NFP_DEBUG("Unknown cmd[%d]\n", cmd);
            ret = ERR_NFP_UNKNOWN_OPTION;

            break;
    }

    return ret;
}
#endif /* CONFIG_VNET */


/*=========================================================================
 Function:          int tbs_nfp_interface_parser(unsigned short cmd, const struct neighbour_entry *nb)

 Description:       interface 配置接口
 Data Accessed:

 Data Updated:
 Input:             unsigned short cmd  命令字
                    const struct interface_entry *itf   interface结构体指针
 Output:            无
 Return:            0:成功

 Others:
=========================================================================*/
static int tbs_nfp_interface_parser(unsigned short cmd, const struct interface_entry *itf)
{
    int ret = 0;

    if(!itf) {
        printk("error: assert(struct interface_entry *itf)\n");
        return -1;
    }

    /* all interface brctl*/
#ifdef TBS_NFP_BRIDGE
    if(NFP_OPT_BRPORT_ADD == cmd ||
        NFP_OPT_BRPORT_DELETE == cmd)
    {
        return tbs_nfp_brctl_process(cmd, itf);
    }
#endif /*TBS_NFP_BRIDGE*/

    /* mtu change*/
    if(NFP_OPT_CHANGE_MTU == cmd)
        return tbs_nfp_if_mtu_change(itf->ifindex, itf->mtu);

    /*mac addr change*/
    if(NFP_OPT_CHANGE_ADDR == cmd)
        return tbs_nfp_if_mac_change(itf->ifindex, itf->macaddr);

    /*ethernet interface */
    if(itf->itf_flags & (ITF_TBS_ETH | ITF_TBS_NAS | ITF_TBS_PTM))
    {
        ret = tbs_nfp_eth_process(cmd, itf);
    }

#ifdef TBS_NFP_WLAN
    else if(itf->itf_flags & ITF_TBS_WLAN)
    {
        ret = tbs_nfp_wlan_process(cmd, itf);
    }
#endif /*#ifdef TBS_NFP_WLAN*/

#ifdef TBS_NFP_BRIDGE
    else if(itf->itf_flags & ITF_BRIDGE)
    {
        ret = tbs_nfp_br_process(cmd,itf);
    }
#endif /*#TBS_NFP_BRIDGE*/

#ifdef TBS_NFP_VLAN
    else if(itf->itf_flags & ITF_VLAN)
    {
        ret = tbs_nfp_vlan_process(cmd, itf);
    }
#endif /*TBS_NFP_VLAN*/

#ifdef TBS_NFP_PPP
    else if(itf->itf_flags & ITF_PPPOE)
    {
        ret = tbs_nfp_pppoe_process(cmd, itf);
    }
#endif /*TBS_NFP_PPP*/

#ifdef TBS_NFP_DUMMYPORT
    else if(itf->itf_flags & ITF_DUMMYPORT)
    {
        ret = tbs_nfp_dummyport_process(cmd, itf);
    }
#endif /*TBS_NFP_DUMMYPORT*/

#if defined(CONFIG_VNET)
    else if(itf->itf_flags & ITF_TBS_VNET)
    {
        ret = tbs_nfp_vnet_process(cmd, itf);
    }
#endif /*CONFIG_VNET*/
    else
    {
        TBS_NFP_ERROR("ifname = %s, itf_flags = %x, nothing todo\n",
            itf->ifname, itf->itf_flags);
    }

    return ret;
}


/*=========================================================================
 Function:		static int tbs_nfp_reset()

  Description:       清除加速器缓存所有规则
  Data Accessed:

  Data Updated:
  Input:             无
  Output:            无
  Return:            0: 成功；其他: 失败

  Others:
 =========================================================================*/
static int tbs_nfp_reset(void)
{
    tbs_nfp_rule_reset();
    return 0;
}


/*=========================================================================
 Function:		    int tbs_nfp_adapter_parser(unsigned short cmd, const struct nfp_adapter_cmd *entry)

 Description:		nfp命令解析
 Data Accessed:

 Data Updated:
 Input:			    unsigned short cmd  命令字
                    const struct nfp_adapter_cmd   命令内容
 Output:			无
 Return:			0:成功

 Others:
=========================================================================*/
int tbs_nfp_cmd_parser(unsigned short cmd, const struct nfp_adapter_cmd *entry)
{
    int ret = ERR_NFP_OTHER;

    switch(cmd)
    {
        case NFP_CMD_ITF:
            ret = tbs_nfp_interface_parser(entry->option, (struct interface_entry *)entry->rule_entry);
            break;

#ifdef TBS_NFP_BRIDGE
        case NFP_CMD_BRIDGE:
            ret = tbs_nfp_bridge_parser(entry->option, (struct tL2Bridge_entry *)entry->rule_entry);
            break;
#endif /*TBS_NFP_BRIDGE*/

#ifdef TBS_NFP_FIB
        case NFP_CMD_ARP:
            ret = tbs_nfp_neigbour_parser(entry->option, (struct neighbour_entry *)entry->rule_entry);
            break;
        case NFP_CMD_FIB:
            ret = tbs_nfp_route_parser(entry->option, (struct rt_entry *)entry->rule_entry);
            break;
#endif  /*TBS_NFP_FIB*/

#ifdef TBS_NFP_CT
        case NFP_CMD_CT:
            ret = tbs_nfp_conntrack_parser(entry->option, (struct conntrack_entry_tab *)entry->rule_entry);
            break;
#endif /*TBS_NFP_CT*/

        case NFP_CMD_RESET:
            ret = tbs_nfp_reset();

        default:
            //TBS_NFP_DEBUG("Unknown NFP_CMD[0x%02x]\n", cmd);
            ret = ERR_NFP_UNKNOWN_CMD;
            break;
    }

    return ret;
}


/*=========================================================================
 Function:		    int tbs_nfp_mng_init(void)

 Description:		配置接口初始化，并同步转发规则
 Data Accessed:

 Data Updated:
 Input:			    无
 Output:			无
 Return:			0:成功; 其实:失败

 Others:
=========================================================================*/
int tbs_nfp_mng_init(void)
{
    int ret;

    ret = nfp_cmd_parser_register(tbs_nfp_cmd_parser);
    if(ret)
        goto err;

    /*重新下发所有转发规则*/
    nfp_rule_flush();

    return TBS_NFP_OK;

err:
    nfp_cmd_parser_unregister(tbs_nfp_cmd_parser);

    return TBS_NFP_ERR;
}


/*=========================================================================
 Function:		    int tbs_nfp_mng_init(void)

 Description:		注销规则配置接口
 Data Accessed:

 Data Updated:
 Input:			    无
 Output:			无
 Return:			无

 Others:
=========================================================================*/
void tbs_nfp_mng_exit(void)
{
    nfp_cmd_parser_unregister(tbs_nfp_cmd_parser);

    return;
}

