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
 * Feature : ACL related function drivers
 *
 */
#include <linux/string.h> 
#include "rtl8367b_asicdrv_acl.h"

#if defined(CONFIG_RTL8367B_ASICDRV_TEST)
rtl8367b_aclrulesmi Rtl8370sVirtualAclRuleTable[RTL8367B_ACLRULENO];
rtl8367b_acl_act_smi_t Rtl8370sVirtualAclActTable[RTL8367B_ACLRULENO]; 
#endif

void _rtl8367b_aclRuleStSmi2User(rtl8367b_aclrule *pAclUser, rtl8367b_aclrulesmi *pAclSmi);
void _rtl8367b_aclRuleStUser2Smi(rtl8367b_aclrule *pAclUser, rtl8367b_aclrulesmi *pAclSmi);
void _rtl8367b_aclActStSmi2User(rtl8367b_acl_act_t *pAclUser, rtl8367b_acl_act_smi_t *pAclSmi);
void _rtl8367b_aclActStUser2Smi(rtl8367b_acl_act_t *pAclUser, rtl8367b_acl_act_smi_t *pAclSmi);

/* Function Name:
 *      rtl8367b_setAsicAcl
 * Description:
 *      Set port acl function enable/disable
 * Input:
 *      port 	- Physical port number (0~7)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_setAsicAcl(unsigned int port, unsigned int enabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	return rtl8367b_setAsicRegBit(RTL8367B_ACL_ENABLE_REG, port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicAcl
 * Description:
 *      Get port acl function enable/disable
 * Input:
 *      port 	- Physical port number (0~7)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicAcl(unsigned int port, unsigned int* pEnabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	return rtl8367b_getAsicRegBit(RTL8367B_ACL_ENABLE_REG, port, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicAclUnmatchedPermit
 * Description:
 *      Set port acl function unmatched permit action
 * Input:
 *      port 	- Physical port number (0~7)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_setAsicAclUnmatchedPermit(unsigned int port, unsigned int enabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	return rtl8367b_setAsicRegBit(RTL8367B_ACL_UNMATCH_PERMIT_REG, port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicAclUnmatchedPermit
 * Description:
 *      Get port acl function unmatched permit action
 * Input:
 *      port 	- Physical port number (0~7)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
int rtl8367b_getAsicAclUnmatchedPermit(unsigned int port, unsigned int* pEnabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	return rtl8367b_getAsicRegBit(RTL8367B_ACL_UNMATCH_PERMIT_REG, port, pEnabled);
}

/*
	Exchange structure type define with MMI and SMI
*/
void _rtl8367b_aclRuleStSmi2User( rtl8367b_aclrule *pAclUser, rtl8367b_aclrulesmi *pAclSmi)
{
   	unsigned char *care_ptr, *data_ptr;
    unsigned char care_tmp, data_tmp;
    unsigned int i;    
    
	pAclUser->data_bits.active_portmsk = pAclSmi->data_bits.active_portmsk;
	pAclUser->data_bits.type = pAclSmi->data_bits.type;
	pAclUser->data_bits.tag_exist = pAclSmi->data_bits.tag_exist;

    care_ptr = (unsigned char*)&pAclSmi->care_bits;
    data_ptr = (unsigned char*)&pAclSmi->data_bits;
    for ( i = 0; i < sizeof(struct acl_rule_smi_st); i++)    
    {
        care_tmp = *(care_ptr + i) ^ (*(data_ptr + i));
        data_tmp = *(data_ptr + i);

        *(care_ptr + i) = care_tmp;
        *(data_ptr + i) = data_tmp;
    }  


	for(i = 0; i < RTL8367B_ACLRULEFIELDNO; i++)
		pAclUser->data_bits.field[i] = pAclSmi->data_bits.field[i];

	pAclUser->valid = pAclSmi->valid;

	pAclUser->care_bits.active_portmsk = pAclSmi->care_bits.active_portmsk;
	pAclUser->care_bits.type = pAclSmi->care_bits.type;
	pAclUser->care_bits.tag_exist = pAclSmi->care_bits.tag_exist;

	for(i = 0; i < RTL8367B_ACLRULEFIELDNO; i++)
		pAclUser->care_bits.field[i] = pAclSmi->care_bits.field[i];
	
}

/*
	Exchange structure type define with MMI and SMI
*/
void _rtl8367b_aclRuleStUser2Smi(rtl8367b_aclrule *pAclUser, rtl8367b_aclrulesmi *pAclSmi)
{
    unsigned char *care_ptr, *data_ptr;
    unsigned char care_tmp, data_tmp;
    unsigned int i;  
    
	pAclSmi->data_bits.active_portmsk = pAclUser->data_bits.active_portmsk;
	pAclSmi->data_bits.type = pAclUser->data_bits.type;
	pAclSmi->data_bits.tag_exist = pAclUser->data_bits.tag_exist;

	for(i = 0;i < RTL8367B_ACLRULEFIELDNO; i++)
		pAclSmi->data_bits.field[i] = pAclUser->data_bits.field[i];

	pAclSmi->valid = pAclUser->valid;

	pAclSmi->care_bits.active_portmsk = pAclUser->care_bits.active_portmsk;
	pAclSmi->care_bits.type = pAclUser->care_bits.type;
	pAclSmi->care_bits.tag_exist = pAclUser->care_bits.tag_exist;

	for(i = 0; i < RTL8367B_ACLRULEFIELDNO; i++)
		pAclSmi->care_bits.field[i] = pAclUser->care_bits.field[i];

    care_ptr = (unsigned char*)&pAclSmi->care_bits;
    data_ptr = (unsigned char*)&pAclSmi->data_bits;
    for ( i = 0; i < sizeof(struct acl_rule_smi_st); i++)    
    {   
        care_tmp = *(care_ptr + i) & ~(*(data_ptr + i));
        data_tmp = *(care_ptr + i) & *(data_ptr + i);

        *(care_ptr + i) = care_tmp;
        *(data_ptr + i) = data_tmp;
    }
}

/*
	Exchange structure type define with MMI and SMI
*/
void _rtl8367b_aclActStSmi2User(rtl8367b_acl_act_t *pAclUser, rtl8367b_acl_act_smi_t *pAclSmi)
{
	pAclUser->cact = pAclSmi->cact;
	pAclUser->cvidx_cact = pAclSmi->cvidx_cact;

	pAclUser->sact = pAclSmi->sact;
	pAclUser->svidx_sact = pAclSmi->svidx_sact;

	pAclUser->aclmeteridx = pAclSmi->aclmeteridx;

	pAclUser->fwdact = pAclSmi->fwdact;
	pAclUser->fwdpmask = pAclSmi->fwdpmask;

	pAclUser->priact = pAclSmi->priact;
	pAclUser->pridx = pAclSmi->pridx;

	pAclUser->aclint = pAclSmi->aclint;
	pAclUser->gpio_en = pAclSmi->gpio_en;
	pAclUser->gpio_pin = pAclSmi->gpio_pin;
}

/*
	Exchange structure type define with MMI and SMI
*/
void _rtl8367b_aclActStUser2Smi(rtl8367b_acl_act_t *pAclUser, rtl8367b_acl_act_smi_t *pAclSmi)
{

	pAclSmi->cact = pAclUser->cact;
	pAclSmi->cvidx_cact = pAclUser->cvidx_cact;

	pAclSmi->sact = pAclUser->sact;
	pAclSmi->svidx_sact = pAclUser->svidx_sact;

	pAclSmi->aclmeteridx = pAclUser->aclmeteridx;

	pAclSmi->fwdact = pAclUser->fwdact;
	pAclSmi->fwdpmask = pAclUser->fwdpmask;

	pAclSmi->priact = pAclUser->priact;
	pAclSmi->pridx = pAclUser->pridx;

	pAclSmi->aclint = pAclUser->aclint;
	pAclSmi->gpio_en = pAclUser->gpio_en;
	pAclSmi->gpio_pin = pAclUser->gpio_pin;
}
/* Function Name:
 *      rtl8367b_setAsicAclRule
 * Description:
 *      Set acl rule content
 * Input:
 *      index 	- ACL rule index (0-63) of 64 ACL rules
 *      pAclRule - ACL rule stucture for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *		System supported 64 shared 289-bit ACL ingress rule. Index was available at range 0-63 only. 
 *		If software want to modify ACL rule, the ACL function should be disable at first or unspecify 
 *		acl action will be executed. 
 *		One ACL rule structure has three parts setting:
 *		Bit 0-143		Data Bits of this Rule
 *		Bit	144		Valid Bit
 *		Bit 145-288	Care Bits of this Rule
 *		There are four kinds of field in Data Bits and Care Bits: Active Portmask, Type, Tag Exist, and 8 fields
 */
int rtl8367b_setAsicAclRule(unsigned int index, rtl8367b_aclrule* pAclRule)
{
	rtl8367b_aclrulesmi aclRuleSmi;
	unsigned short* tableAddr;
	unsigned int regAddr;
	unsigned int	regData;
	unsigned int i;
	int retVal;
	
	if(index > RTL8367B_ACLRULEMAX)
		return RT_ERR_OUT_OF_RANGE;

    memset(&aclRuleSmi, 0x00, sizeof(rtl8367b_aclrulesmi));
	
 	_rtl8367b_aclRuleStUser2Smi(pAclRule, &aclRuleSmi);

    /* Write ACS_ADR register for data bits */
    regAddr = RTL8367B_TABLE_ACCESS_ADDR_REG;
    regData = RTL8367B_ACLRULETBADDR(DATABITS, index);
    retVal = rtl8367b_setAsicReg(regAddr,regData);
    if(retVal !=RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_TABLE_ACCESS_WRDATA_REG(RTL8367B_ACLRULETBLEN), 0x1, 0);
    if(retVal !=RT_ERR_OK)
        return retVal;

    regAddr = RTL8367B_TABLE_ACCESS_CTRL_REG;
    regData = RTL8367B_TABLE_ACCESS_REG_DATA(TB_OP_WRITE, TB_TARGET_ACLRULE);
    retVal = rtl8367b_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;


  	/* Write ACS_ADR register */
	regAddr = RTL8367B_TABLE_ACCESS_ADDR_REG;
	regData = RTL8367B_ACLRULETBADDR(CAREBITS, index);
	retVal = rtl8367b_setAsicReg(regAddr, regData);
	if(retVal != RT_ERR_OK)
		return retVal;
	
	
	retVal = rtl8367b_setAsicReg(regAddr, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Write Care Bits to ACS_DATA registers */
	 tableAddr = (unsigned short*)&aclRuleSmi.care_bits;
	 regAddr = RTL8367B_TABLE_ACCESS_WRDATA_BASE;

	for(i = 0; i < RTL8367B_ACLRULETBLEN; i++)
	{
		regData = *tableAddr;
		retVal = rtl8367b_setAsicReg(regAddr, regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		regAddr++;
		tableAddr++;
	}
	
	/* Write ACS_CMD register */
	regAddr = RTL8367B_TABLE_ACCESS_CTRL_REG;
	regData = RTL8367B_TABLE_ACCESS_REG_DATA(TB_OP_WRITE, TB_TARGET_ACLRULE);
	retVal = rtl8367b_setAsicRegBits(regAddr, RTL8367B_TABLE_TYPE_MASK | RTL8367B_COMMAND_TYPE_MASK,regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Write ACS_ADR register for data bits */
	regAddr = RTL8367B_TABLE_ACCESS_ADDR_REG;
	regData = RTL8367B_ACLRULETBADDR(DATABITS, index);
	retVal = rtl8367b_setAsicReg(regAddr, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Write Data Bits to ACS_DATA registers */
	 tableAddr = (unsigned short*)&aclRuleSmi.data_bits;
	 regAddr = RTL8367B_REG_TABLE_WRITE_DATA0;

	for(i = 0; i < RTL8367B_ACLRULETBLEN; i++)
	{
		regData = *tableAddr;
		retVal = rtl8367b_setAsicReg(regAddr, regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		regAddr++;
		tableAddr++;
	}

	/* Write Valid Bit */
	retVal = rtl8367b_setAsicRegBits(RTL8367B_TABLE_ACCESS_WRDATA_REG(RTL8367B_ACLRULETBLEN), 0x1, aclRuleSmi.valid);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Write ACS_CMD register for care bits*/
	regAddr = RTL8367B_TABLE_ACCESS_CTRL_REG;
	regData = RTL8367B_TABLE_ACCESS_REG_DATA(TB_OP_WRITE, TB_TARGET_ACLRULE);
	retVal = rtl8367b_setAsicRegBits(regAddr, RTL8367B_TABLE_TYPE_MASK | RTL8367B_COMMAND_TYPE_MASK, regData);
	if(retVal != RT_ERR_OK)
		return retVal;
	
#ifdef CONFIG_RTL8367B_ASICDRV_TEST
	memcpy(&Rtl8370sVirtualAclRuleTable[index], &aclRuleSmi, sizeof(rtl8367b_aclrulesmi));
#endif

	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicAclRule
 * Description:
 *      Get acl rule content
 * Input:
 *      index 	- ACL rule index (0-63) of 64 ACL rules
 *      pAclRule - ACL rule stucture for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
  * Note:
 *		None
 */
int rtl8367b_getAsicAclRule(unsigned int index, rtl8367b_aclrule *pAclRule)
{
	rtl8367b_aclrulesmi aclRuleSmi;	
	unsigned int regAddr, regData;
	int retVal;
	unsigned short* tableAddr;
	unsigned int i;

	if(index > RTL8367B_ACLRULEMAX)
		return RT_ERR_OUT_OF_RANGE;

	memset(&aclRuleSmi, 0x00, sizeof(rtl8367b_aclrulesmi));

	/* Write ACS_ADR register for data bits */
	regAddr = RTL8367B_TABLE_ACCESS_ADDR_REG;
	regData = RTL8367B_ACLRULETBADDR(DATABITS, index);
	retVal = rtl8367b_setAsicReg(regAddr, regData);
	if(retVal != RT_ERR_OK)
		return retVal;
	

	/* Write ACS_CMD register */
	regAddr = RTL8367B_TABLE_ACCESS_CTRL_REG;
	regData = RTL8367B_TABLE_ACCESS_REG_DATA(TB_OP_READ, TB_TARGET_ACLRULE);
	retVal = rtl8367b_setAsicRegBits(regAddr, RTL8367B_TABLE_TYPE_MASK | RTL8367B_COMMAND_TYPE_MASK, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Read Data Bits */
	regAddr = RTL8367B_TABLE_ACCESS_RDDATA_BASE;
	tableAddr = (unsigned short*)&aclRuleSmi.data_bits;
	for(i = 0; i < RTL8367B_ACLRULETBLEN; i++)
	{
		retVal = rtl8367b_getAsicReg(regAddr, &regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		*tableAddr = regData;
		
		regAddr ++;
		tableAddr ++;
	}
	
	/* Read Valid Bit */
	retVal = rtl8367b_getAsicReg(RTL8367B_TABLE_ACCESS_RDDATA_REG(RTL8367B_ACLRULETBLEN), &regData);
	if(retVal != RT_ERR_OK)
		return retVal;
	
	aclRuleSmi.valid = regData & 0x1;

	/* Write ACS_ADR register for carebits*/
	regAddr = RTL8367B_TABLE_ACCESS_ADDR_REG;
	regData = RTL8367B_ACLRULETBADDR(CAREBITS, index);
	retVal = rtl8367b_setAsicReg(regAddr, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Write ACS_CMD register */
	regAddr = RTL8367B_TABLE_ACCESS_CTRL_REG;
	regData = RTL8367B_TABLE_ACCESS_REG_DATA(TB_OP_READ, TB_TARGET_ACLRULE);
	retVal = rtl8367b_setAsicRegBits(regAddr, RTL8367B_TABLE_TYPE_MASK | RTL8367B_COMMAND_TYPE_MASK, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Read Care Bits */
	regAddr = RTL8367B_TABLE_ACCESS_RDDATA_BASE;
	tableAddr = (unsigned short*)&aclRuleSmi.care_bits;	
	for(i = 0; i < RTL8367B_ACLRULETBLEN; i++)
	{
		retVal = rtl8367b_getAsicReg(regAddr, &regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		*tableAddr = regData;
		
		regAddr ++;
		tableAddr ++;
	}
	
#ifdef CONFIG_RTL8367B_ASICDRV_TEST
	memcpy(&aclRuleSmi,&Rtl8370sVirtualAclRuleTable[index], sizeof(rtl8367b_aclrulesmi));
#endif

	 _rtl8367b_aclRuleStSmi2User(pAclRule, &aclRuleSmi);

	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicAclNot
 * Description:
 *      Set rule comparison result inversion / no inversion
 * Input:
 *      index 	- ACL rule index (0-63) of 64 ACL rules
 *      not 	- 1: inverse, 0: don't inverse
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *		None
 */
int rtl8367b_setAsicAclNot(unsigned int index, unsigned int not)
{
	if(index > RTL8367B_ACLRULEMAX)
		return RT_ERR_OUT_OF_RANGE;
	

	return rtl8367b_setAsicRegBit(RTL8367B_ACL_ACTION_CTRL_REG(index), RTL8367B_ACL_OP_NOT_OFFSET(index), not);
}
/* Function Name:
 *      rtl8367b_getAsicAcl
 * Description:
 *      Get rule comparison result inversion / no inversion
 * Input:
 *      index 	- ACL rule index (0-63) of 64 ACL rules
 *      pNot 	- 1: inverse, 0: don't inverse
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *		None
 */
int rtl8367b_getAsicAclNot(unsigned int index, unsigned int* pNot)
{
	if(index > RTL8367B_ACLRULEMAX)
		return RT_ERR_OUT_OF_RANGE;

	return rtl8367b_getAsicRegBit(RTL8367B_ACL_ACTION_CTRL_REG(index), RTL8367B_ACL_OP_NOT_OFFSET(index), pNot);
}
/* Function Name:
 *      rtl8367b_setAsicAclTemplate
 * Description:
 *      Set fields of a ACL Template
 * Input:
 *      index 	- ACL rule index (0-63) of 64 ACL rules
 *      pAclType - ACL type stucture for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *	    The API can set type field of the 5 ACL rule templates.
 *		Each type has 8 fields. One field means what data in one field of a ACL rule means
 *		8 fields of ACL rule 0~63 is descripted by one type in ACL group
 */
int rtl8367b_setAsicAclTemplate(unsigned int index, rtl8367b_acltemplate_t* pAclType)
{	
	int retVal;
	unsigned int i;
	unsigned int regAddr, regData;
	
	if(index >= RTL8367B_ACLTEMPLATENO)
		return RT_ERR_OUT_OF_RANGE;

	regAddr = RTL8367B_ACL_RULE_TEMPLATE_CTRL_REG(index);
	for(i = 0; i < (RTL8367B_ACLRULEFIELDNO/2); i++)
    {
    	regData = pAclType->field[i*2+1];
		regData = regData << 8 | pAclType->field[i*2];
		
		retVal = rtl8367b_setAsicReg(regAddr + i, regData);		

		if(retVal != RT_ERR_OK)
	        return retVal;
	}

	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicAclTemplate
 * Description:
 *      Get fields of a ACL Template
 * Input:
 *      index 	- ACL rule index (0-63) of 64 ACL rules
 *      pAclType - ACL type stucture for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *	    None
 */
int rtl8367b_getAsicAclTemplate(unsigned int index, rtl8367b_acltemplate_t *pAclType)
{
	int retVal;
	unsigned int i;
	unsigned int regData, regAddr;
	
	if(index >= RTL8367B_ACLTEMPLATENO)
		return RT_ERR_OUT_OF_RANGE;

	regAddr = RTL8367B_ACL_RULE_TEMPLATE_CTRL_REG(index);

	for(i = 0; i < (RTL8367B_ACLRULEFIELDNO/2); i++)
	{
		retVal = rtl8367b_getAsicReg(regAddr + i,&regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		pAclType->field[i*2] = regData & 0xFF;
		pAclType->field[i*2 + 1] = (regData >> 8) & 0xFF;
	}
	
	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicAclAct
 * Description:
 *      Set ACL rule matched Action
 * Input:
 *      index 	- ACL rule index (0-63) of 64 ACL rules
 *      pAclAct 	- ACL action stucture for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *	    None
 */
int rtl8367b_setAsicAclAct(unsigned int index, rtl8367b_acl_act_t* pAclAct)
{
	rtl8367b_acl_act_smi_t aclActSmi;
	int retVal;
	unsigned int regAddr, regData;
	unsigned short* tableAddr;
	unsigned int i;
	
	if(index > RTL8367B_ACLRULEMAX)
		return RT_ERR_OUT_OF_RANGE;
	
	memset(&aclActSmi, 0x00, sizeof(rtl8367b_acl_act_smi_t));
	 _rtl8367b_aclActStUser2Smi(pAclAct, &aclActSmi);


	/* Write ACS_ADR register for data bits */
	regAddr = RTL8367B_TABLE_ACCESS_ADDR_REG;
	regData = index;
	retVal = rtl8367b_setAsicReg(regAddr, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Write Data Bits to ACS_DATA registers */
	 tableAddr = (unsigned short*)&aclActSmi;
	 regAddr = RTL8367B_TABLE_ACCESS_WRDATA_BASE;

	for(i = 0; i < RTL8367B_ACLACTTBLEN; i++)
	{
		regData = *tableAddr;
		retVal = rtl8367b_setAsicReg(regAddr, regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		regAddr++;
		tableAddr++;
	}
	
	/* Write ACS_CMD register for care bits*/
	regAddr = RTL8367B_TABLE_ACCESS_CTRL_REG;
	regData = RTL8367B_TABLE_ACCESS_REG_DATA(TB_OP_WRITE, TB_TARGET_ACLACT);
	retVal = rtl8367b_setAsicRegBits(regAddr, RTL8367B_TABLE_TYPE_MASK | RTL8367B_COMMAND_TYPE_MASK, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

#ifdef CONFIG_RTL8367B_ASICDRV_TEST
	Rtl8370sVirtualAclActTable[index] = aclActSmi;
#endif

	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicAclAct
 * Description:
 *      Get ACL rule matched Action
 * Input:
 *      index 	- ACL rule index (0-63) of 64 ACL rules
 *      pAclAct 	- ACL action stucture for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
  * Note:
 *	    None
 */
int rtl8367b_getAsicAclAct(unsigned int index, rtl8367b_acl_act_t *pAclAct)
{
	rtl8367b_acl_act_smi_t aclActSmi;
	int retVal;
	unsigned int regAddr, regData;
	short* tableAddr;
	unsigned int i;
	
	if(index > RTL8367B_ACLRULEMAX)
		return RT_ERR_OUT_OF_RANGE;
	
	memset(&aclActSmi, 0x00, sizeof(rtl8367b_acl_act_smi_t));


	/* Write ACS_ADR register for data bits */
	regAddr = RTL8367B_TABLE_ACCESS_ADDR_REG;
	regData = index;
	retVal = rtl8367b_setAsicReg(regAddr, regData);
	if(retVal != RT_ERR_OK)
		return retVal;
	

	/* Write ACS_CMD register */
	regAddr = RTL8367B_TABLE_ACCESS_CTRL_REG;
	regData = RTL8367B_TABLE_ACCESS_REG_DATA(TB_OP_READ, TB_TARGET_ACLACT);
	retVal = rtl8367b_setAsicRegBits(regAddr, RTL8367B_TABLE_TYPE_MASK | RTL8367B_COMMAND_TYPE_MASK, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Read Data Bits */
	regAddr = RTL8367B_TABLE_ACCESS_RDDATA_BASE;
	tableAddr = (unsigned short*)&aclActSmi;
	for(i = 0; i < RTL8367B_ACLACTTBLEN; i++)
	{
		retVal = rtl8367b_getAsicReg(regAddr, &regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		*tableAddr = regData;
		
		regAddr ++;
		tableAddr ++;
	}

#ifdef CONFIG_RTL8367B_ASICDRV_TEST
	aclActSmi = Rtl8370sVirtualAclActTable[index];
#endif

	 _rtl8367b_aclActStSmi2User(pAclAct, &aclActSmi);
	 
	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicAclActCtrl
 * Description:
 *      Set ACL rule matched Action Control Bits
 * Input:
 *      index 		- ACL rule index (0-63) of 64 ACL rules
 *      aclActCtrl 	- 6 ACL Control Bits
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *	    ACL Action Control Bits Indicate which actions will be take when a rule matches
 */
int rtl8367b_setAsicAclActCtrl(unsigned int index, unsigned int aclActCtrl)
{
	if(index > RTL8367B_ACLRULEMAX)
		return RT_ERR_OUT_OF_RANGE;

	return rtl8367b_setAsicRegBits(RTL8367B_ACL_ACTION_CTRL_REG(index), RTL8367B_ACL_OP_ACTION_MASK(index), aclActCtrl);
}
/* Function Name:
 *      rtl8367b_getAsicAclActCtrl
 * Description:
 *      Get ACL rule matched Action Control Bits
 * Input:
 *      index 		- ACL rule index (0-63) of 64 ACL rules
 *      pAclActCtrl 	- 6 ACL Control Bits
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *	    None
 */
int rtl8367b_getAsicAclActCtrl(unsigned int index, unsigned int *pAclActCtrl)
{
	int retVal;
	unsigned int regData;
	
	if(index > RTL8367B_ACLRULEMAX)
		return RT_ERR_OUT_OF_RANGE;

	retVal = rtl8367b_getAsicRegBits(RTL8367B_ACL_ACTION_CTRL_REG(index), RTL8367B_ACL_OP_ACTION_MASK(index), &regData);

	if(retVal != RT_ERR_OK)
		return retVal;

	*pAclActCtrl = regData;

	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicAclPortRange
 * Description:
 *      Set ACL TCP/UDP range check
 * Input:
 *      index 		- ACL rule index (0-63) of 64 ACL rules
 *      type 		- Range check type
 *      upperPort 	- TCP/UDP port range upper bound
 *      lowerPort 	- TCP/UDP port range lower bound
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *	    None
 */
int rtl8367b_setAsicAclPortRange(unsigned int index, unsigned int type, unsigned int upperPort, unsigned int lowerPort)
{
	int retVal;
	
	if(index > RTL8367B_ACLRANGEMAX)
		return RT_ERR_OUT_OF_RANGE;
    
	retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_ACL_SDPORT_RANGE_ENTRY0_CTRL2 + index*3, RTL8367B_ACL_SDPORT_RANGE_ENTRY0_CTRL2_MASK, type);
	if(retVal != RT_ERR_OK)
		return retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_REG_ACL_SDPORT_RANGE_ENTRY0_CTRL1 + index*3, upperPort);
	if(retVal != RT_ERR_OK)
		return retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_REG_ACL_SDPORT_RANGE_ENTRY0_CTRL0 + index*3, lowerPort);
	if(retVal != RT_ERR_OK)
		return retVal;
	
	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicAclPortRange
 * Description:
 *      Get ACL TCP/UDP range check
 * Input:
 *      index 		- ACL rule index (0-63) of 64 ACL rules
 *      pType 		- Range check type
 *      pUpperPort 	- TCP/UDP port range upper bound
 *      pLowerPort 	- TCP/UDP port range lower bound
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *	    None
 */
int rtl8367b_getAsicAclPortRange(unsigned int index, unsigned int* pType, unsigned int* pUpperPort, unsigned int* pLowerPort)
{
	int retVal;
	
	if(index > RTL8367B_ACLRANGEMAX)
		return RT_ERR_OUT_OF_RANGE;
    
	retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_ACL_SDPORT_RANGE_ENTRY0_CTRL2 + index*3, RTL8367B_ACL_SDPORT_RANGE_ENTRY0_CTRL2_MASK, pType);
	if(retVal != RT_ERR_OK)
		return retVal;

	retVal = rtl8367b_getAsicReg(RTL8367B_REG_ACL_SDPORT_RANGE_ENTRY0_CTRL1 + index*3, pUpperPort);
	if(retVal != RT_ERR_OK)
		return retVal;

	retVal = rtl8367b_getAsicReg(RTL8367B_REG_ACL_SDPORT_RANGE_ENTRY0_CTRL0 + index*3, pLowerPort);
	if(retVal != RT_ERR_OK)
		return retVal;
	
	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicAclVidRange
 * Description:
 *      Set ACL VID range check
 * Input:
 *      index 		- ACL rule index (0-63) of 64 ACL rules
 *      type 		- Range check type
 *      upperVid 	- VID range upper bound
 *      lowerVid 	- VID range lower bound
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *	    None
 */
int rtl8367b_setAsicAclVidRange(unsigned int index, unsigned int type, unsigned int upperVid, unsigned int lowerVid)
{
	int retVal;
	unsigned int regData;
	
	if(index > RTL8367B_ACLRANGEMAX)
		return RT_ERR_OUT_OF_RANGE;

	regData = ((type << RTL8367B_ACL_VID_RANGE_ENTRY0_CTRL1_CHECK0_TYPE_OFFSET) & RTL8367B_ACL_VID_RANGE_ENTRY0_CTRL1_CHECK0_TYPE_MASK) |
				(upperVid & RTL8367B_ACL_VID_RANGE_ENTRY0_CTRL1_CHECK0_HIGH_MASK);
				
	retVal = rtl8367b_setAsicReg(RTL8367B_REG_ACL_VID_RANGE_ENTRY0_CTRL1 + index*2, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_REG_ACL_VID_RANGE_ENTRY0_CTRL0 + index*2, lowerVid);
	if(retVal != RT_ERR_OK)
		return retVal;
	
	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicAclVidRange
 * Description:
 *      Get ACL VID range check
 * Input:
 *      index 		- ACL rule index (0-63) of 64 ACL rules
 *      pType 		- Range check type
 *      pUpperVid 	- VID range upper bound
 *      pLowerVid 	- VID range lower bound
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *	    None
 */
int rtl8367b_getAsicAclVidRange(unsigned int index, unsigned int* pType, unsigned int* pUpperVid, unsigned int* pLowerVid)
{
	int retVal;
	unsigned int regData;
	
	if(index > RTL8367B_ACLRANGEMAX)
		return RT_ERR_OUT_OF_RANGE;
			
	retVal = rtl8367b_getAsicReg(RTL8367B_REG_ACL_VID_RANGE_ENTRY0_CTRL1 + index*2, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	*pType = (regData & RTL8367B_ACL_VID_RANGE_ENTRY0_CTRL1_CHECK0_TYPE_MASK) >> RTL8367B_ACL_VID_RANGE_ENTRY0_CTRL1_CHECK0_TYPE_OFFSET;
	*pUpperVid = regData & RTL8367B_ACL_VID_RANGE_ENTRY0_CTRL1_CHECK0_HIGH_MASK;

	retVal = rtl8367b_getAsicReg(RTL8367B_REG_ACL_VID_RANGE_ENTRY0_CTRL0 + index*2, pLowerVid);
	if(retVal != RT_ERR_OK)
		return retVal;
	
	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicAclIpRange
 * Description:
 *      Set ACL IP range check
 * Input:
 *      index 		- ACL rule index (0-63) of 64 ACL rules
 *      type 		- Range check type
 *      upperIp 	- IP range upper bound
 *      lowerIp 	- IP range lower bound
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *	    None
 */
int rtl8367b_setAsicAclIpRange(unsigned int index, unsigned int type, unsigned int upperIp, unsigned int lowerIp)
{
	int retVal;
	unsigned int regData;
	unsigned int ipData;
	
	if(index > RTL8367B_ACLRANGEMAX)
		return RT_ERR_OUT_OF_RANGE;
			
	retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_ACL_IP_RANGE_ENTRY0_CTRL4 + index*5, RTL8367B_ACL_IP_RANGE_ENTRY0_CTRL4_MASK, type);
	if(retVal != RT_ERR_OK)
		return retVal;
	
	ipData = upperIp;

	regData = ipData & 0xFFFF;
	retVal = rtl8367b_setAsicReg(RTL8367B_REG_ACL_IP_RANGE_ENTRY0_CTRL2 + index*5, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	regData = (ipData>>16) & 0xFFFF;
	retVal = rtl8367b_setAsicReg(RTL8367B_REG_ACL_IP_RANGE_ENTRY0_CTRL3 + index*5, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	ipData = lowerIp;

	regData = ipData & 0xFFFF;
	retVal = rtl8367b_setAsicReg(RTL8367B_REG_ACL_IP_RANGE_ENTRY0_CTRL0 + index*5, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	regData = (ipData>>16) & 0xFFFF;
	retVal = rtl8367b_setAsicReg(RTL8367B_REG_ACL_IP_RANGE_ENTRY0_CTRL1 + index*5, regData);
	if(retVal != RT_ERR_OK)
		return retVal;
	
	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicAclIpRange
 * Description:
 *      Get ACL IP range check
 * Input:
 *      index 		- ACL rule index (0-63) of 64 ACL rules
 *      pType 		- Range check type
 *      pUpperIp 	- IP range upper bound
 *      pLowerIp 	- IP range lower bound
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE  	- Invalid ACL rule index (0-63)
 * Note:
 *	    None
 */
int rtl8367b_getAsicAclIpRange(unsigned int index, unsigned int* pType, unsigned int* pUpperIp, unsigned int* pLowerIp)
{
	int retVal;
	unsigned int regData;
	unsigned int ipData;
	
	if(index > RTL8367B_ACLRANGEMAX)
		return RT_ERR_OUT_OF_RANGE;
			
	retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_ACL_IP_RANGE_ENTRY0_CTRL4 + index*5, RTL8367B_ACL_IP_RANGE_ENTRY0_CTRL4_MASK, pType);
	if(retVal != RT_ERR_OK)
		return retVal;
	
	retVal = rtl8367b_getAsicReg(RTL8367B_REG_ACL_IP_RANGE_ENTRY0_CTRL2 + index*5, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;
	ipData = regData;
	
	
	retVal = rtl8367b_getAsicReg(RTL8367B_REG_ACL_IP_RANGE_ENTRY0_CTRL3 + index*5, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	ipData = (regData <<16) | ipData;
	*pUpperIp = ipData;
	

	retVal = rtl8367b_getAsicReg(RTL8367B_REG_ACL_IP_RANGE_ENTRY0_CTRL0 + index*5, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;
	ipData = regData;
	
	
	retVal = rtl8367b_getAsicReg(RTL8367B_REG_ACL_IP_RANGE_ENTRY0_CTRL1 + index*5, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	ipData = (regData << 16) | ipData;
	*pLowerIp = ipData;
	
	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicAclGpioPolarity
 * Description:
 *      Set ACL Goip control palarity
 * Input:
 *      polarity - 1: High, 0: Low
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      none
 */
int rtl8367b_setAsicAclGpioPolarity(unsigned int polarity)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_ACL_GPIO_POLARITY, RTL8367B_ACL_GPIO_POLARITY_OFFSET, polarity);
}
/* Function Name:
 *      rtl8367b_getAsicAclGpioPolarity
 * Description:
 *      Get ACL Goip control palarity
 * Input:
 *      pPolarity - 1: High, 0: Low
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      none
 */
int rtl8367b_getAsicAclGpioPolarity(unsigned int* pPolarity)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_ACL_GPIO_POLARITY, RTL8367B_ACL_GPIO_POLARITY_OFFSET, pPolarity);
}

