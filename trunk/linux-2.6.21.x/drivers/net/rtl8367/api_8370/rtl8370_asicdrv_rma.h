#ifndef _RTL8370_ASICDRV_RMA_H_
#define _RTL8370_ASICDRV_RMA_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_RMAMAX                     0x2F

enum RTL8370_RMAOP
{
    RMAOP_FORWARD = 0,
    RMAOP_TRAP_TO_CPU,
    RMAOP_DROP,
    RMAOP_FORWARD_EXCLUDE_CPU,
};


typedef struct  rtl8370_rma_s{

#ifdef _LITTLE_ENDIAN
    uint16 portiso_leaky:1; 
    uint16 vlan_leaky:1;
    uint16 keep_format:1;
    uint16 trap_priority:3;
    uint16 discard_storm_filter:1;
    uint16 operation:2;
    uint16 reserved:7;
#else
    uint16 reserved:7;
    uint16 operation:2; 
    uint16 discard_storm_filter:1;
    uint16 trap_priority:3;
    uint16 keep_format:1;
    uint16 vlan_leaky:1;
    uint16 portiso_leaky:1; 
#endif

}rtl8370_rma_t;


extern ret_t rtl8370_setAsicRma(uint32 index, rtl8370_rma_t* rmacfg);
extern ret_t rtl8370_getAsicRma(uint32 index, rtl8370_rma_t* rmacfg);

#endif /*#ifndef _RTL8370_ASICDRV_RMA_H_*/

