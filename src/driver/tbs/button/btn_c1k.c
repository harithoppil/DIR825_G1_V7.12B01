/**
 * TBS_TAG: ZhangYu 2009-10-14
 * Desc:    Mindspeed C1000方案的按钮驱动
 * 
 **/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/resource.h>
#include <asm/types.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/hardware.h>

#include <btn.h>


//#define BTN_DEBUG

#ifdef BTN_DEBUG
    #define BTN_TRACE printk
#else
    #define BTN_TRACE(str, args...)  do { ; } while(0);
#endif

extern struct btn_dev c1k_btns[];  /* 定义在product.c中 */


/* 核心驱动通过此函数获得按钮状态 */
btn_status c1k_btn_get_status(struct btn_dev *btn)
{
	unsigned int ret;

	ret = comcerto_gpio_read(comcerto_gpio_mask(btn->gpio));
	

	/* 如果是低电平触发 */
	if(btn->level == BTN_LEVEL_LOW){
		if(ret == 0)
		{
			BTN_TRACE("button %d is on sate down\n", btn->name);
			return BTN_DOWN;
		}
		else
		{
			BTN_TRACE("button %d is on sate up\n", btn->name);
			return BTN_UP;
		}
	}
	else{   /* 如果是高电平触发 */
		if(ret == 0)
			return BTN_UP;
		else
			return BTN_DOWN;
	}

}

static int __init c1k_btn_init(void)
{
	int ret;
	int i;
	
	for(i=0;c1k_btns[i].name != btn_end; i++){

		/* 初始化GPIO接口的工作状态 */
		comcerto_gpio_disable_output(comcerto_gpio_mask(c1k_btns[i].gpio));


		/* 初始各个BTN设备的初始状态 */
		c1k_btns[i].get_status = c1k_btn_get_status;

		ret = btn_dev_register(&c1k_btns[i]);
		
		if (ret != 0) {
			printk (KERN_ERR "Unable to register btn_dev %d(error %d)\n", c1k_btns[i].name, ret);
			return -1;
		}
	
	}

	return 0;

}


static void __exit c1k_btn_exit(void)
{
	/* do nothing */
}

module_init(c1k_btn_init);
module_exit(c1k_btn_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Mindspeed C1000 buttons driver");
MODULE_LICENSE("GPL");
