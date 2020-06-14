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
#include <sys/types.h>

#include <notify_rc.h>
#include <ralink_boards.h>

#include "common.h"

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
			{"vts_srcip_x", "16", NULL, FALSE},
			{"vts_desc_x", "20", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_FirewallConfig_LWFilterList[] = {
			{"filter_lw_srcip_x", "16", NULL, FALSE},
			{"filter_lw_srcport_x", "12", NULL, FALSE},
			{"filter_lw_dstip_x", "16", NULL, FALSE},
			{"filter_lw_dstport_x", "12", NULL, FALSE},
			{"filter_lw_proto_x", "8", NULL, FALSE},
			{"filter_lw_protono_x", "12", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_FirewallConfig_UrlList[] = {
			{"url_keyword_x", "36", NULL, EVM_RESTART_FIREWALL},
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
			{"vpns_rnet_x", "20", NULL, FALSE},
			{"vpns_rmsk_x", "20", NULL, FALSE},
			{0,0,0,0}
		};

#if BOARD_HAS_5G_RADIO
	struct variable variables_DeviceSecurity11a_ACLList[] = {
			{"wl_maclist_x", "32", NULL, FALSE},
			{"wl_macdesc_x", "48", NULL, FALSE},
			{0,0,0,0}
		};
#endif

	struct variable variables_DeviceSecurity11b_rt_ACLList[] = {
			{"rt_maclist_x", "32", NULL, FALSE},
			{"rt_macdesc_x", "48", NULL, FALSE},
			{0,0,0,0}
		};

#if BOARD_HAS_5G_RADIO
	struct variable variables_WLANConfig11a_RBRList[] = {
			{"wl_wdslist_x", "12", NULL, EVM_RESTART_WIFI5},
			{0,0,0,0}
		};
#endif

	struct variable variables_WLANConfig11b_rt_RBRList[] = {
			{"rt_wdslist_x", "12", NULL, EVM_RESTART_WIFI2},
			{0,0,0,0}
		};

	struct variable variables_General[] = {
			{"nvram_manual", "", NULL, FALSE},
			{"rstats_stored", "", NULL, FALSE},
			{"stime_stored", "", NULL, FALSE},
#if defined (USE_NAND_FLASH)
			{"mtd_rwfs_mount", "", NULL, FALSE},
#endif
#if defined (APP_TTYD)
			{"ttyd_enable", "", NULL, EVM_RESTART_TTYD},
			{"ttyd_port", "", NULL, EVM_RESTART_TTYD},
#endif
#if defined (APP_VLMCSD)
			{"vlmcsd_enable", "", NULL, EVM_RESTART_VLMCSD},
#endif
#if defined (APP_NAPT66)
			{"napt66_enable", "", NULL, FALSE},
#endif
			{"http_username", "", NULL, EVM_RESTART_CROND|EVM_BLOCK_UNSAFE},
			{"http_passwd", "", NULL, EVM_BLOCK_UNSAFE},
			{"modem_rule", "", NULL, EVM_RESTART_MODEM},
			{"modem_type", "", NULL, EVM_RESTART_MODEM},
			{"modem_country", "", NULL, EVM_RESTART_MODEM},
			{"modem_isp", "", NULL, EVM_RESTART_MODEM},
			{"modem_apn", "", NULL, EVM_RESTART_MODEM},
			{"modem_dialnum", "", NULL, EVM_RESTART_MODEM},
			{"modem_user", "", NULL, EVM_RESTART_MODEM},
			{"modem_pass", "", NULL, EVM_RESTART_MODEM},
			{"modem_pin", "", NULL, EVM_RESTART_MODEM},
			{"modem_dnsa", "", NULL, EVM_RESTART_MODEM},
			{"modem_node", "", NULL, EVM_RESTART_MODEM},
			{"modem_nets", "", NULL, EVM_RESTART_MODEM},
			{"modem_cmd", "", NULL, EVM_RESTART_MODEM},
			{"modem_mtu", "", NULL, EVM_RESTART_MODEM},
			{"modem_zcd", "", NULL, FALSE},
			{"di_poll_mode", "", NULL, EVM_RESTART_DI},
			{"di_timeout", "", NULL, FALSE },
			{"di_time_done", "", NULL, FALSE},
			{"di_time_fail", "", NULL, FALSE},
			{"di_lost_delay", "", NULL, FALSE},
			{"di_lost_action", "", NULL, FALSE},
			{"di_recon_pause", "", NULL, FALSE},
			{"di_addr0", "", NULL, EVM_RESTART_DI},
			{"di_addr1", "", NULL, EVM_RESTART_DI},
			{"di_addr2", "", NULL, EVM_RESTART_DI},
			{"di_addr3", "", NULL, EVM_RESTART_DI},
			{"di_addr4", "", NULL, EVM_RESTART_DI},
			{"di_addr5", "", NULL, EVM_RESTART_DI},
			{"di_port0", "", NULL, EVM_RESTART_DI},
			{"di_port1", "", NULL, EVM_RESTART_DI},
			{"di_port2", "", NULL, EVM_RESTART_DI},
			{"di_port3", "", NULL, EVM_RESTART_DI},
			{"di_port4", "", NULL, EVM_RESTART_DI},
			{"di_port5", "", NULL, EVM_RESTART_DI},
			{"ez_action_short", "", NULL, FALSE},
			{"ez_action_long", "", NULL, FALSE},
#if defined (BOARD_GPIO_BTN_FN1)
			{"fn1_action_short", "", NULL, FALSE},
			{"fn1_action_long", "", NULL, FALSE},
#endif
#if defined (BOARD_GPIO_BTN_FN2)
			{"fn2_action_short", "", NULL, FALSE},
			{"fn2_action_long", "", NULL, FALSE},
#endif
			{"front_led_all", "", NULL, EVM_RESTART_TWEAKS},
			{"front_led_wan", "", NULL, EVM_RESTART_TWEAKS},
			{"front_led_lan", "", NULL, EVM_RESTART_TWEAKS},
			{"front_led_usb", "", NULL, EVM_RESTART_TWEAKS},
			{"front_led_wif", "", NULL, EVM_RESTART_TWEAKS},
			{"front_led_pwr", "", NULL, EVM_RESTART_TWEAKS},
			{"watchdog_cpu", "", NULL, EVM_RESTART_WDG},
			{"u2ec_enable", "", NULL, EVM_RESTART_SPOOLER},
			{"lprd_enable", "", NULL, EVM_RESTART_SPOOLER},
			{"rawd_enable", "", NULL, EVM_RESTART_SPOOLER},
			{"help_enable", "", NULL, FALSE},
			{"scripts.start_script.sh", "File", NULL, EVM_BLOCK_UNSAFE},
			{"scripts.started_script.sh", "File", NULL, EVM_BLOCK_UNSAFE},
			{"scripts.shutdown_script.sh", "File", NULL, EVM_BLOCK_UNSAFE},
			{"scripts.post_wan_script.sh", "File", NULL, EVM_BLOCK_UNSAFE},
			{"scripts.post_iptables_script.sh", "File", NULL, EVM_RESTART_FIREWALL|EVM_BLOCK_UNSAFE},
			{"scripts.ez_buttons_script.sh", "File", NULL, EVM_BLOCK_UNSAFE},
			{"scripts.inet_state_script.sh", "File", NULL, FALSE},
			{0,0,0,0}
		};

	struct variable variables_Storage[] = {
			{"computer_name", "", NULL, EVM_RESTART_DHCPD|EVM_RESTART_FTPD|EVM_RESTART_NMBD|EVM_RESTART_DMS|EVM_RESTART_ITUNES},
#if defined(APP_SMBD)
			{"enable_samba", "", NULL, EVM_RESTART_SMBD},
			{"st_samba_fp", "", NULL, EVM_RESTART_SYSCTL},
			{"st_samba_mode", "", NULL, EVM_RESTART_SMBD},
#endif
#if defined(APP_SMBD) || defined(APP_NMBD)
			{"st_samba_lmb", "", NULL, EVM_RESTART_NMBD},
			{"st_samba_workgroup", "", NULL, EVM_RESTART_NMBD},
#endif
#if defined(APP_SMBD) || defined(APP_FTPD)
			{"st_max_user", "", NULL, EVM_RESTART_FTPD|EVM_RESTART_SMBD},
			{"acc_num", "", NULL, EVM_RESTART_FTPD|EVM_RESTART_SMBD},
#endif
#if defined(APP_FTPD)
			{"enable_ftp", "", NULL, EVM_RESTART_FTPD},
			{"st_ftp_mode", "", NULL, EVM_RESTART_FTPD},
			{"st_ftp_log", "", NULL, EVM_RESTART_FTPD},
			{"st_ftp_pmin", "", NULL, EVM_RESTART_FTPD},
			{"st_ftp_pmax", "", NULL, EVM_RESTART_FTPD},
			{"st_ftp_anmr", "", NULL, EVM_RESTART_FTPD},
#endif
			{"achk_enable", "", NULL, FALSE},
			{"optw_enable", "", NULL, FALSE},
			{"hdd_spindt", "", NULL, EVM_RESTART_HDDTUNE},
			{"hdd_apmoff", "", NULL, EVM_RESTART_HDDTUNE},
			{"pcache_reclaim", "", NULL, EVM_RESTART_HDDTUNE},
#if defined(APP_NFSD)
			{"nfsd_enable", "", NULL, EVM_RESTART_NFSD},
#endif
#if defined(APP_MINIDLNA)
			{"apps_dms", "", NULL, EVM_RESTART_DMS},
			{"dlna_disc", "", NULL, EVM_RESTART_DMS},
			{"dlna_root", "", NULL, EVM_RESTART_DMS},
			{"dlna_sort", "", NULL, EVM_RESTART_DMS},
			{"dlna_src1", "", NULL, EVM_RESTART_DMS},
			{"dlna_src2", "", NULL, EVM_RESTART_DMS},
			{"dlna_src3", "", NULL, EVM_RESTART_DMS},
			{"dlna_rescan", "", NULL, EVM_RESTART_DMS},
#endif
#if defined(APP_FIREFLY)
			{"apps_itunes", "", NULL, EVM_RESTART_ITUNES},
#endif
#if defined(APP_TRMD)
			{"trmd_enable", "", NULL,EVM_RESTART_TRMD},
			{"trmd_pport", "", NULL, EVM_RESTART_TRMD},
			{"trmd_rport", "", NULL, EVM_RESTART_TRMD},
#endif
#if defined(APP_ARIA)
			{"aria_enable", "", NULL,EVM_RESTART_ARIA},
			{"aria_pport", "", NULL, EVM_RESTART_ARIA},
			{"aria_rport", "", NULL, EVM_RESTART_ARIA},
#endif
#if (BOARD_NUM_UPHY_USB3 > 0)
			{"usb3_disable", "", NULL, EVM_RESTART_REBOOT},
#endif
			{"Storage_UserList", "Group", ARGV((char*)variables_Storage_UserList, "16", "79", "acc_num"), EVM_RESTART_FTPD|EVM_RESTART_SMBD},
			{0,0,0,0}
		};

	struct variable variables_Layer3Forwarding[] = {
			{"x_DHCPClient", "", NULL, EVM_RESTART_WAN},
			{"wan_proto", "", NULL, EVM_RESTART_WAN},
			{"wan_stb_x", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"wan_src_phy", "", NULL, FALSE},
			{"wan_stb_iso", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_filter", "", NULL, EVM_RESTART_WAN},
			{"vlan_vid_cpu", "", NULL, EVM_RESTART_WAN},
			{"vlan_pri_cpu", "", NULL, EVM_RESTART_WAN},
			{"vlan_vid_iptv", "", NULL, EVM_RESTART_WAN},
			{"vlan_pri_iptv", "", NULL, EVM_RESTART_WAN},
			{"vlan_vid_lan1", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_pri_lan1", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_tag_lan1", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_vid_lan2", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_pri_lan2", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_tag_lan2", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_vid_lan3", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_pri_lan3", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_tag_lan3", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_vid_lan4", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_pri_lan4", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"vlan_tag_lan4", "", NULL, EVM_RESTART_SWITCH_VLAN},
			{"wan_auth_mode", "", NULL, EVM_RESTART_WAN},
			{"wan_auth_user", "", NULL, EVM_RESTART_WAN},
			{"wan_auth_pass", "", NULL, EVM_RESTART_WAN},
			{"wan_auth_host", "", NULL, EVM_RESTART_WAN},
			{"wan_ttl_fix", "", NULL, EVM_RESTART_FIREWALL},
			{"wan_ttl_value", "", NULL, EVM_RESTART_FIREWALL},
			{"viptv_mode", "", NULL, EVM_RESTART_WAN},
			{"viptv_ipaddr", "", NULL, EVM_RESTART_WAN},
			{"viptv_netmask", "", NULL, EVM_RESTART_WAN},
			{"viptv_gateway", "", NULL, EVM_RESTART_WAN},
			{0,0,0,0}
		};

	struct variable variables_WANCommonInterface[] = {
			{0,0,0,0}
		};

	struct variable variables_IPConnection[] = {
			{"sw_mode", "", NULL, EVM_RESTART_REBOOT},
			{"gw_arp_ping", "", NULL, EVM_RESTART_WAN},
			{"hw_nat_mode", "", NULL, EVM_RESTART_FIREWALL},
			{"sw_nat_mode", "", NULL, EVM_RESTART_FIREWALL},
			{"sfe_enable", "", NULL, EVM_RESTART_FIREWALL},
			{"wan_ipaddr", "", NULL, EVM_RESTART_WAN},
			{"wan_netmask", "", NULL, EVM_RESTART_WAN},
			{"wan_gateway", "", NULL, EVM_RESTART_WAN},
			{"wan_mtu", "", NULL, EVM_RESTART_WAN},
			{"wan_dnsenable_x", "", NULL, EVM_RESTART_WAN},
			{"wan_dns1_x", "", NULL, EVM_RESTART_WAN},
			{"wan_dns2_x", "", NULL, EVM_RESTART_WAN},
			{"wan_dns3_x", "", NULL, EVM_RESTART_WAN},
			{"wan_hostname", "", NULL, EVM_RESTART_WAN},
			{"wan_vci", "", NULL, EVM_RESTART_WAN},
			{"wan_hwaddr_x", "", NULL, EVM_RESTART_WAN},
			{"upnp_enable_x", "", NULL, EVM_RESTART_UPNP},
			{"upnp_proto", "", NULL, EVM_RESTART_UPNP},
			{"upnp_secure", "", NULL, EVM_RESTART_UPNP},
			{"upnp_clean_min", "", NULL, EVM_RESTART_UPNP},
			{"upnp_clean_int", "", NULL, EVM_RESTART_UPNP},
			{"upnp_iport_min", "", NULL, EVM_RESTART_UPNP},
			{"upnp_iport_max", "", NULL, EVM_RESTART_UPNP},
			{"upnp_eport_min", "", NULL, EVM_RESTART_UPNP},
			{"upnp_eport_max", "", NULL, EVM_RESTART_UPNP},
			{"dmz_ip", "", NULL, EVM_RESTART_FIREWALL},
			{"sp_battle_ips", "", NULL, EVM_RESTART_FIREWALL},
			{"vts_enable_x", "", NULL, EVM_RESTART_FIREWALL},
			{"vts_num_x", "", NULL, EVM_RESTART_FIREWALL},
			{"VSList", "Group", ARGV((char*)variables_IPConnection_VSList, "24", "75", "vts_num_x"), EVM_RESTART_FIREWALL},
			{0,0,0,0}
		};

	struct variable variables_IP6Connection[] = {
#if defined (USE_IPV6)
			{"ip6_service", "", NULL, EVM_RESTART_IPV6},
			{"ip6_ppe_on", "", NULL, EVM_RESTART_FIREWALL},
			{"ip6_wan_if", "", NULL, EVM_RESTART_IPV6},
			{"ip6_6in4_remote", "", NULL, EVM_RESTART_IPV6},
			{"ip6_6to4_relay", "", NULL, EVM_RESTART_IPV6},
			{"ip6_6rd_relay", "", NULL, EVM_RESTART_IPV6},
			{"ip6_6rd_size", "", NULL, EVM_RESTART_IPV6},
			{"ip6_6rd_dhcp", "", NULL, EVM_RESTART_IPV6},
			{"ip6_sit_mtu", "", NULL, EVM_RESTART_IPV6},
			{"ip6_sit_ttl", "", NULL, EVM_RESTART_IPV6},
			{"ip6_wan_dhcp", "", NULL, EVM_RESTART_IPV6},
			{"ip6_wan_priv", "", NULL, EVM_RESTART_IPV6},
			{"ip6_wan_addr", "", NULL, EVM_RESTART_IPV6},
			{"ip6_wan_size", "", NULL, EVM_RESTART_IPV6},
			{"ip6_wan_gate", "", NULL, EVM_RESTART_IPV6},
			{"ip6_dns_auto", "", NULL, EVM_RESTART_IPV6},
			{"ip6_dns1", "", NULL, EVM_RESTART_IPV6},
			{"ip6_dns2", "", NULL, EVM_RESTART_IPV6},
			{"ip6_dns3", "", NULL, EVM_RESTART_IPV6},
			{"ip6_lan_auto", "", NULL, EVM_RESTART_IPV6},
			{"ip6_lan_addr", "", NULL, EVM_RESTART_IPV6},
			{"ip6_lan_size", "", NULL, EVM_RESTART_IPV6},
			{"ip6_lan_radv", "", NULL, EVM_RESTART_RADV},
			{"ip6_lan_dhcp", "", NULL, EVM_RESTART_RADV|EVM_RESTART_DHCPD},
			{"ip6_lan_sflt", "", NULL, EVM_RESTART_DHCPD},
			{"ip6_lan_sfps", "", NULL, EVM_RESTART_DHCPD},
			{"ip6_lan_sfpe", "", NULL, EVM_RESTART_DHCPD},
#endif
			{0,0,0,0}
		};

	struct variable variables_PPPConnection[] = {
			{"wan_pppoe_username", "", NULL, EVM_RESTART_WAN},
			{"wan_pppoe_passwd", "", NULL, EVM_RESTART_WAN},
			{"wan_pppoe_idletime", "", NULL, EVM_RESTART_WAN},
			{"wan_pppoe_txonly_x", "", NULL, EVM_RESTART_WAN},
			{"wan_pppoe_service", "", NULL, EVM_RESTART_WAN},
			{"wan_pppoe_ac", "", NULL, EVM_RESTART_WAN},
			{"wan_pppoe_man", "", NULL, EVM_RESTART_WAN},
			{"wan_pppoe_mtu", "", NULL, EVM_RESTART_WAN},
			{"wan_pppoe_mru", "", NULL, EVM_RESTART_WAN},
			{"wan_pptp_mtu", "", NULL, EVM_RESTART_WAN},
			{"wan_pptp_mru", "", NULL, EVM_RESTART_WAN},
			{"wan_l2tp_mtu", "", NULL, EVM_RESTART_WAN},
			{"wan_l2tp_mru", "", NULL, EVM_RESTART_WAN},
#if defined (APP_RPL2TP)
			{"wan_l2tpd", "", NULL, EVM_RESTART_WAN},
#endif
			{"wan_ppp_peer", "", NULL, EVM_RESTART_WAN},
			{"wan_ppp_auth", "", NULL, EVM_RESTART_WAN},
			{"wan_ppp_mppe", "", NULL, EVM_RESTART_WAN},
			{"wan_ppp_lcp", "", NULL, EVM_RESTART_WAN},
			{"wan_ppp_alcp", "", NULL, EVM_RESTART_WAN},
			{"wan_ppp_pppd", "", NULL, EVM_RESTART_WAN},
			{0,0,0,0}
		};

	struct variable variables_EthernetLink[] = {
			{0,0,0,0}
		};

	struct variable variables_FirewallConfig[] = {
			{"fw_enable_x", "", NULL, EVM_RESTART_FIREWALL},
			{"fw_dos_x", "", NULL, EVM_RESTART_FIREWALL},
			{"fw_log_x", "", NULL, EVM_RESTART_FIREWALL},
			{"misc_http_x", "", NULL, EVM_RESTART_FIREWALL},
			{"misc_httpport_x", "", NULL, EVM_RESTART_FIREWALL},
#if defined (SUPPORT_HTTPS)
			{"https_wopen", "", NULL, EVM_RESTART_FIREWALL},
			{"https_wport", "", NULL, EVM_RESTART_FIREWALL},
#endif
			{"udpxy_wopen", "", NULL, EVM_RESTART_FIREWALL},
			{"udpxy_wport", "", NULL, EVM_RESTART_FIREWALL},
			{"misc_lpr_x", "", NULL, EVM_RESTART_FIREWALL},
			{"misc_ping_x", "", NULL, EVM_RESTART_FIREWALL},
			{"fw_pt_pppoe", "", NULL, EVM_RESTART_SYSCTL},
			{"fw_lw_enable_x", "", NULL, EVM_RESTART_FIREWALL},
			{"filter_lw_date_x", "", NULL, EVM_RESTART_FIREWALL},
			{"filter_lw_time_x", "", NULL, EVM_RESTART_FIREWALL},
			{"filter_lw_default_x", "", NULL, EVM_RESTART_FIREWALL},
			{"filter_lw_icmp_x", "", NULL,  EVM_RESTART_FIREWALL},
			{"filter_lw_num_x", "", NULL, EVM_RESTART_FIREWALL},
			{"url_enable_x", "", NULL, EVM_RESTART_FIREWALL},
			{"url_date_x", "", NULL, EVM_RESTART_FIREWALL},
			{"url_time_x", "", NULL, EVM_RESTART_FIREWALL},
			{"url_mac_x", "", NULL, EVM_RESTART_FIREWALL},
			{"url_inv_x", "", NULL, EVM_RESTART_FIREWALL},
			{"url_num_x", "", NULL, EVM_RESTART_FIREWALL},
			{"macfilter_enable_x", "", NULL, EVM_RESTART_FIREWALL},
			{"macfilter_num_x", "", NULL, EVM_RESTART_FIREWALL},
			{"ftpd_wopen", "", NULL, EVM_RESTART_FIREWALL},
			{"ftpd_wport", "", NULL, EVM_RESTART_FIREWALL},
			{"sshd_wopen", "", NULL, EVM_RESTART_FIREWALL},
			{"sshd_wbfp", "", NULL, EVM_RESTART_FIREWALL},
			{"sshd_wport", "", NULL, EVM_RESTART_FIREWALL},
			{"trmd_ropen", "", NULL, EVM_RESTART_FIREWALL},
			{"aria_ropen", "", NULL, EVM_RESTART_FIREWALL},
			{"fw_syn_cook", "", NULL, EVM_RESTART_SYSCTL},
			{"fw_mac_drop", "", NULL, EVM_RESTART_FIREWALL},
			{"wan_nat_x", "", NULL, EVM_RESTART_NETFILTER},
			{"nf_nat_loop", "", NULL, EVM_RESTART_NETFILTER},
			{"nf_nat_type", "", NULL, EVM_RESTART_SYSCTL},
			{"nf_max_conn", "", NULL, EVM_RESTART_SYSCTL},
			{"nf_alg_ftp0", "", NULL, EVM_RESTART_FIREWALL},
			{"nf_alg_ftp1", "", NULL, EVM_RESTART_FIREWALL},
			{"nf_alg_pptp", "", NULL, EVM_RESTART_FIREWALL},
			{"nf_alg_h323", "", NULL, EVM_RESTART_FIREWALL},
			{"nf_alg_rtsp", "", NULL, EVM_RESTART_FIREWALL},
			{"nf_alg_sip", "", NULL, EVM_RESTART_FIREWALL},
			{"LWFilterList", "Group", ARGV((char*)variables_FirewallConfig_LWFilterList, "32", "63", "filter_lw_num_x"), EVM_RESTART_FIREWALL},
			{"UrlList", "Group", ARGV((char*)variables_FirewallConfig_UrlList, "128", "36", "url_num_x"), EVM_RESTART_FIREWALL},
			{"MFList", "Group", ARGV((char*)variables_FirewallConfig_MFList, "16", "32", "macfilter_num_x"), EVM_RESTART_FIREWALL},
			{0,0,0,0}
		};

	struct variable variables_RouterConfig[] = {
			{"sr_enable_x", "", NULL, EVM_RESTART_WAN},
			{"dr_enable_x", "", NULL, EVM_RESTART_WAN},
			{"mr_enable_x", "", NULL, EVM_RESTART_IPTV},
			{"mr_qleave_x", "", NULL, EVM_RESTART_IPTV},
			{"sr_rip_x", "", NULL, EVM_RESTART_WAN},
			{"sr_num_x", "", NULL, EVM_RESTART_WAN},
			{"dr_static_rip_x", "", NULL, EVM_RESTART_WAN},
			{"dr_static_matric_x", "", NULL, EVM_RESTART_WAN},
			{"dr_default_x", "", NULL, FALSE},
			{"dr_static_rip_x", "", NULL, FALSE},
			{"dr_staticnum_x", "", NULL, EVM_RESTART_WAN},
			{"GWStatic", "Group", ARGV((char*)variables_RouterConfig_GWStatic, "6", "59", "sr_num_x"), EVM_RESTART_WAN},
			{0,0,0,0}
		};

	struct variable variables_LANHostConfig[] = {
			{"lan_proto_x", "", NULL, EVM_RESTART_LAN},
			{"lan_dhcpd_x", "", NULL, EVM_RESTART_DHCPD},
			{"lan_ipaddr", "", NULL, EVM_RESTART_LAN},
			{"lan_netmask", "", NULL, EVM_RESTART_LAN},
			{"lan_gateway", "", NULL, EVM_RESTART_LAN},
			{"lan_dns_x", "", NULL, EVM_RESTART_LAN},
			{"lan_dns1", "", NULL, EVM_RESTART_LAN},
			{"lan_dns2", "", NULL, EVM_RESTART_LAN},
			{"lan_stp", "", NULL, EVM_RESTART_LAN},
			{"lan_domain", "", NULL, EVM_RESTART_DHCPD},
			{"force_igmp", "", NULL, EVM_RESTART_SYSCTL},
#if defined (USE_IPV6)
			{"force_mld", "", NULL, EVM_RESTART_SYSCTL},
#endif
			{"udpxy_enable_x", "", NULL, EVM_RESTART_IPTV},
			{"xupnpd_enable_x", "", NULL, EVM_RESTART_IPTV},
			{"xupnpd_udpxy", "", NULL, EVM_RESTART_IPTV},
			{"dhcp_enable_x", "", NULL, EVM_RESTART_DHCPD|EVM_REAPPLY_VPNSVR},
			{"dhcp_start", "", NULL, EVM_RESTART_DHCPD},
			{"dhcp_end", "", NULL, EVM_RESTART_DHCPD},
			{"dhcp_lease", "", NULL, EVM_RESTART_DHCPD},
			{"dhcp_gateway_x", "", NULL, EVM_RESTART_DHCPD},
			{"dhcp_dns1_x", "", NULL, EVM_RESTART_DHCPD|EVM_REAPPLY_VPNSVR},
			{"dhcp_dns2_x", "", NULL, EVM_RESTART_DHCPD|EVM_REAPPLY_VPNSVR},
			{"dhcp_dns3_x", "", NULL, EVM_RESTART_DHCPD},
			{"dhcp_dnsv6_x", "", NULL, EVM_RESTART_DHCPD},
			{"dhcp_wins_x", "", NULL, EVM_RESTART_DHCPD|EVM_REAPPLY_VPNSVR},
			{"dhcp_verbose", "", NULL, EVM_RESTART_DHCPD},
			{"dhcp_static_x", "", NULL, EVM_RESTART_DHCPD},
			{"dhcp_static_arp", "", NULL, EVM_RESTART_DHCPD},
			{"dhcp_staticnum_x", "", NULL, EVM_RESTART_DHCPD},
			{"dnsmasq.hosts", "File", NULL, EVM_RESTART_DHCPD},
			{"dnsmasq.dnsmasq.conf", "File", NULL, EVM_RESTART_DHCPD},
			{"dnsmasq.dhcp.conf", "File", NULL, EVM_RESTART_DHCPD},
			{"http_access", "", NULL, EVM_RESTART_HTTPD},
			{"http_proto", "", NULL, EVM_RESTART_HTTPD},
			{"http_lanport", "", NULL, EVM_RESTART_HTTPD},
#if defined (SUPPORT_HTTPS)
			{"https_lport", "", NULL, EVM_RESTART_HTTPD},
			{"https_clist", "", NULL, EVM_RESTART_HTTPD},
			{"httpssl.ca.crt", "File", NULL, EVM_RESTART_HTTPD|EVM_BLOCK_UNSAFE},
			{"httpssl.server.crt", "File", NULL, EVM_RESTART_HTTPD|EVM_BLOCK_UNSAFE},
			{"httpssl.server.key", "File", NULL, EVM_RESTART_HTTPD|EVM_BLOCK_UNSAFE},
			{"httpssl.dh1024.pem", "File", NULL, EVM_RESTART_HTTPD|EVM_BLOCK_UNSAFE},
#endif
			{"telnetd", "", NULL, EVM_RESTART_TELNETD},
			{"sshd_enable", "", NULL, EVM_RESTART_SSHD},
			{"wins_enable", "", NULL, EVM_RESTART_WINS|EVM_REAPPLY_VPNSVR},
			{"lltd_enable", "", NULL, EVM_RESTART_LLTD},
			{"adsc_enable", "", NULL, EVM_RESTART_ADSC},
			{"crond_enable", "", NULL, EVM_RESTART_CROND},
			{"crond_log", "", NULL, EVM_RESTART_CROND},
			{"crontab.login", "File", NULL, EVM_RESTART_CROND|EVM_BLOCK_UNSAFE},
			{"scripts.authorized_keys", "File", NULL, EVM_RESTART_SSHD|EVM_BLOCK_UNSAFE},
			{"ether_igmp", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_uport", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_m2u", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_led0", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_led1", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_jumbo", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_green", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_eee", "", NULL, EVM_RESTART_SWITCH_CFG},

			{"ether_link_wan",  "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_flow_wan",  "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_link_lan1", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_flow_lan1", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_link_lan2", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_flow_lan2", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_link_lan3", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_flow_lan3", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_link_lan4", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_flow_lan4", "", NULL, EVM_RESTART_SWITCH_CFG},
#if BOARD_NUM_ETH_EPHY > 5
			{"ether_link_lan5", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_flow_lan5", "", NULL, EVM_RESTART_SWITCH_CFG},
#if BOARD_NUM_ETH_EPHY > 6
			{"ether_link_lan6", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_flow_lan6", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_link_lan7", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"ether_flow_lan7", "", NULL, EVM_RESTART_SWITCH_CFG},
#endif
#endif
			{"controlrate_unknown_unicast", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"controlrate_unknown_multicast", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"controlrate_multicast", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"controlrate_broadcast", "", NULL, EVM_RESTART_SWITCH_CFG},
			{"vpns_enable", "", NULL, EVM_RESTART_VPNSVR},
			{"vpns_type", "", NULL, EVM_RESTART_VPNSVR},
			{"vpns_cast", "", NULL, FALSE},
			{"vpns_auth", "", NULL, EVM_REAPPLY_VPNSVR},
			{"vpns_mppe", "", NULL, EVM_REAPPLY_VPNSVR},
			{"vpns_mtu", "", NULL, EVM_REAPPLY_VPNSVR},
			{"vpns_mru", "", NULL, EVM_REAPPLY_VPNSVR},
			{"vpns_vuse", "", NULL, EVM_RESTART_VPNSVR},
			{"vpns_vnet", "", NULL, EVM_RESTART_VPNSVR},
			{"vpns_actl", "", NULL, EVM_RESTART_FIREWALL},
			{"vpns_cli0", "", NULL, EVM_RESTART_VPNSVR},
			{"vpns_cli1", "", NULL, EVM_RESTART_VPNSVR},
			{"vpnc_enable", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_type", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_peer", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_user", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_pass", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_auth", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_mppe", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_pppd", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_pdns", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_mtu", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_mru", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_sfw", "", NULL, EVM_RESTART_FIREWALL},
			{"vpnc_dgw", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_rnet", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_rmsk", "", NULL, EVM_RESTART_VPNCLI},
#if defined(APP_OPENVPN)
			{"vpns_ov_mode", "", NULL, EVM_RESTART_VPNSVR},
			{"vpns_ov_prot", "", NULL, EVM_RESTART_VPNSVR},
			{"vpns_ov_port", "", NULL, EVM_RESTART_VPNSVR},
			{"vpns_ov_mdig", "", NULL, EVM_REAPPLY_VPNSVR},
			{"vpns_ov_ciph", "", NULL, EVM_REAPPLY_VPNSVR},
			{"vpns_ov_clzo", "", NULL, EVM_REAPPLY_VPNSVR},
			{"vpns_ov_atls", "", NULL, EVM_REAPPLY_VPNSVR},
			{"vpns_ov_rdgw", "", NULL, EVM_REAPPLY_VPNSVR},
			{"vpnc_ov_mode", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_ov_cnat", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_ov_prot", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_ov_port", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_ov_auth", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_ov_mdig", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_ov_ciph", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_ov_clzo", "", NULL, EVM_RESTART_VPNCLI},
			{"vpnc_ov_atls", "", NULL, EVM_RESTART_VPNCLI},
			{"ovpnsvr.ca.crt", "File", NULL, EVM_REAPPLY_VPNSVR|EVM_BLOCK_UNSAFE},
			{"ovpnsvr.server.conf", "File", NULL, EVM_REAPPLY_VPNSVR},
			{"ovpnsvr.server.crt", "File", NULL, EVM_REAPPLY_VPNSVR|EVM_BLOCK_UNSAFE},
			{"ovpnsvr.server.key", "File", NULL, EVM_REAPPLY_VPNSVR|EVM_BLOCK_UNSAFE},
			{"ovpnsvr.dh1024.pem", "File", NULL, EVM_REAPPLY_VPNSVR|EVM_BLOCK_UNSAFE},
			{"ovpnsvr.ta.key", "File", NULL, EVM_REAPPLY_VPNSVR|EVM_BLOCK_UNSAFE},
			{"ovpncli.ca.crt", "File", NULL, EVM_RESTART_VPNCLI},
			{"ovpncli.client.conf", "File", NULL, EVM_RESTART_VPNCLI},
			{"ovpncli.client.crt", "File", NULL, EVM_RESTART_VPNCLI},
			{"ovpncli.client.key", "File", NULL, EVM_RESTART_VPNCLI},
			{"ovpncli.ta.key", "File", NULL, EVM_RESTART_VPNCLI},
#endif
			{"scripts.vpns_client_script.sh", "File", NULL, FALSE},
			{"scripts.vpnc_server_script.sh", "File", NULL, EVM_RESTART_VPNCLI},
			{"log_ipaddr", "", NULL, EVM_RESTART_SYSLOG},
			{"log_port", "", NULL, EVM_RESTART_SYSLOG},
			{"log_float_ui", "", NULL, FALSE},
			{"time_zone", "", NULL, EVM_RESTART_TIME},
			{"ntp_period", "", NULL, FALSE},
			{"ntp_server0", "", NULL, EVM_RESTART_NTPC},
			{"ntp_server1", "", NULL, EVM_RESTART_NTPC},
			{"ddns_enable_x", "", NULL, EVM_RESTART_DDNS},
			{"asusddns_tos_agreement", "", NULL, FALSE},
			{"ddns_server_x", "", NULL, EVM_RESTART_DDNS},
			{"ddns_username_x", "", NULL, EVM_RESTART_DDNS},
			{"ddns_passwd_x", "", NULL, EVM_RESTART_DDNS},
			{"ddns_hostname_x", "", NULL, EVM_RESTART_DDNS},
			{"ddns_hostname2_x", "", NULL, EVM_RESTART_DDNS},
			{"ddns_hostname3_x", "", NULL, EVM_RESTART_DDNS},
			{"ddns_wildcard_x", "", NULL, EVM_RESTART_DDNS},
			{"ddns_cst_svr", "", NULL, EVM_RESTART_DDNS },
			{"ddns_cst_url", "", NULL, EVM_RESTART_DDNS },
			{"ddns_period", "", NULL, EVM_RESTART_DDNS },
			{"ddns_forced", "", NULL, EVM_RESTART_DDNS },
			{"ddns_verbose", "", NULL, EVM_RESTART_DDNS },
			{"ddns_source", "", NULL, EVM_RESTART_DDNS },
			{"ddns_checkip", "", NULL, EVM_RESTART_DDNS },
#if defined (SUPPORT_DDNS_SSL)
			{"ddns_ssl", "", NULL, EVM_RESTART_DDNS },
			{"ddns2_ssl", "", NULL, EVM_RESTART_DDNS },
#endif
			{"ddns2_server", "", NULL, EVM_RESTART_DDNS },
			{"ddns2_hname", "", NULL, EVM_RESTART_DDNS },
			{"ddns2_user", "", NULL, EVM_RESTART_DDNS },
			{"ddns2_pass", "", NULL, EVM_RESTART_DDNS },
			{"ManualDHCPList", "Group", ARGV((char*)variables_LANHostConfig_ManualDHCPList, "8", "55", "dhcp_staticnum_x"), EVM_RESTART_DHCPD},
			{"VPNSACLList", "Group", ARGV((char*)variables_LANHostConfig_VPNSACLList, "8", "107", "vpns_num_x"), EVM_RESTART_VPNSVR},
			{0,0,0,0}
		};

	struct variable variables_DeviceSecurity11a[] = {
#if BOARD_HAS_5G_RADIO
			{"wl_macmode", "", NULL, EVM_RESTART_WIFI5},
			{"wl_macnum_x", "", NULL, EVM_RESTART_WIFI5},
			{"ACLList", "Group", ARGV((char*)variables_DeviceSecurity11a_ACLList, "64", "32", "wl_macnum_x"), EVM_RESTART_WIFI5},
#endif
			{0,0,0,0}
		};

	struct variable variables_DeviceSecurity11b[] = {
			{"rt_macmode", "", NULL, EVM_RESTART_WIFI2},
			{"rt_macnum_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_ACLList", "Group", ARGV((char*)variables_DeviceSecurity11b_rt_ACLList, "64", "32", "rt_macnum_x"), EVM_RESTART_WIFI2},
			{0,0,0,0}
		};

	struct variable variables_WLANAuthentication11a[] = {
#if BOARD_HAS_5G_RADIO
			{"wl_radius_ipaddr", "", NULL, EVM_RESTART_WIFI5},
			{"wl_radius_port", "", NULL, EVM_RESTART_WIFI5},
			{"wl_radius_key", "", NULL, EVM_RESTART_WIFI5},
#endif
			{0,0,0,0}
		};

	struct variable variables_WLANAuthentication11b[] = {
			{"rt_radius_ipaddr", "", NULL, EVM_RESTART_WIFI2},
			{"rt_radius_port", "", NULL, EVM_RESTART_WIFI2},
			{"rt_radius_key", "", NULL, EVM_RESTART_WIFI2},
			{0,0,0,0}
		};

	struct variable variables_WLANConfig11a[] = {
#if BOARD_HAS_5G_RADIO
			{"wl_ssid", "", NULL, EVM_RESTART_WIFI5},
			{"wl_ssid2", "", NULL, EVM_RESTART_WIFI5},
			{"wl_mode_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_channel", "", NULL, EVM_RESTART_WIFI5},
			{"wl_wdsapply_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_wdsnum_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_maxassoc", "", NULL, EVM_RESTART_WIFI5},
			{"wl_gmode", "", NULL, EVM_RESTART_WIFI5},
			{"wl_mcs_mode", "", NULL, EVM_RESTART_WIFI5},
			{"wl_txbf", "", NULL, EVM_RESTART_WIFI5},
			{"wl_auth_mode", "", NULL, EVM_RESTART_WIFI5},
			{"wl_wpa_mode", "", NULL, EVM_RESTART_WIFI5},
			{"wl_crypto", "", NULL, EVM_RESTART_WIFI5},
			{"wl_wpa_psk", "", NULL, EVM_RESTART_WIFI5},
			{"wl_wep_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_phrase_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_key1", "", NULL, EVM_RESTART_WIFI5},
			{"wl_key2", "", NULL, EVM_RESTART_WIFI5},
			{"wl_key3", "", NULL, EVM_RESTART_WIFI5},
			{"wl_key4", "", NULL, EVM_RESTART_WIFI5},
			{"wl_key", "", NULL, EVM_RESTART_WIFI5},
			{"wl_key_type", "", NULL, EVM_RESTART_WIFI5},
			{"wl_wpa_gtk_rekey", "", NULL, EVM_RESTART_WIFI5},
			{"wl_closed", "", NULL, EVM_RESTART_WIFI5},
			{"wl_ap_isolate", "", NULL, EVM_RESTART_WIFI5},
			{"wl_rate", "", NULL, EVM_RESTART_WIFI5},
			{"wl_IgmpSnEnable", "", NULL, EVM_RESTART_WIFI5},
			{"wl_mrate", "", NULL, EVM_RESTART_WIFI5},
			{"wl_frag", "", NULL, EVM_RESTART_WIFI5},
			{"wl_rts", "", NULL, EVM_RESTART_WIFI5},
			{"wl_dtim", "", NULL, EVM_RESTART_WIFI5},
			{"wl_bcn", "", NULL, EVM_RESTART_WIFI5},
			{"wl_radio_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_radio_date_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_radio_time_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_radio_time2_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_wme", "", NULL, EVM_RESTART_WIFI5},
			{"wl_wme_no_ack", "", NULL, EVM_RESTART_WIFI5},
			{"wl_TxPower", "", NULL, EVM_RESTART_WIFI5},
			{"wl_TxBurst", "", NULL, EVM_RESTART_WIFI5},
			{"wl_PktAggregate", "", NULL, EVM_RESTART_WIFI5},
			{"wl_APSDCapable", "", NULL, EVM_RESTART_WIFI5},
			{"wl_HT_OpMode", "", NULL, EVM_RESTART_WIFI5},
			{"wl_HT_BW", "", NULL, EVM_RESTART_WIFI5},
			{"wl_HT_RDG", "", NULL, EVM_RESTART_WIFI5},
			{"wl_HT_AMSDU", "", NULL, EVM_RESTART_WIFI5},
			{"wl_HT_MpduDensity", "", NULL, EVM_RESTART_WIFI5},
			{"wl_HT_BAWinSize", "", NULL, EVM_RESTART_WIFI5},
			{"wl_HT_AutoBA", "", NULL, EVM_RESTART_WIFI5},
#if defined(USE_MT76X2_AP)
			{"wl_VgaClamp", "", NULL, EVM_RESTART_WIFI5},
#endif
#if defined (USE_WID_5G) && USE_WID_5G==7615
			{"wl_mumimo", "", NULL, EVM_RESTART_WIFI5},
#endif
			{"wl_country_code", "", NULL, EVM_RESTART_WIFI5},
			{"wl_stream_tx", "", NULL, EVM_RESTART_WIFI5},
			{"wl_stream_rx", "", NULL, EVM_RESTART_WIFI5},
			{"wl_preamble", "", NULL, EVM_RESTART_WIFI5},
			{"wl_greenap", "", NULL, EVM_RESTART_WIFI5},
			{"wl_ldpc", "", NULL, EVM_RESTART_WIFI5},
			{"wl_sta_ssid", "", NULL, EVM_RESTART_WIFI5},
			{"wl_sta_auth_mode", "", NULL, EVM_RESTART_WIFI5},
			{"wl_sta_wpa_mode", "", NULL, EVM_RESTART_WIFI5},
			{"wl_sta_crypto", "", NULL, EVM_RESTART_WIFI5},
			{"wl_sta_wpa_psk", "", NULL, EVM_RESTART_WIFI5},
			{"wl_sta_wisp", "", NULL, EVM_RESTART_WIFI5},
			{"wl_sta_auto", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_enable", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_date_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_time_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_time2_x", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_ssid", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_closed", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_ap_isolate", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_lan_isolate", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_auth_mode", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_wpa_mode", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_crypto", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_wpa_psk", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_macrule", "", NULL, EVM_RESTART_WIFI5},
			{"wl_guest_mcs_mode", "", NULL, EVM_RESTART_WIFI5},
			{"wl_KickStaRssiLow", "", NULL, EVM_RESTART_WIFI5},
			{"wl_AssocReqRssiThres", "", NULL, EVM_RESTART_WIFI5},
			{"RBRList", "Group", ARGV((char*)variables_WLANConfig11a_RBRList, "16", "32", "wl_wdsnum_x"), EVM_RESTART_WIFI5},
#endif
			{0,0,0,0}
		};

#if defined (APP_SCUT)
	struct variable variables_ScutclientConf[] = {
			{"scutclient_enable", "",NULL, EVM_RESTART_SCUT},
			{"scutclient_debug","",NULL,EVM_RESTART_SCUT },
			{"scutclient_username","",NULL,EVM_RESTART_SCUT },
			{"scutclient_password","",NULL,EVM_RESTART_SCUT },
			{"scutclient_server_auth_ip","",NULL,EVM_RESTART_SCUT },
			{"scutclient_version","",NULL,EVM_RESTART_SCUT },
			{"scutclient_hash","",NULL,EVM_RESTART_SCUT },
			{"scutclient_hostname","",NULL,EVM_RESTART_SCUT },
			{"scutclient_watchcat","",NULL,FALSE},
			{"scutclient_wdg_force","",NULL,FALSE},
			{"scutclient_skip_udp_hb","",NULL,EVM_RESTART_SCUT},
			{0,0,0,0}
	};
#endif

#if defined (APP_MENTOHUST)
	struct variable variables_mentohustConf[] = {
			{"mentohust_enable", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_username", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_password", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_nic", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_ip", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_mask", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_gw", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_dns", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_pinghost", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_timeout", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_interval", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_restart_wait", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_maxfail", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_startmode", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_dhcp", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_daemon", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_ver", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_datafile", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_dhcpscript", "", NULL, EVM_RESTART_MENTOHUST},
			{"mentohust_service", "", NULL, EVM_RESTART_MENTOHUST},
			{0,0,0,0}
	};
#endif

#if defined(APP_DNSFORWARDER)
	struct variable variables_dnsforwarderConf[] = {
			{"dns_forwarder_enable", "", NULL, EVM_RESTART_DNSFORWARDER},
			{"dns_forwarder_bind", "", NULL, EVM_RESTART_DNSFORWARDER},
			{"dns_forwarder_port", "", NULL, EVM_RESTART_DNSFORWARDER},
			{"dns_forwarder_server", "", NULL, EVM_RESTART_DNSFORWARDER},
			{0,0,0,0}
	};
#endif

#if defined(APP_SHADOWSOCKS)
	struct variable variables_ShadowsocksConf[] = {
			{"ss_enable","",NULL, EVM_RESTART_SHADOWSOCKS},
			{"ss_type","",NULL, EVM_RESTART_SHADOWSOCKS|EVM_RESTART_SS_TUNNEL},
			{"ss_mode","",NULL, EVM_RESTART_SHADOWSOCKS},
			{"ss_server","",NULL, EVM_RESTART_SHADOWSOCKS|EVM_RESTART_SS_TUNNEL},
			{"ss_server_port","",NULL, EVM_RESTART_SHADOWSOCKS|EVM_RESTART_SS_TUNNEL},
			{"ss_key","",NULL, EVM_RESTART_SHADOWSOCKS|EVM_RESTART_SS_TUNNEL},
			{"ss_method","",NULL, EVM_RESTART_SHADOWSOCKS|EVM_RESTART_SS_TUNNEL},
			{"ss_udp","",NULL, EVM_RESTART_SHADOWSOCKS},
			{"ss_local_port","",NULL, EVM_RESTART_SHADOWSOCKS},
			{"ss_mtu","",NULL, EVM_RESTART_SHADOWSOCKS},
			{"ss_router_proxy","",NULL, EVM_RESTART_SHADOWSOCKS},
			{"ss_lower_port_only","",NULL, EVM_RESTART_SHADOWSOCKS},
			{"ss_timeout","",NULL, EVM_RESTART_SHADOWSOCKS|EVM_RESTART_SS_TUNNEL},
			{"ss_protocol","",NULL, EVM_RESTART_SHADOWSOCKS|EVM_RESTART_SS_TUNNEL},
			{"ss_proto_param","",NULL, EVM_RESTART_SHADOWSOCKS|EVM_RESTART_SS_TUNNEL},
			{"ss_obfs","",NULL, EVM_RESTART_SHADOWSOCKS|EVM_RESTART_SS_TUNNEL},
			{"ss_obfs_param","",NULL, EVM_RESTART_SHADOWSOCKS|EVM_RESTART_SS_TUNNEL},
			{"ss_watchcat","",NULL, FALSE},
			{"ss_update_chnroute","",NULL, FALSE},
			{"ss_update_gfwlist","",NULL, FALSE},
			{"ss-tunnel_enable","",NULL, EVM_RESTART_SS_TUNNEL},
			{"ss-tunnel_local_port","",NULL, EVM_RESTART_SS_TUNNEL},
			{"ss-tunnel_remote","",NULL, EVM_RESTART_SS_TUNNEL},
			{"ss-tunnel_mtu","",NULL, EVM_RESTART_SS_TUNNEL},
			{0,0,0,0}
	};
#endif

	struct variable variables_WLANConfig11b[] = {
			{"rt_ssid", "", NULL, EVM_RESTART_WIFI2},
			{"rt_ssid2", "", NULL, EVM_RESTART_WIFI2},
			{"rt_mode_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_channel", "", NULL, EVM_RESTART_WIFI2},
			{"rt_bcn", "", NULL, EVM_RESTART_WIFI2},
			{"rt_dtim", "", NULL, EVM_RESTART_WIFI2},
			{"rt_gmode", "", NULL, EVM_RESTART_WIFI2},
			{"rt_gmode_protection", "", NULL, EVM_RESTART_WIFI2},
			{"rt_mcs_mode", "", NULL, EVM_RESTART_WIFI2},
			{"rt_rts", "", NULL, EVM_RESTART_WIFI2},
			{"rt_frag", "", NULL, EVM_RESTART_WIFI2},
			{"rt_ap_isolate", "", NULL, EVM_RESTART_WIFI2},
			{"rt_closed", "", NULL, EVM_RESTART_WIFI2},
			{"rt_IgmpSnEnable", "", NULL, EVM_RESTART_WIFI2},
			{"rt_mrate", "", NULL, EVM_RESTART_WIFI2},
			{"rt_wme", "", NULL, EVM_RESTART_WIFI2},
			{"rt_wme_no_ack", "", NULL, EVM_RESTART_WIFI2},
			{"rt_auth_mode", "", NULL, EVM_RESTART_WIFI2},
			{"rt_crypto", "", NULL, EVM_RESTART_WIFI2},
			{"rt_wpa_psk", "", NULL, EVM_RESTART_WIFI2},
			{"rt_wpa_gtk_rekey", "", NULL, EVM_RESTART_WIFI2},
			{"rt_key", "", NULL, EVM_RESTART_WIFI2},
			{"rt_key_type", "", NULL, EVM_RESTART_WIFI2},
			{"rt_key1", "", NULL, EVM_RESTART_WIFI2},
			{"rt_key2", "", NULL, EVM_RESTART_WIFI2},
			{"rt_key3", "", NULL, EVM_RESTART_WIFI2},
			{"rt_key4", "", NULL, EVM_RESTART_WIFI2},
			{"rt_wdsapply_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_wdsnum_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_wep_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_phrase_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_radio_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_radio_date_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_radio_time_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_radio_time2_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_wpa_mode", "", NULL, EVM_RESTART_WIFI2},
			{"rt_TxPower", "", NULL, EVM_RESTART_WIFI2},
			{"rt_TxBurst", "", NULL, EVM_RESTART_WIFI2},
			{"rt_PktAggregate", "", NULL, EVM_RESTART_WIFI2},
			{"rt_APSDCapable", "", NULL, EVM_RESTART_WIFI2},
			{"rt_HT_OpMode", "", NULL, EVM_RESTART_WIFI2},
			{"rt_HT_BW", "", NULL, EVM_RESTART_WIFI2},
			{"rt_HT_EXTCHA", "", NULL, EVM_RESTART_WIFI2},
			{"rt_HT_RDG", "", NULL, EVM_RESTART_WIFI2 },
			{"rt_HT_AMSDU", "", NULL, EVM_RESTART_WIFI2 },
			{"rt_HT_MpduDensity", "", NULL, EVM_RESTART_WIFI2 },
			{"rt_HT_BAWinSize", "", NULL, EVM_RESTART_WIFI2 },
			{"rt_HT_AutoBA", "", NULL, EVM_RESTART_WIFI2},
#if defined(USE_MT76X2_AP)
			{"rt_VgaClamp", "", NULL, EVM_RESTART_WIFI2},
#endif
#if defined (USE_WID_2G) && USE_WID_2G==7615
			{"rt_turbo_qam", "", NULL, EVM_RESTART_WIFI2},
			{"rt_airtimefairness", "", NULL, EVM_RESTART_WIFI2},
#endif
			{"rt_country_code", "", NULL, EVM_RESTART_WIFI2},
			{"rt_stream_tx", "", NULL, EVM_RESTART_WIFI2},
			{"rt_stream_rx", "", NULL, EVM_RESTART_WIFI2},
			{"rt_preamble", "", NULL, EVM_RESTART_WIFI2},
			{"rt_greenap", "", NULL, EVM_RESTART_WIFI2},
			{"rt_ldpc", "", NULL, EVM_RESTART_WIFI2},
			{"rt_sta_ssid", "", NULL, EVM_RESTART_WIFI2},
			{"rt_sta_auth_mode", "", NULL, EVM_RESTART_WIFI2},
			{"rt_sta_wpa_mode", "", NULL, EVM_RESTART_WIFI2},
			{"rt_sta_crypto", "", NULL, EVM_RESTART_WIFI2},
			{"rt_sta_wpa_psk", "", NULL, EVM_RESTART_WIFI2},
#if !defined(USE_RT3352_MII)
			{"rt_sta_wisp", "", NULL, EVM_RESTART_WIFI2},
			{"rt_sta_auto", "", NULL, EVM_RESTART_WIFI2},
#endif
			{"rt_guest_enable", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_date_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_time_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_time2_x", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_ssid", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_closed", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_ap_isolate", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_lan_isolate", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_auth_mode", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_wpa_mode", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_crypto", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_wpa_psk", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_macrule", "", NULL, EVM_RESTART_WIFI2},
			{"rt_guest_mcs_mode", "", NULL, EVM_RESTART_WIFI2},
			{"rt_KickStaRssiLow", "", NULL, EVM_RESTART_WIFI2},
			{"rt_AssocReqRssiThres", "", NULL, EVM_RESTART_WIFI2},
			{"rt_RBRList", "Group", ARGV((char*)variables_WLANConfig11b_rt_RBRList, "16", "32", "rt_wdsnum_x"), EVM_RESTART_WIFI2},
			{0,0,0,0}
		};

	struct svcLink svcLinks[] = {
		{"General",			variables_General},
		{"Storage",			variables_Storage},
		{"LANHostConfig",		variables_LANHostConfig},
		{"Layer3Forwarding",		variables_Layer3Forwarding},
		{"WANCommonInterface",		variables_WANCommonInterface},
		{"IPConnection",		variables_IPConnection},
		{"IP6Connection",		variables_IP6Connection},
		{"PPPConnection",		variables_PPPConnection},
		{"EthernetLink",		variables_EthernetLink},
		{"FirewallConfig",		variables_FirewallConfig},
		{"RouterConfig",		variables_RouterConfig},
		{"WLANConfig11a",		variables_WLANConfig11a},
		{"WLANConfig11b",		variables_WLANConfig11b},
		{"DeviceSecurity11a",		variables_DeviceSecurity11a},
		{"DeviceSecurity11b",		variables_DeviceSecurity11b},
		{"WLANAuthentication11a",	variables_WLANAuthentication11a},
		{"WLANAuthentication11b",	variables_WLANAuthentication11b},
#if defined(APP_DNSFORWARDER)
		{"dnsforwarderConf",		variables_dnsforwarderConf},
#endif
#if defined(APP_SCUT)
		{"ScutclientConf",		variables_ScutclientConf},
#endif
#if defined(APP_MENTOHUST)
		{"mentohustConf",		variables_mentohustConf},
#endif
#if defined(APP_SHADOWSOCKS)
		{"ShadowsocksConf",		variables_ShadowsocksConf},
#endif
		{"LANGUAGE",			variables_Language},
		{0,0}
	};

	struct evDesc events_desc[] = {
		{EVM_RESTART_IPV6,		EVT_RESTART_IPV6,		RCN_RESTART_IPV6,	EVM_RESTART_DHCPD|EVM_RESTART_RADV|EVM_RESTART_IPTV|EVM_RESTART_DI|EVM_RESTART_UPNP|EVM_RESTART_FIREWALL|EVM_RESTART_VPNCLI|EVM_RESTART_SWITCH_VLAN|EVM_RESTART_LAN|EVM_RESTART_WAN|EVM_RESTART_MODEM},
		{EVM_RESTART_LAN,		EVT_RESTART_LAN,		RCN_RESTART_LAN,	EVM_RESTART_DHCPD|EVM_RESTART_RADV|EVM_RESTART_IPTV|EVM_RESTART_DI|EVM_RESTART_UPNP|EVM_RESTART_FIREWALL|EVM_RESTART_VPNSVR},
#if defined(USE_USB_SUPPORT)
		{EVM_RESTART_MODEM,		EVT_RESTART_MODEM,		RCN_RESTART_MODEM,	EVM_RESTART_IPTV|EVM_RESTART_SWITCH_VLAN|EVM_RESTART_FIREWALL|EVM_RESTART_VPNCLI|EVM_RESTART_WAN},
#endif
		{EVM_RESTART_WAN,		EVT_RESTART_WAN,		RCN_RESTART_WAN,	EVM_RESTART_IPTV|EVM_RESTART_SWITCH_VLAN|EVM_RESTART_FIREWALL|EVM_RESTART_VPNCLI|EVM_RESTART_NETFILTER},
		{EVM_RESTART_NETFILTER,		EVT_RESTART_NETFILTER,		RCN_RESTART_NETFILTER,	EVM_RESTART_FIREWALL},
		{EVM_RESTART_RADV,		EVT_RESTART_RADV,		RCN_RESTART_RADV,	EVM_RESTART_DHCPD},
		{EVM_RESTART_IPTV,		EVT_RESTART_IPTV,		RCN_RESTART_IPTV,	EVM_RESTART_FIREWALL},
		{EVM_RESTART_VPNSVR,		EVT_RESTART_VPNSVR,		RCN_RESTART_VPNSVR,	EVM_RESTART_FIREWALL|EVM_REAPPLY_VPNSVR},
		{EVM_RESTART_VPNCLI,		EVT_RESTART_VPNCLI,		RCN_RESTART_VPNCLI,	EVM_RESTART_FIREWALL},
		{EVM_RESTART_HTTPD,		EVT_RESTART_HTTPD,		RCN_RESTART_HTTPD,	EVM_RESTART_FIREWALL},
		{EVM_RESTART_SSHD,		EVT_RESTART_SSHD,		RCN_RESTART_SSHD,	EVM_RESTART_FIREWALL},
		{EVM_RESTART_TELNETD,		EVT_RESTART_TELNETD,		RCN_RESTART_TELNETD,	0},

#if defined(APP_SMBD) || defined(APP_NMBD)
		{EVM_RESTART_WINS,		EVT_RESTART_WINS,		RCN_RESTART_WINS,	EVM_RESTART_DHCPD|EVM_RESTART_NMBD|EVM_REAPPLY_VPNSVR},
#endif
		{EVM_REAPPLY_VPNSVR,		EVT_REAPPLY_VPNSVR,		RCN_REAPPLY_VPNSVR,	0},
		{EVM_RESTART_DHCPD,		EVT_RESTART_DHCPD,		RCN_RESTART_DHCPD,	0},
		{EVM_RESTART_SWITCH_CFG,	EVT_RESTART_SWITCH_CFG,		RCN_RESTART_SWITCH_CFG,	0},
		{EVM_RESTART_SWITCH_VLAN,	EVT_RESTART_SWITCH_VLAN,	RCN_RESTART_SWITCH_VLAN,0},
		{EVM_RESTART_DDNS,		EVT_RESTART_DDNS,		RCN_RESTART_DDNS,	0},
		{EVM_RESTART_UPNP,		EVT_RESTART_UPNP,		RCN_RESTART_UPNP,	EVM_RESTART_FIREWALL},
		{EVM_RESTART_TIME,		EVT_RESTART_TIME,		RCN_RESTART_TIME,	EVM_RESTART_CROND},
		{EVM_RESTART_NTPC,		EVT_RESTART_NTPC,		RCN_RESTART_NTPC,	0},
		{EVM_RESTART_SYSLOG,		EVT_RESTART_SYSLOG,		RCN_RESTART_SYSLOG,	0},
		{EVM_RESTART_LLTD,		EVT_RESTART_LLTD,		RCN_RESTART_LLTD,	0},
		{EVM_RESTART_ADSC,		EVT_RESTART_ADSC,		RCN_RESTART_ADSC,	0},
		{EVM_RESTART_CROND,		EVT_RESTART_CROND,		RCN_RESTART_CROND,	0},
		{EVM_RESTART_SYSCTL,		EVT_RESTART_SYSCTL,		RCN_RESTART_SYSCTL,	0},
		{EVM_RESTART_TWEAKS,		EVT_RESTART_TWEAKS,		RCN_RESTART_TWEAKS,	0},
		{EVM_RESTART_WDG,		EVT_RESTART_WDG,		RCN_RESTART_WDG,	0},
		{EVM_RESTART_DI,		EVT_RESTART_DI,			RCN_RESTART_DI,		0},
#if defined(USE_USB_SUPPORT)
		{EVM_RESTART_SPOOLER,		EVT_RESTART_SPOOLER,		RCN_RESTART_SPOOLER,	0},
		{EVM_RESTART_HDDTUNE,		EVT_RESTART_HDDTUNE,		RCN_RESTART_HDDTUNE,	0},
#if defined(APP_FTPD)
		{EVM_RESTART_FTPD,		EVT_RESTART_FTPD,		RCN_RESTART_FTPD,	EVM_RESTART_FIREWALL},
#endif
#if defined(APP_SMBD)
		{EVM_RESTART_SMBD,		EVT_RESTART_SMBD,		RCN_RESTART_SMBD,	EVM_RESTART_NMBD},
#endif
#if defined(APP_NFSD)
		{EVM_RESTART_NFSD,		EVT_RESTART_NFSD,		RCN_RESTART_NFSD,	0},
#endif
#if defined(APP_MINIDLNA)
		{EVM_RESTART_DMS,		EVT_RESTART_DMS,		RCN_RESTART_DMS,	0},
#endif
#if defined(APP_FIREFLY)
		{EVM_RESTART_ITUNES,		EVT_RESTART_ITUNES,		RCN_RESTART_ITUNES,	0},
#endif
#if defined(APP_TRMD)
		{EVM_RESTART_TRMD,		EVT_RESTART_TRMD,		RCN_RESTART_TRMD,	EVM_RESTART_FIREWALL},
#endif
#if defined(APP_ARIA)
		{EVM_RESTART_ARIA,		EVT_RESTART_ARIA,		RCN_RESTART_ARIA,	EVM_RESTART_FIREWALL},
#endif
#endif
#if defined(APP_SCUT)
		{EVM_RESTART_SCUT,		EVT_RESTART_SCUT,		RCN_RESTART_SCUT,	0},
#endif
#if defined(APP_MENTOHUST)
		{EVM_RESTART_MENTOHUST,		EVT_RESTART_MENTOHUST,		RCN_RESTART_MENTOHUST,	0},
#endif
#if defined(APP_TTYD)
		{EVM_RESTART_TTYD,		EVT_RESTART_TTYD,		RCN_RESTART_TTYD,	0},
#endif
#if defined(APP_VLMCSD)
		{EVM_RESTART_VLMCSD,	EVT_RESTART_VLMCSD,		RCN_RESTART_VLMCSD,	0},
#endif
#if defined(APP_DNSFORWARDER)
		{EVM_RESTART_DNSFORWARDER,	EVT_RESTART_DNSFORWARDER,	RCN_RESTART_DNSFORWARDER, 0},
#endif
#if defined(APP_SHADOWSOCKS)
		{EVM_RESTART_SHADOWSOCKS,	EVT_RESTART_SHADOWSOCKS,	RCN_RESTART_SHADOWSOCKS,  0},
		{EVM_RESTART_SS_TUNNEL,		EVT_RESTART_SS_TUNNEL,		RCN_RESTART_SS_TUNNEL,	  0},
#endif
#if defined(APP_SMBD) || defined(APP_NMBD)
		{EVM_RESTART_NMBD,		EVT_RESTART_NMBD,		RCN_RESTART_NMBD,	0},
#endif
		{EVM_RESTART_FIREWALL,		EVT_RESTART_FIREWALL,		RCN_RESTART_FIREWALL,	0},
		{0,0,0,0}
	};

