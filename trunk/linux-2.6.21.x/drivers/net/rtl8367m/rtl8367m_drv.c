/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/rtl8367m_drv.h>

#include "ralink_smi.h"
#include "rtk_types.h"
#include "rtk_error.h"
#include "rtk_api_ext.h"
#include "rtl8370_asicdrv_port.h"
#include "rtl8370_asicdrv_vlan.h"
#include "rtl8370_asicdrv_green.h"

#define RTL8367M_DEVNAME			"rtl8367m"
#define RTL8367M_DEVMAJOR			(206)

#define SMI_RTL8367_SMI_ADDR			0xB8
#define SMI_RTL8367_DELAY_NS			1500
#define SMI_RALINK_GPIO_SDA			CONFIG_RTL8367M_SMI_BUS_CPU_GPIO_DATA	/* GPIO used for SMI Data signal */
#define SMI_RALINK_GPIO_SCK			CONFIG_RTL8367M_SMI_BUS_CPU_GPIO_CLCK	/* GPIO used for SMI Clock signal */
#define WAN_PORT_X				CONFIG_RTL8367M_PORT_WAN	/* 8P8C WAN  */
#define LAN_PORT_1				CONFIG_RTL8367M_PORT_LAN1	/* 8P8C LAN1 */
#define LAN_PORT_2				CONFIG_RTL8367M_PORT_LAN2	/* 8P8C LAN2 */
#define LAN_PORT_3				CONFIG_RTL8367M_PORT_LAN3	/* 8P8C LAN3 */
#define LAN_PORT_4				CONFIG_RTL8367M_PORT_LAN4	/* 8P8C LAN4 */

#if !defined(CONFIG_RAETH_GMAC2) || defined(CONFIG_RTL8367M_ASIC_RVB)
#define RTL8367M_SINGLE_EXTIF 1
#endif

#if defined(CONFIG_RTL8367M_ASIC_RVB)
#define LAN_PORT_CPU				(5) /* ExtIf1 -> GMAC1 (port set reduced to 0..5 in RTL8367R-VB) */
#else
#define LAN_PORT_CPU				(8) /* ExtIf1 -> GMAC1 */
#endif

#if defined(RTL8367M_SINGLE_EXTIF)
#define RTL8367M_VLANID_LAN			CONFIG_RA_HW_NAT_LAN_VLANID
#define RTL8367M_VLANID_WAN			CONFIG_RA_HW_NAT_WAN_VLANID
#define WAN_PORT_CPU				LAN_PORT_CPU
#else
#define WAN_PORT_CPU				(9) /* ExtIf0 -> GMAC2 */
#endif

#define RTL8367M_DEFAULT_JUMBO_FRAMES		1
#define RTL8367M_DEFAULT_GREEN_ETHERNET		1
#define RTL8367M_DEFAULT_STORM_RATE		1024
#define RTL8367M_DEFAULT_LINK_MODE		0

////////////////////////////////////////////////////////////////////////////////////

static DEFINE_MUTEX(asic_access_mutex);

static u32 g_wan_bridge_mode                     = RTL8367M_WAN_BRIDGE_DISABLE;
static u32 g_wan_bridge_isolated_mode            = RTL8367M_WAN_BWAN_ISOLATION_NONE;

static u32 g_led_phy_mode_group0                 = RTL8367M_LED_PHYMODE_100_10_ACT;
static u32 g_led_phy_mode_group1                 = RTL8367M_LED_PHYMODE_1000_ACT;
static u32 g_led_phy_mode_group2                 = RTL8367M_LED_OFF;

static u32 g_jumbo_frames_enabled                = RTL8367M_DEFAULT_JUMBO_FRAMES;
static u32 g_green_ethernet_enabled              = RTL8367M_DEFAULT_GREEN_ETHERNET;
static u32 g_storm_rate_unicast_unknown          = RTL8367M_DEFAULT_STORM_RATE;
static u32 g_storm_rate_multicast_unknown        = RTL8367M_DEFAULT_STORM_RATE;
static u32 g_storm_rate_multicast                = RTL8367M_DEFAULT_STORM_RATE;
static u32 g_storm_rate_broadcast                = RTL8367M_DEFAULT_STORM_RATE;

static u32 g_port_link_mode[RTK_PHY_ID_MAX+1]    = {RTL8367M_DEFAULT_LINK_MODE};

static u32 g_rgmii_delay_tx                      = CONFIG_RTL8367M_RGMII_DELAY_TX;	/* 0..1 */
static u32 g_rgmii_delay_rx                      = CONFIG_RTL8367M_RGMII_DELAY_RX;	/* 0..7 */

////////////////////////////////////////////////////////////////////////////////////

unsigned int get_phy_ports_mask_from_user(unsigned int user_port_mask)
{
	unsigned int phy_ports_mask = 0;

	if (user_port_mask & RTL8367M_PORTMASK_LAN1)
		phy_ports_mask |= (1L << LAN_PORT_1);
	if (user_port_mask & RTL8367M_PORTMASK_LAN2)
		phy_ports_mask |= (1L << LAN_PORT_2);
	if (user_port_mask & RTL8367M_PORTMASK_LAN3)
		phy_ports_mask |= (1L << LAN_PORT_3);
	if (user_port_mask & RTL8367M_PORTMASK_LAN4)
		phy_ports_mask |= (1L << LAN_PORT_4);
	if (user_port_mask & RTL8367M_PORTMASK_WAN)
		phy_ports_mask |= (1L << WAN_PORT_X);
	if (user_port_mask & RTL8367M_PORTMASK_CPU_LAN)
		phy_ports_mask |= (1L << LAN_PORT_CPU);
#if !defined(RTL8367M_SINGLE_EXTIF)
	if (user_port_mask & RTL8367M_PORTMASK_CPU_WAN)
		phy_ports_mask |= (1L << WAN_PORT_CPU);
#endif
	return phy_ports_mask;
}

unsigned int get_phy_ports_mask_lan(u32 include_cpu)
{
	unsigned int portmask_lan = ((1L << LAN_PORT_4) | (1L << LAN_PORT_3) | (1L << LAN_PORT_2) | (1L << LAN_PORT_1));
	if (include_cpu)
		portmask_lan |= (1L << LAN_PORT_CPU);

	switch (g_wan_bridge_mode)
	{
	case RTL8367M_WAN_BRIDGE_LAN1:
		portmask_lan &= ~(1L << LAN_PORT_1);
		break;
	case RTL8367M_WAN_BRIDGE_LAN2:
		portmask_lan &= ~(1L << LAN_PORT_2);
		break;
	case RTL8367M_WAN_BRIDGE_LAN3:
		portmask_lan &= ~(1L << LAN_PORT_3);
		break;
	case RTL8367M_WAN_BRIDGE_LAN4:
		portmask_lan &= ~(1L << LAN_PORT_4);
		break;
	case RTL8367M_WAN_BRIDGE_LAN3_LAN4:
		portmask_lan &= ~((1L << LAN_PORT_3) | (1L << LAN_PORT_4));
		break;
	case RTL8367M_WAN_BRIDGE_LAN1_LAN2:
		portmask_lan &= ~((1L << LAN_PORT_1) | (1L << LAN_PORT_2));
		break;
	case RTL8367M_WAN_BRIDGE_LAN1_LAN2_LAN3:
		portmask_lan &= ~((1L << LAN_PORT_1) | (1L << LAN_PORT_2) | (1L << LAN_PORT_3));
		break;
	case RTL8367M_WAN_BRIDGE_DISABLE_WAN:
		portmask_lan |= (1L << WAN_PORT_X);
		if (include_cpu)
			portmask_lan |= (1L << WAN_PORT_CPU);
		break;
	}

	return portmask_lan;
}

unsigned int get_phy_ports_mask_wan(u32 include_cpu)
{
	unsigned int portmask_wan = (1L << WAN_PORT_X);
	if (include_cpu)
		portmask_wan |= (1L << WAN_PORT_CPU);

	switch (g_wan_bridge_mode)
	{
	case RTL8367M_WAN_BRIDGE_LAN1:
		portmask_wan |= (1L << LAN_PORT_1);
		break;
	case RTL8367M_WAN_BRIDGE_LAN2:
		portmask_wan |= (1L << LAN_PORT_2);
		break;
	case RTL8367M_WAN_BRIDGE_LAN3:
		portmask_wan |= (1L << LAN_PORT_3);
		break;
	case RTL8367M_WAN_BRIDGE_LAN4:
		portmask_wan |= (1L << LAN_PORT_4);
		break;
	case RTL8367M_WAN_BRIDGE_LAN3_LAN4:
		portmask_wan |= ((1L << LAN_PORT_3) | (1L << LAN_PORT_4));
		break;
	case RTL8367M_WAN_BRIDGE_LAN1_LAN2:
		portmask_wan |= ((1L << LAN_PORT_1) | (1L << LAN_PORT_2));
		break;
	case RTL8367M_WAN_BRIDGE_LAN1_LAN2_LAN3:
		portmask_wan |= ((1L << LAN_PORT_1) | (1L << LAN_PORT_2) | (1L << LAN_PORT_3));
		break;
	case RTL8367M_WAN_BRIDGE_DISABLE_WAN:
		portmask_wan = 0;
		break;
	}

	return portmask_wan;
}

void asic_bridge_isolate(u32 wan_bridge_mode, u32 bwan_isolated_mode)
{
	int i;
	u32 fwd_mask_bwan_lan;
	rtk_portmask_t fwd_mask_lan, fwd_mask_wan, fwd_mask;

	switch (wan_bridge_mode)
	{
	case RTL8367M_WAN_BRIDGE_LAN1:
		printk("%s - hw bridge: WWLLL\n", RTL8367M_DEVNAME);
		break;
	case RTL8367M_WAN_BRIDGE_LAN2:
		printk("%s - hw bridge: WLWLL\n", RTL8367M_DEVNAME);
		break;
	case RTL8367M_WAN_BRIDGE_LAN3:
		printk("%s - hw bridge: WLLWL\n", RTL8367M_DEVNAME);
		break;
	case RTL8367M_WAN_BRIDGE_LAN4:
		printk("%s - hw bridge: WLLLW\n", RTL8367M_DEVNAME);
		break;
	case RTL8367M_WAN_BRIDGE_LAN3_LAN4:
		printk("%s - hw bridge: WLLWW\n", RTL8367M_DEVNAME);
		break;
	case RTL8367M_WAN_BRIDGE_LAN1_LAN2:
		printk("%s - hw bridge: WWWLL\n", RTL8367M_DEVNAME);
		break;
	case RTL8367M_WAN_BRIDGE_LAN1_LAN2_LAN3:
		printk("%s - hw bridge: WWWWL\n", RTL8367M_DEVNAME);
		break;
	case RTL8367M_WAN_BRIDGE_DISABLE_WAN:
		printk("%s - hw bridge: LLLLL\n", RTL8367M_DEVNAME);
		break;
	default:
		printk("%s - hw bridge: WLLLL\n", RTL8367M_DEVNAME);
		break;
	}

	fwd_mask_lan.bits[0] = get_phy_ports_mask_lan(1);

	/* LAN (efid=0) */
	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
	{
		if ((fwd_mask_lan.bits[0] >> i) & 0x1)
		{
			rtk_port_isolation_set(i, fwd_mask_lan);
#if !defined(RTL8367M_SINGLE_EXTIF)
			rtk_port_efid_set(i, 0);
#endif
		}
	}

	if (wan_bridge_mode == RTL8367M_WAN_BRIDGE_DISABLE_WAN)
	{
		return;
	}

	fwd_mask_wan.bits[0] = get_phy_ports_mask_wan(1);
	fwd_mask_bwan_lan = fwd_mask_wan.bits[0] & ~((1L << WAN_PORT_X) | (1L << WAN_PORT_CPU));

	/* WAN (efid=1) */
	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
	{
		if ((fwd_mask_wan.bits[0] >> i) & 0x1)
		{
			fwd_mask.bits[0] = fwd_mask_wan.bits[0];

#if defined(RTL8367M_SINGLE_EXTIF)
			switch(i)
			{
			case WAN_PORT_CPU:
				/* force add all LAN ports to forward from CPU */
				fwd_mask.bits[0] |= ((1L << LAN_PORT_4) | (1L << LAN_PORT_3) | (1L << LAN_PORT_2) | (1L << LAN_PORT_1));
				break;
			}
#endif
			if (bwan_isolated_mode == RTL8367M_WAN_BWAN_ISOLATION_FROM_CPU)
			{
				switch(i)
				{
				case WAN_PORT_CPU:
					fwd_mask.bits[0] &= ~(fwd_mask_bwan_lan);
					break;
				case LAN_PORT_4:
				case LAN_PORT_3:
				case LAN_PORT_2:
				case LAN_PORT_1:
					fwd_mask.bits[0] &= ~(1L << WAN_PORT_CPU);
					break;
				}
			}
			else if (bwan_isolated_mode == RTL8367M_WAN_BWAN_ISOLATION_BETWEEN)
			{
				switch(i)
				{
				case WAN_PORT_X:
					fwd_mask.bits[0] &= ~((1L << LAN_PORT_4) | (1L << LAN_PORT_3) | (1L << LAN_PORT_2) | (1L << LAN_PORT_1));
					break;
				case LAN_PORT_4:
					fwd_mask.bits[0] &= ~((1L << WAN_PORT_X) | (1L << LAN_PORT_3) | (1L << LAN_PORT_2) | (1L << LAN_PORT_1));
					break;
				case LAN_PORT_3:
					fwd_mask.bits[0] &= ~((1L << WAN_PORT_X) | (1L << LAN_PORT_4) | (1L << LAN_PORT_2) | (1L << LAN_PORT_1));
					break;
				case LAN_PORT_2:
					fwd_mask.bits[0] &= ~((1L << WAN_PORT_X) | (1L << LAN_PORT_4) | (1L << LAN_PORT_3) | (1L << LAN_PORT_1));
					break;
				case LAN_PORT_1:
					fwd_mask.bits[0] &= ~((1L << WAN_PORT_X) | (1L << LAN_PORT_4) | (1L << LAN_PORT_3) | (1L << LAN_PORT_2));
					break;
				}
			}

			rtk_port_isolation_set(i, fwd_mask);
#if !defined(RTL8367M_SINGLE_EXTIF)
			rtk_port_efid_set(i, 1);
#endif
		}
	}
}

void asic_vlan_ingress_mode(u32 ingress_enabled)
{
	u32 reg_ingress;

	if (ingress_enabled)
	{
		/* enable VLAN ingress filtering for each port */
		reg_ingress = ((1L << WAN_PORT_X) | (1L << LAN_PORT_1) | (1L << LAN_PORT_2) | (1L << LAN_PORT_3) | (1L << LAN_PORT_4));
		reg_ingress |= (1L << LAN_PORT_CPU);
		reg_ingress |= (1L << WAN_PORT_CPU);
	}
	else
	{
		/* disable VLAN ingress filtering for all ports */
		reg_ingress = 0;
	}

	rtl8370_setAsicReg(RTL8370_REG_VLAN_INGRESS, reg_ingress);
}

void asic_vlan_accept_port_mode(u32 accept_mode, u32 port_mask)
{
	int i;
	rtk_vlan_acceptFrameType_t acceptFrameType = ACCEPT_FRAME_TYPE_ALL;

	switch (accept_mode)
	{
	case RTL8367M_VLAN_ACCEPT_FRAMES_UNTAG_ONLY:
		acceptFrameType = ACCEPT_FRAME_TYPE_UNTAG_ONLY;
		break;
	case RTL8367M_VLAN_ACCEPT_FRAMES_TAG_ONLY:
		acceptFrameType = ACCEPT_FRAME_TYPE_TAG_ONLY;
		break;
	}

	port_mask = get_phy_ports_mask_from_user(port_mask);

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
	{
		if ((port_mask >> i) & 0x1)
			rtk_vlan_portAcceptFrameType_set(i, acceptFrameType);
	}
}

void asic_vlan_create_port_vid(u32 vlan4k_info, u32 vlan4k_mask)
{
	int i;
	rtk_vlan_t pvid;
	rtk_pri_t prio;
	rtk_fid_t fid;
	rtk_portmask_t mask_member, mask_untag;

	pvid = (rtk_vlan_t)(vlan4k_info & 0x0FFF);
	prio = (rtk_pri_t)((vlan4k_info >> 12) & 0x7);
	fid  = (rtk_fid_t)((vlan4k_info >> 16) & 0xFF);
	mask_member.bits[0] = get_phy_ports_mask_from_user((vlan4k_mask & 0x7F));
	mask_untag.bits[0]  = get_phy_ports_mask_from_user((vlan4k_mask >> 16) & 0x7F);

	rtk_vlan_set(pvid, mask_member, mask_untag, fid);

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
	{
		if ((mask_untag.bits[0] >> i) & 0x1)
			rtk_vlan_portPvid_set(i, pvid, prio);
	}

	printk("%s - create vlan: pvid=[%d], prio=[%d], member=[0x%04X], untag=[0x%04X], fid=[%d]\n",
			RTL8367M_DEVNAME, pvid, prio, mask_member.bits[0], mask_untag.bits[0], fid);
}

void asic_bridge_isolate_vlan(u32 wan_bridge_mode, int force_reset_vlan)
{
#if defined(RTL8367M_SINGLE_EXTIF)
	int i;
	rtk_portmask_t mask_member, mask_untag;
#endif
	if (force_reset_vlan)
	{
		/* init VLAN table (VLAN1) and enable VLAN */
		rtk_vlan_init();
	}

#if defined(RTL8367M_SINGLE_EXTIF)
	if (wan_bridge_mode == RTL8367M_WAN_BRIDGE_DISABLE_WAN)
	{
		/* set CPU port accept mask (accept tag and untag in LLLLL mode) */
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_CPU, ACCEPT_FRAME_TYPE_ALL);

		/* set WAN port accept mask (accept tag and untag in LLLLL mode) */
		rtk_vlan_portAcceptFrameType_set(WAN_PORT_X, ACCEPT_FRAME_TYPE_ALL);

		/* set LAN ports accept mask (accept tag and untag in LLLLL mode) */
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_4, ACCEPT_FRAME_TYPE_ALL);
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_3, ACCEPT_FRAME_TYPE_ALL);
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_2, ACCEPT_FRAME_TYPE_ALL);
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_1, ACCEPT_FRAME_TYPE_ALL);

		/* disable ingress filtering */
		asic_vlan_ingress_mode(0);
	}
	else
	{
		/* VLAN1 for LAN */
		mask_member.bits[0] = get_phy_ports_mask_lan(1);
		mask_untag.bits[0]  = mask_member.bits[0] & ~(1 << LAN_PORT_CPU);
		rtk_vlan_set(RTL8367M_VLANID_LAN, mask_member, mask_untag, 0);

		for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
		{
			if ((mask_untag.bits[0] >> i) & 0x1)
				rtk_vlan_portPvid_set(i, RTL8367M_VLANID_LAN, 0);
		}

		/* VLAN2 for WAN */
		mask_member.bits[0] = get_phy_ports_mask_wan(1);
		mask_untag.bits[0]  = mask_member.bits[0] & ~(1 << LAN_PORT_CPU);
		rtk_vlan_set(RTL8367M_VLANID_WAN, mask_member, mask_untag, 1);

		for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
		{
			if ((mask_untag.bits[0] >> i) & 0x1)
				rtk_vlan_portPvid_set(i, RTL8367M_VLANID_WAN, 0);
		}

		/* set CPU port accept mask (trunk port - accept tag only) */
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_CPU, ACCEPT_FRAME_TYPE_TAG_ONLY);

		/* set WAN port accept mask (accept untag only, it safe for VLAN1/VLAN2) */
		rtk_vlan_portAcceptFrameType_set(WAN_PORT_X, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

		/* set LAN ports accept mask (accept untag only) */
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_4, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_3, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_2, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_1, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

		/* enable ingress filtering */
		asic_vlan_ingress_mode(1);
	}
#else
	if (force_reset_vlan)
	{
		/* set CPU LAN port accept mask (accept untug only) */
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_CPU, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

		/* set CPU WAN port accept mask (accept untug only) */
		rtk_vlan_portAcceptFrameType_set(WAN_PORT_CPU, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

		/* set WAN port accept mask (accept untug only) */
		rtk_vlan_portAcceptFrameType_set(WAN_PORT_X, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

		/* set LAN ports accept mask (accept untag only) */
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_4, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_3, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_2, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_1, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

		/* disable ingress filtering */
		asic_vlan_ingress_mode(0);
	}
#endif
}

void asic_vlan_reset_table(void)
{
	/* reset VLAN table and set VLAN isolation for bridge */
	asic_bridge_isolate_vlan(g_wan_bridge_mode, 1);
}

void asic_led_mode(rtk_led_group_t group, u32 led_mode)
{
	rtk_led_congig_t led_config_group = LED_CONFIG_SPD1000ACT;

	switch (led_mode)
	{
	case RTL8367M_LED_PHYMODE_1000_ACT:
		led_config_group = LED_CONFIG_SPD1000ACT;
		break;
	case RTL8367M_LED_PHYMODE_100_ACT:
		led_config_group = LED_CONFIG_SPD100ACT;
		break;
	case RTL8367M_LED_PHYMODE_10_ACT:
		led_config_group = LED_CONFIG_SPD10ACT;
		break;
	case RTL8367M_LED_PHYMODE_100_10_ACT:
		led_config_group = LED_CONFIG_SPD10010ACT;
		break;
	case RTL8367M_LED_PHYMODE_1000:
		led_config_group = LED_CONFIG_SPD1000;
		break;
	case RTL8367M_LED_PHYMODE_100:
		led_config_group = LED_CONFIG_SPD100;
		break;
	case RTL8367M_LED_PHYMODE_10:
		led_config_group = LED_CONFIG_SPD10;
		break;
	case RTL8367M_LED_LINK_ACT:
		led_config_group = LED_CONFIG_LINK_ACT;
		break;
	case RTL8367M_LED_LINK_ACT_RX:
		led_config_group = LED_CONFIG_LINKRX;
		break;
	case RTL8367M_LED_LINK_ACT_TX:
		led_config_group = LED_CONFIG_LINKTX;
		break;
	case RTL8367M_LED_DUPLEX_COLLISION:
		led_config_group = LED_CONFIG_DUPCOL;
		break;
	case RTL8367M_LED_OFF:
		led_config_group = LED_CONFIG_LEDOFF;
		break;
	}

	rtk_led_groupConfig_set(group, led_config_group);
}

rtk_api_ret_t asic_status_link_port(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus)
{
	u32 regData = 0;
	rtk_api_ret_t retVal = rtl8370_getAsicReg(RTL8370_REG_PORT0_STATUS + port, &regData);
	if (retVal != RT_ERR_OK)
		return retVal;

	if (regData & RTL8370_PORT0_STATUS_LINK_STATE_MASK)
	{
		*pLinkStatus = 1;
	}
	else
	{
		*pLinkStatus = 0;
	}

	return RT_ERR_OK;
}

rtk_api_ret_t asic_status_speed_port(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus, rtk_data_t *pSpeed, rtk_data_t *pDuplex)
{
	u32 regData = 0;
	rtk_api_ret_t retVal = rtl8370_getAsicReg(RTL8370_REG_PORT0_STATUS + port, &regData);
	if (retVal != RT_ERR_OK)
		return retVal;

	if (regData & RTL8370_PORT0_STATUS_LINK_STATE_MASK)
	{
		*pLinkStatus = 1;
		*pSpeed  = (regData & RTL8370_PORT1_STATUS_LINK_SPEED_MASK);
		*pDuplex = (regData & RTL8370_PORT0_STATUS_FULL_DUPLUX_CAP_MASK) ? 1 : 0;
	}
	else
	{
		*pLinkStatus = 0;
		*pSpeed = 0;
		*pDuplex = 0;
	}

	return RT_ERR_OK;
}

rtk_api_ret_t asic_status_link_ports_lan(rtk_port_linkStatus_t *pLinkStatus)
{
	int i;
	rtk_api_ret_t retVal;
	unsigned int portmask_lan = get_phy_ports_mask_lan(0);

	for (i = 0; i <= RTK_PHY_ID_MAX; i++)
	{
		if ((portmask_lan >> i) & 0x1)
		{
			retVal = asic_status_link_port(i, pLinkStatus);
			if (retVal != RT_ERR_OK)
				return retVal;

			if (*pLinkStatus)
				break;
		}
	}

	return RT_ERR_OK;
}

rtk_api_ret_t asic_status_link_ports_wan(rtk_port_linkStatus_t *pLinkStatus)
{
	int i;
	rtk_api_ret_t retVal;
	unsigned int portmask_wan = get_phy_ports_mask_wan(0);

	// check WAN first
	for (i = RTK_PHY_ID_MAX; i>=0; i--)
	{
		if ((portmask_wan >> i) & 0x1)
		{
			retVal = asic_status_link_port(i, pLinkStatus);
			if (retVal != RT_ERR_OK)
				return retVal;

			if (*pLinkStatus)
				break;
		}
	}

	return RT_ERR_OK;
}

int change_bridge_mode(u32 wan_bridge_mode, u32 isolated_mode)
{
	int bridge_changed;

	if (wan_bridge_mode > RTL8367M_WAN_BRIDGE_DISABLE_WAN)
		return -EINVAL;

	if (isolated_mode > RTL8367M_WAN_BWAN_ISOLATION_BETWEEN)
		return -EINVAL;

	bridge_changed = (g_wan_bridge_mode != wan_bridge_mode) ? 1 : 0;

	if (bridge_changed || g_wan_bridge_isolated_mode != isolated_mode)
	{
		// set global bridge_mode first
		g_wan_bridge_mode = wan_bridge_mode;
		g_wan_bridge_isolated_mode = isolated_mode;

		asic_bridge_isolate(wan_bridge_mode, isolated_mode);
	}

#if defined(RTL8367M_SINGLE_EXTIF)
	if (bridge_changed)
	{
		/* set VLAN isolation for bridge */
		asic_bridge_isolate_vlan(wan_bridge_mode, (wan_bridge_mode == RTL8367M_WAN_BRIDGE_DISABLE_WAN) ? 1 : 0);
	}
#endif

	return 0;
}

int change_led_mode_group0(u32 led_mode, int force_change)
{
	if (led_mode > RTL8367M_LED_OFF)
		return -EINVAL;

	if (g_led_phy_mode_group0 != led_mode || force_change)
	{
		asic_led_mode(LED_GROUP_0, led_mode);

		g_led_phy_mode_group0 = led_mode;
	}

	return 0;
}

int change_led_mode_group1(u32 led_mode, int force_change)
{
	if (led_mode > RTL8367M_LED_OFF)
		return -EINVAL;

	if (g_led_phy_mode_group1 != led_mode || force_change)
	{
		asic_led_mode(LED_GROUP_1, led_mode);

		g_led_phy_mode_group1 = led_mode;
	}

	return 0;
}

int change_led_mode_group2(u32 led_mode, int force_change)
{
	if (led_mode > RTL8367M_LED_OFF)
		return -EINVAL;

	if (g_led_phy_mode_group2 != led_mode || force_change)
	{
		asic_led_mode(LED_GROUP_2, led_mode);

		g_led_phy_mode_group2 = led_mode;
	}

	return 0;
}

void change_port_link_mode(rtk_port_t port, u32 port_link_mode, int force_change)
{
	u32 i_port_speed;
	u32 i_port_flowc;
	rtk_api_ret_t retVal;
	rtk_port_phy_ability_t phy_cfg;

	if (g_port_link_mode[port] == port_link_mode && !force_change)
		return;

	i_port_speed =  (port_link_mode & 0x07);
	i_port_flowc = ((port_link_mode >> 8) & 0x03);

	printk("%s - port [%d] link speed: %d, flow control: %d\n", RTL8367M_DEVNAME, port, i_port_speed, i_port_flowc);

	phy_cfg.FC	 = 1; //  Symmetric Flow Control
	phy_cfg.AsyFC	 = 0; // Asymmetric Flow Control (only for 1Gbps)

	switch (i_port_flowc)
	{
	case RTL8367M_LINK_FLOW_CONTROL_RX_ASYNC:
		phy_cfg.FC	 = (i_port_speed != 1) ? 1 : 0;
		phy_cfg.AsyFC	 = 1;
		break;
	case RTL8367M_LINK_FLOW_CONTROL_DISABLE:
		phy_cfg.FC	 = 0;
		phy_cfg.AsyFC	 = 0;
		break;
	}

	phy_cfg.AutoNegotiation	 = 1;
	phy_cfg.Full_1000	 = 1;
	phy_cfg.Full_100	 = 1;
	phy_cfg.Half_100	 = 1;
	phy_cfg.Full_10		 = 1;
	phy_cfg.Half_10		 = 1;

	switch (i_port_speed)
	{
	case RTL8367M_LINK_SPEED_MODE_1000_FD:
		phy_cfg.Full_100  = 0;
		phy_cfg.Half_100  = 0;
		phy_cfg.Full_10   = 0;
		phy_cfg.Half_10   = 0;
		break;
	case RTL8367M_LINK_SPEED_MODE_100_FD:
		phy_cfg.Full_1000 = 0;
		phy_cfg.Half_100  = 0;
		phy_cfg.Full_10   = 0;
		phy_cfg.Half_10   = 0;
		break;
	case RTL8367M_LINK_SPEED_MODE_100_HD:
		phy_cfg.Full_1000 = 0;
		phy_cfg.Full_100  = 0;
		phy_cfg.Full_10   = 0;
		phy_cfg.Half_10   = 0;
		break;
	case RTL8367M_LINK_SPEED_MODE_10_FD:
		phy_cfg.Full_1000 = 0;
		phy_cfg.Full_100  = 0;
		phy_cfg.Half_100  = 0;
		phy_cfg.Half_10   = 0;
		break;
	case RTL8367M_LINK_SPEED_MODE_10_HD:
		phy_cfg.Full_1000 = 0;
		phy_cfg.Full_100  = 0;
		phy_cfg.Half_100  = 0;
		phy_cfg.Full_10   = 0;
		break;
	}

	retVal = rtk_port_phyAutoNegoAbility_set(port, &phy_cfg);
	if (retVal == RT_ERR_OK)
		g_port_link_mode[port] = port_link_mode;
}

void change_jumbo_frames_accept(u32 jumbo_frames_enabled, int force_change)
{
	if (jumbo_frames_enabled) jumbo_frames_enabled = 1;

	if (g_jumbo_frames_enabled != jumbo_frames_enabled || force_change)
	{
		printk("%s - jumbo frames accept: %s bytes\n", RTL8367M_DEVNAME, (jumbo_frames_enabled) ? "16000" : "1536");

		rtk_switch_maxPktLen_set( (jumbo_frames_enabled) ? MAXPKTLEN_16000B : MAXPKTLEN_1536B );

		g_jumbo_frames_enabled = jumbo_frames_enabled;
	}
}

void change_green_ethernet_mode(u32 green_ethernet_enabled, int force_change)
{
	if (green_ethernet_enabled) green_ethernet_enabled = 1;

	if (g_green_ethernet_enabled != green_ethernet_enabled || force_change)
	{
		printk("%s - green ethernet: %d\n", RTL8367M_DEVNAME, green_ethernet_enabled);

		rtk_switch_greenEthernet_set(green_ethernet_enabled);

		g_green_ethernet_enabled = green_ethernet_enabled;
	}
}

int change_storm_control_unicast_unknown(u32 control_rate_mbps, int force_change)
{
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_unicast_unknown != control_rate_mbps || force_change)
	{
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > RTK_MAX_INPUT_RATE) rate_kbps = RTK_MAX_INPUT_RATE;

		printk("%s - set unknown unicast storm control rate as: %d kbps\n", RTL8367M_DEVNAME, rate_kbps);

		rtk_storm_controlRate_set(LAN_PORT_4, STORM_GROUP_UNKNOWN_UNICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_3, STORM_GROUP_UNKNOWN_UNICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_2, STORM_GROUP_UNKNOWN_UNICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_1, STORM_GROUP_UNKNOWN_UNICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(WAN_PORT_X, STORM_GROUP_UNKNOWN_UNICAST, rate_kbps, 1, MODE0);

		g_storm_rate_unicast_unknown = control_rate_mbps;
	}

	return 0;
}

int change_storm_control_multicast_unknown(u32 control_rate_mbps, int force_change)
{
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_multicast_unknown != control_rate_mbps || force_change)
	{
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > RTK_MAX_INPUT_RATE) rate_kbps = RTK_MAX_INPUT_RATE;

		printk("%s - set unknown multicast storm control rate as: %d kbps\n", RTL8367M_DEVNAME, rate_kbps);

		rtk_storm_controlRate_set(LAN_PORT_4, STORM_GROUP_UNKNOWN_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_3, STORM_GROUP_UNKNOWN_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_2, STORM_GROUP_UNKNOWN_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_1, STORM_GROUP_UNKNOWN_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(WAN_PORT_X, STORM_GROUP_UNKNOWN_MULTICAST, rate_kbps, 1, MODE0);

		g_storm_rate_multicast_unknown = control_rate_mbps;
	}

	return 0;
}

int change_storm_control_multicast(u32 control_rate_mbps, int force_change)
{
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_multicast != control_rate_mbps || force_change)
	{
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > RTK_MAX_INPUT_RATE) rate_kbps = RTK_MAX_INPUT_RATE;

		printk("%s - set multicast storm control rate as: %d kbps\n", RTL8367M_DEVNAME, rate_kbps);

		rtk_storm_controlRate_set(LAN_PORT_4, STORM_GROUP_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_3, STORM_GROUP_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_2, STORM_GROUP_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_1, STORM_GROUP_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(WAN_PORT_X, STORM_GROUP_MULTICAST, rate_kbps, 1, MODE0);

		g_storm_rate_multicast = control_rate_mbps;
	}

	return 0;
}

int change_storm_control_broadcast(u32 control_rate_mbps, int force_change)
{
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_broadcast != control_rate_mbps || force_change)
	{
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > RTK_MAX_INPUT_RATE) rate_kbps = RTK_MAX_INPUT_RATE;

		printk("%s - set broadcast storm control rate as: %d kbps\n", RTL8367M_DEVNAME, rate_kbps);

		rtk_storm_controlRate_set(LAN_PORT_4, STORM_GROUP_BROADCAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_3, STORM_GROUP_BROADCAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_2, STORM_GROUP_BROADCAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_1, STORM_GROUP_BROADCAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(WAN_PORT_X, STORM_GROUP_BROADCAST, rate_kbps, 1, MODE0);

		g_storm_rate_broadcast = control_rate_mbps;
	}

	return 0;
}

int change_cpu_rgmii_delay_tx(u32 rgmii_delay_tx, int force_change)
{
	if (rgmii_delay_tx > 1)
		return -EINVAL;

	if (g_rgmii_delay_tx != rgmii_delay_tx || force_change)
	{
		printk("%s - set cpu rgmii delay tx: %d\n", RTL8367M_DEVNAME, rgmii_delay_tx);

#if !defined(RTL8367M_SINGLE_EXTIF)
		rtk_port_rgmiiDelayExt0_set(rgmii_delay_tx, g_rgmii_delay_rx);
#endif
		rtk_port_rgmiiDelayExt1_set(rgmii_delay_tx, g_rgmii_delay_rx);

		g_rgmii_delay_tx = rgmii_delay_tx;
	}

	return 0;
}

int change_cpu_rgmii_delay_rx(u32 rgmii_delay_rx, int force_change)
{
	if (rgmii_delay_rx > 7)
		return -EINVAL;

	if (g_rgmii_delay_rx != rgmii_delay_rx || force_change)
	{
		printk("%s - set cpu rgmii delay rx: %d\n", RTL8367M_DEVNAME, rgmii_delay_rx);

#if !defined(RTL8367M_SINGLE_EXTIF)
		rtk_port_rgmiiDelayExt0_set(g_rgmii_delay_tx, rgmii_delay_rx);
#endif
		rtk_port_rgmiiDelayExt1_set(g_rgmii_delay_tx, rgmii_delay_rx);

		g_rgmii_delay_rx = rgmii_delay_rx;
	}

	return 0;
}

void reset_params_default(void)
{
	int i;
	for (i=0; i <= RTK_PHY_ID_MAX; i++)
		g_port_link_mode[i] = RTL8367M_DEFAULT_LINK_MODE;

	g_storm_rate_unicast_unknown    = RTL8367M_DEFAULT_STORM_RATE;
	g_storm_rate_multicast_unknown  = RTL8367M_DEFAULT_STORM_RATE;
	g_storm_rate_multicast          = RTL8367M_DEFAULT_STORM_RATE;
	g_storm_rate_broadcast          = RTL8367M_DEFAULT_STORM_RATE;

	g_green_ethernet_enabled        = RTL8367M_DEFAULT_GREEN_ETHERNET;
	g_jumbo_frames_enabled          = RTL8367M_DEFAULT_JUMBO_FRAMES;

	g_led_phy_mode_group0           = RTL8367M_LED_PHYMODE_100_10_ACT;
	g_led_phy_mode_group1           = RTL8367M_LED_PHYMODE_1000_ACT;
	g_led_phy_mode_group2           = RTL8367M_LED_OFF;

	g_rgmii_delay_tx                = CONFIG_RTL8367M_RGMII_DELAY_TX;
	g_rgmii_delay_rx                = CONFIG_RTL8367M_RGMII_DELAY_RX;
}

void reset_and_init_switch(int first_call)
{
	rtk_api_ret_t retVal;
	rtk_portmask_t portmask;
	rtk_port_mac_ability_t mac_cfg;
	u32 ports_mask_wan, ports_mask_lan;

	if (!first_call)
		printk("%s - perform software reset asic!\n", RTL8367M_DEVNAME);

	reset_params_default();

	ports_mask_wan = get_phy_ports_mask_wan(0);
	ports_mask_lan = get_phy_ports_mask_lan(0);

	retVal = rtk_switch_init();
	if (retVal != RT_ERR_OK)
		printk("rtk_switch_init() FAILED! (code %d)\n", retVal);

	/* configure ExtIf, RGMII fixed mode w/o autoneg */
	mac_cfg.speed		= SPD_1000M;
	mac_cfg.forcemode	= MAC_FORCE;
	mac_cfg.duplex		= FULL_DUPLEX;
	mac_cfg.link		= 1;
	mac_cfg.nway		= 0;
	mac_cfg.rxpause		= 1;
	mac_cfg.txpause		= 1;
#if !defined(RTL8367M_SINGLE_EXTIF)
	rtk_port_macForceLinkExt0_set(MODE_EXT_RGMII, &mac_cfg);
#endif
	rtk_port_macForceLinkExt1_set(MODE_EXT_RGMII, &mac_cfg);

	/* configure ExtIf TX and RX delay  */
#if !defined(RTL8367M_SINGLE_EXTIF)
	rtk_port_rgmiiDelayExt0_set(g_rgmii_delay_tx, g_rgmii_delay_rx);
#endif
	rtk_port_rgmiiDelayExt1_set(g_rgmii_delay_tx, g_rgmii_delay_rx);

	/* configure bridge isolation mode */
	asic_bridge_isolate(g_wan_bridge_mode, g_wan_bridge_isolated_mode);

	/* configure bridge isolation mode via VLAN */
	asic_bridge_isolate_vlan(g_wan_bridge_mode, 1);

	/* configure leds */
	portmask.bits[0] = ports_mask_wan | ports_mask_lan;
	rtk_led_enable_set(LED_GROUP_0, portmask);
	rtk_led_enable_set(LED_GROUP_1, portmask);
	rtk_led_enable_set(LED_GROUP_2, portmask);
	rtk_led_operation_set(LED_OP_PARALLEL);
	asic_led_mode(LED_GROUP_0, g_led_phy_mode_group0);	// group 0 - 8P8C usually green LED
	asic_led_mode(LED_GROUP_1, g_led_phy_mode_group1);	// group 1 - 8P8C usually yellow LED
	asic_led_mode(LED_GROUP_2, g_led_phy_mode_group2);	// group 2 - 8P8C usually duplex/collision LED
}

////////////////////////////////////////////////////////////////////////////////////

static long rtl8367m_ioctl(struct file *file, unsigned int req, unsigned long arg)
{
	int ioctl_result = 0;
	u32 uint_value = 0;
	u32 uint_result = 0;

	rtk_api_ret_t         retVal;
	rtk_port_linkStatus_t port_link = 0;
	rtk_data_t            port_speed = 0;
	rtk_data_t            port_duplex = 0;
	rtk_stat_port_cntr_t  port_counters;

	unsigned int uint_param = (req >> RTL8367M_IOCTL_CMD_LENGTH_BITS);
	req &= ((1L << RTL8367M_IOCTL_CMD_LENGTH_BITS)-1);

	mutex_lock(&asic_access_mutex);

	switch(req)
	{
	case RTL8367M_IOCTL_GPIO_MODE_SET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		gpio_set_mode(uint_value);
		break;
	case RTL8367M_IOCTL_GPIO_MODE_SET_BIT:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = gpio_set_mode_bit(uint_param, uint_value);
		break;
	case RTL8367M_IOCTL_GPIO_MODE_GET:
		gpio_get_mode(&uint_result);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case RTL8367M_IOCTL_GPIO_PIN_SET_DIRECTION:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = gpio_set_pin_direction(uint_param, uint_value);
		break;
	case RTL8367M_IOCTL_GPIO_PIN_SET_VAL:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = gpio_set_pin_value(uint_param, uint_value);
		break;
	case RTL8367M_IOCTL_GPIO_PIN_GET_VAL:
		ioctl_result = gpio_get_pin_value(uint_param, &uint_result);
		if (ioctl_result == 0)
			put_user(uint_result, (unsigned int __user *)arg);
		break;

	case RTL8367M_IOCTL_STATUS_LINK_PORT_WAN:
		retVal = asic_status_link_port(WAN_PORT_X, &port_link);
		if (retVal == RT_ERR_OK)
			put_user(port_link, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_LINK_PORTS_WAN:
		retVal = asic_status_link_ports_wan(&port_link);
		if (retVal == RT_ERR_OK)
			put_user(port_link, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_LINK_PORTS_LAN:
		retVal = asic_status_link_ports_lan(&port_link);
		if (retVal == RT_ERR_OK)
			put_user(port_link, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_SPEED_PORT_WAN:
		retVal = asic_status_speed_port(WAN_PORT_X, &port_link, &port_speed, &port_duplex);
		port_speed |= (port_duplex << 8);
		port_speed |= (port_link << 16);
		if (retVal == RT_ERR_OK)
			put_user(port_speed, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN1:
		retVal = asic_status_speed_port(LAN_PORT_1, &port_link, &port_speed, &port_duplex);
		port_speed |= (port_duplex << 8);
		port_speed |= (port_link << 16);
		if (retVal == RT_ERR_OK)
			put_user(port_speed, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN2:
		retVal = asic_status_speed_port(LAN_PORT_2, &port_link, &port_speed, &port_duplex);
		port_speed |= (port_duplex << 8);
		port_speed |= (port_link << 16);
		if (retVal == RT_ERR_OK)
			put_user(port_speed, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN3:
		retVal = asic_status_speed_port(LAN_PORT_3, &port_link, &port_speed, &port_duplex);
		port_speed |= (port_duplex << 8);
		port_speed |= (port_link << 16);
		if (retVal == RT_ERR_OK)
			put_user(port_speed, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_SPEED_PORT_LAN4:
		retVal = asic_status_speed_port(LAN_PORT_4, &port_link, &port_speed, &port_duplex);
		port_speed |= (port_duplex << 8);
		port_speed |= (port_link << 16);
		if (retVal == RT_ERR_OK)
			put_user(port_speed, (unsigned int __user *)arg);
		else
			ioctl_result = -EIO;
		break;

	case RTL8367M_IOCTL_STATUS_CNT_PORT_WAN:
		retVal = rtk_stat_port_getAll(WAN_PORT_X, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN1:
		retVal = rtk_stat_port_getAll(LAN_PORT_1, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN2:
		retVal = rtk_stat_port_getAll(LAN_PORT_2, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN3:
		retVal = rtk_stat_port_getAll(LAN_PORT_3, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_CNT_PORT_LAN4:
		retVal = rtk_stat_port_getAll(LAN_PORT_4, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_CNT_PORT_CPU_WAN:
		retVal = rtk_stat_port_getAll(WAN_PORT_CPU, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_CNT_PORT_CPU_LAN:
		retVal = rtk_stat_port_getAll(LAN_PORT_CPU, &port_counters);
		if (retVal == RT_ERR_OK)
			copy_to_user((rtk_stat_port_cntr_t __user *)arg, &port_counters, sizeof(rtk_stat_port_cntr_t));
		else
			ioctl_result = -EIO;
		break;
	case RTL8367M_IOCTL_STATUS_CNT_RESET_ALL:
		rtk_stat_port_reset(WAN_PORT_CPU);
		rtk_stat_port_reset(LAN_PORT_CPU);
		rtk_stat_port_reset(WAN_PORT_X);
		rtk_stat_port_reset(LAN_PORT_1);
		rtk_stat_port_reset(LAN_PORT_2);
		rtk_stat_port_reset(LAN_PORT_3);
		rtk_stat_port_reset(LAN_PORT_4);
		break;

	case RTL8367M_IOCTL_RESET_ASIC:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		if (uint_value == RTL8367M_MAGIC_RESET_ASIC)
			reset_and_init_switch(0);
		break;

	case RTL8367M_IOCTL_BRIDGE_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_bridge_mode(uint_value, uint_param);
		break;

	case RTL8367M_IOCTL_VLAN_RESET_TABLE:
		asic_vlan_reset_table();
		break;
	case RTL8367M_IOCTL_VLAN_INGRESS_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_vlan_ingress_mode(uint_value);
		break;
	case RTL8367M_IOCTL_VLAN_ACCEPT_PORT_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_vlan_accept_port_mode(uint_param, uint_value);
		break;
	case RTL8367M_IOCTL_VLAN_CREATE_PORT_VID:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		asic_vlan_create_port_vid(uint_param, uint_value);
		break;

	case RTL8367M_IOCTL_STORM_UNICAST_UNK:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_unicast_unknown(uint_value, 0);
		break;
	case RTL8367M_IOCTL_STORM_MULTICAST_UNK:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_multicast_unknown(uint_value, 0);
		break;
	case RTL8367M_IOCTL_STORM_MULTICAST:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_multicast(uint_value, 0);
		break;
	case RTL8367M_IOCTL_STORM_BROADCAST:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_storm_control_broadcast(uint_value, 0);
		break;

	case RTL8367M_IOCTL_GREEN_ETHERNET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_green_ethernet_mode(uint_value, 0);
		break;

	case RTL8367M_IOCTL_JUMBO_FRAMES:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_jumbo_frames_accept(uint_value, 0);
		break;

	case RTL8367M_IOCTL_LED_MODE_GROUP0:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_led_mode_group0(uint_value, 0);
		break;
	case RTL8367M_IOCTL_LED_MODE_GROUP1:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_led_mode_group1(uint_value, 0);
		break;
	case RTL8367M_IOCTL_LED_MODE_GROUP2:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_led_mode_group2(uint_value, 0);
		break;

	case RTL8367M_IOCTL_SPEED_PORT_WAN:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(WAN_PORT_X, uint_value, 0);
		break;
	case RTL8367M_IOCTL_SPEED_PORT_LAN1:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_1, uint_value, 0);
		break;
	case RTL8367M_IOCTL_SPEED_PORT_LAN2:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_2, uint_value, 0);
		break;
	case RTL8367M_IOCTL_SPEED_PORT_LAN3:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_3, uint_value, 0);
		break;
	case RTL8367M_IOCTL_SPEED_PORT_LAN4:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_4, uint_value, 0);
		break;

	case RTL8367M_IOCTL_RGMII_DELAY_RX:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_cpu_rgmii_delay_rx(uint_value, 0);
		break;

	case RTL8367M_IOCTL_RGMII_DELAY_TX:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_cpu_rgmii_delay_tx(uint_value, 0);
		break;

	default:
		ioctl_result = -ENOIOCTLCMD;
	}

	mutex_unlock(&asic_access_mutex);

	return ioctl_result;
}

static int rtl8367m_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_INC_USE_COUNT;
#else
	try_module_get(THIS_MODULE);
#endif
	return 0;
}

static int rtl8367m_release(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_DEC_USE_COUNT;
#else
	module_put(THIS_MODULE);
#endif
	return 0;
}

static struct file_operations rtl8367m_fops =
{
	owner:		THIS_MODULE,
	unlocked_ioctl:	rtl8367m_ioctl,
	open:		rtl8367m_open,
	release:	rtl8367m_release,
};

int __init rtl8367m_init(void)
{
	int r;
	mutex_init(&asic_access_mutex);

	smi_init(SMI_RALINK_GPIO_SDA, SMI_RALINK_GPIO_SCK, SMI_RTL8367_DELAY_NS, SMI_RTL8367_SMI_ADDR);

	r = register_chrdev(RTL8367M_DEVMAJOR, RTL8367M_DEVNAME, &rtl8367m_fops);
	if (r < 0) {
		printk(KERN_ERR RTL8367M_DEVNAME ": unable to register character device\n");
		return r;
	}

	mutex_lock(&asic_access_mutex);
	reset_and_init_switch(1);
	mutex_unlock(&asic_access_mutex);

	printk("Realtek RTL8367M/R driver initialized\n");

	return 0;
}

void __exit rtl8367m_exit(void)
{
	unregister_chrdev(RTL8367M_DEVMAJOR, RTL8367M_DEVNAME);
}

module_init(rtl8367m_init);
module_exit(rtl8367m_exit);

MODULE_DESCRIPTION("Realtek RTL8367M/R GigaPHY Switch");
MODULE_LICENSE("GPL");
