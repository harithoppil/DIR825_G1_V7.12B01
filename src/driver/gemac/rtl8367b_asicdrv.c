/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 28599 $
 * $Date: 2012-05-07 09:41:37 +0800 (星期一, 07 五月 2012) $
 *
 * Purpose : RTL8367B switch high-level API for RTL8367B
 * Feature :
 *
 */
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/etherdevice.h>
#include <linux/mii.h>
#include <linux/spinlock.h>
#include "gemac.h" 
#include "smi.h"
#include "rtl8367b_asicdrv.h"

static spinlock_t smi_lock;

void smi_init(void)
{
	spin_lock_init(&smi_lock);
}

void smi_read(unsigned int mAddrs, unsigned int *rData)
{
	unsigned int data;
	
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);/* Write Start command to register 29 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL0_REG, MDC_MDIO_ADDR_OP);/* Write address control code to register 31 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);/* Write Start command to register 29 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_ADDRESS_REG, mAddrs);/* Write address to register 23 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);/* Write Start command to register 29 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL1_REG, MDC_MDIO_READ_OP);/* Write read control code to register 21 */    
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);/* Write Start command to register 29 */
	data = miiStationRead(MDC_MDIO_DUMMY_ID, MDC_MDIO_DATA_READ_REG);/* Read data from register 25 */
	*rData = data;
}



void smi_write(unsigned int mAddrs, unsigned int rData)
{
	//printk("%s: addr=%#x, data=%#x\n", __func__, mAddrs, rData);
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);/* Write Start command to register 29 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL0_REG, MDC_MDIO_ADDR_OP);/* Write address control code to register 31 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);/* Write Start command to register 29 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_ADDRESS_REG, mAddrs);/* Write address to register 23 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP); /* Write Start command to register 29 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_DATA_WRITE_REG, rData);/* Write data to register 24 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_START_REG, MDC_MDIO_START_OP);/* Write Start command to register 29 */
	miiStationWrite(MDC_MDIO_DUMMY_ID, MDC_MDIO_CTRL1_REG, MDC_MDIO_WRITE_OP);/* Write data control code to register 21 */
}

/* Function Description:
 *      Set content of asic register
 * Input:
 *      reg 	- register's address
 *      value 	- Value setting to register
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      The value will be set to ASIC mapping address only and it is always return RT_ERR_OK while setting un-mapping address registers
 */
int rtl8367b_setAsicReg(unsigned int reg, unsigned int value)
{
	unsigned long flags;
	
	spin_lock_irqsave(&smi_lock, flags);
	smi_write(reg, value);
	spin_unlock_irqrestore(&smi_lock, flags);
	
	return RT_ERR_OK;
}
/* Function Description:
 *      Get content of asic register
 * Input:
 *      reg 	- register's address
 *      value 	- Value setting to register
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      Value 0x0000 will be returned for ASIC un-mapping address
 */
int rtl8367b_getAsicReg(unsigned int reg, unsigned int *pValue)
{
	unsigned long flags;

	spin_lock_irqsave(&smi_lock, flags);
	smi_read(reg, pValue);
	spin_unlock_irqrestore(&smi_lock, flags);
	
    return RT_ERR_OK;
}

/* Function Description:
 *      Set a bit value of a specified register
 * Input:
 *      reg 	- register's address
 *      bit 	- bit location
 *      value 	- value to set. It can be value 0 or 1.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT  	- Invalid input parameter
 * Note:
 *      Set a bit of a specified register to 1 or 0.
 */
int rtl8367b_setAsicRegBit(unsigned int reg, unsigned int bit, unsigned int value)
{
	unsigned int regData;
	int ret_val = RT_ERR_INPUT;
	unsigned long flags;

	if(bit < RTL8367B_REGBITLENGTH) {
		spin_lock_irqsave(&smi_lock, flags);
		smi_read(reg, &regData);
		if(0 != value) {
			regData = regData | (1 << bit);
		} else {
			regData = regData & (~(1 << bit));
		}
		smi_write(reg, regData);
		spin_unlock_irqrestore(&smi_lock, flags);
		ret_val = RT_ERR_OK;
	}
	
	return ret_val;
}
/* Function Description:
 *      Get a bit value of a specified register
 * Input:
 *      reg 	- register's address
 *      bit 	- bit location
 *      value 	- value to get.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT  	- Invalid input parameter
 */
int rtl8367b_getAsicRegBit(unsigned int reg, unsigned int bit, unsigned int *pValue)
{
	unsigned int regData;
	int ret_val = RT_ERR_SMI;

	ret_val = rtl8367b_getAsicReg(reg, &regData);
	if(RT_ERR_OK == ret_val) {
		*pValue = (regData & (0x1 << bit)) >> bit;
	}
	return ret_val;
}
/* Function Description:
 *      Set bits value of a specified register
 * Input:
 *      reg 	- register's address
 *      bits 	- bits mask for setting
 *      value 	- bits value for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT  	- Invalid input parameter
 * Note:
 *      Set bits of a specified register to value. Both bits and value are be treated as bit-mask
 */
int rtl8367b_setAsicRegBits(unsigned int reg, unsigned int bits, unsigned int value)
{
	unsigned int regData;
	int ret_val = RT_ERR_INPUT;
	unsigned int bitsShift = 0;
	unsigned int valueShifted;
	unsigned long flags;

	if(bits < (1 << RTL8367B_REGBITLENGTH)) {
		while(!(bits & (1 << bitsShift))) {
			bitsShift++;
			if(bitsShift >= RTL8367B_REGBITLENGTH) {
				goto out;
			}
		}
		valueShifted = value << bitsShift;
		if(valueShifted > RTL8367B_REGDATAMAX) {
			goto out;
		}
		spin_lock_irqsave(&smi_lock, flags);
		smi_read(reg, &regData);
		regData = regData & (~bits);
		regData = regData | (valueShifted & bits);
		smi_write(reg, regData);
		spin_unlock_irqrestore(&smi_lock, flags);
		ret_val = RT_ERR_OK;
	}
out:
	return ret_val;
}
/* Function Description:
 *      Get bits value of a specified register
 * Input:
 *      reg 	- register's address
 *      bits 	- bits mask for setting
 *      value 	- bits value for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT  	- Invalid input parameter
 */
int rtl8367b_getAsicRegBits(unsigned int reg, unsigned int bits, unsigned int *pValue)
{
	unsigned int regData;
	int ret_val = RT_ERR_INPUT;
	unsigned int bitsShift = 0;

	if(bits < (1<<RTL8367B_REGBITLENGTH)) {
		while(!(bits & (1 << bitsShift))) {
			bitsShift++;
			if(bitsShift >= RTL8367B_REGBITLENGTH) {
				goto out;
			}
		}
		ret_val = rtl8367b_getAsicReg(reg, &regData);
		if(RT_ERR_OK == ret_val) {
			*pValue = (regData & bits) >> bitsShift;
		}
	}
out:
	return ret_val;
}


