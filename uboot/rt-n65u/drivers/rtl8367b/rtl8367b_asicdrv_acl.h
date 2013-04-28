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

	rtk_uint16 type:3;
	rtk_uint16 tag_exist:5;
	rtk_uint16 active_portmsk:8;

	rtk_uint16 field[RTL8367B_ACLRULEFIELDNO];
#else
	rtk_uint16 active_portmsk:8;
	rtk_uint16 tag_exist:5;
	rtk_uint16 type:3;

	rtk_uint16 field[RTL8367B_ACLRULEFIELDNO];
#endif
};

typedef struct ACLRULESMI{
	struct acl_rule_smi_st	care_bits;
	rtk_uint16		valid:1;
	struct acl_rule_smi_st	data_bits;
}rtl8367b_aclrulesmi;

struct acl_rule_st{
	rtk_uint16 active_portmsk:8;
	rtk_uint16 type:3;
	rtk_uint16 tag_exist:5;
	rtk_uint16 field[RTL8367B_ACLRULEFIELDNO];
};

typedef struct ACLRULE{
	struct acl_rule_st	data_bits;
	rtk_uint16		valid:1;
	struct acl_rule_st	care_bits;
}rtl8367b_aclrule;


typedef struct rtl8367b_acltemplate_s{
	rtk_uint8 field[8];
}rtl8367b_acltemplate_t;


typedef struct acl_act_smi_s{
#ifdef _LITTLE_ENDIAN
	rtk_uint16 cvidx_cact:6;
	rtk_uint16 cact:2;
	rtk_uint16 svidx_sact:6;
	rtk_uint16 sact:2;

	rtk_uint16 aclmeteridx:6;
	rtk_uint16 fwdpmask:8;
	rtk_uint16 fwdact:2;

	rtk_uint16 pridx:6;
	rtk_uint16 priact:2;
	rtk_uint16 gpio_pin:4;
	rtk_uint16 gpio_en:1;
	rtk_uint16 aclint:1;
	rtk_uint16 reserved:2;
#else
	rtk_uint16 sact:2;
	rtk_uint16 svidx_sact:6;
	rtk_uint16 cact:2;
	rtk_uint16 cvidx_cact:6;

	rtk_uint16 fwdact:2;
	rtk_uint16 fwdpmask:8;
	rtk_uint16 aclmeteridx:6;

	rtk_uint16 reserved:2;
	rtk_uint16 aclint:1;
	rtk_uint16 gpio_en:1;
	rtk_uint16 gpio_pin:4;
	rtk_uint16 priact:2;
	rtk_uint16 pridx:6;

#endif
}rtl8367b_acl_act_smi_t;

typedef struct acl_act_s{
	rtk_uint16 cvidx_cact:6;
	rtk_uint16 cact:2;
	rtk_uint16 svidx_sact:6;
	rtk_uint16 sact:2;


	rtk_uint16 aclmeteridx:6;
	rtk_uint16 fwdpmask:8;
	rtk_uint16 fwdact:2;

	rtk_uint16 pridx:6;
	rtk_uint16 priact:2;
	rtk_uint16 gpio_pin:4;
	rtk_uint16 gpio_en:1;
	rtk_uint16 aclint:1;

}rtl8367b_acl_act_t;

typedef struct acl_rule_union_s
{
    rtl8367b_aclrule aclRule;
    rtl8367b_acl_act_t aclAct;
    rtk_uint32 aclActCtrl;
    rtk_uint32 aclNot;
}rtl8367b_acl_rule_union_t;


extern ret_t rtl8367b_setAsicAcl(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicAcl(rtk_uint32 port, rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicAclUnmatchedPermit(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicAclUnmatchedPermit(rtk_uint32 port, rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicAclRule(rtk_uint32 index, rtl8367b_aclrule *pAclRule);
extern ret_t rtl8367b_getAsicAclRule(rtk_uint32 index, rtl8367b_aclrule *pAclRule);
extern ret_t rtl8367b_setAsicAclNot(rtk_uint32 index, rtk_uint32 not);
extern ret_t rtl8367b_getAsicAclNot(rtk_uint32 index, rtk_uint32* pNot);
extern ret_t rtl8367b_setAsicAclTemplate(rtk_uint32 index, rtl8367b_acltemplate_t* pAclType);
extern ret_t rtl8367b_getAsicAclTemplate(rtk_uint32 index, rtl8367b_acltemplate_t *pAclType);
extern ret_t rtl8367b_setAsicAclAct(rtk_uint32 index, rtl8367b_acl_act_t* pAclAct);
extern ret_t rtl8367b_getAsicAclAct(rtk_uint32 index, rtl8367b_acl_act_t *pAclAct);
extern ret_t rtl8367b_setAsicAclActCtrl(rtk_uint32 index, rtk_uint32 aclActCtrl);
extern ret_t rtl8367b_getAsicAclActCtrl(rtk_uint32 index, rtk_uint32 *aclActCtrl);
extern ret_t rtl8367b_setAsicAclPortRange(rtk_uint32 index, rtk_uint32 type, rtk_uint32 upperPort, rtk_uint32 lowerPort);
extern ret_t rtl8367b_getAsicAclPortRange(rtk_uint32 index, rtk_uint32* pType, rtk_uint32* pUpperPort, rtk_uint32* pLowerPort);
extern ret_t rtl8367b_setAsicAclVidRange(rtk_uint32 index, rtk_uint32 type, rtk_uint32 upperVid, rtk_uint32 lowerVid);
extern ret_t rtl8367b_getAsicAclVidRange(rtk_uint32 index, rtk_uint32* pType, rtk_uint32* pUpperVid, rtk_uint32* pLowerVid);
extern ret_t rtl8367b_setAsicAclIpRange(rtk_uint32 index, rtk_uint32 type, ipaddr_t upperIp, ipaddr_t lowerIp);
extern ret_t rtl8367b_getAsicAclIpRange(rtk_uint32 index, rtk_uint32* pType, ipaddr_t* pUpperIp, ipaddr_t* pLowerIp);
extern ret_t rtl8367b_setAsicAclGpioPolarity(rtk_uint32 polarity);
extern ret_t rtl8367b_getAsicAclGpioPolarity(rtk_uint32* pPolarity);

#endif /*_RTL8367B_ASICDRV_ACL_H_*/


