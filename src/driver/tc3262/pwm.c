/*
** $Id: //BBN_Linux/Branch/Branch_for_Kernel_Upgrade_20120517/tclinux_phoenix/modules/private/tc3262/pwm.c#1 $
*/
/************************************************************************
 *
 *	Copyright (C) 2010 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
/*
** $Log: pwm.c,v $
** Revision 1.2  2011/06/03 02:33:09  lino
** add RT65168 support
**
** Revision 1.1.1.1  2010/09/30 21:14:53  josephxu
** modules/public, private
**
** Revision 1.1  2010/08/30 07:51:11  lino
** add power saving mode kernel module
**
*/

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <asm/processor.h>
#include <asm/addrspace.h>
#include <asm/tc3162/tc3162.h>

#define DP5CR			0xbfb20110
#define SUPPORT_CHIP isRT65168
void (*old_cpu_wait)(void) = NULL;
extern int reset_time_value(int shift);

uint16 psm_enable = 0;
uint16 psm_debug = 0;

/* power saving mode is conflict with DDR PHY 5 Control Register DRVCAL_ENA,DRVCAL_DYNAMIC_PD bit16,17 */
uint32 dp5cr = 0;

/*______________________________________________________________________________
**	psm_wait
**
**	descriptions:
**	parameters:
**	local:
**	global:
**	return:
**	called by:
**	call:
**	revision:
**____________________________________________________________________________*/
void psm_wait(void)
{
	volatile uint32 timer_now, timer_last;
	volatile uint32 tick_acc;
	uint32 one_tick_unit = 1 * SYS_HCLK / 2;
	volatile uint32 timer2_ldv = VPint(CR_TIMER1_LDV);

	if (psm_debug) {
		tick_acc = 0;
		timer_last = VPint(CR_TIMER1_VLR);
	}

	if (psm_enable)
		VPint(CR_PSMCR) |= (1<<2)|(1<<0);		/* enable power saving mode */

	if (old_cpu_wait)
		(*old_cpu_wait)();

	if (psm_debug) {
		timer_now = VPint(CR_TIMER1_VLR);
		if (timer_last >= timer_now) 
			tick_acc += timer_last - timer_now;
		else
			tick_acc += timer2_ldv - timer_now + timer_last;

		if (psm_debug & 0x1)
			printk("time=%d Trigger=%lx CTL=%lx DUR=%lx\n", 
					tick_acc/one_tick_unit, (VPint(CR_PSMCR)>>7)&0x1ff, 
					VPint(CR_PSMCR), VPint(CR_PSMDR));
		else if (psm_debug & 0x2) {
			if (VPint(CR_PSMCR) & (1<<3))
				printk("time=%d Trigger=%lx CTL=%lx DUR=%lx\n", 
						tick_acc/one_tick_unit, (VPint(CR_PSMCR)>>7)&0x1ff, 
						VPint(CR_PSMCR), VPint(CR_PSMDR));
		}
	}
}

/************************************************************************
*     P O W E R  S A V I N G  M O D E  P R O C  D E F I N I T I O N S
*************************************************************************
*/

static int psm_enable_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%0d\n", psm_enable);
	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

static int psm_enable_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];
	uint16 psm_enable_set;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	psm_enable_set = simple_strtoul(val_string, NULL, 16) ? 1 : 0;

	if (psm_enable != psm_enable_set) {
		psm_enable = psm_enable_set;
		if (psm_enable) {
			if (isRT63165) {
				/* Enable DDR self refreshing mode */
				VPint(CR_DMC_DDR_SR) |= (1<<4);
				/* Set DDR self refreshing idle time to 512us */
				VPint(CR_DMC_DDR_SR_CNT) = 512 / 256 * SYS_HCLK; 
			} else {
				/* disable bit16,17 of dp5cr when enable power saving mode */
				VPint(DP5CR) &= ~((1<<16)|(1<<17));

				old_cpu_wait = cpu_wait;
				cpu_wait = psm_wait;
			}
		} else {
			if (isRT63165) {
				/* Disable DDR self refreshing mode */
				VPint(CR_DMC_DDR_SR) &= ~(1<<4);
			} else {
				/* restore bit16,17 of dp5cr when disable power saving mode */
				VPint(DP5CR) |= dp5cr;

				cpu_wait = old_cpu_wait;
				old_cpu_wait = NULL;
			}
		}
	}

	return count;
}

static int psm_cpudiv_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;
	int div;
	if(SUPPORT_CHIP){
		switch(((VPint(CR_CKGEN_CONF)>>5) & 0x7)){
			case 0:
				div = 1;
				break;
			case 1:
				div = 2;
				break;
			case 2:
				div = 4;
				break;
			default:
				break;
		}
		len = sprintf(page, "CPU Clock 1/%d\n", div);
	}else{
		len = sprintf(page, "Not Support\n");	
	}	
	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

static int psm_cpudiv_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];
	unsigned long temp, value, insert_val;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	if(SUPPORT_CHIP){
		val_string[count] = '\0';

		value = simple_strtoul(val_string, NULL, 10);
		//printk("CPU clock 1/%d\n ",value);

		switch(value){
			case 2:
				insert_val = 1;
				break;
			case 4:
				insert_val = 2;
				break;
			case 1:
			default:
				insert_val = 0;
				break;
		}
		temp = VPint(CR_CKGEN_CONF);
		temp &= ~(0x7<<5);
		temp |= ((insert_val & 0x7) << 5);
		VPint(CR_CKGEN_CONF) = temp;

		if(reset_time_value(insert_val) == -1){
			printk("reset time fail\n");	
		}
#if 0
		//delay test
		printk("1\n");
		mdelay(1000);
		printk("2\n");
		mdelay(1000);
		printk("3\n");
		mdelay(1000);
		printk("4\n");
		mdelay(1000);
		printk("5\n");
		mdelay(1000);
#endif
	}else{
		printk("Not Support\n");	
	}	
	return count;
}

static int psm_usbhost_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;
	char tmp[16];
	if(SUPPORT_CHIP){
		if(VPint(0xbfb000c4) & (((1<<2)|(1<<3)|(1<<9)))){
			sprintf(tmp, "Enable");	
		}else{
			sprintf(tmp, "Disable");
		}

		len = sprintf(page, "USB HOST is %s\n",tmp);
	}else{
		len = sprintf(page, "Not Support\n");	
	}	
	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

static int psm_usbhost_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];
	unsigned int value;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';
	
	if(SUPPORT_CHIP){
		value = simple_strtoul(val_string, NULL, 10);
	
		if(value == 1){//Enable usb host
			VPint(0xbfb000a8) |= ((1<<5)|(1<<6));
			VPint(0xbfb000ac) &= ~((1<<24)|(1<<3)); 
			VPint(0xbfb000ac) |= (1<<6);
			VPint(0xbfb000e8) &= ~((1<<24)|(1<<3)); 
			VPint(0xbfb000e8) |= (1<<6);
			VPint(0xbfb000c4) |= ((1<<2)|(1<<3)|(1<<9)); 
		}else if(value == 0){
			VPint(0xbfb000a8) &= ~((1<<5)|(1<<6));
			VPint(0xbfb000ac) |= ((1<<24)|(1<<3)); 
			VPint(0xbfb000ac) &= ~(1<<6);
			VPint(0xbfb000e8) |= ((1<<24)|(1<<3)); 
			VPint(0xbfb000e8) &= ~(1<<6);
			VPint(0xbfb000c4) &= ~((1<<2)|(1<<3)|(1<<9)); 
		}
	}else{
		printk("Not Support\n");	
	}	

	return count;
}

static int psm_debug_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%04x\n", psm_debug);
	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

static int psm_debug_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[32];

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	psm_debug = simple_strtoul(val_string, NULL, 16);

	return count;
}

static int __init pwm_init(void)
{
	struct proc_dir_entry *pwm_proc;

	printk("Power Manager 0.1 init\n");

	if (!isRT63165) {
		/* save original value of bit16,17 of dp5cr */
		dp5cr = VPint(DP5CR) & ((1<<16)|(1<<17));
	}

	pwm_proc = create_proc_entry("tc3162/psm_enable", 0, NULL);
	pwm_proc->read_proc = psm_enable_read_proc;
	pwm_proc->write_proc = psm_enable_write_proc;

	pwm_proc = create_proc_entry("tc3162/psm_debug", 0, NULL);
	pwm_proc->read_proc = psm_debug_read_proc;
	pwm_proc->write_proc = psm_debug_write_proc;

	pwm_proc = create_proc_entry("tc3162/psm_cpudiv", 0, NULL);
	pwm_proc->read_proc = psm_cpudiv_read_proc;
	pwm_proc->write_proc = psm_cpudiv_write_proc;

	pwm_proc = create_proc_entry("tc3162/psm_usbhost", 0, NULL);
	pwm_proc->read_proc = psm_usbhost_read_proc;
	pwm_proc->write_proc = psm_usbhost_write_proc;
	return 0;
}

static void __exit pwm_exit(void)
{
	printk("Power Manager 0.2 exit\n");

	if (isRT63165) {
		if (psm_enable) {
			/* Disable DDR self refreshing mode */
			VPint(CR_DMC_DDR_SR) &= ~(1<<4);
		}
	} else {
		if (psm_enable) {
			cpu_wait = old_cpu_wait;
			old_cpu_wait = NULL;
		}
	}

	remove_proc_entry("tc3162/psm_enable", 0);
	remove_proc_entry("tc3162/psm_debug", 0);
	remove_proc_entry("tc3162/psm_cpudiv", 0);
	remove_proc_entry("tc3162/psm_usbhost", 0);

}

module_init (pwm_init);
module_exit (pwm_exit);
MODULE_LICENSE("GPL");
