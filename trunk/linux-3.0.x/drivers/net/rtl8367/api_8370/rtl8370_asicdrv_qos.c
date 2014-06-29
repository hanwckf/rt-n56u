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
 * $Date: 2010/12/02 04:34:23 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_qos.h"

/*
@func ret_t | rtl8370_setAsicPriorityDot1qRemapping | Set 802.1Q absolutely priority.
@parm uint32 | srcpriority | Priority value.
@parm uint32 | priority | Absolute priority value.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_VLAN_PRIORITY | Invalid priority.
@comm
    The API can set a 3-bit absolutely priority of the specified 802.1Q priority. 
 */
ret_t rtl8370_setAsicPriorityDot1qRemapping( uint32 srcpriority, uint32 priority )
{
    uint16 regAddr;
    uint32 regBits;
    
    if((srcpriority > RTL8370_PRIMAX) || (priority > RTL8370_PRIMAX))
        return RT_ERR_VLAN_PRIORITY; 

    /* Set Related Registers */
    regAddr = RTL8370_QOS_1Q_PRIORITY_REMAPPING_REG(srcpriority);

    regBits = RTL8370_QOS_1Q_PRIORITY_REMAPPING_MASK(srcpriority);

    return rtl8370_setAsicRegBits(regAddr, regBits, priority);        
}

/*
@func ret_t | rtl8370_getAsicPriorityDot1qRemapping | Get 802.1Q absolutely priority. 
@parm uint32 | srcpriority | Priority value. 
@parm uint32* | priority | It will return the absolute priority value.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_VLAN_PRIORITY | Invalid priority. 
@comm
    The API will return the absolutely priority value of the specified 802.1Q priority. 
 */
ret_t rtl8370_getAsicPriorityDot1qRemapping( uint32 srcpriority, uint32 *priority )
{
    uint16 regAddr;
    uint32 regBits;
    
    /* Invalid input parameter */
    if(srcpriority > RTL8370_PRIMAX)
        return RT_ERR_VLAN_PRIORITY; 

    /* Get Related Registers */
    regAddr = RTL8370_QOS_1Q_PRIORITY_REMAPPING_REG(srcpriority);

    regBits = RTL8370_QOS_1Q_PRIORITY_REMAPPING_MASK(srcpriority);

    return rtl8370_getAsicRegBits(regAddr, regBits, priority);        
}

/*
@func ret_t | rtl8370_setAsicPriorityPortBased | Set port based priority.
@parm uint32 | port | The port number.
@parm uint32 | priority | Priority value.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number. 
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority. 
@comm
    The API can set a 3-bit priority value of the specified port. 
 */
ret_t rtl8370_setAsicPriorityPortBased( uint32 port, uint32 priority )
{
    uint16 regAddr;
    uint32 regBits;

    /* Invalid input parameter */
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID; 

    if(priority > RTL8370_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY; 

    /* Set Related Registers */
    regAddr = RTL8370_QOS_PORTBASED_PRIORITY_REG(port);

    regBits = RTL8370_QOS_PORTBASED_PRIORITY_MASK(port);

    return rtl8370_setAsicRegBits(regAddr, regBits, priority);        
}

/*
@func ret_t | rtl8370_getAsicPriorityPortBased | Get port based priority. 
@parm uint32 | port | The port number. 
@parm uint32* | pPriority | Priority value.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number. 
@comm
    The API will return the priority value of the specified port. 
 */
ret_t rtl8370_getAsicPriorityPortBased( uint32 port, uint32 *priority )
{
    uint16 regAddr;
    uint32 regBits;
    
    /* Invalid input parameter */
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID; 

    /* Get Related Registers */
    regAddr = RTL8370_QOS_PORTBASED_PRIORITY_REG(port);

    regBits = RTL8370_QOS_PORTBASED_PRIORITY_MASK(port);

    return rtl8370_getAsicRegBits(regAddr, regBits, priority);        
}

/*
@func ret_t | rtl8370_setAsicPriorityDscpBased | Set DSCP-based priority.
@parm uint32 | dscp | DSCP value.
@parm uint32 | priority | Priority value.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_QOS_DSCP_VALUE | Invalid DSCP value. 
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority. 
@comm
    The API can set a 3-bit priority of the specified DSCP value. 
 */
ret_t rtl8370_setAsicPriorityDscpBased( uint32 dscp, uint32 priority )
{
    uint16 regAddr;
    uint32 regBits;

    /* Invalid input parameter */
    if(priority > RTL8370_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY; 

    if(dscp > RTL8370_DSCPMAX)
        return RT_ERR_QOS_DSCP_VALUE; 

    /* Set Related Registers */
    regAddr = RTL8370_QOS_DSCP_TO_PRIORITY_REG(dscp);

    regBits = RTL8370_QOS_DSCP_TO_PRIORITY_MASK(dscp);

    return rtl8370_setAsicRegBits(regAddr, regBits, priority);        
}

/*
@func ret_t | rtl8370_getAsicPriorityDscpBased | Get DSCP-based priority. 
@parm uint32 | dscp | DSCP value.
@parm uint32* | priority | It will return the priority of the specified DSCP.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_QOS_DSCP_VALUE | Invalid DSCP value. 
@comm
    The API can get the priority of the specified DSCP value. 
 */
ret_t rtl8370_getAsicPriorityDscpBased( uint32 dscp, uint32 *priority )
{
    uint16 regAddr;
    uint32 regBits;

    /* Invalid input parameter */
    if(dscp > RTL8370_DSCPMAX)
        return RT_ERR_QOS_DSCP_VALUE; 

    /* Get Related Registers */
    regAddr = RTL8370_QOS_DSCP_TO_PRIORITY_REG(dscp);
    
    regBits = RTL8370_QOS_DSCP_TO_PRIORITY_MASK(dscp);

    return rtl8370_getAsicRegBits(regAddr, regBits, priority);        
}

/*=======================================================================
  * ASIC DRIVER API: Priority Assignment Control Register 
 *========================================================================*/
/*
@func ret_t | rtl8370_setAsicPriorityDecision | Set priority decision table. 
@parm enum PRIDECISION | prisrc | Priority decision source 
@parm uint32 | decisionpri | Decision priority weight assignment.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_QOS_SEL_PRI_SOURCE | Invalid priority decision source parameter.
@comm
    The API can set the priorities of Port-based, ACL-based,1Q-based, DSCP-based, SVLAN-based, 
    CVLAN-based and DA/SA based prioriity assignments in output queue priority decision table. 
 */
ret_t rtl8370_setAsicPriorityDecision( enum PRIDECISION prisrc, uint32 decisionpri)
{
    /* Invalid input parameter */
    if(prisrc >= PRIDEC_MAX ) 
        return RT_ERR_QOS_SEL_PRI_SOURCE;
    
    if(decisionpri >RTL8370_PRIDECMAX) 
        return RT_ERR_QOS_INT_PRIORITY;

    /* Set Related Registers */
    return rtl8370_setAsicRegBits(RTL8370_QOS_INTERNAL_PRIORITY_DECISION_REG(prisrc), RTL8370_QOS_INTERNAL_PRIORITY_DECISION_MASK(prisrc),decisionpri);        
}


/*
@func ret_t | rtl8370_getAsicPriorityDecision | Get priority decision table.
@parm enum PRIDECISION | prisrc | Priority decision source 
@parm uint32* | decisionpri | Decision priority assignment.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_QOS_SEL_PRI_SOURCE | Invalid priority decision source parameter.
@comm
    The API can get the priorities of Port-based, ACL-based,1Q-based, DSCP-based, SVLAN-based, 
    CVLAN-based and DA/SA based prioriity assignments in output queue priority decision table. 
 */
ret_t rtl8370_getAsicPriorityDecision( enum PRIDECISION prisrc, uint32* decisionpri)
{
    /* Invalid input parameter */
    if(prisrc >= PRIDEC_MAX ) 
        return RT_ERR_QOS_SEL_PRI_SOURCE;

    /* Get Related Registers */
    return rtl8370_getAsicRegBits(RTL8370_QOS_INTERNAL_PRIORITY_DECISION_REG(prisrc), RTL8370_QOS_INTERNAL_PRIORITY_DECISION_MASK(prisrc),decisionpri);        
}

/*
@func ret_t | rtl8370_setAsicOutputQueueMappingIndex | Set output queue number for each port.
@parm uint32 | port | The port number. 
@parm uint32 | index | Mapping table index
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_QUEUE_NUM | Invalid queue number.
@comm
    The API can set the output queue number of the specified port. 
 */
ret_t rtl8370_setAsicOutputQueueMappingIndex(uint32 port, uint32 index )
{
    uint16 regAddr;
    uint32 regBits;

    /* Invalid input parameter */
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID; 

    if(index >= RTL8370_QUEUENO)
        return RT_ERR_QUEUE_NUM; 


    /* Set Related Registers */
    regAddr = RTL8370_QOS_PORT_QUEUE_NUMBER_REG(port);

    regBits = RTL8370_QOS_PORT_QUEUE_NUMBER_MASK(port);

    return rtl8370_setAsicRegBits(regAddr, regBits, index);        
}

/*
@func ret_t | rtl8370_getAsicOutputQueueMappingIndex | Get output queue number.
@parm uint32 | port | The port number. 
@parm uint32* | index | Mapping table index
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API will return the output queue number of the specified port. 
 */
ret_t rtl8370_getAsicOutputQueueMappingIndex( uint32 port, uint32 *index )
{
    uint16 regAddr;
    uint32 regBits;
    
    /* Invalid input parameter */
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID; 

    /* Get Related Registers */
    regAddr = RTL8370_QOS_PORT_QUEUE_NUMBER_REG(port);

    regBits = RTL8370_QOS_PORT_QUEUE_NUMBER_MASK(port);

    return rtl8370_getAsicRegBits(regAddr,regBits,index);        
}

/*
@func ret_t | rtl8370_setAsicPriorityToQIDMappingTable | Set priority to QID mapping table parameters.
@parm uint32 | index | Mapping table index
@parm uint32 | priority | The priority value. 
@parm uint32 | qid | Queue id.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_QUEUE_NUM | Invalid queue number. 
@rvalue RT_ERR_QUEUE_ID | Invalid queue id. 
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority.
@comm
    The API can configure priority to queue id mapping table in different queue number. 
 */
ret_t rtl8370_setAsicPriorityToQIDMappingTable( uint32 index, uint32 priority, uint32 qid )
{
    uint16 regAddr;
    uint32 regBits;

    /* Invalid input parameter */
    if(index >= RTL8370_QUEUENO)
        return RT_ERR_QUEUE_NUM; 

    if(priority > RTL8370_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY; 

    if(qid > RTL8370_QIDMAX) 
        return RT_ERR_QUEUE_ID;

    /* Set Related Registers */
    regAddr = RTL8370_QOS_1Q_PRIORITY_TO_QID_REG(index, priority);

    regBits = RTL8370_QOS_1Q_PRIORITY_TO_QID_MASK(priority);

    return rtl8370_setAsicRegBits(regAddr, regBits, qid);        
}

/*
@func ret_t | rtl8370_getAsicPriorityToQIDMappingTable | Get priority to QID mapping table parameters. 
@parm uint32 | index | Mapping table index
@parm uint32 | priority | The priority value
@parm uint32* | qid | Queue id. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_QUEUE_NUM | Invalid queue number.
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority.
@comm
    The API can return the mapping queue id of the specifed priority and queue number. 
 */
ret_t rtl8370_getAsicPriorityToQIDMappingTable(uint32 index, uint32 priority, uint32* qid)
{
    uint16 regAddr;
    uint32 regBits;

    /* Invalid input parameter */
    if(index >= RTL8370_QUEUENO)
        return RT_ERR_QUEUE_NUM; 

    if(priority > RTL8370_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY; 

    /* Get Related Registers */
    regAddr = RTL8370_QOS_1Q_PRIORITY_TO_QID_REG(index, priority);

    regBits = RTL8370_QOS_1Q_PRIORITY_TO_QID_MASK(priority);

    return rtl8370_getAsicRegBits(regAddr, regBits, qid);        
}

/*=======================================================================
  * ASIC DRIVER API: Remarking Control Register 
 *========================================================================*/

/*
@func ret_t | rtl8370_setAsicRemarkingDot1pAbility | Set 802.1p remarking ability. 
@parm uint32 | enabledd | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable parameter.
@comm
    The API can enable or disable 802.1p remarking ability for whole system. 
 */
ret_t rtl8370_setAsicRemarkingDot1pAbility(uint32 enabled)
{
    /* Invalid input parameter */
    if ((enabled != 0) && (enabled != 1))
        return RT_ERR_ENABLE; 

    return rtl8370_setAsicRegBit(RTL8370_REMARKING_CTRL_REG, RTL8370_REMARKING_1Q_ENABLE_OFFSET, enabled);    
}

/*
@func ret_t | rtl8370_getAsicRemarkingDot1pAbility | Get 802.1p remarking ability. 
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get 802.1p remarking ability.
 */
ret_t rtl8370_getAsicRemarkingDot1pAbility(uint32* enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_REMARKING_CTRL_REG, RTL8370_REMARKING_1Q_ENABLE_OFFSET, enabled);    
}

/*
@func ret_t | rtl8370_setAsicRemarkingDot1pParameter | Set 802.1p remarking parameter.
@parm uint32 | priority | Priority value.
@parm uint32 | newpriority | New priority value.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_VLAN_PRIORITY | Invalid priority. 
@comm
    The API can set 802.1p parameters source priority and new priority.
 */
ret_t rtl8370_setAsicRemarkingDot1pParameter( uint32 priority, uint32 newpriority )
{
    uint16 regAddr;
    uint32 regBits;

    /* Invalid input parameter */
    if((priority > RTL8370_PRIMAX) || (newpriority > RTL8370_PRIMAX))
        return RT_ERR_VLAN_PRIORITY; 

    /* Set Related Registers */
    regAddr = RTL8370_QOS_1Q_REMARK_REG(priority);

    regBits = RTL8370_QOS_1Q_REMARK_MASK(priority);

    return rtl8370_setAsicRegBits(regAddr,regBits,newpriority);        
}

/*
@func ret_t | rtl8370_getAsicRemarkingDot1pParameter | Get 802.1p remarking parameter.
@parm uint32 | priority | Priority value.
@parm uint32 *| newpriority | It will return the new priority value of a specified priority.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_VLAN_PRIORITY | Invalid priority. 
@comm
    The API can get 802.1p remarking parameters. It would return new priority of inputed priority. 
 */
ret_t rtl8370_getAsicRemarkingDot1pParameter( uint32 priority, uint32 *newpriority )
{
    uint16 regAddr;
    uint32 regBits;

    /* Invalid input parameter */
    if(priority > RTL8370_PRIMAX )
        return RT_ERR_VLAN_PRIORITY; 

    /* Get Related Registers */
    regAddr = RTL8370_QOS_1Q_REMARK_REG(priority);

    regBits = RTL8370_QOS_1Q_REMARK_MASK(priority);

    return rtl8370_getAsicRegBits(regAddr,regBits,newpriority);        
}

/*
@func ret_t | rtl8370_setAsicRemarkingDscpAbility | Set DSCP remarking ability.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable parameter.
@comm
    The API can enable or disable DSCP remarking ability for whole system. 
 */
ret_t rtl8370_setAsicRemarkingDscpAbility(uint32 enabled)
{
    /* Invalid input parameter */
    if((enabled != 0) && (enabled != 1))
        return RT_ERR_ENABLE; 

    return rtl8370_setAsicRegBit(RTL8370_REMARKING_CTRL_REG, RTL8370_REMARKING_DSCP_ENABLE_OFFSET, enabled);    
}

/*
@func ret_t | rtl8370_getAsicRemarkingDscpAbility | Get DSCP remarking ability.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get DSCP remarking ability of whole system.
 */
ret_t rtl8370_getAsicRemarkingDscpAbility( uint32* enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_REMARKING_CTRL_REG, RTL8370_REMARKING_DSCP_ENABLE_OFFSET, enabled);    
}

/*
@func ret_t | rtl8370_setAsicRemarkingDscpParameter | Set DSCP remarking parameter.
@parm uint32 | priority | Priority value.
@parm uint32 | newdscp | New DSCP value.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority. 
@rvalue RT_ERR_QOS_DSCP_VALUE | Invalid DSCP value. 
@comm
    The API can set DSCP parameters source priority and new priority.
 */
ret_t rtl8370_setAsicRemarkingDscpParameter( uint32 priority, uint32 newdscp )
{
    uint16 regAddr;
    uint32 regBits;

    /* Invalid input parameter */
    if(priority > RTL8370_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY; 

    if(newdscp > RTL8370_DSCPMAX)
        return RT_ERR_QOS_DSCP_VALUE; 

    /* Set Related Registers */
    regAddr = RTL8370_QOS_DSCP_REMARK_REG(priority);

    regBits = RTL8370_QOS_DSCP_REMARK_MASK(priority);

    return rtl8370_setAsicRegBits(regAddr,regBits,newdscp);
}

/*
@func ret_t | rtl8370_getAsicRemarkingDscpParameter | Get DSCP remarking parameter.
@parm enum PRIORITYVALUE | priority | Priority value.
@parm uint32* | newdscp |new DSCP value.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority.
@comm
    The API can get DSCP parameters. It would return new DSCP value of the specified priority.
 */
ret_t rtl8370_getAsicRemarkingDscpParameter( uint32 priority, uint32* newdscp )
{
    uint16 regAddr;
    uint32 regBits;

    /* Invalid input parameter */
    if(priority > RTL8370_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY; 

    /* Get Related Registers */
    regAddr = RTL8370_QOS_DSCP_REMARK_REG(priority);

    regBits = RTL8370_QOS_DSCP_REMARK_MASK(priority);
    
    return rtl8370_getAsicRegBits(regAddr,regBits,newdscp);        
}

