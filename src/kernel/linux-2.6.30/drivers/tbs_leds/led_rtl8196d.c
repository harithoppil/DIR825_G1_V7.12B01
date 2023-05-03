/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : led_rtl8672.c
 文件描述 : 本文件为RTL8196d方案LED驱动程序

 修订记录 :
          1 作者 : 匡素文
            日期 : 2010-07-14
            描述 : 创建

**********************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <led.h>

#include "bsp/bspchip.h"


//#define LED_DEBUG
#ifdef LED_DEBUG
    #define LED_TRACE(str, args...) printk(str, ##args)
#else
    #define LED_TRACE(str, args...)  do { ; } while(0);
#endif



/* RTL8672原芯片厂商的GPIO驱动接口*/
extern void gpioConfig (int gpio_num, int gpio_func);
extern void gpioConfigCNR(int gpio_num, int mode);
extern void gpioSet(int gpio_num);
extern void gpioClear(int gpio_num);

/* 定义在product.c中 */
extern struct led_dev rtl8196d_leds[];

void rtl8196d_led_on(struct led_dev *led)
{
    LED_TRACE("LED_%d(gpio:%d) is On.\n", led->name, led->gpio);
    gpioConfig(led->gpio, GPIO_FUNC_OUTPUT);

	if(led->level == LED_GPIO_HIGH){
		gpioSet(led->gpio);
	}
	else{
		gpioClear(led->gpio);
	}
}


void rtl8196d_led_off(struct led_dev *led)
{
    LED_TRACE("LED_%d(gpio:%d) is Off.\n", led->name, led->gpio);
	gpioConfig(led->gpio, GPIO_FUNC_OUTPUT);

	if(led->level == LED_GPIO_HIGH){
		gpioClear(led->gpio);
	}
	else{
		gpioSet(led->gpio);
	}
}

struct led_hw_handler rtl8196d_hw_handler =
{
	.led_on = rtl8196d_led_on,
	.led_off = rtl8196d_led_off,
};


static int __init rtl8196d_led_init(void)
{
	int i;
	int ret;

	/* 把handler注册到LED CORE。注意:要先注册hanlder，再注册LED。*/
	ret = led_hw_handle_register(&rtl8196d_hw_handler);
	if(ret<0){
		printk(KERN_ERR "Error:fail to register rtl8196d_hw_handler.\n");
	}

    #if 0
    /* Enable GPIO D2, D3, D4, D6 */
    if( IS_6028B || IS_6085 ) {
		REG32(MISC_PINOCR) |= 0x000c0000;
	}
    #endif

	/* 把LED注册到LED CORE */
	for(i=0; rtl8196d_leds[i].name != led_end;i++)
    {

        /*设置为GPIO模式*/
		gpioConfigCNR(rtl8196d_leds[i].gpio, 0);

		/* 初始化GPIO接口的工作状态，设置为输出模式 */
		gpioConfig(rtl8196d_leds[i].gpio, GPIO_FUNC_OUTPUT);

		ret = led_dev_register(&rtl8196d_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to register rtl8196d_leds[%d].\n",i);
			return -1;
		}
	}

    #if 0
    /* 创建gpio接口 */
    proc_gpio = create_proc_entry( "gpio", 0644, NULL);
	proc_gpio->write_proc  = gpio_proc_write;
    #endif

	return 0;
}

static void __exit rtl8196d_led_exit(void)
{
	int i;
	int ret;

    #if 0
    /* 移除gpio接口 */
    remove_proc_entry("gpio", proc_gpio);
    #endif

	/* 先注销LED，再注销handler */
	for(i=0; rtl8196d_leds[i].name != led_end;i++){
		ret = led_dev_unregister(&rtl8196d_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to unregister rtl8196d_leds[%d]\n",i);
		}
	}

	ret = led_hw_handle_unregister(&rtl8196d_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to unregister rtl8196d_hw_handler.\n");
	}

}



module_init(rtl8196d_led_init);
module_exit(rtl8196d_led_exit);

MODULE_AUTHOR("Kuangsuwen");
MODULE_DESCRIPTION("Rtl8196d led driver");
MODULE_LICENSE("GPL");


