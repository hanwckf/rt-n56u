#ifndef _RTL8370_ASICDRV_DOT1X_H_
#define _RTL8370_ASICDRV_DOT1X_H_

#include "rtl8370_asicdrv.h"


enum DOT1X_UNAUTH_BEHAV
{
    DOT1X_UNAUTH_DROP = 0,
    DOT1X_UNAUTH_TRAP,
    DOT1X_UNAUTH_GVLAN,
    DOT1X_UNAUTH_MAX
};


extern ret_t rtl8370_setAsic1xPBEnConfig(uint32 port,uint32 enabled);
extern ret_t rtl8370_getAsic1xPBEnConfig(uint32 port,uint32 *enabled);
extern ret_t rtl8370_setAsic1xPBAuthConfig(uint32 port,uint32 auth);
extern ret_t rtl8370_getAsic1xPBAuthConfig(uint32 port,uint32 *auth);
extern ret_t rtl8370_setAsic1xPBOpdirConfig(uint32 port,uint32 opdir);
extern ret_t rtl8370_getAsic1xPBOpdirConfig(uint32 port,uint32 *opdir);
extern ret_t rtl8370_setAsic1xMBEnConfig(uint32 port,uint32 enabled);
extern ret_t rtl8370_getAsic1xMBEnConfig(uint32 port,uint32 *enabled);
extern ret_t rtl8370_setAsic1xMBOpdirConfig(uint32 opdir);
extern ret_t rtl8370_getAsic1xMBOpdirConfig(uint32 *opdir);
extern ret_t rtl8370_setAsic1xProcConfig(uint32 port, uint32 proc);
extern ret_t rtl8370_getAsic1xProcConfig(uint32 port, uint32 *proc);
extern ret_t rtl8370_setAsic1xGuestVidx(uint32 index);
extern ret_t rtl8370_getAsic1xGuestVidx(uint32 *index);
extern ret_t rtl8370_setAsic1xGVOpdir(uint32 enabled);
extern ret_t rtl8370_getAsic1xGVOpdir(uint32 *enabled);
extern ret_t rtl8370_setAsic1xTrapPriority(uint32 priority);
extern ret_t rtl8370_getAsic1xTrapPriority(uint32 *priority);

#endif /*_RTL8370_ASICDRV_DOT1X_H_*/

