#ifndef _RTL8367B_ASICDRV_RRCP_H_
#define _RTL8367B_ASICDRV_RRCP_H_

#include "rtl8367b_asicdrv.h"

enum RTL8367B_RRCPV3_HANDLE
{
    RRCPHANDLE_UNAWARE,
	RRCPHANDLE_TRAPSWMAC,
	RRCPHANDLE_TRAPALL,
	RRCPHANDLE_END
};

enum RTL8367B_MALFORMED_HANDLE
{
    RRCPMALFORMED_FWD,
	RRCPMALFORMED_TRAP,
	RRCPMALFORMED_DROP,
	RRCPMALFORMED_END
};

enum RTL8367B_RRCP_TAG
{
    RRCPTAG_KEEP,
	RRCPTAG_ALE,
	RRCPTAG_FIXED,
	RRCPTAG_UNTAG,
	RRCPTAG_END
};

extern ret_t rtl8367b_setAsicRrcp(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRrcp(rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicRrcpAuthPortmask(rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicRrcpAuthPortmask(rtk_uint32* pPortmask);
extern ret_t rtl8367b_setAsicRrcpAdminPortmask(rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicRrcpAdminPortmask(rtk_uint32* pPortmask);
extern ret_t rtl8367b_setAsicRrcpAuthenticationKey(rtk_uint32 authKey);
extern ret_t rtl8367b_getAsicRrcpAuthenticationKey(rtk_uint32 *pAuthKey);
extern ret_t rtl8367b_setAsicRrcpVendorId(rtk_uint32 id);
extern ret_t rtl8367b_getAsicRrcpVendorId(rtk_uint32* pId);
extern ret_t rtl8367b_setAsicRrcpCustomerCode(rtk_uint32 code);
extern ret_t rtl8367b_getAsicRrcpCustomerCode(rtk_uint32* pCode);
extern ret_t rtl8367b_setAsicRrcpPrivateKey(rtk_uint32 key);
extern ret_t rtl8367b_getAsicRrcpPrivateKey(rtk_uint32* pKey);
extern ret_t rtl8367b_setAsicRrcpv3Handle(rtk_uint32 handle);
extern ret_t rtl8367b_getAsicRrcpv3Handle(rtk_uint32 *pHandle);
extern ret_t rtl8367b_setAsicRrcpv1Handle(rtk_uint32 handle);
extern ret_t rtl8367b_getAsicRrcpv1Handle(rtk_uint32 *pHandle);
extern ret_t rtl8367b_setAsicRrcpv1GetCrc(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRrcpv1GetCrc(rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicRrcpv1SetCrc(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRrcpv1SetCrc(rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicRrcpv3Crc(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRrcpv3Crc(rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicRrcpVlanLeaky(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRrcpVlanLeaky(rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicRrcpPbVlan(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRrcpPbVlan(rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicRrcpMalformAct(rtk_uint32 handle);
extern ret_t rtl8367b_getAsicRrcpMalformAct(rtk_uint32 *pHandle);
extern ret_t rtl8367b_setAsicRrcpIndication(rtk_uint32 period, rtk_uint32 time);
extern ret_t rtl8367b_getAsicRrcpIndication(rtk_uint32* pPeriod, rtk_uint32* pTime);
extern ret_t rtl8367b_setAsicRrcpHelloTag(rtk_uint32 format);
extern ret_t rtl8367b_getAsicRrcpHelloTag(rtk_uint32 *pFormat);
extern ret_t rtl8367b_setAsicRrcpFwdTag(rtk_uint32 format);
extern ret_t rtl8367b_getAsicRrcpFwdTag(rtk_uint32 *pFormat);
extern ret_t rtl8367b_setAsicRrcpReplyTag(rtk_uint32 format);
extern ret_t rtl8367b_getAsicRrcpReplyTag(rtk_uint32 *pFormat);
extern ret_t rtl8367b_setAsicRrcpVidPri(rtk_uint32 vid, rtk_uint32 priority);
extern ret_t rtl8367b_getAsicRrcpVidPri(rtk_uint32* pVid, rtk_uint32* pPriority);
#endif /*_RTL8367B_ASICDRV_RRCP_H_*/

