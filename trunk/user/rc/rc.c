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

#include <nvram/bcmnvram.h>

#include "rc.h"
#include "gpio_pins.h"
#include "switch.h"

extern struct nvram_pair router_defaults[];

/* static values */
static int nvram_modem_type = 0;
static int nvram_modem_rule = 0;
static int nvram_nf_nat_type = 0;
static int nvram_ipv6_type = 0;

static int
nvram_restore_defaults(void)
{
	struct nvram_pair *np;
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
	for (np = router_defaults; np->name; np++) {
		if (restore_defaults || !nvram_get(np->name)) {
			nvram_set(np->name, np->value);
		}
	}

	klogctl(8, NULL, nvram_get_int("console_loglevel"));

	/* load static values */
	nvram_modem_type = nvram_get_int("modem_type");
	nvram_modem_rule = nvram_get_int("modem_rule");
	nvram_nf_nat_type = nvram_get_int("nf_nat_type");
	nvram_ipv6_type = get_ipv6_type();

	return restore_defaults;
}

static void
load_wireless_modules(void)
{
#if defined (USE_RT2860V2_AP)
	doSystem("modprobe %s", "rt2860v2_ap");
#endif

#if defined (USE_RT3090_AP)
	doSystem("modprobe %s", "rt3090_ap");
#elif defined (USE_RT5392_AP)
	doSystem("modprobe %s", "rt5392_ap");
#endif

#if defined (USE_RT5592_AP)
	doSystem("modprobe %s", "rt5592_ap");
#elif defined (USE_RT3593_AP)
	doSystem("modprobe %s", "rt3593_ap");
#elif defined (USE_MT7610_AP)
	doSystem("modprobe %s", "mt7610_ap");
#endif
}

#if (BOARD_NUM_USB_PORTS > 0)
static void
load_usb_modules(void)
{
	/* load usb printer module before storage */
	doSystem("modprobe %s", "usblp");

	/* load usb storage module */
	doSystem("modprobe %s", "usb-storage");

	/* load usb modem modules */
	if (!get_ap_mode() && nvram_get_int("modem_rule") > 0)
		reload_modem_modules(nvram_get_int("modem_type"), 0);

	/* start usb host */
#if defined (USE_USB3)
	doSystem("modprobe %s %s=%d", "xhci-hcd", "usb3_disable", nvram_get_int("usb3_disable"));
#else
	doSystem("modprobe %s", "ehci-hcd");
	doSystem("modprobe %s", "ohci-hcd");
#endif
}
#endif

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
#if defined (BOARD_GPIO_LED_WAN)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_WAN, 1);
	LED_CONTROL(BOARD_GPIO_LED_WAN, LED_OFF);
#endif
#if defined (BOARD_GPIO_LED_LAN)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_LAN, 1);
	LED_CONTROL(BOARD_GPIO_LED_LAN, LED_OFF);
#endif
#if defined (BOARD_GPIO_LED_USB)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_USB, 1);
	LED_CONTROL(BOARD_GPIO_LED_USB, LED_OFF);
#endif
#if defined (BOARD_GPIO_LED_ALL)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_ALL, 1);
	LED_CONTROL(BOARD_GPIO_LED_ALL, LED_ON);
#endif
#if defined (BOARD_GPIO_LED_WIFI)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_WIFI, 1);
	LED_CONTROL(BOARD_GPIO_LED_WIFI, LED_ON);
#endif
#if defined (BOARD_GPIO_LED_POWER)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_POWER, 1);
	LED_CONTROL(BOARD_GPIO_LED_POWER, LED_ON);
#endif

#if defined (BOARD_GPIO_BTN_RESET)
	cpu_gpio_set_pin_direction(BOARD_GPIO_BTN_RESET, 0);
#endif
#if defined (BOARD_GPIO_BTN_WPS)
	cpu_gpio_set_pin_direction(BOARD_GPIO_BTN_WPS, 0);
#endif
#if defined (BOARD_GPIO_BTN_WLTOG)
	cpu_gpio_set_pin_direction(BOARD_GPIO_BTN_WLTOG, 0);
#endif
}

static void
set_wan0_vars(void)
{
	struct nvram_pair *np;
	char *v;
	char tmp[100], prefix[16];

	snprintf(prefix, sizeof(prefix), "wan%d_", 0);

	/* Write through to wan0_ variable set */
	for (np = router_defaults; np->name; np++) {
		if (!strncmp(np->name, "wan_", 4)) {
			if (nvram_get(strcat_r(prefix, &np->name[4], tmp)))
				continue;
			v = nvram_get(np->name);
			nvram_set_temp(tmp, v ? v : np->value);
		}
	}

	set_wan_unit_value(0, "unit", "0");
	set_wan_unit_value(0, "primary", "1");
	set_wan_unit_value(0, "ifname", IFNAME_WAN);
}

static void
nvram_convert_old_params(void)
{
	char *test_value;

	/* convert old params */
	test_value = nvram_get("front_leds");
	if (test_value) {
		int front_leds = atoi(test_value);
		if (front_leds > 2)
			nvram_set_int("front_led_all", 0);
		if (front_leds == 4 || front_leds == 2)
			nvram_set_int("front_led_pwr", 0);
		nvram_unset("front_leds");
	}

	test_value = nvram_get("pppoe_dhcp_route");
	if (test_value) {
		if (strlen(test_value) > 0 && strlen(nvram_safe_get("wan_pppoe_man")) == 0)
			nvram_set("wan_pppoe_man", test_value);
		nvram_unset("pppoe_dhcp_route");
	}

	test_value = nvram_get("wan_heartbeat_x");
	if (test_value) {
		if (strlen(test_value) > 0 && strlen(nvram_safe_get("wan_ppp_peer")) == 0)
			nvram_set("wan_ppp_peer", test_value);
		nvram_unset("wan_heartbeat_x");
	}
	nvram_unset("wan0_heartbeat_x");

	test_value = nvram_get("wan_3g_pin");
	if (test_value) {
		if (strlen(test_value) > 0 && strlen(nvram_safe_get("modem_pin")) == 0)
			nvram_set("modem_pin", test_value);
		nvram_unset("wan_3g_pin");
	}
	nvram_unset("wan0_3g_pin");

	/* remove old unused params */
	nvram_unset("lan_route");
	nvram_unset("wan0_route");
	nvram_unset("wan_route");
	nvram_unset("wan_dns_t");
	nvram_unset("wan_proto_t");
	nvram_unset("wan_ipaddr_t");
	nvram_unset("wan_netmask_t");
	nvram_unset("wan_gateway_t");
	nvram_unset("wan_ifname_t");
	nvram_unset("wan_status_t");
	nvram_unset("wan_subnet_t");
	nvram_unset("lan_subnet_t");
	nvram_unset("link_lan");
}

static void
nvram_convert_misc_values(void)
{
	char buff[64];
	int sw_mode;

	/* check router mode */
	sw_mode = nvram_get_int("sw_mode");
	if (sw_mode == 1) {
		/* Internet gateway mode */
		nvram_set_int("wan_nat_x", 1);
		nvram_set("wan_route_x", "IP_Routed");
	} else if (sw_mode == 4) {
		/* Pure router mode */
		nvram_set_int("wan_nat_x", 0);
		nvram_set("wan_route_x", "IP_Routed");
	} else if (sw_mode == 3) {
		/* AP mode (Ethernet convertor) */
		nvram_set_int("wan_nat_x", 0);
		nvram_set("wan_route_x", "IP_Bridged");
	} else {
		nvram_set_int("sw_mode", 1);
		nvram_set_int("wan_nat_x", 1);
		nvram_set("wan_route_x", "IP_Routed");
	}

	if (strlen(nvram_wlan_get("wl", "ssid")) < 1)
		nvram_wlan_set("wl", "ssid", DEF_WLAN_5G_SSID);

	if (strlen(nvram_wlan_get("rt", "ssid")) < 1)
		nvram_wlan_set("rt", "ssid", DEF_WLAN_5G_SSID);

	memset(buff, 0, sizeof(buff));
	char_to_ascii(buff, nvram_wlan_get("wl", "ssid"));
	nvram_wlan_set("wl", "ssid2", buff);

	memset(buff, 0, sizeof(buff));
	char_to_ascii(buff, nvram_wlan_get("rt", "ssid"));
	nvram_wlan_set("rt", "ssid2", buff);

	if (strlen(nvram_safe_get("wl_wpa_mode")) < 1)
		nvram_set_int("wl_wpa_mode", 0);

	if (strlen(nvram_safe_get("wl_gmode")) < 1)
		nvram_set_int("wl_gmode", 2); // a/n Mixed

	if (strlen(nvram_safe_get("rt_gmode")) < 1)
		nvram_set_int("rt_gmode", 2); // b/g/n Mixed

	if (nvram_get_int("wl_HT_BW") > 1)
		nvram_set_int("wl_HT_BW", 1);

	if (nvram_get_int("rt_HT_BW") > 1)
		nvram_set_int("rt_HT_BW", 1);

	if (nvram_get_int("wl_stream_tx") > BOARD_NUM_ANT_5G_TX)
		nvram_set_int("wl_stream_tx", BOARD_NUM_ANT_5G_TX);

	if (nvram_get_int("wl_stream_rx") > BOARD_NUM_ANT_5G_RX)
		nvram_set_int("wl_stream_rx", BOARD_NUM_ANT_5G_RX);

	if (nvram_get_int("rt_stream_tx") > BOARD_NUM_ANT_2G_TX)
		nvram_set_int("rt_stream_tx", BOARD_NUM_ANT_2G_TX);

	if (nvram_get_int("rt_stream_rx") > BOARD_NUM_ANT_2G_RX)
		nvram_set_int("rt_stream_rx", BOARD_NUM_ANT_2G_RX);

	nvram_set_temp("ntpc_counter", "0000000000");
	nvram_set_temp("login_timestamp", "0000000000");

	nvram_set_int_temp("networkmap_fullscan", 0);
	nvram_set_int_temp("link_internet", 2);
	nvram_set_int_temp("link_wan", 0);

	nvram_set_int_temp("reload_svc_wl", 0);
	nvram_set_int_temp("reload_svc_rt", 0);

	nvram_set_int_temp("usb_hotplug_ms", 0);
	nvram_set_int_temp("usb_hotplug_lp", 0);
	nvram_set_int_temp("usb_hotplug_md", 0);
	nvram_set_int_temp("usb_unplug_lp", 0);
	nvram_set_int_temp("usb_unplug_md", 0);
	nvram_set_int_temp("usb_opt_start", 0);

	nvram_set_int_temp("l2tp_wan_t", 0);
	nvram_set_int_temp("l2tp_cli_t", 0);
	nvram_set_int_temp("l2tp_srv_t", 0);
	nvram_set_int_temp("vpnc_state_t", 0);
	nvram_set_temp("vpnc_dns_t", "");
	nvram_set_temp("viptv_ifname", "");

	/* setup wan0 variables */
	set_wan0_vars();
	set_wan_unit_value(0, "uptime", "0000000000");
	set_wan_unit_value(0, "dltime", "0000000000");
	set_wan_unit_value(0, "bytes_rx", "00000000000000000000");
	set_wan_unit_value(0, "bytes_tx", "00000000000000000000");
	set_wan_unit_value(0, "ifname_t", "");
	set_usb_modem_dev_wan(0, 0);
	reset_wan_vars();

	/* setup lan variables */
	reset_lan_vars();
	reset_lan_temp();

	time_zone_x_mapping();
}

static void
write_storage_to_mtd(void)
{
	doSystem("/sbin/mtd_storage.sh %s", "save");
}

void
erase_storage(void)
{
	doSystem("/sbin/mtd_storage.sh %s", "erase");
}

void
erase_nvram(void)
{
	doSystem("/bin/mtd_write %s %s", "erase", "Config");
}

static void
flash_firmware(void)
{
	char* svcs[] = { "l2tpd",
			 "xl2tpd",
			 "pppd",
			 "wpa_cli",
			 "wpa_supplicant",
			 NULL };

	stop_misc();
	stop_services(0); // don't stop httpd/telnetd/sshd/vpn
#if (BOARD_NUM_USB_PORTS > 0)
	stop_usb();
#endif
	stop_igmpproxy("");

	kill_services(svcs, 6, 1);

	/* save storage (if changed) */
	storage_save_time(60);
	write_storage_to_mtd();

	sync();

	if (eval("/tmp/mtd_write", "-r", "write", FW_IMG_NAME, FW_MTD_NAME) != 0) {
		start_watchdog();
	}
}

static void
storage_load_time(void)
{
	FILE *fp;
	char buf[32];
	struct tm storage_tm;
	time_t storage_time = 0;

	if (nvram_match("stime_stored", "0"))
		return;

	fp = fopen("/etc/storage/system_time", "r");
	if (fp) {
		if (fgets(buf, sizeof(buf), fp))
			storage_time = (time_t)strtoul(buf, NULL, 0);
		fclose(fp);
	}

	if (storage_time > 0) {
		localtime_r(&storage_time, &storage_tm);
		if (storage_tm.tm_year > (SYS_START_YEAR - 1900)) {
			storage_time = mktime(&storage_tm);
			storage_time += 5; // add delta 5 sec (~boot time)
			stime(&storage_time);
		}
	}
}

void
storage_save_time(time_t delta)
{
	FILE *fp;
	time_t now_time;
	struct tm now_tm;

	if (nvram_match("stime_stored", "0"))
		return;

	time(&now_time);
	localtime_r(&now_time, &now_tm);
	if (now_tm.tm_year > (SYS_START_YEAR - 1900)) {
		fp = fopen("/etc/storage/system_time", "w");
		if (fp) {
			fprintf(fp, "%lu", (now_time + delta));
			fclose(fp);
		}
	}
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
	static int tz_minuteswest_last = -1;

	/* Update kernel timezone */
	time(&now);
	gmtime_r(&now, &gm);
	localtime_r(&now, &local);

	gm.tm_isdst = local.tm_isdst;
	tz.tz_minuteswest = (mktime(&gm) - mktime(&local)) / 60;

	if (tz_minuteswest_last == tz.tz_minuteswest)
		return;

	tz_minuteswest_last = tz.tz_minuteswest;

	settimeofday(tvp, &tz);
}

void 
LED_CONTROL(int gpio_led, int flag)
{
	int front_led_x = 1;

	switch (gpio_led)
	{
#if defined (BOARD_GPIO_LED_WAN)
	case BOARD_GPIO_LED_WAN:
		front_led_x = nvram_get_int("front_led_wan");
		break;
#endif
#if defined (BOARD_GPIO_LED_LAN)
	case BOARD_GPIO_LED_LAN:
		front_led_x = nvram_get_int("front_led_lan");
		break;
#endif
#if defined (BOARD_GPIO_LED_WIFI)
	case BOARD_GPIO_LED_WIFI:
		front_led_x = nvram_get_int("front_led_wif");
		break;
#endif
#if defined (BOARD_GPIO_LED_USB) && (BOARD_NUM_USB_PORTS > 0)
	case BOARD_GPIO_LED_USB:
		front_led_x = nvram_get_int("front_led_usb");
		break;
#endif
#if defined (BOARD_GPIO_LED_POWER)
	case BOARD_GPIO_LED_POWER:
		front_led_x = nvram_get_int("front_led_pwr");
		break;
#endif
#if defined (BOARD_GPIO_LED_ALL)
	case BOARD_GPIO_LED_ALL:
		front_led_x = nvram_get_int("front_led_all");
		break;
#endif
	default:
		return;
	}

	if (front_led_x == 0)
		flag = LED_OFF;

#if !defined (BOARD_GPIO_LED_ALL)
	if (gpio_led != BOARD_GPIO_LED_POWER && nvram_get_int("front_led_all") == 0)
		flag = LED_OFF;
#endif

#if defined (BOARD_GPIO_LED_WIFI) && defined (CONFIG_RALINK_MT7620)
	if (gpio_led == BOARD_GPIO_LED_WIFI) {
		cpu_gpio_mode_set_bit(13, (flag == LED_OFF) ? 1 : 0); // change GPIO Mode for WLED
		cpu_gpio_set_pin(gpio_led, LED_OFF); // always set GPIO to high
	}
	else
#endif
		cpu_gpio_set_pin(gpio_led, flag);
}

void 
init_router(void)
{
	int log_remote, is_ap_mode, nvram_need_commit;

#if defined (USE_RTL8367)
	rtl8367_node();
#endif
#if defined (USE_MTK_ESW)
	mtk_esw_node();
#endif

	nvram_convert_old_params();

	nvram_need_commit = nvram_restore_defaults();

	get_eeprom_params();

	nvram_convert_misc_values();

	if (nvram_need_commit)
		nvram_commit();

	init_gpio_leds_buttons();

	gen_ralink_config_2g(0);
	gen_ralink_config_5g(0);
	load_wireless_modules();
#if (BOARD_NUM_USB_PORTS > 0)
	load_usb_modules();
#endif
	recreate_passwd_unix(1);

	set_timezone();
	set_pagecache_reclaim();

	storage_load_time();

	is_ap_mode = get_ap_mode();

	log_remote = nvram_invmatch("log_ipaddr", "");
	if (!log_remote)
		start_logger(1);

	init_loopback();
	init_bridge();
#if defined (USE_IPV6)
	init_ipv6();
#endif
	start_detect_link();
	start_detect_internet(0);
	start_lan(is_ap_mode);

	if (log_remote)
		start_logger(1);

	start_dns_dhcpd(is_ap_mode);

	if (!is_ap_mode) {
		ipt_nat_default();
		ipt_filter_default();
#if defined (USE_IPV6)
		ip6t_filter_default();
#endif
		start_wan();
	}

	start_services_once(is_ap_mode);

	notify_leds_detect_link();

	// system ready
	system("/etc/storage/started_script.sh &");
}

void 
shutdown_router(void)
{
	int is_ap_mode = get_ap_mode();

	stop_misc();
	stop_services(1);

#if (BOARD_NUM_USB_PORTS > 0)
	stop_usb();
#endif
#if defined (BOARD_GPIO_LED_USB)
	LED_CONTROL(BOARD_GPIO_LED_USB, LED_OFF);
#endif

	stop_wan();
	stop_services_lan_wan();
	set_ipv4_forward(0);
#if defined (BOARD_GPIO_LED_WAN)
	LED_CONTROL(BOARD_GPIO_LED_WAN, LED_OFF);
#endif

	storage_save_time(10);
	write_storage_to_mtd();

	stop_8021x_all();
	stop_wifi_all_wl();
	stop_wifi_all_rt();
	stop_logger();
	stop_lan(is_ap_mode);

#if defined (BOARD_GPIO_LED_LAN)
	LED_CONTROL(BOARD_GPIO_LED_LAN, LED_OFF);
#endif
#if defined (BOARD_GPIO_LED_POWER)
	LED_CONTROL(BOARD_GPIO_LED_POWER, LED_OFF);
#endif

	module_smart_unload("rt_timer_wdg", 0);
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
		else if (!strcmp(entry->d_name, "flash_firmware"))
		{
			stop_handle = 1;
			flash_firmware();
		}
#if defined (USE_IPV6)
		else if (!strcmp(entry->d_name, "restart_ipv6"))
		{
			if (!get_ap_mode()) {
				full_restart_ipv6(nvram_ipv6_type);
				nvram_ipv6_type = get_ipv6_type();
			}
		}
		else if (strcmp(entry->d_name, "restart_radvd") == 0)
		{
			restart_dhcpd();
			restart_radvd();
		}
#endif
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
		else if(!strcmp(entry->d_name, "auto_wan_reconnect"))
		{
			auto_wan_reconnect();
		}
		else if(!strcmp(entry->d_name, "manual_wan_reconnect"))
		{
			manual_wan_reconnect();
		}
		else if(!strcmp(entry->d_name, "manual_wan_disconnect"))
		{
			manual_wan_disconnect();
		}
		else if(!strcmp(entry->d_name, "manual_ddns_hostname_check"))
		{
			manual_ddns_hostname_check();
		}
#if (BOARD_NUM_USB_PORTS > 0)
		else if (!strcmp(entry->d_name, "restart_modem"))
		{
			int wan_stopped = 0;
			int modules_reloaded = 0;
			int need_restart_wan = get_usb_modem_wan(0);
			int modem_rule = nvram_get_int("modem_rule");
			int modem_type = nvram_get_int("modem_type");
			if (nvram_modem_rule != modem_rule)
			{
				nvram_modem_rule = modem_rule;
				if (need_restart_wan) {
					wan_stopped = 1;
					stop_wan();
				}
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
						if (need_restart_wan && !wan_stopped)
							stop_wan();
						reload_modem_modules(modem_type, 1);
					}
				}
				nvram_modem_type = modem_type;
			}
			if (need_restart_wan)
				full_restart_wan();
		}
		else if (strcmp(entry->d_name, "restart_spooler") == 0)
		{
			restart_usb_printer_spoolers();
		}
		else if (strcmp(entry->d_name, "restart_hddtune") == 0)
		{
			system("/sbin/hddtune.sh");
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
		else if (strcmp(entry->d_name, "restart_dms_rescan") == 0)
		{
			restart_dms(1);
		}
		else if (strcmp(entry->d_name, "restart_dms") == 0)
		{
			restart_dms(0);
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
		else if (!strcmp(entry->d_name, "on_hotplug_usb_storage"))
		{
			// deferred run usb apps
			nvram_set_int_temp("usb_hotplug_ms", 1);
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_unplug_usb_storage"))
		{
			umount_ejected();
		}
		else if (!strcmp(entry->d_name, "on_hotplug_usb_printer"))
		{
			// deferred run usb printer daemons
			nvram_set_int_temp("usb_hotplug_lp", 1);
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_unplug_usb_printer"))
		{
			// deferred stop usb printer daemons
			nvram_set_int_temp("usb_unplug_lp", 1);
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_hotplug_usb_modem"))
		{
			// deferred run usb modem to wan
			nvram_set_int_temp("usb_hotplug_md", 1);
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_unplug_usb_modem"))
		{
			// deferred restart wan
			nvram_set_int_temp("usb_unplug_md", 1);
			alarm(5);
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
			start_ddns(1);
		}
		else if (strcmp(entry->d_name, "restart_httpd") == 0)
		{
			restart_httpd();
		}
		else if (strcmp(entry->d_name, "restart_di") == 0)
		{
			if (get_ap_mode() || has_wan_ip4(0))
				notify_run_detect_internet(2);
		}
		else if (strcmp(entry->d_name, "restart_dhcpd") == 0)
		{
			if (get_ap_mode())
				update_hosts_ap();
			restart_dhcpd();
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
			config_bridge();
			switch_config_base();
			switch_config_storm();
			switch_config_link();
		}
		else if (strcmp(entry->d_name, "restart_switch_vlan") == 0)
		{
			notify_reset_detect_link();
			switch_config_vlan(0);
		}
		else if (strcmp(entry->d_name, "restart_syslog") == 0)
		{
			stop_logger();
			start_logger(0);
		}
		else if (strcmp(entry->d_name, "restart_tweaks") == 0)
		{
			restart_watchdog_cpu();
			notify_leds_detect_link();
		}
		else if (strcmp(entry->d_name, "restart_firewall_wan") == 0)
		{
			restart_firewall();
		}
		else if (strcmp(entry->d_name, "restart_firewall") == 0)
		{
			reload_nat_modules();
			restart_firewall();
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
		else if (strcmp(entry->d_name, "restart_sysctl") == 0)
		{
			int nf_nat_type = nvram_get_int("nf_nat_type");
			
			restart_all_sysctl();
			
			/* flush conntrack after NAT model changing */
			if (nvram_nf_nat_type != nf_nat_type)
			{
				nvram_nf_nat_type = nf_nat_type;
				flush_conntrack_caches();
			}
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
		else if (!strcmp(entry->d_name, "control_wifi_config_wl"))
		{
			gen_ralink_config_5g(0);
		}
		else if (!strcmp(entry->d_name, "control_wifi_config_rt"))
		{
			gen_ralink_config_2g(0);
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
	{ SCRIPT_OVPN_SERVER,	ovpn_server_script_main	},
	{ SCRIPT_OVPN_CLIENT,	ovpn_client_script_main	},
#endif
#if (BOARD_NUM_USB_PORTS > 0)
	{ "mdev_sg",		mdev_sg_main		},
	{ "mdev_sd",		mdev_sd_main		},
	{ "mdev_sr",		mdev_sr_main		},
	{ "mdev_lp",		mdev_lp_main		},
	{ "mdev_tty",		mdev_tty_main		},
	{ "mdev_wdm",		mdev_wdm_main		},
	{ "mdev_net",		mdev_net_main		},
	{ "zerocd",		zerocd_main		},
#endif
	{ "ddns_updated",	ddns_updated_main	},
	{ "ntpc_updated",	ntpc_updated_main	},

	{ "detect_wan",		detect_wan_main		},
	{ "detect_link",	detect_link_main	},
	{ "detect_internet",	detect_internet_main	},

	{ "watchdog",		watchdog_main		},
	{ "rstats",		rstats_main		},
#if defined(USE_RTL8367)
	{ "rtl8367",		rtl8367_main		},
#endif
#if defined(USE_MTK_ESW)
	{ "mtk_esw",		mtk_esw_main		},
#endif
#if defined(USE_RT3352_MII)
	{ "inicd",		inicd_main		},
#endif
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
		if (getpid() != 1 ) {
			dbg("error: %s must be run as PID 1!\n", base);
			return -1;
		}
		init_main_loop();
		return 0;
	}

	/* stub for early kernel hotplug */
	if (!strcmp(base, "hotplug")) {
		return 0;
	}

	if (!strcmp(base, "reboot")) {
		return sys_exit();
	}

	if (!strcmp(base, "shutdown") || !strcmp(base, "halt")) {
		return sys_stop();
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
		restart_dms(0);
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
	else if (!strcmp(base, "start_ddns")) {
		start_ddns(1);
	}
	else if (!strcmp(base, "restart_wan")) {
		notify_rc("manual_wan_reconnect");
	}
	else if (!strcmp(base, "restart_dns")) {
		restart_dns();
	}
	else if (!strcmp(base, "restart_dhcpd")) {
		restart_dhcpd();
	}
	else if (!strcmp(base, "restart_networkmap")) {
		restart_networkmap();
	}
	else if (!strcmp(base, "start_telnetd")) {
		start_telnetd();
	}
	else if (!strcmp(base, "run_telnetd")) {
		run_telnetd();
	}
	else if (!strcmp(base, "restart_firewall")) {
		restart_firewall();
	}
	else if (!strcmp(base, "radio2_toggle")) {
		manual_toggle_radio_rt(-1);
	}
	else if (!strcmp(base, "radio2_toggle_on")) {
		manual_toggle_radio_rt(1);
	}
	else if (!strcmp(base, "radio2_toggle_off")) {
		manual_toggle_radio_rt(0);
	}
	else if (!strcmp(base, "radio2_enable")) {
		manual_change_radio_rt(1);
	}
	else if (!strcmp(base, "radio2_disable")) {
		manual_change_radio_rt(0);
	}
	else if (!strcmp(base, "radio2_guest_enable")) {
		manual_change_guest_rt(1);
	}
	else if (!strcmp(base, "radio2_guest_disable")) {
		manual_change_guest_rt(0);
	}
	else if (!strcmp(base, "radio2_eeprom_mac")) {
		if (argc > 1 && strlen(argv[1]) == 17)
			ret = set_wireless_mac(0, argv[1]);
		else {
			printf("Usage: %s XX:XX:XX:XX:XX:XX\n\n", base);
			ret = get_wireless_mac(0);
		}
	}
	else if (!strcmp(base, "radio2_restart")) {
		int radio_on = get_enabled_radio_rt();
		if (radio_on)
			radio_on = is_radio_allowed_rt();
		restart_wifi_rt(radio_on, 1);
	}
#if BOARD_HAS_5G_RADIO
	else if (!strcmp(base, "radio5_toggle")) {
		manual_toggle_radio_wl(-1);
	}
	else if (!strcmp(base, "radio5_toggle_on")) {
		manual_toggle_radio_wl(1);
	}
	else if (!strcmp(base, "radio5_toggle_off")) {
		manual_toggle_radio_wl(0);
	}
	else if (!strcmp(base, "radio5_enable")) {
		manual_change_radio_wl(1);
	}
	else if (!strcmp(base, "radio5_disable")) {
		manual_change_radio_wl(0);
	}
	else if (!strcmp(base, "radio5_guest_enable")) {
		manual_change_guest_wl(1);
	}
	else if (!strcmp(base, "radio5_guest_disable")) {
		manual_change_guest_wl(0);
	}
	else if (!strcmp(base, "radio5_eeprom_mac")) {
		if (argc > 1 && strlen(argv[1]) == 17)
			ret = set_wireless_mac(1, argv[1]);
		else {
			printf("Usage: %s XX:XX:XX:XX:XX:XX\n\n", base);
			ret = get_wireless_mac(1);
		}
	}
	else if (!strcmp(base, "radio5_restart")) {
		int radio_on = get_enabled_radio_wl();
		if (radio_on)
			radio_on = is_radio_allowed_wl();
		restart_wifi_wl(radio_on, 1);
	}
#endif
#if (BOARD_NUM_USB_PORTS > 0)
	else if (!strcmp(base, "ejusb")) {
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
	else if (!strcmp(base, "ejusb1")) {
		char *devn = (argc > 1) ? argv[1] : NULL;
		ret = safe_remove_usb_device(1, devn);
	}
#if (BOARD_NUM_USB_PORTS > 1)
	else if (!strcmp(base, "ejusb2")) {
		char *devn = (argc > 1) ? argv[1] : NULL;
		ret = safe_remove_usb_device(2, devn);
	}
#endif
#endif
	else if (!strcmp(base, "pids")) {
		if (argc > 1)
			ret = pids_main(argv[1]);
		else
			ret = EINVAL;
	}
	else {
		printf("Unknown applet: %s\n", base);
		ret = EINVAL;
	}

	return ret;
}

