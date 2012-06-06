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
 * Network services
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
#include <dirent.h>
typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;

typedef u_int64_t __u64;
typedef u_int32_t __u32;
typedef u_int16_t __u16;
typedef u_int8_t __u8;

typedef unsigned char   bool;   // 1204 ham

#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <nvram/bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <nvparse.h>
#include <rc.h>
#include <nvram/bcmutils.h>
#include <etioctl.h>
#include <bcmparams.h>
#include <net/route.h>
#include <stdarg.h>
#include "ralink.h"
#include "rtl8367m.h"
#include <semaphore_mfp.h>
#include <linux/rtl8367m_drv.h>

#define MAX_MAC_NUM	64


static int mac_num;
static char mac_clone[MAX_MAC_NUM][18];

void 
kill_services(char* svc_name[], int wtimeout, int forcekill)
{
	int i, k, i_waited, i_killed;
	
	if (wtimeout < 1)
		wtimeout = 1;
	if (wtimeout > 30)
		wtimeout = 30;
	
	for (i=0;svc_name[i] && *svc_name[i];i++)
	{
		doSystem("killall -q %s", svc_name[i]);
	}
	
	for (k=0;k<wtimeout;k++)
	{
		i_waited = 0;
		for (i=0;svc_name[i] && *svc_name[i];i++)
		{
			if (pids(svc_name[i]))
			{
				i_waited = 1;
				break;
			}
		}
		
		if (!i_waited)
			break;
		
		sleep(1);
	}
	
	if (forcekill)
	{
		i_killed = 0;
		for (i=0;svc_name[i] && *svc_name[i];i++)
		{
			if (pids(svc_name[i]))
			{
				i_killed = 1;
				doSystem("killall -SIGKILL %s", svc_name[i]);
			}
		}
		if (i_killed)
		{
			sleep(1);
		}
	}
}

void
config_loopback(void)
{
	/* Bring up loopback interface */
	ifconfig("lo", IFUP, "127.0.0.1", "255.0.0.0");

	/* Add to routing table */
	route_add("lo", 0, "127.0.0.0", "0.0.0.0", "255.0.0.0");
}

int
ifconfig(char *name, int flags, char *addr, char *netmask)
{
	int s;
	struct ifreq ifr;
	struct in_addr in_addr, in_netmask, in_broadaddr;

	/* Open a raw socket to the kernel */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		goto err;

	/* Set interface name */
	strncpy(ifr.ifr_name, name, IFNAMSIZ);

	/* Set interface flags */
	ifr.ifr_flags = flags;
	if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0)
		goto err;

	/* Set IP address */
	if (addr) {
		inet_aton(addr, &in_addr);
		sin_addr(&ifr.ifr_addr).s_addr = in_addr.s_addr;
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFADDR, &ifr) < 0)
			goto err;
	}
	/* Set IP netmask and broadcast */
	if (addr && netmask) {
		inet_aton(netmask, &in_netmask);
		sin_addr(&ifr.ifr_netmask).s_addr = in_netmask.s_addr;
		ifr.ifr_netmask.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFNETMASK, &ifr) < 0)
			goto err;

		in_broadaddr.s_addr = (in_addr.s_addr & in_netmask.s_addr) | ~in_netmask.s_addr;
		sin_addr(&ifr.ifr_broadaddr).s_addr = in_broadaddr.s_addr;
		ifr.ifr_broadaddr.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFBRDADDR, &ifr) < 0)
			goto err;
	}

	close(s);

	return 0;

 err:
	close(s);
	return errno;
}

static int
route_manip(int cmd, char *name, int metric, char *dst, char *gateway, char *genmask)
{
	int s;
	struct rtentry rt;

	/* Open a raw socket to the kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		goto err;
	}

	/* Fill in rtentry */
	memset(&rt, 0, sizeof(rt));
	if (dst) {
		inet_aton(dst, &sin_addr(&rt.rt_dst));
	}
	if (gateway) {
		inet_aton(gateway, &sin_addr(&rt.rt_gateway));
	}
	if (genmask) {	
		inet_aton(genmask, &sin_addr(&rt.rt_genmask));
	}
	rt.rt_metric = metric;
	rt.rt_flags = RTF_UP;
	if (sin_addr(&rt.rt_gateway).s_addr) {
		rt.rt_flags |= RTF_GATEWAY;
	}
	if (sin_addr(&rt.rt_genmask).s_addr == INADDR_BROADCAST) {
		rt.rt_flags |= RTF_HOST;
	}
	rt.rt_dev = name;

	/* Force address family to AF_INET */
	rt.rt_dst.sa_family = AF_INET;
	rt.rt_gateway.sa_family = AF_INET;
	rt.rt_genmask.sa_family = AF_INET;

	if (ioctl(s, cmd, &rt) < 0) {
		goto err;
	}

	close(s);
	return 0;

 err:
	close(s);
	perror(name);
	return errno;
}

int
route_add(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	return route_manip(SIOCADDRT, name, metric, dst, gateway, genmask);
}

int
route_del(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	return route_manip(SIOCDELRT, name, metric, dst, gateway, genmask);
}

static int
add_routes(char *prefix, char *var, char *ifname)
{
	char word[80], *next;
	char *ipaddr, *netmask, *gateway, *metric;
	char tmp[100];

	foreach(word, nvram_safe_get(strcat_r(prefix, var, tmp)), next) {

		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;
		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;
		if (inet_addr_(gateway) == INADDR_ANY) 			// oleg patch
			gateway = nvram_safe_get("wanx_gateway");	// oleg patch

		//route_add(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
		route_add(ifname, 0, ipaddr, gateway, netmask);
	}

	return 0;
}

static void	// oleg patch , add 
add_wanx_routes(char *prefix, char *ifname, int metric)
{
	char *routes, *tmp;
	char buf[30];
	struct in_addr mask;
	char *ipaddr, *gateway;
	int bits;
	char netmask[] = "255.255.255.255";
	
	if (!nvram_match("dr_enable_x", "1"))
		return;
	
	/* routes */
	routes = strdup(nvram_safe_get(strcat_r(prefix, "routes", buf)));
	for (tmp = routes; tmp && *tmp; )
	{
		ipaddr = strsep(&tmp, "/");
		gateway = strsep(&tmp, " ");
		if (gateway && inet_addr_(ipaddr) != INADDR_ANY) {
			route_add(ifname, metric + 1, ipaddr, gateway, netmask);
		}
	}
	free(routes);
	
	/* rfc3442 or ms classless static routes */
	routes = nvram_safe_get(strcat_r(prefix, "routes_rfc", buf));
	if (!*routes)
		routes = nvram_safe_get(strcat_r(prefix, "routes_ms", buf));
	routes = strdup(routes);
	for (tmp = routes; tmp && *tmp; )
	{
		ipaddr  = strsep(&tmp, "/");
		bits    = atoi(strsep(&tmp, " "));
		gateway = strsep(&tmp, " ");

		if (gateway && bits > 0 && bits <= 32)
		{
			mask.s_addr = htonl(0xffffffff << (32 - bits));
			strcpy(netmask, inet_ntoa(mask));
			route_add(ifname, metric + 1, ipaddr, gateway, netmask);
		}
	}
	free(routes);
}

static int
del_routes(char *prefix, char *var, char *ifname)
{
	char word[80], *next;
	char *ipaddr, *netmask, *gateway, *metric;
	char tmp[100];
	
	foreach(word, nvram_safe_get(strcat_r(prefix, var, tmp)), next) {
		dprintf("add %s\n", word);
		
		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;

		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;

		if (inet_addr_(gateway) == INADDR_ANY) 	// oleg patch
			gateway = nvram_safe_get("wanx_gateway");

		dprintf("add %s\n", ifname);
		
		route_del(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
	}

	return 0;
}

void
stop_igmpproxy(void)
{
	char* svcs[] = { "udpxy", "igmpproxy", NULL };
	kill_services(svcs, 3, 1);
}

void	// oleg patch , add
start_igmpproxy(char *wan_ifname)
{
	static char *igmpproxy_conf = "/etc/igmpproxy.conf";
	FILE *fp;
	char *altnet = nvram_safe_get("mr_altnet_x");
	char *altnet_mask;
	
	// Allways close old instance of igmpproxy and udpxy (interface may changed)
	stop_igmpproxy();
	
	if (atoi(nvram_safe_get("udpxy_enable_x")))
	{
		eval("/usr/sbin/udpxy", 
			"-a", nvram_safe_get("lan_ifname") ? : IFNAME_BR,
			"-m", wan_ifname, 
			"-p", nvram_safe_get("udpxy_enable_x"),
			"-B", "65536",
			"-c", "5"
			);
	}
	
	if (!nvram_match("mr_enable_x", "1"))
		return;
	
	printf("start igmpproxy [%s]\n", wan_ifname);   // tmp test

	if ((fp = fopen(igmpproxy_conf, "w")) == NULL) {
		perror(igmpproxy_conf);
		return;
	}

	if (altnet && strlen(altnet) > 0)
		altnet_mask = altnet;
	else
		altnet_mask = "0.0.0.0/0";
	
	printf("start igmpproxy: altnet_mask = %s\n", altnet_mask);	// tmp test
	
	fprintf(fp, "# automagically generated from web settings\n"
		"quickleave\n\n"
		"phyint %s upstream  ratelimit 0  threshold 1\n"
		"\taltnet %s\n\n"
		"phyint %s downstream  ratelimit 0  threshold 1\n\n",
		wan_ifname, 
		altnet_mask, 
		nvram_safe_get("lan_ifname") ? : IFNAME_BR);
	fclose(fp);
	eval("/bin/igmpproxy", igmpproxy_conf);
}

static int
add_lan_routes(char *lan_ifname)
{
	return add_routes("lan_", "route", lan_ifname);
}

static int
del_lan_routes(char *lan_ifname)
{
	return del_routes("lan_", "route", lan_ifname);
}

int 
is_ap_mode(void)
{
	if ((nvram_match("wan_nat_x", "0")) && (nvram_match("wan_route_x", "IP_Bridged")))
		return 1;
	else
		return 0;
}


void 
stop_wifi_all_wl(void)
{
	int i;
	char ifname_wifi[8];
	
	// stop APCLi
	sprintf(ifname_wifi, "apcli%d", 0);
	ifconfig(ifname_wifi, 0, NULL, NULL);
	
	// stop WDS (4 interfaces)
	for (i=3; i>=0; i--)
	{
		sprintf(ifname_wifi, "wds%d", i);
		ifconfig(ifname_wifi, 0, NULL, NULL);
	}
	
	// stop AP (guest + main)
	for (i=1; i>=0; i--)
	{
		sprintf(ifname_wifi, "ra%d", i);
		ifconfig(ifname_wifi, 0, NULL, NULL);
	}
}

void 
stop_wifi_all_rt(void)
{
	int i;
	char ifname_wifi[8];
	
	// stop APCLi
	sprintf(ifname_wifi, "apclii%d", 0);
	ifconfig(ifname_wifi, 0, NULL, NULL);
	
	// stop WDS (4 interfaces)
	for (i=3; i>=0; i--)
	{
		sprintf(ifname_wifi, "wdsi%d", i);
		ifconfig(ifname_wifi, 0, NULL, NULL);
	}
	
	// stop AP (guest + main)
	for (i=1; i>=0; i--)
	{
		sprintf(ifname_wifi, "rai%d", i);
		ifconfig(ifname_wifi, 0, NULL, NULL);
	}
}

void 
start_wifi_ap_wl(int radio_on)
{
	int i;
	int wl_mode_x = atoi(nvram_safe_get("wl_mode_x"));
	char ifname_ap[8];
	
	// check WDS only or Radio disabled
	if (wl_mode_x == 1 || !radio_on)
	{
		for (i=1; i>=0; i--)
		{
			sprintf(ifname_ap, "ra%d", i);
			doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		}
	}
	
	if (!radio_on)
		return;
	
	sprintf(ifname_ap, "ra%d", 0);
	ifconfig(ifname_ap, IFUP, NULL, NULL);
	if (nvram_match("wl_mbss_x", "1"))
	{
		sprintf(ifname_ap, "ra%d", 1);
		ifconfig(ifname_ap, IFUP, NULL, NULL);
	}
	
	// check AP or Hybrid
	if (wl_mode_x != 1)
	{
		sprintf(ifname_ap, "ra%d", 0);
		doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		
		if (nvram_match("wl_mbss_x", "1"))
		{
			sprintf(ifname_ap, "ra%d", 1);
			doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		}
	}
}

void 
start_wifi_ap_rt(int radio_on)
{
	int i;
	int rt_mode_x = atoi(nvram_safe_get("rt_mode_x"));
	char ifname_ap[8];
	
	// check WDS only or Radio disabled
	if (rt_mode_x == 1 || !radio_on)
	{
		for (i=1; i>=0; i--)
		{
			sprintf(ifname_ap, "rai%d", i);
			doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		}
	}
	
	if (!radio_on)
		return;
	
	sprintf(ifname_ap, "rai%d", 0);
	ifconfig(ifname_ap, IFUP, NULL, NULL);
	if (nvram_match("rt_mbss_x", "1"))
	{
		sprintf(ifname_ap, "rai%d", 1);
		ifconfig(ifname_ap, IFUP, NULL, NULL);
	}
	
	// check AP or Hybrid
	if (rt_mode_x != 1)
	{
		sprintf(ifname_ap, "rai%d", 0);
		doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		
		if (nvram_match("rt_mbss_x", "1"))
		{
			sprintf(ifname_ap, "rai%d", 1);
			doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_ap);
		}
	}
}


void
start_wifi_wds_wl(int radio_on)
{
	int i;
	char ifname_wds[8];
	
	if (radio_on && nvram_invmatch("wl_mode_x", "0") && nvram_invmatch("sw_mode_ex", "2"))
	{
		for (i=0; i<4; i++)
		{
			sprintf(ifname_wds, "wds%d", i);
			ifconfig(ifname_wds, IFUP, NULL, NULL);
			doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_wds);
		}
	}
	else
	{
		for (i=3; i>=0; i--)
		{
			sprintf(ifname_wds, "wds%d", i);
			doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_wds);
		}
	}
}

void
start_wifi_wds_rt(int radio_on)
{
	int i;
	char ifname_wds[8];
	
	if (radio_on && nvram_invmatch("rt_mode_x", "0") && nvram_invmatch("sw_mode_ex", "2"))
	{
		for (i=0; i<4; i++)
		{
			sprintf(ifname_wds, "wdsi%d", i);
			ifconfig(ifname_wds, IFUP, NULL, NULL);
			doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_wds);
		}
	}
	else
	{
		for (i=3; i>=0; i--)
		{
			sprintf(ifname_wds, "wdsi%d", i);
			doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_wds);
		}
	}
}

void
start_wifi_apcli_wl(int radio_on)
{
#if 0
	char *ifname_apcli = "apcli0";
	
	if (radio_on && nvram_match("sw_mode_ex", "2") && nvram_invmatch("sta_ssid", ""))
	{
		ifconfig(ifname_apcli, IFUP, NULL, NULL);
		doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_apcli);
	}
	else
	{
		doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_apcli);
	}
#endif
}

void
start_wifi_apcli_rt(int radio_on)
{
#if 0
	char *ifname_apcli = "apclii0";
	
	if (radio_on && nvram_match("sw_mode_ex", "2") && nvram_invmatch("rt_sta_ssid", ""))
	{
		ifconfig(ifname_apcli, IFUP, NULL, NULL);
		doSystem("brctl addif %s %s 2>/dev/null", IFNAME_BR, ifname_apcli);
	}
	else
	{
		doSystem("brctl delif %s %s 2>/dev/null", IFNAME_BR, ifname_apcli);
	}
#endif
}


void
restart_wifi_wl(int radio_on, int need_reload_conf)
{
	stop_lltd();
	stop_8021x_wl();
	
	stop_wifi_all_wl();
	
	if (need_reload_conf)
	{
		gen_ralink_config_wl();
		nvram_set("reload_svc_wl", "1");
	}
	
	start_wifi_ap_wl(radio_on);
	start_wifi_wds_wl(radio_on);
	start_wifi_apcli_wl(radio_on);
	
	start_8021x_wl();
	start_lltd();
}

void
restart_wifi_rt(int radio_on, int need_reload_conf)
{
	stop_lltd();
	stop_8021x_rt();
	
	stop_wifi_all_rt();
	
	if (need_reload_conf)
	{
		gen_ralink_config_rt();
		nvram_set("reload_svc_rt", "1");
	}
	
	start_wifi_ap_rt(radio_on);
	start_wifi_wds_rt(radio_on);
	start_wifi_apcli_rt(radio_on);
	
	start_8021x_rt();
	start_lltd();
}


void 
bridge_init(void)
{
	int ap_mode = is_ap_mode();
	char *lan_hwaddr = nvram_safe_get("lan_hwaddr");
	
	doSystem("ifconfig %s hw ether %s", IFNAME_MAC, lan_hwaddr);
	ifconfig(IFNAME_MAC, IFUP, NULL, NULL);
	
#ifdef USE_SINGLE_MAC
	if (!ap_mode)
	{
		/* create VLAN1/2  */
		doSystem("vconfig add %s %d", IFNAME_MAC, 1);
		doSystem("vconfig add %s %d", IFNAME_MAC, 2);
		doSystem("ifconfig %s hw ether %s txqueuelen %d", IFNAME_LAN, lan_hwaddr, 1000);
		doSystem("ifconfig %s txqueuelen %d", IFNAME_WAN, 1000);
		ifconfig(IFNAME_LAN, IFUP, NULL, NULL);
	}
#endif
	doSystem("brctl addbr %s", IFNAME_BR);
	doSystem("brctl setfd %s 0.1", IFNAME_BR);
	doSystem("brctl sethello %s 0.1", IFNAME_BR);
	doSystem("brctl stp %s %d", IFNAME_BR, (ap_mode) ? 0 : 1);
	doSystem("ifconfig %s hw ether %s txqueuelen %d", IFNAME_BR, lan_hwaddr, 1000);
	
	switch_config_vlan(1);
	
	if (!ap_mode)
	{
		ifconfig(IFNAME_WAN, 0, NULL, NULL);
		doSystem("brctl addif %s %s", IFNAME_BR, IFNAME_LAN);
	}
	else
	{
#ifdef USE_SINGLE_MAC
		doSystem("brctl addif %s %s", IFNAME_BR, IFNAME_MAC);
#else
		wan_mac_config();
		ifconfig(IFNAME_WAN, IFUP, NULL, NULL);
		doSystem("brctl addif %s %s", IFNAME_BR, IFNAME_LAN);
		doSystem("brctl addif %s %s", IFNAME_BR, IFNAME_WAN);
#endif
	}
	
	switch_config_base();
	switch_config_storm();
	switch_config_link();
	
	kill_pidfile_s("/var/run/linkstatus_monitor.pid", SIGALRM);
	
	int wl_radio_on = atoi(nvram_safe_get("wl_radio_x"));
	int rt_radio_on = atoi(nvram_safe_get("rt_radio_x"));
	
	start_wifi_ap_wl(wl_radio_on);
	start_wifi_wds_wl(wl_radio_on);
	start_wifi_apcli_wl(wl_radio_on);
	
	start_wifi_ap_rt(rt_radio_on);
	start_wifi_wds_rt(rt_radio_on);
	start_wifi_apcli_rt(rt_radio_on);
	
	ifconfig(IFNAME_BR, IFUP, nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));
	
	nvram_set("reload_svc_wl", "0");
	nvram_set("reload_svc_rt", "0");
	
	/* clean up... */
	nvram_unset("wan0_hwaddr_x");
}

void
wan_mac_config(void)
{
	if (nvram_invmatch("wan_hwaddr", ""))
		doSystem("ifconfig %s hw ether %s", IFNAME_WAN, nvram_safe_get("wan_hwaddr"));
	else
		doSystem("ifconfig %s hw ether %s", IFNAME_WAN, nvram_safe_get("lan_hwaddr"));
}

void 
switch_config_link(void)
{
	int i_flow_mode;
	int i_link_mode;
	
	// WAN
	i_flow_mode = atoi(nvram_safe_get("ether_flow_wan"));
	if (i_flow_mode < 0 || i_flow_mode > 2)
		i_flow_mode = 0;
	i_link_mode = atoi(nvram_safe_get("ether_link_wan"));
	if (i_link_mode < 0 || i_link_mode > 5)
		i_link_mode = 0;
	
	phy_link_port_wan(i_link_mode, i_flow_mode);
	
	// LAN1
	i_flow_mode = atoi(nvram_safe_get("ether_flow_lan1"));
	if (i_flow_mode < 0 || i_flow_mode > 2)
		i_flow_mode = 0;
	i_link_mode = atoi(nvram_safe_get("ether_link_lan1"));
	if (i_link_mode < 0 || i_link_mode > 5)
		i_link_mode = 0;
	
	phy_link_port_lan1(i_link_mode, i_flow_mode);
	
	// LAN2
	i_flow_mode = atoi(nvram_safe_get("ether_flow_lan2"));
	if (i_flow_mode < 0 || i_flow_mode > 2)
		i_flow_mode = 0;
	i_link_mode = atoi(nvram_safe_get("ether_link_lan2"));
	if (i_link_mode < 0 || i_link_mode > 5)
		i_link_mode = 0;
	
	phy_link_port_lan2(i_link_mode, i_flow_mode);
	
	// LAN3
	i_flow_mode = atoi(nvram_safe_get("ether_flow_lan3"));
	if (i_flow_mode < 0 || i_flow_mode > 2)
		i_flow_mode = 0;
	i_link_mode = atoi(nvram_safe_get("ether_link_lan3"));
	if (i_link_mode < 0 || i_link_mode > 5)
		i_link_mode = 0;
	
	phy_link_port_lan3(i_link_mode, i_flow_mode);
	
	// LAN4
	i_flow_mode = atoi(nvram_safe_get("ether_flow_lan4"));
	if (i_flow_mode < 0 || i_flow_mode > 2)
		i_flow_mode = 0;
	i_link_mode = atoi(nvram_safe_get("ether_link_lan4"));
	if (i_link_mode < 0 || i_link_mode > 5)
		i_link_mode = 0;
	
	phy_link_port_lan4(i_link_mode, i_flow_mode);
}


void 
switch_config_base(void)
{
	int i_ether_led0 = atoi(nvram_safe_get("ether_led0"));
	int i_ether_led1 = atoi(nvram_safe_get("ether_led1"));
	int i_ether_jumbo = atoi(nvram_safe_get("ether_jumbo"));
	int i_ether_green = atoi(nvram_safe_get("ether_green"));

	phy_led_mode_green(i_ether_led0);
	phy_led_mode_yellow(i_ether_led1);
	phy_jumbo_frames(i_ether_jumbo);
	phy_green_ethernet(i_ether_green);
}

void 
switch_config_storm(void)
{
	int controlrate_unknown_unicast;
	int controlrate_unknown_multicast;
	int controlrate_multicast;
	int controlrate_broadcast;

	if (is_ap_mode())
		return;

	/* unknown unicast storm control */
	controlrate_unknown_unicast = atoi(nvram_safe_get("controlrate_unknown_unicast"));
	if (controlrate_unknown_unicast <= 0 || controlrate_unknown_unicast > 1024)
		controlrate_unknown_unicast = 1024;
	
	/* unknown multicast storm control */
	controlrate_unknown_multicast = atoi(nvram_safe_get("controlrate_unknown_multicast"));
	if (controlrate_unknown_multicast <= 0 || controlrate_unknown_multicast > 1024)
		controlrate_unknown_multicast = 1024;

	/* multicast storm control */
	controlrate_multicast = atoi(nvram_safe_get("controlrate_multicast"));
	if (controlrate_multicast <= 0 || controlrate_multicast > 1024)
		controlrate_multicast = 1024;

	/* broadcast storm control */
	controlrate_broadcast = atoi(nvram_safe_get("controlrate_broadcast"));
	if (controlrate_broadcast <= 0 || controlrate_broadcast > 1024)
		controlrate_broadcast = 1024;
	
	phy_storm_unicast_unknown(controlrate_unknown_unicast);
	phy_storm_multicast_unknown(controlrate_unknown_multicast);
	phy_storm_multicast(controlrate_multicast);
	phy_storm_broadcast(controlrate_broadcast);
}


void
switch_config_vlan(int first_call)
{
	int bridge_mode, bwan_isolation, is_vlan_filter;
	int vlan_vid[5] = {0};
	int vlan_pri[5] = {0};
	int vlan_tag[5] = {0};
	int vlan_fid;
	unsigned int vlan_member, vlan_untag, accept_tagged;
	
	if (is_ap_mode())
	{
		if (first_call)
		{
#ifdef USE_SINGLE_MAC
			/* set bridge mode to LLLLL */
			phy_bridge_mode(RTL8367M_WAN_BRIDGE_DISABLE_WAN, RTL8367M_WAN_BWAN_ISOLATION_NONE);
#else
			/* accept tagged and untagged frames for all ports */
			phy_vlan_accept_port_mode(RTL8367M_VLAN_ACCEPT_FRAMES_ALL, RTL8367M_PORTMASK_WAN |
										   RTL8367M_PORTMASK_LAN1 |
										   RTL8367M_PORTMASK_LAN2 |
										   RTL8367M_PORTMASK_LAN3 |
										   RTL8367M_PORTMASK_LAN4 |
										   RTL8367M_PORTMASK_CPU_WAN |
										   RTL8367M_PORTMASK_CPU_LAN );
#endif
		}
		return;
	}
	
	if (!first_call)
	{
		/* reset all vlan entries */
		phy_vlan_reset_table();
	}
	
	bridge_mode = atoi(nvram_safe_get("wan_stb_x"));
	if (bridge_mode < 0 || bridge_mode > 7)
		bridge_mode = RTL8367M_WAN_BRIDGE_DISABLE;
	
	bwan_isolation = atoi(nvram_safe_get("wan_stb_iso"));
	if (bwan_isolation < 0 || bwan_isolation > 2)
		bwan_isolation = RTL8367M_WAN_BWAN_ISOLATION_NONE;
	
	is_vlan_filter = (nvram_match("vlan_filter", "1")) ? 1 : 0;
	
	if (is_vlan_filter)
		bwan_isolation = RTL8367M_WAN_BWAN_ISOLATION_FROM_CPU;
	
	if (bridge_mode == RTL8367M_WAN_BRIDGE_DISABLE)
		bwan_isolation = RTL8367M_WAN_BWAN_ISOLATION_NONE;
	
	phy_bridge_mode(bridge_mode, bwan_isolation);
	
	if (is_vlan_filter)
	{
		vlan_fid = 0;
		accept_tagged = RTL8367M_PORTMASK_WAN;
		
		vlan_vid[0] = atoi(nvram_safe_get("vlan_vid_cpu"));
		vlan_vid[1] = atoi(nvram_safe_get("vlan_vid_lan1"));
		vlan_vid[2] = atoi(nvram_safe_get("vlan_vid_lan2"));
		vlan_vid[3] = atoi(nvram_safe_get("vlan_vid_lan3"));
		vlan_vid[4] = atoi(nvram_safe_get("vlan_vid_lan4"));
		
		vlan_pri[0] = atoi(nvram_safe_get("vlan_pri_cpu"));
		vlan_pri[1] = atoi(nvram_safe_get("vlan_pri_lan1"));
		vlan_pri[2] = atoi(nvram_safe_get("vlan_pri_lan2"));
		vlan_pri[3] = atoi(nvram_safe_get("vlan_pri_lan3"));
		vlan_pri[4] = atoi(nvram_safe_get("vlan_pri_lan4"));
		
		vlan_tag[1] = atoi(nvram_safe_get("vlan_tag_lan1"));
		vlan_tag[2] = atoi(nvram_safe_get("vlan_tag_lan2"));
		vlan_tag[3] = atoi(nvram_safe_get("vlan_tag_lan3"));
		vlan_tag[4] = atoi(nvram_safe_get("vlan_tag_lan4"));
		
		vlan_pri[0] &= 0x07;
		vlan_pri[1] &= 0x07;
		vlan_pri[2] &= 0x07;
		vlan_pri[3] &= 0x07;
		vlan_pri[4] &= 0x07;
		
		if((vlan_vid[0] >= 2 && vlan_vid[0] <= 4094))
		{
			vlan_fid++;
			vlan_member = RTL8367M_PORTMASK_CPU_WAN | RTL8367M_PORTMASK_WAN;
			vlan_untag  = RTL8367M_PORTMASK_CPU_WAN;
			phy_vlan_create_entry(vlan_vid[0], vlan_pri[0], vlan_member, vlan_untag, vlan_fid);
		}
		
		switch (bridge_mode)
		{
		case RTL8367M_WAN_BRIDGE_LAN1:
			if((vlan_vid[1] >= 2 && vlan_vid[1] <= 4094) && (vlan_vid[1] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN1 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				accept_tagged |= (vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				phy_vlan_create_entry(vlan_vid[1], vlan_pri[1], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN2:
			if((vlan_vid[2] >= 2 && vlan_vid[2] <= 4094) && (vlan_vid[2] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN2 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				accept_tagged |= (vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				phy_vlan_create_entry(vlan_vid[2], vlan_pri[2], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN3:
			if((vlan_vid[3] >= 2 && vlan_vid[3] <= 4094) && (vlan_vid[3] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN3 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				accept_tagged |= (vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				phy_vlan_create_entry(vlan_vid[3], vlan_pri[3], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN4:
			if((vlan_vid[4] >= 2 && vlan_vid[4] <= 4094) && (vlan_vid[4] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN4 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
				accept_tagged |= (vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
				phy_vlan_create_entry(vlan_vid[4], vlan_pri[4], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN3_LAN4:
			if((vlan_vid[3] >= 2 && vlan_vid[3] <= 4094) && (vlan_vid[3] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN3 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				accept_tagged |= (vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				if(vlan_vid[4] == vlan_vid[3])
				{
					vlan_member |= RTL8367M_PORTMASK_LAN4;
					vlan_untag  |= (!vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
					accept_tagged |= (vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
				}
				phy_vlan_create_entry(vlan_vid[3], vlan_pri[3], vlan_member, vlan_untag, vlan_fid);
			}
			if((vlan_vid[4] >= 2 && vlan_vid[4] <= 4094) && (vlan_vid[4] != vlan_vid[0]) && (vlan_vid[4] != vlan_vid[3]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN4 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
				accept_tagged |= (vlan_tag[4]) ? RTL8367M_PORTMASK_LAN4 : 0;
				phy_vlan_create_entry(vlan_vid[4], vlan_pri[4], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN1_LAN2:
			if((vlan_vid[1] >= 2 && vlan_vid[1] <= 4094) && (vlan_vid[1] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN1 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				accept_tagged |= (vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				if(vlan_vid[2] == vlan_vid[1])
				{
					vlan_member |= RTL8367M_PORTMASK_LAN2;
					vlan_untag  |= (!vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
					accept_tagged |= (vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				}
				phy_vlan_create_entry(vlan_vid[1], vlan_pri[1], vlan_member, vlan_untag, vlan_fid);
			}
			if((vlan_vid[2] >= 2 && vlan_vid[2] <= 4094) && (vlan_vid[2] != vlan_vid[0]) && (vlan_vid[2] != vlan_vid[1]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN2 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				accept_tagged |= (vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				phy_vlan_create_entry(vlan_vid[2], vlan_pri[2], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		case RTL8367M_WAN_BRIDGE_LAN1_LAN2_LAN3:
			if((vlan_vid[1] >= 2 && vlan_vid[1] <= 4094) && (vlan_vid[1] != vlan_vid[0]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN1 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				accept_tagged |= (vlan_tag[1]) ? RTL8367M_PORTMASK_LAN1 : 0;
				if(vlan_vid[2] == vlan_vid[1])
				{
					vlan_member |= RTL8367M_PORTMASK_LAN2;
					vlan_untag  |= (!vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
					accept_tagged |= (vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				}
				if(vlan_vid[3] == vlan_vid[1])
				{
					vlan_member |= RTL8367M_PORTMASK_LAN3;
					vlan_untag  |= (!vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
					accept_tagged |= (vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				}
				phy_vlan_create_entry(vlan_vid[1], vlan_pri[1], vlan_member, vlan_untag, vlan_fid);
			}
			if((vlan_vid[2] >= 2 && vlan_vid[2] <= 4094) && (vlan_vid[2] != vlan_vid[0]) && (vlan_vid[2] != vlan_vid[1]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN2 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				accept_tagged |= (vlan_tag[2]) ? RTL8367M_PORTMASK_LAN2 : 0;
				if((vlan_vid[3] == vlan_vid[2]) && (vlan_vid[3] != vlan_vid[1]))
				{
					vlan_member |= RTL8367M_PORTMASK_LAN3;
					vlan_untag  |= (!vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
					accept_tagged |= (vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				}
				phy_vlan_create_entry(vlan_vid[2], vlan_pri[2], vlan_member, vlan_untag, vlan_fid);
			}
			if((vlan_vid[3] >= 2 && vlan_vid[3] <= 4094) && (vlan_vid[3] != vlan_vid[0]) && (vlan_vid[3] != vlan_vid[1]) && (vlan_vid[3] != vlan_vid[2]))
			{
				vlan_fid++;
				vlan_member = RTL8367M_PORTMASK_LAN3 | RTL8367M_PORTMASK_WAN;
				vlan_untag  = (!vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				accept_tagged |= (vlan_tag[3]) ? RTL8367M_PORTMASK_LAN3 : 0;
				phy_vlan_create_entry(vlan_vid[3], vlan_pri[3], vlan_member, vlan_untag, vlan_fid);
			}
			break;
		}
		
		/* enable ingress filtering */
		phy_vlan_ingress_mode(RTL8367M_VLAN_INGRESS_FILTER_ENABLED);
		
		/* accept tagged and untagged frames for WAN port and needed LAN ports */
		phy_vlan_accept_port_mode(RTL8367M_VLAN_ACCEPT_FRAMES_ALL, accept_tagged);
	}
}


void
start_lan(void)
{
	char *lan_ipaddr;
	char *lan_netmsk;
	char *lan_ifname = nvram_safe_get("lan_ifname");
	if (!lan_ifname[0])
	{
		lan_ifname = IFNAME_BR;
	}
	
	if (nvram_match("lan_ipaddr", ""))
	{
		nvram_set("lan_ipaddr", "192.168.1.1");
		nvram_set("lan_netmask", "255.255.255.0");
	}
	else if (nvram_match("lan_netmask", ""))
	{
		nvram_set("lan_netmask", "255.255.255.0");
	}
	
	if (nvram_invmatch("computer_name", "") && is_valid_hostname(nvram_safe_get("computer_name")))
		doSystem("hostname %s", nvram_safe_get("computer_name"));
	else
		doSystem("hostname %s", nvram_safe_get("productid"));
	
	lan_ipaddr = nvram_safe_get("lan_ipaddr");
	lan_netmsk = nvram_safe_get("lan_netmask");
	
	/* 
	* Configure DHCP connection. The DHCP client will run 
	* 'udhcpc bound'/'udhcpc deconfig' upon finishing IP address 
	* renew and release.
	*/
	if (nvram_match("router_disable", "1"))
	{
		if (nvram_match("lan_proto_ex", "1"))
		{
			/* bring up and configure LAN interface */
			ifconfig(lan_ifname, IFUP, lan_ipaddr, lan_netmsk);
			
			symlink("/sbin/rc", "/tmp/landhcpc");
			
			/* early fill XXX_t fields */
			update_lan_status(0);
			
			/* start dhcp daemon */
			start_udhcpc_lan(lan_ifname);
		}
		else
		{
			/* bring up and configure LAN interface */
			ifconfig(lan_ifname, IFUP, lan_ipaddr, lan_netmsk);
			
			/* manual config lan gateway and dns */
			lan_up(lan_ifname);
		}
	}
	else
	{
		/* bring up and configure LAN interface */
		ifconfig(lan_ifname, IFUP, lan_ipaddr, lan_netmsk);
		
		/* install lan specific static routes */
		add_lan_routes(lan_ifname);
		
		/* fill XXX_t fields */
		update_lan_status(0);
	}
}

void
stop_lan(void)
{
	// Stop logger if remote
	if (nvram_invmatch("log_ipaddr", ""))
	{
		stop_logger();
	}
	
	/* Remove static routes */
	del_lan_routes(IFNAME_BR);
	
	/* Bring down LAN interface */
	ifconfig(IFNAME_BR, 0, NULL, NULL);
}

static int
wan_prefix(char *ifname, char *prefix)
{
	int unit;
	
	if ((unit = wan_ifunit(ifname)) < 0)
		return -1;

	sprintf(prefix, "wan%d_", unit);
	return 0;
}

static int
add_wan_routes(char *wan_ifname)
{
	char prefix[] = "wanXXXXXXXXXX_";

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;

	return add_routes(prefix, "route", wan_ifname);
}

static int
del_wan_routes(char *wan_ifname)
{
	char prefix[] = "wanXXXXXXXXXX_";

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;

	return del_routes(prefix, "route", wan_ifname);
}

void 
flush_conntrack_caches(void)
{
	FILE *fp;
	
	if ((fp=fopen("/proc/sys/net/nf_conntrack_table_flush", "w"))) {
		fputs("1", fp);
		fclose(fp);
	}
}

void 
flush_route_caches(void)
{
	FILE *fp;
	
	if ((fp=fopen("/proc/sys/net/ipv4/route/flush", "w"))) {
		fputs("-1", fp);
		fclose(fp);
	}
}

int is_module_loaded(char *module_name)
{
	DIR *dir_to_open = NULL;
	char sys_path[128];
	
	sprintf(sys_path, "/sys/module/%s", module_name);
	dir_to_open = opendir(sys_path);
	if (dir_to_open)
	{
		closedir(dir_to_open);
		return 1;
	}
	
	return 0;
}

int is_ftp_conntrack_loaded(int ftp_port)
{
	DIR *dir_to_open = NULL;
	FILE *fp;
	char ports_use[32];
	char ports_val[32];
	
	dir_to_open = opendir("/sys/module/nf_conntrack_ftp");
	if (dir_to_open)
	{
		closedir(dir_to_open);
		if (ftp_port)
		{
			if (ftp_port == 21)
				strcpy(ports_val, "21");
			else
				sprintf(ports_val, "21,%d", ftp_port);
			
			fp = fopen("/sys/module/nf_conntrack_ftp/parameters/ports", "r");
			if (fp) {
				ports_use[0] = 0;
				fgets(ports_use, sizeof(ports_use), fp);
				fclose(fp);
				if (strlen(ports_use) > 0)
					ports_use[strlen(ports_use) - 1] = 0; /* get rid of '\n' */
				
				if (strcmp(ports_val, ports_use) == 0)
					return 2;
			}
		}
		
		return 1;
	}
	
	return 0;
}

int is_hwnat_loaded(void)
{
	DIR *dir_to_open = NULL;
	FILE *fp;
	char offload_val[32];
	
	dir_to_open = opendir("/sys/module/hw_nat");
	if (dir_to_open)
	{
		closedir(dir_to_open);
		fp = fopen("/sys/module/hw_nat/parameters/wifi_offload", "r");
		if (fp) {
			offload_val[0] = 0;
			fgets(offload_val, sizeof(offload_val), fp);
			fclose(fp);
			if (strlen(offload_val) > 0)
				offload_val[strlen(offload_val) - 1] = 0; /* get rid of '\n' */
			
			if ( offload_val[0] == 'Y' || offload_val[0] == '1')
				return 2;
		}
		
		return 1;
	}
	
	return 0;
}


int is_hwnat_allow(void)
{
	if (nvram_invmatch("sw_mode_ex", "1"))
	{
		return 0;
	}
	
	if (nvram_match("hw_nat_mode", "2"))
	{
		return 0;
	}
	
	if (nvram_match("hw_nat_mode", "1"))
	{
		return 2; // wifi_offload=1
	}
	
	return 1; // wifi_offload=0
}

int is_fastnat_allow(void)
{
	if ( nvram_match("sw_nat_mode", "1") && nvram_match("sw_mode_ex", "1") )
	{
		return 1;
	}
	
	return 0;
}

void hwnat_load(void)
{
	doSystem("modprobe -q hw_nat wifi_offload=%d", nvram_match("hw_nat_mode", "1") ? 1 : 0);
}


void hwnat_unload(void)
{
	system("rmmod hw_nat");
}


void hwnat_logmessage(void)
{
	char *hwnat_status = "Disabled";
	char *swnat_status = "Disabled";
	
	if (nvram_invmatch("sw_mode_ex", "1"))
	{
		return;
	}
	
	int i_loaded = is_hwnat_loaded();
	if (i_loaded == 2)
		hwnat_status = "Enabled, IPv4/PPPoE offload [WAN]<->[LAN/Wi-Fi]";
	else if (i_loaded == 1)
		hwnat_status = "Enabled, IPv4/PPPoE offload [WAN]<->[LAN]";
	
	if (is_fastnat_allow())
		swnat_status = "Enabled";
	
	logmessage(LOGNAME, "Hardware NAT: %s", hwnat_status);
	logmessage(LOGNAME, "Software FastNAT: %s", swnat_status);
}

int 
is_wan_ppp(char *wan_proto)
{
	if (strcmp(wan_proto, "pppoe") == 0 || strcmp(wan_proto, "pptp") == 0 || strcmp(wan_proto, "l2tp") == 0)
	{
		return 1;
	}
	
	return 0;
}

void get_wan_ifname(char wan_ifname[16])
{
	char *ifname = IFNAME_WAN;
	char *wan_proto = nvram_safe_get("wan_proto");
	
	if(get_usb_modem_state()){
		if(nvram_match("modem_enable", "4"))
			ifname = nvram_safe_get("rndis_ifname");
		else
			ifname = "ppp0";
	}
	else
	if (is_wan_ppp(wan_proto))
	{
		ifname = "ppp0";
	}
	
	strcpy(wan_ifname, ifname);
}


void 
launch_wanx(char *wan_ifname, char *prefix, int unit, int wait_dhcpc, int use_zcip)
{
	char tmp[100];
	
	char *ip_addr = nvram_safe_get(strcat_r(prefix, "pppoe_ipaddr", tmp));
	char *netmask = nvram_safe_get(strcat_r(prefix, "pppoe_netmask", tmp));
	char *gateway = nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp));
	char *pppname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
	
	/* Bring up physical WAN interface */
	ifconfig(wan_ifname, IFUP, ip_addr, netmask);
	
	if (inet_addr_(ip_addr) == INADDR_ANY)
	{
		/* PPPoE connection not needed WAN physical address first, skip wait DHCP lease */
		/* PPTP and L2TP needed WAN physical first for create VPN tunnel, wait DHCP lease */
		/* Start dhcpc daemon */
		if (!use_zcip)
			start_udhcpc_wan(wan_ifname, unit, wait_dhcpc);
		else
			start_zcip_wan(wan_ifname);
	}
	else
	{
		/* do not use safe_get here, values are optional */
		/* start firewall */
		start_firewall_ex(pppname, "0.0.0.0", IFNAME_BR, nvram_safe_get("lan_ipaddr"));
		
		/* setup static wan routes via physical device */
		add_routes("wan_", "route", wan_ifname);
		
		/* and set default route if specified with metric 1 */
		if ( inet_addr_(gateway) != INADDR_ANY )
		{
			route_add(wan_ifname, 2, "0.0.0.0", gateway, "0.0.0.0");
		}
		
		/* start multicast router */
		start_igmpproxy(wan_ifname);
	}
}

void
start_wan(void)
{
	char *wan_ifname;
	char *wan_proto;
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char eabuf[32];
	int is_pppoe;
	int s;
	struct ifreq ifr;

	/* check if we need to setup WAN */
	if (nvram_match("router_disable", "1"))
	{
		return;
	}
	
	wan_mac_config();
	
	reload_nat_modules();
	reload_nat_modules_vpn();
	
	update_wan_status(0);
	
	/* Create links */
	mkdir("/tmp/ppp", 0777);
	mkdir("/tmp/ppp/peers", 0777);
	
	symlink("/sbin/rc", "/tmp/ppp/ip-up");
	symlink("/sbin/rc", "/tmp/ppp/ip-down");
	symlink("/sbin/rc", "/tmp/udhcpc.script");
	symlink("/sbin/rc", "/tmp/zcip.script");
	symlink("/sbin/rc", "/tmp/wpacli.script");
	
	// Padavan fix for PPTP/L2TP DHCP
	create_resolvconf();
	
	smart_restart_upnp();
	
	/* Start each configured and enabled wan connection and its undelying i/f */
	for (unit = 0; unit < MAX_NVPARSE; unit ++) 
	{
#ifdef ASUS_EXT // Only multiple pppoe is allowed 
		if (unit > 0 && !nvram_match("wan_proto", "pppoe")) break;
#endif
		if (unit > 2) break;

		snprintf(prefix, sizeof(prefix), "wan%d_", unit);

		/* make sure the connection exists and is enabled */ 
		wan_ifname = nvram_get(strcat_r(prefix, "ifname", tmp));
		if (!wan_ifname) {
			continue;
		}
		wan_proto = nvram_get(strcat_r(prefix, "proto", tmp));
		if (!wan_proto || !strcmp(wan_proto, "disabled"))
			continue;
		
		is_pppoe = !strcmp(wan_proto, "pppoe");

		dbg("%s: wan_ifname=%s, wan_proto=%s\n", __FUNCTION__, wan_ifname, wan_proto);

		/* Set i/f hardware address before bringing it up */
		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
			continue;

		strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);

		/* Since WAN interface may be already turned up (by vlan.c),
		   if WAN hardware address is specified (and different than the current one),
		   we need to make it down for synchronizing hwaddr. */
		if (ioctl(s, SIOCGIFHWADDR, &ifr)) {
			close(s);
			continue;
		}

		if (nvram_match("wan_proto", "dhcp") && nvram_match("mac_clone_en", "1"))
			nvram_set(strcat_r(prefix, "hwaddr", tmp), nvram_safe_get("cl0macaddr"));
		
		ether_atoe(nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)), eabuf);
		
		if (/*!nvram_match("wan_hwaddr", "") && */(bcmp(eabuf, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN)))
		{
			/* current hardware address is different than user specified */
			ifconfig(wan_ifname, 0, NULL, NULL);
		}

		/* Configure i/f only once, specially for wireless i/f shared by multiple connections */
		if (ioctl(s, SIOCGIFFLAGS, &ifr)) {
			close(s);
			continue;
		}

		if (!(ifr.ifr_flags & IFF_UP)) {
//			dbg("** wan_ifname: %s is NOT UP\n", wan_ifname);

			/* Sync connection nvram address and i/f hardware address */
			memset(ifr.ifr_hwaddr.sa_data, 0, ETHER_ADDR_LEN);

			if (nvram_match("wan_proto", "dhcp") && nvram_match("mac_clone_en", "1"))
			{
				ether_atoe(nvram_safe_get("cl0macaddr"), ifr.ifr_hwaddr.sa_data);
				ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
				ioctl(s, SIOCSIFHWADDR, &ifr);
			}
			else if (nvram_match(strcat_r(prefix, "hwaddr", tmp), "") ||
			    !ether_atoe(nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)), ifr.ifr_hwaddr.sa_data) ||
			    !memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN)) {
/*
				dbg("** chk hwaddr:[%s]<%s>\n", strcat_r(prefix, "hwaddr", tmp),
					nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)));
				dbg("** atoe result:%d\n", ether_atoe(nvram_safe_get(strcat_r(prefix, "hwaddr", tmp)),
					ifr.ifr_hwaddr.sa_data));
				dbg("** ifr_sa_data not empty:[%d]\n", 
					memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN));
*/
				if (ioctl(s, SIOCGIFHWADDR, &ifr)) {
					dbg("ioctl fail. continue\n");
					close(s);
					continue;
				}
				nvram_set(strcat_r(prefix, "hwaddr", tmp), ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
			}
			else {
				ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
				ioctl(s, SIOCSIFHWADDR, &ifr);
			}

			/* Bring up i/f */
			ifconfig(wan_ifname, IFUP, NULL, NULL);
		}

		close(s);

		if (unit == 0) 
		{		
			FILE *fp;

			start_pppoe_relay(IFNAME_WAN);

			/* Enable Forwarding */
			if ((fp = fopen("/proc/sys/net/ipv4/ip_forward", "r+"))) {
				fputc('1', fp);
				fclose(fp);
			} else
			{	
				perror("/proc/sys/net/ipv4/ip_forward");
			}
		}

		/* 
		* Configure PPPoE connection. The PPPoE client will run 
		* ip-up/ip-down scripts upon link's connect/disconnect.
		*/
		
		if(get_usb_modem_state())
		{
			if(nvram_match("modem_enable", "4"))
			{
				char *rndis_ifname = nvram_safe_get("rndis_ifname");
				if (strlen(rndis_ifname) > 0) {
					ifconfig(rndis_ifname, IFUP, "0.0.0.0", NULL);
					start_udhcpc_wan(rndis_ifname, unit, 0);
					nvram_set("wan_ifname_t", rndis_ifname);
				}
			}
			else
			{
				if (is_wan_ppp(wan_proto))
				{
					if (!is_pppoe || nvram_match("pppoe_dhcp_route", "1"))
						launch_wanx(wan_ifname, prefix, unit, 0, 0);
					else if (is_pppoe && nvram_match("pppoe_dhcp_route", "2"))
						launch_wanx(wan_ifname, prefix, unit, 0, 1);
				}
				else
				{
					/* do not use safe_get here, values are optional */
					/* start firewall */
					start_firewall_ex("ppp0", "0.0.0.0", IFNAME_BR, nvram_safe_get("lan_ipaddr"));
					
					/* setup static wan routes via physical device */
					add_routes("wan_", "route", wan_ifname);
					
					/* start multicast router */
					start_igmpproxy(wan_ifname);
				}
				
				if (create_pppd_script_modem_3g())
				{
					/* launch pppoe client daemon */
					logmessage("start_wan()", "select 3G modem node %s to pppd", nvram_safe_get("modem_node_t"));
					
					eval("pppd", "call", "3g");
				}
				else
				{
					logmessage("start_wan()", "unable to open 3G modem script!");
				}
				
				nvram_set("wan_ifname_t", "ppp0");
			}
		}
		else
		if (is_wan_ppp(wan_proto))
		{
			if (!is_pppoe || nvram_match("pppoe_dhcp_route", "1"))
				launch_wanx(wan_ifname, prefix, unit, !is_pppoe, 0);
			else if (is_pppoe && nvram_match("pppoe_dhcp_route", "2"))
				launch_wanx(wan_ifname, prefix, unit, 0, 1);
			
			/* L2TP does not support idling */ // oleg patch
			int demand = atoi(nvram_safe_get(strcat_r(prefix, "pppoe_idletime", tmp))) && strcmp(wan_proto, "l2tp");
			
			/* update demand option */
			nvram_set(strcat_r(prefix, "pppoe_demand", tmp), demand ? "1" : "0");
			
			/* launch pppoe client daemon */
			start_pppd(prefix);
			
			/* ppp interface name is referenced from this point on */
			wan_ifname = nvram_safe_get(strcat_r(prefix, "pppoe_ifname", tmp));
			
			/* Pretend that the WAN interface is up */
			if (nvram_match(strcat_r(prefix, "pppoe_demand", tmp), "1")) 
			{
				int timeout = 5;
				/* Wait for pppx to be created */
				while (ifconfig(wan_ifname, IFUP, NULL, NULL) && timeout--)
					sleep(1);
				
				/* Retrieve IP info */
				if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
					continue;
				
				strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
				
				/* Set temporary IP address */
				if (ioctl(s, SIOCGIFADDR, &ifr))
					perror(wan_ifname);
				
				nvram_set(strcat_r(prefix, "ipaddr", tmp), inet_ntoa(sin_addr(&ifr.ifr_addr)));
				nvram_set(strcat_r(prefix, "netmask", tmp), "255.255.255.255");
				
				/* Set temporary P-t-P address */
				if (ioctl(s, SIOCGIFDSTADDR, &ifr))
					perror(wan_ifname);
				
				nvram_set(strcat_r(prefix, "gateway", tmp), inet_ntoa(sin_addr(&ifr.ifr_dstaddr)));
				
				close(s);
				/* 
				* Preset routes so that traffic can be sent to proper pppx even before 
				* the link is brought up.
				*/
				preset_wan_routes(wan_ifname);
			}
			nvram_set("wan_ifname_t", wan_ifname);
		}
		
		/* 
		* Configure DHCP connection. The DHCP client will run 
		* 'udhcpc bound'/'udhcpc deconfig' upon finishing IP address 
		* renew and release.
		*/
		else if (strcmp(wan_proto, "dhcp") == 0)
		{
			/* Start eapol-md5 authenticator */
			if (nvram_match("wan_auth_mode", "2"))
				start_auth_eapol(wan_ifname);
			
			/* Start dhcp daemon */
			start_udhcpc_wan(wan_ifname, unit, 0);
			nvram_set("wan_ifname_t", wan_ifname);
		}
		/* Configure static IP connection. */
		else if ((strcmp(wan_proto, "static") == 0)) 
		{
			/* Assign static IP address to i/f */
			ifconfig(wan_ifname, IFUP,
				 nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)), 
				 nvram_safe_get(strcat_r(prefix, "netmask", tmp)));
			
			/* Start eapol-md5 authenticator */
			if (nvram_match("wan_auth_mode", "2"))
				start_auth_eapol(wan_ifname);
			
			/* We are done configuration */
			wan_up(wan_ifname);
			nvram_set("wan_ifname_t", wan_ifname);
		}
	}

	/* Report stats */
	if (nvram_invmatch("stats_server", "")) {
		char *stats_argv[] = { "stats", nvram_safe_get("stats_server"), NULL };
		_eval(stats_argv, NULL, 5, NULL);
	}
}

void 
select_usb_modem_to_wan(int wait_modem_sec)
{
	int i;
	int is_modem_found = 0;
	int modem_type = atoi(nvram_safe_get("modem_enable"));
	
	// Check modem enabled
	if (modem_type > 0)
	{
		for (i=0; i<=wait_modem_sec; i++)
		{
			if (modem_type == 4)
			{
				if ( is_ready_modem_4g() )
				{
					is_modem_found = 1;
					break;
				}
			}
			else
			{
				if ( is_ready_modem_3g() )
				{
					is_modem_found = 1;
					break;
				}
			}
			
			if (i<wait_modem_sec)
				sleep(1);
		}
	}
	
	set_usb_modem_state(is_modem_found);
}

void safe_remove_usb_modem(void)
{
	char* svcs[] = { "pppd", NULL };
	
	if (!is_usb_modem_ready())
		return;
	
	if(nvram_match("modem_enable", "4")) 
	{
		if (get_usb_modem_state())
		{
			if (pids("udhcpc"))
			{
				system("killall -SIGUSR2 udhcpc");
				usleep(50000);
				
				system("killall udhcpc");
			}
		}
		
		stop_modem_4g();
	}
	else
	{
		if (get_usb_modem_state())
		{
			kill_services(svcs, 5, 1);
		}
		
		stop_modem_3g();
	}
	
	set_usb_modem_state(0);
}


int 
is_dns_static(void)
{
	if (get_usb_modem_state())
	{
		return 0; // force dynamic dns for ppp0/eth0
	}
	
	if (nvram_match("wan0_proto", "static"))
	{
		return 1; // always static dns for eth3/eth2.2
	}
	
	return !nvram_match("wan_dnsenable_x", "1"); // dynamic or static dns for ppp0 or eth3/eth2.2
}

int 
is_physical_wan_dhcp(void)
{
	if (nvram_match("wan_proto", "static"))
	{
		return 0;
	}
	
	if (nvram_match("wan_proto", "dhcp") || nvram_match("x_DHCPClient", "1"))
	{
		return 1;
	}
	
	return 0;
}

void
stop_wan(void)
{
	char *rndis_ifname;
	char *wan_ifname = IFNAME_WAN;
	char *svcs[] = { "stats", 
	                 "ntpclient", 
	                 "igmpproxy", 
	                 "udpxy", 
	                 "ip-up",
	                 "ip-down",
	                 "zcip",
	                 "udhcpc",
	                 "l2tpd", 
	                 "pppd", 
	                 "pptp", 
	                 "pppoe-relay", 
	                 "detect_wan",
	                  NULL };
	
	if (pids("udhcpc"))
	{
		logmessage("stop_wan()", "raise DHCP release event");
		system("killall -SIGUSR2 udhcpc");
		usleep(50000);
	}
	
	stop_auth_eapol();
	stop_auth_kabinet();
	
	kill_services(svcs, 5, 1);
	
	if (!is_physical_wan_dhcp() && nvram_match("wan_ifname_t", wan_ifname))
		wan_down(wan_ifname);
	
	/* Bring down WAN interfaces */
	ifconfig(wan_ifname, 0, "0.0.0.0", NULL);
	
	/* Bring down usbnet interface */
	rndis_ifname = nvram_safe_get("rndis_ifname");
	if (strlen(rndis_ifname) > 0) {
		ifconfig(rndis_ifname, 0, "0.0.0.0", NULL);
	}
	
	/* Remove dynamically created links */
	unlink("/tmp/zcip.script");
	unlink("/tmp/udhcpc.script");
	unlink("/tmp/wpacli.script");
	
	unlink("/tmp/ppp/ip-up");
	unlink("/tmp/ppp/ip-down");
	unlink("/tmp/ppp/link.ppp0");
	unlink("/tmp/ppp/options.wan0");
	
	flush_conntrack_caches();
	
	update_wan_status(0);
}

void 
stop_wan_ppp()		/* pptp, l2tp, ppope */
{
	// stop services only for ppp0 interface
	char* svcs[] = { "l2tpd", 
	                 "pppd", 
	                 "pptp", 
	                  NULL };
	
	kill_services(svcs, 5, 1);
	
	nvram_set("wan_status_t", "Disconnected");
}

void
stop_wan_static(void)
{
	char *wan_ifname = IFNAME_WAN;
	char *svcs[] = { "stats", 
	                 "ntpclient", 
	                 "ip-up",
	                 "ip-down",
	                 "zcip",
	                 "udhcpc",
	                 "l2tpd", 
	                 "pppd", 
	                 "pptp", 
	                 "pppoe-relay", 
	                 "igmpproxy", // oleg patch
	                 "udpxy", 
	                  NULL };
	
	if (pids("udhcpc"))
	{
		logmessage("stop_wan_static()", "raise DHCP release event");
		system("killall -SIGUSR2 udhcpc");
		usleep(50000);
	}
	
	stop_auth_eapol();
	stop_auth_kabinet();
	
	kill_services(svcs, 5, 1);
	
	if (nvram_match("wan_ifname_t", wan_ifname))
		wan_down(wan_ifname);
	
	/* Remove dynamically created links */
	unlink("/tmp/zcip.script");
	unlink("/tmp/udhcpc.script");
	unlink("/tmp/wpacli.script");
	
	unlink("/tmp/ppp/ip-up");
	unlink("/tmp/ppp/ip-down");
	
	flush_conntrack_caches();
}

void 
full_restart_wan(int use_wan_reconfig)
{
	char *lan_ifname = IFNAME_BR;
	
	stop_wan();
	
	if (use_wan_reconfig)
	{
		del_lan_routes(lan_ifname);
		
		reset_wan_vars(0);
		
		flush_route_caches();
		
		add_lan_routes(lan_ifname);
		
		switch_config_vlan(0);
		
		select_usb_modem_to_wan(0);
	}
	
	start_wan();
}

int 
create_resolvconf()
{
	FILE *fp;
	char *google_dns = "8.8.8.8";
	int dns_static = is_dns_static();
	int total_dns = 0;
	
	spinlock_lock(SPINLOCK_DNSRenew);
	
	fp = fopen("/etc/resolv.conf", "w+");
	if (fp) 
	{
		/* check if not auto dns enabled */
		if ( dns_static )
		{
			/* Write resolv.conf from static nameservers */
			if (nvram_invmatch("wan_dns1_x", "") && nvram_invmatch("wan_dns1_x", "0.0.0.0")) {
				fprintf(fp, "nameserver %s\n", nvram_safe_get("wan_dns1_x"));
				total_dns++;
			}
			
			if (nvram_invmatch("wan_dns2_x", "") && nvram_invmatch("wan_dns2_x", "0.0.0.0")) {
				fprintf(fp, "nameserver %s\n", nvram_safe_get("wan_dns2_x"));
				total_dns++;
			}
			
			if (total_dns == 0)
				fprintf(fp, "nameserver %s\n", google_dns);
		}
		fclose(fp);
	}
	
	/* create md5 hash for resolv.conf */
	system("md5sum /etc/resolv.conf > /tmp/hashes/resolv_md5");
	
	spinlock_unlock(SPINLOCK_DNSRenew);
	
	/* notify dns relay server */
	if ( dns_static )
	{
		restart_dns();
	}
	
	return 0;
}

int
update_resolvconf()
{
	FILE *fp;
	char word[256], *next, *wan_dns;
	char *google_dns = "8.8.8.8";
	int allow_google = 0;
	int total_dns = 0;
	int resolv_changed = 0;
	
	/* check if auto dns enabled */
	if ( is_dns_static() )
	{
		return 0;
	}
	
	spinlock_lock(SPINLOCK_DNSRenew);
	
	fp = fopen("/etc/resolv.conf", "w+");
	if (fp) 
	{
		if (strlen(nvram_safe_get("wan0_dns")))
		{
			wan_dns = nvram_safe_get("wan0_dns");
			allow_google = 1;
		}
		else
		{
			wan_dns = nvram_safe_get("wanx_dns");
		}
		
		foreach(word, wan_dns, next)
		{
			if (strcmp(word, "0.0.0.0"))
			{
				fprintf(fp, "nameserver %s\n", word);
				total_dns++;
				if (strcmp(word, google_dns) == 0)
					allow_google = 0;
			}
		}
		
		if ((total_dns < 1) || (allow_google && total_dns < 2))
		{
			fprintf(fp, "nameserver %s\n", google_dns);
		}
		
		fclose(fp);
	}
	
	/* check and update hashes for resolv.conf */
	if (system("md5sum -cs /tmp/hashes/resolv_md5") != 0)
	{
		resolv_changed = 1;
		system("md5sum /etc/resolv.conf > /tmp/hashes/resolv_md5");
	}
	
	spinlock_unlock(SPINLOCK_DNSRenew);
	
	/* notify dns relay server */
	if (resolv_changed)
	{
		restart_dns();
	}
	
	return 0;
}

int
is_ifunit_modem(char *wan_ifname)
{
	if (get_usb_modem_state())
	{
		if (ppp_ifunit(wan_ifname) >= 0)
		{
			return 1;
		}
		
		if (strcmp(wan_ifname, nvram_safe_get("rndis_ifname")) == 0)
		{
			return 2;
		}
	}
	
	return 0;
}

void
wan_up(char *wan_ifname)	// oleg patch, replace
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_proto, *gateway;
	char *script_postw = "/etc/storage/post_wan_script.sh";
	int  is_modem_unit = is_ifunit_modem(wan_ifname);
	
	logmessage(LOGNAME, "wan up (%s)", wan_ifname);
	
	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
	{
		/* called for dhcp+ppp */
		if (!nvram_match("wan0_ifname", wan_ifname))
		{
			return;
		}
		
		/* re-start firewall with old ppp0 address or 0.0.0.0 */
		start_firewall_ex("ppp0", nvram_safe_get("wan0_ipaddr"), IFNAME_BR, nvram_safe_get("lan_ipaddr"));
		
		/* setup static wan routes via physical device */
		add_routes("wan_", "route", wan_ifname);
		
		/* and one supplied via DHCP */
		add_wanx_routes("wanx_", wan_ifname, 0);
		
		gateway = nvram_safe_get("wanx_gateway");
		
		/* and default route with metric 1 */
		if (inet_addr_(gateway) != INADDR_ANY)
		{
			char word[100], *next;
			in_addr_t addr = inet_addr(nvram_safe_get("wanx_ipaddr"));
			in_addr_t mask = inet_addr(nvram_safe_get("wanx_netmask"));
			
			/* if the gateway is out of the local network */
			if ((inet_addr(gateway) & mask) != (addr & mask))
				route_add(wan_ifname, 2, gateway, NULL, "255.255.255.255");
			
			/* default route via default gateway */
			route_add(wan_ifname, 2, "0.0.0.0", gateway, "0.0.0.0");
			
			/* ... and to dns servers as well for demand ppp to work */
			if (nvram_match("wan_dnsenable_x", "1") && nvram_invmatch("wan_proto", "pppoe"))
			{
				foreach(word, nvram_safe_get("wanx_dns"), next)
				{
					if ((inet_addr(word) != inet_addr(gateway)) && (inet_addr(word) & mask) != (addr & mask))
						route_add(wan_ifname, 2, word, gateway, "255.255.255.255");
				}
			}
		}
		
		/* start multicast router */
		start_igmpproxy(wan_ifname);
		
		update_resolvconf();
		
		return;
	}
	
	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
	
	dprintf("%s %s\n", wan_ifname, wan_proto);
	
	/* Set default route to gateway if specified */
	if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
	{
		gateway = nvram_safe_get(strcat_r(prefix, "gateway", tmp));
		
		if ( (!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_proto, "static") == 0) )
		{
			/* the gateway is in the local network */
			route_add(wan_ifname, 0, gateway, NULL, "255.255.255.255");
		}
		
		/* default route via default gateway */
		route_add(wan_ifname, 0, "0.0.0.0", gateway, "0.0.0.0");
		
		/* hack: avoid routing cycles, when both peer and server has the same IP */
		if ( (!is_modem_unit) && (strcmp(wan_proto, "pptp") == 0 || strcmp(wan_proto, "l2tp") == 0)) {
			/* delete gateway route as it's no longer needed */
			route_del(wan_ifname, 0, gateway, "0.0.0.0", "255.255.255.255");
		}
	}
	
	/* Install interface dependent static routes */
	add_wan_routes(wan_ifname);
	
	/* Add static wan routes */
	if ( (!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_proto, "static") == 0) )
	{
		nvram_set("wanx_gateway", nvram_safe_get(strcat_r(prefix, "gateway", tmp)));
		add_routes("wan_", "route", wan_ifname);
	}
	
	/* Add dynamic routes supplied via DHCP */
	if ( ((!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0)) || (is_modem_unit == 2) )
	{
		add_wanx_routes(prefix, wan_ifname, 0);
	}
	
	/* Add dns servers to resolv.conf */
	update_resolvconf();
	
	/* Start kabinet authenticator */
	if ( (!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_proto, "static") == 0) )
	{
		if (nvram_match("wan_auth_mode", "1"))
			start_auth_kabinet();
	}
	
	/* Sync time */
	update_wan_status(1);
	
	start_firewall_ex(wan_ifname, nvram_safe_get(strcat_r(prefix, "ipaddr", tmp)),
		IFNAME_BR, nvram_safe_get("lan_ipaddr"));
	
	update_upnp(1);
	
	/* start multicast router */
	if ( (!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_proto, "static") == 0) )
	{
		start_igmpproxy(wan_ifname);
	}
	
	hwnat_logmessage();
	
	start_ddns(0);
	
	refresh_ntpc();
	
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	if ( (!is_modem_unit) && (nvram_match("wan0_proto", "dhcp")) )
	{
		if (nvram_invmatch("detectWan", "0") && !pids("detect_wan"))
		{
			system("detect_wan &");
		}
	}
#endif
	if (check_if_file_exist(script_postw))
	{
		doSystem("%s %s %s", script_postw, "up", wan_ifname);
	}
}

void
wan_down(char *wan_ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan_proto;
	char *script_postw = "/etc/storage/post_wan_script.sh";
	int  is_modem_unit = is_ifunit_modem(wan_ifname);
	
	logmessage(LOGNAME, "wan down (%s)", wan_ifname);
	
	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
	{
		// dhcp + ppp (wan_ifname=eth3/eth2.2)
		/* stop multicast router */
		stop_igmpproxy();
		
		// flush conntrack caches
		flush_conntrack_caches();
		
		return;
	}
	
	wan_proto = nvram_safe_get(strcat_r(prefix, "proto", tmp));
	
	if ( (!is_modem_unit) && (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_proto, "static") == 0) )
	{
		/* Stop multicast router */
		stop_igmpproxy();
		
		/* Stop kabinet authenticator */
		if (nvram_match("wan_auth_mode", "1"))
			stop_auth_kabinet();
	}
	
	/* Remove default route to gateway if specified */
	if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
		route_del(wan_ifname, 0, "0.0.0.0", 
			nvram_safe_get(strcat_r(prefix, "gateway", tmp)),
			"0.0.0.0");
	
	/* Remove interface dependent static routes */
	del_wan_routes(wan_ifname);
	
	/* Update resolv.conf -- leave as is if no dns servers left for demand to work */
	if (*nvram_safe_get("wanx_dns"))	// oleg patch
		nvram_unset(strcat_r(prefix, "dns", tmp));
	
	update_resolvconf();
	
	if ( (!is_modem_unit) && (strcmp(wan_proto, "static")==0) )
	{
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
	}
	
	update_wan_status(0);
	
	// cleanup
	nvram_set("wan_ipaddr_t", "");
	
	// flush conntrack caches
	flush_conntrack_caches();
	
	if (check_if_file_exist(script_postw))
	{
		doSystem("%s %s %s", script_postw, "down", wan_ifname);
	}
}

void
lan_up(char *lan_ifname)
{
	FILE *fp;
	int dns_count = 0;
	char word[100], *next, *gateway_ip;

	gateway_ip = nvram_safe_get("lan_gateway");

	/* Set default route to gateway if specified */
	if (inet_addr_(gateway_ip) != INADDR_ANY)
		route_add(lan_ifname, 0, "0.0.0.0", gateway_ip, "0.0.0.0");

	/* Open resolv.conf */
	fp = fopen("/etc/resolv.conf", "w+");
	if (fp) {
		foreach(word, nvram_safe_get("lan_dns"), next) {
			if (inet_addr_(word) != INADDR_ANY) {
				fprintf(fp, "nameserver %s\n", word);
				dns_count++;
			}
		}
		
		if (!dns_count && inet_addr_(gateway_ip) != INADDR_ANY)
			fprintf(fp, "nameserver %s\n", gateway_ip);
		
		fclose(fp);
	}

	/* sync time */
	refresh_ntpc();

	/* fill XXX_t fields */
	update_lan_status(0);
}

void
lan_up_ex(char *lan_ifname)
{
	FILE *fp;
	int dns_count = 0;
	char word[100], *next, *gateway_ip;

	gateway_ip = nvram_safe_get("lan_gateway_t");

	/* Set default route to gateway if specified */
	if (inet_addr_(gateway_ip) != INADDR_ANY)
		route_add(lan_ifname, 0, "0.0.0.0", gateway_ip, "0.0.0.0");

	/* Open resolv.conf */
	fp = fopen("/etc/resolv.conf", "w+");
	if (fp) {
		foreach(word, nvram_safe_get("lan_dns_t"), next) {
			if (inet_addr_(word) != INADDR_ANY) {
				fprintf(fp, "nameserver %s\n", word);
				dns_count++;
			}
		}
		
		if (!dns_count && inet_addr_(gateway_ip) != INADDR_ANY)
			fprintf(fp, "nameserver %s\n", gateway_ip);
		
		fclose(fp);
	}

#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	if (!pids("detect_wan"))
	{
		system("detect_wan &");
	}
#endif
	/* sync time */
	refresh_ntpc();

	/* fill XXX_t fields */
	update_lan_status(1);
}

void
lan_down_ex(char *lan_ifname)
{
	FILE *fp;
	
	/* Remove default route to gateway if specified */
	route_del(lan_ifname, 0, "0.0.0.0", 
			nvram_safe_get("lan_gateway_t"),
			"0.0.0.0");

	/* Clear resolv.conf */
	fp = fopen("/etc/resolv.conf", "w+");
	if (fp)
		fclose(fp);
	
	/* fill XXX_t fields */
	update_lan_status(0);
}

int
wan_ifunit(char *wan_ifname)
{
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	if ((unit = ppp_ifunit(wan_ifname)) >= 0) {
		return unit;
	} else {
		if (strcmp(wan_ifname, nvram_safe_get("rndis_ifname")) == 0)
			return 0;
		
		for (unit = 0; unit < MAX_NVPARSE; unit ++) {
			snprintf(prefix, sizeof(prefix), "wan%d_", unit);
			if (nvram_match(strcat_r(prefix, "ifname", tmp), wan_ifname) &&
			    (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp") ||
			     nvram_match(strcat_r(prefix, "proto", tmp), "static")))
				return unit;
		}
	}
	return -1;
}

int
preset_wan_routes(char *wan_ifname)
{
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	printf("preset wan routes [%s]\n", wan_ifname);

	/* Figure out nvram variable name prefix for this i/f */
	if (wan_prefix(wan_ifname, prefix) < 0)
		return -1;

	/* Set default route to gateway if specified */
	if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
	{
		route_add(wan_ifname, 0, "0.0.0.0", "0.0.0.0", "0.0.0.0");
	}

	/* Install interface dependent static routes */
	add_wan_routes(wan_ifname);
	return 0;
}

int
wan_primary_ifunit(void)
{
	int unit;
	
	for (unit = 0; unit < MAX_NVPARSE; unit ++) {
		char tmp[100], prefix[] = "wanXXXXXXXXXX_";
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		//snprintf(prefix, sizeof(prefix), "wan_", unit);
		if (nvram_match(strcat_r(prefix, "primary", tmp), "1"))
			return unit;
	}

	return 0;
}

void dumparptable(void)
{
	char buf[256];
	char ip_entry[32], hw_type[8], flags[8], hw_address[32], mask[32], device[8];
	char macbuf[36];

	FILE *fp = fopen("/proc/net/arp", "r");
	if (!fp) {
		dbg("no proc fs mounted!\n");
		return;
	}

	mac_num = 0;

//	while (fgets(buf, 256, fp) && (mac_num < MAX_MAC_NUM - 1)) {
	while (fgets(buf, 256, fp) && (mac_num < MAX_MAC_NUM - 2)) {
		sscanf(buf, "%s %s %s %s %s %s", ip_entry, hw_type, flags, hw_address, mask, device);

		if (!strcmp(device, IFNAME_BR))
		{
			strcpy(mac_clone[mac_num++], hw_address);
//			dbg("%d %s\n", mac_num, mac_clone[mac_num - 1]);
		}
	}
	fclose(fp);

	mac_conv("wan_hwaddr_x", -1, macbuf);
	if (nvram_invmatch("wan_hwaddr_x", "") && strcasecmp(macbuf, "FF:FF:FF:FF:FF:FF"))
		strcpy(mac_clone[mac_num++], macbuf);
//	else
		strcpy(mac_clone[mac_num++], nvram_safe_get("il1macaddr"));

	if (mac_num)
	{
		dbg("num of mac: %d\n", mac_num);
		int i;
		for (i = 0; i < mac_num; i++)
			dbg("mac to clone: %s\n", mac_clone[i]);
	}
}

in_addr_t get_ipv4_addr(char* ifname)
{
	int s;
	struct ifreq ifr;
	struct sockaddr_in *ipv4_inaddr;
	in_addr_t ipv4_addr = INADDR_ANY;

	/* Retrieve IP info */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return INADDR_ANY;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	/* Get IPv4 address */
	if (ioctl(s, SIOCGIFADDR, &ifr) == 0) {
		ipv4_inaddr = (struct sockaddr_in *)&ifr.ifr_addr;
		if (ipv4_inaddr->sin_addr.s_addr != INADDR_ANY &&
		    ipv4_inaddr->sin_addr.s_addr != INADDR_NONE)
			ipv4_addr = ipv4_inaddr->sin_addr.s_addr;
	}

	close(s);

	return ipv4_addr;
}

in_addr_t get_lan_ipaddr(void)
{
	return get_ipv4_addr(IFNAME_BR);
}

in_addr_t get_wan_ipaddr(int only_broadband_wan)
{
	char *ifname = IFNAME_WAN;

	if (nvram_match("wan_route_x", "IP_Bridged"))
		return INADDR_ANY;

	if(!only_broadband_wan && get_usb_modem_state()){
		if(nvram_match("modem_enable", "4"))
			ifname = nvram_safe_get("rndis_ifname");
		else
			ifname = "ppp0";
	} 
	else if (nvram_match("wan0_proto", "dhcp") || nvram_match("wan0_proto", "static"))
		ifname = IFNAME_WAN;
	else
		ifname = "ppp0";
	
	return get_ipv4_addr(ifname);
}

int is_interface_exist(const char *ifname)
{
	int sockfd;
	struct ifreq ifreq;
	int if_exist = 1;
	
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		return 0;
	}
	
	strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifreq) < 0) {
		if_exist = 0;
	}
	
	close(sockfd);
	
	return if_exist;
}

int is_interface_up(const char *ifname)
{
	int sockfd;
	struct ifreq ifreq;
	int iflags = 0;
	
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		return 0;
	}
	
	strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifreq) < 0) {
		iflags = 0;
	} else {
		iflags = ifreq.ifr_flags;
	}
	
	close(sockfd);
	
	if (iflags & IFF_UP)
		return 1;
	
	return 0;
}

int
has_wan_ip(int only_broadband_wan)
{
	if (get_wan_ipaddr(only_broadband_wan) != INADDR_ANY)
		return 1;
	
	return 0;
}

int
got_wan_ip()
{
	char *wan_ip = nvram_safe_get("wan_ipaddr_t");
	if (strcmp("", wan_ip) && strcmp("0.0.0.0", wan_ip))
		return 1;
	else
		return 0;
}

// return value: first bit is WAN port, second bit is USB Modem.
int is_phyconnected(void)
{
	int ret = 0;

	if(is_usb_modem_ready())
		ret += 1<<1;

	if(nvram_match("link_wan", "1"))
		ret += 1;

	return ret;
}

void
start_mac_clone(void)
{
	int had_try = 0;

	if (!nvram_match("wan_proto", "dhcp"))
		return;

	//sleep(15);
	nvram_set("done_auto_mac", "0");

	while (!got_wan_ip() && !had_try)
	{
		int link_wan = is_phyconnected()&0x1;
		if(!link_wan)
		{
			sleep(5);
			continue;
		}

		dumparptable();

		if (mac_num > 1)
		{
			nvram_set("mac_clone_en", "1");
			int i;
			for (i = 0; i < mac_num; i++)
			{
				nvram_set("cl0macaddr", mac_clone[i]);

				stop_wan();
				start_wan();
				sleep(15);

				if (got_wan_ip())
				{
					char buf[13];
					memset(buf, 0, 13);
					mac_conv2("cl0macaddr", -1, buf);
					nvram_set("wan_hwaddr_x", buf);
					dbg("stop mac cloning!\n");
					break;
				}

				if(i == mac_num-1)
					had_try = 1;
			}
			nvram_set("mac_clone_en", "0");
		}

		//sleep(5);
	}

	nvram_set("done_auto_mac", "1");
	nvram_commit_safe();
}
// 2010.09 James modified. }

int
ppp0_as_default_route(void)
{
	int i, n, found;
	FILE *fp;
	unsigned int dest, mask;
	char buf[256], device[256];

	n = 0;
	found = 0;
	mask = 0;
	device[0] = '\0';

	fp = fopen("/proc/net/route", "r");
	if (fp)
	{
		while (fgets(buf, sizeof(buf), fp) != NULL)
		{
			if (++n == 1 && strncmp(buf, "Iface", 5) == 0)
				continue;

			i = sscanf(buf, "%255s %x %*s %*s %*s %*s %*s %x",
						device, &dest, &mask);

			if (i != 3)
				break;

			if (device[0] != '\0' && dest == 0 && mask == 0)
			{
				found = 1;
				break;
			}
		}

		fclose(fp);

		if (found && !strcmp("ppp0", device))
			return 1;
		else
			return 0;
	}

	return 0;
}

void
reltime(unsigned long seconds, char *cs)
{
        int days=0, hours=0, minutes=0;

        if (seconds > 60*60*24) {
                days = seconds / (60*60*24);
                seconds %= 60*60*24;
        }
        if (seconds > 60*60) {
                hours = seconds / (60*60);
                seconds %= 60*60;
        }
        if (seconds > 60) {
                minutes = seconds / 60;
                seconds %= 60;
        }
        sprintf(cs, "%d days, %d hours, %d minutes, %ld seconds", days, hours, minutes, seconds);
}

void
print_uptime(void)
{
        char buf[64];
        double secs;

        FILE *fp = fopen("/proc/uptime", "r");
        if (!fp) {
		fprintf(stderr, "fopen error!\n");
		dbg("fopen error!\n");
                return;
        }

        fgets(buf, 64, fp);
        fclose(fp);

        secs = atof(buf);
        reltime((unsigned long) secs, buf);
	fprintf(stderr, "uptime: %s\n\n", buf);
	dbg("uptime: %s\n\n", buf);
}

int
found_default_route(int only_broadband_wan)
{
	int i, n, found;
	FILE *fp;
	unsigned int dest, mask;
	char buf[256], device[256];
	n = 0;
	found = 0;
	mask = 0;
	device[0] = '\0';

	fp = fopen("/proc/net/route", "r");
	if (fp)
	{
		while (fgets(buf, sizeof(buf), fp) != NULL)
		{
			if (++n == 1 && strncmp(buf, "Iface", 5) == 0)
				continue;

			i = sscanf(buf, "%255s %x %*s %*s %*s %*s %*s %x",
						device, &dest, &mask);

			if (i != 3)
			{
//				dbg("junk in buffer");
				break;
			}

			if (device[0] != '\0' && dest == 0 && mask == 0)
			{
//				dbg("default route dev: %s\n", device);
				found = 1;
				break;
			}
		}

		fclose(fp);

		if (found)
		{
			if(!only_broadband_wan && get_usb_modem_state()){
				if(nvram_match("modem_enable", "4")){
					if(!strcmp(nvram_safe_get("rndis_ifname"), device))
						return 1;
					else
						goto no_default_route;
				}
				else
				{
					if(!strcmp("ppp0", device))
						return 1;
					else
						goto no_default_route;
				}
			}
			else
			if (nvram_match("wan0_proto", "dhcp") || nvram_match("wan0_proto", "static"))
			{
				if (!strcmp(IFNAME_WAN, device))
					return 1;
				else
					goto no_default_route;
			}
			else
			{
				if (!strcmp("ppp0", device) || !strcmp(IFNAME_WAN, device))
					return 1;
				else
					goto no_default_route;
			}
		}
		else
			goto no_default_route;
	}

no_default_route:

	return 0;
}
