#ifndef _RTL8367B_ASICDRV_ACL_H_
#define _RTL8367B_ASICDRV_ACL_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_ACLRULENO					64
#define RTL8367B_ACLRULEMAX					(RTL8367B_ACLRULENO-1)
#define RTL8367B_ACLRULEFIELDNO			    8
#define RTL8367B_ACLTEMPLATENO				5
#define RTL8367B_ACLTYPEMAX					(RTL8367B_ACLTEMPLATENO-1)

#define RTL8367B_ACLRULETBLEN				9
#define RTL8367B_ACLACTTBLEN				3
#define RTL8367B_ACLRULETBADDR(type, rule)	((type << 6) | rule)

#define ACL_ACT_CVLAN_ENABLE_MASK           0x1
#define ACL_ACT_SVLAN_ENABLE_MASK           0x2
#define ACL_ACT_PRIORITY_ENABLE_MASK    	0x4
#define ACL_ACT_POLICING_ENABLE_MASK    	0x8
#define ACL_ACT_FWD_ENABLE_MASK    			0x10
#define ACL_ACT_INTGPIO_ENABLE_MASK    		0x20

#define RTL8367B_ACLRULETAGBITS				5

#define RTL8367B_ACLRANGENO					16
#define RTL8367B_ACLRANGEMAX				(RTL8367B_ACLRANGENO-1)

#define RTL8367B_ACL_PORTRANGEMAX           (0xFFFF)

enum ACLTCAMTYPES
{
	CAREBITS= 0,
	DATABITS
};

typedef enum aclFwdAct
{
    RTL8367B_ACL_FWD_MIRROR = 0,
    RTL8367B_ACL_FWD_REDIRECT,
    RTL8367B_ACL_FWD_MIRRORFUNTION,
    RTL8367B_ACL_FWD_TRAP,
} rtl8367b_aclFwd_t;

enum ACLFIELDTYPES
{
	ACL_UNUSED,
	ACL_DMAC0,
	ACL_DMAC1,
	ACL_DMAC2,
	ACL_SMAC0,
	ACL_SMAC1,
	ACL_SMAC2,
	ACL_ETHERTYPE,
	ACL_STAG,
	ACL_CTAG,
	ACL_IP4SIP0 = 0x10,
	ACL_IP4SIP1,
	ACL_IP4DIP0,
	ACL_IP4DIP1,
	ACL_IP6SIP0WITHIPV4 = 0x20,
	ACL_IP6SIP1WITHIPV4,
	ACL_IP6DIP0WITHIPV4 = 0x28,
	ACL_IP6DIP1WITHIPV4,
	ACL_VIDRANGE = 0x30,
	ACL_IPRANGE,
	ACL_PORTRANGE,
	ACL_FIELD_SELECT00 = 0x40,
	ACL_FIELD_SELECT01,
	ACL_FIELD_SELECT02,
	ACL_FIELD_SELECT03,
	ACL_FIELD_SELECT04,
	ACL_FIELD_SELECT05,
	ACL_FIELD_SELECT06,
	ACL_FIELD_SELECT07,
	ACL_FIELD_SELECT08,
	ACL_FIELD_SELECT09,
	ACL_FIELD_SELECT10,
	ACL_FIELD_SELECT11,
	ACL_FIELD_SELECT12,
	ACL_FIELD_SELECT13,
	ACL_FIELD_SELECT14,
	ACL_FIELD_SELECT15,
	ACL_TCPSPORT = 0x80,
	ACL_TCPDPORT,
	ACL_TCPFLAG,
	ACL_UDPSPORT,
	ACL_UDPDPORT,
	ACL_ICMPCODETYPE,
	ACL_IGMPTYPE,
	ACL_SPORT,
	ACL_DPORT,
	ACL_IP4TOSPROTO,
	ACL_IP4FLAGOFF,
	ACL_TCNH,
	ACL_CPUTAG,
	ACL_L2PAYLOAD,
	ACL_IP6SIP0,
	ACL_IP6SIP1,
	ACL_IP6SIP2,
	ACL_IP6SIP3,
	ACL_IP6SIP4,
	ACL_IP6SIP5,
	ACL_IP6SIP6,
	ACL_IP6SIP7,
	ACL_IP6DIP0,
	ACL_IP6DIP1,
	ACL_IP6DIP2,
	ACL_IP6DIP3,
	ACL_IP6DIP4,
	ACL_IP6DIP5,
	ACL_IP6DIP6,
	ACL_IP6DIP7,
	ACL_TYPE_END
};

struct acl_rule_smi_st{
#ifdef _LITTLE_ENDIAN

	unsigned short type:3;
	unsigned short tag_exist:5;
	unsigned short active_portmsk:8;

	unsigned short field[RTL8367B_ACLRULEFIELDNO];
#else
	unsigned short active_portmsk:8;
	unsigned short tag_exist:5;
	unsigned short type:3;

	unsigned short field[RTL8367B_ACLRULEFIELDNO];
#endif
};

typedef struct ACLRULESMI{
	struct acl_rule_smi_st	care_bits;
	unsigned short		valid:1;
	struct acl_rule_smi_st	data_bits;
}rtl8367b_aclrulesmi;

struct acl_rule_st{
	unsigned short active_portmsk:8;
	unsigned short type:3;
	unsigned short tag_exist:5;
	unsigned short field[RTL8367B_ACLRULEFIELDNO];
};

typedef struct ACLRULE{
	struct acl_rule_st	data_bits;
	unsigned short		valid:1;
	struct acl_rule_st	care_bits;
}rtl8367b_aclrule;


typedef struct rtl8367b_acltemplate_s{
	unsigned char field[8];
}rtl8367b_acltemplate_t;


typedef struct acl_act_smi_s{
#ifdef _LITTLE_ENDIAN
	unsigned short cvidx_cact:6;
	unsigned short cact:2;
	unsigned short svidx_sact:6;
	unsigned short sact:2;

	unsigned short aclmeteridx:6;
	unsigned short fwdpmask:8;
	unsigned short fwdact:2;

	unsigned short pridx:6;
	unsigned short priact:2;
	unsigned short gpio_pin:4;
	unsigned short gpio_en:1;
	unsigned short aclint:1;
	unsigned short reserved:2;
#else
	unsigned short sact:2;
	unsigned short svidx_sact:6;
	unsigned short cact:2;
	unsigned short cvidx_cact:6;

	unsigned short fwdact:2;
	unsigned short fwdpmask:8;
	unsigned short aclmeteridx:6;

	unsigned short reserved:2;
	unsigned short aclint:1;
	unsigned short gpio_en:1;
	unsigned short gpio_pin:4;
	unsigned short priact:2;
	unsigned short pridx:6;

#endif
}rtl8367b_acl_act_smi_t;

typedef struct acl_act_s{
	unsigned short cvidx_cact:6;
	unsigned short cact:2;
	unsigned short svidx_sact:6;
	unsigned short sact:2;


	unsigned short aclmeteridx:6;
	unsigned short fwdpmask:8;
	unsigned short fwdact:2;

	unsigned short pridx:6;
	unsigned short priact:2;
	unsigned short gpio_pin:4;
	unsigned short gpio_en:1;
	unsigned short aclint:1;

}rtl8367b_acl_act_t;

typedef struct acl_rule_union_s
{
    rtl8367b_aclrule aclRule;
    rtl8367b_acl_act_t aclAct;
    unsigned int aclActCtrl;
    unsigned int aclNot;
}rtl8367b_acl_rule_union_t;


extern int rtl8367b_setAsicAcl(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicAcl(unsigned int port, unsigned int* pEnabled);
extern int rtl8367b_setAsicAclUnmatchedPermit(unsigned int port, unsigned int enabled);
extern int rtl8367b_getAsicAclUnmatchedPermit(unsigned int port, unsigned int* pEnabled);
extern int rtl8367b_setAsicAclRule(unsigned int index, rtl8367b_aclrule *pAclRule);
extern int rtl8367b_getAsicAclRule(unsigned int index, rtl8367b_aclrule *pAclRule);
extern int rtl8367b_setAsicAclNot(unsigned int index, unsigned int not);
extern int rtl8367b_getAsicAclNot(unsigned int index, unsigned int* pNot);
extern int rtl8367b_setAsicAclTemplate(unsigned int index, rtl8367b_acltemplate_t* pAclType);
extern int rtl8367b_getAsicAclTemplate(unsigned int index, rtl8367b_acltemplate_t *pAclType);
extern int rtl8367b_setAsicAclAct(unsigned int index, rtl8367b_acl_act_t* pAclAct);
extern int rtl8367b_getAsicAclAct(unsigned int index, rtl8367b_acl_act_t *pAclAct);
extern int rtl8367b_setAsicAclActCtrl(unsigned int index, unsigned int aclActCtrl);
extern int rtl8367b_getAsicAclActCtrl(unsigned int index, unsigned int *aclActCtrl);
extern int rtl8367b_setAsicAclPortRange(unsigned int index, unsigned int type, unsigned int upperPort, unsigned int lowerPort);
extern int rtl8367b_getAsicAclPortRange(unsigned int index, unsigned int* pType, unsigned int* pUpperPort, unsigned int* pLowerPort);
extern int rtl8367b_setAsicAclVidRange(unsigned int index, unsigned int type, unsigned int upperVid, unsigned int lowerVid);
extern int rtl8367b_getAsicAclVidRange(unsigned int index, unsigned int* pType, unsigned int* pUpperVid, unsigned int* pLowerVid);
extern int rtl8367b_setAsicAclIpRange(unsigned int index, unsigned int type, unsigned int upperIp, unsigned int lowerIp);
extern int rtl8367b_getAsicAclIpRange(unsigned int index, unsigned int* pType, unsigned int* pUpperIp, unsigned int* pLowerIp);
extern int rtl8367b_setAsicAclGpioPolarity(unsigned int polarity);
extern int rtl8367b_getAsicAclGpioPolarity(unsigned int* pPolarity);

#endif /*_RTL8367B_ASICDRV_ACL_H_*/


