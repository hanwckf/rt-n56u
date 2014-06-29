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
 * Feature : Port trunking related functions
 *
 */

#include "rtl8367b_asicdrv_trunking.h"
/* Function Name:
 *      rtl8367b_setAsicTrunkingMode
 * Description:
 *      Set port trunking mode
 * Input:
 *      mode 	- 1:dumb 0:user defined
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicTrunkingMode(rtk_uint32 mode)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_PORT_TRUNK_CTRL, RTL8367B_PORT_TRUNK_DUMB_OFFSET, mode);  
}
/* Function Name:
 *      rtl8367b_getAsicTrunkingMode
 * Description:
 *      Get port trunking mode
 * Input:
 *      pMode 	- 1:dumb 0:user defined
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicTrunkingMode(rtk_uint32* pMode)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_PORT_TRUNK_CTRL, RTL8367B_PORT_TRUNK_DUMB_OFFSET, pMode);  
}
/* Function Name:
 *      rtl8367b_setAsicTrunkingFc
 * Description:
 *      Set port trunking flow control
 * Input:
 *      group       - Trunk Group ID
 *      enabled 	- 0:disable, 1:enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicTrunkingFc(rtk_uint32 group, rtk_uint32 enabled)
{
    if(group > RTL8367B_MAX_TRUNK_GID)
        return RT_ERR_LA_TRUNK_ID;

    return rtl8367b_setAsicRegBit(RTL8367B_REG_PORT_TRUNK_FLOWCTRL, (RTL8367B_EN_FLOWCTRL_TG0_OFFSET + group), enabled);  
}
/* Function Name:
 *      rtl8367b_getAsicTrunkingFc
 * Description:
 *      Get port trunking flow control
 * Input:
 *      group       - Trunk Group ID
 *      pEnabled 	- 0:disable, 1:enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicTrunkingFc(rtk_uint32 group, rtk_uint32* pEnabled)
{
    if(group > RTL8367B_MAX_TRUNK_GID)
        return RT_ERR_LA_TRUNK_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_REG_PORT_TRUNK_FLOWCTRL, (RTL8367B_EN_FLOWCTRL_TG0_OFFSET + group), pEnabled);  
}
/* Function Name:
 *      rtl8367b_setAsicTrunkingGroup
 * Description:
 *      Set trunking group available port mask
 * Input:
 *      group       - Trunk Group ID
 *      portmask 	- Logic trunking enable port mask, max 4 ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicTrunkingGroup(rtk_uint32 group, rtk_uint32 portmask)
{
    if(group > RTL8367B_MAX_TRUNK_GID)
        return RT_ERR_LA_TRUNK_ID;
    
    return rtl8367b_setAsicRegBits(RTL8367B_REG_PORT_TRUNK_GROUP_MASK, RTL8367B_PORT_TRUNK_GROUP0_MASK_MASK << (group * 4), portmask);
}
/* Function Name:
 *      rtl8367b_getAsicTrunkingGroup
 * Description:
 *      Get trunking group available port mask
 * Input:
 *      group       - Trunk Group ID
 * Output:
 *      pPortmask 	- Logic trunking enable port mask, max 4 ports
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicTrunkingGroup(rtk_uint32 group, rtk_uint32* pPortmask)
{
    if(group > RTL8367B_MAX_TRUNK_GID)
        return RT_ERR_LA_TRUNK_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_REG_PORT_TRUNK_GROUP_MASK, RTL8367B_PORT_TRUNK_GROUP0_MASK_MASK << (group * 4), pPortmask);
}
/* Function Name:
 *      rtl8367b_setAsicTrunkingFlood
 * Description:
 *      Set port trunking flood function
 * Input:
 *      enabled 	- Port trunking flooding function 0:disable 1:enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicTrunkingFlood(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_PORT_TRUNK_CTRL, RTL8367B_PORT_TRUNK_FLOOD_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicTrunkingFlood
 * Description:
 *      Get port trunking flood function
 * Input:
 *      pEnabled 	- Port trunking flooding function 0:disable 1:enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicTrunkingFlood(rtk_uint32* pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_PORT_TRUNK_CTRL, RTL8367B_PORT_TRUNK_FLOOD_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicTrunkingHashSelect
 * Description:
 *      Set port trunking hash select sources
 * Input:
 *      hashsel 	- hash sources mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *    	7 bits mask for link aggregation group0 hash parameter selection {DIP, SIP, DMAC, SMAC, SPA}
 *    	0b0000001: SPA
 *    	0b0000010: SMAC
 *    	0b0000100: DMAC
 *    	0b0001000: SIP
 *    	0b0010000: DIP
 *    	0b0100000: TCP/UDP Source Port
 *    	0b1000000: TCP/UDP Destination Port
 */
ret_t rtl8367b_setAsicTrunkingHashSelect(rtk_uint32 hashsel)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_PORT_TRUNK_CTRL, RTL8367B_PORT_TRUNK_HASH_MASK, hashsel);
}
/* Function Name:
 *      rtl8367b_getAsicTrunkingHashSelect
 * Description:
 *      Get port trunking hash select sources
 * Input:
 *      pHashsel 	- hash sources mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *    	None
 */
ret_t rtl8367b_getAsicTrunkingHashSelect(rtk_uint32* pHashsel)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_PORT_TRUNK_CTRL, RTL8367B_PORT_TRUNK_HASH_MASK, pHashsel);
}
/* Function Name:
 *      rtl8367b_getAsicQeueuEmptyStatus
 * Description:
 *      Get current output queue if empty status
 * Input:
 *      portmask 	- queue empty port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *    	None
 */
ret_t rtl8367b_getAsicQeueuEmptyStatus(rtk_uint32* portmask)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_PORT_QEMPTY, portmask);
}
/* Function Name:
 *      rtl8367b_setAsicTrunkingHashTable
 * Description:
 *      Set port trunking hash value mapping table
 * Input:
 *      hashval 	- hashing value 0-15
 *      portId 		- trunking port id 0-3
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_OUT_OF_RANGE - Invalid hashing value (0-15)
 * Note:
 *    	None
 */
ret_t rtl8367b_setAsicTrunkingHashTable(rtk_uint32 hashval, rtk_uint32 portId)
{
	if(hashval > RTL8367B_TRUNKING_HASHVALUE_MAX)
		return RT_ERR_OUT_OF_RANGE;

	if(portId >= RTL8367B_TRUNKING_PORTNO)
		return RT_ERR_PORT_ID;

	if(hashval >= 8)
	    return rtl8367b_setAsicRegBits(RTL8367B_REG_PORT_TRUNK_HASH_MAPPING_CTRL1, RTL8367B_HASH8_MASK<<((hashval-8)*2), portId);
	else
		return rtl8367b_setAsicRegBits(RTL8367B_REG_PORT_TRUNK_HASH_MAPPING_CTRL0, RTL8367B_HASH0_MASK<<(hashval*2), portId);
}
/* Function Name:
 *      rtl8367b_getAsicTrunkingHashTable
 * Description:
 *      Get port trunking hash value mapping table
 * Input:
 *      hashval 	- hashing value 0-15
 *      pPortId 		- trunking port id 0-3
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - Invalid hashing value (0-15)
 * Note:
 *    	None
 */
ret_t rtl8367b_getAsicTrunkingHashTable(rtk_uint32 hashval, rtk_uint32* pPortId)
{
	if(hashval > RTL8367B_TRUNKING_HASHVALUE_MAX)
		return RT_ERR_OUT_OF_RANGE;

	if(hashval >= 8)
	    return rtl8367b_getAsicRegBits(RTL8367B_REG_PORT_TRUNK_HASH_MAPPING_CTRL1, RTL8367B_HASH8_MASK<<((hashval-8)*2), pPortId);
	else
		return rtl8367b_getAsicRegBits(RTL8367B_REG_PORT_TRUNK_HASH_MAPPING_CTRL0, RTL8367B_HASH0_MASK<<(hashval*2), pPortId);
}
