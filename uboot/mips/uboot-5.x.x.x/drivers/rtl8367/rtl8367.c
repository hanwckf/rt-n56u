#include <common.h>		/* for cpu_to_le32() and cpu_to_le32() */
#include <command.h>
#include <rt_mmap.h>

#if defined(API_RTL8367B)
#include "api_8367b/rtk_api.h"
#include "api_8367b/rtk_api_ext.h"
#include "api_8367b/rtl8367b_asicdrv_port.h"
#else
#include "api_8370/rtk_api.h"
#include "api_8370/rtk_api_ext.h"
#include "api_8370/rtl8370_asicdrv_port.h"
#endif

#include "ralink_smi.h"

#if defined(SWITCH_CPU_PORT_EXT2)
 #if defined(SWITCH_ASIC_RTL8367RB)
  #define CPU_PORT_LAN		(RTK_EXT_2_MAC)		/* ExtIf2 -> RG2 (7) */
  #define CPU_PORT_WAN		(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
  #define EXT_ID_LAN		(EXT_PORT_2)
  #define EXT_ID_WAN		(EXT_PORT_1)
 #endif
#elif defined(SWITCH_CPU_PORT_EXT0)
 #if defined(SWITCH_ASIC_RTL8367M)
  #define CPU_PORT_LAN		(RTK_EXT_0_MAC9)	/* ExtIf0 -> GMAC2 (9) */
  #define CPU_PORT_WAN		(RTK_EXT_1_MAC8)	/* ExtIf1 -> GMAC1 (8) */
 #endif
 #if defined(SWITCH_ASIC_RTL8367R)
  #define CPU_PORT_LAN		(RTK_EXT_0_MAC9)	/* ExtIf0 -> GMAC2 (9) */
  #undef  CPU_PORT_WAN
 #endif
#else
 #if defined(SWITCH_ASIC_RTL8367M)
  #define CPU_PORT_LAN		(RTK_EXT_1_MAC8)	/* ExtIf1 -> GMAC1 (8) */
  #define CPU_PORT_WAN		(RTK_EXT_0_MAC9)	/* ExtIf0 -> GMAC2 (9) */
 #endif
 #if defined(SWITCH_ASIC_RTL8367RB)
  #define CPU_PORT_LAN		(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
  #define CPU_PORT_WAN		(RTK_EXT_2_MAC)		/* ExtIf2 -> RG2 (7) */
  #define EXT_ID_LAN		(EXT_PORT_1)
  #define EXT_ID_WAN		(EXT_PORT_2)
 #endif
 #if defined(SWITCH_ASIC_RTL8367RVB)
  #define CPU_PORT_LAN		(RTK_EXT_0_MAC)		/* ExtIf1 -> GMAC0 (5) */
  #define EXT_ID_LAN		(EXT_PORT_1)
 #endif
#endif

#if defined(RALINK_DEMO_BOARD_PVLAN)
 // WLLLL, WAN at P0
 #define PHY_PORT_ID_WAN	0
 #define PHY_PORT_ID_LAN	4
#else
 // LLLLW, WAN at P4
 #define PHY_PORT_ID_WAN	4
 #define PHY_PORT_ID_LAN	0
#endif

#define RTL8367_RGMII_DELAY_TX	1
#define RTL8367_RGMII_DELAY_RX	0

#define mdelay(n)		({unsigned long msec=(n); while (msec--) udelay(1000);})

static int
test_asic_ready_and_wait(void)
{
	int i, good = 0;
	u32 data;

	for (i = 0; i < 50; i++) {
		data = 0;
#if defined(API_RTL8367B)
		rtl8367b_getAsicReg(RTL8367B_REG_VS_TPID, &data);
#else
		rtl8370_getAsicReg(RTL8370_REG_VS_TPID, &data);
#endif
//		printf("REG_VS_TPID: 0x%04X\n", data);
		if (data == 0x88a8)
			good++;
		else
			good = 0;
		
		if (good > 2)
			return 0;
		
		mdelay(20);
	}

	return -1;
}

static void
rtl8367_port_power(u32 port, int powerOn)
{
	rtk_api_ret_t retVal;
	rtk_port_phy_data_t reg_data = 0;

	if (port > RTK_PHY_ID_MAX)
		return;

	retVal = rtk_port_phyReg_get(port, 0, &reg_data);
	if (retVal == RT_ERR_OK) {
		if (powerOn) {
			reg_data &= ~(1U<<11);
			reg_data |=  (1U<<9);
		} else {
			reg_data |=  (1U<<11);
		}
		rtk_port_phyReg_set(port, 0, reg_data);
	}
}

static void
partition_bridge_recovery(void)
{
	rtk_portmask_t fwd_mask;

	/* LAN & WAN ports */
	fwd_mask.bits[0] = (1U << CPU_PORT_LAN);
	rtk_port_isolation_set(1, fwd_mask);
	rtk_port_isolation_set(2, fwd_mask);
	rtk_port_isolation_set(3, fwd_mask);
	rtk_port_isolation_set(PHY_PORT_ID_LAN, fwd_mask);
	rtk_port_isolation_set(PHY_PORT_ID_WAN, fwd_mask);

	/* CPU LAN port */
	fwd_mask.bits[0] = 0x1F;
	rtk_port_isolation_set(CPU_PORT_LAN, fwd_mask);
}

int rtl8367_gsw_init_pre(void)
{
	rtk_api_ret_t retVal;
	rtk_port_mac_ability_t mac_cfg;

	smi_init();

	printf("\n Init RTL8367 GSW...");

	/* wait after power-on-reset */
	mdelay(100);
	test_asic_ready_and_wait();

	/* main switch init */
	retVal = rtk_switch_init();
	if (retVal != RT_ERR_OK) {
		printf("FAILED! (code: %d)\n", retVal);
		return retVal;
	}

	/* power down all PHY ports (prevent spoofing) */
	rtl8367_port_power(PHY_PORT_ID_WAN, 0);
	rtl8367_port_power(PHY_PORT_ID_LAN, 0);
	rtl8367_port_power(1, 0);
	rtl8367_port_power(2, 0);
	rtl8367_port_power(3, 0);

	/* Disable CPU RGMII ports link */
	mac_cfg.forcemode	= MAC_FORCE;
	mac_cfg.speed		= SPD_1000M;
	mac_cfg.duplex		= FULL_DUPLEX;
	mac_cfg.link		= PORT_LINKDOWN;
	mac_cfg.nway		= DISABLED;
	mac_cfg.rxpause		= ENABLED;
	mac_cfg.txpause		= ENABLED;

#if defined(API_RTL8367B)
	rtk_port_macForceLinkExt_set(EXT_ID_LAN, MODE_EXT_RGMII, &mac_cfg);
#if defined(EXT_ID_WAN)
	rtk_port_macForceLinkExt_set(EXT_ID_WAN, MODE_EXT_RGMII, &mac_cfg);
#endif
#else
	rtk_port_macForceLinkExt0_set(MODE_EXT_RGMII, &mac_cfg);
	rtk_port_macForceLinkExt1_set(MODE_EXT_RGMII, &mac_cfg);
#endif

	printf("SUCCESS!\n");

	return RT_ERR_OK;
}

int rtl8367_gsw_init_post(void)
{
	rtk_api_ret_t retVal;
	rtk_portmask_t portmask;
	rtk_port_mac_ability_t mac_cfg;

	printf(" Reset and init RTL8367 GSW...");

	/* soft reset switch */
#if defined(API_RTL8367B)
	rtl8367b_setAsicReg(RTL8367B_REG_CHIP_RESET, 1);
#else
	rtl8370_setAsicReg(RTL8370_REG_CHIP_RESET, 1);
#endif

	/* wait 1s for switch ready */
	mdelay(1000);

	/* main switch init */
	retVal = rtk_switch_init();
	if (retVal != RT_ERR_OK) {
		printf("FAILED! (code: %d)\n", retVal);
		return retVal;
	}

	/* create ports isolation for recovery mode */
	partition_bridge_recovery();

	/* configure ExtIf to RGMII, fixed 1000FD mode w/o autoneg */
	mac_cfg.forcemode	= MAC_FORCE;
	mac_cfg.speed		= SPD_1000M;
	mac_cfg.duplex		= FULL_DUPLEX;
	mac_cfg.link		= PORT_LINKUP;
	mac_cfg.nway		= DISABLED;
	mac_cfg.rxpause		= ENABLED;
	mac_cfg.txpause		= ENABLED;

#if defined(API_RTL8367B)
	rtk_port_macForceLinkExt_set(EXT_ID_LAN, MODE_EXT_RGMII, &mac_cfg);
	rtk_port_rgmiiDelayExt_set(EXT_ID_LAN, RTL8367_RGMII_DELAY_TX, RTL8367_RGMII_DELAY_RX);
#if defined(EXT_ID_WAN)
	/* disable second RGMII port */
	mac_cfg.link = PORT_LINKDOWN;
	rtk_port_macForceLinkExt_set(EXT_ID_WAN, MODE_EXT_RGMII, &mac_cfg);
#endif
#else
#if defined(SWITCH_CPU_PORT_EXT1) && !defined(SWITCH_ASIC_RTL8367R)
	rtk_port_macForceLinkExt1_set(MODE_EXT_RGMII, &mac_cfg);
	rtk_port_rgmiiDelayExt1_set(RTL8367_RGMII_DELAY_TX, RTL8367_RGMII_DELAY_RX);

	/* disable second RGMII port */
	mac_cfg.link = PORT_LINKDOWN;
	rtk_port_macForceLinkExt0_set(MODE_EXT_RGMII, &mac_cfg);
#else
	rtk_port_macForceLinkExt0_set(MODE_EXT_RGMII, &mac_cfg);
	rtk_port_rgmiiDelayExt0_set(RTL8367_RGMII_DELAY_TX, RTL8367_RGMII_DELAY_RX);
#if !defined(SWITCH_ASIC_RTL8367R)
	/* disable second RGMII port */
	mac_cfg.link = PORT_LINKDOWN;
	rtk_port_macForceLinkExt1_set(MODE_EXT_RGMII, &mac_cfg);
#endif
#endif
#endif

	/* configure PHY leds */
	portmask.bits[0] = 0x1F;
	rtk_led_enable_set(LED_GROUP_0, portmask);
	rtk_led_enable_set(LED_GROUP_1, portmask);
	rtk_led_operation_set(LED_OP_PARALLEL);
	rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_SPD10010ACT);	// group 0 - green LED (N56U, N65U yet)
	rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_SPD1000ACT);	// group 1 - yellow LED (N56U, N65U yet)

	/* enable all PHY (if disabled by bootstrap) */
	rtk_port_phyEnableAll_set(ENABLED);

	printf("SUCCESS!\n");

	return RT_ERR_OK;
}

