#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <asm/atomic.h>
#include <linux/if_vnet.h>
#include "vnet.h"

static struct vnet_fdb  g_vnet_fdb;

static void vnet_fdb_put(struct vnet_fdb_entry *ent);
static void vnet_fdb_cleanup(unsigned long _data);

static struct vnet_fdb_entry *vnet_alloc_entry(void)
{
	struct vnet_fdb_entry  *f;

	if(g_vnet_fdb.addr_limited &&
	   g_vnet_fdb.addr_num >= g_vnet_fdb.addr_limited)
	{
		printk(KERN_WARNING "Vnet forward table full\n");

        return NULL;
	}

    /*
      kmalloc(sizeof(struct vnet_fdb_entry), GFP_KERNEL)可能引起sleep
      mod by pengyao 20120819
    */
	f = (struct vnet_fdb_entry*)kmalloc(sizeof(struct vnet_fdb_entry), GFP_ATOMIC);
	if(f)
		g_vnet_fdb.addr_num++;

	return f;
}

static void vnet_free_entry(struct vnet_fdb_entry *f)
{
	g_vnet_fdb.addr_num--;
	kfree(f);
}

#define ADDR_ENTRY_MAX_NUM   256
void __init vnet_fdb_init(void)
{
    int i=0;

    spin_lock_init(&(g_vnet_fdb.hash_lock));
    for(i = 0; i < VNET_HASH_SIZE; i++)
    {
        INIT_HLIST_HEAD(&g_vnet_fdb.hash[i]);
    }

    init_timer(&g_vnet_fdb.ageing_timer);
    g_vnet_fdb.ageing_timer.expires = get_timeout();
    g_vnet_fdb.ageing_timer.function = vnet_fdb_cleanup;
    g_vnet_fdb.ageing_timer.data = (unsigned long)&g_vnet_fdb;
    add_timer(&g_vnet_fdb.ageing_timer);

    g_vnet_fdb.addr_limited = ADDR_ENTRY_MAX_NUM;
	g_vnet_fdb.addr_num = 0;
}

void __exit vnet_fdb_fini(void)
{
    vnet_fdb_delete_by_port(NULL);
    del_timer(&g_vnet_fdb.ageing_timer);
}

static __inline__ int vnet_mac_hash(const unsigned char *mac)
{
	//return jhash(mac, ETH_ALEN, 0) & (VNET_HASH_SIZE - 1);
	return mac[5] & 0x7F;
}

static __inline__ void vnet_fdb_delete(struct vnet_fdb_entry *f)
{
	hlist_del_rcu(&f->hlist);
	vnet_fdb_put(f);
}

static struct vnet_fdb_entry *vnet_fdb_find(struct hlist_head *head,
						    				const unsigned char *addr,
						    				struct net_device *real_dev)
{
	struct hlist_node *h;
	struct vnet_fdb_entry *f;

	hlist_for_each_entry_rcu(f, h, head, hlist) {
		if (!compare_ether_addr(f->addr, addr) &&
			f->real_dev == real_dev)
			return f;
	}
	return NULL;
}

static struct vnet_fdb_entry *__vnet_fdb_get(struct vnet_fdb *fdb,
											 const unsigned char *addr,
											 struct net_device *real_dev)
{
	return vnet_fdb_find(&fdb->hash[vnet_mac_hash(addr)], addr, real_dev);
}

struct vnet_fdb_entry *vnet_fdb_get(struct vnet_fdb *fdb,
									const unsigned char *addr,
									struct net_device *real_dev)
{
	struct vnet_fdb_entry *f;

	rcu_read_lock();
	f = __vnet_fdb_get(fdb, addr, real_dev);
	if (f)
		atomic_inc(&f->use_count);
	rcu_read_unlock();
	return f;
}

static void vnet_fdb_rcu_free(struct rcu_head *head)
{
	struct vnet_fdb_entry *ent	= container_of(head, struct vnet_fdb_entry, rcu);
	vnet_free_entry(ent);
}

/* Set entry up for deletion with RCU  */
static void vnet_fdb_put(struct vnet_fdb_entry *ent)
{
	if (atomic_dec_and_test(&ent->use_count))
		call_rcu(&ent->rcu, vnet_fdb_rcu_free);
}

static struct vnet_fdb_entry *vnet_fdb_create(struct hlist_head *head,
											  struct net_device *source,
											  const unsigned char *addr,
											  int is_local)
{
	struct vnet_fdb_entry *f;

	f = vnet_alloc_entry();
	if (f) {
		memcpy(f->addr, addr, ETH_ALEN);
		atomic_set(&f->use_count, 1);
		hlist_add_head_rcu(&f->hlist, head);

		f->dst = source;
		f->real_dev = VNET_DEV_INFO(source)->real_dev;
		f->is_local = is_local;
		f->is_static = is_local;
		f->ageing_timer = get_timeout();
	}
	return f;
}

static int vnet_do_fdb_insert(struct vnet_fdb *fdb, struct net_device *source,
							  const unsigned char *addr)
{
	struct hlist_head *head = &fdb->hash[vnet_mac_hash(addr)];
	struct vnet_fdb_entry *f;

	if (!is_valid_ether_addr(addr))
		return -EINVAL;

	f = vnet_fdb_find(head, addr, VNET_DEV_INFO(source)->real_dev);
	if (f) {

		printk(KERN_WARNING "%s adding interface with same address "
		       "as a received packet\n",
		       source->name);
		vnet_fdb_delete(f);
 	}

	/*insert操作时, 添加到转发表中的entry是静态的*/
	if (!vnet_fdb_create(head, source, addr, 1))
		return -ENOMEM;

	return 0;
}

 int vnet_fdb_insert(struct net_device *source, const unsigned char *addr)
{
	int ret;

	spin_lock_bh(&(g_vnet_fdb.hash_lock));
	ret = vnet_do_fdb_insert(&g_vnet_fdb, source, addr);
	spin_unlock_bh(&(g_vnet_fdb.hash_lock));
	return ret;
}

static void vnet_do_fdb_update(struct vnet_fdb *fdb, struct net_device *source,
							   const unsigned char *addr)
{
	struct vnet_fdb_entry *f;

	f = vnet_fdb_get(fdb, addr, VNET_DEV_INFO(source)->real_dev);
	if (likely(f)) {
		f->dst = source;
		f->ageing_timer = get_timeout();
        atomic_dec(&f->use_count);
	} else {
		spin_lock(&fdb->hash_lock);
		/*update操作时, 添加到转发表中的entry是动态的*/
		vnet_fdb_create(&fdb->hash[vnet_mac_hash(addr)], source, addr, 0);
		spin_unlock(&fdb->hash_lock);
	}
}

void vnet_fdb_changeaddr(struct net_device *vdev, const unsigned char *newaddr)
{
	int i;
	struct vnet_fdb *fdb = &g_vnet_fdb;
	struct hlist_node *h;
	struct vnet_fdb_entry *f;

	spin_lock_bh(&fdb->hash_lock);
	for (i = 0; i < VNET_HASH_SIZE; i++) {
		hlist_for_each(h, &fdb->hash[i]) {
			f = hlist_entry(h, struct vnet_fdb_entry, hlist);
			if (f->dst == vdev && f->is_local) {
				vnet_fdb_delete(f);
				goto insert;
			}
		}
	}
insert:
	/* insert new address,  may fail if invalid address or dup. */
	vnet_do_fdb_insert(fdb, vdev, newaddr);
	spin_unlock_bh(&fdb->hash_lock);
}

static void vnet_fdb_cleanup(unsigned long _data)
{
	struct vnet_fdb *fdb = (struct vnet_fdb *)_data;
	int i;
	struct vnet_fdb_entry *f;
	struct hlist_node *h, *n;

	spin_lock_bh(&fdb->hash_lock);
	for (i = 0; i < VNET_HASH_SIZE; i++) {
		hlist_for_each_entry_safe(f, h, n, &fdb->hash[i], hlist) {
			if (!f->is_static && time_before_eq(f->ageing_timer, jiffies))
				vnet_fdb_delete(f);
		}
	}
	spin_unlock_bh(&fdb->hash_lock);
	mod_timer(&fdb->ageing_timer, get_timeout());
}

void vnet_fdb_delete_by_port(struct net_device *p)
{
	int i;
	struct vnet_fdb *fdb = &g_vnet_fdb;
	struct vnet_fdb_entry *f;
	struct hlist_node *h, *g;

	spin_lock_bh(&fdb->hash_lock);
	for (i = 0; i < VNET_HASH_SIZE; i++) {
		hlist_for_each_safe(h, g, &fdb->hash[i]) {
			f = hlist_entry(h, struct vnet_fdb_entry, hlist);
			if (p && f->dst != p)
				continue;
			vnet_fdb_delete(f);
		}
	}
	spin_unlock_bh(&fdb->hash_lock);
}

void vnet_fdb_del_by_real_dev(struct net_device *real_dev, int local_stat)
{
	int i;
	struct vnet_fdb *fdb = &g_vnet_fdb;
	struct vnet_fdb_entry *f;
	struct hlist_node *h, *g;

	spin_lock_bh(&fdb->hash_lock);
	for (i = 0; i < VNET_HASH_SIZE; i++) {
		hlist_for_each_safe(h, g, &fdb->hash[i]) {
			f = hlist_entry(h, struct vnet_fdb_entry, hlist);
			if (f->real_dev == real_dev &&
			    (f->is_local == local_stat || local_stat == 2))
				vnet_fdb_delete(f);
		}
	}
	spin_unlock_bh(&fdb->hash_lock);
}

void vnet_fdb_visit(void (*func)(void *,struct vnet_fdb_entry*),void *ptr)
{
	int i;
	struct vnet_fdb *fdb=&g_vnet_fdb;
	struct vnet_fdb_entry *f;

	spin_lock_bh(&fdb->hash_lock);
	for (i = 0; i < VNET_HASH_SIZE; i++) {
		struct hlist_node *h, *g;

		hlist_for_each_safe(h, g, &fdb->hash[i]) {
			f= hlist_entry(h, struct vnet_fdb_entry, hlist);
			(*func)(ptr,f);
		}
	}
	spin_unlock_bh(&fdb->hash_lock);
}

struct net_device *vnet_get_device(const unsigned char *addr, struct net_device *real_dev)
{
	struct vnet_fdb_entry *f;

	f = __vnet_fdb_get(&g_vnet_fdb, addr, real_dev);
	if (f) {
		if (!f->is_static)
			f->ageing_timer = get_timeout();
		return f->dst;
	}

	return NULL;
}

void vnet_fdb_update(struct net_device *source, const unsigned char *addr)
{
	vnet_do_fdb_update(&g_vnet_fdb, source, addr);
}

int get_vnet_fdb_entry_num(void)
{
    return g_vnet_fdb.addr_num;
}

int get_vnet_fdb_entry_limited(void)
{
   return g_vnet_fdb.addr_limited;
}

