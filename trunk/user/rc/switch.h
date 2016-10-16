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
 */

#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <stdint.h>

#if defined(USE_RTL8367)
#include <rtl8367_ioctl.h>
int rtl8367_node(void);
int rtl8367_main(int argc, char **argv);
#endif

#if defined(USE_MTK_ESW) || defined(USE_MTK_GSW)
#include <mtk_esw/ioctl.h>
int mtk_esw_node(void);
int mtk_esw_main(int argc, char **argv);
#endif

////////////////////////////////////////////////////////////////////////////////
// SWITCH STATUS
////////////////////////////////////////////////////////////////////////////////

int phy_status_port_link(int port_id_uapi, unsigned int *p_link_on);
int phy_status_port_link_wan_all(unsigned int *p_link_on);
int phy_status_port_link_lan_all(unsigned int *p_link_on);
int phy_status_port_link_changed(unsigned int *p_link_changed);

int phy_status_port_speed(int port_id_uapi, unsigned int *p_speed_mode);

int phy_status_port_bytes(int port_id_uapi, uint64_t *rx, uint64_t *tx);

////////////////////////////////////////////////////////////////////////////////
// SWITCH CONTROL
////////////////////////////////////////////////////////////////////////////////

int phy_set_link_port(int port_id_uapi, unsigned int link_mode, unsigned int flow_mode);

int phy_storm_unicast_unknown(unsigned int storm_rate_mbps);
int phy_storm_multicast_unknown(unsigned int storm_rate_mbps);
int phy_storm_multicast(unsigned int storm_rate_mbps);
int phy_storm_broadcast(unsigned int storm_rate_mbps);

int phy_led_mode_green(unsigned int led_green);
int phy_led_mode_yellow(unsigned int led_yellow);

int phy_green_ethernet(unsigned int green_ethernet_on);
int phy_eee_lpi(unsigned int eee_lpi_on);
int phy_jumbo_frames(unsigned int jumbo_frames_on);
int phy_igmp_static_port(int static_port);
int phy_igmp_snooping(unsigned int igmp_snooping_on);

int phy_ports_power(int power_on);
int phy_ports_wan_power(int power_on);
int phy_ports_lan_power(int power_on);

int phy_bridge_mode(unsigned int bridge_mode, int isolated_mode);
int phy_isolate_inic(unsigned int inic_isolated);
int phy_disable_inic(unsigned int inic_disabled);

int phy_clear_mac_table(void);

int phy_vlan_reset_table(void);
int phy_vlan_pvid_wan_get(void);
int phy_vlan_accept_port_mode(int accept_mode, unsigned int port_pask);
int phy_vlan_create_port_vid(int pvid, int priority, unsigned int member, unsigned int untag, int fid);
int phy_vlan_create_entry(int vid, unsigned int member, unsigned int untag, int fid);
int phy_vlan_rule_set(unsigned int rule_id, int vid, int priority, int tagged);

#endif
