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
 * Feature : Qos related functions
 *
 */

#include "rtl8367b_asicdrv_qos.h"
/* Function Name:
 *      rtl8367b_setAsicPriorityDot1qRemapping
 * Description:
 *      Set 802.1Q absolutely priority
 * Input:
 *      srcpriority - Priority value
 *      priority 	- Absolute priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY	- Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPriorityDot1qRemapping(rtk_uint32 srcpriority, rtk_uint32 priority )
{
	if((srcpriority > RTL8367B_PRIMAX) || (priority > RTL8367B_PRIMAX))
		return RT_ERR_QOS_INT_PRIORITY; 

	return rtl8367b_setAsicRegBits(RTL8367B_QOS_1Q_PRIORITY_REMAPPING_REG(srcpriority), RTL8367B_QOS_1Q_PRIORITY_REMAPPING_MASK(srcpriority),priority);		
}
/* Function Name:
 *      rtl8367b_getAsicPriorityDot1qRemapping
 * Description:
 *      Get 802.1Q absolutely priority
 * Input:
 *      srcpriority - Priority value
 *      pPriority 	- Absolute priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPriorityDot1qRemapping(rtk_uint32 srcpriority, rtk_uint32 *pPriority )
{
	if(srcpriority > RTL8367B_PRIMAX )
		return RT_ERR_QOS_INT_PRIORITY; 

	return rtl8367b_getAsicRegBits(RTL8367B_QOS_1Q_PRIORITY_REMAPPING_REG(srcpriority), RTL8367B_QOS_1Q_PRIORITY_REMAPPING_MASK(srcpriority), pPriority);		
}
/* Function Name:
 *      rtl8367b_setAsicPriorityPortBased
 * Description:
 *      Set port based priority
 * Input:
 *      port 		- Physical port number (0~7)
 *      priority 	- Priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  		- Invalid port number
 *      RT_ERR_QOS_INT_PRIORITY	- Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPriorityPortBased(rtk_uint32 port, rtk_uint32 priority )
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID; 

	if(priority > RTL8367B_PRIMAX )
		return RT_ERR_QOS_INT_PRIORITY; 

	return rtl8367b_setAsicRegBits(RTL8367B_QOS_PORTBASED_PRIORITY_REG(port), RTL8367B_QOS_PORTBASED_PRIORITY_MASK(port), priority);		
}
/* Function Name:
 *      rtl8367b_getAsicPriorityPortBased
 * Description:
 *      Get port based priority
 * Input:
 *      port 		- Physical port number (0~7)
 *      pPriority 	- Priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPriorityPortBased(rtk_uint32 port, rtk_uint32 *pPriority )
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID; 

	return rtl8367b_getAsicRegBits(RTL8367B_QOS_PORTBASED_PRIORITY_REG(port), RTL8367B_QOS_PORTBASED_PRIORITY_MASK(port), pPriority);		
}
/* Function Name:
 *      rtl8367b_setAsicPriorityDscpBased
 * Description:
 *      Set DSCP-based priority
 * Input:
 *      dscp 		- DSCP value
 *      priority 	- Priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_DSCP_VALUE	- Invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY	- Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPriorityDscpBased(rtk_uint32 dscp, rtk_uint32 priority )
{
	if(priority > RTL8367B_PRIMAX )
		return RT_ERR_QOS_INT_PRIORITY; 

	if(dscp > RTL8367B_DSCPMAX)
		return RT_ERR_QOS_DSCP_VALUE; 

	return rtl8367b_setAsicRegBits(RTL8367B_QOS_DSCP_TO_PRIORITY_REG(dscp), RTL8367B_QOS_DSCP_TO_PRIORITY_MASK(dscp), priority);		
}
/* Function Name:
 *      rtl8367b_getAsicPriorityDscpBased
 * Description:
 *      Get DSCP-based priority
 * Input:
 *      dscp 		- DSCP value
 *      pPriority 	- Priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY	- Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPriorityDscpBased(rtk_uint32 dscp, rtk_uint32 *pPriority )
{
	if(dscp > RTL8367B_DSCPMAX)
		return RT_ERR_QOS_DSCP_VALUE; 

	return rtl8367b_getAsicRegBits(RTL8367B_QOS_DSCP_TO_PRIORITY_REG(dscp), RTL8367B_QOS_DSCP_TO_PRIORITY_MASK(dscp), pPriority);		
}
/* Function Name:
 *      rtl8367b_setAsicPriorityDecision
 * Description:
 *      Set priority decision table
 * Input:
 *      prisrc 		- Priority decision source 
 *      decisionPri - Decision priority assignment
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY		- Invalid priority
 *      RT_ERR_QOS_SEL_PRI_SOURCE	- Invalid priority decision source parameter
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPriorityDecision(rtk_uint32 prisrc, rtk_uint32 decisionPri)
{
	if(prisrc >= PRIDEC_END ) 
		return RT_ERR_QOS_SEL_PRI_SOURCE;
	
	if(decisionPri > RTL8367B_DECISIONPRIMAX ) 
		return RT_ERR_QOS_INT_PRIORITY;

	return rtl8367b_setAsicRegBits(RTL8367B_QOS_INTERNAL_PRIORITY_DECISION_REG(prisrc), RTL8367B_QOS_INTERNAL_PRIORITY_DECISION_MASK(prisrc), decisionPri);		
}
/* Function Name:
 *      rtl8367b_getAsicPriorityDecision
 * Description:
 *      Get priority decision table
 * Input:
 *      prisrc 		- Priority decision source 
 *      pDecisionPri - Decision priority assignment
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_QOS_SEL_PRI_SOURCE	- Invalid priority decision source parameter
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPriorityDecision(rtk_uint32 prisrc, rtk_uint32* pDecisionPri)
{
	if(prisrc >= PRIDEC_END ) 
		return RT_ERR_QOS_SEL_PRI_SOURCE;

	return rtl8367b_getAsicRegBits(RTL8367B_QOS_INTERNAL_PRIORITY_DECISION_REG(prisrc), RTL8367B_QOS_INTERNAL_PRIORITY_DECISION_MASK(prisrc), pDecisionPri);		
}
/* Function Name:
 *      rtl8367b_setAsicOutputQueueMappingIndex
 * Description:
 *      Set output queue number for each port
 * Input:
 *      port 	- Physical port number (0~7)
 *      index 	- Mapping table index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_QUEUE_NUM  	- Invalid queue number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicOutputQueueMappingIndex(rtk_uint32 port, rtk_uint32 index )
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID; 

	if(index >= RTL8367B_QUEUENO)
		return RT_ERR_QUEUE_NUM; 

	return rtl8367b_setAsicRegBits(RTL8367B_QOS_PORT_QUEUE_NUMBER_REG(port), RTL8367B_QOS_PORT_QUEUE_NUMBER_MASK(port), index);		
}
/* Function Name:
 *      rtl8367b_getAsicOutputQueueMappingIndex
 * Description:
 *      Get output queue number for each port
 * Input:
 *      port 	- Physical port number (0~7)
 *      pIndex 	- Mapping table index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicOutputQueueMappingIndex(rtk_uint32 port, rtk_uint32 *pIndex )
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID; 
	
	return rtl8367b_getAsicRegBits(RTL8367B_QOS_PORT_QUEUE_NUMBER_REG(port), RTL8367B_QOS_PORT_QUEUE_NUMBER_MASK(port), pIndex);		
}
/* Function Name:
 *      rtl8367b_setAsicPriorityToQIDMappingTable
 * Description:
 *      Set priority to QID mapping table parameters
 * Input:
 *      index 		- Mapping table index
 *      priority 	- The priority value
 *      qid 		- Queue id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QUEUE_ID  		- Invalid queue id
 *      RT_ERR_QUEUE_NUM  		- Invalid queue number
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPriorityToQIDMappingTable(rtk_uint32 index, rtk_uint32 priority, rtk_uint32 qid )
{
	if(index >= RTL8367B_QUEUENO)
		return RT_ERR_QUEUE_NUM; 

	if(priority > RTL8367B_PRIMAX)
		return RT_ERR_QOS_INT_PRIORITY; 

	if(qid > RTL8367B_QIDMAX) 
		return RT_ERR_QUEUE_ID;

	return rtl8367b_setAsicRegBits(RTL8367B_QOS_1Q_PRIORITY_TO_QID_REG(index, priority), RTL8367B_QOS_1Q_PRIORITY_TO_QID_MASK(priority), qid);		
}
/* Function Name:
 *      rtl8367b_getAsicPriorityToQIDMappingTable
 * Description:
 *      Get priority to QID mapping table parameters
 * Input:
 *      index 		- Mapping table index
 *      priority 	- The priority value
 *      pQid 		- Queue id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QUEUE_NUM  		- Invalid queue number
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPriorityToQIDMappingTable(rtk_uint32 index, rtk_uint32 priority, rtk_uint32* pQid)
{
	if(index >= RTL8367B_QUEUENO)
		return RT_ERR_QUEUE_NUM; 

	if(priority > RTL8367B_PRIMAX)
		return RT_ERR_QOS_INT_PRIORITY; 

	return rtl8367b_getAsicRegBits(RTL8367B_QOS_1Q_PRIORITY_TO_QID_REG(index, priority), RTL8367B_QOS_1Q_PRIORITY_TO_QID_MASK(priority), pQid);		
}
/* Function Name:
 *      rtl8367b_setAsicRemarkingDot1pAbility
 * Description:
 *      Set 802.1p remarking ability
 * Input:
 *      port 	- Physical port number (0~7)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRemarkingDot1pAbility(rtk_uint32 port, rtk_uint32 enabled)
{
	return rtl8367b_setAsicRegBit(RTL8367B_PORT_MISC_CFG_REG(port), RTL8367B_1QREMARK_ENABLE_OFFSET, enabled);	
}
/* Function Name:
 *      rtl8367b_getAsicRemarkingDot1pAbility
 * Description:
 *      Get 802.1p remarking ability
 * Input:
 *      port 	- Physical port number (0~7)
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRemarkingDot1pAbility(rtk_uint32 port, rtk_uint32* pEnabled)
{
	return rtl8367b_getAsicRegBit(RTL8367B_PORT_MISC_CFG_REG(port), RTL8367B_1QREMARK_ENABLE_OFFSET, pEnabled);	
}
/* Function Name:
 *      rtl8367b_setAsicRemarkingDot1pParameter
 * Description:
 *      Set 802.1p remarking parameter
 * Input:
 *      priority 	- Priority value
 *      newPriority - New priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRemarkingDot1pParameter(rtk_uint32 priority, rtk_uint32 newPriority )
{
	if(priority > RTL8367B_PRIMAX || newPriority > RTL8367B_PRIMAX)
		return RT_ERR_QOS_INT_PRIORITY; 

	return rtl8367b_setAsicRegBits(RTL8367B_QOS_1Q_REMARK_REG(priority), RTL8367B_QOS_1Q_REMARK_MASK(priority), newPriority);		
}
/* Function Name:
 *      rtl8367b_getAsicRemarkingDot1pParameter
 * Description:
 *      Get 802.1p remarking parameter
 * Input:
 *      priority 	- Priority value
 *      pNewPriority - New priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRemarkingDot1pParameter(rtk_uint32 priority, rtk_uint32 *pNewPriority )
{
	if(priority > RTL8367B_PRIMAX )
		return RT_ERR_QOS_INT_PRIORITY; 

	return rtl8367b_getAsicRegBits(RTL8367B_QOS_1Q_REMARK_REG(priority), RTL8367B_QOS_1Q_REMARK_MASK(priority), pNewPriority);		
}
/* Function Name:
 *      rtl8367b_setAsicRemarkingDscpAbility
 * Description:
 *      Set DSCP remarking ability
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRemarkingDscpAbility(rtk_uint32 enabled)
{
	return rtl8367b_setAsicRegBit(RTL8367B_REMARKING_CTRL_REG, RTL8367B_REMARKING_DSCP_ENABLE_OFFSET, enabled);	
}
/* Function Name:
 *      rtl8367b_getAsicRemarkingDscpAbility
 * Description:
 *      Get DSCP remarking ability
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRemarkingDscpAbility(rtk_uint32* pEnabled)
{
	return rtl8367b_getAsicRegBit(RTL8367B_REMARKING_CTRL_REG, RTL8367B_REMARKING_DSCP_ENABLE_OFFSET, pEnabled);	
}
/* Function Name:
 *      rtl8367b_setAsicRemarkingDscpParameter
 * Description:
 *      Set DSCP remarking parameter
 * Input:
 *      priority 	- Priority value
 *      newDscp 	- New DSCP value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_DSCP_VALUE	- Invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY	- Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRemarkingDscpParameter(rtk_uint32 priority, rtk_uint32 newDscp )
{
	if(priority > RTL8367B_PRIMAX )
		return RT_ERR_QOS_INT_PRIORITY; 

	if(newDscp > RTL8367B_DSCPMAX)
		return RT_ERR_QOS_DSCP_VALUE; 

	return rtl8367b_setAsicRegBits(RTL8367B_QOS_DSCP_REMARK_REG(priority), RTL8367B_QOS_DSCP_REMARK_MASK(priority), newDscp);
}
/* Function Name:
 *      rtl8367b_getAsicRemarkingDscpParameter
 * Description:
 *      Get DSCP remarking parameter
 * Input:
 *      priority 	- Priority value
 *      pNewDscp 	- New DSCP value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY	- Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRemarkingDscpParameter(rtk_uint32 priority, rtk_uint32* pNewDscp )
{
	if(priority > RTL8367B_PRIMAX )
		return RT_ERR_QOS_INT_PRIORITY; 

	return rtl8367b_getAsicRegBits(RTL8367B_QOS_DSCP_REMARK_REG(priority), RTL8367B_QOS_DSCP_REMARK_MASK(priority), pNewDscp);		
}

