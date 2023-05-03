/*
 * 文件名:vx180_led_dev.c 
 * 
 * 说明:本文件为vx180方案LED驱动程序
 *  
 * 作者:Zhang Yu
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <led.h>


/* GPIO驱动接口*/
extern void gpioSetDirIn(unsigned short pin);
extern void gpioSetDirOut(unsigned short pin);
extern void gpioSetFlag(unsigned short pin);
extern void gpioClearFlag(unsigned short pin);
extern void gpioSetIntrEN(unsigned short pin);
extern void gpioClearIntrEN(unsigned short pin);
extern void gpioSetEdge(unsigned short pin); 
extern void gpioClearEdge(unsigned short pin);
extern void gpioSetPolar(unsigned short pin);
extern void gpioClearPolar(unsigned short pin);

/* 定义在product.c中 */
extern struct led_dev vx180_leds[];  


void vx180_led_on(struct led_dev *led)
{
	gpioClearIntrEN(led->gpio);
	gpioSetDirOut(led->gpio);

	if(led->level == LED_GPIO_HIGH){
		gpioSetFlag(led->gpio);
	}
	else{
		gpioClearFlag(led->gpio);
	}
}


void vx180_led_off(struct led_dev *led)
{
	gpioClearIntrEN(led->gpio);
	gpioSetDirOut(led->gpio);

	if(led->level == LED_GPIO_HIGH){
		gpioClearFlag(led->gpio);
	}
	else{
		gpioSetFlag(led->gpio);
	}
}

struct led_hw_handler vx180_hw_handler =
{
	.led_on = vx180_led_on,
	.led_off = vx180_led_off,
};


static int __init vx180_led_init(void)
{
	int i;
	int ret;

	/* 把handler注册到LED CORE。注意:要先注册hanlder，再注册LED。*/
	ret = led_hw_handle_register(&vx180_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to register vx180_hw_handler.\n");
	}

	

	/* 把LED注册到LED CORE */
	for(i=0; vx180_leds[i].name != led_end;i++){
		ret = led_dev_register(&vx180_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to register vx180_leds[%d].\n",i);
			return -1;
		}
	}


	return 0;
}

static void __exit vx180_led_exit(void)
{
	int i;
	int ret;

	/* 先注销LED，再注销handler */
	for(i=0; vx180_leds[i].name != led_end;i++){
		ret = led_dev_unregister(&vx180_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to unregister vx180_leds[%d]\n",i);
		}
	}

	
	ret = led_hw_handle_unregister(&vx180_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to unregister vx180_hw_handler.\n");
	}
	
}

module_init(vx180_led_init);
module_exit(vx180_led_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Vx180 led driver");
MODULE_LICENSE("GPL");

