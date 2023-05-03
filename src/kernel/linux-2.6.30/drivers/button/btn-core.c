/*
 * �ļ���:btn-class.c
 *
 * ˵��:TBS��ť�������Ĳ���
 *
 * ����:����
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/sysdev.h>
#include <linux/timer.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/tbs_msg.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <btn.h>



#ifdef CONFIG_BUTTON_TRIGGER_TIMER  /* ��ѯ��ʽ */
DEFINE_RWLOCK(btn_list_lock);
LIST_HEAD(btn_list);
#endif

btn_status current_btn_status;
static int btn_query_lock;

rwlock_t  netlink_lock;
static struct sock *netlink_btn_sock;
static int blp_cmm_pid = 0;

#ifdef CONFIG_BUTTON_TRIGGER_TIMER  /* ��ѯ��ʽ */
struct timer_list trigger_timer;
#define TRIGGER_TIMER_DEYLAY		500
#endif

struct timer_list query_timer;
#define QUERY_TIMER_DEYLAY		20



#define BLP_BTN_CMM_SRCMID		0x1
#define BLP_BTN_CMM_DSTMID		0x2
#define BLP_BTN_CMM_MSGID		0x3
#define BLP_BTN_CMM_MSGTYPE_CHANGE	0x4

static void recv_handler(struct sock * sk, int length)
{
	//wake_up(sk->sk_sleep);
}

/* ͨ��netlink��ʽ������Ϣ��Ӧ�ò� */
static void send_to_user(struct btn_dev *btn, btn_status btn_status)
{

	int ret;
	int size;
	ST_MSG *tbs_msg;
	struct btn_msg *button_msg;
	unsigned char *old_tail;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;



	if(blp_cmm_pid == 0)
	{
		printk("blp_cmm's pid = %d.Please run blp_cmm.\n",blp_cmm_pid);
	}


	size = sizeof(ST_MSGHEAD) + sizeof(struct btn_msg);


	/*  size = msg header + data size */

	size = NLMSG_SPACE(size);

	skb = alloc_skb(size, GFP_ATOMIC);
	old_tail = skb->tail;

	/* init msg header */
	nlh = NLMSG_PUT(skb, 0, 0, NETLINK_TBS, size-sizeof(*nlh));

	/* point to msg data area */
	tbs_msg = NLMSG_DATA(nlh);

	/* fill data for sending */
	memset(tbs_msg, 0, size);

	tbs_msg->stMsgHead.usSrcMID = BLP_BTN_CMM_SRCMID;
	tbs_msg->stMsgHead.usDstMID = BLP_BTN_CMM_DSTMID;
	tbs_msg->stMsgHead.ulMsgID  = BLP_BTN_CMM_MSGID;
	tbs_msg->stMsgHead.usMsgType = BLP_BTN_CMM_MSGTYPE_CHANGE;

	button_msg = (struct btn_msg *)tbs_msg->szMsgBody;

	button_msg->name = btn->name;

	if(btn_status == BTN_DOWN)
	{
		button_msg->state = BTN_DOWN;
	}
	else
	{
		button_msg->state = BTN_UP;
	}


	//printk("button_driver: netlink sending msg is : %s \n\r", tbs_msg->szMsgBody);
	/* get netlink msg length */
	nlh->nlmsg_len = skb->tail - old_tail;

	NETLINK_CB(skb).pid = 0;
	NETLINK_CB(skb).dst_group = 0;
	//NETLINK_CB(skb).dst_pid = blp_cmm_pid;  //by ZhangYu,kernel 2.6.21 has no member dst_pid

	/* send msg */
	read_lock_bh(&netlink_lock);
	ret = netlink_unicast(netlink_btn_sock, skb, blp_cmm_pid, MSG_DONTWAIT);
	read_unlock_bh(&netlink_lock);

	printk("message send! ret=%x \n",ret);
	return ;

 nlmsg_failure:
 	printk("Fail to send netlink message.\n");
	if(skb)
		kfree_skb(skb);

	return;

}

#ifdef CONFIG_BUTTON_TRIGGER_TIMER
static void trigger_timer_function(unsigned long data)
{
	struct btn_dev *btn;

	list_for_each_entry(btn,&btn_list,node)
	{
		if( btn->get_status(btn) == BTN_DOWN)
		{
			btn_status_query(btn);  /* ����״̬�� */
		}

		//break;
	}

	trigger_timer.expires  = jiffies + msecs_to_jiffies(QUERY_TIMER_DEYLAY);
	add_timer( &trigger_timer);
}
#endif

/* ״̬�������� */
static void query_timer_function(unsigned long p_btn_cdev)
{
	struct btn_dev *btn = (struct btn_dev *)p_btn_cdev;

	if(current_btn_status == BTN_START)
	{
		if( btn->get_status(btn) == BTN_DOWN)
		{
			current_btn_status = BTN_DOWN;
			send_to_user(btn,BTN_DOWN);
			query_timer.expires  = jiffies + msecs_to_jiffies(QUERY_TIMER_DEYLAY);
			add_timer( &query_timer);
		}
		else  //BTN_UP
		{
			current_btn_status = BTN_UP;
			query_timer.expires  = jiffies + msecs_to_jiffies(QUERY_TIMER_DEYLAY);
			add_timer( &query_timer);
		}

		return;
	}


	if(current_btn_status == BTN_DOWN)
	{
		if(btn->get_status(btn) == BTN_UP)
		{
			current_btn_status = BTN_UP;
			query_timer.expires  = jiffies + msecs_to_jiffies(QUERY_TIMER_DEYLAY);
			add_timer( &query_timer);

		}
		else    //BTN_DOWN
		{
			query_timer.expires  = jiffies + msecs_to_jiffies(QUERY_TIMER_DEYLAY);
			add_timer( &query_timer);
		}


		return;

	}


	if(current_btn_status == BTN_UP)
	{
		if(btn->get_status(btn) == BTN_UP)
		{
			current_btn_status = BTN_START;
			btn_query_lock = 0; /* �ͷ�������ʱ������������ť����״̬�� */
			send_to_user(btn,BTN_UP);
		}
 		else  //BTN_DOWN
		{
			current_btn_status = BTN_DOWN;
			query_timer.expires  = jiffies + msecs_to_jiffies(QUERY_TIMER_DEYLAY);
			add_timer( &query_timer);
		}

		return;
	}


}


/* ״̬�������������ɴ˽���״̬�� */
void btn_status_query(struct btn_dev *btn)
{
	if(btn_query_lock)   /* ����,ÿ��ֻ����һ����ť */
	{
		return;
	}
	btn_query_lock = 1;
	query_timer.expires  = jiffies + msecs_to_jiffies(QUERY_TIMER_DEYLAY);
	query_timer.data = (unsigned long)btn;
	current_btn_status = BTN_START;
	add_timer( &query_timer);
}
EXPORT_SYMBOL_GPL(btn_status_query);



/**
 * ��ťע�ắ������һ����ťע�ᵽ���ĵ�������
 *
 */
int btn_dev_register(struct btn_dev *btn)
{

#ifdef CONFIG_BUTTON_TRIGGER_TIMER
	/* ����ť���뵽���������� */
	write_lock(&btn_list_lock);
	list_add_tail(&btn->node, &btn_list);
	write_unlock(&btn_list_lock);
#endif

	/* ������жϷ�ʽ������Ҫ���κδ��� */
	printk(KERN_INFO "Registered button device: %d\n",btn->name);

	return 0;
}
EXPORT_SYMBOL_GPL(btn_dev_register);


void btn_dev_unregister(struct btn_dev *btn)
{
#ifdef CONFIG_BUTTON_TRIGGER_TIMER
	write_lock(&btn_list_lock);
	list_del(&btn->node);
	write_unlock(&btn_list_lock);
#endif
}
EXPORT_SYMBOL_GPL(btn_dev_unregister);

/**
 *  �� /proc/blp_cmm_pid ���Ӧ�ò�������̵�ID
 */

static int proc_blp_cmm_read(char *buf, char **start, off_t offset, int len, int *eof, void *data)
{
	int ret;
	ret = snprintf(buf,len,"%d",blp_cmm_pid);
	return ret;
}

ssize_t proc_blp_cmm_write( struct file *filp, const char __user *buf,unsigned long len, void *data )
{
	int ret;
	ret = sscanf(buf,"%d",&blp_cmm_pid);
	return len;
}



static int __init btn_init(void)
{

	struct proc_dir_entry *proc_blp_cmm;   /** /proc/blp_cmm_pid **/

#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
    netlink_btn_sock = netlink_kernel_create(NETLINK_TBS, 0, recv_handler, THIS_MODULE);
#else
    netlink_btn_sock = netlink_kernel_create(&init_net, NETLINK_TBS, 0, recv_handler, NULL, THIS_MODULE);
#endif

    if (!netlink_btn_sock) {
        printk("Fail to create netlink socket.\n");
        return -1;
    }

/**  ��ѯ��ʽ�³�ʼ����ʱ��  **/
#ifdef CONFIG_BUTTON_TRIGGER_TIMER
	init_timer(&trigger_timer);
	trigger_timer.function = trigger_timer_function;
	trigger_timer.expires  = jiffies + msecs_to_jiffies(TRIGGER_TIMER_DEYLAY);
	trigger_timer.data = (unsigned long)NULL;
	add_timer( &trigger_timer);
#endif

	init_timer(&query_timer);
	query_timer.function = query_timer_function;

	/** ע�� /proc/blp_cmm_pid **/
	proc_blp_cmm = create_proc_entry( "blp_cmm_pid", 0644, NULL);
	proc_blp_cmm->read_proc  = proc_blp_cmm_read;
	proc_blp_cmm->write_proc = proc_blp_cmm_write;

	return 0;
}

static void __exit btn_exit(void)
{
	sock_release(netlink_btn_sock->sk_socket);
	remove_proc_entry("blp_cmm_pid",NULL);
}

module_init(btn_init);
module_exit(btn_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Button Core Interface");
