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

void
stop_syslogd()
{
	char* svcs[] = { "syslogd", NULL };
	kill_services(svcs, 3, 1);
}

void
stop_klogd()
{
	char* svcs[] = { "klogd", NULL };
	kill_services(svcs, 3, 1);
}

int
start_syslogd()
{
	setenv_tz();
	
	if (nvram_invmatch("log_ipaddr", ""))
	{
		return eval("/sbin/syslogd", "-b0", "-s256", "-S", "-O", "/tmp/syslog.log", "-R", nvram_safe_get("log_ipaddr"), "-L");
	}
	else
	{
		return eval("/sbin/syslogd", "-b0", "-s256", "-S", "-D", "-O", "/tmp/syslog.log");
	}
}

int
start_klogd()
{
	return eval("/sbin/klogd");
}

void
stop_infosvr(void)
{
	char* svcs[] = { "infosvr", NULL };
	kill_services(svcs, 3, 1);
}

void
start_infosvr(void)
{
	eval("/usr/sbin/infosvr", IFNAME_BR);
}

int
start_networkmap(int first_call)
{
	return eval("/usr/sbin/networkmap", (first_call) ? "-w" : "");
}

void
stop_networkmap(void)
{
	char* svcs[] = { "networkmap", NULL };
	kill_services(svcs, 3, 1);
}

void
restart_networkmap(void)
{
	if (pids("networkmap"))
		doSystem("killall %s %s", "-SIGUSR1", "networkmap");
	else
		start_networkmap(0);
}

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
start_logger(int showinfo)
{
	start_syslogd();

	if (showinfo)
	{
		// wait for logger daemon started
		usleep(250000);
		
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

int
start_services_once(int is_ap_mode)
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

	if (!is_ap_mode) {
		if (!is_upnp_run())
			start_upnp();
		
		if (!nvram_match("lan_stp", "0")) {
			doSystem("brctl stp %s %d", IFNAME_BR, 1);
			doSystem("brctl setfd %s %d", IFNAME_BR, 15);
		}
	}

	start_lltd();
	start_rstats();
	start_watchdog_cpu();
	start_networkmap(1);

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
		stop_httpd();
		stop_vpn_server();
	}
#if (BOARD_NUM_USB_PORTS > 0)
	stop_p910nd();
#if defined(SRV_LPRD)
	stop_lpd();
#endif
#if defined(SRV_U2EC)
	stop_u2ec();
#endif
#endif
	stop_networkmap();
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
stop_misc(void)
{
	char* svcs[] = {"ntpd",
			"tcpcheck",
			"detect_wan",
			"watchdog",
			NULL
			};

	kill_services(svcs, 3, 1);
}
