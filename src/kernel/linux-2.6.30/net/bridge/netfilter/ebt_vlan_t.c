
/*
 *  ebt_vlan
 *
 *	Authors:
 *	ouyangdi <fireor@126.com>
 *
 *  July, 2008
 *
 */

/* The vlan target can be used in any chain,
 * I believe adding a mangle table just for vlaning is total overkill.
 * Marking a frame doesn't really change anything in the frame anyway.
 *
 *change log:
 *
 * 2008/06/18 ouyangdi
 * fix bug don't normal foward large packet when used --vlan-untag function
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_vlan_t.h>
#include <linux/netfilter/x_tables.h>
#include <linux/module.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>

/*mod by pengyao 2010116*/
#define OPT_VLAN_TARGET     (0x01<<0)
#define OPT_VLAN_SET        (0x01<<1)
#define OPT_VLAN_PRIORITY   (0x01<<2)
#define OPT_VLAN_HEADER     (0x01<<3)
#define OPT_VLAN_UNTAG      (0x01<<4)
#define OPT_VLAN_REDIR      (0x01<<5)

static int ebt_set_vlan_header(struct sk_buff **pskb, unsigned char cmd)
{
	struct sk_buff *skb = *pskb;
    char *dd;

	if (cmd & OPT_VLAN_UNTAG) {
		/* frame should be untagged */

		if (eth_hdr(skb)->h_proto == htons(ETH_P_8021Q)) {
			struct vlan_ethhdr *vhdr = (struct vlan_ethhdr *)eth_hdr(skb);
			/*
			 *get vlan id form skb->date and set to skb->vlan_tci
			 *add by ouyangdi@20091126
			 */
			skb->vlan_tci = ntohs(vhdr->h_vlan_TCI);

			/* remove VLAN tag */
			if (skb_cloned(skb) || skb_shared(skb)) {
                /*
                 * TBS_TAG:add by pengyao 20120310
                 * Desc: 2.6.24后的版本target()入参skb使用指针传递，不能按指针引用方式修改skb指针
                 */
#if 1
                printk(KERN_ERR "[%s %d]: Cloned or shared skb, can not remove VLAN tags\n",
                    __FUNCTION__, __LINE__);

                return 0;
#else
                struct sk_buff *new_skb;
                new_skb = skb_copy(skb, GFP_ATOMIC);
                kfree_skb(skb);
                if (!new_skb)
                    return 1;
                *pskb = skb = new_skb;
#endif
                /*
                 * TBS_END_TAG
                 */
			}

			skb_pull(skb, VLAN_HLEN);
			memmove(((char *)eth_hdr(skb)) + VLAN_HLEN, ((char *)eth_hdr(skb)), ETH_ALEN * 2);
            skb->mac_header += VLAN_HLEN;

			/*reset skb->protocol,if no reset will error in local process*/
			skb->protocol = eth_hdr(skb)->h_proto;
		}
	} else {
		/* frame should be tagged */
		if (eth_hdr(skb)->h_proto != htons(ETH_P_8021Q)) {
			/* add VLAN tag */
			struct vlan_ethhdr *vhdr;
			unsigned tag = skb->vlan_tci;
#ifdef VLAN_DEBUG
			printk("vlan tag = %d\n",skb->vlan_tci);
#endif

			if (skb_cloned(skb) || skb_shared(skb)) {
                /*
                 * TBS_TAG:add by pengyao 20120310
                 * Desc: 2.6.24后的版本target()入参skb使用指针传递，不能按指针引用方式修改skb指针
                 */
#if 1
                printk(KERN_ERR "[%s %d]: Cloned or shared skb, can not add VLAN tags\n",
                    __FUNCTION__, __LINE__);

                return 0;
#else
                struct sk_buff *new_skb;
                new_skb = skb_copy(skb, GFP_ATOMIC);
                kfree_skb(skb);
                if (!new_skb)
                    return 1;
                *pskb = skb = new_skb;
#endif
                /*
                 * TBS_END_TAG
                 */
			}

			if (skb_headroom(skb) < VLAN_HLEN) {
				if (pskb_expand_head(skb, VLAN_HLEN, 0,
							GFP_ATOMIC)) {
					kfree_skb(skb);
					return 1;
				}
			}

			skb_push(skb, VLAN_HLEN);
			if(skb_headroom(skb) < ETH_HLEN) {
				int len = ETH_HLEN - skb_headroom(skb);
				if (pskb_expand_head(skb, len, 0,
							GFP_ATOMIC)) {
					kfree_skb(skb);
					return 1;
				}
			}
            skb->mac_header -= VLAN_HLEN;
            dd = (char *)skb->mac_header;
			memmove(dd, dd + VLAN_HLEN,
					ETH_ALEN * 2);
			vhdr = (struct vlan_ethhdr *)eth_hdr(skb);
			vhdr->h_vlan_proto = htons(ETH_P_8021Q);
#ifdef VLAN_DEBUG
			printk("vlan tag = %d\n",skb->vlan_tci);
#endif

			vhdr->h_vlan_TCI = htons(tag);
			skb->protocol = htons(ETH_P_8021Q);

#ifdef VLAN_DEBUG
			int i = 0;
			printk("\nmac raw \n");
			for(i = 0; i < 16; i++)
			{
				if(!(i%2))
					printk(" ");
				printk("%02x",*((char *)eth_hdr(*pskb) + i));
			}
			printk("\n");
#endif

		} else {
			/* ensure VID is correct */
			struct vlan_ethhdr *vhdr =
				(struct vlan_ethhdr *)eth_hdr(skb);
			vhdr->h_vlan_TCI = htons(skb->vlan_tci);
		}
	}

	return 0;
}

static int ebt_target_vlan(struct sk_buff *skb, const struct xt_match_param *par)
{
    struct net *net;
    struct net_device * pNetDev = NULL;
	const struct ebt_vlan_t_info *info = par->targinfo;
	if(eth_hdr(skb) == NULL ) {
        printk("%s %d\n", __FUNCTION__, __LINE__);
			return EBT_ACCEPT;
	}
	switch(info->cmd )
	{
		case OPT_VLAN_SET:
			if( eth_hdr(skb)->h_proto != htons(ETH_P_8021Q))
			{
				//if( !info->vlan && (skb->vlan_tci) == 0 )
				//	break;

				if( info->vlan )
				{
					skb->vlan_tci &= ~0x1fff;
					skb->vlan_tci |= (info->vlan) & 0x1fff;
				}
			}else{
				if( !info->vlan )
				{
					 struct vlan_ethhdr *vhdr = (struct vlan_ethhdr *)eth_hdr(skb);
					 skb->vlan_tci = (vhdr->h_vlan_TCI & 0x1fff );

				}
				else
					skb->vlan_tci = info->vlan;
				if(!info->priority) {
					struct vlan_ethhdr *vhdr =
                               		(struct vlan_ethhdr *)eth_hdr(skb);
					skb->vlan_tci |= (vhdr->h_vlan_TCI & 0xe000);
				} else {
					skb->vlan_tci &= ~0xe000;
					skb->vlan_tci |= (info->priority <<13 & 0xe000);
				}
			}
			break;
        case OPT_VLAN_PRIORITY:
            if (eth_hdr(skb)->h_proto != htons(ETH_P_8021Q))
            {
                skb->vlan_tci &= ~0xe000;
                skb->vlan_tci |= (info->priority <<13 & 0xe000);
            }
            else
            {
    #if 1
                /*mod by pengyao 20110720,用于802.1q重标记*/
                struct vlan_ethhdr *vhdr =
                               (struct vlan_ethhdr *)eth_hdr(skb);
                skb->vlan_tci = ntohs(vhdr->h_vlan_TCI) & (~0xe000);
                skb->vlan_tci |= (info->priority <<13 & 0xe000);
    #else
                skb->vlan_tci &= ~0xe000;
                skb->vlan_tci |= (info->priority <<13 & 0xe000);
    #endif
            }
            break;

		case OPT_VLAN_HEADER:
			if(skb->vlan_tci == 0)
				return EBT_ACCEPT;
			if(ebt_set_vlan_header(&skb,info->cmd))
				return EBT_DROP;
			break;

		case OPT_VLAN_UNTAG:
			if(ebt_set_vlan_header(&skb,info->cmd))
				return EBT_DROP;
			break;
        case OPT_VLAN_REDIR:
            net = sock_net(skb->sk);
    		pNetDev = __dev_get_by_index(net, info->redir_ifindex);

    		/*don't need use lock*/
    		if(NULL != pNetDev)
    			skb->dev = pNetDev;

    		/*进此接口时由于目的地址和端口的地址不一样，
    		pkt_type被设置成了PACKET_OTHERHOST，
    		重定向端口后需要把pkt_type改成本端口*/
    		skb->pkt_type = PACKET_HOST;
            return EBT_ACCEPT;
            break;

		default:
			break;
	}

    return info->target;
}

static bool ebt_target_vlan_check(struct xt_tgchk_param *par)
{
	struct ebt_vlan_t_info *info = par->targinfo;
//	printk("%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	if( BASE_CHAIN && info->target == EBT_RETURN )
		return false;
	//CLEAR_BASE_CHAIN_BIT;
	if( INVALID_TARGET )
		return false;
	return true;
}

static struct xt_target vlan_target =
{
	.name		= EBT_VLAN_TARGET,
	.family 	= NFPROTO_BRIDGE,
	.revision	= 0,
	.target		= ebt_target_vlan,
	.checkentry	= ebt_target_vlan_check,
	.targetsize	= XT_ALIGN(sizeof(struct ebt_vlan_t_info)),
	.me		= THIS_MODULE,
};

static int __init ebt_vlan_t_init(void)
{
//	printk("%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	return xt_register_target(&vlan_target);
}

static void __exit ebt_vlan_t_fini(void)
{
//	printk("%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
	xt_unregister_target(&vlan_target);
}

module_init(ebt_vlan_t_init);
module_exit(ebt_vlan_t_fini);
MODULE_LICENSE("GPL");

