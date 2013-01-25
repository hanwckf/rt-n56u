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

#include <shutils.h>
#include <usb_info.h>
#include <boards.h>

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

#define IFNAME_MAC  "eth2"
#define IFNAME_MAC2 "eth3"
#ifdef USE_SINGLE_MAC
#define IFNAME_LAN  "eth2.1"
#define IFNAME_WAN  "eth2.2"
#else
#define IFNAME_LAN  IFNAME_MAC
#define IFNAME_WAN  IFNAME_MAC2
#endif
#if defined(USE_RT3352_MII)
#define MIN_EXT_VLAN_VID     4
#define INIC_GUEST_VLAN_VID  3
#define IFNAME_INIC_GUEST_AP "eth2.3"
#define IFNAME_INIC_APCLI    "apcli1" // bad APCli name in iNIC_mii (must be apclii%d)
#else
#define MIN_EXT_VLAN_VID     3
#define IFNAME_INIC_APCLI    "apclii0"
#endif

#define SCRIPT_UDHCPC_LAN "/tmp/udhcpc_lan.script"
#define SCRIPT_UDHCPC_WAN "/tmp/udhcpc.script"
#define SCRIPT_ZCIP_WAN   "/tmp/zcip.script"
#define SCRIPT_ZCIP_VIPTV  "/tmp/zcip_viptv.script"
#define SCRIPT_WPACLI_WAN "/tmp/wpacli.script"
#define SCRIPT_DHCP6C_WAN "/tmp/dhcp6c.script"

#define SR_PREFIX_LAN "LAN"
#define SR_PREFIX_MAN "MAN"
#define SR_PREFIX_WAN "WAN"

#define MAX_CLIENTS_NUM (50)

/* rc.c */
void setenv_tz(void);
void init_router(void);
void shutdown_router(void);
void handle_notifications(void);
void LED_CONTROL(int led, int flag);

/* init.c */
void init_main_loop(void);
void sys_exit(void);

/* auth.c */
int  wpacli_main(int argc, char **argv);
int  start_auth_eapol(const char *ifname);
void stop_auth_eapol(void);
int  start_auth_kabinet(void);
void stop_auth_kabinet(void);

/* common_ex.c */
long uptime(void);
int rand_seed_by_time(void);
in_addr_t inet_addr_(const char *cp);
void wan_netmask_check(void);
void logmessage(char *logheader, char *fmt, ...);
void restart_all_sysctl(void);
void convert_asus_values(int skipflag);
void init_router_mode();
void update_router_mode();
char *mac_conv(char *mac_name, int idx, char *buf);
char *mac_conv2(char *mac_name, int idx, char *buf);
void getsyspara(void);
char *trim_r(char *str);
void char_to_ascii(char *output, char *input);
int fput_string(const char *name, const char *value);
int fput_int(const char *name, int value);

/* net.c */
int  control_static_routes(char *ift, char *ifname, int is_add);
int  route_add(char *name, int metric, char *dst, char *gateway, char *genmask);
int  route_del(char *name, int metric, char *dst, char *gateway, char *genmask);
int  ifconfig(char *ifname, int flags, char *addr, char *netmask);
void kill_services(char* svc_name[], int wtimeout, int forcekill);
int  is_interface_up(const char *ifname);
#if defined(APP_XUPNPD)
void stop_xupnpd(void);
void start_xupnpd(char *wan_ifname);
#endif
void stop_igmpproxy(char *wan_ifname);
void start_igmpproxy(char *wan_ifname);
void restart_iptv(void);
int  is_ap_mode(void);
int  preset_wan_routes(char *ifname);
void safe_remove_usb_modem(void);
void flush_conntrack_caches(void);
void flush_route_caches(void);
void clear_if_route4(char *ifname);
int  is_module_loaded(char *module_name);
int  is_hwnat_allow(void);
int  is_hwnat_loaded(void);
int  is_fastnat_allow(void);
int  is_ftp_conntrack_loaded(int ftp_port0, int ftp_port1);
int  is_interface_exist(const char *ifname);
int  is_phyconnected(void);
int  found_default_route(int only_broadband_wan);
void hwnat_load(void);
void hwnat_unload(void);
void hwnat_configure(void);
void swnat_configure(void);
void reload_nat_modules(void);
void restart_firewall(void);
in_addr_t get_ipv4_addr(char* ifname);

/* net_lan.c */
in_addr_t get_lan_ipaddr(void);
int add_static_lan_routes(char *lan_ifname);
int del_static_lan_routes(char *lan_ifname);
void reset_lan_vars(void);
void start_lan(void);
void stop_lan(void);
void lan_up_manual(char *lan_ifname);
void lan_up_auto(char *lan_ifname);
void lan_down_auto(char *lan_ifname);
void update_lan_status(int isup);
void full_restart_lan(void);
void init_loopback(void);
void init_bridge(void);
void switch_config_base(void);
void switch_config_storm(void);
void switch_config_link(void);
void switch_config_vlan(int first_call);
int  is_vlan_vid_inet_valid(int vlan_vid_inet);
int  is_vlan_vid_iptv_valid(int vlan_vid_inet, int vlan_vid_iptv);
int  start_udhcpc_lan(const char *lan_ifname);
int  stop_udhcpc_lan();
int  udhcpc_lan_main(int argc, char **argv);

/* net_wan.c */
void reset_wan_vars(int full_reset);
void set_man_ifname(char *man_ifname, int unit);
char*get_man_ifname(int unit);
int  get_vlan_vid_wan(void);
void start_wan(void);
void stop_wan(void);
void stop_wan_ppp(void);
void stop_wan_static(void);
void wan_up(char *ifname);
void wan_down(char *ifname);
void select_usb_modem_to_wan(int wait_modem_sec);
void full_restart_wan(void);
void try_wan_reconnect(int try_use_modem);
void add_dhcp_routes(char *prefix, char *ifname, int metric);
int  add_static_wan_routes(char *wan_ifname);
int  del_static_wan_routes(char *wan_ifname);
int  add_static_man_routes(char *wan_ifname);
int  del_static_man_routes(char *wan_ifname);
int  update_resolvconf(int is_first_run, int do_not_notify);
int  update_hosts(void);
int  wan_ifunit(char *ifname);
int  wan_primary_ifunit(void);
int  is_wan_ppp(char *wan_proto);
int  wan_prefix(char *ifname, char *prefix);
void get_wan_ifname(char wan_ifname[16]);
void update_wan_status(int isup);
in_addr_t get_wan_ipaddr(int only_broadband_wan);
int  has_wan_ip(int only_broadband_wan);
int  is_ifunit_modem(char *wan_ifname);
int  is_dns_static(void);
int  is_physical_wan_dhcp(void);
int udhcpc_main(int argc, char **argv);
int zcip_main(int argc, char **argv);
int zcip_viptv_main(int argc, char **argv);
int start_udhcpc_wan(const char *wan_ifname, int unit, int wait_lease);
int renew_udhcpc_wan(int unit);
int release_udhcpc_wan(int unit);
int stop_udhcpc_wan(int unit);
int start_zcip_wan(const char *wan_ifname);
int start_zcip_viptv(const char *wan_ifname);

/* net_ppp.c */
int write_xl2tpd_conf(char *l2tp_conf);
int start_pppd(char *prefix);
void restart_xl2tpd(void);
void set_ip_forward(void);
void set_ppp_limit_cpu(void);
void set_pppoe_passthrough(void);
void disable_all_passthrough(void);
int ipup_main(int argc, char **argv);
int ipdown_main(int argc, char **argv);
int ipup_vpns_main(int argc, char **argv);
int ipdown_vpns_main(int argc, char **argv);
int ppp_ifunit(char *ifname);

#if defined (USE_IPV6)
/* net6.c */
void init_ipv6(void);
void control_if_ipv6_all(int enable);
void control_if_ipv6(char *ifname, int enable);
void control_if_ipv6_autoconf(char *ifname, int enable);
void control_if_ipv6_radv(char *ifname, int enable);
void control_if_ipv6_dad(char *ifname, int enable);
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

/* net_lan6.c */
int is_lan_addr6_static(void);
int is_lan_radv_on(void);
int is_lan_dhcp6s_on(void);
int store_lan_addr6(char *lan_addr6_new);
void reload_lan_addr6(void);
void clear_lan_addr6(void);
void reset_lan6_vars(void);
char *get_lan_addr6_host(char *p_addr6s);
char *get_lan_addr6_prefix(char *p_addr6s);
int reload_radvd(void);
void stop_radvd(void);
void restart_radvd(void);

/* net_wan6.c */
int is_wan_dns6_static(void);
int is_wan_addr6_static(void);
int is_wan_ipv6_type_sit(void);
int is_wan_ipv6_if_ppp(void);
int  store_wan_dns6(char *dns6_new);
void reset_wan6_vars(void);
void store_ip6rd_from_dhcp(const char *env_value, const char *prefix);
void start_sit_tunnel(int ipv6_type, char *wan_addr4, char *wan_addr6);
void stop_sit_tunnel(void);
void wan6_up(char *wan_ifname);
void wan6_down(char *wan_ifname);
int dhcp6c_main(int argc, char **argv);
int start_dhcp6c(char *wan_ifname);
void stop_dhcp6c(void);

/* net_ppp.c */
int ipv6up_main(int argc, char **argv);
int ipv6down_main(int argc, char **argv);

/* firewall_ex.c */
void default_filter6_setting(void);
#endif

/* net_wifi.c */
void mlme_state_wl(int is_on);
void mlme_state_rt(int is_on);
void mlme_radio_wl(int is_on);
void mlme_radio_rt(int is_on);
int  get_mlme_radio_wl(void);
int  get_mlme_radio_rt(void);
void start_wifi_ap_wl(int radio_on);
void start_wifi_ap_rt(int radio_on);
void start_wifi_wds_wl(int radio_on);
void start_wifi_wds_rt(int radio_on);
void start_wifi_apcli_wl(int radio_on);
void start_wifi_apcli_rt(int radio_on);
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
int  timecheck_wifi(char *nv_date, char *nv_time1, char *nv_time2);

/* services.c */
void nvram_commit_safe(void);
void stop_telnetd(void);
void run_telnetd(void);
void start_telnetd(void);
int is_sshd_run(void);
void stop_sshd(void);
void start_sshd(void);
void restart_term(void);
void start_httpd(int restart_fw);
void stop_httpd(void);
void restart_httpd(void);
int start_upnp(void);
void stop_upnp(void);
void smart_restart_upnp(void);
void update_upnp(int force_update);
int start_vpn_server(void);
void stop_vpn_server(void);
void restart_vpn_server(void);
int start_lltd(void);
void stop_lltd(void);
void stop_rstats(void);
void start_rstats(void);
int start_services(void);
void stop_services(int stopall);
void stop_services_lan_wan(void);
void write_storage_to_mtd(void);
void erase_storage(void);
void erase_nvram(void);
int start_logger(int showinfo);
void stop_logger(void);

/* services_ex.c */
int mkdir_if_none(char *dir);
void start_infosvr(void);
void stop_infosvr(void);
void start_u2ec(void);
void stop_u2ec(void);
void start_lpd(void);
void stop_lpd(void);
void start_p910nd(char *devlp);
void stop_p910nd(void);
void stop_nfsd(void);
void stop_samba(void);
void stop_ftp(void);
void run_ftp(void);
void run_samba(void);
void run_nfsd(void);
int is_ftp_run(void);
void restart_ftp(void);
#if defined(APP_MINIDLNA)
int is_dms_run(void);
void update_minidlna_conf(const char *link_path, const char *conf_path);
void stop_dms(void);
void run_dms(void);
void restart_dms(void);
#endif
#if defined(APP_FIREFLY)
int is_itunes_run(void);
void update_firefly_conf(const char *link_path, const char *conf_path);
void stop_itunes(void);
void run_itunes(void);
void restart_itunes(void);
#endif
#if defined(APP_TRMD)
int is_torrent_run(void);
int is_torrent_support(void);
void stop_torrent(void);
void run_torrent(int no_restart_firewall);
void restart_torrent(void);
#endif
#if defined(APP_ARIA)
int is_aria_run(void);
int is_aria_support(void);
void stop_aria(void);
void run_aria(int no_restart_firewall);
void restart_aria(void);
#endif
void stop_networkmap(void);
void restart_networkmap(void);
int start_dns_dhcpd(void);
void stop_dns_dhcpd(void);
int try_start_dns_dhcpd(void);
int ddns_updated_main(int argc, char *argv[]);
int start_ddns(int forced);
void stop_ddns(void);
void stop_misc(int stop_watchdog);
int load_usb_printer_module(void);
int load_usb_storage_module(void);
void stop_usb(void);
void restart_usb_printer_spoolers(void);
int is_usb_printer_exist(char devlp[16]);
void try_start_usb_printer_spoolers(void);
void stop_usb_printer_spoolers(void);
void on_deferred_hotplug_usb(void);
void umount_ejected(void);
void umount_usb_path(int port);
int count_sddev_mountpoint(void);
int count_sddev_partition(void);
void start_usb_apps(void);
void stop_usb_apps(void);
void try_start_usb_apps(void);
void umount_sddev_all(void);
int is_valid_hostname(const char *name);
void manual_wan_disconnect(void);
void manual_wan_connect(void);
void manual_ddns_hostname_check(void);
void try_start_usb_modem_to_wan(void);
int restart_dhcpd(void);
int restart_dns(void);
int safe_remove_usb_mass(int port);
int check_if_file_exist(const char *filepath);
int check_if_dir_exist(const char *dirpath);
void umount_dev(char *sd_dev);
void umount_dev_all(char *sd_dev);
void umount_sddev_all(void);
int stop_service_main(int argc, char *argv[]);
void start_8021x_wl(void);
void stop_8021x_wl(void);
void start_8021x_rt(void);
void stop_8021x_rt(void);
int start_networkmap(void);
void stop_syslogd();
void stop_klogd();
int start_syslogd();
int start_klogd();

/* firewall_ex.c */
void default_nat_setting(void);
void default_filter_setting(void);
void fill_static_routes(char *buf, int len, const char *ift);
void ip2class(char *lan_ip, char *netmask, char *buf);
int start_firewall_ex(char *wan_if, char *wan_ip);

/* ralink.c */
int getMAC(void);
int getMAC_2G(void);
int setMAC(const char *mac);
int setMAC_2G(const char *mac);
int getCountryCode(void);
int setCountryCode(const char *cc);
int FREAD(unsigned int addr_sa, int len);
int FWRITE(char *da, char* str_hex);
int gen_ralink_config_wl(int disable_autoscan);
int gen_ralink_config_rt(int disable_autoscan);
int getPIN(void);
int setPIN(const char *pin);
int getBootVer(void);
int getCountryRegion(const char *str);
int getCountryRegionABand(const char *str);
int getSSID(void);
int getChannel(void);
int getChannel_2G(void);
int getSiteSurvey(void);
int getSiteSurvey_2G(void);
int getBSSID(void);
int get_channel(void);
int get_channel_2G(void);
int asuscfe(const char *PwqV, const char *IF);
int stainfo(void);
int stainfo_2g(void);
int getstat(void);
int getstat_2g(void);
int getrssi(void);
int getrssi_2g(void);

/* watchdog.c */
int  watchdog_main(int argc, char *argv[]);
int  start_watchdog(void);
void notify_watchdog_time(void);
void notify_watchdog_ddns(void);
void notify_watchdog_nmap(void);

/* detect_link.c */
int detect_link_main(int argc, char *argv[]);
int start_detect_link(void);
void stop_detect_link(void);
void start_flash_usbled(void);
void stop_flash_usbled(void);

/* detect_internet.c */
int detect_internet_main(int argc, char *argv[]);
int start_detect_internet(void);
void stop_detect_internet(void);

/* detect_wan.c */
int detect_wan_main(int argc, char *argv[]);

/* usb_devices.c */
int write_3g_ppp_conf(const char *modem_node);
int create_pppd_script_modem_3g(void);
int is_ready_modem_node_3g(void);
int is_ready_modem_3g(void);
int is_ready_modem_4g(void);
void stop_modem_3g(void);
void stop_modem_4g(void);
void detach_swap_partition(char *part_name);
int mdev_sg_main(int argc, char **argv);
int mdev_sd_main(int argc, char **argv);
int mdev_sr_main(int argc, char **argv);
int mdev_lp_main(int argc, char **argv);
int mdev_net_main(int argc, char **argv);
int mdev_tty_main(int argc, char **argv);
int mdev_usb_main(int argc, char **argv);

// for log message title
#define ERR		"err"
#define LOGNAME		BOARD_NAME


#define varkey_nvram_set(key, value, args...)({ \
        char nvram_word[64]; \
        memset(nvram_word, 0x00, sizeof(nvram_word)); \
        sprintf(nvram_word, key, ##args); \
        nvram_set(nvram_word, value); \
})

#define varval_nvram_set(key, value, args...)({ \
        char nvram_value[64]; \
        memset(nvram_value, 0x00, sizeof(nvram_value)); \
        sprintf(nvram_value, value, ##args); \
        nvram_set(key, nvram_value); \
})

#define MACSIZE 12
#endif /* _rc_h_ */
