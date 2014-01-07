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
#include <linux/interrupt.h>

#include <linux/ralink_gpio.h>

#include "ra_ethreg.h"
#include "mii_mgr.h"

#include "ra_esw_ioctl.h"
#include "ra_esw_def.h"

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
#include "../../../net/nat/hw_nat/ra_nat.h"
#include "../../../net/nat/hw_nat/foe_fdb.h"
extern int (*ra_sw_nat_hook_rx)(struct sk_buff *skb);
#endif

////////////////////////////////////////////////////////////////////////////////////

static DEFINE_MUTEX(esw_access_mutex);

static u32 g_wan_bridge_mode                     = SWAPI_WAN_BRIDGE_DISABLE;
static u32 g_wan_bwan_isolation                  = SWAPI_WAN_BWAN_ISOLATION_NONE;

static u32 g_led_phy_mode                        = SWAPI_LED_LINK_ACT;

static u32 g_jumbo_frames_enabled                = ESW_DEFAULT_JUMBO_FRAMES;
static u32 g_igmp_snooping_enabled               = ESW_DEFAULT_IGMP_SNOOPING;

static u32 g_storm_rate_limit                    = ESW_DEFAULT_STORM_RATE;

static u32 g_port_link_mode[ESW_PHY_ID_MAX+1]    = {0, 0, 0, 0, 0};
static u32 g_port_phy_power[ESW_PHY_ID_MAX+1]    = {1, 1, 1, 1, 1};

static u32 g_vlan_rule[SWAPI_VLAN_RULE_NUM]      = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static u32 g_vlan_rule_user[SWAPI_VLAN_RULE_NUM] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

////////////////////////////////////////////////////////////////////////////////////

static atomic_t g_switch_inited                  = ATOMIC_INIT(0);
static atomic_t g_port_link_changed              = ATOMIC_INIT(0);

static bwan_member_t g_bwan_member[SWAPI_WAN_BRIDGE_NUM][ESW_PHY_ID_MAX+1];

////////////////////////////////////////////////////////////////////////////////////

static u32 get_phy_ports_mask_lan(u32 include_cpu)
{
	u32 i, wan_bridge_mode, portmask_lan;

	wan_bridge_mode = g_wan_bridge_mode;

	portmask_lan = ((1u << LAN_PORT_4) | (1u << LAN_PORT_3) | (1u << LAN_PORT_2) | (1u << LAN_PORT_1));
	if (include_cpu)
		portmask_lan |= (1u << LAN_PORT_CPU);

	for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
		if (g_bwan_member[wan_bridge_mode][i].bwan)
			portmask_lan &= ~(1u << i);
	}

	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		portmask_lan |= (1u << WAN_PORT_X);

	return portmask_lan;
}

static u32 get_phy_ports_mask_wan(u32 include_cpu)
{
	u32 i, wan_bridge_mode, portmask_wan;

	wan_bridge_mode = g_wan_bridge_mode;
	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
		return 0;

	portmask_wan = (1u << WAN_PORT_X);
	if (include_cpu)
		portmask_wan |= (1u << WAN_PORT_CPU);

	for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
		if (g_bwan_member[wan_bridge_mode][i].bwan)
			portmask_wan |= (1u << i);
	}

	return portmask_wan;
}

static u32 get_phy_ports_mask_from_user(u32 user_port_mask)
{
	u32 phy_ports_mask = 0;

	if (user_port_mask & SWAPI_PORTMASK_LAN1)
		phy_ports_mask |= (1u << LAN_PORT_1);
	if (user_port_mask & SWAPI_PORTMASK_LAN2)
		phy_ports_mask |= (1u << LAN_PORT_2);
	if (user_port_mask & SWAPI_PORTMASK_LAN3)
		phy_ports_mask |= (1u << LAN_PORT_3);
	if (user_port_mask & SWAPI_PORTMASK_LAN4)
		phy_ports_mask |= (1u << LAN_PORT_4);
	if (user_port_mask & SWAPI_PORTMASK_WAN)
		phy_ports_mask |= (1u << WAN_PORT_X);
	if (user_port_mask & SWAPI_PORTMASK_CPU_LAN)
		phy_ports_mask |= (1u << LAN_PORT_CPU);
	if (user_port_mask & SWAPI_PORTMASK_CPU_WAN)
		phy_ports_mask |= (1u << WAN_PORT_CPU);

	return phy_ports_mask;
}

static void esw_port_matrix_set(u32 port_id, u32 fwd_mask, u32 pvlan_ingress_mode)
{
	u32 reg_pcr;

	reg_pcr = _ESW_REG(REG_ESW_PORT_PCR_P0 + 0x100*port_id);
	reg_pcr &= ~(0xFF << 16);
	reg_pcr &= ~(0x03);
	reg_pcr |= ((fwd_mask & 0xFF) << 16);
	reg_pcr |= (pvlan_ingress_mode & 0x03);
	_ESW_REG(REG_ESW_PORT_PCR_P0 + 0x100*port_id) = reg_pcr;
}

static void esw_port_ingress_mode_set(u32 port_id, u32 pvlan_ingress_mode)
{
	u32 reg_pcr;

	reg_pcr = _ESW_REG(REG_ESW_PORT_PCR_P0 + 0x100*port_id);
	reg_pcr &= ~(0x03);
	reg_pcr |= (pvlan_ingress_mode & 0x03);
	_ESW_REG(REG_ESW_PORT_PCR_P0 + 0x100*port_id) = reg_pcr;
}

static void esw_port_egress_mode_set(u32 port_id, u32 pvlan_egress_tag)
{
	u32 reg_pcr;

	reg_pcr = _ESW_REG(REG_ESW_PORT_PCR_P0 + 0x100*port_id);
	reg_pcr &= ~(0x03 << 28);
	reg_pcr |= ((pvlan_egress_tag & 0x03) << 28);
	_ESW_REG(REG_ESW_PORT_PCR_P0 + 0x100*port_id) = reg_pcr;
}

static void esw_port_attrib_set(u32 port_id, u32 port_attribute)
{
	u32 reg_pvc;

	reg_pvc = _ESW_REG(REG_ESW_PORT_PVC_P0 + 0x100*port_id);
	reg_pvc &= 0x0000FF3F;
	reg_pvc |= 0x81000000; // STAG VPID 8100
	reg_pvc |= ((port_attribute & 0x03) << 6);
	_ESW_REG(REG_ESW_PORT_PVC_P0 + 0x100*port_id) = reg_pvc;
}

static void esw_port_accept_set(u32 port_id, u32 accept_frames)
{
	u32 reg_pvc;

	reg_pvc = _ESW_REG(REG_ESW_PORT_PVC_P0 + 0x100*port_id);
	reg_pvc &= 0xFFFFFFFC;
	reg_pvc |= (accept_frames & 0x03);
	_ESW_REG(REG_ESW_PORT_PVC_P0 + 0x100*port_id) = reg_pvc;
}

static void esw_vlan_pvid_set(u32 port_id, u32 pvid, u32 prio)
{
	u32 reg_ppbv = (1u << 16) | ((prio & 0x7) << 13) | (pvid & 0xfff);

	_ESW_REG(REG_ESW_PORT_PPBV1_P0 + 0x100*port_id) = reg_ppbv;
}

static void esw_igmp_ports_config(u32 wan_bridge_mode)
{
	u32 i, reg_isc, mask_drp;

	if (wan_bridge_mode != SWAPI_WAN_BRIDGE_DISABLE_WAN) {
		mask_drp = get_phy_ports_mask_wan(1);
		for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
			if ((mask_drp >> i) & 0x1)
				_ESW_REG(REG_ESW_PORT_PIC_P0 + 0x100*i) = 0x8000;
		}
	}
	else
		mask_drp = (1u << LAN_PORT_CPU);

	reg_isc = _ESW_REG(REG_ESW_ISC);
	reg_isc &= ~0xFF0000FF;
	reg_isc |= mask_drp;
	_ESW_REG(REG_ESW_ISC) = reg_isc;
}

static void esw_igmp_mld_snooping(u32 enable_igmp, u32 enable_mld)
{
	u32 i, mask_lan, reg_pic_phy, reg_pic_cpu;

	reg_pic_phy = 0x00008000;		// Robustness = 2
	reg_pic_cpu = 0x00008000;		// Robustness = 2
	if (enable_mld) {
		reg_pic_cpu |= (1u << 9);	// IPM_33
		
		reg_pic_phy |= (1u << 13);	// MLD_HW_LEAVE
		reg_pic_phy |= (1u << 7);	// MLD2_JOIN_EN
		reg_pic_phy |= (1u << 5);	// MLD_JOIN_EN
		
		if (g_wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
			reg_pic_phy |= (1u << 9);	// IPM_33
	}

	if (enable_igmp) {
		reg_pic_cpu |= (1u << 8);	// IPM_01
		
		reg_pic_phy |= (1u << 12);	// IGMP_HW_LEAVE
		reg_pic_phy |= (1u << 6);	// IGMP3_JOIN_EN
		reg_pic_phy |= (1u << 4);	// IGMP_JOIN_EN
		
		if (g_wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
			reg_pic_phy |= (1u << 8);	// IPM_01
	}

	mask_lan = get_phy_ports_mask_lan(0);
	for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
		if ((mask_lan >> i) & 0x1)
			_ESW_REG(REG_ESW_PORT_PIC_P0 + 0x100*i) = reg_pic_phy;
	}

	_ESW_REG(REG_ESW_PORT_PIC_P0 + 0x100*LAN_PORT_CPU) = reg_pic_cpu;
}

static int esw_mac_table_clear(void)
{
	u32 i, reg_atc;

	_ESW_REG(REG_ESW_WT_MAC_ATC) = 0x8002;
	udelay(1000);

	for (i = 0; i < 100; i++) {
		reg_atc = _ESW_REG(REG_ESW_WT_MAC_ATC);
		if (!(reg_atc & 0x8000))
			return 0;
		udelay(100);
	}

	printk("%s: ATC timeout!\n", MTK_ESW_DEVNAME);
	return -1;
}

static int esw_write_vtcr(u32 vtcr_cmd, u32 vtcr_val)
{
	u32 i, reg_vtcr;

	reg_vtcr = (vtcr_cmd << 12) | vtcr_val | 0x80000000;
	_ESW_REG(REG_ESW_VLAN_VTCR) = reg_vtcr;

	for (i = 0; i < 100; i++) {
		reg_vtcr = _ESW_REG(REG_ESW_VLAN_VTCR);
		if (!(reg_vtcr & 0x80000000))
			return 0;
		udelay(100);
	}

	printk("%s: VTCR timeout!\n", MTK_ESW_DEVNAME);
	return -1;
}

static int esw_find_free_vlan_slot(u32 start_idx)
{
	u32 i, reg_vawd1;

	for (i = start_idx; i < 16; i++) {
		// read VAWD1
		if (esw_write_vtcr(0, i) != 0)
			continue;
		reg_vawd1 = _ESW_REG(REG_ESW_VLAN_VAWD1);
		if (!(reg_vawd1 & 1))
			return (int)i;
	}

	return -1;
}

static void esw_vlan_set_idx(u32 idx, u32 cvid, u32 svid, u32 port_member, u32 fid)
{
	u32 reg_val;

	idx &= 0xf;
	cvid &= 0xfff;
	svid &= 0xfff;
	port_member &= 0xff;
	fid &= 0x7;

	// set vlan identifier
	reg_val = _ESW_REG(REG_ESW_VLAN_ID_BASE + 4*(idx/2));
	if ((idx % 2) == 0) {
		reg_val &= 0xfff000;
		reg_val |= cvid;
	} else {
		reg_val &= 0x000fff;
		reg_val |= (cvid << 12);
	}
	_ESW_REG(REG_ESW_VLAN_ID_BASE + 4*(idx/2)) = reg_val;

	// set vlan member
	reg_val = 1;				// VALID
#if !ESW_USE_IVL_MODE
	if (fid > 0)
		reg_val |= (fid << 1);		// FID
	else
#endif
	reg_val |= (1u << 30);			// IVL=1
	reg_val |= (svid << 4);			// S_TAG
	reg_val |= (port_member << 16);		// PORT_MEM
	if (svid)
		reg_val |= (1u << 27);		// COPY_PRI

	_ESW_REG(REG_ESW_VLAN_VAWD1) = reg_val;

	esw_write_vtcr(1, idx);
}

static void esw_vlan_reset_table(void)
{
	u32 i;

	// Reset VLAN mappings
	for (i = 0; i < 8; i++)
		_ESW_REG(REG_ESW_VLAN_ID_BASE + 4*i) = (((i<<1)+2) << 12) | ((i<<1)+1);

	// Reset VLAN table from idx 1 to 15
	for(i = 1; i < 16; i++)
		esw_write_vtcr(2, i);
}

static u32 find_vlan_slot(vlan_entry_t *vlan_entry, u32 start_idx, u32 cvid)
{
	u32 i;

	for (i = start_idx; i <= ESW_VLAN_ID_MAX; i++) {
		if (!vlan_entry[i].valid || vlan_entry[i].cvid == cvid)
			return i;
	}

	return ESW_VLAN_ID_MAX + 1; // not found
}

static void esw_vlan_apply_rules(u32 wan_bridge_mode, u32 wan_bwan_isolation)
{
	pvlan_member_t pvlan_member[ESW_PHY_ID_MAX+1];
	vlan_entry_t vlan_entry[ESW_VLAN_ID_MAX+1];
	u32 pvid[SWAPI_VLAN_RULE_NUM];
	u32 prio[SWAPI_VLAN_RULE_NUM];
	u32 tagg[SWAPI_VLAN_RULE_NUM];
	u32 i, next_fid, next_vid, vlan_idx;
	u32 egress_swap_port_cpu, vlan_filter_on;

	next_vid = 3;
	next_fid = 3;
	vlan_filter_on = 0;
	egress_swap_port_cpu = 0;

	for (i = 0; i < SWAPI_VLAN_RULE_NUM; i++) {
		pvid[i] =  (g_vlan_rule_user[i] & 0xFFF);
		prio[i] = ((g_vlan_rule_user[i] >> 16) & 0x7);
		tagg[i] = ((g_vlan_rule_user[i] >> 24) & 0x1);
		if (pvid[i] >= MIN_EXT_VLAN_VID)
			vlan_filter_on = 1;
	}

	if (pvid[SWAPI_VLAN_RULE_WAN_INET] < MIN_EXT_VLAN_VID) {
		pvid[SWAPI_VLAN_RULE_WAN_INET] = 2; // VID 2
		prio[SWAPI_VLAN_RULE_WAN_INET] = 0;
		tagg[SWAPI_VLAN_RULE_WAN_INET] = 0;
	}

	for (i = 0; i <= ESW_VLAN_ID_MAX; i++) {
		vlan_entry[i].valid = 0;
		vlan_entry[i].fid = 0;
		vlan_entry[i].cvid = 0;
		vlan_entry[i].svid = 0;
		vlan_entry[i].port_member = 0;
	}

	/* fill WAN port (use PVID 2 for handle untagged traffic -> VID2) */
	pvlan_member[WAN_PORT_X].pvid = 2;
	pvlan_member[WAN_PORT_X].prio = 0;
	pvlan_member[WAN_PORT_X].tagg = 0;
	pvlan_member[WAN_PORT_X].swap = 0;

	/* VID #1 */
	vlan_entry[0].valid = 1;
	vlan_entry[0].fid = 1;
	vlan_entry[0].cvid = 1;
	vlan_entry[0].port_member |= (1u << LAN_PORT_CPU);

	/* VID #2 */
	vlan_entry[1].valid = 1;
	vlan_entry[1].fid = 2;
	vlan_entry[1].cvid = 2;
	vlan_entry[1].port_member |= ((1u << WAN_PORT_CPU) | (1u << WAN_PORT_X));

	if (!vlan_filter_on && wan_bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_BETWEEN) {
		pvlan_member[WAN_PORT_X].pvid = next_vid;
		
		/* VID #1 */
		vlan_entry[0].svid = 1;		/* used under etag_ctrl=SWAP (CVID<->SVID) */
		
		/* VID #2 */
		vlan_entry[1].svid = 2;		/* used under etag_ctrl=SWAP (CVID<->SVID) */
		
		/* VID #3 */
		vlan_entry[2].valid = 1;
		vlan_entry[2].fid = next_fid;
		vlan_entry[2].cvid = next_vid;
		vlan_entry[2].svid = 2;		/* used under etag_ctrl=SWAP (CVID<->SVID) */
		vlan_entry[2].port_member |= ((1u << WAN_PORT_CPU) | (1u << WAN_PORT_X));
		next_fid++;
		next_vid++;
		
		egress_swap_port_cpu = 1;
	}

	/* check IPTV tagged */
	if (pvid[SWAPI_VLAN_RULE_WAN_IPTV] >= MIN_EXT_VLAN_VID) {
		vlan_idx = find_vlan_slot(&vlan_entry[0], next_vid-1, pvid[SWAPI_VLAN_RULE_WAN_IPTV]);
		if (vlan_idx <= ESW_VLAN_ID_MAX) {
			if (!vlan_entry[vlan_idx].valid) {
				vlan_entry[vlan_idx].valid = 1;
				vlan_entry[vlan_idx].fid = next_fid++;
				vlan_entry[vlan_idx].cvid = pvid[SWAPI_VLAN_RULE_WAN_IPTV];
				vlan_entry[vlan_idx].svid = pvid[SWAPI_VLAN_RULE_WAN_IPTV];
			}
			vlan_entry[vlan_idx].port_member |= ((1u << WAN_PORT_CPU) | (1u << WAN_PORT_X));
			
			pvlan_member[WAN_PORT_X].tagg = 1;
			pvlan_member[WAN_PORT_X].swap = 1;
		}
	}

	/* check INET tagged */
	if (pvid[SWAPI_VLAN_RULE_WAN_INET] >= MIN_EXT_VLAN_VID) {
		vlan_idx = find_vlan_slot(&vlan_entry[0], next_vid-1, pvid[SWAPI_VLAN_RULE_WAN_INET]);
		if (vlan_idx <= ESW_VLAN_ID_MAX) {
			if (!vlan_entry[vlan_idx].valid) {
				vlan_entry[vlan_idx].valid = 1;
				vlan_entry[vlan_idx].fid = next_fid++;
				vlan_entry[vlan_idx].cvid = pvid[SWAPI_VLAN_RULE_WAN_INET];
				vlan_entry[vlan_idx].svid = pvid[SWAPI_VLAN_RULE_WAN_INET];
			}
			vlan_entry[vlan_idx].port_member |= ((1u << WAN_PORT_CPU) | (1u << WAN_PORT_X));
			
			pvlan_member[WAN_PORT_X].tagg = 1;
			pvlan_member[WAN_PORT_X].swap = 1;
		}
	}

	/* fill LAN ports */
	for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
		int rule_id;
		
		if (i == WAN_PORT_X)
			continue;
		
		pvlan_member[i].prio = 0;
		pvlan_member[i].tagg = 0;
		pvlan_member[i].swap = 0;
		
		if (!g_bwan_member[wan_bridge_mode][i].bwan) {
			pvlan_member[i].pvid = 1;
			
			/* VID #1 */
			vlan_entry[0].port_member |= (1u << i);
			
			continue;
		}
		
		rule_id = g_bwan_member[wan_bridge_mode][i].rule;
		if (pvid[rule_id] < MIN_EXT_VLAN_VID) {
			pvlan_member[i].pvid = 2;
			
			/* VID #2 */
			vlan_entry[1].port_member |= (1u << i);
			
			if (!vlan_filter_on && wan_bwan_isolation != SWAPI_WAN_BWAN_ISOLATION_NONE) {
				pvlan_member[i].pvid = next_vid;
				
				vlan_entry[next_vid-1].valid = 1;
				vlan_entry[next_vid-1].fid = next_fid;
				vlan_entry[next_vid-1].cvid = next_vid;
				
				if (wan_bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_BETWEEN) {
					vlan_entry[next_vid-1].svid = 2;	/* used under etag_ctrl=SWAP (CVID<->SVID) */
					vlan_entry[next_vid-1].port_member |= ((1u << i) | (1u << WAN_PORT_CPU));
					next_fid++;
					next_vid++;
				} else {
					vlan_entry[next_vid-1].port_member |= ((1u << i) | (1u << WAN_PORT_X));
				}
			}
		} else {
			vlan_idx = find_vlan_slot(&vlan_entry[0], next_vid-1, pvid[rule_id]);
			if (vlan_idx <= ESW_VLAN_ID_MAX) {
				if (!vlan_entry[vlan_idx].valid) {
					vlan_entry[vlan_idx].valid = 1;
					vlan_entry[vlan_idx].fid = next_fid++;
					vlan_entry[vlan_idx].cvid = pvid[rule_id];
					vlan_entry[vlan_idx].svid = pvid[rule_id];
				}
				vlan_entry[vlan_idx].port_member |= ((1u << i) | (1u << WAN_PORT_X));
				
				pvlan_member[i].pvid = pvid[rule_id];
				pvlan_member[i].prio = prio[rule_id];
				pvlan_member[i].tagg = tagg[rule_id];
				
				pvlan_member[WAN_PORT_X].tagg = 1;
				pvlan_member[WAN_PORT_X].swap = 1;
			} else {
				pvlan_member[i].pvid = 2;
				
				/* VID #2 */
				vlan_entry[1].port_member |= (1u << i);
			}
		}
	}

#if defined (ESW_PORT_PPE)
	/* add PPE port to members with CPU port */
	for (i = 0; i <= ESW_VLAN_ID_MAX; i++) {
		if (!vlan_entry[i].valid)
			continue;
		if (vlan_entry[i].port_member & (1u << ESW_PORT_CPU))
			vlan_entry[i].port_member |= (1u << ESW_PORT_PPE);
	}
#endif

	/* configure PHY ports */
	for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
		esw_vlan_pvid_set(i, pvlan_member[i].pvid, pvlan_member[i].prio);
		if (!pvlan_member[i].tagg) {
			esw_port_attrib_set(i, PORT_ATTRIBUTE_TRANSPARENT);
			esw_port_accept_set(i, PORT_ACCEPT_FRAMES_UNTAGGED);
			esw_port_egress_mode_set(i, PVLAN_EGRESS_UNTAG);
		} else {
			esw_port_attrib_set(i, PORT_ATTRIBUTE_USER);
			esw_port_accept_set(i, PORT_ACCEPT_FRAMES_ALL);
			esw_port_egress_mode_set(i, (pvlan_member[i].swap) ? PVLAN_EGRESS_SWAP : PVLAN_EGRESS_TAG);
		}
	}

	/* configure CPU port */
	esw_port_attrib_set(LAN_PORT_CPU, PORT_ATTRIBUTE_USER);
	esw_port_accept_set(LAN_PORT_CPU, PORT_ACCEPT_FRAMES_ALL);
	esw_port_egress_mode_set(LAN_PORT_CPU, (egress_swap_port_cpu) ? PVLAN_EGRESS_SWAP : PVLAN_EGRESS_TAG);
#if defined (ESW_PORT_PPE)
	esw_port_egress_mode_set(ESW_PORT_PPE, (egress_swap_port_cpu) ? PVLAN_EGRESS_SWAP : PVLAN_EGRESS_TAG);
#endif
#if defined (CONFIG_RALINK_MT7620)
#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	if (ra_sw_nat_hook_rx != NULL)
		esw_port_ingress_mode_set(LAN_PORT_CPU, PVLAN_INGRESS_MODE_FALLBACK);
	else
#endif
#endif
		esw_port_ingress_mode_set(LAN_PORT_CPU, PVLAN_INGRESS_MODE_SECURITY);

	/* fill VLAN table */
	esw_vlan_reset_table();
	for (i = 0; i <= ESW_VLAN_ID_MAX; i++) {
		if (!vlan_entry[i].valid)
			continue;
		esw_vlan_set_idx(i, vlan_entry[i].cvid, vlan_entry[i].svid, vlan_entry[i].port_member, vlan_entry[i].fid);
	}

	/* save VLAN rules */
	for (i = 0; i < SWAPI_VLAN_RULE_NUM; i++)
		g_vlan_rule[i] = g_vlan_rule_user[i];
}

static void esw_vlan_init_vid1(void)
{
	u32 i, port_member;

	port_member = 0xFF;
#if !defined (CONFIG_RAETH_HAS_PORT5)
	port_member &= ~0x20;
#endif

	/* configure PHY ports */
	for (i = 0; i <= ESW_PORT_ID_MAX; i++) {
		esw_vlan_pvid_set(i, 1, 0);
		esw_port_attrib_set(i, PORT_ATTRIBUTE_TRANSPARENT);
		esw_port_accept_set(i, PORT_ACCEPT_FRAMES_UNTAGGED);
		esw_port_egress_mode_set(i, PVLAN_EGRESS_UNTAG);
	}

	/* configure CPU port */
	esw_port_attrib_set(LAN_PORT_CPU, PORT_ATTRIBUTE_USER);
	esw_port_accept_set(LAN_PORT_CPU, PORT_ACCEPT_FRAMES_ALL);
	esw_port_egress_mode_set(LAN_PORT_CPU, PVLAN_EGRESS_TAG);
	esw_port_ingress_mode_set(LAN_PORT_CPU, PVLAN_INGRESS_MODE_SECURITY);

	/* reset VLAN table */
	esw_vlan_reset_table();

	/* set all ports to VLAN 1 member (no SVID) */
	esw_vlan_set_idx(0, 1, 0, port_member, 1);
}

static void esw_show_bridge_partitions(u32 wan_bridge_mode)
{
	char *wan1, *wan2;

	wan1 = "W|";
	wan2 = "";

	switch (wan_bridge_mode)
	{
	case SWAPI_WAN_BRIDGE_LAN1:
		printk("%s - hw bridge: %sWLLL%s\n", MTK_ESW_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN2:
		printk("%s - hw bridge: %sLWLL%s\n", MTK_ESW_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN3:
		printk("%s - hw bridge: %sLLWL%s\n", MTK_ESW_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN4:
		printk("%s - hw bridge: %sLLLW%s\n", MTK_ESW_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN3_LAN4:
		printk("%s - hw bridge: %sLLWW%s\n", MTK_ESW_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2:
		printk("%s - hw bridge: %sWWLL%s\n", MTK_ESW_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_LAN1_LAN2_LAN3:
		printk("%s - hw bridge: %sWWWL%s\n", MTK_ESW_DEVNAME, wan1, wan2);
		break;
	case SWAPI_WAN_BRIDGE_DISABLE_WAN:
		printk("%s - hw bridge: LLLLL\n", MTK_ESW_DEVNAME);
		break;
	default:
		printk("%s - hw bridge: %sLLLL%s\n", MTK_ESW_DEVNAME, wan1, wan2);
		break;
	}
}

static void esw_vlan_bridge_isolate(u32 wan_bridge_mode, u32 wan_bwan_isolation, int bridge_changed, int br_iso_changed, int vlan_rule_changed)
{
	if (wan_bridge_mode == SWAPI_WAN_BRIDGE_DISABLE_WAN)
	{
		if (!bridge_changed)
			return;
		
		esw_vlan_init_vid1();
	}
	else
	{
		if (!bridge_changed && !br_iso_changed && !vlan_rule_changed)
			return;
		
		esw_vlan_apply_rules(wan_bridge_mode, wan_bwan_isolation);
	}

	if (bridge_changed) {
		esw_igmp_ports_config(wan_bridge_mode);
		esw_show_bridge_partitions(wan_bridge_mode);
	}

	esw_mac_table_clear();
}

static void esw_soft_reset(void)
{
	u32 reg_agc;

	// Reset ARL
	reg_agc = _ESW_REG(REG_ESW_AGC);
	_ESW_REG(REG_ESW_AGC) = (reg_agc & ~0x1);
	mdelay(5);
	reg_agc = _ESW_REG(REG_ESW_AGC);
	_ESW_REG(REG_ESW_AGC) = (reg_agc |  0x1);
}

static void esw_init_ports_cpu_ppe(void)
{
	u32 reg_pcr6;
	u32 reg_pvc6;
	u32 reg_pmcr6;
#if defined (CONFIG_RALINK_MT7620)
	u32 i;
	u32 reg_pfc;
	u32 reg_tpf;
	u32 reg_pcr7;
	u32 reg_pvc7;
	u32 reg_psc7;
	u32 reg_pmcr7;
#endif

	/* set security mode, egress always tagged */
	reg_pcr6  =  0x20ff0003;
#if !defined (CONFIG_RAETH_HAS_PORT5)
	reg_pcr6 &= ~0x00200000;
#endif
	/* user port, admit all frames */
	reg_pvc6 = 0x81000000;

	/* forced Port6 link up, 1Gbps FD */
	reg_pmcr6 = 0x0005e33b;

#if defined (CONFIG_RALINK_MT7620)
	/* PPE_PORT=7, PPE_EN=0 */
	reg_pfc = 0x00000007;

	/* disable forward to PPE */
	reg_tpf = 0x00000000;

	/* set security mode, egress always tagged */
	reg_pcr7  =  0x20ff0003;
#if !defined (CONFIG_RAETH_HAS_PORT5)
	reg_pcr7 &= ~0x00200000;
#endif
	/* user port, admit all frames */
	reg_pvc7 = 0x81000000;

	/* enable Port7 SA learning */
	reg_psc7 = 0x000fff00;

	/* forced Port7 link down */
	reg_pmcr7 = 0x0005e330;

#if defined (CONFIG_RA_HW_NAT) || defined (CONFIG_RA_HW_NAT_MODULE)
	if (ra_sw_nat_hook_rx != NULL) {
		/* PPE_PORT=7, PPE_EN=1 */
		reg_pfc = 0x0000000f;
		
		/* enable forward to PPE (exclude broadcast) */
		reg_tpf  = 0x00000031;
#if defined (CONFIG_RA_HW_NAT_IPV6)
		reg_tpf |= 0x00003100;
#endif
#if defined (CONFIG_RA_HW_NAT_MCAST)
		reg_tpf |= 0x00000006;
#if defined (CONFIG_RA_HW_NAT_IPV6)
		reg_tpf |= 0x00000600;
#endif
#endif
		/* set fallback mode and P6|P7 port matrix group */
		reg_pcr6 &= ~0x003F0003;
		reg_pcr7 &= ~0x003F0003;
		reg_pcr6 |=  0x00000001;
		reg_pcr7 |=  0x00000001;
		
		/* disable Port7 SA learning */
		reg_psc7 |= 0x00000010;
		
		/* forced Port7 link up, 1Gbps FD */
		reg_pmcr7 = 0x0005e33b;
	}
#endif
	/* config PPE port */
	_ESW_REG(REG_ESW_PFC) = reg_pfc;
	_ESW_REG(REG_ESW_PORT_PCR_P0 + 0x100*ESW_PORT_PPE) = reg_pcr7;
	_ESW_REG(REG_ESW_PORT_PVC_P0 + 0x100*ESW_PORT_PPE) = reg_pvc7;
	_ESW_REG(REG_ESW_PORT_PSC_P0 + 0x100*ESW_PORT_PPE) = reg_psc7;
	_ESW_REG(REG_ESW_MAC_PMCR_P0 + 0x100*ESW_PORT_PPE) = reg_pmcr7;

	/* config all PHY ports TO_PPE Forwarding */
	for (i = 0; i <= ESW_PHY_ID_MAX; i++)
		_ESW_REG(REG_ESW_PORT_TPF_P0 + 0x100*i) = reg_tpf;
#endif

	/* config CPU port */
	_ESW_REG(REG_ESW_PORT_PCR_P0 + 0x100*ESW_PORT_CPU) = reg_pcr6;
	_ESW_REG(REG_ESW_PORT_PVC_P0 + 0x100*ESW_PORT_CPU) = reg_pvc6;
	_ESW_REG(REG_ESW_MAC_PMCR_P0 + 0x100*ESW_PORT_CPU) = reg_pmcr6;
}

static void esw_init_ports_phy(void)
{
	u32 i, fwd_mask = 0xFF;

#if !defined (CONFIG_RAETH_HAS_PORT5)
	fwd_mask &= ~0x20;
#endif
	for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
		/* set port ingress to security mode */
		esw_port_matrix_set(i, fwd_mask, PVLAN_INGRESS_MODE_SECURITY);
	}
}

static void esw_port_phy_power(u32 port_id, u32 power_on)
{
	u32 esw_phy_mcr = (power_on) ? 0x3300 : 0x3900;

	if (port_id > ESW_PHY_ID_MAX)
		return;

	if (mii_mgr_write(port_id, 0, esw_phy_mcr))
		g_port_phy_power[port_id] = (power_on) ? 1 : 0;
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
		rate_unit_100 = (rate_mbps < 80) ? rate_mbps : 80;
		rate_unit_10 = (rate_mbps < 8) ? rate_mbps : 8;
		
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

	_ESW_REG(REG_ESW_PORT_BSR_P0 + 0x100*port_id) = reg_bsr;
}

static void esw_jumbo_control(u32 jumbo_frames_enabled)
{
	u32 reg_gmaccr;

	reg_gmaccr = _ESW_REG(REG_ESW_MAC_GMACCR);
	reg_gmaccr &= ~(0x3f);
	reg_gmaccr |= (9u << 2);		// MAX_RX_JUMBO = 9 KB

	if (jumbo_frames_enabled) {
		reg_gmaccr |= 0x3;		// MAX_RX_JUMBO
	} else {
		reg_gmaccr |= 0x1;		// 1536 bytes
	}

	_ESW_REG(REG_ESW_MAC_GMACCR) = reg_gmaccr;
}

static void esw_led_mode(u32 led_mode)
{
	u32 reg_gpc1;

	reg_gpc1 = _ESW_REG(REG_ESW_GPC1);
	reg_gpc1 &= ~(0xC000);

	if (led_mode == SWAPI_LED_OFF)
		reg_gpc1 |= 0xC000;

	_ESW_REG(REG_ESW_GPC1) = reg_gpc1;
}

static u32 esw_status_link_port(u32 port_id)
{
	u32 port_link;
	u32 reg_pmsr = _ESW_REG(REG_ESW_MAC_PMSR_P0 + 0x100*port_id);
	port_link = (reg_pmsr & 0x1);
	return port_link;
}

static u32 esw_status_speed_port(u32 port_id)
{
	u32 port_link;
	u32 port_duplex;
	u32 port_speed;
	u32 reg_pmsr = _ESW_REG(REG_ESW_MAC_PMSR_P0 + 0x100*port_id);
	port_link = (reg_pmsr & 0x1);
	if (!port_link)
		return 0;

	port_duplex = (reg_pmsr >> 1) & 0x1;
	port_speed  = (reg_pmsr >> 2) & 0x3;

	return ((port_link << 16) | (port_duplex << 8) | port_speed);
}

static u32 esw_status_link_ports(int is_wan_ports)
{
	int i;
	u32 port_link = 0;
	u32 portmask;

	if (is_wan_ports)
		portmask = get_phy_ports_mask_wan(0);
	else
		portmask = get_phy_ports_mask_lan(0);

	for (i = 0; i <= ESW_PHY_ID_MAX; i++)
	{
		if ((portmask >> i) & 0x1)
		{
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

static void esw_status_mib_port(u32 port_id, arl_mib_counters_t *mibc)
{
	u32 reg_tgpc  = _ESW_REG(REG_ESW_MIB_TGPC_P0 + 0x100*port_id);
	u32 reg_rgpc  = _ESW_REG(REG_ESW_MIB_RGPC_P0 + 0x100*port_id);
	u32 reg_repc1 = _ESW_REG(REG_ESW_MIB_REPC1_P0 + 0x100*port_id);
	u32 reg_repc2 = _ESW_REG(REG_ESW_MIB_REPC2_P0 + 0x100*port_id);

	mibc->TxGoodOctets    = _ESW_REG(REG_ESW_MIB_TGOC_P0 + 0x100*port_id);
	mibc->TxGoodFrames    = (reg_tgpc & 0xffff);
	mibc->TxBadOctets     = _ESW_REG(REG_ESW_MIB_TBOC_P0 + 0x100*port_id);
	mibc->TxBadFrames     = (reg_tgpc >> 16);
	mibc->TxDropFrames    = (_ESW_REG(REG_ESW_MIB_TEPC_P0 + 0x100*port_id)) & 0xffff;

	mibc->RxGoodOctets    = _ESW_REG(REG_ESW_MIB_RGOC_P0 + 0x100*port_id);
	mibc->RxGoodFrames    = (reg_rgpc & 0xffff);
	mibc->RxBadOctets     = _ESW_REG(REG_ESW_MIB_RBOC_P0 + 0x100*port_id);
	mibc->RxBadFrames     = (reg_rgpc >> 16);

	mibc->RxDropFramesFilter  = (reg_repc2 & 0xffff);
	mibc->RxDropFramesIngress = (reg_repc1 & 0xffff);
	mibc->RxDropFramesControl = (reg_repc1 >> 16);
	mibc->RxDropFramesLimiter = (reg_repc2 >> 16);
}

static void change_ports_power(u32 power_on, u32 port_mask)
{
	u32 i;

	port_mask = get_phy_ports_mask_from_user(port_mask & 0xFF);

	for (i = 0; i <= ESW_PHY_ID_MAX; i++)
	{
		if ((port_mask >> i) & 0x1)
			esw_port_phy_power(i, power_on);
	}
}

static int change_wan_ports_power(u32 power_on)
{
	int power_changed = 0;
	u32 i, ports_mask_wan;

	ports_mask_wan = get_phy_ports_mask_wan(0);

	for (i = 0; i <= ESW_PHY_ID_MAX; i++)
	{
		if (((ports_mask_wan >> i) & 0x1) && (g_port_phy_power[i] ^ power_on)) {
			power_changed = 1;
			esw_port_phy_power(i, power_on);
		}
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
		power_changed = change_wan_ports_power(0);
		if (power_changed) {
			// wait for PHY link down
			msleep(1000);
		}
	}

	esw_vlan_bridge_isolate(wan_bridge_mode, wan_bwan_isolation, bridge_changed, br_iso_changed, vlan_rule_changed);

	if (power_changed)
		change_wan_ports_power(1);

	return 0;
}

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

	port_mask = get_phy_ports_mask_from_user(port_mask & 0xFF);

	for (i = 0; i <= ESW_PORT_ID_MAX; i++) {
		if ((port_mask >> i) & 0x1)
			esw_port_accept_set(i, admit_frames);
	}
}

static void vlan_create_entry(u32 vlan4k_info, u32 vlan4k_mask, int set_port_vid)
{
	u32 i, cvid, svid, prio, fid, vlan_table_idx;
	u32 mask_member, mask_untag;

	cvid = (vlan4k_info & 0x0FFF);
	prio = ((vlan4k_info >> 12) & 0x7);
	fid  = ((vlan4k_info >> 16) & 0xFF);
	mask_member = get_phy_ports_mask_from_user((vlan4k_mask & 0xFF));
	mask_untag  = get_phy_ports_mask_from_user((vlan4k_mask >> 16) & 0xFF);
#if defined (ESW_PORT_PPE)
	if (mask_member & (1u << ESW_PORT_CPU))
		mask_member |= (1u << ESW_PORT_PPE);
#endif
	if (cvid < 1) cvid = 1;
	svid = (cvid > 2) ? cvid : 0;

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
	esw_vlan_set_idx(vlan_table_idx, cvid, svid, mask_member, fid);

	/* set phy ports attrib */
	for (i = 0; i <= ESW_PHY_ID_MAX; i++) {
		if ((1u << i) & mask_member) {
			if (!((1u << i) & mask_untag)) {
				esw_port_attrib_set(i, PORT_ATTRIBUTE_USER);
				esw_port_egress_mode_set(i, (i == WAN_PORT_X) ? PVLAN_EGRESS_SWAP : PVLAN_EGRESS_TAG);
			} else {
				if (set_port_vid)
					esw_vlan_pvid_set(i, cvid, prio);
				esw_port_attrib_set(i, PORT_ATTRIBUTE_TRANSPARENT);
				esw_port_egress_mode_set(i, PVLAN_EGRESS_UNTAG);
			}
		}
	}

	printk("%s - create vlan %s: vid=[%d], prio=[%d], member=[0x%02X], untag=[0x%02X], fid=[%d]\n",
			MTK_ESW_DEVNAME, (set_port_vid) ? "ports" : "entry", cvid, prio, mask_member, mask_untag, fid);
}

static void change_port_link_mode(u32 port_id, u32 port_link_mode)
{
	u32 i_port_speed;
	u32 i_port_flowc;
	u32 esw_phy_ana = 0x05e1;
	u32 esw_phy_mcr = 0x3300;

	if (g_port_link_mode[port_id] == port_link_mode)
		return;

	i_port_speed =  (port_link_mode & 0x07);
	i_port_flowc = ((port_link_mode >> 8) & 0x03);

	printk("%s - port [%d] link speed: %d, flow control: %d\n", MTK_ESW_DEVNAME, port_id, i_port_speed, i_port_flowc);

	switch (i_port_flowc)
	{
	case SWAPI_LINK_FLOW_CONTROL_DISABLE:
		esw_phy_ana &= ~(1<<10); // disable pause support (A5)
		break;
	}

	switch (i_port_speed)
	{
	case SWAPI_LINK_SPEED_MODE_1000_FD:
		break;
	case SWAPI_LINK_SPEED_MODE_100_FD:
		esw_phy_ana &= ~((1<<7)|(1<<6)|(1<<5)); // disable 100Base-TX Half Duplex, 10 Base-T Full Duplex, 10 Base-T Half Duplex
		break;
	case SWAPI_LINK_SPEED_MODE_100_HD:
		esw_phy_ana &= ~((1<<8)|(1<<6)|(1<<5)); // disable 100Base-TX Full Duplex, 10 Base-T Full Duplex, 10 Base-T Half Duplex
		break;
	case SWAPI_LINK_SPEED_MODE_10_FD:
		esw_phy_ana &= ~((1<<8)|(1<<7)|(1<<5)); // disable 100Base-TX Full Duplex, 100 Base-TX Half Duplex, 10 Base-T Half Duplex
		break;
	case SWAPI_LINK_SPEED_MODE_10_HD:
		esw_phy_ana &= ~((1<<8)|(1<<7)|(1<<6)); // disable 100Base-TX Full Duplex, 100 Base-TX Half Duplex, 10 Base-T Full Duplex
		break;
	}

	/* set PHY ability */
	mii_mgr_write(port_id, 4, esw_phy_ana);

	/* restart auto-negotiation */
	if (g_port_phy_power[port_id])
		mii_mgr_write(port_id, 0, esw_phy_mcr);
}

static void change_storm_control_multicast_unknown(u32 control_rate_mbps)
{
	u32 i;

	if (control_rate_mbps >= 1024)
		control_rate_mbps = 0;

	if (g_storm_rate_limit != control_rate_mbps)
	{
		g_storm_rate_limit = control_rate_mbps;
		
		if (control_rate_mbps > 0)
			printk("%s - set unknown multicast and broadcast storm control rate as: %d mbps\n", MTK_ESW_DEVNAME, control_rate_mbps);
		
		for (i = 0; i <= ESW_PHY_ID_MAX; i++)
			esw_storm_control(i, 1, 1, 0, control_rate_mbps);
	}
}

static void change_jumbo_frames_accept(u32 jumbo_frames_enabled)
{
	if (jumbo_frames_enabled) jumbo_frames_enabled = 1;

	if (g_jumbo_frames_enabled != jumbo_frames_enabled)
	{
		g_jumbo_frames_enabled = jumbo_frames_enabled;
		printk("%s - jumbo frames accept: %s bytes\n", MTK_ESW_DEVNAME, (jumbo_frames_enabled) ? "9000" : "1536");
		
		esw_jumbo_control(jumbo_frames_enabled);
	}
}

static void change_igmp_snooping_control(u32 igmp_snooping_enabled)
{
	if (igmp_snooping_enabled) igmp_snooping_enabled = 1;

	if (g_igmp_snooping_enabled != igmp_snooping_enabled)
	{
		g_igmp_snooping_enabled = igmp_snooping_enabled;
		printk("%s - IGMP/MLD snooping: %d\n", MTK_ESW_DEVNAME, igmp_snooping_enabled);
		
		esw_igmp_mld_snooping(igmp_snooping_enabled, igmp_snooping_enabled);
	}
}

static void change_led_mode(u32 led_mode)
{
	if (led_mode != SWAPI_LED_OFF)
		led_mode = SWAPI_LED_LINK_ACT;

	if (g_led_phy_mode != led_mode)
	{
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

static void esw_link_status_changed(u32 port_id)
{
	u32 reg_val;
	char *port_desc;
	char *port_state;

	if (port_id <= ESW_PHY_ID_MAX)
		atomic_set(&g_port_link_changed, 1);

#if !ESW_PRINT_LINK_ALL
	if (port_id != WAN_PORT_X)
		return;
	port_desc = "WAN";
#else
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
	case LAN_PORT_CPU:
		port_desc = "CPU";
		break;
	default:
		port_desc = "RGMII";
		break;
	}
#endif

	reg_val = *((volatile u32 *)(RALINK_ETH_SW_BASE + 0x3008 + (port_id*0x100)));
	if (reg_val & 0x1)
		port_state = "Up";
	else
		port_state = "Down";

	printk("%s: Link Status Changed - Port %s Link %s\n", MTK_ESW_DEVNAME, port_desc, port_state);
}

irqreturn_t esw_interrupt(int irq, void *dev_id)
{
	u32 reg_int_val;

	reg_int_val = sysRegRead(ESW_ISR);
	if (!reg_int_val)
		return IRQ_NONE;

	sysRegWrite(ESW_ISR, reg_int_val);

	if (reg_int_val & ACL_INT) {
		unsigned int acl_int_val = sysRegRead(ESW_AISR);
		sysRegWrite(ESW_AISR, acl_int_val);
	}

	if (reg_int_val & P5_LINK_CH)
		esw_link_status_changed(5);

	if (reg_int_val & P4_LINK_CH)
		esw_link_status_changed(4);

	if (reg_int_val & P3_LINK_CH)
		esw_link_status_changed(3);

	if (reg_int_val & P2_LINK_CH)
		esw_link_status_changed(2);

	if (reg_int_val & P1_LINK_CH)
		esw_link_status_changed(1);

	if (reg_int_val & P0_LINK_CH)
		esw_link_status_changed(0);

	return IRQ_HANDLED;
}

int esw_control_post_init(void)
{
	/* configure CPU and PPE ports */
	esw_init_ports_cpu_ppe();

	/* configure PHY ports */
	esw_init_ports_phy();

	/* configure bridge isolation mode via VLAN */
	esw_vlan_bridge_isolate(g_wan_bridge_mode, g_wan_bwan_isolation, 1, 1, 1);

	/* configure igmp/mld snooping */
	esw_igmp_mld_snooping(g_igmp_snooping_enabled, g_igmp_snooping_enabled);

	/* configure leds */
	esw_led_mode(g_led_phy_mode);

	atomic_set(&g_switch_inited, 1);

	return 0;
}

static void reset_and_init_switch(void)
{
	esw_soft_reset();

	esw_control_post_init();
}

static void pre_init_switch(void)
{
	u32 i;

	/* down all PHY ports (please enable from user-level) */
	for (i = 0; i <= ESW_PHY_ID_MAX; i++)
		esw_port_phy_power(i, 0);
}

static void fill_bridge_members(void)
{
	u32 i, j;

	for (i = 0; i < SWAPI_WAN_BRIDGE_NUM; i++) {
		for (j = 0; j <= ESW_PHY_ID_MAX; j++) {
			g_bwan_member[i][j].bwan = 0;
			g_bwan_member[i][j].rule = 0;
		}
	}

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

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long mtk_esw_ioctl(struct file *file, unsigned int req, unsigned long arg)
#else
int mtk_esw_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	int ioctl_result = 0;
	u32 uint_value = 0;
	u32 uint_result = 0;
	arl_mib_counters_t port_counters;

	unsigned int uint_param = (req >> MTK_ESW_IOCTL_CMD_LENGTH_BITS);
	req &= ((1u << MTK_ESW_IOCTL_CMD_LENGTH_BITS)-1);

	mutex_lock(&esw_access_mutex);

	switch(req)
	{
	case MTK_ESW_IOCTL_GPIO_MODE_SET:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ralink_gpio_mode_set(uint_value);
		break;
	case MTK_ESW_IOCTL_GPIO_MODE_SET_BIT:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ralink_gpio_mode_set_bit(uint_param, uint_value);
		break;
	case MTK_ESW_IOCTL_GPIO_MODE_GET:
		uint_result = ralink_gpio_mode_get();
		put_user(uint_result, (unsigned int __user *)arg);
		break;
	case MTK_ESW_IOCTL_GPIO_PIN_SET_DIRECTION:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ralink_gpio_set_pin_direction(uint_param, uint_value);
		break;
	case MTK_ESW_IOCTL_GPIO_PIN_SET_VAL:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ralink_gpio_set_pin_value(uint_param, uint_value);
		break;
	case MTK_ESW_IOCTL_GPIO_PIN_GET_VAL:
		uint_result = ralink_gpio_get_pin_value(uint_param);
		put_user(uint_result, (unsigned int __user *)arg);
		break;

	case MTK_ESW_IOCTL_STATUS_LINK_PORT_WAN:
		uint_result = esw_status_link_port(WAN_PORT_X);
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
		uint_result = esw_status_speed_port(WAN_PORT_X);
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
		esw_status_mib_port(WAN_PORT_X, &port_counters);
		copy_to_user((arl_mib_counters_t __user *)arg, &port_counters, sizeof(arl_mib_counters_t));
		break;
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN1:
		esw_status_mib_port(LAN_PORT_1, &port_counters);
		copy_to_user((arl_mib_counters_t __user *)arg, &port_counters, sizeof(arl_mib_counters_t));
		break;
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN2:
		esw_status_mib_port(LAN_PORT_2, &port_counters);
		copy_to_user((arl_mib_counters_t __user *)arg, &port_counters, sizeof(arl_mib_counters_t));
		break;
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN3:
		esw_status_mib_port(LAN_PORT_3, &port_counters);
		copy_to_user((arl_mib_counters_t __user *)arg, &port_counters, sizeof(arl_mib_counters_t));
		break;
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_LAN4:
		esw_status_mib_port(LAN_PORT_4, &port_counters);
		copy_to_user((arl_mib_counters_t __user *)arg, &port_counters, sizeof(arl_mib_counters_t));
		break;
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_CPU_WAN:
		esw_status_mib_port(WAN_PORT_CPU, &port_counters);
		copy_to_user((arl_mib_counters_t __user *)arg, &port_counters, sizeof(arl_mib_counters_t));
		break;
	case MTK_ESW_IOCTL_STATUS_CNT_PORT_CPU_LAN:
		esw_status_mib_port(LAN_PORT_CPU, &port_counters);
		copy_to_user((arl_mib_counters_t __user *)arg, &port_counters, sizeof(arl_mib_counters_t));
		break;

	case MTK_ESW_IOCTL_RESET_SWITCH:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		if (uint_value == SWAPI_MAGIC_RESET_ASIC)
			reset_and_init_switch();
		break;
	case MTK_ESW_IOCTL_PORT_POWER:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_ports_power(uint_param, uint_value);
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

	case MTK_ESW_IOCTL_STORM_MULTICAST_UNK:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_storm_control_multicast_unknown(uint_value);
		break;

	case MTK_ESW_IOCTL_JUMBO_FRAMES:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_jumbo_frames_accept(uint_value);
		break;

	case MTK_ESW_IOCTL_IGMP_SNOOPING:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_igmp_snooping_control(uint_value);
		break;

	case MTK_ESW_IOCTL_LED_MODE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_led_mode(uint_value);
		break;

	case MTK_ESW_IOCTL_SPEED_PORT_WAN:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		change_port_link_mode(WAN_PORT_X, uint_value);
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

int esw_get_traffic_port_wan(struct rtnl_link_stats64 *stats)
{
	u32 TxGoodOctetsCount = _ESW_REG(REG_ESW_MIB_TGOC_P0 + 0x100*WAN_PORT_X);
	u32 TxGoodFramesCount = (_ESW_REG(REG_ESW_MIB_TGPC_P0 + 0x100*WAN_PORT_X) & 0xffff);

	u32 RxGoodOctetsCount = _ESW_REG(REG_ESW_MIB_RGOC_P0 + 0x100*WAN_PORT_X);
	u32 RxGoodFramesCount = (_ESW_REG(REG_ESW_MIB_RGPC_P0 + 0x100*WAN_PORT_X) & 0xffff);

	stats->rx_bytes = (RxGoodOctetsCount - (RxGoodFramesCount * 4)); // cut FCS
	stats->rx_packets = RxGoodFramesCount;

	stats->tx_bytes = (TxGoodOctetsCount - (TxGoodFramesCount * 4)); // cut FCS
	stats->tx_packets = TxGoodFramesCount;

	return 0;
}
EXPORT_SYMBOL(esw_get_traffic_port_wan);

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
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	.unlocked_ioctl	= mtk_esw_ioctl,
#else
	.ioctl		= mtk_esw_ioctl,
#endif
	.open		= mtk_esw_open,
	.release	= mtk_esw_release,
};

////////////////////////////////////////////////////////////////////////////////////

int esw_ioctl_init(void)
{
	int r;

	mutex_init(&esw_access_mutex);

	fill_bridge_members();

	r = register_chrdev(MTK_ESW_DEVMAJOR, MTK_ESW_DEVNAME, &mtk_esw_fops);
	if (r < 0) {
		printk(KERN_ERR MTK_ESW_DEVNAME ": unable to register character device\n");
		return r;
	}

	pre_init_switch();

	return 0;
}

void esw_ioctl_uninit(void)
{
	unregister_chrdev(MTK_ESW_DEVMAJOR, MTK_ESW_DEVNAME);
}

