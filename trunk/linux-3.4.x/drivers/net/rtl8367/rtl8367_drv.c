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

#include "ralink_gpp.h"
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
#define MAX_STORM_RATE_VAL			RTK_MAX_INPUT_RATE
#else
#include "api_8367b/rtk_types.h"
#include "api_8367b/rtk_error.h"
#include "api_8367b/rtk_api_ext.h"
#include "api_8367b/rtl8367b_asicdrv_port.h"
#include "api_8367b/rtl8367b_asicdrv_vlan.h"
#include "api_8367b/rtl8367b_asicdrv_green.h"
#include "api_8367b/rtl8367b_asicdrv_interrupt.h"
#define MAX_STORM_RATE_VAL			RTL8367B_QOS_RATE_INPUT_MAX
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

static u32 g_rgmii_delay_tx                      = CONFIG_RTL8367_RGMII_DELAY_TX;	/* 0..1 */
static u32 g_rgmii_delay_rx                      = CONFIG_RTL8367_RGMII_DELAY_RX;	/* 0..7 */

static u32 g_vlan_cleared                        = 0;
static u32 g_vlan_rule[6]                        = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static u32 g_vlan_rule_user[6]                   = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

////////////////////////////////////////////////////////////////////////////////////

#ifdef RTL8367_DBG
void asic_dump_bridge(void)
{
	int i;
	rtk_api_ret_t retVal;
	rtk_data_t Efid;
	rtk_vlan_t Pvid;
	rtk_pri_t Priority;
	rtk_fid_t Fid;
	rtk_portmask_t mask1, mask2;
	rtk_enable_t Igr_filter;
	rtk_vlan_acceptFrameType_t Accept_frame_type;

	printk("-----------%s: dump bridge isolation----------\n", RTL8367_DEVNAME);

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
	{
		rtk_port_efid_get(i, &Efid);
		retVal = rtk_port_isolation_get(i, &mask1);
		if (retVal == RT_ERR_OK)
			printk("port (%d) isolation: mask=%04X, efid=%d\n", i, mask1.bits[0], Efid);
	}

	printk("------------%s: dump vlan isolation-----------\n", RTL8367_DEVNAME);

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
	{
		retVal = rtk_vlan_portPvid_get(i, &Pvid, &Priority);
		if (retVal == RT_ERR_OK) 
			printk("port (%d) vlan: pvid=%d, prio=%d\n", i, Pvid, Priority);
	}

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
	{
		rtk_vlan_portIgrFilterEnable_get(i, &Igr_filter);
		retVal = rtk_vlan_portAcceptFrameType_get(i, &Accept_frame_type);
		if (retVal == RT_ERR_OK)
			printk("port (%d) accept: type=%d, ingress=%d\n", i, Accept_frame_type, Igr_filter);
	}

	for (i = 1; i < 10; i++)
	{
		retVal = rtk_vlan_get(i, &mask1, &mask2, &Fid);
		if (retVal == RT_ERR_OK) 
			printk("vlan (%d): member=%04X, untag=%04X, fid=%d\n", i, mask1.bits[0], mask2.bits[0], Fid);
	}
}
#endif

u32 get_phy_ports_mask_from_user(u32 user_port_mask)
{
	u32 phy_ports_mask = 0;

	if (user_port_mask & SWAPI_PORTMASK_LAN1)
		phy_ports_mask |= (1L << LAN_PORT_1);
	if (user_port_mask & SWAPI_PORTMASK_LAN2)
		phy_ports_mask |= (1L << LAN_PORT_2);
	if (user_port_mask & SWAPI_PORTMASK_LAN3)
		phy_ports_mask |= (1L << LAN_PORT_3);
	if (user_port_mask & SWAPI_PORTMASK_LAN4)
		phy_ports_mask |= (1L << LAN_PORT_4);
	if (user_port_mask & SWAPI_PORTMASK_WAN)
		phy_ports_mask |= (1L << WAN_PORT_X);
	if (user_port_mask & SWAPI_PORTMASK_CPU_LAN)
		phy_ports_mask |= (1L << LAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
	if (user_port_mask & SWAPI_PORTMASK_CPU_WAN)
		phy_ports_mask |= (1L << WAN_PORT_CPU);
#endif
#if defined(EXT_PORT_INIC)
	if (user_port_mask & SWAPI_PORTMASK_INIC)
		phy_ports_mask |= (1L << EXT_PORT_INIC);
#endif
	return phy_ports_mask;
}

u32 get_phy_ports_mask_lan(u32 include_cpu)
{
	unsigned int portmask_lan = ((1L << LAN_PORT_4) | (1L << LAN_PORT_3) | (1L << LAN_PORT_2) | (1L << LAN_PORT_1));
	if (include_cpu)
		portmask_lan |= (1L << LAN_PORT_CPU);

	switch (g_wan_bridge_mode)
	{
	case SWAPI_WAN_BRIDGE_LAN1:
		portmask_lan &= ~(1L << LAN_PORT_1);
		break;
	case SWAPI_WAN_BRIDGE_LAN2:
		portmask_lan &= ~(1L << LAN_PORT_2);
		break;
	case SWAPI_WAN_BRIDGE_LAN3:
		portmask_lan &= ~(1L << LAN_PORT_3);
		break;
	case SWAPI_WAN_BRIDGE_LAN4:
		portmask_lan &= ~(1L << LAN_PORT_4);
		break;
	case SWAPI_WAN_BRIDGE_LAN3_LAN4:
		portmask_lan &= ~((1L << LAN_PORT_3) | (1L << LAN_PORT_4));
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2:
		portmask_lan &= ~((1L << LAN_PORT_1) | (1L << LAN_PORT_2));
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		portmask_lan &= ~((1L << LAN_PORT_1) | (1L << LAN_PORT_2) | (1L << LAN_PORT_3));
		break;
	case SWAPI_WAN_BRIDGE_DISABLE_WAN:
		portmask_lan |= (1L << WAN_PORT_X);
		break;
	}

	return portmask_lan;
}

u32 get_phy_ports_mask_wan(u32 include_cpu)
{
	unsigned int portmask_wan = (1L << WAN_PORT_X);
	if (include_cpu)
		portmask_wan |= (1L << WAN_PORT_CPU);

	switch (g_wan_bridge_mode)
	{
	case SWAPI_WAN_BRIDGE_LAN1:
		portmask_wan |= (1L << LAN_PORT_1);
		break;
	case SWAPI_WAN_BRIDGE_LAN2:
		portmask_wan |= (1L << LAN_PORT_2);
		break;
	case SWAPI_WAN_BRIDGE_LAN3:
		portmask_wan |= (1L << LAN_PORT_3);
		break;
	case SWAPI_WAN_BRIDGE_LAN4:
		portmask_wan |= (1L << LAN_PORT_4);
		break;
	case SWAPI_WAN_BRIDGE_LAN3_LAN4:
		portmask_wan |= ((1L << LAN_PORT_3) | (1L << LAN_PORT_4));
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2:
		portmask_wan |= ((1L << LAN_PORT_1) | (1L << LAN_PORT_2));
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		portmask_wan |= ((1L << LAN_PORT_1) | (1L << LAN_PORT_2) | (1L << LAN_PORT_3));
		break;
	case SWAPI_WAN_BRIDGE_DISABLE_WAN:
		portmask_wan = 0;
		break;
	}

	return portmask_wan;
}

void asic_bridge_isolate(u32 wan_bridge_mode, u32 bwan_isolated_mode)
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
	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
	{
		if ((fwd_mask_lan.bits[0] >> i) & 0x1)
		{
			fwd_mask.bits[0] = fwd_mask_lan.bits[0];
			
#if defined(EXT_PORT_INIC)
			if (i == LAN_PORT_CPU)
			{
				/* force add iNIC port to forward from CPU */
				fwd_mask.bits[0] |= (1L << EXT_PORT_INIC);
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
	fwd_mask.bits[0] = ((1L << LAN_PORT_CPU) | (1L << EXT_PORT_INIC));
	rtk_port_isolation_set(EXT_PORT_INIC, fwd_mask);
#endif

	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
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
#if defined(RTL8367_SINGLE_EXTIF)
			if (i == WAN_PORT_CPU)
			{
				/* force add all LAN ports to forward from CPU */
				fwd_mask.bits[0] |= ((1L << LAN_PORT_4) | (1L << LAN_PORT_3) | (1L << LAN_PORT_2) | (1L << LAN_PORT_1));
#if defined(EXT_PORT_INIC)
				/* force add iNIC port to forward from CPU */
				fwd_mask.bits[0] |= (1L << EXT_PORT_INIC);
#endif
			}
#endif
			if (bwan_isolated_mode == SWAPI_WAN_BWAN_ISOLATION_FROM_CPU)
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
			else if (bwan_isolated_mode == SWAPI_WAN_BWAN_ISOLATION_BETWEEN)
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
#if !defined(RTL8367_SINGLE_EXTIF)
			rtk_port_efid_set(i, 1);
#endif
		}
	}
}

#if defined(EXT_PORT_INIC)
void toggle_isolation_inic(u32 inic_isolated)
{
	int i;
	rtk_portmask_t fwd_mask, fwd_mask_lan;

	printk("%s - iNIC isolation: %d\n", RTL8367_DEVNAME, inic_isolated);

	fwd_mask_lan.bits[0] = get_phy_ports_mask_lan(0);

	for (i = 0; i <= RTK_PHY_ID_MAX; i++)
	{
		if ((fwd_mask_lan.bits[0] >> i) & 0x1)
		{
			fwd_mask.bits[0] = (fwd_mask_lan.bits[0] | (1L << LAN_PORT_CPU));
			if (!inic_isolated)
				fwd_mask.bits[0] |= (1L << EXT_PORT_INIC);
			
			rtk_port_isolation_set(i, fwd_mask);
		}
	}

	fwd_mask.bits[0] = ((1L << LAN_PORT_CPU) | (1L << EXT_PORT_INIC));
	if (!inic_isolated)
		fwd_mask.bits[0] |= fwd_mask_lan.bits[0];
	rtk_port_isolation_set(EXT_PORT_INIC, fwd_mask);
}
#endif


void asic_vlan_set_ingress_ports(u32 reg_ingress)
{
#if defined(CONFIG_RTL8367_API_8370)
	rtl8370_setAsicReg(RTL8370_REG_VLAN_INGRESS, reg_ingress);
#else
	rtl8367b_setAsicReg(RTL8367B_REG_VLAN_INGRESS, reg_ingress);
#endif
}

void asic_vlan_ingress_mode_enabled(u32 port_mask)
{
	u32 reg_ingress = get_phy_ports_mask_from_user(port_mask & 0xFF);

	asic_vlan_set_ingress_ports(reg_ingress);
}

void asic_vlan_accept_port_mode(u32 accept_mode, u32 port_mask)
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

	port_mask = get_phy_ports_mask_from_user(port_mask & 0xFF);

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
	mask_member.bits[0] = get_phy_ports_mask_from_user((vlan4k_mask & 0xFF));
	mask_untag.bits[0]  = get_phy_ports_mask_from_user((vlan4k_mask >> 16) & 0xFF);

	rtk_vlan_set(pvid, mask_member, mask_untag, fid);

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
	{
		if ((mask_untag.bits[0] >> i) & 0x1)
			rtk_vlan_portPvid_set(i, pvid, prio);
	}

	g_vlan_cleared = 0;

	printk("%s - create vlan port: pvid=[%d], prio=[%d], member=[0x%04X], untag=[0x%04X], fid=[%d]\n",
			RTL8367_DEVNAME, pvid, prio, mask_member.bits[0], mask_untag.bits[0], fid);
}

void asic_vlan_create_entry(u32 vlan4k_info, u32 vlan4k_mask)
{
	rtk_vlan_t vid;
	rtk_fid_t fid;
	rtk_portmask_t mask_member, mask_untag;

	vid = (rtk_vlan_t)(vlan4k_info & 0x0FFF);
	fid = (rtk_fid_t)((vlan4k_info >> 16) & 0xFF);
	mask_member.bits[0] = get_phy_ports_mask_from_user((vlan4k_mask & 0xFF));
	mask_untag.bits[0]  = get_phy_ports_mask_from_user((vlan4k_mask >> 16) & 0xFF);

	rtk_vlan_set(vid, mask_member, mask_untag, fid);

	g_vlan_cleared = 0;

	printk("%s - create vlan entry: vid=[%d], member=[0x%04X], untag=[0x%04X], fid=[%d]\n",
			RTL8367_DEVNAME, vid, mask_member.bits[0], mask_untag.bits[0], fid);
}

#if defined(EXT_PORT_INIC)
void init_ralink_iNIC_rule(void)
{
	rtk_filter_cfg_t Fc;
	rtk_filter_action_t Fa;
	rtk_filter_field_t Ff;
	rtk_filter_number_t ruleNum;
	rtk_portmask_t mask_member, mask_untag;

	rtk_filter_igrAcl_init();

	/* VLAN for iNIC boot/heartbeat packets */
	mask_member.bits[0] = (1L << EXT_PORT_INIC) | (1L << LAN_PORT_CPU);
	mask_untag.bits[0]  = (1L << EXT_PORT_INIC) | (1L << LAN_PORT_CPU);
	rtk_vlan_set(INIC_HEART_VLAN_VID, mask_member, mask_untag, 0);

	memset(&Fc, 0, sizeof(Fc));
	memset(&Fa, 0, sizeof(Fa));
	memset(&Ff, 0, sizeof(Ff));

	Ff.fieldType = FILTER_FIELD_ETHERTYPE;
	Ff.filter_pattern_union.etherType.dataType = FILTER_FIELD_DATA_MASK;
	Ff.filter_pattern_union.etherType.value = 0xFFFF; // boot/heartbeat packets use etherType 0xFFFF
	Ff.filter_pattern_union.etherType.mask  = 0xFFFF;

	rtk_filter_igrAcl_field_add(&Fc, &Ff);

	Fc.activeport.dataType = FILTER_FIELD_DATA_MASK;
	Fc.activeport.value = (1L << LAN_PORT_CPU) | (1L << EXT_PORT_INIC);
	Fc.activeport.mask  = 0xFF;

	Fa.actEnable[FILTER_ENACT_INGRESS_CVLAN_VID] = 1;
	Fa.filterIngressCvlanVid = INIC_HEART_VLAN_VID;

	rtk_filter_igrAcl_cfg_add(0, &Fc, &Fa, &ruleNum);
}
#endif

void asic_vlan_reset_table(void)
{
	rtk_portmask_t mask_member, mask_untag;

	/* init VLAN table (VLAN1) and enable VLAN */
	rtk_vlan_init();

	/* clear VLAN2 from previous config */
	mask_member.bits[0] = 0;
	mask_untag.bits[0]  = 0;
	rtk_vlan_set(2, mask_member, mask_untag, 0);
#if defined(EXT_PORT_INIC)
	/* clear VLAN3 for iNIC Guest AP */
	if (g_wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		rtk_vlan_set(INIC_GUEST_VLAN_VID, mask_member, mask_untag, 0);
#endif
	g_vlan_cleared = 1;

#if defined(EXT_PORT_INIC)
	/* configure Acl */
	init_ralink_iNIC_rule();
#endif
}

void asic_vlan_apply_rules(u32 wan_bridge_mode)
{
	rtk_vlan_t pvid[6];
	rtk_pri_t prio[6];
	u32 tagg[6];
	int i, port_combine, cpu_inet_combined, cpu_iptv_combined;
	u32 accept_tagged, exclude_wan_vid, include_wan_id2;
	rtk_fid_t next_fid;
	rtk_portmask_t mask_member, mask_untag;

	next_fid = 3;
	cpu_inet_combined = 0;
	cpu_iptv_combined = 0;
	accept_tagged = 0;
	include_wan_id2 = 0;
	exclude_wan_vid = 0;

	for (i = 0; i <= SWAPI_VLAN_RULE_WAN_LAN4; i++)
	{
		pvid[i] =  (g_vlan_rule_user[i] & 0xFFF);
		prio[i] = ((g_vlan_rule_user[i] >> 16) & 0x7);
		tagg[i] = ((g_vlan_rule_user[i] >> 24) & 0x1);
	}

	if (!g_vlan_cleared)
		asic_vlan_reset_table();

	/* VLAN VID1 for LAN */
	mask_member.bits[0] = get_phy_ports_mask_lan(1);
#if defined(EXT_PORT_INIC)
	mask_member.bits[0] |= (1L << EXT_PORT_INIC);
#endif
	mask_untag.bits[0]   = mask_member.bits[0];
#if defined(RTL8367_SINGLE_EXTIF)
	mask_untag.bits[0]  &= ~(1L << LAN_PORT_CPU);
#endif
	rtk_vlan_set(1, mask_member, mask_untag, 1);
	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
	{
		if ((mask_untag.bits[0] >> i) & 0x1)
			rtk_vlan_portPvid_set(i, 1, 0);
	}

#if defined(RTL8367_SINGLE_EXTIF)
	accept_tagged |= (1L << LAN_PORT_CPU);
#if defined(EXT_PORT_INIC)
	accept_tagged |= (1L << EXT_PORT_INIC);
	next_fid = INIC_GUEST_FID + 1;

	/* VLAN3 for iNIC guest AP */
	mask_member.bits[0] = (1L << EXT_PORT_INIC) | (1L << LAN_PORT_CPU);
	mask_untag.bits[0]  = 0;
	rtk_vlan_set(INIC_GUEST_VLAN_VID, mask_member, mask_untag, INIC_GUEST_FID);
#endif
#endif

	if (pvid[SWAPI_VLAN_RULE_WAN_INET] < MIN_EXT_VLAN_VID)
	{
		pvid[SWAPI_VLAN_RULE_WAN_INET] = 2; // VID 2
		prio[SWAPI_VLAN_RULE_WAN_INET] = 0;
		tagg[SWAPI_VLAN_RULE_WAN_INET] = 0;
	}

	switch (wan_bridge_mode)
	{
	case SWAPI_WAN_BRIDGE_LAN1:
		include_wan_id2 |= (1L << LAN_PORT_1);
		if (pvid[SWAPI_VLAN_RULE_WAN_LAN1] >= MIN_EXT_VLAN_VID)
		{
			mask_member.bits[0] = (1L << LAN_PORT_1) | (1L << WAN_PORT_X);
			mask_untag.bits[0]  = (!tagg[SWAPI_VLAN_RULE_WAN_LAN1]) ? (1L << LAN_PORT_1) : 0;
			accept_tagged      |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN1]) ? (1L << LAN_PORT_1) : 0;
			accept_tagged      |= (1L << WAN_PORT_X);
			exclude_wan_vid    |= (1L << LAN_PORT_1);
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN1] == pvid[SWAPI_VLAN_RULE_WAN_INET])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
				mask_untag.bits[0]  |= (1L << WAN_PORT_CPU);
#endif
				cpu_inet_combined = 1;
			}
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN1] == pvid[SWAPI_VLAN_RULE_WAN_IPTV])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
				cpu_iptv_combined = 1;
			}
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_LAN1], mask_member, mask_untag, next_fid++);
			rtk_vlan_portPvid_set(LAN_PORT_1, pvid[SWAPI_VLAN_RULE_WAN_LAN1], prio[SWAPI_VLAN_RULE_WAN_LAN1]);
		}
		break;
	case SWAPI_WAN_BRIDGE_LAN2:
		include_wan_id2 |= (1L << LAN_PORT_2);
		if (pvid[SWAPI_VLAN_RULE_WAN_LAN2] >= MIN_EXT_VLAN_VID)
		{
			mask_member.bits[0] = (1L << LAN_PORT_2) | (1L << WAN_PORT_X);
			mask_untag.bits[0]  = (!tagg[SWAPI_VLAN_RULE_WAN_LAN2]) ? (1L << LAN_PORT_2) : 0;
			accept_tagged      |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN2]) ? (1L << LAN_PORT_2) : 0;
			accept_tagged      |= (1L << WAN_PORT_X);
			exclude_wan_vid    |= (1L << LAN_PORT_2);
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN2] == pvid[SWAPI_VLAN_RULE_WAN_INET])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
				mask_untag.bits[0]  |= (1L << WAN_PORT_CPU);
#endif
				cpu_inet_combined = 1;
			}
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN2] == pvid[SWAPI_VLAN_RULE_WAN_IPTV])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
				cpu_iptv_combined = 1;
			}
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_LAN2], mask_member, mask_untag, next_fid++);
			rtk_vlan_portPvid_set(LAN_PORT_2, pvid[SWAPI_VLAN_RULE_WAN_LAN2], prio[SWAPI_VLAN_RULE_WAN_LAN2]);
		}
		break;
	case SWAPI_WAN_BRIDGE_LAN3:
		include_wan_id2 |= (1L << LAN_PORT_3);
		if (pvid[SWAPI_VLAN_RULE_WAN_LAN3] >= MIN_EXT_VLAN_VID)
		{
			mask_member.bits[0] = (1L << LAN_PORT_3) | (1L << WAN_PORT_X);
			mask_untag.bits[0]  = (!tagg[SWAPI_VLAN_RULE_WAN_LAN3]) ? (1L << LAN_PORT_3) : 0;
			accept_tagged      |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN3]) ? (1L << LAN_PORT_3) : 0;
			accept_tagged      |= (1L << WAN_PORT_X);
			exclude_wan_vid    |= (1L << LAN_PORT_3);
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN3] == pvid[SWAPI_VLAN_RULE_WAN_INET])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
				mask_untag.bits[0]  |= (1L << WAN_PORT_CPU);
#endif
				cpu_inet_combined = 1;
			}
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN3] == pvid[SWAPI_VLAN_RULE_WAN_IPTV])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
				cpu_iptv_combined = 1;
			}
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_LAN3], mask_member, mask_untag, next_fid++);
			rtk_vlan_portPvid_set(LAN_PORT_3, pvid[SWAPI_VLAN_RULE_WAN_LAN3], prio[SWAPI_VLAN_RULE_WAN_LAN3]);
		}
		break;
	case SWAPI_WAN_BRIDGE_LAN4:
		include_wan_id2 |= (1L << LAN_PORT_4);
		if (pvid[SWAPI_VLAN_RULE_WAN_LAN4] >= MIN_EXT_VLAN_VID)
		{
			mask_member.bits[0] = (1L << LAN_PORT_4) | (1L << WAN_PORT_X);
			mask_untag.bits[0]  = (!tagg[SWAPI_VLAN_RULE_WAN_LAN4]) ? (1L << LAN_PORT_4) : 0;
			accept_tagged      |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN4]) ? (1L << LAN_PORT_4) : 0;
			accept_tagged      |= (1L << WAN_PORT_X);
			exclude_wan_vid    |= (1L << LAN_PORT_4);
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN4] == pvid[SWAPI_VLAN_RULE_WAN_INET])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
				mask_untag.bits[0]  |= (1L << WAN_PORT_CPU);
#endif
				cpu_inet_combined = 1;
			}
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN4] == pvid[SWAPI_VLAN_RULE_WAN_IPTV])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
				cpu_iptv_combined = 1;
			}
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_LAN4], mask_member, mask_untag, next_fid++);
			rtk_vlan_portPvid_set(LAN_PORT_4, pvid[SWAPI_VLAN_RULE_WAN_LAN4], prio[SWAPI_VLAN_RULE_WAN_LAN4]);
		}
		break;
	case SWAPI_WAN_BRIDGE_LAN3_LAN4:
		include_wan_id2 |= ((1L << LAN_PORT_3) | (1L << LAN_PORT_4));
		if (pvid[SWAPI_VLAN_RULE_WAN_LAN3] >= MIN_EXT_VLAN_VID)
		{
			mask_member.bits[0] = (1L << LAN_PORT_3) | (1L << WAN_PORT_X);
			mask_untag.bits[0]  = (!tagg[SWAPI_VLAN_RULE_WAN_LAN3]) ? (1L << LAN_PORT_3) : 0;
			accept_tagged      |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN3]) ? (1L << LAN_PORT_3) : 0;
			accept_tagged      |= (1L << WAN_PORT_X);
			exclude_wan_vid    |= (1L << LAN_PORT_3);
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN3] == pvid[SWAPI_VLAN_RULE_WAN_INET])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
				mask_untag.bits[0]  |= (1L << WAN_PORT_CPU);
#endif
				cpu_inet_combined = 1;
			}
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN3] == pvid[SWAPI_VLAN_RULE_WAN_IPTV])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
				cpu_iptv_combined = 1;
			}
			port_combine = 0;
			if(pvid[SWAPI_VLAN_RULE_WAN_LAN4] == pvid[SWAPI_VLAN_RULE_WAN_LAN3])
			{
				mask_member.bits[0] |= (1L << LAN_PORT_4);
				mask_untag.bits[0]  |= (!tagg[SWAPI_VLAN_RULE_WAN_LAN4]) ? (1L << LAN_PORT_4) : 0;
				accept_tagged       |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN4]) ? (1L << LAN_PORT_4) : 0;
				exclude_wan_vid     |= (1L << LAN_PORT_4);
				port_combine        |= 1;
			}
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_LAN3], mask_member, mask_untag, next_fid++);
			rtk_vlan_portPvid_set(LAN_PORT_3, pvid[SWAPI_VLAN_RULE_WAN_LAN3], prio[SWAPI_VLAN_RULE_WAN_LAN3]);
			if(port_combine & 1)
				rtk_vlan_portPvid_set(LAN_PORT_4, pvid[SWAPI_VLAN_RULE_WAN_LAN3], prio[SWAPI_VLAN_RULE_WAN_LAN4]);
		}
		if (pvid[SWAPI_VLAN_RULE_WAN_LAN4] >= MIN_EXT_VLAN_VID &&
		    pvid[SWAPI_VLAN_RULE_WAN_LAN4] != pvid[SWAPI_VLAN_RULE_WAN_LAN3])
		{
			mask_member.bits[0] = (1L << LAN_PORT_4) | (1L << WAN_PORT_X);
			mask_untag.bits[0]  = (!tagg[SWAPI_VLAN_RULE_WAN_LAN4]) ? (1L << LAN_PORT_4) : 0;
			accept_tagged      |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN4]) ? (1L << LAN_PORT_4) : 0;
			accept_tagged      |= (1L << WAN_PORT_X);
			exclude_wan_vid    |= (1L << LAN_PORT_4);
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN4] == pvid[SWAPI_VLAN_RULE_WAN_INET])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
				mask_untag.bits[0]  |= (1L << WAN_PORT_CPU);
#endif
				cpu_inet_combined = 1;
			}
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN4] == pvid[SWAPI_VLAN_RULE_WAN_IPTV])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
				cpu_iptv_combined = 1;
			}
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_LAN4], mask_member, mask_untag, next_fid++);
			rtk_vlan_portPvid_set(LAN_PORT_4, pvid[SWAPI_VLAN_RULE_WAN_LAN4], prio[SWAPI_VLAN_RULE_WAN_LAN4]);
		}
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2:
		include_wan_id2 |= ((1L << LAN_PORT_1) | (1L << LAN_PORT_2));
		if (pvid[SWAPI_VLAN_RULE_WAN_LAN1] >= MIN_EXT_VLAN_VID)
		{
			mask_member.bits[0] = (1L << LAN_PORT_1) | (1L << WAN_PORT_X);
			mask_untag.bits[0]  = (!tagg[SWAPI_VLAN_RULE_WAN_LAN1]) ? (1L << LAN_PORT_1) : 0;
			accept_tagged      |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN1]) ? (1L << LAN_PORT_1) : 0;
			accept_tagged      |= (1L << WAN_PORT_X);
			exclude_wan_vid    |= (1L << LAN_PORT_1);
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN1] == pvid[SWAPI_VLAN_RULE_WAN_INET])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
				mask_untag.bits[0]  |= (1L << WAN_PORT_CPU);
#endif
				cpu_inet_combined = 1;
			}
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN1] == pvid[SWAPI_VLAN_RULE_WAN_IPTV])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
				cpu_iptv_combined = 1;
			}
			port_combine = 0;
			if(pvid[SWAPI_VLAN_RULE_WAN_LAN2] == pvid[SWAPI_VLAN_RULE_WAN_LAN1])
			{
				mask_member.bits[0] |= (1L << LAN_PORT_2);
				mask_untag.bits[0]  |= (!tagg[SWAPI_VLAN_RULE_WAN_LAN2]) ? (1L << LAN_PORT_2) : 0;
				accept_tagged       |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN2]) ? (1L << LAN_PORT_2) : 0;
				exclude_wan_vid     |= (1L << LAN_PORT_2);
				port_combine        |= 1;
			}
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_LAN1], mask_member, mask_untag, next_fid++);
			rtk_vlan_portPvid_set(LAN_PORT_1, pvid[SWAPI_VLAN_RULE_WAN_LAN1], prio[SWAPI_VLAN_RULE_WAN_LAN1]);
			if(port_combine & 1)
				rtk_vlan_portPvid_set(LAN_PORT_2, pvid[SWAPI_VLAN_RULE_WAN_LAN1], prio[SWAPI_VLAN_RULE_WAN_LAN2]);
		}
		if (pvid[SWAPI_VLAN_RULE_WAN_LAN2] >= MIN_EXT_VLAN_VID &&
		    pvid[SWAPI_VLAN_RULE_WAN_LAN2] != pvid[SWAPI_VLAN_RULE_WAN_LAN1])
		{
			mask_member.bits[0] = (1L << LAN_PORT_2) | (1L << WAN_PORT_X);
			mask_untag.bits[0]  = (!tagg[SWAPI_VLAN_RULE_WAN_LAN2]) ? (1L << LAN_PORT_2) : 0;
			accept_tagged      |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN2]) ? (1L << LAN_PORT_2) : 0;
			accept_tagged      |= (1L << WAN_PORT_X);
			exclude_wan_vid    |= (1L << LAN_PORT_2);
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN2] == pvid[SWAPI_VLAN_RULE_WAN_INET])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
				mask_untag.bits[0]  |= (1L << WAN_PORT_CPU);
#endif
				cpu_inet_combined = 1;
			}
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN2] == pvid[SWAPI_VLAN_RULE_WAN_IPTV])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
				cpu_iptv_combined = 1;
			}
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_LAN2], mask_member, mask_untag, next_fid++);
			rtk_vlan_portPvid_set(LAN_PORT_2, pvid[SWAPI_VLAN_RULE_WAN_LAN2], prio[SWAPI_VLAN_RULE_WAN_LAN2]);
		}
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		include_wan_id2 |= ((1L << LAN_PORT_1) | (1L << LAN_PORT_2) | (1L << LAN_PORT_3));
		if (pvid[SWAPI_VLAN_RULE_WAN_LAN1] >= MIN_EXT_VLAN_VID)
		{
			mask_member.bits[0] = (1L << LAN_PORT_1) | (1L << WAN_PORT_X);
			mask_untag.bits[0]  = (!tagg[SWAPI_VLAN_RULE_WAN_LAN1]) ? (1L << LAN_PORT_1) : 0;
			accept_tagged      |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN1]) ? (1L << LAN_PORT_1) : 0;
			accept_tagged      |= (1L << WAN_PORT_X);
			exclude_wan_vid    |= (1L << LAN_PORT_1);
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN1] == pvid[SWAPI_VLAN_RULE_WAN_INET])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
				mask_untag.bits[0]  |= (1L << WAN_PORT_CPU);
#endif
				cpu_inet_combined = 1;
			}
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN1] == pvid[SWAPI_VLAN_RULE_WAN_IPTV])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
				cpu_iptv_combined = 1;
			}
			port_combine = 0;
			if(pvid[SWAPI_VLAN_RULE_WAN_LAN2] == pvid[SWAPI_VLAN_RULE_WAN_LAN1])
			{
				mask_member.bits[0] |= (1L << LAN_PORT_2);
				mask_untag.bits[0]  |= (!tagg[SWAPI_VLAN_RULE_WAN_LAN2]) ? (1L << LAN_PORT_2) : 0;
				accept_tagged       |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN2]) ? (1L << LAN_PORT_2) : 0;
				exclude_wan_vid     |= (1L << LAN_PORT_2);
				port_combine        |= 1;
			}
			if(pvid[SWAPI_VLAN_RULE_WAN_LAN3] == pvid[SWAPI_VLAN_RULE_WAN_LAN1])
			{
				mask_member.bits[0] |= (1L << LAN_PORT_3);
				mask_untag.bits[0]  |= (!tagg[SWAPI_VLAN_RULE_WAN_LAN3]) ? (1L << LAN_PORT_3) : 0;
				accept_tagged       |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN3]) ? (1L << LAN_PORT_3) : 0;
				exclude_wan_vid     |= (1L << LAN_PORT_3);
				port_combine        |= 2;
			}
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_LAN1], mask_member, mask_untag, next_fid++);
			rtk_vlan_portPvid_set(LAN_PORT_1, pvid[SWAPI_VLAN_RULE_WAN_LAN1], prio[SWAPI_VLAN_RULE_WAN_LAN1]);
			if(port_combine & 1)
				rtk_vlan_portPvid_set(LAN_PORT_2, pvid[SWAPI_VLAN_RULE_WAN_LAN1], prio[SWAPI_VLAN_RULE_WAN_LAN2]);
			if(port_combine & 2)
				rtk_vlan_portPvid_set(LAN_PORT_3, pvid[SWAPI_VLAN_RULE_WAN_LAN1], prio[SWAPI_VLAN_RULE_WAN_LAN3]);
		}
		if (pvid[SWAPI_VLAN_RULE_WAN_LAN2] >= MIN_EXT_VLAN_VID &&
		    pvid[SWAPI_VLAN_RULE_WAN_LAN2] != pvid[SWAPI_VLAN_RULE_WAN_LAN1])
		{
			mask_member.bits[0] = (1L << LAN_PORT_2) | (1L << WAN_PORT_X);
			mask_untag.bits[0]  = (!tagg[SWAPI_VLAN_RULE_WAN_LAN2]) ? (1L << LAN_PORT_2) : 0;
			accept_tagged      |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN2]) ? (1L << LAN_PORT_2) : 0;
			accept_tagged      |= (1L << WAN_PORT_X);
			exclude_wan_vid    |= (1L << LAN_PORT_2);
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN2] == pvid[SWAPI_VLAN_RULE_WAN_INET])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
				mask_untag.bits[0]  |= (1L << WAN_PORT_CPU);
#endif
				cpu_inet_combined = 1;
			}
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN2] == pvid[SWAPI_VLAN_RULE_WAN_IPTV])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
				cpu_iptv_combined = 1;
			}
			port_combine = 0;
			if(pvid[SWAPI_VLAN_RULE_WAN_LAN3] == pvid[SWAPI_VLAN_RULE_WAN_LAN2] &&
			   pvid[SWAPI_VLAN_RULE_WAN_LAN3] != pvid[SWAPI_VLAN_RULE_WAN_LAN1])
			{
				mask_member.bits[0] |= (1L << LAN_PORT_3);
				mask_untag.bits[0]  |= (!tagg[SWAPI_VLAN_RULE_WAN_LAN3]) ? (1L << LAN_PORT_3) : 0;
				accept_tagged       |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN3]) ? (1L << LAN_PORT_3) : 0;
				exclude_wan_vid     |= (1L << LAN_PORT_3);
				port_combine        |= 1;
			}
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_LAN2], mask_member, mask_untag, next_fid++);
			rtk_vlan_portPvid_set(LAN_PORT_2, pvid[SWAPI_VLAN_RULE_WAN_LAN2], prio[SWAPI_VLAN_RULE_WAN_LAN2]);
			if(port_combine & 1)
				rtk_vlan_portPvid_set(LAN_PORT_3, pvid[SWAPI_VLAN_RULE_WAN_LAN2], prio[SWAPI_VLAN_RULE_WAN_LAN3]);
		}
		if (pvid[SWAPI_VLAN_RULE_WAN_LAN3] >= MIN_EXT_VLAN_VID &&
		    pvid[SWAPI_VLAN_RULE_WAN_LAN3] != pvid[SWAPI_VLAN_RULE_WAN_LAN1] &&
		    pvid[SWAPI_VLAN_RULE_WAN_LAN3] != pvid[SWAPI_VLAN_RULE_WAN_LAN2])
		{
			mask_member.bits[0] = (1L << LAN_PORT_3) | (1L << WAN_PORT_X);
			mask_untag.bits[0]  = (!tagg[SWAPI_VLAN_RULE_WAN_LAN3]) ? (1L << LAN_PORT_3) : 0;
			accept_tagged      |=  (tagg[SWAPI_VLAN_RULE_WAN_LAN3]) ? (1L << LAN_PORT_3) : 0;
			accept_tagged      |= (1L << WAN_PORT_X);
			exclude_wan_vid    |= (1L << LAN_PORT_3);
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN3] == pvid[SWAPI_VLAN_RULE_WAN_INET])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
#if !defined(RTL8367_SINGLE_EXTIF)
				mask_untag.bits[0]  |= (1L << WAN_PORT_CPU);
#endif
				cpu_inet_combined = 1;
			}
			if (pvid[SWAPI_VLAN_RULE_WAN_LAN3] == pvid[SWAPI_VLAN_RULE_WAN_IPTV])
			{
				mask_member.bits[0] |= (1L << WAN_PORT_CPU);
				cpu_iptv_combined = 1;
			}
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_LAN3], mask_member, mask_untag, next_fid++);
			rtk_vlan_portPvid_set(LAN_PORT_3, pvid[SWAPI_VLAN_RULE_WAN_LAN3], prio[SWAPI_VLAN_RULE_WAN_LAN3]);
		}
		break;
	}

	/* VLAN for WAN IPTV */
	if (pvid[SWAPI_VLAN_RULE_WAN_IPTV] >= MIN_EXT_VLAN_VID)
	{
		accept_tagged |= (1L << WAN_PORT_X) | (1L << WAN_PORT_CPU);
		if (pvid[SWAPI_VLAN_RULE_WAN_IPTV] != pvid[SWAPI_VLAN_RULE_WAN_INET] && !cpu_iptv_combined)
		{
			mask_member.bits[0] = (1L << WAN_PORT_X) | (1L << WAN_PORT_CPU);
			mask_untag.bits[0]  = 0;
			rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_IPTV], mask_member, mask_untag, 2);
		}
	}

	/* always create port VID 2 for untagged + WAN bridged LANx ports w/o VLAN tag */
	include_wan_id2 &= ~exclude_wan_vid;
	if (pvid[SWAPI_VLAN_RULE_WAN_INET] != 2)
	{
		mask_member.bits[0] = include_wan_id2 | (1L << WAN_PORT_X) | (1L << WAN_PORT_CPU);
		mask_untag.bits[0]  = include_wan_id2 | (1L << WAN_PORT_X);
		accept_tagged |= (1L << WAN_PORT_CPU);
		
		rtk_vlan_set(2, mask_member, mask_untag, next_fid++);
		for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
		{
			if ((mask_untag.bits[0] >> i) & 0x1)
				rtk_vlan_portPvid_set(i, 2, 0);
		}
	}

	/* VLAN for WAN INET */
	mask_member.bits[0] = get_phy_ports_mask_wan(1) & ~exclude_wan_vid;
	if (pvid[SWAPI_VLAN_RULE_WAN_INET] != 2 && include_wan_id2)
		mask_member.bits[0] &= ~include_wan_id2;
	mask_untag.bits[0]  = mask_member.bits[0];
#if defined(RTL8367_SINGLE_EXTIF)
	mask_untag.bits[0] &= ~(1L << LAN_PORT_CPU);
#endif
	if (pvid[SWAPI_VLAN_RULE_WAN_INET] >= MIN_EXT_VLAN_VID)
	{
		mask_untag.bits[0] &= ~(1L << WAN_PORT_X);
		accept_tagged      |=  (1L << WAN_PORT_X);
	}
	
	if (!cpu_inet_combined)
		rtk_vlan_set(pvid[SWAPI_VLAN_RULE_WAN_INET], mask_member, mask_untag, 2);
	
	/* force add WAN port to create Port VID (if ID2) */
	if (pvid[SWAPI_VLAN_RULE_WAN_INET] == 2)
		mask_untag.bits[0] |= (1L << WAN_PORT_X);
	else
		mask_untag.bits[0] &= ~(1L << WAN_PORT_X);

	for (i = 0; i < RTK_MAX_NUM_OF_PORT; i++)
	{
		if ((mask_untag.bits[0] >> i) & 0x1)
			rtk_vlan_portPvid_set(i, pvid[SWAPI_VLAN_RULE_WAN_INET], prio[SWAPI_VLAN_RULE_WAN_INET]);
	}

	/* enable ingress filtering for trunk and hybrid ports */
	asic_vlan_set_ingress_ports(accept_tagged);

	/* accept tagged and untagged frames for WAN port and needed LAN ports */
	if (accept_tagged & (1L << WAN_PORT_X))
		rtk_vlan_portAcceptFrameType_set(WAN_PORT_X, ACCEPT_FRAME_TYPE_ALL);
	else
		rtk_vlan_portAcceptFrameType_set(WAN_PORT_X, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

	if (accept_tagged & (1L << LAN_PORT_1))
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_1, ACCEPT_FRAME_TYPE_ALL);
	else
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_1, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

	if (accept_tagged & (1L << LAN_PORT_2))
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_2, ACCEPT_FRAME_TYPE_ALL);
	else
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_2, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

	if (accept_tagged & (1L << LAN_PORT_3))
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_3, ACCEPT_FRAME_TYPE_ALL);
	else
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_3, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

	if (accept_tagged & (1L << LAN_PORT_4))
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_4, ACCEPT_FRAME_TYPE_ALL);
	else
		rtk_vlan_portAcceptFrameType_set(LAN_PORT_4, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

#if defined(RTL8367_SINGLE_EXTIF)
#if defined(EXT_PORT_INIC)
	/* set iNIC port accept mask */
	rtk_vlan_portAcceptFrameType_set(EXT_PORT_INIC, ACCEPT_FRAME_TYPE_ALL);
#endif
	/* set CPU port accept mask (trunk port - accept tag only) */
	rtk_vlan_portAcceptFrameType_set(LAN_PORT_CPU, ACCEPT_FRAME_TYPE_TAG_ONLY);
#else
	/* set CPU WAN port accept mask */
	if (accept_tagged & (1L << WAN_PORT_CPU))
		rtk_vlan_portAcceptFrameType_set(WAN_PORT_CPU, ACCEPT_FRAME_TYPE_ALL);
	else
		rtk_vlan_portAcceptFrameType_set(WAN_PORT_CPU, ACCEPT_FRAME_TYPE_UNTAG_ONLY);

	/* set CPU LAN port accept mask */
	rtk_vlan_portAcceptFrameType_set(LAN_PORT_CPU, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
#endif

	g_vlan_cleared = 0;

	for (i = 0; i <= SWAPI_VLAN_RULE_WAN_LAN4; i++)
		g_vlan_rule[i] = g_vlan_rule_user[i];
}

void asic_vlan_init_vid1(void)
{
	if (!g_vlan_cleared)
		asic_vlan_reset_table();
	
	asic_vlan_set_ingress_ports(1L << LAN_PORT_CPU);
	
	/* set CPU port accept mask */
	rtk_vlan_portAcceptFrameType_set(LAN_PORT_CPU, ACCEPT_FRAME_TYPE_ALL);
	
#if defined(EXT_PORT_INIC)
	/* set iNIC port accept mask */
	rtk_vlan_portAcceptFrameType_set(EXT_PORT_INIC, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
#endif
	/* set LLLLL ports accept mask */
	rtk_vlan_portAcceptFrameType_set(WAN_PORT_X, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
	rtk_vlan_portAcceptFrameType_set(LAN_PORT_4, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
	rtk_vlan_portAcceptFrameType_set(LAN_PORT_3, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
	rtk_vlan_portAcceptFrameType_set(LAN_PORT_2, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
	rtk_vlan_portAcceptFrameType_set(LAN_PORT_1, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
}

void asic_vlan_bridge_isolate(u32 wan_bridge_mode, int bridge_changed, int vlan_rule_changed)
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
}

void asic_led_mode(rtk_led_group_t group, u32 led_mode)
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

void asic_soft_reset(void)
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

rtk_api_ret_t rtk_port_Enable_set(rtk_port_t port, rtk_enable_t enable)
{
	rtk_api_ret_t retVal;
	rtk_port_phy_data_t data, new_data;

	if (port > RTK_PHY_ID_MAX)
		return RT_ERR_PORT_ID;

	data = 0;
	if ((retVal = rtk_port_phyReg_get(port, PHY_CONTROL_REG, &data)) != RT_ERR_OK)
		return retVal;

	new_data = data;

	if (enable == DISABLED)
		new_data |= 0x0800;
	else
		new_data &= ~0x0800;

	if (new_data != data) {
		if ((retVal = rtk_port_phyReg_set(port, PHY_CONTROL_REG, new_data)) != RT_ERR_OK)
			return retVal;
	}

	g_port_phy_power[port] = (enable == DISABLED) ? 0 : 1;

	return RT_ERR_OK;
}

void asic_port_power(u32 port_enabled, u32 port_mask)
{
	int i;
	rtk_enable_t is_enable = (port_enabled) ? ENABLED : DISABLED;

	port_mask = get_phy_ports_mask_from_user(port_mask & 0xFF);

	for (i = 0; i <= RTK_PHY_ID_MAX; i++)
	{
		if ((port_mask >> i) & 0x1)
			rtk_port_Enable_set(i, is_enable);
	}
}

rtk_api_ret_t asic_status_link_port(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus)
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

rtk_api_ret_t asic_status_speed_port(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus, rtk_data_t *pSpeed, rtk_data_t *pDuplex)
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
#if defined(CONFIG_RTL8367_API_8370)
		*pSpeed  = (regData & RTL8370_PORT0_STATUS_LINK_SPEED_MASK);
		*pDuplex = (regData & RTL8370_PORT0_STATUS_FULL_DUPLUX_CAP_MASK) ? 1 : 0;
#else
		*pSpeed  = (regData & RTL8367B_PORT0_STATUS_LINK_SPEED_MASK);
		*pDuplex = (regData & RTL8367B_PORT0_STATUS_FULL_DUPLUX_CAP_MASK) ? 1 : 0;
#endif
	}
	else
	{
		*pLinkStatus = 0;
		*pSpeed = 0;
		*pDuplex = 0;
	}

	return RT_ERR_OK;
}

rtk_api_ret_t asic_status_link_ports(int is_wan, rtk_port_linkStatus_t *pLinkStatus)
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

u32 asic_status_link_changed(void)
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

int change_wan_ports_power(u32 power_on)
{
	int i, power_changed;
	u32 ports_mask_wan;
	rtk_enable_t is_enable = (power_on) ? ENABLED : DISABLED;

	ports_mask_wan = get_phy_ports_mask_wan(0);

	power_changed = 0;
	for (i = 0; i <= RTK_PHY_ID_MAX; i++)
	{
		if (((ports_mask_wan >> i) & 0x1) && (g_port_phy_power[i] ^ power_on)) {
			power_changed = 1;
			rtk_port_Enable_set(i, is_enable);
		}
	}

	return power_changed;
}

int change_bridge_mode(u32 isolated_mode, u32 wan_bridge_mode)
{
	int i, bridge_changed, br_iso_changed, vlan_rule_changed, power_changed;

	if (wan_bridge_mode > SWAPI_WAN_BRIDGE_DISABLE_WAN)
		return -EINVAL;

	if (isolated_mode > SWAPI_WAN_BWAN_ISOLATION_BETWEEN)
		return -EINVAL;

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
		power_changed = change_wan_ports_power(0);
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
		change_wan_ports_power(1);

#ifdef RTL8367_DBG
	asic_dump_bridge();
#endif

	return 0;
}

int change_led_mode_group0(u32 led_mode)
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

int change_led_mode_group1(u32 led_mode)
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

int change_led_mode_group2(u32 led_mode)
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

void change_port_link_mode(rtk_port_t port, u32 port_link_mode)
{
	u32 i_port_speed;
	u32 i_port_flowc;
	rtk_api_ret_t retVal;
	rtk_port_phy_ability_t phy_cfg;

	if (g_port_link_mode[port] == port_link_mode)
		return;

	i_port_speed =  (port_link_mode & 0x07);
	i_port_flowc = ((port_link_mode >> 8) & 0x03);

	printk("%s - port [%d] link speed: %d, flow control: %d\n", RTL8367_DEVNAME, port, i_port_speed, i_port_flowc);

	phy_cfg.FC	 = 1; //  Symmetric Flow Control
	phy_cfg.AsyFC	 = 0; // Asymmetric Flow Control (only for 1Gbps)

	switch (i_port_flowc)
	{
	case SWAPI_LINK_FLOW_CONTROL_RX_ASYNC:
		phy_cfg.FC	 = (i_port_speed != SWAPI_LINK_SPEED_MODE_1000_FD) ? 1 : 0;
		phy_cfg.AsyFC	 = 1;
		break;
	case SWAPI_LINK_FLOW_CONTROL_DISABLE:
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
	case SWAPI_LINK_SPEED_MODE_1000_FD:
		phy_cfg.Full_100  = 0;
		phy_cfg.Half_100  = 0;
		phy_cfg.Full_10   = 0;
		phy_cfg.Half_10   = 0;
		break;
	case SWAPI_LINK_SPEED_MODE_100_FD:
		phy_cfg.Full_1000 = 0;
		phy_cfg.Half_100  = 0;
		phy_cfg.Full_10   = 0;
		phy_cfg.Half_10   = 0;
		break;
	case SWAPI_LINK_SPEED_MODE_100_HD:
		phy_cfg.Full_1000 = 0;
		phy_cfg.Full_100  = 0;
		phy_cfg.Full_10   = 0;
		phy_cfg.Half_10   = 0;
		break;
	case SWAPI_LINK_SPEED_MODE_10_FD:
		phy_cfg.Full_1000 = 0;
		phy_cfg.Full_100  = 0;
		phy_cfg.Half_100  = 0;
		phy_cfg.Half_10   = 0;
		break;
	case SWAPI_LINK_SPEED_MODE_10_HD:
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

void change_jumbo_frames_accept(u32 jumbo_frames_enabled)
{
	if (jumbo_frames_enabled) jumbo_frames_enabled = 1;

	if (g_jumbo_frames_enabled != jumbo_frames_enabled)
	{
		printk("%s - jumbo frames accept: %s bytes\n", RTL8367_DEVNAME, (jumbo_frames_enabled) ? "16000" : "1536");

		rtk_switch_maxPktLen_set( (jumbo_frames_enabled) ? MAXPKTLEN_16000B : MAXPKTLEN_1536B );

		g_jumbo_frames_enabled = jumbo_frames_enabled;
	}
}

void change_green_ethernet_mode(u32 green_ethernet_enabled)
{
	if (green_ethernet_enabled) green_ethernet_enabled = 1;

	if (g_green_ethernet_enabled != green_ethernet_enabled)
	{
		printk("%s - green ethernet: %d\n", RTL8367_DEVNAME, green_ethernet_enabled);

		rtk_switch_greenEthernet_set(green_ethernet_enabled);

		g_green_ethernet_enabled = green_ethernet_enabled;
	}
}

int change_storm_control_unicast_unknown(u32 control_rate_mbps)
{
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_unicast_unknown != control_rate_mbps)
	{
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > MAX_STORM_RATE_VAL) rate_kbps = MAX_STORM_RATE_VAL;

		printk("%s - set unknown unicast storm control rate as: %d kbps\n", RTL8367_DEVNAME, rate_kbps);

		rtk_storm_controlRate_set(LAN_PORT_4, STORM_GROUP_UNKNOWN_UNICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_3, STORM_GROUP_UNKNOWN_UNICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_2, STORM_GROUP_UNKNOWN_UNICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_1, STORM_GROUP_UNKNOWN_UNICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(WAN_PORT_X, STORM_GROUP_UNKNOWN_UNICAST, rate_kbps, 1, MODE0);

		g_storm_rate_unicast_unknown = control_rate_mbps;
	}

	return 0;
}

int change_storm_control_multicast_unknown(u32 control_rate_mbps)
{
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_multicast_unknown != control_rate_mbps)
	{
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > MAX_STORM_RATE_VAL) rate_kbps = MAX_STORM_RATE_VAL;

		printk("%s - set unknown multicast storm control rate as: %d kbps\n", RTL8367_DEVNAME, rate_kbps);

		rtk_storm_controlRate_set(LAN_PORT_4, STORM_GROUP_UNKNOWN_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_3, STORM_GROUP_UNKNOWN_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_2, STORM_GROUP_UNKNOWN_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_1, STORM_GROUP_UNKNOWN_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(WAN_PORT_X, STORM_GROUP_UNKNOWN_MULTICAST, rate_kbps, 1, MODE0);

		g_storm_rate_multicast_unknown = control_rate_mbps;
	}

	return 0;
}

int change_storm_control_multicast(u32 control_rate_mbps)
{
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_multicast != control_rate_mbps)
	{
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > MAX_STORM_RATE_VAL) rate_kbps = MAX_STORM_RATE_VAL;

		printk("%s - set multicast storm control rate as: %d kbps\n", RTL8367_DEVNAME, rate_kbps);

		rtk_storm_controlRate_set(LAN_PORT_4, STORM_GROUP_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_3, STORM_GROUP_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_2, STORM_GROUP_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_1, STORM_GROUP_MULTICAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(WAN_PORT_X, STORM_GROUP_MULTICAST, rate_kbps, 1, MODE0);

		g_storm_rate_multicast = control_rate_mbps;
	}

	return 0;
}

int change_storm_control_broadcast(u32 control_rate_mbps)
{
	rtk_rate_t rate_kbps;

	if ((control_rate_mbps < 1) || (control_rate_mbps > 1024))
		return -EINVAL;

	if (g_storm_rate_broadcast != control_rate_mbps)
	{
		rate_kbps = control_rate_mbps * 1024;
		if (rate_kbps > MAX_STORM_RATE_VAL) rate_kbps = MAX_STORM_RATE_VAL;

		printk("%s - set broadcast storm control rate as: %d kbps\n", RTL8367_DEVNAME, rate_kbps);

		rtk_storm_controlRate_set(LAN_PORT_4, STORM_GROUP_BROADCAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_3, STORM_GROUP_BROADCAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_2, STORM_GROUP_BROADCAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(LAN_PORT_1, STORM_GROUP_BROADCAST, rate_kbps, 1, MODE0);
		rtk_storm_controlRate_set(WAN_PORT_X, STORM_GROUP_BROADCAST, rate_kbps, 1, MODE0);

		g_storm_rate_broadcast = control_rate_mbps;
	}

	return 0;
}

int change_cpu_rgmii_delay_tx(u32 rgmii_delay_tx)
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

int change_cpu_rgmii_delay_rx(u32 rgmii_delay_rx)
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

int change_vlan_rule(u32 vlan_rule_id, u32 vlan_rule)
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

void reset_params_default(void)
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

void reset_and_init_switch(int first_call)
{
	rtk_api_ret_t retVal;
	rtk_portmask_t portmask;
	rtk_mode_ext_t mac_mode;
	rtk_port_mac_ability_t mac_cfg;
	u32 ports_mask_wan, ports_mask_lan;

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
		rtk_port_Enable_set(LAN_PORT_1, 0);
		rtk_port_Enable_set(LAN_PORT_2, 0);
		rtk_port_Enable_set(LAN_PORT_3, 0);
		rtk_port_Enable_set(LAN_PORT_4, 0);
		rtk_port_Enable_set(WAN_PORT_X, 0);
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
	rtk_port_macForceLinkExt0_set(mac_mode, &mac_cfg);
	rtk_port_rgmiiDelayExt0_set(g_rgmii_delay_tx, g_rgmii_delay_rx);
#endif
#else
	rtk_port_macForceLinkExt_set(LAN_EXT_ID, MODE_EXT_RGMII, &mac_cfg);
	rtk_port_rgmiiDelayExt_set(LAN_EXT_ID, g_rgmii_delay_tx, g_rgmii_delay_rx);
#if !defined(RTL8367_SINGLE_EXTIF) || defined(EXT_PORT_INIC)
	rtk_port_macForceLinkExt_set(WAN_EXT_ID, MODE_EXT_RGMII, &mac_cfg);
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
			stats->rx_bytes -= (stats->rx_packets * 8); // cut FCS and VLAN
		}
	}
	retVal = rtk_stat_port_get(EXT_PORT_INIC, STAT_IfOutOctets, &stats->tx_bytes);
	if (retVal == RT_ERR_OK) {
		retVal = rtk_stat_port_get(EXT_PORT_INIC, STAT_IfOutUcastPkts, &stats->tx_packets);
		if (retVal == RT_ERR_OK)
			stats->tx_bytes -= (stats->tx_packets * 8); // cut FCS and VLAN
	}

	return 0;
}
EXPORT_SYMBOL(rtl8367_get_traffic_port_inic);
#endif

#if defined(RTL8367_SINGLE_EXTIF)
int rtl8367_get_traffic_port_wan(struct rtnl_link_stats64 *stats)
{
	rtk_api_ret_t retVal;

	retVal = rtk_stat_port_get(WAN_PORT_X, STAT_IfInOctets, &stats->rx_bytes);
	if (retVal == RT_ERR_OK) {
		retVal = rtk_stat_port_get(WAN_PORT_X, STAT_IfInUcastPkts, &stats->rx_packets);
		if (retVal == RT_ERR_OK) {
			stats->rx_packets += stats->multicast;
			stats->rx_bytes -= (stats->rx_packets * 4); // cut FCS
		}
	}
	retVal = rtk_stat_port_get(WAN_PORT_X, STAT_IfOutOctets, &stats->tx_bytes);
	if (retVal == RT_ERR_OK) {
		retVal = rtk_stat_port_get(WAN_PORT_X, STAT_IfOutUcastPkts, &stats->tx_packets);
		if (retVal == RT_ERR_OK)
			stats->tx_bytes -= (stats->tx_packets * 4); // cut FCS
	}

	return 0;
}
EXPORT_SYMBOL(rtl8367_get_traffic_port_wan);
#endif


#include "rtl8367_ioctl.c"

int __init rtl8367_init(void)
{
	int r;

	mutex_init(&asic_access_mutex);
#if defined(CONFIG_RTL8367_IGMP_SNOOPING)
	igmp_init();
#endif
	gpio_init();
#if defined(CONFIG_RTL8367_CIF_MDIO)
	mdio_init(MDIO_RTL8367_PHYID);
#else
	gpio_smi_init(SMI_RALINK_GPIO_SDA, SMI_RALINK_GPIO_SCK, SMI_RTL8367_DELAY_NS, SMI_RTL8367_SMI_ADDR);
#endif
	r = register_chrdev(RTL8367_DEVMAJOR, RTL8367_DEVNAME, &rtl8367_fops);
	if (r < 0) {
		printk(KERN_ERR RTL8367_DEVNAME ": unable to register character device\n");
		return r;
	}

	mutex_lock(&asic_access_mutex);
	reset_and_init_switch(1);
	mutex_unlock(&asic_access_mutex);

	printk("Realtek RTL8367 GigaPHY Switch Driver %s.\n", RTL8367_VERSION);

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

// for iNIC_mii.ko
EXPORT_SYMBOL(ralink_initGpioPin);
EXPORT_SYMBOL(ralink_gpio_write_bit);

