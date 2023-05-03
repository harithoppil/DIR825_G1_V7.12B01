/**********************************************************************
 Copyright (c), 1991-2013, T&W ELECTRONICS(SHENTHEN) Co., Ltd.
 MT751X LEDs Driver        by xiachaoren on 2013-09-03
**********************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/net.h>
#include <linux/pci.h>
#include <led.h>
#include <gpio.h>
#include <autoconf.h>

#define LED_CTRL			0x0770
#define LED0_S0				0x077C
#define LED0_S1				0x0780

extern struct led_dev led_table[];                   /* Defined in product.c */
static struct net_device *ndev0 = NULL;
static struct net_device *ndev1 = NULL;

static spinlock_t mt751x_led_lock;

static void mt751x_led_on(struct led_dev *dev)
{
	unsigned int port;
	struct pci_dev *pdev = NULL;
	unsigned int vendor;
	unsigned int device;
	unsigned long flags;

	if((dev->gpio >= GPIO42) && (dev->gpio <= GPIO46)) {
		port = (unsigned int)(dev->gpio - GPIO42);
		PHY_led_ops(port, led_off_trig);
		gpio_config(dev->gpio, GPIO_OUT);
	}
	if(dev->gpio < GPIO64) {
	    gpio_write(dev->gpio, dev->level);
	} else if(dev->gpio < GPIO65) {
		if(NULL == ndev0) {
			vendor = PCI_VENDOR_ID_RT;
			device = 0x3091;/*  */
			local_irq_disable();
			pdev = pci_get_device(vendor, device, NULL);
			if(NULL != pdev) {
				ndev0 = (struct net_device *)pci_get_drvdata(pdev);
				pci_dev_put(pdev);
			}
			local_irq_enable();
		}
		if(NULL != ndev0) {
			if(ndev0->flags & IFF_UP) {/* Interface is UP */
				port = reg_read32(ndev0->base_addr + 0x1000);
				reg_write32(ndev0->base_addr + 0x102C, 0x30031432);
			}
		}
	} else if(dev->gpio < GPIO67) {
		if(NULL == ndev1) {
			vendor = PCI_VENDOR_ID_MTK;
			device = 0x7650;/*  */
			local_irq_disable();
			pdev = pci_get_device(vendor, device, NULL);
			if(NULL != pdev) {
				ndev1 = (struct net_device *)pci_get_drvdata(pdev);
				pci_dev_put(pdev);
			}
			local_irq_enable();
		}
		if(NULL != ndev1) {
			if(ndev1->flags & IFF_UP) {/* Interface is UP */
				spin_lock_irqsave(&mt751x_led_lock, flags);
				reg_write32(ndev1->base_addr + LED0_S0, 0x10000);
				reg_write32(ndev1->base_addr + LED0_S1, 0x0);
				reg_write32(ndev1->base_addr + LED_CTRL, 0x82);
				spin_unlock_irqrestore(&mt751x_led_lock, flags);
			}
		}
	}
}

static void mt751x_led_off(struct led_dev *dev)
{
	unsigned int port;
	struct pci_dev *pdev = NULL;
	unsigned int vendor;
	unsigned int device;
	unsigned long flags;

	if((dev->gpio >= GPIO42) && (dev->gpio <= GPIO46)) {
		port = (unsigned int)(dev->gpio - GPIO42);
		PHY_led_ops(port, led_off_trig);
		gpio_config(dev->gpio, GPIO_OUT);
	}
	if(dev->gpio < GPIO64) {
   		if(dev->level) {
			gpio_write(dev->gpio, GPIO_LEVEL_LOW);
		} else {
			gpio_write(dev->gpio, GPIO_LEVEL_HIGH);
		}
	} else if(dev->gpio < GPIO65) {
		if(NULL == ndev0) {
			vendor = PCI_VENDOR_ID_RT;
			device = 0x3091;/*  */
			local_irq_disable();
			pdev = pci_get_device(vendor, device, NULL);
			if(NULL != pdev) {
				ndev0 = (struct net_device *)pci_get_drvdata(pdev);
				pci_dev_put(pdev);
			}
			local_irq_enable();
		}
		if(NULL != ndev0) {
			if(ndev0->flags & IFF_UP) {/* Interface is UP */
				port = reg_read32(ndev0->base_addr + 0x1000);
				reg_write32(ndev0->base_addr + 0x102C, 0x031432);
			}
		}
	} else if(dev->gpio < GPIO67) {
		if(NULL == ndev1) {
			vendor = PCI_VENDOR_ID_MTK;
			device = 0x7650;/*  */
			local_irq_disable();
			pdev = pci_get_device(vendor, device, NULL);
			if(NULL != pdev) {
				ndev1 = (struct net_device *)pci_get_drvdata(pdev);
				pci_dev_put(pdev);
			}
			local_irq_enable();
		}
		if(NULL != ndev1) {
			if(ndev1->flags & IFF_UP) {/* Interface is UP */
				spin_lock_irqsave(&mt751x_led_lock, flags);
				reg_write32(ndev1->base_addr + LED0_S0, 0x0);
				reg_write32(ndev1->base_addr + LED0_S1, 0x0);
				reg_write32(ndev1->base_addr + LED_CTRL, 0x82);
				spin_unlock_irqrestore(&mt751x_led_lock, flags);
			}
		}
	}
}

struct led_hw_handler hw_handler = {
	mt751x_led_on,
	mt751x_led_off,
};

static int __init mt751x_led_init(void)
{
	unsigned int i;
	int ret;
	struct led_dev *dev;

	ret = led_core_init();/* LED core initializing */
	if(ret < 0) {
		goto out;
	}
	ret = led_hw_handle_register(&hw_handler);/* Register hardware on/off handler to core */
	if(ret < 0){
		printk("Error:fail to register hw_handler.\n");
		goto out;
	}
	printk("Register led device for %s:", PRODUCT);
	for(i = 0; led_table[i].name != led_end; i++){/* Register LED devices */
		dev = &led_table[i];
		gpio_config(dev->gpio, GPIO_OUT);
		ret = led_dev_register(dev);
		if(ret<0){
			printk("Error:fail to register led_table[%d].\n", i);
			break;
		}
		printk(" %02d", (unsigned int)dev->name);
	}
	printk("\n");

out:
	return ret;
}

static void __exit mt751x_led_exit(void)
{
	struct led_dev *dev;
	int i;
	int ret;

	spin_lock_init(&mt751x_led_lock);

	for(i=0; led_table[i].name != led_end;i++) {/* Unregister LED devices */
		dev = &led_table[i];
		ret = led_dev_unregister(dev);
		if(ret < 0){
			printk("Error:fail to unregister led_table[%d]\n",i);
		}
	}
	ret = led_hw_handle_unregister(&hw_handler);/* Unregister hardware on/off handler */
	if(ret < 0){
		printk(KERN_ERR "Error:fail to unregister hw_handler.\n");
	}
	ndev0 = NULL;
	ndev1 = NULL;
	led_core_exit();
}

module_init(mt751x_led_init);
module_exit(mt751x_led_exit);

MODULE_AUTHOR("xiachaoren");
MODULE_DESCRIPTION("MT751X LEDs driver");
MODULE_LICENSE("GPL");

