/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称 : nfp_adapter.c
 文件描述 : TBS加速器适配

 修订记录 :
          1 创建 :
            日期 : 2011-04-10
            描述 :
**********************************************************************/
/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称 : nfp_adapter.c
 文件描述 : TBS加速器适配

 修订记录 :
          1 创建 : 彭耀
            日期 : 2011-04-10
            描述 :
**********************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/errno.h>
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/notifier.h>
#include <linux/netfilter.h>

#include <linux/vmalloc.h>
#include <linux/mm.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#ifdef CONFIG_NF_NAT_NEEDED
#include <net/netfilter/nf_nat_core.h>
#include <net/netfilter/nf_nat_protocol.h>
#endif

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

#include "nfp_neighbour.h"
#include "nfp_interface.h"
#include "nfp_route.h"
#include "nfp_conntrack.h"
#include "nfp_bridge.h"

#include "nfp_adapter.h"


/******************************************************************************
 *                               GLOBAL VAR                                   *
 ******************************************************************************/

struct list_head nfp_interface_table[NFP_ITF_HTABLE_SIZE];

struct list_head fdb_tables_by_mac[NFP_FDB_HTABLE_SIZE];

nfp_cmd_parser_fun nfp_cmd_parser __read_mostly = NULL;
struct proc_dir_entry *nfp_adapter_proc = NULL;

/******************************************************************************
 *                               FUNCTION                                     *
 ******************************************************************************/
void nfp_init_listhead(struct list_head *hash, int size)
{
	unsigned int i;

	if (hash)
		for (i = 0; i < size; i++)
			INIT_LIST_HEAD(&hash[i]);
}

void nfp_adapter_hash_init(void)
{
    nfp_init_listhead(fdb_tables_by_mac, NFP_ARRAY_SIZE(fdb_tables_by_mac));

    nfp_init_listhead(nfp_interface_table, NFP_ARRAY_SIZE(nfp_interface_table));

}

int nfp_cmd_parser_register(nfp_cmd_parser_fun new)
{
    int ret = 0;
    nfp_cmd_parser_fun cmd_parser = NULL;
    cmd_parser = rcu_dereference(nfp_cmd_parser);
    if(NULL != cmd_parser){
        ret = -1;
        goto out;
    }
    rcu_assign_pointer(nfp_cmd_parser, new);
out:
    return ret;
}
EXPORT_SYMBOL(nfp_cmd_parser_register);

void nfp_cmd_parser_unregister(nfp_cmd_parser_fun new)
{
    nfp_cmd_parser_fun cmd_parser = NULL;
    cmd_parser = rcu_dereference(nfp_cmd_parser);
    BUG_ON(cmd_parser != new);

    rcu_assign_pointer(nfp_cmd_parser, NULL);
}
EXPORT_SYMBOL(nfp_cmd_parser_unregister);

/*重新下发所有转发规则*/
void nfp_rule_flush(void)
{
    nfp_interface_flush();

    nfp_fdb_flush();

    nfp_bridge_flush();
}
EXPORT_SYMBOL(nfp_rule_flush);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Peng Yao");
MODULE_DESCRIPTION("Tbs Tbs nfp adapter");
MODULE_ALIAS("Tbs nfp adapter");


static int __init nfp_adapter_init(void)
{
	int ret = -1;

    printk("NFP Adapter is running $Version: %s\n", NFP_ADAPTER_VERSION);

    nfp_adapter_proc = proc_mkdir(NFP_ADAPTER_NAME, init_net.proc_net);

    nfp_adapter_hash_init();

    /*注意不要随便调整子模块初始化顺序*/
    /*interface子模块初始化*/
    if(nfp_interface_init()) {
        NFP_ADAPTER_ERROR("interface init faild.\n");

        goto err1;
    }

    /*conntrack子模块初始化*/
    if(nfp_conntrack_init()) {
        NFP_ADAPTER_ERROR("conntrack init faild.\n");

        goto err2;
    }

    /*route子模块初始化*/
    if(nfp_route_init()) {
        NFP_ADAPTER_ERROR("route init faild.\n");

        goto err3;
    }

    /*neighbour子模块初始化*/
    if(nfp_neighbour_init()) {
        NFP_ADAPTER_ERROR("neighbour init faild.\n");

        goto err4;
    }

    /*Layer2bridge子模块初始化*/
    if(nfp_bridge_init()) {
        NFP_ADAPTER_ERROR("layer2bridge init faild.\n");

        goto err5;
    }

    return 0;

err5:
	nfp_neighbour_exit();

err4:
    nfp_route_exit();

err3:
    nfp_conntrack_exit();

err2:
    nfp_interface_exit();

err1:

    remove_proc_entry(NFP_ADAPTER_NAME, init_net.proc_net);

    return ret;
}


static void __exit nfp_adapter_exit(void)
{
    nfp_bridge_exit();

    nfp_conntrack_exit();

    nfp_route_exit();

    nfp_neighbour_exit();

    nfp_interface_exit();

    remove_proc_entry(NFP_ADAPTER_NAME, init_net.proc_net);

    printk("NFP Adapter is exit\n");

	return;
}

module_init(nfp_adapter_init);
module_exit(nfp_adapter_exit);
