#ifndef _NF_CONNTRACK_SNMP_H
#define _NF_CONNTRACK_SNMP_H

extern int (*nf_nat_snmp_hook)(struct sk_buff *skb,
				unsigned int protoff,
				struct nf_conn *ct,
				enum ip_conntrack_info ctinfo);

extern int nf_conntrack_broadcast_help(struct sk_buff *skb,
				unsigned int protoff,
				struct nf_conn *ct,
				enum ip_conntrack_info ctinfo,
				unsigned int timeout);

#endif /* _NF_CONNTRACK_SNMP_H */
