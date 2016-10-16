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
#if !defined(SWITCH_ASIC_RTL8370M)
 #undef  RTK_PHY_ID_MAX
 #define RTK_PHY_ID_MAX		4	/* API 8370 used 0..7 ports, redefine to 0..4 */
#endif
#endif

#include "ralink_smi.h"

#if defined(SWITCH_CPU_PORT_EXT2)
#if defined(SWITCH_ASIC_RTL8367RB) || defined(SWITCH_ASIC_RTL8368MB)
 #define CPU_PORT_LAN		(RTK_EXT_2_MAC)		/* ExtIf2 -> RG2 (7) */
 #define CPU_PORT_WAN		(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
 #define EXT_ID_LAN		(EXT_PORT_2)
 #define EXT_ID_WAN		(EXT_PORT_1)
#endif
#elif defined(SWITCH_CPU_PORT_EXT0)
#if defined(SWITCH_ASIC_RTL8367MB) || defined(SWITCH_ASIC_RTL8367MVB) || defined(SWITCH_ASIC_RTL8368MB)
 #define CPU_PORT_LAN		(RTK_EXT_0_MAC)		/* ExtIf0 -> RG0 (5) */
 #define CPU_PORT_WAN		(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
 #define EXT_ID_LAN		(EXT_PORT_0)
 #define EXT_ID_WAN		(EXT_PORT_1)
#elif defined(SWITCH_ASIC_RTL8367M) || defined(SWITCH_ASIC_RTL8370M)
 #define CPU_PORT_LAN		(RTK_EXT_0_MAC9)	/* ExtIf0 -> GMAC2 (9) */
 #define CPU_PORT_WAN		(RTK_EXT_1_MAC8)	/* ExtIf1 -> GMAC1 (8) */
#endif
#else
#if defined(SWITCH_ASIC_RTL8367RB)
 #define CPU_PORT_LAN		(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
 #define CPU_PORT_WAN		(RTK_EXT_2_MAC)		/* ExtIf2 -> RG2 (7) */
 #define EXT_ID_LAN		(EXT_PORT_1)
 #define EXT_ID_WAN		(EXT_PORT_2)
#elif defined(SWITCH_ASIC_RTL8367MB) || defined(SWITCH_ASIC_RTL8367MVB) || defined(SWITCH_ASIC_RTL8368MB)
 #define CPU_PORT_LAN		(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
 #define CPU_PORT_WAN		(RTK_EXT_0_MAC)		/* ExtIf0 -> RG0 (5) */
 #define EXT_ID_LAN		(EXT_PORT_1)
 #define EXT_ID_WAN		(EXT_PORT_0)
#elif defined(SWITCH_ASIC_RTL8367M) || defined(SWITCH_ASIC_RTL8370M)
 #define CPU_PORT_LAN		(RTK_EXT_1_MAC8)	/* ExtIf1 -> GMAC1 (8) */
 #define CPU_PORT_WAN		(RTK_EXT_0_MAC9)	/* ExtIf0 -> GMAC2 (9) */
#endif
#endif

#if defined(SWITCH_ASIC_RTL8365MB)
 #define CPU_PORT_LAN		(RTK_EXT_1_MAC)		/* ExtIf1 -> RG1 (6) */
 #define EXT_ID_LAN		(EXT_PORT_1)
#elif defined(SWITCH_ASIC_RTL8367RVB)
 #define CPU_PORT_LAN		(RTK_EXT_0_MAC)		/* ExtIf1 -> RG0 (5) */
 #define EXT_ID_LAN		(EXT_PORT_1)
#elif defined(SWITCH_ASIC_RTL8367R)
 #define CPU_PORT_LAN		(RTK_EXT_0_MAC9)	/* ExtIf0 -> GMAC2 (9) */
#endif

#if defined(SWITCH_ASIC_RTL8367RB)
 #define ASIC_NAME "RTL8367RB"
#elif defined(SWITCH_ASIC_RTL8367RVB)
 #define ASIC_NAME "RTL8367R-VB"
#elif defined(SWITCH_ASIC_RTL8367MB)
 #define ASIC_NAME "RTL8367MB"
#elif defined(SWITCH_ASIC_RTL8367MVB)
 #define ASIC_NAME "RTL8367M-VB"
#elif defined(SWITCH_ASIC_RTL8365MB)
 #define ASIC_NAME "RTL8365MB"
#elif defined(SWITCH_ASIC_RTL8368MB)
 #define ASIC_NAME "RTL8368MB"
#elif defined(SWITCH_ASIC_RTL8367R)
 #define ASIC_NAME "RTL8367R"
#elif defined(SWITCH_ASIC_RTL8367M)
 #define ASIC_NAME "RTL8367M"
#elif defined(SWITCH_ASIC_RTL8370M)
 #define ASIC_NAME "RTL8370M"
#else
 #define ASIC_NAME "RTL8367"
#endif

#define RTL8367_RGMII_DELAY_TX	SWITCH_RGMII_DELAY_TX
#define RTL8367_RGMII_DELAY_RX	SWITCH_RGMII_DELAY_RX

static void
rtl8367_reset(void)
{
#if defined(API_RTL8367B)
	rtl8367b_setAsicReg(RTL8367B_REG_CHIP_RESET, 1);
#else
	rtl8370_setAsicReg(RTL8370_REG_CHIP_RESET, 1);
#endif
	/* wait 1s for switch ready */
	mdelay(1000);
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
rtl8367_ports_power(int powerOn)
{
	u32 i;

	for (i = 0; i <= RTK_PHY_ID_MAX; i++)
		rtl8367_port_power(i, powerOn);
}

static void
rtl8367_rgmii_config(void)
{
	rtk_port_mac_ability_t mac_cfg;

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
}

static void
rtl8367_partition_bridge_for_recovery(void)
{
	u32 i;
	rtk_portmask_t fwd_mask;

	/* LAN & WAN ports */
	fwd_mask.bits[0] = (1U << CPU_PORT_LAN);
	for (i = 0; i <= RTK_PHY_ID_MAX; i++) {
		rtk_port_isolation_set(i, fwd_mask);
		rtk_port_efid_set(i, 0);
	}

	/* CPU LAN port */
#if defined(SWITCH_ASIC_RTL8370M)
	fwd_mask.bits[0] = 0xFF;
#else
	fwd_mask.bits[0] = 0x1F;
#endif
	rtk_port_isolation_set(CPU_PORT_LAN, fwd_mask);
	rtk_port_efid_set(CPU_PORT_LAN, 0);
}

static void
rtl8367_init_led(void)
{
	rtk_portmask_t portmask;

#if defined(SWITCH_ASIC_RTL8370M)
	portmask.bits[0] = 0xFF;
#else
	portmask.bits[0] = 0x1F;
#endif

#if defined(SWITCH_LED_GROUP0)
	rtk_led_enable_set(LED_GROUP_0, portmask);
#endif
#if defined(SWITCH_LED_GROUP1)
	rtk_led_enable_set(LED_GROUP_1, portmask);
#endif
#if defined(SWITCH_LED_GROUP2)
	rtk_led_enable_set(LED_GROUP_2, portmask);
#endif
	rtk_led_operation_set(LED_OP_PARALLEL);
#if defined(SWITCH_LED_GROUP0) && defined(SWITCH_LED_GROUP1) && defined(SWITCH_LED_GROUP2)
	rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_SPD10010ACT);	// group 0 - green LED
	rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_SPD1000ACT);	// group 1 - yellow LED
	rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_LOOPDETECT);	// group 2 - red LED
#elif defined(SWITCH_LED_GROUP0) && defined(SWITCH_LED_GROUP1)
	rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_SPD10010ACT);	// group 0 - green LED
	rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_SPD1000ACT);	// group 1 - yellow LED
#elif defined(SWITCH_LED_GROUP0) && defined(SWITCH_LED_GROUP2)
	rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_SPD10010ACT);	// group 0 - green LED
	rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_SPD1000ACT);	// group 2 - yellow LED
#elif defined(SWITCH_LED_GROUP1) && defined(SWITCH_LED_GROUP2)
	rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_SPD10010ACT);	// group 1 - green LED
	rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_SPD1000ACT);	// group 2 - yellow LED
#elif defined(SWITCH_LED_GROUP0)
	rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_LINK_ACT);
#elif defined(SWITCH_LED_GROUP1)
	rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_LINK_ACT);
#elif defined(SWITCH_LED_GROUP2)
	rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_LINK_ACT);
#endif
}

int rtl8367_gsw_init_pre(int sw_reset)
{
	smi_init(ASIC_NAME);

	printf(" Init %s GSW...", ASIC_NAME);

#if !defined(MT7621_MP)
	/* wait after power-on-reset */
	/* MT7621 has stage1 code after power-on (duration ~2s), not need wait */
	if (!sw_reset)
		mdelay(400);
#endif

	/* check asic inited */
	if (sw_reset) {
		u32 reg_magic = 0;
#if defined(API_RTL8367B)
		rtl8367b_getAsicReg(RTL8367B_REG_MAGIC_ID, &reg_magic);
#else
		rtl8370_getAsicReg(RTL8370_REG_MAGIC_ID, &reg_magic);
#endif
		if (reg_magic != 0x0249)
			sw_reset = 0;
	}

	if (!sw_reset) {
		/* main switch init */
		if (rtk_switch_init() != RT_ERR_OK) {
			printf("FAILED!\n");
			return -1;
		}
	}

	/* power down all PHY ports (prevent spoofing) */
	rtl8367_ports_power(0);

	printf("SUCCESS!\n");

	return 0;
}

int rtl8367_gsw_init_post(void)
{
	printf(" Reset & Init %s GSW...", ASIC_NAME);

	/* soft reset switch */
	rtl8367_reset();

	/* main switch init */
	if (rtk_switch_init() != RT_ERR_OK) {
		printf("FAILED!\n");
		return -1;
	}

	/* set ports isolation for recovery mode */
	rtl8367_partition_bridge_for_recovery();

	/* configure ExtIf to RGMII, fixed 1000FD mode */
	rtl8367_rgmii_config();

	/* init PHY leds */
	rtl8367_init_led();

	/* enable all PHY (if disabled by bootstrap) */
	rtk_port_phyEnableAll_set(ENABLED);

	printf("SUCCESS!\n");

	return 0;
}

