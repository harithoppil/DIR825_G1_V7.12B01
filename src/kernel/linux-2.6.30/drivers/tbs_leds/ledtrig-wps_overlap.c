/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 文件名称 : ledtrig-wps_overlap.c
 文件描述 : WPS Session Overlap Trigger
            本事务实现先以每秒5次的频率闪烁1秒，然后灯灭0.5秒，如此循环往复。

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

struct wps_overlap_trig_data {
	unsigned int phase;
	unsigned int flash_times;
	unsigned int flash_on_period;
	unsigned int flash_off_period;
	unsigned int off_period;
	struct timer_list timer;
};

static void led_wps_overlap_function(unsigned long data)
{
	struct led_dev *led = (struct led_dev *) data;
	struct wps_overlap_trig_data *trig_data = led->trigger_data;

    /* 先以0.1s的时间间隔闪烁1s */
    if (trig_data->phase < trig_data->flash_times)
    {   
        /* 奇数时灭灯0.1s */
        if (trig_data->phase & 1)
        {
            led_set_on(led);
            trig_data->phase++;
            mod_timer(&trig_data->timer, jiffies + trig_data->flash_off_period);
        }
        /* 偶数时亮灯0.1s */
        else
        {
            led_set_off(led);
            trig_data->phase++;
            mod_timer(&trig_data->timer, jiffies + trig_data->flash_on_period);
        }
    }
    /* 然后灭0.5s */
    else
    {
        led_set_off(led);
        trig_data->phase = 0;
        mod_timer(&trig_data->timer, jiffies + trig_data->off_period);
    }
}

static void wps_overlap_trig_activate(struct led_dev *led)
{
	struct wps_overlap_trig_data *trig_data;

	trig_data = kzalloc(sizeof(*trig_data), GFP_KERNEL);
	if (!trig_data)
		return;

	led->trigger_data = trig_data;

	/* 闪烁定时器设置 */
	setup_timer(&trig_data->timer,
		    led_wps_overlap_function, (unsigned long) led);

	trig_data->phase = 0;
	trig_data->flash_times = 10;   /* 闪烁次数: 亮5次，灭5次 */
	trig_data->flash_on_period  = msecs_to_jiffies(100);   /* 闪烁周期:亮灯0.1s */
	trig_data->flash_off_period = msecs_to_jiffies(100);   /* 闪烁周期:亮灯0.1s */
	trig_data->off_period       = msecs_to_jiffies(500);   /* 灭灯周期0.5s */
    led_wps_overlap_function(trig_data->timer.data);
}

static void wps_overlap_trig_deactivate(struct led_dev *led)
{
    struct wps_overlap_trig_data *trig_data = led->trigger_data;

	if (trig_data) {
		del_timer_sync(&trig_data->timer);
		kfree(trig_data);
	}

	led->trigger_data = NULL;
	led_set_off(led);
}

static struct led_trigger wps_overlap_led_trigger = {
	.name       = led_wps_overlap_trig,
	.activate   = wps_overlap_trig_activate,
	.deactivate = wps_overlap_trig_deactivate,
};

static int __init wps_overlap_trig_init(void)
{
	return led_trigger_register(&wps_overlap_led_trigger);
}

static void __exit wps_overlap_trig_exit(void)
{
	led_trigger_unregister(&wps_overlap_led_trigger);
}

module_init(wps_overlap_trig_init);
module_exit(wps_overlap_trig_exit);

MODULE_AUTHOR("Kuang Suwen");
MODULE_DESCRIPTION("WPS Session Overlap LED trigger");
MODULE_LICENSE("GPL");

