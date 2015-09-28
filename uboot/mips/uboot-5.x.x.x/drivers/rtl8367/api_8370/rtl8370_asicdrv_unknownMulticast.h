#ifndef _RTL8370_ASICDRV_UNKNOWNMULTICAST_H_
#define _RTL8370_ASICDRV_UNKNOWNMULTICAST_H_

#include "rtl8370_asicdrv.h"

enum L2_SECURITY_BEHAVE
{
    L2_BEHAV_FLOODING = 0,
    L2_BEHAV_DROP,
    L2_BEHAV_TRAP,
    L2_BEHAV_MAX
};

enum MULTICASTTYPE{
	MULTICAST_TYPE_IPV4 = 0,
	MULTICAST_TYPE_IPV6,
	MULTICAST_TYPE_L2,
	MULTICAST_TYPE_MAX
};

extern ret_t rtl8370_setAsicUnknownL2MulticastBehavior(uint32 port, uint32 behav);
extern ret_t rtl8370_getAsicUnknownL2MulticastBehavior(uint32 port, uint32 *behav);
extern ret_t rtl8370_setAsicUnknownIPv4MulticastBehavior(uint32 port, uint32 behav);
extern ret_t rtl8370_getAsicUnknownIPv4MulticastBehavior(uint32 port, uint32 *behav);
extern ret_t rtl8370_setAsicUnknownIPv6MulticastBehavior(uint32 port, uint32 behav);
extern ret_t rtl8370_getAsicUnknownIPv6MulticastBehavior(uint32 port, uint32 *behav);
extern ret_t rtl8370_setAsicUnknownMulticastTrapPriority(uint32 priority);
extern ret_t rtl8370_getAsicUnknownMulticastTrapPriority(uint32 *priority);

#endif /*_RTL8370_ASICDRV_UNKNOWNMULTICAST_H_*/


