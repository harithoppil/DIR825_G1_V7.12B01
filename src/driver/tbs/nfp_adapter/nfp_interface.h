/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称 : nfp_interface.h
 文件描述 : TBS加速器适配

 修订记录 :
          1 创建 : xiachaoren
            日期 : 2011-04-10
            描述 :
**********************************************************************/

#ifndef _NFP_INTERFACE_H_
#define _NFP_INTERFACE_H_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#endif

#include <linux/workqueue.h>
#include <linux/netdevice.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/sysctl.h>
#include <asm/atomic.h>

#define MAX_BRIDGES	8
#define MAX_PORTS	16
#define	ITF_MASK		0xfff

enum {
	ITF_BRIDGE 		= 0x001,
	ITF_VLAN		= 0x002,
	ITF_TUNNEL		= 0x004,
	ITF_PPPOE		= 0x008,
	ITF_DUMMYPORT 	= 0x010,
	ITF_DUMMY 		= 0x020,

    /*logic进程中维护dummyport的属性*/
	ITF_TYPE_LAN	= 0x040,
	ITF_TYPE_WAN 	= 0x080,

	/*dummyport绑定接口标记*/
	ITF_BIND_PHY_ITF= 0x100,
	/*dummyport启用NFP标记，用于相同vlan多wan场景 */
	ITF_NFP_ENABLE  = 0x200,

	ITF_TBS_ETH     = 0x400,
	ITF_TBS_WLAN    = 0x800,
	ITF_BRIDGE_PORT = 0x1000,
	ITF_TBS_VNET    = 0x2000,
	ITF_TBS_NAS     = 0x4000,
	ITF_TBS_PTM     = 0x8000,
};


#define DUMMYPORT_BIND_DISABLE  0
#define DUMMYPORT_BIND_ENABLE   1


/*Structure representing  a route*/
struct interface_entry {
	struct list_head list;				/* hash by ifindex */
	char ifname[IFNAMSIZ];
	/* netlink link information */
	int ifindex;
	unsigned char macaddr[MAX_ADDR_LEN];
	int macaddr_len;
	unsigned short type;				/* ARPHRD_* */
	unsigned int mtu;
	unsigned int itf_flags;				/* bit field with ITF_xxx flags */
	int phys_ifindex;					/* physical interface index, if vlan/pppoe */
	u_int16_t session_id; 				/* session id if pppoe interface */
	unsigned char dst_macaddr[MAX_ADDR_LEN]; /* peer mac address if pppoe interface */
	unsigned short vlan_id;				/* vlan id if vlan interface */
    int br_index;
	atomic_t refcnt;					/* refered count */
	int state;							/* interface state */
};


struct interface_entry *nfp_interface_query(int ifindex);
struct interface_entry *__nfp_interface_entry_get_by_index(int ifindex);
struct interface_entry *__nfp_interface_entry_get_by_name(const char *ifname);
int nfp_interface_get_real_ifindex(const struct interface_entry * itf);
bool nfp_interface_exist(int ifindex);

unsigned int itftype_get_by_index(int ifindex);
void nfp_interface_put(struct interface_entry *ie);

#define nfp_interface_hold(itf)	atomic_inc(&(itf)->refcnt)
void nfp_interface_flush(void);

int nfp_interface_init(void);
int nfp_interface_exit(void);

#endif

