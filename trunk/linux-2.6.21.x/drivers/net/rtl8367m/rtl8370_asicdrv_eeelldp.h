#ifndef _RTL8370_ASICDRV_EEELLDP_H_
#define _RTL8370_ASICDRV_EEELLDP_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_EEELLDP_FRAMEU_START    14
#define RTL8370_EEELLDP_FRAMEU_END    40
#define RTL8370_EEELLDP_FRAMEU_LENGTH    (RTL8370_EEELLDP_FRAMEU_END - RTL8370_EEELLDP_FRAMEU_START + 1)
#define RTL8370_EEELLDP_FRAMEL_START    42
#define RTL8370_EEELLDP_FRAMEL_END    59
#define RTL8370_EEELLDP_FRAMEL_LENGTH    (RTL8370_EEELLDP_FRAMEL_END - RTL8370_EEELLDP_FRAMEL_START + 1)

extern ret_t rtl8370_setAsicEeelldp(uint32 enable);
extern ret_t rtl8370_getAsicEeelldp(uint32 *enable);
extern ret_t rtl8370_setAsicEeelldpTrapCpu(uint32 trap);
extern ret_t rtl8370_getAsicEeelldpTrapCpu(uint32 *trap);
extern ret_t rtl8370_setAsicEeelldpTrap8051(uint32 trap);
extern ret_t rtl8370_getAsicEeelldpTrap8051(uint32 *trap);
extern ret_t rtl8370_setAsicEeelldpInterrupt8051(uint32 interrupt_en);
extern ret_t rtl8370_getAsicEeelldpInterrupt8051(uint32 *interrupt_en);
extern ret_t rtl8370_setAsicEeelldpTrapCpuPri(uint32 priority);
extern ret_t rtl8370_getAsicEeelldpTrapCpuPri(uint32 *priority);
extern ret_t rtl8370_setAsicEeelldpSubtype(uint32 subtype);
extern ret_t rtl8370_getAsicEeelldpSubtype(uint32 * subtype);
extern ret_t rtl8370_setAsicEeelldpTxFrameUpper(int8 *frameUPtr);
extern ret_t rtl8370_getAsicEeelldpTxFrameUpper(int8 *frameUPtr);
extern ret_t rtl8370_setAsicEeelldpTxCapFrameLower(int8 *frameLPtr);
extern ret_t rtl8370_getAsicEeelldpTxCapFrameLower(int8 *frameLPtr);
extern ret_t rtl8370_setAsicEeelldpTxAckFrameLower(int8 *frameLPtr);
extern ret_t rtl8370_getAsicEeelldpTxAckFrameLower(int8 *frameLPtr);
extern ret_t rtl8370_setAsicEeelldpRxPortmask(uint32 pmsk);
extern ret_t rtl8370_getAsicEeelldpRxPortmask(uint32 *pmsk);

extern ret_t rtl8370_getAsicEeelldpRxFrameLower(uint32 port, int8 *frameLPtr);

#endif /*_RTL8370_ASICDRV_EEELLDP_H_*/

