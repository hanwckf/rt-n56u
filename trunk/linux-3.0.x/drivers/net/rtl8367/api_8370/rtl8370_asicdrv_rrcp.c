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
 * $Revision: 1.1.1.1 $
 * $Date: 2010/12/02 04:34:26 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_rrcp.h"

/* [FIXME] these macro should be removed in formal chip */
#define RTL8370_OLD_RRCP_CTRL    0x122b
#define RTL8370_OLD_RRCP_TPMSK   0x122e
#define RTL8370_OLD_RRCP_AUTHKEY 0x122d 
#define RTL8370_OLD_RRCP_PRIKEY  0x122c

/*
@func ret_t | rtl8370_setAsicRrcp | Set RRCP function enable/disable.
@parm uint32 | vOneEnable | 1: enabled, 0: disabled.
@parm uint32 | vTwoEnable | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    Enable / Disable RRCPv1 and RRCPv2 function
    Note that RRCPv1 and RRCPv2 shall not be enabled together
    Only one is needed.
*/
ret_t rtl8370_setAsicRrcp(uint32 vOneEnable, uint32 vTwoEnable)
{
    ret_t  retVal;    
	uint32 tmp;

    if((vOneEnable > 1) || (vTwoEnable > 1))
        return RT_ERR_INPUT;

    retVal = rtl8370_getAsicReg(RTL8370_RRCP_CTRL0_REG, &tmp);
    if(retVal != RT_ERR_OK)
        return RT_ERR_FAILED;

    tmp &= ~(1 << RTL8370_RRCP_V1_EN_OFFSET);
	tmp &= ~(1 << RTL8370_RRCP_V2_EN_OFFSET);

	tmp |= ((vOneEnable & 1) << RTL8370_RRCP_V1_EN_OFFSET);
	tmp |= ((vTwoEnable & 1) << RTL8370_RRCP_V2_EN_OFFSET);

    retVal = rtl8370_setAsicReg(RTL8370_RRCP_CTRL0_REG, tmp);
    if(retVal != RT_ERR_OK)
        return RT_ERR_FAILED;
    
    retVal = rtl8370_setAsicReg(RTL8370_OLD_RRCP_CTRL, tmp);
    if(retVal != RT_ERR_OK)
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicRrcp | Get RRCP function enable/disable.
@parm uint32* | vOneEnable | 1: enabled, 0: disabled.
@parm uint32* | vTwoEnable | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    Enable / Disable RRCPv1 and RRCPv2 function
    Note that RRCPv1 and RRCPv2 shall not be enabled together
    Only one is needed.
*/
ret_t rtl8370_getAsicRrcp(uint32 *vOneEnable, uint32 *vTwoEnable)
{
    ret_t retVal;    
    
    retVal = rtl8370_getAsicRegBit(RTL8370_RRCP_CTRL0_REG, RTL8370_RRCP_V1_EN_OFFSET, vOneEnable);
    if(retVal != RT_ERR_OK)
        return RT_ERR_FAILED;
    
    return rtl8370_getAsicRegBit(RTL8370_RRCP_CTRL0_REG, RTL8370_RRCP_V2_EN_OFFSET, vTwoEnable);
}

/*
@func ret_t | rtl8370_setAsicRrcpTrustPortmask | Set turst portmask of RRCP.
@parm uint32 | pmsk | (0~0xFFFF).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the trust port mask of RRCP packet.
    If SPA of a RRCP is not in the set of trust portmask, the RRCP
    frame will be dropped.
*/
ret_t rtl8370_setAsicRrcpTrustPortmask(uint32 pmsk)
{
    ret_t retVal;

    if(pmsk > RTL8370_PORTMASK)
        return RT_ERR_PORT_MASK;

    retVal = rtl8370_setAsicReg(RTL8370_RRCP_TRUST_PORTMASK_REG, pmsk);
	if(retVal != RT_ERR_OK )
		return retVal;

    retVal = rtl8370_setAsicReg(RTL8370_OLD_RRCP_TPMSK, pmsk);
	if(retVal != RT_ERR_OK )
		return retVal;

	return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicRrcpTrustPortmask | Get turst portmask of RRCP.
@parm uint32* | authKey | (0~0xFFFF).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the trust port mask of RRCP packet.
    If SPA of a RRCP is not in the set of trust portmask, the RRCP
    frame will be dropped.
*/
ret_t rtl8370_getAsicRrcpTrustPortmask(uint32 *pmsk)
{
    return rtl8370_getAsicReg(RTL8370_RRCP_TRUST_PORTMASK_REG, pmsk);    
}

/*
@func ret_t | rtl8370_setAsicRrcpAuthenticationKey | Set authentication key of RRCP.
@parm uint32 | authKey | (0~0xFFFF).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the authentication register.
    Authentication Key Register is used to compare with Authentication key field of
    RRCPv1 Frame, and Public Key field of RRCPv2 frame.
*/
ret_t rtl8370_setAsicRrcpAuthenticationKey(uint32 authKey)
{
    ret_t retVal;

    if(authKey > RTL8370_REGDATAMAX)
        return RT_ERR_PHY_DATAMASK;

    retVal = rtl8370_setAsicReg(RTL8370_RRCP_AUTH_KEY_REG, authKey);    
	if(retVal != RT_ERR_OK )
		return retVal;

    retVal = rtl8370_setAsicReg(RTL8370_OLD_RRCP_AUTHKEY, authKey);    
	if(retVal != RT_ERR_OK )
		return retVal;

	return RT_ERR_OK;	
}

/*
@func ret_t | rtl8370_getAsicRrcpAuthenticationKey | Get authentication key of RRCP.
@parm uint32* | pmsk | (0~0xFFFF).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the authentication register.
    Authentication Key Register is used to compare with Authentication key field of
    RRCPv1 Frame, and Public Key field of RRCPv2 frame.
*/
ret_t rtl8370_getAsicRrcpAuthenticationKey(uint32 *authKey)
{
    return rtl8370_getAsicReg(RTL8370_RRCP_AUTH_KEY_REG, authKey);    
}

/*
@func ret_t | rtl8370_setAsicPrivateKey | Set private key of RRCP.
@parm uint32 | privateKey | (0~0xFFFF).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the private key of RRCPv2
    Authentication Key Register is used to compare with private Key field of RRCPv2 frame.
*/
ret_t rtl8370_setAsicRrcpPrivateKey(uint32 privateKey)
{
    ret_t retVal;

    if(privateKey> RTL8370_REGDATAMAX)
        return RT_ERR_PHY_DATAMASK;

    retVal = rtl8370_setAsicReg(RTL8370_RRCP_PRIVATE_KEY_REG, privateKey);
	if(retVal != RT_ERR_OK )
		return retVal;

    retVal = rtl8370_setAsicReg(RTL8370_OLD_RRCP_PRIKEY, privateKey);
	if(retVal != RT_ERR_OK )
		return retVal;

	return RT_ERR_OK;	
}

/*
@func ret_t | rtl8370_getAsicPrivateKey | Get private key of RRCP.
@parm uint32* | privateKey | (0~0xFFFF).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the private key of RRCPv2
    Authentication Key Register is used to compare with private Key field of RRCPv2 frame.
*/
ret_t rtl8370_getAsicRrcpPrivateKey(uint32 *privateKey)
{
    return rtl8370_getAsicReg(RTL8370_RRCP_PRIVATE_KEY_REG, privateKey);    
}

/*
@func ret_t | rtl8370_setAsicRrcpV2Trap8051 | Set trap to 8051 setting enable/disable.
@parm uint32 | trap | 0: disable/1: enable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set trap RRCPv2 frame to 8051 enable/disable
    Trap to RRCP frame to 8051 setting. If this bit is set, ASIC will
    not handled the received RRCP frame.
*/
ret_t rtl8370_setAsicRrcpV2Trap8051(uint32 trap)
{
    ret_t retVal;

    if(trap > 1)
        return RT_ERR_INPUT;

    retVal = rtl8370_setAsicRegBit(RTL8370_RRCP_CTRL0_REG, RTL8370_RRCP_TRAP_8051_OFFSET, trap);    
    if(retVal != RT_ERR_OK)
		return retVal;

    retVal = rtl8370_setAsicRegBit(RTL8370_OLD_RRCP_CTRL, RTL8370_RRCP_TRAP_8051_OFFSET, trap);

	return retVal;
}

/*
@func ret_t | rtl8370_getAsicRrcpV2Trap8051 | Set trap to 8051 setting enable/disable.
@parm uint32* | trap | 0: disable/1: enable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get trap RRCPv2 frame to 8051 enable/disable
    Trap to RRCP frame to 8051 setting. If this bit is set, ASIC will
    not handled the received RRCP frame.
*/
ret_t rtl8370_getAsicRrcpV2Trap8051(uint32 *trap)
{
    return rtl8370_getAsicRegBit(RTL8370_RRCP_CTRL0_REG, RTL8370_RRCP_TRAP_8051_OFFSET, trap);    
}

