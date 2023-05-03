/*
 * (C) 2006 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include "internal.h"

#ifdef TBS_FLUSH_CONNTRACK_OTHER
static const void *get_attr_orig_ipv4_src(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].src.v4;
}

static const void *get_attr_orig_ipv4_dst(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].dst.v4;
}

static const void *get_attr_repl_ipv4_src(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_REPL].src.v4;
}

static const void *get_attr_repl_ipv4_dst(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_REPL].dst.v4;
}

static const void *get_attr_orig_ipv6_src(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].src.v6;
}

static const void *get_attr_orig_ipv6_dst(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].dst.v6;
}

static const void *get_attr_repl_ipv6_src(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_REPL].src.v6;
}

static const void *get_attr_repl_ipv6_dst(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_REPL].dst.v6;
}

static const void *get_attr_orig_port_src(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].l4src.all; 
}

static const void *get_attr_orig_port_dst(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].l4dst.all; 
}

static const void *get_attr_repl_port_src(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_REPL].l4src.all; 
}

static const void *get_attr_repl_port_dst(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_REPL].l4dst.all;
}

static const void *get_attr_icmp_type(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].l4dst.icmp.type;
}

static const void *get_attr_icmp_code(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].l4dst.icmp.code;
}

static const void *get_attr_icmp_id(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].l4src.icmp.id;
}

static const void *get_attr_orig_l3proto(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].l3protonum;
}

static const void *get_attr_repl_l3proto(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_REPL].l3protonum;
}

static const void *get_attr_orig_l4proto(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].protonum;
}

static const void *get_attr_repl_l4proto(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_REPL].protonum;
}

static const void *get_attr_tcp_state(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.state;
}

static const void *get_attr_tcp_flags_orig(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.flags[__DIR_ORIG].value;
}

static const void *get_attr_tcp_mask_orig(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.flags[__DIR_ORIG].mask;
}

static const void *get_attr_tcp_flags_repl(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.flags[__DIR_REPL].value;
}

static const void *get_attr_tcp_mask_repl(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.flags[__DIR_REPL].mask;
}

static const void *get_attr_snat_ipv4(const struct nf_conntrack *ct)
{
	return &ct->snat.min_ip;
}

static const void *get_attr_dnat_ipv4(const struct nf_conntrack *ct)
{
	return &ct->dnat.min_ip;
}

static const void *get_attr_snat_port(const struct nf_conntrack *ct)
{
	return &ct->snat.l4min.all;
}

static const void *get_attr_dnat_port(const struct nf_conntrack *ct)
{
	return &ct->dnat.l4min.all;
}

static const void *get_attr_timeout(const struct nf_conntrack *ct)
{
	return &ct->timeout;
}

static const void *get_attr_mark(const struct nf_conntrack *ct)
{
	return &ct->mark;
}

static const void *get_attr_secmark(const struct nf_conntrack *ct)
{
	return &ct->secmark;
}

static const void *get_attr_orig_counter_packets(const struct nf_conntrack *ct)
{
	return &ct->counters[__DIR_ORIG].packets;
}

static const void *get_attr_orig_counter_bytes(const struct nf_conntrack *ct)
{
	return &ct->counters[__DIR_ORIG].bytes;
}

static const void *get_attr_repl_counter_packets(const struct nf_conntrack *ct)
{
	return &ct->counters[__DIR_REPL].packets;
}

static const void *get_attr_repl_counter_bytes(const struct nf_conntrack *ct)
{
	return &ct->counters[__DIR_REPL].bytes;
}

static const void *get_attr_status(const struct nf_conntrack *ct)
{
	return &ct->status;
}

static const void *get_attr_use(const struct nf_conntrack *ct)
{
	return &ct->use;
}

static const void *get_attr_orig_cor_pos(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].natseq.correction_pos;
}

static const void *get_attr_orig_off_bfr(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].natseq.offset_before;
}

static const void *get_attr_orig_off_aft(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_ORIG].natseq.offset_after;
}

static const void *get_attr_repl_cor_pos(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_REPL].natseq.correction_pos;
}

static const void *get_attr_repl_off_bfr(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_REPL].natseq.offset_before;
}

static const void *get_attr_repl_off_aft(const struct nf_conntrack *ct)
{
	return &ct->tuple[__DIR_REPL].natseq.offset_after;
}

get_attr get_attr_array[] = {
	[ATTR_ORIG_IPV4_SRC]		= get_attr_orig_ipv4_src,
	[ATTR_ORIG_IPV4_DST] 		= get_attr_orig_ipv4_dst,
	[ATTR_REPL_IPV4_SRC]		= get_attr_repl_ipv4_src,
	[ATTR_REPL_IPV4_DST]		= get_attr_repl_ipv4_dst,
	[ATTR_ORIG_IPV6_SRC]		= get_attr_orig_ipv6_src,
	[ATTR_ORIG_IPV6_DST]		= get_attr_orig_ipv6_dst,
	[ATTR_REPL_IPV6_SRC]		= get_attr_repl_ipv6_src,
	[ATTR_REPL_IPV6_DST]		= get_attr_repl_ipv6_dst,
	[ATTR_ORIG_PORT_SRC]		= get_attr_orig_port_src,
	[ATTR_ORIG_PORT_DST]		= get_attr_orig_port_dst,
	[ATTR_REPL_PORT_SRC]		= get_attr_repl_port_src,
	[ATTR_REPL_PORT_DST]		= get_attr_repl_port_dst,
	[ATTR_ICMP_TYPE]		= get_attr_icmp_type,
	[ATTR_ICMP_CODE]		= get_attr_icmp_code,
	[ATTR_ICMP_ID]			= get_attr_icmp_id,
	[ATTR_ORIG_L3PROTO]		= get_attr_orig_l3proto,
	[ATTR_REPL_L3PROTO]		= get_attr_repl_l3proto,
	[ATTR_ORIG_L4PROTO]		= get_attr_orig_l4proto,
	[ATTR_REPL_L4PROTO]		= get_attr_repl_l4proto,
	[ATTR_TCP_STATE]		= get_attr_tcp_state,
	[ATTR_SNAT_IPV4]		= get_attr_snat_ipv4,
	[ATTR_DNAT_IPV4]		= get_attr_dnat_ipv4,
	[ATTR_SNAT_PORT]		= get_attr_snat_port,
	[ATTR_DNAT_PORT]		= get_attr_dnat_port,
	[ATTR_TIMEOUT]			= get_attr_timeout,
	[ATTR_MARK]			= get_attr_mark,
	[ATTR_ORIG_COUNTER_PACKETS] 	= get_attr_orig_counter_packets,
	[ATTR_ORIG_COUNTER_BYTES]	= get_attr_orig_counter_bytes,
	[ATTR_REPL_COUNTER_PACKETS]	= get_attr_repl_counter_packets,
	[ATTR_REPL_COUNTER_BYTES]	= get_attr_repl_counter_bytes,
	[ATTR_USE]			= get_attr_use,
	[ATTR_STATUS]			= get_attr_status,
	[ATTR_TCP_FLAGS_ORIG]		= get_attr_tcp_flags_orig,
	[ATTR_TCP_FLAGS_REPL]		= get_attr_tcp_flags_repl,
	[ATTR_TCP_MASK_ORIG]		= get_attr_tcp_mask_orig,
	[ATTR_TCP_MASK_REPL]		= get_attr_tcp_mask_repl,
	[ATTR_SECMARK]			= get_attr_secmark,
	[ATTR_ORIG_NAT_SEQ_CORRECTION_POS]	= get_attr_orig_cor_pos,
	[ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE]	= get_attr_orig_off_bfr,
	[ATTR_ORIG_NAT_SEQ_OFFSET_AFTER]	= get_attr_orig_off_aft,
	[ATTR_REPL_NAT_SEQ_CORRECTION_POS]	= get_attr_repl_cor_pos,
	[ATTR_REPL_NAT_SEQ_OFFSET_BEFORE]	= get_attr_repl_off_bfr,
	[ATTR_REPL_NAT_SEQ_OFFSET_AFTER]	= get_attr_repl_off_aft,
};
#endif
