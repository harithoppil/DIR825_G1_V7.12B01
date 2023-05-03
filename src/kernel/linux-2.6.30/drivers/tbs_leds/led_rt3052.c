/*
 * 文件名:rt3052_led.c 
 * 
 * 说明:本文件为Ralink方案LED驱动程序
 *  
 * 作者:Xuan Guanglei
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <led.h>
#include "ralink_gpio.h"
#include <asm/delay.h>
#include <linux/time.h>

extern struct led_dev rt3052_leds[];  
extern void rt3052_gpio_mode(int gpio);
extern void rt3052_gpio_config_output(int gpio);
extern void rt3052_gpio_out_val(int gpio, int val);

void rt3052_led_on(struct led_dev *led)
{
	rt3052_gpio_mode(led->gpio);
	rt3052_gpio_config_output(led->gpio);

	if(led->level == LED_GPIO_HIGH)
	{
		rt3052_gpio_out_val(led->gpio,1);
	}
	else
	{
		rt3052_gpio_out_val(led->gpio,0);
	}
}


void rt3052_led_off(struct led_dev *led)
{
	rt3052_gpio_mode(led->gpio);
	rt3052_gpio_config_output(led->gpio);

	if(led->level == LED_GPIO_HIGH)
	{
		rt3052_gpio_out_val(led->gpio,0);
	}
	else
	{
		rt3052_gpio_out_val(led->gpio,1);
	}
}

struct led_hw_handler rt3052_hw_handler =
{
	.led_on = rt3052_led_on,
	.led_off = rt3052_led_off,
};


static int __init rt3052_led_init(void)
{
	int i;
	int ret;

	/* 把handler注册到LED CORE。注意:要先注册hanlder，再注册LED。*/
	ret = led_hw_handle_register(&rt3052_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to register rt3052_hw_handler.\n");
	}

	/* 把LED注册到LED CORE */
	for(i=0; rt3052_leds[i].name != led_end;i++){
		ret = led_dev_register(&rt3052_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to register rt3052_leds[%d].\n",i);
			return -1;
		}
	}

	return 0;
}

static void __exit rt3052_led_exit(void)
{
	int i;
	int ret;

	/* 先注销LED，再注销handler */
	for(i=0; rt3052_leds[i].name != led_end;i++){
		ret = led_dev_unregister(&rt3052_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to unregister ar7130_leds[%d]\n",i);
		}
	}

	
	ret = led_hw_handle_unregister(&rt3052_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to unregister ar7130_hw_handler.\n");
	}
	
}

module_init(rt3052_led_init);
module_exit(rt3052_led_exit);

MODULE_AUTHOR("Xuang Guanglei");
MODULE_DESCRIPTION("Rt3052 led driver");
MODULE_LICENSE("GPL");

