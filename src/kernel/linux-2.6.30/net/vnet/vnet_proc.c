/******************************************************************************
 * vnet_proc.c	VNET Module. /proc filesystem interface.
 *
 *		This module is completely hardware-independent and provides
 *		access to the router using Linux /proc filesystem.
 *
 * Author:	pangxianfeng
 *
 * Copyright:	(c)
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 * ============================================================================
 * Jan 20, 1998        Ben Greear     Initial Version
 *****************************************************************************/

#include <linux/module.h>
#include <linux/stddef.h>	/* offsetof(), etc. */
#include <linux/errno.h>	/* return codes */
#include <linux/kernel.h>
#include <linux/slab.h>		/* kmalloc(), kfree() */
#include <linux/mm.h>
#include <linux/string.h>	/* inline mem*, str* functions */
#include <linux/init.h>		/* __initfunc et al. */
#include <asm/byteorder.h>	/* htons(), etc. */
#include <asm/uaccess.h>	/* copy_to_user */
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>

#include <linux/if_vnet.h>
#include "vnet.h"

/****** Function Prototypes *************************************************/

/* Methods for preparing data for reading proc entries */
static int vnet_seq_show(struct seq_file *seq, void *v);
static void *vnet_seq_start(struct seq_file *seq, loff_t *pos);
static void *vnet_seq_next(struct seq_file *seq, void *v, loff_t *pos);
static void vnet_seq_stop(struct seq_file *seq, void *);


/* Methods for preparing data for reading proc entries */
static int vnet_fdb_seq_show(struct seq_file *seq, void *v);
static void *vnet_fdb_seq_start(struct seq_file *seq, loff_t *pos);
static void *vnet_fdb_seq_next(struct seq_file *seq, void *v, loff_t *pos);
static void vnet_fdb_seq_stop(struct seq_file *seq, void *);

/*
 *	Global Data
 */


/*
 *	Names of the proc directory entries
 */

static const char name_root[]	 = "vnet";
static const char name_conf[]	 = "config";

static const char name_mac_tbl[]	 = "mac_tbl";

/*
 *	Structures for interfacing with the /proc filesystem.
 *	VLAN creates its own directory /proc/net/vlan with the folowing
 *	entries:
 *	config		device status/configuration
 *	<device>	entry for each  device
 */

/*
 *	Generic /proc/net/vlan/<file> file and inode operations
 */

static struct seq_operations vnet_seq_ops = {
	.start = vnet_seq_start,
	.next = vnet_seq_next,
	.stop = vnet_seq_stop,
	.show = vnet_seq_show,
};

static int vnet_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &vnet_seq_ops);
}

static struct file_operations vnet_fops = {
	.owner	 = THIS_MODULE,
	.open    = vnet_seq_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};


static struct seq_operations vnet_fdb_seq_ops = {
	.start = vnet_fdb_seq_start,
	.next = vnet_fdb_seq_next,
	.stop = vnet_fdb_seq_stop,
	.show = vnet_fdb_seq_show,
};

static int vnet_fdb_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &vnet_fdb_seq_ops);
}

static struct file_operations vnet_fdb_fops = {
	.owner	 = THIS_MODULE,
	.open    = vnet_fdb_seq_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};


/*
 * Proc filesystem derectory entries.
 */

/*
 *	/proc/net/vlan
 */

static struct proc_dir_entry *proc_vnet_dir;

/*
 *	/proc/net/vlan/config
 */

static struct proc_dir_entry *proc_vnet_conf;


/*
 *	/proc/net/vlan/mac_tbl
 */

static struct proc_dir_entry *proc_vnet_mac_tbl;



/* Strings */
static const char *vlan_name_type_str[3] = {
    [0]       = "unknow",
    [VNET_MODE_ROUTER]	= "route mode",
    [VNET_MODE_BRIDGE]	= "bridge mode",
};

#define GET_MODE_STR(v)  (v>VNET_MODE_BRIDGE?vlan_name_type_str[0]:vlan_name_type_str[v])

/*
 *	Interface functions
 */

/*
 *	Clean up /proc/net/vlan entries
 */

void vnet_proc_cleanup(void)
{
	if (proc_vnet_conf)
		remove_proc_entry(name_conf, proc_vnet_dir);

      if (proc_vnet_mac_tbl)
		remove_proc_entry(name_mac_tbl, proc_vnet_dir);
/*
 * TBS_TAG by baiyonghui 2011-3-28
 * Desc:
 */
	if (proc_vnet_dir)
		proc_net_remove(&init_net, name_root);
/*
 *
 * TBS_END_TAG
 */
	/* Dynamically added entries should be cleaned up as their vlan_device
	 * is removed, so we should not have to take care of it here...
	 */
}

/*
 *	Create /proc/net/vlan entries
 */

int __init vnet_proc_init(void)
{
	proc_vnet_dir = proc_mkdir(name_root, init_net.proc_net);
	if (proc_vnet_dir) {
		proc_vnet_conf = create_proc_entry(name_conf,
				S_IFREG|S_IRUSR|S_IWUSR,
				proc_vnet_dir);
		if (proc_vnet_conf) {
			proc_vnet_conf->proc_fops = &vnet_fops;
		}

		proc_vnet_mac_tbl = create_proc_entry(name_mac_tbl,
				S_IFREG|S_IRUSR|S_IWUSR,
				proc_vnet_dir);
		if (proc_vnet_mac_tbl) {
			proc_vnet_mac_tbl->proc_fops = &vnet_fdb_fops;
		}
		return 0;
	}
	vnet_proc_cleanup();
	return -ENOBUFS;
}



/****** Proc filesystem entry points ****************************************/

/*
 * The following few functions build the content of /proc/net/vlan/config
 */

/* starting at dev, find a VLAN device */
/*
 * TBS_TAG by baiyonghui 2011-3-28
 */

/* start read of /proc/net/vlan/config */
static void *vnet_seq_start(struct seq_file *seq, loff_t *pos)
	__acquires(&dev_base_lock)
{
	int i;
	struct net_device *dev;

	read_lock(&dev_base_lock);

	if (*pos == 0)
		return SEQ_START_TOKEN;

	i = 1;
	for_each_netdev(&init_net, dev) {
		if(!(dev->priv_flags & IFF_VNET))
			continue;

		if (i++ == *pos)
			return dev;
	}

	return NULL;
}

static void *vnet_seq_next(struct seq_file *seq, void *v, loff_t *pos)
    __acquires(&dev_base_lock)
{
	struct net_device *dev;

	++*pos;

	dev = (struct net_device *)v;
	if (v == SEQ_START_TOKEN)
		dev = net_device_entry(&init_net.dev_base_head);

	for_each_netdev_continue(&init_net, dev) {
		if (!(dev->priv_flags & IFF_VNET))
			continue;

		return dev;
	}

	return NULL;
}
/*
 * TBS_END_TAG
 *
 */

static void vnet_seq_stop(struct seq_file *seq, void *v)
{
	read_unlock(&dev_base_lock);
}

static int vnet_seq_show(struct seq_file *seq, void *v)
{
	if (v == SEQ_START_TOKEN) {

		seq_printf(seq, "%-15s| %-15s  | %-15s  | %-5s \n",
		"VNET Dev name",  "vnet_index", "real Dev name", "mode");

	} else {
		const struct net_device *vnetdev = v;
		const struct vnet_dev_info *dev_info = VNET_DEV_INFO(vnetdev);

		seq_printf(seq, "%-15s| %-15d  | %-15s  | %-5s \n",
		          vnetdev->name,   dev_info->vnet_index,    dev_info->real_dev->name,
			   GET_MODE_STR(dev_info->mode)
			   );
	}
	return 0;
}



/* start read of /proc/net/vlan/mac_tbl */
static void *vnet_fdb_seq_start(struct seq_file *seq, loff_t *pos)
{

	read_lock(&dev_base_lock);

	if (*pos == 0)
		return SEQ_START_TOKEN;


	return NULL;
}

static void *vnet_fdb_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	++*pos;

	return NULL;
}

static void vnet_fdb_seq_stop(struct seq_file *seq, void *v)
{
    read_unlock(&dev_base_lock);
}

static void vnet_fdb_visit_call(void *ptr,struct vnet_fdb_entry *ent)
{
   struct seq_file *seq=(struct seq_file *)ptr;

   seq_printf(seq, "%02x:%02x:%02x:%02x:%02x:%02x | %-16s | %-16s | %-5s \n",
		      ent->addr[0], ent->addr[1], ent->addr[2],
		      ent->addr[3], ent->addr[4], ent->addr[5],
   			  ent->dst->name, ent->real_dev->name, ent->is_local?"TRUE":"FALSE");
}

static int vnet_fdb_seq_show(struct seq_file *seq, void *v)
{
	if (v == SEQ_START_TOKEN) {
		seq_printf(seq,"mac_addr_num:%d \n",get_vnet_fdb_entry_num());
		seq_printf(seq,"mac_addr_limited:%d\n",get_vnet_fdb_entry_limited());
		seq_printf(seq, "%-17s | %-16s | %-16s | %-5s \n",
				   "mac_addr", "VNET Dev name", "Real Dev Name", "is_local");
		vnet_fdb_visit(vnet_fdb_visit_call, seq);
	}
	return 0;
}


