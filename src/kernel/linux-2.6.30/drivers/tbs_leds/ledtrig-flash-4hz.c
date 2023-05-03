/*
 * LED Flash Trigger
 *
 * 本文件实现LED闪烁事务,闪烁频率为4Hz
 * 
 *        by Zhang Yu
 */
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <led.h>

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
	flash_data->period = msecs_to_jiffies(250);  /* 闪烁周期500ms */
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

static struct led_trigger flash_4hz_led_trigger = {
	.name     = led_flash_4hz_trig,
	.activate = flash_trig_activate,
	.deactivate = flash_trig_deactivate,
};

static int __init flash_4hz_trig_init(void)
{
	return led_trigger_register(&flash_4hz_led_trigger);
}

static void __exit flash_4hz_trig_exit(void)
{
	led_trigger_unregister(&flash_4hz_led_trigger);
}

module_init(flash_4hz_trig_init);
module_exit(flash_4hz_trig_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Flash LED trigger");
MODULE_LICENSE("GPL");
