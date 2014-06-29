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
 * Feature : Flow control related functions
 *
 */

#include "rtl8367b_asicdrv_fc.h"
/* Function Name:
 *      rtl8367b_setAsicFlowControlSelect
 * Description:
 *      Set system flow control type
 * Input:
 *      select 		- System flow control type 1: Ingress flow control 0:Egress flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlSelect(rtk_uint32 select)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_FLOWCTRL_CTRL0, RTL8367B_FLOWCTRL_TYPE_OFFSET, select);
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlSelect
 * Description:
 *      Get system flow control type
 * Input:
 *      pSelect 		- System flow control type 1: Ingress flow control 0:Egress flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlSelect(rtk_uint32 *pSelect)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_FLOWCTRL_CTRL0, RTL8367B_FLOWCTRL_TYPE_OFFSET, pSelect);
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlJumboMode
 * Description:
 *      Set Jumbo threhsold for flow control
 * Input:
 *      enabled 		- Jumbo mode flow control 1: Enable 0:Disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlJumboMode(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_FLOWCTRL_JUMBO_SIZE, RTL8367B_JUMBO_MODE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlJumboMode
 * Description:
 *      Get Jumbo threhsold for flow control
 * Input:
 *      pEnabled 		- Jumbo mode flow control 1: Enable 0:Disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlJumboMode(rtk_uint32* pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_FLOWCTRL_JUMBO_SIZE, RTL8367B_JUMBO_MODE_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlJumboModeSize
 * Description:
 *      Set Jumbo size for Jumbo mode flow control
 * Input:
 *      size 		- Jumbo size 0:3Kbytes 1:4Kbytes 2:6Kbytes 3:9Kbytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI 			- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlJumboModeSize(rtk_uint32 size)
{
	if(size >= FC_JUMBO_SIZE_END)
		return RT_ERR_OUT_OF_RANGE;
	
    return rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_SIZE, RTL8367B_JUMBO_SIZE_MASK, size);
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlJumboModeSize
 * Description:
 *      Get Jumbo size for Jumbo mode flow control
 * Input:
 *      pSize 		- Jumbo size 0:3Kbytes 1:4Kbytes 2:6Kbytes 3:9Kbytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlJumboModeSize(rtk_uint32* pSize)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_SIZE, RTL8367B_JUMBO_SIZE_MASK, pSize);
}

/* Function Name:
 *      rtl8367b_setAsicFlowControlQueueEgressEnable
 * Description:
 *      Set flow control ability for each queue
 * Input:
 *      port 	- Physical port number (0~7)
 *      qid 	- Queue id
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 *      RT_ERR_QUEUE_ID - Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlQueueEgressEnable(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 enabled)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8367B_QIDMAX) 
		return RT_ERR_QUEUE_ID;
    
    return rtl8367b_setAsicRegBit(RTL8367B_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG(port), RTL8367B_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG_OFFSET(port)+ qid, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlQueueEgressEnable
 * Description:
 *      Get flow control ability for each queue
 * Input:
 *      port 	- Physical port number (0~7)
 *      qid 	- Queue id
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 *      RT_ERR_QUEUE_ID - Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlQueueEgressEnable(rtk_uint32 port, rtk_uint32 qid, rtk_uint32* pEnabled)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8367B_QIDMAX) 
		return RT_ERR_QUEUE_ID;
    
    return  rtl8367b_getAsicRegBit(RTL8367B_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG(port), RTL8367B_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG_OFFSET(port)+ qid, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlDropAll
 * Description:
 *      Set system-based drop parameters
 * Input:
 *      dropall 	- Whole system drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlDropAll(rtk_uint32 dropall)
{
	if(dropall >= RTL8367B_PAGE_NUMBER)
   		return RT_ERR_OUT_OF_RANGE;
	 
	return rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_CTRL0, RTL8367B_DROP_ALL_THRESHOLD_MASK, dropall);
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlDropAll
 * Description:
 *      Get system-based drop parameters
 * Input:
 *      pDropall 	- Whole system drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlDropAll(rtk_uint32* pDropall)
{
	return rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_CTRL0, RTL8367B_DROP_ALL_THRESHOLD_MASK, pDropall);
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlPauseAll
 * Description:
 *      Set system-based all ports enable flow control parameters
 * Input:
 *      threshold 	- Whole system pause all threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlPauseAllThreshold(rtk_uint32 threshold)
{
    if(threshold >= RTL8367B_PAGE_NUMBER)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_ALL_ON, RTL8367B_FLOWCTRL_ALL_ON_THRESHOLD_MASK, threshold);
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlPauseAllThreshold
 * Description:
 *      Get system-based all ports enable flow control parameters
 * Input:
 *      pThreshold 	- Whole system pause all threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlPauseAllThreshold(rtk_uint32 *pThreshold)
{    
    return rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_ALL_ON, RTL8367B_FLOWCTRL_ALL_ON_THRESHOLD_MASK, pThreshold);
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlSystemThreshold
 * Description:
 *      Set system-based flow control parameters
 * Input:
 *      onThreshold 	- Flow control turn ON threshold
 *      offThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlSystemThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_SYS_OFF, RTL8367B_FLOWCTRL_SYS_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_SYS_ON, RTL8367B_FLOWCTRL_SYS_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlSystemThreshold
 * Description:
 *      Get system-based flow control parameters
 * Input:
 *      pOnThreshold 	- Flow control turn ON threshold
 *      pOffThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlSystemThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_SYS_OFF, RTL8367B_FLOWCTRL_SYS_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_SYS_ON, RTL8367B_FLOWCTRL_SYS_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlSharedThreshold
 * Description:
 *      Set share-based flow control parameters
 * Input:
 *      onThreshold 	- Flow control turn ON threshold
 *      offThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlSharedThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_SHARE_OFF, RTL8367B_FLOWCTRL_SHARE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_SHARE_ON, RTL8367B_FLOWCTRL_SHARE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlSharedThreshold
 * Description:
 *      Get share-based flow control parameters
 * Input:
 *      pOnThreshold 	- Flow control turn ON threshold
 *      pOffThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlSharedThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_SHARE_OFF, RTL8367B_FLOWCTRL_SHARE_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_SHARE_ON, RTL8367B_FLOWCTRL_SHARE_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlPortThreshold
 * Description:
 *      Set Port-based flow control parameters
 * Input:
 *      onThreshold 	- Flow control turn ON threshold
 *      offThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlPortThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_PORT_OFF, RTL8367B_FLOWCTRL_PORT_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_PORT_ON, RTL8367B_FLOWCTRL_PORT_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlPortThreshold
 * Description:
 *      Get Port-based flow control parameters
 * Input:
 *      pOnThreshold 	- Flow control turn ON threshold
 *      pOffThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlPortThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_PORT_OFF, RTL8367B_FLOWCTRL_PORT_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_PORT_ON, RTL8367B_FLOWCTRL_PORT_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlPortPrivateThreshold
 * Description:
 *      Set Port-private-based flow control parameters
 * Input:
 *      onThreshold 	- Flow control turn ON threshold
 *      offThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlPortPrivateThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_PORT_PRIVATE_OFF, RTL8367B_FLOWCTRL_PORT_PRIVATE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_PORT_PRIVATE_ON, RTL8367B_FLOWCTRL_PORT_PRIVATE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlPortPrivateThreshold
 * Description:
 *      Get Port-private-based flow control parameters
 * Input:
 *      pOnThreshold 	- Flow control turn ON threshold
 *      pOffThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlPortPrivateThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_PORT_PRIVATE_OFF, RTL8367B_FLOWCTRL_PORT_PRIVATE_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_PORT_PRIVATE_ON, RTL8367B_FLOWCTRL_PORT_PRIVATE_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlSystemDropThreshold
 * Description:
 *      Set system-based drop parameters
 * Input:
 *      onThreshold 	- Drop turn ON threshold
 *      offThreshold 	- Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlSystemDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_SYS_OFF, RTL8367B_FLOWCTRL_FCOFF_SYS_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_SYS_ON, RTL8367B_FLOWCTRL_FCOFF_SYS_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlSystemDropThreshold
 * Description:
 *      Get system-based drop parameters
 * Input:
 *      pOnThreshold 	- Drop turn ON threshold
 *      pOffThreshold 	- Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlSystemDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_SYS_OFF, RTL8367B_FLOWCTRL_FCOFF_SYS_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_SYS_ON, RTL8367B_FLOWCTRL_FCOFF_SYS_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlSharedDropThreshold
 * Description:
 *      Set share-based fdrop parameters
 * Input:
 *      onThreshold 	- Drop turn ON threshold
 *      offThreshold 	- Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlSharedDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_SHARE_OFF, RTL8367B_FLOWCTRL_FCOFF_SHARE_OFF_MASK, offThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_SHARE_ON, RTL8367B_FLOWCTRL_FCOFF_SHARE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlSharedDropThreshold
 * Description:
 *      Get share-based fdrop parameters
 * Input:
 *      pOnThreshold 	- Drop turn ON threshold
 *      pOffThreshold 	- Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlSharedDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_SHARE_OFF, RTL8367B_FLOWCTRL_FCOFF_SHARE_OFF_MASK, pOffThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_SHARE_ON, RTL8367B_FLOWCTRL_FCOFF_SHARE_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlPortDropThreshold
 * Description:
 *      Set Port-based drop parameters
 * Input:
 *      onThreshold 	- Drop turn ON threshold
 *      offThreshold 	- Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlPortDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_PORT_OFF, RTL8367B_FLOWCTRL_FCOFF_PORT_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_PORT_ON, RTL8367B_FLOWCTRL_FCOFF_PORT_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlPortDropThreshold
 * Description:
 *      Get Port-based drop parameters
 * Input:
 *      pOnThreshold 	- Drop turn ON threshold
 *      pOffThreshold 	- Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlPortDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_PORT_OFF, RTL8367B_FLOWCTRL_FCOFF_PORT_OFF_MASK, pOffThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_PORT_ON, RTL8367B_FLOWCTRL_FCOFF_PORT_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlPortPrivateDropThreshold
 * Description:
 *      Set Port-private-based drop parameters
 * Input:
 *      onThreshold 	- Drop turn ON threshold
 *      offThreshold 	- Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlPortPrivateDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF, RTL8367B_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_PORT_PRIVATE_ON, RTL8367B_FLOWCTRL_FCOFF_PORT_PRIVATE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlPortPrivateDropThreshold
 * Description:
 *      Get Port-private-based drop parameters
 * Input:
 *      pOnThreshold 	- Drop turn ON threshold
 *      pOffThreshold 	- Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlPortPrivateDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF, RTL8367B_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF_MASK, pOffThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_FCOFF_PORT_PRIVATE_ON, RTL8367B_FLOWCTRL_FCOFF_PORT_PRIVATE_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlSystemJumboThreshold
 * Description:
 *      Set Jumbo system-based flow control parameters
 * Input:
 *      onThreshold 	- Flow control turn ON threshold
 *      offThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlSystemJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_SYS_OFF, RTL8367B_FLOWCTRL_JUMBO_SYS_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_SYS_ON, RTL8367B_FLOWCTRL_JUMBO_SYS_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlSystemJumboThreshold
 * Description:
 *      Get Jumbo system-based flow control parameters
 * Input:
 *      pOnThreshold 	- Flow control turn ON threshold
 *      pOffThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlSystemJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_SYS_OFF, RTL8367B_FLOWCTRL_JUMBO_SYS_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_SYS_ON, RTL8367B_FLOWCTRL_JUMBO_SYS_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlSharedJumboThreshold
 * Description:
 *      Set Jumbo share-based flow control parameters
 * Input:
 *      onThreshold 	- Flow control turn ON threshold
 *      offThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlSharedJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_SHARE_OFF, RTL8367B_FLOWCTRL_JUMBO_SHARE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_SHARE_ON, RTL8367B_FLOWCTRL_JUMBO_SHARE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlSharedJumboThreshold
 * Description:
 *      Get Jumbo share-based flow control parameters
 * Input:
 *      pOnThreshold 	- Flow control turn ON threshold
 *      pOffThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlSharedJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_SHARE_OFF, RTL8367B_FLOWCTRL_JUMBO_SHARE_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_SHARE_ON, RTL8367B_FLOWCTRL_JUMBO_SHARE_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlPortJumboThreshold
 * Description:
 *      Set Jumbo Port-based flow control parameters
 * Input:
 *      onThreshold 	- Flow control turn ON threshold
 *      offThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlPortJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_PORT_OFF, RTL8367B_FLOWCTRL_JUMBO_PORT_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_PORT_ON, RTL8367B_FLOWCTRL_JUMBO_PORT_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlPortJumboThreshold
 * Description:
 *      Get Jumbo Port-based flow control parameters
 * Input:
 *      pOnThreshold 	- Flow control turn ON threshold
 *      pOffThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlPortJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_PORT_OFF, RTL8367B_FLOWCTRL_JUMBO_PORT_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_PORT_ON, RTL8367B_FLOWCTRL_JUMBO_PORT_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicFlowControlPortPrivateJumboThreshold
 * Description:
 *      Set Jumbo Port-private-based flow control parameters
 * Input:
 *      onThreshold 	- Flow control turn ON threshold
 *      offThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlPortPrivateJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;
 
    if((onThreshold >= RTL8367B_PAGE_NUMBER) || (offThreshold >= RTL8367B_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_PORT_PRIVATE_OFF, RTL8367B_FLOWCTRL_JUMBO_PORT_PRIVATE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_PORT_PRIVATE_ON, RTL8367B_FLOWCTRL_JUMBO_PORT_PRIVATE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicFlowControlPortPrivateJumboThreshold
 * Description:
 *      Get Jumbo Port-private-based flow control parameters
 * Input:
 *      pOnThreshold 	- Flow control turn ON threshold
 *      pOffThreshold 	- Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlPortPrivateJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{   
    ret_t retVal;
   
    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_PORT_PRIVATE_OFF, RTL8367B_FLOWCTRL_JUMBO_PORT_PRIVATE_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_JUMBO_PORT_PRIVATE_ON, RTL8367B_FLOWCTRL_JUMBO_PORT_PRIVATE_ON_MASK, pOnThreshold);

    return retVal;
}



/* Function Name:
 *      rtl8367b_setAsicEgressFlowControlQueueDropThreshold
 * Description:
 *      Set Queue-based egress flow control turn on or ingress flow control drop on threshold
 * Input:
 *      qid 		- The queue id
 *      threshold 	- Queue-based flown control/drop turn ON threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_QUEUE_ID 	- Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEgressFlowControlQueueDropThreshold(rtk_uint32 qid, rtk_uint32 threshold)
{
    if( threshold >= RTL8367B_PAGE_NUMBER)
        return RT_ERR_OUT_OF_RANGE;
    
    if(qid > RTL8367B_QIDMAX)
        return RT_ERR_QUEUE_ID;

    return rtl8367b_setAsicRegBits(RTL8367B_FLOWCTRL_QUEUE_DROP_ON_REG(qid), RTL8367B_FLOWCTRL_QUEUE_DROP_ON_MASK, threshold);
}
/* Function Name:
 *      rtl8367b_getAsicEgressFlowControlQueueDropThreshold
 * Description:
 *      Get Queue-based egress flow control turn on or ingress flow control drop on threshold
 * Input:
 *      qid 		- The queue id
 *      pThreshold 	- Queue-based flown control/drop turn ON threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_QUEUE_ID 	- Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEgressFlowControlQueueDropThreshold(rtk_uint32 qid, rtk_uint32 *pThreshold)
{
    if(qid > RTL8367B_QIDMAX)
      return RT_ERR_QUEUE_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_FLOWCTRL_QUEUE_DROP_ON_REG(qid), RTL8367B_FLOWCTRL_QUEUE_DROP_ON_MASK, pThreshold);
}
/* Function Name:
 *      rtl8367b_setAsicEgressFlowControlPortDropThreshold
 * Description:
 *      Set port-based egress flow control turn on or ingress flow control drop on threshold
 * Input:
 *      port 		- Physical port number (0~7)
 *      threshold 	- Queue-based flown control/drop turn ON threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEgressFlowControlPortDropThreshold(rtk_uint32 port, rtk_uint32 threshold)
{
    if(port > RTL8367B_PORTIDMAX)
      return RT_ERR_PORT_ID;

    if(threshold >= RTL8367B_PAGE_NUMBER)
      return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_setAsicRegBits(RTL8367B_FLOWCTRL_PORT_DROP_ON_REG(port), RTL8367B_FLOWCTRL_PORT_DROP_ON_MASK, threshold);
}
/* Function Name:
 *      rtl8367b_setAsicEgressFlowControlPortDropThreshold
 * Description:
 *      Set port-based egress flow control turn on or ingress flow control drop on threshold
 * Input:
 *      port 		- Physical port number (0~7)
 *      pThreshold 	- Queue-based flown control/drop turn ON threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEgressFlowControlPortDropThreshold(rtk_uint32 port, rtk_uint32 *pThreshold)
{
    if(port > RTL8367B_PORTIDMAX)
    	return RT_ERR_PORT_ID; 

    return rtl8367b_getAsicRegBits(RTL8367B_FLOWCTRL_PORT_DROP_ON_REG(port), RTL8367B_FLOWCTRL_PORT_DROP_ON_MASK, pThreshold);
}
/* Function Name:
 *      rtl8367b_setAsicEgressFlowControlPortDropGap
 * Description:
 *      Set port-based egress flow control turn off or ingress flow control drop off gap
 * Input:
 *      gap 	- Flow control/drop turn OFF threshold = turn ON threshold - gap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEgressFlowControlPortDropGap(rtk_uint32 gap)
{
    if(gap >= RTL8367B_PAGE_NUMBER)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_PORT_GAP, RTL8367B_FLOWCTRL_PORT_GAP_MASK, gap);
}
/* Function Name:
 *      rtl8367b_getAsicEgressFlowControlPortDropGap
 * Description:
 *      Get port-based egress flow control turn off or ingress flow control drop off gap
 * Input:
 *      pGap 	- Flow control/drop turn OFF threshold = turn ON threshold - gap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEgressFlowControlPortDropGap(rtk_uint32 *pGap)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_PORT_GAP, RTL8367B_FLOWCTRL_PORT_GAP_MASK, pGap);
}
/* Function Name:
 *      rtl8367b_setAsicEgressFlowControlQueueDropGap
 * Description:
 *      Set Queue-based egress flow control turn off or ingress flow control drop off gap
 * Input:
 *      gap 	- Flow control/drop turn OFF threshold = turn ON threshold - gap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEgressFlowControlQueueDropGap(rtk_uint32 gap)
{
    if(gap >= RTL8367B_PAGE_NUMBER)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_FLOWCTRL_QUEUE_GAP, RTL8367B_FLOWCTRL_QUEUE_GAP_MASK, gap);
}
/* Function Name:
 *      rtl8367b_getAsicEgressFlowControlQueueDropGap
 * Description:
 *      Get Queue-based egress flow control turn off or ingress flow control drop off gap
 * Input:
 *      pGap 	- Flow control/drop turn OFF threshold = turn ON threshold - gap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEgressFlowControlQueueDropGap(rtk_uint32 *pGap)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_QUEUE_GAP, RTL8367B_FLOWCTRL_QUEUE_GAP_MASK, pGap);
}
/* Function Name:
 *      rtl8367b_getAsicEgressQueueEmptyPortMask
 * Description:
 *      Get queue empty port mask
 * Input:
 *      pPortmask 	-  Queue empty port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEgressQueueEmptyPortMask(rtk_uint32 *pPortmask)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_PORT_QEMPTY, pPortmask);
}
/* Function Name:
 *      rtl8367b_getAsicTotalPage
 * Description:
 *      Get system total page usage number
 * Input:
 *      pPageCount 	-  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicTotalPage(rtk_uint32 *pPageCount)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_TOTAL_PAGE_COUNTER, RTL8367B_FLOWCTRL_TOTAL_PAGE_COUNTER_MASK, pPageCount);
}
/* Function Name:
 *      rtl8367b_getAsicPulbicPage
 * Description:
 *      Get system public page usage number
 * Input:
 *      pPageCount 	-  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPulbicPage(rtk_uint32 *pPageCount)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_PUBLIC_PAGE_COUNTER, RTL8367B_FLOWCTRL_PUBLIC_PAGE_COUNTER_MASK, pPageCount);
}
/* Function Name:
 *      rtl8367b_getAsicMaxTotalPage
 * Description:
 *      Get system total page max usage number
 * Input:
 *      pPageCount 	-  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMaxTotalPage(rtk_uint32 *pPageCount)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_TOTAL_PAGE_MAX, RTL8367B_FLOWCTRL_TOTAL_PAGE_MAX_MASK, pPageCount);
}
/* Function Name:
 *      rtl8367b_getAsicPulbicPage
 * Description:
 *      Get system public page max usage number
 * Input:
 *      pPageCount 	-  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMaxPulbicPage(rtk_uint32 *pPageCount)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_FLOWCTRL_PUBLIC_PAGE_MAX, RTL8367B_FLOWCTRL_PUBLIC_PAGE_MAX_MASK, pPageCount);
}
/* Function Name:
 *      rtl8367b_getAsicPortPage
 * Description:
 *      Get per-port page usage number
 * Input:
 *      port 		-  Physical port number (0~7)
 *      pPageCount 	-  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortPage(rtk_uint32 port, rtk_uint32 *pPageCount)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_FLOWCTRL_PORT_PAGE_COUNTER_REG(port), RTL8367B_FLOWCTRL_PORT_PAGE_COUNTER_MASK, pPageCount);
}
/* Function Name:
 *      rtl8367b_getAsicPortPage
 * Description:
 *      Get per-port page max usage number
 * Input:
 *      port 		-  Physical port number (0~7)
 *      pPageCount 	-  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortPageMax(rtk_uint32 port, rtk_uint32 *pPageCount)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_FLOWCTRL_PORT_PAGE_MAX_REG(port), RTL8367B_FLOWCTRL_PORT_PAGE_MAX_MASK, pPageCount);
}

/* Function Name:
 *      rtl8367b_setAsicFlowControlEgressPortIndep
 * Description:
 *      Set per-port egress flow control independent 
 * Input:
 *      port 		- Physical port number (0~7)
 *      enabled     - Egress port flow control usage 1:enable 0:disable.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicFlowControlEgressPortIndep(rtk_uint32 port, rtk_uint32 enable)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_setAsicRegBit(RTL8367B_REG_PORT0_MISC_CFG + (port *0x20), RTL8367B_PORT0_MISC_CFG_FLOWCTRL_INDEP_OFFSET,enable);
}

/* Function Name:
 *      rtl8367b_getAsicFlowControlEgressPortIndep
 * Description:
 *      Get per-port egress flow control independent
 * Input:
 *      port 		- Physical port number (0~7)
 *      enabled     - Egress port flow control usage 1:enable 0:disable.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFlowControlEgressPortIndep(rtk_uint32 port, rtk_uint32 *pEnable)
{    
    return rtl8367b_getAsicRegBit(RTL8367B_REG_PORT0_MISC_CFG + (port *0x20),RTL8367B_PORT0_MISC_CFG_FLOWCTRL_INDEP_OFFSET,pEnable);
}
