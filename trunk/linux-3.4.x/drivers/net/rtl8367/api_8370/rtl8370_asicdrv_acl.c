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
 * $Revision: 14372 $
 * $Date: 2010-11-22 16:22:35 +0800 (星期一, 22 十一月 2010) $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_acl.h"

#if defined(CONFIG_RTL8370_ASICDRV_TEST)
rtl8370_acl_rule_smi_t Rtl8370sVirtualAclRuleTable[RTL8370_ACLRULENO];
rtl8370_acl_act_smi_t Rtl8370sVirtualAclActTable[RTL8370_ACLRULENO]; 
#endif

void _rtl8370_aclRuleStSmi2User( rtl8370_acl_rule_t *aclUser, rtl8370_acl_rule_smi_t *aclSmi);
void _rtl8370_aclRuleStUser2Smi( rtl8370_acl_rule_t *aclUser, rtl8370_acl_rule_smi_t *aclSmi);
void _rtl8370_aclActStSmi2User( rtl8370_acl_act_t *aclUser, rtl8370_acl_act_smi_t *aclSmi);
void _rtl8370_aclActStUser2Smi( rtl8370_acl_act_t *aclUser, rtl8370_acl_act_smi_t *aclSmi);

/*=======================================================================
 *  ACL APIs
 *========================================================================*/
 
/*
@func ret_t | rtl8370_setAsicAcl | Set port acl function enable/disable.
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can set per-port ACL enable function
*/
ret_t rtl8370_setAsicAcl(uint32 port, uint32 enabled)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_setAsicRegBit(RTL8370_ACL_ENABLE_REG,port, enabled);
}

/*
@func ret_t | rtl8370_getAsicAcl | Get port acl function enable/disable.
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get per-port ACL enable function
*/
ret_t rtl8370_getAsicAcl(uint32 port, uint32* enabled)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_ACL_ENABLE_REG, port, enabled);
}

/*
@func ret_t | rtl8370_setAsicAclUnmatchedPermit | Set port acl function unmatched permit action.
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | enabled | 1: Enable, 0: Disable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can set per-port ACL unmatched action. (permit or drop)
*/
ret_t rtl8370_setAsicAclUnmatchedPermit(uint32 port, uint32 enabled)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_setAsicRegBit(RTL8370_ACL_UNMATCH_PERMIT_REG,port, enabled);
}

/*
@func ret_t | rtl8370_getAsicAclUnmatchedPermit | Set port acl function unmatched permit action.
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | enabled | 1: Enable, 0: Disable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can get per-port ACL unmatched action. (permit or drop)
*/
ret_t rtl8370_getAsicAclUnmatchedPermit(uint32 port, uint32* enabled)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_ACL_UNMATCH_PERMIT_REG, port, &(*enabled));
}

/*
    Exchange structure type define with MMI and SMI
*/
void _rtl8370_aclRuleStSmi2User( rtl8370_acl_rule_t *aclUser, rtl8370_acl_rule_smi_t *aclSmi)
{
    uint16 *care_ptr, *data_ptr;
    uint16 care_tmp, data_tmp;
    uint32 i;    
    
    aclUser->data_bits.active_portmsk = aclSmi->data_bits.active_portmsk;
    aclUser->data_bits.type = aclSmi->data_bits.type;
    aclUser->data_bits.tag_exist = aclSmi->data_bits.tag_exist;

    care_ptr = (uint16*)&aclSmi->care_bits;
    data_ptr = (uint16*)&aclSmi->data_bits;
    
    for ( i = 0; i < 9;i++)    
    {
        care_tmp = *(care_ptr + i) ^ (*(data_ptr + i));
        data_tmp = *(data_ptr + i);

        *(care_ptr + i) = care_tmp;
        *(data_ptr + i) = data_tmp;
    }


    for( i = 0; i < RTL8370_ACLTYPEFIELDMAX; i++ )
    {
        aclUser->data_bits.field[i] = aclSmi->data_bits.field[i];
    }

    aclUser->valid = aclSmi->valid;

    aclUser->care_bits.active_portmsk = aclSmi->care_bits.active_portmsk;
    aclUser->care_bits.type = aclSmi->care_bits.type;
    aclUser->care_bits.tag_exist = aclSmi->care_bits.tag_exist;


    for( i = 0; i < RTL8370_ACLTYPEFIELDMAX; i++ )
    {
        aclUser->care_bits.field[i] = aclSmi->care_bits.field[i];
    }
/*
    care_ptr = (uint16*)&aclUser->care_bits;
    data_ptr = (uint16*)&aclUser->data_bits;
    for ( i = 0; i < 9;i++)    
    {
        care_tmp = *(care_ptr + i) ^ (*(data_ptr + i));
        data_tmp = *(data_ptr + i);

        *(care_ptr + i) = care_tmp;
        *(data_ptr + i) = data_tmp;
    }    
*/    
}

/*
    Exchange structure type define with MMI and SMI
*/
void _rtl8370_aclRuleStUser2Smi( rtl8370_acl_rule_t *aclUser, rtl8370_acl_rule_smi_t *aclSmi)
{
    uint16 *care_ptr, *data_ptr;
    uint16 care_tmp, data_tmp;
    uint32 i;  
    
    aclSmi->data_bits.active_portmsk = aclUser->data_bits.active_portmsk;
    aclSmi->data_bits.type = aclUser->data_bits.type;
    aclSmi->data_bits.tag_exist = aclUser->data_bits.tag_exist;

    for( i = 0; i < RTL8370_ACLTYPEFIELDMAX; i++ )
    {
        aclSmi->data_bits.field[i] = aclUser->data_bits.field[i];
    }

   
    aclSmi->valid = aclUser->valid;

    aclSmi->care_bits.active_portmsk = aclUser->care_bits.active_portmsk;
    aclSmi->care_bits.type = aclUser->care_bits.type;
    aclSmi->care_bits.tag_exist = aclUser->care_bits.tag_exist;


    for( i = 0; i < RTL8370_ACLTYPEFIELDMAX; i++ )
    {
        aclSmi->care_bits.field[i] = aclUser->care_bits.field[i];
    }

    care_ptr = (uint16*)&aclSmi->care_bits;
    data_ptr = (uint16*)&aclSmi->data_bits;

    for ( i = 0; i < 9;i++)
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
void _rtl8370_aclActStSmi2User( rtl8370_acl_act_t *aclUser, rtl8370_acl_act_smi_t *aclSmi)
{
    aclUser->arpmsk = aclSmi->arpmsk_1 | (aclSmi->arpmsk_2 << 1);
    aclUser->mrat = aclSmi->mrat;
    aclUser->aclmeteridx = aclSmi->aclmeteridx;
    aclUser->aclsvidx = aclSmi->aclsvidx;
    aclUser->aclpri = aclSmi->pri_1 | (aclSmi->pri_2 << 1);
    aclUser->aclcvid = aclSmi->aclcvid;
    aclUser->ct = aclSmi->ct;
}

/*
    Exchange structure type define with MMI and SMI
*/
void _rtl8370_aclActStUser2Smi( rtl8370_acl_act_t *aclUser, rtl8370_acl_act_smi_t *aclSmi)
{
    aclSmi->arpmsk_1 = aclUser->arpmsk & (0x1);
    aclSmi->arpmsk_2 = (aclUser->arpmsk & (0xFFFE)) >> 1;
    aclSmi->mrat = aclUser->mrat;
    aclSmi->aclmeteridx = aclUser->aclmeteridx;
    aclSmi->aclsvidx = aclUser->aclsvidx;
    aclSmi->pri_1 = aclUser->aclpri& 0x1;
    aclSmi->pri_2 = (aclUser->aclpri & (0x6)) >> 1;
    aclSmi->aclcvid = aclUser->aclcvid;
    aclSmi->ct = aclUser->ct;
}

/*
@func ret_t | rtl8370_setAsicAclRule | Set acl rule content.
@parm uint32 | index | ACL rule index (0-63) of 64 ACL rules.
@parm rtl8370_acltrule | aclRule | ACL rule stucture for setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_FILTER_ENTRYIDX | Invalid ACL rule index (0-63).
@comm
    The API can set ACL rule
    
    System supported 64 shared 289-bit ACL ingress rule. Index was available at range 0-63 only. If software want to
    modify ACL rule, the ACL function should be disable at first or unspecify acl action will be executed. 

    One ACL rule structure has three parts setting:
    Bit 0-143        Data Bits of this Rule
    Bit    144        Valid Bit
    Bit 145-288    Care Bits of this Rule

    There are four kinds of field in Data Bits and Care Bits: Active Portmask, Type, Tag Exist, and 7 fields

    Reading/writing to a ACL Rule is achieved by writing/reading registers at 0x0500 ~ 0x050A
*/
ret_t rtl8370_setAsicAclRule(uint32 index, rtl8370_acl_rule_t *aclRule)
{
    rtl8370_acl_rule_smi_t aclRuleSmi;
    uint16* tableAddr;
    uint32 regAddr;
    uint32    regData;
    uint32 i;
    ret_t retVal;
    
    if(index > RTL8370_ACLRULEMAX)
        return RT_ERR_FILTER_ENTRYIDX;
    
    memset(&aclRuleSmi,0x00,sizeof(rtl8370_acl_rule_smi_t));
     _rtl8370_aclRuleStUser2Smi(aclRule, &aclRuleSmi);

    /* Write ACS_ADR register for data bits */
    regAddr = RTL8370_TABLE_ACCESS_ADDR_REG;
    regData = RTL8370_ACLRULETBADDR(DATABITS, index);
    retVal = rtl8370_setAsicReg(regAddr,regData);
    if(retVal !=RT_ERR_OK)
        return retVal;

    retVal = rtl8370_setAsicRegBits(RTL8370_TABLE_ACCESS_DATA_REG(RTL8370_ACLRULETBLEN), 0x1, 0);
    if(retVal !=RT_ERR_OK)
        return retVal;

    regAddr = RTL8370_TABLE_ACCESS_CTRL_REG;
    regData = RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_WRITE, TB_TARGET_ACLRULE);
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;


    /* Write ACS_ADR register */
    regAddr = RTL8370_TABLE_ACCESS_ADDR_REG;
    regData = RTL8370_ACLRULETBADDR(CAREBITS, index);
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;
    
    
    retVal = rtl8370_setAsicReg(regAddr,regData);
    if(retVal !=RT_ERR_OK)
        return retVal;

    /* Write Care Bits to ACS_DATA registers */
     tableAddr = (uint16*)&aclRuleSmi.care_bits;
     regAddr = RTL8370_TABLE_ACCESS_DATA_BASE;

    for(i=0;i<RTL8370_ACLRULETBLEN;i++)
    {
        regData = *tableAddr;
        retVal = rtl8370_setAsicReg(regAddr,regData);
        if(retVal !=RT_ERR_OK)
            return retVal;

        regAddr++;
        tableAddr++;
    }
    
    /* Write ACS_CMD register */
    regAddr = RTL8370_TABLE_ACCESS_CTRL_REG;
    regData = RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_WRITE, TB_TARGET_ACLRULE);
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;

    /* Write ACS_ADR register for data bits */
    regAddr = RTL8370_TABLE_ACCESS_ADDR_REG;
    regData = RTL8370_ACLRULETBADDR(DATABITS, index);
    retVal = rtl8370_setAsicReg(regAddr,regData);
    if(retVal !=RT_ERR_OK)
        return retVal;

    /* Write Data Bits to ACS_DATA registers */
     tableAddr = (uint16*)&aclRuleSmi.data_bits;
     regAddr = RTL8370_TABLE_ACCESS_DATA_BASE;

    for(i=0;i<RTL8370_ACLRULETBLEN;i++)
    {
        regData = *tableAddr;
        retVal = rtl8370_setAsicReg(regAddr,regData);
        if(retVal !=RT_ERR_OK)
            return retVal;

        regAddr++;
        tableAddr++;
    }

    /* Write Valid Bit */
    retVal = rtl8370_setAsicRegBits(RTL8370_TABLE_ACCESS_DATA_REG(RTL8370_ACLRULETBLEN), 0x1, aclRuleSmi.valid);
    if(retVal !=RT_ERR_OK)
        return retVal;

    /* Write ACS_CMD register for care bits*/
    regAddr = RTL8370_TABLE_ACCESS_CTRL_REG;
    regData = RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_WRITE, TB_TARGET_ACLRULE);
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;

#ifdef CONFIG_RTL8370_ASICDRV_TEST
    Rtl8370sVirtualAclRuleTable[index] = aclRuleSmi;
#endif

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicAclRule | Get acl rule content.
@parm uint32 | index | ACL rule index (0-63) of 64 ACL rules.
@parm rtl8370_acltrule* | aclRule | ACL rule stucture for setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_FILTER_ENTRYIDX | Invalid ACL rule index (0-63).
@comm
	none
*/

ret_t rtl8370_getAsicAclRule( uint32 index, rtl8370_acl_rule_t *aclRule)
{
    rtl8370_acl_rule_smi_t aclRuleSmi;    
    uint32 regAddr, regData;
    ret_t retVal;
    uint16* tableAddr;
    uint32 i;

    if(index > RTL8370_ACLRULEMAX)
        return RT_ERR_FILTER_ENTRYIDX;

    memset(&aclRuleSmi,0x00,sizeof(rtl8370_acl_rule_smi_t));

    /* Write ACS_ADR register for data bits */
    regAddr = RTL8370_TABLE_ACCESS_ADDR_REG;
    regData = RTL8370_ACLRULETBADDR(DATABITS, index);
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;
    

    /* Write ACS_CMD register */
    regAddr = RTL8370_TABLE_ACCESS_CTRL_REG;
    regData = RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_READ, TB_TARGET_ACLRULE);
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;
    

    /* Read Data Bits */
    regAddr = RTL8370_TABLE_ACCESS_DATA_BASE;
    tableAddr = (uint16*)&aclRuleSmi.data_bits;
    for(i=0;i<RTL8370_ACLRULETBLEN;i++)
    {
        retVal = rtl8370_getAsicReg(regAddr, &regData);
        if(retVal !=RT_ERR_OK)
            return retVal;

        *tableAddr = regData;
        
        regAddr ++;
        tableAddr ++;
    }
    
    /* Read Valid Bit */
    retVal = rtl8370_getAsicReg(RTL8370_TABLE_ACCESS_DATA_REG(RTL8370_ACLRULETBLEN), &regData);
    if(retVal !=RT_ERR_OK)
        return retVal;
    
    aclRuleSmi.valid = regData & 0x1;

    /* Write ACS_ADR register for carebits*/
    regAddr = RTL8370_TABLE_ACCESS_ADDR_REG;
    regData = RTL8370_ACLRULETBADDR(CAREBITS, index);
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;

    /* Write ACS_CMD register */
    regAddr = RTL8370_TABLE_ACCESS_CTRL_REG;
    regData = RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_READ, TB_TARGET_ACLRULE);
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;

    /* Read Care Bits */
    regAddr = RTL8370_TABLE_ACCESS_DATA_BASE;
    tableAddr = (uint16*)&aclRuleSmi.care_bits;    
    for(i=0;i<RTL8370_ACLRULETBLEN;i++)
    {
        retVal = rtl8370_getAsicReg(regAddr, &regData);
        if(retVal !=RT_ERR_OK)
            return retVal;

        *tableAddr = regData;
        
        regAddr ++;
        tableAddr ++;
    }
    

#ifdef CONFIG_RTL8370_ASICDRV_TEST
    aclRuleSmi = Rtl8370sVirtualAclRuleTable[index];
#endif

     _rtl8370_aclRuleStSmi2User(aclRule, &aclRuleSmi);

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicAcl | Set rule comparison result inversion / no inversion.
@parm uint32 | index | ACL rule index (0-63) of 64 ACL rules.
@parm uint32 | not | 1: inverse, 0: don't inverse.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_FILTER_ENTRYIDX | Invalid ACL rule index (0-63).
@comm
    The API can set not bit of ACL rules
    If the not bit of a rule is set, the comparison result will be inversed.
*/
ret_t rtl8370_setAsicAclNot(uint32 index, uint32 not)
{
    if(index > RTL8370_ACLRULEMAX)
        return RT_ERR_FILTER_ENTRYIDX;
    
    return rtl8370_setAsicRegBit(RTL8370_ACL_ACTION_CTRL_REG(index), RTL8370_ACL_OP_NOT_OFFSET(index), not);
}

/*
@func ret_t | rtl8370_getAsicAcl | Get rule comparison result inversion / no inversion.
@parm uint32 | index | ACL rule index (0-63) of 64 ACL rules.
@parm uint32 | *not | 1: inverse, 0: don't inverse.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_FILTER_ENTRYIDX | Invalid ACL rule index (0-63).
@comm
    The API can get not bit of ACL rules
    If the not bit of a rule is set, the comparison result will be inversed.
*/
ret_t rtl8370_getAsicAclNot(uint32 index, uint32* not)
{
    if(index > RTL8370_ACLRULEMAX)
        return RT_ERR_FILTER_ENTRYIDX;

    return rtl8370_getAsicRegBit(RTL8370_ACL_ACTION_CTRL_REG(index), RTL8370_ACL_OP_NOT_OFFSET(index), not);
}

/*
@func ret_t | rtl8370_setAsicAclType | Set fields of a ACL type
@parm uint32 | index | ACL type index (0-4) of 5 ACL types.
@parm rtl8370_acl_template_t | aclType | ACL type stucture for setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can set type field of the 5 ACL rule templates.
    Each type has 7 fields. One field means what data in one field of a ACL rule means
    7 fields of ACL rule 0~63 is descripted by one type in ACL group A (0-4)
*/
ret_t rtl8370_setAsicAclType(uint32 index, rtl8370_acl_template_t aclType)
{    
    uint32 retVal;
    uint32 i;
    uint32 regAddr, regBits, regData;

    if(index > RTL8370_ACLTYPEMAX)
        return RT_ERR_INPUT;

    for(i=0;i<RTL8370_ACLTYPEFIELDMAX;i++)
    {
        regAddr = RTL8370_ACL_RULE_TEMPLATE_CTRL_REG(index, i) ;
        regBits = RTL8370_ACL_TEMPLATE_FIELD_MASK(i);
        regData = aclType.field[i];

        retVal = rtl8370_setAsicRegBits(regAddr,regBits,regData);        

        if (retVal != RT_ERR_OK)
            return retVal;
    }

    return retVal;
}


/*
@func ret_t | rtl8370_getAsicAclType | Get fields of a ACL type
@parm uint32 | index | ACL type index (0-4) of 5 ACL types.
@parm rtl8370_acl_template_t* | aclType | ACL type stucture for setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    none
*/

ret_t rtl8370_getAsicAclType(uint32 index, rtl8370_acl_template_t *aclType)
{
    uint32 retVal;
    uint32 i;
    uint32 regData, regAddr, regBits;
    
    if(index > RTL8370_ACLTYPEMAX)
        return RT_ERR_INPUT;

    for(i=0;i<RTL8370_ACLTYPEFIELDMAX;i++){
        regAddr = RTL8370_ACL_RULE_TEMPLATE_CTRL_REG(index, i) ;
        regBits = RTL8370_ACL_TEMPLATE_FIELD_MASK(i);

        retVal = rtl8370_getAsicRegBits(regAddr, regBits, &regData);            

        if (retVal != RT_ERR_OK)
                return retVal;
        
        aclType->field[i] = regData;
    }
    
    return retVal;
}

/*
@func ret_t | rtl8370_setAsicAclAct | Set ACL rule matched Action
@parm uint32 | index | ACL rule index (0-63) of 64 ACL rules.
@parm rtl8370_acltact | aclAct | ACL action stucture for setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_FILTER_ENTRYIDX | Invalid ACL rule index (0-63).
@comm
    The API can set ACL action of ACL rules.

    System supported 64 shared 48-bit ACL matched Action. Index was available at range 0-63 only. If software want to
    modify ACL rule, the ACL function should be disable at first or unspecify acl action will be executed. 

     Length of a ACL action structure is 48 bits
    Reading/writing to a ACL Rule is achieved by writing/reading registers at 0x0500 ~ 0x0504
*/
ret_t rtl8370_setAsicAclAct(uint32 index, rtl8370_acl_act_t aclAct)
{
    rtl8370_acl_act_smi_t aclActSmi;
    ret_t retVal;
    uint32 regAddr, regData;
    uint16* tableAddr;
    uint32 i;
    
    if(index > RTL8370_ACLRULEMAX)
        return RT_ERR_FILTER_ENTRYIDX;
    
    memset(&aclActSmi,0x00,sizeof(rtl8370_acl_act_smi_t));
     _rtl8370_aclActStUser2Smi(&aclAct, &aclActSmi);


    /* Write ACS_ADR register for data bits */
    regAddr = RTL8370_TABLE_ACCESS_ADDR_REG;
    regData = index;
    retVal = rtl8370_setAsicReg(regAddr,regData);
    if(retVal !=RT_ERR_OK)
        return retVal;

    /* Write Data Bits to ACS_DATA registers */
     tableAddr = (uint16*)&aclActSmi;
     regAddr = RTL8370_TABLE_ACCESS_DATA_BASE;

    for(i=0;i<RTL8370_ACLACTTBLEN;i++)
    {
        regData = *tableAddr;
        retVal = rtl8370_setAsicReg(regAddr,regData);
        if(retVal !=RT_ERR_OK)
            return retVal;

        regAddr++;
        tableAddr++;
    }
    
    /* Write ACS_CMD register for care bits*/
    regAddr = RTL8370_TABLE_ACCESS_CTRL_REG;
    regData = RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_WRITE, TB_TARGET_ACLACT);
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;

#ifdef CONFIG_RTL8370_ASICDRV_TEST
    Rtl8370sVirtualAclActTable[index] = aclActSmi;
#endif

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicAclAct | Get ACL rule matched Action
@parm uint32 | index | ACL rule index (0-63) of 64 ACL rules.
@parm rtl8370_acltact* | aclAct | ACL action stucture for setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_FILTER_ENTRYIDX | Invalid ACL rule index (0-63).
@comm
	none
*/

ret_t rtl8370_getAsicAclAct( uint32 index, rtl8370_acl_act_t *aclAct)
{
    rtl8370_acl_act_smi_t aclActSmi;
    ret_t retVal;
    uint32 regAddr, regData;
    uint16* tableAddr;
    uint32 i;
    
    if(index > RTL8370_ACLRULEMAX)
        return RT_ERR_FILTER_ENTRYIDX;
    
    memset(&aclActSmi,0x00,sizeof(rtl8370_acl_act_smi_t));


    /* Write ACS_ADR register for data bits */
    regAddr = RTL8370_TABLE_ACCESS_ADDR_REG;
    regData = index;
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;
    

    /* Write ACS_CMD register */
    regAddr = RTL8370_TABLE_ACCESS_CTRL_REG;
    regData = RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_READ, TB_TARGET_ACLACT);
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if(retVal !=RT_ERR_OK)
        return retVal;
    

    /* Read Data Bits */
    regAddr = RTL8370_TABLE_ACCESS_DATA_BASE;
    tableAddr = (uint16*)&aclActSmi;
    for(i=0;i<RTL8370_ACLACTTBLEN;i++)
    {
        retVal = rtl8370_getAsicReg(regAddr, &regData);
        if(retVal !=RT_ERR_OK)
            return retVal;

        *tableAddr = regData;
        
        regAddr ++;
        tableAddr ++;
    }

     _rtl8370_aclActStSmi2User(aclAct, &aclActSmi);
     
#ifdef CONFIG_RTL8370_ASICDRV_TEST
    Rtl8370sVirtualAclActTable[index] = aclActSmi;
#endif

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicAclActCtrl | Set ACL rule matched Action Control Bits
@parm uint32 | index | ACL rule index (0-63) of 64 ACL rules.
@parm uint32 | aclActCtrl | 5 ACL Control Bits.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_FILTER_ENTRYIDX | Invalid ACL rule index (0-63).
@comm
    The API can set ACL action control bits of ACL rules.
    ACL Action Control Bits Indicate which actions will be take when a rule matches
*/
ret_t rtl8370_setAsicAclActCtrl(uint32 index, uint32 aclActCtrl)
{
    uint32 regAddr, regBits;
    
    if(index > RTL8370_ACLRULEMAX)
        return RT_ERR_FILTER_ENTRYIDX;
    
    regAddr = RTL8370_ACL_ACTION_CTRL_REG(index);
    regBits = RTL8370_ACL_OP_ACTION_MASK(index);

    return rtl8370_setAsicRegBits(regAddr, regBits, aclActCtrl);
}

/*
@func ret_t | rtl8370_getAsicAclActCtrl | Get ACL rule matched Action Control Bits
@parm uint32 | index | ACL rule index (0-63) of 64 ACL rules.
@parm uint32* | aclActCtrl | 5 ACL Control Bits.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_FILTER_ENTRYIDX | Invalid ACL rule index (0-63).
@comm
    The API can get ACL action control bits of ACL rules.
    ACL Action Control Bits Indicate which actions will be take when a rule matches
*/
ret_t rtl8370_getAsicAclActCtrl( uint32 index, uint32 *aclActCtrl)
{
    uint32 regAddr, regBits;
    
    if(index > RTL8370_ACLRULEMAX)
        return RT_ERR_FILTER_ENTRYIDX;

    regAddr = RTL8370_ACL_ACTION_CTRL_REG(index);
    regBits = RTL8370_ACL_OP_ACTION_MASK(index);
	
    return rtl8370_getAsicRegBits(regAddr, regBits, aclActCtrl);
}

