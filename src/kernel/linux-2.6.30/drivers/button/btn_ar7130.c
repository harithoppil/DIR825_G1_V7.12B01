/**
 * �ļ���:btn_ar7130.c
 * ˵��: AR7130�����İ�ť����
 * 
 * ����:Zhang Yu
 **/

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


extern struct btn_dev ar7130_btns[];  /* ������ar7130_pb42_product.c�� */
extern void ar7100_gpio_config_int(int gpio, ar7100_gpio_int_type_t type, int polarity); /* ar7130��ԭ��������������arch/mips�� */

/* �жϴ��������жϴ����������´δ����жϵļ��Է�ת����Ȼ�޷�������һ��״̬ */
irqreturn_t ar7130_btn_irq(int cpl, void *dev_id, struct pt_regs *regs)
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
btn_status ar7130_get_status(struct btn_dev *btn)
{
			return btn->cur_status;
}

static int __init ar7130_btn_init(void)
{
	int req;
	int i;
	char button_name[16];

	
	for(i=0;ar7130_btns[i].name != btn_end; i++){

		/* ��ʼ��GPIO�ӿڵĹ���״̬������Ϊ����ģʽ�������󴥷� */
		ar7100_gpio_config_input(ar7130_btns[i].gpio);
		
		ar7100_gpio_config_int (ar7130_btns[i].gpio, INT_TYPE_LEVEL, ar7130_btns[i].level);

		/* ��ʼ����BTN�豸�ĳ�ʼ״̬ */
		ar7130_btns[i].ignore = 0;
		ar7130_btns[i].cur_status = BTN_UP;
		ar7130_btns[i].get_status = ar7130_get_status;

		sprintf(button_name,"AR7130 button %d",ar7130_btns[i].name);
		
		req = request_irq(AR7100_GPIO_IRQn(ar7130_btns[i].gpio), ar7130_btn_irq, 0, button_name, &ar7130_btns[i]);
		
		if (req != 0) {
			printk (KERN_ERR "unable to request IRQ for button GPIO %d(error %d)\n", ar7130_btns[i].gpio, req);
			return -1;
		}
	
	}

	return 0;

}


static void __exit ar7130_btn_exit(void)
{
	/* do nothing */
}

module_init(ar7130_btn_init);
module_exit(ar7130_btn_exit);

MODULE_AUTHOR("Zhang Yu");
MODULE_DESCRIPTION("Ar7130 buttons driver");
MODULE_LICENSE("GPL");
