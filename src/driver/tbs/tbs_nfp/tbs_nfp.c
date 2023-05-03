/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称 : tbs_nfp.c
 文件描述 : TBS加速器核心代码

 修订记录 :
          1 创建 :
            日期 : 2011-11-10
            描述 :
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
#include <net/ip_vs.h>

#include "tbs_nfp.h"
#include "tbs_nfp_proc.h"
#include "tbs_nfp_skb_parser.h"

#include "tbs_nfp_itf.h"

#ifdef TBS_NFP_BRIDGE
#include "tbs_nfp_bridge.h"
#endif

#ifdef TBS_NFP_FIB
#include "tbs_nfp_fib.h"
#include "tbs_nfp_arp.h"
#endif

#ifdef TBS_NFP_CT
#include "tbs_nfp_ct.h"
#endif

/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/



/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
unsigned char       g_debug_level __read_mostly = TBS_NFP_WARN_PRINT;
unsigned char       g_nfp_enable __read_mostly = 1;
unsigned int        g_jhash_iv __read_mostly;

#ifdef TBS_NFP_STAT
TBS_NFP_STATS g_nfp_stats[TBS_ETH_MAX_PORTS];
//TBS_NFP_STATS       g_debug_status;
#endif

/*加速器处理入口，define in dev.c*/
extern int (*tbs_nfp_rx_hook)(struct sk_buff *skb);

/*全局读写自旋锁*/
DEFINE_RWLOCK(tbs_nfp_lock);
//DEFINE_SPINLOCK(tbs_nfp_lock);


/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/
void tbs_nfp_rule_reset(void)
{
#ifdef TBS_NFP_BRIDGE
    tbs_nfp_bridge_reset();
#endif
}


#ifdef TBS_NFP_VLAN
/*=========================================================================
 Function:		static inline void tbs_nfp_vlanadd(int port, TBS_NFP_RX_DESC *rx_desc,
                                                __be16 vid, bool moveMac)

 Description:		添加vlan tag
 Data Accessed:
 Data Updated:
 Input:			    int port                 vid 对应的vlan接口的if_index
                    TBS_NFP_RX_DESC *rx_desc 数据包描述结构体指针
                    __be16 vid               添加的vlan tag的vlan id
                    bool moveMac             是否需要偏移mac地址
 Output:			无
 Return:			-4  mac头向左移4位

 author: tbs cairong  2011-12-13
=========================================================================*/
static inline void tbs_nfp_vlanadd(int port, TBS_NFP_RX_DESC *rx_desc, __be16 vid, bool moveMac)
{
    unsigned char  *mac_header = rx_desc->mac_header + rx_desc->shift;
    unsigned char  *pNew = mac_header - sizeof(struct vlan_hdr);//4==sizeof(struct vlan_hdr)
    __be16 *pVlan;

    if (moveMac) {
        /* 将mac头部数据向左移动四个字节. Copy 12 bytes (DA + SA) */
        memmove(pNew, mac_header, ETH_ALEN*2);
    }

    pVlan = (__be16 *)(pNew + ETH_ALEN*2);

    /* 设置vlan type   0x8100 */
    *pVlan = htons(0x8100);
    pVlan++;

    /* 设置 VID + priority */
    *pVlan = htons(vid);

    /*skb描述结构中数据包头部偏移量-4*/
    rx_desc->shift -= sizeof(struct vlan_hdr);

    /*增加VLAN接口的数据包计数*/
	TBS_NFP_INC(port, vlan_tx_add);
    return;
}

/*=========================================================================
 Function:		static inline void tbs_nfp_vlanremove(int port, TBS_NFP_RX_DESC *rx_desc,
                                                 bool moveMac)

 Description:		移除vlan tag
 Data Accessed:
 Data Updated:
 Input:			    int port                 vid 对应的vlan接口的if_index
                    TBS_NFP_RX_DESC *rx_desc 数据包描述结构体指针
                    bool moveMac             是否需要偏移mac地址
 Output:			无
 Return:			4  mac头向右移4位

 author: tbs cairong  2011-12-13
=========================================================================*/
static inline void tbs_nfp_vlanremove(int port, TBS_NFP_RX_DESC *rx_desc, bool moveMac)
{
    unsigned char  *mac_header = rx_desc->mac_header + rx_desc->shift;
    unsigned char  *pNew = mac_header + sizeof(struct vlan_hdr);

    if (moveMac) {
        /* move MAC header 4 bytes right. Copy 12 bytes (DA + SA) */
       memmove(pNew, mac_header, ETH_ALEN*2);
    }

    /*skb描述结构中数据包头部偏移量4*/
    rx_desc->shift += sizeof(struct vlan_hdr);


    /*增加VLAN接口的数据包计数*/
	TBS_NFP_INC(port, vlan_tx_remove);
    return;
}

/*=========================================================================
 Function:		static inline void tbs_nfp_vlanreplace(int port,
            unsigned char *mac_header, __be16 vid)

 Description:		vlan头替换
 Data Accessed:     无

 Data Updated:
 Input:			    int port                 vid 对应的vlan接口的if_index
                    TBS_NFP_RX_DESC *rx_desc 数据包描述结构体指针
                    __be16 vid               新的vlan id
 Output:			无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
static inline void tbs_nfp_vlanreplace(int port, TBS_NFP_RX_DESC *rx_desc, __be16 vid)
{
    unsigned char * mac_header = rx_desc->mac_header + rx_desc->shift;
    __be16 *pVlan = (__be16 *)(mac_header + ETH_HLEN);;
    *pVlan = htons(vid);

    /*增加VLAN接口的数据包计数*/
	TBS_NFP_INC(port, vlan_tx_replace);
    return;
}

/*=========================================================================
 Function:		static inline int tbs_nfp_bridge_vlan_update(int port, TBS_NFP_RX_DESC *rx_desc,
                    const TBS_NFP_IF_MAP *out_if,unsigned char *mac_header)

 Description:		更新 bridge加速 vlan tag(add/delete/replace)
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    TBS_NFP_RX_DESC *rx_desc    数据包信息
        			const TBS_NFP_IF_MAP *vir_iifmap  进接口ifmap
        			const TBS_NFP_IF_MAP *vir_oifmap  出接口ifmap
        			, int port                  物理接收接口
 Output:
 Return:
 Others:
=========================================================================*/
static inline int tbs_nfp_bridge_vlan_update(TBS_NFP_RX_DESC *rx_desc, const TBS_NFP_IF_MAP *iifmap,
                               const TBS_NFP_IF_MAP *oifmap, int port)
{
    const TBS_NFP_IF_MAP *vir_iifmap = iifmap;
    const TBS_NFP_IF_MAP *vir_oifmap = oifmap;
    unsigned int status = rx_desc->status;

    //TBS_NFP_INTO_FUNC;

     /*带vlan的桥接*/
#if defined(CONFIG_VNET)
    /*出接口替换*/
    if(vir_oifmap->flags & TBS_NFP_F_MAP_VNET)
    {
        if(vir_oifmap->parent_if->flags & TBS_NFP_F_MAP_VLAN)
        {
            vir_oifmap = vir_oifmap->parent_if;
        }
    }

    /*入接口替换*/
    if(vir_iifmap->flags & TBS_NFP_F_MAP_VNET)
    {
        if(vir_iifmap->parent_if->flags & TBS_NFP_F_MAP_VLAN)
        {
            vir_iifmap = vir_iifmap->parent_if;
        }
    }
#endif

    if(TBS_NFP_RX_ISNOT_VLAN(status))
    {
        /*遍历出接口并处理vlan tag*/
        while(vir_oifmap->parent_if)
        {
            if(vir_oifmap->flags & TBS_NFP_F_MAP_VLAN)
            {
                tbs_nfp_vlanadd(port, rx_desc, vir_oifmap->vlanid, true);
            }
            vir_oifmap = vir_oifmap->parent_if;
        }
        return  TBS_NFP_CONTINUE;
    }
    else if(TBS_NFP_RX_IS_VLAN(status))
    {
        //出接口是物理接口
        if (!(vir_oifmap->flags & TBS_NFP_F_MAP_VLAN))
        {
            if(vir_iifmap->flags & TBS_NFP_F_MAP_VLAN)
            {
                 tbs_nfp_vlanremove(port, rx_desc, true);
            }

            return TBS_NFP_CONTINUE;
        }
        else //出接口是VLAN接口
        {
            if(vir_iifmap->flags & TBS_NFP_F_MAP_VLAN)
            {
                tbs_nfp_vlanreplace(port, rx_desc, vir_oifmap->vlanid);
            }
            else
            {
                tbs_nfp_vlanadd(port, rx_desc, vir_oifmap->vlanid, true);
            }

            TBS_NFP_DEBUG("oif_name:%s ",vir_oifmap->name);//打印imap name

            /*遍历出接口并添加vlan tag*/
            vir_oifmap = vir_oifmap->parent_if;
            while(vir_oifmap->parent_if)
            {
                TBS_NFP_DEBUG("oif_name:%s ",vir_oifmap->name);//打印imap name
                if(vir_oifmap->flags & TBS_NFP_F_MAP_VLAN)
                {
                    tbs_nfp_vlanadd(port, rx_desc, vir_oifmap->vlanid, true);
                }
                vir_oifmap = vir_oifmap->parent_if;
            }

            TBS_NFP_DEBUG("\n");
            return TBS_NFP_CONTINUE;
        }
    }
    else if(TBS_NFP_RX_IS_QINQ(status))/*QinQ  */
    {
        //出接口不是VLAN设备，是物理设备或无线设备
        if (!(vir_oifmap->flags & TBS_NFP_F_MAP_VLAN))
        {
            /*入接口是vlan设备*/
            if (vir_iifmap->flags & TBS_NFP_F_MAP_VLAN)
            {
                tbs_nfp_vlanremove(port, rx_desc, true);

                /*接收接口为双tag vlan设备*/
                while(vir_iifmap->parent_if)
                {
                    if((vir_iifmap->parent_if->flags & TBS_NFP_F_MAP_VLAN))
                    {
                        tbs_nfp_vlanremove(port, rx_desc, true);
                        break;
                    }
                    vir_iifmap = vir_iifmap->parent_if;
                }
            }

            return TBS_NFP_CONTINUE;
        }
        else //出接口是VLAN设备
        {
            /*接收接口为VLAN设备*/
            if(vir_iifmap->flags & TBS_NFP_F_MAP_VLAN)
            {
                /*接收接口为双tag vlan设备，移除最外层VLAN TAG*/
                while(vir_iifmap->parent_if)
                {
                    if((vir_iifmap->parent_if->flags & TBS_NFP_F_MAP_VLAN))
                    {
                        tbs_nfp_vlanremove(port, rx_desc, true);
                        break;
                    }
                    vir_iifmap = vir_iifmap->parent_if;
                }

                /*接收接口为双TAG,替换第二层TAG;若接收接口为单TAG,替换外层TAG*/
                tbs_nfp_vlanreplace(port, rx_desc, vir_oifmap->vlanid);

                /*遍历出设备并添加vlan tag*/
                vir_oifmap = vir_oifmap->parent_if;
                while(vir_oifmap->parent_if)
                {
                    TBS_NFP_DEBUG("oif_name:%s ",vir_oifmap->name);//打印imap name
                    if(vir_oifmap->flags & TBS_NFP_F_MAP_VLAN)
                    {
                        tbs_nfp_vlanadd(port, rx_desc, vir_oifmap->vlanid, true);
                    }
                    vir_oifmap = vir_oifmap->parent_if;
                }

                return TBS_NFP_CONTINUE;
            }
            else /*接收接口为物理设备*/
            {
                /*遍历出设备并添加vlan tag*/
                while(vir_oifmap->parent_if)
                {
                    TBS_NFP_DEBUG("oif_name:%s ",vir_oifmap->name);//打印imap name
                    if(vir_oifmap->flags & TBS_NFP_F_MAP_VLAN)
                    {
                        tbs_nfp_vlanadd(port, rx_desc, vir_oifmap->vlanid, true);
                    }
                    vir_oifmap = vir_oifmap->parent_if;
                }
                return TBS_NFP_CONTINUE;
            }
        }
    }
    else /*more QinQ*/
    {
        return TBS_NFP_TERMINATE;
    }
}


/*=========================================================================
 inline static int tbs_nfp_vlan_rx(int port, TBS_NFP_RX_DESC *rx_desc,
                     unsigned char *mac_header, TBS_NFP_IF_MAP **ppif_map)

 Description:		若接收的数据包是vlan包，检测是否有与之对应的vlan设备
 Data Accessed:     TBS_NFP_IF_MAP *g_if_map[TBS_NFP_ITF_HASH_SIZE];

 Data Updated:
 Input:			    int port    接收物理接口ifindex
 Output:			TBS_NFP_RX_DESC *rx_desc    数据包信息
                    TBS_NFP_IF_MAP **ppif_map   入接口的ifmap可能会被改变

 Output:  			TBS_NFP_IF_MAP **ppif_map   入接口的ifmap可能被此函数改变
 Return:            TBS_NFP_TERMINATE: 匹配失败，返回协议栈处理
                    TBS_NFP_CONTINUE: 桥规则匹配失败，继续路由匹配
 Others:
=========================================================================*/
static inline int tbs_nfp_vlan_rx(int port, const TBS_NFP_RX_DESC *rx_desc,
		TBS_NFP_IF_MAP **ppif_map)
{
    unsigned short vlanid[2];
    TBS_NFP_IF_MAP  *vlanifmap[2] = {0,0};
    struct vlan_hdr *vhdr = NULL;
    unsigned char *mac_header = rx_desc->mac_header + rx_desc->shift;
    unsigned int status = rx_desc->status;

    //TBS_NFP_INTO_FUNC;

    /*vlan tag packets */
    if (TBS_NFP_RX_IS_VLAN(status))
    {
        vhdr = (struct vlan_hdr *)(mac_header + ETH_HLEN);
        vlanid[0] = ntohs(vhdr->h_vlan_TCI) & VLAN_VID_MASK;
        vlanifmap[0] = tbs_nfp_vlan_ifmap_get(port, vlanid[0]);

        if (vlanifmap[0])
        {
			TBS_NFP_INC(port, vlan_rx_found);
            *ppif_map = vlanifmap[0];

            return TBS_NFP_CONTINUE;
        }

        TBS_NFP_INC(port, vlan_rx_not_found);

#ifdef TBS_NFP_BRIDGE
        if ((*ppif_map)->flags & TBS_NFP_F_MAP_BRIDGE_PORT)
        {
			TBS_NFP_INC(port, vlan_rx_trans);
            /*物理接口在桥中,vlan 透传*/
            return TBS_NFP_CONTINUE;
        }
        else
#endif
        {
            /*物理接口不在桥中，可能错误，将返回至协议栈处理*/
            return TBS_NFP_TERMINATE;
        }
    }
    else if (TBS_NFP_RX_IS_QINQ(status))/*qinq*/
    {
        vhdr = (struct vlan_hdr *)(mac_header + ETH_HLEN);
        vlanid[0] = ntohs(vhdr->h_vlan_TCI) & VLAN_VID_MASK;

        vhdr = (struct vlan_hdr *)(mac_header + ETH_HLEN + sizeof(struct vlan_hdr));
        vlanid[1] = ntohs(vhdr->h_vlan_TCI) & VLAN_VID_MASK;

        vlanifmap[0] = tbs_nfp_vlan_ifmap_get(port, vlanid[0]);
        if(vlanifmap[0])
        {
            TBS_NFP_DEBUG("get vlanmap [0]: %s\n",vlanifmap[0]->name);
            vlanifmap[1] = tbs_nfp_vlan_ifmap_get(vlanifmap[0]->ifidx, vlanid[1]);
            if(vlanifmap[1])
            {
                TBS_NFP_DEBUG("get vlanmap [1]:%s\n",vlanifmap[1]->name);
				TBS_NFP_INC(port, vlan_rx_found);
                *ppif_map = vlanifmap[1];
                return TBS_NFP_CONTINUE;
            }
            else
            {
                TBS_NFP_DEBUG("not get vlanmap [1]:%u\n",vlanid[1]);
                *ppif_map = vlanifmap[0];
#ifdef TBS_NFP_BRIDGE
                if (vlanifmap[0]->flags & TBS_NFP_F_MAP_BRIDGE_PORT)
				{ /*vlanifmap[0]接口在桥中,vlan[1] 透传*/
					TBS_NFP_INC(port, vlan_rx_trans);
					return TBS_NFP_CONTINUE;
				}
                else
#endif
                { /*vlanifmap[0]接口不在桥中，可能错误，将返回至协议栈处理*/
                    return TBS_NFP_TERMINATE;
                }
            }
        }
        else
        {
            TBS_NFP_DEBUG("not get vlanmap [0]:%u\n",vlanid[0]);
#ifdef TBS_NFP_BRIDGE
            if ((*ppif_map)->flags & TBS_NFP_F_MAP_BRIDGE_PORT)
            {
                /*物理接口在桥中,vlan[0]、vlan[1] 透传*/
                return TBS_NFP_CONTINUE;
            }
            else
#endif
            {
                /*物理接口不在桥中，可能错误，将返回至协议栈处理*/
                return TBS_NFP_TERMINATE;
            }
        }
    }

    /*more qinq*/
    return TBS_NFP_TERMINATE;
}


/*=========================================================================
 Function:		static inline bool tbs_nfp_bridge_vlanadd_check(struct sk_buff *skb,
                         const TBS_NFP_IF_MAP *in_ifmap,
                         const TBS_NFP_IF_MAP *in_ifmap)

 Description:		检测skb是否能修改并发送出去
 Data Accessed:
 Data Updated:
 Input:   			struct sk_buff *skb             数据包信息
        			const TBS_NFP_IF_MAP *in_ifmap  入接口ifmap
        			const TBS_NFP_IF_MAP *out_ifmap  出接口ifmap
 Output:
 Return:            TBS_NFP_OK:        能成功修改并发送出去
                    TBS_NFP_TERMINATE: 不能修改，返回协议栈处理
 Others:
=========================================================================*/
static inline bool tbs_nfp_bridge_vlanadd_check(struct sk_buff *skb,
                          const TBS_NFP_IF_MAP *virt_iifmap,
                          const TBS_NFP_IF_MAP *virt_oifmap)
{
    const TBS_NFP_IF_MAP *iifmap = virt_iifmap;

    //vlan 透传
    unsigned int vlan_throught_size;
    //const TBS_NFP_IF_MAP *oifmap = virt_oifmap;
    int shift = 0;

    //遍历进设备中vlan设备数，获取数据包vlan 头字节数
    while(iifmap->parent_if)
    {
        if (iifmap->flags & TBS_NFP_F_MAP_VLAN)
        {//改包过程中向右移4字节
            shift+=sizeof(struct vlan_hdr);
        }
        iifmap = iifmap->parent_if;
    }

    #if 0
    while(oifmap->parent_if)
    {
        if (oifmap->flags & TBS_NFP_F_MAP_VLAN)
        {//改包过程中向左移4字节
            shift-=4;
        }
        oifmap = oifmap->parent_if;
    }

    // skb->data - skb->head < -shift
    if (skb->data - skb->head < (-1)*shift)
    {
        /*检查skb头部预留长度是否足够添加-shift个字节*/
        TBS_NFP_ERROR("skb data-head(%u) no enough space(%d)!!!\n",(unsigned int)skb->data-(unsigned int)skb->head,shift);
        return false;
    }
    #endif //if 0

    //根据标准协议栈桥转发时，一个VLAN TAG透传的包，VLAN TAG长度不做mtu的判断
    vlan_throught_size = (*((__be16 *)(skb->mac_header + 12 + shift)) == htons(ETH_P_8021Q) ? VLAN_HLEN : 0);
    shift = shift + vlan_throught_size;
    if (((skb->len-shift) > virt_oifmap->mtu)
         && !skb_is_gso(skb))
    {
        //无法发送，返回协议栈，
        TBS_NFP_ERROR("skb isnot gso and skb length(%u) is larger than mtu(%u)!!\n",skb->len-shift,virt_oifmap->mtu);
        return false;
    }

    return true;
}



/*=========================================================================
 Function:		static inline bool tbs_nfp_vlanremove_check(const TBS_NFP_RX_DESC  *rx_desc,
                          const TBS_NFP_IF_MAP *virt_iifmap)

 Description:		进入ip转发前对packet中vlan tag与接收接口做匹配检查
 Data Accessed:
 Data Updated:
 Input:   			TBS_NFP_RX_DESC  *rx_desc       接收packet描述信息
        			const TBS_NFP_IF_MAP *virt_iifmap接收vlan 接口
 Output:
 Return:            true: 检查通过；false: 检查不通过
 Others:
=========================================================================*/
static inline bool tbs_nfp_vlanremove_check(const TBS_NFP_RX_DESC  *rx_desc,
                          const TBS_NFP_IF_MAP *virt_iifmap)
{
    int vlan_tag_floor = 0;

    //遍历进设备中vlan设备数，获取vlan tag层数
    while(virt_iifmap->parent_if)
    {
        if (virt_iifmap->flags & TBS_NFP_F_MAP_VLAN)
        {
            vlan_tag_floor++;
        }
        virt_iifmap = virt_iifmap->parent_if;
    }

    /*最多支持2层vlan tag*/
    if((TBS_NFP_RX_IS_VLAN(rx_desc->status) && 1 == vlan_tag_floor)
        || (TBS_NFP_RX_IS_QINQ(rx_desc->status) && 2 == vlan_tag_floor))
    {
        return true;
    }

    return false;
}


/*=========================================================================
 Function:		  static inline void tbs_nfp_routefw_vlan_update(int port,
    TBS_NFP_RX_DESC *rx_desc,TBS_NFP_IF_MAP  *out_ifmap)

 Description:	    路由转发vlan tag更新
 Data Accessed:
 Data Updated:
 Input:             TBS_NFP_RX_DESC *prx_desc       接收的数据包描述结构体的指针
                    TBS_NFP_IF_MAP  *pout_ifmap     输出的接口map的指针
                    int port                        物理接收接口
 Output:            无
 Return:            TBS_NFP_CONTINUE:  成功 继续执行
                    TBS_NFP_TERMINATE: 失败，返回协议栈处理
 Others:
=========================================================================*/
static inline void tbs_nfp_routefw_vlan_update(TBS_NFP_RX_DESC *rx_desc,
    TBS_NFP_IF_MAP  *out_ifmap, int port)
{
    const TBS_NFP_IF_MAP *vir_oifmap = out_ifmap;
    unsigned int status = rx_desc->status;

    //TBS_NFP_INTO_FUNC;

    if(TBS_NFP_RX_IS_VLAN(status))
    {
        //出接口是物理接口
        if (!(vir_oifmap->flags & TBS_NFP_F_MAP_VLAN))
        {
            tbs_nfp_vlanremove(port, rx_desc, false);
        }
        else //出接口是VLAN接口
        {
            tbs_nfp_vlanreplace(port, rx_desc, vir_oifmap->vlanid);
            vir_oifmap = vir_oifmap->parent_if;
        }
    }
    else if(TBS_NFP_RX_IS_QINQ(status))
    {
        tbs_nfp_vlanremove(port, rx_desc, false);

        //出接口是物理接口
        if (!(vir_oifmap->flags & TBS_NFP_F_MAP_VLAN))
        {
            tbs_nfp_vlanremove(port, rx_desc, false);
        }
        else //出接口是VLAN接口
        {
            tbs_nfp_vlanreplace(port, rx_desc, vir_oifmap->vlanid);
            vir_oifmap = vir_oifmap->parent_if;
        }
    }

    /*逐层添加vlan tag*/
    while(vir_oifmap->parent_if)
    {
        if(vir_oifmap->flags & TBS_NFP_F_MAP_VLAN)
        {
            tbs_nfp_vlanadd(port, rx_desc, vir_oifmap->vlanid, false);
        }

        vir_oifmap = vir_oifmap->parent_if;
    }
}
#endif /* TBS_NFP_VLAN */


#ifdef TBS_NFP_MCF

#ifdef TBS_NFP_BRIDGE

static inline __tbs_nfp_mc_forward(int rx_port, TBS_NFP_IF_MAP  *in_ifmap,
    TBS_NFP_RX_DESC *rx_desc, TBS_NFP_IF_MAP **phys_oif, bool forward)
{
    int status = TBS_NFP_TERMINATE;

    /*slooping enable*/
    if(slooping)
    {
    }
    else    /*flood*/
    {
        /*遍历br_port*/
    }
}

static inline tbs_nfp_mc_br_forward(int rx_port, TBS_NFP_IF_MAP  *in_ifmap,
    TBS_NFP_RX_DESC *rx_desc, TBS_NFP_IF_MAP **phys_oif)
{
    return __tbs_nfp_mc_forward(rx_port, in_ifmap, rx_desc, phys_oif, true);
}
#endif  /*TBS_NFP_BRIDGE*/


static inline tbs_nfp_mr_forward(int rx_port, TBS_NFP_IF_MAP  *in_ifmap,
    TBS_NFP_RX_DESC *rx_desc, TBS_NFP_IF_MAP **phys_oif)
{
    int status = TBS_NFP_TERMINATE;
    TBS_NFP_IF_MAP  *phy_oifmap, *out_ifmap = NULL;

    status = tbs_nfp_status_check(rx_port, rx_desc);
    if (TBS_NFP_CONTINUE != status)
    {
        //TBS_NFP_DEBUG("tbs_nfp_status_check fail!!!!!!!!!!!\n");
        return status;
    }

    /*muticast route forwarding*/

    /*mfc lookup*/

    if(out_ifmap->flags & TBS_NFP_F_MAP_BRIDGE)
        return __tbs_nfp_mc_forward(rx_port, in_ifmap, rx_desc, phys_oif, false);
    else
    {
        phy_oifmap = tbs_nfp_if_phys_get(out_ifmap);
        if (unlikely(!phy_oifmap ||
                !(phy_oifmap->flags & (TBS_NFP_F_MAP_INT | TBS_NFP_F_MAP_WLAN))))
        {
            TBS_NFP_ERROR("get phys_if fail!!!!!!!!!!!\n");
            status = TBS_NFP_TERMINATE;
            return status;
        }


#if (defined(TBS_NFP_VLAN) || defined(TBS_NFP_PPP))
        if (unlikely(!tbs_nfp_pppvlanadd_check(rx_desc, out_ifmap)))
        {
            TBS_NFP_ERROR("tbs_nfp_pppvlanadd_check fail, mtu(%s) too small!!!!!!!!!!!\n",
                out_ifmap->name);

            TBS_NFP_INC(port, tx_mtu_err);
            status = TBS_NFP_TERMINATE;
            return status;
        }
#endif

        /*layer3层校验，ttl更新*/
        tbs_nfp_l3_update(rx_desc);

        /*根据出接口增加数据包ppp头部*/
#ifdef TBS_NFP_PPP
        tbs_nfp_pppoe_update(rx_desc, out_ifmap, rx_port);
#endif

        /*vnet/dummyport应替换成物理接口*/
#if (defined(TBS_NFP_DUMMYPORT) || defined(CONFIG_VNET))
        if(out_ifmap->flags & (TBS_NFP_F_MAP_DUMMYPORT | TBS_NFP_F_MAP_VNET))
        {
            out_ifmap = out_ifmap->parent_if;
        }
#endif

        /*根据出接口增加数据包vlan头部*/
#ifdef TBS_NFP_VLAN
        tbs_nfp_routefw_vlan_update(rx_desc, out_ifmap, rx_port);
#endif

        /*src mac addr update*/

    }
}


#endif  /*TBS_NFP_MCF*/


#ifdef TBS_NFP_PPP

#if defined(CONFIG_PPPOE) || defined(CONFIG_PPPOE_MODULE)
int updateTxAndRxTime(struct net_device *dev, bool bTx, bool bRx);
#endif

/*=========================================================================
 Function:		static inline void tbs_nfp_pppoe_add(TBS_NFP_RX_DESC *rx_desc,
                                                __be16 vid, bool moveMac)

 Description:		添加vlan tag
 Data Accessed:
 Data Updated:
 Input:			    TBS_NFP_RX_DESC *rx_desc 数据包描述结构体指针
                    __be16 vid               添加的vlan tag的vlan id
                    bool moveMac             是否需要偏移mac地址
 Output:			无
 Return:			-4  mac头向左移4位

 author: tbs baiyonghui 2012-1-12
=========================================================================*/
static inline void tbs_nfp_pppoe_add(TBS_NFP_RX_DESC *rx_desc, const TBS_NFP_IF_MAP *out_ifmap)
{
	unsigned char *mac_header;
	unsigned char *ip_header;
	struct pppoe_hdr *pppoe_header;

	mac_header = (unsigned char*)(rx_desc->mac_header + rx_desc->shift);
	ip_header = (unsigned char *)(rx_desc->mac_header + rx_desc->l3_offset);


    if(!TBS_NFP_RX_L3_IS_IP4(rx_desc->status))
        return ;
	/* Keep VLAN fields*/
	//memmove(mac_header - PPPOE_SES_HLEN + 12, mac_header + 12, ip_header - mac_header - 12);
	memmove(mac_header - PPPOE_SES_HLEN, mac_header, ip_header - mac_header);
    rx_desc->shift -= PPPOE_SES_HLEN;

	pppoe_header = (struct pppoe_hdr*)(ip_header - PPPOE_SES_HLEN);

	/* set Ethernet Protocol ID ETH_P_PPP_SES*/
	*((unsigned char *)pppoe_header - 1) = 0x64;
	*((unsigned char *)pppoe_header - 2) = 0x88;

	/* set pppoe header ver type code fields */
	pppoe_header->ver = 0x1;
	pppoe_header->type = 0x1;
	pppoe_header->code = 0x0;
	pppoe_header->sid = out_ifmap->session_id;
	pppoe_header->length = htons(ntohs(((struct iphdr*)ip_header)->tot_len) + 2);
	pppoe_header->tag[0].tag_type = ntohs(PPP_IP);
	/* set pppoe header session_id out_ifmap's session_id */

	/* set pppoe header session_id out_ifmap's session_id */
}

/*=========================================================================
 Function:		static inline void tbs_nfp_pppoe_remove(TBS_NFP_RX_DESC *rx_desc,
                                                 bool moveMac)

 Description:		移除pppoe header
 Data Accessed:
 Data Updated:
 Input:			    int port                 vid 对应的vlan接口的if_index
                    TBS_NFP_RX_DESC *rx_desc 数据包描述结构体指针
                    bool moveMac             是否需要偏移mac地址
 Output:			无
 Return:			4  mac头向右移4位

 author: tbs baiyonghui 2012-1-12
=========================================================================*/
static inline void tbs_nfp_pppoe_remove(TBS_NFP_RX_DESC *rx_desc)
{
	unsigned char *ip_header;
	/* writing IP ethertype to the new location of ether header */
	ip_header = (unsigned char *)(rx_desc->mac_header + rx_desc->l3_offset);

	/* set Ethernet Protocol ID  ETH_P_IP */
	*(ip_header - 1) = 0x00;
	*(ip_header - 2) = 0x08;

	rx_desc->shift += PPPOE_SES_HLEN;
}

/*=========================================================================
 Function:		static inline void tbs_nfp_pppoe_replace(int port,
            unsigned char *mac_header, __be16 vid)

 Description:		pppoe头替换
 Data Accessed:     无

 Data Updated:
 Input:			    int port                 vid 对应的vlan接口的if_index
                    TBS_NFP_RX_DESC *rx_desc 数据包描述结构体指针
                    __be16 vid               新的vlan id
 Output:			无
 Output:			无
 Return:			无
 Others:
 author: tbs baiyonghui 2012-1-12
=========================================================================*/
static inline void tbs_nfp_pppoe_replace(const TBS_NFP_IF_MAP *out_ifmap, TBS_NFP_RX_DESC *rx_desc)
{
	 struct pppoe_hdr *pppoe_header;
	 pppoe_header = (struct pppoe_hdr*)(rx_desc->mac_header + rx_desc->l3_offset - PPPOE_SES_HLEN);
	 if (pppoe_header)
	     pppoe_header->sid = out_ifmap->session_id;
}

/*=========================================================================
 Function:		  static inline int tbs_nfp_pppoe_update(TBS_NFP_RX_DESC *prx_desc,
    TBS_NFP_IF_MAP  *pout_ifmap, int phys_oif)

 Description:	  增加pppoe头部

 Data Accessed:
 Data Updated:
 Input:             TBS_NFP_RX_DESC *prx_desc       接收的数据包描述结构体的指针
                    TBS_NFP_IF_MAP  *pout_ifmap     输出的接口map的指针
                    int port                        物理接收接口

 Output:            无

 Return:            TBS_NFP_CONTINUE:  成功 继续执行
                    TBS_NFP_TERMINATE: 失败，返回协议栈处理

 Others:
 author: tbs baiyonghui 2012-1-12
=========================================================================*/
static inline int tbs_nfp_pppoe_update(TBS_NFP_RX_DESC *rx_desc,
    TBS_NFP_IF_MAP  **out_ifmap, int port)
{
	/*更新二层头协议号*/
    unsigned int status = rx_desc->status;

	if(TBS_NFP_RX_IS_PPP(status))
	{
		if((*out_ifmap)->flags & TBS_NFP_F_MAP_PPPOE)
		{
			tbs_nfp_pppoe_replace(*out_ifmap, rx_desc);
            *out_ifmap = (*out_ifmap)->parent_if;

			/*增加PPPOE接口的数据包计数*/
			TBS_NFP_INC(port, pppoe_tx_replace);
		}
		else
		{
			//去struct pppoe_hdr 头信息
			tbs_nfp_pppoe_remove(rx_desc);
			TBS_NFP_INC(port, pppoe_tx_remove);
		}
	}
	else if((*out_ifmap)->flags & TBS_NFP_F_MAP_PPPOE)
	{
        /*刷新ppp发送时间*/
        updateTxAndRxTime((*out_ifmap)->dev, true, false);

		/* add pppoe header */
		tbs_nfp_pppoe_add(rx_desc, *out_ifmap);
        *out_ifmap = (*out_ifmap)->parent_if;

		TBS_NFP_INC(port, pppoe_tx_add);
	}

	return TBS_NFP_CONTINUE;
}



/*=========================================================================
 inline static int tbs_nfp_ppp_rx(int port, TBS_NFP_RX_DESC *rx_desc,
                     unsigned char *mac_header)

 Description:		若接收的数据包是vlan包，检测是否有与之对应的vlan设备
 Data Accessed:     TBS_NFP_IF_MAP *g_if_map[TBS_NFP_ITF_HASH_SIZE];

 Data Updated:
 Input:			    int port    接收物理接口ifindex
 Output:			TBS_NFP_RX_DESC *rx_desc    数据包信息

 Output:  			TBS_NFP_IF_MAP **ppif_map   入接口的ifmap可能被此函数改变
 Return:            TBS_NFP_TERMINATE: 匹配失败，返回协议栈处理
                    TBS_NFP_CONTINUE: 桥规则匹配失败，继续路由匹配
 Others:
=========================================================================*/
static inline int tbs_nfp_pppoe_rx(int port, const TBS_NFP_RX_DESC *rx_desc, TBS_NFP_IF_MAP **in_ifmap)
{
    TBS_NFP_IF_MAP *ppp_ifmap = NULL;
	struct pppoe_hdr *ppphdr = NULL;

#if defined(CONFIG_VNET) || defined (TBS_NFP_DUMMYPORT)
    TBS_NFP_IF_MAP *virt_ifmap = NULL;
#endif

    if(NULL == in_ifmap || NULL == *in_ifmap)
        return TBS_NFP_TERMINATE;

	if(TBS_NFP_RX_IS_PPP(rx_desc->status))
	{
		ppphdr = (struct pppoe_hdr *)(rx_desc->mac_header + rx_desc->l3_offset - PPPOE_SES_HLEN);

#if defined(CONFIG_VNET) || defined (TBS_NFP_DUMMYPORT)
        virt_ifmap = (*in_ifmap)->virt_if;
		while(virt_ifmap)
		{
			if((virt_ifmap->flags & TBS_NFP_F_MAP_VNET) ||
					(virt_ifmap->flags & TBS_NFP_F_MAP_DUMMYPORT))
			{
				if(NULL != (ppp_ifmap = tbs_nfp_pppoe_ifmap_get(port, virt_ifmap, ppphdr->sid)))
				{
                    /*刷新ppp接收时间*/
                    updateTxAndRxTime(ppp_ifmap->dev, false, true);

                    *in_ifmap = ppp_ifmap;
                    TBS_NFP_INC(port, pppoe_rx_found);

                    return TBS_NFP_CONTINUE;
                }
			}

			virt_ifmap = virt_ifmap->virt_next;
		}
#endif

		if(NULL !=
            (ppp_ifmap = tbs_nfp_pppoe_ifmap_get(port, *in_ifmap, ppphdr->sid)))
		{
            *in_ifmap = ppp_ifmap;
            TBS_NFP_INC(port, pppoe_rx_found);

			return TBS_NFP_CONTINUE;
		}

        TBS_NFP_INC(port, pppoe_rx_not_found);

		return TBS_NFP_TERMINATE;
	}

	return TBS_NFP_CONTINUE;
}
#endif  /* TBS_NFP_PPP */


#if (defined(TBS_NFP_VLAN) || defined(TBS_NFP_PPP))
/*=========================================================================
 Function:		  static inline bool tbs_nfp_pppvlanadd_check(const TBS_NFP_RX_DESC *prx_desc,
    const TBS_NFP_IF_MAP  *out_ifmap)

 Description:		检查数据包能否增加ppp头和vlan头后还大于mtu
 Data Accessed:
 Data Updated:
 Input:             TBS_NFP_RX_DESC *prx_desc         被检查的数据包描述结构指针
                    TBS_NFP_IF_MAP  *out_ifmap        数据包的发送接口imap指针
 Output:            无

 Return:            true:  能被发送出去
                    false: 不能发送出去
 Others:
=========================================================================*/
static inline bool tbs_nfp_pppvlanadd_check(const TBS_NFP_RX_DESC *prx_desc,
    const TBS_NFP_IF_MAP  *out_ifmap)
{
    int data_shift = 0;

    /*跳过虚接口*/
#if (defined(TBS_NFP_DUMMYPORT) || defined(CONFIG_VNET))
    if(out_ifmap->flags & (TBS_NFP_F_MAP_DUMMYPORT | TBS_NFP_F_MAP_VNET))
    {
        out_ifmap = out_ifmap->parent_if;
        if(!out_ifmap)
            return false;
    }
#endif


    /*pppoe头长度计算*/
#ifdef TBS_NFP_PPP
    if(TBS_NFP_RX_IS_PPP(prx_desc->status))
        data_shift -= PPPOE_SES_HLEN;

    if(out_ifmap->flags & TBS_NFP_F_MAP_PPPOE)
    {
        data_shift += PPPOE_SES_HLEN;
        out_ifmap = out_ifmap->parent_if;
        if(!out_ifmap)
            return false;
    }
#endif

    /*vlan header计算*/
#ifdef TBS_NFP_VLAN
    if(TBS_NFP_RX_IS_VLAN(prx_desc->status))
        data_shift -= VLAN_HLEN;
    else if(TBS_NFP_RX_IS_QINQ(prx_desc->status))
        data_shift -= VLAN_HLEN * 2;

    while(out_ifmap && (out_ifmap->flags & TBS_NFP_F_MAP_VLAN))
    {
//        data_shift += VLAN_HLEN;
        out_ifmap = out_ifmap->parent_if;
    }
#endif

    /*mtu check*/
    if(prx_desc->skb->len + data_shift > out_ifmap->mtu)
        return false;

    return  true;
}
#endif


#ifdef TBS_NFP_BRIDGE
/*=========================================================================
 Function:		static inline int tbs_nfp_bridge_rx(int port, TBS_NFP_RX_DESC *rx_desc,
                    const TBS_NFP_IF_MAP *in_ifmap)

 Description:		桥转发处理数据包接收函数
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    int port    接收物理接口ifindex
        			TBS_NFP_RX_DESC *rx_desc    数据包信息
        			const TBS_NFP_IF_MAP *in_ifmap  入接口ifmap
 Output:
 Return:            TBS_NFP_OK: 转发成功；TBS_NFP_TERMINATE: 转发失败，返回协议栈处理
                    TBS_NFP_CONTINUE: 桥规则匹配失败，继续路由匹配
 Others:
=========================================================================*/
static inline int tbs_nfp_bridge_rx(int port,
          TBS_NFP_RX_DESC  *rx_desc, const TBS_NFP_IF_MAP *virt_iifmap,
          TBS_NFP_IF_MAP **phys_oif, TBS_NFP_RULE_BRIDGE **local_in)
{
    unsigned char *dmac = rx_desc->mac_header + rx_desc->shift;
    unsigned char *smac = dmac + ETH_ALEN;
    TBS_NFP_IF_MAP *phy_oifmap = NULL;
    TBS_NFP_IF_MAP *virt_oifmap = NULL;
    int status = TBS_NFP_CONTINUE;

    //TBS_NFP_INTO_FUNC;
#ifdef TBS_NFP_BRIDGE_RULE
    TBS_NFP_RULE_BRIDGE *bridge_rule = NULL;

    bridge_rule = tbs_nfp_bridge_rule_lookup(dmac, smac, virt_iifmap->ifidx);
    if(bridge_rule)
    {
        TBS_NFP_INC(port, bridge_hit);

        virt_oifmap = tbs_nfp_ifmap_get(bridge_rule->oif);
        if(unlikely(!virt_oifmap))
        {
            TBS_NFP_ERROR("tbs_nfp_ifmap_get(%d) fail!!!!!!!!!!!\n", bridge_rule->oif);

            return TBS_NFP_TERMINATE;
        }

        /*到br的继续路由转发*/
        if(virt_oifmap->flags & TBS_NFP_F_MAP_BRIDGE)
        {
            *local_in = bridge_rule;
            TBS_NFP_INC(port, bridge_local);

            return TBS_NFP_CONTINUE;
        }

        phy_oifmap = tbs_nfp_if_phys_get(virt_oifmap);
        if (!phy_oifmap ||
            !(phy_oifmap->flags & (TBS_NFP_F_MAP_INT | TBS_NFP_F_MAP_WLAN)))
        {
            return TBS_NFP_TERMINATE;
        }

#ifdef TBS_NFP_VLAN
        /*判断数据包能否增加需要的长度并发送出去*/
        if(!tbs_nfp_bridge_vlanadd_check(rx_desc->skb, virt_iifmap, virt_oifmap))
        {
            TBS_NFP_WARN("tbs_nfp_bridge_vlanadd_check fail, return terminate\n");
            return TBS_NFP_TERMINATE;
        }

        /*更新数据包的vlan tag，加vlan tag，去vlan tag，改vlan tag, 并记录mac头部的偏移量*/
        status = tbs_nfp_bridge_vlan_update(rx_desc, virt_iifmap, virt_oifmap, port);
        if (TBS_NFP_CONTINUE != status)
            return status;
#endif

        /*发送数据包*/
        *phys_oif = phy_oifmap;
        bridge_rule->count++;

        return TBS_NFP_OK;
    }

    TBS_NFP_INC(port, bridge_miss);

    //桥转发规则不存在情况下，也尝试进行路由查找
    return TBS_NFP_CONTINUE;

#endif  /*TBS_NFP_BRIDGE_RULE*/

#ifdef TBS_NFP_FDB
    /*fdb规则转发*/
#endif  /*TBS_NFP_FDB*/

    return TBS_NFP_TERMINATE;
}
#endif /*TBS_NFP_BRIDGE*/


#ifdef TBS_NFP_CT
/*=========================================================================
  Function:		  static inline int tbs_nfp_five_tuple_process(TBS_NFP_SKB_5TUPLE *pskb_5tuple)

 Description:		5元组过滤
 Data Accessed:     5元组filter
 Data Updated:
 Input:             TBS_NFP_SKB_5TUPLE  *pskb_5tuple 指向接收数据包的5元组结构

 Output:            无

 Return:            TBS_NFP_CONTINUE:  未被过滤，可走加速
                    TBS_NFP_TERMINATE: 过滤，不能走加速，返回协议栈处理

 Others:
=========================================================================*/
 static inline int tbs_nfp_five_tuple_process(TBS_NFP_SKB_5TUPLE *pskb_5tuple)
 {
      //TBS_NFP_INTO_FUNC;

      return   TBS_NFP_CONTINUE;
 }


#ifdef TBS_NFP_NAT
/*=========================================================================
 Function:		  static inline int tbs_nfp_nat_update(TBS_NFP_RX_DESC *prx_desc,TBS_NFP_IF_MAP  *pout_ifmap)

 Description:	  nat 处理

 Data Accessed:
 Data Updated:
 Input:             TBS_NFP_RX_DESC *prx_desc       接收的数据包描述结构体的指针
                    TBS_NFP_IF_MAP  *pout_ifmap     输出的接口map的指针
                    int port                        物理接收接口
 Output:            无

 Return:            TBS_NFP_CONTINUE:  成功 继续执行
                    TBS_NFP_TERMINATE: 失败，返回协议栈处理

 Others:
=========================================================================*/
static inline int tbs_nfp_nat_update(TBS_NFP_RX_DESC *prx_desc,
        ST_TUPLE_HASH *ct_dir_tuple, int port)
{
    struct iphdr *ip_header = (struct iphdr *)(prx_desc->mac_header + prx_desc->l3_offset);
    struct tcphdr *tcp_header = (struct tcphdr *)((unsigned char *)ip_header + sizeof(struct iphdr));
    struct udphdr *udp_header = (struct udphdr *)tcp_header;

    unsigned int oldip;
    unsigned int newip;
    unsigned short  oldport;
    unsigned short  newport;
	ST_TUPLE_HASH *opposite_dir_ct = NULL;

	opposite_dir_ct = GET_NEXT_DIR_TUPLE(ct_dir_tuple);

    //snat
    if (ct_dir_tuple->flags & TBS_NFP_F_CT_SNAT)
    {
        oldip = ip_header->saddr;
        newip = *((unsigned int *)opposite_dir_ct->dst_l3);
        oldport = tcp_header->source;
		newport = *(((unsigned short *)&opposite_dir_ct->ports) + 1);

        ip_header->saddr = newip;
        tcp_header->source = newport;

        TBS_NFP_INC(port, snat_hit);
    }

    if (ct_dir_tuple->flags & TBS_NFP_F_CT_DNAT)
    {
        oldip = ip_header->daddr;
		newip = *((unsigned int *)opposite_dir_ct->src_l3);
        oldport = tcp_header->dest;

        //ct_dir_tuple->new_dport;
        newport = *((unsigned short *)&opposite_dir_ct->ports);

        ip_header->daddr = newip;
        tcp_header->dest = newport;

        TBS_NFP_INC(port, dnat_hit);
    }

    //tcp 校验
    if(TBS_NFP_RX_L4_IS_TCP(prx_desc->status))
    {
        tcp_header->check = csum_fold(ip_vs_check_diff4(oldip, newip,
        			 ip_vs_check_diff2(oldport, newport,
        				~csum_unfold(tcp_header->check))));
    }
    else//udp 校验
    {
        if(udp_header->check != 0)
        {
	    	udp_header->check = csum_fold(ip_vs_check_diff4(oldip, newip,
					 ip_vs_check_diff2(oldport, newport,
						~csum_unfold(udp_header->check))));

           if (!udp_header->check)
            	udp_header->check = CSUM_MANGLED_0;
        }
     }

    return   TBS_NFP_CONTINUE;
}
#endif  /*TBS_NFP_NAT*/

#endif /* TBS_NFP_CT */



/*=========================================================================
 Function:		  static inline void tbs_nfp_5tuple_get(TBS_NFP_RX_DESC *prx_desc,
                                                                TBS_NFP_SKB_5TUPLE *skb_5tuple)


 Description:		从prx_desc指向的结构中获取接收的数据包的5元组信息，保存至 TBS_NFP_SKB_5TUPLE *skb_5tuple指向的结构
 Data Accessed:     无
 Data Updated:      无
 Input:             TBS_NFP_RX_DESC    *prx_desc        接收的数据包描述结构指针
                    TBS_NFP_SKB_5TUPLE *skb_5tuple      需要被填充的结构体的指针

 Output:            TBS_NFP_SKB_5TUPLE *skb_5tuple      需要被填充的结构体的指针

 Return:            返回被填充的结构体的指针

 Others:
=========================================================================*/

static inline void tbs_nfp_5tuple_get(TBS_NFP_RX_DESC *prx_desc, TBS_NFP_SKB_5TUPLE *skb_5tuple)
 {
    struct iphdr *ipv4_header = NULL;
    struct tcphdr *tcpudp_header = NULL;
    struct ipv6hdr *ipv6_header = NULL;

    //TBS_NFP_INTO_FUNC;

    /*ipv4 包*/
    if (TBS_NFP_RX_L3_IS_IP4(prx_desc->status))
    {
        ipv4_header = (struct iphdr*)(prx_desc->mac_header + prx_desc->l3_offset);
        tcpudp_header = (struct tcphdr*)((unsigned char *)ipv4_header + (ipv4_header->ihl * 4));
        skb_5tuple->family = AF_INET;
        skb_5tuple->src_l3 = (unsigned char *)&(ipv4_header->saddr);
        skb_5tuple->dst_l3 = (unsigned char *)&(ipv4_header->daddr);
        skb_5tuple->proto = ipv4_header->protocol;
        skb_5tuple->ports = *((unsigned int *)&(tcpudp_header->source));
    }
    else //ipv6 数据包
    {
        skb_5tuple->family = AF_INET6;

        ipv6_header = (struct ipv6hdr*)(prx_desc->mac_header + prx_desc->l3_offset);
        tcpudp_header = (struct tcphdr*)((unsigned char *)ipv6_header + sizeof(struct ipv6hdr));
        skb_5tuple->family = AF_INET;
        skb_5tuple->src_l3 = (unsigned char *)&(ipv6_header->saddr);
        skb_5tuple->dst_l3 = (unsigned char *)&(ipv6_header->daddr);
        skb_5tuple->proto = ipv6_header->nexthdr;
        skb_5tuple->ports = *((unsigned int *)&(tcpudp_header->source));
    }

    return;
 }


/*=========================================================================
 Function:		  inline int tbs_nfp_status_check(int port, TBS_NFP_RX_DESC *rx_desc)

 Description:		数据包类型检查函数，若1)l4 协议非tcp/ucp(即三层未知和四层未知)
                                          2)分片报文 返回协议栈处理
 Data Accessed:
 Data Updated:
 Input:            TBS_NFP_RX_DESC *rx_desc

 Output:
 Return:            TBS_NFP_CONTINUE:tcp/udp包；继续往下执行
                    TBS_NFP_TERMINATE: 不能走加速，返回协议栈处理

 Others:
=========================================================================*/
static inline int tbs_nfp_status_check(int port, TBS_NFP_RX_DESC *prx_desc)
{
    /*若 TBS_NFP_RX_L3_UNKNOWN_MASK || TBS_NFP_RX_L4_UNKNOWN_MASK ||分片 返回协议栈处理*/
    struct iphdr *ipv4_header = NULL;
    struct ipv6hdr *ipv6_header = NULL;
    struct tcphdr *tcp_hdr  = NULL;
    unsigned char *l3_hdr = prx_desc->mac_header + prx_desc->l3_offset;

    //TBS_NFP_INTO_FUNC;

    /*layer3未知包，layer4未知包(包括ipv6的分片)，返回协议站处理*/
    if(TBS_NFP_RX_L3_UNKNOW(prx_desc->status))
    {
        TBS_NFP_INC(port, non_ip);
        return TBS_NFP_TERMINATE;
    }

    if(TBS_NFP_RX_L4_UNKNOW(prx_desc->status))
    {
        TBS_NFP_INC(port, l4_unknown);
        return TBS_NFP_TERMINATE;
    }

    /*ipv4分片包，返回协议栈处理*/
    if(TBS_NFP_RX_L3_IS_IP4(prx_desc->status))
    {
        ipv4_header = (struct iphdr *)(l3_hdr);

        if(unlikely(ipv4_header->frag_off & htons(IP_MF | IP_OFFSET)))
        {
            TBS_NFP_INC(port, ipv4_rx_frag);
            return TBS_NFP_TERMINATE;
        }

        if (unlikely(ipv4_header->ttl < 1))
        {
            TBS_NFP_INC(port, ttl_exp);
            return TBS_NFP_TERMINATE;
        }

        TBS_NFP_INC(port, ipv4);
    }
    else
    {
        ipv6_header = (struct ipv6hdr *)(l3_hdr);
        if(unlikely(ipv6_header->hop_limit < 1))
        {
            TBS_NFP_INC(port, ttl_exp);
            return TBS_NFP_TERMINATE;
        }

        TBS_NFP_INC(port, ipv6);
    }

    /*tcp特殊状态过滤FIN/SYN/RST*/
    if(TBS_NFP_RX_L4_IS_TCP(prx_desc->status))
    {
        if(TBS_NFP_RX_L3_IS_IP4(prx_desc->status))
        {
            tcp_hdr = (struct tcphdr *)(l3_hdr + sizeof(struct iphdr));
        }
        else
        {
            tcp_hdr = (struct tcphdr *)(l3_hdr + sizeof(struct ipv6hdr));
        }

        if(tcp_hdr->syn || tcp_hdr->rst || tcp_hdr->fin)
        {
            TBS_NFP_INC(port, ct_tcp_fin_rst);
            return TBS_NFP_TERMINATE;
        }
    }

    return TBS_NFP_CONTINUE;
}

#ifdef TBS_NFP_FIB
/*=========================================================================
 Function:		  static inline int tbs_nfp_skb_routefw_vlan_update(TBS_NFP_RX_DESC *rx_desc,TBS_NFP_IF_MAP  *pout_ifmap)

 Description:	  更新mac头地址

 Data Accessed:
 Data Updated:
 Input:             TBS_NFP_RX_DESC *rx_desc       接收的数据包描述结构体的指针
                    TBS_NFP_IF_MAP  *pout_ifmap     输出的接口map的指针

 Output:            无

 Return:            TBS_NFP_CONTINUE:  成功 继续执行
                    TBS_NFP_TERMINATE: 失败，返回协议栈处理

 Others:
=========================================================================*/
static inline int tbs_nfp_mac_update(TBS_NFP_RX_DESC *rx_desc, const TBS_NFP_RULE_FIB *fib)
{
    unsigned char *dst_mac = rx_desc->mac_header + rx_desc->shift;

    memcpy(dst_mac,(unsigned char *)fib->da, ETH_ALEN);
    memcpy((dst_mac + ETH_ALEN), fib->sa, ETH_ALEN);

    return   TBS_NFP_CONTINUE;
}



/*=========================================================================
 Function:		  static inline void tbs_nfp_l3_update(TBS_NFP_RX_DESC *prx_desc)

 Description:	    ttl更新，校验和计算

 Data Accessed:
 Data Updated:
 Input:             TBS_NFP_RX_DESC *prx_desc       接收的数据包描述结构体的指针

 Output:            无
 Return:            无
 Others:
=========================================================================*/
static inline void tbs_nfp_l3_update(TBS_NFP_RX_DESC *prx_desc)
{
    unsigned char *l3_hdr = prx_desc->mac_header + prx_desc->l3_offset;
    struct iphdr *ip_header = NULL;
    struct ipv6hdr *ipv6_header = NULL;

    if(TBS_NFP_RX_L3_IS_IP4(prx_desc->status))
    {
        ip_header = (struct iphdr *)(l3_hdr);

        /*ip头部校验*/
        ip_header->ttl--;
        ip_header->check = 0;
        ip_header->check = ip_fast_csum((unsigned char *)ip_header, ip_header->ihl);
    }
    else
    {
        ipv6_header = (struct ipv6hdr *)(l3_hdr);
        ipv6_header->hop_limit--;
        /*ipv6校验*/
    }

    return;
}


/*=========================================================================
 Function:		    static inline int tbs_nfp_ip_fw(int port, TBS_NFP_RX_DESC *rx_desc)
 Description:		路由转发处理
 Data Accessed:
 Data Updated:
 Input:			    int port    接收物理接口ifindex
        			TBS_NFP_RX_DESC *rx_desc    接收packet描述
 Output:            TBS_NFP_IF_MAP **phys_oif   发送的物理接口

 Return:            TBS_NFP_OK: 规则匹配改包成功；
                    其他: 匹配规则失败，返回协议栈处理
 Others:
=========================================================================*/
static inline int tbs_nfp_ip_fw(int port, TBS_NFP_RX_DESC *rx_desc,
    TBS_NFP_IF_MAP **phys_oif)
{
	TBS_NFP_IF_MAP  *out_ifmap;
	TBS_NFP_IF_MAP  *phy_oifmap;
    TBS_NFP_RULE_FIB *fib = NULL;
    TBS_NFP_SKB_5TUPLE  skb_5tuple;
    int status = TBS_NFP_CONTINUE;

#ifdef TBS_NFP_CT
	ST_TUPLE_HASH* ct_dir_tuple = NULL;
	TBS_NFP_TUPLE_CT * ct_tuple = NULL;
#endif

    /*获取skb 5元组信息*/
    tbs_nfp_5tuple_get(rx_desc,&skb_5tuple);

#ifdef TBS_NFP_CT
	//处理5元组的过滤
	status = tbs_nfp_five_tuple_process(&skb_5tuple);
	if (unlikely(TBS_NFP_CONTINUE != status))
	{
        TBS_NFP_INC(port, ct_filter);
		return status;
	}

#if 0
	TBS_NFP_DEBUG("skb 5 tuple family:%u,src:%pI4,dst:%pI4,sport:%u,dport:%u,proto:%u\n",
       skb_5tuple.family, skb_5tuple.src_l3, \
       skb_5tuple.dst_l3, ntohs(*(unsigned short *)(&skb_5tuple.ports)),ntohs(*((unsigned short *)&skb_5tuple.ports+1)),\
       skb_5tuple.proto );
#endif
    ct_dir_tuple = tbs_nfp_ct_lookup_by_tuple(skb_5tuple.family, skb_5tuple.src_l3,\
			skb_5tuple.dst_l3, skb_5tuple.ports, \
			skb_5tuple.proto, true);
	if (!ct_dir_tuple)
	{
		//TBS_NFP_DEBUG("conntrack lookup fail!!!!!!!!!!!\n");
		TBS_NFP_INC(port, ct_miss);
		status = TBS_NFP_TERMINATE;

		return status;
	}

    fib = ct_dir_tuple->fib;

    TBS_NFP_INC(port, fib_hit);
    TBS_NFP_INC(port, ct_hit);
#else
    fib = tbs_nfp_fib_lookup(skb_5tuple.family, skb_5tuple.src_l3, skb_5tuple.dst_l3, true);
    if(!fib)
    {
        TBS_NFP_DEBUG("fib lookup fail!!!!!!!!!!!\n");
        TBS_NFP_INC(port, fib_hit);
        status = TBS_NFP_TERMINATE;

        return status;
    }
#endif


	/*获取数据包出接口的ifmap*/
	out_ifmap = tbs_nfp_ifmap_get(fib->oif);
	if (unlikely(!out_ifmap))
	{
		TBS_NFP_ERROR("not out_ifmap!!!!!!!!!!!\n");
		status = TBS_NFP_TERMINATE;
		return status;
	}

	//TBS_NFP_DEBUG("get out_ifmap:%s\n",out_ifmap->name);


	/*在修改包之前就检查物理出接口是否存在
	  若不存在返回协议栈处理*/
	phy_oifmap = tbs_nfp_if_phys_get(out_ifmap);
	if (unlikely(!phy_oifmap ||
			!(phy_oifmap->flags & (TBS_NFP_F_MAP_INT | TBS_NFP_F_MAP_WLAN))))
	{
		TBS_NFP_ERROR("get phys_if fail!!!!!!!!!!!\n");
		status = TBS_NFP_TERMINATE;
		return status;
	}


	/*根据出接口和入接口判断数据包能否增加需要的长度并发送出去，
	  主要检测出接口。当出接口是ppp接口需要增加长度*/
#if (defined(TBS_NFP_VLAN) || defined(TBS_NFP_PPP))
	if (unlikely(!tbs_nfp_pppvlanadd_check(rx_desc, out_ifmap)))
	{
		TBS_NFP_ERROR("tbs_nfp_pppvlanadd_check fail, mtu(%s) too small!!!!!!!!!!!\n",
            out_ifmap->name);

        TBS_NFP_INC(port, tx_mtu_err);
		status = TBS_NFP_TERMINATE;
		return status;
	}
#endif


    /*根据ct规则做nat处理*/
#ifdef TBS_NFP_NAT
    /*ipv6数据包没有nat*/
    if (ct_dir_tuple->flags & (TBS_NFP_F_CT_SNAT | TBS_NFP_F_CT_DNAT))
    {
        status = tbs_nfp_nat_update(rx_desc, ct_dir_tuple, port);
    }
#endif

    /*layer3层校验，ttl更新*/
    tbs_nfp_l3_update(rx_desc);


	/*根据出接口增加数据包ppp头部*/
#ifdef TBS_NFP_PPP
	tbs_nfp_pppoe_update(rx_desc, &out_ifmap, port);
#endif

    /*vnet/dummyport应替换成物理接口*/
#if (defined(TBS_NFP_DUMMYPORT) || defined(CONFIG_VNET))
    if(out_ifmap->flags & (TBS_NFP_F_MAP_DUMMYPORT | TBS_NFP_F_MAP_VNET))
    {
        out_ifmap = out_ifmap->parent_if;
    }
#endif


	/*根据出接口增加数据包vlan头部*/
#ifdef TBS_NFP_VLAN
	tbs_nfp_routefw_vlan_update(rx_desc, out_ifmap, port);
#endif


	/*根据ct中fib、arp信息添加mac头地址*/
	status = tbs_nfp_mac_update(rx_desc, fib);

#ifdef TBS_NFP_CT
    ct_tuple = GET_TUPLE_CT(ct_dir_tuple);
    ct_tuple->count++;
#endif

    fib->count++;
    *phys_oif = phy_oifmap;

    return TBS_NFP_OK;
}
#endif

/*=========================================================================
 Function:		  static inline int tbs_nfp_rx(int rx_port, TBS_NFP_RX_DESC *rx_desc,
    TBS_NFP_IF_MAP **phys_oif)

 Description:		快速转发规则匹配入口
 Data Accessed:     各个子模块中与数据包加速转发相关的数据结构
 Data Updated:      skb->data
 Input:			    int port    接收物理接口ifindex
        			TBS_NFP_RX_DESC *rx_desc    接收packet描述
 Output:            TBS_NFP_IF_MAP **phys_oif   发送的物理接口

 Return:            TBS_NFP_OK: 规则匹配改包成功；
                    其他: 匹配规则失败，返回协议栈处理
 Others:
=========================================================================*/
static inline int tbs_nfp_rx(int rx_port, TBS_NFP_RX_DESC *rx_desc,
    TBS_NFP_IF_MAP **phys_oif)
{
    TBS_NFP_IF_MAP  *in_ifmap = NULL;
    unsigned int    status = TBS_NFP_TERMINATE;

#if defined(CONFIG_VNET)
    TBS_NFP_IF_MAP *pst_vnet = NULL;
#endif

#ifdef TBS_NFP_BRIDGE
    TBS_NFP_RULE_BRIDGE *local_in_rule = NULL;
#endif  /*TBS_NFP_BRIDGE*/

#if defined(TBS_NFP_FIB) && defined(TBS_NFP_BRIDGE)
    TBS_NFP_IF_MAP  *br_ifmap = NULL;
#endif

    //TBS_NFP_DEBUG("*********into tbs_nfp_rx************\n");

    /* Lookup incoming interface (ifindex) 是否存在于ifmap hash表中*/
    in_ifmap = tbs_nfp_ifmap_get(rx_port);
    if (unlikely(!in_ifmap))
    {
        TBS_NFP_INC(rx_port, iif_err);

        return TBS_NFP_TERMINATE;
    }

#ifdef TBS_NFP_VLAN
    //in_ifmap可能被替换为vlan map
    if(!TBS_NFP_RX_ISNOT_VLAN(rx_desc->status)
        && TBS_NFP_CONTINUE != (status = tbs_nfp_vlan_rx(rx_port, rx_desc, &in_ifmap)))
    {
        TBS_NFP_WARN("tbs_nfp_vlan_rx(%d) fail!!!!!!!!!!!\n", rx_port);
        return status;
    }
#endif /* TBS_NFP_VLAN */


#ifdef TBS_NFP_BRIDGE
    if (in_ifmap->flags & TBS_NFP_F_MAP_BRIDGE_PORT)
    {
        //TBS_NFP_DEBUG("indev in bridge\n");
        /* Do bridging: OK - bridging, TERMINATE - slow path, CONTINUE - Routing */
        //if(PACKET_OTHERHOST == skb->pkt_type)
        {
            status = tbs_nfp_bridge_rx(rx_port, rx_desc, in_ifmap, phys_oif, &local_in_rule);
            if (status != TBS_NFP_CONTINUE)
            {
                return status;
            }
        }
    }
#if defined(CONFIG_VNET)
    else if(in_ifmap->virt_if)
    {
        pst_vnet = in_ifmap->virt_if;

        while(pst_vnet)
        {
            if ((pst_vnet->flags & TBS_NFP_F_MAP_VNET) &&
                (pst_vnet->flags & TBS_NFP_F_MAP_BRIDGE_PORT))
            {
                status = tbs_nfp_bridge_rx(rx_port, rx_desc, pst_vnet, phys_oif, &local_in_rule);
                if (status == TBS_NFP_OK)
                {
                    return status;
                }

                /*route forward*/
                if(local_in_rule)
                {
                    in_ifmap = pst_vnet;
                    break;
                }
            }

            pst_vnet = pst_vnet->virt_next;
        }
    }
#endif  /*CONFIG_VNET*/

/*组播二层转发*/
#ifdef TBS_NFP_MCF
    if (PACKET_MULTICAST == rx_desc->skb->pkt_type &&
        in_ifmap->flags & TBS_NFP_F_MAP_BRIDGE_PORT)
    {
        return tbs_nfp_mc_br_forward(rx_port, in_ifmap, rx_desc, phys_oif);
    }
#endif

#endif  /*TBS_NFP_BRIDGE*/


#ifdef TBS_NFP_VLAN
    /*vlan tag 层数与接收设备匹配检查*/
    if(!TBS_NFP_RX_ISNOT_VLAN(rx_desc->status)
        && !tbs_nfp_vlanremove_check(rx_desc, in_ifmap))
    {
        TBS_NFP_WARN("tbs_nfp_vlanremove_check fail!!!!!!!!!!!\n");
        TBS_NFP_INC(rx_port, vlan_rx_not_found);

        return TBS_NFP_TERMINATE;
    }
#endif /*TBS_NFP_VLAN*/

    /*muticast forwarding*/
#ifdef TBS_NFP_MCF
    if(PACKET_MULTICAST == rx_desc->skb->pkt_type)
        return tbs_nfp_mr_forward(rx_port, in_ifmap, rx_desc, phys_oif);
#endif  /*TBS_NFP_MCF*/


#ifdef TBS_NFP_FIB
#ifdef TBS_NFP_BRIDGE /*桥接匹配后，继续路由转发匹配*/
    if (in_ifmap->flags & TBS_NFP_F_MAP_BRIDGE_PORT)
    {
        #if 0
        /*桥接收返回状态为conntinue,可能是br接收的包，
        pkt_type为otherhost即macdest为br地址时走路由。*/
        br_ifmap = tbs_nfp_ifmap_get(in_ifmap->bridge_if);
        if (br_ifmap && compare_ether_addr(br_ifmap->mac, rx_desc->mac_header + rx_desc->shift))
        {
            TBS_NFP_WARN("br_forward local in check fail!!!!!!!!!!!\n");

            return TBS_NFP_TERMINATE;
        }
        #endif
        br_ifmap = tbs_nfp_ifmap_get(in_ifmap->bridge_if);

        //in_ifmap更新为bridge interface
        if(br_ifmap)
            in_ifmap = br_ifmap;
    }
#endif /* NFP_BRIDGE */


    /*tbs nfp route fastpath*/
    /*检查数据包是否可走路由加速转发，
    未知数据包，或者分片包等，返回协议栈处理*/
    status = tbs_nfp_status_check(rx_port, rx_desc);
    if (TBS_NFP_CONTINUE != status)
    {
        //TBS_NFP_DEBUG("tbs_nfp_status_check fail!!!!!!!!!!!\n");
        return status;
    }

    /*in_ifmap可能被替换为PPP map*/
#ifdef TBS_NFP_PPP
    status = tbs_nfp_pppoe_rx(rx_port, rx_desc, &in_ifmap);
    if (unlikely(TBS_NFP_CONTINUE != status))
    {
        TBS_NFP_WARN("tbs_nfp_pppoe_rx fail!!!!!!!!!!!\n");
        return status;
    }
#endif /* TBS_NFP_VLAN */

    /*ip转发规则匹配和改包*/
	status = tbs_nfp_ip_fw(rx_port, rx_desc, phys_oif);
	if (unlikely(TBS_NFP_OK != status))
	{
        //TBS_NFP_DEBUG("tbs_nfp_ip_fw fail!!!!!!!!!!!\n");
        return status;
	}

#ifdef TBS_NFP_BRIDGE
    /*更新通过fdb查找到本地的包计数*/
    if(local_in_rule)
    {
        local_in_rule->count++;
    }

#endif  /*TBS_NFP_BRIDGE*/

#endif /*TBS_NFP_FIB*/

    return status;
}


/*=========================================================================
 Function:		static inline int tbs_nfp_tx(int port, TBS_NFP_RX_DESC *rx_desc)

 Description:		reset skb->mac_header,更新skb信息，调用物理接口发送
 Data Accessed:     struct list_head g_bridge_hash[TBS_NFP_BRIDGE_HASH_SIZE];
 Data Updated:
 Input:			    int port    接收物理接口ifindex
        			TBS_NFP_RX_DESC *rx_desc    数据包信息
 Output:
 Return:            TBS_NFP_OK: 转发成功；TBS_NFP_TERMINATE: 转发失败，返回协议栈处理
 Others:
=========================================================================*/
static inline int tbs_nfp_tx(int port, TBS_NFP_RX_DESC *rx_desc)
{
    int shift = rx_desc->shift;
    struct sk_buff *skb = NULL;
    skb = rx_desc->skb;

    if(shift > 0)
    {
        skb_pull(skb, *(unsigned int*)&shift);
    }
    else if(shift < 0)
    {
        shift = -shift;
        skb_push(skb, *(unsigned int*)&shift);
    }

    skb_push(skb, ETH_HLEN);
    skb_reset_mac_header(skb);

    //debug dump skb
    //tbs_nfp_dump_packet(skb, skb->mac_header);
    skb->dev->netdev_ops->ndo_start_xmit(skb, skb->dev);
    TBS_NFP_INC(port, tx);

    return TBS_NFP_OK;
}


/*=========================================================================
 Function:		  int tbs_nfp_eth(struct sk_buff *skb)

 Description:		桥、路由快速转发入口
 Data Accessed:
 Data Updated:      skb->data
 Input:			    struct sk_buff *skb     接收包缓冲区
 Output:

 Return:            TBS_NFP_OK: 快速路径转发成功；
                    其他: 转发失败，返回协议栈处理
 Others:
=========================================================================*/
int tbs_nfp_eth(struct sk_buff *skb)
{
    unsigned int    port = 0;
    unsigned int    status = TBS_NFP_TERMINATE;
    TBS_NFP_IF_MAP  *phys_oif = NULL;
    TBS_NFP_RX_DESC rx_desc;

    if(!g_nfp_enable)
        return status;

    //TBS_NFP_DEBUG("*********into tbs_nfp_eth************\n");

    /*只有 PACKET_HOST PACKET_OTHERHOST 类型的包才走加速*/
    if((PACKET_OTHERHOST != skb->pkt_type) && (PACKET_HOST != skb->pkt_type))
    {
    #if 0
        	//printk("pkt_type: ");
            switch(skb->pkt_type)
            {
                case PACKET_HOST:
                     printk("PACKET_HOST \n");
                     break;
                case PACKET_BROADCAST:
                    //printk("PACKET_BROADCAST \n");
                     break;
                case PACKET_MULTICAST:
                     //printk("PACKET_MULTICAST \n");
                     break;
                case PACKET_OTHERHOST:
                     printk("PACKET_OTHERHOST \n");
                     break;
                case PACKET_OUTGOING:
                     printk("PACKET_OUTGOING \n");
                     break;
                case PACKET_LOOPBACK:
                     printk("PACKET_LOOPBACK \n");
                     break;
                case PACKET_FASTROUTE:
                     printk("PACKET_FASTROUTE \n");
                     break;
                default:
                     printk("unkown!! \n");
                    break;
            }
    #endif
        //tbs_nfp_dump_packet(skb,skb->mac_header);
        //TBS_NFP_DEBUG("((PACKET_OTHERHOST | PACKET_HOST) & skb->pkt_type) false!!!!!!!!!!!\n");
        return TBS_NFP_TERMINATE;
    }

    memset(&rx_desc,0,sizeof(rx_desc));

    /*解析skb*/
    //tbs_nfp_dump_packet(skb,skb->mac_header);

    if(skb->dev)
        port = skb->dev->ifindex;
	else{
        TBS_NFP_ERROR("skb->dev == NULL!!!!!!!!!!!\n");

	    return TBS_NFP_TERMINATE;
    }

    TBS_NFP_INC(port, rx);

    /*解析packet*/
    status = tbs_nfp_skb_parser(skb, &rx_desc);
    if (TBS_NFP_OK != status)
    {
        TBS_NFP_INC(port, paser_err);
        TBS_NFP_WARN("tbs_nfp_skb_parser fail!!!!!!!!!!!\n");

        return TBS_NFP_TERMINATE;
    }

#if 0
    if(unlikely(PACKET_MULTICAST == skb->pkt_type))
        TBS_NFP_INC(port, mac_mcast);
#endif

    TBS_NFP_READ_LOCK(&tbs_nfp_lock);

    /*匹配转发规则、更新packet*/
    status = tbs_nfp_rx(port, &rx_desc, &phys_oif);
    if(TBS_NFP_OK != status || NULL == phys_oif)
        goto out;

    skb->dev = (struct net_device *)phys_oif->dev;

    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);

    /*预留qos入队*/

    /*发送packet*/
    status = tbs_nfp_tx(port, &rx_desc);

    return status;

out:
    TBS_NFP_READ_UNLOCK(&tbs_nfp_lock);
    return status;
}


/*=========================================================================
 Function:		static int __init tbs_nfp_init(void)

 Description:		模块初始化函数，入口、规则配置勾子赋值，各子模块hash表初始化
 Data Accessed:     hash缓存表规则

 Data Updated:
 Input:			    无
 Output:			无
 Return:			0:  成功；其他: 失败

 Others:
=========================================================================*/
static int __init tbs_nfp_init(void)
{
    int ret = 0;
    printk(KERN_ERR "TBS NFP is running , version: %s\n", TBS_NFP_VERSION);

    g_jhash_iv = jiffies;

    /*interface 初始化*/
    ret = tbs_nfp_if_init();
    if(ret)
        goto err0;

    //bridge子模块初始化
#ifdef TBS_NFP_BRIDGE
    ret = tbs_nfp_bridge_rule_init();
    if(ret)
        goto err1;

#ifdef TBS_NFP_FIB
    ret = tbs_nfp_arp_init();
    if(ret)
        goto err2;
#endif /*TBS_NFP_FIB*/

#endif /*TBS_NFP_BRIDGE*/

    //fib子模块初始化
#ifdef TBS_NFP_FIB
    ret = tbs_nfp_fib_init();
    if(ret)
        goto err3;
#endif

    //ct子模块初始化
#ifdef TBS_NFP_CT
    ret = tbs_nfp_ct_init();
    if(ret)
        goto err4;
#endif

    /*加速器规则配置接口注册*/
    ret = tbs_nfp_mng_init();
    if(ret)
        goto err5;

    /*proc 调试接口初始化*/
    ret = tbs_nfp_proc_init();
    if(ret)
        goto err6;

    //快速转发钩子函数赋值
    tbs_nfp_rx_hook = tbs_nfp_eth;

    return 0;

err6:
    tbs_nfp_mng_exit();
err5:

#ifdef TBS_NFP_CT
    tbs_nfp_ct_exit();
err4:
#endif

#ifdef TBS_NFP_FIB
    tbs_nfp_fib_exit();
err3:

    tbs_nfp_arp_exit();

#ifdef TBS_NFP_BRIDGE
err2:
#endif /*TBS_NFP_BRIDGE*/
#endif /*TBS_NFP_FIB*/

#ifdef TBS_NFP_BRIDGE
    tbs_nfp_bridge_rule_exit();
err1:
#endif

    tbs_nfp_if_exit();
err0:

    return -1;
}


/*=========================================================================
 Function:		static void __exit tbs_nfp_exit(void)

 Description:		模块退出处理函数，勾子函数赋值，缓存释放
 Data Accessed:     hash缓存表规则

 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
static void __exit tbs_nfp_exit(void)
{
    printk(KERN_ERR "TBS NFP is exit\n");

    /*快速转发钩子函数赋值*/
    tbs_nfp_rx_hook = NULL;

    /*加速器规则配置接口解注册*/
    tbs_nfp_mng_exit();

    /*proc 调试接口销毁*/
    tbs_nfp_proc_exit();

    /*ct子模块退出*/
#ifdef TBS_NFP_CT
    tbs_nfp_ct_exit();
#endif

    /*fib子模块退出*/
    /*arp子模块退出*/
#ifdef TBS_NFP_FIB
    tbs_nfp_fib_exit();
    tbs_nfp_arp_exit();
#endif

    /*bridge子模块退出*/
#ifdef TBS_NFP_BRIDGE
    tbs_nfp_bridge_rule_exit();
#endif

    /*interface 模块退出*/
    tbs_nfp_if_exit();

    return;
}

module_init(tbs_nfp_init);
module_exit(tbs_nfp_exit);
MODULE_LICENSE("GPL");
