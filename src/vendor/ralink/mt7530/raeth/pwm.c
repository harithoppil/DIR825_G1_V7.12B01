/*
** $Id: //BBN_Linux/Branch/Branch_for_SDK_Release_MultiChip_20111013/tclinux_phoenix/modules/private/raeth/pwm.c#1 $
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


#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/mii.h>
#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <asm/processor.h>
#include <asm/addrspace.h>
#include <asm/tc3162/tc3162.h>

#include "femac.h"
#include "../tc3162l2hp2h/tsarm.h"

/************************************************************************
 *         F U N C T I O N   D E C L A R A T I O N S
 ************************************************************************/
static int mode_switch_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int mode_switch_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);

static int dram_type_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int pwm_start_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int pwm_start_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);
void change_idle2Normal_mode(void);
void change_normal2Idle_mode(void);
void switch_power_and_pcie1Clk(int flag);
void switch_ratio(int flag);
void switch_selfReflashMode(int flag);
void switch_pcie_phy_aspm_func(int flag);
void switch_ahb_bus_clk(int flag);
/************************************************************************
*     P O W E R  S A V I N G  M O D E  P R O C  D E F I N I T I O N S
*************************************************************************
*/

#define PCIE_PHY_ASPM_5392 0xc0010080
#define IDLE_2_ON 1
#define ON_2_IDLE 0
#define SET_SUCCESS 0
#define SET_FAIL 1

int mode_switch=SET_SUCCESS;//default success
static uint32 def_dmcDdr_reg = 0;
int pwm_enable = 0;

/*_____________________________________________________________________________
**      function name: switch_ratio
**      descriptions:
**			change ratio function: 
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/
void switch_ratio(int flag)
{
	uint32 reg = 0;
	int count = 0;
	
	reg=VPint(CR_CRCC_REG);
	if(reg & (1<<6)){
	/*check whether CHG_FREQ_REG1 bit[6] == 0 3 times; 
	 *if bit[6] is still 1, then tell application set failed
	 *and it will set in next preriod
	 */
		do{
			reg=VPint(CR_CRCC_REG);
			if((reg & (1<<6)) == 0){
				break;
			}
			count++;
		}while(count<3);
	}
	
	if(count >= 3){
		mode_switch =SET_FAIL;
		goto exit;
	}
	switch(flag){
		case ON_2_IDLE:	
			VPint(CR_CRCC_REG) = 0x101;
			VPint(CR_CRCC_REG) = 0x121;//radio=1.5:1
			break;
		case IDLE_2_ON:
			VPint(CR_CRCC_REG) = 0x201;
			VPint(CR_CRCC_REG) = 0x221;//radio=3:1
			break;
		default:
			break;
	}
	mode_switch =SET_SUCCESS;

exit:
	return;
}
/*_____________________________________________________________________________
**      function name: switch_pcie_phy_aspm_func
**      descriptions:
**			disable(down freq) or enable(up freq) pcie phy aspm function
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/

void switch_pcie_phy_aspm_func(int flag)
{
	uint32 reg = 0;
	
	switch(flag){
		case ON_2_IDLE:	//disable pcie phy ASPM func
			reg = VPint(PCIE_PHY_ASPM_5392);
			reg |=0x3;//bit[1:0]:2b'11
			VPint(PCIE_PHY_ASPM_5392)=reg;
			VPint(CR_CFG_ADDR_REG) = 0x80;
			reg = VPint(CR_CFG_DATA_REG);
			reg |= 0x3;//bit[1:0]:2b'11
			VPint(CR_CFG_DATA_REG)=reg;
			break;
		case IDLE_2_ON:	//enable pcie phy ASPM func
			reg = VPint(PCIE_PHY_ASPM_5392);
			reg &= ~0x3;//set bit[1:0]:2b'00
			VPint(PCIE_PHY_ASPM_5392)=reg;
			VPint(CR_CFG_ADDR_REG) = 0x80;
			reg = VPint(CR_CFG_DATA_REG);
			reg &= ~0x3;//set bit[1:0]:2b'00
			VPint(CR_CFG_DATA_REG)=reg;
			break;
		default:
			break;
	}
	return;
}
/*_____________________________________________________________________________
**      function name: switch_power_and_pcie1Clk
**      descriptions:
**			adjust core voltage and switch pcie1 clock when up/down freq
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/

void switch_power_and_pcie1Clk(int flag)
{
	uint32 reg = 0;
	
	switch(flag){
		case ON_2_IDLE:	//disable pcie phy ASPM func
			reg = VPint(CR_AHB_SSR);
			reg &= ~(0x1f<<26);
			reg |= 0x0e<<26;//bit[30-26]:5b'01110
			reg &= ~0x8;//bit3=0
			VPint(CR_AHB_SSR) = reg;//0x39250006
			break;
		case IDLE_2_ON:	//enable pcie phy ASPM func
			reg = VPint(CR_AHB_SSR);
			reg &= ~(0x1f<<26);
			reg |= (0x12<<26);//bit[30-26]:5b'10010
			reg |= 0x8;//bit3:1
			VPint(CR_AHB_SSR) = reg;//0x4925000e
			break;
		default:
			break;
	}
	return;
}
/*_____________________________________________________________________________
**      function name: switch_ahb_bus_clk
**      descriptions:
**			disable some AHB BUS CLK when down freq and restore back when up freq
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/

void switch_ahb_bus_clk(int flag)
{
	uint32 reg = 0;
	
	reg = VPint(CR_AHB_CLK);
	switch(flag){
		case ON_2_IDLE:	
			/*set bit14=1b'0, bit12=1b'0, bit[11:9]=3b'0, bit[6:4]=3b'0,bit0=1b'0*/
			reg &= ~0x5e71;
			VPint(CR_AHB_CLK) = reg;
			break;
		case IDLE_2_ON:	
			/*set bit14=1b'1, bit12=1b'1, bit[11:9]=3b'1, bit[6:4]=3b'1,bit0=1b'1*/
			reg |= 0x5e71;
			VPint(CR_AHB_CLK) = reg;
			break;
		default:
			break;
	}
	return;
}
/*_____________________________________________________________________________
**      function name: switch_selfReflashMode
**      descriptions:
**			disable/enable dram self refresh mode
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/

void switch_selfReflashMode(int flag)
{
	uint32 reg = 0;
	
	reg = VPint(CR_DMC_DDR_SR);//BFB20018
	switch(flag){
		case ON_2_IDLE:	
			/*set bit4=1b'1*/
			reg |= 0x1<<4;
			VPint(CR_DMC_DDR_SR) = reg;
			break;
		case IDLE_2_ON:	
			/*set bit4=1b'0*/
			reg &= ~(0x1<<4);
			VPint(CR_DMC_DDR_SR) = reg;
			break;
		default:
			break;
	}
	return;
}
/*_____________________________________________________________________________
**      function name: change_normal2Idle_mode
**      descriptions:
**			change normal mode to idle mode
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/

void change_normal2Idle_mode(void)
{
	uint32 value = 0;
	unsigned long flags = 0;
	unsigned long vpflags = 0;
	
	local_irq_save(flags);
	vpflags=dvpe();
	switch_selfReflashMode(ON_2_IDLE);
	switch_ratio(ON_2_IDLE);
	if(mode_switch==SET_FAIL){
		printk("switch normal to idle mode FAILED\n");
		goto exit;
	}
	switch_pcie_phy_aspm_func(ON_2_IDLE);
	switch_ahb_bus_clk(ON_2_IDLE);
	switch_power_and_pcie1Clk(ON_2_IDLE);
	#ifdef TCSUPPORT_RA_HWNAT	
	VPint(0xbfb50248) = 0x40400040; //extend hwnat refresh time to 64ms
	#endif
exit:
	evpe(vpflags);
	local_irq_restore(flags);

	return;
}

void
delay1us(
	int us
)
{
	volatile uint32 timer_now, timer_last;
	volatile uint32 tick_acc;
	uint32 one_tick_unit = 1 * SYS_HCLK / 2;
	volatile uint32 tick_wait = us * one_tick_unit;
	volatile uint32 timer1_ldv = VPint(CR_TIMER1_LDV);

	tick_acc = 0;
 	timer_last = VPint(CR_TIMER1_VLR);
	do {
   		timer_now = VPint(CR_TIMER1_VLR);
       	if (timer_last >= timer_now)
       		tick_acc += timer_last - timer_now;
      	else
       		tick_acc += timer1_ldv - timer_now + timer_last;
     	timer_last = timer_now;
	} while (tick_acc < tick_wait);
}/*end delay1us*/

/*_____________________________________________________________________________
**      function name: change_idle2Normal_mode
**      descriptions:
**			change idle mode to normal mode
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/

void change_idle2Normal_mode(void)
{
	unsigned long flags = 0;
	unsigned long vpflags = 0;
	
	local_irq_save(flags);
	vpflags=dvpe();

	switch_power_and_pcie1Clk(IDLE_2_ON);
	delay1us(1);
	switch_ratio(IDLE_2_ON);
	if(mode_switch==SET_FAIL){
		printk("switch idle to normal mode failed \n");
		goto exit;
	}
	#ifdef TCSUPPORT_RA_HWNAT
	/*restore hwnat refresh time to 1ms*/
	VPint(0xbfb50248) = 0x01010001;
	#endif
	switch_selfReflashMode(IDLE_2_ON);
	switch_pcie_phy_aspm_func(IDLE_2_ON);
	switch_ahb_bus_clk(IDLE_2_ON);
exit:
	evpe(vpflags);
	local_irq_restore(flags);

	return;
}
/*_____________________________________________________________________________
**      function name: mode_switch_read_proc
**      descriptions:
**			read proc for mode switch, 0:change normal/idle mode success 1:failed
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      	
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/

static int mode_switch_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len=0;

	len = sprintf(buf, "%d\n", mode_switch);

	len -= off;
	*start = buf + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;

}
/*_____________________________________________________________________________
**      function name: mode_switch_write_proc
**      descriptions:
**			write proc for mode switch, write 0: change to idle mode, write 1: change to normal mode
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      	
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/

static int mode_switch_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char get_buf[8]={0};

	if (count > sizeof(get_buf) - 1)
		return -EINVAL;

	if(copy_from_user(get_buf,buffer,count))
		return -EFAULT;
	
	get_buf[count]='\0';
	
	if(get_buf[0] == '0'){
		change_normal2Idle_mode();		
	}
	else if(get_buf[0] == '1'){
		change_idle2Normal_mode();		
	}
	
	return count;
}
/*_____________________________________________________________________________
**      function name: dram_type_read_proc
**      descriptions:
**			get dram type:SDR/DDR1/DDR2
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      	
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/

static int dram_type_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len=0;
	unsigned long *ptr=NULL;

	ptr = CR_AHB_HWCONF;
	len = sprintf(buf, "%x\n", (unsigned long)*ptr);

	len -= off;
	*start = buf + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;

}
/*_____________________________________________________________________________
**      function name: pwm_start_read_proc
**      descriptions:
**			read proc for start pwm function 
**
**      parameters:
**            None
**
**      global:
**            pwm_enable
**
**      return:
**      	
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/
static int pwm_start_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len =0;
	
	len = sprintf(buf, "%d\n", pwm_enable);

	len -= off;
	*start = buf + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;

}

/*_____________________________________________________________________________
**      function name: pwm_start_write_proc
**      descriptions:
**				write proc for time wait before change normal mode to idle mode 
**
**      parameters:
**            None
**
**      global:
**            down_freq_time
**
**      return:
**      	
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/
static int pwm_start_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char get_buf[8]={0};

	if (count > sizeof(get_buf) - 1)
		return -EINVAL;

	if(copy_from_user(get_buf,buffer,count))
		return -EFAULT;
	
	get_buf[count]='\0';
	
	if (sscanf(get_buf, "%d", &pwm_enable) != 1) {
		return count;
	}

	return count;

}

/*_____________________________________________________________________________
**      function name: pwm_init
**      descriptions:
**			pwm module init
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      	
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/


static int __init pwm_init(void)
{
	struct proc_dir_entry *pwm_proc=NULL;

	printk("Power Manager 0.1 init\n");

	uint32 def_dmcDdr_reg = VPint(CR_DMC_DDR_CFG0);
	
	pwm_proc = create_proc_entry("tc3162/mode_switch", 0, NULL);
	pwm_proc->read_proc = mode_switch_read_proc;
	pwm_proc->write_proc = mode_switch_write_proc;
	pwm_proc = create_proc_entry("tc3162/dram_type", 0, NULL);
	pwm_proc->read_proc = dram_type_read_proc;
	pwm_proc->write_proc = NULL;
	pwm_proc = create_proc_entry("tc3162/pwm_start",0,NULL);
	pwm_proc->read_proc = pwm_start_read_proc;
	pwm_proc->write_proc = pwm_start_write_proc;

	return 0;
}
/*_____________________________________________________________________________
**      function name: pwm_exit
**      descriptions:
**			pwm module exit
**
**      parameters:
**            None
**
**      global:
**            None
**
**      return:
**      	
**		call:
**   	 None
**
**      revision:
**      1. shelven.lu 2012/05/11
**____________________________________________________________________________
*/

static void __exit pwm_exit(void)
{
	printk("Power Manager 0.1 exit\n");

	remove_proc_entry("tc3162/mode_switch", 0);
	remove_proc_entry("tc3162/dram_type", 0);
	remove_proc_entry("tc3162/pwm_start", 0);
}

module_init (pwm_init);
module_exit (pwm_exit);
MODULE_LICENSE("Proprietary");

