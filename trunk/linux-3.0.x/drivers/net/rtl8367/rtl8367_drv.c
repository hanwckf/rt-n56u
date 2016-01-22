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
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/if_link.h>

#include "rtl8367_def.h"
#include "rtl8367_ioctl.h"

#if defined(CONFIG_RTL8367_CIF_SMI)
#include "ralink_smi.h"
#endif
#if defined(CONFIG_RTL8367_CIF_MDIO)
#include "ralink_mdio.h"
#endif

#if defined(CONFIG_RTL8367_API_8370)
#include "api_8370/rtk_types.h"
#include "api_8370/rtk_error.h"
#include "api_8370/rtk_api_ext.h"
#include "api_8370/rtl8370_asicdrv_port.h"
#include "api_8370/rtl8370_asicdrv_vlan.h"
#include "api_8370/rtl8370_asicdrv_green.h"
#include "api_8370/rtl8370_asicdrv_interrupt.h"
#include "api_8370/rtl8370_asicdrv_lut.h"
#define MAX_STORM_RATE_VAL			RTK_MAX_INPUT_RATE
#undef RTK_PHY_ID_MAX
#define RTK_PHY_ID_MAX				4	/* API 8370 used 0..7 ports, redefine to 0..4 */
#else
#include "api_8367b/rtk_types.h"
#include "api_8367b/rtk_error.h"
#include "api_8367b/rtk_api_ext.h"
#include "api_8367b/rtl8367b_asicdrv_port.h"
#include "api_8367b/rtl8367b_asicdrv_vlan.h"
#include "api_8367b/rtl8367b_asicdrv_green.h"
#include "api_8367b/rtl8367b_asicdrv_interrupt.h"
#include "api_8367b/rtl8367b_asicdrv_lut.h"
#define MAX_STORM_RATE_VAL			RTL8367B_QOS_RATE_INPUT_MAX
#endif

#if defined(CONFIG_P5_RGMII_TO_MAC_MODE) && defined(CONFIG_P4_RGMII_TO_MAC_MODE)
#include "../raeth/ra_esw_mt7620.h"
#endif

////////////////////////////////////////////////////////////////////////////////////

static DEFINE_MUTEX(asic_access_mutex);

static u32 g_wan_bridge_mode                     = SWAPI_WAN_BRIDGE_DISABLE;
static u32 g_wan_bridge_isolated_mode            = SWAPI_WAN_BWAN_ISOLATION_NONE;

static u32 g_led_phy_mode_group0                 = SWAPI_LED_PHYMODE_100_10_ACT;
static u32 g_led_phy_mode_group1                 = SWAPI_LED_PHYMODE_1000_ACT;
static u32 g_led_phy_mode_group2                 = SWAPI_LED_OFF;

static u32 g_jumbo_frames_enabled                = RTL8367_DEFAULT_JUMBO_FRAMES;
static u32 g_green_ethernet_enabled              = RTL8367_DEFAULT_GREEN_ETHERNET;

static u32 g_storm_rate_unicast_unknown          = RTL8367_DEFAULT_STORM_RATE;
static u32 g_storm_rate_multicast_unknown        = RTL8367_DEFAULT_STORM_RATE;
static u32 g_storm_rate_multicast                = RTL8367_DEFAULT_STORM_RATE;
static u32 g_storm_rate_broadcast                = RTL8367_DEFAULT_STORM_RATE;

static u32 g_port_link_mode[RTK_PHY_ID_MAX+1]    = {RTL8367_DEFAULT_LINK_MODE};
static u32 g_port_phy_power[RTK_PHY_ID_MAX+1]    = {1};
#if defined(EXT_PORT_INIC)
static u32 g_port_link_inic                      = 0;
#endif

static u32 g_rgmii_delay_tx                      = CONFIG_RTL8367_RGMII_DELAY_TX;	/* 0..1 */
static u32 g_rgmii_delay_rx                      = CONFIG_RTL8367_RGMII_DELAY_RX;	/* 0..7 */

static u32 g_vlan_rule[SWAPI_VLAN_RULE_NUM]      = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static u32 g_vlan_rule_user[SWAPI_VLAN_RULE_NUM] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

////////////////////////////////////////////////////////////////////////////////////

static u32 g_vlan_cleared                        = 0;
static u32 g_vlan_pvid_wan_untagged              = 2;

static bwan_member_t g_bwan_member[SWAPI_WAN_BRIDGE_NUM][RTK_PHY_ID_MAX+1];

////////////////////////////////////////////////////////////////////////////////////

const char *g_port_desc_wan   = "WAN";
const char *g_port_desc_lan1  = "LAN1";
const char *g_port_desc_lan2  = "LAN2";
const char *g_port_desc_lan3  = "LAN3";
const char *g_port_desc_lan4  = "LAN4";

////////////////////////////////////////////////////////////////////////////////////

static void asic_dump_isolation(int max_vlan_vid)
{
	int i, ivl_svl;
	rtk_api_ret_t retVal;
	rtk_data_t Efid;
	rtk_vlan_t Pvid;
	rtk_pri_t Priority;
	rtk_fid_t Fid;
	rtk_portmask_t mask1, mask2;
	rtk_enable_t Igr_filter;
	rtk_vlan_acceptFrameType_t Accept_frame_type;

	printk("%s: dump ports isolation:\n", RTL8367_DEVNAME);

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++) {
		rtk_port_efid_get(i, &Efid);
		retVal = rtk_port_isolation_get(i, &mask1);
		if (retVal == RT_ERR_OK)
			printk("  port (%d) isolation: mask=%04X, efid=%d\n", i, mask1.bits[0], Efid);
	}

	printk("%s: dump port-based vlan:\n", RTL8367_DEVNAME);

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++) {
		retVal = rtk_vlan_portPvid_get(i, &Pvid, &Priority);
		if (retVal == RT_ERR_OK) 
			printk("  port (%d) vlan: pvid=%d, prio=%d\n", i, Pvid, Priority);
	}

	printk("%s: dump ports accept mode:\n", RTL8367_DEVNAME);

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++) {
		rtk_vlan_portIgrFilterEnable_get(i, &Igr_filter);
		retVal = rtk_vlan_portAcceptFrameType_get(i, &Accept_frame_type);
		if (retVal == RT_ERR_OK)
			printk("  port (%d) accept: type=%d, ingress=%d\n", i, Accept_frame_type, Igr_filter);
	}

	printk("%s: dump vlan members:\n", RTL8367_DEVNAME);

	max_vlan_vid &= 0xFFF;
	for (i = 1; i <= max_vlan_vid; i++) {
		retVal = rtk_vlan_get(i, &mask1, &mask2, &Fid);
		if (retVal == RT_ERR_OK) {
			ivl_svl = 0;
#if !defined(CONFIG_RTL8367_API_8370)
			if (Fid == RTK_IVL_MODE_FID) {
				ivl_svl = 1;
				Fid = 0;
			}
#endif
			printk("  vlan (%d): member=%04X, untag=%04X, fid=%d, ivl=%d\n", i, mask1.bits[0], mask2.bits[0], Fid, ivl_svl);
		}
	}
}

static void asic_dump_ucast_table(void)
{
#if !defined(CONFIG_RTL8367_API_8370)
	rtk_api_ret_t retVal;
	int index = 0;
	u32 addr_id = 0;
	rtl8367b_luttb l2t;

	printk("%s - dump L2 MAC table:\n", RTL8367_DEVNAME);

	while (addr_id <= RTK_MAX_LUT_ADDR_ID) {
		memset(&l2t, 0, sizeof(l2t));
		l2t.address = addr_id;
		retVal = rtl8367b_getAsicL2LookupTb(LUTREADMETHOD_NEXT_L2UC, &l2t);
		if (retVal != RT_ERR_OK)
			break;
		if (l2t.address < addr_id)
			break;
		
		addr_id = l2t.address + 1;
		index++;
		printk("%4d. %02X-%02X-%02X-%02X-%02X-%02X, port: %d, efid: %d, fid: %2d, cvid: %4d, ivl: %d, static: %d\n",
			index,
			l2t.mac.octet[0], l2t.mac.octet[1], l2t.mac.octet[2],
			l2t.mac.octet[3], l2t.mac.octet[4], l2t.mac.octet[5],
			l2t.spa,
			l2t.efid,
			l2t.fid,
			l2t.cvid_fid,
			l2t.ivl_svl,
			l2t.nosalearn
		);
	}
#endif
}

static void asic_clear_mac_table(void)
{
#if !defined(CONFIG_RTL8367_API_8370)
	rtl8367b_setAsicLutFlushMode(FLUSHMDOE_PORT);
	rtl8367b_setAsicLutFlushType(FLUSHTYPE_DYNAMIC);
	rtl8367b_setAsicLutForceFlush(RTL8367B_PORTMASK);
#endif
#if defined(CONFIG_P5_RGMII_TO_MAC_MODE) && defined(CONFIG_P4_RGMII_TO_MAC_MODE)
	mt7620_esw_mac_table_clear();
#endif
}

u32 get_phy_ports_mask_lan(u32 include_cpu)
{
	u32 i, wan_bridge_mode, portmask_lan;

	wan_bridge_mode = g_wan_bridge_mode;

	portmask_lan = ((1u << LAN_PORT_4) | (1u << LAN_PORT_3) | (1u << LAN_PORT_2) | (1u << LAN_PORT_1));
	if (include_cpu)
		portmask_lan |= (1u << LAN_PORT_CPU);

	for (i = 0; i <= RTK_PHY_ID_MAX; i++) {
		if (g_bwan_member[wan_bridge_mode][i].bwan)
			portmask_lan &= ~(1u << i);
	}

	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		portmask_lan |= (1u << WAN_PORT_X);

	return portmask_lan;
}

u32 get_phy_ports_mask_wan(u32 include_cpu)
{
	u32 i, wan_bridge_mode, portmask_wan;

	wan_bridge_mode = g_wan_bridge_mode;
	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		return 0;

	portmask_wan = (1u << WAN_PORT_X);

	if (include_cpu)
		portmask_wan |= (1u << WAN_PORT_CPU);

	for (i = 0; i <= RTK_PHY_ID_MAX; i++) {
		if (g_bwan_member[wan_bridge_mode][i].bwan)
			portmask_wan |= (1u << i);
	}

	return portmask_wan;
}

u32 get_ports_mask_from_user(u32 user_port_mask)
{
	u32 gsw_ports_mask = 0;

	if (user_port_mask & SWAPI_PORTMASK_LAN1)
		gsw_ports_mask |= (1u << LAN_PORT_1);
	if (user_port_mask & SWAPI_PORTMASK_LAN2)
		gsw_ports_mask |= (1u << LAN_PORT_2);
	if (user_port_mask & SWAPI_PORTMASK_LAN3)
		gsw_ports_mask |= (1u << LAN_PORT_3);
	if (user_port_mask & SWAPI_PORTMASK_LAN4)
		gsw_ports_mask |= (1u << LAN_PORT_4);
	if (user_port_mask & SWAPI_PORTMASK_WAN)
		gsw_ports_mask |= (1u << WAN_PORT_X);
	if (user_port_mask & SWAPI_PORTMASK_CPU_LAN)
		gsw_ports_mask |= (1u << LAN_PORT_CPU);
	if (user_port_mask & SWAPI_PORTMASK_CPU_WAN)
		gsw_ports_mask |= (1u << WAN_PORT_CPU);
#if defined(EXT_PORT_INIC)
	if (user_port_mask & SWAPI_PORTMASK_INIC)
		gsw_ports_mask |= (1u << EXT_PORT_INIC);
#endif
	return gsw_ports_mask;
}

static rtk_port_t get_port_from_user(u32 user_port_mask)
{
	if (user_port_mask & SWAPI_PORTMASK_LAN1)
		return LAN_PORT_1;
	if (user_port_mask & SWAPI_PORTMASK_LAN2)
		return LAN_PORT_2;
	if (user_port_mask & SWAPI_PORTMASK_LAN3)
		return LAN_PORT_3;
	if (user_port_mask & SWAPI_PORTMASK_LAN4)
		return LAN_PORT_4;
	if (user_port_mask & SWAPI_PORTMASK_WAN)
		return WAN_PORT_X;
	if (user_port_mask & SWAPI_PORTMASK_CPU_LAN)
		return LAN_PORT_CPU;
#if !defined(RTL8367_SINGLE_EXTIF)
	if (user_port_mask & SWAPI_PORTMASK_CPU_WAN)
		return WAN_PORT_CPU;
#endif
#if defined(EXT_PORT_INIC)
	if (user_port_mask & SWAPI_PORTMASK_INIC)
		return EXT_PORT_INIC;
#endif
	return RTK_MAX_NUM_OF_PORT;
}

static const char* get_port_desc(rtk_port_t port)
{
	const char *port_desc;
	static char unk_desc[16] = {0};

	switch (port)
	{
	case WAN_PORT_X:
		port_desc = g_port_desc_wan;
		break;
	case LAN_PORT_1:
		port_desc = g_port_desc_lan1;
		break;
	case LAN_PORT_2:
		port_desc = g_port_desc_lan2;
		break;
	case LAN_PORT_3:
		port_desc = g_port_desc_lan3;
		break;
	case LAN_PORT_4:
		port_desc = g_port_desc_lan4;
		break;
	default:
		snprintf(unk_desc, sizeof(unk_desc), "Port ID: %d", port);
		port_desc = unk_desc;
		break;
	}

	return port_desc;
}

static void asic_bridge_isolate(u32 wan_bridge_mode, u32 bwan_isolated_mode)
{
	int i;
	char *wan1, *wan2;
	u32 fwd_mask_bwan_lan;
	rtk_portmask_t fwd_mask_lan, fwd_mask_wan, fwd_mask;

	if (WAN_PORT_X < LAN_PORT_4)
	{
		wan1 = "";
		wan2 = "|W";
	}
	else
	{
		wan1 = "W|";
		wan2 = "";
	}

	switch (wan_bridge_mode)
	{
	case SWAPI_WAN_BRIDGE_LAN1:
		printk("%s - hw bridge: %sWLLL%s\n", RTL8367_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN2:
		printk("%s - hw bridge: %sLWLL%s\n", RTL8367_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN3:
		printk("%s - hw bridge: %sLLWL%s\n", RTL8367_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN4:
		printk("%s - hw bridge: %sLLLW%s\n", RTL8367_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN3_LAN4:
		printk("%s - hw bridge: %sLLWW%s\n", RTL8367_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2:
		printk("%s - hw bridge: %sWWLL%s\n", RTL8367_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		printk("%s - hw bridge: %sWWWL%s\n", RTL8367_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_DISABLE_WAN:
		printk("%s - hw bridge: LLLLL\n", RTL8367_DEVNAME);
		break;
	default:
		printk("%s - hw bridge: %sLLLL%s\n", RTL8367_DEVNAME, wan1, wan2);
		break;
	}

	fwd_mask_lan.bits[0] = get_phy_ports_mask_lan(1);

	/* LAN (efid=0) */
	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++) {
		if ((fwd_mask_lan.bits[0] >> i) & 0x1) {
			fwd_mask.bits[0] = fwd_mask_lan.bits[0];
			
#if defined(EXT_PORT_INIC)
			if (i == LAN_PORT_CPU) {
				/* force add iNIC port to forward from CPU */
				fwd_mask.bits[0] |= (1u << EXT_PORT_INIC);
			}
#endif
			rtk_port_isolation_set(i, fwd_mask);
#if !defined(RTL8367_SINGLE_EXTIF)
			rtk_port_efid_set(i, 0);
#endif
		}
	}

#if defined(EXT_PORT_INIC)
	/* for prevent flood to uninitialized iNIC port, isolate iNIC from LAN */
	fwd_mask.bits[0] = ((1u << LAN_PORT_CPU) | (1u << EXT_PORT_INIC));
	rtk_port_isolation_set(EXT_PORT_INIC, fwd_mask);
#endif

	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		return;

	fwd_mask_wan.bits[0] = get_phy_ports_mask_wan(1);
	fwd_mask_bwan_lan = fwd_mask_wan.bits[0] & ~((1u << WAN_PORT_X) | (1u << WAN_PORT_CPU));

	/* WAN (efid=1) */
	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++) {
		if ((fwd_mask_wan.bits[0] >> i) & 0x1) {
			fwd_mask.bits[0] = fwd_mask_wan.bits[0];
#if defined(RTL8367_SINGLE_EXTIF)
			if (i == WAN_PORT_CPU) {
				/* force add all LAN ports to forward from CPU */
				fwd_mask.bits[0] |= ((1u << LAN_PORT_4) | (1u << LAN_PORT_3) | (1u << LAN_PORT_2) | (1u << LAN_PORT_1));
#if defined(EXT_PORT_INIC)
				/* force add iNIC port to forward from CPU */
				fwd_mask.bits[0] |= (1u << EXT_PORT_INIC);
#endif
			}
#endif
			if (bwan_isolated_mode == SWAPI_WAN_BWAN_ISOLATION_FROM_CPU) {
				switch(i)
				{
				case WAN_PORT_CPU:
					fwd_mask.bits[0] &= ~(fwd_mask_bwan_lan);
					break;
				case LAN_PORT_4:
				case LAN_PORT_3:
				case LAN_PORT_2:
				case LAN_PORT_1:
					fwd_mask.bits[0] &= ~(1u << WAN_PORT_CPU);
					break;
				}
			} else if (bwan_isolated_mode == SWAPI_WAN_BWAN_ISOLATION_BETWEEN) {
				switch(i)
				{
				case WAN_PORT_X:
					fwd_mask.bits[0] &= ~((1u << LAN_PORT_4)|(1u << LAN_PORT_3)|(1u << LAN_PORT_2)|(1u << LAN_PORT_1));
					break;
				case LAN_PORT_4:
					fwd_mask.bits[0] &= ~((1u << WAN_PORT_X)|(1u << LAN_PORT_3)|(1u << LAN_PORT_2)|(1u << LAN_PORT_1));
					break;
				case LAN_PORT_3:
					fwd_mask.bits[0] &= ~((1u << WAN_PORT_X)|(1u << LAN_PORT_4)|(1u << LAN_PORT_2)|(1u << LAN_PORT_1));
					break;
				case LAN_PORT_2:
					fwd_mask.bits[0] &= ~((1u << WAN_PORT_X)|(1u << LAN_PORT_4)|(1u << LAN_PORT_3)|(1u << LAN_PORT_1));
					break;
				case LAN_PORT_1:
					fwd_mask.bits[0] &= ~((1u << WAN_PORT_X)|(1u << LAN_PORT_4)|(1u << LAN_PORT_3)|(1u << LAN_PORT_2));
					break;
				}
			}
			
			rtk_port_isolation_set(i, fwd_mask);
#if !defined(RTL8367_SINGLE_EXTIF)
			rtk_port_efid_set(i, 1);
#endif
		}
	}
}

#if defined(EXT_PORT_INIC)
static void toggle_isolation_inic(u32 inic_isolated)
{
	int i;
	rtk_portmask_t fwd_mask, fwd_mask_lan;

	fwd_mask_lan.bits[0] = get_phy_ports_mask_lan(0);

	for (i = 0; i <= RTK_PHY_ID_MAX; i++) {
		if ((fwd_mask_lan.bits[0] >> i) & 0x1) {
			fwd_mask.bits[0] = (fwd_mask_lan.bits[0] | (1u << LAN_PORT_CPU));
			if (!inic_isolated)
				fwd_mask.bits[0] |= (1u << EXT_PORT_INIC);
			
			rtk_port_isolation_set(i, fwd_mask);
		}
	}

	fwd_mask.bits[0] = ((1u << LAN_PORT_CPU) | (1u << EXT_PORT_INIC));
	if (!inic_isolated)
		fwd_mask.bits[0] |= fwd_mask_lan.bits[0];
	rtk_port_isolation_set(EXT_PORT_INIC, fwd_mask);

	printk("%s - iNIC isolation: %d\n", RTL8367_DEVNAME, inic_isolated);
}

static void toggle_disable_inic(u32 inic_disable)
{
	rtk_port_mac_ability_t mac_cfg;

	g_port_link_inic = !inic_disable;

	mac_cfg.speed		= SPD_1000M;
	mac_cfg.forcemode	= MAC_FORCE;
	mac_cfg.duplex		= FULL_DUPLEX;
	mac_cfg.link		= (inic_disable) ? PORT_LINKDOWN : PORT_LINKUP;
	mac_cfg.nway		= DISABLED;
	mac_cfg.rxpause		= ENABLED;
	mac_cfg.txpause		= ENABLED;
	rtk_port_macForceLinkExt_set(WAN_EXT_ID, MODE_EXT_RGMII, &mac_cfg);

	printk("%s - iNIC port link: %d\n", RTL8367_DEVNAME, !inic_disable);
}
#endif

static int asic_port_forward_mask(u32 port_mask, u32 fwd_ports_mask)
{
	rtk_portmask_t fwd_mask;
	rtk_port_t port = get_port_from_user(port_mask);

	if (port >= RTK_MAX_NUM_OF_PORT)
		return -EINVAL;

	fwd_mask.bits[0] = get_ports_mask_from_user(fwd_ports_mask & 0xFF);

	rtk_port_isolation_set(port, fwd_mask);

	printk("%s - port %d forward mask: %04X\n", RTL8367_DEVNAME, port, fwd_mask.bits[0]);

	return 0;
}

static void asic_vlan_set_ingress_ports(u32 reg_ingress)
{
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_setAsicReg(RTL8370_REG_VLAN_INGRESS, reg_ingress);
#else
	rtl8367b_setAsicReg(RTL8367B_REG_VLAN_INGRESS, reg_ingress);
#endif
}

static void asic_vlan_accept_port_mode(u32 accept_mode, u32 port_mask)
{
	int i;
	rtk_vlan_acceptFrameType_t acceptFrameType = ACCEPT_FRAME_TYPE_ALL;

	switch (accept_mode)
	{
	case SWAPI_VLAN_ACCEPT_FRAMES_UNTAG_ONLY:
		acceptFrameType = ACCEPT_FRAME_TYPE_UNTAG_ONLY;
		break;
	case SWAPI_VLAN_ACCEPT_FRAMES_TAG_ONLY:
		acceptFrameType = ACCEPT_FRAME_TYPE_TAG_ONLY;
		break;
	}

	port_mask = get_ports_mask_from_user(port_mask & 0xFF);

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++) {
		if ((port_mask >> i) & 0x1)
			rtk_vlan_portAcceptFrameType_set(i, acceptFrameType);
	}
}

static void asic_vlan_create_port_vid(u32 vlan4k_info, u32 vlan4k_mask)
{
	int i;
	rtk_vlan_t pvid;
	rtk_pri_t prio;
	rtk_fid_t fid;
	rtk_portmask_t mask_member, mask_untag;

	pvid = (rtk_vlan_t)(vlan4k_info & 0x0FFF);
	prio = (rtk_pri_t)((vlan4k_info >> 12) & 0x7);
#if defined(CONFIG_RTL8367_API_8370)
	fid  = (rtk_fid_t)((vlan4k_info >> 16) & 0x0FFF);
#else
	fid  = RTK_IVL_MODE_FID;
#endif
	mask_member.bits[0] = get_ports_mask_from_user((vlan4k_mask & 0xFF));
	mask_untag.bits[0]  = get_ports_mask_from_user((vlan4k_mask >> 16) & 0xFF);

	rtk_vlan_set(pvid, mask_member, mask_untag, fid);

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++) {
		if ((mask_untag.bits[0] >> i) & 0x1)
			rtk_vlan_portPvid_set(i, pvid, prio);
	}

	g_vlan_cleared = 0;

	printk("%s - create vlan port: pvid=[%d], prio=[%d], member=[0x%04X], untag=[0x%04X], fid=[%d]\n",
			RTL8367_DEVNAME, pvid, prio, mask_member.bits[0], mask_untag.bits[0], fid);
}

static void asic_vlan_create_entry(u32 vlan4k_info, u32 vlan4k_mask)
{
	rtk_vlan_t vid;
	rtk_fid_t fid;
	rtk_portmask_t mask_member, mask_untag;

	vid = (rtk_vlan_t)(vlan4k_info & 0x0FFF);
#if defined(CONFIG_RTL8367_API_8370)
	fid = (rtk_fid_t)((vlan4k_info >> 16) & 0x0FFF);
#else
	fid  = RTK_IVL_MODE_FID;
#endif
	mask_member.bits[0] = get_ports_mask_from_user((vlan4k_mask & 0xFF));
	mask_untag.bits[0]  = get_ports_mask_from_user((vlan4k_mask >> 16) & 0xFF);

	rtk_vlan_set(vid, mask_member, mask_untag, fid);

	g_vlan_cleared = 0;

	printk("%s - create vlan entry: vid=[%d], member=[0x%04X], untag=[0x%04X], fid=[%d]\n",
			RTL8367_DEVNAME, vid, mask_member.bits[0], mask_untag.bits[0], fid);
}

#if defined(EXT_PORT_INIC)
static void init_ralink_iNIC_rule(void)
{
	rtk_fid_t fid;
	rtk_filter_cfg_t Fc;
	rtk_filter_action_t Fa;
	rtk_filter_field_t Ff;
	rtk_filter_number_t ruleNum;
	rtk_portmask_t mask_member, mask_untag;

	rtk_filter_igrAcl_init();

	/* VLAN for iNIC boot/heartbeat packets */
#if defined(CONFIG_RTL8367_API_8370)
	fid = 0;
#else
	fid = RTK_IVL_MODE_FID;
#endif
	mask_member.bits[0] = (1u << EXT_PORT_INIC) | (1u << LAN_PORT_CPU);
	mask_untag.bits[0]  = (1u << EXT_PORT_INIC) | (1u << LAN_PORT_CPU);
	rtk_vlan_set(INIC_HEART_VLAN_VID, mask_member, mask_untag, fid);

	memset(&Fc, 0, sizeof(Fc));
	memset(&Fa, 0, sizeof(Fa));
	memset(&Ff, 0, sizeof(Ff));

	Ff.fieldType = FILTER_FIELD_ETHERTYPE;
	Ff.filter_pattern_union.etherType.dataType = FILTER_FIELD_DATA_MASK;
	Ff.filter_pattern_union.etherType.value = 0xFFFF; // boot/heartbeat packets use etherType 0xFFFF
	Ff.filter_pattern_union.etherType.mask  = 0xFFFF;

	rtk_filter_igrAcl_field_add(&Fc, &Ff);

	Fc.activeport.dataType = FILTER_FIELD_DATA_MASK;
	Fc.activeport.value = (1u << LAN_PORT_CPU) | (1u << EXT_PORT_INIC);
	Fc.activeport.mask  = 0xFF;

	Fa.actEnable[FILTER_ENACT_INGRESS_CVLAN_VID] = 1;
	Fa.filterIngressCvlanVid = INIC_HEART_VLAN_VID;

	rtk_filter_igrAcl_cfg_add(0, &Fc, &Fa, &ruleNum);
}
#endif

static void asic_vlan_reset_table(void)
{
	u32 i, vid_start;
	rtk_fid_t fid;
	rtk_portmask_t mask_member, mask_untag;

	mask_member.bits[0] = 0;
	mask_untag.bits[0]  = 0;

	/* init VLAN table (VLAN1) and enable VLAN */
	rtk_vlan_init();

	/* clear VLAN2 from previous config */
	rtk_vlan_set(2, mask_member, mask_untag, 0);

	vid_start = 3;
#if defined(EXT_PORT_INIC)
	/* clear VLAN3 for iNIC Guest AP */
	vid_start = INIC_GUEST_VLAN_VID + 1;
#endif

#if !defined(DISABLE_VLAN_SHADOW)
	for (i = vid_start; i < 4095; i++) {
		if (rtk_vlan_get(i, &mask_member, &mask_untag, &fid) == RT_ERR_OK && mask_member.bits[0] != 0) {
			mask_member.bits[0] = 0;
			mask_untag.bits[0]  = 0;
			rtk_vlan_set(i, mask_member, mask_untag, 0);
		}
	}
#endif

	g_vlan_cleared = 1;

#if defined(EXT_PORT_INIC)
	/* configure Acl */
	init_ralink_iNIC_rule();
#endif
}

#define VLAN_ENTRY_ID_MAX	(15)
static u32 find_vlan_slot(vlan_entry_t *vlan_entry, u32 start_idx, u32 cvid)
{
	u32 i;

	for (i = start_idx; i <= VLAN_ENTRY_ID_MAX; i++) {
		if (!vlan_entry[i].valid || vlan_entry[i].cvid == cvid)
			return i;
	}

	return VLAN_ENTRY_ID_MAX + 1; // not found
}

static u32 find_free_min_pvid(u32 *pvid_list, u32 vid)
{
	u32 i, vid_new;

	vid_new = vid;
	for (i = 0; i < SWAPI_VLAN_RULE_NUM; i++) {
		if (vid == pvid_list[i]
#if defined(EXT_PORT_INIC)
		 || vid == INIC_GUEST_VLAN_VID
#endif
		    ) {
			/* recursion step */
			vid_new = find_free_min_pvid(pvid_list, vid+1);
			break;
		}
	}
	return vid_new;
}

static int is_wan_vid_valid(u32 vid)
{
	return (vid == 2 || vid >= MIN_EXT_VLAN_VID) ? 1 : 0;
}

static void asic_vlan_apply_rules(u32 wan_bridge_mode)
{
	vlan_entry_t vlan_entry[VLAN_ENTRY_ID_MAX+1];
	pvlan_member_t pvlan_member[RTK_PHY_ID_MAX+1];
	u32 pvid[SWAPI_VLAN_RULE_NUM];
	u32 prio[SWAPI_VLAN_RULE_NUM];
	u32 tagg[SWAPI_VLAN_RULE_NUM];
	u32 i, cvid, untg_vid, next_idx, vlan_idx, mask_ingress, vlan_filter_on;
#if !defined(RTL8367_SINGLE_EXTIF)
	pvlan_member_t pvlan_member_cpu_wan;
#endif

	untg_vid = 2;	// default PVID for untagged WAN traffic
	next_idx = 2;
	vlan_filter_on = 0;

	memset(vlan_entry, 0, sizeof(vlan_entry));
	memset(pvlan_member, 0, sizeof(pvlan_member));

	for (i = 0; i < SWAPI_VLAN_RULE_NUM; i++) {
		pvid[i] =  (g_vlan_rule_user[i] & 0xFFF);
		prio[i] = ((g_vlan_rule_user[i] >> 16) & 0x7);
		tagg[i] = ((g_vlan_rule_user[i] >> 24) & 0x1);
		if (is_wan_vid_valid(pvid[i]))
			vlan_filter_on = 1;
	}

	/* find minimal unused VID, when VID=2 is used */
	if (vlan_filter_on)
		untg_vid = find_free_min_pvid(pvid, 2);

	g_vlan_pvid_wan_untagged = untg_vid;

	if (!is_wan_vid_valid(pvid[SWAPI_VLAN_RULE_WAN_INET])) {
		pvid[SWAPI_VLAN_RULE_WAN_INET] = untg_vid; // VID 2
		prio[SWAPI_VLAN_RULE_WAN_INET] = 0;
		tagg[SWAPI_VLAN_RULE_WAN_INET] = 0;
	} else {
		tagg[SWAPI_VLAN_RULE_WAN_INET] = 1;
	}

	if (!is_wan_vid_valid(pvid[SWAPI_VLAN_RULE_WAN_IPTV])) {
		pvid[SWAPI_VLAN_RULE_WAN_IPTV] = untg_vid; // VID 2
		prio[SWAPI_VLAN_RULE_WAN_IPTV] = 0;
		tagg[SWAPI_VLAN_RULE_WAN_IPTV] = 0;
	} else {
		tagg[SWAPI_VLAN_RULE_WAN_IPTV] = 1;
	}

	mask_ingress = (1u << LAN_PORT_CPU) | (1u << WAN_PORT_CPU) | (1u << WAN_PORT_X);

	/* fill WAN port (use PVID 2 for handle untagged traffic -> VID2) */
	pvlan_member[WAN_PORT_X].pvid = untg_vid;

#if !defined(RTL8367_SINGLE_EXTIF)
	/* fill CPU WAN port (use PVID 2 for handle untagged traffic -> VID2) */
	pvlan_member_cpu_wan.pvid = untg_vid;
	pvlan_member_cpu_wan.prio = 0;
	pvlan_member_cpu_wan.tagg = 0;
#endif

	/* VID #1 */
	vlan_entry[0].valid = 1;
	vlan_entry[0].fid = 1;
	vlan_entry[0].cvid = 1;
	vlan_entry[0].port_member |= (1u << LAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
	vlan_entry[0].port_untag  |= (1u << LAN_PORT_CPU);
#endif

	/* VID #2 */
	vlan_entry[1].valid = 1;
	vlan_entry[1].fid = untg_vid;
	vlan_entry[1].cvid = untg_vid;
	vlan_entry[1].port_member |= ((1u << WAN_PORT_X) | (1u << WAN_PORT_CPU));
	vlan_entry[1].port_untag  |=  (1u << WAN_PORT_X);

#if defined(EXT_PORT_INIC)
	/* VID #3 (iNIC guest AP) */
	vlan_entry[2].valid = 1;
	vlan_entry[2].fid = INIC_GUEST_VLAN_FID;
	vlan_entry[2].cvid = INIC_GUEST_VLAN_VID;
	vlan_entry[2].port_member |= ((1u << EXT_PORT_INIC) | (1u << LAN_PORT_CPU));
	vlan_entry[0].port_member |=  (1u << EXT_PORT_INIC);
	vlan_entry[0].port_untag  |=  (1u << EXT_PORT_INIC);
	mask_ingress |= (1u << EXT_PORT_INIC);
	next_idx++;
#endif

#if defined(CONFIG_P5_RGMII_TO_MAC_MODE) && defined(CONFIG_P4_RGMII_TO_MAC_MODE)
	/* clear vlan members on MT7620 ESW (slot idx 2..3) */
	mt7620_esw_vlan_clear_idx(2);
	mt7620_esw_vlan_clear_idx(3);

	/* update VID=2 members (P7|P6|P4) */
	mt7620_esw_vlan_set_idx(1, untg_vid, 0xd0);

	/* set ESW P4 PVID */
	mt7620_esw_pvid_set(4, untg_vid, 0);
#endif

	/* check IPTV tagged */
	if (tagg[SWAPI_VLAN_RULE_WAN_IPTV]) {
		cvid = pvid[SWAPI_VLAN_RULE_WAN_IPTV];
		vlan_idx = find_vlan_slot(vlan_entry, next_idx, cvid);
		if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
			if (!vlan_entry[vlan_idx].valid) {
				vlan_entry[vlan_idx].valid = 1;
				vlan_entry[vlan_idx].fid = cvid;
				vlan_entry[vlan_idx].cvid = cvid;
#if defined(CONFIG_P5_RGMII_TO_MAC_MODE) && defined(CONFIG_P4_RGMII_TO_MAC_MODE)
				/* need add vlan members on MT7620 ESW P4 (ESW members P7|P6|P4) */
				mt7620_esw_vlan_set_idx(2, cvid, 0xd0);
#endif
			}
			vlan_entry[vlan_idx].port_member |= ((1u << WAN_PORT_CPU) | (1u << WAN_PORT_X));
			pvlan_member[WAN_PORT_X].tagg = 1;
		}
	}

	/* check INET tagged */
	if (tagg[SWAPI_VLAN_RULE_WAN_INET]) {
		cvid = pvid[SWAPI_VLAN_RULE_WAN_INET];
		vlan_idx = find_vlan_slot(vlan_entry, next_idx, cvid);
		if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
			if (!vlan_entry[vlan_idx].valid) {
				vlan_entry[vlan_idx].valid = 1;
				vlan_entry[vlan_idx].fid = cvid;
				vlan_entry[vlan_idx].cvid = cvid;
#if defined(CONFIG_P5_RGMII_TO_MAC_MODE) && defined(CONFIG_P4_RGMII_TO_MAC_MODE)
				/* need add vlan members on MT7620 ESW P4 (ESW members P7|P6|P4) */
				mt7620_esw_vlan_set_idx(3, cvid, 0xd0);
#endif
			}
			vlan_entry[vlan_idx].port_member |= ((1u << WAN_PORT_CPU) | (1u << WAN_PORT_X));
			pvlan_member[WAN_PORT_X].tagg = 1;
		}
	}

#if !defined(RTL8367_SINGLE_EXTIF)
	/* if INET and IPTV tagged with common VID, untag WAN_CPU + use PVID */
	if (tagg[SWAPI_VLAN_RULE_WAN_INET] && pvid[SWAPI_VLAN_RULE_WAN_INET] == pvid[SWAPI_VLAN_RULE_WAN_IPTV]) {
		/* update VID #2 members (do not forward untagged packets to WAN_CPU) */
		vlan_entry[1].port_member &= ~(1u << WAN_PORT_CPU);
		cvid = pvid[SWAPI_VLAN_RULE_WAN_INET];
		vlan_idx = find_vlan_slot(vlan_entry, next_idx, cvid);
		if (vlan_idx <= VLAN_ENTRY_ID_MAX && vlan_entry[vlan_idx].valid)
			vlan_entry[vlan_idx].port_untag |= (1u << WAN_PORT_CPU);
		pvlan_member_cpu_wan.pvid = cvid;
		pvlan_member_cpu_wan.prio = prio[SWAPI_VLAN_RULE_WAN_IPTV];
	} else if (!tagg[SWAPI_VLAN_RULE_WAN_INET] && !tagg[SWAPI_VLAN_RULE_WAN_IPTV]) {
		/* update VID #2 untag members */
		vlan_entry[1].port_untag |= (1u << WAN_PORT_CPU);
	}
#endif

	/* fill physical LAN ports */
	for (i = 0; i <= RTK_PHY_ID_MAX; i++) {
		int rule_id;
		
		if (i == WAN_PORT_X)
			continue;
		
		mask_ingress |= (1u << i);
		
		if (!g_bwan_member[wan_bridge_mode][i].bwan) {
			pvlan_member[i].pvid = 1;
			
			/* VID #1 */
			vlan_entry[0].port_member |= (1u << i);
			vlan_entry[0].port_untag  |= (1u << i);
			
			continue;
		}
		
		rule_id = g_bwan_member[wan_bridge_mode][i].rule;
		if (!is_wan_vid_valid(pvid[rule_id])) {
			pvlan_member[i].pvid = untg_vid;
			
			/* VID #2 */
			vlan_entry[1].port_member |= (1u << i);
			vlan_entry[1].port_untag  |= (1u << i);
		} else {
			cvid = pvid[rule_id];
			vlan_idx = find_vlan_slot(vlan_entry, next_idx, cvid);
			if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
				if (!vlan_entry[vlan_idx].valid) {
					vlan_entry[vlan_idx].valid = 1;
					vlan_entry[vlan_idx].fid = cvid;
					vlan_entry[vlan_idx].cvid = cvid;
				}
				vlan_entry[vlan_idx].port_member |= ((1u << i) | (1u << WAN_PORT_X));
				if (!tagg[rule_id])
					vlan_entry[vlan_idx].port_untag |= (1u << i);
				
				pvlan_member[i].pvid = cvid;
				pvlan_member[i].prio = prio[rule_id];
				pvlan_member[i].tagg = tagg[rule_id];
				
				pvlan_member[WAN_PORT_X].tagg = 1;
			} else {
				pvlan_member[i].pvid = untg_vid;
				
				/* VID #2 */
				vlan_entry[1].port_member |= (1u << i);
				vlan_entry[1].port_untag  |= (1u << i);
			}
		}
	}

	/* fill VLAN table */
	if (!g_vlan_cleared)
		asic_vlan_reset_table();

	for (i = 0; i <= VLAN_ENTRY_ID_MAX; i++) {
		rtk_fid_t fid;
		rtk_portmask_t mask_member, mask_untag;
		
		if (!vlan_entry[i].valid)
			continue;
		
		mask_member.bits[0] = vlan_entry[i].port_member;
		mask_untag.bits[0]  = vlan_entry[i].port_untag;
#if defined(CONFIG_RTL8367_API_8370)
		fid = vlan_entry[i].fid;
#else
		fid = RTK_IVL_MODE_FID;
#endif
		rtk_vlan_set(vlan_entry[i].cvid, mask_member, mask_untag, fid);
	}

	/* set ingress filtering */
	asic_vlan_set_ingress_ports(mask_ingress);

	/* configure physical LAN/WAN ports */
	for (i = 0; i <= RTK_PHY_ID_MAX; i++) {
		rtk_vlan_portPvid_set(i, pvlan_member[i].pvid, pvlan_member[i].prio);
		rtk_vlan_portAcceptFrameType_set(i, (pvlan_member[i].tagg) ? ACCEPT_FRAME_TYPE_ALL : ACCEPT_FRAME_TYPE_UNTAG_ONLY);
	}

	/* configure CPU LAN port */
	rtk_vlan_portPvid_set(LAN_PORT_CPU, 1, 0);
#if defined(RTL8367_SINGLE_EXTIF)
	/* set CPU port accept mask (trunk port - accept tag only) */
	rtk_vlan_portAcceptFrameType_set(LAN_PORT_CPU, ACCEPT_FRAME_TYPE_TAG_ONLY);
#else
	rtk_vlan_portAcceptFrameType_set(LAN_PORT_CPU, ACCEPT_FRAME_TYPE_ALL);
#endif

	/* configure CPU WAN port (or iNIC) */
#if defined(RTL8367_SINGLE_EXTIF)
#if defined(EXT_PORT_INIC)
	rtk_vlan_portPvid_set(EXT_PORT_INIC, 1, 0);
	rtk_vlan_portAcceptFrameType_set(EXT_PORT_INIC, ACCEPT_FRAME_TYPE_ALL);
#endif
#else
	rtk_vlan_portPvid_set(WAN_PORT_CPU, pvlan_member_cpu_wan.pvid, pvlan_member_cpu_wan.prio);
	rtk_vlan_portAcceptFrameType_set(WAN_PORT_CPU, ACCEPT_FRAME_TYPE_ALL);
#endif

	g_vlan_cleared = 0;

	/* save VLAN rules */
	for (i = 0; i < SWAPI_VLAN_RULE_NUM; i++)
		g_vlan_rule[i] = g_vlan_rule_user[i];
}

static void asic_vlan_init_vid1(void)
{
	u32 i, mask_ingress;
	rtk_fid_t fid;
	rtk_portmask_t mask_member, mask_untag;

	if (!g_vlan_cleared)
		asic_vlan_reset_table();

	mask_ingress = get_phy_ports_mask_lan(1);
#if defined(EXT_PORT_INIC)
	mask_ingress |= (1u << EXT_PORT_INIC);
#endif

	/* fill VLAN table */
#if defined(CONFIG_RTL8367_API_8370)
	fid = 1;
#else
	fid = RTK_IVL_MODE_FID;
#endif
	mask_member.bits[0] = mask_ingress;
	mask_untag.bits[0]  = mask_ingress;
	rtk_vlan_set(1, mask_member, mask_untag, fid);

#if defined(EXT_PORT_INIC)
#if defined(CONFIG_RTL8367_API_8370)
	fid = INIC_GUEST_VLAN_FID;
#else
	fid = RTK_IVL_MODE_FID;
#endif
	mask_member.bits[0] = ((1u << EXT_PORT_INIC) | (1u << LAN_PORT_CPU));
	mask_untag.bits[0]  = 0;
	rtk_vlan_set(INIC_GUEST_VLAN_VID, mask_member, mask_untag, fid);

	/* set iNIC port accept mask */
	rtk_vlan_portAcceptFrameType_set(EXT_PORT_INIC, ACCEPT_FRAME_TYPE_ALL);
#endif

	/* set ingress filtering */
	asic_vlan_set_ingress_ports(mask_ingress);

	/* set LLLLL ports accept mask */
	for (i = 0; i <= RTK_PHY_ID_MAX; i++)
		rtk_vlan_portAcceptFrameType_set(i, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

	/* set CPU port accept mask */
	rtk_vlan_portAcceptFrameType_set(LAN_PORT_CPU, ACCEPT_FRAME_TYPE_ALL);
}

static void asic_vlan_bridge_isolate(u32 wan_bridge_mode, int bridge_changed, int vlan_rule_changed)
{
	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
	{
		if (!bridge_changed)
			return;
		
		asic_vlan_init_vid1();
	}
	else
	{
		if (!bridge_changed && !vlan_rule_changed)
			return;
		
		asic_vlan_apply_rules(wan_bridge_mode);
	}

	asic_clear_mac_table();
}

static void asic_led_mode(rtk_led_group_t group, u32 led_mode)
{
	rtk_led_congig_t led_config_group = LED_CONFIG_SPD1000ACT;

	switch (led_mode)
	{
	case SWAPI_LED_PHYMODE_1000_ACT:
		led_config_group = LED_CONFIG_SPD1000ACT;
		break;
	case SWAPI_LED_PHYMODE_100_ACT:
		led_config_group = LED_CONFIG_SPD100ACT;
		break;
	case SWAPI_LED_PHYMODE_10_ACT:
		led_config_group = LED_CONFIG_SPD10ACT;
		break;
	case SWAPI_LED_PHYMODE_100_10_ACT:
		led_config_group = LED_CONFIG_SPD10010ACT;
		break;
	case SWAPI_LED_PHYMODE_1000:
		led_config_group = LED_CONFIG_SPD1000;
		break;
	case SWAPI_LED_PHYMODE_100:
		led_config_group = LED_CONFIG_SPD100;
		break;
	case SWAPI_LED_PHYMODE_10:
		led_config_group = LED_CONFIG_SPD10;
		break;
	case SWAPI_LED_LINK_ACT:
		led_config_group = LED_CONFIG_LINK_ACT;
		break;
	case SWAPI_LED_LINK_ACT_RX:
		led_config_group = LED_CONFIG_LINKRX;
		break;
	case SWAPI_LED_LINK_ACT_TX:
		led_config_group = LED_CONFIG_LINKTX;
		break;
	case SWAPI_LED_DUPLEX_COLLISION:
		led_config_group = LED_CONFIG_DUPCOL;
		break;
	case SWAPI_LED_OFF:
		led_config_group = LED_CONFIG_LEDOFF;
		break;
	}

	rtk_led_groupConfig_set(group, led_config_group);
}

static void asic_soft_reset(void)
{
	unsigned long reset_start_time;
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_setAsicReg(RTL8370_REG_CHIP_RESET, RTL8370_CHIP_RST_MASK);
#else
	rtl8367b_setAsicReg(RTL8367B_REG_CHIP_RESET, RTL8367B_CHIP_RST_MASK);
#endif
	/* wait at least 1100ms for asic ready */
	reset_start_time = jiffies;
	while(!time_after(jiffies, reset_start_time + 110*HZ/100))
		msleep(50);
}

static rtk_api_ret_t rtk_port_Enable_set(rtk_port_t port, rtk_enable_t enable_phy)
{
	rtk_api_ret_t retVal;
	rtk_port_phy_data_t data, new_data;
	u32 i_port_speed;

	if (port > RTK_PHY_ID_MAX)
		return RT_ERR_PORT_ID;

	i_port_speed = (g_port_link_mode[port] & 0x0F);
	if (i_port_speed == SWAPI_LINK_SPEED_MODE_FORCE_POWER_OFF)
		enable_phy = DISABLED;

	data = 0;
	if ((retVal = rtk_port_phyReg_get(port, PHY_CONTROL_REG, &data)) != RT_ERR_OK)
		return retVal;

	new_data = data;

	if (enable_phy == DISABLED)
		new_data |=  (1 << 11);
	else
		new_data &= ~(1 << 11);

	if (new_data != data) {
		if (!(new_data & (1 << 11)) && (new_data & (1 << 12)))
			new_data |= (1 << 9); // restart AN
		if ((retVal = rtk_port_phyReg_set(port, PHY_CONTROL_REG, new_data)) != RT_ERR_OK)
			return retVal;
	}

	g_port_phy_power[port] = (enable_phy == DISABLED) ? 0 : 1;

	return RT_ERR_OK;
}

static rtk_api_ret_t asic_status_link_port(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus)
{
	u32 regData = 0;

#if defined(CONFIG_RTL8367_API_8370)
	rtk_api_ret_t retVal = rtl8370_getAsicReg(RTL8370_REG_PORT0_STATUS + port, &regData);
#else
	rtk_api_ret_t retVal = rtl8367b_getAsicReg(RTL8367B_REG_PORT0_STATUS + port, &regData);
#endif
	if (retVal != RT_ERR_OK)
		return retVal;

#if defined(CONFIG_RTL8367_API_8370)
	if (regData & RTL8370_PORT0_STATUS_LINK_STATE_MASK)
#else
	if (regData & RTL8367B_PORT0_STATUS_LINK_STATE_MASK)
#endif
	{
		*pLinkStatus = 1;
	}
	else
	{
		*pLinkStatus = 0;
	}

	return RT_ERR_OK;
}

static rtk_api_ret_t asic_status_speed_port(rtk_port_t port, u32 *port_status)
{
	u32 regData = 0;
#if defined(CONFIG_RTL8367_API_8370)
	rtk_api_ret_t retVal = rtl8370_getAsicReg(RTL8370_REG_PORT0_STATUS + port, &regData);
#else
	rtk_api_ret_t retVal = rtl8367b_getAsicReg(RTL8367B_REG_PORT0_STATUS + port, &regData);
#endif
	if (retVal != RT_ERR_OK)
		return retVal;

	*port_status = 0;

#if defined(CONFIG_RTL8367_API_8370)
	if (regData & RTL8370_PORT0_STATUS_LINK_STATE_MASK)
#else
	if (regData & RTL8367B_PORT0_STATUS_LINK_STATE_MASK)
#endif
	{
		*port_status |= (1 << 16);
#if defined(CONFIG_RTL8367_API_8370)
		*port_status |= (regData & RTL8370_PORT0_STATUS_LINK_SPEED_MASK);
		if (regData & RTL8370_PORT0_STATUS_FULL_DUPLUX_CAP_MASK)
			*port_status |= (1 << 8);
		if (regData & RTL8370_PORT0_STATUS_TX_FLOWCTRL_CAP_MASK)
			*port_status |= (1 << 9);
		if (regData & RTL8370_PORT0_STATUS_RX_FLOWCTRL_CAP_MASK)
			*port_status |= (1 << 10);
#else
		*port_status |= (regData & RTL8367B_PORT0_STATUS_LINK_SPEED_MASK);
		if (regData & RTL8367B_PORT0_STATUS_FULL_DUPLUX_CAP_MASK)
			*port_status |= (1 << 8);
		if (regData & RTL8367B_PORT0_STATUS_TX_FLOWCTRL_CAP_MASK)
			*port_status |= (1 << 9);
		if (regData & RTL8367B_PORT0_STATUS_RX_FLOWCTRL_CAP_MASK)
			*port_status |= (1 << 10);
#endif
	}

	return RT_ERR_OK;
}

static rtk_api_ret_t asic_status_link_ports(int is_wan, rtk_port_linkStatus_t *pLinkStatus)
{
	int i;
	rtk_api_ret_t retVal;
	u32 portmask;

	if (is_wan)
		portmask = get_phy_ports_mask_wan(0);
	else
		portmask = get_phy_ports_mask_lan(0);

	for (i = 0; i <= RTK_PHY_ID_MAX; i++)
	{
		if ((portmask >> i) & 0x1)
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

static u32 asic_status_link_changed(void)
{
	u32 int_mask = 0;

#if defined(CONFIG_RTL8367_API_8370)
	if (rtl8370_getAsicInterruptStatus(&int_mask) == RT_ERR_OK)
	{
		if (int_mask & (1 << INT_TYPE_LINK_STATUS))
		{
			rtl8370_setAsicInterruptStatus(1 << INT_TYPE_LINK_STATUS);
			return 1;
		}
	}
#else
	if (rtl8367b_getAsicInterruptStatus(&int_mask) == RT_ERR_OK)
	{
		if (int_mask & (1 << INT_TYPE_LINK_STATUS))
		{
			rtl8367b_setAsicInterruptStatus(1 << INT_TYPE_LINK_STATUS);
			return 1;
		}
	}
#endif
	return 0;
}

static int asic_status_port_bytes(u32 port_mask, port_bytes_t *pb)
{
	rtk_api_ret_t retVal;
	rtk_port_t port = get_port_from_user(port_mask);

	if (port >= RTK_MAX_NUM_OF_PORT)
		return -EINVAL;

	retVal = rtk_stat_port_get(port, STAT_IfInOctets, &pb->RX);
	if (retVal != RT_ERR_OK)
		return -EIO;

	retVal = rtk_stat_port_get(port, STAT_IfOutOctets, &pb->TX);
	if (retVal != RT_ERR_OK)
		return -EIO;

	return 0;
}

static void change_ports_power(u32 power_on, u32 ports_mask)
{
	int i;
	rtk_enable_t is_enable = (power_on) ? ENABLED : DISABLED;

	ports_mask = get_ports_mask_from_user(ports_mask & 0xFF);

	for (i = 0; i <= RTK_PHY_ID_MAX; i++)
	{
		if ((ports_mask >> i) & 0x1)
			rtk_port_Enable_set(i, is_enable);
	}
}

static int change_wan_lan_ports_power(u32 power_on, u32 is_wan)
{
	int i, power_changed = 0;
	u32 ports_mask;
	rtk_enable_t is_enable = (power_on) ? ENABLED : DISABLED;

	if (is_wan)
		ports_mask = get_phy_ports_mask_wan(0);
	else
		ports_mask = get_phy_ports_mask_lan(0);

	for (i = 0; i <= RTK_PHY_ID_MAX; i++)
	{
		if (((ports_mask >> i) & 0x1) && (g_port_phy_power[i] ^ power_on)) {
			power_changed = 1;
			rtk_port_Enable_set(i, is_enable);
		}
	}

	return power_changed;
}

static int change_bridge_mode(u32 isolated_mode, u32 wan_bridge_mode)
{
	int i, bridge_changed, br_iso_changed, vlan_rule_changed, power_changed;

	if (wan_bridge_mode > SWAPI_WAN_BRIDGE_DISABLE_WAN)
		return -EINVAL;

	if (isolated_mode > SWAPI_WAN_BWAN_ISOLATION_BETWEEN)
		return -EINVAL;

	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE)
		isolated_mode = SWAPI_WAN_BWAN_ISOLATION_NONE;

	bridge_changed = (g_wan_bridge_mode != wan_bridge_mode) ? 1 : 0;
	br_iso_changed = (g_wan_bridge_isolated_mode != isolated_mode) ? 1 : 0;
	vlan_rule_changed = 0;
	for (i = 0; i <= SWAPI_VLAN_RULE_WAN_LAN4; i++)
	{
		if (g_vlan_rule[i] != g_vlan_rule_user[i])
		{
			vlan_rule_changed = 1;
			break;
		}
	}

	// set global bridge_mode first
	g_wan_bridge_mode = wan_bridge_mode;
	g_wan_bridge_isolated_mode = isolated_mode;

	power_changed = 0;
	if (bridge_changed || vlan_rule_changed)
	{
		power_changed = change_wan_lan_ports_power(0, 1);
		if (power_changed) {
			// wait for PHY link down
			msleep(1000);
		}
	}

	if (bridge_changed || br_iso_changed)
	{
		asic_bridge_isolate(wan_bridge_mode, isolated_mode);
	}

	asic_vlan_bridge_isolate(wan_bridge_mode, bridge_changed, vlan_rule_changed);

	if (power_changed)
		change_wan_lan_ports_power(1, 1);

	return 0;
}

static int change_led_mode_group0(u32 led_mode)
{
	if (led_mode > SWAPI_LED_OFF)
		return -EINVAL;

	if (g_led_phy_mode_group0 != led_mode)
	{
		asic_led_mode(LED_GROUP_0, led_mode);

		g_led_phy_mode_group0 = led_mode;
	}

	return 0;
}

static int change_led_mode_group1(u32 led_mode)
{
	if (led_mode > SWAPI_LED_OFF)
		return -EINVAL;

	if (g_led_phy_mode_group1 != led_mode)
	{
		asic_led_mode(LED_GROUP_1, led_mode);

		g_led_phy_mode_group1 = led_mode;
	}

	return 0;
}

static int change_led_mode_group2(u32 led_mode)
{
	if (led_mode > SWAPI_LED_OFF)
		return -EINVAL;

	if (g_led_phy_mode_group2 != led_mode)
	{
		asic_led_mode(LED_GROUP_2, led_mode);

		g_led_phy_mode_group2 = led_mode;
	}

	return 0;
}

static void change_port_link_mode(rtk_port_t port, u32 port_link_mode)
{
	u32 i_port_speed, i_port_flowc, i_port_power;
	rtk_api_ret_t retVal;
	rtk_port_phy_ability_t phy_cfg;
	const char *link_desc = "Auto", *flow_desc = "ON";
	const char *port_desc;

	if (g_port_link_mode[port] == port_link_mode)
		return;

	i_port_speed =  (port_link_mode & 0x0F);
	i_port_flowc = ((port_link_mode >> 8) & 0x03);
	i_port_power = (i_port_speed == SWAPI_LINK_SPEED_MODE_FORCE_POWER_OFF) ? 0 : 1;

	phy_cfg.FC			 = 1; //  Symmetric Flow Control
	phy_cfg.AsyFC			 = 0; // Asymmetric Flow Control (only for 1Gbps)
	phy_cfg.Full_1000		 = 1;
	phy_cfg.Full_100		 = 1;
	phy_cfg.Half_100		 = 1;
	phy_cfg.Full_10			 = 1;
	phy_cfg.Half_10			 = 1;
	phy_cfg.AutoNegotiation		 = 1;

	switch (i_port_speed)
	{
	case SWAPI_LINK_SPEED_MODE_AUTO_1000_FD:
		phy_cfg.Full_100	 = 0;
		phy_cfg.Half_100	 = 0;
		phy_cfg.Full_10		 = 0;
		phy_cfg.Half_10		 = 0;
		link_desc		 = "1000FD [AN]";
		break;
	case SWAPI_LINK_SPEED_MODE_AUTO_100_FD:
		phy_cfg.Full_1000	 = 0;
		phy_cfg.Half_100	 = 0;
		phy_cfg.Full_10		 = 0;
		phy_cfg.Half_10		 = 0;
		link_desc		 = "100FD [AN]";
		break;
	case SWAPI_LINK_SPEED_MODE_AUTO_100_HD:
		phy_cfg.Full_1000	 = 0;
		phy_cfg.Full_100	 = 0;
		phy_cfg.Full_10		 = 0;
		phy_cfg.Half_10		 = 0;
		link_desc		 = "100HD [AN]";
		break;
	case SWAPI_LINK_SPEED_MODE_AUTO_10_FD:
		phy_cfg.Full_1000	 = 0;
		phy_cfg.Full_100	 = 0;
		phy_cfg.Half_100	 = 0;
		phy_cfg.Half_10		 = 0;
		link_desc		 = "10FD [AN]";
		break;
	case SWAPI_LINK_SPEED_MODE_AUTO_10_HD:
		phy_cfg.Full_1000	 = 0;
		phy_cfg.Full_100	 = 0;
		phy_cfg.Half_100	 = 0;
		phy_cfg.Full_10		 = 0;
		link_desc		 = "10HD [AN]";
		break;
	case SWAPI_LINK_SPEED_MODE_FORCE_100_FD:
		phy_cfg.Full_1000	 = 0;
		phy_cfg.Half_100	 = 0;
		phy_cfg.Full_10		 = 0;
		phy_cfg.Half_10		 = 0;
		phy_cfg.AutoNegotiation	 = 0;
		link_desc		 = "100FD [Force]";
		break;
	case SWAPI_LINK_SPEED_MODE_FORCE_100_HD:
		phy_cfg.Full_1000	 = 0;
		phy_cfg.Full_100	 = 0;
		phy_cfg.Full_10		 = 0;
		phy_cfg.Half_10		 = 0;
		phy_cfg.AutoNegotiation	 = 0;
		link_desc		 = "100HD [Force]";
		break;
	case SWAPI_LINK_SPEED_MODE_FORCE_10_FD:
		phy_cfg.Full_1000	 = 0;
		phy_cfg.Full_100	 = 0;
		phy_cfg.Half_100	 = 0;
		phy_cfg.Half_10		 = 0;
		phy_cfg.AutoNegotiation	 = 0;
		link_desc		 = "10FD [Force]";
		break;
	case SWAPI_LINK_SPEED_MODE_FORCE_10_HD:
		phy_cfg.Full_1000	 = 0;
		phy_cfg.Full_100	 = 0;
		phy_cfg.Half_100	 = 0;
		phy_cfg.Full_10		 = 0;
		phy_cfg.AutoNegotiation	 = 0;
		link_desc		 = "10HD [Force]";
		break;
	}

	switch (i_port_flowc)
	{
	case SWAPI_LINK_FLOW_CONTROL_TX_ASYNC:
		phy_cfg.FC		 = 0;
		phy_cfg.AsyFC		 = (phy_cfg.Full_1000) ? 1 : 0;
		flow_desc		 = (phy_cfg.Full_1000) ? "TX Asy" : "OFF";
		break;
	case SWAPI_LINK_FLOW_CONTROL_DISABLE:
		phy_cfg.FC		 = 0;
		phy_cfg.AsyFC		 = 0;
		flow_desc		 = "OFF";
		break;
	}

	port_desc = get_port_desc(port);

	if (!i_port_power)
	{
		link_desc = "Power OFF";
		flow_desc = "N/A";
		retVal = rtk_port_Enable_set(port, DISABLED);
	}
	else
	{
		/* RTL8367 not support force link mode for 1000FD */
		if (phy_cfg.AutoNegotiation || phy_cfg.Full_1000)
			retVal = rtk_port_phyAutoNegoAbility_set(port, &phy_cfg);
		else
			retVal = rtk_port_phyForceModeAbility_set(port, &phy_cfg);
	}

	if (retVal != RT_ERR_OK)
	{
		printk("%s - %s, %s FAILED!\n", RTL8367_DEVNAME, port_desc, "phy ability set" );
		return;
	}

	g_port_link_mode[port] = port_link_mode;

	printk("%s - %s link speed: %s, flow control: %s\n", RTL8367_DEVNAME, port_desc, link_desc, flow_desc);
}

static void change_jumbo_frames_accept(u32 jumbo_frames_enabled)
{
	if (jumbo_frames_enabled) jumbo_frames_enabled = 1;

	if (g_jumbo_frames_enabled != jumbo_frames_enabled)
	{
		g_jumbo_frames_enabled = jumbo_frames_enabled;
		printk("%s - jumbo frames accept: %s bytes\n", RTL8367_DEVNAME, (jumbo_frames_enabled) ? "16000" : "1536");
		
		rtk_switch_maxPktLen_set( (jumbo_frames_enabled) ? MAXPKTLEN_16000B : MAXPKTLEN_1536B );
	}
}

static void change_green_ethernet_mode(u32 green_ethernet_enabled)
{
	if (green_ethernet_enabled) green_ethernet_enabled = 1;

	if (g_green_ethernet_enabled != green_ethernet_enabled)
	{
		g_green_ethernet_enabled = green_ethernet_enabled;
		printk("%s - green ethernet: %s\n", RTL8367_DEVNAME, (green_ethernet_enabled) ? "on" : "off");
		
		rtk_switch_greenEthernet_set(green_ethernet_enabled);
	}
}

static int change_storm_control_unicast_unknown(u32 control_rate_mbps)
{
	u32 i;
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_unicast_unknown != control_rate_mbps)
	{
		g_storm_rate_unicast_unknown = control_rate_mbps;
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > MAX_STORM_RATE_VAL) rate_kbps = MAX_STORM_RATE_VAL;
		printk("%s - set %s storm control rate as: %d kbps\n", RTL8367_DEVNAME, "unknown unicast", rate_kbps);
		
		for (i = 0; i <= RTK_PHY_ID_MAX; i++)
			rtk_storm_controlRate_set(i, STORM_GROUP_UNKNOWN_UNICAST, rate_kbps, 1, MODE0);
	}

	return 0;
}

static int change_storm_control_multicast_unknown(u32 control_rate_mbps)
{
	u32 i;
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_multicast_unknown != control_rate_mbps)
	{
		g_storm_rate_multicast_unknown = control_rate_mbps;
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > MAX_STORM_RATE_VAL) rate_kbps = MAX_STORM_RATE_VAL;
		printk("%s - set %s storm control rate as: %d kbps\n", RTL8367_DEVNAME, "unknown multicast", rate_kbps);
		
		for (i = 0; i <= RTK_PHY_ID_MAX; i++)
			rtk_storm_controlRate_set(i, STORM_GROUP_UNKNOWN_MULTICAST, rate_kbps, 1, MODE0);
	}

	return 0;
}

static int change_storm_control_multicast(u32 control_rate_mbps)
{
	u32 i;
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_multicast != control_rate_mbps)
	{
		g_storm_rate_multicast = control_rate_mbps;
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > MAX_STORM_RATE_VAL) rate_kbps = MAX_STORM_RATE_VAL;
		printk("%s - set %s storm control rate as: %d kbps\n", RTL8367_DEVNAME, "multicast", rate_kbps);
		
		for (i = 0; i <= RTK_PHY_ID_MAX; i++)
			rtk_storm_controlRate_set(i, STORM_GROUP_MULTICAST, rate_kbps, 1, MODE0);
	}

	return 0;
}

static int change_storm_control_broadcast(u32 control_rate_mbps)
{
	u32 i;
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_broadcast != control_rate_mbps)
	{
		g_storm_rate_broadcast = control_rate_mbps;
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > MAX_STORM_RATE_VAL) rate_kbps = MAX_STORM_RATE_VAL;
		printk("%s - set %s storm control rate as: %d kbps\n", RTL8367_DEVNAME, "broadcast", rate_kbps);
		
		for (i = 0; i <= RTK_PHY_ID_MAX; i++)
			rtk_storm_controlRate_set(i, STORM_GROUP_BROADCAST, rate_kbps, 1, MODE0);
	}

	return 0;
}

static int change_cpu_rgmii_delay_tx(u32 rgmii_delay_tx)
{
	if (rgmii_delay_tx > 1)
		return -EINVAL;

	if (g_rgmii_delay_tx != rgmii_delay_tx)
	{
		g_rgmii_delay_tx = rgmii_delay_tx;
		printk("%s - set rgmii delay tx: %d\n", RTL8367_DEVNAME, rgmii_delay_tx);
#if defined(CONFIG_RTL8367_API_8370)
#if !defined(CONFIG_RTL8367_ASIC_R)
		rtk_port_rgmiiDelayExt1_set(g_rgmii_delay_tx, g_rgmii_delay_rx);
#endif
#if !defined(RTL8367_SINGLE_EXTIF) || defined(CONFIG_RTL8367_ASIC_R) || \
     defined(CONFIG_RTL8367_LAN_CPU_EXT0) || defined(EXT_PORT_INIC)
		rtk_port_rgmiiDelayExt0_set(g_rgmii_delay_tx, g_rgmii_delay_rx);
#endif
#else
		rtk_port_rgmiiDelayExt_set(LAN_EXT_ID, g_rgmii_delay_tx, g_rgmii_delay_rx);
#if !defined(RTL8367_SINGLE_EXTIF) || defined(EXT_PORT_INIC)
		rtk_port_rgmiiDelayExt_set(WAN_EXT_ID, g_rgmii_delay_tx, g_rgmii_delay_rx);
#endif
#endif
	}

	return 0;
}

static int change_cpu_rgmii_delay_rx(u32 rgmii_delay_rx)
{
	if (rgmii_delay_rx > 7)
		return -EINVAL;

	if (g_rgmii_delay_rx != rgmii_delay_rx)
	{
		g_rgmii_delay_rx = rgmii_delay_rx;
		printk("%s - set rgmii delay rx: %d\n", RTL8367_DEVNAME, rgmii_delay_rx);
#if defined(CONFIG_RTL8367_API_8370)
#if !defined(CONFIG_RTL8367_ASIC_R)
		rtk_port_rgmiiDelayExt1_set(g_rgmii_delay_tx, g_rgmii_delay_rx);
#endif
#if !defined(RTL8367_SINGLE_EXTIF) || defined(CONFIG_RTL8367_ASIC_R) || \
     defined(CONFIG_RTL8367_LAN_CPU_EXT0) || defined(EXT_PORT_INIC)
		rtk_port_rgmiiDelayExt0_set(g_rgmii_delay_tx, g_rgmii_delay_rx);
#endif
#else
		rtk_port_rgmiiDelayExt_set(LAN_EXT_ID, g_rgmii_delay_tx, g_rgmii_delay_rx);
#if !defined(RTL8367_SINGLE_EXTIF) || defined(EXT_PORT_INIC)
		rtk_port_rgmiiDelayExt_set(WAN_EXT_ID, g_rgmii_delay_tx, g_rgmii_delay_rx);
#endif
#endif
	}

	return 0;
}

static int change_vlan_rule(u32 vlan_rule_id, u32 vlan_rule)
{
	if (vlan_rule_id > SWAPI_VLAN_RULE_WAN_LAN4)
		return -EINVAL;

	if ((vlan_rule & 0xFFFF) > 4094)
		return -EINVAL;

	if (((vlan_rule >> 16) & 0xFF) > 7)
		return -EINVAL;

	g_vlan_rule_user[vlan_rule_id] = vlan_rule;

	return 0;
}

static void reset_params_default(void)
{
	int i;
	for (i=0; i <= RTK_PHY_ID_MAX; i++) {
		g_port_link_mode[i] = RTL8367_DEFAULT_LINK_MODE;
		g_port_phy_power[i] = 1;
	}

	g_storm_rate_unicast_unknown    = RTL8367_DEFAULT_STORM_RATE;
	g_storm_rate_multicast_unknown  = RTL8367_DEFAULT_STORM_RATE;
	g_storm_rate_multicast          = RTL8367_DEFAULT_STORM_RATE;
	g_storm_rate_broadcast          = RTL8367_DEFAULT_STORM_RATE;

	g_jumbo_frames_enabled          = RTL8367_DEFAULT_JUMBO_FRAMES;
	g_green_ethernet_enabled        = RTL8367_DEFAULT_GREEN_ETHERNET;

	g_led_phy_mode_group0           = SWAPI_LED_PHYMODE_100_10_ACT;
	g_led_phy_mode_group1           = SWAPI_LED_PHYMODE_1000_ACT;
	g_led_phy_mode_group2           = SWAPI_LED_OFF;

	g_rgmii_delay_tx                = CONFIG_RTL8367_RGMII_DELAY_TX;
	g_rgmii_delay_rx                = CONFIG_RTL8367_RGMII_DELAY_RX;
}

static void reset_and_init_switch(int first_call)
{
	rtk_api_ret_t retVal;
	rtk_portmask_t portmask;
	rtk_mode_ext_t mac_mode;
	rtk_port_mac_ability_t mac_cfg;
	u32 i, ports_mask_wan, ports_mask_lan;

	if (!first_call)
		printk("%s - perform software reset asic!\n", RTL8367_DEVNAME);

	reset_params_default();

	ports_mask_wan = get_phy_ports_mask_wan(0);
	ports_mask_lan = get_phy_ports_mask_lan(0);

	/* soft reset switch */
	asic_soft_reset();

	/* init switch */
	retVal = rtk_switch_init();
	if (retVal != RT_ERR_OK)
		printk("rtk_switch_init() FAILED! (code %d)\n", retVal);

	if (first_call) {
		/* disable link for all PHY ports (please enable from user-level) */
		for (i = 0; i <= RTK_PHY_ID_MAX; i++)
			rtk_port_Enable_set(i, DISABLED);
	}

	/* configure ExtIf */
#if defined (CONFIG_GE1_RGMII_FORCE_100)
	mac_mode		= MODE_EXT_MII_MAC;
	mac_cfg.speed		= SPD_100M;
#else
	mac_mode		= MODE_EXT_RGMII;
	mac_cfg.speed		= SPD_1000M;
#endif
	mac_cfg.forcemode	= MAC_FORCE;
	mac_cfg.duplex		= FULL_DUPLEX;
	mac_cfg.link		= PORT_LINKUP;
	mac_cfg.nway		= DISABLED;
	mac_cfg.rxpause		= ENABLED;
	mac_cfg.txpause		= ENABLED;

#if defined(CONFIG_RTL8367_API_8370)
#if !defined(CONFIG_RTL8367_ASIC_R)
	rtk_port_macForceLinkExt1_set(mac_mode, &mac_cfg);
	rtk_port_rgmiiDelayExt1_set(g_rgmii_delay_tx, g_rgmii_delay_rx);
#endif
#if !defined(RTL8367_SINGLE_EXTIF) || defined(CONFIG_RTL8367_ASIC_R) || \
     defined(CONFIG_RTL8367_LAN_CPU_EXT0) || defined(EXT_PORT_INIC)
#if defined(EXT_PORT_INIC)
	/* do not uplink iNIC port early */
	mac_cfg.link = (g_port_link_inic) ? PORT_LINKUP : PORT_LINKDOWN;
#endif
	rtk_port_macForceLinkExt0_set(mac_mode, &mac_cfg);
	rtk_port_rgmiiDelayExt0_set(g_rgmii_delay_tx, g_rgmii_delay_rx);
#endif
#else
	rtk_port_macForceLinkExt_set(LAN_EXT_ID, mac_mode, &mac_cfg);
	rtk_port_rgmiiDelayExt_set(LAN_EXT_ID, g_rgmii_delay_tx, g_rgmii_delay_rx);
#if !defined(RTL8367_SINGLE_EXTIF) || defined(EXT_PORT_INIC)
#if defined(EXT_PORT_INIC)
	/* do not uplink iNIC port early */
	mac_cfg.link = (g_port_link_inic) ? PORT_LINKUP : PORT_LINKDOWN;
#endif
	rtk_port_macForceLinkExt_set(WAN_EXT_ID, mac_mode, &mac_cfg);
	rtk_port_rgmiiDelayExt_set(WAN_EXT_ID, g_rgmii_delay_tx, g_rgmii_delay_rx);
#endif
#endif
	/* enable all PHY (if disabled by bootstrap) */
	rtk_port_phyEnableAll_set(ENABLED);

	/* configure bridge isolation mode */
	asic_bridge_isolate(g_wan_bridge_mode, g_wan_bridge_isolated_mode);

	/* configure bridge isolation mode via VLAN */
	asic_vlan_bridge_isolate(g_wan_bridge_mode, 1, 1);

	/* configure leds */
	portmask.bits[0] = ports_mask_wan | ports_mask_lan;
	rtk_led_enable_set(LED_GROUP_0, portmask);
	rtk_led_enable_set(LED_GROUP_1, portmask);
	rtk_led_operation_set(LED_OP_PARALLEL);
	asic_led_mode(LED_GROUP_0, g_led_phy_mode_group0);	// group 0 - 8P8C usually green LED
	asic_led_mode(LED_GROUP_1, g_led_phy_mode_group1);	// group 1 - 8P8C usually yellow LED
	asic_led_mode(LED_GROUP_2, g_led_phy_mode_group2);
}

#if defined(EXT_PORT_INIC)
int rtl8367_get_traffic_port_inic(struct rtnl_link_stats64 *stats)
{
	rtk_api_ret_t retVal;

	retVal = rtk_stat_port_get(EXT_PORT_INIC, STAT_IfInOctets, &stats->rx_bytes);
	if (retVal == RT_ERR_OK) {
		retVal = rtk_stat_port_get(EXT_PORT_INIC, STAT_IfInUcastPkts, &stats->rx_packets);
		if (retVal == RT_ERR_OK) {
			stats->rx_packets += stats->multicast;
			stats->rx_bytes -= (stats->rx_packets * 4); // cut VLAN
		}
	}
	retVal = rtk_stat_port_get(EXT_PORT_INIC, STAT_IfOutOctets, &stats->tx_bytes);
	if (retVal == RT_ERR_OK) {
		retVal = rtk_stat_port_get(EXT_PORT_INIC, STAT_IfOutUcastPkts, &stats->tx_packets);
		if (retVal == RT_ERR_OK)
			stats->tx_bytes -= (stats->tx_packets * 4); // cut VLAN
	}

	return 0;
}
EXPORT_SYMBOL(rtl8367_get_traffic_port_inic);
#endif

#if defined(RTL8367_SINGLE_EXTIF) || !defined(CONFIG_RAETH_GMAC2)
int rtl8367_get_traffic_port_wan(struct rtnl_link_stats64 *stats)
{
	rtk_api_ret_t retVal;

	retVal = rtk_stat_port_get(WAN_PORT_X, STAT_IfInOctets, &stats->rx_bytes);
	if (retVal == RT_ERR_OK) {
		retVal = rtk_stat_port_get(WAN_PORT_X, STAT_IfInUcastPkts, &stats->rx_packets);
		if (retVal == RT_ERR_OK) {
			stats->rx_packets += stats->multicast;
		}
	}
	retVal = rtk_stat_port_get(WAN_PORT_X, STAT_IfOutOctets, &stats->tx_bytes);
	if (retVal == RT_ERR_OK) {
		retVal = rtk_stat_port_get(WAN_PORT_X, STAT_IfOutUcastPkts, &stats->tx_packets);
		if (retVal == RT_ERR_OK) {
		}
	}

	return 0;
}
EXPORT_SYMBOL(rtl8367_get_traffic_port_wan);
#endif


#include "rtl8367_ioctl.c"

static void fill_bridge_members(void)
{
	memset(g_bwan_member, 0, sizeof(g_bwan_member));

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1][LAN_PORT_1].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1][LAN_PORT_1].rule = SWAPI_VLAN_RULE_WAN_LAN1;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN2][LAN_PORT_2].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN2][LAN_PORT_2].rule = SWAPI_VLAN_RULE_WAN_LAN2;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3][LAN_PORT_3].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3][LAN_PORT_3].rule = SWAPI_VLAN_RULE_WAN_LAN3;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN4][LAN_PORT_4].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN4][LAN_PORT_4].rule = SWAPI_VLAN_RULE_WAN_LAN4;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3_LAN4][LAN_PORT_3].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3_LAN4][LAN_PORT_3].rule = SWAPI_VLAN_RULE_WAN_LAN3;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3_LAN4][LAN_PORT_4].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN3_LAN4][LAN_PORT_4].rule = SWAPI_VLAN_RULE_WAN_LAN4;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2][LAN_PORT_1].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2][LAN_PORT_1].rule = SWAPI_VLAN_RULE_WAN_LAN1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2][LAN_PORT_2].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2][LAN_PORT_2].rule = SWAPI_VLAN_RULE_WAN_LAN2;

	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_1].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_1].rule = SWAPI_VLAN_RULE_WAN_LAN1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_2].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_2].rule = SWAPI_VLAN_RULE_WAN_LAN2;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_3].bwan = 1;
	g_bwan_member[SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3][LAN_PORT_3].rule = SWAPI_VLAN_RULE_WAN_LAN3;
}

int __init rtl8367_init(void)
{
	int r;

	printk("Realtek RTL8367 GigaPHY Switch Driver %s.\n", RTL8367_VERSION);

#if defined(CONFIG_RTL8367_IGMP_SNOOPING)
	igmp_init();
#endif
#if defined(CONFIG_RTL8367_CIF_MDIO)
	mdio_init(MDIO_RTL8367_PHYID);
#else
	smi_init(SMI_RALINK_GPIO_SDA, SMI_RALINK_GPIO_SCK);
#endif
	fill_bridge_members();

	r = register_chrdev(RTL8367_DEVMAJOR, RTL8367_DEVNAME, &rtl8367_fops);
	if (r < 0) {
		printk(KERN_ERR RTL8367_DEVNAME ": unable to register character device\n");
		return r;
	}

	mutex_lock(&asic_access_mutex);
	reset_and_init_switch(1);
	mutex_unlock(&asic_access_mutex);

	return 0;
}

void __exit rtl8367_exit(void)
{
	unregister_chrdev(RTL8367_DEVMAJOR, RTL8367_DEVNAME);

#if defined(CONFIG_RTL8367_IGMP_SNOOPING)
	igmp_uninit();
#endif
}

module_init(rtl8367_init);
module_exit(rtl8367_exit);

MODULE_DESCRIPTION("Realtek RTL8367 GigaPHY Switch");
MODULE_LICENSE("GPL");

