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
/*
 * Router default NVRAM values
 *
 * Copyright 2001-2003, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: defaults.c,v 1.2 2007/06/21 04:55:53 jordan_hsiao Exp $
 */

#include <nvram/bcmnvram.h>

struct nvram_tuple router_defaults[] = {
	/* Restore defaults */
	{ "restore_defaults", "0", 0 },		/* Set to 0 to not restore defaults on boot */

	/* Miscellaneous parameters */
	{ "time_zone", "GMT0", 0 },
	{ "log_level", "0", 0 },		/* Bitmask 0:off 1:denied 2:accepted */
	{ "upnp_enable", "1", 0 },		/* Start UPnP */
	{ "stats_server", "", 0 },		/* URL for posting stats */
	{ "console_loglevel", "7", 0 },		/* Kernel panics only */

	/* Big switches */
	{ "router_disable", "0", 0 },		/* lan_proto=static lan_stp=0 wan_proto=disabled */
#if defined (W7_LOGO) || defined (WIFI_LOGO)
	{ "fw_disable", "1", 0 },		/* Disable firewall (allow new connections from the WAN) */
	{ "fw_enable_x", "0", 0},
#else
	{ "fw_disable", "0", 0 },		// win7 logo
	{ "fw_enable_x", "1", 0},		// win7 logo
#endif
	{ "log_ipaddr", "", 0 },		/* syslog recipient */

	/* LAN H/W parameters */
	{ "lan_ifname", "br0", 0 },		/* LAN interface name */
	{ "lan_ifnames", "br0", 0 },		/* Enslaved LAN interfaces */
	{ "lan_hwaddr", "", 0 },		/* LAN interface MAC address */

	/* LAN TCP/IP parameters */
	{ "lan_dhcp", "0", 0 },			/* DHCP client [static|dhcp] */
#ifndef W7_LOGO
	{ "lan_ipaddr", "192.168.1.1", 0 },	/* LAN IP address */
#else
	{ "lan_ipaddr", "192.168.0.1", 0 },	// win7 logo
#endif
	{ "lan_netmask", "255.255.255.0", 0 },	/* LAN netmask */
#ifndef W7_LOGO
	{ "lan_gateway", "192.168.1.1", 0 },	/* LAN gateway */
#else
	{ "lan_gateway", "192.168.0.1", 0 },	// win7 logo
#endif
	{ "lan_proto", "dhcp", 0 },		/* DHCP server [static|dhcp] */
	{ "lan_wins", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "lan_domain", "", 0 },		/* LAN domain name */
	{ "lan_lease", "86400", 0 },		/* LAN lease time in seconds */
	{ "lan_stp", "1", 0 },			/* LAN spanning tree protocol */
	{ "lan_route", "", 0 },			/* Static routes (ipaddr:netmask:gateway:metric:ifname ...) */

	/* WAN H/W parameters */
	{ "wan_ifname", "eth3", 0 },		/* WAN interface name */
	{ "wan_ifnames", "eth3", 0 },		/* WAN interface names */
	{ "wan_hwname", "", 0 },		/* WAN driver name (e.g. et1) */
	{ "wan_hwaddr", "", 0 },		/* WAN interface MAC address */

	/* WAN TCP/IP parameters */
#if defined (W7_LOGO) || defined (WIFI_LOGO)
	{ "wan_proto", "static", 0 },		// win7 logo
	{ "wan_ipaddr", "17.1.1.1", 0 },	// win7 logo
	{ "wan_netmask", "255.255.255.0", 0 },	// win7 logo
	{ "wan_gateway", "17.1.1.1", 0 },	// win7 logo
#else
	{ "wan_proto", "dhcp", 0 },		/* [static|dhcp|pppoe|disabled] */
	{ "wan_ipaddr", "0.0.0.0", 0 },		/* WAN IP address */
	{ "wan_netmask", "0.0.0.0", 0 },	/* WAN netmask */
	{ "wan_gateway", "0.0.0.0", 0 },	/* WAN gateway */
#endif
	{ "wan_dns", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "wan_wins", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "wan_hostname", "", 0 },		/* WAN hostname */
	{ "wan_domain", "", 0 },		/* WAN domain name */
	{ "wan_lease", "86400", 0 },		/* WAN lease time in seconds */

	/* PPPoE parameters */
	{ "wan_pppoe_ifname", "ppp0", 0 },	/* PPPoE enslaved interface */
	{ "wan_pppoe_username", "", 0 },	/* PPP username */
	{ "wan_pppoe_passwd", "", 0 },		/* PPP password */
	{ "wan_pppoe_idletime", "0", 0 },	// oleg patch
	{ "wan_pppoe_keepalive", "0", 0 },	/* Restore link automatically */
	{ "wan_pppoe_demand", "0", 0 },		/* Dial on demand */
	{ "wan_pppoe_mru", "1492", 0 },		/* Negotiate MRU to this value */
	{ "wan_pppoe_mtu", "1492", 0 },		/* Negotiate MTU to the smaller of this value or the peer MRU */
	{ "wan_pppoe_service", "", 0 },		/* PPPoE service name */
	{ "wan_pppoe_ac", "", 0 },		/* PPPoE access concentrator name */
	{ "wan_pppoe_lcpa", "1", 0 },		/* Adaptive LCP Echo */
	{ "wan_pppoe_cpul", "0", 0 },		/* Prevent drop PPP session at high CPU load */

	/* Misc WAN parameters */
	{ "wan_desc", "", 0 },			/* WAN connection description */
	{ "wan_route", "", 0 },			/* Static routes (ipaddr:netmask:gateway:metric:ifname ...) */
	{ "wan_primary", "0", 0 },		/* Primary wan connection */
	{ "wan_unit", "0", 0 },			/* Last configured connection */

	/* Filters */
	{ "filter_maclist", "", 0 },		/* xx:xx:xx:xx:xx:xx ... */
	{ "filter_macmode", "deny", 0 },	/* "allow" only, "deny" only, or "disabled" (allow all) */
	{ "filter_client0", "", 0 },		/* [lan_ipaddr0-lan_ipaddr1|*]:lan_port0-lan_port1,
						 * proto,enable,day_start-day_end,sec_start-sec_end,desc */

	/* Port forwards */
	{ "dmz_ip", "", 0 },			/* x.x.x.x (equivalent to 0-60999>dmz_ipaddr: 0-60999) */
	{ "forward_port0", "", 0 },		/* wan_port0-wan_port1>lan_ipaddr: lan_port0-lan_port1[:,]proto[:,]enable[:,]desc */
	{ "autofw_port0", "", 0 },		/* out_proto:out_port,in_proto:in_port0-in_port1>to_port0-to_port1,enable,desc */

	/* DHCP server parameters */
#ifndef W7_LOGO
	{ "dhcp_start", "192.168.1.2", 0 },	/* First assignable DHCP address */
#else
	{ "dhcp_start", "192.168.0.2", 0 },	// win7 logo
#endif
#ifndef W7_LOGO
	{ "dhcp_end", "192.168.1.254", 0 },	/* Last assignable DHCP address */
#else
	{ "dhcp_end", "192.168.0.254", 0 },	// win7 logo
#endif
	{ "dhcp_domain", "wan", 0 },		/* Use WAN domain name first if available (wan|lan) */
	{ "dhcp_wins", "wan", 0 },		/* Use WAN WINS first if available (wan|lan) */

	/* Web server parameters */
	{ "http_username", "admin", 0 },	/* Username */
	{ "http_passwd", "admin", 0 },		/* Password */
	{ "http_wanport", "", 0 },		/* WAN port to listen on */
	{ "http_lanport", "80", 0 },		/* LAN port to listen on */

	{ "httpd_check_ddns", "0", 0 },
	{ "pppd_way", "2", 0 },

	{ "wan_stb_x", "0", 0 },		// oleg patch
	{ "wan_pppoe_options_x", "", 0 },	// oleg patch
	{ "wan_pptp_options_x", "", 0 },	// oleg patch
	{ "fw_dos_x", "0", 0 },			// oleg patch
	{ "dr_enable_x", "1", 0 },		// oleg patch
	{ "mr_enable_x", "0", 0 },		// oleg patch

	/* Wireless parameters */
	{ "wl_country_code", "GB", 0 },		/* Country Code (default obtained from driver) */
	{ "wl_ssid", "ASUS_5G", 0 },		/* Service set ID (network name) */
	{ "wl_gmode", "2", 0 },			/* 54g mode */
	{ "wl_channel", "0", 0 },		/* Channel number */
	{ "wl_rateset", "default", 0 },		/* "default" or "all" or "12" */
	{ "wl_bcn", "100", 0 },			/* Beacon interval */
	{ "wl_dtim", "1", 0 },			/* DTIM period */
	{ "wl_gmode_protection", "auto", 0 },	/* 802.11g RTS/CTS protection (off|auto) */
	{ "wl_rts", "2347", 0 },		/* RTS threshold */
	{ "wl_frag", "2346", 0 },		/* Fragmentation threshold */
	{ "wl_ap_isolate", "0", 0 },		/* AP isolate mode */
	{ "wl_mbssid_isolate", "1", 0 },	/* Isolate between AP and Guests AP */
	{ "wl_closed", "0", 0 },		/* Closed (hidden) network */
	{ "wl_macmode", "disabled", 0 },	/* "allow" only, "deny" only, or "disabled"(allow all) */
	{ "wl_maclist", "", 0 },		/* xx:xx:xx:xx:xx:xx ... */
	{ "wl_wme", "1", 0 },			/* WME mode (off|on) */
	{ "wl_wme_no_ack", "off", 0 },		/* WME No-Acknowledgment mode */
	{ "wl_auth_mode", "open", 0 },		/* Network authentication mode Open System */
	{ "wl_key", "1", 0 },			/* Current WEP key */
	{ "wl_key1", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key2", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key3", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key4", "", 0 },			/* 5/13 char ASCII or 10/26 char hex */
	{ "wl_key_type", "0", 0 } ,		/* WEP key format (HEX/ASCII)*/
	{ "wl_mcastrate", "1", 0 },		/* Mcast Rate (bps) */
	{ "wl_mode", "ap", 0 },			/* AP mode (ap|sta|wds) */

	/* WPA parameters */
	{ "wl_crypto", "aes", 0 },		/* WPA data encryption */
	{ "wl_wpa_psk", "", 0 },		/* WPA pre-shared key */
	{ "wl_wpa_gtk_rekey", "0", 0 },		/* GTK rotation interval */

	{ "wl_radius_ipaddr", "", 0 },		/* RADIUS server IP address */
	{ "wl_radius_port", "1812", 0 },	/* RADIUS server UDP port */
	{ "wl_radius_key", "", 0 },		/* RADIUS shared secret */

	{ "wl_lazywds", "0", 0 },		/* Enable "lazy" WDS mode (0|1) */
	{ "wl_radio_x", "1", 0 },		/* Enable (1) or disable (0) radio */

	{ "wl_IgmpSnEnable", "1", 0 },
	{ "wl_TxPower", "100", 0 },
	{ "wl_TxBurst", "1", 0 },
	{ "wl_PktAggregate", "1", 0 },
	{ "wl_APSDCapable", "1", 0 },
	{ "wl_DLSCapable", "0", 0 },
	{ "wl_HT_OpMode", "0", 0 },
	{ "wl_HT_BW", "1", 0 },

	{ "wl_txbf", "0", 0 },

	/* Wireless parameters  for RT3092 */
	{ "rt_TxPower", "100", 0 },
	{ "rt_TxBurst", "1", 0 },
	{ "rt_PktAggregate", "1", 0 },

	{ "rt_country_code", "GB", 0 },
	{ "rt_ssid", "ASUS", 0 },
	{ "rt_gmode", "2", 0 },
	{ "rt_channel", "0", 0 },
	{ "rt_rateset", "default", 0 },
	{ "rt_bcn", "100", 0 },
	{ "rt_dtim", "1", 0 },
	{ "rt_gmode_protection", "auto", 0 },
	{ "rt_rts", "2347", 0 },
	{ "rt_frag", "2346", 0 },
	{ "rt_ap_isolate", "0", 0 },
	{ "rt_mbssid_isolate", "1", 0 },	/* Isolate between AP and Guests AP */
	{ "rt_closed", "0", 0 },
	{ "rt_macmode", "disabled", 0 },
	{ "rt_mcastrate", "1", 0 },
	{ "rt_mode", "ap", 0 },

	{ "rt_HT_BW", "1", 0 },
	{ "rt_HT_EXTCHA", "1", 0 },
	{ "rt_HT_OpMode", "0", 0 },
	{ "rt_wsc_config_state", "0", 0 },
	{ "rt_secret_code", "0", 0 },

	{ "rt_DLSCapable", "0", 0 },
	{ "rt_APSDCapable", "1", 0 },
	{ "rt_wme", "1", 0 },
	{ "rt_wme_no_ack", "off", 0 },

	{ "rt_IgmpSnEnable", "1", 0 },

	{ "rt_auth_mode", "open", 0 },
	{ "rt_crypto", "aes", 0 },
	{ "rt_wpa_psk", "", 0 },
	{ "rt_wpa_gtk_rekey", "0", 0 },
	{ "rt_key", "1", 0 },
	{ "rt_key_type", "0", 0 } ,
	{ "rt_key1", "", 0 },
	{ "rt_key2", "", 0 },
	{ "rt_key3", "", 0 },
	{ "rt_key4", "", 0 },
	{ "rt_lazywds", "0", 0 },
	{ "rt_radius_ipaddr", "", 0 },
	{ "rt_radius_port", "1812", 0 },
	{ "rt_radius_key", "", 0 },
	{ "rt_radio_x", "1", 0 },

	{ "rt_ssid2", "ASUS", 0 },
	{ "rt_mode_x", "0", 0 },
	{ "rt_wdsapply_x", "0", 0 },
	{ "rt_wdsnum_x", "0", 0 },
	{ "rt_wep_x", "0", 0 },
	{ "rt_phrase_x", "", 0 },
	{ "rt_radio_date_x", "1111111", 0 },
	{ "rt_radio_time_x", "00002359", 0 },
	{ "rt_macapply_x", "Both", 0 },
	{ "rt_macnum_x", "0", 0 },
	{ "rt_wdslist_x", "", 0 },
	{ "rt_maclist_x", "", 0 },
//	{ "rt_gmode_protection_x", "1", 0 },
	{ "rt_wpa_mode", "0", 0 },

	{ "ehci_ports", "1-1 1-2", 0 },
	{ "ohci_ports", "2-1 2-2", 0 },

	#include "./flash.default"
	{ 0, 0, 0 }
};
