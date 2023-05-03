/************************************************************************
 *
 *	Copyright (C) 2006 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
#ifndef _TCSWITCH_H_
#define _TCSWITCH_H_

/* auto detect 4 port switch ic */
#define SWIC_ADM6996M		1
#define SWIC_IP175C			2
#define SWIC_MARVEL6060     3
#define SWIC_RTL8305        4
#define SWIC_RTL8306SD		5
#define SWIC_TC2206	7
#define SWIC_RT63365		8
#define SWIC_RT62806		9
#define HW_IGMPSNOOP	(1<<0)

#define RX_STAG_LEN 8
#define TX_STAG_LEN 6
#define DEF_STAG_VALUE 0x8901
#define IPPROTOL	0x0800
#define SKBUF_COPYTOLAN (1 << 26)
#define SKBUF_TCCONSOLE (1 << 27)
#if !defined(TCSUPPORT_CT)
#define SKBUF_VLAN 		(1 << 28)
#endif

#define MODEL_V0	(1<<0)	/*TC2206A1*/
#define MODEL_V1	(1<<1)	/*TC2206B2*/
#define MODEL_V2	(1<<2)	/*TC2206F*/
#if !defined(TCSUPPORT_CT)
#if 1//def VPORT
#define VLAN_HLEN 4
#define BASE_VID_IDX 1
#endif
#endif

#define RT63365_SWIC_PORTNUM	7

#define VLAN_TPID                     0x80 /* Special TAG, not VLAN tag ETH_P_8021Q */
#define VLAN_BASE_ID                  0x001


typedef struct swic_reg{
	uint16 model_id;
	uint16 phyaddr;
	uint16 regaddr;
	uint16 value;
}swic_reg_t;

extern struct sk_buff *tc2105mj_vlan_tag_insert(struct sk_buff * skb, struct net_device * dev);
extern int tc2105mj_vlan_tag_remove(struct sk_buff * skb);
extern int __init tc2105mj_switch_init(void);
extern void __exit tc2105mj_switch_exit(void);

#endif /* _TCSWITCH_H_ */
