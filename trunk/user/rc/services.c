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
 * Miscellaneous services
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <nvram/bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <semaphore_mfp.h>

#include "rc.h"

#define UPNPD_LEASE_FILE	"/etc/storage/miniupnpd.leases"

extern int stop_service_type_99;

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

void 
stop_sshd(void)
{
	eval("/usr/bin/sshd.sh", "stop");
}

static int dropbear_mode_last = 0;

void 
start_sshd(void)
{
	int dropbear_mode = atoi(nvram_safe_get("sshd_enable"));
	if (!dropbear_mode || dropbear_mode != dropbear_mode_last)
	{
		stop_sshd();
	}
	
	if (dropbear_mode == 2)
	{
		eval("/usr/bin/sshd.sh", "start", "-s");
	}
	else if (dropbear_mode == 1)
	{
		eval("/usr/bin/sshd.sh", "start");
	}
	
	dropbear_mode_last = dropbear_mode;
}

void 
restart_term(void)
{
	start_telnetd();
	start_sshd();
}

int
start_httpd(void)
{
	// daemon mode
	return eval("httpd");
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
	start_httpd();
}

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
//////////vvvvvvvvvvvvvvvvvvvvvjerry5 2009.07
void
stop_rstats(void)
{
	if (pids("rstats"))
		system("killall rstats");
}

void
start_rstats(int new)
{
        if (nvram_match("wan_route_x", "IP_Routed") && nvram_match("rstats_enable", "1")) {
                stop_rstats();
                if (new)
			system("rstats --new");
                else
			system("rstats");
        }
}

void
restart_rstats(void)
{
	if (nvram_match("rstats_bak", "1"))
	{
		nvram_set("rstats_path", "*nvram");
		if (nvram_match("rstats_new", "1"))
		{
			start_rstats(1);
			nvram_set("rstats_new", "0");
		}
		else
			start_rstats(0);
	}
	else 
	{
		nvram_set("rstats_path", "");
		start_rstats(0);
	}
}
////////^^^^^^^^^^^^^^^^^^^jerry5 2009.07
#endif

int 
start_upnp(void)
{
	int ret;
	FILE *fp;
	char *lan_addr, *lan_mask, *lan_url, *nat_pmp, *product;
	char var[100];
	char wan_ifname[16];
	char lan_class[32];
	uint8_t lan_mac[16];
	
	if (!nvram_invmatch("upnp_enable", "0") || nvram_match("router_disable", "1"))
		return 0;
	
	wan_ifname[0] = 0;
	get_wan_ifname(wan_ifname);
	
	lan_addr = nvram_safe_get("lan_ipaddr");
	lan_mask = nvram_safe_get("lan_netmask");
	ip2class(lan_addr, lan_mask, lan_class);
	memset(lan_mac, 0, sizeof(lan_mac));
	ether_atoe(nvram_safe_get("lan_hwaddr"), lan_mac);
	
	lan_url = lan_addr;
	ret = atoi(nvram_safe_get("http_lanport"));
	if (ret && ret != 80) {
		sprintf(var, "%s:%d", lan_addr, ret);
		lan_url = var;
	}
	
	nat_pmp = (nvram_match("upnp_enable", "2")) ? "yes" : "no";
	product = nvram_safe_get("productid");
	
	/* Write configuration file */
	if (!(fp = fopen("/etc/miniupnpd.conf", "w"))) {
		return errno;
	}
	
	fprintf(fp, "# automagically generated\n"
		"ext_ifname=%s\n"
		"listening_ip=%s/%s\n"
		"listening_ip=127.0.0.1/8\n"
		"port=0\n"
		"enable_upnp=yes\n"
		"enable_natpmp=%s\n"
		"lease_file=%s\n"
		"secure_mode=no\n"
		"presentation_url=http://%s/\n"
		"system_uptime=yes\n"
		"notify_interval=60\n"
		"clean_ruleset_interval=600\n"
		"uuid=75802409-bccb-40e7-8e6c-%02x%02x%02x%02x%02x%02x\n"
		"friendly_name=ASUS %s Wireless Router\n"
		"model_number=%s\n"
		"serial=1.0\n"
		"bitrate_up=20000000\n"
		"bitrate_down=20000000\n"
		"allow 80-65535 %s 80-65535\n"
		"deny 0-65535 0.0.0.0/0 0-65535\n",
		wan_ifname,
		lan_addr, lan_mask,
		nat_pmp,
		UPNPD_LEASE_FILE,
		lan_url,
		lan_mac[0], lan_mac[1], lan_mac[2], lan_mac[3], lan_mac[4], lan_mac[5],
		product,
		product,
		lan_class);
	
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
	
	if (!pids("miniupnpd")) 
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
update_upnp(int force_update)
{
	if (!pids("miniupnpd")) 
	{
		start_upnp();
		
		return;
	}
	
	/* update upnp forwards from lease file */
	if (force_update || check_if_file_exist(UPNPD_LEASE_FILE)) {
		system("killall -SIGUSR1 miniupnpd");
	}
}

int 
start_poptop(void)
{
	int i_cast, i_mppe, i_auth, i_mtu, i_mru, i_dhcp, i_cli0, i_cli1;
	char *lanip, *wins, *dns;
	char *pptpd_cfg, *pptpd_opt, *pptpd_sec, *pptpd_ipup, *pptpd_ipdw;
	struct in_addr in;
	unsigned int laddr, lmask;
	FILE *fp;
	
	if (nvram_invmatch("pptpd_enable", "1") || nvram_match("router_disable", "1")) 
	{
		return 0;
	}
	
	pptpd_cfg  = "/etc/pptpd.conf";
	pptpd_opt  = "/tmp/ppp/options.pptpd";
	pptpd_sec  = "/tmp/ppp/chap-secrets";
	pptpd_ipup = "/tmp/ppp/ip-up.pptpd";
	pptpd_ipdw = "/tmp/ppp/ip-down.pptpd";
	
	i_cast = atoi(nvram_safe_get("pptpd_cast"));
	i_auth = atoi(nvram_safe_get("pptpd_auth"));
	i_mppe = atoi(nvram_safe_get("pptpd_mppe"));
	i_mtu  = atoi(nvram_safe_get("pptpd_mtu"));
	i_mru  = atoi(nvram_safe_get("pptpd_mru"));
	i_cli0 = atoi(nvram_safe_get("pptpd_clib"));
	i_cli1 = atoi(nvram_safe_get("pptpd_clie"));
	i_dhcp = atoi(nvram_safe_get("dhcp_enable_x"));
	
	lanip   = nvram_safe_get("lan_ipaddr");
	
	if (i_mtu <  512) i_mtu =  512;
	if (i_mtu > 1460) i_mtu = 1460;
	if (i_mru <  512) i_mru =  512;
	if (i_mru > 1460) i_mru = 1460;
	
	if (i_cli0 <   2) i_cli0 =   2;
	if (i_cli0 > 254) i_cli0 = 254;
	if (i_cli1 <   2) i_cli1 =   2;
	if (i_cli1 > 254) i_cli1 = 254;
	if (i_cli1 < i_cli0) i_cli1 = i_cli0;
	
	laddr = ntohl(inet_addr(lanip));
	lmask = ntohl(inet_addr(nvram_safe_get("lan_netmask")));
	in.s_addr = htonl((laddr & lmask) | (unsigned int)i_cli0);
	
	mkdir("/tmp/ppp", 0777);
	
	unlink(pptpd_sec);
	symlink("/etc/storage/chap-secrets", pptpd_sec);
	
	// Create pptpd.conf
	if (!(fp = fopen(pptpd_cfg, "w"))) {
		return errno;
	}
	
	fprintf(fp, "option %s\n", pptpd_opt);
	fprintf(fp, "connections %d\n", 10);
	fprintf(fp, "localip %s\n", lanip);
	fprintf(fp, "remoteip %s-%d\n", inet_ntoa(in), i_cli1);
	fclose(fp);
	
	// Create pptpd-options
	if (!(fp = fopen(pptpd_opt, "w"))) {
		return -1;
	}
	
	fprintf(fp, "lock\n"
		"name RT-N56U\n"
		"ipcp-accept-local\n"
		"ipcp-accept-remote\n"
		"lcp-echo-failure 10\n"
		"lcp-echo-interval 10\n"
		"deflate 0\n"
		"auth\n"
		"-pap\n"
		"-chap\n");
	
	if (i_auth == 1)
	{
		fprintf(fp, "+mschap\n"
			    "+mschap-v2\n");
	}
	else
	{
		fprintf(fp, "-mschap\n"
			    "+mschap-v2\n");
	}
	
	if (i_mppe != 4)
	{
		fprintf(fp, "+mppc\n");
		if (i_mppe == 1)
		{
			fprintf(fp, "-mppe-40\n");
			fprintf(fp, "-mppe-56\n");
			fprintf(fp, "+mppe-128\n");
		}
		else if (i_mppe == 2)
		{
			fprintf(fp, "-mppe-40\n");
			fprintf(fp, "+mppe-56\n");
			fprintf(fp, "-mppe-128\n");
		}
		else if (i_mppe == 3)
		{
			fprintf(fp, "+mppe-40\n");
			fprintf(fp, "-mppe-56\n");
			fprintf(fp, "-mppe-128\n");
		}
		else
		{
			fprintf(fp, "+mppe-40\n");
			fprintf(fp, "+mppe-56\n");
			fprintf(fp, "+mppe-128\n");
		}
		fprintf(fp, "nomppe-stateful\n");
	}
	else
	{
		fprintf(fp, "nomppc\n");
		fprintf(fp, "nomppe\n");
	}
	
	// DNS Server
	if (i_dhcp == 1)
	{
		dns = nvram_safe_get("dhcp_dns1_x");
		if ((inet_addr_(dns) != INADDR_ANY) && (strcmp(dns, lanip)))
		{
			fprintf(fp, "ms-dns %s\n", dns);
		}
	}
	
	fprintf(fp, "ms-dns %s\n", lanip);
	
	if (i_dhcp == 1)
	{
		// WINS Server
		wins = nvram_safe_get("dhcp_wins_x");
		if (inet_addr_(wins) != INADDR_ANY) 
		{
			fprintf(fp, "ms-wins %s\n", wins);
		}
	}
	
	fprintf(fp, "nobsdcomp\n"
		    "novj\n"
		    "novjccomp\n"
		    "proxyarp\n"
		    "nodefaultroute\n"
		    "mtu %d\n"
		    "mru %d\n"
		    "ip-up-script %s\n"
		    "ip-down-script %s\n",
		    i_mtu, i_mru, pptpd_ipup, pptpd_ipdw);
	
	fclose(fp);
	
	// Create ip-up and ip-down scripts that are unique to pptpd
	fp = fopen(pptpd_ipup, "w");
	fprintf(fp, "#!/bin/sh\n\n"
		    "logger -t ip-up.pptpd \"ifname: $1, local IP: $5, remote IP: $6, login: $PEERNAME\"\n\n");
	if (i_cast == 1 || i_cast == 3)
		fprintf(fp, "/usr/sbin/bcrelay -d -i br0 -o $1 -n\n");
	if (i_cast == 2 || i_cast == 3)
		fprintf(fp, "/usr/sbin/bcrelay -d -i $1 -o br0 -n\n");
	fclose(fp);
	
	fp = fopen(pptpd_ipdw, "w");
	fprintf(fp, "#!/bin/sh\n\n"
		    "logger -t ip-down.pptpd \"ifname: $1\"\n\n"
		    "pids=`ps | grep bcrelay | grep $1 | awk '{print $1}' 2>/dev/null`\n"
		    "for i in $pids ; do\n"
		    "    [ -n \"$i\" ] && kill $i\n"
		    "done\n");
	fclose(fp);
	
	chmod(pptpd_cfg, 0644);
	chmod(pptpd_opt, 0644);
	chmod(pptpd_ipup, 0744);
	chmod(pptpd_ipdw, 0744);
	
	// Execute pptpd daemon
	return eval("/usr/sbin/pptpd", "-c", pptpd_cfg);
}

void 
stop_poptop(void)
{
	char* svcs[] = { "bcrelay", "pptpd", NULL };
	kill_services(svcs, 3, 1);
}

void 
restart_poptop(void)
{
	stop_poptop();
	start_poptop();
	
	rc_restart_firewall();
}

void 
run_poptop_force(void)
{
	// force run pptpd
	int state_changed = 0;
	
	stop_poptop();
	
	if (nvram_match("router_disable", "1")) 
		return;
	
	if (nvram_invmatch("pptpd_enable", "1"))
	{
		nvram_set("pptpd_enable", "1");
		state_changed = 1;
	}
	
	start_poptop();
	
	if (state_changed)
		rc_restart_firewall();
}

int
start_ntpc(void)
{
	char* svcs[] = { "ntpclient", NULL };
	kill_services(svcs, 3, 1);
	
	if (!pids("ntp"))
		system("ntp &");
	
	return 0;
}

void
stop_ntpc(void)
{
	char* svcs[] = { "ntpclient", "ntp", NULL };
	kill_services(svcs, 3, 1);
}

int start_lltd(void)
{
	return eval("lld2d", "br0");
}

void stop_lltd(void)
{
	char* svcs[] = { "lld2d", NULL };
	kill_services(svcs, 3, 1);
}

void nvram_commit_safe(void)
{
	spinlock_lock(SPINLOCK_NVRAMCommit);
	nvram_commit();
	spinlock_unlock(SPINLOCK_NVRAMCommit);
}

int
start_services(void)
{
	printf("[rc] start services\n");
	
	start_8021x_wl();
	start_8021x_rt();
	start_infosvr();
	start_detect_internet();
	start_httpd();
	start_telnetd();
	start_sshd();
	start_poptop();
	start_watchdog();

	if (!is_ap_mode())
	{
		doSystem("brctl setfd %s 15", IFNAME_BR);
		doSystem("brctl sethello %s 2", IFNAME_BR);
	}

	start_networkmap();
	start_lltd();

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	restart_rstats();
#endif

	return 0;
}

void
stop_services(int stopall)
{
	if (stopall) {
		stop_telnetd();
		stop_sshd();
		stop_poptop();
	}
	stop_p910nd();
	stop_lpd();
	stop_u2ec();
	
	stop_lltd();
	stop_detect_internet();
	stop_rstats();
}


void
stop_services_lan_wan(void)
{
	stop_dns_dhcpd();
	stop_upnp();
	stop_linkstatus_monitor();
}

void
write_storage_to_mtd(void)
{
	if (stop_service_type_99)
		return;
	
	system("/sbin/mtd_storage.sh save");
}

void
erase_storage(void)
{
	if (stop_service_type_99)
		return;
	
	system("/sbin/mtd_storage.sh erase");
}


void
erase_nvram(void)
{
	system("/bin/mtd_write erase Config");
}

int 
start_logger(int showinfo)
{	
	start_syslogd();
	
	// wait for logger daemon started
	sleep(1);
	
	if (showinfo)
	{
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


