#ifndef _RTL8370_ASICDRV_MIRROR_H_
#define _RTL8370_ASICDRV_MIRROR_H_

#include "rtl8370_asicdrv.h"

extern ret_t rtl8370_setAsicPortMirror(uint32 source, uint32 monitor);
extern ret_t rtl8370_getAsicPortMirror(uint32 *source, uint32 *monitor);
extern ret_t rtl8370_setAsicPortMirrorRxFunction(uint32 enabled);
extern ret_t rtl8370_getAsicPortMirrorRxFunction(uint32* enabled);
extern ret_t rtl8370_setAsicPortMirrorTxFunction(uint32 enabled);
extern ret_t rtl8370_getAsicPortMirrorTxFunction(uint32* enabled);
extern ret_t rtl8370_setAsicPortMirrorIsolation(uint32 enabled);
extern ret_t rtl8370_getAsicPortMirrorIsolation(uint32* enabled);
extern ret_t rtl8370_setAsicPortMirrorPriority(uint32 priority);
extern ret_t rtl8370_getAsicPortMirrorPriority(uint32* priority);

#endif /*#ifndef _RTL8370_ASICDRV_MIRROR_H_*/

