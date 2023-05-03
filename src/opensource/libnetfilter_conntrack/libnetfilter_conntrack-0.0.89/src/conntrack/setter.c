/*
 * (C) 2006 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include "internal.h"

#ifdef TBS_FLUSH_CONNTRACK_OTHER
static void set_attr_orig_ipv4_src(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].src.v4 = *((u_int32_t *) value);
}

static void set_attr_orig_ipv4_dst(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].dst.v4 = *((u_int32_t *) value);
}

static void set_attr_repl_ipv4_src(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_REPL].src.v4 = *((u_int32_t *) value);
}

static void set_attr_repl_ipv4_dst(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_REPL].dst.v4 = *((u_int32_t *) value);
}

static void set_attr_orig_ipv6_src(struct nf_conntrack *ct, const void *value)
{
	memcpy(&ct->tuple[__DIR_ORIG].src.v6, value, sizeof(u_int32_t)*4);
}

static void set_attr_orig_ipv6_dst(struct nf_conntrack *ct, const void *value)
{
	memcpy(&ct->tuple[__DIR_ORIG].dst.v6, value, sizeof(u_int32_t)*4);
}

static void set_attr_repl_ipv6_src(struct nf_conntrack *ct, const void *value)
{
	memcpy(&ct->tuple[__DIR_REPL].src.v6, value, sizeof(u_int32_t)*4);
}

static void set_attr_repl_ipv6_dst(struct nf_conntrack *ct, const void *value)
{
	memcpy(&ct->tuple[__DIR_REPL].dst.v6, value, sizeof(u_int32_t)*4);
}

static void set_attr_orig_port_src(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].l4src.all = *((u_int16_t *) value);
}

static void set_attr_orig_port_dst(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].l4dst.all = *((u_int16_t *) value);
}

static void set_attr_repl_port_src(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_REPL].l4src.all = *((u_int16_t *) value);
}

static void set_attr_repl_port_dst(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_REPL].l4dst.all = *((u_int16_t *) value);
}

static void set_attr_icmp_type(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].l4dst.icmp.type = *((u_int8_t *) value);
}

static void set_attr_icmp_code(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].l4dst.icmp.code = *((u_int8_t *) value);
}

static void set_attr_icmp_id(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].l4src.icmp.id = *((u_int16_t *) value);
}

static void set_attr_orig_l3proto(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].l3protonum = *((u_int8_t *) value);
}

static void set_attr_repl_l3proto(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_REPL].l3protonum = *((u_int8_t *) value);
}

static void set_attr_orig_l4proto(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].protonum = *((u_int8_t *) value);
}

static void set_attr_repl_l4proto(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_REPL].protonum = *((u_int8_t *) value);
}

static void set_attr_tcp_state(struct nf_conntrack *ct, const void *value)
{
	ct->protoinfo.tcp.state = *((u_int8_t *) value);
}

static void set_attr_tcp_flags_orig(struct nf_conntrack *ct, const void *value)
{
	ct->protoinfo.tcp.flags[__DIR_ORIG].value = *((u_int8_t *) value);
}

static void set_attr_tcp_mask_orig(struct nf_conntrack *ct, const void *value)
{
	ct->protoinfo.tcp.flags[__DIR_ORIG].mask = *((u_int8_t *) value);
}

static void set_attr_tcp_flags_repl(struct nf_conntrack *ct, const void *value)
{
	ct->protoinfo.tcp.flags[__DIR_REPL].value = *((u_int8_t *) value);
}

static void set_attr_tcp_mask_repl(struct nf_conntrack *ct, const void *value)
{
	ct->protoinfo.tcp.flags[__DIR_REPL].mask = *((u_int8_t *) value);
}

static void set_attr_snat_ipv4(struct nf_conntrack *ct, const void *value)
{
	ct->snat.min_ip = ct->snat.max_ip = *((u_int32_t *) value);
}

static void set_attr_dnat_ipv4(struct nf_conntrack *ct, const void *value)
{
	ct->dnat.min_ip = ct->snat.max_ip = *((u_int32_t *) value);
}

static void set_attr_snat_port(struct nf_conntrack *ct, const void *value)
{
	ct->snat.l4min.all = ct->snat.l4max.all = *((u_int16_t *) value);
}

static void set_attr_dnat_port(struct nf_conntrack *ct, const void *value)
{
	ct->dnat.l4min.all = ct->dnat.l4max.all = *((u_int16_t *) value);
}

static void set_attr_timeout(struct nf_conntrack *ct, const void *value)
{
	ct->timeout = *((u_int32_t *) value);
}

static void set_attr_mark(struct nf_conntrack *ct, const void *value)
{
	ct->mark = *((u_int32_t *) value);
}

static void set_attr_secmark(struct nf_conntrack *ct, const void *value)
{
	ct->secmark = *((u_int32_t *) value);
}

static void set_attr_status(struct nf_conntrack *ct, const void *value)
{
	ct->status = *((u_int32_t *) value);
}

static void set_attr_master_ipv4_src(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_MASTER].src.v4 = *((u_int32_t *) value);
}

static void set_attr_master_ipv4_dst(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_MASTER].dst.v4 = *((u_int32_t *) value);
}

static void set_attr_master_ipv6_src(struct nf_conntrack *ct, const void *value)
{
	memcpy(&ct->tuple[__DIR_MASTER].dst.v6, value, sizeof(u_int32_t)*4);
}

static void set_attr_master_ipv6_dst(struct nf_conntrack *ct, const void *value)
{
	memcpy(&ct->tuple[__DIR_MASTER].src.v6, value, sizeof(u_int32_t)*4);
}

static void set_attr_master_port_src(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_MASTER].l4src.all = *((u_int16_t *) value);
}

static void set_attr_master_port_dst(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_MASTER].l4dst.all = *((u_int16_t *) value);
}

static void set_attr_master_l3proto(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_MASTER].l3protonum = *((u_int8_t *) value);
}

static void set_attr_master_l4proto(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_MASTER].protonum = *((u_int8_t *) value);
}

static void set_attr_orig_cor_pos(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].natseq.correction_pos = *((u_int32_t *) value);
}

static void set_attr_orig_off_bfr(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].natseq.offset_before = *((u_int32_t *) value);
}

static void set_attr_orig_off_aft(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_ORIG].natseq.offset_after = *((u_int32_t *) value);
}

static void set_attr_repl_cor_pos(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_REPL].natseq.correction_pos = *((u_int32_t *) value);
}

static void set_attr_repl_off_bfr(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_REPL].natseq.offset_before = *((u_int32_t *) value);
}

static void set_attr_repl_off_aft(struct nf_conntrack *ct, const void *value)
{
	ct->tuple[__DIR_REPL].natseq.offset_after = *((u_int32_t *) value);
}

set_attr set_attr_array[] = {
	[ATTR_ORIG_IPV4_SRC]	= set_attr_orig_ipv4_src,
	[ATTR_ORIG_IPV4_DST] 	= set_attr_orig_ipv4_dst,
	[ATTR_REPL_IPV4_SRC]	= set_attr_repl_ipv4_src,
	[ATTR_REPL_IPV4_DST]	= set_attr_repl_ipv4_dst,
	[ATTR_ORIG_IPV6_SRC]	= set_attr_orig_ipv6_src,
	[ATTR_ORIG_IPV6_DST]	= set_attr_orig_ipv6_dst,
	[ATTR_REPL_IPV6_SRC]	= set_attr_repl_ipv6_src,
	[ATTR_REPL_IPV6_DST]	= set_attr_repl_ipv6_dst,
	[ATTR_ORIG_PORT_SRC]	= set_attr_orig_port_src,
	[ATTR_ORIG_PORT_DST]	= set_attr_orig_port_dst,
	[ATTR_REPL_PORT_SRC]	= set_attr_repl_port_src,
	[ATTR_REPL_PORT_DST]	= set_attr_repl_port_dst,
	[ATTR_ICMP_TYPE]	= set_attr_icmp_type,
	[ATTR_ICMP_CODE]	= set_attr_icmp_code,
	[ATTR_ICMP_ID]		= set_attr_icmp_id,
	[ATTR_ORIG_L3PROTO]	= set_attr_orig_l3proto,
	[ATTR_REPL_L3PROTO]	= set_attr_repl_l3proto,
	[ATTR_ORIG_L4PROTO]	= set_attr_orig_l4proto,
	[ATTR_REPL_L4PROTO]	= set_attr_repl_l4proto,
	[ATTR_TCP_STATE]	= set_attr_tcp_state,
	[ATTR_SNAT_IPV4]	= set_attr_snat_ipv4,
	[ATTR_DNAT_IPV4]	= set_attr_dnat_ipv4,
	[ATTR_SNAT_PORT]	= set_attr_snat_port,
	[ATTR_DNAT_PORT]	= set_attr_dnat_port,
	[ATTR_TIMEOUT]		= set_attr_timeout,
	[ATTR_MARK]		= set_attr_mark,
	[ATTR_STATUS]		= set_attr_status,
	[ATTR_TCP_FLAGS_ORIG]	= set_attr_tcp_flags_orig,
	[ATTR_TCP_FLAGS_REPL]	= set_attr_tcp_flags_repl,
	[ATTR_TCP_MASK_ORIG]	= set_attr_tcp_mask_orig,
	[ATTR_TCP_MASK_REPL]	= set_attr_tcp_mask_repl,
	[ATTR_MASTER_IPV4_SRC]	= set_attr_master_ipv4_src,
	[ATTR_MASTER_IPV4_DST]	= set_attr_master_ipv4_dst,
	[ATTR_MASTER_IPV6_SRC]	= set_attr_master_ipv6_src,
	[ATTR_MASTER_IPV6_DST]	= set_attr_master_ipv6_dst,
	[ATTR_MASTER_PORT_SRC]	= set_attr_master_port_src,
	[ATTR_MASTER_PORT_DST]	= set_attr_master_port_dst,
	[ATTR_MASTER_L3PROTO]	= set_attr_master_l3proto,
	[ATTR_MASTER_L4PROTO]	= set_attr_master_l4proto,
	[ATTR_SECMARK]		= set_attr_secmark,
	[ATTR_ORIG_NAT_SEQ_CORRECTION_POS] 	= set_attr_orig_cor_pos,
	[ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE] 	= set_attr_orig_off_aft,
	[ATTR_ORIG_NAT_SEQ_OFFSET_AFTER] 	= set_attr_orig_off_bfr,
	[ATTR_REPL_NAT_SEQ_CORRECTION_POS] 	= set_attr_repl_cor_pos,
	[ATTR_REPL_NAT_SEQ_OFFSET_BEFORE] 	= set_attr_repl_off_aft,
	[ATTR_REPL_NAT_SEQ_OFFSET_AFTER] 	= set_attr_repl_off_bfr,
};
#endif
