/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 �ļ����� : nfp_neighbour.c
 �ļ����� : TBS���������� arp��������

 �޶���¼ :
          1 ���� : pengyao
            ���� : 2011-04-10
            ���� :
**********************************************************************/


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

#include "nfp_neighbour.h"
#include "nfp_interface.h"
#include "nfp_adapter.h"
#include "nfp_route.h"

/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/

/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/

/*********************************************************************
 *                              FUNCTIONS                            *
 *********************************************************************/

/*in core/neighbour.c*/
extern int register_neigh_notifier(struct notifier_block *nb);
extern int unregister_neigh_notifier(struct notifier_block *nb);

static inline int nfp_neigh_option(unsigned short option, const struct neighbour_entry *nb)
{
    int ret = NFP_UNABLE_PARSER;

    if(nfp_cmd_parser){
        const struct nfp_adapter_cmd cmd_entry = {option, (void *)nb};
        ret = nfp_cmd_parser(NFP_CMD_ARP, &cmd_entry);
    }
    else if(NFP_OPT_STAT == option)
        ret = NFP_RULE_TIMEOUT;

    return ret;
}


/*=========================================================================
 Function:		static int nfp_neigh_add(const struct neighbour_entry *nb)

 Description:		Э��ջ��neighbour��������¼�������
 Data Accessed:

 Data Updated:
 Input:			    unsigned long neighbor      neighbour����ָ��
 Output:			��
 Return:			0:�ɹ�

 Others:
=========================================================================*/

static int nfp_neigh_add(const struct neighbour_entry *nb)
{
	int ret = 0;
	NFP_ASSERT(nb);

	NFP_ADAPTER_INTO_FUNC;

	/*����������ӹ���*/
	ret = nfp_neigh_option(NFP_OPT_ADD, nb);
	if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
	{
		NFP_ADAPTER_ERROR("call nfp_neigh_option(NFP_OPT_ADD) error:%d\n", ret);
		//nfp_neigh_dump(new_neigh);
		goto out;
	}

out:
	return ret;
}


/*=========================================================================
 Function:		static int nfp_neigh_delete(const struct neighbour_entry *nb)

 Description:		Э��ջ��neighbour����ɾ���¼�������
 Data Accessed:

 Data Updated:
 Input:			    unsigned long neighbor      neighbour����ָ��
 Output:			��
 Return:			0:�ɹ�

 Others:
=========================================================================*/

static int nfp_neigh_delete(const struct neighbour_entry *nb)
{
	int ret = 0;
	NFP_ASSERT(nb);

	NFP_ADAPTER_INTO_FUNC;

	/*ɾ���������й���*/
	ret = nfp_neigh_option(NFP_OPT_DELETE, nb);
	if(NFP_UNABLE_PARSER != ret && NFP_NO_ERR != ret)
	{
		NFP_ADAPTER_ERROR("call nfp_neigh_option(NFP_OPT_DELETE) error:%d\n", ret);
	}

	return ret;
}


#ifdef CONFIG_TBS_NFP_FIB
int nfp_arp_age (int family, const u32 *ip)
{
    struct neighbour_entry neigh;

    memset(&neigh, 0, sizeof(neigh));
    nfp_l3_addr_copy(family, (unsigned char *)&(neigh.ip_addr), (unsigned char *)ip);
    neigh.family = family;

	/*����������ӹ���*/
    if(NFP_RULE_TIMEOUT == nfp_neigh_option(NFP_OPT_STAT, &neigh))
        return 0;

	return NFP_RULE_ACTIVE;
}
#endif


/*=========================================================================
 Function:		static int acp_neigh_event(unsigned long events, struct neighbour *nb)

 Description:		ͨ�����Ӻ�����Э��ջneighbourģ��ѧϰneighbour����
 Data Accessed:     neighbour_tables neighbour_tables_by_mac

 Data Updated:
 Input:			    unsigned long events        neighbour�������¼�:new/delete
                    struct neighbour *nb        neighbourָ��
 Output:			��
 Return:			0:�ɹ�

 Others: 			��ӡ�ɾ��neighbour������ӵ����沢�·���������
=========================================================================*/
//static int acp_neigh_event(unsigned long events, struct neighbour *nb)
static int acp_neigh_event(struct notifier_block *this, unsigned long events, void *ptr)
{
    struct neighbour *nb = (struct neighbour *)ptr;
    struct neighbour_entry neigh;

    memset(&neigh, 0, sizeof(neigh));
    //return 0;

    //NFP_ADAPTER_INTO_FUNC;
    NFP_ASSERT(nb);

    if(is_zero_ether_addr(nb->ha)){
        goto out;
    }

    if(is_multicast_ether_addr(nb->ha))
    {
        //NFP_ADAPTER_DEBUG("is multicast ether addr\n");
        goto out;
    }

    if(!nfp_interface_exist(nb->dev->ifindex))
    {
        //NFP_ADAPTER_DEBUG("invalid interface\n");
        goto out;
    }

    /*�Ȳ�����IPV6*/
    if(AF_INET != nb->ops->family){
        goto out;
    }

    memcpy(neigh.mac_addr, nb->ha, nb->dev->hard_header_len);
    memcpy(neigh.ip_addr, nb->primary_key, nb->tbl->key_len);
    neigh.ip_addr_len = nb->tbl->key_len;
    neigh.family = nb->ops->family;

    NFP_ADAPTER_DEBUG("Neighbour event: %lu, nb = %p\n", events, nb);

    switch(events)
    {
        /*����neighbour��¼*/
        case RTM_NEWNEIGH:
            nfp_neigh_add(&neigh);
#ifdef CONFIG_TBS_NFP_FIB
			nb->tbs_nfp = true;
#endif
            break;
        case RTM_DELNEIGH:
            nfp_neigh_delete(&neigh);
#ifdef CONFIG_TBS_NFP_FIB
			nb->tbs_nfp = false;
#endif
            break;
        default:
            NFP_ADAPTER_DEBUG("Neighbour event: %lu, nothing to do\n", events);
            break;
    }

out:
    return NOTIFY_DONE;
}


/*=========================================================================
 Function:		static void nfp_neigh_proc_exit( void )
 Description:		neighbour����״̬��ѯproc�ӿ�ɾ��
 Data Accessed:

 Data Updated:
 Input:			��
 Output:			��
 Return:			��

 Others: 			ɾ��/proc/nfp_neigh_entry�ļ�
=========================================================================*/
static void nfp_neigh_proc_exit( void )
{
    remove_proc_entry("nfp_neigh_entry", nfp_adapter_proc);
}


static struct notifier_block nfp_neigh_notifier = {
        .notifier_call	= acp_neigh_event,
        .next			= NULL,
        .priority		= 0
};


/*=========================================================================
 Function:		    int nfp_neighbour_init(void)

 Description:		neighbour��ģ���ʼ��,����hash����ʼ��
 Data Accessed:

 Data Updated:
 Input:			    ��
 Output:			��
 Return:			0:�ɹ�

 Others:
=========================================================================*/
#ifdef CONFIG_TBS_NFP_FIB
extern int (*tbs_nfp_hook_arp_age)(int, const u32 *);
#endif
int nfp_neighbour_init(void)
{
	int ret = 0;

    //return 0;
    NFP_ADAPTER_INTO_FUNC;

    /*ע��neighbour֪ͨ��*/
    ret = register_neigh_notifier(&nfp_neigh_notifier);
    if (ret < 0) {
        NFP_ADAPTER_ERROR("nfp_neighbour_init: register nfp_neigh_notifier to neighbour_chain failed\n");
        return ret;
    }

#ifdef CONFIG_TBS_NFP_FIB
	tbs_nfp_hook_arp_age = nfp_arp_age;
#endif

    NFP_ADAPTER_OUT_FUNC;
    return ret;
}


/*=========================================================================
 Function:		    int nfp_neighbour_exit(void)

 Description:		neighbour��ģ���˳����ͷ��ڴ���Դ
 Data Accessed:

 Data Updated:
 Input:			    ��
 Output:			��
 Return:			0:�ɹ�

 Others:
=========================================================================*/

int nfp_neighbour_exit(void)
{
    NFP_ADAPTER_INTO_FUNC;

#ifdef CONFIG_TBS_NFP_FIB
    tbs_nfp_hook_arp_age = NULL;
#endif

    if(unregister_neigh_notifier(&nfp_neigh_notifier))
        NFP_ADAPTER_ERROR("nfp_neighbour_exit: unregister_neigh_notifier fail.\n");

    nfp_neigh_proc_exit();

    return 0;
}

