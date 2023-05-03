
/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* �ļ����� : tbs_nfp_skb_parser.h
* �ļ����� : tbs���������ݰ�����ͷ�ļ�
*
* �޶���¼ :
*          1 ���� : cairong
*            ���� : 2011-12-03
*            ���� :
*
*******************************************************************************/

#ifndef __TBS_NFP_SKB_PARSER_H__
#define __TBS_NFP_SKB_PARSER_H__

//#include "tbs_nfp_defs.h"
#include "tbs_nfp_common.h"

#include <linux/skbuff.h>
#include <linux/ppp_defs.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/if_pppox.h>
#include<linux/ip.h>
#include<net/ip.h>
#include<linux/ipv6.h>
#include <net/ipv6.h>
#include <linux/icmp.h>
#include <linux/socket.h>

#include "tbs_nfp.h"
/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/

//һ��VLAN���
#define TBS_NFP_RX_VLAN_FRAME_BIT                15
#define TBS_NFP_RX_VLAN_FRAME_MASK               7<<TBS_NFP_RX_VLAN_FRAME_BIT
#define TBS_NFP_RX_VLAN_FRAME_TYPE               (1<<TBS_NFP_RX_VLAN_FRAME_BIT)
//����VLAN��� qinq
#define TBS_NFP_RX_QINQ_FRAME_TYPE               (2<<TBS_NFP_RX_VLAN_FRAME_BIT)
//�������������VLAN��� more qinq
#define TBS_NFP_RX_MORE_QINQ_FRAME_TYPE          (4<<TBS_NFP_RX_VLAN_FRAME_BIT)

//ppp֡
#define TBS_NFP_RX_PPPOE_FRAME_BIT                18
#define TBS_NFP_RX_PPPOE_FRAME_MASK               (1<<TBS_NFP_RX_PPPOE_FRAME_BIT)

//ipv4��
#define TBS_NFP_RX_IP_FRAME_TYPE_BIT              19
#define TBS_NFP_RX_IP_FRAME_TYPE_MASK             (1<<TBS_NFP_RX_IP_FRAME_TYPE_BIT)

//ipv6��
#define TBS_NFP_RX_IP6_FRAME_TYPE_BIT             20
#define TBS_NFP_RX_IP6_FRAME_TYPE_MASK            (1<<TBS_NFP_RX_IP6_FRAME_TYPE_BIT)

//��ipv4��v6�İ�
#define TBS_NFP_RX_L3_UNKNOWN_BIT                 21
#define TBS_NFP_RX_L3_UNKNOWN_MASK                (1<<TBS_NFP_RX_L3_UNKNOWN_BIT)

//tcp ��
#define TBS_NFP_RX_L4_TCP_BIT                    22
#define TBS_NFP_RX_L4_TCP_TYPE                   (1<<TBS_NFP_RX_L4_TCP_BIT)

//udp ��
#define TBS_NFP_RX_L4_UDP_BIT                    23
#define TBS_NFP_RX_L4_UDP_TYPE                   (1<<TBS_NFP_RX_L4_UDP_BIT)

//���Ĳ�δ֪��
#define TBS_NFP_RX_L4_UNKNOWN_BIT               24
#define TBS_NFP_RX_L4_UNKNOWN_MASK              (1<<TBS_NFP_RX_L4_UNKNOWN_BIT)


//��״̬����
#define TBS_NFP_RX_PARSER_MASK                   0xFFFFFFFF

//��VLAN��
#define TBS_NFP_RX_ISNOT_VLAN(status)       (!((status)&TBS_NFP_RX_VLAN_FRAME_MASK))

/*�Ƿ�ֻ��һ��vlan��*/
#define TBS_NFP_RX_IS_VLAN(status)         ((status)&TBS_NFP_RX_VLAN_FRAME_TYPE)

/*�Ƿ�ֻ������vlan��*/
#define TBS_NFP_RX_IS_QINQ(status)         ((status)&TBS_NFP_RX_QINQ_FRAME_TYPE)

/*�Ƿ���������������vlan��*/
#define TBS_NFP_RX_IS_MORE_QINQ(status)    ((status)&TBS_NFP_RX_MORE_QINQ_FRAME_TYPE)
//ppp֡
#define TBS_NFP_RX_IS_PPP(status)          ((status) & TBS_NFP_RX_PPPOE_FRAME_MASK)

#define TBS_NFP_RX_L3_IS_IP4(status)      ((status) & TBS_NFP_RX_IP_FRAME_TYPE_MASK)
#define TBS_NFP_RX_L3_IS_IP6(status)      ((status) & TBS_NFP_RX_IP6_FRAME_TYPE_MASK)
#define TBS_NFP_RX_L3_UNKNOW(status)      ((status) & TBS_NFP_RX_L3_UNKNOWN_MASK)

#define TBS_NFP_RX_L4_IS_TCP(status)      ((status) &  TBS_NFP_RX_L4_TCP_TYPE)
#define TBS_NFP_RX_L4_IS_UDP(status)      ((status) &  TBS_NFP_RX_L4_UDP_TYPE)
#define TBS_NFP_RX_L4_UNKNOW(status)      ((status) & TBS_NFP_RX_L4_UNKNOWN_MASK)


/******************************************************************************
 *                                 STRUCT                                     *
 ******************************************************************************/
typedef struct tbs_nfp_rx_desc {
    unsigned int  status;

    /*�İ�������mac_headerֵ���䣬ͨ��shiftֵ��̬�����µ�mac_headerλ��*/
    unsigned char *mac_header;
    unsigned int l3_offset;
    struct sk_buff *skb;
    int shift;

   /*   unsigned int  pncInfo;
    unsigned int  dataSize;
    unsigned int  bufPhysAddr;
    unsigned int  pncFlowId;
    unsigned int  bufCookie;
    unsigned int  prefetchCmd;
    unsigned int  csumL4;
    unsigned int  pncExtra;
    unsigned int  hw_cmd;
    */
} TBS_NFP_RX_DESC;


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/
#if defined(CONFIG_TBS_NFP_DEBUG)
void tbs_nfp_debug_check_skbparser(struct tbs_nfp_rx_desc* skb_desc);
void tbs_nfp_dump_packet(const struct sk_buff *skb,	const unsigned char *mac_header);
#endif

int tbs_nfp_skb_parser(const struct  sk_buff *skb, TBS_NFP_RX_DESC *skb_desc);
#endif /* __TBS_NFP_SKB_PARSER_H__ */

