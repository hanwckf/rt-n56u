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
#ifndef _RTL8367_H_
#define _RTL8367_H_

#include <stdint.h>

#define GPIO_DIR_OUT	1
#define GPIO_DIR_IN	0

////////////////////////////////////////////////////////////////////////////////
// RTL8367 STATUS
////////////////////////////////////////////////////////////////////////////////

int phy_status_port_link_wan(unsigned int *p_link_on);
int phy_status_port_link_lan1(unsigned int *p_link_on);
int phy_status_port_link_lan2(unsigned int *p_link_on);
int phy_status_port_link_lan3(unsigned int *p_link_on);
int phy_status_port_link_lan4(unsigned int *p_link_on);
int phy_status_port_link_wan_all(unsigned int *p_link_on);
int phy_status_port_link_lan_all(unsigned int *p_link_on);
int phy_status_port_speed_wan(unsigned int *p_speed_mode);
int phy_status_port_speed_lan1(unsigned int *p_speed_mode);
int phy_status_port_speed_lan2(unsigned int *p_speed_mode);
int phy_status_port_speed_lan3(unsigned int *p_speed_mode);
int phy_status_port_speed_lan4(unsigned int *p_speed_mode);

////////////////////////////////////////////////////////////////////////////////
// RTL8367 CONTROL
////////////////////////////////////////////////////////////////////////////////

int phy_link_port_wan (unsigned int link_mode, unsigned int flow_mode);
int phy_link_port_lan1(unsigned int link_mode, unsigned int flow_mode);
int phy_link_port_lan2(unsigned int link_mode, unsigned int flow_mode);
int phy_link_port_lan3(unsigned int link_mode, unsigned int flow_mode);
int phy_link_port_lan4(unsigned int link_mode, unsigned int flow_mode);

int phy_storm_unicast_unknown(unsigned int storm_rate_mbps);
int phy_storm_multicast_unknown(unsigned int storm_rate_mbps);
int phy_storm_multicast(unsigned int storm_rate_mbps);
int phy_storm_broadcast(unsigned int storm_rate_mbps);

int phy_led_mode_green(unsigned int led_green);
int phy_led_mode_yellow(unsigned int led_yellow);

int phy_green_ethernet(unsigned int green_ethernet_on);
int phy_jumbo_frames(unsigned int jumbo_frames_on);
#if defined(USE_RTL8367_IGMP_SNOOPING)
int phy_igmp_snooping(unsigned int igmp_snooping_on);
#endif

int phy_ports_power(int power_on);
int phy_bridge_mode(unsigned int bridge_mode, int isolated_mode);
int phy_isolate_inic(unsigned int inic_isolated);

int phy_vlan_reset_table(void);
int phy_vlan_ingress_mode(unsigned int port_pask);
int phy_vlan_accept_port_mode(int accept_mode, unsigned int port_pask);
int phy_vlan_create_port_vid(int pvid, int priority, unsigned int member, unsigned int untag, int fid);
int phy_vlan_create_entry(int vid, unsigned int member, unsigned int untag, int fid);
int phy_vlan_rule_set(unsigned int rule_id, int vid, int priority, int tagged);

////////////////////////////////////////////////////////////////////////////////
// RT3883/3662 GPIO CONTROL
////////////////////////////////////////////////////////////////////////////////

int cpu_gpio_mode_set_bit(int idx, unsigned int value);
int cpu_gpio_set_pin_direction(int pin, unsigned int use_output_direction);
int cpu_gpio_set_pin(int pin, unsigned int value);
int cpu_gpio_get_pin(int pin, unsigned int *p_value);
void cpu_gpio_write(int gpionum, int value);

////////////////////////////////////////////////////////////////////////////////
// RTL8367 PROCESS
////////////////////////////////////////////////////////////////////////////////

int rtl8367_main(int argc, char **argv);

#endif


