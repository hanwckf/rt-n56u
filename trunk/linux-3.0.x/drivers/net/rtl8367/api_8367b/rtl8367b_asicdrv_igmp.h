#ifndef _RTL8367B_ASICDRV_IGMP_H_
#define _RTL8367B_ASICDRV_IGMP_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include "rtl8367b_asicdrv.h"

#define RTL8367B_MAX_LEAVE_TIMER        (7)
#define RTL8367B_MAX_QUERY_INT          (0xFFFF)
#define RTL8367B_MAX_ROB_VAR            (7)

#define RTL8367B_IGMP_GOUP_NO		    (256)
#define RTL8367B_IGMP_MAX_GOUP		    (0xFF)
#define RTL8367B_IGMP_GRP_BLEN	        (2)
#define RTL8367B_ROUTER_PORT_INVALID    (0xF)

enum RTL8367B_IGMPTABLE_FULL_OP
{
    TABLE_FULL_FORWARD = 0,
    TABLE_FULL_DROP,
    TABLE_FULL_TRAP,
    TABLE_FULL_OP_END
};

enum RTL8367B_CRC_ERR_OP
{
    CRC_ERR_DROP = 0,
    CRC_ERR_TRAP,
    CRC_ERR_FORWARD,
    CRC_ERR_OP_END
};

enum RTL8367B_IGMP_MLD_PROTOCOL_OP
{
    PROTOCOL_OP_ASIC = 0,
    PROTOCOL_OP_FLOOD,
    PROTOCOL_OP_TRAP,
    PROTOCOL_OP_DROP,
    PROTOCOL_OP_END
};

typedef struct
{
#ifdef _LITTLE_ENDIAN
    rtk_uint32 p0_timer:3;
    rtk_uint32 p1_timer:3;
    rtk_uint32 p2_timer:3;
    rtk_uint32 p3_timer:3;
    rtk_uint32 p4_timer:3;
    rtk_uint32 p5_timer:3;
    rtk_uint32 p6_timer:3;
    rtk_uint32 p7_timer:3;
    rtk_uint32 report_supp_flag:1;
    rtk_uint32 reserved:7;
#else
    rtk_uint32 reserved:7;
    rtk_uint32 report_supp_flag:1;
    rtk_uint32 p7_timer:3;
    rtk_uint32 p6_timer:3;
    rtk_uint32 p5_timer:3;
    rtk_uint32 p4_timer:3;
    rtk_uint32 p3_timer:3;
    rtk_uint32 p2_timer:3;
    rtk_uint32 p1_timer:3;
    rtk_uint32 p0_timer:3;
#endif

}rtl8367b_igmpgroup;
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
 * Feature : IGMP related functions
 *
 */
#include <rtl8367b_asicdrv_igmp.h>

ret_t rtl8367b_setAsicIgmp(rtk_uint32 enabled);
ret_t rtl8367b_getAsicIgmp(rtk_uint32 *pEnabled);
ret_t rtl8367b_setAsicIpMulticastVlanLeaky(rtk_uint32 port, rtk_uint32 enabled );
ret_t rtl8367b_getAsicIpMulticastVlanLeaky(rtk_uint32 port, rtk_uint32 *pEnabled );
ret_t rtl8367b_setAsicIGMPTableFullOP(rtk_uint32 operation);
ret_t rtl8367b_getAsicIGMPTableFullOP(rtk_uint32 *pOperation);
ret_t rtl8367b_setAsicIGMPCRCErrOP(rtk_uint32 operation);
ret_t rtl8367b_getAsicIGMPCRCErrOP(rtk_uint32 *pOperation);
ret_t rtl8367b_setAsicIGMPFastLeaveEn(rtk_uint32 enabled);
ret_t rtl8367b_getAsicIGMPFastLeaveEn(rtk_uint32 *pEnabled);
ret_t rtl8367b_setAsicIGMPLeaveTimer(rtk_uint32 leave_timer);
ret_t rtl8367b_getAsicIGMPLeaveTimer(rtk_uint32 *pLeave_timer);
ret_t rtl8367b_setAsicIGMPQueryInterval(rtk_uint32 interval);
ret_t rtl8367b_getAsicIGMPQueryInterval(rtk_uint32 *pInterval);
ret_t rtl8367b_setAsicIGMPRobVar(rtk_uint32 rob_var);
ret_t rtl8367b_getAsicIGMPRobVar(rtk_uint32 *pRob_var);
ret_t rtl8367b_setAsicIGMPStaticRouterPort(rtk_uint32 pmsk);
ret_t rtl8367b_getAsicIGMPStaticRouterPort(rtk_uint32 *pMsk);
ret_t rtl8367b_getAsicIGMPdynamicRouterPort1(rtk_uint32 *pPort, rtk_uint32 *pTimer);
ret_t rtl8367b_getAsicIGMPdynamicRouterPort2(rtk_uint32 *pPort, rtk_uint32 *pTimer);
ret_t rtl8367b_setAsicIGMPSuppression(rtk_uint32 report_supp_enabled, rtk_uint32 leave_supp_enabled);
ret_t rtl8367b_getAsicIGMPSuppression(rtk_uint32 *pReport_supp_enabled, rtk_uint32 *pLeave_supp_enabled);
ret_t rtl8367b_setAsicIGMPQueryRX(rtk_uint32 port, rtk_uint32 allow_query);
ret_t rtl8367b_getAsicIGMPQueryRX(rtk_uint32 port, rtk_uint32 *pAllow_query);
ret_t rtl8367b_setAsicIGMPReportRX(rtk_uint32 port, rtk_uint32 allow_report);
ret_t rtl8367b_getAsicIGMPReportRX(rtk_uint32 port, rtk_uint32 *pAllow_report);
ret_t rtl8367b_setAsicIGMPLeaveRX(rtk_uint32 port, rtk_uint32 allow_leave);
ret_t rtl8367b_getAsicIGMPLeaveRX(rtk_uint32 port, rtk_uint32 *pAllow_leave);
ret_t rtl8367b_setAsicIGMPMRPRX(rtk_uint32 port, rtk_uint32 allow_mrp);
ret_t rtl8367b_getAsicIGMPMRPRX(rtk_uint32 port, rtk_uint32 *pAllow_mrp);
ret_t rtl8367b_setAsicIGMPMcDataRX(rtk_uint32 port, rtk_uint32 allow_mcdata);
ret_t rtl8367b_getAsicIGMPMcDataRX(rtk_uint32 port, rtk_uint32 *pAllow_mcdata);
ret_t rtl8367b_setAsicIGMPv1Opeartion(rtk_uint32 port, rtk_uint32 igmpv1_op);
ret_t rtl8367b_getAsicIGMPv1Opeartion(rtk_uint32 port, rtk_uint32 *pIgmpv1_op);
ret_t rtl8367b_setAsicIGMPv2Opeartion(rtk_uint32 port, rtk_uint32 igmpv2_op);
ret_t rtl8367b_getAsicIGMPv2Opeartion(rtk_uint32 port, rtk_uint32 *pIgmpv2_op);
ret_t rtl8367b_setAsicIGMPv3Opeartion(rtk_uint32 port, rtk_uint32 igmpv3_op);
ret_t rtl8367b_getAsicIGMPv3Opeartion(rtk_uint32 port, rtk_uint32 *pIgmpv3_op);
ret_t rtl8367b_setAsicMLDv1Opeartion(rtk_uint32 port, rtk_uint32 mldv1_op);
ret_t rtl8367b_getAsicMLDv1Opeartion(rtk_uint32 port, rtk_uint32 *pMldv1_op);
ret_t rtl8367b_setAsicMLDv2Opeartion(rtk_uint32 port, rtk_uint32 mldv2_op);
ret_t rtl8367b_getAsicMLDv2Opeartion(rtk_uint32 port, rtk_uint32 *pMldv2_op);
ret_t rtl8367b_setAsicIGMPPortMAXGroup(rtk_uint32 port, rtk_uint32 max_group);
ret_t rtl8367b_getAsicIGMPPortMAXGroup(rtk_uint32 port, rtk_uint32 *pMax_group);
ret_t rtl8367b_getAsicIGMPPortCurrentGroup(rtk_uint32 port, rtk_uint32 *pCurrent_group);
ret_t rtl8367b_getAsicIGMPGroup(rtk_uint32 idx, rtk_uint32 *pValid, rtl8367b_igmpgroup *pGrp);
ret_t rtl8367b_setAsicIpMulticastPortIsoLeaky(rtk_uint32 port, rtk_uint32 enabled);
ret_t rtl8367b_getAsicIpMulticastPortIsoLeaky(rtk_uint32 port, rtk_uint32 *pEnabled);
ret_t rtl8367b_setAsicIGMPReportFlood(rtk_uint32 flood);
ret_t rtl8367b_getAsicIGMPReportFlood(rtk_uint32 *pFlood);
ret_t rtl8367b_setAsicIGMPDropLeaveZero(rtk_uint32 drop);
ret_t rtl8367b_getAsicIGMPDropLeaveZero(rtk_uint32 *pDrop);
ret_t rtl8367b_setAsicIGMPBypassStormCTRL(rtk_uint32 bypass);
ret_t rtl8367b_getAsicIGMPBypassStormCTRL(rtk_uint32 *pBypass);
ret_t rtl8367b_setAsicIGMPIsoLeaky(rtk_uint32 leaky);
ret_t rtl8367b_getAsicIGMPIsoLeaky(rtk_uint32 *pLeaky);
ret_t rtl8367b_setAsicIGMPVLANLeaky(rtk_uint32 leaky);
ret_t rtl8367b_getAsicIGMPVLANLeaky(rtk_uint32 *pLeaky);

#endif /*#ifndef _RTL8367B_ASICDRV_IGMP_H_*/

