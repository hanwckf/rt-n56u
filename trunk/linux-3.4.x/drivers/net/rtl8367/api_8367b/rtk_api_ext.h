/*
 * Copyright (C) 2010 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : RTL8367RB/RTL8367MB switch high-level API
 *
 * Feature : The file includes all high-layer API defination
 *
 */

#ifndef __RTK_API_EXT_H__
#define __RTK_API_EXT_H__

/*
 * Include Files
 */
#include "rtk_types.h"
#include "rtk_api.h"

/*
 * Function Declaration
 */

/*Misc*/
/* Function Name:
 *      rtk_switch_init
 * Description:
 *      Set chip to default configuration enviroment
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can set chip registers to default configuration for different release chip model.
 */
extern rtk_api_ret_t rtk_switch_init(void);

/* Function Name:
 *      rtk_switch_maxPktLen_set
 * Description:
 *      Set the max packet length of the specific unit
 * Input:
 *      len - max packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can set max packet length of the specific unit to
 *      - MAXPKTLEN_1522B,
 *      - MAXPKTLEN_1536B,
 *      - MAXPKTLEN_1552B,
 *      - MAXPKTLEN_16000B.
 */
extern rtk_api_ret_t rtk_switch_maxPktLen_set(rtk_switch_maxPktLen_t len);

/* Function Name:
 *      rtk_switch_maxPktLen_get
 * Description:
 *      Get the max packet length of the specific unit
 * Input:
 *      None
 * Output:
 *      pLen - pointer to the max packet length
 * Return:
 *      RT_ERR_OK              - set shared meter successfully
 *      RT_ERR_FAILED          - FAILED to iset shared meter
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      The API can set max packet length of the specific unit.
 */
extern rtk_api_ret_t rtk_switch_maxPktLen_get(rtk_switch_maxPktLen_t *pLen);

/* Function Name:
 *      rtk_switch_portMaxPktLen_set
 * Description:
 *      Set 2nd max packet length configuration
 * Input:
 *      port    - Port ID
 *      length  - Length(1522 bytes, 1536 bytes, 1552 bytes, 16000 bytes)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can set 2nd max packet length configuration.
 *      The parameter "port" indicates which port would follow 2nd configuration.
 *      If the paramter "length" is as same as 1st configuration, this API
 *      would set the port back to 1st configuration. Users should always use
 *      rtk_switch_maxPktLen_set to setup 1st configuration for whole system
 *      setting and then use rtk_switch_portMaxPktLen_set to setup a per port
 *      configuration.
 */
extern rtk_api_ret_t rtk_switch_portMaxPktLen_set(rtk_port_t port, rtk_uint32 length);

/* Function Name:
 *      rtk_switch_portMaxPktLen_get
 * Description:
 *      Get 2nd max packet length configuration
 * Input:
 *      port    - Port ID
 * Output:
 *      pLength  - Length(1522 bytes, 1536 bytes, 1552 bytes, 16000 bytes)
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can get 2nd max packet length configuration.
 *      If "port" is not in the 2nd configuration, the setting of
 *      1st configuration woill be returned.
 */
extern rtk_api_ret_t rtk_switch_portMaxPktLen_get(rtk_port_t port, rtk_uint32 *pLength);

/* Function Name:
 *      rtk_switch_greenEthernet_set
 * Description:
 *      Set all Ports Green Ethernet state.
 * Input:
 *      enable - Green Ethernet state.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - OK
 *      RT_ERR_FAILED   - Failed
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_ENABLE   - Invalid enable input.
 * Note:
 *      This API can set all Ports Green Ethernet state.
 *      The configuration is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_switch_greenEthernet_set(rtk_enable_t enable, rtk_enable_t set_phy_psm);

/* Function Name:
 *      rtk_switch_greenEthernet_get
 * Description:
 *      Get all Ports Green Ethernet state.
 * Input:
 *      None
 * Output:
 *      pEnable - Green Ethernet state.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API can get Green Ethernet state.
 */
extern rtk_api_ret_t rtk_switch_greenEthernet_get(rtk_enable_t *pEnable);

/* Rate */
/* Function Name:
 *      rtk_rate_shareMeter_set
 * Description:
 *      Set meter configuration
 * Input:
 *      index       - shared meter index
 *      rate        - rate of share meter
 *      ifg_include - include IFG or not, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 *      RT_ERR_RATE             - Invalid rate
 *      RT_ERR_INPUT            - Invalid input parameters
 * Note:
 *      The API can set shared meter rate and ifg include for each meter.
 *      The rate unit is 1 kbps and the range is from 8k to 1048568k.
 *      The granularity of rate is 8 kbps. The ifg_include parameter is used
 *      for rate calculation with/without inter-frame-gap and preamble.
 */
extern rtk_api_ret_t rtk_rate_shareMeter_set(rtk_meter_id_t index, rtk_rate_t rate, rtk_enable_t ifg_include);

/* Function Name:
 *      rtk_rate_shareMeter_get
 * Description:
 *      Get meter configuration
 * Input:
 *      index        - shared meter index
 * Output:
 *      pRate        - pointer of rate of share meter
 *      pIfg_include - include IFG or not, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can get shared meter rate and ifg include for each meter.
 *      The rate unit is 1 kbps and the granularity of rate is 8 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble
 */
extern rtk_api_ret_t rtk_rate_shareMeter_get(rtk_meter_id_t index, rtk_rate_t *pRate ,rtk_enable_t *pIfg_include);

/* Function Name:
 *      rtk_rate_shareMeterBucket_set
 * Description:
 *      Set meter Bucket Size
 * Input:
 *      index        - shared meter index
 *      bucket_size  - Bucket Size
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_INPUT            - Error Input
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can set shared meter bucket size.
 */
extern rtk_api_ret_t rtk_rate_shareMeterBucket_set(rtk_meter_id_t index, rtk_uint32 bucket_size);

/* Function Name:
 *      rtk_rate_shareMeterBucket_get
 * Description:
 *      Get meter Bucket Size
 * Input:
 *      index        - shared meter index
 * Output:
 *      pBucket_size - Bucket Size
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can get shared meter bucket size.
 */
extern rtk_api_ret_t rtk_rate_shareMeterBucket_get(rtk_meter_id_t index, rtk_uint32 *pBucket_size);

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set port ingress bandwidth control
 * Input:
 *      port        - Port id
 *      rate        - Rate of share meter
 *      ifg_include - include IFG or not, ENABLE:include DISABLE:exclude
 *      fc_enable   - enable flow control or not, ENABLE:use flow control DISABLE:drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 *      RT_ERR_ENABLE 		- Invalid IFG parameter.
 *      RT_ERR_INBW_RATE 	- Invalid ingress rate parameter.
 * Note:
 *      The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
extern rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_set( rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include, rtk_enable_t fc_enable);

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get port ingress bandwidth control
 * Input:
 *      port - Port id
 * Output:
 *      pRate           - Rate of share meter
 *      pIfg_include    - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 *      pFc_enable      - enable flow control or not, ENABLE:use flow control DISABLE:drop
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *     The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps.
 *     The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
extern rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include, rtk_enable_t *pFc_enable);

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set port egress bandwidth control
 * Input:
 *      port        - Port id
 *      rate        - Rate of egress bandwidth
 *      ifg_include - include IFG or not, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_QOS_EBW_RATE - Invalid egress bandwidth/rate
 * Note:
 *     The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps.
 *     The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
extern rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_set(rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_includ);

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_get
 * Description:
 *      Get port egress bandwidth control
 * Input:
 *      port - Port id
 * Output:
 *      pRate           - Rate of egress bandwidth
 *      pIfg_include    - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *     The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps.
 *     The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
extern rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include);

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlEnable_set
 * Description:
 *      Set enable status of egress bandwidth control on specified queue.
 * Input:
 *      port   - port id
 *      queue  - queue id
 *      enable - enable status of egress queue bandwidth control
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
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlEnable_set(rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable);

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlRate_get
 * Description:
 *      Get rate of egress bandwidth control on specified queue.
 * Input:
 *      port  - port id
 *      queue - queue id
 *      pIndex - shared meter index
 * Output:
 *      pRate - pointer to rate of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter id
 * Note:
 *    None.
 */
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlEnable_get(rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlRate_set
 * Description:
 *      Set rate of egress bandwidth control on specified queue.
 * Input:
 *      port  - port id
 *      queue - queue id
 *      index - shared meter index
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
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlRate_set(rtk_port_t port, rtk_qid_t queue, rtk_meter_id_t index);

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlRate_get
 * Description:
 *      Get rate of egress bandwidth control on specified queue.
 * Input:
 *      port  - port id
 *      queue - queue id
 *      pIndex - shared meter index
 * Output:
 *      pRate - pointer to rate of egress queue bandwidth control
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
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlRate_get(rtk_port_t port, rtk_qid_t queue, rtk_meter_id_t *pIndex);


/*Storm Control Rate*/
#if defined(EMBEDDED_SUPPORT)
extern rtk_api_ret_t rtk_storm_controlRate_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t rate, rtk_enable_t ifg_include, rtk_uint32 mode) reentrant;
extern rtk_api_ret_t rtk_storm_controlRate_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t *pRate, rtk_enable_t *pIfg_include, rtk_uint32 mode) reentrant;
#else
/* Function Name:
 *      rtk_storm_controlRate_set
 * Description:
 *      Set per-port storm filter control rate.
 * Input:
 *      port        - Port id
 *      storm_type  - Storm filter control type
 *      rate        - Rate of storm filter control
 *      ifg_include - include IFG or not, ENABLE:include DISABLE:exclude
 *      mode        - Mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              		- OK
 *      RT_ERR_FAILED          		- Failed
 *      RT_ERR_SMI             		- SMI access error
 *      RT_ERR_PORT_ID 				- Invalid port number.
 *      RT_ERR_INPUT 				- Invalid input parameters.
 *      RT_ERR_SFC_UNKNOWN_GROUP 	- unknown storm filter group.
 *      RT_ERR_ENABLE 				- Invalid IFG parameter
 *      RT_ERR_RATE 				- Invalid rate
 * Note:
 *      This API can set per-port stomr filter control rate.
 *      The storm filter control type can be:
 *          - STORM_GROUP_UNKNOWN_UNICAST
 *          - STORM_GROUP_UNKNOWN_MULTICAST
 *          - STORM_GROUP_MULTICAST
 *          - STORM_GROUP_BROADCAST
 *      The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
extern rtk_api_ret_t rtk_storm_controlRate_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t rate, rtk_enable_t ifg_include, rtk_uint32 mode);

/* Function Name:
 *      rtk_storm_controlRate_get
 * Description:
 *      Get per-port packet storm filter control rate.
 * Input:
 *      port        - Port id
 *      storm_type  - Storm filter control type.
 *      mode        - Mode
 * Output:
 *      pRate           - Rate of storm filter control.
 *      pIfg_include    - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK              		- OK
 *      RT_ERR_FAILED          		- Failed
 *      RT_ERR_SMI             		- SMI access error
 *      RT_ERR_PORT_ID 				- Invalid port number.
 *      RT_ERR_INPUT 				- Invalid input parameters.
 *      RT_ERR_SFC_UNKNOWN_GROUP 	- unknown storm filter group.
 * Note:
 *      The storm filter control type can be:
 *          - STORM_GROUP_UNKNOWN_UNICAST
 *          - STORM_GROUP_UNKNOWN_MULTICAST
 *          - STORM_GROUP_MULTICAST
 *          - STORM_GROUP_BROADCAST
 */
extern rtk_api_ret_t rtk_storm_controlRate_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t *pRate, rtk_enable_t *pIfg_include, rtk_uint32 mode);
#endif

/* Function Name:
 *      rtk_storm_bypass_set
 * Description:
 *      Set bypass storm filter control configuration.
 * Input:
 *      type    - Bypass storm filter control type.
 *      enable  - Bypass status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_ENABLE 		- Invalid IFG parameter
 * Note:
 *
 *      This API can set per-port bypass stomr filter control frame type including RMA and igmp.
 *      The bypass frame type is as following:
 *      - BYPASS_BRG_GROUP,
 *      - BYPASS_FD_PAUSE,
 *      - BYPASS_SP_MCAST,
 *      - BYPASS_1X_PAE,
 *      - BYPASS_UNDEF_BRG_04,
 *      - BYPASS_UNDEF_BRG_05,
 *      - BYPASS_UNDEF_BRG_06,
 *      - BYPASS_UNDEF_BRG_07,
 *      - BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - BYPASS_UNDEF_BRG_09,
 *      - BYPASS_UNDEF_BRG_0A,
 *      - BYPASS_UNDEF_BRG_0B,
 *      - BYPASS_UNDEF_BRG_0C,
 *      - BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - BYPASS_8021AB,
 *      - BYPASS_UNDEF_BRG_0F,
 *      - BYPASS_BRG_MNGEMENT,
 *      - BYPASS_UNDEFINED_11,
 *      - BYPASS_UNDEFINED_12,
 *      - BYPASS_UNDEFINED_13,
 *      - BYPASS_UNDEFINED_14,
 *      - BYPASS_UNDEFINED_15,
 *      - BYPASS_UNDEFINED_16,
 *      - BYPASS_UNDEFINED_17,
 *      - BYPASS_UNDEFINED_18,
 *      - BYPASS_UNDEFINED_19,
 *      - BYPASS_UNDEFINED_1A,
 *      - BYPASS_UNDEFINED_1B,
 *      - BYPASS_UNDEFINED_1C,
 *      - BYPASS_UNDEFINED_1D,
 *      - BYPASS_UNDEFINED_1E,
 *      - BYPASS_UNDEFINED_1F,
 *      - BYPASS_GMRP,
 *      - BYPASS_GVRP,
 *      - BYPASS_UNDEF_GARP_22,
 *      - BYPASS_UNDEF_GARP_23,
 *      - BYPASS_UNDEF_GARP_24,
 *      - BYPASS_UNDEF_GARP_25,
 *      - BYPASS_UNDEF_GARP_26,
 *      - BYPASS_UNDEF_GARP_27,
 *      - BYPASS_UNDEF_GARP_28,
 *      - BYPASS_UNDEF_GARP_29,
 *      - BYPASS_UNDEF_GARP_2A,
 *      - BYPASS_UNDEF_GARP_2B,
 *      - BYPASS_UNDEF_GARP_2C,
 *      - BYPASS_UNDEF_GARP_2D,
 *      - BYPASS_UNDEF_GARP_2E,
 *      - BYPASS_UNDEF_GARP_2F,
 *      - BYPASS_IGMP.
 */
extern rtk_api_ret_t rtk_storm_bypass_set(rtk_storm_bypass_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_storm_bypass_get
 * Description:
 *      Get bypass storm filter control configuration.
 * Input:
 *      type - Bypass storm filter control type.
 * Output:
 *      pEnable - Bypass status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can get per-port bypass stomr filter control frame type including RMA and igmp.
 *      The bypass frame type is as following:
 *      - BYPASS_BRG_GROUP,
 *      - BYPASS_FD_PAUSE,
 *      - BYPASS_SP_MCAST,
 *      - BYPASS_1X_PAE,
 *      - BYPASS_UNDEF_BRG_04,
 *      - BYPASS_UNDEF_BRG_05,
 *      - BYPASS_UNDEF_BRG_06,
 *      - BYPASS_UNDEF_BRG_07,
 *      - BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - BYPASS_UNDEF_BRG_09,
 *      - BYPASS_UNDEF_BRG_0A,
 *      - BYPASS_UNDEF_BRG_0B,
 *      - BYPASS_UNDEF_BRG_0C,
 *      - BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - BYPASS_8021AB,
 *      - BYPASS_UNDEF_BRG_0F,
 *      - BYPASS_BRG_MNGEMENT,
 *      - BYPASS_UNDEFINED_11,
 *      - BYPASS_UNDEFINED_12,
 *      - BYPASS_UNDEFINED_13,
 *      - BYPASS_UNDEFINED_14,
 *      - BYPASS_UNDEFINED_15,
 *      - BYPASS_UNDEFINED_16,
 *      - BYPASS_UNDEFINED_17,
 *      - BYPASS_UNDEFINED_18,
 *      - BYPASS_UNDEFINED_19,
 *      - BYPASS_UNDEFINED_1A,
 *      - BYPASS_UNDEFINED_1B,
 *      - BYPASS_UNDEFINED_1C,
 *      - BYPASS_UNDEFINED_1D,
 *      - BYPASS_UNDEFINED_1E,
 *      - BYPASS_UNDEFINED_1F,
 *      - BYPASS_GMRP,
 *      - BYPASS_GVRP,
 *      - BYPASS_UNDEF_GARP_22,
 *      - BYPASS_UNDEF_GARP_23,
 *      - BYPASS_UNDEF_GARP_24,
 *      - BYPASS_UNDEF_GARP_25,
 *      - BYPASS_UNDEF_GARP_26,
 *      - BYPASS_UNDEF_GARP_27,
 *      - BYPASS_UNDEF_GARP_28,
 *      - BYPASS_UNDEF_GARP_29,
 *      - BYPASS_UNDEF_GARP_2A,
 *      - BYPASS_UNDEF_GARP_2B,
 *      - BYPASS_UNDEF_GARP_2C,
 *      - BYPASS_UNDEF_GARP_2D,
 *      - BYPASS_UNDEF_GARP_2E,
 *      - BYPASS_UNDEF_GARP_2F,
 *      - BYPASS_IGMP.
 */
extern rtk_api_ret_t rtk_storm_bypass_get(rtk_storm_bypass_t type, rtk_enable_t *pEnable);

/* QoS */
/* Function Name:
 *      rtk_qos_init
 * Description:
 *      Configure Qos default settings with queue number assigment to each port.
 * Input:
 *      queueNum - Queue number of each port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_NUM 	- Invalid queue number.
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API will initialize related Qos setting with queue number assigment.
 *      The queue number is from 1 to 8.
 */
extern rtk_api_ret_t rtk_qos_init(rtk_queue_num_t queueNum);

/* Function Name:
 *      rtk_qos_priSel_set
 * Description:
 *      Configure the priority order among different priority mechanism.
 * Input:
 *      pPriDec - Priority assign for port, dscp, 802.1p, cvlan, svlan, acl based priority decision.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              		- OK
 *      RT_ERR_FAILED          		- Failed
 *      RT_ERR_SMI             		- SMI access error
 *      RT_ERR_QOS_SEL_PRI_SOURCE 	- Invalid priority decision source parameter.
 * Note:
 *      ASIC will follow user priority setting of mechanisms to select mapped queue priority for receiving frame.
 *      If two priority mechanisms are the same, the ASIC will chose the highest priority from mechanisms to
 *      assign queue priority to receiving frame.
 *      The priority sources are:
 *      - PRIDEC_PORT
 *      - PRIDEC_ACL
 *      - PRIDEC_DSCP
 *      - PRIDEC_1Q
 *      - PRIDEC_1AD
 *      - PRIDEC_CVLAN
 *      - PRIDEC_DA
 *      - PRIDEC_SA
 */
extern rtk_api_ret_t rtk_qos_priSel_set(rtk_priority_select_t *pPriDec);

/* Function Name:
 *      rtk_qos_priSel_get
 * Description:
 *      Get the priority order configuration among different priority mechanism.
 * Input:
 *      None
 * Output:
 *      pPriDec - Priority assign for port, dscp, 802.1p, cvlan, svlan, acl based priority decision .
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      ASIC will follow user priority setting of mechanisms to select mapped queue priority for receiving frame.
 *      If two priority mechanisms are the same, the ASIC will chose the highest priority from mechanisms to
 *      assign queue priority to receiving frame.
 *      The priority sources are:
 *      - PRIDEC_PORT,
 *      - PRIDEC_ACL,
 *      - PRIDEC_DSCP,
 *      - PRIDEC_1Q,
 *      - PRIDEC_1AD,
 *      - PRIDEC_CVLAN,
 *      - PRIDEC_DA,
 *      - PRIDEC_SA,
 */
extern rtk_api_ret_t rtk_qos_priSel_get(rtk_priority_select_t *pPriDec);

/* Function Name:
 *      rtk_qos_1pPriRemap_set
 * Description:
 *      Configure 1Q priorities mapping to internal absolute priority.
 * Input:
 *      dot1p_pri   - 802.1p priority value.
 *      int_pri     - internal priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_INPUT 			- Invalid input parameters.
 *      RT_ERR_VLAN_PRIORITY 	- Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of 802.1Q assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
extern rtk_api_ret_t rtk_qos_1pPriRemap_set(rtk_pri_t dot1p_pri, rtk_pri_t int_pri);

/* Function Name:
 *      rtk_qos_1pPriRemap_get
 * Description:
 *      Get 1Q priorities mapping to internal absolute priority.
 * Input:
 *      dot1p_pri - 802.1p priority value .
 * Output:
 *      pInt_pri - internal priority value.
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_VLAN_PRIORITY 	- Invalid priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of 802.1Q assigment for internal asic priority, and it is uesed for queue usage and packet scheduling.
 */
extern rtk_api_ret_t rtk_qos_1pPriRemap_get(rtk_pri_t dot1p_pri, rtk_pri_t *pInt_pri);

/* Function Name:
 *      rtk_qos_dscpPriRemap_set
 * Description:
 *      Map dscp value to internal priority.
 * Input:
 *      dscp    - Dscp value of receiving frame
 *      int_pri - internal priority value .
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_INPUT 			- Invalid input parameters.
 *      RT_ERR_QOS_DSCP_VALUE 	- Invalid DSCP value.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically
 *      greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of
 *      DSCP are carefully chosen then backward compatibility can be achieved.
 */
extern rtk_api_ret_t rtk_qos_dscpPriRemap_set(rtk_dscp_t dscp, rtk_pri_t int_pri);

/* Function Name:
 *      rtk_qos_dscpPriRemap_get
 * Description:
 *      Get dscp value to internal priority.
 * Input:
 *      dscp - Dscp value of receiving frame
 * Output:
 *      pInt_pri - internal priority value.
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_QOS_DSCP_VALUE 	- Invalid DSCP value.
 * Note:
 *      The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically
 *      greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of
 *      DSCP are carefully chosen then backward compatibility can be achieved.
 */
extern rtk_api_ret_t rtk_qos_dscpPriRemap_get(rtk_dscp_t dscp, rtk_pri_t *pInt_pri);

/* Function Name:
 *      rtk_qos_portPri_set
 * Description:
 *      Configure priority usage to each port.
 * Input:
 *      port - Port id.
 *      int_pri - internal priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_QOS_SEL_PORT_PRI - Invalid port priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The API can set priority of port assignments for queue usage and packet scheduling.
 */
extern rtk_api_ret_t rtk_qos_portPri_set(rtk_port_t port, rtk_pri_t int_pri) ;

/* Function Name:
 *      rtk_qos_portPri_get
 * Description:
 *      Get priority usage to each port.
 * Input:
 *      port - Port id.
 * Output:
 *      pInt_pri - internal priority value.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can get priority of port assignments for queue usage and packet scheduling.
 */
extern rtk_api_ret_t rtk_qos_portPri_get(rtk_port_t port, rtk_pri_t *pInt_pri) ;

/* Function Name:
 *      rtk_qos_queueNum_set
 * Description:
 *      Set output queue number for each port.
 * Input:
 *      port    - Port id.
 *      index   - Mapping queue number (1~8)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 *      RT_ERR_QUEUE_NUM 	- Invalid queue number.
 * Note:
 *      The API can set the output queue number of the specified port. The queue number is from 1 to 8.
 */
extern rtk_api_ret_t rtk_qos_queueNum_set(rtk_port_t port, rtk_queue_num_t queue_num);

/* Function Name:
 *      rtk_qos_queueNum_get
 * Description:
 *      Get output queue number.
 * Input:
 *      port - Port id.
 * Output:
 *      pQueue_num - Mapping queue number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 * Note:
 *      The API will return the output queue number of the specified port. The queue number is from 1 to 8.
 */
extern rtk_api_ret_t rtk_qos_queueNum_get(rtk_port_t port, rtk_queue_num_t *pQueue_num);

/* Function Name:
 *      rtk_qos_priMap_set
 * Description:
 *      Set output queue number for each port.
 * Input:
 *      queue_num   - Queue number usage.
 *      pPri2qid    - Priority mapping to queue ID.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_INPUT 			- Invalid input parameters.
 *      RT_ERR_QUEUE_NUM 		- Invalid queue number.
 *      RT_ERR_QUEUE_ID 		- Invalid queue id.
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      ASIC supports priority mapping to queue with different queue number from 1 to 8.
 *      For different queue numbers usage, ASIC supports different internal available queue IDs.
 */
extern rtk_api_ret_t rtk_qos_priMap_set(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid);


/* Function Name:
 *      rtk_qos_priMap_get
 * Description:
 *      Get priority to queue ID mapping table parameters.
 * Input:
 *      queue_num - Queue number usage.
 * Output:
 *      pPri2qid - Priority mapping to queue ID.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_QUEUE_NUM 	- Invalid queue number.
 * Note:
 *      The API can return the mapping queue id of the specified priority and queue number.
 *      The queue number is from 1 to 8.
 */
extern rtk_api_ret_t rtk_qos_priMap_get(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid);

/* Function Name:
 *      rtk_qos_schedulingQueue_set
 * Description:
 *      Set weight and type of queues in dedicated port.
 * Input:
 *      port        - Port id.
 *      pQweights   - The array of weights for WRR/WFQ queue (0 for STRICT_PRIORITY queue).
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_QOS_QUEUE_WEIGHT - Invalid queue weight.
 * Note:
 *      The API can set weight and type, strict priority or weight fair queue (WFQ) for
 *      dedicated port for using queues. If queue id is not included in queue usage,
 *      then its type and weight setting in dummy for setting. There are priorities
 *      as queue id in strict queues. It means strict queue id 5 carrying higher priority
 *      than strict queue id 4. The WFQ queue weight is from 1 to 128, and weight 0 is
 *      for strict priority queue type.
 */
extern rtk_api_ret_t rtk_qos_schedulingQueue_set(rtk_port_t port, rtk_qos_queue_weights_t *pQweights);

/* Function Name:
 *      rtk_qos_schedulingQueue_get
 * Description:
 *      Get weight and type of queues in dedicated port.
 * Input:
 *      port - Port id.
 * Output:
 *      pQweights - The array of weights for WRR/WFQ queue (0 for STRICT_PRIORITY queue).
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_PORT_ID 		- Invalid port number.
 * Note:
 *      The API can get weight and type, strict priority or weight fair queue (WFQ) for dedicated port for using queues.
 *      The WFQ queue weight is from 1 to 128, and weight 0 is for strict priority queue type.
 */
extern rtk_api_ret_t rtk_qos_schedulingQueue_get(rtk_port_t port, rtk_qos_queue_weights_t *pQweights);

/* Function Name:
 *      rtk_qos_1pRemarkEnable_set
 * Description:
 *      Set 1p Remarking state
 * Input:
 *      port        - Port id.
 *      enable      - State of per-port 1p Remarking
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 *      RT_ERR_ENABLE 		- Invalid enable parameter.
 * Note:
 *      The API can enable or disable 802.1p remarking ability for whole system.
 *      The status of 802.1p remark:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_qos_1pRemarkEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_qos_1pRemarkEnable_get
 * Description:
 *      Get 802.1p remarking ability.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Status of 802.1p remark.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 * Note:
 *      The API can get 802.1p remarking ability.
 *      The status of 802.1p remark:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_qos_1pRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_qos_1pRemark_set
 * Description:
 *      Set 802.1p remarking parameter.
 * Input:
 *      int_pri     - Internal priority value.
 *      dot1p_pri   - 802.1p priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_VLAN_PRIORITY 	- Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The API can set 802.1p parameters source priority and new priority.
 */
extern rtk_api_ret_t rtk_qos_1pRemark_set(rtk_pri_t int_pri, rtk_pri_t dot1p_pri);

/* Function Name:
 *      rtk_qos_1pRemark_get
 * Description:
 *      Get 802.1p remarking parameter.
 * Input:
 *      int_pri - Internal priority value.
 * Output:
 *      pDot1p_pri - 802.1p priority value.
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The API can get 802.1p remarking parameters. It would return new priority of ingress priority.
 */
extern rtk_api_ret_t rtk_qos_1pRemark_get(rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri);

/* Function Name:
 *      rtk_qos_dscpRemarkEnable_set
 * Description:
 *      Set DSCP remarking ability.
 * Input:
 *      port    - Port id.
 *      enable  - status of DSCP remark.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_ENABLE 			- Invalid enable parameter.
 * Note:
 *      The API can enable or disable DSCP remarking ability for whole system.
 *      The status of DSCP remark:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_qos_dscpRemarkEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_qos_dscpRemarkEnable_get
 * Description:
 *      Get DSCP remarking ability.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - status of DSCP remarking.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 * Note:
 *      The API can get DSCP remarking ability.
 *      The status of DSCP remark:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_qos_dscpRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_qos_dscpRemark_set
 * Description:
 *      Set DSCP remarking parameter.
 * Input:
 *      int_pri - Internal priority value.
 *      dscp    - DSCP value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_QOS_DSCP_VALUE 	- Invalid DSCP value.
 * Note:
 *      The API can set DSCP value and mapping priority.
 */
extern rtk_api_ret_t rtk_qos_dscpRemark_set(rtk_pri_t int_pri, rtk_dscp_t dscp);

/* Function Name:
 *      rtk_qos_dscpRemark_get
 * Description:
 *      Get DSCP remarking parameter.
 * Input:
 *      int_pri - Internal priority value.
 * Output:
 *      Dscp - DSCP value.
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      The API can get DSCP parameters. It would return DSCP value for mapping priority.
 */
extern rtk_api_ret_t rtk_qos_dscpRemark_get(rtk_pri_t int_pri, rtk_dscp_t *pDscp);

/* Function Name:
 *      rtk_trap_unknownUnicastPktAction_set
 * Description:
 *      Set unknown unicast packet action configuration.
 * Input:
 *      type            - Unknown unicast packet type.
 *      ucast_action    - Unknown unicast action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_NOT_ALLOWED 		- Invalid action.
 *      RT_ERR_INPUT 			- Invalid input parameters.
 * Note:
 *      This API can set unknown unicast packet action configuration.
 *      (1) The unknown unicast packet type is as following:
 *          - UCAST_UNKNOWNDA
 *          - UCAST_UNKNOWNSA
 *          - UCAST_UNMATCHSA
 *      (2) The unknown unicast action is as following:
 *          - UCAST_ACTION_FORWARD
 *          - UCAST_ACTION_DROP
 *          - UCAST_ACTION_TRAP2CPU
 */
extern rtk_api_ret_t rtk_trap_unknownUnicastPktAction_set(rtk_trap_ucast_type_t type, rtk_trap_ucast_action_t ucast_action);

/* Function Name:
 *      rtk_trap_unknownUnicastPktAction_get
 * Description:
 *      Get unknown unicast packet action configuration.
 * Input:
 *      type - unknown unicast packet type.
 * Output:
 *      pUcast_action - unknown unicast action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can get unknown unicast packet action configuration.
 *      (1) The unknown unicast packet type is as following:
 *          - UCAST_UNKNOWNDA
 *          - UCAST_UNKNOWNSA
 *          - UCAST_UNMATCHSA
 *      (2) The unknown unicast action is as following:
 *          - UCAST_ACTION_FORWARD
 *          - UCAST_ACTION_DROP
 *          - UCAST_ACTION_TRAP2CPU
 */
extern rtk_api_ret_t rtk_trap_unknownUnicastPktAction_get(rtk_trap_ucast_type_t type, rtk_trap_ucast_action_t *pUcast_action);

/* Function Name:
 *      rtk_trap_unknownMcastPktAction_set
 * Description:
 *      Set behavior of unknown multicast
 * Input:
 *      port            - Port id.
 *      type            - unknown multicast packet type.
 *      mcast_action    - unknown multicast action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 *      RT_ERR_NOT_ALLOWED 	- Invalid action.
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop or flood this packet
 *      (1) The unknown multicast packet type is as following:
 *          - MCAST_L2
 *          - MCAST_IPV4
 *          - MCAST_IPV6
 *      (2) The unknown multicast action is as following:
 *          - MCAST_ACTION_FORWARD
 *          - MCAST_ACTION_DROP
 *          - MCAST_ACTION_TRAP2CPU
 */
extern rtk_api_ret_t rtk_trap_unknownMcastPktAction_set(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t mcast_action);

/* Function Name:
 *      rtk_trap_unknownMcastPktAction_get
 * Description:
 *      Get behavior of unknown multicast
 * Input:
 *      type - unknown multicast packet type.
 * Output:
 *      pMcast_action - unknown multicast action.
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_NOT_ALLOWED 		- Invalid operation.
 *      RT_ERR_INPUT 			- Invalid input parameters.
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop or flood this packet
 *      (1) The unknown multicast packet type is as following:
 *          - MCAST_L2
 *          - MCAST_IPV4
 *          - MCAST_IPV6
 *      (2) The unknown multicast action is as following:
 *          - MCAST_ACTION_FORWARD
 *          - MCAST_ACTION_DROP
 *          - MCAST_ACTION_TRAP2CPU
 */
extern rtk_api_ret_t rtk_trap_unknownMcastPktAction_get(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t *pMcast_action);

/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_set
 * Description:
 *      Set IGMP/MLD trap function
 * Input:
 *      type        - IGMP/MLD packet type.
 *      igmp_action - IGMP/MLD action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_NOT_ALLOWED 	- Invalid igmp action.
 * Note:
 *      This API can set both IPv4 IGMP/IPv6 MLD with/without PPPoE header trapping function.
 *      All 4 kinds of IGMP/MLD function can be set seperately.
 *      (1) The IGMP/MLD packet type is as following:
 *          - IGMP_IPV4
 *          - IGMP_PPPOE_IPV4
 *          - IGMP_MLD
 *          - IGMP_PPPOE_MLD
 *      (2) The IGMP/MLD action is as following:
 *          - IGMP_ACTION_FORWARD
 *          - IGMP_ACTION_TRAP2CPU
 *          - IGMP_ACTION_DROP
 *          - IGMP_ACTION_FORWARD_EXCLUDE_CPU
 */
extern rtk_api_ret_t rtk_trap_igmpCtrlPktAction_set(rtk_igmp_type_t type, rtk_trap_igmp_action_t igmp_action);

/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_get
 * Description:
 *      Get IGMP/MLD trap function
 * Input:
 *      type - IGMP/MLD packet type.
 * Output:
 *      pIgmp_action - IGMP/MLD action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can get both IPv4 IGMP/IPv6 MLD with/without PPPoE header trapping function.
 *      (1) The IGMP/MLD packet type is as following:
 *          - IGMP_IPV4
 *          - IGMP_PPPOE_IPV4
 *          - IGMP_MLD
 *          - IGMP_PPPOE_MLD
 *      (2) The IGMP/MLD action is as following:
 *          - IGMP_ACTION_FORWARD
 *          - IGMP_ACTION_TRAP2CPU
 *          - IGMP_ACTION_DROP
 *          - IGMP_ACTION_FORWARD_EXCLUDE_CPU
 */
extern rtk_api_ret_t rtk_trap_igmpCtrlPktAction_get(rtk_igmp_type_t type, rtk_trap_igmp_action_t *pIgmp_action);

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
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_NOT_ALLOWED 		- Invalid action.
 *      RT_ERR_INPUT 			- Invalid input parameters.
 * Note:
 *      There are 48 types of Reserved Multicast Address frame for application usage.
 *      (1) They are as following definition.
 *      - DMAC                        Assignment
 *      - 01-80-C2-00-00-00           Bridge Group Address
 *      - 01-80-C2-00-00-01           IEEE Std 802.3, 1988 Edition Full Duplex PAUSE operation
 *      - 01-80-C2-00-00-02           IEEE Std 802.3ad Slow Protocols-Multicast Address
 *      - 01-80-C2-00-00-03           IEEE Std 802.1X PAE address
 *      - 01-80-C2-00-00-04           Undefined 802.1 bridge address 04
 *      - 01-80-C2-00-00-05           Undefined 802.1 bridge address 05
 *      - 01-80-C2-00-00-06           Undefined 802.1 bridge address 06
 *      - 01-80-C2-00-00-07           Undefined 802.1 bridge address 07
 *      - 01-80-C2-00-00-08           Provider Bridge Group Address
 *      - 01-80-C2-00-00-09           Undefined 802.1 bridge address 09
 *      - 01-80-C2-00-00-0A           Undefined 802.1 bridge address 0A
 *      - 01-80-C2-00-00-0B           Undefined 802.1 bridge address 0B
 *      - 01-80-C2-00-00-0C           Undefined 802.1 bridge address 0C
 *      - 01-80-C2-00-00-0D           Provider Bridge GVRP Address
 *      - 01-80-C2-00-00-0E           IEEE Std 802.1ab Link Layer Discovery Protocol Multicast address
 *      - 01-80-C2-00-00-0F           Undefined 802.1 bridge address
 *      - 01-80-C2-00-00-10           All LANs Bridge Management Group Address
 *      - 01-80-C2-00-00-11~1F        Undefined address 11~1F
 *      - 01-80-C2-00-00-20           GMRP Address
 *      - 01-80-C2-00-00-21           GVRP address
 *      - 01-80-C2-00-00-22~2F        Undefined GARP address 22~2F
 *      (2) The RMA action is as following:
 *      - RMA_ACTION_FORWARD
 *      - RMA_ACTION_TRAP2CPU
 *      - RMA_ACTION_DROP
 *      - RMA_ACTION_FORWARD_EXCLUDE_CPU
 */
extern rtk_api_ret_t rtk_trap_rmaAction_set(rtk_mac_t *pRma_frame, rtk_trap_rma_action_t rma_action);

/* Function Name:
 *      rtk_trap_rmaAction_get
 * Description:
 *      Get reserved multicast address frame trap to CPU.
 * Input:
 *      type - unknown unicast packet type.
 * Output:
 *      pRma_action - RMA action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can retrieved RMA configuration.
 *      (1) They are as following definition.
 *      - DMAC                        Assignment
 *      - 01-80-C2-00-00-00           Bridge Group Address
 *      - 01-80-C2-00-00-01           IEEE Std 802.3, 1988 Edition Full Duplex PAUSE operation
 *      - 01-80-C2-00-00-02           IEEE Std 802.3ad Slow Protocols-Multicast Address
 *      - 01-80-C2-00-00-03           IEEE Std 802.1X PAE address
 *      - 01-80-C2-00-00-04           Undefined 802.1 bridge address 04
 *      - 01-80-C2-00-00-05           Undefined 802.1 bridge address 05
 *      - 01-80-C2-00-00-06           Undefined 802.1 bridge address 06
 *      - 01-80-C2-00-00-07           Undefined 802.1 bridge address 07
 *      - 01-80-C2-00-00-08           Provider Bridge Group Address
 *      - 01-80-C2-00-00-09           Undefined 802.1 bridge address 09
 *      - 01-80-C2-00-00-0A           Undefined 802.1 bridge address 0A
 *      - 01-80-C2-00-00-0B           Undefined 802.1 bridge address 0B
 *      - 01-80-C2-00-00-0C           Undefined 802.1 bridge address 0C
 *      - 01-80-C2-00-00-0D           Provider Bridge GVRP Address
 *      - 01-80-C2-00-00-0E           IEEE Std 802.1ab Link Layer Discovery Protocol Multicast address
 *      - 01-80-C2-00-00-0F           Undefined 802.1 bridge address
 *      - 01-80-C2-00-00-10           All LANs Bridge Management Group Address
 *      - 01-80-C2-00-00-11~1F        Undefined address 11~1F
 *      - 01-80-C2-00-00-20           GMRP Address
 *      - 01-80-C2-00-00-21           GVRP address
 *      - 01-80-C2-00-00-22~2F        Undefined GARP address 22~2F
 *      (2) The RMA action is as following:
 *      - RMA_ACTION_FORWARD
 *      - RMA_ACTION_TRAP2CPU
 *      - RMA_ACTION_DROP
 *      - RMA_ACTION_FORWARD_EXCLUDE_CPU
 */
extern rtk_api_ret_t rtk_trap_rmaAction_get(rtk_mac_t *pRma_frame, rtk_trap_rma_action_t *pRma_action);

/* Function Name:
 *      rtk_trap_ethernetAv_set
 * Description:
 *      Set Ethetnet AV.
 * Input:
 *      enable - enable trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 *      RT_ERR_ENABLE 			- Invalid enable parameter.
 * Note:
 *      The API can enable or disable ethernet AV function. If the function is enabled,
 *      packets with ethernet type 0x88F7 will be trap to CPU with time stamp.
 *      The status of Ethernet AV:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_trap_ethernetAv_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_trap_ethernetAv_get
 * Description:
 *      Get ethernet AV setup.
 * Input:
 * Output:
 *      pEnable - status of ethernet AV.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 * Note:
 *      The API can get ethernet AV status. If the function is enabled,
 *      packets with ethernet type 0x88F7 will be trap to CPU with time stamp.
 *      The status of Ethernet AV:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_trap_ethernetAv_get(rtk_enable_t *pEnable);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      This API can set VLAN leaky for RMA and IGMP/MLD packets.
 *      The leaky frame types are as following:
 *      - LEAKY_BRG_GROUP,
 *      - LEAKY_FD_PAUSE,
 *      - LEAKY_SP_MCAST,
 *      - LEAKY_1X_PAE,
 *      - LEAKY_UNDEF_BRG_04,
 *      - LEAKY_UNDEF_BRG_05,
 *      - LEAKY_UNDEF_BRG_06,
 *      - LEAKY_UNDEF_BRG_07,
 *      - LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - LEAKY_UNDEF_BRG_09,
 *      - LEAKY_UNDEF_BRG_0A,
 *      - LEAKY_UNDEF_BRG_0B,
 *      - LEAKY_UNDEF_BRG_0C,
 *      - LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - LEAKY_8021AB,
 *      - LEAKY_UNDEF_BRG_0F,
 *      - LEAKY_BRG_MNGEMENT,
 *      - LEAKY_UNDEFINED_11,
 *      - LEAKY_UNDEFINED_12,
 *      - LEAKY_UNDEFINED_13,
 *      - LEAKY_UNDEFINED_14,
 *      - LEAKY_UNDEFINED_15,
 *      - LEAKY_UNDEFINED_16,
 *      - LEAKY_UNDEFINED_17,
 *      - LEAKY_UNDEFINED_18,
 *      - LEAKY_UNDEFINED_19,
 *      - LEAKY_UNDEFINED_1A,
 *      - LEAKY_UNDEFINED_1B,
 *      - LEAKY_UNDEFINED_1C,
 *      - LEAKY_UNDEFINED_1D,
 *      - LEAKY_UNDEFINED_1E,
 *      - LEAKY_UNDEFINED_1F,
 *      - LEAKY_GMRP,
 *      - LEAKY_GVRP,
 *      - LEAKY_UNDEF_GARP_22,
 *      - LEAKY_UNDEF_GARP_23,
 *      - LEAKY_UNDEF_GARP_24,
 *      - LEAKY_UNDEF_GARP_25,
 *      - LEAKY_UNDEF_GARP_26,
 *      - LEAKY_UNDEF_GARP_27,
 *      - LEAKY_UNDEF_GARP_28,
 *      - LEAKY_UNDEF_GARP_29,
 *      - LEAKY_UNDEF_GARP_2A,
 *      - LEAKY_UNDEF_GARP_2B,
 *      - LEAKY_UNDEF_GARP_2C,
 *      - LEAKY_UNDEF_GARP_2D,
 *      - LEAKY_UNDEF_GARP_2E,
 *      - LEAKY_UNDEF_GARP_2F,
 *      - LEAKY_IGMP,
 *      - LEAKY_IPMULTICAST.
 */
extern rtk_api_ret_t rtk_leaky_vlan_set(rtk_leaky_type_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_leaky_vlan_get
 * Description:
 *      Get VLAN leaky.
 * Input:
 *      type - Packet type for VLAN leaky.
 * Output:
 *      pEnable - Leaky status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can get VLAN leaky status for RMA and IGMP/MLD packets.
 *      The leaky frame types are as following:
 *      - LEAKY_BRG_GROUP,
 *      - LEAKY_FD_PAUSE,
 *      - LEAKY_SP_MCAST,
 *      - LEAKY_1X_PAE,
 *      - LEAKY_UNDEF_BRG_04,
 *      - LEAKY_UNDEF_BRG_05,
 *      - LEAKY_UNDEF_BRG_06,
 *      - LEAKY_UNDEF_BRG_07,
 *      - LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - LEAKY_UNDEF_BRG_09,
 *      - LEAKY_UNDEF_BRG_0A,
 *      - LEAKY_UNDEF_BRG_0B,
 *      - LEAKY_UNDEF_BRG_0C,
 *      - LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - LEAKY_8021AB,
 *      - LEAKY_UNDEF_BRG_0F,
 *      - LEAKY_BRG_MNGEMENT,
 *      - LEAKY_UNDEFINED_11,
 *      - LEAKY_UNDEFINED_12,
 *      - LEAKY_UNDEFINED_13,
 *      - LEAKY_UNDEFINED_14,
 *      - LEAKY_UNDEFINED_15,
 *      - LEAKY_UNDEFINED_16,
 *      - LEAKY_UNDEFINED_17,
 *      - LEAKY_UNDEFINED_18,
 *      - LEAKY_UNDEFINED_19,
 *      - LEAKY_UNDEFINED_1A,
 *      - LEAKY_UNDEFINED_1B,
 *      - LEAKY_UNDEFINED_1C,
 *      - LEAKY_UNDEFINED_1D,
 *      - LEAKY_UNDEFINED_1E,
 *      - LEAKY_UNDEFINED_1F,
 *      - LEAKY_GMRP,
 *      - LEAKY_GVRP,
 *      - LEAKY_UNDEF_GARP_22,
 *      - LEAKY_UNDEF_GARP_23,
 *      - LEAKY_UNDEF_GARP_24,
 *      - LEAKY_UNDEF_GARP_25,
 *      - LEAKY_UNDEF_GARP_26,
 *      - LEAKY_UNDEF_GARP_27,
 *      - LEAKY_UNDEF_GARP_28,
 *      - LEAKY_UNDEF_GARP_29,
 *      - LEAKY_UNDEF_GARP_2A,
 *      - LEAKY_UNDEF_GARP_2B,
 *      - LEAKY_UNDEF_GARP_2C,
 *      - LEAKY_UNDEF_GARP_2D,
 *      - LEAKY_UNDEF_GARP_2E,
 *      - LEAKY_UNDEF_GARP_2F,
 *      - LEAKY_IGMP,
 *      - LEAKY_IPMULTICAST.
 */
extern rtk_api_ret_t rtk_leaky_vlan_get(rtk_leaky_type_t type, rtk_enable_t *pEnable);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      This API can set port isolation leaky for RMA and IGMP/MLD packets.
 *      The leaky frame types are as following:
 *      - LEAKY_BRG_GROUP,
 *      - LEAKY_FD_PAUSE,
 *      - LEAKY_SP_MCAST,
 *      - LEAKY_1X_PAE,
 *      - LEAKY_UNDEF_BRG_04,
 *      - LEAKY_UNDEF_BRG_05,
 *      - LEAKY_UNDEF_BRG_06,
 *      - LEAKY_UNDEF_BRG_07,
 *      - LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - LEAKY_UNDEF_BRG_09,
 *      - LEAKY_UNDEF_BRG_0A,
 *      - LEAKY_UNDEF_BRG_0B,
 *      - LEAKY_UNDEF_BRG_0C,
 *      - LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - LEAKY_8021AB,
 *      - LEAKY_UNDEF_BRG_0F,
 *      - LEAKY_BRG_MNGEMENT,
 *      - LEAKY_UNDEFINED_11,
 *      - LEAKY_UNDEFINED_12,
 *      - LEAKY_UNDEFINED_13,
 *      - LEAKY_UNDEFINED_14,
 *      - LEAKY_UNDEFINED_15,
 *      - LEAKY_UNDEFINED_16,
 *      - LEAKY_UNDEFINED_17,
 *      - LEAKY_UNDEFINED_18,
 *      - LEAKY_UNDEFINED_19,
 *      - LEAKY_UNDEFINED_1A,
 *      - LEAKY_UNDEFINED_1B,
 *      - LEAKY_UNDEFINED_1C,
 *      - LEAKY_UNDEFINED_1D,
 *      - LEAKY_UNDEFINED_1E,
 *      - LEAKY_UNDEFINED_1F,
 *      - LEAKY_GMRP,
 *      - LEAKY_GVRP,
 *      - LEAKY_UNDEF_GARP_22,
 *      - LEAKY_UNDEF_GARP_23,
 *      - LEAKY_UNDEF_GARP_24,
 *      - LEAKY_UNDEF_GARP_25,
 *      - LEAKY_UNDEF_GARP_26,
 *      - LEAKY_UNDEF_GARP_27,
 *      - LEAKY_UNDEF_GARP_28,
 *      - LEAKY_UNDEF_GARP_29,
 *      - LEAKY_UNDEF_GARP_2A,
 *      - LEAKY_UNDEF_GARP_2B,
 *      - LEAKY_UNDEF_GARP_2C,
 *      - LEAKY_UNDEF_GARP_2D,
 *      - LEAKY_UNDEF_GARP_2E,
 *      - LEAKY_UNDEF_GARP_2F,
 *      - LEAKY_IGMP,
 *      - LEAKY_IPMULTICAST.
 */
extern rtk_api_ret_t rtk_leaky_portIsolation_set(rtk_leaky_type_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_leaky_portIsolation_get
 * Description:
 *      Get port isolation leaky.
 * Input:
 *      type - Packet type for port isolation leaky.
 * Output:
 *      pEnable - Leaky status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can get port isolation leaky status for RMA and IGMP/MLD packets.
 *      The leaky frame types are as following:
 *      - LEAKY_BRG_GROUP,
 *      - LEAKY_FD_PAUSE,
 *      - LEAKY_SP_MCAST,
 *      - LEAKY_1X_PAE,
 *      - LEAKY_UNDEF_BRG_04,
 *      - LEAKY_UNDEF_BRG_05,
 *      - LEAKY_UNDEF_BRG_06,
 *      - LEAKY_UNDEF_BRG_07,
 *      - LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - LEAKY_UNDEF_BRG_09,
 *      - LEAKY_UNDEF_BRG_0A,
 *      - LEAKY_UNDEF_BRG_0B,
 *      - LEAKY_UNDEF_BRG_0C,
 *      - LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - LEAKY_8021AB,
 *      - LEAKY_UNDEF_BRG_0F,
 *      - LEAKY_BRG_MNGEMENT,
 *      - LEAKY_UNDEFINED_11,
 *      - LEAKY_UNDEFINED_12,
 *      - LEAKY_UNDEFINED_13,
 *      - LEAKY_UNDEFINED_14,
 *      - LEAKY_UNDEFINED_15,
 *      - LEAKY_UNDEFINED_16,
 *      - LEAKY_UNDEFINED_17,
 *      - LEAKY_UNDEFINED_18,
 *      - LEAKY_UNDEFINED_19,
 *      - LEAKY_UNDEFINED_1A,
 *      - LEAKY_UNDEFINED_1B,
 *      - LEAKY_UNDEFINED_1C,
 *      - LEAKY_UNDEFINED_1D,
 *      - LEAKY_UNDEFINED_1E,
 *      - LEAKY_UNDEFINED_1F,
 *      - LEAKY_GMRP,
 *      - LEAKY_GVRP,
 *      - LEAKY_UNDEF_GARP_22,
 *      - LEAKY_UNDEF_GARP_23,
 *      - LEAKY_UNDEF_GARP_24,
 *      - LEAKY_UNDEF_GARP_25,
 *      - LEAKY_UNDEF_GARP_26,
 *      - LEAKY_UNDEF_GARP_27,
 *      - LEAKY_UNDEF_GARP_28,
 *      - LEAKY_UNDEF_GARP_29,
 *      - LEAKY_UNDEF_GARP_2A,
 *      - LEAKY_UNDEF_GARP_2B,
 *      - LEAKY_UNDEF_GARP_2C,
 *      - LEAKY_UNDEF_GARP_2D,
 *      - LEAKY_UNDEF_GARP_2E,
 *      - LEAKY_UNDEF_GARP_2F,
 *      - LEAKY_IGMP,
 *      - LEAKY_IPMULTICAST.
 */
extern rtk_api_ret_t rtk_leaky_portIsolation_get(rtk_leaky_type_t type, rtk_enable_t *pEnable);

/* Port and PHY setting */
/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set ethernet PHY auto-negotiation desired ability.
 * Input:
 *      port        - port id.
 *      pAbility    - Ability structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_PHY_REG_ID 		- Invalid PHY address
 *      RT_ERR_INPUT 			- Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1. While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 */
extern rtk_api_ret_t rtk_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get PHY ability through PHY registers.
 * Input:
 *      port - Port id.
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_PHY_REG_ID 		- Invalid PHY address
 *      RT_ERR_INPUT 			- Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get the capablity of specified PHY.
 */
extern rtk_api_ret_t rtk_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyForceModeAbility_set
 * Description:
 *      Set the port speed/duplex mode/pause/asy_pause in the PHY force mode.
 * Input:
 *      port        - port id.
 *      pAbility    - Ability structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_PHY_REG_ID 		- Invalid PHY address
 *      RT_ERR_INPUT 			- Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1. While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 */
extern rtk_api_ret_t rtk_port_phyForceModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyStatus_get
 * Description:
 *      Get ethernet PHY linking status
 * Input:
 *      port - Port id.
 * Output:
 *      linkStatus  - PHY link status
 *      speed       - PHY link speed
 *      duplex      - PHY duplex mode
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_PHY_REG_ID 		- Invalid PHY address
 *      RT_ERR_INPUT 			- Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      API will return auto negotiation status of phy.
 */
extern rtk_api_ret_t rtk_port_phyStatus_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex);

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
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Set PHY in test mode and only one PHY can be in test mode at the same time.
 *      It means API will return FALED if other PHY is in test mode.
 *      This API only provide test mode 1 setup, and if users want other test modes,
 *      please contact realtek FAE.
 */
extern rtk_api_ret_t rtk_port_phyTestMode_set(rtk_port_t port, rtk_port_phy_test_mode_t mode);

/* Function Name:
 *      rtk_port_phyTestMode_get
 * Description:
 *      Get PHY in which test mode.
 * Input:
 *      port - Port id.
 * Output:
 *      mode - PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_INPUT 			- Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get test mode of PHY from register setting 9.15 to 9.13.
 */
extern rtk_api_ret_t rtk_port_phyTestMode_get(rtk_port_t port, rtk_port_phy_test_mode_t *pMode);

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
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_PORT_ID 			- Invalid port number.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 *      RT_ERR_ENABLE 			- Invalid enable input.
 * Note:
 *      Set enable/disable MASTER/SLAVE manual configuration under 1000Base-T with register 9.12-9.11. If MASTER/SLAVE manual configuration is enabled with MASTER, the
 *      link partner must be set as SLAVE or auto negotiation will fail.
 */
extern rtk_api_ret_t rtk_port_phy1000BaseTMasterSlave_set(rtk_port_t port, rtk_enable_t enabled, rtk_enable_t masterslave);

/* Function Name:
 *      rtk_port_macForceLink_set
 * Description:
 *      Set port force linking configuration.
 * Input:
 *      port            - port id.
 *      pPortability    - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 * Note:
 *      This API can set Port/MAC force mode properties.
 */
extern rtk_api_ret_t rtk_port_macForceLink_set(rtk_port_t port, rtk_port_mac_ability_t *pPortability);

/* Function Name:
 *      rtk_port_macForceLink_get
 * Description:
 *      Get port force linking configuration.
 * Input:
 *      port - Port id.
 * Output:
 *      pPortability - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can get Port/MAC force mode properties.
 */
extern rtk_api_ret_t rtk_port_macForceLink_get(rtk_port_t port, rtk_port_mac_ability_t *pPortability);

/* Function Name:
 *      rtk_port_macForceLinkExt0_set
 * Description:
 *      Set external interface 0 force linking configuration.
 * Input:
 *      mode            - external interface mode
 *      pPortability    - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can set external interface 0 force mode properties.
 *      The external interface can be set to:
 *      - MODE_EXT_DISABLE,
 *      - MODE_EXT_RGMII,
 *      - MODE_EXT_MII_MAC,
 *      - MODE_EXT_MII_PHY,
 *      - MODE_EXT_TMII_MAC,
 *      - MODE_EXT_TMII_PHY,
 *      - MODE_EXT_GMII,
 *      - MODE_EXT_RMII_MAC,
 *      - MODE_EXT_RMII_PHY,
 */
extern rtk_api_ret_t rtk_port_macForceLinkExt0_set(rtk_mode_ext_t mode, rtk_port_mac_ability_t *pPortability);

/* Function Name:
 *      rtk_port_macForceLinkExt0_get
 * Description:
 *      Get external interface 0 force linking configuration.
 * Input:
 *      None
 * Output:
 *      pMode           - external interface mode
 *      pPortability    - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can get external interface 0 force mode properties.
 */
extern rtk_api_ret_t rtk_port_macForceLinkExt0_get(rtk_mode_ext_t *pMode, rtk_port_mac_ability_t *pPortability);

/* Function Name:
 *      rtk_port_macForceLinkExt1_set
 * Description:
 *      Set external interface 1 force linking configuration.
 * Input:
 *      mode            - external interface mode
 *      pPortability    - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can set external interface 1 force mode properties.
 *      The external interface can be set to:
 *      - MODE_EXT_DISABLE,
 *      - MODE_EXT_RGMII,
 *      - MODE_EXT_MII_MAC,
 *      - MODE_EXT_MII_PHY,
 *      - MODE_EXT_TMII_MAC,
 *      - MODE_EXT_TMII_PHY,
 *      - MODE_EXT_GMII,
 *      - MODE_EXT_RMII_MAC,
 *      - MODE_EXT_RMII_PHY,
 */
extern rtk_api_ret_t rtk_port_macForceLinkExt1_set(rtk_mode_ext_t mode, rtk_port_mac_ability_t *pPortability);

/* Function Name:
 *      rtk_port_macForceLinkExt1_get
 * Description:
 *      Get external interface 1 force linking configuration.
 * Input:
 *      None
 * Output:
 *      pMode           - external interface mode
 *      pPortability    - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can get external interface 1 force mode properties.
 */
extern rtk_api_ret_t rtk_port_macForceLinkExt1_get(rtk_mode_ext_t *pMode, rtk_port_mac_ability_t *pPortability);

/* Function Name:
 *      rtk_port_macForceLinkExt_set
 * Description:
 *      Set external interface force linking configuration.
 * Input:
 *      port            - external port ID
 *      mode            - external interface mode
 *      pPortability    - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can set external interface force mode properties.
 *      The external interface can be set to:
 *      - MODE_EXT_DISABLE,
 *      - MODE_EXT_RGMII,
 *      - MODE_EXT_MII_MAC,
 *      - MODE_EXT_MII_PHY,
 *      - MODE_EXT_TMII_MAC,
 *      - MODE_EXT_TMII_PHY,
 *      - MODE_EXT_GMII,
 *      - MODE_EXT_RMII_MAC,
 *      - MODE_EXT_RMII_PHY,
 */
extern rtk_api_ret_t rtk_port_macForceLinkExt_set(rtk_ext_port_t port, rtk_mode_ext_t mode, rtk_port_mac_ability_t *pPortability);

/* Function Name:
 *      rtk_port_macForceLinkExt_get
 * Description:
 *      Set external interface force linking configuration.
 * Input:
 *      port            - external port ID
 * Output:
 *      pMode           - external interface mode
 *      pPortability    - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can get external interface force mode properties.
 */
extern rtk_api_ret_t rtk_port_macForceLinkExt_get(rtk_ext_port_t port, rtk_mode_ext_t *pMode, rtk_port_mac_ability_t *pPortability);

/* Function Name:
 *      rtk_port_macStatus_get
 * Description:
 *      Get port link status.
 * Input:
 *      port - Port id.
 * Output:
 *      pPortstatus - port ability configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 * Note:
 *      This API can get Port/PHY properties.
 */
extern rtk_api_ret_t rtk_port_macStatus_get(rtk_port_t port, rtk_port_mac_ability_t *pPortstatus);

/* Function Name:
 *      rtk_port_macLocalLoopbackEnable_set
 * Description:
 *      Set Port Local Loopback. (Redirect TX to RX.)
 * Input:
 *      port    - Port id.
 *      enable  - Loopback state, 0:disable, 1:enable
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 * Note:
 *      This API can enable/disable Local loopback in MAC.
 *      For UTP port, This API will also enable the digital
 *      loopback bit in PHY register for sync of speed between
 *      PHY and MAC. For EXT port, users need to force the
 *      link state by themself.
 */
extern rtk_api_ret_t rtk_port_macLocalLoopbackEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_macLocalLoopbackEnable_get
 * Description:
 *      Get Port Local Loopback. (Redirect TX to RX.)
 * Input:
 *      port    - Port id.
 * Output:
 *      pEnable  - Loopback state, 0:disable, 1:enable
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port number.
 * Note:
 *      None.
 */
extern rtk_api_ret_t rtk_port_macLocalLoopbackEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port.
 * Input:
 *      port    - port id.
 *      reg     - Register id
 *      regData - Register data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      This API can set PHY register data of the specific port.
 */
extern rtk_api_ret_t rtk_port_phyReg_set(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t value);

/* Function Name:
 *      rtk_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port.
 * Input:
 *      port    - Port id.
 *      reg     - Register id
 * Output:
 *      pData   - Register data
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_PHY_REG_ID       - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      This API can get PHY register data of the specific port.
 */
extern rtk_api_ret_t rtk_port_phyReg_get(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t *pData);

/* Function Name:
 *      rtk_port_backpressureEnable_set
 * Description:
 *      Set the half duplex backpressure enable status of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set the half duplex backpressure enable status of the specific port.
 *      The half duplex backpressure enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_port_backpressureEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_backpressureEnable_get
 * Description:
 *      Get the half duplex backpressure enable status of the specific port.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get the half duplex backpressure enable status of the specific port.
 *      The half duplex backpressure enable status of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_port_backpressureEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_adminEnable_get
 * Description:
 *      Get port admin configurationof the specific port.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_port_adminEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_port_isolation_set
 * Description:
 *      Set permitted port isolation portmask
 * Input:
 *      port        - port id.
 *      portmask    - Permit port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 * Note:
 *      This API set the port mask that a port can trasmit packet to of each port
 *      A port can only transmit packet to ports included in permitted portmask
 */
extern rtk_api_ret_t rtk_port_isolation_set(rtk_port_t port, rtk_portmask_t portmask);

/* Function Name:
 *      rtk_port_isolation_get
 * Description:
 *      Get permitted port isolation portmask
 * Input:
 *      port - Port id.
 * Output:
 *      pPortmask - Permit port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API get the port mask that a port can trasmit packet to of each port
 *      A port can only transmit packet to ports included in permitted portmask
 */
extern rtk_api_ret_t rtk_port_isolation_get(rtk_port_t port, rtk_portmask_t *pPortmask);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can set external interface 0 RGMII delay.
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for n0-delay, and 7 for maximum delay.
 */
extern rtk_api_ret_t rtk_port_rgmiiDelayExt0_set(rtk_data_t txDelay, rtk_data_t rxDelay);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can set external interface 0 RGMII delay.
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for n0-delay, and 7 for maximum delay.
 */
extern rtk_api_ret_t rtk_port_rgmiiDelayExt0_get(rtk_data_t *pTxDelay, rtk_data_t *pRxDelay);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can set external interface 1 RGMII delay.
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for n0-delay, and 7 for maximum delay.
 */
extern rtk_api_ret_t rtk_port_rgmiiDelayExt1_set(rtk_data_t txDelay, rtk_data_t rxDelay);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can set external interface 1 RGMII delay.
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for n0-delay, and 7 for maximum delay.
 */
extern rtk_api_ret_t rtk_port_rgmiiDelayExt1_get(rtk_data_t *pTxDelay, rtk_data_t *pRxDelay);

/* Function Name:
 *      rtk_port_rgmiiDelayExt_set
 * Description:
 *      Set RGMII interface delay value for TX and RX.
 * Input:
 *      txDelay - TX delay value, 1 for delay 2ns and 0 for no-delay
 *      rxDelay - RX delay value, 0~7 for delay setup.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can set external interface 2 RGMII delay.
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for no-delay, and 7 for maximum delay.
 */
extern rtk_api_ret_t rtk_port_rgmiiDelayExt_set(rtk_ext_port_t port, rtk_data_t txDelay, rtk_data_t rxDelay);

/* Function Name:
 *      rtk_port_rgmiiDelayExt_get
 * Description:
 *      Get RGMII interface delay value for TX and RX.
 * Input:
 *      None
 * Output:
 *      pTxDelay - TX delay value
 *      pRxDelay - RX delay value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can set external interface 2 RGMII delay.
 *      In TX delay, there are 2 selection: no-delay and 2ns delay.
 *      In RX dekay, there are 8 steps for delay tunning. 0 for n0-delay, and 7 for maximum delay.
 */
extern rtk_api_ret_t rtk_port_rgmiiDelayExt_get(rtk_ext_port_t port, rtk_data_t *pTxDelay, rtk_data_t *pRxDelay);

/* Function Name:
 *      rtk_port_phyEnableAll_set
 * Description:
 *      Set all PHY enable status.
 * Input:
 *      enable - PHY Enable State.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set all PHY status.
 *      The configuration of all PHY is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_port_phyEnableAll_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_port_phyEnableAll_get
 * Description:
 *      Get all PHY enable status.
 * Input:
 *      None
 * Output:
 *      pEnable - PHY Enable State.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      This API can set all PHY status.
 *      The configuration of all PHY is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_port_phyEnableAll_get(rtk_enable_t *pEnable);

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
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_L2_FID - Invalid fid.
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can set port-based enhanced filtering database.
 */
extern rtk_api_ret_t rtk_port_efid_set(rtk_port_t port, rtk_data_t efid);

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
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can get port-based enhanced filtering database status.
 */
extern rtk_api_ret_t rtk_port_efid_get(rtk_port_t port, rtk_data_t *pEfid);

/* Function Name:
 *      rtk_port_phyComboPortMedia_set
 * Description:
 *      Set Combo port media type
 * Input:
 *      port    - Port id. (Should be Port 4)
 *      media   - Media (COPPER or FIBER)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_PORT_ID          - Invalid port ID.
 * Note:
 *      The API can Set Combo port media type.
 */
extern rtk_api_ret_t rtk_port_phyComboPortMedia_set(rtk_port_t port, rtk_port_media_t media);

/* Function Name:
 *      rtk_port_phyComboPortMedia_get
 * Description:
 *      Get Combo port media type
 * Input:
 *      port    - Port id. (Should be Port 4)
 * Output:
 *      pMedia  - Media (COPPER or FIBER)
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_PORT_ID          - Invalid port ID.
 * Note:
 *      The API can Set Combo port media type.
 */
extern rtk_api_ret_t rtk_port_phyComboPortMedia_get(rtk_port_t port, rtk_port_media_t *pMedia);

/* Function Name:
 *      rtk_port_rtctEnable_set
 * Description:
 *      Enable RTCT test
 * Input:
 *      portmask    - Port mask of RTCT enabled port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_MASK        - Invalid port mask.
 * Note:
 *      The API can enable RTCT Test
 */
extern rtk_api_ret_t rtk_port_rtctEnable_set(rtk_portmask_t portmask);

/* Function Name:
 *      rtk_port_rtctResult_get
 * Description:
 *      Get the result of RTCT test
 * Input:
 *      port        - Port ID
 * Output:
 *      pRtctResult - The result of RTCT result
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port ID.
 *      RT_ERR_PHY_RTCT_NOT_FINISH  - Testing does not finish.
 * Note:
 *      The API can get RTCT test result.
 *      RTCT test may takes 4.8 seconds to finish its test at most.
 *      Thus, if this API return RT_ERR_PHY_RTCT_NOT_FINISH or
 *      other error code, the result can not be referenced and
 *      user should call this API again until this API returns
 *      a RT_ERR_OK.
 *      The result is stored at pRtctResult->ge_result
 *      pRtctResult->linkType is unused.
 *      The unit of channel length is 2.5cm. Ex. 300 means 300 * 2.5 = 750cm = 7.5M
 */
extern rtk_api_ret_t rtk_port_rtctResult_get(rtk_port_t port, rtk_rtctResult_t *pRtctResult);

/* CVLAN */
/* Function Name:
 *      rtk_vlan_init
 * Description:
 *      Initialize VLAN.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      VLAN is disabled by default. User has to call this API to enable VLAN before
 *      using it. And It will set a default VLAN(vid 1) including all ports and set
 *      all ports PVID to the default VLAN.
 */
extern rtk_api_ret_t rtk_vlan_init(void);

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
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT 		        - Invalid input parameters.
 *      RT_ERR_L2_FID               - Invalid FID.
 *      RT_ERR_VLAN_PORT_MBR_EXIST  - Invalid member port mask.
 *      RT_ERR_VLAN_VID             - Invalid VID parameter.
 * Note:
 *      There are 4K VLAN entry supported. User could configure the member set and untag set
 *      for specified vid through this API. The portmask's bit N means port N.
 *      For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *      FID is for SVL/IVL usage, and the range is 0~4095.
 */
extern rtk_api_ret_t rtk_vlan_set(rtk_vlan_t vid, rtk_portmask_t mbrmsk, rtk_portmask_t untagmsk, rtk_fid_t fid);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *     The API can get the member set, untag set and fid settings for specified vid.
 */
extern rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_portmask_t *pMbrmsk, rtk_portmask_t *pUntagmsk, rtk_fid_t *pFid);

/* Function Name:
 *      rtk_vlan_mbrCfg_set
 * Description:
 *      Set a VLAN Member Configuration entry by index.
 * Input:
 *      idx     - Index of VLAN Member Configuration.
 *      pMbrcfg - VLAN member Configuration.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *     Set a VLAN Member Configuration entry by index.
 */
extern rtk_api_ret_t rtk_vlan_mbrCfg_set(rtk_uint32 idx, rtk_vlan_mbrcfg_t *pMbrcfg);

/* Function Name:
 *      rtk_vlan_mbrCfg_get
 * Description:
 *      Get a VLAN Member Configuration entry by index.
 * Input:
 *      idx - Index of VLAN Member Configuration.
 * Output:
 *      pMbrcfg - VLAN member Configuration.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *     Get a VLAN Member Configuration entry by index.
 */
extern rtk_api_ret_t rtk_vlan_mbrCfg_get(rtk_uint32 idx, rtk_vlan_mbrcfg_t *pMbrcfg);

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
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_VLAN_PRIORITY        - Invalid priority.
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - VLAN entry not found.
 *      RT_ERR_VLAN_VID             - Invalid VID parameter.
 * Note:
 *       The API is used for Port-based VLAN. The untagged frame received from the
 *       port will be classified to the specified VLAN and assigned to the specified priority.
 */
extern rtk_api_ret_t rtk_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     The API can get the PVID and 802.1p priority for the PVID of Port-based VLAN.
 */
extern rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The status of vlan ingress filter is as following:
 *      - DISABLED
 *      - ENABLED
 *      While VLAN function is enabled, ASIC will decide VLAN ID for each received frame and get belonged member
 *      ports from VLAN table. If received port is not belonged to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_get
 * Description:
 *      Get VLAN Ingress Filter
 * Input:
 *      port        - Port id.
 * Output:
 *      pIgr_filter - VLAN ingress function enable status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     The API can Get the VLAN ingress filter status.
 *     The status of vlan ingress filter is as following:
 *     - DISABLED
 *     - ENABLED
 */
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN accept_frame_type
 * Input:
 *      port                - Port id.
 *      accept_frame_type   - accept frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_PORT_ID                  - Invalid port number.
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE   - Invalid frame type.
 * Note:
 *      The API is used for checking 802.1Q tagged frames.
 *      The accept frame type as following:
 *      - ACCEPT_FRAME_TYPE_ALL
 *      - ACCEPT_FRAME_TYPE_TAG_ONLY
 *      - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN accept_frame_type
 * Input:
 *      port - Port id.
 * Output:
 *      pAccept_frame_type - accept frame type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     The API can Get the VLAN ingress filter.
 *     The accept frame type as following:
 *     - ACCEPT_FRAME_TYPE_ALL
 *     - ACCEPT_FRAME_TYPE_TAG_ONLY
 *     - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type);

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
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_VLAN_VID         - Invalid VID parameter.
 *      RT_ERR_VLAN_PRIORITY    - Invalid priority.
 * Note:
 *      This API is used to set priority per VLAN.
 */
extern rtk_api_ret_t rtk_vlan_vlanBasedPriority_set(rtk_vlan_t vid, rtk_pri_t priority);

/* Function Name:
 *      rtk_vlan_vlanBasedPriority_get
 * Description:
 *      Get VLAN priority for each CVLAN.
 * Input:
 *      vid - Specified VLAN ID.
 * Output:
 *      pPriority - 802.1p priority for the PVID.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     This API is used to set priority per VLAN.
 */
extern rtk_api_ret_t rtk_vlan_vlanBasedPriority_get(rtk_vlan_t vid, rtk_pri_t *pPriority);

/* Function Name:
 *      rtk_vlan_tagMode_set
 * Description:
 *      Set CVLAN egress tag mode
 * Input:
 *      port        - Port id.
 *      tag_mode    - The egress tag mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      The API can set Egress tag mode. There are 4 mode for egress tag:
 *      - VLAN_TAG_MODE_ORIGINAL,
 *      - VLAN_TAG_MODE_KEEP_FORMAT,
 *      - VLAN_TAG_MODE_PRI.
 *      - VLAN_TAG_MODE_REAL_KEEP_FORMAT,
 */
extern rtk_api_ret_t rtk_vlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode);

/* Function Name:
 *      rtk_vlan_tagMode_get
 * Description:
 *      Get CVLAN egress tag mode
 * Input:
 *      port - Port id.
 * Output:
 *      pTag_mode - The egress tag mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get Egress tag mode. There are 4 mode for egress tag:
 *      - VLAN_TAG_MODE_ORIGINAL,
 *      - VLAN_TAG_MODE_KEEP_FORMAT,
 *      - VLAN_TAG_MODE_PRI.
 *      - VLAN_TAG_MODE_REAL_KEEP_FORMAT,
 */
extern rtk_api_ret_t rtk_vlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode);

/* Function Name:
 *      rtk_vlan_transparent_set
 * Description:
 *      Set VLAN transparent mode
 * Input:
 *      egr_port        - Egress Port id.
 *      igr_pmask       - Ingress Port Mask.
 *      enabled         - VLAN Transparent. Enabled: Ignore VLAN egress Filtering, Disable: Follow VLAN egress Filtering
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
extern rtk_api_ret_t rtk_vlan_transparent_set(rtk_port_t egr_port, rtk_portmask_t igr_pmask, rtk_enable_t enable);

/* Function Name:
 *      rtk_vlan_transparent_get
 * Description:
 *      Get VLAN transparent mode
 * Input:
 *      egr_port        - Egress Port id.
 * Output:
 *      pIgr_pmask      - Ingress Port Mask
 *      pEnabled        - VLAN Transparent. Enabled: Ignore VLAN egress Filtering, Disable: Follow VLAN egress Filtering
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
extern rtk_api_ret_t rtk_vlan_transparent_get(rtk_port_t egr_port, rtk_portmask_t *pIgr_pmask, rtk_enable_t *pEnable);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_MSTI         - Invalid msti parameter
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *      The API can set spanning tree group instance of the vlan to the specified device.
 */
extern rtk_api_ret_t rtk_vlan_stg_set(rtk_vlan_t vid, rtk_stg_t stg);

/* Function Name:
 *      rtk_vlan_stg_get
 * Description:
 *      Get spanning tree group instance of the vlan to the specified device
 * Input:
 *      vid - Specified VLAN ID.
 * Output:
 *      pStg - spanning tree group instance.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *      The API can get spanning tree group instance of the vlan to the specified device.
 */
extern rtk_api_ret_t rtk_vlan_stg_get(rtk_vlan_t vid, rtk_stg_t *pStg);

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
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_VLAN_VID         - Invalid VID parameter.
 *      RT_ERR_VLAN_PRIORITY    - Invalid priority.
 *      RT_ERR_TBL_FULL         - Table is full.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *      The frame type is shown in the following:
 *      - FRAME_TYPE_ETHERNET
 *      - FRAME_TYPE_RFC1042
 *      - FRAME_TYPE_LLCOTHER
 */
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_add(rtk_port_t port, rtk_vlan_protoAndPortInfo_t info);

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
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 *      RT_ERR_TBL_FULL         - Table is full.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     The frame type is shown in the following:
 *      - FRAME_TYPE_ETHERNET
 *      - FRAME_TYPE_RFC1042
 *      - FRAME_TYPE_LLCOTHER
 */
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_get(rtk_port_t port, rtk_vlan_proto_type_t proto_type, rtk_vlan_protoVlan_frameType_t frame_type, rtk_vlan_protoAndPortInfo_t *pInfo);

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_del
 * Description:
 *      Delete the protocol-and-port-based vlan from the specified port of device.
 * Input:
 *      port        - Port id.
 *      proto_type  - protocol-and-port-based vlan protocol type.
 *      frame_type  - protocol-and-port-based vlan frame type.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 *      RT_ERR_TBL_FULL         - Table is full.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     The frame type is shown in the following:
 *      - FRAME_TYPE_ETHERNET
 *      - FRAME_TYPE_RFC1042
 *      - FRAME_TYPE_LLCOTHER
 */
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_del(rtk_port_t port, rtk_vlan_proto_type_t proto_type, rtk_vlan_protoVlan_frameType_t frame_type);

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_delAll
 * Description:
 *     Delete all protocol-and-port-based vlans from the specified port of device.
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     Delete all flow table protocol-and-port-based vlan entries.
 */
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_delAll(rtk_port_t port);

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
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_L2_FID - Invalid fid.
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can set port-based filtering database. If the function is enabled, all input
 *      packets will be assigned to the port-based fid regardless vlan tag.
 */
extern rtk_api_ret_t rtk_vlan_portFid_set(rtk_port_t port, rtk_enable_t enable, rtk_fid_t fid);

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
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can get port-based filtering database status. If the function is enabled, all input
 *      packets will be assigned to the port-based fid regardless vlan tag.
 */
extern rtk_api_ret_t rtk_vlan_portFid_get(rtk_port_t port, rtk_enable_t *pEnable, rtk_fid_t *pFid);

/*Spanning Tree*/
/* Function Name:
 *      rtk_stp_init
 * Description:
 *      Initialize stp module of the specified device
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Initialize stp module before calling any vlan APIs
 */
extern rtk_api_ret_t rtk_stp_init(void);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_MSTI         - Invalid msti parameter.
 *      RT_ERR_MSTP_STATE   - Invalid STP state.
 * Note:
 *      System supports per-port multiple spanning tree state for each msti.
 *      There are four states supported by ASIC.
 *      - STP_STATE_DISABLED
 *      - STP_STATE_BLOCKING
 *      - STP_STATE_LEARNING
 *      - STP_STATE_FORWARDING
 */
extern rtk_api_ret_t rtk_stp_mstpState_set(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t stp_state);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_MSTI         - Invalid msti parameter.
 * Note:
 *      System supports per-port multiple spanning tree state for each msti.
 *      There are four states supported by ASIC.
 *      - STP_STATE_DISABLED
 *      - STP_STATE_BLOCKING
 *      - STP_STATE_LEARNING
 *      - STP_STATE_FORWARDING
 */
extern rtk_api_ret_t rtk_stp_mstpState_get(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t *pStp_state);

/* LUT */
/* Function Name:
 *      rtk_l2_init
 * Description:
 *      Initialize l2 module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      Initialize l2 module before calling any l2 APIs.
 */
extern rtk_api_ret_t rtk_l2_init(void);

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
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_MAC              - Invalid MAC address.
 *      RT_ERR_L2_FID           - Invalid FID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      If the unicast mac address already existed in LUT, it will udpate the status of the entry.
 *      Otherwise, it will find an empty or asic auto learned entry to write. If all the entries
 *      with the same hash value can't be replaced, ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
extern rtk_api_ret_t rtk_l2_addr_add(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data);

/* Function Name:
 *      rtk_l2_addr_get
 * Description:
 *      Get LUT unicast entry.
 * Input:
 *      pMac    - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 * Output:
 *      pL2_data - Unicast entry parameter
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the unicast mac address existed in LUT, it will return the port and fid where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
extern rtk_api_ret_t rtk_l2_addr_get(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data);

/* Function Name:
 *      rtk_l2_addr_next_get
 * Description:
 *      Get Next LUT unicast entry.
 * Input:
 *      read_method     - The reading method.
 *      port            - The port number if the read_metohd is READMETHOD_NEXT_L2UCSPA
 *      pAddress        - The Address ID
 * Output:
 *      pL2_data - Unicast entry parameter
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next unicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all entries is LUT.
 */
extern rtk_api_ret_t rtk_l2_addr_next_get(rtk_l2_read_method_t read_method, rtk_port_t port, rtk_uint32 *pAddress, rtk_l2_ucastAddr_t *pL2_data);

/* Function Name:
 *      rtk_l2_addr_del
 * Description:
 *      Delete LUT unicast entry.
 * Input:
 *      pMac - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      fid - Filtering database
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the mac has existed in the LUT, it will be deleted. Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
extern rtk_api_ret_t rtk_l2_addr_del(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data);

/* Function Name:
 *      rtk_l2_mcastAddr_add
 * Description:
 *      Add LUT multicast entry.
 * Input:
 *      pMac        - 6 bytes multicast(I/G bit is 1) mac address to be written into LUT.
 *      ivl         - IVL/SVL setting, 1:IVL, 0:SVL
 *      cvid_fid    - CVID or FID for the input LUT entry.
 *      portmask    - Port mask to be forwarded to.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_MAC              - Invalid MAC address.
 *      RT_ERR_L2_FID           - Invalid FID .
 *      RT_ERR_L2_VID           - Invalid VID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      If the multicast mac address already existed in the LUT, it will udpate the
 *      port mask of the entry. Otherwise, it will find an empty or asic auto learned
 *      entry to write. If all the entries with the same hash value can't be replaced,
 *      ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
extern rtk_api_ret_t rtk_l2_mcastAddr_add(rtk_mac_t *pMac, rtk_data_t ivl, rtk_data_t cvid_fid, rtk_portmask_t portmask);

/* Function Name:
 *      rtk_l2_mcastAddr_get
 * Description:
 *      Get LUT multicast entry.
 * Input:
 *      pMac        - 6 bytes multicast(I/G bit is 1) mac address to be written into LUT.
 *      ivl         - IVL/SVL setting, 1:IVL, 0:SVL
 *      cvid_fid    - Filtering database
 * Output:
 *      pPortmask   - Port mask to be forwarded to.
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_VID               - Invalid VID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the multicast mac address existed in the LUT, it will return the port where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
extern rtk_api_ret_t rtk_l2_mcastAddr_get(rtk_mac_t *pMac, rtk_data_t ivl, rtk_data_t cvid_fid, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_l2_mcastAddr_next_get
 * Description:
 *      Get Next L2 Multicast entry.
 * Input:
 *      pAddress        - The Address ID
 * Output:
 *      pMac            - Multicast MAC address
 *      pIvl            - IVL/SVL setting, 1:IVL, 0:SVL
 *      pCvid_fid       - CVID or FID
 *      pPortmask       - Member Port Mask
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next L2 multicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all multicast entries is LUT.
 */
extern rtk_api_ret_t rtk_l2_mcastAddr_next_get(rtk_uint32 *pAddress, rtk_mac_t *pMac, rtk_data_t *pIvl, rtk_data_t *pCvid_fid, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_l2_mcastAddr_del
 * Description:
 *      Delete LUT multicast entry.
 * Input:
 *      pMac        - 6 bytes multicast(I/G bit is 1) mac address to be written into LUT.
 *      ivl         - IVL/SVL setting, 1:IVL, 0:SVL
 *      cvid_fid    - Filtering database
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_VID               - Invalid VID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the mac has existed in the LUT, it will be deleted. Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
extern rtk_api_ret_t rtk_l2_mcastAddr_del(rtk_mac_t *pMac, rtk_data_t ivl, rtk_data_t cvid_fid);

/* Function Name:
 *      rtk_l2_ipMcastAddr_add
 * Description:
 *      Add Lut IP multicast entry
 * Input:
 *      dip - Destination IP Address.
 *      sip - Source IP Address.
 *      portmask - Destination port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      System supports L2 entry with IP multicast DIP/SIP to forward IP multicasting frame as user
 *      desired. If this function is enabled, then system will be looked up L2 IP multicast entry to
 *      forward IP multicast frame directly without flooding.
 */
extern rtk_api_ret_t rtk_l2_ipMcastAddr_add(ipaddr_t sip, ipaddr_t dip, rtk_portmask_t portmask);

/* Function Name:
 *      rtk_l2_ipMcastAddr_get
 * Description:
 *      Get LUT IP multicast entry.
 * Input:
 *      dip - Destination IP Address.
 *      sip - Source IP Address.
 * Output:
 *      pPortmask - Destination port mask.
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      The API can get Lut table of IP multicast entry.
 */
extern rtk_api_ret_t rtk_l2_ipMcastAddr_get(ipaddr_t sip, ipaddr_t dip, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_l2_ipMcastAddr_next_get
 * Description:
 *      Get Next IP Multicast entry.
 * Input:
 *      pAddress        - The Address ID
 * Output:
 *      pSip            - Source IP
 *      pDip            - Destination IP
 *      pPortmask       - Member Port Mask
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next IP multicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all IP multicast entries is LUT.
 */
extern rtk_api_ret_t rtk_l2_ipMcastAddr_next_get(rtk_uint32 *pAddress, ipaddr_t *pSip, ipaddr_t *pDip, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_l2_ipMcastAddr_del
 * Description:
 *      Delete a ip multicast address entry from the specified device.
 * Input:
 *      pMac    - 6 bytes multicast(I/G bit is 1) mac address to be written into LUT.
 *      fid     - Filtering database
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      The API can delete a IP multicast address entry from the specified device.
 */
extern rtk_api_ret_t rtk_l2_ipMcastAddr_del(ipaddr_t sip, ipaddr_t dip);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function trigger flushing of per-port L2 learning.
 *      When flushing operaton completes, the corresponding bit will be clear.
 *      The flush type as following:
 *      - FLUSH_TYPE_BY_PORT        (physical port)
 *      - FLUSH_TYPE_BY_PORT_VID    (physical port + VID)
 */
extern rtk_api_ret_t rtk_l2_flushType_set(rtk_l2_flushType_t type, rtk_vlan_t vid, rtk_uint32 portOrTid);

/* Function Name:
 *      rtk_l2_ucastAddr_flush
 * Description:
 *      Flush L2 mac address by type in the specified device (both dynamic and static).
 * Input:
 *      pConfig - flush configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      flushByVid          - 1: Flush by VID, 0: Don't flush by VID
 *      vid                 - VID (0 ~ 4095)
 *      flushByPort         - 1: Flush by Port, 0: Don't flush by Port
 *      reserved            - Unused
 *      port                - Port ID
 *      flushByMac          - Not Supported
 *      ucastAddr           - Not Supported
 *      flushStaticAddr     - 1: Flush both Static and Dynamic entries, 0: Flush only Dynamic entries
 *      flushAddrOnAllPorts - 1: Flush VID-matched entries at all ports, 0: Flush VID-matched entries per port.
 */
extern rtk_api_ret_t rtk_l2_ucastAddr_flush(rtk_l2_flushCfg_t *pConfig);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      The status of flush linkdown port address is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_l2_flushLinkDownPortAddrEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - link down flush status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The status of flush linkdown port address is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_l2_flushLinkDownPortAddrEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_l2_agingEnable_set
 * Description:
 *      Set L2 LUT aging status per port setting.
 * Input:
 *      port    - Port id.
 *      enable  - Aging status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
extern rtk_api_ret_t rtk_l2_agingEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_l2_agingEnable_get
 * Description:
 *      Get L2 LUT aging status per port setting.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Aging status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can be used to get L2 LUT aging function per port.
 */
extern rtk_api_ret_t rtk_l2_agingEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_l2_limitLearningCnt_set
 * Description:
 *      Set per-Port auto learning limit number
 * Input:
 *      port    - Port id.
 *      mac_cnt - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_LIMITED_L2ENTRY_NUM  - Invalid auto learning limit number
 * Note:
 *      The API can set per-port ASIC auto learning limit number from 0(disable learning)
 *      to 8k.
 */
extern rtk_api_ret_t rtk_l2_limitLearningCnt_set(rtk_port_t port, rtk_mac_cnt_t mac_cnt);

/* Function Name:
 *      rtk_l2_limitLearningCnt_get
 * Description:
 *      Get per-Port auto learning limit number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - Auto learning entries limit number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get per-port ASIC auto learning limit number.
 */
extern rtk_api_ret_t rtk_l2_limitLearningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_NOT_ALLOWED  - Invalid learn over action
 * Note:
 *      The API can set SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
extern rtk_api_ret_t rtk_l2_limitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t action);

/* Function Name:
 *      rtk_l2_limitLearningCntAction_get
 * Description:
 *      Get auto learn over limit number action.
 * Input:
 *      port - Port id.
 * Output:
 *      pAction - Learn over action
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
extern rtk_api_ret_t rtk_l2_limitLearningCntAction_get(rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction);

/* Function Name:
 *      rtk_l2_learningCnt_get
 * Description:
 *      Get per-Port current auto learning number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - ASIC auto learning entries number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get per-port ASIC auto learning number
 */
extern rtk_api_ret_t rtk_l2_learningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can set the flooding mask.
 *      The flooding type is as following:
 *      - FLOOD_UNKNOWNDA
 *      - FLOOD_UNKNOWNMC
 *      - FLOOD_BC
 */
extern rtk_api_ret_t rtk_l2_floodPortMask_set(rtk_l2_flood_type_t floood_type, rtk_portmask_t flood_portmask);

/* Function Name:
 *      rtk_l2_floodPortMask_get
 * Description:
 *      Get flooding portmask
 * Input:
 *      type - flooding type.
 * Output:
 *      pFlood_portmask - flooding porkmask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get the flooding mask.
 *      The flooding type is as following:
 *      - FLOOD_UNKNOWNDA
 *      - FLOOD_UNKNOWNMC
 *      - FLOOD_BC
 */
extern rtk_api_ret_t rtk_l2_floodPortMask_get(rtk_l2_flood_type_t floood_type, rtk_portmask_t *pFlood_portmask);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid permit value.
 * Note:
 *      This API is setted to permit frame if its source port is equal to destination port.
 */
extern rtk_api_ret_t rtk_l2_localPktPermit_set(rtk_port_t port, rtk_enable_t permit);

/* Function Name:
 *      rtk_l2_localPktPermit_get
 * Description:
 *      Get permittion of frames if source port and destination port are the same.
 * Input:
 *      port - Port id.
 * Output:
 *      pPermit - permittion status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API is to get permittion status for frames if its source port is equal to destination port.
 */
extern rtk_api_ret_t rtk_l2_localPktPermit_get(rtk_port_t port, rtk_enable_t *pPermit);

/* Function Name:
 *      rtk_l2_aging_set
 * Description:
 *      Set LUT agging out speed
 * Input:
 *      aging_time - Agging out time.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can set LUT agging out period for each entry and the range is from 14s to 800s.
 */
extern rtk_api_ret_t rtk_l2_aging_set(rtk_l2_age_time_t aging_time);

/* Function Name:
 *      rtk_l2_aging_get
 * Description:
 *      Get LUT agging out time
 * Input:
 *      None
 * Output:
 *      pEnable - Aging status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get LUT agging out period for each entry.
 */
extern rtk_api_ret_t rtk_l2_aging_get(rtk_l2_age_time_t *pAging_time);

/* Function Name:
 *      rtk_l2_ipMcastAddrLookup_set
 * Description:
 *      Set Lut IP multicast lookup function
 * Input:
 *      type - Lookup type for IPMC packet.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      This API can work with rtk_l2_ipMcastAddrLookupException_add.
 *      If users set the lookup type to DIP, the group in exception table
 *      will be lookup by DIP+SIP
 *      If users set the lookup type to DIP+SIP, the group in exception table
 *      will be lookup by only DIP
 */
extern rtk_api_ret_t rtk_l2_ipMcastAddrLookup_set(rtk_l2_lookup_type_t type);

/* Function Name:
 *      rtk_l2_ipMcastAddrLookup_get
 * Description:
 *      Get Lut IP multicast lookup function
 * Input:
 *      None.
 * Output:
 *      pType - Lookup type for IPMC packet.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      None.
 */
extern rtk_api_ret_t rtk_l2_ipMcastAddrLookup_get(rtk_l2_lookup_type_t *pType);

/* Function Name:
 *      rtk_l2_ipMcastAddrLookupException_add
 * Description:
 *      Add an IP Multicast Exception group
 * Input:
 *      ip_addr     - IP address
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 *      RT_ERR_TBL_FULL    - Table Full
 * Note:
 *      Add an entry to IP Multicast exception table.
 */
extern rtk_api_ret_t rtk_l2_ipMcastAddrLookupException_add(ipaddr_t ip_addr);

/* Function Name:
 *      rtk_l2_ipMcastAddrLookupException_del
 * Description:
 *      Delete an IP Multicast Exception group
 * Input:
 *      ip_addr     - IP address
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 *      RT_ERR_TBL_FULL    - Table Full
 * Note:
 *      Delete an entry to IP Multicast exception table.
 */
extern rtk_api_ret_t rtk_l2_ipMcastAddrLookupException_del(ipaddr_t ip_addr);

/* Function Name:
 *      rtk_l2_entry_get
 * Description:
 *      Get LUT unicast entry.
 * Input:
 *      pL2_entry - Index field in the structure.
 * Output:
 *      pL2_entry - other fields such as MAC, port, age...
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_L2_EMPTY_ENTRY   - Empty LUT entry.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      This API is used to get address by index from 1~2112.
 */
extern rtk_api_ret_t rtk_l2_entry_get(rtk_l2_addr_table_t *pL2_entry);

/* SVLAN */
/* Function Name:
 *      rtk_svlan_init
 * Description:
 *      Initialize SVLAN Configuration
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design.
 *      User can set mathced ether type as service provider supported protocol.
 */
extern rtk_api_ret_t rtk_svlan_init(void);

/* Function Name:
 *      rtk_svlan_servicePort_add
 * Description:
 *      Add one service port in the specified device
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
extern rtk_api_ret_t rtk_svlan_servicePort_add(rtk_port_t port);

/* Function Name:
 *      rtk_svlan_servicePort_get
 * Description:
 *      Get service ports in the specified device.
 * Input:
 *      None
 * Output:
 *      pSvlan_portmask - pointer buffer of svlan ports.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
extern rtk_api_ret_t rtk_svlan_servicePort_get(rtk_portmask_t *pSvlan_portmask);

/* Function Name:
 *      rtk_svlan_servicePort_del
 * Description:
 *      Delete one service port in the specified device
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API is removing SVLAN service port in the specified device.
 */
extern rtk_api_ret_t rtk_svlan_servicePort_del(rtk_port_t port);

/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Configure accepted S-VLAN ether type.
 * Input:
 *      svlan_tag_id - Ether type of S-tag frame parsing in uplink ports.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design.
 *      User can set mathced ether type as service provider supported protocol.
 */
extern rtk_api_ret_t rtk_svlan_tpidEntry_set(rtk_uint32 svlan_tag_id);

/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get accepted S-VLAN ether type setting.
 * Input:
 *      None
 * Output:
 *      pSvlan_tag_id -  Ether type of S-tag frame parsing in uplink ports.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
extern rtk_api_ret_t rtk_svlan_tpidEntry_get(rtk_uint32 *pSvlan_tag_id);

/* Function Name:
 *      rtk_svlan_priorityRef_set
 * Description:
 *      Set S-VLAN upstream priority reference setting.
 * Input:
 *      ref - reference selection parameter.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      The API can set the upstream SVLAN tag priority reference source. The related priority
 *      sources are as following:
 *      - REF_INTERNAL_PRI,
 *      - REF_CTAG_PRI,
 *      - REF_SVLAN_PRI.
 */
extern rtk_api_ret_t rtk_svlan_priorityRef_set(rtk_svlan_pri_ref_t ref);

/* Function Name:
 *      rtk_svlan_priorityRef_get
 * Description:
 *      Get S-VLAN upstream priority reference setting.
 * Input:
 *      None
 * Output:
 *      pRef - reference selection parameter.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can get the upstream SVLAN tag priority reference source. The related priority
 *      sources are as following:
 *      - REF_INTERNAL_PRI,
 *      - REF_CTAG_PRI,
 *      - REF_SVLAN_PRI.
 */
extern rtk_api_ret_t rtk_svlan_priorityRef_get(rtk_svlan_pri_ref_t *pRef);

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
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameter.
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_SVLAN_TABLE_FULL - SVLAN configuration is full.
 * Note:
 *      The API can set system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped by default setup.
 *      - rtk_svlan_memberCfg_t->svid is SVID of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->memberport is member port mask of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->fid is filtering database of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->priority is priority of SVLAN member configuration.
 */
extern rtk_api_ret_t rtk_svlan_memberPortEntry_set(rtk_uint32 svid_idx, rtk_svlan_memberCfg_t *psvlan_cfg);

/* Function Name:
 *      rtk_svlan_memberPortEntry_get
 * Description:
 *      Get SVLAN member Configure.
 * Input:
 *      svid - SVLAN id
 * Output:
 *      pSvlan_cfg - SVLAN member configuration
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can get system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped.
 */
extern rtk_api_ret_t rtk_svlan_memberPortEntry_get(rtk_uint32 svid_idx, rtk_svlan_memberCfg_t *pSvlan_cfg);

/* Function Name:
 *      rtk_svlan_memberPortEntry_adv_set
 * Description:
 *      Configure system SVLAN member by index
 * Input:
 *      idx         - Index (0 ~ 63)
 *      psvlan_cfg  - SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameter.
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_SVLAN_TABLE_FULL - SVLAN configuration is full.
 * Note:
 *      The API can set system 64 accepted s-tag frame format by index.
 *      - rtk_svlan_memberCfg_t->svid is SVID of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->memberport is member port mask of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->fid is filtering database of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->priority is priority of SVLAN member configuration.
 */
extern rtk_api_ret_t rtk_svlan_memberPortEntry_adv_set(rtk_uint32 idx, rtk_svlan_memberCfg_t *pSvlan_cfg);

/* Function Name:
 *      rtk_svlan_memberPortEntry_adv_get
 * Description:
 *      Get SVLAN member Configure by index.
 * Input:
 *      idx         - Index (0 ~ 63)
 * Output:
 *      pSvlan_cfg  - SVLAN member configuration
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can get system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped.
 */
extern rtk_api_ret_t rtk_svlan_memberPortEntry_adv_get(rtk_uint32 idx, rtk_svlan_memberCfg_t *pSvlan_cfg);

/* Function Name:
 *      rtk_svlan_defaultSvlan_set
 * Description:
 *      Configure default egress SVLAN.
 * Input:
 *      port - Source port
 *      svid - SVLAN id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_INPUT                    - Invalid input parameter.
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 * Note:
 *      The API can set port n S-tag format index while receiving frame from port n
 *      is transmit through uplink port with s-tag field
 */
extern rtk_api_ret_t rtk_svlan_defaultSvlan_set(rtk_port_t port, rtk_vlan_t svid);

/* Function Name:
 *      rtk_svlan_defaultSvlan_get
 * Description:
 *      Get the configure default egress SVLAN.
 * Input:
 *      port - Source port
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get port n S-tag format index while receiving frame from port n
 *      is transmit through uplink port with s-tag field
 */
extern rtk_api_ret_t rtk_svlan_defaultSvlan_get(rtk_port_t port, rtk_vlan_t *pSvid);

/* Function Name:
 *      rtk_svlan_c2s_add
 * Description:
 *      Configure SVLAN C2S table
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
 *      svid - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port ID.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can set system C2S configuration. ASIC will check upstream's VID and assign related
 *      SVID to mathed packet. There are 128 SVLAN C2S configurations.
 */
extern rtk_api_ret_t rtk_svlan_c2s_add(rtk_vlan_t vid, rtk_port_t src_port, rtk_vlan_t svid);

/* Function Name:
 *      rtk_svlan_c2s_del
 * Description:
 *      Delete one C2S entry
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
 *      svid - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_VLAN_VID         - Invalid VID parameter.
 *      RT_ERR_PORT_ID          - Invalid port ID.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete system C2S configuration. There are 128 SVLAN C2S configurations.
 */
extern rtk_api_ret_t rtk_svlan_c2s_del(rtk_vlan_t vid, rtk_port_t src_port);

/* Function Name:
 *      rtk_svlan_c2s_get
 * Description:
 *      Get configure SVLAN C2S table
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
 * Output:
 *      pSvid - SVLAN ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port ID.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *     The API can get system C2S configuration. There are 128 SVLAN C2S configurations.
 */
extern rtk_api_ret_t rtk_svlan_c2s_get(rtk_vlan_t vid, rtk_port_t src_port, rtk_vlan_t *pSvid);

/* Function Name:
 *      rtk_svlan_untag_action_set
 * Description:
 *      Configure Action of downstream Un-Stag packet
 * Input:
 *      action  - Action for UnStag
 *      svid    - The SVID assigned to UnStag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of downstream Un-Stag packet. A SVID assigned
 *      to the un-stag is also supported by this API. The parameter of svid is
 *      only referenced when the action is set to UNTAG_ASSIGN
 */
extern rtk_api_ret_t rtk_svlan_untag_action_set(rtk_svlan_untag_action_t action, rtk_vlan_t svid);

/* Function Name:
 *      rtk_svlan_untag_action_get
 * Description:
 *      Get Action of downstream Un-Stag packet
 * Input:
 *      None
 * Output:
 *      pAction  - Action for UnStag
 *      pSvid    - The SVID assigned to UnStag packet
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can Get action of downstream Un-Stag packet. A SVID assigned
 *      to the un-stag is also retrieved by this API. The parameter pSvid is
 *      only refernced when the action is UNTAG_ASSIGN
 */
extern rtk_api_ret_t rtk_svlan_untag_action_get(rtk_svlan_untag_action_t *pAction, rtk_vlan_t *pSvid);

/* Function Name:
 *      rtk_svlan_unmatch_action_set
 * Description:
 *      Configure Action of downstream Unmatch packet
 * Input:
 *      action  - Action for Unmatch
 *      svid    - The SVID assigned to Unmatch packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of downstream Un-match packet. A SVID assigned
 *      to the un-match is also supported by this API. The parameter od svid is
 *      only refernced when the action is set to UNMATCH_ASSIGN
 */
extern rtk_api_ret_t rtk_svlan_unmatch_action_set(rtk_svlan_unmatch_action_t action, rtk_vlan_t svid);

/* Function Name:
 *      rtk_svlan_unmatch_action_get
 * Description:
 *      Get Action of downstream Unmatch packet
 * Input:
 *      None
 * Output:
 *      pAction  - Action for Unmatch
 *      pSvid    - The SVID assigned to Unmatch packet
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can Get action of downstream Un-match packet. A SVID assigned
 *      to the un-match is also retrieved by this API. The parameter pSvid is
 *      only refernced when the action is UNMATCH_ASSIGN
 */
extern rtk_api_ret_t rtk_svlan_unmatch_action_get(rtk_svlan_unmatch_action_t *pAction, rtk_vlan_t *pSvid);

/* Function Name:
 *      rtk_svlan_dmac_vidsel_set
 * Description:
 *      Set DMAC CVID selection
 * Input:
 *      port    - Port
 *      enable  - state of DMAC CVID Selection
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      This API can set DMAC CVID Selection state
 */
extern rtk_api_ret_t rtk_svlan_dmac_vidsel_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_svlan_dmac_vidsel_get
 * Description:
 *      Get DMAC CVID selection
 * Input:
 *      port    - Port
 * Output:
 *      pEnable - state of DMAC CVID Selection
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      This API can get DMAC CVID Selection state
 */
extern rtk_api_ret_t rtk_svlan_dmac_vidsel_get(rtk_port_t port, rtk_enable_t *pEnable);

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
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can set IP mutlicast to SVID configuration. If upstream packet is IPv4 multicast
 *      packet and DIP is matched MC2S configuration, ASIC will assign egress SVID to the packet.
 *      There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_ipmc2s_add(ipaddr_t ipmc, rtk_vlan_t svid);

/* Function Name:
 *      rtk_svlan_ipmc2s_del
 * Description:
 *      delete ip multicast address to SVLAN
 * Input:
 *      ipmc - ip multicast address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete IP mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_ipmc2s_del(ipaddr_t ipmc);

/* Function Name:
 *      rtk_svlan_ipmc2s_get
 * Description:
 *      Get ip multicast address to SVLAN
 * Input:
 *      ipmc - ip multicast address
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can get IP mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_ipmc2s_get(ipaddr_t ipmc, rtk_vlan_t *pSvid);

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
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can set L2 Mutlicast to SVID configuration. If upstream packet is L2 multicast
 *      packet and DMAC is matched, ASIC will assign egress SVID to the packet. There are 32
 *      SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_l2mc2s_add(rtk_vlan_t svid, rtk_mac_t mac);

/* Function Name:
 *      rtk_svlan_l2mc2s_del
 * Description:
 *      delete L2 multicast address to SVLAN
 * Input:
 *      mac - L2 multicast address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete Mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_l2mc2s_del(rtk_mac_t mac);

/* Function Name:
 *      rtk_svlan_l2mc2s_get
 * Description:
 *      Get L2 multicast address to SVLAN
 * Input:
 *      mac - L2 multicast address
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can get L2 mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_l2mc2s_get(rtk_mac_t mac, rtk_vlan_t *pSvid);

/* Function Name:
 *      rtk_svlan_sp2c_add
 * Description:
 *      Add system SP2C configuration
 * Input:
 *      cvid        - VLAN ID
 *      dst_port    - Destination port of SVLAN to CVLAN configuration
 *      svid        - SVLAN VID
 *
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can add SVID & Destination Port to CVLAN configuration. The downstream frames with assigned
 *      SVID will be add C-tag with assigned CVID if the output port is the assigned destination port.
 *      There are 128 SP2C configurations.
 */
extern rtk_api_ret_t rtk_svlan_sp2c_add(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t cvid);

/* Function Name:
 *      rtk_svlan_sp2c_get
 * Description:
 *      Get configure system SP2C content
 * Input:
 *      svid 	    - SVLAN VID
 *      dst_port 	- Destination port of SVLAN to CVLAN configuration
 * Output:
 *      pCvid - VLAN ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 * Note:
 *     The API can get SVID & Destination Port to CVLAN configuration. There are 128 SP2C configurations.
 */
extern rtk_api_ret_t rtk_svlan_sp2c_get(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t *pCvid);

/* Function Name:
 *      rtk_svlan_sp2c_del
 * Description:
 *      Delete system SP2C configuration
 * Input:
 *      svid        - SVLAN VID
 *      dst_port    - Destination port of SVLAN to CVLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can delete SVID & Destination Port to CVLAN configuration. There are 128 SP2C configurations.
 */
extern rtk_api_ret_t rtk_svlan_sp2c_del(rtk_vlan_t svid, rtk_port_t dst_port);

/* CPU Port */
/* Function Name:
 *      rtk_cpu_enable_set
 * Description:
 *      Set CPU port function enable/disable.
 * Input:
 *      enable - CPU port function enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can set CPU port function enable/disable.
 */
extern rtk_api_ret_t rtk_cpu_enable_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_cpu_enable_get
 * Description:
 *      Get CPU port and its setting.
 * Input:
 *      None
 * Output:
 *      pEnable - CPU port function enable
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get CPU port function enable/disable.
 */
extern rtk_api_ret_t rtk_cpu_enable_get(rtk_enable_t *pEnable);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can set CPU port and inserting proprietary CPU tag mode (Length/Type 0x8899)
 *      to the frame that transmitting to CPU port.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
extern rtk_api_ret_t rtk_cpu_tagPort_set(rtk_port_t port, rtk_cpu_insert_t mode);

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
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get configured CPU port and its setting.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
extern rtk_api_ret_t rtk_cpu_tagPort_get(rtk_port_t *pPort, rtk_cpu_insert_t *pMode);

/* 802.1X */
/* Function Name:
 *      rtk_dot1x_unauthPacketOper_set
 * Description:
 *      Set 802.1x unauth action configuration.
 * Input:
 *      port            - Port id.
 *      unauth_action   - 802.1X unauth action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      This API can set 802.1x unauth action configuration.
 *      The unauth action is as following:
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_GUESTVLAN
 */
extern rtk_api_ret_t rtk_dot1x_unauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action);

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_get
 * Description:
 *      Get 802.1x unauth action configuration.
 * Input:
 *      port - Port id.
 * Output:
 *      pUnauth_action - 802.1X unauth action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get 802.1x unauth action configuration.
 *      The unauth action is as following:
 *      - DOT1X_ACTION_DROP
 *      - DOT1X_ACTION_TRAP2CPU
 *      - DOT1X_ACTION_GUESTVLAN
 */
extern rtk_api_ret_t rtk_dot1x_unauthPacketOper_get(rtk_port_t port, rtk_dot1x_unauth_action_t *pUnauth_action);

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_set
 * Description:
 *      Set 802.1x EAPOL packet trap to CPU configuration
 * Input:
 *      enable - The status of 802.1x EAPOL packet.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      To support 802.1x authentication functionality, EAPOL frame (ether type = 0x888E) has to
 *      be trapped to CPU.
 *      The status of EAPOL frame trap to CPU is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_get
 * Description:
 *      Get 802.1x EAPOL packet trap to CPU configuration
 * Input:
 *      None
 * Output:
 *      pEnable - The status of 802.1x EAPOL packet.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      To support 802.1x authentication functionality, EAPOL frame (ether type = 0x888E) has to
 *      be trapped to CPU.
 *      The status of EAPOL frame trap to CPU is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_get(rtk_enable_t *pEnable);

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
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_ENABLE               - Invalid enable input.
 *      RT_ERR_DOT1X_PORTBASEDPNEN  - 802.1X port-based enable error
 * Note:
 *      The API can update the port-based port enable register content. If a port is 802.1x
 *      port based network access control "enabled", it should be authenticated so packets
 *      from that port won't be dropped or trapped to CPU.
 *      The status of 802.1x port-based network access control is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_portBasedEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_dot1x_portBasedEnable_get
 * Description:
 *      Get 802.1x port-based enable configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - The status of 802.1x port.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get the 802.1x port-based port status.
 */
extern rtk_api_ret_t rtk_dot1x_portBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

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
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *     RT_ERR_DOT1X_PORTBASEDAUTH   - 802.1X port-based auth error
 * Note:
 *      The authenticated status of 802.1x port-based network access control is as following:
 *      - UNAUTH
 *      - AUTH
 */
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_set(rtk_port_t port, rtk_dot1x_auth_status_t port_auth);

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_get
 * Description:
 *      Get 802.1x port-based auth. port configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pPort_auth - The status of 802.1x port.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get 802.1x port-based port auth.information.
 */
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_get(rtk_port_t port, rtk_dot1x_auth_status_t *pPort_auth);

/* Function Name:
 *      rtk_dot1x_portBasedDirection_set
 * Description:
 *      Set 802.1x port-based operational direction configuration
 * Input:
 *      port            - Port id.
 *      port_direction  - Operation direction
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_DOT1X_PORTBASEDOPDIR - 802.1X port-based operation direction error
 * Note:
 *      The operate controlled direction of 802.1x port-based network access control is as following:
 *      - BOTH
 *      - IN
 */
extern rtk_api_ret_t rtk_dot1x_portBasedDirection_set(rtk_port_t port, rtk_dot1x_direction_t port_direction);

/* Function Name:
 *      rtk_dot1x_portBasedDirection_get
 * Description:
 *      Get 802.1X port-based operational direction configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pPort_direction - Operation direction
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get 802.1x port-based operational direction information.
 */
extern rtk_api_ret_t rtk_dot1x_portBasedDirection_get(rtk_port_t port, rtk_dot1x_direction_t *pPort_direction);

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
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_ENABLE               - Invalid enable input.
 *      RT_ERR_DOT1X_MACBASEDPNEN   - 802.1X mac-based enable error
 * Note:
 *      If a port is 802.1x MAC based network access control "enabled", the incoming packets should
 *       be authenticated so packets from that port won't be dropped or trapped to CPU.
 *      The status of 802.1x MAC-based network access control is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_macBasedEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_dot1x_macBasedEnable_get
 * Description:
 *      Get 802.1x mac-based port enable configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - The status of 802.1x port.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      If a port is 802.1x MAC based network access control "enabled", the incoming packets should
 *      be authenticated so packets from that port wont be dropped or trapped to CPU.
 *      The status of 802.1x MAC-based network access control is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t rtk_dot1x_macBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_add
 * Description:
 *      Add an authenticated MAC to ASIC
 * Input:
 *      port        - Port id.
 *      pAuth_mac   - The authenticated MAC.
 *      fid         - filtering database.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_ENABLE               - Invalid enable input.
 *      RT_ERR_DOT1X_MACBASEDPNEN   - 802.1X mac-based enable error
 * Note:
 *      The API can add a 802.1x authenticated MAC address to port. If the MAC does not exist in LUT,
 *      user can't add this MAC to auth status.
 */
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_add(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_MAC          - Invalid MAC address.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can delete a 802.1x authenticated MAC address to port. It only change the auth status of
 *      the MAC and won't delete it from LUT.
 */
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_del(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid);

/* Function Name:
 *      rtk_dot1x_macBasedDirection_set
 * Description:
 *      Set 802.1x mac-based operational direction configuration
 * Input:
 *      mac_direction - Operation direction
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameter.
 *      RT_ERR_DOT1X_MACBASEDOPDIR  - 802.1X mac-based operation direction error
 * Note:
 *      The operate controlled direction of 802.1x mac-based network access control is as following:
 *      - BOTH
 *      - IN
 */
extern rtk_api_ret_t rtk_dot1x_macBasedDirection_set(rtk_dot1x_direction_t mac_direction);

/* Function Name:
 *      rtk_dot1x_macBasedDirection_get
 * Description:
 *      Get 802.1x mac-based operational direction configuration
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_direction - Operation direction
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get 802.1x mac-based operational direction information.
 */
extern rtk_api_ret_t rtk_dot1x_macBasedDirection_get(rtk_dot1x_direction_t *pMac_direction);

/* Function Name:
 *      Set 802.1x guest VLAN configuration
 * Description:
 *      Set 802.1x mac-based operational direction configuration
 * Input:
 *      vid - 802.1x guest VLAN ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      The operate controlled 802.1x guest VLAN
 */
extern rtk_api_ret_t rtk_dot1x_guestVlan_set(rtk_vlan_t vid);

/* Function Name:
 *      rtk_dot1x_guestVlan_get
 * Description:
 *      Get 802.1x guest VLAN configuration
 * Input:
 *      None
 * Output:
 *      pVid - 802.1x guest VLAN ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get 802.1x guest VLAN information.
 */
extern rtk_api_ret_t rtk_dot1x_guestVlan_get(rtk_vlan_t *pVid);

/* Function Name:
 *      rtk_dot1x_guestVlan2Auth_set
 * Description:
 *      Set 802.1x guest VLAN to auth host configuration
 * Input:
 *      enable - The status of guest VLAN to auth host.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      The operational direction of 802.1x guest VLAN to auth host control is as following:
 *      - ENABLED
 *      - DISABLED
 */
extern rtk_api_ret_t rtk_dot1x_guestVlan2Auth_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_dot1x_guestVlan2Auth_get
 * Description:
 *      Get 802.1x guest VLAN to auth host configuration
 * Input:
 *      None
 * Output:
 *      pEnable - The status of guest VLAN to auth host.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get 802.1x guest VLAN to auth host information.
 */
extern rtk_api_ret_t rtk_dot1x_guestVlan2Auth_get(rtk_enable_t *pEnable);

/* Port Trunk */
/* Function Name:
 *      rtk_trunk_port_set
 * Description:
 *      Set trunking group available port mask
 * Input:
 *      trk_gid                 - trunk group id
 *      trunk_member_portmask   - Logic trunking member port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_LA_TRUNK_ID  - Invalid trunking group
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 * Note:
 *      The API can set 4 port trunking group enabled port mask. Each port trunking group has max 4 ports.
 *      If enabled port mask has less than 2 ports available setting, then this trunking group function is disabled.
 *      The group port members for trunk group are as following:
 *      - TRUNK_GROUP0: port 0 to port 3.
 *      - TRUNK_GROUP1: port 4 to port 7.
 */
extern rtk_api_ret_t rtk_trunk_port_set(rtk_trunk_group_t trk_gid, rtk_portmask_t trunk_member_portmask);

/* Function Name:
 *      rtk_trunk_port_get
 * Description:
 *      Get trunking group available port mask
 * Input:
 *      trk_gid - trunk group id
 * Output:
 *      pTrunk_member_portmask - Logic trunking member port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_LA_TRUNK_ID  - Invalid trunking group
 * Note:
 *      The API can get 4 port trunking group enabled port mask. Each port trunking group has max 4 ports.
 *      If enabled port mask has less than 2 ports available setting, then this trunking group function is disabled.
 *      The group port members for trunk group are as following:
 *      - TRUNK_GROUP0: port 0 to port 3.
 *      - TRUNK_GROUP1: port 4 to port 7.
 */
extern rtk_api_ret_t rtk_trunk_port_get(rtk_trunk_group_t trk_gid, rtk_portmask_t *pTrunk_member_portmask);

/* Function Name:
 *      rtk_trunk_distributionAlgorithm_set
 * Description:
 *      Set port trunking hash select sources
 * Input:
 *      trk_gid         - trunk group id
 *      algo_bitmask    - Bitmask of the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_LA_TRUNK_ID  - Invalid trunking group
 *      RT_ERR_LA_HASHMASK  - Hash algorithm selection error.
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 * Note:
 *      The API can set port trunking hash algorithm sources.
 *      7 bits mask for link aggregation group0 hash parameter selection {DIP, SIP, DMAC, SMAC, SPA}
 *      - 0b0000001: SPA
 *      - 0b0000010: SMAC
 *      - 0b0000100: DMAC
 *      - 0b0001000: SIP
 *      - 0b0010000: DIP
 *      - 0b0100000: TCP/UDP Source Port
 *      - 0b1000000: TCP/UDP Destination Port
 *      Example:
 *      - 0b0000011: SMAC & SPA
 *      - Note that it could be an arbitrary combination or independent set
 */
extern rtk_api_ret_t rtk_trunk_distributionAlgorithm_set(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t algo_bitmask);

/* Function Name:
 *      rtk_trunk_distributionAlgorithm_get
 * Description:
 *      Get port trunking hash select sources
 * Input:
 *      trk_gid - trunk group id
 * Output:
 *      pAlgo_bitmask -  Bitmask of the distribution algorithm
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_LA_TRUNK_ID  - Invalid trunking group
 * Note:
 *      The API can get port trunking hash algorithm sources.
 */
extern rtk_api_ret_t rtk_trunk_distributionAlgorithm_get(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t *pAlgo_bitmask);

/* Function Name:
 *      rtk_trunk_qeueuEmptyStatus_get
 * Description:
 *      Get current output queue if empty status
 * Input:
 *      trk_gid - trunk group id
 * Output:
 *      pPortmask - queue empty port mask, 1 for empty and 0 for not empty
 * Return:
 *      RT_ERR_OK       - OK
 *      RT_ERR_FAILED   - Failed
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      The API can get queues are empty port mask
 */
extern rtk_api_ret_t rtk_trunk_qeueuEmptyStatus_get(rtk_portmask_t *pPortmask);

/*Port Mirror */
/* Function Name:
 *      rtk_mirror_portBased_set
 * Description:
 *      Set port mirror function.
 * Input:
 *      mirroring_port          - Monitor port.
 *      pMirrored_rx_portmask   - Rx mirror port mask.
 *      pMirrored_tx_portmask   - Tx mirror port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 * Note:
 *      The API is to set mirror function of source port and mirror port.
 *      The mirror port can only be set to one port and the TX and RX mirror ports
 *      should be identical.
 */
extern rtk_api_ret_t rtk_mirror_portBased_set(rtk_port_t mirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);

/* Function Name:
 *      rtk_mirror_portBased_get
 * Description:
 *      Get port mirror function.
 * Input:
 *      None
 * Output:
 *      pMirroring_port         - Monitor port.
 *      pMirrored_rx_portmask   - Rx mirror port mask.
 *      pMirrored_tx_portmask   - Tx mirror port mask.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API is to get mirror function of source port and mirror port.
 */
extern rtk_api_ret_t rtk_mirror_portBased_get(rtk_port_t* pMirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);

/* Function Name:
 *      rtk_mirror_portIso_set
 * Description:
 *      Set mirror port isolation.
 * Input:
 *      enable |Mirror isolation status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The API is to set mirror isolation function that prevent normal forwarding packets to miror port.
 */
extern rtk_api_ret_t rtk_mirror_portIso_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_mirror_portIso_get
 * Description:
 *      Get mirror port isolation.
 * Input:
 *      None
 * Output:
 *      pEnable |Mirror isolation status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API is to get mirror isolation status.
 */
extern rtk_api_ret_t rtk_mirror_portIso_get(rtk_enable_t *pEnable);

/* MIB */
#ifdef EMBEDDED_SUPPORT
extern rtk_api_ret_t rtk_stat_global_reset(void);
extern rtk_api_ret_t rtk_stat_port_reset(rtk_port_t port);
extern rtk_api_ret_t rtk_stat_port_get(rtk_port_t port, rtk_stat_port_type_t cntr_idx, rtk_stat_counter_t *pCntrH, rtk_stat_counter_t *pCntrL);
extern rtk_api_ret_t rtk_stat_global_get(rtk_stat_global_type_t cntr_idx, rtk_stat_counter_t *pCntrH, rtk_stat_counter_t *pCntrL);
#else

/* Function Name:
 *      rtk_stat_global_reset
 * Description:
 *      Reset global MIB counter.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
extern rtk_api_ret_t rtk_stat_global_reset(void);

/* Function Name:
 *      rtk_stat_port_reset
 * Description:
 *      Reset per port MIB counter by port.
 * Input:
 *      port - port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
extern rtk_api_ret_t rtk_stat_port_reset(rtk_port_t port);

/* Function Name:
 *      rtk_stat_global_get
 * Description:
 *      Get global MIB counter
 * Input:
 *      cntr_idx - global counter index.
 * Output:
 *      pCntr - global counter value.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      Get global MIB counter by index definition.
 */
extern rtk_api_ret_t rtk_stat_global_get(rtk_stat_global_type_t cntr_idx, rtk_stat_counter_t *pCntr);

/* Function Name:
 *      rtk_stat_global_getAll
 * Description:
 *      Get all global MIB counter
 * Input:
 *      None
 * Output:
 *      pGlobal_cntrs - global counter structure.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      Get all global MIB counter by index definition.
 */
extern rtk_api_ret_t rtk_stat_global_getAll(rtk_stat_global_cntr_t *pGlobal_cntrs);

/* Function Name:
 *      rtk_stat_port_get
 * Description:
 *      Get per port MIB counter by index
 * Input:
 *      port        - port id.
 *      cntr_idx    - port counter index.
 * Output:
 *      pCntr - MIB retrived counter.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Get per port MIB counter by index definition.
 */
extern rtk_api_ret_t rtk_stat_port_get(rtk_port_t port, rtk_stat_port_type_t cntr_idx, rtk_stat_counter_t *pCntr);

/* Function Name:
 *      rtk_stat_port_getAll
 * Description:
 *      Get all counters of one specified port in the specified device.
 * Input:
 *      port - port id.
 * Output:
 *      pPort_cntrs - buffer pointer of counter value.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      Get all MIB counters of one port.
 */
extern rtk_api_ret_t rtk_stat_port_getAll(rtk_port_t port, rtk_stat_port_cntr_t *pPort_cntrs);
#endif

/* Function Name:
 *      rtk_stat_logging_counterCfg_set
 * Description:
 *      Set the type and mode of Logging Counter
 * Input:
 *      idx     - The index of Logging Counter. Should be even number only.(0,2,4,6,8.....30)
 *      mode    - 32 bits or 64 bits mode
 *      type    - Packet counter or byte counter
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_OUT_OF_RANGE - Out of range.
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      Set the type and mode of Logging Counter.
 */
extern rtk_api_ret_t rtk_stat_logging_counterCfg_set(rtk_uint32 idx, rtk_logging_counter_mode_t mode, rtk_logging_counter_type_t type);

/* Function Name:
 *      rtk_stat_logging_counterCfg_get
 * Description:
 *      Get the type and mode of Logging Counter
 * Input:
 *      idx     - The index of Logging Counter. Should be even number only.(0,2,4,6,8.....30)
 * Output:
 *      pMode   - 32 bits or 64 bits mode
 *      pType   - Packet counter or byte counter
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_OUT_OF_RANGE - Out of range.
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - NULL Pointer
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      Get the type and mode of Logging Counter.
 */
extern rtk_api_ret_t rtk_stat_logging_counterCfg_get(rtk_uint32 idx, rtk_logging_counter_mode_t *pMode, rtk_logging_counter_type_t *pType);

/* Function Name:
 *      rtk_stat_logging_counter_reset
 * Description:
 *      Reset Logging Counter
 * Input:
 *      idx     - The index of Logging Counter. (0~31)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_OUT_OF_RANGE - Out of range.
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Reset Logging Counter.
 */
extern rtk_api_ret_t rtk_stat_logging_counter_reset(rtk_uint32 idx);

/* Function Name:
 *      rtk_stat_logging_counter_get
 * Description:
 *      Get Logging Counter
 * Input:
 *      idx     - The index of Logging Counter. (0~31)
 * Output:
 *      pCnt    - Logging counter value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_OUT_OF_RANGE - Out of range.
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Get Logging Counter.
 */
extern rtk_api_ret_t rtk_stat_logging_counter_get(rtk_uint32 idx, rtk_uint32 *pCnt);

/* Interrupt */
/* Function Name:
 *      rtk_int_polarity_set
 * Description:
 *      Set interrupt polarity configuration.
 * Input:
 *      type - Interruptpolarity type.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can set interrupt polarity configuration.
 */
extern rtk_api_ret_t rtk_int_polarity_set(rtk_int_polarity_t type);

/* Function Name:
 *      rtk_int_polarity_get
 * Description:
 *      Get interrupt polarity configuration.
 * Input:
 *      None
 * Output:
 *      pType - Interruptpolarity type.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can get interrupt polarity configuration.
 */
extern rtk_api_ret_t rtk_int_polarity_get(rtk_int_polarity_t *pType);

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
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 *      RT_ERR_ENABLE 		- Invalid enable input.
 * Note:
 *      The API can set interrupt status configuration.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_LINK_STATUS
 *      - INT_TYPE_METER_EXCEED
 *      - INT_TYPE_LEARN_LIMIT
 *      - INT_TYPE_LINK_SPEED
 *      - INT_TYPE_CONGEST
 *      - INT_TYPE_GREEN_FEATURE
 *      - INT_TYPE_LOOP_DETECT
 *      - INT_TYPE_8051,
 *      - INT_TYPE_CABLE_DIAG,
 *      - INT_TYPE_ACL,
 *      - INT_TYPE_UPS,
 *      - INT_TYPE_SLIENT
 */
extern rtk_api_ret_t rtk_int_control_set(rtk_int_type_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_int_control_get
 * Description:
 *      Get interrupt trigger status configuration.
 * Input:
 *      type - Interrupt type.
 * Output:
 *      pEnable - Interrupt status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can get interrupt status configuration.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_LINK_STATUS
 *      - INT_TYPE_METER_EXCEED
 *      - INT_TYPE_LEARN_LIMIT
 *      - INT_TYPE_LINK_SPEED
 *      - INT_TYPE_CONGEST
 *      - INT_TYPE_GREEN_FEATURE
 *      - INT_TYPE_LOOP_DETECT
 *      - INT_TYPE_8051,
 *      - INT_TYPE_CABLE_DIAG,
 *      - INT_TYPE_ACL,
 *      - INT_TYPE_UPS,
 *      - INT_TYPE_SLIENT
 */
extern rtk_api_ret_t rtk_int_control_get(rtk_int_type_t type, rtk_enable_t* pEnable);

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
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      The API can clean interrupt trigger status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_LINK_STATUS    (value[0] (Bit0))
 *      - INT_TYPE_METER_EXCEED   (value[0] (Bit1))
 *      - INT_TYPE_LEARN_LIMIT    (value[0] (Bit2))
 *      - INT_TYPE_LINK_SPEED     (value[0] (Bit3))
 *      - INT_TYPE_CONGEST        (value[0] (Bit4))
 *      - INT_TYPE_GREEN_FEATURE  (value[0] (Bit5))
 *      - INT_TYPE_LOOP_DETECT    (value[0] (Bit6))
 *      - INT_TYPE_8051           (value[0] (Bit7))
 *      - INT_TYPE_CABLE_DIAG     (value[1] (Bit0))
 *      - INT_TYPE_ACL            (value[1] (Bit1))
 *      - INT_TYPE_UPS            (value[1] (Bit2))
 *      - INT_TYPE_SLIENT         (value[1] (Bit3))
 *      The status will be cleared after execute this API.
 */
extern rtk_api_ret_t rtk_int_status_set(rtk_int_status_t statusMask);

/* Function Name:
 *      rtk_int_status_get
 * Description:
 *      Get interrupt trigger status.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt status bit mask.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can get interrupt trigger status when interrupt happened.
 *      The interrupt trigger status is shown in the following:
 *      - INT_TYPE_LINK_STATUS    (value[0] (Bit0))
 *      - INT_TYPE_METER_EXCEED   (value[0] (Bit1))
 *      - INT_TYPE_LEARN_LIMIT    (value[0] (Bit2))
 *      - INT_TYPE_LINK_SPEED     (value[0] (Bit3))
 *      - INT_TYPE_CONGEST        (value[0] (Bit4))
 *      - INT_TYPE_GREEN_FEATURE  (value[0] (Bit5))
 *      - INT_TYPE_LOOP_DETECT    (value[0] (Bit6))
 *      - INT_TYPE_8051           (value[0] (Bit7))
 *      - INT_TYPE_CABLE_DIAG     (value[1] (Bit0))
 *      - INT_TYPE_ACL            (value[1] (Bit1))
 *      - INT_TYPE_UPS            (value[1] (Bit2))
 *      - INT_TYPE_SLIENT         (value[1] (Bit3))
 *
 */
extern rtk_api_ret_t rtk_int_status_get(rtk_int_status_t* pStatusMask);

/* Function Name:
 *      rtk_int_advanceInfo_get
 * Description:
 *      Get interrupt advanced information.
 * Input:
 *      adv_type - Advanced interrupt type.
 * Output:
 *      info - Information per type.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This API can get advanced information when interrupt happened.
 *      The status will be cleared after execute this API.
 */
extern rtk_api_ret_t rtk_int_advanceInfo_get(rtk_int_advType_t adv_type, rtk_int_info_t* info);

/*LED*/
/* Function Name:
 *      rtk_led_enable_set
 * Description:
 *      Set Led enable congiuration
 * Input:
 *      group       - LED group id.
 *      portmask    - LED enable port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can be used to enable LED per port per group.
 */
extern rtk_api_ret_t rtk_led_enable_set(rtk_led_group_t group, rtk_portmask_t portmask);

/* Function Name:
 *      rtk_led_enable_get
 * Description:
 *      Get Led enable congiuration
 * Input:
 *      group - LED group id.
 * Output:
 *      pPortmask - LED enable port mask.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can be used to get LED enable status.
 */
extern rtk_api_ret_t rtk_led_enable_get(rtk_led_group_t group, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_led_operation_set
 * Description:
 *      Set Led operation mode
 * Input:
 *      mode - LED operation mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can set Led operation mode.
 *      The modes that can be set are as following:
 *      - LED_OP_SCAN,
 *      - LED_OP_PARALLEL,
 *      - LED_OP_SERIAL,
 */
extern rtk_api_ret_t rtk_led_operation_set(rtk_led_operation_t mode);

/* Function Name:
 *      rtk_led_operation_get
 * Description:
 *      Get Led operation mode
 * Input:
 *      None
 * Output:
 *      pMode - Support LED operation mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can get Led operation mode.
 *      The modes that can be set are as following:
 *      - LED_OP_SCAN,
 *      - LED_OP_PARALLEL,
 *      - LED_OP_SERIAL,
 */
extern rtk_api_ret_t rtk_led_operation_get(rtk_led_operation_t *pMode);

/* Function Name:
 *      rtk_led_mode_set
 * Description:
 *      Set Led to congiuration mode
 * Input:
 *      mode - Support LED mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      There are three LED groups for each port for indicating information about dedicated port.
 *      LED groups are set to indicate different information of port in different mode.
 *
 *      (0) Mode0
 *          - LED0-Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
 *          - LED1-1000Mb/s Speed Indicator. Low for 1000Mb/s.
 *          - LED2-100Mb/s Speed Indicator. Low for 100Mb/s.
 *
 *      (1) Mode1
 *          - LED0-Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
 *          - LED1-1000Mb/s Speed Indicator. Low for 1000Mb/s.
 *          - LED2-Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
 *
 *      (2) Mode2
 *          - LED0-Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
 *          - LED1-1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *          - LED2-10/100Mb/s Speed/Activity Indicator. Low for 10/100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *
 *      (3) Mode3
 *          - LED0-10Mb/s Speed/Activity Indicator. Low for 10Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *          - LED1-1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *          - LED2-100Mb/s Speed/Activity Indicator. Low for 100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 */
extern rtk_api_ret_t rtk_led_mode_set(rtk_led_mode_t mode);

/* Function Name:
 *      rtk_led_mode_get
 * Description:
 *      Get Led to congiuration mode
 * Input:
 *      None
 * Output:
 *      pMode - Support LED mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      There are three LED groups for each port for indicating information about dedicated port.
 *      LED groups is set to indicate different information of port in different mode.
 */
extern rtk_api_ret_t rtk_led_mode_get(rtk_led_mode_t *pMode);

/* Function Name:
 *      rtk_led_modeForce_set
 * Description:
 *      Set Led group to congiuration force mode
 * Input:
 *      group   - Support LED group id.
 *      mode    - Support LED force mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can force all Leds in the same group to one force mode.
 *      The force modes that can be set are as following:
 *      - LED_FORCE_NORMAL,
 *      - LED_FORCE_BLINK,
 *      - LED_FORCE_OFF,
 *      - LED_FORCE_ON.
 */
extern rtk_api_ret_t rtk_led_modeForce_set(rtk_led_group_t group, rtk_led_force_mode_t mode);

/* Function Name:
 *      rtk_led_modeForce_get
 * Description:
 *      Get Led group to congiuration force mode
 * Input:
 *      group - Support LED group id.
 *      pMode - Support LED force mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can get forced Led group mode.
 *      The force modes that can be set are as following:
 *      - LED_FORCE_NORMAL,
 *      - LED_FORCE_BLINK,
 *      - LED_FORCE_OFF,
 *      - LED_FORCE_ON.
 */
extern rtk_api_ret_t rtk_led_modeForce_get(rtk_led_group_t group, rtk_led_force_mode_t *pMode);

/* Function Name:
 *      rtk_led_blinkRate_set
 * Description:
 *      Set LED blinking rate
 * Input:
 *      blinkRate - blinking rate.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      ASIC support 6 types of LED blinking rates at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */
extern rtk_api_ret_t rtk_led_blinkRate_set(rtk_led_blink_rate_t blinkRate);

/* Function Name:
 *      rtk_led_blinkRate_get
 * Description:
 *      Get LED blinking rate at mode 0 to mode 3
 * Input:
 *      None
 * Output:
 *      pBlinkRate - blinking rate.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      There are  6 types of LED blinking rates at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */
extern rtk_api_ret_t rtk_led_blinkRate_get(rtk_led_blink_rate_t *pBlinkRate);

/* Function Name:
 *      rtk_led_groupConfig_set
 * Description:
 *      Set per group Led to congiuration mode
 * Input:
 *      group   - LED group.
 *      config  - LED configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can set LED indicated information configuration for each LED group with 1 to 1 led mapping to each port.
 *      - Definition  LED Statuses      Description
 *      - 0000        LED_Off           LED pin Tri-State.
 *      - 0001        Dup/Col           Collision, Full duplex Indicator.
 *      - 0010        Link/Act          Link, Activity Indicator.
 *      - 0011        Spd1000           1000Mb/s Speed Indicator.
 *      - 0100        Spd100            100Mb/s Speed Indicator.
 *      - 0101        Spd10             10Mb/s Speed Indicator.
 *      - 0110        Spd1000/Act       1000Mb/s Speed/Activity Indicator.
 *      - 0111        Spd100/Act        100Mb/s Speed/Activity Indicator.
 *      - 1000        Spd10/Act         10Mb/s Speed/Activity Indicator.
 *      - 1001        Spd100 (10)/Act   10/100Mb/s Speed/Activity Indicator.
 *      - 1010        LoopDetect        LoopDetect Indicator.
 *      - 1011        EEE               EEE Indicator.
 *      - 1100        Link/Rx           Link, Activity Indicator.
 *      - 1101        Link/Tx           Link, Activity Indicator.
 *      - 1110        Master            Link on Master Indicator.
 *      - 1111        Act               Activity Indicator. Low for link established.
 */
extern rtk_api_ret_t rtk_led_groupConfig_set(rtk_led_group_t group, rtk_led_congig_t config);

/* Function Name:
 *      rtk_led_groupConfig_get
 * Description:
 *      Get Led group congiuration mode
 * Input:
 *      group - LED group.
 * Output:
 *      pConfig - LED configuration.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *       The API can get LED indicated information configuration for each LED group.
 */
extern rtk_api_ret_t rtk_led_groupConfig_get(rtk_led_group_t group, rtk_led_congig_t *pConfig);

/* Function Name:
 *      rtk_led_serialMode_set
 * Description:
 *      Set Led serial mode active congiuration
 * Input:
 *      active - LED group.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can set LED serial mode active congiuration.
 */
extern rtk_api_ret_t rtk_led_serialMode_set(rtk_led_active_t active);

/* Function Name:
 *      rtk_led_serialMode_get
 * Description:
 *      Get Led group congiuration mode
 * Input:
 *      group - LED group.
 * Output:
 *      pConfig - LED configuration.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *       The API can get LED serial mode active configuration.
 */
extern rtk_api_ret_t rtk_led_serialMode_get(rtk_led_active_t *pActive);


/*ACL APIs*/
/* Function Name:
 *      rtk_filter_igrAcl_init
 * Description:
 *      ACL initialization function
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Pointer pFilter_field or pFilter_cfg point to NULL.
 * Note:
 *      This function enable and intialize ACL function
 */
extern rtk_api_ret_t rtk_filter_igrAcl_init(void);

/* Function Name:
 *      rtk_filter_igrAcl_field_add
 * Description:
 *      Add comparison rule to an ACL configuration
 * Input:
 *      pFilter_cfg     - The ACL configuration that this function will add comparison rule
 *      pFilter_field   - The comparison rule that will be added.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_NULL_POINTER    	- Pointer pFilter_field or pFilter_cfg point to NULL.
 *      RT_ERR_INPUT 			- Invalid input parameters.
 * Note:
 *      This function add a comparison rule (*pFilter_field) to an ACL configuration (*pFilter_cfg).
 *      Pointer pFilter_cfg points to an ACL configuration structure, this structure keeps multiple ACL
 *      comparison rules by means of linked list. Pointer pFilter_field will be added to linked
 *      list keeped by structure that pFilter_cfg points to.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_field_add(rtk_filter_cfg_t *pFilter_cfg, rtk_filter_field_t *pFilter_field);

/* Function Name:
 *      rtk_filter_igrAcl_cfg_add
 * Description:
 *      Add an ACL configuration to ASIC
 * Input:
 *      filter_id       - Start index of ACL configuration.
 *      pFilter_cfg     - The ACL configuration that this function will add comparison rule
 *      pFilter_action  - Action(s) of ACL configuration.
 * Output:
 *      ruleNum - number of rules written in acl table
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_SMI             					- SMI access error
 *      RT_ERR_NULL_POINTER    					- Pointer pFilter_field or pFilter_cfg point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 *      RT_ERR_ENTRY_INDEX 						- Invalid filter_id .
 *      RT_ERR_NULL_POINTER 					- Pointer pFilter_action or pFilter_cfg point to NULL.
 *      RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT 	- Action is not supported in this chip.
 *      RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT 	- Rule is not supported.
 * Note:
 *      This function store pFilter_cfg, pFilter_action into ASIC. The starting
 *      index(es) is filter_id.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_cfg_add(rtk_filter_id_t filter_id, rtk_filter_cfg_t *pFilter_cfg, rtk_filter_action_t *pAction, rtk_filter_number_t *ruleNum);

/* Function Name:
 *      rtk_filter_igrAcl_cfg_del
 * Description:
 *      Delete an ACL configuration from ASIC
 * Input:
 *      filter_id   - Start index of ACL configuration.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_ENTRYIDX  - Invalid filter_id.
 * Note:
 *      This function delete a group of ACL rules starting from filter_id.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_cfg_del(rtk_filter_id_t filter_id);

/* Function Name:
 *      rtk_filter_igrAcl_cfg_delAll
 * Description:
 *      Delete all ACL entries from ASIC
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      This function delete all ACL configuration from ASIC.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_cfg_delAll(void);

/* Function Name:
 *      rtk_filter_igrAcl_cfg_get
 * Description:
 *      Get one ingress acl configuration from ASIC.
 * Input:
 *      filter_id       - Start index of ACL configuration.
 * Output:
 *      pFilter_cfg     - buffer pointer of ingress acl data
 *      pFilter_action  - buffer pointer of ingress acl action
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_SMI             	- SMI access error
 *      RT_ERR_NULL_POINTER 	- Pointer pFilter_action or pFilter_cfg point to NULL.
 *      RT_ERR_FILTER_ENTRYIDX 	- Invalid entry index.
 * Note:
 *      This function delete all ACL configuration from ASIC.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_cfg_get(rtk_filter_id_t filter_id, rtk_filter_cfg_raw_t *pFilter_cfg, rtk_filter_action_t *pAction);

/* Function Name:
 *      rtk_filter_igrAcl_unmatchAction_set
 * Description:
 *      Set action to packets when no ACL configuration match
 * Input:
 *      port    - Port id.
 *      action  - Action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID 		- Invalid port id.
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      This function sets action of packets when no ACL configruation matches.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_unmatchAction_set(rtk_port_t port, rtk_filter_unmatch_action_t action);

/* Function Name:
 *      rtk_filter_igrAcl_unmatchAction_get
 * Description:
 *      Get action to packets when no ACL configuration match
 * Input:
 *      port    - Port id.
 * Output:
 *      pAction - Action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port id.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no ACL configruation matches.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_unmatchAction_get(rtk_port_t port, rtk_filter_unmatch_action_t* action);

/* Function Name:
 *      rtk_filter_igrAcl_state_set
 * Description:
 *      Set state of ingress ACL.
 * Input:
 *      port    - Port id.
 *      state   - Ingress ACL state.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port id.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no ACL configruation matches.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_state_set(rtk_port_t port, rtk_filter_state_t state);

/* Function Name:
 *      rtk_filter_igrAcl_state_get
 * Description:
 *      Get state of ingress ACL.
 * Input:
 *      port    - Port id.
 * Output:
 *      pState  - Ingress ACL state.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port id.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no ACL configruation matches.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_state_get(rtk_port_t port, rtk_filter_state_t* state);

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
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Invalid input parameters.
 * Note:
 *      This function set ACL template.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_template_set(rtk_filter_template_t *aclTemplate);

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
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This function gets template of ACL.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_template_get(rtk_filter_template_t *aclTemplate);

/* Function Name:
 *      rtk_filter_igrAcl_field_sel_set
 * Description:
 *      Set user defined field selectors in HSB
 * Input:
 *      index 		- index of field selector 0-15
 *      format 		- Format of field selector
 *      offset 		- Retrieving data offset
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      System support 16 user defined field selctors.
 * 		Each selector can be enabled or disable.
 *      User can defined retrieving 16-bits in many predefiend
 * 		standard l2/l3/l4 payload.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_field_sel_set(rtk_uint32 index, rtk_field_sel_t format, rtk_uint32 offset);

/* Function Name:
 *      rtk_filter_igrAcl_field_sel_get
 * Description:
 *      Get user defined field selectors in HSB
 * Input:
 *      index 	    - index of field selector 0-15
 * Output:
 *      pFormat 	- Format of field selector
 *      pOffset 	- Retrieving data offset
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      None.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_field_sel_get(rtk_uint32 index, rtk_field_sel_t *pFormat, rtk_uint32 *pOffset);

/* Function Name:
 *      rtk_filter_iprange_set
 * Description:
 *      Set IP Range check
 * Input:
 *      index 	    - index of IP Range 0-15
 *      type        - IP Range check type, 0:Delete a entry, 1: IPv4_SIP, 2: IPv4_DIP, 3:IPv6_SIP, 4:IPv6_DIP
 *      upperIp     - The upper bound of IP range
 *      lowerIp     - The lower Bound of IP range
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      upperIp must be larger or equal than lowerIp.
 */
extern rtk_api_ret_t rtk_filter_iprange_set(rtk_uint32 index, rtk_filter_iprange_t type, ipaddr_t upperIp, ipaddr_t lowerIp);

/* Function Name:
 *      rtk_filter_iprange_get
 * Description:
 *      Set IP Range check
 * Input:
 *      index 	    - index of IP Range 0-15
 * Output:
 *      pType        - IP Range check type, 0:Delete a entry, 1: IPv4_SIP, 2: IPv4_DIP, 3:IPv6_SIP, 4:IPv6_DIP
 *      pUpperIp     - The upper bound of IP range
 *      pLowerIp     - The lower Bound of IP range
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      upperIp must be larger or equal than lowerIp.
 */
extern rtk_api_ret_t rtk_filter_iprange_get(rtk_uint32 index, rtk_filter_iprange_t *pType, ipaddr_t *pUpperIp, ipaddr_t *pLowerIp);

/* Function Name:
 *      rtk_filter_vidrange_set
 * Description:
 *      Set VID Range check
 * Input:
 *      index 	    - index of VID Range 0-15
 *      type        - IP Range check type, 0:Delete a entry, 1: CVID, 2: SVID
 *      upperVid    - The upper bound of VID range
 *      lowerVid    - The lower Bound of VID range
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      upperVid must be larger or equal than lowerVid.
 */
extern rtk_api_ret_t rtk_filter_vidrange_set(rtk_uint32 index, rtk_filter_vidrange_t type, rtk_uint32 upperVid, rtk_uint32 lowerVid);

/* Function Name:
 *      rtk_filter_vidrange_get
 * Description:
 *      Get VID Range check
 * Input:
 *      index 	    - index of VID Range 0-15
 * Output:
 *      pType        - IP Range check type, 0:Unused, 1: CVID, 2: SVID
 *      pUpperVid    - The upper bound of VID range
 *      pLowerVid    - The lower Bound of VID range
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      None.
 */
extern rtk_api_ret_t rtk_filter_vidrange_get(rtk_uint32 index, rtk_filter_vidrange_t *pType, rtk_uint32 *pUpperVid, rtk_uint32 *pLowerVid);

/* Function Name:
 *      rtk_filter_portrange_set
 * Description:
 *      Set Port Range check
 * Input:
 *      index 	    - index of Port Range 0-15
 *      type        - IP Range check type, 0:Delete a entry, 1: Source Port, 2: Destnation Port
 *      upperPort   - The upper bound of Port range
 *      lowerPort   - The lower Bound of Port range
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      upperPort must be larger or equal than lowerPort.
 */
extern rtk_api_ret_t rtk_filter_portrange_set(rtk_uint32 index, rtk_filter_portrange_t type, rtk_uint32 upperPort, rtk_uint32 lowerPort);

/* Function Name:
 *      rtk_filter_portrange_get
 * Description:
 *      Set Port Range check
 * Input:
 *      index 	    - index of Port Range 0-15
 * Output:
 *      pType       - IP Range check type, 0:Delete a entry, 1: Source Port, 2: Destnation Port
 *      pUpperPort  - The upper bound of Port range
 *      pLowerPort  - The lower Bound of Port range
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      None.
 */
extern rtk_api_ret_t rtk_filter_portrange_get(rtk_uint32 index, rtk_filter_portrange_t *pType, rtk_uint32 *pUpperPort, rtk_uint32 *pLowerPort);

/*EEE APIs*/
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
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is used to initialize EEE status.
 */
extern rtk_api_ret_t rtk_eee_init(void);

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
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_eee_portEnable_set(rtk_port_t port, rtk_enable_t enable);

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
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can set EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_eee_portEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* IGMP APIs */
/* Function Name:
 *      rtk_igmp_init
 * Description:
 *      This API enables H/W IGMP and set a default initial configuration.
 * Input:
 *      None.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API enables H/W IGMP and set a default initial configuration.
 */
extern rtk_api_ret_t rtk_igmp_init(void);

/* Function Name:
 *      rtk_igmp_state_set
 * Description:
 *      This API set H/W IGMP state.
 * Input:
 *      enabled     - H/W IGMP state
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      This API set H/W IGMP state.
 */
extern rtk_api_ret_t rtk_igmp_state_set(rtk_enable_t enabled);

/* Function Name:
 *      rtk_igmp_state_get
 * Description:
 *      This API get H/W IGMP state.
 * Input:
 *      None.
 * Output:
 *      pEnabled        - H/W IGMP state
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      This API set current H/W IGMP state.
 */
extern rtk_api_ret_t rtk_igmp_state_get(rtk_enable_t *pEnabled);

/* Function Name:
 *      rtk_igmp_static_router_port_set
 * Description:
 *      Configure static router port
 * Input:
 *      portmask    - Static Port mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API set static router port
 */
extern rtk_api_ret_t rtk_igmp_static_router_port_set(rtk_portmask_t portmask);

/* Function Name:
 *      rtk_igmp_static_router_port_get
 * Description:
 *      Get static router port
 * Input:
 *      None.
 * Output:
 *      pPortmask       - Static port mask
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API get static router port
 */
extern rtk_api_ret_t rtk_igmp_static_router_port_get(rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_igmp_protocol_set
 * Description:
 *      set IGMP/MLD protocol action
 * Input:
 *      port        - Port ID
 *      protocol    - IGMP/MLD protocol
 *      action      - Per-port and per-protocol IGMP action seeting
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API set IGMP/MLD protocol action
 */
extern rtk_api_ret_t rtk_igmp_protocol_set(rtk_port_t port, rtk_igmp_protocol_t protocol, rtk_trap_igmp_action_t action);

/* Function Name:
 *      rtk_igmp_protocol_get
 * Description:
 *      set IGMP/MLD protocol action
 * Input:
 *      port        - Port ID
 *      protocol    - IGMP/MLD protocol
 *      action      - Per-port and per-protocol IGMP action seeting
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API set IGMP/MLD protocol action
 */
extern rtk_api_ret_t rtk_igmp_protocol_get(rtk_port_t port, rtk_igmp_protocol_t protocol, rtk_trap_igmp_action_t *pAction);

/* Function Name:
 *      rtk_igmp_fastLeave_set
 * Description:
 *      set IGMP/MLD FastLeave state
 * Input:
 *      state       - ENABLED: Enable FastLeave, DISABLED: disable FastLeave
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API set IGMP/MLD FastLeave state
 */
extern rtk_api_ret_t rtk_igmp_fastLeave_set(rtk_enable_t state);

/* Function Name:
 *      rtk_igmp_fastLeave_get
 * Description:
 *      get IGMP/MLD FastLeave state
 * Input:
 *      None
 * Output:
 *      pState      - ENABLED: Enable FastLeave, DISABLED: disable FastLeave
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - NULL pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API get IGMP/MLD FastLeave state
 */
extern rtk_api_ret_t rtk_igmp_fastLeave_get(rtk_enable_t *pState);

/* Function Name:
 *      rtk_igmp_maxGroup_set
 * Description:
 *      Set per port multicast group learning limit.
 * Input:
 *      port        - Port ID
 *      group       - The number of multicast group learning limit.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_PORT_ID         - Error Port ID
 *      RT_ERR_OUT_OF_RANGE    - parameter out of range
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API set per port multicast group learning limit.
 */
extern rtk_api_ret_t rtk_igmp_maxGroup_set(rtk_port_t port, rtk_uint32 group);

/* Function Name:
 *      rtk_igmp_maxGroup_get
 * Description:
 *      Get per port multicast group learning limit.
 * Input:
 *      port        - Port ID
 * Output:
 *      pGroup      - The number of multicast group learning limit.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_PORT_ID         - Error Port ID
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API get per port multicast group learning limit.
 */
extern rtk_api_ret_t rtk_igmp_maxGroup_get(rtk_port_t port, rtk_uint32 *pGroup);

/* Function Name:
 *      rtk_igmp_currentGroup_get
 * Description:
 *      Get per port multicast group learning count.
 * Input:
 *      port        - Port ID
 * Output:
 *      pGroup      - The number of multicast group learning count.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_PORT_ID         - Error Port ID
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API get per port multicast group learning count.
 */
extern rtk_api_ret_t rtk_igmp_currentGroup_get(rtk_port_t port, rtk_uint32 *pGroup);

#endif /* __RTK_API_EXT_H__ */

