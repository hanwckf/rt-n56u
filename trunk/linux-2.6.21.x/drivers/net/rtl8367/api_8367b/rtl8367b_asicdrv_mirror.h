#ifndef _RTL8367B_ASICDRV_MIRROR_H_
#define _RTL8367B_ASICDRV_MIRROR_H_

#include "rtl8367b_asicdrv.h"

extern ret_t rtl8367b_setAsicPortMirror(rtk_uint32 source, rtk_uint32 monitor);
extern ret_t rtl8367b_getAsicPortMirror(rtk_uint32 *pSource, rtk_uint32 *pMonitor);
extern ret_t rtl8367b_setAsicPortMirrorRxFunction(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicPortMirrorRxFunction(rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicPortMirrorTxFunction(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicPortMirrorTxFunction(rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicPortMirrorIsolation(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicPortMirrorIsolation(rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicPortMirrorPriority(rtk_uint32 priority);
extern ret_t rtl8367b_getAsicPortMirrorPriority(rtk_uint32* pPriority);
extern ret_t rtl8367b_setAsicPortMirrorMask(rtk_uint32 SourcePortmask);
extern ret_t rtl8367b_getAsicPortMirrorMask(rtk_uint32 *pSourcePortmask);

#endif /*#ifndef _RTL8367B_ASICDRV_MIRROR_H_*/

