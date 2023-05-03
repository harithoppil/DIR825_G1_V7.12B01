/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : ledtrig-wps_overlap.c
 �ļ����� : WPS Session Overlap Trigger
            ������ʵ������ÿ��5�ε�Ƶ����˸1�룬Ȼ�����0.5�룬���ѭ��������

 �޶���¼ :
          1 ���� : ������
            ���� : 2009-07-24
            ���� : ����

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

    /* ����0.1s��ʱ������˸1s */
    if (trig_data->phase < trig_data->flash_times)
    {   
        /* ����ʱ���0.1s */
        if (trig_data->phase & 1)
        {
            led_set_on(led);
            trig_data->phase++;
            mod_timer(&trig_data->timer, jiffies + trig_data->flash_off_period);
        }
        /* ż��ʱ����0.1s */
        else
        {
            led_set_off(led);
            trig_data->phase++;
            mod_timer(&trig_data->timer, jiffies + trig_data->flash_on_period);
        }
    }
    /* Ȼ����0.5s */
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

	/* ��˸��ʱ������ */
	setup_timer(&trig_data->timer,
		    led_wps_overlap_function, (unsigned long) led);

	trig_data->phase = 0;
	trig_data->flash_times = 10;   /* ��˸����: ��5�Σ���5�� */
	trig_data->flash_on_period  = msecs_to_jiffies(100);   /* ��˸����:����0.1s */
	trig_data->flash_off_period = msecs_to_jiffies(100);   /* ��˸����:����0.1s */
	trig_data->off_period       = msecs_to_jiffies(500);   /* �������0.5s */
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

