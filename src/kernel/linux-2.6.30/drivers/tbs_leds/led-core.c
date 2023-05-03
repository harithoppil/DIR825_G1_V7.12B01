/*
 * LED Core
 * 
 * TBS平台LED核心驱动
 */
#include "autoconf.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <tbs_ioctl.h>
#include <led.h>
#include <asm/uaccess.h>


struct led_dev *virt_leds[LED_NAME_MAX];
struct led_trigger *virt_trig[TRIG_NAME_MAX];

struct led_hw_handler *globe_led_hw_handler;

struct proc_dir_entry *proc_led;



/***************************************************
*
*    注册led_dev到core
*
***************************************************/
int led_dev_register(struct led_dev *led)
{
	int ret;
	
	if(led == 0){
		printk(KERN_ERR "Error:led point to NULL.\n");
		return -1;
	}

	if(globe_led_hw_handler == 0){
		printk(KERN_ERR "Error:globe_led_hw_handler point to NULL.Please register globe_led_hw_handler at first.\n");
		return -1;
	}
	
	if(virt_leds[led->name] != 0){
		printk(KERN_ERR "Error:this LED have been used.\n");
		return -1;
	}

	virt_leds[led->name] = led;

	ret = led_trigger_set(led->name,led->kernel_default);

	if(ret<0){
		printk(KERN_WARNING "Warning:fail to set default trigger.\n");
	}

	return 0;
	
}

int led_dev_unregister(struct led_dev *led)
{
	if(led == 0){
		printk(KERN_ERR "Error:led point to NULL.\n");
		return -1;
	}
	
	if(virt_leds[led->name] == 0){
		printk(KERN_WARNING "Warning:this LED is not register.\n");
		return 0;
	}

	if(led->cur_trig){
		if(led->cur_trig->deactivate)
			led->cur_trig->deactivate(led);

	}

	virt_leds[led->name] = 0;

	return 0;
	
}



/***************************************************
*
*    注册trigger到core
*
***************************************************/
int led_trigger_register(struct led_trigger *trig)
{
	if(trig == 0){
		printk(KERN_ERR "Error:trig point to NULL.\n");
		return -1;
	}
	
	if(virt_trig[trig->name] != 0){
		printk(KERN_ERR "Error:This trigger is registed.\n");
	}

	virt_trig[trig->name] = trig;

	return 0;
}

int led_trigger_unregister(struct led_trigger *trig)
{
	int i;
	
	if(trig == 0){
		printk(KERN_ERR "Error:trig point to NULL.\n");
		return -1;
	}
	
	if(virt_trig[trig->name] == 0){
		printk(KERN_WARNING "Warning:This trigger is not registered.\n");
		return 0;
	}

	for(i=0; i<LED_NAME_MAX; i++){
		if(virt_leds[i] && virt_leds[i]->cur_trig == trig){
			trig->deactivate(virt_leds[i]);
		}
	}
	
	virt_trig[trig->name] = 0;

	return 0;
}

/***************************************************
*
*    注册hw_handler到core
*
***************************************************/
int led_hw_handle_register(struct led_hw_handler *handler)
{
	if(globe_led_hw_handler != 0){
		printk(KERN_ERR "Error:globe_led_hw_handler is registered.\n");
		return -1;
	}
	
	globe_led_hw_handler = handler;

	return 0;
}

int led_hw_handle_unregister(struct led_hw_handler *handler)
{
	if(globe_led_hw_handler == 0){
		printk(KERN_WARNING "Warning:led_hw_handle led_off is not registered.\n");
	}
	
	globe_led_hw_handler = 0;

	return 0;
}


/***************************************************
*
*    设置led_dev所用的trigger
*
***************************************************/
int led_trigger_set(led_name led, trig_name trigger)
{
	struct led_dev *p_led;
	struct led_trigger *p_trig;
	
	if(led >= LED_NAME_MAX || virt_leds[led] == 0){
		//printk(KERN_ERR "Error:Led name larger then LED_NAME_MAX,OR this led doesn't register.\n");
		return -1;
	}

	p_led = virt_leds[led];
	
	if(trigger >= TRIG_NAME_MAX || virt_trig[trigger] == 0){
		//printk(KERN_ERR "Error:Trigger name larger then TRIG_NAME_MAX,OR this trigger doesn't register.\n");
		return -1;
	}

	p_trig = virt_trig[trigger];

	/* 先让原来设置的trigger失效 */
	if(p_led->cur_trig){
		if(p_led->cur_trig->deactivate)
			p_led->cur_trig->deactivate(p_led);
	}

	/* 设置新的trigger */
	if(p_trig->activate)
		p_trig->activate(p_led);

	p_led->cur_trig = p_trig;
	
	return 0;
}

/***************************************************
*
*    获取led_dev所用的trigger
*    说明：正常情况返回LED已设置trigger;
*          如果led_name异常，或此LED没有设置trigger，返回-1;
*
***************************************************/
trig_name led_trigger_get(led_name led)
{
	struct led_dev *p_led;
	struct led_trigger *p_trig;
	
	if(led >= LED_NAME_MAX || virt_leds[led] == 0){
		printk(KERN_ERR "Error:Led name larger then LED_NAME_MAX,OR this led doesn't register.\n");
		return -1;
	}

	p_led = virt_leds[led];
	

	p_trig = p_led->cur_trig;
	if(p_trig == NULL){
		printk(KERN_ERR "Error: No setting trigger on this led.\n");
		return -1;
	}
	
	return p_trig->name;
}
#if defined(CONFIG_APPS_HTML_WEB_STYLE_DLINK620) || defined(CONFIG_APPS_HTML_WEB_STYLE_DLINK620_MULTIPLE)
#if 0
void RTL8192ER_GPIO_write(unsigned int gpio_num, unsigned int value)
{   
    gpio_num = gpio_num - 31;
	if (((gpio_num >= 0) && (gpio_num <= 7))) {
		if (value)
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) & ~BIT(gpio_num + 8));
		else
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) | BIT(gpio_num + 8));
		return;
	}
	if (((gpio_num >= 8) && (gpio_num <= 11)) && ) {
		if (value)
			RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) & ~BIT(gpio_num + 12));
		else
			RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) | BIT(gpio_num + 12));
		return;
	}

	panic_printk("GPIO %d set value %d not support!\n", gpio_num, value);
	return;
}
#endif
extern void RTLWIFINIC_GPIO_write(unsigned int gpio_num, unsigned int value);
void rtl8192er_led_on_wps(struct led_dev *led)
{
	if (((led->gpio >= (31+0)) && (led->gpio <= (31+11))))
		RTLWIFINIC_GPIO_write(led->gpio, 1); 
	else 
        panic_printk("GPIO pin not supported!\n");
}
void rtl8192er_led_off_wps(struct led_dev *led)
{
	if (((led->gpio >= (31+0)) && (led->gpio <= (31+11))))
		RTLWIFINIC_GPIO_write(led->gpio, 0); 
	else 
        panic_printk("GPIO pin not supported!\n");
}
#endif


/***************************************************************
*
*    设置led_dev亮灭，统一检查globe_led_hw_handler是否已注册
*    避免在trigger中频繁检查。
*
****************************************************************/
void led_set_on(struct led_dev *led)
{
    if(led->gpio<31)
    {
	    if(globe_led_hw_handler == 0){
		    printk(KERN_ERR "Error:globe_led_hw_handler point to NULL.\n");
		    return;
	    }

	    globe_led_hw_handler->led_on(led);
    }
#if defined(CONFIG_APPS_HTML_WEB_STYLE_DLINK620) || defined(CONFIG_APPS_HTML_WEB_STYLE_DLINK620_MULTIPLE)
	else
	   rtl8192er_led_on_wps(led);
#endif
	
}

void led_set_off(struct led_dev *led)
{
	if(led->gpio<31)
    {
	    if(globe_led_hw_handler == 0){
		    printk(KERN_ERR "Error:globe_led_hw_handler point to NULL.\n");
		    return;
	    }

	    globe_led_hw_handler->led_off(led);
    }
#if defined(CONFIG_APPS_HTML_WEB_STYLE_DLINK620) || defined(CONFIG_APPS_HTML_WEB_STYLE_DLINK620_MULTIPLE)
	else
	   rtl8192er_led_off_wps(led);
#endif
	
}


/***************************************************
*
*    提供proc接口操作LED
*
***************************************************/

int led_core_proc_write( struct file *filp, const char __user *buf,unsigned long len, void *data )
{
	int ret;
	char str_buf[256];
	led_name name = led_end;
	trig_name trig = led_end_trig;
	int i;

	if(len > 255)
	{
		printk("Error. Sample: echo 0 1 > /proc/led \n");
		return len;
	}

	copy_from_user(str_buf,buf,len);
	str_buf[len] = '\0';

	for(i=0;i<len;i++)
	{
		if(str_buf[i] < 0x30 || str_buf[i] > 0x39)
		{
			if(str_buf[i] != 0x20 && str_buf[i] != 0x0a)
			{
				printk("error cmd:%s\n",str_buf);
				return len;
			}
		}
	}
	//printk("len %d:%s\n",(int)len,str_buf);

	ret = sscanf(str_buf,"%d %d", (int*)&name, (int*)&trig);
	if(ret == -1 || name == led_end || trig == led_end_trig)
	{
		printk("Error.Sample: echo 0 1 > /proc/led \n");
		return len;
	}

	//printk("Set:name %d, trigger %d\n", name, trig);
	ret = led_trigger_set(name, trig);
	if(ret == -1)
	{
		//printk("Error: trigger set fail\n");
		return len;
	}

	return len;
}


static int __init led_core_init(void)
{
	int i;

	for(i=0;i<LED_NAME_MAX;i++)
	{
		virt_leds[i] = 0;
	}

	for(i=0;i<TRIG_NAME_MAX;i++)
	{
		virt_trig[i] = 0;
	}

	proc_led = create_proc_entry( "led", 0644, NULL);
                
	proc_led->write_proc  = led_core_proc_write;

	
	return 0;
}

static void __exit led_core_exit(void)
{
	remove_proc_entry("led",proc_led);
}

module_init(led_core_init);
module_exit(led_core_exit);

EXPORT_SYMBOL(led_dev_register);
EXPORT_SYMBOL(led_dev_unregister);
EXPORT_SYMBOL(led_trigger_register);
EXPORT_SYMBOL(led_trigger_unregister);
EXPORT_SYMBOL(led_hw_handle_register);
EXPORT_SYMBOL(led_hw_handle_unregister);
EXPORT_SYMBOL(led_trigger_set);
EXPORT_SYMBOL(led_trigger_get);
EXPORT_SYMBOL_GPL(globe_led_hw_handler);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("TBS LED Core");

