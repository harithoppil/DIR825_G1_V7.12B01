/*=========================================================================
 Copyright (c), 1991-2007, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : nfp_bridge.h
 文件描述 : tbs nfp适配


 修订记录 :
          1 创建 : pengyao
            日期 : 2011-06-16
            描述 :
=========================================================================*/

#ifndef __BRIDGE_ADAPTER_H__
#define __BRIDGE_ADAPTER_H__



/* Some length */
#define IPV6_ADDRESS_LENGTH	16

#define BRIDGE_ENTRY_TIMEOUT	( 300 * HZ )
#define FDB_RULE_TIMEOUT	( 300 * HZ )


#define BRIDGE_ENTRYNUM_MAX 128
#define FDB_ENTRYNUM_MAX    128


/*
* Structures
*
*/
typedef struct tL2Bridge_entry {
    unsigned char da[ETH_ALEN];
    unsigned char sa[ETH_ALEN];
    unsigned short ethertype;
    unsigned short input_interface;
    unsigned short output_interface;

    unsigned char input_name[IFNAMSIZ];
    unsigned char output_name[IFNAMSIZ];

    int input_ifindex;
    int output_ifindex;
} L2Bridge_entry, *PL2Bridge_entry;
/*
 * L2 Bridge Acp Table structure
 */
typedef struct tL2Bridge_entry_tab
{
    struct tL2Bridge_entry_tab *next;
	struct timer_list timeout;          /* 定时器 */
    L2Bridge_entry direction[2];
    unsigned long ageing_timer;
}L2Bridge_entry_tab,*PL2Bridge_entry_tab;

struct fdb_entry_tab{
    struct list_head list_by_mac;
	struct timer_list timeout;          /* 定时器 */
    unsigned char da[ETH_ALEN];
    unsigned char sa[ETH_ALEN];
    unsigned long ageing_timer;
    int       iif;
    int       oif;
    //atomic_t refcnt;                    /*引用计数*/
};


typedef enum  {
    LEFT_DIRECTION = 0,
    RIGHT_DIRECTION,
    BI_DIRECTION,
}L2Bridge_entry_direction;


/*=========================================================================
struct add_entry_t: 此结构体用于向工作队列传递参数
=========================================================================*/

struct add_entry_t {
    unsigned short input_interface;
    unsigned short output_interface;
    unsigned short input_svlan;
    unsigned short input_cvlan;
    unsigned short output_svlan;
    unsigned short output_cvlan;
    unsigned char da[ETH_ALEN];
    unsigned char sa[ETH_ALEN];
    unsigned char input_name[IFNAMSIZ];
    unsigned char output_name[IFNAMSIZ];
    int input_ifindex;
    int output_ifindex;
    struct work_struct add_entry_work;
};


struct fdb_entry_t {
    unsigned char da[ETH_ALEN];
    unsigned char sa[ETH_ALEN];
    int iif;
    int oif;
    struct work_struct fdb_entry_work;
};

/* fpp adapter  public functions */
extern int (*nfp_bridge_add_entry_cmd)(const struct sk_buff *skb);
extern void (*nfp_bridge_clean_by_ifindex)(int ifindex);


int nfp_bridge_init( void );
void nfp_bridge_exit( void );
void nfp_bridge_flush(void);
void nfp_fdb_flush(void);


/*bridge rule hook*/
extern int(*nfp_fdb_hook)(const struct sk_buff  *skb,const struct net_device *indev);

#endif /* __BRIDGE_ADAPTER_H__ */

