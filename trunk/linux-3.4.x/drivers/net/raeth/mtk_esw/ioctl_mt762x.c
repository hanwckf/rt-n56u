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

#include "../ra_eth_reg.h"
#include "../ra_esw_reg.h"
#include "../mii_mgr.h"

#include "../ra_phy.h"
#include "../ra_esw_base.h"
#include "../ra_esw_mt7620.h"
#include "../ra_gsw_mt7530.h"
#include "ioctl.h"
#include "ioctl_igmp.h"
#include "ioctl_mt762x.h"

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../../net/nat/hw_nat/ra_nat.h"
#include "../../../../net/nat/hw_nat/foe_fdb.h"
extern int (*ra_sw_nat_hook_rx)(struct sk_buff *skb);
#endif

////////////////////////////////////////////////////////////////////////////////////

static DEFINE_MUTEX(esw_access_mutex);
#if defined (CONFIG_MT7530_GSW)
static DECLARE_BITMAP(g_vlan_pool, 4096);
#endif

static u32 g_wan_bridge_mode                     = SWAPI_WAN_BRIDGE_DISABLE;
static u32 g_wan_bwan_isolation                  = SWAPI_WAN_BWAN_ISOLATION_NONE;

static u32 g_led_phy_mode                        = SWAPI_LED_LINK_ACT;

static u32 g_jumbo_frames_enabled                = ESW_DEFAULT_JUMBO_FRAMES;
static u32 g_eee_lpi_enabled                     = ESW_DEFAULT_EEE_LPI;
#if defined (CONFIG_RAETH_ESW_IGMP_SNOOP_HW)
static u32 g_igmp_snooping_enabled               = ESW_DEFAULT_IGMP_SNOOPING;
static u32 g_igmp_static_ports                   = 0;
#endif

static u32 g_storm_rate_limit                    = ESW_DEFAULT_STORM_RATE;

static u32 g_port_link_mode[ESW_EPHY_ID_MAX+1]   = {0, 0, 0, 0, 0};
static u32 g_port_phy_power[ESW_EPHY_ID_MAX+1]   = {0, 0, 0, 0, 0};

static u32 g_vlan_rule[SWAPI_VLAN_RULE_NUM]      = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static u32 g_vlan_rule_user[SWAPI_VLAN_RULE_NUM] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

////////////////////////////////////////////////////////////////////////////////////

static atomic_t g_switch_inited                  = ATOMIC_INIT(0);
static atomic_t g_switch_allow_irq               = ATOMIC_INIT(0);
static atomic_t g_port_link_changed              = ATOMIC_INIT(0);

static bwan_member_t g_bwan_member[SWAPI_WAN_BRIDGE_NUM][ESW_EPHY_ID_MAX+1];

static u32 g_vlan_pvid_wan_untagged              = 2;

////////////////////////////////////////////////////////////////////////////////////

u32 get_ports_mask_lan(u32 include_cpu, int is_phy_id)
{
	u32 i, wan_bridge_mode, portmask_lan;

	wan_bridge_mode = g_wan_bridge_mode;

	portmask_lan = MASK_LAN_PORTS_ALL;
	if (include_cpu)
		portmask_lan |= MASK_LAN_PORT_CPU;

	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if (g_bwan_member[wan_bridge_mode][i].bwan)
			portmask_lan &= ~(1u << i);
	}

	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN) {
		if (is_phy_id)
			portmask_lan |= (1u << WAN_PORT_X);
		else
			portmask_lan |= MASK_WAN_PORT_X;
	}

	return portmask_lan;
}

static u32 get_ports_mask_wan(u32 include_cpu, int is_phy_id)
{
	u32 i, wan_bridge_mode, portmask_wan;

	wan_bridge_mode = g_wan_bridge_mode;
	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		return 0;

	if (is_phy_id)
		portmask_wan = (1u << WAN_PORT_X);
	else
		portmask_wan = MASK_WAN_PORT_X;

	if (include_cpu)
		portmask_wan |= MASK_WAN_PORT_CPU;

	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if (g_bwan_member[wan_bridge_mode][i].bwan)
			portmask_wan |= (1u << i);
	}

	return portmask_wan;
}

static u32 get_ports_mask_from_uapi(u32 port_mask_uapi, int is_phy_id)
{
	u32 gsw_ports_mask = 0;

	if (port_mask_uapi & SWAPI_PORTMASK_WAN) {
		if (is_phy_id)
			gsw_ports_mask |= (1u << WAN_PORT_X);
		else
			gsw_ports_mask |= MASK_WAN_PORT_X;
	}
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
	if (port_mask_uapi & SWAPI_PORTMASK_CPU_LAN)
		gsw_ports_mask |= MASK_LAN_PORT_CPU;
	if (port_mask_uapi & SWAPI_PORTMASK_CPU_WAN)
		gsw_ports_mask |= MASK_WAN_PORT_CPU;

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
		return LAN_PORT_CPU;
	case SWAPI_PORT_CPU_WAN:
		return WAN_PORT_CPU;
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
	case LAN_PORT_CPU:
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

static void esw_port_matrix_set(u32 port_id, u32 fwd_mask, u32 pvlan_ingress_mode)
{
	u32 reg_pcr;

	reg_pcr = esw_reg_get(REG_ESW_PORT_PCR_P0 + 0x100*port_id);
	reg_pcr &= ~(0xFF << 16);
	reg_pcr &= ~(0x03);
	reg_pcr |= ((fwd_mask & 0xFF) << 16);
	reg_pcr |= (pvlan_ingress_mode & 0x03);
	esw_reg_set(REG_ESW_PORT_PCR_P0 + 0x100*port_id, reg_pcr);
}

static void esw_port_ingress_mode_set(u32 port_id, u32 pvlan_ingress_mode)
{
	u32 reg_pcr;

	reg_pcr = esw_reg_get(REG_ESW_PORT_PCR_P0 + 0x100*port_id);
	reg_pcr &= ~(0x03);
	reg_pcr |= (pvlan_ingress_mode & 0x03);
	esw_reg_set(REG_ESW_PORT_PCR_P0 + 0x100*port_id, reg_pcr);
}

static void esw_port_attrib_set(u32 port_id, u32 port_attribute)
{
	u32 reg_pvc;

	reg_pvc = esw_reg_get(REG_ESW_PORT_PVC_P0 + 0x100*port_id);
	reg_pvc &= 0x0000FF3F;
	reg_pvc |= 0x81000000; // STAG VPID 8100
	reg_pvc |= ((port_attribute & 0x03) << 6);
	esw_reg_set(REG_ESW_PORT_PVC_P0 + 0x100*port_id, reg_pvc);
}

static void esw_port_accept_set(u32 port_id, u32 accept_frames)
{
	u32 reg_pvc;

	reg_pvc = esw_reg_get(REG_ESW_PORT_PVC_P0 + 0x100*port_id);
	reg_pvc &= 0xFFFFFFFC;
	reg_pvc |= (accept_frames & 0x03);
	esw_reg_set(REG_ESW_PORT_PVC_P0 + 0x100*port_id, reg_pvc);
}

static void esw_vlan_pvid_set(u32 port_id, u32 pvid, u32 prio)
{
	u32 reg_ppbv = (1u << 16) | ((prio & 0x7) << 13) | (pvid & 0xfff);

	esw_reg_set(REG_ESW_PORT_PPBV1_P0 + 0x100*port_id, reg_ppbv);
}

#if !defined (CONFIG_RAETH_ESW_IGMP_SNOOP_OFF)
void esw_igmp_flood_to_cpu(int flood_to_cpu)
{
	u32 reg_imc;

	reg_imc = esw_reg_get(REG_ESW_IMC);
	reg_imc &= ~((0x7<<28)|(0x7<<12));
	/* Note: TO_CPU applied to P5 port too and IGMP/MLD P5->WAN will be dropped */
#if !defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) && !defined (CONFIG_GE2_INTERNAL_GMAC_P5)
	if (flood_to_cpu)
		reg_imc |= (0x6<<28)|(0x6<<12);
#endif
	esw_reg_set(REG_ESW_IMC, reg_imc);
}
#endif

#if defined (CONFIG_RAETH_ESW_IGMP_SNOOP_HW)
static void esw_igmp_ports_config(u32 wan_bridge_mode)
{
	u32 i, reg_isc, mask_no_learn;

	mask_no_learn = 0;

	if (wan_bridge_mode != SWAPI_WAN_BRIDGE_DISABLE_WAN) {
		mask_no_learn = get_ports_mask_wan(0, 0);
		for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
			if ((mask_no_learn >> i) & 0x1)
				esw_reg_set(REG_ESW_PORT_PIC_P0 + 0x100*i, 0x8000);
		}
	}

	/* make CPU ports always static */
	mask_no_learn |= MASK_WAN_PORT_CPU;
	mask_no_learn |= MASK_LAN_PORT_CPU;

	if (g_igmp_snooping_enabled)
		mask_no_learn |= g_igmp_static_ports;
	else
		mask_no_learn |= get_ports_mask_lan(0, 0);

	reg_isc = esw_reg_get(REG_ESW_ISC);
	reg_isc &= ~0xFF1C00FF;
	reg_isc |= mask_no_learn;

	/* also enable upstream router port learning (in AP mode) */
	if (g_igmp_static_ports)
		reg_isc |= (1u << 19) | (1u << 18);

	esw_reg_set(REG_ESW_ISC, reg_isc);
}

static void esw_igmp_mld_snooping(u32 enable_igmp, u32 enable_mld)
{
	u32 i, mask_lan, dst_igmp, src_join, reg_pic, reg_pic_lan;
	u32 mask_static = g_igmp_static_ports;

	dst_igmp = 0;
	src_join = 0;
	reg_pic = (2u << 14);			// Robustness = 2

	if (enable_mld) {
		dst_igmp |= (1u << 9);		// IPM_33
		
		src_join |= (1u << 13);		// MLD_HW_LEAVE
		src_join |= (1u << 7);		// MLD2_JOIN_EN
		src_join |= (1u << 5);		// MLD_JOIN_EN
	}

	if (enable_igmp) {
		dst_igmp |= (1u << 8);		// IPM_01
		
		src_join |= (1u << 12);		// IGMP_HW_LEAVE
		src_join |= (1u << 6);		// IGMP3_JOIN_EN
		src_join |= (1u << 4);		// IGMP_JOIN_EN
	}

	mask_lan = get_ports_mask_lan(0, 0);
	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if ((mask_lan >> i) & 0x1) {
			reg_pic_lan = reg_pic;
			if ((mask_static >> i) & 0x1)
				reg_pic_lan |= dst_igmp;
			else
				reg_pic_lan |= src_join;
			esw_reg_set(REG_ESW_PORT_PIC_P0 + 0x100*i, reg_pic_lan);
		}
	}

	/* make CPU port always static */
	esw_reg_set(REG_ESW_PORT_PIC_P0 + 0x100*LAN_PORT_CPU, reg_pic | dst_igmp);
}
#endif

static void esw_mac_table_clear(void)
{
#if defined (CONFIG_MT7530_GSW)
	mt7530_gsw_mac_table_clear(0);
#if defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0) || defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || \
    defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
	mt7620_esw_mac_table_clear(0);
#endif
#else
	mt7620_esw_mac_table_clear(0);
#endif
}

static int esw_write_vtcr(u32 vtcr_cmd, u32 vtcr_val)
{
	u32 i, reg_vtcr;

	reg_vtcr = (vtcr_cmd << 12) | vtcr_val | 0x80000000;
	esw_reg_set(REG_ESW_VLAN_VTCR, reg_vtcr);

	for (i = 0; i < 200; i++) {
		udelay(100);
		reg_vtcr = esw_reg_get(REG_ESW_VLAN_VTCR);
		if (!(reg_vtcr & 0x80000000))
			return 0;
	}

	printk("%s: VTCR timeout!\n", MTK_ESW_DEVNAME);
	return -1;
}

#if !defined (CONFIG_MT7530_GSW)
static int esw_find_free_vlan_slot(u32 start_idx)
{
	u32 i, reg_vawd1;

	for (i = start_idx; i < 16; i++) {
		// read VAWD1
		if (esw_write_vtcr(0, i) != 0)
			continue;
		reg_vawd1 = esw_reg_get(REG_ESW_VLAN_VAWD1);
		if (!(reg_vawd1 & 1))
			return (int)i;
	}

	return -1;
}

static void esw_vlan_set_idx(u32 idx, u32 cvid, u32 svid, u32 mask_member, u32 mask_untag, u32 mask_swap, u32 fid)
{
	u32 i, reg_val, reg_val2;

	idx &= 0xf;
	cvid &= 0xfff;
	svid &= 0xfff;
	mask_member &= 0xff;
	mask_untag &= 0xff;
	mask_swap &= 0xff;
	fid &= 0x7;

	// set vlan identifier
	reg_val = esw_reg_get(REG_ESW_VLAN_ID_BASE + 4*(idx/2));
	if ((idx % 2) == 0) {
		reg_val &= 0xfff000;
		reg_val |= cvid;
	} else {
		reg_val &= 0x000fff;
		reg_val |= (cvid << 12);
	}
	esw_reg_set(REG_ESW_VLAN_ID_BASE + 4*(idx/2), reg_val);

	reg_val2 = 0;

	/* set vlan member */
	reg_val = 1;				// VALID
#if !ESW_USE_IVL_MODE
	if (fid > 0)
		reg_val |= (fid << 1);		// FID
	else
#endif
	reg_val |= (1u << 30);			// IVL=1
	reg_val |= (svid << 4);			// S_TAG
	reg_val |= (mask_member << 16);		// PORT_MEM
	if (svid)
		reg_val |= (1u << 27);		// COPY_PRI
	reg_val |= (1u << 28);			// VTAG_EN=1

	/* set vlan untag ports */
	for (i = 0; i < 8; i++) {
		if (!(mask_member & (1u << i)))
			continue;
		
		if (!(mask_untag & (1u << i))) {
			if (mask_swap & (1u << i))
				reg_val2 |= (0x1u << (i*2));	// EG_TAG=Swap
			else
				reg_val2 |= (0x2u << (i*2));	// EG_TAG=Tagged
		}
	}

	esw_reg_set(REG_ESW_VLAN_VAWD1, reg_val);
	esw_reg_set(REG_ESW_VLAN_VAWD2, reg_val2);

	esw_write_vtcr(1, idx);
}

static void esw_vlan_reset_table(void)
{
	u32 i;

	/* Reset VLAN mappings */
	for (i = 0; i < 8; i++)
		esw_reg_set(REG_ESW_VLAN_ID_BASE + 4*i, (((i<<1)+2) << 12) | ((i<<1)+1));

	/* Reset VLAN table from idx 1 to 15 */
	for(i = 1; i < 16; i++)
		esw_write_vtcr(2, i);
}
#else
static void esw_vlan_set(u32 cvid, u32 svid, u32 mask_member, u32 mask_untag, u32 mask_swap, u32 fid)
{
	u32 i, reg_val, reg_val2;

	cvid &= 0xfff;
	svid &= 0xfff;
	mask_member &= 0x7f;
	mask_untag &= 0x7f;
	mask_swap &= 0x7f;
	fid &= 0x7;

	reg_val2 = 0;

	// set vlan member
	reg_val = 1;				// VALID
#if !ESW_USE_IVL_MODE
	if (fid > 0)
		reg_val |= (fid << 1);		// FID
	else
#endif
	reg_val |= (1u << 30);			// IVL=1
	reg_val |= (svid << 4);			// S_TAG
	reg_val |= (mask_member << 16);		// PORT_MEM
	if (svid)
		reg_val |= (1u << 27);		// COPY_PRI
	reg_val |= (1u << 28);			// VTAG_EN=1

	/* set vlan untag ports */
	for (i = 0; i < 7; i++) {
		if (!(mask_member & (1u << i)))
			continue;
		
		if (!(mask_untag & (1u << i))) {
			if (mask_swap & (1u << i))
				reg_val2 |= (0x1u << (i*2));	// EG_TAG=Swap
			else
				reg_val2 |= (0x2u << (i*2));	// EG_TAG=Tagged
		}
	}

	esw_reg_set(REG_ESW_VLAN_VAWD1, reg_val);
	esw_reg_set(REG_ESW_VLAN_VAWD2, reg_val2);

	esw_write_vtcr(1, cvid);
	set_bit(cvid, g_vlan_pool);
}

static void esw_vlan_reset_table(void)
{
	u32 i;

	/* Reset VLAN table from VID #2 */
	for (i = 2; i < 4096; i++) {
		if (test_and_clear_bit(i, g_vlan_pool))
			esw_write_vtcr(2, i);
	}
}
#endif

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
	u32 i, cvid, next_fid, untg_vid, vlan_idx, vlan_filter_on;
#if !defined (CONFIG_MT7530_GSW)
	u32 next_vid = 3;
#endif
#if defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || defined (CONFIG_GE2_INTERNAL_GMAC_P5)
	pvlan_member_t pvlan_member_cpu_wan;
#endif

	untg_vid = 2;	// default PVID for untagged WAN traffic
	next_fid = 3;
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
#if !defined (CONFIG_MT7530_GSW)
		next_vid = find_free_min_pvid(pvid, untg_vid+1);
		
		/* this isolation mode possible only via MT7530 PORT_MATRIX */
		if (wan_bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_BETWEEN)
			wan_bwan_isolation = SWAPI_WAN_BWAN_ISOLATION_NONE;
#endif
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

#if defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || defined (CONFIG_GE2_INTERNAL_GMAC_P5)
	/* fill CPU WAN port (use PVID 2 for handle untagged traffic -> VID2) */
	pvlan_member_cpu_wan.pvid = untg_vid;
	pvlan_member_cpu_wan.prio = 0;
	pvlan_member_cpu_wan.tagg = 0;
#endif

	/* VID #1 */
	vlan_entry[0].valid = 1;
	vlan_entry[0].fid = 1;
	vlan_entry[0].cvid = 1;
	vlan_entry[0].port_member |= MASK_LAN_PORT_CPU;
#if defined (MT7530_P6_UNTAGGED)
	vlan_entry[0].port_untag  |= MASK_LAN_PORT_CPU;
#endif

	/* VID #2 */
	vlan_entry[1].valid = 1;
	vlan_entry[1].fid = 2;
	vlan_entry[1].cvid = untg_vid;
	vlan_entry[1].port_member |= (MASK_WAN_PORT_X | MASK_WAN_PORT_CPU);
	vlan_entry[1].port_untag  |=  MASK_WAN_PORT_X;

#if !defined (CONFIG_MT7530_GSW)
	/* MT7620 not support PORT_MATRIX in security mode */
	if (wan_bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_BETWEEN) {
		pvlan_member[WAN_PORT_X].pvid = next_vid;
		
		/* VID #3 */
		vlan_entry[2].valid = 1;
		vlan_entry[2].fid = next_fid;
		vlan_entry[2].cvid = next_vid;
		vlan_entry[2].svid = 2;		/* used under etag_ctrl=SWAP (CVID<->SVID) */
		vlan_entry[2].port_member |= (MASK_WAN_PORT_X | MASK_WAN_PORT_CPU);
		vlan_entry[2].port_untag  |=  MASK_WAN_PORT_X;
		vlan_entry[2].port_swap   |=  MASK_WAN_PORT_CPU;
		next_fid++;
		next_vid++;
	}
#endif

#if defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0) || defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || \
    defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
	/* clear vlan members on MT7620 ESW (slot idx 2..3) */
	mt7620_esw_vlan_clear_idx(2);
	mt7620_esw_vlan_clear_idx(3);

	/* update VID=2 members (P7|P6|P4) */
	mt7620_esw_vlan_set_idx(1, untg_vid, 0xd0);

	/* set P4 PVID */
	mt7620_esw_pvid_set(4, untg_vid, 0);
#endif

	/* check IPTV tagged */
	if (tagg[SWAPI_VLAN_RULE_WAN_IPTV]) {
		cvid = pvid[SWAPI_VLAN_RULE_WAN_IPTV];
		vlan_idx = find_vlan_slot(vlan_entry, 2, cvid);
		if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
			if (!vlan_entry[vlan_idx].valid) {
				vlan_entry[vlan_idx].valid = 1;
				vlan_entry[vlan_idx].fid = next_fid++;
				vlan_entry[vlan_idx].cvid = cvid;
				
#if defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0) || defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || \
    defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
				/* need add vlan members on MT7620 ESW P4 (ESW members P7|P6|P4) */
				mt7620_esw_vlan_set_idx(2, cvid, 0xd0);
#endif
			}
			vlan_entry[vlan_idx].port_member |= (MASK_WAN_PORT_X | MASK_WAN_PORT_CPU);
			pvlan_member[WAN_PORT_X].tagg = 1;
		}
	}

	/* check INET tagged */
	if (tagg[SWAPI_VLAN_RULE_WAN_INET]) {
		cvid = pvid[SWAPI_VLAN_RULE_WAN_INET];
		vlan_idx = find_vlan_slot(vlan_entry, 2, cvid);
		if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
			if (!vlan_entry[vlan_idx].valid) {
				vlan_entry[vlan_idx].valid = 1;
				vlan_entry[vlan_idx].fid = next_fid++;
				vlan_entry[vlan_idx].cvid = cvid;
				
#if defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0) || defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || \
    defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
				/* need add vlan members on MT7620 ESW P4 (ESW members P7|P6|P4) */
				mt7620_esw_vlan_set_idx(3, cvid, 0xd0);
#endif
			}
			vlan_entry[vlan_idx].port_member |= (MASK_WAN_PORT_X | MASK_WAN_PORT_CPU);
			pvlan_member[WAN_PORT_X].tagg = 1;
		}
	}

#if defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || defined (CONFIG_GE2_INTERNAL_GMAC_P5)
	/* if INET and IPTV tagged with common VID, untag WAN_CPU + use PVID */
	if (tagg[SWAPI_VLAN_RULE_WAN_INET] && pvid[SWAPI_VLAN_RULE_WAN_INET] == pvid[SWAPI_VLAN_RULE_WAN_IPTV]) {
		/* update VID #2 members (do not forward untagged packets to WAN_CPU) */
		vlan_entry[1].port_member &= ~(MASK_WAN_PORT_CPU);
		cvid = pvid[SWAPI_VLAN_RULE_WAN_INET];
		vlan_idx = find_vlan_slot(vlan_entry, 2, cvid);
		if (vlan_idx <= VLAN_ENTRY_ID_MAX && vlan_entry[vlan_idx].valid)
			vlan_entry[vlan_idx].port_untag |= MASK_WAN_PORT_CPU;
		pvlan_member_cpu_wan.pvid = cvid;
		pvlan_member_cpu_wan.prio = prio[SWAPI_VLAN_RULE_WAN_IPTV];
	} else if (!tagg[SWAPI_VLAN_RULE_WAN_INET] && !tagg[SWAPI_VLAN_RULE_WAN_IPTV]) {
		/* update VID #2 untag members */
		vlan_entry[1].port_untag |= MASK_WAN_PORT_CPU;
	} else {
		/* mark CPU WAN as tagged */
		pvlan_member_cpu_wan.tagg = 1;
	}
#endif

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
			
#if !defined (CONFIG_MT7530_GSW)
			/* MT7620 not support PORT_MATRIX in security mode */
			if (wan_bwan_isolation != SWAPI_WAN_BWAN_ISOLATION_NONE) {
				vlan_idx = find_vlan_slot(vlan_entry, 2, next_vid);
				if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
					pvlan_member[i].pvid = next_vid;
					
					vlan_entry[vlan_idx].valid = 1;
					vlan_entry[vlan_idx].fid = next_fid;
					vlan_entry[vlan_idx].cvid = next_vid;
					
					if (wan_bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_BETWEEN) {
						vlan_entry[vlan_idx].svid = 2;	/* used under etag_ctrl=SWAP (CVID<->SVID) */
						vlan_entry[vlan_idx].port_member |= ((1u << i) | MASK_WAN_PORT_CPU);
						vlan_entry[vlan_idx].port_untag  |=  (1u << i);
						vlan_entry[vlan_idx].port_swap   |=  MASK_WAN_PORT_CPU;
						next_fid++;
						next_vid++;
					} else {
						vlan_entry[vlan_idx].port_member |= ((1u << i) | MASK_WAN_PORT_X);
						vlan_entry[vlan_idx].port_untag  |= ((1u << i) | MASK_WAN_PORT_X);
					}
				}
			}
#endif
		} else {
			cvid = pvid[rule_id];
			vlan_idx = find_vlan_slot(vlan_entry, 2, cvid);
			if (vlan_idx <= VLAN_ENTRY_ID_MAX) {
				if (!vlan_entry[vlan_idx].valid) {
					vlan_entry[vlan_idx].valid = 1;
					vlan_entry[vlan_idx].fid = next_fid++;
					vlan_entry[vlan_idx].cvid = cvid;
				}
				vlan_entry[vlan_idx].port_member |= ((1u << i) | MASK_WAN_PORT_X);
#if !defined (RAETH_GE2_MAC_TO_GPHY)
				if (wan_bwan_isolation != SWAPI_WAN_BWAN_ISOLATION_FROM_CPU)
#endif
				{
					vlan_entry[vlan_idx].port_member |= MASK_WAN_PORT_CPU;
#if defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || defined (CONFIG_GE2_INTERNAL_GMAC_P5)
					/* mark CPU WAN as tagged */
					pvlan_member_cpu_wan.tagg = 1;
#endif
				}
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

#if defined (RAETH_GE2_MAC_TO_GPHY)
	for (i = 0; i <= VLAN_ENTRY_ID_MAX; i++) {
		if (!vlan_entry[i].valid)
			continue;
		if (!vlan_entry[i].port_member)
			continue;
		if (wan_bridge_mode != SWAPI_WAN_BRIDGE_DISABLE) {
			vlan_entry[i].port_member |= MASK_LAN_PORT_CPU;
			vlan_entry[i].port_untag &= ~MASK_LAN_PORT_CPU;
		}
	}
#endif

#if defined (ESW_PORT_PPE)
	/* add PPE port to members with CPU port */
	for (i = 0; i <= VLAN_ENTRY_ID_MAX; i++) {
		if (!vlan_entry[i].valid)
			continue;
		if (vlan_entry[i].port_member & (1u << ESW_PORT_CPU))
			vlan_entry[i].port_member |= (1u << ESW_PORT_PPE);
		if (vlan_entry[i].port_swap & (1u << ESW_PORT_CPU))
			vlan_entry[i].port_swap |= (1u << ESW_PORT_PPE);
	}
#endif

	/* configure physical LAN/WAN ports */
	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if ((1u << i) & ESW_MASK_EXCLUDE)
			continue;
		esw_vlan_pvid_set(i, pvlan_member[i].pvid, pvlan_member[i].prio);
		if (!pvlan_member[i].tagg) {
			esw_port_attrib_set(i, PORT_ATTRIBUTE_TRANSPARENT);
			esw_port_accept_set(i, PORT_ACCEPT_FRAMES_UNTAGGED);
		} else {
			esw_port_attrib_set(i, PORT_ATTRIBUTE_USER);
			esw_port_accept_set(i, PORT_ACCEPT_FRAMES_ALL);
		}
	}

	/* configure CPU LAN port */
	esw_vlan_pvid_set(LAN_PORT_CPU, 1, 0);
	esw_port_accept_set(LAN_PORT_CPU, PORT_ACCEPT_FRAMES_ALL);
#if defined (RAETH_GE2_MAC_TO_GPHY)
	esw_port_attrib_set(LAN_PORT_CPU, (wan_bridge_mode != SWAPI_WAN_BRIDGE_DISABLE) ? PORT_ATTRIBUTE_USER : PORT_ATTRIBUTE_TRANSPARENT);
#elif defined (MT7530_P6_UNTAGGED)
	esw_port_attrib_set(LAN_PORT_CPU, PORT_ATTRIBUTE_TRANSPARENT);
#else
	esw_port_attrib_set(LAN_PORT_CPU, PORT_ATTRIBUTE_USER);
#endif

#if defined (CONFIG_RALINK_MT7620) && defined (ESW_PORT_PPE)
#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	if (ra_sw_nat_hook_rx != NULL)
		esw_port_ingress_mode_set(LAN_PORT_CPU, PVLAN_INGRESS_MODE_FALLBACK);
	else
#endif
#endif
		esw_port_ingress_mode_set(LAN_PORT_CPU, PVLAN_INGRESS_MODE_SECURITY);

	/* configure CPU WAN port */
#if defined (CONFIG_P4_RGMII_TO_MT7530_GMAC_P5) || defined (CONFIG_GE2_INTERNAL_GMAC_P5)
	/* [MT7620 P4 -> MT7530 P5] or [MT7621 GE2 -> MT7530 P5] */
	esw_vlan_pvid_set(WAN_PORT_CPU, pvlan_member_cpu_wan.pvid, pvlan_member_cpu_wan.prio);
	esw_port_accept_set(WAN_PORT_CPU, PORT_ACCEPT_FRAMES_ALL);
	esw_port_attrib_set(WAN_PORT_CPU, (pvlan_member_cpu_wan.pvid != untg_vid || pvlan_member_cpu_wan.tagg) ? PORT_ATTRIBUTE_USER : PORT_ATTRIBUTE_TRANSPARENT);
	esw_port_ingress_mode_set(WAN_PORT_CPU, PVLAN_INGRESS_MODE_SECURITY);
#endif

	/* reset VLAN table */
	esw_vlan_reset_table();

	/* fill VLAN table */
	for (i = 0; i <= VLAN_ENTRY_ID_MAX; i++) {
		if (!vlan_entry[i].valid)
			continue;
		if (!vlan_entry[i].port_member)
			continue;
#if !defined (CONFIG_MT7530_GSW)
		esw_vlan_set_idx(i, vlan_entry[i].cvid, vlan_entry[i].svid,
			vlan_entry[i].port_member, vlan_entry[i].port_untag, vlan_entry[i].port_swap, vlan_entry[i].fid);
#else
		esw_vlan_set(vlan_entry[i].cvid, vlan_entry[i].svid,
			vlan_entry[i].port_member, vlan_entry[i].port_untag, 0, vlan_entry[i].fid);
#endif
	}

	/* save VLAN rules */
	for (i = 0; i < SWAPI_VLAN_RULE_NUM; i++)
		g_vlan_rule[i] = g_vlan_rule_user[i];
}

static void esw_vlan_init_vid1(void)
{
	u32 i, port_member, port_untag;

	port_member = 0xFF;
	port_member &= ~(ESW_MASK_EXCLUDE);
	port_untag = port_member;
#if defined (ESW_PORT_PPE)
	port_untag &= ~(1u << ESW_PORT_PPE);
#endif

	/* configure physical LAN ports */
	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if ((1u << i) & ESW_MASK_EXCLUDE)
			continue;
		esw_vlan_pvid_set(i, 1, 0);
		esw_port_attrib_set(i, PORT_ATTRIBUTE_TRANSPARENT);
		esw_port_accept_set(i, PORT_ACCEPT_FRAMES_UNTAGGED);
	}

	/* configure CPU port */
	esw_vlan_pvid_set(LAN_PORT_CPU, 1, 0);
	esw_port_accept_set(LAN_PORT_CPU, PORT_ACCEPT_FRAMES_ALL);
#if defined (CONFIG_RALINK_MT7620) && !defined (MT7530_P5_ENABLED)
	port_untag &= ~(MASK_LAN_PORT_CPU);
	esw_port_attrib_set(LAN_PORT_CPU, PORT_ATTRIBUTE_USER);
#else
	esw_port_attrib_set(LAN_PORT_CPU, PORT_ATTRIBUTE_TRANSPARENT);
#endif
#if defined (CONFIG_RALINK_MT7620) && defined (ESW_PORT_PPE)
#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	if (ra_sw_nat_hook_rx != NULL)
		esw_port_ingress_mode_set(LAN_PORT_CPU, PVLAN_INGRESS_MODE_FALLBACK);
	else
#endif
#endif
		esw_port_ingress_mode_set(LAN_PORT_CPU, PVLAN_INGRESS_MODE_SECURITY);

	/* reset VLAN table */
	esw_vlan_reset_table();

	/* set all ports to VLAN 1 member (no SVID, no SWAP) */
#if !defined (CONFIG_MT7530_GSW)
	esw_vlan_set_idx(0, 1, 0, port_member, port_untag, 0, 1);
#else
	esw_vlan_set(1, 0, port_member, port_untag, 0, 1);
#endif
}

#if defined (CONFIG_MT7530_GSW)
static void esw_mask_bridge_isolate(u32 wan_bridge_mode, u32 wan_bwan_isolation)
{
	u32 i;
	u32 fwd_mask_bwan_lan;
	u32 fwd_mask_lan, fwd_mask_wan, fwd_mask;

	fwd_mask_lan = get_ports_mask_lan(1, 0);

	/* LAN forward mask */
	for (i = 0; i <= ESW_PORT_ID_MAX; i++) {
		if ((fwd_mask_lan >> i) & 0x1) {
			fwd_mask = fwd_mask_lan;
			/* set all port ingress to security mode (VLAN + fwd_mask) */
			esw_port_matrix_set(i, fwd_mask, PVLAN_INGRESS_MODE_SECURITY);
		}
	}

	/* clear forward mask for P5 */
#if (MASK_WAN_PORT_CPU == 0)
	esw_port_matrix_set(5, 0, PVLAN_INGRESS_MODE_SECURITY);
#endif

	/* clear forward mask for P4/P0 */
#if (MASK_WAN_PORT_X == 0 && WAN_PORT_X < 5)
	esw_port_matrix_set(WAN_PORT_X, 0, PVLAN_INGRESS_MODE_SECURITY);
#endif

	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		return;

	fwd_mask_wan = get_ports_mask_wan(1, 0);
	fwd_mask_bwan_lan = fwd_mask_wan & ~(MASK_WAN_PORT_X|MASK_WAN_PORT_CPU);

#if defined (RAETH_GE2_MAC_TO_GPHY)
	if (wan_bridge_mode != SWAPI_WAN_BRIDGE_DISABLE)
		fwd_mask_wan |= MASK_LAN_PORT_CPU;
#endif

	/* WAN forward mask */
	for (i = 0; i <= ESW_PORT_ID_MAX; i++) {
		if ((fwd_mask_wan >> i) & 0x1) {
			fwd_mask = fwd_mask_wan;
#if defined (RAETH_GE2_MAC_TO_GPHY)
			/* force add all LAN ports to forward from CPU */
			if (i == LAN_PORT_CPU)
				fwd_mask |= MASK_LAN_PORTS_ALL;
#else
#if !defined (MT7530_P6_UNTAGGED)
			/* force add all LAN ports to forward from CPU */
			if (i == WAN_PORT_CPU)
				fwd_mask |= MASK_LAN_PORTS_ALL;
#endif
			if (wan_bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_FROM_CPU) {
				if (i == WAN_PORT_CPU)
					fwd_mask &= ~fwd_mask_bwan_lan;
				else if ((1u << i) & MASK_LAN_PORTS_ALL)
					fwd_mask &= ~MASK_WAN_PORT_CPU;
			} else
#endif
			if (wan_bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_BETWEEN) {
				if (i <= ESW_EPHY_ID_MAX) {
					fwd_mask &= ~(MASK_WAN_PORT_X|MASK_LAN_PORTS_ALL);
					fwd_mask |= (1u << i);
				}
			}
			
			/* set all port ingress to security mode (VLAN + fwd_mask) */
			esw_port_matrix_set(i, fwd_mask, PVLAN_INGRESS_MODE_SECURITY);
		}
	}
}
#else
static void esw_mask_bridge_isolate(void)
{
	u32 i, fwd_mask;

	fwd_mask = 0xFF;
	fwd_mask &= ~(ESW_MASK_EXCLUDE);

	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if ((1u << i) & ESW_MASK_EXCLUDE)
			continue;
		/* set all port ingress to security mode */
		esw_port_matrix_set(i, fwd_mask, PVLAN_INGRESS_MODE_SECURITY);
	}
}
#endif

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

	if (bridge_changed) {
#if defined (CONFIG_RAETH_ESW_IGMP_SNOOP_HW)
		esw_igmp_ports_config(wan_bridge_mode);
		if (g_igmp_snooping_enabled)
			esw_igmp_mld_snooping(1, 1);
#endif
		esw_show_bridge_partitions(wan_bridge_mode);
	}

	esw_mac_table_clear();
}

static void esw_mac_to_phy_enable(void)
{
	u32 i, mac_mask, reg_pmcr;

	/* full AN */
	reg_pmcr = 0x00056330;

	mac_mask = MASK_WAN_PORT_X | MASK_LAN_PORTS_ALL;

	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if ((mac_mask >> i) & 0x1)
			esw_reg_set(REG_ESW_MAC_PMCR_P0 + 0x100*i, reg_pmcr);
	}
}

static void esw_soft_reset(void)
{
#if !defined (CONFIG_MT7530_GSW)
	u32 reg_agc;

	/* Reset ARL engine */
	reg_agc = esw_reg_get(REG_ESW_AGC);
	esw_reg_set(REG_ESW_AGC, (reg_agc & ~0x1));
	udelay(100);
	reg_agc = esw_reg_get(REG_ESW_AGC);
	esw_reg_set(REG_ESW_AGC, (reg_agc | 0x1));
#else
	/* disable switch interrupts */
	esw_irq_disable();

	/* disable CPU ports P5/P6 link */
	esw_reg_set(0x3500, 0x8000);
	esw_reg_set(0x3600, 0x8000);

	/* reset MT7530 */
	esw_reg_set(0x7000, 0x3);
	udelay(100);

	/* base init MT7530 */
	mt7530_gsw_init();

	/* enable switch interrupts */
	esw_irq_init();
	esw_irq_enable();
#endif

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
#if defined (CONFIG_P4_MAC_TO_PHY_MODE)
	if (phy_port_id == 4)
		phy_mdio_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
#endif
#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
	if (phy_port_id == 5)
		phy_mdio_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#elif defined (CONFIG_GE2_RGMII_AN)
	if (phy_port_id == 5)
		phy_mdio_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
#endif

	i_port_speed = (g_port_link_mode[phy_port_id] & 0x0F);
	if (i_port_speed == SWAPI_LINK_SPEED_MODE_FORCE_POWER_OFF)
		power_on = 0;

	is_power_on = 0;
	if (mii_mgr_read(phy_mdio_addr, 0, &esw_phy_mcr)) {
		is_power_on = (esw_phy_mcr & (1<<11)) ? 0 : 1;
		esw_phy_mcr &= ~((1<<11)|(1<<9));
		
		/* fix PHY init after buggy Uboot 4.3.0.0 */
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

	/* block PHY changes */
	atomic_set(&g_switch_allow_irq, 0);

	/* down all PHY ports */
	for (i = 0; i <= ESW_EPHY_ID_MAX; i++)
		power_changed |= esw_port_phy_power(i, 0, 0);

	if (power_changed)
		msleep(500);
}

static void esw_storm_control(u32 port_id, int set_bcast, int set_mcast, int set_ucast, u32 rate_mbps)
{
	u32 reg_bsr = 0;
	u32 rate_unit_1000;
	u32 rate_unit_100;
	u32 rate_unit_10;

	if (rate_mbps > 0) {
		if (rate_mbps > (0xff * 4))
			rate_mbps = (0xff * 4);
		
		rate_unit_1000 = rate_mbps;
		rate_unit_100 = (rate_mbps < 90) ? rate_mbps : 90;
		rate_unit_10 = (rate_mbps < 9) ? rate_mbps : 9;
		
		reg_bsr |= BIT(31);			// STRM_MODE = Rate-based
		if (set_bcast)
			reg_bsr |= BIT(30);		// STRM_BC_INC
		if (set_mcast)
			reg_bsr |= BIT(29);		// STRM_MC_INC
		if (set_ucast)
			reg_bsr |= BIT(28);		// STRM_UC_INC
		
		if (rate_mbps > 0xff) {
			rate_unit_1000 >>= 2;
			rate_unit_100 >>= 2;
			rate_unit_10 >>= 2;
			reg_bsr |= (3u << 24);		// STRM_UNIT = 4 Mbps
		} else {
			reg_bsr |= (2u << 24);		// STRM_UNIT = 1 Mbps
		}
		
		reg_bsr |= (rate_unit_1000 << 16);	// STORM_1G
		reg_bsr |= (rate_unit_100 << 8);	// STORM_100M
		reg_bsr |= (rate_unit_10);		// STORM_10M
	}

	esw_reg_set(REG_ESW_PORT_BSR_P0 + 0x100*port_id, reg_bsr);
}

static void esw_jumbo_control(u32 jumbo_frames_enabled)
{
	u32 reg_gmaccr;

	reg_gmaccr = esw_reg_get(REG_ESW_MAC_GMACCR);
	reg_gmaccr &= ~(0x3f);
	reg_gmaccr |= (9u << 2);		// MAX_RX_JUMBO = 9 KB

	if (jumbo_frames_enabled) {
		reg_gmaccr |= 0x3;		// MAX_RX_JUMBO
	} else {
		reg_gmaccr |= 0x1;		// 1536 bytes
	}

	esw_reg_set(REG_ESW_MAC_GMACCR, reg_gmaccr);
}

static void esw_eee_control(u32 is_eee_enabled)
{
	u32 i, port_phy_power[ESW_EPHY_ID_MAX+1];

	/* store PHY power state before down */
	memcpy(port_phy_power, g_port_phy_power, sizeof(g_port_phy_power));

	/* disable PHY ports link */
	power_down_all_phy();

	/* set EEE on switch ports */
#if defined (CONFIG_MT7530_GSW)
	mt7530_gsw_eee_enable(is_eee_enabled);
#else
	mt7620_esw_eee_enable(is_eee_enabled);
#endif

	/* set EEE on external EPHY */
#if defined (CONFIG_P4_MAC_TO_PHY_MODE)
	ext_gphy_eee_enable(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, is_eee_enabled);
#endif
#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
	ext_gphy_eee_enable(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, is_eee_enabled);
#elif defined (CONFIG_GE2_RGMII_AN)
	ext_gphy_eee_enable(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, is_eee_enabled);
#endif

	/* allow PHY changes */
	atomic_set(&g_switch_allow_irq, 1);

	/* restore PHY ports link */
	for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
		if (port_phy_power[i])
			esw_port_phy_power(i, 1, 1);
	}
}

static void esw_led_mode(u32 led_mode)
{
#if !defined (CONFIG_MT7530_GSW)
// LED mode #0: Link + Activity
// LED mode #1: Link 100
// LED mode #2: Link 100/10
// LED mode #3: Disable

	u32 reg_gpc1;

	reg_gpc1 = esw_reg_get(REG_ESW_GPC1);
	reg_gpc1 &= ~(0xC000);

	if (led_mode == SWAPI_LED_OFF)
		reg_gpc1 |= 0xC000;

	esw_reg_set(REG_ESW_GPC1, reg_gpc1);
#else
	u32 reg_ledc;

	reg_ledc = esw_reg_get(0x7d00);
	reg_ledc |= 0x77777;

	if (led_mode == SWAPI_LED_OFF)
		reg_ledc &= ~(0x77777);

	esw_reg_set(0x7d00, reg_ledc);
#endif
}

static u32 esw_status_link_port(u32 port_id)
{
	u32 reg_pmsr;

#if defined (RAETH_GE2_MAC_TO_GPHY)
	if (port_id == WAN_PORT_X)
		reg_pmsr = sysRegRead(REG_ETH_GE2_MAC_STATUS);	// read state from GE2
	else
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0) || defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
	if (port_id == WAN_PORT_X)
		reg_pmsr = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_MAC_PMSR_P0 + 0x100*4);	// read state from P4
	else
#endif
		reg_pmsr = esw_reg_get(REG_ESW_MAC_PMSR_P0 + 0x100*port_id);

	return (reg_pmsr & 0x1);
}

static u32 esw_status_speed_port_uapi(u32 port_id_uapi)
{
	u32 port_link, port_duplex, port_speed;
	u32 port_eee, port_fc_rx, port_fc_tx;
	u32 reg_pmsr;
	u32 port_id = get_port_from_uapi(port_id_uapi);

	if (port_id > ESW_EPHY_ID_MAX)
		return 0;

#if defined (RAETH_GE2_MAC_TO_GPHY)
	if (port_id == WAN_PORT_X) {
		reg_pmsr = sysRegRead(REG_ETH_GE2_MAC_STATUS);	// read state from GE2
#if defined (CONFIG_GE2_RGMII_AN)
		reg_pmsr |= ext_gphy_fill_pmsr(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2);
#endif
	} else
#elif defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P0) || defined (CONFIG_P4_MAC_TO_MT7530_GPHY_P4)
	if (port_id == WAN_PORT_X)
		reg_pmsr = sysRegRead(RALINK_ETH_SW_BASE+REG_ESW_MAC_PMSR_P0 + 0x100*4);	// read state from P4
	else
#endif
		reg_pmsr = esw_reg_get(REG_ESW_MAC_PMSR_P0 + 0x100*port_id);

	port_link = (reg_pmsr & 0x1);

	if (!port_link)
		return 0;

	port_duplex = (reg_pmsr >> 1) & 0x1;
	port_speed  = (reg_pmsr >> 2) & 0x3;
	port_fc_tx  = (reg_pmsr >> 4) & 0x1;
	port_fc_rx  = (reg_pmsr >> 5) & 0x1;
	port_eee    = (reg_pmsr >> 6) & 0x3;

	return ((port_link << 16) | (port_eee << 11) | (port_fc_rx << 10) | (port_fc_tx << 9) | (port_duplex << 8) | port_speed);
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
		portmask = get_ports_mask_wan(0, 1);
	else
		portmask = get_ports_mask_lan(0, 1);

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

#if defined (RAETH_GE2_MAC_TO_GPHY)
extern int VirtualIF_get_bytes(port_bytes_t *pb);
#endif

static int esw_status_mib_port_uapi(u32 port_id_uapi, esw_mib_counters_t *mibc)
{
	ULARGE_INTEGER rx_goct, tx_goct;
	u32 port_id = get_port_from_uapi(port_id_uapi);

	if (port_id > ESW_PORT_ID_MAX)
		return -EINVAL;

#if defined (RAETH_GE2_MAC_TO_GPHY)
	if (port_id == WAN_PORT_X || port_id == WAN_PORT_CPU) {
		port_bytes_t pb;
		memset(mibc, 0, sizeof(esw_mib_counters_t));
		if (VirtualIF_get_bytes(&pb) == 0) {
			mibc->TxGoodOctets = pb.TX;
			mibc->RxGoodOctets = pb.RX;
		}
		return 0;
	}
#endif

	rx_goct.u.LowPart = esw_get_port_mib_rgoc(port_id, &rx_goct.u.HighPart);
	tx_goct.u.LowPart = esw_get_port_mib_tgoc(port_id, &tx_goct.u.HighPart);

	mibc->TxGoodOctets	= tx_goct.QuadPart;
	mibc->RxGoodOctets	= rx_goct.QuadPart;

#if defined (CONFIG_MT7530_GSW)
	mibc->TxDropFrames	= esw_reg_get(0x4000 + 0x100*port_id);
	mibc->TxCRCError	= esw_reg_get(0x4004 + 0x100*port_id);
	mibc->TxUcastFrames	= esw_reg_get(0x4008 + 0x100*port_id);
	mibc->TxMcastFrames	= esw_reg_get(0x400c + 0x100*port_id);
	mibc->TxBcastFrames	= esw_reg_get(0x4010 + 0x100*port_id);
	mibc->TxCollision	= esw_reg_get(0x4014 + 0x100*port_id);
	mibc->TxPauseFrames	= esw_reg_get(0x402c + 0x100*port_id);

	mibc->RxDropFrames	= esw_reg_get(0x4060 + 0x100*port_id);
	mibc->RxFilterFrames	= esw_reg_get(0x4064 + 0x100*port_id);
	mibc->RxUcastFrames	= esw_reg_get(0x4068 + 0x100*port_id);
	mibc->RxMcastFrames	= esw_reg_get(0x406c + 0x100*port_id);
	mibc->RxBcastFrames	= esw_reg_get(0x4070 + 0x100*port_id);
	mibc->RxAligmentError	= esw_reg_get(0x4074 + 0x100*port_id);
	mibc->RxCRCError	= esw_reg_get(0x4078 + 0x100*port_id);
//	mibc->RxUndersizeError	= esw_reg_get(0x407c + 0x100*port_id);
//	mibc->RxFragmentError	= esw_reg_get(0x4080 + 0x100*port_id);
//	mibc->RxOversizeError	= esw_reg_get(0x4084 + 0x100*port_id);
//	mibc->RxJabberError	= esw_reg_get(0x4088 + 0x100*port_id);
	mibc->RxPauseFrames	= esw_reg_get(0x408c + 0x100*port_id);
#else
	mibc->TxBadOctets	= esw_get_port_mib_tboc(port_id);
	mibc->TxGoodFrames	= esw_get_port_mib_tgpc(port_id);
	mibc->TxBadFrames	= esw_get_port_mib_tbpc(port_id);
	mibc->TxDropFrames	= esw_get_port_mib_tepc(port_id);

	mibc->RxBadOctets	= esw_get_port_mib_rboc(port_id);
	mibc->RxGoodFrames	= esw_get_port_mib_rgpc(port_id);
	mibc->RxBadFrames	= esw_get_port_mib_rbpc(port_id);
	mibc->RxDropFramesErr	= esw_get_port_mib_repc(port_id);
	mibc->RxDropFramesFilter= esw_get_port_mib_rfpc(port_id);
#endif
	return 0;
}

static int esw_status_bytes_port_uapi(u32 port_id_uapi, port_bytes_t *pb)
{
	ULARGE_INTEGER rx_goct, tx_goct;
	u32 port_id = get_port_from_uapi(port_id_uapi);

	if (port_id > ESW_PORT_ID_MAX)
		return -EINVAL;

#if defined (RAETH_GE2_MAC_TO_GPHY)
	if (port_id == WAN_PORT_X || port_id == WAN_PORT_CPU)
		return VirtualIF_get_bytes(pb);
#endif

	rx_goct.u.LowPart = esw_get_port_mib_rgoc(port_id, &rx_goct.u.HighPart);
	tx_goct.u.LowPart = esw_get_port_mib_tgoc(port_id, &tx_goct.u.HighPart);

	pb->RX = rx_goct.QuadPart;
	pb->TX = tx_goct.QuadPart;

	return 0;
}

static void esw_status_mib_reset(void)
{
	u32 mib_val;
#if defined (CONFIG_MT7530_GSW)
	mib_val = esw_reg_get(0x4fe0);
	esw_reg_set(0x4fe0, mib_val & ~BIT(31));
	esw_reg_set(0x4fe0, mib_val |  BIT(31));
#else
	mib_val = esw_reg_get(REG_ESW_MIB_MIBCNTEN);
	esw_reg_set(REG_ESW_MIB_MIBCNTEN, mib_val & ~(0xff << 24));
	esw_reg_set(REG_ESW_MIB_MIBCNTEN, mib_val |  (0xff << 24));
	esw_mib_init();
#endif
}

static void change_ports_power(u32 power_on, u32 ports_mask)
{
	u32 i;

	ports_mask = get_ports_mask_from_uapi(ports_mask, 1);

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
		ports_mask = get_ports_mask_wan(0, 1);
	else
		ports_mask = get_ports_mask_lan(0, 1);

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

#if defined (CONFIG_MT7530_GSW)
	if (bridge_changed || br_iso_changed)
		esw_mask_bridge_isolate(wan_bridge_mode, wan_bwan_isolation);
#endif

	esw_vlan_bridge_isolate(wan_bridge_mode, wan_bwan_isolation, bridge_changed, br_iso_changed, vlan_rule_changed);

	if (power_changed)
		change_wan_lan_ports_power(1, 1);

	return 0;
}

#if defined (CONFIG_MT7530_GSW)
static int override_port_matrix_uapi(u32 port_id_uapi, u32 fwd_ports_mask)
{
	u32 fwd_mask;
	u32 port_id = get_port_from_uapi(port_id_uapi);

	if (port_id > ESW_PORT_ID_MAX)
		return -EINVAL;

	fwd_ports_mask &= 0xFF;

	fwd_mask = get_ports_mask_from_uapi(fwd_ports_mask, 0);

	/* set port ingress to security mode (VLAN + fwd_mask) */
	esw_port_matrix_set(port_id, fwd_mask, PVLAN_INGRESS_MODE_SECURITY);

	printk("%s - port ID %d forward mask: %04X\n", MTK_ESW_DEVNAME, port_id, fwd_mask);

	return 0;
}
#endif

static void vlan_accept_port_mode(u32 accept_mode, u32 port_mask)
{
	u32 i, admit_frames = PORT_ACCEPT_FRAMES_ALL;

	switch (accept_mode)
	{
	case SWAPI_VLAN_ACCEPT_FRAMES_UNTAG_ONLY:
		admit_frames = PORT_ACCEPT_FRAMES_UNTAGGED;
		break;
	case SWAPI_VLAN_ACCEPT_FRAMES_TAG_ONLY:
		admit_frames = PORT_ACCEPT_FRAMES_TAGGED;
		break;
	}

	port_mask = get_ports_mask_from_uapi(port_mask, 0);

	for (i = 0; i <= ESW_PORT_ID_MAX; i++) {
		if ((port_mask >> i) & 0x1)
			esw_port_accept_set(i, admit_frames);
	}
}

static void vlan_create_entry(u32 vlan4k_info, u32 vlan4k_mask, int set_port_vid)
{
	u32 i, cvid, prio, fid;
	u32 mask_member, mask_untag;
#if !defined (CONFIG_MT7530_GSW)
	u32 vlan_table_idx;
#endif

	cvid = (vlan4k_info & 0x0FFF);
	prio = ((vlan4k_info >> 12) & 0x7);
	fid  = ((vlan4k_info >> 16) & 0xFF);
	mask_member = get_ports_mask_from_uapi((vlan4k_mask & 0xFFFF), 0);
	mask_untag  = get_ports_mask_from_uapi((vlan4k_mask >> 16) & 0xFFFF, 0);
#if defined (ESW_PORT_PPE)
	if (mask_member & (1u << ESW_PORT_CPU))
		mask_member |= (1u << ESW_PORT_PPE);
#endif
	if (cvid < 1)
		cvid = 1;

#if !defined (CONFIG_MT7530_GSW)
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
	esw_vlan_set_idx(vlan_table_idx, cvid, 0, mask_member, mask_untag, 0, fid);
#else
	/* set vlan table */
	esw_vlan_set(cvid, 0, mask_member, mask_untag, 0, fid);
#endif

	/* set ports attrib */
	for (i = 0; i <= ESW_PORT_ID_MAX; i++) {
		if ((1u << i) & mask_member) {
			if (!((1u << i) & mask_untag)) {
				esw_port_attrib_set(i, PORT_ATTRIBUTE_USER);
			} else {
				if (set_port_vid)
					esw_vlan_pvid_set(i, cvid, prio);
				esw_port_attrib_set(i, PORT_ATTRIBUTE_TRANSPARENT);
			}
		}
	}

	printk("%s - create vlan %s: vid=[%d], prio=[%d], member=[0x%02X], untag=[0x%02X], fid=[%d]\n",
			MTK_ESW_DEVNAME, (set_port_vid) ? "ports" : "entry", cvid, prio, mask_member, mask_untag, fid);
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
#if defined (CONFIG_P4_MAC_TO_PHY_MODE)
	if (port_id == 4)
		phy_mdio_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
#endif
#if defined (CONFIG_P5_MAC_TO_PHY_MODE)
	if (port_id == 5)
		phy_mdio_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
#elif defined (CONFIG_GE2_RGMII_AN)
	if (port_id == 5)
		phy_mdio_addr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
#endif

	/* MT7621/MT7530 GSW or external GigaPHY */
#if defined (CONFIG_P5_MAC_TO_PHY_MODE) || defined (CONFIG_MT7530_GSW) || \
    defined (CONFIG_P4_MAC_TO_PHY_MODE) || defined (CONFIG_GE2_RGMII_AN)

#if !defined (CONFIG_MT7530_GSW)
#if defined (CONFIG_P4_MAC_TO_PHY_MODE) && defined (CONFIG_P5_MAC_TO_PHY_MODE)
	if (port_id == 4 || port_id == 5)
#elif defined (CONFIG_P4_MAC_TO_PHY_MODE)
	if (port_id == 4)
#elif defined (CONFIG_P5_MAC_TO_PHY_MODE)
	if (port_id == 5)
#endif
#endif
	{
		u32 esw_phy_gcr = 0;
		
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
	u32 i;
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
		
		for (i = 0; i <= ESW_EPHY_ID_MAX; i++) {
			if ((1u << i) & ESW_MASK_EXCLUDE)
				continue;
			esw_storm_control(i, 1, 0, 0, control_rate_mbps);
		}
	}
}

static void change_jumbo_frames_accept(u32 jumbo_frames_enabled)
{
	if (jumbo_frames_enabled)
		jumbo_frames_enabled = 1;

	if (g_jumbo_frames_enabled != jumbo_frames_enabled) {
		g_jumbo_frames_enabled = jumbo_frames_enabled;
		printk("%s - jumbo frames accept: %d bytes\n", MTK_ESW_DEVNAME, (jumbo_frames_enabled) ? 9000 : 1536);
		
		esw_jumbo_control(jumbo_frames_enabled);
	}
}

static void change_eee_lpi_mode(u32 eee_lpi_enabled)
{
	if (eee_lpi_enabled)
		eee_lpi_enabled = 1;

	if (g_eee_lpi_enabled != eee_lpi_enabled) {
		g_eee_lpi_enabled = eee_lpi_enabled;
		printk("%s - 802.3az EEE: %s\n", MTK_ESW_DEVNAME, (eee_lpi_enabled) ? "on" : "off");
		
		esw_eee_control(eee_lpi_enabled);
	}
}

static void change_igmp_static_ports(u32 ports_mask)
{
	ports_mask = get_ports_mask_from_uapi(ports_mask, 0);

#if defined (CONFIG_RAETH_ESW_IGMP_SNOOP_HW)
	if (g_igmp_static_ports != ports_mask) {
		g_igmp_static_ports = ports_mask;
		
		if (g_igmp_snooping_enabled) {
			esw_igmp_ports_config(g_wan_bridge_mode);
			esw_igmp_flood_to_cpu( (!ports_mask) ? 1 : 0 );
			esw_igmp_mld_snooping(1, 1);
		}
	}
#elif defined (CONFIG_RAETH_ESW_IGMP_SNOOP_SW)
	igmp_sn_set_static_ports(ports_mask);
#endif
}

static void change_igmp_snooping_control(u32 igmp_snooping_enabled)
{
#if defined (CONFIG_RAETH_ESW_IGMP_SNOOP_HW)
	if (igmp_snooping_enabled)
		igmp_snooping_enabled = 1;

	if (g_igmp_snooping_enabled != igmp_snooping_enabled) {
		g_igmp_snooping_enabled = igmp_snooping_enabled;
		printk("%s - hardware IGMP/MLD snooping: %s\n", MTK_ESW_DEVNAME, (igmp_snooping_enabled) ? "on" : "off");
		
		esw_igmp_ports_config(g_wan_bridge_mode);
		esw_igmp_flood_to_cpu( (igmp_snooping_enabled && !g_igmp_static_ports) ? 1 : 0 );
		esw_igmp_mld_snooping(igmp_snooping_enabled, igmp_snooping_enabled);
	}
#elif defined (CONFIG_RAETH_ESW_IGMP_SNOOP_SW)
	igmp_sn_set_enable(igmp_snooping_enabled);
#endif
}

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

static void esw_link_status_changed(u32 port_id, int port_link)
{
	const char *port_state;

	if (atomic_read(&g_switch_allow_irq) == 1) {
#if defined (CONFIG_MT7530_GSW)
		mt7530_gsw_eee_on_link(port_id, port_link, g_eee_lpi_enabled);
#else
		mt7620_esw_eee_on_link(port_id, port_link, g_eee_lpi_enabled);
#endif
	}

	if (port_id <= ESW_EPHY_ID_MAX)
		atomic_set(&g_port_link_changed, 1);

#if !ESW_PRINT_LINK_ALL
	{
		u32 wan_ports_mask = get_ports_mask_wan(0, 1);
		
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
#if !defined (CONFIG_MT7530_GSW)
	/* configure ports security and forwards mask */
	esw_mask_bridge_isolate();
#else
	/* configure bridge isolation mode via forwards mask */
	esw_mask_bridge_isolate(g_wan_bridge_mode, g_wan_bwan_isolation);
#endif

	/* enable MAC for all PHY ports */
	esw_mac_to_phy_enable();

	/* configure bridge isolation mode via VLAN */
	esw_vlan_bridge_isolate(g_wan_bridge_mode, g_wan_bwan_isolation, 1, 1, 1);

#if defined (CONFIG_RAETH_ESW_IGMP_SNOOP_HW)
	/* configure igmp/mld snooping */
	esw_igmp_flood_to_cpu( (g_igmp_snooping_enabled && !g_igmp_static_ports) ? 1 : 0 );
	esw_igmp_mld_snooping(g_igmp_snooping_enabled, g_igmp_snooping_enabled);
#endif

	/* configure leds */
	esw_led_mode(g_led_phy_mode);

	/* disable 802.3az EEE by default */
	if (!g_eee_lpi_enabled) {
#if defined (CONFIG_MT7530_GSW)
		mt7530_gsw_eee_enable(0);
#else
		mt7620_esw_eee_enable(0);
#endif
	}

	atomic_set(&g_switch_inited, 1);
	atomic_set(&g_switch_allow_irq, 1);

	return 0;
}

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#if !defined (CONFIG_RAETH_BOTH_GMAC)
int esw_get_traffic_port_wan(struct rtnl_link_stats64 *stats)
{
	ULARGE_INTEGER rx_goct, tx_goct;

	rx_goct.u.LowPart = esw_get_port_mib_rgoc(WAN_PORT_X, &rx_goct.u.HighPart);
	tx_goct.u.LowPart = esw_get_port_mib_tgoc(WAN_PORT_X, &tx_goct.u.HighPart);

	stats->rx_packets = esw_get_port_mib_rgpc(WAN_PORT_X);
	stats->tx_packets = esw_get_port_mib_tgpc(WAN_PORT_X);

	stats->rx_bytes = rx_goct.QuadPart;
	stats->tx_bytes = tx_goct.QuadPart;

	return 0;
}
EXPORT_SYMBOL(esw_get_traffic_port_wan);
#endif
#endif

////////////////////////////////////////////////////////////////////////////////////
#include "ioctl.c"
////////////////////////////////////////////////////////////////////////////////////

