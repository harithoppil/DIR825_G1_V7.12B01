/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.

 文件名称 : leds-rt63365.c
 文件描述 : 本文件为RT63365 ADSL方案LED驱动程序

 修订记录 :
          1 创建 : pengyao
            日期 : 2012-05-25
            描述 :
**********************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/tc3162/tc3162.h>
#include <led.h>
#include <gpio.h>


extern struct led_dev led_table[];                  /* 定义在product.c中 */
extern unsigned long pci_base_addr[NR_WLAN_CHIPS];  /* 定义在gpio.c中 */
extern int gpio_num_get(int gpio);                  /* 定义在btn.c中 */
gpio_level rt_led_config(struct led_dev *led)
{
	unsigned int pins;
	unsigned int value;
	gpio_level level = led->level;
	unsigned int gpio_num = (unsigned int)led->gpio;
	
	if((gpio_num >= RT63365_GPIO07) && (gpio_num <= RT63365_GPIO11)) {/* GPIO Shared Pin */
		pins = (1 << (gpio_num + 2));
		value = REG32(CR_GPIO_SHARE);
		value &= ~pins;/* Set shared pin as GPIO */
		REGWRITE32(CR_GPIO_SHARE, value);
		if(RT63365_GPIO08 == gpio_num) {/* Hardware config pin of "DRAM type" */
			value = REG32(CR_AHB_HWCONF);
			if(value & (1 << 25)) {
				if(GPIO_LEVEL_HIGH == level) {
					level = GPIO_LEVEL_LOW;
				} else {
					level = GPIO_LEVEL_HIGH;
				}
			}
		}
	}
	return level;
}

void rt_led_on(struct led_dev *led)
{
	unsigned int value;
	gpio_level level = led->level;
	unsigned long reg = pci_base_addr[0];
	int gpio_num;

	gpio_num = gpio_num_get(led->gpio);
	if(GPIO_END == gpio_num) {
		if(0 != reg) {
			leddebug("pci_base_addr=0x%lx\n", reg);
			value = REG32(reg + WLAN_ASIC_VER_ID);/* Read mac base address register first */
			leddebug("%s: VerID=%#x\n", __func__, value >> 16);
			reg += WLAN_LED_CONFIG;
			value = REG32(reg);
			if(value & (1 << 30)) { /* LED polarity active HIGH */
				if(led_wlan == led->name) {
					value &= ~(0x03 << 28); /* LED_ACT_N off */
				} else {
					value &= ~(0x03 << 24); /* LED_RDYG_N off */
				}
			} else {/* LED polarity active LOW */
				if(led_wlan == led->name) {
					value |= (0x03 << 28); /* LED_ACT_N always on */
				} else {
					value |= (0x03 << 24); /* LED_RDYG_N always on */
				}
			}
			REGWRITE32(reg, value);
		} else {
			printk("%s: pci_base_addr not registered yet!\n", __func__);			
		}
	} else {
		level = rt_led_config(led);
		gpio_write(gpio_num, level);
	}
}


void rt_led_off(struct led_dev *led)
{
	unsigned int value;
	gpio_level level = led->level;
	unsigned long reg = pci_base_addr[0];
	int gpio_num;

	gpio_num = gpio_num_get(led->gpio);
	if(GPIO_END == gpio_num) {
		if(0 != reg) {
			leddebug("pci_base_addr=0x%lx\n", reg);
			value = REG32(reg + WLAN_ASIC_VER_ID);/* Read mac base address register first */
			leddebug("%s: VerID=%#x\n", __func__, value >> 16);
			reg += WLAN_LED_CONFIG;
			value = REG32(reg);
			if(value & (1 << 30)) { /* LED polarity active HIGH */
				if(led_wlan == led->name) {
					value |= (0x03 << 28); /* LED_ACT_N always on */
				} else {
					value |= (0x03 << 24); /* LED_RDYG_N always on */
				}
			} else {/* LED polarity active LOW */
				if(led_wlan == led->name) {
					value &= ~(0x03 << 28);/* LED_ACT_N off */
				} else {
					value &= ~(0x03 << 24); /* LED_RDYG_N off */
				}
			}
			REGWRITE32(reg, value);
		} else {
			printk("%s: pci_base_addr not registered yet!\n", __func__);
		}
	} else {
		level = rt_led_config(led);
		if(level) {
			gpio_write(gpio_num, GPIO_LEVEL_LOW);
		} else {
			gpio_write(gpio_num, GPIO_LEVEL_HIGH);
		}
	}
}

struct led_hw_handler hw_handler = {
	rt_led_on, 
	rt_led_off,
};

int __init rt63365_led_init(void)
{
	int i;
	int ret;
	int gpio_num;

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
		gpio_num = gpio_num_get(led_table[i].gpio);
		gpio_config(gpio_num, GPIO_OUT);
		ret = led_dev_register(&led_table[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to register led_table[%d].\n",i);
			break;
		}
		printk(" %02d", (unsigned int)led_table[i].name);		
	}
	printk("\n");
	for(i = 0; i < NR_WLAN_CHIPS; i++) {
		printk("PCIE IO base address[%d]=0x%lX\n", i, pci_base_addr[i]);
	}
out:	
	return ret;
}

void __exit rt63365_led_exit(void)
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

module_init(rt63365_led_init);
module_exit(rt63365_led_exit);
MODULE_AUTHOR("xiachaoren");
MODULE_LICENSE("GPL");
