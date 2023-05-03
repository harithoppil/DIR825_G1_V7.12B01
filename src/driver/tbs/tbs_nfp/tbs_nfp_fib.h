
/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_fib.h
* 文件描述 : tbs加速器路由转发头文件
*
* 修订记录 :
*          1 创建 : pengyao
*            日期 : 2011-12-06
*            描述 :
*
*******************************************************************************/

#ifndef __TBS_NFP_FIB_H__
#define __TBS_NFP_FIB_H__

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

#ifdef TBS_NFP_BRIDGE
#include "tbs_nfp_bridge.h"
#endif

/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
#define TBS_NFP_FIB_RULE_DEF    2048



/******************************************************************************
 *                                 ENUM                                       *
 ******************************************************************************/



/******************************************************************************
 *                                 STRUCT                                     *
 ******************************************************************************/
typedef struct tbs_nfp_rule_fib {
    /*g_fib_hash或g_fib_inv_hash表链节点*/
	struct list_head list;
    /*g_fib_hash_by_gtw或g_fib_inv_hash_by_gtw链节点*/
    struct list_head list_by_gtw;

	int	family;
 	unsigned char	src_l3[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned char	dst_l3[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned char	da[ETH_ALEN];
	unsigned char	sa[ETH_ALEN];
	unsigned int	count;
	unsigned char	def_gtw_l3[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned int	flags;
	int     oif;
	unsigned int	ref;
} TBS_NFP_RULE_FIB;




/******************************************************************************
 *                                 DEBUG                                      *
 ******************************************************************************/
    /* Enable / Disable TBS_NFP Debug Prints: */
#define TBS_NFP_FIB_DBG(x...)  if (g_debug_level & TBS_NFP_DBG_PRINT) printk(x)
    /*#define TBS_NFP_FIB_WARN(x...)*/

#define TBS_NFP_FIB_WARN(x...)  if (g_debug_level & TBS_NFP_WARN_PRINT) printk(x)
    /*#define TBS_NFP_FIB_WARN(x...)*/


#if 0
    #define TBS_NFP_FIB_DEBUG(fmt, args...) TBS_NFP_DEBUG(fmt, ##args)
#else
    #define TBS_NFP_FIB_DEBUG(fmt, args...)  do { ; } while(0)
#endif


#define TBS_NFP_FIB_DEBUG_LEVEL(level, fmt, args...)\
        do {  \
                printk(level "[%s:%4d] %20s: " fmt, strrchr(__FILE__, '/'), __LINE__, __FUNCTION__, ##args); \
        } while(0)

#define TBS_NFP_FIB_INTO_FUNC  TBS_NFP_FIB_DEBUG("##In## %s\n", __func__) //do { ; } while(0)
#define TBS_NFP_FIB_OUT_FUNC TBS_NFP_FIB_DEBUG("##Out## %s\n", __func__) //do { ; } while(0)

//#define TBS_NFP_FIB_INTO_FUNC  do { ; } while(0)
//#define TBS_NFP_FIB_OUT_FUNC do { ; } while(0)



/******************************************************************************
 *                                 TOOLS                                      *
 ******************************************************************************/



/******************************************************************************
 *                                GLOBAL                                      *
 ******************************************************************************/

/*fib规则的hash表*/
extern struct list_head g_fib_hash[];
extern struct list_head g_fib_hash_by_gtw[];
extern struct list_head g_fib_inv_hash[];
extern struct list_head g_fib_inv_hash_by_gtw[];


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

#ifdef TBS_NFP_BRIDGE
void tbs_nfp_fib_bridge_update(const TBS_NFP_RULE_BRIDGE *bridge_rule);
void tbs_nfp_fib_bridge_invalid(int br_index, int br_port_index, const unsigned char *da);
#endif

/*arp规则的变更，对fib模块的影响。fib模块与arp模块交互接口*/
void tbs_nfp_fib_arp_invalid(int family, const unsigned char *next_hop);
void tbs_nfp_fib_arp_update(int family, const unsigned char *next_hop, const unsigned char *mac);

/*fib子模块内部接口*/
void tbs_nfp_fib_invalid(TBS_NFP_RULE_FIB *fib,unsigned int inv_flag);
void tbs_nfp_fib_valid(TBS_NFP_RULE_FIB *fib,unsigned int inv_flag);


/*fib子模块外部接口，添加删除查询fib规则*/
int tbs_nfp_fib_add(int family, const unsigned char *sip, const unsigned char *dip,
    const unsigned char *gtw, int oif);
int tbs_nfp_fib_delete(int family, const unsigned char *sip, const unsigned char *dip);
int tbs_nfp_fib_age(int family, const unsigned char *sip, const unsigned char *dip);
void tbs_nfp_fib_dump(void);
void tbs_nfp_fib_reset(void);

TBS_NFP_RULE_FIB *tbs_nfp_fib_lookup(int family, const unsigned char *src_l3,
		const unsigned char *dst_l3, const bool valid);

void tbs_nfp_fib_exit(void);
int tbs_nfp_fib_init(void);

#endif /* __TBS_NFP_FIB_H__ */
