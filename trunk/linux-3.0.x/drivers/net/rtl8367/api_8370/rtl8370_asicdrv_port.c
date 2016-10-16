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
 * $Revision: 23354 $
 * $Date: 2011-09-27 18:29:01 +0800 (星期二, 27 九月 2011) $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_port.h"
#include "rtl8370_asicdrv_phy.h"
/*
@func ret_t | rtl8370_setAsicPortForceFlush | Set per port force flush setting
@parm uint32 | pmsk | portmask(0~0xFFFF)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_MASK | Invalid portmask.
@comm
     This function trigger flushing of per-port L2 learning.
     When flushing operaton completes, the corresponding bit will be clear.
*/
ret_t rtl8370_setAsicPortForceFlush(uint32 pmsk)
{
    if(pmsk > RTL8370_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8370_setAsicReg(RTL8370_FORCE_FLUSH_REG, pmsk);
}

/*
@func ret_t | rtl8370_getAsicPortForceFlush | Get per port force flush setting
@parm uint32* | pmsk | portmask(0~0xFFFF)
@rvalue RT_ERR_OK | Success.
@comm
     This function trigger flushing of per-port L2 learning.
     When flushing operaton completes, the corresponding bit will be clear.
*/
ret_t rtl8370_getAsicPortForceFlush(uint32 *pmsk)
{
    return rtl8370_getAsicReg(RTL8370_FORCE_FLUSH_REG, pmsk);
}

/*
@func ret_t | rtl8370_setAsicPortDisableAging | Set L2 LUT aging per port setting.
@parm uint32 | port | Physical port number.
@parm uint32 | disable | 0: enable aging; 1: disabling aging
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid disable.
@comm
     This API can be used to get L2 LUT aging function per port. 
*/
ret_t rtl8370_setAsicPortDisableAging(uint32 port, uint32 disable)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(disable > 1)
        return RT_ERR_ENABLE;

    return rtl8370_setAsicRegBit(RTL8370_LUT_AGEOUT_CRTL_REG, port, disable);
}

/*
@func ret_t | rtl8370_getAsicPortDisableAging | Get L2 LUT aging per port setting.
@parm uint32 | port | Physical port number.
@parm uint32* | disable | 0: enable aging; 1: disabling aging
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
     This API can be used to get L2 LUT aging function per port. 
*/
ret_t rtl8370_getAsicPortDisableAging(uint32 port, uint32 *disable)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_LUT_AGEOUT_CRTL_REG, port, disable);
}

/*
@func ret_t | rtl8370_setAsicPortUnknownDaBehavior | Set UNDA behavior
@parm uint32 | behavior | 0: flooding; 1: drop; 2:trap
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_NOT_ALLOWED | Invalid behavior.
@comm
    The API set the behavior switch when the DA of received packet is unknown
*/
ret_t rtl8370_setAsicPortUnknownDaBehavior(uint32 behavior)
{
    if(behavior >= L2_BEHAV_MAX)
        return RT_ERR_NOT_ALLOWED;

    return rtl8370_setAsicRegBits(RTL8370_PORT_SECURIT_CTRL_REG, RTL8370_UNKNOWN_UNICAST_DA_BEHAVE_MASK, behavior);
}

/*
@func ret_t | rtl8370_getAsicPortUnknownDaBehavior | Get UNDA behavior
@parm uint32 | behavior | 0: flooding; 1: drop; 2:trap
@rvalue RT_ERR_OK | Success.
@comm
    The API get the behavior switch when the DA of received packet is unknown
*/
ret_t rtl8370_getAsicPortUnknownDaBehavior(uint32 *behavior)
{
    return rtl8370_getAsicRegBits(RTL8370_PORT_SECURIT_CTRL_REG, RTL8370_UNKNOWN_UNICAST_DA_BEHAVE_MASK, behavior);
}

/*
@func ret_t | rtl8370_setAsicPortUnknownSaBehavior | Set UNSA behavior
@parm uint32 | behavior | 0: flooding; 1: drop; 2:trap
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_NOT_ALLOWED | Invalid behavior.
@comm
    The API set the behavior switch when the SA of received packet is unknown
*/
ret_t rtl8370_setAsicPortUnknownSaBehavior(uint32 behavior)
{
    if(behavior >= L2_BEHAV_MAX)
        return RT_ERR_NOT_ALLOWED;

    return rtl8370_setAsicRegBits(RTL8370_PORT_SECURIT_CTRL_REG, RTL8370_UNKNOW_SA_BEHAVE_MASK, behavior);
}

/*
@func ret_t | rtl8370_getAsicPortUnknownSaBehavior | Get UNSA behavior
@parm uint32 | behavior | 0: flooding; 1: drop; 2:trap
@rvalue RT_ERR_OK | Success.
@comm
    The API get the behavior switch when the SA of received packet is unknown
*/
ret_t rtl8370_getAsicPortUnknownSaBehavior(uint32 *behavior)
{
    return rtl8370_getAsicRegBits(RTL8370_PORT_SECURIT_CTRL_REG, RTL8370_UNKNOW_SA_BEHAVE_MASK, behavior);
}

/*
@func ret_t | rtl8370_setAsicPortUnmatchedSaBehavior | Set Unmatched SA behavior
@parm uint32 | behavior | 0: flooding; 1: drop; 2:trap
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_NOT_ALLOWED | Invalid behavior.
@comm
    The API set the behavior switch when the SA and SPA of received packet are unmatched
*/
ret_t rtl8370_setAsicPortUnmatchedSaBehavior(uint32 behavior)
{
    if(behavior >= L2_BEHAV_MAX)
        return RT_ERR_NOT_ALLOWED;

    return rtl8370_setAsicRegBits(RTL8370_PORT_SECURIT_CTRL_REG, RTL8370_UNMATCHED_SA_BEHAVE_MASK, behavior);
}

/*
@func ret_t | rtl8370_getAsicPortUnmatchedSaBehavior | Get Unmatched SA behavior
@parm uint32 | behavior | 0: flooding; 1: drop; 2:trap
@rvalue RT_ERR_OK | Success.
@comm
    The API set the behavior switch when the SA and SPA of received packet are unmatched
*/
ret_t rtl8370_getAsicPortUnmatchedSaBehavior(uint32 *behavior)
{
    return rtl8370_getAsicRegBits(RTL8370_PORT_SECURIT_CTRL_REG, RTL8370_UNMATCHED_SA_BEHAVE_MASK, behavior);
}

/*
@func ret_t | rtl8370_setAsicPortUnknownDaFloodingPortmask | Set UNDA flooding portmask
@parm uint32 | pmsk | porkmask (0~0xFFFF)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_MASK | Invalid portmask
@comm
    This API set the flooding mask of unknown unicast
*/
ret_t rtl8370_setAsicPortUnknownDaFloodingPortmask(uint32 pmsk)
{
    if(pmsk > RTL8370_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8370_setAsicReg(RTL8370_UNUCAST_FLOADING_PMSK_REG, pmsk);
}

/*
@func ret_t | rtl8370_getAsicPortUnknownDaFloodingPortmask | Get UNDA flooding portmask
@parm uint32* | pmsk | portmask
@rvalue RT_ERR_OK | Success.
@comm
    This API get the flooding mask of unknown unicast
*/
ret_t rtl8370_getAsicPortUnknownDaFloodingPortmask(uint32 *pmsk)
{
    return rtl8370_getAsicReg(RTL8370_UNUCAST_FLOADING_PMSK_REG, pmsk);
}

/*
@func ret_t | rtl8370_setAsicPortUnknownMulticastFloodingPortmask | Set UNMC flooding portmask
@parm uint32 | pmsk | portmask
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_MASK | Invalid portmask
@comm
    This API set the flooding mask of unknown multicast
*/
ret_t rtl8370_setAsicPortUnknownMulticastFloodingPortmask(uint32 pmsk)
{
    if(pmsk > RTL8370_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8370_setAsicReg(RTL8370_UNMCAST_FLOADING_PMSK_REG, pmsk);
}

/*
@func ret_t | rtl8370_getAsicPortUnknownMulticastFloodingPortmask | Get UNMC flooding portmask
@parm uint32 | pmsk | portmask (0~0xFFFF)
@rvalue RT_ERR_OK | Success.
@comm
    This API get the flooding mask of unknown multicast
*/
ret_t rtl8370_getAsicPortUnknownMulticastFloodingPortmask(uint32 *pmsk)
{
    return rtl8370_getAsicReg(RTL8370_UNMCAST_FLOADING_PMSK_REG, pmsk);
}

/*
@func ret_t | rtl8370_setAsicPortBcastFloodingPortmask | Set Bcast flooding portmask
@parm uint32 | pmsk | portmask (0~0xFFFF)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_MASK | Invalid portmask.
@comm
    This API set the flooding mask of broadcast
*/
ret_t rtl8370_setAsicPortBcastFloodingPortmask(uint32 pmsk)
{
    if(pmsk > RTL8370_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8370_setAsicReg(RTL8370_BCAST_FLOADING_PMSK_REG, pmsk);
}

/*
@func ret_t | rtl8370_getAsicPortBcastFloodingPortmask | Get Bcast flooding portmask
@parm uint32 | pmsk | portmask(0~0xFFFF)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get the flooding mask of broadcast
*/
ret_t rtl8370_getAsicPortBcastFloodingPortmask(uint32 *pmsk)
{
    return rtl8370_getAsicReg(RTL8370_BCAST_FLOADING_PMSK_REG, pmsk);   
}

/*
@func ret_t | rtl8370_setAsicPortBlockSpa | Disable blocking frame if source port and destination port are the same.
@parm uint32 | port | Physical port number.
@parm uint32 | permit | 0: block; 1: permit
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid permit value.
@comm
     This API is setted to disable block frame if source port = destination port.
*/
ret_t rtl8370_setAsicPortBlockSpa(uint32 port,uint32 permit)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(permit > 1)
        return RT_ERR_ENABLE;

    return rtl8370_setAsicRegBit(RTL8370_SOURCE_PORT_BLOCK_REG, port, permit);
}

/*
@func ret_t | rtl8370_getAsicPortBlockSpa | Disable blocking frame if source port and destination port are the same.
@parm uint32 | port | Physical port number.
@parm uint32 | permit | 0: block; 1: permit
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
     This API is setted to disable block frame if source port = destination port.
*/
ret_t rtl8370_getAsicPortBlockSpa(uint32 port,uint32* permit)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_SOURCE_PORT_BLOCK_REG, port, permit);
}
/*
@func ret_t | rtl8370_setAsicPortForceLink | Set port force linking configuration.
@parm uint32 | port | port number.
@parm rtl8370_port_ability_t* | portability | port ability configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FAILED | Invalid parameter.
@comm
      This API can set Port/MAC force mode properties. 
 */
ret_t rtl8370_setAsicPortForceLink(uint32 port, rtl8370_port_ability_t *portability)
{
    uint32 regData;
    uint16 *accessPtr;
    rtl8370_port_ability_t ability;

    /* Invalid input parameter */
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
    
    memset(&ability, 0x00, sizeof(rtl8370_port_ability_t));
    memcpy(&ability, portability, sizeof(rtl8370_port_ability_t));    

    accessPtr =  (uint16*)&ability;
 
    regData = *accessPtr;
    return rtl8370_setAsicReg(RTL8370_REG_MAC0_FORCE_SELECT+port,regData);
}


/*
@func ret_t | rtl8370_getAsicPortForceLink | Get port force linking configuration.
@parm uint32 | port | port number.
@parm rtl8370_port_ability_t* | portability | port ability configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FAILED | Invalid parameter.
@comm
      This API can get Port/MAC force mode properties. 
 */
ret_t rtl8370_getAsicPortForceLink(uint32 port, rtl8370_port_ability_t *portability)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    rtl8370_port_ability_t ability;

    /* Invalid input parameter */
    if(port >=RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
    
    memset(&ability, 0x00, sizeof(rtl8370_port_ability_t));


    accessPtr =  (uint16*)&ability;
 
    retVal = rtl8370_getAsicReg(RTL8370_REG_MAC0_FORCE_SELECT+port,&regData);
    if(retVal !=  RT_ERR_OK)
        return retVal;
    
    *accessPtr = regData;

    memcpy(portability, &ability, sizeof(rtl8370_port_ability_t));        
    
    return RT_ERR_OK;  
}

/*
@func ret_t | rtl8370_getAsicPortStatus | Get port link status.
@parm uint32 | port | port number.
@parm rtl8370_port_ability_t* | portability | port ability configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FAILED | Invalid parameter.
@comm
      This API can get Port/PHY properties. 
 */
ret_t rtl8370_getAsicPortStatus(uint32 port, rtl8370_port_status_t *portstatus)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    rtl8370_port_status_t status;

    /* Invalid input parameter */
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
    
    memset(&status, 0x00, sizeof(rtl8370_port_status_t));


    accessPtr =  (uint16*)&status;
 
    retVal = rtl8370_getAsicReg(RTL8370_REG_PORT0_STATUS + port, &regData);
    if(retVal !=  RT_ERR_OK)
        return retVal;
    
    *accessPtr = regData;

    memcpy(portstatus, &status, sizeof(rtl8370_port_status_t));        
    
    return RT_ERR_OK;  
}

/*
@func ret_t | rtl8370_setAsicPortForceLinkExt | Set external interface force linking configuration.
@parm uint32 | id | external interface id (0~1).
@parm rtl8370_port_ability_t* | portability | port ability configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FAILED | Invalid parameter.
@comm
      This API can set external interface force mode properties. 
 */
ret_t rtl8370_setAsicPortForceLinkExt(uint32 id, rtl8370_port_ability_t *portability)
{
    uint32 regData;
    uint16 *accessPtr;
    rtl8370_port_ability_t ability;

    /* Invalid input parameter */
    if(id >=RTL8370_EXTNO)
        return RT_ERR_PORT_ID;
    
    memset(&ability, 0x00, sizeof(rtl8370_port_ability_t));
    memcpy(&ability, portability, sizeof(rtl8370_port_ability_t));    

    accessPtr = (uint16*)&ability;
 
    regData = *accessPtr;

    return rtl8370_setAsicReg(RTL8370_REG_DIGITIAL_INTERFACE0_FORCE + id, regData);
}


/*
@func ret_t | rtl8370_getAsicPortForceLinkExt | Get external interface force linking configuration.
@parm uint32 | id | external interface id (0~1).
@parm rtl8370_port_ability_t* | portability | port ability configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FAILED | Invalid parameter.
@comm
      This API can get external interface force mode properties. 
 */
ret_t rtl8370_getAsicPortForceLinkExt(uint32 id, rtl8370_port_ability_t *portability)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    rtl8370_port_ability_t ability;

    /* Invalid input parameter */
    if(id >= RTL8370_EXTNO)
        return RT_ERR_PORT_ID;
    
    memset(&ability,0x00,sizeof(rtl8370_port_ability_t));


    accessPtr =  (uint16*)&ability;
 
    retVal = rtl8370_getAsicReg(RTL8370_REG_DIGITIAL_INTERFACE0_FORCE + id, &regData);
    if(retVal !=  RT_ERR_OK)
        return retVal;
    
    *accessPtr = regData;

    memcpy(portability, &ability, sizeof(rtl8370_port_ability_t));        
    
    return RT_ERR_OK;  
}

/*
@func ret_t | rtl8370_setAsicPortExtMode | Set external interface mode configuration.
@parm uint32 | id | external interface id (0~1).
@parm uint32 |mode | external interface mode.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FAILED | Invalid parameter.
@comm
  This API can set external interface mode properties. 
    DISABLE,
    RGMII,
    MII_MAC,
    MII_PHY, 
    TMII_MAC,
    TMII_PHY, 
    GMII,
    RGMII_33V,    
 */
ret_t rtl8370_setAsicPortExtMode(uint32 id, uint32 mode)
{
    ret_t retVal;
    
    if(id >= RTL8370_EXTNO)
        return RT_ERR_INPUT;	

    if(mode > EXT_RGMII_33V)
        return RT_ERR_INPUT;	

    if( mode == EXT_RGMII_33V || mode == EXT_RGMII )
    {
        if((retVal= rtl8370_setAsicReg(RTL8370_REG_CHIP_DEBUG0,0x0367)) !=  RT_ERR_OK)
            return retVal;
        if((retVal= rtl8370_setAsicReg(RTL8370_REG_CHIP_DEBUG1,0x7777)) !=  RT_ERR_OK)
            return retVal;
    }
    else if((mode == EXT_TMII_MAC)||(mode == EXT_TMII_PHY))
    {
        if((retVal= rtl8370_setAsicRegBit(RTL8370_REG_BYPASS_LINE_RATE,(id+1)%2,1)) !=  RT_ERR_OK)
            return retVal;
    } 
    else if( mode == EXT_GMII )
    {
        if((retVal= rtl8370_setAsicRegBit(RTL8370_REG_CHIP_DEBUG0,RTL8370_CHIP_DEBUG0_DUMMY_0_OFFSET+id,1)) !=  RT_ERR_OK)
            return retVal;
        if((retVal= rtl8370_setAsicRegBit(RTL8370_REG_EXT0_RGMXF+id,6,1)) !=  RT_ERR_OK)
            return retVal;        
    } 
    else 
    {
        if((retVal= rtl8370_setAsicRegBit(RTL8370_REG_BYPASS_LINE_RATE,(id+1)%2,0)) !=  RT_ERR_OK)
            return retVal;
        if((retVal= rtl8370_setAsicRegBit(RTL8370_REG_EXT0_RGMXF+id,6,0)) !=  RT_ERR_OK)
            return retVal;     
    }      

    return rtl8370_setAsicRegBits(RTL8370_REG_DIGITIAL_INTERFACE_SELECT, RTL8370_SELECT_RGMII_0_MASK<<(id*RTL8370_SELECT_RGMII_1_OFFSET), mode);
}

/*
@func ret_t | rtl8370_getAsicPortExtMode | Get external interface mode configuration.
@parm uint32 | id | external interface id (0~1).
@parm uint32 |mode | external interface mode.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FAILED | Invalid parameter.
@comm
  This API can get external interface mode properties. 
    DISABLE,
    RGMII,
    MII_MAC,
    MII_PHY, 
    TMII_MAC,
    TMII_PHY, 
    GMII     
 */
ret_t rtl8370_getAsicPortExtMode(uint32 id, uint32 *mode)
{
    if(id >= RTL8370_EXTNO)
        return RT_ERR_INPUT;	
	
    return rtl8370_getAsicRegBits(RTL8370_REG_DIGITIAL_INTERFACE_SELECT, RTL8370_SELECT_RGMII_0_MASK<<(id*RTL8370_SELECT_RGMII_1_OFFSET), mode);
}

/*
@func ret_t | rtl8370_setAsicPortEnableAll | Set ALL ports enable.
@parm uint32 |enable | enable all ports.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FAILED | Invalid parameter.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
  This API can set all ports enable.  
 */
ret_t rtl8370_setAsicPortEnableAll(uint32 enable)
{
    if(enable >= 2)
        return RT_ERR_INPUT;

    return rtl8370_setAsicRegBit(RTL8370_REG_PHY_AD, RTL8370_PHY_AD_DUMMY_1_OFFSET, !enable);
}
/*
@func ret_t | rtl8370_getAsicPortEnableAll | Get ALL ports enable.
@parm uint32 | *enable | enable all ports.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FAILED | Invalid parameter.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
  This API can set all ports enable.  
 */
ret_t rtl8370_getAsicPortEnableAll(uint32 *enable)
{
    ret_t retVal;
    uint32 regData;
    
    retVal = rtl8370_getAsicRegBit(RTL8370_REG_PHY_AD, RTL8370_PHY_AD_DUMMY_1_OFFSET, &regData);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    if (regData==0)
        *enable = 1;
    else
        *enable = 0;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicPortRTCT
 * Description:
 *      Set RTCT
 * Input:
 *      portmask 	- Port mask of RTCT enabled (0-4)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK 		    - Success
 *      RT_ERR_SMI  	    - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid port mask
 * Note:
 *      RTCT test takes 4.8 seconds at most.
 */
ret_t rtl8370_setAsicPortRTCT(uint32 portmask)
{
    ret_t   retVal;

    if(portmask > (0x0001 << RTL8370_PHYNO))
		return RT_ERR_PORT_MASK;

    if((retVal = rtl8370_setAsicRegBits(RTL8370_REG_RTCT_ENABLE, 0xFF, portmask)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8370_setAsicRegBit(RTL8370_REG_SEL_RTCT_PARA, RTL8370_EN_RTCT_TIMOUT_OFFSET, 1)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8370_setAsicRegBit(RTL8370_REG_SEL_RTCT_PARA, RTL8370_EN_ALL_RTCT_OFFSET, 0)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8370_setAsicRegBit(RTL8370_REG_SEL_RTCT_PARA, RTL8370_DO_RTCT_COMMAND_OFFSET, 0)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8370_setAsicRegBit(RTL8370_REG_SEL_RTCT_PARA, RTL8370_DO_RTCT_COMMAND_OFFSET, 1)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicPortRTCTResult
 * Description:
 *      Get RTCT result
 * Input:
 *      port 	- Port ID of RTCT result
 * Output:
 *      pResult - The result of port ID
 * Return:
 *      RT_ERR_OK 		            - Success
 *      RT_ERR_SMI  	            - SMI access error
 *      RT_ERR_PORT_MASK            - Invalid port mask
 *      RT_ERR_PHY_RTCT_NOT_FINISH  - RTCT test doesn't finish.
 * Note:
 *      RTCT test takes 4.8 seconds at most.
 *      If this API returns RT_ERR_PHY_RTCT_NOT_FINISH,
 *      users should wait a whole then read it again.
 */
ret_t rtl8370_getAsicPortRTCTResult(uint32 port, rtl8370_port_rtct_result_t *pResult)
{
    ret_t       retVal;
    uint32  regData, finish = 1;

    if(port >= RTL8370_PHYNO)
		return RT_ERR_PORT_ID;

    if((retVal = rtl8370_setAsicPHYReg(port, RTL8370_PHY_PAGE_ADDRESS, RTL8370_RTCT_PAGE)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8370_getAsicPHYReg(port, RTL8370_RTCT_RESULT_A_REG, &regData)) != RT_ERR_OK)
        return retVal;

    if((regData & 0x4000) == 0x4000)
    {
        pResult->channelALen = regData & 0x1FFF;

        if((retVal = rtl8370_getAsicPHYReg(port, RTL8370_RTCT_RESULT_B_REG, &regData)) != RT_ERR_OK)
            return retVal;

        pResult->channelBLen = regData & 0x1FFF;

        if((retVal = rtl8370_getAsicPHYReg(port, RTL8370_RTCT_RESULT_C_REG, &regData)) != RT_ERR_OK)
            return retVal;

        pResult->channelCLen = regData & 0x1FFF;

        if((retVal = rtl8370_getAsicPHYReg(port, RTL8370_RTCT_RESULT_D_REG, &regData)) != RT_ERR_OK)
            return retVal;

        pResult->channelDLen = regData & 0x1FFF;

        if((retVal = rtl8370_getAsicPHYReg(port, RTL8370_RTCT_STATUS_REG, &regData)) != RT_ERR_OK)
            return retVal;

        pResult->channelALinedriver = (regData & 0x0001);
        pResult->channelBLinedriver = (regData & 0x0002);
        pResult->channelCLinedriver = (regData & 0x0004);
        pResult->channelDLinedriver = (regData & 0x0008);

        pResult->channelAMismatch   = (regData & 0x0010);
        pResult->channelBMismatch   = (regData & 0x0020);
        pResult->channelCMismatch   = (regData & 0x0040);
        pResult->channelDMismatch   = (regData & 0x0080);

        pResult->channelAOpen       = (regData & 0x0100);
        pResult->channelBOpen       = (regData & 0x0200);
        pResult->channelCOpen       = (regData & 0x0400);
        pResult->channelDOpen       = (regData & 0x0800);

        pResult->channelAShort      = (regData & 0x1000);
        pResult->channelBShort      = (regData & 0x2000);
        pResult->channelCShort      = (regData & 0x4000);
        pResult->channelDShort      = (regData & 0x8000);
    }
    else
        finish = 0;

    if((retVal = rtl8370_setAsicPHYReg(port, RTL8370_PHY_PAGE_ADDRESS, 0)) != RT_ERR_OK)
        return retVal;

    if(finish == 0)
        return RT_ERR_PHY_RTCT_NOT_FINISH;
    else
        return RT_ERR_OK;
}



