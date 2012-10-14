#ifndef _RTL8370_ASICDRV_EEE_H_
#define _RTL8370_ASICDRV_EEE_H_

#include "rtl8370_asicdrv.h"

typedef struct  rtl8370_eee_status_s
{
#ifdef _LITTLE_ENDIAN
	uint16 eeep_sleep_req:1;
	uint16 eeep_wake_req:1;
    uint16 eee_pause_flag:1;
    uint16 eee_rx:1;	
    uint16 eee_tx:1;   
	uint16 eee_lpi:1;
	uint16 reserved:10;
#else
   uint16 reserved:10;
    uint16 eee_lpi:1; 
    uint16 eee_tx:1;
    uint16 eee_rx:1;    
    uint16 eee_pause_flag:1;
	uint16 eeep_wake_req:1;
	uint16 eeep_sleep_req:1;
#endif
}rtl8370_eee_status_t;


extern ret_t rtl8370_setAsicEeeTxEnable(uint32 port, uint32 enable);
extern ret_t rtl8370_getAsicEeeTxEnable(uint32 port, uint32 *enable);
extern ret_t rtl8370_setAsicEeeRxEnable(uint32 port, uint32 enable);
extern ret_t rtl8370_getAsicEeeRxEnable(uint32 port, uint32 *enable);
extern ret_t rtl8370_setAsicEeeForceMode(uint32 port, uint32 enable);
extern ret_t rtl8370_getAsicEeeForceMode(uint32 port, uint32 *enable);
extern ret_t rtl8370_setAsicEee100M(uint32 port, uint32 enable);
extern ret_t rtl8370_getAsicEee100M(uint32 port, uint32 *enable);
extern ret_t rtl8370_setAsicEeeGiga(uint32 port, uint32 enable);
extern ret_t rtl8370_getAsicEeeGiga(uint32 port, uint32 *enable);
extern ret_t rtl8370_getAsicEeeTxMeter(uint32 port, uint32 *cnt);
extern ret_t rtl8370_getAsicEeeRxMeter(uint32 port, uint32 *cnt);
extern ret_t rtl8370_getAsicEeeStatus(uint32 port, rtl8370_eee_status_t *status);

#endif /*_RTL8370_ASICDRV_EEE_H_*/
