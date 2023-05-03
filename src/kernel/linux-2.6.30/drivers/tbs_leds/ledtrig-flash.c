/*
 * LED Flash Trigger
 *
 * 本文件实现LED闪烁事务
 * 
 *        by Zhang Yu
 */
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <led.h>

#ifdef CONFIG_WLAN_RSSI_LED
unsigned short led_control=0x0;
#define DATA_BIT 0xf
#endif
struct flash_trig_data {
	unsigned int phase;
	unsigned int period;
	struct timer_list timer;
};

static void led_flash_function(unsigned long data)
{
	struct led_dev *led = (struct led_dev *) data;
	struct flash_trig_data *flash_data = led->trigger_data;


	switch (flash_data->phase) {
	case 0:
		led_set_on(led);
		flash_data->phase = 1;
		break;
	case 1:
		led_set_off(led);
		flash_data->phase = 0;
		break;
	default:
		led_set_off(led);
		flash_data->phase = 0;
		break;
	}
	
//若有数据传输，则将闪烁的频率提高到10HZ
#ifdef CONFIG_WLAN_RSSI_LED
	if(led_control & DATA_BIT)
	{
		flash_data->period = msecs_to_jiffies(50);  /* 闪烁周期100ms ，检测周期也是100ms*/
		led_control&=(~DATA_BIT); /*去掉数据传输的标记*/
	}
	else
	{
		flash_data->period = msecs_to_jiffies(500);  /* 闪烁周期1000ms */
	}
#endif

	mod_timer(&flash_data->timer, jiffies + flash_data->period);
}

static void flash_trig_activate(struct led_dev *led)
{
	struct flash_trig_data *flash_data;

	flash_data = kzalloc(sizeof(*flash_data), GFP_KERNEL);
	if (!flash_data)
		return;

	led->trigger_data = flash_data;
	setup_timer(&flash_data->timer,
		    led_flash_function, (unsigned long) led);
	
	flash_data->phase = 0;
	flash_data->period = msecs_to_jiffies(500);  /* 闪烁周期500ms */
	led_flash_function(flash_data->timer.data);
}

static void flash_trig_deactivate(struct led_dev *led)
{
	struct flash_trig_data *flash_data = led->trigger_data;

	if (flash_data) {
		del_timer_sync(&flash_data->timer);
		kfree(flash_data);
	}

	led->trigger_data = NULL;
	led_set_off(led);
		
}

static struct led_trigger flash_led_trigger = {
	.name     = led_flash_trig,
	.activate = flash_trig_activate,
	.deactivate = flash_trig_deactivate,
};

static int __init flash_trig_init(void)
{
	return led_trigger_register(&flash_led_trigger);
}

static void __exit flash_trig_exit(void)
{
	led_trigger_unregister(&flash_led_trigger);
}

module_init(flash_trig_init);
module_exit(flash_trig_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Flash LED trigger");
MODULE_LICENSE("GPL");
