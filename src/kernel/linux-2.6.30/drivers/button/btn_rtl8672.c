/**
 * �ļ���: btn_rtl8672.c
 * ˵  ��: RTL8672�����İ�ť����
 *
 * ��  ��: Kuang Suwen
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
#include "platform.h"


//#define BTN_DEBUG
#ifdef BTN_DEBUG
    #define BTN_TRACE printk
#else
    #define BTN_TRACE(str, args...)  do { ; } while(0);
#endif

/* port function definition */
#define GPIO_FUNC_INPUT 	    0x0001  /*data input*/
#define GPIO_FUNC_OUTPUT 	    0x0002	/*data output*/

#define GPIO_EDGE_FALL 	        0x01	/*falling edge IRQ*/
#define GPIO_EDGE_RISE 	        0x02	/*rising edge IRQ*/
#define GPIO_EDGE_BOTH 	        0x03	/*both falling or rising edge IRQ*/

#if 0
#define GET_EDGE(level)         (level == BTN_LEVEL_LOW ? GPIO_EDGE_FALL : GPIO_EDGE_RISE)
#define REVERT_EDGE(edge)       ((edge == GPIO_EDGE_FALL) ? GPIO_EDGE_RISE : GPIO_EDGE_FALL)
#endif

/* RTL8672ԭоƬ���̵�GPIO�����ӿ�*/
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


/* ������rtl8672_product.c�� */
extern struct btn_dev rtl8672_btns[];



#ifdef CONFIG_BUTTON_TRIGGER_TIMER
/*=======================================================================
* ʹ�ö�ʱ����ѯ��鰴ť״̬
=======================================================================*/

/* ��������ͨ���˺�����ð�ť״̬ */
static btn_status rtl8672_get_status(struct btn_dev *btn)
{
	int ret;

	ret =  gpioRead(btn->gpio);

	BTN_TRACE("button %d is on sattus %d\n", btn->name, ret);

	/* ����ǵ͵�ƽ���� */
	if(btn->level == BTN_LEVEL_LOW)
	{
		if(ret == 0)
			return BTN_DOWN;
		else
			return BTN_UP;
	}
    /* ����Ǹߵ�ƽ���� */
	else
	{
		if(ret == 0)
			return BTN_UP;
		else
			return BTN_DOWN;
	}

}

static int __init rtl8672_btn_init(void)
{
	int ret;
	int i;

	for(i=0;rtl8672_btns[i].name != btn_end; i++)
	{
		/*����ΪGPIOģʽ*/
		gpioConfigCNR(rtl8672_btns[i].gpio, 0);

		/* ��ʼ��GPIO�ӿڵĹ���״̬������Ϊ����ģʽ�������󴥷� */
		gpioConfig(rtl8672_btns[i].gpio, GPIO_FUNC_INPUT);

		/* ���û�ȡ��ť״̬��� */
		rtl8672_btns[i].get_status = rtl8672_get_status;

		ret = btn_dev_register(&rtl8672_btns[i]);
		if (ret != 0) {
			printk (KERN_ERR "Unable to register btn_dev %d(error %d)\n", rtl8672_btns[i].name, ret);
			return -1;
		}
	}

	return 0;
}

static void __exit rtl8672_btn_exit(void)
{
    int i;

    /* ע�����е�btn_dev */
    for(i=0;rtl8672_btns[i].name != btn_end; i++)
	{
		btn_dev_unregister(&rtl8672_btns[i]);
	}
}


#else

#define DEYLAY_TIME     2           //�����ӳ�ʱ��
struct timer_list delay_timer;      //�ӳٶ�ʱ��

/************************************************************************
* ʹ���ж�ģʽ��ⰴť״̬
************************************************************************/

void rtl8672_button_irq(unsigned long data)
{
    struct btn_dev *btn = (struct btn_dev *)data;
    int state = gpioRead(btn->gpio);

    BTN_TRACE("GIMR=0x%x, GPIO_PABCD_CNR=0x%x, GPIO_PABCD_PTYPE=0x%x, GPIO_PABCD_DIR=0x%x\n"
              "GPIO_PABCD_DAT=0x%x, GPIO_PABCD_ISR=0x%x, GPIO_PAB_IMR=0x%x, GPIO_PCD_IMR=0x%x\n"
              "gpio(%d)=%d\n",
              REG32(GIMR), REG32(GPIO_PABCD_CNR), REG32(GPIO_PABCD_PTYPE), REG32(GPIO_PABCD_DIR),
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

    /* ���¿���GPIO_ABCD�ж� */
    gpioConfigIntr(1);
}


void rtl8672_delay(struct btn_dev *btn)
{
    /* �ӳ�һ��ʱ��ȵ�ƽ�ȶ� */
    setup_timer(&delay_timer, rtl8672_button_irq, (unsigned long)btn);
	mod_timer( &delay_timer, jiffies + msecs_to_jiffies(DEYLAY_TIME));
}


irqreturn_t rtl8672_gpio_irq_dispatch(int cpl, void *dev_id, struct pt_regs *regs)
{
	int i;

    /* �ر�GPIO_ABCD�жϣ���ֹ���� */
    gpioConfigIntr(0);

	if((int)dev_id == GPIO_ABCD_IRQ)
	{
		for(i=0;rtl8672_btns[i].name != btn_end;i++)
		{
			if( gpioGetIntrStatus(rtl8672_btns[i].gpio))
			{
                BTN_TRACE("GIMR=0x%x, GPIO_PABCD_CNR=0x%x, GPIO_PABCD_PTYPE=0x%x, GPIO_PABCD_DIR=0x%x\n"
                          "GPIO_PABCD_DAT=0x%x, GPIO_PABCD_ISR=0x%x, GPIO_PAB_IMR=0x%x, GPIO_PCD_IMR=0x%x\n"
                          "gpio(%d)=%d\n",
                          REG32(GIMR), REG32(GPIO_PABCD_CNR), REG32(GPIO_PABCD_PTYPE), REG32(GPIO_PABCD_DIR),
                          REG32(GPIO_PABCD_DAT), REG32(GPIO_PABCD_ISR), REG32(GPIO_PAB_IMR), REG32(GPIO_PCD_IMR),
                          rtl8672_btns[i].gpio, gpioRead(rtl8672_btns[i].gpio)
                );

                gpioClearIntr(rtl8672_btns[i].gpio);
				rtl8672_delay(&rtl8672_btns[i]);  /* �Դ����жϵ�GPIO���д��� */
			}
		}

	}

	return IRQ_HANDLED;
}


/* ��������ͨ���˺�����ð�ť״̬ */
btn_status rtl8672_get_status(struct btn_dev *btn)
{
	return btn->cur_status;
}



static int __init rtl8672_btn_init(void)
{

	int i;
	int ret;

    BTN_TRACE("Before button init:\n"
        "GIMR=0x%x, GPIO_PABCD_CNR=0x%x, GPIO_PABCD_PTYPE=0x%x, GPIO_PABCD_DIR=0x%x\n"
        "GPIO_PABCD_DAT=0x%x, GPIO_PABCD_ISR=0x%x, GPIO_PAB_IMR=0x%x, GPIO_PCD_IMR=0x%x\n",
        REG32(GIMR), REG32(GPIO_PABCD_CNR), REG32(GPIO_PABCD_PTYPE), REG32(GPIO_PABCD_DIR),
        REG32(GPIO_PABCD_DAT), REG32(GPIO_PABCD_ISR), REG32(GPIO_PAB_IMR), REG32(GPIO_PCD_IMR)
    );

    /* ����GPIO_ABCD�����ж� */
    gpioConfigIntr(1);

	for(i=0;rtl8672_btns[i].name != btn_end; i++)
    {
        /* ����ָ����GPIOΪGPIOģʽ */
		gpioConfigCNR(rtl8672_btns[i].gpio, 0);

		/* ����ָ����GPIOΪ����ģʽ */
		gpioConfig(rtl8672_btns[i].gpio, GPIO_FUNC_INPUT);

        #if 0
        /* ����ָ����GPIO���жϴ���ģʽΪ����ʱ����(�½��ش���) */
        gpioConfigEdge(rtl8672_btns[i].gpio, GET_EDGE(rtl8672_btns[i].level));
        #endif

        /* ����ָ����GPIO���жϴ���ģʽΪ˫�򴥷� */
        gpioConfigEdge(rtl8672_btns[i].gpio, GPIO_EDGE_BOTH);

		/* ��ʼ����BTN�豸�ĳ�ʼ״̬ */
		rtl8672_btns[i].ignore = 0;
		rtl8672_btns[i].cur_status = BTN_UP;
		rtl8672_btns[i].get_status = rtl8672_get_status;

	}

    BTN_TRACE("After button init:\n"
        "GIMR=0x%x, GPIO_PABCD_CNR=0x%x, GPIO_PABCD_PTYPE=0x%x, GPIO_PABCD_DIR=0x%x\n"
        "GPIO_PABCD_DAT=0x%x, GPIO_PABCD_ISR=0x%x, GPIO_PAB_IMR=0x%x, GPIO_PCD_IMR=0x%x\n",
        REG32(GIMR), REG32(GPIO_PABCD_CNR), REG32(GPIO_PABCD_PTYPE), REG32(GPIO_PABCD_DIR),
        REG32(GPIO_PABCD_DAT), REG32(GPIO_PABCD_ISR), REG32(GPIO_PAB_IMR), REG32(GPIO_PCD_IMR)
    );


	ret = request_irq (GPIO_ABCD_IRQ, rtl8672_gpio_irq_dispatch, 0,
                       "GPIO_ABCD", (void *)GPIO_ABCD_IRQ);
	if (ret != 0)
	{
		printk (KERN_ERR "unable to request IRQ for GPIO (error %d)\n", ret);
		return -1;
	}

	return 0;
}


static void __exit rtl8672_btn_exit(void)
{
	/* do nothing */
}
#endif


module_init(rtl8672_btn_init);
module_exit(rtl8672_btn_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Amazon-SE buttons driver");
MODULE_LICENSE("GPL");


