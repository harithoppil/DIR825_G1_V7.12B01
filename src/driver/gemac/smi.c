/*
* Copyright c                  Realtek Semiconductor Corporation, 2006
* All rights reserved.
*
* Program : Control  smi connected RTL8366
* Abstract :
* Author : Yu-Mei Pan (ympan@realtek.com.cn)
*  $Id: smi.c 28599 2012-05-07 01:41:37Z kobe_wu $
*/

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/etherdevice.h>
#include <linux/mii.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>
#include <asm/tc3162/tc3162.h>                  /* for IRQ number */
#include <gpio.h>
#include "gemac.h"
#include "rtk_types.h"
#include "smi.h"
#include "rtk_api_ext.h"
#include "rtl8367b_asicdrv_port.h"
#include "rtl8367b_asicdrv_led.h"
#include "rtl8367b_asicdrv_interrupt.h"
#include "rtk_error.h"


#define LOOKUP_PROC_ENTRY                "tc3162/lookup_table"
#define MIB_COUNTER_ENTRY                "tc3162/switch_mib"
#define SWITCH_MODE_ENTRY                "tc3162/switch_mode"

extern struct net_device *switch_dev[SWITCH_PORT_MAX];
extern struct phy_link_state phystate[SWITCH_PORT_MAX];
/* Timer */
static struct timer_list link_state_timer;
static struct tasklet_struct link_dsr_tasklet;

void rtl8365_switch_hw_reset_kernel(void)
{
	gpio_write(RT63368_GPIO22, GPIO_LEVEL_HIGH);
	gpio_config(RT63368_GPIO22, GPIO_OUT);
	gpio_write(RT63368_GPIO22, GPIO_LEVEL_LOW);
	mdelay(20);
	gpio_write(RT63368_GPIO22, GPIO_LEVEL_HIGH);
	mdelay(1000);
}

static inline int rtl8365_phy_status_get_by_port(unsigned int port)
{
	struct phy_link_state st;
	unsigned int ret_val = 0;

	ret_val = rtk_port_phyStatus_get(port, &st.pLinkStatus, &st.pSpeed, &st.pDuplex);
	if(0 == ret_val) {/* save current state */
		phystate[port] = st;
	}

	return ret_val;

}
void rtl8365_get_link_state(void)
{
	struct phy_link_state st;
	int port;
	int ret_val = 0;
	
	for(port = 0; port < RTK_PHY_ID_MAX; port++) {
		st = phystate[port];
		ret_val = rtl8365_phy_status_get_by_port(port);
		if(0 == ret_val) {
			if(phystate[port].pLinkStatus != st.pLinkStatus) {
				if(LINK_UP != st.pLinkStatus) {
					netif_carrier_on(switch_dev[port]);
					printk("Port %d Link UP!\n", port + 1);
				} else {
					netif_carrier_off(switch_dev[port]);
					printk("Port %d Link DOWN!\n", port + 1);					
				}
			}
		}
	}
}

void rtl8365_interrupt_dsr_link(unsigned long v)
{
	rtl8365_get_link_state();
}

static irqreturn_t rtl8365_isr(int irq, void *dev_id)
{
	unsigned int ims = 0;
	
	if(gpio_get_ins(RT63368_GPIO02)) {
		rtl8367b_getAsicInterruptStatus(&ims);
		rtl8367b_setAsicInterruptStatus(ims);
		tasklet_schedule(&link_dsr_tasklet);
	}
	
	return IRQ_HANDLED;
}

int rtl8365_interrupt_init(void)
{
	unsigned int gpio = RT63368_GPIO02;
	int ret_val = -EOPNOTSUPP;
	gpio_level level;
	unsigned int mask;

	gpio_config(gpio, GPIO_IN);/* Set GPIO as input pin */
	ret_val = rtk_int_polarity_set(INT_POLAR_HIGH);
	if(0 != ret_val) {
		printk("%s: Fail to set interrupt polarity!\n", __func__);
		goto out;
	}
	level = gpio_read(gpio);
	if(GPIO_LEVEL_LOW == level) {
		ret_val = rtk_int_polarity_set(INT_POLAR_LOW);
		level = gpio_read(gpio);
		if(GPIO_LEVEL_LOW == level) {
			ret_val = -EOPNOTSUPP;
		} else {
			gpio_set_edge(gpio, IRQF_TRIGGER_FALLING);/* Trigger at falling edge */
			rtl8367b_getAsicInterruptMask(&mask);
			mask |= (1 << INT_TYPE_LINK_STATUS) | (1 << INT_TYPE_LINK_SPEED) | (1 << INT_TYPE_LOOP_DETECT);
			ret_val = rtl8367b_setAsicInterruptMask(mask);
			if(0 != ret_val) {
				printk("%s: Fail to set interrupt mask!\n", __func__);
			}
		}
	} else {
		ret_val = -EOPNOTSUPP;
	}
out:
	return ret_val;
}

static void link_state_timer_handler(unsigned long data)
{	
	rtl8365_get_link_state();
	mod_timer(&link_state_timer, jiffies + msecs_to_jiffies(500));
}


static int rtl8365_ports_init_kernel(void)
{
	unsigned int port;
	int ret_val = -EINVAL;
	struct rtk_port_mac_ability_s ability;
	struct rtk_port_phy_ability_s pAbility;

	ret_val = rtk_stat_global_reset();
	if(0 != ret_val) {
		printk("%s: Fail to Clear all PHY status!\n", __func__);
		goto out;
	}	
	ret_val = rtk_port_phyEnableAll_set(ENABLED);
	if(0 != ret_val) {
		printk("%s: Fail to Enable all PHY ports!\n", __func__);
		goto out;
	}	
	memset(&pAbility, 0x01, sizeof(struct rtk_port_phy_ability_s));
	#if 0
	pAbility.AsyFC = 0;
	pAbility.FC = 0; /* Disable Flow Control */
	#endif
	ret_val = rtk_eee_init();
	for(port = 0; port < RTK_PHY_ID_MAX; port++) {
		rtk_port_phyAutoNegoAbility_set(port, &pAbility);
		rtk_eee_portEnable_set(port, ENABLED);
	}
	ability.forcemode = MAC_FORCE;
	ability.nway = 0;
	ability.txpause = 1;/* 0 Disable Flow Control */
	ability.rxpause = 1;/* 0 Disable Flow Control */
	ability.link = LINK_UP; /* Force CPU port link up */
	ability.duplex = FULL_DUPLEX;
	ability.speed = SPD_1000M;
	ret_val = rtk_port_macForceLinkExt_set(EXT_PORT_1, EXT_RGMII, &ability);
	ret_val = rtk_port_rgmiiDelayExt_set(EXT_PORT_1, 1, 4);
	
out:	
	return ret_val;
}

#ifdef CONFIG_RTL8365_CPU_TAG
int rtl8365_cpu_tag_init_kernel(void)
{
	int ret_val = 0;
	unsigned int cpu_port = 6;
	unsigned int port;
	unsigned int portmask;
	
	ret_val = rtk_cpu_enable_set(ENABLED);
	if(0 != ret_val) {
		printk("%s: CPU port disabled!\n", __func__);
		goto out;
	}
	ret_val = rtl8367b_setAsicCputagMode(CPU_TAG_MODE); /* 1 for 4 bytes, 0 for 8 bytes */
	if(0 != ret_val) {
		printk("%s: Fail to set tag length!\n", __func__);
		goto out;
	}
	ret_val = rtk_cpu_tagPort_set(cpu_port, CPU_INSERT_TO_ALL);
	if(0 != ret_val) {
		printk("%s: TAG insert disabled!\n", __func__);
		goto out;
	}
	ret_val = rtl8367b_setAsicPortUnknownDaBehavior(UCAST_ACTION_TRAP2CPU);
	if(0 != ret_val) {
		printk("%s: Fail to set unknown DA behavior!\n", __func__);
		goto out;
	}
	ret_val = rtl8367b_setAsicPortUnknownSaBehavior(UCAST_ACTION_TRAP2CPU);
	if(0 != ret_val) {
		printk("%s: Fail to set unknown SA behavior!\n", __func__);
		goto out;
	}
	ret_val = rtl8367b_setAsicPortUnmatchedSaBehavior(UCAST_ACTION_TRAP2CPU);
	if(0 != ret_val) {
		printk("%s: Fail to set unmatched SA behavior!\n", __func__);
		goto out;
	}
	portmask = 0x40;
	ret_val = rtl8367b_setAsicPortUnknownDaFloodingPortmask(portmask);
	ret_val = rtl8367b_setAsicPortUnknownMulticastFloodingPortmask(portmask);
	ret_val = rtl8367b_setAsicPortBcastFloodingPortmask(portmask);
	for(port = 0; port < RTK_PHY_ID_MAX; port++) {
		ret_val = rtl8367b_setAsicUnknownL2MulticastBehavior(port, MCAST_ACTION_TRAP2CPU);
		ret_val = rtl8367b_setAsicUnknownIPv4MulticastBehavior(port, MCAST_ACTION_TRAP2CPU);
		ret_val = rtl8367b_setAsicUnknownIPv6MulticastBehavior(port, MCAST_ACTION_TRAP2CPU);
	}
	
out:
	return ret_val;
}

#ifdef CONFIG_INTERNAL_SPECIAL_TAG
struct sk_buff *rtl8365_cpu_tag_insert(struct sk_buff *skb, struct net_device *dev)
{
	struct rtl8365_cpu_tag *tag;
	unsigned char *p;
	unsigned int *cpu_special_tag;
	struct sk_buff *tagskb = NULL;
	unsigned int rtl8365_tag_len = sizeof(struct rtl8365_cpu_tag);
	unsigned int cpu_special_tag_len = sizeof(unsigned int);
	unsigned int tag_len = rtl8365_tag_len + cpu_special_tag_len;

	skb = skb_unshare(skb, GFP_ATOMIC);
	if(skb_headroom(skb) < tag_len) {
		tagskb = skb_realloc_headroom(skb, tag_len);
		dev_kfree_skb_any(skb);
		if(NULL == tagskb) {
			printk("Failed to realloc headroom!\n");
			goto out;
		}
	} else {
		tagskb = skb;
	}
	p = skb_push(tagskb, tag_len);
	p += 2* VLAN_ETH_ALEN;
    memmove(tagskb->data, tagskb->data + tag_len, 2 * VLAN_ETH_ALEN);/* move MAC to new head of data */
	tagskb->mac_header -= tag_len;
	tag = (struct rtl8365_cpu_tag *)(p + cpu_special_tag_len);
	cpu_special_tag = (unsigned int *)p;
	*cpu_special_tag = 0x80200000; /* Add Special Tag *//* Use port 5 as cpu port */
	memset(tag, 0x00, rtl8365_tag_len);
    tag->ethertype = htons(RTL8365_ETHTYPE);/* Add CPU Tag for this skb */
	tag->protocol = 0x04;
	#ifdef CONFIG_EXTERNAL_CPU_TAG_8
    tag->port = (unsigned short)(1 << dev->base_addr);
	tag->dis_learn = 1;
	#else
	tag->port = (unsigned char)(1 << dev->base_addr);
	#endif	

out:
	return tagskb;
}

int rtl8365_cpu_tag_remove(struct sk_buff *skb)
{
	unsigned short port = 0;
	unsigned char *src;
	unsigned int pull_len = ETH_ALEN * 2;
	unsigned int rtl8365_tag_len = sizeof(struct rtl8365_cpu_tag);
	unsigned int cpu_special_tag_len = sizeof(unsigned int);
	unsigned int tag_len;
	unsigned short *cpu_special_tag;
	struct rtl8365_cpu_tag *tag;
	struct net_device_stats *stats;

	tag_len = rtl8365_tag_len + cpu_special_tag_len;
	cpu_special_tag = (unsigned short *)(skb->data + pull_len);
	tag = (struct rtl8365_cpu_tag *)((unsigned char *)(cpu_special_tag) + cpu_special_tag_len);
	if(RTL8365_ETHTYPE == __constant_ntohs(tag->ethertype)) {
		port = tag->port;
		src = skb->data;
		skb_pull(skb, tag_len);
		memmove(skb->data, src, pull_len);
		if(port <= SWITCH_PORT3) {
			skb->dev = switch_dev[port];
			stats = (struct net_device_stats *)netdev_priv(skb->dev);
			stats->rx_packets++;
			stats->rx_bytes += skb->len;
		} else {
			skb->dev = switch_dev[SWITCH_CPU_PORT];
		}
	} else {
		etdebug("EtherType=0x%04X\n", __constant_ntohs(tag->ethertype));
		skb->dev = switch_dev[SWITCH_CPU_PORT];
	}
	
	return 0;
}
#else
__IMEM struct sk_buff *rtl8365_cpu_tag_insert(struct sk_buff *skb, struct net_device *dev)
{
	struct rtl8365_cpu_tag *tag;
	unsigned char *p;
	struct sk_buff *tagskb = NULL;
	unsigned int rtl8365_tag_len = sizeof(struct rtl8365_cpu_tag);

	if(skb_headroom(skb) < rtl8365_tag_len) {
		tagskb = skb_realloc_headroom(skb, rtl8365_tag_len);
		dev_kfree_skb_any(skb);
		if(NULL == tagskb) {
			printk("Failed to realloc headroom!\n");
			goto out;
		}
	} else {
		tagskb = skb_unshare(skb, GFP_ATOMIC);
		if(NULL == tagskb) {
			goto out;
		}
	}
	p = skb_push(tagskb, rtl8365_tag_len);
	p += (ETH_ALEN << 1);
    memmove(tagskb->data, tagskb->data + rtl8365_tag_len, (ETH_ALEN << 1));/* move MAC to new head of data */
	tag = (struct rtl8365_cpu_tag *)(p);
	memset(tag, 0x00, rtl8365_tag_len);
    tag->ethertype = htons(RTL8365_ETHTYPE);/* Add CPU Tag for this skb */
	tag->protocol = 0x04;
	#ifdef CONFIG_EXTERNAL_CPU_TAG_8
    tag->port = (unsigned short)(1 << dev->base_addr);
	tag->dis_learn = 1;
	#else
	tag->port = (unsigned char)(1 << dev->base_addr);
	#endif	

out:
	return tagskb;
}

__IMEM int rtl8365_cpu_tag_remove(struct sk_buff *skb)
{
	unsigned short port = 0;
	unsigned char *src;
	unsigned int pull_len = (ETH_ALEN << 1);
	unsigned int rtl8365_tag_len = sizeof(struct rtl8365_cpu_tag);
	struct rtl8365_cpu_tag *tag;
	struct net_device_stats *stats;

	tag = (struct rtl8365_cpu_tag *)(skb->data + pull_len);
	if(RTL8365_ETHTYPE == __constant_ntohs(tag->ethertype)) {
		port = tag->port;
		src = skb->data;
		skb_pull(skb, rtl8365_tag_len);
		memmove(skb->data, src, pull_len);
		if(port <= SWITCH_PORT3) {
			skb->dev = switch_dev[port];
			stats = (struct net_device_stats *)netdev_priv(skb->dev);
			stats->rx_packets++;
			stats->rx_bytes += skb->len;
		} else {
			skb->dev = switch_dev[SWITCH_CPU_PORT];
		}
	} else {
		//etdebug("EtherType=0x%04X\n", __constant_ntohs(tag->ethertype));
		skb->dev = switch_dev[SWITCH_CPU_PORT];
	}
	
	return 0;
}
#endif
#else

#ifdef CONFIG_STACK_VLAN
int rtl8365_svlan_tag_init_kernel(void)
{
	struct rtk_svlan_memberCfg_s svlan_cfg;
	unsigned int port;
	struct rtk_portmask_s portmask;
	int ret_val = -EINVAL;

	ret_val = rtk_svlan_init();/* Initial svlan */
	if(0 != ret_val) {
		printk("%s: VLAN features disabled!\n", __func__);
		goto out;
	}
	ret_val = rtk_svlan_servicePort_add(6);/* Uplink port 6 */
	if(0 != ret_val) {
		printk("%s: VLAN service port set failed!\n", __func__);
		goto out;
	}
	ret_val = rtk_svlan_tpidEntry_set(VLAN_TPID);
	if(0 != ret_val) {
		printk("%s: VLAN TPID set failed!\n", __func__);
		goto out;
	}
	for(port = 0; port < RTK_PHY_ID_MAX; port++) {/* Setup SVLAN groups */
		memset(&svlan_cfg, 0x00, sizeof(rtk_svlan_memberCfg_t));
		svlan_cfg.svid = VLAN_BASE_ID + port;
		svlan_cfg.memberport = (1 << 6) | (1 << port);
		svlan_cfg.untagport = 0xF;
		ret_val = rtk_svlan_memberPortEntry_set(svlan_cfg.svid, &svlan_cfg);
		if(0 != ret_val) {
			printk("%s: Fail to add svlan group %d!\n", __func__, port);
			goto out;
		}
	}
	for(port = 0; port < RTK_PHY_ID_MAX; port++) {/* setup default svlan vid per ingress port*/
		ret_val = rtk_svlan_defaultSvlan_set(port, VLAN_BASE_ID + port);
		if(0 != ret_val) {
			printk("%s: Fail to set svlan default for port %d!\n", __func__, port);
			goto out;
		}
	}
	for(port = 0; port < RTK_PHY_ID_MAX; port++) {/* setup port isolation 6&0 6&1 6&2 6&3 */
		portmask.bits[0] = (1 << 6) | (1 << port);
		ret_val = rtk_port_isolation_set(port, portmask);
		if(0 != ret_val) {
			printk("%s: Fail to set isolation for port %d!\n", __func__, port);
			goto out;
		}
	}
	portmask.bits[0] = 0x4F;/* setup port isolation: port6-> port0&1&2&3&6 */
	port = 6;
	ret_val = rtk_port_isolation_set(port, portmask);
	if(0 != ret_val) {
		printk("%s: Fail to set isolation for port %d!\n", __func__, port);
		goto out;
	}
out:
	return ret_val;
}

#else

int rtl8365_pvlan_tag_init_kernel(void)
{
	struct rtk_portmask_s mbrmask;
	struct rtk_portmask_s untagmask;
	unsigned int vid;
	unsigned int fid = RTK_IVL_MODE_FID;
	unsigned int port;
	int ret_val = -EINVAL;
	
	ret_val = rtk_vlan_init();
	if(0 != ret_val) {
		printk("%s: VLAN features disabled!\n", __func__);
		goto out;
	}
	for(port = 0; port < RTK_PHY_ID_MAX; port++) {
		vid = VLAN_BASE_ID + port;
		mbrmask.bits[0] = (1 << 6) | (1 << port);/* 6+0 6+1 6+2 6+3 */
		untagmask.bits[0] = 0xF;		
		ret_val = rtk_vlan_set(vid, mbrmask, untagmask, fid);
		if(0 != ret_val) {
			printk("%s: Failed to create VLAN for port %d!\n", __func__, port);
			break;
		}
		ret_val = rtk_vlan_portPvid_set(port, vid, 0);
		if(0 != ret_val) {
			printk("%s: Failed to set VLAN ID for port %d!\n", __func__, port);
			break;
		}
	}
out:
	return ret_val;
}
#endif

__IMEM struct sk_buff *rtl8365_vlan_tag_insert(struct sk_buff *skb, struct net_device *dev)
{
	struct sk_buff *tagskb = NULL;
	unsigned short *vhdr = NULL;
	unsigned char *p;
	unsigned int cpu_special_tag_len = VLAN_HLEN;

	if(skb_headroom(skb) < cpu_special_tag_len) {
		tagskb = skb_realloc_headroom(skb, cpu_special_tag_len);
		dev_kfree_skb(skb);
		if(NULL == tagskb) {
			printk("Failed to realloc headroom!\n");
			goto out;
		}
	} else {
		tagskb = skb_unshare(skb, GFP_ATOMIC);
		if(NULL == tagskb) {
			goto out;
		}
	}
	p = skb_push(tagskb, cpu_special_tag_len);
	p += 2 * VLAN_ETH_ALEN;	
    memmove(tagskb->data, tagskb->data + cpu_special_tag_len, 2 * VLAN_ETH_ALEN);/* move MAC to new head of data */
	tagskb->dev = switch_dev[SWITCH_CPU_PORT];
	vhdr = (unsigned short *)(p);
    *vhdr = htons(VLAN_TPID);/* Add VLAN Tag for this skb *//* first, the ethernet type */
	*(vhdr + 1) = htons(VLAN_BASE_ID + dev->base_addr);/* now, the TCI */
out:
	return tagskb;
}

__IMEM int rtl8365_vlan_tag_remove(struct sk_buff *skb)
{
	int ret_val = -EINVAL;
	unsigned short port = 0;
	unsigned char *src;
	unsigned int pull_len = ETH_ALEN * 2;
	unsigned int cpu_special_tag_len = VLAN_HLEN;
	unsigned short *vlan_tag;
	struct net_device_stats *stats;

	vlan_tag = (unsigned short *)(skb->data + pull_len);
	if(VLAN_TPID == __constant_ntohs(*vlan_tag)) {
		port = (__constant_ntohs(*(vlan_tag + 1)) - VLAN_BASE_ID);/* Find out which port the packet receive from */
		if(port <= SWITCH_PORT3) {
			skb->dev = switch_dev[port];
			stats = (struct net_device_stats *)netdev_priv(skb->dev);
			stats->rx_packets++;
			stats->rx_bytes += skb->len;
			src = skb->data;
			skb_pull(skb, cpu_special_tag_len);
			memmove(skb->data, src, pull_len);
		} else {
//			etdebug("Unknown port %d, using default!\n", port);
			skb->dev = switch_dev[SWITCH_CPU_PORT];
		}
		ret_val = 0;
	} else {
//		etdebug("VLAN_ID=0x%04X\n", __constant_ntohs(*(vlan_tag + 1)));
		skb->dev = switch_dev[SWITCH_CPU_PORT];
	}
	
	return ret_val;
}
#endif

int rtl8365_mac_learning_off(unsigned int learn_mask)
{
	int ret_val = -EINVAL;
	unsigned int port;
	
	for(port = 0; port <= 6; port++) {/* Set up CPU port */
		if(learn_mask & (1 << port)) {
			ret_val = rtk_l2_limitLearningCnt_set(port, 0);/* Disable MAC learning */
			if(0 != ret_val) {
				printk("%s: Address learn for port %d enabled!\n", __func__, port);
				break;
			}
		}
	}

	return ret_val;
}

int rtl8365_packet_size_set(void)
{
	unsigned int port;
	int ret_val = -EINVAL;
	
	for(port = 0; port < RTK_PHY_ID_MAX; port++) {
		rtk_switch_portMaxPktLen_set(port, 16000);
	}
	ret_val = rtk_switch_maxPktLen_set(MAXPKTLEN_16000B); /* Maximum Packet Size */
	if(0 != ret_val) {
		printk("%s: Fail to set Jumbo frame size!\n", __func__);
		goto out;
	}
	port = 6;
	ret_val = rtk_switch_portMaxPktLen_set(port, 16000);
	
out:	
	return ret_val;
}

static int rtl8365_storm_control_init(void)
{
	unsigned int port;
	rtk_rate_storm_group_t storm_type;
	unsigned int rate = RTL8367B_QOS_RATE_INPUT_MAX;
	rtk_enable_t ifg_include = DISABLED;
	rtk_mode_t mode = MODE0;
	int ret_val = -EINVAL;

	for(storm_type = STORM_GROUP_UNKNOWN_UNICAST; storm_type < STORM_GROUP_END; storm_type++) {
		for(port = SWITCH_PORT0; port < RTK_PHY_ID_MAX; port++) {		
			ret_val = rtk_storm_controlRate_set(port, storm_type, rate, ifg_include, mode);
			if(0 != ret_val) {
				printk("%s: Fail to set storm control rule!\n", __func__);
				goto out;
			}
		}
		port = 6;
		ret_val = rtk_storm_controlRate_set(port, storm_type, rate, ifg_include, mode);
		if(0 != ret_val) {
			printk("%s: Fail to set cpu port storm control rule!\n", __func__);
			goto out;
		}		
	}
out:
	return ret_val;
}

int rtl8365_igmp_init(void)
{
	int ret_val = -1;

	ret_val = rtk_igmp_init();
	if(0 != ret_val) {
		printk("%s: call init failed!\n", __func__);
		goto out;
	}
	ret_val = rtk_trap_igmpCtrlPktAction_set(IGMP_IPV4, IGMP_ACTION_TRAP2CPU);
	if(0 != ret_val) {
		printk("%s: CTRL pkt %d action set failed, ret=%d!\n", __func__, IGMP_IPV4, ret_val);
		goto out;
	}	
	ret_val = rtk_trap_igmpCtrlPktAction_set(IGMP_MLD, IGMP_ACTION_TRAP2CPU);
	if(0 != ret_val) {
		printk("%s: CTRL pkt %d action set failed, ret=%d!\n", __func__, IGMP_MLD, ret_val);
		goto out;
	}		
out:
	return ret_val;
}

int rtl8365_led_init(void)
{
	struct rtk_portmask_s portmask;
	int ret_val = -EINVAL;

	portmask.bits[0] = 0x1F;
	ret_val = rtk_led_enable_set(LED_GROUP_1, portmask);
	if(0 != ret_val) {
		printk("%s: led disabled!\n", __func__);
		goto out;
	}
	ret_val = rtl8367b_setAsicLedOperationMode(LEDOP_PARALLEL);
	if(0 != ret_val) {
		printk("%s: Fail to set led operation mode!\n", __func__);
		goto out;
	}
	ret_val = rtl8367b_setAsicLedGroupMode(RTL8367B_LED_MODE_0);
	if(0 != ret_val) {
		printk("%s: Fail to set led group mode!\n", __func__);
		goto out;
	}
	rtl8367b_setAsicLedBlinkRate(LED_BLINKRATE_32MS);
	if(0 != ret_val) {
		printk("%s: Using default blink rate!\n", __func__);
	}
	ret_val = rtk_led_groupConfig_set(LED_GROUP_1, LEDCONF_LINK_ACT);
	if(0 != ret_val) {
		printk("%s: Fail to set link act mode!\n", __func__);
	}
out:
	return ret_val;
}

int rtl8365_hw_init(void)
{
	int ret_val = -ENOMEM;

	#if 0
	rtl8365_switch_hw_reset_kernel();
	ret_val = rtl8365_chip_init_kernel();
	if(0 != ret_val) {
		printk("%s: Fail to init chip!\n", __func__);
		goto out;
	}
	#endif
	ret_val = rtk_l2_init();
	if(0 != ret_val) {
		printk("%s: Layer 2 features disabled!\n", __func__);
		goto out;
	}
	#ifdef CONFIG_RTL8365_CPU_TAG
	ret_val = rtl8365_cpu_tag_init_kernel();
	#else
	#ifdef CONFIG_STACK_VLAN
	ret_val = rtk_cpu_enable_set(DISABLED);
	ret_val = rtl8365_svlan_tag_init_kernel();
	#else
	ret_val = rtl8365_pvlan_tag_init_kernel();
	#endif
	#endif
	if(0 != ret_val) {
		printk("%s: Fail to set switch tag!\n", __func__);
		goto out;
	}
	#ifdef CONFIG_RTL8365_CPU_TAG
	ret_val = rtl8365_mac_learning_off(0x40);/* Disable MAC Learning */ /* 0x40 for CPU port */
	#else
	ret_val = rtl8365_mac_learning_off(0x4F);/* Disable MAC Learning */
	#endif
	if(0 != ret_val) {
		printk("%s: Fail to set MAC Learning!\n", __func__);
	}
	ret_val = rtl8365_packet_size_set();
	if(0 != ret_val) {
		printk("%s: Fail to set packet size!\n", __func__);
	}
	ret_val = rtl8365_igmp_init();
	if(0 != ret_val) {
		printk("%s: Fail to set igmp!\n", __func__);
	}	
#if 1	
	ret_val = rtl8365_ports_init_kernel();
	if(0 != ret_val) {
		printk("%s: Fail to set port!\n", __func__);
		goto out;
	}
	ret_val = rtl8365_led_init();
	if(0 != ret_val) {
		printk("%s: Fail to set PHY led!\n", __func__);
	}
	rtl8365_storm_control_init();
#else		
	ret_val = rtk_stat_global_reset();
	if(0 != ret_val) {
		printk("%s: Fail to Clear all PHY status!\n", __func__);
		goto out;
	}
#endif	
	
out:	
	return ret_val;
}


void PHY_power_ops(unsigned int port, int optcode)
{
	unsigned int value;

	if(port < RTK_PHY_ID_MAX) {
		rtl8367b_getAsicPHYReg(port, GIGA_PHY_CTRL_REG, &value);
		if(0 == optcode) {
			value |= BMCR_PDOWN; /* Power down the PHY */
		} else {
			value &= ~BMCR_PDOWN;
			value |= BMCR_ANRESTART; /* Restart auto-negotiation process */
		}
		etdebug("Turn %s PHY power on port %d\n", (optcode)? "ON" : "DOWN", port);
		rtl8367b_setAsicPHYReg(port, GIGA_PHY_CTRL_REG, value);
		if(0 != optcode) { /* Waiting for the link to be ready */
			mdelay(100);
		}
	}
}

void PHY_led_ops(unsigned int port, unsigned int optcode)
{
	unsigned int group = LED_GROUP_1;
	unsigned int mode;
	unsigned int value;

	switch(optcode) {
		case RTL8367B_LED_MODE_0:
			mode = LEDFORCEMODE_OFF;
			break;
		
		case RTL8367B_LED_MODE_1:
			mode = LEDFORCEMODE_ON;
			break;
		
		default:
			mode = LEDFORCEMODE_NORMAL;
			break;
	}
	if(port < RTK_PHY_ID_MAX) {
		value = LEDFORCEMODE_END;
		while(value != mode) {
			rtl8367b_setAsicForceLed(port, group, mode);
			rtl8367b_getAsicForceLed(port, group, &value);
			if(value != mode) {
				printk("%s: set led for port %d\n", __func__, port);
			}
		}
	}
}

int rtl8365_port_mode_control(unsigned int port, unsigned int mode)
{
	unsigned int value;
	const unsigned int modemask = 0xE000;/* Bit15~Bit13 */
	int ret_val = -EINVAL;

	if((port < RTK_PHY_ID_MAX) && (mode < 5)) {
		rtl8367b_getAsicPHYReg(port, GIGA_BASET_CTRL_REG, &value);
		value &= ~(modemask);
		value |= (mode << 13);
		rtl8367b_setAsicPHYReg(port, GIGA_BASET_CTRL_REG, value);
		ret_val = 0;
	} else if(9 == mode) {/* Reset switch */
		del_timer_sync(&link_state_timer);
		disable_irq(GPIO_INT);
		rtl8365_switch_hw_reset_kernel();
		ret_val = rtl8365_chip_init_kernel();
		if(0 != ret_val) {
			printk("%s: Fail to init chip!\n", __func__);
		}		
		ret_val = rtl8365_hw_init();
		if(0 == ret_val) {
			ret_val = rtl8365_interrupt_init();
			if(0 != ret_val) {
				ret_val = mod_timer(&link_state_timer, jiffies + msecs_to_jiffies(500));
			}
		}
		enable_irq(GPIO_INT);
		for(port = 0; port < RTK_PHY_ID_MAX; port++) {
			PHY_power_ops(port, ENABLED);
		}
	} else if(8 == mode) {
		ret_val = del_timer_sync(&link_state_timer);
		if(0 != port) {/* Restart link state timer */
			ret_val = mod_timer(&link_state_timer, jiffies + msecs_to_jiffies(500));
		}
	}
	return ret_val;
}

int rtl8365_port_mode_get(unsigned int port, unsigned int *mode)
{
	unsigned int value;
	const unsigned int modemask = 0xE000;/* Bit15~Bit13 */
	int ret_val = -EINVAL;

	if(port < RTK_PHY_ID_MAX) {
		rtl8367b_getAsicPHYReg(port, GIGA_BASET_CTRL_REG, &value);
		value &= modemask;
		*mode = (value >> 13);
		ret_val = 0;
	}
	return ret_val;
}

static int rtl8365_open(struct net_device *dev)/* Starting up the ethernet device */
{
	int ret_val = 0;

	netif_carrier_off(dev);
	PHY_power_ops(dev->base_addr, ENABLED);
	PHY_led_ops(dev->base_addr, RTL8367B_LED_MODE_3);
	netif_start_queue(dev);

  	return ret_val;
}


static int rtl8365_close(struct net_device *dev)/* Stopping the ethernet device */
{
  	netif_stop_queue(dev);
	PHY_power_ops(dev->base_addr, DISABLED);
  	return 0;
}

__IMEM netdev_tx_t rtl8365_tx(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats;
	struct net_device *rootdev = switch_dev[SWITCH_CPU_PORT];
	
	netdev_tx_t ret_val = NETDEV_TX_OK;

	stats = (struct net_device_stats *)netdev_priv(dev);
	
    if(unlikely(netif_queue_stopped(rootdev))) {
		dev_kfree_skb_any(skb);
		stats->tx_dropped++;
		printk("%s: Root device is stopped!\n", __func__);
		goto out;
   	}
	ret_val = tc3262_gmac_tx(skb, dev);

out:	
	return ret_val;
}


static int rtl8365_set_macaddr(struct net_device *dev, void *p)/* Setting customized mac address */
{
	struct sockaddr *addr = p;
	int ret_val = -EIO;
	#ifdef CONFIG_RTL8365_CPU_TAG
	struct rtk_mac_s *pMac;
	struct rtk_l2_ucastAddr_s pL2_data;
	#endif

	/* Check if given address is valid ethernet MAC address */
  	if(!is_valid_ether_addr(addr->sa_data)) {
    	goto out;
	}
	#ifdef CONFIG_RTL8365_CPU_TAG
	if(is_valid_ether_addr(switch_dev[SWITCH_CPU_PORT]->dev_addr)) {
		pMac = (struct rtk_mac_s *)(switch_dev[SWITCH_CPU_PORT]->dev_addr);
		memset(&pL2_data, 0x00, sizeof(struct rtk_l2_ucastAddr_s));
		memcpy(pL2_data.mac.octet, pMac->octet, ETHER_ADDR_LEN);
		pL2_data.port = 6;
		pL2_data.is_static = 1;
		ret_val = rtk_l2_addr_del(pMac, &pL2_data);
	}
	#endif
  	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);/* Save the customize mac address */
	tc3262_gmac_set_macaddr(switch_dev[SWITCH_CPU_PORT], addr);
	#ifdef CONFIG_RTL8365_CPU_TAG
	pMac = (struct rtk_mac_s *)(dev->dev_addr);	
	memset(&pL2_data, 0x00, sizeof(struct rtk_l2_ucastAddr_s));
	memcpy(pL2_data.mac.octet, pMac->octet, ETHER_ADDR_LEN);
	pL2_data.port = 6;
	pL2_data.is_static = 1;
	ret_val = rtk_l2_addr_add(pMac, &pL2_data);
	if(0 != ret_val) {
		printk("Fail to add new MAC entry!\n");
		goto out;
	}
	#endif
out:
	return ret_val;
}

static struct net_device_stats *rtl8365_get_stats(struct net_device *dev)
{
	return (struct net_device_stats *)netdev_priv(dev);
}

static const struct net_device_ops switch_netdev_ops = {
	.ndo_open				= rtl8365_open,
	.ndo_stop 				= rtl8365_close,
	.ndo_start_xmit			= rtl8365_tx,
	.ndo_get_stats			= rtl8365_get_stats,
	.ndo_set_mac_address 	= rtl8365_set_macaddr,
};

static int lookup_table_entry_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	struct rtk_l2_addr_table_s l2_entry;
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;	
	int i;
	int ret_val;

	for (i = 1; i <= 2112; i++) {
		memset(&l2_entry, 0x00, sizeof(struct rtk_l2_addr_table_s));
		l2_entry.index = i;
		ret_val = rtk_l2_entry_get(&l2_entry);
		if(RT_ERR_OK == ret_val) {
			if(0 != l2_entry.is_ipmul) {
				index += sprintf(page+index, "\nIndex\tSourceIP\tDestinationIP\tMemberPort\tState\n");
				CHK_BUF();				
				index += sprintf(page+index, "%04d", l2_entry.index);
				CHK_BUF();
				index += sprintf(page+index, "\t%08x", l2_entry.sip);
				CHK_BUF();
				index += sprintf(page+index, "\t%08x", l2_entry.dip);
				CHK_BUF();
				index += sprintf(page+index, "\t%08x", l2_entry.portmask);
				CHK_BUF();
				index += sprintf(page+index, "\t%s\n", l2_entry.is_static ? "Static" : "Auto");
				CHK_BUF();
			} else if((l2_entry.mac.octet[0] & 0x01) || (0 != l2_entry.age) || (1 == l2_entry.is_static)){
				index += sprintf(page+index, "\nIndex\tSourceMAC\t\tPortmask\tFID\tAUTH\tBLOCK\tState\tAGE\n");
				CHK_BUF();			
				index += sprintf(page+index, "%4d", l2_entry.index);
				CHK_BUF();
				index += sprintf(page+index, "\t%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X", 
					l2_entry.mac.octet[0], l2_entry.mac.octet[1], l2_entry.mac.octet[2], 
					l2_entry.mac.octet[3], l2_entry.mac.octet[4], l2_entry.mac.octet[5]);
				CHK_BUF();
				index += sprintf(page+index, "\t%08x", l2_entry.portmask);
				CHK_BUF();
				index += sprintf(page+index, "\t%d", l2_entry.fid);
				CHK_BUF();
				index += sprintf(page+index, "\t%s", l2_entry.auth ? "Auth" : "*");
				CHK_BUF();
				index += sprintf(page+index, "\t%s", l2_entry.sa_block ? "Block" : "*");
				CHK_BUF();
				index += sprintf(page+index, "\t%s", l2_entry.is_static ? "Static" : "Auto");
				CHK_BUF();
				index += sprintf(page+index, "\t%d\n", l2_entry.age);
				CHK_BUF();
			}
		}
	}
	*eof = 1;
done:
	*start = page + (off - begin);
	index -= (off - begin);
	if (index<0) 
		index = 0;
	if (index>count) 
		index = count;
	return index;
}

static int mib_entry_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	struct rtk_stat_port_cntr_s cntrs;
	int index = 0;
	off_t pos = 0;
	off_t begin = 0;	
	int port;
	int ret_val;

	for (port = 0; port < 7; port++) {
		if((4 == port) || (5 == port)) {
			continue;
		}
		ret_val = rtk_stat_port_getAll(port, &cntrs);
		if(RT_ERR_OK == ret_val) {
			index += sprintf(page+index, "\r\nMIB counter for port %d\n", port);
			CHK_BUF();
			index += sprintf(page+index, "ifInOctets:                [%16llx]\t", cntrs.ifInOctets);
			CHK_BUF();
			index += sprintf(page+index, "dot3StatsFCSErrors:                [%08x]\n", cntrs.dot3StatsFCSErrors);
			CHK_BUF();
			index += sprintf(page+index, "dot3StatsSymbolErrors:             [%08x]\t", cntrs.dot3StatsSymbolErrors);
			CHK_BUF();
			index += sprintf(page+index, "dot3InPauseFrames:                 [%08x]\n", cntrs.dot3InPauseFrames);
			CHK_BUF();
			index += sprintf(page+index, "dot3ControlInUnknownOpcodes:       [%08x]\t", cntrs.dot3ControlInUnknownOpcodes);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsFragments:               [%08x]\n", cntrs.etherStatsFragments);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsJabbers:                 [%08x]\t", cntrs.etherStatsJabbers);
			CHK_BUF();
			index += sprintf(page+index, "ifInUcastPkts:                     [%08x]\n", cntrs.ifInUcastPkts);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsDropEvents:              [%08x]\t", cntrs.etherStatsDropEvents);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsOctets:          [%16llx]\n", cntrs.etherStatsOctets);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsUndersizePkts:           [%08x]\t", cntrs.etherStatsUndersizePkts);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsOversizePkts:            [%08x]\n", cntrs.etherStatsOversizePkts);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsPkts64Octets:            [%08x]\t", cntrs.etherStatsPkts64Octets);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsPkts65to127Octets:       [%08x]\n", cntrs.etherStatsPkts65to127Octets);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsPkts128to255Octets:      [%08x]\t", cntrs.etherStatsPkts128to255Octets);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsPkts256to511Octets:      [%08x]\n", cntrs.etherStatsPkts256to511Octets);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsPkts512to1023Octets:     [%08x]\t", cntrs.etherStatsPkts512to1023Octets);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsPkts1024toMaxOctets:     [%08x]\n", cntrs.etherStatsPkts1024toMaxOctets);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsMcastPkts:               [%08x]\t", cntrs.etherStatsMcastPkts);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsBcastPkts:               [%08x]\n", cntrs.etherStatsBcastPkts);
			CHK_BUF();
			index += sprintf(page+index, "ifOutOctets:               [%16llx]\t", cntrs.ifOutOctets);
			CHK_BUF();
			index += sprintf(page+index, "dot3StatsSingleCollisionFrames:    [%08x]\n", cntrs.dot3StatsSingleCollisionFrames);
			CHK_BUF();
			index += sprintf(page+index, "dot3StatsMultipleCollisionFrames:  [%08x]\t", cntrs.dot3StatsMultipleCollisionFrames);
			CHK_BUF();
			index += sprintf(page+index, "dot3StatsDeferredTransmissions:    [%08x]\n", cntrs.dot3StatsDeferredTransmissions);
			CHK_BUF();
			index += sprintf(page+index, "dot3StatsLateCollisions:           [%08x]\t", cntrs.dot3StatsLateCollisions);
			CHK_BUF();
			index += sprintf(page+index, "etherStatsCollisions:              [%08x]\n", cntrs.etherStatsCollisions);
			CHK_BUF();
			index += sprintf(page+index, "dot3StatsExcessiveCollisions:      [%08x]\t", cntrs.dot3StatsExcessiveCollisions);
			CHK_BUF();
			index += sprintf(page+index, "dot3OutPauseFrames:                [%08x]\n", cntrs.dot3OutPauseFrames);
			CHK_BUF();
			index += sprintf(page+index, "dot1dBasePortDelayExceededDiscards:[%08x]\t", cntrs.dot1dBasePortDelayExceededDiscards);
			CHK_BUF();
			index += sprintf(page+index, "dot1dTpPortInDiscards:             [%08x]\n", cntrs.dot1dTpPortInDiscards);
			CHK_BUF();
			index += sprintf(page+index, "ifOutUcastPkts:                    [%08x]\t", cntrs.ifOutUcastPkts);
			CHK_BUF();
			index += sprintf(page+index, "ifOutMulticastPkts:                [%08x]\n", cntrs.ifOutMulticastPkts);
			CHK_BUF();
			index += sprintf(page+index, "ifOutBrocastPkts:                  [%08x]\t", cntrs.ifOutBrocastPkts);
			CHK_BUF();
			index += sprintf(page+index, "outOampduPkts:                     [%08x]\n", cntrs.outOampduPkts);
			CHK_BUF();
			index += sprintf(page+index, "inOampduPkts:                      [%08x]\t", cntrs.inOampduPkts);
			CHK_BUF();
			index += sprintf(page+index, "pktgenPkts:                        [%08x]\n", cntrs.pktgenPkts);
			CHK_BUF();
			index += sprintf(page+index, "inMldChecksumError:                [%08x]\t", cntrs.inMldChecksumError);
			CHK_BUF();
			index += sprintf(page+index, "inIgmpChecksumError:               [%08x]\n", cntrs.inIgmpChecksumError);
			CHK_BUF();
			index += sprintf(page+index, "inMldSpecificQuery:                [%08x]\t", cntrs.inMldSpecificQuery);
			CHK_BUF();
			index += sprintf(page+index, "inMldGeneralQuery:                 [%08x]\n", cntrs.inMldGeneralQuery);
			CHK_BUF();
			index += sprintf(page+index, "inIgmpSpecificQuery:               [%08x]\t", cntrs.inIgmpSpecificQuery);
			CHK_BUF();
			index += sprintf(page+index, "inIgmpGeneralQuery:                [%08x]\n", cntrs.inIgmpGeneralQuery);
			CHK_BUF();
			index += sprintf(page+index, "inMldLeaves:                       [%08x]\t", cntrs.inMldLeaves);
			CHK_BUF();
			index += sprintf(page+index, "inIgmpLeaves:                      [%08x]\n", cntrs.inIgmpLeaves);
			CHK_BUF();
			index += sprintf(page+index, "inIgmpJoinsSuccess:                [%08x]\t", cntrs.inIgmpJoinsSuccess);
			CHK_BUF();
			index += sprintf(page+index, "inIgmpJoinsFail:                   [%08x]\n", cntrs.inIgmpJoinsFail);
			CHK_BUF();
			index += sprintf(page+index, "inMldJoinsSuccess:                 [%08x]\t", cntrs.inMldJoinsSuccess);
			CHK_BUF();
			index += sprintf(page+index, "inMldJoinsFail:                    [%08x]\n", cntrs.inMldJoinsFail);
			CHK_BUF();
			index += sprintf(page+index, "inReportSuppressionDrop:           [%08x]\t", cntrs.inReportSuppressionDrop);
			CHK_BUF();
			index += sprintf(page+index, "inLeaveSuppressionDrop:            [%08x]\n", cntrs.inLeaveSuppressionDrop);
			CHK_BUF();
			index += sprintf(page+index, "outIgmpReports:                    [%08x]\t", cntrs.outIgmpReports);
			CHK_BUF();
			index += sprintf(page+index, "outIgmpLeaves:                     [%08x]\n", cntrs.outIgmpLeaves);
			CHK_BUF();
			index += sprintf(page+index, "outIgmpGeneralQuery:               [%08x]\t", cntrs.outIgmpGeneralQuery);
			CHK_BUF();
			index += sprintf(page+index, "outIgmpSpecificQuery:              [%08x]\n", cntrs.outIgmpSpecificQuery);
			CHK_BUF();
			index += sprintf(page+index, "outMldReports:                     [%08x]\t", cntrs.outMldReports);
			CHK_BUF();
			index += sprintf(page+index, "outMldLeaves:                      [%08x]\n", cntrs.outMldLeaves);
			CHK_BUF();
			index += sprintf(page+index, "outMldGeneralQuery:                [%08x]\t", cntrs.outMldGeneralQuery);
			CHK_BUF();
			index += sprintf(page+index, "outMldSpecificQuery:               [%08x]\n", cntrs.outMldSpecificQuery);
			CHK_BUF();
			index += sprintf(page+index, "inKnownMulticastPkts:              [%08x]\t", cntrs.inKnownMulticastPkts);
			CHK_BUF();
			index += sprintf(page+index, "ifInMulticastPkts:                 [%08x]\n", cntrs.ifInMulticastPkts);
			CHK_BUF();
			index += sprintf(page+index, "ifInBroadcastPkts:                 [%08x]\n", cntrs.ifInBroadcastPkts);
			CHK_BUF();
		}
	}
	*eof = 1;
done:
	*start = page + (off - begin);
	index -= (off - begin);
	if (index<0) 
		index = 0;
	if (index>count) 
		index = count;
	return index;
}

static int mib_entry_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char temp[32];
	int value = 0;
	unsigned int port;
	int ret_val;
	
	if (count > sizeof(temp) - 1)
		goto out;

	if (copy_from_user(temp, buffer, count))
		goto out;

	temp[count] = '\0';
	sscanf(temp, "%d", &value);
	if(1 == value) {
		for(port = 0; port < 7; port++) {
			if((4 == port) || (5 == port)) {
				continue;
			}
			ret_val = rtk_stat_port_reset(port);
			if(0 != ret_val) {
				printk("Failed to reset mib counter for port %d\n", port);
				goto out;
			}
		}
	}
	
out:
	return count;
}


/**************************************************
    SWITCH mode operation interface
***************************************************/
static int rtl8365_mode_proc_write(struct file *filp, const char __user *buf, unsigned long len, void *data)
{
    int ret_val;
    char str_buf[256];
    unsigned int port = 0;
    unsigned int mode = 0;
    int i;

    if(len > 255) {
        goto err;
   	}
    copy_from_user(str_buf, buf, len);
    str_buf[len] = '\0';
    for(i = 0; i < len; i++) {/* ¼ì²éÊäÈë¸ñÊ½ */
        if(str_buf[i] < '0' || str_buf[i] > '9') {
            if(str_buf[i] != 0x20 && str_buf[i] != 0x0a) {
                goto err;
           	}
       	}
   	}
	etdebug("len %d:%s\n",(int)len,str_buf);
	ret_val = sscanf(str_buf, "%d %d", &port, &mode);
    if(ret_val < 0) {
		printk("Failed to get input!\n");
    	goto err;
   	}
    etdebug("Set:port %d, mode %d\n", port, mode);
    ret_val = rtl8365_port_mode_control(port, mode);
    if(ret_val < 0) {
		printk("Set switch port %d to mode %d failed\n", port, mode);
		goto err;
   	} else {
		goto out;
   	}
err:
	printk("Usage: echo port_num(0~3) mode(0~4) > /proc/%s\n", SWITCH_MODE_ENTRY);
out:
    return len;
}

static int rtl8365_mode_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	unsigned int port;
	int len = 0;
	unsigned int mode = 0;
	int ret_val = -EINVAL;

	len += sprintf(page + len, "Switch mode state:\n");
	for(port = 0; port < 4; port++) {
		ret_val = rtl8365_port_mode_get(port, &mode);
		if(ret_val < 0) {
			len += sprintf(page + len, "fail to get mode on port %d", port);
		} else {
			len += sprintf(page + len, "port %d is running on mode %d\n", port, mode);
		}
	}
	if(len <= off + count) {
		*eof = 1;
	}
	*start = page + off;
	len -= off;
	if(len > count) {
		len = count;
	}
	if(len < 0) {
		len = 0;
	}

	return len;
}

/*=========================================================================
 Function Description:		¼Ä´æÆ÷¶ÁÐ´²Ù×÷
 Data Accessed:
 Data Updated:
 Input:				reg -r reg_addr length
                    reg -w reg_addr data
 Output:			register data
 Return:			always return 0
 Others:
=========================================================================*/
ssize_t switch_register_operation( struct file *filp, const char __user *buff, unsigned long len, void *data)
{
	char cmd[64] = {'\0'};
	char *addrp = &cmd[0];
	char *valp;
	unsigned int addr = 0;
	unsigned int value = 0;
	unsigned int length = 0;
	int i;

	if(len > sizeof(cmd)) {
		goto err1;
	}
	if(0 != copy_from_user(cmd, buff, len)) {
		goto err1;
	}
	if(0 == memcmp(cmd, "reg -w", 6)) {
		addrp += 7;
		addr = simple_strtoul(addrp, &valp, 16);
		if((addr > 0xFFFF) || (valp == addrp)) {
			goto err1;
		}
		value = simple_strtoul(valp + 1, NULL, 16);
		if(value > 0xFFFF) {
			goto err1;
		}
		rtl8367b_setAsicReg(addr, value);
		value = 0;
		rtl8367b_getAsicReg(addr, &value);
		printk("Addr: %#x = 0x%04X\n", addr, value);
	} else if(0 == memcmp(cmd, "reg -r", 6)) {
		addrp += 7;
		addr = simple_strtoul(addrp, &valp, 16);
		if((addr > 0xFFFF) || (valp == addrp)) {
			goto err1;
		}
		length= simple_strtoul(valp + 1, NULL, 10);
		if((length > 0xFFFF) || (length < 1)) {
			goto err1;
		}
		for(i = 0; i < length; i++) {
			if(0 == (i % 16)) {
				printk("\n@%#x: \n", addr);
			}
			rtl8367b_getAsicReg(addr, &value);
			printk(" %04X", value);
			addr += 1;
		}
		printk("\n");		
	} else {
		goto err1;
	}
	goto err0;

err1:	
	printk("Input Error!\n");
err0:
	return len;
}


int __init rtl8365_switch_init(void)
{
	int i, port;
	struct net_device *dev = NULL;
	struct proc_dir_entry *entry;
	int ret_val = -ENOMEM;

	smi_init();
	tasklet_init(&link_dsr_tasklet, rtl8365_interrupt_dsr_link, (unsigned long)switch_dev[SWITCH_CPU_PORT]);
  	setup_timer(&link_state_timer, link_state_timer_handler, (unsigned long)switch_dev[SWITCH_CPU_PORT]);
	ret_val = rtl8365_hw_init();
	if(0 != ret_val) {
		goto out;
	}
	for(port = 0; port < RTK_PHY_ID_MAX; port++) {
        dev = alloc_etherdev(sizeof(struct net_device_stats));
		if(NULL == dev) {
            printk("%s: No memory!\n", __func__);
            for(i = port; i > 0; i--) {
				unregister_netdev(switch_dev[i]);
				free_netdev(switch_dev[i]);
			}
            break;
        }
        memset(netdev_priv(dev), 0x00, sizeof(struct net_device_stats));
        sprintf(dev->name, "eth0.%d", port + 1);
        dev->netdev_ops = &switch_netdev_ops;
        dev->base_addr = port;
		dev->irq = GPIO_INT;
        memcpy(dev->dev_addr, switch_dev[SWITCH_CPU_PORT]->dev_addr, ETH_ALEN);
        switch_dev[port] = dev;
        ret_val = register_netdev(dev);
		if(0 != ret_val) {
			printk("Fail to register device %s\n", dev->name);
            free_netdev(dev);
            for(i = port; i > 0; i--) {
				unregister_netdev(switch_dev[i]);
				free_netdev(switch_dev[i]);
			}
            break;
        }

        /*
         * TBS_TAG:add by pengyao 20130311
         * Desc: ether net interface for tbs_nfp
         */
        dev->priv_flags |= IFF_ETH;
        /*
         * TBS_TAG:END
         */
   	}
	create_proc_read_entry(LOOKUP_PROC_ENTRY, 0, NULL, lookup_table_entry_read, NULL);
	entry = create_proc_entry(MIB_COUNTER_ENTRY, 0644, NULL);
	if(NULL != entry) {
		entry->read_proc = mib_entry_read;
		entry->write_proc = mib_entry_write;
	}
	entry = create_proc_entry(SWITCH_MODE_ENTRY, 0644, NULL);
	if(NULL != entry) {
		entry->read_proc = rtl8365_mode_proc_read;
		entry->write_proc = rtl8365_mode_proc_write;
	}
	entry = create_proc_entry(SWITCH_REG_ENTRY, 0644, NULL);
	if(NULL != entry) {
		entry->write_proc = switch_register_operation;
	}
	ret_val = rtl8365_interrupt_init();
	printk("Switch interrupt ");
	if(0 != ret_val) {/* Schedule timer for monitoring link status */
		printk("NOT support!\n");
		ret_val = mod_timer(&link_state_timer, jiffies + msecs_to_jiffies(500));
	} else {
		ret_val = request_irq(GPIO_INT, rtl8365_isr, IRQF_SHARED, "RTL8365", switch_dev[SWITCH_CPU_PORT]);
		if(0 != ret_val) {
			printk("%s: Fail to register irq %d, ret=%d!\n", __func__, GPIO_INT, ret_val);
			goto out;
		}
		printk("Enabled, IRQ=%d\n", GPIO_INT);
	}	

out:
	return ret_val;
}

void __exit rtl8365_switch_exit(void)
{
	int port;

	synchronize_net();
  	/* Kill timer */
  	del_timer_sync(&link_state_timer);
	tasklet_kill(&link_dsr_tasklet);	
	free_irq(GPIO_INT, NULL);
	remove_proc_entry(LOOKUP_PROC_ENTRY, 0);
	remove_proc_entry(MIB_COUNTER_ENTRY, 0);
	remove_proc_entry(SWITCH_MODE_ENTRY, 0);
	remove_proc_entry(SWITCH_REG_ENTRY, 0);
	for(port = 0; port < RTK_PHY_ID_MAX; port++) {
		if(NULL != switch_dev[port]) {
            unregister_netdev(switch_dev[port]);
            free_netdev(switch_dev[port]);
            switch_dev[port] = NULL;
        }
    }
}

EXPORT_SYMBOL_GPL(PHY_led_ops);

