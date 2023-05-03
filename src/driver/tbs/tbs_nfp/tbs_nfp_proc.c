/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_proc.c
* 文件描述 : tbs加速器proc配置调试接口
*
* 修订记录 :
*          1 创建 : pengyao
*            日期 : 2011-12-03
*            描述 :
*
*******************************************************************************/

#include "tbs_nfp_proc.h"


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/


/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
struct proc_dir_entry *g_tbs_nfp_proc_dir = NULL;

const static char* g_tbs_nfp_proc[] ={
    TBS_NFP_IF_DIR,
    TBS_NFP_BRIDGE_DIR,
    TBS_NFP_FIB_DIR,
    TBS_NFP_CT_DIR,
    TBS_NFP_RULE_RESET,
    TBS_NFP_HELP,
    TBS_NFP_DEBUG_LEVEL,
    TBS_NFP_ENABLE,
#ifdef TBS_NFP_STAT
    TBS_NFP_STATUS,
#endif
    NULL
    };

const char* g_common_proc[] ={
    TBS_NFP_HELP,
    TBS_NFP_RULE_DUMP,
    TBS_NFP_RULE_ADD,
    TBS_NFP_RULE_DELETE,
    TBS_NFP_RULE_RESET,
    TBS_NFP_RULE_SIZE,
    NULL
    };

/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

int tbs_nfp_proc_itf_init(void);
void tbs_nfp_proc_itf_exit(void);

#ifdef TBS_NFP_FIB
    int tbs_nfp_proc_ct_init(void);
    void tbs_nfp_proc_ct_exit(void);
#endif /*TBS_NFP_FIB*/

#ifdef TBS_NFP_FIB
    int tbs_nfp_proc_fib_init(void);
    void tbs_nfp_proc_fib_exit(void);
    int tbs_nfp_proc_arp_init(void);
    void tbs_nfp_proc_arp_exit(void);
#endif /*TBS_NFP_FIB*/

#ifdef TBS_NFP_BRIDGE
    int tbs_nfp_proc_bridge_init(void);
    void tbs_nfp_proc_bridge_exit(void);
#endif /*TBS_NFP_BRIDGE*/


/*=========================================================================
 Function:		int tbs_nfp_proc_remove_entry(const char *proc_array[],
    struct proc_dir_entry *parent_dir)
 Description:		移除proc节点
 Data Accessed:
 Data Updated:
 Input:			    const char *proc_array[]    proc节点名称指针数组
                    struct proc_dir_entry *parent_dir   目录指针
 Output:			无
 Return:			0:成功  其他:   失败
 Others:
=========================================================================*/
int tbs_nfp_proc_remove_entry(const char *proc_array[],
    struct proc_dir_entry *parent_dir)
{
    const char **proc_entry = NULL;

    if(NULL == proc_array)
        return TBS_NFP_ERR;

    proc_entry = proc_array;

    while(*proc_entry)
    {
        remove_proc_entry(*(proc_entry++), parent_dir);
    }

    return TBS_NFP_OK;
}


/*=========================================================================
 Function:		static int tbs_nfp_proc_reset( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		清除所有转发规则
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_reset( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
    char str_buf[16] = {0};

	/* 限定一条输入长度不大于16 */
	if(len > 15)
	{
	    TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

        if(str_buf[0] != '0')
        {
            tbs_nfp_rule_reset();
        }
    }
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
        goto out;
    }

out:
    return len;
}

/*=========================================================================
 Function:		static int tbs_nfp_proc_enable_write( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		清除所有转发规则
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_enable_write( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
    char str_buf[16] = {0};

	/* 限定一条输入长度不大于16 */
	if(len > 15)
	{
	    TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

        if(str_buf[0] != '0')
            g_nfp_enable = 1;
        else
            g_nfp_enable = 0;
    }
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
        goto out;
    }

out:
    return len;
}


/*=========================================================================
 Function:		static int tbs_nfp_proc_enable_read(char *page, char **start, off_t off,
			  int count, int *eof, void *data))
 Description:		get debug_level value
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_enable_read(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
    char *out = page;
	int len = 0;

    *out++ = g_nfp_enable + '0';
    *out++ = '\n';

    if(g_nfp_enable == 0)
        out += sprintf(out, "Tbs_nfp Disable\n");
    else
        out += sprintf(out, "Tbs_nfp Enable\n");

	len = out - page;
	len -= off;
	if (len < count) {
		*eof = 1;
		if (len <= 0)
            return 0;
	} else
		len = count;

	*start = page + off;
	return len;
}
/*=========================================================================
 Function:		static int tbs_nfp_proc_status_write( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
 Description:		get debug_level value
 Data Accessed:
 Data Updated:
 Input:			0-7
 Output:			无
 Return:			0:成功
 Others:		show the nfp stats static msg
 auther:		baiyonghui
=========================================================================*/
#ifdef TBS_NFP_STAT
static int tbs_nfp_proc_status_write( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
	unsigned int port;
	sscanf(buf, "%u", &port);
	if ((port < 0) || (port >= TBS_ETH_MAX_PORTS)) {
		printk("Invalid port number %d\n", port);
		return len;
	}

	printk("\n====================================================\n");
	printk(" TBS NFP statistics");
	printk("\n----------------------------------------------------\n");
    printk("\trx:               %u\n", g_nfp_stats[port].rx);
    printk("\tiif_err:          %u\n", g_nfp_stats[port].iif_err);
    printk("\tpaser_err:        %u\n", g_nfp_stats[port].paser_err);
    printk("\tmac_mcast:        %u\n", g_nfp_stats[port].mac_mcast);
    printk("\tnon_ip:           %u\n", g_nfp_stats[port].non_ip);
    printk("\tipv4:             %u\n", g_nfp_stats[port].ipv4);
    printk("\tipv6:             %u\n", g_nfp_stats[port].ipv6);
    printk("\tipv4_rx_frag:     %u\n", g_nfp_stats[port].ipv4_rx_frag);
    printk("\tttl_exp:          %u\n", g_nfp_stats[port].ttl_exp);
    printk("\tl4_unknown:       %u\n", g_nfp_stats[port].l4_unknown);
    printk("\ttx_mtu_err:       %u\n", g_nfp_stats[port].tx_mtu_err);
    printk("\ttx:               %u\n", g_nfp_stats[port].tx);

#ifdef TBS_NFP_BRIDGE
	printk("(nfp bridge status)\n");
	printk("\tbridge_hit:       %u\n", g_nfp_stats[port].bridge_hit);
	printk("\tbridge_miss:      %u\n", g_nfp_stats[port].bridge_miss);
	printk("\tbridge_local:     %u\n", g_nfp_stats[port].bridge_local);
#endif /* TBS_NFP_BRIDGE */

#ifdef TBS_NFP_VLAN
	printk("(nfp vlan status)\n");
    printk("\tvlan_rx_not_found:%u\n", g_nfp_stats[port].vlan_rx_not_found);
	printk("\tvlan_rx_found:    %u\n", g_nfp_stats[port].vlan_rx_found);
	printk("\tvlan_rx_trans:    %u\n", g_nfp_stats[port].vlan_rx_trans);
	printk("\tvlan_tx_add:      %u\n", g_nfp_stats[port].vlan_tx_add);
	printk("\tvlan_tx_remove:   %u\n", g_nfp_stats[port].vlan_tx_remove);
	printk("\tvlan_tx_replace:  %u\n", g_nfp_stats[port].vlan_tx_replace);
#endif  /*TBS_NFP_VLAN*/

#ifdef TBS_NFP_PPP
    printk("(nfp pppoe status)\n");
    printk("\tpppoe_rx_not_found:%u\n", g_nfp_stats[port].pppoe_rx_not_found);
    printk("\tpppoe_rx_found:   %u\n", g_nfp_stats[port].pppoe_rx_found);
    printk("\tpppoe_tx_add:     %u\n", g_nfp_stats[port].pppoe_tx_add);
    printk("\tpppoe_tx_remove:  %u\n", g_nfp_stats[port].pppoe_tx_remove);
    printk("\tpppoe_tx_replace: %u\n", g_nfp_stats[port].pppoe_tx_replace);
#endif  /*TBS_NFP_PPP*/

#ifdef TBS_NFP_FIB
    printk("(nfp fib status)\n");
    printk("\tfib_hit:          %u\n", g_nfp_stats[port].fib_hit);
#endif  /*TBS_NFP_FIB*/

#ifdef TBS_NFP_CT
    printk("(nfp ct status)\n");
    printk("\tct_hit:           %u\n", g_nfp_stats[port].ct_hit);
    printk("\tct_tcp_fin_rst:   %u\n", g_nfp_stats[port].ct_tcp_fin_rst);
#endif /* TBS_NFP_CT */

#ifdef TBS_NFP_NAT
    printk("(nfp nat status)\n");
    printk("\tdnat_hit:         %u\n", g_nfp_stats[port].dnat_hit);
    printk("\tsnat_hit:         %u\n", g_nfp_stats[port].snat_hit);
#endif	/* TBS_NFP_NAT */

	return len;
}
#endif /* TBS_NFP_STAT */


/*=========================================================================
 Function:		tatic int tbs_nfp_proc_help(char *buf, char **start, off_t offset,
                    int count, int *eof, void *data)
 Description:		显示tbs_nfp目录下 proc help信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_help(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    printk("Local commands:\n");
    printk("cat                                             help           - print this help\n");
    printk("echo 1                                        > reset          - clean all nfp rule\n");
    printk("echo 1/0                                      > enable         - enable/disable tbs_nfp\n");
    printk("echo <0-7>                                    > status         - print NFP port statistics\n");
    printk("echo Debug_level                              > debug_level    - set debug level\n");
    printk("cat                                             debug_level    - print debug delvel\n"
            "\t\t0:TBS_NFP_NO_PRINT; 1:TBS_NFP_WARN_PRINT; 3:TBS_NFP_DBG_PRINT\n");

    return len;
}


/*=========================================================================
 Function:		static int tbs_nfp_proc_level_read(char *page, char **start, off_t off,
			  int count, int *eof, void *data))
 Description:		get debug_level value
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_level_read(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
    char *out = page;
	int len = 0;

    *out++ = g_debug_level + '0';
    *out++ = '\n';

    out += sprintf(out, "%d:    TBS_NFP_NO_PRINT\n", TBS_NFP_NO_PRINT);
    out += sprintf(out, "%d:    TBS_NFP_WARN_PRINT\n", TBS_NFP_WARN_PRINT);
    out += sprintf(out, "%d:    TBS_NFP_DBG_PRINT\n", TBS_NFP_DBG_PRINT);

	len = out - page;
	len -= off;
	if (len < count) {
		*eof = 1;
		if (len <= 0)
            return 0;
	} else
		len = count;

	*start = page + off;
	return len;
}


/*=========================================================================
 Function:		static int tbs_nfp_proc_level_write( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		set debug_level value
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_level_write( struct file *filp, const char __user *buf,unsigned long len, void *data )
{
    char str_buf[16];

    if(len > 15)
    {
        printk("Error. Sample: echo Debug_level > /proc/tbs_nfp/debug_level \n");
        return len;
    }

    if (copy_from_user(str_buf,buf,len))
        return -EFAULT;

    g_debug_level = str_buf[0] - '0';

    if(g_debug_level == TBS_NFP_NO_PRINT)
        printk("\nTbs nfp debug_leve: TBS_NFP_NO_PRINT\n");
    else if(g_debug_level == TBS_NFP_DBG_PRINT)
        printk("\nTbs nfp debug_leve: TBS_NFP_DBG_PRINT\n");
    else if(g_debug_level == TBS_NFP_WARN_PRINT)
        printk("\nTbs nfp debug_leve: TBS_NFP_WARN_PRINT\n");

    return len;
}


/*=========================================================================
 Function:      int tbs_nfp_proc_init(void)

 Description:       proc 调试接口初始化
 Data Accessed:     itf/bridge/fib/ct hash表
 Data Updated:
 Input:             无
 Output:            无
 Return:            TBS_NFP_OK: 成功；其他: 失败
 Others:
=========================================================================*/
int tbs_nfp_proc_init(void)
{
    struct proc_dir_entry *proc_entry = NULL;

    g_tbs_nfp_proc_dir = proc_mkdir(TBS_NFP_DIR, init_net.proc_net);
    if(NULL == g_tbs_nfp_proc_dir)
        goto err;

    if(tbs_nfp_proc_itf_init())
        goto err;

#ifdef TBS_NFP_BRIDGE
    if(tbs_nfp_proc_bridge_init())
        goto err;
#endif /*TBS_NFP_BRIDGE*/

#ifdef TBS_NFP_FIB
    if(tbs_nfp_proc_fib_init())
        goto err;

    if(tbs_nfp_proc_arp_init())
        goto err;
#endif /*TBS_NFP_FIB*/

#ifdef TBS_NFP_CT
    if(tbs_nfp_proc_ct_init())
        goto err;
#endif /*TBS_NFP_CT*/

    proc_entry = create_proc_entry(TBS_NFP_HELP, 0444, g_tbs_nfp_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_DIR"/"TBS_NFP_HELP);
        goto err;
    }
    proc_entry->read_proc = &tbs_nfp_proc_help;

    proc_entry = create_proc_entry(TBS_NFP_RULE_RESET, 0222, g_tbs_nfp_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_DIR"/"TBS_NFP_RULE_RESET);
        goto err;
    }
    proc_entry->write_proc = &tbs_nfp_proc_reset;


    proc_entry = create_proc_entry(TBS_NFP_DEBUG_LEVEL, 0666, g_tbs_nfp_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_DIR"/"TBS_NFP_DEBUG_LEVEL);
        goto err;
    }
    proc_entry->write_proc = &tbs_nfp_proc_level_write;
    proc_entry->read_proc = &tbs_nfp_proc_level_read;

    proc_entry = create_proc_entry(TBS_NFP_ENABLE, 0666, g_tbs_nfp_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_DIR"/"TBS_NFP_ENABLE);
        goto err;
    }
    proc_entry->write_proc = &tbs_nfp_proc_enable_write;
    proc_entry->read_proc = &tbs_nfp_proc_enable_read;

/*
 * TBS_TAG: add by baiyonghui 2011-12-12
 * Description:
 */
#ifdef TBS_NFP_STAT
	proc_entry = create_proc_entry(TBS_NFP_STATUS, 0222, g_tbs_nfp_proc_dir);
	if(NULL == proc_entry)
	{
		TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_DIR"/"TBS_NFP_STATUS);
		goto err;
	}
	proc_entry->write_proc = &tbs_nfp_proc_status_write;
#endif

/*
 * TBS_END_TAG
 */
    return TBS_NFP_OK;

err:
    if(g_tbs_nfp_proc_dir)
    {
        tbs_nfp_proc_remove_entry(g_tbs_nfp_proc, g_tbs_nfp_proc_dir);
        remove_proc_entry(TBS_NFP_DIR, init_net.proc_net);
    }

    g_tbs_nfp_proc_dir = NULL;

    return TBS_NFP_ERR;
}


/*=========================================================================
 Function:      void tbs_nfp_proc_exit(void)

 Description:       proc 调试接口释放
 Data Accessed:     itf/bridge/fib/ct hash表
 Data Updated:
 Input:             无
 Output:            无
 Return:            无
 Others:
=========================================================================*/
void tbs_nfp_proc_exit(void)
{
    /*interface proc节点释放*/
    tbs_nfp_proc_itf_exit();

#ifdef TBS_NFP_BRIDGE
    tbs_nfp_proc_bridge_exit();
#endif

#ifdef TBS_NFP_FIB
    tbs_nfp_proc_fib_exit();
    tbs_nfp_proc_arp_exit();
#endif

#ifdef TBS_NFP_CT
    tbs_nfp_proc_ct_exit();
#endif

    tbs_nfp_proc_remove_entry(g_tbs_nfp_proc, g_tbs_nfp_proc_dir);
    remove_proc_entry(TBS_NFP_DIR, init_net.proc_net);
}

