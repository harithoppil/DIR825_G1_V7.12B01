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
#include <linux/inet.h>

/******************************************************************************
 *                                 DEFINE                                     *
 ******************************************************************************/



/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
static struct proc_dir_entry *g_fib_proc_dir = NULL;
extern unsigned int g_fib_max_size;



/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:		tatic int tbs_nfp_proc_fib_help(char *buf, char **start, off_t offset,
                    int count, int *eof, void *data)
 Description:		显示fib proc help信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_fib_help(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    printk("Local commands:\n");
    printk("cat                                     help           - print this help\n");
    printk("cat                                     dump           - print all fib rule\n");
    printk("echo family srcip dstip gtw   oif_name  >rule_add      - add fib rule\n");
    printk("echo family srcip dstip gtw   oif_name  >rule_delete   - delete fib rule\n");
    printk("echo MAX_SIZE                           > max_size       - set max size\n");
    printk("echo 1                                  > reset        - clean all fib\n");
    return len;
}


/*=========================================================================
 Function:		tatic int tbs_nfp_proc_fib_dump(char *buf, char **start, off_t offset,
                    int count, int *eof, void *data)
 Description:		显示fib proc help信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_fib_dump(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    tbs_nfp_fib_dump();
    return 0;
}


/*=========================================================================
 Function:		static int tbs_nfp_proc_fib_add(struct file *filp, const char __user *buf,
                    unsigned long len, void *data)

 Description:		proc 添加fib规则,
                    使用方法:echo family sip dip gtw oifname >fib_add
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
static int tbs_nfp_proc_fib_add(struct file *filp, const char __user *buf,
                    unsigned long len, void *data)
{
	int ret = 0;
    char str_buf[256] = {0};

  	unsigned char inet_family[16] = {0};
	unsigned char sip_str[INET6_ADDRSTRLEN] = {0};
    unsigned char dip_str[INET6_ADDRSTRLEN] = {0};
    unsigned char gtw_str[INET6_ADDRSTRLEN] = {0};
    char ofname[IFNAMSIZ] = {0};

    int family = AF_INET;
	unsigned char sip[TBS_NFP_MAX_L3_ADDR_SIZE] = {0};
    unsigned char dip[TBS_NFP_MAX_L3_ADDR_SIZE] = {0};
    unsigned char gtw[TBS_NFP_MAX_L3_ADDR_SIZE] = {0};
    struct net_device *odev = NULL;

	/* 限定一条输入长度不大于256 */
	if(len > (sizeof(str_buf) - 1))
	{
	    //TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
        TBS_NFP_ERROR("Usage: family(2/11) src_ip dst_ip gtw_ip oif_name\n");

	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

		ret = sscanf(str_buf, "%s %s %s %s %s", \
                    inet_family, sip_str, dip_str,\
                    gtw_str, ofname);

        if(ret != 5)
		{
            //TBS_NFP_ERROR("Write_proc:family sip dip gtw oifname\n");
            TBS_NFP_ERROR("Usage: family(2/11) src_ip dst_ip gtw_ip oif_name\n");

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
            TBS_NFP_ERROR("family: AF_INET or AF_INET6\n");
            return len;
        }


        if(AF_INET == family)
        {
    		if(true!= in4_pton(sip_str, strlen(sip_str),(u8 *)sip, '\0',NULL))
    		{
    			TBS_NFP_ERROR("Write_proc: invalid srcip addr!\n");
    			return len;
    		}

    		if(true!= in4_pton(dip_str, strlen(dip_str),(u8 *)dip, '\0',NULL))
    		{
    			TBS_NFP_ERROR("Write_proc: invalid srcip addr!\n");
    			return len;
    		}

    		if(true!= in4_pton(gtw_str, strlen(gtw_str),(u8 *)gtw, '\0',NULL))
    		{
    			TBS_NFP_ERROR("Write_proc: invalid srcip addr!\n");
    			return len;
    		}
        }
        else
        {
             /*ipv6 form convert*/
        }

        ofname[IFNAMSIZ - 1] = '\0';
    }
    else
    {
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }

    odev = dev_get_by_name(&init_net, ofname);
    if(NULL == odev)
    {
        TBS_NFP_ERROR("interface %s no exist\n", ofname);
		return len;
    }

	if (TBS_NFP_OK != tbs_nfp_fib_add(family, (const unsigned char *)sip,
                (const unsigned char *)dip, (const unsigned char *)gtw, odev->ifindex))
	{
		TBS_NFP_ERROR("tbs_nfp_fib_rule_addfail \n");
	}

    if(odev)
        dev_put(odev);
    return len;

}


/*=========================================================================
 Function:		static int tbs_nfp_proc_fib_del(struct file *filp, const char __user *buf,
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

static int tbs_nfp_proc_fib_del(struct file *filp, const char __user *buf,
                    unsigned long len, void *data)
{
	int ret = 0;
    char str_buf[256] = {0};

  	unsigned char inet_family[16] = {0};
	unsigned char sip_str[INET6_ADDRSTRLEN] = {0};
    unsigned char dip_str[INET6_ADDRSTRLEN] = {0};
    unsigned char gtw_str[INET6_ADDRSTRLEN] = {0};
    char ofname[IFNAMSIZ] = {0};

    int family = AF_INET;
	unsigned char sip[TBS_NFP_MAX_L3_ADDR_SIZE] = {0};
    unsigned char dip[TBS_NFP_MAX_L3_ADDR_SIZE] = {0};
    unsigned char gtw[TBS_NFP_MAX_L3_ADDR_SIZE] = {0};
    struct net_device *odev = NULL;

	/* 限定一条输入长度不大于256 */
	if(len > (sizeof(str_buf) - 1))
	{
	    //TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
        TBS_NFP_ERROR("Usage: family(2/11) src_ip dst_ip gtw_ip oif_name\n");

	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

		ret = sscanf(str_buf, "%s %s %s %s %s", \
                    inet_family, sip_str, dip_str,\
                    gtw_str, ofname);

        if(ret != 5)
		{
            //TBS_NFP_ERROR("Write_proc:family sip dip gtw oifname\n");
            TBS_NFP_ERROR("Usage: family(2/11) src_ip dst_ip gtw_ip oif_name\n");

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
            TBS_NFP_ERROR("family: AF_INET or AF_INET6\n");
            return len;
        }


        if(AF_INET == family)
        {
    		if(true!= in4_pton(sip_str, strlen(sip_str),(u8 *)sip, '\0',NULL))
    		{
    			TBS_NFP_ERROR("Write_proc: invalid srcip addr!\n");
    			return len;
    		}

    		if(true!= in4_pton(dip_str, strlen(dip_str),(u8 *)dip, '\0',NULL))
    		{
    			TBS_NFP_ERROR("Write_proc: invalid srcip addr!\n");
    			return len;
    		}

    		if(true!= in4_pton(gtw_str, strlen(gtw_str),(u8 *)gtw, '\0',NULL))
    		{
    			TBS_NFP_ERROR("Write_proc: invalid srcip addr!\n");
    			return len;
    		}
        }
        else
        {
             /*ipv6 form convert*/
        }

        ofname[IFNAMSIZ - 1] = '\0';
    }
    else
    {
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }

    odev = dev_get_by_name(&init_net, ofname);
    if(NULL == odev)
    {
        TBS_NFP_ERROR("interface %s no exist\n", ofname);
		return len;
    }

	if (TBS_NFP_OK != tbs_nfp_fib_delete(family, (const unsigned char *)sip,
                        (const unsigned char *)dip))
	{
		TBS_NFP_ERROR("tbs_nfp_fib_rule_addfail \n");
	}

    if(odev)
        dev_put(odev);
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
static int tbs_nfp_proc_fib_reset( struct file *filp, const char __user *buf,
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
            tbs_nfp_fib_reset();
        }
    }
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
    }

    return len;
}



static inline int tbs_nfp_proc_fib_size_set( struct file *filp, const char __user *buf,
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

        g_fib_max_size = max_size;
	}
	else{
		TBS_NFP_ERROR("str_buf is NULL\n");
	}

	return len;
}


static inline int tbs_nfp_proc_fib_size_get(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    printk("fib rule max size: %u\n", g_fib_max_size);

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
int tbs_nfp_proc_fib_init(void)
{
    struct proc_dir_entry *proc_entry = NULL;

    g_fib_proc_dir = proc_mkdir(TBS_NFP_FIB_DIR, g_tbs_nfp_proc_dir);
    if(NULL == g_fib_proc_dir)
    {
        TBS_NFP_ERROR("proc_mkdir(%s) fail\n", TBS_NFP_FIB_DIR);
        goto err;
    }

    proc_entry = create_proc_entry(TBS_NFP_HELP, 0444, g_fib_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_DIR"/"TBS_NFP_FIB_DIR"/"TBS_NFP_HELP);
        goto err;
    }
    proc_entry->read_proc = &tbs_nfp_proc_fib_help;


    proc_entry = create_proc_entry(TBS_NFP_RULE_DUMP, 0444, g_fib_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_FIB_DIR"/"TBS_NFP_RULE_DUMP);
        goto err;
    }
    proc_entry->read_proc = &tbs_nfp_proc_fib_dump;


    proc_entry = create_proc_entry(TBS_NFP_RULE_ADD, 0222, g_fib_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_FIB_DIR"/"TBS_NFP_RULE_ADD);
        goto err;
    }
    proc_entry->write_proc = &tbs_nfp_proc_fib_add;


    proc_entry = create_proc_entry(TBS_NFP_RULE_DELETE, 0222, g_fib_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_FIB_DIR"/"TBS_NFP_RULE_DELETE);
        goto err;
    }
    proc_entry->write_proc = &tbs_nfp_proc_fib_del;

    proc_entry = create_proc_entry(TBS_NFP_RULE_RESET, 0222, g_fib_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_FIB_DIR"/"TBS_NFP_RULE_RESET);
        goto err;
    }
    proc_entry->write_proc = &tbs_nfp_proc_fib_reset;

    proc_entry = create_proc_entry(TBS_NFP_RULE_SIZE, 0666, g_fib_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_FIB_DIR"/"TBS_NFP_RULE_SIZE);
        goto err;
    }
    proc_entry->write_proc = &tbs_nfp_proc_fib_size_set;
    proc_entry->read_proc = &tbs_nfp_proc_fib_size_get;

    return TBS_NFP_OK;

err:

    if(g_fib_proc_dir)
    {
        /*移除fib 目录下节点*/
        tbs_nfp_proc_remove_entry(g_common_proc, g_fib_proc_dir);

        remove_proc_entry(TBS_NFP_FIB_DIR, g_tbs_nfp_proc_dir);
    }

    g_fib_proc_dir = NULL;

    return TBS_NFP_ERR;
}


/*=========================================================================
 Function:		int tbs_nfp_proc_fib_exit(void)
 Description:		fib proc 调试接口退出
 Data Accessed:
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_proc_fib_exit(void)
{
    if(g_fib_proc_dir)
    {
        /*移除fib目录下节点*/
        tbs_nfp_proc_remove_entry(g_common_proc, g_fib_proc_dir);

        remove_proc_entry(TBS_NFP_FIB_DIR, g_tbs_nfp_proc_dir);
    }

    g_fib_proc_dir = NULL;
}

