/*
 * LED DataFlash Trigger
 *
 * ���ļ�ʵ�����ݵ���˸����
 * ˵����������ʵ����0.1�룬��0.1�룬��ѭ��������һ����˸һ�Σ����Ը�Ƶ�ʵ��á�
 *       ʵ��ԭ������Ƶ�������²�����/�ͷ��ڴ棬����5���Ӻ������ͷ���Դ�Ķ�����
 *
 * ע�⣺һ������£�ֻ�������ݵ�����Ϊ�����񡣲���������ݵ����ù����������
 *       ���Ҫ��led->trigger_data���㡣
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
		dataflash_data->phase = 3;  /* 3--���δ������������ */
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

	if(led->trigger_data == NULL) 	/* dataflash_trig_data �Ѿ�������� */
	{
		dataflash_data = kzalloc(sizeof(*dataflash_data), GFP_KERNEL);
		if (!dataflash_data)
			return;

		led->trigger_data = dataflash_data;

		/* ��˸��ʱ������ */
		setup_timer(&dataflash_data->flash_timer,
			    led_dataflash_function, (unsigned long) led);
	
		dataflash_data->phase = 0;
		dataflash_data->flash_period = msecs_to_jiffies(50);  /* ��˸����50ms */
		dataflash_data->release_period = msecs_to_jiffies(5000);  /* ��������5000ms */
		led_dataflash_function(dataflash_data->flash_timer.data);

		/* ����dataflash_trig_data�Ķ�ʱ������ */
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
	

/* ��ʱ�䲻�������ݵƺ��Զ��ͷ�data_flash_data��һ�и�ԭ��*/
static void auto_dataflash_trig_deactivate(unsigned long data) 
{
	struct dataflash_trig_data *dataflash_data = (struct dataflash_trig_data *)data; 
	struct led_dev *led;

	if (!dataflash_data)
		return;

	led = dataflash_data->led;

	if(led->trigger_data == dataflash_data) /* �������û�б�����trig�޸� */
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
