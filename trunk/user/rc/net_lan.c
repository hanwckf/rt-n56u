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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>

#include <nvram/bcmnvram.h>
#include <shutils.h>

#include "rc.h"
#include "rtl8367m.h"

#include <ralink.h>
#include <linux/rtl8367m_drv.h>

static char udhcpc_lan_state[16] = {0};

in_addr_t get_lan_ipaddr(void)
{
	return get_ipv4_addr(IFNAME_BR);
}

int
add_lan_routes(char *lan_ifname)
{
	return add_routes("lan_", "route", lan_ifname);
}

int
del_lan_routes(char *lan_ifname)
{
	return del_routes("lan_", "route", lan_ifname);
}

void
init_loopback(void)
{
	/* Bring up loopback interface */
	ifconfig("lo", IFUP, "127.0.0.1", "255.0.0.0");

	/* Add to routing table */
	route_add("lo", 0, "127.0.0.0", "0.0.0.0", "255.0.0.0");
}

void
init_bridge(void)
{
	int ap_mode = is_ap_mode();
	int wl_radio_on = nvram_get_int("wl_radio_x");
	int rt_radio_on = nvram_get_int("rt_radio_x");
	int wl_mode_x = nvram_get_int("wl_mode_x");
	int rt_mode_x = nvram_get_int("rt_mode_x");
	char *lan_hwaddr = nvram_safe_get("lan_hwaddr");

	doSystem("ifconfig %s hw ether %s", IFNAME_MAC, lan_hwaddr);
	ifconfig(IFNAME_MAC, IFUP, NULL, NULL);

#ifdef USE_SINGLE_MAC
	if (!ap_mode)
	{
		/* create VLAN1/2  */
		doSystem("vconfig add %s %d", IFNAME_MAC, 1);
		doSystem("vconfig add %s %d", IFNAME_MAC, 2);
		doSystem("ifconfig %s hw ether %s txqueuelen %d", IFNAME_LAN, lan_hwaddr, 1000);
		doSystem("ifconfig %s txqueuelen %d", IFNAME_WAN, 1000);
		ifconfig(IFNAME_LAN, IFUP, NULL, NULL);
	}
#endif
	if (!wl_radio_on || (wl_mode_x == 1 || wl_mode_x == 3))
	{
		/* workaround for create all pseudo interfaces and fix rt3090_ap issue */
		gen_ralink_config_wl(1);
		ifconfig(WIF, IFUP, NULL, NULL);
	}

	if (!rt_radio_on || (rt_mode_x == 1 || rt_mode_x == 3))
	{
		/* workaround for create all pseudo interfaces */
		gen_ralink_config_rt(1);
		ifconfig(WIF2G, IFUP, NULL, NULL);
	}

	doSystem("brctl addbr %s", IFNAME_BR);
	doSystem("brctl stp %s %d", IFNAME_BR, 0);
	doSystem("brctl setfd %s %d", IFNAME_BR, 2);
	doSystem("ifconfig %s hw ether %s txqueuelen %d", IFNAME_BR, lan_hwaddr, 1000);

	switch_config_vlan(1);

	if (!ap_mode)
	{
		ifconfig(IFNAME_WAN, 0, NULL, NULL);
		doSystem("brctl addif %s %s", IFNAME_BR, IFNAME_LAN);
	}
	else
	{
#ifdef USE_SINGLE_MAC
		doSystem("brctl addif %s %s", IFNAME_BR, IFNAME_MAC);
#else
		wan_mac_config();
		ifconfig(IFNAME_WAN, IFUP, NULL, NULL);
		doSystem("brctl addif %s %s", IFNAME_BR, IFNAME_LAN);
		doSystem("brctl addif %s %s", IFNAME_BR, IFNAME_WAN);
#endif
	}

	switch_config_base();
	switch_config_storm();
	switch_config_link();

	if (wl_radio_on)
	{
		start_wifi_ap_wl(1);
		start_wifi_wds_wl(1);
		start_wifi_apcli_wl(1);
	}

	if (!wl_radio_on || (wl_mode_x == 1 || wl_mode_x == 3))
	{
		/* close after workaround */
		ifconfig(WIF, 0, NULL, NULL);
	}

	if (rt_radio_on)
	{
		start_wifi_ap_rt(1);
		start_wifi_wds_rt(1);
		start_wifi_apcli_rt(1);
	}

	if (!rt_radio_on || (rt_mode_x == 1 || rt_mode_x == 3))
	{
		/* close after workaround */
		ifconfig(WIF2G, 0, NULL, NULL);
	}

	ifconfig(IFNAME_BR, IFUP, nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

	nvram_set("reload_svc_wl", "0");
	nvram_set("reload_svc_rt", "0");
}

void 
switch_config_link(void)
{
	int i_flow_mode;
	int i_link_mode;
	
	// WAN
	i_flow_mode = nvram_get_int("ether_flow_wan");
	if (i_flow_mode < 0 || i_flow_mode > 2)
		i_flow_mode = 0;
	i_link_mode = nvram_get_int("ether_link_wan");
	if (i_link_mode < 0 || i_link_mode > 5)
		i_link_mode = 0;
	
	phy_link_port_wan(i_link_mode, i_flow_mode);
	
	// LAN1
	i_flow_mode = nvram_get_int("ether_flow_lan1");
	if (i_flow_mode < 0 || i_flow_mode > 2)
		i_flow_mode = 0;
	i_link_mode = nvram_get_int("ether_link_lan1");
	if (i_link_mode < 0 || i_link_mode > 5)
		i_link_mode = 0;
	
	phy_link_port_lan1(i_link_mode, i_flow_mode);
	
	// LAN2
	i_flow_mode = nvram_get_int("ether_flow_lan2");
	if (i_flow_mode < 0 || i_flow_mode > 2)
		i_flow_mode = 0;
	i_link_mode = nvram_get_int("ether_link_lan2");
	if (i_link_mode < 0 || i_link_mode > 5)
		i_link_mode = 0;
	
	phy_link_port_lan2(i_link_mode, i_flow_mode);
	
	// LAN3
	i_flow_mode = nvram_get_int("ether_flow_lan3");
	if (i_flow_mode < 0 || i_flow_mode > 2)
		i_flow_mode = 0;
	i_link_mode = nvram_get_int("ether_link_lan3");
	if (i_link_mode < 0 || i_link_mode > 5)
		i_link_mode = 0;
	
	phy_link_port_lan3(i_link_mode, i_flow_mode);
	
	// LAN4
	i_flow_mode = nvram_get_int("ether_flow_lan4");
	if (i_flow_mode < 0 || i_flow_mode > 2)
		i_flow_mode = 0;
	i_link_mode = nvram_get_int("ether_link_lan4");
	if (i_link_mode < 0 || i_link_mode > 5)
		i_link_mode = 0;
	
	phy_link_port_lan4(i_link_mode, i_flow_mode);
}

void 
switch_config_base(void)
{
	int i_ether_led0 = nvram_get_int("ether_led0");
	int i_ether_led1 = nvram_get_int("ether_led1");
	int i_ether_jumbo = nvram_get_int("ether_jumbo");
	int i_ether_green = nvram_get_int("ether_green");

	phy_led_mode_green(i_ether_led0);
	phy_led_mode_yellow(i_ether_led1);
	phy_jumbo_frames(i_ether_jumbo);
	phy_green_ethernet(i_ether_green);
}

void 
switch_config_storm(void)
{
	int controlrate_unknown_unicast;
	int controlrate_unknown_multicast;
	int controlrate_multicast;
	int controlrate_broadcast;

	if (is_ap_mode())
		return;

	/* unknown unicast storm control */
	controlrate_unknown_unicast = nvram_get_int("controlrate_unknown_unicast");
	if (controlrate_unknown_unicast <= 0 || controlrate_unknown_unicast > 1024)
		controlrate_unknown_unicast = 1024;
	
	/* unknown multicast storm control */
	controlrate_unknown_multicast = nvram_get_int("controlrate_unknown_multicast");
	if (controlrate_unknown_multicast <= 0 || controlrate_unknown_multicast > 1024)
		controlrate_unknown_multicast = 1024;

	/* multicast storm control */
	controlrate_multicast = nvram_get_int("controlrate_multicast");
	if (controlrate_multicast <= 0 || controlrate_multicast > 1024)
		controlrate_multicast = 1024;

	/* broadcast storm control */
	controlrate_broadcast = nvram_get_int("controlrate_broadcast");
	if (controlrate_broadcast <= 0 || controlrate_broadcast > 1024)
		controlrate_broadcast = 1024;
	
	phy_storm_unicast_unknown(controlrate_unknown_unicast);
	phy_storm_multicast_unknown(controlrate_unknown_multicast);
	phy_storm_multicast(controlrate_multicast);
	phy_storm_broadcast(controlrate_broadcast);
}


void
switch_config_vlan(int first_call)
{
	int bridge_mode, bwan_isolation;
#ifndef USE_SINGLE_MAC
	int is_vlan_filter;
	int vlan_vid[5] = {0};
	int vlan_pri[5] = {0};
	int vlan_tag[5] = {0};
	int vlan_fid;
	unsigned int vlan_member, vlan_untag, accept_tagged;
#endif
	
	if (is_ap_mode())
	{
		if (first_call)
		{
#ifdef USE_SINGLE_MAC
			/* set bridge mode to LLLLL */
			phy_bridge_mode(RTL8367M_WAN_BRIDGE_DISABLE_WAN, RTL8367M_WAN_BWAN_ISOLATION_NONE);
#else
			/* accept tagged and untagged frames for all ports */
			phy_vlan_accept_port_mode(RTL8367M_VLAN_ACCEPT_FRAMES_ALL, RTL8367M_PORTMASK_WAN |
										   RTL8367M_PORTMASK_LAN1 |
										   RTL8367M_PORTMASK_LAN2 |
										   RTL8367M_PORTMASK_LAN3 |
										   RTL8367M_PORTMASK_LAN4 |
										   RTL8367M_PORTMASK_CPU_WAN |
										   RTL8367M_PORTMASK_CPU_LAN );
#endif
		}
		return;
	}
	
	if (!first_call)
	{
		/* reset all vlan entries */
		phy_vlan_reset_table();
	}
	
	bridge_mode = nvram_get_int("wan_stb_x");
	if (bridge_mode < 0 || bridge_mode > 7)
		bridge_mode = RTL8367M_WAN_BRIDGE_DISABLE;
	
	bwan_isolation = nvram_get_int("wan_stb_iso");
	if (bwan_isolation < 0 || bwan_isolation > 2)
		bwan_isolation = RTL8367M_WAN_BWAN_ISOLATION_NONE;
	
#ifndef USE_SINGLE_MAC
	is_vlan_filter = (nvram_match("vlan_filter", "1")) ? 1 : 0;
	if (is_vlan_filter)
		bwan_isolation = RTL8367M_WAN_BWAN_ISOLATION_FROM_CPU;
#endif
	
	if (bridge_mode == RTL8367M_WAN_BRIDGE_DISABLE)
		bwan_isolation = RTL8367M_WAN_BWAN_ISOLATION_NONE;
	
	phy_bridge_mode(bridge_mode, bwan_isolation);
	
#ifndef USE_SINGLE_MAC
	if (is_vlan_filter)
	{
		vlan_fid = 0;
		accept_tagged = RTL8367M_PORTMASK_WAN;
		
		vlan_vid[0] = nvram_get_int("vlan_vid_cpu");
		vlan_vid[1] = nvram_get_int("vlan_vid_lan1");
		vlan_vid[2] = nvram_get_int("vlan_vid_lan2");
		vlan_vid[3] = nvram_get_int("vlan_vid_lan3");
		vlan_vid[4] = nvram_get_int("vlan_vid_lan4");
		
		vlan_pri[0] = nvram_get_int("vlan_pri_cpu");
		vlan_pri[1] = nvram_get_int("vlan_pri_lan1");
		vlan_pri[2] = nvram_get_int("vlan_pri_lan2");
		vlan_pri[3] = nvram_get_int("vlan_pri_lan3");
		vlan_pri[4] = nvram_get_int("vlan_pri_lan4");
		
		vlan_tag[1] = nvram_get_int("vlan_tag_lan1");
		vlan_tag[2] = nvram_get_int("vlan_tag_lan2");
		vlan_tag[3] = nvram_get_int("vlan_tag_lan3");
		vlan_tag[4] = nvram_get_int("vlan_tag_lan4");
		
		vlan_pri[0] &= 0x07;
		vlan_pri[1] &= 0x07;
		vlan_pri[2] &= 0x07;
		vlan_pri[3] &= 0x07;
		vlan_pri[4] &= 0x07;
		
		if((vlan_vid[0] >= 2 && vlan_vid[0] <= 4094))
		{
			vlan_fid++;
			vlan_member = RTL8367M_PORTMASK_CPU_WAN | RTL8367M_PORTMASK_WAN;
			vlan_untag  = RTL8367M_PORTMASK_CPU_WAN;
			phy_vlan_create_entry(vlan_vid[0], vlan_pri[0], vlan_member, vlan_untag, vlan_fid);
		}
		
		switch (bridge_mode)
		{
		case RTL8367M_WAN_BRIDGE_LAN1:
			if((vlan_vid[1] >= 2 && vlan_vid[1] <= 4094) && (vlan_vid[1] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN1 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				accept_tagged |= (vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				phy_vlan_create_entry(vlan_vid[1], vlan_pri[1], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN2:
			if((vlan_vid[2] >= 2 && vlan_vid[2] <= 4094) && (vlan_vid[2] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN2 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				accept_tagged |= (vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				phy_vlan_create_entry(vlan_vid[2], vlan_pri[2], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN3:
			if((vlan_vid[3] >= 2 && vlan_vid[3] <= 4094) && (vlan_vid[3] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN3 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				accept_tagged |= (vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				phy_vlan_create_entry(vlan_vid[3], vlan_pri[3], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN4:
			if((vlan_vid[4] >= 2 && vlan_vid[4] <= 4094) && (vlan_vid[4] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN4 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
				accept_tagged |= (vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
				phy_vlan_create_entry(vlan_vid[4], vlan_pri[4], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN3_LAN4:
			if((vlan_vid[3] >= 2 && vlan_vid[3] <= 4094) && (vlan_vid[3] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN3 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				accept_tagged |= (vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				if(vlan_vid[4] == vlan_vid[3])
				{
					vlan_member |= RTL8367M_PORTMASK_LAN4;
					vlan_untag  |= (!vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
					accept_tagged |= (vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
				}
				phy_vlan_create_entry(vlan_vid[3], vlan_pri[3], vlan_member, vlan_untag, vlan_fid);
			}
			if((vlan_vid[4] >= 2 && vlan_vid[4] <= 4094) && (vlan_vid[4] != vlan_vid[0]) && (vlan_vid[4] != vlan_vid[3]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN4 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
				accept_tagged |= (vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
				phy_vlan_create_entry(vlan_vid[4], vlan_pri[4], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN1_LAN2:
			if((vlan_vid[1] >= 2 && vlan_vid[1] <= 4094) && (vlan_vid[1] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN1 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				accept_tagged |= (vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				if(vlan_vid[2] == vlan_vid[1])
				{
					vlan_member |= RTL8367M_PORTMASK_LAN2;
					vlan_untag  |= (!vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
					accept_tagged |= (vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				}
				phy_vlan_create_entry(vlan_vid[1], vlan_pri[1], vlan_member, vlan_untag, vlan_fid);
			}
			if((vlan_vid[2] >= 2 && vlan_vid[2] <= 4094) && (vlan_vid[2] != vlan_vid[0]) && (vlan_vid[2] != vlan_vid[1]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN2 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				accept_tagged |= (vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				phy_vlan_create_entry(vlan_vid[2], vlan_pri[2], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN1_LAN2_LAN3:
			if((vlan_vid[1] >= 2 && vlan_vid[1] <= 4094) && (vlan_vid[1] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN1 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				accept_tagged |= (vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				if(vlan_vid[2] == vlan_vid[1])
				{
					vlan_member |= RTL8367M_PORTMASK_LAN2;
					vlan_untag  |= (!vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
					accept_tagged |= (vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				}
				if(vlan_vid[3] == vlan_vid[1])
				{
					vlan_member |= RTL8367M_PORTMASK_LAN3;
					vlan_untag  |= (!vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
					accept_tagged |= (vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				}
				phy_vlan_create_entry(vlan_vid[1], vlan_pri[1], vlan_member, vlan_untag, vlan_fid);
			}
			if((vlan_vid[2] >= 2 && vlan_vid[2] <= 4094) && (vlan_vid[2] != vlan_vid[0]) && (vlan_vid[2] != vlan_vid[1]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN2 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				accept_tagged |= (vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				if((vlan_vid[3] == vlan_vid[2]) && (vlan_vid[3] != vlan_vid[1]))
				{
					vlan_member |= RTL8367M_PORTMASK_LAN3;
					vlan_untag  |= (!vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
					accept_tagged |= (vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				}
				phy_vlan_create_entry(vlan_vid[2], vlan_pri[2], vlan_member, vlan_untag, vlan_fid);
			}
			if((vlan_vid[3] >= 2 && vlan_vid[3] <= 4094) && (vlan_vid[3] != vlan_vid[0]) && (vlan_vid[3] != vlan_vid[1]) && (vlan_vid[3] != vlan_vid[2]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN3 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				accept_tagged |= (vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				phy_vlan_create_entry(vlan_vid[3], vlan_pri[3], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		}
		
		/* enable ingress filtering */
		phy_vlan_ingress_mode(RTL8367M_VLAN_INGRESS_FILTER_ENABLED);
		
		/* accept tagged and untagged frames for WAN port and needed LAN ports */
		phy_vlan_accept_port_mode(RTL8367M_VLAN_ACCEPT_FRAMES_ALL, accept_tagged);
	}
#endif
}

void 
reset_lan_vars(void)
{
	nvram_set("lan_hwaddr", nvram_safe_get("il0macaddr"));
}

void
start_lan(void)
{
	char *lan_ipaddr;
	char *lan_netmsk;
	char *lan_ifname = nvram_safe_get("lan_ifname");
	if (!lan_ifname[0])
	{
		lan_ifname = IFNAME_BR;
	}
	
	if (nvram_match("lan_ipaddr", ""))
	{
		nvram_set("lan_ipaddr", "192.168.1.1");
		nvram_set("lan_netmask", "255.255.255.0");
	}
	else if (nvram_match("lan_netmask", ""))
	{
		nvram_set("lan_netmask", "255.255.255.0");
	}
	
	if (nvram_invmatch("computer_name", "") && is_valid_hostname(nvram_safe_get("computer_name")))
		doSystem("hostname %s", nvram_safe_get("computer_name"));
	else
		doSystem("hostname %s", nvram_safe_get("productid"));
	
	lan_ipaddr = nvram_safe_get("lan_ipaddr");
	lan_netmsk = nvram_safe_get("lan_netmask");
	
	/* 
	* Configure DHCP connection. The DHCP client will run 
	* 'udhcpc bound'/'udhcpc deconfig' upon finishing IP address 
	* renew and release.
	*/
	if (is_ap_mode())
	{
		if (nvram_match("lan_proto_x", "1"))
		{
			/* bring up and configure LAN interface */
			ifconfig(lan_ifname, IFUP, lan_ipaddr, lan_netmsk);
			
			symlink("/sbin/rc", SCRIPT_UDHCPC_LAN);
			
			/* early fill XXX_t fields */
			update_lan_status(0);
			
			/* start dhcp daemon */
			start_udhcpc_lan(lan_ifname);
		}
		else
		{
			/* bring up and configure LAN interface */
			ifconfig(lan_ifname, IFUP, lan_ipaddr, lan_netmsk);
			
			/* manual config lan gateway and dns */
			lan_up_manual(lan_ifname);
		}
	}
	else
	{
		/* bring up and configure LAN interface */
		ifconfig(lan_ifname, IFUP, lan_ipaddr, lan_netmsk);
		
		/* install lan specific static routes */
		add_lan_routes(lan_ifname);
		
		/* fill XXX_t fields */
		update_lan_status(0);
	}

#if defined (USE_IPV6)
	if (get_ipv6_type() != IPV6_DISABLED)
		reload_lan_addr6();
#endif
}

void
stop_lan(void)
{
	char *svcs[] = { "udhcpc", "detect_wan", NULL };

	if (is_ap_mode())
	{
		kill_services(svcs, 3, 1);
	}
	else
	{
		/* Remove static routes */
		del_lan_routes(IFNAME_BR);
	}

#if defined (USE_IPV6)
	clear_lan_addr6();
#endif

	/* Bring down LAN interface */
	ifconfig(IFNAME_BR, 0, NULL, NULL);
}

void 
full_restart_lan(void)
{
	int ap_mode = is_ap_mode();
	int lan_stp = nvram_match("lan_stp", "0") ? 0 : 1;
	int log_remote = nvram_invmatch("log_ipaddr", "");

	// Stop logger if remote
	if (log_remote)
		stop_logger();

	stop_upnp();
	stop_vpn_server();
	stop_dns_dhcpd();
	stop_lan();

	reset_lan_vars();

	if (!ap_mode)
	{
		doSystem("brctl stp %s %d", IFNAME_BR, 0);
		doSystem("brctl setfd %s %d", IFNAME_BR, 2);
	}

	start_lan();

	sleep(1);

	// Start logger if remote
	if (log_remote)
	{
		start_logger(0);
	}

	if (!ap_mode)
	{
		if (lan_stp) {
			doSystem("brctl stp %s %d", IFNAME_BR, 1);
			doSystem("brctl setfd %s %d", IFNAME_BR, 15);
		}
		
		/* restart vpn server, firewall and miniupnpd */
		restart_vpn_server();
		
		/* restart dns relay and dhcp server */
		start_dns_dhcpd();
	}

	// Reload NFS server
	if (pids("nfsd"))
		run_nfsd();
}


void
lan_up_manual(char *lan_ifname)
{
	FILE *fp;
	int dns_count = 0;
	char word[100], *next, *gateway_ip;

	gateway_ip = nvram_safe_get("lan_gateway");

	/* Set default route to gateway if specified */
	if (inet_addr_(gateway_ip) != INADDR_ANY)
		route_add(lan_ifname, 0, "0.0.0.0", gateway_ip, "0.0.0.0");

	/* Open resolv.conf */
	fp = fopen("/etc/resolv.conf", "w+");
	if (fp) {
		foreach(word, nvram_safe_get("lan_dns"), next) {
			if (inet_addr_(word) != INADDR_ANY) {
				fprintf(fp, "nameserver %s\n", word);
				dns_count++;
			}
		}
		
		if (!dns_count && inet_addr_(gateway_ip) != INADDR_ANY)
			fprintf(fp, "nameserver %s\n", gateway_ip);
		
		fclose(fp);
	}

	/* sync time */
	notify_watchdog_time();

	/* fill XXX_t fields */
	update_lan_status(0);
}

void
lan_up_auto(char *lan_ifname)
{
	FILE *fp;
	int dns_count = 0;
	char word[100], *next, *gateway_ip;

	gateway_ip = nvram_safe_get("lan_gateway_t");

	/* Set default route to gateway if specified */
	if (inet_addr_(gateway_ip) != INADDR_ANY)
		route_add(lan_ifname, 0, "0.0.0.0", gateway_ip, "0.0.0.0");

	/* Open resolv.conf */
	fp = fopen("/etc/resolv.conf", "w+");
	if (fp) {
		foreach(word, nvram_safe_get("lan_dns_t"), next) {
			if (inet_addr_(word) != INADDR_ANY) {
				fprintf(fp, "nameserver %s\n", word);
				dns_count++;
			}
		}
		
		if (!dns_count && inet_addr_(gateway_ip) != INADDR_ANY)
			fprintf(fp, "nameserver %s\n", gateway_ip);
		
		fclose(fp);
	}

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	if (!pids("detect_wan"))
	{
		eval("detect_wan");
	}
#endif

	/* sync time */
	notify_watchdog_time();

	/* fill XXX_t fields */
	update_lan_status(1);
}

void
lan_down_auto(char *lan_ifname)
{
	FILE *fp;
	
	/* Remove default route to gateway if specified */
	route_del(lan_ifname, 0, "0.0.0.0", 
			nvram_safe_get("lan_gateway_t"),
			"0.0.0.0");

	/* Clear resolv.conf */
	fp = fopen("/etc/resolv.conf", "w+");
	if (fp)
		fclose(fp);
	
	/* fill XXX_t fields */
	update_lan_status(0);
}

void 
update_lan_status(int isup)
{
	char lan_ipaddr[32], lan_netmask[32], lan_subnet[32];
	
	if (!isup) {
		nvram_set("lan_ipaddr_t", nvram_safe_get("lan_ipaddr"));
		nvram_set("lan_netmask_t", nvram_safe_get("lan_netmask"));
		
		if (nvram_match("wan_route_x", "IP_Routed")) {
			if (nvram_match("dhcp_enable_x", "1")) {
				if (nvram_invmatch("dhcp_gateway_x", ""))
					nvram_set("lan_gateway_t", nvram_safe_get("dhcp_gateway_x"));
				else
					nvram_set("lan_gateway_t", nvram_safe_get("lan_ipaddr"));
			}
			else
				nvram_set("lan_gateway_t", nvram_safe_get("lan_ipaddr"));
		}
		else
			nvram_set("lan_gateway_t", nvram_safe_get("lan_gateway"));
	}
	
	strcpy(lan_ipaddr, nvram_safe_get("lan_ipaddr_t"));
	strcpy(lan_netmask, nvram_safe_get("lan_netmask_t"));
	sprintf(lan_subnet, "0x%x", inet_network(lan_ipaddr)&inet_network(lan_netmask));
	
	nvram_set("lan_subnet_t", lan_subnet);
}


static int 
udhcpc_lan_deconfig(char *lan_ifname)
{
	ifconfig(lan_ifname, IFUP,
		 nvram_safe_get("lan_ipaddr"),
		 nvram_safe_get("lan_netmask"));

	lan_down_auto(lan_ifname);

	logmessage("DHCP LAN client", "%s: lease is lost", udhcpc_lan_state);

	return 0;
}

static int 
udhcpc_lan_bound(char *lan_ifname)
{
	char *value;
	char tmp[100], prefix[16];

	snprintf(prefix, sizeof(prefix), "lan_");
	
	if ((value = getenv("ip")))
		nvram_set(strcat_r(prefix, "ipaddr_t", tmp), value);
	if ((value = getenv("subnet")))
		nvram_set(strcat_r(prefix, "netmask_t", tmp), value);
        if ((value = getenv("router")))
		nvram_set(strcat_r(prefix, "gateway_t", tmp), value);
	if ((value = getenv("dns")))
		nvram_set(strcat_r(prefix, "dns_t", tmp), value);
	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins_t", tmp), value);
#if 0
	if ((value = getenv("hostname")))
		sethostname(value, strlen(value) + 1);
#endif
	if ((value = getenv("domain")))
		nvram_set(strcat_r(prefix, "domain_t", tmp), value);
	if ((value = getenv("lease"))) {
		nvram_set(strcat_r(prefix, "lease_t", tmp), value);
	}

	ifconfig(lan_ifname, IFUP,
		 nvram_safe_get(strcat_r(prefix, "ipaddr_t", tmp)),
		 nvram_safe_get(strcat_r(prefix, "netmask_t", tmp)));

	lan_up_auto(lan_ifname);

	logmessage("DHCP LAN Client", "%s IP : %s from %s", 
		udhcpc_lan_state, 
		nvram_safe_get(strcat_r(prefix, "ipaddr_t", tmp)), 
		nvram_safe_get(strcat_r(prefix, "gateway_t", tmp)));

	return 0;
}

static int 
udhcpc_lan_renew(char *lan_ifname)
{
	return udhcpc_lan_bound(lan_ifname);
}

static int 
udhcpc_lan_leasefail(char *lan_ifname)
{
	return 0;
}

int
udhcpc_lan_main(int argc, char **argv)
{
	char *lan_ifname;

	if (argc<2 || !argv[1])
		return EINVAL;

	lan_ifname = safe_getenv("interface");
	strncpy(udhcpc_lan_state, argv[1], sizeof(udhcpc_lan_state));

	if (!strcmp(argv[1], "deconfig"))
		return udhcpc_lan_deconfig(lan_ifname);
	else if (!strcmp(argv[1], "bound"))
		return udhcpc_lan_bound(lan_ifname);
	else if (!strcmp(argv[1], "renew"))
		return udhcpc_lan_renew(lan_ifname);
	else if (!strcmp(argv[1], "leasefail"))
		return udhcpc_lan_leasefail(lan_ifname);
	else
		return udhcpc_lan_deconfig(lan_ifname);
}

int 
start_udhcpc_lan(const char *lan_ifname)
{
	char *lan_hostname;
	if (nvram_invmatch("computer_name", "") && is_valid_hostname(nvram_safe_get("computer_name")))
		lan_hostname = nvram_safe_get("computer_name");
	else
		lan_hostname = nvram_safe_get("productid");
	
	char *dhcp_argv[] = {
		"udhcpc",
		"-i", (char *)lan_ifname,
		"-s", SCRIPT_UDHCPC_LAN,
		"-p", "/var/run/udhcpc_lan.pid",
		"-t5",
		"-d", /* Background after run (new patch for udhcpc) */
		"-H", lan_hostname,
		NULL
	};
	
	logmessage("DHCP LAN client", "starting dhcp client (%s) ...", lan_ifname);
	
	return _eval(dhcp_argv, NULL, 0, NULL);
}

int 
stop_udhcpc_lan()
{
	return kill_pidfile_s("/var/run/udhcpc_lan.pid", SIGTERM);
}
