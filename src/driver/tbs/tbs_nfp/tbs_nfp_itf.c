/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_itf.c
* 文件描述 : TBS加速器interface处理模块
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

#include <linux/if_pppox.h>
#include <linux/if_vlan.h>
#include <linux/if.h>
#include <linux/jhash.h>
#include <../net/8021q/vlan.h>

#if (defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE))
#include <linux/if_bridge.h>
#endif

#ifdef TBS_NFP_DUMMYPORT
/*in build dir*/
#include <driver/tbs/dummyport/dummyport.h>
#endif  /* TBS_NFP_DUMMYPORT */

#if defined(CONFIG_VNET)
#include <linux/if_vnet.h>
#endif  /* CONFIG_VNET */

/*in root dir*/
#include "tbs_nfp.h"
#include "tbs_nfp_itf.h"

#ifdef TBS_NFP_BRIDGE
#include "tbs_nfp_bridge.h"
#endif


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/

/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
/*interface map*/
TBS_NFP_IF_MAP *g_if_map[TBS_NFP_ITF_HASH_SIZE];


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:      inline static TBS_NFP_IF_MAP* tbs_nfp_if_map_alloc(int if_idx, int mtu,
    const char* mac, const char *if_name)

 Description:       if_map节点分配，并初始化
 Data Accessed:     无
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    接口mtu值
                    const char* mac:   mac地址
                    const char *if_name:    接口名称

 Output:            无
 Return:            ifmap指针
 Others:
=========================================================================*/
inline static TBS_NFP_IF_MAP* tbs_nfp_if_map_alloc(int if_idx, int mtu,
    const char* mac, const char *if_name)
{
    TBS_NFP_IF_MAP *ifmap = kmalloc(sizeof(TBS_NFP_IF_MAP), GFP_ATOMIC);
    if(NULL == ifmap)
        return NULL;

    memset(ifmap, 0, sizeof(TBS_NFP_IF_MAP));
    ifmap->ifidx = if_idx;
    ifmap->mtu = mtu;
    if(mac)
        memcpy(ifmap->mac, mac, sizeof(ifmap->mac));

    strncpy(ifmap->name, if_name, sizeof(ifmap->name)-1);

    return ifmap;
}



/*=========================================================================
 Function:      inline void tbs_nfp_if_map_free( TBS_NFP_IF_MAP *if_map)

 Description:       释放if_map节点
 Data Accessed:     无
 Data Updated:
 Input:             无
 Output:            无
 Return:            ifmap指针
 Others:
=========================================================================*/
inline void tbs_nfp_if_map_free( TBS_NFP_IF_MAP *if_map)
{
    if(NULL == if_map)
        return;

    if(if_map->dev)
        dev_put(if_map->dev);

    if_map->dev = NULL;

    kfree(if_map);
}


/*=========================================================================
 Function:      inline void tbs_nfp_if_print(const TBS_NFP_IF_MAP *if_map)

 Description:       打印指定interface接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             const TBS_NFP_IF_MAP *if_map:   interface map指针
 Output:            无
 Return:            无
 Others:
=========================================================================*/
inline void tbs_nfp_if_print(const TBS_NFP_IF_MAP *if_map)
{
    const unsigned char *mac_addr = NULL;
    const TBS_NFP_IF_MAP *virt_ifmap = NULL;

    if(NULL == if_map)
        return;

#ifdef TBS_NFP_PPP
    mac_addr = (if_map->flags & TBS_NFP_F_MAP_PPPOE)?if_map->remote_mac:if_map->mac;
#else
    mac_addr = if_map->mac;
#endif

    printk("%d\t%-16s%d\t\t%d\t\t%02x:%02x:%02x:%02x:%02x:%02x\t%d\t0x%08x\n",
        if_map->ifidx, if_map->name,
        (NULL == if_map->parent_if)?if_map->ifidx:if_map->parent_if->ifidx,
        if_map->bridge_if,
        mac_addr[0], mac_addr[1],
        mac_addr[2], mac_addr[3],
        mac_addr[4], mac_addr[5], if_map->mtu, if_map->flags);

    /*打印派生接口信息*/
    if(if_map->virt_if)
    {
        printk("\tvirt_if: %s", if_map->virt_if->name);
        virt_ifmap = if_map->virt_if->virt_next;

        while(virt_ifmap)
        {
            printk("\t%s", virt_ifmap->name);
            virt_ifmap = virt_ifmap->virt_next;
        }
        printk("\n");
    }
}


/*=========================================================================
 Function:      inline void tbs_nfp_if_print(const TBS_NFP_IF_MAP *if_map)

 Description:       打印hash链表中所有interface接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             无
 Output:            无
 Return:            无
 Others:
=========================================================================*/
void tbs_nfp_if_dump(void)
{
    int i = 0;
    TBS_NFP_IF_MAP *ifmap = NULL;

    printk("ifindex\tifname\t\tphys_ifindex\tbr_ifindex\tmacaddr\t\t\tmtu\titf_flags\n");

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);

    for(; i < TBS_NFP_ITF_HASH_SIZE; i++)
    {
        ifmap = g_if_map[i];

        while(ifmap)
        {
            tbs_nfp_if_print(ifmap);
            ifmap = ifmap->next_map;
        }
    }
    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

    return;
}


/*=========================================================================
 Function:      inline TBS_NFP_IF_MAP *tbs_nfp_ifmap_get(int if_idx)

 Description:       通过if_idx，获取ifmap指针
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx      ifindex
 Output:            无
 Return:            ifmap指针
 Others:
=========================================================================*/
inline TBS_NFP_IF_MAP *tbs_nfp_ifmap_get(int if_idx)
{
    TBS_NFP_IF_MAP *ifmap = g_if_map[tbs_nfp_itf_hash(if_idx)];

    while (ifmap) {
        if (ifmap->ifidx == if_idx){
            return ifmap;
        }

        ifmap = ifmap->next_map;
    }

    return NULL;
}


#ifdef TBS_NFP_VLAN
/*=========================================================================
 Function:      inline TBS_NFP_IF_MAP *tbs_nfp_vlan_ifmap_get(int if_idx)

 Description:       通过物理接口和vlan id获取vlan接口的节点指针
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int parent_if_idx:  物理接口index
                    unsigned short vlan_id: vlan id
 Output:            无
 Return:            vlan ifmap指针
 Others:
=========================================================================*/
inline TBS_NFP_IF_MAP *tbs_nfp_vlan_ifmap_get(int parent_if_idx, unsigned short vlan_id)
{
    TBS_NFP_IF_MAP *ifmap_list, *virt_ifmap = NULL;

    if(vlan_id >= 4096)
        return NULL;

    /*获取hash链表，并遍历链表，找到parent_if_idx节点*/
    ifmap_list = g_if_map[tbs_nfp_itf_hash(parent_if_idx)];
    while(ifmap_list)
    {
        if(ifmap_list->ifidx == parent_if_idx)
        {
            /*检查第一个虚接口*/
            virt_ifmap = ifmap_list->virt_if;
            if(NULL == virt_ifmap)
                return NULL;

            while(virt_ifmap)
            {
                if((virt_ifmap->flags & TBS_NFP_F_MAP_VLAN)
                    && (virt_ifmap->vlanid & VLAN_VID_MASK) == vlan_id)
                    return virt_ifmap;

                /*遍历其他兄弟虚接口*/
                virt_ifmap = virt_ifmap->virt_next;
            }

            break;
        }

        ifmap_list = ifmap_list->next_map;
    }

    return NULL;
}
#endif

/*=========================================================================
 Function:      inline TBS_NFP_IF_MAP *tbs_nfp_if_phys_get(TBS_NFP_IF_MAP *virt_node)

 Description:       通过虚接口节点指针获取最终物理接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             const TBS_NFP_IF_MAP *virt_node:    虚接口节点指针
 Output:            无
 Return:            vlan ifmap指针
 Others:
=========================================================================*/
inline TBS_NFP_IF_MAP *tbs_nfp_if_phys_get(TBS_NFP_IF_MAP *virt_node)
{
    if(NULL == virt_node)
        return NULL;

    while(virt_node->parent_if)
    {
        virt_node = virt_node->parent_if;
    }

    return virt_node;
}


/*=========================================================================
 Function:      inline static void tbs_nfp_if_add_to_list(TBS_NFP_IF_MAP *ifmap)

 Description:       if_map节点添加到hash链表中
 Data Accessed:     无
 Data Updated:
 Input:             TBS_NFP_IF_MAP *ifmap   ifmap节点指针
 Output:            无
 Return:            无
 Others:
=========================================================================*/
inline static void tbs_nfp_if_add_to_list(TBS_NFP_IF_MAP *ifmap)
{
    int hash = tbs_nfp_itf_hash(ifmap->ifidx);

    ifmap->next_map = g_if_map[hash];
    g_if_map[hash] = ifmap;

    return;
}



/*=========================================================================
 Function:      static TBS_NFP_IF_MAP * tbs_nfp_if_remove_from_list(TBS_NFP_IF_MAP **list,
                    int if_idx)

 Description:       从指定设备链表中移除链表节点，不释放内存
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             TBS_NFP_IF_MAP **list: 设备链表指针
                    int if_idx: 节点ifindex
 Output:            TBS_NFP_IF_MAP **list:  新的链表头指针
 Return:            移除ifmap节点指针
 Others:
=========================================================================*/
inline static TBS_NFP_IF_MAP * tbs_nfp_if_remove_from_list(TBS_NFP_IF_MAP **list,
    int if_idx)
{
    TBS_NFP_IF_MAP *ifmap, *prev_ifmap;

    if(NULL == list)
        return NULL;

    ifmap = prev_ifmap = *list;

    while (ifmap) {
        if (ifmap->ifidx == if_idx)
            break;
        prev_ifmap = ifmap;
        ifmap = ifmap->next_map;
    }

    if(NULL == ifmap){
        TBS_NFP_ERROR("not fund if_map node(%d) from list\n", if_idx);
        return NULL;
    }

    if(ifmap == prev_ifmap)
        *list = ifmap->next_map;
    else
        prev_ifmap->next_map = ifmap->next_map;

    ifmap->next_map = NULL;

    return ifmap;
}



/*=========================================================================
 Function:      static int tbs_nfp_if_clean_virtif_by_parent(TBS_NFP_IF_MAP *if_map)

 Description:       物理接口移除时，清除所有派生接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             TBS_NFP_IF_MAP *if_map: 物理接口map指针
 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
inline static int tbs_nfp_if_clean_virtif_by_parent(TBS_NFP_IF_MAP *if_map)
{
    TBS_NFP_IF_MAP *virt_ifmap, *virt_next_ifmap;

    if(NULL == if_map)
        return TBS_NFP_ERR;

    virt_ifmap = if_map->virt_if;
    if_map->virt_if = NULL;

    /*遍历虚接口链表并清除接口*/
    while(virt_ifmap)
    {
        virt_next_ifmap = virt_ifmap->virt_next;
#ifdef TBS_NFP_DUMMYPORT
        /*即使删除已经绑定的接口，dummyport也不从链表中移除*/
        if(virt_ifmap->flags & TBS_NFP_F_MAP_DUMMYPORT)
        {
            virt_ifmap->parent_if = NULL;
            virt_ifmap->virt_next = NULL;
        }
        else
#endif  /* TBS_NFP_DUMMYPORT */
		{
			tbs_nfp_if_remove_from_list(&g_if_map[tbs_nfp_itf_hash(virt_ifmap->ifidx)], virt_ifmap->ifidx);
			tbs_nfp_if_map_free(virt_ifmap);
		}

        virt_ifmap = virt_next_ifmap;
    }

    return TBS_NFP_OK;
}


/*=========================================================================
 Function:      static int tbs_nfp_if_unlink_from_parent(TBS_NFP_IF_MAP *parent_map,
                    int if_idx)

 Description:       删除虚接口时，将该节点从对应物理接口的虚设备链表中移除
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             TBS_NFP_IF_MAP *parent_map: 物理接口map指针
                    int if_idx: 要移除的虚接口ifindex
 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
inline static int tbs_nfp_if_unlink_from_parent(TBS_NFP_IF_MAP *parent_map,
        int if_idx)
{
    TBS_NFP_IF_MAP *virt_ifmap, *virt_prev_ifmap;

    if(NULL == parent_map)
        return TBS_NFP_ERR;

    /*遍历虚接口链表，移除if_idx节点*/
    virt_prev_ifmap = virt_ifmap = parent_map->virt_if;

    while(virt_ifmap)
    {
        if(virt_ifmap->ifidx == if_idx)
        {
            virt_ifmap->parent_if = NULL;

            if(virt_prev_ifmap == virt_ifmap)
                parent_map->virt_if = virt_ifmap->virt_next;
            else
                virt_prev_ifmap->virt_next = virt_ifmap->virt_next;

            virt_ifmap->virt_next = NULL;
            break;
        }

        virt_prev_ifmap = virt_ifmap;
        virt_ifmap = virt_ifmap->virt_next;
    }

    return TBS_NFP_OK;
}

/*=========================================================================
 Function:      static int tbs_nfp_if_link_to_virtlist(TBS_NFP_IF_MAP *parent_map,
        TBS_NFP_IF_MAP *virt_ifmap)

 Description:       虚接口链接到父接口的虚接口链表中
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             TBS_NFP_IF_MAP *parent_map: 物理接口map指针
                    TBS_NFP_IF_MAP *virt_ifmap: 虚接口节点指针
 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
inline static int tbs_nfp_if_link_to_virtlist(TBS_NFP_IF_MAP *parent_map,
        TBS_NFP_IF_MAP *virt_ifmap)
{
    if(NULL == parent_map || NULL == virt_ifmap)
        return TBS_NFP_ERR;

    virt_ifmap->virt_next = parent_map->virt_if;
    parent_map->virt_if = virt_ifmap;
    virt_ifmap->parent_if = parent_map;

    return TBS_NFP_OK;
}


#ifdef TBS_NFP_BRIDGE
/*=========================================================================
 Function:      inline static void tbs_nfp_if_br_port_clean(int br_idx)

 Description:       清除指定bridge中的bridge_port
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int br_idx: bridge ifindex
 Output:            无
 Return:            无
 Others:
=========================================================================*/
inline static void tbs_nfp_if_br_port_clean(int br_idx)
{
    int i = 0;
    TBS_NFP_IF_MAP *ifmap;

    for(; i < TBS_NFP_ITF_HASH_SIZE; i++)
    {
        ifmap = g_if_map[i];

        while(ifmap)
        {
            if(ifmap->bridge_if == br_idx)
                ifmap->bridge_if = 0;

            ifmap = ifmap->next_map;
        }
    }

    return;
}
#endif


/*=========================================================================
 Function:      inline bool tbs_nfp_ifmap_exist(int if_idx)

 Description:       判断接口是否已经存在
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx      ifindex
 Output:            无
 Return:            true:   已经存在，false:    不存在
 Others:
=========================================================================*/
inline bool tbs_nfp_ifmap_exist(int if_idx)
{
    bool exist = false;

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);

    if(tbs_nfp_ifmap_get(if_idx))
        exist = true;

    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

    return exist;
}


/*=========================================================================
 Function:      int tbs_nfp_if_phys_add(int if_idx, int mtu, const char* mac,
    const char *if_name)

 Description:       以太网物理接口添加
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    接口mtu值
                    const char* mac:   mac地址
                    const char *if_name:    接口名称
 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_phys_add(int if_idx, int mtu, const char* mac,
    const char *if_name)
{
    TBS_NFP_IF_MAP *ifmap;
    struct net_device *dev = NULL;

    if(NULL == mac || NULL == if_name || mtu < 0 ||
        mtu > 1500)
    {
        TBS_NFP_ERROR("tbs_nfp_if_wlan_add() parameter error\n");
        return TBS_NFP_ERR;
    }

    if(tbs_nfp_ifmap_exist(if_idx))
    {
        TBS_NFP_ERROR("if: %s(%d) exist\n", if_name, if_idx);
        return TBS_NFP_ERR;
    }

    dev = dev_get_by_name(&init_net, if_name);
    if(NULL == dev)
    {
        TBS_NFP_ERROR("dev_get_by_name(%s)\n", if_name);
        return TBS_NFP_ERR;
    }

    ifmap = tbs_nfp_if_map_alloc(if_idx, mtu, mac, if_name);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_if_map_alloc() fail\n");
        dev_put(dev);

        return TBS_NFP_ERR;
    }

    ifmap->flags |= TBS_NFP_F_MAP_INT;
    ifmap->dev = dev;

    /*g_if_map中添加节点*/
    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    tbs_nfp_if_add_to_list(ifmap);

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return TBS_NFP_OK;
}


#ifdef TBS_NFP_WLAN
/*=========================================================================
 Function:      int tbs_nfp_if_wlan_add(int if_idx, int mtu,
    const char* mac, const char *if_name)

 Description:       无线接口添加
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    接口mtu值
                    const char* mac:   mac地址
                    const char *if_name:    接口名称
 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_wlan_add(int if_idx, int mtu,
    const char* mac, const char *if_name)
{
    TBS_NFP_IF_MAP *ifmap;
    struct net_device *dev = NULL;

    if(NULL == mac || NULL == if_name || mtu < 0)
    {
        TBS_NFP_ERROR("tbs_nfp_if_wlan_add() parameter error\n");
        return TBS_NFP_ERR;
    }

    if(tbs_nfp_ifmap_exist(if_idx))
    {
        TBS_NFP_ERROR("if: %s(%d) exist\n", if_name, if_idx);
        return TBS_NFP_ERR;
    }

    dev = dev_get_by_name(&init_net, if_name);
    if(NULL == dev)
    {
        TBS_NFP_ERROR("dev_get_by_name(%s)\n", if_name);
        return TBS_NFP_ERR;
    }

    ifmap = tbs_nfp_if_map_alloc(if_idx, mtu, mac, if_name);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_if_map_alloc() fail\n");
        dev_put(dev);

        return TBS_NFP_ERR;
    }

    ifmap->flags |= TBS_NFP_F_MAP_WLAN;
    ifmap->dev = dev;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    tbs_nfp_if_add_to_list(ifmap);

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return TBS_NFP_OK;
}
#endif


#ifdef TBS_NFP_BRIDGE
/*=========================================================================
 Function:      int tbs_nfp_if_bridge_add(int if_idx, int mtu, const char* mac,
    const char *if_name)

 Description:       bridge接口添加
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    接口mtu值
                    const char* mac:   mac地址
                    const char *if_name:    接口名称
 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_bridge_add(int if_idx, int mtu, const char* mac,
    const char *if_name)
{
    TBS_NFP_IF_MAP *ifmap;

    if(NULL == mac || NULL == if_name || mtu < 0 ||
        mtu > 1500)
    {
        TBS_NFP_ERROR("tbs_nfp_if_bridge_add() parameter error\n");
        return TBS_NFP_ERR;
    }

    if(tbs_nfp_ifmap_exist(if_idx))
    {
        TBS_NFP_ERROR("if: %s(%d) exist\n", if_name, if_idx);
        return TBS_NFP_ERR;
    }

    ifmap = tbs_nfp_if_map_alloc(if_idx, mtu, mac, if_name);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_if_map_alloc() fail\n");
        return TBS_NFP_ERR;
    }

    ifmap->flags |= TBS_NFP_F_MAP_BRIDGE;
    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    tbs_nfp_if_add_to_list(ifmap);

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return TBS_NFP_OK;
}
#endif

#ifdef TBS_NFP_VLAN
/*=========================================================================
 Function:      int tbs_nfp_if_vlan_add(int if_idx, int mtu, int phys_if_idx,
    unsigned short vlan_id, const char *mac, const char *if_name)

 Description:       vlan接口添加
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    接口mtu值
                    int phys_if_idx:    物理接口ifindex
                    unsigned short vlan_id: vlanid
                    const char* mac:   mac地址
                    const char *if_name:    接口名称
 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_vlan_add(int if_idx, int mtu, int phys_if_idx,
    unsigned short vlan_id, const char *mac, const char *if_name)
{
    TBS_NFP_IF_MAP *ifmap, *phys_ifmap;

    if(NULL == mac || NULL == if_name || mtu < 0 ||
        mtu > 1500 || phys_if_idx <= 0)
    {
        TBS_NFP_ERROR("tbs_nfp_if_vlan_add() parameter error\n");
        return TBS_NFP_ERR;
    }

    if(tbs_nfp_ifmap_exist(if_idx))
    {
        TBS_NFP_ERROR("if: %s(%d) exist\n", if_name, if_idx);
        return TBS_NFP_ERR;
    }

    ifmap = tbs_nfp_if_map_alloc(if_idx, mtu, mac, if_name);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_if_map_alloc() fail\n");
        return TBS_NFP_ERR;
    }

    ifmap->flags |= TBS_NFP_F_MAP_VLAN;
    ifmap->vlanid = vlan_id;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    phys_ifmap = tbs_nfp_ifmap_get(phys_if_idx);

    if(NULL == phys_ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_ifmap_get() fail, phys_if_idx:%d\n", phys_if_idx);
        goto err;
    }

    tbs_nfp_if_add_to_list(ifmap);

    /*插入到物理接口的派生接口链表中*/
    tbs_nfp_if_link_to_virtlist(phys_ifmap, ifmap);
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return TBS_NFP_OK;

err:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    if(ifmap)
        tbs_nfp_if_map_free(ifmap);

    return TBS_NFP_ERR;
}
#endif

#ifdef TBS_NFP_PPP
/*=========================================================================
 Function:      int tbs_nfp_if_pppoe_add(int if_idx, int phys_if_idx, int mtu,
    unsigned short session_id,
                    const char* remot_mac, const char *if_name)

 Description:       pppoe接口添加
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    接口mtu值
                    int phys_if_idx:    物理接口ifindex
                    unsigned short session_id: pppoe连接session_id
                    const char* remot_mac:   对端mac地址
                    const char *if_name:    接口名称
 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_pppoe_add(int if_idx, int phys_if_idx, int mtu, unsigned short session_id,
    const char* remot_mac, const char *if_name)
{
    TBS_NFP_IF_MAP *ifmap = NULL;
    TBS_NFP_IF_MAP *phys_ifmap = NULL;
    struct net_device *dev = NULL;

    if(NULL == remot_mac || NULL == if_name ||
        phys_if_idx < 0 || phys_if_idx <= 0)
    {
        TBS_NFP_ERROR("tbs_nfp_if_pppoe_add() parameter error\n");
        return TBS_NFP_ERR;
    }

    if(tbs_nfp_ifmap_exist(if_idx))
    {
        TBS_NFP_ERROR("if: %s(%d) exist\n", if_name, if_idx);
        return TBS_NFP_ERR;
    }

    ifmap = tbs_nfp_if_map_alloc(if_idx, mtu, NULL, if_name);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_if_map_alloc() fail\n");
        return TBS_NFP_ERR;
    }

    dev = dev_get_by_name(&init_net, if_name);
    if(NULL == dev)
    {
        TBS_NFP_ERROR("dev_get_by_name(%s)\n", if_name);
        return TBS_NFP_ERR;
    }

    ifmap->dev = dev;
    ifmap->flags |= TBS_NFP_F_MAP_PPPOE;
    ifmap->session_id = session_id;
    memcpy(ifmap->remote_mac, remot_mac, sizeof(ifmap->remote_mac));

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    phys_ifmap = tbs_nfp_ifmap_get(phys_if_idx);
    if(NULL == phys_ifmap)
    {
        if(dev)
            dev_put(dev);

        TBS_NFP_ERROR("tbs_nfp_ifmap_get() fail, phys_if_idx:%d\n", phys_if_idx);
        goto err;
    }

    tbs_nfp_if_add_to_list(ifmap);

    /*插入到物理接口的派生接口链表中*/
    tbs_nfp_if_link_to_virtlist(phys_ifmap, ifmap);
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return TBS_NFP_OK;

err:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    if(ifmap)
        tbs_nfp_if_map_free(ifmap);

    return TBS_NFP_ERR;
}

/*=========================================================================
 Function:      inline TBS_NFP_IF_MAP *tbs_nfp_pppoe_ifmap_get(int port, const TBS_NFP_IF_MAP *in_ifmap,
    unsigned short sess_id)

 Description:       通过父接口和session id获取pppoe接口的节点指针
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int parent_if_idx:  物理接口index
                    const TBS_NFP_IF_MAP *in_ifmap: 父接口指针
                    unsigned short sess_id: session id
 Output:            无
 Return:            pppoe ifmap指针
 Others:
 Authors: baiyonghui 2012-1-12
=========================================================================*/
inline TBS_NFP_IF_MAP *tbs_nfp_pppoe_ifmap_get(int port, const TBS_NFP_IF_MAP *in_ifmap,
    unsigned short sess_id)
{
    TBS_NFP_IF_MAP *virt_ifmap;

    if(NULL == in_ifmap)
        return NULL;

    /*检查第一个虚接口*/
    virt_ifmap = in_ifmap->virt_if;
    while(virt_ifmap)
    {
        if((virt_ifmap->flags & TBS_NFP_F_MAP_PPPOE)
                && virt_ifmap->session_id == sess_id)
            break;

        virt_ifmap = virt_ifmap->virt_next;
    }

    return virt_ifmap;
}
#endif


#ifdef TBS_NFP_DUMMYPORT
/*=========================================================================
 Function:      int tbs_nfp_if_dummyport_add(int if_idx, int mtu, int phys_if_idx,
    const char* mac, const char *if_name)

 Description:       dummyport接口添加
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    接口mtu值
                    int phys_if_idx:    物理接口ifindex，非0时有效
                    const char* mac:   mac地址
                    const char *if_name:    接口名称
 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_dummyport_add(int if_idx, int mtu, int phys_if_idx,
    const char* mac, const char *if_name)
{
    TBS_NFP_IF_MAP *ifmap, *phys_ifmap = NULL;

    if(NULL == mac || NULL == if_name || mtu < 0 ||
        mtu > 1500 || phys_if_idx < 0)
    {
        TBS_NFP_ERROR("tbs_nfp_if_dummyport_add() parameter error\n");
        return TBS_NFP_ERR;
    }

    if(tbs_nfp_ifmap_exist(if_idx))
    {
        TBS_NFP_ERROR("if: %s(%d) exist\n", if_name, if_idx);
        return TBS_NFP_ERR;
    }

    ifmap = tbs_nfp_if_map_alloc(if_idx, mtu, mac, if_name);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tsb_nfp_if_map_node_add() fail\n");
        return TBS_NFP_ERR;
    }

    ifmap->flags |= TBS_NFP_F_MAP_DUMMYPORT;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    /*带绑定关系*/
    if(phys_if_idx > 0)
    {
        phys_ifmap = tbs_nfp_ifmap_get(phys_if_idx);

        if(NULL == phys_ifmap)
        {
            TBS_NFP_ERROR("tbs_nfp_if_get() fail, phys_if_idx:%d\n", phys_if_idx);
            goto err;
        }
    }

    tbs_nfp_if_add_to_list(ifmap);

    /*插入到物理接口的派生接口链表中*/
    if(phys_ifmap)
        tbs_nfp_if_link_to_virtlist(phys_ifmap, ifmap);

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return TBS_NFP_OK;

err:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    if(ifmap)
        tbs_nfp_if_map_free(ifmap);

    return TBS_NFP_ERR;
}


/*=========================================================================
 Function:      int tbs_nfp_if_dummyport_bind(int if_idx, int phys_if_idx)

 Description:       dummyport绑定接口操作
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int phys_if_idx:    物理接口ifindex
 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_dummyport_bind(int if_idx, int phys_if_idx)
{
    int ret = TBS_NFP_OK;
    TBS_NFP_IF_MAP *ifmap, *phys_ifmap;

    TBS_NFP_INTO_FUNC;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    ifmap = tbs_nfp_ifmap_get(if_idx);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_ifmap_get() fail, ifindex:%d\n", if_idx);
        ret = TBS_NFP_ERR;
        goto out;
    }

    if(!(ifmap->flags & TBS_NFP_F_MAP_DUMMYPORT))
    {
        TBS_NFP_ERROR("interface %s & TBS_NFP_F_MAP_DUMMYPORT false\n", ifmap->name);
        ret = TBS_NFP_ERR;
        goto out;
    }

    phys_ifmap = tbs_nfp_ifmap_get(phys_if_idx);
    if(NULL == phys_ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_ifmap_get() fail, ifindex:%d\n", phys_if_idx);
        ret = TBS_NFP_ERR;
        goto out;
    }

    /*已经绑定接口，先解除绑定*/
    if(ifmap->parent_if)
    {
        tbs_nfp_if_unlink_from_parent(ifmap->parent_if, if_idx);
        ifmap->parent_if = NULL;
    }

    /*添加到所绑定接口的虚接口链表中，并对parent赋值，指向绑定的接口*/
    tbs_nfp_if_link_to_virtlist(phys_ifmap, ifmap);

out:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}


/*=========================================================================
 Function:      int tbs_nfp_if_dummyport_unbind(int if_idx)

 Description:       dummyport解除绑定接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex

 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_dummyport_unbind(int if_idx)
{
    int ret = TBS_NFP_OK;
    TBS_NFP_IF_MAP *ifmap;

    TBS_NFP_INTO_FUNC;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    ifmap = tbs_nfp_ifmap_get(if_idx);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_ifmap_get() fail, ifindex:%d\n", if_idx);
        ret = TBS_NFP_ERR;
        goto out;
    }

    if(!(ifmap->flags & TBS_NFP_F_MAP_DUMMYPORT))
    {
        TBS_NFP_ERROR("interface %s & TBS_NFP_F_MAP_DUMMYPORT false\n", ifmap->name);
        ret = TBS_NFP_ERR;
        goto out;
    }

    if(ifmap->parent_if)
    {
        tbs_nfp_if_unlink_from_parent(ifmap->parent_if, if_idx);
        ifmap->parent_if = NULL;
    }

out:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}
#endif  /* TBS_NFP_DUMMYPORT */


#if defined(CONFIG_VNET)
int tbs_nfp_if_vnet_add(int if_idx, int mtu, int phys_if_idx,
    const char* mac, const char *if_name)
{
    TBS_NFP_IF_MAP *ifmap, *phys_ifmap;

    if(NULL == mac || NULL == if_name || mtu < 0 ||
        mtu > 1500 || phys_if_idx <= 0)
    {
        TBS_NFP_ERROR("tbs_nfp_if_vnet_add() parameter error\n");
        return TBS_NFP_ERR;
    }

    if(tbs_nfp_ifmap_exist(if_idx))
    {
        TBS_NFP_ERROR("if: %s(%d) exist\n", if_name, if_idx);
        return TBS_NFP_ERR;
    }

    ifmap = tbs_nfp_if_map_alloc(if_idx, mtu, mac, if_name);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_if_map_alloc() fail\n");
        return TBS_NFP_ERR;
    }

    ifmap->flags |= TBS_NFP_F_MAP_VNET;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    phys_ifmap = tbs_nfp_ifmap_get(phys_if_idx);

    if(NULL == phys_ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_ifmap_get() fail, phys_if_idx:%d\n", phys_if_idx);
        goto err;
    }

    tbs_nfp_if_add_to_list(ifmap);

    /*插入到物理接口的派生接口链表中*/
    tbs_nfp_if_link_to_virtlist(phys_ifmap, ifmap);
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return TBS_NFP_OK;

err:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    if(ifmap)
        tbs_nfp_if_map_free(ifmap);

    return TBS_NFP_ERR;
}
#endif  /* CONFIG_VNET */


#ifdef TBS_NFP_BRIDGE
/*=========================================================================
 Function:      int tbs_nfp_if_bridge_port_add(int br_idx, int br_port_idx)

 Description:       添加brport操作接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int br_idx: bridge ifindex
                    int br_port_idx:    br_port ifindex

 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_br_port_add(int br_idx, int br_port_idx)
{
    int ret = TBS_NFP_OK;
    TBS_NFP_IF_MAP *ifmap, *br_ifmap;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
    br_ifmap = tbs_nfp_ifmap_get(br_idx);
    if(NULL == br_ifmap || !(br_ifmap->flags & TBS_NFP_F_MAP_BRIDGE))
    {
        TBS_NFP_ERROR("tbs_nfp_ifmap_get() fail, ifindex:%d\n", br_port_idx);
        ret = TBS_NFP_ERR;
        goto out;
    }

    ifmap = tbs_nfp_ifmap_get(br_port_idx);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_ifmap_get() fail, ifindex:%d\n", br_port_idx);
        ret = TBS_NFP_ERR;
        goto out;
    }
    ifmap->flags |= TBS_NFP_F_MAP_BRIDGE_PORT;
    ifmap->bridge_if = br_idx;

out:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}


/*=========================================================================
 Function:      int tbs_nfp_if_bridge_port_remove(int br_port_idx)

 Description:       移除brport操作接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int br_port_idx:    br_port ifindex

 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_br_port_remove(int br_port_idx)
{
    int ret = TBS_NFP_OK;
    TBS_NFP_IF_MAP *ifmap;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    ifmap = tbs_nfp_ifmap_get(br_port_idx);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_ifmap_get() fail, ifindex:%d\n", br_port_idx);
        ret = TBS_NFP_ERR;
        goto out;
    }

    ifmap->bridge_if = 0;
    ifmap->flags &= ~TBS_NFP_F_MAP_BRIDGE_PORT;
    /*清除桥转发规则*/
    tbs_nfp_bridge_rule_clear_by_ifindex(ifmap->ifidx);

out:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}
#endif

/*=========================================================================
 Function:      int tbs_nfp_if_brport_remove(int br_port_idx)

 Description:       通用接口删除操作
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx:    interface index

 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_delete(int if_idx)
{
    int ret = TBS_NFP_OK;
    unsigned int hash = tbs_nfp_itf_hash(if_idx);
    TBS_NFP_IF_MAP *ifmap;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    ifmap = tbs_nfp_ifmap_get(if_idx);
    if(NULL == ifmap)
    {
        TBS_NFP_ERROR("tbs_nfp_ifmap_get() fail, ifindex:%d\n", if_idx);
        ret = TBS_NFP_ERR;
        goto out;
    }

    /*从物理接口的虚接口链表中脱离*/
    if(ifmap->parent_if)
        tbs_nfp_if_unlink_from_parent(ifmap->parent_if, ifmap->ifidx);

    /*移除所有派生接口，dummyport除外*/
    if(ifmap->virt_if){
        ret = tbs_nfp_if_clean_virtif_by_parent(ifmap);
        if(TBS_NFP_OK != ret){
            TBS_NFP_ERROR("clean virt if fail, parent ifname: %s)\n", ifmap->name);
            goto out;
        }
    }

#ifdef TBS_NFP_BRIDGE
    /*如果是bridge，则从bridge中移除所有bridge_port*/
    if(ifmap->flags & TBS_NFP_F_MAP_BRIDGE)
    {
        tbs_nfp_if_br_port_clean(ifmap->ifidx);
    }
#endif

    /*从hash表中移除该节点*/
    ifmap = tbs_nfp_if_remove_from_list(&g_if_map[hash], if_idx);
    tbs_nfp_if_map_free(ifmap);

out:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}


/*=========================================================================
 Function:      int tbs_nfp_if_state(int if_idx, unsigned int state)

 Description:       interface状态变化操接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx:    interface index
                    unsigned int state: 接口状态

 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_state(int if_idx, unsigned int state)
{
    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return TBS_NFP_OK;
}


/*=========================================================================
 Function:      int tbs_nfp_if_mac_change(int if_idx, const char *mac)

 Description:       mac变化操接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx:    interface index
                    const char *mac:    更新后的mac地址

 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_mac_change(int if_idx, const char *mac)
{
    int ret = TBS_NFP_ERR;
    TBS_NFP_IF_MAP *ifmap = NULL;

    if(NULL == mac)
        return ret;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    ifmap = tbs_nfp_ifmap_get(if_idx);
    if (ifmap) {
        memcpy(ifmap->mac, mac, sizeof(ifmap->mac));
        ret = TBS_NFP_OK;
    }

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return ret;
}


/*=========================================================================
 Function:      int tbs_nfp_if_mtu_change(int if_idx, int mtu)

 Description:       mtu变化操接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx:    interface index
                    int mtu:    接口MTU值

 Output:            无
 Return:            0:  成功;   其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_mtu_change(int if_idx, int mtu)
{
    int ret = TBS_NFP_ERR;
    TBS_NFP_IF_MAP *ifmap = NULL;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    ifmap = tbs_nfp_ifmap_get(if_idx);

    if(ifmap) {
        ifmap->mtu = mtu;
        ret = TBS_NFP_OK;
    }

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return ret;
}

/*=========================================================================
 Function:      static inline void tbs_nfp_if_clean(void)

 Description:       清除所有interface接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             无
 Output:            无
 Return:            无
 Others:
=========================================================================*/
static inline void tbs_nfp_if_clean(void)
{
    int i = 0;
    TBS_NFP_IF_MAP *prev_if_map, *ifmap;

    for(; i < TBS_NFP_ITF_HASH_SIZE; i++)
    {
        prev_if_map = ifmap = g_if_map[i];

        while(ifmap)
        {
            kfree((void *)ifmap);
            ifmap = prev_if_map->next_map;
        }

        g_if_map[i] = NULL;
    }

    return;
}


/*=========================================================================
 Function:      int tbs_nfp_if_reset(void)

 Description:       清除所有interface接口
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             无
 Output:            无
 Return:            0:  成功，  其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_if_reset(void)
{
    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
    tbs_nfp_if_clean();
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return TBS_NFP_OK;
}


/*=========================================================================
 Function:		void tbs_nfp_if_exit(void)

 Description:		销毁interface 节点和hash表，用于加速器模块退出
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_if_exit(void)
{
    tbs_nfp_if_clean();
}

/*=========================================================================
 Function:		inline int tbs_nfp_if_init(void)

 Description:		初始化interface hash表，用于加速器模块加载
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:			    无
 Output:			无
 Return:			0:  成功,   其他:   失败
 Others:
=========================================================================*/
inline int tbs_nfp_if_init(void)
{
    int i = 0;

    for(; i < TBS_NFP_ITF_HASH_SIZE; i++)
        g_if_map[i] = NULL;

    return TBS_NFP_OK;
}

