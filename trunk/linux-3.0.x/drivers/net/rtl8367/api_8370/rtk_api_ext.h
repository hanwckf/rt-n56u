/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 1.1.1.1 $
 * $Date: 2010/12/02 04:34:44 $
 *
 * Purpose : Definition the error number in the SDK.
 *
 * Feature : error definition
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
extern rtk_api_ret_t rtk_switch_init(void);
extern rtk_api_ret_t rtk_switch_maxPktLen_set(rtk_switch_maxPktLen_t len); 
extern rtk_api_ret_t rtk_switch_maxPktLen_get(rtk_switch_maxPktLen_t *pLen);
extern rtk_api_ret_t rtk_switch_greenEthernet_set(rtk_enable_t enable, rtk_enable_t set_phy_psm);
extern rtk_api_ret_t rtk_switch_greenEthernet_get(rtk_data_t *pEnable);

/* Rate */
extern rtk_api_ret_t rtk_rate_shareMeter_set(rtk_meter_id_t index, rtk_rate_t rate, rtk_enable_t ifg_include);
extern rtk_api_ret_t rtk_rate_shareMeter_get(rtk_meter_id_t index, rtk_rate_t *pRate ,rtk_enable_t *pIfg_include);
extern rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_set( rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include, rtk_enable_t fc_enable);
extern rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include, rtk_enable_t *pFc_enable);
extern rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_set(rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_includ);
extern rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include);
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlEnable_set(rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable);
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlEnable_get(rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlRate_get(rtk_port_t port, rtk_qid_t queue, rtk_data_t *pIndex);
extern rtk_api_ret_t rtk_rate_egrQueueBwCtrlRate_set(rtk_port_t port, rtk_qid_t queue, rtk_data_t index);

/*Storm Control Rate*/
#if defined(EMBEDDED_SUPPORT)
extern rtk_api_ret_t rtk_storm_controlRate_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t rate, rtk_enable_t ifg_include, uint32 mode) reentrant;
extern rtk_api_ret_t rtk_storm_controlRate_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t *pRate, rtk_enable_t *pIfg_include, uint32 mode) reentrant;
#else
extern rtk_api_ret_t rtk_storm_controlRate_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t rate, rtk_enable_t ifg_include, uint32 mode);
extern rtk_api_ret_t rtk_storm_controlRate_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t *pRate, rtk_enable_t *pIfg_include, uint32 mode);
#endif
extern rtk_api_ret_t rtk_storm_bypass_set(rtk_storm_bypass_t type, rtk_enable_t enable);
extern rtk_api_ret_t rtk_storm_bypass_get(rtk_storm_bypass_t type, rtk_enable_t *pEnable);

/* QoS */
extern rtk_api_ret_t rtk_qos_init(rtk_queue_num_t queueNum);
extern rtk_api_ret_t rtk_qos_priSel_set(rtk_priority_select_t *pPriDec);
extern rtk_api_ret_t rtk_qos_priSel_get(rtk_priority_select_t *pPriDec);
extern rtk_api_ret_t rtk_qos_1pPriRemap_set(rtk_pri_t dot1p_pri, rtk_pri_t int_pri);
extern rtk_api_ret_t rtk_qos_1pPriRemap_get(rtk_pri_t dot1p_pri, rtk_pri_t *pInt_pri);
extern rtk_api_ret_t rtk_qos_dscpPriRemap_set(rtk_dscp_t dscp, rtk_pri_t int_pri);
extern rtk_api_ret_t rtk_qos_dscpPriRemap_get(rtk_dscp_t dscp, rtk_pri_t *pInt_pri);
extern rtk_api_ret_t rtk_qos_portPri_set(rtk_port_t port, rtk_pri_t int_pri) ;
extern rtk_api_ret_t rtk_qos_portPri_get(rtk_port_t port, rtk_pri_t *pInt_pri) ;
extern rtk_api_ret_t rtk_qos_queueNum_set(rtk_port_t port, rtk_queue_num_t queue_num);
extern rtk_api_ret_t rtk_qos_queueNum_get(rtk_port_t port, rtk_queue_num_t *pQueue_num);
extern rtk_api_ret_t rtk_qos_priMap_set(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid);
extern rtk_api_ret_t rtk_qos_priMap_get(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid);
extern rtk_api_ret_t rtk_qos_schedulingQueue_set(rtk_port_t port, rtk_qos_queue_weights_t *pQweights);
extern rtk_api_ret_t rtk_qos_schedulingQueue_get(rtk_port_t port, rtk_qos_queue_weights_t *pQweights);
extern rtk_api_ret_t rtk_qos_schedulingAlgorithm_get(rtk_port_t port, rtk_qos_scheduling_type_t *pScheduling_type);
extern rtk_api_ret_t rtk_qos_schedulingAlgorithm_set(rtk_port_t port, rtk_qos_scheduling_type_t scheduling_type);
extern rtk_api_ret_t rtk_qos_1pRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_qos_1pRemarkEnable_set(rtk_port_t port, rtk_enable_t enable); 
extern rtk_api_ret_t rtk_qos_1pRemark_set(rtk_pri_t int_pri, rtk_pri_t dot1p_pri);
extern rtk_api_ret_t rtk_qos_1pRemark_get(rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri);
extern rtk_api_ret_t rtk_qos_dscpRemarkEnable_set(rtk_port_t port, rtk_enable_t enable);
extern rtk_api_ret_t rtk_qos_dscpRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_qos_dscpRemark_set(rtk_pri_t int_pri, rtk_dscp_t dscp);
extern rtk_api_ret_t rtk_qos_dscpRemark_get(rtk_pri_t int_pri, rtk_dscp_t *pDscp);

/* Trap & Reserved Multicast Address (More Action like leaky, bypass storm not define) */
extern rtk_api_ret_t rtk_trap_unknownUnicastPktAction_set(rtk_trap_ucast_type_t type, rtk_trap_ucast_action_t ucast_action); 
extern rtk_api_ret_t rtk_trap_unknownUnicastPktAction_get(rtk_trap_ucast_type_t type, rtk_trap_ucast_action_t *pUcast_action); 
extern rtk_api_ret_t rtk_trap_unknownMcastPktAction_set(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t mcast_action);
extern rtk_api_ret_t rtk_trap_unknownMcastPktAction_get(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t *pMcast_action);
extern rtk_api_ret_t rtk_trap_igmpCtrlPktAction_set(rtk_igmp_type_t type, rtk_trap_igmp_action_t igmp_action);
extern rtk_api_ret_t rtk_trap_igmpCtrlPktAction_get(rtk_igmp_type_t type, rtk_trap_igmp_action_t *pIgmp_action);
extern rtk_api_ret_t rtk_trap_rmaAction_set(rtk_mac_t *pRma_frame, rtk_trap_rma_action_t rma_action);
extern rtk_api_ret_t rtk_trap_rmaAction_get(rtk_mac_t *pRma_frame, rtk_trap_rma_action_t *pRma_action);
extern rtk_api_ret_t rtk_trap_ethernetAv_set(rtk_enable_t enable);
extern rtk_api_ret_t rtk_trap_ethernetAv_get(rtk_data_t *pEnable);

/* Leaky */
extern rtk_api_ret_t rtk_leaky_vlan_set(rtk_leaky_type_t type, rtk_enable_t enable);
extern rtk_api_ret_t rtk_leaky_vlan_get(rtk_leaky_type_t type, rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_leaky_portIsolation_set(rtk_leaky_type_t type, rtk_enable_t enable);
extern rtk_api_ret_t rtk_leaky_portIsolation_get(rtk_leaky_type_t type, rtk_enable_t *pEnable);

/* Port and PHY setting */
extern rtk_api_ret_t rtk_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);
extern rtk_api_ret_t rtk_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility); 
extern rtk_api_ret_t rtk_port_phyForceModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);
extern rtk_api_ret_t rtk_port_phyLink_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus);
extern rtk_api_ret_t rtk_port_phyStatus_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus, rtk_data_t *pSpeed, rtk_data_t *pDuplex);
extern rtk_api_ret_t rtk_port_phyTestMode_set(rtk_port_t port, rtk_port_phy_test_mode_t mode);
extern rtk_api_ret_t rtk_port_phyTestMode_get(rtk_port_t port, rtk_port_phy_test_mode_t *pMode);
extern rtk_api_ret_t rtk_port_phy1000BaseTMasterSlave_set(rtk_port_t port, rtk_enable_t enabled, rtk_enable_t masterslave);
extern rtk_api_ret_t rtk_port_macForceLink_set(rtk_port_t port, rtk_port_mac_ability_t *pPortability);
extern rtk_api_ret_t rtk_port_macForceLink_get(rtk_port_t port, rtk_port_mac_ability_t *pPortability);
extern rtk_api_ret_t rtk_port_macForceLinkExt0_set(rtk_mode_ext_t mode, rtk_port_mac_ability_t *pPortability);
extern rtk_api_ret_t rtk_port_macForceLinkExt0_get(rtk_mode_ext_t *pMode, rtk_port_mac_ability_t *pPortability);
extern rtk_api_ret_t rtk_port_macForceLinkExt1_set(rtk_mode_ext_t mode, rtk_port_mac_ability_t *pPortability);
extern rtk_api_ret_t rtk_port_macForceLinkExt1_get(rtk_mode_ext_t *pMode, rtk_port_mac_ability_t *pPortability);
extern rtk_api_ret_t rtk_port_macStatus_get(rtk_port_t port, rtk_port_mac_ability_t *pPortstatus);
extern rtk_api_ret_t rtk_port_phyReg_set(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t value);
extern rtk_api_ret_t rtk_port_phyReg_get(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t *pData); 
extern rtk_api_ret_t rtk_port_backpressureEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_port_backpressureEnable_set(rtk_port_t port, rtk_enable_t enable); 
extern rtk_api_ret_t rtk_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable);
extern rtk_api_ret_t rtk_port_adminEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_port_isolation_set(rtk_port_t port, rtk_portmask_t portmask);
extern rtk_api_ret_t rtk_port_isolation_get(rtk_port_t port, rtk_portmask_t *pPortmask);
extern rtk_api_ret_t rtk_port_rgmiiDelayExt0_set(rtk_data_t txDelay, rtk_data_t rxDelay);
extern rtk_api_ret_t rtk_port_rgmiiDelayExt0_get(rtk_data_t *pTxDelay, rtk_data_t *pRxDelay);
extern rtk_api_ret_t rtk_port_rgmiiDelayExt1_set(rtk_data_t txDelay, rtk_data_t rxDelay);
extern rtk_api_ret_t rtk_port_rgmiiDelayExt1_get(rtk_data_t *pTxDelay, rtk_data_t *pRxDelay);
extern rtk_api_ret_t rtk_port_phyEnableAll_set(rtk_enable_t enable);
extern rtk_api_ret_t rtk_port_phyEnableAll_get(rtk_data_t *pEnable);
extern rtk_api_ret_t rtk_port_efid_set(rtk_port_t port, rtk_data_t efid);
extern rtk_api_ret_t rtk_port_efid_get(rtk_port_t port, rtk_data_t *pEfid);

/* CVLAN */
extern rtk_api_ret_t rtk_vlan_init(void);
extern rtk_api_ret_t rtk_vlan_set(rtk_vlan_t vid, rtk_portmask_t mbrmsk, rtk_portmask_t untagmsk, rtk_fid_t fid);
extern rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_portmask_t *pMbrmsk, rtk_portmask_t *pUntagmsk, rtk_fid_t *pFid);
extern rtk_api_ret_t rtk_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority);
extern rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority);
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter);
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter);
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type);
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type);
extern rtk_api_ret_t rtk_vlan_vlanBasedPriority_set(rtk_vlan_t vid, rtk_pri_t priority);
extern rtk_api_ret_t rtk_vlan_vlanBasedPriority_get(rtk_vlan_t vid, rtk_pri_t *pPriority);
extern rtk_api_ret_t rtk_vlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode);
extern rtk_api_ret_t rtk_vlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode);
extern rtk_api_ret_t rtk_vlan_stg_get(rtk_vlan_t vid, rtk_stg_t *pStg);
extern rtk_api_ret_t rtk_vlan_stg_set(rtk_vlan_t vid, rtk_stg_t stg);
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_add(rtk_port_t port, rtk_vlan_protoAndPortInfo_t info);
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_get(rtk_port_t port, rtk_vlan_proto_type_t proto_type, rtk_vlan_protoVlan_frameType_t frame_type, rtk_vlan_protoAndPortInfo_t *pInfo);
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_del(rtk_port_t port, rtk_vlan_proto_type_t proto_type, rtk_vlan_protoVlan_frameType_t frame_type);
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_delAll(rtk_port_t port);
extern rtk_api_ret_t rtk_vlan_portFid_set(rtk_port_t port, rtk_enable_t enable, rtk_fid_t fid);
extern rtk_api_ret_t rtk_vlan_portFid_get(rtk_port_t port, rtk_data_t *pEnable, rtk_data_t *pFid);

/*Spanning Tree*/
extern rtk_api_ret_t rtk_stp_init(void);
extern rtk_api_ret_t rtk_stp_mstpState_set(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t stp_state);
extern rtk_api_ret_t rtk_stp_mstpState_get(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t *pStp_state);

/* LUT */
extern rtk_api_ret_t rtk_l2_init(void);
extern rtk_api_ret_t rtk_l2_addr_add(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data);
extern rtk_api_ret_t rtk_l2_addr_get(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data);
extern rtk_api_ret_t rtk_l2_addr_del(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data); 
extern rtk_api_ret_t rtk_l2_mcastAddr_add(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t portmask);
extern rtk_api_ret_t rtk_l2_mcastAddr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t *pPortmask);
extern rtk_api_ret_t rtk_l2_mcastAddr_del(rtk_mac_t *pMac, rtk_fid_t fid);    
extern rtk_api_ret_t rtk_l2_ipMcastAddr_add(ipaddr_t sip, ipaddr_t dip, rtk_portmask_t portmask);
extern rtk_api_ret_t rtk_l2_ipMcastAddr_get(ipaddr_t sip, ipaddr_t dip, rtk_portmask_t *pPortmask);
extern rtk_api_ret_t rtk_l2_ipMcastAddr_del(ipaddr_t sip, ipaddr_t dip);
extern rtk_api_ret_t rtk_l2_flushType_set(rtk_l2_flushType_t type, rtk_vlan_t vid, uint32 portOrTid);
extern rtk_api_ret_t rtk_l2_flushLinkDownPortAddrEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_l2_flushLinkDownPortAddrEnable_set(rtk_port_t port, rtk_enable_t enable);
extern rtk_api_ret_t rtk_l2_agingEnable_set(rtk_port_t port, rtk_enable_t enable);
extern rtk_api_ret_t rtk_l2_agingEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_l2_limitLearningCnt_set(rtk_port_t port, rtk_mac_cnt_t mac_cnt);
extern rtk_api_ret_t rtk_l2_limitLearningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);
extern rtk_api_ret_t rtk_l2_limitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t action);
extern rtk_api_ret_t rtk_l2_limitLearningCntAction_get(rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction);
extern rtk_api_ret_t rtk_l2_learningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);
extern rtk_api_ret_t rtk_l2_floodPortMask_set(rtk_l2_flood_type_t floood_type, rtk_portmask_t flood_portmask);
extern rtk_api_ret_t rtk_l2_floodPortMask_get(rtk_l2_flood_type_t floood_type, rtk_portmask_t *pFlood_portmask);
extern rtk_api_ret_t rtk_l2_localPktPermit_set(rtk_port_t port, rtk_enable_t permit);
extern rtk_api_ret_t rtk_l2_localPktPermit_get(rtk_port_t port, rtk_enable_t *pPermit);
extern rtk_api_ret_t rtk_l2_aging_get(rtk_l2_age_time_t *pAging_time);
extern rtk_api_ret_t rtk_l2_aging_set(rtk_l2_age_time_t aging_time); 
extern rtk_api_ret_t rtk_l2_hashMethod_set(rtk_hash_method_t mode);
extern rtk_api_ret_t rtk_l2_hashMethod_get(rtk_hash_method_t *pMode);
extern rtk_api_ret_t rtk_l2_ipMcastAddrLookup_set(rtk_l2_lookup_type_t type);
extern rtk_api_ret_t rtk_l2_ipMcastAddrLookup_get(rtk_l2_lookup_type_t *pType);
extern rtk_api_ret_t rtk_l2_entry_get(rtk_l2_addr_table_t *pL2_entry);

/* SVLAN */
extern rtk_api_ret_t rtk_svlan_init(void);
extern rtk_api_ret_t rtk_svlan_servicePort_add(rtk_port_t port);
extern rtk_api_ret_t rtk_svlan_servicePort_get(rtk_portmask_t *pSvlan_portmask); 
extern rtk_api_ret_t rtk_svlan_servicePort_del(rtk_port_t port);
extern rtk_api_ret_t rtk_svlan_tpidEntry_set(uint32 svlan_tag_id);
extern rtk_api_ret_t rtk_svlan_tpidEntry_get(uint32 *pSvlan_tag_id);
extern rtk_api_ret_t rtk_svlan_priorityRef_set(rtk_svlan_pri_ref_t ref);
extern rtk_api_ret_t rtk_svlan_priorityRef_get(rtk_svlan_pri_ref_t *pRef);
extern rtk_api_ret_t rtk_svlan_memberPortEntry_set(uint32 svid_idx, rtk_svlan_memberCfg_t *psvlan_cfg);
extern rtk_api_ret_t rtk_svlan_memberPortEntry_get(uint32 svid_idx, rtk_svlan_memberCfg_t *pSvlan_cfg);
extern rtk_api_ret_t rtk_svlan_defaultSvlan_set(rtk_vlan_t svid);
extern rtk_api_ret_t rtk_svlan_defaultSvlan_get(rtk_vlan_t *pSvid);
extern rtk_api_ret_t rtk_svlan_c2s_add(rtk_vlan_t vid, rtk_port_t port, rtk_vlan_t svid);
extern rtk_api_ret_t rtk_svlan_c2s_del(rtk_vlan_t vid, rtk_port_t port);
extern rtk_api_ret_t rtk_svlan_c2s_get(rtk_vlan_t vid, rtk_port_t port, rtk_vlan_t *pSvid);
extern rtk_api_ret_t rtk_svlan_ipmc2s_add(ipaddr_t ipmc, rtk_vlan_t svid);
extern rtk_api_ret_t rtk_svlan_ipmc2s_del(ipaddr_t ipmc);
extern rtk_api_ret_t rtk_svlan_ipmc2s_get(ipaddr_t ipmc, rtk_vlan_t *pSvid);
extern rtk_api_ret_t rtk_svlan_l2mc2s_add(rtk_vlan_t svid, rtk_mac_t mac);
extern rtk_api_ret_t rtk_svlan_l2mc2s_del(rtk_mac_t mac);
extern rtk_api_ret_t rtk_svlan_l2mc2s_get(rtk_mac_t mac, rtk_vlan_t *pSvid);
extern rtk_api_ret_t rtk_svlan_sp2c_add(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t cvid);
extern rtk_api_ret_t rtk_svlan_sp2c_get(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t *pCvid); 
extern rtk_api_ret_t rtk_svlan_sp2c_del(rtk_vlan_t svid, rtk_port_t dst_port);   

/* CPU Port */
extern rtk_api_ret_t rtk_cpu_enable_set(rtk_enable_t enable);
extern rtk_api_ret_t rtk_cpu_enable_get(rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_cpu_tagPort_set(rtk_port_t port, rtk_cpu_insert_t mode);
extern rtk_api_ret_t rtk_cpu_tagPort_get(rtk_port_t *pPort, rtk_cpu_insert_t *pMode);

/* 802.1X */
extern rtk_api_ret_t rtk_dot1x_unauthPacketOper_get(rtk_port_t port, rtk_dot1x_unauth_action_t *pUnauth_action);
extern rtk_api_ret_t rtk_dot1x_unauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action);
extern rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_get(rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_set(rtk_enable_t enable);
extern rtk_api_ret_t rtk_dot1x_portBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_dot1x_portBasedEnable_set(rtk_port_t port, rtk_enable_t enable);
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_get(rtk_port_t port, rtk_dot1x_auth_status_t *pPort_auth);
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_set(rtk_port_t port, rtk_dot1x_auth_status_t port_auth);
extern rtk_api_ret_t rtk_dot1x_portBasedDirection_get(rtk_port_t port, rtk_dot1x_direction_t *pPort_direction);
extern rtk_api_ret_t rtk_dot1x_portBasedDirection_set(rtk_port_t port, rtk_dot1x_direction_t port_direction);
extern rtk_api_ret_t rtk_dot1x_macBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern rtk_api_ret_t rtk_dot1x_macBasedEnable_set(rtk_port_t port, rtk_enable_t enable);
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_add(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid);
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_del(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid);
extern rtk_api_ret_t rtk_dot1x_macBasedDirection_get(rtk_dot1x_direction_t *pMac_direction);
extern rtk_api_ret_t rtk_dot1x_macBasedDirection_set(rtk_dot1x_direction_t mac_direction);
extern rtk_api_ret_t rtk_dot1x_guestVlan_set(rtk_vlan_t vid);
extern rtk_api_ret_t rtk_dot1x_guestVlan_get(rtk_vlan_t *pVid);
extern rtk_api_ret_t rtk_dot1x_guestVlan2Auth_set(rtk_enable_t enable);
extern rtk_api_ret_t rtk_dot1x_guestVlan2Auth_get(rtk_enable_t *pEnable);

/* Port Trunk */
extern rtk_api_ret_t rtk_trunk_port_get(rtk_trunk_group_t trk_gid, rtk_portmask_t *pTrunk_member_portmask);
extern rtk_api_ret_t rtk_trunk_port_set(rtk_trunk_group_t trk_gid, rtk_portmask_t trunk_member_portmask); 
extern rtk_api_ret_t rtk_trunk_distributionAlgorithm_get(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t *pAlgo_bitmask);
extern rtk_api_ret_t rtk_trunk_distributionAlgorithm_set(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t algo_bitmask);
extern rtk_api_ret_t rtk_trunk_qeueuEmptyStatus_get(rtk_portmask_t *pPortmask);

/*Port Mirror */
extern rtk_api_ret_t rtk_mirror_portBased_set(rtk_port_t mirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);
extern rtk_api_ret_t rtk_mirror_portBased_get(rtk_port_t* pMirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);
extern rtk_api_ret_t rtk_mirror_portIso_set(rtk_enable_t enable);
extern rtk_api_ret_t rtk_mirror_portIso_get(rtk_enable_t *pEnable);

/* MIB */
#ifdef EMBEDDED_SUPPORT
extern rtk_api_ret_t rtk_stat_global_reset(void);
extern rtk_api_ret_t rtk_stat_port_reset(rtk_port_t port);
extern rtk_api_ret_t rtk_stat_port_get(rtk_port_t port, rtk_stat_port_type_t cntr_idx, rtk_stat_counter_t *pCntrH, rtk_stat_counter_t *pCntrL);
extern rtk_api_ret_t rtk_stat_global_get(rtk_stat_global_type_t cntr_idx, rtk_stat_counter_t *pCntrH, rtk_stat_counter_t *pCntrL);
#else
extern rtk_api_ret_t rtk_stat_global_reset(void);
extern rtk_api_ret_t rtk_stat_port_reset(rtk_port_t port);
extern rtk_api_ret_t rtk_stat_global_get(rtk_stat_global_type_t cntr_idx, rtk_stat_counter_t *pCntr);
extern rtk_api_ret_t rtk_stat_global_getAll(rtk_stat_global_cntr_t *pGlobal_cntrs);
extern rtk_api_ret_t rtk_stat_port_get(rtk_port_t port, rtk_stat_port_type_t cntr_idx, rtk_stat_counter_t *pCntr);
extern rtk_api_ret_t rtk_stat_port_getAll(rtk_port_t port, rtk_stat_port_cntr_t *pPort_cntrs);
#endif

/* Interrupt */
extern rtk_api_ret_t rtk_int_polarity_set(rtk_int_polarity_t type);
extern rtk_api_ret_t rtk_int_polarity_get(rtk_int_polarity_t *pType);
extern rtk_api_ret_t rtk_int_control_set(rtk_int_type_t type, rtk_enable_t enable);
extern rtk_api_ret_t rtk_int_control_get(rtk_int_type_t type, rtk_enable_t* pEnable);
extern rtk_api_ret_t rtk_int_status_set(rtk_int_status_t statusMask);
extern rtk_api_ret_t rtk_int_status_get(rtk_int_status_t* pStatusMask);
extern rtk_api_ret_t rtk_int_advanceInfo_get(rtk_int_advType_t adv_type, rtk_int_info_t* info);

/*LED*/
extern rtk_api_ret_t rtk_led_enable_set(rtk_led_group_t group, rtk_portmask_t portmask);
extern rtk_api_ret_t rtk_led_enable_get(rtk_led_group_t group, rtk_portmask_t *pPortmask);
extern rtk_api_ret_t rtk_led_operation_set(rtk_led_operation_t mode);
extern rtk_api_ret_t rtk_led_operation_get(rtk_data_t *pMode);
extern rtk_api_ret_t rtk_led_mode_set(rtk_led_mode_t mode);
extern rtk_api_ret_t rtk_led_mode_get(rtk_led_mode_t *pMode);
extern rtk_api_ret_t rtk_led_modeForce_set(rtk_led_group_t group, rtk_led_force_mode_t mode);
extern rtk_api_ret_t rtk_led_modeForce_get(rtk_data_t *pGroupmask, rtk_data_t *pMode);
extern rtk_api_ret_t rtk_led_blinkRate_set(rtk_led_blink_rate_t blinkRate);
extern rtk_api_ret_t rtk_led_blinkRate_get(rtk_led_blink_rate_t *pBlinkRate);
extern rtk_api_ret_t rtk_led_groupConfig_set(rtk_led_group_t group, rtk_led_congig_t config);
extern rtk_api_ret_t rtk_led_groupConfig_get(rtk_led_group_t group, rtk_data_t *pConfig);
extern rtk_api_ret_t rtk_led_serialMode_set(rtk_led_active_t active);
extern rtk_api_ret_t rtk_led_serialMode_get(rtk_data_t *pActive);


/*Green ethernet*/
extern rtk_api_ret_t rtk_green_feature_set(uint32 greenFeature, uint32 powerSaving);
extern rtk_api_ret_t rtk_green_feature_get(uint32 greenFeature, uint32 powerSaving);

/*ACL APIs*/
extern rtk_api_ret_t rtk_filter_igrAcl_init(void);
extern rtk_api_ret_t rtk_filter_igrAcl_field_add(rtk_filter_cfg_t *pFilter_cfg, rtk_filter_field_t *pFilter_field);
extern rtk_api_ret_t rtk_filter_igrAcl_cfg_add(rtk_filter_id_t filter_id, rtk_filter_cfg_t *pFilter_cfg, rtk_filter_action_t *pAction, rtk_filter_number_t *ruleNum);
extern rtk_api_ret_t rtk_filter_igrAcl_cfg_del(rtk_filter_id_t filter_id);
extern rtk_api_ret_t rtk_filter_igrAcl_cfg_delAll(void);
extern rtk_api_ret_t rtk_filter_igrAcl_cfg_set(rtk_filter_id_t filter_id, rtk_filter_template_index_t template_index, rtk_filter_cfg_raw_t *pFilter_cfg, rtk_filter_action_t *pFilter_action);
extern rtk_api_ret_t rtk_filter_igrAcl_cfg_get(rtk_filter_id_t filter_id, rtk_filter_cfg_raw_t *pFilter_cfg, rtk_filter_action_t *pAction);
extern rtk_api_ret_t rtk_filter_igrAcl_unmatchAction_set(rtk_port_t port, rtk_filter_unmatch_action_t action);
extern rtk_api_ret_t rtk_filter_igrAcl_unmatchAction_get(rtk_port_t port, rtk_filter_unmatch_action_t* action);
extern rtk_api_ret_t rtk_filter_igrAcl_state_set(rtk_port_t port, rtk_filter_state_t state);
extern rtk_api_ret_t rtk_filter_igrAcl_state_get(rtk_port_t port, rtk_filter_state_t* state);
extern rtk_api_ret_t rtk_filter_igrAcl_template_set(rtk_filter_template_t *aclTemplate);
extern rtk_api_ret_t rtk_filter_igrAcl_template_get(rtk_filter_template_t *aclTemplate);

/*RLDP APIs*/
extern rtk_api_ret_t rtk_rldp_init(rtk_portmask_t txPortmask, rtk_mac_t Mac);
extern rtk_api_ret_t rtk_rldp_getLoopPortmask(rtk_portmask_t* loopedPortmask);
extern rtk_api_ret_t rtk_rldp_getLoopedPortPair(uint32 port, uint32 *portPair);


/*EEE APIs*/
extern rtk_api_ret_t rtk_eee_init(void);
extern rtk_api_ret_t rtk_eee_portEnable_set(rtk_port_t port, rtk_enable_t enable);
extern rtk_api_ret_t rtk_eee_portEnable_get(rtk_port_t port, rtk_data_t *pEnable);

extern rtk_api_ret_t rtk_aldp_init(void);
extern rtk_api_ret_t rtk_aldp_enable_set(rtk_enable_t enable);
extern rtk_api_ret_t rtk_aldp_enable_get(rtk_data_t *pEnable);

extern rtk_api_ret_t rtk_port_rtct_init(void);
extern rtk_api_ret_t rtk_port_rtctEnable_set(rtk_portmask_t portmask);
extern rtk_api_ret_t rtk_port_rtctResult_get(rtk_port_t port, rtk_rtctResult_t *pRtctResult);

#endif /* __RTK_API_EXT_H__ */

