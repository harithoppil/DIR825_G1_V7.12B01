/**
 * �ļ���:btn_rt3052.c
 * ˵��: RT3052�����İ�ť����
 * 
 * ����:Xuan Guanglei
 **/
#if 1


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/resource.h>
#include <asm/types.h>
#include <asm/irq.h>

#include <btn.h>

//#define BTN_DEBUG

#ifdef BTN_DEBUG
    #define BTN_TRACE printk
#else
    #define BTN_TRACE(str, args...)  do { ; } while(0);
#endif


extern struct btn_dev rt3052_btns[];  /* ������rt3052_product.c�� */
extern void rt3052_gpio_mode(int gpio);
extern void rt3052_gpio_config_input(int gpio);
extern btn_status rt3052_get_in(int gpio);


/* ��������ͨ���˺�����ð�ť״̬ */
btn_status rt3052_get_status(struct btn_dev *btn)
{
	int ret;

	ret =  rt3052_get_in(btn->gpio);
	
	BTN_TRACE("button %d is on sattus %d\n", btn->name, ret);

	/* ����ǵ͵�ƽ���� */
	if(btn->level == BTN_LEVEL_LOW)
	{
		if(ret == 0)
			return BTN_DOWN;
		else
			return BTN_UP;
	}
	else
	{   /* ����Ǹߵ�ƽ���� */
		if(ret == 0)
			return BTN_UP;
		else
			return BTN_DOWN;
	}

}

static int __init rt3052_btn_init(void)
{
	int ret;
	int i;
	
	for(i=0;rt3052_btns[i].name != btn_end; i++)
	{
		/*����ΪGPIOģʽ*/
		rt3052_gpio_mode(rt3052_btns[i].gpio);
		
		/* ��ʼ��GPIO�ӿڵĹ���״̬������Ϊ����ģʽ�������󴥷� */
		rt3052_gpio_config_input(rt3052_btns[i].gpio);

		/* ��ʼ����BTN�豸�ĳ�ʼ״̬ */
		rt3052_btns[i].get_status = rt3052_get_status;

		ret = btn_dev_register(&rt3052_btns[i]);
		
		if (ret != 0) {
			printk (KERN_ERR "Unable to register btn_dev %d(error %d)\n", rt3052_btns[i].name, ret);
			return -1;
		}	
	}

	return 0;
}


static void __exit rt3052_btn_exit(void)
{
	/* do nothing */
}

module_init(rt3052_btn_init);
module_exit(rt3052_btn_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Amazon-SE buttons driver");
MODULE_LICENSE("GPL");



#else

#include <linux/kernel.h>
#include <linux/init.h>


#include <linux/config.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/resource.h>
#include <asm/types.h>
#include <asm/irq.h>

#include <btn.h>

#include "ar7100.h"

typedef enum {
	INT_TYPE_EDGE,
	INT_TYPE_LEVEL,
}ar7100_gpio_int_type_t;
   
typedef enum {
    INT_POL_ACTIVE_LOW,
    INT_POL_ACTIVE_HIGH,
}ar7100_gpio_int_pol_t;


extern struct btn_dev rt3052_btns[];  /* ������rt3052_product.c�� */
extern void ar7100_gpio_config_int(int gpio, ar7100_gpio_int_type_t type, int polarity); /* ar7130��ԭ��������������arch/mips�� */

/* �жϴ��������жϴ����������´δ����жϵļ��Է�ת����Ȼ�޷�������һ��״̬ */
irqreturn_t rt3052_btn_irq(int cpl, void *dev_id, struct pt_regs *regs)
{
	struct btn_dev *btn = (struct btn_dev *)dev_id;
	ar7100_gpio_int_pol_t polarity;
	
    if (btn->ignore) {
		/* ת����AR7130ԭ�����ı�ʾ��ʽ */
		if(btn->level == BTN_LEVEL_LOW)
			polarity = INT_POL_ACTIVE_LOW;
		else
			polarity = INT_POL_ACTIVE_HIGH;

		
        ar7100_gpio_config_int (btn->gpio,INT_TYPE_LEVEL,polarity);
		btn->ignore = 0;
		btn->cur_status = BTN_UP;  /* ����״̬ */

		printk("%d button up\n",btn->name);
	
    }else
    	{
			/* ת����AR7130ԭ�����ı�ʾ��ʽ�����ҷ�ת */
			if(btn->level == BTN_LEVEL_LOW)
				polarity = INT_POL_ACTIVE_HIGH;
			else
				polarity = INT_POL_ACTIVE_LOW;


			
			ar7100_gpio_config_int (btn->gpio,INT_TYPE_LEVEL, polarity);
			btn->ignore = 1;
			btn->cur_status = BTN_DOWN;  /* ����״̬ */
			btn_status_query(btn);
			printk("%d button down\n",btn->name);
    	}
	
    return IRQ_HANDLED;
}


/* ��������ͨ���˺�����ð�ť״̬ */
btn_status rt3052_get_status(struct btn_dev *btn)
{
			return btn->cur_status;
}

static int __init rt3052_btn_init(void)
{
	int req;
	int i;
	char button_name[16];

	
	for(i=0;rt3052_btns[i].name != btn_end; i++)
	{
		/*����ΪGPIOģʽ*/
		rt3052_gpio_mode(rt3052_btns[i].gpio)
		
		/* ��ʼ��GPIO�ӿڵĹ���״̬������Ϊ����ģʽ�������󴥷� */
		rt3052_gpio_config_input(rt3052_btns[i].gpio);
		
		rt3052_gpio_config_int (rt3052_btns[i].gpio, INT_TYPE_LEVEL, rt3052_btns[i].level);

		/* ��ʼ����BTN�豸�ĳ�ʼ״̬ */
		rt3052_btns[i].ignore = 0;
		rt3052_btns[i].cur_status = BTN_UP;
		rt3052_btns[i].get_status = rt3052_get_status;

		sprintf(button_name,"RT3052 button %d",rt3052_btns[i].name);
		
		req = request_irq(AR7100_GPIO_IRQn(rt3052_btns[i].gpio), rt3052_btn_irq, 0, button_name, &rt3052_btns[i]);
		
		if (req != 0) {
			printk (KERN_ERR "unable to request IRQ for button GPIO %d(error %d)\n", rt3052_btns[i].gpio, req);
			return -1;
		}
	
	}

	return 0;

}


static void __exit rt3052_btn_exit(void)
{
	/* do nothing */
}

module_init(rt3052_btn_init);
module_exit(rt3052_btn_exit);

MODULE_AUTHOR("Xuan Guanglei");
MODULE_DESCRIPTION("RT3052 buttons driver");
MODULE_LICENSE("GPL");
#endif

