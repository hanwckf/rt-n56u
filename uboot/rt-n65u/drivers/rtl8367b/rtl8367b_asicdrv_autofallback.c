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
 * $Revision: 7550 $
 * $Date: 2009-12-08 14:16:22 +0800 (星期二, 08 十二月 2009) $
 *
 * Purpose : RTL8367B switch high-level API for RTL8367B
 * Feature : Proprietary Auto-fallback related function drivers
 *
 */
#include "rtl8367b_asicdrv_autofallback.h"


/* Function Name:
 *      rtl8367b_setAsicAutoFallBackEnable
 * Description:
 *      Set Per port auto-Fallback function enable/disable
 * Input:
 *      port    - port id.
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_ENABLE  	- Invalid enable/disable input
 *      RT_ERR_PORT_ID  - Error Port ID
 * Note:
 *      Set Per port Auto-Fallback function.
 */
ret_t rtl8367b_setAsicAutoFallBackEnable(rtk_uint32 port, rtk_uint32 enabled)
{
    ret_t   ret_val;
    rtk_uint32  data;

    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    if(enabled > 1)
        return RT_ERR_ENABLE;

    if((ret_val = rtl8367b_setAsicPHYReg(port, 31, 5)) != RT_ERR_OK)
        return ret_val;

    if((ret_val = rtl8367b_setAsicPHYReg(port, 5, 0x8b86)) != RT_ERR_OK)
        return ret_val;

    if((ret_val = rtl8367b_getAsicPHYReg(port, 6, &data)) != RT_ERR_OK)
        return ret_val;

    if(enabled)
        data |= 0x0100;
    else
        data &= 0xFEFF;
    
    if((ret_val = rtl8367b_setAsicPHYReg(port, 6, data)) != RT_ERR_OK)
        return ret_val;

    if((ret_val = rtl8367b_setAsicPHYReg(port, 31, 0)) != RT_ERR_OK)
        return ret_val;

    if((ret_val = rtl8367b_getAsicPHYReg(port, 0, &data)) != RT_ERR_OK)
        return ret_val;

    data |= 0x0200;

    if((ret_val = rtl8367b_setAsicPHYReg(port, 0, data)) != RT_ERR_OK)
        return ret_val;

    return rtl8367b_setAsicRegBit(RTL8367B_FALLBACK_PORT_CFG_REG(port), RTL8367B_FALLBACK_PORT0_CFG0_ENABLE_OFFSET, enabled);   
}

/* Function Name:
 *      rtl8367b_getAsicAutoFallBackEnable
 * Description:
 *      Set Per port auto-Fallback function enable/disable
 * Input:
 *      port        - port id.
 * Output:  
 *      pEnabled    - 1: enabled, 0: disabled
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Error Port ID
 * Note:
 *      Set Per port Auto-Fallback function.
 */
ret_t rtl8367b_getAsicAutoFallBackEnable(rtk_uint32 port, rtk_uint32 *pEnabled)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_FALLBACK_PORT_CFG_REG(port), RTL8367B_FALLBACK_PORT0_CFG0_ENABLE_OFFSET, pEnabled);   
}

/* Function Name:
 *      rtl8367b_setAsicAutoFallBackMaxCount
 * Description:
 *      Set Monitor and Error Ration
 * Input:
 *      monitor     - The Max value of monitor packet.
 * Output:  
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT    - Error Input
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicAutoFallBackMaxCount(RTL8367B_AUTOFALLBACK_MONITOR_MAX monitor)
{
    if(monitor >= MONITOR_MAX_END)
        return RT_ERR_INPUT;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_FALLBACK_CTRL, RTL8367B_FALLBACK_MONITORMAX_MASK, monitor);
}

/* Function Name:
 *      rtl8367b_setAsicAutoFallBackMaxCount
 * Description:
 *      Set Monitor and Error Ration
 * Input:
 *      pMonitor     - The Max value of monitor packet.
 * Output:  
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicAutoFallBackMaxCount(RTL8367B_AUTOFALLBACK_MONITOR_MAX *pMonitor)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_FALLBACK_CTRL, RTL8367B_FALLBACK_MONITORMAX_MASK, pMonitor);
}

/* Function Name:
 *      rtl8367b_setAsicAutoFallBackErrorRate
 * Description:
 *      Set Monitor and Error Ration
 * Input:
 *      ratio      - The Error Ratio threshold
 * Output:  
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicAutoFallBackErrorRate(RTL8367B_AUTOFALLBACK_ERR_RATIO ratio)
{
    if(ratio >= ERR_RATIO_END)
        return RT_ERR_INPUT;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_FALLBACK_CTRL, RTL8367B_FALLBACK_ERROR_RATIO_THRESHOLD_MASK, ratio);
}

/* Function Name:
 *      rtl8367b_setAsicAutoFallBackMaxCount
 * Description:
 *      Set Monitor and Error Ration
 * Input:
 *      pRatio      - The Error Ratio threshold
 * Output:  
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicAutoFallBackErrorRate(RTL8367B_AUTOFALLBACK_ERR_RATIO *pRatio)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_FALLBACK_CTRL, RTL8367B_FALLBACK_ERROR_RATIO_THRESHOLD_MASK, pRatio);
}

/* Function Name:
 *      rtl8367b_setAsicAutoFallBackTimeout
 * Description:
 *      Set timeout value
 * Input:
 *      timeout_ms  - The timeout value, should be multiple of 4ms.
 * Output:  
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT    - Error Input
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicAutoFallBackTimeout(rtk_uint32 timeout_ms)
{
    ret_t   ret_val;

    if(timeout_ms >= AUTOFALLBACK_MAX_TIMEOUT)
        return RT_ERR_INPUT;

    if((timeout_ms % 4) != 0)
        return RT_ERR_INPUT;

    if((ret_val = rtl8367b_setAsicRegBits(RTL8367B_REG_FALLBACK_CTRL, RTL8367B_FALLBACK_MONITOR_TIMEOUT_MASK, (timeout_ms / 4))) != RT_ERR_OK)
        return ret_val;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicAutoFallBackTimeout
 * Description:
 *      Get timeout value
 * Input:
 *      None.
 * Output:  
 *      pTimeout_ms  - The timeout value.
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT    - Error Input
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicAutoFallBackTimeout(rtk_uint32 *pTimeout_ms)
{
    ret_t   ret_val;

    if((ret_val = rtl8367b_getAsicRegBits(RTL8367B_REG_FALLBACK_CTRL, RTL8367B_FALLBACK_MONITOR_TIMEOUT_MASK, pTimeout_ms)) != RT_ERR_OK)
        return ret_val;

    *pTimeout_ms *= 4;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicAutoFallBackTimeoutIgnore
 * Description:
 *      Set timeout ignore
 * Input:
 *      enabled  - The timeout ignore, 1:enable, 0:disable.
 * Output:  
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT    - Error Input
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicAutoFallBackTimeoutIgnore(rtk_uint32 enabled)
{
    if(enabled > 1)
        return RT_ERR_ENABLE;

    return rtl8367b_setAsicRegBit(RTL8367B_REG_FALLBACK_CTRL, RTL8367B_FALLBACK_MONITOR_TIMEOUT_IGNORE_OFFSET, enabled);   
}

/* Function Name:
 *      rtl8367b_getAsicAutoFallBackTimeoutIgnore
 * Description:
 *      Set timeout ignore
 * Input:
 *      None
 * Output:  
 *      pEnabled  - The timeout ignore, 1:enable, 0:disable.
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT    - Error Input
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicAutoFallBackTimeoutIgnore(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_FALLBACK_CTRL, RTL8367B_FALLBACK_MONITOR_TIMEOUT_IGNORE_OFFSET, pEnabled);   
}

/* Function Name:
 *      rtl8367b_getAsicAutoFallBackMonitorCNT
 * Description:
 *      Get Monitor Counter
 * Input:
 *      port        - port id.
 * Output:  
 *      pMcnt       - Monitor Counter.
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT    - Error Input
 *      RT_ERR_PORT_ID  - Error Port ID
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicAutoFallBackMonitorCNT(rtk_uint32 port, rtk_uint32 *pMcnt)
{
    rtk_uint32  data;
    ret_t   ret_val;

    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    if((ret_val = rtl8367b_getAsicReg(RTL8367B_FALLBACK_PORT_MON_CNT_REG(port), &data)) != RT_ERR_OK)
        return ret_val;

    *pMcnt = data;

    if((ret_val = rtl8367b_getAsicReg(RTL8367B_FALLBACK_PORT_MON_CNT_REG(port)+1, &data)) != RT_ERR_OK)
        return ret_val;

    *pMcnt |= (data << 16);

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicAutoFallBackErrorCNT
 * Description:
 *      Get Error Counter
 * Input:
 *      port        - port id.
 * Output:  
 *      pMcnt       - Monitor Counter.
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT    - Error Input
 *      RT_ERR_PORT_ID  - Error Port ID
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicAutoFallBackErrorCNT(rtk_uint32 port, rtk_uint32 *pEcnt)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicReg(RTL8367B_FALLBACK_PORT_ERR_CNT_REG(port), pEcnt);
}
