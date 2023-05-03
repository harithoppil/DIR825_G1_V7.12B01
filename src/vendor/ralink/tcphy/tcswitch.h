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
#if 0
#define SKBUF_COPYTOLAN (1 << 22)
#define SKBUF_TCCONSOLE (1 << 23)
#else
#define SKBUF_COPYTOLAN (1 << 26)
#define SKBUF_TCCONSOLE (1 << 27)
#if !defined(TCSUPPORT_CT)
#if 1//def VPORT
#define SKBUF_VLAN 		(1 << 28)
#endif
#endif

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

typedef struct swic_reg{
	uint16 model_id;
	uint16 phyaddr;
	uint16 regaddr;
	uint16 value;
}swic_reg_t;

void periodChk(void);
void refillPhyDefVal(uint32 enetPhyAddr, uint32 phyReg, uint32 miiData);
int macRxPortEnable(int ChanID);
int macAdmSTagEnable(int ChanID);
int macIpSTagEnable(int ChanID);
void macIpSTagInsert(struct sk_buff *bp);
void macIpSTagRemove(struct sk_buff *bp);
int macTC2206STagEnable(int chanID);
void macTC2206STagInsert(struct sk_buff *bp);
void macTC2206STagRemove(struct sk_buff *bp);
#if defined(RT62806_VERIFY)
int macRT62806STagEnable(int chanID);
struct sk_buff *macRT62806STagInsert(struct sk_buff *bp);
void macRT62806STagRemove(struct sk_buff *bp);
#endif
void filedSwicDefVal(void);
int getTC2206InitRegFlag(void);
void updatingUpnpGid(void);
void esdSwPatch(void);

#if defined(TCSUPPORT_WAN_ETHER)
int isComeFromWan(struct sk_buff *skb);
void setComeFromWan(struct sk_buff *skb, int yes);
//#define WAN_PORT_ID 0x4
#define CB_MAGIC	0x3262
#endif

void enable_abnormal_irq(void);
int polling_abnormal_irq(void);
struct sk_buff *macRT63365STagInsert(struct sk_buff *bp);
void macRT63365STagRemove(struct sk_buff *bp);
void init_ethernet_port_map(void);
void duplex_sw_patch(void);
#endif /* _TCSWITCH_H_ */
