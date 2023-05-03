/*
 * LED Off Trigger
 *
 * 本文件实现熄灭LED事务
 * 
 *        by Zhang Yu
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <led.h>

static void off_trig_activate(struct led_dev *led)
{
	led_set_off(led);
}

static void off_trig_deactivate(struct led_dev *led)
{
	led_set_off(led);
}

static struct led_trigger off_led_trigger = {
	.name     = led_off_trig,
	.activate = off_trig_activate,
	.deactivate = off_trig_deactivate,
};

static int __init off_trig_init(void)
{
	return led_trigger_register(&off_led_trigger);
}

static void __exit off_trig_exit(void)
{
	led_trigger_unregister(&off_led_trigger);
}

module_init(off_trig_init);
module_exit(off_trig_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Set LED off trigger");
MODULE_LICENSE("GPL");
