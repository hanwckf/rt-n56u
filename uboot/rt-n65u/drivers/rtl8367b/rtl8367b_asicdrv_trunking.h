#ifndef _RTL8367B_ASICDRV_TRUNKING_H_
#define _RTL8367B_ASICDRV_TRUNKING_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_MAX_TRUNK_GID              (1)
#define RTL8367B_TRUNKING_PORTNO       		(4)
#define RTL8367B_TRUNKING_HASHVALUE_MAX     (15)

extern ret_t rtl8367b_setAsicTrunkingGroup(rtk_uint32 group, rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicTrunkingGroup(rtk_uint32 group, rtk_uint32* pPortmask);
extern ret_t rtl8367b_setAsicTrunkingFlood(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicTrunkingFlood(rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicTrunkingHashSelect(rtk_uint32 hashsel);
extern ret_t rtl8367b_getAsicTrunkingHashSelect(rtk_uint32* pHashsel);

extern ret_t rtl8367b_getAsicQeueuEmptyStatus(rtk_uint32* pPortmask);

extern ret_t rtl8367b_setAsicTrunkingMode(rtk_uint32 mode);
extern ret_t rtl8367b_getAsicTrunkingMode(rtk_uint32* pMode);
extern ret_t rtl8367b_setAsicTrunkingFc(rtk_uint32 group, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicTrunkingFc(rtk_uint32 group, rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicTrunkingHashTable(rtk_uint32 hashval, rtk_uint32 portId);
extern ret_t rtl8367b_getAsicTrunkingHashTable(rtk_uint32 hashval, rtk_uint32* pPortId);

#endif /*_RTL8367B_ASICDRV_TRUNKING_H_*/

