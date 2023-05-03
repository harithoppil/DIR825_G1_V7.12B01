/*******************************************************************************
* Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
*
* 文件名称 : tbs_nfp_itf_proc.c
* 文件描述 : tbs加速器interface proc配置调试接口
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

#define TBS_NFP_IF_DUMP             "dump"
#define TBS_NFP_IF_ADD              "if_add"
#define TBS_NFP_IF_DELETE           "if_delete"
#define TBS_NFP_IF_BRPORT_ADD       "brport_add"
#define TBS_NFP_IF_BRPORT_REMOVE    "brport_remove"
#define TBS_NFP_IF_BIND             "dummyport_bind"
#define TBS_NFP_IF_UNBIND           "dummyport_unbind"
#define TBS_NFP_IF_CLEAN            "clean"


/******************************************************************************
 *                                 GLOBAL                                     *
 ******************************************************************************/
static struct proc_dir_entry *g_itf_proc_dir = NULL;

const static char* g_itf_proc[] ={
    TBS_NFP_HELP,
    TBS_NFP_IF_DUMP,
    TBS_NFP_IF_ADD,
    TBS_NFP_IF_DELETE,
    TBS_NFP_IF_BRPORT_ADD,
    TBS_NFP_IF_BRPORT_REMOVE,
    TBS_NFP_IF_BIND,
    TBS_NFP_IF_UNBIND,
    TBS_NFP_IF_CLEAN,
    NULL
    };

/******************************************************************************
 *                                FUNCTIONS                                   *
 ******************************************************************************/

/*=========================================================================
 Function:		tatic int tbs_nfp_proc_itf_help(char *buf, char **start, off_t offset,
                    int count, int *eof, void *data)
 Description:		显示interface proc help信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_itf_help(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    printk("Local commands:\n");
    printk("cat                                             help           - print this help\n");
    printk("cat                                             dump           - print all interface\n");

    printk("\ninterface commands:\n");
    printk("echo [phys | wlan | bridge | vlan | pppoe | dummyport] ifname  > if_add        - add interface\n");
    printk("echo ifname                                   > if_delete      - delete interface\n");
    printk("echo brname br_port                           > brport_add     - add bridge port\n");
    printk("echo br_port                                  > brport_remove  - remove bridge port\n");
    printk("echo dummyport phys_if                        > dummyport_bind - dummyport bind phys interrace\n");
    printk("echo dummyport                                > dummyport_unbind - dummyport unbind phys interrace\n");
    printk("echo 1                                        > clean          - clean all interface\n");
    printk("echo ifname [up | down ]                      > status         - set interface status\n");

    return len;
}


/*=========================================================================
 Function:		tatic int tbs_nfp_proc_itf_dump(char *buf, char **start, off_t offset,
                    int count, int *eof, void *data)
 Description:		显示interface proc help信息
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_itf_dump(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    printk("Interface dump:\n");
    tbs_nfp_if_dump();

    return len;
}


/*=========================================================================
 Function:		static int tbs_nfp_proc_itf_add( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		添加interface
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_itf_add( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
	int ret = 0;
    char str_buf[36] = {0};
    char ifname[IFNAMSIZ] = {0};
    char iftype[16] = {0};
    struct net_device *dev = NULL;

#ifdef TBS_NFP_VLAN
    struct vlan_dev_info *vdi = NULL;
#endif /*TBS_NFP_VLAN*/

	/* 限定一条输入长度不大于32 */
	if(len > 32)
	{
	    TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

        ret = sscanf(str_buf, "%s %s ", iftype, ifname);
        if(ret != 2)
		{
            TBS_NFP_ERROR("Write_proc: Need 2 parameters to write in\n");
            return len;
        }

        ifname[IFNAMSIZ - 1] = '\0';
        iftype[15] = '\0';
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

    if(!strncmp(iftype, "phys", sizeof(iftype)))
    {
        if(!(dev->priv_flags & IFF_ETH))
        {
            TBS_NFP_ERROR("interface %s not phys device\n", ifname);
            goto out;
        }

        if(tbs_nfp_if_phys_add(dev->ifindex, dev->mtu,
            dev->dev_addr, dev->name) != TBS_NFP_OK)
        {
            TBS_NFP_ERROR("tbs_nfp_if_phys_add(%s) fail \n", ifname);
            goto out;
        }
    }
#ifdef TBS_NFP_WLAN
    else if(!strncmp(iftype, "wlan", sizeof(iftype)))
    {
        if(!(dev->priv_flags & IFF_WLAN))
        {
            TBS_NFP_ERROR("interface %s not wlan device\n", ifname);
            goto out;
        }

        if(tbs_nfp_if_wlan_add(dev->ifindex, dev->mtu,
            dev->dev_addr, dev->name) != TBS_NFP_OK)
        {
            TBS_NFP_ERROR("tbs_nfp_if_wlan_add(%s) fail \n", ifname);
            goto out;
        }
    }
#endif /*TBS_NFP_WLAN*/

#ifdef TBS_NFP_BRIDGE
    else if(!strncmp(iftype, "bridge", sizeof(iftype)))
    {
        if(!(dev->priv_flags & IFF_EBRIDGE))
        {
            TBS_NFP_ERROR("interface %s not bridge device\n", ifname);
            goto out;
        }

        if(tbs_nfp_if_bridge_add(dev->ifindex, dev->mtu,
            dev->dev_addr, dev->name) != TBS_NFP_OK)
        {
            TBS_NFP_ERROR("tbs_nfp_if_bridge_add(%s) fail \n", ifname);
            goto out;
        }
    }
#endif  /*TBS_NFP_BRIDGE*/

#ifdef TBS_NFP_VLAN
    else if(!strncmp(iftype, "vlan", sizeof(iftype)))
    {
        if(!(dev->priv_flags & IFF_802_1Q_VLAN))
        {
            TBS_NFP_ERROR("interface %s not vlan device\n", ifname);
            goto out;
        }

        vdi =netdev_priv(dev);
        vdi->vlan_id = 1;
        if(tbs_nfp_if_vlan_add(dev->ifindex, dev->mtu, vdi->real_dev->ifindex,
            vdi->vlan_id, dev->dev_addr, dev->name) != TBS_NFP_OK)
        {
            TBS_NFP_ERROR("tbs_nfp_if_vlan_add(%s) fail \n", ifname);
            goto out;
        }
    }
#endif /*TBS_NFP_VLAN*/

#ifdef TBS_NFP_PPP
    else if(!strncmp(iftype, "pppoe", sizeof(iftype)))
    {
        if(!(dev->flags & IFF_POINTOPOINT))
        {
            TBS_NFP_ERROR("interface %s not pppoe device\n", ifname);
            goto out;
        }

        /*添加pppoe接口*/
    }
#endif /*TBS_NFP_PPP*/

#ifdef TBS_NFP_DUMMYPORT
    else if(!strncmp(iftype, "dummyport", sizeof(iftype)))
    {
        if(!(dev->priv_flags & IFF_DUMMYPORT))
        {
            TBS_NFP_ERROR("interface %s not dummyport device\n", ifname);
            goto out;
        }

        /*添加dummyport接口*/
    }
#endif /*TBS_NFP_DUMMYPORT*/

    else
    {
        TBS_NFP_ERROR("unknown type interfac %s\n", iftype);
    }

out:
    if(dev)
        dev_put(dev);
    return len;
}


/*=========================================================================
 Function:		static int tbs_nfp_proc_itf_delete( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
 Description:		添加interface
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_itf_delete( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
	int ret = 0;
    char str_buf[16] = {0};
    char ifname[IFNAMSIZ] = {0};
    struct net_device *dev = NULL;

	/* 限定一条输入长度不大于16 */
	if(len > (sizeof(str_buf) - 1))
	{
	    TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

        ret = sscanf(str_buf, "%s", ifname);
        if(ret != 1)
		{
            TBS_NFP_ERROR("Write_proc: Need 1 parameters to write in\n");
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

    if(tbs_nfp_if_delete(dev->ifindex) != TBS_NFP_OK)
    {
        TBS_NFP_ERROR("tbs_nfp_if_delete(%s) fail\n", ifname);
        goto out;
    }

out:
    if(dev)
        dev_put(dev);

    return len;
}


#ifdef TBS_NFP_BRIDGE
/*=========================================================================
 Function:		static int tbs_nfp_proc_itf_brport_add( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		添加接口到bridge中
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_itf_brport_add( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
	int ret = 0;
    char str_buf[36] = {0};
    char br_name[IFNAMSIZ] = {0};
    char brport_name[IFNAMSIZ] = {0};
    struct net_device *br_dev = NULL;
    struct net_device *brport_dev = NULL;

	/* 限定一条输入长度不大于32 */
	if(len > 32)
	{
	    TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

        ret = sscanf(str_buf, "%s %s ", br_name, brport_name);
        if(ret != 2)
		{
            TBS_NFP_ERROR("Write_proc: Need 2 parameters to write in\n");
            return len;
        }

        br_name[IFNAMSIZ - 1] = '\0';
        brport_name[IFNAMSIZ - 1] = '\0';
    }
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }

    br_dev = dev_get_by_name(&init_net, br_name);
    if(NULL == br_dev)
    {
        TBS_NFP_ERROR("bridge %s no exist\n", br_name);
        return len;
    }

    brport_dev = dev_get_by_name(&init_net, brport_name);
    if(NULL == brport_dev)
    {
        TBS_NFP_ERROR("interface %s no exist\n", brport_name);
        goto out;
    }

    if(tbs_nfp_if_br_port_add(br_dev->ifindex, brport_dev->ifindex)
        != TBS_NFP_OK)
    {
        TBS_NFP_ERROR("tbs_nfp_if_br_port_add(%s, %s) fail \n",
            br_name, brport_name);
        goto out;
    }

out:
    if(br_dev)
        dev_put(br_dev);

    if(brport_dev)
        dev_put(brport_dev);

    return len;
}

/*=========================================================================
 Function:		static int tbs_nfp_proc_itf_brport_remove( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		从桥中移除接口
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_itf_brport_remove( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
	int ret = 0;
    char str_buf[36] = {0};
    char brport_name[IFNAMSIZ] = {0};
    struct net_device *brport_dev = NULL;

	/* 限定一条输入长度不大于32 */
	if(len > 16)
	{
	    TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

        ret = sscanf(str_buf, "%s ", brport_name);
        if(ret != 1)
		{
            TBS_NFP_ERROR("Write_proc: Need 1 parameters to write in\n");
            return len;
        }

        brport_name[IFNAMSIZ - 1] = '\0';
    }
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }

    brport_dev = dev_get_by_name(&init_net, brport_name);
    if(NULL == brport_dev)
    {
        TBS_NFP_ERROR("interface %s no exist\n", brport_name);
        return len;
    }

    if(tbs_nfp_if_br_port_remove(brport_dev->ifindex)
        != TBS_NFP_OK)
    {
        TBS_NFP_ERROR("tbs_nfp_if_br_port_remove(%s) fail \n",
            brport_name);
        goto out;
    }

out:
    if(brport_dev)
        dev_put(brport_dev);

    return len;
}
#endif


#ifdef TBS_NFP_DUMMYPORT
/*=========================================================================
 Function:		static int tbs_nfp_proc_itf_dummyport_bind( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		dummyport绑定接口
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_itf_dummyport_bind( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
	int ret = 0;
    char str_buf[36] = {0};
    char dummyport_name[IFNAMSIZ] = {0};
    char phys_name[IFNAMSIZ] = {0};
    struct net_device *dummyport_dev = NULL;
    struct net_device *phys_dev = NULL;

	/* 限定一条输入长度不大于32 */
	if(len > 32)
	{
	    TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

        ret = sscanf(str_buf, "%s %s ", dummyport_name, phys_name);
        if(ret != 2)
		{
            TBS_NFP_ERROR("Write_proc: Need 2 parameters to write in\n");
            goto out;
        }

        dummyport_name[IFNAMSIZ - 1] = '\0';
        phys_name[IFNAMSIZ - 1] = '\0';
    }
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
        goto out;
    }

    dummyport_dev= dev_get_by_name(&init_net, dummyport_name);
    if(NULL == dummyport_dev)
    {
        TBS_NFP_ERROR("dummyport %s no exist\n", dummyport_name);
        return len;
    }

    phys_dev = dev_get_by_name(&init_net, phys_name);
    if(NULL == phys_dev)
    {
        TBS_NFP_ERROR("interface %s no exist\n", phys_name);
        goto out;
    }

    if(tbs_nfp_if_dummyport_bind(dummyport_dev->ifindex, phys_dev->ifindex)
        != TBS_NFP_OK)
    {
        TBS_NFP_ERROR("tbs_nfp_if_dummyport_bind(%s, %s) fail \n",
            dummyport_name, phys_name);
        goto out;
    }

out:
    if(dummyport_dev)
        dev_put(dummyport_dev);

    if(phys_dev)
        dev_put(phys_dev);

    return len;
}

/*=========================================================================
 Function:		static int tbs_nfp_proc_itf_dummyport_unbind( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		dummyport解除绑定物理接口
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_itf_dummyport_unbind( struct file *filp, const char __user *buf,
    unsigned long len, void *data )
{
	int ret = 0;
    char str_buf[36] = {0};
    char dummyport_name[IFNAMSIZ] = {0};
    struct net_device *dummyport_dev = NULL;

	/* 限定一条输入长度不大于16 */
	if(len > 16)
	{
	    TBS_NFP_ERROR("Write_proc: Parameter length=%lu out of range\n", len);
	    return len;
	}

	if (buf && (!copy_from_user(str_buf, buf, len)))
	{
        str_buf[len]='\0';

        ret = sscanf(str_buf, "%s ", dummyport_name);
        if(ret != 1)
		{
            TBS_NFP_ERROR("Write_proc: Need 1 parameters to write in\n");
            return len;
        }

        dummyport_name[IFNAMSIZ - 1] = '\0';
    }
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }

    dummyport_dev = dev_get_by_name(&init_net, dummyport_name);
    if(NULL == dummyport_dev)
    {
        TBS_NFP_ERROR("bridge %s no exist\n", dummyport_name);
        return len;
    }

    if(tbs_nfp_if_dummyport_unbind(dummyport_dev->ifindex)
        != TBS_NFP_OK)
    {
        TBS_NFP_ERROR("tbs_nfp_if_dummyport_unbind(%s) fail \n",
            dummyport_name);
        goto out;
    }

out:
    if(dummyport_dev)
        dev_put(dummyport_dev);

    return len;
}
#endif


/*=========================================================================
 Function:		static int tbs_nfp_proc_itf_clean( struct file *filp,
    const char __user *buf,unsigned long len, void *data )
 Description:		清除interface所有节点
 Data Accessed:
 Data Updated:
 Input:			无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
static int tbs_nfp_proc_itf_clean( struct file *filp, const char __user *buf,
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
            if(tbs_nfp_if_reset() != TBS_NFP_OK)
            {
                TBS_NFP_ERROR("tbs_nfp_if_flush() fail \n");
                return len;
            }
        }
    }
    else{
        TBS_NFP_ERROR("str_buf is NULL\n");
        return len;
    }

    return len;
}


/*=========================================================================
 Function:		int tbs_nfp_proc_itf_init(void)
 Description:		interface proc 调试接口初始化
 Data Accessed:
 Data Updated:
 Input:			    无
 Output:			无
 Return:			0:成功
 Others:
=========================================================================*/
int tbs_nfp_proc_itf_init(void)
{
    struct proc_dir_entry *proc_entry = NULL;

    g_itf_proc_dir = proc_mkdir(TBS_NFP_IF_DIR, g_tbs_nfp_proc_dir);
    if(NULL == g_itf_proc_dir)
    {
        TBS_NFP_ERROR("proc_mkdir(%s) fail\n", TBS_NFP_IF_DIR);
        goto err;
    }

    proc_entry = create_proc_entry(TBS_NFP_HELP, 0444, g_itf_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_DIR"/"TBS_NFP_IF_DIR"/"TBS_NFP_HELP);
        goto err;
    }
    proc_entry->read_proc = &tbs_nfp_proc_itf_help;


    proc_entry = create_proc_entry(TBS_NFP_IF_DUMP, 0444, g_itf_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_IF_DIR"/"TBS_NFP_IF_DUMP);
        goto err;
    }
    proc_entry->read_proc = &tbs_nfp_proc_itf_dump;


    proc_entry = create_proc_entry(TBS_NFP_IF_ADD, 0222, g_itf_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_IF_DIR"/"TBS_NFP_IF_ADD);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_itf_add;


    proc_entry = create_proc_entry(TBS_NFP_IF_DELETE, 0222, g_itf_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_IF_DIR"/"TBS_NFP_IF_DELETE);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_itf_delete;


#ifdef TBS_NFP_BRIDGE
    proc_entry = create_proc_entry(TBS_NFP_IF_BRPORT_ADD, 0222, g_itf_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_IF_DIR"/"TBS_NFP_IF_BRPORT_ADD);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_itf_brport_add;


    proc_entry = create_proc_entry(TBS_NFP_IF_BRPORT_REMOVE, 0222, g_itf_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_IF_DIR"/"TBS_NFP_IF_BRPORT_REMOVE);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_itf_brport_remove;
#endif /*TBS_NFP_BRIDGE*/

#ifdef TBS_NFP_DUMMYPORT
    proc_entry = create_proc_entry(TBS_NFP_IF_BIND, 0222, g_itf_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_IF_DIR"/"TBS_NFP_IF_BIND);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_itf_dummyport_bind;

    proc_entry = create_proc_entry(TBS_NFP_IF_UNBIND, 0222, g_itf_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_IF_DIR"/"TBS_NFP_IF_UNBIND);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_itf_dummyport_unbind;
#endif /*TBS_NFP_DUMMYPORT*/

    proc_entry = create_proc_entry(TBS_NFP_IF_CLEAN, 0222, g_itf_proc_dir);
    if(NULL == proc_entry)
    {
        TBS_NFP_ERROR("create_proc_entry(%s) fail\n", TBS_NFP_IF_DIR"/"TBS_NFP_IF_CLEAN);
        goto err;
    }
    proc_entry->write_proc= &tbs_nfp_proc_itf_clean;

    return TBS_NFP_OK;

err:

    if(g_itf_proc_dir)
    {
        /*移除interface目录下节点*/
        tbs_nfp_proc_remove_entry(g_itf_proc, g_itf_proc_dir);

        remove_proc_entry(TBS_NFP_IF_DIR, g_tbs_nfp_proc_dir);
    }

    g_itf_proc_dir = NULL;

    return TBS_NFP_ERR;
}


/*=========================================================================
 Function:		int tbs_nfp_proc_itf_init(void)
 Description:		interface proc 调试接口退出
 Data Accessed:
 Data Updated:
 Input:			    无
 Output:			无
 Return:			无
 Others:
=========================================================================*/
void tbs_nfp_proc_itf_exit(void)
{
    if(g_itf_proc_dir)
    {
        /*移除interface目录下节点*/
        tbs_nfp_proc_remove_entry(g_itf_proc, g_itf_proc_dir);

        remove_proc_entry(TBS_NFP_IF_DIR, g_tbs_nfp_proc_dir);
    }

    g_itf_proc_dir = NULL;
}

