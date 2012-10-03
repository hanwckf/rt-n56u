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
#include "rtl8367m.h"

extern struct nvram_tuple router_defaults[];

/* static values */
static int nvram_nf_nat_type = 0;
static int nvram_ipv6_type = 0;

static void
nvram_restore_defaults(void)
{
	nvram_set("NVRAMMAGIC", "");

	struct nvram_tuple *t;
	int restore_defaults;

	/* Restore defaults if told to or OS has changed */
	restore_defaults = !nvram_match("restore_defaults", "0");

	if (restore_defaults) {
		logmessage(LOGNAME, "Restoring defaults...");
	}

	/* Restore defaults */
	for (t = router_defaults; t->name; t++) {
		if (restore_defaults || !nvram_get(t->name)) {
			nvram_set(t->name, t->value);
		}
	}

	klogctl(8, NULL, nvram_get_int("console_loglevel"));

	/* load static values */
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
	system("modprobe -q ehci-hcd");
	system("modprobe -q ohci-hcd");
	system("modprobe -q rt2860v2_ap");
	system("modprobe -q rt3090_ap");
}

static void
set_timezone(void)
{
	time_t now;
	struct tm gm, local;
	struct timezone tz;
	struct timeval *tvp = NULL;

	/* Export TZ variable for the time libraries to 
	 * use.
	 */
	time_zone_x_mapping();
	setenv_tz();
	
	/* Update kernel timezone */
	time(&now);
	gmtime_r(&now, &gm);
	localtime_r(&now, &local);
	tz.tz_minuteswest = (mktime(&gm) - mktime(&local)) / 60;
	settimeofday(tvp, &tz);
}

static void
init_gpio_leds_buttons(void)
{
	cpu_gpio_set_pin_direction(LED_WAN,   GPIO_DIR_OUT);
	cpu_gpio_set_pin_direction(LED_LAN,   GPIO_DIR_OUT);
	cpu_gpio_set_pin_direction(LED_USB,   GPIO_DIR_OUT);
	cpu_gpio_set_pin_direction(LED_POWER, GPIO_DIR_OUT);
	cpu_gpio_set_pin_direction(BTN_RESET, GPIO_DIR_IN);
	cpu_gpio_set_pin_direction(BTN_WPS,   GPIO_DIR_IN);

	LED_CONTROL(LED_POWER, LED_ON);
	LED_CONTROL(LED_WAN,   LED_OFF);
	LED_CONTROL(LED_LAN,   LED_OFF);
	LED_CONTROL(LED_USB,   LED_OFF);
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
	nvram_set(strcat_r(prefix, "unit", tmp), "0");
	nvram_set(strcat_r(prefix, "desc", tmp), "Default Connection");
	nvram_set(strcat_r(prefix, "primary", tmp), "1");
	nvram_set(strcat_r(prefix, "ifname", tmp), IFNAME_WAN);
	nvram_set(strcat_r(prefix, "ifnames", tmp), IFNAME_WAN);
}

static void 
convert_misc_values()
{
	char buff[100];

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

	if (!strcmp(nvram_safe_get("wl_gmode"), ""))
		nvram_set("wl_gmode", "2");

	if (!strcmp(nvram_safe_get("rt_gmode"), ""))
		nvram_set("rt_gmode", "2");

	nvram_set("lan_ipaddr_t", "");
	nvram_set("lan_netmask_t", "");
	nvram_set("lan_gateway_t", "");
	nvram_set("lan_dns_t", "");

	nvram_set("wan_ipaddr_t", "");
	nvram_set("wan_netmask_t", "");
	nvram_set("wan_gateway_t", "");
	nvram_set("wan_dns_t", "");

	nvram_unset("wanx_ipaddr"); 
	nvram_unset("wanx_netmask");
	nvram_unset("wanx_gateway");
	nvram_unset("wanx_dns");
	nvram_unset("wanx_lease");

	nvram_set("qos_enable", "0");

	nvram_set("link_wan", "0");
	nvram_set("link_lan", "0");
	nvram_set("usb_path", "");
	nvram_set("usb_path1", "");
	nvram_set("usb_path2", "");
	nvram_set("usb_hotplug_ms", "0");
	nvram_set("usb_hotplug_lp", "0");
	nvram_set("usb_hotplug_md", "0");
	nvram_set("modem_node_t", "");
	nvram_set("rndis_ifname", "");
	nvram_set("lld2d_wif", "");
	nvram_set("l2tp_cli_t", "0");
	nvram_set("l2tp_srv_t", "0");

	/* Setup wan0 variables if necessary */
	set_wan0_vars();
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
	
	log_remote = nvram_invmatch("log_ipaddr", "");
	if (!log_remote)
	{
		start_logger(1);
	}
	
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
	{
		start_logger(1);
	}
	
	default_filter_setting();
	default_nat_setting();
#if defined (USE_IPV6)
	default_filter6_setting();
#endif
	if (nvram_match("modem_arun", "1"))
		select_usb_modem_to_wan(5);
	start_wan();
	load_usb_storage_module();
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
	LED_CONTROL(LED_USB, LED_OFF);
	
	stop_wan();
	stop_services_lan_wan();
	LED_CONTROL(LED_WAN, LED_OFF);
	
	write_storage_to_mtd();
	
	stop_wifi_all_wl();
	stop_wifi_all_rt();
	stop_logger();
	stop_lan();

	LED_CONTROL(LED_LAN, LED_OFF);
	LED_CONTROL(LED_POWER, LED_OFF);
}

void 
handle_notifications(void)
{
	int i, stop_handle = 0;
	DIR *directory = opendir("/tmp/rc_notification");
	if (!directory)
		return;
	
	// handle max 10 requests at once (prevent deadlock)
	for (i=0; i < 10; i++)
	{
		struct dirent *entry;
		char *full_name;
		FILE *test_fp;
		
		entry = readdir(directory);
		if (!entry)
			break;
		if (strcmp(entry->d_name, ".") == 0)
			continue;
		if (strcmp(entry->d_name, "..") == 0)
			continue;
		
		/* Remove the marker file. */
		full_name = (char *)(malloc(strlen(entry->d_name) + 100));
		if (!full_name)
		{
			break;
		}
		sprintf(full_name, "/tmp/rc_notification/%s", entry->d_name);
		remove(full_name);
		
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
#if defined (USE_IPV6)
		else if (!strcmp(entry->d_name, "restart_ipv6"))
		{
			full_restart_ipv6(nvram_ipv6_type);
			nvram_ipv6_type = get_ipv6_type();
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
			stop_samba();
			run_samba();
			restart_ftp();
		}
		else if (strcmp(entry->d_name, "restart_nfs") == 0)
		{
			stop_nfsd();
			sleep(1);
			run_nfsd();
		}
		else if (strcmp(entry->d_name, "restart_dms") == 0)
		{
			restart_dms();
		}
		else if (strcmp(entry->d_name, "restart_torrent") == 0)
		{
			restart_torrent();
		}
		else if (strcmp(entry->d_name, "restart_term") == 0)
		{
			restart_term();
		}
		else if (strcmp(entry->d_name, "restart_vpn_server") == 0)
		{
			restart_vpn_server();
		}
		else if (strcmp(entry->d_name, "restart_ddns") == 0)
		{
			nvram_set("ddns_updated", "0");
			notify_watchdog_ddns();
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
			stop_upnp();
			start_upnp();
		}
		else if (strcmp(entry->d_name, "restart_switch_config") == 0)
		{
			switch_config_base();
			switch_config_storm();
			switch_config_link();
		}
		else if (strcmp(entry->d_name, "restart_switch_vlan") == 0)
		{
			switch_config_vlan(0);
		}
		else if (strcmp(entry->d_name, "restart_syslog") == 0)
		{
			stop_logger();
			start_logger(0);
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
			start_logger(0);
		}
		else if (strcmp(entry->d_name, "restart_spooler") == 0)
		{
			restart_usb_printer_spoolers();
		}
		else if (strcmp(entry->d_name, "restart_hddtune") == 0)
		{
			system("/sbin/hddtune.sh");
		}
		else if (strcmp(entry->d_name, "restart_sysctl") == 0)
		{
			restart_all_sysctl();
		}
		else if (!strcmp(entry->d_name, "restart_wifi_wl"))
		{
			int radio_on = !nvram_match("wl_radio_x", "0");
			if (radio_on)
				radio_on = is_radio_allowed_wl();
			restart_wifi_wl(radio_on, 1);
		}
		else if (!strcmp(entry->d_name, "restart_wifi_rt"))
		{
			int radio_on = !nvram_match("rt_radio_x", "0");
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
			nvram_set("usb_hotplug_ms", "1");
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_removal_usb_storage"))
		{
			// try deferred restart usb apps after surprise removal
			nvram_set("usb_hotplug_ms", "1");
			alarm(1);
		}
		else if (!strcmp(entry->d_name, "on_hotplug_usb_printer"))
		{
			// deferred run usb printer daemons
			nvram_set("usb_hotplug_lp", "1");
			alarm(5);
		}
		else if (!strcmp(entry->d_name, "on_hotplug_usb_modem"))
		{
			// deferred run usb modem to wan
			nvram_set("usb_hotplug_md", "1");
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
		test_fp = fopen(full_name, "r");
		if (test_fp != NULL)
		{
			fclose(test_fp);
		}
		else
		{
			/* Remove the marker file. */
			sprintf(full_name, "/tmp/rc_action_incomplete/%s", entry->d_name);
			remove(full_name);
		}
		
		free(full_name);
		
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
	{ "zcip.script",	zcip_main		},
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

	{ "mdev_sg",		mdev_sg_main		},
	{ "mdev_sd",		mdev_sd_main		},
	{ "mdev_sr",		mdev_sr_main		},
	{ "mdev_lp",		mdev_lp_main		},
	{ "mdev_tty",		mdev_tty_main		},
	{ "mdev_net",		mdev_net_main		},
	{ "mdev_usb",		mdev_usb_main		},

	{ "ddns_updated",	ddns_updated_main	},

	{ "detect_wan",		detect_wan_main		},
	{ "detect_link",	detect_link_main	},
	{ "detect_internet",	detect_internet_main	},

	{ "watchdog",		watchdog_main		},
	{ "rtl8367m",		rtl8367m_main		},

	{ NULL, NULL }
};


int
main(int argc, char **argv)
{
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

	if (!strcmp(base, "reset_to_defaults")) {
		erase_nvram();
		erase_storage();
		sys_exit();
		return 0;
	}
	else if (!strcmp(base, "run_ftpsamba")) {
		stop_samba();
		run_samba();
		restart_ftp();
		return 0;
	}
	else if (!strcmp(base, "run_samba")) {
		stop_samba();
		run_samba();
		return 0;
	}
	else if (!strcmp(base, "run_ftp")) {
		restart_ftp();
		return 0;
	}
	else if (!strcmp(base, "run_nfsd")) {
		run_nfsd();
		return 0;
	}
	else if (!strcmp(base, "run_dms")) {
		restart_dms();
		return 0;
	}
	else if (!strcmp(base, "run_torrent")) {
		restart_torrent();
		return 0;
	}
	else if (!strcmp(base, "stop_ftp")) {
		stop_ftp();
		return 0;
	}
	else if (!strcmp(base, "stop_samba")) {
		stop_samba();
		return 0;
	}
	else if (!strcmp(base, "stop_ftpsamba")) {
		stop_ftp();
		stop_samba();
		return 0;
	}
	else if (!strcmp(base, "stop_nfsd")) {
		stop_nfsd();
		return 0;
	}
	else if (!strcmp(base, "stop_dms")) {
		stop_dms();
		return 0;
	}
	else if (!strcmp(base, "stop_torrent")) {
		stop_torrent();
		return 0;
	}
	else if (!strcmp(base, "stopservice")) {
		if (argc >= 2)
			return stop_service_main(atoi(argv[1]));
		else
			return stop_service_main(0);
	}
	else if (!strcmp(base, "start_ddns")) {
		nvram_set("ddns_updated", "0");
		notify_watchdog_ddns();
		return 0;
	}
	else if (!strcmp(base, "getCountryCode")) {
		return getCountryCode();
	}
	else if (!strcmp(base, "setCountryCode")) {
		if (argc == 2)
			return setCountryCode(argv[1]);
		else
			return EINVAL;
	}
	else if (!strcmp(base, "gen_ralink_config")) {
		return gen_ralink_config_wl(0);
	}
	else if (!strcmp(base, "gen_ralink_config_rt")) {
		return gen_ralink_config_rt(0);
	}
	else if (!strcmp(base, "restart_dns"))
	{
		restart_dns();
		return 0;
	}
	else if (!strcmp(base, "restart_dhcpd"))
	{
		restart_dhcpd();
		return 0;
	}
	else if (!strcmp(base, "restart_networkmap"))
	{
		restart_networkmap();
		return 0;
	}
	else if (!strcmp(base, "start_telnetd"))
	{
		start_telnetd();
		return 0;
	}
	else if (!strcmp(base, "run_telnetd"))
	{
		run_telnetd();
		return 0;
	}
	else if (!strcmp(base, "restart_firewall"))
	{
		restart_firewall();
		return 0;
	}
	else if (!strcmp(base, "ejusb"))
	{
		return safe_remove_usb_mass( (argc > 1) ? atoi(argv[1]) : 0 );
	}
	else if (!strcmp(base, "ejusb1"))
	{
		return safe_remove_usb_mass(1);
	}
	else if (!strcmp(base, "ejusb2"))
	{
		return safe_remove_usb_mass(2);
	}
	else if (!strcmp(base, "pids"))
	{
		pids_main(argv[1]);
		return 0;
	}

	printf("Unknown applet: %s\n", base);

	return EINVAL;
}


