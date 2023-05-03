/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_proc.h
* 文件描述 : tbs加速器proc配置调试接口
*
* 修订记录 :
*          1 创建 : pengyao
*            日期 : 2011-12-03
*            描述 :
*
*******************************************************************************/

#ifndef __TBS_NFP_PROC_H__
#define __TBS_NFP_PROC_H__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>

#include "tbs_nfp.h"
#include "../nfp_adapter/nfp_interface.h"

#ifdef TBS_NFP_VLAN
#include <../net/8021q/vlan.h>
#endif /*TBS_NFP_VLAN*/

#include "tbs_nfp_itf.h"

#ifdef TBS_NFP_BRIDGE
#include "tbs_nfp_bridge.h"
#endif

#ifdef TBS_NFP_FIB
#include "tbs_nfp_fib.h"
#endif

#ifdef TBS_NFP_CT
#include "tbs_nfp_ct.h"
#endif


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
#define TBS_NFP_DIR                 "tbs_nfp"
#define TBS_NFP_IF_DIR              "interface"
#define TBS_NFP_BRIDGE_DIR          "bridge"
#define TBS_NFP_ARP_DIR             "arp"
#define TBS_NFP_FIB_DIR             "fib"
#define TBS_NFP_CT_DIR              "ct"

#define TBS_NFP_ENABLE              "enable"
#define TBS_NFP_STATUS              "status"
#define TBS_NFP_HELP                "help"
#define TBS_NFP_DEBUG_LEVEL         "debug_level"
#define TBS_NFP_RULE_RESET          "reset"
#define TBS_NFP_RULE_DUMP           "dump"
#define TBS_NFP_RULE_ADD            "rule_add"
#define TBS_NFP_RULE_DELETE         "rule_delete"
#define TBS_NFP_RULE_SIZE           "max_size"


/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
extern struct proc_dir_entry *g_tbs_nfp_proc_dir;
extern const char* g_common_proc[];


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

int tbs_nfp_proc_remove_entry(const char *proc_array[],
    struct proc_dir_entry *parent_dir);
bool tbs_mac_str_to_hex(const char *mac_str, u8 *mac_hex);

int tbs_nfp_proc_init(void);
void tbs_nfp_proc_exit(void);

#endif /*__TBS_NFP_PROC_H__*/
