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

#include "rc.h"

#define VPNS_LOG_NAME		"VPN server"
#define VPNS_PPP_UP_SCRIPT	"/tmp/ppp/ip-up.vpns"
#define VPNS_PPP_DW_SCRIPT	"/tmp/ppp/ip-down.vpns"

static int xl2tpd_killed_vpns = 0;

static int
get_xl2tpd_vpns_active(void)
{
	return ((nvram_match("vpns_enable", "1") && nvram_match("vpns_type", "1")) || 
		 nvram_match("l2tp_srv_t", "1"));
}

static int
gen_vpns_pppd_options(int vpns_type)
{
	FILE *fp;
	int i_mppe, i_auth, i_vuse, i_dhcp, i_items;
	char *lanip, *wins, *dns1, *dns2;
	const char *vpns_opt = VPN_SERVER_PPPD_OPTIONS;

	i_auth = nvram_get_int("vpns_auth");
	i_mppe = nvram_get_int("vpns_mppe");
	i_vuse = nvram_get_int("vpns_vuse");
	i_dhcp = is_dhcpd_enabled(0);

	lanip  = nvram_safe_get("lan_ipaddr");

	if (!(fp = fopen(vpns_opt, "w")))
		return -1;

	fprintf(fp, "name %s\n", get_our_hostname());
	fprintf(fp, "auth\n");
	fprintf(fp, "refuse-eap\n");
	fprintf(fp, "refuse-pap\n");
	fprintf(fp, "refuse-mschap\n");

	if (i_auth == 0) {
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "require-mschap-v2\n");
	}

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

	// DNS Server
	i_items = 0;
	if (i_dhcp) {
		dns1 = nvram_safe_get("dhcp_dns1_x");
		dns2 = nvram_safe_get("dhcp_dns2_x");
		if (is_valid_ipv4(dns1)) {
			i_items++;
			fprintf(fp, "ms-dns %s\n", dns1);
		}
		if (is_valid_ipv4(dns2) && strcmp(dns2, dns1)) {
			i_items++;
			fprintf(fp, "ms-dns %s\n", dns2);
		}
	}

	if (i_items < 1)
		fprintf(fp, "ms-dns %s\n", lanip);

	// WINS Server
	i_items = 0;
	if (i_dhcp) {
		wins = nvram_safe_get("dhcp_wins_x");
		if (is_valid_ipv4(wins)) {
			i_items++;
			fprintf(fp, "ms-wins %s\n", wins);
		}
	}

#if defined(APP_SMBD) || defined(APP_NMBD)
	if ((i_items < 1) && nvram_get_int("wins_enable"))
		fprintf(fp, "ms-wins %s\n", lanip);
#endif

	fprintf(fp, "mtu %d\n", nvram_safe_get_int("vpns_mtu", 1450, 1000, 1460));
	fprintf(fp, "mru %d\n", nvram_safe_get_int("vpns_mru", 1450, 1000, 1460));
	fprintf(fp, "ipcp-accept-remote ipcp-accept-local\n");
	fprintf(fp, "nodefaultroute\n");

	if (i_vuse == 0)
		fprintf(fp, "proxyarp\n");

	if (vpns_type == 1) {
		// L2TP: Don't wait for LCP term responses; exit immediately when killed
		fprintf(fp, "lcp-max-terminate %d\n", 0);
	}

	/* echo failures (6*20s) */
	fprintf(fp, "lcp-echo-interval %d\n", 20);
	fprintf(fp, "lcp-echo-failure %d\n", 6);
	fprintf(fp, "lcp-echo-adaptive\n");

	fprintf(fp, "ip-up-script %s\n", VPNS_PPP_UP_SCRIPT);
	fprintf(fp, "ip-down-script %s\n", VPNS_PPP_DW_SCRIPT);
	fprintf(fp, "minunit %d\n", VPN_SERVER_PPP_UNIT);

	fclose(fp);

	chmod(vpns_opt, 0644);

	return 0;
}

int
start_vpn_server(void)
{
	FILE *fp;
	int i, i_type, i_vuse;
	char *vpns_cfg = "/etc/pptpd.conf";
	const char *vpns_sec = "/tmp/ppp/chap-secrets";
	unsigned int vaddr, vmask, vp_b, vp_e;
	struct in_addr pool_in;

	if (nvram_invmatch("vpns_enable", "1") || get_ap_mode())
		return 0;

	unlink(VPN_SERVER_LEASE_FILE);

	i_type = nvram_get_int("vpns_type");
#if defined(APP_OPENVPN)
	if (i_type == 2)
		return start_openvpn_server();
#endif

	mkdir("/tmp/ppp", 0777);
	symlink("/sbin/rc", VPNS_PPP_UP_SCRIPT);
	symlink("/sbin/rc", VPNS_PPP_DW_SCRIPT);

	i_vuse = nvram_get_int("vpns_vuse");

	get_vpns_pool(i_vuse, &vaddr, &vmask, &vp_b, &vp_e);

	if (i_type != 1) {
		if (!(fp = fopen(vpns_cfg, "w")))
			return -1;
		
		fprintf(fp, "option %s\n", VPN_SERVER_PPPD_OPTIONS);
		fprintf(fp, "connections %d\n", MAX_CLIENTS_NUM);
		
		pool_in.s_addr = htonl(vaddr);
		fprintf(fp, "localip %s\n", inet_ntoa(pool_in));
		
		pool_in.s_addr = htonl((vaddr & vmask) | vp_b);
		fprintf(fp, "remoteip %s-%u\n", inet_ntoa(pool_in), vp_e);
		
		fclose(fp);
		
		chmod(vpns_cfg, 0644);
	}

	gen_vpns_pppd_options(i_type);

	/* create /tmp/ppp/chap-secrets */
	fp = fopen(vpns_sec, "w+");
	if (fp) {
		char *acl_user, *acl_pass;
		char acl_user_var[32], acl_pass_var[32], acl_addr_var[32];
		unsigned int vp_a;
		int i_max = nvram_get_int("vpns_num_x");
		
		if (i_max > MAX_CLIENTS_NUM) i_max = MAX_CLIENTS_NUM;
		for (i = 0; i < i_max; i++) {
			snprintf(acl_user_var, sizeof(acl_user_var), "vpns_user_x%d", i);
			snprintf(acl_pass_var, sizeof(acl_pass_var), "vpns_pass_x%d", i);
			acl_user = nvram_safe_get(acl_user_var);
			acl_pass = nvram_safe_get(acl_pass_var);
			if (*acl_user && *acl_pass) {
				snprintf(acl_addr_var, sizeof(acl_addr_var), "vpns_addr_x%d", i);
				vp_a = (unsigned int)nvram_get_int(acl_addr_var);
				if (vp_a >= vp_b && vp_a <= vp_e ) {
					pool_in.s_addr = htonl((vaddr & vmask) | vp_a);
					strcpy(acl_addr_var, inet_ntoa(pool_in));
				} else
					strcpy(acl_addr_var, "*");
				
				fprintf(fp, "\"%s\"	*	\"%s\"	%s\n", acl_user, acl_pass, acl_addr_var);
			}
		}
		fclose(fp);
		
		chmod(vpns_sec, 0600);
	}

	if (i_type == 1) {
		nvram_set_int_temp("l2tp_srv_t", 1);
		
		safe_start_xl2tpd();
	} else {
		nvram_set_int_temp("l2tp_srv_t", 0);
		
		/* execute pptpd daemon */
		return eval("/usr/sbin/pptpd", "-c", vpns_cfg);
	}

	return 0;
}

void
stop_vpn_server(void)
{
	int i;
	char pppd_pid[32];
	char* svcs[] = { "bcrelay", "pptpd", NULL,  NULL };

	if (get_xl2tpd_vpns_active()) {
		svcs[2] = "xl2tpd";
		xl2tpd_killed_vpns = 1;
	}

	kill_services(svcs, 5, 1);

	/* force kill all clients pppd */
	for (i=0; i<MAX_CLIENTS_NUM; i++) {
		snprintf(pppd_pid, sizeof(pppd_pid), "/var/run/ppp%d.pid", VPN_SERVER_PPP_UNIT+i);
		kill_pidfile_s(pppd_pid, SIGKILL);
	}

	nvram_set_int_temp("l2tp_srv_t", 0);

	unlink(VPNS_PPP_UP_SCRIPT);
	unlink(VPNS_PPP_DW_SCRIPT);

#if defined(APP_OPENVPN)
	stop_openvpn_server();
#endif

	unlink(VPN_SERVER_LEASE_FILE);
}

void
restart_vpn_server(void)
{
	if (get_ap_mode())
		return;

	xl2tpd_killed_vpns = 0;

	stop_vpn_server();
	start_vpn_server();

	restart_firewall();

#if defined(APP_NFSD)
	reload_nfsd();
#endif

	/* restore L2TP WAN client or L2TP VPNC */
	if (xl2tpd_killed_vpns && (nvram_match("l2tp_wan_t", "1") || nvram_match("l2tp_cli_t", "1")))
		safe_start_xl2tpd();
}

void
reapply_vpn_server(void)
{
	int i_type;

	if (nvram_invmatch("vpns_enable", "1") || get_ap_mode())
		return;

	i_type = nvram_get_int("vpns_type");
#if defined(APP_OPENVPN)
	if (i_type == 2)
		restart_openvpn_server();
	else
#endif
		gen_vpns_pppd_options(i_type);
}

static void
vpns_firewall_permission(char *ifname, int add)
{
	doSystem("/bin/iptables -%s %s -i %s -j %s", (add) ? "A" : "D", IPT_CHAIN_NAME_VPN_LIST, ifname, "ACCEPT");
}

void
get_vpns_pool(int i_vuse, unsigned int *ip_v, unsigned int *ip_m, unsigned int *vp_b, unsigned int *vp_e)
{
	unsigned int lsnet, lpool[2];

	if (i_vuse == 0) {
		*ip_v = ntohl(inet_addr(nvram_safe_get("lan_ipaddr")));
		*ip_m = ntohl(inet_addr(nvram_safe_get("lan_netmask")));
		lsnet = ~(*ip_m) - 1;
		
		lpool[0] = (unsigned int)nvram_safe_get_int("vpns_cli0", 245, 1, 254);
		lpool[1] = (unsigned int)nvram_safe_get_int("vpns_cli1", 254, 2, 254);
		if (lpool[0] >= lsnet)
			lpool[0] = (lsnet - 1);
		if (lpool[1] > lsnet)
			lpool[1] = lsnet;
		if (lpool[1] <= lpool[0])
			lpool[1] = lpool[0] + 1;
		*ip_v = (*ip_v & *ip_m) | lpool[0];
		lpool[0] += 1;
	} else {
		*ip_v = ntohl(inet_addr(nvram_safe_get("vpns_vnet")));
		*ip_m = ntohl(inet_addr(VPN_SERVER_SUBNET_MASK));
		*ip_v = (*ip_v & *ip_m) | 1;
		
		lpool[0] = 2;
		lpool[1] = lpool[0] + MAX_CLIENTS_NUM - 1;
	}

	*vp_b = lpool[0];
	*vp_e = lpool[1];
}

void
vpns_route_to_remote_lan(const char *cname, char *ifname, char *gw, int add)
{
	int i, i_max;
	char *acl_user, *acl_rnet, *acl_rmsk, *lnet, *lmsk;
	char acl_user_var[16], acl_rnet_var[16], acl_rmsk_var[16];

	lnet = nvram_safe_get("lan_ipaddr");
	lmsk = nvram_safe_get("lan_netmask");

	i_max = nvram_get_int("vpns_num_x");
	if (i_max > MAX_CLIENTS_NUM) i_max = MAX_CLIENTS_NUM;
	for (i = 0; i < i_max; i++) {
		snprintf(acl_user_var, sizeof(acl_user_var), "vpns_user_x%d", i);
		snprintf(acl_rnet_var, sizeof(acl_rnet_var), "vpns_rnet_x%d", i);
		snprintf(acl_rmsk_var, sizeof(acl_rmsk_var), "vpns_rmsk_x%d", i);
		acl_user = nvram_safe_get(acl_user_var);
		acl_rnet = nvram_safe_get(acl_rnet_var);
		acl_rmsk = nvram_safe_get(acl_rmsk_var);
		if (*acl_user && strcmp(acl_user, cname) == 0 && is_valid_ipv4(acl_rnet) && is_valid_ipv4(acl_rmsk)) {
			if (!is_same_subnet2(acl_rnet, lnet, acl_rmsk, lmsk)) {
				if (add)
					route_add(ifname, 0, acl_rnet, gw, acl_rmsk);
				else
					route_del(ifname, 0, acl_rnet, gw, acl_rmsk);
			}
			
			break;
		}
	}
}

int
ipup_vpns_main(int argc, char **argv)
{
	FILE *fp;
	int i_cast, i_vuse;
	char *peer_name;
	char *svcs[] = { "bcrelay", NULL };
	const char *script_name = VPN_SERVER_UPDOWN_SCRIPT;

	if (argc < 7)
		return -1;

	peer_name = safe_getenv("PEERNAME");

	logmessage(VPNS_LOG_NAME, "peer %s (%s) connected - ifname: %s, local IP: %s",
			argv[6], peer_name, argv[1], argv[5]);

	umask(0000);

	/* add firewall permission for this client */
	vpns_firewall_permission(argv[1], 1);

	i_vuse = nvram_get_int("vpns_vuse");
	if (i_vuse) {
		/* disable multicast flag */
		doSystem("ifconfig %s %s", argv[1], "-multicast");
	}

	/* add route to client's LAN */
	vpns_route_to_remote_lan(peer_name, argv[1], NULL, 1);

	set_vpn_balancing(argv[1], 1);

	fp = fopen(VPN_SERVER_LEASE_FILE, "a+");
	if (fp) {
		fprintf(fp, "%s %s %s %s\n", argv[1], argv[5], argv[6], peer_name);
		fclose(fp);
	}

	i_cast = nvram_get_int("vpns_cast");
	if (pids(svcs[0])) {
		if (i_cast == 0)
			kill_services(svcs, 3, 1);
	} else if (i_vuse == 0) {
		if (i_cast == 1 || i_cast == 3)
			eval("/usr/sbin/bcrelay", "-d", "-i", IFNAME_BR, "-o", "ppp[1-5][0-9]", "-n");
		if (i_cast == 2 || i_cast == 3)
			eval("/usr/sbin/bcrelay", "-d", "-i", "ppp[1-5][0-9]", "-o", IFNAME_BR, "-n");
	}

	if (check_if_file_exist(script_name))
		doSystem("%s %s %s %s %s %s", script_name, "up", argv[1], argv[5], argv[6], peer_name);

	return 0;
}

int
ipdown_vpns_main(int argc, char **argv)
{
	FILE *fp1, *fp2;
	int i_clients;
	char ifname[16], addr_l[64], addr_r[64], name_p[64];
	char *peer_name;
	char *clients_l1 = VPN_SERVER_LEASE_FILE;
	char *clients_l2 = "/tmp/.vpns.leases";
	char *svcs[] = { "bcrelay", NULL };
	const char *script_name = VPN_SERVER_UPDOWN_SCRIPT;

	if (argc < 7)
		return -1;

	peer_name = safe_getenv("PEERNAME");

	logmessage(VPNS_LOG_NAME, "peer %s (%s) disconnected", argv[6], peer_name);

	umask(0000);

	vpns_firewall_permission(argv[1], 0);
	vpns_route_to_remote_lan(peer_name, argv[1], NULL, 0);

	i_clients = 0;
	fp1 = fopen(clients_l1, "r");
	fp2 = fopen(clients_l2, "w");
	if (fp1) {
		while(fscanf(fp1, "%15s %63s %63s %63[^\n]\n", ifname, addr_l, addr_r, name_p) == 4) {
			if (strcmp(ifname, argv[1])) {
				i_clients++;
				if (fp2)
					fprintf(fp2, "%s %s %s %s\n", ifname, addr_l, addr_r, name_p);
			}
		}
		
		fclose(fp1);
	}

	if (fp2) {
		fclose(fp2);
		rename(clients_l2, clients_l1);
		unlink(clients_l2);
	}

	if (check_if_file_exist(script_name))
		doSystem("%s %s %s %s %s %s", script_name, "down", argv[1], argv[5], argv[6], peer_name);

	if ((i_clients == 0 || nvram_get_int("vpns_cast") == 0) && pids(svcs[0]))
		kill_services(svcs, 3, 1);

	return 0;
}

