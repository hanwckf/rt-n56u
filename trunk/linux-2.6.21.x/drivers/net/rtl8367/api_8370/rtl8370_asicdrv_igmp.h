#ifndef _RTL8370_ASICDRV_IGMP_H_
#define _RTL8370_ASICDRV_IGMP_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include "rtl8370_asicdrv.h"

enum RTL8370_IGMP
{
    IGMP_FORWARD = 0,
    IGMP_TRAP_TO_CPU,
    IGMP_DROP,
    IGMP_FORWARD_EXCLUDE_CPU,
};

enum RTL8370_IGMP_TYPE
{
    IPV4_IGMP = 0,
    IPV6_MLD,
    PPPOE_IPV4_IGMP,
    PPPOE_IPV6_MLD,
};

typedef struct  rtl8370_igmp_s{

#ifdef _LITTLE_ENDIAN
    uint16 portiso_leaky:1; 
    uint16 vlan_leaky:1;
    uint16 trap_priority:3;
    uint16 discard_storm_filter:1;
    uint16 igmp_trap:2;
    uint16 mld_trap:2;
    uint16 pppoe_igmp_trap:2;
    uint16 pppoe_mld_trap:2;
    uint16 reserved:2;    
#else
    uint16 reserved:2;
    uint16 pppoe_mld_trap:2;
    uint16 pppoe_igmp_trap:2;
    uint16 mld_trap:2;
    uint16 igmp_trap:2;
    uint16 discard_storm_filter:1;
    uint16 trap_priority:3;
    uint16 vlan_leaky:1;
    uint16 portiso_leaky:1; 
#endif

}rtl8370_igmp_t;


extern ret_t rtl8370_setAsicIpMulticastVlanLeaky(uint32 port, uint32 enabled );
extern ret_t rtl8370_getAsicIpMulticastVlanLeaky(uint32 port, uint32 *ptr_enabled );
extern ret_t rtl8370_setAsicIpMulticastPortIsoLeaky( uint32 port, uint32 enabled );
extern ret_t rtl8370_getAsicIpMulticastPortIsoLeaky( uint32 port, uint32* enabled );
extern ret_t rtl8370_getAsicIgmp( rtl8370_igmp_t* igmpcfg);
extern ret_t rtl8370_setAsicIgmp( rtl8370_igmp_t* igmpcfg);

#endif /*#ifndef _RTL8370_ASICDRV_IGMP_H_*/
