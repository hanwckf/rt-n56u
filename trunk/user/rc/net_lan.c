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

#include "rc.h"
#include "switch.h"

#if BOARD_RAM_SIZE < 32
#define SHRINK_TX_QUEUE_LEN	(300)
#elif BOARD_RAM_SIZE < 64
#define SHRINK_TX_QUEUE_LEN	(600)
#endif

static char udhcpc_lan_state[16] = {0};

in_addr_t get_lan_ip4(void)
{
	return get_interface_addr4(IFNAME_BR);
}

int
has_lan_ip4(void)
{
	if (get_lan_ip4() != INADDR_ANY)
		return 1;

	return 0;
}

int
has_lan_gw4(void)
{
	if (is_valid_ipv4(nvram_safe_get("lan_gateway_t")))
		return 1;

	return 0;
}

int
add_static_lan_routes(char *lan_ifname)
{
	return control_static_routes(SR_PREFIX_LAN, lan_ifname, 1);
}

int
del_static_lan_routes(char *lan_ifname)
{
	return control_static_routes(SR_PREFIX_LAN, lan_ifname, 0);
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
init_bridge(int is_ap_mode)
{
	int rt_radio_on = get_enabled_radio_rt();
#if !defined(USE_RT3352_MII)
	int rt_mode_x = get_mode_radio_rt();
#endif
#if BOARD_HAS_5G_RADIO
	int wl_radio_on = get_enabled_radio_wl();
	int wl_mode_x = get_mode_radio_wl();
#endif
	char *lan_hwaddr = nvram_safe_get("lan_hwaddr");

	if (!is_ap_mode)
	{
		/* set switch bridge mode and vlan isolation */
		switch_config_vlan(1);
	}
	else
	{
		/* set switch bridge mode to LLLLL */
		phy_bridge_mode(SWAPI_WAN_BRIDGE_DISABLE_WAN, SWAPI_WAN_BWAN_ISOLATION_NONE);
	}

#if BOARD_RAM_SIZE < 64
	doSystem("ifconfig %s txqueuelen %d", IFNAME_MAC, SHRINK_TX_QUEUE_LEN);
#endif
	set_interface_hwaddr(IFNAME_MAC, lan_hwaddr);
	ifconfig(IFNAME_MAC, IFUP, NULL, NULL);

	switch_config_base();
	switch_config_storm();
	switch_config_link();

	/* power up all switch PHY */
	phy_ports_power(1);

#if defined (USE_SINGLE_MAC)
	if (!is_ap_mode)
	{
		/* create VLAN1/2 */
		create_vlan_iface(IFNAME_MAC, 1, -1, -1, lan_hwaddr, 1);
		create_vlan_iface(IFNAME_MAC, 2, -1, -1, NULL, 0);
	}
#if defined (AP_MODE_LAN_TAGGED)
	else
	{
		/* create VLAN1 */
		create_vlan_iface(IFNAME_MAC, 1, -1, -1, lan_hwaddr, 1);
	}
#endif
#endif

#if defined (USE_RT3352_MII)
	/* create VLAN3 for guest AP */
	create_vlan_iface(IFNAME_MAC, INIC_GUEST_VLAN_VID, -1, -1, lan_hwaddr, 1);
#endif

#if BOARD_2G_IN_SOC || defined (BOARD_MT7915_DBDC)
#if !defined (USE_RT3352_MII)
	if (!rt_radio_on || (rt_mode_x == 1 || rt_mode_x == 3))
	{
		/* workaround for create all pseudo interfaces and fix iNIC issue (common PLL config) */
		gen_ralink_config_2g(1);
		doSystem("ifconfig %s %s", IFNAME_2G_MAIN, "up");
	}
#endif
#if BOARD_HAS_5G_RADIO
	if (!wl_radio_on || (wl_mode_x == 1 || wl_mode_x == 3))
	{
		/* workaround for create all pseudo interfaces */
		gen_ralink_config_5g(1);
		doSystem("ifconfig %s %s", IFNAME_5G_MAIN, "up");
	}
#endif
#else /* BOARD_2G_IN_SOC || defined (BOARD_MT7915_DBDC) */
#if BOARD_HAS_5G_RADIO
	if (!wl_radio_on || (wl_mode_x == 1 || wl_mode_x == 3))
	{
		/* workaround for create all pseudo interfaces and fix iNIC issue (common PLL config) */
		gen_ralink_config_5g(1);
		doSystem("ifconfig %s %s", IFNAME_5G_MAIN, "up");
	}
#endif
#if !defined (USE_RT3352_MII)
	if (!rt_radio_on || (rt_mode_x == 1 || rt_mode_x == 3))
	{
		/* workaround for create all pseudo interfaces */
		gen_ralink_config_2g(1);
		doSystem("ifconfig %s %s", IFNAME_2G_MAIN, "up");
	}
#endif
#endif

	br_add_del_bridge(IFNAME_BR, 1);
	br_set_stp(IFNAME_BR, 0);
	br_set_fd(IFNAME_BR, 2);
	set_interface_hwaddr(IFNAME_BR, lan_hwaddr);

	if (!is_ap_mode) {
		/* add eth2 (or eth2.1) to bridge */
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
		if (get_wan_bridge_mode() != SWAPI_WAN_BRIDGE_DISABLE) {
			create_vlan_iface(IFNAME_MAC, 1, -1, -1, lan_hwaddr, 1);
			br_add_del_if(IFNAME_BR, "eth2.1", 1);
		} else
#endif
		br_add_del_if(IFNAME_BR, IFNAME_LAN, 1);
	} else {
#if defined (AP_MODE_LAN_TAGGED)
		/* add eth2 (or eth2.1) to bridge */
		br_add_del_if(IFNAME_BR, IFNAME_LAN, 1);
#else
		/* add only eth2 to bridge */
		br_add_del_if(IFNAME_BR, IFNAME_MAC, 1);
#endif
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
		/* add eth3 to bridge */
		ifconfig(IFNAME_MAC2, IFUP, NULL, NULL);
		br_add_del_if(IFNAME_BR, IFNAME_MAC2, 1);
#if defined (USE_HW_NAT)
		/* enable PPE to forward bridge traffic */
		module_smart_load("hw_nat", "ttl_regen=0");
		logmessage(LOGNAME, "%s: %s", "Hardware NAT/Routing", "Enabled, L2 bridge offload");
#endif
#endif
	}

#if defined(USE_RT3352_MII)
	{
		char inic_param[80];
		snprintf(inic_param, sizeof(inic_param), "miimaster=%s mode=%s syncmiimac=%d bridge=%d max_fw_upload=%d",
			IFNAME_MAC, "ap", 0, 1, 10);
		module_smart_load("iNIC_mii", inic_param);
	}
#endif

#if BOARD_2G_IN_SOC || defined (BOARD_MT7915_DBDC)
	start_wifi_ap_rt(rt_radio_on);
	start_wifi_wds_rt(rt_radio_on);
	start_wifi_apcli_rt(rt_radio_on);
#if BOARD_HAS_5G_RADIO
	start_wifi_ap_wl(wl_radio_on);
	start_wifi_wds_wl(wl_radio_on);
	start_wifi_apcli_wl(wl_radio_on);
#endif
#else /* BOARD_2G_IN_SOC || defined (BOARD_MT7915_DBDC) */
#if BOARD_HAS_5G_RADIO
	start_wifi_ap_wl(wl_radio_on);
	start_wifi_wds_wl(wl_radio_on);
	start_wifi_apcli_wl(wl_radio_on);
#endif
	start_wifi_ap_rt(rt_radio_on);
	start_wifi_wds_rt(rt_radio_on);
	start_wifi_apcli_rt(rt_radio_on);
#endif

#if defined (BOARD_GPIO_LED_SW2G)
	if (rt_radio_on)
		LED_CONTROL(BOARD_GPIO_LED_SW2G, LED_ON);
#endif
#if defined (BOARD_GPIO_LED_SW5G) && BOARD_HAS_5G_RADIO
	if (wl_radio_on)
		LED_CONTROL(BOARD_GPIO_LED_SW5G, LED_ON);
#endif

	sleep(1);

#if BOARD_RAM_SIZE < 64
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_MAIN, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_GUEST, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_APCLI, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_WDS0, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_WDS1, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_WDS2, SHRINK_TX_QUEUE_LEN);
	doSystem("ifconfig %s txqueuelen %d", IFNAME_2G_WDS3, SHRINK_TX_QUEUE_LEN);
#endif

	ifconfig(IFNAME_BR, IFUP, NULL, NULL);

#if BOARD_HAS_5G_RADIO
	if (!wl_radio_on || (wl_mode_x == 1 || wl_mode_x == 3)) {
		usleep(500000);
		doSystem("ifconfig %s %s", IFNAME_5G_MAIN, "down");
		gen_ralink_config_5g(0);
	}

	if (wl_radio_on)
		update_vga_clamp_wl(1);
#endif

#if !defined(USE_RT3352_MII)
	if (!rt_radio_on || (rt_mode_x == 1 || rt_mode_x == 3)) {
		usleep(500000);
		doSystem("ifconfig %s %s", IFNAME_2G_MAIN, "down");
		gen_ralink_config_2g(0);
	}

	if (rt_radio_on)
		update_vga_clamp_rt(1);
#endif

	restart_guest_lan_isolation();

	nvram_set_int_temp("reload_svc_wl", 0);
	nvram_set_int_temp("reload_svc_rt", 0);
}

void
config_bridge(int is_ap_mode)
{
	const char *wired_ifname;
	int multicast_router, multicast_querier, igmp_static_port;
	int igmp_snoop = nvram_get_int("ether_igmp");
	int wired_m2u = nvram_get_int("ether_m2u");

	if (!is_ap_mode) {
		igmp_static_port = -1;
		if (nvram_match("mr_enable_x", "1")) {
			multicast_router = 2;   // bridge is mcast router path (br0 <--igmpproxy--> eth3)
			multicast_querier = 0;  // bridge is not needed internal mcast querier (igmpproxy is mcast querier)
		} else {
			multicast_router = 1;   // bridge may be mcast router path
			multicast_querier = 1;  // bridge is needed internal mcast querier (for eth2-ra0-rai0 snooping work)
		}
		wired_ifname = IFNAME_LAN;
#if defined (USE_GMAC2_TO_GPHY) || defined (USE_GMAC2_TO_GSW)
		if (get_wan_bridge_mode() != SWAPI_WAN_BRIDGE_DISABLE)
			wired_ifname = "eth2.1";
#endif
	} else {
		igmp_static_port = nvram_get_int("ether_uport");
		multicast_router = 0;   // bridge is not mcast router path
		multicast_querier = 1;  // bridge is needed internal mcast querier (for eth2-ra0-rai0 snooping work)
#if defined (AP_MODE_LAN_TAGGED)
		wired_ifname = IFNAME_LAN;
#else
		wired_ifname = IFNAME_MAC;
#endif
	}

	br_set_param_int(IFNAME_BR, "multicast_router", multicast_router);
	br_set_param_int(IFNAME_BR, "multicast_querier", multicast_querier);

	/* allow use bridge IP address as IGMP/MLD query source IP (avoid cisco issue) */
	br_set_param_int(IFNAME_BR, "multicast_query_use_ifaddr", 1);

	br_set_param_int(IFNAME_BR, "multicast_snooping", (igmp_snoop) ? 1 : 0);

	brport_set_m2u(wired_ifname, (igmp_snoop && wired_m2u == 1) ? 1 : 0);

	phy_igmp_static_port(igmp_static_port);
	phy_igmp_snooping((igmp_snoop && wired_m2u == 2) ? 1 : 0);
}

void
switch_config_link(void)
{
	int i, i_flow_mode, i_link_mode;
	char nvram_param[20];

	// WAN
	i_link_mode = nvram_safe_get_int("ether_link_wan", SWAPI_LINK_SPEED_MODE_AUTO,
			SWAPI_LINK_SPEED_MODE_AUTO, SWAPI_LINK_SPEED_MODE_FORCE_POWER_OFF);
	i_flow_mode = nvram_safe_get_int("ether_flow_wan", SWAPI_LINK_FLOW_CONTROL_TX_RX,
			SWAPI_LINK_FLOW_CONTROL_TX_RX, SWAPI_LINK_FLOW_CONTROL_DISABLE);
	phy_set_link_port(SWAPI_PORT_WAN, i_link_mode, i_flow_mode);

	for (i = 0; i < BOARD_NUM_ETH_EPHY-1; i++) {
		snprintf(nvram_param, sizeof(nvram_param), "ether_link_lan%d", i+1);
		i_link_mode = nvram_safe_get_int(nvram_param, SWAPI_LINK_SPEED_MODE_AUTO,
				SWAPI_LINK_SPEED_MODE_AUTO, SWAPI_LINK_SPEED_MODE_FORCE_POWER_OFF);
		snprintf(nvram_param, sizeof(nvram_param), "ether_flow_lan%d", i+1);
		i_flow_mode = nvram_safe_get_int(nvram_param, SWAPI_LINK_FLOW_CONTROL_TX_RX,
				SWAPI_LINK_FLOW_CONTROL_TX_RX, SWAPI_LINK_FLOW_CONTROL_DISABLE);
		phy_set_link_port(SWAPI_PORT_LAN1+i, i_link_mode, i_flow_mode);
	}
}

void
switch_config_base(void)
{
	update_ether_leds();

	phy_jumbo_frames(nvram_get_int("ether_jumbo"));
	phy_green_ethernet(nvram_get_int("ether_green"));
	phy_eee_lpi(nvram_get_int("ether_eee"));
}

void
switch_config_storm(void)
{
	int controlrate_unknown_unicast;
	int controlrate_unknown_multicast;
	int controlrate_multicast;
	int controlrate_broadcast;

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
	int bridge_mode, bwan_isolation, is_vlan_filter;
	int vlan_vid[SWAPI_VLAN_RULE_NUM] = {0};
	int vlan_pri[SWAPI_VLAN_RULE_NUM] = {0};
	int vlan_tag[SWAPI_VLAN_RULE_NUM] = {0};
	unsigned int vrule;

	bridge_mode = get_wan_bridge_mode();
	bwan_isolation = get_wan_bridge_iso_mode(bridge_mode);

	is_vlan_filter = (nvram_match("vlan_filter", "1")) ? 1 : 0;
	if (is_vlan_filter) {
#if defined(USE_MTK_ESW)
		/* MT7620 and MT7628 ESW not support port matrix + security */
		if (bwan_isolation == SWAPI_WAN_BWAN_ISOLATION_BETWEEN)
			bwan_isolation = SWAPI_WAN_BWAN_ISOLATION_NONE;
#endif
		vlan_vid[SWAPI_VLAN_RULE_WAN_INET] = nvram_get_int("vlan_vid_cpu");
		vlan_vid[SWAPI_VLAN_RULE_WAN_IPTV] = nvram_get_int("vlan_vid_iptv");
		vlan_vid[SWAPI_VLAN_RULE_WAN_LAN1] = nvram_get_int("vlan_vid_lan1");
		vlan_vid[SWAPI_VLAN_RULE_WAN_LAN2] = nvram_get_int("vlan_vid_lan2");
		vlan_vid[SWAPI_VLAN_RULE_WAN_LAN3] = nvram_get_int("vlan_vid_lan3");
		vlan_vid[SWAPI_VLAN_RULE_WAN_LAN4] = nvram_get_int("vlan_vid_lan4");
		
		vlan_pri[SWAPI_VLAN_RULE_WAN_INET] = nvram_get_int("vlan_pri_cpu")  & 0x07;
		vlan_pri[SWAPI_VLAN_RULE_WAN_IPTV] = nvram_get_int("vlan_pri_iptv") & 0x07;
		vlan_pri[SWAPI_VLAN_RULE_WAN_LAN1] = nvram_get_int("vlan_pri_lan1") & 0x07;
		vlan_pri[SWAPI_VLAN_RULE_WAN_LAN2] = nvram_get_int("vlan_pri_lan2") & 0x07;
		vlan_pri[SWAPI_VLAN_RULE_WAN_LAN3] = nvram_get_int("vlan_pri_lan3") & 0x07;
		vlan_pri[SWAPI_VLAN_RULE_WAN_LAN4] = nvram_get_int("vlan_pri_lan4") & 0x07;
		
		vlan_tag[SWAPI_VLAN_RULE_WAN_INET] = 0;
		vlan_tag[SWAPI_VLAN_RULE_WAN_IPTV] = 0;
		vlan_tag[SWAPI_VLAN_RULE_WAN_LAN1] = nvram_get_int("vlan_tag_lan1");
		vlan_tag[SWAPI_VLAN_RULE_WAN_LAN2] = nvram_get_int("vlan_tag_lan2");
		vlan_tag[SWAPI_VLAN_RULE_WAN_LAN3] = nvram_get_int("vlan_tag_lan3");
		vlan_tag[SWAPI_VLAN_RULE_WAN_LAN4] = nvram_get_int("vlan_tag_lan4");
		
		if (is_vlan_vid_valid(vlan_vid[SWAPI_VLAN_RULE_WAN_INET]))
			vlan_tag[SWAPI_VLAN_RULE_WAN_INET] = 1;
		else
			vlan_vid[SWAPI_VLAN_RULE_WAN_INET] = 0;
		
		if (is_vlan_vid_valid(vlan_vid[SWAPI_VLAN_RULE_WAN_IPTV]))
			vlan_tag[SWAPI_VLAN_RULE_WAN_IPTV] = 1;
		else
			vlan_vid[SWAPI_VLAN_RULE_WAN_IPTV] = 0;
	}

	/* set vlan rule before change bridge mode! */
	for (vrule = 0; vrule < SWAPI_VLAN_RULE_NUM; vrule++)
		phy_vlan_rule_set(vrule, vlan_vid[vrule], vlan_pri[vrule], vlan_tag[vrule]);

	phy_bridge_mode(bridge_mode, bwan_isolation);

#if defined(USE_RT3352_MII)
	if (!first_call) {
		// clear isolation iNIC port from all LAN ports
		if (is_interface_up(IFNAME_INIC_MAIN) && get_mlme_radio_rt())
			phy_isolate_inic(0);
	}
#endif
}

void
restart_switch_config_vlan(void)
{
#if !defined (USE_GMAC2_TO_GPHY) && !defined (USE_GMAC2_TO_GSW)
	int pvid_wan = phy_vlan_pvid_wan_get();
#endif

	if (get_ap_mode())
		return;

	notify_reset_detect_link();
	switch_config_vlan(0);

#if !defined (USE_GMAC2_TO_GPHY) && !defined (USE_GMAC2_TO_GSW)
	if (phy_vlan_pvid_wan_get() != pvid_wan)
#endif
		full_restart_wan();
}

int
is_vlan_vid_valid(int vlan_vid)
{
	if (vlan_vid == 2)
		return 1;
	return (vlan_vid >= MIN_EXT_VLAN_VID && vlan_vid < 4095) ? 1 : 0;
}

void
update_ether_leds(void)
{
#if (BOARD_NUM_ETH_LEDS > 1)
	int led0 = nvram_get_int("ether_led0");
	int led1 = nvram_get_int("ether_led1");

	if (!nvram_get_int("led_ether_t")) {
		led0 = SWAPI_LED_OFF;
		led1 = SWAPI_LED_OFF;
	}
#if BOARD_ETH_LED_SWAP
	phy_led_mode_green(led1);
	phy_led_mode_yellow(led0);
#else
	phy_led_mode_green(led0);
	phy_led_mode_yellow(led1);
#endif
#elif (BOARD_NUM_ETH_LEDS == 1)
	int led0 = nvram_get_int("ether_led0");

	if (!nvram_get_int("led_ether_t"))
		led0 = SWAPI_LED_OFF;
#if BOARD_ETH_LED_SWAP
	phy_led_mode_yellow(led0);
#else
	phy_led_mode_green(led0);
#endif
#endif
}

void
reset_lan_temp(void)
{
	if (nvram_match("lan_ipaddr", "")) {
		nvram_set("lan_ipaddr", DEF_LAN_ADDR);
		nvram_set("lan_netmask", DEF_LAN_MASK);
	} else if (nvram_match("lan_netmask", "")) {
		nvram_set("lan_netmask", DEF_LAN_MASK);
	}

	nvram_set_temp("lan_ipaddr_t", nvram_safe_get("lan_ipaddr"));
	nvram_set_temp("lan_netmask_t", nvram_safe_get("lan_netmask"));
	nvram_set_temp("lan_gateway_t", "");
	nvram_set_temp("lan_domain_t", "");
	nvram_set_temp("lan_dns_t", "");
}

void
reset_lan_vars(void)
{
	nvram_set("lan_hwaddr", nvram_safe_get("il0macaddr"));
}

static void
create_hosts_lan(const char *lan_ipaddr, const char *lan_dname)
{
	FILE *fp;
	char *lan_hname = get_our_hostname();

	sethostname(lan_hname, strlen(lan_hname));
	setdomainname(lan_dname, strlen(lan_dname));

	fp = fopen("/etc/hostname", "w+");
	if (fp) {
		fprintf(fp, "%s\n", lan_hname);
		fclose(fp);
	}

	fp = fopen("/etc/hosts", "w+");
	if (fp) {
		fprintf(fp, "%s %s %s\n", "127.0.0.1", "localhost.localdomain", "localhost");
		if (strlen(lan_dname) > 0)
			fprintf(fp, "%s %s.%s %s\n", lan_ipaddr, lan_hname, lan_dname, lan_hname);
		else
			fprintf(fp, "%s %s\n", lan_ipaddr, lan_hname);
		fclose(fp);
	}
}

void
update_hosts_ap(void)
{
	create_hosts_lan(nvram_safe_get("lan_ipaddr_t"), nvram_safe_get("lan_domain_t"));
}

void
start_lan(int is_ap_mode, int do_wait)
{
	char *lan_ipaddr;
	char *lan_netmsk;
	char *lan_ifname = IFNAME_BR;

	lan_ipaddr = nvram_safe_get("lan_ipaddr");
	lan_netmsk = nvram_safe_get("lan_netmask");

	/* bring up and configure LAN interface */
	ifconfig(lan_ifname, IFUP, lan_ipaddr, lan_netmsk);

	/*
	* Configure DHCP connection. The DHCP client will run 
	* 'udhcpc bound'/'udhcpc deconfig' upon finishing IP address 
	* renew and release.
	*/
	if (is_ap_mode) {
		char *lan_dname = nvram_safe_get("lan_domain");
		
		create_hosts_lan(lan_ipaddr, lan_dname);
		
		if (nvram_match("lan_proto_x", "1")) {
			
			symlink("/sbin/rc", SCRIPT_UDHCPC_LAN);
			
			/* early fill XXX_t fields */
			update_lan_status(0);
			
			/* wait PHY ports link ready */
			if (do_wait)
				sleep(1);
			
			/* di wakeup after 60 secs */
			notify_run_detect_internet(60);
			
			/* start dhcp daemon */
			start_udhcpc_lan(lan_ifname);
		} else {
			
			/* manual config lan gateway and dns */
			lan_up_manual(lan_ifname, lan_dname);
			
			/* di wakeup after 2 secs */
			notify_run_detect_internet(2);
		}
	} else {
		
		/* install lan specific static routes */
		add_static_lan_routes(lan_ifname);
		
		/* fill XXX_t fields */
		update_lan_status(0);
	}

#if defined (USE_IPV6)
	if (get_ipv6_type() != IPV6_DISABLED)
		reload_lan_addr6();
#endif

	config_bridge(is_ap_mode);
}

void
stop_lan(int is_ap_mode)
{
	char *svcs[] = { "udhcpc", "detect_wan", NULL };

	if (is_ap_mode) {
		notify_pause_detect_internet();
		
		kill_services(svcs, 3, 1);
	} else {
		char *lan_ip = nvram_safe_get("lan_ipaddr_t");
		
		/* flush conntrack table (only old LAN IP records) */
		if (is_valid_ipv4(lan_ip))
			flush_conntrack_table(lan_ip);
		
		/* Remove static routes */
		clear_if_route4(IFNAME_BR);
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
	int is_wan_err = 0, is_lan_stp = 0;
	int is_ap_mode = get_ap_mode();
	int log_remote = nvram_invmatch("log_ipaddr", "");

	if (!is_ap_mode) {
		is_wan_err = get_wan_unit_value_int(0, "err");
		is_lan_stp = nvram_get_int("lan_stp");
	}

	/* stop logger if remote */
	if (log_remote)
		stop_logger();

	stop_lltd();
	stop_infosvr();
	stop_networkmap();
	stop_upnp();
	stop_vpn_server();
	stop_dns_dhcpd();
	stop_lan(is_ap_mode);

	reset_lan_vars();

	if (!is_ap_mode) {
		br_set_stp(IFNAME_BR, 0);
		br_set_fd(IFNAME_BR, 2);
	}

	/* down and up all LAN ports link */
	phy_ports_lan_power(0);
	sleep(1);
	phy_ports_lan_power(1);

	start_lan(is_ap_mode, 1);

	/* start logger if remote */
	if (log_remote)
		start_logger(0);

#if defined(APP_SMBD)
	/* update SMB fastpath owner address */
	config_smb_fastpath(1);
#endif

	/* restart dns relay and dhcp server */
	start_dns_dhcpd(is_ap_mode);

	if (!is_ap_mode) {
		if (is_lan_stp) {
			br_set_stp(IFNAME_BR, 1);
			br_set_fd(IFNAME_BR, 15);
		}
		
		if (is_wan_err) {
			full_restart_wan();
			start_vpn_server();
		} else {
			/* restart vpn server, firewall and miniupnpd */
			restart_vpn_server();
		}
	}

	/* restart igmpproxy, udpxy, xupnpd */
	if (!is_wan_err)
		restart_iptv(is_ap_mode);

#if defined(APP_NFSD)
	/* reload NFS server exports */
	reload_nfsd();
#endif

#if defined(APP_SMBD) || defined(APP_NMBD)
	reload_nmbd();
#endif

	start_infosvr();
	start_lltd();

	/* start ARP network scanner */
	start_networkmap(1);

	/* force httpd logout */
	doSystem("killall %s %s", "-SIGUSR1", "httpd");
}

void
lan_up_manual(char *lan_ifname, char *lan_dname)
{
	FILE *fp;
	int lock;
	int dns_count = 0;
	char *dns_ip, *gateway_ip;

	gateway_ip = nvram_safe_get("lan_gateway");

	/* Set default route to gateway if specified */
	if (is_valid_ipv4(gateway_ip))
		route_add(lan_ifname, 0, "0.0.0.0", gateway_ip, "0.0.0.0");

	lock = file_lock("resolv");

	/* Open resolv.conf */
	fp = fopen(DNS_RESOLV_CONF, "w+");
	if (fp) {
		if (strlen(lan_dname) > 0)
			fprintf(fp, "domain %s\n", lan_dname);
		
		dns_ip = nvram_safe_get("lan_dns1");
		if (is_valid_ipv4(dns_ip)) {
			fprintf(fp, "nameserver %s\n", dns_ip);
			dns_count++;
		}
		
		dns_ip = nvram_safe_get("lan_dns2");
		if (is_valid_ipv4(dns_ip)) {
			fprintf(fp, "nameserver %s\n", dns_ip);
			dns_count++;
		}
		
		if (!dns_count && is_valid_ipv4(gateway_ip))
			fprintf(fp, "nameserver %s\n", gateway_ip);
		
		fclose(fp);
	}

	file_unlock(lock);

	/* sync time */
	notify_watchdog_time();

	/* fill XXX_t fields */
	update_lan_status(0);
}

static void
lan_up_auto(char *lan_ifname, char *lan_gateway, char *lan_dname)
{
	FILE *fp;
	int dns_count = 0;
	char word[100], *next, *dns_ip;

	/* Set default route to gateway if specified */
	if (is_valid_ipv4(lan_gateway))
		route_add(lan_ifname, 0, "0.0.0.0", lan_gateway, "0.0.0.0");

	/* Open resolv.conf */
	fp = fopen(DNS_RESOLV_CONF, "w+");
	if (fp) {
		if (strlen(lan_dname) > 0)
			fprintf(fp, "domain %s\n", lan_dname);
		
		if (nvram_get_int("lan_dns_x") == 0) {
			dns_ip = nvram_safe_get("lan_dns1");
			if (is_valid_ipv4(dns_ip)) {
				fprintf(fp, "nameserver %s\n", dns_ip);
				dns_count++;
			}
			
			dns_ip = nvram_safe_get("lan_dns2");
			if (is_valid_ipv4(dns_ip)) {
				fprintf(fp, "nameserver %s\n", dns_ip);
				dns_count++;
			}
		} else {
			foreach(word, nvram_safe_get("lan_dns_t"), next) {
				if (is_valid_ipv4(word)) {
					fprintf(fp, "nameserver %s\n", word);
					dns_count++;
				}
			}
		}
		
		if (!dns_count && is_valid_ipv4(lan_gateway))
			fprintf(fp, "nameserver %s\n", lan_gateway);
		
		fclose(fp);
	}

	/* sync time */
	notify_watchdog_time();

	/* fill XXX_t fields */
	update_lan_status(1);

#if defined(APP_SMBD)
	/* update SMB fastpath owner address */
	config_smb_fastpath(1);
#endif

	/* di wakeup after 2 secs */
	notify_run_detect_internet(2);
}

static void
lan_down_auto(char *lan_ifname)
{
	FILE *fp;
	char *lan_gateway = nvram_safe_get("lan_gateway_t");

	notify_pause_detect_internet();

	/* Remove default route to gateway if specified */
	if (is_valid_ipv4(lan_gateway))
		route_del(lan_ifname, 0, "0.0.0.0", lan_gateway, "0.0.0.0");

	/* Clear resolv.conf */
	fp = fopen(DNS_RESOLV_CONF, "w+");
	if (fp)
		fclose(fp);

	/* fill XXX_t fields */
	update_lan_status(0);

#if defined(APP_SMBD)
	/* update SMB fastpath owner address */
	config_smb_fastpath(1);
#endif
}

void 
update_lan_status(int is_auto)
{
	if (!is_auto) {
		nvram_set_temp("lan_ipaddr_t", nvram_safe_get("lan_ipaddr"));
		nvram_set_temp("lan_netmask_t", nvram_safe_get("lan_netmask"));
		nvram_set_temp("lan_domain_t", nvram_safe_get("lan_domain"));
		
		if (!get_ap_mode()) {
			if (is_dhcpd_enabled(0)) {
				if (nvram_invmatch("dhcp_gateway_x", ""))
					nvram_set_temp("lan_gateway_t", nvram_safe_get("dhcp_gateway_x"));
				else
					nvram_set_temp("lan_gateway_t", nvram_safe_get("lan_ipaddr"));
			} else
				nvram_set_temp("lan_gateway_t", nvram_safe_get("lan_ipaddr"));
		}
		else
			nvram_set_temp("lan_gateway_t", nvram_safe_get("lan_gateway"));
	}
}

static int 
udhcpc_lan_deconfig(char *lan_ifname)
{
	ifconfig(lan_ifname, IFUP,
		 nvram_safe_get("lan_ipaddr"),
		 nvram_safe_get("lan_netmask"));

	lan_down_auto(lan_ifname);

	logmessage("DHCP LAN Client", "%s: lease is lost", udhcpc_lan_state);

	return 0;
}

static int 
udhcpc_lan_bound(char *lan_ifname, int is_renew)
{
	char *param, *value;
	char tmp[100], prefix[16];
	int is_changed = 0, ip_changed = 0, lease_dur = 0;

	snprintf(prefix, sizeof(prefix), "lan_");

	if ((value = getenv("ip"))) {
		param = strcat_r(prefix, "ipaddr_t", tmp);
		is_changed |= nvram_invmatch(param, value);
		ip_changed |= is_changed;
		nvram_set_temp(param, value);
	}
	if ((value = getenv("subnet"))) {
		param = strcat_r(prefix, "netmask_t", tmp);
		is_changed |= nvram_invmatch(param, value);
		nvram_set_temp(param, value);
	} else {
		is_changed |= 1;
	}
	if ((value = getenv("router"))) {
		param = strcat_r(prefix, "gateway_t", tmp);
		is_changed |= nvram_invmatch(param, value);
		nvram_set_temp(param, value);
	}
	if ((value = getenv("dns"))) {
		param = strcat_r(prefix, "dns_t", tmp);
		is_changed |= nvram_invmatch(param, value);
		nvram_set_temp(param, value);
	}
	if ((value = getenv("domain"))) {
		param = strcat_r(prefix, "domain_t", tmp);
		is_changed |= nvram_invmatch(param, value);
		nvram_set_temp(param, value);
	}
	if ((value = getenv("wins")))
		nvram_set_temp(strcat_r(prefix, "wins_t", tmp), value);
	if ((value = getenv("lease"))) {
		lease_dur = atoi(value);
		nvram_set_temp(strcat_r(prefix, "lease_t", tmp), value);
	}

	if (is_changed || !is_renew) {
		char *lan_ipaddr  = nvram_safe_get(strcat_r(prefix, "ipaddr_t", tmp));
		char *lan_ipmask  = nvram_safe_get(strcat_r(prefix, "netmask_t", tmp));
		char *lan_gateway = nvram_safe_get(strcat_r(prefix, "gateway_t", tmp));
		char *lan_domain  = nvram_safe_get(strcat_r(prefix, "domain_t", tmp));
		
		if (ip_changed)
			ifconfig(lan_ifname, IFUP, "0.0.0.0", NULL);
		
		ifconfig(lan_ifname, IFUP, lan_ipaddr, lan_ipmask);
		
		create_hosts_lan(lan_ipaddr, lan_domain);
		
		lan_up_auto(lan_ifname, lan_gateway, lan_domain);
		
		logmessage("DHCP LAN Client", "%s, IP: %s/%s, GW: %s, lease time: %d",
			udhcpc_lan_state, lan_ipaddr, lan_ipmask, lan_gateway, lease_dur);
	}

	if (!is_renew)
		restart_networkmap();

	return 0;
}

static int 
udhcpc_lan_leasefail(char *lan_ifname)
{
	return 0;
}

static int 
udhcpc_lan_noack(char *lan_ifname)
{
	logmessage("DHCP LAN Client", "Received NAK for %s", lan_ifname);
	return 0;
}

int
udhcpc_lan_main(int argc, char **argv)
{
	int ret = 0;
	char *lan_ifname;

	if (argc<2 || !argv[1])
		return EINVAL;

	lan_ifname = safe_getenv("interface");
	snprintf(udhcpc_lan_state, sizeof(udhcpc_lan_state), "%s", argv[1]);

	umask(0000);

	if (!strcmp(argv[1], "deconfig"))
		ret = udhcpc_lan_deconfig(lan_ifname);
	else if (!strcmp(argv[1], "bound"))
		ret = udhcpc_lan_bound(lan_ifname, 0);
	else if (!strcmp(argv[1], "renew"))
		ret = udhcpc_lan_bound(lan_ifname, 1);
	else if (!strcmp(argv[1], "leasefail"))
		ret = udhcpc_lan_leasefail(lan_ifname);
	else if (!strcmp(argv[1], "nak"))
		ret = udhcpc_lan_noack(lan_ifname);

	return ret;
}

int 
start_udhcpc_lan(char *lan_ifname)
{
	char *lan_hostname = get_our_hostname();
	char *dhcp_argv[] = {
		"/sbin/udhcpc",
		"-i", lan_ifname,
		"-s", SCRIPT_UDHCPC_LAN,
		"-p", "/var/run/udhcpc_lan.pid",
		"-t4",
		"-T4",
		"-d", /* Background after run (new patch for udhcpc) */
		"-H", lan_hostname,
		NULL
	};
	
	logmessage("DHCP LAN Client", "starting on %s ...", lan_ifname);
	
	return _eval(dhcp_argv, NULL, 0, NULL);
}

int 
stop_udhcpc_lan()
{
	return kill_pidfile("/var/run/udhcpc_lan.pid");
}
