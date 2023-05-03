/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : nfp_bridge.c
 �ļ����� : tbs nfp ����


 �޶���¼ :
          1 ���� : pengyao
            ���� : 2011-06-16
            ���� :
=========================================================================*/
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/notifier.h>
#include <linux/if_bridge.h>
#include <linux/netfilter_bridge.h>
#include <linux/../../net/bridge/br_private.h>
#include <linux/netdevice.h>
#include <linux/list.h>
#include <linux/if.h>
#include <linux/net.h>
#include <linux/spinlock.h>
#include <asm/irq.h>

#include "nfp_bridge.h"
#include "nfp_adapter.h"
#include "nfp_interface.h"
#include "module_stat.h"
#include "autoconf.h"


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
#define USE_SPIN_LOCK   1

#define BRIDGE_RULE_DELAY   (HZ)
#define FDB_RULE_DELAY      (HZ)


/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/

static PL2Bridge_entry_tab bt_hash[NFP_BRIDGE_HTABLE_SIZE] = {0};

static struct kmem_cache *nfp_bridge_cache __read_mostly;
static struct kmem_cache *nfp_fdb_cache __read_mostly;

//����������
static unsigned int  bridge_entry_num = 0;
static unsigned int  fdb_rule_num = 0;

/*��ֹƵ����ӹ���*/
//static unsigned long bridge_ageing_time = 0;
//static unsigned long fdb_ageing_time = 0;

/*fdb�����д��*/

static spinlock_t fdb_hash_lock;
static spinlock_t br_hash_lock;

/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/
static int nfp_fdb_acp_entry_delete(struct fdb_entry_tab *rule);
static void nfp_bridge_entry_timeout(unsigned long data);


static __inline unsigned char HASH_L2BRIDGE(void *da,
        void *sa,
        unsigned short ethertype,
        unsigned short input_interface,
        unsigned short output_interface)
{
    unsigned short sum = 0;
    unsigned char hash = 0;
    unsigned short *pda = da;
    unsigned short *psa = sa;

    sum += pda[0] + pda[1] + pda[2];
    sum += psa[0] + psa[1] + psa[2];
    //sum += ethertype + input_interface + output_interface;
    hash = (sum >> 8) ^ (sum & 0xFF);
    return (hash ^ (hash >> 4)) & (NFP_BRIDGE_HTABLE_SIZE - 1);
}

static __inline int TESTEQ_MACADDR2(void *pmacaddr1, void *pmacaddr2)
{
    unsigned short *p1;
    unsigned short *p2;
    p1 = (unsigned short *)pmacaddr1;
    p2 = (unsigned short *)pmacaddr2;
    return p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2];
}


static inline int nfp_bridge_option(unsigned short option, PL2Bridge_entry pEntry)
{
    int ret = NFP_UNABLE_PARSER;

    if(nfp_cmd_parser){
        struct nfp_adapter_cmd cmd_entry = {option, (void *)pEntry};
        ret = nfp_cmd_parser(NFP_CMD_BRIDGE, &cmd_entry);
    }
    else if(NFP_OPT_STAT == option)
        ret = NFP_RULE_TIMEOUT;

    return ret;
}


/*=========================================================================
 Function:		static int nfp_bridge_stats( const PL2Bridge_entry pEntry )
 Description:	��ȡָ���ļ���������״̬
 Data Accessed:
 Data Updated:
 Input:			pEntry: Ҫ��ѯ�Ĺ���
 Output:		��
 Return:		1:�˼�������������
                            0:�˼����������Ѿ�����
				-1:��ѯʧ�ܻ�����
 Others:
=========================================================================*/
static int nfp_bridge_stats(PL2Bridge_entry pEntry)
{
    if(NULL == pEntry){
        NFP_ADAPTER_ERROR("Args error\n");
        return -1;
    }

    return nfp_bridge_option(NFP_OPT_STAT, pEntry);
}


static bool nfp_bridge_valid_entry(PL2Bridge_entry pentry)
{
    if(is_zero_ether_addr(pentry->sa) || is_zero_ether_addr(pentry->da))
        return false;

    return true;
}


/*=========================================================================
 Function:		static int nfp_bridge_add( const PL2Bridge_entry pEntry )
 Description:	����������FPP���
 Data Accessed:
 Data Updated:
 Input:			pEntry: Ҫ��ӵ��¹���
 Output:		��
 Return:		0:�ɹ�
				-1:�����Ѿ�����,���ʧ�ܻ�����
 Others:
=========================================================================*/
static int nfp_bridge_add(const PL2Bridge_entry pEntry)
{
    int ret = 0;

    if(NULL == pEntry){
        NFP_ADAPTER_ERROR("Args error\n");
        return -1;
    }

    ret = nfp_bridge_option(NFP_OPT_ADD, pEntry);
    if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
    {
        NFP_ADAPTER_ERROR("call nfp_bridge_option(NFP_OPT_ADD) error:%d\n", ret);
    }

    return ret ;
}



/*=========================================================================
 Function:		static int nfp_bridge_delete( const PL2Bridge_entry pEntry )
 Description:	����������FPPɾ��
 Data Accessed:
 Data Updated:
 Input:			pEntry: Ҫɾ���Ĺ���
 Output:		��
 Return:		0:�ɹ�
				-1������:ɾ��ʧ��
 Others:
=========================================================================*/
static int nfp_bridge_delete(const PL2Bridge_entry pEntry)
{
    int ret = 0;

    if(NULL == pEntry){
        NFP_ADAPTER_ERROR("Args error\n");
        return -1;
    }

    ret = nfp_bridge_option(NFP_OPT_DELETE, pEntry);
    if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
    {
        NFP_ADAPTER_ERROR("call nfp_bridge_option(NFP_OPT_DELETE) error:%d\n", ret);
    }

    return ret;
}


inline static void br_timer_init( PL2Bridge_entry_tab br_entry )
{
    setup_timer(&(br_entry->timeout), nfp_bridge_entry_timeout, (unsigned long)br_entry);
    mod_timer(&(br_entry->timeout) , jiffies + net_random()%BRIDGE_ENTRY_TIMEOUT + BRIDGE_ENTRY_TIMEOUT);
}

inline static void br_timer_exit( struct timer_list *timeout )
{
    //del_timer_sync(timeout);
    del_timer(timeout);
}


static int nfp_bridge_entry_free(PL2Bridge_entry_tab pEntryTab ,unsigned long hash)
{
    PL2Bridge_entry_tab pEntryTabPre;

    PL2Bridge_entry_tab pEntryTabTmp = NULL;

    if(NULL == pEntryTab){
        NFP_ADAPTER_ERROR("Args error\n");
        return -1;
    }

    pEntryTabTmp = bt_hash[hash];
    pEntryTabPre = pEntryTabTmp;
    while(NULL != pEntryTabTmp && pEntryTabTmp != pEntryTab) {

        pEntryTabPre = pEntryTabTmp;
        pEntryTabTmp = pEntryTabTmp->next;
    }

    /*�ҵ��ڵ�*/
    if(pEntryTabTmp) {
		if(pEntryTabTmp != pEntryTab )
			return -1;

        /*�׽ڵ�*/
        if(bt_hash[hash] == pEntryTabTmp) {
            bt_hash[hash] = pEntryTabTmp->next;
        }
        else {
            pEntryTabPre->next = pEntryTabTmp->next;
        }

        /*�ͷŽڵ�*/
        br_timer_exit(&pEntryTab->timeout);

        if (NULL != pEntryTab) {
            kmem_cache_free(nfp_bridge_cache, pEntryTab);
            pEntryTab = NULL;

            /*�����������1*/
            bridge_entry_num--;
        }
    }
    else/*û�ҵ����*/
    {
        /*nothing to do*/
        return -1;
    }

    return 0;
}


static void nfp_bridge_entry_timeout(unsigned long data)
{
    unsigned long hash = 0;
    PL2Bridge_entry_tab pEntryTab = NULL;
	PL2Bridge_entry     pEntry = NULL;

	pEntryTab = (PL2Bridge_entry_tab)data;
	NFP_ASSERT(pEntryTab );

    if (unlikely(!spin_trylock_bh(&br_hash_lock)))
    {
        mod_timer(&(pEntryTab->timeout) , jiffies + net_random()%BRIDGE_ENTRY_TIMEOUT + BRIDGE_ENTRY_TIMEOUT);
        NFP_ADAPTER_ERROR("spin_trylock_irqsave() faild\n");
        return ;
    }

	pEntry  = &(pEntryTab->direction[LEFT_DIRECTION]);
    hash = HASH_L2BRIDGE(pEntry->da, pEntry->sa , pEntry->ethertype,
                     pEntry->input_interface , pEntry->output_interface);

    if((!nfp_bridge_valid_entry(&(pEntryTab->direction[LEFT_DIRECTION]))
            ||(NFP_RULE_TIMEOUT == nfp_bridge_stats(&(pEntryTab->direction[LEFT_DIRECTION]))))
          && (!nfp_bridge_valid_entry(&(pEntryTab->direction[RIGHT_DIRECTION]))
            || (NFP_RULE_TIMEOUT == nfp_bridge_stats(&(pEntryTab->direction[RIGHT_DIRECTION]))))){
        NFP_ADAPTER_DEBUG("bridge_entry status: ERR_RULE_TIMEOUT\n");

        /*������,ɾ��bt_hash��Fpp����*/
        if(nfp_bridge_valid_entry(&(pEntryTab->direction[LEFT_DIRECTION])))
            nfp_bridge_delete(&(pEntryTab->direction[LEFT_DIRECTION]));

        if(nfp_bridge_valid_entry(&(pEntryTab->direction[RIGHT_DIRECTION])))
            nfp_bridge_delete(&(pEntryTab->direction[RIGHT_DIRECTION]));

        nfp_bridge_entry_free(pEntryTab , hash);

        pEntryTab = NULL;
    }
    else{
        mod_timer(&(pEntryTab->timeout) , jiffies + net_random()%BRIDGE_ENTRY_TIMEOUT + BRIDGE_ENTRY_TIMEOUT);
        NFP_ADAPTER_DEBUG("bridge_entry status: NFP_RULE_ACTIVE\n");
    }

        spin_unlock_bh(&br_hash_lock);

}


/*=========================================================================
 Function:		static int nfp_bridge_acp_entry_add(const PL2Bridge_entry pEntry)

 Description:	���������򻺴�������
 Data Accessed: bt_hash
 Data Updated:  bt_hash
 Input:			pEntry: Ҫ���ҵĹ���
 Output:		��
 Return:		0:�ɹ�
				-1:���򲻴���,���ʧ�ܻ�����
 Others:
=========================================================================*/
static int nfp_bridge_acp_entry_add(const PL2Bridge_entry pEntry)
{
    unsigned long hash = 0;
    PL2Bridge_entry_tab pEntryTabTmp= NULL;
    int DirectionFree = BI_DIRECTION;
    int ret = NFP_ADAPTER_FAULT;
    bool itf_update = false;

    if(NULL == pEntry){
        NFP_ADAPTER_ERROR("Args error\n");
        return ret;
    }

    /*����˹����ϣֵ*/
    hash = HASH_L2BRIDGE(pEntry->da, pEntry->sa , pEntry->ethertype,
                         pEntry->input_interface , pEntry->output_interface);

    spin_lock(&br_hash_lock);
    /*��ѯ�˹����Ƿ��ڻ�����л���Գƹ����Ƿ����*/
    pEntryTabTmp = bt_hash[hash];
    while (NULL != pEntryTabTmp){

        /*�˹����Ƿ��ڻ����һ�����������*/
        if (TESTEQ_MACADDR2(pEntry->da , pEntryTabTmp->direction[LEFT_DIRECTION].da)
                && TESTEQ_MACADDR2(pEntry->sa , pEntryTabTmp->direction[LEFT_DIRECTION].sa))
        {
            if((pEntry->input_ifindex == pEntryTabTmp->direction[LEFT_DIRECTION].input_ifindex)
            && (pEntry->output_ifindex == pEntryTabTmp->direction[LEFT_DIRECTION].output_ifindex))
            {
                ret = NFP_ADAPTER_FAULT;
                goto out;
            }

            /*�ӿڱ����*/
            itf_update = true;
            break;
        }

        /*�˹����Ƿ��ڻ����һ��������Ұ��*/
        if (TESTEQ_MACADDR2(pEntry->da, pEntryTabTmp->direction[RIGHT_DIRECTION].da)
                && TESTEQ_MACADDR2(pEntry->sa, pEntryTabTmp->direction[RIGHT_DIRECTION].sa))
        {
            if((pEntry->input_ifindex == pEntryTabTmp->direction[RIGHT_DIRECTION].input_ifindex)
            && (pEntry->output_ifindex == pEntryTabTmp->direction[RIGHT_DIRECTION].output_ifindex))
            {
                ret = NFP_ADAPTER_FAULT;
                goto out;
            }

            /*�ӿڱ����*/
            itf_update = true;
            break;
        }

        /*�˹���Գƹ����Ƿ��ڻ����һ�����������*/
        if (TESTEQ_MACADDR2(pEntry->sa, pEntryTabTmp->direction[LEFT_DIRECTION].da)
                && TESTEQ_MACADDR2(pEntry->da, pEntryTabTmp->direction[LEFT_DIRECTION].sa))
        {
            if((pEntry->input_ifindex == pEntryTabTmp->direction[LEFT_DIRECTION].output_ifindex)
            && (pEntry->output_ifindex == pEntryTabTmp->direction[LEFT_DIRECTION].input_ifindex))
            {
                DirectionFree = RIGHT_DIRECTION;
            }
            else {
                /*�ӿڱ����*/
                itf_update = true;
            }

            break;
        }


        /*�˹���Գƹ����Ƿ��ڻ����һ��������Ұ��*/
        if (TESTEQ_MACADDR2(pEntry->sa, pEntryTabTmp->direction[RIGHT_DIRECTION].da)
                && TESTEQ_MACADDR2(pEntry->da, pEntryTabTmp->direction[RIGHT_DIRECTION].sa))
        {
            if((pEntry->input_ifindex == pEntryTabTmp->direction[RIGHT_DIRECTION].output_ifindex)
            && (pEntry->output_ifindex == pEntryTabTmp->direction[RIGHT_DIRECTION].input_ifindex))
            {
                DirectionFree = LEFT_DIRECTION;
            }
            else {
                /*�ӿڱ����*/
                itf_update = true;
            }

            break;
        }

        pEntryTabTmp = pEntryTabTmp->next;
    }

    /*ɾ���ڵ㲢�������*/
    if(itf_update && pEntryTabTmp != NULL)
    {
        if(nfp_bridge_valid_entry(&(pEntryTabTmp->direction[LEFT_DIRECTION])))
            nfp_bridge_delete(&(pEntryTabTmp->direction[LEFT_DIRECTION]));

        if(nfp_bridge_valid_entry(&(pEntryTabTmp->direction[RIGHT_DIRECTION])))
            nfp_bridge_delete(&(pEntryTabTmp->direction[RIGHT_DIRECTION]));

        nfp_bridge_entry_free(pEntryTabTmp , hash);

        DirectionFree = BI_DIRECTION;
    }

    if(BI_DIRECTION == DirectionFree)
    {
        /*����͹���ĶԳƹ�������ڻ������,Ϊ���������½ڵ�*/
        pEntryTabTmp = kmem_cache_alloc(nfp_bridge_cache, GFP_ATOMIC);
        if(NULL == pEntryTabTmp){
            NFP_ADAPTER_ERROR("kmalloc error\n");
            ret = NFP_ADAPTER_FAULT;
            goto out;
        }

        memset(pEntryTabTmp, 0, sizeof(L2Bridge_entry_tab));
        pEntryTabTmp->next = bt_hash[hash];
        bt_hash[hash] = pEntryTabTmp;

        br_timer_init(pEntryTabTmp);

        /*�����������1*/
        bridge_entry_num++;

        /*Ĭ�Ͻ��¹�����뻺���ڵ�����*/
        memcpy(&(pEntryTabTmp->direction[LEFT_DIRECTION]) , pEntry , sizeof(L2Bridge_entry));
    }
    else if(LEFT_DIRECTION == DirectionFree)
    {
        /*��������ڵ�����Ϊ��,���¹����������*/
        memcpy(&(pEntryTabTmp->direction[LEFT_DIRECTION]) , pEntry , sizeof(L2Bridge_entry));
    }
    else
    {
        /*��������ڵ��Ұ��Ϊ��,���¹�������Ұ��*/
        memcpy(&(pEntryTabTmp->direction[RIGHT_DIRECTION]) , pEntry , sizeof(L2Bridge_entry));
    }

	//bridge_ageing_time = jiffies;

    ret = NFP_ADAPTER_SUCCESS;

out:

    spin_unlock(&br_hash_lock);

    return ret;
}


/*=========================================================================
 Function:		static PL2Bridge_entry nfp_bridge_acp_entry_exist(const PL2Bridge_entry pEntry)

 Description:	���������򻺴�������
 Data Accessed: bt_hash
 Data Updated:  bt_hash
 Input:			pEntry: Ҫ���ҵĹ���
 Output:		��
 Return:		0:�ɹ�
				-1:���򲻴���,���ʧ�ܻ�����
 Others:
=========================================================================*/
static bool nfp_bridge_acp_entry_exist(const PL2Bridge_entry pEntry)
{
    unsigned long hash = 0;
    PL2Bridge_entry_tab pEntryTabTmp= NULL;
    bool bentry_exist = false;

    if(NULL == pEntry){
        NFP_ADAPTER_ERROR("Args error\n");
        return false;
    }

    /*����˹����ϣֵ*/
    hash = HASH_L2BRIDGE(pEntry->da, pEntry->sa , pEntry->ethertype,
                         pEntry->input_interface , pEntry->output_interface);

    spin_lock(&br_hash_lock);

    /*��ѯ�˹����Ƿ��ڻ������*/
    pEntryTabTmp = bt_hash[hash];
    while (NULL != pEntryTabTmp){
        /*�˹����Ƿ��ڻ����һ����������߻��Ұ��*/
        if ((TESTEQ_MACADDR2(pEntry->da , pEntryTabTmp->direction[LEFT_DIRECTION].da)
                && TESTEQ_MACADDR2(pEntry->sa , pEntryTabTmp->direction[LEFT_DIRECTION].sa)
                && (pEntry->input_ifindex == pEntryTabTmp->direction[LEFT_DIRECTION].input_ifindex)
                && (pEntry->output_ifindex == pEntryTabTmp->direction[LEFT_DIRECTION].output_ifindex))
                ||(TESTEQ_MACADDR2(pEntry->da, pEntryTabTmp->direction[RIGHT_DIRECTION].da)
                && TESTEQ_MACADDR2(pEntry->sa, pEntryTabTmp->direction[RIGHT_DIRECTION].sa)
                && (pEntry->input_ifindex == pEntryTabTmp->direction[RIGHT_DIRECTION].input_ifindex)
                && (pEntry->output_ifindex == pEntryTabTmp->direction[RIGHT_DIRECTION].output_ifindex)))
        {
            bentry_exist = true;
            break;
        }

        pEntryTabTmp = pEntryTabTmp->next;
    }

    spin_unlock(&br_hash_lock);

    return bentry_exist;
}

/*=========================================================================
 Function:		int nfp_bridge_entry_add_delay(const struct net_bridge_port *to, const struct sk_buff *skb)
 Description:		��������������Ӻ�������
 Data Accessed:     add_entry
                            padd_entry_queue
 Data Updated:      add_entry
 Input:			to
                            skb
 Output:			��
 Return:			0:�ɹ�
				-1:����������Ϸ�
 Others: 			�˺����ǹ���__br_forward��������Ӽ����������ӵ�ʵ��
                            ����__br_forward�����ж��е���,�������ӿں������е���
                            schedule,����Ҫ�ù������еķ�ʽ������֮�佻��,��ֹϵͳ��
                            ��
=========================================================================*/
static int nfp_bridge_entry_add_delay(const struct sk_buff *skb)
{
    int ret = -1;
    unsigned char *dest = NULL;
    unsigned char *src = NULL;
    //struct net_device *real_dev = NULL;
    struct net_device *input_dev = NULL;
    struct net_device *output_dev = NULL;
    struct net_bridge_fdb_entry	*fdb_entry = NULL;
    struct net_bridge  *br1 = NULL;
    //struct net_bridge_port *p = NULL;
    L2Bridge_entry BridgeEntry;

    /*��ֹƵ���ӹ���*/
    //if(time_after(bridge_ageing_time + BRIDGE_RULE_DELAY, jiffies))
    //    goto ruc_unlock;

    if( (NULL == skb)){
        NFP_ADAPTER_ERROR("Args error\n");
        goto out;
    }

    if(bridge_entry_num > BRIDGE_ENTRYNUM_MAX)
    {
        NFP_ADAPTER_DEBUG("Bridge entries beyond the limit\n");
        goto out;
    }

    output_dev = skb->dev;
    if(!output_dev)
    {
        NFP_ADAPTER_ERROR("skb->dev is NULL\n");
        goto out;
    }

    //br1 = skb->dev->br_port->br;
    br1 = br_port_get_rcu(skb->dev)->br;

    dest = eth_hdr(skb)->h_dest;
    src = eth_hdr(skb)->h_source;

    if ( (!is_valid_ether_addr(dest) || (!is_valid_ether_addr(src)) )){
        goto out;
    }

    rcu_read_lock();
    fdb_entry = __br_fdb_get(br1, src);

    if(NULL == fdb_entry){
        NFP_ADAPTER_ERROR("Get net bridge fdb entry error\n");
        goto ruc_unlock;
    }

    /*smartbit�������ʱ����û��ѧ��Ŀ��macʱ�����鵼�¹���ѧϰ����*/
    if(NULL == __br_fdb_get(br1, dest))
        goto ruc_unlock;

    if(fdb_entry->is_local)
        goto ruc_unlock;

    if(NULL == fdb_entry->dst){
        NFP_ADAPTER_ERROR("Get net bridge port error\n");

        goto ruc_unlock;
    }

    input_dev=fdb_entry->dst->dev;
    if(NULL == input_dev){
        NFP_ADAPTER_ERROR("Get input dev error\n");
        goto ruc_unlock;
    }
    rcu_read_unlock();

    /*��������߽ӿ�ath0 ath1 ...��ֱ�ӷ���*/
    if(output_dev->name[0] == 'a' || input_dev->name[0] == 'a') {
        goto out;
    }

    memset(&BridgeEntry, 0, sizeof(BridgeEntry));
    memcpy( BridgeEntry.da , dest, ETH_ALEN );
    memcpy( BridgeEntry.sa, src, ETH_ALEN );
    strncpy( BridgeEntry.output_name, output_dev->name, IFNAMSIZ);
    strncpy( BridgeEntry.input_name, input_dev->name, IFNAMSIZ);
    BridgeEntry.input_ifindex = input_dev->ifindex;
    BridgeEntry.output_ifindex = output_dev->ifindex;

    if(nfp_bridge_acp_entry_exist(&BridgeEntry))
        goto out;

    if(NFP_ADAPTER_SUCCESS == nfp_bridge_acp_entry_add(&BridgeEntry)){
        nfp_bridge_add(&BridgeEntry);
    }

    return ret;

ruc_unlock:
    rcu_read_unlock();

out:

    return ret;
}

/*=========================================================================
 Function:		static void nfp_bridge_entry_print( void )
 Description:		��ʾACP��������
 Data Accessed:     bt_hash
 Data Updated:
 Input:			��
 Output:			��
 Return:			��
 Others:
=========================================================================*/
static void nfp_bridge_entry_print( void )
{
    PL2Bridge_entry_tab pEntryTab = NULL;
    int i = 0;

    printk("bridge entries:\n");

    if(!spin_trylock_bh(&br_hash_lock))
        return;

    for(i = 0; i < NFP_ARRAY_SIZE(bt_hash); i++){
        pEntryTab = bt_hash[i];
        while (pEntryTab){

            printk( "bt_hash[%d] LEFT:  "
                    "Input=%-6s "
                    "DA=%02X:%02X:%02X:%02X:%02X:%02X "
                    "SA=%02X:%02X:%02X:%02X:%02X:%02X "
                    "Type=%04X "
                    "Output=%s ",
                    i,
                    pEntryTab->direction[LEFT_DIRECTION].input_name,
                    pEntryTab->direction[LEFT_DIRECTION].da[0], pEntryTab->direction[LEFT_DIRECTION].da[1],
                    pEntryTab->direction[LEFT_DIRECTION].da[2], pEntryTab->direction[LEFT_DIRECTION].da[3],
                    pEntryTab->direction[LEFT_DIRECTION].da[4], pEntryTab->direction[LEFT_DIRECTION].da[5],

                    pEntryTab->direction[LEFT_DIRECTION].sa[0], pEntryTab->direction[LEFT_DIRECTION].sa[1],
                    pEntryTab->direction[LEFT_DIRECTION].sa[2], pEntryTab->direction[LEFT_DIRECTION].sa[3],
                    pEntryTab->direction[LEFT_DIRECTION].sa[4], pEntryTab->direction[LEFT_DIRECTION].sa[5],

                    pEntryTab->direction[LEFT_DIRECTION].ethertype,
                    pEntryTab->direction[LEFT_DIRECTION].output_name);

            printk( "bt_hash[%d] RIGHT: "
                    "Input=%-6s "
                    "DA=%02X:%02X:%02X:%02X:%02X:%02X "
                    "SA=%02X:%02X:%02X:%02X:%02X:%02X "
                    "Type=%04X "
                    "Output=%s\n",
                    i,
                    pEntryTab->direction[RIGHT_DIRECTION].input_name,
                    pEntryTab->direction[RIGHT_DIRECTION].da[0], pEntryTab->direction[RIGHT_DIRECTION].da[1],
                    pEntryTab->direction[RIGHT_DIRECTION].da[2], pEntryTab->direction[RIGHT_DIRECTION].da[3],
                    pEntryTab->direction[RIGHT_DIRECTION].da[4], pEntryTab->direction[RIGHT_DIRECTION].da[5],

                    pEntryTab->direction[RIGHT_DIRECTION].sa[0], pEntryTab->direction[RIGHT_DIRECTION].sa[1],
                    pEntryTab->direction[RIGHT_DIRECTION].sa[2], pEntryTab->direction[RIGHT_DIRECTION].sa[3],
                    pEntryTab->direction[RIGHT_DIRECTION].sa[4], pEntryTab->direction[RIGHT_DIRECTION].sa[5],

                    pEntryTab->direction[RIGHT_DIRECTION].ethertype,
                    pEntryTab->direction[RIGHT_DIRECTION].output_name);


            pEntryTab = pEntryTab->next;
        }
    }

    spin_unlock_bh(&br_hash_lock);

    printk("\n");
}


static void nfp_bridge_entry_clean(void)
{
    unsigned long hash = 0;
    PL2Bridge_entry_tab pEntryTab = NULL;
    PL2Bridge_entry_tab pEntryTabNext = NULL;


    spin_lock_bh(&br_hash_lock);

    for (hash = 0; hash < NFP_ARRAY_SIZE(bt_hash); hash++) {

        pEntryTab = bt_hash[hash];
        pEntryTabNext = bt_hash[hash];

        while (NULL != pEntryTab){
            pEntryTabNext = pEntryTab->next;
            if(nfp_bridge_valid_entry(&(pEntryTab->direction[LEFT_DIRECTION])))
                nfp_bridge_delete(&(pEntryTab->direction[LEFT_DIRECTION]));

            if(nfp_bridge_valid_entry(&(pEntryTab->direction[RIGHT_DIRECTION])))
                nfp_bridge_delete(&(pEntryTab->direction[RIGHT_DIRECTION]));

            br_timer_exit(&pEntryTab->timeout);

            if (NULL != pEntryTab) {
                kmem_cache_free(nfp_bridge_cache, pEntryTab);
                pEntryTab = NULL;

                /*�����������1*/
                bridge_entry_num--;
            }

            pEntryTab = pEntryTabNext;
        }

        bt_hash[hash] = NULL;
    }

    spin_unlock_bh(&br_hash_lock);

    return;
}


/*=========================================================================
 Function:		static int nfp_bridge_entry_dump(char *buf, char **start, off_t offset, int count, int *eof, void *data)
 Description:		��ʾFPP������������ACP�����
 Data Accessed:
 Data Updated:
 Input:			��
 Output:			��
 Return:			0:�ɹ�
 Others:
=========================================================================*/
static int nfp_bridge_entry_dump(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    nfp_bridge_entry_print();

    return 0;
}



/*=========================================================================
 Function:		static int nfp_bridge_entry_reset( struct file *filp, const char __user *buff,unsigned long len, void *data )
 Description:		��λFPP������������ACP�����
 Data Accessed:
 Data Updated:
 Input:			��
 Output:			��
 Return:			0:�ɹ�
 Others:
=========================================================================*/
static ssize_t nfp_bridge_entry_reset( struct file *filp, const char __user *buff,unsigned long len, void *data )
{
    if(NULL == buff){
        return len;
    }

    /*����echo 0 > /proc/fpp_bridge_entry ʱ������й���*/
    if('0' == buff[0]){
        nfp_bridge_entry_clean();
    }

    return len;
}


/*=========================================================================
 Function:		static int nfp_bridge_proc_init( void )
 Description:		������״̬��ѯproc�ӿڳ�ʼ��
 Data Accessed:

 Data Updated:
 Input:			��
 Output:			��
 Return:			0:�ɹ�

 Others: 			����/proc/fpp_bridge_entry�ļ�,ֻ��
=========================================================================*/
static int nfp_bridge_proc_init( void )
{
    struct proc_dir_entry *proc_entry = NULL;

    proc_entry = create_proc_entry( "nfp_bridge_entry", 0666, nfp_adapter_proc );
    proc_entry->read_proc = &nfp_bridge_entry_dump;
    proc_entry->write_proc= &nfp_bridge_entry_reset;
    return 0;
}



/*=========================================================================
 Function:		static void nfp_bridge_proc_exit( void )
 Description:		������״̬��ѯproc�ӿ�ɾ��
 Data Accessed:

 Data Updated:
 Input:			��
 Output:			��
 Return:			��

 Others: 			ɾ��/proc/fpp_bridge_entry�ļ�
=========================================================================*/
static void nfp_bridge_proc_exit( void )
{
    remove_proc_entry("nfp_bridge_entry", nfp_adapter_proc);
}

static inline u_int32_t HASH_FDB_MAC(const u_int8_t *macaddr)
{
	u_int32_t key;
	key = ((u_int32_t *)macaddr)[0] ^ ((u_int16_t *)macaddr)[2];
	return (jhash_1word(key, 0x12345678) & (NFP_FDB_HTABLE_SIZE - 1));
}


static struct fdb_entry_tab * __nfp_fdb_find(const struct fdb_entry_tab *rule)
{
    struct fdb_entry_tab *tmp_entry = NULL;
    unsigned int hash_key;

    NFP_ASSERT(rule);

    hash_key = HASH_FDB_MAC(rule->sa);

    list_for_each_entry(tmp_entry, &fdb_tables_by_mac[hash_key], list_by_mac)
    {
        if(!memcmp(tmp_entry->sa,rule->sa,ETH_ALEN) &&
            !memcmp(tmp_entry->da,rule->da,ETH_ALEN) &&
            tmp_entry->iif == rule->iif)
        {
            return tmp_entry;
        }
    }

    return NULL;
}


struct fdb_entry_tab *nfp_fdb_find(const struct fdb_entry_tab *rule)
{
    struct fdb_entry_tab *fdb_entry = NULL;

    if(!spin_trylock(&fdb_hash_lock))
        return NULL;

    fdb_entry = __nfp_fdb_find(rule);

    spin_unlock(&fdb_hash_lock);

    return fdb_entry;
}



static int nfp_fdb_add(const struct fdb_entry_tab *fdb_entry)
{
    int ret = 0;
    L2Bridge_entry br_entry, br_entry_replay;

    memset(&br_entry, 0, sizeof(br_entry));
    memset(&br_entry_replay, 0, sizeof(br_entry_replay));

    memcpy(br_entry.sa, fdb_entry->sa, sizeof(br_entry.sa));
    memcpy(br_entry.da, fdb_entry->da, sizeof(br_entry.da));
    br_entry.input_ifindex = fdb_entry->iif;
    br_entry.output_ifindex = fdb_entry->oif;

    memcpy(br_entry_replay.da, fdb_entry->sa, sizeof(br_entry_replay.da));
    memcpy(br_entry_replay.sa, fdb_entry->da, sizeof(br_entry_replay.sa));
    br_entry_replay.output_ifindex = fdb_entry->iif;
    br_entry_replay.input_ifindex = fdb_entry->oif;

    ret = nfp_bridge_option(NFP_OPT_ADD, &br_entry);
    if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
    {
        NFP_ADAPTER_ERROR("call nfp_bridge_option(NFP_OPT_ADD) error:%d\n", ret);
    }

    ret = nfp_bridge_option(NFP_OPT_ADD, &br_entry_replay);
    if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
    {
        NFP_ADAPTER_ERROR("call nfp_bridge_option(NFP_OPT_ADD) error:%d\n", ret);
    }

    return ret;
}


static int nfp_fdb_delete(const struct fdb_entry_tab *fdb_entry)
{
    int ret = 0;
    L2Bridge_entry br_entry, br_entry_replay;

    memset(&br_entry, 0, sizeof(br_entry));
    memset(&br_entry_replay, 0, sizeof(br_entry_replay));

    memcpy(br_entry.sa, fdb_entry->sa, sizeof(br_entry.sa));
    memcpy(br_entry.da, fdb_entry->da, sizeof(br_entry.da));
    br_entry.input_ifindex = fdb_entry->iif;
    br_entry.output_ifindex = fdb_entry->oif;

    memcpy(br_entry_replay.da, fdb_entry->sa, sizeof(br_entry_replay.da));
    memcpy(br_entry_replay.sa, fdb_entry->da, sizeof(br_entry_replay.sa));
    br_entry_replay.output_ifindex = fdb_entry->iif;
    br_entry_replay.input_ifindex = fdb_entry->oif;

    ret = nfp_bridge_option(NFP_OPT_DELETE, &br_entry);
    if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
    {
        NFP_ADAPTER_ERROR("call nfp_bridge_option(NFP_OPT_DELETE) error:%d\n", ret);
    }

    ret = nfp_bridge_option(NFP_OPT_DELETE, &br_entry_replay);
    if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
    {
        NFP_ADAPTER_ERROR("call nfp_bridge_option(NFP_OPT_DELETE) error:%d\n", ret);
    }

    return ret;
}


/*fdb��ʱ������ʱ��ѯ����ֹ���е�����ʱ��NFP��fdb��age���ܸ��£����¹���ɾ��*/
static int nfp_fdb_age(const struct fdb_entry_tab *fdb_entry)
{
    int ret = NFP_RULE_ACTIVE;
    L2Bridge_entry br_entry, br_entry_replay;

    memset(&br_entry, 0, sizeof(br_entry));
    memset(&br_entry_replay, 0, sizeof(br_entry_replay));

    memcpy(br_entry.sa, fdb_entry->sa, sizeof(br_entry.sa));
    memcpy(br_entry.da, fdb_entry->da, sizeof(br_entry.da));
    br_entry.input_ifindex = fdb_entry->iif;
    br_entry.output_ifindex = fdb_entry->oif;

    memcpy(br_entry_replay.da, fdb_entry->sa, sizeof(br_entry_replay.da));
    memcpy(br_entry_replay.sa, fdb_entry->da, sizeof(br_entry_replay.sa));
    br_entry_replay.output_ifindex = fdb_entry->iif;
    br_entry_replay.input_ifindex = fdb_entry->oif;

    /*local in fdb������NFP��ֻ�е��߼���*/
    if(NFP_RULE_TIMEOUT == nfp_bridge_option(NFP_OPT_STAT, &br_entry))
    //    && NFP_RULE_TIMEOUT == nfp_bridge_option(NFP_OPT_STAT, &br_entry_replay))
    {
        ret = NFP_RULE_TIMEOUT;
    }

    return ret;
}


static void nfp_fdb_timeout(unsigned long data)
{
    int ret = 0;
    struct fdb_entry_tab *tmp_entry;

	tmp_entry = (struct fdb_entry_tab *)data;
	NFP_ASSERT(tmp_entry);

    if (unlikely(!spin_trylock_bh(&fdb_hash_lock))) {
        mod_timer(&(tmp_entry->timeout) , jiffies + net_random()%FDB_RULE_TIMEOUT + FDB_RULE_TIMEOUT);
        NFP_ADAPTER_ERROR("spin_trylock_bh() faild\n");
        return ;
    }
    if(NFP_RULE_ACTIVE != nfp_fdb_age(tmp_entry)){
        ret = nfp_fdb_delete(tmp_entry);
        if(ret)
            NFP_ADAPTER_DEBUG("Call nfp_fdb_delete faild\n");

        nfp_fdb_acp_entry_delete(tmp_entry);
        fdb_rule_num--;
    }
    else{
        mod_timer(&(tmp_entry->timeout) , jiffies + net_random()%FDB_RULE_TIMEOUT + FDB_RULE_TIMEOUT);
    }

    spin_unlock_bh(&fdb_hash_lock);
}


inline static void fdb_timer_init( struct fdb_entry_tab *fdb_entry )
{

    setup_timer(&(fdb_entry->timeout), nfp_fdb_timeout, (unsigned long)fdb_entry);
    mod_timer(&(fdb_entry->timeout) , jiffies + net_random()%FDB_RULE_TIMEOUT + FDB_RULE_TIMEOUT);
}

inline static void fdb_timer_exit( struct timer_list *timeout )
{
    //del_timer_sync(timeout);
    del_timer(timeout);
}


static int nfp_fdb_acp_add(const struct fdb_entry_tab *rule)
{
    struct fdb_entry_tab *tmp_entry = NULL;
    struct fdb_entry_tab *next_entry = NULL;
    struct fdb_entry_tab *new_entry = NULL;
    unsigned int hash_key;
    int ret = 0;

    NFP_ASSERT(rule);

    hash_key = HASH_FDB_MAC(rule->sa);

    spin_lock(&fdb_hash_lock);

    list_for_each_entry_safe(tmp_entry, next_entry, &fdb_tables_by_mac[hash_key], list_by_mac)
    {
        if(!memcmp(tmp_entry->sa,rule->sa,ETH_ALEN) &&
            !memcmp(tmp_entry->da,rule->da,ETH_ALEN))
        {
            if(tmp_entry->iif != rule->iif){
                ret = nfp_fdb_delete(tmp_entry);
                nfp_fdb_acp_entry_delete(tmp_entry);
            }
            else{
                goto out;
            }
        }
    }

    new_entry = kmem_cache_alloc(nfp_fdb_cache, GFP_ATOMIC);
    if(NULL == new_entry)
    {
        NFP_ADAPTER_DEBUG("kmalloc new fdb_entry faild \n");
        goto out;
    }
    memcpy(new_entry,rule,sizeof(struct fdb_entry_tab));
    fdb_rule_num++;

    fdb_timer_init(new_entry);

    list_add(&new_entry->list_by_mac,&fdb_tables_by_mac[hash_key]);

    ret = nfp_fdb_add(new_entry);
    if(ret)
        NFP_ADAPTER_DEBUG("Call nfp_fdb_add  faild\n");

     spin_unlock(&fdb_hash_lock);

    return ret;

out:

    spin_unlock(&fdb_hash_lock);

    return 1;
}


static int nfp_fdb_acp_entry_delete(struct fdb_entry_tab *rule)
{
    if(NULL == rule)
    {
        NFP_ADAPTER_DEBUG("fdb rule is null! \n");
        return -1;
    }

	fdb_timer_exit(&rule->timeout);

    list_del(&rule->list_by_mac);
    kmem_cache_free(nfp_fdb_cache, rule);

    return 0;
}


static void nfp_fdb_clean(void)
{
    unsigned long hash = 0;
    struct fdb_entry_tab *tmp_entry = NULL;
    struct fdb_entry_tab *next_entry = NULL;

    spin_lock_bh(&fdb_hash_lock);

    for (hash = 0; hash < NFP_ARRAY_SIZE(fdb_tables_by_mac); hash++) {
        if(list_empty(&(fdb_tables_by_mac[hash])))
            continue;

        list_for_each_entry_safe(tmp_entry, next_entry, &fdb_tables_by_mac[hash], list_by_mac)
        {
            nfp_fdb_delete(tmp_entry);

            nfp_fdb_acp_entry_delete(tmp_entry);
            fdb_rule_num--;
        }
    }

    spin_unlock_bh(&fdb_hash_lock);

    return;
}


static int nfp_fdb_print(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    struct fdb_entry_tab *tmp_entry = NULL;
    int hash_key;


    if(!spin_trylock_bh(&fdb_hash_lock))
        return 0;

    printk("%d fdb rules : \n", fdb_rule_num);

    printk("hash\tsrc_addr\t\tdst_addr\t\tiif\toif\n");
    for(hash_key = 0 ;hash_key < NFP_ARRAY_SIZE(fdb_tables_by_mac); hash_key++)
    {
        list_for_each_entry(tmp_entry, &fdb_tables_by_mac[hash_key], list_by_mac)
        {
            printk("%d\t%02x:%02x:%02x:%02x:%02x:%02x\t%02x:%02x:%02x:%02x:%02x:%02x\t%d\t%d\n",
                   hash_key,
                   tmp_entry->sa[0],tmp_entry->sa[1],tmp_entry->sa[2],tmp_entry->sa[3],tmp_entry->sa[4],tmp_entry->sa[5],
                   tmp_entry->da[0],tmp_entry->da[1],tmp_entry->da[2],tmp_entry->da[3],tmp_entry->da[4],tmp_entry->da[5],
                   tmp_entry->iif,
                   tmp_entry->oif);
        }
    }

    spin_unlock_bh(&fdb_hash_lock);

    return 0;
}


static ssize_t nfp_fdb_reset( struct file *filp, const char __user *buff,unsigned long len, void *data )
{
    if(NULL == buff){
        return len;
    }

    /*����echo 0 > /proc/fpp_proc ʱ������й���*/
    if('0' == buff[0]){
        nfp_fdb_clean();
    }

    return len;
}


/********************************************************************************
 Function:		void nfp_fdb_flush(void)
 Description:	�����·�fdb���򵽼�����
 Data Accessed:
 Data Updated:
 Input:			��
 Output:
 Return:		��
 Others:
*********************************************************************************/
void nfp_fdb_flush(void)
{
    unsigned long hash = 0;
    struct fdb_entry_tab *tmp_entry = NULL;
    struct fdb_entry_tab *next_entry = NULL;

    spin_lock_bh(&fdb_hash_lock);

    for (hash = 0; hash < NFP_ARRAY_SIZE(fdb_tables_by_mac); hash++) {
        list_for_each_entry_safe(tmp_entry, next_entry, &fdb_tables_by_mac[hash], list_by_mac)
        {
            nfp_fdb_add(tmp_entry);
        }
    }

    spin_unlock_bh(&fdb_hash_lock);

    return;
}


int nfp_bridge_acp_add_delay(const struct sk_buff  *skb, const struct net_device *indev)
{
    int ret;
    struct fdb_entry_tab fdb_rule;
    unsigned char * dest;
    unsigned char *src;

    /*��ֹƵ���ӹ���*/
    //if(time_after(fdb_ageing_time + BRIDGE_RULE_DELAY, jiffies))
    //    goto out;

    if(NULL == skb || NULL == indev)
    {
        NFP_ADAPTER_DEBUG("sk_buff is null!\n");
        goto out;
    }

    if(NULL == skb->dev)
    {
        goto out;
    }

    if(fdb_rule_num > FDB_ENTRYNUM_MAX)
    {
        NFP_ADAPTER_DEBUG("Fdb entries beyond the limit\n");
        goto out;
    }

    /*ֻ����������ѧϰbridge rule*/
     /* TBS_TAG:add by pengyao 20120112
     * Desc: ֻѧϰ���е�fdb
     */
#if defined (CONFIG_TBS_FORWARDING)
    if(!(indev->tbs_forward.flags & EN_ITF_LAN))
        goto out;
#endif
    /* TBS_TAG:end
    */

    if(!(skb->dev->priv_flags & IFF_EBRIDGE))
        goto out;

    /*������pppoe proxy����*/
    if (skb->protocol == __constant_htons(ETH_P_PPP_SES)||
        skb->protocol == __constant_htons(ETH_P_PPP_DISC))
        goto out;

    dest = eth_hdr(skb)->h_dest;
    src = eth_hdr(skb)->h_source;
    if ( (!is_valid_ether_addr(dest) || (!is_valid_ether_addr(src)) )){
        goto out;
    }
 	memset(&fdb_rule, 0, sizeof(fdb_rule));
    memcpy(&(fdb_rule.da), dest , ETH_ALEN);
    memcpy(&(fdb_rule.sa), src , ETH_ALEN);
    fdb_rule.iif = indev->ifindex;
    fdb_rule.oif = skb->dev->ifindex;

    if(nfp_fdb_find(&fdb_rule))
        goto out;
    ret = nfp_fdb_acp_add(&fdb_rule);
    if(ret) {
        NFP_ADAPTER_DEBUG("Call nfp_fdb_acp_add faild\n");
        goto out;
    }

    return 0;

out:

    return -1;
}

static int nfp_fdb_proc_init(void )
{
    struct proc_dir_entry *fdb_proc = NULL;

    fdb_proc = create_proc_entry( "nfp_fdb_entry", 0666, nfp_adapter_proc );
    if(fdb_proc){
        fdb_proc->read_proc = &nfp_fdb_print;
        fdb_proc->write_proc= &nfp_fdb_reset;

        return 0;
    }
    return -1;
}

static void nfp_fdb_proc_exit(void )
{
    remove_proc_entry("nfp_fdb_entry", nfp_adapter_proc);
}

static void nfp_fdb_delete_by_ifindex(int ifindex)
{
    unsigned long hash = 0;
    struct fdb_entry_tab *tmp_entry = NULL;
    struct fdb_entry_tab *next_entry = NULL;


    spin_lock_bh(&fdb_hash_lock);

    for (hash = 0; hash < NFP_ARRAY_SIZE(fdb_tables_by_mac); hash++) {
        if(list_empty(&(fdb_tables_by_mac[hash])))
            continue;

        list_for_each_entry_safe(tmp_entry, next_entry, &fdb_tables_by_mac[hash], list_by_mac)
        {
            if(tmp_entry->iif != ifindex &&
                tmp_entry->oif != ifindex)
                continue;

            nfp_fdb_delete(tmp_entry);
            nfp_fdb_acp_entry_delete(tmp_entry);
            fdb_rule_num--;
        }
    }

    spin_unlock_bh(&fdb_hash_lock);

    return;
}


static void nfp_bridge_entry_clean_by_ifindex(int ifindex)
{
    unsigned long hash = 0;
    PL2Bridge_entry_tab pEntryTab = NULL;
    PL2Bridge_entry_tab pEntryTabPre = NULL;
    PL2Bridge_entry_tab pEntryTabNext = NULL;

    spin_lock_bh(&br_hash_lock);

    for (hash = 0; hash < NFP_ARRAY_SIZE(bt_hash); hash++) {

        pEntryTab = bt_hash[hash];
        pEntryTabPre = pEntryTab;

        while (NULL != pEntryTab){
            pEntryTabNext = pEntryTab->next;

            if(pEntryTab->direction[LEFT_DIRECTION].input_ifindex == ifindex ||
                pEntryTab->direction[LEFT_DIRECTION].output_ifindex == ifindex ||
                pEntryTab->direction[RIGHT_DIRECTION].input_ifindex == ifindex ||
                pEntryTab->direction[RIGHT_DIRECTION].output_ifindex == ifindex) {

                if(nfp_bridge_valid_entry(&(pEntryTab->direction[LEFT_DIRECTION])))
                    nfp_bridge_delete(&(pEntryTab->direction[LEFT_DIRECTION]));

                if(nfp_bridge_valid_entry(&(pEntryTab->direction[RIGHT_DIRECTION])))
                    nfp_bridge_delete(&(pEntryTab->direction[RIGHT_DIRECTION]));

                if(pEntryTab != bt_hash[hash])
                {
                    pEntryTabPre->next = pEntryTabNext;
                }
                else
                {
                    bt_hash[hash] = pEntryTabNext;
                    pEntryTabPre = pEntryTabNext;
                }

                br_timer_exit(&pEntryTab->timeout);

                kmem_cache_free(nfp_bridge_cache, pEntryTab);
                pEntryTab = NULL;

                /*�����������1*/
                bridge_entry_num--;
            }
            else {
                pEntryTabPre = pEntryTab;
            }

            pEntryTab = pEntryTabNext;
        }
    }

    spin_unlock_bh(&br_hash_lock);

    return;
}


/********************************************************************************
 Function:		void nfp_bridge_flush(void)
 Description:	�����·�bridge���򵽼�����
 Data Accessed:
 Data Updated:
 Input:			��
 Output:
 Return:		��
 Others:
*********************************************************************************/
void nfp_bridge_flush(void)
{
    unsigned long hash = 0;
    PL2Bridge_entry_tab pEntryTab = NULL;
    PL2Bridge_entry_tab pEntryTabNext = NULL;

    spin_lock_bh(&br_hash_lock);

    for (hash = 0; hash < NFP_ARRAY_SIZE(bt_hash); hash++) {

        pEntryTab = bt_hash[hash];
        pEntryTabNext = bt_hash[hash];

        while (NULL != pEntryTab){
            pEntryTabNext = pEntryTab->next;
            if(nfp_bridge_valid_entry(&(pEntryTab->direction[LEFT_DIRECTION])))
                nfp_bridge_add(&(pEntryTab->direction[LEFT_DIRECTION]));

            if(nfp_bridge_valid_entry(&(pEntryTab->direction[RIGHT_DIRECTION])))
                nfp_bridge_add(&(pEntryTab->direction[RIGHT_DIRECTION]));

            pEntryTab = pEntryTabNext;
        }
    }

    spin_unlock_bh(&br_hash_lock);

    return;
}


static int nfp_bridge_brport_event_handler(struct notifier_block *this, unsigned long event, void *ptr)
{
    struct net_device *dev = ptr;
    struct net_bridge_port *p = NULL;

    NFP_ASSERT(dev);

    //p = dev->br_port;
    p = br_port_get_rcu(dev);

    /*ֻ����bridge_portt*/
    if(p == NULL)
        return NOTIFY_OK;

    switch(event) {
        case NETDEV_DOWN:
        case NETDEV_UNREGISTER:
        case NETDEV_CHANGE:

            if(p->state == BR_STATE_DISABLED) {
                #if defined CONFIG_TBS_NFP_MARVELL_ADAPTER || defined(CONFIG_TBS_NFP_TBS_ADAPTER)
                nfp_fdb_delete_by_ifindex(dev->ifindex);
                #endif

                nfp_bridge_entry_clean_by_ifindex(dev->ifindex);
            }

            break;

        default:
            break;
    }

    return NOTIFY_OK;
}


static struct notifier_block brport_notifier = {
        .notifier_call	= nfp_bridge_brport_event_handler,
        .next			= NULL,
        .priority		= 0
                                                };

int nfp_fdb_init(void)
{
    int ret = 0;

    /*fdb�����д����ʼ��*/
    spin_lock_init(&fdb_hash_lock);

    nfp_fdb_cache = kmem_cache_create("nfp_fdb_cache",
                     sizeof(struct fdb_entry_tab),
                     0,
                     SLAB_HWCACHE_ALIGN, NULL);
    if (!nfp_fdb_cache){
        NFP_ADAPTER_ERROR("Fail to kmem hash cache\n");
        ret = -ENOMEM;
        goto out;
    }

    ret = nfp_fdb_proc_init();
    if(ret < 0){
        NFP_ADAPTER_ERROR("call nfp_fdb_proc_init() failed\n");
        goto out;
    }

    fdb_rule_num = 0;
    nfp_fdb_hook = nfp_bridge_acp_add_delay;

out:
    return ret;
}
void nfp_fdb_exit(void)
{
    nfp_fdb_hook = NULL;

    /*ɾ��proc*/
    nfp_fdb_proc_exit();

    nfp_fdb_clean();
    kmem_cache_destroy(nfp_fdb_cache);
}


/*=========================================================================
 Function:		int nfp_bridge_init( void )
 Description:		�����������������ʼ��
 Data Accessed:     bt_hash:������������ACP�Ļ����ϣ��,��ʼ��Ϊȫ0
                            L2BridgeEnableCommand_ptr: �豸�˿ڼ�����ʹ�ܹ��ӳ�ʼ��
                            L2BridgeDisableCommand_ptr: �豸�˿ڼ��������ù��ӳ�ʼ��
                            nfp_bridge_add_entry_cmd: ������������ӹ��ӳ�ʼ��
                            c1k_link_status_intr_hook: ����������λ���ӳ�ʼ��
 Data Updated:
 Input:			��
 Output:			��
 Return:			0:�ɹ�

 Others: 			����FPP eth0 eth2�ӿڼ���
                            ��FPP Bridge״̬��ѯ����
=========================================================================*/

int nfp_bridge_init( void )
{
    int ret = 0;

    spin_lock_init(&br_hash_lock);

    memset(bt_hash, 0, sizeof(bt_hash));
    bridge_entry_num= 0;

    nfp_bridge_cache = kmem_cache_create("nfp_bridge_cache",
                     sizeof(struct tL2Bridge_entry_tab),
                     0,
                     SLAB_HWCACHE_ALIGN, NULL);
    if (!nfp_bridge_cache){
        NFP_ADAPTER_ERROR("Fail to kmem hash cache\n");
        ret = -ENOMEM;
        goto err1;
    }

    nfp_bridge_proc_init();

    #if defined CONFIG_TBS_NFP_MARVELL_ADAPTER || defined(CONFIG_TBS_NFP_TBS_ADAPTER)
    if(nfp_fdb_init())
        goto err2;
    #endif

    /*ע�ᵽnetdev_chain֪ͨ��*/
    ret = register_netdevice_notifier(&brport_notifier );
    if (ret < 0) {
        NFP_ADAPTER_ERROR("nfp_bridge_init: register brport_notifier to netdev_chain failed\n");
        goto err3;
    }

    nfp_bridge_add_entry_cmd = nfp_bridge_entry_add_delay;
    nfp_bridge_clean_by_ifindex = nfp_bridge_entry_clean_by_ifindex;

    NFP_ADAPTER_DEBUG("Nfp bridge initialized SUCCESSFULLY.\n");

    return 0;

err3:
#if defined CONFIG_TBS_NFP_MARVELL_ADAPTER || defined(CONFIG_TBS_NFP_TBS_ADAPTER)
    nfp_fdb_exit();
err2:
#endif

    kmem_cache_destroy(nfp_bridge_cache);

err1:

    return ret;
}


/*=========================================================================
 Function:		void FppBridgeExit( void )
 Description:		�����������������˳�
 Data Accessed:     L2BridgeEnableCommand_ptr: �豸�˿ڼ�����ʹ�ܹ��ӳ�ʼ��ΪNULL
                            L2BridgeDisableCommand_ptr: �豸�˿ڼ��������ù��ӳ�ʼ��ΪNULL
                            nfp_bridge_add_entry_cmd: ������������ӹ��ӳ�ʼ��ΪNULL
                            c1k_link_status_intr_hook: ����������λ���ӳ�ʼ��ΪNULL
 Data Updated:
 Input:			��
 Output:			��
 Return:			��

 Others: 			��λFPP Bridge���ݰ���������
                            �ر�FPP Bridge״̬��ѯ����
                            �ر�FPP eth0 eth2�ӿڼ���
                            ��������������
=========================================================================*/

void nfp_bridge_exit( void )
{
    /* �˳�ʱ����ע��֪ͨ�� */
    unregister_netdevice_notifier(&brport_notifier);

    nfp_bridge_add_entry_cmd = NULL;
    nfp_bridge_clean_by_ifindex = NULL;

    nfp_bridge_proc_exit();

    nfp_bridge_entry_clean();
    #if defined CONFIG_TBS_NFP_MARVELL_ADAPTER || defined(CONFIG_TBS_NFP_TBS_ADAPTER)
    nfp_fdb_exit();
    #endif

    kmem_cache_destroy(nfp_bridge_cache);
    NFP_ADAPTER_DEBUG("Nfp bridge unloaded SUCCESSFULLY.\n");
}


