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
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <shutils.h>
#include <nvram/bcmnvram.h>
#include <rc.h>
#include <netconf.h>
#include <nvparse.h>

int
start_bpalogin(void)
{
	FILE *fp;
	int ret;
	char authserver[20];
	char authdomain[80];
	char buf[254];

	if (!nvram_match("wan_heartbeat_x", "")) {
		snprintf(authserver, sizeof(authserver), "%s", nvram_safe_get("wan_heartbeat_x"));
		snprintf(authdomain, sizeof(authdomain), "%s", "");
	}
	else if (!nvram_match("wan0_domain", " ") && !nvram_match("wan0_domain", ""))
	{	
		snprintf(authserver, sizeof(authserver), "%s", "sm-server");
		snprintf(authdomain, sizeof(authdomain), "%s", nvram_safe_get("wan0_domain"));
	}
#ifdef REMOVE
	else if (nvram_match("wan0_domain", "nsw.bigpond.net.au")) {	    // NSW
		snprintf(authserver, sizeof(authserver), "%s", "spr3");
		snprintf(authdomain, sizeof(authdomain), "%s", "nsw-remote.bigpond.net.au");
	}
	else if (nvram_match("wan0_domain", "vic.bigpond.net.au")) {	   // Victoria
		snprintf(authserver, sizeof(authserver), "%s", "mer3");
		snprintf(authdomain, sizeof(authdomain), "%s", "vic-remote.bigpond.net.au");
	}
	else if (nvram_match("wan0_domain","qld.bigpond.net.au")) {	    // Queensland
		snprintf(authserver, sizeof(authserver), "%s", "bcr3");
		snprintf(authdomain, sizeof(authdomain), "%s", "qld-remote.bigpond.net.au");
	}
	else if (nvram_match("wan0_domain", "sa.bigpond.net.au")) {	    // South Austrialia
		snprintf(authserver, sizeof(authserver), "%s", "afr3");
		snprintf(authdomain, sizeof(authdomain), "%s", "sa-remote.bigpond.net.au");
	}
	else if (nvram_match("wan0_domain", "wa.bigpond.net.au")) {	    // Western Austrialia
		snprintf(authserver, sizeof(authserver), "%s", "pwr3");
		snprintf(authdomain, sizeof(authdomain), "%s", "wa-remote.bigpond.net.au");
	}
#endif
	else{
		snprintf(authserver, sizeof(authserver), "%s", nvram_safe_get("wan0_gateway"));
		snprintf(authdomain, sizeof(authdomain), "%s", "");
	}

	snprintf(buf, sizeof(buf), "%s%s%s", authserver, !strcmp(authdomain,"") ? "" : ".", authdomain);

	nvram_set("hb_server_name", buf);
	
	if (!(fp = fopen("/tmp/bpalogin.conf", "w"))) {
		perror("/tmp/bpalogin.conf");
		return errno;
	}
	fprintf(fp, "username %s\n", nvram_safe_get("wan_pppoe_username"));
	fprintf(fp, "password %s\n", nvram_safe_get("wan_pppoe_passwd"));
	fprintf(fp, "authserver %s\n", authserver);
	if (strcmp(authdomain, "")) {
		fprintf(fp, "authdomain %s\n", authdomain);
	}
	fprintf(fp, "localport 5050\n");
	fprintf(fp, "logging syslog\n");
	fprintf(fp, "debuglevel 0\n");
	fprintf(fp, "minheartbeatinterval 60\n");
	fprintf(fp, "maxheartbeatinterval 420\n");
	fprintf(fp, "connectedprog bpa_connect\n"); 
	fprintf(fp, "disconnectedprog bpa_disconnect\n");
	fclose(fp);
	
	//mkdir("/tmp/ppp", 0777);
	if ((fp = fopen("/tmp/bpa_connect_success", "r"))) {
		ret = system("bpalogin -c /tmp/bpalogin.conf -t");
		fclose(fp);	
	}
	else
		ret = system("bpalogin -c /tmp/bpalogin.conf");

	return ret;
}

int
stop_bpalogin(void)
{
	int ret;
	
	ret = system("killall bpalogin");
	ret += system("killall -9 bpalogin");
	dprintf("done\n");
	
	return ret;
}
/*
int
bpa_connect_main(int argc, char **argv)
{
	FILE *fp;
	char buf[254];	
#ifdef NOIPTABLES
	netconf_filter_t filter;
	int log_level, log_accept;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int i;

	// Optionally log connections
	log_level = atoi(nvram_safe_get("log_level"));
	log_accept = (log_level & 2) ? NETCONF_LOG_ACCEPT : NETCONF_ACCEPT;
	
	memset(&filter, 0, sizeof(filter));
	strcpy(filter.match.in.name, nvram_safe_get("wan0_ifname"));
	filter.target = log_accept;
	filter.match.ipproto = IPPROTO_UDP;
	inet_aton(nvram_safe_get("wan0_ipaddr"), &filter.match.dst.ipaddr);
	inet_aton(nvram_safe_get("wan0_netmask"), &filter.match.dst.netmask);
	filter.match.dst.ports[0] = htons(atoi("5050"));
	add_forward(&filter, NETCONF_IN, log_accept);
	nvram_set("wan_auth_t", "OK");
#else
	nvram_set("wan_auth_t", "OK");
	snprintf(buf, sizeof(buf), "iptables -I INPUT -d %s -i %s -p udp --dport %d -j %s", 
		   nvram_safe_get("wan0_ipaddr"), 
		   nvram_safe_get("wan0_ifname"),
		   5050, 
		   "ACCEPT");

	system(buf);
#endif
	return TRUE;	
}

int
bpa_disconnect_main(int argc, char **argv)
{
	char tmp[100], *str;
	int pid;

#if 1 // Thanks for Oleg
	nvram_set("wan_auth_t", "FAIL");
#else

	snprintf(tmp, sizeof(tmp), "/var/run/udhcpc%d.pid", 0);
	if ((str = file2str(tmp))) {
		pid = atoi(str);
		free(str);	
		kill(pid, SIGUSR2);
	}
#endif
	return TRUE;
}
*/
