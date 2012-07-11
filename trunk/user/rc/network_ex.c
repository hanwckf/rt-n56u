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
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <signal.h>

#include <nvram/bcmnvram.h>
#include <netconf.h>
#include <shutils.h>

typedef unsigned char   bool;   // 1204 ham

#include <wlutils.h>
#include <nvparse.h>
#include <rc.h>
#include <nvram/bcmutils.h>

void set_ppp_limit_cpu(void)
{
	FILE *fp;
	int cpu_lim;
	char tmp[16];

	fp=fopen("/proc/sys/net/ipv4/ppp_cpu_load_limit", "w+");
	if (fp)
	{
		cpu_lim = atoi(nvram_safe_get("wan_pppoe_cpul"));
		if (cpu_lim < 0 || cpu_lim > 5000) cpu_lim = 0;
		sprintf(tmp, "%d", cpu_lim);
		fputs(tmp, fp);
		fclose(fp);
	}
}

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
		i_cli0 = atoi(nvram_safe_get("vpns_cli0"));
		i_cli1 = atoi(nvram_safe_get("vpns_cli1"));
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

#ifndef USE_RPL2TP
	if (nvram_match(strcat_r(prefix, "proto", tmp), "l2tp"))
	{
		unit = atoi(nvram_safe_get(strcat_r(prefix, "unit", tmp)));
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
#endif

	fclose(fp);

	return has_work;
}

#ifdef USE_RPL2TP
int write_rpl2tp_conf(void)
{
	FILE *fp;
	int unit;
	char tmp[100];
	char *prefix = "wan0_";
	char *l2tp_conf, *l2tp_peer;

	unit = atoi(nvram_safe_get(strcat_r(prefix, "unit", tmp)));
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
#endif

void restart_xl2tpd(void)
{
#ifndef USE_RPL2TP
	char *l2tp_conf;
	
	l2tp_conf = "/etc/xl2tpd.conf";
	
	/* check has lac or lns */
	if (write_xl2tpd_conf(l2tp_conf) < 0)
		return;
	
	chmod(l2tp_conf, 0644);
	
	/* launch xl2tpd */
	eval("/usr/sbin/xl2tpd", "-c", l2tp_conf);
#endif
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
	char *svcs[] = { 
#ifdef USE_RPL2TP
			"l2tpd",
#else
			"xl2tpd",
#endif
			NULL };

	unit = atoi(nvram_safe_get(strcat_r(prefix, "unit", tmp)));
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

	if (	atoi(nvram_safe_get(strcat_r(prefix, "pppoe_idletime", tmp))) &&
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

	fclose(fp);

	if (nvram_match(strcat_r(prefix, "proto", tmp), "l2tp"))
	{
		/* kill l2tpd/xl2tpd if exist */
		kill_services(svcs, 5, 1);
		
#ifdef USE_RPL2TP
		if (write_rpl2tp_conf() < 0)
			return -1;
		
		/* launch l2tp */
		eval("/usr/sbin/l2tpd");
		
		sleep(1);
		
		/* start-session */
		system("/usr/sbin/l2tp-control \"start-session 0.0.0.0\"");
#else
		nvram_set("l2tp_cli_t", "1");
		
		restart_xl2tpd();
#endif
	}
	else
	{
		eval("/usr/sbin/pppd", "file", options);
	}

	return 0;
}


void start_pppoe_relay(char *wan_if)
{
	if (nvram_match("wan_pppoe_relay_x", "1"))
	{
		eval("/usr/sbin/pppoe-relay", "-C", IFNAME_BR, "-S", wan_if);
	}
}
