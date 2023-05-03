/*
    Module Name:
    frame_engine.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-01-11      Initial version
*/

#ifndef _FE_WANTED
#define _FE_WANTED

#include <linux/version.h>
#include "fe_api.h"
#ifndef TCSUPPORT_RA_HWNAT
#include <asm/rt2880/rt_mmap.h>
#endif
/*
 * DEFINITIONS AND MACROS
 */
#define MAC_ARG(x) ((u8*)(x))[0],((u8*)(x))[1],((u8*)(x))[2], \
                       ((u8*)(x))[3],((u8*)(x))[4],((u8*)(x))[5]

#define IPV6_ADDR(x) ntohs(x[0]),ntohs(x[1]),ntohs(x[2]),ntohs(x[3]),ntohs(x[4]),\
		     ntohs(x[5]),ntohs(x[6]),ntohs(x[7])

#define IN
#define OUT
#define INOUT

#define NAT_DEBUG


#ifdef NAT_DEBUG
#define NAT_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define NAT_PRINT(fmt, args...) { }
#endif

#define CHIPID		    RALINK_SYSCTL_BASE + 0x00
#define REVID		    RALINK_SYSCTL_BASE + 0x0C

#if defined (CONFIG_HNAT_V2)
#ifdef TCSUPPORT_RA_HWNAT
#define RALINK_SYSCTL_BASE			0xBFB00000
#define RALINK_ETH_SW_BASE 			0xBFB58000
#define RALINK_FRAME_ENGINE_BASE 	0xBFB50000
#define RALINK_PPE_BASE				0xBFB50c00
#endif

#define PFC		    RALINK_ETH_SW_BASE + 0x0004
#define TPF0		    RALINK_ETH_SW_BASE + 0x2030
#define TPF1		    RALINK_ETH_SW_BASE + 0x2130
#define TPF2		    RALINK_ETH_SW_BASE + 0x2230
#define TPF3		    RALINK_ETH_SW_BASE + 0x2330
#define TPF4		    RALINK_ETH_SW_BASE + 0x2430
#define TPF5		    RALINK_ETH_SW_BASE + 0x2530
#define TPF6		    RALINK_ETH_SW_BASE + 0x2630

#define PMCR_P7		    RALINK_ETH_SW_BASE + 0x3700
#define PSC_P7		    RALINK_ETH_SW_BASE + 0x270c
#define FOE_TS		    RALINK_FRAME_ENGINE_BASE + 0x0010

#define PPE_FQFC_CFG	    RALINK_PPE_BASE + 0x00
#define PPE_IQ_CFG	    RALINK_PPE_BASE + 0x04
#define PPE_QUE_STA	    RALINK_PPE_BASE + 0x08
#if defined(TCSUPPORT_WAN_GPON) || defined(TCSUPPORT_WAN_EPON)
#define GDM1_LEN_CFG			RALINK_FRAME_ENGINE_BASE + 0x514
#endif
#ifdef TCSUPPORT_RA_HWNAT
#define GDM1_FWD_CFG_OFFSET	    0x500
#define GDM2_FWD_CFG_OFFSET	    0x1500
#define L2_BR_CONFIG_OFFSET		0x280
#define L2_BR_ETYPE_EN_OFFSET	0x284
#define L2_ETHTYPE_0_OFFSET		0x290
#define L2_ETHTYPE_1_OFFSET		0x294
#else
#define GDM2_FWD_CFG	    RALINK_PPE_BASE + 0x100
#define GDM2_SHPR_CFG	    RALINK_PPE_BASE + 0x104
#endif
#define PPE_GLO_CFG	    	RALINK_PPE_BASE + 0x200
#define PPE_FLOW_CFG	    RALINK_PPE_BASE + 0x204
#define PPE_FLOW_SET	    PPE_FLOW_CFG
#define PPE_IP_PROT_CHK	    RALINK_PPE_BASE + 0x208
#define PPE_IP_PROT_0	    RALINK_PPE_BASE + 0x20C
#define PPE_IP_PROT_1	    RALINK_PPE_BASE + 0x210
#define PPE_IP_PROT_2	    RALINK_PPE_BASE + 0x214
#define PPE_IP_PROT_3	    RALINK_PPE_BASE + 0x218
#define PPE_TB_CFG	    	RALINK_PPE_BASE + 0x21C
#define PPE_FOE_CFG	    	PPE_TB_CFG
#define PPE_TB_BASE	    	RALINK_PPE_BASE + 0x220
#define PPE_FOE_BASE	    PPE_TB_BASE
#define PPE_TB_USED	    	RALINK_PPE_BASE + 0x224
#define PPE_BNDR	    	RALINK_PPE_BASE + 0x228
#define PPE_FOE_BNDR	    PPE_BNDR
#define PPE_BIND_LMT_0	    RALINK_PPE_BASE + 0x22C
#define PPE_FOE_LMT1	    PPE_BIND_LMT_0
#define PPE_BIND_LMT_1	    RALINK_PPE_BASE + 0x230
#define PPE_FOE_LMT2	    PPE_BIND_LMT_1
#define PPE_KA		    	RALINK_PPE_BASE + 0x234
#define PPE_FOE_KA	    	PPE_KA
#define PPE_UNB_AGE	    	RALINK_PPE_BASE + 0x238
#define PPE_FOE_UNB_AGE	    PPE_UNB_AGE
#define PPE_BND_AGE_0	    RALINK_PPE_BASE + 0x23C
#define PPE_FOE_BND_AGE0    PPE_BND_AGE_0
#define PPE_BND_AGE_1	    RALINK_PPE_BASE + 0x240
#define PPE_FOE_BND_AGE1    PPE_BND_AGE_1
#define PPE_HASH_SEED	    RALINK_PPE_BASE + 0x244
#ifdef TCSUPPORT_RA_HWNAT
#define PPE_DFP_CPORT	    RALINK_PPE_BASE + 0x248
#define PPE_MCAST_PPSE	    RALINK_PPE_BASE + 0x284
#define PPE_VLAN_TPID	    RALINK_PPE_BASE + 0x318
#else
#define PPE_FP_BMAP_0	    RALINK_PPE_BASE + 0x248
#define PPE_FP_BMAP_1	    RALINK_PPE_BASE + 0x24C
#define PPE_FP_BMAP_2	    RALINK_PPE_BASE + 0x250
#define PPE_FP_BMAP_3	    RALINK_PPE_BASE + 0x254
#define PPE_FP_BMAP_4	    RALINK_PPE_BASE + 0x258
#define PPE_FP_BMAP_5	    RALINK_PPE_BASE + 0x25C
#define PPE_FP_BMAP_6	    RALINK_PPE_BASE + 0x260
#define PPE_FP_BMAP_7	    RALINK_PPE_BASE + 0x264

#define PPE_TIPV4_0	    RALINK_PPE_BASE + 0x268
#define PPE_TIPV4_1	    RALINK_PPE_BASE + 0x26C
#define PPE_TIPV4_2	    RALINK_PPE_BASE + 0x270
#define PPE_TIPV4_3	    RALINK_PPE_BASE + 0x274
#define PPE_TIPV4_4	    RALINK_PPE_BASE + 0x278
#define PPE_TIPV4_5	    RALINK_PPE_BASE + 0x27C
#define PPE_TIPV4_6	    RALINK_PPE_BASE + 0x280
#define PPE_TIPV4_7	    RALINK_PPE_BASE + 0x284

#define PPE_TIPV6_127_0	    RALINK_PPE_BASE + 0x288
#define PPE_TIPV6_95_0	    RALINK_PPE_BASE + 0x28C
#define PPE_TIPV6_63_0	    RALINK_PPE_BASE + 0x290
#define PPE_TIPV6_31_0	    RALINK_PPE_BASE + 0x294

#define PPE_TIPV6_127_1	    RALINK_PPE_BASE + 0x298
#define PPE_TIPV6_95_1	    RALINK_PPE_BASE + 0x29C
#define PPE_TIPV6_63_1	    RALINK_PPE_BASE + 0x2A0
#define PPE_TIPV6_31_1	    RALINK_PPE_BASE + 0x2A4

#define PPE_TIPV6_127_2	    RALINK_PPE_BASE + 0x2A8
#define PPE_TIPV6_95_2	    RALINK_PPE_BASE + 0x2AC
#define PPE_TIPV6_63_2	    RALINK_PPE_BASE + 0x2B0
#define PPE_TIPV6_31_2	    RALINK_PPE_BASE + 0x2B4

#define PPE_TIPV6_127_3	    RALINK_PPE_BASE + 0x2B8
#define PPE_TIPV6_95_3	    RALINK_PPE_BASE + 0x2BC
#define PPE_TIPV6_63_3	    RALINK_PPE_BASE + 0x2C0
#define PPE_TIPV6_31_3	    RALINK_PPE_BASE + 0x2C4

#define PPE_TIPV6_127_4	    RALINK_PPE_BASE + 0x2C8
#define PPE_TIPV6_95_4	    RALINK_PPE_BASE + 0x2CC
#define PPE_TIPV6_63_4	    RALINK_PPE_BASE + 0x2D0
#define PPE_TIPV6_31_4	    RALINK_PPE_BASE + 0x2D4

#define PPE_TIPV6_127_5	    RALINK_PPE_BASE + 0x2D8
#define PPE_TIPV6_95_5	    RALINK_PPE_BASE + 0x2DC
#define PPE_TIPV6_63_5	    RALINK_PPE_BASE + 0x2E0
#define PPE_TIPV6_31_5	    RALINK_PPE_BASE + 0x2E4

#define PPE_TIPV6_127_6	    RALINK_PPE_BASE + 0x2E8
#define PPE_TIPV6_95_6	    RALINK_PPE_BASE + 0x2EC
#define PPE_TIPV6_63_6	    RALINK_PPE_BASE + 0x2F0
#define PPE_TIPV6_31_6	    RALINK_PPE_BASE + 0x2F4

#define PPE_TIPV6_127_7	    RALINK_PPE_BASE + 0x2F8
#define PPE_TIPV6_95_7	    RALINK_PPE_BASE + 0x2FC
#define PPE_TIPV6_63_7	    RALINK_PPE_BASE + 0x300
#define PPE_TIPV6_31_7	    RALINK_PPE_BASE + 0x304
#endif
#define PPE_MTU_DRP	    RALINK_PPE_BASE + 0x308
#define PPE_MTU_VLYR_0	    RALINK_PPE_BASE + 0x30C
#define PPE_MTU_VLYR_1	    RALINK_PPE_BASE + 0x310
#define PPE_MTU_VLYR_2	    RALINK_PPE_BASE + 0x314
#ifdef TCSUPPORT_RA_HWNAT
#define PPE_TPID	    RALINK_PPE_BASE + 0x318
#endif
#define CAH_CTRL	    RALINK_PPE_BASE + 0x320
#define CAH_TAG_SRH	    RALINK_PPE_BASE + 0x324
#define CAH_LINE_RW	    RALINK_PPE_BASE + 0x328
#define CAH_WDATA	    RALINK_PPE_BASE + 0x32C

/* 
 * CAH_RDATA[17:16]
 *  0: invalid
 *  1: valid
 *  2: dirty
 *  3: lock
 *
 * CAH_RDATA[15:0]: entry num 
 */
#define CAH_RDATA	    RALINK_PPE_BASE + 0x330

#ifdef TCSUPPORT_RA_HWNAT
#define CRSN_MASK		RALINK_PPE_BASE + 0x380
#endif
#define GDM1_OFRC_P_CPU     (0 << 0)
#define GDM1_MFRC_P_CPU     (0 << 4)
#define GDM1_BFRC_P_CPU     (0 << 8)
#define GDM1_UFRC_P_CPU     (0 << 12)

#ifdef TCSUPPORT_RA_HWNAT
#define GDM1_OFRC_P_QDMA     (5 << 0)
#define GDM1_MFRC_P_QDMA     (5 << 4)
#define GDM1_BFRC_P_QDMA     (5 << 8)
#define GDM1_UFRC_P_QDMA     (5 << 12)


#define GDM1_OFRC_P_PPE     (4 << 0)
#define GDM1_MFRC_P_PPE     (4 << 4)
#define GDM1_BFRC_P_PPE     (4 << 8)
#define GDM1_UFRC_P_PPE     (4 << 12)

//source port define 
#define SP_PDMA				(0)
#define SP_GDMA1			(1)
#define SP_GDMA2			(2)
#define SP_PPE				(4)
#define SP_QDMA				(5)
#define SP_GSW_PORT0		(8)
#define SP_GSW_PORT1		(9)
#define SP_GSW_PORT2		(10)
#define SP_GSW_PORT3		(11)
#define SP_GSW_PORT4		(12)
#define SP_GSW_PORT5		(13)
#define SP_GSW_PORT6		(14)
#define SP_GSW_PORT7		(15)


//force port define 
#define FP_PDMA				(0)
#define FP_GDMA1			(1)
#define FP_GDMA2			(2)
#define FP_PPE				(4)
#define FP_QDMA_SW			(5)
#define FP_QDMA_HW			(6)
#define FP_DROP				(7)			

#define WHITE_LIST			(0)
#define BLACK_LIST			(1)
#endif


/* TO PPE */
#define IPV4_PPE_MYUC	    (1 << 0) //my mac
#define IPV4_PPE_MC	    (1 << 1) //multicast
#define IPV4_PPE_IPM	    (1 << 2) //ip multicast
#define IPV4_PPE_BC	    (1 << 3) //broadcast
#define IPV4_PPE_UC	    (1 << 4) //ipv4 learned UC frame
#define IPV4_PPE_UN	    (1 << 5) //ipv4 unknown  UC frame

#define IPV6_PPE_MYUC	    (1 << 8) //my mac
#define IPV6_PPE_MC	    (1 << 9) //multicast
#define IPV6_PPE_IPM	    (1 << 10) //ipv6 multicast
#define IPV6_PPE_BC	    (1 << 11) //broadcast
#define IPV6_PPE_UC	    (1 << 12) //ipv6 learned UC frame
#define IPV6_PPE_UN	    (1 << 13) //ipv6 unknown  UC frame


#define AC_BASE		    RALINK_FRAME_ENGINE_BASE + 0x2000
#define METER_BASE	    RALINK_FRAME_ENGINE_BASE + 0x1200


#else
#define FE_GLO_BASE         RALINK_FRAME_ENGINE_BASE
#define PPE_BASE	    RALINK_FRAME_ENGINE_BASE + 0x200
#define AC_BASE		    RALINK_FRAME_ENGINE_BASE + 0x400
#define METER_BASE	    RALINK_FRAME_ENGINE_BASE + 0x600

#define FOE_TS		    FE_GLO_BASE+0x1C
#define GDMA1_BASE          FE_GLO_BASE+0x20
#define FE_GDMA1_SCH_CFG    GDMA1_BASE+0x04
#define GDMA2_BASE          FE_GLO_BASE+0x60
#define FE_GDMA2_SCH_CFG    GDMA2_BASE+0x04

#define PPE_GLO_CFG	    PPE_BASE + 0x00
#define PPE_STD_GA_H	    PPE_BASE + 0x04
#define PPE_STD_GA_L	    PPE_BASE + 0x08
#define PPE_ETD_GA_H	    PPE_BASE + 0x0C
#define PPE_EXT_GA_L	    PPE_BASE + 0x10
#define PPE_FLOW_SET	    PPE_BASE + 0x14
#define PPE_PRE_ACL	    PPE_BASE + 0x18
#define PPE_PRE_MTR	    PPE_BASE + 0x1C
#define PPE_PRE_AC	    PPE_BASE + 0x20
#define PPE_POST_MTR	    PPE_BASE + 0x24
#define PPE_POST_AC	    PPE_BASE + 0x28
#define PPE_POL_CFG	    PPE_BASE + 0x2C
#define PPE_FOE_CFG	    PPE_BASE + 0x30
#define PPE_FOE_BASE	    PPE_BASE + 0x34
#define PPE_FOE_USE	    PPE_BASE + 0x38
#define PPE_FOE_BNDR	    PPE_BASE + 0x3C
#define PPE_FOE_LMT1	    PPE_BASE + 0x40
#define PPE_FOE_LMT2	    PPE_BASE + 0x44
#define PPE_FOE_KA	    PPE_BASE + 0x48
#define PPE_FOE_UNB_AGE	    PPE_BASE + 0x4C
#define PPE_FOE_BND_AGE0    PPE_BASE + 0x50
#define PPE_FOE_BND_AGE1    PPE_BASE + 0x54
#define CPU_PORT_CFG	    PPE_BASE + 0x58
#define GE1_PORT_CFG	    PPE_BASE + 0x5C
#define DSCP0_7_MAP_UP	    PPE_BASE + 0x60
#define DSCP8_15_MAP_UP	    PPE_BASE + 0x64
#define DSCP16_23_MAP_UP    PPE_BASE + 0x68
#define DSCP24_31_MAP_UP    PPE_BASE + 0x6C
#define DSCP32_39_MAP_UP    PPE_BASE + 0x70
#define DSCP40_47_MAP_UP    PPE_BASE + 0x74
#define DSCP48_55_MAP_UP    PPE_BASE + 0x78
#define DSCP56_63_MAP_UP    PPE_BASE + 0x7C
#define AUTO_UP_CFG1	    PPE_BASE + 0x80
#define AUTO_UP_CFG2	    PPE_BASE + 0x84
#define UP_RES		    PPE_BASE + 0x88
#define UP_MAP_VPRI	    PPE_BASE + 0x8C
#define UP0_3_MAP_IDSCP	    PPE_BASE + 0x90
#define UP4_7_MAP_IDSCP	    PPE_BASE + 0x94
#define UP0_3_MAP_ODSCP	    PPE_BASE + 0x98
#define UP4_7_MAP_ODSCP	    PPE_BASE + 0x9C
#define UP_MAP_AC	    PPE_BASE + 0xA0

#define FE_GDMA1_FWD_CFG    RALINK_FRAME_ENGINE_BASE + 0x20
#define FE_GDMA2_FWD_CFG    RALINK_FRAME_ENGINE_BASE + 0x60
#define FE_COS_MAP	    RALINK_FRAME_ENGINE_BASE + 0xC8

/* GDMA1 My MAC unicast frame destination port */
#define GDM1_UFRC_P_CPU     (0 << 12)
#define GDM1_UFRC_P_GDMA1   (1 << 12)
#define GDM1_UFRC_P_PPE     (6 << 12)

/* GDMA1 broadcast frame MAC address destination port */
#define GDM1_BFRC_P_CPU     (0 << 8)
#define GDM1_BFRC_P_GDMA1   (1 << 8)
#define GDM1_BFRC_P_PPE     (6 << 8)

/* GDMA1 multicast frame MAC address destination port */
#define GDM1_MFRC_P_CPU     (0 << 4)
#define GDM1_MFRC_P_GDMA1   (1 << 4)
#define GDM1_MFRC_P_PPE     (6 << 4)

/* GDMA1 other MAC address frame destination port */
#define GDM1_OFRC_P_CPU     (0 << 0)
#define GDM1_OFRC_P_GDMA1   (1 << 0)
#define GDM1_OFRC_P_PPE     (6 << 0)
#endif

enum FoeSma {
	DROP = 0,		/* Drop the packet */
	DROP2 = 1,		/* Drop the packet */
	ONLY_FWD_CPU = 2,	/* Only Forward to CPU */
	FWD_CPU_BUILD_ENTRY = 3	/* Forward to CPU and build new FOE entry */
};

enum BindDir {
	UPSTREAM_ONLY = 0,	/* only speed up upstream flow */
	DOWNSTREAM_ONLY = 1,	/* only speed up downstream flow */
	BIDIRECTION = 2		/* speed up bi-direction flow */
};

#if defined (CONFIG_HNAT_V2)
enum FoeCpuReason {
#ifdef TCSUPPORT_RA_HWNAT
	IPTU_CSUMF = 0x1, /* ipv4, tcp udp checksum fail */
#endif
	TTL_0 = 0x02, /* IPv4(IPv6) TTL(hop limit) = 0 */
	HAS_OPTION_HEADER = 0x03, /* IPv4(IPv6) has option(extension) header */
	NO_FLOW_IS_ASSIGNED = 0x07,	/* No flow is assigned */
	IPV4_WITH_FRAGMENT = 0x08,	/* IPv4 HNAT doesn't support IPv4 /w fragment */
	IPV4_HNAPT_DSLITE_WITH_FRAGMENT = 0x09,	/* IPv4 HNAPT/DS-Lite doesn't support IPv4 /w fragment */
	IPV4_HNAPT_DSLITE_WITHOUT_TCP_UDP = 0x0A,	/* IPv4 HNAPT/DS-Lite can't find TCP/UDP sport/dport */
	IPV6_5T_6RD_WITHOUT_TCP_UDP = 0x0B,	/* IPv6 5T-route/6RD can't find TCP/UDP sport/dport */
	TCP_FIN_SYN_RST = 0x0C,	/* Ingress packet is TCP fin/syn/rst (for IPv4 NAPT/DS-Lite or IPv6 5T-route/6RD) */
	UN_HIT = 0x0D,		/* FOE Un-hit */
	HIT_UNBIND = 0x0E,	/* FOE Hit unbind */
	HIT_UNBIND_RATE_REACH = 0x0F,	/* FOE Hit unbind & rate reach */
	HIT_BIND_TCP_FIN = 0x10,	/* Hit bind PPE TCP FIN entry */
	HIT_BIND_TTL_1 = 0x11,	/* Hit bind PPE entry and TTL(hop limit) = 1 and TTL(hot limit) - 1 */
	HIT_BIND_WITH_VLAN_VIOLATION = 0x12,	/* Hit bind and VLAN replacement violation
						   (Ingress 1(0) VLAN layers and egress 4(3 or 4) VLAN layers) */
	HIT_BIND_KEEPALIVE_UC_OLD_HDR = 0x13,	/* Hit bind and keep alive with unicast old-header packet */
	HIT_BIND_KEEPALIVE_MC_NEW_HDR = 0x14,	/* Hit bind and keep alive with multicast new-header packet */
	HIT_BIND_KEEPALIVE_DUP_OLD_HDR = 0x15,	/* Hit bind and keep alive with duplicate old-header packet */
	HIT_BIND_FORCE_TO_CPU = 0x16,	/* FOE Hit bind & force to CPU */
	HIT_BIND_WITH_OPTION_HEADER = 0x17, /* Hit bind and remove tunnel IP header, but inner IP has option/next header */
#ifdef TCSUPPORT_RA_HWNAT
	HIT_BIND_MUL_CPU = 0x18, /*  Hit Bind and Multicast to CPU*/
	HIT_BIND_MUL_CPUR = 0x19, /*  Hit Bind and Multicast to CPU force to CPU*/
	HIT_PREBIND = 0x1A, /*  Hit Pre-Bind*/
	UNHIT_CLASS = 0x1B, /*  UnHit CLASS Packet*/
#endif
	HIT_BIND_EXCEED_MTU = 0x1C	/* Hit bind and exceed MTU */
};
#else
enum FoeCpuReason {
	TTL_0 = 0x80,		/* TTL=0 */
	FOE_EBL_NOT_IPV4_HLEN5 = 0x90,	/* FOE enable & not IPv4h5nf */
	FOE_EBL_NOT_TCP_UDP_L4_READY = 0x91,	/* FOE enable & not TCP/UDP/L4_read */
	TCP_SYN_FIN_RST = 0x92,	/* TCP SYN/FIN/RST */
	UN_HIT = 0x93,		/* FOE Un-hit */
	HIT_UNBIND = 0x94,	/* FOE Hit unbind */
	HIT_UNBIND_RATE_REACH = 0x95,	/* FOE Hit unbind & rate reach */
	HIT_FIN = 0x96,		/* FOE Hit fin */
	HIT_BIND_TTL_1 = 0x97,	/* FOE Hit bind & ttl=1 & ttl-1 */
	HIT_BIND_KEEPALIVE = 0x98,	/* FOE Hit bind & keep alive */
	HIT_BIND_FORCE_TO_CPU = 0x99,	/* FOE Hit bind & force to CPU */
	ACL_FOE_TBL_ERR = 0x9A,	/* ACL link foe table error !(static & unbind) */
	ACL_TBL_TTL_1 = 0x9B,	/* ACL link FOE table & TTL=1 & TTL-1 */
	ACL_ALERT_CPU = 0x9C,	/* ACL alert CPU */
	NO_FORCE_DEST_PORT = 0xA0,	/* No force destination port */
	ACL_FORCE_PRIORITY0 = 0xA8,	/* ACL to UP0 */
	ACL_FORCE_PRIORITY1 = 0xA9,	/* ACL to UP1 */
	ACL_FORCE_PRIORITY2 = 0xAA,	/* ACL to UP2 */
	ACL_FORCE_PRIORITY3 = 0xAB,	/* ACL to UP3 */
	ACL_FORCE_PRIORITY4 = 0xAC,	/* ACL to UP4 */
	ACL_FORCE_PRIORITY5 = 0xAD,	/* ACL to UP5 */
	ACL_FORCE_PRIORITY6 = 0xAE,	/* ACL to UP6 */
	ACL_FORCE_PRIORITY7 = 0xAF,	/* ACL to UP7 */
	EXCEED_MTU = 0xA1	/* Exceed mtu */
};
#endif

#ifdef TCSUPPORT_RA_HWNAT
#define CONFIG_RA_NAT_HW
#define CONFIG_RALINK_MT7620

#define CONFIG_RA_HW_NAT_LAN_VLANID 1
#define CONFIG_RA_HW_NAT_WAN_VLANID 5

#define CONFIG_RA_HW_NAT_BINDING_THRESHOLD	30
#define CONFIG_RA_HW_NAT_QURT_LMT		100
#define CONFIG_RA_HW_NAT_HALF_LMT		50
#define CONFIG_RA_HW_NAT_FULL_LMT		25

//#define CONFIG_RA_HW_NAT_TBL_1K
//#define CONFIG_RA_HW_NAT_TBL_2K
//#define CONFIG_RA_HW_NAT_TBL_4K
#define CONFIG_RA_HW_NAT_TBL_8K
//#define CONFIG_RA_HW_NAT_TBL_16K

//#define CONFIG_RA_HW_NAT_HASH0
#define CONFIG_RA_HW_NAT_HASH1
//#define CONFIG_RA_HW_NAT_HASH2
//#define CONFIG_RA_HW_NAT_HASH3


#define CONFIG_RA_HW_NAT_TCP_KA			1
#define CONFIG_RA_HW_NAT_UDP_KA			1
#define CONFIG_RA_HW_NAT_NTU_KA			1
//#define CONFIG_RA_HW_NAT_PREBIND		1

#define CONFIG_RA_HW_NAT_UNB_DLTA		3
#define CONFIG_RA_HW_NAT_UNB_MNP		1000
#define CONFIG_RA_HW_NAT_UDP_DLTA		30
#define CONFIG_RA_HW_NAT_TCP_DLTA		30
#define CONFIG_RA_HW_NAT_FIN_DLTA		5
#define CONFIG_RA_HW_NAT_NTU_DLTA		15

#define CONFIG_RA_HW_NAT_L2B
#define CONFIG_RA_HW_NAT_IP_FRAG

#define CONFIG_RA_HW_NAT_WIFI
#define WLAN_IF_NUM		4 //8 is maximum
#define WLAN_IF_I_NUM		4 //8 is maximum, foe rai

#define CONFIG_RA_HW_NAT_VPN_PASSTHROUGH

#endif

/* PPE_GLO_CFG, Offset=0x200 */
#define DFL_TTL0_DRP		(1)	/* 1:Drop, 0: Alert CPU */
#if! defined (CONFIG_HNAT_V2)

#define DFL_VPRI_EN		(1)	/* Use VLAN pri tag as priority desision */
#define DFL_DPRI_EN		(1)	/* Use DSCP as priority decision */

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
#define DFL_REG_DSCP		(0)	/* Re-gePnerate DSCP */
#define DFL_REG_VPRI		(1)	/* Re-generate VLAN priority tag */
#else
#define DFL_REG_DSCP		(0)	/* Re-gePnerate DSCP */
#define DFL_REG_VPRI		(0)	/* Re-generate VLAN priority tag */
#endif

#define DFL_ACL_PRI_EN		(1)	/* ACL force priority for hit unbind and rate reach */

/* FreeQueueLen	|RedMode=0| RedMode=1| RedMode=2 | RedMode=3
 * -------------+----------+----------+-----------+-----------
 *    0-31	  50%	    75%		100%	    100%
 *    32-63	  25%	    50%		75%	    100%
 *    64-95	   0%	    25%		50%	    75%
 *    96-128	   0%	    0%		25%	    50%
 */
#define DFL_RED_MODE		(1)

/* DSCPx_y_MAP_UP=0x60~0x7C */
#define DFL_DSCP0_7_UP		(0x00000000)	/* User priority of DSCP0~7 */
#define DFL_DSCP24_31_UP	(0x33333333)	/* User priority of DSCP24~31 */
#define DFL_DSCP8_15_UP		(0x11111111)	/* User priority of DSCP8~15 */
#define DFL_DSCP16_23_UP	(0x22222222)	/* User priority of DSCP16~23 */
#define DFL_DSCP32_39_UP	(0x44444444)	/* User priority of DSCP32~39 */
#define DFL_DSCP40_47_UP	(0x55555555)	/* User priority of DSCP40~47 */
#define DFL_DSCP48_55_UP	(0x66666666)	/* User priority of DSCP48~55 */
#define DFL_DSCP56_63_UP	(0x77777777)	/* User priority of DSCP56~63 */

/* AUTO_UP_CFG=0x80~0x84 */

/* 
 * 0	    ATUP_BND1	    ATUP_BND2	    ATUP_BND3		16K
 * |-------------|-------------|-----------------|--------------|
 *    ATUP_R1_UP    ATUP_R2_UP	    ATUP_R3_UP	    ATUP_R4_UP
 */
#define DFL_ATUP_BND1		(256)	/* packet length boundary 1 */
#define DFL_ATUP_BND2		(512)	/* packet length boundary 2 */
#define DFL_ATUP_BND3		(1024)	/* packet length boundary 3 */
#define DFL_ATUP_R1_UP		(0)	/* user priority of range 1 */
#define DFL_ATUP_R2_UP		(2)	/* user priority of range 2 */
#define DFL_ATUP_R3_UP		(4)	/* user priority of range 3 */
#define DFL_ATUP_R4_UP		(6)	/* user priority of range 4 */

/* UP_RES=0x88 */
#define PUP_WT_OFFSET		(0)	/* weight of port priority */
#define VUP_WT_OFFSET		(4)	/* weight of VLAN priority */
#define DUP_WT_OFFSET		(8)	/* weight of DSCP priority */
#define AUP_WT_OFFSET		(12)	/* weight of ACL priority */
#define FUP_WT_OFFSET		(16)	/* weight of FOE priority */
#define ATP_WT_OFFSET		(20)	/* weight of Auto priority */

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
#define DFL_UP_RES		 (7<<FUP_WT_OFFSET)	/* use user priority in foe entry */
#else
#define DFL_UP_RES		((0<<ATP_WT_OFFSET) | (0<<VUP_WT_OFFSET) |\
				 (7<<DUP_WT_OFFSET) | (0<<AUP_WT_OFFSET) | \
				 (0<<FUP_WT_OFFSET) | (0<<ATP_WT_OFFSET) )
#endif

/* UP_MAP_VPRI=0x8C */
#define DFL_UP0_VPRI		(0x0)	/* user pri 0 map to vlan pri tag */
#define DFL_UP3_VPRI		(0x3)	/* user pri 3 map to vlan pri tag */
#define DFL_UP1_VPRI		(0x1)	/* user pri 1 map to vlan pri tag */
#define DFL_UP2_VPRI		(0x2)	/* user pri 2 map to vlan pri tag */
#define DFL_UP4_VPRI		(0x4)	/* user pri 4 map to vlan pri tag */
#define DFL_UP5_VPRI		(0x5)	/* user pri 5 map to vlan pri tag */
#define DFL_UP6_VPRI		(0x6)	/* user pri 6 map to vlan pri tag */
#define DFL_UP7_VPRI		(0x7)	/* user pri 7 map to vlan pri tag */

/* UPx_y_MAP_IDSCP=0x90~0x94 */
#define DFL_UP0_IDSCP		(0x00)	/* user pri 0 map to in-profile DSCP */
#define DFL_UP3_IDSCP		(0x18)	/* user pri 3 map to in-profile DSCP */
#define DFL_UP1_IDSCP		(0x08)	/* user pri 1 map to in-profile DSCP */
#define DFL_UP2_IDSCP		(0x10)	/* user pri 2 map to in-profile DSCP */
#define DFL_UP4_IDSCP		(0x20)	/* user pri 4 map to in-profile DSCP */
#define DFL_UP5_IDSCP		(0x28)	/* user pri 5 map to in-profile DSCP */
#define DFL_UP6_IDSCP		(0x30)	/* user pri 6 map to in-profile DSCP */
#define DFL_UP7_IDSCP		(0x38)	/* user pri 7 map to in-profile DSCP */

/* UPx_y_MAP_ODSCP=0x98~0x9C */
#define DFL_UP0_ODSCP		(0x00)	/* user pri 0 map to out-profile DSCP */
#define DFL_UP3_ODSCP		(0x10)	/* user pri 3 map to out-profile DSCP */
#define DFL_UP1_ODSCP		(0x00)	/* user pri 1 map to out-profile DSCP */
#define DFL_UP2_ODSCP		(0x08)	/* user pri 2 map to out-profile DSCP */
#define DFL_UP4_ODSCP		(0x18)	/* user pri 4 map to out-profile DSCP */
#define DFL_UP5_ODSCP		(0x20)	/* user pri 5 map to out-profile DSCP */
#define DFL_UP6_ODSCP		(0x28)	/* user pri 6 map to out-profile DSCP */
#define DFL_UP7_ODSCP		(0x30)	/* user pri 7 map to out-profile DSCP */

/* UP_MAP_AC=0xA0 */
/* GDMA Q3 for CPU(slow path), Q2-Q0 for HNAT(fast path) */
#define DFL_UP0_AC		(0)	/* user pri 0 map to access category */
#define DFL_UP1_AC		(0)	/* user pri 1 map to access category */
#define DFL_UP2_AC		(0)	/* user pri 2 map to access category */
#define DFL_UP3_AC		(0)	/* user pri 3 map to access category */
#define DFL_UP4_AC		(1)	/* user pri 4 map to access category */
#define DFL_UP5_AC		(1)	/* user pri 5 map to access category */
#define DFL_UP6_AC		(2)	/* user pri 6 map to access category */
#define DFL_UP7_AC		(2)	/* user pri 7 map to access category */

#endif

/* 
 * PPE Flow Set 
 */
#if defined (CONFIG_HNAT_V2)
#ifdef TCSUPPORT_RA_HWNAT
#define BIT_IPV6_HASH_GRE_EN		(1<<20)	
#define BIT_IPV4_HASH_GRE_EN		(1<<19)	
#define BIT_L2_BRIDGE_EN			(1<<15)	
#define BIT_IPV4_NAT_FRAG_UDP_EN	(1<<7)
#define BIT_IPV4_NAT_FRAG_TCP_EN	(1<<6)	
#else
#define BIT_FBC_FOE		(1<<0)	/* PPE engine for broadcast flow */
#define BIT_FMC_FOE		(1<<1)	/* PPE engine for multicast flow */
#define BIT_FUC_FOE		(1<<2)	/* PPE engine for multicast flow */
#endif
#define BIT_IPV6_3T_ROUTE_EN	(1<<8)	/* IPv6 3-tuple route */
#define BIT_IPV6_5T_ROUTE_EN	(1<<9)	/* IPv6 5-tuple route */
#define BIT_IPV6_6RD_EN		(1<<10)	/* IPv6 6RD */
#define BIT_IPV4_NAT_EN		(1<<12)	/* IPv4 NAT */
#define BIT_IPV4_NAPT_EN	(1<<13)	/* IPv4 NAPT */
#define BIT_IPV4_DSL_EN		(1<<14)	/* IPv4 DS-Lite */
#define BIT_IP_PROT_CHK_BLIST	(1<<16)	/* IP protocol check is black/white list */
#define BIT_IPV4_NAT_FRAG_EN	(1<<17)	/* Enable fragment support for IPv4 NAT flow */
#define BIT_IPV6_HASH_FLAB	(1<<18)	/* For IPv6 5-tuple and 6RD flow, using flow label instead of sport and dport to do HASH */

#define IS_IPV6_FLAB_EBL()	(RegRead(PPE_FLOW_SET) & BIT_IPV6_HASH_FLAB) ? 1 : 0

#else
#define BIT_FBC_POSA		(1<<0)	/* post-account engine for broadcase flow */
#define BIT_FBC_POSM		(1<<1)	/* post-meter engine for broadcast flow */
#define BIT_FBC_FOE		(1<<2)	/* FOE engine for broadcast flow */
#define BIT_FBC_PREA		(1<<3)	/* pre-account engine for broadcast flow */
#define BIT_FBC_PREM		(1<<4)	/* pre-meter engine for broadcast flow */
#define BIT_FBC_ACL		(1<<5)	/* ACL engine for boardcast flow */

#define BIT_FMC_POSA		(1<<8)	/* post-account engine for broadcase flow */
#define BIT_FMC_POSM		(1<<9)	/* post-meter engine for multicast flow */
#define BIT_FMC_FOE		(1<<10)	/* FOE engine for multicast flow */
#define BIT_FMC_PREA		(1<<11)	/* pre-account engine for multicast flow */
#define BIT_FMC_PREM		(1<<12)	/* pre-meter engine for multicast flow */
#define BIT_FMC_ACL		(1<<13)	/* ACL engine for multicast flow */

#define BIT_FUC_POSA		(1<<16)	/* post-account engine for broadcase flow */
#define BIT_FUC_POSM	    	(1<<17)	/* post-meter engine for multicast flow */
#define BIT_FUC_FOE		(1<<18)	/* FOE engine for multicast flow */
#define BIT_FUC_PREA		(1<<19)	/* pre-account engine for multicast flow */
#define BIT_FUC_PREM		(1<<20)	/* pre-meter engine for multicast flow */
#define BIT_FUC_ACL		(1<<21)	/* ACL engine for boardcast flow */
#define BIT_IPV4_NAPT_EN        (1<<27)	/* Enable HNAPT engine for IPv4 flow */
#define BIT_IPV4_NAT_EN         (1<<26)	/* Enable HNAT engine for IPv4 flow */
#define BIT_IPV6_FOE_EN		(1<<24)	/* plicy engine for IPV6 */
#define BIT_IPV6_PE_EN		(1<<25)	/* FoE engine for IPV6 */

#define PRE_ACL_SIZE		CONFIG_RA_HW_NAT_PRE_ACL_SIZE
#define PRE_MTR_SIZE		CONFIG_RA_HW_NAT_PRE_MTR_SIZE
#define PRE_AC_SIZE		CONFIG_RA_HW_NAT_PRE_AC_SIZE
#define POST_MTR_SIZE		CONFIG_RA_HW_NAT_POST_MTR_SIZE
#define POST_AC_SIZE		CONFIG_RA_HW_NAT_POST_AC_SIZE

/* Account engine enable period (unit in 1ms) */
#define DFL_POL_AC_PRD		0xFFFF

#endif

/* 
 * PPE FOE Bind Rate 
 */
/* packet in a time stamp unit */
#define DFL_FOE_BNDR		CONFIG_RA_HW_NAT_BINDING_THRESHOLD

/* 
 * PPE_FOE_LMT 
 */
/* smaller than 1/4 of total entries */
#define DFL_FOE_QURT_LMT	CONFIG_RA_HW_NAT_QURT_LMT

/* between 1/2 and 1/4 of total entries */
#define DFL_FOE_HALF_LMT	CONFIG_RA_HW_NAT_HALF_LMT

/* between full and 1/2 of total entries */
#define DFL_FOE_FULL_LMT	CONFIG_RA_HW_NAT_FULL_LMT

/* 
 * PPE_FOE_KA 
 */
/* visit a FOE entry every FOE_KA_T * 1 msec */
#define DFL_FOE_KA_T		1

/* FOE_TCP_KA * FOE_KA_T * FOE_4TB_SIZ */
#define DFL_FOE_TCP_KA		CONFIG_RA_HW_NAT_TCP_KA

/* FOE_UDP_KA * FOE_KA_T * FOE_4TB_SIZ */
#define DFL_FOE_UDP_KA		CONFIG_RA_HW_NAT_UDP_KA

/* FOE_NTU_KA * FOE_KA_T * FOE_4TB_SIZ */
#define DFL_FOE_NTU_KA		CONFIG_RA_HW_NAT_NTU_KA

/* 
 * PPE_FOE_CFG 
 */
#if defined (CONFIG_RA_HW_NAT_HASH0)
#define DFL_FOE_HASH_MODE	0
#elif defined (CONFIG_RA_HW_NAT_HASH1)
#define DFL_FOE_HASH_MODE	1
#elif defined (CONFIG_RA_HW_NAT_HASH2)
#define DFL_FOE_HASH_MODE	2
#elif defined (CONFIG_RA_HW_NAT_HASH3)
#define DFL_FOE_HASH_MODE	3
#endif

#define HASH_SEED		0x12345678
#define DFL_FOE_UNB_AGE		1	/* Unbind state age enable */
#define DFL_FOE_TCP_AGE		1	/* Bind TCP age enable */
#define DFL_FOE_NTU_AGE		1	/* Bind TCP age enable */
#define DFL_FOE_UDP_AGE		1	/* Bind UDP age enable */
#define DFL_FOE_FIN_AGE		1	/* Bind TCP FIN age enable */
#define DFL_FOE_PBIND_AGE		1	/* Bind Pre-Bind age enable */

#ifdef TCSUPPORT_RA_HWNAT
#define DFL_BYTE_SWAP      (1)  /* 1:Enable byte swap, 0: Disable byte swap */
#endif

#if defined (CONFIG_HNAT_V2)
#define DFL_FOE_KA		3	/* 0:disable 1:unicast old 2: multicast new 3. duplicate old */
#else
#define DFL_FOE_KA		0	/* KeepAlive packet with (0:NewHeader,1:OrgHeader) */
#define DFL_FOE_KA_EN		1	/* Keep alive enable */
#endif

/* 
 * PPE_FOE_UNB_AGE 
 */
/*The min threshold of packet count for aging out at unbind state */
#define DFL_FOE_UNB_MNP		CONFIG_RA_HW_NAT_UNB_MNP
/* Delta time for aging out an ACL link to FOE entry */
#define DFL_FOE_ACL_DLTA        CONFIG_RA_HW_NAT_ACL_DLTA
/* Delta time for aging out an unbind FOE entry */
#define DFL_FOE_UNB_DLTA	CONFIG_RA_HW_NAT_UNB_DLTA


/* 
 * PPE_FOE_BND_AGE1 
 */
/* Delta time for aging out an bind UDP FOE entry */
#define DFL_FOE_UDP_DLTA	CONFIG_RA_HW_NAT_UDP_DLTA

/* 
 * PPE_FOE_BND_AGE2
 */
/* Delta time for aging out an bind TCP FIN entry */
#define DFL_FOE_FIN_DLTA 	CONFIG_RA_HW_NAT_FIN_DLTA
/* Delta time for aging out an bind TCP entry */
#define DFL_FOE_TCP_DLTA	CONFIG_RA_HW_NAT_TCP_DLTA
/* Delta time for aging out an bind Non-TCP/UDP FOE entry */
#define DFL_FOE_NTU_DLTA	CONFIG_RA_HW_NAT_NTU_DLTA

#define DFL_FOE_TTL_REGEN	1	/* TTL = TTL -1 */

/*++++++++++add for internet led when hw nat learn rule,packets hw forward+++++*/
/********begin**********/
#define FE_BASE     		0xBFB50000
#define FE_DMA_GLO_CFG      (FE_BASE + 0x00)
#define FE_RST_GLO          (FE_BASE + 0x04)
#define FE_INT_STATUS       (FE_BASE + 0x08)
#define FE_INT_ENABLE       (FE_BASE + 0x0c)
#define FOE_TS_T            (FE_BASE + 0x10)
#define IPV6_EXT            (FE_BASE + 0x14)

#define PDMA_BASE     		(FE_BASE + 0x0800)
#define TX_BASE_PTR(n)    	(PDMA_BASE + (n)*0x10 + 0x000)
#define TX_MAX_CNT(n)    	(PDMA_BASE + (n)*0x10 + 0x004)
#define TX_CTX_IDX(n)     	(PDMA_BASE + (n)*0x10 + 0x008)
#define TX_DTX_IDX(n) 		(PDMA_BASE + (n)*0x10 + 0x00C)

#define RX_BASE_PTR(n)     	(PDMA_BASE + (n)*0x10 + 0x100)
#define RX_MAX_CNT(n)  		(PDMA_BASE + (n)*0x10 + 0x104)
#define RX_CALC_IDX(n)     	(PDMA_BASE + (n)*0x10 + 0x108)
#define RX_DRX_IDX(n)      	(PDMA_BASE + (n)*0x10 + 0x10C)

#define PDMA_INFO        	(PDMA_BASE + 0x200)
#define PDMA_GLO_CFG     	(PDMA_BASE + 0x204)
#define PDMA_RST_IDX       	(PDMA_BASE + 0x208)
#define DLY_INT_CFG        	(PDMA_BASE + 0x20C)
#define FREEQ_THRES        	(PDMA_BASE + 0x210)
#define INT_STATUS         	(PDMA_BASE + 0x220)
#define INT_MASK           	(PDMA_BASE + 0x228)
#define SCH_Q01_CFG        	(PDMA_BASE + 0x280)
#define SCH_Q23_CFG        	(PDMA_BASE + 0x284)


#define PSE_BASE     		(FE_BASE + 0x0100)
#define PSE_FQFC_CFG        (PSE_BASE + 0x00)
#define PSE_IQ_REV1         (PSE_BASE + 0x08)
#define PSE_IQ_REV2         (PSE_BASE + 0x0c)
#define PSE_IQ_STA1        	(PSE_BASE + 0x10)
#define PSE_IQ_STA2        	(PSE_BASE + 0x14)
#define PSE_OQ_STA1         (PSE_BASE + 0x18)
#define PSE_OQ_STA2        	(PSE_BASE + 0x1c)
#define PSE_DROP_COUNT_0    (PSE_BASE + 0x80)
#define PSE_DROP_COUNT_1    (PSE_BASE + 0x84)
#define PSE_DROP_COUNT_2    (PSE_BASE + 0x88)
#define PSE_DROP_COUNT_4    (PSE_BASE + 0x90)
#define PSE_DROP_COUNT_5    (PSE_BASE + 0x94)

#define GDMA1_BASE     		(FE_BASE + 0x0500)
#define GDMA1_FWD_CFG       (GDMA1_BASE + 0x00)
#define GDMA1_SHRP_CFG      (GDMA1_BASE + 0x04)
#define GDMA1_MAC_ADRL      (GDMA1_BASE + 0x08)
#define GDMA1_MAC_ADRH      (GDMA1_BASE + 0x0c)
#define GDMA1_VLAN_GEN    (GDMA1_BASE + 0x10)

#define CDMA_BASE     		(FE_BASE + 0x400)
#define CDMA_CSG_CFG        (CDMA_BASE + 0x00)
#define CDMA_PPP_GEN        (CDMA_BASE + 0x04)

#define GDMA2_BASE     		(FE_BASE + 0x1500)
#define GDMA2_FWD_CFG       (GDMA2_BASE + 0x00)
#define GDMA2_SHRP_CFG      (GDMA2_BASE + 0x04)
#define GDMA2_MAC_ADRL      (GDMA2_BASE + 0x08)
#define GDMA2_MAC_ADRH      (GDMA2_BASE + 0x0c)
#define GDMA2_VLAN_CHECK    (GDMA2_BASE + 0x10)
#define GDMA2_MIB_CLR    	(GDMA2_BASE + 0x20)


//count define
#define GDMA_COUNT_BASE 	(FE_BASE + 0x2400)
#define GDMA_RX_BYTECNT_L   (GDMA_COUNT_BASE + 0x00)
#define GDMA_RX_BYTECNT_H   (GDMA_COUNT_BASE + 0x04)
#define GDMA_RX_PKTCNT      (GDMA_COUNT_BASE + 0x08)
#define GDMA_RX_OERCNT    	(GDMA_COUNT_BASE + 0x10)
#define GDMA_RX_FCSCNT    	(GDMA_COUNT_BASE + 0x14)
#define GDMA_RX_RUNTCNT    	(GDMA_COUNT_BASE + 0x18)
#define GDMA_RX_LONGCNT   	(GDMA_COUNT_BASE + 0x1c)
#define GDMA_RX_ITUCCNT    	(GDMA_COUNT_BASE + 0x20)
#define GDMA_RX_FCCNT    	(GDMA_COUNT_BASE + 0x24)
#define GDMA_TX_ABORTCNT   	(GDMA_COUNT_BASE + 0x28)
#define GDMA_TX_COLCNT   	(GDMA_COUNT_BASE + 0x2C)
#define GDMA_TX_BYTECNT_L   (GDMA_COUNT_BASE + 0x30)
#define GDMA_TX_BYTECNT_H   (GDMA_COUNT_BASE + 0x34)
#define GDMA_TX_PKTCNT      (GDMA_COUNT_BASE + 0x38)


#define GDMA2_COUNT_BASE 	(FE_BASE + 0x1600)
#define GDMA2_TX_GETCNT     (GDMA2_COUNT_BASE + 0x00)
#define GDMA2_TX_OKCNT     	(GDMA2_COUNT_BASE + 0x4)
#define GDMA2_TX_DROPCNT   	(GDMA2_COUNT_BASE + 0x8)

#define GDMA2_RX_OKCNT     	(GDMA2_COUNT_BASE + 0x50)
#define GDMA2_RX_OVDROPCNT    (GDMA2_COUNT_BASE + 0x54)
#define GDMA2_RX_ERRDROPCNT    (GDMA2_COUNT_BASE + 0x58)
#define GDMA2_RX_ETHERPCNT  (GDMA2_COUNT_BASE + 0x60)
#define GDMA2_RX_ETHDROPCNT (GDMA2_COUNT_BASE + 0x68)
#define GDMA2_RX_ETHBCCNT   (GDMA2_COUNT_BASE + 0x6c)
#define GDMA2_RX_ETHMCCNT   (GDMA2_COUNT_BASE + 0x70)
#define GDMA2_RX_ETHCRCCNT  (GDMA2_COUNT_BASE + 0x74)
#define GDMA2_RX_ETHFRACCNT (GDMA2_COUNT_BASE + 0x78)
#define GDMA2_RX_ETHJABCNT  (GDMA2_COUNT_BASE + 0x7c)
#define GDMA2_RX_ETHRUNTCNT (GDMA2_COUNT_BASE + 0x80)
#define GDMA2_RX_ETHLONGCNT (GDMA2_COUNT_BASE + 0x84)
/********end**********/
#endif
