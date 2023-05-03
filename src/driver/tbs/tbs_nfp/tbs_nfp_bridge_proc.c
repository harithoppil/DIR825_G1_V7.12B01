/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_bridge_proc.c
* 文件描述 : tbs加速器bridge rule proc配置调试接口
*
* 修订记录 :
*          1 创建 : pengyao
*            日期 : 2011-12-03
*            描述 :
*
*******************************************************************************/

#include "tbs_nfp_proc.h"
#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE)
/*in kernel dir*/
#include <linux/../../net/bridge/br_private.h>
#endif


/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/



/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
static struct proc_dir_entry *g_bridge_proc_dir = NULL;
extern unsigned int g_bridge_max_size;



/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:		tatic int tbs_nfp_proc_bridge_help(char *buf, char **start, off_t offset,
                    int count, int *eof, void *data)
 Description:		显示bridge proc help信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_bridge_help(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    printk("Local commands:\n");
    printk("cat                                             help           - print this help\n");
    printk("cat                                             dump           - print all bridge rule\n");
    printk("echo src_addr dst_addr iif oif                > rule_add       - add bridge rule\n");
    printk("echo src_addr dst_addr iif                    > rule_delete    - delete bridge rule\n");
    printk("echo MAX_SIZE                                 > max_size       - set max size\n");
    printk("echo 1                                        > reset          - clean all bridge rule\n");

    return len;
}


/*=========================================================================
 Function:		tatic int tbs_nfp_proc_bridge_dump(char *buf, char **start, off_t offset,
                    int count, int *eof, void *data)
 Description:		显示bridge rule proc help信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_bridge_dump(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    printk("Bridge rule dump:\n");
    tbs_nfp_bridge_rule_dump();
    return 0;
}


/*=========================================================================
 Function:		static int tbs_nfp_proc_bridge_reset( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		清除bridge rule
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_bridge_reset( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
    char str_buf[16] = {0};

	/* 限定一条输入长度不大于16 */
	if(len > (sizeof(str_buf) - 1))
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
            tbs_nfp_bridge_reset();
        }
    }
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
    }

    return len;
}


#define MAC_SIZE 24
#define strtol              simple_strtoul
bool tbs_mac_str_to_hex(const char *mac_str, u8 *mac_hex)
{
	int i;
	char tmp[3];

	for (i = 0; i < ETH_ALEN; i++) {
		tmp[0] = mac_str[(i * 3) + 0];
		tmp[1] = mac_str[(i * 3) + 1];
		if(('0' > tmp[0] || 'f' < tmp[0])||('0' > tmp[1] || 'f' < tmp[1]))
			return false;
		tmp[2] = '\0';
		mac_hex[i] = (u8) (strtol(tmp, NULL, 16));
	}
	return true;
}

/*=========================================================================
 Function:		static int tbs_nfp_proc_bridge_add( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		添加interface
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_bridge_add( struct file *filp, const char __user *buf,
                    unsigned long len, void *data )
{
	int ret = 0;
    char str_buf[64] = {0};
    char ifname[IFNAMSIZ] = {0};
    char ofname[IFNAMSIZ] = {0};
    struct net_device *idev = NULL;
    struct net_device *odev = NULL;
	char sa_str[MAC_SIZE], da_str[MAC_SIZE];

	u8 sa[ETH_ALEN], da[ETH_ALEN];

	/* 限定一条输入长度不大于32 */
	if(len > (sizeof(str_buf) - 1))
	{
	    //TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
        TBS_NFP_ERROR("Usage: src_mac dst_mac iif_name oif_name\n");

	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

		ret = sscanf(str_buf, "%s %s %s %s", sa_str, da_str, ifname, ofname);
        if(ret != 4)
		{
            //TBS_NFP_ERROR("Write_proc: Need 4 parameters to write in\n");
            TBS_NFP_ERROR("Usage: src_mac dst_mac iif_name oif_name\n");

            return len;
        }

		if((false == tbs_mac_str_to_hex(sa_str, sa)) ||
				(false == tbs_mac_str_to_hex(da_str, da)))
		{
			TBS_NFP_ERROR("Write_proc: invalid mac addr!\n");
			return len;
		}

        ifname[IFNAMSIZ - 1] = '\0';
    }
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }

    idev = dev_get_by_name(&init_net, ifname);
    if(NULL == idev)
    {
        TBS_NFP_ERROR("interface %s no exist\n", ifname);
		return len;
    }

    odev = dev_get_by_name(&init_net, ofname);
    if(NULL == idev)
    {
        TBS_NFP_ERROR("interface %s no exist\n", ofname);
		goto out;
    }

	//if((NULL == idev->br_port) && (NULL == odev->br_port))
	if((NULL == br_port_get_rcu(idev)) && (NULL == br_port_get_rcu(odev)))
	{
		TBS_NFP_ERROR("idev(%s) or odev(%s) isn't bridge stuff!\n", ifname, ofname);
		goto out;
	}

	if(tbs_nfp_bridge_rule_add(sa, da, idev->ifindex, odev->ifindex) != TBS_NFP_OK)
	{
		TBS_NFP_ERROR("tbs_nfp_bridge_rule_add(%s) fail \n", ifname);
	}

out:
    if(idev)
        dev_put(idev);
    if(odev)
        dev_put(odev);
    return len;
}


/*=========================================================================
 Function:		static int tbs_nfp_proc_bridge_delete( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
 Description:		添加interface
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_bridge_delete( struct file *filp, const char __user *buf,
                    unsigned long len, void *data )
{
	int ret = 0;
    char str_buf[64] = {0};
    char ifname[IFNAMSIZ] = {0};
    struct net_device *dev = NULL;
	char sa_str[MAC_SIZE], da_str[MAC_SIZE];

	u8 sa[ETH_ALEN], da[ETH_ALEN];

	/* 限定一条输入长度不大于64 */
	if(len > (sizeof(str_buf) - 1))
	{
	    //TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
        TBS_NFP_ERROR("Usage: src_mac dst_mac iif_name\n");

	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
		str_buf[len]='\0';

		ret = sscanf(str_buf, "%s %s %s", sa_str, da_str, ifname);

		if(ret != 3)
		{
			//TBS_NFP_ERROR("Write_proc: Need 3 parameters to write in\n");
            TBS_NFP_ERROR("Usage: src_mac dst_mac iif_name\n");

			return len;
		}
		//tbs_mac_str_to_hex(sa_str, sa);
		//tbs_mac_str_to_hex(da_str, da);

		if((false == tbs_mac_str_to_hex(sa_str, sa)) ||
				(false == tbs_mac_str_to_hex(da_str, da)))
		{
			TBS_NFP_ERROR("Write_proc: invalid mac addr!\n");
			return len;
		}
		ifname[IFNAMSIZ - 1] = '\0';

	}
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }

    dev = dev_get_by_name(&init_net, ifname);
    if(NULL == dev)
    {
        TBS_NFP_ERROR("interface %s no exist\n", ifname);
		return len;
    }

    if(tbs_nfp_bridge_rule_delete(sa, da, dev->ifindex) != TBS_NFP_OK)
    {
        TBS_NFP_ERROR("tbs_nfp_if_delete(%s) fail\n", ifname);
    }

    if(dev)
        dev_put(dev);

    return len;
}


static inline int tbs_nfp_proc_bridge_size_set( struct file *filp, const char __user *buf,
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

        g_bridge_max_size = max_size;
	}
	else{
		TBS_NFP_ERROR("str_buf is NULL\n");
	}

	return len;
}


static inline int tbs_nfp_proc_bridge_size_get(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    printk("bridge rule max size: %u\n", g_bridge_max_size);

    return 0;
}


/*=========================================================================
 Function:		int tbs_nfp_proc_bridge_init(void)
 Description:		bridge proc 调试接口初始化
 Data Accessed:
 Data Updated:
 Input:			    无
 Output:			无
 Return:			0: 成功
 Others:
=========================================================================*/
int tbs_nfp_proc_bridge_init(void)
{
    struct proc_dir_entry *proc_entry = NULL;

    g_bridge_proc_dir = proc_mkdir(TBS_NFP_BRIDGE_DIR, g_tbs_nfp_proc_dir);
    if(NULL == g_bridge_proc_dir)
    {
        TBS_NFP_ERROR("proc_mkdir(%s) fail\n", TBS_NFP_BRIDGE_DIR);
        goto err;
    }

    proc_entry = create_proc_entry(TBS_NFP_HELP, 0444, g_bridge_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_DIR"/"TBS_NFP_BRIDGE_DIR"/"TBS_NFP_HELP);
        goto err;
    }
    proc_entry->read_proc = &tbs_nfp_proc_bridge_help;


    proc_entry = create_proc_entry(TBS_NFP_RULE_DUMP, 0444, g_bridge_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_BRIDGE_DIR"/"TBS_NFP_RULE_DUMP);
        goto err;
    }
    proc_entry->read_proc = &tbs_nfp_proc_bridge_dump;


    proc_entry = create_proc_entry(TBS_NFP_RULE_ADD, 0222, g_bridge_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_BRIDGE_DIR"/"TBS_NFP_RULE_ADD);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_bridge_add;


    proc_entry = create_proc_entry(TBS_NFP_RULE_DELETE, 0222, g_bridge_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_BRIDGE_DIR"/"TBS_NFP_RULE_DELETE);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_bridge_delete;

    proc_entry = create_proc_entry(TBS_NFP_RULE_RESET, 0222, g_bridge_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_BRIDGE_DIR"/"TBS_NFP_RULE_RESET);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_bridge_reset;

    proc_entry = create_proc_entry(TBS_NFP_RULE_SIZE, 0666, g_bridge_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_FIB_DIR"/"TBS_NFP_RULE_SIZE);
        goto err;
    }
    proc_entry->write_proc = &tbs_nfp_proc_bridge_size_set;
    proc_entry->read_proc = &tbs_nfp_proc_bridge_size_get;

    return TBS_NFP_OK;

err:

    if(g_bridge_proc_dir)
    {
        /*移除bridge 目录下节点*/
        tbs_nfp_proc_remove_entry(g_common_proc, g_bridge_proc_dir);

        remove_proc_entry(TBS_NFP_BRIDGE_DIR, g_tbs_nfp_proc_dir);
    }

    g_bridge_proc_dir = NULL;

    return TBS_NFP_ERR;
}


/*=========================================================================
 Function:		int tbs_nfp_proc_bridge_exit(void)
 Description:		bridge proc 调试接口退出
 Data Accessed:
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_proc_bridge_exit(void)
{
    if(g_bridge_proc_dir)
    {
        /*移除bridge目录下节点*/
        tbs_nfp_proc_remove_entry(g_common_proc, g_bridge_proc_dir);

        remove_proc_entry(TBS_NFP_BRIDGE_DIR, g_tbs_nfp_proc_dir);
    }

    g_bridge_proc_dir = NULL;
}

