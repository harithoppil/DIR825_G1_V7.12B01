/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* Everything about the rules for NEWDNAT(the dst ip and port map). */
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <net/checksum.h>
#include <net/route.h>
#include <linux/bitops.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <net/netfilter/ipt_DNATNEW.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_core.h>


static unsigned int
ipt_dnat_target_new(struct sk_buff *skb, const struct xt_target_param *par)
{
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	const struct nf_nat_multi_range_compat_new *mr = par->targinfo;

	NF_CT_ASSERT(par->hooknum == NF_INET_PRE_ROUTING ||
		     par->hooknum == NF_INET_LOCAL_OUT);

	ct = nf_ct_get(skb, &ctinfo);

	/* Connection must be valid and new. */
	NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED));

	return nf_nat_setup_info_new(ct, mr, IP_NAT_MANIP_DST);

}

static int ipt_dnat_checkentry(const struct xt_tgchk_param *par)
{
	const struct nf_nat_multi_range_compat *mr = par->targinfo;

	/* Must be a valid range */
	if (mr->rangesize != 2) {
		pr_info("DNATNEW: want two ranges \n");
		return -EINVAL;
	}
	return 0;
}

static struct xt_target ipt_dnat_new_reg __read_mostly = {
	.name		= "DNATNEW",
	.target		= ipt_dnat_target_new,
	.targetsize	= sizeof(struct nf_nat_multi_range_compat_new),
	.table		= "nat",
	.hooks		= (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_LOCAL_OUT),
	.checkentry	= ipt_dnat_checkentry,
	.family		= AF_INET,
};

int __init ipt_dnat_new_tg_init(void)
{
	int ret;

	ret = xt_register_target(&ipt_dnat_new_reg);
	if (ret != 0)
		goto unregister_dnat_new;

	return ret;


unregister_dnat_new:
	xt_unregister_target(&ipt_dnat_new_reg);

 out:
	return ret;
}

void ipt_dnat_new_tg_exit(void)
{
	xt_unregister_target(&ipt_dnat_new_reg);
}

module_init(ipt_dnat_new_tg_init);
module_exit(ipt_dnat_new_tg_exit);


