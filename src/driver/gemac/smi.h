
#ifndef __SMI_H__
#define __SMI_H__

#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include "rtk_api.h"
#include "rtl8367b_asicdrv_cputag.h"
#include "rtk_types.h"
#include "rtk_error.h"

#define CONFIG_RTL8365_CPU_TAG            1
#ifdef CONFIG_RTL8365_CPU_TAG
	//#define CONFIG_EXTERNAL_CPU_TAG_8   1
	//#define CONFIG_INTERNAL_SPECIAL_TAG 1
	#define RTL8365_ETHTYPE               0x8899
#else
	#define CONFIG_STAG_TO_VTAG           1
	#define CONFIG_STACK_VLAN             1           
	#define VLAN_TPID                     ETH_P_8021Q //0x5AA5//0x88A8
	#define VLAN_BASE_ID                  0x001
#endif

#define GIGA_PHY_CTRL_REG           0
#define GIGA_BASET_CTRL_REG         9


#define MDC_MDIO_DUMMY_ID           0
#define MDC_MDIO_CTRL0_REG          31
#define MDC_MDIO_START_REG          29
#define MDC_MDIO_CTRL1_REG          21
#define MDC_MDIO_ADDRESS_REG        23
#define MDC_MDIO_DATA_WRITE_REG     24
#define MDC_MDIO_DATA_READ_REG      25
#define MDC_MDIO_PREAMBLE_LEN       32
#define MDC_MDIO_START_OP          0xFFFF
#define MDC_MDIO_ADDR_OP           0x000E
#define MDC_MDIO_READ_OP           0x0001
#define MDC_MDIO_WRITE_OP          0x0003

#define ack_timer                    5
#define max_register                0x018A

#ifdef CONFIG_EXTERNAL_CPU_TAG_8
#define CPU_TAG_MODE                0    /* 8 Bytes Mode */
typedef struct rtl8365_cpu_tag {
	unsigned short ethertype;            /* Fixed with 0x8899 for RTL8365MB */
	unsigned char protocol;
	unsigned char reason;
	unsigned char efid              : 1;
	unsigned char enhanced_fid      : 3;
	unsigned char priority_select   : 1;
	unsigned char priority          : 3;
	unsigned char keep              : 1;
	unsigned char vsel              : 1;
	unsigned char dis_learn         : 1;
	unsigned char vidx              : 5;
	unsigned short port;	
}cpu_tag_t;
#else
#define CPU_TAG_MODE                1    /* 4 Bytes Mode */
typedef struct rtl8365_cpu_tag {
	unsigned short ethertype;            /* Fixed with 0x8899 for RTL8365MB */
	unsigned char protocol;
	unsigned char port;
}cpu_tag_t;
#endif


extern void smi_init(void);
#ifdef CONFIG_ETHERNET_DEBUG
extern unsigned long dump_mask;
extern void dump_skb(struct sk_buff * skb, char *s);
#endif
extern int rtl8365_chip_init_kernel(void);
#ifdef CONFIG_RTL8365_CPU_TAG
extern struct sk_buff *rtl8365_cpu_tag_insert(struct sk_buff *skb, struct net_device *dev);
extern int rtl8365_cpu_tag_remove(struct sk_buff *skb);
#else
extern struct sk_buff *rtl8365_vlan_tag_insert(struct sk_buff *skb, struct net_device *dev);
extern int rtl8365_find_dev_by_tag(struct sk_buff *skb);
extern int rtl8365_vlan_tag_remove(struct sk_buff *skb);
#endif
extern void rtl8365_switch_hw_reset_kernel(void);
extern void rtl8365_get_link_state(void);
extern int __init rtl8365_switch_init(void);
extern void __exit rtl8365_switch_exit(void);

#endif /* __SMI_H__ */


