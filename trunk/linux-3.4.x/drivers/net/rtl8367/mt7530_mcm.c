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
#include <linux/delay.h>

#include "rtl8367_def.h"
#include "rtl8367_ioctl.h"

#if defined(CONFIG_RTL8367_API_8370)
#include "api_8370/rtk_types.h"
#include "api_8370/rtk_api.h"
#else
#include "api_8367b/rtk_types.h"
#include "api_8367b/rtk_api.h"
#endif

#include "../raeth/mii_mgr.h"
#include "../raeth/ra_eth_mt7621.h"
#include "../raeth/ra_esw_reg.h"
#include "../raeth/ra_esw_base.h"

#include "mt7530_mcm.h"

#define MCM_LOG_PREFIX			"mt7621-gsw"

#define MCM_EPHY_ID_MAX			4
#define MCM_WAN_PORT_CPU		6
#define MCM_MASK_WAN_PORT_X		(1u << MCM_WAN_PORT_X)
#define MCM_MASK_WAN_PORT_CPU		(1u << MCM_WAN_PORT_CPU)
#define MCM_MASK_WAN_MATRIX		(MCM_MASK_WAN_PORT_X|MCM_MASK_WAN_PORT_CPU)

#define PVLAN_INGRESS_MODE_MATRIX	0x00
#define PVLAN_INGRESS_MODE_FALLBACK	0x01
#define PVLAN_INGRESS_MODE_CHECK	0x02
#define PVLAN_INGRESS_MODE_SECURITY	0x03

#define PORT_ACCEPT_FRAMES_ALL		0x00
#define PORT_ACCEPT_FRAMES_TAGGED	0x01
#define PORT_ACCEPT_FRAMES_UNTAGGED	0x02

#define PORT_ATTRIBUTE_USER		0x00
#define PORT_ATTRIBUTE_STACK		0x01
#define PORT_ATTRIBUTE_TRANSLATION	0x02
#define PORT_ATTRIBUTE_TRANSPARENT	0x03

////////////////////////////////////////////////////////////////////////////////////

static DECLARE_BITMAP(g_vlan_pool, 4096);
static u32 g_port_link_mode = 0;
static atomic_t g_port_link_changed = ATOMIC_INIT(0);

////////////////////////////////////////////////////////////////////////////////////

static int
mcm_write_vtcr(u32 vtcr_cmd, u32 vtcr_val)
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

	return -1;
}

static void
mcm_port_matrix_set(u32 port_id, u32 fwd_mask, u32 pvlan_ingress_mode)
{
	u32 reg_pcr;

	if (port_id > ESW_PORT_ID_MAX)
		return;

	reg_pcr = esw_reg_get(REG_ESW_PORT_PCR_P0 + 0x100*port_id);
	reg_pcr &= ~(0xFF << 16);
	reg_pcr &= ~(0x03);
	reg_pcr |= ((fwd_mask & 0xFF) << 16);
	reg_pcr |= (pvlan_ingress_mode & 0x03);
	esw_reg_set(REG_ESW_PORT_PCR_P0 + 0x100*port_id, reg_pcr);
}

static void
mcm_port_attrib_set(u32 port_id, u32 port_attribute, u32 accept_frames)
{
	u32 reg_pvc;

	if (port_id > ESW_PORT_ID_MAX)
		return;

	reg_pvc = esw_reg_get(REG_ESW_PORT_PVC_P0 + 0x100*port_id);
	reg_pvc &= 0x0000FF3C;
	reg_pvc |= 0x81000000; // STAG VPID 8100
	reg_pvc |= ((port_attribute & 0x03) << 6);
	reg_pvc |= (accept_frames & 0x03);
	esw_reg_set(REG_ESW_PORT_PVC_P0 + 0x100*port_id, reg_pvc);
}

static void
mcm_port_pvid_set(u32 port_id, u32 pvid, u32 prio)
{
	u32 reg_ppbv;

	if (port_id > ESW_PORT_ID_MAX)
		return;

	reg_ppbv = (1u << 16) | ((prio & 0x7) << 13) | (pvid & 0xfff);
	esw_reg_set(REG_ESW_PORT_PPBV1_P0+0x100*port_id, reg_ppbv);
}

static void
mcm_vlan_set(u32 cvid, u32 mask_member, u32 mask_untag)
{
	u32 i, reg_val, reg_val2;

	cvid &= 0xfff;
	mask_member &= 0x7f;
	mask_untag &= 0x7f;

	reg_val2 = 0;

	// set vlan member
	reg_val = 1;				// VALID
	reg_val |= (1u << 30);			// IVL=1
	reg_val |= (mask_member << 16);		// PORT_MEM
	reg_val |= (1u << 28);			// VTAG_EN=1

	/* set vlan untag ports */
	for (i = 0; i < 7; i++) {
		if (!(mask_member & (1u << i)))
			continue;
		
		if (!(mask_untag & (1u << i)))
			reg_val2 |= (0x2u << (i*2));	// EG_TAG=Tagged
	}

	esw_reg_set(REG_ESW_VLAN_VAWD1, reg_val);
	esw_reg_set(REG_ESW_VLAN_VAWD2, reg_val2);
	mcm_write_vtcr(1, cvid);
	set_bit(cvid, g_vlan_pool);
}

void
mcm_vlan_set_mode_matrix(void)
{
	mcm_port_matrix_set(MCM_WAN_PORT_X, MCM_MASK_WAN_MATRIX, PVLAN_INGRESS_MODE_MATRIX);
	mcm_port_matrix_set(MCM_WAN_PORT_CPU, MCM_MASK_WAN_MATRIX, PVLAN_INGRESS_MODE_MATRIX);

	mcm_port_attrib_set(MCM_WAN_PORT_X, PORT_ATTRIBUTE_TRANSPARENT, PORT_ACCEPT_FRAMES_ALL);
	mcm_port_attrib_set(MCM_WAN_PORT_CPU, PORT_ATTRIBUTE_TRANSPARENT, PORT_ACCEPT_FRAMES_ALL);
}

void
mcm_vlan_set_port_wan(u32 pvid, u32 prio, int tagg)
{
	u32 port_attrib = (tagg) ? PORT_ATTRIBUTE_USER : PORT_ATTRIBUTE_TRANSPARENT;
	u32 port_accept = (tagg) ? PORT_ACCEPT_FRAMES_ALL : PORT_ACCEPT_FRAMES_UNTAGGED;

	mcm_port_matrix_set(MCM_WAN_PORT_X, MCM_MASK_WAN_MATRIX, PVLAN_INGRESS_MODE_SECURITY);
	mcm_port_attrib_set(MCM_WAN_PORT_X, port_attrib, port_accept);
	mcm_port_pvid_set(MCM_WAN_PORT_X, pvid, prio);
}

void
mcm_vlan_set_port_cpu(u32 pvid, u32 prio, int tagg)
{
	u32 port_attrib = (tagg) ? PORT_ATTRIBUTE_USER : PORT_ATTRIBUTE_TRANSPARENT;

	mcm_port_matrix_set(MCM_WAN_PORT_CPU, MCM_MASK_WAN_MATRIX, PVLAN_INGRESS_MODE_SECURITY);
	mcm_port_attrib_set(MCM_WAN_PORT_CPU, port_attrib, PORT_ACCEPT_FRAMES_ALL);
	mcm_port_pvid_set(MCM_WAN_PORT_CPU, pvid, prio);
}

void
mcm_vlan_set_entries(vlan_entry_t *vlan_entry, u32 entry_count)
{
	u32 i, mask_member, mask_untag;

	/* skip VID #1 */
	for (i = 1; i < entry_count; i++) {
		if (!vlan_entry[i].valid)
			continue;
		if (!vlan_entry[i].port_member)
			continue;
		
		mask_member = MCM_MASK_WAN_PORT_X;
		mask_untag = 0;
		
		if (vlan_entry[i].port_member & (1u << WAN_PORT_CPU))
			mask_member |= MCM_MASK_WAN_PORT_CPU;
		
		if (vlan_entry[i].port_untag & (1u << WAN_PORT_X))
			mask_untag |= MCM_MASK_WAN_PORT_X;
		
		if (vlan_entry[i].port_untag & (1u << WAN_PORT_CPU))
			mask_untag |= MCM_MASK_WAN_PORT_CPU;
		
		mcm_vlan_set(vlan_entry[i].cvid, mask_member, mask_untag);
	}
}

void
mcm_vlan_reset_table(void)
{
	u32 i;

	/* Reset VLAN table from VID #1 */
	for (i = 1; i < 4096; i++) {
		if (test_and_clear_bit(i, g_vlan_pool))
			mcm_write_vtcr(2, i);
	}
}

int
mcm_mac_table_clear(void)
{
	u32 i, atc_val;

	esw_reg_set(REG_ESW_WT_MAC_ATC, 0x8002);

	for (i = 0; i < 200; i++) {
		udelay(100);
		atc_val = esw_reg_get(REG_ESW_WT_MAC_ATC);
		if (!(atc_val & BIT(15)))
			return 0;
	}

	return -1;
}

void
mcm_mib_reset(void)
{
	u32 reg_mib;

	reg_mib = esw_reg_get(0x4fe0);
	esw_reg_set(0x4fe0, reg_mib & ~BIT(31));
	esw_reg_set(0x4fe0, reg_mib |  BIT(31));
}

void
mcm_led_mode(u32 led_mode)
{
	u32 reg_ledc;

	reg_ledc = esw_reg_get(0x7d00);
	reg_ledc |= 0x77777;

	if (led_mode == SWAPI_LED_OFF)
		reg_ledc &= ~(0x77777);

	esw_reg_set(0x7d00, reg_ledc);
}

void
mcm_storm_control(u32 port_id, int set_bcast, int set_mcast, int set_ucast, u32 rate_mbps)
{
	u32 reg_bsr = 0;
	u32 rate_unit_1000;
	u32 rate_unit_100;
	u32 rate_unit_10;

	if (rate_mbps >= 1024)
		rate_mbps = 0;

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

int
mcm_set_port_phy_power(u32 port_id, u32 power_on)
{
	u32 esw_phy_mcr = 0x3100;
	u32 i_port_speed, is_power_on;

	if (port_id > MCM_EPHY_ID_MAX)
		return 0;

	i_port_speed = (g_port_link_mode & 0x0F);
	if (i_port_speed == SWAPI_LINK_SPEED_MODE_FORCE_POWER_OFF)
		is_power_on = 0;

	is_power_on = 0;
	if (mii_mgr_read(port_id, 0, &esw_phy_mcr)) {
		is_power_on = (esw_phy_mcr & BIT(11)) ? 0 : 1;
		esw_phy_mcr &= ~(BIT(11)|BIT(9));
		
		/* fix PHY init after buggy Uboot 4.3.0.0 */
		if (i_port_speed < SWAPI_LINK_SPEED_MODE_FORCE_100_FD)
			esw_phy_mcr |= BIT(12);
		
		if (power_on)
			esw_phy_mcr |= BIT(9);
		else
			esw_phy_mcr |= BIT(11);
		
		if (is_power_on ^ power_on)
			mii_mgr_write(port_id, 0, esw_phy_mcr);
	}

	/* return 1 when PHY power is changed */
	return (is_power_on ^ power_on) ? 1 : 0;
}

int
mcm_set_port_link_mode(u32 port_id, u32 port_link_mode)
{
	const char *link_desc = "Auto", *flow_desc = "ON";
	u32 i_port_speed, i_port_flowc, i_port_power;
	u32 esw_phy_ana = 0x05e1;
	u32 esw_phy_mcr = 0x3100; /* 100 FD + auto-negotiation */
	u32 esw_phy_gcr = 0;

	if (port_id > MCM_EPHY_ID_MAX)
		return -EINVAL;

	if (g_port_link_mode == port_link_mode)
		return 0;

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
	mii_mgr_read(port_id, 9, &esw_phy_gcr);
	if (i_port_speed <= SWAPI_LINK_SPEED_MODE_AUTO_1000_FD) {
		/* enable 1000Base-T Advertisement */
		esw_phy_gcr |=  ((1<<9)|(1<<8));
	} else {
		/* disable 1000Base-T Advertisement */
		esw_phy_gcr &= ~((1<<9)|(1<<8));
	}
	mii_mgr_write(port_id, 9, esw_phy_gcr);

	/* set PHY ability */
	mii_mgr_write(port_id, 4, esw_phy_ana);

	if (i_port_power) {
		if (!(esw_phy_mcr & (1<<12))) {
			/* power-down PHY */
			esw_phy_mcr |= (1<<11);
			
			/* set PHY mode */
			mii_mgr_write(port_id, 0, esw_phy_mcr);
			
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
	mii_mgr_write(port_id, 0, esw_phy_mcr);

	g_port_link_mode = port_link_mode;

	if (!i_port_power) {
		link_desc = "Power OFF";
		flow_desc = "N/A";
	}

	printk("%s - %s link speed: %s, flow control: %s\n",
		MCM_LOG_PREFIX, "WAN", link_desc, flow_desc);

	return 0;
}

static void
mcm_link_state_changed(u32 port_id, int port_link)
{
	const char *port_state;

	if (port_id > MCM_EPHY_ID_MAX)
		return;

	atomic_set(&g_port_link_changed, 1);

	port_state = (port_link) ? "Up" : "Down";

	printk("%s: Link Status Changed - Port %s Link %s\n",
		MCM_LOG_PREFIX, "WAN", port_state);
}

u32
mcm_status_link_changed(void)
{
	return atomic_cmpxchg(&g_port_link_changed, 1, 0);
}

u32
mcm_status_link_port(u32 port_id)
{
	u32 reg_pmsr;

	if (port_id > MCM_EPHY_ID_MAX)
		return 0;

	reg_pmsr = esw_reg_get(REG_ESW_MAC_PMSR_P0 + 0x100*port_id);
	return (reg_pmsr & 0x1);
}

u32
mcm_status_speed_port(u32 port_id)
{
	u32 port_link, port_duplex, port_speed;
	u32 port_eee, port_fc_rx, port_fc_tx;
	u32 reg_pmsr;

	if (port_id > MCM_EPHY_ID_MAX)
		return 0;

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

int
mcm_status_bytes_port(u32 port_id, port_bytes_t *pb)
{
	ULARGE_INTEGER rx_goct, tx_goct;

	if (port_id > MCM_EPHY_ID_MAX)
		return -EINVAL;

	tx_goct.u.LowPart  = esw_reg_get(0x4048 + 0x100*port_id);
	tx_goct.u.HighPart = esw_reg_get(0x404C + 0x100*port_id);

	rx_goct.u.LowPart  = esw_reg_get(0x40A8 + 0x100*port_id);
	rx_goct.u.HighPart = esw_reg_get(0x40AC + 0x100*port_id);

	pb->RX = rx_goct.QuadPart;
	pb->TX = tx_goct.QuadPart;

	return 0;
}

int
mcm_status_mib_port(u32 port_id, rtk_stat_port_cntr_t *pc)
{
	ULARGE_INTEGER rx_goct, tx_goct;

	if (port_id > MCM_EPHY_ID_MAX)
		return -EINVAL;

	memset(pc, 0, sizeof(rtk_stat_port_cntr_t));

	tx_goct.u.LowPart  = esw_reg_get(0x4048 + 0x100*port_id);
	tx_goct.u.HighPart = esw_reg_get(0x404C + 0x100*port_id);

	rx_goct.u.LowPart  = esw_reg_get(0x40A8 + 0x100*port_id);
	rx_goct.u.HighPart = esw_reg_get(0x40AC + 0x100*port_id);

	pc->dot3OutPauseFrames   = esw_reg_get(0x402c + 0x100*port_id);	// TxPauseFrames
	pc->etherStatsDropEvents = esw_reg_get(0x4060 + 0x100*port_id);	// RxDropFrames
	pc->etherStatsMcastPkts  = esw_reg_get(0x406c + 0x100*port_id);	// RxMcastFrames
	pc->etherStatsBcastPkts  = esw_reg_get(0x4070 + 0x100*port_id);	// RxBcastFrames
	pc->dot3StatsFCSErrors   = esw_reg_get(0x4078 + 0x100*port_id);	// RxCRCError
	pc->dot3InPauseFrames    = esw_reg_get(0x408c + 0x100*port_id);	// RxPauseFrames

	pc->ifInOctets = rx_goct.QuadPart;
	pc->ifOutOctets = tx_goct.QuadPart;

	return 0;
}

void
mcm_init(void)
{
#if defined (CONFIG_RALINK_MT7621)
	mt7621_eth_init();
#endif
	mcm_vlan_set_mode_matrix();

	esw_link_status_hook = mcm_link_state_changed;
}

void
mcm_uninit(void)
{
	esw_link_status_hook = NULL;
}

