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

#ifndef __MT7530_MCM_H__
#define __MT7530_MCM_H__

void mcm_init(void);
void mcm_uninit(void);

void mcm_vlan_set_mode_matrix(void);
void mcm_vlan_set_port_wan(u32 pvid, u32 prio, int tagg);
void mcm_vlan_set_port_cpu(u32 pvid, u32 prio, int tagg);
void mcm_vlan_set_entries(vlan_entry_t *vlan_entry, u32 entry_count);
void mcm_vlan_reset_table(void);

int  mcm_mac_table_clear(void);
void mcm_mib_reset(void);
void mcm_led_mode(u32 led_mode);
void mcm_storm_control(u32 port_id, int set_bcast, int set_mcast, int set_ucast, u32 rate_mbps);

int  mcm_set_port_phy_power(u32 port_id, u32 power_on);
int  mcm_set_port_link_mode(u32 port_id, u32 port_link_mode);

u32  mcm_status_link_changed(void);
u32  mcm_status_link_port(u32 port_id);
u32  mcm_status_speed_port(u32 port_id);
int  mcm_status_bytes_port(u32 port_id, port_bytes_t *pb);
int  mcm_status_mib_port(u32 port_id, rtk_stat_port_cntr_t *pc);

#endif
