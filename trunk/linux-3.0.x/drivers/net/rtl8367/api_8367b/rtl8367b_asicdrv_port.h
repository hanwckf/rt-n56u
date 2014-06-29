#ifndef _RTL8367B_ASICDRV_PORTSECURITY_H_
#define _RTL8367B_ASICDRV_PORTSECURITY_H_

#include "rtl8367b_asicdrv.h"
#include "rtl8367b_asicdrv_unknownMulticast.h"
#include "rtl8367b_asicdrv_phy.h"

/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

#define	RTL8367B_MAC7		7
#define RTL8367B_EXTNO       3

#define RTL8367B_RTCT_PAGE          (11)
#define RTL8367B_RTCT_RESULT_A_REG  (27)
#define RTL8367B_RTCT_RESULT_B_REG  (28)
#define RTL8367B_RTCT_RESULT_C_REG  (29)
#define RTL8367B_RTCT_RESULT_D_REG  (30)
#define RTL8367B_RTCT_STATUS_REG    (26)

enum L2_SECURITY_BEHAVE
{
    L2_BEHAVE_FLOODING = 0,
    L2_BEHAVE_DROP,
    L2_BEHAVE_TRAP,
    L2_BEHAVE_END
};
enum L2_SECURITY_SA_BEHAVE
{
    L2_BEHAVE_SA_FLOODING = 0,
    L2_BEHAVE_SA_DROP,
    L2_BEHAVE_SA_TRAP,
    L2_BEHAVE_SA_COPY28051,
    L2_BEHAVE_SA_END
};

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
    EXT_RMII_MAC,
    EXT_RMII_PHY,
    EXT_END
};

enum DOSTYPE
{
	DOS_DAEQSA = 0,
	DOS_LANDATTACKS,
	DOS_BLATATTACKS,
	DOS_SYNFINSCAN,
	DOS_XMASCAN,
	DOS_NULLSCAN,
	DOS_SYN1024,
	DOS_TCPSHORTHDR,
	DOS_TCPFRAGERROR,
	DOS_ICMPFRAGMENT,
	DOS_END,

};

typedef struct  rtl8367b_port_ability_s{
#ifdef _LITTLE_ENDIAN
    rtk_uint16 speed:2;
    rtk_uint16 duplex:1;
    rtk_uint16 reserve1:1;
    rtk_uint16 link:1;
    rtk_uint16 rxpause:1;
    rtk_uint16 txpause:1;
    rtk_uint16 nway:1;
    rtk_uint16 mstmode:1;
    rtk_uint16 mstfault:1;
    rtk_uint16 reserve2:2;
    rtk_uint16 forcemode:1;
    rtk_uint16 reserve3:3;
#else
    rtk_uint16 reserve3:3;
    rtk_uint16 forcemode:1;
    rtk_uint16 reserve2:2;
    rtk_uint16 mstfault:1;
    rtk_uint16 mstmode:1;
    rtk_uint16 nway:1;
    rtk_uint16 txpause:1;
    rtk_uint16 rxpause:1;
    rtk_uint16 link:1;
    rtk_uint16 reserve1:1;
    rtk_uint16 duplex:1;
    rtk_uint16 speed:2;

#endif
}rtl8367b_port_ability_t;

typedef struct  rtl8367b_port_status_s{
#ifdef _LITTLE_ENDIAN
    rtk_uint16 speed:2;
    rtk_uint16 duplex:1;
    rtk_uint16 reserve1:1;
    rtk_uint16 link:1;
    rtk_uint16 rxpause:1;
    rtk_uint16 txpause:1;
    rtk_uint16 nway:1;
    rtk_uint16 mstmode:1;
    rtk_uint16 mstfault:1;
    rtk_uint16 lpi100:1;
    rtk_uint16 lpi1000:1;
    rtk_uint16 reserve2:4;
#else
    rtk_uint16 reserve2:4;
    rtk_uint16 lpi1000:1;
    rtk_uint16 lpi100:1;
    rtk_uint16 mstfault:1;
    rtk_uint16 mstmode:1;
    rtk_uint16 nway:1;
    rtk_uint16 txpause:1;
    rtk_uint16 rxpause:1;
    rtk_uint16 link:1;
    rtk_uint16 reserve1:1;
    rtk_uint16 duplex:1;
    rtk_uint16 speed:2;

#endif
}rtl8367b_port_status_t;

typedef struct rtct_result_s
{
    rtk_uint32      channelAShort;
    rtk_uint32      channelBShort;
    rtk_uint32      channelCShort;
    rtk_uint32      channelDShort;

    rtk_uint32      channelAOpen;
    rtk_uint32      channelBOpen;
    rtk_uint32      channelCOpen;
    rtk_uint32      channelDOpen;

    rtk_uint32      channelAMismatch;
    rtk_uint32      channelBMismatch;
    rtk_uint32      channelCMismatch;
    rtk_uint32      channelDMismatch;

    rtk_uint32      channelALinedriver;
    rtk_uint32      channelBLinedriver;
    rtk_uint32      channelCLinedriver;
    rtk_uint32      channelDLinedriver;

    rtk_uint32      channelALen;
    rtk_uint32      channelBLen;
    rtk_uint32      channelCLen;
    rtk_uint32      channelDLen;
} rtl8367b_port_rtct_result_t;


/****************************************************************/
/* Driver Proto Type Definition                                 */
/****************************************************************/
extern ret_t rtl8367b_setAsicPortUnknownDaBehavior(rtk_uint32 behavior);
extern ret_t rtl8367b_getAsicPortUnknownDaBehavior(rtk_uint32 *pBehavior);
extern ret_t rtl8367b_setAsicPortUnknownSaBehavior(rtk_uint32 behavior);
extern ret_t rtl8367b_getAsicPortUnknownSaBehavior(rtk_uint32 *pBehavior);
extern ret_t rtl8367b_setAsicPortUnmatchedSaBehavior(rtk_uint32 behavior);
extern ret_t rtl8367b_getAsicPortUnmatchedSaBehavior(rtk_uint32 *pBehavior);
extern ret_t rtl8367b_setAsicPortUnknownDaFloodingPortmask(rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicPortUnknownDaFloodingPortmask(rtk_uint32 *pPortmask);
extern ret_t rtl8367b_setAsicPortUnknownMulticastFloodingPortmask(rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicPortUnknownMulticastFloodingPortmask(rtk_uint32 *pPortmask);
extern ret_t rtl8367b_setAsicPortBcastFloodingPortmask(rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicPortBcastFloodingPortmask(rtk_uint32 *pPortmask);
extern ret_t rtl8367b_setAsicPortBlockSpa(rtk_uint32 port, rtk_uint32 block);
extern ret_t rtl8367b_getAsicPortBlockSpa(rtk_uint32 port, rtk_uint32 *pBlock);
extern ret_t rtl8367b_setAsicPortForceLink(rtk_uint32 port, rtl8367b_port_ability_t *pPortAbility);
extern ret_t rtl8367b_getAsicPortForceLink(rtk_uint32 port, rtl8367b_port_ability_t *pPortAbility);
extern ret_t rtl8367b_getAsicPortStatus(rtk_uint32 port, rtl8367b_port_status_t *pPortStatus);
extern ret_t rtl8367b_setAsicPortForceLinkExt(rtk_uint32 id, rtl8367b_port_ability_t *pPortAbility);
extern ret_t rtl8367b_getAsicPortForceLinkExt(rtk_uint32 id, rtl8367b_port_ability_t *pPortAbility);
extern ret_t rtl8367b_setAsicPortExtMode(rtk_uint32 id, rtk_uint32 mode);
extern ret_t rtl8367b_getAsicPortExtMode(rtk_uint32 id, rtk_uint32 *pMode);
extern ret_t rtl8367b_setAsicPortDos(rtk_uint32 type, rtk_uint32 drop);
extern ret_t rtl8367b_getAsicPortDos(rtk_uint32 type, rtk_uint32* pDrop);
extern ret_t rtl8367b_setAsicPortEnableAll(rtk_uint32 enable);
extern ret_t rtl8367b_getAsicPortEnableAll(rtk_uint32 *pEnable);
extern ret_t rtl8367b_setAsicPortSmallIpg(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367b_getAsicPortSmallIpg(rtk_uint32 port, rtk_uint32* pEnable);
extern ret_t rtl8367b_setAsicPortLoopback(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367b_getAsicPortLoopback(rtk_uint32 port, rtk_uint32 *pEnable);
extern ret_t rtl8367b_setAsicPortRTCT(rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicPortRTCTResult(rtk_uint32 port, rtl8367b_port_rtct_result_t *pResult);

#endif /*_RTL8367B_ASICDRV_PORTSECURITY_H_*/

