/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称 : nfp_adapter.h
 文件描述 : TBS加速器适配

 修订记录 :
          1 创建 : 彭耀
            日期 : 2011-04-10
            描述 :
**********************************************************************/

#ifndef _NFP_ADAPTER_H_
#define _NFP_ADAPTER_H_

#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv6/nf_conntrack_ipv6.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <net/netfilter/nf_conntrack_tuple.h>

#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/jhash.h>
#include <linux/workqueue.h>
#include <linux/err.h>
#include <linux/sysctl.h>
#include <asm/atomic.h>

/*in build directory*/
#include "autoconf.h"

#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE)
/*in kernel dir*/
#include <linux/../../net/bridge/br_private.h>
#endif

/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/
#define NFP_ADAPTER_NAME    "nfp_adapter"

#define NFP_ADAPTER_SUCCESS 0
#define NFP_ADAPTER_FAULT   -1

/*规则同步到协议栈操作类型*/
#define NFP_TIMEOUT  0X1
#define NFP_UPDATE    0X2

/*加速器配置规则类型命令字*/
#define NFP_CMD_ITF     0X1
#define NFP_CMD_BRIDGE  0X2
#define NFP_CMD_ARP     0X3
#define NFP_CMD_FIB     0X4
#define NFP_CMD_CT      0X5
#define NFP_CMD_RESET   0X6


/*加速器配置操作命令字*/
#define NFP_OPT_NOTHING 0X0
#define NFP_OPT_ADD     0X1
#define NFP_OPT_DELETE  0X2
#define NFP_OPT_STAT    0X3

/*only use for NFP_CMD_ITF*/
#define NFP_OPT_CHANGE_ADDR     0x4
#define NFP_OPT_BRPORT_ADD      0X5
#define NFP_OPT_CHANGE_MTU      0X6
#define NFP_OPT_CHANGE_NAME     0X7
#define NFP_OPT_BRPORT_DELETE   0X8
/*only use for dummyport*/
#define NFP_OPT_DUMMYPORT_BIND  0X9
#define NFP_OPT_DUMMYPORT_UNBIND 0X10


/*加速器配置接口返回值*/
#define NFP_NO_ERR                  0
#define ERR_NFP_UNKNOWN_CMD         1
#define ERR_NFP_UNKNOWN_OPTION      2
#define ERR_NFP_ALREADY_EXIST       3
#define ERR_NFP_NOT_FOUND           4
#define ERR_NFP_OTHER               5
#define NFP_RULE_TIMEOUT            6
#define NFP_RULE_ACTIVE             7
#define NFP_UNABLE_PARSER           8


#define NFP_INET_ADDRSTRLEN     40
#define NFP_MAC_ADDRSTRLEN      18

/*定义hash桶大小*/
#define NFP_ROUTE_HTABLE_SIZE       256
#define NFP_CT_HTABLE_SIZE          256
#define NFP_ITF_HTABLE_SIZE	        64
#define NFP_NEIGHBOR_HTABLE_SIZE    256
#define NFP_FDB_HTABLE_SIZE         64
#define NFP_BRIDGE_HTABLE_SIZE      64

#define NFP_ADAPTER_VERSION    "TBS_NFP_ADAPTER_0_01_1"
#define	NFP_ARRAY_SIZE(x)	( sizeof(x) / sizeof( (x)[0]) )

extern struct list_head nfp_interface_table[NFP_ITF_HTABLE_SIZE];

extern struct list_head fdb_tables_by_mac[NFP_FDB_HTABLE_SIZE];

#if 0
extern struct list_head *nfp_alloc_hashtable(int size, int *vmalloced);
extern void nfp_free_hashtable(struct list_head *hash, int vmalloced, int size);
extern void nfp_init_listhead(struct list_head *hash, int size);
#endif

/*********************************************************************
 *                              DEBUG                                *
 *********************************************************************/

#if defined(CONFIG_NFP_ADAPTER_DEBUG)
    #define NFP_ADAPTER_DEBUG(fmt, args...) COMMON_TRACE(fmt, ##args)
#else
    #define NFP_ADAPTER_DEBUG(fmt, args...)  do { ; } while(0)
#endif

#define NFP_ADAPTER_ERROR(fmt, args...)\
    do {  \
            printk(KERN_ERR "[%s:%4d] %20s: " fmt, strrchr(__FILE__, '/'), __LINE__, __FUNCTION__, ##args); \
    } while(0)

#define NFP_ADAPTER_DEBUG_LEVEL(level, fmt, args...)\
    do {  \
            printk(level "[%s:%4d] %20s: " fmt, strrchr(__FILE__, '/'), __LINE__, __FUNCTION__, ##args); \
    } while(0)

#define NFP_ADAPTER_INTO_FUNC  do { ; } while(0)//NFP_ADAPTER_DEBUG("##In## %s\n", __func__)
#define NFP_ADAPTER_OUT_FUNC do { ; } while(0) //NFP_ADAPTER_DEBUG("##Out## %s\n", __func__)


#define COMMON_TRACE(fmt, args...) \
    do {  \
            printk("[%s:%4d] %20s: " fmt, strrchr(__FILE__, '/'), __LINE__, __FUNCTION__, ##args); \
    } while(0)


/*********************************************************************
 *                              STRUCT                               *
 *********************************************************************/
struct nfp_adapter_cmd{
    unsigned short option;
    void *rule_entry;
};


/******************************************************************************
 *                                 ENUM                                       *
 ******************************************************************************/


/*********************************************************************
 *                              FUNCTION                             *
 *********************************************************************/
/*=========================================================================
 Function:      static inline void nfp_l3_addr_copy(int family, unsigned char *dst, const unsigned char *src)
 Description:       L3 address copy. Supports AF_INET and AF_INET6
 Data Accessed:
 Data Updated:
 Input:
 Output:            无
 Return:            无

 Others:
=========================================================================*/
static inline void nfp_l3_addr_copy(int family, unsigned char *dst, const unsigned char *src)
{
    const unsigned int *s = (const u32 *)src;
    unsigned int *d = (unsigned int *) dst;

    *d++ = *s++;        /* 4 */
    if (family == AF_INET)
        return;

    *d++ = *s++;        /* 8 */
    *d++ = *s++;        /* 12 */
    *d++ = *s++;        /* 16 */
}


/*加速器规则配置解决*/
typedef int (*nfp_cmd_parser_fun)(unsigned short, const struct nfp_adapter_cmd *);
extern int nfp_cmd_parser_register(nfp_cmd_parser_fun new);
extern void nfp_cmd_parser_unregister(nfp_cmd_parser_fun new);
extern void nfp_rule_flush(void);

/******************************************************************************
 *                               GLOBAL VAR                                   *
 ******************************************************************************/
extern nfp_cmd_parser_fun nfp_cmd_parser;
extern struct proc_dir_entry *nfp_adapter_proc;



/*********************************************************************
 *                              TOOLS                                *
 *********************************************************************/

#define NFP_ASSERT(i)   BUG_ON(!(i))


/*=========================================================================
 Function:

 Description:		将网络字节序ipv4/v6地址分别转换成点分十进制和冒号分十六进制格式
 Data Accessed:

 Data Updated:
 Input:			    int family          协议族
                    ip_addr             ipv4/v6网络序地址指针
                    addr_buf            字符串缓冲区(ipv6地址缓冲区为40Byte)
                    buf_size            缓冲区大小
 Output:			无
 Return:			NULL:不存在；其他:neighbour指针

 Others:
=========================================================================*/
#ifndef NIP6_FMT
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#endif
#define NFP_INET_NTOP(family, ip_addr, addr_buf, buf_size) \
    do { \
        if(likely(AF_INET == family)) { \
            snprintf(addr_buf, buf_size, NIPQUAD_FMT, NFP_NIPQUAD(ip_addr)); \
        }else { \
            snprintf(addr_buf, buf_size, NIP6_FMT, NFP_NIP6(ip_addr)); \
        } \
    }while(0)

#define NFP_NIPQUAD(paddr) \
            ((unsigned char *)paddr)[0], \
            ((unsigned char *)paddr)[1], \
            ((unsigned char *)paddr)[2], \
            ((unsigned char *)paddr)[3]

#define NFP_NIP6(paddr) \
            ntohs(((__be16 *)paddr)[0]), \
            ntohs(((__be16 *)paddr)[1]), \
            ntohs(((__be16 *)paddr)[2]), \
            ntohs(((__be16 *)paddr)[3]), \
            ntohs(((__be16 *)paddr)[4]), \
            ntohs(((__be16 *)paddr)[5]), \
            ntohs(((__be16 *)paddr)[6]), \
            ntohs(((__be16 *)paddr)[7])


extern int nfp_bridge_init( void );
extern void nfp_bridge_exit( void );

#endif
