#include <common.h>		/* for cpu_to_le32() and cpu_to_le32() */
#include <command.h>
#include <rt_mmap.h>
#include <ralink_gpio.h>

#include "rtl8367b/rtk_api.h"
#include "rtl8367b/rtk_api_ext.h"
#include "rtl8367b/rtl8367b_asicdrv_port.h"

#include "ralink_smi.h"

/* N65U ports mapping */
#define RTL8367_PORT_WAN	0
#define RTL8367_PORT_LAN1	4
#define RTL8367_PORT_LAN2	3
#define RTL8367_PORT_LAN3	2
#define RTL8367_PORT_LAN4	1

#define RTL8367_RGMII_DELAY_TX	1
#define RTL8367_RGMII_DELAY_RX	0

#define mdelay(n)		({unsigned long msec=(n); while (msec--) udelay(1000);})

#if defined(SMI_SCK_GPIO)
#define SMI_SCK			SMI_SCK_GPIO	/* Use SMI_SCK_GPIO as SMI_SCK */
#else
#define SMI_SCK			2		/* Use SMI_SCK/GPIO#2 as SMI_SCK */
#endif

#if defined(SMI_SDA_GPIO)
#define SMI_SDA			SMI_SDA_GPIO	/* Use SMI_SDA_GPIO as SMI_SDA */
#else
#define SMI_SDA			1		/* Use SMI_SDA/GPIO#1 as SMI_SDA */
#endif

static int test_smi_signal_and_wait(void)
{
	int i, good = 0;
	rtk_uint32 data;

	for (i = 0; i < 20; i++) {
		data = 0;
		rtl8367b_getAsicReg(0x1202, &data);
		
		if (data == 0x88a8)
			good++;
		else
			good = 0;
		
		if (good > 2)
			return 0;
		
		mdelay(30);
	}

	return -1;
}

static void rtl8367_port_power(int port, int powerOn)
{
#define PHY_CONTROL_REG			0
#define CONTROL_REG_PORT_POWER_BIT	0x800
	rtk_api_ret_t retVal;
	rtk_port_phy_data_t reg_data;

	if (port < 0 || port > RTK_PHY_ID_MAX)
		return;

	retVal = rtk_port_phyReg_get(port, PHY_CONTROL_REG, &reg_data);
	if (retVal == RT_ERR_OK) {
		if (powerOn)
			reg_data &= ~CONTROL_REG_PORT_POWER_BIT;
		else
			reg_data |= CONTROL_REG_PORT_POWER_BIT;
		
		rtk_port_phyReg_set(port, PHY_CONTROL_REG, reg_data);
	}
}

static void partition_bridge_default(void)
{
	rtk_portmask_t fwd_mask;

	/* LAN ports */
	fwd_mask.bits[0] = (1 << RTK_EXT_1_MAC);
	rtk_port_isolation_set(RTL8367_PORT_LAN1, fwd_mask);
	rtk_port_isolation_set(RTL8367_PORT_LAN2, fwd_mask);
	rtk_port_isolation_set(RTL8367_PORT_LAN3, fwd_mask);
	rtk_port_isolation_set(RTL8367_PORT_LAN4, fwd_mask);

	/* WAN port */
	fwd_mask.bits[0] = (1 << RTK_EXT_1_MAC);
	rtk_port_isolation_set(RTL8367_PORT_WAN, fwd_mask);

	/* CPU port */
	fwd_mask.bits[0] = 0x1F | (1 << RTK_EXT_2_MAC);
	rtk_port_isolation_set(RTK_EXT_1_MAC, fwd_mask);

	/* iNIC port */
	fwd_mask.bits[0] = (1 << RTK_EXT_1_MAC);
	rtk_port_isolation_set(RTK_EXT_2_MAC, fwd_mask);
}

int rtl8367_switch_init_pre(void)
{
	u32 data;
	rtk_api_ret_t retVal;

	data = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));

	/* Configure I2C pin as GPIO mode or I2C mode */
	if (SMI_SCK == 2 || SMI_SDA == 1)
		data |= RALINK_GPIOMODE_I2C;
	else
		data &= ~RALINK_GPIOMODE_I2C;

	/* Configure MDC/MDIO pin as GPIO mode or MDIO mode */
	if (SMI_SCK == 23 || SMI_SDA == 22)
		data |= RALINK_GPIOMODE_MDIO;
	else
		data &= ~RALINK_GPIOMODE_MDIO;

	*((volatile uint32_t *)(RALINK_REG_GPIOMODE)) = cpu_to_le32(data);

	smi_init(SMI_SCK, SMI_SDA);

	printf("\n Init RTL8367 external switch...");

	/* wait min 200ms after power-on-reset */
	mdelay(200);
	test_smi_signal_and_wait();

	/* main switch init */
	retVal = rtk_switch_init();
	if (retVal != RT_ERR_OK) {
		printf("FAILED! (code: %d)\n", retVal);
		return retVal;
	}

	/* power down all ports (prevent spoofing) */
	rtl8367_port_power(RTL8367_PORT_WAN, 0);
	rtl8367_port_power(RTL8367_PORT_LAN4, 0);
	rtl8367_port_power(RTL8367_PORT_LAN3, 0);
	rtl8367_port_power(RTL8367_PORT_LAN2, 0);
	rtl8367_port_power(RTL8367_PORT_LAN1, 0);

	printf("SUCCESS!\n");

	return RT_ERR_OK;
}

int rtl8367_switch_init_post(void)
{
	rtk_api_ret_t retVal;
	rtk_portmask_t portmask;
	rtk_port_mac_ability_t mac_cfg;

	printf(" Reset and init RTL8367 external switch...");

	/* soft reset switch */
	rtl8367b_setAsicReg(RTL8367B_REG_CHIP_RESET, 1);

	/* wait 1s for switch ready */
	mdelay(1000);

	/* main switch init */
	retVal = rtk_switch_init();
	if (retVal != RT_ERR_OK) {
		printf("FAILED! (code: %d)\n", retVal);
		return retVal;
	}

	/* create default ports isolation */
	partition_bridge_default();

	/* configure ExtIf to RGMII, fixed gigabit mode w/o autoneg */
	mac_cfg.forcemode	= MAC_FORCE;
	mac_cfg.speed		= SPD_1000M;
	mac_cfg.duplex		= FULL_DUPLEX;
	mac_cfg.link		= PORT_LINKUP;
	mac_cfg.nway		= DISABLED;
	mac_cfg.rxpause		= ENABLED;
	mac_cfg.txpause		= ENABLED;
	rtk_port_macForceLinkExt_set(EXT_PORT_1, MODE_EXT_RGMII, &mac_cfg);

	/* disable iNIC_mii port link */
	mac_cfg.link		= PORT_LINKDOWN;
	rtk_port_macForceLinkExt_set(EXT_PORT_2, MODE_EXT_RGMII, &mac_cfg);

	/* configure ExtIf RGMII delays */
	rtk_port_rgmiiDelayExt_set(EXT_PORT_1, RTL8367_RGMII_DELAY_TX, RTL8367_RGMII_DELAY_RX);
	rtk_port_rgmiiDelayExt_set(EXT_PORT_2, RTL8367_RGMII_DELAY_TX, RTL8367_RGMII_DELAY_RX);

	/* configure PHY leds */
	portmask.bits[0] = 0x1F;
	rtk_led_enable_set(LED_GROUP_0, portmask);
	rtk_led_enable_set(LED_GROUP_1, portmask);
	rtk_led_operation_set(LED_OP_PARALLEL);
	rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_SPD10010ACT);	// group 0 - green LED
	rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_SPD1000ACT);	// group 1 - yellow LED

	printf("SUCCESS!\n");

	return RT_ERR_OK;
}

int rtk_switch_reg_access(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int cmd = 0, cnt = 1, ret = 0, r, infinit = 0;
	uint32_t reg, data, data1;

	if (!strcmp(argv[0], "rtkswreg.r") && (argc >= 2 && argc <= 3)) {
		cmd = 1;
		if (argc == 3)
			cnt = simple_strtoul(argv[2], NULL, 0);
	} else if (!strcmp(argv[0], "rtkswreg.w") && (argc >= 3 && argc <= 4)) {
		cmd = 2;
		if (argc == 4) {
			cnt = simple_strtoul(argv[3], NULL, 0);
		}
	}
	if (cnt >= 99999)
		infinit = 1;

	if (!infinit && cnt > 1)
		printf("Repeat rtkswreg command %d times\n", cnt);
	while (infinit || cnt-- > 0) {
		switch (cmd) {
		case 1:
			reg = simple_strtoul(argv[1], NULL, 0);
			r = smi_read(reg, &data);
			if (r == RT_ERR_OK)
				printf("Realtek Switch register 0x%x = 0x%x\n", reg, data);
			else {
				printf("%s() smi_read(0x%x) failed. return %d\n", __func__, reg, r);
				ret = 2;
			}
			break;
		case 2:
			reg = simple_strtoul(argv[1], NULL, 0);
			data = simple_strtoul(argv[2], NULL, 0);
			data1 = ~data;
			r = smi_write(reg, data);
			if (r == RT_ERR_OK)
				printf("Realtek Switch register 0x%x = 0x%x\n", reg, data);
			else {
				printf("%s() smi_write(0x%x) failed. return %d\n", __func__, reg, r);
				ret = 3;
			}

			/* Verify */
			if (!ret) {
				r = smi_read(reg, &data1);
				if (r == RT_ERR_OK && data == data1) {
					printf("Write 0x%x to Realtek Switch register 0x%x. Verify OK.\n", data, reg);
				}
				else if (r == RT_ERR_OK) {
					printf("Write 0x%x to Realtek Switch register 0x%x. Got 0x%x. Mismatch.\n", data, reg, data1);
					ret = 4;
				} else {
					printf("%s() smi_read(0x%x) failed. return %d\n", __func__, reg, r);
					ret = 5;
				}
			}
			break;
		default:
#ifdef	CFG_LONGHELP
			printf ("%s\n%s\n", cmdtp->usage, cmdtp->help);
#else
			printf ("Usage:\n%s\n", cmdtp->usage);
#endif
			cnt = 0;
			ret = 1;
		}
	}

	return ret;
}

U_BOOT_CMD(
	rtkswreg,	4,	1,	rtk_switch_reg_access,
	"rtkswreg - Read/Write Realtek Switch register through MDIO interface\n",
	"Usage:\n"
	"rtkswreg.r register_address [count]\n"
	"rtkswreg.w register_address data [count]\n"
);
