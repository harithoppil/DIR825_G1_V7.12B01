
/**********************************************************************
 * Copyright (c), 1991-2013, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 * mt751x Buttons driver
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

extern struct btn_dev btn_table[];           /* Defined in product.c */
extern struct btn_cfg btn_config[];          /* Defined in product.c */

static void gpio_interrupt_ctrl(struct btn_dev *dev, unsigned char enable)
{
	unsigned char edge = IRQF_TRIGGER_NONE;
	
	if(enable) {
		if(GPIO_LEVEL_HIGH == dev->level) {/* Trigger at rising edge */
			edge = IRQF_TRIGGER_RISING;
		} else {/* Trigger at falling edge */
			edge = IRQF_TRIGGER_FALLING;
		}
	}
	gpio_set_edge(dev->gpio, edge);
}

static irqreturn_t button_isr(int irq, void *dev_id)
{
    struct btn_dev *dev = NULL;
	int i;	

    for(i = 0; btn_table[i].gpio != GPIO_END; i++) {
		dev = &btn_table[i];
		if(gpio_get_ins(dev->gpio)) {/* Read IRQ status and make it clean */
			if(BTN_STATE_IDLE == dev->cur_state) {
				dev->int_ctrl(dev, 0);/* Disable IRQ first */
				mod_timer(&dev->timer, jiffies + msecs_to_jiffies(DELAY_TIME));
			}
		}
    }

    return IRQ_HANDLED;
}

static int mt751x_btn_state_read(struct btn_dev *dev)
{
	int ret_val = 0;
	
	if(dev->gpio < GPIO64) {
		ret_val = gpio_read(dev->gpio);
	}
	
	return ret_val;
}

static int __init mt751x_btn_init(void)
{
	struct btn_dev *dev = NULL;
	struct btn_cfg *cfg = NULL;
	int i, j, ret_val = -EFAULT;

	ret_val = btn_core_init();
	if(ret_val < 0) {
		goto out;
	}
	for(i = 0; btn_table[i].gpio != GPIO_END; i++) { 
		dev = &btn_table[i];
		for(j = 0; btn_config[j].act != BTN_MSG_END; j++) {/* Get the maximum active time for each button device */
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
		dev->btn_read = mt751x_btn_state_read;
		dev->cur_state = BTN_STATE_IDLE;/* Button's state initializing */
		setup_timer(&dev->timer, button_timer_handler, (unsigned long)dev);/* Button event validation timer */
		if(dev->gpio < GPIO64) {
			if(dev->gpio < GPIO16) {
				dev->int_ctrl = gpio_interrupt_ctrl;			
				dev->int_ctrl(dev, 1);/* Enable IRQ */
			} else {
				dev->int_ctrl = NULL;
				mod_timer(&dev->timer, jiffies + HZ);
			}
			gpio_config(dev->gpio, GPIO_IN);/* Buttons should be working at GPIO input mode */
		}
	}
	ret_val = request_irq(GPIO_INT, button_isr, IRQF_SHARED, "Button", dev);
	if(ret_val != 0) {
		printk(KERN_ERR "unable to request IRQ for GPIO (error %d)\n", ret_val);
	} else {
		printk("TBS button driver for %s initialized, IRQ=%d\n", PRODUCT, GPIO_INT);
	}
	
out:
	return ret_val;
}


static void __exit mt751x_btn_exit(void)
{
	struct btn_dev *dev = NULL;
	int i;

	for(i=0;btn_table[i].gpio != GPIO_END; i++) {
		dev = &btn_table[i];
		if(NULL != dev->int_ctrl) {
			dev->int_ctrl(dev, IRQF_TRIGGER_NONE);/* Disable IRQ */
		}
		dev->cur_state = BTN_STATE_IDLE;
		del_timer_sync(&dev->timer);
		dev->int_ctrl = NULL;
		dev->btn_read = NULL;
	}
	free_irq(GPIO_INT, dev);
	btn_core_exit();
}

module_init(mt751x_btn_init);
module_exit(mt751x_btn_exit);

MODULE_AUTHOR("XiaChaoRen");
MODULE_DESCRIPTION("mt751x buttons driver");
MODULE_LICENSE("GPL");


