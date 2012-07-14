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
 *
 * Router rc control script
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifndef _rc_h_
#define _rc_h_

//#include <bcmconfig.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <shutils.h>

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

#include <usb_info.h>

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

#define IFNAME_BR "br0"
#define IFNAME_MAC "eth2"
#ifdef USE_SINGLE_MAC
#define IFNAME_LAN "eth2.1"
#define IFNAME_WAN "eth2.2"
#else
#define IFNAME_LAN "eth2"
#define IFNAME_WAN "eth3"
#endif

/* rc.c */
void sys_exit(void);
void setenv_tz(void);
void set_timezone(void);
void nvram_restore_defaults(void);
int shutdown_prepare(void);
void reload_nat_modules(void);
void rc_restart_firewall(void);
int start_detect_internet(void);
void stop_detect_internet(void);
void start_flash_usbled(void);
void stop_flash_usbled(void);
int start_watchdog(void);

/* udhcpc.c */
int zcip_main(int argc, char **argv);
int udhcpc_main(int argc, char **argv);
int start_udhcpc_wan(const char *wan_ifname, int unit, int wait_lease);
int start_zcip_wan(const char *wan_ifname);
int release_udhcpc_wan(int unit);

/* udhcpc_ex.c */
int udhcpc_ex_main(int argc, char **argv);
int start_udhcpc_lan(const char *lan_ifname);

/* auth.c */
int  wpacli_main(int argc, char **argv);
int  start_auth_eapol(const char *ifname);
void stop_auth_eapol(void);
int  start_auth_kabinet(void);
void stop_auth_kabinet(void);

/* ppp.c */
int ipup_main(int argc, char **argv);
int ipdown_main(int argc, char **argv);
int ppp_ifunit(char *ifname);

/* ntp.c */
int ntp_main(int argc, char **argv);

/* init.c */
int console_init(void);
pid_t run_shell(int timeout, int nowait);
void signal_init(void);
void fatal_signal(int sig);

/* common_ex.c */
long uptime(void);
int rand_seed_by_time(void);
in_addr_t inet_addr_(const char *cp);
void update_lan_status(int isup);
void update_wan_status(int isup);
void wanmessage(char *fmt, ...);
char *pppstatus(char *buf);
void reset_lan_vars(void);
void reset_wan_vars(int full_reset);
void logmessage(char *logheader, char *fmt, ...);
void restart_all_sysctl(void);
void convert_asus_values(int skipflag);
void init_router_mode();
char *mac_conv(char *mac_name, int idx, char *buf);
char *mac_conv2(char *mac_name, int idx, char *buf);
void getsyspara(void);
char *trim_r(char *str);
void char_to_ascii(char *output, char *input);

/* network.c */
void start_lan(void);
void stop_lan(void);
void start_wan(void);
void stop_wan(void);
void stop_wan_ppp(void);
void stop_wan_static(void);
void wan_up(char *ifname);
void wan_down(char *ifname);
void lan_up(char *lan_ifname);
void lan_up_ex(char *lan_ifname);
void lan_down_ex(char *lan_ifname);
int  route_add(char *name, int metric, char *dst, char *gateway, char *genmask);
int  route_del(char *name, int metric, char *dst, char *gateway, char *genmask);
void kill_services(char* svc_name[], int wtimeout, int forcekill);
void config_loopback(void);
int  ifconfig(char *ifname, int flags, char *addr, char *netmask);
int  create_resolvconf(void);
void wan_mac_config(void);
void switch_config_base(void);
void switch_config_storm(void);
void switch_config_link(void);
void switch_config_vlan(int first_call);
void select_usb_modem_to_wan(int wait_modem_sec);
void full_restart_wan(void);
void try_wan_reconnect(int try_use_modem);
int  is_interface_up(const char *ifname);
void start_wifi_ap_wl(int radio_on);
void start_wifi_ap_rt(int radio_on);
void start_wifi_wds_wl(int radio_on);
void start_wifi_wds_rt(int radio_on);
void start_wifi_apcli_wl(int radio_on);
void start_wifi_apcli_rt(int radio_on);
void restart_wifi_wl(int radio_on, int need_reload_conf);
void control_wifi_guest_wl(void);
void restart_wifi_rt(int radio_on, int need_reload_conf);
void control_wifi_guest_rt(void);
void stop_wifi_all_wl(void);
void stop_wifi_all_rt(void);
void stop_igmpproxy(void);
void start_igmpproxy(char *wan_ifname);
int  wan_ifunit(char *ifname);
int  wan_primary_ifunit(void);
int  update_resolvconf(void);
int  is_ap_mode(void);
int  preset_wan_routes(char *ifname);
void safe_remove_usb_modem(void);
void flush_conntrack_caches(void);
void flush_route_caches(void);
int  is_module_loaded(char *module_name);
int  is_hwnat_allow(void);
int  is_hwnat_loaded(void);
int  is_fastnat_allow(void);
void hwnat_load(void);
void hwnat_unload(void);
int  is_ftp_conntrack_loaded(int ftp_port);
void get_wan_ifname(char wan_ifname[16]);
void bridge_init(void);
int  is_dns_static(void);
int  is_physical_wan_dhcp(void);
void start_mac_clone(void);
void dumparptable(void);
int  has_wan_ip(int only_broadband_wan);
int  is_interface_exist(const char *ifname);
int  is_phyconnected(void);
int  found_default_route(int only_broadband_wan);
in_addr_t get_wan_ipaddr(int only_broadband_wan);
in_addr_t get_lan_ipaddr(void);

/* network_ex.c */
void set_ppp_limit_cpu(void);
int write_xl2tpd_conf(char *l2tp_conf);
int start_pppd(char *prefix);
void restart_xl2tpd(void);
void start_pppoe_relay(char *wan_if);

/* services.c */
void nvram_commit_safe(void);
void stop_telnetd(void);
void run_telnetd(void);
void start_telnetd(void);
void stop_sshd(void);
void start_sshd(void);
void restart_term(void);
int start_httpd(void);
void stop_httpd(void);
void restart_httpd(void);
int start_upnp(void);
void stop_upnp(void);
void smart_restart_upnp(void);
void update_upnp(int force_update);
int start_vpn_server(void);
void stop_vpn_server(void);
void restart_vpn_server(void);
int start_ntpc(void);
void stop_ntpc(void);
int start_lltd(char *wlan_ifname);
void stop_lltd(void);
void startup_lltd(void);
void stop_rstats(void);
void start_rstats(int new);
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
void start_u2ec(void);
void stop_u2ec(void);
void start_lpd(void);
void stop_lpd(void);
void start_p910nd(char *devlp);
void stop_p910nd(void);
void stop_nfsd(void);
void stop_samba(void);
void stop_ftp(void);
void stop_dms(void);
void stop_torrent(int no_restart_firewall);
void run_ftp(void);
void run_samba(void);
void run_nfsd(void);
void run_dms(void);
void run_torrent(int no_restart_firewall);
int is_torrent_run(void);
int is_torrent_support(void);
void restart_dms(void);
void restart_torrent(void);
void stop_networkmap(void);
void restart_networkmap(void);
int start_dns_dhcpd(void);
void stop_dns_dhcpd(void);
int ddns_updated_main(int argc, char *argv[]);
int start_ddns(int forced);
void stop_ddns(void);
void stop_misc(void);
void stop_misc_no_watchdog(void);
int start_infosvr(void);
void stop_infosvr(void);
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
void umount_dev(char *sd_dev);
void umount_dev_all(char *sd_dev);
void umount_sddev_all(void);
int stop_service_main(int type);
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
void convert_routes(void);
void ip2class(char *lan_ip, char *netmask, char *buf);
int porttrigger_setting_without_netconf(FILE *fp);
int start_firewall_ex(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip);

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
int gettxbfcal(void);


/* watchdog.c */
int watchdog_main(int argc, char *argv[]);
void refresh_ntpc(void);
int control_radio_wl(int radio_on);
int control_radio_rt(int radio_on);

/* linkstatus.c */
int linkstatus_monitor_main(int argc, char *argv[]);
int start_linkstatus_monitor(void);
void stop_linkstatus_monitor(void);

/* detect_internet.c */
int detect_internet_main(int argc, char *argv[]);

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
int asus_sd(const char *device_name, const char *action);
int asus_sg(const char *device_name, const char *action);
int asus_sr(const char *device_name, const char *action);
int asus_lp(const char *device_name, const char *action);
int asus_net(const char *device_name, const char *action);
int asus_tty(const char *device_name, const char *action);
int asus_usb_interface(const char *device_name, const char *action);


// for log message title
#define LOGNAME	"RT-N56U"
#define ERR	"err"

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
