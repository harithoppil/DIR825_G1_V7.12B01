
/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* �ļ����� : tbs_nfp_mcf.h
* �ļ����� : tbs�������ಥת����ض���
*
* �޶���¼ :
*          1 ���� : pengyao
*            ���� : 2012-02-20
*            ���� :
*
*******************************************************************************/

#ifndef __TBS_NFP_MCF_H__
#define __TBS_NFP_MCF_H__

#include <linux/skbuff.h>
#include <linux/if_ether.h>

#include "tbs_nfp_defs.h"
#include "tbs_nfp_common.h"
#include "tbs_nfp_itf.h"
#include "tbs_nfp_skb_parser.h"


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
#define TBS_NFP_MC_FDB_DEF  128

#define IS_CLASSD_ADDR(ipv4addr)				((((uint32)(ipv4addr)) & 0xf0000000) == 0xe0000000)
#define RESERVE_MULTICAST_ADDR1 	0xEFFFFFFA
#define IN_MULTICAST_RESV1(addr)	((((uint32)(addr)) & 0xFFFFFF00) == 0xe0000000)	// 224.0.0.x
#define IN_MULTICAST_RESV2(addr)	((((uint32)(addr)) & 0xFF000000) == 0xEF000000)	// 239.0.0.0~239.255.255.255

#define IS_IPV6_MULTICAST_ADDRESS(ipv6addr)	((ipv6addr[0] & 0xFF000000)==0xff000000)
#define IS_IPV4_MULTICAST_ADDRESS(ipv4addr)	(IS_CLASSD_ADDR(ipv4addr[0]))

#define TBS_NFP_MAXVIFS 32
#define VIFF_AF_INET    0xFF01          /* for ipv4 multicast forwarding*/
#define VIFF_AF_INET6   0xFF02          /* for ipv6 multicast forwarding*/

/******************************************************************************
 *                                 ENUM                                       *
 ******************************************************************************/
/* ������ӿ�index�ͽ��յ�igmp����ttl*/
enum mfc_vif_info
{
    VIF_TTL,
    VIF_INDEX,
    VIF_INFO_MAX
};


/******************************************************************************
 *                                 STRUCT                                     *
 ******************************************************************************/
#ifdef TBS_NFP_IGMP_SNOOPING
typedef struct tbs_nfp_mc_fdb {
    struct list_head    list;

    int                 family;
	unsigned int 		br_port;
    #if 0
	unsigned char		mc_addr[ETH_ALEN];
	unsigned char		host_addr[ETH_ALEN];
    #endif
    unsigned char       src[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned char       group[TBS_NFP_MAX_L3_ADDR_SIZE];
    int			        filt_mode;

    unsigned short      orig_vid;
    unsigned short      new_vid;

    unsigned long bytes;
    unsigned long pkt;

} ST_TBS_NFP_MC_FDB;
#endif  /*TBS_NFP_SNOOP*/


typedef struct tbs_nfp_mfc_cache
{
	struct list_head    list;			/* Next entry on cache line 	*/

    char family;                        /* AF_INET or AF_INET6 */
	unsigned char mfc_mcastgrp[TBS_NFP_MAX_L3_ADDR_SIZE];	    /* Group the entry belongs to 	*/
	unsigned char mfc_origin[TBS_NFP_MAX_L3_ADDR_SIZE];			/* Source of packet 		*/

#ifdef CONFIG_IGMP_PROXY_MULTIWAN
    int mfc_parent_num;
    int mfc_parent_other[TBS_NFP_MAXVIFS];           /* Source interface     */
#else
    int mfc_parent;			            /* Source interface	index*/
#endif

	int mfc_flags;				        /* Flags on line		*/
    int vif_num;
	unsigned char vif_info[VIF_INFO_MAX][TBS_NFP_MAXVIFS];	/* TTL thresholds, vif ttl*/

	unsigned long last_assert;
	unsigned long bytes;
	unsigned long pkt;
	unsigned long wrong_if;
} ST_TBS_NFP_MCF;


/* ipv4/6����vif���飬ͨ��flags����*/
typedef struct tbs_nfp_vif
{
	TBS_NFP_IF_MAP *ifmp;			    /* Device we are using */
	unsigned long	bytes_in,bytes_out;
	unsigned long	pkt_in,pkt_out;		/* Statistics 			*/
	unsigned long	rate_limit;		    /* Traffic shaping (NI) 	*/
	unsigned char	threshold;		    /* TTL threshold 		*/
	unsigned short	flags;			    /* Control flags 		*/
	__be32		    local,remote;		/* Addresses(remote for tunnels), only used for ipv4*/
	int		        link;			    /* Physical interface index	*/
} ST_TBS_NFP_VIF;



/******************************************************************************
 *                                 DEBUG                                      *
 ******************************************************************************/



/******************************************************************************
 *                                 TOOLS                                      *
 ******************************************************************************/


/******************************************************************************
 *                                GLOBAL                                      *
 ******************************************************************************/
extern struct tbs_nfp_vif *nfp_vif[TBS_NFP_MAXVIFS];


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/
#ifdef TBS_NFP_IGMP_SNOOPING
/*=========================================================================
 Function:      int tbs_nfp_mc_fdb_add(int family, unsigned int br_port, unsigned char *src,
    unsigned char *group, int filt_mode
#if defined(TBS_NFP_IGMP_VLAN)
    , unsigned short      orig_vlan, unsigned short      new_vlan
#endif)

 Description:       mc fdb������Ӻ���
 Data Accessed:     g_mc_fdb_hash
 Data Updated:
 Input:             unsigned int br_port    �Ŷ˿�
                    unsigned char *src      �鲥Դ��ַ
                    unsigned char *group    �鲥���ַ
                    int filt_mode           filterģʽ
                    unsigned short      orig_vlan/new_vlan, �鲥Դ/�㲥��vlanid
 Output:            ��
 Return:            0   �ɹ�������  ʧ��
 Others:
=========================================================================*/
int tbs_nfp_mc_fdb_add(int family, unsigned int br_port, unsigned char *src,
    unsigned char *group, int filt_mode
#if defined(TBS_NFP_IGMP_VLAN)
    , unsigned short      orig_vlan, unsigned short      new_vlan
#endif  /* TBS_NFP_IGMP_VLAN */
);


/*=========================================================================
 Function:      int tbs_nfp_mc_fdb_delete(int family, unsigned int br_port, unsigned char *src,
    unsigned char *group)

 Description:       mc fdb����ɾ������
 Data Accessed:     g_mc_fdb_hash
 Data Updated:
 Input:             unsigned int br_port    �Ŷ˿�
                    unsigned char *src      �鲥Դ��ַ
                    unsigned char *group    �鲥���ַ
 Output:            ��
 Return:            0   �ɹ�������  ʧ��
 Others:
=========================================================================*/
int tbs_nfp_mc_fdb_delete(int family, unsigned int br_port, unsigned char *src,
    unsigned char *group
#if defined(TBS_NFP_IGMP_VLAN)
            , unsigned short      orig_vlan, unsigned short      new_vlan
#endif  /* TBS_NFP_IGMP_VLAN */
);


/*=========================================================================
 Function:      TBS_NFP_MC_FDB *tbs_nfp_mc_fdb_lookup(int family, unsigned char *src, unsigned char *group
#if defined(TBS_NFP_IGMP_VLAN)
        , unsigned short      orig_vlan, unsigned short      new_vlan
#endif)

 Description:       mc fdb������Һ���
 Data Accessed:     g_mc_fdb_hash
 Data Updated:
 Input:             unsigned char *src      �鲥Դ��ַ
                    unsigned char *group    �鲥���ַ
                    unsigned short      orig_vlan/new_vlan, �鲥Դ/�㲥��vlanid

 Output:            ��
 Return:            mc fdb����ָ��
 Others:
=========================================================================*/
ST_TBS_NFP_MC_FDB *tbs_nfp_mc_fdb_lookup(int family, unsigned char *src, unsigned char *group
#if defined(TBS_NFP_IGMP_VLAN)
        , unsigned short      orig_vlan
#endif  /* TBS_NFP_IGMP_VLAN */
    );


/*=========================================================================
 Function:      void tbs_nfp_mc_fdb_dump(void)

 Description:       mc fdb�����ӡ
 Data Accessed:     g_mc_fdb_hash
 Data Updated:
 Input:             ��

 Output:            ��
 Return:            ��
 Others:
=========================================================================*/
void tbs_nfp_mc_fdb_dump(void);


/*=========================================================================
 Function:      void tbs_nfp_mc_fdb_reset(void)

 Description:       mc fdb�������
 Data Accessed:     g_mc_fdb_hash
 Data Updated:
 Input:             ��

 Output:            ��
 Return:            ��
 Others:
=========================================================================*/
void tbs_nfp_mc_fdb_reset(void);
#endif /* TBS_NFP_IGMP_SNOOPING */


/*=========================================================================
 Function:      int tbs_nfp_vif_add(int family, int ifindex, unsigned rate_limit,
    unsigned char threshold, unsigned char flags)

 Description:       ������ӿ�
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:             int family                  Э����
                    int ifindex                 dev ifindex
                    unsigned rate_limit         rate_limit
                    unsigned char threshold     ttl
                    unsigned char flags         control flags
 Output:            0   �ɹ���  ����    ʧ��
 Return:            ��
 Others:
=========================================================================*/
int tbs_nfp_vif_add(int family, int ifindex, unsigned rate_limit, unsigned char threshold,
    unsigned char flags);


/*=========================================================================
 Function:      int tbs_nfp_vif_delete(int family, int ifindex)

 Description:       ɾ����ӿ�
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:             int family                  Э����
                    int ifindex                 dev ifindex
 Output:            0   �ɹ���  ����    ʧ��
 Return:            ��
 Others:
=========================================================================*/
int tbs_nfp_vif_delete(int family, int ifindex);


/*=========================================================================
 Function:      int tbs_nfp_mcf_add(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int ivif, ing flags, int vif_count, unsigned char *vif_info[VIF_INFO_MAX][TBS_NFP_MAXVIFS])

 Description:       mcf ������Ӻ���
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:             unsigned char *mc_group     �鲥���ַ
                    unsigned char *mc_origin    �鲥Դ��ַ
                    int ivif                    �鲥��ӿ�
                    ing flags                   mcf����
                    int vif_count               �鲥�ַ��ӿ���
                    unsigned char *vif_info     �鲥�ַ��ӿ���Ϣ

 Output:            0   �ɹ���  ����    ʧ��
 Return:            ��
 Others:
=========================================================================*/
int tbs_nfp_mcf_add(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int in_vif, ing flags, int vif_count, unsigned char *vif_info[VIF_INFO_MAX][TBS_NFP_MAXVIFS]);


/*=========================================================================
 Function:      int tbs_nfp_mcf_delete(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int ivif)

 Description:       mcf ����ɾ������
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:             unsigned char *mc_group     �鲥���ַ
                    unsigned char *mc_origin    �鲥Դ��ַ
                    int ivif                    �鲥��ӿ�

 Output:            0   �ɹ���  ����    ʧ��
 Return:            ��
 Others:
=========================================================================*/
int tbs_nfp_mcf_delete(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int ivif);

/*=========================================================================
 Function:      int tbs_nfp_mcf_delete(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int ivif)

 Description:       mcf �����������
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:             unsigned char *mc_group     �鲥���ַ
                    unsigned char *mc_origin    �鲥Դ��ַ
                    int ivif                    �鲥��ӿ�

 Output:            mcf ����
 Return:            ��
 Others:
=========================================================================*/
ST_TBS_NFP_MCF *tbs_nfp_mcf_lookup(int family, unsigned char *mc_group, unsigned char *mc_origin,
    int ivif);


/*=========================================================================
 Function:      void tbs_nfp_mcf_dump(void)

 Description:       mcf �����ӡ����
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:             ��
 Output:            ��
 Return:            ��
 Others:
=========================================================================*/
void tbs_nfp_mcf_dump(void);


/*=========================================================================
 Function:      void tbs_nfp_mcf_reset(void)

 Description:       mcf �����������
 Data Accessed:     g_mcf_hash
 Data Updated:
 Input:             ��
 Output:            ��
 Return:            ��
 Others:
=========================================================================*/
void tbs_nfp_mcf_reset(void);


/*=========================================================================
 Function:      void tbs_nfp_mcf_init(void)

 Description:       mcf��ģ���ʼ������
 Data Accessed:     g_mcf_cache_hash/g_mc_fdb_hash
 Data Updated:
 Input:             ��Ԫ����Ϣ
 Output:            ��
 Return:            ��
 Others:
=========================================================================*/
int tbs_nfp_mcf_init(void);


/*=========================================================================
 Function:      void tbs_nfp_mcf_exit(void)

 Description:       mcf��ģ���˳�����
 Data Accessed:     g_mcf_cache_hash/g_mc_fdb_hash
 Data Updated:
 Input:             ��Ԫ����Ϣ
 Output:            ��
 Return:            ��
 Others:
=========================================================================*/
void tbs_nfp_mcf_exit(void);



#endif /* __TBS_NFP_MCF_H__ */
