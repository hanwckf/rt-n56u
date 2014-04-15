#include <common.h>
#include <command.h>
#include <rt_mmap.h>
#include <ralink_gpio.h>

#include "rtl8370/rtk_api.h"
#include "rtl8370/rtk_api_ext.h"
#include "rtl8370/rtl8370_asicdrv_port.h"
#include "ralink_smi.h"

/* N56U ports mapping */
#define RTL8367_PORT_WAN	4
#define RTL8367_PORT_LAN1	3
#define RTL8367_PORT_LAN2	2
#define RTL8367_PORT_LAN3	1
#define RTL8367_PORT_LAN4	0

/* N56U RGMII delays */
#define RTL8367_RGMII_DELAY_TX	1
#define RTL8367_RGMII_DELAY_RX	0

#define mdelay(n)		({unsigned long msec=(n); while (msec--) udelay(1000);})

static void
rtl8367_init_rgmii(void)
{
	rtk_port_mac_ability_t mac_cfg;

	/* configure ExtIf to RGMII, fixed gigabit mode w/o autoneg */
	mac_cfg.forcemode	= MAC_FORCE;
	mac_cfg.speed		= SPD_1000M;
	mac_cfg.duplex		= FULL_DUPLEX;
	mac_cfg.link		= 1;
	mac_cfg.nway		= 0;
	mac_cfg.rxpause		= 1;
	mac_cfg.txpause		= 1;
	rtk_port_macForceLinkExt1_set(MODE_EXT_RGMII, &mac_cfg);
//	rtk_port_macForceLinkExt0_set(MODE_EXT_RGMII, &mac_cfg);

	/* configure ExtIf RGMII delays */
	rtk_port_rgmiiDelayExt1_set(RTL8367_RGMII_DELAY_TX, RTL8367_RGMII_DELAY_RX);
//	rtk_port_rgmiiDelayExt0_set(RTL8367_RGMII_DELAY_TX, RTL8367_RGMII_DELAY_RX);
}

static void
rtl8367_partition_bridge_default(void)
{
	rtk_portmask_t fwd_mask;

	/* LAN */
	fwd_mask.bits[0] = 0x0F | (1 << 8);
	rtk_port_isolation_set(RTL8367_PORT_LAN1, fwd_mask);
	rtk_port_isolation_set(RTL8367_PORT_LAN2, fwd_mask);
	rtk_port_isolation_set(RTL8367_PORT_LAN3, fwd_mask);
	rtk_port_isolation_set(RTL8367_PORT_LAN4, fwd_mask);
	rtk_port_isolation_set(8, fwd_mask);

	/* WAN */
	fwd_mask.bits[0] = 0x10 | (1 << 9);
	rtk_port_isolation_set(RTL8367_PORT_WAN, fwd_mask);
	rtk_port_isolation_set(9, fwd_mask);

	/* EFID LAN */
	rtk_port_efid_set(RTL8367_PORT_LAN1, 0);
	rtk_port_efid_set(RTL8367_PORT_LAN2, 0);
	rtk_port_efid_set(RTL8367_PORT_LAN3, 0);
	rtk_port_efid_set(RTL8367_PORT_LAN4, 0);
	rtk_port_efid_set(8, 0);

	/* EFID WAN */
	rtk_port_efid_set(RTL8367_PORT_WAN, 1);
	rtk_port_efid_set(9, 1);
}


static void
rtl8367_partition_bridge_emergency(void)
{
	rtk_portmask_t fwd_mask;

	/* LAN & WAN ports */
	fwd_mask.bits[0] = (1 << 8);
	rtk_port_isolation_set(RTL8367_PORT_LAN1, fwd_mask);
	rtk_port_isolation_set(RTL8367_PORT_LAN2, fwd_mask);
	rtk_port_isolation_set(RTL8367_PORT_LAN3, fwd_mask);
	rtk_port_isolation_set(RTL8367_PORT_LAN4, fwd_mask);
	rtk_port_isolation_set(RTL8367_PORT_WAN, fwd_mask);

	/* CPU LAN port */
	fwd_mask.bits[0] = 0x1F;
	rtk_port_isolation_set(8, fwd_mask);

	/* CPU WAN port */
	fwd_mask.bits[0] = 0x00 | (1 << 9);
	rtk_port_isolation_set(9, fwd_mask);

	/* EFID LAN */
	rtk_port_efid_set(RTL8367_PORT_LAN1, 0);
	rtk_port_efid_set(RTL8367_PORT_LAN2, 0);
	rtk_port_efid_set(RTL8367_PORT_LAN3, 0);
	rtk_port_efid_set(RTL8367_PORT_LAN4, 0);
	rtk_port_efid_set(RTL8367_PORT_WAN, 0);
	rtk_port_efid_set(8, 0);

	/* EFID WAN */
	rtk_port_efid_set(9, 1);
}

static void
rtl8367_init_phy_leds(void)
{
	rtk_portmask_t portmask;

	portmask.bits[0] = 0x1F;
	rtk_led_enable_set(LED_GROUP_0, portmask);
	rtk_led_enable_set(LED_GROUP_1, portmask);

	rtk_led_operation_set(LED_OP_PARALLEL);
	rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_SPD10010ACT);
	rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_SPD1000ACT);
}

static void
rtl8367_port_power(rtk_port_t port, int is_power_on)
{
#define PHY_CONTROL_REG			0
#define CONTROL_REG_PORT_POWER_BIT	0x800
	rtk_api_ret_t retVal;
	rtk_port_phy_data_t reg_data;

	if (port < 0 || port > RTK_PHY_ID_MAX)
		return;

	retVal = rtk_port_phyReg_get(port, PHY_CONTROL_REG, &reg_data);
	if (retVal == RT_ERR_OK)
	{
		if (is_power_on)
			reg_data &= ~CONTROL_REG_PORT_POWER_BIT;
		else
			reg_data |= CONTROL_REG_PORT_POWER_BIT;
		
		rtk_port_phyReg_set(port, PHY_CONTROL_REG, reg_data);
	}
}

static void
ralink_init_smi(void)
{
	unsigned long gpiomode;

	gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
	gpiomode |= RALINK_GPIOMODE_I2C;
	gpiomode |= RALINK_GPIOMODE_UARTF;
	gpiomode |= RALINK_GPIOMODE_JTAG;
	*(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpiomode);
}

int
rtl8367_switch_init_pre(void)
{
	rtk_api_ret_t retVal;

	printf("\n Init RTL8367 external switch...");

	ralink_init_smi();

	/* wait min 200ms after power-on-reset */
	mdelay(200);

	/* main switch init */
	retVal = rtk_switch_init();
	if (retVal != RT_ERR_OK) {
		printf("FAILED! (code: %d)\n", retVal);
		return retVal;
	}

	/* power down all ports (prevent spoofing) */
	rtl8367_port_power(RTL8367_PORT_WAN, 0);
	rtl8367_port_power(RTL8367_PORT_LAN1, 0);
	rtl8367_port_power(RTL8367_PORT_LAN2, 0);
	rtl8367_port_power(RTL8367_PORT_LAN3, 0);
	rtl8367_port_power(RTL8367_PORT_LAN4, 0);

	/* create ports isolation */
	rtl8367_partition_bridge_default();

	printf("SUCCESS!\n");

	return RT_ERR_OK;
}


int
rtl8367_switch_init_post(void)
{
	rtk_api_ret_t retVal;

	printf(" Reset and init RTL8367 external switch...");

	/* soft reset switch */
	rtl8370_setAsicReg(0x1322, 1);

	/* wait 1s for switch ready */
	mdelay(1000);

	/* main switch init */
	retVal = rtk_switch_init();
	if (retVal != RT_ERR_OK) {
		printf("FAILED! (code: %d)\n", retVal);
		return retVal;
	}

	/* create ports isolation */
	rtl8367_partition_bridge_emergency();

	/* configure ExtIf to RGMII */
	rtl8367_init_rgmii();

	/* configure PHY leds */
	rtl8367_init_phy_leds();

	printf("SUCCESS!\n");

	return RT_ERR_OK;
}


