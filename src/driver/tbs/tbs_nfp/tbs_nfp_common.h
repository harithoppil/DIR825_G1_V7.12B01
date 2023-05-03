/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* �ļ����� : tbs_nfp_common.h
* �ļ����� : tbs�������������ݶ���
*
* �޶���¼ :
*          1 ���� : pengyao
*            ���� : 2011-11-10
*            ���� :
*
*******************************************************************************/

#ifndef __TBS_NFP_COMMON_H__
#define __TBS_NFP_COMMON_H__

#include <linux/timer.h>
#include <asm/atomic.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/jhash.h>

#include <linux/err.h>
#include <linux/sysctl.h>
#include <linux/socket.h>
#include <linux/if_ether.h>


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
#define	TBS_NFP_ARRAY_SIZE(x)	( sizeof(x) / sizeof( (x)[0]) )

#define TBS_NFP_INET_ADDRSTRLEN     40
#define TBS_NFP_MAC_ADDRSTRLEN      18

/* IPv4: 4, IPv6: 16 */
#define TBS_NFP_MAX_IPV4_ADDR_SIZE	(4)

#ifdef CONFIG_IPV6
#define TBS_NFP_MAX_L3_ADDR_SIZE	(16)
#else
#define TBS_NFP_MAX_L3_ADDR_SIZE	(4)
#endif

/*����hashͰ��С*/
#define TBS_NFP_FIB_HASH_BIT            8
#define TBS_NFP_FIB_HASH_SIZE           (1 << TBS_NFP_FIB_HASH_BIT)
#define TBS_NFP_FIB_HASH_MASK           (TBS_NFP_FIB_HASH_SIZE - 1)

#define TBS_NFP_FIB_HASH_BY_GTW_SIZE    TBS_NFP_FIB_HASH_SIZE
#define TBS_NFP_FIB_HASH_BY_GTW_MASK    TBS_NFP_FIB_HASH_MASK


#define TBS_NFP_FIB_INV_HASH_BIT        6
#define TBS_NFP_FIB_INV_HASH_SIZE       (1 << TBS_NFP_FIB_INV_HASH_BIT)
#define TBS_NFP_FIB_INV_HASH_MASK       (TBS_NFP_FIB_INV_HASH_SIZE - 1)

#define TBS_NFP_FIB_INV_HASH_BY_GTW_SIZE   TBS_NFP_FIB_INV_HASH_SIZE
#define TBS_NFP_FIB_INV_HASH_BY_GTW_MASK   TBS_NFP_FIB_INV_HASH_MASK

#define TBS_NFP_CT_HASH_BIT             8
#define TBS_NFP_CT_HASH_SIZE            (1 << TBS_NFP_CT_HASH_BIT)
#define TBS_NFP_CT_HASH_MASK            (TBS_NFP_CT_HASH_SIZE - 1)

#define TBS_NFP_CT_HASH_BY_FIB_SIZE        TBS_NFP_CT_HASH_SIZE
#define TBS_NFP_CT_HASH_BY_FIB_MASK        TBS_NFP_CT_HASH_MASK

#define TBS_NFP_CT_INV_HASH_BIT         6
#define TBS_NFP_CT_INV_HASH_SIZE        (1 << TBS_NFP_CT_INV_HASH_BIT)
#define TBS_NFP_CT_INV_HASH_MASK        (TBS_NFP_CT_INV_HASH_SIZE - 1)

#define TBS_NFP_CT_INV_HASH_BY_FIB_SIZE    TBS_NFP_CT_INV_HASH_SIZE
#define TBS_NFP_CT_INV_HASH_BY_FIB_MASK    TBS_NFP_CT_INV_HASH_MASK

#define TBS_NFP_ITF_HASH_BIT	        6
#define TBS_NFP_ITF_HASH_SIZE           (1 << TBS_NFP_ITF_HASH_BIT)
#define TBS_NFP_ITF_HASH_MASK           (TBS_NFP_ITF_HASH_SIZE - 1)

#define TBS_NFP_ARP_HASH_BIT            8
#define TBS_NFP_ARP_HASH_SIZE           (1 << TBS_NFP_ARP_HASH_BIT)
#define TBS_NFP_ARP_HASH_MASK           (TBS_NFP_ARP_HASH_SIZE - 1)

#define TBS_NFP_FDB_HASH_BIT            6
#define TBS_NFP_FDB_HASH_SIZE           (1 << TBS_NFP_FDB_HASH_BIT)
#define TBS_NFP_FDB_HASH_MASK           (TBS_NFP_FDB_HASH_SIZE - 1)

#define TBS_NFP_BRIDGE_HASH_BIT         6
#define TBS_NFP_BRIDGE_HASH_SIZE        (1 << TBS_NFP_BRIDGE_HASH_BIT)
#define TBS_NFP_BRIDGE_HASH_MASK        (TBS_NFP_BRIDGE_HASH_SIZE - 1)

#ifdef TBS_NFP_MCF
#define TBS_NFP_MCF_HASH_BIT            3
#define TBS_NFP_MCF_HASH_SIZE           (1 << TBS_NFP_MCF_HASH_BIT)
#define TBS_NFP_MCF_HASH_MASK           (TBS_NFP_MCF_HASH_SIZE - 1)

#ifdef TBS_NFP_IGMP_SNOOPING
#define TBS_NFP_MC_FDB_HASH_BIT         3
#define TBS_NFP_MC_FDB_HASH_SIZE        (1 << TBS_NFP_MC_FDB_HASH_BIT)
#define TBS_NFP_MC_FDB_HASH_MASK        (TBS_NFP_MC_FDB_HASH_SIZE - 1)
#endif  /* TBS_NFP_IGMP_SNOOPING */

#endif  /* TBS_NFP_MCF */

/******************************************************************************
 *                                 ENUM                                       *
 ******************************************************************************/


/******************************************************************************
 *                                 TOOLS                                      *
 ******************************************************************************/


/******************************************************************************
 *                                GLOBAL                                      *
 ******************************************************************************/
extern unsigned int      g_jhash_iv;



/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:      static inline unsigned int tbs_nfp_fib_hash(int family, const unsigned int *saddr,
                                    const unsigned int *daddr, bool valid)

 Description:       ͨ��family��saddr��daddr����hash��֧��ipv4��ipv6
 Data Accessed:

 Data Updated:
 Input:             int family      Э����
                    const unsigned int *saddr �����ֽ���Դip��ַ
                    const unsigned int *daddr �����ֽ���Ŀ��ip��ַ
                    valid: ����valid����invalid HASH���key
 Output:            ��
 Return:            hash_key

 Others:
=========================================================================*/
static inline unsigned int tbs_nfp_fib_hash(int family, const unsigned int *saddr,
                                    const unsigned int *reply_saddr, bool valid)
{
    unsigned int key1, key2, mask = TBS_NFP_FIB_INV_HASH_MASK;

    if(valid)
        mask = TBS_NFP_FIB_HASH_MASK;

    if (family == AF_INET6)
    {
        key1 = saddr[0] ^ saddr[1] ^ saddr[2] ^ saddr[3];
        key2 = reply_saddr[0] ^ reply_saddr[1] ^ reply_saddr[2] ^ reply_saddr[3];
        return (jhash_2words(key1, key2, g_jhash_iv) & mask);
    }
    else
    {
        key1 = saddr[0];
        key2 = reply_saddr[0];
        return (jhash_2words(key1, key2, g_jhash_iv) & mask);
    }
}


/*=========================================================================
 Function:      static inline unsigned int tbs_nfp_hash_fib_gtw(int family, const unsigned int *gw_ipaddr,
                            bool valid)

 Description:       ͨ��family��gtw_addr����hash��֧��ipv4��ipv6
 Data Accessed:

 Data Updated:
 Input:             int family      Э����AF_INET/AF_INET6
                    const unsigned int *ipaddr �����ֽ���ip��ַ
                    valid: ����valid����invalid HASH���key
 Output:            ��
 Return:            hash_key

 Others:
=========================================================================*/
static inline unsigned int tbs_nfp_fib_hash_gtw(int family, const unsigned int *gw_ipaddr,
                            bool valid)
{
    unsigned int key, mask = TBS_NFP_FIB_INV_HASH_BY_GTW_MASK;

    if(valid)
        mask = TBS_NFP_FIB_HASH_BY_GTW_MASK;

    if (family == AF_INET6)
    {
        key = gw_ipaddr[0] ^ gw_ipaddr[1] ^ gw_ipaddr[2] ^ gw_ipaddr[3];
        return (jhash_1word(key, g_jhash_iv) & mask);
    }
    else
    {
        key = gw_ipaddr[0];
        return (jhash_1word(key, g_jhash_iv) & mask);
    }
}


/*=========================================================================
 Function:		static inline unsigned int tbs_nfp_arp_hash(int family, const unsigned int *ipaddr)

 Description:		ͨ��family��ipaddr����hash��֧��ipv4��ipv6
 Data Accessed:

 Data Updated:
 Input:			    int family      Э����AF_INET/AF_INET6
                    const unsigned int *ipaddr �����ֽ���ip��ַ
 Output:			��
 Return:			hash_key

 Others:
=========================================================================*/
static inline unsigned int tbs_nfp_arp_hash(int family, const unsigned int *ipaddr)
{
	unsigned int key;
	if (family == AF_INET6)
	{
		key = ipaddr[0] ^ ipaddr[1] ^ ipaddr[2] ^ ipaddr[3];
		return (jhash_1word(key, g_jhash_iv) & TBS_NFP_ARP_HASH_MASK);
	}
	else
	{
		key = ipaddr[0];
		return (jhash_1word(key, g_jhash_iv) & TBS_NFP_ARP_HASH_MASK);
	}
}



/*=========================================================================
 Function:		static inline unsigned int tbs_nfp_ct_hash(unsigned short family, const void *saddr,
            const void *daddr, unsigned short sport, unsigned short dport,
            unsigned char proto, bool valid)
 Description:		����5 Ԫ��ȡhash
 Data Accessed:
 Data Updated:
 Input:			daddr: Ŀ��IP�� dport: Ŀ�Ķ˿�
                sport: Դ�˿ڣ� saddr: ԴIP
                family: Э����AF_INET/AF_INET6
                proto: Э��SOL_TCP/SOL_UDP
                valid: ����valid����invalid HASH���key
 Output:			��
 Return:			-1:ʧ��

 Others:
=========================================================================*/
static inline unsigned int tbs_nfp_ct_hash(unsigned short family, const void *saddr,
            const void *daddr, unsigned short sport, unsigned short dport,
            unsigned char proto, bool valid)
{
	unsigned int key1, key2, mask = TBS_NFP_CT_INV_HASH_MASK;

    if(valid)
        mask = TBS_NFP_CT_HASH_MASK;

	if (family == AF_INET)
	{
		key1 = jhash((void *)saddr, 4, (proto << 16) | proto);
		key2 = jhash((void *)daddr, 4, (sport << 16) | dport);
	}
	else
	{
    	key1 = jhash((void *)saddr, 16, (proto << 16) | proto);
    	key2 = jhash((void *)daddr, 16, (sport << 16) | dport);
	}

	return jhash_2words(key1, key2, g_jhash_iv) & mask;
}


/*=========================================================================
 Function:		static inline unsigned int tbs_nfp_ct_hash_by_fib(unsigned short family, const unsigned int *saddr,
        const unsigned int *daddr, bool valid)
 Description:		����5 Ԫ��ȡhash
 Data Accessed:
 Data Updated:
 Input:			daddr: Ŀ��IP
                saddr: ԴIP
                family: Э����AF_INET/AF_INET6
                valid: ����valid����invalid HASH���key
 Output:			��
 Return:			-1:ʧ��

 Others:
=========================================================================*/
static inline unsigned int tbs_nfp_ct_hash_by_fib(unsigned short family, const unsigned int *saddr,
        const unsigned int *reply_saddr, bool valid)
{
    unsigned int key1, key2, mask = TBS_NFP_CT_INV_HASH_BY_FIB_MASK;

    if(valid)
        mask = TBS_NFP_CT_HASH_BY_FIB_MASK;

    if (family == AF_INET6)
    {
        key1 = saddr[0] ^ saddr[1] ^ saddr[2] ^ saddr[3];
        key2 = reply_saddr[0] ^ reply_saddr[1] ^ reply_saddr[2] ^ reply_saddr[3];
        return (jhash_2words(key1, key2, g_jhash_iv) & mask);
    }
    else
    {
        key1 = saddr[0];
        key2 = reply_saddr[0];
        return (jhash_2words(key1, key2, g_jhash_iv) & mask);
    }
}


/*=========================================================================
 Function:		static inline unsigned int tbs_nfp_bridge_hash(const unsigned char *da,
            const unsigned char *sa, unsigned int iif)
 Description:		����bridge ruleȡhash
 Data Accessed:
 Data Updated:
 Input:             da: dst address
                    sa: src address
                    iif: input interface
 Output:			��
 Return:			-1:ʧ��

 Others:
=========================================================================*/
static inline unsigned int tbs_nfp_bridge_hash(const unsigned char *da, const unsigned char *sa,
        unsigned int iif)
{
    unsigned int key1 = 0;
    unsigned int key2 = 0;
    unsigned int key3 = iif;

    key1 += ((unsigned int) da[0]);
    key1 += ((unsigned int) da[1] << 8);
    key1 += ((unsigned int) (da[2] + da[5]) << 16);
    key1 += ((unsigned int) (da[3] + da[4]) << 24);

    key2 += ((unsigned int) sa[0]);
    key2 += ((unsigned int) sa[1] << 8);
    key2 += ((unsigned int) (sa[2] + sa[5]) << 16);
    key2 += ((unsigned int) (sa[3] + sa[4]) << 24);

    return (jhash_3words(key1, key2, key3, g_jhash_iv) & TBS_NFP_BRIDGE_HASH_MASK);
}


/*=========================================================================
 Function:		static inline unsigned int HASH_FDB(unsigned int bridge_if, const unsigned char *mac)

 Description:		get fdb hash key
 Data Accessed:
 Data Updated:
 Input:             bridge_if: bridge interface index
                    mac: mac address
 Output:			��
 Return:			-1:ʧ��

 Others:
=========================================================================*/
static inline unsigned int tbs_nfp_fdb_hash(unsigned int bridge_if, const unsigned char *mac)
{
	unsigned int hash = 0;
	unsigned int align = (unsigned int)mac & 3;

	switch (align) {
	case 0:
		/* SA - 4 byte alignement - BE support TBD */
		hash = jhash_3words(bridge_if, *(unsigned short *)(mac), (*(unsigned short *)(mac + 2)) | ((*(unsigned short *)(mac + 4)) << 16),
							g_jhash_iv);
		break;

	case 2:
		/* DA - 2 byte alignement */
		hash = jhash_3words(bridge_if, *(unsigned short *)(mac), *(unsigned int *)(mac + 2), g_jhash_iv);
		break;

	default:
		printk("%s: Unexpected alignment: mac=%p\n", __func__, mac);
	}

	hash &= TBS_NFP_FDB_HASH_MASK;
	return hash;
}


/*=========================================================================
 Function:		static inline unsigned int tbs_nfp_itf_hash(unsigned int ifindex)
 Description:		�����Interface index ȡhash
 Data Accessed:
 Data Updated:
 Input:
 Output:			��
 Return:			-1:ʧ��

 Others:
=========================================================================*/
static inline unsigned int tbs_nfp_itf_hash(unsigned int ifindex)
{
    return ifindex & TBS_NFP_ITF_HASH_MASK;
}


/*=========================================================================
 Function:		static inline int tbs_nfp_mac_addr_eq(void *pmacaddr1, void *pmacaddr2)
 Description:		mac address compare
 Data Accessed:
 Data Updated:
 Input:
 Output:			��
 Return:			true:��ȣ� false:�����

 Others:
=========================================================================*/
static inline int tbs_nfp_mac_addr_eq(void *pmacaddr1, void *pmacaddr2)
{
    unsigned short *p1;
    unsigned short *p2;
    p1 = (unsigned short *)pmacaddr1;
    p2 = (unsigned short *)pmacaddr2;
    return p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2];
}


/*=========================================================================
 Function:		static inline void tbs_nfp_l3_addr_copy(int family, unsigned char *dst, const unsigned char *src)
 Description:		L3 address copy. Supports AF_INET and AF_INET6
 Data Accessed:
 Data Updated:
 Input:
 Output:			��
 Return:			��

 Others:
=========================================================================*/
static inline void tbs_nfp_l3_addr_copy(int family, unsigned char *dst, const unsigned char *src)
{
	const unsigned int *s = (const u32 *)src;
	unsigned int *d = (unsigned int *) dst;

	*d++ = *s++;		/* 4 */
	if (family == AF_INET)
		return;

	*d++ = *s++;		/* 8 */
	*d++ = *s++;		/* 12 */
	*d++ = *s++;		/* 16 */
}


/*=========================================================================
 Function:		static inline unsigned int tbs_nfp_l3_addr_eq(int family, const unsigned char *a, const unsigned char *b)
 Description:		L3 address compare. Supports AF_INET and AF_INET6
 Data Accessed:
 Data Updated:
 Input:
 Output:			��
 Return:			true:��ȣ� false:�����

 Others:
=========================================================================*/
static inline unsigned int tbs_nfp_l3_addr_eq(int family, const unsigned char *a, const unsigned char *b)
{
	const unsigned int *aa = (const unsigned int *)a;
	const unsigned int *bb = (const unsigned int *)b;
	unsigned int r;

	r = *aa++ ^ *bb++;	/* 4 */
	if (family == AF_INET)
		return !r;

	r |= *aa++ ^ *bb++;	/* 8 */
	r |= *aa++ ^ *bb++;	/* 12 */
	r |= *aa++ ^ *bb++;	/* 16 */

	return !r;
}


#ifdef TBS_NFP_MCF
#ifdef TBS_NFP_IGMP_SNOOPING
/*=========================================================================
 Function:      static inline unsigned int tbs_nfp_mc_fdb_hash(int family, const unsigned int *saddr,
                                    const unsigned int *group, unsigned int br_port)
 Description:       ͨ��family, saddr��group����hash��֧��ipv4��ipv6
 Data Accessed:
 Data Updated:
 Input:             int family      Э����
                    const unsigned int *saddr �����ֽ����鲥Դip��ַ
                    const unsigned int *group �����ֽ����鲥��ip��ַ
 Output:            ��
 Return:            hash_key
 Others:
=========================================================================*/
static inline unsigned int tbs_nfp_mc_fdb_hash(int family, const unsigned int *saddr,
                                    const unsigned int *group)
{
    unsigned int key1, key2;

    if (family == AF_INET6)
    {
        key1 = saddr[0] ^ saddr[1] ^ saddr[2] ^ saddr[3];
        key2 = group[0] ^ group[1] ^ group[2] ^ group[3];
        return (jhash_2words(key1, key2, g_jhash_iv) & TBS_NFP_MC_FDB_HASH_MASK);
    }
    else
    {
        key1 = saddr[0];
        key2 = group[0];
        return (jhash_2words(key1, key2, g_jhash_iv) & TBS_NFP_MC_FDB_HASH_MASK);
    }
}
#endif


/*=========================================================================
 Function:      static inline unsigned int tbs_nfp_mc_fdb_hash(int family, const unsigned int *saddr,
                                    const unsigned int *group, unsigned int br_port)
 Description:       ͨ��family,br_port, saddr��group����hash��֧��ipv4��ipv6
 Data Accessed:
 Data Updated:
 Input:             int family      Э����
                    const unsigned int *saddr �����ֽ����鲥Դip��ַ
                    const unsigned int *group �����ֽ����鲥��ip��ַ
                    int in_vif                ��ӿ�vif_index
 Output:            ��
 Return:            hash_key
 Others:
=========================================================================*/
static inline unsigned int tbs_nfp_mcf_hash(int family, const unsigned int *saddr,
                                    const unsigned int *group, int in_vif)
{
    unsigned int key1, key2;

    if (family == AF_INET6)
    {
        key1 = saddr[0] ^ saddr[1] ^ saddr[2] ^ saddr[3];
        key2 = group[0] ^ group[1] ^ group[2] ^ group[3];
        return (jhash_3words(key1, key2, in_vif, g_jhash_iv) & TBS_NFP_MCF_HASH_MASK);
    }
    else
    {
        key1 = saddr[0];
        key2 = group[0];
        return (jhash_3words(key1, key2, in_vif, g_jhash_iv) & TBS_NFP_MCF_HASH_MASK);
    }
}
#endif

#endif  /* __TBS_NFP_COMMON_H__ */
