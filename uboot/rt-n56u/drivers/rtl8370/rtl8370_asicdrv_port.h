#ifndef _RTL8370_ASICDRV_PORTSECURITY_H_
#define _RTL8370_ASICDRV_PORTSECURITY_H_

#include "rtl8370_asicdrv.h"
#include "rtl8370_asicdrv_unknownMulticast.h"

/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

#define	RTL8370_MAC7		7
#define    RTL8370_EXTNO       2


/* enum for port current link speed */
enum SPEEDMODE
{
	SPD_10M = 0,
	SPD_100M,
	SPD_1000M
};

/* enum for mac link mode */
enum LINKMODE
{
	MAC_NORMAL = 0,
	MAC_FORCE,
};

/* enum for port current link duplex mode */
enum DUPLEXMODE
{
	HALF_DUPLEX = 0,
	FULL_DUPLEX
};

/* enum for port current MST mode */
enum MSTMODE
{
	SLAVE_MODE= 0,
	MASTER_MODE
};

enum EXTMODE
{
    EXT_DISABLE = 0,
    EXT_RGMII,
    EXT_MII_MAC,
    EXT_MII_PHY, 
    EXT_TMII_MAC,
    EXT_TMII_PHY, 
    EXT_GMII,
    EXT_RGMII_33V
};

typedef struct  rtl8370_port_ability_s{
#ifdef _LITTLE_ENDIAN
    uint16 speed:2;
    uint16 duplex:1;
    uint16 reserve1:1;
    uint16 link:1;
    uint16 rxpause:1;
    uint16 txpause:1;
    uint16 nway:1;	
    uint16 mstmode:1;
    uint16 mstfault:1;	
    uint16 lpi100:1;
    uint16 lpi1000:1;
    uint16 forcemode:1;
    uint16 reserve3:3;
#else
    uint16 reserve3:3;
    uint16 forcemode:1;
    uint16 lpi1000:1;
    uint16 lpi100:1;
    uint16 mstfault:1; 
    uint16 mstmode:1;
    uint16 nway:1;	
    uint16 txpause:1;
    uint16 rxpause:1;
    uint16 link:1;
    uint16 reserve1:1;	
    uint16 duplex:1;	
    uint16 speed:2;
	
#endif
}rtl8370_port_ability_t;

typedef struct  rtl8370_port_status_s{
#ifdef _LITTLE_ENDIAN
    uint16 speed:2;
    uint16 duplex:1;
    uint16 reserve1:1;
    uint16 link:1;
    uint16 rxpause:1;
    uint16 txpause:1;
    uint16 nway:1;	
    uint16 mstmode:1;
    uint16 mstfault:1;	
    uint16 lpi100:1;
    uint16 lpi1000:1;
    uint16 reserve2:4;
#else
    uint16 reserve2:4;
    uint16 lpi1000:1;
    uint16 lpi100:1;
    uint16 mstfault:1; 
    uint16 mstmode:1;
    uint16 nway:1;	
    uint16 txpause:1;
    uint16 rxpause:1;
    uint16 link:1;
    uint16 reserve1:1;	
    uint16 duplex:1;	
    uint16 speed:2;

#endif
}rtl8370_port_status_t;


/****************************************************************/
/* Driver Proto Type Definition                                 */
/****************************************************************/
extern ret_t rtl8370_setAsicPortForceFlush(uint32 pmsk);
extern ret_t rtl8370_getAsicPortForceFlush(uint32 *pmsk);
extern ret_t rtl8370_setAsicPortDisableAging(uint32 port, uint32 disable);
extern ret_t rtl8370_getAsicPortDisableAging(uint32 port, uint32 *disable);
extern ret_t rtl8370_setAsicPortUnknownDaBehavior(uint32 behavior);
extern ret_t rtl8370_getAsicPortUnknownDaBehavior(uint32 *behavior);
extern ret_t rtl8370_setAsicPortUnknownSaBehavior(uint32 behavior);
extern ret_t rtl8370_getAsicPortUnknownSaBehavior(uint32 *behavior);
extern ret_t rtl8370_setAsicPortUnmatchedSaBehavior(uint32 behavior);
extern ret_t rtl8370_getAsicPortUnmatchedSaBehavior(uint32 *behavior);
extern ret_t rtl8370_setAsicPortUnknownDaFloodingPortmask(uint32 pmsk);
extern ret_t rtl8370_getAsicPortUnknownDaFloodingPortmask(uint32 *pmsk);
extern ret_t rtl8370_setAsicPortUnknownMulticastFloodingPortmask(uint32 pmsk);
extern ret_t rtl8370_getAsicPortUnknownMulticastFloodingPortmask(uint32 *pmsk);
extern ret_t rtl8370_setAsicPortBcastFloodingPortmask(uint32 pmsk);
extern ret_t rtl8370_getAsicPortBcastFloodingPortmask(uint32 *pmsk);
extern ret_t rtl8370_setAsicPortBlockSpa(uint32 port, uint32 block);
extern ret_t rtl8370_getAsicPortBlockSpa(uint32 port, uint32 *block);
extern ret_t rtl8370_setAsicPortForceLink(uint32 port, rtl8370_port_ability_t *portability);
extern ret_t rtl8370_getAsicPortForceLink(uint32 port, rtl8370_port_ability_t *portability);
extern ret_t rtl8370_getAsicPortStatus(uint32 port, rtl8370_port_status_t *portstatus);
extern ret_t rtl8370_setAsicPortForceLinkExt(uint32 id, rtl8370_port_ability_t *portability);
extern ret_t rtl8370_getAsicPortForceLinkExt(uint32 id, rtl8370_port_ability_t *portability);
extern ret_t rtl8370_setAsicPortExtMode(uint32 id, uint32 mode);
extern ret_t rtl8370_getAsicPortExtMode(uint32 id, uint32 *mode);
extern ret_t rtl8370_setAsicPortEnableAll(uint32 enable);
extern ret_t rtl8370_getAsicPortEnableAll(uint32 *enable);

#endif /*_RTL8370_ASICDRV_PORTSECURITY_H_*/

