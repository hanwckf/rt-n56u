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
#include <unistd.h>
#include <string.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>

#include <nvram/bcmnvram.h>
#include <netconf.h>
#include <shutils.h>

#include "rc.h"

int write_xl2tpd_conf(char *l2tp_conf)
{
	FILE *fp;
	int unit, i_cli0, i_cli1, has_work;
	char *prefix = "wan0_";
	char *l2tp_peer, *l2tp_user, *srv_name, *lanip;
	char pool1[32], pool2[32], tmp[100];
	struct in_addr pool_in;
	unsigned int laddr, lmask;

	srv_name = nvram_safe_get("computer_name");
	if (!(*srv_name) || !is_valid_hostname(srv_name))
		srv_name = nvram_safe_get("productid");

	if (!(fp = fopen(l2tp_conf, "w"))) {
		perror(l2tp_conf);
		return -1;
	}

	has_work = 0;

	fprintf(fp,
		"[global]\n"
		"port = 1701\n"
		"access control = no\n"
		"rand source = dev\n");

	if (nvram_match("vpns_enable", "1") && nvram_match("vpns_type", "1"))
	{
		i_cli0 = nvram_get_int("vpns_cli0");
		i_cli1 = nvram_get_int("vpns_cli1");
		if (i_cli0 <   2) i_cli0 =   2;
		if (i_cli0 > 254) i_cli0 = 254;
		if (i_cli1 <   2) i_cli1 =   2;
		if (i_cli1 > 254) i_cli1 = 254;
		if (i_cli1 < i_cli0) i_cli1 = i_cli0;
		
		lanip = nvram_safe_get("lan_ipaddr");
		laddr = ntohl(inet_addr(lanip));
		lmask = ntohl(inet_addr(nvram_safe_get("lan_netmask")));
		pool_in.s_addr = htonl((laddr & lmask) | (unsigned int)i_cli0);
		strcpy(pool1, inet_ntoa(pool_in));
		pool_in.s_addr = htonl((laddr & lmask) | (unsigned int)i_cli1);
		strcpy(pool2, inet_ntoa(pool_in));
		
		fprintf(fp, 
			"\n[lns default]\n"
			"hostname = %s\n"
			"local ip = %s\n"
			"ip range = %s-%s\n"
			"pppoptfile = /tmp/ppp/options.xl2tpd\n"
			"exclusive = yes\n"
			"hidden bit = no\n"
			"length bit = yes\n"
			"tunnel rws = 8\n"
			"require authentication = no\n"
			"require chap = yes\n"
			"refuse pap = yes\n",
			srv_name, lanip, pool1, pool2);
		
		has_work++;
	}

	if (nvram_match("wan_l2tpd", "0") && nvram_match(strcat_r(prefix, "proto", tmp), "l2tp"))
	{
		unit = nvram_get_int(strcat_r(prefix, "unit", tmp));
		if (unit < 0 || unit > 9) unit = 0;
		
		l2tp_peer = nvram_safe_get("wan_heartbeat_x");
		l2tp_user = nvram_safe_get(strcat_r(prefix, "pppoe_username", tmp));
		
		if (!(*l2tp_peer))
			l2tp_peer = nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp));
		
		fprintf(fp,
			"\n[lac ISP%d]\n"
			"pppoptfile = /tmp/ppp/options.wan%d\n"
			"lns = %s\n"
			"name = %s\n"
			"require authentication = no\n"
			"tx bps = 100000000\n"
			"rx bps = 100000000\n"
			"autodial = yes\n"
			"redial = yes\n"
			"redial timeout = 15\n"
			"tunnel rws = 8\n",
			unit, unit, l2tp_peer, l2tp_user);
		
		has_work++;
	}

	fclose(fp);

	return has_work;
}

int write_rpl2tp_conf(void)
{
	FILE *fp;
	int unit;
	char tmp[100];
	char *prefix = "wan0_";
	char *l2tp_conf, *l2tp_peer;

	unit = nvram_get_int(strcat_r(prefix, "unit", tmp));
	if (unit < 0 || unit > 9) unit = 0;

	l2tp_conf = "/etc/l2tp/l2tp.conf";
	l2tp_peer = nvram_safe_get("wan_heartbeat_x");

	if (!(*l2tp_peer))
		l2tp_peer = nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp));

	mkdir_if_none("/etc/l2tp");

	if (!(fp = fopen(l2tp_conf, "w"))) {
		perror(l2tp_conf);
		return -1;
	}

	fprintf(fp, "# automagically generated\n"
		"global\n\n"
		"load-handler \"sync-pppd.so\"\n"
		"load-handler \"cmd.so\"\n\n"
		"section sync-pppd\n\n"
		"lac-pppd-opts \"file /tmp/ppp/options.wan%d\"\n\n"
		"section peer\n"
		"peername %s\n"
		"lac-handler sync-pppd\n"
		"persist yes\n"
		"maxfail 0\n"    // l2tpd re-call count (0=infinite)
		"holdoff 15\n"   // l2tpd re-call time (15s)
		"hide-avps no\n"
		"section cmd\n\n",
		unit, l2tp_peer);

	fclose(fp);

	return 0;
}

void restart_xl2tpd(void)
{
	char *l2tp_conf;
	
	l2tp_conf = "/etc/xl2tpd.conf";
	
	/* check has lac or lns */
	if (write_xl2tpd_conf(l2tp_conf) < 0)
		return;
	
	chmod(l2tp_conf, 0644);
	
	/* launch xl2tpd */
	eval("/usr/sbin/xl2tpd", "-c", l2tp_conf);
}

int start_pppd(char *prefix)
{
	FILE *fp;
	int unit;
	char options[80];
	char tmp[100];
	char *pptp_mpp;
	mode_t mask;
	char *ppp_user, *ppp_pass;
	char *svcs[] = { NULL, NULL };

	unit = nvram_get_int(strcat_r(prefix, "unit", tmp));
	if (unit < 0 || unit > 9) unit = 0;
	sprintf(options, "/tmp/ppp/options.wan%d", unit);

	mask = umask(0000);

	/* Generate options file */
	if (!(fp = fopen(options, "w"))) {
		perror(options);
		umask(mask);
		return -1;
	}

	umask(mask);

	ppp_user = nvram_safe_get(strcat_r(prefix, "pppoe_username", tmp));
	ppp_pass = nvram_safe_get(strcat_r(prefix, "pppoe_passwd", tmp));

	/* do not authenticate peer and do not use eap */
	fprintf(fp, "noauth\n");
	fprintf(fp, "refuse-eap\n");
	fprintf(fp, "user '%s'\n", ppp_user);
	fprintf(fp, "password '%s'\n", ppp_pass);

	if (nvram_match(strcat_r(prefix, "proto", tmp), "pptp"))
	{
		fprintf(fp, "plugin pptp.so\n");
		fprintf(fp, "pptp_server '%s'\n",
			nvram_invmatch("wan_heartbeat_x", "") ?
			nvram_safe_get("wan_heartbeat_x") :
			nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp)));
		
		pptp_mpp = nvram_safe_get(strcat_r(prefix, "pptp_options_x", tmp));
		if (strcmp(pptp_mpp, "-mppc") == 0 || strcmp(pptp_mpp, "nomppe") == 0)
			pptp_mpp = "nomppe nomppc";
		
		/* see KB Q189595 -- historyless & mtu */
		fprintf(fp, "nomppe-stateful %s mtu 1400\n", pptp_mpp);
	} else {
		fprintf(fp, "nomppe nomppc\n");
	}

	if (nvram_match(strcat_r(prefix, "proto", tmp), "pppoe"))
	{
		fprintf(fp, "plugin rp-pppoe.so");
		
		if (nvram_invmatch(strcat_r(prefix, "pppoe_service", tmp), "")) {
			fprintf(fp, " rp_pppoe_service '%s'",
				nvram_safe_get(strcat_r(prefix, "pppoe_service", tmp)));
		}
		
		if (nvram_invmatch(strcat_r(prefix, "pppoe_ac", tmp), "")) {
			fprintf(fp, " rp_pppoe_ac '%s'",
				nvram_safe_get(strcat_r(prefix, "pppoe_ac", tmp)));
		}
		
		fprintf(fp, " nic-%s\n", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
		
		fprintf(fp, "mru %s mtu %s\n",
			nvram_safe_get(strcat_r(prefix, "pppoe_mru", tmp)),
			nvram_safe_get(strcat_r(prefix, "pppoe_mtu", tmp)));
	}

	if (	nvram_get_int(strcat_r(prefix, "pppoe_idletime", tmp)) &&
		nvram_match(strcat_r(prefix, "pppoe_demand", tmp), "1")	)
	{
		fprintf(fp, "idle %s ", nvram_safe_get(strcat_r(prefix, "pppoe_idletime", tmp)));
		if (!nvram_match(strcat_r(prefix, "pppoe_txonly_x", tmp), "0")) {
			fprintf(fp, "tx_only ");
		}
		fprintf(fp, "demand\n");
	}

	fprintf(fp, "maxfail 0\n");	// pppd re-call count (0=infinite)
	fprintf(fp, "holdoff 10\n");	// pppd re-call time(10s)

	if (!nvram_match(strcat_r(prefix, "dnsenable_x", tmp), "0"))
		fprintf(fp, "usepeerdns\n");

	if (!nvram_match(strcat_r(prefix, "proto", tmp), "l2tp"))
		fprintf(fp, "persist\n");

	fprintf(fp, "ipcp-accept-remote ipcp-accept-local noipdefault\n");
	fprintf(fp, "ktune\n");

	/* pppoe set these options automatically */
	/* looks like pptp also likes them */
	fprintf(fp, "default-asyncmap nopcomp noaccomp\n");

	/* pppoe disables "vj bsdcomp deflate" automagically */
	/* ccp should still be enabled - mppe/mppc requires this */
	fprintf(fp, "novj nobsdcomp nodeflate\n");

	/* echo failures (10*10s) */
	fprintf(fp, "lcp-echo-interval 10\n");
	fprintf(fp, "lcp-echo-failure 10\n");

	if (nvram_match("wan_pppoe_lcpa", "1"))
	{
		fprintf(fp, "lcp-echo-adaptive\n");
	}

	fprintf(fp, "unit %d\n", unit);

	/* user specific options */
	fprintf(fp, "%s\n", nvram_safe_get(strcat_r(prefix, "pppoe_options_x", tmp)));

#if defined (USE_IPV6)
	/* Enable IPv6CP */
	if (is_wan_ipv6_type_sit() == 0 && is_wan_ipv6_if_ppp())
		fprintf(fp, "+ipv6\n");
#endif
	fclose(fp);

	if (nvram_match(strcat_r(prefix, "proto", tmp), "l2tp"))
	{
		if (nvram_match("wan_l2tpd", "0"))
		{
			svcs[0] = "xl2tpd";
			kill_services(svcs, 5, 1);
			
			nvram_set("l2tp_cli_t", "1");
			
			restart_xl2tpd();
		}
		else
		{
			svcs[0] = "l2tpd";
			kill_services(svcs, 5, 1);
			
			nvram_set("l2tp_cli_t", "0");
			
			if (write_rpl2tp_conf() < 0)
				return -1;
			
			/* launch l2tp */
			eval("/usr/sbin/l2tpd");
			
			sleep(1);
			
			/* start-session */
			system("/usr/sbin/l2tp-control \"start-session 0.0.0.0\"");
		}
	}
	else
	{
		eval("/usr/sbin/pppd", "file", options);
	}

	return 0;
}

int
ppp_ifunit(char *ifname)
{
	if (strncmp(ifname, "ppp", 3))
		return -1;
	if (!isdigit(ifname[3]))
		return -1;
	return atoi(&ifname[3]);
}

int
ipup_main(int argc, char **argv)
{
	FILE *fp;
	char *wan_ifname = safe_getenv("IFNAME");
	char *value;
	char buf[256];
	int unit;
	char tmp[100], prefix[16];

	umask(0000);
 
	if ((unit = ppp_ifunit(wan_ifname)) < 0)
		return -1;
	
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	/* Touch connection file */
	if (!(fp = fopen(strcat_r("/tmp/ppp/link.", wan_ifname, tmp), "a"))) {
		perror(tmp);
		return errno;
	}
	fclose(fp);

	if (!nvram_get(strcat_r(prefix, "ifname", tmp)))
		return -1;

	if ((value = getenv("IPLOCAL"))) {
		ifconfig(wan_ifname, IFUP,
			 value, "255.255.255.255");
		nvram_set(strcat_r(prefix, "ipaddr", tmp), value);
		nvram_set(strcat_r(prefix, "netmask", tmp), "255.255.255.255");
	}

	if ((value = getenv("IPREMOTE")))
		nvram_set(strcat_r(prefix, "gateway", tmp), value);
	strcpy(buf, "");
	if (getenv("DNS1"))
		sprintf(buf, "%s", getenv("DNS1"));
	if (getenv("DNS2"))
		sprintf(buf + strlen(buf), "%s%s", strlen(buf) ? " " : "", getenv("DNS2"));
	nvram_set(strcat_r(prefix, "dns", tmp), buf);

	wan_up(wan_ifname);

	logmessage(nvram_safe_get("wan_proto_t"), "connect to ISP");

	return 0;
}

int
ipdown_main(int argc, char **argv)
{
	char *wan_ifname = safe_getenv("IFNAME");
	int unit;
	char tmp[100], prefix[16];

	umask(0000);

	if ((unit = ppp_ifunit(wan_ifname)) < 0)
		return -1;

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	if (!nvram_get(strcat_r(prefix, "ifname", tmp)))
		return -1;

	wan_down(wan_ifname);

	unlink(strcat_r("/tmp/ppp/link.", wan_ifname, tmp));

	preset_wan_routes(wan_ifname);

	logmessage(nvram_safe_get("wan_proto_t"), "Disconnected");

	return 0;
}

#if defined (USE_IPV6)
int
ipv6up_main(int argc, char **argv)
{
	char *wan_ifname;

	wan_ifname = safe_getenv("IFNAME");
	if (ppp_ifunit(wan_ifname) < 0)
		return -1;

	if (!is_wan_ipv6_if_ppp())
		return 0;

	wan6_up(wan_ifname);

	return 0;
}

int
ipv6down_main(int argc, char **argv)
{
	char *wan_ifname;

	wan_ifname = safe_getenv("IFNAME");
	if (ppp_ifunit(wan_ifname) < 0)
		return -1;

	if (!is_wan_ipv6_if_ppp())
		return 0;

	wan6_down(wan_ifname);

	update_resolvconf(0, 0);

	return 0;
}
#endif

int
ipup_vpns_main(int argc, char **argv)
{
	FILE *fp;
	int i_cast;
	char *peer_name;
	char *script_name = "/etc/storage/vpns_client_script.sh";

	if (argc < 7)
		return -1;

	peer_name = safe_getenv("PEERNAME");

	logmessage("vpn server", "peer connected - ifname: %s, local IP: %s, remote IP: %s, login: %s",
			argv[1], argv[5], argv[6], peer_name);

	fp = fopen("/tmp/vpns.leases", "a+");
	if (fp)
	{
		fprintf(fp, "%s %s %s %s\n", argv[1], argv[5], argv[6], peer_name);
		fclose(fp);
	}

	if (!pids("bcrelay"))
	{
		i_cast = nvram_get_int("vpns_cast");
		if (i_cast == 1 || i_cast == 3)
			eval("/usr/sbin/bcrelay", "-d", "-i", IFNAME_BR, "-o", "ppp[1-2][0-9]", "-n");
		if (i_cast == 2 || i_cast == 3)
			eval("/usr/sbin/bcrelay", "-d", "-i", "ppp[1-2][0-9]", "-o", IFNAME_BR, "-n");
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
	char ifname[16], addr_l[32], addr_r[32], peer_name[64], peer_info[128];
	char *clients_l1 = "/tmp/vpns.leases";
	char *clients_l2 = "/tmp/.vpns.leases";
	char *script_name = "/etc/storage/vpns_client_script.sh";
	char *svcs[] = { "bcrelay", NULL };

	if (argc < 2)
		return -1;

	logmessage("vpn server", "peer disconnected - ifname: %s", argv[1]);

	peer_info[0] = 0;
	i_clients = 0;
	fp1 = fopen(clients_l1, "r");
	fp2 = fopen(clients_l2, "w");
	if (fp1)
	{
		while(fscanf(fp1, "%s %s %s %[^\n]\n", ifname, addr_l, addr_r, peer_name) == 4)
		{
			if (strcmp(ifname, argv[1]))
			{
				i_clients++;
				if (fp2)
					fprintf(fp2, "%s %s %s %s\n", ifname, addr_l, addr_r, peer_name);
			}
			else
			{
				snprintf(peer_info, sizeof(peer_info), " %s %s %s", addr_l, addr_r, peer_name);
			}
		}
		
		fclose(fp1);
	}

	if (fp2)
	{
		fclose(fp2);
		rename(clients_l2, clients_l1);
		unlink(clients_l2);
	}

	if (check_if_file_exist(script_name))
		doSystem("%s %s %s%s", script_name, "down", argv[1], peer_info);

	if (i_clients == 0 && pids(svcs[0]))
		kill_services(svcs, 3, 1);

	return 0;
}

