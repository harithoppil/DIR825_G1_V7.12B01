

/**********************************************************************
 * Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 * 文件描述 : RT63365 方案的按钮驱动
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
#include <asm/tc3162/tc3162.h>                      /* for IRQ number */
#include <gpio.h>
#include <btn.h>

extern struct btn_dev btn_table[];                  /* Defined in product.c */
extern struct btn_cfg btn_config[];                 /* Defined in product.c */
extern unsigned long pci_base_addr[NR_WLAN_CHIPS];  /* Defined in gpio.c */

/* Wireless chip pin to pin GPIO number convert */
int gpio_num_get(int gpio)
{
	int gpio_num = gpio;
	unsigned long reg = pci_base_addr[0];
	unsigned int value = 0;

	if((gpio >= RT5392_GPIO000) && (gpio < GPIO_END)) {
		if(0 != reg) {
			value = REG32(reg + WLAN_ASIC_VER_ID);  /* Get ASIC version ID */
			value = value >> 16;			
			switch(value) {
				case 0x5390:/* 1T1R */
					switch(gpio) {
						case RT5392_GPIO004:
							gpio_num = RT5392_GPIO002;
							break;
							
						case RT5392_GPIO000:
						case RT5392_GPIO001:
						case RT5392_GPIO002:
						case RT5392_GPIO003:
						case RT5392_GPIO005:							
						case RT5392_GPIO008:
						case RT5392_GPIO009:
						case RT5392_GPIO010:
							break;

						case RT5392_GPIO011:
							gpio_num = RT5392_GPIO004;
							break;
							
						case RT5392_GPIO012:
							gpio_num = RT5392_GPIO005;
							break;
							
						case RT5392_GPIO013:
							gpio_num = RT5392_GPIO006;
							break;
							
						case RT5392_GPIO014:
							gpio_num = RT5392_GPIO007;
							break;
							
						default:
							gpio_num = GPIO_END;
							break;
					}
					break;
					
				default:					
					break;
			}
		}
	}
	
	return gpio_num;
}

static void gpio_interrupt_ctrl(struct btn_dev *dev, unsigned char enable)
{
	unsigned char edge = GPIO_E_DIS;
	int gpio_num;
	
	if(enable) {
		if(GPIO_LEVEL_HIGH == dev->level) {/* 设置指定的GPIO的中断触发模式为上升沿触发 */
			edge = IRQF_TRIGGER_RISING;
		} else {/* 设置指定的GPIO的中断触发模式为下降沿触发 */
			edge = IRQF_TRIGGER_FALLING;
		}
	}
	gpio_num = gpio_num_get(dev->gpio);
	gpio_set_edge(gpio_num, edge);
}

static irqreturn_t button_irq_handler(int irq, void *dev_id)
{
    struct btn_dev *dev = NULL;
	int status = 0;
	int i;	

    for(i = 0; btn_table[i].gpio < RT63365_GPIO14; i++) {
		dev = &btn_table[i];
		status = gpio_get_ins(dev->gpio);
		if(0 != status) {
			if(BTN_STATE_IDLE == dev->cur_state) {
				dev->int_ctrl(dev, 0);/* 关闭GPIO中断，防止重入 */
				mod_timer(&dev->timer, jiffies + msecs_to_jiffies(DELAY_TIME));
			}
		}
    }

    return IRQ_HANDLED;
}

static int rt63365_btn_state_read(struct btn_dev *dev)
{
	int gpio_num = gpio_num_get(dev->gpio);
	
	return gpio_read(gpio_num);
}

static int __init rt63365_btn_init(void)
{
	struct btn_dev *dev = NULL;
	struct btn_cfg *cfg = NULL;
    int i, j, gpio_num, ret;

	ret = btn_core_init();
	if(ret < 0) {
		goto out;
	}
	for(i = 0; btn_table[i].gpio != GPIO_END; i++) { /* 初始各个BTN设备的初始状态 */
		dev = &btn_table[i];
		for(j = 0; btn_config[j].act != BTN_MSG_END; j++) {/* 设置按键的最大响应时间 */
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
		dev->cur_state = BTN_STATE_IDLE;
		gpio_num = gpio_num_get(dev->gpio);
		gpio_config(gpio_num, GPIO_IN);/* 设置指定的GPIO为输入模式 */
		setup_timer(&dev->timer, button_timer_handler, (unsigned long)dev);/* 抖动消除定时器 */
		dev->btn_read = rt63365_btn_state_read;
		if(dev->gpio <= RT63365_GPIO14) {/* 仅GPIO0 ~ GPIO14支持中断 */
			dev->int_ctrl = gpio_interrupt_ctrl;
			dev->int_ctrl(dev, 1);/* 开启中断 */
		} else {
			dev->int_ctrl = NULL;
			mod_timer(&dev->timer, jiffies + HZ);/* 启用定时器查询 */
		}
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


static void __exit rt63365_btn_exit(void)
{
    struct btn_dev *dev = NULL;
	int i;

	for(i=0;btn_table[i].gpio != GPIO_END; i++) {
		dev = &btn_table[i];
		if(NULL != dev->int_ctrl) {
			dev->int_ctrl(dev, 0);/* 关闭GPIO中断 */
		}
		dev->cur_state = BTN_STATE_IDLE;
		del_timer_sync(&dev->timer);
		dev->btn_read = NULL;
		dev->int_ctrl = NULL;
    }
	free_irq(GPIO_INT, dev);
	btn_core_exit();
}

EXPORT_SYMBOL_GPL(gpio_num_get);
module_init(rt63365_btn_init);
module_exit(rt63365_btn_exit);

MODULE_AUTHOR("XiaChaoRen");
MODULE_DESCRIPTION("RT63365 buttons driver");
MODULE_LICENSE("GPL");

