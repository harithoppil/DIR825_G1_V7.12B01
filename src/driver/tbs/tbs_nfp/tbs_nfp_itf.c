/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* �ļ����� : tbs_nfp_itf.c
* �ļ����� : TBS������interface����ģ��
*
* �޶���¼ :
*          1 ���� : pengyao
*            ���� : 2011-11-10
*            ���� :
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

 Description:       if_map�ڵ���䣬����ʼ��
 Data Accessed:     ��
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    �ӿ�mtuֵ
                    const char* mac:   mac��ַ
                    const char *if_name:    �ӿ�����

 Output:            ��
 Return:            ifmapָ��
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

 Description:       �ͷ�if_map�ڵ�
 Data Accessed:     ��
 Data Updated:
 Input:             ��
 Output:            ��
 Return:            ifmapָ��
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

 Description:       ��ӡָ��interface�ӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             const TBS_NFP_IF_MAP *if_map:   interface mapָ��
 Output:            ��
 Return:            ��
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

    /*��ӡ�����ӿ���Ϣ*/
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

 Description:       ��ӡhash����������interface�ӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             ��
 Output:            ��
 Return:            ��
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

 Description:       ͨ��if_idx����ȡifmapָ��
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx      ifindex
 Output:            ��
 Return:            ifmapָ��
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

 Description:       ͨ������ӿں�vlan id��ȡvlan�ӿڵĽڵ�ָ��
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int parent_if_idx:  ����ӿ�index
                    unsigned short vlan_id: vlan id
 Output:            ��
 Return:            vlan ifmapָ��
 Others:
=========================================================================*/
inline TBS_NFP_IF_MAP *tbs_nfp_vlan_ifmap_get(int parent_if_idx, unsigned short vlan_id)
{
    TBS_NFP_IF_MAP *ifmap_list, *virt_ifmap = NULL;

    if(vlan_id >= 4096)
        return NULL;

    /*��ȡhash���������������ҵ�parent_if_idx�ڵ�*/
    ifmap_list = g_if_map[tbs_nfp_itf_hash(parent_if_idx)];
    while(ifmap_list)
    {
        if(ifmap_list->ifidx == parent_if_idx)
        {
            /*����һ����ӿ�*/
            virt_ifmap = ifmap_list->virt_if;
            if(NULL == virt_ifmap)
                return NULL;

            while(virt_ifmap)
            {
                if((virt_ifmap->flags & TBS_NFP_F_MAP_VLAN)
                    && (virt_ifmap->vlanid & VLAN_VID_MASK) == vlan_id)
                    return virt_ifmap;

                /*���������ֵ���ӿ�*/
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

 Description:       ͨ����ӿڽڵ�ָ���ȡ��������ӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             const TBS_NFP_IF_MAP *virt_node:    ��ӿڽڵ�ָ��
 Output:            ��
 Return:            vlan ifmapָ��
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

 Description:       if_map�ڵ���ӵ�hash������
 Data Accessed:     ��
 Data Updated:
 Input:             TBS_NFP_IF_MAP *ifmap   ifmap�ڵ�ָ��
 Output:            ��
 Return:            ��
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

 Description:       ��ָ���豸�������Ƴ�����ڵ㣬���ͷ��ڴ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             TBS_NFP_IF_MAP **list: �豸����ָ��
                    int if_idx: �ڵ�ifindex
 Output:            TBS_NFP_IF_MAP **list:  �µ�����ͷָ��
 Return:            �Ƴ�ifmap�ڵ�ָ��
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

 Description:       ����ӿ��Ƴ�ʱ��������������ӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             TBS_NFP_IF_MAP *if_map: ����ӿ�mapָ��
 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
 Others:
=========================================================================*/
inline static int tbs_nfp_if_clean_virtif_by_parent(TBS_NFP_IF_MAP *if_map)
{
    TBS_NFP_IF_MAP *virt_ifmap, *virt_next_ifmap;

    if(NULL == if_map)
        return TBS_NFP_ERR;

    virt_ifmap = if_map->virt_if;
    if_map->virt_if = NULL;

    /*������ӿ���������ӿ�*/
    while(virt_ifmap)
    {
        virt_next_ifmap = virt_ifmap->virt_next;
#ifdef TBS_NFP_DUMMYPORT
        /*��ʹɾ���Ѿ��󶨵Ľӿڣ�dummyportҲ�����������Ƴ�*/
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

 Description:       ɾ����ӿ�ʱ�����ýڵ�Ӷ�Ӧ����ӿڵ����豸�������Ƴ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             TBS_NFP_IF_MAP *parent_map: ����ӿ�mapָ��
                    int if_idx: Ҫ�Ƴ�����ӿ�ifindex
 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
 Others:
=========================================================================*/
inline static int tbs_nfp_if_unlink_from_parent(TBS_NFP_IF_MAP *parent_map,
        int if_idx)
{
    TBS_NFP_IF_MAP *virt_ifmap, *virt_prev_ifmap;

    if(NULL == parent_map)
        return TBS_NFP_ERR;

    /*������ӿ������Ƴ�if_idx�ڵ�*/
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

 Description:       ��ӿ����ӵ����ӿڵ���ӿ�������
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             TBS_NFP_IF_MAP *parent_map: ����ӿ�mapָ��
                    TBS_NFP_IF_MAP *virt_ifmap: ��ӿڽڵ�ָ��
 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

 Description:       ���ָ��bridge�е�bridge_port
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int br_idx: bridge ifindex
 Output:            ��
 Return:            ��
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

 Description:       �жϽӿ��Ƿ��Ѿ�����
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx      ifindex
 Output:            ��
 Return:            true:   �Ѿ����ڣ�false:    ������
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

 Description:       ��̫������ӿ����
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    �ӿ�mtuֵ
                    const char* mac:   mac��ַ
                    const char *if_name:    �ӿ�����
 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

    /*g_if_map����ӽڵ�*/
    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    tbs_nfp_if_add_to_list(ifmap);

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return TBS_NFP_OK;
}


#ifdef TBS_NFP_WLAN
/*=========================================================================
 Function:      int tbs_nfp_if_wlan_add(int if_idx, int mtu,
    const char* mac, const char *if_name)

 Description:       ���߽ӿ����
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    �ӿ�mtuֵ
                    const char* mac:   mac��ַ
                    const char *if_name:    �ӿ�����
 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

 Description:       bridge�ӿ����
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    �ӿ�mtuֵ
                    const char* mac:   mac��ַ
                    const char *if_name:    �ӿ�����
 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

 Description:       vlan�ӿ����
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    �ӿ�mtuֵ
                    int phys_if_idx:    ����ӿ�ifindex
                    unsigned short vlan_id: vlanid
                    const char* mac:   mac��ַ
                    const char *if_name:    �ӿ�����
 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

    /*���뵽����ӿڵ������ӿ�������*/
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

 Description:       pppoe�ӿ����
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    �ӿ�mtuֵ
                    int phys_if_idx:    ����ӿ�ifindex
                    unsigned short session_id: pppoe����session_id
                    const char* remot_mac:   �Զ�mac��ַ
                    const char *if_name:    �ӿ�����
 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

    /*���뵽����ӿڵ������ӿ�������*/
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

 Description:       ͨ�����ӿں�session id��ȡpppoe�ӿڵĽڵ�ָ��
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int parent_if_idx:  ����ӿ�index
                    const TBS_NFP_IF_MAP *in_ifmap: ���ӿ�ָ��
                    unsigned short sess_id: session id
 Output:            ��
 Return:            pppoe ifmapָ��
 Others:
 Authors: baiyonghui 2012-1-12
=========================================================================*/
inline TBS_NFP_IF_MAP *tbs_nfp_pppoe_ifmap_get(int port, const TBS_NFP_IF_MAP *in_ifmap,
    unsigned short sess_id)
{
    TBS_NFP_IF_MAP *virt_ifmap;

    if(NULL == in_ifmap)
        return NULL;

    /*����һ����ӿ�*/
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

 Description:       dummyport�ӿ����
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int mtu:    �ӿ�mtuֵ
                    int phys_if_idx:    ����ӿ�ifindex����0ʱ��Ч
                    const char* mac:   mac��ַ
                    const char *if_name:    �ӿ�����
 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

    /*���󶨹�ϵ*/
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

    /*���뵽����ӿڵ������ӿ�������*/
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

 Description:       dummyport�󶨽ӿڲ���
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex
                    int phys_if_idx:    ����ӿ�ifindex
 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

    /*�Ѿ��󶨽ӿڣ��Ƚ����*/
    if(ifmap->parent_if)
    {
        tbs_nfp_if_unlink_from_parent(ifmap->parent_if, if_idx);
        ifmap->parent_if = NULL;
    }

    /*��ӵ����󶨽ӿڵ���ӿ������У�����parent��ֵ��ָ��󶨵Ľӿ�*/
    tbs_nfp_if_link_to_virtlist(phys_ifmap, ifmap);

out:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}


/*=========================================================================
 Function:      int tbs_nfp_if_dummyport_unbind(int if_idx)

 Description:       dummyport����󶨽ӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx: ifindex

 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

    /*���뵽����ӿڵ������ӿ�������*/
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

 Description:       ���brport�����ӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int br_idx: bridge ifindex
                    int br_port_idx:    br_port ifindex

 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

 Description:       �Ƴ�brport�����ӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int br_port_idx:    br_port ifindex

 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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
    /*�����ת������*/
    tbs_nfp_bridge_rule_clear_by_ifindex(ifmap->ifidx);

out:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}
#endif

/*=========================================================================
 Function:      int tbs_nfp_if_brport_remove(int br_port_idx)

 Description:       ͨ�ýӿ�ɾ������
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx:    interface index

 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

    /*������ӿڵ���ӿ�����������*/
    if(ifmap->parent_if)
        tbs_nfp_if_unlink_from_parent(ifmap->parent_if, ifmap->ifidx);

    /*�Ƴ����������ӿڣ�dummyport����*/
    if(ifmap->virt_if){
        ret = tbs_nfp_if_clean_virtif_by_parent(ifmap);
        if(TBS_NFP_OK != ret){
            TBS_NFP_ERROR("clean virt if fail, parent ifname: %s)\n", ifmap->name);
            goto out;
        }
    }

#ifdef TBS_NFP_BRIDGE
    /*�����bridge�����bridge���Ƴ�����bridge_port*/
    if(ifmap->flags & TBS_NFP_F_MAP_BRIDGE)
    {
        tbs_nfp_if_br_port_clean(ifmap->ifidx);
    }
#endif

    /*��hash�����Ƴ��ýڵ�*/
    ifmap = tbs_nfp_if_remove_from_list(&g_if_map[hash], if_idx);
    tbs_nfp_if_map_free(ifmap);

out:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}


/*=========================================================================
 Function:      int tbs_nfp_if_state(int if_idx, unsigned int state)

 Description:       interface״̬�仯�ٽӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx:    interface index
                    unsigned int state: �ӿ�״̬

 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

 Description:       mac�仯�ٽӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx:    interface index
                    const char *mac:    ���º��mac��ַ

 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

 Description:       mtu�仯�ٽӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             int if_idx:    interface index
                    int mtu:    �ӿ�MTUֵ

 Output:            ��
 Return:            0:  �ɹ�;   ����:   ʧ��
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

 Description:       �������interface�ӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             ��
 Output:            ��
 Return:            ��
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

 Description:       �������interface�ӿ�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:             ��
 Output:            ��
 Return:            0:  �ɹ���  ����:   ʧ��
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

 Description:		����interface �ڵ��hash�����ڼ�����ģ���˳�
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			��
 Others:
=========================================================================*/
void tbs_nfp_if_exit(void)
{
    tbs_nfp_if_clean();
}

/*=========================================================================
 Function:		inline int tbs_nfp_if_init(void)

 Description:		��ʼ��interface hash�����ڼ�����ģ�����
 Data Accessed:     struct list_head g_if_map[TBS_NFP_ITF_HASH_SIZE];
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			0:  �ɹ�,   ����:   ʧ��
 Others:
=========================================================================*/
inline int tbs_nfp_if_init(void)
{
    int i = 0;

    for(; i < TBS_NFP_ITF_HASH_SIZE; i++)
        g_if_map[i] = NULL;

    return TBS_NFP_OK;
}

