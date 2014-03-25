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
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <signal.h>

#include <nvram/bcmnvram.h>

#include "rc.h"
#include "switch.h"

static char udhcp_state[16] = {0};

static void
set_wan0_param(const char* param_name)
{
	char wan_param[64], wan0_param[64];

	snprintf(wan_param, sizeof(wan_param), "wan_%s", param_name);
	snprintf(wan0_param, sizeof(wan0_param), "wan%d_%s", 0, param_name);

	nvram_set_temp(wan0_param, nvram_safe_get(wan_param));
}

static void
set_wan0_value(const char* param_name, const char* value)
{
	char wan0_param[64];

	snprintf(wan0_param, sizeof(wan0_param), "wan%d_%s", 0, param_name);
	nvram_set_temp(wan0_param, value);
}

static void
wan_netmask_check(void)
{
	char *wan_proto;
	unsigned int ip, gw, nm, lip, lnm;

	wan_proto = nvram_safe_get("wan0_proto");

	if ((strcmp(wan_proto, "static") == 0) ||
	    (strcmp(wan_proto, "pppoe") == 0 && nvram_match("pppoe_dhcp_route", "1")) ||
	    (strcmp(wan_proto, "pptp") == 0) ||
	    (strcmp(wan_proto, "l2tp") == 0))
	{
		ip = inet_addr(nvram_safe_get("wan0_ipaddr"));
		gw = inet_addr(nvram_safe_get("wan0_gateway"));
		nm = inet_addr(nvram_safe_get("wan0_netmask"));
		
		lip = inet_addr(nvram_safe_get("lan_ipaddr"));
		lnm = inet_addr(nvram_safe_get("lan_netmask"));
		
		if (ip==0x0 && strcmp(wan_proto, "static") != 0)
			return;
		
		if (ip==0x0 || ip==0xffffffff || (ip&lnm)==(lip&lnm))
		{
			set_wan0_value("ipaddr", "1.1.1.1");
			set_wan0_value("netmask", "255.0.0.0");
		}
		
		// check netmask here
		if (gw!=0 && gw!=0xffffffff && (ip&nm)!=(gw&nm))
		{
			for (nm=0xffffffff;nm!=0;nm=(nm>>8))
			{
				if ((ip&nm)==(gw&nm)) break;
			}
			
			if (nm==0xffffffff) set_wan0_value("netmask", "255.255.255.255");
			else if (nm==0xffffff) set_wan0_value("netmask", "255.255.255.0");
			else if (nm==0xffff) set_wan0_value("netmask", "255.255.0.0");
			else if (nm==0xff) set_wan0_value("netmask", "255.0.0.0");
			else set_wan0_value("netmask", "0.0.0.0");
		}
		
		nvram_set_temp("wanx_ipaddr", nvram_safe_get("wan0_ipaddr"));	// oleg patch, he suggests to mark the following 3 lines
		nvram_set_temp("wanx_netmask", nvram_safe_get("wan0_netmask"));
		nvram_set_temp("wanx_gateway", nvram_safe_get("wan0_gateway"));
	}
}

void 
reset_wan_temp(void)
{
	nvram_set_temp("wan_status_t", "Disconnected");
	nvram_set_temp("wan_ipaddr_t", "");
	nvram_set_temp("wan_netmask_t", "");
	nvram_set_temp("wan_gateway_t", "");
	nvram_set_temp("wan_subnet_t", "");
}

void 
reset_man_vars(void)
{
	nvram_set_temp("wanx_ipaddr", "");
	nvram_set_temp("wanx_netmask", "");
	nvram_set_temp("wanx_gateway", "");
	nvram_set_temp("wanx_dns", "");
	nvram_set_temp("wanx_lease", "");
	nvram_set_temp("wanx_routes", "");
	nvram_set_temp("wanx_routes_ms", "");
	nvram_set_temp("wanx_routes_rfc", "");

	nvram_set_temp("manv_ipaddr", "");
	nvram_set_temp("manv_netmask", "");
	nvram_set_temp("manv_gateway", "");
	nvram_set_temp("manv_routes", "");
	nvram_set_temp("manv_routes_ms", "");
	nvram_set_temp("manv_routes_rfc", "");
}

void 
reset_wan_vars(int full_reset)
{
	char macbuf[36];

	if (full_reset)
	{
		nvram_set_temp("wan_ifname_t", "");
	}

	nvram_set_int_temp("l2tp_wan_t", 0);

	reset_wan_temp();
	reset_man_vars();

	set_wan0_param("proto");
	set_wan0_value("time", "0");
	set_wan0_value("dns", "");
	set_wan0_value("wins", "");
	set_wan0_value("lease", "");
	set_wan0_value("routes", "");
	set_wan0_value("routes_ms", "");
	set_wan0_value("routes_rfc", "");

	if (nvram_match("x_DHCPClient", "0") || nvram_match("wan_proto", "static"))
	{
		set_wan0_param("ipaddr");
		set_wan0_param("netmask");
		set_wan0_param("gateway");
	}
	else
	{
		set_wan0_value("ipaddr", "0.0.0.0");
		set_wan0_value("netmask", "0.0.0.0");
		set_wan0_value("gateway", "0.0.0.0");
	}

	wan_netmask_check();

	set_wan0_param("dnsenable_x");
	set_wan0_param("hostname");

	if (nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "pptp") || nvram_match("wan_proto", "l2tp"))
	{
		set_wan0_value("pppoe_ifname", IFNAME_PPP);
		
		set_wan0_param("pppoe_username");
		set_wan0_param("pppoe_passwd");
		
		if (nvram_match("wan_proto", "pppoe"))
			set_wan0_param("pppoe_idletime");
		else
			set_wan0_value("pppoe_idletime", "0");
		
		set_wan0_param("pppoe_txonly_x");
		set_wan0_param("pppoe_service");
		set_wan0_param("pppoe_ac");
		set_wan0_param("pppoe_mtu");
		set_wan0_param("pppoe_mru");
		set_wan0_param("pptp_mtu");
		set_wan0_param("pptp_mru");
		set_wan0_param("l2tp_mtu");
		set_wan0_param("l2tp_mru");
		
		set_wan0_param("ppp_peer");
		set_wan0_param("ppp_auth");
		set_wan0_param("ppp_mppe");
		set_wan0_param("ppp_alcp");
		set_wan0_param("ppp_pppd");
		
		set_wan0_value("pppoe_ipaddr", nvram_safe_get("wan0_ipaddr"));
		set_wan0_value("pppoe_netmask", (inet_addr_(nvram_safe_get("wan0_ipaddr")) && inet_addr_(nvram_safe_get("wan0_netmask"))) ? nvram_safe_get("wan0_netmask") : NULL);
		set_wan0_value("pppoe_gateway", nvram_safe_get("wan0_gateway"));
		
		nvram_set_temp("wanx_ipaddr", nvram_safe_get("wan0_ipaddr"));
	}

	mac_conv("wan_hwaddr_x", -1, macbuf);
	if (!nvram_match("wan_hwaddr_x", "") && strcasecmp(macbuf, "FF:FF:FF:FF:FF:FF"))
	{
		nvram_set("wan_hwaddr", macbuf);
		set_wan0_value("hwaddr", macbuf);
	}
	else
	{
		nvram_set("wan_hwaddr", nvram_safe_get("il1macaddr"));
		set_wan0_value("hwaddr", nvram_safe_get("il1macaddr"));
	}

#if defined (USE_IPV6)
	reset_wan6_vars();
#endif
}

void
set_man_ifname(char *man_ifname, int unit)
{
	char prefix[16], tmp[32];
	
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	nvram_set_temp(strcat_r(prefix, "ifname", tmp), man_ifname);
}

char*
get_man_ifname(int unit)
{
	char prefix[16], tmp[32];

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	return nvram_safe_get(strcat_r(prefix, "ifname", tmp));
}

int
get_vlan_vid_wan(void)
{
	int vlan_vid;
	int is_vlan_filter;

	is_vlan_filter = (nvram_match("vlan_filter", "1")) ? 1 : 0;
	if (is_vlan_filter)
		vlan_vid = nvram_get_int("vlan_vid_cpu");
	else
		vlan_vid = 0;

	if (!is_vlan_vid_inet_valid(vlan_vid))
		vlan_vid = 2;

	return vlan_vid;
}

void
remove_vlan_iface(char *vlan_ifname)
{
	doSystem("ifconfig %s %s", vlan_ifname, "down");
	doSystem("vconfig rem %s", vlan_ifname);
}

static void
update_hw_vlan_tx(int inet_vid, int iptv_vid)
{
	int vidx14, vidx15;
	char vlan_tx_info[32];

#if !defined (USE_SINGLE_MAC)
	// 2xRGMII mode used untag for Internet source, no needed tagged output
	inet_vid = 0;
#endif
	/* send VLAN VID for IDX 14/15 to raeth (for support RT3883/MT7620 HW_VLAN_TX with VID > 15) */
	vidx14 = (inet_vid > 13) ? inet_vid : 14;
	vidx15 = (iptv_vid > 13 && iptv_vid != vidx14) ? iptv_vid : 15;
	snprintf(vlan_tx_info, sizeof(vlan_tx_info), "%d %d", vidx14, vidx15);

#if defined (CONFIG_RALINK_RT3883)
	fput_string("/proc/rt3883/vlan_tx", vlan_tx_info);
#elif defined (CONFIG_RALINK_MT7620)
	fput_string("/proc/mt7620/vlan_tx", vlan_tx_info);
#endif
}

static void
config_vinet_wan(void)
{
#ifdef USE_SINGLE_MAC
	int vlan_vid, vlan_pri;
	int is_vlan_filter;
	char *vinet_iflast;
	char vinet_ifname[32];

	is_vlan_filter = (nvram_match("vlan_filter", "1")) ? 1 : 0;
	if (is_vlan_filter)
	{
		vlan_vid = nvram_get_int("vlan_vid_cpu");
		vlan_pri = nvram_get_int("vlan_pri_cpu") & 0x07;
	}
	else
	{
		vlan_vid = 0;
		vlan_pri = 0;
	}

	if (!is_vlan_vid_inet_valid(vlan_vid))
	{
		vlan_vid = 2;
		vlan_pri = 0;
	}

	vinet_iflast = get_man_ifname(0);
	snprintf(vinet_ifname, sizeof(vinet_ifname), "%s.%d", IFNAME_MAC, vlan_vid);

	if (*vinet_iflast && strcmp(vinet_iflast, vinet_ifname) &&
	                     strcmp(vinet_iflast, IFNAME_WAN) && is_interface_exist(vinet_iflast))
		remove_vlan_iface(vinet_iflast);

	if (!is_interface_exist(vinet_ifname))
		doSystem("vconfig add %s %d", IFNAME_MAC, vlan_vid);

	doSystem("vconfig set_egress_map %s %d %d", vinet_ifname, 0, vlan_pri);
	doSystem("ifconfig %s hw ether %s", vinet_ifname, nvram_safe_get("wan_hwaddr"));
	doSystem("ifconfig %s up %s", vinet_ifname, "0.0.0.0");

#if defined (USE_IPV6)
	if (get_ipv6_type() != IPV6_DISABLED)
		control_if_ipv6(vinet_ifname, 1);
#endif
	set_man_ifname(vinet_ifname, 0);
#else
	doSystem("ifconfig %s hw ether %s", IFNAME_MAC2, nvram_safe_get("wan_hwaddr"));
	doSystem("ifconfig %s up %s", IFNAME_MAC2, "0.0.0.0");
#endif
}

static void
launch_viptv_wan(void)
{
	int vlan_vid[2];
	int vlan_pri;
	int is_vlan_filter;
	char *viptv_iflast, *vinet_iflast;
	char viptv_ifname[32], rp_path[64];

	is_vlan_filter = (nvram_match("vlan_filter", "1")) ? 1 : 0;
	if (is_vlan_filter)
	{
		vlan_vid[0] = nvram_get_int("vlan_vid_cpu");
		vlan_vid[1] = nvram_get_int("vlan_vid_iptv");
		vlan_pri = nvram_get_int("vlan_pri_iptv") & 0x07;
	}
	else
	{
		vlan_vid[0] = 0;
		vlan_vid[1] = 0;
		vlan_pri = 0;
	}

	vinet_iflast = get_man_ifname(0);
	viptv_iflast = nvram_safe_get("viptv_ifname");

	if (is_vlan_vid_iptv_valid(vlan_vid[0], vlan_vid[1]))
	{
		/* update VLAN for raeth */
		update_hw_vlan_tx(vlan_vid[0], vlan_vid[1]);
		
		/* create VLAN for IPTV */
#ifdef USE_SINGLE_MAC
		snprintf(viptv_ifname, sizeof(viptv_ifname), "%s.%d", IFNAME_MAC,  vlan_vid[1]);
#else
		snprintf(viptv_ifname, sizeof(viptv_ifname), "%s.%d", IFNAME_MAC2, vlan_vid[1]);
#endif
		if (*viptv_iflast && strcmp(viptv_iflast, vinet_iflast) && strcmp(viptv_iflast, IFNAME_WAN) &&
		                     strcmp(viptv_iflast, viptv_ifname) && is_interface_exist(viptv_iflast))
			remove_vlan_iface(viptv_iflast);
		
		if (!is_interface_exist(viptv_ifname))
		{
#ifdef USE_SINGLE_MAC
			doSystem("vconfig add %s %d", IFNAME_MAC,  vlan_vid[1]);
#else
			doSystem("vconfig add %s %d", IFNAME_MAC2, vlan_vid[1]);
#endif
		}
		
		doSystem("vconfig set_egress_map %s %d %d", viptv_ifname, 0, vlan_pri);
		doSystem("ifconfig %s hw ether %s", viptv_ifname, nvram_safe_get("wan_hwaddr"));
		doSystem("ifconfig %s up %s", viptv_ifname, "0.0.0.0");
		
		/* disable rp_filter */
		sprintf(rp_path, "/proc/sys/net/ipv4/conf/%s/rp_filter", viptv_ifname);
		fput_int(rp_path, 0);
		
		nvram_set_temp("viptv_ifname", viptv_ifname);
		
		start_udhcpc_viptv(viptv_ifname);
	}
	else
	{
		/* update VLAN for raeth */
		update_hw_vlan_tx(vlan_vid[0], 0);
		
		if (is_vlan_vid_inet_valid(vlan_vid[0]) && vlan_vid[0] != vlan_vid[1])
		{
			/* case: CPU Internet tagged, CPU IPTV not tagged */
#ifdef USE_SINGLE_MAC
			snprintf(viptv_ifname, sizeof(viptv_ifname), "%s", IFNAME_WAN); // eth2.2
#else
			snprintf(viptv_ifname, sizeof(viptv_ifname), "%s.%d", IFNAME_MAC2, 2);  // eth3.2
			if (!is_interface_exist(viptv_ifname))
				doSystem("vconfig add %s %d", IFNAME_MAC2, 2);
#endif
			if (*viptv_iflast && strcmp(viptv_iflast, vinet_iflast) && strcmp(viptv_iflast, IFNAME_WAN) &&
			                     strcmp(viptv_iflast, viptv_ifname) && is_interface_exist(viptv_iflast))
				remove_vlan_iface(viptv_iflast);
			
			doSystem("ifconfig %s hw ether %s", viptv_ifname, nvram_safe_get("wan_hwaddr"));
			doSystem("ifconfig %s up %s", viptv_ifname, "0.0.0.0");
			
			/* disable rp_filter */
			sprintf(rp_path, "/proc/sys/net/ipv4/conf/%s/rp_filter", viptv_ifname);
			fput_int(rp_path, 0);
			
			nvram_set_temp("viptv_ifname", viptv_ifname);
			
			start_udhcpc_viptv(viptv_ifname);
		}
		else
		{
			/* case1: CPU Internet not tagged, CPU IPTV not tagged.
			   case2: CPU Internet tagged, CPU IPTV tagged with common VLAN ID. */
			
			if (*viptv_iflast && strcmp(viptv_iflast, vinet_iflast) && strcmp(viptv_iflast, IFNAME_WAN) &&
			                    is_interface_exist(viptv_iflast))
				remove_vlan_iface(viptv_iflast);
			
			viptv_ifname[0] = 0;
			nvram_set_temp("viptv_ifname", viptv_ifname);
		}
	}
}

static void 
launch_wanx(char *wan_ifname, char *ppp_ifname, char *prefix, int unit, int wait_dhcpc, int use_zcip)
{
	char tmp[100];
	
	char *ip_addr = nvram_safe_get(strcat_r(prefix, "pppoe_ipaddr", tmp));
	char *netmask = nvram_safe_get(strcat_r(prefix, "pppoe_netmask", tmp));
	char *gateway = nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp));
	
	if (!(*netmask))
		netmask = NULL;
	
	/* Bring up physical WAN interface */
	ifconfig(wan_ifname, IFUP, ip_addr, netmask);
	
	if (use_zcip || inet_addr_(ip_addr) == INADDR_ANY)
	{
		/* PPPoE connection not needed WAN physical address first, skip wait DHCP lease */
		/* PPTP and L2TP needed WAN physical first for create VPN tunnel, wait DHCP lease */
		/* Start dhcpc daemon */
		if (!use_zcip)
		{
			start_udhcpc_wan(wan_ifname, unit, wait_dhcpc);
			
			/* add delay 2s after eth3 up: gethostbyname issue (L2TP/PPTP) */
			if (wait_dhcpc)
				sleep(2);
		}
		else
			start_zcip_wan(wan_ifname);
	}
	else
	{
		/* start firewall */
		start_firewall_ex(ppp_ifname, "0.0.0.0");
		
		/* setup static wan routes via physical device */
		add_static_man_routes(wan_ifname);
		
		/* and set default route if specified with metric 1 */
		if ( inet_addr_(gateway) != INADDR_ANY )
		{
			route_add(wan_ifname, 2, "0.0.0.0", gateway, "0.0.0.0");
		}
		
		/* start multicast router */
		start_igmpproxy(wan_ifname);
	}
	
#if defined (USE_IPV6)
	if (is_wan_ipv6_type_sit() == 0 && !is_wan_ipv6_if_ppp())
		wan6_up(wan_ifname);
#endif
}

static int 
wait_ppp_up(char *prefix, char *ppp_ifname)
{
	int timeout, sockfd;
	struct ifreq ifr;
	char tmp[100], *ppp_gw;

	if (!(*ppp_ifname))
		return 0;

	/* Wait for pppX to be created */
	timeout = 5;
	while (ifconfig(ppp_ifname, IFUP, NULL, NULL) && timeout--)
		sleep(1);

	/* Retrieve IP info */
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return 0;

	/* Set temporary IP address */
	ppp_gw = "";
	strncpy(ifr.ifr_name, ppp_ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFADDR, &ifr) == 0)
	{
		nvram_set_temp(strcat_r(prefix, "ipaddr", tmp), inet_ntoa(sin_addr(&ifr.ifr_addr)));
		nvram_set_temp(strcat_r(prefix, "netmask", tmp), "255.255.255.255");
		
		/* Set temporary P-t-P address */
		if (ioctl(sockfd, SIOCGIFDSTADDR, &ifr) == 0)
		{
			ppp_gw = inet_ntoa(sin_addr(&ifr.ifr_dstaddr));
			nvram_set_temp(strcat_r(prefix, "gateway", tmp), ppp_gw);
		}
	}
	close(sockfd);

	/*
	* Preset routes so that traffic can be sent to proper pppx even before 
	* the link is brought up.
	*/
	preset_wan_routes(ppp_ifname);

	/* Stimulate link up */
	if (*ppp_gw)
		doSystem("ping -c1 %s", ppp_gw);

	return 1;
}

static void
launch_wan_usbnet(int unit)
{
	int modem_devnum = 0;
	char ndis_ifname[16] = {0};
	
	if (get_modem_ndis_ifname(ndis_ifname, &modem_devnum) && is_interface_exist(ndis_ifname)) {
		int ndis_mtu = nvram_safe_get_int("modem_mtu", 1500, 1000, 1500);
		doSystem("ifconfig %s mtu %d up %s", ndis_ifname, ndis_mtu, "0.0.0.0");
		connect_ndis(modem_devnum);
		start_udhcpc_wan(ndis_ifname, unit, 0);
		nvram_set_temp("wan_ifname_t", ndis_ifname);
	}
	else
		nvram_set_temp("wan_ifname_t", "");
}

static void
stop_wan_usbnet(void)
{
	int modem_devnum = 0;
	char ndis_ifname[16] = {0};
	
	if (get_modem_ndis_ifname(ndis_ifname, &modem_devnum)) {
		disconnect_ndis(modem_devnum);
		if (is_interface_exist(ndis_ifname))
			ifconfig(ndis_ifname, 0, "0.0.0.0", NULL);
	}
}

static void
create_cb_links(void)
{
	mkdir("/tmp/ppp", 0777);
	mkdir(PPP_PEERS_DIR, 0777);
	
	symlink("/sbin/rc", "/tmp/ppp/ip-up");
	symlink("/sbin/rc", "/tmp/ppp/ip-down");
	symlink("/sbin/rc", SCRIPT_UDHCPC_WAN);
	symlink("/sbin/rc", SCRIPT_UDHCPC_VIPTV);
	symlink("/sbin/rc", SCRIPT_ZCIP_WAN);
	symlink("/sbin/rc", SCRIPT_ZCIP_VIPTV);
	symlink("/sbin/rc", SCRIPT_WPACLI_WAN);
#if defined (USE_IPV6)
	symlink("/sbin/rc", "/tmp/ppp/ipv6-up");
	symlink("/sbin/rc", "/tmp/ppp/ipv6-down");
	symlink("/sbin/rc", SCRIPT_DHCP6C_WAN);
#endif
}

static void
remove_cb_links(void)
{
	/* Remove dynamically created links */
	unlink(SCRIPT_ZCIP_WAN);
	unlink(SCRIPT_ZCIP_VIPTV);
	unlink(SCRIPT_UDHCPC_WAN);
	unlink(SCRIPT_UDHCPC_VIPTV);
	unlink(SCRIPT_WPACLI_WAN);
	unlink("/tmp/ppp/ip-up");
	unlink("/tmp/ppp/ip-down");
#if defined (USE_IPV6)
	unlink(SCRIPT_DHCP6C_WAN);
	unlink("/tmp/ppp/ipv6-up");
	unlink("/tmp/ppp/ipv6-down");
#endif
}

void
start_wan(void)
{
	int unit, is_pppoe, wan_auth_mode;
	char *wan_ifname, *ppp_ifname, *wan_proto;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	
	/* check if we need to setup WAN */
	if (get_ap_mode())
		return;
	
	/* Create links */
	create_cb_links();
	
	reload_nat_modules();
	
	config_vinet_wan();
	launch_viptv_wan();
	
	update_wan_status(0);
	
	update_resolvconf(1, 0);
	
	smart_restart_upnp();
	
	wan_auth_mode = nvram_get_int("wan_auth_mode");
	
	/* Start each configured and enabled wan connection and its undelying i/f */
	for (unit = 0; unit < 2; unit ++)
	{
		if (unit > 0 && !nvram_match("wan_proto", "pppoe"))
			break;
		
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		
		/* make sure the connection exists and is enabled */ 
		wan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
		if (!wan_ifname)
			continue;
		
		wan_proto = nvram_get(strcat_r(prefix, "proto", tmp));
		if (!wan_proto || !strcmp(wan_proto, "disabled"))
			continue;
		
		is_pppoe = !strcmp(wan_proto, "pppoe");
		
		/* Bring up if */
		ifconfig(wan_ifname, IFUP, NULL, NULL);
		
		dbg("%s: wan_ifname=%s, wan_proto=%s\n", __FUNCTION__, wan_ifname, wan_proto);
		
		if (unit == 0)
		{
			set_ipv4_forward();
			set_pppoe_passthrough();
		}
		
		/* 
		* Configure PPPoE connection. The PPPoE client will run 
		* ip-up/ip-down scripts upon link's connect/disconnect.
		*/
		
		if (get_usb_modem_wan(unit))
		{
			if (nvram_match("modem_type", "3"))
			{
				launch_wan_usbnet(unit);
			}
			else
			{
				ppp_ifname = IFNAME_RAS;
				
				if (is_wan_ppp(wan_proto))
				{
					int i_pppoe_man = nvram_get_int("pppoe_dhcp_route");
					if (!is_pppoe || i_pppoe_man == 1)
						launch_wanx(wan_ifname, ppp_ifname, prefix, unit, 0, 0);
					else if (is_pppoe && i_pppoe_man == 2)
						launch_wanx(wan_ifname, ppp_ifname, prefix, unit, 0, 1);
				}
				else
				{
					/* start firewall */
					start_firewall_ex(ppp_ifname, "0.0.0.0");
				}
				
				launch_modem_ras_pppd(unit);
				
				nvram_set_temp("wan_ifname_t", ppp_ifname);
			}
		}
		else
		if (is_wan_ppp(wan_proto))
		{
			int demand;
			int i_pppoe_man = nvram_get_int("pppoe_dhcp_route");
			
			ppp_ifname = IFNAME_PPP;
			
			if (!is_pppoe || i_pppoe_man == 1)
				launch_wanx(wan_ifname, ppp_ifname, prefix, unit, !is_pppoe, 0);
			else if (is_pppoe && i_pppoe_man == 2)
				launch_wanx(wan_ifname, ppp_ifname, prefix, unit, 0, 1);
			
			demand = nvram_get_int(strcat_r(prefix, "pppoe_idletime", tmp));
			if (!is_pppoe || demand < 0)
				demand = 0;
			
			/* update demand option */
			nvram_set_int(strcat_r(prefix, "pppoe_demand", tmp), (demand) ? 1 : 0);
			
			/* launch pppoe client daemon */
			start_pppd(prefix);
			
			/* Pretend that the WAN interface is up */
			if (demand)
			{
				if (!wait_ppp_up(prefix, ppp_ifname))
					continue;
			}
			
			nvram_set_temp("wan_ifname_t", ppp_ifname);
		}
		
		/* 
		* Configure DHCP connection. The DHCP client will run 
		* 'udhcpc bound'/'udhcpc deconfig' upon finishing IP address 
		* renew and release.
		*/
		else if (strcmp(wan_proto, "dhcp") == 0)
		{
			/* Start eapol authenticator */
			if (wan_auth_mode > 1)
				start_auth_eapol(wan_ifname, wan_auth_mode - 2);
			
			/* Start dhcp daemon */
			start_udhcpc_wan(wan_ifname, unit, 0);
			nvram_set_temp("wan_ifname_t", wan_ifname);
#if defined (USE_IPV6)
			if (is_wan_ipv6_type_sit() == 0)
				wan6_up(wan_ifname);
#endif
		}
		/* Configure static IP connection. */
		else if ((strcmp(wan_proto, "static") == 0)) 
		{
			/* Assign static IP address to i/f */
			ifconfig(wan_ifname, IFUP,
				 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
				 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
			
			/* Start eapol-md5 authenticator */
			if (wan_auth_mode > 1)
				start_auth_eapol(wan_ifname, wan_auth_mode - 2);
			
			/* We are done configuration */
			wan_up(wan_ifname);
			nvram_set_temp("wan_ifname_t", wan_ifname);
#if defined (USE_IPV6)
			if (is_wan_ipv6_type_sit() == 0)
				wan6_up(wan_ifname);
#endif
		}
	}
}

void 
stop_wan_ppp()
{
	// stop services only for ppp0 interface
	char* svcs[] = { "l2tpd", 
	                 "xl2tpd", 
	                 "pppd", 
	                  NULL };
	
	stop_vpn_client();
	
	kill_services(svcs, 6, 1);
	
	set_wan0_value("time", "0");
	nvram_set_int_temp("l2tp_wan_t", 0);
	nvram_set_temp("wan_status_t", "Disconnected");
}

void
stop_wan(void)
{
	char *man_ifname = get_man_ifname(0);
	char *svcs[] = { "ntpd", 
	                 "inadyn", 
	                 "igmpproxy", 
	                 "udpxy", 
	                 "ip-up",
	                 "ip-down",
	                 "udhcpc",
	                 "zcip",
	                 "pppoe-relay",
#if defined (USE_IPV6)
	                 "ipv6-up",
	                 "ipv6-down",
#endif
	                 "l2tpd",
	                 "xl2tpd",
	                 "pppd",
	                 "detect_wan",
	                  NULL };
	
#if defined (USE_IPV6)
	if (is_wan_ipv6_type_sit() == 0)
		wan6_down(man_ifname);
#endif
	if (pids("udhcpc"))
	{
		logmessage("stop_wan()", "raise DHCP release event");
		doSystem("killall %s %s", "-SIGUSR2", "udhcpc");
		usleep(250000);
	}
	
	stop_vpn_client();
	stop_auth_eapol();
	stop_auth_kabinet();
	disable_all_passthrough();
	
	kill_services(svcs, 6, 1);
	
	if (!is_physical_wan_dhcp() && nvram_match("wan_ifname_t", man_ifname))
		wan_down(man_ifname);
	
	/* Bring down WAN interfaces */
	ifconfig(man_ifname, 0, "0.0.0.0", NULL);
	
	/* Bring down usbnet interface */
	stop_wan_usbnet();
	
	/* Remove dynamically created links */
	remove_cb_links();
	
	flush_conntrack_caches();
	
	set_wan0_value("time", "0");
	nvram_set_int_temp("l2tp_wan_t", 0);
	
	update_wan_status(0);
}

void
wan_up(char *wan_ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_proto, *gateway;
	char *script_postw = SCRIPT_POST_WAN;
	int  is_modem_unit;
	
	logmessage(LOGNAME, "wan up (%s)", wan_ifname);
	
	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0) {
		int is_modem_active;
		
		/* called for dhcp+ppp */
		if (!nvram_match("wan0_ifname", wan_ifname))
			return;
		
		/* re-start firewall */
		notify_rc("restart_firewall_wan");
		
		is_modem_active = get_usb_modem_wan(0);
		
		/* setup static wan routes via physical device */
		add_static_man_routes(wan_ifname);
		
		/* and one supplied via DHCP */
		add_dhcp_routes_by_prefix("wanx_", wan_ifname, 0);
		
		gateway = nvram_safe_get("wanx_gateway");
		
		/* and default route with metric 1 */
		if (inet_addr_(gateway) != INADDR_ANY) {
			char word[100], *next;
			in_addr_t addr = inet_addr(nvram_safe_get("wanx_ipaddr"));
			in_addr_t mask = inet_addr(nvram_safe_get("wanx_netmask"));
			
			/* if the gateway is out of the local network */
			if ((inet_addr(gateway) & mask) != (addr & mask))
				route_add(wan_ifname, 2, gateway, NULL, "255.255.255.255");
			
			/* default route via default gateway */
			route_add(wan_ifname, 2, "0.0.0.0", gateway, "0.0.0.0");
			
			/* ... and to dns servers as well for demand ppp to work */
			if (nvram_match("wan_dnsenable_x", "1") && nvram_invmatch("wan_proto", "pppoe") && !is_modem_active) {
				foreach(word, nvram_safe_get("wanx_dns"), next) {
					if ((inet_addr(word) != inet_addr(gateway)) && (inet_addr(word) & mask) != (addr & mask))
						route_add(wan_ifname, 2, word, gateway, "255.255.255.255");
				}
			}
		}
		
		/* start multicast router */
		start_igmpproxy(wan_ifname);
		
		update_resolvconf(0, 0);
		
		return;
	}
	
	/* re-start firewall */
	notify_rc("restart_firewall_wan");
	
	is_modem_unit = is_ifunit_modem(wan_ifname);
	
	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
	
	/* Set default route to gateway if specified */
	if (nvram_match(strcat_r(prefix, "primary", tmp), "1")) {
		gateway = nvram_safe_get(strcat_r(prefix, "gateway", tmp));
		
		if ( (!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_proto, "static") == 0) ) {
			/* the gateway is in the local network */
			route_add(wan_ifname, 0, gateway, NULL, "255.255.255.255");
		}
		
		/* default route via default gateway */
		route_add(wan_ifname, 0, "0.0.0.0", gateway, "0.0.0.0");
		
		/* hack: avoid routing cycles, when both peer and server has the same IP */
		if ( (!is_modem_unit) && (strcmp(wan_proto, "pptp") == 0 || strcmp(wan_proto, "l2tp") == 0)) {
			/* delete gateway route as it's no longer needed */
			route_del(wan_ifname, 0, gateway, "0.0.0.0", "255.255.255.255");
		}
	}
	
	/* Install interface dependent static routes */
	add_static_wan_routes(wan_ifname);
	
	/* Add static MAN routes for IPoE */
	if ( (!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_proto, "static") == 0) ) {
		nvram_set_temp("wanx_gateway", nvram_safe_get(strcat_r(prefix, "gateway", tmp)));
		add_static_man_routes(wan_ifname);
	}
	
	/* Add dynamic routes supplied via DHCP */
	if ( ((!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0)) || (is_modem_unit == 2) )
		add_dhcp_routes_by_prefix(prefix, wan_ifname, 0);
	
#if defined (USE_IPV6)
	if (is_wan_ipv6_type_sit() == 1)
		wan6_up(wan_ifname);
#endif
	
	/* Add dns servers to resolv.conf */
	update_resolvconf(0, 0);
	
	/* Start kabinet authenticator */
	if ( (!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_proto, "static") == 0) ) {
		if (nvram_match("wan_auth_mode", "1"))
			start_auth_kabinet();
	}
	
	/* Sync time */
	update_wan_status(1);
	
	/* start multicast router */
	if ( ((!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_proto, "static") == 0)) || (is_modem_unit == 2) )
		start_igmpproxy(wan_ifname);
	
	update_ddns();
	
	notify_watchdog_time();
	
	if ( (!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0) ) {
		if (nvram_match("gw_arp_ping", "1") && !pids("detect_wan"))
			eval("detect_wan");
	}
	
	notify_rc("start_vpn_client");
	
	if (check_if_file_exist(script_postw))
		doSystem("%s %s %s", script_postw, "up", wan_ifname);
}

void
wan_down(char *wan_ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_proto;
	char *script_postw = SCRIPT_POST_WAN;
	int  is_modem_unit;
	
	logmessage(LOGNAME, "wan down (%s)", wan_ifname);
	
	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
	{
		// dhcp + ppp (wan_ifname=eth3/eth2.2)
		/* stop multicast router */
		stop_igmpproxy(wan_ifname);
		
		return;
	}
	
	notify_rc("stop_vpn_client");
	
	is_modem_unit = is_ifunit_modem(wan_ifname);
	
#if defined (USE_IPV6)
	if (is_wan_ipv6_type_sit() == 1)
		wan6_down(wan_ifname);
#endif
	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
	
	if ( (!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_proto, "static") == 0) )
	{
		/* Stop multicast router */
		stop_igmpproxy(wan_ifname);
		
		/* Stop kabinet authenticator */
		if (nvram_match("wan_auth_mode", "1"))
			stop_auth_kabinet();
	}
	
	/* Remove default route to gateway if specified */
	if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
		route_del(wan_ifname, 0, "0.0.0.0", 
			nvram_safe_get(strcat_r(prefix, "gateway", tmp)),
			"0.0.0.0");
	
	/* Remove interface dependent static routes */
	del_static_wan_routes(wan_ifname);
	
	/* Update resolv.conf -- leave as is if no dns servers left for demand to work */
	if (*nvram_safe_get("wanx_dns"))	// oleg patch
		nvram_set_temp(strcat_r(prefix, "dns", tmp), "");
	
	update_resolvconf(0, 0);
	
	if ( (!is_modem_unit) && (strcmp(wan_proto, "static")==0) )
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
	
	update_wan_status(0);
	
	// cleanup
	nvram_set_temp("wan_ipaddr_t", "");
	
	// flush conntrack caches
	flush_conntrack_caches();
	
	if (check_if_file_exist(script_postw))
		doSystem("%s %s %s", script_postw, "down", wan_ifname);
}

void 
full_restart_wan(void)
{
	if (get_ap_mode())
		return;

	stop_wan();

	clear_if_route4(get_man_ifname(0));
	clear_if_route4(IFNAME_BR);
	flush_route_caches();

	phy_clear_mac_table();

	update_router_mode();

	reset_wan_vars(0);

	ipt_nat_default();

	add_static_lan_routes(IFNAME_BR);

	reset_detect_link();

	switch_config_vlan(0);

	select_usb_modem_to_wan();

	start_wan();

	/* restore L2TP VPN server after L2TP WAN client closed */
	if (nvram_match("l2tp_srv_t", "1"))
		safe_start_xl2tpd();
}

void 
try_wan_reconnect(int try_use_modem)
{
	if (get_ap_mode())
		return;

	stop_wan();

	reset_wan_vars(0);

	if (try_use_modem)
		select_usb_modem_to_wan();

	reset_detect_link();
	start_wan();

	/* restore L2TP VPN server after L2TP WAN client closed */
	if (nvram_match("l2tp_srv_t", "1"))
		safe_start_xl2tpd();
}

int
update_resolvconf(int is_first_run, int do_not_notify)
{
	FILE *fp;
	char word[512], *next, *wan_dns;
	char *google_dns = "8.8.8.8";
	char *resolv_conf, *resolv_temp;
	int lock, dns_static;
	int total_dns = 0;
	int resolv_changed = 0;

	resolv_conf = "/etc/resolv.conf";
	resolv_temp = "/etc/resolv.tmp";

	lock = file_lock("resolv");

	dns_static = is_dns_static();

	wan_dns = NULL;
	fp = fopen((is_first_run) ? resolv_conf : resolv_temp, "w+");
	if (fp) {
		int i_pdns = nvram_get_int("vpnc_pdns");
		
		/* dnsmasq will resolve localhost DNS queries */
		fprintf(fp, "nameserver %s\n", "127.0.0.1");
		
		/* DNS servers for static VPN client */
		if (!is_first_run && i_pdns > 0) {
			wan_dns = nvram_safe_get("vpnc_dns_t");
			if (*wan_dns) {
				foreach(word, wan_dns, next) {
					if (strcmp(word, "0.0.0.0")) {
						fprintf(fp, "nameserver %s\n", word);
						total_dns++;
					}
				}
			}
		}
		
		/* DNS servers for WAN/MAN */
		if (i_pdns != 2 || total_dns < 1) {
			if (dns_static) {
				if (is_first_run)
					resolv_changed = 1;
				
				if (nvram_invmatch("wan_dns1_x", "") && nvram_invmatch("wan_dns1_x", "0.0.0.0")) {
					fprintf(fp, "nameserver %s\n", nvram_safe_get("wan_dns1_x"));
					total_dns++;
				}
				
				if (nvram_invmatch("wan_dns2_x", "") && nvram_invmatch("wan_dns2_x", "0.0.0.0")) {
					fprintf(fp, "nameserver %s\n", nvram_safe_get("wan_dns2_x"));
					total_dns++;
				}
			} else if (!is_first_run) {
				if (strlen(nvram_safe_get("wan0_dns")) > 6)
					wan_dns = nvram_safe_get("wan0_dns");
				else
					wan_dns = nvram_safe_get("wanx_dns");
				
				foreach(word, wan_dns, next) {
					if (strcmp(word, "0.0.0.0")) {
						fprintf(fp, "nameserver %s\n", word);
						total_dns++;
					}
				}
			}
		}
		
		if (total_dns < 1)
			fprintf(fp, "nameserver %s\n", google_dns);
		
#if defined (USE_IPV6)
		/* DNSv6 servers */
		wan_dns = nvram_safe_get("wan0_dns6");
		foreach(word, wan_dns, next) {
			if (strlen(word) > 0) {
				char dns6s[INET6_ADDRSTRLEN] = {0};
				if (ipv6_compact(word, dns6s, 0) == 0) {
					fprintf(fp, "nameserver %s\n", dns6s);
					if (is_first_run)
						resolv_changed = 1;
				}
			}
		}
#endif
		fclose(fp);
	}

	if (!is_first_run) {
		if (compare_text_files(resolv_conf, resolv_temp) != 0) {
			rename(resolv_temp, resolv_conf);
			resolv_changed = 1;
		}
		unlink(resolv_temp);
	}

	file_unlock(lock);

	/* force notify dnsmasq for modem */
	if (!resolv_changed && get_usb_modem_wan(0))
		resolv_changed = 1;

	/* notify dnsmasq */
	if (resolv_changed && !do_not_notify)
		restart_dns();

	return 0;
}

int 
update_hosts(void)
{
	FILE *fp;
	int i, i_max, i_sdhcp;
	char dhcp_ip[32], dhcp_name[32], *sip, *sname;
	char *ipaddr;
	char *host_name_nbt;

	ipaddr = nvram_safe_get("lan_ipaddr");

	host_name_nbt = get_our_hostname();

	i_sdhcp = nvram_get_int("dhcp_static_x");
	i_max  = nvram_get_int("dhcp_staticnum_x");
	if (i_max > 64) i_max = 64;

	fp = fopen("/etc/hosts", "w+");
	if (fp) {
		fprintf(fp, "127.0.0.1 %s %s\n", "localhost.localdomain", "localhost");
		fprintf(fp, "%s my.router\n", ipaddr);
		fprintf(fp, "%s %s\n", ipaddr, host_name_nbt);
		if (i_sdhcp == 1) {
			for (i = 0; i < i_max; i++) {
				sprintf(dhcp_ip, "dhcp_staticip_x%d", i);
				sprintf(dhcp_name, "dhcp_staticname_x%d", i);
				sip = nvram_safe_get(dhcp_ip);
				sname = nvram_safe_get(dhcp_name);
				if (inet_addr_(sip) != INADDR_ANY && inet_addr_(sip) != inet_addr_(ipaddr) && is_valid_hostname(sname))
				{
					fprintf(fp, "%s %s\n", sip, sname);
				}
			}
		}
		
#if defined (USE_IPV6)
		if (get_ipv6_type() != IPV6_DISABLED) {
			fprintf(fp, "::1 %s %s\n", "localhost.localdomain", "localhost");
			char addr6s[INET6_ADDRSTRLEN];
			char* lan_addr6_host = get_lan_addr6_host(addr6s);
			if (lan_addr6_host) {
				fprintf(fp, "%s my.router\n", lan_addr6_host);
				fprintf(fp, "%s my.%s\n", lan_addr6_host, nvram_safe_get("productid"));
				fprintf(fp, "%s %s\n", lan_addr6_host, host_name_nbt);
			}
		}
#endif
		fclose(fp);
	}

	return 0;
}

void
add_dhcp_routes(char *rt, char *rt_rfc, char *rt_ms, char *ifname, int metric)
{
	char *routes, *tmp;
	struct in_addr mask;
	char *ipaddr, *gateway;
	int bits;
	char netmask[] = "255.255.255.255";

	if (!nvram_match("dr_enable_x", "1"))
		return;

	/* routes */
	routes = strdup(rt);
	for (tmp = routes; tmp && *tmp; )
	{
		ipaddr = strsep(&tmp, "/");
		gateway = strsep(&tmp, " ");
		if (gateway && inet_addr_(ipaddr) != INADDR_ANY) {
			route_add(ifname, metric + 1, ipaddr, gateway, netmask);
		}
	}
	free(routes);

	/* rfc3442 or ms classless static routes */
	routes = rt_rfc;
	if (!*routes)
		routes = rt_ms;
	routes = strdup(routes);
	for (tmp = routes; tmp && *tmp; )
	{
		ipaddr  = strsep(&tmp, "/");
		bits    = atoi(strsep(&tmp, " "));
		gateway = strsep(&tmp, " ");
		
		if (gateway && bits > 0 && bits <= 32)
		{
			mask.s_addr = htonl(0xffffffff << (32 - bits));
			strcpy(netmask, inet_ntoa(mask));
			route_add(ifname, metric + 1, ipaddr, gateway, netmask);
		}
	}
	free(routes);
}

void
add_dhcp_routes_by_prefix(char *prefix, char *ifname, int metric)
{
	char buf[32];
	char *rt, *rt_rfc, *rt_ms;

	rt     = nvram_safe_get(strcat_r(prefix, "routes", buf));
	rt_rfc = nvram_safe_get(strcat_r(prefix, "routes_rfc", buf));
	rt_ms  = nvram_safe_get(strcat_r(prefix, "routes_ms", buf));

	add_dhcp_routes(rt, rt_rfc, rt_ms, ifname, metric);
}

int
add_static_wan_routes(char *wan_ifname)
{
	char prefix[] = "wanXXXXXXXXXX_";

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;

	return control_static_routes(SR_PREFIX_WAN, wan_ifname, 1);
}

int
del_static_wan_routes(char *wan_ifname)
{
	char prefix[] = "wanXXXXXXXXXX_";

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;

	return control_static_routes(SR_PREFIX_WAN, wan_ifname, 0);
}

int
add_static_man_routes(char *wan_ifname)
{
	return control_static_routes(SR_PREFIX_MAN, wan_ifname, 1);
}

int
del_static_man_routes(char *wan_ifname)
{
	return control_static_routes(SR_PREFIX_MAN, wan_ifname, 0);
}


void 
select_usb_modem_to_wan(void)
{
	int modem_devnum = 0;

	// Check modem enabled
	if (nvram_get_int("modem_rule") > 0)
	{
		if (nvram_match("modem_type", "3"))
		{
			if (!is_ready_modem_ndis(&modem_devnum))
				modem_devnum = 0;
		}
		else
		{
			if (!is_ready_modem_ras(&modem_devnum))
				modem_devnum = 0;
		}
	}

	set_usb_modem_dev_wan(0, modem_devnum);
}

int 
is_dns_static(void)
{
	if (get_usb_modem_wan(0))
	{
		return nvram_match("modem_dnsa", "0"); // dynamic or static dns for ppp2/eth0
	}

	if (nvram_match("wan0_proto", "static"))
	{
		return 1; // always static dns for eth3/eth2.2
	}

	return !nvram_match("wan_dnsenable_x", "1"); // dynamic or static dns for ppp0 or eth3/eth2.2
}

int 
is_physical_wan_dhcp(void)
{
	if (nvram_match("wan_proto", "static"))
	{
		return 0;
	}

	if (nvram_match("wan_proto", "dhcp") || nvram_match("x_DHCPClient", "1"))
	{
		return 1;
	}

	return 0;
}


int 
is_wan_ppp(char *wan_proto)
{
	if (strcmp(wan_proto, "pppoe") == 0 || strcmp(wan_proto, "pptp") == 0 || strcmp(wan_proto, "l2tp") == 0)
	{
		return 1;
	}
	
	return 0;
}

void get_wan_ifname(char wan_ifname[16])
{
	char *ifname = get_man_ifname(0);
	char *ndis_ifname = nvram_safe_get("wan_ifname_t");
	char *wan_proto = nvram_safe_get("wan_proto");
	
	if (get_usb_modem_wan(0)){
		if (nvram_match("modem_type", "3")) {
			if (isUsbNetIf(ndis_ifname))
				ifname = ndis_ifname;
		}
		else
			ifname = IFNAME_RAS;
	}
	else
	if (is_wan_ppp(wan_proto))
	{
		ifname = IFNAME_PPP;
	}
	
	strcpy(wan_ifname, ifname);
}

int
wan_prefix(char *ifname, char *prefix)
{
	int unit;
	
	if ((unit = wan_ifunit(ifname)) < 0)
		return -1;

	sprintf(prefix, "wan%d_", unit);
	return 0;
}


int
wan_ifunit(char *wan_ifname)
{
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	if (ppp_ifunit(wan_ifname) >= 0) {
		return 0;
	} else {
		char *ndis_ifname = nvram_safe_get("wan_ifname_t");
		if (isUsbNetIf(ndis_ifname) && strcmp(wan_ifname, ndis_ifname) == 0)
			return 0;
		
		for (unit = 0; unit < 2; unit ++) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname) &&
			    (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp") ||
			     nvram_match(strcat_r(prefix, "proto", tmp), "static")))
				return unit;
		}
	}

	return -1;
}

int
preset_wan_routes(char *wan_ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	printf("preset wan routes [%s]\n", wan_ifname);

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;

	/* Set default route to gateway if specified */
	if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
	{
		route_add(wan_ifname, 0, "0.0.0.0", "0.0.0.0", "0.0.0.0");
	}

	/* Install interface dependent static routes */
	add_static_wan_routes(wan_ifname);
	return 0;
}

int
wan_primary_ifunit(void)
{
	int unit;
	char tmp[100], prefix[16];
	
	for (unit = 0; unit < 2; unit ++) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
			return unit;
	}

	return 0;
}

int
is_ifunit_modem(char *wan_ifname)
{
	if (get_usb_modem_wan(0))
	{
		char *ndis_ifname;
		
		if (ppp_ifunit(wan_ifname) >= RAS_PPP_UNIT)
			return 1;
		
		ndis_ifname = nvram_safe_get("wan_ifname_t");
		if (isUsbNetIf(ndis_ifname) && strcmp(wan_ifname, ndis_ifname) == 0)
			return 2;
	}
	
	return 0;
}

int
has_wan_ip(int only_broadband_wan)
{
	if (get_wan_ipaddr(only_broadband_wan) != INADDR_ANY)
		return 1;
	
	return 0;
}

int
got_wan_ip()
{
	char *wan_ip = nvram_safe_get("wan_ipaddr_t");
	if (strcmp("", wan_ip) && strcmp("0.0.0.0", wan_ip))
		return 1;
	else
		return 0;
}

in_addr_t get_wan_ipaddr(int only_broadband_wan)
{
	char *ifname = get_man_ifname(0);

	if (get_ap_mode())
		return INADDR_ANY;

	if(!only_broadband_wan && get_usb_modem_wan(0)){
		ifname = nvram_safe_get("wan_ifname_t");
	}
	else if (nvram_match("wan0_proto", "dhcp") || nvram_match("wan0_proto", "static"))
		ifname = get_man_ifname(0);
	else
		ifname = IFNAME_PPP;
	
	return get_ipv4_addr(ifname);
}

int
found_default_route(int only_broadband_wan)
{
	int i, n, found;
	FILE *fp;
	unsigned int dest, mask;
	char buf[256], device[256];
	n = 0;
	found = 0;
	mask = 0;
	device[0] = '\0';

	fp = fopen("/proc/net/route", "r");
	if (fp)
	{
		while (fgets(buf, sizeof(buf), fp) != NULL)
		{
			if (++n == 1 && strncmp(buf, "Iface", 5) == 0)
				continue;

			i = sscanf(buf, "%255s %x %*s %*s %*s %*s %*s %x",
						device, &dest, &mask);

			if (i != 3)
			{
				break;
			}

			if (device[0] != '\0' && dest == 0 && mask == 0)
			{
				found = 1;
				break;
			}
		}

		fclose(fp);

		if (found)
		{
			if(!only_broadband_wan && get_usb_modem_wan(0)){
				char *modem_ifname = nvram_safe_get("wan_ifname_t");
				if (strcmp(device, modem_ifname) == 0)
					return 1;
				else
					goto no_default_route;
			}
			else
			if (nvram_match("wan0_proto", "dhcp") || nvram_match("wan0_proto", "static"))
			{
				if (!strcmp(get_man_ifname(0), device))
					return 1;
				else
					goto no_default_route;
			}
			else
			{
				if (!strcmp(IFNAME_PPP, device) || !strcmp(get_man_ifname(0), device))
					return 1;
				else
					goto no_default_route;
			}
		}
		else
			goto no_default_route;
	}

no_default_route:

	return 0;
}

void 
update_wan_status(int isup)
{
	char wan_subnet[32];
	char *proto = nvram_safe_get("wan_proto");

	if(get_usb_modem_wan(0))
		nvram_set_temp("wan_proto_t", "Modem");
	else if (!strcmp(proto, "static")) nvram_set_temp("wan_proto_t", "Static");
	else if (!strcmp(proto, "dhcp")) nvram_set_temp("wan_proto_t", "Automatic IP");
	else if (!strcmp(proto, "pppoe")) nvram_set_temp("wan_proto_t", "PPPoE");
	else if (!strcmp(proto, "pptp")) nvram_set_temp("wan_proto_t", "PPTP");
	else if (!strcmp(proto, "l2tp")) nvram_set_temp("wan_proto_t", "L2TP");

	if (!isup)
	{
		reset_wan_temp();
	}
	else
	{
		nvram_set_temp("wan_ipaddr_t", nvram_safe_get("wan0_ipaddr"));
		nvram_set_temp("wan_netmask_t", nvram_safe_get("wan0_netmask"));
		nvram_set_temp("wan_gateway_t", nvram_safe_get("wan0_gateway"));
		
		snprintf(wan_subnet, sizeof(wan_subnet), "0x%x", 
			inet_network(nvram_safe_get("wan0_ipaddr"))&inet_network(nvram_safe_get("wan0_netmask")));
		
		nvram_set_temp("wan_subnet_t", wan_subnet);
		nvram_set_temp("wan_status_t", "Connected");
	}
}


static int
udhcpc_deconfig(char *wan_ifname, int is_zcip)
{
	char *client_info = (is_zcip) ? "ZeroConf WAN Client" : "DHCP WAN Client";
	
	int unit = wan_ifunit(wan_ifname);
	
	if ( (unit < 0) && (nvram_match("wan0_proto", "l2tp") || nvram_match("wan0_proto", "pptp")))
	{
		/* fix hang-up issue */
		logmessage(client_info, "skipping resetting IP address to 0.0.0.0");
	}
	else
	{
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
		
		if (unit < 0)
		{
			nvram_set_temp("wanx_ipaddr", "0.0.0.0");
		}
	}

	wan_down(wan_ifname);

	logmessage(client_info, "%s: lease is lost", udhcp_state);

	return 0;
}

static int
udhcpc_bound(char *wan_ifname)	// udhcpc bound here, also call wanup
{
	char *value;
	char tmp[100], prefix[16], route[32];
	int unit;
	int changed = 0;
	int gateway = 0;
	int lease_dur = 0;

	if ((unit = wan_ifunit(wan_ifname)) < 0) 
		strcpy(prefix, "wanx_");
	else
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	if ((value = getenv("ip"))) {
		changed = nvram_invmatch(strcat_r(prefix, "ipaddr", tmp), value);
		nvram_set_temp(strcat_r(prefix, "ipaddr", tmp), trim_r(value));
	}
	if ((value = getenv("subnet")))
		nvram_set_temp(strcat_r(prefix, "netmask", tmp), trim_r(value));
	if ((value = getenv("router"))) {
		gateway = 1;
		nvram_set_temp(strcat_r(prefix, "gateway", tmp), trim_r(value));
	}
	if ((value = getenv("dns")))
		nvram_set_temp(strcat_r(prefix, "dns", tmp), trim_r(value));
	if ((value = getenv("wins")))
		nvram_set_temp(strcat_r(prefix, "wins", tmp), trim_r(value));

	nvram_set_temp(strcat_r(prefix, "routes", tmp), safe_getenv("routes"));
	nvram_set_temp(strcat_r(prefix, "routes_ms", tmp), safe_getenv("msstaticroutes"));
	nvram_set_temp(strcat_r(prefix, "routes_rfc", tmp), safe_getenv("staticroutes"));
#if 0
	if ((value = getenv("hostname")))
		sethostname(trim_r(value), strlen(value) + 1);
#endif
	if ((value = getenv("domain")))
		nvram_set_temp(strcat_r(prefix, "domain", tmp), trim_r(value));
	if ((value = getenv("lease"))) {
		nvram_set_temp(strcat_r(prefix, "lease", tmp), trim_r(value));
		lease_dur = atoi(value);
	}
	
#if defined (USE_IPV6)
	if ((value = getenv("ip6rd")))
		store_ip6rd_from_dhcp(value, prefix);
#endif
	
	if (!gateway) {
		foreach(route, nvram_safe_get(strcat_r(prefix, "routes_rfc", tmp)), value) {
			if (gateway) {
				nvram_set_temp(strcat_r(prefix, "gateway", tmp), route);
				break;
			} else
				gateway = !strcmp(route, "0.0.0.0/0");
		}
	}
	
	if (changed && unit == 0)
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
	
	ifconfig(wan_ifname, IFUP,
		 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
		 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));

	wan_up(wan_ifname);

	logmessage("DHCP WAN Client", "%s (%s), IP: %s, GW: %s, lease time: %d", 
		udhcp_state, 
		wan_ifname,
		nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
		nvram_safe_get(strcat_r(prefix, "gateway", tmp)), lease_dur);

	return 0;
}

static int
udhcpc_viptv_bound(char *man_ifname)
{
	char *value, *ip, *nm, *gw, *rt, *rt_ms, *rt_rfc;
	char tmp[100], prefix[16];
	int lease_dur = 0;

	strcpy(prefix, "manv_");

	if ((value = getenv("ip")))
		ip = trim_r(value);
	else
		ip = "0.0.0.0";

	nvram_set_temp(strcat_r(prefix, "ipaddr", tmp), ip);

	if ((value = getenv("subnet")))
		nm = trim_r(value);
	else
		nm = "255.255.0.0";

	nvram_set_temp(strcat_r(prefix, "netmask", tmp), nm);

	if ((value = getenv("router")))
		gw = trim_r(value);
	else
		gw = "";

	nvram_set_temp(strcat_r(prefix, "gateway", tmp), gw);

	if ((value = getenv("routes")))
		rt = trim_r(value);
	else
		rt = "";

	if ((value = getenv("msstaticroutes")))
		rt_ms = trim_r(value);
	else
		rt_ms = "";

	if ((value = getenv("staticroutes")))
		rt_rfc = trim_r(value);
	else
		rt_rfc = "";

	nvram_set_temp(strcat_r(prefix, "routes", tmp), rt);
	nvram_set_temp(strcat_r(prefix, "routes_ms", tmp), rt_ms);
	nvram_set_temp(strcat_r(prefix, "routes_rfc", tmp), rt_rfc);

	if ((value = getenv("lease")))
		lease_dur = atoi(value);

	ifconfig(man_ifname, IFUP, "0.0.0.0", NULL);
	ifconfig(man_ifname, IFUP, ip, nm);

	if (*rt || *rt_rfc || *rt_ms)
		add_dhcp_routes(rt, rt_rfc, rt_ms, man_ifname, 0);

	/* default route via default gateway (metric 2) */
	if (*gw)
		route_add(man_ifname, 3, "0.0.0.0", gw, "0.0.0.0");

	start_igmpproxy(man_ifname);

	logmessage("DHCP IPTV Client", "%s (%s), IP: %s, GW: %s, lease time: %d", 
			"bound", man_ifname, ip, gw, lease_dur);

	return 0;
}

static int
zcip_bound(char *wan_ifname)
{
	char *value;
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	int changed = 0;
	
	strcpy(prefix, "wanx_");
	
	if ((value = getenv("ip"))) {
		changed = nvram_invmatch(strcat_r(prefix, "ipaddr", tmp), value);
		nvram_set_temp(strcat_r(prefix, "ipaddr", tmp), trim_r(value));
	}
	
	nvram_set_temp(strcat_r(prefix, "netmask", tmp), "255.255.0.0");
	nvram_set_temp(strcat_r(prefix, "gateway", tmp), "");
	nvram_set_temp(strcat_r(prefix, "dns", tmp), "");
	
	if (changed)
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
	
	ifconfig(wan_ifname, IFUP,
		nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
		nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
	
	wan_up(wan_ifname);
	
	return 0;
}

static int
zcip_viptv_bound(char *man_ifname)
{
	char *value;

	if ((value = getenv("ip"))) {
		ifconfig(man_ifname, IFUP, trim_r(value), "255.255.0.0");
		
		start_igmpproxy(man_ifname);
	}

	return 0;
}

static int
udhcpc_renew(char *wan_ifname)
{
	char *value;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;
	int changed = 0;

	if ((unit = wan_ifunit(wan_ifname)) < 0)
		strcpy(prefix, "wanx_");
	else
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	if (!(value = getenv("subnet")) || nvram_invmatch(strcat_r(prefix, "netmask", tmp), trim_r(value)))
		return udhcpc_bound(wan_ifname);
	if ((value = getenv("router")) && nvram_invmatch(strcat_r(prefix, "gateway", tmp), trim_r(value)))
		return udhcpc_bound(wan_ifname);
	if ((value = getenv("ip")) && nvram_invmatch(strcat_r(prefix, "ipaddr", tmp), trim_r(value)))
		return udhcpc_bound(wan_ifname);
	if ((value = getenv("dns")) && nvram_invmatch(strcat_r(prefix, "dns", tmp), trim_r(value))) {
		nvram_set_temp(strcat_r(prefix, "dns", tmp), trim_r(value));
		changed = 1;
	}
	if ((value = getenv("wins")))
		nvram_set_temp(strcat_r(prefix, "wins", tmp), trim_r(value));
#if 0
	if ((value = getenv("hostname")))
		sethostname(trim_r(value), strlen(value) + 1);
#endif
	if ((value = getenv("domain")))
		nvram_set_temp(strcat_r(prefix, "domain", tmp), trim_r(value));
	if ((value = getenv("lease")))
		nvram_set_temp(strcat_r(prefix, "lease", tmp), trim_r(value));

	if (changed){
		update_resolvconf(0, 0);
		
		if (unit == 0)
			update_wan_status(1);
		
		logmessage("DHCP WAN Client", "%s (%s), new dns: %s", 
			udhcp_state, 
			wan_ifname,
			nvram_safe_get(strcat_r(prefix, "dns", tmp)) );
	}

	return 0;
}

static int
udhcpc_viptv_renew(char *man_ifname)
{
	char *value;
	char tmp[100], prefix[16];

	strcpy(prefix, "manv_");

	if ((value = getenv("subnet")) && nvram_invmatch(strcat_r(prefix, "netmask", tmp), trim_r(value)))
		return udhcpc_viptv_bound(man_ifname);
	if ((value = getenv("router")) && nvram_invmatch(strcat_r(prefix, "gateway", tmp), trim_r(value)))
		return udhcpc_viptv_bound(man_ifname);
	if ((value = getenv("ip")) && nvram_invmatch(strcat_r(prefix, "ipaddr", tmp), trim_r(value)))
		return udhcpc_viptv_bound(man_ifname);

	return 0;
}

static int 
udhcpc_leasefail(char *wan_ifname)
{
	return 0;
}

static int 
udhcpc_viptv_leasefail(char *man_ifname)
{
	/* DHCP failed for IPTV, start ZCIP */
	start_zcip_viptv(man_ifname);
	stop_udhcpc_viptv();

	return 0;
}

static int 
udhcpc_noack(char *wan_ifname)
{
	logmessage("DHCP WAN Client", "Received NAK for %s", wan_ifname);
	return 0;
}

static int 
udhcpc_viptv_noack(char *man_ifname)
{
	logmessage("DHCP IPTV Client", "Received NAK for %s", man_ifname);
	return 0;
}

int
udhcpc_main(int argc, char **argv)
{
	int ret = 0;
	char *wan_ifname;

	if (argc<2 || !argv[1])
		return EINVAL;

	wan_ifname = safe_getenv("interface");
	strncpy(udhcp_state, argv[1], sizeof(udhcp_state));

	umask(0000);

	if (!strcmp(argv[1], "deconfig"))
		ret = udhcpc_deconfig(wan_ifname, 0);
	else if (!strcmp(argv[1], "bound"))
		ret = udhcpc_bound(wan_ifname);
	else if (!strcmp(argv[1], "renew"))
		ret = udhcpc_renew(wan_ifname);
	else if (!strcmp(argv[1], "leasefail"))
		ret = udhcpc_leasefail(wan_ifname);
	else if (!strcmp(argv[1], "nak"))
		ret = udhcpc_noack(wan_ifname);

	return ret;
}

int
udhcpc_viptv_main(int argc, char **argv)
{
	int ret = 0;
	char *man_ifname;

	if (argc<2 || !argv[1])
		return EINVAL;

	man_ifname = safe_getenv("interface");

	umask(0000);

	if (!strcmp(argv[1], "deconfig"))
		ret = 0;
	else if (!strcmp(argv[1], "bound"))
		ret = udhcpc_viptv_bound(man_ifname);
	else if (!strcmp(argv[1], "renew"))
		ret = udhcpc_viptv_renew(man_ifname);
	else if (!strcmp(argv[1], "leasefail"))
		ret = udhcpc_viptv_leasefail(man_ifname);
	else if (!strcmp(argv[1], "nak"))
		ret = udhcpc_viptv_noack(man_ifname);

	return ret;
}

int
zcip_main(int argc, char **argv)
{
	int ret = 0;
	char *wan_ifname;

	if (argc<2 || !argv[1])
		return EINVAL;

	wan_ifname = safe_getenv("interface");
	strncpy(udhcp_state, argv[1], sizeof(udhcp_state));

	umask(0000);

	if (!strcmp(argv[1], "deconfig"))
		ret = udhcpc_deconfig(wan_ifname, 1);
	else if (!strcmp(argv[1], "config"))
		ret = zcip_bound(wan_ifname);

	return ret;
}

int
zcip_viptv_main(int argc, char **argv)
{
	int ret = 0;
	char *man_ifname;

	if (argc<2 || !argv[1])
		return EINVAL;

	man_ifname = safe_getenv("interface");

	umask(0000);

	if (!strcmp(argv[1], "config"))
		ret = zcip_viptv_bound(man_ifname);

	return ret;
}

int start_udhcpc_wan(const char *wan_ifname, int unit, int wait_lease)
{
	char tmp[100], prefix[16];
	char pidfile[32];
	char *wan_hostname;
	int index;
	
	sprintf(pidfile, "/var/run/udhcpc%d.pid", unit);
	
	char *dhcp_argv[] = {
		"/sbin/udhcpc",
		"-i", (char *)wan_ifname,
		"-s", SCRIPT_UDHCPC_WAN,
		"-p", pidfile,
		"-t4",
		"-T4",
		NULL,
		NULL, NULL,	/* -H wan_hostname	*/
		NULL,		/* -O routes		*/
		NULL,		/* -O staticroutes	*/
		NULL,		/* -O msstaticroutes	*/
#if defined (USE_IPV6)
		NULL,		/* -O 6rd		*/
		NULL,		/* -O comcast6rd	*/
#endif
		NULL
	};
	index = 9;		/* first NULL index	*/
	
	if (wait_lease)
		dhcp_argv[index++] = "-b"; /* Background if lease is not obtained (timeout 4*4 sec) */
	else
		dhcp_argv[index++] = "-d"; /* Background after run (new patch for udhcpc) */
	
	/* We have to trust unit */
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	
	wan_hostname = nvram_safe_get(strcat_r(prefix, "hostname", tmp));
	if (*wan_hostname) {
		dhcp_argv[index++] = "-H";
		dhcp_argv[index++] = sanity_hostname(wan_hostname);
	}
	
	if (nvram_match("dr_enable_x", "1")) {
		dhcp_argv[index++] = "-O33";	/* "routes" */
		dhcp_argv[index++] = "-O121";	/* "staticroutes" */
		dhcp_argv[index++] = "-O249";   /* "msstaticroutes" */
	}
	
#if defined (USE_IPV6)
	if (get_ipv6_type() == IPV6_6RD) {
		dhcp_argv[index++] = "-O212";	/* "6rd" */
		dhcp_argv[index++] = "-O150";	/* "comcast6rd" */
	}
#endif
	logmessage("DHCP WAN Client", "starting on %s ...", wan_ifname);
	
	return _eval(dhcp_argv, NULL, 0, NULL);
}

int start_udhcpc_viptv(const char *man_ifname)
{
	int index;
	char pidfile[32];

	sprintf(pidfile, "/var/run/udhcpc_viptv.pid");

	char *dhcp_argv[] = {
		"/sbin/udhcpc",
		"-i", (char *)man_ifname,
		"-s", SCRIPT_UDHCPC_VIPTV,
		"-p", pidfile,
		"-t4",
		"-T3",
		"-d",		/* Background after run (new patch for udhcpc) */
		NULL,		/* -O routes		*/
		NULL,		/* -O staticroutes	*/
		NULL,		/* -O msstaticroutes	*/
		NULL
	};

	index = 10;		/* first NULL index	*/

	if (nvram_match("dr_enable_x", "1")) {
		dhcp_argv[index++] = "-O33";	/* "routes" */
		dhcp_argv[index++] = "-O121";	/* "staticroutes" */
		dhcp_argv[index++] = "-O249";	/* "msstaticroutes" */
	}
	
	logmessage("DHCP IPTV Client", "starting on %s ...", man_ifname);
	
	return _eval(dhcp_argv, NULL, 0, NULL);
}

int start_zcip_wan(const char *wan_ifname)
{
	return eval("/sbin/zcip", (char*)wan_ifname, SCRIPT_ZCIP_WAN);
}

int start_zcip_viptv(const char *man_ifname)
{
	logmessage("ZeroConf IPTV Client", "starting on %s ...", man_ifname);

	return eval("/sbin/zcip", "-q", (char*)man_ifname, SCRIPT_ZCIP_VIPTV);
}

int renew_udhcpc_wan(int unit)
{
	char pidfile[32];
	
	sprintf(pidfile, "/var/run/udhcpc%d.pid", unit);
	
	return kill_pidfile_s(pidfile, SIGUSR1);
}

int release_udhcpc_wan(int unit)
{
	char pidfile[32];
	
	sprintf(pidfile, "/var/run/udhcpc%d.pid", unit);
	
	return kill_pidfile_s(pidfile, SIGUSR2);
}

int stop_udhcpc_wan(int unit)
{
	char pidfile[32];
	
	sprintf(pidfile, "/var/run/udhcpc%d.pid", unit);
	
	return kill_pidfile(pidfile);
}

int stop_udhcpc_viptv(void)
{
	char pidfile[32];
	
	sprintf(pidfile, "/var/run/udhcpc_viptv.pid");
	
	return kill_pidfile(pidfile);
}
