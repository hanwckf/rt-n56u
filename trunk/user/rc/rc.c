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
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <dirent.h>

#include <shutils.h>
#include <nvram/bcmnvram.h>
#include <ralink.h>
#include <notify_rc.h>

#include "rc.h"
#include "rtl8367.h"

extern struct nvram_tuple router_defaults[];

/* static values */
static int nvram_modem_type = 0;
static int nvram_modem_rule = 0;
static int nvram_nf_nat_type = 0;
static int nvram_ipv6_type = 0;

static void
nvram_restore_defaults(void)
{
	struct nvram_tuple *t;
	int restore_defaults;

	/* Restore defaults if told to or OS has changed */
	restore_defaults = !nvram_match("restore_defaults", "0");
	
	/* check asus-wrt NVRAM content (sorry, but many params is incompatible) */
	if (!restore_defaults) {
		if (nvram_get("buildno") && nvram_get("buildinfo") && nvram_get("extendno"))
			restore_defaults = 1;
	}

	if (restore_defaults)
		nvram_clear();

	/* Restore defaults */
	for (t = router_defaults; t->name; t++) {
		if (restore_defaults || !nvram_get(t->name)) {
			nvram_set(t->name, t->value);
		}
	}

	klogctl(8, NULL, nvram_get_int("console_loglevel"));

	/* load static values */
	nvram_modem_type = nvram_get_int("modem_type");
	nvram_modem_rule = nvram_get_int("modem_rule");
	nvram_nf_nat_type = nvram_get_int("nf_nat_type");
	nvram_ipv6_type = get_ipv6_type();

	/* Commit values */
	if (restore_defaults) {
		nvram_commit();
	}
}


static void
insertmodules(void)
{
#if defined(BOARD_N65U)
	set_pcie_aspm();
#endif
#if defined(USE_USB3)
	system("modprobe xhci-hcd");
#else
	system("modprobe ehci-hcd");
	system("modprobe ohci-hcd");
#endif
	system("modprobe rt2860v2_ap");
#if defined(USE_RT3090_AP)
	system("modprobe rt3090_ap");
#endif
}

static void
set_timezone(void)
{
	time_zone_x_mapping();
	setenv_tz();
	setkernel_tz();
}

static void
init_gpio_leds_buttons(void)
{
#if defined(LED_WAN)
	cpu_gpio_set_pin_direction(LED_WAN, GPIO_DIR_OUT);
	LED_CONTROL(LED_WAN, LED_OFF);
#endif
#if defined(LED_LAN)
	cpu_gpio_set_pin_direction(LED_LAN, GPIO_DIR_OUT);
	LED_CONTROL(LED_LAN, LED_OFF);
#endif
#if defined(LED_USB)
	cpu_gpio_set_pin_direction(LED_USB, GPIO_DIR_OUT);
	LED_CONTROL(LED_USB, LED_OFF);
#endif
#if defined(LED_ALL)
	cpu_gpio_set_pin_direction(LED_ALL, GPIO_DIR_OUT);
	LED_CONTROL(LED_ALL, LED_ON);
#endif
	cpu_gpio_set_pin_direction(LED_POWER, GPIO_DIR_OUT);
	LED_CONTROL(LED_POWER, LED_ON);

	cpu_gpio_set_pin_direction(BTN_RESET, GPIO_DIR_IN);
#if defined(BTN_WPS)
	cpu_gpio_set_pin_direction(BTN_WPS, GPIO_DIR_IN);
#endif
}

static void
set_wan0_vars(void)
{
	struct nvram_tuple *t;
	char *v;
	char tmp[100], prefix[16];
	
	/* Write through to wan0_ variable set */
	snprintf(prefix, sizeof(prefix), "wan%d_", 0);
	for (t = router_defaults; t->name; t ++) {
		if (!strncmp(t->name, "wan_", 4)) {
			if (nvram_get(strcat_r(prefix, &t->name[4], tmp)))
				continue;
			v = nvram_get(t->name);
			nvram_set(tmp, v ? v : t->value);
		}
	}
	nvram_set_int(strcat_r(prefix, "unit", tmp), WAN_PPP_UNIT);
	nvram_set_int(strcat_r(prefix, "primary", tmp), 1);
	nvram_set(strcat_r(prefix, "ifname", tmp), IFNAME_WAN);
	nvram_set(strcat_r(prefix, "desc", tmp), "Default Connection");
}

static void 
convert_misc_values()
{
	char buff[128];
	char *test_value;

	nvram_unset("lan_route");
	nvram_unset("wan0_route");
	nvram_unset("wan_route");

	nvram_unset("wanx_ipaddr"); 
	nvram_unset("wanx_netmask");
	nvram_unset("wanx_gateway");
	nvram_unset("wanx_dns");
	nvram_unset("wanx_lease");

	test_value = nvram_safe_get("wan_heartbeat_x");
	if (*test_value && strlen(nvram_safe_get("wan_ppp_peer")) == 0)
		nvram_set("wan_ppp_peer", test_value);

	if (nvram_match("modem_pin", "") && nvram_invmatch("wan_3g_pin", ""))
		nvram_set("modem_pin", nvram_safe_get("wan_3g_pin"));
	nvram_unset("wan_3g_pin");
	nvram_unset("wan0_3g_pin");

	if (!strcmp(nvram_safe_get("wl_ssid"), ""))
		nvram_set("wl_ssid", "ASUS_5G");

	if (!strcmp(nvram_safe_get("rt_ssid"), ""))
		nvram_set("rt_ssid", "ASUS");

	memset(buff, 0, sizeof(buff));
	char_to_ascii(buff, nvram_safe_get("wl_ssid"));
	nvram_set("wl_ssid2", buff);

	memset(buff, 0, sizeof(buff));
	char_to_ascii(buff, nvram_safe_get("rt_ssid"));
	nvram_set("rt_ssid2", buff);

	if (nvram_match("wl_wpa_mode", ""))
		nvram_set("wl_wpa_mode", "0");

	if (!strcmp(nvram_safe_get("wl_gmode"), ""))
		nvram_set("wl_gmode", "2");

	if (!strcmp(nvram_safe_get("rt_gmode"), ""))
		nvram_set("rt_gmode", "2");

	if (nvram_get_int("wl_HT_BW") > 1)
		nvram_set_int("wl_HT_BW", 1);

	if (nvram_get_int("rt_HT_BW") > 1)
		nvram_set_int("rt_HT_BW", 1);

	if (nvram_get_int("wl_stream_tx") > RT3883_RF_TX)
		nvram_set_int("wl_stream_tx", RT3883_RF_TX);

	if (nvram_get_int("wl_stream_rx") > RT3883_RF_RX)
		nvram_set_int("wl_stream_rx", RT3883_RF_RX);

	if (nvram_get_int("rt_stream_tx") > INIC_RF_TX)
		nvram_set_int("rt_stream_tx", INIC_RF_TX);

	if (nvram_get_int("rt_stream_rx") > INIC_RF_RX)
		nvram_set_int("rt_stream_rx", INIC_RF_RX);

	nvram_set("lan_ipaddr_t", "");
	nvram_set("lan_netmask_t", "");
	nvram_set("lan_gateway_t", "");
	nvram_set("lan_dns_t", "");

	nvram_set("wan_ipaddr_t", "");
	nvram_set("wan_netmask_t", "");
	nvram_set("wan_gateway_t", "");
	nvram_set("wan_dns_t", "");

	nvram_set("reboot", "");
	nvram_set("viptv_ifname", "");

	nvram_set_int("networkmap_fullscan", 0);
	nvram_set_int("fullscan_timestamp", 0);
	nvram_set_int("detect_timestamp", 0);
	nvram_set_int("link_internet", 2);

	nvram_set_int("reload_svc_wl", 0);
	nvram_set_int("reload_svc_rt", 0);

	nvram_set_int("link_wan", 0);
	nvram_set_int("link_lan", 0);
	nvram_set_int("usb_hotplug_ms", 0);
	nvram_set_int("usb_hotplug_lp", 0);
	nvram_set_int("usb_hotplug_md", 0);
	nvram_set_int("usb_unplug_lp", 0);
	nvram_set_int("usb_unplug_md", 0);

	nvram_set_int("l2tp_wan_t", 0);
	nvram_set_int("l2tp_cli_t", 0);
	nvram_set_int("l2tp_srv_t", 0);
	nvram_set_int("vpnc_state_t", 0);

	/* Setup wan0 variables if necessary */
	set_wan0_vars();
}

static void
flash_firmware(void)
{
	char* svcs[] = { "watchdog", 
			 "l2tpd",
			 "xl2tpd",
			 "pppd",
			 "wpa_cli",
			 "wpa_supplicant",
			 NULL };

	kill_services(svcs, 6, 1);

	/* save storage (if changed) */
	write_storage_to_mtd();

	system("cp -f /bin/mtd_write /tmp");

	if (eval("/tmp/mtd_write", "-r", "write", FW_IMG_NAME, FW_MTD_NAME) != 0) {
		nvram_set("reboot", "");
		start_watchdog();
	}
}

static void
load_usb_modem_modules(void)
{
	if (nvram_get_int("modem_rule") > 0)
		reload_modem_modules(nvram_get_int("modem_type"), 0);
}

static void
load_usb_printer_module(void)
{
	system("modprobe -q usblp");
}

static void
load_usb_storage_module(void)
{
	system("modprobe -q usb-storage");
}

void 
setenv_tz(void)
{
	static char TZ_env[64];

	snprintf(TZ_env, sizeof(TZ_env), "TZ=%s", nvram_safe_get("time_zone_x"));
	TZ_env[sizeof(TZ_env)-1] = '\0';
	putenv(TZ_env);
}

void 
setkernel_tz(void)
{
	time_t now;
	struct tm gm, local;
	struct timezone tz;
	struct timeval *tvp = NULL;

	/* Update kernel timezone */
	time(&now);
	gmtime_r(&now, &gm);
	localtime_r(&now, &local);
	
	gm.tm_isdst = local.tm_isdst;
	tz.tz_minuteswest = (mktime(&gm) - mktime(&local)) / 60;
	settimeofday(tvp, &tz);
}

void 
LED_CONTROL(int led, int flag)
{
	int i_front_leds = nvram_get_int("front_leds");
	switch (i_front_leds)
	{
	case 1:
		if ((led != LED_POWER)
#if defined(LED_ALL)
		 && (led != LED_ALL)
#endif
		   )
			flag = LED_OFF;
		break;
	case 2:
#if defined(LED_ALL)
		if (led != LED_ALL)
#endif
			flag = LED_OFF;
		break;
#if defined(LED_ALL)
	case 3:
		if (led != LED_POWER)
			flag = LED_OFF;
		break;
	case 4:
		flag = LED_OFF;
		break;
#endif
	}
	
	cpu_gpio_set_pin(led, flag);
}

void 
init_router(void)
{
	int log_remote;
	
	nvram_restore_defaults();
	
	getsyspara();
	
	init_router_mode();
	convert_misc_values(); //  convert_misc_values must be run first!!! (wanx_... cleared)
	convert_asus_values(0);
	
	gen_ralink_config_wl(0);
	gen_ralink_config_rt(0);
	insertmodules();
	
	init_gpio_leds_buttons();
	
	recreate_passwd_unix(1);
	
	set_timezone();
	set_pagecache_reclaim();
	
	log_remote = nvram_invmatch("log_ipaddr", "");
	if (!log_remote)
		start_logger(1);
	
	init_loopback();
	init_bridge();
#if defined (USE_IPV6)
	init_ipv6();
#endif
	start_detect_link();
	start_lan();
	start_dns_dhcpd();
	load_usb_printer_module();
	
	if (log_remote)
		start_logger(1);
	
	ipt_filter_default();
	ipt_nat_default();
#if defined (USE_IPV6)
	ip6t_filter_default();
#endif
	start_wan();
	load_usb_storage_module();
	load_usb_modem_modules();
	start_services();
	
	// system ready
	system("/etc/storage/started_script.sh &");
}

void 
shutdown_router(void)
{
	stop_misc(1);
	stop_services(1);
	
	stop_usb();
#if defined(LED_USB)
	LED_CONTROL(LED_USB, LED_OFF);
#endif
	
	stop_wan();
	stop_services_lan_wan();
#if defined(LED_WAN)
	LED_CONTROL(LED_WAN, LED_OFF);
#endif
	
	write_storage_to_mtd();
	
	stop_wifi_all_wl();
	stop_wifi_all_rt();
	stop_logger();
	stop_lan();

#if defined(LED_LAN)
	LED_CONTROL(LED_LAN, LED_OFF);
#endif
	LED_CONTROL(LED_POWER, LED_OFF);
}

void 
handle_notifications(void)
{
	int i, stop_handle = 0;
	char notify_name[256];
	DIR *directory = opendir("/tmp/rc_notification");
	if (!directory)
		return;
	
	// handle max 10 requests at once (prevent deadlock)
	for (i=0; i < 10; i++)
	{
		struct dirent *entry;
		FILE *test_fp;
		
		entry = readdir(directory);
		if (!entry)
			break;
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;
		
		/* Remove the marker file. */
		snprintf(notify_name, sizeof(notify_name), "/tmp/rc_notification/%s", entry->d_name);
		remove(notify_name);
		
		printf("rc notification: %s\n", entry->d_name);
		
		/* Take the appropriate action. */
		if (!strcmp(entry->d_name, "restart_reboot"))
		{
			stop_handle = 1;
			sys_exit();
		}
		else if (!strcmp(entry->d_name, "shutdown_prepare"))
		{
			stop_handle = 1;
			shutdown_router();
		}
		else if (!strcmp(entry->d_name, "flash_firmware"))
		{
			stop_handle = 1;
			flash_firmware();
		}
#if defined (USE_IPV6)
		else if (!strcmp(entry->d_name, "restart_ipv6"))
		{
			full_restart_ipv6(nvram_ipv6_type);
			nvram_ipv6_type = get_ipv6_type();
		}
#endif
		else if (!strcmp(entry->d_name, "restart_modem"))
		{
			int modules_reloaded = 0;
			int need_restart_wan = 0;
			int modem_rule = nvram_get_int("modem_rule");
			int modem_type = nvram_get_int("modem_type");
			if (nvram_modem_rule != modem_rule)
			{
				nvram_modem_rule = modem_rule;
				need_restart_wan = 1;
				stop_wan();
				if (modem_rule > 0) {
					modules_reloaded = 1;
					reload_modem_modules(modem_type, 1);
				} else {
					unload_modem_modules();
				}
			}
			if (nvram_modem_type != modem_type)
			{
				if (nvram_modem_type == 3 || modem_type == 3) {
					if (modem_rule > 0 && !modules_reloaded) {
						stop_wan();
						reload_modem_modules(modem_type, 1);
					}
				}
				nvram_modem_type = modem_type;
				need_restart_wan = 1;
			}
			if (need_restart_wan)
				full_restart_wan();
		}
		else if (!strcmp(entry->d_name, "restart_whole_wan"))
		{
			full_restart_wan();
		}
		else if (!strcmp(entry->d_name, "restart_whole_lan"))
		{
			full_restart_lan();
		}
		else if (!strcmp(entry->d_name, "stop_whole_wan"))
		{
			stop_wan();
		}
		else if (!strcmp(entry->d_name, "restart_iptv"))
		{
			restart_iptv();
			restart_firewall();
		}
		else if(!strcmp(entry->d_name, "manual_wan_connect"))
		{
			manual_wan_connect();
		}
		else if(!strcmp(entry->d_name, "manual_wan_disconnect"))
		{
			manual_wan_disconnect();
		}
		else if(!strcmp(entry->d_name, "manual_ddns_hostname_check"))
		{
			manual_ddns_hostname_check();
		}
		else if (strcmp(entry->d_name, "restart_cifs") == 0)
		{
#if defined(APP_FTPD)
			int is_run_before = is_ftp_run();
#endif
#if defined(APP_SMBD)
			stop_samba();
#endif
#if defined(APP_FTPD)
			stop_ftp();
#endif
			if (count_sddev_mountpoint()) {
#if defined(APP_SMBD)
				run_samba();
#endif
#if defined(APP_FTPD)
				run_ftp();
				control_ftp_fw(is_run_before);
#endif
			}
		}
#if defined(APP_NFSD)
		else if (strcmp(entry->d_name, "restart_nfs") == 0)
		{
			stop_nfsd();
			if (count_sddev_mountpoint()) {
				sleep(1);
				run_nfsd();
			}
		}
#endif
#if defined(APP_MINIDLNA)
		else if (strcmp(entry->d_name, "restart_dms") == 0)
		{
			restart_dms();
		}
#endif
#if defined(APP_FIREFLY)
		else if (strcmp(entry->d_name, "restart_itunes") == 0)
		{
			restart_itunes();
		}
#endif
#if defined(APP_TRMD)
		else if (strcmp(entry->d_name, "restart_torrent") == 0)
		{
			restart_torrent();
		}
#endif
#if defined(APP_ARIA)
		else if (strcmp(entry->d_name, "restart_aria") == 0)
		{
			restart_aria();
		}
#endif
		else if (strcmp(entry->d_name, "restart_term") == 0)
		{
			restart_term();
		}
		else if (strcmp(entry->d_name, "restart_vpn_server") == 0)
		{
			restart_vpn_server();
		}
		else if (strcmp(entry->d_name, "restart_vpn_client") == 0)
		{
			restart_vpn_client();
		}
		else if (strcmp(entry->d_name, "start_vpn_client") == 0)
		{
			start_vpn_client();
		}
		else if (strcmp(entry->d_name, "stop_vpn_client") == 0)
		{
			stop_vpn_client();
		}
		else if (strcmp(entry->d_name, "restart_ddns") == 0)
		{
			stop_ddns();
			start_ddns();
		}
		else if (strcmp(entry->d_name, "restart_httpd") == 0)
		{
			restart_httpd();
		}
		else if (strcmp(entry->d_name, "restart_dns") == 0)
		{
			restart_dns();
		}
		else if (strcmp(entry->d_name, "restart_dhcpd") == 0)
		{
			restart_dhcpd();
#if defined (USE_IPV6)
			restart_radvd();
#endif
		}
		else if (strcmp(entry->d_name, "restart_upnp") == 0)
		{
			int is_run_before = is_upnp_run();
			
			stop_upnp();
			start_upnp();
			
			if (is_upnp_run() != is_run_before)
				restart_firewall();
		}
		else if (strcmp(entry->d_name, "restart_switch_config") == 0)
		{
			switch_config_base();
			switch_config_storm();
			switch_config_link();
		}
		else if (strcmp(entry->d_name, "restart_switch_vlan") == 0)
		{
			reset_detect_link();
			switch_config_vlan(0);
		}
		else if (strcmp(entry->d_name, "restart_syslog") == 0)
		{
			stop_logger();
			start_logger(0);
		}
		else if (strcmp(entry->d_name, "restart_firewall_wan") == 0)
		{
			restart_firewall();
		}
		else if (strcmp(entry->d_name, "restart_firewall") == 0)
		{
			reload_nat_modules();
			restart_firewall();
			
			/* flush conntrack after NAT model changing */
			int nf_nat_type = nvram_get_int("nf_nat_type");
			if (nvram_nf_nat_type != nf_nat_type)
			{
				nvram_nf_nat_type = nf_nat_type;
				flush_conntrack_caches();
			}
		}
		else if (strcmp(entry->d_name, "restart_ntpc") == 0)
		{
			notify_watchdog_time();
		}
		else if (strcmp(entry->d_name, "restart_time") == 0)
		{
			stop_logger();
			set_timezone();
			notify_watchdog_time();
			notify_rstats_time();
			start_logger(0);
		}
		else if (strcmp(entry->d_name, "restart_spooler") == 0)
		{
			restart_usb_printer_spoolers();
		}
		else if (strcmp(entry->d_name, "restart_hddtune") == 0)
		{
#if defined(BOARD_N65U)
			set_pcie_aspm();
			sleep(1);
#endif
			system("/sbin/hddtune.sh");
		}
		else if (strcmp(entry->d_name, "restart_sysctl") == 0)
		{
			restart_all_sysctl();
		}
		else if (!strcmp(entry->d_name, "restart_wifi_wl"))
		{
			int radio_on = get_enabled_radio_wl();
			if (radio_on)
				radio_on = is_radio_allowed_wl();
			restart_wifi_wl(radio_on, 1);
		}
		else if (!strcmp(entry->d_name, "restart_wifi_rt"))
		{
			int radio_on = get_enabled_radio_rt();
			if (radio_on)
				radio_on = is_radio_allowed_rt();
			restart_wifi_rt(radio_on, 1);
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_wl"))
		{
			int guest_on = is_guest_allowed_wl();
			control_guest_wl(guest_on, 1);
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_rt"))
		{
			int guest_on = is_guest_allowed_rt();
			control_guest_rt(guest_on, 1);
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_wl_on"))
		{
			control_guest_wl(1, 0);
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_wl_off"))
		{
			control_guest_wl(0, 0);
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_rt_on"))
		{
			control_guest_rt(1, 0);
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_rt_off"))
		{
			control_guest_rt(0, 0);
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_wl"))
		{
			int radio_on = get_enabled_radio_wl();
			if (radio_on)
				radio_on = is_radio_allowed_wl();
			control_radio_wl(radio_on, 1);
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_rt"))
		{
			int radio_on = get_enabled_radio_rt();
			if (radio_on)
				radio_on = is_radio_allowed_rt();
			control_radio_rt(radio_on, 1);
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_wl_on"))
		{
			control_radio_wl(1, 0);
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_wl_off"))
		{
			control_radio_wl(0, 0);
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_rt_on"))
		{
			control_radio_rt(1, 0);
		}
		else if (!strcmp(entry->d_name, "control_wifi_radio_rt_off"))
		{
			control_radio_rt(0, 0);
		}
		else if (!strcmp(entry->d_name, "on_hotplug_usb_storage"))
		{
			// deferred run usb apps
			nvram_set_int("usb_hotplug_ms", 1);
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_unplug_usb_storage"))
		{
			umount_ejected();
		}
		else if (!strcmp(entry->d_name, "on_hotplug_usb_printer"))
		{
			// deferred run usb printer daemons
			nvram_set_int("usb_hotplug_lp", 1);
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_unplug_usb_printer"))
		{
			// deferred stop usb printer daemons
			nvram_set_int("usb_unplug_lp", 1);
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_hotplug_usb_modem"))
		{
			// deferred run usb modem to wan
			nvram_set_int("usb_hotplug_md", 1);
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_unplug_usb_modem"))
		{
			// deferred restart wan
			nvram_set_int("usb_unplug_md", 1);
			alarm(5);
		}
		else
		{
			dbg("WARNING: rc notified of unrecognized event `%s'.\n", entry->d_name);
		}
		
		/*
		 * If there hasn't been another request for the same event made since
		 * we started, we can safely remove the ``action incomplete'' marker.
		 * Otherwise, we leave the marker because we'll go through here again
		 * for this even and mark it complete only after we've completed it
		 * without getting another request for the same event while handling
		 * it.
		 */
		test_fp = fopen(notify_name, "r");
		if (test_fp != NULL)
		{
			fclose(test_fp);
		}
		else
		{
			/* Remove the marker file. */
			snprintf(notify_name, sizeof(notify_name), "/tmp/rc_action_incomplete/%s", entry->d_name);
			remove(notify_name);
		}
		
		if (stop_handle)
			break;
	}
	
	closedir(directory);
}


typedef struct {
	const char *name;
	int (*main)(int argc, char *argv[]);
} applet_rc_t;


static const applet_rc_t applets_rc[] = {
	{ "udhcpc.script",	udhcpc_main		},
	{ "udhcpc_lan.script",	udhcpc_lan_main		},
	{ "udhcpc_viptv.script",udhcpc_viptv_main	},
	{ "zcip.script",	zcip_main		},
	{ "zcip_viptv.script",	zcip_viptv_main		},
	{ "wpacli.script",	wpacli_main		},
	{ "ip-up",		ipup_main		},
	{ "ip-down",		ipdown_main		},
#if defined(USE_IPV6)
	{ "ipv6-up",		ipv6up_main		},
	{ "ipv6-down",		ipv6down_main		},
	{ "dhcp6c.script",	dhcp6c_main		},
#endif
	{ "ip-up.vpns",		ipup_vpns_main		},
	{ "ip-down.vpns",	ipdown_vpns_main	},
	{ "ip-up.vpnc",		ipup_vpnc_main		},
	{ "ip-down.vpnc",	ipdown_vpnc_main	},

#if defined(APP_OPENVPN)
	{ SCRIPT_OPENVPN,	openvpn_script_main	},
#endif
	{ "mdev_sg",		mdev_sg_main		},
	{ "mdev_sd",		mdev_sd_main		},
	{ "mdev_sr",		mdev_sr_main		},
	{ "mdev_lp",		mdev_lp_main		},
	{ "mdev_tty",		mdev_tty_main		},
	{ "mdev_wdm",		mdev_wdm_main		},
	{ "mdev_net",		mdev_net_main		},

	{ "ddns_updated",	ddns_updated_main	},

	{ "detect_wan",		detect_wan_main		},
	{ "detect_link",	detect_link_main	},
	{ "detect_internet",	detect_internet_main	},

	{ "watchdog",		watchdog_main		},
	{ "rstats",		rstats_main		},
	{ "rtl8367",		rtl8367_main		},
#if defined(USE_RT3352_MII)
	{ "inicd",		inicd_main		},
#endif
	{ "stopservice",	stop_service_main	},

	{ NULL, NULL }
};


int
main(int argc, char **argv)
{
	int ret;
	char *base = strrchr(argv[0], '/');
	const applet_rc_t *app;
	base = base ? base + 1 : argv[0];

	/* init */
	if (!strcmp(base, "init")) {
		init_main_loop();
		return 0;
	}
	
	/* stub for early kernel hotplug */
	if (!strcmp(base, "hotplug")) {
		return 0;
	}
	
	if (!strcmp(base, "shutdown") || !strcmp(base, "halt")) {
		notify_rc("shutdown_prepare");
		return 0;
	}
	
	if (!strcmp(base, "reboot")) {
		return kill(1, SIGTERM);
	}
	
	if (!strcmp(base, "rc")) {
		dbg("error: cannot run rc directly!\n");
		return EINVAL;
	}
	
	/* Set TZ for all rc programs */
	setenv_tz();
	
	/* Start applets */
	for (app = applets_rc; app->name; app++) {
		if (strcmp(base, app->name) == 0)
			return app->main(argc, argv);
	}

	ret = 0;
	if (!strcmp(base, "reset_to_defaults")) {
		erase_nvram();
		sys_exit();
	}
	else if (!strcmp(base, "run_ftpsamba")) {
#if defined(APP_SMBD)
		stop_samba();
		run_samba();
#else
		;
#endif
#if defined(APP_FTPD)
		restart_ftp();
#endif
	}
#if defined(APP_SMBD)
	else if (!strcmp(base, "run_samba")) {
		stop_samba();
		run_samba();
	}
#endif
#if defined(APP_FTPD)
	else if (!strcmp(base, "run_ftp")) {
		restart_ftp();
	}
#endif
#if defined(APP_NFSD)
	else if (!strcmp(base, "run_nfsd")) {
		run_nfsd();
	}
#endif
#if defined(APP_MINIDLNA)
	else if (!strcmp(base, "run_minidlna")) {
		restart_dms();
	}
#endif
#if defined(APP_FIREFLY)
	else if (!strcmp(base, "run_firefly")) {
		restart_itunes();
	}
#endif
#if defined(APP_TRMD)
	else if (!strcmp(base, "run_transmission")) {
		restart_torrent();
	}
#endif
#if defined(APP_ARIA)
	else if (!strcmp(base, "run_aria")) {
		restart_aria();
	}
#endif
#if defined(APP_FTPD)
	else if (!strcmp(base, "stop_ftp")) {
		stop_ftp();
	}
#endif
#if defined(APP_SMBD)
	else if (!strcmp(base, "stop_samba")) {
		stop_samba();
	}
#endif
	else if (!strcmp(base, "stop_ftpsamba")) {
#if defined(APP_FTPD)
		stop_ftp();
#endif
#if defined(APP_SMBD)
		stop_samba();
#else
		;
#endif
	}
#if defined(APP_NFSD)
	else if (!strcmp(base, "stop_nfsd")) {
		stop_nfsd();
	}
#endif
#if defined(APP_MINIDLNA)
	else if (!strcmp(base, "stop_minidlna")) {
		stop_dms();
	}
#endif
#if defined(APP_FIREFLY)
	else if (!strcmp(base, "stop_firefly")) {
		stop_itunes();
	}
#endif
#if defined(APP_TRMD)
	else if (!strcmp(base, "stop_transmission")) {
		stop_torrent();
	}
#endif
#if defined(APP_ARIA)
	else if (!strcmp(base, "stop_aria")) {
		stop_aria();
	}
#endif
	else if (!strcmp(base, "start_ddns"))
		start_ddns();
	else if (!strcmp(base, "getCountryCode"))
		ret = getCountryCode();
	else if (!strcmp(base, "setCountryCode")) {
		if (argc == 2)
			ret = setCountryCode(argv[1]);
		else
			ret = EINVAL;
	}
	else if (!strcmp(base, "gen_ralink_config")) {
		gen_ralink_config_wl(0);
	}
	else if (!strcmp(base, "gen_ralink_config_rt")) {
		gen_ralink_config_rt(0);
	}
	else if (!strcmp(base, "restart_wan"))
	{
		notify_rc("manual_wan_connect");
	}
	else if (!strcmp(base, "restart_dns"))
	{
		restart_dns();
	}
	else if (!strcmp(base, "restart_dhcpd"))
	{
		restart_dhcpd();
	}
	else if (!strcmp(base, "restart_networkmap"))
	{
		restart_networkmap();
	}
	else if (!strcmp(base, "start_telnetd"))
	{
		start_telnetd();
	}
	else if (!strcmp(base, "run_telnetd"))
	{
		run_telnetd();
	}
	else if (!strcmp(base, "restart_firewall"))
	{
		restart_firewall();
	}
	else if (!strcmp(base, "radio2_toggle"))
	{
		manual_toggle_radio_rt(-1);
	}
	else if (!strcmp(base, "radio2_toggle_on"))
	{
		manual_toggle_radio_rt(1);
	}
	else if (!strcmp(base, "radio2_toggle_off"))
	{
		manual_toggle_radio_rt(0);
	}
	else if (!strcmp(base, "radio5_toggle"))
	{
		manual_toggle_radio_wl(-1);
	}
	else if (!strcmp(base, "radio5_toggle_on"))
	{
		manual_toggle_radio_wl(1);
	}
	else if (!strcmp(base, "radio5_toggle_off"))
	{
		manual_toggle_radio_wl(0);
	}
	else if (!strcmp(base, "radio2_enable"))
	{
		manual_forced_radio_rt(1);
	}
	else if (!strcmp(base, "radio2_disable"))
	{
		manual_forced_radio_rt(0);
	}
	else if (!strcmp(base, "radio5_enable"))
	{
		manual_forced_radio_wl(1);
	}
	else if (!strcmp(base, "radio5_disable"))
	{
		manual_forced_radio_wl(0);
	}
	else if (!strcmp(base, "ejusb"))
	{
		int port = 0;
		char *devn = NULL;
		if (argc > 1) {
			if (strncmp(argv[1], "sd", 2) == 0)
				devn = argv[1];
			else {
				port = atoi(argv[1]);
				if (argc > 2)
					devn = argv[2];
			}
		}
		ret = safe_remove_usb_device(port, devn);
	}
	else if (!strcmp(base, "ejusb1"))
	{
		char *devn = (argc > 1) ? argv[1] : NULL;
		ret = safe_remove_usb_device(1, devn);
	}
	else if (!strcmp(base, "ejusb2"))
	{
		char *devn = (argc > 1) ? argv[1] : NULL;
		ret = safe_remove_usb_device(2, devn);
	}
	else if (!strcmp(base, "pids"))
	{
		if (argc > 1)
			ret = pids_main(argv[1]);
		else
			ret = EINVAL;
	}
	else
	{
		printf("Unknown applet: %s\n", base);
		ret = EINVAL;
	}

	return ret;
}

