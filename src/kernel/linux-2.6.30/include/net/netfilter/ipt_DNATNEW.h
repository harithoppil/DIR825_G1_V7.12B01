#ifndef _NF_NAT_RULE_H
#define _NF_NAT_RULE_H
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_nat.h>
#include <linux/netfilter_ipv4/ip_tables.h>

extern int ipt_dnat_new_tg_init(void) __init;
extern void ipt_dnat_new_tg_exit(void);

#endif /* _NF_NAT_RULE_H */

