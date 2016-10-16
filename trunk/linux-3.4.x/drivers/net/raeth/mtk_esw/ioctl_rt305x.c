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
#include "ioctl_igmp.h"
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
static u32 g_eee_lpi_enabled                     = ESW_DEFAULT_EEE_LPI;
#endif

static u32 g_port_link_mode[ESW_EPHY_ID_MAX+1]   = {0, 0, 0, 0, 0};
static u32 g_port_phy_power[ESW_EPHY_ID_MAX+1]   = {0, 0, 0, 0, 0};

static u32 g_vlan_rule[SWAPI_VLAN_RULE_NUM]      = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static u32 g_vlan_rule_user[SWAPI_VLAN_RULE_NUM] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

////////////////////////////////////////////////////////////////////////////////////

static atomic_t g_switch_inited                  = ATOMIC_INIT(0);
static atomic_t g_port_link_changed              = ATOMIC_INIT(0);

static bwan_member_t g_bwan_member[SWAPI_WAN_BRIDGE_NUM][ESW_EPHY_ID_MAX+1];

static u32 g_vlan_pvid_wan_untagged              = 2;

////////////////////////////////////////////////////////////////////////////////////

u32 get_ports_mask_lan(u32 include_cpu)
{
	u32 i, wan_bridge_mode, portmask_lan;

	wan_bridge_mode = g_wan_bridge_mode;

	portmask_lan = MASK_LAN_PORT_1|MASK_LAN_PORT_2|MASK_LAN_PORT_3|MASK_LAN_PORT_4|MASK_LAN_PORT_5;
	if (include_cpu)
		portmask_lan |= MASK_ESW_PORT_CPU;

	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if (g_bwan_member[wan_bridge_mode][i].bwan)
			portmask_lan &= ~(1u << i);
	}

	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		portmask_lan |= MASK_WAN_PORT_X;

	return portmask_lan;
}

static u32 get_ports_mask_wan(u32 include_cpu)
{
	u32 i, wan_bridge_mode, portmask_wan;

	wan_bridge_mode = g_wan_bridge_mode;
	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		return 0;

	portmask_wan = MASK_WAN_PORT_X;
	if (include_cpu)
		portmask_wan |= MASK_ESW_PORT_CPU;

	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if (g_bwan_member[wan_bridge_mode][i].bwan)
			portmask_wan |= (1u << i);
	}

	return portmask_wan;
}

static u32 get_ports_mask_from_uapi(u32 port_mask_uapi)
{
	u32 gsw_ports_mask = 0;

	if (port_mask_uapi & SWAPI_PORTMASK_LAN1)
		gsw_ports_mask |= MASK_LAN_PORT_1;
	if (port_mask_uapi & SWAPI_PORTMASK_LAN2)
		gsw_ports_mask |= MASK_LAN_PORT_2;
	if (port_mask_uapi & SWAPI_PORTMASK_LAN3)
		gsw_ports_mask |= MASK_LAN_PORT_3;
	if (port_mask_uapi & SWAPI_PORTMASK_LAN4)
		gsw_ports_mask |= MASK_LAN_PORT_4;
	if (port_mask_uapi & SWAPI_PORTMASK_LAN5)
		gsw_ports_mask |= MASK_LAN_PORT_5;
	if (port_mask_uapi & SWAPI_PORTMASK_WAN)
		gsw_ports_mask |= MASK_WAN_PORT_X;
	if (port_mask_uapi & SWAPI_PORTMASK_CPU_LAN)
		gsw_ports_mask |= MASK_ESW_PORT_CPU;
	if (port_mask_uapi & SWAPI_PORTMASK_CPU_WAN)
		gsw_ports_mask |= MASK_ESW_PORT_CPU;

	return gsw_ports_mask;
}

static u32 get_port_from_uapi(u32 port_id_uapi)
{
	switch (port_id_uapi)
	{
	case SWAPI_PORT_WAN:
		return WAN_PORT_X;
	case SWAPI_PORT_LAN1:
		return LAN_PORT_1;
	case SWAPI_PORT_LAN2:
		return LAN_PORT_2;
	case SWAPI_PORT_LAN3:
		return LAN_PORT_3;
	case SWAPI_PORT_LAN4:
		return LAN_PORT_4;
#if defined (LAN_PORT_5)
	case SWAPI_PORT_LAN5:
		return LAN_PORT_5;
#endif
	case SWAPI_PORT_CPU_LAN:
	case SWAPI_PORT_CPU_WAN:
		return ESW_PORT_CPU;
	}

	return ESW_PORT_ID_MAX+1;
}

static const char* get_port_desc(u32 port_id)
{
	const char *port_desc;

	switch (port_id)
	{
	case WAN_PORT_X:
		port_desc = "WAN";
		break;
	case LAN_PORT_1:
		port_desc = "LAN1";
		break;
	case LAN_PORT_2:
		port_desc = "LAN2";
		break;
	case LAN_PORT_3:
		port_desc = "LAN3";
		break;
	case LAN_PORT_4:
		port_desc = "LAN4";
		break;
#if defined (LAN_PORT_5)
	case LAN_PORT_5:
		port_desc = "LAN5";
		break;
#endif
	case ESW_PORT_CPU:
	default:
		port_desc = "CPU";
		break;
	}

	return port_desc;
}

static void esw_show_bridge_partitions(u32 wan_bridge_mode)
{
	const char *wanl, *wanr;
	char lans[8];

	wanl = "W|";
	wanr = "";

	switch (wan_bridge_mode)
	{
	case SWAPI_WAN_BRIDGE_LAN1:
		strcpy(lans, "WLLL");
		break;
	case SWAPI_WAN_BRIDGE_LAN2:
		strcpy(lans, "LWLL");
		break;
	case SWAPI_WAN_BRIDGE_LAN3:
		strcpy(lans, "LLWL");
		break;
	case SWAPI_WAN_BRIDGE_LAN4:
		strcpy(lans, "LLLW");
		break;
	case SWAPI_WAN_BRIDGE_LAN3_LAN4:
		strcpy(lans, "LLWW");
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2:
		strcpy(lans, "WWLL");
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		strcpy(lans, "WWWL");
		break;
	case SWAPI_WAN_BRIDGE_DISABLE_WAN:
		strcpy(lans, "LLLL");
		wanl = "L";
		wanr = "";
		break;
	default:
		strcpy(lans, "LLLL");
		break;
	}

#if defined (LAN_PORT_5)
	strcat(lans, "L");
#endif

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

#if !defined (CONFIG_RAETH_ESW_IGMP_SNOOP_OFF)
void esw_igmp_flood_to_cpu(int flood_to_cpu)
{
	u32 reg_val;

	/* IGMP */
	reg_val = esw_reg_get(REG_ESW_PFC1);
	if (flood_to_cpu)
		reg_val |=  BIT(23);
	else
		reg_val &= ~BIT(23);
	esw_reg_set(REG_ESW_PFC1, reg_val);

#if !defined (CONFIG_RALINK_RT3052)
	/* MLD */
	reg_val = esw_reg_get(REG_ESW_POC2);
	if (flood_to_cpu)
		reg_val |=  BIT(25);
	else
		reg_val &= ~BIT(25);
	esw_reg_set(REG_ESW_POC2, reg_val);
#endif
}
#endif

static void esw_mac_table_clear(void)
{
	rt305x_esw_mac_table_clear(0);
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

static int is_vlan_rule_included(u32 wan_bridge_mode, u32 rule_id)
{
	u32 i;

	if (rule_id == SWAPI_VLAN_RULE_WAN_INET ||
	    rule_id == SWAPI_VLAN_RULE_WAN_IPTV)
		return 1;

	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if (g_bwan_member[wan_bridge_mode][i].bwan &&
		    g_bwan_member[wan_bridge_mode][i].rule == (u8)rule_id)
			return 1;
	}

	return 0;
}

static int is_wan_vid_valid(u32 vid)
{
	return (vid == 2 || vid >= MIN_EXT_VLAN_VID) ? 1 : 0;
}

static void esw_vlan_apply_rules(u32 wan_bridge_mode, u32 wan_bwan_isolation)
{
	vlan_entry_t vlan_entry[VLAN_ENTRY_ID_MAX+1];
	pvlan_member_t pvlan_member[ESW_EPHY_ID_MAX+1];
	u32 pvid[SWAPI_VLAN_RULE_NUM] = {0};
	u32 prio[SWAPI_VLAN_RULE_NUM] = {0};
	u32 tagg[SWAPI_VLAN_RULE_NUM] = {0};
	u32 i, cvid, next_vid, untg_vid, vlan_idx, vlan_filter_on;

	untg_vid = 2;	// default PVID for untagged WAN traffic
	next_vid = 3;
	vlan_filter_on = 0;

	memset(vlan_entry, 0, sizeof(vlan_entry));
	memset(pvlan_member, 0, sizeof(pvlan_member));

	for (i = 0; i < SWAPI_VLAN_RULE_NUM; i++) {
		if (!is_vlan_rule_included(wan_bridge_mode, i))
			continue;
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
	pvlan_member[WAN_PORT_X].pvid = untg_vid;

	/* VID #1 */
	vlan_entry[0].valid = 1;
	vlan_entry[0].cvid = 1;
	vlan_entry[0].port_member |= MASK_ESW_PORT_CPU;

	/* VID #2 */
	vlan_entry[1].valid = 1;
	vlan_entry[1].cvid = untg_vid;
	vlan_entry[1].port_member |= (MASK_WAN_PORT_X | MASK_ESW_PORT_CPU);
	vlan_entry[1].port_untag  |=  MASK_WAN_PORT_X;

	/* check IPTV tagged */
	if (tagg[SWAPI_VLAN_RULE_WAN_IPTV]) {
		cvid = pvid[SWAPI_VLAN_RULE_WAN_IPTV];
		vlan_idx = find_vlan_slot(vlan_entry, 2, cvid);
		if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
			vlan_entry[vlan_idx].valid = 1;
			vlan_entry[vlan_idx].cvid = cvid;
			vlan_entry[vlan_idx].port_member |= (MASK_WAN_PORT_X | MASK_ESW_PORT_CPU);
			pvlan_member[WAN_PORT_X].tagg = 1;
		}
	}

	/* check INET tagged */
	if (tagg[SWAPI_VLAN_RULE_WAN_INET]) {
		cvid = pvid[SWAPI_VLAN_RULE_WAN_INET];
		vlan_idx = find_vlan_slot(vlan_entry, 2, cvid);
		if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
			vlan_entry[vlan_idx].valid = 1;
			vlan_entry[vlan_idx].cvid = cvid;
			vlan_entry[vlan_idx].port_member |= (MASK_WAN_PORT_X | MASK_ESW_PORT_CPU);
			pvlan_member[WAN_PORT_X].tagg = 1;
		}
	}

	/* fill physical LAN ports */
	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		int rule_id;
		
		if (i == WAN_PORT_X)
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
					vlan_entry[vlan_idx].port_member |= ((1u << i) | MASK_WAN_PORT_X);
					vlan_entry[vlan_idx].port_untag  |= ((1u << i) | MASK_WAN_PORT_X);
				}
			}
		} else {
			cvid = pvid[rule_id];
			vlan_idx = find_vlan_slot(vlan_entry, 2, cvid);
			if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
				vlan_entry[vlan_idx].valid = 1;
				vlan_entry[vlan_idx].cvid = cvid;
				vlan_entry[vlan_idx].port_member |= ((1u << i) | MASK_WAN_PORT_X);
				if (wan_bwan_isolation != SWAPI_WAN_BWAN_ISOLATION_FROM_CPU)
					vlan_entry[vlan_idx].port_member |= MASK_ESW_PORT_CPU;
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

	/* configure physical LAN/WAN ports */
	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if ((1u << i) & ESW_MASK_EXCLUDE)
			continue;
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
	esw_vlan_pvid_set(ESW_PORT_CPU, 1, 0);
	esw_port_ingress_filter(ESW_PORT_CPU, 1);
	esw_port_ingress_transparent(ESW_PORT_CPU, 0);
	esw_port_egress_untag(ESW_PORT_CPU, 0);

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
	port_untag &= ~(MASK_ESW_PORT_CPU);

	/* configure physical LAN ports */
	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if ((1u << i) & ESW_MASK_EXCLUDE)
			continue;
		esw_vlan_pvid_set(i, 1, 0);
		esw_port_ingress_filter(i, 0);
		esw_port_ingress_transparent(i, 1);
		esw_port_egress_untag(i, 1);
	}

	/* configure CPU port */
	esw_vlan_pvid_set(ESW_PORT_CPU, 1, 0);
	esw_port_ingress_filter(ESW_PORT_CPU, 1);
	esw_port_ingress_transparent(ESW_PORT_CPU, 0);
	esw_port_egress_untag(ESW_PORT_CPU, 0);

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

	if (phy_port_id > ESW_EPHY_ID_MAX)
		return 0;

	/* external GigaPHY */
#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
	if (phy_port_id == 5)
		phy_mdio_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#endif

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
	for (i = 0; i <= ESW_EPHY_ID_MAX; i++)
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
	u32 i, port_phy_power[ESW_EPHY_ID_MAX+1];

	/* store PHY power state before down */
	memcpy(port_phy_power, g_port_phy_power, sizeof(g_port_phy_power));

	/* disable PHY ports link */
	power_down_all_phy();

	mt7628_esw_eee_enable(is_eee_enabled);

	/* restore PHY ports link */
	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
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

static u32 esw_status_speed_port_uapi(u32 port_id_uapi)
{
	u32 port_link, port_duplex, port_speed;
	u32 port_eee, port_fc_rx, port_fc_tx;
	u32 reg_poa;
	u32 port_id = get_port_from_uapi(port_id_uapi);

	if (port_id > ESW_EPHY_ID_MAX)
		return 0;

	reg_poa = esw_reg_get(REG_ESW_POA);

	port_link = ((reg_poa >> 25) >> port_id) & 0x1;
	if (!port_link)
		return 0;

#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
	if (port_id == 5) {
		port_speed = (reg_poa >> 5) & 0x3;
		port_fc_tx = (reg_poa >> 22) & 0x1;
		port_fc_rx = (reg_poa >> 21) & 0x1;
	} else
#endif
	{
		port_speed = (reg_poa >> port_id) & 0x1;
		port_fc_tx = ((reg_poa >> 16) >> port_id) & 0x1;
		port_fc_rx = port_fc_tx;
	}

	port_duplex = ((reg_poa >> 9) >> port_id) & 0x1;
	port_eee = 0;

#if defined (CONFIG_RALINK_MT7628)
	if (g_eee_lpi_enabled) {
		u32 phy_val = 0;
		mii_mgr_write(port_id, 31, 0x8000);	// L0 page
		mii_mgr_read(port_id, 27, &phy_val);
		
		if (phy_val & (1u << 5))
			port_eee = 1;
	}
#endif

	return ((port_link << 16)|(port_eee << 11)|(port_fc_rx << 10)|(port_fc_tx << 9)|(port_duplex << 8)|port_speed);
}

static u32 esw_status_link_port_uapi(u32 port_id_uapi)
{
	u32 port_id = get_port_from_uapi(port_id_uapi);

	if (port_id > ESW_EPHY_ID_MAX)
		return 0;

	return esw_status_link_port(port_id);
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

	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
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

static int esw_status_mib_port_uapi(u32 port_id_uapi, esw_mib_counters_t *mibc)
{
	u32 port_id = get_port_from_uapi(port_id_uapi);

	if (port_id > ESW_EPHY_ID_MAX)
		return -EINVAL;

	mibc->TxGoodFrames	= esw_get_port_mib_tgpc(port_id);
	mibc->TxBadFrames	= esw_get_port_mib_tbpc(port_id);

	mibc->RxGoodFrames	= esw_get_port_mib_rgpc(port_id);
	mibc->RxBadFrames	= esw_get_port_mib_rbpc(port_id);

	return 0;
}

static inline int esw_status_bytes_port_uapi(u32 port_id_uapi, port_bytes_t *pb)
{
	return -ENOIOCTLCMD;
}

static void esw_status_mib_reset(void)
{
#if !defined (CONFIG_RALINK_RT3052)
	esw_reg_set(0x14c, 0xff7f7f7f);
	esw_mib_init();
#endif
}

static void change_ports_power(u32 power_on, u32 ports_mask)
{
	u32 i;

	ports_mask = get_ports_mask_from_uapi(ports_mask);

	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
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

	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
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

	/* this isolation mode not possible for this switch */
	if (wan_bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_BETWEEN)
		wan_bwan_isolation = SWAPI_WAN_BWAN_ISOLATION_NONE;

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

	port_mask = get_ports_mask_from_uapi(port_mask);

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
	mask_member = get_ports_mask_from_uapi((vlan4k_mask & 0xFFFF));
	mask_untag  = get_ports_mask_from_uapi((vlan4k_mask >> 16) & 0xFFFF);

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

	/* set ports attrib */
	for (i = 0; i <= ESW_PORT_ID_MAX; i++) {
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

static int change_port_link_mode_uapi(u32 port_id_uapi, u32 port_link_mode)
{
	const char *link_desc = "Auto", *flow_desc = "ON";
	u32 i_port_speed, i_port_flowc, i_port_power;
	u32 esw_phy_ana = 0x05e1;
	u32 esw_phy_mcr = 0x3100; /* 100 FD + auto-negotiation */
	u32 phy_mdio_addr;
	u32 port_id = get_port_from_uapi(port_id_uapi);

	if (port_id > ESW_EPHY_ID_MAX)
		return -EINVAL;

	if (g_port_link_mode[port_id] == port_link_mode)
		return 0;

	phy_mdio_addr = port_id;

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

	/* external GigaPHY */
#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
	if (port_id == 5) {
		u32 esw_phy_gcr = 0;
		
		phy_mdio_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
		
		/* set MII control register [6,13] */
		if (i_port_speed <= SWAPI_LINK_SPEED_MODE_AUTO_1000_FD) {
			/* set 1000 Mbps */
			esw_phy_mcr &= ~(1<<13);
			esw_phy_mcr |=  (1<<6);
		}
		
		/* set auto-negotiation advertisement register [5,6,7,8] */
		if (i_port_speed == SWAPI_LINK_SPEED_MODE_AUTO_1000_FD) {
			/* disable ability 100 FD, 100 HD, 10 FD, 10 HD */
			esw_phy_ana &= ~((1<<8)|(1<<7)|(1<<6)|(1<<5));
			link_desc = "1000FD [AN]";
		}
		
		/* set auto-negotiation advertisement register [11] */
		if (i_port_speed <= SWAPI_LINK_SPEED_MODE_AUTO_1000_FD &&
		    i_port_flowc == SWAPI_LINK_FLOW_CONTROL_TX_ASYNC) {
			/* enable asymmetric pause ability (A6) */
			esw_phy_ana |= (1<<11);
			flow_desc = "TX Asy";
		}
		
		/* set 1000Base-T control register [8,9] */
		mii_mgr_read(phy_mdio_addr, 9, &esw_phy_gcr);
		if (i_port_speed <= SWAPI_LINK_SPEED_MODE_AUTO_1000_FD) {
			/* enable 1000Base-T Advertisement */
			esw_phy_gcr |=  ((1<<9)|(1<<8));
		} else {
			/* disable 1000Base-T Advertisement */
			esw_phy_gcr &= ~((1<<9)|(1<<8));
		}
		mii_mgr_write(phy_mdio_addr, 9, esw_phy_gcr);
	}
#endif

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

	g_port_link_mode[port_id] = port_link_mode;

	if (!i_port_power) {
		link_desc = "Power OFF";
		flow_desc = "N/A";
	}

	printk("%s - %s link speed: %s, flow control: %s\n",
		MTK_ESW_DEVNAME, get_port_desc(port_id), link_desc, flow_desc);

	return 0;
}

static void change_storm_control_broadcast(u32 control_rate_mbps)
{
#if !defined (CONFIG_RALINK_RT3052)
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
#endif
}

static inline void change_jumbo_frames_accept(u32 jumbo_frames_enabled)
{
	// N.A.
}

static void change_eee_lpi_mode(u32 eee_lpi_enabled)
{
#if defined (CONFIG_RALINK_MT7628)
	if (eee_lpi_enabled)
		eee_lpi_enabled = 1;

	if (g_eee_lpi_enabled != eee_lpi_enabled) {
		g_eee_lpi_enabled = eee_lpi_enabled;
		printk("%s - 802.3az EEE: %s\n", MTK_ESW_DEVNAME, (eee_lpi_enabled) ? "on" : "off");
		
		esw_eee_control(eee_lpi_enabled);
	}
#endif
}

static void change_igmp_static_ports(u32 ports_mask)
{
#if defined (CONFIG_RAETH_ESW_IGMP_SNOOP_SW)
	ports_mask = get_ports_mask_from_uapi(ports_mask);

	igmp_sn_set_static_ports(ports_mask);
#endif
}

static void change_igmp_snooping_control(u32 igmp_snooping_enabled)
{
#if defined (CONFIG_RAETH_ESW_IGMP_SNOOP_SW)
	igmp_sn_set_enable(igmp_snooping_enabled);
#endif
}

static void change_led_mode(u32 led_mode)
{
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

static void esw_link_status_changed(u32 port_id, int port_link)
{
	const char *port_state;

	if (port_id <= ESW_EPHY_ID_MAX)
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

int esw_ioctl_init_post(void)
{
	/* configure bridge isolation mode via VLAN */
	esw_vlan_bridge_isolate(g_wan_bridge_mode, g_wan_bwan_isolation, 1, 1, 1);

	/* configure leds */
	esw_led_mode(g_led_phy_mode);

#if defined (CONFIG_RALINK_MT7628)
	/* disable 802.3az EEE by default */
	if (!g_eee_lpi_enabled)
		mt7628_esw_eee_enable(0);
#endif

	atomic_set(&g_switch_inited, 1);

	return 0;
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
#include "ioctl.c"
////////////////////////////////////////////////////////////////////////////////////

