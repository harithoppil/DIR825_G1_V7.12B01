/*
 * LED DataFlash Trigger
 *
 * 本文件实现数据灯闪烁事务。
 * 说明：本事务实现亮0.1秒，灭0.1秒，不循环。设置一次闪烁一次，可以高频率调用。
 *       实现原理是在频繁调用下不分配/释放内存，空闲5秒钟后再做释放资源的动作。
 *
 * 注意：一般情况下，只能让数据灯设置为此事务。并且如果数据灯设置过其它事务后，
 *       务必要把led->trigger_data清零。
 * 
 * by Zhang Yu
 */
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <led.h>

struct dataflash_trig_data {
	struct led_dev *led;
	unsigned int phase;
	unsigned int flash_period;
	unsigned int release_period;
	struct timer_list flash_timer;
	struct timer_list release_timer;
};


static void auto_dataflash_trig_deactivate(unsigned long data);


static void led_dataflash_function(unsigned long data)
{
	struct led_dev *led = (struct led_dev *) data;
	struct dataflash_trig_data *dataflash_data = led->trigger_data;


	switch (dataflash_data->phase) {
	case 0:
		//led_set_off(led);
		dataflash_data->phase = 1;
		mod_timer(&dataflash_data->flash_timer, jiffies + dataflash_data->flash_period);
		break;
	case 1:
		led_set_off(led);
		dataflash_data->phase = 2;
		mod_timer(&dataflash_data->flash_timer, jiffies + dataflash_data->flash_period);
		break;
	case 2:
		led_set_on(led);
		dataflash_data->phase = 3;  /* 3--本次处理周期已完成 */
		mod_timer(&dataflash_data->release_timer, jiffies + dataflash_data->release_period);
		break;
	default:
		led_set_on(led);
		dataflash_data->phase = 3;
		mod_timer(&dataflash_data->release_timer, jiffies + dataflash_data->release_period);
		break;
	}

}

static void dataflash_trig_activate(struct led_dev *led)
{
	struct dataflash_trig_data *dataflash_data;

	if(led->trigger_data == NULL) 	/* dataflash_trig_data 已经被清理掉 */
	{
		dataflash_data = kzalloc(sizeof(*dataflash_data), GFP_KERNEL);
		if (!dataflash_data)
			return;

		led->trigger_data = dataflash_data;

		/* 闪烁定时器设置 */
		setup_timer(&dataflash_data->flash_timer,
			    led_dataflash_function, (unsigned long) led);
	
		dataflash_data->phase = 0;
		dataflash_data->flash_period = msecs_to_jiffies(50);  /* 闪烁周期50ms */
		dataflash_data->release_period = msecs_to_jiffies(5000);  /* 清理周期5000ms */
		led_dataflash_function(dataflash_data->flash_timer.data);

		/* 清理dataflash_trig_data的定时器设置 */
		setup_timer(&dataflash_data->release_timer,
			    auto_dataflash_trig_deactivate, (unsigned long) dataflash_data);

		dataflash_data->led = led;
	}
	else
	{
		dataflash_data = led->trigger_data;

		if(dataflash_data->phase != 3)
			return;

		dataflash_data->phase = 0;
		led_dataflash_function(dataflash_data->flash_timer.data);

	}
}

static void dataflash_trig_deactivate(struct led_dev *led)
{
	/* do nothing */
}
	

/* 长时间不点亮数据灯后，自动释放data_flash_data，一切复原。*/
static void auto_dataflash_trig_deactivate(unsigned long data) 
{
	struct dataflash_trig_data *dataflash_data = (struct dataflash_trig_data *)data; 
	struct led_dev *led;

	if (!dataflash_data)
		return;

	led = dataflash_data->led;

	if(led->trigger_data == dataflash_data) /* 如果此域还没有被其它trig修改 */
		led->trigger_data = NULL;

	del_timer_sync(&dataflash_data->flash_timer);
	del_timer_sync(&dataflash_data->release_timer);
	kfree(dataflash_data);
}

static struct led_trigger dataflash_led_trigger = {
	.name     = led_dataflash_trig,
	.activate = dataflash_trig_activate,
	.deactivate = dataflash_trig_deactivate,
};

static int __init dataflash_trig_init(void)
{
	return led_trigger_register(&dataflash_led_trigger);
}

static void __exit dataflash_trig_exit(void)
{
	led_trigger_unregister(&dataflash_led_trigger);
}

module_init(dataflash_trig_init);
module_exit(dataflash_trig_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Data Flash LED trigger");
MODULE_LICENSE("GPL");
