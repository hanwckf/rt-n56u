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
#include <boards.h>

#define STR1(x) #x
#define STR(x) STR1(x)

struct nvram_tuple router_defaults[] = {
	/* Restore defaults */
	{ "restore_defaults", "0", 0 },		/* Set to 0 to not restore defaults on boot */
	{ "nvram_manual", "0", 0 },

	/* Miscellaneous parameters */
	{ "time_zone", "GMT0", 0 },
	{ "log_level", "0", 0 },		/* Bitmask 0:off 1:denied 2:accepted */
	{ "upnp_enable", "1", 0 },		/* Start UPnP */
	{ "stats_server", "", 0 },		/* URL for posting stats */
	{ "console_loglevel", "7", 0 },		/* Kernel panics only */

	/* Big switches */
	{ "router_disable", "0", 0 },		/* lan_proto=static lan_stp=0 wan_proto=disabled */
	{ "fw_enable_x", "1", 0},		// win7 logo
	{ "log_ipaddr", "", 0 },		/* syslog recipient */

	/* LAN H/W parameters */
	{ "lan_ifname", "br0", 0 },		/* LAN interface name */
	{ "lan_hwaddr", "", 0 },		/* LAN interface MAC address */

	/* LAN TCP/IP parameters */
	{ "lan_proto_x", "0", 0 },		/* DHCP client [static|dhcp] */
	{ "lan_ipaddr", "192.168.1.1", 0 },	/* LAN IP address */
	{ "lan_netmask", "255.255.255.0", 0 },	/* LAN netmask */
	{ "lan_gateway", "192.168.1.1", 0 },	/* LAN gateway */
	{ "lan_dns_x", "1", 0 },		/* LAN DNS [static|dhcp] */
	{ "lan_dns1", "", 0 },			/* LAN DNS1 */
	{ "lan_dns2", "", 0 },			/* LAN DNS2 */
	{ "lan_domain", "", 0 },		/* LAN domain name */
	{ "lan_stp", "1", 0 },			/* LAN spanning tree protocol */

	/* WAN H/W parameters */
	{ "wan_ifname", "eth3", 0 },		/* WAN interface name */
	{ "wan_hwname", "", 0 },		/* WAN driver name (e.g. et1) */
	{ "wan_hwaddr", "", 0 },		/* WAN interface MAC address */

	/* WAN TCP/IP parameters */
	{ "wan_proto", "dhcp", 0 },		/* [static|dhcp|pppoe|disabled] */
	{ "wan_ipaddr", "0.0.0.0", 0 },		/* WAN IP address */
	{ "wan_netmask", "0.0.0.0", 0 },	/* WAN netmask */
	{ "wan_gateway", "0.0.0.0", 0 },	/* WAN gateway */
	{ "wan_dns", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "wan_wins", "", 0 },			/* x.x.x.x x.x.x.x ... */
	{ "wan_hostname", "", 0 },		/* WAN hostname */
	{ "wan_domain", "", 0 },		/* WAN domain name */
	{ "wan_lease", "86400", 0 },		/* WAN lease time in seconds */

	/* PPPoE parameters */
	{ "wan_pppoe_ifname", "ppp0", 0 },	/* PPPoE enslaved interface */
	{ "wan_pppoe_username", "", 0 },	/* PPP username */
	{ "wan_pppoe_passwd", "", 0 },		/* PPP password */
	{ "wan_pppoe_auth", "0", 0 },		/* PPP authentication */
	{ "wan_pppoe_idletime", "0", 0 },	// oleg patch
	{ "wan_pppoe_demand", "0", 0 },		/* Dial on demand */
	{ "wan_pppoe_service", "", 0 },		/* PPPoE service name */
	{ "wan_pppoe_ac", "", 0 },		/* PPPoE access concentrator name */
	{ "wan_pppoe_lcpa", "0", 0 },		/* Adaptive LCP Echo */
	{ "wan_pppoe_mtu", "1492", 0 },		/* Negotiate MTU to the smaller of this value or the peer MRU */
	{ "wan_pppoe_mru", "1492", 0 },		/* Negotiate MRU to this value */
	{ "wan_pptp_mtu", "1400", 0 },		/* Negotiate MTU to the smaller of this value or the peer MRU */
	{ "wan_pptp_mru", "1500", 0 },		/* Negotiate MRU to this value */
	{ "wan_l2tp_mtu", "1460", 0 },		/* Negotiate MTU to the smaller of this value or the peer MRU */
	{ "wan_l2tp_mru", "1500", 0 },		/* Negotiate MRU to this value */

	/* Misc WAN parameters */
	{ "wan_desc", "", 0 },			/* WAN connection description */
	{ "wan_primary", "0", 0 },		/* Primary wan connection */
	{ "wan_unit", "0", 0 },			/* Last configured connection */

	/* Exposed station */
	{ "dmz_ip", "", 0 },			/* x.x.x.x (equivalent to 0-60999>dmz_ipaddr: 0-60999) */

	/* DHCP server parameters */
	{ "dhcp_start", "192.168.1.2", 0 },	/* First assignable DHCP address */
	{ "dhcp_end", "192.168.1.244", 0 },	/* Last assignable DHCP address */
	{ "dhcp_domain", "wan", 0 },		/* Use WAN domain name first if available (wan|lan) */
	{ "dhcp_wins", "wan", 0 },		/* Use WAN WINS first if available (wan|lan) */

	/* Web server parameters */
	{ "http_username", "admin", 0 },	/* Username */
	{ "http_passwd", "admin", 0 },		/* Password */
	{ "http_lanport", "80", 0 },		/* LAN port to listen on */

	{ "wan_stb_x", "0", 0 },		// oleg patch
	{ "wan_pppoe_options_x", "", 0 },	// oleg patch
	{ "wan_pptp_options_x", "", 0 },	// oleg patch
	{ "fw_dos_x", "0", 0 },			// oleg patch
	{ "dr_enable_x", "1", 0 },		// oleg patch
	{ "mr_enable_x", "0", 0 },		// oleg patch
	{ "mr_ttl_fix", "0", 0 },

	/* Wireless parameters */
	{ "wl_country_code", "GB", 0 },		/* Country Code (default obtained from driver) */
	{ "wl_ssid", "ASUS_5G", 0 },		/* Service set ID (network name) */
	{ "wl_gmode", "2", 0 },			/* 54g mode */
	{ "wl_channel", "0", 0 },		/* Channel number */
	{ "wl_rateset", "default", 0 },		/* "default" or "all" or "12" */
	{ "wl_bcn", "100", 0 },			/* Beacon interval */
	{ "wl_dtim", "1", 0 },			/* DTIM period */
	{ "wl_gmode_protection", "off", 0 },	/* 802.11g RTS/CTS protection (off|auto) */
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
	{ "wl_crypto", "aes", 0 },		/* WPA data encryption */
	{ "wl_wpa_psk", "", 0 },		/* WPA pre-shared key */
	{ "wl_wpa_gtk_rekey", "3600", 0 },		/* GTK rotation interval */
	{ "wl_radius_ipaddr", "", 0 },		/* RADIUS server IP address */
	{ "wl_radius_port", "1812", 0 },	/* RADIUS server UDP port */
	{ "wl_radius_key", "", 0 },		/* RADIUS shared secret */
	{ "wl_lazywds", "0", 0 },		/* Enable "lazy" WDS mode (0|1) */
	{ "wl_radio_x", "1", 0 },		/* Enable (1) or disable (0) radio */
	{ "wl_IgmpSnEnable", "1", 0 },
	{ "wl_TxPower", "100", 0 },
	{ "wl_TxBurst", "1", 0 },
	{ "wl_PktAggregate", "1", 0 },
	{ "wl_APSDCapable", "0", 0 },
	{ "wl_DLSCapable", "0", 0 },
	{ "wl_HT_OpMode", "0", 0 },
	{ "wl_HT_BW", "1", 0 },
	{ "wl_txbf", "0", 0 },
	{ "wl_ssid2", "ASUS_5G", 0 },
	{ "wl_mode_x", "0", 0 },
	{ "wl_wdsapply_x", "0", 0 },
	{ "wl_wdsnum_x", "0", 0 },
	{ "wl_wep_x", "0", 0 },
	{ "wl_phrase_x", "", 0 },
	{ "wl_radio_date_x", "1111111", 0 },
	{ "wl_radio_time_x", "00002359", 0 },
	{ "wl_radio_time2_x", "00002359", 0 },
	{ "wl_macnum_x", "0", 0 },
	{ "wl_wdslist_x", "", 0 },
	{ "wl_maclist_x", "", 0 },
	{ "wl_wpa_mode", "0", 0 },
	{ "wl_stream_tx", STR(RT3883_RF_TX), 0 },
	{ "wl_stream_rx", STR(RT3883_RF_RX), 0 },
	{ "wl_preamble", "0", 0 },
	{ "wl_greenap", "0", 0 },
	{ "wl_HT_RDG", "1", 0 },
	{ "wl_HT_AMSDU", "0", 0 },
	{ "wl_HT_MpduDensity", "5", 0 },
	{ "wl_HT_BAWinSize", "64", 0 },
	{ "wl_HT_AutoBA", "1", 0 },

	// guest AP 5Ghz
	{ "wl_guest_enable", "0", 0 },
	{ "wl_guest_date_x", "1111111", 0 },
	{ "wl_guest_time_x", "00002359", 0 },
	{ "wl_guest_time2_x", "00002359", 0 },
	{ "wl_guest_ssid", "ASUS_GUEST_5G", 0 },
	{ "wl_guest_closed", "0", 0 },
	{ "wl_guest_ap_isolate", "1", 0 },
	{ "wl_guest_lan_isolate", "1", 0 },
	{ "wl_guest_auth_mode", "open", 0 },
	{ "wl_guest_wpa_mode", "2", 0 },
	{ "wl_guest_crypto", "aes", 0 },
	{ "wl_guest_wpa_psk", "", 0 },
	{ "wl_guest_macrule", "0", 0 },

	// ApCli 5Ghz
	{ "wl_sta_ssid", "", 0 },
	{ "wl_sta_auth_mode", "open", 0 },
	{ "wl_sta_wpa_mode", "2", 0 },
	{ "wl_sta_crypto", "aes", 0 },
	{ "wl_sta_wpa_psk", "", 0 },

	/* Wireless parameters  for RT3092/RT3352 */
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
	{ "rt_mcastrate", "6", 0 },
	{ "rt_mode", "ap", 0 },
	{ "rt_HT_BW", "1", 0 },
	{ "rt_HT_EXTCHA", "1", 0 },
	{ "rt_HT_OpMode", "0", 0 },
	{ "rt_wsc_config_state", "0", 0 },
	{ "rt_secret_code", "0", 0 },
	{ "rt_DLSCapable", "0", 0 },
	{ "rt_APSDCapable", "0", 0 },
	{ "rt_wme", "1", 0 },
	{ "rt_wme_no_ack", "off", 0 },
	{ "rt_IgmpSnEnable", "1", 0 },
	{ "rt_auth_mode", "open", 0 },
	{ "rt_crypto", "aes", 0 },
	{ "rt_wpa_psk", "", 0 },
	{ "rt_wpa_gtk_rekey", "3600", 0 },
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
	{ "rt_radio_time2_x", "00002359", 0 },
	{ "rt_macnum_x", "0", 0 },
	{ "rt_wdslist_x", "", 0 },
	{ "rt_maclist_x", "", 0 },
	{ "rt_wpa_mode", "0", 0 },
	{ "rt_stream_tx", STR(INIC_RF_TX), 0 },
	{ "rt_stream_rx", STR(INIC_RF_RX), 0 },
	{ "rt_preamble", "0", 0 },
	{ "rt_greenap", "0", 0 },
	{ "rt_HT_RDG", "0", 0 },
	{ "rt_HT_AMSDU", "0", 0 },
	{ "rt_HT_MpduDensity", "5", 0 },
	{ "rt_HT_BAWinSize", "64", 0 },
	{ "rt_HT_AutoBA", "1", 0 },

	// guest AP 2.4Ghz
	{ "rt_guest_enable", "0", 0 },
	{ "rt_guest_date_x", "1111111", 0 },
	{ "rt_guest_time_x", "00002359", 0 },
	{ "rt_guest_time2_x", "00002359", 0 },
	{ "rt_guest_ssid", "ASUS_GUEST", 0 },
	{ "rt_guest_closed", "0", 0 },
	{ "rt_guest_ap_isolate", "1", 0 },
	{ "rt_guest_lan_isolate", "1", 0 },
	{ "rt_guest_auth_mode", "open", 0 },
	{ "rt_guest_wpa_mode", "2", 0 },
	{ "rt_guest_crypto", "aes", 0 },
	{ "rt_guest_wpa_psk", "", 0 },
	{ "rt_guest_macrule", "0", 0 },

	// ApCli 2.4Ghz
	{ "rt_sta_ssid", "", 0 },
	{ "rt_sta_auth_mode", "open", 0 },
	{ "rt_sta_wpa_mode", "2", 0 },
	{ "rt_sta_crypto", "aes", 0 },
	{ "rt_sta_wpa_psk", "", 0 },

	{ "acc_num", "0", 0 },
	{ "enable_ftp", "0", 0 },
	{ "enable_samba", "1", 0 },
	{ "st_samba_mode", "1", 0 },
	{ "st_samba_lmb", "1", 0 },
	{ "st_ftp_mode", "1", 0 },
	{ "st_ftp_log", "0", 0 },
	{ "st_max_user", "10", 0 },
	{ "st_samba_workgroup", "WORKGROUP", 0 },
	{ "st_samba_workgroupb", "WORKGROUP", 0 },	// 2008.03 James.
	{ "apps_dms", "0", 0 },
	{ "apps_itunes", "0", 0},
	{ "sh_num", "0", 0 },
	{ "machine_name", BOARD_NAME, 0 },
	{ "computer_name", BOARD_NAME, 0 },
	{ "computer_nameb", BOARD_NAME, 0 },

	{ "pcache_reclaim", "0", 0 },

	{ "ehci_ports", "1-1 1-2", 0 },
	{ "ohci_ports", "2-1 2-2", 0 },

#if defined (USE_IPV6)
	{ "ip6_service", "", 0 },
	{ "ip6_ppe_on", "0", 0 },
	{ "ip6_wan_if", "0", 0 },

	{ "ip6_6in4_remote", "", 0 },
	{ "ip6_6to4_relay", "192.88.99.1", 0 },
	{ "ip6_6rd_relay", "", 0 },
	{ "ip6_6rd_size", "0", 0 },
	{ "ip6_6rd_dhcp", "1", 0 },
	{ "ip6_sit_mtu", "1280", 0 },
	{ "ip6_sit_ttl", "64", 0 },

	{ "ip6_wan_dhcp", "0", 0 },
	{ "ip6_wan_addr", "", 0 },
	{ "ip6_wan_size", "64", 0 },
	{ "ip6_wan_gate", "", 0 },

	{ "ip6_dns_auto", "0", 0 },
	{ "ip6_dns1", "", 0 },
	{ "ip6_dns2", "", 0 },
	{ "ip6_dns3", "", 0 },

	{ "ip6_lan_auto", "0", 0 },
	{ "ip6_lan_addr", "", 0 },
	{ "ip6_lan_size", "64", 0 },
	{ "ip6_lan_radv", "1", 0 },
	{ "ip6_lan_dhcp", "1", 0 },
#endif

	{ "x_DHCPClient", "1", 0 },
	{ "wan_mode_x", "2", 0 },
	{ "wan_route_x", "IP_Routed", 0 },
	{ "wan_nat_x", "1", 0 },
	{ "wan_dnsenable_x", "1", 0 },
	{ "wan_dns1_x", "", 0 },
	{ "wan_dns2_x", "", 0 },
	{ "sp_battle_ips", "0", 0 },
	{ "vts_enable_x", "0", 0 },
	{ "vts_num_x", "0", 0 },
	{ "wan_pppoe_txonly_x", "0", 0 },
	{ "wan_hwaddr_x", "", 0 },
	{ "wan_heartbeat_x", "", 0 },
	{ "wan_proto_t", "", 0 },
	{ "wan_ipaddr_t", "", 0 },
	{ "wan_netmask_t", "", 0 },
	{ "wan_gateway_t", "", 0 },
	{ "wan_dns_t", "", 0 },
	{ "wan_status_t", "", 0 },
	{ "fw_log_x", "none", 0 },
	{ "misc_http_x", "0", 0 },
	{ "misc_httpport_x", "8080", 0 },
	{ "misc_lpr_x", "0", 0 },
	{ "misc_ping_x", "0", 0 },
	{ "fw_lw_enable_x", "0", 0 },
	{ "fw_lw_enable_x_1", "0", 0 },
	{ "filter_lw_date_x", "1111111", 0 },
	{ "filter_lw_time_x", "00002359", 0 },
	{ "filter_lw_time_x_1", "00002359", 0 },
	{ "filter_lw_default_x", "ACCEPT", 0 },
	{ "filter_lw_icmp_x", "", 0 },
	{ "filter_lw_num_x", "0", 0 },
	{ "url_enable_x", "0", 0 },
	{ "url_enable_x_1", "0", 0 },
	{ "url_date_x", "1111111", 0 },
	{ "url_time_x", "00002359", 0 },
	{ "url_time_x_1", "00002359", 0 },
	{ "url_num_x", "0", 0 },
	{ "macfilter_enable_x", "0", 0 },
	{ "macfilter_num_x", "0", 0 },
	{ "sr_enable_x", "0", 0 },
	{ "sr_rip_x", "0", 0 },
	{ "sr_num_x", "0", 0 },
	{ "dr_static_rip_x", "0", 0 },
	{ "dr_static_matric_x", "1", 0 },
	{ "dr_default_x", "1", 0 },
	{ "dr_staticnum_x", "0", 0 },
	{ "dr_staticipaddr_x", "", 0 },
	{ "dr_staticnetmask_x", "0", 0 },
	{ "dr_staticgateway_x", "", 0 },
	{ "dhcp_enable_x", "1", 0 },
	{ "dhcp_lease", "86400", 0 },
	{ "dhcp_gateway_x", "", 0 },
	{ "dhcp_dns1_x", "", 0 },
	{ "dhcp_dns2_x", "", 0 },
	{ "dhcp_dns3_x", "", 0 },
	{ "dhcp_wins_x", "", 0 },
	{ "dhcp_wpad_x", "", 0 },
	{ "dhcp_static_x", "0", 0 },
	{ "dhcp_staticnum_x", "0", 0 },
	{ "ntp_period", "24", 0 },
	{ "ntp_server0", "pool.ntp.org", 0 },
	{ "ntp_server1", "time.nist.gov", 0 },

	{ "ddns_enable_x", "0", 0 },
	{ "ddns_server_x", "", 0 },
	{ "ddns_username_x", "", 0 },
	{ "ddns_passwd_x", "", 0 },
	{ "ddns_hostname_x", "", 0 },
	{ "ddns_hostname2_x", "", 0 },
	{ "ddns_hostname3_x", "", 0 },
	{ "ddns_wildcard_x", "0", 0 },
	{ "ddns_period", "12", 0 },
	{ "ddns_verbose", "1", 0 },
	{ "ddns_source", "0", 0 },
	{ "asusddns_tos_agreement", "0", 0 },

	{ "preferred_lang", "", 0 },

#if 0
	{ "qos_enable","0", 0 },
	{ "qos_rulenum_x", "0", 0 },
	{ "qos_shortpkt_prio","0", 0 },			// Internet
	{ "qos_pshack_prio","0", 0 },			// Game
	{ "qos_tos_prio","0", 0 },
	{ "qos_service_enable","0", 0 },
	{ "qos_service_ubw","0", 0 },
	{ "qos_manual_ubw","0", 0 },			// 2008.05 James.
	{ "qos_dfragment_enable","0", 0 },
	{ "qos_dfragment_size","0", 0 },
#endif

	{ "x_Setting", "0", 0 },			// 2007.10 James
	{ "w_Setting", "0", 0 },

	{ "modem_rule", "0", 0 },
	{ "modem_type", "0", 0 },
	{ "modem_country", "", 0 },
	{ "modem_isp", "", 0 },
	{ "modem_apn", "", 0 },
	{ "modem_dialnum", "", 0 },
	{ "modem_user", "", 0 },
	{ "modem_pass", "", 0 },
	{ "modem_pin", "", 0 },
	{ "modem_node", "0", 0 },
	{ "modem_nets", "0", 0 },
	{ "modem_arun", "2", 0 },
	{ "modem_zcd", "0", 0 },
	{ "modem_cmd", "", 0 },
	{ "modem_mtu", "1500", 0 },
	{ "modem_mru", "1500", 0 },

	{ "udpxy_enable_x", "0", 0 },
	{ "xupnpd_enable_x", "0", 0 },
	{ "xupnpd_udpxy", "0", 0 },

/* traffic monitor - added by jerry5 2009/07 */
	{ "rstats_enable", "1", 0 },
	{ "rstats_stored", "1", 0 },
	{ "rstats_colors", "", 0 },
	{ "http_id", "TIDe855a6487043d70a", 0 },
	{ "controlrate_unknown_unicast", "0", 0 },
	{ "controlrate_unknown_multicast", "0", 0 },
	{ "controlrate_multicast", "0", 0 },
	{ "controlrate_broadcast", "10", 0 },
	{ "asus_debug", "0", 0 },
	{ "di_debug", "0", 0 },
	{ "telnetd", "1", 0 },

	{ "fw_pt_pptp", "1", 0 },
	{ "fw_pt_l2tp", "1", 0 },
	{ "fw_pt_ipsec", "1", 0 },
	{ "fw_pt_pppoe", "0", 0 },

	{ "wan_src_phy", "0", 0},
	{ "wan_stb_iso", "0", 0},
	{ "vlan_filter", "0", 0},
	{ "vlan_vid_cpu", "", 0},
	{ "vlan_pri_cpu", "0", 0},
	{ "vlan_vid_iptv", "", 0},
	{ "vlan_pri_iptv", "0", 0},
	{ "vlan_vid_lan1", "", 0},
	{ "vlan_pri_lan1", "0", 0},
	{ "vlan_tag_lan1", "0", 0},
	{ "vlan_vid_lan2", "", 0},
	{ "vlan_pri_lan2", "0", 0},
	{ "vlan_tag_lan2", "0", 0},
	{ "vlan_vid_lan3", "", 0},
	{ "vlan_pri_lan3", "0", 0},
	{ "vlan_tag_lan3", "0", 0},
	{ "vlan_vid_lan4", "", 0},
	{ "vlan_pri_lan4", "0", 0},
	{ "vlan_tag_lan4", "0", 0},

	{ "pppoe_dhcp_route", "1", 0},
	{ "sw_mode", "1", 0},

	{ "u2ec_enable", "1", 0 },
	{ "lprd_enable", "1", 0 },
	{ "rawd_enable", "1", 0 },
	{ "achk_enable", "0", 0 },
	{ "nfsd_enable", "0", 0 },
	{ "ftpd_wopen", "0", 0 },
	{ "ftpd_wport", "21", 0 },
	{ "sshd_enable", "0", 0 },
	{ "sshd_wopen", "0", 0 },
	{ "sshd_wport", "10022", 0 },
	{ "optw_enable", "0", 0 },
	{ "dlna_disc", "90", 0 },
	{ "dlna_root", "0", 0 },
	{ "dlna_src1", "A,/media", 0 },
	{ "dlna_src2", "V,/media", 0 },
	{ "dlna_src3", "P,/media", 0 },
	{ "dlna_rescan", "0", 0},
	{ "trmd_enable", "0", 0 },
	{ "trmd_pport", "51413", 0 },
	{ "trmd_rport", "9091", 0 },
	{ "trmd_ropen", "0", 0 },
	{ "aria_enable", "0", 0 },
	{ "aria_pport", "16888", 0 },
	{ "aria_rport", "6800", 0 },
	{ "aria_ropen", "0", 0 },
	{ "hdd_spindt", "0", 0 },
	{ "hdd_apmoff", "0", 0 },
	{ "stb_cpu_iso", "0", 0 },
	{ "wan_l2tpd", "0", 0 },
	{ "wan_auth_mode", "0", 0 },
	{ "wan_auth_user", "", 0 },
	{ "wan_auth_pass", "", 0 },
	{ "wol_mac_last", "", 0 },
	{ "gw_arp_ping", "0", 0 },
	{ "ez_action_short", "0", 0 },
	{ "ez_action_long", "0", 0 },
	{ "front_leds", "0", 0 },
	{ "ether_led0", "3", 0 },
	{ "ether_led1", "0", 0 },
	{ "ether_igmp", "1", 0 },
	{ "ether_jumbo", "1", 0 },
	{ "ether_green", "1", 0 },
	{ "ether_link_wan",  "0", 0 },
	{ "ether_link_lan1", "0", 0 },
	{ "ether_link_lan2", "0", 0 },
	{ "ether_link_lan3", "0", 0 },
	{ "ether_link_lan4", "0", 0 },
	{ "ether_flow_wan",  "0", 0 },
	{ "ether_flow_lan1", "0", 0 },
	{ "ether_flow_lan2", "0", 0 },
	{ "ether_flow_lan3", "0", 0 },
	{ "ether_flow_lan4", "0", 0 },
	{ "hw_nat_mode", "1", 0 },
	{ "sw_nat_mode", "0", 0 },
	{ "fw_syn_cook", "0", 0 },
	{ "fw_mac_drop", "0", 0 },
	{ "nf_nat_type", "2", 0 },
	{ "nf_nat_loop", "1", 0 },
	{ "nf_max_conn", "16384", 0 },
	{ "nf_alg_ftp0", "21", 0 },
	{ "nf_alg_ftp1", "", 0 },
	{ "nf_alg_pptp", "0", 0 },
	{ "nf_alg_h323", "0", 0 },
	{ "nf_alg_rtsp", "0", 0 },
	{ "nf_alg_sip", "0", 0 },
	{ "help_enable", "1", 0 },
	{ "vpns_enable", "0", 0 },
	{ "vpns_type", "0", 0 },
	{ "vpns_cast", "0", 0 },
	{ "vpns_auth", "0", 0 },
	{ "vpns_mppe", "1", 0 },
	{ "vpns_mtu", "1460", 0 },
	{ "vpns_mru", "1460", 0 },
	{ "vpns_cli0", "245", 0 },
	{ "vpns_cli1", "254", 0 },
	{ "vpns_num_x", "0", 0 },
#if defined(APP_OPENVPN)
	{ "vpns_ov_prot", "0", 0 },
	{ "vpns_ov_port", "1194", 0 },
	{ "vpns_ov_auth", "0", 0 },
	{ "vpns_ov_atls", "0", 0 },
	{ "vpns_ov_ciph", "1", 0 },
	{ "vpns_ov_comp", "0", 0 },
#endif

	{ 0, 0, 0 }
};

struct nvram_tuple tables_defaults[] = {

	{"vts_port_x", "", 0},
	{"vts_ipaddr_x", "", 0},
	{"vts_lport_x", "", 0},
	{"vts_proto_x", "", 0},
	{"vts_protono_x", "", 0},
	{"vts_desc_x", "", 0},

	{"filter_lw_srcip_x", "", 0},
	{"filter_lw_srcport_x", "", 0},
	{"filter_lw_dstip_x", "", 0},
	{"filter_lw_dstport_x", "", 0},
	{"filter_lw_proto_x", "", 0},
	{"filter_lw_desc_x", "", 0},

	{"macfilter_list_x", "", 0},
	{"macfilter_date_x", "", 0},
	{"macfilter_time_x", "", 0},

	{"url_keyword_x", "", 0},

	{"acc_username", "", 0},
	{"acc_password", "", 0},

	{"sr_ipaddr_x", "", 0},
	{"sr_netmask_x", "", 0},
	{"sr_gateway_x", "", 0},
	{"sr_matric_x", "", 0},
	{"sr_if_x", "", 0},

	{"dhcp_staticmac_x", "", 0},
	{"dhcp_staticip_x", "", 0},
	{"dhcp_staticname_x", "", 0},

	{"vpns_user_x", "", 0},
	{"vpns_pass_x", "", 0},
	{"vpns_addr_x", "", 0},

	{"wl_maclist_x", "", 0},
	{"wl_macdesc_x", "", 0},

	{"rt_maclist_x", "", 0},
	{"rt_macdesc_x", "", 0},

	{"wl_wdslist_x", "", 0},
	{"rt_wdslist_x", "", 0},

	{ 0, 0, 0 }
};
