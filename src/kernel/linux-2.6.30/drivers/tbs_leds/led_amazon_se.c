/*
 * 文件名:amazon_se_dev.c 
 * 
 * 说明:本文件为Infineon Amazon-SE方案LED驱动程序
 *  
 * 作者:Zhang Yu
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <led.h>


/* Amazon-SE的原版驱动，定义在arch/mips下 */
extern int amazon_se_port_reserve_pin(int port, int pin, int module_id);
extern int amazon_se_port_set_open_drain(int port, int pin, int module_id); 
extern int amazon_se_port_clear_altsel0(int port, int pin, int module_id);
extern int amazon_se_port_clear_altsel1(int port, int pin, int module_id);
extern int amazon_se_port_set_dir_out(int port, int pin, int module_id);
extern int amazon_se_port_set_output(int port, int pin, int module_id);
extern int amazon_se_port_clear_output(int port, int pin, int module_id);
extern int amazon_se_port_free_pin(int port, int pin, int module_id);



/* 定义在product.c中 */
extern struct led_dev amazon_se_leds[];  


/*
 * Amazon-SE 中有两个port，它们的pin都是0~15，
 * 为方便使用，此驱动中将两个port的pin统一编号，
 * port0为0~15，port1为16~31
 */
int pin_to_gpio_port(int pin)
{
	if(pin>=0 && pin<16)
		return 0;
	else if(pin>=16 && pin<32)
			return 1;
	else
		{
			printk("ERROR:Pin must be 0~31\n");
			return -1;
		}
}


int pin_to_gpio_pin(int pin)
{
	if(pin>=0 && pin<16)
		return pin;
	else if(pin>=16 && pin<32)
			return (pin-16);
	else
		{
			printk("ERROR:Pin must be 0~31\n");
			return -1;
		}
}


void amazon_se_led_on(struct led_dev *led)
{
	amazon_se_port_set_open_drain(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);
	amazon_se_port_clear_altsel0(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);
	amazon_se_port_clear_altsel1(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);
	amazon_se_port_set_dir_out(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);


	if(led->level == LED_GPIO_HIGH){
		amazon_se_port_set_output(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);
	}
	else{
		amazon_se_port_clear_output(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);
	}
}


void amazon_se_led_off(struct led_dev *led)
{
	amazon_se_port_set_open_drain(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);
	amazon_se_port_clear_altsel0(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);
	amazon_se_port_clear_altsel1(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);
	amazon_se_port_set_dir_out(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);


	if(led->level == LED_GPIO_HIGH){
		amazon_se_port_clear_output(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);
	}
	else{
		amazon_se_port_set_output(pin_to_gpio_port(led->gpio), pin_to_gpio_pin(led->gpio),(unsigned int)THIS_MODULE);
	}
}

struct led_hw_handler amazon_se_hw_handler =
{
	.led_on = amazon_se_led_on,
	.led_off = amazon_se_led_off,
};


static int __init amazon_se_led_init(void)
{
	int i;
	int ret;

	/* 把handler注册到LED CORE。注意:要先注册hanlder，再注册LED。*/
	ret = led_hw_handle_register(&amazon_se_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to register amazon_se_hw_handler.\n");
	}

	

	/* 把LED注册到LED CORE */
	for(i=0; amazon_se_leds[i].name != led_end;i++){
		/* 获取GPIO */
		ret = amazon_se_port_reserve_pin(pin_to_gpio_port(amazon_se_leds[i].gpio), pin_to_gpio_pin(amazon_se_leds[i].gpio),(unsigned int)THIS_MODULE);
		if(ret!=0){
			printk(KERN_ERR "Error:fail to reserve pin %d.\n",amazon_se_leds[i].gpio);
			return -1;
		}
		
		ret = led_dev_register(&amazon_se_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to register amazon_se_leds[%d].\n",i);
			return -1;
		}
	}


	return 0;
}

static void __exit amazon_se_led_exit(void)
{
	int i;
	int ret;

	/* 先注销LED，再注销handler */
	for(i=0; amazon_se_leds[i].name != led_end;i++){
		ret = led_dev_unregister(&amazon_se_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to unregister amazon_se_leds[%d]\n",i);
		}
		/* 释放GPIO */
		amazon_se_port_free_pin(pin_to_gpio_port(amazon_se_leds[i].gpio), pin_to_gpio_pin(amazon_se_leds[i].gpio),(unsigned int)THIS_MODULE);
	}

	
	ret = led_hw_handle_unregister(&amazon_se_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to unregister amazon_se_hw_handler.\n");
	}
	
}

module_init(amazon_se_led_init);
module_exit(amazon_se_led_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Amazon-SE led driver");
MODULE_LICENSE("GPL");

