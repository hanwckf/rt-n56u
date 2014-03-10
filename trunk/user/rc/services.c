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
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <nvram/bcmnvram.h>

#include "rc.h"

#define UPNPD_LEASE_FILE	"/tmp/miniupnpd.leases"

void 
stop_telnetd(void)
{
	char* svcs[] = { "telnetd", NULL };
	kill_services(svcs, 3, 1);
}

void 
run_telnetd(void)
{
	stop_telnetd();
	
	eval("telnetd");
}

void 
start_telnetd(void)
{
	if (!nvram_match("telnetd", "1"))
	{
		stop_telnetd();
	}
	else if (!pids("telnetd"))
	{
		eval("telnetd");
	}
}

#if defined(APP_SSHD)
int 
is_sshd_run(void)
{
	if (check_if_file_exist("/usr/bin/dropbearmulti"))
	{
		if (pids("dropbear"))
			return 1;
	}
	else if (check_if_file_exist("/usr/sbin/sshd"))
	{
		if (pids("sshd"))
			return 1;
	}
	
	return 0;
}

void 
stop_sshd(void)
{
	eval("/usr/bin/sshd.sh", "stop");
}

void 
start_sshd(void)
{
	static int sshd_mode_last = 0;

	int sshd_mode = nvram_get_int("sshd_enable");
	if (!sshd_mode || sshd_mode != sshd_mode_last)
	{
		stop_sshd();
	}

	if (sshd_mode != sshd_mode_last || !is_sshd_run())
	{
		if (sshd_mode == 2)
			eval("/usr/bin/sshd.sh", "start", "-s");
		else if (sshd_mode == 1)
			eval("/usr/bin/sshd.sh", "start");
	}
	
	sshd_mode_last = sshd_mode;
}
#endif

void 
restart_term(void)
{
#if defined(APP_SSHD)
	int is_run_before = is_sshd_run();
	int is_run_after;

	start_sshd();
#endif
	start_telnetd();

#if defined(APP_SSHD)
	is_run_after = is_sshd_run();

	if (is_run_after && !is_run_before && nvram_match("sshd_wopen", "1") && nvram_match("fw_enable_x", "1"))
		restart_firewall();
#endif
}

void
start_httpd(int restart_fw)
{
	int http_proto, argv_index, http_port, restart_fw_need;
	char http_port_s[8];
#if defined (SUPPORT_HTTPS)
	char https_port_s[8];
#endif
	char *httpd_argv[] = {
		"/usr/sbin/httpd",
		NULL, NULL,
		NULL, NULL,
		NULL,
	};

	argv_index = 1;
	http_port = 0;
	restart_fw_need = 0;

#if defined (SUPPORT_HTTPS)
	http_proto = nvram_get_int("http_proto");
#else
	http_proto = 0;
#endif
	if (http_proto == 0 || http_proto == 2) {

		http_port = nvram_get_int("http_lanport");
		if (http_port < 80 || http_port > 65535) {
			http_port = 80;
			nvram_set_int("http_lanport", http_port);
		}
		
		sprintf(http_port_s, "%d", http_port);
		
		httpd_argv[argv_index++] = "-p";
		httpd_argv[argv_index++] = http_port_s;
		
		restart_fw_need |= nvram_get_int("misc_http_x");
	}

#if defined (SUPPORT_HTTPS)
	if (http_proto == 1 || http_proto == 2) {
		int https_port = nvram_get_int("https_lport");
		if (https_port < 81 || https_port > 65535 || https_port == http_port) {
			https_port = 443;
			nvram_set_int("https_lport", https_port);
		}
		
		sprintf(https_port_s, "%d", https_port);
		
		httpd_argv[argv_index++] = "-s";
		httpd_argv[argv_index++] = https_port_s;
		
		restart_fw_need |= nvram_get_int("https_wopen");
	}
#endif

	_eval(httpd_argv, NULL, 0, NULL);

	if (restart_fw && restart_fw_need && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}

void
stop_httpd(void)
{
	char* svcs[] = { "httpd", NULL };
	kill_services(svcs, 3, 1);
}

void 
restart_httpd(void)
{
	stop_httpd();
	start_httpd(1);
}

void
stop_rstats(void)
{
	char* svcs[] = { "rstats", NULL };
	kill_services(svcs, 3, 1);
}

void
start_rstats(void)
{
	if (!get_ap_mode() && nvram_match("rstats_enable", "1")) {
		eval("/sbin/rstats");
	}
}

int
is_upnp_run(void)
{
	return pids("miniupnpd");
}

int
start_upnp(void)
{
	FILE *fp;
	int ret, i_proto_use, i_clean_min, i_clean_int, i_iports[2], i_eports[2];
	char *lan_addr, *lan_mask, *lan_url, *proto_upnp, *proto_npmp, *secured;
	char var[100];
	char wan_ifname[16];
	char lan_class[32];
	uint8_t lan_mac[16];

	if (!nvram_get_int("upnp_enable_x") || !nvram_get_int("wan_nat_x") || get_ap_mode())
		return 0;

	i_proto_use = nvram_get_int("upnp_proto");
	i_clean_min = nvram_safe_get_int("upnp_clean_min", 10, 1, 999);
	i_clean_int = nvram_safe_get_int("upnp_clean_int", 600, 0, 86400);
	i_iports[0] = nvram_safe_get_int("upnp_iport_min", 21, 1, 65535);
	i_iports[1] = nvram_safe_get_int("upnp_iport_max", 65535, 1, 65535);
	i_eports[0] = nvram_safe_get_int("upnp_eport_min", 21, 1, 65535);
	i_eports[1] = nvram_safe_get_int("upnp_eport_max", 65535, 1, 65535);

	if (i_iports[0] > i_iports[1])
		i_iports[0] = i_iports[1];
	if (i_eports[0] > i_eports[1])
		i_eports[0] = i_eports[1];

	wan_ifname[0] = 0;
	get_wan_ifname(wan_ifname);

	lan_addr = nvram_safe_get("lan_ipaddr");
	lan_mask = nvram_safe_get("lan_netmask");
	ip2class(lan_addr, lan_mask, lan_class);
	memset(lan_mac, 0, sizeof(lan_mac));
	ether_atoe(nvram_safe_get("lan_hwaddr"), lan_mac);

	lan_url = lan_addr;
	ret = nvram_get_int("http_lanport");
	if (ret && ret != 80) {
		sprintf(var, "%s:%d", lan_addr, ret);
		lan_url = var;
	}

	if (i_proto_use == 2) {
		proto_upnp = "yes";
		proto_npmp = "yes";
	} else if (i_proto_use == 1) {
		proto_upnp = "no";
		proto_npmp = "yes";
	} else {
		proto_upnp = "yes";
		proto_npmp = "no";
	}

	secured = (nvram_get_int("upnp_secure")) ? "yes" : "no";

	/* Write configuration file */
	if (!(fp = fopen("/etc/miniupnpd.conf", "w"))) {
		return errno;
	}

	fprintf(fp, "# automagically generated\n"
		"ext_ifname=%s\n"
		"listening_ip=%s/%s\n"
		"port=%d\n"
		"enable_upnp=%s\n"
		"enable_natpmp=%s\n"
		"upnp_forward_chain=%s\n"
		"upnp_nat_chain=%s\n"
		"secure_mode=%s\n"
		"lease_file=%s\n"
		"presentation_url=http://%s/\n"
		"system_uptime=yes\n"
		"notify_interval=%d\n"
		"clean_ruleset_interval=%d\n"
		"clean_ruleset_threshold=%d\n"
		"uuid=75802409-bccb-40e7-8e6c-%02x%02x%02x%02x%02x%02x\n"
		"friendly_name=%s\n"
		"manufacturer_name=%s\n"
		"manufacturer_url=%s\n"
		"model_name=%s\n"
		"model_description=%s\n"
		"model_url=%s\n"
		"model_number=%s\n"
		"serial=%s\n"
		"bitrate_up=%d\n"
		"bitrate_down=%d\n"
		"allow %d-%d %s %d-%d\n"
		"deny 0-65535 0.0.0.0/0 0-65535\n",
		wan_ifname,
		lan_addr, lan_mask, /*IFNAME_BR,*/
		0,
		proto_upnp,
		proto_npmp,
		MINIUPNPD_CHAIN_IP4_FORWARD,
		MINIUPNPD_CHAIN_IP4_NAT,
		secured,
		UPNPD_LEASE_FILE,
		lan_url,
		60,
		i_clean_int,
		i_clean_min,
		lan_mac[0], lan_mac[1], lan_mac[2], lan_mac[3], lan_mac[4], lan_mac[5],
		BOARD_DESC,
		BOARD_VENDOR_NAME,
		BOARD_VENDOR_URL,
		"Wireless Router",
		BOARD_DESC,
		BOARD_MODEL_URL,
		BOARD_NAME,
		"1.0",
		100000000,
		100000000,
		i_eports[0], i_eports[1], lan_class, i_iports[0], i_iports[1]);

	fclose(fp);

	return eval("/usr/bin/miniupnpd");
}

void
stop_upnp(void)
{
	char* svcs[] = { "miniupnpd", NULL };
	kill_services(svcs, 3, 1);
}

void
smart_restart_upnp(void)
{
	char wan_ifname[16];
	
	if (!is_upnp_run())
	{
		start_upnp();
		
		return;
	}
	
	wan_ifname[0] = 0;
	get_wan_ifname(wan_ifname);
	
	/* restart miniupnpd only if wan interface changed */
	if (strcmp(wan_ifname, nvram_safe_get("wan_ifname_t")) != 0) {
		stop_upnp();
		start_upnp();
	}
}

void
update_upnp(void)
{
	if (!is_upnp_run())
	{
		start_upnp();
		
		return;
	}
	
	/* update upnp forwards from lease file */
	if (check_if_file_exist(UPNPD_LEASE_FILE)) {
		doSystem("killall %s %s", "-SIGUSR1", "miniupnpd");
	}
}

int start_lltd(void)
{
	if (pids("lld2d"))
		return 0;

	return eval("lld2d", IFNAME_BR);
}

void stop_lltd(void)
{
	char* svcs[] = { "lld2d", NULL };
	kill_services(svcs, 2, 1);
}

int
start_services_once(void)
{
	start_8021x_wl();
	start_8021x_rt();
	start_detect_internet();
	start_httpd(0);
	start_telnetd();
#if defined(APP_SSHD)
	start_sshd();
#endif
	start_vpn_server();
	start_watchdog();
	start_infosvr();

	if (!get_ap_mode() && !nvram_match("lan_stp", "0"))
	{
		doSystem("brctl stp %s %d", IFNAME_BR, 1);
		doSystem("brctl setfd %s %d", IFNAME_BR, 15);
	}

	start_lltd();

	start_rstats();

	start_watchdog_cpu();

	start_networkmap();

	return 0;
}

void
stop_services(int stopall)
{
	if (stopall) {
		stop_telnetd();
#if defined(APP_SSHD)
		stop_sshd();
#endif
		stop_vpn_server();
	}
	stop_p910nd();
#if defined(SRV_LPRD)
	stop_lpd();
#endif
#if defined(SRV_U2EC)
	stop_u2ec();
#endif
	stop_lltd();
	stop_detect_internet();
	stop_rstats();
	stop_infosvr();
#if defined(APP_XUPNPD)
	stop_xupnpd();
#endif
}


void
stop_services_lan_wan(void)
{
	stop_dns_dhcpd();
	stop_upnp();
	stop_detect_link();
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
	doSystem("/bin/mtd_write %s %s", "erase", "Config");
}

int 
start_logger(int showinfo)
{
	start_syslogd();
	
	if (showinfo)
	{
		// wait for logger daemon started
		sleep(1);
		
		logmessage(LOGNAME, "bootloader version: %s", nvram_safe_get("blver"));
		logmessage(LOGNAME, "firmware version: %s", nvram_safe_get("firmver_sub"));
	}
	
	start_klogd();
	
	return 0;
}

void
stop_logger(void)
{
	char* svcs[] = { "klogd", "syslogd", NULL };
	kill_services(svcs, 3, 1);
}

void
set_pagecache_reclaim(void)
{
	int pagecache_ratio = 100;
	int pagecache_reclaim = nvram_get_int("pcache_reclaim");

	if (pagecache_reclaim == 1)
		pagecache_ratio = 50;
	else if (pagecache_reclaim == 2)
		pagecache_ratio = 30;

	fput_int("/proc/sys/vm/pagecache_ratio", pagecache_ratio);
}

void
start_watchdog_cpu(void)
{
	if (nvram_get_int("watchdog_cpu") != 0)
		module_smart_load("rt_timer_wdg", NULL);
}

void
restart_watchdog_cpu(void)
{
	if (nvram_get_int("watchdog_cpu") == 0)
		module_smart_unload("rt_timer_wdg", 0);
	else
		module_smart_load("rt_timer_wdg", NULL);
}

