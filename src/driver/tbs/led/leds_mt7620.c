/**********************************************************************
 Copyright (c), 1991-2011, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 MT7620 LEDs Driver                     by xiachaoren 2013-06-25
**********************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/mach-ralink/rt_mmap.h>
#include <linux/net.h>
#include <led.h>
#include <gpio.h>
#include <autoconf.h>
#include "../../../vendor/triductor/triapi/tri_ctl.h"
#include "../../../vendor/triductor/triapi/tri_para.h"

extern struct led_dev led_table[];                   /* Defined in product.c */
extern int g2_ctrl_xmit(struct g2_ctrl_msg *ctrl, size_t len);

/* Bit31: LED Polarity  0: Active low  1: Active high
 * Bit30: LED Control Owner 0: Control by G2 1: Control by External CPU
 * Bit29:28: LED Control  mode 00: Solid OFF;  01: Continuous Blinking; 10:Blinking once; 11:Solid ON
 * Bit27:14 LED Blinking OFF period(range:0~0x3FFF, time units: ms)
 * Bit13:0 LED Blinking ON period(range:0~0x3FFF, time units: ms)
 */
static void g2_led_ctrl(struct led_dev *dev, trig_name trig)
{
	struct g2_ctrl_msg msg;
	unsigned int value = BIT(30);/* Controled by Host CPU */
	
	msg.hdr.type = CTL_PKT_TYPE_SET_VAL_REQUEST;
	msg.hdr.length = htons(sizeof(msg.code) + sizeof(msg.mask) + sizeof(msg.value));
	msg.hdr.seq = htons(net_random() & 0xFFFF);
	msg.code = htonl(PARA_CODE_GPIO_VAL);
	msg.mask = htonl(BIT(dev->gpio - GPIO73));
	switch(trig) {
		case led_on_trig:
			value |= (3 << 28) | 0x3FFF;/* ON period, Bit13:0, range 0~0x3FFF */
			break;
		case led_off_trig:
			value |= 0x3FFF << 14;/* OFF period, Bit27:14, range 0~0x3FFF */
			break;
		
		case led_blinking_trig:
			value |= (2 << 28) | (30 << 14) | 70;
			break;
			
		case led_flash_hz_trig:
			value |= (1 << 28) | (500 << 14) | 500;
			break;
			
		default:
			value &= ~(BIT(30));/* Controled by G2 self */
			break;
	}
	msg.value = htonl(value);
	g2_ctrl_xmit(&msg, sizeof(struct g2_ctrl_msg));	
}

static void mt7620_led_on(struct led_dev *dev)
{
	unsigned int port;
	
	if((dev->name >= led_lan_1) && (dev->name <= led_lan_5)) {
		port = (unsigned int)(dev->name - led_lan_1);
		PHY_led_ops(port, led_on_trig);
		gpio_config(dev->gpio, GPIO_OUT);
	}
	if(dev->gpio < GPIO73) {
	    gpio_write(dev->gpio, dev->level);
	} else {
		g2_led_ctrl(dev, led_on_trig);
	}
}

static void mt7620_led_off(struct led_dev *dev)
{
	unsigned int port;
	
	if((dev->name >= led_lan_1) && (dev->name <= led_lan_5)) {
		port = (unsigned int)(dev->name - led_lan_1);
		PHY_led_ops(port, led_off_trig);
		gpio_config(dev->gpio, GPIO_OUT);
	}
	if(dev->gpio < GPIO73) {
   		if(dev->level) {
			gpio_write(dev->gpio, GPIO_LEVEL_LOW);
		} else {
			gpio_write(dev->gpio, GPIO_LEVEL_HIGH);
		}
	} else {
		g2_led_ctrl(dev, led_off_trig);
	}
}

struct led_hw_handler hw_handler = {
	mt7620_led_on, 
	mt7620_led_off,
};

static int __init mt7620_led_init(void)
{
	unsigned int i;
	int ret;
	struct led_dev *dev;

	ret = led_core_init();/* LED core initializing */
	if(ret < 0) {
		goto out;
	}
	ret = led_hw_handle_register(&hw_handler);/* Register hardware on/off handler to core */
	if(ret < 0){
		printk("Error:fail to register hw_handler.\n");
		goto out;
	}
	printk("Register led device for %s:", PRODUCT);
	for(i = 0; led_table[i].name != led_end; i++){/* Register LED devices */
		dev = &led_table[i];
		gpio_config(dev->gpio, GPIO_OUT);
		ret = led_dev_register(dev);
		if(ret<0){
			printk("Error:fail to register led_table[%d].\n", i);
			break;
		}
		if(dev->gpio >= GPIO73) {/* GPIO pins lend from G2 */
			g2_led_ctrl(dev, led_end_trig);/* Let G2 control the LED itsself */
		}
		printk(" %02d", (unsigned int)dev->name);
	}
	printk("\n");
	
out:	
	return ret;
}

static void __exit mt7620_led_exit(void)
{
	struct led_dev *dev;
	int i;
	int ret;	

	for(i=0; led_table[i].name != led_end;i++) {/* Unregister LED devices */
		dev = &led_table[i];
		ret = led_dev_unregister(dev);
		if(ret < 0){
			printk("Error:fail to unregister led_table[%d]\n",i);
		}
		if(dev->gpio >= GPIO73) {/* GPIO pins lend from G2 */
			g2_led_ctrl(dev, led_end_trig);/* Let G2 control the LED itsself */
		}		
	}
	ret = led_hw_handle_unregister(&hw_handler);/* Unregister hardware on/off handler */
	if(ret < 0){
		printk(KERN_ERR "Error:fail to unregister hw_handler.\n");
	}
	led_core_exit();
}

module_init(mt7620_led_init);
module_exit(mt7620_led_exit);

MODULE_AUTHOR("xiachaoren");
MODULE_DESCRIPTION("MT7620 LEDs driver");
MODULE_LICENSE("GPL");

