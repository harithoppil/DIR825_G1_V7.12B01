#include <linux/list.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>   
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    
#include <asm/system.h>

#ifdef CONFIG_DEBUG
int DBG_LV=1;
#define DEBUG 1
#define UTM_PRINTK(level,fmt,args...) if(level<=DBG_LV) printk(fmt,##args);
#else
int DBG_LV=1;
#define DEBUG 0
#define UTM_PRINTK(level,fmt,args...)
#endif

#ifndef LINUX_VERSION_CODE
#define LINUX_VERSION_CODE KERNEL_VERSION(2,6,21)
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,21)
// IPv6 header
#define IPV6HDR(a)			a->nh.ipv6h
// Network protocol header treated as raw data
#define	NETHDR(a)			a->nh.raw
// TCP header
#define	TCPHDR(a)			a->h.th
#else
// IPv6 header
#define IPV6HDR(a)			(ipv6_hdr(a))
// Network protocol header treated as raw data
#define	NETHDR(a)			(skb_network_header(a))
// TCP header
#define	TCPHDR(a)			(tcp_hdr(a))
#endif

#ifdef LITTLE_ENDIAN
#define TCP_FLAG_ACT(a)			((a&0x0000FF00))
#else
#define TCP_FLAG_ACT(a)			((a&0x00FF0000))
#endif

enum UTM_TX_STATUS
{
    UTM_NAT_TX_OK,
    UTM_NAT_TX_DROP,
    UTM_NAT_TX_PASS,
    UTM_NAT_TX_STATUS_TOTAL
};

enum UTM_RX_STATUS
{
    UTM_NAT_RX_OK,
    UTM_NAT_RX_DROP,
    UTM_NAT_RX_PASS,
    UTM_NAT_RX_STATUS_TOTAL
};

enum utm_nat_status
{
    utm_nat_status_success,
    utm_nat_status_drop,
    utm_nat_status_pass,
    utm_nat_status_total
};

struct utm_nat_tuple
{
    /*52 BYTES*/
    /*The detail of connection track tuple*/
    /*TBD*/
    struct in6_addr org_saddr;
    struct in6_addr org_daddr;
    u_int16_t	    org_sport;
    u_int16_t	    org_dport;
    //u_int16_t	    protocol;
    //u_int32_t	    sin6_scope_id;
    u_int32_t	    timestamp;
    //u_int8_t	    tcpdone;
    u_int8_t	    rx_fin;
    u_int8_t	    rx_fin_ack;
    u_int8_t	    tx_fin;
    u_int8_t	    tx_fin_ack;
    //u_int32_t	    seq_num;
};

struct utm_nat_tuple_hash
{
    /*52 BYTES*/
    struct hlist_node utm_list;/*8 BYTES*/
    struct utm_nat_tuple tuple;/*44 BYTES*/
};

