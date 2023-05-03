/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称 : leds-rt65168.c
 文件描述 : 本文件为RT65168 VDSL方案LED驱动程序

 修订记录 :
          1 创建 : pengyao
            日期 : 2012-05-25
            描述 :
**********************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <led.h>
#include <gpio.h>

extern struct led_dev led_table[];                   /* 定义在product.c中 */
extern unsigned long pci_base_addr[NR_WLAN_CHIPS];   /* 定义在gpio.c中 */

void rt_led_on(struct led_dev *led)
{
	unsigned int value;
	unsigned int reg = pci_base_addr[NR_WLAN_CHIPS - 1];
	
	if(led_wlan_green == led->name) {
		if(0 != reg) {
			leddebug("pci_base_addr=%#x\n", reg);
			value = REG32(reg + WLAN_ASIC_VER_ID);/* Read mac base address register first */			
			REGWRITE32((reg + WLAN_LED_CONFIG), 0x30031432);
		} else {
			leddebug("pci_base_addr not registered yet!\n");
		}
	} else {
		gpio_write(led->gpio, led->level);
	}
}


void rt_led_off(struct led_dev *led)
{
	unsigned int value;
	unsigned int reg = pci_base_addr[NR_WLAN_CHIPS - 1];

	if(led_wlan_green == led->name) {
		if(0 != reg) {
			leddebug("%s: pci_base_addr=%#x\n", __func__, reg);
			value = REG32(reg + WLAN_ASIC_VER_ID);/* Read mac base address register first */
			REGWRITE32((reg + WLAN_LED_CONFIG), 0);

		} else {
			leddebug("%s: pci_base_addr not registered yet!\n", __func__);
		}
	} else {
		if(led->level) {
			gpio_write(led->gpio, GPIO_LEVEL_LOW);
		} else {
			gpio_write(led->gpio, GPIO_LEVEL_HIGH);
		}
	}
}

struct led_hw_handler hw_handler = {
	rt_led_on, 
	rt_led_off,
};


int __init rt65168_led_init(void)
{
	int i;
	int ret;

	ret = led_core_init();/* 初始化LED CORE */
	if(ret < 0) {
		goto out;
	}
	/* 把handler注册到LED CORE。注意:要先注册hanlder，再注册LED。*/
	ret = led_hw_handle_register(&hw_handler);
	if(ret<0){
		printk(KERN_ERR "Error:fail to register hw_handler.\n");
		goto out;
	}
	printk("Register led device for %s:", PRODUCT);
	/* 把LED注册到LED CORE */
	for(i=0; led_table[i].name != led_end;i++){
		gpio_config(led_table[i].gpio, GPIO_OUT);
		ret = led_dev_register(&led_table[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to register led_table[%d].\n",i);
			break;
		}
		printk(" %02d", (unsigned int)led_table[i].name);		
	}
	printk("\n");
out:	
	return ret;
}

void __exit rt65168_led_exit(void)
{
	int i;
	int ret;

	/* 先注销LED，再注销handler */
	for(i=0; led_table[i].name != led_end;i++){
		ret = led_dev_unregister(&led_table[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to unregister led_table[%d]\n",i);
		}
	}

	ret = led_hw_handle_unregister(&hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to unregister hw_handler.\n");
	}
	led_core_exit();
}

module_init(rt65168_led_init);
module_exit(rt65168_led_exit);
MODULE_AUTHOR("xiachaoren");
MODULE_LICENSE("GPL");
