
/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp.h
* 文件描述 : tbs加速器interface头文件
*
* 修订记录 :
*          1 创建 : pengyao
*            日期 : 2011-12-06
*            描述 :
*
*******************************************************************************/

#ifndef __TBS_NFP_ITF_H__
#define __TBS_NFP_ITF_H__

#include "tbs_nfp_defs.h"
#include "tbs_nfp_common.h"
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/


/******************************************************************************
 *                                 ENUM                                       *
 ******************************************************************************/

/******************************************************************************
 *                                 STRUCT                                     *
 ******************************************************************************/
//时间戳
/*__net_timestamp*/


/*****************
 * Interface Map
 *****************/
typedef struct tbs_nfp_if_map_st {
	struct tbs_nfp_if_map_st *next_map;
	struct tbs_nfp_if_map_st *parent_if;
	struct tbs_nfp_if_map_st *virt_if;
	struct tbs_nfp_if_map_st *virt_next;
	int     ifidx;
	char    name[IFNAMSIZ];
	void    *dev;
	unsigned char   mac[ETH_ALEN];
	int     mtu;
	int     bridge_if;
	unsigned int  flags;

#ifdef TBS_NFP_VLAN
    unsigned short  vlanid;
#endif

#ifdef TBS_NFP_PPP
	unsigned short  session_id;
	unsigned char   remote_mac[ETH_ALEN];
#endif /* TBS_NFP_PPP */
} TBS_NFP_IF_MAP;



/******************************************************************************
 *                                 DEBUG                                      *
 ******************************************************************************/
    /* Enable / Disable TBS_NFP Debug Prints: */
#define TBS_NFP_ITF_DBG(x...)  if (g_debug_level & TBS_NFP_DBG_PRINT) printk(x)
    /*#define TBS_NFP_ITF_WARN(x...)*/

#define TBS_NFP_ITF_WARN(x...)  if (g_debug_level & TBS_NFP_WARN_PRINT) printk(x)
    /*#define TBS_NFP_ITF_WARN(x...)*/


#if 0
    #define TBS_NFP_ITF_DEBUG(fmt, args...) TBS_NFP_DEBUG(fmt, ##args)
#else
    #define TBS_NFP_ITF_DEBUG(fmt, args...)  do { ; } while(0)
#endif


#define TBS_NFP_ITF_DEBUG_LEVEL(level, fmt, args...)\
        do {  \
                printk(level "[%s:%4d] %20s: " fmt, strrchr(__FILE__, '/'), __LINE__, __FUNCTION__, ##args); \
        } while(0)

#define TBS_NFP_ITF_INTO_FUNC  TBS_NFP_ITF_DEBUG("##In## %s\n", __func__) //do { ; } while(0)
#define TBS_NFP_ITF_OUT_FUNC TBS_NFP_ITF_DEBUG("##Out## %s\n", __func__) //do { ; } while(0)

//#define TBS_NFP_ITF_INTO_FUNC  do { ; } while(0)
//#define TBS_NFP_ITF_OUT_FUNC do { ; } while(0)


/******************************************************************************
 *                                 TOOLS                                      *
 ******************************************************************************/


/******************************************************************************
 *                                GLOBAL                                      *
 ******************************************************************************/
extern TBS_NFP_IF_MAP *g_if_map[];


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/************
 * interface
 ************/
#ifdef TBS_NFP_WLAN
int tbs_nfp_if_wlan_add(int if_idx, int mtu,
    const char* mac, const char *if_name);
#endif

#ifdef TBS_NFP_BRIDGE
int tbs_nfp_if_bridge_add(int if_idx, int mtu, const char* mac,
    const char *if_name);
#endif

#ifdef TBS_NFP_VLAN
int tbs_nfp_if_vlan_add(int if_idx, int mtu, int phys_if_idx,
    unsigned short vlan_id, const char *mac, const char *if_name);
TBS_NFP_IF_MAP *tbs_nfp_vlan_ifmap_get(int parent_if_idx, unsigned short vlan_id);
#endif

#ifdef TBS_NFP_PPP
int tbs_nfp_if_pppoe_add(int if_idx, int phys_if_idx, int mtu, unsigned short session_id,
    const char* remot_mac, const char *if_name);
TBS_NFP_IF_MAP *tbs_nfp_pppoe_ifmap_get(int port, const TBS_NFP_IF_MAP *in_ifmap, unsigned short sess_id);
#endif

#ifdef TBS_NFP_DUMMYPORT
int tbs_nfp_if_dummyport_add(int if_idx, int mtu, int phys_if_idx,
    const char* mac, const char *if_name);
int tbs_nfp_if_dummyport_bind(int if_idx, int phys_if_idx);
int tbs_nfp_if_dummyport_unbind(int if_idx);
#endif

#ifdef TBS_NFP_BRIDGE
int tbs_nfp_if_br_port_add(int br_idx, int br_port_idx);
int tbs_nfp_if_br_port_remove(int br_port_idx);
#endif

#if defined(CONFIG_VNET)
int tbs_nfp_if_vnet_add(int if_idx, int mtu, int phys_if_idx,
    const char* mac, const char *if_name);
#endif

int tbs_nfp_if_phys_add(int if_idx, int mtu, const char* mac,
    const char *if_name);
int tbs_nfp_if_delete(int if_idx);
int tbs_nfp_if_state(int if_idx, unsigned int state);
int tbs_nfp_if_mac_change(int if_idx, const char *mac);
int tbs_nfp_if_mtu_change(int if_idx, int mtu);
int tbs_nfp_if_reset(void);
void tbs_nfp_if_print(const TBS_NFP_IF_MAP *if_map);
void tbs_nfp_if_dump(void);
void tbs_nfp_if_exit(void);
int tbs_nfp_if_init(void);
TBS_NFP_IF_MAP *tbs_nfp_ifmap_get(int if_idx);
TBS_NFP_IF_MAP *tbs_nfp_if_phys_get(TBS_NFP_IF_MAP *virt_node);

#endif /* __TBS_NFP_ITF_H__ */
