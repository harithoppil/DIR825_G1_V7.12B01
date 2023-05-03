/*
 * 文件名:ar7130_led_dev.c 
 * 
 * 说明:本文件为ar7130方案LED驱动程序
 *  
 * 作者:Zhang Yu
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <led.h>


/* ar7130原芯片厂商的GPIO驱动接口*/
extern void ar7100_gpio_intr_disable(unsigned int irq);
extern void ar7100_gpio_config_output(int gpio);
extern void ar7100_gpio_out_val(int gpio, int val);

/* 定义在ar7130_pb42_product.c中 */
extern struct led_dev ar7130_leds[];  


void ar7130_led_on(struct led_dev *led)
{
	ar7100_gpio_intr_disable(led->gpio);
	ar7100_gpio_config_output(led->gpio);

	if(led->level == LED_GPIO_HIGH){
		ar7100_gpio_out_val(led->gpio,1);
	}
	else{
		ar7100_gpio_out_val(led->gpio,0);
	}
}


void ar7130_led_off(struct led_dev *led)
{
	ar7100_gpio_intr_disable(led->gpio);
	ar7100_gpio_config_output(led->gpio);

	if(led->level == LED_GPIO_HIGH){
		ar7100_gpio_out_val(led->gpio,0);
	}
	else{
		ar7100_gpio_out_val(led->gpio,1);
	}
}

struct led_hw_handler ar7130_hw_handler =
{
	.led_on = ar7130_led_on,
	.led_off = ar7130_led_off,
};


static int __init ar7130_led_init(void)
{
	int i;
	int ret;

	/* 把handler注册到LED CORE。注意:要先注册hanlder，再注册LED。*/
	ret = led_hw_handle_register(&ar7130_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to register ar7130_hw_handler.\n");
	}

	

	/* 把LED注册到LED CORE */
	for(i=0; ar7130_leds[i].name != led_end;i++){
		ret = led_dev_register(&ar7130_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to register ar7130_leds[%d].\n",i);
			return -1;
		}
	}


	return 0;
}

static void __exit ar7130_led_exit(void)
{
	int i;
	int ret;

	/* 先注销LED，再注销handler */
	for(i=0; ar7130_leds[i].name != led_end;i++){
		ret = led_dev_unregister(&ar7130_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to unregister ar7130_leds[%d]\n",i);
		}
	}

	
	ret = led_hw_handle_unregister(&ar7130_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to unregister ar7130_hw_handler.\n");
	}
	
}

module_init(ar7130_led_init);
module_exit(ar7130_led_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Ar7130 led driver");
MODULE_LICENSE("GPL");

