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

#include <rstats.h>
#if defined (USE_STORAGE)
#include <disk_initial.h>
#endif

#include "rc.h"
#include "gpio_pins.h"
#include "switch.h"
#include <ralink_priv.h>

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
	char tmp[32] = {0};
	unsigned char buffer[2] = {0};
	char lan_mac[] = "FFFF";

	int i_offset = get_wired_mac_e2p_offset(0) + 4;
	if (flash_mtd_read(MTD_PART_NAME_FACTORY, i_offset, buffer, 2) == 0) {
		sprintf(lan_mac, "%02X%02X", buffer[0] & 0xff, buffer[1] & 0xff);
	}

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
			if (strstr(np->name,"wl_ssid") || strstr(np->name,"rt_ssid") || !strcmp(np->name,"wl_guest_ssid") || !strcmp(np->name,"rt_guest_ssid")){
				sprintf(tmp, np->value, lan_mac);
				nvram_set(np->name, tmp);
			} else {
				nvram_set(np->name, np->value);
			}
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
	module_smart_load("rt2860v2_ap", NULL);
#endif

#if defined (USE_MT7628_AP)
	module_smart_load("mt7628_ap", NULL);
#endif

#if defined (USE_MT76X2_AP)
	module_smart_load("mt76x2_ap", NULL);
#endif

#if defined (USE_MT7615_AP)
	module_smart_load("mt_7615e", NULL);
#endif

#if defined (USE_RT3090_AP)
	module_smart_load("rt3090_ap", NULL);
#elif defined (USE_RT5392_AP)
	module_smart_load("rt5392_ap", NULL);
#elif defined (USE_MT76X3_AP)
	module_smart_load("mt76x3_ap", NULL);
#endif

#if defined (USE_RT5592_AP)
	module_smart_load("rt5592_ap", NULL);
#elif defined (USE_RT3593_AP)
	module_smart_load("rt3593_ap", NULL);
#elif defined (USE_MT7610_AP)
	module_smart_load("mt7610_ap", NULL);
#endif
}

#if defined (USE_USB_SUPPORT)
static void
load_usb_modules(void)
{
	/* load usb printer module before storage */
	module_smart_load("usblp", NULL);

	/* load usb storage module */
	module_smart_load("usb_storage", NULL);

	/* load usb modem modules */
	if (!get_ap_mode() && nvram_get_int("modem_rule") > 0)
		reload_modem_modules(nvram_get_int("modem_type"), 0);

	/* start usb host */
#if defined (USE_USB_XHCI)
	{
		char xhci_param[32];
		snprintf(xhci_param, sizeof(xhci_param), "%s=%d", "usb3_disable", nvram_get_int("usb3_disable"));
		module_smart_load("xhci_hcd", xhci_param);
	}
#else
	module_smart_load("ehci_hcd", NULL);
	module_smart_load("ohci_hcd", NULL);
#endif
}
#endif

#if defined (USE_ATA_SUPPORT)
static void
load_ata_modules(void)
{
	/* start ahci device */
	module_smart_load("ahci", NULL);
}
#endif

#if defined (USE_MMC_SUPPORT)
static void
load_mmc_modules(void)
{
	/* start mmc block device */
	module_smart_load("mmc_block", NULL);

	/* start mmc host */
#if defined (USE_MTK_MMC)
	module_smart_load("mtk_sd", NULL);
#endif
}
#endif

#if defined (USE_MTK_AES)
static void
load_crypto_modules(void)
{
	/* start aes engine */
	module_smart_load("mtk_aes", NULL);

#if 0
	/* start cryptodev-linux */
	module_smart_load("cryptodev", NULL);
#endif
}
#endif

#if defined (USE_IPSET)
static void
load_ipset_modules(void)
{
	module_smart_load("xt_set", NULL);
	module_smart_load("ip_set_hash_ip", NULL);
	module_smart_load("ip_set_hash_mac", NULL);
	module_smart_load("ip_set_hash_net", NULL);
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
	/* hide WiFi 2G soft-led  */
#if defined (BOARD_GPIO_LED_SW2G)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_SW2G, 1);
	cpu_gpio_set_pin(BOARD_GPIO_LED_SW2G, LED_OFF);
	cpu_gpio_led_set(BOARD_GPIO_LED_SW2G, LED_BLINK_STAY_SHOW);
#endif
	/* hide WiFi 5G soft-led  */
#if defined (BOARD_GPIO_LED_SW5G) && (!defined (BOARD_GPIO_LED_SW2G) || (BOARD_GPIO_LED_SW5G != BOARD_GPIO_LED_SW2G))
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_SW5G, 1);
	cpu_gpio_set_pin(BOARD_GPIO_LED_SW5G, LED_OFF);
	cpu_gpio_led_set(BOARD_GPIO_LED_SW5G, LED_BLINK_STAY_SHOW);
#endif
	/* hide WAN soft-led  */
#if defined (BOARD_GPIO_LED_WAN)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_WAN, 1);
	cpu_gpio_set_pin(BOARD_GPIO_LED_WAN, LED_OFF);
#endif
	/* hide LAN soft-led  */
#if defined (BOARD_GPIO_LED_LAN)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_LAN, 1);
	cpu_gpio_set_pin(BOARD_GPIO_LED_LAN, LED_OFF);
#endif
	/* hide USB soft-led  */
#if defined (BOARD_GPIO_LED_USB)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_USB, 1);
	cpu_gpio_set_pin(BOARD_GPIO_LED_USB, LED_OFF);
	cpu_gpio_led_set(BOARD_GPIO_LED_USB, LED_BLINK_STAY_HIDE);
#if defined (BOARD_GPIO_LED_USB2)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_USB2, 1);
	cpu_gpio_set_pin(BOARD_GPIO_LED_USB2, LED_OFF);
	cpu_gpio_led_set(BOARD_GPIO_LED_USB2, LED_BLINK_STAY_HIDE);
#endif
#endif
	/* hide ROUTER soft-led  */
#if defined (BOARD_GPIO_LED_ROUTER)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_ROUTER, 1);
	cpu_gpio_set_pin(BOARD_GPIO_LED_ROUTER, LED_OFF);
#endif
	/* enable common led trigger */
#if defined (BOARD_GPIO_LED_ALL)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_ALL, 1);
	LED_CONTROL(BOARD_GPIO_LED_ALL, LED_ON);
#endif
	/* allow WiFi hw-led  */
#if defined (BOARD_GPIO_LED_WIFI)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_WIFI, 1);
	LED_CONTROL(BOARD_GPIO_LED_WIFI, LED_ON);
#endif
	/* show PWR soft-led  */
#if defined (BOARD_GPIO_LED_POWER)
	cpu_gpio_set_pin_direction(BOARD_GPIO_LED_POWER, 1);
	LED_CONTROL(BOARD_GPIO_LED_POWER, LED_ON);
#endif

	/* enable USB port 5V power */
#if defined (BOARD_GPIO_PWR_USB)
	cpu_gpio_set_pin_direction(BOARD_GPIO_PWR_USB, 1);
	cpu_gpio_set_pin(BOARD_GPIO_PWR_USB, BOARD_GPIO_PWR_USB_ON);
#endif
#if defined (BOARD_GPIO_PWR_USB2)
	cpu_gpio_set_pin_direction(BOARD_GPIO_PWR_USB2, 1);
	cpu_gpio_set_pin(BOARD_GPIO_PWR_USB2, BOARD_GPIO_PWR_USB_ON);
#endif

	/* init BTN Reset  */
#if defined (BOARD_GPIO_BTN_RESET)
	cpu_gpio_set_pin_direction(BOARD_GPIO_BTN_RESET, 0);
#endif
	/* init BTN WPS  */
#if defined (BOARD_GPIO_BTN_WPS)
	cpu_gpio_set_pin_direction(BOARD_GPIO_BTN_WPS, 0);
#endif
	/* init BTN FN1  */
#if defined (BOARD_GPIO_BTN_FN1)
	cpu_gpio_set_pin_direction(BOARD_GPIO_BTN_FN1, 0);
#endif
	/* init BTN FN2  */
#if defined (BOARD_GPIO_BTN_FN2)
	cpu_gpio_set_pin_direction(BOARD_GPIO_BTN_FN2, 0);
#endif
	/* init BTN ROUTER  */
#if defined (BOARD_GPIO_BTN_ROUTER)
	cpu_gpio_set_pin_direction(BOARD_GPIO_BTN_ROUTER, 0);
#endif
	/* init BTN POWER  */
#if defined (BOARD_GPIO_BTN_PWR_CUT) && defined (BOARD_GPIO_BTN_PWR_INT)
	/* Shortcut POWER button */
	cpu_gpio_set_pin_direction(BOARD_GPIO_BTN_PWR_CUT, 1);
	cpu_gpio_set_pin(BOARD_GPIO_BTN_PWR_CUT, 1);

	/* IRQ on rising edge POWER, send SIGUSR2 to pid 1 */
	cpu_gpio_set_pin_direction(BOARD_GPIO_BTN_PWR_INT, 0);
	cpu_gpio_irq_set(BOARD_GPIO_BTN_PWR_INT, 1, 0, 1);
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

	set_wan_unit_value_int(0, "unit", 0);
	set_wan_unit_value_int(0, "primary", 1);
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

	test_value = nvram_get("mr_ttl_fix");
	if (test_value) {
		if (atoi(test_value) > 0 && strlen(nvram_safe_get("wan_ttl_fix")) == 0)
			nvram_set_int("wan_ttl_fix", 2);
		nvram_unset("mr_ttl_fix");
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
	nvram_unset("rt_mcastrate");
	nvram_unset("wl_mcastrate");
}

static void
nvram_convert_misc_values(void)
{
	char buff[64];
	int sw_mode;
#if defined (BOARD_GPIO_BTN_ROUTER)
	int i_router_switch = BTN_PRESSED;

	if (cpu_gpio_get_pin(BOARD_GPIO_BTN_ROUTER, &i_router_switch) < 0)
		i_router_switch = BTN_PRESSED;

	if (i_router_switch != BTN_PRESSED)
		nvram_set_int("sw_mode", 3);
	else if (nvram_get_int("sw_mode") == 3)
		nvram_set_int("sw_mode", 1);
#endif

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
		nvram_set("wan_route_x", "IP_Bridged");
	} else {
		sw_mode = 1;
		nvram_set_int("sw_mode", 1);
		nvram_set_int("wan_nat_x", 1);
		nvram_set("wan_route_x", "IP_Routed");
	}

#if defined (BOARD_GPIO_LED_ROUTER)
	if (sw_mode != 3)
		LED_CONTROL(BOARD_GPIO_LED_ROUTER, LED_ON);
#endif

#if BOARD_HAS_5G_RADIO
	if (strlen(nvram_wlan_get(1, "ssid")) < 1)
		nvram_wlan_set(1, "ssid", DEF_WLAN_5G_SSID);

	memset(buff, 0, sizeof(buff));
	char_to_ascii(buff, nvram_wlan_get(1, "ssid"));
	nvram_wlan_set(1, "ssid2", buff);

	if (strlen(nvram_wlan_get(1, "wpa_mode")) < 1)
		nvram_wlan_set_int(1, "wpa_mode", 0);

#if BOARD_HAS_5G_11AC
	if (strlen(nvram_wlan_get(1, "gmode")) < 1)
		nvram_wlan_set_int(1, "gmode", 4); // a/n/ac Mixed

	if (nvram_wlan_get_int(1, "HT_BW") > 3)
		nvram_wlan_set_int(1, "HT_BW", 3);
#else
	if (strlen(nvram_wlan_get(1, "gmode")) < 1)
		nvram_wlan_set_int(1, "gmode", 2); // a/n Mixed

	if (nvram_wlan_get_int(1, "HT_BW") > 1)
		nvram_wlan_set_int(1, "HT_BW", 1);
#endif

	if (nvram_wlan_get_int(1, "stream_tx") > BOARD_NUM_ANT_5G_TX)
		nvram_wlan_set_int(1, "stream_tx", BOARD_NUM_ANT_5G_TX);

	if (nvram_wlan_get_int(1, "stream_rx") > BOARD_NUM_ANT_5G_RX)
		nvram_wlan_set_int(1, "stream_rx", BOARD_NUM_ANT_5G_RX);
#endif

	if (strlen(nvram_wlan_get(0, "ssid")) < 1)
		nvram_wlan_set(0, "ssid", DEF_WLAN_2G_SSID);

	memset(buff, 0, sizeof(buff));
	char_to_ascii(buff, nvram_wlan_get(0, "ssid"));
	nvram_wlan_set(0, "ssid2", buff);

	if (strlen(nvram_wlan_get(0, "gmode")) < 1)
		nvram_wlan_set_int(0, "gmode", 2); // b/g/n Mixed

	if (nvram_wlan_get_int(0, "HT_BW") > 1)
		nvram_wlan_set_int(0, "HT_BW", 1);

	if (nvram_wlan_get_int(0, "stream_tx") > BOARD_NUM_ANT_2G_TX)
		nvram_wlan_set_int(0, "stream_tx", BOARD_NUM_ANT_2G_TX);

	if (nvram_wlan_get_int(0, "stream_rx") > BOARD_NUM_ANT_2G_RX)
		nvram_wlan_set_int(0, "stream_rx", BOARD_NUM_ANT_2G_RX);

	nvram_set_temp("ntpc_counter", "0000000000");
	nvram_set_temp("login_timestamp", "0000000000");

	nvram_set_int_temp("networkmap_fullscan", 0);
	nvram_set_int_temp("link_internet", 2);
	nvram_set_int_temp("link_wan", 0);

	nvram_set_int_temp("led_front_t", 1);
	nvram_set_int_temp("led_ether_t", 1);

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
	nvram_set_temp("deferred_wanup_t", "0000000000");
	nvram_set_temp("vpnc_dns_t", "");
	nvram_set_temp("vpnc_dom_t", "");
	nvram_set_temp("viptv_ifname", "");

	nvram_set_temp(RSTATS_NVKEY_24, (sw_mode != 3) ? IFDESC_WAN : IFDESC_LAN);
	nvram_set_temp(RSTATS_NVKEY_DM, IFDESC_WAN);

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

void
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
	nvram_set_int("restore_defaults", 1);
	nvram_commit();
}

static void
flash_firmware(void)
{
	const char *script_name = SCRIPT_SHUTDOWN;
	char* svcs[] = { "l2tpd",
			 "xl2tpd",
			 "pppd",
			 "wpa_cli",
			 "wpa_supplicant",
			 NULL };

	stop_misc();
	stop_services(0); // don't stop httpd/telnetd/sshd/vpn

	if (check_if_file_exist(script_name))
		doSystem("%s %d", script_name, 0);

#if defined (USE_STORAGE)
	safe_remove_all_stor_devices(0);
#endif
#if defined (USE_USB_SUPPORT)
	stop_usb_printer_spoolers();
#endif
	stop_igmpproxy(NULL);

	kill_services(svcs, 6, 1);

	/* save storage (if changed) */
	storage_save_time(60);
	write_storage_to_mtd();

	sync();
	sleep(1);

	if (eval("/tmp/mtd_write", "-r", "write", FW_IMG_NAME, FW_MTD_NAME) != 0) {
		start_watchdog();
	}
}

static void
storage_load_time(void)
{
#if !defined (USE_RTC_HCTOSYS)
	FILE *fp;
	char buf[32];
	struct tm storage_tm;
	time_t storage_time = 0;

	fp = fopen("/etc/storage/system_time", "r");
	if (fp) {
		if (fgets(buf, sizeof(buf), fp))
			storage_time = (time_t)strtoul(buf, NULL, 0);
		fclose(fp);
	}

	if (storage_time > 0) {
		localtime_r(&storage_time, &storage_tm);
		if (storage_tm.tm_year >= (SYS_START_YEAR - 1900)) {
			storage_time = mktime(&storage_tm);
			storage_time += 5; // add delta 5 sec (~boot time)
			stime(&storage_time);
		}
	}
#endif
}

void
storage_save_time(time_t delta)
{
#if !defined (USE_RTC_HCTOSYS)
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
#endif
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
	int is_soft_blink = 0;

	switch (gpio_led)
	{
#if defined (BOARD_GPIO_LED_ROUTER)
	case BOARD_GPIO_LED_ROUTER:
		break;
#endif
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
#if defined (BOARD_GPIO_LED_SW2G)
	case BOARD_GPIO_LED_SW2G:
		is_soft_blink = 1;
		front_led_x = nvram_get_int("front_led_wif");
		if (front_led_x) {
#if defined (BOARD_GPIO_LED_SW5G) && (BOARD_GPIO_LED_SW5G == BOARD_GPIO_LED_SW2G)
			flag = (is_radio_on_rt() || is_radio_on_wl()) ? LED_ON : LED_OFF;
#else
			flag = (is_radio_on_rt()) ? LED_ON : LED_OFF;
#endif
		}
		break;
#endif
#if defined (BOARD_GPIO_LED_SW5G) && (!defined (BOARD_GPIO_LED_SW2G) || (BOARD_GPIO_LED_SW5G != BOARD_GPIO_LED_SW2G))
	case BOARD_GPIO_LED_SW5G:
		is_soft_blink = 1;
		front_led_x = nvram_get_int("front_led_wif");
		if (front_led_x) {
			flag = (is_radio_on_wl()) ? LED_ON : LED_OFF;
		}
		break;
#endif
#if defined (BOARD_GPIO_LED_USB)
	case BOARD_GPIO_LED_USB:
#if defined (BOARD_GPIO_LED_USB2)
	case BOARD_GPIO_LED_USB2:
#endif
#if defined (USE_USB_SUPPORT)
		front_led_x = nvram_get_int("front_led_usb");
#else
		front_led_x = 0;
#endif
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
	if (nvram_get_int("front_led_all") == 0
#if defined (BOARD_GPIO_LED_POWER)
	    && gpio_led != BOARD_GPIO_LED_POWER
#endif
	)
		flag = LED_OFF;
#endif

	if (flag != LED_OFF && !nvram_get_int("led_front_t"))
		flag = LED_OFF;

#if defined (BOARD_GPIO_LED_WIFI)
#if defined (CONFIG_RALINK_MT7620) && (BOARD_GPIO_LED_WIFI == 72)
	if (gpio_led == BOARD_GPIO_LED_WIFI) {
		cpu_gpio_mode_set_bit(13, (flag == LED_OFF) ? 1 : 0); // change GPIO Mode for WLED
		cpu_gpio_set_pin(gpio_led, LED_OFF); // always set GPIO to high
	} else
#endif
#if defined (CONFIG_RALINK_MT7628) && (BOARD_GPIO_LED_WIFI == 44)
	if (gpio_led == BOARD_GPIO_LED_WIFI) {
		cpu_gpio_mode_set_bit(32, (flag == LED_OFF) ? 1 : 0); // change GPIO Mode for WLED_AN
		cpu_gpio_mode_set_bit(48, (flag == LED_OFF) ? 1 : 0); // change GPIO Mode for WLED_KN
		cpu_gpio_set_pin(gpio_led, LED_OFF); // always set GPIO to high
	} else
#endif
#endif
	{
#if defined (BOARD_HC5761A)
		if (gpio_led == BOARD_GPIO_LED_SW5G) {
			cpu_gpio_mode_set_bit(40, 1);
			cpu_gpio_mode_set_bit(41, 0);
		}
#endif
		if (is_soft_blink)
			cpu_gpio_led_enabled(gpio_led, (flag == LED_OFF) ? 0 : 1);
		
		cpu_gpio_set_pin(gpio_led, flag);
	}
}

int
init_crontab(void)
{
	int ret = 0; //no change
#if defined (APP_SCUT)
	ret |= system("/sbin/check_crontab.sh a/1 a a a a scutclient_watchcat.sh");
#endif
#if defined (APP_SHADOWSOCKS)
	ret |= system("/sbin/check_crontab.sh a/5 a a a a ss-watchcat.sh");
	ret |= system("/sbin/check_crontab.sh 0 8 a/10 a a update_chnroute.sh");
	ret |= system("/sbin/check_crontab.sh 0 7 a/10 a a update_gfwlist.sh");
#endif
	return ret;
}

void 
init_router(void)
{
	int log_remote, is_ap_mode, nvram_need_commit;

#if defined (USE_RTL8367)
	rtl8367_node();
#endif
#if defined (USE_MTK_ESW) || defined (USE_MTK_GSW)
	mtk_esw_node();
#endif

	nvram_convert_old_params();

	nvram_need_commit = nvram_restore_defaults();

	get_eeprom_params();

	nvram_convert_misc_values();

	init_gpio_leds_buttons();

	if (nvram_need_commit)
		nvram_commit();

	mount_rwfs_partition();

	gen_ralink_config_2g(0);
	gen_ralink_config_5g(0);
	load_wireless_modules();
#if defined (USE_MMC_SUPPORT)
	load_mmc_modules();
#endif
#if defined (USE_USB_SUPPORT)
	load_usb_modules();
#endif
#if defined (USE_ATA_SUPPORT)
	load_ata_modules();
#endif
#if defined (USE_MTK_AES)
	load_crypto_modules();
#endif
#if defined (USE_IPSET)
	load_ipset_modules();
#endif

	recreate_passwd_unix(1);

	set_timezone();
	set_pagecache_reclaim();

	storage_load_time();

	log_remote = nvram_invmatch("log_ipaddr", "");
	if (!log_remote)
		start_logger(1);

	is_ap_mode = get_ap_mode();

	init_loopback();
	init_bridge(is_ap_mode);
#if defined (USE_IPV6)
	init_ipv6();
#endif
	set_cpu_affinity(is_ap_mode);

	start_detect_link();
	start_detect_internet(0);
	start_lan(is_ap_mode, 0);

	if (log_remote)
		start_logger(1);

#if defined (BOARD_HC5761A)
	cpu_gpio_mode_set_bit(38, 1);
	cpu_gpio_mode_set_bit(39, 0);
	cpu_gpio_set_pin_direction(BOARD_GPIO_PWR_USB, 1);
	cpu_gpio_set_pin(BOARD_GPIO_PWR_USB, BOARD_GPIO_PWR_USB_ON);
#endif

	start_dns_dhcpd(is_ap_mode);
#if defined (APP_SMBD) || defined (APP_NMBD)
	start_wins();
#endif

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

	start_rwfs_optware();
#if defined(APP_NAPT66)
	start_napt66();
#endif
	if (init_crontab()) {
		write_storage_to_mtd();
		restart_crond();
	}
	// system ready
	system("/etc/storage/started_script.sh &");
}

/*
 * level {0: reboot, 1: halt, 2: power-off}
 */
void
shutdown_router(int level)
{
	int use_halt = (level == 1) ? 1 : 0;
	int is_ap_mode = get_ap_mode();
	const char *script_name = SCRIPT_SHUTDOWN;

	stop_misc();

	if (level < 2)
		stop_services(use_halt);

	if (check_if_file_exist(script_name))
		doSystem("%s %d", script_name, level);

#if defined (USE_STORAGE)
	safe_remove_all_stor_devices(use_halt);
#endif

#if defined (USE_USB_SUPPORT)
	stop_usb_printer_spoolers();
#endif
#if defined (BOARD_GPIO_LED_USB)
	LED_CONTROL(BOARD_GPIO_LED_USB, LED_OFF);
#endif
#if defined (BOARD_GPIO_LED_USB2)
	LED_CONTROL(BOARD_GPIO_LED_USB2, LED_OFF);
#endif

	stop_wan();
	if (level < 2) {
		stop_services_lan_wan();
		set_ipv4_forward(0);
	}
#if defined (BOARD_GPIO_LED_WAN)
	LED_CONTROL(BOARD_GPIO_LED_WAN, LED_OFF);
#endif

	storage_save_time(10);
	write_storage_to_mtd();

	stop_8021x_all();
	stop_wifi_all_wl();
	stop_wifi_all_rt();

	if (level < 2) {
		stop_logger();
		stop_lan(is_ap_mode);
	}

	umount_rwfs_partition();

	if (use_halt) {
		module_smart_unload("hw_nat", 0);
		module_smart_unload("rt_timer_wdg", 0);
	}

#if defined (BOARD_GPIO_LED_LAN)
	LED_CONTROL(BOARD_GPIO_LED_LAN, LED_OFF);
#endif
#if defined (BOARD_GPIO_LED_POWER)
	LED_CONTROL(BOARD_GPIO_LED_POWER, LED_OFF);
#endif
}

void 
handle_notifications(void)
{
	int i, stop_handle = 0;
	char notify_name[300];

	DIR *directory = opendir(DIR_RC_NOTIFY);
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
		snprintf(notify_name, sizeof(notify_name), "%s/%s", DIR_RC_NOTIFY, entry->d_name);
		remove(notify_name);
		
		printf("rc notification: %s\n", entry->d_name);
		
		/* Take the appropriate action. */
		if (!strcmp(entry->d_name, RCN_RESTART_REBOOT))
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
		else if (!strcmp(entry->d_name, RCN_RESTART_IPV6))
		{
			if (!get_ap_mode()) {
				full_restart_ipv6(nvram_ipv6_type);
				nvram_ipv6_type = get_ipv6_type();
			}
		}
		else if (strcmp(entry->d_name, RCN_RESTART_RADV) == 0)
		{
			restart_dhcpd();
		}
#endif
		else if (!strcmp(entry->d_name, RCN_RESTART_WAN))
		{
			full_restart_wan();
		}
		else if (!strcmp(entry->d_name, RCN_RESTART_LAN))
		{
			full_restart_lan();
		}
		else if (!strcmp(entry->d_name, "stop_whole_wan"))
		{
			stop_wan();
		}
		else if (!strcmp(entry->d_name, RCN_RESTART_IPTV))
		{
			int is_ap_mode = get_ap_mode();
			restart_iptv(is_ap_mode);
			if (!is_ap_mode)
				restart_firewall();
		}
		else if(!strcmp(entry->d_name, "deferred_wan_connect"))
		{
			deferred_wan_connect();
		}
		else if(!strcmp(entry->d_name, "auto_wan_reconnect"))
		{
			auto_wan_reconnect();
		}
		else if(!strcmp(entry->d_name, "auto_wan_reconnect_pause"))
		{
			auto_wan_reconnect_pause();
		}
		else if(!strcmp(entry->d_name, "manual_wan_reconnect"))
		{
			manual_wan_reconnect();
		}
		else if(!strcmp(entry->d_name, "manual_wan_disconnect"))
		{
			manual_wan_disconnect();
		}
		else if(!strcmp(entry->d_name, "manual_wisp_reassoc"))
		{
			manual_wisp_reassoc();
		}
		else if(!strcmp(entry->d_name, "manual_ddns_hostname_check"))
		{
			manual_ddns_hostname_check();
		}
#if defined (USE_USB_SUPPORT)
		else if (!strcmp(entry->d_name, RCN_RESTART_MODEM))
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
		else if (strcmp(entry->d_name, RCN_RESTART_SPOOLER) == 0)
		{
			restart_usb_printer_spoolers();
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
#if defined (USE_STORAGE)
		else if (!strcmp(entry->d_name, "on_hotplug_mass_storage"))
		{
			// deferred run stor apps
			nvram_set_int_temp("usb_hotplug_ms", 1);
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_unplug_mass_storage"))
		{
			umount_ejected();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_HDDTUNE) == 0)
		{
			system("/sbin/hddtune.sh");
			set_pagecache_reclaim();
		}
#if defined(APP_FTPD)
		else if (strcmp(entry->d_name, RCN_RESTART_FTPD) == 0)
		{
			restart_ftpd();
		}
#endif
#if defined(APP_SMBD)
		else if (strcmp(entry->d_name, RCN_RESTART_SMBD) == 0)
		{
			restart_smbd();
		}
#endif
#if defined(APP_NFSD)
		else if (strcmp(entry->d_name, RCN_RESTART_NFSD) == 0)
		{
			restart_nfsd();
		}
#endif
#if defined(APP_MINIDLNA)
		else if (strcmp(entry->d_name, "restart_dms_rescan") == 0)
		{
			restart_dms(1);
		}
		else if (strcmp(entry->d_name, RCN_RESTART_DMS) == 0)
		{
			restart_dms(0);
		}
#endif
#if defined(APP_FIREFLY)
		else if (strcmp(entry->d_name, RCN_RESTART_ITUNES) == 0)
		{
			restart_itunes();
		}
#endif
#if defined(APP_TRMD)
		else if (strcmp(entry->d_name, RCN_RESTART_TRMD) == 0)
		{
			restart_torrent();
		}
#endif
#if defined(APP_ARIA)
		else if (strcmp(entry->d_name, RCN_RESTART_ARIA) == 0)
		{
			restart_aria();
		}
#endif
#endif
		else if (strcmp(entry->d_name, RCN_RESTART_HTTPD) == 0)
		{
			restart_httpd();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_TELNETD) == 0)
		{
			stop_telnetd();
			start_telnetd();
		}
#if defined(APP_SSHD)
		else if (strcmp(entry->d_name, RCN_RESTART_SSHD) == 0)
		{
			restart_sshd();
		}
#endif
#if defined(APP_SCUT)
		else if (strcmp(entry->d_name, RCN_RESTART_SCUT) == 0)
		{
			restart_scutclient();
		}
		else if (strcmp(entry->d_name, "stop_scutclient") == 0)
		{
			stop_scutclient();
		}
#endif
#if defined(APP_MENTOHUST)
		else if (strcmp(entry->d_name, RCN_RESTART_MENTOHUST) == 0)
		{
			restart_mentohust();
		}
		else if (strcmp(entry->d_name, "stop_mentohust") == 0)
		{
			stop_mentohust();
		}
#endif
#if defined(APP_TTYD)
		else if (strcmp(entry->d_name, RCN_RESTART_TTYD) == 0)
		{
			restart_ttyd();
		}
#endif
#if defined(APP_SHADOWSOCKS)
		else if (strcmp(entry->d_name, RCN_RESTART_SHADOWSOCKS) == 0)
		{
			restart_ss();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_SS_TUNNEL) == 0)
		{
			restart_ss_tunnel();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_CHNROUTE_UPD) == 0)
		{
			update_chnroute();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_GFWLIST_UPD) == 0)
		{
			update_gfwlist();
		}
#endif
#if defined(APP_VLMCSD)
		else if (strcmp(entry->d_name, RCN_RESTART_VLMCSD) == 0)
		{
			restart_vlmcsd();
		}
#endif
#if defined(APP_DNSFORWARDER)
		else if (strcmp(entry->d_name, RCN_RESTART_DNSFORWARDER) == 0)
		{
			restart_dnsforwarder();
		}
#endif
#if defined(APP_SMBD) || defined(APP_NMBD)
		else if (strcmp(entry->d_name, RCN_RESTART_NMBD) == 0)
		{
			restart_nmbd();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_WINS) == 0)
		{
			restart_nmbd();
			restart_dhcpd();
			reapply_vpn_server();
		}
#endif
		else if (strcmp(entry->d_name, RCN_RESTART_LLTD) == 0)
		{
			restart_lltd();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_ADSC) == 0)
		{
			restart_infosvr();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_CROND) == 0)
		{
			restart_crond();
		}
		else if (strcmp(entry->d_name, RCN_REAPPLY_VPNSVR) == 0)
		{
			reapply_vpn_server();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_VPNSVR) == 0)
		{
			restart_vpn_server();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_VPNCLI) == 0)
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
		else if (strcmp(entry->d_name, RCN_RESTART_DDNS) == 0)
		{
			stop_ddns();
			start_ddns(1);
		}
		else if (strcmp(entry->d_name, RCN_RESTART_DI) == 0)
		{
			if (get_ap_mode() || has_wan_ip4(0))
				notify_run_detect_internet(2);
		}
		else if (strcmp(entry->d_name, RCN_RESTART_DHCPD) == 0)
		{
			if (get_ap_mode())
				update_hosts_ap();
			restart_dhcpd();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_UPNP) == 0)
		{
			restart_upnp();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_SWITCH_CFG) == 0)
		{
			config_bridge(get_ap_mode());
			switch_config_base();
			switch_config_storm();
			switch_config_link();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_SWITCH_VLAN) == 0)
		{
			restart_switch_config_vlan();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_SYSLOG) == 0)
		{
			stop_logger();
			start_logger(0);
		}
		else if (strcmp(entry->d_name, RCN_RESTART_WDG) == 0)
		{
			restart_watchdog_cpu();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_TWEAKS) == 0)
		{
			notify_leds_detect_link();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_NETFILTER) == 0)
		{
			update_router_mode();
			reload_nat_modules();
			restart_firewall();
			flush_conntrack_table(NULL);
		}
		else if (strcmp(entry->d_name, RCN_RESTART_FIREWALL) == 0)
		{
			reload_nat_modules();
			restart_firewall();
		}
		else if (strcmp(entry->d_name, "restart_firewall_wan") == 0)
		{
			restart_firewall();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_NTPC) == 0)
		{
			notify_watchdog_time();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_TIME) == 0)
		{
			stop_logger();
			set_timezone();
			notify_watchdog_time();
			notify_rstats_time();
			start_logger(0);
			restart_crond();
		}
		else if (strcmp(entry->d_name, RCN_RESTART_SYSCTL) == 0)
		{
			int nf_nat_type = nvram_get_int("nf_nat_type");
			
			restart_all_sysctl();
			
			/* flush conntrack after NAT model changing */
			if (nvram_nf_nat_type != nf_nat_type) {
				nvram_nf_nat_type = nf_nat_type;
				flush_conntrack_table(NULL);
			}
		}
		else if (!strcmp(entry->d_name, RCN_RESTART_WIFI5))
		{
			int radio_on = get_enabled_radio_wl();
			if (radio_on)
				radio_on = is_radio_allowed_wl();
			restart_wifi_wl(radio_on, 1);
		}
		else if (!strcmp(entry->d_name, RCN_RESTART_WIFI2))
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
			snprintf(notify_name, sizeof(notify_name), "%s/%s", DIR_RC_INCOMPLETE, entry->d_name);
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
#if defined (USE_IPV6)
	{ "ipv6-up",		ipv6up_main		},
	{ "ipv6-down",		ipv6down_main		},
	{ "dhcp6c.script",	dhcp6c_main		},
#endif
	{ "ip-up.vpns",		ipup_vpns_main		},
	{ "ip-down.vpns",	ipdown_vpns_main	},
	{ "ip-up.vpnc",		ipup_vpnc_main		},
	{ "ip-down.vpnc",	ipdown_vpnc_main	},

#if defined (APP_OPENVPN)
	{ SCRIPT_OVPN_SERVER,	ovpn_server_script_main	},
	{ SCRIPT_OVPN_CLIENT,	ovpn_client_script_main	},
	{ "ovpn_export_client",	ovpn_server_expcli_main	},
#endif
#if defined (USE_MMC_SUPPORT)
	{ "mdev_mmc",		mdev_mmc_main		},
#endif
#if defined (USE_BLK_DEV_SD)
	{ "mdev_sd",		mdev_sd_main		},
#endif
#if defined (USE_USB_SUPPORT)
	{ "mdev_sg",		mdev_sg_main		},
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

	{ "mtk_gpio",		cpu_gpio_main		},
#if defined (USE_MTK_ESW) || defined (USE_MTK_GSW)
	{ "mtk_esw",		mtk_esw_main		},
#endif
#if defined (USE_RTL8367)
	{ "rtl8367",		rtl8367_main		},
#endif
#if defined (USE_RT3352_MII)
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
	else if (!strcmp(base, "run_telnetd")) {
		run_telnetd();
	}
	else if (!strcmp(base, "run_ftpsamba")) {
#if defined(APP_SMBD)
		restart_smbd();
#else
		;
#endif
#if defined(APP_FTPD)
		restart_ftpd();
#endif
	}
#if defined(APP_SMBD)
	else if (!strcmp(base, "run_samba")) {
		restart_smbd();
	}
#endif
#if defined(APP_FTPD)
	else if (!strcmp(base, "run_ftp")) {
		restart_ftpd();
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
		stop_samba(0);
	}
#endif
	else if (!strcmp(base, "stop_ftpsamba")) {
#if defined(APP_FTPD)
		stop_ftp();
#endif
#if defined(APP_SMBD)
		stop_samba(0);
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
	else if (!strcmp(base, "stop_wan")) {
		notify_rc("manual_wan_disconnect");
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
	else if (!strcmp(base, "restart_vpn_server")) {
		restart_vpn_server();
	}
	else if (!strcmp(base, "restart_vpn_client")) {
		restart_vpn_client();
	}
	else if (!strcmp(base, "restart_networkmap")) {
		restart_networkmap();
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
	else if (!strcmp(base, "lan_eeprom_mac")) {
		if (argc > 1 && strlen(argv[1]) == 17)
			ret = set_wired_mac(0, argv[1]);
		else {
			printf("Usage: %s XX:XX:XX:XX:XX:XX\n\n", base);
			ret = get_wired_mac(0);
		}
	}
	else if (!strcmp(base, "wan_eeprom_mac")) {
		if (argc > 1 && strlen(argv[1]) == 17)
			ret = set_wired_mac(1, argv[1]);
		else {
			printf("Usage: %s XX:XX:XX:XX:XX:XX\n\n", base);
			ret = get_wired_mac(1);
		}
	}
	else if (!strcmp(base, "leds_front")) {
		if (argc > 1) {
			int power_on = atoi(argv[1]);
			show_hide_front_leds(power_on);
		} else {
			printf("Usage: %s <show>\n\n", base);
		}
	}
	else if (!strcmp(base, "leds_ether")) {
		if (argc > 1) {
			int power_on = atoi(argv[1]);
			show_hide_ether_leds(power_on);
		} else {
			printf("Usage: %s <show>\n\n", base);
		}
	}
#if defined (USE_USB_SUPPORT)
	else if (!strcmp(base, "usb5v")) {
		if (argc > 1) {
#if defined (BOARD_GPIO_PWR_USB) || defined (BOARD_GPIO_PWR_USB2)
			int port = 0;
			int power_on = atoi(argv[1]);
			if (argc > 2)
				port = atoi(argv[2]);
			power_control_usb_port(port, power_on);
#endif
		} else {
			printf("Usage: %s <power> [port]\n\n", base);
		}
	}
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
		safe_remove_usb_device(port, devn);
	}
	else if (!strcmp(base, "ejusb1")) {
		char *devn = (argc > 1) ? argv[1] : NULL;
		safe_remove_usb_device(1, devn);
	}
#if (BOARD_NUM_USB_PORTS > 1)
	else if (!strcmp(base, "ejusb2")) {
		char *devn = (argc > 1) ? argv[1] : NULL;
		safe_remove_usb_device(2, devn);
	}
#endif
#endif
#if defined (USE_ATA_SUPPORT)
	else if (!strcmp(base, "ejata")) {
		char *devn = (argc > 1) ? argv[1] : NULL;
		safe_remove_stor_device(ATA_VIRT_PORT_ID, ATA_VIRT_PORT_ID, devn, 1);
	}
#endif
#if defined (USE_MMC_SUPPORT)
	else if (!strcmp(base, "ejmmc")) {
		safe_remove_stor_device(MMC_VIRT_PORT_ID, MMC_VIRT_PORT_ID, NULL, 0);
	}
#endif
#if defined (USE_STORAGE)
	else if (!strcmp(base, "ejall")) {
		safe_remove_all_stor_devices(1);
	}
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

