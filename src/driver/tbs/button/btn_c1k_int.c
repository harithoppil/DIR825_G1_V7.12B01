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
#include <asm/arch/irq.h>

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
	return btn->cur_status;
}

irqreturn_t c1k_btn_irq_handler(int irq, void *dev_id)
{
	struct btn_dev *btn = (struct btn_dev*)dev_id;

	if(btn->ignore){

		/* 设置中断模式为下降沿触发 */
		__raw_writel((__raw_readl(COMCERTO_GPIO_INT_CFG_REG) & ~(0x3 << (btn->gpio * 2))) | (0x1 << (btn->gpio * 2)), COMCERTO_GPIO_INT_CFG_REG);

		btn->ignore = 0;
		btn->cur_status = BTN_UP;

		BTN_TRACE("btn %d up\n",btn->name);
		
	} else {

		/* 设置中断模式为上升沿触发 */
		__raw_writel((__raw_readl(COMCERTO_GPIO_INT_CFG_REG) & ~(0x3 << (btn->gpio * 2))) | (0x2 << (btn->gpio * 2)), COMCERTO_GPIO_INT_CFG_REG);
	
		btn->ignore = 1;
		btn->cur_status = BTN_DOWN;
		btn_status_query(btn);

		BTN_TRACE("btn %d down\n",btn->name);
	}

	comcerto_irq_ack_1(btn->gpio);
	return IRQ_HANDLED;
}

static int __init c1k_btn_init(void)
{
	int ret;
	int i;
	
	for(i=0;c1k_btns[i].name != btn_end; i++){
		/* 只有GPIO 0~3 5~7 支持中断*/
		if (c1k_btns[i].gpio < 0 || c1k_btns[i].gpio > 7 || c1k_btns[i].gpio == 4) {
			printk (KERN_ERR "Unable to register btn_dev.GPIO is %d(should be 0~3 5~7)\n", c1k_btns[i].gpio);
			return -1;
		}

		/* 初始化GPIO接口的工作状态 */
		comcerto_gpio_disable_output(comcerto_gpio_mask(c1k_btns[i].gpio));

		/* 设置中断模式为下降沿触发 */
		__raw_writel(__raw_readl(COMCERTO_GPIO_INT_CFG_REG) | (0x1 << (c1k_btns[i].gpio * 2)), COMCERTO_GPIO_INT_CFG_REG);

		/* 注册中断 */
		/************ IRQ define *************   
		#define IRQ_G7              (8 + 32) 
		#define IRQ_G6              (7 + 32) 
		#define IRQ_G5              (6 + 32) 
		#define IRQ_G4              (5 + 32) 
		#define IRQ_G3              (4 + 32) 
		#define IRQ_G2              (3 + 32) 
		#define IRQ_G1              (2 + 32) 
		#define IRQ_G0              (1 + 32)
		**************************************/

		ret = request_irq(32+1+c1k_btns[i].gpio, c1k_btn_irq_handler, SA_SHIRQ, "BUTTON IRQ", &c1k_btns[i]);
		if (ret != 0) {
			printk (KERN_ERR "Button: unable to register IRQ %d\n", 32+1+c1k_btns[i].gpio);
			return -1;
		}

		c1k_btns[i].ignore = 0;
		c1k_btns[i].cur_status = BTN_UP;

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
