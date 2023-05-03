/*
 * LED On Trigger
 *
 * 本文件实现点亮LED事务
 * 
 *        by Zhang Yu
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <led.h>

static void on_trig_activate(struct led_dev *led)
{
	led_set_on(led);
}

static void on_trig_deactivate(struct led_dev *led)
{
	led_set_off(led);
}

static struct led_trigger on_led_trigger = {
	.name     = led_on_trig,
	.activate = on_trig_activate,
	.deactivate = on_trig_deactivate,
};

static int __init on_trig_init(void)
{
	return led_trigger_register(&on_led_trigger);
}

static void __exit on_trig_exit(void)
{
	led_trigger_unregister(&on_led_trigger);
}

module_init(on_trig_init);
module_exit(on_trig_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Set LED on trigger");
MODULE_LICENSE("GPL");
