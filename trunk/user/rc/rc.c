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
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
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
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/vfs.h>

#include <shutils.h>
#include <nvram/typedefs.h>
#include <nvram/bcmnvram.h>
#include <nvparse.h>
#include "rtl8367m.h"
#include <ralink.h>
#include <fcntl.h>
#include <notify_rc.h>
#include <semaphore_mfp.h>
#include <linux/autoconf.h>

#include "rc.h"
#include "rc_event.h"


extern struct nvram_tuple router_defaults[];

static int noconsole = 0;

static const char *const environment[] = {
	"HOME=/",
	"PATH=/sbin:/usr/sbin:/bin:/usr/bin",
	"SHELL=/bin/sh",
	"USER=admin",
	NULL
};

void sys_exit(void)
{
	kill(1, SIGTERM);
}

void setenv_tz(void)
{
	static char TZ_env[64];

	snprintf(TZ_env, sizeof(TZ_env), "TZ=%s", nvram_safe_get("time_zone_x"));
	TZ_env[sizeof(TZ_env)-1] = '\0';
	putenv(TZ_env);
}


void
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

	/* Commit values */
	if (restore_defaults) {
		/* default value of vlan */
		nvram_commit();
	}

	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
}

static void
set_wan0_vars(void)
{
	struct nvram_tuple *t;
	char *v;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	
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
insertmodules(void)
{
	system("modprobe -q ehci-hcd");
	system("modprobe -q ohci-hcd");
	system("modprobe -q rt2860v2_ap");
	system("modprobe -q rt3090_ap");
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

/* States */
enum {
	IDLE,
	TIMER,
	SERVICE,
};

static int state = IDLE;
static int signalled = -1;

/* Signal handling */
static void
rc_signal(int sig)
{
	dbG("[init] catch signal: %d\n", sig);
	
	if (sig == SIGHUP) {
		;
	} else if (sig == SIGUSR1) {
		signalled = SERVICE;
	} else if (sig == SIGUSR2) {
		;
	} else if (sig == SIGALRM) {
		signalled = TIMER;
	} else if (sig == SIGINT) {
		;
	}
}

/* Get the timezone from NVRAM and set the timezone in the kernel
 * and export the TZ variable 
 */
void
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

void 
stop_watchdog(void)
{
	system("killall watchdog");
}

int 
start_watchdog(void)
{
	return eval("watchdog");
}

void
stop_detect_internet(void)
{
	system("killall -q detect_internet");
}

int
start_detect_internet(void)
{
        char *detect_internet_argv[] = {"detect_internet", NULL};
        pid_t dipid;

        stop_detect_internet();

	if (!nvram_match("wan_route_x", "IP_Routed"))
		return 1;

        return _eval(detect_internet_argv, NULL, 0, &dipid);
}

void
start_flash_usbled(void)
{
	if (pids("linkstatus_monitor"))
	{
		system("killall -SIGUSR1 linkstatus_monitor");
	}
}

void
stop_flash_usbled(void)
{
	if (pids("linkstatus_monitor"))
	{
		system("killall -SIGUSR2 linkstatus_monitor");
	}
}

void reload_nat_modules(void)
{
	int loaded_ftp;
	int needed_ftp = 0;
	int needed_sip = 0;
	int needed_h323 = 0;
	int needed_pptp = 0;
	int hwnat_allow = is_hwnat_allow();
	int hwnat_loaded = is_hwnat_loaded();
	
	if (nvram_match("wan_route_x", "IP_Routed"))
	{
		needed_ftp = atoi(nvram_safe_get("nf_alg_ftp1"));
		if (needed_ftp < 1024 || needed_ftp > 65535) needed_ftp = 21;
		
		if (nvram_match("nf_alg_pptp", "1"))
			needed_pptp = 1;
		
		if (nvram_match("nf_alg_h323", "1"))
			needed_h323 = 1;
		
		if (nvram_match("nf_alg_sip", "1"))
			needed_sip = 1;
	}
	
	if ((hwnat_loaded) && ((!hwnat_allow) || (hwnat_loaded != hwnat_allow)))
	{
		hwnat_loaded = 0;
		hwnat_unload();
	}
	
	if (needed_pptp)
	{
		if (nvram_match("wan_nat_x", "0"))
			system("modprobe -q nf_conntrack_pptp");
		else
			system("modprobe -q nf_nat_pptp");
	}
	else
		system("modprobe -r nf_nat_pptp");
	
	if (needed_h323)
	{
		if (nvram_match("wan_nat_x", "0"))
			system("modprobe -q nf_conntrack_h323");
		else
			system("modprobe -q nf_nat_h323");
	}
	else
		system("modprobe -r nf_nat_h323");
	
	if (needed_sip)
	{
		if (nvram_match("wan_nat_x", "0"))
			system("modprobe -q nf_conntrack_sip");
		else
			system("modprobe -q nf_nat_sip");
	}
	else
		system("modprobe -r nf_nat_sip");
	
	loaded_ftp = is_ftp_conntrack_loaded(needed_ftp);
	if (loaded_ftp == 1)
	{
		system("rmmod nf_nat_ftp 2>/dev/null");
		system("rmmod nf_conntrack_ftp 2>/dev/null");
	}
	
	if (needed_ftp && loaded_ftp != 2)
	{
		if (needed_ftp != 21)
			doSystem("modprobe -q nf_conntrack_ftp ports=21,%d", needed_ftp);
		else
			system("modprobe -q nf_conntrack_ftp");
		
		if (nvram_invmatch("wan_nat_x", "0"))
			system("modprobe -q nf_nat_ftp");
	}
	
	if (hwnat_allow && !hwnat_loaded)
	{
		hwnat_load();
	}
}

void rc_restart_firewall(void)
{
	char wan_ifname[16];
	
	wan_ifname[0] = 0;
	strncpy(wan_ifname, nvram_safe_get("wan_ifname_t"), sizeof(wan_ifname));
	if (strlen(wan_ifname) == 0) {
		get_wan_ifname(wan_ifname);
	}
	
	start_firewall_ex(wan_ifname, nvram_safe_get("wan0_ipaddr"), IFNAME_BR, nvram_safe_get("lan_ipaddr"));
	
	/* update upnp forwards from lease file */
	update_upnp(0);
}

// 2008.08 magic {
static void handle_notifications(void)
{
	int i, stop_handle = 0;
	DIR *directory = opendir("/tmp/rc_notification");
	if (!directory)
		return;
	
	// handle max 20 requests at once (prevent deadlock)
	for (i=0; i < 20; i++)
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
			dbg("Error: Failed trying to allocate %lu bytes of memory for "
			    "the full name of an rc notification marker file.\n",
			    (unsigned long)(strlen(entry->d_name) + 100));
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
			shutdown_prepare();
		}
		else if (!strcmp(entry->d_name, "restart_whole_wan"))
		{
			full_restart_wan();
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
			stop_ftp();
			stop_samba();
			run_samba();
			run_ftp();
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
			stop_ddns();
			if (nvram_match("wan_route_x", "IP_Routed"))
				start_ddns(1);
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
			rc_restart_firewall();
		}
		else if (strcmp(entry->d_name, "restart_ntpc") == 0)
		{
			refresh_ntpc();
		}
		else if (strcmp(entry->d_name, "ddns_update") == 0)
		{
//			update_ddns_changes();
		}
		else if (strcmp(entry->d_name, "restart_time") == 0)
		{
			set_timezone();
			stop_logger();
			start_logger(0);
			refresh_ntpc();
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
			int radio_on = atoi(nvram_safe_get("wl_radio_x"));
			restart_wifi_wl(radio_on, 1);
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_wl"))
		{
			control_wifi_guest_wl();
		}
		else if (!strcmp(entry->d_name, "restart_wifi_rt"))
		{
			int radio_on = atoi(nvram_safe_get("rt_radio_x"));
			restart_wifi_rt(radio_on, 1);
		}
		else if (!strcmp(entry->d_name, "control_wifi_guest_rt"))
		{
			control_wifi_guest_rt();
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


int shutdown_prepare(void)
{
	stop_misc_no_watchdog();
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
	
	return 0;
}

void convert_misc_values()
{
	nvram_set("run_sh", "off");

	if (!strcmp(nvram_safe_get("wl_ssid"), ""))
		nvram_set("wl_ssid", "ASUS_5G");

	if (!strcmp(nvram_safe_get("rt_ssid"), ""))
		nvram_set("rt_ssid", "ASUS");
		

//	if (strcmp(nvram_safe_get("wl_ssid"), nvram_safe_get("wl_ssid2"))) {
		char buff[100];
		memset(buff, 0, 100);
		char_to_ascii(buff, nvram_safe_get("wl_ssid"));
		nvram_set("wl_ssid2", buff);
//	}

//	if (strcmp(nvram_safe_get("rt_ssid"), nvram_safe_get("rt_ssid2"))) {
		char buff2[100];
		memset(buff2, 0, 100);
		char_to_ascii(buff2, nvram_safe_get("rt_ssid"));
		nvram_set("rt_ssid2", buff2);
//	}

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

#if defined (W7_LOGO) || defined (WIFI_LOGO)
	nvram_set("wan_proto", "static");
	nvram_set("wan0_proto", "static");
	nvram_set("wan_ipaddr", "17.1.1.1");
	nvram_set("wan0_ipaddr", "17.1.1.1");
	nvram_set("wanx_ipaddr", "17.1.1.1");
	nvram_set("wan_ipaddr_t", "17.1.1.1");
	nvram_set("wan_gateway", "17.1.1.1");
	nvram_set("wan0_gateway", "17.1.1.1");
	nvram_set("wanx_gateway", "17.1.1.1");
	nvram_set("wan_gateway_t", "17.1.1.1");
	nvram_set("wan_netmask", "255.0.0.0");
	nvram_set("wan0_netmask", "255.0.0.0");
	nvram_set("wanx_netmask", "255.0.0.0");
	nvram_set("wan_netmask_t", "255.0.0.0");
#else
	nvram_unset("wanx_ipaddr"); 
	nvram_unset("wanx_netmask");
	nvram_unset("wanx_gateway");
	nvram_unset("wanx_dns");
	nvram_unset("wanx_lease");
#endif
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

extern int stop_service_type_99;

static void 
init_router_control(void)
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
	
	start_linkstatus_monitor();
	config_loopback();
	bridge_init();
	start_lan();
	start_dns_dhcpd();
	load_usb_printer_module();
	
	if (log_remote)
	{
		start_logger(1);
	}
	
	default_filter_setting();
	default_nat_setting();
	if (nvram_match("modem_arun", "1"))
		select_usb_modem_to_wan(5);
	start_wan();
	load_usb_storage_module();
	start_services();
}

/* Main loop */
static void
main_loop(void)
{
	pid_t shell_pid = 0;
	sigset_t sigset;
	
	/* Basic initialization */
	umask(0000);
	system("dev_init.sh");
	
	/* Setup console */
	if (console_init())
		noconsole = 1;
	
	chdir("/");
	setsid();
	
	{
		const char *const *e;
		/* Make sure environs is set to something sane */
		for (e = environment; *e; e++)
			putenv((char *) *e);
	}
	
	spinlock_init(SPINLOCK_SiteSurvey);
	spinlock_init(SPINLOCK_NVRAMCommit);
	spinlock_init(SPINLOCK_DNSRenew);
	spinlock_init(SPINLOCK_Networkmap);
	
	/* Setup signal handlers */
	signal_init();
	signal(SIGHUP,  rc_signal);
	signal(SIGUSR1, rc_signal);
	signal(SIGUSR2, rc_signal);
	signal(SIGINT,  rc_signal);
	signal(SIGALRM, rc_signal);
	sigemptyset(&sigset);
	
	init_router_control();
	
	/* Loop forever */
	for (;;) {
		switch (state) {
		case SERVICE:
			handle_notifications();
			state = IDLE;
			break;
		case TIMER:
			on_deferred_hotplug_usb();
			state = IDLE;
			break;
		case IDLE:
			/* Wait for user input or state change */
			while (signalled == -1) {
				if (!noconsole && (!shell_pid || kill(shell_pid, 0) != 0))
				{
					shell_pid = run_shell(0, 1);
				}
				else
				{
					sigsuspend(&sigset);
				}
			}
			state = signalled;
			signalled = -1;
			break;
		default:
			return;
		}
	}
}

int
main(int argc, char **argv)
{
	char *base = strrchr(argv[0], '/');
	base = base ? base + 1 : argv[0];

	/* init */
	if (!strcmp(base, "init")) {
		main_loop();
		return 0;
	}
	
	/* Set TZ for all rc programs */
	setenv_tz();
	
	if (!strcmp(base, "nvram_restore")) {
		nvram_restore_defaults();
		return 0;
	}
	else if (!strcmp(base, "nvram_erase")) {
		erase_nvram();
		return 0;
	}
	else if (!strcmp(base, "reset_to_defaults")) {
		erase_nvram();
		erase_storage();
		sys_exit();
		return 0;
	}
	else if (!strcmp(base, "watchdog")) {
		return watchdog_main(argc, argv);
	}
	else if (!strcmp(base, "udhcpc.script")) {
		return udhcpc_main(argc, argv);
	}
	else if (!strcmp(base, "zcip.script")) {
		return zcip_main(argc, argv);
	}
	else if (!strcmp(base, "wpacli.script")) {
		return wpacli_main(argc, argv);
	}
	else if (!strcmp(base, "hotplug")) {
		// stub for early kernel hotplug
		return 0;
	}
	else if (!strcmp(base, "shutdown")) {
		notify_rc("shutdown_prepare");
		return 0;
	}
	else if (!strcmp(base, "halt")) {
		notify_rc("shutdown_prepare");
		return 0;
	}
	else if (!strcmp(base, "reboot")) {
		return kill(1, SIGTERM);
	}
	else if (!strcmp(base, "run_ftpsamba")) {
		stop_ftp();
		stop_samba();
		run_samba();
		run_ftp();
		return 0;
	}
	else if (!strcmp(base, "run_samba")) {
		stop_samba();
		run_samba();
		return 0;
	}
	else if (!strcmp(base, "run_ftp")) {
		stop_ftp();
		run_ftp();
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
		stop_torrent(0);
		return 0;
	}
	/* ddns update ok */
	else if (!strcmp(base, "stopservice")) {
		if (argc >= 2)
			return stop_service_main(atoi(argv[1]));
		else
			return stop_service_main(0);
	}
	/* ddns update ok */
	else if (!strcmp(base, "ddns_updated")) {
		return ddns_updated_main(argc, argv);
	}
	/* ddns update ok */
	else if (!strcmp(base, "start_ddns")) {
		return start_ddns(0);
	}
	/* run ntp client */
	else if (!strcmp(base, "ntp")) {
		return ntp_main(argc, argv);
	}
	else if (!strcmp(base, "refresh_ntpc")) {
		refresh_ntpc();
	}
	else if (!strcmp(base, "gpio_write")) {
		if (argc >= 3)
			cpu_gpio_write(atoi(argv[1]), atoi(argv[2]));
		return 0;
	}
	else if (!strcmp(base, "landhcpc")) {
		return udhcpc_ex_main(argc, argv);
	}
	else if (!strcmp(base, "rc")) {
		dbg("error: cannot run rc directly!\n");
		return EINVAL;
	}
	else if (!strcmp(base, "getMAC") || !strcmp(base, "getMAC_5G")) {
		return getMAC();
	}
	else if (!strcmp(base, "setMAC") || !strcmp(base, "setMAC_5G")) {
		if (argc == 2)
			return setMAC(argv[1]);
		else
			return EINVAL;
	}
	else if (!strcmp(base, "getMAC_2G")) {
		return getMAC_2G();
	}
	else if (!strcmp(base, "setMAC_2G")) {
		if (argc == 2)
			return setMAC_2G(argv[1]);
		else
			return EINVAL;
	}
	else if (!strcmp(base, "FWRITE")) {
		if (argc == 3)
			return FWRITE(argv[1], argv[2]);
		else
		return EINVAL;
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
	else if (!strcmp(base, "getPIN")) {
		return getPIN();
	}
	else if (!strcmp(base, "setPIN")) {
		if (argc == 2)
			return setPIN(argv[1]);
		else
			return EINVAL;
	}
	else if (!strcmp(base, "slink")) {
		return symlink(argv[1], argv[2]);
	}
	else if (!strcmp(base, "getSSID")) {
		return getSSID();
	}
	else if (!strcmp(base, "getChannel")) {
		return getChannel();
	}
	else if (!strcmp(base, "getChannel_2g")) {
		return getChannel_2G();
	}
	else if (!strcmp(base, "getSiteSurvey")) {
		return getSiteSurvey();
	}
	else if (!strcmp(base, "getSiteSurvey_2g")) {
		return getSiteSurvey_2G();
	}
	else if (!strcmp(base, "getBSSID")) {
		return getBSSID();
	}
	else if (!strcmp(base, "getBootV")) {
		return getBootVer();
	}
	else if (!strcmp(base, "asuscfe") || !strcmp(base, "asuscfe_5g")) {
		if (argc == 2)
			return asuscfe(argv[1], WIF5G);
		else
			return EINVAL;
	}
	else if (!strcmp(base, "asuscfe_2g")) {
		if (argc == 2)
			return asuscfe(argv[1], WIF2G);
		else
			return EINVAL;
	}
	else if (!strcmp(base, "start_mac_clone")) {
		start_mac_clone();
		return 0;
	}
	/* ppp */
	else if (!strcmp(base, "ip-up"))
		return ipup_main(argc, argv);
	else if (!strcmp(base, "ip-down"))
		return ipdown_main(argc, argv);
	else if (!strcmp(base, "wan-up"))
		return ipup_main(argc, argv);
	else if (!strcmp(base, "wan-down"))
		return ipdown_main(argc, argv);
	else if (!strcmp(base, "restart_dns"))
	{
		dbg("rc restart_dns\n");
		restart_dns();
		
		return 0;
	}
	else if (!strcmp(base, "restart_dhcpd"))
	{
		dbg("rc restart_dhcpd\n");
		restart_dhcpd();
		return 0;
	}
	else if (!strcmp(base, "restart_networkmap"))
	{
		restart_networkmap();
		return 0;
	}
	else if (!strcmp(base, "convert_asus_values"))
	{
		convert_asus_values(1);
		
		return 0;
	}
	else if (!strcmp(base, "umount2"))
	{
		umount2(argv[1], 0x00000002);	// MNT_DETACH
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
	else if (!strcmp(base, "start_telnetd"))
	{
		start_telnetd();
		return 0;
	}
	else if (!strcmp(base, "restart_firewall"))
	{
		rc_restart_firewall();
		return 0;
	}
	else if (!strcmp(base, "run_telnetd"))
	{
		run_telnetd();
		return 0;
	}
	else if (!strcmp(base, "start_ntp"))
	{
		stop_ntpc();
		start_ntpc();
		return 0;
	}
	else if (!strcmp(base, "get_sw"))
	{
		printf("sw mode is %s", nvram_safe_get("sw_mode"));
		return 0;
	}
	else if (!strcmp(base, "ledoff"))
	{
		LED_CONTROL(LED_POWER, LED_OFF);
		return 0;
	}
	else if (!strcmp(base, "ledon"))
	{
		LED_CONTROL(LED_POWER, LED_ON);
		return 0;
	}
	else if (!strcmp(base, "rtl8367m"))
	{
		return rtl8367m_main(argc, argv);
	}
	else if (!strcmp(base, "umount_dev_all"))
	{
		if (argc == 2)
			umount_dev_all(argv[1]);
                return 0;
	}
	else if (!strcmp(base, "dumparptable"))
	{
		dumparptable();
		return 0;
	}
	else if (!strcmp(base, "linkstatus_monitor")) {
		return linkstatus_monitor_main(argc, argv);
	}
	else if (!strcmp(base, "detect_internet")) {
		return detect_internet_main(argc, argv);
	}
	else if (!strcmp(base, "detect_wan")) {
		return detect_wan_main(argc, argv);
	}
	else if (!strcmp(base, "stainfo")) {
		return stainfo();
	}
	else if (!strcmp(base, "stainfo_2g")) {
		return stainfo_2g();
	}
	else if (!strcmp(base, "getstat")) {
		return getstat();
	}
	else if (!strcmp(base, "getstat_2g")) {
		return getstat_2g();
	}
	else if (!strcmp(base, "getrssi") || !strcmp(base, "getrssi_5g")) {
		return getrssi();
	}
	else if (!strcmp(base, "getrssi_2g")) {
		return getrssi_2g();
	}
	else if (!strcmp(base, "gettxbfcal")) {
		return gettxbfcal();
	}
	else if(!strcmp(base, "asus_lp")){
		if(argc != 3){
			printf("Usage: asus_lp [device_name] [action]\n");
			return 0;
		}
		return asus_lp(argv[1], argv[2]);
	}
	else if(!strcmp(base, "asus_sd")){
		if(argc != 3){
			printf("Usage: asus_sd [device_name] [action]\n");
			return 0;
		}
		return asus_sd(argv[1], argv[2]);
	}
	else if(!strcmp(base, "asus_sg")){
		if(argc != 3){
			printf("Usage: asus_sg [device_name] [action]\n");
			return 0;
		}
		return asus_sg(argv[1], argv[2]);
	}
	else if(!strcmp(base, "asus_sr")){
		if(argc != 3){
			printf("Usage: asus_sr [device_name] [action]\n");
			return 0;
		}
		return asus_sr(argv[1], argv[2]);
	}
	else if(!strcmp(base, "asus_tty")){
		if(argc != 3){
			printf("Usage: asus_tty [device_name] [action]\n");
			return 0;
		}
		return asus_tty(argv[1], argv[2]);
	}
	else if(!strcmp(base, "asus_net")){
		if(argc != 3){
			printf("Usage: asus_net [device_name] [action]\n");
			return 0;
		}
		return asus_net(argv[1], argv[2]);
	}
	else if(!strcmp(base, "asus_usb_interface")){
		if(argc != 3){
			printf("Usage: asus_usb_interface [device_name] [action]\n");
			return 0;
		}

		return asus_usb_interface(argv[1], argv[2]);
	}
	return EINVAL;
}

