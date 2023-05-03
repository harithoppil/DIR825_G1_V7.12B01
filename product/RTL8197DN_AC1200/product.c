/*
 * File name: product.c 
 * 
 * Note: for RTL8197D configuration file
 *  
 * Author: 2016-1-12 taogan
 */

#ifndef __LINUX_RTL8197D_PRODUCT_H_INCLUDED
#define __LINUX_RTL8197D_PRODUCT_H_INCLUDED

#include <led.h>
#include <btn.h>


/* 定义LED参数 */
#define CFG_LED_GPIO_WPS			6//4
#define CFG_LED_GPIO_USB			11 //GPIO B3
#define CFG_LED_GPIO_USB1			12 //GPIO B4


#define CFG_LED_LEVEL_WPS			LED_GPIO_LOW
#define CFG_LED_LEVEL_USB			LED_GPIO_LOW

#define CFG_LED_DEFAULT_KERNEL_WPS			led_off_trig
#define CFG_LED_DEFAULT_KERNEL_USB			led_off_trig

#define CFG_LED_DEFAULT_BOOT_WPS			LED_BOOT_OFF
#define CFG_LED_DEFAULT_BOOT_USB			LED_BOOT_OFF

struct led_dev rtl8197d_leds[] = 
{
	{led_wps, CFG_LED_GPIO_WPS, CFG_LED_LEVEL_WPS, CFG_LED_DEFAULT_KERNEL_WPS, CFG_LED_DEFAULT_BOOT_WPS, 0, 0},
	{led_usb, CFG_LED_GPIO_USB1, CFG_LED_LEVEL_USB, CFG_LED_DEFAULT_KERNEL_USB, CFG_LED_DEFAULT_BOOT_USB, 0, 0},
	{led_end, 0, LED_GPIO_HIGH, LED_GPIO_HIGH, LED_BOOT_OFF, 0, 0},
};





/* 定义按钮参数 */
#define CFG_BTN_GPIO_RESET		5
#define CFG_BTN_GPIO_WPS		3
/* GPIO 号
0   1   2   3   4   5   6   7    GPIOA 0-7
8	9	10	11	12	13	14	15   GPIOB 0-7
16	17	18	19	20	21	22	23   GPIOC 0-7
24	25	26	27	28	29	30	31   GPIOD 0-7
32	33	34	35	36	37	38	39   GPIOE 0-7
40	41	42	43	44	45	46	47   GPIOF 0-7
48	49	50	51	52	53	54	55   GPIOG 0-7
56	57	58	59	60	61	62	63   GPIOH 0-7
*/


#define CFG_BTN_GPIO_G5_WLAN	53


#define CFG_BTN_LEVEL_RESET		BTN_LEVEL_LOW
#define CFG_BTN_LEVEL_WPS		BTN_LEVEL_LOW


#define CFG_BTN_LEVEL_WLAN		BTN_LEVEL_LOW



struct btn_dev rtl8197d_btns[] = 
{
	{
		.name  = btn_reset,
		.gpio  = CFG_BTN_GPIO_RESET,
		.level = CFG_BTN_LEVEL_RESET,
	},

	{
		.name  = btn_wps,
		.gpio  = CFG_BTN_GPIO_WPS,
		.level = CFG_BTN_LEVEL_WPS,
	},

	{
		.name  = btn_wlan,
		.gpio  = CFG_BTN_GPIO_G5_WLAN,
		.level = CFG_BTN_LEVEL_WLAN,
	},


	{
		.name  = btn_end,
	},
};


#endif /* __LINUX_RTL8197D_PRODUCT_H_INCLUDED */

