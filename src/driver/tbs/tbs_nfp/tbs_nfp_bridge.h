
/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_bridge.h
* 文件描述 : tbs加速器桥转发文件
*
* 修订记录 :
*          1 创建 : pengyao
*            日期 : 2011-12-06
*            描述 :
*
*******************************************************************************/

#ifndef __TBS_NFP_BIRDGE_H__
#define __TBS_NFP_BIRDGE_H__

#include <linux/skbuff.h>
#include <linux/if_ether.h>

#include "tbs_nfp_defs.h"
#include "tbs_nfp_common.h"
#include "tbs_nfp_itf.h"
#include "tbs_nfp_skb_parser.h"


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
#define TBS_NFP_BRIDGE_RULE_DEF 256
#define TBS_NFP_BRIDGE_INV		0
#define TBS_NFP_BRIDGE_LOCAL	1
#define TBS_NFP_BRIDGE_NON_LOCAL	2

/******************************************************************************
 *                                 ENUM                                       *
 ******************************************************************************/


/******************************************************************************
 *                                 STRUCT                                     *
 ******************************************************************************/
#ifdef TBS_ETH_NFP_FDB
typedef struct tbs_nfp_rule_fdb {
    /*hash表链节点*/
	struct list_head list;
	unsigned short  reserved;
	unsigned char   mac[ETH_ALEN];
	int	    bridgeIf;
	int	    if_index;
	unsigned int count;
	unsigned char	status;
} TBS_NFP_RULE_FDB;
#endif

typedef struct nfp_rule_bridge {
    /*g_arp_hash链节点*/
	struct list_head list;

	unsigned short  reserved;
	unsigned char	da[ETH_ALEN];
	unsigned char	sa[ETH_ALEN];
	int	iif;
	int	oif;
	unsigned int	flags;
	unsigned int count;
} TBS_NFP_RULE_BRIDGE;

/******************************************************************************
 *                                 DEBUG                                      *
 ******************************************************************************/
/* Enable / Disable TBS_NFP Debug Prints: */
#define TBS_NFP_BRIDGE_DBG(x...)  if (g_debug_level & TBS_NFP_DBG_PRINT) printk(x)
/*#define TBS_NFP_BRIDGE_WARN(x...)*/

#define TBS_NFP_BRIDGE_WARN(x...)  if (g_debug_level & TBS_NFP_WARN_PRINT) printk(x)
/*#define TBS_NFP_BRIDGE_WARN(x...)*/


#if 0
    #define TBS_NFP_BRIDGE_DEBUG(fmt, args...) TBS_NFP_DEBUG(fmt, ##args)
#else
    #define TBS_NFP_BRIDGE_DEBUG(fmt, args...)  do { ; } while(0)
#endif


#define TBS_NFP_BRIDGE_DEBUG_LEVEL(level, fmt, args...)\
    do {  \
            printk(level "[%s:%4d] %20s: " fmt, strrchr(__FILE__, '/'), __LINE__, __FUNCTION__, ##args); \
    } while(0)

#define TBS_NFP_BRIDGE_INTO_FUNC  TBS_NFP_BRIDGE_DEBUG("##In## %s\n", __func__) //do { ; } while(0)
#define TBS_NFP_BRIDGE_OUT_FUNC TBS_NFP_BRIDGE_DEBUG("##Out## %s\n", __func__) //do { ; } while(0)

//#define TBS_NFP_BRIDGE_INTO_FUNC  do { ; } while(0)
//#define TBS_NFP_BRIDGE_OUT_FUNC do { ; } while(0)


/******************************************************************************
 *                                 TOOLS                                      *
 ******************************************************************************/


/******************************************************************************
 *                                GLOBAL                                      *
 ******************************************************************************/
extern struct list_head g_bridge_hash[];


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/


/************
 * Bridging
 ************/

int tbs_nfp_bridge_rule_add(const u8 *sa, const u8 *da, int iif, int oif);
int tbs_nfp_bridge_rule_delete(const u8 *sa, const u8 *da, int iif);
int tbs_nfp_bridge_rule_age(const u8 *sa, const u8 *da, int iif);

int tbs_nfp_bridge_rule_cmp(const unsigned char *da, const unsigned char *sa,
            int iif, TBS_NFP_RULE_BRIDGE *rule);
TBS_NFP_RULE_BRIDGE *tbs_nfp_bridge_rule_lookup(const unsigned char *da,
            const unsigned char *sa, int iif);

void tbs_nfp_bridge_rule_clear_by_ifindex(int ifindex);
void tbs_nfp_bridge_rule_print(int hash_key, const TBS_NFP_RULE_BRIDGE * br_rule);
void tbs_nfp_bridge_reset(void);
void tbs_nfp_bridge_rule_dump(void);
int tbs_nfp_bridge_rule_init(void);
void tbs_nfp_bridge_rule_exit(void);
bool tbs_nfp_if_on_same_bridge(TBS_NFP_IF_MAP *if_map1, TBS_NFP_IF_MAP *if_map2);
unsigned short tbs_nfp_bridge_if_get(int port_if);


#endif /* __TBS_NFP_BIRDGE_H__ */
