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
#include <shutils.h>
#include <ralink.h>

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

	if (sshd_mode == 2)
	{
		eval("/usr/bin/sshd.sh", "start", "-s");
	}
	else if (sshd_mode == 1)
	{
		eval("/usr/bin/sshd.sh", "start");
	}

	sshd_mode_last = sshd_mode;
}

void 
restart_term(void)
{
	int is_run_before = is_sshd_run();
	int is_run_after;

	start_telnetd();
	start_sshd();

	is_run_after = is_sshd_run();

	if (is_run_after && !is_run_before && nvram_match("sshd_wopen", "1") && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}

void
start_httpd(int restart_fw)
{
	char *http_port = nvram_safe_get("http_lanport");
	if (atoi(http_port) < 80 || atoi(http_port) > 65535)
	{
		http_port = "80";
		nvram_set("http_lanport", http_port);
	}
	
	eval("/usr/sbin/httpd", http_port);
	
	if (restart_fw && nvram_match("misc_http_x", "1") && nvram_match("fw_enable_x", "1"))
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
	if (nvram_match("wan_route_x", "IP_Routed") && nvram_match("rstats_enable", "1")) {
		eval("rstats");
	}
}

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
	
	if (nvram_match("upnp_enable", "0") || nvram_match("router_disable", "1"))
		return 0;
	
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
start_vpn_server(void)
{
	int i_type, i_cast, i_mppe, i_auth, i_mtu, i_mru, i_dhcp, i_cli0, i_cli1;
	int i, i_max, i_cli2, i_dns;
	char *lanip, *wins, *dns1, *dns2, *srv_name, *acl_user, *acl_pass;
	char *vpns_cfg, *vpns_opt, *vpns_sec, *vpns_ipup, *vpns_ipdw;
	char acl_user_var[32], acl_pass_var[32], acl_addr_var[32];
	struct in_addr pool_in;
	unsigned int laddr, lmask;
	FILE *fp;
	
	if (nvram_invmatch("vpns_enable", "1") || nvram_match("router_disable", "1")) 
	{
		return 0;
	}
	
	vpns_sec = "/tmp/ppp/chap-secrets";
	vpns_ipup = "/tmp/ppp/ip-up.vpns";
	vpns_ipdw = "/tmp/ppp/ip-down.vpns";
	
	mkdir("/tmp/ppp", 0777);
	unlink("/tmp/vpns.leases");
	symlink("/sbin/rc", vpns_ipup);
	symlink("/sbin/rc", vpns_ipdw);
	
	i_type = nvram_get_int("vpns_type");
	i_cast = nvram_get_int("vpns_cast");
	i_auth = nvram_get_int("vpns_auth");
	i_mppe = nvram_get_int("vpns_mppe");
	i_mtu  = nvram_get_int("vpns_mtu");
	i_mru  = nvram_get_int("vpns_mru");
	i_cli0 = nvram_get_int("vpns_cli0");
	i_cli1 = nvram_get_int("vpns_cli1");
	i_dhcp = nvram_get_int("dhcp_enable_x");
	
	lanip  = nvram_safe_get("lan_ipaddr");
	laddr = ntohl(inet_addr(lanip));
	lmask = ntohl(inet_addr(nvram_safe_get("lan_netmask")));
	
	srv_name = nvram_safe_get("computer_name");
	if (!srv_name[0] || !is_valid_hostname(srv_name))
		srv_name = nvram_safe_get("productid");
	
	if (i_mtu <  512) i_mtu =  512;
	if (i_mtu > 1460) i_mtu = 1460;
	if (i_mru <  512) i_mru =  512;
	if (i_mru > 1460) i_mru = 1460;
	
	if (i_cli0 <   2) i_cli0 =   2;
	if (i_cli0 > 254) i_cli0 = 254;
	if (i_cli1 <   2) i_cli1 =   2;
	if (i_cli1 > 254) i_cli1 = 254;
	if (i_cli1 < i_cli0) i_cli1 = i_cli0;
	
	if (i_type == 1)
	{
		vpns_cfg  = "/etc/xl2tpd.conf";
		vpns_opt  = "/tmp/ppp/options.xl2tpd";
		
		if (write_xl2tpd_conf(vpns_cfg) < 0)
			return -1;
	}
	else
	{
		vpns_cfg  = "/etc/pptpd.conf";
		vpns_opt  = "/tmp/ppp/options.pptpd";
		
		pool_in.s_addr = htonl((laddr & lmask) | (unsigned int)i_cli0);
		
		// Create pptpd.conf
		if (!(fp = fopen(vpns_cfg, "w"))) {
			return errno;
		}
		
		fprintf(fp, "option %s\n", vpns_opt);
		fprintf(fp, "connections %d\n", 10);
		fprintf(fp, "localip %s\n", lanip);
		fprintf(fp, "remoteip %s-%d\n", inet_ntoa(pool_in), i_cli1);
		fclose(fp);
	}
	
	// Create options for pppd
	if (!(fp = fopen(vpns_opt, "w"))) {
		return -1;
	}
	
	fprintf(fp, "lock\n"
		"name %s\n"
		"ipcp-accept-local\n"
		"ipcp-accept-remote\n"
		"lcp-echo-failure 10\n"
		"lcp-echo-interval 15\n"
		"nodeflate\n"
		"auth\n"
		"-pap\n",
		srv_name);
	
	if (i_auth == 2)
	{
		fprintf(fp, "+chap\n"
			    "+mschap\n"
			    "+mschap-v2\n");
	}
	else if (i_auth == 1)
	{
		fprintf(fp, "-chap\n"
			    "+mschap\n"
			    "+mschap-v2\n");
	}
	else
	{
		fprintf(fp, "-chap\n"
			    "-mschap\n"
			    "+mschap-v2\n");
	}
	
	if (i_mppe != 3 && i_type == 0)
	{
		fprintf(fp, "+mppc\n");
		if (i_mppe == 1)
		{
			fprintf(fp, "-mppe-40\n"
				    "+mppe-128\n");
		}
		else if (i_mppe == 2)
		{
			fprintf(fp, "+mppe-40\n"
				    "-mppe-128\n");
		}
		else
		{
			fprintf(fp, "+mppe-40\n"
				    "+mppe-128\n");
		}
		fprintf(fp, "nomppe-stateful\n");
	}
	else
	{
		fprintf(fp, "nomppc\n"
			    "nomppe\n");
	}
	
	// DNS Server
	i_dns = 0;
	if (i_dhcp == 1)
	{
		dns1 = nvram_safe_get("dhcp_dns1_x");
		dns2 = nvram_safe_get("dhcp_dns2_x");
		if ((inet_addr_(dns1) != INADDR_ANY) && (strcmp(dns1, lanip))) {
			i_dns++;
			fprintf(fp, "ms-dns %s\n", dns1);
		}
		if ((inet_addr_(dns2) != INADDR_ANY) && (strcmp(dns2, lanip)) && (strcmp(dns2, dns1))) {
			i_dns++;
			fprintf(fp, "ms-dns %s\n", dns2);
		}
	}
	
	if (i_dns < 2)
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
		    "ip-down-script %s\n"
		    "minunit %d\n",
		    i_mtu, i_mru, vpns_ipup, vpns_ipdw, 10);
	
	fclose(fp);
	
	/* create /tmp/ppp/chap-secrets */
	fp = fopen(vpns_sec, "w+");
	if (fp) {
		i_max = nvram_get_int("vpns_num_x");
		if (i_max > 10) i_max = 10;
		for (i = 0; i < i_max; i++) {
			sprintf(acl_user_var, "vpns_user_x%d", i);
			sprintf(acl_pass_var, "vpns_pass_x%d", i);
			acl_user = nvram_safe_get(acl_user_var);
			acl_pass = nvram_safe_get(acl_pass_var);
			if (*acl_user && *acl_pass)
			{
				sprintf(acl_addr_var, "vpns_addr_x%d", i);
				i_cli2 = nvram_get_int(acl_addr_var);
				if (i_cli2 >= i_cli0 && i_cli2 <= i_cli1 ) {
					pool_in.s_addr = htonl((laddr & lmask) | (unsigned int)i_cli2);
					strcpy(acl_addr_var, inet_ntoa(pool_in));
				}
				else
					strcpy(acl_addr_var, "*");
				
				fprintf(fp, "\"%s\"	*	\"%s\"	%s\n", acl_user, acl_pass, acl_addr_var);
			}
		}
		fclose(fp);
	}
	
	chmod(vpns_sec, 0600);
	chmod(vpns_cfg, 0644);
	chmod(vpns_opt, 0644);
	
	/* set CPU load limit for prevent drop PPP session */
	set_ppp_limit_cpu();
	
	if (i_type == 1)
	{
		nvram_set("l2tp_srv_t", "1");
		
		if (!pids("xl2tpd"))
		{
			/* execute xl2tpd daemon */
			return eval("/usr/sbin/xl2tpd", "-c", vpns_cfg);
		}
	}
	else
	{
		nvram_set("l2tp_srv_t", "0");
		
		/* execute pptpd daemon */
		return eval("/usr/sbin/pptpd", "-c", vpns_cfg);
	}
	
	return 0;
}

void 
stop_vpn_server(void)
{
	char* svcs[] = { "bcrelay", "pptpd", NULL,  NULL };

	if (nvram_match("l2tp_srv_t", "1"))
		svcs[2] = "xl2tpd";

	kill_services(svcs, 5, 1);

	nvram_set("l2tp_srv_t", "0");

	unlink("/tmp/ppp/ip-up.vpns");
	unlink("/tmp/ppp/ip-down.vpns");
	unlink("/tmp/vpns.leases");
}

void 
restart_vpn_server(void)
{
	stop_vpn_server();
	start_vpn_server();

	restart_firewall();

	/* restore L2TP client */
	if (nvram_match("wan_l2tpd", "0") && nvram_match("l2tp_cli_t", "1") && !pids("xl2tpd"))
	{
		restart_xl2tpd();
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

void nvram_commit_safe(void)
{
	nvram_commit();
}

int
start_services(void)
{
	printf("[rc] start services\n");
	
	start_8021x_wl();
	start_8021x_rt();
	start_detect_internet();
	start_httpd(0);
	start_telnetd();
	start_sshd();
	start_vpn_server();
	start_watchdog();
	start_infosvr();

	if (!is_ap_mode() && !nvram_match("lan_stp", "0"))
	{
		doSystem("brctl stp %s %d", IFNAME_BR, 1);
		doSystem("brctl setfd %s %d", IFNAME_BR, 15);
	}

	start_networkmap();

	start_lltd();

	start_rstats();

	return 0;
}

void
stop_services(int stopall)
{
	if (stopall) {
		stop_telnetd();
		stop_sshd();
		stop_vpn_server();
	}
	stop_p910nd();
	stop_lpd();
	stop_u2ec();
	
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
	system("/sbin/mtd_storage.sh save");
}

void
erase_storage(void)
{
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


