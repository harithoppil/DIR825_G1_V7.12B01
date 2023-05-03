/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : ledtrig-wps_inprogress.c
 �ļ����� : WPS In Progress Trigger
            ������ʵ����0.2�룬��0.1�룬ѭ��������

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

struct wps_ingress_trig_data {
	unsigned int phase;
	unsigned int on_period;
	unsigned int off_period;
	struct timer_list timer;
};

static void led_wps_inprogress_function(unsigned long data)
{
	struct led_dev *led = (struct led_dev *) data;
	struct wps_ingress_trig_data *trig_data = led->trigger_data;

	switch (trig_data->phase) 
    {
    	case 0:
    		led_set_on(led);
    		trig_data->phase = 1;
    		mod_timer(&trig_data->timer, jiffies + trig_data->on_period);
    		break;
    	case 1:
    		led_set_off(led);
    		trig_data->phase = 0;
    		mod_timer(&trig_data->timer, jiffies + trig_data->off_period);
    		break;
    	default:
    		led_set_off(led);
    		trig_data->phase = 0;
    		mod_timer(&trig_data->timer, jiffies + trig_data->off_period);
    		break;
	}

}

static void wps_inprogress_trig_activate(struct led_dev *led)
{
	struct wps_ingress_trig_data *trig_data = NULL;

	trig_data = kzalloc(sizeof(*trig_data), GFP_KERNEL);
	if (!trig_data)
		return;

	led->trigger_data = trig_data;

	/* ��˸��ʱ������ */
	setup_timer(&trig_data->timer,
		    led_wps_inprogress_function, (unsigned long) led);

	trig_data->phase = 0;
	trig_data->on_period  = msecs_to_jiffies(200);          /* ��������0.2s */
	trig_data->off_period = msecs_to_jiffies(100);          /* �������0.1s */
    led_wps_inprogress_function(trig_data->timer.data);
}

static void wps_inprogress_trig_deactivate(struct led_dev *led)
{
    struct wps_ingress_trig_data *trig_data = led->trigger_data;

	if (trig_data) {
		del_timer_sync(&trig_data->timer);
		kfree(trig_data);
	}

	led->trigger_data = NULL;
	led_set_off(led);
}

static struct led_trigger wps_inprogress_led_trigger = {
	.name       = led_wps_inprogress_trig,
	.activate   = wps_inprogress_trig_activate,
	.deactivate = wps_inprogress_trig_deactivate,
};

static int __init wps_inprogress_trig_init(void)
{
	return led_trigger_register(&wps_inprogress_led_trigger);
}

static void __exit wps_inprogress_trig_exit(void)
{
	led_trigger_unregister(&wps_inprogress_led_trigger);
}

module_init(wps_inprogress_trig_init);
module_exit(wps_inprogress_trig_exit);

MODULE_AUTHOR("Kuang Suwen");
MODULE_DESCRIPTION("WPS In Progress LED trigger");
MODULE_LICENSE("GPL");

