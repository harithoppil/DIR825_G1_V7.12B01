/**
 * 文件名: btn_mv88f6560p.c
 * 说  明: MV88F6560P方案的按钮驱动
 *
 * 作  者: fuhuoping
 **/
#include <linux/kernel.h>
#include <linux/init.h>
//#include <linux/config.h>  /*fuhuoping 编译时找不到该头文件*/
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
/* 定义在MARVELL_88F65XX/product.c中 */
extern struct btn_dev product_btns[];


extern int marvell_gpio_status_read(unsigned gpioNum);
extern int marvell_gpio_mode_input(unsigned gpioNum);
#define BTN_TEST

int GetGpioState(int gpio)
{
	unsigned long gpio_val=0;
    marvell_gpio_mode_input(gpio);
	gpio_val = marvell_gpio_status_read(gpio);

	return(gpio_val);
}

#ifdef BUTTON_TRIGGER_TIMER
/*=======================================================================
* 使用定时器轮询检查按钮状态
=======================================================================*/

/* 核心驱动通过此函数获得按钮状态 */
static btn_status mv88f6560p_get_status(struct btn_dev *btn)
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

static int __init mv88f6560p_btn_init(void)
{
	int ret;
	int i;

	for(i=0;product_btns[i].name != btn_end; i++)
	{
		/*设置为GPIO模式*/

		/* 初始化GPIO接口的工作状态，设置为输入模式，避免误触发 */

		/* 设置获取按钮状态句柄 */
		product_btns[i].get_status = mv88f6560p_get_status;

		ret = btn_dev_register(&product_btns[i]);
		if (ret != 0) {
			printk (KERN_ERR "Unable to register btn_dev %d(error %d)\n", product_btns[i].name, ret);
			return -1;
		}
	}
	return 0;
}

static void __exit mv88f6560p_btn_exit(void)
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

void mv88f6560p_button_irq(unsigned long data)
{
	return 0;
}


irqreturn_t mv88f6560p_gpio_irq_dispatch(int cpl, void *dev_id)
{
	return IRQ_HANDLED;
}


/* 核心驱动通过此函数获得按钮状态 */
btn_status mv88f6560p_get_status(struct btn_dev *btn)
{
	return btn->cur_status;
}

static int __init mv88f6560p_btn_init(void)
{
	return 0;
}


static void __exit mv88f6560p_btn_exit(void)
{
	/* do nothing */
}
#endif

#if 1

int btn_dev_init(void)
{
	return mv88f6560p_btn_init();
}
void btn_dev_exit(void)
{
	mv88f6560p_btn_exit();
}

#else
module_init(mv88f6560p_btn_init);
module_exit(mv88f6560p_btn_exit);

MODULE_AUTHOR("Chenzanhui");
MODULE_DESCRIPTION("MV88f6560P button driver");
MODULE_LICENSE("GPL");
#endif


