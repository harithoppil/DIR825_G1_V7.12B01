
/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* �ļ����� : tbs_nfp_ct.h
* �ļ����� : tbs������conntrackͷ�ļ�
*
* �޶���¼ :
*          1 ���� : pengyao
*            ���� : 2011-12-06
*            ���� :
*
*******************************************************************************/

#ifndef __TBS_NFP_CT_H__
#define __TBS_NFP_CT_H__

#include "tbs_nfp_defs.h"
#include "tbs_nfp_common.h"
#include "tbs_nfp_fib.h"
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
#define TBS_NFP_CT_RULE_DEF 2048


/******************************************************************************
 *                                 ENUM                                       *
 ******************************************************************************/
/*����5Ԫ�鷽��*/
enum tbs_nfp_ct_dir
{
    CT_DIR_ORIGINAL,
    CT_DIR_REPLY,
    CT_DIR_MAX
};
#define CT_NEXT_DIR(dir)  ((dir)^CT_DIR_REPLY)


/******************************************************************************
 *                                 STRUCT                                     *
 ******************************************************************************/
//ʱ���
/*__net_timestamp*/

typedef struct {
    int  family;
    int  ip_offset;
    int  ip_hdrLen;
    unsigned short ipLen;
    union {
        struct iphdr  *ip4;
        struct ipv6hdr *ip6;
    } ip_hdr;
} TBS_IP_HEADER_INFO;


#if 0
typedef struct nfp_rule_ct {
    /*g_ct_hash��g_ct_inv_hash�����ڵ�*/
	struct list_head list;
    /*g_ct_hash_by_fib��g_ct_inv_hash_by_fib�����ڵ�, ����src_ip��dst_ip����hash*/
    struct list_head list_by_fib;

	/* 5 tuple key */
	int family;
	unsigned char src_l3[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned char dst_l3[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned int ports;
	unsigned short proto;
	unsigned short reserved;	/* for alignment */
	TBS_NFP_RULE_FIB *fib;

#ifdef TBS_NFP_NAT
	unsigned int new_sip;
	unsigned int new_dip;
	unsigned short new_sport;
	unsigned short new_dport;
#endif /* TBS_NFP_NAT */

	unsigned int flags;
	unsigned int count;
} TBS_NFP_RULE_CT;
#endif


typedef struct tbs_nfp_tuple_hash {
    struct list_head list;          /*hash by 5tuple*/
    struct list_head list_by_fib;   /*hash by gtw*/

	/* 5 tuple key, 3 */
	unsigned char src_l3[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned char dst_l3[TBS_NFP_MAX_L3_ADDR_SIZE];
//	unsigned short sport;
//  unsigned short dport;
 	unsigned int ports;
    unsigned char dir;          /* CT_DIR_ORIGINAL/CT_DIR_REPLY */
    unsigned char flags;        /* nat & invalid flags*/

	TBS_NFP_RULE_FIB *fib;
    unsigned short reserved;	/* for alignment */
}ST_TUPLE_HASH;


typedef struct tbs_nfp_tuple_ct {
    /* ˫����Ԫ�� */
    struct tbs_nfp_tuple_hash tuplehash[CT_DIR_MAX];

	/* 5 tuple key, 2 */
	int family;
	unsigned char proto;

    /* ת����ͳ�� */
	unsigned int count;
} TBS_NFP_TUPLE_CT;



/******************************************************************************
 *                                 DEBUG                                      *
 ******************************************************************************/
    /* Enable / Disable TBS_NFP Debug Prints: */
#define TBS_NFP_CT_DBG(x...)  if (g_debug_level & TBS_NFP_DBG_PRINT) printk(x)
    /*#define TBS_NFP_CT_WARN(x...)*/

#define TBS_NFP_CT_WARN(x...)  if (g_debug_level & TBS_NFP_WARN_PRINT) printk(x)
    /*#define TBS_NFP_CT_WARN(x...)*/


#if 0
    #define TBS_NFP_CT_DEBUG(fmt, args...) TBS_NFP_DEBUG(fmt, ##args)
#else
    #define TBS_NFP_CT_DEBUG(fmt, args...)  do { ; } while(0)
#endif


#define TBS_NFP_CT_DEBUG_LEVEL(level, fmt, args...)\
        do {  \
                printk(level "[%s:%4d] %20s: " fmt, strrchr(__FILE__, '/'), __LINE__, __FUNCTION__, ##args); \
        } while(0)

#define TBS_NFP_CT_INTO_FUNC  TBS_NFP_CT_DEBUG("##In## %s\n", __func__) //do { ; } while(0)
#define TBS_NFP_CT_OUT_FUNC TBS_NFP_CT_DEBUG("##Out## %s\n", __func__) //do { ; } while(0)

//#define TBS_NFP_CT_INTO_FUNC  do { ; } while(0)
//#define TBS_NFP_CT_OUT_FUNC do { ; } while(0)



/******************************************************************************
 *                                 TOOLS                                      *
 ******************************************************************************/


/******************************************************************************
 *                                GLOBAL                                      *
 ******************************************************************************/
extern struct list_head g_ct_hash[];
extern struct list_head g_ct_hash_by_fib[];
extern struct list_head g_ct_inv_hash[];
extern struct list_head g_ct_inv_hash_by_fib[];


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

extern void tbs_nfp_ct_dump(void);
ST_TUPLE_HASH *tbs_nfp_ct_lookup_by_tuple(int family,
    const unsigned char *sip, const unsigned char *dip, unsigned int ports,
    unsigned char proto, bool valid);
extern void tbs_nfp_ct_fib_invalid(const TBS_NFP_RULE_FIB *fib);
extern void tbs_nfp_ct_fib_update(TBS_NFP_RULE_FIB *fib);
extern void tbs_nfp_ct_reset(void);
extern int tbs_nfp_ct_add(int family, const unsigned char *sip, const unsigned char *dip,
		unsigned short sport, unsigned short dport, unsigned char proto,
		const unsigned char *reply_sip, const unsigned char *reply_dip,
		unsigned short reply_sport, unsigned short reply_dport);
extern int tbs_nfp_ct_delete(int family, const unsigned char *sip, const unsigned char *dip,
               unsigned short sport, unsigned short dport, unsigned char proco);
extern unsigned int tbs_nfp_ct_age(int family, const unsigned char *sip, const unsigned char *dip,
               unsigned short sport, unsigned short dport, unsigned char proto);
void tbs_nfp_ct_exit(void);
int tbs_nfp_ct_init(void);
/*=========================================================================
 Function:      static inline ST_TUPLE_HASH *GET_NEXT_DIR_TUPLE(ST_TUPLE_HASH *this_tuple)
>>>>>>> .r18747

 Description:       ����һ�������5 tuple��ȡ��һ�������5 tupleָ��
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:             ����5 tuple
 Output:            ��
 Return:            ����5 tupleָ��
 Others:            �ر�ע�⣬this_tuple������tuplehash[CT_DIR_MAX]�е�һ��������ᵼ��Խ��
=========================================================================*/
static inline ST_TUPLE_HASH *GET_NEXT_DIR_TUPLE(ST_TUPLE_HASH *this_tuple)
{
    TBS_NFP_TUPLE_CT *ct = NULL;

    if(this_tuple->dir >= CT_DIR_REPLY)
    {
        ct = container_of(this_tuple, TBS_NFP_TUPLE_CT, tuplehash[CT_DIR_REPLY]);
        return &ct->tuplehash[CT_DIR_ORIGINAL];
    }

    ct = container_of(this_tuple, TBS_NFP_TUPLE_CT, tuplehash[CT_DIR_ORIGINAL]);
    return &ct->tuplehash[CT_DIR_REPLY];
}


/*=========================================================================
 Function:      static inline ST_TUPLE_HASH *GET_NEXT_DIR_TUPLE(ST_TUPLE_HASH *this_tuple)

 Description:       ����һ�������5 tuple��ȡ����TBS_NFP_TUPLE_CT�����ָ��
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:             ����5 tuple
 Output:            ��
 Return:            TBS_NFP_TUPLE_CT����ָ��
 Others:            �ر�ע�⣬this_tuple������tuplehash[CT_DIR_MAX]�е�һ��������ᵼ��Խ��
=========================================================================*/
static inline TBS_NFP_TUPLE_CT *GET_TUPLE_CT(const ST_TUPLE_HASH *this_tuple)
{
    if(this_tuple->dir >= CT_DIR_REPLY)
    {
        return container_of(this_tuple, TBS_NFP_TUPLE_CT, tuplehash[CT_DIR_REPLY]);
    }

    return container_of(this_tuple, TBS_NFP_TUPLE_CT, tuplehash[CT_DIR_ORIGINAL]);
}

#ifdef TBS_NFP_NAT
#endif /* TBS_NFP_NAT */


#endif /* __TBS_NFP_CT_H__ */
