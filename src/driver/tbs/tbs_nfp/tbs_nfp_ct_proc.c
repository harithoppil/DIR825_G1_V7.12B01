/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_ct_proc.c
* 文件描述 : tbs加速器conntrack rule proc配置调试接口
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
extern unsigned int g_ct_max_size;
static struct proc_dir_entry *g_ct_proc_dir = NULL;



/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:		tatic int tbs_nfp_proc_ct_help(char *buf, char **start, off_t offset,
                    int count, int *eof, void *data)
 Description:		显示ct proc help信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_ct_help(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    //char *out = buf;
    int len = 0;

    printk("Local commands:\n");
    printk("cat                                                                                     help           - print this help\n");
    printk("cat                                                                                     dump           - print all ct rule\n");
    printk("echo family proto sip dip sport dport reply_sip reply_dip reply_sport reply_dport       > rule_add       - add ct rule\n");
    printk("echo family sip dip sport dport proto                                                   > rule_delete    - delete ct rule\n");
    printk("echo MAX_SIZE                                                                           > max_size       - set max size\n");
    printk("echo 1                                                                                  > reset          - clean all ct\n");

    return len;
}


/*=========================================================================
 Function:		tatic int tbs_nfp_proc_ct_dump(char *buf, char **start, off_t offset,
                    int count, int *eof, void *data)
 Description:		显示conntrack proc help信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_ct_dump(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	tbs_nfp_ct_dump();
    return 0;
}

/*
 * TBS_TAG: add by baiyonghui 2011-12-28
 * Description:
 */

#define IPV6_ADDR_LENGTH 64
#define IPV4_ADDR_LENGTH 16
/*=========================================================================
 Function:		static int tbs_nfp_proc_ct_add( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		添加interface
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_ct_add( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
	int ret = 0;
    char str_buf[256] = {0};
	int family;
	unsigned char sip[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned char dip[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned char sip_str[IPV6_ADDR_LENGTH];
	unsigned char dip_str[IPV6_ADDR_LENGTH];
	unsigned short sport, dport;
	unsigned short proto;
	unsigned char reply_sip[TBS_NFP_MAX_IPV4_ADDR_SIZE];
	unsigned char reply_dip[TBS_NFP_MAX_IPV4_ADDR_SIZE];
	unsigned short reply_sport;
	unsigned short reply_dport;
	unsigned char reply_sip_str[IPV4_ADDR_LENGTH];
	unsigned char reply_dip_str[IPV4_ADDR_LENGTH];

	memset(reply_sip_str, 0, sizeof(reply_sip_str));
	memset(reply_dip_str, 0, sizeof(reply_dip_str));

	memset(sip_str, 0, sizeof(sip_str));
	memset(dip_str, 0, sizeof(dip_str));
	memset(sip, 0, sizeof(sip));
	memset(dip, 0, sizeof(dip));

	/* 限定一条输入长度不大于64 */
	if(len > (sizeof(str_buf) - 1))
	{
	    //TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
        TBS_NFP_ERROR("Usage: family(2/11) proto(6/17) sip dip sport dport rep_sip rep_dip rep_sport rep_dport\n");

        return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
		str_buf[len]='\0';

		ret = sscanf(str_buf, "%u %hu %s %s %hu %hu %s %s %hu %hu",
				&family, &proto, sip_str, dip_str, &sport, &dport,
				reply_sip_str, reply_dip_str, &reply_sport, &reply_dport);

		if(ret != 10)
		{
			//TBS_NFP_ERROR("Write_proc: Need 10 parameters to write in\n");
            TBS_NFP_ERROR("Usage: family(2/11) proto(6/17) sip dip sport dport rep_sip rep_dip rep_sport rep_dport\n");

			return len;
		}

		sip_str[IPV6_ADDR_LENGTH - 1] = '\0';
		dip_str[IPV6_ADDR_LENGTH - 1] = '\0';

		reply_sip_str[IPV4_ADDR_LENGTH - 1] = '\0';
		reply_dip_str[IPV4_ADDR_LENGTH - 1] = '\0';
	}
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }


	if(AF_INET == family)
	{
		if(true!= in4_pton(sip_str, strlen(sip_str), (u8 *)sip, '\0', NULL))
		{
			TBS_NFP_ERROR("Write_proc: invalid ipv4 src ip addr!\n");
			return len;
		}

		if(true!= in4_pton(dip_str, strlen(dip_str), (u8 *)dip, '\0', NULL))
		{
			TBS_NFP_ERROR("Write_proc: invalid ipv4 dst ip addr!\n");
			return len;
		}
	}
	else if(AF_INET6 == family)
	{
		if(true!= in6_pton(sip_str, strlen(sip_str), (u8 *)sip, '\0', NULL))
		{
			TBS_NFP_ERROR("Write_proc: invalid ipv6 src ip addr!\n");
			return len;
		}

		if(true!= in6_pton(dip_str, strlen(dip_str), (u8 *)dip, '\0', NULL))
		{
			TBS_NFP_ERROR("Write_proc: invalid ipv6 dst ip addr!\n");
			return len;
		}
	}
	else
	{
		TBS_NFP_ERROR("Write_proc: invalid family addr!\n");
		return len;
	}

	if(true!= in4_pton(reply_sip_str, strlen(reply_sip_str), (u8 *)reply_sip, '\0', NULL))
	{
		TBS_NFP_ERROR("Write_proc: invalid ipv4 src ip addr!\n");
		return len;
	}

	if(true!= in4_pton(reply_dip_str, strlen(reply_dip_str), (u8 *)reply_dip, '\0', NULL))
	{
		TBS_NFP_ERROR("Write_proc: invalid ipv4 dst ip addr!\n");
		return len;
	}

	ret = tbs_nfp_ct_add(family, sip, dip, htons(sport), htons(dport), proto,
			reply_sip, reply_dip, htons(reply_sport), htons(reply_dport));
	if(TBS_NFP_OK != ret)
	{
		TBS_NFP_ERROR("tbs_nfp_ct_rule_add fail \n");
	}

    return len;
}


/*=========================================================================
 Function:		static int tbs_nfp_proc_ct_delete( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
 Description:		添加interface
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_ct_delete( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
	int ret = 0;
    char str_buf[256] = {0};
	int family;
	unsigned char sip[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned char dip[TBS_NFP_MAX_L3_ADDR_SIZE];
	unsigned char sip_str[IPV6_ADDR_LENGTH];
	unsigned char dip_str[IPV6_ADDR_LENGTH];
	unsigned short sport, dport;
	unsigned short proto;

	memset(sip_str, 0, sizeof(sip_str));
	memset(dip_str, 0, sizeof(dip_str));
	memset(sip, 0, sizeof(sip));
	memset(dip, 0, sizeof(dip));

	/* 限定一条输入长度不大于64 */
	if(len > (sizeof(str_buf) - 1))
	{
	    //TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
        TBS_NFP_ERROR("Usage: family(2/11) proto(6/17) sip dip sport dport\n");

	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
		str_buf[len]='\0';

		ret = sscanf(str_buf, "%u %hu %s %s %hu %hu",
				&family, &proto, sip_str, dip_str, &sport, &dport);

		if(ret != 6)
		{
			//TBS_NFP_ERROR("Write_proc: Need 6 parameters to write in\n");
            TBS_NFP_ERROR("Usage: family(2/11) proto(6/17) sip dip sport dport\n");

			return len;
		}

		sip_str[IPV6_ADDR_LENGTH - 1] = '\0';
		dip_str[IPV6_ADDR_LENGTH - 1] = '\0';
	}
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }


	if(AF_INET == family)
	{
		if(true!= in4_pton(sip_str, strlen(sip_str), (u8 *)sip, '\0', NULL))
		{
			TBS_NFP_ERROR("Write_proc: invalid ipv4 src ip addr!\n");
			return len;
		}

		if(true!= in4_pton(dip_str, strlen(dip_str), (u8 *)dip, '\0', NULL))
		{
			TBS_NFP_ERROR("Write_proc: invalid ipv4 dst ip addr!\n");
			return len;
		}
	}
	else if(AF_INET6 == family)
	{
		if(true!= in4_pton(sip_str, strlen(sip_str), (u8 *)sip, '\0', NULL))
		{
			TBS_NFP_ERROR("Write_proc: invalid ipv6 src ip addr!\n");
			return len;
		}

		if(true!= in4_pton(dip_str, strlen(dip_str), (u8 *)dip, '\0', NULL))
		{
			TBS_NFP_ERROR("Write_proc: invalid ipv6 dst ip addr!\n");
			return len;
		}
	}
	else
	{
		TBS_NFP_ERROR("Write_proc: invalid family addr!\n");
		return len;
	}

    if(tbs_nfp_ct_delete(family, sip, dip, htons(sport), htons(dport), proto) != TBS_NFP_OK)
    {
        TBS_NFP_ERROR("tbs_nfp_ct_delete fail\n");
    }

    return len;
}
#undef IPV6_ADDR_LENGTH
/*
 * TBS_END_TAG
 */


/*=========================================================================
 Function:		static int tbs_nfp_proc_ct_reset( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		清除conntrack rule
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_ct_reset( struct file *filp, const char __user *buf,
		unsigned long len, void *data )
{
	char str_buf[16] = {0};

	/* 限定一条输入长度不大于16 */
	if(len > (sizeof(str_buf) - 1 ))
	{
		//TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
		TBS_NFP_ERROR("Usage: 0/1");

		return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
		str_buf[len]='\0';

		if(str_buf[0] != '0')
		{
			tbs_nfp_ct_reset();
		}
	}
	else{
		TBS_NFP_ERROR("str_buf is NULL\n");
	}

	return len;
}


static inline int tbs_nfp_proc_ct_size_set( struct file *filp, const char __user *buf,
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

        g_ct_max_size = max_size;
	}
	else{
		TBS_NFP_ERROR("str_buf is NULL\n");
	}

	return len;
}


static inline int tbs_nfp_proc_ct_size_get(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    printk("ct rule max size: %u\n", g_ct_max_size);

    return len;
}

/*=========================================================================
 Function:		int tbs_nfp_proc_ct_init(void)
 Description:		conntrack proc 调试接口初始化
 Data Accessed:
 Data Updated:
 Input:			    无
 Output:			无
 Return:			0: 成功
 Others:
=========================================================================*/
int tbs_nfp_proc_ct_init(void)
{
    struct proc_dir_entry *proc_entry = NULL;

    g_ct_proc_dir = proc_mkdir(TBS_NFP_CT_DIR, g_tbs_nfp_proc_dir);
    if(NULL == g_ct_proc_dir)
    {
        TBS_NFP_ERROR("proc_mkdir(%s) fail\n", TBS_NFP_CT_DIR);
        goto err;
    }

    proc_entry = create_proc_entry(TBS_NFP_HELP, 0444, g_ct_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_DIR"/"TBS_NFP_CT_DIR"/"TBS_NFP_HELP);
        goto err;
    }
    proc_entry->read_proc = &tbs_nfp_proc_ct_help;


    proc_entry = create_proc_entry(TBS_NFP_RULE_DUMP, 0444, g_ct_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_CT_DIR"/"TBS_NFP_RULE_DUMP);
        goto err;
    }
    proc_entry->read_proc = &tbs_nfp_proc_ct_dump;

	proc_entry = create_proc_entry(TBS_NFP_RULE_ADD, 0222, g_ct_proc_dir);
	if(NULL == proc_entry)
	{
		TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_CT_DIR"/"TBS_NFP_RULE_ADD);
		goto err;
	}
	proc_entry->write_proc= &tbs_nfp_proc_ct_add;


	proc_entry = create_proc_entry(TBS_NFP_RULE_DELETE, 0222, g_ct_proc_dir);
	if(NULL == proc_entry)
	{
		TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_CT_DIR"/"TBS_NFP_RULE_DELETE);
		goto err;
	}
	proc_entry->write_proc= &tbs_nfp_proc_ct_delete;

	proc_entry = create_proc_entry(TBS_NFP_RULE_RESET, 0222, g_ct_proc_dir);
	if(NULL == proc_entry)
	{
		TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_CT_DIR"/"TBS_NFP_RULE_RESET);
		goto err;
	}
	proc_entry->write_proc= &tbs_nfp_proc_ct_reset;

	proc_entry = create_proc_entry(TBS_NFP_RULE_SIZE, 0666, g_ct_proc_dir);
	if(NULL == proc_entry)
	{
		TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_CT_DIR"/"TBS_NFP_RULE_SIZE);
		goto err;
	}
	proc_entry->write_proc = &tbs_nfp_proc_ct_size_set;
    proc_entry->read_proc = &tbs_nfp_proc_ct_size_get;

    return TBS_NFP_OK;

err:
    if(g_ct_proc_dir)
    {
        /*移除ct 目录下节点*/
        tbs_nfp_proc_remove_entry(g_common_proc, g_ct_proc_dir);

        remove_proc_entry(TBS_NFP_CT_DIR, g_tbs_nfp_proc_dir);
    }

    g_ct_proc_dir = NULL;

    return TBS_NFP_ERR;
}


/*=========================================================================
 Function:		int tbs_nfp_proc_ct_exit(void)
 Description:		conntrack proc 调试接口退出
 Data Accessed:
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_proc_ct_exit(void)
{
    if(g_ct_proc_dir)
    {
        /*移除ct目录下节点*/
        tbs_nfp_proc_remove_entry(g_common_proc, g_ct_proc_dir);

        remove_proc_entry(TBS_NFP_CT_DIR, g_tbs_nfp_proc_dir);
    }

    g_ct_proc_dir = NULL;
}

