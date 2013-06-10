#ifndef _RTL8367B_ASICDRV_PKG_H_
#define _RTL8367B_ASICDRV_PKG_H_

#include "rtl8367b_asicdrv.h"

#define PKG_PAYLOAD_SIZE	48

enum PKG_CONFIG
{
	PKG_CONFIG_START = 0,
	PKG_CONFIG_PAUSE,
	PKG_CONFIG_CONTINUE,
	PKG_CONFIG_STOP,
	PKG_CONFIG_CRC,
	PKG_CONFIG_RANDOM_DATA,
	PKG_CONFIG_INCREMENTAL_DA,
	PKG_CONFIG_INCREMENTAL_SA,
	PKG_CONFIG_INCREMENTAL_PKT_LENGTH,
	PKG_CONFIG_ENABLE,
	PKG_CONFIG_RANDOM_LENGTH,
	PKG_CONFIG_OVER_TYPE_LENGTH,
};


extern ret_t rtl8367b_setAsicPkgCfg(rtk_uint32 port, rtk_uint32 cfg, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicPkgCfg(rtk_uint32 port, rtk_uint32 cfg, rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicPkgMac(rtk_uint32 port, rtk_uint32 type, smi_ether_addr_t* pMac);
extern ret_t rtl8367b_getAsicPkgMac(rtk_uint32 port, rtk_uint32 type, smi_ether_addr_t* pMac);
extern ret_t rtl8367b_setAsicPkgNum(rtk_uint32 port, rtk_uint32 number);
extern ret_t rtl8367b_getAsicPkgNum(rtk_uint32 port, rtk_uint32* pNumber);
extern ret_t rtl8367b_setAsicPkgLength(rtk_uint32 port, rtk_uint32 length);
extern ret_t rtl8367b_getAsicPkgLength(rtk_uint32 port, rtk_uint32* pLength);
extern ret_t rtl8367b_setAsicPkgMaxLength(rtk_uint32 port, rtk_uint32 length);
extern ret_t rtl8367b_getAsicPkgMaxLength(rtk_uint32 port, rtk_uint32* pLength);
extern ret_t rtl8367b_setAsicPkgBypassFC(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicPkgBypassFC(rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicPkgPayload(rtk_uint32 index, rtk_uint32 payload);
extern ret_t rtl8367b_getAsicPkgPayload(rtk_uint32 index, rtk_uint32* pPayload);

#endif /*#ifndef _RTL8367B_ASICDRV_PKG_H_*/

