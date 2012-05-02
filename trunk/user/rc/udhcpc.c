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
 * udhcpc scripts
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
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <nvram/bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <signal.h>
#include <semaphore_mfp.h>

#include "rc.h"

static char udhcpstate[16] = "";

int start_udhcpc_wan(const char *wan_ifname, int unit, int wait_lease)
{
	char tmp[100], prefix[sizeof("wanXXXXXXXXXX_")];
	char pidfile[sizeof("/var/run/udhcpcXXXXXXXXXX.pid")];
	char *wan_hostname;
	int index;
	
	sprintf(pidfile, "/var/run/udhcpc%d.pid", unit);
	
	char *dhcp_argv[] = {
		"udhcpc",
		"-i", (char *)wan_ifname,
		"-s", "/tmp/udhcpc.script",
		"-p", pidfile,
		"-t4",
		"-T4",
		NULL,
		NULL, NULL,	/* -H wan_hostname	*/
		NULL,		/* -O routes		*/
		NULL,		/* -O staticroutes	*/
		NULL,		/* -O msstaticroutes	*/
		NULL
	};
	index = 9;		/* first NULL index	*/
	
	if (wait_lease)
	{
		dhcp_argv[index++] = "-b"; /* Background if lease is not obtained (timeout 4*4 sec) */
	}
	else
	{
		dhcp_argv[index++] = "-d"; /* Background after run (new patch for udhcpc) */
	}
	
	/* We have to trust unit */
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	
	wan_hostname = nvram_safe_get(strcat_r(prefix, "hostname", tmp));
	if (*wan_hostname) {
		dhcp_argv[index++] = "-H";
		dhcp_argv[index++] = wan_hostname;
	}
	
	if (nvram_match("dr_enable_x", "1")) {
		dhcp_argv[index++] = "-O33";	/* "routes" */
		dhcp_argv[index++] = "-O121";	/* "staticroutes" */
		dhcp_argv[index++] = "-O249";   /* "msstaticroutes" */
	}
	
	logmessage("dhcp client", "starting wan dhcp (%s) ...", wan_ifname);
	
	return _eval(dhcp_argv, NULL, 0, NULL);
}

int renew_udhcpc_wan(int unit)
{
	int pid;
	char tmp[64], *str;
	
	snprintf(tmp, sizeof(tmp), "/var/run/udhcpc%d.pid", unit);
	
	str = file2str(tmp);
	if (str) {
		pid = atoi(str);
		free(str);
		if (pid > 1)
			kill(pid, SIGUSR1);
	}
	
	return 0;
}

int release_udhcpc_wan(int unit)
{
	int pid;
	char tmp[64], *str;
	
	snprintf(tmp, sizeof(tmp), "/var/run/udhcpc%d.pid", unit);
	
	str = file2str(tmp);
	if (str) {
		pid = atoi(str);
		free(str);
		if (pid > 1)
			kill(pid, SIGUSR2);
	}
	
	return 0;
}

static int
expires(char *wan_ifname, unsigned int in)
{
	time_t now;
	FILE *fp;
	char tmp[100];
	int unit;

	if ((unit = wan_ifunit(wan_ifname)) < 0)
		return -1;
	
	time(&now);
	snprintf(tmp, sizeof(tmp), "/tmp/udhcpc%d.expires", unit); 
	if (!(fp = fopen(tmp, "w"))) {
		perror(tmp);
		return errno;
	}
	fprintf(fp, "%d", (unsigned int) now + in);
	fclose(fp);
	return 0;
}	

/* 
 * deconfig: This argument is used when udhcpc starts, and when a
 * leases is lost. The script should put the interface in an up, but
 * deconfigured state.
*/
static int
deconfig(char *wan_ifname)
{
	if (nvram_match("wan0_proto", "l2tp") || nvram_match("wan0_proto", "pptp"))
	{
		/* fix hang-up issue */
		logmessage("dhcp client", "skipping resetting IP address to 0.0.0.0");
	} else
	{
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
		
		if (wan_ifunit(wan_ifname) < 0)
		{
			nvram_set("wanx_ipaddr", "0.0.0.0");
		}
	}

	expires(wan_ifname, 0);

	wan_down(wan_ifname);

	logmessage("dhcp client", "%s: lease is lost", udhcpstate);
	wanmessage("lost IP from server");

	return 0;
}

/*
 * bound: This argument is used when udhcpc moves from an unbound, to
 * a bound state. All of the paramaters are set in enviromental
 * variables, The script should configure the interface, and set any
 * other relavent parameters (default gateway, dns server, etc).
*/

static int
bound(char *wan_ifname)	// udhcpc bound here, also call wanup
{
	char *value;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char route[sizeof("255.255.255.255/255")];
	int unit;
	int changed = 0;
	int gateway = 0;
	int lease_dur = 0;

	if ((unit = wan_ifunit(wan_ifname)) < 0) 
		strcpy(prefix, "wanx_");
	else
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	if ((value = getenv("ip"))) {
		changed = nvram_invmatch(strcat_r(prefix, "ipaddr", tmp), value);
		nvram_set(strcat_r(prefix, "ipaddr", tmp), trim_r(value));
	}
	if ((value = getenv("subnet")))
		nvram_set(strcat_r(prefix, "netmask", tmp), trim_r(value));
        if ((value = getenv("router"))) {
		gateway = 1;
		nvram_set(strcat_r(prefix, "gateway", tmp), trim_r(value));
	}
	if ((value = getenv("dns")))
		nvram_set(strcat_r(prefix, "dns", tmp), trim_r(value));
	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins", tmp), trim_r(value));
	else
		nvram_set(strcat_r(prefix, "wins", tmp), "");

	nvram_set(strcat_r(prefix, "routes", tmp), getenv("routes"));
	nvram_set(strcat_r(prefix, "routes_ms", tmp), getenv("msstaticroutes"));
	nvram_set(strcat_r(prefix, "routes_rfc", tmp), getenv("staticroutes"));
#if 0
	if ((value = getenv("hostname")))
		sethostname(trim_r(value), strlen(value) + 1);
#endif
	if ((value = getenv("domain")))
		nvram_set(strcat_r(prefix, "domain", tmp), trim_r(value));
	if ((value = getenv("lease"))) {
		nvram_set(strcat_r(prefix, "lease", tmp), trim_r(value));
		lease_dur = atoi(value);
		expires(wan_ifname, lease_dur);
	}
	
	if (!gateway) {
		foreach(route, nvram_safe_get(strcat_r(prefix, "routes_rfc", tmp)), value) {
			if (gateway) {
				nvram_set(strcat_r(prefix, "gateway", tmp), route);
				break;
			} else
				gateway = !strcmp(route, "0.0.0.0/0");
		}
	}
	
	if (changed && unit == 0)
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
	
	ifconfig(wan_ifname, IFUP,
		 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
		 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));

	wan_up(wan_ifname);

	logmessage("dhcp client", "%s (%s), IP: %s, GW: %s, lease time: %d", 
		udhcpstate, 
		wan_ifname,
		nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
		nvram_safe_get(strcat_r(prefix, "gateway", tmp)), lease_dur);
	
	wanmessage("");
	dprintf("done\n");
	return 0;
}

/*
 * renew: This argument is used when a DHCP lease is renewed. All of
 * the paramaters are set in enviromental variables. This argument is
 * used when the interface is already configured, so the IP address,
 * will not change, however, the other DHCP paramaters, such as the
 * default gateway, subnet mask, and dns server may change.
 */
static int
renew(char *wan_ifname)
{
	char *value;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int unit;
	int changed = 0;

	if ((unit = wan_ifunit(wan_ifname)) < 0)
		strcpy(prefix, "wanx_");
	else
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	
	if (!(value = getenv("subnet")) || nvram_invmatch(strcat_r(prefix, "netmask", tmp), trim_r(value)))
		return bound(wan_ifname);
	if (!(value = getenv("router")) || nvram_invmatch(strcat_r(prefix, "gateway", tmp), trim_r(value)))
		return bound(wan_ifname);
	if ((value = getenv("ip")) && nvram_invmatch(strcat_r(prefix, "ipaddr", tmp), trim_r(value)))
		return bound(wan_ifname);
	
	if ((value = getenv("dns")) && nvram_invmatch(strcat_r(prefix, "dns", tmp), trim_r(value))) {
		nvram_set(strcat_r(prefix, "dns", tmp), trim_r(value));
		changed = 1;
	}

	if ((value = getenv("wins")))
		nvram_set(strcat_r(prefix, "wins", tmp), trim_r(value));
	else
		nvram_set(strcat_r(prefix, "wins", tmp), "");
#if 0
	if ((value = getenv("hostname")))
		sethostname(trim_r(value), strlen(value) + 1);
#endif
	if ((value = getenv("domain")))
		nvram_set(strcat_r(prefix, "domain", tmp), trim_r(value));
	if ((value = getenv("lease"))) {
		nvram_set(strcat_r(prefix, "lease", tmp), trim_r(value));
		expires(wan_ifname, atoi(value));
	}
	
	if (changed)
	{
		update_resolvconf();
		
		if (unit == 0)
			update_wan_status(1);
		
		logmessage("dhcp client", "%s (%s), new dns: %s", 
			udhcpstate, 
			wan_ifname,
			nvram_safe_get(strcat_r(prefix, "dns", tmp)) );
	}
	
	wanmessage("");
	
	dprintf("done\n");
	return 0;
}

static int leasefail(char *wan_ifname)
{
	return 0;
}

static int noack(char *wan_ifname)
{
	logmessage("dhcp client", "nak", wan_ifname);
	return 0;
}

int
udhcpc_main(int argc, char **argv)
{
	char *wan_ifname;

	if (argc<2 || !argv[1])
		return EINVAL;

	wan_ifname = safe_getenv("interface");
	strcpy(udhcpstate, argv[1]);

	if (!strcmp(argv[1], "deconfig"))
		return deconfig(wan_ifname);
	else if (!strcmp(argv[1], "bound"))
		return bound(wan_ifname);
	else if (!strcmp(argv[1], "renew"))
		return renew(wan_ifname);
	else if (!strcmp(argv[1], "leasefail"))
		return leasefail(wan_ifname);
	else if (!strcmp(argv[1], "nak"))
		return noack(wan_ifname);
	else
		return 0;
}
