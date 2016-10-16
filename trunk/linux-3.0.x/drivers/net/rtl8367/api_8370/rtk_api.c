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
 * $Revision: 23355 $
 * $Date: 2011-09-27 18:33:58 +0800 (星期二, 27 九月 2011) $
 *
 * Purpose : RTK switch high-level API for RTL8370/RTL8367
 * Feature : Here is a list of all functions and variables in this module.
 *
 */


#include "rtl8370_asicdrv.h"
#include "rtl8370_asicdrv_dot1x.h"
#if !defined(_REDUCE_CODE)
 #include "rtl8370_asicdrv_acl.h"
 #include "rtl8370_asicdrv_qos.h"
 #include "rtl8370_asicdrv_scheduling.h"
 #include "rtl8370_asicdrv_fc.h"
 #include "rtl8370_asicdrv_svlan.h"
 #include "rtl8370_asicdrv_inbwctrl.h"
 #include "rtl8370_asicdrv_mirror.h"
 #include "rtl8370_asicdrv_igmp.h"
 #include "rtl8370_asicdrv_rma.h"
 #include "rtl8370_asicdrv_cputag.h"
#endif
#include "rtl8370_asicdrv_port.h"
#include "rtl8370_asicdrv_phy.h"
#include "rtl8370_asicdrv_unknownMulticast.h"
#include "rtl8370_asicdrv_vlan.h"
#include "rtl8370_asicdrv_lut.h"
#include "rtl8370_asicdrv_led.h"
#include "rtl8370_asicdrv_meter.h"
#include "rtl8370_asicdrv_storm.h"
#include "rtl8370_asicdrv_misc.h"
#include "rtl8370_asicdrv_portIsolation.h"
#include "rtl8370_asicdrv_trunking.h"
#include "rtl8370_asicdrv_mib.h"
#include "rtl8370_asicdrv_interrupt.h"
#include "rtl8370_asicdrv_green.h"
#include "rtl8370_asicdrv_eee.h"

#include "rtk_api.h"

#include "rtk_api_ext.h"
#include "rtk_error.h"

typedef enum rtk_filter_data_type_e
{
    RTK_FILTER_DATA_MAC = 0,
    RTK_FILTER_DATA_UINT16,
    RTK_FILTER_DATA_TAG,
    RTK_FILTER_DATA_IPV4,
    RTK_FILTER_DATA_UINT8_HIGH,    
    RTK_FILTER_DATA_UINT8_LOW,
    RTK_FILTER_DATA_IPV4FLAG,
    RTK_FILTER_DATA_UINT13_LOW,
    RTK_FILTER_DATA_TCPFLAG,
    RTK_FILTER_DATA_IPV6,
} rtk_filter_data_type_t;

/* Function Name:
 *      rtk_rate_shareMeter_set
 * Description:
 *      Set meter configuration
 * Input:
 *      index       - Shared meter index
 *      rate        - Rate of share meter
 *      ifg_include - Include IFG or not, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_FILTER_METER_ID - Invalid meter
 *      RT_ERR_RATE            - Invalid rate
 *      RT_ERR_INPUT           - Invalid input parameters
 * Note:
 *      The API can set shared meter rate and ifg include for each meter. 
 *      The rate unit is 1 kbps and the range is from 8k to 1048568k.
 *      The granularity of rate is 8 kbps. The ifg_include parameter is used 
 *      for rate calculation with/without inter-frame-gap and preamble.
 */
rtk_api_ret_t rtk_rate_shareMeter_set(rtk_meter_id_t index, rtk_rate_t rate, rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;
    
    if (index>=RTK_MAX_NUM_OF_METER)
        return RT_ERR_FILTER_METER_ID;

    if (rate>RTK_QOS_RATE_INPUT_MAX || rate<RTK_QOS_RATE_INPUT_MIN)
        return RT_ERR_RATE ;

    if (ifg_include>=RTK_ENABLE_END)
        return RT_ERR_INPUT;    
        
    if ((retVal = rtl8370_setAsicShareMeter(index,rate>>3,ifg_include))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_shareMeter_get
 * Description:
 *      Get meter configuration
 * Input:
 *      index        - Shared meter index
 * Output:
 *      pRate        - Pointer of rate of share meter
 *      pIfg_include - Include IFG or not, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_FILTER_METER_ID - Invalid meter
 * Note:
 *      The API can get shared meter rate and ifg include for each meter. 
 *      The rate unit is 1 kbps and the granularity of rate is 8 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble 
 */

rtk_api_ret_t rtk_rate_shareMeter_get(rtk_meter_id_t index, rtk_rate_t *pRate ,rtk_data_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    uint32 regData;
    
    if (index>=RTK_MAX_NUM_OF_METER)
        return RT_ERR_FILTER_METER_ID;

    if ((retVal = rtl8370_getAsicShareMeter(index, &regData, pIfg_include))!=RT_ERR_OK)
        return retVal; 

    *pRate = regData<<3;
        
    return RT_ERR_OK;
}

#if !defined(_REDUCE_CODE)

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set port ingress bandwidth control
 * Input:
 *      port - Port id
 *      rate - Rate of share meter
 *      ifg_include - Include IFG or not, ENABLE:include DISABLE:exclude
 *      fc_enable - Enable flow control or not, ENABLE:use flow control DISABLE:drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE - Invalid ingress rate parameter.
 * Note:
 *      The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps. 
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */

rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_set(rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include, rtk_enable_t fc_enable)
{
    rtk_api_ret_t retVal;

    if (port >RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;    

    if (rate>RTK_QOS_RATE_INPUT_MAX || rate<RTK_QOS_RATE_INPUT_MIN)
        return RT_ERR_INBW_RATE ;

    if (ifg_include>=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8370_setAsicPortIngressBandwidth(port,rate>>3,ifg_include,fc_enable))!=RT_ERR_OK)
        return retVal;            

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get port ingress bandwidth control
 * Input:
 *      port - Port id
 * Output:
 *      pRate - Rate of share meter
 *      pIfg_include - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 *      pFc_enable - Enable flow control or not, ENABLE:use flow control DISABLE:drop
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *     The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps. 
 *     The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */

rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_data_t *pIfg_include, rtk_data_t *pFc_enable)
{
    rtk_api_ret_t retVal;
    uint32 regData;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;    

    if ((retVal = rtl8370_getAsicPortIngressBandwidth(port,&regData, pIfg_include, pFc_enable))!=RT_ERR_OK)
        return retVal;            

    *pRate = regData<<3;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set port egress bandwidth control
 * Input:
 *      port - Port id
 *      rate - Rate of egress bandwidth
 *      ifg_include - Include IFG or not, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_EBW_RATE - Invalid egress bandwidth/rate
 * Note:
 *     The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps. 
 *     The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */
 
rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_set( rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;    

    if (rate>RTK_QOS_RATE_INPUT_MAX || rate<RTK_QOS_RATE_INPUT_MIN )
        return RT_ERR_QOS_EBW_RATE ;

    if (ifg_include>=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8370_setAsicPortEgressRate(port,rate>>3))!=RT_ERR_OK)
        return retVal;  

    if ((retVal = rtl8370_setAsicPortEgressRateIfg(ifg_include))!=RT_ERR_OK)
        return retVal;  
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_get
 * Description:
 *      Get port egress bandwidth control
 * Input:
 *      port - Port id
 * Output:
 *      pRate - Rate of egress bandwidth
 *      pIfg_include - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *     The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps. 
 *     The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */

rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_data_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    uint32 regData;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;    

    if ((retVal = rtl8370_getAsicPortEgressRate(port,&regData))!=RT_ERR_OK)
        return retVal; 

    *pRate = regData<<3;

    if ((retVal = rtl8370_getAsicPortEgressRateIfg((uint32*)pIfg_include))!=RT_ERR_OK)
        return retVal; 
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_rate_egrQueueBwCtrlEnable_get
 * Description:
 *      Get enable status of egress bandwidth control on specified queue.
 * Input:
 *      port    - Port id
 *      queue   - Queue id
 * Output:
 *      pEnable - Pointer to enable status of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
rtk_api_ret_t rtk_rate_egrQueueBwCtrlEnable_get(rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;  

    /*for whole port function, the queue value should be 0xFF*/
    if (queue != RTK_WHOLE_SYSTEM)
        return RT_ERR_QUEUE_ID;

    if ((retVal = rtl8370_getAsicAprEnable(port,pEnable))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlEnable_set
 * Description:
 *      Set enable status of egress bandwidth control on specified queue.
 * Input:
 *      port   - Port id
 *      queue  - Queue id
 *      enable - Enable status of egress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_rate_egrQueueBwCtrlEnable_set(rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;  

    /*for whole port function, the queue value should be 0xFF*/
    if (queue != RTK_WHOLE_SYSTEM)
        return RT_ERR_QUEUE_ID;

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;  

    if ((retVal = rtl8370_setAsicAprEnable(port,enable))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlRate_get
 * Description:
 *      Get rate of egress bandwidth control on specified queue.
 * Input:
 *      port  - Port id
 *      queue - Queue id
 * Output:
 *      pIndex - shared meter index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter id
 * Note:
 *    The actual rate control is set in shared meters.
 *    The unit of granularity is 8Kbps.
 */
rtk_api_ret_t rtk_rate_egrQueueBwCtrlRate_get(rtk_port_t port, rtk_qid_t queue, rtk_meter_id_t *pIndex)
{
    rtk_api_ret_t retVal;
    uint32 offset_idx;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;  

    if (queue >= RTK_MAX_NUM_OF_QUEUE)
        return RT_ERR_QUEUE_ID;
    

    if ((retVal=rtl8370_getAsicAprMeter(port,queue,&offset_idx))!=RT_ERR_OK)
        return retVal;

    *pIndex = offset_idx+ port*8;

     return RT_ERR_OK;
}


/* Function Name:
 *      rtk_rate_egrQueueBwCtrlRate_set
 * Description:
 *      Set rate of egress bandwidth control on specified queue.
 * Input:
 *      port  - Port id
 *      queue - Queue id
 *      index - Shared meter index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter id
 * Note:
 *    The actual rate control is set in shared meters.
 *    The unit of granularity is 8Kbps.
 */
rtk_api_ret_t rtk_rate_egrQueueBwCtrlRate_set(rtk_port_t port, rtk_qid_t queue, rtk_meter_id_t index)
{
    rtk_api_ret_t retVal;
    uint32 offset_idx;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;  

    if (queue >= RTK_MAX_NUM_OF_QUEUE)
        return RT_ERR_QUEUE_ID;

    if (index>=RTK_MAX_NUM_OF_METER)
        return RT_ERR_FILTER_METER_ID;
            
    if (index < (port*8) ||  index > (7 + port*8))
        return RT_ERR_FILTER_METER_ID;   

    offset_idx = index- port*8;
                
    if ((retVal=rtl8370_setAsicAprMeter(port,queue,offset_idx))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;   
}



/* Function Name:
 *      rtk_qos_init
 * Description:
 *      Configure Qos default settings with queue number assigment to each port.
 * Input:
 *      queueNum - Queue number of each port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_QUEUE_NUM - Invalid queue number.
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API will initialize related Qos setting with queue number assigment.
 *      The queue number is from 1 to 8.
 */
rtk_api_ret_t rtk_qos_init(rtk_queue_num_t queueNum)
{
    CONST_T uint16 g_prioritytToQid[8][8]= { 
        {0,0,0,0,0,0,0,0}, 
        {0,0,0,0,7,7,7,7}, 
        {0,0,0,0,1,1,7,7}, 
        {0,0,1,1,2,2,7,7},
        {0,0,1,1,2,3,7,7},
        {0,0,1,2,3,4,7,7},
        {0,0,1,2,3,4,5,7},
        {0,1,2,3,4,5,6,7}
    };

    CONST_T uint32 g_priorityDecision[8] = {0x01,0x80,0x04,0x02,0x20,0x40,0x10,0x08};
    CONST_T uint32 g_prioritytRemap[8] = {0,1,2,3,4,5,6,7};

    rtk_api_ret_t retVal;
    uint32 qmapidx;
    uint32 priority;
    uint32 priDec;
    
    uint32 qid;
    uint32 port;
    uint32 dscp;

    if (queueNum <= 0 || queueNum > RTK_MAX_NUM_OF_QUEUE)
        return RT_ERR_QUEUE_NUM;

    /*Set Output Queue Number*/
    if (RTK_MAX_NUM_OF_QUEUE == queueNum)
        qmapidx = 0;
    else
        qmapidx = queueNum;           
    for (port = 0;port<RTK_MAX_NUM_OF_PORT;port++)
    {
        if ((retVal = rtl8370_setAsicOutputQueueMappingIndex(port, qmapidx))!=RT_ERR_OK)
            return retVal;             
    }

    /*Set Priority to Qid*/
    for (priority = 0;priority<RTK_DOT1P_PRIORITY_MAX;priority++)
    {
        if ((retVal = rtl8370_setAsicPriorityToQIDMappingTable(qmapidx, priority, g_prioritytToQid[qmapidx][priority]))!=RT_ERR_OK)
            return retVal;
    }

    /*Set Queue Type to Strict Priority*/
    for (port = 0;port<RTK_MAX_NUM_OF_PORT;port++)
    {
        for (qid = 0;qid<RTK_MAX_NUM_OF_QUEUE;qid++)
        {
            if ((retVal = rtl8370_setAsicQueueType(port,qid,QTYPE_STRICT))!=RT_ERR_OK)
                return retVal;
        }    
    }

    /*Priority Decision Order*/
    for (priDec = 0;priDec<PRIDEC_MAX;priDec++)
    {
        if ((retVal = rtl8370_setAsicPriorityDecision(priDec,g_priorityDecision[priDec]))!=RT_ERR_OK)
            return retVal;
    }

    /*Set Port-based Priority to 0*/
    for (port = 0;port<RTK_MAX_NUM_OF_PORT;port++)
    {
        if ((retVal = rtl8370_setAsicPriorityPortBased(port,0))!=RT_ERR_OK)
            return retVal;    
    }

    /*Disable 1p Remarking*/
    if ((retVal = rtl8370_setAsicRemarkingDot1pAbility(DISABLE))!=RT_ERR_OK)
        return retVal;

    /*Disable DSCP Remarking*/
    if ((retVal = rtl8370_setAsicRemarkingDscpAbility(DISABLE))!=RT_ERR_OK)
        return retVal;

    /*Set 1p & DSCP  Priority Remapping & Remarking*/
    for (priority = 0;priority<RTK_DOT1P_PRIORITY_MAX;priority++)
    {
        if ((retVal = rtl8370_setAsicPriorityDot1qRemapping(priority,g_prioritytRemap[priority]))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8370_setAsicRemarkingDot1pParameter(priority,0))!=RT_ERR_OK)
            return retVal;
        
        if ((retVal = rtl8370_setAsicRemarkingDscpParameter(priority,0))!=RT_ERR_OK)
            return retVal;
    }

    /*Set DSCP Priority*/
    for (dscp = 0;dscp<63;dscp++)
    {
        if ((retVal = rtl8370_setAsicPriorityDscpBased(dscp,0))!=RT_ERR_OK)
            return retVal;
    }


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_priSel_set
 * Description:
 *      Configure the priority order among different priority mechanism.
 * Input:
 *      pPriDec - Priority assign for port, dscp, 802.1p, cvlan, svlan, acl based priority decision.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 * Note:
 *      ASIC will follow user priority setting of mechanisms to select mapped queue priority for receiving frame. 
 *      If two priority mechanisms are the same, the ASIC will chose the highest priority from mechanisms to 
 *      assign queue priority to receiving frame.  
 *      The priority sources are:
 *      PRIDEC_PORT
 *      PRIDEC_ACL
 *      PRIDEC_DSCP
 *      PRIDEC_1Q
 *      PRIDEC_1AD
 *      PRIDEC_CVLAN
 *      PRIDEC_DA
 *      PRIDEC_SA 
 */

rtk_api_ret_t rtk_qos_priSel_set(rtk_priority_select_t *pPriDec)
{ 
    rtk_api_ret_t retVal;
    uint32 port_pow;
    uint32 dot1q_pow;
    uint32 dscp_pow;
    uint32 acl_pow;
    uint32 svlan_pow;
    uint32 cvlan_pow;
    uint32 smac_pow;
    uint32 dmac_pow;
    
    if (pPriDec->port_pri > 8 || pPriDec->dot1q_pri > 8 || pPriDec->acl_pri > 8 || pPriDec->dscp_pri > 8 ||
       pPriDec->cvlan_pri > 8 || pPriDec->svlan_pri > 8 || pPriDec->dmac_pri > 8 || pPriDec->smac_pri > 8)
        return RT_ERR_QOS_SEL_PRI_SOURCE;

    port_pow = 1;  
    for (; pPriDec->port_pri > 0; pPriDec->port_pri--)
        port_pow = (port_pow)*2;

    dot1q_pow = 1;
    for (; pPriDec->dot1q_pri > 0; pPriDec->dot1q_pri--)
        dot1q_pow = (dot1q_pow)*2;

    acl_pow = 1;
    for (; pPriDec->acl_pri > 0; pPriDec->acl_pri--)
        acl_pow = (acl_pow)*2;

    dscp_pow = 1;
    for (; pPriDec->dscp_pri > 0; pPriDec->dscp_pri--)
        dscp_pow = (dscp_pow)*2;

    svlan_pow = 1;
    for (; pPriDec->svlan_pri > 0; pPriDec->svlan_pri--)
        svlan_pow = (svlan_pow)*2;

    cvlan_pow = 1;
    for (; pPriDec->cvlan_pri > 0; pPriDec->cvlan_pri--)
        cvlan_pow = (cvlan_pow)*2;

    dmac_pow = 1;
    for (; pPriDec->dmac_pri > 0; pPriDec->dmac_pri--)
        dmac_pow = (dmac_pow)*2;

    smac_pow = 1;
    for (; pPriDec->smac_pri > 0; pPriDec->smac_pri--)
        smac_pow = (smac_pow)*2;   

    if ((retVal = rtl8370_setAsicPriorityDecision(PRIDEC_PORT,port_pow))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicPriorityDecision(PRIDEC_ACL,acl_pow))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicPriorityDecision(PRIDEC_DSCP,dscp_pow))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicPriorityDecision(PRIDEC_1Q,dot1q_pow))!=RT_ERR_OK)
        return retVal;
    
    if ((retVal = rtl8370_setAsicPriorityDecision(PRIDEC_1AD,svlan_pow))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicPriorityDecision(PRIDEC_CVLAN,cvlan_pow))!=RT_ERR_OK)
        return retVal;
    
    if ((retVal = rtl8370_setAsicPriorityDecision(PRIDEC_DA,dmac_pow))!=RT_ERR_OK)
        return retVal;
    
    if ((retVal = rtl8370_setAsicPriorityDecision(PRIDEC_SA,smac_pow))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_priSel_get
 * Description:
 *      Get the priority order configuration among different priority mechanism.
 * Input:
 *      None
 * Output:
 *      pPriDec - Priority assign for port, dscp, 802.1p, cvlan, svlan, acl based priority decision .
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      ASIC will follow user priority setting of mechanisms to select mapped queue priority for receiving frame. 
 *      If two priority mechanisms are the same, the ASIC will chose the highest priority from mechanisms to 
 *      assign queue priority to receiving frame. 
 *      The priority sources are:
 *      PRIDEC_PORT,
 *      PRIDEC_ACL,
 *      PRIDEC_DSCP,
 *      PRIDEC_1Q,
 *      PRIDEC_1AD,
 *      PRIDEC_CVLAN,
 *      PRIDEC_DA,
 *      PRIDEC_SA,
 */

rtk_api_ret_t rtk_qos_priSel_get(rtk_priority_select_t *pPriDec)
{
    rtk_api_ret_t retVal;
    uint32 i;
    uint32 port_pow;
    uint32 dot1q_pow;
    uint32 dscp_pow;
    uint32 acl_pow;
    uint32 svlan_pow;
    uint32 cvlan_pow;
    uint32 smac_pow;
    uint32 dmac_pow;    

    if ((retVal = rtl8370_getAsicPriorityDecision(PRIDEC_PORT,&port_pow))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicPriorityDecision(PRIDEC_ACL,&acl_pow))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicPriorityDecision(PRIDEC_DSCP,&dscp_pow))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicPriorityDecision(PRIDEC_1Q,&dot1q_pow))!=RT_ERR_OK)
        return retVal;
    
    if ((retVal = rtl8370_getAsicPriorityDecision(PRIDEC_1AD,&svlan_pow))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicPriorityDecision(PRIDEC_CVLAN,&cvlan_pow))!=RT_ERR_OK)
        return retVal;
    
    if ((retVal = rtl8370_getAsicPriorityDecision(PRIDEC_DA,&dmac_pow))!=RT_ERR_OK)
        return retVal;
    
    if ((retVal = rtl8370_getAsicPriorityDecision(PRIDEC_SA,&smac_pow))!=RT_ERR_OK)
        return retVal;

    for (i=31;i>=0;i--)
    {
        if (port_pow&(1<<i))
        {
            pPriDec->port_pri = i;
            break;
        }
    }
    
    for (i=31;i>=0;i--)
    {
        if (dot1q_pow&(1<<i))
        {
            pPriDec->dot1q_pri = i;
            break;
        }
    }

    for (i=31;i>=0;i--)
    {
        if (acl_pow&(1<<i))
        {
            pPriDec->acl_pri = i;
            break;
        }
    }

    for (i=31;i>=0;i--)
    {
        if (dscp_pow&(1<<i))
        {
            pPriDec->dscp_pri = i;
            break;
        }
    }

    for (i=31;i>=0;i--)
    {
        if (svlan_pow&(1<<i))
        {
            pPriDec->svlan_pri = i;
            break;
        }
    }

    for (i=31;i>=0;i--)
    {
        if (cvlan_pow&(1<<i))
        {
            pPriDec->cvlan_pri = i;
            break;
        }
    }

    for (i=31;i>=0;i--)
    {
        if (dmac_pow&(1<<i))
        {
            pPriDec->dmac_pri = i;
            break;
        }
    }

    for (i=31;i>=0;i--)
    {
        if (smac_pow&(1<<i))
        {
            pPriDec->smac_pri = i;
            break;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_1pPriRemap_set
 * Description:
 *      Configure 1Q priorities mapping to internal absolute priority.
 * Input:
 *      dot1p_pri - 802.1p priority value.
 *      int_pri - internal priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_VLAN_PRIORITY - Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority. 
 * Note:
 *      Priority of 802.1Q assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtk_api_ret_t rtk_qos_1pPriRemap_set(rtk_pri_t dot1p_pri, rtk_pri_t int_pri)
{
    rtk_api_ret_t retVal;

    if (int_pri>RTK_DOT1P_PRIORITY_MAX)
        return  RT_ERR_QOS_INT_PRIORITY;

    if (dot1p_pri>RTK_DOT1P_PRIORITY_MAX||int_pri>RTK_DOT1P_PRIORITY_MAX)
        return  RT_ERR_VLAN_PRIORITY;
    
    if ((retVal = rtl8370_setAsicPriorityDot1qRemapping(dot1p_pri, int_pri))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_1pPriRemap_get
 * Description:
 *      Get 1Q priorities mapping to internal absolute priority.  
 * Input:
 *      dot1p_pri - 802.1p priority value .
 * Output:
 *      pInt_pri - internal priority value.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_VLAN_PRIORITY - Invalid priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority. 
 * Note:
 *      Priority of 802.1Q assigment for internal asic priority, and it is uesed for queue usage and packet scheduling.
 */
rtk_api_ret_t rtk_qos_1pPriRemap_get(rtk_pri_t dot1p_pri, rtk_pri_t *pInt_pri)
{
    rtk_api_ret_t retVal;

    if (dot1p_pri>RTK_DOT1P_PRIORITY_MAX)
        return  RT_ERR_QOS_INT_PRIORITY;
    

    if ((retVal = rtl8370_getAsicPriorityDot1qRemapping(dot1p_pri, pInt_pri))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_dscpPriRemap_set
 * Description:
 *      Map dscp value to internal priority.
 * Input:
 *      dscp - Dscp value of receiving frame
 *      int_pri - internal priority value .
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QOS_DSCP_VALUE - Invalid DSCP value. 
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority. 
 * Note:
 *      The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically 
 *      greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of 
 *      DSCP are carefully chosen then backward compatibility can be achieved.    
 */
rtk_api_ret_t rtk_qos_dscpPriRemap_set(rtk_dscp_t dscp, rtk_pri_t int_pri)
{
    rtk_api_ret_t retVal;

    if (int_pri > RTK_DOT1P_PRIORITY_MAX )
        return RT_ERR_QOS_INT_PRIORITY; 

    if (dscp > RTK_VALUE_OF_DSCP_MAX)
        return RT_ERR_QOS_DSCP_VALUE; 

    if ((retVal = rtl8370_setAsicPriorityDscpBased(dscp,int_pri))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;    
}


/* Function Name:
 *      rtk_qos_dscpPriRemap_get
 * Description:
 *      Get dscp value to internal priority.
 * Input:
 *      dscp - Dscp value of receiving frame
 * Output:
 *      pInt_pri - internal priority value. 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_QOS_DSCP_VALUE - Invalid DSCP value. 
 * Note:
 *      The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically 
 *      greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of 
 *      DSCP are carefully chosen then backward compatibility can be achieved.    
 */
rtk_api_ret_t rtk_qos_dscpPriRemap_get(rtk_dscp_t dscp, rtk_pri_t *pInt_pri)
{
    rtk_api_ret_t retVal;

    if (dscp > RTK_VALUE_OF_DSCP_MAX)
        return RT_ERR_QOS_DSCP_VALUE; 

    if ((retVal = rtl8370_getAsicPriorityDscpBased(dscp,pInt_pri))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;    
}

/* Function Name:
 *      rtk_qos_portPri_set
 * Description:
 *      Configure priority usage to each port.
 * Input:
 *      port - Port id.
 *      int_pri - Internal priority value. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_QOS_SEL_PORT_PRI - Invalid port priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority. 
 * Note:
 *      The API can set priority of port assignments for queue usage and packet scheduling.
 */
rtk_api_ret_t rtk_qos_portPri_set(rtk_port_t port, rtk_pri_t int_pri)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 

    if (int_pri > RTK_DOT1P_PRIORITY_MAX )
        return RT_ERR_QOS_INT_PRIORITY; 

    if ((retVal = rtl8370_setAsicPriorityPortBased(port, int_pri))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_portPri_get
 * Description:
 *      Get priority usage to each port.
 * Input:
 *      port - Port id.
 * Output:
 *      pInt_pri - Internal priority value. 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number. 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get priority of port assignments for queue usage and packet scheduling.
 */
rtk_api_ret_t rtk_qos_portPri_get(rtk_port_t port, rtk_pri_t *pInt_pri)
{
    rtk_api_ret_t retVal;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 

    if ((retVal = rtl8370_getAsicPriorityPortBased(port, pInt_pri))!=RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}
/* Function Name:
 *      rtk_qos_queueNum_set
 * Description:
 *      Set output queue number for each port.
 * Input:
 *      port - Port id.
 *      queue_num - Queue number
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_QUEUE_NUM - Invalid queue number. 
 * Note:
 *      The API can set the output queue number of the specified port. The queue number is from 1 to 8.
 */
rtk_api_ret_t rtk_qos_queueNum_set(rtk_port_t port, rtk_queue_num_t queue_num)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (( 0 == queue_num) || (queue_num > RTK_MAX_NUM_OF_QUEUE)) 
        return RT_ERR_FAILED;

    if (RTK_MAX_NUM_OF_QUEUE== queue_num)
        queue_num = 0;

    if ((retVal = rtl8370_setAsicOutputQueueMappingIndex(port, queue_num))!=RT_ERR_OK)
        return retVal;      

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_queueNum_get
 * Description:
 *      Get output queue number.
 * Input:
 *      port - Port id.
 * Output:
 *      pQueue_num - Mapping queue number
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      The API will return the output queue number of the specified port. The queue number is from 1 to 8.
 */
rtk_api_ret_t rtk_qos_queueNum_get(rtk_port_t port, rtk_queue_num_t *pQueue_num)
{
    rtk_api_ret_t retVal;
    uint32 qidx;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicOutputQueueMappingIndex(port, &qidx))!=RT_ERR_OK)
        return retVal;  

    if (0 == qidx)
        *pQueue_num    = 8;
    else
        *pQueue_num = qidx;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_priMap_set
 * Description:
 *      Set output queue number for each port.
 * Input:
 *      queue_num - Queue number usage.
 *      pPri2qid - Priority mapping to queue ID.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QUEUE_NUM - Invalid queue number. 
 *      RT_ERR_QUEUE_ID - Invalid queue id.
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      ASIC supports priority mapping to queue with different queue number from 1 to 8.
 *      For different queue numbers usage, ASIC supports different internal available queue IDs.
 */
rtk_api_ret_t rtk_qos_priMap_set(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid)
{    
    rtk_api_ret_t retVal;
    uint32 pri;
    
    if ((0 == queue_num) || (queue_num > RTK_MAX_NUM_OF_QUEUE)) 
        return RT_ERR_QUEUE_NUM;

    for (pri=0;pri<=RTK_DOT1P_PRIORITY_MAX;pri++)
    {
        if (pPri2qid->pri2queue[pri] > RTK_QUEUE_ID_MAX) 
            return RT_ERR_QUEUE_ID;

        if ((retVal =  rtl8370_setAsicPriorityToQIDMappingTable(queue_num-1,pri,pPri2qid->pri2queue[pri]))!=RT_ERR_OK)
            return retVal;        
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_priMap_get
 * Description:
 *      Get priority to queue ID mapping table parameters.
 * Input:
 *      queue_num - Queue number usage. 
 * Output:
 *      pPri2qid - Priority mapping to queue ID.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_QUEUE_NUM - Invalid queue number.
 * Note:
 *      The API can return the mapping queue id of the specified priority and queue number. 
 *      The queue number is from 1 to 8.
 */
rtk_api_ret_t rtk_qos_priMap_get(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid)
{
    rtk_api_ret_t retVal;
    uint32 pri;
    
    if ((0 == queue_num) || (queue_num > RTK_MAX_NUM_OF_QUEUE)) 
        return RT_ERR_QUEUE_NUM;

    for (pri=0;pri<=RTK_DOT1P_PRIORITY_MAX;pri++)
    {
        if ((retVal =  rtl8370_getAsicPriorityToQIDMappingTable(queue_num-1,pri,&pPri2qid->pri2queue[pri]))!=RT_ERR_OK)
            return retVal;        
    }
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_schedulingQueue_set
 * Description:
 *      Set weight and type of queues in dedicated port.
 * Input:
 *      port - Port id.
 *      pQweights - The array of weights for WRR/WFQ queue (0 for STRICT_PRIORITY queue).
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_QOS_QUEUE_WEIGHT - Invalid queue weight. 
 * Note:
 *      The API can set weight and type, strict priority or weight fair queue (WFQ) for 
 *      dedicated port for using queues. If queue id is not included in queue usage, 
 *      then its type and weight setting in dummy for setting. There are priorities 
 *      as queue id in strict queues. It means strict queue id 5 carrying higher priority 
 *      than strict queue id 4. The WFQ queue weight is from 1 to 128, and weight 0 is 
 *      for strict priority queue type.
 */
rtk_api_ret_t rtk_qos_schedulingQueue_set(rtk_port_t port,rtk_qos_queue_weights_t *pQweights)
{
    rtk_api_ret_t retVal;
    uint32 qid;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_FAILED;


    for (qid = 0;qid<RTK_MAX_NUM_OF_QUEUE;qid ++)
    {

        if (pQweights->weights[qid] > QOS_WEIGHT_MAX)
            return RT_ERR_QOS_QUEUE_WEIGHT;

        if (0 == pQweights->weights[qid])
        {
            if ((retVal = rtl8370_setAsicQueueType(port, qid, QTYPE_STRICT))!=RT_ERR_OK)
                return retVal;
        }
        else
        {
            if ((retVal = rtl8370_setAsicQueueType(port, qid, QTYPE_WFQ))!=RT_ERR_OK)
                return retVal;
        
            if ((retVal = rtl8370_setAsicWFQWeight(port,qid,pQweights->weights[qid]))!=RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_schedulingQueue_get
 * Description:
 *      Get weight and type of queues in dedicated port.
 * Input:
 *      port - Port id.
 * Output:
 *      pQweights - The array of weights for WRR/WFQ queue (0 for STRICT_PRIORITY queue).
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get weight and type, strict priority or weight fair queue (WFQ) for dedicated port for using queues.
 *      The WFQ queue weight is from 1 to 128, and weight 0 is for strict priority queue type.
 */   
rtk_api_ret_t rtk_qos_schedulingQueue_get(rtk_port_t port, rtk_qos_queue_weights_t *pQweights)
{
    rtk_api_ret_t retVal;
    uint32 qid,qtype,qweight;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_FAILED;


    for (qid = 0;qid<RTK_MAX_NUM_OF_QUEUE;qid ++)
    {
        if ((retVal = rtl8370_getAsicQueueType(port, qid, &qtype))!=RT_ERR_OK)
            return retVal;

        if (QTYPE_STRICT == qtype)
           {
          pQweights->weights[qid]=0;
           }
        else if (QTYPE_WFQ == qtype)
        {
            if ((retVal = rtl8370_getAsicWFQWeight(port,qid,&qweight))!=RT_ERR_OK)
                return retVal;
            pQweights->weights[qid]=qweight;
        }
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_1pRemarkEnable_set
 * Description:
 *      Set weight and type of queues in dedicated port.
 * Input:
 *      port - Port id.
 *      enable - Status of 802.1p remark.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API can enable or disable 802.1p remarking ability for whole system. 
 *      The status of 802.1p remark:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_qos_1pRemarkEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /*for whole system function, the port value should be 0xFF*/
    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;
    
    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;    

    if ((retVal = rtl8370_setAsicRemarkingDot1pAbility(enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_1pRemarkEnable_get
 * Description:
 *      Get 802.1p remarking ability. 
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Status of 802.1p remark.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get 802.1p remarking ability.
 *      The status of 802.1p remark:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_qos_1pRemarkEnable_get(rtk_port_t port, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;

    /*for whole system function, the port value should be 0xFF*/
    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicRemarkingDot1pAbility(pEnable))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_1pRemark_set
 * Description:
 *      Set 802.1p remarking parameter.
 * Input:
 *      int_pri - Internal priority value.
 *      dot1p_pri - 802.1p priority value.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_VLAN_PRIORITY - Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The API can set 802.1p parameters source priority and new priority.
 */
rtk_api_ret_t rtk_qos_1pRemark_set(rtk_pri_t int_pri, rtk_pri_t dot1p_pri)
{
    rtk_api_ret_t retVal;

    if (int_pri > RTK_DOT1P_PRIORITY_MAX )
        return RT_ERR_QOS_INT_PRIORITY; 

    if (dot1p_pri > RTK_DOT1P_PRIORITY_MAX)
        return RT_ERR_VLAN_PRIORITY; 

    if ((retVal = rtl8370_setAsicRemarkingDot1pParameter(int_pri, dot1p_pri))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_1pRemark_get
 * Description:
 *      Get 802.1p remarking parameter.
 * Input:
 *      int_pri - Internal priority value.
 * Output:
 *      pDot1p_pri - 802.1p priority value.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority. 
 * Note:
 *      The API can get 802.1p remarking parameters. It would return new priority of ingress priority. 
 */
rtk_api_ret_t rtk_qos_1pRemark_get(rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri)
{
    rtk_api_ret_t retVal;
    
    if (int_pri > RTK_DOT1P_PRIORITY_MAX )
        return RT_ERR_QOS_INT_PRIORITY; 

    if ((retVal = rtl8370_getAsicRemarkingDot1pParameter(int_pri, pDot1p_pri))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_dscpRemarkEnable_set
 * Description:
 *      Set DSCP remarking ability.
 * Input:
 *      port - Port id.
 *      enable - status of DSCP remark.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API can enable or disable DSCP remarking ability for whole system.
 *      The status of DSCP remark:
 *      DISABLED
 *      ENABLED 
 */
rtk_api_ret_t rtk_qos_dscpRemarkEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    
    /*for whole system function, the port value should be 0xFF*/
    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_setAsicRemarkingDscpAbility(enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_dscpRemarkEnable_get
 * Description:
 *      Get DSCP remarking ability.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - status of DSCP remarking.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get DSCP remarking ability.
 *      The status of DSCP remark:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_qos_dscpRemarkEnable_get(rtk_port_t port, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;

    /*for whole system function, the port value should be 0xFF*/
    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicRemarkingDscpAbility(pEnable))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_qos_dscpRemark_set
 * Description:
 *      Set DSCP remarking parameter.
 * Input:
 *      int_pri - Internal priority value.
 *      dscp - DSCP value.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority. 
 *      RT_ERR_QOS_DSCP_VALUE - Invalid DSCP value. 
 * Note:
 *      The API can set DSCP value and mapping priority.
 */
rtk_api_ret_t rtk_qos_dscpRemark_set(rtk_pri_t int_pri, rtk_dscp_t dscp)
{
    rtk_api_ret_t retVal;

    if (int_pri > RTK_DOT1P_PRIORITY_MAX )
        return RT_ERR_QOS_INT_PRIORITY; 

    if (dscp > RTK_VALUE_OF_DSCP_MAX)
        return RT_ERR_QOS_DSCP_VALUE;     

    if ((retVal = rtl8370_setAsicRemarkingDscpParameter(int_pri, dscp))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_qos_dscpRemark_get
 * Description:
 *      Get DSCP remarking parameter.
 * Input:
 *      int_pri - Internal priority value.
 * Output:
 *      Dscp |DSCP value.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority. 
 * Note:
 *      The API can get DSCP parameters. It would return DSCP value for mapping priority.
 */
rtk_api_ret_t rtk_qos_dscpRemark_get(rtk_pri_t int_pri, rtk_dscp_t *pDscp)
{
    rtk_api_ret_t retVal;

    if (int_pri > RTK_DOT1P_PRIORITY_MAX )
        return RT_ERR_QOS_INT_PRIORITY; 

    if ((retVal = rtl8370_getAsicRemarkingDscpParameter(int_pri, pDscp))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unknownUnicastPktAction_set
 * Description:
 *      Set unknown unicast packet action configuration.
 * Input:
 *      type - Unknown unicast packet type.
 *      ucast_action - Unknown unicast action. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_NOT_ALLOWED - Invalid action.
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can set unknown unicast packet action configuration.
 *      The unknown unicast packet type is as following:
 *      UCAST_UNKNOWNDA
 *      UCAST_UNKNOWNSA
 *      UCAST_UNMATCHSA
 *      The unknown unicast action is as following:
 *      UCAST_ACTION_FORWARD
 *      UCAST_ACTION_DROP
 *      UCAST_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_trap_unknownUnicastPktAction_set(rtk_trap_ucast_type_t type, rtk_trap_ucast_action_t ucast_action)
{
    rtk_api_ret_t retVal;

    if (type >= UCAST_END)
        return RT_ERR_INPUT; 

    if (ucast_action >= UCAST_ACTION_END)
        return RT_ERR_INPUT; 
    

    switch (type)
    {
        case UCAST_UNKNOWNDA:
            if ((retVal = rtl8370_setAsicPortUnknownDaBehavior(ucast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        case UCAST_UNKNOWNSA:
            if ((retVal = rtl8370_setAsicPortUnknownSaBehavior(ucast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        case UCAST_UNMATCHSA:
            if ((retVal = rtl8370_setAsicPortUnmatchedSaBehavior(ucast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        default:
            break;            
    }
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unknownUnicastPktAction_get
 * Description:
 *      Get unknown unicast packet action configuration.
 * Input:
 *      type - unknown unicast packet type.
 * Output:
 *      pUcast_action - unknown unicast action. 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can get unknown unicast packet action configuration.
 *      The unknown unicast packet type is as following:
 *      UCAST_UNKNOWNDA
 *      UCAST_UNKNOWNSA
 *      UCAST_UNMATCHSA
 *      The unknown unicast action is as following:
 *      UCAST_ACTION_FORWARD
 *      UCAST_ACTION_DROP
 *      UCAST_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_trap_unknownUnicastPktAction_get(rtk_trap_ucast_type_t type, rtk_data_t *pUcast_action)
{
    rtk_api_ret_t retVal;

    if (type >= UCAST_END)
        return RT_ERR_INPUT; 

    switch (type)
    {
        case UCAST_UNKNOWNDA:
            if ((retVal = rtl8370_getAsicPortUnknownDaBehavior(pUcast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        case UCAST_UNKNOWNSA:
            if ((retVal = rtl8370_getAsicPortUnknownSaBehavior((uint32*)pUcast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        case UCAST_UNMATCHSA:
            if ((retVal = rtl8370_getAsicPortUnmatchedSaBehavior((uint32*)pUcast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        default:
            break;            
    }
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_trap_rmaAction_set
 * Description:
 *      Set reserved multicast address frame trap to CPU.
 * Input:
 *      pRma_frame - Reserved multicast address.
 *      rma_action - RMA action. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_NOT_ALLOWED - Invalid action.
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      There are 48 types of Reserved Multicast Address frame for application usage. They are as following definition.
 *      DMAC                        Assignment
 *      01-80-C2-00-00-00           Bridge Group Address
 *      01-80-C2-00-00-01           IEEE Std 802.3, 1988 Edition Full Duplex PAUSE operation
 *      01-80-C2-00-00-02           IEEE Std 802.3ad Slow Protocols-Multicast Address
 *      01-80-C2-00-00-03           IEEE Std 802.1X PAE address
 *      01-80-C2-00-00-04           Undefined 802.1 bridge address 04
 *      01-80-C2-00-00-05           Undefined 802.1 bridge address 05
 *      01-80-C2-00-00-06           Undefined 802.1 bridge address 06
 *      01-80-C2-00-00-07           Undefined 802.1 bridge address 07
 *      01-80-C2-00-00-08           Provider Bridge Group Address
 *      01-80-C2-00-00-09           Undefined 802.1 bridge address 09
 *      01-80-C2-00-00-0A           Undefined 802.1 bridge address 0A
 *      01-80-C2-00-00-0B           Undefined 802.1 bridge address 0B
 *      01-80-C2-00-00-0C           Undefined 802.1 bridge address 0C
 *      01-80-C2-00-00-0D           Provider Bridge GVRP Address
 *      01-80-C2-00-00-0E           IEEE Std 802.1ab Link Layer Discovery Protocol Multicast address
 *      01-80-C2-00-00-0F           Undefined 802.1 bridge address
 *      01-80-C2-00-00-10           All LANs Bridge Management Group Address
 *      01-80-C2-00-00-11~1F        Undefined address 11~1F
 *      01-80-C2-00-00-20           GMRP Address
 *      01-80-C2-00-00-21           GVRP address
 *      01-80-C2-00-00-22~2F        Undefined GARP address 22~2F
 *      The RMA action is as following:
 *      RMA_ACTION_FORWARD
 *      RMA_ACTION_TRAP2CPU
 *      RMA_ACTION_DROP
 *      RMA_ACTION_FORWARD_EXCLUDE_CPU
 */
rtk_api_ret_t rtk_trap_rmaAction_set(rtk_mac_t *pRma_frame, rtk_trap_rma_action_t rma_action)
{
    rtk_api_ret_t retVal;
    rtl8370_rma_t rmacfg;


    if (pRma_frame->octet[0]!=0x01&&pRma_frame->octet[1]!=0x80&&pRma_frame->octet[2]!=0xC2
        &&pRma_frame->octet[3]!=0&&pRma_frame->octet[4]!=0&&pRma_frame->octet[5]>0x2F)
        return RT_ERR_RMA_ADDR;

    if (rma_action >= RMA_ACTION_END)
        return RT_ERR_RMA_ACTION;

    if ((retVal = rtl8370_getAsicRma(pRma_frame->octet[5], &rmacfg))!=RT_ERR_OK)
        return retVal; 

    rmacfg.operation = rma_action;
    
    if ((retVal = rtl8370_setAsicRma(pRma_frame->octet[5], &rmacfg))!=RT_ERR_OK)
        return retVal;    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_rmaAction_get
 * Description:
 *      Get reserved multicast address frame trap to CPU.
 * Input:
 *      type - unknown unicast packet type.
 * Output:
 *      pRma_action - RMA action. 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can retrieved RMA configuration.
 *      DMAC                        Assignment
 *      01-80-C2-00-00-00           Bridge Group Address
 *      01-80-C2-00-00-01           IEEE Std 802.3, 1988 Edition Full Duplex PAUSE operation
 *      01-80-C2-00-00-02           IEEE Std 802.3ad Slow Protocols-Multicast Address
 *      01-80-C2-00-00-03           IEEE Std 802.1X PAE address
 *      01-80-C2-00-00-04           Undefined 802.1 bridge address 04
 *      01-80-C2-00-00-05           Undefined 802.1 bridge address 05
 *      01-80-C2-00-00-06           Undefined 802.1 bridge address 06
 *      01-80-C2-00-00-07           Undefined 802.1 bridge address 07
 *      01-80-C2-00-00-08           Provider Bridge Group Address
 *      01-80-C2-00-00-09           Undefined 802.1 bridge address 09
 *      01-80-C2-00-00-0A           Undefined 802.1 bridge address 0A
 *      01-80-C2-00-00-0B           Undefined 802.1 bridge address 0B
 *      01-80-C2-00-00-0C           Undefined 802.1 bridge address 0C
 *      01-80-C2-00-00-0D           Provider Bridge GVRP Address
 *      01-80-C2-00-00-0E           IEEE Std 802.1ab Link Layer Discovery Protocol Multicast address
 *      01-80-C2-00-00-0F           Undefined 802.1 bridge address
 *      01-80-C2-00-00-10           All LANs Bridge Management Group Address
 *      01-80-C2-00-00-11~1F        Undefined address 11~1F
 *      01-80-C2-00-00-20           GMRP Address
 *      01-80-C2-00-00-21           GVRP address
 *      01-80-C2-00-00-22~2F        Undefined GARP address 22~2F
 *      The RMA action is as following:
 *      RMA_ACTION_FORWARD
 *      RMA_ACTION_DROP
 *      RMA_ACTION_TRAP2CPU
 *      RMA_ACTION_FORWARD_EXCLUDE_CPU
 */
rtk_api_ret_t rtk_trap_rmaAction_get(rtk_mac_t *pRma_frame, rtk_data_t *pRma_action)
{
    rtk_api_ret_t retVal;
    rtl8370_rma_t rmacfg;

    if (pRma_frame->octet[0]!=0x01&&pRma_frame->octet[1]!=0x80&&pRma_frame->octet[2]!=0xC2
        &&pRma_frame->octet[3]!=0&&pRma_frame->octet[4]!=0&&pRma_frame->octet[5]>0x2F)
        return RT_ERR_RMA_ADDR;


    if ((retVal = rtl8370_getAsicRma(pRma_frame->octet[5], &rmacfg))!=RT_ERR_OK)
        return retVal; 

    *pRma_action = rmacfg.operation;       

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_set
 * Description:
 *      Set IGMP/MLD trap function
 * Input:
 *      type - IGMP/MLD packet type.
 *      igmp_action - IGMP/MLD action. 
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_NOT_ALLOWED - Invalid igmp action.
 * Note:
 *      This API can set both IPv4 IGMP/IPv6 MLD with/without PPPoE header trapping function.
 *      All 4 kinds of IGMP/MLD function can be set seperately.
 *      The IGMP/MLD packet type is as following:
 *      IGMP_IPV4
 *      IGMP_PPPOE_IPV4
 *      IGMP_MLD
 *      IGMP_PPPOE_MLD
 *      The IGMP/MLD action is as following:
 *      IGMP_ACTION_FORWARD
 *      IGMP_ACTION_TRAP2CPU
 *      IGMP_ACTION_DROP
 *      IGMP_ACTION_FORWARD_EXCLUDE_CPU
 */
rtk_api_ret_t rtk_trap_igmpCtrlPktAction_set(rtk_igmp_type_t type, rtk_trap_igmp_action_t igmp_action)
{
    rtk_api_ret_t retVal;
    rtl8370_igmp_t igmpcfg;

    if (type >= IGMP_TYPE_END)
        return RT_ERR_INPUT; 

    if (igmp_action >= IGMP_ACTION_END)
        return RT_ERR_NOT_ALLOWED; 
    

    if ((retVal = rtl8370_getAsicIgmp(&igmpcfg))!=RT_ERR_OK)
        return retVal; 

    switch (type)
    {
        case IGMP_IPV4:
            igmpcfg.igmp_trap=igmp_action;
            break;
        case IGMP_PPPOE_IPV4:
            igmpcfg.pppoe_igmp_trap=igmp_action; 
            break;
        case IGMP_MLD:
            igmpcfg.mld_trap=igmp_action; 
            break;
        case IGMP_PPPOE_MLD:
            igmpcfg.pppoe_mld_trap=igmp_action; 
            break;            
        default:
            break;            
    }

    if ((retVal = rtl8370_setAsicIgmp(&igmpcfg))!=RT_ERR_OK)
        return retVal; 
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_get
 * Description:
 *      Get IGMP/MLD trap function
 * Input:
 *      type - IGMP/MLD packet type.
 * Output:
 *      pIgmp_action - IGMP/MLD action. 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can get both IPv4 IGMP/IPv6 MLD with/without PPPoE header trapping function.
 *      The IGMP/MLD packet type is as following:
 *      IGMP_IPV4
 *      IGMP_PPPOE_IPV4
 *      IGMP_MLD
 *      IGMP_PPPOE_MLD
 *      The IGMP/MLD action is as following:
 *      IGMP_ACTION_FORWARD
 *      IGMP_ACTION_DROP
 *      IGMP_ACTION_TRAP2CPU
 *      IGMP_ACTION_FORWARD_EXCLUDE_CPU
 */
rtk_api_ret_t rtk_trap_igmpCtrlPktAction_get(rtk_igmp_type_t type, rtk_data_t *pIgmp_action)
{
    rtk_api_ret_t retVal;
    rtl8370_igmp_t igmpcfg;

    if (type >= IGMP_TYPE_END)
        return RT_ERR_INPUT;     

    if ((retVal = rtl8370_getAsicIgmp(&igmpcfg))!=RT_ERR_OK)
        return retVal; 

    switch (type)
    {
        case IGMP_IPV4:
            *pIgmp_action=igmpcfg.igmp_trap;
            break;
        case IGMP_PPPOE_IPV4:
            *pIgmp_action=igmpcfg.pppoe_igmp_trap; 
            break;
        case IGMP_MLD:
            *pIgmp_action=igmpcfg.mld_trap; 
            break;
        case IGMP_PPPOE_MLD:
            *pIgmp_action=igmpcfg.pppoe_mld_trap; 
            break;            
        default:
            break;            
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unknownMcastPktAction_set
 * Description:
 *      Set behavior of unknown multicast
 * Input:
 *      port - Port id.
 *      type - unknown multicast packet type.
 *      mcast_action - unknown multicast action. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_NOT_ALLOWED - Invalid action.
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop or flood this packet
 *      The unknown multicast packet type is as following:
 *      MCAST_L2
 *      MCAST_IPV4
 *      MCAST_IPV6
 *      The unknown multicast action is as following:
 *      MCAST_ACTION_FORWARD
 *      MCAST_ACTION_DROP
 *      MCAST_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_trap_unknownMcastPktAction_set(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t mcast_action)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 

    if (type >= MCAST_END)
        return RT_ERR_INPUT; 

    if (mcast_action >= MCAST_ACTION_END)
        return RT_ERR_INPUT; 
    

    switch (type)
    {
        case MCAST_L2:
            if ((retVal = rtl8370_setAsicUnknownL2MulticastBehavior(port,mcast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        case MCAST_IPV4:
            if ((retVal = rtl8370_setAsicUnknownIPv4MulticastBehavior(port,mcast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        case MCAST_IPV6:
            if ((retVal = rtl8370_setAsicUnknownIPv6MulticastBehavior(port,mcast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        default:
            break;            
    }
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unknownMcastPktAction_get
 * Description:
 *      Get behavior of unknown multicast
 * Input:
 *      port - Port id.
 *      type - Unknown multicast packet type.
 * Output:
 *      pMcast_action - unknown multicast action. 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_NOT_ALLOWED - Invalid operation.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop or flood this packet
 *      The unknown multicast packet type is as following:
 *      MCAST_L2
 *      MCAST_IPV4
 *      MCAST_IPV6
 *      The unknown multicast action is as following:
 *      MCAST_ACTION_FORWARD
 *      MCAST_ACTION_DROP
 *      MCAST_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_trap_unknownMcastPktAction_get(rtk_port_t port, rtk_mcast_type_t type, rtk_data_t *pMcast_action)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 

    if (type >= MCAST_END)
        return RT_ERR_INPUT; 
    
    switch (type)
    {
        case MCAST_L2:
            if ((retVal = rtl8370_getAsicUnknownL2MulticastBehavior(port, pMcast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        case MCAST_IPV4:
            if ((retVal = rtl8370_getAsicUnknownIPv4MulticastBehavior(port, pMcast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        case MCAST_IPV6:
            if ((retVal = rtl8370_getAsicUnknownIPv6MulticastBehavior(port,(uint32*)pMcast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        default:
            break;            
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_ethernetAv_set
 * Description:
 *      Set Ethetnet AV.
 * Input:
 *      enable - enable trap
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API can enable or disable ethernet AV function. If the function is enabled,
 *      packets with ethernet type 0x88F7 will be trap to CPU with time stamp.
 *      The status of Ethernet AV:
 *      DISABLED
 *      ENABLED 
 */
rtk_api_ret_t rtk_trap_ethernetAv_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    
    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_setAsicEthernetAv(enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_ethernetAv_get
 * Description:
 *      Get ethernet AV setup.
 * Input:
 * Output:
 *      pEnable - status of ethernet AV.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get ethernet AV status. If the function is enabled,
 *      packets with ethernet type 0x88F7 will be trap to CPU with time stamp.
 *      The status of Ethernet AV:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_trap_ethernetAv_get(rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicEthernetAv(pEnable))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

#endif


/* Function Name:
 *      rtk_storm_controlRate_set
 * Description:
 *      Set per-port storm filter control rate.
 * Input:
 *      port - Port id
 *      storm_type - Storm filter control type
 *      mode - Storm filter control mode.
 *      rate - Rate of storm filter control(mode 0), shared meter index(mode 1) 
 *      ifg_include - include IFG or not(mode 0), storm control enable(mode 1)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_SFC_UNKNOWN_GROUP - unknown storm filter group.
 *      RT_ERR_ENABLE - Invalid IFG parameter
 *      RT_ERR_RATE - Invalid rate
 * Note:
 *      This API can set per-port stomr filter control rate. 
 *      The storm filter control type can be:
 *      STORM_GROUP_UNKNOWN_UNICAST 
 *      STORM_GROUP_UNKNOWN_MULTICAST 
 *      STORM_GROUP_MULTICAST
 *      STORM_GROUP_BROADCAST
 *      The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps. 
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 *      In mode 0:
 *      Use rate to assign storm control rate.
 *      Use ifg_include to control inter-frame-gap include or not.
 *      In mode 1:
 *      Use rate to assign storm control shared meter index.
 *      Use ifg_include to be storm control enable/disable parameter.  
 */    
rtk_api_ret_t rtk_storm_controlRate_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t rate, rtk_enable_t ifg_include, rtk_mode_t mode)
{
    rtk_api_ret_t retVal;
    uint32 enable;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if (storm_type>=STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;
    
    if (mode >= MODE_END)
        return RT_ERR_INPUT;

    if (ifg_include >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (mode == MODE0)
    {
        if (rate>RTK_MAX_INPUT_RATE || rate<RTK_MIN_INPUT_RATE ||rate%RTK_RATE_GRANULARTY_UNIT )
            return RT_ERR_RATE;
    
        if (RTK_MAX_INPUT_RATE == rate)
        enable=FALSE;
    else
        enable=TRUE;

    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8370_setAsicStormFilterUnknownUnicastEnable(port,enable))!=RT_ERR_OK)
                return retVal; 
            if (enable)
            {
                if ((retVal = rtl8370_setAsicStormFilterUnknownUnicastMeter(port,STORM_UNUC_INDEX))!=RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_setAsicShareMeter(STORM_UNUC_INDEX,rate>>3,ifg_include))!=RT_ERR_OK)
                    return retVal;
            }
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8370_setAsicStormFilterUnknownMulticastEnable(port,enable))!=RT_ERR_OK)
                return retVal; 
            if (enable)
            {
                if ((retVal = rtl8370_setAsicStormFilterUnknownMulticastMeter(port,STORM_UNMC_INDEX))!=RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_setAsicShareMeter(STORM_UNMC_INDEX,rate>>3,ifg_include))!=RT_ERR_OK)
                    return retVal;
            }
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8370_setAsicStormFilterMulticastEnable(port,enable))!=RT_ERR_OK)
                return retVal; 
            if (enable) 
            {
                if ((retVal = rtl8370_setAsicStormFilterMulticastMeter(port,STORM_MC_INDEX))!=RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_setAsicShareMeter(STORM_MC_INDEX,rate>>3,ifg_include))!=RT_ERR_OK)
                    return retVal;
            }
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8370_setAsicStormFilterBroadcastEnable(port,enable))!=RT_ERR_OK)
                return retVal; 
            if (enable) 
            {
                if ((retVal = rtl8370_setAsicStormFilterBroadcastMeter(port,STORM_BC_INDEX))!=RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_setAsicShareMeter(STORM_BC_INDEX,rate>>3,ifg_include))!=RT_ERR_OK)
                    return retVal;
            }
            default:
                break;      
        }
    }
    else if (mode == MODE1)
    {  
        /*Use rate to assign storm control shared meter index in mode 1.*/
        if (rate >= RTK_MAX_NUM_OF_METER)
            return RT_ERR_FILTER_METER_ID;

        /*Use ifg_include to be storm control enable/disable parameter in mode 1.*/
        enable = ifg_include;
    
        switch (storm_type)
        {
            case STORM_GROUP_UNKNOWN_UNICAST:
                if ((retVal = rtl8370_setAsicStormFilterUnknownUnicastEnable(port,enable))!=RT_ERR_OK)
                    return retVal; 
                if ((retVal = rtl8370_setAsicStormFilterUnknownUnicastMeter(port,rate))!=RT_ERR_OK)
                    return retVal;
            break;
            case STORM_GROUP_UNKNOWN_MULTICAST:
                if ((retVal = rtl8370_setAsicStormFilterUnknownMulticastEnable(port,enable))!=RT_ERR_OK)
                    return retVal; 
                if ((retVal = rtl8370_setAsicStormFilterUnknownMulticastMeter(port,rate))!=RT_ERR_OK)
                    return retVal;
                break;
            case STORM_GROUP_MULTICAST:
                if ((retVal = rtl8370_setAsicStormFilterMulticastEnable(port,enable))!=RT_ERR_OK)
                    return retVal; 
                if ((retVal = rtl8370_setAsicStormFilterMulticastMeter(port,rate))!=RT_ERR_OK)
                    return retVal;
                break;
            case STORM_GROUP_BROADCAST:
                if ((retVal = rtl8370_setAsicStormFilterBroadcastEnable(port,enable))!=RT_ERR_OK)
                    return retVal; 
                if ((retVal = rtl8370_setAsicStormFilterBroadcastMeter(port,rate))!=RT_ERR_OK)
                    return retVal;
            default:
                break;             
        }
    }  
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_storm_controlRate_get
 * Description:
 *      Get per-port packet storm filter control rate.
 * Input:
 *      port - Port id
 *      storm_type - Storm filter control type.
 *      mode - Storm filter control mode.
 * Output:
 *      pRate - Rate of storm filter control.
 *      pIfg_include - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_SFC_UNKNOWN_GROUP - unknown storm filter group.
 * Note:
 *      The storm filter control type can be:
 *      STORM_GROUP_UNKNOWN_UNICAST 
 *      STORM_GROUP_UNKNOWN_MULTICAST 
 *      STORM_GROUP_MULTICAST
 *      STORM_GROUP_BROADCAST
 *      In mode 0:
 *      pRate is assigned to get storm control rate.
 *      pIfg_include is assigned to get inter-frame-gap include or not.
 *      In mode 1:
 *      pRate is assigned to get storm control shared meter index.
 *      pIfg_include is assigned to get storm control enable/disable parameter.  
 */
rtk_api_ret_t rtk_storm_controlRate_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t *pRate, rtk_data_t *pIfg_include, rtk_mode_t mode)
{
    rtk_api_ret_t retVal;
    uint32 enable;
    uint32 index;
    uint32 regData;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if (storm_type>=STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (mode >= MODE_END)
        return RT_ERR_INPUT;

    if (mode == MODE0)
    {
    switch (storm_type)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8370_getAsicStormFilterUnknownUnicastEnable(port,&enable))!=RT_ERR_OK)
                return retVal; 
            if (enable)
            {
                if ((retVal = rtl8370_getAsicStormFilterUnknownUnicastMeter(port,&index))!=RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_getAsicShareMeter(index, &regData, pIfg_include))!=RT_ERR_OK)
                    return retVal;
                *pRate = regData<<3;
            }
            else
            {
                *pRate = 0x1FFFF<<3;
            }
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8370_getAsicStormFilterUnknownMulticastEnable(port,&enable))!=RT_ERR_OK)
                return retVal; 
            if (enable)
            {
                if ((retVal = rtl8370_getAsicStormFilterUnknownMulticastMeter(port,&index))!=RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_getAsicShareMeter(index, &regData, pIfg_include))!=RT_ERR_OK)
                    return retVal;
                *pRate = regData<<3;
            }
            else
            {
                *pRate = 0x1FFFF<<3;
            }
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8370_getAsicStormFilterMulticastEnable(port,&enable))!=RT_ERR_OK)
                return retVal; 
            if (enable)
            {
                if ((retVal = rtl8370_getAsicStormFilterMulticastMeter(port,&index))!=RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_getAsicShareMeter(index, &regData, pIfg_include))!=RT_ERR_OK)
                    return retVal;
                *pRate = regData<<3;
            }
            else
            {
                *pRate = 0x1FFFF<<3;
            }
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8370_getAsicStormFilterBroadcastEnable(port,&enable))!=RT_ERR_OK)
                return retVal; 
            if (enable)
            {
                if ((retVal = rtl8370_getAsicStormFilterBroadcastMeter(port,&index))!=RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_getAsicShareMeter(index, &regData, pIfg_include))!=RT_ERR_OK)
                    return retVal;
                *pRate = regData<<3;
            }
            else
            {
                *pRate = 0x1FFFF<<3;
            }
            break;
        default:
            break;    
    }
    }
    else if (mode == MODE1)
    {
        /*Use pRate to assign storm control shared meter index in mode 1.*/
        /*Use pIfg_include to be storm control enable/disable parameter in mode 1.*/   
        switch (storm_type)
        {
            case STORM_GROUP_UNKNOWN_UNICAST:
                if ((retVal = rtl8370_getAsicStormFilterUnknownUnicastEnable(port,pIfg_include))!=RT_ERR_OK)
                    return retVal; 
                if ((retVal = rtl8370_getAsicStormFilterUnknownUnicastMeter(port,pRate))!=RT_ERR_OK)
                    return retVal;
                break;
            case STORM_GROUP_UNKNOWN_MULTICAST:
                if ((retVal = rtl8370_getAsicStormFilterUnknownMulticastEnable(port,pIfg_include))!=RT_ERR_OK)
                    return retVal; 
                if ((retVal = rtl8370_getAsicStormFilterUnknownMulticastMeter(port,pRate))!=RT_ERR_OK)
                    return retVal;
                break;
            case STORM_GROUP_MULTICAST:
                if ((retVal = rtl8370_getAsicStormFilterMulticastEnable(port,pIfg_include))!=RT_ERR_OK)
                    return retVal; 
                if ((retVal = rtl8370_getAsicStormFilterMulticastMeter(port,pRate))!=RT_ERR_OK)
                    return retVal;
                break;
            case STORM_GROUP_BROADCAST:
                if ((retVal = rtl8370_getAsicStormFilterBroadcastEnable(port,pIfg_include))!=RT_ERR_OK)
                    return retVal; 
                if ((retVal = rtl8370_getAsicStormFilterBroadcastMeter(port,pRate))!=RT_ERR_OK)
                    return retVal;
                break;
            default:
                break;    
        }
    }

    return RT_ERR_OK;
}

#if !defined(_REDUCE_CODE)
/* Function Name:
 *      rtk_storm_bypass_set
 * Description:
 *      Set bypass storm filter control configuration.
 * Input:
 *      type - Bypass storm filter control type.
 *      enable - Bypass status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_ENABLE - Invalid IFG parameter
 * Note:
 *      
 *      This API can set per-port bypass stomr filter control frame type including RMA and igmp.
 *      The bypass frame type is as following:
 *      BYPASS_BRG_GROUP,
 *      BYPASS_FD_PAUSE,
 *      BYPASS_SP_MCAST,
 *      BYPASS_1X_PAE,
 *      BYPASS_UNDEF_BRG_04,
 *      BYPASS_UNDEF_BRG_05,
 *      BYPASS_UNDEF_BRG_06,
 *      BYPASS_UNDEF_BRG_07,
 *      BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      BYPASS_UNDEF_BRG_09,
 *      BYPASS_UNDEF_BRG_0A,
 *      BYPASS_UNDEF_BRG_0B,
 *      BYPASS_UNDEF_BRG_0C,
 *      BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      BYPASS_8021AB,
 *      BYPASS_UNDEF_BRG_0F,
 *      BYPASS_BRG_MNGEMENT,
 *      BYPASS_UNDEFINED_11,
 *      BYPASS_UNDEFINED_12,
 *      BYPASS_UNDEFINED_13,
 *      BYPASS_UNDEFINED_14,
 *      BYPASS_UNDEFINED_15,
 *      BYPASS_UNDEFINED_16,
 *      BYPASS_UNDEFINED_17,
 *      BYPASS_UNDEFINED_18,
 *      BYPASS_UNDEFINED_19,
 *      BYPASS_UNDEFINED_1A,
 *      BYPASS_UNDEFINED_1B,
 *      BYPASS_UNDEFINED_1C,
 *      BYPASS_UNDEFINED_1D,
 *      BYPASS_UNDEFINED_1E,
 *      BYPASS_UNDEFINED_1F,
 *      BYPASS_GMRP,
 *      BYPASS_GVRP,
 *      BYPASS_UNDEF_GARP_22,
 *      BYPASS_UNDEF_GARP_23,
 *      BYPASS_UNDEF_GARP_24,
 *      BYPASS_UNDEF_GARP_25,
 *      BYPASS_UNDEF_GARP_26,
 *      BYPASS_UNDEF_GARP_27,
 *      BYPASS_UNDEF_GARP_28,
 *      BYPASS_UNDEF_GARP_29,
 *      BYPASS_UNDEF_GARP_2A,
 *      BYPASS_UNDEF_GARP_2B,
 *      BYPASS_UNDEF_GARP_2C,
 *      BYPASS_UNDEF_GARP_2D,
 *      BYPASS_UNDEF_GARP_2E,
 *      BYPASS_UNDEF_GARP_2F,
 *      BYPASS_IGMP.
*/ 

rtk_api_ret_t rtk_storm_bypass_set(rtk_storm_bypass_t type, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtl8370_rma_t rmacfg;
    rtl8370_igmp_t igmpcfg;

    if (type>=BYPASS_END)
        return RT_ERR_INPUT;

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;        
    
    if (type>=0&&type<=BYPASS_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8370_getAsicRma(type, &rmacfg))!=RT_ERR_OK)
            return retVal; 

        rmacfg.discard_storm_filter = enable;
        
        if ((retVal = rtl8370_setAsicRma(type, &rmacfg))!=RT_ERR_OK)
            return retVal;
    }
    else if (BYPASS_IGMP == type)
    {
        if ((retVal = rtl8370_getAsicIgmp(&igmpcfg))!=RT_ERR_OK)
            return retVal;
        
        igmpcfg.discard_storm_filter = enable;

        if ((retVal = rtl8370_setAsicIgmp(&igmpcfg))!=RT_ERR_OK)
            return retVal;
    }
    else
        return RT_ERR_INPUT;        

    return RT_ERR_OK;
}
/* Function Name:
 *      rtk_storm_bypass_get
 * Description:
 *      Get bypass storm filter control configuration.
 * Input:
 *      type - Bypass storm filter control type.
 * Output:
 *      enable - Bypass status. 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can get per-port bypass stomr filter control frame type including RMA and igmp.
 *      The bypass frame type is as following:
 *      BYPASS_BRG_GROUP,
 *      BYPASS_FD_PAUSE,
 *      BYPASS_SP_MCAST,
 *      BYPASS_1X_PAE,
 *      BYPASS_UNDEF_BRG_04,
 *      BYPASS_UNDEF_BRG_05,
 *      BYPASS_UNDEF_BRG_06,
 *      BYPASS_UNDEF_BRG_07,
 *      BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      BYPASS_UNDEF_BRG_09,
 *      BYPASS_UNDEF_BRG_0A,
 *      BYPASS_UNDEF_BRG_0B,
 *      BYPASS_UNDEF_BRG_0C,
 *      BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      BYPASS_8021AB,
 *      BYPASS_UNDEF_BRG_0F,
 *      BYPASS_BRG_MNGEMENT,
 *      BYPASS_UNDEFINED_11,
 *      BYPASS_UNDEFINED_12,
 *      BYPASS_UNDEFINED_13,
 *      BYPASS_UNDEFINED_14,
 *      BYPASS_UNDEFINED_15,
 *      BYPASS_UNDEFINED_16,
 *      BYPASS_UNDEFINED_17,
 *      BYPASS_UNDEFINED_18,
 *      BYPASS_UNDEFINED_19,
 *      BYPASS_UNDEFINED_1A,
 *      BYPASS_UNDEFINED_1B,
 *      BYPASS_UNDEFINED_1C,
 *      BYPASS_UNDEFINED_1D,
 *      BYPASS_UNDEFINED_1E,
 *      BYPASS_UNDEFINED_1F,
 *      BYPASS_GMRP,
 *      BYPASS_GVRP,
 *      BYPASS_UNDEF_GARP_22,
 *      BYPASS_UNDEF_GARP_23,
 *      BYPASS_UNDEF_GARP_24,
 *      BYPASS_UNDEF_GARP_25,
 *      BYPASS_UNDEF_GARP_26,
 *      BYPASS_UNDEF_GARP_27,
 *      BYPASS_UNDEF_GARP_28,
 *      BYPASS_UNDEF_GARP_29,
 *      BYPASS_UNDEF_GARP_2A,
 *      BYPASS_UNDEF_GARP_2B,
 *      BYPASS_UNDEF_GARP_2C,
 *      BYPASS_UNDEF_GARP_2D,
 *      BYPASS_UNDEF_GARP_2E,
 *      BYPASS_UNDEF_GARP_2F,
 *      BYPASS_IGMP. 
 */
rtk_api_ret_t rtk_storm_bypass_get(rtk_storm_bypass_t type, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtl8370_rma_t rmacfg;
    rtl8370_igmp_t igmpcfg;

    if (type>=BYPASS_END)
        return RT_ERR_INPUT;    
    
    if (type>=0&&type<=BYPASS_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8370_getAsicRma(type, &rmacfg))!=RT_ERR_OK)
            return retVal; 

        *pEnable = rmacfg.discard_storm_filter;

    }
    else if (BYPASS_IGMP == type)
    {
        if ((retVal = rtl8370_getAsicIgmp(&igmpcfg))!=RT_ERR_OK)
            return retVal;
        
        *pEnable = igmpcfg.discard_storm_filter;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}

#endif

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set ethernet PHY auto-negotiation desired ability.
 * Input:
 *      port - port id.
 *      pAbility - Ability structure
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_PHY_REG_ID - Invalid PHY address
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1. While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 */
rtk_api_ret_t rtk_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t retVal;
    uint32 phyData;
    uint32 phyEnMsk0;
    uint32 phyEnMsk4;
    uint32 phyEnMsk9;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    /*for PHY auto mode setup*/
    pAbility->AutoNegotiation = 1;

    phyEnMsk0 = 0;
    phyEnMsk4 = 0;
    phyEnMsk9 = 0;

    if (pAbility->Half_10)
    {
        /*10BASE-TX half duplex capable in reg 4.5*/
        phyEnMsk4 |= (1<<5);
    }

    if (pAbility->Full_10)
    {
        /*10BASE-TX full duplex capable in reg 4.6*/
        phyEnMsk4 |= (1<<6);
    }

    if (pAbility->Half_100)
    {
        /*100BASE-TX half duplex capable in reg 4.7*/
        phyEnMsk4 |= (1<<7);
    }

    if (pAbility->Full_100)
    {
        /*100BASE-TX full duplex capable in reg 4.8*/
        phyEnMsk4 |= (1<<8);
    }

    if (pAbility->Full_1000)
    {
        /*1000 BASE-T FULL duplex capable setting in reg 9.9*/
        phyEnMsk9 |= (1<<9);
    }

    if (pAbility->Half_100 || pAbility->Full_100)
    {
        /*Speed selection [1:0] */
        /* 01 = 100Mpbs*/
        phyEnMsk0 |= (1 << 13);
    }

    if (pAbility->Full_100 || pAbility->Full_10)
    {
        /*Full duplex mode in reg 0.8*/
        phyEnMsk0 |= (1<<8);
    }

    if (pAbility->AsyFC)
    {
        /*Asymetric flow control in reg 4.11*/
        phyEnMsk4 |= (1<<11);
    }

    if (pAbility->FC)
    {
        /*Flow control in reg 4.10*/
        phyEnMsk4 |= (1<<10);
    }

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;

    /*1000 BASE-T control register setting*/
    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_1000_BASET_CONTROL_REG,&phyData))!=RT_ERR_OK)
        return retVal;

    phyData &= ~(0x0200);
    phyData |= phyEnMsk9;
    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_1000_BASET_CONTROL_REG,phyData))!=RT_ERR_OK)
        return retVal;

    /*Auto-Negotiation control register setting*/
    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_AN_ADVERTISEMENT_REG,&phyData))!=RT_ERR_OK)
        return retVal;

    phyData &= ~(0x0DE0);
    phyData |= phyEnMsk4;
    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_AN_ADVERTISEMENT_REG,phyData))!=RT_ERR_OK)
        return retVal;

    /*Control register setting and restart auto*/
    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_CONTROL_REG,&phyData))!=RT_ERR_OK)
        return retVal;

    /*Enable and restart auto negotiation*/
    phyData &= ~(0x3940);
    phyData |= phyEnMsk0 | (1<<12) | (1<<9);
    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_CONTROL_REG,phyData))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY ability through PHY registers.
 * Input:
 *      port - Port id.
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_PHY_REG_ID - Invalid PHY address
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get the capablity of specified PHY.
 */
rtk_api_ret_t rtk_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t retVal;
    uint32 phyData0;
    uint32 phyData4;
    uint32 phyData9;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;

    /*Control register setting and restart auto*/
    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_CONTROL_REG,&phyData0))!=RT_ERR_OK)
        return retVal;

    /*Auto-Negotiation control register setting*/
    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_AN_ADVERTISEMENT_REG,&phyData4))!=RT_ERR_OK)
        return retVal;

    /*1000 BASE-T control register setting*/
    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_1000_BASET_CONTROL_REG,&phyData9))!=RT_ERR_OK)
        return retVal;

    if (phyData9 & (1<<9))
        pAbility->Full_1000 = 1;
    else
        pAbility->Full_1000 = 0;

    if (phyData4 & (1<<11))
        pAbility->AsyFC = 1;
    else
        pAbility->AsyFC = 0;

    if (phyData4 & (1<<10))
        pAbility->FC = 1;
    else
        pAbility->FC = 0;

    if (phyData4 & (1<<8))
        pAbility->Full_100 = 1;
    else
        pAbility->Full_100 = 0;

    if (phyData4 & (1<<7))
        pAbility->Half_100 = 1;
    else
        pAbility->Half_100 = 0;

    if (phyData4 & (1<<6))
        pAbility->Full_10 = 1;
    else
        pAbility->Full_10 = 0;

    if (phyData4 & (1<<5))
        pAbility->Half_10 = 1;
    else
        pAbility->Half_10 = 0;

    if (phyData0 & (1<<12))
        pAbility->AutoNegotiation= 1;
    else
        pAbility->AutoNegotiation= 0;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyForceModeAbility_set
 * Description:
 *      Set the port speed/duplex mode/pause/asy_pause in the PHY force mode.
 * Input:
 *      port - port id.
 *      pAbility - Ability structure
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - set shared meter successfully
 *      RT_ERR_FAILED          - FAILED to iset shared meter
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_PHY_REG_ID - Invalid PHY address
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1. While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 */
rtk_api_ret_t rtk_port_phyForceModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t retVal;
    uint32 phyData;
    uint32 phyEnMsk0;
    uint32 phyEnMsk4;
    uint32 phyEnMsk9;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (pAbility->Full_1000)
        return RT_ERR_INPUT;

    /*for PHY force mode setup*/
    pAbility->AutoNegotiation = 0;

    phyEnMsk0 = 0;
    phyEnMsk4 = 0;
    phyEnMsk9 = 0;

    if (pAbility->Half_10)
    {
        /*10BASE-TX half duplex capable in reg 4.5*/
        phyEnMsk4 |= (1<<5);

        /*Speed selection [1:0] */
        /* 00 = 10Mpbs*/
    }

    if (pAbility->Full_10)
    {
        /*10BASE-TX full duplex capable in reg 4.6*/
        phyEnMsk4 |= (1<<6);

        /*Speed selection [1:0] */
        /* 00 = 10Mpbs*/
    }

    if (pAbility->Half_100)
    {
        /*100BASE-TX half duplex capable in reg 4.7*/
        phyEnMsk4 |= (1<<7);

        /*Speed selection [1:0] */
        /* 01 = 100Mpbs*/
        phyEnMsk0 |= (1<<13);
    }

    if (pAbility->Full_100)
    {
        /*100BASE-TX full duplex capable in reg 4.8*/
        phyEnMsk4 |= (1<<8);

        /*Speed selection [1:0] */
        /* 01 = 100Mpbs*/
        phyEnMsk0 |= (1<<13);
    }

    if (pAbility->Full_100 || pAbility->Full_10)
    {
        /*Full duplex mode in reg 0.8*/
        phyEnMsk0 |= (1<<8);
    }

    if (pAbility->AsyFC)
    {
        /*Asymetric flow control in reg 4.11*/
        phyEnMsk4 |= (1<<11);
    }

    if (pAbility->FC)
    {
        /*Flow control in reg 4.10*/
        phyEnMsk4 |= (1<<10);
    }

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;

    /*1000 BASE-T control register setting*/
    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_1000_BASET_CONTROL_REG,&phyData))!=RT_ERR_OK)
        return retVal;

    phyData &= ~(0x0200);
    phyData |= phyEnMsk9;
    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_1000_BASET_CONTROL_REG,phyData))!=RT_ERR_OK)
        return retVal;

    /*Auto-Negotiation control register setting*/
    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_AN_ADVERTISEMENT_REG,&phyData))!=RT_ERR_OK)
        return retVal;

    phyData &= ~(0x0DE0);
    phyData |= phyEnMsk4;
    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_AN_ADVERTISEMENT_REG,phyData))!=RT_ERR_OK)
        return retVal;

    /*Control register setting and power off/on*/
    phyData = phyEnMsk0;
    phyData &= ~(1 << 12);
    phyData |= (1 << 11);   /* power down PHY, bit 11 should be set to 1 */
    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_CONTROL_REG,phyData))!=RT_ERR_OK)
        return retVal;

    msleep(100);

    phyData &= ~(1 << 11);   /* power on PHY, bit 11 should be set to 0*/
    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_CONTROL_REG,phyData))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyLink_get
 * Description:
 *      Get ethernet PHY linking status
 * Input:
 *      port - Port id.
 * Output:
 *      linkStatus - PHY link status
 * Return:
 *      RT_ERR_OK               - set shared meter successfully
 *      RT_ERR_FAILED           - FAILED to iset shared meter
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      API will return auto negotiation status of phy.
 */
rtk_api_ret_t rtk_port_phyLink_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus)
{
    rtk_api_ret_t retVal;
    uint32 phyData;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;

    /*Get PHY status register*/
    if (RT_ERR_OK != rtl8370_getAsicPHYReg(port,PHY_STATUS_REG,&phyData))
        return RT_ERR_FAILED;

    /*check link status*/
    if (phyData & (1<<2))
    {
        *pLinkStatus = 1;
    }
    else
    {
        *pLinkStatus = 0;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_port_phyStatus_get
 * Description:
 *      Get ethernet PHY linking status
 * Input:
 *      port - Port id.
 * Output:
 *      linkStatus - PHY link status
 *      speed - PHY link speed
 *      duplex - PHY duplex mode
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_PHY_REG_ID - Invalid PHY address
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      API will return auto negotiation status of phy.  
 */
rtk_api_ret_t rtk_port_phyStatus_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus, rtk_data_t *pSpeed, rtk_data_t *pDuplex)
{
    rtk_api_ret_t retVal;
    uint32 phyData;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;

    /*Get PHY status register*/
    if (RT_ERR_OK != rtl8370_getAsicPHYReg(port,PHY_STATUS_REG,&phyData))
        return RT_ERR_FAILED;

    /*check link status*/
    if (phyData & (1<<2))
    {
        *pLinkStatus = 1;

        /*Get PHY resolved register*/
        if ((retVal = rtl8370_getAsicPHYReg(port,PHY_RESOLVED_REG,&phyData))!=RT_ERR_OK)
            return retVal;

        /*check resolution is complete or not*/
        if (!(phyData&(1<<11)))
            return RT_ERR_BUSYWAIT_TIMEOUT;

        /*check link speed*/
        *pSpeed = (phyData&0xC000)>>14;

        /*check link duplex*/
        *pDuplex = (phyData&0x2000)>>13;           
    }
    else
    {
        *pLinkStatus = 0;
        *pSpeed = 0;
        *pDuplex = 0; 
    }

    return RT_ERR_OK;
}

#if !defined(_REDUCE_CODE)

/* Function Name:
 *      rtk_port_phyTestMode_set
 * Description:
 *      Set PHY in test mode.
 * Input:
 *      port - port id.
 *      mode - PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Set PHY in test mode and only one PHY can be in test mode at the same time. 
 *      It means API will return FALED if other PHY is in test mode.
 *      This API only provide test mode 1 setup, and if users want other test modes, 
 *      please contact realtek FAE.
 */
rtk_api_ret_t rtk_port_phyTestMode_set(rtk_port_t port, rtk_port_phy_test_mode_t mode)
{
    rtk_api_ret_t retVal;
    uint32 phyData,regData;
    int16 phyIdx;
    uint32 i,index,busyFlag,cnt;

    CONST_T uint32 ParaTM0_0[][2] = {{0x133f,0x0030},{0x133e,0x000e},{0x221f,0x0002},{0x2207,0x3620},
                                   {0x2204,0x80C8},{0x221f,0x0000},{0x133f,0x0010},{0x209f,0x0002},
                                   {0x2094,0x00AA},{0x2095,0x00AA},{0x2096,0x00AA},{0x2097,0x00AA},
                                   {0x2098,0x0055},{0x2099,0x00AA},{0x209A,0x00AA},{0x209B,0x00AA},
                                   {0x209f,0x0000},{0x133f,0x0030},{0x133e,0x000e},{0x221f,0x0000},
                                   {0x2209,0x0E00},{0x2215,0x1006},{0x2200,0x1340},{0x133f,0x0010},
                                   {0xFFFF, 0xABCD}}; 

    CONST_T uint32 ParaTM0_1[][2]= {{0x133f,0x0030},{0x133e,0x000e},{0x221f,0x0005},{0x2201,0x0700},
                                   {0x2205,0x8B82},{0x2206,0x05CB},{0x221f,0x0002},{0x2204,0x80C2},
                                   {0x2205,0x0938},{0x221F,0x0003},{0x2212,0xC4D2},{0x220D,0x0207},
                                   {0x221f,0x0001},{0x2207,0x267E},{0x221C,0xE5F7},{0x221B,0x0424},
                                   {0x221f,0x0005},{0x2205,0xfff6},{0x2206,0x0080},{0x2205,0x8000},
                                   {0x2206,0xf8e0},{0x2206,0xe000},{0x2206,0xe1e0},{0x2206,0x01ac},
                                   {0x2206,0x2408},{0x2206,0xe08b},{0x2206,0x84f7},{0x2206,0x20e4},
                                   {0x2206,0x8b84},{0x2206,0xfc05},{0x2205,0x8b90},{0x2206,0x8000},
                                   {0x2205,0x8b92},{0x2206,0x8000},{0x2208,0xfffa},{0x2202,0x3265},
                                   {0x2205,0xfff6},{0x2206,0x00f3},{0x221f,0x0000},{0x221f,0x0007},
                                   {0x221e,0x0042},{0x2218,0x0000},{0x221e,0x002D},{0x2218,0xF010},
                                   {0x221f,0x0000},{0x133f,0x0010},{0x133e,0x0ffe},{0x207f,0x0002},
                                   {0x2079,0x0200},{0x207f,0x0000},{0x209f,0x0002},{0x2093,0x00AA},
                                   {0x2094,0x00AA},{0x2095,0x00AA},{0x2096,0x00AA},{0x2097,0x0055},
                                   {0x2098,0x00AA},{0x2099,0x00AA},{0x209A,0x00AA},{0x209f,0x0000},
                                   {0x130f,0x0840},{0x133f,0x0030},{0x133e,0x000e},{0x221f,0x0000},
                                   {0x2215,0x1006},{0x221f,0x0002},{0x2207,0x3620},{0x133f,0x0010},
                                   {0x133e,0x0ffe},{0xFFFF,0xABCD}};

    CONST_T uint32 ParaTM1_0[][2] = {{0x133f,0x0030},{0x133e,0x000e},{0x221f,0x0000},{0x2215,0x0006},                                    
                                   {0x221f,0x0005},{0x2205,0xfff6},{0x2206,0x0000},{0x2205,0x8000},
                                   {0x2206,0xd480},{0x2206,0x18e4},{0x2206,0x8bbc},{0x2206,0xe58b},
                                   {0x2206,0xbdee},{0x2206,0x8400},{0x2206,0x00ee},{0x2206,0xfff6},
                                   {0x2206,0x00ee},{0x2206,0xfff7},{0x2206,0x70af},{0x2206,0x015d},
                                   {0x2206,0xf802},{0x2206,0x24bc},{0x2206,0xe08b},{0x2206,0xa602},
                                   {0x2206,0x80ef},{0x2206,0xe184},{0x2206,0x00ad},{0x2206,0x2803},
                                   {0x2206,0x0221},{0x2206,0x6cad},{0x2206,0x2203},{0x2206,0x0280},
                                   {0x2206,0x33fc},{0x2206,0x05f8},{0x2206,0xf9fa},{0x2206,0xfbef},
                                   {0x2206,0x79e2},{0x2206,0x8ba5},{0x2206,0xac1a},{0x2206,0x25e0},
                                   {0x2206,0xe022},{0x2206,0xe1e0},{0x2206,0x23ef},{0x2206,0x300d},
                                   {0x2206,0x311f},{0x2206,0x325b},{0x2206,0x029e},{0x2206,0x157a},
                                   {0x2206,0x0258},{0x2206,0xc4a0},{0x2206,0x0408},{0x2206,0xbf81},
                                   {0x2206,0x2402},{0x2206,0x250a},{0x2206,0xae06},{0x2206,0xbf81},
                                   {0x2206,0x2202},{0x2206,0x250a},{0x2206,0xac1b},{0x2206,0x56e0},
                                   {0x2206,0xe012},{0x2206,0xe1e0},{0x2206,0x13ef},{0x2206,0x300d},
                                   {0x2206,0x331f},{0x2206,0x325b},{0x2206,0x1c9e},{0x2206,0x46ef},
                                   {0x2206,0x325b},{0x2206,0x1c9f},{0x2206,0x09bf},{0x2206,0x811e},
                                   {0x2206,0x0225},{0x2206,0x0a02},{0x2206,0x80d3},{0x2206,0x5a03},
                                   {0x2206,0x0d03},{0x2206,0x581c},{0x2206,0x1e20},{0x2206,0xa000},
                                   {0x2206,0x36bf},{0x2206,0x8ba0},{0x2206,0xe1e0},{0x2206,0x82e5},
                                   {0x2206,0xe082},{0x2206,0xe9e0},{0x2206,0x8319},{0x2206,0xe1e0},
                                   {0x2206,0x8ee5},{0x2206,0xe08e},{0x2206,0xe9e0},{0x2206,0x8f19},
                                   {0x2206,0xe1e0},{0x2206,0xb8e5},{0x2206,0xe0b8},{0x2206,0xe9e0},
                                   {0x2206,0xb9a0},{0x2206,0x0009},{0x2206,0xad18},{0x2206,0x06bf},
                                   {0x2206,0x8120},{0x2206,0x0225},{0x2206,0x0ae6},{0x2206,0x8ba5},
                                   {0x2206,0xef97},{0x2206,0xfffe},{0x2206,0xfdfc},{0x2206,0x0458},
                                   {0x2206,0x089f},{0x2206,0xc6bf},{0x2206,0x8126},{0x2206,0x0225},
                                   {0x2206,0x0aae},{0x2206,0xeabf},{0x2206,0xe082},{0x2206,0xdb19},
                                   {0x2206,0xd9e5},{0x2206,0x8ba0},{0x2206,0xbfe0},{0x2206,0x8edb},
                                   {0x2206,0x19d9},{0x2206,0xe58b},{0x2206,0xa1bf},{0x2206,0xe0b8},
                                   {0x2206,0xdb19},{0x2206,0xd9e5},{0x2206,0x8ba2},{0x2206,0x04f8},
                                   {0x2206,0xe0e0},{0x2206,0x12e1},{0x2206,0xe013},{0x2206,0x58e0},
                                   {0x2206,0x9e1e},{0x2206,0xeee4},{0x2206,0x6000},{0x2206,0xeee4},
                                   {0x2206,0x6100},{0x2206,0xee8a},{0x2206,0xe800},{0x2206,0xee8a},
                                   {0x2206,0xec00},{0x2206,0xee8a},{0x2206,0xed00},{0x2206,0xee8a},
                                   {0x2206,0xee00},{0x2206,0xee84},{0x2206,0x0000},{0x2206,0xae04},
                                   {0x2206,0xee84},{0x2206,0x0001},{0x2206,0xfc04},{0x2206,0x7950},
                                   {0x2206,0x7960},{0x2206,0x4550},{0x2206,0x4560},{0x2206,0x4116},
                                   {0x2206,0x5c24},{0x2206,0x47e3},{0x2206,0x5400},{0x2201,0x0001},
                                   {0x2200,0x0001},{0x221f,0x0000},{0x221f,0x0002},{0x2207,0x3678},
                                   {0x221f,0x0000},{0x133f,0x0010},{0x133e,0x0ffe},{0x209f,0x0002},
                                   {0x2094,0xAA00},{0x2095,0xAA00},{0x2096,0xAA00},{0x2097,0xFA00},
                                   {0x2098,0xAF00},{0x2099,0xAA00},{0x209A,0xAA00},{0x209B,0xAA00},
                                   {0x209f,0x0000},{0xFFFF, 0xABCD}}; 

    CONST_T uint32 ParaTM1_1[][2]= {{0x133f,0x0030},{0x133e,0x000e},{0x221f,0x0000},{0x2215,0x0006},
                                   {0x133f,0x0010},{0x133e,0x0ffe},{0x209F,0x0002},{0x2093,0xAA00},
                                   {0x2094,0xAA00},{0x2095,0xAA00},{0x2096,0xFA00},{0x2097,0xAF00},
                                   {0x2098,0xAA00},{0x2099,0xAA00},{0x209A,0xAA00},{0x209F,0x0000},
                                   {0x130f,0x0800},{0xFFFF, 0xABCD}};
    
    CONST_T uint32 ParaTM[][2] =  {{0x133f,0x0030},{0x133e,0x000e},{0x221f,0x0007},{0x221e,0x002C},
                                   {0x2218,0x0043},{0x2218,0x004B},{0x221f,0x0000},{0x133f,0x0010},
                                   {0x133e,0x0ffe},{0xFFFF, 0xABCD}}; 

   if (port > RTK_PHY_ID_MAX)
        return RT_ERR_PORT_ID; 

    if (mode >= PHY_TEST_MODE_2)
        return RT_ERR_FAILED;

    if ((retVal = rtl8370_setAsicReg(0x13c2,0x0249))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicRegBits(0x1301, 0xf000,&regData))!=RT_ERR_OK)
        return retVal;

    if (0 == regData)
    {
        if (PHY_TEST_MODE_1 == mode)
        {
            index = 0;
            while (ParaTM1_0[index][0] != 0xFFFF && ParaTM1_0[index][1] != 0xABCD)
            {  
                if (index == 4)
                    for (i=0; i<20000; i++); /*Delay 2000ms*/
         
                if ((ParaTM1_0[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (cnt==5)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
            
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, 0)) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, PHY_PAGE_ADDRESS)) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                    
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (cnt==5)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, ParaTM1_0[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, ParaTM1_0[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg(ParaTM1_0[index][0],ParaTM1_0[index][1]))
                        return RT_ERR_FAILED;
                }
                index ++;    
            }
        }
    }    
    else if (1 == regData)
    { 
#ifdef MDC_MDIO_OPERATION 
        if (PHY_TEST_MODE_1 == mode)
        {
            index = 0;
            while (ParaTM1_1[index][0] != 0xFFFF && ParaTM1_1[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg(ParaTM1_1[index][0],ParaTM1_1[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            } 
        }
#else 
        if (PHY_TEST_MODE_1 == mode)
        {
            index = 0;
            while (ParaTM1_1[index][0] != 0xFFFF && ParaTM1_1[index][1] != 0xABCD)
            {  
                if (index == 4)
                    for (i=0; i<20000; i++); /*Delay 2000ms*/
    
                             
                if ((ParaTM1_1[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (cnt==5)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
            
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, 0)) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, PHY_PAGE_ADDRESS)) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
    
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (cnt==5)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, ParaTM1_1[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, ParaTM1_1[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg(ParaTM1_1[index][0],ParaTM1_1[index][1]))
                        return RT_ERR_FAILED;
                }
                index ++;    
            }
        }
#endif /*End of #ifdef MDC_MDIO_OPERATION*/
    }
       
    phyIdx = 0;
    while (phyIdx <= RTK_PHY_ID_MAX)
    {
        if (phyIdx != port)
        {
            if ((retVal = rtl8370_setAsicPHYReg(phyIdx,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
                 return retVal; 
          
            if ((retVal = rtl8370_getAsicPHYReg(phyIdx,PHY_1000_BASET_CONTROL_REG,&phyData))!=RT_ERR_OK)
                return retVal;                
              
            /*Have other PHY in test mode*/    
            if (phyData & (0x7<<13))
            {
                return RT_ERR_FAILED;
            }  
        }
        phyIdx ++;
    }

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal; 

    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_1000_BASET_CONTROL_REG,&phyData))!=RT_ERR_OK)
        return retVal;
        
    phyData = (phyData & (~(0x7<<13))) | (mode<<13);

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_1000_BASET_CONTROL_REG,phyData))!=RT_ERR_OK)
        return retVal; 
    
    if (0 == regData)
    {
        if (PHY_TEST_MODE_1 == mode)
        {
            index = 0;
            while (ParaTM[index][0] != 0xFFFF && ParaTM[index][1] != 0xABCD)
            {                    
                if (0x2000 == (ParaTM[index][0]&0xF000))
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
            
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, 0)) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, PHY_PAGE_ADDRESS)) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
            
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
                     
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, ParaTM[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, ParaTM[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg(ParaTM[index][0],ParaTM[index][1]))
                        return RT_ERR_FAILED;
                }
                index ++;    
            }
        }
    }

    if (0 == regData)
    {
        if (PHY_TEST_MODE_NORMAL == mode)
        {
            index = 0;
            while (ParaTM0_0[index][0] != 0xFFFF && ParaTM0_0[index][1] != 0xABCD)
            {  
                if ((ParaTM0_0[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                                return retVal;
                    }
                    if (cnt==5)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
            
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, 0)) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, PHY_PAGE_ADDRESS)) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
    
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (cnt==5)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, ParaTM0_0[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, ParaTM0_0[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg(ParaTM0_0[index][0],ParaTM0_0[index][1]))
                        return RT_ERR_FAILED;
                }
                index ++;    
            } 
        } 
    }    
    else if (1 == regData)
    { 
#ifdef MDC_MDIO_OPERATION 
        if (PHY_TEST_MODE_NORMAL == mode)
        {
            index = 0;
            while (ParaTM0_1[index][0] != 0xFFFF && ParaTM0_1[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg(ParaTM0_1[index][0],ParaTM0_1[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            } 
        }
#else 
        if (PHY_TEST_MODE_NORMAL == mode)
        {
            index = 0;
            while (ParaTM0_1[index][0] != 0xFFFF && ParaTM0_1[index][1] != 0xABCD)
            {  
                if ((ParaTM0_1[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                                return retVal;
                    }
                    if (cnt==5)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
            
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, 0)) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, PHY_PAGE_ADDRESS)) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
    
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (cnt==5)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, ParaTM0_1[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, ParaTM0_1[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg(ParaTM0_1[index][0],ParaTM0_1[index][1]))
                        return RT_ERR_FAILED;
                }
                index ++;    
            } 
        }    
#endif /*End of #ifdef MDC_MDIO_OPERATION*/
    }
 
    return RT_ERR_OK;
}    


/* Function Name:
 *      rtk_port_phyTestMode_get
 * Description:
 *      Get PHY in which test mode.
 * Input:
 *      port - Port id.
 * Output:
 *      mode - PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get test mode of PHY from register setting 9.15 to 9.13.
 */
rtk_api_ret_t rtk_port_phyTestMode_get(rtk_port_t port, rtk_data_t *mode)
{
    rtk_api_ret_t retVal;
    uint32 phyData;

    if (port > RTK_PHY_ID_MAX)
        return RT_ERR_PORT_ID; 
    
    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_1000_BASET_CONTROL_REG,&phyData))!=RT_ERR_OK)
        return retVal;

    *mode = (phyData>>13) & 0x7;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phy1000BaseTMasterSlave_set
 * Description:
 *      Set PHY control enable MASTER/SLAVE manual configuration.
 * Input:
 *      port - port id.
 *      enable - Manual configuration function 1:enable 0:disable.
 *      masterslave - Manual config mode 1:master 0: slave
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      Set enable/disable MASTER/SLAVE manual configuration under 1000Base-T with register 9.12-9.11. If MASTER/SLAVE manual configuration is enabled with MASTER, the
 *      link partner must be set as SLAVE or auto negotiation will fail. 
 */
rtk_api_ret_t rtk_port_phy1000BaseTMasterSlave_set(rtk_port_t port, rtk_enable_t enable, rtk_enable_t masterslave)
{
    rtk_api_ret_t retVal;
    uint32 phyData;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;  

    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_1000_BASET_CONTROL_REG,&phyData))!=RT_ERR_OK)
        return retVal;

    phyData = (phyData & (~(0x3<<11))) | (enable<<12) | (masterslave<<11);

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_1000_BASET_CONTROL_REG,phyData))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_1000_BASET_CONTROL_REG,phyData))!=RT_ERR_OK)
        return retVal;
    
    /*Restart N-way*/
    if ((retVal = rtl8370_getAsicPHYReg(port,PHY_CONTROL_REG,&phyData))!=RT_ERR_OK)
        return retVal;


    phyData = phyData | (1 << 9);
    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_CONTROL_REG,phyData))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal; 
    

    return RT_ERR_OK;
}

#endif

/* Function Name:
 *      rtk_port_macForceLink_set
 * Description:
 *      Set port force linking configuration.
 * Input:
 *      port - port id.
 *      pPortability - port ability configuration
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can set Port/MAC force mode properties. 
 */
rtk_api_ret_t rtk_port_macForceLink_set(rtk_port_t port, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8370_port_ability_t ability;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (pPortability->forcemode>1||pPortability->speed>2||pPortability->duplex>1||
       pPortability->link>1||pPortability->nway>1||pPortability->txpause>1||pPortability->rxpause>1)
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_getAsicPortForceLink(port,&ability))!=RT_ERR_OK)
        return retVal;
     
    ability.forcemode = pPortability->forcemode;
    ability.speed     = pPortability->speed;
    ability.duplex    = pPortability->duplex;
    ability.link      = pPortability->link;
    ability.nway      = pPortability->nway;
    ability.txpause   = pPortability->txpause;
    ability.rxpause   = pPortability->rxpause;

    if ((retVal = rtl8370_setAsicPortForceLink(port,&ability))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macForceLink_get
 * Description:
 *      Get port force linking configuration.
 * Input:
 *      port - Port id.
 * Output:
 *      pPortability - port ability configuration
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can get Port/MAC force mode properties. 
 */
rtk_api_ret_t rtk_port_macForceLink_get(rtk_port_t port, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8370_port_ability_t ability;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicPortForceLink(port,&ability))!=RT_ERR_OK)
        return retVal;
     
    pPortability->forcemode = ability.forcemode;
    pPortability->speed     = ability.speed;
    pPortability->duplex    = ability.duplex;
    pPortability->link      = ability.link;
    pPortability->nway      = ability.nway;
    pPortability->txpause   = ability.txpause;
    pPortability->rxpause   = ability.rxpause;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macForceLinkExt0_set
 * Description:
 *      Set external interface 0 force linking configuration.
 * Input:
 *      mode - external interface mode
 *      pPortability - port ability configuration
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can set external interface 0 force mode properties. 
 *      The external interface can be set to:
 *      MODE_EXT_DISABLE,
 *      MODE_EXT_RGMII,
 *      MODE_EXT_MII_MAC,
 *      MODE_EXT_MII_PHY, 
 *      MODE_EXT_TMII_MAC,
 *      MODE_EXT_TMII_PHY, 
 *      MODE_EXT_GMII, 
 *      MODE_EXT_RGMII_33V,
 */
rtk_api_ret_t rtk_port_macForceLinkExt0_set(rtk_mode_ext_t mode, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8370_port_ability_t ability;

    if (mode >=MODE_EXT_END)
        return RT_ERR_INPUT;    

    if (pPortability->forcemode>1||pPortability->speed>2||pPortability->duplex>1||
       pPortability->link>1||pPortability->nway>1||pPortability->txpause>1||pPortability->rxpause>1)
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_setAsicPortExtMode(RTK_EXT_0, mode))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicPortForceLinkExt(RTK_EXT_0,&ability))!=RT_ERR_OK)
        return retVal;
     
    ability.forcemode = pPortability->forcemode;
    ability.speed     = pPortability->speed;
    ability.duplex    = pPortability->duplex;
    ability.link      = pPortability->link;
    ability.nway      = pPortability->nway;
    ability.txpause   = pPortability->txpause;
    ability.rxpause   = pPortability->rxpause;

    if ((retVal = rtl8370_setAsicPortForceLinkExt(RTK_EXT_0,&ability))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macForceLinkExt0_get
 * Description:
 *      Get external interface 0 force linking configuration.
 * Input:
 *      None
 * Output:
 *      pMode - external interface mode
 *      pPortability - port ability configuration
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can get external interface 0 force mode properties. 
 */
rtk_api_ret_t rtk_port_macForceLinkExt0_get(rtk_data_t *pMode, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8370_port_ability_t ability;

    if ((retVal = rtl8370_getAsicPortExtMode(RTK_EXT_0, pMode))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicPortForceLinkExt(RTK_EXT_0,&ability))!=RT_ERR_OK)
        return retVal;
     
    pPortability->forcemode = ability.forcemode;
    pPortability->speed     = ability.speed;
    pPortability->duplex    = ability.duplex;
    pPortability->link      = ability.link;
    pPortability->nway      = ability.nway;
    pPortability->txpause   = ability.txpause;
    pPortability->rxpause   = ability.rxpause;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_port_macForceLinkExt1_set
 * Description:
 *      Set external interface 1 force linking configuration.
 * Input:
 *      mode - external interface mode
 *      pPortability - port ability configuration
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can set external interface 1 force mode properties. 
 *      The external interface can be set to:
 *      MODE_EXT_DISABLE,
 *      MODE_EXT_RGMII,
 *      MODE_EXT_MII_MAC,
 *      MODE_EXT_MII_PHY, 
 *      MODE_EXT_TMII_MAC,
 *      MODE_EXT_TMII_PHY, 
 *      MODE_EXT_GMII, 
 *      MODE_EXT_RGMII_33V,
 */
rtk_api_ret_t rtk_port_macForceLinkExt1_set(rtk_mode_ext_t mode, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8370_port_ability_t ability;

    if (mode >=MODE_EXT_END)
        return RT_ERR_INPUT;

    if (pPortability->forcemode>1||pPortability->speed>2||pPortability->duplex>1||
       pPortability->link>1||pPortability->nway>1||pPortability->txpause>1||pPortability->rxpause>1)
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_setAsicPortExtMode(RTK_EXT_1, mode))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicPortForceLinkExt(RTK_EXT_1,&ability))!=RT_ERR_OK)
        return retVal;
     
    ability.forcemode = pPortability->forcemode;
    ability.speed     = pPortability->speed;
    ability.duplex    = pPortability->duplex;
    ability.link      = pPortability->link;
    ability.nway      = pPortability->nway;
    ability.txpause   = pPortability->txpause;
    ability.rxpause   = pPortability->rxpause;

    if ((retVal = rtl8370_setAsicPortForceLinkExt(RTK_EXT_1,&ability))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macForceLinkExt1_get
 * Description:
 *      Get external interface 1 force linking configuration.
 * Input:
 *      None
 * Output:
 *      pMode - external interface mode
 *      pPortability - port ability configuration
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can get external interface 1 force mode properties. 
 */
rtk_api_ret_t rtk_port_macForceLinkExt1_get(rtk_mode_ext_t *pMode, rtk_port_mac_ability_t *pPortability)
{
    rtk_api_ret_t retVal;
    rtl8370_port_ability_t ability;

    if ((retVal = rtl8370_getAsicPortExtMode(RTK_EXT_1, pMode))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicPortForceLinkExt(RTK_EXT_1,&ability))!=RT_ERR_OK)
        return retVal;
     
    pPortability->forcemode = ability.forcemode;
    pPortability->speed     = ability.speed;
    pPortability->duplex    = ability.duplex;
    pPortability->link      = ability.link;
    pPortability->nway      = ability.nway;
    pPortability->txpause   = ability.txpause;
    pPortability->rxpause   = ability.rxpause;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_macStatus_get
 * Description:
 *      Get port link status.
 * Input:
 *      port - Port id.
 * Output:
 *      pPortstatus - port ability configuration
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can get Port/PHY properties. 
 */
rtk_api_ret_t rtk_port_macStatus_get(rtk_port_t port, rtk_port_mac_ability_t *pPortstatus)
{
    rtk_api_ret_t retVal;
    rtl8370_port_status_t status;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicPortStatus(port,&status))!=RT_ERR_OK)
        return retVal;
     
    pPortstatus->speed     = status.speed;
    pPortstatus->duplex    = status.duplex;
    pPortstatus->link      = status.link;
    pPortstatus->nway      = status.nway;
    pPortstatus->txpause   = status.txpause;
    pPortstatus->rxpause   = status.rxpause;
    pPortstatus->lpi100    = status.lpi100;
    pPortstatus->lpi1000   = status.lpi1000;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port.
 * Input:
 *      port - port id.
 *      reg - Register id
 *      regData - Register data
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_PHY_REG_ID - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      This API can set PHY register data of the specific port.
 */
rtk_api_ret_t rtk_port_phyReg_set(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t regData)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PHY_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicPHYReg(port,reg,regData))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port.
 * Input:
 *      port - Port id.
 *      reg - Register id
 * Output:
 *      pData - Register data
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_PHY_REG_ID - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      This API can get PHY register data of the specific port. 
 */
rtk_api_ret_t rtk_port_phyReg_get(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t *pData) 
{
    rtk_api_ret_t retVal;

    if (port > RTK_PHY_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal; 

    if ((retVal = rtl8370_getAsicPHYReg(port,reg,pData))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_backpressureEnable_set
 * Description:
 *      Set the half duplex backpressure enable status of the specific port.
 * Input:
 *      port - port id.
 *      enable - Back pressure status.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set the half duplex backpressure enable status of the specific port.
 *      The half duplex backpressure enable status of the port is as following:
 *      DISABLE   
 *      ENABLE
 */
rtk_api_ret_t rtk_port_backpressureEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_setAsicPortJamMode(!enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_port_backpressureEnable_get
 * Description:
 *      Get the half duplex backpressure enable status of the specific port.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can get the half duplex backpressure enable status of the specific port.
 *      The half duplex backpressure enable status of the port is as following:
 *      DISABLE   
 *      ENABLE
 */
rtk_api_ret_t rtk_port_backpressureEnable_get(rtk_port_t port, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;
    uint32 regData;

    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;
    
    if ((retVal = rtl8370_getAsicPortJamMode(&regData))!=RT_ERR_OK)
        return retVal;

    *pEnable = !regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port.
 * Input:
 *      port - port id.
 *      enable - Back pressure status.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      DISABLE   
 *      ENABLE
 */
rtk_api_ret_t rtk_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtl8370_port_ability_t ability;
    uint32 regData, extif;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (port <= RTK_PHY_ID_MAX)
    {

        if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
            return retVal; 
    
        if ((retVal = rtl8370_getAsicPHYReg(port,PHY_CONTROL_REG,&regData))!=RT_ERR_OK)
            return retVal;

    if ((retVal = rtl8370_getAsicPortForceLink(port,&ability))!=RT_ERR_OK)
        return retVal;

        if (ENABLED == enable)
        {
            regData = regData & (~(1<<11)); 
            ability.forcemode= 0;
        }
        else if (DISABLED == enable)
        {
            regData = regData | (1<<11);
            ability.forcemode= 1;
            ability.link = 0;
        }
        else
            return RT_ERR_FAILED;
    
        if ((retVal = rtl8370_setAsicPHYReg(port,PHY_CONTROL_REG,regData))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
            return retVal; 

        if ((retVal = rtl8370_setAsicPortForceLink(port,&ability))!=RT_ERR_OK)
            return retVal;
    
    }
    else if (port <= RTK_PORT_ID_MAX )
    {
        if (port == RTK_EXT_0_MAC9)
            extif =  RTK_EXT_0;
        else
            extif =  RTK_EXT_1;
        
        if ((retVal = rtl8370_getAsicPortForceLinkExt(extif,&ability))!=RT_ERR_OK)
            return retVal;
    
        if (ENABLED == enable)
        {
            ability.link = 1;
        }
        else if (DISABLED == enable)
        {
            ability.link = 0;
        }
        else
            return RT_ERR_FAILED;

        ability.forcemode = 1;
        
        if ((retVal = rtl8370_setAsicPortForceLinkExt(extif,&ability))!=RT_ERR_OK)
            return retVal;
    }
    else
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_port_adminEnable_get
 * Description:
 *      Get port admin configurationof the specific port.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can get port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      DISABLE   
 *      ENABLE
 */

rtk_api_ret_t rtk_port_adminEnable_get(rtk_port_t port, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtl8370_port_ability_t ability;
    uint32 regData, extif;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (port <= RTK_PHY_ID_MAX)
    {
        if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
            return retVal;    
        if ((retVal = rtl8370_getAsicPHYReg(port,PHY_CONTROL_REG,&regData))!=RT_ERR_OK)
            return retVal;
        if (regData& (1<<11))
           *pEnable = 0;
        else
           *pEnable = 1;
        if ((retVal = rtl8370_setAsicPHYReg(port,PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
            return retVal;     
    }
    else if (port <= RTK_PORT_ID_MAX )
    {
        if (port == RTK_EXT_0_MAC9)
            extif =  RTK_EXT_0;
        else
            extif =  RTK_EXT_1;
    
        if ((retVal = rtl8370_getAsicPortForceLinkExt(extif,&ability))!=RT_ERR_OK)
            return retVal;
        *pEnable = ability.link;
    }
    else
        return RT_ERR_FAILED;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_isolation_set
 * Description:
 *      Set permitted port isolation portmask
 * Input:
 *      port - port id.
 *      portmask - Permit port mask
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_PORT_MASK - Invalid portmask.
 * Note:
 *      This API set the port mask that a port can trasmit packet to of each port
 *      A port can only transmit packet to ports included in permitted portmask
 */
rtk_api_ret_t rtk_port_isolation_set(rtk_port_t port, rtk_portmask_t portmask)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ( portmask.bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK;

    if ((retVal = rtl8370_setAsicPortIsolationPermittedPortmask(port,portmask.bits[0]))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_isolation_get
 * Description:
 *      Get permitted port isolation portmask
 * Input:
 *      port - Port id.
 * Output:
 *      pPortmask - Permit port mask
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API get the port mask that a port can trasmit packet to of each port
 *      A port can only transmit packet to ports included in permitted portmask
 */
rtk_api_ret_t rtk_port_isolation_get(rtk_port_t port, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicPortIsolationPermittedPortmask(port,&pPortmask->bits[0]))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_port_rgmiiDelayExt0_set
 * Description:
 *      Set RGMII interface 0 delay value for TX and RX.
 * Input:
 *      txDelay - TX delay value, 1 for delay 2ns and 0 for no-delay
 *      rxDelay - RX delay value, 0~7 for delay setup.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can set external interface 0 RGMII delay. 
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for no-delay, and 7 for maximum delay.  
 */
rtk_api_ret_t rtk_port_rgmiiDelayExt0_set(rtk_data_t txDelay, rtk_data_t rxDelay)
{
    rtk_api_ret_t retVal;
    uint32 regData;

    if ((txDelay > 1)||(rxDelay > 7))
        return RT_ERR_INPUT;
    
    if ((retVal = rtl8370_getAsicReg(RTL8370_REG_EXT0_RGMXF, &regData))!=RT_ERR_OK)
        return retVal;

    regData = (regData&0xFFF0) | ((txDelay<<3)&0x0008) | (rxDelay&0x007);

    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_EXT0_RGMXF, regData))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_rgmiiDelayExt0_get
 * Description:
 *      Get RGMII interface 0 delay value for TX and RX.
 * Input:
 *      None
 * Output:
 *      pTxDelay - TX delay value
 *      pRxDelay - RX delay value
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can set external interface 0 RGMII delay. 
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for n0-delay, and 7 for maximum delay.  
 */
rtk_api_ret_t rtk_port_rgmiiDelayExt0_get(rtk_data_t *pTxDelay, rtk_data_t *pRxDelay)
{
    rtk_api_ret_t retVal;
    uint32 regData;

    if ((retVal = rtl8370_getAsicReg(RTL8370_REG_EXT0_RGMXF, &regData))!=RT_ERR_OK)
        return retVal;

    *pTxDelay = (regData&0x0008)>>3;

    *pRxDelay = regData&0x0007;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_rgmiiDelayExt1_set
 * Description:
 *      Set RGMII interface 1 delay value for TX and RX.
 * Input:
 *      txDelay - TX delay value, 1 for delay 2ns and 0 for no-delay
 *      rxDelay - RX delay value, 0~7 for delay setup.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can set external interface 1 RGMII delay. 
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for n0-delay, and 7 for maximum delay.  
 */
rtk_api_ret_t rtk_port_rgmiiDelayExt1_set(rtk_data_t txDelay, rtk_data_t rxDelay)
{
    rtk_api_ret_t retVal;
    uint32 regData;

    if ((txDelay > 1)||(rxDelay > 7))
        return RT_ERR_INPUT;
    
    if ((retVal = rtl8370_getAsicReg(RTL8370_REG_EXT1_RGMXF, &regData))!=RT_ERR_OK)
        return retVal;

    regData = (regData&0xFFF0) | ((txDelay<<3)&0x0008) | (rxDelay&0x007);

    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_EXT1_RGMXF, regData))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_rgmiiDelayExt1_get
 * Description:
 *      Get RGMII interface 1 delay value for TX and RX.
 * Input:
 *      None
 * Output:
 *      pTxDelay - TX delay value
 *      pRxDelay - RX delay value
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can set external interface 1 RGMII delay. 
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for n0-delay, and 7 for maximum delay.  
 */
rtk_api_ret_t rtk_port_rgmiiDelayExt1_get(rtk_data_t *pTxDelay, rtk_data_t *pRxDelay)
{
    rtk_api_ret_t retVal;
    uint32 regData;

    if ((retVal = rtl8370_getAsicReg(RTL8370_REG_EXT1_RGMXF, &regData))!=RT_ERR_OK)
        return retVal;

    *pTxDelay = (regData&0x0008)>>3;

    *pRxDelay = regData&0x0007;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyEnableAll_set
 * Description:
 *      Set all PHY enable status.
 * Input:
 *      enable - Back pressure status.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set all PHY status.
 *      The configuration of all PHY is as following:
 *      DISABLE   
 *      ENABLE
 */
rtk_api_ret_t rtk_port_phyEnableAll_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >=RTK_ENABLE_END)
        return RT_ERR_ENABLE;    

    if ((retVal = rtl8370_setAsicPortEnableAll(enable))!=RT_ERR_OK)
        return retVal; 
    
    return RT_ERR_OK;

}


/* Function Name:
 *      rtk_port_phyEnableAll_get
 * Description:
 *      Get all PHY enable status.
 * Input:
 *      None
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API can set all PHY status.
 *      The configuration of all PHY is as following:
 *      DISABLE   
 *      ENABLE
 */

rtk_api_ret_t rtk_port_phyEnableAll_get(rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;
  

    if ((retVal = rtl8370_getAsicPortEnableAll(pEnable))!=RT_ERR_OK)
        return retVal; 
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_efid_set
 * Description:
 *      Set port-based enhanced filtering database
 * Input:
 *      port - Port id.
 *      efid - Specified enhanced filtering database.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_L2_FID - Invalid fid.
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can set port-based enhanced filtering database.  
 */
rtk_api_ret_t rtk_port_efid_set(rtk_port_t port, rtk_data_t efid)
{
    rtk_api_ret_t retVal;  

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
  
    /* efid must be 0~7 */
    if (efid > RTK_EFID_MAX)
        return RT_ERR_INPUT;
    
    if ((retVal = rtl8370_setAsicPortIsolationEfid(port, efid))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_efid_get
 * Description:
 *      Get port-based enhanced filtering database
 * Input:
 *      port - Port id.
 * Output:
 *      pEfid - Specified enhanced filtering database.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can get port-based enhanced filtering database status.
 */
rtk_api_ret_t rtk_port_efid_get(rtk_port_t port, rtk_data_t *pEfid)
{
    rtk_api_ret_t retVal; 
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
          
    if ((retVal = rtl8370_getAsicPortIsolationEfid(port, pEfid))!=RT_ERR_OK)
        return retVal;       

    return RT_ERR_OK;
}

#if !defined(_REDUCE_CODE)
/* Function Name:
 *      rtk_leaky_vlan_set
 * Description:
 *      Set VLAN leaky.
 * Input:
 *      type - Packet type for VLAN leaky.
 *      enable - Leaky status.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set VLAN leaky for RMA and IGMP/MLD packets.
 *      The leaky frame types are as following:
 *      LEAKY_BRG_GROUP,
 *      LEAKY_FD_PAUSE,
 *      LEAKY_SP_MCAST,
 *      LEAKY_1X_PAE,
 *      LEAKY_UNDEF_BRG_04,
 *      LEAKY_UNDEF_BRG_05,
 *      LEAKY_UNDEF_BRG_06,
 *      LEAKY_UNDEF_BRG_07,
 *      LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      LEAKY_UNDEF_BRG_09,
 *      LEAKY_UNDEF_BRG_0A,
 *      LEAKY_UNDEF_BRG_0B,
 *      LEAKY_UNDEF_BRG_0C,
 *      LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      LEAKY_8021AB,
 *      LEAKY_UNDEF_BRG_0F,
 *      LEAKY_BRG_MNGEMENT,
 *      LEAKY_UNDEFINED_11,
 *      LEAKY_UNDEFINED_12,
 *      LEAKY_UNDEFINED_13,
 *      LEAKY_UNDEFINED_14,
 *      LEAKY_UNDEFINED_15,
 *      LEAKY_UNDEFINED_16,
 *      LEAKY_UNDEFINED_17,
 *      LEAKY_UNDEFINED_18,
 *      LEAKY_UNDEFINED_19,
 *      LEAKY_UNDEFINED_1A,
 *      LEAKY_UNDEFINED_1B,
 *      LEAKY_UNDEFINED_1C,
 *      LEAKY_UNDEFINED_1D,
 *      LEAKY_UNDEFINED_1E,
 *      LEAKY_UNDEFINED_1F,
 *      LEAKY_GMRP,
 *      LEAKY_GVRP,
 *      LEAKY_UNDEF_GARP_22,
 *      LEAKY_UNDEF_GARP_23,
 *      LEAKY_UNDEF_GARP_24,
 *      LEAKY_UNDEF_GARP_25,
 *      LEAKY_UNDEF_GARP_26,
 *      LEAKY_UNDEF_GARP_27,
 *      LEAKY_UNDEF_GARP_28,
 *      LEAKY_UNDEF_GARP_29,
 *      LEAKY_UNDEF_GARP_2A,
 *      LEAKY_UNDEF_GARP_2B,
 *      LEAKY_UNDEF_GARP_2C,
 *      LEAKY_UNDEF_GARP_2D,
 *      LEAKY_UNDEF_GARP_2E,
 *      LEAKY_UNDEF_GARP_2F,
 *      LEAKY_IGMP,
 *      LEAKY_IPMULTICAST.
 */
rtk_api_ret_t rtk_leaky_vlan_set(rtk_leaky_type_t type, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    uint32 port;
    rtl8370_rma_t rmacfg;
    rtl8370_igmp_t igmpcfg;

    if (type>=LEAKY_END)
        return RT_ERR_INPUT;

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;        
    
    if (type>=0&&type<=LEAKY_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8370_getAsicRma(type, &rmacfg))!=RT_ERR_OK)
            return retVal; 

        rmacfg.vlan_leaky = enable;
        
        if ((retVal = rtl8370_setAsicRma(type, &rmacfg))!=RT_ERR_OK)
            return retVal;
    }
    else if (LEAKY_IPMULTICAST == type)
    {
        for (port = 0; port <=RTK_PORT_ID_MAX; port++)
        {    
            if ((retVal = rtl8370_setAsicIpMulticastVlanLeaky(port,enable))!=RT_ERR_OK)
                return retVal;  
        }     
    }
    else
    {
        if ((retVal = rtl8370_getAsicIgmp(&igmpcfg))!=RT_ERR_OK)
            return retVal;
        
        igmpcfg.vlan_leaky = enable;

        if ((retVal = rtl8370_setAsicIgmp(&igmpcfg))!=RT_ERR_OK)
            return retVal;
    }
 

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_leaky_vlan_get
 * Description:
 *      Get VLAN leaky.
 * Input:
 *      type - Packet type for VLAN leaky.
 * Output:
 *      pEnable - Leaky status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can get VLAN leaky status for RMA and IGMP/MLD packets.
 *      The leaky frame types are as following:
 *      LEAKY_BRG_GROUP,
 *      LEAKY_FD_PAUSE,
 *      LEAKY_SP_MCAST,
 *      LEAKY_1X_PAE,
 *      LEAKY_UNDEF_BRG_04,
 *      LEAKY_UNDEF_BRG_05,
 *      LEAKY_UNDEF_BRG_06,
 *      LEAKY_UNDEF_BRG_07,
 *      LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      LEAKY_UNDEF_BRG_09,
 *      LEAKY_UNDEF_BRG_0A,
 *      LEAKY_UNDEF_BRG_0B,
 *      LEAKY_UNDEF_BRG_0C,
 *      LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      LEAKY_8021AB,
 *      LEAKY_UNDEF_BRG_0F,
 *      LEAKY_BRG_MNGEMENT,
 *      LEAKY_UNDEFINED_11,
 *      LEAKY_UNDEFINED_12,
 *      LEAKY_UNDEFINED_13,
 *      LEAKY_UNDEFINED_14,
 *      LEAKY_UNDEFINED_15,
 *      LEAKY_UNDEFINED_16,
 *      LEAKY_UNDEFINED_17,
 *      LEAKY_UNDEFINED_18,
 *      LEAKY_UNDEFINED_19,
 *      LEAKY_UNDEFINED_1A,
 *      LEAKY_UNDEFINED_1B,
 *      LEAKY_UNDEFINED_1C,
 *      LEAKY_UNDEFINED_1D,
 *      LEAKY_UNDEFINED_1E,
 *      LEAKY_UNDEFINED_1F,
 *      LEAKY_GMRP,
 *      LEAKY_GVRP,
 *      LEAKY_UNDEF_GARP_22,
 *      LEAKY_UNDEF_GARP_23,
 *      LEAKY_UNDEF_GARP_24,
 *      LEAKY_UNDEF_GARP_25,
 *      LEAKY_UNDEF_GARP_26,
 *      LEAKY_UNDEF_GARP_27,
 *      LEAKY_UNDEF_GARP_28,
 *      LEAKY_UNDEF_GARP_29,
 *      LEAKY_UNDEF_GARP_2A,
 *      LEAKY_UNDEF_GARP_2B,
 *      LEAKY_UNDEF_GARP_2C,
 *      LEAKY_UNDEF_GARP_2D,
 *      LEAKY_UNDEF_GARP_2E,
 *      LEAKY_UNDEF_GARP_2F,
 *      LEAKY_IGMP,
 *      LEAKY_IPMULTICAST.  
 */
 rtk_api_ret_t rtk_leaky_vlan_get(rtk_leaky_type_t type, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;
    uint32 port,tmp;
    rtl8370_rma_t rmacfg;
    rtl8370_igmp_t igmpcfg;

    if (type>=LEAKY_END)
        return RT_ERR_INPUT;    
    
    if (type>=0&&type<=LEAKY_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8370_getAsicRma(type, &rmacfg))!=RT_ERR_OK)
            return retVal; 

        *pEnable = rmacfg.vlan_leaky;

    }
    else if (LEAKY_IPMULTICAST == type)
    {
        for (port = 0; port <=RTK_PORT_ID_MAX; port++)
        {    
            if ((retVal = rtl8370_getAsicIpMulticastVlanLeaky(port,&tmp))!=RT_ERR_OK)
                return retVal;
            if (port>0&&(tmp!=*pEnable))
                return RT_ERR_FAILED;  
            *pEnable = tmp;
        }     
    }
    else
    {
        if ((retVal = rtl8370_getAsicIgmp(&igmpcfg))!=RT_ERR_OK)
            return retVal;
        
        *pEnable = igmpcfg.vlan_leaky;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_leaky_portIsolation_set
 * Description:
 *      Set port isolation leaky.
 * Input:
 *      type - Packet type for port isolation leaky.
 *      enable - Leaky status.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set port isolation leaky for RMA and IGMP/MLD packets.
 *      The leaky frame types are as following:
 *      LEAKY_BRG_GROUP,
 *      LEAKY_FD_PAUSE,
 *      LEAKY_SP_MCAST,
 *      LEAKY_1X_PAE,
 *      LEAKY_UNDEF_BRG_04,
 *      LEAKY_UNDEF_BRG_05,
 *      LEAKY_UNDEF_BRG_06,
 *      LEAKY_UNDEF_BRG_07,
 *      LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      LEAKY_UNDEF_BRG_09,
 *      LEAKY_UNDEF_BRG_0A,
 *      LEAKY_UNDEF_BRG_0B,
 *      LEAKY_UNDEF_BRG_0C,
 *      LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      LEAKY_8021AB,
 *      LEAKY_UNDEF_BRG_0F,
 *      LEAKY_BRG_MNGEMENT,
 *      LEAKY_UNDEFINED_11,
 *      LEAKY_UNDEFINED_12,
 *      LEAKY_UNDEFINED_13,
 *      LEAKY_UNDEFINED_14,
 *      LEAKY_UNDEFINED_15,
 *      LEAKY_UNDEFINED_16,
 *      LEAKY_UNDEFINED_17,
 *      LEAKY_UNDEFINED_18,
 *      LEAKY_UNDEFINED_19,
 *      LEAKY_UNDEFINED_1A,
 *      LEAKY_UNDEFINED_1B,
 *      LEAKY_UNDEFINED_1C,
 *      LEAKY_UNDEFINED_1D,
 *      LEAKY_UNDEFINED_1E,
 *      LEAKY_UNDEFINED_1F,
 *      LEAKY_GMRP,
 *      LEAKY_GVRP,
 *      LEAKY_UNDEF_GARP_22,
 *      LEAKY_UNDEF_GARP_23,
 *      LEAKY_UNDEF_GARP_24,
 *      LEAKY_UNDEF_GARP_25,
 *      LEAKY_UNDEF_GARP_26,
 *      LEAKY_UNDEF_GARP_27,
 *      LEAKY_UNDEF_GARP_28,
 *      LEAKY_UNDEF_GARP_29,
 *      LEAKY_UNDEF_GARP_2A,
 *      LEAKY_UNDEF_GARP_2B,
 *      LEAKY_UNDEF_GARP_2C,
 *      LEAKY_UNDEF_GARP_2D,
 *      LEAKY_UNDEF_GARP_2E,
 *      LEAKY_UNDEF_GARP_2F,
 *      LEAKY_IGMP,
 *      LEAKY_IPMULTICAST.
 */
rtk_api_ret_t rtk_leaky_portIsolation_set(rtk_leaky_type_t type, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    uint32 port;
    rtl8370_rma_t rmacfg;
    rtl8370_igmp_t igmpcfg;

    if (type>=LEAKY_END)
        return RT_ERR_INPUT;

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;        
    
    if (type>=0&&type<=LEAKY_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8370_getAsicRma(type, &rmacfg))!=RT_ERR_OK)
            return retVal; 

        rmacfg.portiso_leaky = enable;
        
        if ((retVal = rtl8370_setAsicRma(type, &rmacfg))!=RT_ERR_OK)
            return retVal;
    }
    else if (LEAKY_IPMULTICAST == type)
    {
        for (port = 0; port < RTK_MAX_NUM_OF_PORT; port++)
        {    
            if ((retVal = rtl8370_setAsicIpMulticastPortIsoLeaky(port,enable))!=RT_ERR_OK)
                return retVal;  
        }     
    }
    else
    {
        if ((retVal = rtl8370_getAsicIgmp(&igmpcfg))!=RT_ERR_OK)
            return retVal;
        
        igmpcfg.portiso_leaky = enable;

        if ((retVal = rtl8370_setAsicIgmp(&igmpcfg))!=RT_ERR_OK)
            return retVal;
    } 


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_leaky_portIsolation_get
 * Description:
 *      Get port isolation leaky.
 * Input:
 *      type - Packet type for port isolation leaky.
 * Output:
 *      pEnable - Leaky status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can get port isolation leaky status for RMA and IGMP/MLD packets.
 *      The leaky frame types are as following:
 *      LEAKY_BRG_GROUP,
 *      LEAKY_FD_PAUSE,
 *      LEAKY_SP_MCAST,
 *      LEAKY_1X_PAE,
 *      LEAKY_UNDEF_BRG_04,
 *      LEAKY_UNDEF_BRG_05,
 *      LEAKY_UNDEF_BRG_06,
 *      LEAKY_UNDEF_BRG_07,
 *      LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      LEAKY_UNDEF_BRG_09,
 *      LEAKY_UNDEF_BRG_0A,
 *      LEAKY_UNDEF_BRG_0B,
 *      LEAKY_UNDEF_BRG_0C,
 *      LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      LEAKY_8021AB,
 *      LEAKY_UNDEF_BRG_0F,
 *      LEAKY_BRG_MNGEMENT,
 *      LEAKY_UNDEFINED_11,
 *      LEAKY_UNDEFINED_12,
 *      LEAKY_UNDEFINED_13,
 *      LEAKY_UNDEFINED_14,
 *      LEAKY_UNDEFINED_15,
 *      LEAKY_UNDEFINED_16,
 *      LEAKY_UNDEFINED_17,
 *      LEAKY_UNDEFINED_18,
 *      LEAKY_UNDEFINED_19,
 *      LEAKY_UNDEFINED_1A,
 *      LEAKY_UNDEFINED_1B,
 *      LEAKY_UNDEFINED_1C,
 *      LEAKY_UNDEFINED_1D,
 *      LEAKY_UNDEFINED_1E,
 *      LEAKY_UNDEFINED_1F,
 *      LEAKY_GMRP,
 *      LEAKY_GVRP,
 *      LEAKY_UNDEF_GARP_22,
 *      LEAKY_UNDEF_GARP_23,
 *      LEAKY_UNDEF_GARP_24,
 *      LEAKY_UNDEF_GARP_25,
 *      LEAKY_UNDEF_GARP_26,
 *      LEAKY_UNDEF_GARP_27,
 *      LEAKY_UNDEF_GARP_28,
 *      LEAKY_UNDEF_GARP_29,
 *      LEAKY_UNDEF_GARP_2A,
 *      LEAKY_UNDEF_GARP_2B,
 *      LEAKY_UNDEF_GARP_2C,
 *      LEAKY_UNDEF_GARP_2D,
 *      LEAKY_UNDEF_GARP_2E,
 *      LEAKY_UNDEF_GARP_2F,
 *      LEAKY_IGMP,
 *      LEAKY_IPMULTICAST.  
 */
rtk_api_ret_t rtk_leaky_portIsolation_get(rtk_leaky_type_t type, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;
    uint32 port, tmp;
    rtl8370_rma_t rmacfg;
    rtl8370_igmp_t igmpcfg;

    if (type>=LEAKY_END)
        return RT_ERR_INPUT;    
    
    if (type>=0&&type<=LEAKY_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8370_getAsicRma(type, &rmacfg))!=RT_ERR_OK)
            return retVal; 

        *pEnable = rmacfg.portiso_leaky;

    }
    else if (LEAKY_IPMULTICAST == type)
    {
        for (port = 0; port < RTK_MAX_NUM_OF_PORT; port++)
        {    
            if ((retVal = rtl8370_getAsicIpMulticastPortIsoLeaky(port,&tmp))!=RT_ERR_OK)
                return retVal;
            if (port>0&&(tmp!=*pEnable))
                return RT_ERR_FAILED;  
            *pEnable = tmp;
        }     
    }
    else
    {
        if ((retVal = rtl8370_getAsicIgmp(&igmpcfg))!=RT_ERR_OK)
            return retVal;
        
        *pEnable = igmpcfg.portiso_leaky;
    }


    return RT_ERR_OK;
}

#endif

/* Function Name:
 *      rtk_vlan_init
 * Description:
 *      Initialize VLAN.
 * Input:
 *      None
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      VLAN is disabled by default. User has to call this API to enable VLAN before
 *      using it. And It will set a default VLAN(vid 1) including all ports and set 
 *      all ports PVID to the default VLAN.
 */
rtk_api_ret_t rtk_vlan_init(void)
{
    rtk_api_ret_t retVal;
    uint32 i;
    rtl8370_user_vlan4kentry vlan4K;
    rtl8370_vlanconfiguser vlanMC;

    /* clean 32 VLAN member configuration */
    for (i = 0; i < RTK_MAX_NUM_OF_VLAN_INDEX; i++)
    {    
        vlanMC.evid = 0;
        vlanMC.lurep = 0;
        vlanMC.mbr = 0;        
        vlanMC.msti = 0;            
        vlanMC.fid = 0;
        vlanMC.meteridx = 0;        
        vlanMC.envlanpol= 0;            
        vlanMC.vbpen= 0;
        vlanMC.vbpri= 0;        
        if ((retVal = rtl8370_setAsicVlanMemberConfig(i, &vlanMC))!=RT_ERR_OK)
            return retVal;
        
    }

    /* Set a default VLAN with vid 1 to 4K table for all ports */
    memset(&vlan4K,0,sizeof(rtl8370_user_vlan4kentry));
    vlan4K.vid = 1;
    vlan4K.mbr = RTK_MAX_PORT_MASK;
    vlan4K.untag = RTK_MAX_PORT_MASK;
    vlan4K.fid = 0;    
    if ((retVal = rtl8370_setAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
        return retVal;
    

    /* Also set the default VLAN to 32 member configuration index 0 */
    memset(&vlanMC,0,sizeof(rtl8370_vlanconfiguser));
    vlanMC.evid = 1;
    vlanMC.mbr = RTK_MAX_PORT_MASK;                   
    vlanMC.fid = 0;
    if ((retVal = rtl8370_setAsicVlanMemberConfig(0, &vlanMC))!=RT_ERR_OK)
            return retVal;  

    /* Set all ports PVID to default VLAN and tag-mode to original */    
    for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
    {
        if ((retVal = rtl8370_setAsicVlanPortBasedVID(i, 0, 0))!=RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8370_setAsicVlanEgressTagMode(i, EG_TAG_MODE_ORI))!=RT_ERR_OK)
            return retVal;
    }

    /* enable VLAN */   
    if ((retVal = rtl8370_setAsicVlanFilter(TRUE))!=RT_ERR_OK)
        return retVal;  
       
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_vlan_set
 * Description:
 *      Set a VLAN entry.
 * Input:
 *      vid - VLAN ID to configure.
 *      mbrmsk - VLAN member set portmask.
 *      untagmsk - VLAN untag set portmask.
 *      fid - filtering database.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_FID - Invalid FID.
 *      RT_ERR_VLAN_PORT_MBR_EXIST - Invalid member port mask.
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 * Note:
 *      There are 4K VLAN entry supported. User could configure the member set and untag set
 *      for specified vid through this API. The portmask's bit N means port N.
 *      For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *      FID is for SVL/IVL usage, and the range is 0~4095.
 */
rtk_api_ret_t rtk_vlan_set(rtk_vlan_t vid, rtk_portmask_t mbrmsk, rtk_portmask_t untagmsk, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;
    rtl8370_user_vlan4kentry vlan4K;   
    
    /* vid must be 0~4095 */
    if (vid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    if (mbrmsk.bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_VLAN_PORT_MBR_EXIST;

    if (untagmsk.bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_VLAN_PORT_MBR_EXIST;

    if (fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;

    /* update 4K table */
    memset(&vlan4K,0,sizeof(rtl8370_user_vlan4kentry));    
    vlan4K.vid = vid;            
    vlan4K.mbr = mbrmsk.bits[0];
    vlan4K.untag = untagmsk.bits[0];
    vlan4K.fid = fid;    
    if ((retVal = rtl8370_setAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
            return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_get
 * Description:
 *      Get a VLAN entry.
 * Input:
 *      vid - VLAN ID to configure.
 * Output:
 *      pMbrmsk - VLAN member set portmask.
 *      pUntagmsk - VLAN untag set portmask.
 *      pFid - filtering database. 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 * Note:
 *     The API can get the member set, untag set and fid settings for specified vid.
 */
rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_portmask_t *pMbrmsk, rtk_portmask_t *pUntagmsk, rtk_fid_t *pFid)
{
    rtk_api_ret_t retVal;
    rtl8370_user_vlan4kentry vlan4K;
    
    /* vid must be 0~4095 */
    if (vid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    vlan4K.vid = vid;

    if ((retVal = rtl8370_getAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
        return retVal;    

    pMbrmsk->bits[0] = vlan4K.mbr;
    pUntagmsk->bits[0] = vlan4K.untag;    
    *pFid = vlan4K.fid;
    return RT_ERR_OK;
}

/* Function Name:
 *     rtk_vlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID).
 * Input:
 *      port - Port id.
 *      pvid - Specified VLAN ID.
 *      priority - 802.1p priority for the PVID.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_VLAN_PRIORITY - Invalid priority. 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - VLAN entry not found.
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 * Note:
 *       The API is used for Port-based VLAN. The untagged frame received from the
 *       port will be classified to the specified VLAN and assigned to the specified priority.
 */
rtk_api_ret_t rtk_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority)
{
    rtk_api_ret_t retVal;
    int32 i;
    uint32 j;
      uint32 k;
    uint32 index,empty_idx;
      uint32 gvidx,proc;
    uint32  bUsed,pri;    
    rtl8370_user_vlan4kentry vlan4K;
    rtl8370_vlanconfiguser vlanMC;    
    rtl8370_protocolvlancfg ppb_vlan_cfg;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    /* vid must be 0~4095 */
    if (pvid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    /* priority must be 0~7 */
    if (priority > RTK_DOT1P_PRIORITY_MAX)
        return RT_ERR_VLAN_PRIORITY;


      empty_idx = 0xFFFF;

    for (i = (RTK_MAX_NUM_OF_VLAN_INDEX-1); i >= 0; i--)
    {               
        if ((retVal = rtl8370_getAsicVlanMemberConfig(i, &vlanMC))!=RT_ERR_OK)
            return retVal;
            
        if (pvid == vlanMC.evid)
        {          
            if ((retVal = rtl8370_setAsicVlanPortBasedVID(port,i,priority))!=RT_ERR_OK)
                return retVal;
            
            return RT_ERR_OK;
        }
        else if (vlanMC.evid == 0 && vlanMC.mbr == 0)
        {
            empty_idx = i;
        }
    }


    /*
        vid doesn't exist in 32 member configuration. Find an empty entry in 
        32 member configuration, then copy entry from 4K. If 32 member configuration
        are all full, then find an entry which not used by Port-based VLAN and 
        then replace it with 4K. Finally, assign the index to the port.
    */ 

    if (empty_idx!=0xFFFF)
    {
        vlan4K.vid = pvid;
        if ((retVal = rtl8370_getAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
            return retVal;

        vlanMC.evid = pvid;
        vlanMC.mbr = vlan4K.mbr;                   
        vlanMC.fid = vlan4K.fid;
        vlanMC.msti= vlan4K.msti;
        vlanMC.meteridx= vlan4K.meteridx;
        vlanMC.envlanpol= vlan4K.envlanpol;
        vlanMC.lurep= vlan4K.lurep;    

        if ((retVal = rtl8370_setAsicVlanMemberConfig(empty_idx,&vlanMC))!=RT_ERR_OK)
            return retVal; 

        if ((retVal = rtl8370_setAsicVlanPortBasedVID(port,empty_idx,priority))!=RT_ERR_OK)
            return retVal;  
        
        return RT_ERR_OK;            
     }       

    if ((retVal = rtl8370_getAsic1xGuestVidx(&gvidx))!=RT_ERR_OK)
        return retVal; 

    /* 32 member configuration is full, found a unused entry to replace */
    for (i = 0; i < RTK_MAX_NUM_OF_VLAN_INDEX; i++)
    {    
        bUsed = FALSE;    

        for (j = 0; j < RTK_MAX_NUM_OF_PORT; j++)
        {    
            if ((retVal = rtl8370_getAsicVlanPortBasedVID(j, &index, &pri))!=RT_ERR_OK)
                return retVal;

            if (i == index)/*index i is in use by port j*/
            {
                bUsed = TRUE;
                break;
            } 

            if (i == gvidx)
            {
                if ((retVal = rtl8370_getAsic1xProcConfig(j, &proc))!=RT_ERR_OK)
                    return retVal;
                if (DOT1X_UNAUTH_GVLAN == proc )
                {
                    bUsed = TRUE;
                    break;
                }
            }

            for (k=0;k<=RTK_PROTOVLAN_GROUP_ID_MAX;k++)
            {
                if ((retVal = rtl8370_getAsicVlanPortAndProtocolBased(port, k, &ppb_vlan_cfg)) != RT_ERR_OK)
                    return retVal; 
                if (ppb_vlan_cfg.valid==TRUE && ppb_vlan_cfg.vlan_idx==i)
                {
                    bUsed = TRUE;
                    break;
                }
            }    
        }

        if (FALSE == bUsed)/*found a unused index, replace it*/
        {
            vlan4K.vid = pvid;
            if ((retVal = rtl8370_getAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
                return retVal; 
            vlanMC.mbr = vlan4K.mbr;                   
            vlanMC.fid = vlan4K.fid;
            vlanMC.msti= vlan4K.msti;
            vlanMC.meteridx= vlan4K.meteridx;
            vlanMC.envlanpol= vlan4K.envlanpol;
            vlanMC.lurep= vlan4K.lurep;               
            if ((retVal = rtl8370_setAsicVlanMemberConfig(i,&vlanMC))!=RT_ERR_OK)
                return retVal; 

            if ((retVal = rtl8370_setAsicVlanPortBasedVID(port,i,priority))!=RT_ERR_OK)
                return retVal;    

            return RT_ERR_OK;            
        }
    }    
    
    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_vlan_portPvid_get
 * Description:
 *      Get VLAN ID(PVID) on specified port.
 * Input:
 *      port - Port id.
 * Output:
 *      pPvid - Specified VLAN ID.
 *      pPriority - 802.1p priority for the PVID.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *     The API can get the PVID and 802.1p priority for the PVID of Port-based VLAN.
 */
rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority)
{
    rtk_api_ret_t retVal;
    uint32 index,pri;
    rtl8370_vlanconfiguser vlanMC;    
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicVlanPortBasedVID(port,&index,&pri))!=RT_ERR_OK)
        return retVal; 

    if ((retVal = rtl8370_getAsicVlanMemberConfig(index, &vlanMC))!=RT_ERR_OK)
        return retVal; 

    *pPvid = vlanMC.evid;
    *pPriority = pri;   
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_set
 * Description:
 *      Set VLAN ingress for each port.
 * Input:
 *      port - Port id.
 *      igr_filter - VLAN ingress function enable status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      The status of vlan ingress filter is as following:
 *      DISABLED
 *      ENABLED
 *      While VLAN function is enabled, ASIC will decide VLAN ID for each received frame and get belonged member
 *      ports from VLAN table. If received port is not belonged to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (igr_filter>=RTK_ENABLE_END)
        return RT_ERR_ENABLE;      

    if ((retVal = rtl8370_setAsicVlanIngressFilter(port,igr_filter))!=RT_ERR_OK)
        return retVal; 

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_get
 * Description:
 *      Get VLAN Ingress Filter
 * Input:
 *      port - Port id.
 * Output:
 *      pIgr_filter - VLAN ingress function enable status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *     The API can Get the VLAN ingress filter status.
 *     The status of vlan ingress filter is as following:
 *     DISABLED
 *     ENABLED    
 */

rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_data_t *pIgr_filter)
{
    rtk_api_ret_t retVal;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicVlanIngressFilter(port, pIgr_filter))!=RT_ERR_OK)
        return retVal; 

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN accept_frame_type
 * Input:
 *      port - Port id.
 *      accept_frame_type - accept frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE - Invalid frame type.
 * Note:
 *      The API is used for checking 802.1Q tagged frames.
 *      The accept frame type as following:
 *      ACCEPT_FRAME_TYPE_ALL
 *      ACCEPT_FRAME_TYPE_TAG_ONLY
 *      ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
rtk_api_ret_t rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (accept_frame_type >= ACCEPT_FRAME_TYPE_END)
        return RT_ERR_VLAN_ACCEPT_FRAME_TYPE;    

    if ((retVal = rtl8370_setAsicVlanAccpetFrameType(port, accept_frame_type)) != RT_ERR_OK)
        return retVal; 

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN accept_frame_type
 * Input:
 *      port - Port id.
 * Output:
 *      pAccept_frame_type - accept frame type
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *     The API can Get the VLAN ingress filter.
 *     The accept frame type as following:
 *     ACCEPT_FRAME_TYPE_ALL
 *     ACCEPT_FRAME_TYPE_TAG_ONLY
 *     ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
rtk_api_ret_t rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_data_t *pAccept_frame_type)
{
    rtk_api_ret_t retVal;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicVlanAccpetFrameType(port, pAccept_frame_type)) != RT_ERR_OK)
        return retVal; 

    return RT_ERR_OK;
}    

/* Function Name:
 *      rtk_vlan_vlanBasedPriority_set
 * Description:
 *      Set VLAN priority for each CVLAN.
 * Input:
 *      vid - Specified VLAN ID.
 *      priority - 802.1p priority for the PVID.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 *      RT_ERR_VLAN_PRIORITY - Invalid priority. 
 * Note:
 *      This API is used to set priority per VLAN.
 */
rtk_api_ret_t rtk_vlan_vlanBasedPriority_set(rtk_vlan_t vid, rtk_pri_t priority)
{
    rtk_api_ret_t retVal;
    rtl8370_user_vlan4kentry vlan4K;   
    
    /* vid must be 0~4095 */
    if (vid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    /* priority must be 0~7 */
    if (priority > RTK_DOT1P_PRIORITY_MAX)
        return RT_ERR_VLAN_PRIORITY;

    /* update 4K table */
    vlan4K.vid = vid; 
    if ((retVal = rtl8370_getAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
        return retVal;
    
    vlan4K.vbpen= 1;      
    vlan4K.vbpri= priority;    
    if ((retVal = rtl8370_setAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_vlanBasedPriority_get
 * Description:
 *      Get VLAN priority for each CVLAN.
 * Input:
 *      vid - Specified VLAN ID.
 * Output:
 *      pPriority - 802.1p priority for the PVID.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *     This API is used to set priority per VLAN.
 */
rtk_api_ret_t rtk_vlan_vlanBasedPriority_get(rtk_vlan_t vid, rtk_pri_t *pPriority)
{
    rtk_api_ret_t retVal;
    rtl8370_user_vlan4kentry vlan4K;   
    
    /* vid must be 0~4095 */
    if (vid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    /* update 4K table */
    vlan4K.vid = vid; 
    if ((retVal = rtl8370_getAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
        return retVal;
    
    if (vlan4K.vbpen!=1)
        return RT_ERR_FAILED;    
    
    *pPriority = vlan4K.vbpri;    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_add
 * Description:
 *      Add the protocol-and-port-based vlan to the specified port of device. 
 * Input:
 *      port - Port id.
 *      info - Protocol and port based VLAN configuration information.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 *      RT_ERR_VLAN_PRIORITY - Invalid priority. 
 *      RT_ERR_TBL_FULL - Table is full.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *      The frame type is shown in the following:
 *      FRAME_TYPE_ETHERNET
 *      FRAME_TYPE_RFC1042
 *      FRAME_TYPE_LLCOTHER
 */
rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_add(rtk_port_t port, rtk_vlan_protoAndPortInfo_t info)
{
    rtk_api_ret_t retVal,i;
    uint32 j, k, index, pri;
    uint32 gvidx,proc;
    uint32 exist, empty, used, bUsed;
    rtl8370_protocolgdatacfg ppb_data_cfg;
    rtl8370_protocolvlancfg ppb_vlan_cfg;
    rtl8370_user_vlan4kentry vlan4K;
    rtl8370_vlanconfiguser vlanMC; 
    rtl8370_provlan_frametype tmp;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (info.proto_type > RTK_MAX_NUM_OF_PROTO_TYPE)
        return RT_ERR_OUT_OF_RANGE;

    if (info.frame_type>=FRAME_TYPE_END)
        return RT_ERR_OUT_OF_RANGE;

    if (info.cvid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    if (info.cpri>RTK_DOT1P_PRIORITY_MAX)
        return RT_ERR_VLAN_PRIORITY;    

    exist = 0xFF;
    empty = 0xFF;
    for (i = RTK_PROTOVLAN_GROUP_ID_MAX; i >= 0; i--)
    {    
        if ((retVal = rtl8370_getAsicVlanProtocolBasedGroupData(i,&ppb_data_cfg))!=RT_ERR_OK)
            return retVal;
        tmp = info.frame_type;
        if (ppb_data_cfg.ether_type==info.proto_type&&ppb_data_cfg.frame_type==tmp)
        {
            /*Already exist*/
            exist = i;
            break; 
        }
        else if (ppb_data_cfg.ether_type==0&&ppb_data_cfg.frame_type==0)
        {
            /*find empty index*/
            empty = i;    
        }
    }

    used = 0xFF;
    /*No empty and exist index*/
    if (0xFF == exist && 0xFF == empty)
        return RT_ERR_TBL_FULL;    
    else if (exist<RTK_MAX_NUM_OF_PROTOVLAN_GROUP)
    {
       /*exist index*/
       used =exist;    
    }
    else if (empty<RTK_MAX_NUM_OF_PROTOVLAN_GROUP)
    {
        /*No exist index, but have empty index*/
        ppb_data_cfg.frame_type = info.frame_type;
        ppb_data_cfg.ether_type = info.proto_type;        
        if ((retVal = rtl8370_setAsicVlanProtocolBasedGroupData(empty,&ppb_data_cfg))!=RT_ERR_OK)
            return retVal;    
        used = empty;
    }
    else
        return RT_ERR_FAILED;    
    
    /* 
        Search 32 member configuration to see if the entry already existed.
        If existed, update the priority and assign the index to the port.
    */
    for (i = 0; i < RTK_MAX_NUM_OF_VLAN_INDEX; i++)
    {      
        if ((retVal = rtl8370_getAsicVlanMemberConfig(i, &vlanMC))!=RT_ERR_OK)
            return retVal;
        if (info.cvid== vlanMC.evid)
        {
            if ((retVal = rtl8370_getAsicVlanPortAndProtocolBased(port,used,&ppb_vlan_cfg))!=RT_ERR_OK)
                return retVal;    
            if (FALSE == ppb_vlan_cfg.valid)
            {
                ppb_vlan_cfg.vlan_idx = i;
                ppb_vlan_cfg.valid = TRUE;
                ppb_vlan_cfg.priority = info.cpri;
                if ((retVal = rtl8370_setAsicVlanPortAndProtocolBased(port,used,&ppb_vlan_cfg))!=RT_ERR_OK)
                    return retVal;
                return RT_ERR_OK;
            }
            else
                return RT_ERR_VLAN_EXIST;
        }    
    }

    /*
        vid doesn't exist in 32 member configuration. Find an empty entry in 
        32 member configuration, then copy entry from 4K. If 32 member configuration
        are all full, then find an entry which not used by Port-based VLAN and 
        then replace it with 4K. Finally, assign the index to the port.
    */
    for (i = 0; i < RTK_MAX_NUM_OF_VLAN_INDEX; i++)
    {    
        if (rtl8370_getAsicVlanMemberConfig(i, &vlanMC) != RT_ERR_OK)
            return RT_ERR_FAILED;    

        if (vlanMC.evid == 0 && vlanMC.mbr == 0)
        {
            vlan4K.vid = info.cvid;
            if ((retVal = rtl8370_getAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
                return retVal;

            vlanMC.evid = info.cvid;
            vlanMC.mbr = vlan4K.mbr;                   
            vlanMC.fid = vlan4K.fid;
            vlanMC.msti= vlan4K.msti;
            vlanMC.meteridx= vlan4K.meteridx;
            vlanMC.envlanpol= vlan4K.envlanpol;
            vlanMC.lurep= vlan4K.lurep;    
            
            if ((retVal = rtl8370_setAsicVlanMemberConfig(i,&vlanMC))!=RT_ERR_OK)
                return retVal; 
            
            if ((retVal = rtl8370_getAsicVlanPortAndProtocolBased(port,used,&ppb_vlan_cfg))!=RT_ERR_OK)
                return retVal;    
            if (FALSE == ppb_vlan_cfg.valid)
            {
                ppb_vlan_cfg.vlan_idx = i;
                ppb_vlan_cfg.valid = TRUE;
                ppb_vlan_cfg.priority = info.cpri;
                if ((retVal = rtl8370_setAsicVlanPortAndProtocolBased(port,used,&ppb_vlan_cfg))!=RT_ERR_OK)
                    return retVal;
                return RT_ERR_OK;
            }
            else
                return RT_ERR_VLAN_EXIST;        
        }    
    }    

    if ((retVal = rtl8370_getAsic1xGuestVidx(&gvidx))!=RT_ERR_OK)
        return retVal; 

    /* 32 member configuration is full, found a unused entry to replace */
    for (i = 0; i < RTK_MAX_NUM_OF_VLAN_INDEX; i++)
    {    
        bUsed = FALSE;    
        for (j = 0; j < RTK_MAX_NUM_OF_PORT; j++)
        {    
            if (rtl8370_getAsicVlanPortBasedVID(j, &index, &pri) != RT_ERR_OK)
                return RT_ERR_FAILED;    

            if (i == index)/*index i is in use by port j*/
            {
                bUsed = TRUE;
                break;
            }  

            if (i == gvidx)
            {
                if ((retVal = rtl8370_getAsic1xProcConfig(j, &proc))!=RT_ERR_OK)
                    return retVal;
                if (DOT1X_UNAUTH_GVLAN == proc)
                {
                    bUsed = TRUE;
                    break;
                }
            }
            
            for (k=0;k<=RTK_PROTOVLAN_GROUP_ID_MAX;k++)
            {
                if ((retVal = rtl8370_getAsicVlanPortAndProtocolBased(port, k, &ppb_vlan_cfg)) != RT_ERR_OK)
                    return retVal; 
                if (TRUE == ppb_vlan_cfg.valid && ppb_vlan_cfg.vlan_idx == i)
                {
                    bUsed = TRUE;
                    break;
                }
            }            
        }

        if (FALSE == bUsed) /*found a unused index, replace it*/
        {
            vlan4K.vid = info.cvid;
            if (rtl8370_getAsicVlan4kEntry(&vlan4K) != RT_ERR_OK)
                return RT_ERR_FAILED;

            vlanMC.mbr = vlan4K.mbr;                   
            vlanMC.fid = vlan4K.fid;
            vlanMC.msti= vlan4K.msti;
            vlanMC.meteridx= vlan4K.meteridx;
            vlanMC.envlanpol= vlan4K.envlanpol;
            vlanMC.lurep= vlan4K.lurep;               
            if ((retVal = rtl8370_setAsicVlanMemberConfig(i,&vlanMC))!=RT_ERR_OK)
                return retVal; 

            if ((retVal = rtl8370_getAsicVlanPortAndProtocolBased(port,used,&ppb_vlan_cfg))!=RT_ERR_OK)
                return retVal;    
            if (FALSE == ppb_vlan_cfg.valid)
            {
                ppb_vlan_cfg.vlan_idx = i;
                ppb_vlan_cfg.valid = TRUE;
                ppb_vlan_cfg.priority = info.cpri;
                if ((retVal = rtl8370_setAsicVlanPortAndProtocolBased(port,used,&ppb_vlan_cfg))!=RT_ERR_OK)
                    return retVal;
                return RT_ERR_OK;
            }
            else
                return RT_ERR_VLAN_EXIST;            
        }
    }        

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_get
 * Description:
 *      Get the protocol-and-port-based vlan to the specified port of device. 
 * Input:
 *      port - Port id.
 *      proto_type - protocol-and-port-based vlan protocol type.
 *      frame_type - protocol-and-port-based vlan frame type.
 * Output:
 *      pInfo - Protocol and port based VLAN configuration information.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_TBL_FULL - Table is full.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     The frame type is shown in the following:
 *     FRAME_TYPE_ETHERNET
 *     FRAME_TYPE_RFC1042
 *     FRAME_TYPE_LLCOTHER
 */
rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_get(rtk_port_t port, rtk_vlan_proto_type_t proto_type, rtk_vlan_protoVlan_frameType_t frame_type, rtk_vlan_protoAndPortInfo_t *pInfo)
{
    rtk_api_ret_t retVal;
    uint32 i;
    uint32 ppb_idx;
    rtl8370_vlanconfiguser vlanMC; 
    rtl8370_protocolgdatacfg ppb_data_cfg;
    rtl8370_protocolvlancfg ppb_vlan_cfg;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (proto_type > RTK_MAX_NUM_OF_PROTO_TYPE)
        return RT_ERR_OUT_OF_RANGE;

    if (frame_type>=FRAME_TYPE_END)
        return RT_ERR_OUT_OF_RANGE;    

   ppb_idx = 0;
    
    for (i=0;i<=RTK_PROTOVLAN_GROUP_ID_MAX;i++)
    {
        if ((retVal = rtl8370_getAsicVlanProtocolBasedGroupData(i,&ppb_data_cfg)) != RT_ERR_OK)
            return retVal; 
            
        if (ppb_data_cfg.frame_type == (rtl8370_provlan_frametype)frame_type && ppb_data_cfg.ether_type==proto_type)
        {
            ppb_idx = i;
            break;
        }
        else if (RTK_PROTOVLAN_GROUP_ID_MAX == i)
            return RT_ERR_TBL_FULL; 
    }            
   
    if ((retVal = rtl8370_getAsicVlanPortAndProtocolBased(port,ppb_idx, &ppb_vlan_cfg)) != RT_ERR_OK)
        return retVal;

    if (FALSE == ppb_vlan_cfg.valid)
        return RT_ERR_FAILED; 
    
    if ((retVal = rtl8370_getAsicVlanMemberConfig(ppb_vlan_cfg.vlan_idx, &vlanMC))!=RT_ERR_OK)
        return retVal;
    
    pInfo->frame_type = frame_type;
    pInfo->proto_type = proto_type;
    pInfo->cvid = vlanMC.evid;
    pInfo->cpri = ppb_vlan_cfg.priority;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_del
 * Description:
 *      Delete the protocol-and-port-based vlan from the specified port of device. 
 * Input:
 *      port - Port id.
 *      proto_type - protocol-and-port-based vlan protocol type.
 *      frame_type - protocol-and-port-based vlan frame type.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_TBL_FULL - Table is full.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     The frame type is shown in the following:
 *     FRAME_TYPE_ETHERNET
 *     FRAME_TYPE_RFC1042
 *     FRAME_TYPE_LLCOTHER
 */
rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_del(rtk_port_t port, rtk_vlan_proto_type_t proto_type, rtk_vlan_protoVlan_frameType_t frame_type)
{
    rtk_api_ret_t retVal;
    uint32 i, bUsed;
    uint32 ppb_idx;
    rtl8370_protocolgdatacfg ppb_data_cfg;
    rtl8370_protocolvlancfg ppb_vlan_cfg;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (proto_type > RTK_MAX_NUM_OF_PROTO_TYPE)
        return RT_ERR_OUT_OF_RANGE;

    if (frame_type>=FRAME_TYPE_END)
        return RT_ERR_OUT_OF_RANGE;    

   ppb_idx = 0;

    for (i=0;i<=RTK_PROTOVLAN_GROUP_ID_MAX;i++)
    {
        if ((retVal = rtl8370_getAsicVlanProtocolBasedGroupData(i,&ppb_data_cfg)) != RT_ERR_OK)
            return retVal; 
            
        if (ppb_data_cfg.frame_type == (rtl8370_provlan_frametype)frame_type && ppb_data_cfg.ether_type == proto_type)
        {
            ppb_idx = i;
            ppb_vlan_cfg.valid = FALSE;
            ppb_vlan_cfg.vlan_idx = 0;
            ppb_vlan_cfg.priority = 0;        
            if ((retVal = rtl8370_setAsicVlanPortAndProtocolBased(port,ppb_idx,&ppb_vlan_cfg)) != RT_ERR_OK)
                return retVal;
        }
    }            

    bUsed = FALSE;
    for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
    {    
        if ((retVal = rtl8370_getAsicVlanPortAndProtocolBased(i,ppb_idx, &ppb_vlan_cfg)) != RT_ERR_OK)
            return retVal;
        
        if (TRUE == ppb_vlan_cfg.valid)
        {
            bUsed = TRUE;
                break;
        }        
    }

    if (FALSE == bUsed) /*No Port use this PPB Index, Delete it*/
    {
        ppb_data_cfg.ether_type=0;
        ppb_data_cfg.frame_type=0;
        if ((retVal = rtl8370_setAsicVlanProtocolBasedGroupData(ppb_idx,&ppb_data_cfg))!=RT_ERR_OK)
            return retVal; 
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_delAll
 * Description:
 *     Delete all protocol-and-port-based vlans from the specified port of device. 
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     Delete all flow table protocol-and-port-based vlan entries.
 */
rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_delAll(rtk_port_t port)
{
    rtk_api_ret_t retVal;
    uint32 i, j, bUsed[4];
    rtl8370_protocolgdatacfg ppb_data_cfg;
    rtl8370_protocolvlancfg ppb_vlan_cfg;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    for (i=0;i<=RTK_PROTOVLAN_GROUP_ID_MAX;i++)
    {
        ppb_vlan_cfg.valid = FALSE;
        ppb_vlan_cfg.vlan_idx = 0;
        ppb_vlan_cfg.priority = 0;        
        if ((retVal = rtl8370_setAsicVlanPortAndProtocolBased(port,i,&ppb_vlan_cfg)) != RT_ERR_OK)
            return retVal;
    }            

    bUsed[0] = FALSE;
    bUsed[1] = FALSE;
    bUsed[2] = FALSE;
    bUsed[3] = FALSE;    
    for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
    {    
        for (j=0;j<=RTK_PROTOVLAN_GROUP_ID_MAX;j++)
        {
            if ((retVal = rtl8370_getAsicVlanPortAndProtocolBased(i,j,&ppb_vlan_cfg)) != RT_ERR_OK)
                return retVal;
        
            if (TRUE == ppb_vlan_cfg.valid)
            {
                bUsed[j] = TRUE;
            }
        }
    }
    
    for (i=0;i<=RTK_PROTOVLAN_GROUP_ID_MAX;i++)
    {
        if (FALSE == bUsed[i]) /*No Port use this PPB Index, Delete it*/
        {
            ppb_data_cfg.ether_type=0;
            ppb_data_cfg.frame_type=0;
            if ((retVal = rtl8370_setAsicVlanProtocolBasedGroupData(i,&ppb_data_cfg))!=RT_ERR_OK)
                return retVal; 
        }
    }



    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_tagMode_set
 * Description:
 *      Set CVLAN egress tag mode
 * Input:
 *      port - Port id.
 *      tag_mode - The egress tag mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      The API can set Egress tag mode. There are 4 mode for egress tag:
 *      VLAN_TAG_MODE_ORIGINAL,
 *      VLAN_TAG_MODE_KEEP_FORMAT,
 *      VLAN_TAG_MODE_PRI. 
 *      VLAN_TAG_MODE_REAL_KEEP_FORMAT,
 */
rtk_api_ret_t rtk_vlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;    

    if (tag_mode >= VLAN_TAG_MODE_END)
        return RT_ERR_PORT_ID;
    
    if ((retVal = rtl8370_setAsicVlanEgressTagMode(port, tag_mode))!=RT_ERR_OK)
        return retVal;    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_tagMode_get
 * Description:
 *      Get CVLAN egress tag mode
 * Input:
 *      port - Port id.
 * Output:
 *      pTag_mode - The egress tag mode.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get Egress tag mode. There are 4 mode for egress tag:
 *      VLAN_TAG_MODE_ORIGINAL,
 *      VLAN_TAG_MODE_KEEP_FORMAT,
 *      VLAN_TAG_MODE_REAL_KEEP_FORMAT,
 *      VLAN_TAG_MODE_PRI. 
 */
rtk_api_ret_t rtk_vlan_tagMode_get(rtk_port_t port, rtk_data_t *pTag_mode)
{
    rtk_api_ret_t retVal;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;    
    
    if ((retVal = rtl8370_getAsicVlanEgressTagMode(port, pTag_mode))!=RT_ERR_OK)
        return retVal;    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_stg_set
 * Description:
 *      Set spanning tree group instance of the vlan to the specified device
 * Input:
 *      vid - Specified VLAN ID.
 *      stg - spanning tree group instance.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_MSTI - Invalid msti parameter
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 * Note:
 *      The API can set spanning tree group instance of the vlan to the specified device.
 */
rtk_api_ret_t rtk_vlan_stg_set(rtk_vlan_t vid, rtk_stg_t stg)
{
    rtk_api_ret_t retVal;
    rtl8370_user_vlan4kentry vlan4K;   
    
    /* vid must be 0~4095 */
    if (vid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    /* priority must be 0~15 */
    if (stg >= RTK_MAX_NUM_OF_MSTI)
        return RT_ERR_MSTI;

    /* update 4K table */
    vlan4K.vid = vid; 
    if ((retVal = rtl8370_getAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
        return retVal;
    
    vlan4K.msti= stg;        
    if ((retVal = rtl8370_setAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_stg_get
 * Description:
 *      Get spanning tree group instance of the vlan to the specified device
 * Input:
 *      vid - Specified VLAN ID.
 * Output:
 *      pStg - spanning tree group instance.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 * Note:
 *      The API can get spanning tree group instance of the vlan to the specified device.
 */
rtk_api_ret_t rtk_vlan_stg_get(rtk_vlan_t vid, rtk_stg_t *pStg)
{
    rtk_api_ret_t retVal;
    rtl8370_user_vlan4kentry vlan4K;   
    
    /* vid must be 0~4095 */
    if (vid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    /* update 4K table */
    vlan4K.vid = vid; 
    if ((retVal = rtl8370_getAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
        return retVal;
    
    *pStg = vlan4K.msti;        

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portFid_set
 * Description:
 *      Set port-based filtering database
 * Input:
 *      port - Port id.
 *      enable - ebable port-based FID
 *      fid - Specified filtering database.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_L2_FID - Invalid fid.
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can set port-based filtering database. If the function is enabled, all input
 *      packets will be assigned to the port-based fid regardless vlan tag. 
 */
rtk_api_ret_t rtk_vlan_portFid_set(rtk_port_t port, rtk_enable_t enable, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;  

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_ENABLE;   

    /* fid must be 0~4095 */
    if (fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;
    
    if ((retVal = rtl8370_setAsicPortBasedFidEn(port, enable))!=RT_ERR_OK)
        return retVal;
          
    if ((retVal = rtl8370_setAsicPortBasedFid(port, fid))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portFid_get
 * Description:
 *      Get port-based filtering database
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - ebable port-based FID
 *      pFid - Specified filtering database.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can get port-based filtering database status. If the function is enabled, all input
 *      packets will be assigned to the port-based fid regardless vlan tag.
 */
rtk_api_ret_t rtk_vlan_portFid_get(rtk_port_t port, rtk_data_t *pEnable, rtk_data_t *pFid)
{
    rtk_api_ret_t retVal; 
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if ((retVal = rtl8370_getAsicPortBasedFidEn(port, pEnable))!=RT_ERR_OK)
        return retVal;
          
    if ((retVal = rtl8370_getAsicPortBasedFid(port, pFid))!=RT_ERR_OK)
        return retVal;       

    return RT_ERR_OK;
}

#if !defined(_REDUCE_CODE)
/* Function Name:
 *      rtk_stp_init 
 * Description:
 *      Initialize stp module of the specified device
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      Initialize stp module before calling any vlan APIs
 */
rtk_api_ret_t rtk_stp_init(void)
{
    rtk_api_ret_t retVal;
    uint32 port, msti;

    for (port=0;port<RTK_MAX_NUM_OF_PORT;port++)
    {
        for(msti=0;msti<RTK_MAX_NUM_OF_MSTI;msti++)
        {
            if ((retVal = rtl8370_setAsicSpanningTreeStatus(port, msti, STP_STATE_FORWARDING))!=RT_ERR_OK)
                return retVal;        
        }
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_stp_mstpState_set
 * Description:
 *      Configure spanning tree state per each port.
 * Input:
 *      port - Port id
 *      msti - Multiple spanning tree instance.
 *      stp_state - Spanning tree state for msti
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_MSTI - Invalid msti parameter.
 *      RT_ERR_MSTP_STATE - Invalid STP state.
 * Note:
 *      System supports per-port multiple spanning tree state for each msti. 
 *      There are four states supported by ASIC.
 *      STP_STATE_DISABLED
 *      STP_STATE_BLOCKING
 *      STP_STATE_LEARNING
 *      STP_STATE_FORWARDING
 */
rtk_api_ret_t rtk_stp_mstpState_set(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t stp_state)
{
    rtk_api_ret_t retVal;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if (msti > RTK_MAX_NUM_OF_MSTI)
        return RT_ERR_MSTI;

    if (stp_state >= STP_STATE_END)
        return RT_ERR_MSTP_STATE;

    if ((retVal = rtl8370_setAsicSpanningTreeStatus(port, msti, stp_state))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}    

/* Function Name:
 *      rtk_stp_mstpState_get
 * Description:
 *      Get spanning tree state per each port.
 * Input:
 *      port - Port id.
 *      msti - Multiple spanning tree instance.
 * Output:
 *      pStp_state - Spanning tree state for msti
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_MSTI - Invalid msti parameter.
 * Note:
 *      System supports per-port multiple spanning tree state for each msti. 
 *      There are four states supported by ASIC.
 *      STP_STATE_DISABLED
 *      STP_STATE_BLOCKING
 *      STP_STATE_LEARNING
 *      STP_STATE_FORWARDING
 */
rtk_api_ret_t rtk_stp_mstpState_get(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_data_t *pStp_state)
{
    rtk_api_ret_t retVal;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if ((retVal = rtl8370_getAsicSpanningTreeStatus(port, msti, pStp_state))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}    

/* Function Name:
 *      rtk_l2_init 
 * Description:
 *      Initialize l2 module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      Initialize l2 module before calling any l2 APIs.
 */
rtk_api_ret_t rtk_l2_init(void)
{
    rtk_api_ret_t retVal;
    uint32 i;

    if ((retVal = rtl8370_setAsicLutIpMulticastLookup(DISABLE))!=RT_ERR_OK)
        return retVal;

    /*Enable CAM Usage*/
    if ((retVal = rtl8370_setAsicLutCamTbUsage(ENABLE))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicLutAgeTimerSpeed(RTK_L2_DEFAULT_TIME,RTK_L2_DEFAULT_SPEED))!=RT_ERR_OK)
        return retVal;

    for (i = 0; i <= RTK_PORT_ID_MAX; i++)
    {
        if ((retVal = rtl8370_setAsicLutLearnLimitNo(i,RTK_MAX_NUM_OF_LEARN_LIMIT))!=RT_ERR_OK)
            return retVal;
    }
    
    return RT_ERR_OK;
}   

/* Function Name:
 *      rtk_l2_ipMcastAddrLookup_set
 * Description:
 *      Set IP mulcast lookup function enable/disable.
 * Input:
 *      type - IP mulcast lookup type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model.
 * Note:
 *      The API can set IP mulcast lookup function.
 *      There are 2 states supported by ASIC.
 *      LOOKUP_MAC,
 *      LOOKUP_SIP_DIP,
 */
rtk_api_ret_t rtk_l2_ipMcastAddrLookup_set(rtk_l2_lookup_type_t type)
{
    rtk_api_ret_t retVal;

    if (type >=LOOKUP_END)
        return RT_ERR_INPUT;
    
    if (type ==LOOKUP_DIP)
        return RT_ERR_CHIP_NOT_SUPPORTED;
    
    if ((retVal = rtl8370_setAsicLutIpMulticastLookup(type))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
   
/* Function Name:
 *      rtk_l2_ipMcastAddrLookup_get
 * Description:
 *      Get IP mulcast lookup function enable/disable.
 * Input:
 *      None
 * Output:
 *      pType - IP mulcast lookup type
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get IP mulcast lookup function status.
 */
rtk_api_ret_t rtk_l2_ipMcastAddrLookup_get(rtk_data_t *pType)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicLutIpMulticastLookup(pType))!=RT_ERR_OK)
        return retVal;
   

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_addr_add
 * Description:
 *      Add LUT unicast entry.
 * Input:
 *      pMac - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      pL2_data - Unicast entry parameter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      If the unicast mac address already existed in LUT, it will udpate the status of the entry. 
 *      Otherwise, it will find an empty or asic auto learned entry to write. If all the entries 
 *      with the same hash value can't be replaced, ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
rtk_api_ret_t rtk_l2_addr_add(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t retVal;
    uint32 method;
    rtl8370_luttb l2Table;
        
    /* must be unicast address */
    if ((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if (pL2_data->port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;    

    if (pL2_data->fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;

    if (pL2_data->is_static>= RTK_ENABLE_END)
        return RT_ERR_INPUT; 
    
    if (pL2_data->sa_block>= RTK_ENABLE_END)
        return RT_ERR_INPUT; 

    if (pL2_data->auth>= RTK_ENABLE_END)
        return RT_ERR_INPUT;     

    memset(&l2Table,0,sizeof(rtl8370_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.fid = pL2_data->fid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8370_getAsicL2LookupTb(method,&l2Table);
    if (RT_ERR_OK == retVal )
    {
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
        l2Table.fid = pL2_data->fid;
        l2Table.spa = pL2_data->port;
        l2Table.static_bit = pL2_data->is_static;
        l2Table.block = pL2_data->sa_block;
        l2Table.ipmul = 0;
        l2Table.age = 6;
        retVal = rtl8370_setAsicL2LookupTb(&l2Table);
        return retVal;        
    }    
    else if (RT_ERR_L2_ENTRY_NOTFOUND == retVal )
    {
        memset(&l2Table,0,sizeof(rtl8370_luttb));    
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);    
        l2Table.fid = pL2_data->fid;
        l2Table.spa = pL2_data->port;
        l2Table.static_bit = pL2_data->is_static;
        l2Table.block = pL2_data->sa_block;
        l2Table.ipmul = 0;
        l2Table.age = 6;
        if ((retVal = rtl8370_setAsicL2LookupTb(&l2Table))!=RT_ERR_OK)
            return retVal;
        
        method = LUTREADMETHOD_MAC;
        retVal = rtl8370_getAsicL2LookupTb(method,&l2Table);
        if (RT_ERR_L2_ENTRY_NOTFOUND == retVal )
            return     RT_ERR_L2_INDEXTBL_FULL;
        else
            return retVal;              
    }
    else
        return retVal;             

}

/* Function Name:
 *      rtk_l2_addr_get
 * Description:
 *      Get LUT unicast entry.
 * Input:
 *      pMac - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      pL2_data - Unicast entry parameter. The fid (filtering database) should be added as input parameter
 * Output:
 *      pL2_data - Unicast entry parameter
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      If the unicast mac address existed in LUT, it will return the port and fid where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
rtk_api_ret_t rtk_l2_addr_get(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t retVal;
    uint32 method;
    rtl8370_luttb l2Table;
        
    /* must be unicast address */
    if ((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;  

    if (pL2_data->fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;  

    memset(&l2Table,0,sizeof(rtl8370_luttb));

    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.fid = pL2_data->fid;
    method = LUTREADMETHOD_MAC;

    if ((retVal = rtl8370_getAsicL2LookupTb(method,&l2Table))!=RT_ERR_OK)
        return retVal;
    
    memcpy(pL2_data->mac.octet,pMac->octet,ETHER_ADDR_LEN);
    pL2_data->port = l2Table.spa;
    pL2_data->fid  = l2Table.fid;
    pL2_data->is_static = l2Table.static_bit;
    pL2_data->auth = l2Table.auth;
    pL2_data->sa_block = l2Table.block;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_addr_del
 * Description:
 *      Delete LUT unicast entry.
 * Input:
 *      pMac - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      pL2_data - Unicast entry parameter. The fid (filtering database) should be added as input parameter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      If the mac has existed in the LUT, it will be deleted. Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
rtk_api_ret_t rtk_l2_addr_del(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t retVal;
    uint32 method;
    rtl8370_luttb l2Table;
        
    /* must be unicast address */
    if ((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;  

    if (pL2_data->fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;  

    memset(&l2Table,0,sizeof(rtl8370_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.fid = pL2_data->fid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8370_getAsicL2LookupTb(method,&l2Table);
    if (RT_ERR_OK ==  retVal)
    {
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
        l2Table.fid = pL2_data->fid;
        l2Table.spa = 0;
        l2Table.static_bit = 0;
        l2Table.block = 0;
        l2Table.ipmul = 0;
        l2Table.age = 0;
        retVal = rtl8370_setAsicL2LookupTb(&l2Table);
        return retVal;        
    }    
    else
        return retVal;             

}

/* Function Name:
 *      rtk_l2_mcastAddr_add
 * Description:
 *      Add LUT multicast entry.
 * Input:
 *      pMac - 6 bytes multicast(I/G bit is 1) mac address to be written into LUT.
 *      fid - filtering database for the input LUT entry.
 *      portmask - Port mask to be forwarded to.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK - Invalid portmask.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      If the multicast mac address already existed in the LUT, it will udpate the
 *      port mask of the entry. Otherwise, it will find an empty or asic auto learned
 *      entry to write. If all the entries with the same hash value can't be replaced, 
 *      ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
rtk_api_ret_t rtk_l2_mcastAddr_add(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t portmask)
{
    rtk_api_ret_t retVal;
    uint32 method;    
    rtl8370_luttb l2Table;

    /* must be L2 multicast address */
    if ((pMac == NULL) || (!(pMac->octet[0] & 0x1)))
        return RT_ERR_MAC;

    if (portmask.bits[0]> RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK;    

    if (fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;

    memset(&l2Table,0,sizeof(rtl8370_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.fid = fid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8370_getAsicL2LookupTb(method,&l2Table);
    if (RT_ERR_OK == retVal)
    {  
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
        l2Table.fid = fid;
        l2Table.portmask= portmask.bits[0];
        l2Table.static_bit = 1;
        l2Table.ipmul = 0;
        retVal = rtl8370_setAsicL2LookupTb(&l2Table);
        return retVal;        
    }    
    else if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
    {
        memset(&l2Table,0,sizeof(rtl8370_luttb));    
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);    
        l2Table.fid = fid;
        l2Table.portmask= portmask.bits[0];
        l2Table.static_bit = 1;
        if ((retVal = rtl8370_setAsicL2LookupTb(&l2Table))!=RT_ERR_OK)
            return retVal;
        
        method = LUTREADMETHOD_MAC;
        retVal = rtl8370_getAsicL2LookupTb(method,&l2Table);
        if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
            return     RT_ERR_L2_INDEXTBL_FULL;
        else
            return retVal; 
        
    }
    else
        return retVal;             

}

/* Function Name:
 *      rtk_l2_mcastAddr_get
 * Description:
 *      Get LUT multicast entry.
 * Input:
 *      pMac - 6 bytes multicast(I/G bit is 1) mac address to be written into LUT.
 *      fid - Filtering database
 * Output:
 *      pPortmask - Port mask to be forwarded to.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      If the multicast mac address existed in the LUT, it will return the port where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
rtk_api_ret_t rtk_l2_mcastAddr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    uint32 method;
    rtl8370_luttb l2Table;
        
    /* must be unicast address */
    if ((pMac == NULL) || !(pMac->octet[0] & 0x1))
        return RT_ERR_MAC;  

    if (fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;  

    memset(&l2Table,0,sizeof(rtl8370_luttb));

    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.fid = fid;
    method = LUTREADMETHOD_MAC;

    if ((retVal = rtl8370_getAsicL2LookupTb(method,&l2Table))!=RT_ERR_OK)
        return retVal;

    pPortmask->bits[0] = l2Table.portmask;
      
    return RT_ERR_OK;   
}

/* Function Name:
 *      rtk_l2_mcastAddr_del
 * Description:
 *      Delete LUT multicast entry.
 * Input:
 *      pMac - 6 bytes multicast(I/G bit is 1) mac address to be written into LUT.
 *      fid - Filtering database
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      If the mac has existed in the LUT, it will be deleted. Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
rtk_api_ret_t rtk_l2_mcastAddr_del(rtk_mac_t *pMac, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;
    uint32 method;
    rtl8370_luttb l2Table;
        
    /* must be unicast address */
    if ((pMac == NULL) || !(pMac->octet[0] & 0x1))
        return RT_ERR_MAC;  

    if (fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;  

    memset(&l2Table,0,sizeof(rtl8370_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.fid = fid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8370_getAsicL2LookupTb(method,&l2Table);
    if (RT_ERR_OK == retVal)
    { 
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);    
        l2Table.fid = fid;        
        l2Table.portmask= 0;
        l2Table.static_bit = 0;
        l2Table.block = 0;
        l2Table.ipmul = 0;
        retVal = rtl8370_setAsicL2LookupTb(&l2Table);
        return retVal;        
    }    
    else
        return retVal;             
  
}

/* Function Name:
 *      rtk_l2_ipMcastAddr_add
 * Description:
 *      Add Lut IP multicast entry
 * Input:
 *      sip - Source IP Address.
 *      dip - Destination IP Address.
 *      portmask - Destination port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK - Invalid portmask.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      System supports L2 entry with IP multicast DIP/SIP to forward IP multicasting frame as user 
 *      desired. If this function is enabled, then system will be looked up L2 IP multicast entry to 
 *      forward IP multicast frame directly without flooding.
 */
rtk_api_ret_t rtk_l2_ipMcastAddr_add(ipaddr_t sip, ipaddr_t dip, rtk_portmask_t portmask)
{
    rtk_api_ret_t retVal;
    uint32 method;
    rtl8370_luttb l2Table;

    if (portmask.bits[0]> RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_ID; 
    
    l2Table.sip = sip;
    l2Table.dip = dip;
    l2Table.ipmul = 1;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8370_getAsicL2LookupTb(method,&l2Table);
    if (RT_ERR_OK == retVal)
    { 
        l2Table.sip = sip;
        l2Table.dip = dip;    
        l2Table.portmask= portmask.bits[0];
        l2Table.static_bit = 1;
        l2Table.ipmul = 1;
        retVal = rtl8370_setAsicL2LookupTb(&l2Table);
        return retVal;        
    }    
    else if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
    {
        memset(&l2Table,0,sizeof(rtl8370_luttb));    
        l2Table.sip = sip;
        l2Table.dip = dip;
        l2Table.portmask= portmask.bits[0];
        l2Table.static_bit = 1;
        l2Table.ipmul = 1;        
        if ((retVal = rtl8370_setAsicL2LookupTb(&l2Table))!=RT_ERR_OK)
            return retVal;
                
        method = LUTREADMETHOD_MAC;
        retVal = rtl8370_getAsicL2LookupTb(method,&l2Table);
        if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
            return     RT_ERR_L2_INDEXTBL_FULL;
        else
            return retVal; 
        
    }
    else
        return retVal;                

}

/* Function Name:
 *      rtk_l2_ipMcastAddr_get
 * Description:
 *      Get LUT IP multicast entry.
 * Input:
 *      sip - Source IP Address.
 *      dip - Destination IP Address.
 * Output:
 *      pPortmask - Destination port mask. 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      The API can get Lut table of IP multicast entry.
 */
rtk_api_ret_t rtk_l2_ipMcastAddr_get(ipaddr_t sip, ipaddr_t dip, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    uint32 method;
    rtl8370_luttb l2Table;

    l2Table.sip = sip;
    l2Table.dip = dip;
    l2Table.ipmul = 1;
    method = LUTREADMETHOD_MAC;
    if ((retVal = rtl8370_getAsicL2LookupTb(method,&l2Table))!=RT_ERR_OK)
        return retVal;
    
     pPortmask->bits[0] = l2Table.portmask;
    
    return RT_ERR_OK;
}    

/* Function Name:
 *      rtk_l2_ipMcastAddr_del
 * Description:
 *      Delete a ip multicast address entry from the specified device.
 * Input:
 *      sip - Source IP Address.
 *      dip - Destination IP Address. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      The API can delete a IP multicast address entry from the specified device.
 */
rtk_api_ret_t rtk_l2_ipMcastAddr_del(ipaddr_t sip, ipaddr_t dip)
{
    rtk_api_ret_t retVal;
    uint32 method;
    rtl8370_luttb l2Table;

    l2Table.sip = sip;
    l2Table.dip = dip;
    l2Table.ipmul = 1;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8370_getAsicL2LookupTb(method,&l2Table);
    if (RT_ERR_OK == retVal)
    { 
        l2Table.sip = sip;
        l2Table.dip = dip;    
        l2Table.portmask= 0;
        l2Table.static_bit = 0;
        l2Table.ipmul = 1;
        retVal = rtl8370_setAsicL2LookupTb(&l2Table);
        return retVal;        
    }    
    else
        return retVal; 

}    

/* Function Name:
 *      rtk_l2_flushType_set
 * Description:
 *      Flush L2 mac address by type in the specified device.
 * Input:
 *      type - flush type
 *      vid - VLAN id
 *      portOrTid - port id or trunk id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This function trigger flushing of per-port L2 learning.
 *      When flushing operaton completes, the corresponding bit will be clear.
 *      The flush type as following:
 *      FLUSH_TYPE_BY_PORT        (physical port)
 */
rtk_api_ret_t rtk_l2_flushType_set(rtk_l2_flushType_t type, rtk_vlan_t vid, rtk_l2_flushItem_t portOrTid)
{
    rtk_api_ret_t retVal;

    if (type>=FLUSH_TYPE_END)
        return RT_ERR_INPUT;   

    if (portOrTid > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (vid != 0)
        return RT_ERR_VLAN_VID;
    
    switch (type)
    {
        case FLUSH_TYPE_BY_PORT:
            if ((retVal = rtl8370_setAsicPortForceFlush(1<<portOrTid))!=RT_ERR_OK)
                return retVal; 
            break;
        default:
            break;            
    }

    return RT_ERR_OK;
}    

/* Function Name:
 *      rtk_l2_flushLinkDownPortAddrEnable_set
 * Description:
 *      Set HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      port - Port id.
 *      enable - link down flush status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      The status of flush linkdown port address is as following:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_l2_flushLinkDownPortAddrEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (port !=RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;    
    
    if (enable >=RTK_ENABLE_END)
        return RT_ERR_ENABLE;    

    if ((retVal = rtl8370_setAsicLutLinkDownForceAging(enable))!=RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}    

/* Function Name:
 *      rtk_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - link down flush status
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      The status of flush linkdown port address is as following:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_l2_flushLinkDownPortAddrEnable_get(rtk_port_t port, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;
    
    if (port !=RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;    

    if ((retVal = rtl8370_getAsicLutLinkDownForceAging(pEnable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}  

/* Function Name:
 *      rtk_l2_agingEnable_set
 * Description:
 *      Set L2 LUT aging status per port setting.
 * Input:
 *      port - Port id.
 *      enable - Aging status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
rtk_api_ret_t rtk_l2_agingEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (enable >=RTK_ENABLE_END)
        return RT_ERR_ENABLE;    

    if ((retVal = rtl8370_setAsicPortDisableAging(port,enable))!=RT_ERR_OK)
        return retVal; 
    
    return RT_ERR_OK;
}    

/* Function Name:
 *      rtk_l2_agingEnable_get
 * Description:
 *      Get L2 LUT aging status per port setting.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Aging status
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      This API can be used to get L2 LUT aging function per port. 
 */
rtk_api_ret_t rtk_l2_agingEnable_get(rtk_port_t port, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicPortDisableAging(port, pEnable))!=RT_ERR_OK)
        return retVal; 
    
    return RT_ERR_OK;
}    


/* Function Name:
 *      rtk_l2_limitLearningCnt_set
 * Description:
 *      Set per-Port auto learning limit number
 * Input:
 *      port - Port id.
 *      mac_cnt - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_LIMITED_L2ENTRY_NUM - Invalid auto learning limit number
 * Note:
 *      The API can set per-port ASIC auto learning limit number from 0(disable learning) 
 *      to 8k. 
 */
rtk_api_ret_t rtk_l2_limitLearningCnt_set(rtk_port_t port, rtk_mac_cnt_t mac_cnt)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (mac_cnt > RTK_MAX_NUM_OF_LEARN_LIMIT)
        return RT_ERR_LIMITED_L2ENTRY_NUM;

    if ((retVal = rtl8370_setAsicLutLearnLimitNo(port,mac_cnt))!=RT_ERR_OK)
        return retVal; 
    
    return RT_ERR_OK;
}    

/* Function Name:
 *      rtk_l2_limitLearningCnt_get
 * Description:
 *      Get per-Port auto learning limit number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - Auto learning entries limit number
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      The API can get per-port ASIC auto learning limit number.
 */
rtk_api_ret_t rtk_l2_limitLearningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicLutLearnLimitNo(port,pMac_cnt))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l2_limitLearningCntAction_set
 * Description:
 *      Configure auto learn over limit number action.
 * Input:
 *      port - Port id.
 *      action - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_NOT_ALLOWED - Invalid learn over action
 * Note:
 *      The API can set SA unknown packet action while auto learn limit number is over 
 *      The action symbol as following:
 *      LIMIT_LEARN_CNT_ACTION_DROP,
 *      LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
rtk_api_ret_t rtk_l2_limitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t action)
{
    rtk_api_ret_t retVal;
    uint32 data;
    
    if (port !=RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;    
    
    if ( LIMIT_LEARN_CNT_ACTION_DROP == action )
        data = 1;
    else if ( LIMIT_LEARN_CNT_ACTION_FORWARD == action )
        data = 0;
    else if ( LIMIT_LEARN_CNT_ACTION_TO_CPU == action )
        data = 2;
    else
        return RT_ERR_NOT_ALLOWED;    

    
    if ((retVal = rtl8370_setAsicLutLearnOverAct(data))!=RT_ERR_OK)
        return retVal; 

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitLearningCntAction_get
 * Description:
 *      Get auto learn over limit number action.
 * Input:
 *      port - Port id.
 * Output:
 *      pAction - Learn over action
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      The API can get SA unknown packet action while auto learn limit number is over 
 *      The action symbol as following:
 *      LIMIT_LEARN_CNT_ACTION_DROP,
 *      LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      LIMIT_LEARN_CNT_ACTION_TO_CPU, 
 */

rtk_api_ret_t rtk_l2_limitLearningCntAction_get(rtk_port_t port, rtk_data_t *pAction)
{
    rtk_api_ret_t retVal;
    uint32 action;
    
    if (port !=RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;    
    
    if ((retVal = rtl8370_getAsicLutLearnOverAct(&action))!=RT_ERR_OK)
        return retVal; 

    if ( 1 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_DROP;
    else if ( 0 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_FORWARD;
    else if ( 2 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    else
    *pAction = action; 

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_learningCnt_get
 * Description:
 *      Get per-Port current auto learning number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - ASIC auto learning entries number
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      The API can get per-port ASIC auto learning number
 */
rtk_api_ret_t rtk_l2_learningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt)
{
    rtk_api_ret_t retVal;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if ((retVal = rtl8370_getAsicLutLearnNo(port,pMac_cnt))!=RT_ERR_OK)
        return retVal; 
            
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_floodPortMask_set
 * Description:
 *      Set flooding portmask
 * Input:
 *      type - flooding type.
 *      flood_portmask - flooding porkmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK - Invalid portmask.
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can set the flooding mask.
 *      The flooding type is as following:
 *      FLOOD_UNKNOWNDA
 *      FLOOD_UNKNOWNMC
 *      FLOOD_BC
 */
rtk_api_ret_t rtk_l2_floodPortMask_set(rtk_l2_flood_type_t floood_type, rtk_portmask_t flood_portmask)
{
    rtk_api_ret_t retVal;

    if (floood_type >= FLOOD_END)
        return RT_ERR_INPUT; 

    if (flood_portmask.bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK; 
    

    switch (floood_type)
    {
        case FLOOD_UNKNOWNDA:
            if ((retVal = rtl8370_setAsicPortUnknownDaFloodingPortmask(flood_portmask.bits[0]))!=RT_ERR_OK)
                return retVal; 
            break;
        case FLOOD_UNKNOWNMC:
            if ((retVal = rtl8370_setAsicPortUnknownMulticastFloodingPortmask(flood_portmask.bits[0]))!=RT_ERR_OK)
                return retVal; 
            break;
        case FLOOD_BC:
            if ((retVal = rtl8370_setAsicPortBcastFloodingPortmask(flood_portmask.bits[0]))!=RT_ERR_OK)
                return retVal; 
            break;
        default:
            break;            
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_floodPortMask_get
 * Description:
 *      Get flooding portmask
 * Input:
 *      type - flooding type.
 * Output:
 *      pFlood_portmask - flooding porkmask
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      This API can get the flooding mask.
 *      The flooding type is as following:
 *      FLOOD_UNKNOWNDA
 *      FLOOD_UNKNOWNMC
 *      FLOOD_BC 
 */
rtk_api_ret_t rtk_l2_floodPortMask_get(rtk_l2_flood_type_t floood_type, rtk_portmask_t *pFlood_portmask)
{
    rtk_api_ret_t retVal;
    
    if (floood_type >= FLOOD_END)
        return RT_ERR_INPUT; 

    switch (floood_type)
    {
        case FLOOD_UNKNOWNDA:
            if ((retVal = rtl8370_getAsicPortUnknownDaFloodingPortmask(&pFlood_portmask->bits[0]))!=RT_ERR_OK)
                return retVal; 
            break;
        case FLOOD_UNKNOWNMC:
            if ((retVal = rtl8370_getAsicPortUnknownMulticastFloodingPortmask(&pFlood_portmask->bits[0]))!=RT_ERR_OK)
                return retVal; 
            break;
        case FLOOD_BC:
            if ((retVal = rtl8370_getAsicPortBcastFloodingPortmask(&pFlood_portmask->bits[0]))!=RT_ERR_OK)
                return retVal; 
            break;
        default:
            break;            
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_localPktPermit_set
 * Description:
 *      Set permittion of frames if source port and destination port are the same.
 * Input:
 *      port - Port id.
 *      permit - permittion status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid permit value.
 * Note:
 *      This API is setted to permit frame if its source port is equal to destination port.
 */
rtk_api_ret_t rtk_l2_localPktPermit_set(rtk_port_t port, rtk_enable_t permit)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (permit >=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8370_setAsicPortBlockSpa(port,permit))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_localPktPermit_get
 * Description:
 *      Get permittion of frames if source port and destination port are the same.
 * Input:
 *      port - Port id.
 * Output:
 *      pPermit - permittion status
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      This API is to get permittion status for frames if its source port is equal to destination port.
 */
rtk_api_ret_t rtk_l2_localPktPermit_get(rtk_port_t port, rtk_data_t *pPermit)
{
    rtk_api_ret_t retVal;
	
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicPortBlockSpa(port, pPermit))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_aging_set
 * Description:
 *      Set LUT agging out speed
 * Input:
 *      aging_time - Agging out time.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can set LUT agging out period for each entry and the range is from 14s to 800s.
 */
rtk_api_ret_t rtk_l2_aging_set(rtk_data_t aging_time) 
{
    uint32 i;
    CONST_T uint32 agePara[19][3] = {{14,0,1},{28,0,2},{42,0,3},{57,0,4},{71,0,5},
                                   {85,0,6},{100,0,7},{114,1,4},{142,1,5},{171,1,6},
                                   {200,1,7},{228,2,4},{284,2,5},{342,2,6},{400,2,7},
                                   {456,3,4},{568,3,5},{684,3,6},{800,3,7}};

    if (aging_time>agePara[18][0])
        return RT_ERR_OUT_OF_RANGE;
    
    for (i=0;i<19;i++)
    {
        if (aging_time<=agePara[i][0])
        {
            return rtl8370_setAsicLutAgeTimerSpeed(agePara[i][2],agePara[i][1]);
        }            
    }

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_l2_aging_get
 * Description:
 *      Get LUT agging out time
 * Input:
 *      None
 * Output:
 *      pEnable - Aging status
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      The API can get LUT agging out period for each entry. 
 */
rtk_api_ret_t rtk_l2_aging_get(rtk_data_t *pAging_time)
{
    rtk_api_ret_t retVal;
    uint32 i,time,speed;
    CONST_T uint32 agePara[19][3] = {{14,0,1},{28,0,2},{42,0,3},{57,0,4},{71,0,5},
                                   {85,0,6},{100,0,7},{114,1,4},{142,1,5},{171,1,6},
                                   {200,1,7},{228,2,4},{284,2,5},{342,2,6},{400,2,7},
                                   {456,3,4},{568,3,5},{684,3,6},{800,3,7}};    

    if ((retVal = rtl8370_getAsicLutAgeTimerSpeed(&time,&speed))!=RT_ERR_OK)
        return retVal;
    
    for (i=0;i<19;i++)
    {
        if (time==agePara[i][2]&&speed==agePara[i][1])
        {
            *pAging_time = agePara[i][0];
            return RT_ERR_OK;
        }            
    }
    
    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_l2_addr_get
 * Description:
 *      Get LUT unicast entry.
 * Input:
 *      pL2_entry - Index field in the structure.
 * Output:
 *      pL2_entry - other fields such as MAC, port, age...
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_L2_EMPTY_ENTRY - Empty LUT entry.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      This API is used to get address by index from 1~8256.
 */
rtk_api_ret_t rtk_l2_entry_get(rtk_l2_addr_table_t *pL2_entry)
{
    rtk_api_ret_t retVal;
    uint32 method;
    rtl8370_luttb l2Table;
        
    if ((pL2_entry->index>RTK_MAX_NUM_OF_LEARN_LIMIT)||(pL2_entry->index<1))
        return RT_ERR_INPUT;
        
    l2Table.address= pL2_entry->index-1;          
    method = LUTREADMETHOD_ADDRESS;
    if ((retVal = rtl8370_getAsicL2LookupTb(method,&l2Table))!=RT_ERR_OK)
        return retVal; 

    if ((pL2_entry->index>=RTK_MAX_NUM_OF_LEARN_LIMIT)&&(l2Table.valid==0))
         return RT_ERR_L2_EMPTY_ENTRY;       

    if(l2Table.ipmul)
    {
        memset(&pL2_entry->mac,0,sizeof(rtk_mac_t));
        pL2_entry->is_ipmul  = l2Table.ipmul;
        pL2_entry->sip       = l2Table.sip;
        pL2_entry->dip       = l2Table.dip;
        pL2_entry->is_static = l2Table.static_bit;
        pL2_entry->portmask  = l2Table.portmask;
        pL2_entry->fid       = 0;
        pL2_entry->age       = 0;
        pL2_entry->auth      = 0;
        pL2_entry->sa_block  = 0;         
    }
    else if(l2Table.mac.octet[0]&0x01)
    {
        memset(&pL2_entry->sip,0,sizeof(ipaddr_t));
        memset(&pL2_entry->dip,0,sizeof(ipaddr_t));
        pL2_entry->mac.octet[0] = l2Table.mac.octet[0];
        pL2_entry->mac.octet[1] = l2Table.mac.octet[1];
        pL2_entry->mac.octet[2] = l2Table.mac.octet[2];
        pL2_entry->mac.octet[3] = l2Table.mac.octet[3];
        pL2_entry->mac.octet[4] = l2Table.mac.octet[4];
        pL2_entry->mac.octet[5] = l2Table.mac.octet[5];
        pL2_entry->is_ipmul  = l2Table.ipmul;
        pL2_entry->is_static = l2Table.static_bit;
        pL2_entry->portmask  = l2Table.portmask;
        pL2_entry->fid       = l2Table.fid;
        pL2_entry->auth      = l2Table.auth;
        pL2_entry->sa_block  = l2Table.block; 
        pL2_entry->age       = 0; 
    }
    else if((l2Table.age != 0)||(l2Table.static_bit == 1))
    {
        memset(&pL2_entry->sip,0,sizeof(ipaddr_t));
        memset(&pL2_entry->dip,0,sizeof(ipaddr_t));
        pL2_entry->mac.octet[0] = l2Table.mac.octet[0];
        pL2_entry->mac.octet[1] = l2Table.mac.octet[1];
        pL2_entry->mac.octet[2] = l2Table.mac.octet[2];
        pL2_entry->mac.octet[3] = l2Table.mac.octet[3];
        pL2_entry->mac.octet[4] = l2Table.mac.octet[4];
        pL2_entry->mac.octet[5] = l2Table.mac.octet[5];
        pL2_entry->is_ipmul  = l2Table.ipmul;
        pL2_entry->is_static = l2Table.static_bit;
        pL2_entry->portmask  = 1<<(l2Table.spa);
        pL2_entry->fid       = l2Table.fid;
        pL2_entry->auth      = l2Table.auth;
        pL2_entry->sa_block  = l2Table.block; 
        pL2_entry->age       = l2Table.age;               
    }
    else
       return RT_ERR_L2_EMPTY_ENTRY;  
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_init 
 * Description:
 *      Initialize SVLAN Configuration
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design. 
 *      User can set mathced ether type as service provider supported protocol. 
 */
rtk_api_ret_t rtk_svlan_init(void)
{
    uint32 i;
    rtk_api_ret_t retVal;
    rtl8370_svlan_memconf_t svlanMemConf;
    rtl8370_svlan_s2c_t svlanSP2CConf;
    rtl8370_svlan_mc2s_t svlanMC2SConf;

    /*default use C-priority*/
    if ((retVal = rtl8370_setAsicSvlanPrioritySel(SPRISEL_CTAGPRI))!=RT_ERR_OK)
        return retVal;
    
    /*Drop SVLAN untag frame*/
    if ((retVal = rtl8370_setAsicSvlanIngressUntag(DISABLED))!=RT_ERR_OK)
        return retVal;
    
    /*Drop SVLAN unmatch frame*/
    if ((retVal = rtl8370_setAsicSvlanIngressUnmatch(DISABLED))!=RT_ERR_OK)
        return retVal;
    
    /*Set TPID to 0x88a8*/
    if ((retVal = rtl8370_setAsicSvlanTpid(RTK_SVLAN_TPID))!=RT_ERR_OK)
        return retVal;
    
    /*Clean Uplink Port Mask to none*/
    if ((retVal = rtl8370_setAsicSvlanUplinkPortMask(0))!=RT_ERR_OK)
        return retVal;

    /*Clean SVLAN Member Configuration*/
    for (i=0; i<RTK_MAX_NUM_OF_SVLAN_INDEX; i++)
    {       
        memset(&svlanMemConf,0,sizeof(rtl8370_svlan_memconf_t));     
        if ((retVal = rtl8370_setAsicSvlanMemberConfiguration(i, &svlanMemConf))!=RT_ERR_OK)
            return retVal;
    }

    /*Clean C2S Configuration*/
    for (i=0; i<RTK_MAX_NUM_OF_C2S_INDEX; i++)
    {       
        if ((retVal = rtl8370_setAsicSvlanC2SConf(i,0,0,0))!=RT_ERR_OK)
            return retVal;
    }
    
    /*Clean SP2C Configuration*/
    for (i=0; i <RTK_MAX_NUM_OF_SP2C_INDEX ; i++)
    {       
        memset(&svlanSP2CConf,0,sizeof(rtl8370_svlan_s2c_t));
        if ((retVal = rtl8370_setAsicSvlanSP2CConf(i,&svlanSP2CConf))!=RT_ERR_OK)
            return retVal;
    }
    
    /*Clean MC2S Configuration*/
    for (i=0 ; i<RTK_MAX_NUM_OF_MC2S_INDEX; i++)
    {       
        memset(&svlanMC2SConf,0,sizeof(rtl8370_svlan_mc2s_t));
        if ((retVal = rtl8370_setAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
            return retVal;
    }
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_servicePort_add
 * Description:
 *      Add one service port in the specified device
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must 
 *      contain accept SVID in S-tag field.
 */
rtk_api_ret_t rtk_svlan_servicePort_add(rtk_port_t port)
{
    rtk_api_ret_t retVal;
    uint32 pmsk;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;     

    if ((retVal = rtl8370_getAsicSvlanUplinkPortMask(&pmsk))!=RT_ERR_OK)
        return retVal;

    pmsk = pmsk | (1<<port);
    
    if ((retVal = rtl8370_setAsicSvlanUplinkPortMask(pmsk))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_servicePort_get
 * Description:
 *      Get service ports in the specified device.
 * Input:
 *      None
 * Output:
 *      pSvlan_portmask - pointer buffer of svlan ports.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must 
 *      contain accept SVID in S-tag field. 
 */
rtk_api_ret_t rtk_svlan_servicePort_get(rtk_portmask_t *pSvlan_portmask)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicSvlanUplinkPortMask(&pSvlan_portmask->bits[0]))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_servicePort_del
 * Description:
 *      Delete one service port in the specified device
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      This API is removing SVLAN service port in the specified device.
 */
rtk_api_ret_t rtk_svlan_servicePort_del(rtk_port_t port)
{
    rtk_api_ret_t retVal;
    uint32 pmsk;
    
    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;     

    if ((retVal = rtl8370_getAsicSvlanUplinkPortMask(&pmsk))!=RT_ERR_OK)
        return retVal;

    pmsk = pmsk & ~(1<<port);
    
    if ((retVal = rtl8370_setAsicSvlanUplinkPortMask(pmsk))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Configure accepted S-VLAN ether type. 
 * Input:
 *      svlan_tag_id - Ether type of S-tag frame parsing in uplink ports.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design. 
 *      User can set mathced ether type as service provider supported protocol. 
 */
rtk_api_ret_t rtk_svlan_tpidEntry_set(rtk_svlan_tpid_t svlan_tag_id)
{
    rtk_api_ret_t retVal;

    if (svlan_tag_id>RTK_MAX_NUM_OF_PROTO_TYPE)
        return RT_ERR_INPUT;
    
    if ((retVal = rtl8370_setAsicSvlanTpid(svlan_tag_id))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get accepted S-VLAN ether type setting.
 * Input:
 *      None
 * Output:
 *      pSvlan_tag_id -  Ether type of S-tag frame parsing in uplink ports.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must 
 *      contain accept SVID in S-tag field. 
 */
rtk_api_ret_t rtk_svlan_tpidEntry_get(rtk_svlan_tpid_t *pSvlan_tag_id)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicSvlanTpid(pSvlan_tag_id))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_priorityRef_set
 * Description:
 *      Set S-VLAN upstream priority reference setting.
 * Input:
 *      ref - reference selection parameter.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *      The API can set the upstream SVLAN tag priority reference source. The related priority
 *      sources are as following:
 *      REF_INTERNAL_PRI,
 *      REF_CTAG_PRI,
 *      REF_SVLAN_PRI.
 */
rtk_api_ret_t rtk_svlan_priorityRef_set(rtk_svlan_pri_ref_t ref)
{
    rtk_api_ret_t retVal;

    if (ref>REF_PRI_END)
        return RT_ERR_INPUT;
    
    if ((retVal = rtl8370_setAsicSvlanPrioritySel(ref))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_priorityRef_get
 * Description:
 *      Get S-VLAN upstream priority reference setting.
 * Input:
 *      None
 * Output:
 *      pRef - reference selection parameter.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      The API can get the upstream SVLAN tag priority reference source. The related priority
 *      sources are as following:
 *      REF_INTERNAL_PRI,
 *      REF_CTAG_PRI,
 *      REF_SVLAN_PRI. 
 */
rtk_api_ret_t rtk_svlan_priorityRef_get(rtk_data_t *pRef)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicSvlanPrioritySel(pRef))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_memberPortEntry_set
 * Description:
 *      Configure system SVLAN member content
 * Input:
 *      svid - SVLAN id
 *      psvlan_cfg - SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_SVLAN_VID - Invalid SVLAN VID parameter.
 *      RT_ERR_PORT_MASK - Invalid portmask.
 *      RT_ERR_SVLAN_TABLE_FULL - SVLAN configuration is full. 
 * Note:
 *      The API can set system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped by default setup.
 *      rtk_svlan_memberCfg_t->svid is SVID of SVLAN member configuration.
 *      rtk_svlan_memberCfg_t->memberport is member port mask of SVLAN member configuration.
 *      rtk_svlan_memberCfg_t->fid is filtering database of SVLAN member configuration.
 *      rtk_svlan_memberCfg_t->priority is priority of SVLAN member configuration.
 */
rtk_api_ret_t rtk_svlan_memberPortEntry_set(rtk_vlan_t svid, rtk_svlan_memberCfg_t *pSvlan_cfg)
{
    rtk_api_ret_t retVal;
    int32 i, j, k;
    uint32 empty_idx,bUsed;
    uint32 evid,pmsk,svidx;
    rtl8370_svlan_memconf_t svlanMemConf;
    rtl8370_svlan_s2c_t svlanSP2CConf;
    rtl8370_svlan_mc2s_t svlanMC2SConf;

    if ((svid > RTK_VLAN_ID_MAX) || (svid != pSvlan_cfg->svid))
        return RT_ERR_SVLAN_VID;

    if (pSvlan_cfg->svid>RTK_VLAN_ID_MAX)
        return RT_ERR_SVLAN_VID; 

    if (pSvlan_cfg->memberport > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK;

    if (pSvlan_cfg->fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;

    if (pSvlan_cfg->priority > RTK_DOT1P_PRIORITY_MAX)
        return RT_ERR_VLAN_PRIORITY;

    if (pSvlan_cfg->efiden >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if (pSvlan_cfg->efid > RTK_EFID_MAX)
        return RT_ERR_INPUT;

    empty_idx = 0xFF;
    
    for (i = 0; i < RTK_MAX_NUM_OF_SVLAN_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(i,&svlanMemConf))!=RT_ERR_OK)
            return retVal;
        if (svid == svlanMemConf.vs_svid)
        { 
            svlanMemConf.vs_svid = pSvlan_cfg->svid;
            svlanMemConf.vs_member = pSvlan_cfg->memberport;
            svlanMemConf.vs_fid = pSvlan_cfg->fid;
            svlanMemConf.vs_priority = pSvlan_cfg->priority;
            svlanMemConf.vs_efiden = pSvlan_cfg->efiden;
            svlanMemConf.vs_efid= pSvlan_cfg->efid;
            svlanMemConf.vs_relaysvid = pSvlan_cfg->svid;
            
            if ((retVal = rtl8370_setAsicSvlanMemberConfiguration(i, &svlanMemConf))!=RT_ERR_OK)
                return retVal;
            
            return RT_ERR_OK;
        }
        else if (svlanMemConf.vs_svid==0 && svlanMemConf.vs_member==0)
        {
            empty_idx = i;
            break;
        }
    }

    if (empty_idx != 0xFF)
    {
        memset(&svlanMemConf,0,sizeof(rtl8370_svlan_memconf_t));
        svlanMemConf.vs_svid = svid;
        svlanMemConf.vs_member = pSvlan_cfg->memberport;
        svlanMemConf.vs_fid = pSvlan_cfg->fid;
        svlanMemConf.vs_priority = pSvlan_cfg->priority;
        svlanMemConf.vs_efiden = pSvlan_cfg->efiden;
        svlanMemConf.vs_efid= pSvlan_cfg->efid;
        svlanMemConf.vs_relaysvid = pSvlan_cfg->svid;
        
        if ((retVal = rtl8370_setAsicSvlanMemberConfiguration(empty_idx, &svlanMemConf))!=RT_ERR_OK)
        {
            return retVal;  
        }
        
        return RT_ERR_OK;            
    }       


    /* 64 SVLAN member configuration is full, found a unused entry to replace */
    for (i = 0; i < RTK_MAX_NUM_OF_SVLAN_INDEX; i++)
    {    
        bUsed = FALSE;
        
        if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(i, &svlanMemConf))!=RT_ERR_OK)
            return retVal;
        
        for (j = 0; j <RTK_MAX_NUM_OF_C2S_INDEX; j++)
        {    
            if ((retVal = rtl8370_getAsicSvlanC2SConf(j,&evid,&pmsk,&svidx))!=RT_ERR_OK)
                return retVal;
            if (i == svidx)/*index i is in use by C2S*/
            {
                bUsed = TRUE;
                break;
            } 
            
            if ((retVal = rtl8370_getAsicSvlanSP2CConf(j,&svlanSP2CConf))!=RT_ERR_OK)
                return retVal;
            if (svlanMemConf.vs_svid==svlanSP2CConf.svid) /*index i is in use by SP2C*/
            {
                bUsed = TRUE;
                break;
            }
        }

        for (k = 0; k <RTK_MAX_NUM_OF_MC2S_INDEX; k++)
        {    
            if ((retVal = rtl8370_getAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
                return retVal;
            if (i == svlanMC2SConf.svidx && svlanMC2SConf.valid == 1)/*index i is in use by MC2S*/
            {
                bUsed = TRUE;
                break;
            } 
        }

        if (FALSE == bUsed)/*found a unused index, replace it*/
        {
            memset(&svlanMemConf,0,sizeof(rtl8370_svlan_memconf_t));
            svlanMemConf.vs_svid = svid;
            svlanMemConf.vs_member = pSvlan_cfg->memberport;
            svlanMemConf.vs_fid = pSvlan_cfg->fid;
            svlanMemConf.vs_priority = pSvlan_cfg->priority;
            svlanMemConf.vs_efiden = pSvlan_cfg->efiden;
            svlanMemConf.vs_efid= pSvlan_cfg->efid;
            svlanMemConf.vs_relaysvid = pSvlan_cfg->svid;
        
            if ((retVal = rtl8370_setAsicSvlanMemberConfiguration(i, &svlanMemConf))!=RT_ERR_OK)
                return retVal;  
        
            return RT_ERR_OK;             
        }
    }    

    return RT_ERR_SVLAN_TABLE_FULL;
}

/* Function Name:
 *      rtk_svlan_memberPortEntry_get
 * Description:
 *      Get SVLAN member Configure.
 * Input:
 *      svid - SVLAN id
 * Output:
 *      pSvlan_cfg - SVLAN member configuration
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found.
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped.
 */
rtk_api_ret_t rtk_svlan_memberPortEntry_get(rtk_vlan_t svid, rtk_svlan_memberCfg_t *pSvlan_cfg)
{
    rtk_api_ret_t retVal;
    uint32 i;
    rtl8370_svlan_memconf_t svlanMemConf;

    if (svid > RTK_VLAN_ID_MAX)
        return RT_ERR_SVLAN_VID;


    for (i=0;i<RTK_MAX_NUM_OF_SVLAN_INDEX;i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(i, &svlanMemConf))!=RT_ERR_OK)
            return retVal;
        if (svid == svlanMemConf.vs_svid)
        { 
            pSvlan_cfg->svid = svlanMemConf.vs_svid; 
            pSvlan_cfg->memberport = svlanMemConf.vs_member;
            pSvlan_cfg->fid = svlanMemConf.vs_fid;
            pSvlan_cfg->priority = svlanMemConf.vs_priority;
            pSvlan_cfg->efiden = svlanMemConf.vs_efiden;
            pSvlan_cfg->efid = svlanMemConf.vs_efid;
            return RT_ERR_OK;
        }
    }

    return RT_ERR_SVLAN_ENTRY_NOT_FOUND;

}

/* Function Name:
 *      rtk_svlan_defaultSvlan_set
 * Description:
 *      Configure default egress SVLAN.
 * Input:
 *      svid - SVLAN id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_SVLAN_VID - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found.
 * Note:
 *      The API can set port n S-tag format index while receiving frame from port n 
 *      is transmit through uplink port with s-tag field
 */
rtk_api_ret_t rtk_svlan_defaultSvlan_set(rtk_vlan_t svid)
{
    rtk_api_ret_t retVal;
    uint32 i;
    rtl8370_svlan_memconf_t svlanMemConf;
    
    /* svid must be 0~4095 */
    if (svid > RTK_VLAN_ID_MAX)
        return RT_ERR_SVLAN_VID;

    for (i = 0; i < RTK_MAX_NUM_OF_SVLAN_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(i, &svlanMemConf))!=RT_ERR_OK)
            return retVal;
        
        if (svid == svlanMemConf.vs_svid)
        {          
            if ((retVal = rtl8370_setAsicSvlanDefaultVlan(i))!=RT_ERR_OK)
                return retVal;
            
            return RT_ERR_OK;
        }    
    }

    return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
}

/* Function Name:
 *      rtk_svlan_defaultSvlan_get
 * Description:
 *      Get the configure default egress SVLAN.
 * Input:
 *      None
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get port n S-tag format index while receiving frame from port n 
 *      is transmit through uplink port with s-tag field
 */
rtk_api_ret_t rtk_svlan_defaultSvlan_get(rtk_vlan_t *pSvid)
{
    rtk_api_ret_t retVal;
    uint32 idx;
    rtl8370_svlan_memconf_t svlanMemConf;


    if ((retVal = rtl8370_getAsicSvlanDefaultVlan(&idx))!=RT_ERR_OK)
        return retVal;
    
    if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(idx, &svlanMemConf))!=RT_ERR_OK)
        return retVal;
        
    *pSvid = svlanMemConf.vs_svid;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_svlan_c2s_add
 * Description:
 *      Configure SVLAN function CVLAN-to-SVLAN table 
 * Input:
 *      vid - VLAN ID
 *      port - available c2s service port
 *      svid - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port id.
 *      RT_ERR_SVLAN_VID - Invalid SVLAN VID parameter.
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      The API can set system C2S configuration. ASIC will check upstream's VID and assign related
 *      SVID to mathed packet. There are 128 SVLAN C2S configurations.
 */
rtk_api_ret_t rtk_svlan_c2s_add(rtk_vlan_t vid, rtk_port_t port, rtk_vlan_t svid)
{
    rtk_api_ret_t retVal,i;
    uint32 empty_idx;
    uint32 evid, pmsk, svidx, c2s_svidx;
    rtl8370_svlan_memconf_t svlanMemConf;

    if (vid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    if (svid > RTK_VLAN_ID_MAX)
        return RT_ERR_SVLAN_VID;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    empty_idx = 0xFFFF;
    svidx = 0xFFFF;
    
    for (i = 0; i<RTK_MAX_NUM_OF_SVLAN_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(i,&svlanMemConf))!=RT_ERR_OK)
            return retVal;
        
        if (svid == svlanMemConf.vs_svid)
        {          
            svidx = i;
            break;
        }
    }

    if (0xFFFF == svidx)
        return RT_ERR_SVLAN_VID;
    
    for (i=0; i<RTK_MAX_NUM_OF_C2S_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanC2SConf(i,&evid,&pmsk,&c2s_svidx))!=RT_ERR_OK)
                return retVal;

        if (evid == vid)
        {          
            if (svidx == c2s_svidx)
            {
                pmsk = pmsk | (1<<port);
                if ((retVal = rtl8370_setAsicSvlanC2SConf(i,vid,pmsk,svidx))!=RT_ERR_OK)
                    return retVal;
                return RT_ERR_OK;
            }
        }
        else if (evid==0&&pmsk==0&&c2s_svidx==0)
        {
            empty_idx = i;
            break;
        }
    }

    if (0xFFFF != empty_idx)
    {
       if ((retVal = rtl8370_setAsicSvlanC2SConf(empty_idx,vid,(1<<port),svidx))!=RT_ERR_OK)
           return retVal;
       return RT_ERR_OK;
    }
    
    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_c2s_del
 * Description:
 *      Delete one service port in CVLAN-to-SVLAN table
 * Input:
 *      vid - VLAN ID
 *      port - available c2s service port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 *      RT_ERR_OUT_OF_RANGE - Input out of range.
 * Note:
 *      The API can delete system C2S configuration. There are 128 SVLAN C2S configurations.
 */
rtk_api_ret_t rtk_svlan_c2s_del(rtk_vlan_t vid, rtk_port_t port)
{
    rtk_api_ret_t retVal;
    uint32 i;
    uint32 evid, pmsk, svidx;

    if (vid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    for (i = 0; i < RTK_MAX_NUM_OF_C2S_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanC2SConf(i,&evid,&pmsk,&svidx))!=RT_ERR_OK)
            return retVal;
        
        if (evid == vid)
        {          
            pmsk = pmsk & ~(1<<port);
            if (pmsk == 0)
            {
                vid = 0;
                svidx = 0;
            }
            if ((retVal = rtl8370_setAsicSvlanC2SConf(i,vid,pmsk,svidx))!=RT_ERR_OK)
                return retVal;

            return RT_ERR_OK;
        }
    }
    
    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_c2s_get
 * Description:
 *      Get configure SVLAN C2S table 
 * Input:
 *      vid - VLAN ID
 *      port - available c2s service port
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *     The API can get system C2S configuration. There are 128 SVLAN C2S configurations.
 */
rtk_api_ret_t rtk_svlan_c2s_get(rtk_vlan_t vid, rtk_port_t port, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t retVal;
    uint32 i;
    uint32 evid, pmsk, svidx;
    rtl8370_svlan_memconf_t svlanMemConf;

    if (vid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    for (i = 0; i < RTK_MAX_NUM_OF_C2S_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanC2SConf(i,&evid,&pmsk,&svidx))!=RT_ERR_OK)
            return retVal;
        
        if ((evid == vid)&&(pmsk&(1<<port)))
        {          
            if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(svidx,&svlanMemConf))!=RT_ERR_OK)
                return retVal;

            *pSvid = svlanMemConf.vs_svid;
                return RT_ERR_OK;
        }
    }
    
    return RT_ERR_OUT_OF_RANGE;;
}

/* Function Name:
 *      rtk_svlan_ipmc2s_add
 * Description:
 *      add ip multicast address to SVLAN
 * Input:
 *      svid - SVLAN VID
 *      ipmc - ip multicast address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_SVLAN_VID - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      The API can set IP mutlicast to SVID configuration. If upstream packet is IPv4 multicast
 *      packet and DIP is matched MC2S configuration, ASIC will assign egress SVID to the packet.
 *      There are 32 SVLAN multicast configurations for IP and L2 multicast. 
 */
rtk_api_ret_t rtk_svlan_ipmc2s_add(ipaddr_t ipmc, rtk_vlan_t svid)
{
    rtk_api_ret_t retVal,i;
    uint32 empty_idx;
    uint32 svidx;
    rtl8370_svlan_memconf_t svlanMemConf;
    rtl8370_svlan_mc2s_t svlanMC2SConf;

    if (svid > RTK_VLAN_ID_MAX)
        return RT_ERR_SVLAN_VID;

    svidx = 0xFFFF;   

    for (i = 0; i < RTK_MAX_NUM_OF_SVLAN_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(i, &svlanMemConf))!=RT_ERR_OK)
            return retVal;
        
        if (svid == svlanMemConf.vs_svid)
        {          
            svidx = i;
            break;
        }
    }
        
    if (0xFFFF == svidx)
            return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
    
    
    empty_idx = 0xFFFF;
    
    for (i = 0; i < RTK_MAX_NUM_OF_MC2S_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
            return retVal;

        if (TRUE == svlanMC2SConf.valid)
        {
            if (svlanMC2SConf.format == 1 && svlanMC2SConf.value==ipmc&&svlanMC2SConf.mask!=0)
            {
                svlanMC2SConf.svidx = svidx;
                if ((retVal = rtl8370_setAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
                    return retVal;
            }
        }
        else 
        {
            empty_idx = i;
            break;
        }
    }

    if (empty_idx!=0xFFFF)
    {
        svlanMC2SConf.valid = TRUE;
        svlanMC2SConf.svidx = svidx;
        svlanMC2SConf.format = 1;
        svlanMC2SConf.value = ipmc;
        svlanMC2SConf.mask = 0xFFFFFFFF;    
        if ((retVal = rtl8370_setAsicSvlanMC2SConf(empty_idx,&svlanMC2SConf))!=RT_ERR_OK)
            return retVal;
        return RT_ERR_OK;
    }
    
    return RT_ERR_OUT_OF_RANGE;

}

/* Function Name:
 *      rtk_svlan_ipmc2s_del
 * Description:
 *      delete ip multicast address to SVLAN
 * Input:
 *      ipmc - ip multicast address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_SVLAN_VID - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can delete IP mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtk_api_ret_t rtk_svlan_ipmc2s_del(ipaddr_t ipmc)
{
    rtk_api_ret_t retVal;
    uint32 i;
    rtl8370_svlan_mc2s_t svlanMC2SConf;
    
    for (i = 0; i < RTK_MAX_NUM_OF_MC2S_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
            return retVal;

        if (TRUE == svlanMC2SConf.valid)
        {        
            if (svlanMC2SConf.format == 1 && svlanMC2SConf.value==ipmc&&svlanMC2SConf.mask!=0)
            {
                memset(&svlanMC2SConf,0,sizeof(rtl8370_svlan_mc2s_t));
                if ((retVal = rtl8370_setAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
                    return retVal;
                return RT_ERR_OK;            
            }
        }
    }

    return RT_ERR_OUT_OF_RANGE;
}


/* Function Name:
 *      rtk_svlan_ipmc2s_get
 * Description:
 *      Get ip multicast address to SVLAN
 * Input:
 *      ipmc - ip multicast address
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can get IP mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtk_api_ret_t rtk_svlan_ipmc2s_get(ipaddr_t ipmc, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t retVal;
    uint32 i;
    rtl8370_svlan_memconf_t svlanMemConf;
    rtl8370_svlan_mc2s_t svlanMC2SConf;
    
    for (i = 0; i < RTK_MAX_NUM_OF_MC2S_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
            return retVal;
        
        if (TRUE == svlanMC2SConf.valid && svlanMC2SConf.format == 1&& svlanMC2SConf.value == ipmc && svlanMC2SConf.mask != 0)
        {
            if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(svlanMC2SConf.svidx, &svlanMemConf))!=RT_ERR_OK)
                return retVal;
            *pSvid = svlanMemConf.vs_svid;
            return RT_ERR_OK;            
        }
    }
    
    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_l2mc2s_add
 * Description:
 *      Add L2 multicast address to SVLAN
 * Input:
 *      svid - SVLAN VID
 *      mac - L2 multicast address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_SVLAN_VID - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      The API can set L2 Mutlicast to SVID configuration. If upstream packet is L2 multicast
 *      packet and DMAC is matched, ASIC will assign egress SVID to the packet. There are 32 
 *      SVLAN multicast configurations for IP and L2 multicast.
 */
rtk_api_ret_t rtk_svlan_l2mc2s_add(rtk_vlan_t svid, rtk_mac_t mac)
{
    rtk_api_ret_t retVal,i;
    uint32 empty_idx;
    uint32 svidx, l2add;
    rtl8370_svlan_memconf_t svlanMemConf;
    rtl8370_svlan_mc2s_t svlanMC2SConf;

    if (svid > RTK_VLAN_ID_MAX)
        return RT_ERR_SVLAN_VID;
    
    if (mac.octet[0]!= 1&&mac.octet[1]!=0)
        return RT_ERR_INPUT;
    
    memcpy(&l2add,&mac.octet[2],4);

     svidx = 0xFFFF;   

    for (i = 0; i < RTK_MAX_NUM_OF_SVLAN_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(i, &svlanMemConf))!=RT_ERR_OK)
            return retVal;
        
        if (svid == svlanMemConf.vs_svid)
        {          
            svidx = i;
            break;
        }
    }
    
    if (0xFFFF == svidx)
        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
    
    empty_idx = 0xFFFF;
    
    for (i = 0; i < RTK_MAX_NUM_OF_MC2S_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
            return retVal;
        
        if (TRUE == svlanMC2SConf.valid)
        {
            if (svlanMC2SConf.format == 0 && svlanMC2SConf.value==l2add&&svlanMC2SConf.mask!=0)
            {
                svlanMC2SConf.svidx = svidx;
                if ((retVal = rtl8370_setAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
                    return retVal;
            }
        }
        else
        {
            empty_idx = i;
            break;
        }
    }

    if (empty_idx!=0xFFFF)
    {
        svlanMC2SConf.valid = TRUE;
        svlanMC2SConf.svidx = svidx;
        svlanMC2SConf.format = 0;
        svlanMC2SConf.value = l2add;
        svlanMC2SConf.mask = 0xFFFFFFFF;
    
        if ((retVal = rtl8370_setAsicSvlanMC2SConf(empty_idx,&svlanMC2SConf))!=RT_ERR_OK)
            return retVal;
        return RT_ERR_OK;
    }
    
    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_l2mc2s_del
 * Description:
 *      delete L2 multicast address to SVLAN
 * Input:
 *      mac - L2 multicast address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_SVLAN_VID - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can delete Mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtk_api_ret_t rtk_svlan_l2mc2s_del(rtk_mac_t mac)
{
    rtk_api_ret_t retVal;
    uint32 i;
    uint32 l2add;
      rtl8370_svlan_mc2s_t svlanMC2SConf;

      if (mac.octet[0]!= 1&&mac.octet[1]!=0)
        return RT_ERR_INPUT;
    

    memcpy(&l2add,&mac.octet[2],4);
    
    for (i = 0; i < RTK_MAX_NUM_OF_MC2S_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
            return retVal;

        if (TRUE == svlanMC2SConf.valid)
        {        
            if (svlanMC2SConf.format == 0 && svlanMC2SConf.value==l2add&&svlanMC2SConf.mask!=0)
            {
                memset(&svlanMC2SConf,0,sizeof(rtl8370_svlan_mc2s_t));
                if ((retVal = rtl8370_setAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
                    return retVal;
                return RT_ERR_OK;                
            }
        }
    }
    
    return RT_ERR_OUT_OF_RANGE;
}


/* Function Name:
 *      rtk_svlan_l2mc2s_get
 * Description:
 *      Get L2 multicast address to SVLAN
 * Input:
 *      mac - L2 multicast address
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can get L2 mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
rtk_api_ret_t rtk_svlan_l2mc2s_get(rtk_mac_t mac, rtk_vlan_t *pSvid)
{
    rtk_api_ret_t retVal;
    uint32 i;
    uint32 l2add;
    rtl8370_svlan_memconf_t svlanMemConf;
    rtl8370_svlan_mc2s_t svlanMC2SConf;

    if (mac.octet[0]!= 1&&mac.octet[1]!=0)
        return RT_ERR_INPUT;
    
    memcpy(&l2add,&mac.octet[2],4);
    
    for (i = 0; i < RTK_MAX_NUM_OF_MC2S_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMC2SConf(i,&svlanMC2SConf))!=RT_ERR_OK)
            return retVal;

        if (TRUE == svlanMC2SConf.valid)
        {        
            if (svlanMC2SConf.format == 0 && svlanMC2SConf.value==l2add&&svlanMC2SConf.mask!=0)
            {
                if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(svlanMC2SConf.svidx, &svlanMemConf))!=RT_ERR_OK)
                    return retVal;
                *pSvid = svlanMemConf.vs_svid;
                
                return RT_ERR_OK;
            }
        }
    }
    
    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_svlan_sp2c_add
 * Description:
 *      Add system SP2C configuration
 * Input:
 *      cvid - VLAN ID
 *      svid - SVLAN VID
 *      dst_port - Destination port of SVLAN to CVLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_SVLAN_VID - Invalid SVLAN VID parameter.
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      The API can add SVID & Destination Port to CVLAN configuration. The downstream frames with assigned
 *      SVID will be add C-tag with assigned CVID if the output port is the assigned destination port.
 *      There are 128 SP2C configurations.
 */
rtk_api_ret_t rtk_svlan_sp2c_add(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t cvid)
{
    rtk_api_ret_t retVal,i;
    uint32 empty_idx,svidx;
    rtl8370_svlan_memconf_t svlanMemConf;
    rtl8370_svlan_s2c_t svlanSP2CConf;

    if (svid > RTK_VLAN_ID_MAX)
        return RT_ERR_SVLAN_VID;

    if (cvid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

      if (dst_port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 

    svidx = 0xFFFF;

    for (i = 0; i < RTK_MAX_NUM_OF_SVLAN_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(i, &svlanMemConf))!=RT_ERR_OK)
            return retVal;
        
        if (svid == svlanMemConf.vs_svid)
        {          
            svidx = i;
            break;
        }
    }
    
    if (0xFFFF == svidx)
        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
    
    empty_idx = 0xFFFF;
    
    for (i = 0; i < RTK_MAX_NUM_OF_SP2C_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanSP2CConf(i,&svlanSP2CConf))!=RT_ERR_OK)
            return retVal;
        
        if (svlanSP2CConf.svid==svid && svlanSP2CConf.dstport==dst_port)
        {
            empty_idx = i;
            break;
        }
        else if (svlanSP2CConf.evid == 0 && svlanSP2CConf.svid==0 && svlanSP2CConf.dstport==0)
        {
            empty_idx = i;
            break;
        }
    }

    if (empty_idx!=0xFFFF)
    {
        svlanSP2CConf.evid = cvid;
        svlanSP2CConf.svid = svid;
        svlanSP2CConf.dstport = dst_port;
    
        if ((retVal = rtl8370_setAsicSvlanSP2CConf(empty_idx,&svlanSP2CConf))!=RT_ERR_OK)
            return retVal;
        return RT_ERR_OK;
    }
    
    return RT_ERR_OUT_OF_RANGE;

}

/* Function Name:
 *      rtk_svlan_sp2c_get
 * Description:
 *      Get configure system SP2C content
 * Input:
 *      dst_port - Destination port of SVLAN to CVLAN configuration
 *      svid - SVLAN VID
 * Output:
 *      pCvid - VLAN ID
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_SVLAN_VID - Invalid SVLAN VID parameter.
 * Note:
 *     The API can get SVID & Destination Port to CVLAN configuration. There are 128 SP2C configurations.
 */
rtk_api_ret_t rtk_svlan_sp2c_get(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t *pCvid)
{
    rtk_api_ret_t retVal;
    uint32 i;
    rtl8370_svlan_s2c_t svlanSP2CConf;

    if (svid > RTK_VLAN_ID_MAX)
        return RT_ERR_SVLAN_VID;

    if (dst_port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 

    
    for (i = 0; i < RTK_MAX_NUM_OF_SP2C_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanSP2CConf(i,&svlanSP2CConf))!=RT_ERR_OK)
            return retVal;
        
        if (svlanSP2CConf.svid==svid && svlanSP2CConf.dstport==dst_port)
        {
            *pCvid = svlanSP2CConf.evid;
            return RT_ERR_OK;
        }
    }

    return RT_ERR_OUT_OF_RANGE;
}


/* Function Name:
 *      rtk_svlan_sp2c_del
 * Description:
 *      Delete system SP2C configuration
 * Input:
 *      svid - SVLAN VID
 *      dst_port - Destination port of SVLAN to CVLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_SVLAN_VID - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can delete SVID & Destination Port to CVLAN configuration. There are 128 SP2C configurations.
 */

rtk_api_ret_t rtk_svlan_sp2c_del(rtk_vlan_t svid, rtk_port_t dst_port)
{
    rtk_api_ret_t retVal;
    uint32 i;
    rtl8370_svlan_s2c_t svlanSP2CConf;

    if (svid > RTK_VLAN_ID_MAX)
        return RT_ERR_SVLAN_VID;

    if (dst_port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 

    
    for (i = 0; i < RTK_MAX_NUM_OF_SP2C_INDEX; i++)
    {       
        if ((retVal = rtl8370_getAsicSvlanSP2CConf(i,&svlanSP2CConf))!=RT_ERR_OK)
            return retVal;
        
        if (svlanSP2CConf.svid==svid && svlanSP2CConf.dstport==dst_port)
        {
            svlanSP2CConf.evid = 0;
            svlanSP2CConf.svid = 0;
            svlanSP2CConf.dstport = 0;
    
            if ((retVal = rtl8370_setAsicSvlanSP2CConf(i,&svlanSP2CConf))!=RT_ERR_OK)
                return retVal;
            return RT_ERR_OK;
        }

    }

    return RT_ERR_OUT_OF_RANGE;
}

/* Function Name:
 *      rtk_cpu_enable_set
 * Description:
 *      Set CPU port function enable/disable.
 * Input:
 *      enable - CPU port function enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API can set CPU port function enable/disable. 
 */
rtk_api_ret_t rtk_cpu_enable_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8370_setAsicCputagEnable(enable))!=RT_ERR_OK)
        return retVal;

    if (DISABLED == enable)
    {
        if ((retVal = rtl8370_setAsicCputagPortmask(0))!=RT_ERR_OK)
            return retVal;
    }    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_cpu_enable_get
 * Description:
 *      Get CPU port enable.
 * Input:
 *      None
 * Output:
 *      pEnable - CPU port function enable
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      The API can get CPU port function enable/disable.
 */
rtk_api_ret_t rtk_cpu_enable_get(rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicCputagEnable(pEnable))!=RT_ERR_OK)
        return retVal;
   

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_cpu_tagPort_set
 * Description:
 *      Set CPU port and CPU tag insert mode.
 * Input:
 *      port - Port id.
 *      mode - CPU tag insert for packets egress from CPU port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can set CPU port and inserting proprietary CPU tag mode (Length/Type 0x8899)
 *      to the frame that transmitting to CPU port.
 *      The inset cpu tag mode is as following:
 *      CPU_INSERT_TO_ALL
 *      CPU_INSERT_TO_TRAPPING
 *      CPU_INSERT_TO_NONE   
 */
rtk_api_ret_t rtk_cpu_tagPort_set(rtk_port_t port, rtk_cpu_insert_t mode)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_INPUT;

    if (mode >= CPU_INSERT_END)
        return RT_ERR_INPUT;
    
    if ((retVal = rtl8370_setAsicCputagPortmask(1<<port))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicCputagTrapPort(port))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicCputagInsertMode(mode))!=RT_ERR_OK)
        return retVal;
        
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_cpu_tagPort_get
 * Description:
 *      Get CPU port and CPU tag insert mode.
 * Input:
 *      None
 * Output:
 *      pPort - Port id.
 *      pMode - CPU tag insert for packets egress from CPU port, 0:all insert 1:Only for trapped packets 2:no insert.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT - CPU port is not exist
 * Note:
 *      The API can get configured CPU port and its setting.
 *      The inset cpu tag mode is as following:
 *      CPU_INSERT_TO_ALL
 *      CPU_INSERT_TO_TRAPPING
 *      CPU_INSERT_TO_NONE  
 */
rtk_api_ret_t rtk_cpu_tagPort_get(rtk_port_t *pPort, rtk_data_t *pMode)
{
    rtk_api_ret_t retVal;
    uint32 i, pmsk, port;

    if ((retVal = rtl8370_getAsicCputagPortmask(&pmsk))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicCputagTrapPort(&port))!=RT_ERR_OK)
        return retVal;

    for (i=0;i< RTK_MAX_NUM_OF_PORT;i++)
    {
        if ((pmsk&(1<<i))!=0)
        {
            if (i==port)
                *pPort = port;
            else
                return RT_ERR_FAILED;
        }
    }    

    if ((retVal = rtl8370_getAsicCputagInsertMode(pMode))!=RT_ERR_OK)
        return retVal; 
            
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_set
 * Description:
 *      Set 802.1x unauth action configuration.
 * Input:
 *      port - Port id.
 *      unauth_action - 802.1X unauth action. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *      This API can set 802.1x unauth action configuration.
 *      The unauth action is as following:
 *      DOT1X_ACTION_DROP
 *      DOT1X_ACTION_TRAP2CPU
 *      DOT1X_ACTION_GUESTVLAN 
 */
rtk_api_ret_t rtk_dot1x_unauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (unauth_action >= DOT1X_ACTION_END)
        return RT_ERR_DOT1X_PROC;
    
    if ((retVal = rtl8370_setAsic1xProcConfig(port,unauth_action))!=RT_ERR_OK)
        return retVal; 

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_get
 * Description:
 *      Get 802.1x unauth action configuration.
 * Input:
 *      port - Port id.
 * Output:
 *      pUnauth_action - 802.1X unauth action. 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can get 802.1x unauth action configuration.
 *      The unauth action is as following:
 *      DOT1X_ACTION_DROP
 *      DOT1X_ACTION_TRAP2CPU
 *      DOT1X_ACTION_GUESTVLAN 
 */
rtk_api_ret_t rtk_dot1x_unauthPacketOper_get(rtk_port_t port, rtk_data_t *pUnauth_action)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if ((retVal = rtl8370_getAsic1xProcConfig(port,pUnauth_action))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_set
 * Description:
 *      Set 802.1x EAPOL packet trap to CPU configuration
 * Input:
 *      enable - The status of 802.1x EAPOL packet.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      To support 802.1x authentication functionality, EAPOL frame (ether type = 0x888E) has to
 *      be trapped to CPU.
 *      The status of EAPOL frame trap to CPU is as following:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtl8370_rma_t rmacfg;

    if (enable >=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8370_getAsicRma(RTK_DOT1X_PAE, &rmacfg))!=RT_ERR_OK)
        return retVal; 

    if (ENABLED == enable)
        rmacfg.operation = RMAOP_TRAP_TO_CPU;
    else if (DISABLED == enable)
        rmacfg.operation = RMAOP_FORWARD;
    else
        return RT_ERR_INPUT;
    
    if ((retVal = rtl8370_setAsicRma(3, &rmacfg))!=RT_ERR_OK)
        return retVal;    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_get
 * Description:
 *      Get 802.1x EAPOL packet trap to CPU configuration
 * Input:
 *      None
 * Output:
 *      pEnable - The status of 802.1x EAPOL packet.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      To support 802.1x authentication functionality, EAPOL frame (ether type = 0x888E) has to
 *      be trapped to CPU.
 *      The status of EAPOL frame trap to CPU is as following:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_get(rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtl8370_rma_t rmacfg;

    if ((retVal = rtl8370_getAsicRma(3, &rmacfg))!=RT_ERR_OK)
        return retVal; 

    if (RMAOP_TRAP_TO_CPU == rmacfg.operation)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedEnable_set
 * Description:
 *      Set 802.1x port-based enable configuration
 * Input:
 *      port - Port id.
 *      enable - The status of 802.1x port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 *      RT_ERR_DOT1X_PORTBASEDPNEN - 802.1X port-based enable error
 * Note:
 *      The API can update the port-based port enable register content. If a port is 802.1x 
 *      port based network access control "enabled", it should be authenticated so packets 
 *      from that port won't be dropped or trapped to CPU. 
 *      The status of 802.1x port-based network access control is as following:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_dot1x_portBasedEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;
    
    if (enable >=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8370_setAsic1xPBEnConfig(port,enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedEnable_get
 * Description:
 *      Get 802.1x port-based enable configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - The status of 802.1x port.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get the 802.1x port-based port status.
 */
rtk_api_ret_t rtk_dot1x_portBasedEnable_get(rtk_port_t port, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsic1xPBEnConfig(port, pEnable))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_set
 * Description:
 *      Set 802.1x port-based auth. port configuration
 * Input:
 *      port - Port id.
 *      port_auth - The status of 802.1x port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *     RT_ERR_DOT1X_PORTBASEDAUTH - 802.1X port-based auth error
 * Note:
 *      The authenticated status of 802.1x port-based network access control is as following:
 *      UNAUTH
 *      AUTH
 */
rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_set(rtk_port_t port, rtk_dot1x_auth_status_t port_auth)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

     if (port_auth >= AUTH_STATUS_END)
        return RT_ERR_DOT1X_PORTBASEDAUTH;

    if ((retVal = rtl8370_setAsic1xPBAuthConfig(port,port_auth))!=RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_get
 * Description:
 *      Get 802.1x port-based auth. port configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pPort_auth - The status of 802.1x port.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get 802.1x port-based port auth.information.
 */
rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_get(rtk_port_t port, rtk_data_t *pPort_auth)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsic1xPBAuthConfig(port, pPort_auth))!=RT_ERR_OK)
        return retVal;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedDirection_set
 * Description:
 *      Set 802.1x port-based operational direction configuration
 * Input:
 *      port - Port id.
 *      port_direction - Operation direction
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_DOT1X_PORTBASEDOPDIR - 802.1X port-based operation direction error
 * Note:
 *      The operate controlled direction of 802.1x port-based network access control is as following:
 *      BOTH
 *      IN
 */
rtk_api_ret_t rtk_dot1x_portBasedDirection_set(rtk_port_t port, rtk_dot1x_direction_t port_direction)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (port_direction >= DIRECTION_END)
        return RT_ERR_DOT1X_PORTBASEDOPDIR;

    if ((retVal = rtl8370_setAsic1xPBOpdirConfig(port,port_direction))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedDirection_get
 * Description:
 *      Get 802.1X port-based operational direction configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pPort_direction - Operation direction
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get 802.1x port-based operational direction information.
 */
rtk_api_ret_t rtk_dot1x_portBasedDirection_get(rtk_port_t port, rtk_data_t *pPort_direction)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsic1xPBOpdirConfig(port, pPort_direction))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedEnable_set
 * Description:
 *      Set 802.1x mac-based port enable configuration
 * Input:
 *      port - Port id.
 *      enable - The status of 802.1x port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 *      RT_ERR_DOT1X_MACBASEDPNEN - 802.1X mac-based enable error
 * Note:
 *      If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *       be authenticated so packets from that port won't be dropped or trapped to CPU.
 *      The status of 802.1x MAC-based network access control is as following:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_dot1x_macBasedEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8370_setAsic1xMBEnConfig(port,enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedEnable_get
 * Description:
 *      Get 802.1x mac-based port enable configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - The status of 802.1x port.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *      be authenticated so packets from that port wont be dropped or trapped to CPU.
 *      The status of 802.1x MAC-based network access control is as following:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_dot1x_macBasedEnable_get(rtk_port_t port, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsic1xMBEnConfig(port, pEnable))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_add
 * Description:
 *      Add an authenticated MAC to ASIC
 * Input:
 *      port - Port id.
 *      pAuth_mac - The authenticated MAC.
 *      fid - filtering database.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 *      RT_ERR_DOT1X_MACBASEDPNEN - 802.1X mac-based enable error
 * Note:
 *      The API can add a 802.1x authenticated MAC address to port. If the MAC does not exist in LUT, 
 *      user can't add this MAC to auth status.
 */
rtk_api_ret_t rtk_dot1x_macBasedAuthMac_add(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;
      uint32 method;    
    rtl8370_luttb l2Table;

    /* must be unicast address */
    if ((pAuth_mac == NULL) || (pAuth_mac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;    

    if (fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;    

    memset(&l2Table,0,sizeof(rtl8370_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pAuth_mac->octet, ETHER_ADDR_LEN);
    l2Table.fid = fid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8370_getAsicL2LookupTb(method,&l2Table);
    if ( RT_ERR_OK == retVal)
    {
        if (l2Table.spa != port)
            return RT_ERR_DOT1X_MAC_PORT_MISMATCH;
      
        memcpy(l2Table.mac.octet, pAuth_mac->octet, ETHER_ADDR_LEN);
        l2Table.fid = fid;
        l2Table.efid = 0;
        l2Table.auth = 1;
        retVal = rtl8370_setAsicL2LookupTb(&l2Table);
        return retVal;        
    }    
    else 
        return retVal;            

}

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_del
 * Description:
 *      Delete an authenticated MAC to ASIC
 * Input:
 *      port - Port id.
 *      pAuth_mac - The authenticated MAC.
 *      fid - filtering database.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can delete a 802.1x authenticated MAC address to port. It only change the auth status of
 *      the MAC and won't delete it from LUT.
 */
rtk_api_ret_t rtk_dot1x_macBasedAuthMac_del(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;
    uint32 method;    
    rtl8370_luttb l2Table;

    /* must be unicast address */
    if ((pAuth_mac == NULL) || (pAuth_mac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;    

    if (fid > RTK_FID_MAX)
        return RT_ERR_L2_FID;    

    memset(&l2Table,0,sizeof(rtl8370_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pAuth_mac->octet, ETHER_ADDR_LEN);
    l2Table.fid = fid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8370_getAsicL2LookupTb(method,&l2Table);
    if (RT_ERR_OK == retVal)
    {
        if (l2Table.spa != port)
            return RT_ERR_DOT1X_MAC_PORT_MISMATCH;
      
        memcpy(l2Table.mac.octet, pAuth_mac->octet, ETHER_ADDR_LEN);
        l2Table.fid = fid;
        l2Table.auth = 0;
        retVal = rtl8370_setAsicL2LookupTb(&l2Table);
        return retVal;        
    }    
    else 
        return retVal;            

}

/* Function Name:
 *      rtk_dot1x_macBasedDirection_set
 * Description:
 *      Set 802.1x mac-based operational direction configuration
 * Input:
 *      mac_direction - Operation direction
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_DOT1X_MACBASEDOPDIR - 802.1X mac-based operation direction error
 * Note:
 *      The operate controlled direction of 802.1x mac-based network access control is as following:
 *      BOTH
 *      IN
 */
rtk_api_ret_t rtk_dot1x_macBasedDirection_set(rtk_dot1x_direction_t mac_direction)
{
    rtk_api_ret_t retVal;

    if (mac_direction >= DIRECTION_END)
        return RT_ERR_DOT1X_MACBASEDOPDIR;

    if ((retVal = rtl8370_setAsic1xMBOpdirConfig(mac_direction))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedDirection_get
 * Description:
 *      Get 802.1x mac-based operational direction configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_direction - Operation direction 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get 802.1x mac-based operational direction information.
 */
rtk_api_ret_t rtk_dot1x_macBasedDirection_get(rtk_data_t *pMac_direction)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsic1xMBOpdirConfig(pMac_direction))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      Set 802.1x guest VLAN configuration
 * Description:
 *      Set 802.1x mac-based operational direction configuration
 * Input:
 *      vid - 802.1x guest VLAN ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *      The operate controlled 802.1x guest VLAN
 */
rtk_api_ret_t rtk_dot1x_guestVlan_set(rtk_vlan_t vid)
{
    rtk_api_ret_t retVal;
    int32 i;
    uint32 j;
    uint32 k;
    uint32 index,empty_idx;    
    uint32  bUsed,pri;    
    rtl8370_user_vlan4kentry vlan4K;
    rtl8370_vlanconfiguser vlanMC;    
    rtl8370_protocolvlancfg ppb_vlan_cfg;
    
    /* vid must be 0~4095 */
    if (vid > RTK_VLAN_ID_MAX)
        return RT_ERR_VLAN_VID;

    empty_idx = 0xFFFF;
    
    for (i = (RTK_MAX_NUM_OF_VLAN_INDEX-1); i >=0 ; i--)
    {       
        if ((retVal = rtl8370_getAsicVlanMemberConfig(i, &vlanMC))!=RT_ERR_OK)
            return retVal;
        if (vid == vlanMC.evid)
        {          
            if ((retVal = rtl8370_setAsic1xGuestVidx(i))!=RT_ERR_OK)
                return retVal;
            return RT_ERR_OK;
        }    
        else if (vlanMC.evid == 0 && vlanMC.mbr == 0)
        {
             empty_idx = i;
        }
        
    }

    /*
        vid doesn't exist in 32 member configuration. Find an empty entry in 
        32 member configuration, then copy entry from 4K. If 32 member configuration
        are all full, then find an entry which not used by Port-based VLAN and 
        then replace it with 4K. Finally, assign the index to the port.
    */
    if (empty_idx!=0xFFFF)
        {
            vlan4K.vid = vid;
            if ((retVal = rtl8370_getAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
                return retVal;

            vlanMC.evid = vid;
            vlanMC.mbr = vlan4K.mbr;                   
            vlanMC.fid = vlan4K.fid;
            vlanMC.msti= vlan4K.msti;
            vlanMC.meteridx= vlan4K.meteridx;
            vlanMC.envlanpol= vlan4K.envlanpol;
            vlanMC.lurep= vlan4K.lurep;    

        if ((retVal = rtl8370_setAsicVlanMemberConfig(empty_idx,&vlanMC))!=RT_ERR_OK)
                return retVal; 

            if ((retVal = rtl8370_setAsic1xGuestVidx(empty_idx))!=RT_ERR_OK)
                return retVal;    

            return RT_ERR_OK;            
        }    
    

    /* 32 member configuration is full, found a unused entry to replace */
    for (i = 0; i < RTK_MAX_NUM_OF_VLAN_INDEX; i++)
    {    
        bUsed = FALSE;   

        for (j = 0; j < RTK_MAX_NUM_OF_PORT; j++)
        {    
            if ((retVal = rtl8370_getAsicVlanPortBasedVID(j, &index, &pri))!=RT_ERR_OK)
                return retVal;

            if (i == index)/*index i is in use by port j*/
            {
                bUsed = TRUE;
                break;
            } 

            for (k=0;k<=RTK_PROTOVLAN_GROUP_ID_MAX;k++)
            {
                if ((retVal = rtl8370_getAsicVlanPortAndProtocolBased(j, k, &ppb_vlan_cfg)) != RT_ERR_OK)
                    return retVal; 
                if (TRUE == ppb_vlan_cfg.valid&& ppb_vlan_cfg.vlan_idx == i)
                {
                    bUsed = TRUE;
                    break;
                }
            }            
        }

        if (FALSE == bUsed)/*found a unused index, replace it*/
        {
            vlan4K.vid = vid;
            if ((retVal = rtl8370_getAsicVlan4kEntry(&vlan4K))!=RT_ERR_OK)
                return retVal; 
            vlanMC.mbr = vlan4K.mbr;                   
            vlanMC.fid = vlan4K.fid;
            vlanMC.msti= vlan4K.msti;
            vlanMC.meteridx= vlan4K.meteridx;
            vlanMC.envlanpol= vlan4K.envlanpol;
            vlanMC.lurep= vlan4K.lurep;               
            if ((retVal = rtl8370_setAsicVlanMemberConfig(i,&vlanMC))!=RT_ERR_OK)
                return retVal; 

            if ((retVal = rtl8370_setAsic1xGuestVidx(i))!=RT_ERR_OK)
                return retVal;    

            return RT_ERR_OK;            
        }
    }    
    
    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_dot1x_guestVlan_get
 * Description:
 *      Get 802.1x guest VLAN configuration
 * Input:
 *      None
 * Output:
 *      pVid - 802.1x guest VLAN ID
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get 802.1x guest VLAN information.
 */
rtk_api_ret_t rtk_dot1x_guestVlan_get(rtk_vlan_t *pVid)
{
    rtk_api_ret_t retVal;
    uint32 gvidx;
    rtl8370_vlanconfiguser vlanMC;
    
    if ((retVal = rtl8370_getAsic1xGuestVidx(&gvidx))!=RT_ERR_OK)
        return retVal; 

    if ((retVal = rtl8370_getAsicVlanMemberConfig(gvidx,&vlanMC))!=RT_ERR_OK)
        return retVal; 

    *pVid = vlanMC.evid;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_guestVlan2Auth_set
 * Description:
 *      Set 802.1x guest VLAN to auth host configuration
 * Input:
 *      enable - The status of guest VLAN to auth host.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *      The operational direction of 802.1x guest VLAN to auth host control is as following:
 *      ENABLED
 *      DISABLED
 */
rtk_api_ret_t rtk_dot1x_guestVlan2Auth_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8370_setAsic1xGVOpdir(enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_guestVlan2Auth_get
 * Description:
 *      Get 802.1x guest VLAN to auth host configuration
 * Input:
 *      None
 * Output:
 *      pEnable - The status of guest VLAN to auth host.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get 802.1x guest VLAN to auth host information.
 */
rtk_api_ret_t rtk_dot1x_guestVlan2Auth_get(rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsic1xGVOpdir(pEnable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

#endif

/* Function Name:
 *      rtk_trunk_port_set
 * Description:
 *      Set trunking group available port mask
 * Input:
 *      trk_gid - trunk group id
 *      trunk_member_portmask - Logic trunking member port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_LA_TRUNK_ID - Invalid trunking group
 *      RT_ERR_PORT_MASK - Invalid portmask.
 * Note:
 *      The API can set 4 port trunking group enabled port mask. Each port trunking group has max 4 ports.
 *      If enabled port mask has less than 2 ports available setting, then this trunking group function is disabled. 
 *      The group port members for trunk group are as following: 
 *      TRUNK_GROUP0: port 0 to port 3.
 *      TRUNK_GROUP1: port 4 to port 7.
 *      TRUNK_GROUP2: port 8 to port 11.
 *      TRUNK_GROUP3: port 12 to port 15.
 */

rtk_api_ret_t rtk_trunk_port_set(rtk_trunk_group_t trk_gid, rtk_portmask_t trunk_member_portmask)
{
    rtk_api_ret_t retVal;
    uint32 pmsk;

    if (trk_gid>=TRUNK_GROUP_END)
        return RT_ERR_LA_TRUNK_ID; 

    if (trunk_member_portmask.bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK; 

    if ((trunk_member_portmask.bits[0]|RTK_PORT_TRUNK_GROUP_MASK(trk_gid))!=RTK_PORT_TRUNK_GROUP_MASK(trk_gid))
        return RT_ERR_PORT_MASK;

    pmsk = (trunk_member_portmask.bits[0]&RTK_PORT_TRUNK_GROUP_MASK(trk_gid))>>RTK_PORT_TRUNK_GROUP_OFFSET(trk_gid);

    if ((retVal = rtl8370_setAsicTrunkingGroup(trk_gid,pmsk))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trunk_port_get
 * Description:
 *      Get trunking group available port mask
 * Input:
 *      trk_gid - trunk group id
 * Output:
 *      pTrunk_member_portmask - Logic trunking member port mask
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_LA_TRUNK_ID - Invalid trunking group
 * Note:
 *      The API can get 4 port trunking group enabled port mask. Each port trunking group has max 4 ports.
 *      If enabled port mask has less than 2 ports available setting, then this trunking group function is disabled.
 *      The group port members for trunk group are as following: 
 *      TRUNK_GROUP0: port 0 to port 3.
 *      TRUNK_GROUP1: port 4 to port 7.
 *      TRUNK_GROUP2: port 8 to port 11.
 *      TRUNK_GROUP3: port 12 to port 15.
 */
rtk_api_ret_t rtk_trunk_port_get(rtk_trunk_group_t trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    rtk_api_ret_t retVal;

    uint32 pmsk;

    if (trk_gid>=TRUNK_GROUP_END)
        return RT_ERR_LA_TRUNK_ID; 

    if ((retVal = rtl8370_getAsicTrunkingGroup(trk_gid,&pmsk))!=RT_ERR_OK)
        return retVal;

    pTrunk_member_portmask->bits[0] = pmsk<<RTK_PORT_TRUNK_GROUP_OFFSET(trk_gid);
        
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trunk_distributionAlgorithm_set
 * Description:
 *      Set port trunking hash select sources
 * Input:
 *      trk_gid - trunk group id
 *      algo_bitmask -  Bitmask of the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_LA_TRUNK_ID - Invalid trunking group
 *      RT_ERR_LA_HASHMASK - Hash algorithm selection error.
 *      RT_ERR_PORT_MASK - Invalid portmask.
 * Note:
 *      The API can set port trunking hash algorithm sources.
 *      7 bits mask for link aggregation group0 hash parameter selection {DIP, SIP, DMAC, SMAC, SPA}
 *      0b0000001: SPA
 *      0b0000010: SMAC
 *      0b0000100: DMAC
 *      0b0001000: SIP
 *      0b0010000: DIP
 *      0b0100000: TCP/UDP Source Port
 *      0b1000000: TCP/UDP Destination Port
 *      Example:
 *      0b0000011: SMAC & SPA
 *      Note that it could be an arbitrary combination or independent set
 */
rtk_api_ret_t rtk_trunk_distributionAlgorithm_set(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t algo_bitmask)
{
    rtk_api_ret_t retVal;

    if (trk_gid != RTK_WHOLE_SYSTEM)
        return RT_ERR_LA_TRUNK_ID;
    
    if (algo_bitmask.value[0]>=128)
        return RT_ERR_LA_HASHMASK;
        
    if ((retVal = rtl8370_setAsicTrunkingHashSelect(algo_bitmask.value[0]))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trunk_distributionAlgorithm_get
 * Description:
 *      Get port trunking hash select sources
 * Input:
 *      trk_gid - trunk group id
 * Output:
 *      pAlgo_bitmask -  Bitmask of the distribution algorithm
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_LA_TRUNK_ID - Invalid trunking group
 * Note:
 *      The API can get port trunking hash algorithm sources.
 */
rtk_api_ret_t rtk_trunk_distributionAlgorithm_get(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t *pAlgo_bitmask)
{
    rtk_api_ret_t retVal;
    
    if (trk_gid != RTK_WHOLE_SYSTEM)
        return RT_ERR_LA_TRUNK_ID;
    
        
    if ((retVal = rtl8370_getAsicTrunkingHashSelect((uint32*)&pAlgo_bitmask->value[0]))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trunk_qeueuEmptyStatus_get
 * Description:
 *      Get current output queue if empty status
 * Input:
 *      trk_gid - trunk group id
 * Output:
 *      pPortmask - queue empty port mask, 1 for empty and 0 for not empty
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      The API can get queues are empty port mask
 */
rtk_api_ret_t rtk_trunk_qeueuEmptyStatus_get(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicQeueuEmptyStatus(&pPortmask->bits[0]))!=RT_ERR_OK)
        return retVal;    

    return RT_ERR_OK;
}

#if !defined(_REDUCE_CODE)
static rtk_api_ret_t _rtk_switch_init0(void)
{
    rtk_api_ret_t retVal;
    uint32 index,regData;
    uint32 busyFlag,cnt;
    CONST_T uint16 chipData0[][2] = {{0x133f,0x0030},{0x133e,0x000e},{0x221f,0x0000},{0x2215,0x1006},
                                     {0x221f,0x0005},{0x2200,0x00c6},{0x221f,0x0007},{0x221e,0x0048},
                                     {0x2215,0x6412},{0x2216,0x6412},{0x2217,0x6412},{0x2218,0x6412},
                                     {0x2219,0x6412},{0x221A,0x6412},{0x221f,0x0001},{0x220c,0xdbf0},
                                     {0x2209,0x2576},{0x2207,0x287E},{0x220A,0x68E5},{0x221D,0x3DA4},
                                     {0x221C,0xE7F7},{0x2214,0x7F52},{0x2218,0x7FCE},{0x2208,0x04B7},
                                     {0x2206,0x4072},{0x2210,0xF05E},{0x221B,0xB414},{0x221F,0x0003},
                                     {0x221A,0x06A6},{0x2210,0xF05E},{0x2213,0x06EB},{0x2212,0xF4D2},
                                     {0x220E,0xE120},{0x2200,0x7C00},{0x2202,0x5FD0},{0x220D,0x0207},
                                     {0x221f,0x0002},{0x2205,0x0978},{0x2202,0x8C01},{0x2207,0x3620},
                                     {0x221C,0x0001},{0x2203,0x0420},{0x2204,0x80C8},{0x133e,0x0ede},
                                     {0x221f,0x0002},{0x220c,0x0073},{0x220d,0xEB65},{0x220e,0x51d1},
                                     {0x220f,0x5dcb},{0x2210,0x3044},{0x2211,0x1800},{0x2212,0x7E00},
                                     {0x2213,0x0000},{0x133f,0x0010},{0x133e,0x0ffe},{0x207f,0x0002},
                                     {0x2074,0x3D22},{0x2075,0x2000},{0x2076,0x6040},{0x2077,0x0000},
                                     {0x2078,0x0f0a},{0x2079,0x50AB},{0x207a,0x0000},{0x207b,0x0f0f},
                                     {0x205f,0x0002},{0x2054,0xFF00},{0x2055,0x000A},{0x2056,0x000A},
                                     {0x2057,0x0005},{0x2058,0x0005},{0x2059,0x0000},{0x205A,0x0005},
                                     {0x205B,0x0005},{0x205C,0x0005},{0x209f,0x0002},{0x2094,0x00AA},
                                     {0x2095,0x00AA},{0x2096,0x00AA},{0x2097,0x00AA},{0x2098,0x0055},
                                     {0x2099,0x00AA},{0x209A,0x00AA},{0x209B,0x00AA},{0x1363,0x8354},
                                     {0x1270,0x3333},{0x1271,0x3333},{0x1272,0x3333},{0x1330,0x00DB},
                                     {0x1203,0xff00},{0x1200,0x7fc4},{0x121d,0x1006},{0x121e,0x03e8},
                                     {0x121f,0x02b3},{0x1220,0x028f},{0x1221,0x029b},{0x1222,0x0277},
                                     {0x1223,0x02b3},{0x1224,0x028f},{0x1225,0x029b},{0x1226,0x0277},
                                     {0x1227,0x00c0},{0x1228,0x00b4},{0x122f,0x00c0},{0x1230,0x00b4},
                                     {0x1229,0x0020},{0x122a,0x000c},{0x1231,0x0030},{0x1232,0x0024},
                                     {0x0219,0x0032},{0x0200,0x03e8},{0x0201,0x03e8},{0x0202,0x03e8},
                                     {0x0203,0x03e8},{0x0204,0x03e8},{0x0205,0x03e8},{0x0206,0x03e8},
                                     {0x0207,0x03e8},{0x0218,0x0032},{0x0208,0x029b},{0x0209,0x029b},
                                     {0x020a,0x029b},{0x020b,0x029b},{0x020c,0x029b},{0x020d,0x029b},
                                     {0x020e,0x029b},{0x020f,0x029b},{0x0210,0x029b},{0x0211,0x029b},
                                     {0x0212,0x029b},{0x0213,0x029b},{0x0214,0x029b},{0x0215,0x029b},
                                     {0x0216,0x029b},{0x0217,0x029b},{0x0900,0x0000},{0x0901,0x0000},
                                     {0x0902,0x0000},{0x0903,0x0000},{0x0865,0x3210},{0x087b,0x0000},
                                     {0x087c,0xff00},{0x087d,0x0000},{0x087e,0x0000},{0x0801,0x0100},
                                     {0x0802,0x0100},{0x1700,0x014C},{0x0301,0x00FF},{0x12AA,0x0096},
                                     {0x133f,0x0030},{0x133e,0x000e},{0x221f,0x0005},{0x2200,0x00C4},
                                     {0x221f,0x0000},{0x2210,0x05EF},{0x2204,0x05E1},{0x2200,0x1340},
                                     {0x133f,0x0010},{0x20A0,0x0940},{0x20C0,0x0940},{0x20E0,0x0940},
                                     {0xFFFF, 0xABCD}};
 
    CONST_T uint16 chipData1[][2] = {{0x133f,0x0030},{0x133e,0x000e},{0x221f,0x0000},{0x2215,0x1006}, 
                                     {0x221f,0x0005},{0x2200,0x00c6},{0x221f,0x0007},{0x221e,0x0048},
                                     {0x2215,0x6412},{0x2216,0x6412},{0x2217,0x6412},{0x2218,0x6412},
                                     {0x2219,0x6412},{0x221A,0x6412},{0x221f,0x0001},{0x220c,0xdbf0},
                                     {0x2209,0x2576},{0x2207,0x287E},{0x220A,0x68E5},{0x221D,0x3DA4},
                                     {0x221C,0xE7F7},{0x2214,0x7F52},{0x2218,0x7FCE},{0x2208,0x04B7},
                                     {0x2206,0x4072},{0x2210,0xF05E},{0x221B,0xB414},{0x221F,0x0003},
                                     {0x221A,0x06A6},{0x2210,0xF05E},{0x2213,0x06EB},{0x2212,0xF4D2},
                                     {0x220E,0xE120},{0x2200,0x7C00},{0x2202,0x5FD0},{0x220D,0x0207},
                                     {0x221f,0x0002},{0x2205,0x0978},{0x2202,0x8C01},{0x2207,0x3620},
                                     {0x221C,0x0001},{0x2203,0x0420},{0x2204,0x80C8},{0x133e,0x0ede},
                                     {0x221f,0x0002},{0x220c,0x0073},{0x220d,0xEB65},{0x220e,0x51d1},
                                     {0x220f,0x5dcb},{0x2210,0x3044},{0x2211,0x1800},{0x2212,0x7E00},
                                     {0x2213,0x0000},{0x133f,0x0010},{0x133e,0x0ffe},{0x207f,0x0002},
                                     {0x2074,0x3D22},{0x2075,0x2000},{0x2076,0x6040},{0x2077,0x0000},
                                     {0x2078,0x0f0a},{0x2079,0x50AB},{0x207a,0x0000},{0x207b,0x0f0f},
                                     {0x205f,0x0002},{0x2054,0xFF00},{0x2055,0x000A},{0x2056,0x000A},
                                     {0x2057,0x0005},{0x2058,0x0005},{0x2059,0x0000},{0x205A,0x0005},
                                     {0x205B,0x0005},{0x205C,0x0005},{0x209f,0x0002},{0x2094,0x00AA},
                                     {0x2095,0x00AA},{0x2096,0x00AA},{0x2097,0x00AA},{0x2098,0x0055},
                                     {0x2099,0x00AA},{0x209A,0x00AA},{0x209B,0x00AA},{0x1363,0x8354},
                                     {0x1270,0x3333},{0x1271,0x3333},{0x1272,0x3333},{0x1330,0x00DB},
                                     {0x1203,0xff00},{0x1200,0x7fc4},{0x121d,0x1b06},{0x121e,0x07f0},
                                     {0x121f,0x0438},{0x1220,0x040f},{0x1221,0x040f},{0x1222,0x03eb},
                                     {0x1223,0x0438},{0x1224,0x040f},{0x1225,0x040f},{0x1226,0x03eb},
                                     {0x1227,0x0144},{0x1228,0x0138},{0x122f,0x0144},{0x1230,0x0138},
                                     {0x1229,0x0020},{0x122a,0x000c},{0x1231,0x0030},{0x1232,0x0024},
                                     {0x0219,0x0032},{0x0200,0x07d0},{0x0201,0x07d0},{0x0202,0x07d0},
                                     {0x0203,0x07d0},{0x0204,0x07d0},{0x0205,0x07d0},{0x0206,0x07d0},
                                     {0x0207,0x07d0},{0x0218,0x0032},{0x0208,0x0190},{0x0209,0x0190},
                                     {0x020a,0x0190},{0x020b,0x0190},{0x020c,0x0190},{0x020d,0x0190},
                                     {0x020e,0x0190},{0x020f,0x0190},{0x0210,0x0190},{0x0211,0x0190},
                                     {0x0212,0x0190},{0x0213,0x0190},{0x0214,0x0190},{0x0215,0x0190},
                                     {0x0216,0x0190},{0x0217,0x0190},{0x0900,0x0000},{0x0901,0x0000},
                                     {0x0902,0x0000},{0x0903,0x0000},{0x0865,0x3210},{0x087b,0x0000},
                                     {0x087c,0xff00},{0x087d,0x0000},{0x087e,0x0000},{0x0801,0x0100},
                                     {0x0802,0x0100},{0x1700,0x0125},{0x0301,0x00FF},{0x12AA,0x0096},
                                     {0x133f,0x0030},{0x133e,0x000e},{0x221f,0x0005},{0x2200,0x00C4},
                                     {0x221f,0x0000},{0x2210,0x05EF},{0x2204,0x05E1},{0x2200,0x1340},
                                     {0x133f,0x0010},{0xFFFF, 0xABCD}};

    if ((retVal = rtl8370_setAsicReg(0x13C2,0x0249))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicRegBits(0x1302, 0x7,&regData))!=RT_ERR_OK)
        return retVal;   

    index = 0;
    switch(regData)
    {
    case 0x0000:
        while (chipData0[index][0] != 0xFFFF && chipData0[index][1] != 0xABCD)
        {    
            if ((chipData0[index][0]&0xF000)==0x2000)
            {
                cnt = 0;
                busyFlag = 1;
                while (busyFlag&&cnt<5)
                {
                    cnt++;
                    if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                        return retVal;
                }
                if (5 == cnt)
                    return RT_ERR_BUSYWAIT_TIMEOUT;
            
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32)chipData0[index][1])) !=  RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32)chipData0[index][0])) !=  RT_ERR_OK)
                    return retVal;    
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                    return retVal; 
            }
            else
            {
                if (RT_ERR_OK != rtl8370_setAsicReg((uint32) chipData0[index][0],(uint32) chipData0[index][1]))
                    return RT_ERR_FAILED;
            }
            index ++;    
        } 
    case 0x0001:    
        while (chipData1[index][0] != 0xFFFF && chipData1[index][1] != 0xABCD)
        {    
            if ((chipData1[index][0]&0xF000)==0x2000)
            {
                cnt = 0;
                busyFlag = 1;
                while (busyFlag&&cnt<5)
                {
                    cnt++;
                    if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                        return retVal;
                }
                if (5 == cnt)
                    return RT_ERR_BUSYWAIT_TIMEOUT;

                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32) chipData1[index][1])) !=  RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32) chipData1[index][0])) !=  RT_ERR_OK)
                    return retVal;    
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                    return retVal; 
            }
            else
            {
                if (RT_ERR_OK != rtl8370_setAsicReg((uint32) chipData1[index][0],(uint32) chipData1[index][1]))
                    return RT_ERR_FAILED;
            } 
            index ++;    
        }
        
        if (RT_ERR_OK != rtl8370_setAsicReg(0x1700,0x135))
            return RT_ERR_FAILED;    
        break;
    case 0x0002:    
        while (chipData1[index][0] != 0xFFFF && chipData1[index][1] != 0xABCD)
        {    
            if ((chipData1[index][0]&0xF000)==0x2000)
            {
                cnt = 0;
                busyFlag = 1;
                while (busyFlag&&cnt<5)
                {
                    cnt++;
                    if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                        return retVal;
                }
                if (5 == cnt)
                    return RT_ERR_BUSYWAIT_TIMEOUT;

                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32) chipData1[index][1])) !=  RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32) chipData1[index][0])) !=  RT_ERR_OK)
                    return retVal;    
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                    return retVal; 
            }
            else
            {
                if (RT_ERR_OK != rtl8370_setAsicReg((uint32) chipData1[index][0],(uint32) chipData1[index][1]))
                    return RT_ERR_FAILED;
            } 
            index ++;    
        }
        
        if (RT_ERR_OK != rtl8370_setAsicReg(0x1700,0x135))
            return RT_ERR_FAILED;    
        break;
    default:
        return RT_ERR_FAILED;
    }

    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_LED_ACTIVE_LOW_CFG0, 0x6677))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_LED_ACTIVE_LOW_CFG1, 0x7406))!=RT_ERR_OK)
        return retVal;     
    
    if ((retVal = rtl8370_getAsicReg(RTL8370_REG_MISCELLANEOUS_CONFIGURE0,&regData))!=RT_ERR_OK)
        return retVal;

    if (regData & RTL8370_EFUSE_EN_MASK)
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT3_LED_ACTIVE_LOW_OFFSET+2, 0))!=RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT3_LED_ACTIVE_LOW_OFFSET+2, 1))!=RT_ERR_OK)
            return retVal;
    }
            
    if (regData & RTL8370_AUTOLOAD_EN_MASK)
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT2_LED_ACTIVE_LOW_OFFSET, 0))!=RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT2_LED_ACTIVE_LOW_OFFSET, 1))!=RT_ERR_OK)
            return retVal;
    }

    if (regData & RTL8370_DW8051_EN_MASK)
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT2_LED_ACTIVE_LOW_OFFSET+2, 0))!=RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT2_LED_ACTIVE_LOW_OFFSET+2, 1))!=RT_ERR_OK)
            return retVal;
    }

    if ((retVal = rtl8370_getAsicReg(RTL8370_REG_MISCELLANEOUS_CONFIGURE1,&regData))!=RT_ERR_OK)
        return retVal;

    if (regData & RTL8370_EEPROM_ADDRESS_16B_MASK)
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG1, RTL8370_PORT4_LED_ACTIVE_LOW_OFFSET, 1))!=RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG1, RTL8370_PORT4_LED_ACTIVE_LOW_OFFSET, 0))!=RT_ERR_OK)
            return retVal;
    }
            
    if (regData & 0x1000)
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT3_LED_ACTIVE_LOW_OFFSET, 1))!=RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT3_LED_ACTIVE_LOW_OFFSET, 0))!=RT_ERR_OK)
            return retVal;
    }

    if ((retVal = rtl8370_getAsicReg(RTL8370_REG_DW8051_RDY,&regData))!=RT_ERR_OK)
        return retVal;

    if (regData & 0x10)
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG1, RTL8370_PORT5_LED_ACTIVE_LOW_OFFSET, 1))!=RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG1, RTL8370_PORT5_LED_ACTIVE_LOW_OFFSET, 0))!=RT_ERR_OK)
            return retVal;
    }
            
    if (regData & 0x20)
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT1_LED_ACTIVE_LOW_OFFSET+2, 1))!=RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT1_LED_ACTIVE_LOW_OFFSET+2, 0))!=RT_ERR_OK)
            return retVal;
    }

    if (regData & 0x40)
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT1_LED_ACTIVE_LOW_OFFSET, 1))!=RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_ACTIVE_LOW_CFG0, RTL8370_PORT1_LED_ACTIVE_LOW_OFFSET, 0))!=RT_ERR_OK)
            return retVal;
    }

    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_PARA_LED_IO_EN1,0))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_PARA_LED_IO_EN2,0))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_SCAN0_LED_IO_EN,0))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicReg(RTL8370_REG_SCAN1_LED_IO_EN,0))!=RT_ERR_OK)
        return retVal;
        
    /*Set learn limit to 8256*/
    for (index= 0; index <= RTK_MAX_NUM_OF_PORT; index++)
    {
        if ((retVal = rtl8370_setAsicLutLearnLimitNo(index,RTK_MAX_NUM_OF_LEARN_LIMIT))!=RT_ERR_OK)
            return retVal;
    } 


    
    return RT_ERR_OK;
}
#endif

static rtk_api_ret_t _rtk_switch_init1(void)
{
    rtk_api_ret_t retVal;
    uint32 index,regData;
#ifndef MDC_MDIO_OPERATION 
    uint32 busyFlag,cnt;
#endif
    CONST_T uint16 chipData0[][2] ={{0x1B24, 0x0000},{0x1B25, 0x0000},{0x1B26, 0x0000},{0x1B27, 0x0000},
                                    {0x207F, 0x0007},{0x207E, 0x000B},{0x2076, 0x1A00},{0x207F, 0x0000},
                                    {0x205F, 0x0007},{0x205E, 0x000A},{0x2059, 0x0000},{0x205A, 0x0000},
                                    {0x205B, 0x0000},{0x205C, 0x0000},{0x205E, 0x000B},{0x2055, 0x0500},
                                    {0x2056, 0x0000},{0x2057, 0x0000},{0x2058, 0x0000},{0x205F, 0x0000},
                                    {0x133F, 0x0030},{0x133E, 0x000E},{0x221F, 0x0005},{0x2201, 0x0700},
                                    {0x2205, 0x8B82},{0x2206, 0x05CB},{0x221F, 0x0007},{0x221E, 0x0008},
                                    {0x2219, 0x80C2},{0x221A, 0x0938},{0x221F, 0x0000},{0x221F, 0x0003},
                                    {0x2212, 0xC4D2},{0x220D, 0x0207},{0x221F, 0x0001},{0x2207, 0x267E},
                                    {0x221C, 0xE5F7},{0x221B, 0x0424},{0x221F, 0x0007},{0x221E, 0x0040},
                                    {0x2218, 0x0000},{0x221F, 0x0007},{0x221E, 0x002C},{0x2218, 0x008B},
                                    {0x221F, 0x0005},{0x2205, 0xFFF6},{0x2206, 0x0080},{0x2205, 0x8000},
                                    {0x2206, 0xF8E0},{0x2206, 0xE000},{0x2206, 0xE1E0},{0x2206, 0x01AC},
                                    {0x2206, 0x2408},{0x2206, 0xE08B},{0x2206, 0x84F7},{0x2206, 0x20E4},
                                    {0x2206, 0x8B84},{0x2206, 0xFC05},{0x2206, 0xF8FA},{0x2206, 0xEF69},
                                    {0x2206, 0xE08B},{0x2206, 0x86AC},{0x2206, 0x201A},{0x2206, 0xBF80},
                                    {0x2206, 0x59D0},{0x2206, 0x2402},{0x2206, 0x803D},{0x2206, 0xE0E0},
                                    {0x2206, 0xE4E1},{0x2206, 0xE0E5},{0x2206, 0x5806},{0x2206, 0x68C0},
                                    {0x2206, 0xD1D2},{0x2206, 0xE4E0},{0x2206, 0xE4E5},{0x2206, 0xE0E5},
                                    {0x2206, 0xEF96},{0x2206, 0xFEFC},{0x2206, 0x05FB},{0x2206, 0x0BFB},
                                    {0x2206, 0x58FF},{0x2206, 0x9E11},{0x2206, 0x06F0},{0x2206, 0x0C81},
                                    {0x2206, 0x8AE0},{0x2206, 0x0019},{0x2206, 0x1B89},{0x2206, 0xCFEB},
                                    {0x2206, 0x19EB},{0x2206, 0x19B0},{0x2206, 0xEFFF},{0x2206, 0x0BFF},
                                    {0x2206, 0x0425},{0x2206, 0x0807},{0x2206, 0x2640},{0x2206, 0x7227},
                                    {0x2206, 0x267E},{0x2206, 0x2804},{0x2206, 0xB729},{0x2206, 0x2576},
                                    {0x2206, 0x2A68},{0x2206, 0xE52B},{0x2206, 0xAD00},{0x2206, 0x2CDB},
                                    {0x2206, 0xF02D},{0x2206, 0x67BB},{0x2206, 0x2E7B},{0x2206, 0x0F2F},
                                    {0x2206, 0x7365},{0x2206, 0x31AC},{0x2206, 0xCC32},{0x2206, 0x2300},
                                    {0x2206, 0x332D},{0x2206, 0x1734},{0x2206, 0x7F52},{0x2206, 0x3510},
                                    {0x2206, 0x0036},{0x2206, 0x1000},{0x2206, 0x3710},{0x2206, 0x0038},
                                    {0x2206, 0x7FCE},{0x2206, 0x3CE5},{0x2206, 0xF73D},{0x2206, 0x3DA4},
                                    {0x2206, 0x6530},{0x2206, 0x3E67},{0x2206, 0x0053},{0x2206, 0x69D2},
                                    {0x2206, 0x0F6A},{0x2206, 0x012C},{0x2206, 0x6C2B},{0x2206, 0x136E},
                                    {0x2206, 0xE100},{0x2206, 0x6F12},{0x2206, 0xF771},{0x2206, 0x006B},
                                    {0x2206, 0x7306},{0x2206, 0xEB74},{0x2206, 0x94C7},{0x2206, 0x7698},
                                    {0x2206, 0x0A77},{0x2206, 0x5000},{0x2206, 0x788A},{0x2206, 0x1579},
                                    {0x2206, 0x7F6F},{0x2206, 0x7A06},{0x2206, 0xA600},{0x2205, 0x8B90},
                                    {0x2206, 0x8000},{0x2205, 0x8B92},{0x2206, 0x8000},{0x2205, 0x8B94},
                                    {0x2206, 0x8014},{0x2208, 0xFFFA},{0x2202, 0x3C65},{0x2205, 0xFFF6},
                                    {0x2206, 0x00F7},{0x221F, 0x0000},{0x221F, 0x0007},{0x221E, 0x0042},
                                    {0x2218, 0x0000},{0x221E, 0x002D},{0x2218, 0xF010},{0x221E, 0x0020},
                                    {0x2215, 0x0000},{0x221E, 0x0023},{0x2216, 0x8000},{0x221F, 0x0000},
                                    {0x133F, 0x0010},{0x133E, 0x0FFE},{0x1362, 0x0115},{0x1363, 0x0002},
                                    {0x1363, 0x0000},{0x1306, 0x000C},{0x1307, 0x000C},{0x1303, 0x0367},
                                    {0x1304, 0x7777},{0x1203, 0xFF00},{0x1200, 0x7FC4},{0x121D, 0x7D16},
                                    {0x121E, 0x03E8},{0x121F, 0x024E},{0x1220, 0x0230},{0x1221, 0x0244},
                                    {0x1222, 0x0226},{0x1223, 0x024E},{0x1224, 0x0230},{0x1225, 0x0244},
                                    {0x1226, 0x0226},{0x1227, 0x00C0},{0x1228, 0x00B4},{0x122F, 0x00C0},
                                    {0x1230, 0x00B4},{0x0208, 0x03E8},{0x0209, 0x03E8},{0x020A, 0x03E8},
                                    {0x020B, 0x03E8},{0x020C, 0x03E8},{0x020D, 0x03E8},{0x020E, 0x03E8},
                                    {0x020F, 0x03E8},{0x0210, 0x03E8},{0x0211, 0x03E8},{0x0212, 0x03E8},
                                    {0x0213, 0x03E8},{0x0214, 0x03E8},{0x0215, 0x03E8},{0x0216, 0x03E8},
                                    {0x0217, 0x03E8},{0x0865, 0x3210},{0x087B, 0x0000},{0x087C, 0xFF00},
                                    {0x087D, 0x0000},{0x087E, 0x0000},{0x0801, 0x0100},{0x0802, 0x0100},
                                    {0x0A20, 0x2040},{0x0A21, 0x2040},{0x0A22, 0x2040},{0x0A23, 0x2040},
                                    {0x0A24, 0x2040},{0x0A28, 0x2040},{0x0A29, 0x2040},{0x20A0, 0x0940},
                                    {0x20C0, 0x0940},{0x20E0, 0x0940},{0x130C, 0x0050},{0x1B03, 0x0876},
                                    {0xFFFF, 0xABCD}};

    CONST_T uint16 chipData1[][2] ={{0x1B24, 0x0000},{0x1B25, 0x0000},{0x1B26, 0x0000},{0x1B27, 0x0000},
                                    {0x207F, 0x0007},{0x207E, 0x000B},{0x2076, 0x1A00},{0x207F, 0x0000},
                                    {0x205F, 0x0007},{0x205E, 0x000A},{0x2059, 0x0000},{0x205A, 0x0000},
                                    {0x205B, 0x0000},{0x205C, 0x0000},{0x205E, 0x000B},{0x2055, 0x0500},
                                    {0x2056, 0x0000},{0x2057, 0x0000},{0x2058, 0x0000},{0x205F, 0x0000},
                                    {0x133F, 0x0030},{0x133E, 0x000E},{0x221F, 0x0005},{0x2201, 0x0700},
                                    {0x2205, 0x8B82},{0x2206, 0x05CB},{0x221F, 0x0007},{0x221E, 0x0008},
                                    {0x2219, 0x80C2},{0x221A, 0x0938},{0x221F, 0x0000},{0x221F, 0x0003},
                                    {0x2212, 0xC4D2},{0x220D, 0x0207},{0x221F, 0x0001},{0x2207, 0x267E},
                                    {0x221C, 0xE5F7},{0x221B, 0x0424},{0x221F, 0x0007},{0x221E, 0x0040},
                                    {0x2218, 0x0000},{0x221F, 0x0007},{0x221E, 0x002C},{0x2218, 0x008B},
                                    {0x221F, 0x0005},{0x2205, 0xFFF6},{0x2206, 0x0080},{0x2205, 0x8000},
                                    {0x2206, 0xF8E0},{0x2206, 0xE000},{0x2206, 0xE1E0},{0x2206, 0x01AC},
                                    {0x2206, 0x2408},{0x2206, 0xE08B},{0x2206, 0x84F7},{0x2206, 0x20E4},
                                    {0x2206, 0x8B84},{0x2206, 0xFC05},{0x2206, 0xF8FA},{0x2206, 0xEF69},
                                    {0x2206, 0xE08B},{0x2206, 0x86AC},{0x2206, 0x201A},{0x2206, 0xBF80},
                                    {0x2206, 0x59D0},{0x2206, 0x2402},{0x2206, 0x803D},{0x2206, 0xE0E0},
                                    {0x2206, 0xE4E1},{0x2206, 0xE0E5},{0x2206, 0x5806},{0x2206, 0x68C0},
                                    {0x2206, 0xD1D2},{0x2206, 0xE4E0},{0x2206, 0xE4E5},{0x2206, 0xE0E5},
                                    {0x2206, 0xEF96},{0x2206, 0xFEFC},{0x2206, 0x05FB},{0x2206, 0x0BFB},
                                    {0x2206, 0x58FF},{0x2206, 0x9E11},{0x2206, 0x06F0},{0x2206, 0x0C81},
                                    {0x2206, 0x8AE0},{0x2206, 0x0019},{0x2206, 0x1B89},{0x2206, 0xCFEB},
                                    {0x2206, 0x19EB},{0x2206, 0x19B0},{0x2206, 0xEFFF},{0x2206, 0x0BFF},
                                    {0x2206, 0x0425},{0x2206, 0x0807},{0x2206, 0x2640},{0x2206, 0x7227},
                                    {0x2206, 0x267E},{0x2206, 0x2804},{0x2206, 0xB729},{0x2206, 0x2576},
                                    {0x2206, 0x2A68},{0x2206, 0xE52B},{0x2206, 0xAD00},{0x2206, 0x2CDB},
                                    {0x2206, 0xF02D},{0x2206, 0x67BB},{0x2206, 0x2E7B},{0x2206, 0x0F2F},
                                    {0x2206, 0x7365},{0x2206, 0x31AC},{0x2206, 0xCC32},{0x2206, 0x2300},
                                    {0x2206, 0x332D},{0x2206, 0x1734},{0x2206, 0x7F52},{0x2206, 0x3510},
                                    {0x2206, 0x0036},{0x2206, 0x1000},{0x2206, 0x3710},{0x2206, 0x0038},
                                    {0x2206, 0x7FCE},{0x2206, 0x3CE5},{0x2206, 0xF73D},{0x2206, 0x3DA4},
                                    {0x2206, 0x6530},{0x2206, 0x3E67},{0x2206, 0x0053},{0x2206, 0x69D2},
                                    {0x2206, 0x0F6A},{0x2206, 0x012C},{0x2206, 0x6C2B},{0x2206, 0x136E},
                                    {0x2206, 0xE100},{0x2206, 0x6F12},{0x2206, 0xF771},{0x2206, 0x006B},
                                    {0x2206, 0x7306},{0x2206, 0xEB74},{0x2206, 0x94C7},{0x2206, 0x7698},
                                    {0x2206, 0x0A77},{0x2206, 0x5000},{0x2206, 0x788A},{0x2206, 0x1579},
                                    {0x2206, 0x7F6F},{0x2206, 0x7A06},{0x2206, 0xA600},{0x2205, 0x8B90},
                                    {0x2206, 0x8000},{0x2205, 0x8B92},{0x2206, 0x8000},{0x2205, 0x8B94},
                                    {0x2206, 0x8014},{0x2208, 0xFFFA},{0x2202, 0x3C65},{0x2205, 0xFFF6},
                                    {0x2206, 0x00F7},{0x221F, 0x0000},{0x221F, 0x0007},{0x221E, 0x0042},
                                    {0x2218, 0x0000},{0x221E, 0x002D},{0x2218, 0xF010},{0x221E, 0x0020},
                                    {0x2215, 0x0000},{0x221E, 0x0023},{0x2216, 0x8000},{0x221F, 0x0000},
                                    {0x133F, 0x0010},{0x133E, 0x0FFE},{0x1362, 0x0115},{0x1363, 0x0002},
                                    {0x1363, 0x0000},{0x1306, 0x000C},{0x1307, 0x000C},{0x1303, 0x0367},
                                    {0x1304, 0x7777},{0x1203, 0xFF00},{0x1200, 0x7FC4},{0x0865, 0x3210},
                                    {0x087B, 0x0000},{0x087C, 0xFF00},{0x087D, 0x0000},{0x087E, 0x0000},
                                    {0x0801, 0x0100},{0x0802, 0x0100},{0x0A20, 0x2040},{0x0A21, 0x2040},
                                    {0x0A22, 0x2040},{0x0A23, 0x2040},{0x0A24, 0x2040},{0x0A25, 0x2040},
                                    {0x0A26, 0x2040},{0x0A27, 0x2040},{0x0A28, 0x2040},{0x0A29, 0x2040},
                                    {0x130C, 0x0050},{0x1B03, 0x0876},{0xFFFF, 0xABCD}};

    if ((retVal = rtl8370_setAsicReg(0x13C2,0x0249))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicRegBits(0x1302, 0x7,&regData))!=RT_ERR_OK)
        return retVal;  

#ifdef MDC_MDIO_OPERATION  
    index = 0;
    switch(regData)
    {
        case 0x0000:
            while (chipData0[index][0] != 0xFFFF && chipData0[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData0[index][0],(uint32)chipData0[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            } 
            break;
        case 0x0001:    
            while (chipData1[index][0] != 0xFFFF && chipData1[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData1[index][0],(uint32)chipData1[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            }            
            break;
        case 0x0002:    
            while (chipData1[index][0] != 0xFFFF && chipData1[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData1[index][0],(uint32)chipData1[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            }            
            break;
        default:
            return RT_ERR_FAILED;
    }
#else 
    index = 0;
    switch(regData)
    {
        
        case 0x0000:
            while (chipData0[index][0] != 0xFFFF && chipData0[index][1] != 0xABCD)
            {    
                if ((chipData0[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
                    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32)chipData0[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32)chipData0[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData0[index][0],(uint32)chipData0[index][1]))
                        return RT_ERR_FAILED;
                }
                index ++;    
            } 
            break;
        case 0x0001:    
            while (chipData1[index][0] != 0xFFFF && chipData1[index][1] != 0xABCD)
            {    
                if ((chipData1[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
        
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32)chipData1[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32)chipData1[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData1[index][0],(uint32)chipData1[index][1]))
                        return RT_ERR_FAILED;
                } 
                index ++;    
            }            
            break;
        case 0x0002:    
            while (chipData1[index][0] != 0xFFFF && chipData1[index][1] != 0xABCD)
            {    
                if ((chipData1[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
        
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32)chipData1[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32)chipData1[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData1[index][0],(uint32)chipData1[index][1]))
                        return RT_ERR_FAILED;
                } 
                index ++;    
            }            
            break;
        default:
            return RT_ERR_FAILED;
    } 
#endif /*End of #ifdef MDC_MDIO_OPERATION*/

    return RT_ERR_OK;

}

static rtk_api_ret_t _rtk_switch_init2(void)
{
    rtk_api_ret_t retVal;
    uint32 index,regData;
#ifndef MDC_MDIO_OPERATION 
    uint32 busyFlag,cnt;
#endif
    CONST_T uint16 chipData0[][2] ={{0x1B24, 0x0000},{0x1B25, 0x0000},{0x1B26, 0x0000},{0x1B27, 0x0000},
                                    {0x207F, 0x0007},{0x207E, 0x000B},{0x2076, 0x1A00},{0x207E, 0x000A},
                                    {0x2078, 0x1D22},{0x207F, 0x0000},{0x205F, 0x0007},{0x205E, 0x000A},
                                    {0x2059, 0x0000},{0x205A, 0x0000},{0x205B, 0x0000},{0x205C, 0x0000},
                                    {0x205E, 0x000B},{0x2055, 0x0500},{0x2056, 0x0000},{0x2057, 0x0000},
                                    {0x2058, 0x0000},{0x205F, 0x0000},{0x1362, 0x0115},{0x1363, 0x0002},
                                    {0x1363, 0x0000},{0x133F, 0x0030},{0x133E, 0x000E},{0x221F, 0x0007},
                                    {0x221E, 0x0040},{0x2218, 0x0000},{0x221F, 0x0007},{0x221E, 0x002C},
                                    {0x2218, 0x008B},{0x221F, 0x0005},{0x2205, 0x8B6E},{0x2206, 0x0000},
                                    {0x220F, 0x0100},{0x2205, 0xFFF6},{0x2206, 0x0080},{0x2205, 0x8000},
                                    {0x2206, 0x0280},{0x2206, 0x1EF7},{0x2206, 0x00E0},{0x2206, 0xFFF7},
                                    {0x2206, 0xA080},{0x2206, 0x02AE},{0x2206, 0xF602},{0x2206, 0x8046},
                                    {0x2206, 0x0201},{0x2206, 0x5002},{0x2206, 0x0163},{0x2206, 0x0280},
                                    {0x2206, 0xCD02},{0x2206, 0x0179},{0x2206, 0xAEE7},{0x2206, 0xBF80},
                                    {0x2206, 0x61D7},{0x2206, 0x8580},{0x2206, 0xD06C},{0x2206, 0x0229},
                                    {0x2206, 0x71EE},{0x2206, 0x8B64},{0x2206, 0x00EE},{0x2206, 0x8570},
                                    {0x2206, 0x00EE},{0x2206, 0x8571},{0x2206, 0x00EE},{0x2206, 0x8AFC},
                                    {0x2206, 0x07EE},{0x2206, 0x8AFD},{0x2206, 0x73EE},{0x2206, 0xFFF6},
                                    {0x2206, 0x00EE},{0x2206, 0xFFF7},{0x2206, 0xFC04},{0x2206, 0xBF85},
                                    {0x2206, 0x80D0},{0x2206, 0x6C02},{0x2206, 0x2978},{0x2206, 0xE0E0},
                                    {0x2206, 0xE4E1},{0x2206, 0xE0E5},{0x2206, 0x5806},{0x2206, 0x68C0},
                                    {0x2206, 0xD1D2},{0x2206, 0xE4E0},{0x2206, 0xE4E5},{0x2206, 0xE0E5},
                                    {0x2206, 0x0425},{0x2206, 0x0807},{0x2206, 0x2640},{0x2206, 0x7227},
                                    {0x2206, 0x267E},{0x2206, 0x2804},{0x2206, 0xB729},{0x2206, 0x2576},
                                    {0x2206, 0x2A68},{0x2206, 0xE52B},{0x2206, 0xAD00},{0x2206, 0x2CDB},
                                    {0x2206, 0xF02D},{0x2206, 0x67BB},{0x2206, 0x2E7B},{0x2206, 0x0F2F},
                                    {0x2206, 0x7365},{0x2206, 0x31AC},{0x2206, 0xCC32},{0x2206, 0x2300},
                                    {0x2206, 0x332D},{0x2206, 0x1734},{0x2206, 0x7F52},{0x2206, 0x3510},
                                    {0x2206, 0x0036},{0x2206, 0x1000},{0x2206, 0x3710},{0x2206, 0x0038},
                                    {0x2206, 0x7FCE},{0x2206, 0x3CE5},{0x2206, 0xF73D},{0x2206, 0x3DA4},
                                    {0x2206, 0x6530},{0x2206, 0x3E67},{0x2206, 0x0053},{0x2206, 0x69D2},
                                    {0x2206, 0x0F6A},{0x2206, 0x012C},{0x2206, 0x6C2B},{0x2206, 0x136E},
                                    {0x2206, 0xE100},{0x2206, 0x6F12},{0x2206, 0xF771},{0x2206, 0x006B},
                                    {0x2206, 0x7306},{0x2206, 0xEB74},{0x2206, 0x94C7},{0x2206, 0x7698},
                                    {0x2206, 0x0A77},{0x2206, 0x5000},{0x2206, 0x788A},{0x2206, 0x1579},
                                    {0x2206, 0x7F6F},{0x2206, 0x7A06},{0x2206, 0xA6F8},{0x2206, 0xE08B},
                                    {0x2206, 0x8EAD},{0x2206, 0x2006},{0x2206, 0x0280},{0x2206, 0xDC02},
                                    {0x2206, 0x8109},{0x2206, 0xFC04},{0x2206, 0xF8F9},{0x2206, 0xE08B},
                                    {0x2206, 0x87AD},{0x2206, 0x2022},{0x2206, 0xE0E2},{0x2206, 0x00E1},
                                    {0x2206, 0xE201},{0x2206, 0xAD20},{0x2206, 0x11E2},{0x2206, 0xE022},
                                    {0x2206, 0xE3E0},{0x2206, 0x23AD},{0x2206, 0x3908},{0x2206, 0x5AC0},
                                    {0x2206, 0x9F04},{0x2206, 0xF724},{0x2206, 0xAE02},{0x2206, 0xF624},
                                    {0x2206, 0xE4E2},{0x2206, 0x00E5},{0x2206, 0xE201},{0x2206, 0xFDFC},
                                    {0x2206, 0x04F8},{0x2206, 0xF9E0},{0x2206, 0x8B85},{0x2206, 0xAD25},
                                    {0x2206, 0x48E0},{0x2206, 0x8AD2},{0x2206, 0xE18A},{0x2206, 0xD37C},
                                    {0x2206, 0x0000},{0x2206, 0x9E35},{0x2206, 0xEE8A},{0x2206, 0xD200},
                                    {0x2206, 0xEE8A},{0x2206, 0xD300},{0x2206, 0xE08A},{0x2206, 0xFCE1},
                                    {0x2206, 0x8AFD},{0x2206, 0xE285},{0x2206, 0x70E3},{0x2206, 0x8571},
                                    {0x2206, 0x0229},{0x2206, 0x3AAD},{0x2206, 0x2012},{0x2206, 0xEE8A},
                                    {0x2206, 0xD203},{0x2206, 0xEE8A},{0x2206, 0xD3B7},{0x2206, 0xEE85},
                                    {0x2206, 0x7000},{0x2206, 0xEE85},{0x2206, 0x7100},{0x2206, 0xAE11},
                                    {0x2206, 0x15E6},{0x2206, 0x8570},{0x2206, 0xE785},{0x2206, 0x71AE},
                                    {0x2206, 0x08EE},{0x2206, 0x8570},{0x2206, 0x00EE},{0x2206, 0x8571},
                                    {0x2206, 0x00FD},{0x2206, 0xFC04},{0x2206, 0xCCE2},{0x2206, 0x0000},
                                    {0x2205, 0xE142},{0x2206, 0x0701},{0x2205, 0xE140},{0x2206, 0x0405},
                                    {0x220F, 0x0000},{0x221F, 0x0000},{0x221F, 0x0005},{0x2205, 0x85E4},
                                    {0x2206, 0x8A14},{0x2205, 0x85E7},{0x2206, 0x7F6E},{0x221F, 0x0007},
                                    {0x221E, 0x002D},{0x2218, 0xF030},{0x221E, 0x0023},{0x2216, 0x0005},
                                    {0x2215, 0x005C},{0x2219, 0x0068},{0x2215, 0x0082},{0x2219, 0x000A},
                                    {0x2215, 0x00A1},{0x2219, 0x0081},{0x2215, 0x00AF},{0x2219, 0x0080},
                                    {0x2215, 0x00D4},{0x2219, 0x0000},{0x2215, 0x00E4},{0x2219, 0x0081},
                                    {0x2215, 0x00E7},{0x2219, 0x0080},{0x2215, 0x010D},{0x2219, 0x0083},
                                    {0x2215, 0x0118},{0x2219, 0x0083},{0x2215, 0x0120},{0x2219, 0x0082},
                                    {0x2215, 0x019C},{0x2219, 0x0081},{0x2215, 0x01A4},{0x2219, 0x0080},
                                    {0x2215, 0x01CD},{0x2219, 0x0000},{0x2215, 0x01DD},{0x2219, 0x0081},
                                    {0x2215, 0x01E0},{0x2219, 0x0080},{0x2215, 0x0147},{0x2219, 0x0096},
                                    {0x2216, 0x0000},{0x221E, 0x002D},{0x2218, 0xF010},{0x221F, 0x0005},
                                    {0x2205, 0x8B84},{0x2206, 0x0062},{0x221F, 0x0000},{0x220D, 0x0003},
                                    {0x220E, 0x0015},{0x220D, 0x4003},{0x220E, 0x0006},{0x133F, 0x0010},
                                    {0x133E, 0x0FFE},{0x12A4, 0x380A},{0x1303, 0x0367},{0x1304, 0x7777},
                                    {0x1203, 0xFF00},{0x1200, 0x7FC4},{0x121D, 0x7D16},{0x121E, 0x03E8},
                                    {0x121F, 0x024E},{0x1220, 0x0230},{0x1221, 0x0244},{0x1222, 0x0226},
                                    {0x1223, 0x024E},{0x1224, 0x0230},{0x1225, 0x0244},{0x1226, 0x0226},
                                    {0x1227, 0x00C0},{0x1228, 0x00B4},{0x122F, 0x00C0},{0x1230, 0x00B4},
                                    {0x0208, 0x03E8},{0x0209, 0x03E8},{0x020A, 0x03E8},{0x020B, 0x03E8},
                                    {0x020C, 0x03E8},{0x020D, 0x03E8},{0x020E, 0x03E8},{0x020F, 0x03E8},
                                    {0x0210, 0x03E8},{0x0211, 0x03E8},{0x0212, 0x03E8},{0x0213, 0x03E8},
                                    {0x0214, 0x03E8},{0x0215, 0x03E8},{0x0216, 0x03E8},{0x0217, 0x03E8},
                                    {0x0865, 0x3210},{0x087B, 0x0000},{0x087C, 0xFF00},{0x087D, 0x0000},
                                    {0x087E, 0x0000},{0x0801, 0x0100},{0x0802, 0x0100},{0x0A20, 0x2040},
                                    {0x0A21, 0x2040},{0x0A22, 0x2040},{0x0A23, 0x2040},{0x0A24, 0x2040},
                                    {0x0A28, 0x2040},{0x0A29, 0x2040},{0x20A0, 0x0940},{0x20C0, 0x0940},
                                    {0x20E0, 0x0940},{0x1B03, 0x0876},{0xFFFF, 0xABCD}};


    CONST_T uint16 chipData1[][2] ={{0x1B24, 0x0000},{0x1B25, 0x0000},{0x1B26, 0x0000},{0x1B27, 0x0000},
                                    {0x207F, 0x0007},{0x207E, 0x000B},{0x2076, 0x1A00},{0x207E, 0x000A},
                                    {0x2078, 0x1D22},{0x207F, 0x0000},{0x205F, 0x0007},{0x205E, 0x000A},
                                    {0x2059, 0x0000},{0x205A, 0x0000},{0x205B, 0x0000},{0x205C, 0x0000},
                                    {0x205E, 0x000B},{0x2055, 0x0500},{0x2056, 0x0000},{0x2057, 0x0000},
                                    {0x2058, 0x0000},{0x205F, 0x0000},{0x1362, 0x0115},{0x1363, 0x0002},
                                    {0x1363, 0x0000},{0x133F, 0x0030},{0x133E, 0x000E},{0x221F, 0x0007},
                                    {0x221E, 0x0040},{0x2218, 0x0000},{0x221F, 0x0007},{0x221E, 0x002C},
                                    {0x2218, 0x008B},{0x221F, 0x0005},{0x2205, 0x8B6E},{0x2206, 0x0000},
                                    {0x220F, 0x0100},{0x2205, 0xFFF6},{0x2206, 0x0080},{0x2205, 0x8000},
                                    {0x2206, 0x0280},{0x2206, 0x1EF7},{0x2206, 0x00E0},{0x2206, 0xFFF7},
                                    {0x2206, 0xA080},{0x2206, 0x02AE},{0x2206, 0xF602},{0x2206, 0x8046},
                                    {0x2206, 0x0201},{0x2206, 0x5002},{0x2206, 0x0163},{0x2206, 0x0280},
                                    {0x2206, 0xCD02},{0x2206, 0x0179},{0x2206, 0xAEE7},{0x2206, 0xBF80},
                                    {0x2206, 0x61D7},{0x2206, 0x8580},{0x2206, 0xD06C},{0x2206, 0x0229},
                                    {0x2206, 0x71EE},{0x2206, 0x8B64},{0x2206, 0x00EE},{0x2206, 0x8570},
                                    {0x2206, 0x00EE},{0x2206, 0x8571},{0x2206, 0x00EE},{0x2206, 0x8AFC},
                                    {0x2206, 0x07EE},{0x2206, 0x8AFD},{0x2206, 0x73EE},{0x2206, 0xFFF6},
                                    {0x2206, 0x00EE},{0x2206, 0xFFF7},{0x2206, 0xFC04},{0x2206, 0xBF85},
                                    {0x2206, 0x80D0},{0x2206, 0x6C02},{0x2206, 0x2978},{0x2206, 0xE0E0},
                                    {0x2206, 0xE4E1},{0x2206, 0xE0E5},{0x2206, 0x5806},{0x2206, 0x68C0},
                                    {0x2206, 0xD1D2},{0x2206, 0xE4E0},{0x2206, 0xE4E5},{0x2206, 0xE0E5},
                                    {0x2206, 0x0425},{0x2206, 0x0807},{0x2206, 0x2640},{0x2206, 0x7227},
                                    {0x2206, 0x267E},{0x2206, 0x2804},{0x2206, 0xB729},{0x2206, 0x2576},
                                    {0x2206, 0x2A68},{0x2206, 0xE52B},{0x2206, 0xAD00},{0x2206, 0x2CDB},
                                    {0x2206, 0xF02D},{0x2206, 0x67BB},{0x2206, 0x2E7B},{0x2206, 0x0F2F},
                                    {0x2206, 0x7365},{0x2206, 0x31AC},{0x2206, 0xCC32},{0x2206, 0x2300},
                                    {0x2206, 0x332D},{0x2206, 0x1734},{0x2206, 0x7F52},{0x2206, 0x3510},
                                    {0x2206, 0x0036},{0x2206, 0x1000},{0x2206, 0x3710},{0x2206, 0x0038},
                                    {0x2206, 0x7FCE},{0x2206, 0x3CE5},{0x2206, 0xF73D},{0x2206, 0x3DA4},
                                    {0x2206, 0x6530},{0x2206, 0x3E67},{0x2206, 0x0053},{0x2206, 0x69D2},
                                    {0x2206, 0x0F6A},{0x2206, 0x012C},{0x2206, 0x6C2B},{0x2206, 0x136E},
                                    {0x2206, 0xE100},{0x2206, 0x6F12},{0x2206, 0xF771},{0x2206, 0x006B},
                                    {0x2206, 0x7306},{0x2206, 0xEB74},{0x2206, 0x94C7},{0x2206, 0x7698},
                                    {0x2206, 0x0A77},{0x2206, 0x5000},{0x2206, 0x788A},{0x2206, 0x1579},
                                    {0x2206, 0x7F6F},{0x2206, 0x7A06},{0x2206, 0xA6F8},{0x2206, 0xE08B},
                                    {0x2206, 0x8EAD},{0x2206, 0x2006},{0x2206, 0x0280},{0x2206, 0xDC02},
                                    {0x2206, 0x8109},{0x2206, 0xFC04},{0x2206, 0xF8F9},{0x2206, 0xE08B},
                                    {0x2206, 0x87AD},{0x2206, 0x2022},{0x2206, 0xE0E2},{0x2206, 0x00E1},
                                    {0x2206, 0xE201},{0x2206, 0xAD20},{0x2206, 0x11E2},{0x2206, 0xE022},
                                    {0x2206, 0xE3E0},{0x2206, 0x23AD},{0x2206, 0x3908},{0x2206, 0x5AC0},
                                    {0x2206, 0x9F04},{0x2206, 0xF724},{0x2206, 0xAE02},{0x2206, 0xF624},
                                    {0x2206, 0xE4E2},{0x2206, 0x00E5},{0x2206, 0xE201},{0x2206, 0xFDFC},
                                    {0x2206, 0x04F8},{0x2206, 0xF9E0},{0x2206, 0x8B85},{0x2206, 0xAD25},
                                    {0x2206, 0x48E0},{0x2206, 0x8AD2},{0x2206, 0xE18A},{0x2206, 0xD37C},
                                    {0x2206, 0x0000},{0x2206, 0x9E35},{0x2206, 0xEE8A},{0x2206, 0xD200},
                                    {0x2206, 0xEE8A},{0x2206, 0xD300},{0x2206, 0xE08A},{0x2206, 0xFCE1},
                                    {0x2206, 0x8AFD},{0x2206, 0xE285},{0x2206, 0x70E3},{0x2206, 0x8571},
                                    {0x2206, 0x0229},{0x2206, 0x3AAD},{0x2206, 0x2012},{0x2206, 0xEE8A},
                                    {0x2206, 0xD203},{0x2206, 0xEE8A},{0x2206, 0xD3B7},{0x2206, 0xEE85},
                                    {0x2206, 0x7000},{0x2206, 0xEE85},{0x2206, 0x7100},{0x2206, 0xAE11},
                                    {0x2206, 0x15E6},{0x2206, 0x8570},{0x2206, 0xE785},{0x2206, 0x71AE},
                                    {0x2206, 0x08EE},{0x2206, 0x8570},{0x2206, 0x00EE},{0x2206, 0x8571},
                                    {0x2206, 0x00FD},{0x2206, 0xFC04},{0x2206, 0xCCE2},{0x2206, 0x0000},
                                    {0x2205, 0xE142},{0x2206, 0x0701},{0x2205, 0xE140},{0x2206, 0x0405},
                                    {0x220F, 0x0000},{0x221F, 0x0000},{0x221F, 0x0005},{0x2205, 0x85E4},
                                    {0x2206, 0x8A14},{0x2205, 0x85E7},{0x2206, 0x7F6E},{0x221F, 0x0007},
                                    {0x221E, 0x002D},{0x2218, 0xF030},{0x221E, 0x0023},{0x2216, 0x0005},
                                    {0x2215, 0x005C},{0x2219, 0x0068},{0x2215, 0x0082},{0x2219, 0x000A},
                                    {0x2215, 0x00A1},{0x2219, 0x0081},{0x2215, 0x00AF},{0x2219, 0x0080},
                                    {0x2215, 0x00D4},{0x2219, 0x0000},{0x2215, 0x00E4},{0x2219, 0x0081},
                                    {0x2215, 0x00E7},{0x2219, 0x0080},{0x2215, 0x010D},{0x2219, 0x0083},
                                    {0x2215, 0x0118},{0x2219, 0x0083},{0x2215, 0x0120},{0x2219, 0x0082},
                                    {0x2215, 0x019C},{0x2219, 0x0081},{0x2215, 0x01A4},{0x2219, 0x0080},
                                    {0x2215, 0x01CD},{0x2219, 0x0000},{0x2215, 0x01DD},{0x2219, 0x0081},
                                    {0x2215, 0x01E0},{0x2219, 0x0080},{0x2215, 0x0147},{0x2219, 0x0096},
                                    {0x2216, 0x0000},{0x221E, 0x002D},{0x2218, 0xF010},{0x221F, 0x0005},
                                    {0x2205, 0x8B84},{0x2206, 0x0062},{0x221F, 0x0000},{0x220D, 0x0003},
                                    {0x220E, 0x0015},{0x220D, 0x4003},{0x220E, 0x0006},{0x133F, 0x0010},
                                    {0x133E, 0x0FFE},{0x12A4, 0x380A},{0x1303, 0x0367},{0x1304, 0x7777},
                                    {0x1203, 0xFF00},{0x1200, 0x7FC4},{0x0865, 0x3210},{0x087B, 0x0000},
                                    {0x087C, 0xFF00},{0x087D, 0x0000},{0x087E, 0x0000},{0x0801, 0x0100},
                                    {0x0802, 0x0100},{0x0A20, 0x2040},{0x0A21, 0x2040},{0x0A22, 0x2040},
                                    {0x0A23, 0x2040},{0x0A24, 0x2040},{0x0A25, 0x2040},{0x0A26, 0x2040},
                                    {0x0A27, 0x2040},{0x0A28, 0x2040},{0x0A29, 0x2040},{0x1B03, 0x0876},
                                    {0xFFFF, 0xABCD}};


    if ((retVal = rtl8370_setAsicReg(0x13C2,0x0249))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicRegBits(0x1302, 0x7,&regData))!=RT_ERR_OK)
        return retVal;   

#ifdef MDC_MDIO_OPERATION  
    index = 0;
    switch(regData)
    {
        case 0x0000:
            while (chipData0[index][0] != 0xFFFF && chipData0[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData0[index][0],(uint32)chipData0[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            } 
            break;
        case 0x0001:    
            while (chipData1[index][0] != 0xFFFF && chipData1[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData1[index][0],(uint32)chipData1[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            }            
            break;
        case 0x0002:    
            while (chipData1[index][0] != 0xFFFF && chipData1[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData1[index][0],(uint32)chipData1[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            }            
            break;
        default:
            return RT_ERR_FAILED;
    }
#else 
    index = 0;
    switch(regData)
    {
        
        case 0x0000:
            while (chipData0[index][0] != 0xFFFF && chipData0[index][1] != 0xABCD)
            {    
                if ((chipData0[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
                    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32)chipData0[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32)chipData0[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData0[index][0],(uint32)chipData0[index][1]))
                        return RT_ERR_FAILED;
                }
                index ++;    
            } 
            break;
        case 0x0001:
            while (chipData1[index][0] != 0xFFFF && chipData1[index][1] != 0xABCD)
            {    
                if ((chipData1[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
        
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32)chipData1[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32)chipData1[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData1[index][0],(uint32)chipData1[index][1]))
                        return RT_ERR_FAILED;
                } 
                index ++;    
            }            
            break;
        case 0x0002:
            while (chipData1[index][0] != 0xFFFF && chipData1[index][1] != 0xABCD)
            {    
                if ((chipData1[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
        
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32)chipData1[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32)chipData1[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData1[index][0],(uint32)chipData1[index][1]))
                        return RT_ERR_FAILED;
                } 
                index ++;    
            }            
            break;
        default:
            return RT_ERR_FAILED;
    } 
#endif /*End of #ifdef MDC_MDIO_OPERATION*/

    return RT_ERR_OK;
}




/* Function Name:
 *      rtk_switch_init 
 * Description:
 *      Set chip to default configuration enviroment
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      The API can set chip registers to default configuration for different release chip model.
 */


rtk_api_ret_t rtk_switch_init(void)
{

    rtk_api_ret_t retVal;
    uint32 regData1,regData2;

    if ((retVal = rtl8370_setAsicReg(0x13C2,0x0249))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicRegBits(0x1301, 0xF000,&regData1))!=RT_ERR_OK)
        return retVal; 

    if ((retVal = rtl8370_setAsicPHYReg(0,31,5))!=RT_ERR_OK)
        return retVal; 
    if ((retVal = rtl8370_setAsicPHYReg(0,5,0x3ffe))!=RT_ERR_OK)
        return retVal; 
    if ((retVal = rtl8370_getAsicPHYReg(0,6,&regData2))!=RT_ERR_OK)
        return retVal; 

    if (0 == regData1)
    {
#if !defined(_REDUCE_CODE)
        if ((retVal = _rtk_switch_init0()) != RT_ERR_OK)
            return retVal;
#else
        return RT_ERR_CHIP_NOT_SUPPORTED;
#endif
    }
    else if (1 == regData1)
    {
        if (0x94eb == regData2)
        {
            if ((retVal = _rtk_switch_init1()) != RT_ERR_OK)
                return retVal;
        }
        else if (0x2104 == regData2)
        {
            if ((retVal = _rtk_switch_init2()) != RT_ERR_OK)
                return retVal;
        }
    }


    /*Enable System Based LED*/
    if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_SYS_CONFIG, RTL8370_LED_IO_DISABLE_OFFSET, 0))!=RT_ERR_OK)
        return retVal;  

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_maxPktLen_set
 * Description:
 *      Set the max packet length of the specific unit
 * Input:
 *      len - max packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can set max packet length of the specific unit to 
 *      MAXPKTLEN_1522B,
 *      MAXPKTLEN_1536B,
 *      MAXPKTLEN_1552B,
 *      MAXPKTLEN_16000B.
 */
rtk_api_ret_t rtk_switch_maxPktLen_set(rtk_switch_maxPktLen_t len)
{
    rtk_api_ret_t retVal;

    if (len>=MAXPKTLEN_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_setAsicMaxLengthInRx(len))!=RT_ERR_OK)
        return retVal;    


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_switch_maxPktLen_get
 * Description:
 *      Get the max packet length of the specific unit
 * Input:
 *      None
 * Output:
 *      pLen - pointer to the max packet length
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      The API can set max packet length of the specific unit.
 */
rtk_api_ret_t rtk_switch_maxPktLen_get(rtk_data_t *pLen)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicMaxLengthInRx(pLen))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_switch_greenEthernet_set
 * Description:
 *      Set all PHY enable status.
 * Input:
 *      enable - Back pressure status.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set all PHY status.
 *      The configuration of all PHY is as following:
 *      DISABLE   
 *      ENABLE
 */
rtk_api_ret_t rtk_switch_greenEthernet_set(rtk_enable_t enable, rtk_enable_t set_phy_psm)
{
    rtk_api_ret_t retVal;
    uint32 phy, regData;
    uint32 index;
#ifndef MDC_MDIO_OPERATION 
    uint32 busyFlag,cnt;
#endif
    CONST_T uint32 Para0En[][2] = {{0x133f,0x0030},{0x133e,0x000e},{0x221F,0x0005},{0x2205,0x80BD},
                                   {0x2206,0x8A15},{0x2205,0x80C0},{0x2206,0x7F6F},{0x221F,0x0000},
                                   {0x133f,0x0010},{0x133e,0x0ffe},{0xFFFF, 0xABCD}};

    CONST_T uint32 Para0Dis[][2] = {{0x133f,0x0030},{0x133e,0x000e},{0x221F,0x0005},{0x2205,0x80BD},
                                   {0x2206,0x8A14},{0x2205,0x80C0},{0x2206,0x7F6E},{0x221F,0x0000},
                                   {0x133f,0x0010},{0x133e,0x0ffe},{0xFFFF, 0xABCD}};

    CONST_T uint32 Para1En[][2] = {{0x133f,0x0030},{0x133e,0x000e},{0x221F,0x0005},{0x2201,0x0701},
                                   {0x2205,0x85E4},{0x2206,0x8A15},{0x2205,0x85E7},{0x2206,0x7F6F},
                                   {0x221F,0x0000},{0x133f,0x0010},{0x133e,0x0ffe},{0xFFFF,0xABCD}};

    CONST_T uint32 Para1Dis[][2] = {{0x133f,0x0030},{0x133e,0x000e},{0x221F,0x0005},{0x2201,0x0601},
                                    {0x2205,0x85E4},{0x2206,0x8A14},{0x2205,0x85E7},{0x2206,0x7F6E},
                                    {0x221F,0x0000},{0x133f,0x0010},{0x133e,0x0ffe},{0xFFFF,0xABCD}};
 


    if (enable >=RTK_ENABLE_END)
        return RT_ERR_ENABLE;    

    if ((retVal = rtl8370_setAsicPHYReg(0,PHY_PAGE_ADDRESS,5))!=RT_ERR_OK)
        return retVal; 
    if ((retVal = rtl8370_setAsicPHYReg(0,5,0x3ffe))!=RT_ERR_OK)
        return retVal; 
    if ((retVal = rtl8370_getAsicPHYReg(0,6,&regData))!=RT_ERR_OK)
        return retVal; 


#ifdef MDC_MDIO_OPERATION 
    if (ENABLED == enable)
    {
        if (regData == 0x94eb)
        {
            index = 0;
            while (Para0En[index][0] != 0xFFFF && Para0En[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg(Para0En[index][0],Para0En[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            }   
        }
        else if (regData == 0x2104)    
        {
            index = 0;
            while (Para1En[index][0] != 0xFFFF && Para1En[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg(Para1En[index][0],Para1En[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            } 
        }
    }
    else if (DISABLED == enable)
    {
        if (regData == 0x94eb)
        {
            index = 0;
            while (Para0Dis[index][0] != 0xFFFF && Para0Dis[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg(Para0Dis[index][0],Para0Dis[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            }   
        }
        else if (regData == 0x2104)    
        {
            index = 0;
            while (Para1Dis[index][0] != 0xFFFF && Para1Dis[index][1] != 0xABCD)
            {    
                if (RT_ERR_OK != rtl8370_setAsicReg(Para1Dis[index][0],Para1Dis[index][1]))
                    return RT_ERR_FAILED;
                index ++;    
            } 
        }
    }
    else
        return RT_ERR_FAILED;

#else 

    if (ENABLED == enable)
    {
        if (regData == 0x94eb)
        {
            index = 0;
            while (Para0En[index][0] != 0xFFFF && Para0En[index][1] != 0xABCD)
            {    
                if ((Para0En[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
                
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, Para0En[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, Para0En[index][0])) !=  RT_ERR_OK)
                        return retVal; 
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal;
                }
                else
                {
                    if ((retVal = rtl8370_setAsicReg(Para0En[index][0],Para0En[index][1])) !=  RT_ERR_OK)
                        return retVal;
                }
                index ++; 
            }         
        }
        else if (regData == 0x2104)    
        {
            index = 0;
            while (Para1En[index][0] != 0xFFFF && Para1En[index][1] != 0xABCD)
            {    
                if ((Para1En[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
                
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, Para1En[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, Para1En[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg(Para1En[index][0],Para1En[index][1]))
                        return RT_ERR_FAILED;
                }
                index ++;    
            } 
        }
    }
    else if (DISABLED == enable)
    {
        if (regData == 0x94eb)
        {
            index = 0;
            while (Para0Dis[index][0] != 0xFFFF && Para0Dis[index][1] != 0xABCD)
            {    
                if ((Para0Dis[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
                
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, Para0Dis[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, Para0Dis[index][0])) !=  RT_ERR_OK)
                        return retVal; 
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal;
                }
                else
                {
                    if ((retVal = rtl8370_setAsicReg(Para0Dis[index][0],Para0Dis[index][1])) !=  RT_ERR_OK)
                        return retVal;
                }
                index ++; 
            }         
        }
        else if (regData == 0x2104)    
        {
            index = 0;
            while (Para1Dis[index][0] != 0xFFFF && Para1Dis[index][1] != 0xABCD)
            {    
                if ((Para1Dis[index][0]&0xF000)==0x2000)
                {
                    cnt = 0;
                    busyFlag = 1;
                    while (busyFlag&&cnt<5)
                    {
                        cnt++;
                        if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                            return retVal;
                    }
                    if (5 == cnt)
                        return RT_ERR_BUSYWAIT_TIMEOUT;
                
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, Para1Dis[index][1])) !=  RT_ERR_OK)
                        return retVal;
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, Para1Dis[index][0])) !=  RT_ERR_OK)
                        return retVal;    
                    if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                        return retVal; 
                }
                else
                {
                    if (RT_ERR_OK != rtl8370_setAsicReg(Para1Dis[index][0],Para1Dis[index][1]))
                        return RT_ERR_FAILED;
                }
                index ++;    
            } 
        }
    }
    else
        return RT_ERR_FAILED;
    
#endif /*End of #ifdef MDC_MDIO_OPERATION*/

    if (set_phy_psm == DISABLED)
        return RT_ERR_OK;

    for (phy=0;phy<=RTK_PHY_ID_MAX;phy++)
    {
        if ((retVal = rtl8370_setAsicPowerSaving(phy,enable))!=RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;

}


/* Function Name:
 *      rtk_switch_greenEthernet_get
 * Description:
 *      Get all PHY enable status.
 * Input:
 *      None
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API can set all PHY status.
 *      The configuration of all PHY is as following:
 *      DISABLE   
 *      ENABLE
 */
rtk_api_ret_t rtk_switch_greenEthernet_get(rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_data_t value1, value2;
    uint32 phy;   

    for (phy=0;phy<=RTK_PHY_ID_MAX;phy++)
    {

        if ((retVal = rtl8370_setAsicPHYReg(phy,PHY_PAGE_ADDRESS,5))!=RT_ERR_OK)
            return retVal; 
        if ((retVal = rtl8370_setAsicPHYReg(phy,5,0x85E4))!=RT_ERR_OK)
            return retVal; 
        if ((retVal = rtl8370_getAsicPHYReg(phy,6,&value1))!=RT_ERR_OK)
            return retVal; 
        if ((retVal = rtl8370_setAsicPHYReg(phy,5,0x85E7))!=RT_ERR_OK)
            return retVal; 
        if ((retVal = rtl8370_getAsicPHYReg(phy,6,&value2))!=RT_ERR_OK)
            return retVal; 
        
        if (((value1&0x1)!=1) && ((value2&0x1)!=1)) 
        {
            *pEnable = DISABLED;
            return RT_ERR_OK;
        }
    }

    *pEnable = ENABLED;
    
    return RT_ERR_OK;
}

#if !defined(_REDUCE_CODE)

/* Function Name:
 *      rtk_mirror_portBased_set
 * Description:
 *      Set port mirror function.
 * Input:
 *      mirroring_port - Monitor port.
 *      pMirrored_rx_portmask - Rx mirror port mask.
 *      pMirrored_tx_portmask - Tx mirror port mask. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_PORT_MASK - Invalid portmask.
 * Note:
 *      The API is to set mirror function of source port and mirror port.
 *      The mirror port can only be set to one port and the TX and RX mirror ports
 *      should be identical.
 */
rtk_api_ret_t rtk_mirror_portBased_set(rtk_port_t mirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask)
{
    rtk_api_ret_t retVal;
    rtk_enable_t mirRx, mirTx;
    uint32 i;
      rtk_port_t source_port;

    if (mirroring_port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;     

    if (pMirrored_rx_portmask->bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK; 

    if (pMirrored_tx_portmask->bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK;

    /*Only one port for tx & rx mirror*/
    if (pMirrored_tx_portmask->bits[0]!=pMirrored_rx_portmask->bits[0]&&pMirrored_tx_portmask->bits[0]!=0&&pMirrored_rx_portmask->bits[0]!=0)
        return RT_ERR_PORT_MASK;

     /*mirror port != source port*/
    if ((pMirrored_tx_portmask->bits[0]&(1<<mirroring_port))>0||(pMirrored_rx_portmask->bits[0]&(1<<mirroring_port))>0)
        return RT_ERR_PORT_MASK;    

   source_port = 0;

   for (i=0;i< RTK_MAX_NUM_OF_PORT;i++)
   {
        if (pMirrored_tx_portmask->bits[0]&(1<<i))
        {
            source_port = i;
            break;
        }

        if (pMirrored_rx_portmask->bits[0]&(1<<i))
        {
            source_port = i;
            break;
        }
    }
    
    /*Only one port for tx & rx mirror*/
    if (pMirrored_tx_portmask->bits[0]>>(source_port+1))
        return RT_ERR_PORT_MASK;

    if (pMirrored_rx_portmask->bits[0]>>(source_port+1))
        return RT_ERR_PORT_MASK;    

    if ((retVal = rtl8370_setAsicPortMirror(source_port,mirroring_port))!=RT_ERR_OK)
        return retVal;  

    if (pMirrored_rx_portmask->bits[0])
        mirRx = ENABLED;
    else
        mirRx = DISABLED;

    if ((retVal = rtl8370_setAsicPortMirrorRxFunction(mirRx))!=RT_ERR_OK)
        return retVal;        

    if (pMirrored_tx_portmask->bits[0])
        mirTx = ENABLED;
    else
        mirTx = DISABLED;

    if ((retVal = rtl8370_setAsicPortMirrorTxFunction(mirTx))!=RT_ERR_OK)
        return retVal;        

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_mirror_portBased_get
 * Description:
 *      Get port mirror function.
 * Input:
 *      None
 * Output:
 *      pMirroring_port - Monitor port.
 *      pMirrored_rx_portmask - Rx mirror port mask.
 *      pMirrored_tx_portmask - Tx mirror port mask.  
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API is to get mirror function of source port and mirror port.
 */
rtk_api_ret_t rtk_mirror_portBased_get(rtk_port_t* pMirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask)
{
    rtk_api_ret_t retVal;
    rtk_port_t source_port;
    rtk_enable_t mirRx, mirTx;
    
    if ((retVal = rtl8370_getAsicPortMirror(&source_port,pMirroring_port))!=RT_ERR_OK)
        return retVal;     

    if ((retVal = rtl8370_getAsicPortMirrorRxFunction((uint32*)&mirRx))!=RT_ERR_OK)
        return retVal;        

    if ((retVal = rtl8370_getAsicPortMirrorTxFunction((uint32*)&mirTx))!=RT_ERR_OK)
        return retVal; 

    if (DISABLED == mirRx)
        pMirrored_rx_portmask->bits[0]=0;
    else
        pMirrored_rx_portmask->bits[0]=1<<source_port;

     if (DISABLED == mirTx)
        pMirrored_tx_portmask->bits[0]=0;
    else
        pMirrored_tx_portmask->bits[0]=1<<source_port;    

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_mirror_portIso_set
 * Description:
 *      Set mirror port isolation.
 * Input:
 *      enable |Mirror isolation status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      The API is to set mirror isolation function that prevent normal forwarding packets to miror port.
 */
rtk_api_ret_t rtk_mirror_portIso_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;
    
    if ((retVal = rtl8370_setAsicPortMirrorIsolation(enable))!=RT_ERR_OK)
        return retVal;  

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_mirror_portIso_get
 * Description:
 *      Get mirror port isolation.
 * Input:
 *      None
 * Output:
 *      pEnable |Mirror isolation status. 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API is to get mirror isolation status.
 */
rtk_api_ret_t rtk_mirror_portIso_get(rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicPortMirrorIsolation(pEnable))!=RT_ERR_OK)
        return retVal;     
    
    return RT_ERR_OK;
}

#endif

/* Function Name:
 *      rtk_stat_global_reset
 * Description:
 *      Reset global MIB counter.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
rtk_api_ret_t rtk_stat_global_reset(void)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_setAsicMIBsCounterReset(TRUE,FALSE,0))!=RT_ERR_OK)
        return retVal; 
        
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_stat_port_reset
 * Description:
 *      Reset per port MIB counter by port.
 * Input:
 *      port - port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */

rtk_api_ret_t rtk_stat_port_reset(rtk_port_t port)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 
    
    if ((retVal = rtl8370_setAsicMIBsCounterReset(FALSE,FALSE,1<<port))!=RT_ERR_OK)
        return retVal; 
        
    return RT_ERR_OK;
}

#ifdef EMBEDDED_SUPPORT

rtk_api_ret_t rtk_stat_global_get(rtk_stat_global_type_t cntr_idx, rtk_stat_counter_t *pCntrH, rtk_stat_counter_t *pCntrL)
{
    rtk_api_ret_t retVal;

    if (cntr_idx!=DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX)
            return RT_ERR_STAT_INVALID_GLOBAL_CNTR;

    if ((retVal = rtl8370_getAsicMIBsCounter(0,cntr_idx,pCntrH, pCntrL))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

rtk_api_ret_t rtk_stat_port_get(rtk_port_t port, rtk_stat_port_type_t cntr_idx, rtk_stat_counter_t *pCntrH, rtk_stat_counter_t *pCntrL)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 
    
    if (cntr_idx>=STAT_PORT_CNTR_END)
        return RT_ERR_STAT_INVALID_PORT_CNTR;

    if ((retVal = rtl8370_getAsicMIBsCounter(port,cntr_idx,pCntrH, pCntrL))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

#else

/* Function Name:
 *      rtk_stat_global_get
 * Description:
 *      Get global MIB counter
 * Input:
 *      cntr_idx - global counter index.
 * Output:
 *      pCntr - global counter value.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      Get global MIB counter by index definition. 
 */
rtk_api_ret_t rtk_stat_global_get(rtk_stat_global_type_t cntr_idx, rtk_stat_counter_t *pCntr)
{
    rtk_api_ret_t retVal;

    if (cntr_idx!=DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX)
        return RT_ERR_STAT_INVALID_GLOBAL_CNTR;

    if ((retVal = rtl8370_getAsicMIBsCounter(0,cntr_idx,pCntr))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_stat_global_getAll
 * Description:
 *      Get all global MIB counter
 * Input:
 *      None
 * Output:
 *      pGlobal_cntrs - global counter structure.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      Get all global MIB counter by index definition.  
 */
rtk_api_ret_t rtk_stat_global_getAll(rtk_stat_global_cntr_t *pGlobal_cntrs)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicMIBsCounter(0,DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX,&pGlobal_cntrs->dot1dTpLearnedEntryDiscards))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_stat_port_get
 * Description:
 *      Get per port MIB counter by index
 * Input:
 *      port - port id.
 *      cntr_idx - port counter index.
 * Output:
 *      pCntr - MIB retrived counter.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      Get per port MIB counter by index definition. 
 */
rtk_api_ret_t rtk_stat_port_get(rtk_port_t port, rtk_stat_port_type_t cntr_idx, rtk_stat_counter_t *pCntr)
{
    rtk_api_ret_t retVal;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 
    
    if (cntr_idx>=STAT_PORT_CNTR_END)
        return RT_ERR_STAT_INVALID_PORT_CNTR;

    if ((retVal = rtl8370_getAsicMIBsCounter(port,cntr_idx,pCntr))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_stat_port_getAll
 * Description:
 *      Get all counters of one specified port in the specified device.
 * Input:
 *      port - port id.
 * Output:
 *      pPort_cntrs - buffer pointer of counter value.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      Get all MIB counters of one port.
 */
rtk_api_ret_t rtk_stat_port_getAll(rtk_port_t port, rtk_stat_port_cntr_t *pPort_cntrs)
{
    rtk_api_ret_t retVal;
    uint32 mibIndex;
    uint64 mibCounter;
    uint32 *accessPtr;
    /* address offset to MIBs counter */
    CONST_T uint16 mibLength[STAT_PORT_CNTR_END]= {
        2,1,1,1,1,1,1,1,1,
        2,1,1,1,1,1,1,1,1,1,1,
        2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID; 

    accessPtr = (uint32*)pPort_cntrs;    
    for (mibIndex=0;mibIndex<STAT_PORT_CNTR_END;mibIndex++)
    {
        if ((retVal = rtl8370_getAsicMIBsCounter(port,mibIndex,&mibCounter))!=RT_ERR_OK)        
            return retVal;

        if (2 == mibLength[mibIndex])
            *(uint64*)accessPtr = mibCounter;
        else if (1 == mibLength[mibIndex])
            *accessPtr = mibCounter;
        else 
            return RT_ERR_FAILED;
        
        accessPtr+=mibLength[mibIndex];
    }

    return RT_ERR_OK;
}

#endif

/* Function Name:
 *      rtk_int_polarity_set
 * Description:
 *      Set interrupt polarity configuration.
 * Input:
 *      type - Interruptpolarity type.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can set interrupt polarity configuration.
 */
rtk_api_ret_t rtk_int_polarity_set(rtk_int_polarity_t type)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_setAsicInterruptPolarity(type))!=RT_ERR_OK)        
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_int_polarity_get
 * Description:
 *      Get interrupt polarity configuration.
 * Input:
 *      None
 * Output:
 *      pType - Interruptpolarity type.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      The API can get interrupt polarity configuration.
 */
rtk_api_ret_t rtk_int_polarity_get(rtk_data_t *pType)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicInterruptPolarity(pType))!=RT_ERR_OK)        
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_int_control_set
 * Description:
 *      Set interrupt trigger status configuration.
 * Input:
 *      type - Interrupt type.
 *      enable - Interrupt status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      The API can set interrupt status configuration.
 *      The interrupt trigger status is shown in the following:
 *      INT_TYPE_LINK_STATUS
 *      INT_TYPE_METER_EXCEED
 *      INT_TYPE_LEARN_LIMIT 
 *      INT_TYPE_LINK_SPEED
 *      INT_TYPE_CONGEST
 *      INT_TYPE_GREEN_FEATURE
 *      INT_TYPE_LOOP_DETECT
 */
rtk_api_ret_t rtk_int_control_set(rtk_int_type_t type, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    uint32 mask;

    if (type>=INT_TYPE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_getAsicInterruptMask(&mask))!=RT_ERR_OK)        
        return retVal;

    if (ENABLED == enable)
        mask = mask | (1<<type);
    else if (DISABLED == enable)
        mask = mask & ~(1<<type);
    else
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_setAsicInterruptMask(mask))!=RT_ERR_OK)        
        return retVal;
    
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_int_control_get
 * Description:
 *      Get interrupt trigger status configuration.
 * Input:
 *      type - Interrupt type.
 * Output:
 *      pEnable - Interrupt status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get interrupt status configuration.
 *      The interrupt trigger status is shown in the following:
 *      INT_TYPE_LINK_STATUS
 *      INT_TYPE_METER_EXCEED
 *      INT_TYPE_LEARN_LIMIT 
 *      INT_TYPE_LINK_SPEED
 *      INT_TYPE_CONGEST
 *      INT_TYPE_GREEN_FEATURE
 *      INT_TYPE_LOOP_DETECT
 */
rtk_api_ret_t rtk_int_control_get(rtk_int_type_t type, rtk_data_t* pEnable)
{
    rtk_api_ret_t retVal;
    uint32 mask;

    if ((retVal = rtl8370_getAsicInterruptMask(&mask))!=RT_ERR_OK)        
        return retVal;

    if (0 == (mask&(1<<type)))
        *pEnable=DISABLED;
    else
        *pEnable=ENABLED;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtk_int_status_set
 * Description:
 *      Set interrupt trigger status to clean.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt status bit mask.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can clean interrupt trigger status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      INT_TYPE_LINK_STATUS    (Bit0)
 *      INT_TYPE_METER_EXCEED   (Bit1)
 *      INT_TYPE_LEARN_LIMIT    (Bit2)
 *      INT_TYPE_LINK_SPEED     (Bit3)
 *      INT_TYPE_CONGEST        (Bit4)
 *      INT_TYPE_GREEN_FEATURE  (Bit5)
 *      INT_TYPE_LOOP_DETECT    (Bit6)
 */
rtk_api_ret_t rtk_int_status_set(rtk_int_status_t statusMask)
{
    rtk_api_ret_t retVal;
    
    if ((retVal = rtl8370_setAsicInterruptStatus((uint32)statusMask.value[0]))!=RT_ERR_OK)        
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_int_status_get
 * Description:
 *      Get interrupt trigger status.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt status bit mask.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get interrupt trigger status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      INT_TYPE_LINK_STATUS    (Bit0)
 *      INT_TYPE_METER_EXCEED   (Bit1)
 *      INT_TYPE_LEARN_LIMIT    (Bit2)
 *      INT_TYPE_LINK_SPEED     (Bit3)
 *      INT_TYPE_CONGEST        (Bit4)
 *      INT_TYPE_GREEN_FEATURE  (Bit5)
 *      INT_TYPE_LOOP_DETECT    (Bit6)
 */
rtk_api_ret_t rtk_int_status_get(rtk_int_status_t* pStatusMask)
{
    rtk_api_ret_t retVal;
    
    if ((retVal = rtl8370_getAsicInterruptStatus((uint32*)pStatusMask))!=RT_ERR_OK)        
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_int_advanceInfo_get
 * Description:
 *      Get interrupt advanced information.
 * Input:
 *      adv_type - Advanced interrupt type.
 * Output:
 *      info - Information per type.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can get advanced information when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      INT_LINK_CHANGE
 *      INT_SPEED_CHANGE
 *      INT_RLDP
 *      INT_METER
 *      INT_LEARN_LIMIT
 *      The status will be cleared after execute this API.
 */
 rtk_api_ret_t rtk_int_advanceInfo_get(rtk_int_advType_t adv_type, rtk_int_info_t* info)
{
    rtk_api_ret_t retVal;

    if (adv_type>=ADV_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_getAsicInterruptRelatedStatus(adv_type,info))!=RT_ERR_OK)        
        return retVal;    

    if ((retVal = rtl8370_setAsicInterruptRelatedStatus(adv_type,0xFFFF))!=RT_ERR_OK)        
        return retVal;    
      
    return retVal;
}

/* Function Name:
 *      rtk_led_enable_set
 * Description:
 *      Set Led parallel mode enable congiuration
 * Input:
 *      group - LED group id.
 *      portmask - LED enable port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can be used to enable LED parallel mode per port per group. 
 */

rtk_api_ret_t rtk_led_enable_set(rtk_led_group_t group, rtk_portmask_t portmask)
{
    rtk_api_ret_t retVal;

    if (group >= LED_GROUP_END)
        return RT_ERR_INPUT;

    if (portmask.bits[0] >= (1 << (RTK_PHY_ID_MAX + 1)))
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_setAsicLedGroupEnable(group, portmask.bits[0]))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_led_enable_get
 * Description:
 *      Get Led parallel mode enable congiuration
 * Input:
 *      group - LED group id.
 * Output:
 *      pPortmask - LED enable port mask.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can be used to get LED parallel mode enable status. 
 */
rtk_api_ret_t rtk_led_enable_get(rtk_led_group_t group, rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;

    if (group >= LED_GROUP_END)
        return RT_ERR_INPUT;  

    if ((retVal = rtl8370_getAsicLedGroupEnable(group, &(pPortmask->bits[0])))!=RT_ERR_OK)        
        return retVal;
    
    return RT_ERR_OK;

}



/* Function Name:
 *      rtk_led_operation_set
 * Description:
 *      Set Led operation mode
 * Input:
 *      mode - LED operation mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can set Led operation mode.
 *      The modes that can be set are as following:
 *      LED_OP_SCAN,        
 *      LED_OP_PARALLEL,        
 *      LED_OP_SERIAL, 
 */
rtk_api_ret_t rtk_led_operation_set(rtk_led_operation_t mode)
{
    rtk_api_ret_t retVal;
    uint32 regData;
    
    if ( mode >= LED_OP_END)
      return RT_ERR_INPUT; 

    switch (mode)
    {
        case LED_OP_SCAN:
            regData = 1;
            break;
         case LED_OP_PARALLEL:
            regData = 2;
            break;
        case LED_OP_SERIAL:
            regData = 3;
            if ((retVal = rtl8370_setAsicLedSerialModeConfig(0, 1))!=RT_ERR_OK)        
                return retVal;
            if ((retVal = rtl8370_setAsicLedSerialEnable(ENABLED))!=RT_ERR_OK)        
                return retVal; 
            break;
        default:
            regData = 0;
            break;
    }
    
    if ((retVal = rtl8370_setAsicLedOperationMode(regData))!=RT_ERR_OK)        
        return retVal;    

    return RT_ERR_OK;
}
/* Function Name:
 *      rtk_led_operation_get
 * Description:
 *      Get Led operation mode
 * Input:
 *      None
 * Output:
 *      pMode - Support LED operation mode.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get Led operation mode.
 *      The modes that can be set are as following:
 *      LED_OP_SCAN,        
 *      LED_OP_PARALLEL,        
 *      LED_OP_SERIAL,  
 */
rtk_api_ret_t rtk_led_operation_get(rtk_data_t *pMode)
{
    rtk_api_ret_t retVal;
    uint32 regData;
    
    if ((retVal = rtl8370_getAsicLedOperationMode(&regData))!=RT_ERR_OK)        
        return retVal; 

    if (regData == 1)
        *pMode = LED_OP_SCAN;
    else if (regData == 2)
        *pMode = LED_OP_PARALLEL;
    else if (regData == 3)    
        *pMode = LED_OP_SERIAL;
    else
       return RT_ERR_FAILED; 

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_led_mode_set
 * Description:
 *      Set Led to congiuration mode
 * Input:
 *      mode - Support LED mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      There are three LED groups for each port for indicating information about dedicated port.
 *      LED groups are set to indicate different information of port in different mode.
 *      
 *      Mode0
 *      LED0-Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      LED1-1000Mb/s Speed Indicator. Low for 1000Mb/s.
 *      LED2-100Mb/s Speed Indicator. Low for 100Mb/s.
 *      
 *      Mode1
 *      LED0-Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      LED1-1000Mb/s Speed Indicator. Low for 1000Mb/s.
 *      LED2-Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
 *      
 *      Mode2
 *      LED0-Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
 *      LED1-1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      LED2-10/100Mb/s Speed/Activity Indicator. Low for 10/100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      
 *      Mode3
 *      LED0-10Mb/s Speed/Activity Indicator. Low for 10Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      LED1-1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *      LED2-100Mb/s Speed/Activity Indicator. Low for 100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.  
 */

rtk_api_ret_t rtk_led_mode_set(rtk_led_mode_t mode)
{
    rtk_api_ret_t retVal;      

    if (mode >= LED_MODE_END)
        return RT_ERR_FAILED;

    if ((retVal = rtl8370_setAsicLedGroupMode(mode))!=RT_ERR_OK)        
        return retVal;    

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_led_mode_get
 * Description:
 *      Get Led to congiuration mode
 * Input:
 *      None
 * Output:
 *      pMode - Support LED mode.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      There are three LED groups for each port for indicating information about dedicated port. 
 *      LED groups is set to indicate different information of port in different mode.
 */
rtk_api_ret_t rtk_led_mode_get(rtk_data_t *pMode)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicLedGroupMode(pMode))!=RT_ERR_OK)        
        return retVal;
    
    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_led_modeForce_set
 * Description:
 *      Set Led group to congiuration force mode
 * Input:
 *      group - Support LED group id.
 *      mode - Support LED force mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can force all Leds in the same group to one force mode.
 *      The force modes that can be set are as following:
 *      LED_FORCE_NORMAL,
 *      LED_FORCE_BLINK,
 *      LED_FORCE_OFF,
 *      LED_FORCE_ON.  
 */
rtk_api_ret_t rtk_led_modeForce_set(rtk_led_group_t group, rtk_led_force_mode_t mode)
{
    rtk_api_ret_t retVal;
    uint32 gmsk;

    if (group >= LED_GROUP_END)
        return RT_ERR_INPUT;

    if (mode >= LED_FORCE_END)
        return RT_ERR_NOT_ALLOWED;

    gmsk = 1<<group;

    if ((retVal = rtl8370_setAsicForceGroupLed(gmsk,mode))!=RT_ERR_OK)        
        return retVal;    

    return RT_ERR_OK;
}
/* Function Name:
 *      rtk_led_modeForce_get
 * Description:
 *      Get Led group to congiuration force mode
 * Input:
 *      group - Support LED group id..
 * Output:
 *      pMode - Support LED force mode
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can get forced Led group mode.
 *      The force modes that can be set are as following:
 *      LED_FORCE_NORMAL,
 *      LED_FORCE_BLINK,
 *      LED_FORCE_OFF,
 *      LED_FORCE_ON.  
 */
rtk_api_ret_t rtk_led_modeForce_get(rtk_data_t *pGroupmask, rtk_data_t *pMode)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicForceGroupLed(pGroupmask, pMode))!=RT_ERR_OK)        
        return retVal;    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_blinkRate_set
 * Description:
 *      Set LED blinking rate 
 * Input:
 *      blinkRate - blinking rate.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      ASIC support 6 types of LED blinking rates at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */
rtk_api_ret_t rtk_led_blinkRate_set(rtk_led_blink_rate_t blinkRate)
{
    rtk_api_ret_t retVal;

    if (blinkRate >= (rtk_led_blink_rate_t)LEDBLINKRATE_MAX)
        return RT_ERR_FAILED;

    if ((retVal = rtl8370_setAsicLedBlinkRate(blinkRate))!=RT_ERR_OK)        
        return retVal;    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_blinkRate_get
 * Description:
 *      Get LED blinking rate at mode 0 to mode 3
 * Input:
 *      None
 * Output:
 *      pBlinkRate - blinking rate.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      There are  6 types of LED blinking rates at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */
rtk_api_ret_t rtk_led_blinkRate_get(rtk_data_t *pBlinkRate)
{
    rtk_api_ret_t retVal;

    if ((retVal = rtl8370_getAsicLedBlinkRate(pBlinkRate))!=RT_ERR_OK)        
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_groupConfig_set
 * Description:
 *      Set per group Led to congiuration mode
 * Input:
 *      group - LED group.
 *      config - LED configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can set LED indicated information configuration for each LED group with 1 to 1 led mapping to each port.
 *      Definition  LED Statuses      Description
 *      0000        LED_Off           LED pin Tri-State.
 *      0001        Dup/Col           Collision, Full duplex Indicator.
 *      0010        Link/Act          Link, Activity Indicator. 
 *      0011        Spd1000           1000Mb/s Speed Indicator.
 *      0100        Spd100            100Mb/s Speed Indicator.
 *      0101        Spd10             10Mb/s Speed Indicator. 
 *      0110        Spd1000/Act       1000Mb/s Speed/Activity Indicator. 
 *      0111        Spd100/Act        100Mb/s Speed/Activity Indicator. 
 *      1000        Spd10/Act         10Mb/s Speed/Activity Indicator. 
 *      1001        Spd100 (10)/Act   10/100Mb/s Speed/Activity Indicator. 
 *      1010        Fiber             Fiber link Indicator. 
 *      1011        Fault             Auto-negotiation Fault Indicator. 
 *      1100        Link/Rx           Link, Activity Indicator. 
 *      1101        Link/Tx           Link, Activity Indicator. 
 *      1110        Master            Link on Master Indicator. 
 *      1111        Act               Activity Indicator. Low for link established.
 */
rtk_api_ret_t rtk_led_groupConfig_set(rtk_led_group_t group, rtk_led_congig_t config)
{
    rtk_api_ret_t retVal;
    
    if ( LED_GROUP_END <= group)
        return RT_ERR_FAILED;

    if ( LED_CONFIG_END <= config)
        return RT_ERR_FAILED;

     if ((retVal = rtl8370_setAsicLedIndicateInfoConfig(group, config))!=RT_ERR_OK)        
        return retVal;   
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_led_groupConfig_get
 * Description:
 *      Get Led group congiuration mode
 * Input:
 *      group - LED group.
 * Output:
 *      pConfig - LED configuration.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *       The API can get LED indicated information configuration for each LED group.
 */
rtk_api_ret_t rtk_led_groupConfig_get(rtk_led_group_t group, rtk_data_t *pConfig)
{
    rtk_api_ret_t retVal;
    
    if ( LED_GROUP_END <= group)
        return RT_ERR_FAILED;

     if ((retVal = rtl8370_getAsicLedIndicateInfoConfig(group, pConfig))!=RT_ERR_OK)        
        return retVal;   
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_led_serialMode_set
 * Description:
 *      Set Led serial mode active congiuration
 * Input:
 *      active - LED group.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can set LED serial mode active congiuration.
 */
rtk_api_ret_t rtk_led_serialMode_set(rtk_led_active_t active)
{
    rtk_api_ret_t retVal;
    
    if ( active >= LED_ACTIVE_END)
        return RT_ERR_INPUT;

     if ((retVal = rtl8370_setAsicLedSerialModeConfig(active,1))!=RT_ERR_OK)        
        return retVal;   
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_led_serialMode_get
 * Description:
 *      Get Led group congiuration mode
 * Input:
 *      group - LED group.
 * Output:
 *      pConfig - LED configuration.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *       The API can get LED serial mode active configuration.
 */
rtk_api_ret_t rtk_led_serialMode_get(rtk_data_t *pActive)
{
    rtk_api_ret_t retVal;
    uint32 regData;
    
    if ((retVal = rtl8370_getAsicLedSerialModeConfig(pActive,&regData))!=RT_ERR_OK)        
        return retVal;   
    
    return RT_ERR_OK;
}

#if !defined(_REDUCE_CODE)

CONST_T uint32 filter_templateField[RTK_MAX_NUM_OF_FILTER_TYPE][RTK_MAX_NUM_OF_FILTER_FIELD] = {
    {DMAC2, DMAC1, DMAC0, SMAC2, SMAC1, SMAC0, ETHERTYPE},
    {IP4SIP1, IP4SIP0, IP4DIP1, IP4DIP0, IP4FLAGOFF, IP4TOSPROTO, CTAG},
    {IP6SIP7, IP6SIP6, IP6SIP4, IP6SIP3, IP6SIP2, IP6SIP1, IP6SIP0},
    {IP6DIP7, IP6DIP6, IP6DIP4, IP6DIP3, IP6DIP2, IP6DIP1, IP6DIP0},
    {TCPSPORT, TCPDPORT, TCPFLAG, ICMPCODETYPE, IGMPTYPE, TOSNH, STAG}
};

/* Function Name:
 *      rtk_filter_igrAcl_init 
 * Description:
 *      ACL initialization function
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_NULL_POINTER    - Pointer pFilter_field or pFilter_cfg point to NULL.
 * Note:
 *      This function enable and intialize ACL function
 */
rtk_api_ret_t rtk_filter_igrAcl_init(void)
{
    rtl8370_acl_template_t aclTemp;
    uint32                 i, j;
    rtk_api_ret_t          ret;

    if ((ret = rtk_filter_igrAcl_cfg_delAll()) != RT_ERR_OK)
        return ret;

    for(i = 0; i < RTK_MAX_NUM_OF_FILTER_TYPE; i++)
    {
        for(j = 0; j < RTK_MAX_NUM_OF_FILTER_FIELD;j++)
            aclTemp.field[j] = filter_templateField[i][j];
        
        if ((ret = rtl8370_setAsicAclType(i, aclTemp)) != SUCCESS)
            return ret;
    }

    for(i=0; i < RTK_MAX_NUM_OF_PORT;i++)
    {
        if ((ret = rtl8370_setAsicAcl(i, TRUE)) != SUCCESS)
            return ret;    
        
        if ((ret = rtl8370_setAsicAclUnmatchedPermit(i, TRUE)) != SUCCESS)
            return ret;    
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_igrAcl_field_add
 * Description:
 *      Add comparison rule to an ACL configuration
 * Input:
 *      pFilter_cfg | The ACL configuration that this function will add comparison rule
 *      pFilter_field | The comparison rule that will be added.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_NULL_POINTER    - Pointer pFilter_field or pFilter_cfg point to NULL.
 *      RT_ERR_INPUT - Invalid input parameters. 
 * Note:
 *      This function add a comparison rule (*pFilter_field) to an ACL configuration (*pFilter_cfg). 
 *      Pointer pFilter_cfg points to an ACL configuration structure, this structure keeps multiple ACL 
 *      comparison rules by means of linked list. Pointer pFilter_field will be added to linked 
 *      list keeped by structure that pFilter_cfg points to.
 */
rtk_api_ret_t rtk_filter_igrAcl_field_add(rtk_filter_cfg_t* pFilter_cfg, rtk_filter_field_t* pFilter_field)
{
    rtk_filter_field_t *tailPtr;

    if(NULL == pFilter_cfg || NULL == pFilter_field)
        return RT_ERR_NULL_POINTER;

    if (pFilter_field->fieldType >= FILTER_FIELD_MAX)
        return RT_ERR_ENTRY_INDEX;

    if(NULL == pFilter_cfg->fieldHead )
    {
        pFilter_cfg->fieldHead = pFilter_field;
    }
    else
    {
        if (pFilter_cfg->fieldHead->next == NULL)
        {
            pFilter_cfg->fieldHead->next = pFilter_field;
        }
        else
        {
            tailPtr = pFilter_cfg->fieldHead->next;
            while( tailPtr->next != NULL)
            {
                tailPtr = tailPtr->next;
            }
            tailPtr->next = pFilter_field;
        }
    }

    return RT_ERR_OK;
}

static rtk_api_ret_t _rtk_filter_igrAcl_writeDataField(rtl8370_acl_rule_t *aclRule, uint32 *tempIdx, uint32 *fieldIdx, rtk_filter_field_t *fieldPtr, rtk_filter_data_type_t type)
{
    uint32 i, aclIdx;

    aclIdx = 0xFF;

    for(i = 0; i < RTK_MAX_NUM_OF_FILTER_TYPE; i++)
    {
        if (FALSE == aclRule[i].valid)
        {
            aclIdx=i;
            break;
        }
        else if (TRUE == aclRule[i].valid && tempIdx[0] == aclRule[i].data_bits.type)
        {
            aclIdx=i;
            break;
        }
    }    

    if (0xFF == aclIdx)
        return RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;

    switch ( type )
    {
    /* use DMAC structure as representative for mac structure */
    case RTK_FILTER_DATA_MAC:
        if(FILTER_FIELD_DATA_MASK != fieldPtr->filter_pattern_union.dmac.dataType )
            return RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;

        for(i = 0; i < MAC_ADDR_LEN / 2;i++)
        {
            if(RTK_MAX_NUM_OF_FILTER_FIELD != tempIdx[i] || RTK_MAX_NUM_OF_FILTER_TYPE != fieldIdx[i])
            {
                aclRule[aclIdx].data_bits.field[fieldIdx[i]] = fieldPtr->filter_pattern_union.dmac.value.octet[5-i*2] | (fieldPtr->filter_pattern_union.dmac.value.octet[5-(i*2 + 1)] << 8);
                aclRule[aclIdx].care_bits.field[fieldIdx[i]] = fieldPtr->filter_pattern_union.dmac.mask.octet[5-i*2] | (fieldPtr->filter_pattern_union.dmac.mask.octet[5-(i*2 + 1)] << 8);
                aclRule[aclIdx].data_bits.type = tempIdx[i];
                aclRule[aclIdx].valid = TRUE;
            }        
        }
        break;
    /* use ETHERTYPE structure as representative for uint16 structure */
    case RTK_FILTER_DATA_UINT16:
        if(FILTER_FIELD_DATA_MASK != fieldPtr->filter_pattern_union.etherType.dataType)
            return RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;
        if(fieldPtr->filter_pattern_union.etherType.value > 0xFFFF || fieldPtr->filter_pattern_union.etherType.mask > 0xFFFF)
            return RT_ERR_INPUT;

        aclRule[aclIdx].data_bits.field[fieldIdx[0]] = fieldPtr->filter_pattern_union.etherType.value;
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] = fieldPtr->filter_pattern_union.etherType.mask;
        aclRule[aclIdx].data_bits.type = tempIdx[0];
        aclRule[aclIdx].valid = TRUE;
        break;
    /* use STAG structure as representative for TAG structure */        
    case RTK_FILTER_DATA_TAG:
        if(FILTER_FIELD_DATA_MASK != fieldPtr->filter_pattern_union.stag.vid.dataType ||
             FILTER_FIELD_DATA_MASK != fieldPtr->filter_pattern_union.stag.pri.dataType)
            return RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;
        if(fieldPtr->filter_pattern_union.stag.cfi.value > TRUE || fieldPtr->filter_pattern_union.stag.cfi.mask > TRUE ||
             fieldPtr->filter_pattern_union.stag.pri.value > RTK_DOT1P_PRIORITY_MAX || fieldPtr->filter_pattern_union.stag.pri.mask > RTK_DOT1P_PRIORITY_MAX ||
             fieldPtr->filter_pattern_union.stag.vid.value > RTK_VLAN_ID_MAX|| fieldPtr->filter_pattern_union.stag.vid.mask > RTK_VLAN_ID_MAX )
            return RT_ERR_INPUT;

        aclRule[aclIdx].data_bits.field[fieldIdx[0]] = (fieldPtr->filter_pattern_union.stag.pri.value << 13) | (fieldPtr->filter_pattern_union.stag.cfi.value << 12) | fieldPtr->filter_pattern_union.stag.vid.value;
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] = (fieldPtr->filter_pattern_union.stag.pri.mask << 13) | (fieldPtr->filter_pattern_union.stag.cfi.mask << 12) | fieldPtr->filter_pattern_union.stag.vid.mask;
        aclRule[aclIdx].data_bits.type = tempIdx[0];
        aclRule[aclIdx].valid = TRUE;
        break;            
    /* use sip structure as representative for IPV4 structure */        
    case RTK_FILTER_DATA_IPV4:
        if(FILTER_FIELD_DATA_MASK != fieldPtr->filter_pattern_union.sip.dataType)
            return RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;
        for(i = 0; i < IPV4_ADDR_LEN / 2; i++)
        {
            if(RTK_MAX_NUM_OF_FILTER_FIELD != tempIdx[i] || RTK_MAX_NUM_OF_FILTER_TYPE != fieldIdx[i] )
            {
                aclRule[aclIdx].data_bits.field[fieldIdx[i]] = (fieldPtr->filter_pattern_union.sip.value & (0xFFFF << (i << 4))) >> (i << 4);
                aclRule[aclIdx].care_bits.field[fieldIdx[i]] = (fieldPtr->filter_pattern_union.sip.mask & (0xFFFF << (i << 4))) >> (i << 4);
                aclRule[aclIdx].data_bits.type = tempIdx[i];
                aclRule[aclIdx].valid = TRUE;
            }
        }
        break;
    /* use ToS structure as representative for UINT8_HIGH structure */
    case RTK_FILTER_DATA_UINT8_HIGH:
        if(FILTER_FIELD_DATA_MASK != fieldPtr->filter_pattern_union.ipTos.dataType)
            return RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;
        if(fieldPtr->filter_pattern_union.ipTos.value > 0xFF || fieldPtr->filter_pattern_union.ipTos.mask > 0xFF )
            return RT_ERR_INPUT;

        aclRule[aclIdx].data_bits.field[fieldIdx[0]] &= 0xFF;
        aclRule[aclIdx].data_bits.field[fieldIdx[0]] |= (fieldPtr->filter_pattern_union.ipTos.value << 8);
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] &= 0xFF;
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] |= (fieldPtr->filter_pattern_union.ipTos.mask << 8);
        aclRule[aclIdx].data_bits.type = tempIdx[0]; 
        aclRule[aclIdx].valid = TRUE;                        
        break;
    /* use protocol structure as representative for UINT8_LOW structure */
    case RTK_FILTER_DATA_UINT8_LOW:
        if(FILTER_FIELD_DATA_MASK != fieldPtr->filter_pattern_union.protocol.dataType)
            return RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;
        if(fieldPtr->filter_pattern_union.protocol.value > 0xFF || fieldPtr->filter_pattern_union.protocol.mask > 0xFF )
            return RT_ERR_INPUT;
            
        aclRule[aclIdx].data_bits.field[fieldIdx[0]] &= 0xFF00;
        aclRule[aclIdx].data_bits.field[fieldIdx[0]] |= fieldPtr->filter_pattern_union.protocol.value;
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] &= 0xFF00;
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] |= fieldPtr->filter_pattern_union.protocol.mask;
        aclRule[aclIdx].data_bits.type = tempIdx[0]; 
        aclRule[aclIdx].valid = TRUE;                        
        break;
    case RTK_FILTER_DATA_IPV4FLAG:
        aclRule[aclIdx].data_bits.field[fieldIdx[0]] &= 0xFFF;
        aclRule[aclIdx].data_bits.field[fieldIdx[0]] |= (fieldPtr->filter_pattern_union.ipFlag.df.value<< 14);
        aclRule[aclIdx].data_bits.field[fieldIdx[0]] |= (fieldPtr->filter_pattern_union.ipFlag.mf.value << 13);
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] &= 0xFFF;
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] |= (fieldPtr->filter_pattern_union.ipFlag.df.mask << 14);
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] |= (fieldPtr->filter_pattern_union.ipFlag.mf.mask << 13);
        aclRule[aclIdx].data_bits.type = tempIdx[0]; 
        aclRule[aclIdx].valid = TRUE;         
        break;
    case RTK_FILTER_DATA_UINT13_LOW:
        if(FILTER_FIELD_DATA_MASK != fieldPtr->filter_pattern_union.ipOffset.dataType)
            return RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;
        if(fieldPtr->filter_pattern_union.ipOffset.value > 0x1FFF || fieldPtr->filter_pattern_union.ipOffset.mask > 0x1FFF )
            return RT_ERR_INPUT;
            
        aclRule[aclIdx].data_bits.field[fieldIdx[0]] &= 0xE000;
        aclRule[aclIdx].data_bits.field[fieldIdx[0]] |= fieldPtr->filter_pattern_union.ipOffset.value;
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] &= 0xE000;
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] |= fieldPtr->filter_pattern_union.ipOffset.mask;
        aclRule[aclIdx].data_bits.type = tempIdx[0]; 
        aclRule[aclIdx].valid = TRUE;                        
        break;
    case RTK_FILTER_DATA_TCPFLAG:
        aclRule[aclIdx].data_bits.field[fieldIdx[0]] = 
            (fieldPtr->filter_pattern_union.tcpFlag.fin.value) | (fieldPtr->filter_pattern_union.tcpFlag.syn.value << 1) |
            (fieldPtr->filter_pattern_union.tcpFlag.rst.value << 2) | (fieldPtr->filter_pattern_union.tcpFlag.psh.value << 3) |    
            (fieldPtr->filter_pattern_union.tcpFlag.ack.value << 4) | (fieldPtr->filter_pattern_union.tcpFlag.urg.value << 5) |
            (fieldPtr->filter_pattern_union.tcpFlag.ece.value << 6) | (fieldPtr->filter_pattern_union.tcpFlag.cwr.value << 7);
        aclRule[aclIdx].care_bits.field[fieldIdx[0]] = 
            (fieldPtr->filter_pattern_union.tcpFlag.fin.mask) | (fieldPtr->filter_pattern_union.tcpFlag.syn.mask << 1) |
            (fieldPtr->filter_pattern_union.tcpFlag.rst.mask << 2) | (fieldPtr->filter_pattern_union.tcpFlag.psh.mask << 3) |    
            (fieldPtr->filter_pattern_union.tcpFlag.ack.mask << 4) | (fieldPtr->filter_pattern_union.tcpFlag.urg.mask << 5) |
            (fieldPtr->filter_pattern_union.tcpFlag.ece.mask << 6) | (fieldPtr->filter_pattern_union.tcpFlag.cwr.mask << 7);
        aclRule[aclIdx].valid = TRUE; 
        break;        
    case RTK_FILTER_DATA_IPV6:
        if(FILTER_FIELD_DATA_MASK != fieldPtr->filter_pattern_union.dipv6.dataType )
            return RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;                
        for(i = 0; i < IPV6_ADDR_LEN / 2; i++)
        {
            if(RTK_MAX_NUM_OF_FILTER_FIELD != tempIdx[i] || RTK_MAX_NUM_OF_FILTER_TYPE != fieldIdx[i] )
            {
                if (i!=5)
                {
                    aclRule[aclIdx].data_bits.field[fieldIdx[i]] = (fieldPtr->filter_pattern_union.dipv6.value.addr[i/2] & (0xFFFF << ((i & 1) << 4))) >> ((i & 1) << 4);
                    aclRule[aclIdx].care_bits.field[fieldIdx[i]] = (fieldPtr->filter_pattern_union.dipv6.mask.addr[i/2] & (0xFFFF << ((i & 1) << 4))) >> ((i & 1) << 4);
                }
                aclRule[aclIdx].data_bits.type = tempIdx[i];
                aclRule[aclIdx].valid = TRUE;
            }                
        }
        aclRule[aclIdx].data_bits.type = tempIdx[0]; 
        aclRule[aclIdx].valid = TRUE;                        
        break;     
    default:
        return RT_ERR_INPUT;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_igrAcl_cfg_add
 * Description:
 *      Add an ACL configuration to ASIC
 * Input:
 *      filter_id - Start index of ACL configuration.
 *      pFilter_cfg - The ACL configuration that this function will add comparison rule
 *      pFilter_action - Action(s) of ACL configuration.
 * Output:
 *      ruleNum - number of rules written in acl table
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_NULL_POINTER    - Pointer pFilter_field or pFilter_cfg point to NULL.
 *      RT_ERR_INPUT - Invalid input parameters. 
 *      RT_ERR_ENTRY_INDEX - Invalid filter_id .
 *      RT_ERR_NULL_POINTER - Pointer pFilter_action or pFilter_cfg point to NULL.
 *      RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT - Action is not supported in this chip.
 *      RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT - Rule is not supported.
 * Note:
 *      This function store pFilter_cfg, pFilter_action into ASIC. The starting
 *      index(es) is filter_id.
 */
rtk_api_ret_t rtk_filter_igrAcl_cfg_add(rtk_filter_id_t filter_id, rtk_filter_cfg_t* pFilter_cfg, rtk_filter_action_t* pFilter_action, rtk_filter_number_t *ruleNum)
{
    rtk_api_ret_t           ret;
    uint32                  careTagData = 0, careTagMask = 0;
    uint32                  i, j, k, fieldCnt = 0;
    uint32                  fieldTypeLog[sizeof(filter_templateField) / sizeof(uint32)];
    uint32                  tempIdx[8], fieldIdx[8];
    CONST_T uint32            fieldSize[FILTER_FIELD_MAX] = {
        3, 3, 1, 1, 1, 
        2, 2, 1, 1, 1, 1, 8, 8, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1};
    CONST_T uint32             fieldStartIdx[FILTER_FIELD_MAX] = {
        DMAC0, SMAC0, ETHERTYPE, CTAG, STAG, 
        IP4SIP0, IP4DIP0, IP4TOSPROTO, IP4TOSPROTO, IP4FLAGOFF, IP4FLAGOFF, IP6SIP0, IP6DIP0, TOSNH, TOSNH,
        TCPSPORT, TCPDPORT, TCPFLAG, TCPSPORT, TCPDPORT, ICMPCODETYPE, ICMPCODETYPE, IGMPTYPE};
    CONST_T uint32             fieldDataType[FILTER_FIELD_MAX] = {
        RTK_FILTER_DATA_MAC, RTK_FILTER_DATA_MAC, RTK_FILTER_DATA_UINT16, RTK_FILTER_DATA_TAG, RTK_FILTER_DATA_TAG,
        RTK_FILTER_DATA_IPV4, RTK_FILTER_DATA_IPV4, RTK_FILTER_DATA_UINT8_HIGH, RTK_FILTER_DATA_UINT8_LOW, RTK_FILTER_DATA_IPV4FLAG,
        RTK_FILTER_DATA_UINT13_LOW, RTK_FILTER_DATA_IPV6, RTK_FILTER_DATA_IPV6, RTK_FILTER_DATA_UINT8_HIGH, RTK_FILTER_DATA_UINT8_LOW,
        RTK_FILTER_DATA_UINT16, RTK_FILTER_DATA_UINT16, RTK_FILTER_DATA_TCPFLAG,
        RTK_FILTER_DATA_UINT16, RTK_FILTER_DATA_UINT16, RTK_FILTER_DATA_UINT8_HIGH, RTK_FILTER_DATA_UINT8_LOW, RTK_FILTER_DATA_UINT16};
    uint32                  fwdEnable;
    uint32                  aclActCtrl;
    uint32                  cpuPort;
    uint32                  usedRule;
    rtl8370_acl_template_t  aclTemp[RTK_MAX_NUM_OF_FILTER_TYPE];
    rtk_filter_field_t*     fieldPtr;
    rtl8370_acl_rule_t      aclRule[RTK_MAX_NUM_OF_FILTER_TYPE];
    rtl8370_acl_rule_t      tempRule;
    rtl8370_acl_act_t       aclAct;
    rtl8370_svlan_memconf_t svlanMemConf;
    uint32 matchIdx;

    if(filter_id >= RTK_MAX_NUM_OF_ACL_RULE )
        return RT_ERR_ENTRY_INDEX;

    if(NULL == pFilter_cfg)
        return RT_ERR_NULL_POINTER;

    if(NULL == pFilter_action )
        return RT_ERR_NULL_POINTER;

    fieldPtr = pFilter_cfg->fieldHead;

    /* get template table from ASIC */
    for(i = 0; i < RTK_MAX_NUM_OF_FILTER_TYPE; i++)
    {
        if((ret=rtl8370_getAsicAclType(i, &aclTemp[i])) != SUCCESS )
            return ret;
    }
    /* init RULE */
    for(i = 0; i < RTK_MAX_NUM_OF_FILTER_TYPE; i++)
    {
        memset(&aclRule[i], 0, sizeof(rtl8370_acl_rule_t));
        aclRule[i].care_bits.type= 0x7;
    }
 
    while(NULL != fieldPtr)
    {
        /* check if the same data type has inputed */
        for(i = 0; i < fieldCnt;i++)
        {
            if(fieldTypeLog[i] == fieldPtr->fieldType )
                return RT_ERR_INPUT;
        }
        fieldTypeLog[fieldCnt] = fieldPtr->fieldType;
        fieldCnt++;

        /* check if data type is supported in RTL8370 */
        if(TYPE_MAX <= fieldStartIdx[fieldPtr->fieldType] )
            return RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;

        /* initialize field and template index array */
        for(i = 0; i < 8;i++)
        {
            fieldIdx[i] = RTK_MAX_NUM_OF_FILTER_FIELD;
            tempIdx[i]  = RTK_MAX_NUM_OF_FILTER_TYPE;
        }


        /* find the position in template */
        for(i = 0; i < fieldSize[fieldPtr->fieldType]; i++)
        {                
            for(j = 0;j < RTK_MAX_NUM_OF_FILTER_TYPE; j++)
            {
                for(k = 0; k < RTK_MAX_NUM_OF_FILTER_FIELD ;k++)
                {
                    if((fieldStartIdx[fieldPtr->fieldType] + i) == aclTemp[j].field[k] )
                    {
                 
                        tempIdx[i] = j;
                        fieldIdx[i] = k;
                    }
                }
            }
        }

        /* if no template match the input field, return err */
        for(i = 0; i < 8; i++)
        {
            if(RTK_MAX_NUM_OF_FILTER_FIELD != tempIdx[i] || RTK_MAX_NUM_OF_FILTER_TYPE != fieldIdx[i])
                break;
        }

        if(8 == i )
            return RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT;

        ret = _rtk_filter_igrAcl_writeDataField(aclRule, tempIdx, fieldIdx, fieldPtr, fieldDataType[fieldPtr->fieldType]);
        if(RT_ERR_OK != ret )
            return ret;     
        fieldPtr = fieldPtr->next;
    }

    for(i = 0; i < CARE_TAG_MAX;i++)
    {
        if(0 == pFilter_cfg->careTag.tagType[i].mask )
        {
            careTagMask &=  ~(1 << i);
        }
        else
        {
            careTagMask |= (1 << i);
            if(0 == pFilter_cfg->careTag.tagType[i].value )
                careTagData &= ~(1 << i);
            else
                careTagData |= (1 << i);
        }
    }

    for(i = 0; i < RTK_MAX_NUM_OF_FILTER_TYPE;i++)
    {
        aclRule[i].data_bits.tag_exist = (careTagData) & 0x1FF;
        aclRule[i].care_bits.tag_exist = (careTagMask) & 0x1FF;
    }
    
    if(FILTER_FIELD_DATA_RANGE == pFilter_cfg->activeport.dataType )
    {

        if(pFilter_cfg->activeport.rangeStart >= RTK_MAX_NUM_OF_FILTER_PORT || pFilter_cfg->activeport.rangeEnd >= RTK_MAX_NUM_OF_FILTER_PORT 
          || pFilter_cfg->activeport.rangeEnd > pFilter_cfg->activeport.rangeStart)
            return RT_ERR_INPUT;
    
        for(i = pFilter_cfg->activeport.rangeStart;i <= pFilter_cfg->activeport.rangeEnd;i++)
            aclRule[0].data_bits.active_portmsk |= 1 << i;

        aclRule[0].care_bits.active_portmsk = 0xFFFF;
    }
    else if(FILTER_FIELD_DATA_MASK == pFilter_cfg->activeport.dataType )
    {   
        if(pFilter_cfg->activeport.value >= (1 << RTK_MAX_NUM_OF_FILTER_PORT) || pFilter_cfg->activeport.mask >= (1 << RTK_MAX_NUM_OF_FILTER_PORT))
            return RT_ERR_INPUT;            
        aclRule[0].data_bits.active_portmsk = pFilter_cfg->activeport.value;
        aclRule[0].care_bits.active_portmsk = pFilter_cfg->activeport.mask;
    }
    else
        return RT_ERR_INPUT;
    if(pFilter_cfg->invert >= FILTER_INVERT_END )
        return RT_ERR_INPUT;

    /* check if there are multiple cvlan action */
    if(pFilter_action->actEnable[FILTER_ENACT_INGRESS_CVLAN_INDEX] == TRUE &&
         pFilter_action->actEnable[FILTER_ENACT_INGRESS_CVLAN_VID] == TRUE )
        return RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT;

    /* check if there are multiple forwarding action */
    fwdEnable = FALSE;
    for(i = FILTER_ENACT_TRAP_CPU; i <=  FILTER_ENACT_ADD_DSTPORT; i++)
    {
        if(pFilter_action->actEnable[i] == TRUE )
        {
            if(fwdEnable == FALSE )
                fwdEnable = TRUE;
            else
                return RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT;
        }
    }

    memset(&aclAct, 0, sizeof(rtl8370_acl_act_t));
    aclActCtrl = 0;
    for(i = 0; i < FILTER_ENACT_MAX;i++)
    {    
        if(pFilter_action->actEnable[i] > TRUE )
            return RT_ERR_INPUT;

        if(pFilter_action->actEnable[i] == TRUE )
        {
            switch (i)
            {
            case FILTER_ENACT_INGRESS_CVLAN_INDEX:
                if(pFilter_action->filterIngressCvlanIdx > RTK_VLAN_ID_MAX )
                    return RT_ERR_INPUT;
                aclAct.ct = TRUE;
                aclAct.aclcvid = pFilter_action->filterIngressCvlanIdx;
                aclActCtrl |= ACL_ACT_CVLAN_ENABLE_MASK;
                break;
            case FILTER_ENACT_INGRESS_CVLAN_VID:
                if(pFilter_action->filterIngressCvlanIdx >= RTK_MAX_NUM_OF_VLAN_INDEX )
                    return RT_ERR_INPUT;
                aclAct.ct = FALSE;
                aclAct.aclcvid = pFilter_action->filterIngressCvlanVid;
                aclActCtrl |= ACL_ACT_CVLAN_ENABLE_MASK;
                break;
            case FILTER_ENACT_EGRESS_SVLAN_INDEX:
                if(pFilter_action->filterEgressSvlanIdx >= RTK_MAX_NUM_OF_SVLAN_INDEX )
                    return RT_ERR_INPUT;
                aclAct.aclsvidx = pFilter_action->filterEgressSvlanIdx;
                aclActCtrl |= ACL_ACT_SVLAN_ENABLE_MASK;
                break;
            case FILTER_ENACT_POLICING_0:
                if(pFilter_action->filterPolicingIdx[0] >= RTK_MAX_NUM_OF_METER )
                    return RT_ERR_INPUT;
                aclAct.aclmeteridx = pFilter_action->filterPolicingIdx[0];
                aclActCtrl |= ACL_ACT_POLICING_ENABLE_MASK;
                break;
            case FILTER_ENACT_PRIORITY:
                if(pFilter_action->filterPriority > RTK_DOT1P_PRIORITY_MAX )
                    return RT_ERR_INPUT;
                aclAct.aclpri= pFilter_action->filterPriority;
                aclActCtrl |= ACL_ACT_PRIORITY_ENABLE_MASK;
                break;
            case FILTER_ENACT_DROP:
                aclAct.arpmsk = 0;
                aclAct.mrat = RTK_FILTER_FWD_REDIRECT;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_REDIRECT:
                if(pFilter_action->filterRedirectPortmask >= 1 << 10 )
                    return RT_ERR_INPUT;
                aclAct.arpmsk = pFilter_action->filterRedirectPortmask;
                aclAct.mrat = RTK_FILTER_FWD_REDIRECT;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_ADD_DSTPORT:
                if(pFilter_action->filterAddDstPortmask>= 1 << RTK_MAX_NUM_OF_FILTER_PORT )
                    return RT_ERR_INPUT;
                aclAct.arpmsk = pFilter_action->filterAddDstPortmask;
                aclAct.mrat = RTK_FILTER_FWD_MIRROR;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_MIRROR:
                aclAct.mrat = RTK_FILTER_FWD_MIRRORFUNTION;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_TRAP_CPU:
                aclAct.mrat = RTK_FILTER_FWD_TRAP;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_COPY_CPU:
                aclAct.mrat = RTK_FILTER_FWD_MIRROR;
                if((ret = rtl8370_getAsicCputagTrapPort(&cpuPort)) != SUCCESS)
                    return ret;
                aclAct.arpmsk = 1 << cpuPort;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_EGRESS_SVLAN_VID:
               if (pFilter_action->actEnable[FILTER_ENACT_EGRESS_SVLAN_INDEX] != TRUE)
                {
                    if (pFilter_action->filterEgressSvlanVid > RTK_VLAN_ID_MAX )
                        return RT_ERR_INPUT;
                    matchIdx = 0xFF;
                    for (j = 0; j <= RTK_MAX_NUM_OF_SVLAN_INDEX; j++)
                    {       
                        if ((ret = rtl8370_getAsicSvlanMemberConfiguration(j,&svlanMemConf))!=RT_ERR_OK)
                            return ret;
                        if ((pFilter_action->filterEgressSvlanVid==svlanMemConf.vs_svid) && (svlanMemConf.vs_member!=0))
                        { 
                            matchIdx = j;
                            aclAct.aclsvidx = j;
                            aclActCtrl |= ACL_ACT_SVLAN_ENABLE_MASK;
                            break;
                        }
                    }
                    if (matchIdx == 0xFF)
                           return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
                }
                else
                    return RT_ERR_INPUT;
                break;
            default:
                return RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT;                
            }
        }
    }

    usedRule = 0;
    for(i = 0; i < RTK_MAX_NUM_OF_FILTER_TYPE;i++)
    {
        if(aclRule[i].valid == TRUE )
            usedRule++;
    }

    *ruleNum = usedRule; 

    for(i = filter_id; i < filter_id + usedRule;i++)
    {
        if((ret = rtl8370_getAsicAclRule(i, &tempRule)) != SUCCESS )
            return ret;

        if(tempRule.valid== TRUE )
        {           
            return RT_ERR_TBL_FULL;
        }
    }

    for(i = 0; i < RTK_MAX_NUM_OF_FILTER_TYPE;i++)
    {
        if(aclRule[i].valid == TRUE )
        {         
            /* write ACL action control */
            if((ret = rtl8370_setAsicAclActCtrl(filter_id + i, aclActCtrl)) != SUCCESS )
                return ret;
            /* write ACL action */
            if((ret = rtl8370_setAsicAclAct(filter_id + i, aclAct)) != SUCCESS )
                return ret;
            /* write ACL not */
            if((ret = rtl8370_setAsicAclNot(filter_id + i, pFilter_cfg->invert)) != SUCCESS )
                return ret;
            /* write ACL rule */
            if((ret = rtl8370_setAsicAclRule(filter_id + i, &aclRule[i])) != SUCCESS )
                return ret;
            /* only the first rule will be written with input action control, aclActCtrl of other rules will be zero */
            aclActCtrl = 0;            
            memset(&aclAct, 0, sizeof(rtl8370_acl_act_t));
        }
    }    
 
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_igrAcl_cfg_del
 * Description:
 *      Delete an ACL configuration from ASIC
 * Input:
 *      filter_id | Start index of ACL configuration.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_FILTER_ENTRYIDX - Invalid filter_id.
 * Note:
 *      This function delete a group of ACL rules starting from filter_id.
 */
rtk_api_ret_t rtk_filter_igrAcl_cfg_del(rtk_filter_id_t filter_id)
{
#define FILTER_ACL_ACTCTRL_INIT 0x1F

    rtl8370_acl_rule_t initRule;
    rtl8370_acl_act_t initAct;
    rtk_api_ret_t ret;

    if(filter_id >= RTK_MAX_NUM_OF_ACL_RULE )
        return RT_ERR_FILTER_ENTRYIDX;

    memset(&initRule, 0, sizeof(rtl8370_acl_rule_t));
    memset(&initAct, 0, sizeof(rtl8370_acl_act_t));

    if((ret = rtl8370_setAsicAclRule(filter_id, &initRule)) != RT_ERR_OK)
        return ret;
    if((ret = rtl8370_setAsicAclActCtrl(filter_id, FILTER_ACL_ACTCTRL_INIT))!= RT_ERR_OK)
        return ret;
     if ((ret = rtl8370_setAsicAclAct(filter_id, initAct))!=RT_ERR_OK)
        return ret;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_igrAcl_cfg_delAll
 * Description:
 *      Delete all ACL entries from ASIC
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This function delete all ACL configuration from ASIC.
 */
rtk_api_ret_t rtk_filter_igrAcl_cfg_delAll(void)
{
#define ACL_ACTCTRL_INIT 0x1F

    rtl8370_acl_rule_t initRule;
    rtl8370_acl_act_t initAct;
    uint32 i;
    rtk_api_ret_t ret;

    memset(&initRule, 0, sizeof(rtl8370_acl_rule_t));
    memset(&initAct, 0, sizeof(rtl8370_acl_act_t));

    for(i = 0; i < RTK_MAX_NUM_OF_ACL_RULE;i++)
    {
        if((ret = rtl8370_setAsicAclRule(i, &initRule)) != RT_ERR_OK)
            return ret;
        if((ret = rtl8370_setAsicAclActCtrl(i, ACL_ACTCTRL_INIT))!= RT_ERR_OK)
            return ret;
         if ((ret = rtl8370_setAsicAclAct(i, initAct))!=RT_ERR_OK)
            return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_igrAcl_cfg_set
 * Description:
 *      Set one ingress acl configuration to ASIC.
 * Input:
 *      filter_id - Start index of ACL configuration.
 *      pFilter_cfg - buffer pointer of ingress acl data
 *      pFilter_action - buffer pointer of ingress acl action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This function delete all ACL configuration from ASIC.  
 */
rtk_api_ret_t rtk_filter_igrAcl_cfg_set(rtk_filter_id_t filter_id, rtk_filter_template_index_t template_index, rtk_filter_cfg_raw_t *pFilter_cfg, rtk_filter_action_t *pFilter_action)
{
    rtk_api_ret_t   retVal;
    uint32  i, j, aclActCtrl;
    rtl8370_acl_act_t   aclAct;
    uint32  cpuPort;
    rtl8370_svlan_memconf_t svlanMemConf;
    uint32 matchIdx;
    rtl8370_acl_rule_t aclRule;
    uint32 careTagData = 0, careTagMask = 0;
    
    if(filter_id >= RTK_MAX_NUM_OF_ACL_RULE)
        return RT_ERR_ENTRY_INDEX;

    if(NULL == pFilter_cfg)
        return RT_ERR_NULL_POINTER;

    if(NULL == pFilter_action )
        return RT_ERR_NULL_POINTER;

    memset(&aclAct, 0, sizeof(rtl8370_acl_act_t));
    aclActCtrl = 0;
    for(i = 0; i < FILTER_ENACT_MAX;i++)
    {    
        if(pFilter_action->actEnable[i] > TRUE )
            return RT_ERR_INPUT;

        if(pFilter_action->actEnable[i] == TRUE )
        {
            switch (i)
            {
            case FILTER_ENACT_INGRESS_CVLAN_INDEX:
                if(pFilter_action->filterIngressCvlanIdx > RTK_VLAN_ID_MAX )
                    return RT_ERR_INPUT;
                aclAct.ct = TRUE;
                aclAct.aclcvid = pFilter_action->filterIngressCvlanIdx;
                aclActCtrl |= ACL_ACT_CVLAN_ENABLE_MASK;
                break;
            case FILTER_ENACT_INGRESS_CVLAN_VID:
                if(pFilter_action->filterIngressCvlanIdx >= RTK_MAX_NUM_OF_VLAN_INDEX )
                    return RT_ERR_INPUT;
                aclAct.ct = FALSE;
                aclAct.aclcvid = pFilter_action->filterIngressCvlanVid;
                aclActCtrl |= ACL_ACT_CVLAN_ENABLE_MASK;
                break;
            case FILTER_ENACT_EGRESS_SVLAN_INDEX:
                if(pFilter_action->filterEgressSvlanIdx >= RTK_MAX_NUM_OF_SVLAN_INDEX )
                    return RT_ERR_INPUT;
                aclAct.aclsvidx = pFilter_action->filterEgressSvlanIdx;
                aclActCtrl |= ACL_ACT_SVLAN_ENABLE_MASK;
                break;
            case FILTER_ENACT_POLICING_0:
                if(pFilter_action->filterPolicingIdx[0] >= RTK_MAX_NUM_OF_METER )
                    return RT_ERR_INPUT;
                aclAct.aclmeteridx = pFilter_action->filterPolicingIdx[0];
                aclActCtrl |= ACL_ACT_POLICING_ENABLE_MASK;
                break;
            case FILTER_ENACT_PRIORITY:
                if(pFilter_action->filterPriority > RTK_DOT1P_PRIORITY_MAX )
                    return RT_ERR_INPUT;
                aclAct.aclpri= pFilter_action->filterPriority;
                aclActCtrl |= ACL_ACT_PRIORITY_ENABLE_MASK;
                break;
            case FILTER_ENACT_DROP:
                aclAct.arpmsk = 0;
                aclAct.mrat = RTK_FILTER_FWD_REDIRECT;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_REDIRECT:
                if(pFilter_action->filterRedirectPortmask >= 1 << 10 )
                    return RT_ERR_INPUT;
                aclAct.arpmsk = pFilter_action->filterRedirectPortmask;
                aclAct.mrat = RTK_FILTER_FWD_REDIRECT;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_ADD_DSTPORT:
                if(pFilter_action->filterAddDstPortmask>= 1 << RTK_MAX_NUM_OF_FILTER_PORT )
                    return RT_ERR_INPUT;
                aclAct.arpmsk = pFilter_action->filterAddDstPortmask;
                aclAct.mrat = RTK_FILTER_FWD_MIRROR;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_MIRROR:
                aclAct.mrat = RTK_FILTER_FWD_MIRRORFUNTION;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_TRAP_CPU:
                aclAct.mrat = RTK_FILTER_FWD_TRAP;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_COPY_CPU:
                aclAct.mrat = RTK_FILTER_FWD_MIRROR;
                if((retVal = rtl8370_getAsicCputagTrapPort(&cpuPort)) != SUCCESS)
                    return retVal;
                aclAct.arpmsk = 1 << cpuPort;
                aclActCtrl |= ACL_ACT_FWD_ENABLE_MASK;
                break;
            case FILTER_ENACT_EGRESS_SVLAN_VID:
               if (pFilter_action->actEnable[FILTER_ENACT_EGRESS_SVLAN_INDEX] != TRUE)
                {
                    if (pFilter_action->filterEgressSvlanVid > RTK_VLAN_ID_MAX )
                        return RT_ERR_INPUT;
                    matchIdx = 0xFF;
                    for (j = 0; j <= RTK_MAX_NUM_OF_SVLAN_INDEX; j++)
                    {       
                        if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(j,&svlanMemConf))!=RT_ERR_OK)
                            return retVal;
                        if ((pFilter_action->filterEgressSvlanVid==svlanMemConf.vs_svid) && (svlanMemConf.vs_member!=0))
                        { 
                            matchIdx = j;
                            aclAct.aclsvidx = j;
                            aclActCtrl |= ACL_ACT_SVLAN_ENABLE_MASK;
                            break;
                        }
                    }
                    if (matchIdx == 0xFF)
                        return RT_ERR_SVLAN_ENTRY_NOT_FOUND;
                }
                else
                    return RT_ERR_INPUT;
                break;
            default:
                return RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT;                
            }
        }
    }

    if(pFilter_cfg->invert >= FILTER_INVERT_END )
        return RT_ERR_INPUT;


    for(i = 0; i < CARE_TAG_MAX;i++)
    {
        if(0 == pFilter_cfg->careTag.tagType[i].mask )
        {
            careTagMask &=  ~(1 << i);
        }
        else
        {
            careTagMask |= (1 << i);
            if(0 == pFilter_cfg->careTag.tagType[i].value )
                careTagData &= ~(1 << i);
            else
                careTagData |= (1 << i);
        }
    }

    aclRule.data_bits.tag_exist = (careTagData) & 0x1FF;
    aclRule.care_bits.tag_exist = (careTagMask) & 0x1FF;
    
    if(FILTER_FIELD_DATA_RANGE == pFilter_cfg->activeport.dataType )
    {

        if(pFilter_cfg->activeport.rangeStart >= RTK_MAX_NUM_OF_FILTER_PORT || pFilter_cfg->activeport.rangeEnd >= RTK_MAX_NUM_OF_FILTER_PORT 
          || pFilter_cfg->activeport.rangeEnd > pFilter_cfg->activeport.rangeStart)
            return RT_ERR_INPUT;
    
        for(i = pFilter_cfg->activeport.rangeStart;i <= pFilter_cfg->activeport.rangeEnd;i++)
            aclRule.data_bits.active_portmsk |= 1 << i;

        aclRule.care_bits.active_portmsk = 0xFFFF;
    }
    else if(FILTER_FIELD_DATA_MASK == pFilter_cfg->activeport.dataType )
    {   
        if(pFilter_cfg->activeport.value >= (1 << RTK_MAX_NUM_OF_FILTER_PORT) || pFilter_cfg->activeport.mask >= (1 << RTK_MAX_NUM_OF_FILTER_PORT))
            return RT_ERR_INPUT;            
        aclRule.data_bits.active_portmsk = pFilter_cfg->activeport.value;
        aclRule.care_bits.active_portmsk = pFilter_cfg->activeport.mask;
    }
    else
        return RT_ERR_INPUT;
    

    aclRule.data_bits.type = template_index;
    aclRule.care_bits.type = 0x7;
    for(i = 0; i < RTK_MAX_NUM_OF_FILTER_FIELD; i ++)
    {
        aclRule.data_bits.field[i] = pFilter_cfg->dataFieldRaw[i];
        aclRule.care_bits.field[i] = pFilter_cfg->careFieldRaw[i];
    }

    aclRule.valid = ENABLED;
    
    /* write ACL action control */
    if((retVal = rtl8370_setAsicAclActCtrl(filter_id, aclActCtrl)) != SUCCESS )
        return retVal;
    /* write ACL action */
    if((retVal = rtl8370_setAsicAclAct(filter_id, aclAct)) != SUCCESS )
        return retVal;
    /* write ACL not */
    if((retVal = rtl8370_setAsicAclNot(filter_id, pFilter_cfg->invert)) != SUCCESS )
        return retVal;  
    /* write ACL rule */
    if((retVal = rtl8370_setAsicAclRule(filter_id, &aclRule)) != SUCCESS )
        return retVal;           
    
    return RT_ERR_OK;
}
/* Function Name:
 *      rtk_filter_igrAcl_cfg_get
 * Description:
 *      Get one ingress acl configuration from ASIC.
 * Input:
 *      filter_id - Start index of ACL configuration.
 * Output:
 *      pFilter_cfg - buffer pointer of ingress acl data
 *      pFilter_action - buffer pointer of ingress acl action
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_NULL_POINTER - Pointer pFilter_action or pFilter_cfg point to NULL.
 *      RT_ERR_FILTER_ENTRYIDX - Invalid entry index.
 * Note:
 *      This function delete all ACL configuration from ASIC.  
 */
rtk_api_ret_t rtk_filter_igrAcl_cfg_get(rtk_filter_id_t filter_id, rtk_filter_cfg_raw_t *pFilter_cfg, rtk_filter_action_t *pAction)
{
    rtk_api_ret_t                       retVal;
    uint32                              i, tmp;
    rtl8370_acl_rule_t                  aclRule;
    rtl8370_acl_act_t                   aclAct;
    uint32                              cpuPort;
    rtl8370_acl_template_t              type;
    rtl8370_svlan_memconf_t             svlanMemConf;
    CONST_T rtk_filter_field_type_raw_t rawTypeMap[] = {
        FILTER_FIELD_RAW_DMAC_15_0, FILTER_FIELD_RAW_DMAC_31_16, FILTER_FIELD_RAW_DMAC_47_32, 
        FILTER_FIELD_RAW_SMAC_15_0, FILTER_FIELD_RAW_SMAC_31_16, FILTER_FIELD_RAW_SMAC_47_32,
        FILTER_FIELD_RAW_ETHERTYPE, FILTER_FIELD_RAW_STAG, FILTER_FIELD_RAW_CTAG, 
        FILTER_FIELD_RAW_IPV4_SIP_15_0, FILTER_FIELD_RAW_IPV4_SIP_31_16, 
        FILTER_FIELD_RAW_IPV4_DIP_15_0, FILTER_FIELD_RAW_IPV4_DIP_31_16, 
        FILTER_FIELD_RAW_IPV4_TOS_PROTOCOL, FILTER_FIELD_RAW_IPV4_FLAG_OFFSET, 
        FILTER_FIELD_RAW_TCP_SPORT, FILTER_FIELD_RAW_TCP_DPORT, FILTER_FIELD_RAW_TCP_FLAG, 
        FILTER_FIELD_RAW_UDP_SPORT, FILTER_FIELD_RAW_UDP_DPORT, 
        FILTER_FIELD_RAW_ICMP_CODE_TYPE, FILTER_FIELD_RAW_IGMP_TYPE, 
        FILTER_FIELD_RAW_IPV6_SIP_15_0, FILTER_FIELD_RAW_IPV6_SIP_31_16, FILTER_FIELD_RAW_IPV6_SIP_47_32, 
        FILTER_FIELD_RAW_IPV6_SIP_63_48, FILTER_FIELD_RAW_IPV6_SIP_79_64, FILTER_FIELD_RAW_IPV6_SIP_95_80,         
        FILTER_FIELD_RAW_IPV6_SIP_111_96, FILTER_FIELD_RAW_IPV6_SIP_127_112, 
        FILTER_FIELD_RAW_IPV6_DIP_15_0, FILTER_FIELD_RAW_IPV6_DIP_31_16, FILTER_FIELD_RAW_IPV6_DIP_47_32, 
        FILTER_FIELD_RAW_IPV6_DIP_63_48, FILTER_FIELD_RAW_IPV6_DIP_79_64, FILTER_FIELD_RAW_IPV6_DIP_95_80,         
        FILTER_FIELD_RAW_IPV6_DIP_111_96, FILTER_FIELD_RAW_IPV6_DIP_127_112,         
        FILTER_FIELD_RAW_IPV6_TRAFFIC_CLASS_NEXT_HEADER
    };


    if ((retVal = rtl8370_getAsicAclRule(filter_id, &aclRule)) != RT_ERR_OK)
        return retVal;

    pFilter_cfg->activeport.dataType = FILTER_FIELD_DATA_MASK;
    pFilter_cfg->activeport.value = aclRule.data_bits.active_portmsk;
    pFilter_cfg->activeport.mask = aclRule.care_bits.active_portmsk;

    for( i = 0; i < CARE_TAG_MAX; i++)
    {
        if(aclRule.data_bits.tag_exist & (1 << i))
            pFilter_cfg->careTag.tagType[i].value = 1;
        else
            pFilter_cfg->careTag.tagType[i].value = 0;
        
        if (aclRule.care_bits.tag_exist & (1 << i))
            pFilter_cfg->careTag.tagType[i].mask = 1;
        else
            pFilter_cfg->careTag.tagType[i].mask = 0;
    }

    for(i = 0; i < RTK_MAX_NUM_OF_FILTER_FIELD;i++)
    {
        pFilter_cfg->careFieldRaw[i] = aclRule.care_bits.field[i];
        pFilter_cfg->dataFieldRaw[i] = aclRule.data_bits.field[i];          
    }

    if ((retVal = rtl8370_getAsicAclNot(filter_id, &tmp))!= RT_ERR_OK)
        return retVal;

    pFilter_cfg->invert = tmp;

    pFilter_cfg->valid = aclRule.valid;

    memset(pAction, 0, sizeof(rtk_filter_action_t));

    if ((retVal = rtl8370_getAsicAclActCtrl(filter_id, &tmp))!= RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_getAsicAclAct(filter_id, &aclAct)) != RT_ERR_OK)
        return retVal;

    if(tmp & ACL_ACT_FWD_ENABLE_MASK )
    {
        if(aclAct.mrat == RTK_FILTER_FWD_TRAP )
            pAction->actEnable[FILTER_ENACT_TRAP_CPU] = TRUE;
        else if (aclAct.mrat == RTK_FILTER_FWD_MIRRORFUNTION )
            pAction->actEnable[FILTER_ENACT_MIRROR] = TRUE;
        else if (aclAct.mrat == RTK_FILTER_FWD_REDIRECT)
        {
            if(aclAct.arpmsk == 0 )
                pAction->actEnable[FILTER_ENACT_DROP] = TRUE;
            else
            {
                pAction->actEnable[FILTER_ENACT_REDIRECT] = TRUE;
                pAction->filterRedirectPortmask = aclAct.arpmsk;
            }
        }
        else if (aclAct.mrat == RTK_FILTER_FWD_MIRROR)
        {
            if((retVal = rtl8370_getAsicCputagTrapPort(&cpuPort)) != SUCCESS)
                return retVal;
            if (aclAct.arpmsk == (1 << cpuPort))
            {
                pAction->actEnable[FILTER_ENACT_COPY_CPU] = TRUE;
            }
            else
            {
                pAction->actEnable[FILTER_ENACT_ADD_DSTPORT] = TRUE;
                pAction->filterAddDstPortmask = aclAct.arpmsk;
            }
        }        
        else
        {
            return RT_ERR_FAILED;
        }
    }

    if(tmp & ACL_ACT_POLICING_ENABLE_MASK )
    {
        pAction->actEnable[FILTER_ENACT_POLICING_0] = TRUE;
        pAction->filterPolicingIdx[0] = aclAct.aclmeteridx;
    }

    if(tmp & ACL_ACT_PRIORITY_ENABLE_MASK )
    {
        pAction->actEnable[FILTER_ENACT_PRIORITY] = TRUE;
        pAction->filterPriority = aclAct.aclpri;
    }

    if(tmp & ACL_ACT_SVLAN_ENABLE_MASK )
    {
        pAction->actEnable[FILTER_ENACT_EGRESS_SVLAN_INDEX] = TRUE;
        pAction->actEnable[FILTER_ENACT_EGRESS_SVLAN_VID] = TRUE;
        pAction->filterEgressSvlanIdx = aclAct.aclsvidx;
        if ((retVal = rtl8370_getAsicSvlanMemberConfiguration(aclAct.aclsvidx,&svlanMemConf))!=RT_ERR_OK)
            return retVal;
        pAction->filterEgressSvlanVid = svlanMemConf.vs_svid;
    }

    if(tmp & ACL_ACT_CVLAN_ENABLE_MASK)
    {
        if(aclAct.ct == 0 )
        {
            pAction->actEnable[FILTER_ENACT_INGRESS_CVLAN_VID] = TRUE;
            pAction->filterIngressCvlanVid = aclAct.aclcvid;            
        }
        else if(aclAct.ct == 1)
        {
            pAction->actEnable[FILTER_ENACT_INGRESS_CVLAN_INDEX] = TRUE;
            pAction->filterIngressCvlanIdx = aclAct.aclcvid;                        
        }
        else
        {
            return RT_ERR_FAILED;
        }
    }

    /* Write field type of RAW data */
    if ((retVal = rtl8370_getAsicAclType(aclRule.data_bits.type, &type))!= RT_ERR_OK)
        return retVal;
    
    for(i = 0; i < RTK_MAX_NUM_OF_FILTER_FIELD; i++)
    {
        pFilter_cfg->fieldRawType[i] = rawTypeMap[type.field[i]];
    }/* end of for(i...) */

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_igrAcl_unmatchAction_set
 * Description:
 *      Set action to packets when no ACL configuration match
 * Input:
 *      port - Port id.
 *      action - Action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port id.
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This function sets action of packets when no ACL configruation matches.
 */
rtk_api_ret_t rtk_filter_igrAcl_unmatchAction_set(rtk_port_t port, rtk_filter_unmatch_action_t action)
{
    rtk_api_ret_t ret;

    if(port >= RTK_MAX_NUM_OF_PORT )
        return RT_ERR_PORT_ID;
    if(action >     RTK_ENABLE_END)
        return RT_ERR_INPUT;

   if((ret = rtl8370_setAsicAclUnmatchedPermit(port, action)) != RT_ERR_OK)
       return ret;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_igrAcl_unmatchAction_get
 * Description:
 *      Get action to packets when no ACL configuration match
 * Input:
 *      port - Port id.
 * Output:
 *      pAction - Action.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID         - Invalid port id.
 *      RT_ERR_INPUT           - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no ACL configruation matches.
 */
rtk_api_ret_t rtk_filter_igrAcl_unmatchAction_get(rtk_port_t port, rtk_filter_unmatch_action_t* pAction)
{
    rtk_api_ret_t ret;

    if(port >= RTK_MAX_NUM_OF_PORT )
        return RT_ERR_PORT_ID;

   if((ret = rtl8370_getAsicAclUnmatchedPermit(port, pAction)) != RT_ERR_OK)
       return ret;

   return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_igrAcl_state_set
 * Description:
 *      Set state of ingress ACL.
 * Input:
 *      port - Port id.
 *      state - Ingress ACL state.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID         - Invalid port id.
 *      RT_ERR_INPUT           - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no ACL configruation matches.
 */
rtk_api_ret_t rtk_filter_igrAcl_state_set(rtk_port_t port, rtk_filter_state_t state)
{
    rtk_api_ret_t ret;

    if(port >= RTK_MAX_NUM_OF_PORT )
        return RT_ERR_PORT_ID;
    if(state >     RTK_ENABLE_END)
        return RT_ERR_INPUT;

   if((ret = rtl8370_setAsicAcl(port, state)) != RT_ERR_OK)
       return ret;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_filter_igrAcl_state_get
 * Description:
 *      Get state of ingress ACL.
 * Input:
 *      port - Port id.
 * Output:
 *      pState - Ingress ACL state.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID         - Invalid port id.
 *      RT_ERR_INPUT           - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no ACL configruation matches.
 */
rtk_api_ret_t rtk_filter_igrAcl_state_get(rtk_port_t port, rtk_filter_state_t* pState)
{
    rtk_api_ret_t ret;

    if(port >= RTK_MAX_NUM_OF_PORT )
        return RT_ERR_PORT_ID;

   if((ret = rtl8370_getAsicAcl(port, pState)) != RT_ERR_OK)
       return ret;

   return RT_ERR_OK;   
}
/* Function Name:
 *      rtk_filter_igrAcl_template_set
 * Description:
 *      Set template of ingress ACL.
 * Input:
 *      template - Ingress ACL template
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Invalid input parameters.
 * Note:
 *      This function set ACL template.
 */
rtk_api_ret_t rtk_filter_igrAcl_template_set(rtk_filter_template_t *aclTemplate)
{
    rtk_api_ret_t ret;
    uint32 idxField;
    rtl8370_acl_template_t aclType;
    
    if(aclTemplate->index >= RTK_MAX_NUM_OF_FILTER_TYPE)
        return RT_ERR_INPUT;

    for(idxField = 0; idxField < RTK_MAX_NUM_OF_FILTER_FIELD; idxField ++)
    {
        if(aclTemplate->fieldType[idxField] >= FILTER_FIELD_RAW_MAX)
            return RT_ERR_INPUT;
        
        aclType.field[idxField] = aclTemplate->fieldType[idxField];
    }    

    ret = rtl8370_setAsicAclType(aclTemplate->index, aclType);

    return RT_ERR_OK;
}
/* Function Name:
 *      rtk_filter_igrAcl_template_get
 * Description:
 *      Get template of ingress ACL.
 * Input:
 *      template - Ingress ACL template
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This function gets template of ACL.
 */
rtk_api_ret_t rtk_filter_igrAcl_template_get(rtk_filter_template_t *aclTemplate)
{
    rtk_api_ret_t ret;
    uint32 idxField;
    rtl8370_acl_template_t aclType;
    
    if(aclTemplate->index >= RTK_MAX_NUM_OF_FILTER_TYPE)
        return RT_ERR_INPUT;

   if((ret = rtl8370_getAsicAclType(aclTemplate->index, &aclType)) != RT_ERR_OK)
       return ret;

    for(idxField = 0; idxField < RTK_MAX_NUM_OF_FILTER_FIELD; idxField ++)
    {
        aclTemplate->fieldType[idxField] = aclType.field[idxField];
    }  

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_eee_init
 * Description:
 *      EEE function initialization.
 * Input:
 *      None
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is used to initialize EEE status.
 */
 
rtk_api_ret_t rtk_eee_init(void)
{
    rtk_api_ret_t retVal;
    uint32 index,regData;
#ifndef MDC_MDIO_OPERATION 
    uint32 busyFlag,cnt;
#endif

    CONST_T uint16 ParaA[][2] = {
    {0x133F, 0x0030},{0x133E, 0x000E},{0x221F, 0x0005},{0x2213, 0x0003},
    {0x2205, 0x8B82},{0x2206, 0x05CB},{0x221F, 0x0002},{0x2204, 0x80C2},
    {0x2205, 0x0938},{0x221F, 0x0003},{0x2212, 0xC4D2},{0x220D, 0x0207},
    {0x221F, 0x0001},{0x2207, 0x267E},{0x221C, 0xE5F7},{0x221B, 0x0424},
    {0x221F, 0x0007},{0x221E, 0x0042},{0x2218, 0x0000},{0x221E, 0x002D},
    {0x2218, 0xF010},{0x221E, 0x0028},{0x2216, 0xF640},{0x221E, 0x0021},
    {0x2219, 0x2929},{0x221A, 0x1005},{0x221E, 0x0020},{0x2217, 0x000A},
    {0x221B, 0x2F4A},{0x2215, 0x0100},{0x221E, 0x0040},{0x221A, 0x5110},
    {0x2218, 0x0000},{0x221E, 0x0041},{0x2215, 0x0E02},{0x2216, 0x2185},
    {0x2217, 0x000C},{0x221C, 0x0008},{0x221E, 0x0042},{0x2215, 0x0D00},
    {0x221F, 0x0005},{0x2205, 0xFFF6},{0x2206, 0x0080},{0x221F, 0x0000},
    {0x2217, 0x2160},{0x221F, 0x0007},{0x221E, 0x0040},{0x2218, 0x0004},
    {0x2218, 0x0004},{0x2219, 0x4000},{0x2218, 0x0014},{0x2219, 0x7F00},
    {0x2218, 0x0024},{0x2219, 0x0000},{0x2218, 0x0034},{0x2219, 0x0100},
    {0x2218, 0x0044},{0x2219, 0xE000},{0x2218, 0x0054},{0x2219, 0x0000},
    {0x2218, 0x0064},{0x2219, 0x0000},{0x2218, 0x0074},{0x2219, 0x0000},
    {0x2218, 0x0084},{0x2219, 0x0400},{0x2218, 0x0094},{0x2219, 0x8000},
    {0x2218, 0x00A4},{0x2219, 0x7F00},{0x2218, 0x00B4},{0x2219, 0x4000},
    {0x2218, 0x00C4},{0x2219, 0x2000},{0x2218, 0x00D4},{0x2219, 0x0100},
    {0x2218, 0x00E4},{0x2219, 0x8400},{0x2218, 0x00F4},{0x2219, 0x7A00},
    {0x2218, 0x0104},{0x2219, 0x4000},{0x2218, 0x0114},{0x2219, 0x3F00},
    {0x2218, 0x0124},{0x2219, 0x0100},{0x2218, 0x0134},{0x2219, 0x7800},
    {0x2218, 0x0144},{0x2219, 0x0000},{0x2218, 0x0154},{0x2219, 0x0000},
    {0x2218, 0x0164},{0x2219, 0x0000},{0x2218, 0x0174},{0x2219, 0x0400},
    {0x2218, 0x0184},{0x2219, 0x8000},{0x2218, 0x0194},{0x2219, 0x7F00},
    {0x2218, 0x01A4},{0x2219, 0x8300},{0x2218, 0x01B4},{0x2219, 0x8300},
    {0x2218, 0x01C4},{0x2219, 0xE200},{0x2218, 0x01D4},{0x2219, 0x0A00},
    {0x2218, 0x01E4},{0x2219, 0x8800},{0x2218, 0x01F4},{0x2219, 0x0300},
    {0x2218, 0x0204},{0x2219, 0xE100},{0x2218, 0x0214},{0x2219, 0x4600},
    {0x2218, 0x0224},{0x2219, 0x4000},{0x2218, 0x0234},{0x2219, 0x7F00},
    {0x2218, 0x0244},{0x2219, 0x0000},{0x2218, 0x0254},{0x2219, 0x0100},
    {0x2218, 0x0264},{0x2219, 0x4000},{0x2218, 0x0274},{0x2219, 0x3E00},
    {0x2218, 0x0284},{0x2219, 0x0000},{0x2218, 0x0294},{0x2219, 0xE000},
    {0x2218, 0x02A4},{0x2219, 0x1200},{0x2218, 0x02B4},{0x2219, 0x8000},
    {0x2218, 0x02C4},{0x2219, 0x7F00},{0x2218, 0x02D4},{0x2219, 0x8900},
    {0x2218, 0x02E4},{0x2219, 0x8300},{0x2218, 0x02F4},{0x2219, 0xE000},
    {0x2218, 0x0304},{0x2219, 0x0000},{0x2218, 0x0314},{0x2219, 0x4000},
    {0x2218, 0x0324},{0x2219, 0x7F00},{0x2218, 0x0334},{0x2219, 0x0000},
    {0x2218, 0x0344},{0x2219, 0x2000},{0x2218, 0x0354},{0x2219, 0x4000},
    {0x2218, 0x0364},{0x2219, 0x3E00},{0x2218, 0x0374},{0x2219, 0xFD00},
    {0x2218, 0x0384},{0x2219, 0x0000},{0x2218, 0x0394},{0x2219, 0x1200},
    {0x2218, 0x03A4},{0x2219, 0xAB00},{0x2218, 0x03B4},{0x2219, 0x0C00},
    {0x2218, 0x03C4},{0x2219, 0x0600},{0x2218, 0x03D4},{0x2219, 0xA000},
    {0x2218, 0x03E4},{0x2219, 0x3D00},{0x2218, 0x03F4},{0x2219, 0xFB00},
    {0x2218, 0x0404},{0x2219, 0xE000},{0x2218, 0x0414},{0x2219, 0x0000},
    {0x2218, 0x0424},{0x2219, 0x4000},{0x2218, 0x0434},{0x2219, 0x7F00},
    {0x2218, 0x0444},{0x2219, 0x0000},{0x2218, 0x0454},{0x2219, 0x0100},
    {0x2218, 0x0464},{0x2219, 0x4000},{0x2218, 0x0474},{0x2219, 0xC600},
    {0x2218, 0x0484},{0x2219, 0xFF00},{0x2218, 0x0494},{0x2219, 0x0000},
    {0x2218, 0x04A4},{0x2219, 0x1000},{0x2218, 0x04B4},{0x2219, 0x0200},
    {0x2218, 0x04C4},{0x2219, 0x7F00},{0x2218, 0x04D4},{0x2219, 0x4000},
    {0x2218, 0x04E4},{0x2219, 0x7F00},{0x2218, 0x04F4},{0x2219, 0x0200},
    {0x2218, 0x0504},{0x2219, 0x0200},{0x2218, 0x0514},{0x2219, 0x5200},
    {0x2218, 0x0524},{0x2219, 0xC400},{0x2218, 0x0534},{0x2219, 0x7400},
    {0x2218, 0x0544},{0x2219, 0x0000},{0x2218, 0x0554},{0x2219, 0x1000},
    {0x2218, 0x0564},{0x2219, 0xBC00},{0x2218, 0x0574},{0x2219, 0x0600},
    {0x2218, 0x0584},{0x2219, 0xFE00},{0x2218, 0x0594},{0x2219, 0x4000},
    {0x2218, 0x05A4},{0x2219, 0x7F00},{0x2218, 0x05B4},{0x2219, 0x0000},
    {0x2218, 0x05C4},{0x2219, 0x0A00},{0x2218, 0x05D4},{0x2219, 0x5200},
    {0x2218, 0x05E4},{0x2219, 0xE400},{0x2218, 0x05F4},{0x2219, 0x3C00},
    {0x2218, 0x0604},{0x2219, 0x0000},{0x2218, 0x0614},{0x2219, 0x1000},
    {0x2218, 0x0624},{0x2219, 0x8A00},{0x2218, 0x0634},{0x2219, 0x7F00},
    {0x2218, 0x0644},{0x2219, 0x4000},{0x2218, 0x0654},{0x2219, 0x7F00},
    {0x2218, 0x0664},{0x2219, 0x0100},{0x2218, 0x0674},{0x2219, 0x2000},
    {0x2218, 0x0684},{0x2219, 0x0000},{0x2218, 0x0694},{0x2219, 0xE600},
    {0x2218, 0x06A4},{0x2219, 0xFF00},{0x2218, 0x06B4},{0x2219, 0x0000},
    {0x2218, 0x06C4},{0x2219, 0x5000},{0x2218, 0x06D4},{0x2219, 0x9D00},
    {0x2218, 0x06E4},{0x2219, 0xFF00},{0x2218, 0x06F4},{0x2219, 0x4000},
    {0x2218, 0x0704},{0x2219, 0x7F00},{0x2218, 0x0714},{0x2219, 0x0000},
    {0x2218, 0x0724},{0x2219, 0x2000},{0x2218, 0x0734},{0x2219, 0x0000},
    {0x2218, 0x0744},{0x2219, 0xE600},{0x2218, 0x0754},{0x2219, 0xFF00},
    {0x2218, 0x0764},{0x2219, 0x0000},{0x2218, 0x0774},{0x2219, 0x5000},
    {0x2218, 0x0784},{0x2219, 0x8500},{0x2218, 0x0794},{0x2219, 0x7F00},
    {0x2218, 0x07A4},{0x2219, 0xAC00},{0x2218, 0x07B4},{0x2219, 0x0800},
    {0x2218, 0x07C4},{0x2219, 0xFC00},{0x2218, 0x07D4},{0x2219, 0x4000},
    {0x2218, 0x07E4},{0x2219, 0x7F00},{0x2218, 0x07F4},{0x2219, 0x0400},
    {0x2218, 0x0804},{0x2219, 0x0200},{0x2218, 0x0814},{0x2219, 0x0000},
    {0x2218, 0x0824},{0x2219, 0xFF00},{0x2218, 0x0834},{0x2219, 0x7F00},
    {0x2218, 0x0844},{0x2219, 0x0000},{0x2218, 0x0854},{0x2219, 0x4200},
    {0x2218, 0x0864},{0x2219, 0x0500},{0x2218, 0x0874},{0x2219, 0x9000},
    {0x2218, 0x0884},{0x2219, 0x8000},{0x2218, 0x0894},{0x2219, 0x7D00},
    {0x2218, 0x08A4},{0x2219, 0x8C00},{0x2218, 0x08B4},{0x2219, 0x8300},
    {0x2218, 0x08C4},{0x2219, 0xE000},{0x2218, 0x08D4},{0x2219, 0x0000},
    {0x2218, 0x08E4},{0x2219, 0x4000},{0x2218, 0x08F4},{0x2219, 0x0400},
    {0x2218, 0x0904},{0x2219, 0xFF00},{0x2218, 0x0914},{0x2219, 0x0500},
    {0x2218, 0x0924},{0x2219, 0x8500},{0x2218, 0x0934},{0x2219, 0x8C00},
    {0x2218, 0x0944},{0x2219, 0xFA00},{0x2218, 0x0954},{0x2219, 0xE000},
    {0x2218, 0x0964},{0x2219, 0x0000},{0x2218, 0x0974},{0x2219, 0x4000},
    {0x2218, 0x0984},{0x2219, 0x5F00},{0x2218, 0x0994},{0x2219, 0x0400},
    {0x2218, 0x09A4},{0x2219, 0x0000},{0x2218, 0x09B4},{0x2219, 0xFE00},
    {0x2218, 0x09C4},{0x2219, 0x7300},{0x2218, 0x09D4},{0x2219, 0x0D00},
    {0x2218, 0x09E4},{0x2219, 0x0300},{0x2218, 0x09F4},{0x2219, 0x4000},
    {0x2218, 0x0A04},{0x2219, 0x2000},{0x2218, 0x0A14},{0x2219, 0x0000},
    {0x2218, 0x0A24},{0x2219, 0x0400},{0x2218, 0x0A34},{0x2219, 0xDA00},
    {0x2218, 0x0A44},{0x2219, 0x0600},{0x2218, 0x0A54},{0x2219, 0x7D00},
    {0x2218, 0x0A64},{0x2219, 0x4000},{0x2218, 0x0A74},{0x2219, 0x5F00},
    {0x2218, 0x0A84},{0x2219, 0x0400},{0x2218, 0x0A94},{0x2219, 0x0000},
    {0x2218, 0x0AA4},{0x2219, 0x0000},{0x2218, 0x0AB4},{0x2219, 0x7300},
    {0x2218, 0x0AC4},{0x2219, 0x0D00},{0x2218, 0x0AD4},{0x2219, 0x0300},
    {0x2218, 0x0AE4},{0x2219, 0x0400},{0x2218, 0x0AF4},{0x2219, 0xCE00},
    {0x2218, 0x0B04},{0x2219, 0x0900},{0x2218, 0x0B14},{0x2219, 0x9D00},
    {0x2218, 0x0B24},{0x2219, 0x0800},{0x2218, 0x0B34},{0x2219, 0x9000},
    {0x2218, 0x0B44},{0x2219, 0x0700},{0x2218, 0x0B54},{0x2219, 0x7900},
    {0x2218, 0x0B64},{0x2219, 0x4000},{0x2218, 0x0B74},{0x2219, 0x7F00},
    {0x2218, 0x0B84},{0x2219, 0x0400},{0x2218, 0x0B94},{0x2219, 0x0000},
    {0x2218, 0x0BA4},{0x2219, 0x0000},{0x2218, 0x0BB4},{0x2219, 0x0400},
    {0x2218, 0x0BC4},{0x2219, 0x7300},{0x2218, 0x0BD4},{0x2219, 0x0D00},
    {0x2218, 0x0BE4},{0x2219, 0x0100},{0x2218, 0x0BF4},{0x2219, 0x0900},
    {0x2218, 0x0C04},{0x2219, 0x8E00},{0x2218, 0x0C14},{0x2219, 0x0800},
    {0x2218, 0x0C24},{0x2219, 0x7D00},{0x2218, 0x0C34},{0x2219, 0x4000},
    {0x2218, 0x0C44},{0x2219, 0x7F00},{0x2218, 0x0C54},{0x2219, 0x0000},
    {0x2218, 0x0C64},{0x2219, 0x0000},{0x2218, 0x0C74},{0x2219, 0x0200},
    {0x2218, 0x0C84},{0x2219, 0x0000},{0x2218, 0x0C94},{0x2219, 0x7000},
    {0x2218, 0x0CA4},{0x2219, 0x0C00},{0x2218, 0x0CB4},{0x2219, 0x0100},
    {0x2218, 0x0CC4},{0x2219, 0x0900},{0x2218, 0x0CD4},{0x2219, 0x7F00},
    {0x2218, 0x0CE4},{0x2219, 0x4000},{0x2218, 0x0CF4},{0x2219, 0x7F00},
    {0x2218, 0x0D04},{0x2219, 0x3400},{0x2218, 0x0D14},{0x2219, 0x8300},
    {0x2218, 0x0D24},{0x2219, 0x8200},{0x2218, 0x0D34},{0x2219, 0x0000},
    {0x2218, 0x0D44},{0x2219, 0x7000},{0x2218, 0x0D54},{0x2219, 0x0D00},
    {0x2218, 0x0D64},{0x2219, 0x0100},{0x2218, 0x0D74},{0x2219, 0x0F00},
    {0x2218, 0x0D84},{0x2219, 0x7F00},{0x2218, 0x0D94},{0x2219, 0x9A00},
    {0x2218, 0x0DA4},{0x2219, 0x7D00},{0x2218, 0x0DB4},{0x2219, 0x4000},
    {0x2218, 0x0DC4},{0x2219, 0x7F00},{0x2218, 0x0DD4},{0x2219, 0x1400},
    {0x2218, 0x0DE4},{0x2219, 0x0000},{0x2218, 0x0DF4},{0x2219, 0x8200},
    {0x2218, 0x0E04},{0x2219, 0x0000},{0x2218, 0x0E14},{0x2219, 0x7000},
    {0x2218, 0x0E24},{0x2219, 0x0F00},{0x2218, 0x0E34},{0x2219, 0x0100},
    {0x2218, 0x0E44},{0x2219, 0x9B00},{0x2218, 0x0E54},{0x2219, 0x7F00},
    {0x2218, 0x0E64},{0x2219, 0x4000},{0x2218, 0x0E74},{0x2219, 0x1F00},
    {0x2218, 0x0E84},{0x2219, 0x0200},{0x2218, 0x0E94},{0x2219, 0x0600},
    {0x2218, 0x0EA4},{0x2219, 0x7100},{0x2218, 0x0EB4},{0x2219, 0x1D00},
    {0x2218, 0x0EC4},{0x2219, 0x0100},{0x2218, 0x0ED4},{0x2219, 0x4000},
    {0x2218, 0x0EE4},{0x2219, 0x1F00},{0x2218, 0x0EF4},{0x2219, 0x0200},
    {0x2218, 0x0F04},{0x2219, 0x0600},{0x2218, 0x0F14},{0x2219, 0x7100},
    {0x2218, 0x0F24},{0x2219, 0x0D00},{0x2218, 0x0F34},{0x2219, 0x0100},
    {0x2218, 0x0F44},{0x2219, 0x4000},{0x2218, 0x0F54},{0x2219, 0x1F00},
    {0x2218, 0x0F64},{0x2219, 0x0200},{0x2218, 0x0F74},{0x2219, 0x0600},
    {0x2218, 0x0F84},{0x2219, 0x7100},{0x2218, 0x0F94},{0x2219, 0x0D00},
    {0x2218, 0x0FA4},{0x2219, 0x0100},{0x2218, 0x0FB4},{0x2219, 0x4000},
    {0x2218, 0x0FC4},{0x2219, 0x1F00},{0x2218, 0x0FD4},{0x2219, 0x0200},
    {0x2218, 0x0FE4},{0x2219, 0x0600},{0x2218, 0x0FF4},{0x2219, 0x7100},
    {0x2218, 0x1004},{0x2219, 0x0D00},{0x2218, 0x1014},{0x2219, 0x0100},
    {0x2218, 0x1024},{0x2219, 0x4000},{0x2218, 0x1034},{0x2219, 0x1F00},
    {0x2218, 0x1044},{0x2219, 0x0200},{0x2218, 0x1054},{0x2219, 0x0600},
    {0x2218, 0x1064},{0x2219, 0x7100},{0x2218, 0x1074},{0x2219, 0x0D00},
    {0x2218, 0x1084},{0x2219, 0x0100},{0x2218, 0x1094},{0x2219, 0x4000},
    {0x2218, 0x10A4},{0x2219, 0x1F00},{0x2218, 0x10B4},{0x2219, 0x0200},
    {0x2218, 0x10C4},{0x2219, 0x0600},{0x2218, 0x10D4},{0x2219, 0x7100},
    {0x2218, 0x10E4},{0x2219, 0x0D00},{0x2218, 0x10F4},{0x2219, 0x0100},
    {0x2218, 0x1104},{0x2219, 0x4000},{0x2218, 0x1114},{0x2219, 0x7F00},
    {0x2218, 0x1124},{0x2219, 0x0400},{0x2218, 0x1134},{0x2219, 0x9000},
    {0x2218, 0x1144},{0x2219, 0x0200},{0x2218, 0x1154},{0x2219, 0x0600},
    {0x2218, 0x1164},{0x2219, 0x7300},{0x2218, 0x1174},{0x2219, 0x0D00},
    {0x2218, 0x1184},{0x2219, 0x0100},{0x2218, 0x1194},{0x2219, 0x0B00},
    {0x2218, 0x11A4},{0x2219, 0x9500},{0x2218, 0x11B4},{0x2219, 0x9400},
    {0x2218, 0x11C4},{0x2219, 0x0400},{0x2218, 0x11D4},{0x2219, 0x4000},
    {0x2218, 0x11E4},{0x2219, 0x4000},{0x2218, 0x11F4},{0x2219, 0x0500},
    {0x2218, 0x1204},{0x2219, 0x8000},{0x2218, 0x1214},{0x2219, 0x7800},
    {0x2218, 0x1224},{0x2219, 0x4000},{0x2218, 0x1234},{0x2219, 0x7F00},
    {0x2218, 0x1244},{0x2219, 0x0400},{0x2218, 0x1254},{0x2219, 0x0000},
    {0x2218, 0x1264},{0x2219, 0x0200},{0x2218, 0x1274},{0x2219, 0x0000},
    {0x2218, 0x1284},{0x2219, 0x7000},{0x2218, 0x1294},{0x2219, 0x0F00},
    {0x2218, 0x12A4},{0x2219, 0x0100},{0x2218, 0x12B4},{0x2219, 0x9B00},
    {0x2218, 0x12C4},{0x2219, 0x7F00},{0x2218, 0x12D4},{0x2219, 0xE100},
    {0x2218, 0x12E4},{0x2219, 0x1000},{0x2218, 0x12F4},{0x2219, 0x4000},
    {0x2218, 0x1304},{0x2219, 0x7F00},{0x2218, 0x1314},{0x2219, 0x0500},
    {0x2218, 0x1324},{0x2219, 0x0000},{0x2218, 0x1334},{0x2219, 0x0000},
    {0x2218, 0x1344},{0x2219, 0x0600},{0x2218, 0x1354},{0x2219, 0x7300},
    {0x2218, 0x1364},{0x2219, 0x0D00},{0x2218, 0x1374},{0x2219, 0x0100},
    {0x2218, 0x1384},{0x2219, 0x0400},{0x2218, 0x1394},{0x2219, 0x0600},
    {0x2218, 0x13A4},{0x2219, 0x4000},{0x2218, 0x13B4},{0x2219, 0x4000},
    {0x2218, 0x13C4},{0x2219, 0x0400},{0x2218, 0x13D4},{0x2219, 0xE000},
    {0x2218, 0x13E4},{0x2219, 0x7D00},{0x2218, 0x13F4},{0x2219, 0x0500},
    {0x2218, 0x1404},{0x2219, 0x7800},{0x2218, 0x1414},{0x2219, 0x4000},
    {0x2218, 0x1424},{0x2219, 0x4000},{0x2218, 0x1434},{0x2219, 0x0400},
    {0x2218, 0x1444},{0x2219, 0xE000},{0x2218, 0x1454},{0x2219, 0x9700},
    {0x2218, 0x1464},{0x2219, 0x4000},{0x2218, 0x1474},{0x2219, 0x7F00},
    {0x2218, 0x1484},{0x2219, 0x0000},{0x2218, 0x1494},{0x2219, 0x0100},
    {0x2218, 0x14A4},{0x2219, 0x4400},{0x2218, 0x14B4},{0x2219, 0x0000},
    {0x2218, 0x14C4},{0x2219, 0x0000},{0x2218, 0x14D4},{0x2219, 0x0000},
    {0x2218, 0x14E4},{0x2219, 0x4000},{0x2218, 0x14F4},{0x2219, 0x8000},
    {0x2218, 0x1504},{0x2219, 0x7F00},{0x2218, 0x1514},{0x2219, 0x4000},
    {0x2218, 0x1524},{0x2219, 0x3F00},{0x2218, 0x1534},{0x2219, 0x0400},
    {0x2218, 0x1544},{0x2219, 0x5000},{0x2218, 0x1554},{0x2219, 0xF800},
    {0x2218, 0x1564},{0x2219, 0x0000},{0x2218, 0x1574},{0x2219, 0xE000},
    {0x2218, 0x1584},{0x2219, 0x4000},{0x2218, 0x1594},{0x2219, 0x8000},
    {0x2218, 0x15A4},{0x2219, 0x7F00},{0x2218, 0x15B4},{0x2219, 0x8900},
    {0x2218, 0x15C4},{0x2219, 0x8300},{0x2218, 0x15D4},{0x2219, 0xE000},
    {0x2218, 0x15E4},{0x2219, 0x0000},{0x2218, 0x15F4},{0x2219, 0x4000},
    {0x2218, 0x1604},{0x2219, 0x7F00},{0x2218, 0x1614},{0x2219, 0x0200},
    {0x2218, 0x1624},{0x2219, 0x1000},{0x2218, 0x1634},{0x2219, 0x0000},
    {0x2218, 0x1644},{0x2219, 0xFC00},{0x2218, 0x1654},{0x2219, 0xFD00},
    {0x2218, 0x1664},{0x2219, 0x0000},{0x2218, 0x1674},{0x2219, 0x4000},
    {0x2218, 0x1684},{0x2219, 0xBC00},{0x2218, 0x1694},{0x2219, 0x0E00},
    {0x2218, 0x16A4},{0x2219, 0xFE00},{0x2218, 0x16B4},{0x2219, 0x8A00},
    {0x2218, 0x16C4},{0x2219, 0x8300},{0x2218, 0x16D4},{0x2219, 0xE000},
    {0x2218, 0x16E4},{0x2219, 0x0000},{0x2218, 0x16F4},{0x2219, 0x4000},
    {0x2218, 0x1704},{0x2219, 0x7F00},{0x2218, 0x1714},{0x2219, 0x0100},
    {0x2218, 0x1724},{0x2219, 0xFF00},{0x2218, 0x1734},{0x2219, 0x0000},
    {0x2218, 0x1744},{0x2219, 0xFC00},{0x2218, 0x1754},{0x2219, 0xFF00},
    {0x2218, 0x1764},{0x2219, 0x0000},{0x2218, 0x1774},{0x2219, 0x4000},
    {0x2218, 0x1784},{0x2219, 0x9D00},{0x2218, 0x1794},{0x2219, 0xFF00},
    {0x2218, 0x17A4},{0x2219, 0x4000},{0x2218, 0x17B4},{0x2219, 0x7F00},
    {0x2218, 0x17C4},{0x2219, 0x0000},{0x2218, 0x17D4},{0x2219, 0xFF00},
    {0x2218, 0x17E4},{0x2219, 0x0000},{0x2218, 0x17F4},{0x2219, 0xFC00},
    {0x2218, 0x1804},{0x2219, 0xFF00},{0x2218, 0x1814},{0x2219, 0x0000},
    {0x2218, 0x1824},{0x2219, 0x4000},{0x2218, 0x1834},{0x2219, 0x8900},
    {0x2218, 0x1844},{0x2219, 0x8300},{0x2218, 0x1854},{0x2219, 0xE000},
    {0x2218, 0x1864},{0x2219, 0x0000},{0x2218, 0x1874},{0x2219, 0xAC00},
    {0x2218, 0x1884},{0x2219, 0x0800},{0x2218, 0x1894},{0x2219, 0xFA00},
    {0x2218, 0x18A4},{0x2219, 0x4000},{0x2218, 0x18B4},{0x2219, 0x7F00},
    {0x2218, 0x18C4},{0x2219, 0x0400},{0x2218, 0x18D4},{0x2219, 0x0200},
    {0x2218, 0x18E4},{0x2219, 0x0000},{0x2218, 0x18F4},{0x2219, 0xFD00},
    {0x2218, 0x1904},{0x2219, 0x7F00},{0x2218, 0x1914},{0x2219, 0x0000},
    {0x2218, 0x1924},{0x2219, 0x4000},{0x2218, 0x1934},{0x2219, 0x0500},
    {0x2218, 0x1944},{0x2219, 0x9000},{0x2218, 0x1954},{0x2219, 0x8000},
    {0x2218, 0x1964},{0x2219, 0x7D00},{0x2218, 0x1974},{0x2219, 0x8C00},
    {0x2218, 0x1984},{0x2219, 0x8300},{0x2218, 0x1994},{0x2219, 0xE000},
    {0x2218, 0x19A4},{0x2219, 0x0000},{0x2218, 0x19B4},{0x2219, 0x4000},
    {0x2218, 0x19C4},{0x2219, 0x0400},{0x2218, 0x19D4},{0x2219, 0xFF00},
    {0x2218, 0x19E4},{0x2219, 0x0500},{0x2218, 0x19F4},{0x2219, 0x8500},
    {0x2218, 0x1A04},{0x2219, 0x8C00},{0x2218, 0x1A14},{0x2219, 0xFA00},
    {0x2218, 0x1A24},{0x2219, 0xE000},{0x2218, 0x1A34},{0x2219, 0x0000},
    {0x2218, 0x1A44},{0x2219, 0x4000},{0x2218, 0x1A54},{0x2219, 0x5F00},
    {0x2218, 0x1A64},{0x2219, 0x0400},{0x2218, 0x1A74},{0x2219, 0x0000},
    {0x2218, 0x1A84},{0x2219, 0xFC00},{0x2218, 0x1A94},{0x2219, 0x7300},
    {0x2218, 0x1AA4},{0x2219, 0x0D00},{0x2218, 0x1AB4},{0x2219, 0x0100},
    {0x2218, 0x1AC4},{0x2219, 0x4000},{0x2218, 0x1AD4},{0x2219, 0x2000},
    {0x2218, 0x1AE4},{0x2219, 0x0000},{0x2218, 0x1AF4},{0x2219, 0x0400},
    {0x2218, 0x1B04},{0x2219, 0xDA00},{0x2218, 0x1B14},{0x2219, 0x0600},
    {0x2218, 0x1B24},{0x2219, 0x7D00},{0x2218, 0x1B34},{0x2219, 0x4000},
    {0x2218, 0x1B44},{0x2219, 0x5F00},{0x2218, 0x1B54},{0x2219, 0x0400},
    {0x2218, 0x1B64},{0x2219, 0x0000},{0x2218, 0x1B74},{0x2219, 0x0000},
    {0x2218, 0x1B84},{0x2219, 0x7300},{0x2218, 0x1B94},{0x2219, 0x0D00},
    {0x2218, 0x1BA4},{0x2219, 0x0100},{0x2218, 0x1BB4},{0x2219, 0x0400},
    {0x2218, 0x1BC4},{0x2219, 0xCE00},{0x2218, 0x1BD4},{0x2219, 0x0800},
    {0x2218, 0x1BE4},{0x2219, 0x9200},{0x2218, 0x1BF4},{0x2219, 0x0900},
    {0x2218, 0x1C04},{0x2219, 0x9B00},{0x2218, 0x1C14},{0x2219, 0x0700},
    {0x2218, 0x1C24},{0x2219, 0x7900},{0x2218, 0x1C34},{0x2219, 0x4000},
    {0x2218, 0x1C44},{0x2219, 0x7F00},{0x2218, 0x1C54},{0x2219, 0x0400},
    {0x2218, 0x1C64},{0x2219, 0x0000},{0x2218, 0x1C74},{0x2219, 0x0000},
    {0x2218, 0x1C84},{0x2219, 0x0400},{0x2218, 0x1C94},{0x2219, 0x7300},
    {0x2218, 0x1CA4},{0x2219, 0x0D00},{0x2218, 0x1CB4},{0x2219, 0x0100},
    {0x2218, 0x1CC4},{0x2219, 0x0900},{0x2218, 0x1CD4},{0x2219, 0x8E00},
    {0x2218, 0x1CE4},{0x2219, 0x0800},{0x2218, 0x1CF4},{0x2219, 0x7D00},
    {0x2218, 0x1D04},{0x2219, 0x4000},{0x2218, 0x1D14},{0x2219, 0x7F00},
    {0x2218, 0x1D24},{0x2219, 0x0000},{0x2218, 0x1D34},{0x2219, 0x0000},
    {0x2218, 0x1D44},{0x2219, 0x0000},{0x2218, 0x1D54},{0x2219, 0x0000},
    {0x2218, 0x1D64},{0x2219, 0x7000},{0x2218, 0x1D74},{0x2219, 0x0C00},
    {0x2218, 0x1D84},{0x2219, 0x0100},{0x2218, 0x1D94},{0x2219, 0x0900},
    {0x2218, 0x1DA4},{0x2219, 0x7F00},{0x2218, 0x1DB4},{0x2219, 0x4000},
    {0x2218, 0x1DC4},{0x2219, 0x7F00},{0x2218, 0x1DD4},{0x2219, 0x0400},
    {0x2218, 0x1DE4},{0x2219, 0x0000},{0x2218, 0x1DF4},{0x2219, 0x0000},
    {0x2218, 0x1E04},{0x2219, 0x0000},{0x2218, 0x1E14},{0x2219, 0x7000},
    {0x2218, 0x1E24},{0x2219, 0x0D00},{0x2218, 0x1E34},{0x2219, 0x0100},
    {0x2218, 0x1E44},{0x2219, 0x0B00},{0x2218, 0x1E54},{0x2219, 0x7F00},
    {0x2218, 0x1E64},{0x2219, 0x9A00},{0x2218, 0x1E74},{0x2219, 0x7F00},
    {0x2218, 0x1E84},{0x2219, 0x4000},{0x2218, 0x1E94},{0x2219, 0x7F00},
    {0x2218, 0x1EA4},{0x2219, 0x0400},{0x2218, 0x1EB4},{0x2219, 0x0000},
    {0x2218, 0x1EC4},{0x2219, 0x0000},{0x2218, 0x1ED4},{0x2219, 0x0000},
    {0x2218, 0x1EE4},{0x2219, 0x7100},{0x2218, 0x1EF4},{0x2219, 0x0D00},
    {0x2218, 0x1F04},{0x2219, 0x0100},{0x2218, 0x1F14},{0x2219, 0x9400},
    {0x2218, 0x1F24},{0x2219, 0x7F00},{0x2218, 0x1F34},{0x2219, 0x4000},
    {0x2218, 0x1F44},{0x2219, 0x7F00},{0x2218, 0x1F54},{0x2219, 0x0500},
    {0x2218, 0x1F64},{0x2219, 0x0000},{0x2218, 0x1F74},{0x2219, 0x0000},
    {0x2218, 0x1F84},{0x2219, 0x0000},{0x2218, 0x1F94},{0x2219, 0x7300},
    {0x2218, 0x1FA4},{0x2219, 0x0D00},{0x2218, 0x1FB4},{0x2219, 0x0100},
    {0x2218, 0x1FC4},{0x2219, 0x0500},{0x2218, 0x1FD4},{0x2219, 0x8800},
    {0x2218, 0x1FE4},{0x2219, 0x0400},{0x2218, 0x1FF4},{0x2219, 0x7D00},
    {0x2218, 0x2004},{0x2219, 0x4000},{0x2218, 0x2014},{0x2219, 0x4000},
    {0x2218, 0x2024},{0x2219, 0x0400},{0x2218, 0x2034},{0x2219, 0xE100},
    {0x2218, 0x2044},{0x2219, 0x8A00},{0x2218, 0x2054},{0x2219, 0x4000},
    {0x2218, 0x2064},{0x2219, 0x4000},{0x2218, 0x2074},{0x2219, 0x0400},
    {0x2218, 0x2084},{0x2219, 0xE100},{0x2218, 0x2094},{0x2219, 0xA400},
    {0x2218, 0x20A4},{0x2219, 0x4000},{0x2218, 0x20B4},{0x2219, 0x7F00},
    {0x2218, 0x20C4},{0x2219, 0x0000},{0x2218, 0x20D4},{0x2219, 0x0100},
    {0x2218, 0x20E4},{0x2219, 0x4000},{0x2218, 0x20F4},{0x2219, 0x3E00},
    {0x2218, 0x2104},{0x2219, 0x0000},{0x2218, 0x2114},{0x2219, 0xE000},
    {0x2218, 0x2124},{0x2219, 0x1200},{0x2218, 0x2134},{0x2219, 0x8000},
    {0x2218, 0x2144},{0x2219, 0x7F00},{0x2218, 0x2154},{0x2219, 0x8900},
    {0x2218, 0x2164},{0x2219, 0x8300},{0x2218, 0x2174},{0x2219, 0xE000},
    {0x2218, 0x2184},{0x2219, 0x0000},{0x2218, 0x2194},{0x2219, 0x4000},
    {0x2218, 0x21A4},{0x2219, 0x7F00},{0x2218, 0x21B4},{0x2219, 0x0000},
    {0x2218, 0x21C4},{0x2219, 0x2000},{0x2218, 0x21D4},{0x2219, 0x4000},
    {0x2218, 0x21E4},{0x2219, 0x3E00},{0x2218, 0x21F4},{0x2219, 0xFF00},
    {0x2218, 0x2204},{0x2219, 0x0000},{0x2218, 0x2214},{0x2219, 0x1200},
    {0x2218, 0x2224},{0x2219, 0x8000},{0x2218, 0x2234},{0x2219, 0x7F00},
    {0x2218, 0x2244},{0x2219, 0x8600},{0x2218, 0x2254},{0x2219, 0x8500},
    {0x2218, 0x2264},{0x2219, 0x8900},{0x2218, 0x2274},{0x2219, 0xFD00},
    {0x2218, 0x2284},{0x2219, 0xE000},{0x2218, 0x2294},{0x2219, 0x0000},
    {0x2218, 0x22A4},{0x2219, 0x9500},{0x2218, 0x22B4},{0x2219, 0x0400},
    {0x2218, 0x22C4},{0x2219, 0x4000},{0x2218, 0x22D4},{0x2219, 0x4000},
    {0x2218, 0x22E4},{0x2219, 0x1000},{0x2218, 0x22F4},{0x2219, 0x4000},
    {0x2218, 0x2304},{0x2219, 0x3F00},{0x2218, 0x2314},{0x2219, 0x0200},
    {0x2218, 0x2324},{0x2219, 0x4000},{0x2218, 0x2334},{0x2219, 0x3700},
    {0x2218, 0x2344},{0x2219, 0x7F00},{0x2218, 0x2354},{0x2219, 0x0000},
    {0x2218, 0x2364},{0x2219, 0x0200},{0x2218, 0x2374},{0x2219, 0x0200},
    {0x2218, 0x2384},{0x2219, 0x9000},{0x2218, 0x2394},{0x2219, 0x8000},
    {0x2218, 0x23A4},{0x2219, 0x7D00},{0x2218, 0x23B4},{0x2219, 0x8900},
    {0x2218, 0x23C4},{0x2219, 0x8300},{0x2218, 0x23D4},{0x2219, 0xE000},
    {0x2218, 0x23E4},{0x2219, 0x0000},{0x2218, 0x23F4},{0x2219, 0x4000},
    {0x2218, 0x2404},{0x2219, 0x0400},{0x2218, 0x2414},{0x2219, 0xFF00},
    {0x2218, 0x2424},{0x2219, 0x0200},{0x2218, 0x2434},{0x2219, 0x8500},
    {0x2218, 0x2444},{0x2219, 0x8900},{0x2218, 0x2454},{0x2219, 0xFA00},
    {0x2218, 0x2464},{0x2219, 0xE000},{0x2218, 0x2474},{0x2219, 0x0000},
    {0x2218, 0x2484},{0x2219, 0x4000},{0x2218, 0x2494},{0x2219, 0x7F00},
    {0x2218, 0x24A4},{0x2219, 0x0000},{0x2218, 0x24B4},{0x2219, 0x0000},
    {0x2218, 0x24C4},{0x2219, 0x4000},{0x2218, 0x24D4},{0x2219, 0x3700},
    {0x2218, 0x24E4},{0x2219, 0x7300},{0x2218, 0x24F4},{0x2219, 0x0500},
    {0x2218, 0x2504},{0x2219, 0x0200},{0x2218, 0x2514},{0x2219, 0x0100},
    {0x2218, 0x2524},{0x2219, 0xD800},{0x2218, 0x2534},{0x2219, 0x0400},
    {0x2218, 0x2544},{0x2219, 0x7D00},{0x2218, 0x2554},{0x2219, 0x4000},
    {0x2218, 0x2564},{0x2219, 0x7F00},{0x2218, 0x2574},{0x2219, 0x0000},
    {0x2218, 0x2584},{0x2219, 0x0000},{0x2218, 0x2594},{0x2219, 0x4000},
    {0x2218, 0x25A4},{0x2219, 0x0000},{0x2218, 0x25B4},{0x2219, 0x7200},
    {0x2218, 0x25C4},{0x2219, 0x0400},{0x2218, 0x25D4},{0x2219, 0x0000},
    {0x2218, 0x25E4},{0x2219, 0x0800},{0x2218, 0x25F4},{0x2219, 0x7F00},
    {0x2218, 0x2604},{0x2219, 0x4000},{0x2218, 0x2614},{0x2219, 0x7F00},
    {0x2218, 0x2624},{0x2219, 0x0000},{0x2218, 0x2634},{0x2219, 0x0000},
    {0x2218, 0x2644},{0x2219, 0xC000},{0x2218, 0x2654},{0x2219, 0x0000},
    {0x2218, 0x2664},{0x2219, 0x7200},{0x2218, 0x2674},{0x2219, 0x0500},
    {0x2218, 0x2684},{0x2219, 0x0000},{0x2218, 0x2694},{0x2219, 0x0400},
    {0x2218, 0x26A4},{0x2219, 0xEB00},{0x2218, 0x26B4},{0x2219, 0x8400},
    {0x2218, 0x26C4},{0x2219, 0x7D00},{0x2218, 0x26D4},{0x2219, 0x4000},
    {0x2218, 0x26E4},{0x2219, 0x7F00},{0x2218, 0x26F4},{0x2219, 0x0000},
    {0x2218, 0x2704},{0x2219, 0x0000},{0x2218, 0x2714},{0x2219, 0x4000},
    {0x2218, 0x2724},{0x2219, 0x0000},{0x2218, 0x2734},{0x2219, 0x7200},
    {0x2218, 0x2744},{0x2219, 0x0700},{0x2218, 0x2754},{0x2219, 0x0000},
    {0x2218, 0x2764},{0x2219, 0x0400},{0x2218, 0x2774},{0x2219, 0xDE00},
    {0x2218, 0x2784},{0x2219, 0x9B00},{0x2218, 0x2794},{0x2219, 0x7D00},
    {0x2218, 0x27A4},{0x2219, 0x4000},{0x2218, 0x27B4},{0x2219, 0x7F00},
    {0x2218, 0x27C4},{0x2219, 0x0000},{0x2218, 0x27D4},{0x2219, 0x9000},
    {0x2218, 0x27E4},{0x2219, 0x4000},{0x2218, 0x27F4},{0x2219, 0x0400},
    {0x2218, 0x2804},{0x2219, 0x7300},{0x2218, 0x2814},{0x2219, 0x1500},
    {0x2218, 0x2824},{0x2219, 0x0000},{0x2218, 0x2834},{0x2219, 0x0400},
    {0x2218, 0x2844},{0x2219, 0xD100},{0x2218, 0x2854},{0x2219, 0x9400},
    {0x2218, 0x2864},{0x2219, 0x9200},{0x2218, 0x2874},{0x2219, 0x8000},
    {0x2218, 0x2884},{0x2219, 0x7B00},{0x2218, 0x2894},{0x2219, 0x4000},
    {0x2218, 0x28A4},{0x2219, 0x7F00},{0x2218, 0x28B4},{0x2219, 0x0000},
    {0x2218, 0x28C4},{0x2219, 0x0000},{0x2218, 0x28D4},{0x2219, 0x4000},
    {0x2218, 0x28E4},{0x2219, 0x0000},{0x2218, 0x28F4},{0x2219, 0x7200},
    {0x2218, 0x2904},{0x2219, 0x0700},{0x2218, 0x2914},{0x2219, 0x0000},
    {0x2218, 0x2924},{0x2219, 0x0400},{0x2218, 0x2934},{0x2219, 0xC200},
    {0x2218, 0x2944},{0x2219, 0x9B00},{0x2218, 0x2954},{0x2219, 0x7D00},
    {0x2218, 0x2964},{0x2219, 0xE200},{0x2218, 0x2974},{0x2219, 0x7A00},
    {0x2218, 0x2984},{0x2219, 0x4000},{0x2218, 0x2994},{0x2219, 0x7F00},
    {0x2218, 0x29A4},{0x2219, 0x0000},{0x2218, 0x29B4},{0x2219, 0x0000},
    {0x2218, 0x29C4},{0x2219, 0x4000},{0x2218, 0x29D4},{0x2219, 0x3700},
    {0x2218, 0x29E4},{0x2219, 0x7300},{0x2218, 0x29F4},{0x2219, 0x0500},
    {0x2218, 0x2A04},{0x2219, 0x0000},{0x2218, 0x2A14},{0x2219, 0x0100},
    {0x2218, 0x2A24},{0x2219, 0x0300},{0x2218, 0x2A34},{0x2219, 0xE200},
    {0x2218, 0x2A44},{0x2219, 0x2A00},{0x2218, 0x2A54},{0x2219, 0x0200},
    {0x2218, 0x2A64},{0x2219, 0x7B00},{0x2218, 0x2A74},{0x2219, 0xE200},
    {0x2218, 0x2A84},{0x2219, 0x4800},{0x221F, 0x0000},{0x2217, 0x2100},
    {0x221F, 0x0007},{0x221E, 0x0040},{0x2218, 0x0000},{0x221F, 0x0000},
    {0x2217, 0x2160},{0x221F, 0x0001},{0x2210, 0xF25E},{0x221F, 0x0007},
    {0x221E, 0x0042},{0x2215, 0x0F00},{0x2215, 0x0F00},{0x2216, 0x7408},
    {0x2215, 0x0E00},{0x2215, 0x0F00},{0x2215, 0x0F01},{0x2216, 0x4000},
    {0x2215, 0x0E01},{0x2215, 0x0F01},{0x2215, 0x0F02},{0x2216, 0x9400},
    {0x2215, 0x0E02},{0x2215, 0x0F02},{0x2215, 0x0F03},{0x2216, 0x7408},
    {0x2215, 0x0E03},{0x2215, 0x0F03},{0x2215, 0x0F04},{0x2216, 0x4008},
    {0x2215, 0x0E04},{0x2215, 0x0F04},{0x2215, 0x0F05},{0x2216, 0x9400},
    {0x2215, 0x0E05},{0x2215, 0x0F05},{0x2215, 0x0F06},{0x2216, 0x0803},
    {0x2215, 0x0E06},{0x2215, 0x0F06},{0x2215, 0x0D00},{0x2215, 0x0100},
    {0x221F, 0x0001},{0x2210, 0xF05E},{0x221F, 0x0000},{0x2217, 0x2100},
    {0x221F, 0x0007},{0x221E, 0x0023},{0x2216, 0x0006},{0x2215, 0x0000},
    {0x2219, 0x007F},{0x2215, 0x0001},{0x2219, 0x0000},{0x2215, 0x0002},
    {0x2219, 0x0000},{0x2215, 0x0003},{0x2219, 0x0000},{0x2215, 0x0004},
    {0x2219, 0x0000},{0x2215, 0x0005},{0x2219, 0x0000},{0x2215, 0x0006},
    {0x2219, 0x0000},{0x2215, 0x0007},{0x2219, 0x00B9},{0x2215, 0x0008},
    {0x2219, 0x0064},{0x2215, 0x0009},{0x2219, 0x0018},{0x2215, 0x000A},
    {0x2219, 0x00B8},{0x2215, 0x000B},{0x2219, 0x005C},{0x2215, 0x000C},
    {0x2219, 0x0007},{0x2215, 0x000D},{0x2219, 0x0088},{0x2215, 0x000E},
    {0x2219, 0x0079},{0x2215, 0x000F},{0x2219, 0x008C},{0x2215, 0x0010},
    {0x2219, 0x0091},{0x2215, 0x0011},{0x2219, 0x008B},{0x2215, 0x0012},
    {0x2219, 0x0075},{0x2215, 0x0013},{0x2219, 0x00E0},{0x2215, 0x0014},
    {0x2219, 0x008D},{0x2215, 0x0015},{0x2219, 0x006A},{0x2215, 0x0016},
    {0x2219, 0x0006},{0x2215, 0x0017},{0x2219, 0x0029},{0x2215, 0x0018},
    {0x2219, 0x0014},{0x2215, 0x0019},{0x2219, 0x0080},{0x2215, 0x001A},
    {0x2219, 0x0017},{0x2215, 0x001B},{0x2219, 0x00BB},{0x2215, 0x001C},
    {0x2219, 0x0080},{0x2215, 0x001D},{0x2219, 0x0094},{0x2215, 0x001E},
    {0x2219, 0x00BB},{0x2215, 0x001F},{0x2219, 0x0003},{0x2215, 0x0020},
    {0x2219, 0x00F9},{0x2215, 0x0021},{0x2219, 0x0042},{0x2215, 0x0022},
    {0x2219, 0x0002},{0x2215, 0x0023},{0x2219, 0x005D},{0x2215, 0x0024},
    {0x2219, 0x0080},{0x2215, 0x0025},{0x2219, 0x0001},{0x2215, 0x0026},
    {0x2219, 0x0000},{0x2215, 0x0027},{0x2219, 0x0000},{0x2215, 0x0028},
    {0x2219, 0x008C},{0x2215, 0x0029},{0x2219, 0x0057},{0x2215, 0x002A},
    {0x2219, 0x0099},{0x2215, 0x002B},{0x2219, 0x0004},{0x2215, 0x002C},
    {0x2219, 0x008E},{0x2215, 0x002D},{0x2219, 0x007B},{0x2215, 0x002E},
    {0x2219, 0x00C3},{0x2215, 0x002F},{0x2219, 0x0091},{0x2215, 0x0030},
    {0x2219, 0x0078},{0x2215, 0x0031},{0x2219, 0x006B},{0x2215, 0x0032},
    {0x2219, 0x00C4},{0x2215, 0x0033},{0x2219, 0x0021},{0x2215, 0x0034},
    {0x2219, 0x0004},{0x2215, 0x0035},{0x2219, 0x0001},{0x2215, 0x0036},
    {0x2219, 0x0099},{0x2215, 0x0037},{0x2219, 0x0086},{0x2215, 0x0038},
    {0x2219, 0x009D},{0x2215, 0x0039},{0x2219, 0x0092},{0x2215, 0x003A},
    {0x2219, 0x0091},{0x2215, 0x003B},{0x2219, 0x0066},{0x2215, 0x003C},
    {0x2219, 0x00DC},{0x2215, 0x003D},{0x2219, 0x0080},{0x2215, 0x003E},
    {0x2219, 0x000D},{0x2215, 0x003F},{0x2219, 0x00B0},{0x2215, 0x0040},
    {0x2219, 0x0004},{0x2215, 0x0041},{0x2219, 0x0054},{0x2215, 0x0042},
    {0x2219, 0x009D},{0x2215, 0x0043},{0x2219, 0x007A},{0x2215, 0x0044},
    {0x2219, 0x004B},{0x2215, 0x0045},{0x2219, 0x0001},{0x2215, 0x0046},
    {0x2219, 0x001E},{0x2215, 0x0047},{0x2219, 0x0011},{0x2215, 0x0048},
    {0x2219, 0x008C},{0x2215, 0x0049},{0x2219, 0x0058},{0x2215, 0x004A},
    {0x2219, 0x00DE},{0x2215, 0x004B},{0x2219, 0x006B},{0x2215, 0x004C},
    {0x2219, 0x0013},{0x2215, 0x004D},{0x2219, 0x0041},{0x2215, 0x004E},
    {0x2219, 0x0008},{0x2215, 0x004F},{0x2219, 0x00A5},{0x2215, 0x0050},
    {0x2219, 0x0099},{0x2215, 0x0051},{0x2219, 0x0007},{0x2215, 0x0052},
    {0x2219, 0x00BE},{0x2215, 0x0053},{0x2219, 0x0002},{0x2215, 0x0054},
    {0x2219, 0x0070},{0x2215, 0x0055},{0x2219, 0x0080},{0x2215, 0x0056},
    {0x2219, 0x007C},{0x2215, 0x0057},{0x2219, 0x00C4},{0x2215, 0x0058},
    {0x2219, 0x00B1},{0x2215, 0x0059},{0x2219, 0x00F2},{0x2215, 0x005A},
    {0x2219, 0x007E},{0x2215, 0x005B},{0x2219, 0x006B},{0x2215, 0x005C},
    {0x2219, 0x0068},{0x2215, 0x005D},{0x2219, 0x0051},{0x2215, 0x005E},
    {0x2219, 0x0011},{0x2215, 0x005F},{0x2219, 0x0049},{0x2215, 0x0060},
    {0x2219, 0x0069},{0x2215, 0x0061},{0x2219, 0x009C},{0x2215, 0x0062},
    {0x2219, 0x0055},{0x2215, 0x0063},{0x2219, 0x0009},{0x2215, 0x0064},
    {0x2219, 0x009C},{0x2215, 0x0065},{0x2219, 0x008F},{0x2215, 0x0066},
    {0x2219, 0x009A},{0x2215, 0x0067},{0x2219, 0x00DD},{0x2215, 0x0068},
    {0x2219, 0x0099},{0x2215, 0x0069},{0x2219, 0x0012},{0x2215, 0x006A},
    {0x2219, 0x0080},{0x2215, 0x006B},{0x2219, 0x0060},{0x2215, 0x006C},
    {0x2219, 0x0090},{0x2215, 0x006D},{0x2219, 0x0092},{0x2215, 0x006E},
    {0x2219, 0x008F},{0x2215, 0x006F},{0x2219, 0x0075},{0x2215, 0x0070},
    {0x2219, 0x0041},{0x2215, 0x0071},{0x2219, 0x0029},{0x2215, 0x0072},
    {0x2219, 0x00E0},{0x2215, 0x0073},{0x2219, 0x0031},{0x2215, 0x0074},
    {0x2219, 0x0044},{0x2215, 0x0075},{0x2219, 0x0040},{0x2215, 0x0076},
    {0x2219, 0x004C},{0x2215, 0x0077},{0x2219, 0x0045},{0x2215, 0x0078},
    {0x2219, 0x0000},{0x2215, 0x0079},{0x2219, 0x00E0},{0x2215, 0x007A},
    {0x2219, 0x0064},{0x2215, 0x007B},{0x2219, 0x0090},{0x2215, 0x007C},
    {0x2219, 0x007F},{0x2215, 0x007D},{0x2219, 0x0091},{0x2215, 0x007E},
    {0x2219, 0x00F2},{0x2215, 0x007F},{0x2219, 0x00E0},{0x2215, 0x0080},
    {0x2219, 0x0021},{0x2215, 0x0081},{0x2219, 0x006A},{0x2215, 0x0082},
    {0x2219, 0x000A},{0x2215, 0x0083},{0x2219, 0x0001},{0x2215, 0x0084},
    {0x2219, 0x0028},{0x2215, 0x0085},{0x2219, 0x0048},{0x2215, 0x0086},
    {0x2219, 0x0009},{0x2215, 0x0087},{0x2219, 0x009B},{0x2215, 0x0088},
    {0x2219, 0x007F},{0x2215, 0x0089},{0x2219, 0x0042},{0x2215, 0x008A},
    {0x2219, 0x0088},{0x2215, 0x008B},{0x2219, 0x0044},{0x2215, 0x008C},
    {0x2219, 0x0024},{0x2215, 0x008D},{0x2219, 0x0044},{0x2215, 0x008E},
    {0x2219, 0x0020},{0x2215, 0x008F},{0x2219, 0x006B},{0x2215, 0x0090},
    {0x2219, 0x0010},{0x2215, 0x0091},{0x2219, 0x0011},{0x2215, 0x0092},
    {0x2219, 0x0088},{0x2215, 0x0093},{0x2219, 0x00FF},{0x2215, 0x0094},
    {0x2219, 0x008B},{0x2215, 0x0095},{0x2219, 0x0020},{0x2215, 0x0096},
    {0x2219, 0x0098},{0x2215, 0x0097},{0x2219, 0x0085},{0x2215, 0x0098},
    {0x2219, 0x00B1},{0x2215, 0x0099},{0x2219, 0x0034},{0x2215, 0x009A},
    {0x2219, 0x001D},{0x2215, 0x009B},{0x2219, 0x00D9},{0x2215, 0x009C},
    {0x2219, 0x0082},{0x2215, 0x009D},{0x2219, 0x000F},{0x2215, 0x009E},
    {0x2219, 0x008A},{0x2215, 0x009F},{0x2219, 0x0004},{0x2215, 0x00A0},
    {0x2219, 0x0050},{0x2215, 0x00A1},{0x2219, 0x0081},{0x2215, 0x00A2},
    {0x2219, 0x00C3},{0x2215, 0x00A3},{0x2219, 0x0050},{0x2215, 0x00A4},
    {0x2219, 0x0081},{0x2215, 0x00A5},{0x2219, 0x00BC},{0x2215, 0x00A6},
    {0x2219, 0x001B},{0x2215, 0x00A7},{0x2219, 0x00ED},{0x2215, 0x00A8},
    {0x2219, 0x00A4},{0x2215, 0x00A9},{0x2219, 0x002B},{0x2215, 0x00AA},
    {0x2219, 0x00EA},{0x2215, 0x00AB},{0x2219, 0x00CC},{0x2215, 0x00AC},
    {0x2219, 0x008A},{0x2215, 0x00AD},{0x2219, 0x0004},{0x2215, 0x00AE},
    {0x2219, 0x0050},{0x2215, 0x00AF},{0x2219, 0x0080},{0x2215, 0x00B0},
    {0x2219, 0x00C3},{0x2215, 0x00B1},{0x2219, 0x0050},{0x2215, 0x00B2},
    {0x2219, 0x0080},{0x2215, 0x00B3},{0x2219, 0x00E0},{0x2215, 0x00B4},
    {0x2219, 0x0094},{0x2215, 0x00B5},{0x2219, 0x00E0},{0x2215, 0x00B6},
    {0x2219, 0x0000},{0x2215, 0x00B7},{0x2219, 0x0044},{0x2215, 0x00B8},
    {0x2219, 0x0021},{0x2215, 0x00B9},{0x2219, 0x0054},{0x2215, 0x00BA},
    {0x2219, 0x0081},{0x2215, 0x00BB},{0x2219, 0x0020},{0x2215, 0x00BC},
    {0x2219, 0x0062},{0x2215, 0x00BD},{0x2219, 0x003B},{0x2215, 0x00BE},
    {0x2219, 0x002B},{0x2215, 0x00BF},{0x2219, 0x0086},{0x2215, 0x00C0},
    {0x2219, 0x0083},{0x2215, 0x00C1},{0x2219, 0x0060},{0x2215, 0x00C2},
    {0x2219, 0x002F},{0x2215, 0x00C3},{0x2219, 0x0048},{0x2215, 0x00C4},
    {0x2219, 0x0081},{0x2215, 0x00C5},{0x2219, 0x0068},{0x2215, 0x00C6},
    {0x2219, 0x0083},{0x2215, 0x00C7},{0x2219, 0x0085},{0x2215, 0x00C8},
    {0x2219, 0x0068},{0x2215, 0x00C9},{0x2219, 0x004E},{0x2215, 0x00CA},
    {0x2219, 0x008D},{0x2215, 0x00CB},{0x2219, 0x0098},{0x2215, 0x00CC},
    {0x2219, 0x008A},{0x2215, 0x00CD},{0x2219, 0x0091},{0x2215, 0x00CE},
    {0x2219, 0x002D},{0x2215, 0x00CF},{0x2219, 0x009F},{0x2215, 0x00D0},
    {0x2219, 0x00AD},{0x2215, 0x00D1},{0x2219, 0x009B},{0x2215, 0x00D2},
    {0x2219, 0x0003},{0x2215, 0x00D3},{0x2219, 0x0050},{0x2215, 0x00D4},
    {0x2219, 0x0000},{0x2215, 0x00D5},{0x2219, 0x00D6},{0x2215, 0x00D6},
    {0x2219, 0x00BB},{0x2215, 0x00D7},{0x2219, 0x0057},{0x2215, 0x00D8},
    {0x2219, 0x0009},{0x2215, 0x00D9},{0x2219, 0x0082},{0x2215, 0x00DA},
    {0x2219, 0x0004},{0x2215, 0x00DB},{0x2219, 0x0050},{0x2215, 0x00DC},
    {0x2219, 0x0081},{0x2215, 0x00DD},{0x2219, 0x00CB},{0x2215, 0x00DE},
    {0x2219, 0x0050},{0x2215, 0x00DF},{0x2219, 0x0080},{0x2215, 0x00E0},
    {0x2219, 0x00C8},{0x2215, 0x00E1},{0x2219, 0x0082},{0x2215, 0x00E2},
    {0x2219, 0x0004},{0x2215, 0x00E3},{0x2219, 0x0050},{0x2215, 0x00E4},
    {0x2219, 0x0081},{0x2215, 0x00E5},{0x2219, 0x00C3},{0x2215, 0x00E6},
    {0x2219, 0x0050},{0x2215, 0x00E7},{0x2219, 0x0080},{0x2215, 0x00E8},
    {0x2219, 0x00BB},{0x2215, 0x00E9},{0x2219, 0x004E},{0x2215, 0x00EA},
    {0x2219, 0x0083},{0x2215, 0x00EB},{0x2219, 0x0048},{0x2215, 0x00EC},
    {0x2219, 0x0085},{0x2215, 0x00ED},{0x2219, 0x00A9},{0x2215, 0x00EE},
    {0x2219, 0x00D0},{0x2215, 0x00EF},{0x2219, 0x00DC},{0x2215, 0x00F0},
    {0x2219, 0x0082},{0x2215, 0x00F1},{0x2219, 0x000A},{0x2215, 0x00F2},
    {0x2219, 0x00B7},{0x2215, 0x00F3},{0x2219, 0x0018},{0x2215, 0x00F4},
    {0x2219, 0x0087},{0x2215, 0x00F5},{0x2219, 0x00BF},{0x2215, 0x00F6},
    {0x2219, 0x001C},{0x2215, 0x00F7},{0x2219, 0x0006},{0x2215, 0x00F8},
    {0x2219, 0x0097},{0x2215, 0x00F9},{0x2219, 0x0052},{0x2215, 0x00FA},
    {0x2219, 0x00C3},{0x2215, 0x00FB},{0x2219, 0x00E0},{0x2215, 0x00FC},
    {0x2219, 0x0081},{0x2215, 0x00FD},{0x2219, 0x006A},{0x2215, 0x00FE},
    {0x2219, 0x0032},{0x2215, 0x00FF},{0x2219, 0x0011},{0x2215, 0x0100},
    {0x2219, 0x002D},{0x2215, 0x0101},{0x2219, 0x0098},{0x2215, 0x0102},
    {0x2219, 0x0085},{0x2215, 0x0103},{0x2219, 0x0091},{0x2215, 0x0104},
    {0x2219, 0x0077},{0x2215, 0x0105},{0x2219, 0x00E1},{0x2215, 0x0106},
    {0x2219, 0x0046},{0x2215, 0x0107},{0x2219, 0x0068},{0x2215, 0x0108},
    {0x2219, 0x0006},{0x2215, 0x0109},{0x2219, 0x0019},{0x2215, 0x010A},
    {0x2219, 0x008A},{0x2215, 0x010B},{0x2219, 0x0004},{0x2215, 0x010C},
    {0x2219, 0x0050},{0x2215, 0x010D},{0x2219, 0x0083},{0x2215, 0x010E},
    {0x2219, 0x00C3},{0x2215, 0x010F},{0x2219, 0x0050},{0x2215, 0x0110},
    {0x2219, 0x0083},{0x2215, 0x0111},{0x2219, 0x009B},{0x2215, 0x0112},
    {0x2219, 0x0012},{0x2215, 0x0113},{0x2219, 0x0082},{0x2215, 0x0114},
    {0x2219, 0x0009},{0x2215, 0x0115},{0x2219, 0x008A},{0x2215, 0x0116},
    {0x2219, 0x0004},{0x2215, 0x0117},{0x2219, 0x0050},{0x2215, 0x0118},
    {0x2219, 0x0083},{0x2215, 0x0119},{0x2219, 0x00CB},{0x2215, 0x011A},
    {0x2219, 0x0050},{0x2215, 0x011B},{0x2219, 0x0083},{0x2215, 0x011C},
    {0x2219, 0x00C8},{0x2215, 0x011D},{0x2219, 0x008A},{0x2215, 0x011E},
    {0x2219, 0x0004},{0x2215, 0x011F},{0x2219, 0x0050},{0x2215, 0x0120},
    {0x2219, 0x0082},{0x2215, 0x0121},{0x2219, 0x00C3},{0x2215, 0x0122},
    {0x2219, 0x0050},{0x2215, 0x0123},{0x2219, 0x0082},{0x2215, 0x0124},
    {0x2219, 0x00B7},{0x2215, 0x0125},{0x2219, 0x0018},{0x2215, 0x0126},
    {0x2219, 0x0090},{0x2215, 0x0127},{0x2219, 0x009C},{0x2215, 0x0128},
    {0x2219, 0x0090},{0x2215, 0x0129},{0x2219, 0x0081},{0x2215, 0x012A},
    {0x2219, 0x0002},{0x2215, 0x012B},{0x2219, 0x00C4},{0x2215, 0x012C},
    {0x2219, 0x0082},{0x2215, 0x012D},{0x2219, 0x0099},{0x2215, 0x012E},
    {0x2219, 0x00C6},{0x2215, 0x012F},{0x2219, 0x0097},{0x2215, 0x0130},
    {0x2219, 0x0061},{0x2215, 0x0131},{0x2219, 0x00A2},{0x2215, 0x0132},
    {0x2219, 0x001C},{0x2215, 0x0133},{0x2219, 0x005E},{0x2215, 0x0134},
    {0x2219, 0x00E1},{0x2215, 0x0135},{0x2219, 0x0069},{0x2215, 0x0136},
    {0x2219, 0x00E0},{0x2215, 0x0137},{0x2219, 0x0081},{0x2215, 0x0138},
    {0x2219, 0x006A},{0x2215, 0x0139},{0x2219, 0x002B},{0x2215, 0x013A},
    {0x2219, 0x0021},{0x2215, 0x013B},{0x2219, 0x004F},{0x2215, 0x013C},
    {0x2219, 0x009D},{0x2215, 0x013D},{0x2219, 0x0087},{0x2215, 0x013E},
    {0x2219, 0x00A2},{0x2215, 0x013F},{0x2219, 0x001B},{0x2215, 0x0140},
    {0x2219, 0x00A9},{0x2215, 0x0141},{0x2219, 0x0081},{0x2215, 0x0142},
    {0x2219, 0x0004},{0x2215, 0x0143},{0x2219, 0x00D9},{0x2215, 0x0144},
    {0x2219, 0x00E1},{0x2215, 0x0145},{0x2219, 0x00F7},{0x2215, 0x0146},
    {0x2219, 0x006A},{0x2215, 0x0147},{0x2219, 0x0098},{0x2215, 0x0148},
    {0x2219, 0x0041},{0x2215, 0x0149},{0x2219, 0x004F},{0x2215, 0x014A},
    {0x2219, 0x0098},{0x2215, 0x014B},{0x2219, 0x000A},{0x2215, 0x014C},
    {0x2219, 0x0068},{0x2215, 0x014D},{0x2219, 0x009C},{0x2215, 0x014E},
    {0x2219, 0x0051},{0x2215, 0x014F},{0x2219, 0x009C},{0x2215, 0x0150},
    {0x2219, 0x008C},{0x2215, 0x0151},{0x2219, 0x00A1},{0x2215, 0x0152},
    {0x2219, 0x0016},{0x2215, 0x0153},{0x2219, 0x0096},{0x2215, 0x0154},
    {0x2219, 0x00DB},{0x2215, 0x0155},{0x2219, 0x0043},{0x2215, 0x0156},
    {0x2219, 0x0031},{0x2215, 0x0157},{0x2219, 0x008F},{0x2215, 0x0158},
    {0x2219, 0x00BE},{0x2215, 0x0159},{0x2219, 0x008E},{0x2215, 0x015A},
    {0x2219, 0x008F},{0x2215, 0x015B},{0x2219, 0x00DA},{0x2215, 0x015C},
    {0x2219, 0x0043},{0x2215, 0x015D},{0x2219, 0x0051},{0x2215, 0x015E},
    {0x2219, 0x0080},{0x2215, 0x015F},{0x2219, 0x0050},{0x2215, 0x0160},
    {0x2219, 0x0093},{0x2215, 0x0161},{0x2219, 0x00A0},{0x2215, 0x0162},
    {0x2219, 0x0016},{0x2215, 0x0163},{0x2219, 0x0086},{0x2215, 0x0164},
    {0x2219, 0x0041},{0x2215, 0x0165},{0x2219, 0x0000},{0x2215, 0x0166},
    {0x2219, 0x00AD},{0x2215, 0x0167},{0x2219, 0x00F0},{0x2215, 0x0168},
    {0x2219, 0x00F9},{0x2215, 0x0169},{0x2219, 0x004A},{0x2215, 0x016A},
    {0x2219, 0x0001},{0x2215, 0x016B},{0x2219, 0x0033},{0x2215, 0x016C},
    {0x2219, 0x0068},{0x2215, 0x016D},{0x2219, 0x0029},{0x2215, 0x016E},
    {0x2219, 0x0009},{0x2215, 0x016F},{0x2219, 0x0068},{0x2215, 0x0170},
    {0x2219, 0x0012},{0x2215, 0x0171},{0x2219, 0x0029},{0x2215, 0x0172},
    {0x2219, 0x0068},{0x2215, 0x0173},{0x2219, 0x0020},{0x2215, 0x0174},
    {0x2219, 0x002D},{0x2215, 0x0175},{0x2219, 0x0068},{0x2215, 0x0176},
    {0x2219, 0x0062},{0x2215, 0x0177},{0x2219, 0x006D},{0x2215, 0x0178},
    {0x2219, 0x0098},{0x2215, 0x0179},{0x2219, 0x0088},{0x2215, 0x017A},
    {0x2219, 0x0051},{0x2215, 0x017B},{0x2219, 0x0000},{0x2215, 0x017C},
    {0x2219, 0x00FF},{0x2215, 0x017D},{0x2219, 0x009C},{0x2215, 0x017E},
    {0x2219, 0x007A},{0x2215, 0x017F},{0x2219, 0x00E1},{0x2215, 0x0180},
    {0x2219, 0x00BF},{0x2215, 0x0181},{0x2219, 0x0082},{0x2215, 0x0182},
    {0x2219, 0x0004},{0x2215, 0x0183},{0x2219, 0x0050},{0x2215, 0x0184},
    {0x2219, 0x0083},{0x2215, 0x0185},{0x2219, 0x00C3},{0x2215, 0x0186},
    {0x2219, 0x0050},{0x2215, 0x0187},{0x2219, 0x0082},{0x2215, 0x0188},
    {0x2219, 0x0041},{0x2215, 0x0189},{0x2219, 0x00FF},{0x2215, 0x018A},
    {0x2219, 0x009B},{0x2215, 0x018B},{0x2219, 0x0076},{0x2215, 0x018C},
    {0x2219, 0x006A},{0x2215, 0x018D},{0x2219, 0x005E},{0x2215, 0x018E},
    {0x2219, 0x0065},{0x2215, 0x018F},{0x2219, 0x0055},{0x2215, 0x0190},
    {0x2219, 0x0048},{0x2215, 0x0191},{0x2219, 0x006D},{0x2215, 0x0192},
    {0x2219, 0x0092},{0x2215, 0x0193},{0x2219, 0x0004},{0x2215, 0x0194},
    {0x2219, 0x0068},{0x2215, 0x0195},{0x2219, 0x0020},{0x2215, 0x0196},
    {0x2219, 0x007D},{0x2215, 0x0197},{0x2219, 0x0082},{0x2215, 0x0198},
    {0x2219, 0x0009},{0x2215, 0x0199},{0x2219, 0x009C},{0x2215, 0x019A},
    {0x2219, 0x0004},{0x2215, 0x019B},{0x2219, 0x0050},{0x2215, 0x019C},
    {0x2219, 0x0081},{0x2215, 0x019D},{0x2219, 0x00CB},{0x2215, 0x019E},
    {0x2219, 0x0050},{0x2215, 0x019F},{0x2219, 0x0081},{0x2215, 0x01A0},
    {0x2219, 0x00C8},{0x2215, 0x01A1},{0x2219, 0x009C},{0x2215, 0x01A2},
    {0x2219, 0x0004},{0x2215, 0x01A3},{0x2219, 0x0050},{0x2215, 0x01A4},
    {0x2219, 0x0080},{0x2215, 0x01A5},{0x2219, 0x00C3},{0x2215, 0x01A6},
    {0x2219, 0x0050},{0x2215, 0x01A7},{0x2219, 0x0080},{0x2215, 0x01A8},
    {0x2219, 0x009B},{0x2215, 0x01A9},{0x2219, 0x0003},{0x2215, 0x01AA},
    {0x2219, 0x0042},{0x2215, 0x01AB},{0x2219, 0x005F},{0x2215, 0x01AC},
    {0x2219, 0x009A},{0x2215, 0x01AD},{0x2219, 0x0089},{0x2215, 0x01AE},
    {0x2219, 0x009E},{0x2215, 0x01AF},{0x2219, 0x008E},{0x2215, 0x01B0},
    {0x2219, 0x00A6},{0x2215, 0x01B1},{0x2219, 0x0038},{0x2215, 0x01B2},
    {0x2219, 0x00E0},{0x2215, 0x01B3},{0x2219, 0x009D},{0x2215, 0x01B4},
    {0x2219, 0x005E},{0x2215, 0x01B5},{0x2219, 0x00CA},{0x2215, 0x01B6},
    {0x2219, 0x0044},{0x2215, 0x01B7},{0x2219, 0x0060},{0x2215, 0x01B8},
    {0x2219, 0x004C},{0x2215, 0x01B9},{0x2219, 0x0059},{0x2215, 0x01BA},
    {0x2219, 0x0020},{0x2215, 0x01BB},{0x2219, 0x00E1},{0x2215, 0x01BC},
    {0x2219, 0x0092},{0x2215, 0x01BD},{0x2219, 0x00E1},{0x2215, 0x01BE},
    {0x2219, 0x00F7},{0x2215, 0x01BF},{0x2219, 0x006A},{0x2215, 0x01C0},
    {0x2219, 0x002F},{0x2215, 0x01C1},{0x2219, 0x006D},{0x2215, 0x01C2},
    {0x2219, 0x0037},{0x2215, 0x01C3},{0x2219, 0x00A6},{0x2215, 0x01C4},
    {0x2219, 0x0093},{0x2215, 0x01C5},{0x2219, 0x0083},{0x2215, 0x01C6},
    {0x2219, 0x0048},{0x2215, 0x01C7},{0x2219, 0x007D},{0x2215, 0x01C8},
    {0x2219, 0x0098},{0x2215, 0x01C9},{0x2219, 0x0086},{0x2215, 0x01CA},
    {0x2219, 0x009C},{0x2215, 0x01CB},{0x2219, 0x0003},{0x2215, 0x01CC},
    {0x2219, 0x0050},{0x2215, 0x01CD},{0x2219, 0x0000},{0x2215, 0x01CE},
    {0x2219, 0x00C8},{0x2215, 0x01CF},{0x2219, 0x00BC},{0x2215, 0x01D0},
    {0x2219, 0x0057},{0x2215, 0x01D1},{0x2219, 0x0009},{0x2215, 0x01D2},
    {0x2219, 0x0082},{0x2215, 0x01D3},{0x2219, 0x0004},{0x2215, 0x01D4},
    {0x2219, 0x0050},{0x2215, 0x01D5},{0x2219, 0x0081},{0x2215, 0x01D6},
    {0x2219, 0x00CB},{0x2215, 0x01D7},{0x2219, 0x0050},{0x2215, 0x01D8},
    {0x2219, 0x0080},{0x2215, 0x01D9},{0x2219, 0x00C8},{0x2215, 0x01DA},
    {0x2219, 0x0082},{0x2215, 0x01DB},{0x2219, 0x0004},{0x2215, 0x01DC},
    {0x2219, 0x0050},{0x2215, 0x01DD},{0x2219, 0x0081},{0x2215, 0x01DE},
    {0x2219, 0x00C3},{0x2215, 0x01DF},{0x2219, 0x0050},{0x2215, 0x01E0},
    {0x2219, 0x0080},{0x2215, 0x01E1},{0x2219, 0x009A},{0x2215, 0x01E2},
    {0x2219, 0x008A},{0x2215, 0x01E3},{0x2219, 0x009E},{0x2215, 0x01E4},
    {0x2219, 0x008F},{0x2215, 0x01E5},{0x2219, 0x0089},{0x2215, 0x01E6},
    {0x2219, 0x005D},{0x2215, 0x01E7},{0x2219, 0x00A4},{0x2215, 0x01E8},
    {0x2219, 0x002B},{0x2215, 0x01E9},{0x2219, 0x00DA},{0x2215, 0x01EA},
    {0x2219, 0x00E0},{0x2215, 0x01EB},{0x2219, 0x00BC},{0x2215, 0x01EC},
    {0x2219, 0x0044},{0x2215, 0x01ED},{0x2219, 0x0060},{0x2215, 0x01EE},
    {0x2219, 0x004C},{0x2215, 0x01EF},{0x2219, 0x0059},{0x2215, 0x01F0},
    {0x2219, 0x0020},{0x2215, 0x01F1},{0x2219, 0x00E1},{0x2215, 0x01F2},
    {0x2219, 0x00C3},{0x2215, 0x01F3},{0x2219, 0x0042},{0x2215, 0x01F4},
    {0x2219, 0x0057},{0x2215, 0x01F5},{0x2219, 0x0042},{0x2215, 0x01F6},
    {0x2219, 0x0056},{0x2215, 0x01F7},{0x2219, 0x0046},{0x2215, 0x01F8},
    {0x2219, 0x0022},{0x2215, 0x01F9},{0x2219, 0x0002},{0x2215, 0x01FA},
    {0x2219, 0x0044},{0x2215, 0x01FB},{0x2219, 0x0020},{0x2215, 0x01FC},
    {0x2219, 0x00E0},{0x2215, 0x01FD},{0x2219, 0x0000},{0x2216, 0x8002},
    {0x221F, 0x0005},{0x2205, 0x8000},{0x2206, 0xEEFF},{0x2206, 0xFC8B},
    {0x2206, 0xEEFF},{0x2206, 0xFDA0},{0x2206, 0x0280},{0x2206, 0x33F7},
    {0x2206, 0x00E0},{0x2206, 0xFFF7},{0x2206, 0xA080},{0x2206, 0x02AE},
    {0x2206, 0xF602},{0x2206, 0x84E1},{0x2206, 0x0201},{0x2206, 0x4802},
    {0x2206, 0x80BC},{0x2206, 0x0280},{0x2206, 0xD0E0},{0x2206, 0x8B8C},
    {0x2206, 0xE18B},{0x2206, 0x8D1E},{0x2206, 0x01E1},{0x2206, 0x8B8E},
    {0x2206, 0x1E01},{0x2206, 0xA000},{0x2206, 0xE4AE},{0x2206, 0xD8EE},
    {0x2206, 0x8590},{0x2206, 0x00EE},{0x2206, 0x8591},{0x2206, 0x00EE},
    {0x2206, 0x8592},{0x2206, 0x03EE},{0x2206, 0x8593},{0x2206, 0xD0EE},
    {0x2206, 0xE080},{0x2206, 0x00EE},{0x2206, 0xE081},{0x2206, 0xFFEE},
    {0x2206, 0xE082},{0x2206, 0x16EE},{0x2206, 0xE083},{0x2206, 0x5AEE},
    {0x2206, 0xE144},{0x2206, 0x77EE},{0x2206, 0xE145},{0x2206, 0x65EE},
    {0x2206, 0x8B85},{0x2206, 0x42EE},{0x2206, 0x8594},{0x2206, 0x6EEE},
    {0x2206, 0x859B},{0x2206, 0x10EE},{0x2206, 0x8595},{0x2206, 0x00EE},
    {0x2206, 0x8596},{0x2206, 0x00EE},{0x2206, 0x8597},{0x2206, 0x02EE},
    {0x2206, 0x8598},{0x2206, 0x00EE},{0x2206, 0x8599},{0x2206, 0x00EE},
    {0x2206, 0x859C},{0x2206, 0x00D4},{0x2206, 0x82F2},{0x2206, 0xE48B},
    {0x2206, 0x96E5},{0x2206, 0x8B97},{0x2206, 0xD482},{0x2206, 0xFBE4},
    {0x2206, 0x8B94},{0x2206, 0xE58B},{0x2206, 0x95D1},{0x2206, 0x00BF},
    {0x2206, 0x8513},{0x2206, 0x0228},{0x2206, 0x88BF},{0x2206, 0x8B88},
    {0x2206, 0xEC00},{0x2206, 0x19A9},{0x2206, 0x8B90},{0x2206, 0xF9EE},
    {0x2206, 0xFFF6},{0x2206, 0x00EE},{0x2206, 0xFFF7},{0x2206, 0xFCE0},
    {0x2206, 0xE140},{0x2206, 0xE1E1},{0x2206, 0x41F7},{0x2206, 0x2FF6},
    {0x2206, 0x28E4},{0x2206, 0xE140},{0x2206, 0xE5E1},{0x2206, 0x4104},
    {0x2206, 0xE08B},{0x2206, 0x8DAD},{0x2206, 0x200D},{0x2206, 0xEE8B},
    {0x2206, 0x8D00},{0x2206, 0x0213},{0x2206, 0xD002},{0x2206, 0x818C},
    {0x2206, 0x021D},{0x2206, 0x4204},{0x2206, 0xE08B},{0x2206, 0x8EAD},
    {0x2206, 0x2017},{0x2206, 0xF620},{0x2206, 0xE48B},{0x2206, 0x8E02},
    {0x2206, 0x2509},{0x2206, 0x0225},{0x2206, 0xE302},{0x2206, 0x8304},
    {0x2206, 0x0282},{0x2206, 0x8302},{0x2206, 0x8413},{0x2206, 0x0284},
    {0x2206, 0x99E0},{0x2206, 0x8B8E},{0x2206, 0xAD23},{0x2206, 0x05F6},
    {0x2206, 0x23E4},{0x2206, 0x8B8E},{0x2206, 0xE08B},{0x2206, 0x8EAD},
    {0x2206, 0x2408},{0x2206, 0xF624},{0x2206, 0xE48B},{0x2206, 0x8E02},
    {0x2206, 0x26AC},{0x2206, 0xE08B},{0x2206, 0x8EAD},{0x2206, 0x260B},
    {0x2206, 0xF626},{0x2206, 0xE48B},{0x2206, 0x8E02},{0x2206, 0x04D8},
    {0x2206, 0x021B},{0x2206, 0xC902},{0x2206, 0x811E},{0x2206, 0x0281},
    {0x2206, 0x7804},{0x2206, 0xF8E0},{0x2206, 0x8B83},{0x2206, 0xAD23},
    {0x2206, 0x21E0},{0x2206, 0xE022},{0x2206, 0xE1E0},{0x2206, 0x23AD},
    {0x2206, 0x2920},{0x2206, 0xE08B},{0x2206, 0x83AD},{0x2206, 0x2106},
    {0x2206, 0xE18B},{0x2206, 0x84AD},{0x2206, 0x283C},{0x2206, 0xE08B},
    {0x2206, 0x85AD},{0x2206, 0x2106},{0x2206, 0xE18B},{0x2206, 0x84AD},
    {0x2206, 0x2930},{0x2206, 0xBF30},{0x2206, 0x2602},{0x2206, 0x2817},
    {0x2206, 0xAE28},{0x2206, 0xEE8A},{0x2206, 0xE600},{0x2206, 0xEE8A},
    {0x2206, 0xE900},{0x2206, 0xEE85},{0x2206, 0x9000},{0x2206, 0xEE85},
    {0x2206, 0x9100},{0x2206, 0xEE8B},{0x2206, 0x7200},{0x2206, 0xE08B},
    {0x2206, 0x83AD},{0x2206, 0x2108},{0x2206, 0xE08B},{0x2206, 0x84F6},
    {0x2206, 0x20E4},{0x2206, 0x8B84},{0x2206, 0xBF30},{0x2206, 0x2902},
    {0x2206, 0x2817},{0x2206, 0xFC04},{0x2206, 0xF8E0},{0x2206, 0xE038},
    {0x2206, 0xE1E0},{0x2206, 0x39AC},{0x2206, 0x2E08},{0x2206, 0xEEE0},
    {0x2206, 0x8E36},{0x2206, 0xEEE0},{0x2206, 0x8F20},{0x2206, 0xFC04},
    {0x2206, 0xF8F9},{0x2206, 0xFAE0},{0x2206, 0x8B81},{0x2206, 0xAC26},
    {0x2206, 0x0EE0},{0x2206, 0x8B81},{0x2206, 0xAC21},{0x2206, 0x08E0},
    {0x2206, 0x8B85},{0x2206, 0xAC20},{0x2206, 0x02AE},{0x2206, 0x6EEE},
    {0x2206, 0xE0EA},{0x2206, 0x00EE},{0x2206, 0xE0EB},{0x2206, 0x00E2},
    {0x2206, 0xE07C},{0x2206, 0xE3E0},{0x2206, 0x7DA5},{0x2206, 0x1111},
    {0x2206, 0x18D2},{0x2206, 0x60D6},{0x2206, 0x6666},{0x2206, 0x0208},
    {0x2206, 0x2CD2},{0x2206, 0xA0D6},{0x2206, 0xAAAA},{0x2206, 0x0208},
    {0x2206, 0x2C02},{0x2206, 0x1F1F},{0x2206, 0x0282},{0x2206, 0x3FAE},
    {0x2206, 0x44A5},{0x2206, 0x6666},{0x2206, 0x02AE},{0x2206, 0x38A5},
    {0x2206, 0xAAAA},{0x2206, 0x02AE},{0x2206, 0x32EE},{0x2206, 0xE0EA},
    {0x2206, 0x04EE},{0x2206, 0xE0EB},{0x2206, 0x06E2},{0x2206, 0xE07C},
    {0x2206, 0xE3E0},{0x2206, 0x7DE0},{0x2206, 0xE038},{0x2206, 0xE1E0},
    {0x2206, 0x39AD},{0x2206, 0x2E21},{0x2206, 0xAD3F},{0x2206, 0x13E0},
    {0x2206, 0xE414},{0x2206, 0xE1E4},{0x2206, 0x1568},{0x2206, 0x80E4},
    {0x2206, 0xE414},{0x2206, 0xE5E4},{0x2206, 0x1502},{0x2206, 0x823F},
    {0x2206, 0xAE0B},{0x2206, 0xAC3E},{0x2206, 0x02AE},{0x2206, 0x0602},
    {0x2206, 0x8215},{0x2206, 0x021E},{0x2206, 0xBEFE},{0x2206, 0xFDFC},
    {0x2206, 0x04F8},{0x2206, 0xE08B},{0x2206, 0x81AD},{0x2206, 0x2603},
    {0x2206, 0x021F},{0x2206, 0xD4E0},{0x2206, 0x8B81},{0x2206, 0xAD21},
    {0x2206, 0x09E0},{0x2206, 0x8B32},{0x2206, 0xAC20},{0x2206, 0x0302},
    {0x2206, 0x217E},{0x2206, 0xE08B},{0x2206, 0x85AD},{0x2206, 0x2009},
    {0x2206, 0xE08B},{0x2206, 0x32AC},{0x2206, 0x2103},{0x2206, 0x0282},
    {0x2206, 0x69FC},{0x2206, 0x04F8},{0x2206, 0xE18B},{0x2206, 0x32E0},
    {0x2206, 0x8B81},{0x2206, 0xAD26},{0x2206, 0x0502},{0x2206, 0x208A},
    {0x2206, 0xF728},{0x2206, 0xE08B},{0x2206, 0x81AD},{0x2206, 0x2105},
    {0x2206, 0x0221},{0x2206, 0x8FF7},{0x2206, 0x29E0},{0x2206, 0x8B85},
    {0x2206, 0xAD20},{0x2206, 0x0502},{0x2206, 0x827A},{0x2206, 0xF72A},
    {0x2206, 0xE58B},{0x2206, 0x32FC},{0x2206, 0x04F8},{0x2206, 0xF9FA},
    {0x2206, 0xEF69},{0x2206, 0xE08B},{0x2206, 0x85AD},{0x2206, 0x2000},
    {0x2206, 0xEF96},{0x2206, 0xFEFD},{0x2206, 0xFC04},{0x2206, 0xF8E0},
    {0x2206, 0x8B85},{0x2206, 0xAD20},{0x2206, 0x00FC},{0x2206, 0x04F8},
    {0x2206, 0xE08B},{0x2206, 0x83AD},{0x2206, 0x2131},{0x2206, 0xE08A},
    {0x2206, 0xE6A0},{0x2206, 0x0005},{0x2206, 0x0202},{0x2206, 0x60AE},
    {0x2206, 0xF5A0},{0x2206, 0x0105},{0x2206, 0x0202},{0x2206, 0x77AE},
    {0x2206, 0x1EA0},{0x2206, 0x0205},{0x2206, 0x0202},{0x2206, 0x7FAE},
    {0x2206, 0x16A0},{0x2206, 0x0305},{0x2206, 0x0282},{0x2206, 0xBDAE},
    {0x2206, 0x0EA0},{0x2206, 0x0405},{0x2206, 0x0203},{0x2206, 0x2BAE},
    {0x2206, 0x06EE},{0x2206, 0x8AE6},{0x2206, 0x00AE},{0x2206, 0xCFFC},
    {0x2206, 0x04F8},{0x2206, 0xF9D0},{0x2206, 0x0002},{0x2206, 0x0333},
    {0x2206, 0xE08A},{0x2206, 0xE6A0},{0x2206, 0x0002},{0x2206, 0xAE23},
    {0x2206, 0xE085},{0x2206, 0x90E1},{0x2206, 0x8591},{0x2206, 0xE285},
    {0x2206, 0x92E3},{0x2206, 0x8593},{0x2206, 0x1B54},{0x2206, 0x9F0C},
    {0x2206, 0xBF30},{0x2206, 0x0B02},{0x2206, 0x2817},{0x2206, 0xEE8A},
    {0x2206, 0xE604},{0x2206, 0xAE07},{0x2206, 0x14E4},{0x2206, 0x8590},
    {0x2206, 0xE585},{0x2206, 0x91FD},{0x2206, 0xFC04},{0x2206, 0xEEE0},
    {0x2206, 0x8E36},{0x2206, 0xEEE0},{0x2206, 0x8F21},{0x2206, 0x05EE},
    {0x2206, 0xE08E},{0x2206, 0x36EE},{0x2206, 0xE08F},{0x2206, 0x2005},
    {0x2206, 0xF8FA},{0x2206, 0xEF69},{0x2206, 0xE08B},{0x2206, 0x85AD},
    {0x2206, 0x2139},{0x2206, 0xE0E0},{0x2206, 0x22E1},{0x2206, 0xE023},
    {0x2206, 0x58C0},{0x2206, 0x5902},{0x2206, 0x1E01},{0x2206, 0xE18B},
    {0x2206, 0x721F},{0x2206, 0x109E},{0x2206, 0x26E4},{0x2206, 0x8B72},
    {0x2206, 0xAD21},{0x2206, 0x1DE1},{0x2206, 0x8B84},{0x2206, 0xF729},
    {0x2206, 0xE58B},{0x2206, 0x84AC},{0x2206, 0x270D},{0x2206, 0xAC26},
    {0x2206, 0x0502},{0x2206, 0x0471},{0x2206, 0xAE0D},{0x2206, 0x0283},
    {0x2206, 0x85AE},{0x2206, 0x0802},{0x2206, 0x83AE},{0x2206, 0xAE03},
    {0x2206, 0x0283},{0x2206, 0x4CEF},{0x2206, 0x96FE},{0x2206, 0xFC04},
    {0x2206, 0xD100},{0x2206, 0xBF85},{0x2206, 0x1302},{0x2206, 0x2888},
    {0x2206, 0xD103},{0x2206, 0xBF30},{0x2206, 0x4D02},{0x2206, 0x2888},
    {0x2206, 0xD100},{0x2206, 0xBF30},{0x2206, 0x5002},{0x2206, 0x2888},
    {0x2206, 0xD100},{0x2206, 0xBF85},{0x2206, 0x1902},{0x2206, 0x2888},
    {0x2206, 0xD10F},{0x2206, 0xBF30},{0x2206, 0x4402},{0x2206, 0x2888},
    {0x2206, 0xD101},{0x2206, 0xBF30},{0x2206, 0x4702},{0x2206, 0x2888},
    {0x2206, 0xD101},{0x2206, 0xBF30},{0x2206, 0x4A02},{0x2206, 0x2888},
    {0x2206, 0x04D1},{0x2206, 0x02BF},{0x2206, 0x304D},{0x2206, 0x0228},
    {0x2206, 0x88D1},{0x2206, 0x06BF},{0x2206, 0x3044},{0x2206, 0x0228},
    {0x2206, 0x88D1},{0x2206, 0x07BF},{0x2206, 0x3047},{0x2206, 0x0228},
    {0x2206, 0x88D1},{0x2206, 0x07BF},{0x2206, 0x304A},{0x2206, 0x0228},
    {0x2206, 0x88D1},{0x2206, 0x01BF},{0x2206, 0x8513},{0x2206, 0x0228},
    {0x2206, 0x8804},{0x2206, 0xD102},{0x2206, 0xBF30},{0x2206, 0x4D02},
    {0x2206, 0x2888},{0x2206, 0xD101},{0x2206, 0xBF85},{0x2206, 0x1302},
    {0x2206, 0x2888},{0x2206, 0xD011},{0x2206, 0x0227},{0x2206, 0x3359},
    {0x2206, 0x03EF},{0x2206, 0x01D1},{0x2206, 0x00A0},{0x2206, 0x0002},
    {0x2206, 0xD101},{0x2206, 0xBF30},{0x2206, 0x5002},{0x2206, 0x2888},
    {0x2206, 0xD111},{0x2206, 0xAD20},{0x2206, 0x020C},{0x2206, 0x11AD},
    {0x2206, 0x2102},{0x2206, 0x0C12},{0x2206, 0xBF85},{0x2206, 0x1902},
    {0x2206, 0x2888},{0x2206, 0x0283},{0x2206, 0xEA04},{0x2206, 0xE08B},
    {0x2206, 0x85AD},{0x2206, 0x2422},{0x2206, 0xD400},{0x2206, 0x01BF},
    {0x2206, 0x850A},{0x2206, 0x0228},{0x2206, 0x88BF},{0x2206, 0x850D},
    {0x2206, 0x0228},{0x2206, 0xC6E0},{0x2206, 0x8594},{0x2206, 0x1B10},
    {0x2206, 0xAA04},{0x2206, 0xD101},{0x2206, 0xAE02},{0x2206, 0xD100},
    {0x2206, 0xBF85},{0x2206, 0x1002},{0x2206, 0x2888},{0x2206, 0x04F8},
    {0x2206, 0xF9E0},{0x2206, 0x8B85},{0x2206, 0xAD25},{0x2206, 0x7BE0},
    {0x2206, 0xE022},{0x2206, 0xE1E0},{0x2206, 0x23E2},{0x2206, 0xE036},
    {0x2206, 0xE3E0},{0x2206, 0x375A},{0x2206, 0xC40D},{0x2206, 0x0158},
    {0x2206, 0x021E},{0x2206, 0x20E3},{0x2206, 0x8595},{0x2206, 0xAC31},
    {0x2206, 0x4FAC},{0x2206, 0x3A05},{0x2206, 0xAC3E},{0x2206, 0x23AE},
    {0x2206, 0x56AD},{0x2206, 0x3753},{0x2206, 0xE085},{0x2206, 0x9610},
    {0x2206, 0xE485},{0x2206, 0x96E1},{0x2206, 0x8597},{0x2206, 0x1B10},
    {0x2206, 0x9E02},{0x2206, 0xAE43},{0x2206, 0xD100},{0x2206, 0xBF85},
    {0x2206, 0x1602},{0x2206, 0x2888},{0x2206, 0xEE85},{0x2206, 0x9600},
    {0x2206, 0xAE35},{0x2206, 0xAD36},{0x2206, 0x19E0},{0x2206, 0x8598},
    {0x2206, 0xE185},{0x2206, 0x99A4},{0x2206, 0x03B8},{0x2206, 0x02AE},
    {0x2206, 0x2614},{0x2206, 0xE485},{0x2206, 0x98E5},{0x2206, 0x8599},
    {0x2206, 0xA403},{0x2206, 0xB81B},{0x2206, 0xAE0D},{0x2206, 0xEE85},
    {0x2206, 0x9800},{0x2206, 0xEE85},{0x2206, 0x9900},{0x2206, 0xAE0F},
    {0x2206, 0xAC39},{0x2206, 0x0CD1},{0x2206, 0x01BF},{0x2206, 0x8516},
    {0x2206, 0x0228},{0x2206, 0x88EE},{0x2206, 0x8596},{0x2206, 0x00E6},
    {0x2206, 0x8595},{0x2206, 0xFDFC},{0x2206, 0x04F8},{0x2206, 0xF9E0},
    {0x2206, 0x8B85},{0x2206, 0xAD26},{0x2206, 0x3DE0},{0x2206, 0xE036},
    {0x2206, 0xE1E0},{0x2206, 0x37E1},{0x2206, 0x859C},{0x2206, 0x1F10},
    {0x2206, 0x9E30},{0x2206, 0xE485},{0x2206, 0x9CAD},{0x2206, 0x212A},
    {0x2206, 0xD00B},{0x2206, 0x0227},{0x2206, 0x3358},{0x2206, 0x8278},
    {0x2206, 0x829F},{0x2206, 0x1FE0},{0x2206, 0xE000},{0x2206, 0xE1E0},
    {0x2206, 0x01F7},{0x2206, 0x27E4},{0x2206, 0xE000},{0x2206, 0xE5E0},
    {0x2206, 0x01E2},{0x2206, 0xE020},{0x2206, 0xE3E0},{0x2206, 0x21AD},
    {0x2206, 0x30F7},{0x2206, 0xF627},{0x2206, 0xE4E0},{0x2206, 0x00E5},
    {0x2206, 0xE001},{0x2206, 0xFDFC},{0x2206, 0x04F8},{0x2206, 0xFAEF},
    {0x2206, 0x69E0},{0x2206, 0x8B86},{0x2206, 0xAC20},{0x2206, 0x1ABF},
    {0x2206, 0x851C},{0x2206, 0xD072},{0x2206, 0x0227},{0x2206, 0xEAE0},
    {0x2206, 0xE0E4},{0x2206, 0xE1E0},{0x2206, 0xE558},{0x2206, 0x0668},
    {0x2206, 0xC0D1},{0x2206, 0xD2E4},{0x2206, 0xE0E4},{0x2206, 0xE5E0},
    {0x2206, 0xE5EF},{0x2206, 0x96FE},{0x2206, 0xFC04},{0x2206, 0xA0E0},
    {0x2206, 0xEAF0},{0x2206, 0xE07C},{0x2206, 0x55E2},{0x2206, 0x3211},
    {0x2206, 0xE232},{0x2206, 0x88E2},{0x2206, 0x0070},{0x2206, 0xE426},
    {0x2206, 0x2508},{0x2206, 0x0726},{0x2206, 0x4072},{0x2206, 0x2726},
    {0x2206, 0x7E28},{0x2206, 0x04B7},{0x2206, 0x2925},{0x2206, 0x762A},
    {0x2206, 0x68E5},{0x2206, 0x2BAD},{0x2206, 0x002C},{0x2206, 0xDBF0},
    {0x2206, 0x2D67},{0x2206, 0xBB2E},{0x2206, 0x7B0F},{0x2206, 0x2F73},
    {0x2206, 0x6531},{0x2206, 0xACCC},{0x2206, 0x3223},{0x2206, 0x0033},
    {0x2206, 0x2D17},{0x2206, 0x347F},{0x2206, 0x5235},{0x2206, 0x1000},
    {0x2206, 0x3606},{0x2206, 0x0037},{0x2206, 0x0CC0},{0x2206, 0x387F},
    {0x2206, 0xCE3C},{0x2206, 0xE5F7},{0x2206, 0x3D3D},{0x2206, 0xA461},
    {0x2206, 0x368A},{0x2206, 0x6402},{0x2206, 0x2665},{0x2206, 0x307D},
    {0x2206, 0x6700},{0x2206, 0x5369},{0x2206, 0xD20F},{0x2206, 0x6A01},
    {0x2206, 0x2C6C},{0x2206, 0x2B13},{0x2206, 0x6EE1},{0x2206, 0x206F},
    {0x2206, 0x12F7},{0x2206, 0x7100},{0x2206, 0x6B73},{0x2206, 0x06EB},
    {0x2206, 0x7494},{0x2206, 0xC776},{0x2206, 0x980A},{0x2206, 0x7750},
    {0x2206, 0x0078},{0x2206, 0x8A15},{0x2206, 0x797F},{0x2206, 0x6F7A},
    {0x2206, 0x06A6},{0x2205, 0x8BF0},{0x2206, 0x0000},{0x2206, 0x0000},
    {0x2206, 0x0000},{0x2206, 0x0000},{0x2206 ,0x0000},{0x2206, 0x0000},
    {0x2206, 0x0000},{0x2206, 0x0000},{0x2201, 0x0701},{0x2200, 0x0405},
    {0x221F, 0x0000},{0x221F, 0x0005},{0x2205, 0x8B84},{0x2206, 0x0062},
    {0x221F, 0x0000},{0x133F, 0x0010},{0x133E, 0x0FFE},{0x12A4, 0x380A},
    {0x1362, 0x0115},{0x1363, 0x0002},{0x1363, 0x0000},{0x133F, 0x0030},
    {0x133E, 0x000E},{0x221F, 0x0000},{0x2200, 0x1340},{0x221F, 0x0000},
    {0x133F, 0x0010},{0x133E, 0x0FFE},{0xFFFF,0xABCD}};

    CONST_T uint16 ParaB[][2] = {
    {0x133F, 0x0030},{0x133E, 0x000E},{0x221F, 0x0005},{0x2205, 0x8B84},
    {0x2206, 0x0062},{0x221F, 0x0007},{0x221E, 0x0020},{0x2215, 0x0100},
    {0x221F, 0x0000},{0x133F, 0x0010},{0x133E, 0x0FFE},{0x12A4, 0x380A},
    {0xFFFF, 0xABCD}};

    if ((retVal = rtl8370_setAsicPHYReg(0,PHY_PAGE_ADDRESS,5))!=RT_ERR_OK)
        return retVal; 
    if ((retVal = rtl8370_setAsicPHYReg(0,5,0x3ffe))!=RT_ERR_OK)
        return retVal; 
    if ((retVal = rtl8370_getAsicPHYReg(0,6,&regData))!=RT_ERR_OK)
        return retVal; 

#ifdef MDC_MDIO_OPERATION 
    if (regData == 0x94eb)
    {
        index = 0;
        while (ParaA[index][0] != 0xFFFF && ParaA[index][1] != 0xABCD)
        {    
            if (RT_ERR_OK != rtl8370_setAsicReg((uint32)ParaA[index][0],(uint32)ParaA[index][1]))
                return RT_ERR_FAILED;
            index ++;    
        }   
    }
    else if (regData == 0x2104)    
    {
        index = 0;
        while (ParaB[index][0] != 0xFFFF && ParaB[index][1] != 0xABCD)
        {    
            if (RT_ERR_OK != rtl8370_setAsicReg((uint32)ParaB[index][0],(uint32)ParaB[index][1]))
                return RT_ERR_FAILED;
            index ++;    
        } 
    }
    else
        return RT_ERR_FAILED;   

#else  

    if (regData == 0x94eb)
    {
        index = 0;
        while (ParaA[index][0] != 0xFFFF && ParaA[index][1] != 0xABCD)
        {    
            if ((ParaA[index][0]&0xF000)==0x2000)
            {
                cnt = 0;
                busyFlag = 1;
                while (busyFlag&&cnt<5)
                {
                    cnt++;
                    if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                        return retVal;
                }
                if (5 == cnt)
                    return RT_ERR_BUSYWAIT_TIMEOUT;
            
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32)ParaA[index][1])) !=  RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32)ParaA[index][0])) !=  RT_ERR_OK)
                    return retVal; 
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if ((retVal = rtl8370_setAsicReg((uint32)ParaA[index][0],(uint32)ParaA[index][1])) !=  RT_ERR_OK)
                    return retVal;
            }
            index ++; 
        }         
    }
    else if (regData == 0x2104)    
    {
        index = 0;
        while (ParaB[index][0] != 0xFFFF && ParaB[index][1] != 0xABCD)
        {    
            if ((ParaB[index][0]&0xF000)==0x2000)
            {
                cnt = 0;
                busyFlag = 1;
                while (busyFlag&&cnt<5)
                {
                    cnt++;
                    if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                        return retVal;
                }
                if (5 == cnt)
                    return RT_ERR_BUSYWAIT_TIMEOUT;
            
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32)ParaB[index][1])) !=  RT_ERR_OK)
                    return retVal;
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32)ParaB[index][0])) !=  RT_ERR_OK)
                    return retVal;    
                if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                    return retVal; 
            }
            else
            {
                if (RT_ERR_OK != rtl8370_setAsicReg((uint32)ParaB[index][0],(uint32)ParaB[index][1]))
                    return RT_ERR_FAILED;
            }
            index ++;    
        } 
    }
    else
        return RT_ERR_FAILED;

    
#endif /*End of #ifdef MDC_MDIO_OPERATION*/

    return RT_ERR_OK;
}
#endif

/* Function Name:
 *      rtk_eee_portEnable_set
 * Description:
 *      Set enable status of EEE function.
 * Input:
 *      port - port id.
 *      enable - enable EEE status.
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set EEE function to the specific port.
 *      The configuration of the port is as following:
 *      DISABLE   
 *      ENABLE
 */
rtk_api_ret_t rtk_eee_portEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    uint32 eee_cfg = 0;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (enable == ENABLED)
        eee_cfg = RTL8370_PORT_EEE_100M_MASK|RTL8370_PORT_EEE_GIGA_MASK;

    if ((retVal = rtl8370_setAsicReg(RTL8370_PORT_EEE_CFG_REG(port), eee_cfg))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8370_setAsicPHYReg(port,31,5))!=RT_ERR_OK)
        return retVal;

    if (enable == ENABLED)
    {
        if ((retVal = rtl8370_setAsicPHYReg(port,5,0x8B84))!=RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8370_setAsicPHYReg(port,6,0x0062))!=RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8370_setAsicPHYReg(port,31,7))!=RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8370_setAsicPHYReg(port,30,32))!=RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8370_setAsicPHYReg(port,21,0x0100))!=RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8370_setAsicPHYReg(port,5,0x8B84))!=RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8370_setAsicPHYReg(port,6,0x0042))!=RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8370_setAsicPHYReg(port,31,7))!=RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8370_setAsicPHYReg(port,30,32))!=RT_ERR_OK)
            return retVal;
        if ((retVal = rtl8370_setAsicPHYReg(port,21,0x0000))!=RT_ERR_OK)
            return retVal;
    }

    if ((retVal = rtl8370_setAsicPHYReg(port,31,0))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

#if !defined(_REDUCE_CODE)
/* Function Name:
 *      rtk_eee_portEnable_get
 * Description:
 *      Get port admin configuration of the specific port.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can set EEE function to the specific port.
 *      The configuration of the port is as following:
 *      DISABLE   
 *      ENABLE
 */

rtk_api_ret_t rtk_eee_portEnable_get(rtk_port_t port, rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;
    uint32 regData1, regData2;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_getAsicEee100M(port,&regData1))!=RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8370_getAsicEeeGiga(port,&regData2))!=RT_ERR_OK)
        return retVal;

    if (regData1==1&&regData2==1)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;       
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_aldp_init
 * Description:
 *      Initialize Advanced Link Down Power Saving (ALDP) mode.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API is used to initialize ALDP mode.
 */
rtk_api_ret_t rtk_aldp_init(void)
{
    rtk_api_ret_t retVal;
    uint32 index;
#ifndef MDC_MDIO_OPERATION
    uint32 busyFlag,cnt;
#endif
    CONST_T uint16 chipData[][2] ={ {0x203f,0x0002},{0x202c,0x6b64},{0x203f,0x0000},{0x209f,0x0002},
                                    {0x208c,0x6b64},{0x209f,0x0000},{0x1326,0x2d1e},{0x1364,0x31f0},
                                    {0x1365,0x3a80},{0x133e,0x000e},{0x133f,0x0030},{0x221f,0x0005},
                                    {0x2201,0x0500},{0x2210,0x0000},{0x221f,0x0000},{0x133e,0x000e},
                                    {0x133f,0x0010},{0xFFFF,0xABCD}};

#ifdef MDC_MDIO_OPERATION  
    index = 0;
    while (chipData[index][0] != 0xFFFF && chipData[index][1] != 0xABCD)
    {
        if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData[index][0],(uint32)chipData[index][1]))
            return RT_ERR_FAILED;
        index ++;
    } 
#else 
    index = 0;
    while (chipData[index][0] != 0xFFFF && chipData[index][1] != 0xABCD)
    {
        if ((chipData[index][0]&0xF000)==0x2000)
        {
            cnt = 0;
            busyFlag = 1;
            while (busyFlag&&cnt<5)
            {
                cnt++;
                if ((retVal = rtl8370_getAsicRegBit(RTK_INDRECT_ACCESS_STATUS, RTK_PHY_BUSY_OFFSET,&busyFlag)) !=  RT_ERR_OK)
                    return retVal;
            }
            if (5 == cnt)
               return RT_ERR_BUSYWAIT_TIMEOUT;

            if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_WRITE_DATA, (uint32)chipData[index][1])) !=  RT_ERR_OK)
                return retVal;
            if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_ADDRESS, (uint32)chipData[index][0])) !=  RT_ERR_OK)
                return retVal;    
            if ((retVal = rtl8370_setAsicReg(RTK_INDRECT_ACCESS_CRTL, RTK_CMD_MASK | RTK_RW_MASK)) !=  RT_ERR_OK)
                return retVal; 
        }
        else
        {
            if (RT_ERR_OK != rtl8370_setAsicReg((uint32)chipData[index][0],(uint32)chipData[index][1]))
               return RT_ERR_FAILED;
        }
        index ++;
    }
#endif /*End of #ifdef MDC_MDIO_OPERATION*/

    return RT_ERR_OK;
}



/* Function Name:
 *      rtk_aldp_enable_set
 * Description:
 *      Set Advanced Link Down Power Saving (ALDP) enable/disable.
 * Input:
 *      enable - enable ALDP
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - FAILED
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_ENABLE - Invalid enable parameter.
 * Note:
 *      The API can enable or disable ALDP function.
 *      The status of ALDP:
 *      DISABLED
 *      ENABLED 
 */
rtk_api_ret_t rtk_aldp_enable_set(rtk_enable_t enable)
{
    if (RT_ERR_OK != rtl8370_setAsicRegBit(RTL8370_ALDP_CFG_REG, RTL8370_ALDP_ENABLE_OFFSET, enable))
       return RT_ERR_FAILED;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_aldp_enable_get
 * Description:
 *      Get Advanced Link Down Power Saving (ALDP setup.
 * Input:
 * Output:
 *      pEnable - status of ALDP.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get ALDP status.
 *      The status of ALDP:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_aldp_enable_get(rtk_data_t *pEnable)
{
    rtk_api_ret_t retVal;

    if ((retVal =  rtl8370_getAsicRegBit(RTL8370_ALDP_CFG_REG, RTL8370_ALDP_ENABLE_OFFSET, pEnable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

#endif

