/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* �ļ����� : tbs_nfp_ct.h
* �ļ����� : TBS������conntrack����ģ��
*
* �޶���¼ :
*          1 ���� :
*            ���� : 2011-11-10
*            ���� :
*
*******************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/notifier.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/rtnetlink.h>
#include <linux/etherdevice.h>
#include <linux/notifier.h>
#include <net/neighbour.h>

#include "tbs_nfp.h"
#include "tbs_nfp_ct.h"


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/

/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
static struct kmem_cache *tbs_nfp_ct_rule_cache __read_mostly;
unsigned int g_ct_max_size __read_mostly = TBS_NFP_CT_RULE_DEF;
static unsigned int  tbs_nfp_ct_rule_num = 0;

#ifdef TBS_NFP_CT
struct list_head g_ct_hash[TBS_NFP_CT_HASH_SIZE];
struct list_head g_ct_hash_by_fib[TBS_NFP_CT_HASH_BY_FIB_SIZE];

struct list_head g_ct_inv_hash[TBS_NFP_CT_INV_HASH_SIZE];
struct list_head g_ct_inv_hash_by_fib[TBS_NFP_CT_INV_HASH_BY_FIB_SIZE];
#endif /* TBS_NFP_CT */


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:		static inline void tbs_nfp_ct_print(const TBS_NFP_RULE_CT *ct)

 Description:		��Ӧconntrack
 Data Accessed:     ��
 Data Updated:
 Input:			    const TBS_NFP_RULE_CT *ct   �����conntrack
 Output:			��
 Return:			��
 Others:
=========================================================================*/
static inline void tbs_nfp_ct_print(const TBS_NFP_TUPLE_CT *tuple_ct)
{
	/*show the orignal rule*/
	if(NULL == tuple_ct)
		return;

	printk("[orignal] family=%d proto=%hu ",
			tuple_ct->family, tuple_ct->proto);

	if(AF_INET == tuple_ct->family)
		printk("src=%pI4 dst=%pI4 ",
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_ORIGINAL].src_l3,
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_ORIGINAL].dst_l3);
	else
		printk("src=%pI6 dst=%pI6 ",
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_ORIGINAL].src_l3,
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_ORIGINAL].dst_l3);

	printk("sport=%hu dport=%hu count=%u flags=0x%02x %s\n",
			ntohs(((unsigned short *)&tuple_ct->tuplehash[CT_DIR_ORIGINAL].ports)[0]),
			ntohs(((unsigned short *)&tuple_ct->tuplehash[CT_DIR_ORIGINAL].ports)[1]),
			tuple_ct->count, tuple_ct->tuplehash[CT_DIR_ORIGINAL].flags,
			(tuple_ct->tuplehash[CT_DIR_ORIGINAL].fib) ? "valid":"invalid");

	/*show the reply rule*/
	printk("[reply]   family=%d proto=%hu ",
			tuple_ct->family, tuple_ct->proto);

	if(AF_INET == tuple_ct->family)
		printk("src=%pI4 dst=%pI4 ",
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_REPLY].src_l3,
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_REPLY].dst_l3);
	else
		printk("src=%pI6 dst=%pI6 ",
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_REPLY].src_l3,
				(struct in_addr*)tuple_ct->tuplehash[CT_DIR_REPLY].dst_l3);

	printk("sport=%hu dport=%hu count=%u flags=0x%02x %s\n\n",
			ntohs(((unsigned short *)&tuple_ct->tuplehash[CT_DIR_REPLY].ports)[0]),
			ntohs(((unsigned short *)&tuple_ct->tuplehash[CT_DIR_REPLY].ports)[1]),
			tuple_ct->count, tuple_ct->tuplehash[CT_DIR_REPLY].flags,
			(tuple_ct->tuplehash[CT_DIR_REPLY].fib) ? "valid":"invalid");

	return;
}


/*=========================================================================
 Function:		void tbs_nfp_ct_dump(void)

 Description:		��ӡȫ��conntrack
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			��
 Others:
=========================================================================*/
void tbs_nfp_ct_dump(void)
{
	ST_TUPLE_HASH *tmp_entry = NULL;
	unsigned int hash;

	printk("ct_rule_num:%u\n", tbs_nfp_ct_rule_num);

	TBS_NFP_READ_LOCK(&tbs_nfp_lock);

	for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_ct_hash); hash++)
	{
		list_for_each_entry(tmp_entry, &g_ct_hash[hash], list)
		{
			if(tmp_entry->dir == CT_DIR_ORIGINAL)
				tbs_nfp_ct_print(GET_TUPLE_CT(tmp_entry));
		}
	}

	for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_ct_inv_hash); hash++)
	{
		list_for_each_entry(tmp_entry, &g_ct_inv_hash[hash], list)
		{
			if(tmp_entry->dir == CT_DIR_ORIGINAL)
				tbs_nfp_ct_print(GET_TUPLE_CT(tmp_entry));
		}
	}
	TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

    return;
}


/*=========================================================================
 Function:		TBS_NFP_RULE_CT *tbs_nfp_ct_lookup_by_tuple(int family,
    const unsigned char *sip, const unsigned char *dip, unsigned int ports,
    unsigned short proto, bool valid)

 Description:		ͨ����Ԫ���ָ��hash���в���conntrack
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    int family              Э����
                    const unsigned char *src_l3 ԴIP��ַ
                    const unsigned char *dst_l3 Ŀ��IP��ַ
                    unsigned int ports      Դ��Ŀ�Ķ˿ں�
                    unsigned short proto    Э���
                    const bool valid        hash��g_ct_hash or g_ct_inv_hash
 Output:			��
 Return:			NULL:�����ڣ�����:TBS_NFP_RULE_CT ָ��
 Others:
=========================================================================*/
ST_TUPLE_HASH *tbs_nfp_ct_lookup_by_tuple(int family,
    const unsigned char *sip, const unsigned char *dip, unsigned int ports,
    unsigned char proto, bool valid)
{
    unsigned int hash;
	TBS_NFP_TUPLE_CT *tuple_ct;
	ST_TUPLE_HASH *ct;
    const struct list_head *head = (valid)? g_ct_hash:g_ct_inv_hash;

    hash = tbs_nfp_ct_hash(family, sip, dip,
		((unsigned short *)&ports)[0], ((unsigned short *)&ports)[1],
        proto, valid);

    if(list_empty(&head[hash]))
        return NULL;

	list_for_each_entry(ct, &head[hash], list) {
		tuple_ct = GET_TUPLE_CT(ct);
		if ((tuple_ct->family == family) &&
				tbs_nfp_l3_addr_eq(tuple_ct->family, ct->src_l3, sip) &&
				tbs_nfp_l3_addr_eq(tuple_ct->family, ct->dst_l3, dip) &&
				(ct->ports == ports) && (tuple_ct->proto == proto))
			return ct;
	}

    return NULL;
}


/*=========================================================================
 Function:		static inline void tbs_nfp_ct_valid(TBS_NFP_RULE_CT *ct,
            TBS_NFP_RULE_FIB *fib)

 Description:		��ct��g_ct_inv_hash���Ƶ�g_ct_hash����
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   ·�ɻ������
 Output:			��
 Return:			��
 Others:
=========================================================================*/
static inline void tbs_nfp_ct_valid(ST_TUPLE_HASH *ct, TBS_NFP_RULE_FIB *fib, unsigned char proto)
{
    unsigned int hash , hash_by_fib;
    TBS_NFP_INTO_FUNC;

    ct->flags &= ~TBS_NFP_F_CT_FIB_INV;
    if(!(ct->flags & TBS_NFP_F_CT_INV_MASK))
    {
		hash = tbs_nfp_ct_hash(fib->family, ct->src_l3,
				ct->dst_l3, ((unsigned short *)&ct->ports)[0],
				((unsigned short *)&ct->ports)[1], proto, true);

		hash_by_fib = tbs_nfp_ct_hash_by_fib(fib->family, (unsigned int *)fib->src_l3,
				(unsigned int *)fib->dst_l3, true);

        /*fib �������ü���*/
        ct->fib = fib;
        fib->ref++;

        /*move ct to valid hash��*/
        list_move(&ct->list, &g_ct_hash[hash]);
        list_move(&ct->list_by_fib, &g_ct_hash_by_fib[hash_by_fib]);
    }

    return;
}


/*=========================================================================
 Function:		static inline void tbs_nfp_ct_invalid(TBS_NFP_RULE_CT *ct)

 Description:		��ct��g_ct_inv_hash�����Ƶ�g_ct_hash����
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   ·�ɻ������
 Output:			��
 Return:			��
 Others:
=========================================================================*/
static inline void tbs_nfp_ct_invalid(ST_TUPLE_HASH *ct, int family, unsigned char proto)
{
	unsigned int hash , hash_by_fib;

	ct->flags |= TBS_NFP_F_CT_FIB_INV;

	/*�ͷ�fib���ü���*/
	if(ct->fib)
	{
		ct->fib->ref--;
		ct->fib = NULL;
	}

	hash = tbs_nfp_ct_hash(family, ct->src_l3,
			ct->dst_l3, *(unsigned short *)&ct->ports,
			*((unsigned short *)&ct->ports + 1), proto, false);

	hash_by_fib = tbs_nfp_ct_hash_by_fib(family, (unsigned int *)ct->src_l3,
			(unsigned int *)(GET_NEXT_DIR_TUPLE(ct)->src_l3), false);

	/*move ct to invalid hash��*/
	list_move(&ct->list, &g_ct_inv_hash[hash]);
	list_move(&ct->list_by_fib, &g_ct_inv_hash_by_fib[hash_by_fib]);

	return;
}


/*=========================================================================
 Function:		void tbs_nfp_ct_fib_update(const TBS_NFP_RULE_FIB *fib)

 Description:		��g_ct_inv_hash������fib�й�����ct�Ƶ�g_ct_hash����
 Data Accessed:     g_ct_hash_by_fib/g_ct_inv_hash
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   ·�ɻ������
 Output:			��
 Return:			��
 Others:
=========================================================================*/
void tbs_nfp_ct_fib_invalid(const TBS_NFP_RULE_FIB *fib)
{
	ST_TUPLE_HASH*ct, *next;
	TBS_NFP_TUPLE_CT *tuple_ct;
	unsigned char *pdst_l3;

	unsigned int hash_by_fib = tbs_nfp_ct_hash_by_fib(fib->family, (unsigned int *)fib->src_l3,
			(unsigned int *)fib->dst_l3, true);

	list_for_each_entry_safe(ct, next, &g_ct_hash_by_fib[hash_by_fib], list_by_fib)
	{
		tuple_ct = GET_TUPLE_CT(ct);
		if(ct->flags & TBS_NFP_F_CT_DNAT)
			pdst_l3 = tuple_ct->tuplehash[CT_NEXT_DIR(ct->dir)].src_l3;
		else
			pdst_l3 = tuple_ct->tuplehash[ct->dir].dst_l3;

		if(fib->family == tuple_ct->family &&
				tbs_nfp_l3_addr_eq(fib->family, fib->src_l3, tuple_ct->tuplehash[ct->dir].src_l3) &&
				tbs_nfp_l3_addr_eq(fib->family, fib->dst_l3, pdst_l3))
			tbs_nfp_ct_invalid(ct, tuple_ct->family, tuple_ct->proto);
	}

	return;
}


/*=========================================================================
 Function:		void tbs_nfp_ct_fib_update(TBS_NFP_RULE_FIB *fib)

 Description:		��g_ct_inv_hash������fib�й�����ct�Ƶ�g_ct_hash����
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   ·�ɻ������
 Output:			��
 Return:			��
 Others:
=========================================================================*/
void tbs_nfp_ct_fib_update(TBS_NFP_RULE_FIB *fib)
{
	ST_TUPLE_HASH*ct, *next;
	TBS_NFP_TUPLE_CT *tuple_ct;
	unsigned char *pdst_l3;

	unsigned int hash_by_fib = tbs_nfp_ct_hash_by_fib(fib->family, (unsigned int *)fib->src_l3,
			(unsigned int *)fib->dst_l3, false);

	list_for_each_entry_safe(ct, next, &(g_ct_inv_hash_by_fib[hash_by_fib]), list_by_fib)
	{
		tuple_ct = GET_TUPLE_CT(ct);
		if(ct->flags & TBS_NFP_F_CT_DNAT)
			pdst_l3 = tuple_ct->tuplehash[CT_NEXT_DIR(ct->dir)].src_l3;
		else
			pdst_l3 = ct->dst_l3;

		if(tbs_nfp_l3_addr_eq(fib->family, fib->src_l3, ct->src_l3) &&
				tbs_nfp_l3_addr_eq(fib->family, fib->dst_l3, pdst_l3))
		{
			tbs_nfp_ct_valid(ct, fib, tuple_ct->proto);
        }
	}

	return;
}


/*=========================================================================
 Function:		static inline void __tbs_nfp_ct_clean(void)
 Description:		�������ָ��hash���еĹ���
 Data Accessed:     g_ct_hash or g_ct_inv_hash
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			��
 Others:
=========================================================================*/
static inline void __tbs_nfp_ct_clean(bool valid)
{
	unsigned int hash = 0;
	struct list_head *head;
	unsigned int hash_num;
	ST_TUPLE_HASH *tmp_entry = NULL;
	TBS_NFP_TUPLE_CT *tuple_ct;

	head = valid ? g_ct_hash:g_ct_inv_hash;
	hash_num = valid ?TBS_NFP_ARRAY_SIZE(g_ct_hash):TBS_NFP_ARRAY_SIZE(g_ct_inv_hash);

	TBS_NFP_INTO_FUNC;

	for (hash = 0; hash < hash_num; hash++)
	{
        /*
        ������ܻ����˫����Ԫ����ͬһ��hashͰ�У����Բ�����
        list_for_each_entry_safe()����ɾ��
        */
        while(!list_empty(&(head[hash])))
        {
            tmp_entry = list_entry((head[hash]).next, ST_TUPLE_HASH, list);
			tuple_ct = GET_TUPLE_CT(tmp_entry);

			/* delete CT_DIR_ORIGINAL direction ct rule */
			if(tuple_ct->tuplehash[CT_DIR_ORIGINAL].fib)
				tuple_ct->tuplehash[CT_DIR_ORIGINAL].fib->ref --;

			list_del(&tuple_ct->tuplehash[CT_DIR_ORIGINAL].list);
			list_del(&tuple_ct->tuplehash[CT_DIR_ORIGINAL].list_by_fib);

			/* delete CT_DIR_REPLY direction ct rule */
			if(tuple_ct->tuplehash[CT_DIR_REPLY].fib)
				tuple_ct->tuplehash[CT_DIR_REPLY].fib->ref --;

			list_del(&tuple_ct->tuplehash[CT_DIR_REPLY].list);
			list_del(&tuple_ct->tuplehash[CT_DIR_REPLY].list_by_fib);

			tbs_nfp_ct_rule_num --;

			kmem_cache_free(tbs_nfp_ct_rule_cache, tuple_ct);
        }
	}
}
/*=========================================================================
 Function:		static inline void tbs_nfp_ct_clean(void)
 Description:		�������ct����
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			��
 Others:
=========================================================================*/
static inline void tbs_nfp_ct_clean(void)
{
	__tbs_nfp_ct_clean(true);
	__tbs_nfp_ct_clean(false);
	return;
}


/*=========================================================================
 Function:		static inline int tbs_nfp_ct_reset(int family,
                const unsigned char *sip, const unsigned char *dip,
                unsigned int ports, unsigned char proto)

 Description:		�������ct����
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			��
 Others:
=========================================================================*/
void tbs_nfp_ct_reset(void)
{
    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
    tbs_nfp_ct_clean();
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return;
}



/*=========================================================================
 Function:      static inline bool tbs_nfp_ct_exist(int family, const unsigned char *sip,
				const unsigned char *dip, unsigned short sport, unsigned short dport,
				unsigned char proto)

 Description:		������Ԫ����Ϣ�Ƿ��Ѿ�����hash������
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    ��Ԫ����Ϣ
                    family: IP version
					sip: source ip address
					dip: dest ip address
					sport: source port
					dport: dest port
					proto: protocols number
 Output:			��
 Return:			0:  �ɹ�������: ʧ��
 Others:
=========================================================================*/
static inline bool tbs_nfp_ct_exist(int family, const unsigned char *sip,
    const unsigned char *dip, unsigned int ports, unsigned char proto)
{
	bool exist = false;
	TBS_NFP_READ_LOCK(&tbs_nfp_lock);
	if(tbs_nfp_ct_lookup_by_tuple(family, sip, dip, ports, proto, true)||
			tbs_nfp_ct_lookup_by_tuple(family, sip, dip, ports, proto, false))
		exist = true;
	TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

	return exist;
}


/*=========================================================================
 Function:		int tbs_nfp_ct_add(int family, const unsigned char *sip, const unsigned char *dip,
		unsigned short sport, unsigned short dport, unsigned char proto,
		const unsigned char *reply_sip, const unsigned char *reply_dip,
		unsigned short reply_sport, unsigned short reply_dport)

 Description:		��g_ct_inv_hash/g_ct_hash�������ָ����5Ԫ����Ϣ
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    ˫����Ԫ����Ϣ
 Output:			��
 Return:			0:  �ɹ�������: ʧ��
 Others:
=========================================================================*/
int tbs_nfp_ct_add(int family, const unsigned char *sip, const unsigned char *dip,
		unsigned short sport, unsigned short dport, unsigned char proto,
		const unsigned char *reply_sip, const unsigned char *reply_dip,
		unsigned short reply_sport, unsigned short reply_dport)
{
	TBS_NFP_TUPLE_CT * new_tupe_ct;
	TBS_NFP_RULE_FIB * fib_entry;
	unsigned int hash;
	unsigned int ports;
	int ip_len;
#ifdef TBS_NFP_NAT
	int original_flags = 0;
	int reply_flags = 0;
#endif

	((unsigned short* )&ports)[0] = sport;
	((unsigned short* )&ports)[1] = dport;

	//��ǰ�����Ƿ��Ѿ�����
	if(tbs_nfp_ct_exist(family, sip, dip, ports, proto))
		return TBS_NFP_OK;

	//�����������ܳ��� g_ct_max_size
	if(g_ct_max_size == tbs_nfp_ct_rule_num)
	{
        TBS_NFP_DEBUG("tbs_nfp_ct_rule_num(%u) == g_ct_max_size\n",
            tbs_nfp_ct_rule_num);

		return TBS_NFP_ERR;
	}

	new_tupe_ct = kmem_cache_alloc(tbs_nfp_ct_rule_cache, GFP_ATOMIC);
	if(NULL == new_tupe_ct )
		return TBS_NFP_ERR;

	memset(new_tupe_ct, 0, sizeof(*new_tupe_ct));
	/* 5 tuple key */
	new_tupe_ct->family = family;
	new_tupe_ct->proto = proto;

	if(AF_INET == family) //ipv4
	{
		ip_len = TBS_NFP_MAX_IPV4_ADDR_SIZE;
	}
	else //ipv6
	{
		ip_len = TBS_NFP_MAX_L3_ADDR_SIZE;
	}

#ifdef TBS_NFP_NAT
//tbs_nfp_l3_addr_eq(family, reply_sip, sip)
	if(((!tbs_nfp_l3_addr_eq(family, reply_dip, sip))|| reply_dport != sport) &&
			(tbs_nfp_l3_addr_eq(family, reply_sip, dip) && reply_sport == dport))
	{
		/*snat*/
		original_flags = TBS_NFP_F_CT_SNAT;
		reply_flags = TBS_NFP_F_CT_DNAT;
	}
//	else if((reply_dip ==
//				sip && reply_dport == sport) && (
//					reply_sip != dip || reply_sport != dport))
	else if(((tbs_nfp_l3_addr_eq(family, reply_dip, sip) && reply_dport == sport ))&&
			((!tbs_nfp_l3_addr_eq(family, reply_sip, dip)) || reply_sport != dport))

	{
		/*dnat*/
		original_flags = TBS_NFP_F_CT_DNAT;
		reply_flags = TBS_NFP_F_CT_SNAT;
	}
	else if((!tbs_nfp_l3_addr_eq(family, reply_dip, sip)||reply_dport != sport) &&
			(!tbs_nfp_l3_addr_eq(family, reply_dip, dip) ||  reply_sport != dport))
	//else if((reply_dip !=
	//			sip || reply_dport != sport) && (
	//				reply_sip != dip || reply_sport != dport))
	{
		/*dnat & snat*/
		original_flags = TBS_NFP_F_CT_SNAT | TBS_NFP_F_CT_DNAT;
		reply_flags = TBS_NFP_F_CT_DNAT| TBS_NFP_F_CT_SNAT;
	}
	else
	{
		/*no nat*/
	}
#endif /*TBS_NFP_NAT*/

	((unsigned short* )&ports)[0] = sport;
	((unsigned short* )&ports)[1] = dport;
	/* original direction initial */
	memcpy(new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].src_l3, sip, ip_len);
	memcpy(new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].dst_l3, dip, ip_len);
	new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].ports = ports;
#ifdef TBS_NFP_NAT
	new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].flags = original_flags;
#endif
	new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].dir = CT_DIR_ORIGINAL;

	/* reply direction initial */
	((unsigned short* )&ports)[0] = reply_sport;
	((unsigned short* )&ports)[1] = reply_dport;
	memcpy(new_tupe_ct->tuplehash[CT_DIR_REPLY].src_l3, reply_sip, ip_len);
	memcpy(new_tupe_ct->tuplehash[CT_DIR_REPLY].dst_l3, reply_dip, ip_len);
	new_tupe_ct->tuplehash[CT_DIR_REPLY].ports = ports;
#ifdef TBS_NFP_NAT
	new_tupe_ct->tuplehash[CT_DIR_REPLY].flags = reply_flags;
#endif
	new_tupe_ct->tuplehash[CT_DIR_REPLY].dir = CT_DIR_REPLY;

	TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

	/*
	 * ����fib�������Ƿ������Ӧ�Ĺ���
	 */
	fib_entry = tbs_nfp_fib_lookup(family, sip, reply_sip, true);
	new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].fib = fib_entry;

	/* fib�����в�������Ӧ�Ĺ��� */
	if(NULL == fib_entry)
	{
		new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].flags |= TBS_NFP_F_CT_FIB_INV;

		hash = tbs_nfp_ct_hash(family, sip, dip,
				sport, dport, proto, false);
		list_add(&new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].list, &g_ct_inv_hash[hash]);

		hash = tbs_nfp_ct_hash_by_fib(family, (unsigned int*)sip, (unsigned int*)reply_sip, false);

		list_add(&new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].list_by_fib, &g_ct_inv_hash_by_fib[hash]);

	}
	else /* fib�����д�����Ӧ�Ĺ��� */
	{
		new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].fib->ref ++;
		hash = tbs_nfp_ct_hash(family, sip, dip,
				sport, dport, proto, true);

		list_add(&new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].list, &g_ct_hash[hash]);

		hash = tbs_nfp_ct_hash_by_fib(family, (unsigned int*)sip, (unsigned int*)reply_sip, true);
		list_add(&new_tupe_ct->tuplehash[CT_DIR_ORIGINAL].list_by_fib, &g_ct_hash_by_fib[hash]);
	}

	//reply
	fib_entry = tbs_nfp_fib_lookup(family, reply_sip, sip, true);
	new_tupe_ct->tuplehash[CT_DIR_REPLY].fib = fib_entry;

	/* fib�����в�������Ӧ�Ĺ��� */
	if(NULL == fib_entry)
	{

		new_tupe_ct->tuplehash[CT_DIR_REPLY].flags |= TBS_NFP_F_CT_FIB_INV;

		hash = tbs_nfp_ct_hash(family, reply_sip, reply_dip,
				dport, sport, proto, false);
		list_add(&new_tupe_ct->tuplehash[CT_DIR_REPLY].list, &(g_ct_inv_hash[hash]));

		hash = tbs_nfp_ct_hash_by_fib(family, (unsigned int*)reply_sip, (unsigned int*)sip, false);

		list_add(&(new_tupe_ct->tuplehash[CT_DIR_REPLY].list_by_fib), &(g_ct_inv_hash_by_fib[hash]));

	}
	else /* fib�����д�����Ӧ�Ĺ��� */
	{
		new_tupe_ct->tuplehash[CT_DIR_REPLY].fib->ref ++;
		hash = tbs_nfp_ct_hash(family, reply_sip, reply_dip,
				dport, sport, proto, true);

		list_add(&new_tupe_ct->tuplehash[CT_DIR_REPLY].list, &g_ct_hash[hash]);

		hash = tbs_nfp_ct_hash_by_fib(family, (unsigned int*)reply_sip, (unsigned int*)sip, true);
		list_add(&new_tupe_ct->tuplehash[CT_DIR_REPLY].list_by_fib, &g_ct_hash_by_fib[hash]);
	}
	//new_ct_entry->count = 10;
	++tbs_nfp_ct_rule_num;
	TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

	return TBS_NFP_OK;
}

/*=========================================================================
 Function:		int tbs_nfp_ct_delete(int family, const unsigned char *sip,
                const unsigned char *dip, unsigned short sport,
                unsigned short dport, unsigned char proco)

 Description:		��g_ct_inv_hash/g_ct_hash�����Ƴ�ָ����5Ԫ����Ϣ
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    ��Ԫ����Ϣ
 Output:			��
 Return:			0:  �ɹ�������: ʧ��
 Others:
=========================================================================*/
int tbs_nfp_ct_delete(int family, const unsigned char *sip, const unsigned char *dip,
               unsigned short sport, unsigned short dport, unsigned char proto)
{
	TBS_NFP_TUPLE_CT *ct_tuple;
	ST_TUPLE_HASH * ct_dir_tuple;
	int ret = TBS_NFP_OK;
	unsigned int ports;

	((unsigned short* )&ports)[0] = sport;
	((unsigned short* )&ports)[1] = dport;
	TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
	ct_dir_tuple = tbs_nfp_ct_lookup_by_tuple(family, sip, dip,
			ports, proto, true);
	if(NULL == ct_dir_tuple)
	{
		ct_dir_tuple = tbs_nfp_ct_lookup_by_tuple(family, sip, dip,
				ports, proto, false);
		if(NULL == ct_dir_tuple)
		{
			ret = TBS_NFP_ERR;
			goto out1;
		}
	}

	ct_tuple = GET_TUPLE_CT(ct_dir_tuple);

	/* delete CT_DIR_ORIGINAL direction ct rule */
	if(ct_tuple->tuplehash[CT_DIR_ORIGINAL].fib)
		ct_tuple->tuplehash[CT_DIR_ORIGINAL].fib->ref--;
	list_del(&ct_tuple->tuplehash[CT_DIR_ORIGINAL].list);
	list_del(&ct_tuple->tuplehash[CT_DIR_ORIGINAL].list_by_fib);

	/* delete CT_DIR_REPLY direction ct rule */
	if(ct_tuple->tuplehash[CT_DIR_REPLY].fib)
		ct_tuple->tuplehash[CT_DIR_REPLY].fib->ref--;
	list_del(&ct_tuple->tuplehash[CT_DIR_REPLY].list);
	list_del(&ct_tuple->tuplehash[CT_DIR_REPLY].list_by_fib);

	kmem_cache_free(tbs_nfp_ct_rule_cache, ct_tuple);
	tbs_nfp_ct_rule_num--;

out1:
	TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
	return ret;
}


/*=========================================================================
 Function:		int tbs_nfp_ct_age(int family, const unsigned char *sip,
                const unsigned char *dip, unsigned short sport,
                unsigned short dport, unsigned char proco)

 Description:		��g_ct_inv_hash/g_ct_hash���в���ָ����Ԫ���¼�İ�����
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    ��Ԫ����Ϣ
 Output:			��
 Return:			0:  �ϻ���-1: ʧ�ܣ�����:   ������
 Others:
=========================================================================*/
unsigned int tbs_nfp_ct_age(int family, const unsigned char *sip, const unsigned char *dip,
               unsigned short sport, unsigned short dport, unsigned char proto)
{
	ST_TUPLE_HASH *ct_dir_tuple= NULL;
	TBS_NFP_TUPLE_CT *ct_tuple= NULL;
	unsigned int count = 0;
	unsigned int ports;
	((unsigned short* )&ports)[0] = sport;
	((unsigned short* )&ports)[1] = dport;

	TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

	ct_dir_tuple = tbs_nfp_ct_lookup_by_tuple(family, sip, dip, ports, proto, true);
	if(NULL == ct_dir_tuple)
	{
		ct_dir_tuple = tbs_nfp_ct_lookup_by_tuple(family, sip, dip, ports, proto, false);
		if(NULL == ct_dir_tuple)
			goto out;
	}
	ct_tuple = GET_TUPLE_CT(ct_dir_tuple);
	count = ct_tuple->count;
	ct_tuple->count = 0;

out:
	TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
	return count;
}


/*=========================================================================
 Function:		void tbs_nfp_ct_exit(void)

 Description:		conntrack��ģ���˳�����
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    ��Ԫ����Ϣ
 Output:			��
 Return:			��
 Others:
=========================================================================*/
void tbs_nfp_ct_exit(void)
{
	//�������ct����
	tbs_nfp_ct_reset();

	//�ͷ��ڴ�cache
	kmem_cache_destroy(tbs_nfp_ct_rule_cache);
	return;
}


/*=========================================================================
 Function:		int tbs_nfp_ct_init(void)

 Description:		conntrack��ģ���ʼ������
 Data Accessed:     g_ct_hash/g_ct_inv_hash
 Data Updated:
 Input:			    ��Ԫ����Ϣ
 Output:			��
 Return:			0:  �ϻ�������: ʧ��
 Others:
=========================================================================*/
int tbs_nfp_ct_init(void)
{
	unsigned int i = 0;
	int ret = 0;

	//�ڴ�Ԥ���仺����
	tbs_nfp_ct_rule_cache = kmem_cache_create("tbs_nfp_ct_rule_cache",
			sizeof(TBS_NFP_TUPLE_CT),
			0,
			SLAB_HWCACHE_ALIGN, NULL);
	if(!tbs_nfp_ct_rule_cache)
	{
		TBS_NFP_ERROR("Fail to kmem hash cache tbs_nfp_ct_rule_cache \n");
		ret = -ENOMEM;
		goto out;
	}
	tbs_nfp_ct_rule_num = 0;

    //��ʼ��hash��
	for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_ct_hash); i++)
	{
		INIT_LIST_HEAD(&g_ct_hash[i]);
	}

	for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_ct_hash_by_fib); i++)
	{
		INIT_LIST_HEAD(&g_ct_hash_by_fib[i]);
	}

	for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_ct_inv_hash); i++)
	{
		INIT_LIST_HEAD(&g_ct_inv_hash[i]);
	}
	for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_ct_inv_hash_by_fib); i++)
	{
		INIT_LIST_HEAD(&g_ct_inv_hash_by_fib[i]);
	}

out:
	return ret;
}

