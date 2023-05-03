 /**********************************************************************
  Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

  �ļ����� : nfp_interface.c
  �ļ����� : TBS����������

  �޶���¼ :
           1 ���� : xiachaoren
             ���� : 2011-04-10
             ���� :
 **********************************************************************/

#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/if_pppox.h>
#include <linux/if_vlan.h>
#include <linux/rtnetlink.h>
#include <linux/notifier.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/jhash.h>
#include <../net/8021q/vlan.h>

#if (defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE))
#include <linux/if_bridge.h>
#endif

#if defined(CONFIG_TBSMOD_DUMMYPORT)
#include <driver/tbs/dummyport/dummyport.h>
#endif  /* CONFIG_TBSMOD_DUMMYPORT */

#if defined(CONFIG_VNET)
#include <linux/if_vnet.h>
#endif  /* CONFIG_VNET */

/*in root dir*/
#include "nfp_interface.h"
#include "nfp_route.h"
#include "nfp_adapter.h"



/*defined in ppp_generic.c*/
#if defined(CONFIG_PPPOE) || defined(CONFIG_PPPOE_MODULE)
extern char *getPhysicalIfName(char *pIfName);
extern int getSessIdAndAddr(char *pIfName, unsigned char *addr);
#endif

static struct kmem_cache *nfp_interface_cache __read_mostly;


static unsigned int nfp_interface_entry_count = 0;

const char* un_care_dev[] ={
    "lo",
    "imq",
    "sit",
    "tunl",
    "tun",
    "dummy",
    "gre",
    "wifi",
    NULL
    };

static rwlock_t nfp_interface_lock;

static void nfp_interface_print(const struct interface_entry *ife);
static void nfp_interface_del_hash_table_entry(const struct interface_entry *ie);
static void nfp_interface_change_mtu(struct interface_entry *new_entry);

static inline int nfp_interface_option(unsigned short option, struct interface_entry *itf)
{
    int ret = NFP_UNABLE_PARSER;

    if(nfp_cmd_parser){
        struct nfp_adapter_cmd cmd_entry = {option, (void *)itf};
        ret = nfp_cmd_parser(NFP_CMD_ITF, &cmd_entry);
    }

    return ret;
}


/********************************************************************************
 Function:		static inline u_int32_t hash_interface(int ifindex)
 Description:	��interface hash_table��hash���㷽��
 Data Accessed:
 Data Updated:
 Input:			�ӿ�������ifindex
 Output:
 Return:		hash���
 Others:
*********************************************************************************/
static inline u_int32_t hash_interface(int ifindex)
{
    return ifindex & (NFP_ITF_HTABLE_SIZE - 1);
}


static void __nfp_interface_release(struct interface_entry *ie)
{
    NFP_ADAPTER_INTO_FUNC;

    if (atomic_dec_and_test(&ie->refcnt))
        nfp_interface_del_hash_table_entry(ie);
}

/********************************************************************************
 Function:		struct interface_entry *__nfp_interface_entry_get_by_name(const char *ifname)
 Description:	�ڲ��ӿڲ�ѯ����,ͨ��ifname��ѯ�ӿڵĸ�����
 Data Accessed:
 Data Updated:
 Input:			const char *ifname	�ӿ�����
 Output:
 Return:		NULL:�����ڣ�����:interface_entry�ṹ��ָ��
 Others:
********************************************************************************/
struct interface_entry *__nfp_interface_entry_get_by_name(const char *ifname)
{
    int hash_key;
    struct interface_entry *query = NULL;

    NFP_ASSERT(ifname);

    for(hash_key = 0; hash_key < NFP_ITF_HTABLE_SIZE; hash_key++){

        if(list_empty(&nfp_interface_table[hash_key]))
            continue;

        list_for_each_entry(query, &nfp_interface_table[hash_key], list){
            if((atomic_read(&(query->refcnt)) >= 1) &&
                !strncmp(ifname, query->ifname, sizeof(query->ifname)))
                return query;
            else
                continue;
        }
    }

    return NULL;
}

EXPORT_SYMBOL(__nfp_interface_entry_get_by_name);
/********************************************************************************
 Function:		struct interface_entry *__nfp_interface_entry_get_by_index(int index)
 Description:	�ڲ��ӿڲ�ѯ����,ͨ��index��ѯ�ӿڵĸ�����
 Data Accessed:
 Data Updated:
 Input:			int index	�ӿ�������
 Output:
 Return:		NULL:�����ڣ�����:interface_entry�ṹ��ָ��
 Others:
********************************************************************************/
struct interface_entry *__nfp_interface_entry_get_by_index(int ifindex)
{
    int hash_key;
    struct interface_entry *query = NULL;

    hash_key = hash_interface(ifindex);
    list_for_each_entry(query, &nfp_interface_table[hash_key], list){

        if((atomic_read(&(query->refcnt)) >= 1) && (ifindex == query->ifindex))
            return query;
    }

    return NULL;
}

EXPORT_SYMBOL(__nfp_interface_entry_get_by_index);

/********************************************************************************
 Function:		static void nfp_interface_dump(const struct interface_entry *ife)
 Description:	��ʾ�ṹ�����Ҫ��Աֵ
 Data Accessed:
 Data Updated:
 Input:			struct interface_entry 	�ṹ��ָ��
 Output:
 Return:
 Others:
*********************************************************************************/

static void nfp_interface_print(const struct interface_entry *ife)
{
    printk("ifindex\tifname\t\tphys_ifindex\tbr_ifindex\tmacaddr\t\t\tmtu\titf_flags\trefcnt\tstate\n");
    printk("%d\t%-16s%d\t\t%d\t\t%02x:%02x:%02x:%02x:%02x:%02x\t%d\t0x%08x\t%d\t0x%08x\n",
        ife->ifindex, ife->ifname, ife->phys_ifindex, ife->br_index,
        ife->macaddr[0], ife->macaddr[1],
        ife->macaddr[2], ife->macaddr[3],
        ife->macaddr[4], ife->macaddr[5], ife->mtu,
        ife->itf_flags, atomic_read(&ife->refcnt), ife->state);
}


/********************************************************************************
 Function:		static struct interface_entry *__nfp_interface_add_hash_table_entry(const struct interface_entry *entry)
 Description:	����½ڵ㵽hash����
 Data Accessed:
 Data Updated:
 Input:			Ҫ��ӵ�struct interface_entry 	�ṹ��ָ��
 Output:
 Return:		NULL==ʧ�ܣ���ӵ�struct interface_entryָ��==�ɹ�
 Others:
*********************************************************************************/
static struct interface_entry *__nfp_interface_add_hash_table_entry(const struct interface_entry *entry)
{
    struct interface_entry *new_entry = NULL;
    int hash_key;

    /* �����µĽڵ㣬�ڵ���ڼ����ڵ��ñ�����֮ǰ */
    new_entry = kmem_cache_alloc(nfp_interface_cache, GFP_ATOMIC);
    if(!new_entry){
        printk("kmalloc new interface_entry failed\n");
        goto out;
    }

    memcpy(new_entry, entry, sizeof(struct interface_entry));
    memset(&(new_entry->list), 0, sizeof(struct list_head));
    atomic_set(&new_entry->refcnt, 1);
    hash_key = hash_interface(new_entry->ifindex);
    list_add(&new_entry->list, &nfp_interface_table[hash_key]);

    NFP_ADAPTER_DEBUG("add hash node hash_key = %d\n", hash_key);
    nfp_interface_print(entry);

    nfp_interface_entry_count++;

out:
    return new_entry;
}

static struct interface_entry * __nfp_interface_register(const struct interface_entry *new_entry)
{
    int ret = 0;
    struct interface_entry *entry = NULL;

    entry = __nfp_interface_add_hash_table_entry(new_entry);
    if(!entry)
    {
        NFP_ADAPTER_DEBUG("Add hash entry failed\n");
        goto out;
    }

    ret = nfp_interface_option(NFP_OPT_ADD, entry);
    if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
    {
        NFP_ADAPTER_ERROR("call nfp_interface_option(NFP_OPT_ADD) error:%d\n", ret);
        goto out;
    }

out:
    return entry;
}

static int nfp_interface_unregister(const struct interface_entry *new_entry)
{
    int ret = 0;
    struct interface_entry *entry = NULL;

    write_lock(&nfp_interface_lock);

    entry = __nfp_interface_entry_get_by_index(new_entry->ifindex);

    if(!entry){
        NFP_ADAPTER_DEBUG("get interface fail, [Dev=%s] [index=%d]\n", new_entry->ifname, new_entry->ifindex);
        goto out;
    }
    else
    {
        __nfp_interface_release(entry);

        ret = nfp_interface_option(NFP_OPT_DELETE, entry);
        if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
        {
            NFP_ADAPTER_ERROR("call nfp_interface_option(NFP_OPT_DELETE) error:%d\n", ret);
            goto out;
        }
    }

out:
    write_unlock(&nfp_interface_lock);

    return ret;
}

static void nfp_interface_up(struct interface_entry *new_entry)
{
    //int ret = 0;
    struct interface_entry *entry = NULL;

    write_lock(&nfp_interface_lock);
    entry = __nfp_interface_entry_get_by_index(new_entry->ifindex);

    if(!entry){
        if(!__nfp_interface_register(new_entry)){
            NFP_ADAPTER_DEBUG("get interface fail, [Dev=%s] [index=%d]\n", new_entry->ifname, new_entry->ifindex);
            goto out;
        }
    }
    else
    {
        /*�ӿ�UPʱMTU�����Ѿ��仯*/
        nfp_interface_change_mtu(new_entry);

        /*����״̬�仯����Ӧ������������*/
        entry->state = new_entry->state;
    }

out:
    write_unlock(&nfp_interface_lock);

    return;
}

static void nfp_interface_down(struct interface_entry *new_entry)
{
    //int ret = 0;
    struct interface_entry *entry = NULL;

    write_lock(&nfp_interface_lock);
    entry = __nfp_interface_entry_get_by_index(new_entry->ifindex);

    if(!entry){
        NFP_ADAPTER_DEBUG("get interface fail, [Dev=%s] [index=%d]\n", new_entry->ifname, new_entry->ifindex);
        goto out;
    }
    else
    {
        /*����״̬�仯����Ӧ������������*/
        entry->state = new_entry->state;
    }

out:

    write_unlock(&nfp_interface_lock);

    return;
}


static void nfp_interface_change_addr(struct interface_entry *new_entry)
{
    int ret;
    struct interface_entry *entry = NULL;

    write_lock(&nfp_interface_lock);

    entry = __nfp_interface_entry_get_by_index(new_entry->ifindex);

    if(!entry){
        NFP_ADAPTER_DEBUG("get interface fail, [Dev=%s] [index=%d]\n", new_entry->ifname, new_entry->ifindex);
        goto out;
    }
    else
    {
        /*����״̬�仯����Ӧ������������*/
        memcpy(entry->macaddr, new_entry->macaddr, new_entry->macaddr_len);

        ret = nfp_interface_option(NFP_OPT_CHANGE_ADDR, new_entry);
        if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
        {
        	NFP_ADAPTER_ERROR("change interface addr fail, [Dev=%s] [index=%d]\n", new_entry->ifname, new_entry->ifindex);
        	goto out;
        }
    }
out:
    write_unlock(&nfp_interface_lock);

    return;
}

static void nfp_interface_change_name(struct interface_entry *new_entry)
{
    struct interface_entry *entry = NULL;

    write_lock(&nfp_interface_lock);

    entry = __nfp_interface_entry_get_by_index(new_entry->ifindex);

    if(!entry){
        NFP_ADAPTER_DEBUG("get interface fail, [Dev=%s] [index=%d]\n", new_entry->ifname, new_entry->ifindex);
        goto out;
    }
    else
    {
        /*����״̬�仯����Ӧ������������*/
        strncpy(entry->ifname, new_entry->ifname, sizeof(entry->ifname));
    }

out:
    write_unlock(&nfp_interface_lock);

    return;
}

static void nfp_interface_change_mtu(struct interface_entry *new_entry)
{
    int ret = NFP_NO_ERR;
    struct interface_entry *entry = NULL;

    write_lock(&nfp_interface_lock);

    entry = __nfp_interface_entry_get_by_index(new_entry->ifindex);
    if(!entry){
        NFP_ADAPTER_DEBUG("get interface fail, [Dev=%s] [index=%d]\n", new_entry->ifname, new_entry->ifindex);
        goto out;
    }
    else
    {
        /*����״̬�仯����Ӧ������������*/
        entry->mtu = new_entry->mtu;

        ret = nfp_interface_option(NFP_OPT_CHANGE_MTU, new_entry);
        if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
        {
        	NFP_ADAPTER_ERROR("change interface mtu fail, [Dev=%s] [index=%d]\n", new_entry->ifname, new_entry->ifindex);
        	goto out;
        }
    }

out:
    write_unlock(&nfp_interface_lock);

    return ;
}


#if defined(CONFIG_TBSMOD_DUMMYPORT)
static void nfp_interface_dummyport_bind(struct interface_entry *new_entry)
{
    int ret;
    struct interface_entry *hash_entry = NULL;
    struct interface_entry *phy_if_entry = NULL;

    read_lock(&nfp_interface_lock);

    NFP_ADAPTER_DEBUG("interface: %s, itf_flags: 0x%08x, phys_if_index: %d\n",
        new_entry->ifname, new_entry->itf_flags, new_entry->phys_ifindex);

    hash_entry = __nfp_interface_entry_get_by_index(new_entry->ifindex);

    if(NULL == hash_entry){ /* �ӿڲ��ڹ�ϣ����,�½�Node */
        NFP_ADAPTER_DEBUG("hash_entry is NULL\n");

        goto out;
    }

    if(new_entry->itf_flags & ITF_BIND_PHY_ITF){

        /* ֻά��WAN��Ĺ�ϵ��������LAN�� */
        hash_entry->itf_flags |= ITF_BIND_PHY_ITF;
        hash_entry->itf_flags |= new_entry->itf_flags & (ITF_TYPE_WAN | ITF_TYPE_LAN);

        /*�Ҳ�����Ӧ������ӿ�*/
        phy_if_entry = __nfp_interface_entry_get_by_index(new_entry->phys_ifindex);
        if(!phy_if_entry){
            NFP_ADAPTER_DEBUG("Fail to get phy_interface by index %d\n", new_entry->phys_ifindex);
            goto out;
        }

        if (new_entry->itf_flags & ITF_TYPE_WAN)
        {
            /*�Ѿ��󶨹�*/
            //if(hash_entry->phys_ifindex != 0) {
            if(0 != hash_entry->phys_ifindex){

                /*ǰ��󶨵���ͬ�Ľ�ڣ�����ϵİ󶨹�ϵ*/
                if(hash_entry->phys_ifindex != new_entry->phys_ifindex )
                {

                    /*����µİ󶨹�ϵ*/
                    hash_entry->phys_ifindex = phy_if_entry->ifindex;

                    if(new_entry->itf_flags & ITF_NFP_ENABLE) {
                        hash_entry->itf_flags |= ITF_NFP_ENABLE;
                    }
                    else {
                        hash_entry->itf_flags &= ~ITF_NFP_ENABLE;
                    }
                }
                /*�󶨵���ͬ�ӿ�*/
                else {
                    if(!(hash_entry->itf_flags & ITF_NFP_ENABLE) &&
                        (new_entry->itf_flags & ITF_NFP_ENABLE)) {

                        hash_entry->itf_flags |= ITF_NFP_ENABLE;
                    }
                    else if((hash_entry->itf_flags & ITF_NFP_ENABLE) &&
                        !(new_entry->itf_flags & ITF_NFP_ENABLE)) {

                        hash_entry->itf_flags &= ~ITF_NFP_ENABLE;

                    }
                    else {
                        /*nothing to do*/
                    }
                }
            }
            /*δ�󶨹�*/
            else {
                /*����µİ󶨹�ϵ*/
                hash_entry->phys_ifindex = phy_if_entry->ifindex;

                if(new_entry->itf_flags & ITF_NFP_ENABLE) {

                    hash_entry->itf_flags |= ITF_NFP_ENABLE;
                }
                else {
                    hash_entry->itf_flags &= ~ITF_NFP_ENABLE;
                }
            }
        }
        /*LAN��dummyport*/
        else {
            if(new_entry->itf_flags & ITF_NFP_ENABLE) {
                hash_entry->itf_flags |= ITF_NFP_ENABLE;
                hash_entry->phys_ifindex = phy_if_entry->ifindex;
            }
            else {
                hash_entry->itf_flags &= ~ITF_NFP_ENABLE;
                hash_entry->phys_ifindex = 0;
            }
        }

        /*dummyport bind for tbs nfp*/
        ret = nfp_interface_option(NFP_OPT_DUMMYPORT_BIND, hash_entry);
        if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
        {
            NFP_ADAPTER_ERROR("call nfp_interface_option(NFP_OPT_DUMMYPORT_BIND) error:%d\n", ret);
            goto out;
        }
    }
    else{
        /*dummyport unbind for tbs nfp*/
        ret = nfp_interface_option(NFP_OPT_DUMMYPORT_UNBIND, hash_entry);
        if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
        {
            NFP_ADAPTER_ERROR("call nfp_interface_option(NFP_OPT_DUMMYPORT_UNBIND) error:%d\n", ret);
            goto out;
        }

        /*����󶨹�ϵ*/
        hash_entry->phys_ifindex = 0;

        hash_entry->itf_flags &= ~ITF_BIND_PHY_ITF;
        hash_entry->itf_flags &= ~(new_entry->itf_flags & (ITF_TYPE_WAN | ITF_TYPE_LAN));
        hash_entry->itf_flags &= ~ITF_NFP_ENABLE;
    }

out:
    read_unlock(&nfp_interface_lock);

    return;
}
#endif  /* CONFIG_TBSMOD_DUMMYPORT */

/********************************************************************************
 Function:		static struct interface_entry *__nfp_interface_malloc_entry(struct net_device *ndev)
 Description:	����½ӿڵ�ϵͳ��
 Data Accessed:
 Data Updated:
 Input:			Ҫ��ӵ�struct net_device 	�ṹ��ָ��
 Output:
 Return:		NULL==ʧ�ܣ���ӵ�struct interface_entryָ��==�ɹ�
 Others:
*********************************************************************************/
static struct interface_entry *__nfp_interface_malloc_entry(struct net_device *ndev,
    unsigned long event)
{
    struct interface_entry *ie = NULL;
    struct vlan_dev_info *vdi = NULL;
    //struct net_device *real_dev = NULL;
    struct net_bridge *br = NULL;

    //struct net_bridge_port *p = br_port_get_rcu(ndev);

#if defined(CONFIG_PPPOE) || defined(CONFIG_PPPOE_MODULE)
    const char *phys_ifname = NULL;
    struct net_device *phys_dev = NULL;
#endif

#if defined(CONFIG_VNET)
    struct vnet_dev_info *vnet_info = NULL;
#endif

#if defined(CONFIG_TBSMOD_DUMMYPORT)
    struct dummyport *dummyport_info  =NULL;
#endif  /* CONFIG_TBSMOD_DUMMYPORT */

    NFP_ADAPTER_DEBUG("dev = %s, flags = %08x\n",
                    ndev->name, ndev->priv_flags);

    NFP_ASSERT(ndev);
    ie = kmem_cache_alloc(nfp_interface_cache, GFP_ATOMIC);
    if(NULL == ie){
        goto out;
    }
    else{
        memset(ie, 0, sizeof(struct interface_entry));
        ie->ifindex = ndev->ifindex;
        ie->phys_ifindex = ndev->ifindex;
        memcpy(ie->ifname, ndev->name, IFNAMSIZ);
        if(ndev->dev_addr){
            memcpy(ie->macaddr, ndev->dev_addr, ndev->addr_len);
            ie->macaddr_len = ndev->addr_len;
        }

        ie->type = ndev->type;
        ie->mtu = ndev->mtu;

        /* ��̫�� */
        if(ndev->priv_flags & IFF_ETH){
            ie->itf_flags |= ITF_TBS_ETH;
            ie->phys_ifindex = 0;
        }

        /* WLAN�� */
        else if(ndev->priv_flags & IFF_WLAN){
            ie->itf_flags |= ITF_TBS_WLAN;
            ie->phys_ifindex = 0;
        }

        /* �Žӿ� */
        else if(ndev->priv_flags & IFF_EBRIDGE){
            //br = ndev->br_port->br;
            br = br_port_get_rcu(ndev)->br;
            ie->itf_flags |= ITF_BRIDGE;
            ie->phys_ifindex = 0;
        }

        /* VLAN�ӿ� */
        else if(ndev->priv_flags & IFF_802_1Q_VLAN){
            vdi = netdev_priv(ndev);
            ie->itf_flags |= ITF_VLAN;
            ie->phys_ifindex = vdi->real_dev->ifindex;
            ie->vlan_id = vdi->vlan_id;
        }

        /* ATM�� */
        else if(ndev->priv_flags & IFF_NAS){
            ie->itf_flags |= ITF_TBS_NAS;
            ie->phys_ifindex = 0;
        }

        /* PTM�� */
        else if(ndev->priv_flags & IFF_PTM){
            ie->itf_flags |= ITF_TBS_PTM;
            ie->phys_ifindex = 0;
        }

        /* DUMMYPORT�ӿ� */
#if defined(CONFIG_TBSMOD_DUMMYPORT)
        else if(ndev->priv_flags & IFF_DUMMYPORT){
            dummyport_info = netdev_priv(ndev);
            ie->itf_flags |= ITF_DUMMYPORT;
            ie->phys_ifindex = 0;
            if(dummyport_info->bind_enable){
                ie->itf_flags |= ITF_BIND_PHY_ITF;

                /*mod by pengyao 20111205 for tbs nfp*/
            #ifndef CONFIG_TBS_NFP
                if(DUMMYPORT_WAN & dummyport_info->dummyport_flag){
                    real_dev = dev_get_by_name(&init_net, dummyport_info->phys_dev);
                    if(real_dev){
                        ie->phys_ifindex = real_dev->ifindex;
                        dev_put(real_dev);
                        real_dev = NULL;
                    }

                    ie->itf_flags |= ITF_TYPE_WAN;
                }
                else{
                    ie->itf_flags |= ITF_TYPE_LAN;
                }
            #else
                real_dev = dev_get_by_name(&init_net, dummyport_info->phys_dev);
                if(real_dev){
                    ie->phys_ifindex = real_dev->ifindex;
                    dev_put(real_dev);
                    real_dev = NULL;
                }

                ie->itf_flags |= ITF_TYPE_WAN;
            #endif
            }
            else{
                ie->itf_flags &= ~ITF_BIND_PHY_ITF;
            }

            /*�ж��Ƿ�����NFP���ܣ�������ͬvlan��wan�ĳ���*/
            if(DUMMYPORT_NFP_ENABLE & dummyport_info->dummyport_flag){
                ie->itf_flags |= ITF_NFP_ENABLE;
            }
            else {
                ie->itf_flags &= ~ITF_NFP_ENABLE;
            }
        }
#endif  /* CONFIG_TBSMOD_DUMMYPORT */

        /* PPP�ӿ� NETDEV_REGISTER��NETDEV_UNREGISTER�¼�ʱ���ȡmac��session idʧ��*/
#if defined(CONFIG_PPPOE) || defined(CONFIG_PPPOE_MODULE)
        else if((ndev->flags & IFF_POINTOPOINT) &&
            (ARPHRD_PPP == ndev->type) &&
            ((NETDEV_UP | NETDEV_PPP_CHANNEL_ADD) & event)){

            ie->itf_flags |= ITF_PPPOE;
            ie->session_id = getSessIdAndAddr(ndev->name, ie->dst_macaddr);
            if(!ie->session_id)
            {
                NFP_ADAPTER_ERROR("get sessiond id and mac_addr faild ,dev = %s, flags = 0x%08x\n",
                    ndev->name, ndev->flags);
                kmem_cache_free(nfp_interface_cache, (void *)ie);

                ie = NULL;

                goto out;
            }

            phys_ifname = getPhysicalIfName(ndev->name);
            if(!phys_ifname)
            {
                NFP_ADAPTER_ERROR("get physical if name faild ,dev = %s, flags = 0x%08x\n",
                    ndev->name, ndev->flags);
                kmem_cache_free(nfp_interface_cache, (void *)ie);

                ie = NULL;

                goto out;
            }

            phys_dev = dev_get_by_name(&init_net,phys_ifname);
            if(!phys_dev)
            {
                NFP_ADAPTER_ERROR("get device faild ,dev = %s\n", phys_ifname);
                kmem_cache_free(nfp_interface_cache, (void *)ie);
                ie = NULL;

                goto out;
            }

            ie->phys_ifindex = phys_dev->ifindex;

            NFP_ADAPTER_DEBUG("malloc ppp device success, dev = %s, physic dev = %s\n",
                ndev->name, phys_ifname);
        }
#endif
        /*vnet�ӿ�*/
#if defined(CONFIG_VNET)
        else if(ndev->priv_flags & IFF_VNET){
            vnet_info = netdev_priv(ndev);
            ie->itf_flags |= ITF_TBS_VNET;
            ie->phys_ifindex = vnet_info->real_dev->ifindex;
        }
#endif  /* CONFIG_VNET */

        ie->state = ndev->state;
    }

out:
    if(phys_dev)
        dev_put(phys_dev);

    return ie;
}


/********************************************************************************
 Function:		static int nfp_interface_read_proc(char *page,
						char **start, off_t off, int count, int *eof, void *data)
 Description:	��ȡproc�ļ�ϵͳ�ڵ�
 Data Accessed:
 Data Updated:
 Input:
 Output:
 Return:		0==�ɹ� С�����ֵ��ʾ����
 Others:
*********************************************************************************/
static int nfp_interface_read_proc(char *page,
                                   char **start, off_t off, int count, int *eof, void *data)
{
    struct interface_entry *entry = NULL;
    char *out = page;
    int hash = 0;
    int len = 0;

    read_lock(&nfp_interface_lock);

    printk("ifindex\tifname\t\tphys_ifindex\tbr_ifindex\tmacaddr\t\t\tmtu\titf_flags\trefcnt\tstate\n");
    for(hash=0; hash<NFP_ITF_HTABLE_SIZE; hash++){
        list_for_each_entry(entry, &nfp_interface_table[hash], list){

            printk("%d\t%-16s%d\t\t%d\t\t%02x:%02x:%02x:%02x:%02x:%02x\t%d\t0x%08x\t%d\t0x%08x\n",
                entry->ifindex, entry->ifname, entry->phys_ifindex, entry->br_index,
                entry->macaddr[0], entry->macaddr[1],
                entry->macaddr[2], entry->macaddr[3],
                entry->macaddr[4], entry->macaddr[5], entry->mtu,
                entry->itf_flags, atomic_read(&entry->refcnt), entry->state);
        }
    }

    read_unlock(&nfp_interface_lock);

    len = out - page;
    len -= off;
    if (len < count){
        *eof = 1;
        if (len <= 0)
            return 0;
    }
    else
        len = count;
    *start = page + off;

    return len;
}


/********************************************************************************
 Function:		static void nfp_interface_del_hash_table_entry(const struct interface_entry *ie)
 Description:	ɾ��hash����ָ���Ľڵ�,���ͷ��ڴ�
 Data Accessed:
 Data Updated:
 Input:			Ҫɾ����struct interface_entry 	�ṹ��ָ��
 Output:
 Return:		��
 Others:
*********************************************************************************/
static void nfp_interface_del_hash_table_entry(const struct interface_entry *ie)
{
    struct list_head lhd = ie->list;

    nfp_interface_print(ie);
    list_del(&lhd);
    kmem_cache_free(nfp_interface_cache, (void *)ie);

    if(nfp_interface_entry_count)
        nfp_interface_entry_count--;
}

/********************************************************************************
 Function:		int nfp_interface_force_free_all_node(struct list_head *hash_table, int size)
 Description:	ǿ��ɾ��hash�������еĽڵ�,���ͷ��ڴ�
 Data Accessed:
 Data Updated:
 Input:			Ҫɾ����struct list_head *hash_table�ṹ��ָ��, hash����
 Output:
 Return:		0==�ɹ�
 Others:
*********************************************************************************/
int nfp_interface_force_free_all_node(struct list_head *hash_table, int size)
{
    int ret, hash = 0;
    struct interface_entry *ife = NULL;
    struct interface_entry *next = NULL;

    write_lock(&nfp_interface_lock);
    for(hash = 0; hash < size; hash++){
        if(list_empty(&(hash_table[hash])))
            continue;
        /*����hash_table[hash]�������������Ƴ��ڵ㣬���ͷſռ�*/
        list_for_each_entry_safe(ife, next, &hash_table[hash], list) {
            ret = nfp_interface_option(NFP_OPT_DELETE, ife);
            if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
            {
                NFP_ADAPTER_ERROR("call nfp_interface_option(NFP_OPT_DELETE) error:%d\n", ret);
            }

            nfp_interface_del_hash_table_entry(ife);
        }
    }
    write_unlock(&nfp_interface_lock);
    return 0;
}


/********************************************************************************
 Function:		int nfp_interface_get_real_ifindex(const struct interface_entry *itf)
 Description:	�ݹ������������ӿ�
 Data Accessed:
 Data Updated:
 Input:			const struct interface_entry *ie	interfaceʵ��
 Output:
 Return:		0:�����ڣ�����:����ӿ�ifindex
 Others:
********************************************************************************/
int nfp_interface_get_real_ifindex(const struct interface_entry *itf)
{
    int ifindex = 0;
    const struct interface_entry *query = NULL;
    NFP_ASSERT(itf);

    query = itf;
    read_lock(&nfp_interface_lock);

	while(1) {
		if (query->ifindex == query->phys_ifindex ||
            query->phys_ifindex == 0){
			ifindex = query->ifindex;
            break;
		}

		query = __nfp_interface_entry_get_by_index(query->phys_ifindex);
		if (query == NULL) {
            ifindex = 0;
			break;
		}

		if (query->itf_flags & ITF_VLAN)
			continue;
	}

    read_unlock(&nfp_interface_lock);

    return ifindex;
}
EXPORT_SYMBOL(nfp_interface_get_real_ifindex);


/********************************************************************************
 Function:		struct interface_entry *nfp_interface_query(int index)
 Description:	�ӿڲ�ѯ����,ͨ��index��ѯ�ӿڵĸ�����
 Data Accessed:
 Data Updated:
 Input:			int index	�ӿ�������
 Output:
 Return:		NULL:�����ڣ�����:interface_entry�ṹ��ָ��
 Others:
********************************************************************************/
struct interface_entry *nfp_interface_query(int ifindex)
{
    struct interface_entry *query = NULL;

    NFP_ADAPTER_INTO_FUNC;
    write_lock(&nfp_interface_lock);
    query = __nfp_interface_entry_get_by_index(ifindex);
    if(query){
        atomic_inc(&query->refcnt);
    }
    write_unlock(&nfp_interface_lock);

    return query;
}
EXPORT_SYMBOL(nfp_interface_query);


/********************************************************************************
 Function:		bool nfp_interface_exist(int index)
 Description:	�ӿڲ�ѯ����,ͨ��index��ѯ�ӿ��ͷŴ���
 Data Accessed:
 Data Updated:
 Input:			int index	�ӿ�������
 Output:
 Return:		NULL:�����ڣ�����:interface_entry�ṹ��ָ��
 Others:
********************************************************************************/
bool nfp_interface_exist(int ifindex)
{
    struct interface_entry *query = NULL;

    read_lock(&nfp_interface_lock);
    query = __nfp_interface_entry_get_by_index(ifindex);
    read_unlock(&nfp_interface_lock);
    if(query){
        return true;
    }

    return false;
}
EXPORT_SYMBOL(nfp_interface_exist);

/********************************************************************************
 Function:		void nfp_interface_put(struct interface_entry *ie)
 Description:	�ͷ����ü���
 Data Accessed:
 Data Updated:
 Input:			int index	�ӿ�������
 Output:
 Return:
 Others:
*********************************************************************************/
void nfp_interface_put(struct interface_entry *ie)
{
    NFP_ADAPTER_INTO_FUNC;
    write_lock(&nfp_interface_lock);
    if (atomic_dec_and_test(&ie->refcnt))
        nfp_interface_del_hash_table_entry(ie);
    write_unlock(&nfp_interface_lock);
    NFP_ADAPTER_OUT_FUNC;
}
EXPORT_SYMBOL(nfp_interface_put);


static bool nfp_interface_is_attention(const char * ifname)
{
    const char *ptr = NULL;
    const char *pos = NULL;
    int i = 0;

    while((ptr = un_care_dev[i]))
    {
        if(strstr(ifname, ptr)){
            pos = ifname + strlen(ptr);
            if('\0' == *pos ||
                (*pos >= '0' && *pos <= '9')){
                return false;
            }
        }
        i++;
    }

    return true;
}


/********************************************************************************
 Function:		unsigned int itftype_get_by_index(int ifindex)
 Description:	�ýӿ���������ѯ�ӿ�����
 Data Accessed:
 Data Updated:
 Input:			int index	�ӿ�������
 Output:
 Return:		0x000=δ�ҵ���¼ itf_flags=�ӿ����ͱ�־(���ܰ�����������)
 Others:
*********************************************************************************/
unsigned int itftype_get_by_index(int ifindex)
{
    unsigned int ret_val = 0;
    struct interface_entry *ie = NULL;

    NFP_ADAPTER_INTO_FUNC;
    read_lock(&nfp_interface_lock);
    ie = __nfp_interface_entry_get_by_index(ifindex);
    if(NULL == ie){
        NFP_ADAPTER_ERROR("Interface [ifindex=%d] not found in hash table\n", ifindex);
        ret_val &= ~ITF_MASK;
        goto out;
    }
    else
        ret_val = ie->itf_flags;

out:
    read_unlock(&nfp_interface_lock);
    return ret_val;
}
EXPORT_SYMBOL(itftype_get_by_index);



/********************************************************************************
 Function:		unsigned int nfp_interface_flush(void)
 Description:	�����·�interface���򵽼�����
 Data Accessed:
 Data Updated:
 Input:			��
 Output:
 Return:		0x000=δ�ҵ���¼ itf_flags=�ӿ����ͱ�־(���ܰ�����������)
 Others:
*********************************************************************************/
void nfp_interface_flush(void)
{
    struct interface_entry *entry = NULL;
    struct interface_entry *phys_entry = NULL;
    int ret = 0;
    int hash = 0;

    NFP_ADAPTER_INTO_FUNC;
    read_lock(&nfp_interface_lock);

    for(hash=0; hash<NFP_ITF_HTABLE_SIZE; hash++){

        list_for_each_entry(entry, &nfp_interface_table[hash], list){
            if(atomic_read(&(entry->refcnt)) >= 1)
            {
                /* for marvell nfp*/
                #if (!defined(CONFIG_TBS_NFP) && defined(CONFIG_TBSMOD_DUMMYPORT))
                /*δ������ӿڵ�dummyport����lan��dummyport�����·�����*/
                if((entry->itf_flags & ITF_DUMMYPORT) &&
                    ((entry->itf_flags & ITF_TYPE_LAN) ||
                    (((entry->itf_flags & (ITF_BIND_PHY_ITF | ITF_NFP_ENABLE)) !=
                    (ITF_BIND_PHY_ITF | ITF_NFP_ENABLE))))){

                    continue;
                }
                #endif  /* CONFIG_TBSMOD_DUMMYPORT */

                /*dummyport���˽ӿڣ�������Ӱ󶨵Ľӿ�*/
                #if defined(CONFIG_TBS_NFP)
                if((entry->itf_flags & ITF_DUMMYPORT) &&
                    (entry->itf_flags & (ITF_BIND_PHY_ITF)) &&
                    entry->phys_ifindex > 0)
                {
                    phys_entry = __nfp_interface_entry_get_by_index(entry->phys_ifindex);
                    if(phys_entry)
                    {
                        ret = nfp_interface_option(NFP_OPT_ADD, phys_entry);
                        if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
                        {
                            NFP_ADAPTER_ERROR("call nfp_interface_option(NFP_OPT_ADD) ifname: %s error:%d\n", phys_entry->ifname, ret);
                        }
                    }
                    else
                        NFP_ADAPTER_ERROR("get %s physical device fail, phys_ifindex = %d\n", entry->ifname, entry->phys_ifindex);
                }
                #endif

                ret = nfp_interface_option(NFP_OPT_ADD, entry);
                if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
                {
                    NFP_ADAPTER_ERROR("call nfp_interface_option(NFP_OPT_ADD) ifname: %s error:%d\n", entry->ifname, ret);
                }
            }
        }
    }

    /*�Žӿڲ���*/
    for(hash=0; hash<NFP_ITF_HTABLE_SIZE; hash++){
        list_for_each_entry(entry, &nfp_interface_table[hash], list){
            if(atomic_read(&(entry->refcnt)) >= 1)
            {
                if(0 == entry->br_index){
                    continue;
                }

                ret = nfp_interface_option(NFP_OPT_BRPORT_ADD, entry);
                if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
                {
                    NFP_ADAPTER_ERROR("call nfp_interface_option(NFP_OPT_BRPORT_ADD) error:%d\n", ret);
                }
            }
        }
    }

    read_unlock(&nfp_interface_lock);
}


/********************************************************************************
 Function:		static int acp_itf_event_handler(struct notifier_block *this, unsigned long event, void *ptr)
 Description:	�����¼�����
 Data Accessed:
 Data Updated:
 Input:			�¼�����event net_deviceָ��ptr
 Output:
 Return:		NOTIFY_BAD==���� NOTIFY_OK==���� NOTIFY_DONE==����ע�¼�
 Others:
*********************************************************************************/
static int acp_itf_event_handler(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct interface_entry *entry = NULL;
    struct net_device *dev = ptr;
    int ret_val = NOTIFY_OK;

    NFP_ASSERT(dev);

    if(!nfp_interface_is_attention(dev->name)){
        ret_val = NOTIFY_OK;
        goto out;
    }

    NFP_ADAPTER_DEBUG("interface: %s, event = %lu\n", dev->name, event);

    entry = __nfp_interface_malloc_entry(dev, event);
    if(!entry){
        NFP_ADAPTER_ERROR("malloc interface entry faild, devname = %s\n", dev->name);

        goto out;
    }

    switch(event) {
        case NETDEV_UP:
            entry->state |= IFF_UP;
            nfp_interface_up(entry);

            break;
        case NETDEV_DOWN:
            entry->state &= ~IFF_UP;
            nfp_interface_down(entry);

            break;
        case NETDEV_CHANGE:

            break;
        case NETDEV_REGISTER:
            /*
            Ϊ�˱���ppp�ӿ���REGISTER�¼������ʧ��
            ͳһ��NETDEV_UP�¼�����ӽӿ�
            */

            break;
        case NETDEV_UNREGISTER:
            nfp_interface_unregister(entry);

            break;
        case NETDEV_CHANGEMTU:
            nfp_interface_change_mtu(entry);

            break;
        case NETDEV_CHANGEADDR:
            nfp_interface_change_addr(entry);

            break;
        case NETDEV_CHANGENAME:
            nfp_interface_change_name(entry);

            break;

#if defined(CONFIG_TBSMOD_DUMMYPORT)
		case NETDEV_DUMMYPORT_BIND:
			nfp_interface_dummyport_bind(entry);
			break;
#endif  /* CONFIG_TBSMOD_DUMMYPORT */

#if defined(CONFIG_PPPOE) || defined(CONFIG_PPPOE_MODULE)
        case NETDEV_PPP_CHANNEL_ADD:
            if(nfp_interface_exist(dev->ifindex))
                break;

            nfp_interface_up(entry);
            break;
        case NETDEV_PPP_CHANNEL_DEL:
            nfp_interface_unregister(entry);
            break;
#endif

        case NETDEV_GOING_DOWN:
        case NETDEV_REBOOT:
        case NETDEV_FEAT_CHANGE:
        default:
            NFP_ADAPTER_DEBUG("nothing to do, [Dev=%s] [event=%lu]\n", entry->ifname, event);
            break;
    }
    ret_val = NOTIFY_OK;

    kmem_cache_free(nfp_interface_cache, (void *)entry);
    entry = NULL;

out:
    return ret_val;
}

static struct notifier_block nfp_itf_notifier = {
	.notifier_call	= acp_itf_event_handler,
	.next			= NULL,
	.priority		= 0
};


#if (defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE))
static int nfp_interface_brport_add(struct interface_entry *itf_br, struct interface_entry *itf_port)
{
    int ret = -1;

    NFP_ADAPTER_INTO_FUNC;

    if(NULL == itf_port || NULL == itf_br){
        return ret;
    }

#if defined(CONFIG_TBSMOD_DUMMYPORT)
    if(itf_port->itf_flags & (ITF_DUMMYPORT | ITF_DUMMY))
        return ret;
#endif  /* CONFIG_TBSMOD_DUMMYPORT */

    itf_port->br_index = itf_br->ifindex;
    ret = nfp_interface_option(NFP_OPT_BRPORT_ADD, itf_port);
    if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
    {
        NFP_ADAPTER_ERROR("call nfp_interface_option(NFP_OPT_BRPORT_ADD) error:%d\n", ret);
        return ret;
    }

    return ret;
}


static int nfp_interface_brport_delete(struct interface_entry *itf_port)
{
    int ret = -1;

    NFP_ADAPTER_INTO_FUNC;

    if(NULL == itf_port)
        return ret;

#if defined(CONFIG_TBSMOD_DUMMYPORT)
    if(itf_port->itf_flags & (ITF_DUMMYPORT | ITF_DUMMY))
        return ret;
#endif  /* CONFIG_TBSMOD_DUMMYPORT */

    itf_port->br_index = 0;
    ret = nfp_interface_option(NFP_OPT_BRPORT_DELETE, itf_port);
    if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
    {
        NFP_ADAPTER_ERROR("call nfp_interface_option(NFP_OPT_BRPORT_DELETE) error:%d\n", ret);
        return ret;
    }

    return ret;
}


/********************************************************************************
 Function:		static int acp_itf_event_handler(struct notifier_block *this, unsigned long event, void *ptr)
 Description:	�Žӿ����ɾ���¼�������
 Data Accessed:
 Data Updated:
 Input:			�¼�����event net_deviceָ��ptr
 Output:
 Return:		NOTIFY_BAD==���� NOTIFY_OK==���� NOTIFY_DONE==����ע�¼�
 Others:
*********************************************************************************/
static int acp_brport_event_handler(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct net_device *dev = ptr;
    struct net_device *br_dev = NULL;
    int ret_val = NOTIFY_DONE;
    struct interface_entry *itf_port = NULL;
    struct interface_entry *itf_br = NULL;

    NFP_ASSERT(dev);
    NFP_ADAPTER_INTO_FUNC;

    write_lock(&nfp_interface_lock);

    itf_port = __nfp_interface_entry_get_by_index(dev->ifindex);
    if(NULL == itf_port)
    {
        NFP_ADAPTER_DEBUG("get interface entry faild by br_port_index: %d\n", dev->ifindex);
        goto out;
    }

    switch(event)
    {
        case BR_IF_ADD:
            /*�ӿ�������*/
            //if(NULL == dev->br_port){
            if(NULL == br_port_get_rcu(dev)){
                NFP_ADAPTER_DEBUG("dev->br_port is NULL, devname = %s\n", dev->name);
                goto out;
            }

            //br_dev = dev->br_port->br->dev;
            br_dev = br_port_get_rcu(dev)->br->dev;
            itf_br = __nfp_interface_entry_get_by_index(br_dev->ifindex);
            if(NULL == itf_port)
            {
                NFP_ADAPTER_DEBUG("get interface entry faild by br_index: %d\n", br_dev->ifindex);
                goto out;
            }

            ret_val = nfp_interface_brport_add(itf_br, itf_port);
            break;

        case BR_IF_DELETE:
            if(0 == itf_port->br_index)
            {
                NFP_ADAPTER_DEBUG("itf_port->ifname = %s, itf_port->br_index = %d\n",
                    itf_port->ifname, itf_port->br_index);
                goto out;
            }

            br_dev = __dev_get_by_index(dev_net(dev), itf_port->br_index);
            NFP_ADAPTER_DEBUG("__dev_get_by_index(%d) fail\n\n",
                itf_port->br_index);

            itf_br = __nfp_interface_entry_get_by_index(itf_port->br_index);

            ret_val = nfp_interface_brport_delete(itf_port);
            break;
        default:
            break;
    };

    /*bridge mac��ַ���ܱ��*/
    if(NULL != br_dev && NULL != itf_br &&
        compare_ether_addr(br_dev->dev_addr, itf_br->macaddr))
    {
        memcpy(itf_br->macaddr, br_dev->dev_addr, sizeof(itf_br->macaddr));

        ret_val = nfp_interface_option(NFP_OPT_CHANGE_ADDR, itf_br);
        if(NFP_UNABLE_PARSER != ret_val && NFP_NO_ERR != ret_val)
        {
            NFP_ADAPTER_ERROR("change interface addr fail, [Dev=%s] [index=%d]\n", itf_br->ifname, itf_br->ifindex);
            goto out;
        }
    }

out:
    write_unlock(&nfp_interface_lock);

    return NOTIFY_OK;
}

static struct notifier_block nfp_brport_notifier = {
	.notifier_call	= acp_brport_event_handler,
	.next			= NULL,
	.priority		= 0
};
#endif


__init int nfp_interface_init(void)
{
    struct proc_dir_entry *pfile = NULL;
    int ret_val = -1;
    NFP_ADAPTER_INTO_FUNC;

    rwlock_init(&nfp_interface_lock);

	nfp_interface_cache = kmem_cache_create("nfp_itf_cache",
					 sizeof(struct interface_entry),
					 0,
					 SLAB_HWCACHE_ALIGN, NULL);
	if (!nfp_interface_cache){
        NFP_ADAPTER_ERROR("Fail to kmem hash cache\n");
        ret_val = -ENOMEM;
        goto err1;
    }

    /*ע�ᵽnetdev_chain֪ͨ��*/
    ret_val = register_netdevice_notifier(&nfp_itf_notifier );
    if (ret_val < 0) {
        NFP_ADAPTER_ERROR("nfp_interface_init: register nfp_itf_notifier to netdev_chain failed\n");
        goto err2;
    }

    /*ע�ᵽbr_if_chain֪ͨ��*/
    ret_val = register_br_if_notifier(&nfp_brport_notifier);
    if (ret_val < 0) {
        NFP_ADAPTER_ERROR("nfp_interface_init: register nfp_brport_notifier to br_if_chain failed\n");
        goto err3;
    }

    /*ע��proc�ļ�ϵͳĿ¼*/
    pfile = create_proc_entry("nfp_interfaces", 0444, nfp_adapter_proc);
    if (NULL == pfile){
        NFP_ADAPTER_ERROR("Create nfp_interface proc entry failed!\n");
        ret_val = -ENOMEM;
        goto err4;
    }
    else{
        pfile->read_proc = &nfp_interface_read_proc;
    }
    NFP_ADAPTER_OUT_FUNC;

    return ret_val;

err4:
    unregister_br_if_notifier(&nfp_brport_notifier);

err3:
    unregister_netdevice_notifier(&nfp_itf_notifier);

err2:
    kmem_cache_destroy(nfp_interface_cache);
err1:
    return ret_val;
}

__exit int nfp_interface_exit(void)
{
    /* �˳�ʱɾ��proc/net/nfp_interfaces���������нڵ� */
    remove_proc_entry("nfp_interfaces", nfp_adapter_proc);

    /*����ע��br_if_chain֪ͨ��*/
    unregister_br_if_notifier(&nfp_brport_notifier);

    /* �˳�ʱ����ע��֪ͨ�� */
    unregister_netdevice_notifier(&nfp_itf_notifier );

    /* �˳�ʱǿ��ɾ�����еĹ�ϣ��ڵ� */
    nfp_interface_force_free_all_node(nfp_interface_table, NFP_ARRAY_SIZE(nfp_interface_table));

    kmem_cache_destroy(nfp_interface_cache);
    return 0;
}

