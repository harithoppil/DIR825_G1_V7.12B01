/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

  Module Name:
  ra_nat.c

  Abstract:

  Revision History:
  Who         When            What
  --------    ----------      ----------------------------------------------
  Name        Date            Modification logs
  Steven Liu  2011-04-11      Support RT63365
  Steven Liu  2011-02-08      Support IPv6 over PPPoE
  Steven Liu  2010-11-25      Fix double VLAN + PPPoE header bug
  Steven Liu  2010-11-24      Support upstream/downstream/bi-direction acceleration
  Steven Liu  2010-11-17      Support Linux 2.6.36 kernel
  Steven Liu  2010-07-13      Support DSCP to User Priority helper
  Steven Liu  2010-06-03      Support skb headroom/tailroom/cb to keep HNAT information
  Kurtis Ke   2010-03-30      Support HNAT parameter can be changed by application
  Steven Liu  2010-04-08      Support RT3883 + RT309x concurrent AP
  Steven Liu  2010-03-01      Support RT3352
  Steven Liu  2009-11-26      Support WiFi pseudo interface by using VLAN tag
  Steven Liu  2009-07-21      Support IPV6 Forwarding
  Steven Liu  2009-04-02      Support RT3883/RT3350
  Steven Liu  2008-03-19      Support RT3052
  Steven Liu  2007-09-25      Support RT2880 MP
  Steven Liu  2006-10-06      Initial version
 *
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <asm/string.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <asm/checksum.h>
#include <linux/pci.h>
#include <linux/etherdevice.h>
#ifdef TCSUPPORT_RA_HWNAT
#include <linux/foe_hook.h>
#endif

#include "ra_nat.h"
#include "foe_fdb.h"
#include "frame_engine.h"
#include "hwnat_ioctl.h"
#include "acl_ioctl.h"
#include "ac_ioctl.h"
#include "acl_policy.h"
#include "mtr_policy.h"
#include "ac_policy.h"
#include "util.h"
#include "ra_rfrw.h"

#define ETH_P_STAG0		0x8000          /* special tag */
#define ETH_P_STAG1		0x8100          /* special tag */
#define ETH_P_STAG_MASK	0xff00          /* special tag mask */

#if !defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
#define LAN_PORT_VLAN_ID	CONFIG_RA_HW_NAT_LAN_VLANID
#define WAN_PORT_VLAN_ID	CONFIG_RA_HW_NAT_WAN_VLANID
#endif

#ifndef TCSUPPORT_RA_HWNAT
extern int (*ra_sw_nat_hook_rx) (struct sk_buff * skb);
extern int (*ra_sw_nat_hook_tx) (struct sk_buff * skb, int gmac_no);
#endif
extern unsigned char bind_dir;
extern unsigned char multicast_en;

struct FoeEntry    *PpeFoeBase;
dma_addr_t	    PpePhyFoeBase;
struct net_device  *DstPort[MAX_IF_NUM];
uint32_t	    DebugLevel=0;
uint32_t	    ChipVer=0;
uint32_t	    ChipId=0;
uint32_t	    feSTag=0;
#if 0  //we cannot use GPL-only symbol
struct class	   *hnat_class;
#endif

uint16_t GLOBAL_PRE_ACL_STR  = DFL_PRE_ACL_STR; 
uint16_t GLOBAL_PRE_ACL_END  = DFL_PRE_ACL_END; 
uint16_t GLOBAL_PRE_MTR_STR  = DFL_PRE_MTR_STR; 
uint16_t GLOBAL_PRE_MTR_END  = DFL_PRE_MTR_END; 
uint16_t GLOBAL_PRE_AC_STR   = DFL_PRE_AC_STR; 
uint16_t GLOBAL_PRE_AC_END   = DFL_PRE_AC_END; 
uint16_t GLOBAL_POST_MTR_STR = DFL_POST_MTR_STR; 
uint16_t GLOBAL_POST_MTR_END = DFL_POST_MTR_END; 
uint16_t GLOBAL_POST_AC_STR  = DFL_POST_AC_STR; 
uint16_t GLOBAL_POST_AC_END  = DFL_POST_AC_END; 

#if 0
void skb_dump(struct sk_buff* sk) {
        unsigned int i;

        printk("\nskb_dump: from %s with len %d (%d) headroom=%d tailroom=%d\n",
                sk->dev?sk->dev->name:"ip stack",sk->len,sk->truesize,
                skb_headroom(sk),skb_tailroom(sk));

        for(i=(unsigned int)sk->head;i<(unsigned int)sk->tail;i++) {
                if((i % 16) == 0)
		    printk("\n");

                if(i==(unsigned int)sk->head) printk("@h");
                if(i==(unsigned int)sk->data) printk("@d");
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
                if(i==(unsigned int)sk->mac_header) printk("*");
#else
                if(i==(unsigned int)sk->mac.raw) printk("@m");
#endif
                printk("%02X-",*((unsigned char*)i));
        }
        printk("\n");
}
#endif

int RemoveVlanTag(struct sk_buff **pskb)
{
    struct ethhdr *eth;
    struct vlan_ethhdr *veth;
    uint16_t VirIfIdx;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
    veth = (struct vlan_ethhdr *)((*pskb)->mac_header);
#else
    veth = (struct vlan_ethhdr *)((*pskb)->mac.raw);
#endif

    //something wrong
    if(veth->h_vlan_proto != htons(ETH_P_8021Q)) {
		printk("HNAT: Reentry packet is untagged frame?\n");
		return 65535;
    }

    VirIfIdx = ntohs(veth->h_vlan_TCI);

	if ((VirIfIdx >= MAX_IF_NUM) || (DstPort[VirIfIdx] == NULL))
		return 65535;

    if (skb_cloned(*pskb) || skb_shared(*pskb)) {
		struct sk_buff *new_skb;

		new_skb = skb_copy(*pskb, GFP_ATOMIC);
		if (!new_skb)
			return 65535;
		kfree_skb(*pskb);
		*pskb = new_skb;
    }

    /* remove VLAN tag */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
    (*pskb)->data= (*pskb)->mac_header;
    (*pskb)->mac_header += VLAN_HLEN;
    memmove((*pskb)->mac_header, (*pskb)->data, ETH_ALEN * 2);
#else
    (*pskb)->data= (*pskb)->mac.raw;
    (*pskb)->mac.raw += VLAN_HLEN;
    memmove((*pskb)->mac.raw, (*pskb)->data, ETH_ALEN * 2);
#endif
    skb_pull(*pskb, VLAN_HLEN);
    (*pskb)->data += ETH_HLEN;  //pointer to layer3 header

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
    eth = (struct ethhdr *)((*pskb)->mac_header);
#else
    eth = (struct ethhdr *)((*pskb)->mac.raw);
#endif
    (*pskb)->protocol = eth->h_proto;

    return VirIfIdx;
}

static void FoeAllocTbl(uint32_t NumOfEntry)
{
    uint32_t FoeTblSize;

    FoeTblSize = NumOfEntry * sizeof(struct FoeEntry);
    PpeFoeBase = dma_alloc_coherent(NULL, FoeTblSize, &PpePhyFoeBase, GFP_KERNEL);

    RegWrite(PPE_FOE_BASE, PpePhyFoeBase);
    memset(PpeFoeBase, 0, FoeTblSize);
}

#if !defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
static uint8_t *ShowCpuReason(struct sk_buff *skb)
{
    static uint8_t Buf[32];

    switch(FOE_AI(skb))
    {
    case TTL_0: /* 0x80 */
	return("TTL=0\n");
    case FOE_EBL_NOT_IPV4_HLEN5: /* 0x90 */
	return("FOE enable & not IPv4h5nf\n");
    case FOE_EBL_NOT_TCP_UDP_L4_READY: /* 0x91 */
	return("FOE enable & not TCP/UDP/L4_read\n");
    case TCP_SYN_FIN_RST: /* 0x92 */
	return("TCP SYN/FIN/RST\n");
    case UN_HIT: /* 0x93 */
	return("Un-hit\n");
    case HIT_UNBIND: /* 0x94 */
	return("Hit unbind\n");
    case HIT_UNBIND_RATE_REACH: /* 0x95 */
	return("Hit unbind & rate reach\n");
    case HIT_FIN:  /* 0x96 */
	return("Hit fin\n");
    case HIT_BIND_TTL_1: /* 0x97 */
	return("Hit bind & ttl=1 & ttl-1\n");
    case HIT_BIND_KEEPALIVE:  /* 0x98 */
	return("Hit bind & keep alive\n");
    case HIT_BIND_FORCE_TO_CPU: /* 0x99 */
	return("Hit bind & force to CPU\n");
    case ACL_FOE_TBL_ERR: /* 0x9A */
	return("acl link foe table error (!static & !unbind)\n");
    case ACL_TBL_TTL_1: /* 0x9B */
	return("acl link FOE table & TTL=1 & TTL-1\n");
    case ACL_ALERT_CPU: /* 0x9C */
	return("acl alert cpu\n");
    case NO_FORCE_DEST_PORT: /* 0xA0 */
	return("No force destination port\n");
    case ACL_FORCE_PRIORITY0: /* 0xA8 */
	return("ACL FORCE PRIORITY0\n");
    case ACL_FORCE_PRIORITY1: /* 0xA9 */
	return("ACL FORCE PRIORITY1\n");
    case ACL_FORCE_PRIORITY2: /* 0xAA */
	return("ACL FORCE PRIORITY2\n");
    case ACL_FORCE_PRIORITY3: /* 0xAB */
	return("ACL FORCE PRIORITY3\n");
    case ACL_FORCE_PRIORITY4: /* 0xAC */
	return("ACL FORCE PRIORITY4\n");
    case ACL_FORCE_PRIORITY5: /* 0xAD */
	return("ACL FORCE PRIORITY5\n");
    case ACL_FORCE_PRIORITY6: /* 0xAE */
	return("ACL FORCE PRIORITY6\n");
    case ACL_FORCE_PRIORITY7: /* 0xAF */
	return("ACL FORCE PRIORITY7\n");
    case EXCEED_MTU: /* 0xA1 */
	return("Exceed mtu\n");
    }

    sprintf(Buf,"CPU Reason Error - %X\n",FOE_AI(skb));
    return(Buf);
}


uint32_t FoeDumpPkt(struct sk_buff *skb)
{
#if 0  //dump related info from packet
    struct ethhdr *eth = NULL;
    struct vlan_hdr *vh1 = NULL;
    struct vlan_hdr *vh2 = NULL;
    struct iphdr *iph = NULL;
    struct tcphdr *th = NULL;
    struct udphdr *uh = NULL;

    uint32_t vlan1_gap = 0;
    uint32_t vlan2_gap = 0;
    uint32_t pppoe_gap=0;
    uint16_t pppoe_sid = 0;
    uint16_t eth_type=0;
    

    NAT_PRINT("\nRx===<FOE_Entry=%d>=====\n",FOE_ENTRY_NUM(skb)); 
    NAT_PRINT("RcvIF=%s\n", skb->dev->name);
    NAT_PRINT("FOE_Entry=%d\n",FOE_ENTRY_NUM(skb));
    NAT_PRINT("FVLD=%d\n",FOE_FVLD(skb));
    NAT_PRINT("CPU Reason=%s",ShowCpuReason(skb));
    NAT_PRINT("ALG=%d\n",FOE_ALG(skb));
    NAT_PRINT("SP=%d\n",FOE_SP(skb));
    NAT_PRINT("AIS=%d\n",FOE_AIS(skb));


    eth_type=ntohs(skb->protocol);

    // Layer 2
    if(eth_type==ETH_P_8021Q) {
	vlan1_gap = VLAN_HLEN;
	vh1 = (struct vlan_hdr *)(skb->data);

	/* VLAN + PPPoE */
	if(ntohs(vh1->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
	    pppoe_gap = 8;
	    if (GetPppoeSid(skb, vlan1_gap, &pppoe_sid, 0)) {
		return 0;
	    }
	    /* Double VLAN = VLAN + VLAN */
	}else if(ntohs(vh1->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
	    vlan2_gap = VLAN_HLEN;
	    vh2 = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

	    /* VLAN + VLAN + PPPoE */
	    if(ntohs(vh2->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
		pppoe_gap = 8;
		if (GetPppoeSid(skb, (vlan1_gap + vlan2_gap), &pppoe_sid, 0)) {
		    return 0;
		}
		/* VLAN + VLAN + IP */
	    }else if(ntohs(vh2->h_vlan_encapsulated_proto)!=ETH_P_IP) {
		return 0;
	    }
	    /* VLAN + IP */
	}else if(ntohs(vh1->h_vlan_encapsulated_proto)!=ETH_P_IP) {
	    return 0;
	}
    }else if(eth_type != ETH_P_IP) {
	return 0;
    }
    
    eth = (struct ethhdr *)(skb->data-14) ; /* DA + SA + ETH_TYPE */

    // Layer 3
    iph = (struct iphdr *) (skb->data + vlan1_gap + vlan2_gap + pppoe_gap);


    // Layer 4
    if(iph->protocol==IPPROTO_TCP) {
	th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
    }else if(iph->protocol==IPPROTO_UDP) {
	uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
    }else { //Not TCP or UDP
	return 0;
    }

    if(vlan1_gap) {
	NAT_PRINT("VLAN1: %d\n",ntohs(vh1->h_vlan_TCI));
    }
    if(vlan2_gap) {
	NAT_PRINT("VLAN2: %d\n",ntohs(vh2->h_vlan_TCI));
    }
    if(pppoe_gap) {
	NAT_PRINT("PPPoE Session ID: %d\n", ntohs(pppoe_sid));
    }
    
    NAT_PRINT("----------------------------------\n");
    NAT_PRINT("SrcMac=%0X:%0X:%0X:%0X:%0X:%0X\n",MAC_ARG(eth->h_source));
    NAT_PRINT("DstMac=%0X:%0X:%0X:%0X:%0X:%0X\n",MAC_ARG(eth->h_dest));
    NAT_PRINT("SrcIp:%s\n",Ip2Str(ntohl(iph->saddr)));
    NAT_PRINT("DstIp:%s\n",Ip2Str(ntohl(iph->daddr)));
    if(th!=NULL) {
		NAT_PRINT("SrcPort:%d Dstport:%d\n",ntohs(th->source),ntohs(th->dest));
    }else {
		NAT_PRINT("SrcPort:%d Dstport:%d\n",ntohs(uh->source),ntohs(uh->dest));
    }

#else //dump related info from FoE table
    struct FoeEntry *foe_entry = &PpeFoeBase[FOE_ENTRY_NUM(skb)];

    NAT_PRINT("\nRx===<FOE_Entry=%d>=====\n",FOE_ENTRY_NUM(skb)); 
    NAT_PRINT("RcvIF=%s\n", skb->dev->name);
    NAT_PRINT("FOE_Entry=%d\n",FOE_ENTRY_NUM(skb));
    NAT_PRINT("FVLD=%d\n",FOE_FVLD(skb));
    NAT_PRINT("CPU Reason=%s",ShowCpuReason(skb));
    NAT_PRINT("ALG=%d\n",FOE_ALG(skb));
    NAT_PRINT("SP=%d\n",FOE_SP(skb));
    NAT_PRINT("AIS=%d\n",FOE_AIS(skb));

    NAT_PRINT("Information Block 1=%x\n",foe_entry->info_blk1);

    if(foe_entry->bfib1.fmt == IPV4_NAPT) {
		NAT_PRINT("SIP=%s\n",Ip2Str(foe_entry->sip));
		NAT_PRINT("DIP=%s\n",Ip2Str(foe_entry->dip));
		NAT_PRINT("SPORT=%d\n",foe_entry->sport);
		NAT_PRINT("DPORT=%d\n",foe_entry->dport);
    }else if(foe_entry->bfib1.fmt == IPV4_NAT) {
		NAT_PRINT("SIP=%s\n",Ip2Str(foe_entry->sip));
		NAT_PRINT("DIP=%s\n",Ip2Str(foe_entry->dip));
    }else if(foe_entry->bfib1.fmt == IPV6_ROUTING) {
		NAT_PRINT("IPv6_DIP0=%08X\n", foe_entry->ipv6_dip0);
		NAT_PRINT("IPv6_DIP1=%08X\n", foe_entry->ipv6_dip1);
		NAT_PRINT("IPv6_DIP2=%08X\n", foe_entry->ipv6_dip2);
		NAT_PRINT("IPv6_DIP3=%08X\n", foe_entry->ipv6_dip3);
    } else {
		NAT_PRINT("Wrong MFT value\n");
    }
#endif
    NAT_PRINT("==================================\n");

    return 1;

}

int32_t PpeRxHandler(struct sk_buff * skb)
{
    struct ethhdr *eth=NULL;
    struct vlan_hdr *vh = NULL;
    struct iphdr *iph = NULL;
    struct tcphdr *th = NULL;
    struct udphdr *uh = NULL;
    struct FoeEntry *foe_entry=NULL;
    uint32_t vlan1_gap = 0;
    uint32_t vlan2_gap = 0;
    uint32_t pppoe_gap=0;
    uint16_t eth_type=0;
#if defined(CONFIG_RA_HW_NAT_WIFI) || defined(CONFIG_RA_HW_NAT_ATM)
    uint32_t SrcPortNo=0;
    uint16_t VirIfIdx=0;
#endif

    foe_entry=&PpeFoeBase[FOE_ENTRY_NUM(skb)];
    eth_type=ntohs(skb->protocol);

    if(DebugLevel==1) {
       FoeDumpPkt(skb);
    }

    if(((FOE_MAGIC_TAG(skb) == FOE_MAGIC_PCI) ||
			    (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN)) ||
					(FOE_MAGIC_TAG(skb) == FOE_MAGIC_ATM)){ 

#if defined(CONFIG_RA_HW_NAT_WIFI) || defined(CONFIG_RA_HW_NAT_ATM)


	    /* PPE only can handle IPv4/VLAN/IPv6/PPP packets */
	    if(skb->protocol != htons(ETH_P_IP) && skb->protocol != htons(ETH_P_8021Q)  &&
	       skb->protocol != htons(ETH_P_IPV6) && skb->protocol != htons(ETH_P_PPP_SES) &&
	       skb->protocol != htons(ETH_P_PPP_DISC)) {
			return 1;
	    }

	    if(skb->dev == DstPort[DP_RA0]) { VirIfIdx=DP_RA0;}
#ifdef TCSUPPORT_RA_HWNAT
	    else if(skb->dev == DstPort[DP_RA1]) { VirIfIdx=DP_RA1; }
#endif
#if defined (CONFIG_RT2860V2_AP_MBSS)
	    else if(skb->dev == DstPort[DP_RA1]) { VirIfIdx=DP_RA1; }
	    else if(skb->dev == DstPort[DP_RA2]) { VirIfIdx=DP_RA2; }
	    else if(skb->dev == DstPort[DP_RA3]) { VirIfIdx=DP_RA3; }
	    else if(skb->dev == DstPort[DP_RA4]) { VirIfIdx=DP_RA4; }
	    else if(skb->dev == DstPort[DP_RA5]) { VirIfIdx=DP_RA5; }
	    else if(skb->dev == DstPort[DP_RA6]) { VirIfIdx=DP_RA6; }
	    else if(skb->dev == DstPort[DP_RA7]) { VirIfIdx=DP_RA7; }
	    else if(skb->dev == DstPort[DP_RA8]) { VirIfIdx=DP_RA8; }
	    else if(skb->dev == DstPort[DP_RA9]) { VirIfIdx=DP_RA9; }
	    else if(skb->dev == DstPort[DP_RA10]) { VirIfIdx=DP_RA10; }
	    else if(skb->dev == DstPort[DP_RA11]) { VirIfIdx=DP_RA11; }
	    else if(skb->dev == DstPort[DP_RA12]) { VirIfIdx=DP_RA12; }
	    else if(skb->dev == DstPort[DP_RA13]) { VirIfIdx=DP_RA13; }
	    else if(skb->dev == DstPort[DP_RA14]) { VirIfIdx=DP_RA14; }
	    else if(skb->dev == DstPort[DP_RA15]) { VirIfIdx=DP_RA15; }
#endif // CONFIG_RT2860V2_AP_MBSS //
#if defined (CONFIG_RT2860V2_AP_WDS)
	    else if(skb->dev == DstPort[DP_WDS0]) { VirIfIdx=DP_WDS0; }
	    else if(skb->dev == DstPort[DP_WDS1]) { VirIfIdx=DP_WDS1; }
	    else if(skb->dev == DstPort[DP_WDS2]) { VirIfIdx=DP_WDS2; }
	    else if(skb->dev == DstPort[DP_WDS3]) { VirIfIdx=DP_WDS3; }
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
	    else if(skb->dev == DstPort[DP_APCLI0]) { VirIfIdx=DP_APCLI0; }
#endif // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_MESH)
	    else if(skb->dev == DstPort[DP_MESH0]) { VirIfIdx=DP_MESH0; }
#endif // CONFIG_RT2860V2_AP_MESH //
#if defined (CONFIG_RT3090_AP) || defined (CONFIG_RT3090_AP_MODULE)
	    else if(skb->dev == DstPort[DP_RAI0]) { VirIfIdx=DP_RAI0; }
#if defined (CONFIG_RT3090_AP_MBSS)
	    else if(skb->dev == DstPort[DP_RAI1]) { VirIfIdx=DP_RAI1; }
	    else if(skb->dev == DstPort[DP_RAI2]) { VirIfIdx=DP_RAI2; }
	    else if(skb->dev == DstPort[DP_RAI3]) { VirIfIdx=DP_RAI3; }
	    else if(skb->dev == DstPort[DP_RAI4]) { VirIfIdx=DP_RAI4; }
	    else if(skb->dev == DstPort[DP_RAI5]) { VirIfIdx=DP_RAI5; }
	    else if(skb->dev == DstPort[DP_RAI6]) { VirIfIdx=DP_RAI6; }
	    else if(skb->dev == DstPort[DP_RAI7]) { VirIfIdx=DP_RAI7; }
	    else if(skb->dev == DstPort[DP_RAI8]) { VirIfIdx=DP_RAI8; }
	    else if(skb->dev == DstPort[DP_RAI9]) { VirIfIdx=DP_RAI9; }
	    else if(skb->dev == DstPort[DP_RAI10]) { VirIfIdx=DP_RAI10; }
	    else if(skb->dev == DstPort[DP_RAI11]) { VirIfIdx=DP_RAI11; }
	    else if(skb->dev == DstPort[DP_RAI12]) { VirIfIdx=DP_RAI12; }
	    else if(skb->dev == DstPort[DP_RAI13]) { VirIfIdx=DP_RAI13; }
	    else if(skb->dev == DstPort[DP_RAI14]) { VirIfIdx=DP_RAI14; }
	    else if(skb->dev == DstPort[DP_RAI15]) { VirIfIdx=DP_RAI15; }
#endif // CONFIG_RT3090_AP_MBSS //
#endif
#if defined (CONFIG_RT3090_AP_APCLI)
	    else if(skb->dev == DstPort[DP_APCLII0]) { VirIfIdx=DP_APCLII0; }
#endif // CONFIG_RT3090_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH)
	    else if(skb->dev == DstPort[DP_MESHI0]) { VirIfIdx=DP_MESHI0; }
#endif // CONFIG_RT3090_AP_MESH //
	    else if(skb->dev == DstPort[DP_PCI]) { VirIfIdx=DP_PCI; }
#if defined(CONFIG_RA_HW_NAT_ATM)
	    else if(skb->dev == DstPort[DP_NAS0]) { VirIfIdx=DP_NAS0; }
	    else if(skb->dev == DstPort[DP_NAS1]) { VirIfIdx=DP_NAS1; }
	    else if(skb->dev == DstPort[DP_NAS2]) { VirIfIdx=DP_NAS2; }
	    else if(skb->dev == DstPort[DP_NAS3]) { VirIfIdx=DP_NAS3; }
	    else if(skb->dev == DstPort[DP_NAS4]) { VirIfIdx=DP_NAS4; }
	    else if(skb->dev == DstPort[DP_NAS5]) { VirIfIdx=DP_NAS5; }
	    else if(skb->dev == DstPort[DP_NAS6]) { VirIfIdx=DP_NAS6; }
	    else if(skb->dev == DstPort[DP_NAS7]) { VirIfIdx=DP_NAS7; }
#endif
	    else { 		
			if(1 == DebugLevel) 
				printk("HNAT: The interface %s is unknown\n", skb->dev->name); 
			return 1;
		}

	    //push vlan tag to stand for actual incoming interface,
	    //so HNAT module can know the actual incoming interface from vlan id.
	    skb_push(skb, ETH_HLEN); //pointer to layer2 header before calling hard_start_xmit
	    skb = __vlan_put_tag(skb, VirIfIdx);
		if (skb == NULL)
			return 0;

	    //redirect to PPE
	    FOE_AI(skb) = UN_HIT;
	    FOE_MAGIC_TAG(skb) = FOE_MAGIC_PPE;
	    skb->dev = DstPort[DP_GMAC]; //we use GMAC1 to send to packet to PPE
		#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	    skb->dev->netdev_ops->ndo_start_xmit(skb,skb->dev);
		#else
	    skb->dev->hard_start_xmit(skb, skb->dev);
		#endif
#else
	    return 1;
#endif // CONFIG_RA_HW_NAT_WIFI //

	    return 0;

    }

    /* It means the flow is already in binding state, just transfer to output interface 
     * rax<->raix binded traffic: HIT_BIND_FORCE_TO_CPU + FOE_AIS=1 + FOE_SP = 0 or 6
     */
    if((FOE_AI(skb)==HIT_BIND_FORCE_TO_CPU)) {
#ifdef TCSUPPORT_RA_HWNAT
	    skb->dev = DstPort[foe_entry->act_dp];
#else
	    skb->dev = DstPort[foe_entry->iblk2.act_dp];
#endif
	    skb_push(skb, ETH_HLEN); //pointer to layer2 header
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	    skb->dev->netdev_ops->ndo_start_xmit(skb,skb->dev);
#else
	    skb->dev->hard_start_xmit(skb, skb->dev);
#endif
	    return 0;
    }

#if defined(CONFIG_RA_HW_NAT_WIFI) || defined(CONFIG_RA_HW_NAT_ATM)
    /* 
     * RT3883/RT3352/RT6855:
     * If FOE_AIS=1 and FOE_SP=0/6, it means this is reentry packet.
     * (WLAN->CPU->PPE->CPU or PCI->CPU->PPE->CPU)
     *
     *    Incoming  |   SP[2:0]   |  SP[2:0]
     *      Port    | EXT_SW_EN=1 | EXT_SW_EN=0
     *  ------------+-------------+------------
     *       P0	    |	  0	  |	1
     *       P1	    |	  1	  |	1
     *       P2	    |	  2	  |	1
     *       P3     |     3       |     1
     *	     P4     |     4       |     1
     *       P5     |     5*      |     1
     *      PDMA    |     6*      |     0
     *       GE1    |     N/A     |     1
     *       GE2    |     5*      |     2
     */
#if defined(CONFIG_RALINK_RT3883) || defined(CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT6855) || defined(TCSUPPORT_RA_HWNAT)
    if(IS_EXT_SW_EN(RegRead(FE_COS_MAP))){
		SrcPortNo=6;
    }
#endif

/*
 added by pengyao 20130326
 resolve hwnat keeplive issue
*/
    if((FOE_AIS(skb) == 1) && (FOE_SP(skb) == SrcPortNo)/* && (FOE_AI(skb)!=HIT_BIND_KEEPALIVE)*/) {

		VirIfIdx = RemoveVlanTag(&skb);

		//recover to right incoming interface
		if(VirIfIdx < MAX_IF_NUM) {
			skb->dev=DstPort[VirIfIdx];
		}else {
			printk("HNAT: unknow interface (VirIfIdx=%d)\n", VirIfIdx);
			dev_kfree_skb_any(skb);
			return 0;
		}

		if (skb->dev == NULL) 
			return 1;
		
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21)
		eth=(struct ethhdr *)(skb->mac_header);
#else
		eth=(struct ethhdr *)(skb->mac.raw);
#endif

		if(eth->h_dest[0] & 1)
		{
			if(memcmp(eth->h_dest, skb->dev->broadcast, ETH_ALEN)==0){
				skb->pkt_type=PACKET_BROADCAST;
			} else {
				skb->pkt_type=PACKET_MULTICAST;
			}
		}else {

			if(memcmp(eth->h_dest, skb->dev->dev_addr, ETH_ALEN)==0){
				skb->pkt_type=PACKET_HOST;
			}else{
				skb->pkt_type=PACKET_OTHERHOST;
			}
		}

		return 1;
    }
#endif


    if( (FOE_AI(skb)==HIT_BIND_KEEPALIVE) && (DFL_FOE_KA_ORG==0)){

	  /* FIXME:	 
	   * Recover to original SMAC/DMAC, but we don't know that.
	   * just swap SMAC and DMAC to avoid "received packet with  own address as source address" error.
	   */
	    eth=(struct ethhdr *)(skb->data-ETH_HLEN);	 
					 
	    FoeGetMacInfo(eth->h_dest, foe_entry->smac_hi);	 
	    FoeGetMacInfo(eth->h_source, foe_entry->dmac_hi);
	    eth->h_source[0]=0x1;//change to multicast packet, make bridge not learn this packet
	    if(eth_type==ETH_P_8021Q) {
		    vlan1_gap = VLAN_HLEN;
		    vh = (struct vlan_hdr *) skb->data;

#ifdef TCSUPPORT_RA_HWNAT
		    if(ntohs(vh->h_vlan_TCI)==WAN_PORT_VLAN_ID){
			    /* It make packet like coming from LAN port */
			    vh->h_vlan_TCI=htons(LAN_PORT_VLAN_ID);
		    } else {
			    /* It make packet like coming from WAN port */
			    vh->h_vlan_TCI=htons(WAN_PORT_VLAN_ID);
		    }
#else
		    if(ntohs(vh->h_vlan_TCI)==LAN_PORT_VLAN_ID){
			    /* It make packet like coming from WAN port */
			    vh->h_vlan_TCI=htons(WAN_PORT_VLAN_ID);

		    } else {
			    /* It make packet like coming from LAN port */
			    vh->h_vlan_TCI=htons(LAN_PORT_VLAN_ID);
		    }
#endif

		    if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES) {
			    pppoe_gap = 8;
		    }else if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
                            vlan2_gap = VLAN_HLEN;
                            vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

                            /* VLAN + VLAN + PPPoE */
                            if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
                                pppoe_gap = 8;
                            }else {
                                /* VLAN + VLAN + IP */
                                eth_type = ntohs(vh->h_vlan_encapsulated_proto);
                            }
		    }else {
                            /* VLAN + IP */
                            eth_type = ntohs(vh->h_vlan_encapsulated_proto);
		    }
	    }

	    /* Only Ipv4 NAT need KeepAlive Packet to refresh iptable */
	    if(eth_type == ETH_P_IP) {
			iph = (struct iphdr *) (skb->data + vlan1_gap + vlan2_gap + pppoe_gap);

			//Recover to original layer 4 header 
			if (iph->protocol == IPPROTO_TCP) {
				th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
				FoeToOrgTcpHdr(foe_entry, iph, th);

			} else if (iph->protocol == IPPROTO_UDP) {
				uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
				FoeToOrgUdpHdr(foe_entry, iph, uh);
			}

			//Recover to original layer 3 header 
			FoeToOrgIpHdr(foe_entry,iph);
	    }else if(eth_type == ETH_P_IPV6) {
			/* Nothing to do */
	    }else {
			return 1;
	    }

	    /*
	     * Ethernet driver will call eth_type_trans() to set skb->pkt_type.
	     * If(destination mac != my mac) 
	     *   skb->pkt_type=PACKET_OTHERHOST;
	     * In order to pass ip_rcv() check, we change pkt_type=PACKET_HOST here
	     */
	    skb->pkt_type=PACKET_HOST;	
	    return 1;

    }

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
    if( (FOE_AI(skb)==HIT_UNBIND_RATE_REACH) ) 
    {
        AclClassifyKey NewRateReach;
		eth=(struct ethhdr *)(skb->data-ETH_HLEN);
		
		memset(&NewRateReach, 0, sizeof(AclClassifyKey));
		memcpy(NewRateReach.Mac, eth->h_source,ETH_ALEN);
        NewRateReach.Ethertype = eth_type; //Ethertype
		if(eth_type==ETH_P_8021Q)
		{
			vlan1_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *) skb->data;
			NewRateReach.Vid = ntohs(vh->h_vlan_TCI); //VID
			if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES) {
				pppoe_gap = 8;
			}else if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
				vlan2_gap = VLAN_HLEN;
				vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

				/* VLAN + VLAN + PPPoE */
				if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
					pppoe_gap = 8;
				}else {
					/* VLAN + VLAN + IP */
					eth_type = ntohs(vh->h_vlan_encapsulated_proto);
				}
			}else {
				/* VLAN + IP */
				eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			}
		}

		/*IPv4*/
		if(eth_type == ETH_P_IP)
		{
			iph = (struct iphdr *) (skb->data + vlan1_gap + vlan2_gap + pppoe_gap);

			NewRateReach.Sip = ntohl(iph->saddr);
			NewRateReach.Dip = ntohl(iph->daddr);
			NewRateReach.Tos = iph->tos; //TOS
			if (iph->protocol == IPPROTO_TCP) 
			{
				th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
				NewRateReach.Sp = ntohs(th->source);
				NewRateReach.Dp = ntohs(th->dest);
				NewRateReach.Proto = ACL_PROTO_TCP;
			}
			else if (iph->protocol == IPPROTO_UDP) 
			{
				uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
				NewRateReach.Sp = ntohs(uh->source);
				NewRateReach.Dp = ntohs(uh->dest);
				NewRateReach.Proto = ACL_PROTO_UDP;

			}

		}

		/*classify user priority*/
		FOE_SP(skb)= AclClassify(&NewRateReach);

		return 1;

    }
#endif

    return 1;
}

/* is_in = 1 --> in  */
/* is_in = 0 --> out */
int32_t GetPppoeSid(struct sk_buff *skb, uint32_t vlan_gap, 
		uint16_t *sid, uint16_t *ppp_tag, uint32_t is_in)
{
	struct pppoe_hdr *peh = NULL;
	uint32_t offset = 0;

	if(!is_in) {
		offset = ETH_HLEN;
	}

	peh = (struct pppoe_hdr *) (skb->data + offset + vlan_gap);

	if(DebugLevel==1) { 
		NAT_PRINT("\n==============\n");
		NAT_PRINT(" Ver=%d\n",peh->ver);
		NAT_PRINT(" Type=%d\n",peh->type);
		NAT_PRINT(" Code=%d\n",peh->code);
		NAT_PRINT(" sid=%x\n",ntohs(peh->sid));
		NAT_PRINT(" Len=%d\n",ntohs(peh->length));
		NAT_PRINT(" tag_type=%x\n",ntohs(peh->tag[0].tag_type));
		NAT_PRINT(" tag_len=%d\n",ntohs(peh->tag[0].tag_len));
		NAT_PRINT("=================\n");
	}

	*ppp_tag = ntohs(peh->tag[0].tag_type);
#if defined (CONFIG_RA_HW_NAT_IPV6)
	if (peh->ver != 1 || peh->type != 1 || (*ppp_tag != PPP_IP && *ppp_tag != PPP_IPV6) ) {
#else
	if (peh->ver != 1 || peh->type != 1 || *ppp_tag != PPP_IP ) {
#endif
		return 1;
	}

	*sid = peh->sid;
	return 0;
}

/*
  wan2lan packert, don't let it go hwnat path,
  added by pengyao 20130520
 */
#define SKBUF_COPYTOLAN (1 << 26)

int32_t PpeTxHandler(struct sk_buff *skb, int gmac_no)
{
	struct vlan_hdr *vh = NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *th = NULL;
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT63365) || defined (TCSUPPORT_RA_HWNAT)
	struct udphdr *uh = NULL;
#elif defined (CONFIG_RALINK_RT3052)
	struct udphdr *uh = NULL;
	uint32_t phy_val;
#endif
	struct ethhdr *eth = NULL;
	uint32_t vlan1_gap = 0;
	uint32_t vlan2_gap = 0;
	uint32_t pppoe_gap = 0;
	uint16_t pppoe_sid = 0;
	uint16_t ppp_tag = 0;
	struct FoeEntry *foe_entry;
	uint32_t current_time;
	struct FoeEntry entry;
	uint16_t eth_type=0;
	uint32_t offset=0;
#if defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
	uint32_t now=0;
#endif

    /*
      wan2lan packert, don't let it go hwnat path,
      added by pengyao 20130520
     */
    if(skb->mark & SKBUF_COPYTOLAN)
        return 1;

	/* 
	 * Packet is interested by ALG?
	 * Yes: Don't enter binind state
	 * No: If flow rate exceed binding threshold, enter binding state.
	 */
	if(IS_MAGIC_TAG_VALID(skb) && (FOE_AI(skb)==HIT_UNBIND_RATE_REACH) && (FOE_ALG(skb)==0)) 
	{
		eth = (struct ethhdr *) skb->data;
		eth_type=ntohs(eth->h_proto);
		foe_entry=&PpeFoeBase[FOE_ENTRY_NUM(skb)];

		char *ptr=NULL;
		ptr=(char*)&(foe_entry->dip);
	
#if defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
		// It's ready for becoming binding state in semi-auto 
		// bind mode, so there is no need to update any 
		// information within refresh interval.
#define SEMIAUTO_REFRESH_INTERVAL	30
		now = RegRead(FOE_TS_T)&0xFFFF;
		if(time_before((unsigned long)now, 
			    (unsigned long)foe_entry->tmp_buf.time_stamp 
			    + SEMIAUTO_REFRESH_INTERVAL)) {
		    goto err1;
		}
#endif
		//if this entry is already in binding state, skip it 
		if(foe_entry->bfib1.state == BIND) {
			goto err1;
		}

		if (!multicast_en) {
			if ((eth->h_dest[0] & 0x01) || (((*ptr)&0xf0) == 0xe0 )) {
				goto err1;
			}
		}

		/* Get original setting */
		memcpy(&entry, foe_entry, sizeof(entry));


		/* Set Layer2 Info - DMAC, SMAC */
		FoeSetMacInfo(entry.dmac_hi,eth->h_dest);
		FoeSetMacInfo(entry.smac_hi,eth->h_source);

		/* Set VLAN Info - VLAN1/VLAN2 */
		if ((eth_type==ETH_P_8021Q) || 
				(feSTag && (((eth_type&ETH_P_STAG_MASK)==ETH_P_STAG0) || 
				 			((eth_type&ETH_P_STAG_MASK)==ETH_P_STAG1)))) {
			vlan1_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *)(skb->data + ETH_HLEN);
			entry.vlan1 = ntohs(vh->h_vlan_TCI);

			/* VLAN + PPPoE */
			if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
				pppoe_gap = 8;
				if (GetPppoeSid(skb, vlan1_gap, &pppoe_sid, &ppp_tag, 0)) {
					goto err1;
				}
				entry.pppoe_id = ntohs(pppoe_sid);
				eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			/* Double VLAN = VLAN + VLAN */
			}else if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
			    vlan2_gap = VLAN_HLEN;
			    vh = (struct vlan_hdr *)(skb->data + ETH_HLEN + VLAN_HLEN);
			    entry.vlan2 = ntohs(vh->h_vlan_TCI);

			    /* VLAN + VLAN + PPPoE */
					if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
					pppoe_gap = 8;
					if (GetPppoeSid(skb, (vlan1_gap + vlan2_gap), &pppoe_sid, &ppp_tag, 0)) {
						goto err1;
					}
					entry.pppoe_id = ntohs(pppoe_sid);
					eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			    }else {
					/* VLAN + VLAN + IP */
					eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			    }
			}else {
			    /* VLAN + IP */
			    eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			}
		}

		/* 
		 * PPE support SMART VLAN/PPPoE Tag Push/PoP feature
		 *
		 *         | MODIFY | INSERT | DELETE
		 * --------+--------+--------+----------
		 * Tagged  | modify | modify | delete
		 * Untagged| no act | insert | no act
		 *
		 */

		if(vlan1_gap) {
			entry.bfib1.v1 = INSERT;
		} else {
			entry.bfib1.v1 = DELETE ;
		}

		if(vlan2_gap) {
			entry.bfib1.v2 = INSERT;
		} else {
			entry.bfib1.v2 = DELETE ;
		}

		if(pppoe_gap) { 
			entry.bfib1.pppoe = INSERT ;
		} else { 
			entry.bfib1.pppoe = DELETE ;
		}

		/* Set Layer3 Info */
		/* IPv4 or IPv4 over PPPoE */
		if( (eth_type == ETH_P_IP) || (eth_type == ETH_P_PPP_SES && ppp_tag == PPP_IP) ) {
		    iph = (struct iphdr *) (skb->data + ETH_HLEN + vlan1_gap + vlan2_gap + pppoe_gap);
		    entry.new_sip = ntohl(iph->saddr);
		    entry.new_dip = ntohl(iph->daddr);

#ifdef TCSUPPORT_RA_HWNAT
			/* RT63365 HWNAT has IP checksum error with DSCP value != 0 if PPE_DSCP=1 and RMDSCP=0. */
			//if((entry.vlan1 & VLAN_VID_MASK)==WAN_PORT_VLAN_ID) {
				entry.iblk2.rmdscp=1;
				entry.iblk2.dscp=iph->tos;
			//}
#endif
		    /* Set Layer4 Info - NEW_SPORT, NEW_DPORT */
		    if (iph->protocol == IPPROTO_TCP) {
				th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
				entry.new_sport = ntohs(th->source);
				entry.new_dport = ntohs(th->dest);
				entry.bfib1.t_u = TCP;
			} else if (iph->protocol == IPPROTO_UDP) {
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT63365) || defined (TCSUPPORT_RA_HWNAT)
				uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
				entry.new_sport = ntohs(uh->source);
				entry.new_dport = ntohs(uh->dest);
				entry.bfib1.t_u = UDP;
#elif defined (CONFIG_RALINK_RT3052)
				rw_rf_reg(0, 0, &phy_val);
				phy_val = phy_val & 0xFF;

				if(phy_val > 0x53) {
					uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
					entry.new_sport = ntohs(uh->source);
					entry.new_dport = ntohs(uh->dest);
					entry.bfib1.t_u = UDP;
				} else {
					goto err1;
				}
#else
				/* if udp check is zero, it cannot be accelerated by HNAT */
				/* we found the application is possible to use udp checksum=0 at first stage, 
				 * then use non-zero checksum in the same session later, so we disable HNAT acceleration
				 * for all UDP traffic */
				goto err1;
#endif
			}else {
				/* we support IPv4 NAT mode */
				goto err1;
			}
#if defined (CONFIG_RA_HW_NAT_IPV6)
		/* IPv6 or IPv6 over PPPoE */
		} else if (eth_type == ETH_P_IPV6 || (eth_type == ETH_P_PPP_SES && ppp_tag == PPP_IPV6) ) {
		    /* Nothing to do */
		    ;
#endif
		} else {
		   goto err1;
		}

		/* Set Current time to time_stamp field in information block 1 */
		current_time =RegRead(FOE_TS_T)&0xFFFF;
		entry.bfib1.time_stamp=(uint16_t)current_time;
              
#if defined(CONFIG_RA_HW_NAT_ACL2UP_HELPER) || defined(TCSUPPORT_RA_HWNAT)
		/*set user priority*/
		entry.iblk2.up = FOE_SP(skb);
		entry.iblk2.fp = 1;
#endif
		/* Set Information block 2 */
		entry.iblk2.fd=1;
		/* CPU need to handle traffic between WLAN/PCI and GMAC port */	
#ifdef TCSUPPORT_RA_HWNAT
		if ((strncmp(skb->dev->name, "ra0", 3) == 0) ||
			(strncmp(skb->dev->name, "ra1", 3) == 0) ||
#if defined(CONFIG_RA_HW_NAT_ATM)
			((strncmp(skb->dev->name,"nas", 3) == 0) && (strlen(skb->dev->name) == 4) &&
			 	((skb->dev->name[3] >= '0') && (skb->dev->name[3] <= '7')))) {
#endif
#else
		if( (strncmp(skb->dev->name,"ra",2)==0) ||
		    (strncmp(skb->dev->name,"wds",3)==0) ||
		    (strncmp(skb->dev->name,"mesh",4)==0) ||
		    (strncmp(skb->dev->name,"apcli",5)==0) ||
#if defined(CONFIG_RA_HW_NAT_ATM)
		    ((strncmp(skb->dev->name,"nas",3)==0) && (strlen(skb->dev->name) == 4) &&
			 	((skb->dev->name[3] >= '0') && (skb->dev->name[3] <= '7'))) ||
#endif
		    (skb->dev == DstPort[DP_PCI])) {
#endif
#if defined(CONFIG_RA_HW_NAT_WIFI) || defined(CONFIG_RA_HW_NAT_ATM)
			entry.iblk2.dp=0; /* cpu */
#else
			goto err1;
#endif // CONFIG_RA_HW_NAT_WIFI //

		}else {
/* RT3883 with 2xGMAC - Assuming GMAC2=WAN  and GMAC1=LAN */
#if defined (CONFIG_RAETH_GMAC2) 
			if(gmac_no==1) {
			    if((bind_dir == DOWNSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					entry.iblk2.dp=1; 
			    }else {
					goto err1;
			    }
			}else if(gmac_no==2) {
			    if((bind_dir == UPSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					entry.iblk2.dp=2;
			    }else {
					goto err1;
			    }
			}

/* RT2880, RT3883 */
#elif defined (CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT3883)
			if((entry.vlan1 & VLAN_VID_MASK)==LAN_PORT_VLAN_ID) {
			    if((bind_dir == DOWNSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					entry.iblk2.dp=1; 
			    }else {
					goto err1;	
			    }
			}else if((entry.vlan1 & VLAN_VID_MASK)==WAN_PORT_VLAN_ID) {
			    if((bind_dir == UPSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					entry.iblk2.dp=1; 
			    }else {
					goto err1;
			    }
			}
/*  RT3052, RT335x */
#else
#ifdef TCSUPPORT_RA_HWNAT
			if((entry.vlan1 & VLAN_VID_MASK)==WAN_PORT_VLAN_ID) {
			    if((bind_dir == UPSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					entry.iblk2.dp=2; /* WAN traffic use VirtualPort2 in GMAC2*/
			    }else {
					goto err1;
			    }
			}else {
			    if((bind_dir == DOWNSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					entry.iblk2.dp=1; /* LAN traffic use VirtualPort1 in GMAC1*/
			    }else {
					goto err1;
			    }
			}
#else
			if((entry.vlan1 & VLAN_VID_MASK)==LAN_PORT_VLAN_ID) {
			    if((bind_dir == DOWNSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					entry.iblk2.dp=1; /* LAN traffic use VirtualPort1 in GMAC1*/
			    }else {
					goto err1;
			    }
			}else if((entry.vlan1 & VLAN_VID_MASK)==WAN_PORT_VLAN_ID) {
			    if((bind_dir == UPSTREAM_ONLY) || (bind_dir == BIDIRECTION)) {
					entry.iblk2.dp=2; /* WAN traffic use VirtualPort2 in GMAC1*/
			    }else {
					goto err1;
			    }
			}else {
			    /* for one arm NAT test -> no vlan tag */
			    entry.iblk2.dp=1; 
			}
#endif
#endif
		}

		if(IS_FORCE_ACL_TO_UP(skb))
		{
			entry.iblk2.up=(GET_ACL_TO_UP(skb)); /* new user priority */ 
			entry.iblk2.fp=1; /* enable force user priority */ 
		}



		/* This is ugly soultion to support WiFi pseudo interface.
		 * Please double check the definition is the same as include/rt_linux.h 
		 */
#define CB_OFF  10
#define RTMP_GET_PACKET_IF(skb)                 skb->cb[CB_OFF+6]
#define MIN_NET_DEVICE_FOR_MBSSID               0x00
#define MIN_NET_DEVICE_FOR_WDS                  0x10
#define MIN_NET_DEVICE_FOR_APCLI                0x20
#define MIN_NET_DEVICE_FOR_MESH                 0x30

		/* Set actual output port info */
#if defined (CONFIG_RT3090_AP) || defined (CONFIG_RT3090_AP_MODULE)
		if(strncmp(skb->dev->name, "rai", 3)==0) {
#if defined (CONFIG_RT3090_AP_MESH)
		    if(RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH) {
				offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESHI0);
		    }else 
#endif // CONFIG_RT3090_AP_MESH //

#if defined (CONFIG_RT3090_AP_APCLI)
		    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI) {
				offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLII0);
		    }else 
#endif // CONFIG_RT3090_AP_APCLI //
#if defined (CONFIG_RT3090_AP_WDS)
	   	    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS) {
				offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDSI0);
		    }else 
#endif // CONFIG_RT3090_AP_WDS //
		    {
				offset = RTMP_GET_PACKET_IF(skb) + DP_RAI0;
		    }
		}else 
#endif // CONFIG_RT3090_AP || CONFIG_RT3090_AP_MODULE // 

#ifdef TCSUPPORT_RA_HWNAT
		if(strncmp(skb->dev->name, "ra0", 3)==0) {
			offset = DP_RA0;
		}else if(strncmp(skb->dev->name, "ra1", 3)==0) {
			offset = DP_RA1;
#else
		if(strncmp(skb->dev->name, "ra", 2)==0) {
#if defined (CONFIG_RT2860V2_AP_MESH)
		    if(RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH) {
				offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESH0);
		    }else 
#endif // CONFIG_RT2860V2_AP_MESH //
#if defined (CONFIG_RT2860V2_AP_APCLI)
		    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI) {
				offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLI0);
		    }else 
#endif  // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_WDS)
		    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS) {
				offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDS0);
		    }else 
#endif // CONFIG_RT2860V2_AP_WDS //
		    {
				offset = RTMP_GET_PACKET_IF(skb) + DP_RA0;
		    }
#endif
#ifdef TCSUPPORT_RA_HWNAT
		}else if((strncmp(skb->dev->name, "eth0", 4)==0) || 
					strncmp(skb->dev->name, "nas10", 5)==0) {
#else
		}else if(strncmp(skb->dev->name, "eth2", 4)==0) {
#endif
			offset = DP_GMAC; //for debugging purpose
#ifdef CONFIG_RAETH_GMAC2
		}else if(strncmp(skb->dev->name, "eth3", 4)==0) {
			offset = DP_GMAC2; //for debugging purpose
#endif
#ifndef TCSUPPORT_RA_HWNAT
		}else if(strncmp(skb->dev->name, "eth0", 4)==0) {
			offset = DP_PCI; //for debugging purpose
#endif
		}else {
			if(DebugLevel==1) 
		    	printk("HNAT: unknow interface %s\n",skb->dev->name);
			goto err1;
		}
		
#ifdef TCSUPPORT_RA_HWNAT
		entry.act_dp = offset; 
#else
		entry.iblk2.act_dp = offset; 
#endif

		/* Ipv4: TTL / Ipv6: Hot Limit filed */
		entry.bfib1.ttl = DFL_FOE_TTL_REGEN;

		/* Change Foe Entry State to Binding State*/
#if defined (CONFIG_RA_HW_NAT_AUTO_BIND)
		entry.bfib1.state = BIND;
#elif defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
		/* Don't bind this flow until user wants to bind it. */
		memcpy(&entry.tmp_buf, &entry.bfib1 , sizeof(entry.bfib1));
#endif
		memcpy(foe_entry, &entry, sizeof(entry));
		if(DebugLevel==7) {
		    FoeDumpEntry(FOE_ENTRY_NUM(skb));	
		}

	}else if((FOE_AI(skb)==HIT_BIND_KEEPALIVE) && (DFL_FOE_KA_ORG==0)){
		/* this is duplicate packet in keepalive new header mode, 
		 * just drop it */
		memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
		return 0;
	}else if( (FOE_AI(skb)==HIT_UNBIND_RATE_REACH)&& FOE_ALG(skb)==1) {
		if(DebugLevel==1) {
		    NAT_PRINT("%s: I cannot bind it becuase of FOE_ALG=1\n",__FUNCTION__);
		}
	}
	goto err0;
err1:
	memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
err0:
	return 1;
}
#endif

#ifdef TCSUPPORT_RA_HWNAT
int PpeFreeHandler(struct sk_buff * skb)
{
	if (IS_SPACE_AVAILABLED(skb)  &&
			((FOE_MAGIC_TAG(skb) == FOE_MAGIC_PCI) ||
			 (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN) ||
			 (FOE_MAGIC_TAG(skb) == FOE_MAGIC_GE) ||
			 (FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE) ||
			 (FOE_MAGIC_TAG(skb) == FOE_MAGIC_ATM))) {
			//FOE_ALG(skb)=1;
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
	}

	return 0;
}

int PpeRxinfoHandler(struct sk_buff * skb, int magic, char *data, int data_length)
{
	FOE_MAGIC_TAG(skb) = magic;
	if (magic == FOE_MAGIC_GE)
		memcpy(FOE_INFO_START_ADDR(skb)+2, data, data_length);

	return 0;
}

int PpeTxqHandler(struct sk_buff * skb, int txq)
{
	if (IS_MAGIC_TAG_VALID(skb))
		FOE_SP(skb) = txq;

	return 0;
}

int PpeMagicHandler(struct sk_buff * skb, int magic)
{
	if (FOE_MAGIC_TAG(skb) == magic) 
		return 1;
	else
		return 0;
}

int PpeSetMagicHandler(struct sk_buff * skb, int magic)
{
	FOE_MAGIC_TAG(skb) = magic;
	return 0;
}

int PpeXferHandler(struct sk_buff *new, struct sk_buff *old)
{
	if (IS_SPACE_AVAILABLED(new) && IS_SPACE_AVAILABLED(old) &&
			((FOE_MAGIC_TAG(old) == FOE_MAGIC_PCI) ||
			 (FOE_MAGIC_TAG(old) == FOE_MAGIC_WLAN) ||
			 (FOE_MAGIC_TAG(old) == FOE_MAGIC_GE) ||
			 (FOE_MAGIC_TAG(old) == FOE_MAGIC_PPE) ||
			 (FOE_MAGIC_TAG(old) == FOE_MAGIC_ATM))) {
		memcpy(FOE_INFO_START_ADDR(new), FOE_INFO_START_ADDR(old), FOE_INFO_LEN);
		memset(FOE_INFO_START_ADDR(old), 0, FOE_INFO_LEN);
	} else {
		if (IS_SPACE_AVAILABLED(new))
			memset(FOE_INFO_START_ADDR(new), 0, FOE_INFO_LEN);
	}

	return 0;
}
#endif

void  PpeSetFoeEbl(uint32_t FoeEbl)
{
	uint32_t PpeFlowSet=0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	/* FOE engine need to handle unicast/multicast/broadcast flow */
	if(FoeEbl==1) {
		PpeFlowSet = (BIT_FUC_FOE | BIT_FMC_FOE | BIT_FBC_FOE);
		PpeFlowSet|= (BIT_IPV4_NAPT_EN /*| BIT_IPV4_NAT_EN*/);

#if defined(CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet |= (BIT_IPV6_FOE_EN);
#endif
	} else {
		PpeFlowSet &= ~(BIT_FUC_FOE | BIT_FMC_FOE | BIT_FBC_FOE);
		PpeFlowSet &= ~(BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);
#if defined(CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet &= ~(BIT_IPV6_FOE_EN);
#endif
	}

	RegWrite( PPE_FLOW_SET, PpeFlowSet);
}


static void PpeSetFoeHashMode(uint32_t HashMode)
{

	/* Allocate FOE table base */ 
	FoeAllocTbl(FOE_4TB_SIZ);

	switch(FOE_4TB_SIZ){
	case 1024:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_1K, 0, 3);
		break;
	case 2048:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_2K, 0, 3);
		break;
	case 4096:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_4K, 0, 3);
		break;
	case 8192:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_8K, 0, 3);
		break;
	case 16384:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_16K, 0, 3);
		break;
	}

#ifndef TCSUPPORT_RA_HWNAT
	/*
	 * RT2880-Shuttle/RT2880_MP Bug
	 *
	 * HashMode=0/1 in 1K table size -> set HashMode =0/1
	 * HashMode=0/1 in 2K,4K,8K,16K table size -> set HashMode =1/0
	 *
	 */
	if(ChipId==RT2880 && ChipVer < RT2880_MP2) {
		if(FOE_4TB_SIZ!=1024){
			HashMode=~HashMode;
		}
	}
#endif

	/* Set Hash Mode */
	RegModifyBits(PPE_FOE_CFG, HashMode , 3, 1);

	/* Set action for FOE search miss */
#if defined (CONFIG_RA_HW_NAT_AUTO_BIND) || defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
	RegModifyBits(PPE_FOE_CFG, FWD_CPU_BUILD_ENTRY, 4, 2);
#elif defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
	RegModifyBits(PPE_FOE_CFG, ONLY_FWD_CPU, 4, 2);
#else
	#error "Please Choice Action for FoE search miss"
#endif
}

static void PpeSetAgeOut(void)
{
	/* set Unbind State Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_UNB_AGE, 8, 1);

	/* set min threshold of packet count for aging out at unbind state */
	RegModifyBits(PPE_FOE_UNB_AGE, DFL_FOE_UNB_MNP, 16, 16);

	/* set Delta time for aging out an unbind FOE entry */
	RegModifyBits(PPE_FOE_UNB_AGE, DFL_FOE_UNB_DLTA, 0, 8);

#if defined (CONFIG_RA_HW_NAT_AUTO_BIND) || defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
	/* set Bind TCP Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_TCP_AGE, 9, 1);

	/* set Bind UDP Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_UDP_AGE, 10, 1);

	/* set Bind TCP FIN Age Enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_FIN_AGE, 11, 1);

	/* Delta time for aging out an ACL link to FOE entry */
	//RegModifyBits(PPE_FOE_UNB_AGE, DFL_FOE_ACL_DLTA, 8, 8);

	/* set Delta time for aging out an bind UDP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE1, DFL_FOE_UDP_DLTA, 0, 16);

	/* set Delta time for aging out an bind TCP FIN FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, DFL_FOE_FIN_DLTA, 16, 16);

	/* set Delta time for aging out an bind TCP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, DFL_FOE_TCP_DLTA, 0, 16);
#elif defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
	/* fix TCP last ACK issue */
	/* Only need to enable Bind TCP FIN aging out function */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_FIN_AGE, 11, 1);

	/* set Delta time for aging out an bind TCP FIN FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, DFL_FOE_FIN_DLTA, 16, 16);
#endif
}

static void PpeSetFoeKa(void)
{
	/* set Keep alive packet with new/org header */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_KA_ORG, 12, 1);

	/* set Keep alive enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_KA_EN, 13, 1);

	/* ACL link to FOE age enable */
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_ACL_AGE, 14, 1);
	
	/* Keep alive timer value */
	RegModifyBits(PPE_FOE_KA, DFL_FOE_KA_T, 0, 16);

	/* Keep alive time for bind FOE TCP entry */
	RegModifyBits(PPE_FOE_KA, DFL_FOE_TCP_KA, 16, 8);

	/* Keep alive timer for bind FOE UDP entry */
	RegModifyBits(PPE_FOE_KA, DFL_FOE_UDP_KA, 24, 8);

}

static void PpeSetFoeBindRate(uint32_t FoeBindRate)
{
	/* Allowed max entries to be build during a time stamp unit */

	/* smaller than 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, DFL_FOE_QURT_LMT, 0, 14);

	/* between 1/2 and 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, DFL_FOE_HALF_LMT, 16, 14);

	/* between full and 1/2 of total entries */
	RegModifyBits(PPE_FOE_LMT2, DFL_FOE_FULL_LMT, 0, 14);

	/* Set reach bind rate for unbind state */
	RegWrite(PPE_FOE_BNDR, FoeBindRate);
}


static void PpeSetFoeGloCfgEbl(uint32_t Ebl)
{
    uint32_t gdma1_fwd_cfg;

	if(Ebl==1) {
		/* PPE Engine Enable */ 
		RegModifyBits(PPE_GLO_CFG, 1, 0, 1);

#ifdef __BIG_ENDIAN
		/* PPE Packet with BYTE_SWAP=1 */ 
		RegModifyBits(PPE_GLO_CFG, DFL_BYTE_SWAP, 5, 1);
#endif

		/* PPE Packet with TTL=0 */ 
		RegModifyBits(PPE_GLO_CFG, DFL_TTL0_DRP, 4, 1);

		/* Use VLAN priority tag as priority decision */
		RegModifyBits(PPE_GLO_CFG, DFL_VPRI_EN, 8, 1);

		/* Use DSCP as priority decision */
		RegModifyBits(PPE_GLO_CFG, DFL_DPRI_EN, 9, 1);

		/* Re-generate VLAN priority tag */
		RegModifyBits(PPE_GLO_CFG, DFL_REG_VPRI, 10, 1);

		/* Re-generate DSCP */
		RegModifyBits(PPE_GLO_CFG, DFL_REG_DSCP, 11, 1);
		
		/* Random early drop mode */
		RegModifyBits(PPE_GLO_CFG, DFL_RED_MODE, 12, 2);

		/* Enable use ACL force priority for hit unbind 
		 * and rate reach packet in CPU reason */
		RegModifyBits(PPE_GLO_CFG, DFL_ACL_PRI_EN, 14, 1);

#ifdef TCSUPPORT_RA_HWNAT
		/* PPE DSCP remarking */
		RegModifyBits(PPE_GLO_CFG, DFL_PPE_DSCP, 23, 1);
#endif

#if defined (CONFIG_RALINK_RT3052)
		/* Disable switch port 6 flow control to fix rt3052 tx/rx FC wired issue*/
		RegModifyBits(RALINK_ETH_SW_BASE+0xC8, 0x0, 8, 2);
		
		/* Set switch scheduler to SPQ */
		RegModifyBits(RALINK_ETH_SW_BASE+0x10, 0x0, 0, 16);
#elif defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT63365) || defined (TCSUPPORT_RA_HWNAT)
#if 0
		/* Set switch port 6 flow control only tx on for QOS support*/
		RegModifyBits(RALINK_ETH_SW_BASE+0xC8, 0x2, 8, 2);
#endif

#if defined (CONFIG_RAETH_SPECIAL_TAG)
		/* Set GDMA1 GDM1_TCI_81xx */
		RegModifyBits(FE_GDMA1_FWD_CFG, 0x1, 24, 1);
		/* Set GDMA2 GDM2_TCI_81xx */
		RegModifyBits(FE_GDMA2_FWD_CFG, 0x1, 24, 1);
		/* Set EXT_SW_EN = 1 */
		RegModifyBits(FE_COS_MAP, 0x1, 30, 1);
#endif

#endif

		/* check if special tag is on or off */
		gdma1_fwd_cfg = RegRead(FE_GDMA1_FWD_CFG);
		if (gdma1_fwd_cfg & GDM1_TCI_81xx)
			feSTag = 1;
		else
			feSTag = 0;

		if (feSTag) {
			/* Set EXT_SW_EN = 1 */
			RegModifyBits(FE_COS_MAP, 0x1, 30, 1);
		}

	} else {
		/* PPE Engine Disable */ 
		RegModifyBits(PPE_GLO_CFG, 0, 0, 1);

#if defined (CONFIG_RAETH_SPECIAL_TAG)
		/* Remove GDMA1 GDM1_TCI_81xx */
		RegModifyBits(FE_GDMA1_FWD_CFG, 0x0, 24, 1);
		/* Remove GDMA2 GDM2_TCI_81xx */
		RegModifyBits(FE_GDMA2_FWD_CFG, 0x0, 24, 1);
		/* Remove EXT_SW_EN = 1 */
		RegModifyBits(FE_COS_MAP, 0x0, 30, 1);
#endif	
	}

}

#ifndef CONFIG_RALINK_RT3052_MP
/*
 * - VLAN->UP: Incoming VLAN Priority to User Priority (Fixed)
 * - DSCP->UP: Incoming DSCP to User Priority
 * - UP->xxx : User Priority to VLAN/InDSCP/OutDSCP/AC Priority Mapping
 *
 * VLAN | DSCP |  UP |VLAN Pri|In-DSCP |Out-DSCP| AC | WMM_AC
 * -----+------+-----+--------+--------+--------+----+-------
 *   0	| 00-07|  0  |   0    |  0x00  |  0x00	|  0 |  BE
 *   3	| 24-31|  3  |   3    |  0x18  |  0x10	|  0 |  BE
 *   1	| 08-15|  1  |   1    |  0x08  |  0x00	|  0 |  BG
 *   2  | 16-23|  2  |   2    |  0x10  |  0x08	|  0 |  BG
 *   4	| 32-39|  4  |   4    |  0x20  |  0x18	|  1 |  VI
 *   5  | 40-47|  5  |   5    |  0x28  |  0x20	|  1 |  VI
 *   6  | 48-55|  6  |   6    |  0x30  |  0x28	|  2 |  VO
 *   7	| 56-63|  7  |   7    |  0x38  |  0x30	|  2 |  VO
 * -----+------+-----+--------+--------+--------+----+--------
 *
 */
static void  PpeSetUserPriority(void)
{
    /* Set weight of decision in resolution */
    RegWrite(UP_RES, DFL_UP_RES);
   
    /* Set DSCP to User priority mapping table */ 
    RegWrite(DSCP0_7_MAP_UP, DFL_DSCP0_7_UP);
    RegWrite(DSCP24_31_MAP_UP, DFL_DSCP24_31_UP);
    RegWrite(DSCP8_15_MAP_UP, DFL_DSCP8_15_UP);
    RegWrite(DSCP16_23_MAP_UP, DFL_DSCP16_23_UP);
    RegWrite(DSCP32_39_MAP_UP, DFL_DSCP32_39_UP);
    RegWrite(DSCP40_47_MAP_UP, DFL_DSCP40_47_UP);
    RegWrite(DSCP48_55_MAP_UP, DFL_DSCP48_55_UP);
    RegWrite(DSCP56_63_MAP_UP, DFL_DSCP56_63_UP);
   
#if 0 
    /* Set boundary and range of auto user priority */ 
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_BND1, 16, 14);
    RegModifyBits(AUTO_UP_CFG2, DFL_ATUP_BND2, 0, 14);
    RegModifyBits(AUTO_UP_CFG2, DFL_ATUP_BND3, 16, 14);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R1_UP, 0, 3);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R2_UP, 4, 3);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R3_UP, 8, 3);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R4_UP, 12, 3);
#endif

    /* Set mapping table of user priority to vlan priority */
    RegModifyBits(UP_MAP_VPRI, DFL_UP0_VPRI, 0, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP1_VPRI, 4, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP2_VPRI, 8, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP3_VPRI, 12, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP4_VPRI, 16, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP5_VPRI, 20, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP6_VPRI, 24, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP7_VPRI, 28, 3);
   
    /* Set mapping table of user priority to in-profile DSCP */
    RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP0_IDSCP, 0, 6);
    RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP1_IDSCP, 8, 6);
    RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP2_IDSCP, 16, 6);
    RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP3_IDSCP, 24, 6);
    RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP4_IDSCP, 0, 6);
    RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP5_IDSCP, 8, 6);
    RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP6_IDSCP, 16, 6);
    RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP7_IDSCP, 24, 6);
     
    /* Set mapping table of user priority to out-profile DSCP */
    RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP0_ODSCP, 0, 6);
    RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP1_ODSCP, 8, 6);
    RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP2_ODSCP, 16, 6);
    RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP3_ODSCP, 24, 6);
    RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP4_ODSCP, 0, 6);
    RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP5_ODSCP, 8, 6);
    RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP6_ODSCP, 16, 6);
    RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP7_ODSCP, 24, 6);

    /* Set mapping table of user priority to access category */
    RegModifyBits(UP_MAP_AC, DFL_UP0_AC, 0, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP1_AC, 2, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP2_AC, 4, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP3_AC, 6, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP4_AC, 8, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP5_AC, 10, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP6_AC, 12, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP7_AC, 14, 2);
}
#endif

static void FoeFreeTbl(uint32_t NumOfEntry)
{
	uint32_t FoeTblSize;

	FoeTblSize = NumOfEntry * sizeof(struct FoeEntry);
	dma_free_coherent(NULL, FoeTblSize, PpeFoeBase, PpePhyFoeBase);
	RegWrite( PPE_FOE_BASE, 0);
}

static int32_t PpeEngStart(void)
{
	/* Set PPE Flow Set */
	PpeSetFoeEbl(1);

	/* Set PPE FOE Hash Mode */
	PpeSetFoeHashMode(DFL_FOE_HASH_MODE);

	/* Set default index in policy table */
	PpeSetPreAclEbl(0);
	PpeSetPreMtrEbl(0);
	PpeSetPostMtrEbl(0);
	PpeSetPreAcEbl(0);
	PpeSetPostAcEbl(0);

	/* Set Auto Age-Out Function */
	PpeSetAgeOut();

	/* Set PPE FOE KEEPALIVE TIMER */
	PpeSetFoeKa(); 

	/* Set PPE FOE Bind Rate */
	PpeSetFoeBindRate(DFL_FOE_BNDR); 

	/* Set PPE Global Configuration */
	PpeSetFoeGloCfgEbl(1);

#ifndef CONFIG_RALINK_RT3052_MP
	/* Set User Priority related register */
	PpeSetUserPriority();
#endif
	return 0;
}

static int32_t PpeEngStop(void)
{
	/* Set PPE FOE ENABLE */
	PpeSetFoeGloCfgEbl(0);

	/* Set PPE Flow Set */
	PpeSetFoeEbl(0);
	
	/* Set default index in policy table */
	PpeSetPreAclEbl(0);
	PpeSetPreMtrEbl(0);
	PpeSetPostMtrEbl(0);
	PpeSetPreAcEbl(0);
	PpeSetPostAcEbl(0);

	/* Free FOE table */ 
	FoeFreeTbl(FOE_4TB_SIZ);

	return 0;
}

struct net_device *ra_dev_get_by_name(const char *name)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    return dev_get_by_name(&init_net, name);
#else
    return dev_get_by_name(name);
#endif
}

static void PpeSetDstPort(uint32_t Ebl)
{
    if(Ebl) {
	DstPort[DP_RA0]=ra_dev_get_by_name("ra0"); 
#ifdef TCSUPPORT_RA_HWNAT
	DstPort[DP_RA1]=ra_dev_get_by_name("ra1"); 
#endif
#if defined (CONFIG_RT2860V2_AP_MBSS)
	DstPort[DP_RA1]=ra_dev_get_by_name("ra1"); 
	DstPort[DP_RA2]=ra_dev_get_by_name("ra2"); 
	DstPort[DP_RA3]=ra_dev_get_by_name("ra3"); 
	DstPort[DP_RA4]=ra_dev_get_by_name("ra4"); 
	DstPort[DP_RA5]=ra_dev_get_by_name("ra5"); 
	DstPort[DP_RA6]=ra_dev_get_by_name("ra6"); 
	DstPort[DP_RA7]=ra_dev_get_by_name("ra7"); 
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT63365)
	DstPort[DP_RA8]=ra_dev_get_by_name("ra8"); 
	DstPort[DP_RA9]=ra_dev_get_by_name("ra9"); 
	DstPort[DP_RA10]=ra_dev_get_by_name("ra10"); 
	DstPort[DP_RA11]=ra_dev_get_by_name("ra11"); 
	DstPort[DP_RA12]=ra_dev_get_by_name("ra12"); 
	DstPort[DP_RA13]=ra_dev_get_by_name("ra13"); 
	DstPort[DP_RA14]=ra_dev_get_by_name("ra14"); 
	DstPort[DP_RA15]=ra_dev_get_by_name("ra15"); 
#endif
#endif
#if defined (CONFIG_RT2860V2_AP_WDS)
	DstPort[DP_WDS0]=ra_dev_get_by_name("wds0");
	DstPort[DP_WDS1]=ra_dev_get_by_name("wds1");
	DstPort[DP_WDS2]=ra_dev_get_by_name("wds2");
	DstPort[DP_WDS3]=ra_dev_get_by_name("wds3");
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
	DstPort[DP_APCLI0]=ra_dev_get_by_name("apcli0");
#endif
#if defined (CONFIG_RT2860V2_AP_MESH)
	DstPort[DP_MESH0]=ra_dev_get_by_name("mesh0");
#endif
#if defined (CONFIG_RT3090_AP) || defined (CONFIG_RT3090_AP_MODULE)
	DstPort[DP_RAI0]=ra_dev_get_by_name("rai0"); 
#if defined (CONFIG_RT3090_AP_MBSS)
	DstPort[DP_RAI1]=ra_dev_get_by_name("rai1"); 
	DstPort[DP_RAI2]=ra_dev_get_by_name("rai2"); 
	DstPort[DP_RAI3]=ra_dev_get_by_name("rai3"); 
	DstPort[DP_RAI4]=ra_dev_get_by_name("rai4"); 
	DstPort[DP_RAI5]=ra_dev_get_by_name("rai5"); 
	DstPort[DP_RAI6]=ra_dev_get_by_name("rai6"); 
	DstPort[DP_RAI7]=ra_dev_get_by_name("rai7"); 
	DstPort[DP_RAI8]=ra_dev_get_by_name("rai8"); 
	DstPort[DP_RAI9]=ra_dev_get_by_name("rai9"); 
	DstPort[DP_RAI10]=ra_dev_get_by_name("rai10"); 
	DstPort[DP_RAI11]=ra_dev_get_by_name("rai11"); 
	DstPort[DP_RAI12]=ra_dev_get_by_name("rai12"); 
	DstPort[DP_RAI13]=ra_dev_get_by_name("rai13"); 
	DstPort[DP_RAI14]=ra_dev_get_by_name("rai14"); 
	DstPort[DP_RAI15]=ra_dev_get_by_name("rai15"); 
#endif // CONFIG_RT3090_AP_MBSS //
#endif
#if defined (CONFIG_RT3090_AP_APCLI)
	DstPort[DP_APCLII0]=ra_dev_get_by_name("apclii0");
#endif // CONFIG_RT3090_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH)
	DstPort[DP_MESHI0]=ra_dev_get_by_name("meshi0");
#endif // CONFIG_RT3090_AP_MESH //
#ifdef TCSUPPORT_RA_HWNAT
	DstPort[DP_GMAC]=ra_dev_get_by_name("eth0");
#else
	DstPort[DP_GMAC]=ra_dev_get_by_name("eth2");
#endif
#ifdef CONFIG_RAETH_GMAC2
	DstPort[DP_GMAC2]=ra_dev_get_by_name("eth3");
#endif
#ifndef TCSUPPORT_RA_HWNAT
	DstPort[DP_PCI]=ra_dev_get_by_name("eth2"); // PCI interface name
#endif
#if defined(CONFIG_RA_HW_NAT_ATM)
	DstPort[DP_NAS0]=ra_dev_get_by_name("nas0"); 
	DstPort[DP_NAS1]=ra_dev_get_by_name("nas1"); 
	DstPort[DP_NAS2]=ra_dev_get_by_name("nas2"); 
	DstPort[DP_NAS3]=ra_dev_get_by_name("nas3"); 
	DstPort[DP_NAS4]=ra_dev_get_by_name("nas4"); 
	DstPort[DP_NAS5]=ra_dev_get_by_name("nas5"); 
	DstPort[DP_NAS6]=ra_dev_get_by_name("nas6"); 
	DstPort[DP_NAS7]=ra_dev_get_by_name("nas7"); 
#endif
    }else {
	if(DstPort[DP_RA0]!=NULL) { dev_put(DstPort[DP_RA0]); }
#ifdef TCSUPPORT_RA_HWNAT
	if(DstPort[DP_RA1]!=NULL) { dev_put(DstPort[DP_RA1]); }
#endif
#if defined (CONFIG_RT2860V2_AP_MBSS)
	if(DstPort[DP_RA1]!=NULL) { dev_put(DstPort[DP_RA1]); }
	if(DstPort[DP_RA2]!=NULL) { dev_put(DstPort[DP_RA2]); }
	if(DstPort[DP_RA3]!=NULL) { dev_put(DstPort[DP_RA3]); }
	if(DstPort[DP_RA4]!=NULL) { dev_put(DstPort[DP_RA4]); }
	if(DstPort[DP_RA5]!=NULL) { dev_put(DstPort[DP_RA5]); }
	if(DstPort[DP_RA6]!=NULL) { dev_put(DstPort[DP_RA6]); }
	if(DstPort[DP_RA7]!=NULL) { dev_put(DstPort[DP_RA7]); }
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT63365)
	if(DstPort[DP_RA8]!=NULL) { dev_put(DstPort[DP_RA8]); }
	if(DstPort[DP_RA9]!=NULL) { dev_put(DstPort[DP_RA9]); }
	if(DstPort[DP_RA10]!=NULL) { dev_put(DstPort[DP_RA10]); }
	if(DstPort[DP_RA11]!=NULL) { dev_put(DstPort[DP_RA11]); }
	if(DstPort[DP_RA12]!=NULL) { dev_put(DstPort[DP_RA12]); }
	if(DstPort[DP_RA13]!=NULL) { dev_put(DstPort[DP_RA13]); }
	if(DstPort[DP_RA14]!=NULL) { dev_put(DstPort[DP_RA14]); }
	if(DstPort[DP_RA15]!=NULL) { dev_put(DstPort[DP_RA15]); }
#endif
#endif
#if defined (CONFIG_RT2860V2_AP_WDS)
	if(DstPort[DP_WDS0]!=NULL) { dev_put(DstPort[DP_WDS0]); }
	if(DstPort[DP_WDS1]!=NULL) { dev_put(DstPort[DP_WDS1]); }
	if(DstPort[DP_WDS2]!=NULL) { dev_put(DstPort[DP_WDS2]); }
	if(DstPort[DP_WDS3]!=NULL) { dev_put(DstPort[DP_WDS3]); }
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
	if(DstPort[DP_APCLI0]!=NULL) { dev_put(DstPort[DP_APCLI0]); }
#endif
#if defined (CONFIG_RT2860V2_AP_MESH)
	if(DstPort[DP_MESH0]!=NULL) { dev_put(DstPort[DP_MESH0]); }
#endif
#if defined (CONFIG_RT3090_AP) || defined (CONFIG_RT3090_AP_MODULE)
	if(DstPort[DP_RAI0]!=NULL) { dev_put(DstPort[DP_RAI0]); }
#if defined (CONFIG_RT3090_AP_MBSS)
	if(DstPort[DP_RAI1]!=NULL) { dev_put(DstPort[DP_RAI1]); }
	if(DstPort[DP_RAI2]!=NULL) { dev_put(DstPort[DP_RAI2]); }
	if(DstPort[DP_RAI3]!=NULL) { dev_put(DstPort[DP_RAI3]); }
	if(DstPort[DP_RAI4]!=NULL) { dev_put(DstPort[DP_RAI4]); }
	if(DstPort[DP_RAI5]!=NULL) { dev_put(DstPort[DP_RAI5]); }
	if(DstPort[DP_RAI6]!=NULL) { dev_put(DstPort[DP_RAI6]); }
	if(DstPort[DP_RAI7]!=NULL) { dev_put(DstPort[DP_RAI7]); }
	if(DstPort[DP_RAI8]!=NULL) { dev_put(DstPort[DP_RAI8]); }
	if(DstPort[DP_RAI9]!=NULL) { dev_put(DstPort[DP_RAI9]); }
	if(DstPort[DP_RAI10]!=NULL) { dev_put(DstPort[DP_RAI10]); }
	if(DstPort[DP_RAI11]!=NULL) { dev_put(DstPort[DP_RAI11]); }
	if(DstPort[DP_RAI12]!=NULL) { dev_put(DstPort[DP_RAI12]); }
	if(DstPort[DP_RAI13]!=NULL) { dev_put(DstPort[DP_RAI13]); }
	if(DstPort[DP_RAI14]!=NULL) { dev_put(DstPort[DP_RAI14]); }
	if(DstPort[DP_RAI15]!=NULL) { dev_put(DstPort[DP_RAI15]); }
#endif // CONFIG_RT3090_AP_MBSS //
#endif
#if defined (CONFIG_RT3090_AP_APCLI)
	if(DstPort[DP_APCLII0]!=NULL) { dev_put(DstPort[DP_APCLII0]); }
#endif // CONFIG_RT3090_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH)
	if(DstPort[DP_MESHI0]!=NULL) { dev_put(DstPort[DP_MESHI0]); }
#endif // CONFIG_RT3090_AP_MESH //
	if(DstPort[DP_GMAC]!=NULL) { dev_put(DstPort[DP_GMAC]); }
#ifdef CONFIG_RAETH_GMAC2
	if(DstPort[DP_GMAC2]!=NULL) { dev_put(DstPort[DP_GMAC2]); }
#endif
	if(DstPort[DP_PCI]!=NULL) { dev_put(DstPort[DP_PCI]); }
#if defined(CONFIG_RA_HW_NAT_ATM)
	if(DstPort[DP_NAS0]!=NULL) { dev_put(DstPort[DP_NAS0]); }
	if(DstPort[DP_NAS1]!=NULL) { dev_put(DstPort[DP_NAS1]); }
	if(DstPort[DP_NAS2]!=NULL) { dev_put(DstPort[DP_NAS2]); }
	if(DstPort[DP_NAS3]!=NULL) { dev_put(DstPort[DP_NAS3]); }
	if(DstPort[DP_NAS4]!=NULL) { dev_put(DstPort[DP_NAS4]); }
	if(DstPort[DP_NAS5]!=NULL) { dev_put(DstPort[DP_NAS5]); }
	if(DstPort[DP_NAS6]!=NULL) { dev_put(DstPort[DP_NAS6]); }
	if(DstPort[DP_NAS7]!=NULL) { dev_put(DstPort[DP_NAS7]); }
#endif
    }

}

static uint32_t SetGdmaFwd(uint32_t Ebl) 
{
	uint32_t data=0;

	data=RegRead(FE_GDMA1_FWD_CFG);

	if(Ebl) {	
	    //Uni-cast frames forward to PPE
	    data |= GDM1_UFRC_P_PPE;
	    //Broad-cast MAC address frames forward to PPE
	    data |= GDM1_BFRC_P_PPE;
	    //Multi-cast MAC address frames forward to PPE
	    data |= GDM1_MFRC_P_PPE;
	    //Other MAC address frames forward to PPE
	    data |= GDM1_OFRC_P_PPE;

	}else {
	    //Uni-cast frames forward to CPU
	    data &= ~GDM1_UFRC_P_PPE;
	    //Broad-cast MAC address frames forward to CPU
	    data &= ~GDM1_BFRC_P_PPE;
	    //Multi-cast MAC address frames forward to CPU
	    data &= ~GDM1_MFRC_P_PPE;
	    //Other MAC address frames forward to CPU
	    data &= ~GDM1_OFRC_P_PPE;
	
	}

	RegWrite(FE_GDMA1_FWD_CFG, data);

#ifdef CONFIG_RAETH_GMAC2
	data=RegRead(FE_GDMA2_FWD_CFG);

	if(Ebl) {	
	    //Uni-cast frames forward to PPE
	    data |= GDM1_UFRC_P_PPE;
	    //Broad-cast MAC address frames forward to PPE
	    data |= GDM1_BFRC_P_PPE;
	    //Multi-cast MAC address frames forward to PPE
	    data |= GDM1_MFRC_P_PPE;
	    //Other MAC address frames forward to PPE
	    data |= GDM1_OFRC_P_PPE;

	}else {
	    //Uni-cast frames forward to CPU
	    data &= ~GDM1_UFRC_P_PPE;
	    //Broad-cast MAC address frames forward to CPU
	    data &= ~GDM1_BFRC_P_PPE;
	    //Multi-cast MAC address frames forward to CPU
	    data &= ~GDM1_MFRC_P_PPE;
	    //Other MAC address frames forward to CPU
	    data &= ~GDM1_OFRC_P_PPE;
	
	}
	RegWrite(FE_GDMA2_FWD_CFG, data);
#endif

	return 0;  
}

static int ppe_device_event(struct notifier_block *unused, unsigned long event, void *ptr);

struct notifier_block ppe_device_notifier = {
	.notifier_call = ppe_device_event
};

static int ppe_device_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
	struct net_device *dev = ptr;

	if (dev->name == NULL)
		return NOTIFY_DONE; 

	switch (event) {
	case NETDEV_REGISTER:
		if (strncmp(dev->name, "ra0", 3) == 0) {
			DstPort[DP_RA0]=ra_dev_get_by_name(dev->name); 
		} else if (strncmp(dev->name, "ra1", 3) == 0) {
			DstPort[DP_RA1]=ra_dev_get_by_name(dev->name); 
		} 
#if defined(CONFIG_RA_HW_NAT_ATM)
		else if ((strncmp(dev->name,"nas", 3) == 0) && (strlen(dev->name) == 4) &&
			 			((dev->name[3] >= '0') && (dev->name[3] <= '7'))) {
			int index = dev->name[3] - '0';
			DstPort[DP_NAS0 + index]=ra_dev_get_by_name(dev->name); 
		}
#endif
		break;

	case NETDEV_UNREGISTER:
		if (strncmp(dev->name, "ra0", 3) == 0) {
			if (DstPort[DP_RA0] != NULL) {
				dev_put(DstPort[DP_RA0]); 
				DstPort[DP_RA0] = NULL;
			}
		} else if (strncmp(dev->name, "ra1", 3) == 0) {
			if (DstPort[DP_RA1] != NULL) {
				dev_put(DstPort[DP_RA1]); 
				DstPort[DP_RA1] = NULL;
			}
		} 
#if defined(CONFIG_RA_HW_NAT_ATM)
		else if ((strncmp(dev->name,"nas", 3) == 0) && (strlen(dev->name) == 4) &&
			 			((dev->name[3] >= '0') && (dev->name[3] <= '7'))) {
			int index = dev->name[3] - '0';
			if (DstPort[DP_NAS0 + index] != NULL) {
				dev_put(DstPort[DP_NAS0 + index]); 
				DstPort[DP_NAS0 + index] = NULL;
			}
		}
#endif
		break;
	}

	return NOTIFY_DONE;
}

/*
 * PPE Enabled: GMAC<->PPE<->CPU
 * PPE Disabled: GMAC<->CPU
 */
static int32_t PpeInitMod(void)
{
    NAT_PRINT("Ralink HW NAT Module Enabled\n");

    //Get net_device structure of Dest Port 
    PpeSetDstPort(1);

    /* Register ioctl handler */
    PpeRegIoctlHandler();
    AclRegIoctlHandler();
    AcRegIoctlHandler();
    MtrRegIoctlHandler();

    /* Initialize PPE related register */
    PpeEngStart();

#if ! defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
    /* In manual mode, PPE always reports UN-HIT CPU reason, so we don't need to process it */
    /* Register RX/TX hook point */
    ra_sw_nat_hook_tx = PpeTxHandler;
    ra_sw_nat_hook_rx = PpeRxHandler;
#ifdef TCSUPPORT_RA_HWNAT
    ra_sw_nat_hook_free = PpeFreeHandler;
    ra_sw_nat_hook_rxinfo = PpeRxinfoHandler;
    ra_sw_nat_hook_txq = PpeTxqHandler;
    ra_sw_nat_hook_magic = PpeMagicHandler;
    ra_sw_nat_hook_set_magic = PpeSetMagicHandler;
    ra_sw_nat_hook_xfer = PpeXferHandler;
#endif
#endif

    /* Set GMAC fowrards packet to PPE */
    SetGdmaFwd(1);

	register_netdevice_notifier(&ppe_device_notifier);

#if 0  //we cannot use GPL-only symbol
    hnat_class = class_create(THIS_MODULE, "hnat");
    class_device_create(hnat_class, NULL, MKDEV(220, 0), NULL, "hwnat0");
    class_device_create(hnat_class, NULL, MKDEV(230, 0), NULL, "acl0");
    class_device_create(hnat_class, NULL, MKDEV(240, 0), NULL, "ac0");
    class_device_create(hnat_class, NULL, MKDEV(250, 0), NULL, "mtr0");
#endif

    return 0;
}

static void PpeCleanupMod(void)
{
    NAT_PRINT("Ralink HW NAT Module Disabled\n");

    /* Set GMAC fowrards packet to CPU */
    SetGdmaFwd(0);

#if ! defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
    /* Unregister RX/TX hook point */
    ra_sw_nat_hook_rx = NULL;
    ra_sw_nat_hook_tx = NULL;
#ifdef TCSUPPORT_RA_HWNAT
    ra_sw_nat_hook_free = NULL;
    ra_sw_nat_hook_rxinfo = NULL;
    ra_sw_nat_hook_txq = NULL;
    ra_sw_nat_hook_magic = NULL;
    ra_sw_nat_hook_set_magic = NULL;
    ra_sw_nat_hook_xfer = NULL;
#endif
#endif

    /* Restore PPE related register */
    PpeEngStop();

    /* Unregister ioctl handler */
    PpeUnRegIoctlHandler();
    AclUnRegIoctlHandler();
    AcUnRegIoctlHandler();
    MtrUnRegIoctlHandler();

    //Release net_device structure of Dest Port 
    PpeSetDstPort(0);

	unregister_netdevice_notifier(&ppe_device_notifier);

#if 0
    /* defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3352) || defined (CONFIG_RALINK_RT5350) || defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT63365) || defined (TCSUPPORT_RA_HWNAT) */  
    /* Restore switch port 6 flow control to default on */
    RegModifyBits(RALINK_ETH_SW_BASE+0xC8, 0x3, 8, 2);
#endif
#if 0  //we cannot use GPL-only symbol
    class_device_destroy(hnat_class, MKDEV(220, 0));
    class_device_destroy(hnat_class, MKDEV(230, 0));
    class_device_destroy(hnat_class, MKDEV(240, 0));
    class_device_destroy(hnat_class, MKDEV(250, 0));
    class_destroy(hnat_class);
#endif
}

/*HNAT QOS*/
int PpeSetDscpRemarkEbl(uint32_t enable)
{
    /* Re-generate DSCP */
    RegModifyBits(PPE_GLO_CFG, enable, 11, 1);
    return HWNAT_SUCCESS;
}

int PpeSetVpriRemarkEbl(uint32_t enable)
{
    /* Re-generate VLAN Priority */
    RegModifyBits(PPE_GLO_CFG, enable, 10, 1);
    return HWNAT_SUCCESS;
}

int PpeSetWeightFOE(uint32_t weight)
{
    /* Set weight of decision in resolution */
    RegModifyBits(UP_RES, weight, FUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}

int PpeSetWeightACL(uint32_t weight)
{ 
    /* Set weight of decision in resolution */
    RegModifyBits(UP_RES, weight, AUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}    

int PpeSetWeightDSCP(uint32_t weight)
{
    RegModifyBits(UP_RES, weight, DUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}

int PpeSetWeightVPRI(uint32_t weight)
{
    /* Set weight of decision in resolution */
    RegModifyBits(UP_RES, weight, VUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}

int PpeSetDSCP_UP(uint32_t DSCP_SET, unsigned char UP)
{ 
    int DSCP_UP;

    DSCP_UP = ((UP<<0) | (UP<<4) | (UP<<8) | (UP<<12)\
	    | (UP<<16) | (UP<<20) | (UP<<24) | (UP<<28));
    /* Set DSCP to User priority mapping table */ 
    switch(DSCP_SET)
    {	
    case 0:	
	RegWrite(DSCP0_7_MAP_UP, DSCP_UP);
	break;
    case 1:
	RegWrite(DSCP8_15_MAP_UP, DSCP_UP);
	break;
    case 2:
	RegWrite(DSCP16_23_MAP_UP, DSCP_UP);
	break;
    case 3:
	RegWrite(DSCP24_31_MAP_UP, DSCP_UP);
	break;
    case 4:
	RegWrite(DSCP32_39_MAP_UP, DSCP_UP);
	break;
    case 5:
	RegWrite(DSCP40_47_MAP_UP, DSCP_UP);
	break;
    case 6:
	RegWrite(DSCP48_55_MAP_UP, DSCP_UP);
	break;
    case 7:
	RegWrite(DSCP56_63_MAP_UP, DSCP_UP);
	break;
    default:

	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetUP_IDSCP(uint32_t UP, uint32_t IDSCP)
{    
    /* Set mapping table of user priority to in-profile DSCP */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 0, 6);
	break;
    case 1:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 8, 6);
	break;
    case 2:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 16, 6);
	break;
    case 3:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 24, 6);
	break;
    case 4:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 0, 6);
	break;
    case 5:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 8, 6);
	break;
    case 6:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 16, 6);
	break;
    case 7:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 24, 6);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}
int PpeSetUP_ODSCP(uint32_t UP, uint32_t ODSCP)
{    
    /* Set mapping table of user priority to out-profile DSCP */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 0, 6);
	break;
    case 1:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 8, 6);
	break;
    case 2:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 16, 6);
	break;
    case 3:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 24, 6);
	break;
    case 4:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 0, 6);
	break;
    case 5:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 8, 6);
	break;
    case 6:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 16, 6);
	break;
    case 7:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 24, 6);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetUP_VPRI(uint32_t UP, uint32_t VPRI)
{    
    /* Set mapping table of user priority to vlan priority */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP_MAP_VPRI, VPRI, 0, 3);
	break;
    case 1:
	RegModifyBits(UP_MAP_VPRI, VPRI, 4, 3);
	break;
    case 2:
	RegModifyBits(UP_MAP_VPRI, VPRI, 8, 3);
	break;
    case 3:
	RegModifyBits(UP_MAP_VPRI, VPRI, 12, 3);
	break;
    case 4:
	RegModifyBits(UP_MAP_VPRI, VPRI, 16, 3);
	break;
    case 5:
	RegModifyBits(UP_MAP_VPRI, VPRI, 20, 3);
	break;
    case 6:
	RegModifyBits(UP_MAP_VPRI, VPRI, 24, 3);
	break;
    case 7:
	RegModifyBits(UP_MAP_VPRI, VPRI, 28, 3);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetUP_AC(uint32_t UP, uint32_t AC)
{
    /* Set mapping table of user priority to access category */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP_MAP_AC, AC, 0, 2);
	break;
    case 1:
	RegModifyBits(UP_MAP_AC, AC, 2, 2);
	break;
    case 2:
	RegModifyBits(UP_MAP_AC, AC, 4, 2);
	break;
    case 3:
	RegModifyBits(UP_MAP_AC, AC, 6, 2);
	break;
    case 4:
	RegModifyBits(UP_MAP_AC, AC, 8, 2);
	break;
    case 5:
	RegModifyBits(UP_MAP_AC, AC, 10, 2);
	break;
    case 6:
	RegModifyBits(UP_MAP_AC, AC, 12, 2);
	break;
    case 7:
	RegModifyBits(UP_MAP_AC, AC, 14, 2);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetSchMode(uint32_t policy)
{
    /* Set GDMA1&2 Schduling Mode */
    RegModifyBits(FE_GDMA1_SCH_CFG, policy, 24, 2);
    RegModifyBits(FE_GDMA2_SCH_CFG, policy, 24, 2);

    return HWNAT_SUCCESS;
}

/* In general case, we only need 1/2/4/8 weight */
int PpeWeightRemap(uint8_t W)
{
    switch(W)
    {
    case 8:
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
	return 3;
#else
	return 7;
#endif
    case 4:
#if defined (CONFIG_RALINK_RT3052) || defined (CONFIG_RALINK_RT3883)
	return 2;
#else
	return 3;
#endif
    case 2:
	return 1;
    case 1:
	return 0;
    default:
	/* invalid value */
	return -1;
    }
}

int PpeSetSchWeight(uint8_t W0, uint8_t W1, uint8_t W2, uint8_t W3)
{
    int32_t _W0, _W1, _W2, _W3;

    _W0=PpeWeightRemap(W0);
    _W1=PpeWeightRemap(W1);
    _W2=PpeWeightRemap(W2);
    _W3=PpeWeightRemap(W3);

    if((_W0==-1) || (_W1==-1) || (_W2==-1) || (_W3==-1)) {
	return HWNAT_FAIL;
    }

    /* Set GDMA1 Schduling Weight */
    RegModifyBits(FE_GDMA1_SCH_CFG, _W0, 0, 3);
    RegModifyBits(FE_GDMA1_SCH_CFG, _W1, 4, 3);
    RegModifyBits(FE_GDMA1_SCH_CFG, _W2, 8, 3);
    RegModifyBits(FE_GDMA1_SCH_CFG, _W3, 12, 3);

    /* Set GDMA2 Schduling Weight */
    RegModifyBits(FE_GDMA2_SCH_CFG, _W0, 0, 3);
    RegModifyBits(FE_GDMA2_SCH_CFG, _W1, 4, 3);
    RegModifyBits(FE_GDMA2_SCH_CFG, _W2, 8, 3);
    RegModifyBits(FE_GDMA2_SCH_CFG, _W3, 12, 3);

    return HWNAT_SUCCESS;
}


int PpeSetBindThreshold(uint32_t threshold)
{
   /* Set reach bind rate for unbind state */
    RegWrite(PPE_FOE_BNDR, threshold);

    return HWNAT_SUCCESS;
}

int PpeSetMaxEntryLimit(uint32_t full, uint32_t half, uint32_t qurt)
{
	/* Allowed max entries to be build during a time stamp unit */

	/* smaller than 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, qurt, 0, 14);

	/* between 1/2 and 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, half, 16, 14);

	/* between full and 1/2 of total entries */
	RegModifyBits(PPE_FOE_LMT2, full, 0, 14);

    return HWNAT_SUCCESS;
}

int PpeSetRuleSize(uint16_t pre_acl, uint16_t pre_meter, uint16_t pre_ac, uint16_t post_meter, uint16_t post_ac)
{

/* Pre Access Control List Rule Start Index */
	GLOBAL_PRE_ACL_STR = 0;

    /* Pre Access Control List Rule End Index */
	GLOBAL_PRE_ACL_END = GLOBAL_PRE_ACL_STR;

    /* Pre Meter Rule Start Index */
	GLOBAL_PRE_MTR_STR = GLOBAL_PRE_ACL_STR + pre_acl;
     /* Pre Meter Rule End Index */
	GLOBAL_PRE_MTR_END = GLOBAL_PRE_MTR_STR;

    /* Pre Accounting Rule Start Index */
	GLOBAL_PRE_AC_STR = GLOBAL_PRE_MTR_STR + pre_meter;

    /* Pre Accounting Rule End Index */
	GLOBAL_PRE_AC_END = GLOBAL_PRE_AC_STR;

    /* Post Meter Rule Start Index */
	GLOBAL_POST_MTR_STR = GLOBAL_PRE_AC_STR + pre_ac;

    /* Post Meter Rule End Index */
	GLOBAL_POST_MTR_END = GLOBAL_POST_MTR_STR;

    /* Post Accounting Rule Start Index */
	GLOBAL_POST_AC_STR = GLOBAL_POST_MTR_STR + post_meter;

    /* Post Accounting Rule End Index */
	GLOBAL_POST_AC_END = GLOBAL_POST_AC_STR;




    /* Set Pre ACL Table */
    RegModifyBits(PPE_PRE_ACL, GLOBAL_PRE_ACL_STR, 0, 9);
    RegModifyBits(PPE_PRE_ACL, GLOBAL_PRE_ACL_END, 16, 9);
    /* Set Pre AC Table */
    RegModifyBits(PPE_PRE_AC, GLOBAL_PRE_AC_STR, 0, 9);
    RegModifyBits(PPE_PRE_AC, GLOBAL_PRE_AC_END, 16, 9);

    /* Set Post AC Table */
    RegModifyBits(PPE_POST_AC, GLOBAL_POST_AC_STR, 0, 9);
    RegModifyBits(PPE_POST_AC, GLOBAL_POST_AC_END, 16, 9);
    /* Set Pre MTR Table */
    RegModifyBits(PPE_PRE_MTR, GLOBAL_PRE_MTR_STR, 0, 9);
    RegModifyBits(PPE_PRE_MTR, GLOBAL_PRE_MTR_END, 16, 9);

    /* Set Post MTR Table */
    RegModifyBits(PPE_POST_MTR, GLOBAL_POST_MTR_STR, 0, 9);
    RegModifyBits(PPE_POST_MTR, GLOBAL_POST_MTR_END, 16, 9);



    return HWNAT_SUCCESS;
}

int PpeSetKaInterval(uint8_t tcp_ka, uint8_t udp_ka)
{
	/* Keep alive time for bind FOE TCP entry */
	RegModifyBits(PPE_FOE_KA, tcp_ka, 16, 8);

	/* Keep alive timer for bind FOE UDP entry */
	RegModifyBits(PPE_FOE_KA, udp_ka, 24, 8);


    return HWNAT_SUCCESS;
}

int PpeSetUnbindLifeTime(uint8_t lifetime)
{
	/* set Delta time for aging out an unbind FOE entry */
	RegModifyBits(PPE_FOE_UNB_AGE, lifetime, 0, 8);

    return HWNAT_SUCCESS;
}

int PpeSetBindLifetime(uint16_t tcp_life, uint16_t udp_life, uint16_t fin_life)
{

	/* set Delta time for aging out an bind UDP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE1, udp_life, 0, 16);

	/* set Delta time for aging out an bind TCP FIN FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, fin_life, 16, 16);

	/* set Delta time for aging out an bind TCP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, tcp_life, 0, 16);




    return HWNAT_SUCCESS;
}


module_init(PpeInitMod);
module_exit(PpeCleanupMod);

MODULE_AUTHOR("Steven Liu/Kurtis Ke");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ralink Hardware NAT v0.91\n");

