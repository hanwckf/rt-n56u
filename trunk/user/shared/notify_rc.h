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

#ifndef NOTIFY_RC_H
#define NOTIFY_RC_H

#define RCN_RESTART_FIREWALL		"restart_firewall"
#define RCN_RESTART_DHCPD		"restart_dhcpd"
#define RCN_RESTART_RADV		"restart_radv"
#define RCN_RESTART_DDNS		"restart_ddns"
#define RCN_RESTART_UPNP		"restart_upnp"
#define RCN_RESTART_TIME		"restart_time"
#define RCN_RESTART_NTPC		"restart_ntpc"
#define RCN_RESTART_SYSLOG		"restart_syslog"
#define RCN_RESTART_NETFILTER		"restart_netfilter"
#define RCN_REAPPLY_VPNSVR		"reapply_vpn_server"
#define RCN_RESTART_VPNSVR		"restart_vpn_server"
#define RCN_RESTART_VPNCLI		"restart_vpn_client"
#define RCN_RESTART_WIFI2		"restart_wifi_rt"
#define RCN_RESTART_WIFI5		"restart_wifi_wl"
#define RCN_RESTART_SWITCH_CFG		"restart_switch_config"
#define RCN_RESTART_SWITCH_VLAN		"restart_switch_vlan"
#define RCN_RESTART_LAN			"restart_whole_lan"
#define RCN_RESTART_WAN			"restart_whole_wan"
#define RCN_RESTART_IPV6		"restart_ipv6"
#define RCN_RESTART_HTTPD		"restart_httpd"
#define RCN_RESTART_TELNETD		"restart_telnetd"
#define RCN_RESTART_SSHD		"restart_sshd"
#define RCN_RESTART_WINS		"restart_wins"
#define RCN_RESTART_LLTD		"restart_lltd"
#define RCN_RESTART_ADSC		"restart_adsc"
#define RCN_RESTART_CROND		"restart_crond"
#define RCN_RESTART_IPTV		"restart_iptv"
#define RCN_RESTART_SYSCTL		"restart_sysctl"
#define RCN_RESTART_TWEAKS		"restart_tweaks"
#define RCN_RESTART_WDG			"restart_wdg"
#define RCN_RESTART_DI			"restart_di"
#define RCN_RESTART_SPOOLER		"restart_spooler"
#define RCN_RESTART_MODEM		"restart_modem"
#define RCN_RESTART_HDDTUNE		"restart_hddtune"
#define RCN_RESTART_FTPD		"restart_ftpd"
#define RCN_RESTART_NMBD		"restart_nmbd"
#define RCN_RESTART_SMBD		"restart_smbd"
#define RCN_RESTART_NFSD		"restart_nfsd"
#define RCN_RESTART_DMS			"restart_dms"
#define RCN_RESTART_ITUNES		"restart_itunes"
#define RCN_RESTART_TRMD		"restart_trmd"
#define RCN_RESTART_ARIA		"restart_aria"
#define RCN_RESTART_SCUT		"restart_scutclient"
#define RCN_RESTART_MENTOHUST	"restart_mentohust"
#define RCN_RESTART_TTYD		"restart_ttyd"
#define RCN_RESTART_VLMCSD		"restart_vlmcsd"
#define RCN_RESTART_SHADOWSOCKS	"restart_shadowsocks"
#define RCN_RESTART_CHNROUTE_UPD	"restart_chnroute_upd"
#define RCN_RESTART_DNSFORWARDER	"restart_dns_forwarder"
#define RCN_RESTART_SS_TUNNEL		"restart_ss_tunnel"
#define RCN_RESTART_GFWLIST_UPD		"restart_gfwlist_upd"
#define RCN_RESTART_REBOOT		"restart_reboot"

////////////////////////////////////////////////////////////

#define DIR_RC_NOTIFY			"/tmp/rc_notification"
#define DIR_RC_INCOMPLETE		"/tmp/rc_action_incomplete"

extern void notify_rc(const char *event_name);
extern void notify_rc_and_wait(const char *event_name, int wait_sec);


#endif /* NOTIFY_RC_H */
