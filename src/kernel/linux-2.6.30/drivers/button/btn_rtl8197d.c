/**
 * 文件名: btn_rtl8672.c
 * 说  明: RTL8197d方案的按钮驱动
 *
 * 作  者: Kuang Suwen
 **/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/resource.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <asm/types.h>
#include <asm/irq.h>

#include <btn.h>
#include "bsp/bspchip.h"

#ifdef BTN_DEBUG
    #define BTN_TRACE(str, args...) printk(str, ##args)
#else
    #define BTN_TRACE(str, args...)  do { ; } while(0);
#endif


/* RTL8672原芯片厂商的GPIO驱动接口*/
extern void gpioConfig (int gpio_num, int gpio_func);
extern void gpioConfigCNR(int gpio_num, int mode);
extern void gpioConfigIntr(int enable);
extern int  gpioGetEdge(int gpio_num);
extern void gpioConfigEdge(int gpio_num, int mode);
extern void gpioClearIntr(int gpio_num);
extern int  gpioGetIntrStatus(int gpio_num);
extern void gpioSet(int gpio_num);
extern void gpioClear(int gpio_num);
extern int  gpioRead(int gpio_num);


/* 定义在rtl8197d_product.c中 */
extern struct btn_dev rtl8197d_btns[];



#if 0//def CONFIG_BUTTON_TRIGGER_TIMER
/*=======================================================================
* 使用定时器轮询检查按钮状态
=======================================================================*/

/* 核心驱动通过此函数获得按钮状态 */
static btn_status rtl8197d_get_status(struct btn_dev *btn)
{
	int ret;

	ret =  gpioRead(btn->gpio);

	BTN_TRACE("button %d is on sattus %d\n", btn->name, ret);

	/* 如果是低电平触发 */
	if(btn->level == BTN_LEVEL_LOW)
	{
		if(ret == 0)
			return BTN_DOWN;
		else
			return BTN_UP;
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

static int __init rtl8197d_btn_init(void)
{
	int ret;
	int i;

	for(i=0;rtl8197d_btns[i].name != btn_end; i++)
	{
		/*设置为GPIO模式*/
		gpioConfigCNR(rtl8197d_btns[i].gpio, 0);

		/* 初始化GPIO接口的工作状态，设置为输入模式，避免误触发 */
		gpioConfig(rtl8197d_btns[i].gpio, GPIO_FUNC_INPUT);

		/* 设置获取按钮状态句柄 */
		rtl8197d_btns[i].get_status = rtl8197d_get_status;

		ret = btn_dev_register(&rtl8197d_btns[i]);
		if (ret != 0) {
			printk (KERN_ERR "Unable to register btn_dev %d(error %d)\n", rtl8197d_btns[i].name, ret);
			return -1;
		}
	}

	return 0;
}

static void __exit rtl8197d_btn_exit(void)
{
    int i;

    /* 注销所有的btn_dev */
    for(i=0;rtl8197d_btns[i].name != btn_end; i++)
	{
		btn_dev_unregister(&rtl8197d_btns[i]);
	}
}


#else


/************************************************************************
* 使用中断模式检测按钮状态
************************************************************************/

void rtl8197d_button_irq(unsigned long data)
{
    struct btn_dev *btn = (struct btn_dev *)data;
    int state = gpioRead(btn->gpio);

    BTN_TRACE("1:GIMR=0x%x, GPIO_PABCD_CNR=0x%x, GPIO_PABCD_PTYPE=0x%x, GPIO_PABCD_DIR=0x%x\n"
              "GPIO_PABCD_DAT=0x%x, GPIO_PABCD_ISR=0x%x, GPIO_PAB_IMR=0x%x, GPIO_PCD_IMR=0x%x\n"
              "gpio(%d)=%d\n",
              REG32(BSP_GIMR), REG32(GPIO_PABCD_CNR), REG32(GPIO_PABCD_PTYPE), REG32(GPIO_PABCD_DIR),
              REG32(GPIO_PABCD_DAT), REG32(GPIO_PABCD_ISR), REG32(GPIO_PAB_IMR), REG32(GPIO_PCD_IMR),
              btn->gpio, gpioRead(btn->gpio)
    );

    if ((btn->level == BTN_LEVEL_LOW && !state)
        || (btn->level == BTN_LEVEL_HIGH && state))
    {
        #if 0
        if (btn->level == BTN_LEVEL_LOW)
        {
            gpioConfigEdge(btn->gpio, GPIO_EDGE_RISE);
        }
        else
        {
            gpioConfigEdge(btn->gpio, GPIO_EDGE_FALL);
        }
        #endif

        btn->cur_status =  BTN_DOWN;
		printk("button %d down, gpio=%d\n", btn->name, btn->gpio);
		btn_status_query(btn);
    }
    else if ((btn->level == BTN_LEVEL_LOW && state)
        || (btn->level == BTN_LEVEL_HIGH && !state))
    {
        #if 0
        if (btn->level == BTN_LEVEL_LOW)
        {
            gpioConfigEdge(btn->gpio, GPIO_EDGE_FALL);
        }
        else
        {
            gpioConfigEdge(btn->gpio, GPIO_EDGE_RISE);
        }
        #endif

		btn->cur_status = BTN_UP;
		printk("button %d up, gpio=%d\n", btn->name, btn->gpio);
    }

    /* 重新开启GPIO_ABCD中断 */
    gpioConfigIntr(1);

}

void rtl8197d_button_irq1(unsigned long data)
{
    struct btn_dev *btn = (struct btn_dev *)data;
    int state = gpioRead(btn->gpio);


    if ((btn->level == BTN_LEVEL_LOW && !state)
        || (btn->level == BTN_LEVEL_HIGH && state))
    {
        #if 0
        if (btn->level == BTN_LEVEL_LOW)
        {
            gpioConfigEdge(btn->gpio, GPIO_EDGE_RISE);
        }
        else
        {
            gpioConfigEdge(btn->gpio, GPIO_EDGE_FALL);
        }
        #endif

        btn->cur_status =  BTN_DOWN;
		printk("button %d down, gpio=%d\n", btn->name, btn->gpio);
		btn_status_query(btn);
    }
    else if ((btn->level == BTN_LEVEL_LOW && state)
        || (btn->level == BTN_LEVEL_HIGH && !state))
    {
        #if 0
        if (btn->level == BTN_LEVEL_LOW)
        {
            gpioConfigEdge(btn->gpio, GPIO_EDGE_FALL);
        }
        else
        {
            gpioConfigEdge(btn->gpio, GPIO_EDGE_RISE);
        }
        #endif

		btn->cur_status = BTN_UP;
		printk("button %d up, gpio=%d\n", btn->name, btn->gpio);
    }

    /* 重新开启GPIO_ABCD中断 */

	gpioConfigIntr1(1);
}

static irqreturn_t rtl8197d_gpio_irq_dispatch(int cpl, void *dev_id)
{
	int i;

    /* 关闭GPIO_ABCD中断，防止重入 */
    gpioConfigIntr(0);
	//printk("rtl8197d_gpio_irq_dispatch() dev_id=%d\n",(int)dev_id);
	if(((int)dev_id == GPIO_ABCD_IRQ))
	{
		for(i=0;rtl8197d_btns[i].name != btn_end;i++)
		{
		    if(rtl8197d_btns[i].gpio < 32)
		    {
				if( gpioGetIntrStatus(rtl8197d_btns[i].gpio))
				{
	                          BTN_TRACE("2:GIMR=0x%x, GPIO_PABCD_CNR=0x%x, GPIO_PABCD_PTYPE=0x%x, GPIO_PABCD_DIR=0x%x\n"
	                          "GPIO_PABCD_DAT=0x%x, GPIO_PABCD_ISR=0x%x, GPIO_PAB_IMR=0x%x, GPIO_PCD_IMR=0x%x\n"
	                          "gpio(%d)=%d\n",
	                          REG32(BSP_GIMR), REG32(GPIO_PABCD_CNR), REG32(GPIO_PABCD_PTYPE), REG32(GPIO_PABCD_DIR),
	                          REG32(GPIO_PABCD_DAT), REG32(GPIO_PABCD_ISR), REG32(GPIO_PAB_IMR), REG32(GPIO_PCD_IMR),
	                          rtl8197d_btns[i].gpio, gpioRead(rtl8197d_btns[i].gpio)
	                          );

	                          gpioClearIntr(rtl8197d_btns[i].gpio);
	                          rtl8197d_button_irq((unsigned long)(&rtl8197d_btns[i]));  /* 对触发中断的GPIO进行处理 */
				}
		    }
		}

	}
	
	return IRQ_HANDLED;
}
static irqreturn_t rtl8197d_gpio_irq_dispatch1(int cpl, void *dev_id)
{
	int i;

    /* 关闭GPIO_ABCD中断，防止重入 */
	gpioConfigIntr1(0);
	//printk("rtl8197d_gpio_irq_dispatch1() dev_id=%d\n",(int)dev_id);
	if(((int)dev_id == GPIO_EFGH_IRQ))
	{
		for(i=0;rtl8197d_btns[i].name != btn_end;i++)
		{
		    if(rtl8197d_btns[i].gpio >31)
		    {
				if( gpioGetIntrStatus(rtl8197d_btns[i].gpio))
				{

	                          gpioClearIntr(rtl8197d_btns[i].gpio);
	                          rtl8197d_button_irq1((unsigned long)(&rtl8197d_btns[i]));  /* 对触发中断的GPIO进行处理 */
				}
		    }
		}

	}
	
	return IRQ_HANDLED;
}



/* 核心驱动通过此函数获得按钮状态 */
btn_status rtl8197d_get_status(struct btn_dev *btn)
{
	return btn->cur_status;
}



static int __init rtl8197d_btn_init(void)
{

	int i;
	int ret;

    BTN_TRACE("Before button init:\n"
        "GIMR=0x%x, GPIO_PABCD_CNR=0x%x, GPIO_PABCD_PTYPE=0x%x, GPIO_PABCD_DIR=0x%x\n"
        "GPIO_PABCD_DAT=0x%x, GPIO_PABCD_ISR=0x%x, GPIO_PAB_IMR=0x%x, GPIO_PCD_IMR=0x%x\n",
        REG32(BSP_GIMR), REG32(GPIO_PABCD_CNR), REG32(GPIO_PABCD_PTYPE), REG32(GPIO_PABCD_DIR),
        REG32(GPIO_PABCD_DAT), REG32(GPIO_PABCD_ISR), REG32(GPIO_PAB_IMR), REG32(GPIO_PCD_IMR)
    );
    
    /* 允许GPIO_ABCD触发中断 */
    gpioConfigIntr(1);
	/* 允许GPIO_EFGH触发中断 */
    gpioConfigIntr1(1);

	for(i=0;rtl8197d_btns[i].name != btn_end; i++)
    {
        /* 设置指定的GPIO为GPIO模式 */
		gpioConfigCNR(rtl8197d_btns[i].gpio, 0);
        printk("---rtl8197d_btns[%d].gpio-%d-\n",i,rtl8197d_btns[i].gpio);
		/* 设置指定的GPIO为输入模式 */
		gpioConfig(rtl8197d_btns[i].gpio, GPIO_FUNC_INPUT);

        #if 0
        /* 设置指定的GPIO的中断触发模式为按下时触发(下降沿触发) */
        gpioConfigEdge(rtl8197d_btns[i].gpio, GET_EDGE(rtl8197d_btns[i].level));
        #endif
        /*
        if(16 == rtl8197d_btns[i].gpio)
        {
            REG32(0xB8000040) |=0x3000;
		}
		*/
		/*JTAG引脚复用*/
        if(5 == rtl8197d_btns[i].gpio)
        {
//            REG32(0xB8000040) &= ~0x7;
            REG32(0xB8000040) |=0x6;
        }
        /*
        
		GPIOG_4 P0_RXD7_G4          52
		GPIOG_5 P0_RXD6_G5          53
		GPIOG_6 P0_RXD5_G6          54
		GPIOG_7 P0_RXD4_G7          55

		*/
		if((rtl8197d_btns[i].gpio>51) || (rtl8197d_btns[i].gpio<56))
        {
            //bit10-11 : 11  做GPIO使用
            REG32(0xB8000040) |=0xc00;
        }

		
        /* 设置指定的GPIO的中断触发模式为双向触发 */
        gpioConfigEdge(rtl8197d_btns[i].gpio, GPIO_EDGE_BOTH);

		/* 初始各个BTN设备的初始状态 */
		rtl8197d_btns[i].ignore = 0;
		rtl8197d_btns[i].cur_status = BTN_UP;
		rtl8197d_btns[i].get_status = rtl8197d_get_status;

	}

    BTN_TRACE("After button init:\n"
        "GIMR=0x%x, GPIO_PABCD_CNR=0x%x, GPIO_PABCD_PTYPE=0x%x, GPIO_PABCD_DIR=0x%x\n"
        "GPIO_PABCD_DAT=0x%x, GPIO_PABCD_ISR=0x%x, GPIO_PAB_IMR=0x%x, GPIO_PCD_IMR=0x%x\n",
        REG32(BSP_GIMR), REG32(GPIO_PABCD_CNR), REG32(GPIO_PABCD_PTYPE), REG32(GPIO_PABCD_DIR),
        REG32(GPIO_PABCD_DAT), REG32(GPIO_PABCD_ISR), REG32(GPIO_PAB_IMR), REG32(GPIO_PCD_IMR)
    );


	ret = request_irq (GPIO_ABCD_IRQ, rtl8197d_gpio_irq_dispatch, 0, "GPIO_ABCD", (void *)GPIO_ABCD_IRQ);
	if (ret != 0)
	{
		printk (KERN_ERR "unable to request IRQ for GPIO (error %d)\n", ret);
	}
	ret = request_irq (GPIO_EFGH_IRQ, rtl8197d_gpio_irq_dispatch1, 0, "GPIO_EFGH", (void *)GPIO_EFGH_IRQ);
	if (ret != 0)
	{
		printk (KERN_ERR "unable to request IRQ for GPIO (error %d)\n", ret);
	}

	return ret;
}


static void __exit rtl8197d_btn_exit(void)
{
	/* do nothing */
}
#endif


module_init(rtl8197d_btn_init);
module_exit(rtl8197d_btn_exit);

MODULE_AUTHOR("KuangSuwen");
MODULE_DESCRIPTION("Rtl8197d button driver");
MODULE_LICENSE("GPL");



