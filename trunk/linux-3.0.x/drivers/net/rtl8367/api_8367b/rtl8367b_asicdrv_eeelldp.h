#ifndef _RTL8367B_ASICDRV_EEELLDP_H_
#define _RTL8367B_ASICDRV_EEELLDP_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_EEELLDP_FRAMEU_START    	14
#define RTL8367B_EEELLDP_FRAMEU_END    		40
#define RTL8367B_EEELLDP_FRAMEU_LENGTH    	(RTL8367B_EEELLDP_FRAMEU_END - RTL8367B_EEELLDP_FRAMEU_START + 1)
#define RTL8367B_EEELLDP_FRAMEL_START    	42
#define RTL8367B_EEELLDP_FRAMEL_END    		59
#define RTL8367B_EEELLDP_FRAMEL_LENGTH    	(RTL8367B_EEELLDP_FRAMEL_END - RTL8367B_EEELLDP_FRAMEL_START + 1)

extern ret_t rtl8367b_setAsicEeelldp(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicEeelldp(rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicEeelldpTrapCpu(rtk_uint32 trap);
extern ret_t rtl8367b_getAsicEeelldpTrapCpu(rtk_uint32 *pTrap);
extern ret_t rtl8367b_setAsicEeelldpTrap8051(rtk_uint32 trap);
extern ret_t rtl8367b_getAsicEeelldpTrap8051(rtk_uint32 *pTrap);
extern ret_t rtl8367b_setAsicEeelldpInterrupt8051(rtk_uint32 interrupt);
extern ret_t rtl8367b_getAsicEeelldpInterrupt8051(rtk_uint32 *pInterrupt);
extern ret_t rtl8367b_setAsicEeelldpTrapCpuPri(rtk_uint32 priority);
extern ret_t rtl8367b_getAsicEeelldpTrapCpuPri(rtk_uint32 *pPriority);
extern ret_t rtl8367b_setAsicEeelldpSubtype(rtk_uint32 subtype);
extern ret_t rtl8367b_getAsicEeelldpSubtype(rtk_uint32 * pSubtype);
extern ret_t rtl8367b_setAsicEeelldpRxPortmask(rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicEeelldpRxPortmask(rtk_uint32 *pPortmask);

extern ret_t rtl8367b_getAsicEeelldpRxFrameLower(rtk_uint32 port, rtk_int8 *pFrames);

#endif /*_RTL8367B_ASICDRV_EEELLDP_H_*/

