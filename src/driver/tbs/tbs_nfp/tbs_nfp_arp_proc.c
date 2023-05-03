/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_fib_proc.c
* 文件描述 : tbs加速器fib rule proc配置调试接口
*
* 修订记录 :
*          1 创建 :
*            日期 : 2011-12-03
*            描述 :
*
*******************************************************************************/

#include "tbs_nfp_proc.h"
#include "tbs_nfp_arp.h"
#include <linux/inet.h>
/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/



/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
static struct proc_dir_entry *g_arp_proc_dir = NULL;
extern unsigned int g_arp_max_size;



/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:		tatic int tbs_nfp_proc_arp_help(char *buf, char **start, off_t offset,
                    int count, int *eof, void *data)
 Description:		显示fib proc help信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_arp_help(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    printk("Local commands:\n");
    printk("cat                                             help           - print this help\n");
    printk("cat                                             dump           - print all arp rule\n");
    printk("echo family(AF_INET|AF_INET6) internet_addr   phy_addr          > rule_add       - add arp rule\n");
    printk("echo family(AF_INET|AF_INET6) internet_addr   phy_addr          > rule_del       - del arp rule\n");
    printk("echo MAX_SIZE                                                   > max_size       - set max size\n");
    printk("echo 1                                        > reset          - clean all arp\n");
    return len;
}


/*=========================================================================
 Function:		tatic int tbs_nfp_proc_arp_dump(char *buf, char **start, off_t offset,
                    int count, int *eof, void *data)
 Description:		显示arp proc help信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_arp_dump(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    tbs_nfp_arp_dump();
    return 0;
}


/*=========================================================================
 Function:		static int tbs_nfp_proc_arp_add(struct file *filp, const char __user *buf,
                    unsigned long len, void *data)

 Description:		proc 添加fib规则,
                    使用方法:echo family gtw mac_addr >rule_add
 Data Accessed:     g_arp_hash[hash]

 Data Updated:      g_arp_hash[hash]

 Input:			    const char __user *buf,        通过proc输入的字符串
                    unsigned long len              字符串的长度
 Output:			无
 Return:			len:成功
 author:    tbs cairong
=========================================================================*/
static int tbs_nfp_proc_arp_add(struct file *filp, const char __user *buf,
                    unsigned long len, void *data)
{
	int ret = 0;
    char str_buf[64] = {0};

  	unsigned char inet_family[16] = {0};
	unsigned char next_ip_str[INET6_ADDRSTRLEN] = {0};
    unsigned char dst_mac_str[TBS_NFP_MAC_ADDRSTRLEN] = {0};

    int family = AF_INET;
	unsigned char next_ip[TBS_NFP_MAX_L3_ADDR_SIZE] = {0};
  	unsigned char dst_mac[ETH_ALEN] = {0};

	/* 限定一条输入长度不大于64 */
	if(len > (sizeof(str_buf) - 1))
	{
	    //TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
        TBS_NFP_ERROR("Usage: echo family(2/11) gtw_ip mac_addr > rule_add\n");

        return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

		ret = sscanf(str_buf, "%s %s %s", \
                    inet_family, next_ip_str, dst_mac_str);

        if(ret != 3)
		{
            //TBS_NFP_ERROR("Write_proc:echo family internet_addr phy_addr > rule_add\n");
            TBS_NFP_ERROR("Usage: echo family(2/11) gtw_ip mac_addr > rule_add\n");

            return len;
        }

        if(!strncmp(inet_family,"AF_INET",sizeof("AF_INET")))
        {
            family = AF_INET;
        }
        else if(!strncmp(inet_family,"AF_INET6",sizeof("AF_INET6")))
        {
            family = AF_INET6;
        }
        else
        {
            TBS_NFP_ERROR("family type unkown\n");
            return len;
        }

        if(AF_INET == family)
        {
    		if(true!= in4_pton(next_ip_str, strlen(next_ip_str),(u8 *)next_ip, '\0',NULL))
    		{
    			TBS_NFP_ERROR("Write_proc: invalid srcip4 addr!\n");
    			return len;
    		}
        }
        else
        {
             /*ipv6 form convert 字符串转化为网络字节序*/
     		if(true!= in6_pton(next_ip_str, strlen(next_ip_str),(u8 *)next_ip, '\0',NULL))
    		{
    			TBS_NFP_ERROR("Write_proc: invalid srcip6 addr!\n");
    			return len;
    		}
        }

		if((false == tbs_mac_str_to_hex(dst_mac_str, dst_mac)))
		{
			TBS_NFP_ERROR("Write_proc: invalid mac addr!\n");
			return len;
		}

    }
    else
    {
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }

	if (TBS_NFP_OK != tbs_nfp_arp_add(family,
                        (const unsigned char *)next_ip,
                        (const unsigned char *)dst_mac))
	{
		TBS_NFP_ERROR("tbs_nfp_arp_rule_add fail \n");
	}

    printk("arp rule add ok\n");

    return len;

}


/*=========================================================================
 Function:		static int tbs_nfp_proc_arp_del(struct file *filp, const char __user *buf,
                    unsigned long len, void *data)

 Description:		proc 删除fib规则,
                    使用方法:echo family sip dip gtw oifname >fib_del
 Data Accessed:     g_fib_hash[hash],g_fib_hash_by_gtw[hash]
                    g_fib_inv_hash[hash],g_fib_inv_hash_by_gtw[hash]

 Data Updated:      g_fib_hash[hash],g_fib_hash_by_gtw[hash]
                    g_fib_inv_hash[hash],g_fib_inv_hash_by_gtw[hash]
 Input:			    const char __user *buf,        通过proc输入的字符串
                    unsigned long len              字符串的长度
 Output:			无
 Return:			len:成功
 author:    tbs cairong
=========================================================================*/

static int tbs_nfp_proc_arp_del(struct file *filp, const char __user *buf,
                    unsigned long len, void *data)
{
	int ret = 0;
    char str_buf[64] = {0};

  	unsigned char inet_family[16] = {0};
	unsigned char next_ip_str[INET6_ADDRSTRLEN] = {0};

    int family = AF_INET;
	unsigned char next_ip[TBS_NFP_MAX_L3_ADDR_SIZE] = {0};

	/* 限定一条输入长度不大于64 */
	if(len > (sizeof(str_buf) - 1))
	{
	    //TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
        TBS_NFP_ERROR("Usage: echo family(2/11) gtw_ip > rule_delete\n");

	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

		ret = sscanf(str_buf, "%s %s", \
                    inet_family, next_ip_str);

        if(ret != 2)
		{
            //TBS_NFP_ERROR("Write_proc:\n");
            TBS_NFP_ERROR("Usage: echo family(2/11) gtw_ip > rule_delete\n");
            return len;
        }

        if(!strncmp(inet_family,"AF_INET",sizeof("AF_INET")))
        {
            family = AF_INET;
        }
        else if(!strncmp(inet_family,"AF_INET6",sizeof("AF_INET6")))
        {
            family = AF_INET6;
        }
        else
        {
            TBS_NFP_ERROR("family type unkown\n");
            return len;
        }

        if(AF_INET == family)
        {
    		if(true!= in4_pton(next_ip_str, strlen(next_ip_str),(u8 *)next_ip, '\0',NULL))
    		{
    			TBS_NFP_ERROR("Write_proc: invalid srcip4 addr!\n");
    			return len;
    		}
        }
        else
        {
             /*ipv6 form convert 字符串转化为网络字节序*/
     		if(true!= in6_pton(next_ip_str, strlen(next_ip_str),(u8 *)next_ip, '\0',NULL))
    		{
    			TBS_NFP_ERROR("Write_proc: invalid srcip6 addr!\n");
    			return len;
    		}
        }
    }
    else
    {
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }

	if (TBS_NFP_OK != tbs_nfp_arp_delete(family,
                        (const unsigned char *)next_ip))
	{
		TBS_NFP_ERROR("tbs_nfp_arp_delete fail \n");
	}

    printk("del ok\n");

    return len;
}

/*=========================================================================
 Function:		static int tbs_nfp_proc_fib_reset( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		清除fib rule
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_arp_reset( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
    char str_buf[16] = {0};

	/* 限定一条输入长度不大于16 */
	if(len > 15)
	{
	    //TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
	    TBS_NFP_ERROR("Usage: echo 0/1\n");

	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

        if(str_buf[0] != '0')
        {
            tbs_nfp_arp_reset();
        }
    }
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
    }

    return len;
}


static inline int tbs_nfp_proc_arp_size_set( struct file *filp, const char __user *buf,
		unsigned long len, void *data )
{
    int ret = 0;
    unsigned int max_size = 0;
	char str_buf[16] = {0};

	/* 限定一条输入长度不大于16 */
	if(len > (sizeof(str_buf) - 1 ))
	{
		TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
		return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
		str_buf[len]='\0';

        ret = sscanf(str_buf, "%u", &max_size);
        if(max_size > 4096)
        {
            TBS_NFP_ERROR("max_size out of range 4096\n");
        }

        g_arp_max_size = max_size;
	}
	else{
		TBS_NFP_ERROR("str_buf is NULL\n");
	}

	return len;
}


static inline int tbs_nfp_proc_arp_size_get(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    printk("arp rule max size: %u\n", g_arp_max_size);

    return 0;
}


/*=========================================================================
 Function:		int tbs_nfp_proc_fib_init(void)
 Description:		fib proc 调试接口初始化
 Data Accessed:
 Data Updated:
 Input:			    无
 Output:			无
 Return:			0: 成功
 Others:
=========================================================================*/
int tbs_nfp_proc_arp_init(void)
{
    struct proc_dir_entry *proc_entry = NULL;

    g_arp_proc_dir = proc_mkdir(TBS_NFP_ARP_DIR, g_tbs_nfp_proc_dir);
    if(NULL == g_arp_proc_dir)
    {
        TBS_NFP_ERROR("proc_mkdir(%s) fail\n", TBS_NFP_ARP_DIR);
        goto err;
    }

    proc_entry = create_proc_entry(TBS_NFP_HELP, 0444, g_arp_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_DIR"/"TBS_NFP_ARP_DIR"/"TBS_NFP_HELP);
        goto err;
    }
    proc_entry->read_proc = &tbs_nfp_proc_arp_help;


    proc_entry = create_proc_entry(TBS_NFP_RULE_DUMP, 0444, g_arp_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_ARP_DIR"/"TBS_NFP_RULE_DUMP);
        goto err;
    }
    proc_entry->read_proc = &tbs_nfp_proc_arp_dump;


    proc_entry = create_proc_entry(TBS_NFP_RULE_ADD, 0222, g_arp_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_ARP_DIR"/"TBS_NFP_RULE_ADD);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_arp_add;


    proc_entry = create_proc_entry(TBS_NFP_RULE_DELETE, 0222, g_arp_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_ARP_DIR"/"TBS_NFP_RULE_DELETE);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_arp_del;

    proc_entry = create_proc_entry(TBS_NFP_RULE_RESET, 0222, g_arp_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_ARP_DIR"/"TBS_NFP_RULE_RESET);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_arp_reset;

    proc_entry = create_proc_entry(TBS_NFP_RULE_SIZE, 0666, g_arp_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_FIB_DIR"/"TBS_NFP_RULE_SIZE);
        goto err;
    }
    proc_entry->write_proc = &tbs_nfp_proc_arp_size_set;
    proc_entry->read_proc = &tbs_nfp_proc_arp_size_get;

    return TBS_NFP_OK;

err:

    if(g_arp_proc_dir)
    {
        /*移除fib 目录下节点*/
        tbs_nfp_proc_remove_entry(g_common_proc, g_arp_proc_dir);

        remove_proc_entry(TBS_NFP_FIB_DIR, g_tbs_nfp_proc_dir);
    }

    g_arp_proc_dir = NULL;

    return TBS_NFP_ERR;
}


/*=========================================================================
 Function:		int tbs_nfp_proc_arp_exit(void)
 Description:		fib proc 调试接口退出
 Data Accessed:
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_proc_arp_exit(void)
{
    if(g_arp_proc_dir)
    {
        /*移除fib目录下节点*/
        tbs_nfp_proc_remove_entry(g_common_proc, g_arp_proc_dir);

        remove_proc_entry(TBS_NFP_ARP_DIR, g_tbs_nfp_proc_dir);
    }

    g_arp_proc_dir = NULL;
}

