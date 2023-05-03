/*
 * 文件名:c1k_led.c 
 * 
 * 说明:本文件为Mindspeed C1000方案LED驱动程序
 *  
 * 作者:Zhang Yu
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <led.h>
#include <asm/io.h>
#include <asm/hardware.h>


/* 定义在product.c中 */
extern struct led_dev c1k_leds[];  


void c1k_led_on(struct led_dev *led)
{
	comcerto_gpio_enable_output(comcerto_gpio_mask(led->gpio));

	if(led->level == LED_GPIO_HIGH){
		comcerto_gpio_set_1(comcerto_gpio_mask(led->gpio));
	}
	else{
		comcerto_gpio_set_0(comcerto_gpio_mask(led->gpio));
	}
}


void c1k_led_off(struct led_dev *led)
{
	comcerto_gpio_enable_output(comcerto_gpio_mask(led->gpio));

	if(led->level == LED_GPIO_HIGH){
		comcerto_gpio_set_0(comcerto_gpio_mask(led->gpio));
	}
	else{
		comcerto_gpio_set_1(comcerto_gpio_mask(led->gpio));
	}
}

struct led_hw_handler c1k_hw_handler =
{
	.led_on = c1k_led_on,
	.led_off = c1k_led_off,
};


static int __init c1k_led_init(void)
{
	int i;
	int ret;

	/* 把handler注册到LED CORE。注意:要先注册hanlder，再注册LED。*/
	ret = led_hw_handle_register(&c1k_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to register c1k_hw_handler.\n");
	}


	/* 把LED注册到LED CORE */
	for(i=0; c1k_leds[i].name != led_end;i++){
		ret = led_dev_register(&c1k_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to register c1k_leds[%d].\n",i);
			return -1;
		}

		/* 把GPIO引脚功能设置为GPIO模式。有些GPIO引脚是复用的，注意确认启用此GPIO不会影响到其它功能 */
		writel(readl(COMCERTO_GPIO_PIN_SELECT_REG) | 0x1 << c1k_leds[i].gpio, COMCERTO_GPIO_PIN_SELECT_REG);	
	}

	printk(KERN_INFO "C1K LED driver is registered.\n");

	return 0;
}

static void __exit c1k_led_exit(void)
{
	int i;
	int ret;

	/* 先注销LED，再注销handler */
	for(i=0; c1k_leds[i].name != led_end;i++){
		ret = led_dev_unregister(&c1k_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to unregister c1k_leds[%d]\n",i);
		}
	}

	
	ret = led_hw_handle_unregister(&c1k_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to unregister c1k_hw_handler.\n");
	}
	
}

module_init(c1k_led_init);
module_exit(c1k_led_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Ar7130 led driver");
MODULE_LICENSE("GPL");

