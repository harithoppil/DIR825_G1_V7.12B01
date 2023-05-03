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
/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/

/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
struct list_head g_arp_hash[TBS_NFP_ARP_HASH_SIZE];
static struct kmem_cache *tbs_nfp_arp_rule_cache __read_mostly;
unsigned int g_arp_max_size __read_mostly = TBS_NFP_ARP_RULE_MAX;
static unsigned int  tbs_nfp_arp_rule_num = 0;


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:		static inline void tbs_nfp_arp_printk(const TBS_NFP_RULE_ARP *arp)

 Description:		��ӡarp����
 Data Accessed:     g_arp_hash[]

 Data Updated:       ��
 Input:			    unsigned int hash_key       arp �����hask key
                    const TBS_NFP_RULE_ARP *arp arp����ָ��
 Output:			��
 Return:			��

 Others:
=========================================================================*/
static inline void tbs_nfp_arp_printk(unsigned int hash_key,const TBS_NFP_RULE_ARP *arp)
{
    if(AF_INET == arp->family)
    {
        printk("%u\t%pI4\t\t\t\t"
        "%02x:%02x:%02x:%02x:%02x:%02x\n",
        hash_key, arp->next_hop_l3,
        arp->da[0], arp->da[1],
        arp->da[2], arp->da[3],
        arp->da[4], arp->da[5]);
    }
    else
    {
        printk("%u\t%pI6\t"
        "%02x:%02x:%02x:%02x:%02x:%02x\n",
        hash_key,arp->next_hop_l3,
        arp->da[0], arp->da[1],
        arp->da[2], arp->da[3],
        arp->da[4], arp->da[5]);
    }
    return;
}


/*=========================================================================
 Function:		void tbs_nfp_arp_dump(void)

 Description:		��ӡarp hash�����й���
 Data Accessed:     ��

 Data Updated:
 Input:			    ��
 Output:			��
 Return:			��

 Others:
=========================================================================*/
void tbs_nfp_arp_dump(void)
{
    unsigned int hash = 0;
    TBS_NFP_RULE_ARP *tmp_entry;

    printk("arp_rule_num:%u\n", tbs_nfp_arp_rule_num);
    printk("hash\tgtw_ip\t\t\t\t\tdst_mac\n");

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);
    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_arp_hash); hash++)
    {
        list_for_each_entry(tmp_entry, &g_arp_hash[hash], list)
        {
            tbs_nfp_arp_printk(hash, tmp_entry);
        }
    }

    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);
    return ;
}


/*=========================================================================
 Function:		static inline int tbs_nfp_arp_cmp(int family,
    const unsigned char *nextHopL3, TBS_NFP_RULE_ARP *rule)

 Description:		arp����Ƚ�
 Data Accessed:     nfp_route_table nfp_route_table_by_mac

 Data Updated:
 Input:			    int family              Э����
                    const unsigned char *nextHopL3  ��һ��ip��ַ
                    TBS_NFP_RULE_ARP *rule  arp����
 Output:			��
 Return:			true:   ��ͬ    false:��ͬ

 Others:
=========================================================================*/
static inline int tbs_nfp_arp_cmp(int family, const unsigned char *nextHopL3, TBS_NFP_RULE_ARP *rule)
{
    if ((family == rule->family) && tbs_nfp_l3_addr_eq(family, rule->next_hop_l3, nextHopL3))
        return true;

    return false;
}


inline TBS_NFP_RULE_ARP *tbs_nfp_arp_lookup(int family, const unsigned char *nextHopL3)
{
    unsigned int hash;
    TBS_NFP_RULE_ARP *rule;

    hash = tbs_nfp_arp_hash(family, (const unsigned int *)nextHopL3);
    if(list_empty(&g_arp_hash[hash]))
        return NULL;

    list_for_each_entry(rule, &g_arp_hash[hash], list) {
        if (tbs_nfp_arp_cmp(family, nextHopL3, rule))
            return rule;
    }
    return NULL;
}

/*=========================================================================
 Function:		static bool tbs_nfp_arp_exist(int family, unsigned char *nextHopL3)

 Description:		ARP�����Ƿ����
 Data Accessed:     struct list_head g_arp_hash[TBS_NFP_ARP_HASH_SIZE];
 Data Updated:
 Input:			    int family,               Э������ipv4/ipv6
                    unsigned char *nextHopL3  ��һ����ַ
 Output:			��
 Return:			true:   ���ڣ�flse: ������
 Others:
=========================================================================*/
inline bool tbs_nfp_arp_exist(int family, const unsigned char *nextHopL3)
{
    bool exist = false;

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);

    if(tbs_nfp_arp_lookup(family,nextHopL3))
        exist = true;

    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

    return exist;
}

/*=========================================================================
 Function:		int tbs_nfp_arp_add(int family, const unsigned char *next_hop,
    const unsigned char *mac)

 Description:		���arpת������
 Data Accessed:     struct list_head g_arp_hash[]/g_arp_inv_hash[];
 Data Updated:
 Input:			    int family  Э����ipv4/6
                    const unsigned char *next_hop   ��һ��ip��ַ
                    const unsigned char *mac        ��һ����ַ��Ӧ��mac��ַ
 Output:
 Return:			0:�ɹ�  ����:ʧ��
 Others:
=========================================================================*/
int tbs_nfp_arp_add(int family, const unsigned char *next_hop,
                    const unsigned char *mac)
{
    unsigned int hash;
    TBS_NFP_RULE_ARP *new_entry;

    if(true == tbs_nfp_arp_exist(family,next_hop))
    {
       return TBS_NFP_OK;
    }

    //�����������ܳ��� g_arp_max_size
    if (g_arp_max_size == tbs_nfp_arp_rule_num)
    {
        TBS_NFP_DEBUG("tbs_nfp_arp_rule_num(%u) == g_arp_max_size\n",
            tbs_nfp_arp_rule_num);

        return TBS_NFP_ERR;
    }

    new_entry = kmem_cache_alloc(tbs_nfp_arp_rule_cache, GFP_ATOMIC);
    if(NULL == new_entry)
    {
        TBS_NFP_ERROR("kmalloc new TBS_NFP_RULE_arp faild \n");
        return TBS_NFP_ERR;
    }

    hash = 	tbs_nfp_arp_hash(family, (const unsigned int *)next_hop);
    memset(new_entry, 0, sizeof(*new_entry));

    new_entry->family = family;
    memcpy(new_entry->next_hop_l3,next_hop,sizeof(new_entry->next_hop_l3));
    memcpy(new_entry->da,mac,sizeof(new_entry->da));

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    list_add(&new_entry->list,&g_arp_hash[hash]);
    tbs_nfp_fib_arp_update(family, next_hop, mac);

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    tbs_nfp_arp_rule_num++;

    return TBS_NFP_OK;
}


/*=========================================================================
 Function:		int tbs_nfp_arp_delete(int family, const unsigned char *next_hop)

 Description:		ɾ��arpת������
 Data Accessed:     struct list_head g_arp_hash[]/g_arp_inv_hash[];
 Data Updated:
 Input:			    int family  Э����ipv4/6
                    const unsigned char *next_hop   ��һ��ip��ַ
 Output:
 Return:			0:�ɹ�  ����:ʧ��
 Others:
=========================================================================*/
int tbs_nfp_arp_delete(int family, const unsigned char *next_hop)
{
    TBS_NFP_RULE_ARP *entry;

    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    entry = tbs_nfp_arp_lookup(family,next_hop);
    if(entry)
    {
        TBS_NFP_DEBUG("del arp rule\n");
        list_del(&entry->list);
        kmem_cache_free(tbs_nfp_arp_rule_cache,entry);
        tbs_nfp_arp_rule_num--;
    }

    tbs_nfp_fib_arp_invalid(family, next_hop);

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);
    return TBS_NFP_OK;
}

/*=========================================================================
 Function:		int tbs_nfp_arp_age(int family, const unsigned char *next_hop)

 Description:		��ѯarpת������ƥ�����
 Data Accessed:     struct list_head g_arp_hash[]
 Data Updated:
 Input:			    int family  Э����ipv4/6
                    const unsigned char *next_hop   ��һ��ip��ַ
 Output:
 Return:			С��0:  ʧ��,   ����:   ƥ�����
 Others:
=========================================================================*/
int tbs_nfp_arp_age(int family, const unsigned char *next_hop)
{
    return 0;
}
/*=========================================================================
 Function:		static void tbs_nfp_arp_flush(void)

 Description:		�������arp����
 Data Accessed:     g_arp_hash/g_arp_inv_hash
 Data Updated:
 Input:			    ��
 Output:			��
 Return:
 Others:
=========================================================================*/
static void tbs_nfp_arp_flush(void)
{
    unsigned long hash = 0;
    TBS_NFP_RULE_FIB *tmp_entry = NULL;
    TBS_NFP_RULE_FIB *next_entry = NULL;

    TBS_NFP_INTO_FUNC;

//���α�����ɾ������g_fib_hash �й���Ҳһ����ɾ��
    for (hash = 0; hash < TBS_NFP_ARRAY_SIZE(g_arp_hash); hash++)
    {
        if(list_empty(&(g_arp_hash[hash])))
            continue;

        list_for_each_entry_safe(tmp_entry, next_entry, &g_arp_hash[hash], list)
        {
            tbs_nfp_fib_arp_invalid(tmp_entry->family,tmp_entry->def_gtw_l3);
            list_del(&tmp_entry->list);
            kmem_cache_free(tbs_nfp_arp_rule_cache, tmp_entry);
            tbs_nfp_arp_rule_num--;
        }
    }

}


/*=========================================================================
 Function:		void tbs_nfp_arp_reset(void)

 Description:		�������fib����
 Data Accessed:     g_arp_hash
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			��
 Others:
=========================================================================*/
void tbs_nfp_arp_reset(void)
{
    TBS_NFP_WRITE_LOCK(&tbs_nfp_lock);

    tbs_nfp_arp_flush();

    TBS_NFP_WRITE_UNLOCK(&tbs_nfp_lock);

    return;
}
/*=========================================================================
 Function:		void tbs_nfp_fib_exit(void)

 Description:		������ת������hash�����ڼ�����ģ���˳�
 Data Accessed:     struct list_head g_arp_hash[]
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			0:�ɹ�  ����:ʧ��
 Others:
=========================================================================*/
void tbs_nfp_arp_exit(void)
{
    //����ת������
    tbs_nfp_arp_reset();

    //�ͷ��ڴ�cache
    kmem_cache_destroy(tbs_nfp_arp_rule_cache);
    return;
}


/*=========================================================================
 Function:		void tbs_nfp_arp_init(void)

 Description:		��ʼ��fib����hash�����ڼ�����ģ�����
 Data Accessed:     struct list_head g_arp_hash[]
 Data Updated:
 Input:			    ��
 Output:			��
 Return:			0:�ɹ�  ����:ʧ��
 Others:
=========================================================================*/
int tbs_nfp_arp_init(void)
{
    int i = 0;
    int ret = 0;

    //�ڴ�Ԥ���仺����
    tbs_nfp_arp_rule_cache = kmem_cache_create("tbs_nfp_arp_rule_cache",\
                                sizeof(struct tbs_nfp_rule_arp),\
                                0,\
                                SLAB_HWCACHE_ALIGN, NULL);
    if (!tbs_nfp_arp_rule_cache)
    {
        TBS_NFP_ERROR("Fail to kmem hash cache\n");
        ret = -ENOMEM;
        goto out;
    }

    //��ʼ��hash��
    for(i = 0; i < TBS_NFP_ARRAY_SIZE(g_arp_hash); i++)
    {
        INIT_LIST_HEAD(&g_arp_hash[i]);
    }

out:
    return ret;

}


