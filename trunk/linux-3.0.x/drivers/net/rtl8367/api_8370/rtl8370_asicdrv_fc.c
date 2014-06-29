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
 * $Date: 2010/12/02 04:34:20 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_fc.h"

/*
@func ret_t | rtl8370_setAsicFlowControlSelect | Set system flow control type
@parm uint32 | select | System flow control tyep 1: Ingress flow control 0:Egress flow control.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
     This API can be used to set system flow control type. ASIC will operate ingress or egress flow 
     control by this configuration
*/
ret_t rtl8370_setAsicFlowControlSelect(uint32 select)
{
    uint32 retVal;
    uint32 regData;
    
    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_MAGIC_ID,0x0249))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicRegBits(RTL8370_REG_CHIP_VER, RTL8370_RLVID_MASK,&regData))!=RT_ERR_OK)
        return retVal; 


    if (0 == regData)
    {
        retVal = rtl8370_setAsicRegBit(RTL8370_FLOWCTRL_CTRL_REG, 12,select);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8370_setAsicRegBit(RTL8370_FLOWCTRL_CTRL_REG, RTL8370_FLOWCTRL_TYPE_OFFSET,select);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}
/*
@func ret_t | rtl8370_getAsicFlowControlSelect | Get system flow control type
@parm uint32* | select | System flow control tyep 1: Ingress flow control 0:Egress flow control.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
     This API can be used to get system flow control type. ASIC will operate ingress or egress flow 
     control by this configuration
*/
ret_t rtl8370_getAsicFlowControlSelect(uint32 *select)
{
    uint32 retVal;
    uint32 regData;
    
    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_MAGIC_ID,0x0249))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicRegBits(RTL8370_REG_CHIP_VER, RTL8370_RLVID_MASK,&regData))!=RT_ERR_OK)
        return retVal; 


    if (0 == regData)
    {
        retVal = rtl8370_getAsicRegBit(RTL8370_FLOWCTRL_CTRL_REG, 12,select);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8370_getAsicRegBit(RTL8370_FLOWCTRL_CTRL_REG, RTL8370_FLOWCTRL_TYPE_OFFSET,select);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}
/*
@func ret_t | rtl8370_setAsicFlowControlQueueEgressEnable | Set flow control ability for each queue. 
@parm uint32 | port | The port number. 
@parm uint32 | qid | The queue id. 
@parm uint32 | enabled | 1: enabled, 0: disabled.  
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_QUEUE_ID | Invalid queue id.
@comm
    The API can enable or disable flow control ability of the specified output queue. 
 */
ret_t rtl8370_setAsicFlowControlQueueEgressEnable(uint32 port,uint32 qid, uint32 enabled)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8370_QIDMAX) 
        return RT_ERR_QUEUE_ID;
    
    return rtl8370_setAsicRegBit(RTL8370_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG(port), RTL8370_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG_OFFSET(port)+ qid, enabled);
}
/*
@func ret_t | rtl8370_getAsicFlowControlQueueEgressEnable | Set flow control ability for each queue. 
@parm uint32 | port | The port number. 
@parm uint32 | qid | The queue id. 
@parm uint32* | enabled | 1: enabled, 0: disabled.  
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_QUEUE_ID | Invalid queue id.
@comm
    The API can get setting of flow control ability of the specified output queue. 
 */
ret_t rtl8370_getAsicFlowControlQueueEgressEnable(uint32 port,uint32 qid, uint32* enabled)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8370_QIDMAX) 
        return RT_ERR_QUEUE_ID;
    
    return rtl8370_getAsicRegBit(RTL8370_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG(port), RTL8370_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG_OFFSET(port)+ qid, enabled);
}
/*
@func ret_t | rtl8370_setAsicFlowControlPortEgressEnable | Set per-port egress flow control
@parm uint32 | port | The port number
@parm uint32 | enabled | Egress port flow control usage 1:enable 0:disable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
     This API can be used to set per-port egress flow control. If per-port egress flow control is disabled, ASIC will
     directly drop incoming packet while page usage overed threshold
*/
ret_t rtl8370_setAsicFlowControlPortEgressEnable(uint32 port,uint32 enable)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_setAsicRegBit(RTL8370_FLOWCRTL_EGRESS_PORT_ENABLE_REG,port,enable);
}

/*
@func ret_t | rtl8370_getAsicFlowControlPortEgressEnable | Get per-port egress flow control
@parm uint32 | port | The port number
@parm uint32* | enabled | Egress port flow control usage 1:enable 0:disable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
     This API can be used to get per-port egress flow control. If per-port egress flow control is disabled, ASIC will
     directly drop incoming packet while page usage overed threshold
*/
ret_t rtl8370_getAsicFlowControlPortEgressEnable(uint32 port,uint32* enable)
{    
    return rtl8370_getAsicRegBit(RTL8370_FLOWCRTL_EGRESS_PORT_ENABLE_REG,port,enable);
}
/*
@func ret_t | rtl8370_setAsicFlowControlDropAll | Set system-based drop parameters. 
@parm uint32 | dropall | Whole system drop threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can set drop threshold of whole system. The system drop all pages is 2048-4*dropall
 */
ret_t rtl8370_setAsicFlowControlDropAll(uint32 dropall)
{
    uint32 retVal;
    uint32 regData;
    
    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_MAGIC_ID,0x0249))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicRegBits(RTL8370_REG_CHIP_VER, RTL8370_RLVID_MASK,&regData))!=RT_ERR_OK)
        return retVal; 


    if (0 == regData)
    {
        if(dropall > 15)
            return RT_ERR_INPUT;
    
        retVal = rtl8370_setAsicRegBits(RTL8370_REG_FLOWCTRL_CTRL0, 0x0F00, dropall);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
    if(dropall > RTL8370_INGRESS_DROP_ALL_THREHSOLD_MAX)
        return RT_ERR_INPUT;

        retVal = rtl8370_setAsicRegBits(RTL8370_REG_FLOWCTRL_CTRL0, RTL8370_DROP_ALL_THRESHOLD_MASK, dropall);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicFlowControlDropAll | Set system-based drop parameters. 
@parm uint32* | dropall | Whole system drop threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can set drop threshold of whole system.
 */
ret_t rtl8370_getAsicFlowControlDropAll(uint32* dropall)
{
    uint32 retVal;
    uint32 regData;
    
    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_MAGIC_ID,0x0249))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicRegBits(RTL8370_REG_CHIP_VER, RTL8370_RLVID_MASK,&regData))!=RT_ERR_OK)
        return retVal; 


    if (0 == regData)
    {   
        retVal = rtl8370_getAsicRegBits(RTL8370_REG_FLOWCTRL_CTRL0, 0x0F00, dropall);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {    
        retVal = rtl8370_getAsicRegBits(RTL8370_REG_FLOWCTRL_CTRL0, RTL8370_DROP_ALL_THRESHOLD_MASK, dropall);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}
/*
@func ret_t | rtl8370_setAsicFlowControlPauseAll | Set system-based all ports enable flow control parameters. 
@parm uint32 | threshold | Whole system drop threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set all ports enable flow control threshold of whole system.
 */
ret_t rtl8370_setAsicFlowControlPauseAllThreshold(uint32 threshold)
{
    if(threshold >= RTL8370_PAGE_NUMBER)
        return RT_ERR_PHY_PAGE_ID;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_ALL_ON_REG, RTL8370_FLOWCTRL_ALL_ON_THRESHOLD_MASK, threshold);
}
/*
@func ret_t | rtl8370_getAsicFlowControlPauseAllThreshold | Get system-based all ports enable flow control parameters. 
@parm uint32 | threshold | Whole system drop threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get all ports enable flow control threshold of whole system.
 */
ret_t rtl8370_getAsicFlowControlPauseAllThreshold(uint32 *threshold)
{    
    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_ALL_ON_REG, RTL8370_FLOWCTRL_ALL_ON_THRESHOLD_MASK, threshold);
}
/*
@func ret_t | rtl8370_setAsicFlowControlSystemThreshold | Set system-based flow control parameters. 
@parm uint32 | onThreshold | System-based flow control turn ON threshold. 
@parm uint32 | offThreshold | System-based flow control turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set system-based flow control threshold. This threshold is used by which port is flow control enabled.
 */
ret_t rtl8370_setAsicFlowControlSystemThreshold(uint32 onThreshold, uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold > RTL8370_PAGE_NUMBER) || (offThreshold > RTL8370_PAGE_NUMBER))
        return RT_ERR_PHY_PAGE_ID;
    
    retVal = rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_SYS_OFF_REG, RTL8370_FLOWCTRL_SYS_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_SYS_ON_REG, RTL8370_FLOWCTRL_SYS_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_getAsicFlowControlSystemThreshold | Get system-based flow control parameters. 
@parm uint32* | onThreshold | System-based flow control turn ON threshold. 
@parm uint32* | offThreshold | System-based flow control turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get system-based flow control threshold. This threshold is used by which port is flow control enabled.
 */
ret_t rtl8370_getAsicFlowControlSystemThreshold(uint32 *onThreshold, uint32 *offThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_SYS_OFF_REG, RTL8370_FLOWCTRL_SYS_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_SYS_ON_REG, RTL8370_FLOWCTRL_SYS_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_setAsicFlowControlSharedThreshold | Set share-based flow control parameters. 
@parm uint32 | onThreshold | Share-based flow control turn ON threshold. 
@parm uint32 | offThreshold | Share-based flow control turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set shared-based flow control threshold.  This threshold is used by which port is flow control enabled. 
 */
ret_t rtl8370_setAsicFlowControlSharedThreshold(uint32 onThreshold, uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold > RTL8370_PAGE_NUMBER) || (offThreshold > RTL8370_PAGE_NUMBER))
        return RT_ERR_PHY_PAGE_ID;
    
    retVal = rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_SHARE_OFF_REG, RTL8370_FLOWCTRL_SHARE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_SHARE_ON_REG, RTL8370_FLOWCTRL_SHARE_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_getAsicFlowControlSharedThreshold | Get share-based flow control parameters. 
@parm uint32* | onThreshold | Share-based flow control turn ON threshold. 
@parm uint32* | offThreshold | Share-based flow control turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get shared-based flow control threshold.  This threshold is used by which port is flow control enabled.
 */
ret_t rtl8370_getAsicFlowControlSharedThreshold(uint32 *onThreshold, uint32 *offThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_SHARE_OFF_REG, RTL8370_FLOWCTRL_SHARE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_SHARE_ON_REG, RTL8370_FLOWCTRL_SHARE_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_setAsicFlowControlPortThreshold | Set Port-based flow control parameters. 
@parm uint32 | onThreshold | Port-based flow control turn ON threshold. 
@parm uint32 | offThreshold | Port-based flow control turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set Port-based flow control threshold. This threshold is used by ingress flow control mechanism and packet ingress port is flow control enabled
 */
ret_t rtl8370_setAsicFlowControlPortThreshold(uint32 onThreshold, uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold > RTL8370_PAGE_NUMBER) || (offThreshold > RTL8370_PAGE_NUMBER))
        return RT_ERR_PHY_PAGE_ID;
    
    retVal = rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_PORT_OFF_REG, RTL8370_FLOWCTRL_PORT_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_PORT_ON_REG, RTL8370_FLOWCTRL_PORT_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_getAsicFlowControlPortThreshold | Get Port-based flow control parameters. 
@parm uint32* | onThreshold | Port-based flow control turn ON threshold. 
@parm uint32* | offThreshold | Port-based flow control turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get Port-based flow control threshold. This threshold is used by ingress flow control mechanism and packet ingress port is flow control enabled
 */
ret_t rtl8370_getAsicFlowControlPortThreshold(uint32 *onThreshold, uint32 *offThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_PORT_OFF_REG, RTL8370_FLOWCTRL_PORT_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_PORT_ON_REG, RTL8370_FLOWCTRL_PORT_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_setAsicFlowControlPortPrivateThreshold | Set Port-private-based flow control parameters. 
@parm uint32 | onThreshold | Port-private-based flow control turn ON threshold. 
@parm uint32 | offThreshold | Port-private-based flow control turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set Port-private-based flow control threshold. This threshold is used by ingress flow control mechanism and packet ingress port is flow control enabled
 */
ret_t rtl8370_setAsicFlowControlPortPrivateThreshold(uint32 onThreshold, uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold > RTL8370_PAGE_NUMBER) || (offThreshold > RTL8370_PAGE_NUMBER))
        return RT_ERR_PHY_PAGE_ID;
    
    retVal = rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_PORT_PRIVATE_OFF_REG, RTL8370_FLOWCTRL_PORT_PRIVATE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_PORT_PRIVATE_ON_REG, RTL8370_FLOWCTRL_PORT_PRIVATE_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_getAsicFlowControlPortPrivateThreshold | Get Port-private-based flow control parameters. 
@parm uint32* | onThreshold | Port-private-based flow control turn ON threshold. 
@parm uint32* | offThreshold | Port-private-based flow control turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get Port-private-based flow control threshold. This threshold is used by ingress flow control mechanism and packet ingress port is flow control enabled
 */
ret_t rtl8370_getAsicFlowControlPortPrivateThreshold(uint32 *onThreshold, uint32 *offThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_PORT_PRIVATE_OFF_REG, RTL8370_FLOWCTRL_PORT_PRIVATE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_PORT_PRIVATE_ON_REG, RTL8370_FLOWCTRL_PORT_PRIVATE_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_setAsicFlowControlSystemDropThreshold | Set system-based drop parameters. 
@parm uint32 | onThreshold | System-based drop turn ON threshold. 
@parm uint32 | offThreshold | System-based drop turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set system-based drop threshold. This threshold is used by which port is flow control disabled.
 */
ret_t rtl8370_setAsicFlowControlSystemDropThreshold(uint32 onThreshold, uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold > RTL8370_PAGE_NUMBER) || (offThreshold > RTL8370_PAGE_NUMBER))
        return RT_ERR_PHY_PAGE_ID;
    
    retVal = rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_FCOFF_SYS_OFF_REG, RTL8370_FLOWCTRL_FCOFF_SYS_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_FCOFF_SYS_ON_REG, RTL8370_FLOWCTRL_FCOFF_SYS_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_getAsicFlowControlSystemDropThreshold | Get system-based drop parameters. 
@parm uint32* | onThreshold | System-based drop turn ON threshold. 
@parm uint32* | offThreshold | System-based drop turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get system-based drop threshold. This threshold is used by which port is flow control disabled.
 */
ret_t rtl8370_getAsicFlowControlSystemDropThreshold(uint32 *onThreshold, uint32 *offThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_FCOFF_SYS_OFF_REG, RTL8370_FLOWCTRL_FCOFF_SYS_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_FCOFF_SYS_ON_REG, RTL8370_FLOWCTRL_FCOFF_SYS_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_setAsicFlowControlSharedDropThreshold | Set share-based fdrop parameters. 
@parm uint32 | onThreshold | Share-based drop turn ON threshold. 
@parm uint32 | offThreshold | Share-based drop turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set shared-based drop threshold.  This threshold is used by which port is flow control disabled. 
 */
ret_t rtl8370_setAsicFlowControlSharedDropThreshold(uint32 onThreshold, uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold > RTL8370_PAGE_NUMBER) || (offThreshold > RTL8370_PAGE_NUMBER))
        return RT_ERR_PHY_PAGE_ID;
    
    retVal = rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_FCOFF_SHARE_OFF_REG, RTL8370_FLOWCTRL_FCOFF_SHARE_OFF_MASK, offThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_FCOFF_SHARE_ON_REG, RTL8370_FLOWCTRL_FCOFF_SHARE_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_getAsicFlowControlSharedDropThreshold | Set share-based drop parameters. 
@parm uint32* | onThreshold | Share-based drop turn ON threshold. 
@parm uint32* | offThreshold | Share-based drop turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get shared-based drop threshold.  This threshold is used by which port is flow control disabled.
 */
ret_t rtl8370_getAsicFlowControlSharedDropThreshold(uint32 *onThreshold, uint32 *offThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_FCOFF_SHARE_OFF_REG, RTL8370_FLOWCTRL_FCOFF_SHARE_OFF_MASK, offThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_FCOFF_SHARE_ON_REG, RTL8370_FLOWCTRL_FCOFF_SHARE_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_setAsicFlowControlPortDropThreshold | Set Port-based drop parameters. 
@parm uint32 | onThreshold | Port-based drop turn ON threshold. 
@parm uint32 | offThreshold | Port-based drop turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set Port-based drop threshold. This threshold is used by ingress flow control mechanism and packet ingress port is flow control disabled
 */
ret_t rtl8370_setAsicFlowControlPortDropThreshold(uint32 onThreshold, uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold > RTL8370_PAGE_NUMBER) || (offThreshold > RTL8370_PAGE_NUMBER))
        return RT_ERR_PHY_PAGE_ID;
    
    retVal = rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_FCOFF_PORT_OFF_REG, RTL8370_FLOWCTRL_FCOFF_PORT_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_FCOFF_PORT_ON_REG, RTL8370_FLOWCTRL_FCOFF_PORT_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_getAsicFlowControlPortDropThreshold | Get Port-based drop parameters. 
@parm uint32* | onThreshold | Port-based drop turn ON threshold. 
@parm uint32* | offThreshold | Port-based drop turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get Port-based drop threshold. This threshold is used by ingress flow control mechanism and packet ingress port is flow control disabled
 */
ret_t rtl8370_getAsicFlowControlPortDropThreshold(uint32 *onThreshold, uint32 *offThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_FCOFF_PORT_OFF_REG, RTL8370_FLOWCTRL_FCOFF_PORT_OFF_MASK, offThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_FCOFF_PORT_ON_REG, RTL8370_FLOWCTRL_FCOFF_PORT_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_setAsicFlowControlPortPrivateDropThreshold | Set Port-private-based drop parameters. 
@parm uint32 | onThreshold | Port-private-based drop turn ON threshold. 
@parm uint32 | offThreshold | Port-private-based drop turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set Port-private-based drop threshold. This threshold is used by ingress flow control mechanism and packet ingress port is flow control disabled
 */
ret_t rtl8370_setAsicFlowControlPortPrivateDropThreshold(uint32 onThreshold, uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold > RTL8370_PAGE_NUMBER) || (offThreshold > RTL8370_PAGE_NUMBER))
        return RT_ERR_PHY_PAGE_ID;
    
    retVal = rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF_REG, RTL8370_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_FCOFF_PORT_PRIVATE_ON_REG, RTL8370_FLOWCTRL_FCOFF_PORT_PRIVATE_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_getAsicFlowControlPortPrivateDropThreshold | Get Port-private-based drop parameters. 
@parm uint32* | onThreshold | Port-private-based drop turn ON threshold. 
@parm uint32* | offThreshold | Port-private-based drop turn OFF threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get Port-private-based drop threshold. This threshold is used by ingress flow control mechanism and packet ingress port is flow control disabled
 */
ret_t rtl8370_getAsicFlowControlPortPrivateDropThreshold(uint32 *onThreshold, uint32 *offThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF_REG, RTL8370_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF_MASK, offThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_FCOFF_PORT_PRIVATE_ON_REG, RTL8370_FLOWCTRL_FCOFF_PORT_PRIVATE_ON_MASK, onThreshold);
}
/*
@func ret_t | rtl8370_setAsicEgressFlowControlQueueDropThreshold | Set Queue-based egress flow control turn on or ingress flow control drop on threshold. 
@parm uint32 | qid | The queue id. 
@parm uint32 | threshold | Queue-based flown control/drop turn ON threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@rvalue RT_ERR_QUEUE_ID | Invalid queue id.
@comm
    The API can set Queue-based egress flow control turn on threshold or queue-based egress drop threshold for ingress flow control mechanism. 
 */
ret_t rtl8370_setAsicEgressFlowControlQueueDropThreshold(uint32 qid, uint32 threshold)
{
    if(threshold >= RTL8370_PAGE_NUMBER)
        return RT_ERR_PHY_PAGE_ID;
    
    if(qid > RTL8370_QIDMAX)
        return RT_ERR_QUEUE_NUM;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_QUEUE_DROP_ON_REG(qid), RTL8370_FLOWCTRL_QUEUE_DROP_ON_MASK, threshold);
}
/*
@func ret_t | rtl8370_getAsicEgressFlowControlQueueDropThreshold | Get Queue-based egress flow control turn on or ingress flow control drop on threshold. 
@parm uint32 | qid | The queue id. 
@parm uint32* | threshold | Queue-based flown control/drop turn ON threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_QUEUE_ID | Invalid queue id.
@comm
    The API can get Queue-based egress flow control turn on threshold or queue-based egress drop threshold for ingress flow control mechanism. 
 */
ret_t rtl8370_getAsicEgressFlowControlQueueDropThreshold(uint32 qid, uint32 *threshold)
{
    if(qid > RTL8370_QIDMAX)
      return RT_ERR_QUEUE_NUM;

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_QUEUE_DROP_ON_REG(qid), RTL8370_FLOWCTRL_QUEUE_DROP_ON_MASK, threshold);
}
/*
@func ret_t | rtl8370_setAsicEgressFlowControlPortDropThreshold | Set port-based egress flow control turn on or ingress flow control drop on threshold. 
@parm uint32 | port | The port number
@parm uint32 | threshold | Port-based flown control/drop turn ON threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set Port-based egress flow control turn on threshold or port-based egress drop threshold for ingress flow control mechanism. 
 */
ret_t rtl8370_setAsicEgressFlowControlPortDropThreshold(uint32 port, uint32 threshold)
{
    if(port > RTL8370_PORTIDMAX)
      return RT_ERR_PORT_ID;

    if(threshold >= RTL8370_PAGE_NUMBER)
      return RT_ERR_PHY_PAGE_ID;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_PORT_DROP_ON_REG(port), RTL8370_FLOWCTRL_PORT_DROP_ON_MASK, threshold);
}
/*
@func ret_t | rtl8370_getAsicEgressFlowControlPortDropThreshold | Get port-based egress flow control turn on or ingress flow control drop on threshold. 
@parm uint32 | port | The port number
@parm uint32* | threshold | Port-based flown control/drop turn ON threshold. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get Port-based egress flow control turn on threshold or port-based egress drop threshold for ingress flow control mechanism. 
 */
ret_t rtl8370_getAsicEgressFlowControlPortDropThreshold(uint32 port, uint32 *threshold)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID; 

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_PORT_DROP_ON_REG(port), RTL8370_FLOWCTRL_PORT_ON_MASK, threshold);
}
/*
@func ret_t | rtl8370_setAsicEgressFlowControlPortDropGap | Set port-based egress flow control turn off or ingress flow control drop off gap. 
@parm uint32 | gap | Port-based flown control/drop turn OFF threshold = turn ON threshold - gap. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set Port-based egress flow control turn off gap or port-based egress drop off gap for ingress flow control mechanism. 
 */
ret_t rtl8370_setAsicEgressFlowControlPortDropGap(uint32 gap)
{
    if(gap >= RTL8370_PAGE_NUMBER)
        return RT_ERR_PHY_PAGE_ID;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_PORT_GAP_REG, RTL8370_FLOWCTRL_PORT_GAP_MASK, gap);
}
/*
@func ret_t | rtl8370_getAsicEgressFlowControlPortDropGap | Get port-based egress flow control turn off or ingress flow control drop off gap. 
@parm uint32* | gap | Port-based flown control/drop turn OFF threshold = turn ON threshold - gap. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get Port-based egress flow control turn off gap or port-based egress drop off gap for ingress flow control mechanism. 
 */
ret_t rtl8370_getAsicEgressFlowControlPortDropGap(uint32 *gap)
{
    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_PORT_GAP_REG, RTL8370_FLOWCTRL_PORT_GAP_MASK, gap);
}
/*
@func ret_t | rtl8370_setAsicEgressFlowControlQueueDropGap | Set Queue-based egress flow control turn off or ingress flow control drop off gap. 
@parm uint32 | gap | Queue-based flown control/drop turn OFF threshold = turn ON threshold - gap. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PHY_PAGE_ID | Invalid page threshold.
@comm
    The API can set Queue-based egress flow control turn off gap or queue-based egress drop off gap for ingress flow control mechanism. 
 */
ret_t rtl8370_setAsicEgressFlowControlQueueDropGap(uint32 gap)
{
    if(gap >= RTL8370_PAGE_NUMBER)
        return RT_ERR_PHY_PAGE_ID;

    return rtl8370_setAsicRegBits(RTL8370_FLOWCTRL_QUEUE_GAP_REG, RTL8370_FLOWCTRL_QUEUE_GAP_MASK, gap);
}
/*
@func ret_t | rtl8370_getAsicEgressFlowControlQueueDropGap | Get Queue-based egress flow control turn off or ingress flow control drop off gap. 
@parm uint32* | gap | Queue-based flown control/drop turn OFF threshold = turn ON threshold - gap. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get Queue-based egress flow control turn off gap or queue-based egress drop off gap for ingress flow control mechanism. 
 */
ret_t rtl8370_getAsicEgressFlowControlQueueDropGap(uint32 *gap)
{
    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_QUEUE_GAP_REG, RTL8370_FLOWCTRL_QUEUE_GAP_MASK, gap);
}

/*
@func ret_t | rtl8370_getAsicEgressQueueEmptyPortMask | Get queue empty port mask
@parm uint32* | pmsk | Queue empty port mask 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get current output queues are empty port mask 
 */
ret_t rtl8370_getAsicEgressQueueEmptyPortMask(uint32 *pmsk)
{
    return rtl8370_getAsicReg(RTL8370_FLOWCTRL_PORT_QEMPTY_REG, pmsk);
}

/*
@func ret_t | rtl8370_getAsicTotalPage | Get system total page usage number
@parm uint32* | pageCount | System total page usage number
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get system total page usage number 
 */
ret_t rtl8370_getAsicTotalPage(uint32 *pageCount)
{
    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_TOTAL_PAGE_COUNTER_REG, RTL8370_FLOWCTRL_TOTAL_PAGE_COUNTER_COUNTER_MASK, pageCount);
}

/*
@func ret_t | rtl8370_getAsicPulbicPage | Get system public page usage number
@parm uint32* | pageCount | System total page usage number
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get system public page usage number. The public page number = total page-SUM{Port n page exceed port private reserved number} 
 */
ret_t rtl8370_getAsicPulbicPage(uint32 *pageCount)
{
    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_PUBLIC_PAGE_COUNTER_REG, RTL8370_FLOWCTRL_PUBLIC_PAGE_COUNTER_MASK, pageCount);
}
/*
@func ret_t | rtl8370_getAsicMaxTotalPage | Get system total page max usage number
@parm uint32* | pageCount | System total page usage number
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get system total page max usage number 
 */
ret_t rtl8370_getAsicMaxTotalPage(uint32 *pageCount)
{
    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_TOTAL_PAGE_MAX_REG, RTL8370_FLOWCTRL_TOTAL_PAGE_MAX_MASK, pageCount);
}
/*
@func ret_t | rtl8370_getAsicPulbicPage | Get system public page max usage number
@parm uint32* | pageCount | System total page usage number
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get system public page max usage number. The public page number = total page-SUM{Port n page exceed port private reserved number} 
 */
ret_t rtl8370_getAsicMaxPulbicPage(uint32 *pageCount)
{
    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_PUBLIC_PAGE_MAX_REG, RTL8370_FLOWCTRL_PUBLIC_PAGE_MAX_MASK, pageCount);
}
/*
@func ret_t | rtl8370_getAsicPortPage | Get per-port page usage number
@parm uint32 | port | The port number
@parm uint32* | pageCount | System total page usage number
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get per-port page usage number.
 */
ret_t rtl8370_getAsicPortPage(uint32 port, uint32 *pageCount)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_PORT_PAGE_COUNTER_REG(port), RTL8370_FLOWCTRL_PORT_PAGE_COUNTER_MASK, pageCount);
}
/*
@func ret_t | rtl8370_getAsicPortPage | Get per-port page max usage number
@parm uint32 | port | The port number
@parm uint32* | pageCount | System total page usage number
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get per-port page max usage number.
 */
ret_t rtl8370_getAsicPortPageMax(uint32 port, uint32 *pageCount)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_FLOWCTRL_PORT_PAGE_MAX_REG(port), RTL8370_FLOWCTRL_PORT_PAGE_MAX_MASK, pageCount);
}
