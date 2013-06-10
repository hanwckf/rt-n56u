#ifndef _RTL8367B_ASICDRV_OAM_H_
#define _RTL8367B_ASICDRV_OAM_H_

#include "rtl8367b_asicdrv.h"

enum OAMPARACT
{
	OAM_PARFWD = 0,
	OAM_PARLB,	
	OAM_PARDISCARD,
	OAM_PARFWDCPU
};

enum OAMMULACT
{	
	OAM_MULFWD = 0,
	OAM_MULDISCARD,
	OAM_MULCPU
};

extern ret_t rtl8367b_setAsicOamParser(rtk_uint32 port, rtk_uint32 parser);
extern ret_t rtl8367b_getAsicOamParser(rtk_uint32 port, rtk_uint32* pParser);
extern ret_t rtl8367b_setAsicOamMultiplexer(rtk_uint32 port, rtk_uint32 multiplexer);
extern ret_t rtl8367b_getAsicOamMultiplexer(rtk_uint32 port, rtk_uint32* pMultiplexer);
extern ret_t rtl8367b_setAsicOamCpuPri(rtk_uint32 priority);
extern ret_t rtl8367b_getAsicOamCpuPri(rtk_uint32 *pPriority);
extern ret_t rtl8367b_setAsicOamEnable(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicOamEnable(rtk_uint32 *pEnabled);
#endif /*_RTL8367B_ASICDRV_OAM_H_*/

