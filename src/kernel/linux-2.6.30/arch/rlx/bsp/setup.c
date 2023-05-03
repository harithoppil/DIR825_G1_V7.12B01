/*
 * Copyright 2006, Realtek Semiconductor Corp.
 *
 * arch/rlx/rlxocp/setup.c
 *   Interrupt and exception initialization for RTL8198
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Nov. 7, 2006
 */
#include <linux/console.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>

#include <asm/addrspace.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <asm/bootinfo.h>
#include <asm/time.h>
#include <asm/reboot.h>
#include <asm/rlxbsp.h>

#include <asm/rtl865x/rtl865xc_asicregs.h>

#include "bspchip.h"

extern int bsp_swcore_init(unsigned int version);

static void prom_putchar(char c)
{
#define UART0_BASE		0xB8002000
#define UART0_THR		(UART0_BASE + 0x000)
#define UART0_FCR		(UART0_BASE + 0x008)
#define UART0_LSR       (UART0_BASE + 0x014)
#define TXRST			0x04
#define CHAR_TRIGGER_14	0xC0
#define LSR_THRE		0x20
#define TxCHAR_AVAIL	0x00
#define TxCHAR_EMPTY	0x20
unsigned int busy_cnt = 0;

	do
	{
		/* Prevent Hanging */
		if (busy_cnt++ >= 30000)
		{
		 /* Reset Tx FIFO */
		 REG8(UART0_FCR) = TXRST | CHAR_TRIGGER_14;
		 return;
		}
	} while ((REG8(UART0_LSR) & LSR_THRE) == TxCHAR_AVAIL);

	/* Send Character */
	REG8(UART0_THR) = c;
}

static void early_console_write(const char *s, unsigned n)
{
	while (n-- && *s) {
		if (*s == '\n')
			prom_putchar('\r');
		prom_putchar(*s);
		s++;
	}
}

static int GPIOdataReg=0;
/*
Check BD800000 to identify which GPIO pins can be used
*/
unsigned int get_GPIOMask(void)
{
	unsigned int portMask=0xFFFFFFFF;

	//portMask &= ~(GPIO_PB3|GPIO_PB4|GPIO_PB5|GPIO_PB6|GPIO_PB7);  //disable B3-B7

	return portMask;
}


/*
Config one GPIO pin. Release 1 only support output function
number and PIN location map:
Pin	num
PB7	15
:	:
PB0	8
PA7	7
:	:
PA0	0
*/
void gpioConfig (int gpio_num, int gpio_func)
{
    unsigned int useablePin;
    unsigned int mask;

	//printk( "<<<<<<<<<enter gpioConfig(gpio_num:%d, gpio_func:%d)\n", gpio_num, gpio_func );

	if ((gpio_num>63)||(gpio_num<0))
        return;

	useablePin = get_GPIOMask();
	mask=1<<(gpio_num%32);
	if ((useablePin&mask)==0) {  //GPIO pins is shared by other modules
		printk("GPIO config Error! PIN %d is used by a hardware module\n",gpio_num);
		return;
	};

    //mask=1<<gpio_num;
	if (GPIO_FUNC_INPUT == gpio_func)
	{
	    if(gpio_num<31)	
		    REG32(GPIO_PABCD_DIR) = REG32(GPIO_PABCD_DIR) & (~mask);
		else
			REG32(GPIO_PEFGH_DIR) = REG32(GPIO_PEFGH_DIR) & (~mask);
	}
	else
	{
		if(gpio_num<31)
            REG32(GPIO_PABCD_DIR) = REG32(GPIO_PABCD_DIR) | mask;
		else
			REG32(GPIO_PEFGH_DIR) = REG32(GPIO_PEFGH_DIR) | mask;
	}
}

/*
 set PABCD_CNR
 0-GPIO pin
 1-dedicated peripheral pin
*/
void gpioConfigCNR(int gpio_num, int mode)
{
    if (mode)
    {
		if(gpio_num<31)
            REG32(GPIO_PABCD_CNR) |= (1<<gpio_num);
		else
			REG32(GPIO_PEFGH_CNR) |= (1<<(gpio_num%32));
    }
    else
    {
		if(gpio_num<31)
            REG32(GPIO_PABCD_CNR) &= ~(1<<gpio_num);
		else
			REG32(GPIO_PEFGH_CNR) &= ~(1<<(gpio_num%32));
    }
    //REG32(GPIO_PABCD_PTYPE) &= ~(1<<gpio_num);
}

/* GPIO ABCD interrupt enable */
void gpioConfigIntr(int enable)
{
    if(enable)
        REG32(BSP_GIMR) |= (1<<BSP_GPIO_ABCD_IRQ);
    else
        REG32(BSP_GIMR) &= ~(1<<BSP_GPIO_ABCD_IRQ);
}
/* GPIO EFGH interrupt enable */
void gpioConfigIntr1(int enable)
{
    if(enable)
        REG32(BSP_GIMR) |= (1<<BSP_GPIO_EFGH_IRQ);
    else
        REG32(BSP_GIMR) &= ~(1<<BSP_GPIO_EFGH_IRQ);
}


/*
 Config Port A,B,C,D interrupt mask register
 0-disable interrupt
 1-enable falling edge interrupt
 2-enable rising edge interrupt
 3-enable both falling or rising edge interrupt
*/
void gpioConfigEdge(int gpio_num, int mode)
{
    int intrNum  = 0;
	int Org_gpio_num = gpio_num;
	
    if ((mode>63)||(mode<0))
        return;
	
    gpio_num = (gpio_num%32);

	printk("gpioConfigEdge() gpio_num=%d\n",gpio_num);
    intrNum = (gpio_num <16) ? gpio_num : (gpio_num - 16);
	printk("gpioConfigEdge(1) intrNum=%d\n",intrNum);
    if (gpio_num <16)
    {
        if(Org_gpio_num<32)
            REG32(GPIO_PAB_IMR) = (REG32(GPIO_PAB_IMR) & ~(3<<(intrNum * 2))) | (mode<<(intrNum * 2));
		else
			REG32(GPIO_PEF_IMR) = (REG32(GPIO_PEF_IMR) & ~(3<<(intrNum * 2))) | (mode<<(intrNum * 2));
    }
    else
    {
        if(Org_gpio_num<32)
            REG32(GPIO_PCD_IMR) = (REG32(GPIO_PCD_IMR) & ~(3<<(intrNum * 2))) | (mode<<(intrNum * 2));
		else
			{
			printk("gpioConfigEdge(2) intrNum=%d\n",intrNum);
			REG32(GPIO_PGH_IMR) = (REG32(GPIO_PGH_IMR) & ~(3<<(intrNum * 2))) | (mode<<(intrNum * 2));
			}
    }
}

/*
 Get Port A,B,C,D interrupt mask register
*/
int gpioGetEdge(int gpio_num)
{
    int Org_gpio_num = gpio_num;
	int intrNum = 0;
    gpio_num = (gpio_num%32);
	
    intrNum  = (gpio_num <16) ? gpio_num : (gpio_num - 16);
	
    if (gpio_num <16)
    {
        if(Org_gpio_num < 32)
            return (REG32(GPIO_PAB_IMR) & (3<<(intrNum * 2)))>>(intrNum * 2);
		else 
			return (REG32(GPIO_PEF_IMR) & (3<<(intrNum * 2)))>>(intrNum * 2);
    }
    else
    {
        if(Org_gpio_num < 32)
            return (REG32(GPIO_PCD_IMR) & (3<<(intrNum * 2)))>>(intrNum * 2);
		else
			return (REG32(GPIO_PGH_IMR) & (3<<(intrNum * 2)))>>(intrNum * 2);
    }
}


/* write interrupt status register 1 to clear interrupt*/
void gpioClearIntr(int gpio_num)
{
    if(gpio_num < 32)
        REG32(GPIO_PABCD_ISR) |= (1 << gpio_num);
	else
		REG32(GPIO_PEFGH_ISR) |= (1 << (gpio_num%32));
}




/* read interrupt status register */
int gpioGetIntrStatus(int gpio_num)
{
    unsigned int val;
	if ((gpio_num>63)||(gpio_num<0))
		return 0;
    if(gpio_num<32)
	    val = REG32(GPIO_PABCD_ISR);
	else
		val = REG32(GPIO_PEFGH_ISR);
	
	if (val & (1 << (gpio_num%32)))
		return 1;
	else
		return 0;
}

/*set GPIO pins on*/
void gpioSet(int gpio_num)
{
    unsigned int portMask=0;
	unsigned int pins;

	//printk( ">>>>>>>>enter gpioSet( gpio_num:%d )\n", gpio_num );
	if ((gpio_num>63)||(gpio_num<0)) 
		return;

	pins = 1<<(gpio_num%32);
	portMask = get_GPIOMask();
	pins &= portMask;  //mask out disable pins
	if (pins==0)
        return;  //no pins to set

	//write out
	if(gpio_num<32)
	    REG32(GPIO_PABCD_DAT) = REG32(GPIO_PABCD_DAT) | pins;
	else
		REG32(GPIO_PEFGH_DAT) = REG32(GPIO_PEFGH_DAT) | pins;
}

/*set GPIO pins off*/
void gpioClear(int gpio_num)
{
    unsigned int portMask=0;
	unsigned int pins;

    //printk( ">>>>>>>>enter gpioClear( gpio_num:%d )\n", gpio_num );
	if ((gpio_num>63)||(gpio_num<0)) 
		return;

	pins = 1<<(gpio_num%32);
	portMask = get_GPIOMask();
	pins &= portMask;  //mask out disable pins
	if (pins==0) return;  //no pins to reset

	//write out
	if(gpio_num<32)
	    REG32(GPIO_PABCD_DAT) = REG32(GPIO_PABCD_DAT) & (~pins);
	else
		REG32(GPIO_PEFGH_DAT) = REG32(GPIO_PEFGH_DAT) & (~pins);
}


int gpioRead(int gpio_num)
{
	unsigned int val;
	if ((gpio_num>63)||(gpio_num<0))
		return 0;
    if(gpio_num<32)
	    val = REG32(GPIO_PABCD_DAT);
	else
		val = REG32(GPIO_PEFGH_DAT);
	
	if (val & (1 << (gpio_num%32)))
		return 1;
	else
		return 0;
}

static void shutdown_netdev(void)
{
	struct net_device *dev;

	printk("Shutdown network interface\n");
	read_lock(&dev_base_lock);

	for_each_netdev(&init_net, dev)
	{
		if(dev->flags &IFF_UP) 
		{
			printk("%s:===>\n",dev->name);			
			rtnl_lock();
#if defined(CONFIG_COMPAT_NET_DEV_OPS)
			if(dev->stop)
				dev->stop(dev);
#else
			if ((dev->netdev_ops)&&(dev->netdev_ops->ndo_stop))
				dev->netdev_ops->ndo_stop(dev);
#endif
			rtnl_unlock();
		}
      	}
#if defined(CONFIG_RTL8192CD)
	{
		extern void force_stop_wlan_hw(void);
		force_stop_wlan_hw();
	}
#endif
#ifdef CONFIG_RTL_8367R_SUPPORT
	// for boot code tftp feature after did the kernel reboot, of course rtk_vlan_init() can be moved to boot code, but the boot code size is near 24Kbytes
	{	
//	extern int rtk_vlan_init(void);
//	rtk_vlan_init();
	extern void rtl8367rb_reset(void);
	rtl8367rb_reset();
	}
#endif
	read_unlock(&dev_base_lock);
}

static void bsp_machine_restart(char *command)
{
    static void (*back_to_prom)(void) = (void (*)(void)) 0xbfc00000;
	
    REG32(GIMR)=0;
	
    local_irq_disable();
#ifdef CONFIG_NET    
    shutdown_netdev();
#endif    
    REG32(BSP_WDTCNR) = 0; //enable watch dog
    while (1) ;
    /* Reboot */
    back_to_prom();
}
                                                                                                    
static void bsp_machine_halt(void)
{
    while(1);
}
                                                                                                    
static void bsp_machine_power_off(void)
{
    while(1);
}

/*
 * callback function
 */
#define REG32(reg)       (*(volatile unsigned int *)(reg))
extern void _imem_dmem_init(void);
void __init bsp_setup(void)
{
	int ret= -1;
	unsigned int version = 0;
	 if((REG32(0xb8000000)&0xf)<3)
    {
		REG32(0xb8000088)= (REG32(0xb8000088) & ( ~(3<<5)&~(0xF<<0)));
		REG32(0xb8000088) =(REG32(0xb8000088)|(1<<4));
		REG32(0xb8000088) = REG32(0xb8000088) & (~(3<<7));
	}


    /* define io/mem region */
    ioport_resource.start = 0x18000000; 
    ioport_resource.end = 0x1fffffff;

    iomem_resource.start = 0x18000000;
    iomem_resource.end = 0x1fffffff;

    /* set reset vectors */
    _machine_restart = bsp_machine_restart;
    _machine_halt = bsp_machine_halt;
    pm_power_off = bsp_machine_power_off;

#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D) 
{
	unsigned int tmp=0,tmp1=0,tmp2=0;
	tmp1=REG32(0xb8001004);
	if((REG32(0xb8001000)&0x80000000)==0x80000000)
	{	
		//REG32(0xb8001008)=0x6d13a4c0;
		REG32(0xb8001004)=tmp1;
	}
}
#endif
	version = 8;
    /* initialize uart */
    bsp_serial_init();
    _imem_dmem_init();

	/* initialize switch core */
#if defined(CONFIG_RTL_819X)
	ret = bsp_swcore_init(version);
	if(ret != 0)
	{
		bsp_machine_halt();
	}
#endif
}
