/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : led-rtl8676.c
 文件描述 : 本文件为RTL8676方案LED驱动程序
 作者 : 	夏超仁
 日期 : 	2011-11-14
 
**********************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <led.h>
#include <bspchip.h>
#include <rtl8676_gpio.h>

/* 定义在product.c中 */
extern struct led_dev rtl8676_leds[];

void rtl8676_led_on(struct led_dev *led)
{
    leddebug("LED_%d(gpio:%d) is set ON.\n", led->name, led->gpio);
	rtl8676_gpio_set(led->gpio, led->level);

}


void rtl8676_led_off(struct led_dev *led)
{
    leddebug("LED_%d(gpio:%d) is set OFF.\n", led->name, led->gpio);
	rtl8676_gpio_set(led->gpio, !(led->level));
}

struct led_hw_handler rtl8676_hw_handler =
{
	.led_on = rtl8676_led_on,
	.led_off = rtl8676_led_off,
};


static int __init rtl8676_led_init(void)
{
	int i;
	int ret;

	/* 把handler注册到LED CORE。注意:要先注册hanlder，再注册LED。*/
	ret = led_hw_handle_register(&rtl8676_hw_handler);
	if(ret<0)
		{
		printk(KERN_ERR "Error:fail to register rtl8676_hw_handler.\n");
		}
	printk("Register led device for %s:", "rtl8676");
	for(i=0; rtl8676_leds[i].name != led_end;i++)
		{/* Set Multiple Pins as GPIO */
		gpioConfigCNR(rtl8676_leds[i].gpio, 0);
		/* Set GPIO direction as output */
		gpioConfig(rtl8676_leds[i].gpio, GPIO_FUNC_OUTPUT);
		ret = led_dev_register(&rtl8676_leds[i]);
		if(ret<0)
			{
			printk(KERN_ERR "Error:fail to register rtl8676_leds[%d].\n", i);
			return -1;
			}
		printk(" %d", (unsigned int)rtl8676_leds[i].name);
		}
	printk("\n");
	tbs_led_system_fault_blinking(led_end);
	
	return 0;
}

static void __exit rtl8676_led_exit(void)
{
	int i;
	int ret;

	/* 先注销LED，再注销handler */
	for(i = 0; rtl8676_leds[i].name != led_end;i++)
		{
		ret = led_dev_unregister(&rtl8676_leds[i]);
		if(ret<0)
			{
			printk(KERN_ERR "Error:fail to unregister rtl8676_leds[%d]\n",i);
			}
		}
	ret = led_hw_handle_unregister(&rtl8676_hw_handler);
	if(ret<0)
		{
		printk(KERN_ERR "Error:fail to unregister rtl8676_hw_handler.\n");
		}
}
	
module_init(rtl8676_led_init);
module_exit(rtl8676_led_exit);

MODULE_AUTHOR("XiaChaoRen");
MODULE_DESCRIPTION("rtl8676 led driver");
MODULE_LICENSE("GPL");


