/*
    Module Name:
    ra_nat.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2006-10-06      Initial version
*/

#ifndef _RA_NAT_WANTED
#define _RA_NAT_WANTED

#include "foe_fdb.h"
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/version.h>
#define KERNEL_2_6_36 		(LINUX_VERSION_CODE > KERNEL_VERSION(2,6,31))

/*
 * TYPEDEFS AND STRUCTURES
 */
enum DstPort {
#ifdef TCSUPPORT_RA_HWNAT
	DP_RA0 = 6,
	DP_RA1 = 7,
	DP_RA2 = 8,
	DP_RA3 = 9,
	DP_RA4 = 10,
	DP_RA5 = 11,
	DP_RA6 = 12,
	DP_RA7 = 13,
	DP_RAI0 = 15,
	DP_RAI1 = 16,
	DP_RAI2 = 17,
	DP_RAI3 = 18,
	DP_RAI4 = 19,
	DP_RAI5 = 20,
	DP_RAI6 = 21,
	DP_RAI7 = 22,
	DP_GMAC = 25,
	/*below for crypto mapping*/
	DP_CRYPTO_E_0 = 100,
   	DP_CRYPTO_E_1 = 101,
   	DP_CRYPTO_E_2 = 102,
   	DP_CRYPTO_E_3 = 103,
   	DP_CRYPTO_E_4 = 104,
   	DP_CRYPTO_E_5 = 105,
   	DP_CRYPTO_E_6 = 106,
   	DP_CRYPTO_E_7 = 107,
   	DP_CRYPTO_E_8 = 108,
   	DP_CRYPTO_E_9 = 109,
   	DP_CRYPTO_E_10 = 110,
   	DP_CRYPTO_E_11 = 111,
   	DP_CRYPTO_E_12 = 112,
   	DP_CRYPTO_E_13 = 113,
	DP_CRYPTO_E_14 = 114,
	DP_CRYPTO_E_15 = 115,
	DP_CRYPTO_E_MAX = 116,	//modify when ipsec max entry index changes
	/*117~199 reserved for vpn request for the coming future*/
  	DP_CRYPTO_D_0 = 200,
   	DP_CRYPTO_D_1 = 201,
   	DP_CRYPTO_D_2 = 202,
   	DP_CRYPTO_D_3 = 203,
   	DP_CRYPTO_D_4 = 204,
   	DP_CRYPTO_D_5 = 205,
   	DP_CRYPTO_D_6 = 206,
   	DP_CRYPTO_D_7 = 207,
   	DP_CRYPTO_D_8 = 208,
   	DP_CRYPTO_D_9 = 209,
   	DP_CRYPTO_D_10 = 210,
   	DP_CRYPTO_D_11 = 211,
   	DP_CRYPTO_D_12 = 212,
   	DP_CRYPTO_D_13 = 213,
	DP_CRYPTO_D_14 = 214,
	DP_CRYPTO_D_15 = 215,	
	DP_CRYPTO_D_MAX = 216,//modify when ipsec max entry index changes
	/*217~299 reserved for vpn request for the coming future*/
#else
#if defined (CONFIG_RT2860V2_AP_MBSS)
	DP_RA0 = 1,
	DP_RA1 = 2,
	DP_RA2 = 3,
	DP_RA3 = 4,
	DP_RA4 = 5,
	DP_RA5 = 6,
	DP_RA6 = 7,
	DP_RA7 = 8,
	DP_RA8 = 9,
	DP_RA9 = 10,
	DP_RA10 = 11,
	DP_RA11 = 12,
	DP_RA12 = 13,
	DP_RA13 = 14,
	DP_RA14 = 15,
	DP_RA15 = 16,
#endif // CONFIG_RT2860V2_AP_MBSS //
#if defined (CONFIG_RT2860V2_AP_WDS)
	DP_WDS0 = 17,
	DP_WDS1 = 18,
	DP_WDS2 = 19,
	DP_WDS3 = 20,
#endif // CONFIG_RT2860V2_AP_WDS //
#if defined (CONFIG_RT2860V2_AP_APCLI)
	DP_APCLI0 = 21,
#endif // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_MESH)
	DP_MESH0 = 22,
#endif // CONFIG_RT2860V2_AP_MESH //
	DP_RAI0 = 23,
#if defined (CONFIG_RT3090_AP_MBSS) || defined (CONFIG_RT5392_AP_MBSS) || \
    defined (CONFIG_RT3572_AP_MBSS) || defined (CONFIG_RT5572_AP_MBSS) || \
    defined (CONFIG_RT5592_AP_MBSS) || defined (CONFIG_RT3593_AP_MBSS)

	DP_RAI1 = 24,
	DP_RAI2 = 25,
	DP_RAI3 = 26,
	DP_RAI4 = 27,
	DP_RAI5 = 28,
	DP_RAI6 = 29,
	DP_RAI7 = 30,
	DP_RAI8 = 31,
	DP_RAI9 = 32,
	DP_RAI10 = 33,
	DP_RAI11 = 34,
	DP_RAI12 = 35,
	DP_RAI13 = 36,
	DP_RAI14 = 37,
	DP_RAI15 = 38,
#endif // CONFIG_RTDEV_AP_MBSS //
#if defined (CONFIG_RT3090_AP_WDS) || defined (CONFIG_RT5392_AP_WDS) || \
    defined (CONFIG_RT3572_AP_WDS) || defined (CONFIG_RT5572_AP_WDS) || \
    defined (CONFIG_RT5592_AP_WDS) || defined (CONFIG_RT3593_AP_WDS)
	DP_WDSI0 = 39,
	DP_WDSI1 = 40,
	DP_WDSI2 = 41,
	DP_WDSI3 = 42,
#endif // CONFIG_RTDEV_AP_WDS //
#if defined (CONFIG_RT3090_AP_APCLI) || defined (CONFIG_RT5392_AP_APCLI) || \
    defined (CONFIG_RT3572_AP_APCLI) || defined (CONFIG_RT5572_AP_APCLI) || \
    defined (CONFIG_RT5592_AP_APCLI) || defined (CONFIG_RT3593_AP_APCLI)
	DP_APCLII0 = 43,
#endif // CONFIG_RTDEV_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH) || defined (CONFIG_RT5392_AP_MESH) || \
    defined (CONFIG_RT3572_AP_MESH) || defined (CONFIG_RT5572_AP_MESH) || \
    defined (CONFIG_RT5592_AP_MESH) || defined (CONFIG_RT3593_AP_MESH)
	DP_MESHI0 = 44,
#endif // CONFIG_RTDEV_AP_MESH //
	DP_GMAC = 50,
	DP_GMAC2 = 51,
	DP_PCI = 52,
#endif	
	MAX_IF_NUM
};

typedef struct {
#if defined (CONFIG_RALINK_MT7620)
#ifdef __BIG_ENDIAN
	uint16_t MAGIC_TAG;
#ifdef TCSUPPORT_RA_HWNAT
	uint32_t RESV:3;
	uint32_t UDF:6;
	uint32_t SPORT:4;
#else
	uint32_t ALG:10;
	uint32_t SPORT:3;
#endif	
	uint32_t CRSN:5;
	uint32_t FOE_Entry:14;
#else
	uint16_t MAGIC_TAG;
	uint32_t FOE_Entry:14;
	uint32_t CRSN:5;
	uint32_t SPORT:3;
	uint32_t ALG:10;
#endif	
#else
	uint16_t MAGIC_TAG;
	uint32_t FOE_Entry:14;
	uint32_t FVLD:1;
	uint32_t ALG:1;
	uint32_t AI:8;
	uint32_t SP:3;
	uint32_t AIS:1;
	uint32_t RESV2:4;
#endif
}  __attribute__ ((packed)) PdmaRxDescInfo4;

typedef struct {
	//layer2 header
	uint8_t dmac[6];
	uint8_t smac[6];

	//vlan header 
	uint16_t vlan_tag;
	uint16_t vlan1_gap;
	uint16_t vlan1;
	uint16_t vlan2_gap;
	uint16_t vlan2;
	uint16_t vlan_layer;

	//pppoe header
	uint32_t pppoe_gap;
	uint16_t ppp_tag;
	uint16_t pppoe_sid;

	//layer3 header
	uint16_t eth_type;
	struct iphdr iph;
	struct ipv6hdr ip6h;

	//layer4 header
	struct tcphdr th;
	struct udphdr uh;

	uint32_t pkt_type;
#ifdef TCSUPPORT_RA_HWNAT	
	uint8_t rmt:1;
	uint8_t psn:1;
	uint8_t resv:6;
#endif

} PktParseResult;


/*
 * DEFINITIONS AND MACROS
 */
#ifndef NEXTHDR_IPIP
#define NEXTHDR_IPIP 4
#endif

/*
 *    2bytes	    4bytes 
 * +-----------+-------------------+
 * | Magic Tag | RX/TX Desc info4  |
 * +-----------+-------------------+
 * |<------FOE Flow Info---------->|
 */
#define FOE_INFO_LEN		    6
#ifndef TCSUPPORT_RA_HWNAT
#define FOE_MAGIC_PCI		    0x7273
#define FOE_MAGIC_WLAN		    0x7274
#define FOE_MAGIC_GE		    0x7275
#define FOE_MAGIC_PPE		    0x7276
#endif
/* choose one of them to keep HNAT related information in somewhere. */
//#define HNAT_USE_HEADROOM
//#define HNAT_USE_TAILROOM
//#define HNAT_USE_SKB_CB
#define HNAT_USE_SKB_FOE


#if defined (HNAT_USE_HEADROOM)
#define IS_SPACE_AVAILABLED(skb)    ((skb_headroom(skb) >= FOE_INFO_LEN) ? 1 : 0)
#define FOE_INFO_START_ADDR(skb)    (skb->head)

#if defined (CONFIG_HNAT_V2)
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->FOE_Entry
#ifndef TCSUPPORT_RA_HWNAT
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->ALG
#endif
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->CRSN
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->SPORT	//src_port or user priority
#else
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->head))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->head))->SP	//src_port or user priority
#endif

#elif defined (HNAT_USE_TAILROOM)

#define IS_SPACE_AVAILABLED(skb)    ((skb_tailroom(skb) >= FOE_INFO_LEN) ? 1 : 0)
#define FOE_INFO_START_ADDR(skb)    (skb->end - FOE_INFO_LEN)

#if defined (CONFIG_HNAT_V2)
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->FOE_Entry
#ifndef TCSUPPORT_RA_HWNAT
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->ALG
#endif
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->CRSN
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->SPORT //src_port or user priority
#else
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->end-FOE_INFO_LEN))->SP	//src_port or user priority
#endif

#elif defined (HNAT_USE_SKB_CB)
//change the position of skb_CB if necessary
#define CB_OFFSET		    32
#define IS_SPACE_AVAILABLED(skb)    1
#define FOE_INFO_START_ADDR(skb)    (skb->cb +  CB_OFFSET)

#if defined (CONFIG_HNAT_V2)
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->FOE_Entry
#ifndef TCSUPPORT_RA_HWNAT
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->ALG
#endif
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->CRSN
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->SPORT	//src_port or user priority
#else
#define FOE_MAGIC_TAG(skb)	    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)	    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->FOE_Entry
#define FOE_ALG(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->ALG
#define FOE_AI(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->AI
#define FOE_SP(skb)		    ((PdmaRxDescInfo4 *)((skb)->cb + CB_OFFSET))->SP	//src_port or user priority
#endif
#elif defined (HNAT_USE_SKB_FOE)
//change the position of skb_foe if necessary
#ifdef TCSUPPORT_RA_HWNAT
#define IS_SPACE_AVAILABLED(skb)    1
#define FOE_INFO_START_ADDR(skb)    (&(skb)->foe[0])
#define FOE_MAGIC_TAG(skb)          ((PdmaRxDescInfo4 *)(&(skb)->foe[0]))->MAGIC_TAG
#define FOE_ENTRY_NUM(skb)          ((PdmaRxDescInfo4 *)(&(skb)->foe[0]))->FOE_Entry
#define FOE_FVLD(skb)               ((PdmaRxDescInfo4 *)(&(skb)->foe[0]))->FVLD
#define FOE_UDF(skb)                ((PdmaRxDescInfo4 *)(&(skb)->foe[0]))->UDF
#define FOE_AI(skb)                 ((PdmaRxDescInfo4 *)(&(skb)->foe[0]))->CRSN
#define FOE_SP(skb)                 ((PdmaRxDescInfo4 *)(&(skb)->foe[0]))->SPORT //src_port or user priority
#define FOE_AIS(skb)                ((PdmaRxDescInfo4 *)(&(skb)->foe[0]))->AIS
#endif
#endif


#ifdef TCSUPPORT_RA_HWNAT

#ifdef TCSUPPORT_IPSEC_PASSTHROUGH
#define IS_MAGIC_TAG_VALID(skb)	    ((FOE_MAGIC_TAG(skb) == FOE_MAGIC_GE)   || \
				    (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN) || \
				    (FOE_MAGIC_TAG(skb) == FOE_MAGIC_ATM) || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_PTM) || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_EPON) || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_CRYPTO_E_1) || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_CRYPTO_D_1) || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_CRYPTO_E_2) || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_CRYPTO_D_2) || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_GPON))
#else
#define IS_MAGIC_TAG_VALID(skb)	    ((FOE_MAGIC_TAG(skb) == FOE_MAGIC_GE)   || \
				    (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN) || \
				    (FOE_MAGIC_TAG(skb) == FOE_MAGIC_ATM) || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_PTM) || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_EPON) || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_GPON))
#endif
#if defined(CONFIG_RA_HW_NAT_PREBIND)
#define PPESETPREBIND(x)	((x)->udib1.preb = 1)
#endif
#else				    
#define IS_MAGIC_TAG_VALID(skb)	    ((FOE_MAGIC_TAG(skb) == FOE_MAGIC_PCI) || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_GE)   || \
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN))

#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
#define LAYER2_HEADER(skb)		(skb)->mac_header
#else
#define LAYER2_HEADER(skb)		(skb)->mac.raw
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
#define LAYER3_HEADER(skb)		(skb)->network_header
#else
#define LAYER3_HEADER(skb)		(skb)->nh.raw
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
#define LAYER4_HEADER(skb)		(skb)->transport_header
#else
#define LAYER4_HEADER(skb)		(skb)->h.raw
#endif


/*
 * EXPORT FUNCTION
 */
int32_t GetPppoeSid(struct sk_buff *skb, uint32_t vlan_gap, uint16_t * sid, uint16_t * ppp_tag);

int PpeSetBindThreshold(uint32_t threshold);
int PpeSetMaxEntryLimit(uint32_t full, uint32_t half, uint32_t qurt);
int PpeSetRuleSize(uint16_t pre_acl, uint16_t pre_meter, uint16_t pre_ac,
		   uint16_t post_meter, uint16_t post_ac);

int PpeSetKaInterval(uint8_t tcp_ka, uint8_t udp_ka);
int PpeSetUnbindLifeTime(uint8_t lifetime);
#ifdef TCSUPPORT_RA_HWNAT
int PpeSetBindLifetime(uint16_t tcp_fin, uint16_t udp_life, uint16_t fin_life, uint16_t ntu_life);
void PpeRegDump(void);
extern unsigned char multicast_en;
extern short int ip_proto_chk;
void setup_ip_chk(char ip_chk_type);
int32_t PpeCleanTableHandler(void);

#else
int PpeSetBindLifetime(uint16_t tcp_fin, uint16_t udp_life, uint16_t fin_life);
#endif

#endif
