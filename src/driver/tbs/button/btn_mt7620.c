
/**********************************************************************
 * Copyright (c), 1991-2013, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 * MT7620 Buttons driver
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
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <asm/types.h>
#include <asm/irq.h>
#include <asm/mach-ralink/surfboardint.h>
#include "../../../vendor/triductor/triapi/tri_ctl.h"
#include "../../../vendor/triductor/triapi/tri_para.h"
#include <gpio.h>
#include <btn.h>

extern struct btn_dev btn_table[];           /* Defined in product.c */
extern struct btn_cfg btn_config[];          /* Defined in product.c */
extern gpio_level *btn_level;

static gpio_level btn_level_table[4];

extern int g2_ctrl_xmit(struct g2_ctrl_msg *ctrl, size_t len);

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

static irqreturn_t button_irq_handler(int irq, void *dev_id)
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

static int mt7620_btn_state_read(struct btn_dev *dev)
{
	int ret_val = 0;
	if(dev->gpio < GPIO73) {
		ret_val = gpio_read(dev->gpio);
	} else {
		ret_val = btn_level[dev->gpio - GPIO95];
	}
	return ret_val;
}

static int __init mt7620_btn_init(void)
{
	struct btn_dev *dev = NULL;
	struct btn_cfg *cfg = NULL;
	struct g2_ctrl_msg msg;
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
		dev->btn_read = mt7620_btn_state_read;
		dev->cur_state = BTN_STATE_IDLE;/* Button's state initializing */
		setup_timer(&dev->timer, button_timer_handler, (unsigned long)dev);/* Button event validation timer */
		if(dev->gpio < GPIO73) {
			dev->int_ctrl = gpio_interrupt_ctrl;
			gpio_config(dev->gpio, GPIO_IN);/* Buttons should be working at GPIO input mode */
			dev->int_ctrl(dev, 1);/* Enable IRQ */
		}
	}
	ret_val = request_irq(SURFBOARDINT_GPIO, button_irq_handler, IRQF_SHARED, "Button", dev);
	if(ret_val != 0) {
		printk(KERN_ERR "unable to request IRQ for GPIO (error %d)\n", ret_val);
	} else {
		printk("TBS button driver for %s initialized, IRQ=%d\n", PRODUCT, SURFBOARDINT_GPIO);
	}
	for(i = 0; i < (sizeof(btn_level_table) / sizeof(gpio_level)); i++) {
		btn_level_table[i] = GPIO_LEVEL_HIGH;
	}
	btn_level = &btn_level_table[0];
	msg.hdr.type = CTL_PKT_TYPE_SET_VAL_REQUEST;
	msg.hdr.length = htons(sizeof(unsigned int) * 3);
	msg.hdr.seq = htons(net_random() & 0xFFFF);
	msg.code = htonl(PARA_CODE_GPIO_DIR);
	msg.mask = htonl(0x00c00f80);
	msg.value = htonl(0x00c00000);
	ret_val = g2_ctrl_xmit((unsigned char *)&msg, sizeof(struct g2_ctrl_msg));
	
out:
	return ret_val;
}


static void __exit mt7620_btn_exit(void)
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
	free_irq(SURFBOARDINT_GPIO, dev);
	btn_level = NULL;
	btn_core_exit();
}

module_init(mt7620_btn_init);
module_exit(mt7620_btn_exit);

MODULE_AUTHOR("XiaChaoRen");
MODULE_DESCRIPTION("MT7620 buttons driver");
MODULE_LICENSE("GPL");


