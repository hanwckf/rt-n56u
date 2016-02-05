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
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/netdevice.h>

#include "../ra_esw_reg.h"
#include "../mii_mgr.h"

#include "../ra_esw_base.h"
#include "../ra_esw_rt305x.h"
#include "ioctl.h"
#include "ioctl_rt305x.h"

////////////////////////////////////////////////////////////////////////////////////

static DEFINE_MUTEX(esw_access_mutex);

static u32 g_wan_bridge_mode                     = SWAPI_WAN_BRIDGE_DISABLE;
static u32 g_wan_bwan_isolation                  = SWAPI_WAN_BWAN_ISOLATION_NONE;

static u32 g_led_phy_mode                        = SWAPI_LED_LINK_ACT;

#if !defined (CONFIG_RALINK_RT3052)
static u32 g_storm_rate_limit                    = ESW_DEFAULT_STORM_RATE;
#endif

#if defined (CONFIG_RALINK_MT7628)
static u32 g_green_ethernet_enabled              = ESW_DEFAULT_GREEN_ETHERNET;
#endif

static u32 g_port_link_mode[ESW_PHY_ID_MAX+1]    = {0, 0, 0, 0, 0};
static u32 g_port_phy_power[ESW_PHY_ID_MAX+1]    = {0, 0, 0, 0, 0};

static u32 g_vlan_rule[SWAPI_VLAN_RULE_NUM]      = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static u32 g_vlan_rule_user[SWAPI_VLAN_RULE_NUM] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

////////////////////////////////////////////////////////////////////////////////////

static atomic_t g_switch_inited                  = ATOMIC_INIT(0);
static atomic_t g_port_link_changed              = ATOMIC_INIT(0);

static bwan_member_t g_bwan_member[SWAPI_WAN_BRIDGE_NUM][ESW_MAC_ID_MAX+1];

static u32 g_vlan_pvid_wan_untagged              = 2;

////////////////////////////////////////////////////////////////////////////////////

const char *g_port_desc_cpu   = "CPU";
const char *g_port_desc_wan   = "WAN";
const char *g_port_desc_lan1  = "LAN1";
const char *g_port_desc_lan2  = "LAN2";
const char *g_port_desc_lan3  = "LAN3";
const char *g_port_desc_lan4  = "LAN4";

////////////////////////////////////////////////////////////////////////////////////

static u32 get_ports_mask_lan(u32 include_cpu)
{
	u32 i, wan_bridge_mode, portmask_lan;

	wan_bridge_mode = g_wan_bridge_mode;

	portmask_lan = ((1u << LAN_PORT_4) | (1u << LAN_PORT_3) | (1u << LAN_PORT_2) | (1u << LAN_PORT_1));
	if (include_cpu)
		portmask_lan |= (1u << LAN_PORT_CPU);

	for (i = 0; i <= ESW_MAC_ID_MAX; i++) {
		if (g_bwan_member[wan_bridge_mode][i].bwan)
			portmask_lan &= ~(1u << i);
	}

	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		portmask_lan |= (1u << WAN_PORT_1);

	return portmask_lan;
}

static u32 get_ports_mask_wan(u32 include_cpu)
{
	u32 i, wan_bridge_mode, portmask_wan;

	wan_bridge_mode = g_wan_bridge_mode;
	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		return 0;

	portmask_wan = (1u << WAN_PORT_1);
	if (include_cpu)
		portmask_wan |= (1u << WAN_PORT_CPU);

	for (i = 0; i <= ESW_MAC_ID_MAX; i++) {
		if (g_bwan_member[wan_bridge_mode][i].bwan)
			portmask_wan |= (1u << i);
	}

	return portmask_wan;
}

static u32 get_ports_mask_from_user(u32 user_port_mask)
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
		gsw_ports_mask |= (1u << WAN_PORT_1);
	if (user_port_mask & SWAPI_PORTMASK_CPU_LAN)
		gsw_ports_mask |= (1u << LAN_PORT_CPU);
	if (user_port_mask & SWAPI_PORTMASK_CPU_WAN)
		gsw_ports_mask |= (1u << WAN_PORT_CPU);

	return gsw_ports_mask;
}

static const char* get_port_desc(u32 port_id)
{
	const char *port_desc;

	switch (port_id)
	{
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
	case WAN_PORT_1:
		port_desc = g_port_desc_wan;
		break;
	case LAN_PORT_CPU:
	default:
		port_desc = g_port_desc_cpu;
		break;
	}

	return port_desc;
}

static void esw_show_bridge_partitions(u32 wan_bridge_mode)
{
	char *wanl, *wanr, *lans;

	wanl = "W|";
	wanr = "";

	switch (wan_bridge_mode)
	{
	case SWAPI_WAN_BRIDGE_LAN1:
		lans = "WLLL";
		break;
	case SWAPI_WAN_BRIDGE_LAN2:
		lans = "LWLL";
		break;
	case SWAPI_WAN_BRIDGE_LAN3:
		lans = "LLWL";
		break;
	case SWAPI_WAN_BRIDGE_LAN4:
		lans = "LLLW";
		break;
	case SWAPI_WAN_BRIDGE_LAN3_LAN4:
		lans = "LLWW";
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2:
		lans = "WWLL";
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		lans = "WWWL";
		break;
	case SWAPI_WAN_BRIDGE_DISABLE_WAN:
		lans = "LLLL";
		wanl = "L";
		wanr = "";
		break;
	default:
		lans = "LLLL";
		break;
	}

	printk("%s - %s: %s%s%s\n", MTK_ESW_DEVNAME, "hw bridge", wanl, lans, wanr);
}

static void esw_port_ingress_filter(u32 port_id, int en_vlan)
{
	u32 reg_pfc1;

	reg_pfc1 = esw_reg_get(REG_ESW_PFC1);

	if (en_vlan)
		reg_pfc1 |=  ((1u << port_id) << 16);
	else
		reg_pfc1 &= ~((1u << port_id) << 16);

	esw_reg_set(REG_ESW_PFC1, reg_pfc1);
}

static void esw_port_ingress_transparent(u32 port_id, int double_tag_en)
{
	u32 reg_sgc2;

	reg_sgc2 = esw_reg_get(REG_ESW_SGC2);

	if (double_tag_en)
		reg_sgc2 |=  (1u << port_id);
	else
		reg_sgc2 &= ~(1u << port_id);

	esw_reg_set(REG_ESW_SGC2, reg_sgc2);
}

static void esw_port_egress_untag(u32 port_id, int untag_en)
{
	u32 reg_poc2;

	reg_poc2 = esw_reg_get(REG_ESW_POC2);
#if !defined (CONFIG_RALINK_RT3052)
	reg_poc2 |=  (1u << 15); // enable per vlan untag
#endif
	if (untag_en)
		reg_poc2 |=  (1u << port_id);
	else
		reg_poc2 &= ~(1u << port_id);

	esw_reg_set(REG_ESW_POC2, reg_poc2);
}

static void esw_vlan_pvid_set(u32 port_id, u32 pvid, u32 prio)
{
	u32 reg_pvid, reg_pfc1, qnum = 1;

	pvid &= 0xfff;
	prio &= 0x7;

	switch (prio)
	{
	case 1:
	case 2:
		qnum = 0;	// BK
		break;
	case 0:
	case 3:
		qnum = 1;	// BE
		break;
	case 4:
	case 5:
		qnum = 2;	// CL
		break;
	case 6:
	case 7:
		qnum = 3;	// VO
		break;
	}

	reg_pvid = esw_reg_get(REG_ESW_PVIDC_BASE + 4*(port_id/2));
	if ((port_id % 2) == 0) {
		reg_pvid &= 0xfffff000;
		reg_pvid |= pvid;
	} else {
		reg_pvid &= 0xff000fff;
		reg_pvid |= (pvid << 12);
	}
	esw_reg_set(REG_ESW_PVIDC_BASE + 4*(port_id/2), reg_pvid);

	reg_pfc1 = esw_reg_get(REG_ESW_PFC1);
	reg_pfc1 &= ~(0x3u << (port_id*2));
	reg_pfc1 |=  (qnum << (port_id*2));
	esw_reg_set(REG_ESW_PFC1, reg_pfc1);
}

static int esw_wait_wt_mac(void)
{
	int i;
	u32 value;

	for (i = 0; i < 200; i++) {
		value = esw_reg_get(REG_ESW_WT_MAC_AD0);
		if (value & 0x2)
			return 0;
		udelay(100);
	}

	return -1;
}

static void esw_mac_table_clear(void)
{
	int i, j;
	u32 value, mac;

	esw_reg_set(REG_ESW_TABLE_SEARCH, 0x1);
	for (i = 0; i < 0x400; i++) {
		for (j = 0; j < 1000; j++) {
			value = esw_reg_get(REG_ESW_TABLE_STATUS0);
			if (value & 0x1) {
				if ((value & 0x70) == 0)
					return;
				mac = esw_reg_get(REG_ESW_TABLE_STATUS2);
				esw_reg_set(REG_ESW_WT_MAC_AD2, mac);
				mac = esw_reg_get(REG_ESW_TABLE_STATUS1);
				esw_reg_set(REG_ESW_WT_MAC_AD1, mac & 0xffff);
				esw_reg_set(REG_ESW_WT_MAC_AD0, (value & 0x780) | 0x01);
				
				esw_wait_wt_mac();
				
				if (value & 0x2) {
					/* end of table */
					return;
				}
				break;
			}
			else if (value & 0x2) {
				/* end of table */
				return;
			}
			udelay(100);
		}
		esw_reg_set(REG_ESW_TABLE_SEARCH, 0x2);
	}
}

static u32 esw_get_vlan_slot(u32 idx)
{
	u32 reg_val;

	reg_val = esw_reg_get(REG_ESW_VLAN_MEMB_BASE + 4*(idx/4));

	if ((idx % 4) == 0)
		;
	else if ((idx % 4) == 1)
		reg_val >>= 8;
	else if ((idx % 4) == 2)
		reg_val >>= 16;
	else
		reg_val >>= 24;

	return (reg_val & 0xff);
}

static int esw_find_free_vlan_slot(u32 start_idx)
{
	u32 i;

	for (i = start_idx; i < 16; i++) {
		if (esw_get_vlan_slot(i) == 0)
			return (int)i;
	}

	return -1;
}

static void esw_vlan_set_idx(u32 idx, u32 cvid, u32 mask_member, u32 mask_untag)
{
	u32 reg_val;

	idx &= 0xf;
	cvid &= 0xfff;
	mask_member &= 0x7f;
	mask_untag &= 0x7f;

	/* set vlan identifier */
	reg_val = esw_reg_get(REG_ESW_VLAN_ID_BASE + 4*(idx/2));
	if ((idx % 2) == 0) {
		reg_val &= 0xfff000;
		reg_val |= cvid;
	} else {
		reg_val &= 0x000fff;
		reg_val |= (cvid << 12);
	}
	esw_reg_set(REG_ESW_VLAN_ID_BASE + 4*(idx/2), reg_val);

	/* set vlan member ports */
	reg_val = esw_reg_get(REG_ESW_VLAN_MEMB_BASE + 4*(idx/4));
	if ((idx % 4) == 0) {
		reg_val &= ~(0xff);
		reg_val |= mask_member;
	} else if ((idx % 4) == 1) {
		reg_val &= ~(0xff << 8);
		reg_val |= (mask_member << 8);
	} else if ((idx % 4) == 2) {
		reg_val &= ~(0xff << 16);
		reg_val |= (mask_member << 16);
	} else {
		reg_val &= ~(0xff << 24);
		reg_val |= (mask_member << 24);
	}
	esw_reg_set(REG_ESW_VLAN_MEMB_BASE + 4*(idx/4), reg_val);

#if !defined (CONFIG_RALINK_RT3052)
	/* set vlan untag ports */
	reg_val = esw_reg_get(REG_ESW_VLAN_UNTAG_BASE + 4*(idx/4));
	if ((idx % 4) == 0) {
		reg_val &= ~(0x7f);
		reg_val |= mask_untag;
	} else if ((idx % 4) == 1) {
		reg_val &= ~(0x7f << 7);
		reg_val |= (mask_untag << 7);
	} else if ((idx % 4) == 2) {
		reg_val &= ~(0x7f << 14);
		reg_val |= (mask_untag << 14);
	} else {
		reg_val &= ~(0x7f << 21);
		reg_val |= (mask_untag << 21);
	}
	esw_reg_set(REG_ESW_VLAN_UNTAG_BASE + 4*(idx/4), reg_val);
#endif
}

static void esw_vlan_reset_table(void)
{
	u32 i;

	/* Reset VLAN mappings */
	for (i = 0; i < 8; i++)
		esw_reg_set(REG_ESW_VLAN_ID_BASE + 4*i, (((i<<1)+2) << 12) | ((i<<1)+1));

	/* Reset VLAN member mappings from idx 1 to 15 */
	esw_reg_set(REG_ESW_VLAN_MEMB_BASE + 0x0, 0xff);
	esw_reg_set(REG_ESW_VLAN_MEMB_BASE + 0x4, 0);
	esw_reg_set(REG_ESW_VLAN_MEMB_BASE + 0x8, 0);
	esw_reg_set(REG_ESW_VLAN_MEMB_BASE + 0xc, 0);

#if !defined (CONFIG_RALINK_RT3052)
	/* Reset VLAN untag mappings from idx 1 to 15 */
	esw_reg_set(REG_ESW_VLAN_UNTAG_BASE + 0x0, 0x3f);
	esw_reg_set(REG_ESW_VLAN_UNTAG_BASE + 0x4, 0);
	esw_reg_set(REG_ESW_VLAN_UNTAG_BASE + 0x8, 0);
	esw_reg_set(REG_ESW_VLAN_UNTAG_BASE + 0xc, 0);
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
		if (vid == pvid_list[i]) {
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

static void esw_vlan_apply_rules(u32 wan_bridge_mode, u32 wan_bwan_isolation)
{
	vlan_entry_t vlan_entry[VLAN_ENTRY_ID_MAX+1];
	pvlan_member_t pvlan_member[ESW_MAC_ID_MAX+1];
	u32 pvid[SWAPI_VLAN_RULE_NUM];
	u32 prio[SWAPI_VLAN_RULE_NUM];
	u32 tagg[SWAPI_VLAN_RULE_NUM];
	u32 i, cvid, next_vid, untg_vid, vlan_idx, vlan_filter_on;

	untg_vid = 2;	// default PVID for untagged WAN traffic
	next_vid = 3;
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
	if (vlan_filter_on) {
		untg_vid = find_free_min_pvid(pvid, 2);
		next_vid = find_free_min_pvid(pvid, untg_vid+1);
	}

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

	/* fill WAN port (use PVID 2 for handle untagged traffic -> VID2) */
	pvlan_member[WAN_PORT_1].pvid = untg_vid;

	/* VID #1 */
	vlan_entry[0].valid = 1;
	vlan_entry[0].cvid = 1;
	vlan_entry[0].port_member |= (1u << LAN_PORT_CPU);

	/* VID #2 */
	vlan_entry[1].valid = 1;
	vlan_entry[1].cvid = untg_vid;
	vlan_entry[1].port_member |= ((1u << WAN_PORT_1) | (1u << WAN_PORT_CPU));
	vlan_entry[1].port_untag  |=  (1u << WAN_PORT_1);

	/* check IPTV tagged */
	if (tagg[SWAPI_VLAN_RULE_WAN_IPTV]) {
		cvid = pvid[SWAPI_VLAN_RULE_WAN_IPTV];
		vlan_idx = find_vlan_slot(vlan_entry, 2, cvid);
		if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
			vlan_entry[vlan_idx].valid = 1;
			vlan_entry[vlan_idx].cvid = cvid;
			vlan_entry[vlan_idx].port_member |= ((1u << WAN_PORT_CPU) | (1u << WAN_PORT_1));
			pvlan_member[WAN_PORT_1].tagg = 1;
		}
	}

	/* check INET tagged */
	if (tagg[SWAPI_VLAN_RULE_WAN_INET]) {
		cvid = pvid[SWAPI_VLAN_RULE_WAN_INET];
		vlan_idx = find_vlan_slot(vlan_entry, 2, cvid);
		if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
			vlan_entry[vlan_idx].valid = 1;
			vlan_entry[vlan_idx].cvid = cvid;
			vlan_entry[vlan_idx].port_member |= ((1u << WAN_PORT_CPU) | (1u << WAN_PORT_1));
			pvlan_member[WAN_PORT_1].tagg = 1;
		}
	}

	/* fill physical LAN ports */
	for (i = 0; i <= ESW_MAC_ID_MAX; i++) {
		int rule_id;
		
		if (i == WAN_PORT_1)
			continue;
		
		if ((1u << i) & ESW_MASK_EXCLUDE)
			continue;
		
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
			
			if (wan_bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_FROM_CPU) {
				vlan_idx = find_vlan_slot(vlan_entry, 2, next_vid);
				if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
					pvlan_member[i].pvid = next_vid;
					
					vlan_entry[vlan_idx].valid = 1;
					vlan_entry[vlan_idx].cvid = next_vid;
					vlan_entry[vlan_idx].port_member |= ((1u << i) | (1u << WAN_PORT_1));
					vlan_entry[vlan_idx].port_untag  |= ((1u << i) | (1u << WAN_PORT_1));
				}
			}
		} else {
			cvid = pvid[rule_id];
			vlan_idx = find_vlan_slot(vlan_entry, 2, cvid);
			if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
				vlan_entry[vlan_idx].valid = 1;
				vlan_entry[vlan_idx].cvid = cvid;
				vlan_entry[vlan_idx].port_member |= ((1u << i) | (1u << WAN_PORT_1));
				if (!tagg[rule_id])
					vlan_entry[vlan_idx].port_untag |= (1u << i);
				
				pvlan_member[i].pvid = cvid;
				pvlan_member[i].prio = prio[rule_id];
				pvlan_member[i].tagg = tagg[rule_id];
				
				pvlan_member[WAN_PORT_1].tagg = 1;
			} else {
				pvlan_member[i].pvid = untg_vid;
				
				/* VID #2 */
				vlan_entry[1].port_member |= (1u << i);
				vlan_entry[1].port_untag  |= (1u << i);
			}
		}
	}

	/* configure physical LAN/WAN ports */
	for (i = 0; i <= ESW_MAC_ID_MAX; i++) {
		esw_vlan_pvid_set(i, pvlan_member[i].pvid, pvlan_member[i].prio);
		if (!pvlan_member[i].tagg) {
			esw_port_ingress_filter(i, 0);
			esw_port_ingress_transparent(i, 1);
			esw_port_egress_untag(i, 1);
		} else {
			esw_port_ingress_filter(i, 1);
			esw_port_ingress_transparent(i, 0);
			esw_port_egress_untag(i, 0);
		}
	}

	/* configure CPU LAN port */
	esw_vlan_pvid_set(LAN_PORT_CPU, 1, 0);
	esw_port_ingress_filter(LAN_PORT_CPU, 1);
	esw_port_ingress_transparent(LAN_PORT_CPU, 0);
	esw_port_egress_untag(LAN_PORT_CPU, 0);

	/* reset VLAN table */
	esw_vlan_reset_table();

	/* fill VLAN table */
	for (i = 0; i <= VLAN_ENTRY_ID_MAX; i++) {
		if (!vlan_entry[i].valid)
			continue;
		if (!vlan_entry[i].port_member)
			continue;
		esw_vlan_set_idx(i, vlan_entry[i].cvid, vlan_entry[i].port_member, vlan_entry[i].port_untag);
	}

	/* save VLAN rules */
	for (i = 0; i < SWAPI_VLAN_RULE_NUM; i++)
		g_vlan_rule[i] = g_vlan_rule_user[i];
}

static void esw_vlan_init_vid1(void)
{
	u32 i, port_member, port_untag;

	port_member = 0xff;
	port_member &= ~(ESW_MASK_EXCLUDE);

	port_untag = (port_member & 0x7f);
	port_untag &= ~(1u << LAN_PORT_CPU);

	/* configure LAN ports */
	for (i = 0; i <= ESW_PORT_ID_MAX; i++) {
		esw_vlan_pvid_set(i, 1, 0);
		esw_port_ingress_filter(i, 0);
		esw_port_ingress_transparent(i, 1);
		esw_port_egress_untag(i, 1);
	}

	/* configure CPU port */
	esw_port_ingress_filter(LAN_PORT_CPU, 1);
	esw_port_ingress_transparent(LAN_PORT_CPU, 0);
	esw_port_egress_untag(LAN_PORT_CPU, 0);

	/* reset VLAN table */
	esw_vlan_reset_table();

	/* set all ports to VLAN 1 member */
	esw_vlan_set_idx(0, 1, port_member, port_untag);
}

static void esw_vlan_bridge_isolate(u32 wan_bridge_mode, u32 wan_bwan_isolation, int bridge_changed, int br_iso_changed, int vlan_rule_changed)
{
	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN) {
		if (!bridge_changed)
			return;
		
		esw_vlan_init_vid1();
	} else {
		if (!bridge_changed && !br_iso_changed && !vlan_rule_changed)
			return;
		
		esw_vlan_apply_rules(wan_bridge_mode, wan_bwan_isolation);
	}

	if (bridge_changed)
		esw_show_bridge_partitions(wan_bridge_mode);

	esw_mac_table_clear();
}

static void esw_soft_reset(void)
{
	esw_reg_set(REG_ESW_STRT, 1);

	atomic_set(&g_switch_inited, 0);
}

static int esw_port_phy_power(u32 phy_port_id, u32 power_on, int is_force)
{
	u32 esw_phy_mcr = 0x3100;
	u32 phy_mdio_addr = phy_port_id;
	u32 i_port_speed, is_power_on;

	if (phy_port_id > ESW_PHY_ID_MAX)
		return 0;

	i_port_speed = (g_port_link_mode[phy_port_id] & 0x0F);
	if (i_port_speed == SWAPI_LINK_SPEED_MODE_FORCE_POWER_OFF)
		power_on = 0;

	is_power_on = 0;
	if (mii_mgr_read(phy_mdio_addr, 0, &esw_phy_mcr)) {
		is_power_on = (esw_phy_mcr & (1<<11)) ? 0 : 1;
		esw_phy_mcr &= ~((1<<11)|(1<<9));
		
		if (i_port_speed < SWAPI_LINK_SPEED_MODE_FORCE_100_FD)
			esw_phy_mcr |= (1<<12);
		
		if (power_on)
			esw_phy_mcr |= (1<<9);
		else
			esw_phy_mcr |= (1<<11);
		
		if (is_force || (is_power_on ^ power_on))
			mii_mgr_write(phy_mdio_addr, 0, esw_phy_mcr);
	}

	g_port_phy_power[phy_port_id] = (power_on) ? 1 : 0;

	/* return 1 when PHY power is changed */
	return (is_power_on ^ power_on) ? 1 : 0;
}

static void power_down_all_phy(void)
{
	u32 i;
	int power_changed = 0;

	/* down all PHY ports */
	for (i = 0; i <= ESW_PHY_ID_MAX; i++)
		power_changed |= esw_port_phy_power(i, 0, 0);

	if (power_changed)
		msleep(500);
}

#if !defined (CONFIG_RALINK_RT3052)
static void esw_storm_control(int set_bcast, int set_mcast, int set_ucast, u32 rate_mbps)
{
	u32 reg_bmu_ctrl = 0x7d000000;
	u32 reg_bmu_lmt1 = 0xffffffff;
	u32 rate_unit_100;
	u32 rate_unit_10;

	/* Use simplification for packet-based mode: 1024 pkts/s ~= 1Mbps */

	if (rate_mbps > 0) {
		rate_unit_100 = (rate_mbps < 90) ? rate_mbps : 90;
		rate_unit_10 = (rate_mbps < 9) ? rate_mbps : 9;
		
		reg_bmu_lmt1  = rate_unit_10 * 1024;
		reg_bmu_lmt1 |= ((rate_unit_100 * 1024) / 10) << 16;
		
		if (set_bcast)
			reg_bmu_ctrl |= 0x00444444;
		if (set_mcast)
			reg_bmu_ctrl |= 0x00222222;
		if (set_ucast)
			reg_bmu_ctrl |= 0x00111111;
	}

	esw_reg_set(REG_ESW_BMU_LMT_NUM1, reg_bmu_lmt1);
	esw_reg_set(REG_ESW_BMU_CTRL, reg_bmu_ctrl);
}
#endif

#if defined (CONFIG_RALINK_MT7628)
static void esw_eee_control(u32 is_eee_enabled)
{
	u32 i, port_phy_power[ESW_PHY_ID_MAX+1];

	/* store PHY power state before down */
	memcpy(port_phy_power, g_port_phy_power, sizeof(g_port_phy_power));

	/* disable PHY ports link */
	power_down_all_phy();

	mt7628_esw_eee_enable(is_eee_enabled);

	/* restore PHY ports link */
	for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
		if (port_phy_power[i])
			esw_port_phy_power(i, 1, 1);
	}
}
#endif

static void esw_led_mode(u32 led_mode)
{
// LED mode 0x0: Link
// LED mode 0x1: Link 100
// LED mode 0x2: Duplex
// LED mode 0x3: Activity
// LED mode 0x4: Collision
// LED mode 0x5: Link/Activity
// LED mode 0x6: Duplex/Collision
// LED mode 0x7: Link 10M/Activity
// LED mode 0x8: Link 100M/Activity
// LED mode 0xa: Blink
// LED mode 0xb: OFF
// LED mode 0xc: ON

	u32 i, reg_ledp;

	switch (led_mode)
	{
	case SWAPI_LED_PHYMODE_100:
		reg_ledp = 0x1;
		break;
	case SWAPI_LED_DUPLEX_COLLISION:
		reg_ledp = 0x6;
		break;
	case SWAPI_LED_PHYMODE_10_ACT:
		reg_ledp = 0x7;
		break;
	case SWAPI_LED_PHYMODE_100_ACT:
		reg_ledp = 0x8;
		break;
	case SWAPI_LED_OFF:
		reg_ledp = 0xb;
		break;
	default:
		reg_ledp = 0x5;
		break;
	}

	for (i = 0; i < 5; i++)
		esw_reg_set(REG_ESW_LEDP_BASE+i*4, reg_ledp);
}

static u32 esw_status_link_port(u32 port_id)
{
	u32 reg_poa;

	reg_poa = esw_reg_get(REG_ESW_POA);

	return (((reg_poa >> 25) >> port_id) & 0x1);
}

static u32 esw_status_speed_port(u32 port_id)
{
	u32 port_link, port_duplex, port_speed;
	u32 port_eee, port_fc_rx, port_fc_tx;
	u32 reg_poa;

	reg_poa = esw_reg_get(REG_ESW_POA);

	port_link = ((reg_poa >> 25) >> port_id) & 0x1;

	if (!port_link)
		return 0;

	port_duplex = ((reg_poa >>  9) >> port_id) & 0x1;
	port_speed  = ((reg_poa >>  0) >> port_id) & 0x1;
	port_fc_tx  = ((reg_poa >> 16) >> port_id) & 0x1;
	port_fc_rx  = port_fc_tx;
	port_eee    = 0;

#if defined (CONFIG_RALINK_MT7628)
	if (g_green_ethernet_enabled) {
		u32 phy_val = 0;
		mii_mgr_write(port_id, 31, 0x8000);	// L0 page
		mii_mgr_read(port_id, 27, &phy_val);
		
		if (phy_val & (1u << 5))
			port_eee = 1;
	}
#endif

	return ((port_link << 16) | (port_eee << 11) | (port_fc_rx << 10) | (port_fc_tx << 9) | (port_duplex << 8) | port_speed);
}

static u32 esw_status_link_ports(int is_wan_ports)
{
	int i;
	u32 port_link = 0;
	u32 portmask;

	if (is_wan_ports)
		portmask = get_ports_mask_wan(0);
	else
		portmask = get_ports_mask_lan(0);

	for (i = 0; i <= ESW_MAC_ID_MAX; i++) {
		if ((portmask >> i) & 0x1) {
			port_link = esw_status_link_port(i);
			if (port_link)
				break;
		}
	}

	return port_link;
}

static u32 esw_status_link_changed(void)
{
	return atomic_cmpxchg(&g_port_link_changed, 1, 0);
}

static void esw_status_mib_port(u32 port_id, esw_mib_counters_t *mibc)
{
	mibc->TxGoodFrames	= esw_get_port_mib_tgpc(port_id);
	mibc->TxBadFrames	= esw_get_port_mib_tbpc(port_id);

	mibc->RxGoodFrames	= esw_get_port_mib_rgpc(port_id);
	mibc->RxBadFrames	= esw_get_port_mib_rbpc(port_id);
}

static void change_ports_power(u32 power_on, u32 ports_mask)
{
	u32 i;

	ports_mask = get_ports_mask_from_user(ports_mask & 0xFF);

	for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
		if ((ports_mask >> i) & 0x1)
			esw_port_phy_power(i, power_on, 0);
	}
}

static int change_wan_lan_ports_power(u32 power_on, u32 is_wan)
{
	int power_changed = 0;
	u32 i, ports_mask;

	if (is_wan)
		ports_mask = get_ports_mask_wan(0);
	else
		ports_mask = get_ports_mask_lan(0);

	for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
		if ((ports_mask >> i) & 0x1)
			power_changed |= esw_port_phy_power(i, power_on, 0);
	}

	return power_changed;
}

static int change_bridge_mode(u32 wan_bwan_isolation, u32 wan_bridge_mode)
{
	int i, bridge_changed, br_iso_changed, vlan_rule_changed, power_changed;

	if (wan_bridge_mode > SWAPI_WAN_BRIDGE_DISABLE_WAN)
		return -EINVAL;

	if (wan_bwan_isolation > SWAPI_WAN_BWAN_ISOLATION_BETWEEN)
		return -EINVAL;

	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE)
		wan_bwan_isolation = SWAPI_WAN_BWAN_ISOLATION_NONE;

	bridge_changed = (g_wan_bridge_mode != wan_bridge_mode) ? 1 : 0;
	br_iso_changed = (g_wan_bwan_isolation != wan_bwan_isolation) ? 1 : 0;
	vlan_rule_changed = 0;
	for (i = 0; i <= SWAPI_VLAN_RULE_WAN_LAN4; i++) {
		if (g_vlan_rule[i] != g_vlan_rule_user[i]) {
			vlan_rule_changed = 1;
			break;
		}
	}

	// set global bridge_mode first
	g_wan_bridge_mode = wan_bridge_mode;
	g_wan_bwan_isolation = wan_bwan_isolation;

	if (atomic_read(&g_switch_inited) == 0)
		return 0;

	power_changed = 0;
	if (bridge_changed || vlan_rule_changed) {
		power_changed = change_wan_lan_ports_power(0, 1);
		if (power_changed) {
			// wait for PHY link down
			msleep(500);
		}
	}

	esw_vlan_bridge_isolate(wan_bridge_mode, wan_bwan_isolation, bridge_changed, br_iso_changed, vlan_rule_changed);

	if (power_changed)
		change_wan_lan_ports_power(1, 1);

	return 0;
}

static void vlan_accept_port_mode(u32 accept_mode, u32 port_mask)
{
	u32 i;
	int en_vlan = 1;

	switch (accept_mode)
	{
	case SWAPI_VLAN_ACCEPT_FRAMES_UNTAG_ONLY:
		en_vlan = 0;
		break;
	}

	port_mask = get_ports_mask_from_user(port_mask & 0xFF);

	for (i = 0; i <= ESW_PORT_ID_MAX; i++) {
		if ((port_mask >> i) & 0x1)
			esw_port_ingress_filter(i, en_vlan);
	}
}

static void vlan_create_entry(u32 vlan4k_info, u32 vlan4k_mask, int set_port_vid)
{
	u32 i, cvid, prio;
	u32 mask_member, mask_untag;
	u32 vlan_table_idx;

	cvid = (vlan4k_info & 0x0FFF);
	prio = ((vlan4k_info >> 12) & 0x7);
	mask_member = get_ports_mask_from_user((vlan4k_mask & 0xFF));
	mask_untag  = get_ports_mask_from_user((vlan4k_mask >> 16) & 0xFF);

	if (cvid < 1)
		cvid = 1;

	/* get vlan slot idx */
	if (cvid > 1) {
		int free_idx = esw_find_free_vlan_slot(1);
		if (free_idx < 0)
			return;
		
		vlan_table_idx = (u32)free_idx;
	} else {
		vlan_table_idx = cvid - 1; // use slot idx 0
	}

	/* set vlan table */
	esw_vlan_set_idx(vlan_table_idx, cvid, mask_member, mask_untag);

	/* set phy ports attrib */
	for (i = 0; i <= ESW_MAC_ID_MAX; i++) {
		if ((1u << i) & mask_member) {
			if (!((1u << i) & mask_untag)) {
				esw_port_ingress_transparent(i, 0);
				esw_port_egress_untag(i, 0);
			} else {
				if (set_port_vid)
					esw_vlan_pvid_set(i, cvid, prio);
				esw_port_ingress_transparent(i, 1);
				esw_port_egress_untag(i, 1);
			}
		}
	}

	printk("%s - create vlan %s: vid=[%d], prio=[%d], member=[0x%02X], untag=[0x%02X]\n",
			MTK_ESW_DEVNAME, (set_port_vid) ? "ports" : "entry", cvid, prio, mask_member, mask_untag);
}

static void change_port_link_mode(u32 phy_port_id, u32 port_link_mode)
{
	u32 i_port_speed, i_port_flowc, i_port_power;
	u32 esw_phy_ana = 0x05e1;
	u32 esw_phy_mcr = 0x3100; /* 100 FD + auto-negotiation */
	u32 phy_mdio_addr = phy_port_id;

	char *link_desc = "Auto";
	char *flow_desc = "ON";

	if (phy_port_id > ESW_PHY_ID_MAX)
		return;

	if (g_port_link_mode[phy_port_id] == port_link_mode)
		return;

	i_port_speed =  (port_link_mode & 0x0F);
	i_port_flowc = ((port_link_mode >> 8) & 0x03);
	i_port_power = (i_port_speed == SWAPI_LINK_SPEED_MODE_FORCE_POWER_OFF) ? 0 : 1;

	if (!i_port_power)
		i_port_speed = SWAPI_LINK_SPEED_MODE_AUTO;

	switch (i_port_speed)
	{
	case SWAPI_LINK_SPEED_MODE_AUTO_100_FD:
		link_desc = "100FD [AN]";
		/* disable ability 100 HD, 10 FD, 10 HD */
		esw_phy_ana &= ~((1<<7)|(1<<6)|(1<<5));
		break;
	case SWAPI_LINK_SPEED_MODE_AUTO_100_HD:
		link_desc = "100HD [AN]";
		/* disable ability 100 FD, 10 FD, 10 HD */
		esw_phy_ana &= ~((1<<8)|(1<<6)|(1<<5));
		/* disable FD */
		esw_phy_mcr &= ~((1<<8));
		break;
	case SWAPI_LINK_SPEED_MODE_AUTO_10_FD:
		link_desc = "10FD [AN]";
		/* disable ability 100 FD, 100 HD, 10 HD */
		esw_phy_ana &= ~((1<<8)|(1<<7)|(1<<5));
		/* set 10Mbps */
		esw_phy_mcr &= ~((1<<13));
		break;
	case SWAPI_LINK_SPEED_MODE_AUTO_10_HD:
		link_desc = "10HD [AN]";
		/* disable ability 100 FD, 100 HD, 10 FD */
		esw_phy_ana &= ~((1<<8)|(1<<7)|(1<<6));
		/* set 10Mbps, disable FD */
		esw_phy_mcr &= ~((1<<13)|(1<<8));
		break;
	case SWAPI_LINK_SPEED_MODE_FORCE_100_FD:
		link_desc = "100FD [Force]";
		/* disable ability 100 HD, 10 FD, 10 HD */
		esw_phy_ana &= ~((1<<7)|(1<<6)|(1<<5));
		/* disable auto-negotiation */
		esw_phy_mcr &= ~((1<<12));
		break;
	case SWAPI_LINK_SPEED_MODE_FORCE_100_HD:
		link_desc = "100HD [Force]";
		/* disable ability 100 FD, 10 FD, 10 HD */
		esw_phy_ana &= ~((1<<8)|(1<<6)|(1<<5));
		/* disable auto-negotiation, disable FD */
		esw_phy_mcr &= ~((1<<12)|(1<<8));
		break;
	case SWAPI_LINK_SPEED_MODE_FORCE_10_FD:
		link_desc = "10FD [Force]";
		/* disable ability 100 FD, 100 HD, 10 HD */
		esw_phy_ana &= ~((1<<8)|(1<<7)|(1<<5));
		/* disable auto-negotiation, set 10Mbps */
		esw_phy_mcr &= ~((1<<13)|(1<<12));
		break;
	case SWAPI_LINK_SPEED_MODE_FORCE_10_HD:
		link_desc = "10HD [Force]";
		/* disable ability 100 FD, 100 HD, 10 FD */
		esw_phy_ana &= ~((1<<8)|(1<<7)|(1<<6));
		/* disable auto-negotiation, set 10Mbps, disable FD */
		esw_phy_mcr &= ~((1<<13)|(1<<12)|(1<<8));
		break;
	}

	switch (i_port_flowc)
	{
	case SWAPI_LINK_FLOW_CONTROL_TX_ASYNC:
	case SWAPI_LINK_FLOW_CONTROL_DISABLE:
		flow_desc = "OFF";
		/* disable pause ability (A6,A5) */
		esw_phy_ana &= ~((1<<11)|(1<<10));
		break;
	}

	/* set PHY ability */
	mii_mgr_write(phy_mdio_addr, 4, esw_phy_ana);

	if (i_port_power) {
		if (!(esw_phy_mcr & (1<<12))) {
			/* power-down PHY */
			esw_phy_mcr |= (1<<11);
			
			/* set PHY mode */
			mii_mgr_write(phy_mdio_addr, 0, esw_phy_mcr);
			
			/* wait for PHY down */
			msleep(500);
			
			/* power-up PHY */
			esw_phy_mcr &= ~(1<<11);
		} else {
			/* restart PHY auto-negotiation */
			esw_phy_mcr |= (1<<9);
		}
	} else {
		/* power-down PHY */
		esw_phy_mcr |= (1<<11);
	}

	/* set PHY mode */
	mii_mgr_write(phy_mdio_addr, 0, esw_phy_mcr);

	g_port_link_mode[phy_port_id] = port_link_mode;

	if (!i_port_power) {
		link_desc = "Power OFF";
		flow_desc = "N/A";
	}

	printk("%s - %s link speed: %s, flow control: %s\n",
		MTK_ESW_DEVNAME, get_port_desc(phy_port_id), link_desc, flow_desc);
}

#if !defined (CONFIG_RALINK_RT3052)
static void change_storm_control_broadcast(u32 control_rate_mbps)
{
	char rate_desc[16];

	if (control_rate_mbps >= 1024)
		control_rate_mbps = 0;

	if (g_storm_rate_limit != control_rate_mbps) {
		g_storm_rate_limit = control_rate_mbps;
		
		if (control_rate_mbps > 0)
			snprintf(rate_desc, sizeof(rate_desc), "%d mbps", control_rate_mbps);
		else
			strcpy(rate_desc, "off");
		
		printk("%s - set broadcast storm control rate as: %s\n", MTK_ESW_DEVNAME, rate_desc);
		
		esw_storm_control(1, 0, 0, control_rate_mbps);
	}
}
#endif

#if defined (CONFIG_RALINK_MT7628)
static void change_green_ethernet_mode(u32 green_ethernet_enabled)
{
	if (green_ethernet_enabled)
		green_ethernet_enabled = 1;

	if (g_green_ethernet_enabled != green_ethernet_enabled) {
		g_green_ethernet_enabled = green_ethernet_enabled;
		printk("%s - 802.3az EEE: %s\n", MTK_ESW_DEVNAME, (green_ethernet_enabled) ? "on" : "off");
		
		esw_eee_control(green_ethernet_enabled);
	}
}
#endif

static void change_led_mode(u32 led_mode)
{
	if (led_mode != SWAPI_LED_OFF)
		led_mode = SWAPI_LED_LINK_ACT;

	if (g_led_phy_mode != led_mode) {
		g_led_phy_mode = led_mode;
		esw_led_mode(led_mode);
	}
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

void esw_link_status_changed(u32 port_id, int port_link)
{
	const char *port_state;

	if (port_id <= ESW_PHY_ID_MAX)
		atomic_set(&g_port_link_changed, 1);

#if !ESW_PRINT_LINK_ALL
	{
		u32 wan_ports_mask = get_ports_mask_wan(0);
		
		if (!(wan_ports_mask & (1u << port_id)))
			return;
	}
#endif

	port_state = (port_link) ? "Up" : "Down";

	printk("%s: Link Status Changed - Port %s Link %s\n",
		MTK_ESW_DEVNAME, get_port_desc(port_id), port_state);
}

int esw_control_post_init(void)
{
	/* configure bridge isolation mode via VLAN */
	esw_vlan_bridge_isolate(g_wan_bridge_mode, g_wan_bwan_isolation, 1, 1, 1);

	/* configure leds */
	esw_led_mode(g_led_phy_mode);

#if defined (CONFIG_RALINK_MT7628)
	/* disable 802.3az EEE by default */
	if (!g_green_ethernet_enabled)
		mt7628_esw_eee_enable(0);
#endif

	atomic_set(&g_switch_inited, 1);

	return 0;
}

static void reset_and_init_switch(void)
{
	u32 i, port_phy_power[ESW_PHY_ID_MAX+1];

	/* store PHY power state before down */
	memcpy(port_phy_power, g_port_phy_power, sizeof(g_port_phy_power));

	power_down_all_phy();

	esw_soft_reset();
	esw_control_post_init();

	/* restore PHY power state */
	for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
		if (port_phy_power[i])
			esw_port_phy_power(i, 1, 1);
	}
}

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

////////////////////////////////////////////////////////////////////////////////////

long mtk_esw_ioctl(struct file *file, unsigned int req, unsigned long arg)
{
	int ioctl_result = 0;
	u32 uint_value = 0;
	u32 uint_result = 0;
	esw_mib_counters_t port_counters;

	unsigned int uint_param = (req >> MTK_ESW_IOCTL_CMD_LENGTH_BITS);
	req &= ((1u << MTK_ESW_IOCTL_CMD_LENGTH_BITS)-1);

	mutex_lock(&esw_access_mutex);

	switch(req)
	{
	case MTK_ESW_IOCTL_STATUS_LINK_PORT_WAN:
		uint_result = esw_status_link_port(WAN_PORT_1);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_PORTS_WAN:
		uint_result = esw_status_link_ports(1);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_PORTS_LAN:
		uint_result = esw_status_link_ports(0);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN1:
		uint_result = esw_status_link_port(LAN_PORT_1);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN2:
		uint_result = esw_status_link_port(LAN_PORT_2);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN3:
		uint_result = esw_status_link_port(LAN_PORT_3);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_PORT_LAN4:
		uint_result = esw_status_link_port(LAN_PORT_4);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_LINK_CHANGED:
		uint_result = esw_status_link_changed();
		put_user(uint_result, (unsigned int __user *)arg);
		break;

	case MTK_ESW_IOCTL_STATUS_SPEED_PORT_WAN:
		uint_result = esw_status_speed_port(WAN_PORT_1);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN1:
		uint_result = esw_status_speed_port(LAN_PORT_1);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN2:
		uint_result = esw_status_speed_port(LAN_PORT_2);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN3:
		uint_result = esw_status_speed_port(LAN_PORT_3);
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_STATUS_SPEED_PORT_LAN4:
		uint_result = esw_status_speed_port(LAN_PORT_4);
		put_user(uint_result, (unsigned int __user *)arg);
		break;

	case MTK_ESW_IOCTL_STATUS_CNT_PORT_WAN:
		esw_status_mib_port(WAN_PORT_1, &port_counters);
		copy_to_user((esw_mib_counters_t __user *)arg, &port_counters, sizeof(esw_mib_counters_t));
		break;
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN1:
		esw_status_mib_port(LAN_PORT_1, &port_counters);
		copy_to_user((esw_mib_counters_t __user *)arg, &port_counters, sizeof(esw_mib_counters_t));
		break;
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN2:
		esw_status_mib_port(LAN_PORT_2, &port_counters);
		copy_to_user((esw_mib_counters_t __user *)arg, &port_counters, sizeof(esw_mib_counters_t));
		break;
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN3:
		esw_status_mib_port(LAN_PORT_3, &port_counters);
		copy_to_user((esw_mib_counters_t __user *)arg, &port_counters, sizeof(esw_mib_counters_t));
		break;
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN4:
		esw_status_mib_port(LAN_PORT_4, &port_counters);
		copy_to_user((esw_mib_counters_t __user *)arg, &port_counters, sizeof(esw_mib_counters_t));
		break;

	case MTK_ESW_IOCTL_RESET_SWITCH:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		if (uint_value == SWAPI_MAGIC_RESET_ASIC)
			reset_and_init_switch();
		break;
	case MTK_ESW_IOCTL_PORTS_POWER:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_ports_power(uint_param, uint_value);
		break;
	case MTK_ESW_IOCTL_PORTS_WAN_LAN_POWER:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_wan_lan_ports_power(uint_param, uint_value);
		break;
	case MTK_ESW_IOCTL_MAC_TABLE_CLEAR:
		esw_mac_table_clear();
		break;

	case MTK_ESW_IOCTL_BRIDGE_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_bridge_mode(uint_param, uint_value);
		break;

	case MTK_ESW_IOCTL_VLAN_RESET_TABLE:
		esw_vlan_reset_table();
		break;
	case MTK_ESW_IOCTL_VLAN_PVID_WAN_GET:
		uint_result = g_vlan_pvid_wan_untagged;
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_VLAN_ACCEPT_PORT_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		vlan_accept_port_mode(uint_param, uint_value);
		break;
	case MTK_ESW_IOCTL_VLAN_CREATE_PORT_VID:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		vlan_create_entry(uint_param, uint_value, 1);
		break;
	case MTK_ESW_IOCTL_VLAN_CREATE_ENTRY:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		vlan_create_entry(uint_param, uint_value, 0);
		break;
	case MTK_ESW_IOCTL_VLAN_RULE_SET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ioctl_result = change_vlan_rule(uint_param, uint_value);
		break;

	case MTK_ESW_IOCTL_STORM_BROADCAST:
#if !defined (CONFIG_RALINK_RT3052)
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_storm_control_broadcast(uint_value);
#endif
		break;

	case MTK_ESW_IOCTL_JUMBO_FRAMES:
		/* N/A */
		break;

	case MTK_ESW_IOCTL_GREEN_ETHERNET:
#if defined (CONFIG_RALINK_MT7628)
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_green_ethernet_mode(uint_value);
#endif
		break;

	case MTK_ESW_IOCTL_IGMP_STATIC_PORTS:
		break;

	case MTK_ESW_IOCTL_IGMP_SNOOPING:
		break;

	case MTK_ESW_IOCTL_LED_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_led_mode(uint_value);
		break;

	case MTK_ESW_IOCTL_SPEED_PORT_WAN:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(WAN_PORT_1, uint_value);
		break;
	case MTK_ESW_IOCTL_SPEED_PORT_LAN1:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_1, uint_value);
		break;
	case MTK_ESW_IOCTL_SPEED_PORT_LAN2:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_2, uint_value);
		break;
	case MTK_ESW_IOCTL_SPEED_PORT_LAN3:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_3, uint_value);
		break;
	case MTK_ESW_IOCTL_SPEED_PORT_LAN4:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(LAN_PORT_4, uint_value);
		break;

	default:
		ioctl_result = -ENOIOCTLCMD;
	}

	mutex_unlock(&esw_access_mutex);

	return ioctl_result;
}

////////////////////////////////////////////////////////////////////////////////////

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
int esw_get_traffic_port_wan(struct rtnl_link_stats64 *stats)
{
	/* RT3052/RT3352/RT5350/MT7628 ESW not support port byte counters.
	   This function need for RT3052/RT3352 but cannot be implemented :(.
	 */
	stats->rx_bytes = 0;
	stats->tx_bytes = 0;

	return 0;
}
EXPORT_SYMBOL(esw_get_traffic_port_wan);
#endif

////////////////////////////////////////////////////////////////////////////////////

static int mtk_esw_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////

static int mtk_esw_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////

static const struct file_operations mtk_esw_fops =
{
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= mtk_esw_ioctl,
	.open		= mtk_esw_open,
	.release	= mtk_esw_release,
};

////////////////////////////////////////////////////////////////////////////////////

int esw_ioctl_init(void)
{
	int r;

	fill_bridge_members();

	r = register_chrdev(MTK_ESW_DEVMAJOR, MTK_ESW_DEVNAME, &mtk_esw_fops);
	if (r < 0) {
		printk(KERN_ERR MTK_ESW_DEVNAME ": unable to register character device\n");
		return r;
	}

	return 0;
}

void esw_ioctl_uninit(void)
{
	unregister_chrdev(MTK_ESW_DEVMAJOR, MTK_ESW_DEVNAME);
}

