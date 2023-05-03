
/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* �ļ����� : tbs_nfp_arp.h
* �ļ����� : tbs������·��ת��ͷ�ļ�
*
* �޶���¼ :
*          1 ���� : pengyao
*            ���� : 2011-12-06
*            ���� :
*
*******************************************************************************/

#ifndef __TBS_NFP_ARP_H__
#define __TBS_NFP_ARP_H__

#include "tbs_nfp_defs.h"
#include "tbs_nfp_common.h"
#include <linux/skbuff.h>
#include <linux/ppp_defs.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/if_pppox.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>

#include "tbs_nfp.h"

/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
#define TBS_NFP_ARP_RULE_MAX    1024



/******************************************************************************
 *                                 ENUM                                       *
 ******************************************************************************/



/******************************************************************************
 *                                 STRUCT                                     *
 ******************************************************************************/
typedef struct tbs_nfp_rule_arp {
    /*g_arp_hash���ڵ�*/
	struct list_head list;

	int	family;
	unsigned char	next_hop_l3[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned char	da[ETH_ALEN];
} TBS_NFP_RULE_ARP;

/******************************************************************************
 *                                 DEBUG                                      *
 ******************************************************************************/


/******************************************************************************
 *                                 TOOLS                                      *
 ******************************************************************************/



/******************************************************************************
 *                                GLOBAL                                      *
 ******************************************************************************/
extern struct list_head g_arp_hash[TBS_NFP_ARP_HASH_SIZE];


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/
int tbs_nfp_arp_add(int family, const unsigned char *next_hop, const unsigned char *mac);
int tbs_nfp_arp_delete(int family, const unsigned char *next_hop);
int tbs_nfp_arp_age(int family, const unsigned char *next_hop);

/*������غ���*/
TBS_NFP_RULE_ARP *tbs_nfp_arp_lookup(int family, const unsigned char *nextHopL3);
void tbs_nfp_arp_reset(void);
void tbs_nfp_arp_dump(void);

void tbs_nfp_arp_exit(void);
int tbs_nfp_arp_init(void);

#endif /* __TBS_NFP_ARP_H__ */
