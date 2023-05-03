/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称 : nfp_neighbour.h
 文件描述 : TBS加速器适配

 修订记录 :
          1 创建 : pengyao
            日期 : 2011-04-10
            描述 :
**********************************************************************/

#ifndef _NFP_NEIGHBOR_H_
#define _NFP_NEIGHBOR_H_


#include <linux/timer.h>
#include <asm/atomic.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/jhash.h>

#include <linux/err.h>
#include <linux/sysctl.h>
#include "nfp_adapter.h"



/*********************************************************************
 *                              GLOBAL                               *
 *********************************************************************/


/*********************************************************************
 *                              STRUCT                               *
 *********************************************************************/

/* neighbor_entry structure */
struct neighbour_entry {
    unsigned char mac_addr[6];
    unsigned int ip_addr[4];
    unsigned short ip_addr_len;
    int family;                         /*ipv4/ipv6*/
    int flags;
};

/*neighbour规则超时延后处理结构*/
struct neighbour_timeout_t {
	struct neighbour_entry *pnb;
    struct work_struct entry_timeout_work;
};

/*neighbour规则事件延后处理结构*/
struct neighbour_event_t {
	struct neighbour_entry nb;
    unsigned long event;
    struct work_struct entry_event_work;
};

/*
neighbour规则缓存状态，NB_INVALID状态代表以及无效，
但是被其他模块引用，只待释放而已
*/
enum NEIGH_STAT{
	NB_INVALID 		= 0x0,
    NB_ACTIVE       = 0x1,
};


/*********************************************************************
 *                              FUNCTIONS                            *
 *********************************************************************/
int nfp_neighbour_init(void);
int nfp_neighbour_exit(void);

void nfp_neigh_reset(void);

#endif
