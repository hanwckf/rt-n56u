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

	struct variable variables_Language[] = {
			{"preferred_lang", "", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_Storage_UserList[] = {
			{"acc_username", "41", NULL, FALSE},
			{"acc_password", "38", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_IPConnection_VSList[] = {
			{"vts_port_x", "12", NULL, FALSE},
			{"vts_ipaddr_x", "16", NULL, FALSE},
			{"vts_lport_x", "7", NULL, FALSE},
			{"vts_proto_x", "8", NULL, FALSE},
			{"vts_protono_x", "12", NULL, FALSE},
			{"vts_desc_x", "20", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_FirewallConfig_LWFilterList[] = {
			{"filter_lw_srcip_x", "16", NULL, FALSE},
			{"filter_lw_srcport_x", "12", NULL, FALSE},
			{"filter_lw_dstip_x", "16", NULL, FALSE},
			{"filter_lw_dstport_x", "12", NULL, FALSE},
			{"filter_lw_proto_x", "8", NULL, FALSE},
			{"filter_lw_desc_x", "20", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_FirewallConfig_UrlList[] = {
			{"url_keyword_x", "36", NULL, RESTART_FIREWALL},
			{0,0,0,0}
		};

	struct variable variables_FirewallConfig_MFList[] = {
			{"macfilter_list_x", "20", NULL, FALSE},
			{"macfilter_time_x", "10", NULL, FALSE},
			{"macfilter_date_x", "10", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_RouterConfig_GWStatic[] = {
			{"sr_ipaddr_x", "17", NULL, FALSE},
			{"sr_netmask_x", "17", NULL, FALSE},
			{"sr_gateway_x", "17", NULL, FALSE},
			{"sr_matric_x", "5", NULL, FALSE},
			{"sr_if_x", "3", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_LANHostConfig_ManualDHCPList[] = {
			{"dhcp_staticmac_x", "14", NULL, FALSE},
			{"dhcp_staticip_x", "17", NULL, FALSE},
			{"dhcp_staticname_x", "24", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_LANHostConfig_VPNSACLList[] = {
			{"vpns_user_x", "32", NULL, FALSE},
			{"vpns_pass_x", "32", NULL, FALSE},
			{"vpns_addr_x", "3", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_DeviceSecurity11a_ACLList[] = {
			{"wl_maclist_x", "32", NULL, FALSE},
			{"wl_macdesc_x", "48", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_DeviceSecurity11b_rt_ACLList[] = {
			{"rt_maclist_x", "32", NULL, FALSE},
			{"rt_macdesc_x", "48", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_WLANConfig11a_RBRList[] = {
			{"wl_wdslist_x", "32", NULL, RESTART_WIFI},
			{0,0,0,0}
		};

	struct variable variables_WLANConfig11b_rt_RBRList[] = {
			{"rt_wdslist_x", "32", NULL, RESTART_WIFI},
			{0,0,0,0}
		};

#ifndef NOQOS 
	struct variable variables_PrinterStatus_x_USRRuleList[] = {
			{"qos_service_name_x", "16", NULL, FALSE},
			{"qos_ip_x", "16", NULL, FALSE},
			{"qos_port_x", "12", NULL, FALSE},
			{"qos_port_y", "12", NULL, FALSE},
			{"qos_prio_x", "3", NULL, FALSE},
			{0,0,0,0}
		};
#endif

	struct variable variables_General[] = {
			{"x_Setting", "", NULL, FALSE},
			{"w_Setting", "", NULL, FALSE},
			{"http_username", "", NULL, FALSE},
			{"http_passwd", "", NULL, FALSE},
			{"modem_rule", "", NULL, RESTART_MODEM},
			{"modem_type", "", NULL, RESTART_MODEM},
			{"modem_country", "", NULL, RESTART_WAN},
			{"modem_isp", "", NULL, RESTART_WAN},
			{"modem_apn", "", NULL, RESTART_WAN},
			{"modem_dialnum", "", NULL, RESTART_WAN},
			{"modem_user", "", NULL, RESTART_WAN},
			{"modem_pass", "", NULL, RESTART_WAN},
			{"modem_node", "", NULL, RESTART_WAN},
			{"modem_cmd", "", NULL, RESTART_WAN},
			{"modem_mtu", "", NULL, RESTART_WAN},
			{"modem_mru", "", NULL, RESTART_WAN},
			{"modem_zcd", "", NULL, FALSE},
			{"modem_arun", "", NULL, FALSE},
			{"wan_3g_pin", "", NULL, RESTART_WAN},
			{"front_leds", "", NULL, FALSE},
			{"help_enable", "", NULL, FALSE},
			{"ez_action_short", "", NULL, FALSE},
			{"ez_action_long", "", NULL, FALSE},
			{"u2ec_enable", "", NULL, RESTART_SPOOLER},
			{"lprd_enable", "", NULL, RESTART_SPOOLER},
			{"rawd_enable", "", NULL, RESTART_SPOOLER},
			{0,0,0,0}
		};

	struct variable variables_Storage[] = {
			{"enable_samba", "", NULL, RESTART_FTPSAMBA},
			{"st_samba_mode", "", NULL, RESTART_FTPSAMBA},
			{"st_samba_lmb", "", NULL, RESTART_FTPSAMBA},
			{"computer_name", "", NULL, RESTART_FTPSAMBA},
			{"computer_nameb", "", NULL, RESTART_FTPSAMBA},
			{"st_samba_workgroup", "", NULL, RESTART_FTPSAMBA},
			{"st_samba_workgroupb", "", NULL, RESTART_FTPSAMBA},
			{"enable_ftp", "", NULL, RESTART_FTPSAMBA},
			{"st_ftp_mode", "", NULL, RESTART_FTPSAMBA},
			{"st_ftp_log", "", NULL, RESTART_FTPSAMBA},
			{"st_max_user", "", NULL, RESTART_FTPSAMBA},
			{"acc_num", "", NULL, RESTART_FTPSAMBA},
			{"apps_dms", "", NULL, RESTART_DMS},
			{"apps_itunes", "", NULL, RESTART_ITUNES},
			{"achk_enable", "", NULL, FALSE},
			{"nfsd_enable", "", NULL, RESTART_NFS},
			{"optw_enable", "", NULL, FALSE},
			{"dlna_disc", "", NULL, RESTART_DMS},
			{"dlna_root", "", NULL, RESTART_DMS},
			{"dlna_src1", "", NULL, RESTART_DMS},
			{"dlna_src2", "", NULL, RESTART_DMS},
			{"dlna_src3", "", NULL, RESTART_DMS},
			{"dlna_rescan", "", NULL, RESTART_DMS},
			{"trmd_enable", "", NULL,RESTART_TORRENT},
			{"trmd_pport", "", NULL, RESTART_TORRENT},
			{"trmd_rport", "", NULL, RESTART_TORRENT},
			{"aria_enable", "", NULL,RESTART_ARIA},
			{"aria_pport", "", NULL, RESTART_ARIA},
			{"aria_rport", "", NULL, RESTART_ARIA},
			{"hdd_spindt", "", NULL, RESTART_HDDTUNE},
			{"hdd_apmoff", "", NULL, RESTART_HDDTUNE},
			{"Storage_UserList", "Group", ARGV((char*)variables_Storage_UserList, "16", "79", "acc_num"), RESTART_FTPSAMBA},
			{0,0,0,0}
		};

	struct variable variables_Layer3Forwarding[] = {
			{"x_DHCPClient", "", NULL, RESTART_WAN},
			{"wan_pptp_options_x", "", NULL, RESTART_WAN},
			{"wan_proto", "", NULL, RESTART_WAN},
			{"wan_mode_x", "", NULL, RESTART_WAN},
			{"pppoe_dhcp_route", "", NULL, RESTART_WAN},
			{"wan_stb_x", "", NULL, RESTART_SWITCH_VLAN},
			{"wan_src_phy", "", NULL, FALSE},
			{"wan_stb_iso", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_filter", "", NULL, RESTART_WAN},
			{"vlan_vid_cpu", "", NULL, RESTART_WAN},
			{"vlan_pri_cpu", "", NULL, RESTART_WAN},
			{"vlan_vid_iptv", "", NULL, RESTART_WAN},
			{"vlan_pri_iptv", "", NULL, RESTART_WAN},
			{"vlan_vid_lan1", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_pri_lan1", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_tag_lan1", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_vid_lan2", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_pri_lan2", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_tag_lan2", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_vid_lan3", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_pri_lan3", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_tag_lan3", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_vid_lan4", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_pri_lan4", "", NULL, RESTART_SWITCH_VLAN},
			{"vlan_tag_lan4", "", NULL, RESTART_SWITCH_VLAN},
			{"wan_auth_mode", "", NULL, RESTART_WAN},
			{"wan_auth_user", "", NULL, RESTART_WAN},
			{"wan_auth_pass", "", NULL, RESTART_WAN},
			{0,0,0,0}
		};

	struct variable variables_WANCommonInterface[] = {
			{0,0,0,0}
		};

	struct variable variables_IPConnection[] = {
			{"sw_mode", "", NULL, RESTART_REBOOT},
			{"wan_nat_x", "", NULL, RESTART_WAN},
			{"gw_arp_ping", "", NULL, RESTART_WAN},
			{"hw_nat_mode", "", NULL, RESTART_FIREWALL},
			{"sw_nat_mode", "", NULL, RESTART_FIREWALL},
			{"wan_ipaddr", "", NULL, RESTART_WAN},
			{"wan_netmask", "", NULL, RESTART_WAN},
			{"wan_gateway", "", NULL, RESTART_WAN},
			{"wan_dnsenable_x", "", NULL, RESTART_WAN},
			{"wan_dns1_x", "", NULL, RESTART_WAN},
			{"wan_dns2_x", "", NULL, RESTART_WAN},
			{"dmz_ip", "", NULL, RESTART_FIREWALL},
			{"sp_battle_ips", "", NULL, RESTART_FIREWALL},
			{"vts_enable_x", "", NULL, RESTART_FIREWALL},
			{"vts_num_x", "", NULL, RESTART_FIREWALL},
			{"VSList", "Group", ARGV((char*)variables_IPConnection_VSList, "24", "75", "vts_num_x"), RESTART_FIREWALL},
			{0,0,0,0}
		};


	struct variable variables_IP6Connection[] = {
#if defined (USE_IPV6)
			{"ip6_service", "", NULL, RESTART_IPV6},
			{"ip6_ppe_on", "", NULL, RESTART_FIREWALL},
			{"ip6_wan_if", "", NULL, RESTART_IPV6},
			{"ip6_6in4_remote", "", NULL, RESTART_IPV6},
			{"ip6_6to4_relay", "", NULL, RESTART_IPV6},
			{"ip6_6rd_relay", "", NULL, RESTART_IPV6},
			{"ip6_6rd_size", "", NULL, RESTART_IPV6},
			{"ip6_6rd_dhcp", "", NULL, RESTART_IPV6},
			{"ip6_sit_mtu", "", NULL, RESTART_IPV6},
			{"ip6_sit_ttl", "", NULL, RESTART_IPV6},
			{"ip6_wan_dhcp", "", NULL, RESTART_IPV6},
			{"ip6_wan_addr", "", NULL, RESTART_IPV6},
			{"ip6_wan_size", "", NULL, RESTART_IPV6},
			{"ip6_wan_gate", "", NULL, RESTART_IPV6},
			{"ip6_dns_auto", "", NULL, RESTART_IPV6},
			{"ip6_dns1", "", NULL, RESTART_IPV6},
			{"ip6_dns2", "", NULL, RESTART_IPV6},
			{"ip6_dns3", "", NULL, RESTART_IPV6},
			{"ip6_lan_auto", "", NULL, RESTART_IPV6},
			{"ip6_lan_addr", "", NULL, RESTART_IPV6},
			{"ip6_lan_size", "", NULL, RESTART_IPV6},
			{"ip6_lan_radv", "", NULL, RESTART_DHCPD},
			{"ip6_lan_dhcp", "", NULL, RESTART_DHCPD},
#endif
			{0,0,0,0}
		};

	struct variable variables_PPPConnection[] = {
			{"wan_pppoe_username", "", NULL, RESTART_WAN},
			{"wan_pppoe_passwd", "", NULL, RESTART_WAN},
			{"wan_pppoe_idletime", "", NULL, RESTART_WAN},
			{"wan_pppoe_txonly_x", "", NULL, RESTART_WAN},
			{"wan_pppoe_options_x", "", NULL, RESTART_WAN},
			{"wan_pppoe_mtu", "", NULL, RESTART_WAN},
			{"wan_pppoe_mru", "", NULL, RESTART_WAN},
			{"wan_pppoe_service", "", NULL, RESTART_WAN},
			{"wan_pppoe_ac", "", NULL, RESTART_WAN},
			{"wan_pppoe_lcpa", "", NULL, RESTART_WAN},
			{"wan_pppoe_cpul", "", NULL, RESTART_SYSCTL},
			{"wan_hostname", "", NULL, RESTART_WAN},
			{"wan_l2tpd", "", NULL, RESTART_WAN},
			{"wan_hwaddr_x", "", NULL,  RESTART_WAN},
			{"wan_heartbeat_x", "", NULL, RESTART_WAN},
			{0,0,0,0}
		};

	struct variable variables_EthernetLink[] = {
			{0,0,0,0}
		};

	struct variable variables_FirewallConfig[] = {
			{"fw_enable_x", "", NULL, RESTART_FIREWALL},
			{"fw_dos_x", "", NULL, RESTART_FIREWALL},
			{"fw_log_x", "", NULL, RESTART_FIREWALL},
			{"misc_http_x", "", NULL, RESTART_FIREWALL},
			{"misc_httpport_x", "", NULL, RESTART_FIREWALL},
			{"misc_lpr_x", "", NULL, RESTART_FIREWALL},
			{"misc_ping_x", "", NULL, RESTART_FIREWALL},
			{"fw_pt_pptp", "", NULL, RESTART_FIREWALL},
			{"fw_pt_l2tp", "", NULL, RESTART_FIREWALL},
			{"fw_pt_ipsec", "", NULL, RESTART_FIREWALL},
			{"fw_pt_pppoe", "", NULL, RESTART_SYSCTL},
			{"fw_lw_enable_x", "", NULL, RESTART_FIREWALL},
			{"fw_lw_enable_x_1", "", NULL, RESTART_FIREWALL},
			{"filter_lw_date_x", "", NULL, RESTART_FIREWALL},
			{"filter_lw_time_x", "", NULL, RESTART_FIREWALL},
			{"filter_lw_time_x_1", "", NULL, RESTART_FIREWALL},
			{"filter_lw_default_x", "", NULL, RESTART_FIREWALL},
			{"filter_lw_icmp_x", "", NULL,  RESTART_FIREWALL},
			{"filter_lw_num_x", "", NULL, RESTART_FIREWALL},
			{"url_enable_x", "", NULL, RESTART_FIREWALL},
			{"url_enable_x_1", "", NULL, RESTART_FIREWALL},
			{"url_date_x", "", NULL, RESTART_FIREWALL},
			{"url_time_x", "", NULL, RESTART_FIREWALL},
			{"url_time_x_1", "", NULL, RESTART_FIREWALL},
			{"url_num_x", "", NULL, RESTART_FIREWALL},
			{"macfilter_enable_x", "", NULL, RESTART_FIREWALL},
			{"macfilter_num_x", "", NULL, RESTART_FIREWALL},
			{"ftpd_wopen", "", NULL, RESTART_FIREWALL},
			{"ftpd_wport", "", NULL, RESTART_FIREWALL},
			{"sshd_wopen", "", NULL, RESTART_FIREWALL},
			{"sshd_wport", "", NULL, RESTART_FIREWALL},
			{"trmd_ropen", "", NULL, RESTART_FIREWALL},
			{"aria_ropen", "", NULL, RESTART_FIREWALL},
			{"fw_syn_cook", "", NULL, RESTART_FIREWALL},
			{"fw_mac_drop", "", NULL, RESTART_FIREWALL},
			{"nf_nat_type", "", NULL, RESTART_FIREWALL},
			{"nf_nat_loop", "", NULL, RESTART_FIREWALL},
			{"nf_max_conn", "", NULL, RESTART_FIREWALL},
			{"nf_alg_ftp0", "", NULL, RESTART_FIREWALL},
			{"nf_alg_ftp1", "", NULL, RESTART_FIREWALL},
			{"nf_alg_pptp", "", NULL, RESTART_FIREWALL},
			{"nf_alg_h323", "", NULL, RESTART_FIREWALL},
			{"nf_alg_rtsp", "", NULL, RESTART_FIREWALL},
			{"nf_alg_sip", "", NULL, RESTART_FIREWALL},
			{"LWFilterList", "Group", ARGV((char*)variables_FirewallConfig_LWFilterList, "32", "63", "filter_lw_num_x"), RESTART_FIREWALL},
			{"UrlList", "Group", ARGV((char*)variables_FirewallConfig_UrlList, "128", "36", "url_num_x"), RESTART_FIREWALL},
			{"MFList", "Group", ARGV((char*)variables_FirewallConfig_MFList, "16", "32", "macfilter_num_x"), RESTART_FIREWALL},
			{0,0,0,0}
		};

	struct variable variables_RouterConfig[] = {
			{"sr_enable_x", "", NULL, RESTART_WAN},
			{"dr_enable_x", "", NULL, RESTART_WAN},
			{"mr_enable_x", "", NULL, RESTART_IPTV},
			{"sr_rip_x", "", NULL, RESTART_WAN},
			{"sr_num_x", "", NULL, RESTART_WAN},
			{"dr_static_rip_x", "", NULL, RESTART_WAN},
			{"dr_static_matric_x", "", NULL, RESTART_WAN},
			{"dr_default_x", "", NULL, FALSE},
			{"dr_static_rip_x", "", NULL, FALSE},
			{"dr_staticnum_x", "", NULL, RESTART_WAN},
			{"GWStatic", "Group", ARGV((char*)variables_RouterConfig_GWStatic, "6", "59", "sr_num_x"), RESTART_WAN},
			{0,0,0,0}
		};

	struct variable variables_LANHostConfig[] = {
			{"lan_proto_x", "", NULL, RESTART_LAN},
			{"lan_ipaddr", "", NULL, RESTART_LAN},
			{"lan_netmask", "", NULL, RESTART_LAN},
			{"lan_gateway", "", NULL, RESTART_LAN},
			{"lan_dns_x", "", NULL, RESTART_LAN},
			{"lan_dns1", "", NULL, RESTART_LAN},
			{"lan_dns2", "", NULL, RESTART_LAN},
			{"lan_stp", "", NULL, RESTART_LAN},
			{"lan_domain", "", NULL, RESTART_DHCPD},
			{"udpxy_enable_x", "", NULL, RESTART_IPTV},
			{"xupnpd_enable_x", "", NULL, RESTART_IPTV},
			{"xupnpd_udpxy", "", NULL, RESTART_IPTV},
			{"dhcp_enable_x", "", NULL, RESTART_DHCPD},
			{"dhcp_start", "", NULL, RESTART_DHCPD},
			{"dhcp_end", "", NULL, RESTART_DHCPD},
			{"dhcp_lease", "", NULL, RESTART_DHCPD},
			{"dhcp_gateway_x", "", NULL, RESTART_DHCPD},
			{"dhcp_dns1_x", "", NULL, RESTART_DHCPD},
			{"dhcp_dns2_x", "", NULL, RESTART_DHCPD},
			{"dhcp_dns3_x", "", NULL, RESTART_DHCPD},
			{"dhcp_wins_x", "", NULL, RESTART_DHCPD},
			{"dhcp_static_x", "", NULL, RESTART_DHCPD},
			{"dhcp_staticnum_x", "", NULL, RESTART_DHCPD},
			{"upnp_enable", "", NULL, RESTART_UPNP},
			{"http_lanport", "", NULL, RESTART_HTTPD},
			{"telnetd", "", NULL, RESTART_TERMINAL},
			{"sshd_enable", "", NULL, RESTART_TERMINAL},
			{"ether_led0", "", NULL, RESTART_SWITCH},
			{"ether_led1", "", NULL, RESTART_SWITCH},
			{"ether_igmp", "", NULL, RESTART_SWITCH},
			{"ether_jumbo", "", NULL, RESTART_SWITCH},
			{"ether_green", "", NULL, RESTART_SWITCH},
			{"ether_link_wan",  "", NULL, RESTART_SWITCH},
			{"ether_link_lan1", "", NULL, RESTART_SWITCH},
			{"ether_link_lan2", "", NULL, RESTART_SWITCH},
			{"ether_link_lan3", "", NULL, RESTART_SWITCH},
			{"ether_link_lan4", "", NULL, RESTART_SWITCH},
			{"ether_flow_wan",  "", NULL, RESTART_SWITCH},
			{"ether_flow_lan1", "", NULL, RESTART_SWITCH},
			{"ether_flow_lan2", "", NULL, RESTART_SWITCH},
			{"ether_flow_lan3", "", NULL, RESTART_SWITCH},
			{"ether_flow_lan4", "", NULL, RESTART_SWITCH},
			{"controlrate_unknown_unicast", "", NULL, RESTART_SWITCH},
			{"controlrate_unknown_multicast", "", NULL, RESTART_SWITCH},
			{"controlrate_multicast", "", NULL, RESTART_SWITCH},
			{"controlrate_broadcast", "", NULL, RESTART_SWITCH},
			{"vpns_enable", "", NULL, RESTART_VPNSRV},
			{"vpns_type", "", NULL, RESTART_VPNSRV},
			{"vpns_cast", "", NULL, RESTART_VPNSRV},
			{"vpns_auth", "", NULL, RESTART_VPNSRV},
			{"vpns_mppe", "", NULL, RESTART_VPNSRV},
			{"vpns_mtu", "", NULL, RESTART_VPNSRV},
			{"vpns_mru", "", NULL, RESTART_VPNSRV},
			{"vpns_cli0", "", NULL, RESTART_VPNSRV},
			{"vpns_cli1", "", NULL, RESTART_VPNSRV},
			{"log_ipaddr", "", NULL, RESTART_SYSLOG},
			{"rstats_enable", "", NULL, FALSE},
			{"rstats_stored", "", NULL, FALSE},
			{"time_zone", "", NULL, RESTART_TIME},
			{"ntp_period", "", NULL, FALSE},
			{"ntp_server0", "", NULL, RESTART_NTPC},
			{"ntp_server1", "", NULL, RESTART_NTPC},
			{"ddns_enable_x", "", NULL, RESTART_DDNS},
			{"asusddns_tos_agreement", "", NULL, FALSE},
			{"ddns_server_x", "", NULL, RESTART_DDNS},
			{"ddns_username_x", "", NULL, RESTART_DDNS},
			{"ddns_passwd_x", "", NULL, RESTART_DDNS},
			{"ddns_hostname_x", "", NULL, RESTART_DDNS},
			{"ddns_hostname2_x", "", NULL, RESTART_DDNS},
			{"ddns_hostname3_x", "", NULL, RESTART_DDNS},
			{"ddns_wildcard_x", "", NULL, RESTART_DDNS},
			{"ddns_period", "", NULL, RESTART_DDNS },
			{"ManualDHCPList", "Group", ARGV((char*)variables_LANHostConfig_ManualDHCPList, "8", "55", "dhcp_staticnum_x"), RESTART_DHCPD},
			{"VPNSACLList", "Group", ARGV((char*)variables_LANHostConfig_VPNSACLList, "8", "67", "vpns_num_x"), RESTART_VPNSRV},
			{0,0,0,0}
		};

	struct variable variables_DeviceSecurity11a[] = {
			{"wl_macmode", "", NULL, RESTART_WIFI},
			{"wl_macnum_x", "", NULL, RESTART_WIFI},
			{"ACLList", "Group", ARGV((char*)variables_DeviceSecurity11a_ACLList, "64", "32", "wl_macnum_x"), RESTART_WIFI},
			{0,0,0,0}
		};

	struct variable variables_DeviceSecurity11b[] = {
			{"rt_macmode", "", NULL, RESTART_WIFI},
			{"rt_macnum_x", "", NULL, RESTART_WIFI},
			{"rt_ACLList", "Group", ARGV((char*)variables_DeviceSecurity11b_rt_ACLList, "64", "32", "rt_macnum_x"), RESTART_WIFI},
			{0,0,0,0}
		};

	struct variable variables_WLANAuthentication11a[] = {
			{"wl_radius_ipaddr", "", NULL, RESTART_WIFI},
			{"wl_radius_port", "", NULL, RESTART_WIFI},
			{"wl_radius_key", "", NULL, RESTART_WIFI},
			{0,0,0,0}
		};

	struct variable variables_WLANAuthentication11b[] = {
			{"rt_radius_ipaddr", "", NULL, RESTART_WIFI},
			{"rt_radius_port", "", NULL, RESTART_WIFI},
			{"rt_radius_key", "", NULL, RESTART_WIFI},
			{0,0,0,0}
		};

	struct variable variables_WLANConfig11a[] = {
			{"wl_ssid", "", NULL, RESTART_WIFI},
			{"wl_ssid2", "", NULL, RESTART_WIFI},
			{"wl_mode_x", "", NULL, RESTART_WIFI},
			{"wl_channel", "", NULL, RESTART_WIFI},
			{"wl_wdsapply_x", "", NULL, RESTART_WIFI},
			{"wl_lazywds", "", NULL, RESTART_WIFI},
			{"wl_wdsnum_x", "", NULL, RESTART_WIFI},
			{"wl_maxassoc", "", NULL, RESTART_WIFI},
			{"wl_gmode", "", NULL, RESTART_WIFI},
			{"wl_gmode_protection", "", NULL, RESTART_WIFI},
			{"wl_txbf", "", NULL, RESTART_WIFI},
			{"wl_auth_mode", "", NULL, RESTART_WIFI},
			{"wl_wpa_mode", "", NULL, RESTART_WIFI},
			{"wl_crypto", "", NULL, RESTART_WIFI},
			{"wl_wpa_psk", "", NULL, RESTART_WIFI},
			{"wl_wep_x", "", NULL, RESTART_WIFI},
			{"wl_phrase_x", "", NULL, RESTART_WIFI},
			{"wl_key1", "", NULL, RESTART_WIFI},
			{"wl_key2", "", NULL, RESTART_WIFI},
			{"wl_key3", "", NULL, RESTART_WIFI},
			{"wl_key4", "", NULL, RESTART_WIFI},
			{"wl_key", "", NULL, RESTART_WIFI},
			{"wl_key_type", "", NULL, RESTART_WIFI},
			{"wl_wpa_gtk_rekey", "", NULL, RESTART_WIFI},
			{"wl_closed", "", NULL, RESTART_WIFI},
			{"wl_ap_isolate", "", NULL, RESTART_WIFI},
			{"wl_mbssid_isolate", "", NULL, RESTART_WIFI},
			{"wl_rate", "", NULL, RESTART_WIFI},
			{"wl_IgmpSnEnable", "", NULL, RESTART_WIFI},
			{"wl_mcastrate", "", NULL, RESTART_WIFI},
			{"wl_rateset", "", NULL, RESTART_WIFI},
			{"wl_frag", "", NULL, RESTART_WIFI},
			{"wl_rts", "", NULL, RESTART_WIFI},
			{"wl_dtim", "", NULL, RESTART_WIFI},
			{"wl_bcn", "", NULL, RESTART_WIFI},
			{"wl_radio_x", "", NULL, RESTART_WIFI},
			{"wl_radio_date_x", "", NULL, RESTART_WIFI},
			{"wl_radio_time_x", "", NULL, RESTART_WIFI},
			{"wl_radio_time2_x", "", NULL, RESTART_WIFI},
			{"wl_wme", "", NULL, RESTART_WIFI},
			{"wl_wme_no_ack", "", NULL, RESTART_WIFI},
			{"wl_TxPower", "", NULL, RESTART_WIFI},
			{"wl_TxBurst", "", NULL, RESTART_WIFI},
			{"wl_PktAggregate", "", NULL, RESTART_WIFI},
			{"wl_APSDCapable", "", NULL, RESTART_WIFI},
			{"wl_DLSCapable", "", NULL, RESTART_WIFI},
			{"wl_HT_OpMode", "", NULL, RESTART_WIFI},
			{"wl_HT_BW", "", NULL, RESTART_WIFI},
			{"wl_HT_RDG", "", NULL, RESTART_WIFI},
			{"wl_HT_AMSDU", "", NULL, RESTART_WIFI},
			{"wl_HT_MpduDensity", "", NULL, RESTART_WIFI},
			{"wl_HT_BAWinSize", "", NULL, RESTART_WIFI},
			{"wl_country_code", "", NULL, RESTART_WIFI},
			{"wl_stream_tx", "", NULL, RESTART_WIFI},
			{"wl_stream_rx", "", NULL, RESTART_WIFI},
			{"wl_preamble", "", NULL, RESTART_WIFI},
			{"wl_greenap", "", NULL, RESTART_WIFI},
			{"wl_sta_ssid", "", NULL, RESTART_WIFI},
			{"wl_sta_auth_mode", "", NULL, RESTART_WIFI},
			{"wl_sta_wpa_mode", "", NULL, RESTART_WIFI},
			{"wl_sta_crypto", "", NULL, RESTART_WIFI},
			{"wl_sta_wpa_psk", "", NULL, RESTART_WIFI},
			{"wl_guest_enable", "", NULL, RESTART_WIFI},
			{"wl_guest_date_x", "", NULL, RESTART_WIFI},
			{"wl_guest_time_x", "", NULL, RESTART_WIFI},
			{"wl_guest_time2_x", "", NULL, RESTART_WIFI},
			{"wl_guest_ssid", "", NULL, RESTART_WIFI},
			{"wl_guest_closed", "", NULL, RESTART_WIFI},
			{"wl_guest_ap_isolate", "", NULL, RESTART_WIFI},
			{"wl_guest_lan_isolate", "", NULL, RESTART_WIFI},
			{"wl_guest_auth_mode", "", NULL, RESTART_WIFI},
			{"wl_guest_wpa_mode", "", NULL, RESTART_WIFI},
			{"wl_guest_crypto", "", NULL, RESTART_WIFI},
			{"wl_guest_wpa_psk", "", NULL, RESTART_WIFI},
			{"wl_guest_macrule", "", NULL, RESTART_WIFI},
			{"RBRList", "Group", ARGV((char*)variables_WLANConfig11a_RBRList, "16", "32", "wl_wdsnum_x"), RESTART_WIFI},
			{0,0,0,0}
		};

	struct variable variables_WLANConfig11b[] = {
			{"rt_ssid", "", NULL, RESTART_WIFI},
			{"rt_ssid2", "", NULL, RESTART_WIFI},
			{"rt_mode_x", "", NULL, RESTART_WIFI},
			{"rt_channel", "", NULL, RESTART_WIFI},
			{"rt_rateset", "", NULL, RESTART_WIFI},
			{"rt_bcn", "", NULL, RESTART_WIFI},
			{"rt_dtim", "", NULL, RESTART_WIFI},
			{"rt_gmode", "", NULL, RESTART_WIFI},
			{"rt_gmode_protection", "", NULL, RESTART_WIFI},
			{"rt_rts", "", NULL, RESTART_WIFI},
			{"rt_frag", "", NULL, RESTART_WIFI},
			{"rt_ap_isolate", "", NULL, RESTART_WIFI},
			{"rt_mbssid_isolate", "", NULL, RESTART_WIFI},
			{"rt_closed", "", NULL, RESTART_WIFI},
			{"rt_IgmpSnEnable", "", NULL, RESTART_WIFI},
			{"rt_mcastrate", "", NULL, RESTART_WIFI},
			{"rt_wme", "", NULL, RESTART_WIFI},
			{"rt_wme_no_ack", "", NULL, RESTART_WIFI},
			{"rt_auth_mode", "", NULL, RESTART_WIFI},
			{"rt_crypto", "", NULL, RESTART_WIFI},
			{"rt_wpa_psk", "", NULL, RESTART_WIFI},
			{"rt_wpa_gtk_rekey", "", NULL, RESTART_WIFI},
			{"rt_key", "", NULL, RESTART_WIFI},
			{"rt_key_type", "", NULL, RESTART_WIFI},
			{"rt_key1", "", NULL, RESTART_WIFI},
			{"rt_key2", "", NULL, RESTART_WIFI},
			{"rt_key3", "", NULL, RESTART_WIFI},
			{"rt_key4", "", NULL, RESTART_WIFI},
			{"rt_lazywds", "", NULL, RESTART_WIFI},
			{"rt_wdsapply_x", "", NULL, RESTART_WIFI},
			{"rt_wdsnum_x", "", NULL, RESTART_WIFI},
			{"rt_wep_x", "", NULL, RESTART_WIFI},
			{"rt_phrase_x", "", NULL, RESTART_WIFI},
			{"rt_radio_x", "", NULL, RESTART_WIFI},
			{"rt_radio_date_x", "", NULL, RESTART_WIFI},
			{"rt_radio_time_x", "", NULL, RESTART_WIFI},
			{"rt_radio_time2_x", "", NULL, RESTART_WIFI},
			{"rt_wpa_mode", "", NULL, RESTART_WIFI},
			{"rt_TxPower", "", NULL, RESTART_WIFI},
			{"rt_TxBurst", "", NULL, RESTART_WIFI},
			{"rt_PktAggregate", "", NULL, RESTART_WIFI},
			{"rt_APSDCapable", "", NULL, RESTART_WIFI},
			{"rt_DLSCapable", "", NULL, RESTART_WIFI},
			{"rt_HT_OpMode", "", NULL, RESTART_WIFI},
			{"rt_HT_BW", "", NULL, RESTART_WIFI},
			{"rt_HT_EXTCHA", "", NULL, RESTART_WIFI},
			{"rt_HT_RDG", "", NULL, RESTART_WIFI },
			{"rt_HT_AMSDU", "", NULL, RESTART_WIFI },
			{"rt_HT_MpduDensity", "", NULL, RESTART_WIFI },
			{"rt_HT_BAWinSize", "", NULL, RESTART_WIFI },
			{"rt_country_code", "", NULL, RESTART_WIFI},
			{"rt_stream_tx", "", NULL, RESTART_WIFI},
			{"rt_stream_rx", "", NULL, RESTART_WIFI},
			{"rt_preamble", "", NULL, RESTART_WIFI},
			{"rt_greenap", "", NULL, RESTART_WIFI},
			{"rt_sta_ssid", "", NULL, RESTART_WIFI},
			{"rt_sta_auth_mode", "", NULL, RESTART_WIFI},
			{"rt_sta_wpa_mode", "", NULL, RESTART_WIFI},
			{"rt_sta_crypto", "", NULL, RESTART_WIFI},
			{"rt_sta_wpa_psk", "", NULL, RESTART_WIFI},
			{"rt_guest_enable", "", NULL, RESTART_WIFI},
			{"rt_guest_date_x", "", NULL, RESTART_WIFI},
			{"rt_guest_time_x", "", NULL, RESTART_WIFI},
			{"rt_guest_time2_x", "", NULL, RESTART_WIFI},
			{"rt_guest_ssid", "", NULL, RESTART_WIFI},
			{"rt_guest_closed", "", NULL, RESTART_WIFI},
			{"rt_guest_ap_isolate", "", NULL, RESTART_WIFI},
			{"rt_guest_lan_isolate", "", NULL, RESTART_WIFI},
			{"rt_guest_auth_mode", "", NULL, RESTART_WIFI},
			{"rt_guest_wpa_mode", "", NULL, RESTART_WIFI},
			{"rt_guest_crypto", "", NULL, RESTART_WIFI},
			{"rt_guest_wpa_psk", "", NULL, RESTART_WIFI},
			{"rt_guest_macrule", "", NULL, RESTART_WIFI},
			{"rt_RBRList", "Group", ARGV((char*)variables_WLANConfig11b_rt_RBRList, "16", "32", "rt_wdsnum_x"), RESTART_WIFI},
			{0,0,0,0}
		};

	struct variable variables_PrinterStatus[] = {
#ifndef NOQOS 
			{"qos_rulenum_x", "8", NULL, RESTART_QOS},
			{"qos_enable", "", NULL, RESTART_QOS},
			{"qos_global_enable", "", NULL, RESTART_QOS},
			{"qos_service_enable", "", NULL, RESTART_QOS},
			{"qos_tos_prio", "", NULL, RESTART_QOS},
			{"qos_pshack_prio", "", NULL, RESTART_QOS},
			{"qos_shortpkt_prio", "", NULL, RESTART_QOS},
			{"qos_dfragment_enable", "", NULL, RESTART_QOS},
			{"qos_dfragment_size", "", NULL, RESTART_QOS},
			{"qos_service_ubw", "", NULL, RESTART_QOS},
			{"qos_manual_ubw", "", NULL, RESTART_QOS},
			{"qos_orules", "", NULL, RESTART_QOS },
			{"qos_default", "", NULL, RESTART_QOS},
			{"qos_orates", "", NULL, RESTART_QOS},
			{"qos_user_enable", "", NULL, RESTART_QOS},
			{"qos_inuse", "", NULL, RESTART_QOS},
			{"x_USRRuleList", "Group", ARGV((char*)variables_PrinterStatus_x_USRRuleList, "8", "47", "qos_rulenum_x"), RESTART_QOS},
#endif
			{0,0,0,0}
		};


	struct svcLink svcLinks[] = {
		{"General", "urn:schemas-upnp-org:service:General:1", variables_General},
		{"Storage", "urn:schemas-upnp-org:service:Storage:1", variables_Storage},
		{"LANHostConfig", "urn:schemas-upnp-org:service:Layer3Forwarding:1", variables_LANHostConfig},
		{"Layer3Forwarding", "urn:schemas-upnp-org:service:LANHostConfigManagement:0.8", variables_Layer3Forwarding},
		{"WANCommonInterface", "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1", variables_WANCommonInterface},
		{"IPConnection", "urn:schemas-upnp-org:service:WANIPConnection:1", variables_IPConnection},
		{"IP6Connection", "urn:schemas-upnp-org:service:WANIP6Connection:1", variables_IP6Connection},
		{"PPPConnection", "urn:schemas-upnp-org:service:WANPPPConnection:1", variables_PPPConnection},
		{"EthernetLink", "urn:schemas-upnp-org:service:WANEthernetLinkConfig:1", variables_EthernetLink},
		{"FirewallConfig", "urn:schemas-upnp-org:service:FirewallConfig:1", variables_FirewallConfig},
		{"RouterConfig", "urn:schemas-upnp-org:service:RouterConfig:1", variables_RouterConfig},
		{"WLANConfig11a", "urn:schemas-upnp-org:service:WLANConfiguration:1", variables_WLANConfig11a},
		{"WLANConfig11b", "urn:schemas-upnp-org:service:WLANConfiguration:1", variables_WLANConfig11b},
		{"DeviceSecurity11a", "urn:schemas-upnp-org:service:DeviceSecurity:1", variables_DeviceSecurity11a},
		{"DeviceSecurity11b", "urn:schemas-upnp-org:service:DeviceSecurity:1", variables_DeviceSecurity11b},
		{"WLANAuthentication11a", "urn:schemas-upnp-org:service:WLANAuthentication:1", variables_WLANAuthentication11a},
		{"WLANAuthentication11b", "urn:schemas-upnp-org:service:WLANAuthentication:1", variables_WLANAuthentication11b},
		{"PrinterStatus", "urn:schemas-upnp-org:service:PrinterStatus:1", variables_PrinterStatus},
		{"LANGUAGE", "urn:schemas-upnp-org:service:LANGUAGE:1", variables_Language},
		{0,0,0}
	};

