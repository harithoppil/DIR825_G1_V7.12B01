/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : ledtrig-wps_error.c
 文件描述 : WPS Error Trigger
            本事务实现亮0.1秒，灭0.1秒，循环往复。

 修订记录 :
          1 作者 : 匡素文
            日期 : 2009-07-24
            描述 : 创建

**********************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <led.h>

struct wps_error_trig_data {
	unsigned int phase;
	unsigned int period;
	struct timer_list timer;
};

static void led_wps_error_function(unsigned long data)
{
	struct led_dev *led = (struct led_dev *) data;
	struct wps_error_trig_data *trig_data = led->trigger_data;

	switch (trig_data->phase) 
    {
    	case 0:
    		led_set_on(led);
    		trig_data->phase = 1;
    		break;
    	case 1:
    		led_set_off(led);
    		trig_data->phase = 0;
    		break;
    	default:
    		led_set_off(led);
    		trig_data->phase = 0;
    		break;
	}

	mod_timer(&trig_data->timer, jiffies + trig_data->period);
}

static void wps_error_trig_activate(struct led_dev *led)
{
	struct wps_error_trig_data *trig_data = NULL;

	trig_data = kzalloc(sizeof(*trig_data), GFP_KERNEL);
	if (!trig_data)
		return;

	led->trigger_data = trig_data;
	setup_timer(&trig_data->timer,
		    led_wps_error_function, (unsigned long) led);
	
	trig_data->phase = 0;
	trig_data->period = msecs_to_jiffies(100);  /* 闪烁周期0.1s */
	led_wps_error_function(trig_data->timer.data);
}

static void wps_error_trig_deactivate(struct led_dev *led)
{
	struct wps_error_trig_data *trig_data = led->trigger_data;

	if (trig_data) {
		del_timer_sync(&trig_data->timer);
		kfree(trig_data);
	}

	led->trigger_data = NULL;
	led_set_off(led);
		
}

static struct led_trigger wps_error_led_trigger = {
	.name     = led_wps_error_trig,
	.activate = wps_error_trig_activate,
	.deactivate = wps_error_trig_deactivate,
};

static int __init wps_error_trig_init(void)
{
	return led_trigger_register(&wps_error_led_trigger);
}

static void __exit wps_error_trig_exit(void)
{
	led_trigger_unregister(&wps_error_led_trigger);
}

module_init(wps_error_trig_init);
module_exit(wps_error_trig_exit);

MODULE_AUTHOR("Kuang Suwen");
MODULE_DESCRIPTION("WPS Error LED trigger");
MODULE_LICENSE("GPL");

