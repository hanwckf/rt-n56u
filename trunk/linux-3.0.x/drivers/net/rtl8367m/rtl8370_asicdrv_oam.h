#ifndef _RTL8370_ASICDRV_OAM_H_
#define _RTL8370_ASICDRV_OAM_H_

#include "rtl8370_asicdrv.h"

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

extern ret_t rtl8370_setAsicOamParser(uint32 port, uint32 parser);
extern ret_t rtl8370_getAsicOamParser(uint32 port, uint32* parser);
extern ret_t rtl8370_setAsicOamMultiplexer(uint32 port, uint32 multiplexer);
extern ret_t rtl8370_getAsicOamMultiplexer(uint32 port, uint32* multiplexer);
extern ret_t rtl8370_setAsicOamCpuPri(uint32 priority);
extern ret_t rtl8370_getAsicOamCpuPri(uint32 *priority);
extern ret_t rtl8370_setAsicOamEnable(uint32 enabled);
extern ret_t rtl8370_getAsicOamEnable(uint32 *enabled);
#endif /*_RTL8370_ASICDRV_OAM_H_*/

