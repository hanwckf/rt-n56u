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
#include <stdarg.h>
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
#include <net/route.h>
#include <dirent.h>

#include <nvram/bcmnvram.h>
#include <shutils.h>

#include "rc.h"

#define SR_BUF_LEN (8192)

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

int
control_static_routes(char *ift, char *ifname, int is_add)
{
	char word[128], *next;
	char *route_buf;
	char *ipaddr, *netmask, *gateway, *metric;

	if (is_add && nvram_invmatch("sr_enable_x", "1"))
		return 0;

	route_buf = (char*)malloc(SR_BUF_LEN*sizeof(char));
	if (!route_buf)
		return -1;

	fill_static_routes(route_buf, SR_BUF_LEN, ift);

	foreach(word, route_buf, next) {
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
		
		if (is_add)
			route_add(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
		else
			route_del(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
	}

	free(route_buf);

	return 0;
}

void
stop_igmpproxy(void)
{
	char* svcs[] = { "udpxy", "igmpproxy", NULL };
	kill_services(svcs, 3, 1);
}

void
start_igmpproxy(char *wan_ifname)
{
	static char *igmpproxy_conf = "/etc/igmpproxy.conf";
	FILE *fp;
	char *altnet = nvram_safe_get("mr_altnet_x");
	char *altnet_mask;
	
	// Allways close old instance of igmpproxy and udpxy (interface may changed)
	stop_igmpproxy();
	
	if (nvram_get_int("udpxy_enable_x") > 0)
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

int 
is_ap_mode(void)
{
	if ((nvram_match("wan_nat_x", "0")) && (nvram_match("wan_route_x", "IP_Bridged")))
		return 1;
	else
		return 0;
}

void 
flush_conntrack_caches(void)
{
	fput_int("/proc/sys/net/nf_conntrack_table_flush", 1);
}

void 
flush_route_caches(void)
{
	fput_int("/proc/sys/net/ipv4/route/flush", -1);
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
	int sw_mode = nvram_get_int("sw_mode");
	int hw_nat_mode = nvram_get_int("hw_nat_mode");

	if (sw_mode != 1 && sw_mode != 4)
		return 0;

	if (hw_nat_mode == 2)
		return 0;

	if (hw_nat_mode == 1 || hw_nat_mode == 4)
		return 2; // wifi_offload=1

	return 1; // wifi_offload=0
}

int is_fastnat_allow(void)
{
	if ( nvram_match("sw_nat_mode", "1") && nvram_match("sw_mode", "1") )
	{
		return 1;
	}
	
	return 0;
}

void hwnat_load(void)
{
	int hw_nat_mode = nvram_get_int("hw_nat_mode");
	doSystem("modprobe -q hw_nat wifi_offload=%d", (hw_nat_mode == 1 || hw_nat_mode == 4) ? 1 : 0);
}


void hwnat_unload(void)
{
	system("rmmod hw_nat");
}


void hwnat_configure(void)
{
	int hw_nat_mode, ipv6_type, ppe_udp, ppe_ipv6;
	char *hwnat_status = "Disabled";

	if (!is_module_loaded("hw_nat")) {
		logmessage(LOGNAME, "%s: %s", "Hardware NAT/Routing", hwnat_status);
		return;
	}

	hw_nat_mode = nvram_get_int("hw_nat_mode");
	ppe_udp = (hw_nat_mode == 3 || hw_nat_mode == 4) ? 1 : 0;
	doSystem("/bin/hw_nat %s %d", "-Y", ppe_udp);
	
	if (hw_nat_mode == 1 || hw_nat_mode == 4)
		hwnat_status = "Enabled, IPoE/PPPoE offload [WAN]<->[LAN/Wi-Fi]";
	else if (hw_nat_mode == 0 || hw_nat_mode == 3)
		hwnat_status = "Enabled, IPoE/PPPoE offload [WAN]<->[LAN]";

	logmessage(LOGNAME, "%s: %s", "Hardware NAT/Routing", hwnat_status);
	logmessage(LOGNAME, "%s: IPv4 UDP flow offload - %s", "Hardware NAT/Routing", (ppe_udp) ? "ON" : "OFF");

#if defined (USE_IPV6)
	ipv6_type = get_ipv6_type();
	if (nvram_get_int("ip6_ppe_on") && (ipv6_type == IPV6_NATIVE_STATIC || ipv6_type == IPV6_NATIVE_DHCP6))
		ppe_ipv6 = 1;
	else
		ppe_ipv6 = 0;

	doSystem("/bin/hw_nat %s %d", "-6", ppe_ipv6);
	logmessage(LOGNAME, "%s: IPv6 routes offload - %s", "Hardware NAT/Routing", (ppe_ipv6) ? "ON" : "OFF");
#endif
}

void swnat_configure(void)
{
#if !defined (USE_KERNEL3X)
	int swnat_allow = is_fastnat_allow();
	fput_int("/proc/sys/net/nf_conntrack_fastnat", swnat_allow);
	if (nvram_match("sw_mode", "1"))
		logmessage(LOGNAME, "Software FastNAT: %s", (swnat_allow) ? "Enabled" : "Disabled");
#endif
}

void reload_nat_modules(void)
{
	int loaded_ftp;
	int needed_ftp = 0;
	int needed_sip = 0;
	int needed_h323 = 0;
	int needed_pptp = 0;
	int wan_nat_x = nvram_get_int("wan_nat_x");
	int hwnat_allow = is_hwnat_allow();
	int hwnat_loaded = is_hwnat_loaded();
	
	if (nvram_match("wan_route_x", "IP_Routed"))
	{
		needed_ftp = nvram_get_int("nf_alg_ftp1");
		if (needed_ftp < 1024 || needed_ftp > 65535) needed_ftp = 21;
		
		if (nvram_match("nf_alg_pptp", "1"))
			needed_pptp = 1;
		
		if (nvram_match("nf_alg_h323", "1"))
			needed_h323 = 1;
		
		if (nvram_match("nf_alg_sip", "1"))
			needed_sip = 1;
	}
	
	if ((hwnat_loaded) && ((!hwnat_allow) || (hwnat_loaded != hwnat_allow)))
	{
		hwnat_loaded = 0;
		hwnat_unload();
	}
	
	if (needed_pptp)
	{
		if (wan_nat_x == 0)
			system("modprobe -q nf_conntrack_pptp");
		else
			system("modprobe -q nf_nat_pptp");
	}
	else
		system("modprobe -r nf_nat_pptp");
	
	if (needed_h323)
	{
		if (wan_nat_x == 0)
			system("modprobe -q nf_conntrack_h323");
		else
			system("modprobe -q nf_nat_h323");
	}
	else
		system("modprobe -r nf_nat_h323");
	
	if (needed_sip)
	{
		if (wan_nat_x == 0)
			system("modprobe -q nf_conntrack_sip");
		else
			system("modprobe -q nf_nat_sip");
	}
	else
		system("modprobe -r nf_nat_sip");
	
	loaded_ftp = is_ftp_conntrack_loaded(needed_ftp);
	if (loaded_ftp == 1)
	{
		system("rmmod nf_nat_ftp 2>/dev/null");
		system("rmmod nf_conntrack_ftp 2>/dev/null");
	}
	
	if (needed_ftp && loaded_ftp != 2)
	{
		if (needed_ftp != 21)
			doSystem("modprobe -q nf_conntrack_ftp ports=21,%d", needed_ftp);
		else
			system("modprobe -q nf_conntrack_ftp");
		
		if (wan_nat_x != 0)
			system("modprobe -q nf_nat_ftp");
	}
	
	if (hwnat_allow && !hwnat_loaded)
		hwnat_load();

	hwnat_configure();
	swnat_configure();
}

void restart_firewall(void)
{
	char wan_ifname[16];
	
	wan_ifname[0] = 0;
	strncpy(wan_ifname, nvram_safe_get("wan_ifname_t"), sizeof(wan_ifname));
	if (strlen(wan_ifname) == 0) {
		get_wan_ifname(wan_ifname);
	}
	
	start_firewall_ex(wan_ifname, nvram_safe_get("wan0_ipaddr"));
	
	/* update upnp forwards from lease file */
	update_upnp(0);
}

void set_ip_forward(void)
{
	if (nvram_match("router_disable", "1"))
		return;
	
	/* Enable Forwarding IPv4 */
	fput_int("/proc/sys/net/ipv4/ip_forward", 1);
}

void set_ppp_limit_cpu(void)
{
#if !defined (USE_KERNEL3X)
	int cpu_lim = nvram_get_int("wan_pppoe_cpul");
	if (cpu_lim < 0 || cpu_lim > 5000) cpu_lim = 0;

	fput_int("/proc/sys/net/ipv4/ppp_cpu_load_limit", cpu_lim);
#endif
}

void set_pppoe_passthrough(void)
{
#if defined (USE_KERNEL3X)
	char* svcs[] = { "pppoe-relay", NULL };
	if (nvram_match("fw_pt_pppoe", "1") && nvram_invmatch("router_disable", "1")) {
		if (!pids(svcs[0]))
			eval("/usr/sbin/pppoe-relay", "-C", IFNAME_BR, "-S", IFNAME_WAN);
	}
	else
		kill_services(svcs, 3, 1);
#else
	char pthrough[32];
	
	if (nvram_match("fw_pt_pppoe", "1") && nvram_invmatch("router_disable", "1"))
		sprintf(pthrough, "%s,%s\n", IFNAME_BR, IFNAME_WAN);
	else
		strcpy(pthrough, "null,null\n");
	
	fput_string("/proc/pthrough/pppoe", pthrough);
#endif
}

void disable_all_passthrough(void)
{
#if !defined (USE_KERNEL3X)
	char pthrough[32];

	strcpy(pthrough, "null,null\n");

	fput_string("/proc/pthrough/pppoe", pthrough);
	fput_string("/proc/pthrough/ipv6", pthrough);
#endif
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



