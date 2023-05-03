#include <linux/module.h>
#include <linux/mm.h>
#include <linux/in.h>
#include <linux/init.h>
#include <asm/uaccess.h> /* for copy_from_user */
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/datalink.h>
#include <net/p8022.h>
#include <net/arp.h>
#include <net/ip.h>
#include <linux/if_vnet.h>
#include "vnet.h"

struct net_device_stats *vnet_dev_get_stats(struct net_device *dev)
{
	return &(VNET_DEV_INFO(dev)->dev_stats);
	//return &dev->stats;
}


int vnet_dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats = vnet_dev_get_stats(dev);

	stats->tx_packets++;
	stats->tx_bytes += skb->len;

#if 1
	/*如果源MAC地址与vnet设备的MAC地址不相同(桥模式), 则需要在转发表中
	  维护该连接相关的信息*/
	if(compare_ether_addr(skb->data+6, dev->dev_addr))
	{
        vnet_fdb_update(dev, skb->data+6);
	}
#endif

	skb->dev = VNET_DEV_INFO(dev)->real_dev;

	//printk("......dev name = %s.........\n", skb->dev->name);
	dev_queue_xmit(skb);
	return 0;
}

/*
 * TBS_TAG by baiyonghui 2011-3-29
 * Desc:
 *
 */
#if 0
int vnet_dev_hard_header(struct sk_buff *skb, struct net_device *dev,
                         unsigned short type, void *daddr, void *saddr, unsigned len)
{
      struct net_device *vnetdev=dev;

      dev = VNET_DEV_INFO(dev)->real_dev;
//
// TBS_TAG by baiyonghui 2011-3-28
//      return dev->hard_header(skb, dev, type, daddr, saddr?saddr:vnetdev->dev_addr, len);
      return dev_hard_header(skb, dev, type, daddr, saddr?saddr:vnetdev->dev_addr, len);
}
/*
 * TBS_END_TAG
 */
#endif

int vnet_dev_change_mtu(struct net_device *dev, int new_mtu)
{
	/* TODO: gotta make sure the underlying layer can handle it,
	 * maybe an IFF_VLAN_CAPABLE flag for devices?
	 */
	if (VNET_DEV_INFO(dev)->real_dev->mtu < new_mtu)
		return -ERANGE;

	dev->mtu = new_mtu;
	return 0;
}

int vnet_dev_set_mac_address(struct net_device *dev, void *addr_struct_p)
{
	struct sockaddr *addr = (struct sockaddr *)(addr_struct_p);

	if (netif_running(dev))
		return -EBUSY;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
	VNET_PRINTK("Setting dev %s MAC address to ", dev->name);
	print_mac(dev->dev_addr);

    vnet_fdb_changeaddr(dev, dev->dev_addr);
	if (!(VNET_DEV_INFO(dev)->real_dev->flags & IFF_PROMISC)) {
		int flgs = VNET_DEV_INFO(dev)->real_dev->flags;
		/* Increment our in-use promiscuity counter */
		dev_set_promiscuity(VNET_DEV_INFO(dev)->real_dev, 1);
		/* Make PROMISC visible to the user. */
		flgs |= IFF_PROMISC;
		dev_change_flags(VNET_DEV_INFO(dev)->real_dev, flgs);
	}
	return 0;
}

/*
 * TBS_TAG: by WuGuoxiang 2012-7-6
 * Desc: struct dev_mc_list was abandoned in linux2.6.30
 */
#if 0
static inline int vnet_dmi_equals(struct dev_mc_list *dmi1,
                                  struct dev_mc_list *dmi2)
{
	return ((dmi1->dmi_addrlen == dmi2->dmi_addrlen) &&
		(memcmp(dmi1->dmi_addr, dmi2->dmi_addr, dmi1->dmi_addrlen) == 0));
}

/** dmi is a single entry into a dev_mc_list, a single node.  mc_list is
 *  an entire list, and we'll iterate through it.
 */
static int vnet_should_add_mc(struct dev_mc_list *dmi, struct dev_mc_list *mc_list)
{
	struct dev_mc_list *idmi;

	for (idmi = mc_list; idmi != NULL; ) {
		if (vnet_dmi_equals(dmi, idmi)) {
			if (dmi->dmi_users > idmi->dmi_users)
				return 1;
			else
				return 0;
		} else {
			idmi = idmi->next;
		}
	}

	return 1;
}

static inline void vnet_destroy_mc_list(struct dev_mc_list *mc_list)
{
	struct dev_mc_list *dmi = mc_list;
	struct dev_mc_list *next;

	while(dmi) {
		next = dmi->next;
		kfree(dmi);
		dmi = next;
	}
}

static void vnet_copy_mc_list(struct dev_mc_list *mc_list, struct vnet_dev_info *vnet_info)
{
	struct dev_mc_list *dmi, *new_dmi;

	vnet_destroy_mc_list(vnet_info->old_mc_list);
	vnet_info->old_mc_list = NULL;

	for (dmi = mc_list; dmi != NULL; dmi = dmi->next) {
		new_dmi = kmalloc(sizeof(*new_dmi), GFP_ATOMIC);
		if (new_dmi == NULL) {
			printk(KERN_ERR "vnet: cannot allocate memory. "
			       "Multicast may not work properly from now.\n");
			return;
		}

		/* Copy whole structure, then make new 'next' pointer */
		*new_dmi = *dmi;
		new_dmi->next = vnet_info->old_mc_list;
		vnet_info->old_mc_list = new_dmi;
	}
}

static void vnet_flush_mc_list(struct net_device *dev)
{
	struct dev_mc_list *dmi = dev->mc_list;

	while (dmi) {
		printk(KERN_DEBUG "%s: del %.2x:%.2x:%.2x:%.2x:%.2x:%.2x mcast address from vnet interface\n",
		       dev->name,
		       dmi->dmi_addr[0],
		       dmi->dmi_addr[1],
		       dmi->dmi_addr[2],
		       dmi->dmi_addr[3],
		       dmi->dmi_addr[4],
		       dmi->dmi_addr[5]);
		dev_mc_delete(dev, dmi->dmi_addr, dmi->dmi_addrlen, 0);
		dmi = dev->mc_list;
	}

	/* dev->mc_list is NULL by the time we get here. */
	vnet_destroy_mc_list(VNET_DEV_INFO(dev)->old_mc_list);
	VNET_DEV_INFO(dev)->old_mc_list = NULL;

}
#endif
/*
 * TBS_END_TAG
 */

int vnet_dev_open(struct net_device *dev)
{
	if (!(VNET_DEV_INFO(dev)->real_dev->flags & IFF_UP))
		return -ENETDOWN;

	return 0;
}

int vnet_dev_stop(struct net_device *dev)
{

	//vnet_flush_mc_list(dev);
	return 0;
}

int vnet_dev_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct net_device *real_dev = VNET_DEV_INFO(dev)->real_dev;
	struct ifreq ifrr;
	int err = -EOPNOTSUPP;

	strncpy(ifrr.ifr_name, real_dev->name, IFNAMSIZ);
	ifrr.ifr_ifru = ifr->ifr_ifru;

	switch(cmd) {
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
#ifdef CONFIG_COMPAT_NET_DEV_OPS
		if (real_dev->do_ioctl && netif_device_present(real_dev))
			err = real_dev->do_ioctl(real_dev, &ifrr, cmd);
#else
		if (real_dev->netdev_ops->ndo_do_ioctl && netif_device_present(real_dev))
			err = real_dev->netdev_ops->ndo_do_ioctl(real_dev, &ifrr, cmd);
#endif
		break;
#ifdef CONFIG_ETHTOOL
	case SIOCETHTOOL:
	    err = dev_ethtool(&init_net, &ifrr);
#endif
	}

	if (!err)
		ifr->ifr_ifru = ifrr.ifr_ifru;

	return err;
}


/** Taken from Gleb + Lennert's VLAN code, and modified... */
void vnet_dev_set_multicast_list(struct net_device *vnet_dev)
{
/*
 * TBS_TAG: by WuGuoxiang 2012-7-6
 * Desc: struct dev_mc_list was abandoned in linux2.6.30
 */
#if 0
	struct dev_mc_list *dmi;
	struct net_device *real_dev;
	int inc;

	if (vnet_dev && (vnet_dev->priv_flags & IFF_VNET)) {
		/* Then it's a real vlan device, as far as we can tell.. */
		real_dev = VNET_DEV_INFO(vnet_dev)->real_dev;

		/* compare the current promiscuity to the last promisc we had.. */
		inc = vnet_dev->promiscuity - VNET_DEV_INFO(vnet_dev)->old_promiscuity;
		if (inc) {
			printk(KERN_INFO "%s: dev_set_promiscuity(master, %d)\n",
			       vnet_dev->name, inc);
			dev_set_promiscuity(real_dev, inc); /* found in dev.c */
			VNET_DEV_INFO(vnet_dev)->old_promiscuity = vnet_dev->promiscuity;
		}

		inc = vnet_dev->allmulti - VNET_DEV_INFO(vnet_dev)->old_allmulti;
		if (inc) {
			printk(KERN_INFO "%s: dev_set_allmulti(master, %d)\n",
			       vnet_dev->name, inc);
			dev_set_allmulti(real_dev, inc); /* dev.c */
			VNET_DEV_INFO(vnet_dev)->old_allmulti = vnet_dev->allmulti;
		}

		/* looking for addresses to add to master's list */
		for (dmi = vnet_dev->mc_list; dmi != NULL; dmi = dmi->next) {
			if (vnet_should_add_mc(dmi, VNET_DEV_INFO(vnet_dev)->old_mc_list)) {
				dev_mc_add(real_dev, dmi->dmi_addr, dmi->dmi_addrlen, 0);
				printk(KERN_DEBUG "%s: add %.2x:%.2x:%.2x:%.2x:%.2x:%.2x mcast address to master interface\n",
				       vnet_dev->name,
				       dmi->dmi_addr[0],
				       dmi->dmi_addr[1],
				       dmi->dmi_addr[2],
				       dmi->dmi_addr[3],
				       dmi->dmi_addr[4],
				       dmi->dmi_addr[5]);
			}
		}

		/* looking for addresses to delete from master's list */
		for (dmi = VNET_DEV_INFO(vnet_dev)->old_mc_list; dmi != NULL; dmi = dmi->next) {
			if (vnet_should_add_mc(dmi, vnet_dev->mc_list)) {
				/* if we think we should add it to the new list, then we should really
				 * delete it from the real list on the underlying device.
				 */
				dev_mc_delete(real_dev, dmi->dmi_addr, dmi->dmi_addrlen, 0);
				printk(KERN_DEBUG "%s: del %.2x:%.2x:%.2x:%.2x:%.2x:%.2x mcast address from master interface\n",
				       vnet_dev->name,
				       dmi->dmi_addr[0],
				       dmi->dmi_addr[1],
				       dmi->dmi_addr[2],
				       dmi->dmi_addr[3],
				       dmi->dmi_addr[4],
				       dmi->dmi_addr[5]);
			}
		}

		/* save multicast list */
		vnet_copy_mc_list(vnet_dev->mc_list, VNET_DEV_INFO(vnet_dev));

	}
#endif
/*
 * TBS_END_TAG
 */
}

