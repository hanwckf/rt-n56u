#ifndef _RTL8367B_ASICDRV_PORTISOLATION_H_
#define _RTL8367B_ASICDRV_PORTISOLATION_H_

#include "rtl8367b_asicdrv.h"

extern ret_t rtl8367b_setAsicPortIsolationPermittedPortmask(rtk_uint32 port, rtk_uint32 permitPortmask);
extern ret_t rtl8367b_getAsicPortIsolationPermittedPortmask(rtk_uint32 port, rtk_uint32 *pPermitPortmask);
extern ret_t rtl8367b_setAsicPortIsolationEfid(rtk_uint32 port, rtk_uint32 efid);
extern ret_t rtl8367b_getAsicPortIsolationEfid(rtk_uint32 port, rtk_uint32 *pEfid);

#endif /*_RTL8367B_ASICDRV_PORTISOLATION_H_*/
