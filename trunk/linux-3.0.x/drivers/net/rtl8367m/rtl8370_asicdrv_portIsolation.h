#ifndef _RTL8370_ASICDRV_PORTISOLATION_H_
#define _RTL8370_ASICDRV_PORTISOLATION_H_

#include "rtl8370_asicdrv.h"

extern ret_t rtl8370_setAsicPortIsolationPermittedPortmask(uint32 port, uint32 permitPortmask);
extern ret_t rtl8370_getAsicPortIsolationPermittedPortmask(uint32 port, uint32 *permitPortmask);
extern ret_t rtl8370_setAsicPortIsolationEfid(uint32 port, uint32 efid);
extern ret_t rtl8370_getAsicPortIsolationEfid(uint32 port, uint32 *efid);

#endif /*_RTL8370_ASICDRV_PORTISOLATION_H_*/
