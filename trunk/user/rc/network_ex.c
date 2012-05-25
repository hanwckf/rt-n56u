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

int start_pppd(char *prefix)
{
	if (!((nvram_match("wan0_proto", "pppoe")) || (nvram_match("wan0_proto", "pptp")) || (nvram_match("wan0_proto", "l2tp"))))
		return -1;

	int ret;
	FILE *fp;
	char options[80];
	char tmp[100];
	char *pptp_mpp;
	mode_t mask;
	char *l2tp_peer, *l2tp_host;
	char *svcs[] = { "l2tpd", NULL };
	
	sprintf(options, "/tmp/ppp/options.wan%s",
		nvram_safe_get(strcat_r(prefix, "unit", tmp)));

	mask = umask(0000);

	/* Generate options file */
	if (!(fp = fopen(options, "w"))) {
		perror(options);
		umask(mask);
		return -1;
	}

	umask(mask);

	/* do not authenticate peer and do not use eap */
	fprintf(fp, "noauth\n");
	fprintf(fp, "refuse-eap\n");
	fprintf(fp, "user '%s'\n",
		nvram_safe_get(strcat_r(prefix, "pppoe_username", tmp)));
	fprintf(fp, "password '%s'\n",
		nvram_safe_get(strcat_r(prefix, "pppoe_passwd", tmp)));

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
	
	fprintf(fp, "unit %s\n",
		nvram_safe_get(strcat_r(prefix, "unit", tmp)) ? : "0");

	/* user specific options */
	fprintf(fp, "%s\n",
		nvram_safe_get(strcat_r(prefix, "pppoe_options_x", tmp)));
	
	fclose(fp);
	
	if (nvram_match(strcat_r(prefix, "proto", tmp), "l2tp"))
	{
		l2tp_peer = nvram_safe_get("wan_heartbeat_x");
		l2tp_host = nvram_safe_get(strcat_r(prefix, "hostname", tmp));
		
		if (!(*l2tp_peer)) {
			l2tp_peer = nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp));
		}
		
		if (!(*l2tp_host)) {
			l2tp_host = "localhost";
		}
		
		mkdir_if_none("/etc/l2tp");
		
		if (!(fp = fopen("/etc/l2tp/l2tp.conf", "w"))) {
			perror(options);
			return -1;
		}
		
		fprintf(fp, "# automagically generated\n"
			"global\n\n"
			"load-handler \"sync-pppd.so\"\n"
			"load-handler \"cmd.so\"\n\n"
			"section sync-pppd\n\n"
			"lac-pppd-opts \"file %s\"\n\n"
			"section peer\n"
			"peername %s\n"
			"hostname %s\n"
			"lac-handler sync-pppd\n"
			"persist yes\n"
			"maxfail 0\n"    // l2tpd re-call count (0=infinite)
			"holdoff 10\n"   // l2tpd re-call time (10s)
			"hide-avps no\n"
			"section cmd\n\n",
			options, l2tp_peer, l2tp_host);
		
		fclose(fp);
		
		/* kill l2tpd if exist */
		kill_services(svcs, 3, 1);
		
		/* launch l2tp */
		eval("/usr/sbin/l2tpd");
		
		sleep(1);
		
		/* start-session */
		ret = system("/usr/sbin/l2tp-control \"start-session 0.0.0.0\"");
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
