/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : ledtrig-wps_success.c
 文件描述 : WPS Success Trigger
            本事务实现常亮300s，然后关闭

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

struct wps_success_trig_data {
	unsigned int phase;
	unsigned int period;
	struct timer_list timer;
};

static void led_wps_success_function(unsigned long data)
{
	struct led_dev *led = (struct led_dev *) data;
	struct wps_success_trig_data *trig_data = led->trigger_data;

	switch (trig_data->phase) 
    {
        /* 亮300s, 然后关闭 */
    	case 0:
    		led_set_on(led);
    		trig_data->phase = 1;
            mod_timer(&trig_data->timer, jiffies + trig_data->period);
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
}

static void wps_success_trig_activate(struct led_dev *led)
{
	struct wps_success_trig_data *trig_data = NULL;

	trig_data = kzalloc(sizeof(*trig_data), GFP_KERNEL);
	if (!trig_data)
		return;

	led->trigger_data = trig_data;
	setup_timer(&trig_data->timer,
		    led_wps_success_function, (unsigned long) led);
	
	trig_data->phase = 0;
	trig_data->period = msecs_to_jiffies(1000*300);  /* 亮300s */
	led_wps_success_function(trig_data->timer.data);
}

static void wps_success_trig_deactivate(struct led_dev *led)
{
	struct wps_success_trig_data *trig_data = led->trigger_data;

	if (trig_data) {
		del_timer_sync(&trig_data->timer);
		kfree(trig_data);
	}

	led->trigger_data = NULL;
	led_set_off(led);
}

static struct led_trigger wps_success_led_trigger = {
	.name     = led_wps_success_trig,
	.activate = wps_success_trig_activate,
	.deactivate = wps_success_trig_deactivate,
};

static int __init wps_success_trig_init(void)
{
	return led_trigger_register(&wps_success_led_trigger);
}

static void __exit wps_success_trig_exit(void)
{
	led_trigger_unregister(&wps_success_led_trigger);
}

module_init(wps_success_trig_init);
module_exit(wps_success_trig_exit);

MODULE_AUTHOR("Kuang Suwen");
MODULE_DESCRIPTION("WPS Success LED trigger");
MODULE_LICENSE("GPL");

