/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* �ļ����� : tbs_nfp_fib.c
* �ļ����� : TBS������fib����ģ��
*
* �޶���¼ :
*          1 ���� : pengyao
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

#include "tbs_nfp_arp.h"
#include "tbs_nfp_fib.h"
#include "tbs_nfp_ct.h"
/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/

/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
struct list_head g_fib_hash[TBS_NFP_FIB_HASH_SIZE];
struct list_head g_fib_hash_by_gtw[TBS_NFP_FIB_HASH_BY_GTW_SIZE];

struct list_head g_fib_inv_hash[TBS_NFP_FIB_INV_HASH_SIZE];
struct list_head g_fib_inv_hash_by_gtw[TBS_NFP_FIB_INV_HASH_BY_GTW_SIZE];

static struct kmem_cache *tbs_nfp_fib_rule_cache __read_mostly;
unsigned int g_fib_max_size __read_mostly = TBS_NFP_FIB_RULE_DEF;
static unsigned int  tbs_nfp_fib_rule_num = 0;


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:		static inline void tbs_nfp_fib_printk(const TBS_NFP_RULE_FIB *fib)

 Description:		��ӡfib����
 Data Accessed:     ��

 Data Updated:
 Input:			    const TBS_NFP_RULE_FIB *fib fib����ָ��
 Output:			��
 Return:			��

 Others:
=========================================================================*/
static inline void tbs_nfp_fib_printk(unsigned char hash_key, const TBS_NFP_RULE_FIB *fib_rule)
{

    TBS_NFP_IF_MAP *oif_map = NULL;

    oif_map = tbs_nfp_ifmap_get(fib_rule->oif);
    if(NULL == oif_map)
    {
        printk("Get if_map fail(oif:%d)\n", fib_rule->oif);
        return;
    }

    if (AF_INET == fib_rule->family)
    {
        printk("%u\t%-16s"
            "%pI4\t%pI4\t%pI4\t"
            "%02x:%02x:%02x:%02x:%02x:%02x\t%u\t%u\t%u\n",
            hash_key, oif_map->name,
            fib_rule->src_l3,fib_rule->dst_l3,fib_rule->def_gtw_l3,
            fib_rule->da[0], fib_rule->da[1],
            fib_rule->da[2], fib_rule->da[3],
            fib_rule->da[4], fib_rule->da[5],
            fib_rule->count,fib_rule->flags,fib_rule->ref);
    }
    else
    {
        printk("%u\t%-16s"
            "%pI6\t\t%pI6\t\t%pI6\t\t"
            "%02x:%02x:%02x:%02x:%02x:%02x\t%u\t%u\t%u\n",
            hash_key, oif_map->name,
            fib_rule->src_l3,fib_rule->dst_l3,fib_rule->def_gtw_l3,
            fib_rule->da[0], fib_rule->da[1],
            fib_rule->da[2], fib_rule->da[3],
            fib_rule->da[4], fib_rule->da[5],
            fib_rule->count,fib_rule->flags,fib_rule->ref);
    }

    return;
}

/*=========================================================================
 Function:		void tbs_nfp_fib_dump(void)

 Description:		��ӡfib hash�����й���
 Data Accessed:     g_fib_hash[hash]
                    g_fib_inv_hash[hash]

 Data Updated:      ��
 Input:			    ��
 Output:			��ӡ���е�fib����
 Return:			��

 author:    tbs cairong
=========================================================================*/
void tbs_nfp_fib_dump(void)
{
    unsigned int hash = 0;
    TBS_NFP_RULE_FIB *tmp_entry;

    printk("fib_rule_num:%u\n", tbs_nfp_fib_rule_num);
    printk("valid fib:\n");
    printk("hash\toutdev\t\tsip\t\tdip\t\tgtw\t\t"
        "dmac\t\t\tcount\tflags\tref\n");

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);

    //���α�����ɾ������g_fib_hash��g_fib_hash_by_gtw�й���Ҳһ����ɾ��

    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_fib_hash); hash++)
    {
        list_for_each_entry(tmp_entry, &g_fib_hash[hash], list)
        {
            tbs_nfp_fib_printk(hash, tmp_entry);
        }
    }

    //���α�����ɾ������g_fib_inv_hash��g_fib_inv_hash_by_gtw�й���Ҳһ����ɾ��
    printk("\ninvalid fib:\n");
    printk("hash\toutdev\t\tsip\t\tdip\t\tgtw\t\t"
        "dmac\t\t\tcount\tflags\tref\n");
    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_fib_inv_hash); hash++)
    {
        list_for_each_entry(tmp_entry, &g_fib_inv_hash[hash], list)
        {
            tbs_nfp_fib_printk(hash, tmp_entry);
        }
    }

    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);
    return ;
}


static inline int tbs_nfp_fib_rule_cmp(int family, const unsigned char *src_l3,
		const unsigned char *dst_l3, TBS_NFP_RULE_FIB *rule)
{
	if ((family == rule->family) &&
			tbs_nfp_l3_addr_eq(family, rule->src_l3, src_l3) &&
			tbs_nfp_l3_addr_eq(family, rule->dst_l3, dst_l3))
		return true;

	return false;
}

inline TBS_NFP_RULE_FIB *tbs_nfp_fib_lookup(int family, const unsigned char *src_l3,
		const unsigned char *dst_l3, const bool valid)
{
	unsigned int hash;
	TBS_NFP_RULE_FIB *rule;
	struct list_head *head = (valid)? g_fib_hash:g_fib_inv_hash;

	hash = tbs_nfp_fib_hash(family, (unsigned int *)src_l3, (unsigned int *)dst_l3, valid);

	list_for_each_entry(rule, &head[hash], list)
    {
		if (tbs_nfp_fib_rule_cmp(family, src_l3, dst_l3, rule))
			return rule;
	}

	return NULL;
}

/*=========================================================================
 Function:		static bool tbs_nfp_fib_rule_exist(int family, unsigned char *nextHopL3)

 Description:		FIB�����Ƿ����
 Data Accessed:     struct list_head g_fib_hash[TBS_NFP_FIB_HASH_SIZE];
 Data Updated:
 Input:			    int family,               Э������ipv4/ipv6
                    unsigned char *nextHopL3  ��һ����ַ
 Output:			��
 Return:			true:   ���ڣ�flse: ������
 Others:
=========================================================================*/
inline bool tbs_nfp_fib_exist(int family, const unsigned char *src_l3,
		const unsigned char *dst_l3, const bool valid)
{
    bool exist = false;

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);

    if(tbs_nfp_fib_lookup(family, src_l3, dst_l3, valid))
        exist = true;

    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

    return exist;
}

/*=========================================================================
 Function:		inline void tbs_nfp_fib_valid(TBS_NFP_RULE_FIB *fib,
    unsigned int inv_flag)

 Description:		ȥ��fib����� inv_flag��־
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   ·�ɻ������
                    inv_flag    TBS_NFP_F_FIB_ARP_INV/TBS_NFP_F_FIB_BRIDGE_INV
 Output:
 Return:			��
 Others:            ������µ�fib��¼��g_fib_inv_hash�У������Ƶ�g_fib_hash
=========================================================================*/
inline void tbs_nfp_fib_valid(TBS_NFP_RULE_FIB *fib,
    unsigned int inv_flag)
{
    int hash, hash_by_gtw;

#ifdef TBS_NFP_BRIDGE
    TBS_NFP_RULE_BRIDGE *bridge_rule;
#endif /*TBS_NFP_BRIDGE*/

    TBS_NFP_INTO_FUNC;

    if(TBS_NFP_F_FIB_BRIDGE_INV != inv_flag &&
        TBS_NFP_F_FIB_ARP_INV != inv_flag)
    {
        TBS_NFP_ERROR("invalid fib inv_flag(%d)\n", inv_flag);
        return;
    }

    /*�����arp���£���Ҫ��lan��fib���ӿ�Ϊbr���и���*/
    if(TBS_NFP_F_FIB_ARP_INV == inv_flag &&
        (fib->flags & TBS_NFP_F_FIB_BRIDGE_INV))
    {
#ifdef TBS_NFP_BRIDGE
        bridge_rule = tbs_nfp_bridge_rule_lookup(fib->da, fib->sa, fib->oif);
        if(bridge_rule)
        {
            fib->oif = bridge_rule->oif;
            fib->flags &= ~TBS_NFP_F_FIB_BRIDGE_INV;
        }
#endif /*TBS_NFP_BRIDGE*/
    }

    fib->flags &= ~inv_flag;

    if(!(fib->flags & TBS_NFP_F_FIB_INV_MASK))
    {
        hash = tbs_nfp_fib_hash(fib->family, (unsigned int *)fib->src_l3,
            (unsigned int *)fib->dst_l3, true);

        /*move fib to valid hash ��*/
        list_move(&fib->list, &g_fib_hash[hash]);

        hash_by_gtw = tbs_nfp_fib_hash_gtw(fib->family, (unsigned int *)fib->def_gtw_l3, true);
        list_move(&fib->list_by_gtw, &g_fib_hash_by_gtw[hash_by_gtw]);

        /*����conntrack�����fib*/
#ifdef TBS_NFP_CT
        tbs_nfp_ct_fib_update(fib);
#endif  /* TBS_NFP_CT */
    }

    return;
}


/*=========================================================================
 Function:	inline void tbs_nfp_fib_invalid(TBS_NFP_RULE_FIB *fib,
    unsigned int inv_flag)

 Description:		����fib inv_flag��־
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    TBS_NFP_RULE_FIB *fib   ·�ɻ������
                    inv_flag    TBS_NFP_F_FIB_ARP_INV/TBS_NFP_F_FIB_BRIDGE_INV
 Output:
 Return:			��
 Others:            ������µ�fib��¼��g_fib_hash�У������Ƶ�g_fib_inv_hash
=========================================================================*/
inline void tbs_nfp_fib_invalid(TBS_NFP_RULE_FIB *fib,
    unsigned int inv_flag)
{
    int hash, hash_by_gtw;
    TBS_NFP_IF_MAP *if_map = NULL;

    if(TBS_NFP_F_FIB_BRIDGE_INV != inv_flag &&
        TBS_NFP_F_FIB_ARP_INV != inv_flag)
    {
        TBS_NFP_ERROR("invalid fib inv_flag(%d)\n", inv_flag);
        return;
    }

    if(!(fib->flags & TBS_NFP_F_FIB_INV_MASK))
    {
        if_map = tbs_nfp_ifmap_get(fib->oif);

        /*����TBS_NFP_F_FIB_BRIDGE_INV*/
        if(if_map && (if_map->flags & TBS_NFP_F_MAP_BRIDGE_PORT))
        {
            fib->oif = if_map->bridge_if;
            fib->flags |= TBS_NFP_F_FIB_BRIDGE_INV;
        }

        /*move fib to invalid hash����*/
        hash = tbs_nfp_fib_hash(fib->family, (unsigned int *)fib->src_l3,
                        (unsigned int *)fib->dst_l3, false);
        list_move(&fib->list, &g_fib_inv_hash[hash]);

        hash_by_gtw = tbs_nfp_fib_hash_gtw(fib->family, (unsigned int *)fib->def_gtw_l3, false);
        list_move(&fib->list_by_gtw, &g_fib_inv_hash_by_gtw[hash_by_gtw]);

        tbs_nfp_ct_fib_invalid(fib);
    }

    fib->flags |= inv_flag;

    return;
}


#ifdef TBS_NFP_BRIDGE
/*=========================================================================
 Function:		void tbs_nfp_fib_bridge_update(const TBS_NFP_RULE_BRIDGE *bridge_rule)

 Description:		���·�ɻ�����г��ӿ�Ϊbridge_rule->oif�����
                    ��TBS_NFP_F_FIB_BRIDGE_INV��־��������·�ɻ���
                    ������ӿ�Ϊbridge_rule->oif�豸
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    const TBS_NFP_RULE_BRIDGE *bridge_rule  ���������Ź���
 Output:
 Return:			0:�ɹ�  ����:ʧ��
 Others:
=========================================================================*/
void tbs_nfp_fib_bridge_update(const TBS_NFP_RULE_BRIDGE *bridge_rule)
{
    int i = 0;
    TBS_NFP_RULE_FIB *next_fib, *fib = NULL;

    //printk("In %s:   br_rule->da:%02x:%02x:%02x:%02x:%02x:%02x, sa:%02x:%02x:%02x:%02x:%02x:%02x, bridge_rule->iif:%d\n",
    //    __func__, bridge_rule->da[0], bridge_rule->da[1], bridge_rule->da[2], bridge_rule->da[3], bridge_rule->da[4], bridge_rule->da[5],
    //    bridge_rule->sa[0], bridge_rule->sa[1], bridge_rule->sa[2], bridge_rule->sa[3], bridge_rule->sa[4], bridge_rule->sa[5], bridge_rule->iif);


    //����hash��
    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_inv_hash); i++)
    {
        //��������
        list_for_each_entry_safe(fib, next_fib, &g_fib_inv_hash[i], list)
        {
            if(!(fib->flags & TBS_NFP_F_FIB_ARP_INV) &&
                (fib->flags & TBS_NFP_F_FIB_BRIDGE_INV))
            {
                if(tbs_nfp_mac_addr_eq((void *)fib->da, (void *)bridge_rule->da) &&
                    tbs_nfp_mac_addr_eq((void *)fib->sa, (void *)bridge_rule->sa) &&
                    fib->oif == bridge_rule->iif)
                {
                    tbs_nfp_fib_valid(fib, TBS_NFP_F_FIB_BRIDGE_INV);
                    fib->oif = bridge_rule->oif;
                }
            }
        }
    }

    return;
}


/*=========================================================================
 Function:		void tbs_nfp_fib_bridge_invalid(int br_index, int br_port_index)

 Description:		����·�ɻ����г��ӿ�Ϊbr_port_index��·�ɻ����־ΪTBS_NFP_F_FIB_BRIDGE_INV
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    int family  Э����ipv4/6
                    const char *sip Դip
                    const char *dip Ŀ��ip
 Output:
 Return:			0:�ɹ�  ����:ʧ��
 Others:
=========================================================================*/
void tbs_nfp_fib_bridge_invalid(int br_index, int br_port_index, unsigned const char *da)
{
    int i = 0;
    TBS_NFP_RULE_FIB *next_fib, *fib = NULL;

    //printk("In %s:%d, br_port:%d, da:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__, __LINE__,
    //    br_port_index, da[0], da[0], da[0], da[0], da[0], da[0]);

    //����hash��
    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_hash); i++)
    {
        //��������
        list_for_each_entry_safe(fib, next_fib, &g_fib_hash[i], list){
            if(fib->oif == br_port_index &&
                tbs_nfp_mac_addr_eq((void *)fib->da, (void *)da)) {
                tbs_nfp_fib_invalid(fib, TBS_NFP_F_FIB_BRIDGE_INV);
                fib->oif = br_index;
            }
        }
    }

    return;
}
#endif  /*TBS_NFP_BRIDGE*/


/*=========================================================================
 Function:		inline void tbs_nfp_fib_arp_update(int family, const unsigned char *next_hop, const unsigned char *mac)


 Description:		����·�ɻ�����Ŀ��ipΪdip��·�ɻ����־ΪTBS_NFP_F_FIB_ARP_INV
 Data Accessed:     struct list_head g_fib_hash[]
 Data Updated:
 Input:			    int family  Э����ipv4/6
                    const char *next_hop ��һ��ip
 Output:
 Return:			0:�ɹ�  ����:ʧ��
 Others:            ������µ�fib��¼��g_fib_hash�У������Ƶ�g_fib_inv_hash
=========================================================================*/
inline void tbs_nfp_fib_arp_update(int family, const unsigned char *next_hop, const unsigned char *mac)
{
    TBS_NFP_RULE_FIB *fib, *next_fib;
    int hash_by_gtw = tbs_nfp_fib_hash_gtw(family, (const unsigned int *)next_hop, false);

    TBS_NFP_INTO_FUNC;

    TBS_NFP_DEBUG("hash_by_gtw:%d,family:%d,next_hop:%pI4\n",hash_by_gtw,family,(void *)next_hop);
    list_for_each_entry_safe(fib, next_fib, &g_fib_inv_hash_by_gtw[hash_by_gtw], list_by_gtw)
    {
        if(fib->family == family
            && tbs_nfp_l3_addr_eq(family, next_hop, fib->def_gtw_l3))
        {
            memcpy(fib->da, mac, sizeof(fib->da));
            tbs_nfp_fib_valid(fib, TBS_NFP_F_FIB_ARP_INV);
        }
    }

    return;
}


/*=========================================================================
 Function:		inline void tbs_nfp_fib_arp_invalid(int family, const char *dip)

 Description:		����·�ɻ�����Ŀ��ipΪdip��·�ɻ����־ΪTBS_NFP_F_FIB_ARP_INV
 Data Accessed:     struct list_head g_fib_hash[]
 Data Updated:
 Input:			    int family  Э����ipv4/6
                    const char *next_hop ��һ��ip
 Output:
 Return:			0:�ɹ�  ����:ʧ��
 Others:            ������µ�fib��¼��g_fib_hash�У������Ƶ�g_fib_inv_hash
=========================================================================*/
inline void tbs_nfp_fib_arp_invalid(int family, const unsigned char *next_hop)
{
    TBS_NFP_RULE_FIB *fib, *fib_next;

    int hash_by_gtw = tbs_nfp_fib_hash_gtw(family, (unsigned int *)next_hop, true);
    int hash_inv_by_gtw = tbs_nfp_fib_hash_gtw(family, (unsigned int *)next_hop, false);

     TBS_NFP_INTO_FUNC;
    /*����g_fib_hash*/
    list_for_each_entry_safe(fib, fib_next, &g_fib_hash_by_gtw[hash_by_gtw], list_by_gtw)
    {
        if(tbs_nfp_l3_addr_eq(family, next_hop, fib->def_gtw_l3))
        {
            memset(fib->da, 0, sizeof(fib->da));
            tbs_nfp_fib_invalid(fib, TBS_NFP_F_FIB_ARP_INV);
        }
    }

    /*����g_fib_inv_hash*/
    list_for_each_entry_safe(fib, fib_next, &g_fib_inv_hash_by_gtw[hash_inv_by_gtw], list_by_gtw)
    {
        if(tbs_nfp_l3_addr_eq(family, next_hop, fib->def_gtw_l3) &&
            !(fib->flags & TBS_NFP_F_FIB_ARP_INV))
        {
            memset(fib->da, 0, sizeof(fib->da));
            tbs_nfp_fib_invalid(fib, TBS_NFP_F_FIB_ARP_INV);
        }
    }

    return;
}

/*=========================================================================
 Function:		int tbs_nfp_fib_delete(int family, const char *sip,
    const char *dip, const char *gtw, int oif)

 Description:		��hash����ɾ��ָ��·�ɻ������
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    int family  Э����ipv4/6
                    const char *sip Դip
                    const char *dip Ŀ��ip
                    const char *gtw ���ص�ַ
                    int oif ���ӿ�
 Output:
 Return:			0:�ɹ�  ����:ʧ��
 Others:
=========================================================================*/
int tbs_nfp_fib_add(int family, const unsigned char *sip, const unsigned char *dip,
    const unsigned char *gtw, int oif)
{
    TBS_NFP_RULE_FIB *new_entry = NULL;
    TBS_NFP_RULE_FIB *old_entry = NULL;
    unsigned int hash;
    unsigned int hash_by_gtw;
	int ip_len;

    TBS_NFP_RULE_ARP *arp_rule = NULL;
    TBS_NFP_IF_MAP *if_map = NULL;
    unsigned int addr_len = (family == AF_INET)?4:16;
    int ret = TBS_NFP_ERR;

#ifdef TBS_NFP_BRIDGE
    TBS_NFP_RULE_BRIDGE *bridge_rule = NULL;
#endif

	//�����������ܳ��� g_fib_max_size
	if(g_fib_max_size == tbs_nfp_fib_rule_num)
	{
        TBS_NFP_DEBUG("tbs_nfp_fib_rule_num(%u) == g_fib_max_size\n",
            tbs_nfp_fib_rule_num);

		return TBS_NFP_ERR;
	}

    //�½�����ӹ��� ,Ϊ�˼������д�����������ʼ���ŵ���ǰ��
    new_entry = kmem_cache_alloc(tbs_nfp_fib_rule_cache, GFP_ATOMIC);
    if(NULL == new_entry)
    {
        TBS_NFP_ERROR("kmalloc new TBS_NFP_RULE_FIB faild \n");
        return TBS_NFP_ERR;
    }

	if(AF_INET == family) //ipv4
	{
		ip_len = TBS_NFP_MAX_IPV4_ADDR_SIZE;
	}
	else //ipv6
	{
		ip_len = TBS_NFP_MAX_L3_ADDR_SIZE;
	}

    memset(new_entry, 0, sizeof(*new_entry));
    new_entry->family = family;
	new_entry->oif    = oif;
    new_entry->flags |= TBS_NFP_F_FIB_ARP_INV;
    new_entry->ref    = 1;
    memcpy(new_entry->src_l3, sip, ip_len);
    memcpy(new_entry->dst_l3, dip, ip_len);
    memcpy(new_entry->def_gtw_l3, gtw, ip_len);

    memcpy(new_entry->src_l3, sip, addr_len);
    memcpy(new_entry->dst_l3, dip, addr_len);
    memcpy(new_entry->def_gtw_l3, gtw, addr_len);

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
    //����Ƿ��Ѵ��ڹ���
    old_entry = tbs_nfp_fib_lookup(family, sip, dip, true);
    if(NULL == old_entry)
    {
        old_entry = tbs_nfp_fib_lookup(family, sip, dip, false);
    }

    //����Ƿ��Ѵ��ڹ���
    if (old_entry)
    {
        //�Ϲ�����Ҫɾ��
        tbs_nfp_ct_fib_invalid(old_entry);

        BUG_ON(old_entry->ref > 1);

        list_del(&old_entry->list);
        list_del(&old_entry->list_by_gtw);

        tbs_nfp_fib_rule_num--;
        kmem_cache_free(tbs_nfp_fib_rule_cache, old_entry);
		old_entry = NULL;
    }

    //�����ӿ��Ƿ���ڣ�����ȡimap
    if_map = tbs_nfp_ifmap_get(oif);
    if (NULL == if_map)
    {
        kmem_cache_free(tbs_nfp_fib_rule_cache, new_entry);

        ret = TBS_NFP_ERR;
        goto end;
    }

    //��ʼ��ԴmacΪ���ӿڵ�mac
    memcpy(new_entry->sa, if_map->mac, sizeof(new_entry->sa));

    //���ӿ���dummyport,ȫ���滻Ϊ�丸�豸
#ifdef TBS_NFP_DUMMYPORT
    if((if_map->flags & TBS_NFP_F_MAP_DUMMYPORT))
    {
        if (NULL == if_map->parent_if)
        {
            kmem_cache_free(tbs_nfp_fib_rule_cache, new_entry);
            ret = TBS_NFP_ERR;
            goto end;
        }

#ifdef TBS_NFP_BRIDGE
        if (if_map->parent_if->flags & TBS_NFP_F_MAP_BRIDGE)
        {
            /*fib����TBS_NFP_F_FIB_BRIDGE_INV, fib->oif = if_map->parent_if->ifidx*/
            new_entry->oif = if_map->parent_if->ifidx;
            new_entry->flags |= TBS_NFP_F_FIB_BRIDGE_INV;
        }
        else
#endif  /*TBS_NFP_BRIDGE*/
        {
            new_entry->oif = if_map->parent_if->ifidx;
        }
    }
    else
#endif  /*TBS_NFP_DUMMYPORT*/

#ifdef TBS_NFP_BRIDGE
	if(if_map->flags & TBS_NFP_F_MAP_BRIDGE)
    {
        //���ӿ������豸
        new_entry->oif = if_map->ifidx;
        new_entry->flags |= TBS_NFP_F_FIB_BRIDGE_INV;
    }
    else
#endif  /*TBS_NFP_BRIDGE*/

#ifdef TBS_NFP_PPP
    //���ӿ���ppp�豸,����ѯarp��ֱ�Ӵ�ifmap�л�ȡ�Զ˵�MAC��ַ
    if(if_map->flags & TBS_NFP_F_MAP_PPPOE)
    {
        memcpy(new_entry->da, if_map->remote_mac, sizeof(new_entry->da));
        memcpy(new_entry->sa, if_map->parent_if->mac, sizeof(new_entry->sa));

        goto fib_valid_rule;
    }
#endif  /*TBS_NFP_PPP*/

    arp_rule = tbs_nfp_arp_lookup(family, gtw);
    if (!arp_rule)
    {
        goto fib_invalid_rule;
    }
    else
    {
        memcpy(new_entry->da, arp_rule->da, sizeof(new_entry->da));
        new_entry->flags &= ~TBS_NFP_F_FIB_ARP_INV;

#ifdef TBS_NFP_BRIDGE
         /*������ӿ����Žӿ�(��ǰ�ų��ڳ�ʼ��ʱΪ������)*/
        if (new_entry->flags & TBS_NFP_F_FIB_BRIDGE_INV)
        {
            bridge_rule = tbs_nfp_bridge_rule_lookup(new_entry->da, if_map->mac, new_entry->oif);
            if (!bridge_rule)
            {
                /*δ�ҵ���ת�����򣬽�fib�����������hash��*/
                 goto fib_invalid_rule;
            }
            else
            {  //�ҵ���ת������,���ĳ��ӿ�Ϊ��ת�����ӿ�
                new_entry->oif = bridge_rule->oif;
            }
        }
#endif /*TBS_NFP_BRIDGE*/
    }

    /*���ӿ���1)����ӿ�
              2)VLAN�ӿ�,
              3)�Žӿ����ҵ���ת������
              4)ppp�ӿ�
      ��fib�������������fib hash���У�������ct����
    */
#ifdef TBS_NFP_PPP
fib_valid_rule:
#endif  /*TBS_NFP_PPP*/

    hash = 	tbs_nfp_fib_hash(family, (unsigned int *)sip, (unsigned int *)dip, true);
    new_entry->flags &= ~TBS_NFP_F_FIB_INV_MASK;
    list_add(&new_entry->list,&g_fib_hash[hash]);

    hash_by_gtw = tbs_nfp_fib_hash_gtw(family, (unsigned int *)gtw, true);
    list_add(&new_entry->list_by_gtw, &g_fib_hash_by_gtw[hash_by_gtw]);

    tbs_nfp_fib_rule_num++;
    tbs_nfp_ct_fib_update(new_entry);

    ret = TBS_NFP_OK;
    goto end;

fib_invalid_rule:
    hash = 	tbs_nfp_fib_hash(family,(unsigned int *)sip, (unsigned int *)dip, false);
    list_add(&new_entry->list, &g_fib_inv_hash[hash]);

    hash_by_gtw = tbs_nfp_fib_hash_gtw(family, (unsigned int *)gtw, false);
    list_add(&new_entry->list_by_gtw, &g_fib_inv_hash_by_gtw[hash_by_gtw]);
    tbs_nfp_fib_rule_num++;

    ret = TBS_NFP_OK;

end:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}


/*=========================================================================
 Function:		int tbs_nfp_fib_delete(int family, const char *sip, const char *dip)

 Description:		��hash����ɾ��ָ��·�ɻ������
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    int family  Э����ipv4/6
                    const char *sip Դip
                    const char *dip Ŀ��ip
 Output:
 Return:			0:�ɹ�  ����:ʧ��
 Others:
=========================================================================*/
inline int tbs_nfp_fib_delete(int family, const unsigned char *sip, const unsigned char *dip)
{
    TBS_NFP_RULE_FIB *entry = NULL;
    int ret = TBS_NFP_ERR;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);
    //����Ƿ��Ѵ��ڹ���

    entry = tbs_nfp_fib_lookup(family, sip, dip, true);
	if(NULL == entry)
		entry = tbs_nfp_fib_lookup(family, sip, dip, false);
	else
        tbs_nfp_ct_fib_invalid(entry);     /* ɾ��fib������invalid ct ���� */

    if (entry)
    {
        BUG_ON(entry->ref > 1);

        list_del(&entry->list);
        list_del(&entry->list_by_gtw);
        kmem_cache_free(tbs_nfp_fib_rule_cache, entry);

        ret = TBS_NFP_OK;
        tbs_nfp_fib_rule_num--;
        goto end;
    }

end:
    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return ret;
}


/*=========================================================================
 Function:		int tbs_nfp_fib_age(int family, const char *sip, const char *dip)

 Description:		��ѯ·�ɻ������ƥ�������
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    int family  Э����ipv4/6
                    const char *sip Դip
                    const char *dip Ŀ��ip
 Output:
 Return:			0:�ɹ�  ����:ʧ��
 Others:
=========================================================================*/
int tbs_nfp_fib_age(int family, const unsigned char *sip, const unsigned char *dip)
{
    TBS_NFP_RULE_FIB *rule;
    unsigned int count = 0;

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);

    rule = tbs_nfp_fib_lookup(family, sip, dip, true);
    if(NULL == rule)
	{
		rule = tbs_nfp_fib_lookup(family, sip, dip, false);
		if(NULL == rule)
			goto out;
	}
    count = rule->count;
	rule->count = 0;

out:
    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);
    return count ;
}

/*=========================================================================
 Function:		static void tbs_nfp_fib_flush(void)

 Description:		ɾ������fibת������
 Data Accessed:     g_fib_hash/g_fib_inv_hash
 Data Updated:
 Input:			    ��
 Output:			��
 Return:
 Others:
=========================================================================*/
static void tbs_nfp_fib_flush(void)
{
    unsigned long hash = 0;
    TBS_NFP_RULE_FIB *tmp_entry = NULL;
    TBS_NFP_RULE_FIB *next_entry = NULL;

    TBS_NFP_INTO_FUNC;

    //���α�����ɾ������g_fib_hash��g_fib_hash_by_gtw�й���Ҳһ����ɾ��
    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_fib_hash); hash++)
    {
        if(list_empty(&(g_fib_hash[hash])))
            continue;

        list_for_each_entry_safe(tmp_entry, next_entry, &g_fib_hash[hash], list)
        {
            tbs_nfp_ct_fib_invalid(tmp_entry);
            BUG_ON(tmp_entry->ref > 1);

            list_del(&tmp_entry->list);
            list_del(&tmp_entry->list_by_gtw);
            kmem_cache_free(tbs_nfp_fib_rule_cache, tmp_entry);
            tbs_nfp_fib_rule_num--;
        }
    }

    //���α�����ɾ������g_fib_inv_hash��g_fib_inv_hash_by_gtw�й���Ҳһ����ɾ��
    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_fib_inv_hash); hash++)
    {
        if(list_empty(&(g_fib_inv_hash[hash])))
            continue;

        list_for_each_entry_safe(tmp_entry, next_entry, &g_fib_inv_hash[hash], list)
        {
            BUG_ON(tmp_entry->ref > 1);

            list_del(&tmp_entry->list);
            list_del(&tmp_entry->list_by_gtw);
            kmem_cache_free(tbs_nfp_fib_rule_cache, tmp_entry);
            tbs_nfp_fib_rule_num--;
        }
    }

    return;
}


/*=========================================================================
 Function:		void tbs_nfp_fib_reset(void)

 Description:		�������fib����
 Data Accessed:     g_fib_hash/g_fib_inv_hash
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			��
 Others:
=========================================================================*/
void tbs_nfp_fib_reset(void)
{
    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    tbs_nfp_fib_flush();

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return;
}
/*=========================================================================
 Function:		void tbs_nfp_fib_exit(void)

 Description:		������ת������hash�����ڼ�����ģ���˳�
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			0:�ɹ�  ����:ʧ��
 Others:
=========================================================================*/
void tbs_nfp_fib_exit(void)
{
    //����ת������
    tbs_nfp_fib_reset();

    //�ͷ��ڴ�cache
    kmem_cache_destroy(tbs_nfp_fib_rule_cache);
    return;
}


/*=========================================================================
 Function:		void tbs_nfp_fib_init(void)

 Description:		��ʼ��fib����hash�����ڼ�����ģ�����
 Data Accessed:     struct list_head g_fib_hash[]/g_fib_inv_hash[];
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			0:�ɹ�  ����:ʧ��
 Others:
=========================================================================*/
int tbs_nfp_fib_init(void)
{
    int i = 0;
    int ret = 0;

    //�ڴ�Ԥ���仺����
    tbs_nfp_fib_rule_cache = kmem_cache_create("tbs_nfp_fib_rule_cache",\
                                sizeof(struct tbs_nfp_rule_fib),\
                                0,\
                                SLAB_HWCACHE_ALIGN, NULL);
    if (!tbs_nfp_fib_rule_cache)
    {
        TBS_NFP_ERROR("Fail to kmem hash cache\n");
        ret = -ENOMEM;
        goto out;
    }

    //��ʼ��hash��
    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_hash); i++)
    {
        INIT_LIST_HEAD(&g_fib_hash[i]);
    }

    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_hash_by_gtw); i++)
    {
        INIT_LIST_HEAD(&g_fib_hash_by_gtw[i]);
    }

    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_inv_hash); i++)
    {
        INIT_LIST_HEAD(&g_fib_inv_hash[i]);
    }

    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_fib_inv_hash_by_gtw); i++)
    {
        INIT_LIST_HEAD(&g_fib_inv_hash_by_gtw[i]);
    }

out:
    return ret;

}


