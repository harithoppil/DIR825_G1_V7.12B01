
/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp.h
* 文件描述 : tbs加速器头文件
*
* 修订记录 :
*          1 创建 : pengyao
*            日期 : 2011-11-10
*            描述 :
*
*******************************************************************************/

#ifndef __TBS_NFP_H__
#define __TBS_NFP_H__

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
#include <linux/socket.h>

#include "tbs_nfp_skb_parser.h"
#include "tbs_nfp_defs.h"
#include "tbs_nfp_common.h"
#include "tbs_nfp_itf.h"


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
#define TBS_NFP_VERSION    "TBS_NFP_1_00_1"

#ifdef TBS_NFP_BRIDGE
#define TBS_NFP_BRIDGE_INV		0
#define TBS_NFP_BRIDGE_LOCAL	1
#define TBS_NFP_BRIDGE_NON_LOCAL	2
#endif /* TBS_NFP_BRIDGE */

/*Lock define*/
#define TBS_NFP_READ_LOCK(lock)       read_lock_bh(lock)
#define TBS_NFP_READ_UNLOCK(lock)     read_unlock_bh(lock)

#define TBS_NFP_WRITE_LOCK(lock)      write_lock_bh(lock)
#define TBS_NFP_WRITE_UNLOCK(lock)    write_unlock_bh(lock)


/*********
 * Flags
 *********/
/* Flags relevant for TBS_NFP_IF_MAP, to be used in TBS_NFP_IF_MAP flags */
#define TBS_NFP_F_MAP_INT           0x0001  /* 代表物理接口 */
#define TBS_NFP_F_MAP_VLAN          0x0002
#define TBS_NFP_F_MAP_PPPOE         0x0004
#define TBS_NFP_F_MAP_DUMMYPORT     0x0008
#define TBS_NFP_F_MAP_WLAN          0x0010
#define TBS_NFP_F_MAP_BRIDGE        0x0020
#define TBS_NFP_F_MAP_BRIDGE_PORT   0x0040
#define TBS_NFP_F_MAP_VNET          0x0080
#define TBS_NFP_F_MAP_EXT           0x0100


/* Flags relevant for TBS_NFP Bridging, to be used in TBS_NFP_RULE_BRIDGE flags */
#define TBS_NFP_F_BR_SET_VLAN_PRIO  0x1
#define TBS_NFP_F_BR_SET_TXQ        0x2

/* Flags relevant for TBS_NFP FIB rules, to be used in TBS_NFP_RULE_FIB flags */
#define TBS_NFP_F_FIB_BRIDGE_INV    0x1
#define TBS_NFP_F_FIB_ARP_INV       0x2
#define TBS_NFP_F_FIB_INV_MASK	    (TBS_NFP_F_FIB_BRIDGE_INV | TBS_NFP_F_FIB_ARP_INV)

/* Flags relevant for 5 Tuple TBS_NFP mode (CT), to be used in TBS_NFP_RULE_CT flags */
#define	TBS_NFP_F_CT_SNAT           0x1
#define	TBS_NFP_F_CT_DNAT           0x2
#define TBS_NFP_F_CT_FIB_INV        0x4
#define TBS_NFP_F_CT_INV_MASK      (TBS_NFP_F_CT_FIB_INV)

#define TBS_NFP_INVALID_VLAN        4096


#define	TBS_NFP_NO_PRINT	(0x0)
#define	TBS_NFP_WARN_PRINT	(0x1)
#define	TBS_NFP_DBG_PRINT	(0x2 | TBS_NFP_WARN_PRINT)


#define TBS_NFP_ERR         (-1)    /*处理失败*/
#define TBS_NFP_OK          (0)     /*加速器处理完成，返回成功*/
#define TBS_NFP_TERMINATE   (1)	    /*加速器处理失败，返回协议栈处理*/
#define TBS_NFP_DROPPED     (2)	    /*加速器处理失败，丢弃packet*/
#define TBS_NFP_CONTINUE    (3)     /*当前函数处理正常，继续下一步处理*/

/******************************************************************************
 *                                 ENUM                                       *
 ******************************************************************************/



/******************************************************************************
 *                                 STRUCT                                     *
 ******************************************************************************/
//时间戳
/*__net_timestamp*/


/*****************
 * stat
 *****************/
#ifdef TBS_NFP_STAT
typedef struct {
	unsigned int rx;
    unsigned int tx;
	unsigned int iif_err;
    unsigned int paser_err;
	unsigned int mac_mcast;
	unsigned int non_ip;
	unsigned int ipv4;
	unsigned int ipv6;
	unsigned int ipv4_rx_frag;
	unsigned int ttl_exp;
	unsigned int l4_unknown;
    unsigned int tx_mtu_err;

#ifdef TBS_NFP_BRIDGE
	unsigned int bridge_hit;
	unsigned int bridge_miss;
	unsigned int bridge_local;
#endif /* TBS_NFP_BRIDGE */

#ifdef TBS_NFP_VLAN
    unsigned int vlan_rx_not_found;
	unsigned int vlan_rx_found;
	unsigned int vlan_rx_trans;
	unsigned int vlan_tx_add;
	unsigned int vlan_tx_remove;
	unsigned int vlan_tx_replace;
#endif /* TBS_NFP_VLAN */

#ifdef TBS_NFP_PPP
	unsigned int pppoe_rx_not_found;
	unsigned int pppoe_rx_found;
	unsigned int pppoe_tx_add;
	unsigned int pppoe_tx_remove;
	unsigned int pppoe_tx_replace;
#endif	/* TBS_NFP_PPP */

#ifdef TBS_NFP_FIB
	unsigned int fib_hit;
#endif	/* TBS_NFP_FIB */

#ifdef TBS_NFP_CT
	unsigned int ct_hit;
    unsigned int ct_miss;
    unsigned int ct_filter;
    unsigned int ct_tcp_fin_rst;
#endif /* TBS_NFP_CT */

#ifdef TBS_NFP_NAT
	unsigned int dnat_hit;
	unsigned int snat_hit;
#endif	/* TBS_NFP_NAT */
} TBS_NFP_STATS;
#endif	/* TBS_NFP_STAT */


typedef struct tbs_nfp_skb_5tuple{
	int family;
	unsigned char *src_l3;
	unsigned char *dst_l3;

    /*低16字节为sport,高16字节为dport*/
	unsigned int  ports;
    unsigned char proto;
} TBS_NFP_SKB_5TUPLE;


/******************************************************************************
 *                                 DEBUG                                      *
 ******************************************************************************/
#if defined(CONFIG_TBS_NFP_DEBUG)
    #define TBS_NFP_DEBUG(fmt, args...) \
        if ((g_debug_level & TBS_NFP_DBG_PRINT) == TBS_NFP_DBG_PRINT) \
            TBS_NFP_COMMON_TRACE(fmt, ##args)
#else
    #define TBS_NFP_DEBUG(fmt, args...)  do { ; } while(0)
#endif

#if defined(CONFIG_TBS_NFP_DEBUG)
    #define TBS_NFP_WARN(fmt, args...)  \
        if ((g_debug_level & TBS_NFP_WARN_PRINT) == TBS_NFP_WARN_PRINT) \
            TBS_NFP_COMMON_TRACE(fmt, ##args)
#else
    #define TBS_NFP_WARN(fmt, args...)  do { ; } while(0)
#endif

#if 1
#define TBS_NFP_ERROR(fmt, args...)\
    do {  \
            printk(KERN_ERR "[%s:%4d] %20s: " fmt, strrchr(__FILE__, '/'), __LINE__, __FUNCTION__, ##args); \
    } while(0)
#else
#define TBS_NFP_ERROR(fmt, args...)
#endif

#define TBS_NFP_DEBUG_LEVEL_PRINT(level, fmt, args...)\
    do {  \
            printk(level "[%s:%4d] %20s: " fmt, strrchr(__FILE__, '/'), __LINE__, __FUNCTION__, ##args); \
    } while(0)

//#define TBS_NFP_INTO_FUNC   TBS_NFP_DEBUG("##In## %s\n", __func__)
//#define TBS_NFP_OUT_FUNC    TBS_NFP_DEBUG("##Out## %s\n", __func__)

#define TBS_NFP_INTO_FUNC  do { ; } while(0)
#define TBS_NFP_OUT_FUNC do { ; } while(0)

#define TBS_NFP_COMMON_TRACE(fmt, args...) \
    do {  \
            printk("[%s:%4d] %20s: " fmt, strrchr(__FILE__, '/'), __LINE__, __FUNCTION__, ##args); \
    } while(0)



/******************************************************************************
 *                                 TOOLS                                      *
 ******************************************************************************/


/******************************************************************************
 *                                GLOBAL                                      *
 ******************************************************************************/
/* defined in tbs_nfp.c */
extern unsigned char     g_debug_level;
extern unsigned char     g_nfp_enable;

#ifdef TBS_NFP_STAT
#define TBS_ETH_MAX_PORTS   8
extern TBS_NFP_STATS g_nfp_stats[];
#endif

#ifdef TBS_NFP_STAT
#define TBS_NFP_INC(p, s) \
	do{ \
		if(p <= TBS_ETH_MAX_PORTS)\
			g_nfp_stats[p].s++;\
	}while(0);
#else
#define TBS_NFP_INC(p, s)
#endif


/*全局读写自旋锁*/
extern rwlock_t tbs_nfp_lock;


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/
void tbs_nfp_debug_level_set(int dbg_level_flags);
void tbs_nfp_stats(unsigned int port);
void tbs_nfp_rule_reset(void);


/************
 * mng
 ************/
int tbs_nfp_mng_init(void);
void tbs_nfp_mng_exit(void);

#endif /* __TBS_NFP_H__ */
