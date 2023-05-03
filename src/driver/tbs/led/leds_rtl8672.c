/**********************************************************************
 Copyright (c), 1991-2009, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 �ļ����� : led_rtl8672.c
 �ļ����� : ���ļ�ΪRTL8672����LED��������

 �޶���¼ :
          1 ���� : ������
            ���� : 2009-07-24
            ���� : ����
 
**********************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <led.h>
#include <bspchip.h>


//#define LED_DEBUG
#ifdef LED_DEBUG
    #define LED_TRACE printk
#else
    #define LED_TRACE(str, args...)  do { ; } while(0);
#endif


/*port function definition*/
#define GPIO_FUNC_INPUT 	    0x0001  /*data input*/
#define GPIO_FUNC_OUTPUT 	    0x0002	/*data output*/
#define GPIO_FUNC_IRQ_FALL 	    0x0003	/*falling edge IRQ*/
#define GPIO_FUNC_IRQ_RISE 	    0x0004	/*rising edge IRQ*/
#define GPIO_FUNC_IRQ_LEVEL 	0x0005	/*level trigger IRQ*/


/* RTL8672ԭоƬ���̵�GPIO�����ӿ�*/
extern void gpioConfig (int gpio_num, int gpio_func);
extern void gpioConfigCNR(int gpio_num, int mode);
extern void gpioSet(int gpio_num);
extern void gpioClear(int gpio_num);

/* ������product.c�� */
extern struct led_dev rtl8672_leds[];


void rtl8672_led_on(struct led_dev *led)
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


void rtl8672_led_off(struct led_dev *led)
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

#if 0
/***************************************************
*
*    �ṩproc�ӿڲ���gpio
*
***************************************************/
struct proc_dir_entry *proc_gpio;
static int gpio_proc_write( struct file *filp, const char __user *buf,unsigned long len, void *data )
{
	int ret;
	char str_buf[256];
	int gpio = led_end;
	int val = 0;
	int i;

	if(len > 255)
	{
		printk("Error. Sample: echo 0 1 > /proc/gpio \n");
		return len;
	}

	copy_from_user(str_buf,buf,len);
	str_buf[len] = '\0';

	for(i=0;i<len;i++)
	{
		if(str_buf[i] < 0x30 || str_buf[i] > 0x39)
		{
			if(str_buf[i] != 0x20 && str_buf[i] != 0x0a)
			{
				printk("error cmd:%s\n",str_buf);
				return len;
			}
		}
	}

	ret = sscanf(str_buf,"%d %d", (int*)&gpio, (int*)&val);
	if(ret == -1 || gpio < 0 || (val != 0 && val != 1))
	{
		printk("Error.Sample: echo 0 1 > /proc/gpio \n");
		return len;
	}

	//ioMode(gpio);
    gpioConfig(gpio, GPIO_FUNC_OUTPUT);
    if (val == 1)
    {
        gpioSet(gpio);
    }
    else
    {
        gpioClear(gpio);
    }
    LED_TRACE("gpio(%d) set to %d.\n", gpio, val);

	return len;
}
#endif

struct led_hw_handler rtl8672_hw_handler =
{
	.led_on = rtl8672_led_on,
	.led_off = rtl8672_led_off,
};


static int __init rtl8672_led_init(void)
{
	int i;
	int ret;

	/* ��handlerע�ᵽLED CORE��ע��:Ҫ��ע��hanlder����ע��LED��*/
	ret = led_hw_handle_register(&rtl8672_hw_handler);
	if(ret<0){
		printk(KERN_ERR "Error:fail to register rtl8672_hw_handler.\n");
	}

    /* Enable GPIO D2, D3, D4, D6 */
    if( IS_6028B || IS_6085 ) {
		REG32(MISC_PINOCR) |= 0x000c0000; 
	}
    
	/* ��LEDע�ᵽLED CORE */
	for(i=0; rtl8672_leds[i].name != led_end;i++)
    {
        
        /*����ΪGPIOģʽ*/
		gpioConfigCNR(rtl8672_leds[i].gpio, 0);
		
		/* ��ʼ��GPIO�ӿڵĹ���״̬������Ϊ���ģʽ */
		gpioConfig(rtl8672_leds[i].gpio, GPIO_FUNC_OUTPUT);
        
		ret = led_dev_register(&rtl8672_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to register rtl8672_leds[%d].\n",i);
			return -1;
		}
	}

    #if 0
    /* ����gpio�ӿ� */
    proc_gpio = create_proc_entry( "gpio", 0644, NULL);
	proc_gpio->write_proc  = gpio_proc_write;
    #endif

	return 0;
}

static void __exit rtl8672_led_exit(void)
{
	int i;
	int ret;

    #if 0
    /* �Ƴ�gpio�ӿ� */
    remove_proc_entry("gpio", proc_gpio);
    #endif

	/* ��ע��LED����ע��handler */
	for(i=0; rtl8672_leds[i].name != led_end;i++){
		ret = led_dev_unregister(&rtl8672_leds[i]);
		if(ret<0){
			printk(KERN_ERR "Error:fail to unregister rtl8672_leds[%d]\n",i);
		}
	}
	
	ret = led_hw_handle_unregister(&rtl8672_hw_handler);

	if(ret<0){
		printk(KERN_ERR "Error:fail to unregister rtl8672_hw_handler.\n");
	}
	
}



module_init(rtl8672_led_init);
module_exit(rtl8672_led_exit);

MODULE_AUTHOR("kuangsuwen");
MODULE_DESCRIPTION("rtl8672 led driver");
MODULE_LICENSE("GPL");


