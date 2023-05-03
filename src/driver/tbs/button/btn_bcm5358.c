/**
 * 文件名: btn_bcm5358.c
 * 说  明: BCM5358方案的按钮驱动
 *
 * 作  者: chenzanhui
 **/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/timer.h>
#include <asm/types.h>
#include <asm/irq.h>
#include <btn.h>


//#define BTN_DEBUG
#ifdef BTN_DEBUG
    #define BTN_TRACE printk
#else
    #define BTN_TRACE(str, args...)  do { ; } while(0);
#endif
/* 定义在bcm5358_product.c中 */
extern struct btn_dev product_btns[];


extern unsigned long get_gpio(void);
#define BTN_TEST

int GetGpioState(int gpio)
{
	unsigned long gpio_val=0;

	gpio_val = get_gpio();

	return (gpio_val &  1 << gpio) == 0 ? 0 : 1;
}

#ifdef BUTTON_TRIGGER_TIMER
/*=======================================================================
* 使用定时器轮询检查按钮状态
=======================================================================*/

/* 核心驱动通过此函数获得按钮状态 */
static btn_status bcm5358_get_status(struct btn_dev *btn)
{
	int ret;

	ret =  GetGpioState(btn->gpio);

	BTN_TRACE("button %d is on sattus %d\n", btn->name, ret);

	/* 如果是低电平触发 */
	if(btn->level == BTN_LEVEL_LOW)
	{
		if(ret == 0)
		{
			return BTN_DOWN;
		}
		else
		{
			return BTN_UP;
		}
	}
    /* 如果是高电平触发 */
	else
	{
		if(ret == 0)
			return BTN_UP;
		else
			return BTN_DOWN;
	}

}

static int __init bcm5358_btn_init(void)
{
	int ret;
	int i;

	for(i=0;product_btns[i].name != btn_end; i++)
	{
		/*设置为GPIO模式*/

		/* 初始化GPIO接口的工作状态，设置为输入模式，避免误触发 */

		/* 设置获取按钮状态句柄 */
		product_btns[i].get_status = bcm5358_get_status;

		ret = btn_dev_register(&product_btns[i]);
		if (ret != 0) {
			printk (KERN_ERR "Unable to register btn_dev %d(error %d)\n", product_btns[i].name, ret);
			return -1;
		}
	}
	return 0;
}

static void __exit bcm5358_btn_exit(void)
{
	int i;

    /* 注销所有的btn_dev */
    for(i=0;product_btns[i].name != btn_end; i++)
	{
		btn_dev_unregister(&product_btns[i]);
	}
}


#else

/************************************************************************
* 使用中断模式检测按钮状态
************************************************************************/

void bcm5358_button_irq(unsigned long data)
{
	return 0;
}


irqreturn_t bcm5358_gpio_irq_dispatch(int cpl, void *dev_id)
{
	return IRQ_HANDLED;
}


/* 核心驱动通过此函数获得按钮状态 */
btn_status bcm5358_get_status(struct btn_dev *btn)
{
	return btn->cur_status;
}

static int __init bcm5358_btn_init(void)
{
	return 0;
}


static void __exit bcm5358_btn_exit(void)
{
	/* do nothing */
}
#endif

#if 1

int btn_dev_init(void)
{
	return bcm5358_btn_init();
}
void btn_dev_exit(void)
{
	bcm5358_btn_exit();
}

#else
module_init(bcm5358_btn_init);
module_exit(bcm5358_btn_exit);

MODULE_AUTHOR("Chenzanhui");
MODULE_DESCRIPTION("BCM5358 button driver");
MODULE_LICENSE("GPL");
#endif


