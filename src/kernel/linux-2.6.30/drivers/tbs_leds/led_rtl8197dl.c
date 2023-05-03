/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : led_rtl8672.c
 �ļ����� : ���ļ�ΪRTL8197d����LED��������

 �޶���¼ :
          1 ���� : ������
            ���� : 2010-07-14
            ���� : ����

**********************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <led.h>

#include "bsp/bspchip.h"


//#define LED_DEBUG
#ifdef LED_DEBUG
    #define LED_TRACE(str, args...) printk(str, ##args)
#else
    #define LED_TRACE(str, args...)  do { ; } while(0);
#endif



/* RTL8672ԭоƬ���̵�GPIO�����ӿ�*/
extern void gpioConfig (int gpio_num, int gpio_func);
extern void gpioConfigCNR(int gpio_num, int mode);
extern void gpioSet(int gpio_num);
extern void gpioClear(int gpio_num);

/* ������product.c�� */
extern struct led_dev rtl8197d_leds[];

void rtl8197d_led_on(struct led_dev *led)
{
    LED_TRACE("LED_%d(gpio:%d) is On.\n", led->name, led->gpio);
    gpioConfig(led->gpio, GPIO_FUNC_OUTPUT);

	if(led->level == LED_GPIO_HIGH){
		gpioSet(led->gpio);
	}
	else{
		gpioClear(led->gpio);
	}
}


void rtl8197d_led_off(struct led_dev *led)
{
    LED_TRACE("LED_%d(gpio:%d) is Off.\n", led->name, led->gpio);
	gpioConfig(led->gpio, GPIO_FUNC_OUTPUT);

	if(led->level == LED_GPIO_HIGH){
		gpioClear(led->gpio);
	}
	else{
		gpioSet(led->gpio);
	}
}

struct led_hw_handler rtl8197d_hw_handler =
{
	.led_on = rtl8197d_led_on,
	.led_off = rtl8197d_led_off,
};


static int __init rtl8197d_led_init(void)
{
	int i;    
	int ret;
    
    //printk("%s, %d start! \n", __FUNCTION__, __LINE__);

	/* ��handlerע�ᵽLED CORE��ע��:Ҫ��ע��hanlder����ע��LED��*/
	ret = led_hw_handle_register(&rtl8197d_hw_handler);
	if(ret<0){
		printk(KERN_ERR "Error:fail to register rtl8197d_hw_handler.\n");
	}

    #if 0
    /* Enable GPIO D2, D3, D4, D6 */
    if( IS_6028B || IS_6085 ) {
		REG32(MISC_PINOCR) |= 0x000c0000;
	}
    #endif
#ifdef CONFIG_WLAN_RSSI_LED /* PIN_MUX_SEL_2*/	
	REG32(0xB8000044) &= ~0x3;
	REG32(0xB8000044) |= 0x3;
	REG32(0xB8000044) &= ~0x18;
	REG32(0xB8000044) |= 0x18;
	REG32(0xB8000044) &= ~0xc0;
	REG32(0xB8000044) |= 0xc0;
	REG32(0xB8000044) &= ~0x600;
	REG32(0xB8000044) |= 0x600;
	REG32(0xB8000044) &= ~0x1c0000;
	REG32(0xB8000044) |= 0x100000;
#endif
    
    //REG32(0xB8000044)  : share pin register    
    //REG32(0xB8000040)  : share pin register
	/* ��LEDע�ᵽLED CORE */
	for(i = 0; rtl8197d_leds[i].name != led_end; i++)
    {
        /*//shared pin:
                CFG_LED_GPIO_WPS = 6, //GPIOA6
                CFG_LED_GPIO_USB = 8, //GPIO B0       
                CFG_LED_GPIO_LAN1 = 10, //GPIO B2  
                CFG_LED_GPIO_LAN2 = 11, //GPIO B3 
                CFG_LED_GPIO_LAN3 = 12, //GPIO B4  
                CFG_LED_GPIO_LAN4 = 13, //GPIO B5  
                CFG_LED_GPIO_WAN = 14 //GPIO B6   
            */
        switch (rtl8197d_leds[i].gpio)
        {
            case 6://configure jtag pins as gpio mode
                /*1. keep precede bit.   2. change mode of GPIO A6 */
    			//REG32(0xB8000040) &= ~0x6;
    			REG32(0xB8000040) |= 0x6; 
                break;
                
            case 8://configure rxd/ txd pins as gpio mode
    			//REG32(0xB8000040) &= ~0x20; 
    			REG32(0xB8000040) |= 0x20;
                break;

            case 10:
    			REG32(0xB8000044) |= 0x3;
                break;

            case 11:
    			REG32(0xB8000044) |= 0x18;
                break;

            case 12:
    			REG32(0xB8000044) |= 0xC0;
                break;
                
            case 13:
    			REG32(0xB8000044) |= 0x600;
                break;
                
            case 14:
    			REG32(0xB8000044) |= 0x3000;
                break;

            default:
                printk(KERN_ERR "Error:fail to config rtl8197d_leds[%d], unknowed pin.\n",i);
                break;
        }

        //printk("setting: rtl8197d_leds[i].name= %d, rtl8197d_leds[i].gpio= %d, index= %x \n", 
        //        rtl8197d_leds[i].name, rtl8197d_leds[i].gpio, i);

        /*����ΪGPIOģʽ0-as gpio*/
		gpioConfigCNR(rtl8197d_leds[i].gpio, 0);

		/* ��ʼ��GPIO�ӿڵĹ���״̬������Ϊ���ģʽ  in pdf:  0-input 1-output !!!*/
		gpioConfig(rtl8197d_leds[i].gpio, GPIO_FUNC_OUTPUT);

		ret = led_dev_register(&rtl8197d_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to register rtl8197d_leds[%d].\n",i);
			return -1;
		}
#ifdef CONFIG_WLAN_RSSI_LED	  /* ��ʼ����λ�ر�*/	
		rtl8197d_led_off(&rtl8197d_leds[i]);
#endif
	}

    #if 0
    /* ����gpio�ӿ� */
    proc_gpio = create_proc_entry( "gpio", 0644, NULL);
	proc_gpio->write_proc  = gpio_proc_write;
    #endif

    //printk("%s, %d, end! \n", __FUNCTION__, __LINE__);

	return 0;
}

static void __exit rtl8197d_led_exit(void)
{
	int i;
	int ret;

    #if 0
    /* �Ƴ�gpio�ӿ� */
    remove_proc_entry("gpio", proc_gpio);
    #endif

	/* ��ע��LED����ע��handler */
	for(i=0; rtl8197d_leds[i].name != led_end;i++){
		ret = led_dev_unregister(&rtl8197d_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to unregister rtl8197d_leds[%d]\n",i);
		}
	}

	ret = led_hw_handle_unregister(&rtl8197d_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to unregister rtl8197d_hw_handler.\n");
	}

}



module_init(rtl8197d_led_init);
module_exit(rtl8197d_led_exit);

MODULE_AUTHOR("Kuangsuwen");
MODULE_DESCRIPTION("Rtl8197d led driver");
MODULE_LICENSE("GPL");

