/*
 * 文件名:led.h
 * 说明:TBS LED驱动头文件
 * 
 * 作者:Zhang Yu
 *
 */
 
#ifndef __LINUX_LEDS_CORE_H_INCLUDED
#define __LINUX_LEDS_CORE_H_INCLUDED

#if 0
#ifdef __KERNEL__
#include <linux/kernel.h>
#endif
#include <gpio.h>
#endif

#ifdef CONFIG_LED_DEBUG
	#ifdef _LINUX_KERNEL_H
		#define leddebug(fmt, arg...)		 printk(fmt, arg...)
	#else
		#define leddebug(fmt, arg...)		 printf(fmt, arg...)
	#endif
#else
	#define leddebug(fmt, arg...)            do{}while(0)
#endif

#define LED_NAME_MAX	32
#define TRIG_NAME_MAX	32
#define LED_TRIG_TTL_MAX             0x7FFFFFFF


typedef enum  {
	led_power	= 0,
	led_power_green,	/* 1 */
	led_power_red,
	led_internet,
	led_internet_green,
	led_internet_red,	/* 5 */
	led_usb,
	led_wlan,
	led_wps,
	led_wps_green,
	led_wps_red,		/* 10 */
	led_wps_yellow,
	led_dsl,
	led_register,
	led_phone_1,
	led_phone_2,		/* 15 */
	led_line,
	led_prov_alm,
	led_lan_1,
	led_lan_2,
	led_lan_3,			/* 20 */
	led_lan_4,
	led_lan_5,
	led_end,		 	/* 数组结束 */
}led_name;

typedef enum  {
	led_off_trig	= 0,
	led_on_trig,
	led_flash_trig,
	led_flash_4hz_trig,
	led_dataflash_trig,
	led_wps_inprogress_trig, /* 5 */
	led_wps_error_trig,
	led_wps_overlap_trig,
	led_wps_success_trig,
	led_blinking_trig,
	led_end_trig, /* 10 */
}trig_name;

typedef enum  {
	LED_GPIO_LOW  = 0,
	LED_GPIO_HIGH,
}led_level;


typedef enum  {
	LED_BOOT_OFF  = 0,
	LED_BOOT_ON,
}led_boot_default;

//gxw / 2014-10-16 / Dlink INTERNET LED CONTROL
typedef enum  {
	LED_PROT_IPV4 = 0,
	LED_PROT_IPV6,
	LED_PROT_END,
}led_prot_type;

typedef enum  {
	LED_CONN_FIN = 0,
	LED_CONN_SET,
	LED_CONN_DEL,
	LED_CONN_END,
}led_conn_type;

struct led_dev {
	led_name    name; 
	int	        gpio;    /* GPIO号 */
	led_level   level;   /* 高低电平触发 */
	trig_name   kernel_default; /* kernel默认状态 */
	led_boot_default boot_default; /* Bootloader默认状态 */
	struct led_trigger *cur_trig;  /* 当前trigger */
	void	*trigger_data;
};

struct led_trigger {
	trig_name name;
	void	(*activate)(struct led_dev *led_cdev);
	void	(*deactivate)(struct led_dev *led_cdev);
};


struct led_hw_handler {
	void	(*led_on)(struct led_dev *led_cdev);
	void	(*led_off)(struct led_dev *led_cdev);
};

#ifdef LED_MULTIPLEXED
extern led_name multiple_use_led;	/* multiple-use led name, such as led_wlan */
#endif

extern struct led_hw_handler* globe_led_hw_handler;   /* LED点灯句柄，全局使用 */

int led_dev_register(struct led_dev *led);
int led_dev_unregister(struct led_dev *led);

int led_trigger_register(struct led_trigger *trig);
int led_trigger_unregister(struct led_trigger *trig);

int led_hw_handle_register(struct led_hw_handler *handler);
int led_hw_handle_unregister(struct led_hw_handler *handler);

int led_trigger_set(led_name led, trig_name trigger);
int led_wps_set(trig_name trigger);

trig_name led_trigger_get(led_name led);
void led_set_on(struct led_dev *led);
void led_set_off(struct led_dev *led);



/* ioctl led 参数定义 */

struct tbs_ioctl_led_parms
{
	led_name led;
	trig_name trig;
};

#endif		/* __LINUX_LEDS_CORE_H_INCLUDED */
