
/**********************************************************************
 * Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 * �ļ����� : RT63368 �����İ�ť����
**********************************************************************/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/resource.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <asm/types.h>
#include <asm/irq.h>
#include <asm/tc3162/tc3162.h>                  /* for IRQ number */
#include <gpio.h>
#include <btn.h>

extern struct btn_dev btn_table[];              /* ������product.c�� */
extern struct btn_cfg btn_config[];             /* ������product.c�� */

static void gpio_interrupt_ctrl(struct btn_dev *dev, unsigned char enable)
{
	unsigned char edge = IRQF_TRIGGER_NONE;
	
	if(enable) {
		if(GPIO_LEVEL_HIGH == dev->level) {/* ����ָ����GPIO���жϴ���ģʽΪ�����ش��� */
			edge = IRQF_TRIGGER_RISING;
		} else {/* ����ָ����GPIO���жϴ���ģʽΪ�½��ش��� */
			edge = IRQF_TRIGGER_FALLING;
		}
	}
	gpio_set_edge(dev->gpio, edge);
}

static irqreturn_t button_irq_handler(int irq, void *dev_id)
{
    struct btn_dev *dev = NULL;
	int i;	

    for(i = 0; btn_table[i].gpio != GPIO_END; i++) {
		dev = &btn_table[i];
		if(gpio_get_ins(dev->gpio)) {
			if(BTN_STATE_IDLE == dev->cur_state) {
				dev->int_ctrl(dev, 0);/* �ر�GPIO�жϣ���ֹ���� */
				mod_timer(&dev->timer, jiffies + msecs_to_jiffies(DELAY_TIME));
			}
		}
    }

    return IRQ_HANDLED;
}

static int rt63368_btn_state_read(struct btn_dev *dev)
{
	return gpio_read(dev->gpio);
}

static int __init rt63368_btn_init(void)
{
	struct btn_dev *dev = NULL;
	struct btn_cfg *cfg = NULL;
	int i, j, ret;

	ret = btn_core_init();
	if(ret < 0) {
		goto out;
	}
	for(i = 0; btn_table[i].gpio != GPIO_END; i++) { /* ��ʼ����BTN�豸�ĳ�ʼ״̬ */
		dev = &btn_table[i];
		for(j = 0; btn_config[j].act != BTN_MSG_END; j++) {/* ���ð����������Ӧʱ�� */
			cfg = &btn_config[j];
			if((cfg->mem1 == dev->gpio) && (cfg->t1_max > dev->t_max)) {
				dev->t_max = cfg->t1_max;
			}
			if((cfg->mem2 == dev->gpio) && (cfg->t2_max > dev->t_max)) {
				dev->t_max = cfg->t2_max;
			}
			if((cfg->mem3 == dev->gpio) && (cfg->t3_max > dev->t_max)) {
				dev->t_max = cfg->t3_max;
			}
		}
		dev->btn_read = rt63368_btn_state_read;
		dev->cur_state = BTN_STATE_IDLE;
		dev->int_ctrl = gpio_interrupt_ctrl;
	        gpio_config(dev->gpio, GPIO_IN);/* ����ָ����GPIOΪ����ģʽ */
		setup_timer(&dev->timer, button_timer_handler, (unsigned long)dev);/* �������� */
        	dev->int_ctrl(dev, 1);/* �����ж� */
	}
	ret = request_irq(GPIO_INT, button_irq_handler, IRQF_SHARED, "Button", dev);
	if(ret != 0) {
		printk(KERN_ERR "unable to request IRQ for GPIO (error %d)\n", ret);
	} else {
		printk("TBS button driver for %s initialized, IRQ=%d\n", PRODUCT, GPIO_INT);
	}
out:
	return ret;
}


static void __exit rt63368_btn_exit(void)
{
	struct btn_dev *dev = NULL;
	int i;

	for(i=0;btn_table[i].gpio != GPIO_END; i++) {
		dev = &btn_table[i];
		if(NULL != dev->int_ctrl) {
			dev->int_ctrl(dev, 0);/* �ر�GPIO�ж� */
		}
		dev->cur_state = BTN_STATE_IDLE;
		del_timer_sync(&dev->timer);
		dev->int_ctrl = NULL;
		dev->btn_read = NULL;
	}
	free_irq(GPIO_INT, dev);
	btn_core_exit();
}

module_init(rt63368_btn_init);
module_exit(rt63368_btn_exit);

MODULE_AUTHOR("XiaChaoRen");
MODULE_DESCRIPTION("RT63368 buttons driver");
MODULE_LICENSE("GPL");


