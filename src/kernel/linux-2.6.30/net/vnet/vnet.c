
#include <asm/uaccess.h> /* for copy_from_user */
#include <linux/capability.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <net/datalink.h>
#include <linux/mm.h>
#include <linux/in.h>
#include <linux/init.h>
#include <net/p8022.h>
#include <net/arp.h>
#include <linux/rtnetlink.h>
#include <linux/notifier.h>
#include <linux/flash_layout_kernel.h>
#include <linux/if_vnet.h>
#include "vnet.h"
#include "nfmark.h"

static int vnet_device_event(struct notifier_block *unused, unsigned long event, void *ptr);
static int vnet_ioctl_handler(void __user *arg);
static int vnet_skb_recv(struct sk_buff *skb, struct net_device *dev);
static void do_unregister_vnet_device(struct net_device *dev);

/*
static struct packet_type vnet_packet_type = {
	.type = __constant_htons(ETH_P_ALL),
	.func = vnet_skb_recv,
};
*/

static struct notifier_block vnet_notifier_block = {
	.notifier_call = vnet_device_event,
};

static unsigned int vnet_index_bitmap = 0;

/* Our listing of real device group(s) */
static struct hlist_head real_dev_hash[REAL_DEV_HASH_SIZE];
#define real_dev_hashfn(IDX)	((((IDX) >> REAL_DEV_HASH_SHIFT) ^ (IDX)) & REAL_DEV_HASH_MASK)

int get_vnet_index(void)
{
	unsigned int i;
	for(i=0; i<MAX_VNET_NUM; i++) {
		if((vnet_index_bitmap & (0x1<<i)) == 0) {
			vnet_index_bitmap |= (0x01<<i);
			return i;
		}
	}
    return -1;
}

void free_vnet_index(unsigned int index)
{
	if(index>=MAX_VNET_NUM)
		return;
	vnet_index_bitmap &= (~(0x01<<index));
}

static void vnet_set_real_dev_promiscuity(struct net_device *dev,int inc)
{
	/* Increment our in-use promiscuity counter */
	dev_set_promiscuity(dev, inc);
	if(inc > 0)
		printk(KERN_NOTICE "VNET: Setting underlying device(%s) to promiscious mode.\n", dev->name);
}

static unsigned char mac_used[MAX_VNET_NUM] = {0};
static void vnet_set_dev_addr(struct net_device *vnet, struct net_device *real_dev)
{
    unsigned char macaddr[ETH_ALEN];
    int i = 0, err = 0;

    memcpy(vnet->broadcast, real_dev->broadcast, real_dev->addr_len);
    vnet->addr_len = real_dev->addr_len;

    /*set mac address for new device*/
	for(i = 0; i < MAX_VNET_NUM; i++) {
		if(!mac_used[i]) {
			memset(macaddr, 0, ETH_ALEN);
			err = tbs_read_mac(WAN_MAC, i, macaddr);
			if (!err) {
				memcpy(vnet->dev_addr, macaddr, ETH_ALEN);
				VNET_DEV_INFO(vnet)->mac_flag = i;
				mac_used[i] = 1;
			}
			break;
		}
	}

	if (i == MAX_VNET_NUM)
	printk(KERN_ALERT "The number of vnet device is more than %d\n", MAX_VNET_NUM);
	vnet_set_real_dev_promiscuity(real_dev, 1);
}

/* Must be invoked with RCU read lock (no preempt) */
static struct vnet_real_dev *__vnet_find_real_dev(int real_dev_ifindex)
{
	struct vnet_real_dev *real_dev;
	struct hlist_node *n;
	int hash = real_dev_hashfn(real_dev_ifindex);

	hlist_for_each_entry_rcu(real_dev, n, &real_dev_hash[hash], hlist) {
		if (real_dev->real_dev_ifindex == real_dev_ifindex)
			return real_dev;
	}
	return NULL;
}

static struct vnet_real_dev * __alloc_real_dev(int real_dev_ifindex)
{
	struct vnet_real_dev *real_dev;

	real_dev = kzalloc(sizeof(struct vnet_real_dev), GFP_KERNEL);
	if (!real_dev)
		return NULL;

	real_dev->real_dev_ifindex = real_dev_ifindex;
	hlist_add_head_rcu(&real_dev->hlist,
					   &real_dev_hash[real_dev_hashfn(real_dev_ifindex)]);
	return real_dev;
}

static void vnet_real_dev_rcu_free(struct rcu_head *rcu)
{
	VNET_PRINTK("\n");
	kfree(container_of(rcu, struct vnet_real_dev, rcu));
}

void __vnet_free_real_dev(struct vnet_real_dev *dev)
{
	VNET_PRINTK("\n");
	hlist_del_rcu(&dev->hlist);
	call_rcu(&dev->rcu, vnet_real_dev_rcu_free);
}

static int real_dev_add_vnet(int real_dev_ifindex, struct net_device *vnet)
{
	struct vnet_real_dev *real_dev = __vnet_find_real_dev(real_dev_ifindex);
	if(NULL==real_dev) {
		real_dev = __alloc_real_dev(real_dev_ifindex);
		if(real_dev==NULL)
			return -ENOMEM;
	}

	/* TBS_TAG: by user 2009-6-18 修改WAN接口删除时导致kernel panic的问题 */
    ASSERT_RTNL();
	if(real_dev->vnet_cnt >= MAX_VNET_NUM)
		return -ENOMEM;

	real_dev->vnet_dev[real_dev->vnet_cnt] = vnet;
	real_dev->vnet_cnt++;
	return 0;
}

static int real_dev_del_vnet(struct net_device *real_dev, struct net_device *vnet)
{
	struct vnet_real_dev *dev;
	int i=0;

    ASSERT_RTNL();
	dev = __vnet_find_real_dev(real_dev->ifindex);
	if(NULL == dev) {
		VNET_PRINTK("WARNING: real_dev==NULL.\n");
		return 0;
	}
	if(dev->vnet_cnt==0) {
		VNET_PRINTK("WARNING: dev->vnet_cnt==0 \n");
		return 0;
	}

	for(i=0; i < dev->vnet_cnt; i++) {
		if (dev->vnet_dev[i] == vnet) {
			dev->vnet_dev[i] = NULL;
    	 	/*非列表的最后一个非空元素,需要把最后的元素挪到
    	   	  删除的元素上形成连续列表 */
			if(dev->vnet_cnt > 1 && (i != dev->vnet_cnt-1))
			{
				dev->vnet_dev[i] = dev->vnet_dev[dev->vnet_cnt-1];
				dev->vnet_dev[dev->vnet_cnt-1]=NULL;
			}
			dev->vnet_cnt--;
			if (dev->vnet_cnt==0)
				__vnet_free_real_dev(dev);
			return 0;
		}
	}

	VNET_PRINTK("Can not find vnet device\n");
	return -1;
}

static void vnet_transfer_operstate(const struct net_device *dev, struct net_device *vdev)
{
	/* Have to respect userspace enforced dormant state
	 * of real device, also must allow supplicant running
	 * on VLAN device
	 */
	if (dev->operstate == IF_OPER_DORMANT)
		netif_dormant_on(vdev);
	else
		netif_dormant_off(vdev);

	if (netif_carrier_ok(dev)) {
		if (!netif_carrier_ok(vdev))
			netif_carrier_on(vdev);
	} else {
		if (netif_carrier_ok(vdev))
			netif_carrier_off(vdev);
	}
}

static int vnet_device_event(struct notifier_block *unused,
							 unsigned long event, void *ptr)
{
	struct net_device *dev = ptr;
	int i, flgs, num;
	struct net_device *vnetdev;
	struct vnet_real_dev *real_dev;
    struct net_device *phy_dev = NULL;
    //LIST_HEAD(list);

    ASSERT_RTNL();

	real_dev = __vnet_find_real_dev(dev->ifindex);
	if(!real_dev)
	{
		return NOTIFY_DONE;
	}
	num = real_dev->vnet_cnt;

	/* It is OK that we do not hold the group lock right now,
	 * as we run under the RTNL lock.
	 */

	switch (event) {
		case NETDEV_CHANGE:
			/* Propagate real device state to VNET devices */
			for (i = 0; i <num; i++) {
				if(real_dev->vnet_dev[i])
					vnet_transfer_operstate(dev, real_dev->vnet_dev[i]);
			}
			break;
		case NETDEV_DOWN:
			for (i = 0; i < num; i++) {
				vnetdev = real_dev->vnet_dev[i];
				if(vnetdev) {
					flgs = vnetdev->flags;
					if (!(flgs & IFF_UP))
						continue;
					dev_change_flags(vnetdev, flgs & ~IFF_UP);
				}
			}
			vnet_fdb_del_by_real_dev(dev, 0);
			break;
		case NETDEV_UP:
			/* Put all VNETs for this dev in the up state too.  */
			for (i = 0; i < num; i++){
				vnetdev = real_dev->vnet_dev[i];
				if (vnetdev) {
					flgs = vnetdev->flags;
					if (flgs & IFF_UP)
						continue;
					dev_change_flags(vnetdev, flgs | IFF_UP);
				}
			}
			break;
		case NETDEV_UNREGISTER:
			/* Delete all VNETs for this dev. */
			for (i = 0; i < num; i++){
				vnetdev = real_dev->vnet_dev[i];
				if (vnetdev) {
                    phy_dev = VNET_DEV_INFO(vnetdev)->real_dev;

					free_vnet_index(VNET_DEV_INFO(vnetdev)->vnet_index);
					vnet_fdb_delete_by_port(vnetdev);
					mac_used[VNET_DEV_INFO(vnetdev)->mac_flag] = 0;

                    //unregister_netdevice_queue(vnetdev, &list);	
					unregister_netdevice(vnetdev);
                    dev_put(phy_dev);
				}
			}

            //unregister_netdevice_many(&list);
			vnet_fdb_del_by_real_dev(dev, 2);
			__vnet_free_real_dev(real_dev);
			break;
	};

	return NOTIFY_DONE;
}

extern void vnet_ioctl_set(int (*hook)(void __user *));
extern int (*vnet_handle)(struct sk_buff *skb, struct net_device *dev);

static int __init vnet_init(void)
{
	int err;

	/* proc file system initialization */
	err = vnet_proc_init();
	if (err < 0) {
		printk(KERN_ERR "%s %s: can't create entry in proc filesystem!\n",
		       __FUNCTION__, VNET_NAME);
		return err;
	}

	vnet_fdb_init();
	/* Register us to receive netdevice events */
	err = register_netdevice_notifier(&vnet_notifier_block);
	if (err < 0) {
		vnet_proc_cleanup();
		return err;
	}

	vnet_ioctl_set(vnet_ioctl_handler);
	vnet_handle = vnet_skb_recv;
	return 0;
}

/* Cleanup all vnet devices
 * Note: devices that have been registered that but not
 * brought up will exist but have no module ref count.
 */
/*
 * TBS_TAG by baiyonghui 2011-3-28
 * Desc:
 */
#if 0
static void __exit vnet_cleanup_devices(void)
{
	struct net_device *dev, *nxt;
	struct net *net = &init_net;
	rtnl_lock();
/*
	for (dev = base_dev; dev; dev = nxt) {
		nxt = dev->next;
		if (dev->priv_flags & IFF_VNET) {
			do_unregister_vnet_device(dev);
		}
	}
	*/
	for_each_netdev(net, dev)
	{
		if (dev->priv_flags & IFF_VNET) {
			//do_unregister_vnet_device(dev);
			//wait ??
		}

	}

	rtnl_unlock();
}
#else

static void vnet_exit(struct net *net)
{
	struct net_device *dev;

	rtnl_lock();
restart:
	for_each_netdev(net, dev)
	{
		if (dev->priv_flags & IFF_VNET) {
			do_unregister_vnet_device(dev);
			goto restart;
		}
	}
	rtnl_unlock();
}
static struct pernet_operations vnet_ops = {
	.exit   = vnet_exit,
};
#endif
/*
 * END_TBS_TAG
 */
/*
 *     Module 'remove' entry point.
 *     o delete /proc/net/router directory and static entries.
 */
static void __exit vnet_cleanup_module(void)
{

	vnet_ioctl_set(NULL);

	/* Un-register us from receiving netdevice events */
	unregister_netdevice_notifier(&vnet_notifier_block);

	//dev_remove_pack(&vnet_packet_type);
	/*
	 * TBS_TAG by baiyonghui 2011-3-28
	 * Desc: supported for linux-2.6.30
	 */
	//vnet_cleanup_devices();
unregister_pernet_subsys(&vnet_ops);
/*
 * TBS_END_TAG
 */
	vnet_proc_cleanup();

	vnet_fdb_fini();

	synchronize_net();
	vnet_handle = NULL;
}

module_init(vnet_init);
module_exit(vnet_cleanup_module);

static void vnet_pass_frame_up(struct net_device *vnet, struct sk_buff *skb)
{
    struct net_device_stats *stats = vnet_dev_get_stats(vnet);

	stats->rx_packets++;
	stats->rx_bytes += skb->len;

	skb->dev = vnet;
	netif_rx(skb);
}


static int vnet_skb_recv(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats;
	unsigned char *dst_mac = eth_hdr(skb)->h_dest;
	struct vnet_real_dev *real_dev;
	struct net_device   *vnet;

	rcu_read_lock();

	if (dev->priv_flags & IFF_VNET)
		goto out;

    /* when is called by dev_queue_xmit_nit and by ip_dev_loopback_xmit, should return directly. */
	if (skb->pkt_type == PACKET_OUTGOING || skb->pkt_type == PACKET_LOOPBACK)
	{
        goto out;
	}

    /*802.1Q包*/
	if(skb->protocol == htons(ETH_P_8021Q)
	   /*&& dev->priv_flags != IFF_802_1Q_VLAN*/) {
		goto out;
	}

	real_dev = __vnet_find_real_dev(dev->ifindex);
	if(!real_dev)
		goto out;;

	/*广播或多播时属于同一个设备所有的VNET接口都会接收数据包*/
	if(dst_mac[0] & 0x01) {
		int i;
		struct sk_buff *new_skb;

        for(i=0; i < (real_dev->vnet_cnt-1); i++) {
            new_skb = skb_clone(skb, GFP_ATOMIC);
		    if (new_skb != NULL) {
			    vnet_pass_frame_up(real_dev->vnet_dev[i], new_skb);
		    }
        }
        vnet = real_dev->vnet_dev[real_dev->vnet_cnt-1];
	} else { /*单播包*/
		/*通过数据包的目的MAC和skb->dev,在转发表中查找对应的vnet设备*/
		if((vnet = vnet_get_device(dst_mac, skb->dev)) == NULL) {
			goto out;
		}

        if(!compare_ether_addr(dst_mac, vnet->dev_addr))
        {
            skb->pkt_type = PACKET_HOST;
        }
	}

	VNET_PRINTK("vnet_dev_name: %s\n", vnet->name);
    stats = vnet_dev_get_stats(vnet);
    stats->rx_packets++;
	stats->rx_bytes += skb->len;
	skb->dev = vnet;
	/*mark the WAN_LAN interface for the qos*/
	if (skb->dev->priv_flags & IFF_VNET)
	{
		skb->mark |= DOWN_STREAM;
	}
out:
    rcu_read_unlock();
	return 0;
}
static int vnet_dev_hard_header(struct sk_buff *skb, struct net_device *dev,
                         unsigned short type, void *daddr, void *saddr, unsigned len)
{
	struct net_device *vnetdev=dev;

	dev = VNET_DEV_INFO(dev)->real_dev;

	return dev_hard_header(skb, dev, type, daddr, saddr?saddr:vnetdev->dev_addr, len);
}
static const struct net_device_ops vnet_netdev_ops = {
	.ndo_start_xmit         = vnet_dev_hard_start_xmit,
	.ndo_get_stats          = vnet_dev_get_stats,
	.ndo_change_mtu         = vnet_dev_change_mtu,
	.ndo_open               = vnet_dev_open,
	.ndo_stop               = vnet_dev_stop,
	.ndo_set_mac_address    = vnet_dev_set_mac_address,
	.ndo_set_multicast_list = vnet_dev_set_multicast_list,
	.ndo_do_ioctl           = vnet_dev_ioctl,
	.ndo_validate_addr      = NULL,
};
#if 1
const struct header_ops vnet_header_ops ____cacheline_aligned = {
	.create		= vnet_dev_hard_header,
	.parse		= eth_header_parse,
	.rebuild	= eth_rebuild_header,
	.cache		= eth_header_cache,
	.cache_update	= eth_header_cache_update,
};
#endif

static void vnet_setup(struct net_device *new_dev)
{
	//	SET_MODULE_OWNER(new_dev);

	/* new_dev->ifindex = 0;  it will be set when added to
	 * the global list.
	 * iflink is set as well.
	 */
	/* Make this thing known as a VLAN device */
	new_dev->priv_flags |= IFF_VNET;
	new_dev->destructor = free_netdev;
	/* Set us up to have no queue, as the underlying Hardware device
	 * can do all the queueing we could want.
	 */
	new_dev->tx_queue_len = 0;
	/* set up method calls */
	new_dev->netdev_ops = &vnet_netdev_ops;
	new_dev->header_ops = &vnet_header_ops;

}


static struct net_device *register_vnet_device(const char *eth_IF_name,
											   const char *vnet_IF_name,
											   unsigned int mode)
{
	struct net_device *new_dev;
	struct net_device *real_dev; /* the ethernet device */
	int vnet_index;

	VNET_PRINTK("if_name :%s, vnet_name :%s  mode: %i\n",
				eth_IF_name, vnet_IF_name, mode);

	/* find the device relating to eth_IF_name. */
	real_dev = dev_get_by_name(&init_net, eth_IF_name);
	if (!real_dev)
		goto out_ret_null;

	VNET_PRINTK("get real dev successfully.\n");

	/* find the device relating to vnet_IF_name. */
	new_dev = dev_get_by_name(&init_net, vnet_IF_name);
	if (new_dev)
	{
        dev_put(new_dev);
		goto out_put_dev;
    }

	VNET_PRINTK("vnet Dev(%s) is not exist,can be created.\n", vnet_IF_name);
	/* From this point on, all the data structures must remain consistent.*/
	rtnl_lock();

	/* The real device must be up and operating in order to
	 * assosciate a VNET device with it.
	 */
	if (!(real_dev->flags & IFF_UP)){
		VNET_PRINTK("real_dev(%s) is not IFF_UP.\n", eth_IF_name);
		goto out_unlock;
	}

	vnet_index = get_vnet_index();
	if(vnet_index == -1)
	   goto out_unlock;
	VNET_PRINTK("get_vnet_index successfully.\n");

	new_dev = alloc_netdev(sizeof(struct vnet_dev_info),
						   vnet_IF_name, ether_setup);
	if (new_dev == NULL)
		goto out_free_index;

	vnet_setup(new_dev);

	/* IFF_BROADCAST|IFF_MULTICAST */
	new_dev->flags = real_dev->flags;
	new_dev->flags &= ~IFF_UP;
	//new_dev->priv_flags |= IFF_DOMAIN_WAN;
	new_dev->state = (real_dev->state & ((1<<__LINK_STATE_NOCARRIER) |
					     (1<<__LINK_STATE_DORMANT))) | (1<<__LINK_STATE_PRESENT);
	new_dev->features |= NETIF_F_LLTX;
	new_dev->mtu = real_dev->mtu;
	/* TODO: maybe just assign it to be ETHERNET? */
	new_dev->type = real_dev->type;
	new_dev->hard_header_len = real_dev->hard_header_len;
	/*
	 * TBS_TAG by baiyonghui 2011-3-28
	 */
//	new_dev->hard_header = vnet_dev_hard_header;
#if 0
//	new_dev->hard_start_xmit = vnet_dev_hard_start_xmit;
	new_dev->hard_header = vnet_dev_hard_header;
	new_dev->rebuild_header = real_dev->rebuild_header;
	new_dev->hard_header_parse = real_dev->hard_header_parse;
#endif
	VNET_DEV_INFO(new_dev)->mode = mode;
	VNET_DEV_INFO(new_dev)->real_dev = real_dev;
	VNET_DEV_INFO(new_dev)->vnet_index = vnet_index;

	vnet_set_dev_addr(new_dev, real_dev);
   	if (register_netdevice(new_dev))
		goto out_free_newdev;

	new_dev->iflink = real_dev->ifindex;
	vnet_transfer_operstate(real_dev, new_dev);
	linkwatch_fire_event(new_dev); /* _MUST_ call rfc2863_policy() */

	if(real_dev_add_vnet(real_dev->ifindex, new_dev)<0) {
        goto out_free_unregister;
	}

    vnet_fdb_insert(new_dev, new_dev->dev_addr);
	rtnl_unlock();
	VNET_PRINTK("Allocated new device successfully\n");
	return new_dev;

out_free_unregister:
	unregister_netdevice(new_dev);
	goto out_unlock;

out_free_newdev:
	free_netdev(new_dev);

out_free_index:
      free_vnet_index(vnet_index);

out_unlock:
	rtnl_unlock();

out_put_dev:
	dev_put(real_dev);

out_ret_null:
	return NULL;
}

static void do_unregister_vnet_device(struct net_device *dev)
{
	ASSERT_RTNL();
	real_dev_del_vnet(VNET_DEV_INFO(dev)->real_dev, dev);
	free_vnet_index(VNET_DEV_INFO(dev)->vnet_index);
	vnet_fdb_delete_by_port(dev);
	mac_used[VNET_DEV_INFO(dev)->mac_flag] = 0;
	vnet_set_real_dev_promiscuity(VNET_DEV_INFO(dev)->real_dev, -1);

    /*
    add by pengyao 20120725
    将real_dev引用计数释放从real_dev_del_vnet函数中移到此处，
    如果real_dev提前删除，这里引用real_dev指针有被释放的危险
    */
    dev_put(VNET_DEV_INFO(dev)->real_dev);
	unregister_netdevice(dev);
}

static int unregister_vnet_device(const char *vnet_IF_name)
{
	struct net_device *dev = NULL;
	int ret = -EINVAL;

	dev = dev_get_by_name(&init_net, vnet_IF_name);
	if (dev) {
		if (dev->priv_flags & IFF_VNET) {
			rtnl_lock();
			synchronize_net();
			dev_put(dev);
			do_unregister_vnet_device(dev);
			rtnl_unlock();
			VNET_PRINTK("dev %s will be removed \n", vnet_IF_name);
			ret = 0;
		} else {
			printk(VNET_ERR "%s: ERROR:	Tried to remove a non-vnet device "
			                "with vnet code, name: %s  priv_flags: %hX\n",
			                __FUNCTION__, dev->name, dev->priv_flags);
			dev_put(dev);
			ret = -EPERM;
		}
	} else {
		VNET_PRINTK("WARNING: Could not find dev: %s\n", vnet_IF_name);
		ret = -EINVAL;
	}
	return ret;
}

/*
 *	VNET IOCTL handler.
 *	execute requested action or pass command to the device driver
 *  arg is really a struct vlan_ioctl_args __user *.
 */
static int vnet_ioctl_handler(void __user *arg)
{
	int err = 0;
	struct vnet_ioctl_args args;

	if (copy_from_user(&args, arg, sizeof(struct vnet_ioctl_args)))
		return -EFAULT;

	/* Null terminate this sucker, just in case. */
	args.device1[23] = 0;
	args.device2[23] = 0;
	VNET_PRINTK("args.cmd: %x\n", args.cmd);
//	printk("%s:%s:%d:%d\n", __FILE__, __FUNCTION__, __LINE__, args.cmd);
	switch (args.cmd) {
	case ADD_VNET_CMD:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (register_vnet_device(args.device2, args.device1, args.mode)) {
			err = 0;
		} else {
			err = -EINVAL;
		}
		break;
	case DEL_VNET_CMD:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		/* Here, the args.dev1 is the actual vnet dev we want
		 * to get rid of.
		 */
		err = unregister_vnet_device(args.device1);
		break;
	default:
		printk(VNET_DBG "%s: Unknown VLAN CMD: %x \n", __FUNCTION__, args.cmd);
		return -EINVAL;
	}
	return err;
}

MODULE_LICENSE("GPL");

