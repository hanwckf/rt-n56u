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
#include <stdint.h>
#include <fcntl.h>

#include "rc.h"

#define VPNC_LOG_NAME		"VPN client"
#define VPNC_PPP_LINK_NAME	"vpnc"
#define VPNC_PPP_UP_SCRIPT	"/tmp/ppp/ip-up.vpnc"
#define VPNC_PPP_DW_SCRIPT	"/tmp/ppp/ip-down.vpnc"

static int xl2tpd_killed_vpnc = 0;

static int
control_xl2tpd(const char *cmd, const char *lac)
{
	char buf[32];
	
	int control_fd = open("/var/run/xl2tpd-control", O_WRONLY, 0600);
	if (control_fd < 0)
		return -1;
	
	snprintf(buf, sizeof(buf), "%s %s", cmd, lac);
	write(control_fd, buf, strlen(buf));
	close(control_fd);
	
	return 0;
}

static int
get_xl2tpd_vpnc_active(void)
{
	return ((nvram_match("vpnc_enable", "1") && nvram_match("vpnc_type", "1")) || 
		 nvram_match("l2tp_cli_t", "1"));
}

int
start_vpn_client(void)
{
	FILE *fp;
	int i_type, i_mppe, i_auth;
	char *vpnc_peer, *vpnc_opt, tmp[256];

	if (nvram_invmatch("vpnc_enable", "1") || get_ap_mode())
		return 1;

	vpnc_peer = nvram_safe_get("vpnc_peer");
	if (strlen(vpnc_peer) < 1) {
		logmessage(VPNC_LOG_NAME, "Unable to start - remote server host is not defined!");
		return 1;
	}

	nvram_set_temp("vpnc_dns_t", "");
	nvram_set_temp("vpnc_dom_t", "");
	nvram_set_int_temp("vpnc_state_t", 0);

	i_type = nvram_get_int("vpnc_type");
#if defined(APP_OPENVPN)
	if (i_type == 2)
		return start_openvpn_client();
#endif
	vpnc_opt = VPN_CLIENT_PPPD_OPTIONS;

	mkdir("/tmp/ppp", 0777);
	symlink("/sbin/rc", VPNC_PPP_UP_SCRIPT);
	symlink("/sbin/rc", VPNC_PPP_DW_SCRIPT);

	i_auth = nvram_get_int("vpnc_auth");
	i_mppe = nvram_get_int("vpnc_mppe");

	// Create options for pppd
	if (!(fp = fopen(vpnc_opt, "w"))) {
		return -1;
	}

	fprintf(fp, "noauth\n");
	fprintf(fp, "user '%s'\n", safe_pppd_line(nvram_safe_get("vpnc_user"), tmp, sizeof(tmp)));
	fprintf(fp, "password '%s'\n", safe_pppd_line(nvram_safe_get("vpnc_pass"), tmp, sizeof(tmp)));
	fprintf(fp, "refuse-eap\n");

	if (i_auth == 1) {
		/* MS-CHAPv2 */
		fprintf(fp, "refuse-pap\n");
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
	}
	else if (i_auth == 2) {
		/* CHAP */
		fprintf(fp, "refuse-pap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}
	else if (i_auth == 3) {
		/* PAP */
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}

	if (i_type != 1) {
		fprintf(fp, "plugin pptp.so\n");
		fprintf(fp, "pptp_server '%s'\n", vpnc_peer);
		fprintf(fp, "route_rdgw %d\n", (nvram_match("vpnc_dgw", "1")) ? 2 : 0);
		fprintf(fp, "persist\n");
		fprintf(fp, "linkname %s\n", VPNC_PPP_LINK_NAME);
	}

	fprintf(fp, "mtu %d\n", nvram_safe_get_int("vpnc_mtu", 1450, 1000, 1460));
	fprintf(fp, "mru %d\n", nvram_safe_get_int("vpnc_mru", 1450, 1000, 1460));

	fprintf(fp, "maxfail %d\n", 0);		// pppd re-call count (0=infinite)
	fprintf(fp, "holdoff %d\n", 10);	// pppd re-call time (10s)

	fprintf(fp, "ipcp-accept-remote ipcp-accept-local\n");
	fprintf(fp, "noipdefault\n");
	fprintf(fp, "usepeerdns\n");

	fprintf(fp, "default-asyncmap\n");

	/* looks like pptp also likes them */
	fprintf(fp, "nopcomp noaccomp\n");

	/* ccp should still be enabled - mppe/mppc requires this */
	fprintf(fp, "novj nobsdcomp nodeflate\n");

	if (i_mppe == 3) {
		fprintf(fp, "nomppe nomppc\n");
	} else {
		if (i_mppe == 1) {
			fprintf(fp, "+mppe\n");
			fprintf(fp, "-mppe-40\n");
			fprintf(fp, "+mppe-128\n");
		} else if (i_mppe == 2) {
			fprintf(fp, "+mppe\n");
			fprintf(fp, "+mppe-40\n");
			fprintf(fp, "-mppe-128\n");
		} else {
			fprintf(fp, "+mppe-40\n");
			fprintf(fp, "+mppe-128\n");
		}
		fprintf(fp, "nomppe-stateful\n");
	}

	if (i_type == 1)
	{
		// Don't wait for LCP term responses; exit immediately when killed
		fprintf(fp, "lcp-max-terminate %d\n", 0);
	}

	/* echo failures (6*20s) */
	fprintf(fp, "lcp-echo-interval %d\n", 20);
	fprintf(fp, "lcp-echo-failure %d\n", 6);
	fprintf(fp, "lcp-echo-adaptive\n");

	fprintf(fp, "ip-up-script %s\n", VPNC_PPP_UP_SCRIPT);
	fprintf(fp, "ip-down-script %s\n", VPNC_PPP_DW_SCRIPT);

	fprintf(fp, "minunit %d\n", VPNC_PPP_UNIT);
	fprintf(fp, "ktune\n");

	/* user specific options */
	fprintf(fp, "%s\n", nvram_safe_get("vpnc_pppd"));

	fclose(fp);

	chmod(vpnc_opt, 0600);

	if (i_type == 1)
	{
		nvram_set_int_temp("l2tp_cli_t", 1);
		
		if (safe_start_xl2tpd() != 0)
			control_xl2tpd("c", "VPNC");
	}
	else
	{
		nvram_set_int_temp("l2tp_cli_t", 0);
		
		return eval("/usr/sbin/pppd", "file", vpnc_opt);
	}

	return 0;
}

void
stop_vpn_client(void)
{
	char pppd_pid[32];

	if (get_xl2tpd_vpnc_active() && pids("xl2tpd"))
		control_xl2tpd("d", "VPNC");

	sprintf(pppd_pid, "/var/run/ppp-%s.pid", VPNC_PPP_LINK_NAME);
	kill_process_pidfile(pppd_pid, 5, 1);

#if defined(APP_OPENVPN)
	stop_openvpn_client();
#endif

	nvram_set_int_temp("l2tp_cli_t", 0);
	nvram_set_int_temp("vpnc_state_t", 0);

	unlink(VPNC_PPP_UP_SCRIPT);
	unlink(VPNC_PPP_DW_SCRIPT);
}

static void
stop_vpn_client_force(void)
{
	if (get_xl2tpd_vpnc_active()) {
		char* svcs[] = { "xl2tpd", NULL };
		kill_services(svcs, 5, 1);
		xl2tpd_killed_vpnc = 1;
	}
}

void
restart_vpn_client(void)
{
	if (get_ap_mode())
		return;

	xl2tpd_killed_vpnc = 0;

	stop_vpn_client_force();
	stop_vpn_client();

	restore_dns_from_vpnc();

	sleep(1);
	start_vpn_client();

	restart_firewall();

	/* restore L2TP WAN client or L2TP VPNS */
	if (xl2tpd_killed_vpnc && (nvram_match("l2tp_wan_t", "1") || nvram_match("l2tp_srv_t", "1")))
		safe_start_xl2tpd();
}

void
restore_dns_from_vpnc(void)
{
	char *vpnc_dns = nvram_safe_get("vpnc_dns_t");

	if (*vpnc_dns) {
		nvram_set_temp("vpnc_dns_t", "");
		update_resolvconf(0, 0);
	}
}

static void
vpnc_route_dgw(char *ifname, char *gw, int add)
{
	if (nvram_match("vpnc_dgw", "1")) {
		if (strlen(ifname) > 0 && gw) {
			if (add) {
				route_add(ifname, 0, "0.0.0.0", gw, "128.0.0.0");
				route_add(ifname, 0, "128.0.0.0", gw, "128.0.0.0");
			} else {
				route_del(ifname, 0, "0.0.0.0", gw, "128.0.0.0");
				route_del(ifname, 0, "128.0.0.0", gw, "128.0.0.0");
			}
		}
	}
}

static void
vpnc_route_to_remote_lan(char *ifname, char *gw, int add)
{
	char *rnet = nvram_safe_get("vpnc_rnet");
	char *rmsk = nvram_safe_get("vpnc_rmsk");

	if (is_valid_ipv4(rnet) && is_valid_ipv4(rmsk)) {
		char *lnet = nvram_safe_get("lan_ipaddr");
		char *lmsk = nvram_safe_get("lan_netmask");
		if (strlen(ifname) > 0 && !is_same_subnet2(rnet, lnet, rmsk, lmsk)) {
			if (add)
				route_add(ifname, 0, rnet, gw, rmsk);
			else
				route_del(ifname, 0, rnet, gw, rmsk);
		}
	}
}

int
ipup_vpnc_main(int argc, char **argv)
{
	char buf[256];
	char *script_name = VPN_CLIENT_UPDOWN_SCRIPT;
	char *ifname = safe_getenv("IFNAME");
	char *gw = getenv("IPREMOTE");

	umask(0000);

	vpnc_route_to_remote_lan(ifname, gw, 1);
	vpnc_route_dgw(ifname, gw, 1);

	set_vpn_balancing(ifname, 0);

	nvram_set_int_temp("vpnc_state_t", 1);

	buf[0] = 0;
	if (nvram_get_int("vpnc_pdns") > 0) {
		char *value;
		
		value = getenv("DNS1");
		if (value)
			snprintf(buf, sizeof(buf), "%s", value);
		value = getenv("DNS2");
		if (value && strcmp(value, buf) != 0) {
			int buf_len = strlen(buf);
			snprintf(buf + buf_len, sizeof(buf) - buf_len, "%s%s", (buf_len) ? " " : "", value);
		}
	}

	nvram_set_temp("vpnc_dns_t", buf);
	if (strlen(buf) > 0)
		update_resolvconf(0, 0);

	if (check_if_file_exist(script_name))
		doSystem("%s %s", script_name, "up");

	return 0;
}

int
ipdown_vpnc_main(int argc, char **argv)
{
	char *script_name = VPN_CLIENT_UPDOWN_SCRIPT;
	char *ifname = safe_getenv("IFNAME");
	char *gw = getenv("IPREMOTE");

	umask(0000);

	vpnc_route_dgw(ifname, gw, 0);
	vpnc_route_to_remote_lan(ifname, gw, 0);

	nvram_set_int_temp("vpnc_state_t", 0);

	restore_dns_from_vpnc();

	if (check_if_file_exist(script_name))
		doSystem("%s %s", script_name, "down");

	return 0;
}

