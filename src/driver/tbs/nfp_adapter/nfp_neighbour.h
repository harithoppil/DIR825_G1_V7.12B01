/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 �ļ����� : nfp_neighbour.h
 �ļ����� : TBS����������

 �޶���¼ :
          1 ���� : pengyao
            ���� : 2011-04-10
            ���� :
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

/*neighbour����ʱ�Ӻ���ṹ*/
struct neighbour_timeout_t {
	struct neighbour_entry *pnb;
    struct work_struct entry_timeout_work;
};

/*neighbour�����¼��Ӻ���ṹ*/
struct neighbour_event_t {
	struct neighbour_entry nb;
    unsigned long event;
    struct work_struct entry_event_work;
};

/*
neighbour���򻺴�״̬��NB_INVALID״̬�����Լ���Ч��
���Ǳ�����ģ�����ã�ֻ���ͷŶ���
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
