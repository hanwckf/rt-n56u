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
	
	{ "CountryRegion", "5", 0 },
//	{ "CountryRegionABand", "7", 0 },
//	{ "CountryCode", "", 0 },
//	{ "BssidNum", "1", 0 },
//	{ "SSID", "RT2880_AP", 0 },
//	{ "WirelessMode", "9", 0 },
//	{ "FixedTxMode", "1", 0 },
//	{ "TxRate", "0", 0 },
//	{ "Channel", "6", 0 },
//	{ "BasicRate", "15", 0 },
//	{ "BeaconPeriod", "100", 0 },
//	{ "DtimPeriod", "1", 0 },
	{ "TxPower", "100", 0 },
//	{ "BGProtection", "0", 0 },
//	{ "DisableOLBC", "0", 0 },
//	{ "TxAntenna", "", 0 },
//	{ "RxAntenna", "", 0 },
//	{ "RTSThreshold", "2347", 0 },
//	{ "FragThreshold", "2346", 0 },
	{ "TxBurst", "1", 0 },
	{ "PktAggregate", "1", 0 },
//	{ "NoForwarding", "0", 0 },
//	{ "NoForwardingBTNBSSID", "0", 0 },
//	{ "HideSSID", "0", 0 },
//	{ "ShortSlot", "1", 0 },
//	{ "AutoChannelSelect", "0", 0 },
//	{ "WiFiTest", "0", 0 },
//	{ "TGnWifiTest", "0", 0 },
//	{ "AccessPolicy0", "0", 0 },
//	{ "AccessControlList0", "", 0 },
//	{ "AccessPolicy1", "0", 0 },
//	{ "AccessControlList1", "", 0 },
//	{ "AccessPolicy2", "0", 0 },
//	{ "AccessControlList2", "", 0 },
//	{ "AccessPolicy3", "0", 0 },
//	{ "AccessControlList3", "", 0 },

	{ "HT_AutoBA", "1", 0 },
	{ "HT_HTC", "1", 0 },
	{ "HT_RDG", "1", 0 },
	{ "HT_LinkAdapt", "0", 0 },
#if defined (W7_LOGO) || defined (WIFI_LOGO)
	{ "HT_BW", "0", 0 },
#else
	{ "HT_BW", "1", 0 },
#endif
	{ "HT_EXTCHA", "1", 0 },
	{ "HT_OpMode", "0", 0 },
	{ "HT_MpduDensity", "5", 0 },
	{ "HT_AMSDU", "0", 0 },
	{ "HT_GI", "1", 0 },
	{ "HT_BAWinSize", "64", 0 },
	{ "HT_MCS", "33", 0 },
	{ "HT_BADecline", "0", 0 },
	{ "HT_TxStream", "2", 0 },
	{ "HT_RxStream", "2", 0 },
	{ "HT_STBC", "1", 0 },
	{ "HT_PROTECT", "1", 0 },

//	{ "McastPhyMode", "0", 0 },
//	{ "McastMcs", "0", 0 },
	{ "GreenAP", "0", 0 },

//	{ "WmmCapable", "1", 0 },
//	{ "APAifsn", "3;7;1;1", 0 },
//	{ "APCwmin", "4;4;3;2", 0 },
//	{ "APCwmax", "6;10;4;3", 0 },
//	{ "APTxop", "0;0;94;47", 0 },
//	{ "APACM", "0;0;0;0", 0 },
//	{ "BSSAifsn", "3;7;2;2", 0 },
//	{ "BSSCwmin", "4;4;3;2", 0 },
//	{ "BSSCwmax", "10;10;4;3", 0 },
//	{ "BSSTxop", "0;0;94;47", 0 },
//	{ "BSSACM", "0;0;0;0", 0 },
//	{ "AckPolicy", "0;0;0;0", 0 },
	{ "APSDCapable", "1", 0 },
	{ "DLSCapable", "0", 0 },

	{ "IEEE80211H", "0", 0 },
	{ "CSPeriod", "10", 0 },
	{ "RDRegion", "FCC", 0 },
	{ "CarrierDetect", "0", 0},
	{ "ChannelGeography", "2", 0},
//	{ "WirelessEvent", "0", 0 },

//	{ "AuthMode", "OPEN", 0 },
//	{ "EncrypType", "NONE", 0 },
//	{ "DefaultKeyID", "1", 0 },
//	{ "Key1Str", "", 0 },
//	{ "Key2Str", "", 0 },
//	{ "Key3Str", "", 0 },
//	{ "Key4Str", "", 0 },
//	{ "Key1Type", "0", 0 },	
//	{ "Key2Type", "0", 0 },
//	{ "Key3Type", "0", 0 },
//	{ "Key4Type", "0", 0 },
//	{ "WPAPSK", "12345678", 0 },
//	{ "RekeyMethod", "TIME", 0 },
//	{ "RekeyInterval", "0", 0 },
//	{ "PMKCachePeriod", "10", 0 },
//	{ "PreAuth", "0", 0 },

//	{ "WdsEnable", "0", 0 },
//	{ "WdsPhyMode", "0"},		// 0:CCK, 1:OFDM, 2:HTMIX, 3:GREENFIELD
//	{ "WdsEncrypType", "NONE", 0 },	// NONE, WEP, TKIP, AES
//	{ "WdsList", "", 0 },
//	{ "WdsKey", "", 0 },

//	{ "IEEE8021X", "0", 0 },
//	{ "RADIUS_Server", "", 0 },
//	{ "RADIUS_Port", "1812", 0 },
//	{ "RADIUS_Key", "", 0 },
//	{ "RADIUS_Acct_Server", "", 0 },
//	{ "RADIUS_Acct_Port", "1813", 0 },
//	{ "RADIUS_Acct_Key", "", 0 },
//	{ "EAPifname", "br0", 0 },
//	{ "PreAuthifname", "br0", 0 },
//	{ "session_timeout_interval", "0", 0 },
//	{ "idle_timeout_interval", "0", 0 },

	{ "wl_IgmpSnEnable", "1", 0 },

	/* Miscellaneous parameters */
	{ "timer_interval", "3600", 0 },	/* Timer interval in seconds */
	{ "ntp_server", "192.5.41.40 192.5.41.41 133.100.9.2", 0 },		/* NTP server */
//	{ "time_zone", "PST8PDT", 0 },		/* Time zone (GNU TZ format) */
	{ "time_zone", "GMT0", 0 },
	{ "log_level", "0", 0 },		/* Bitmask 0:off 1:denied 2:accepted */
	{ "upnp_enable", "1", 0 },		/* Start UPnP */
	{ "ezc_enable", "1", 0 },		/* Enable EZConfig updates */
//	{ "ezc_version", EZC_VERSION_STR, 0 },	/* EZConfig version */
//	{ "is_default", "1", 0 },		/* is it default setting: 1:yes 0:no */
//	{ "os_server", "", 0 },			/* URL for getting upgrades */
	{ "stats_server", "", 0 },		/* URL for posting stats */
	{ "console_loglevel", "7", 0 },		/* Kernel panics only */

	/* Big switches */
	{ "router_disable", "0", 0 },		/* lan_proto=static lan_stp=0 wan_proto=disabled */
//	{ "ure_disable", "1", 0 },		/* sets APSTA for radio and puts wirelesss interfaces in correct lan */
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
//	{ "lan_hwnames", "", 0 },		/* LAN driver names (e.g. et0) */
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
//	{ "wan_pppoe_idletime", "60", 0 },	/* Dial on demand max idle time (seconds) */
	{ "wan_pppoe_idletime", "0", 0 },	// oleg patch
	{ "wan_pppoe_keepalive", "0", 0 },	/* Restore link automatically */
	{ "wan_pppoe_demand", "0", 0 },		/* Dial on demand */
	{ "wan_pppoe_mru", "1492", 0 },		/* Negotiate MRU to this value */
	{ "wan_pppoe_mtu", "1492", 0 },		/* Negotiate MTU to the smaller of this value or the peer MRU */
	{ "wan_pppoe_service", "", 0 },		/* PPPoE service name */
	{ "wan_pppoe_ac", "", 0 },		/* PPPoE access concentrator name */
	{ "wan_pppoe_lcpa", "1", 0 },		/* Adaptive LCP Echo */

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
	{ "wl_mcastrate", "3", 0 },		/* Mcast Rate (bps) */
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

#if 0
	/* WME parameters (cwmin cwmax aifsn txop_b txop_ag adm_control oldest_first) */

	/* EDCA parameters for STA */
	{ "wl_wme_sta_be", "15 1023 3 0 0 off off", 0 },	/* WME STA AC_BE parameters */
	{ "wl_wme_sta_bk", "15 1023 7 0 0 off off", 0 },	/* WME STA AC_BK parameters */
	{ "wl_wme_sta_vi", "7 15 2 6016 3008 off off", 0 },	/* WME STA AC_VI parameters */
	{ "wl_wme_sta_vo", "3 7 2 3264 1504 off off", 0 },	/* WME STA AC_VO parameters */

	/* EDCA parameters for AP */
	{ "wl_wme_ap_be", "15 63 3 0 0 off off", 0 },		/* WME AP AC_BE parameters */
	{ "wl_wme_ap_bk", "15 1023 7 0 0 off off", 0 },		/* WME AP AC_BK parameters */
	{ "wl_wme_ap_vi", "7 15 1 6016 3008 off off", 0 },	/* WME AP AC_VI parameters */
	{ "wl_wme_ap_vo", "3 7 1 3264 1504 off off", 0 },	/* WME AP AC_VO parameters */

	{ "wl_wme_apsd", "on", 0 },				/* WME APSD mode */
#endif
	{ "wl_txbf", "1", 0 },					/* TxBF */

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
	{ "rt_closed", "0", 0 },
	{ "rt_macmode", "disabled", 0 },
	{ "rt_mcastrate", "1", 0 },
	{ "rt_mode", "ap", 0 },

	{ "rt_HT_AutoBA", "1", 0 },
	{ "rt_HT_HTC", "1", 0 },
	{ "rt_HT_RDG", "1", 0 },
	{ "rt_HT_LinkAdapt", "0", 0 },
#ifdef MR
	{ "rt_HT_BW", "1", 0 },
#else
	{ "rt_HT_BW", "0", 0 },
#endif
	{ "rt_HT_EXTCHA", "1", 0 },
	{ "rt_HT_OpMode", "0", 0 },
	{ "rt_HT_MpduDensity", "5", 0 },
	{ "rt_HT_AMSDU", "0", 0 },
	{ "rt_HT_GI", "1", 0 },
	{ "rt_HT_BAWinSize", "64", 0 },
	{ "rt_HT_MCS", "33", 0 },
	{ "rt_HT_BADecline", "0", 0 },
	{ "rt_HT_TxStream", "2", 0 },
	{ "rt_HT_RxStream", "2", 0 },
	{ "rt_HT_STBC", "1", 0 },
	{ "rt_HT_PROTECT", "1", 0 },
	{ "rt_wsc_config_state", "0", 0 },
	{ "rt_secret_code", "0", 0 },

	{ "rt_DLSCapable", "0", 0 },
	{ "rt_APSDCapable", "1", 0 },
	{ "rt_wme", "1", 0 },
	{ "rt_wme_no_ack", "off", 0 },

	{ "rt_GreenAP", "0", 0 },
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

	/* qos rule for media review */
#ifdef MR
	{ "qos_rulenum_x", "43", 0 },
        { "qos_port_x", "", 0 },
        { "qos_port_x0", "6112:6119", 0 },
        { "qos_port_x1", "4000", 0 },
        { "qos_port_x2", "27030:27039", 0 },
        { "qos_port_x3", "2302:2400", 0 },
        { "qos_port_x4", "6073", 0 },
        { "qos_port_x5", "80", 0 },
        { "qos_port_x6", "4711", 0 },
        { "qos_port_x7", "29900", 0 },
        { "qos_port_x8", "29901", 0 },
        { "qos_port_x9", "", 0 },
        { "qos_port_x10", "", 0 },
        { "qos_port_x11", "", 0 },
        { "qos_port_x12", "28960", 0 },
        { "qos_port_x13", "", 0 },
        { "qos_port_x14", "", 0 },
        { "qos_port_x15", "", 0 },
        { "qos_port_x16", "", 0 },
        { "qos_port_x17", "6112:6119", 0 },
        { "qos_port_x18", "4000", 0 },
        { "qos_port_x19", "", 0 },
        { "qos_port_x20", "1024:6000", 0 },
        { "qos_port_x21", "7000", 0 },
        { "qos_port_x22", "", 0 },
        { "qos_port_x23", "6003", 0 },
        { "qos_port_x24", "7002", 0 },
        { "qos_port_x25", "", 0 },
        { "qos_port_x26", "", 0 },
        { "qos_port_x27", "27660", 0 },
        { "qos_port_x28", "27960", 0 },
        { "qos_port_x29", "3453", 0 },
        { "qos_port_x30", "9442", 0 },
        { "qos_port_x31", "1030", 0 },
        { "qos_port_x32", "6881:6999", 0 },
        { "qos_port_x33", "3074", 0 },
        { "qos_port_x34", "", 0 },
        { "qos_port_x35", "", 0 },
        { "qos_port_x36", "4661:4662", 0 },
        { "qos_port_x37", "6881:6889", 0 },
        { "qos_port_x38", "6699", 0 },
        { "qos_port_x39", "4661:4662", 0 },
        { "qos_port_x40", "4711", 0 },
        { "qos_port_x41", "6667", 0 },
        { "qos_port_x42", "2300:2400", 0 },
        { "qos_port_x43", "47624", 0 },
        { "qos_port_y0", "6112:6119", 0 },
        { "qos_port_y1", "", 0 },
        { "qos_port_y2", "27000:27015", 0 },
        { "qos_port_y3", "2302:2400", 0 },
        { "qos_port_y4", "6073", 0 },
        { "qos_port_y5", "1500:4999", 0 },
        { "qos_port_y6", "16567", 0 },
        { "qos_port_y7", "27900", 0 },
        { "qos_port_y8", "29900", 0 },
        { "qos_port_y9", "3000:23009", 0 },
        { "qos_port_y10", "27900", 0 },
        { "qos_port_y11", "28900", 0 },
        { "qos_port_y12", "289", 0 },
        { "qos_port_y13", "602", 0 },
        { "qos_port_y14", "050", 0 },
        { "qos_port_y15", "020", 0 },
        { "qos_port_y16", "510", 0 },
        { "qos_port_y17", "6112:6119", 0 },
        { "qos_port_y18", "", 0 },
        { "qos_port_y19", "27666", 0 },
        { "qos_port_y20", "1024:6000", 0 },
        { "qos_port_y21", "7000", 0 },
        { "qos_port_y22", "6500", 0 },
        { "qos_port_y23", "27005", 0 },
        { "qos_port_y24", "27010", 0 },
        { "qos_port_y25", "27011", 0 },
        { "qos_port_y26", "27015", 0 },
        { "qos_port_y27", "27660", 0 },
        { "qos_port_y28", "27960", 0 },
        { "qos_port_y29", "3453", 0 },
        { "qos_port_y30", "9442", 0 },
        { "qos_port_y31", "1030", 0 },
        { "qos_port_y32", "", 0 },
        { "qos_port_y33", "88", 0 },
        { "qos_port_y34", "3074", 0 },
        { "qos_port_y35", "10480:10483", 0 },
        { "qos_port_y36", "4665", 0 },
        { "qos_port_y37", "", 0 },
        { "qos_port_y38", "6257", 0 },
        { "qos_port_y39", "4672", 0 },
        { "qos_port_y40", "4665", 0 },
        { "qos_port_y41", "28800:29000", 0 },
        { "qos_port_y42", "2300:2400", 0 },
        { "qos_port_y43", "", 0 },
        { "qos_prio_x", "", 0 },
        { "qos_prio_x0", "1", 0 },
        { "qos_prio_x1", "1", 0 },
        { "qos_prio_x2", "1", 0 },
        { "qos_prio_x3", "1", 0 },
        { "qos_prio_x4", "1", 0 },
        { "qos_prio_x5", "1", 0 },
        { "qos_prio_x6", "1", 0 },
        { "qos_prio_x7", "1", 0 },
        { "qos_prio_x8", "1", 0 },
        { "qos_prio_x9", "1", 0 },
        { "qos_prio_x10", "1", 0 },
        { "qos_prio_x11", "1", 0 },
        { "qos_prio_x12", "1", 0 },
        { "qos_prio_x13", "1", 0 },
        { "qos_prio_x14", "1", 0 },
        { "qos_prio_x15", "1", 0 },
        { "qos_prio_x16", "1", 0 },
        { "qos_prio_x17", "1", 0 },
        { "qos_prio_x18", "1", 0 },
        { "qos_prio_x19", "1", 0 },
        { "qos_prio_x20", "1", 0 },
        { "qos_prio_x21", "1", 0 },
        { "qos_prio_x22", "1", 0 },
        { "qos_prio_x23", "1", 0 },
        { "qos_prio_x24", "1", 0 },
        { "qos_prio_x25", "1", 0 },
        { "qos_prio_x26", "1", 0 },
        { "qos_prio_x27", "1", 0 },
        { "qos_prio_x28", "1", 0 },
        { "qos_prio_x29", "1", 0 },
        { "qos_prio_x30", "1", 0 },
        { "qos_prio_x31", "1", 0 },
        { "qos_prio_x32", "1", 0 },
        { "qos_prio_x33", "1", 0 },
        { "qos_prio_x34", "1", 0 },
        { "qos_prio_x35", "1", 0 },
        { "qos_prio_x36", "6", 0 },
        { "qos_prio_x37", "6", 0 },
        { "qos_prio_x38", "6", 0 },
        { "qos_prio_x39", "6", 0 },
        { "qos_prio_x40", "6", 0 },
        { "qos_prio_x41", "4", 0 },
        { "qos_prio_x42", "4", 0 },
        { "qos_prio_x43", "4", 0 },
        { "qos_service_name_x", "", 0 },
        { "qos_service_name_x0", "Warcraft III", 0 },
        { "qos_service_name_x1", "Warcraft III", 0 },
        { "qos_service_name_x2", "Counter Strike", 0 },
        { "qos_service_name_x3", "Age of Empires", 0 },
        { "qos_service_name_x4", "Age of Empires", 0 },
        { "qos_service_name_x5", "Battlefield 2", 0 },
        { "qos_service_name_x6", "Battlefield 2", 0 },
        { "qos_service_name_x7", "Battlefield 2", 0 },
        { "qos_service_name_x8", "Battlefield 2", 0 },
        { "qos_service_name_x9", "Battlefield 1942", 0 },
        { "qos_service_name_x10", "Battlefield 1942", 0 },
        { "qos_service_name_x11", "Battlefield 1942", 0 },
        { "qos_service_name_x12", "Call for Duty", 0 },
        { "qos_service_name_x13", "Call for Duty", 0 },
        { "qos_service_name_x14", "Call for Duty", 0 },
        { "qos_service_name_x15", "Call for Duty", 0 },
        { "qos_service_name_x16", "Call for Duty", 0 },
        { "qos_service_name_x17", "Diablo I and II", 0 },
        { "qos_service_name_x18", "Diablo I and II", 0 },
        { "qos_service_name_x19", "Doom 3", 0 },
        { "qos_service_name_x20", "Everquest", 0 },
        { "qos_service_name_x21", "Everquest", 0 },
        { "qos_service_name_x22", "Gamespy Arcade", 0 },
        { "qos_service_name_x23", "Half Life", 0 },
        { "qos_service_name_x24", "Half Life", 0 },
        { "qos_service_name_x25", "Half Life", 0 },
        { "qos_service_name_x26", "Half Life", 0 },
        { "qos_service_name_x27", "Quake 3", 0 },
        { "qos_service_name_x28", "Quake 3", 0 },
        { "qos_service_name_x29", "Myth", 0 },
        { "qos_service_name_x30", "Need for Speed", 0 },
        { "qos_service_name_x31", "Need for Speed 3", 0 },
        { "qos_service_name_x32", "World of Warcraft", 0 },
        { "qos_service_name_x33", "Xbox Live", 0 },
        { "qos_service_name_x34", "Xbox Live", 0 },
        { "qos_service_name_x35", "SWAT 4", 0 },
        { "qos_service_name_x36", "eDonkey", 0 },
        { "qos_service_name_x37", "BitTorrent", 0 },
        { "qos_service_name_x38", "WinMX", 0 },
        { "qos_service_name_x39", "eMule", 0 },
        { "qos_service_name_x40", "eMule", 0 },
        { "qos_service_name_x41", "MSN Game Zone", 0 },
        { "qos_service_name_x42", "MSN Game Zone (DX)", 0 },
        { "qos_service_name_x43", "MSN Game Zone (DX)", 0 },
#endif

	{ "ehci_ports", "1-1 1-2", 0 },
	{ "ohci_ports", "2-1 2-2", 0 },

	#include "./flash.default"
	{ 0, 0, 0 }
};
