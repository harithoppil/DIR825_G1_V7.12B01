/*
 * TBS buttons driver core
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/sysdev.h>
#include <linux/timer.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/netlink.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <net/sock.h>
#include <asm/uaccess.h>
#include <btn.h>
#include <tbs_message.h>/* defined in src/include/ */

extern struct btn_dev btn_table[];           /* Defined in product.c */
extern struct btn_cfg btn_config[];          /* Defined in product.c */

struct btn_proc {
	struct sock *sk;
	unsigned int factory_mode;                  /* 产测模式或出厂模式 */
	pid_t pid;
	unsigned int gid;
	rwlock_t lock;
};

static struct btn_proc user_proc;
static DEFINE_MUTEX(btn_msg_mutex);

#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
static void recv_btn_msg(struct sock *sk, int len)
{
	struct sk_buff *skb;
	struct btn_msg *msg;
	struct nlmsghdr *nlh = NULL;
	
	do {
		mutex_lock(&btn_msg_mutex);
		while((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL) {
			if(skb->len <= NLMSG_LENGTH(sizeof(struct btn_msg))) {
				nlh = (struct nlmsghdr *)skb->data;
				msg = (struct btn_msg *)NLMSG_DATA(nlh);
				if((nlh->nlmsg_len <= NLMSG_LENGTH(sizeof(struct btn_msg))) && (skb->len <= nlh->nlmsg_len)) {					
					if(NETLINK_TBS == nlh->nlmsg_type) {
						write_lock_bh(&user_proc.lock);
						user_proc.factory_mode = msg->event & (1 << NETLINK_TBS);
						msg->event &= ~(1 << NETLINK_TBS);
						if(BTN_MSG_PID == msg->event) { //获取用户空间进程的PID和多播组ID
							if(0 == user_proc.factory_mode) {
								user_proc.pid = nlh->nlmsg_pid;
							} else {
								user_proc.factory_mode |= nlh->nlmsg_pid;
							}
							user_proc.gid = msg->gid;
						} else if (BTN_MSG_CLOSE == msg->event) { //关闭netlink通信功能
							if(nlh->nlmsg_pid == user_proc.pid) {
								user_proc.pid = 0;
								user_proc.gid = 0;
							}
						}
						write_unlock_bh(&user_proc.lock);
					}					
				}
			}
			kfree_skb(skb);
		}
		mutex_unlock(&btn_msg_mutex);
	} while (user_proc.sk && user_proc.sk->sk_receive_queue.qlen);
	//wake_up(sk->sk_sleep);	
}

#else

static void recv_btn_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;
	struct btn_msg *msg; //发给内核空间的消息数据结构

	if(NULL == skb) {
		return;
	}
	mutex_lock(&btn_msg_mutex);
	nlh = (struct nlmsghdr *)skb->data;
	if(skb->len <= NLMSG_LENGTH(sizeof(struct btn_msg))) {
		msg = (struct btn_msg *)NLMSG_DATA(nlh);
		if((nlh->nlmsg_len <= NLMSG_LENGTH(sizeof(struct btn_msg))) && (skb->len <= nlh->nlmsg_len)) {
			if(NETLINK_TBS == nlh->nlmsg_type) {
				write_lock_bh(&user_proc.lock);
				user_proc.factory_mode = msg->event & (1 << NETLINK_TBS);
				msg->event &= ~(1 << NETLINK_TBS);
				if(BTN_MSG_PID == msg->event) { //获取用户空间进程的PID和多播组ID
					if(0 == user_proc.factory_mode) {
						user_proc.pid = nlh->nlmsg_pid;
					} else {
						user_proc.factory_mode |= nlh->nlmsg_pid;
					}
					user_proc.gid = msg->gid;
				} else if(BTN_MSG_CLOSE == msg->event) { //关闭netlink通信功能
					if(nlh->nlmsg_pid == user_proc.pid) {
						user_proc.pid = 0;
						user_proc.gid = 0;
					}
					
				}
				write_unlock_bh(&user_proc.lock);
			}
		}
	}
	mutex_unlock(&btn_msg_mutex);
}
#endif

/* 通过netlink方式发送消息到应用层 */
static void send_btn_msg(unsigned int msg)
{
	int ret;
	int size;
	ST_MSG *tbs_msg;
	struct btn_msg *button_msg;
	unsigned char *old_tail;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh;
	pid_t pid = user_proc.pid;

	if(0 != user_proc.factory_mode) {
		pid = user_proc.factory_mode & ~(1 << NETLINK_TBS);
	}
	if(0 == pid) {
		printk("Receiving process not registered yet!\n");
		goto nlmsg_failure;
	}
	size = sizeof(ST_MSGHEAD) + sizeof(struct btn_msg);	
	size = NLMSG_SPACE(size);/*  size = msg header + data size */
	skb = alloc_skb(size, GFP_ATOMIC);
	if(NULL == skb) {
		goto nlmsg_failure;
	}
	old_tail = skb->tail;	
	nlh = NLMSG_PUT(skb, 0, 0, NETLINK_TBS, size-sizeof(*nlh));/* init msg header */	
	tbs_msg = NLMSG_DATA(nlh);/* point to msg data area */	
	memset(tbs_msg, 0, size);/* fill data for sending */
	tbs_msg->stMsgHead.usSrcMID = BLP_BTN_CMM_SRCMID;
	tbs_msg->stMsgHead.usDstMID = BLP_BTN_CMM_DSTMID;
	tbs_msg->stMsgHead.ulMsgID  = BLP_BTN_CMM_MSGID;
	tbs_msg->stMsgHead.usMsgType = BLP_BTN_CMM_MSGTYPE_CHANGE;
	button_msg = (struct btn_msg *)tbs_msg->szMsgBody;
	button_msg->event = msg;
	nlh->nlmsg_len = skb->tail - old_tail;/* get netlink msg length */
	NETLINK_CB(skb).pid = 0;
	NETLINK_CB(skb).dst_group = user_proc.gid;
	#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))//kernel 2.6.21 has no member dst_pid
	NETLINK_CB(skb).dst_pid = pid;
	#endif
	read_lock_bh(&user_proc.lock);
	if(0 == user_proc.gid) {
		ret = netlink_unicast(user_proc.sk, skb, pid, MSG_DONTWAIT);/* send msg */
	} else {
		ret = netlink_broadcast(user_proc.sk, skb, pid, user_proc.gid, GFP_ATOMIC);/* multicast */
	}
	read_unlock_bh(&user_proc.lock);
	if(ret <= 0) {
		printk("%s: msg=%d, gid=%d, ret=%d, is process(pid=%d) running?\n", __func__, msg, user_proc.gid, ret, pid);
	}
	goto out;

 nlmsg_failure:
	if(skb) {
		kfree_skb(skb);
	}
out:
	user_proc.factory_mode = 0;
}

static void query_btn_event(struct btn_dev *dev, unsigned long t)
{
	int i, j;
	unsigned long tvms1, tvms2, tvms3;
	struct btn_cfg *cfg;
	struct btn_dev *dev2 = NULL;
	struct btn_dev *dev3 = NULL;

	tvms1 = (t - dev->t_press) / 1000;
	for(j = 0; btn_config[j].act != BTN_MSG_END; j++) {
		cfg = &btn_config[j];
		if(0 != user_proc.factory_mode) {/* Factory test mode */
			if((BTN_STATE_RELEASE_UP == dev->cur_state) && 
				(cfg->mem1 == dev->gpio)) {/* match the release up state and 1st member's gpio number */
				dev->msg = cfg->act;
				break;
			}
		} else if(tvms1 < (dev->t_max + DELAY_TIME)) {/* Check if this button event is totally timed out */	
			if((cfg->mem1 == dev->gpio) && (cfg->state == dev->cur_state)) {/* match the gpio number and active state */
				if(GPIO_END == cfg->mem2) {/* 非组合键 */
					if((tvms1 > cfg->t1_min) && (tvms1 < cfg->t1_max)) {/* 按下时间满足要求 */
						dev->msg = cfg->act;
						break;
					}
				} else {
					for(i = 0; btn_table[i].gpio != GPIO_END; i++) {
						if(cfg->mem2 == btn_table[i].gpio) {
							dev2 = &btn_table[i];
						}
						if(cfg->mem3 == btn_table[i].gpio) {
							dev3 = &btn_table[i];
						}
					}
					if(NULL != dev2) {
						tvms2 = (t - dev2->t_press) / 1000;
						if(GPIO_END == cfg->mem3) {/* 组合二键 */
							if((tvms1 > cfg->t1_min) && (tvms1 < cfg->t1_max) && 
								(tvms2 > cfg->t2_min) && (tvms2 < cfg->t2_max) && /* 按下时间满足要求 */
								(BTN_STATE_PRESS_DOWN == dev2->cur_state)) {
								dev->msg = cfg->act;
								dev2->msg = cfg->act;
								break;
							}
						} else if (NULL != dev3) {/* 组合三键 */
							tvms3 = (t - dev3->t_press) / 1000;
							if((tvms1 > cfg->t1_min) && (tvms1 < cfg->t1_max) && 
								(tvms2 > cfg->t2_min) && (tvms2 < cfg->t2_max) && 
								(tvms3 > cfg->t3_min) && (tvms3 < cfg->t3_max) && /* 按下时间满足要求 */
								((BTN_STATE_PRESS_DOWN == dev2->cur_state) && (BTN_STATE_PRESS_DOWN == dev3->cur_state))) {/* 均为按下状态 */
								dev->msg = cfg->act;
								dev2->msg = cfg->act;
								dev3->msg = cfg->act;
								break;
							}
						}
					}
				}
			}
		}
	}
}


/************************************************************************
 * Button state handler
************************************************************************/
void button_timer_handler(unsigned long data)
{
    struct btn_dev *dev = (struct btn_dev *)data;
	struct timeval now;
	unsigned long t_us;
    int state = 0;
	
	do_gettimeofday(&now);
	t_us = (now.tv_sec * 1000000L + now.tv_usec);
	switch(dev->cur_state) {
		case BTN_STATE_IDLE:/* 抖动消除 */
			if(NULL != dev->btn_read) {
				state = dev->btn_read(dev);
			} else {
				break;
			}
			if(state == dev->level) {
				printk("Button %d pressed down\n", dev->gpio);
				dev->t_press = t_us;
				dev->msg = BTN_MSG_END;
				dev->cur_state = BTN_STATE_PRESS_DOWN;
			}
			if(NULL != dev->int_ctrl) {
				dev->int_ctrl(dev, 1);/* Renable gpio irq */
				t_us = DELAY_TIME;
			} else {
				t_us = 200; /* Redetect per t_us ms */
			}
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(t_us));
			break;

		case BTN_STATE_PRESS_DOWN:
			if(NULL != dev->btn_read) {
				state = dev->btn_read(dev);
			} else {
				break;
			}
			if(state != dev->level) {				
				printk("Button %d released up\n", dev->gpio);
				dev->t_release = t_us;
				dev->cur_state = BTN_STATE_RELEASE_UP;
			}
			if(BTN_MSG_END == dev->msg) {
				query_btn_event(dev, t_us);
				if(BTN_MSG_END != dev->msg) {
					send_btn_msg(dev->msg);
				}
			}
			mod_timer(&dev->timer, jiffies + msecs_to_jiffies(DELAY_TIME));
			break;

		case BTN_STATE_RELEASE_UP:
			dev->msg = BTN_MSG_END;
			dev->cur_state = BTN_STATE_IDLE;
			if(NULL != dev->int_ctrl) {
				dev->int_ctrl(dev, 1);/* Renable gpio irq */
			} else {
				mod_timer(&dev->timer, jiffies + (HZ / 2));
			}
			break;

		default:
			break;
	}
}

int __init btn_core_init(void)
{
	rwlock_init(&user_proc.lock);
	memset(&user_proc, 0x00, sizeof(struct btn_proc));
	#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
	user_proc.sk = netlink_kernel_create(NETLINK_TBS, 0, recv_btn_msg, NULL, THIS_MODULE);
	#else
	user_proc.sk = netlink_kernel_create(&init_net, NETLINK_TBS, 0, recv_btn_msg, NULL, THIS_MODULE);
	#endif
    if(0 == user_proc.sk) {
        printk("Fail to create netlink socket.\n");
        return -1;
    }
	return 0;
}

void __exit btn_core_exit(void)
{
	if(user_proc.sk) {
		sock_release(user_proc.sk->sk_socket);
	}
	memset(&user_proc, 0x00, sizeof(struct btn_proc));
}

MODULE_AUTHOR("XiaChaoRen");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Button Core Interface");

