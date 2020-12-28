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

#ifndef _rc_h_
#define _rc_h_

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/oom.h>

#include <ralink_boards.h>

#include <nvram_linux.h>

#include <rtutils.h>
#include <netutils.h>
#include <shutils.h>
#include <notify_rc.h>
#include <bin_sem_asus.h>

/* do not set current year, it used for ntp done check! */
#define SYS_START_YEAR			2015

#define DNS_RESOLV_CONF			"/etc/resolv.conf"
#define DNS_SERVERS_FILE		"/tmp/dnsmasq.servers"
#define DNS_RELAY_QUERIES_MAX		512

#define SCRIPT_UDHCPC_LAN		"/tmp/udhcpc_lan.script"
#define SCRIPT_UDHCPC_WAN		"/tmp/udhcpc.script"
#define SCRIPT_UDHCPC_VIPTV		"/tmp/udhcpc_viptv.script"
#define SCRIPT_ZCIP_WAN			"/tmp/zcip.script"
#define SCRIPT_ZCIP_VIPTV		"/tmp/zcip_viptv.script"
#define SCRIPT_WPACLI_WAN		"/tmp/wpacli.script"
#define SCRIPT_DHCP6C_WAN		"/tmp/dhcp6c.script"

#define SCRIPT_POST_WAN			"/etc/storage/post_wan_script.sh"
#define SCRIPT_POST_FIREWALL		"/etc/storage/post_iptables_script.sh"
#define SCRIPT_INTERNET_STATE		"/etc/storage/inet_state_script.sh"
#define SCRIPT_SHUTDOWN			"/etc/storage/shutdown_script.sh"

#define SCRIPT_OVPN_SERVER		"ovpns.script"
#define SCRIPT_OVPN_CLIENT		"ovpnc.script"

#define VPN_SERVER_LEASE_FILE		"/tmp/vpns.leases"
#define VPN_SERVER_SUBNET_MASK		"255.255.255.0"
#define VPN_SERVER_PPP_UNIT		10
#define VPN_SERVER_PPPD_OPTIONS		"/tmp/ppp/options.vpns"
#define VPN_SERVER_UPDOWN_SCRIPT	"/etc/storage/vpns_client_script.sh"

#define VPN_CLIENT_PPPD_OPTIONS		"/tmp/ppp/options.vpnc"
#define VPN_CLIENT_UPDOWN_SCRIPT	"/etc/storage/vpnc_server_script.sh"

#define IPT_CHAIN_NAME_VPN_LIST		"vpnlist"
#define IPT_CHAIN_NAME_MAC_LIST		"maclist"
#define IPT_CHAIN_NAME_URL_LIST		"urllist"
#define IPT_CHAIN_NAME_LWF_LIST		"lwflist"
#define IPT_CHAIN_NAME_BFP_LIMIT	"bfplimit"
#define IPT_CHAIN_NAME_DOS_LIMIT	"doslimit"
#define IPT_CHAIN_NAME_LOG_ACCEPT	"logaccept"
#define IPT_CHAIN_NAME_LOG_DROP		"logdrop"
#define IPT_CHAIN_NAME_VSERVER		"vserver"

#define MODEM_NODE_DIR			"/tmp/modem"
#define PPP_PEERS_DIR			"/tmp/ppp/peers"

#define QMI_CLIENT_ID			"/tmp/qmi-client-id"
#define QMI_HANDLE_PDH			"/tmp/qmi-handle"

#define DDNS_CONF_FILE			"/etc/inadyn.conf"
#define DDNS_DONE_SCRIPT		"/sbin/ddns_updated"
#define DDNS_CACHE_DIR			"/tmp/inadyn"

#define NTPC_DONE_SCRIPT		"/sbin/ntpc_updated"

#define SAMBA_CONF			"/etc/smb.conf"

#define MP_MTD_RWFS			"/media/mtd_rwfs"

#define SR_PREFIX_LAN			"LAN"
#define SR_PREFIX_MAN			"MAN"
#define SR_PREFIX_WAN			"WAN"

#define MAX_CLIENTS_NUM			(50)

#define MINIUPNPD_CHAIN_IP4_NAT		"upnp"
#define MINIUPNPD_CHAIN_IP4_NAT_POST	"upnp-post"
#define MINIUPNPD_CHAIN_IP4_FORWARD	"upnp"
#define MINIUPNPD_CHAIN_IP6_FORWARD	"upnp"

// for log message title
#define LOGNAME				BOARD_NAME

#if BOARD_RAM_SIZE > 128
 #define KERNEL_NET_CORE_RMEM		1310720
 #define KERNEL_NET_CORE_WMEM		1310720
 #define KERNEL_MIN_FREE_KBYTES		12288
 #define DNS_RELAY_CACHE_MAX		1536
 #define LOG_ROTATE_SIZE_MAX		1024
#elif BOARD_RAM_SIZE > 64
 #define KERNEL_NET_CORE_RMEM		983040
 #define KERNEL_NET_CORE_WMEM		983040
 #define KERNEL_MIN_FREE_KBYTES		8192
 #define DNS_RELAY_CACHE_MAX		1024
 #define LOG_ROTATE_SIZE_MAX		512
#elif BOARD_RAM_SIZE > 32
 #define KERNEL_NET_CORE_RMEM		655360
 #define KERNEL_NET_CORE_WMEM		655360
 #define KERNEL_MIN_FREE_KBYTES		4096
 #define DNS_RELAY_CACHE_MAX		512
 #define LOG_ROTATE_SIZE_MAX		256
#elif BOARD_RAM_SIZE > 16
 #define KERNEL_NET_CORE_RMEM		327680
 #define KERNEL_NET_CORE_WMEM		327680
 #define KERNEL_MIN_FREE_KBYTES		2048
 #define DNS_RELAY_CACHE_MAX		256
 #define LOG_ROTATE_SIZE_MAX		128
#else
 #define KERNEL_NET_CORE_RMEM		163840
 #define KERNEL_NET_CORE_WMEM		163840
 #define KERNEL_MIN_FREE_KBYTES		1024
 #define DNS_RELAY_CACHE_MAX		160
 #define LOG_ROTATE_SIZE_MAX		80
#endif

//////////////////////////////////////////////////////////

struct disk_info_t;

/* rc.c */
void setenv_tz(void);
void setkernel_tz(void);
void init_router(void);
void shutdown_router(int level);
void handle_notifications(void);
void LED_CONTROL(int gpio_led, int flag);
void storage_save_time(time_t delta);
void write_storage_to_mtd(void);
void erase_storage(void);
void erase_nvram(void);

/* init.c */
void init_main_loop(void);
int  sys_exit(void);
int  sys_stop(void);

/* auth.c */
int  wpacli_main(int argc, char **argv);
int  start_auth_eapol(char *ifname, int unit, int eap_algo);
void stop_auth_eapol(void);
int  start_auth_kabinet(void);
void stop_auth_kabinet(void);

/* common_ex.c */
long uptime(void);
int rand_seed_by_time(void);
void set_pagecache_reclaim(void);
void restart_all_sysctl(void);
void update_router_mode();
char *mac_conv(const char *mac_nvkey, int idx, char *buf);
char *mac_conv2(const char *mac_nvkey, int idx, char *buf);
void get_eeprom_params(void);
void char_to_ascii(char *output, char *input);
unsigned int get_param_int_hex(const char *param);
void load_user_config(FILE *fp, const char *dir_name, const char *file_name, const char **forbid_list);
int is_module_loaded(const char *module_name);
int get_module_refcount(const char *module_name);
int module_smart_load(const char *module_name, const char *module_param);
int module_smart_unload(const char *module_name, int recurse_unload);
int module_param_get(const char *module_name, const char *module_param, char *param_value, size_t param_value_size);
int module_param_set_int(const char *module_name, const char *module_param, int param_value);
void oom_score_adjust(pid_t pid, int oom_score_adj);
void mount_rwfs_partition(void);
void umount_rwfs_partition(void);
void start_rwfs_optware(void);
void kill_services(char* svc_name[], int wtimeout, int forcekill);
int kill_process_pidfile(char *pidfile, int wtimeout, int forcekill);
int create_file(const char *fn);
int mkdir_if_none(const char *dirpath, const char *mode);
int check_if_file_exist(const char *filepath);
int check_if_dir_exist(const char *dirpath);
int check_if_dev_exist(const char *devpath);
int get_hotplug_action(const char *action);

/* net.c */
int  route_add(char *ifname, int metric, char *dst, char *gateway, char *genmask);
int  route_del(char *ifname, int metric, char *dst, char *gateway, char *genmask);
int  control_static_routes(char *ift, char *ifname, int is_add);
char* sanity_hostname(char *hname);
char* get_our_hostname(void);
int  is_same_subnet(const char *ip1, const char *ip2, const char *msk);
int  is_same_subnet2(const char *ip1, const char *ip2, const char *msk1, const char *msk2);
void create_vlan_iface(const char *dev_ifname, int vid, int prio, int mtu, const char *hwaddr, int do_up);
void remove_vlan_iface(const char *vlan_ifname);
#if defined (APP_XUPNPD)
void stop_xupnpd(void);
void start_xupnpd(char *wan_ifname);
#endif
void stop_udpxy(void);
void start_udpxy(char *wan_ifname);
void stop_igmpproxy(char *wan_ifname);
void start_igmpproxy(char *wan_ifname);
void restart_iptv(int is_ap_mode);
void flush_conntrack_table(char *ip);
void flush_route_caches(void);
void clear_if_route4(char *ifname);
int  is_ftp_conntrack_loaded(int ftp_port0, int ftp_port1);
void hw_vlan_tx_map(int idx, int vid);
void reload_nat_modules(void);
void restart_firewall(void);
void set_ipv4_forward(int is_on);
void set_nf_conntrack(void);
void set_tcp_syncookies(void);
void set_tcp_tweaks(void);
void set_passthrough_pppoe(int is_on);
void set_igmp_mld_version(void);
void set_libc_gai(int ipv4_first);

/* net_lan.c */
in_addr_t get_lan_ip4(void);
int  has_lan_ip4(void);
int  has_lan_gw4(void);
int  add_static_lan_routes(char *lan_ifname);
int  del_static_lan_routes(char *lan_ifname);
void reset_lan_temp(void);
void reset_lan_vars(void);
void update_hosts_ap(void);
void start_lan(int is_ap_mode, int do_wait);
void stop_lan(int is_ap_mode);
void lan_up_manual(char *lan_ifname, char *lan_dname);
void update_lan_status(int is_auto);
void full_restart_lan(void);
void init_loopback(void);
void init_bridge(int is_ap_mode);
void config_bridge(int is_ap_mode);
void switch_config_base(void);
void switch_config_storm(void);
void switch_config_link(void);
void switch_config_vlan(int first_call);
void restart_switch_config_vlan(void);
void update_ether_leds(void);
int  is_vlan_vid_valid(int vlan_vid);
int  start_udhcpc_lan(char *lan_ifname);
int  stop_udhcpc_lan();
int  udhcpc_lan_main(int argc, char **argv);

/* net_wan.c */
char* get_wan_unit_value(int unit, const char* param_name);
int  get_wan_unit_value_int(int unit, const char* param_name);
void set_wan_unit_value(int unit, const char* param_name, const char* value);
void set_wan_unit_value_int(int unit, const char* param_name, int value);
void set_wan_unit_param(int unit, const char* param_name);
void reset_wan_temp(void);
void reset_man_vars(void);
void reset_wan_vars(void);
int  get_vlan_vid_wan(void);
int  get_wan_bridge_mode(void);
int  get_wan_bridge_iso_mode(int bridge_mode);
void start_wan(void);
void stop_wan(void);
void man_up(char *man_ifname, int unit, int is_static);
void man_down(char *man_ifname, int unit);
void wan_up(char *wan_ifname, int unit, int is_static);
void wan_down(char *wan_ifname, int unit, int is_static);
void select_usb_modem_to_wan(void);
void full_restart_wan(void);
void try_wan_reconnect(int try_use_modem, long pause_in_seconds);
void auto_wan_reconnect(void);
void auto_wan_reconnect_pause(void);
void manual_wan_reconnect(void);
void manual_wan_disconnect(void);
void manual_wisp_reassoc(void);
void deferred_wan_connect(void);
void notify_on_wan_ether_link_restored(void);
void notify_on_internet_state_changed(int has_internet, long elapsed);
void add_dhcp_routes(char *rt, char *rt_rfc, char *rt_ms, char *ifname, int metric);
void add_dhcp_routes_by_prefix(char *prefix, char *ifname, int metric);
int  add_static_wan_routes(char *wan_ifname);
int  del_static_wan_routes(char *wan_ifname);
int  add_static_man_routes(char *man_ifname);
int  del_static_man_routes(char *man_ifname);
int  update_resolvconf(int is_first_run, int do_not_notify);
int  update_hosts_router(const char *lan_ipaddr);
int  get_wan_ether_link_direct(int is_ap_mode);
int  get_wan_dns_static(void);
int  get_wan_wisp_active(int *p_has_link);
void get_wan_ifname(char wan_ifname[16]);
void update_wan_status(int unit);
in_addr_t get_wan_ip4(int only_broadband_wan);
int  has_wan_ip4(int only_broadband_wan);
int  has_wan_gw4(void);
int  has_wan_phy_link(void);
int udhcpc_main(int argc, char **argv);
int udhcpc_viptv_main(int argc, char **argv);
int zcip_main(int argc, char **argv);
int zcip_viptv_main(int argc, char **argv);
int start_udhcpc_wan(char *wan_ifname, int unit, int wait_lease);
int renew_udhcpc_wan(int unit);
int release_udhcpc_wan(int unit);
int stop_udhcpc_wan(int unit);
int start_udhcpc_viptv(char *man_ifname);
int stop_udhcpc_viptv(void);
int start_zcip_wan(char *wan_ifname);
int start_zcip_viptv(char *man_ifname);

/* net_ppp.c */
int  safe_start_xl2tpd(void);
char *safe_pppd_line(const char *line, char *buf, size_t buf_size);
int  launch_wan_pppd(int unit, int wan_proto);
void preset_wan_ppp_routes(char *ppp_ifname, int unit);
int  ppp_ifindex(char *ifname);
int  ipup_main(int argc, char **argv);
int  ipdown_main(int argc, char **argv);

#if defined (USE_IPV6)
/* net6.c */
void init_ipv6(void);
void control_if_ipv6_all(int enable);
void control_if_ipv6(const char *ifname, int enable);
void control_if_ipv6_autoconf(const char *ifname, int enable);
void control_if_ipv6_radv(const char *ifname, int enable);
void control_if_ipv6_dad(const char *ifname, int enable);
void control_if_ipv6_privacy(const char *ifname, int enable);
void full_restart_ipv6(int ipv6_type_old);
void clear_if_addr6(char *ifname);
void clear_if_route6(char *ifname);
void clear_if_neigh6(char *ifname);
void clear_all_addr6(void);
void clear_all_route6(void);
int ipv6_from_string(const char *str, struct in6_addr *addr6);
int ipv6_to_net(struct in6_addr *addr6, int prefix);
int ipv6_to_host(struct in6_addr *addr6, int prefix);
int ipv6_to_ipv4_map(struct in6_addr *addr6, int size6, struct in_addr *addr4, int size4);
int ipv6_compact(const char *str6, char *p_comp6, int allow_prefix);

/* net_lan6.c */
int is_lan_radv_on(void);
int is_lan_addr6_static(void);
int get_lan_dhcp6s_mode(void);
int get_lan_dhcp6s_irt(void);
int get_lan_dhcp6s_prefix_size(void);
int store_lan_addr6(char *lan_addr6_new);
void reload_lan_addr6(void);
void clear_lan_addr6(void);
void reset_lan6_vars(void);
char *get_lan_addr6_host(char *p_addr6s);
char *get_lan_addr6_prefix(char *p_addr6s);

/* net_wan6.c */
int is_wan_dns6_static(void);
int is_wan_addr6_static(void);
int is_wan_ipv6_type_sit(void);
int is_wan_ipv6_if_ppp(void);
int  store_wan_dns6(char *dns6_new);
void reset_wan6_vars(void);
void store_ip6rd_from_dhcp(const char *env_value, const char *prefix);
char *get_wan_addr6_host(char *p_addr6s);
void start_sit_tunnel(int ipv6_type, char *wan_ifname, char *wan_addr4, char *wan_gate4, char *wan_addr6);
void stop_sit_tunnel(void);
void wan6_up(char *wan_ifname, int unit);
void wan6_down(char *wan_ifname, int unit);
int dhcp6c_main(int argc, char **argv);
int start_dhcp6c(char *wan_ifname);
void stop_dhcp6c(void);

/* net_ppp.c */
int ipv6up_main(int argc, char **argv);
int ipv6down_main(int argc, char **argv);

/* firewall_ex.c */
void ip6t_filter_default(void);
#endif

/* vpn_server.c */
int start_vpn_server(void);
void stop_vpn_server(void);
void restart_vpn_server(void);
void reapply_vpn_server(void);
void get_vpns_pool(int i_vuse, unsigned int *ip_v, unsigned int *ip_m, unsigned int *vp_b, unsigned int *vp_e);
void vpns_route_to_remote_lan(const char *cname, char *ifname, char *gw, int add);
int ipup_vpns_main(int argc, char **argv);
int ipdown_vpns_main(int argc, char **argv);

/* vpn_client.c */
int start_vpn_client(void);
void stop_vpn_client(void);
void restart_vpn_client(void);
void restore_dns_from_vpnc(void);
int ipup_vpnc_main(int argc, char **argv);
int ipdown_vpnc_main(int argc, char **argv);

#if defined (APP_OPENVPN)
/* openvpn.c */
int start_openvpn_server(void);
int start_openvpn_client(void);
void stop_openvpn_server(void);
void stop_openvpn_client(void);
void restart_openvpn_server(void);
int ovpn_server_script_main(int argc, char **argv);
int ovpn_client_script_main(int argc, char **argv);
int ovpn_server_expcli_main(int argc, char **argv);
#endif

/* net_wifi.c */
void mlme_state_wl(int is_on);
void mlme_state_rt(int is_on);
void mlme_radio_wl(int is_on);
void mlme_radio_rt(int is_on);
int  get_mlme_radio_wl(void);
int  get_mlme_radio_rt(void);
int  get_enabled_radio_wl(void);
int  get_enabled_radio_rt(void);
int  get_enabled_guest_wl(void);
int  get_enabled_guest_rt(void);
int  get_mode_radio_wl(void);
int  get_mode_radio_rt(void);
int  is_apcli_wisp_wl(void);
int  is_apcli_wisp_rt(void);
int  get_apcli_sta_auto(int is_aband);
char* get_apcli_wisp_ifname(void);
#if defined (USE_RT3352_MII)
void check_inic_mii_rebooted(void);
#endif
void update_vga_clamp_wl(int first_call);
void update_vga_clamp_rt(int first_call);
void start_wifi_ap_wl(int radio_on);
void start_wifi_ap_rt(int radio_on);
void start_wifi_wds_wl(int radio_on);
void start_wifi_wds_rt(int radio_on);
void start_wifi_apcli_wl(int radio_on);
void start_wifi_apcli_rt(int radio_on);
void reconnect_apcli(const char *ifname_apcli, int force);
int  is_need_8021x(char *auth_mode);
void start_8021x_wl(void);
void start_8021x_rt(void);
void stop_8021x_wl(void);
void stop_8021x_rt(void);
void stop_8021x_all(void);
int  is_radio_on_wl(void);
int  is_radio_on_rt(void);
int  is_radio_allowed_wl(void);
int  is_radio_allowed_rt(void);
int  is_guest_allowed_wl(void);
int  is_guest_allowed_rt(void);
int  control_radio_wl(int radio_on, int manual);
int  control_radio_rt(int radio_on, int manual);
int  control_guest_wl(int guest_on, int manual);
int  control_guest_rt(int guest_on, int manual);
void restart_wifi_wl(int radio_on, int need_reload_conf);
void restart_wifi_rt(int radio_on, int need_reload_conf);
void stop_wifi_all_wl(void);
void stop_wifi_all_rt(void);
void restart_guest_lan_isolation(void);
int  manual_toggle_radio_rt(int radio_on);
int  manual_toggle_radio_wl(int radio_on);
int  manual_change_radio_rt(int radio_on);
int  manual_change_radio_wl(int radio_on);
int  manual_change_guest_rt(int radio_on);
int  manual_change_guest_wl(int radio_on);
int  timecheck_wifi(int is_aband, const char *nv_date, const char *nv_time1, const char *nv_time2);

/* services.c */
void stop_syslogd();
void stop_klogd();
int start_syslogd();
int start_klogd();
int start_infosvr(void);
void stop_infosvr(void);
void restart_infosvr(void);
int start_networkmap(int first_call);
void stop_networkmap(void);
void restart_networkmap(void);
void stop_telnetd(void);
void run_telnetd(void);
void start_telnetd(void);
#if defined (APP_SSHD)
int is_sshd_run(void);
void stop_sshd(void);
void start_sshd(void);
void restart_sshd(void);
#endif
void start_httpd(int restart_fw);
void stop_httpd(void);
void restart_httpd(void);
int start_lltd(void);
void stop_lltd(void);
void restart_lltd(void);
int start_crond(void);
void stop_crond(void);
void restart_crond(void);
void stop_rstats(void);
int start_rstats(void);
void restart_rstats(void);
int start_logger(int showinfo);
void stop_logger(void);
void start_watchdog_cpu(void);
void restart_watchdog_cpu(void);
int start_services_once(int is_ap_mode);
void stop_services(int stopall);
void stop_services_lan_wan(void);
void stop_misc(void);
#if defined(APP_SCUT)
int is_scutclient_run(void);
void stop_scutclient(void);
void start_scutclient(void);
void restart_scutclient(void);
#endif
#if defined(APP_MENTOHUST)
int is_mentohust_run(void);
void stop_mentohust(void);
void start_mentohust(void);
void restart_mentohust(void);
#endif
#if defined(APP_TTYD)
void stop_ttyd(void);
void start_ttyd(void);
void restart_ttyd(void);
#endif
#if defined(APP_SHADOWSOCKS)
void stop_ss(void);
void start_ss(void);
void restart_ss(void);
void stop_ss_tunnel(void);
void start_ss_tunnel(void);
void restart_ss_tunnel(void);
void update_chnroute(void);
void update_gfwlist(void);
#endif
#if defined(APP_VLMCSD)
void stop_vlmcsd(void);
void start_vlmcsd(void);
void restart_vlmcsd(void);
#endif
#if defined(APP_NAPT66)
void start_napt66(void);
#endif
#if defined(APP_DNSFORWARDER)
void stop_dnsforwarder(void);
void start_dnsforwarder(void);
void restart_dnsforwarder(void);
#endif

/* services_ex.c */
int fill_dnsmasq_servers(void);
int is_dns_dhcpd_run(void);
int is_dhcpd_enabled(int is_ap_mode);
int start_dns_dhcpd(int is_ap_mode);
void stop_dns_dhcpd(void);
#if defined (APP_SMBD) || defined (APP_NMBD)
FILE *write_smb_conf_header(void);
void stop_nmbd(void);
void start_wins(void);
void reload_nmbd(void);
void restart_nmbd(void);
#endif
int is_upnp_run(void);
int start_upnp(void);
void stop_upnp(void);
void check_upnp_wanif_changed(char *wan_ifname);
void update_upnp(void);
void restart_upnp(void);
int ddns_updated_main(int argc, char *argv[]);
int notify_ddns_update(void);
int start_ddns(int clear_cache);
void stop_ddns(void);
char *get_ddns_fqdn(void);
void manual_ddns_hostname_check(void);
int restart_dhcpd(void);
int restart_dns(void);

#if defined (USE_STORAGE)
/* services_stor.c */
void spindown_hdd(char *sd_dev);
#if defined (APP_FTPD)
int is_ftp_run(void);
void stop_ftp(void);
void run_ftp(void);
void control_ftp_fw(int is_run_before);
void restart_ftpd(void);
#endif
#if defined (APP_SMBD)
int write_smb_conf(void);
void config_smb_fastpath(int check_pid);
void stop_samba(int force_stop);
void run_samba(void);
void restart_smbd(void);
#endif
#if defined (APP_NFSD)
void unload_nfsd(void);
void stop_nfsd(void);
void run_nfsd(void);
void reload_nfsd(void);
void restart_nfsd(void);
#endif
#if defined (APP_MINIDLNA)
int is_dms_run(void);
void update_minidlna_conf(const char *link_path, const char *conf_path);
void stop_dms(void);
void run_dms(int force_rescan);
void restart_dms(int force_rescan);
#endif
#if defined (APP_FIREFLY)
int is_itunes_run(void);
void stop_itunes(void);
void run_itunes(void);
void restart_itunes(void);
#endif
#if defined (APP_TRMD)
int is_torrent_run(void);
int is_torrent_support(void);
void stop_torrent(void);
void run_torrent(void);
void restart_torrent(void);
#endif
#if defined (APP_ARIA)
int is_aria_run(void);
int is_aria_support(void);
void stop_aria(void);
void run_aria(void);
void restart_aria(void);
#endif
int count_stor_mountpoint(void);
void umount_stor_path(struct disk_info_t *disks_info, int port, const char *dev_name, int do_spindown);
void umount_ejected(void);
void start_stor_apps(void);
void stop_stor_apps(void);
void on_deferred_hotplug_dev(void);
void safe_remove_stor_device(int port_b, int port_e, const char *dev_name, int do_spindown);
void safe_remove_all_stor_devices(int do_spindown);
#endif

#if defined (USE_USB_SUPPORT)
/* services_usb.c */
#if defined (SRV_U2EC)
void start_u2ec(void);
void stop_u2ec(void);
#endif
#if defined (SRV_LPRD)
void start_lpd(void);
void stop_lpd(void);
#endif
void start_p910nd(char *devlp);
void stop_p910nd(void);
void safe_remove_usb_device(int port, const char *dev_name);
void power_control_usb_port(int port, int power_on);
void restart_usb_printer_spoolers(void);
void stop_usb_printer_spoolers(void);
void try_start_usb_printer_spoolers(void);
void try_start_usb_modem_to_wan(void);
#endif

/* firewall_ex.c */
void ipt_nat_default(void);
void ipt_filter_default(void);
void fill_static_routes(char *buf, int len, const char *ift);
void ip2class(const char *addr, const char *mask, char *out_buf, size_t out_len);
void start_firewall_ex(void);

/* ralink.c */
int get_wired_mac_is_single(void);
int get_wired_mac_e2p_offset(int is_wan);
int get_wired_mac(int is_wan);
int set_wired_mac(int is_wan, const char *mac);
int get_wireless_mac_e2p_offset(int is_5ghz);
int get_wireless_mac(int is_5ghz);
int set_wireless_mac(int is_5ghz, const char *mac);
int get_wireless_cc(void);
int set_wireless_cc(const char *cc);
int gen_ralink_config_2g(int disable_autoscan);
int gen_ralink_config_5g(int disable_autoscan);
int getPIN(void);
int setPIN(const char *pin);
int getBootVer(void);
int get_apcli_connected(const char *ifname);
int check_regspec_code(const char *spec);

/* watchdog.c */
#if defined (BOARD_GPIO_BTN_WPS) || defined (BOARD_GPIO_BTN_FN1) || defined (BOARD_GPIO_BTN_FN2)
void ez_event_short(int btn_id);
void ez_event_long(int btn_id);
#endif
int  is_ntpc_updated(void);
int  ntpc_updated_main(int argc, char *argv[]);
int  watchdog_main(int argc, char *argv[]);
int  start_watchdog(void);
void notify_watchdog_time(void);
void notify_watchdog_wifi(int is_5ghz);

#if defined (USE_RT3352_MII)
/* inicd */
int inicd_main(int argc, char *argv[]);
int start_inicd(void);
int stop_inicd(void);
#endif

#if defined (USE_SMP)
/* smp.c */
void set_cpu_affinity(int is_ap_mode);
void set_vpn_balancing(const char *vpn_ifname, int is_server);
#else
#define set_cpu_affinity(x)
#define set_vpn_balancing(ptr,val)
#endif

/* rstats.c */
int  rstats_main(int argc, char *argv[]);
void notify_rstats_time(void);

/* detect_link.c */
int detect_link_main(int argc, char *argv[]);
int start_detect_link(void);
void stop_detect_link(void);
void notify_reset_detect_link(void);
void notify_leds_detect_link(void);
void show_hide_front_leds(int is_show);
void show_hide_ether_leds(int is_show);

/* detect_internet.c */
int detect_internet_main(int argc, char *argv[]);
int start_detect_internet(int autorun_time);
void stop_detect_internet(void);
void notify_run_detect_internet(int delay_time);
void notify_pause_detect_internet(void);

/* detect_wan.c */
int detect_wan_main(int argc, char *argv[]);

#if defined (USE_USB_SUPPORT)
/* usb_modem.c */
int  get_modem_devnum(void);
int  get_modem_ndis_ifname(char ndis_ifname[16], int *devnum_out);
void notify_modem_on_wan_ether_link_changed(int has_link);
void safe_remove_usb_modem(void);
void unload_modem_modules(void);
void reload_modem_modules(int modem_type, int reload);
int  launch_wan_modem_ras(int unit);
int  launch_wan_usbnet(int unit);
void stop_wan_usbnet(void);
int  zerocd_main(int argc, char **argv);

/* hotplug_usb.c */
int  usb_port_module_used(const char *mod_usb);
int  mdev_sg_main(int argc, char **argv);
int  mdev_sr_main(int argc, char **argv);
int  mdev_lp_main(int argc, char **argv);
int  mdev_net_main(int argc, char **argv);
int  mdev_tty_main(int argc, char **argv);
int  mdev_wdm_main(int argc, char **argv);
#endif

#if defined (USE_STORAGE)
/* hotplug_stor.c */
#if defined (USE_BLK_DEV_SD)
int  mdev_sd_main(int argc, char **argv);
#endif
#if defined (USE_MMC_SUPPORT)
int  mdev_mmc_main(int argc, char **argv);
#endif
#endif

#endif /* _rc_h_ */
