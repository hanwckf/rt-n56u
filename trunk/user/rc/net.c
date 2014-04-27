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

#include "rc.h"

#define SR_BUF_LEN (8192)

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
		if (!is_valid_ipv4(gateway))
			gateway = nvram_safe_get("wanx_gateway");	// oleg patch
		
		if (is_add)
			route_add(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
		else
			route_del(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
	}

	free(route_buf);

	return 0;
}

char*
sanity_hostname(char *hname)
{
	char *c;

	for (c = hname; *c; c++) {
		if (*c == 0x20)
			*c = '_';
	}

	return hname;
}

char*
get_our_hostname(void)
{
	char* host_name = nvram_safe_get("computer_name");
	if (!(*host_name) || !is_valid_hostname(host_name))
		host_name = nvram_safe_get("productid");
	return sanity_hostname(host_name);
}

int
is_same_subnet(char *ip1, char *ip2, char *msk)
{
	unsigned int addr1, addr2, mask;

	addr1 = ntohl(inet_addr(ip1));
	addr2 = ntohl(inet_addr(ip2));
	mask  = ntohl(inet_addr(msk));

	return (addr1 & mask) == (addr2 & mask);
}

int
is_same_subnet2(char *ip1, char *ip2, char *msk1, char *msk2)
{
	unsigned int addr1, addr2, mask, mask1, mask2;

	addr1 = ntohl(inet_addr(ip1));
	addr2 = ntohl(inet_addr(ip2));
	mask1 = ntohl(inet_addr(msk1));
	mask2 = ntohl(inet_addr(msk2));

	mask = (mask1 < mask2) ? mask1 : mask2;

	return (addr1 & mask) == (addr2 & mask);
}

#if defined(APP_XUPNPD)
int is_xupnpd_support(void)
{
	return check_if_file_exist("/usr/bin/xupnpd");
}

void stop_xupnpd(void)
{
	char* svcs[] = { "xupnpd", NULL };
	
	if (!is_xupnpd_support())
		return;
	
	kill_services(svcs, 3, 1);
}

void start_xupnpd(char *wan_ifname)
{
	int i, xport, has_daemon;
	FILE *fp1, *fp2;
	char tmp1[64], tmp2[64], line[256];
	char *dir_src = "/etc_ro/xupnpd";
	char *dir_dst = "/etc/storage/xupnpd";
	char *xdir1[] = { "config", "playlists", NULL };
	char *xdir2[] = { "plugins", "profiles", NULL };
	char *xlua[] = { "", "_http", "_m3u", "_main", "_mime", "_soap", "_ssdp", NULL };

	if (!is_xupnpd_support())
		return;

	unlink("/var/run/xupnpd.pid");

	xport = nvram_get_int("xupnpd_enable_x");
	if (xport < 1024)
		return;

	if (!check_if_dir_exist(dir_dst))
		mkdir(dir_dst, 0755);

	for (i=0; xdir1[i]; i++)
	{
		snprintf(tmp2, sizeof(tmp2), "%s/%s", dir_dst, xdir1[i]);
		if (!check_if_dir_exist(tmp2))
			mkdir(tmp2, 0755);
	}

	for (i=0; xdir2[i]; i++)
	{
		snprintf(tmp2, sizeof(tmp2), "%s/%s", dir_dst, xdir2[i]);
		if (!check_if_dir_exist(tmp2)) {
			snprintf(tmp1, sizeof(tmp1), "%s/%s", dir_src, xdir2[i]);
			if (get_mtd_size("Storage") > 0x10000)
				doSystem("cp -rf %s %s", tmp1, tmp2);
			else
				symlink(tmp1, tmp2);
		}
	}

	for (i=0; xlua[i]; i++)
	{
		snprintf(tmp1, sizeof(tmp1), "%s/xupnpd%s.lua", dir_src, xlua[i]);
		snprintf(tmp2, sizeof(tmp2), "%s/xupnpd%s.lua", dir_dst, xlua[i]);
		if (!check_if_file_exist(tmp2))
			doSystem("cp -f %s %s", tmp1, tmp2);
	}

	snprintf(tmp1, sizeof(tmp1), "%s/config/common.lua.tmp", dir_dst);
	snprintf(tmp2, sizeof(tmp2), "%s/config/common.lua", dir_dst);
	fp1 = fopen(tmp1, "w");
	if (fp1) {
		fp2 = fopen(tmp2, "r");
		if (fp2) {
			while (fgets(line, sizeof(line), fp2)){
				if (strstr(line, "mcast_interface") && !strstr(line, "--"))
					snprintf(line, sizeof(line), "cfg[\"mcast_interface\"]=\"%s\"\n", wan_ifname);
				else if (strstr(line, "http_port") && !strstr(line, "--"))
					snprintf(line, sizeof(line), "cfg[\"http_port\"]=%d\n", xport);
				fprintf(fp1, "%s", line);
			}
			fclose(fp2);
		}
		fclose(fp1);
		doSystem("mv -f %s %s", tmp1, tmp2);
	}

	has_daemon = 0;
	snprintf(tmp1, sizeof(tmp1), "%s/xupnpd.lua.tmp", dir_dst);
	snprintf(tmp2, sizeof(tmp2), "%s/xupnpd.lua", dir_dst);
	fp1 = fopen(tmp1, "w");
	if (fp1) {
		fp2 = fopen(tmp2, "r");
		if (fp2) {
			while (fgets(line, sizeof(line), fp2)){
				if (strstr(line, "cfg.mcast_interface=") && !strstr(line, "--"))
					snprintf(line, sizeof(line), "cfg.mcast_interface='%s'\n", wan_ifname);
				else if (strstr(line, "cfg.http_port=") && !strstr(line, "--"))
					snprintf(line, sizeof(line), "cfg.http_port=%d\n", xport);
				else if (strstr(line, "cfg.udpxy_url=")) {
					char *lua_rem = "--";
					char *lan_add = nvram_safe_get("lan_ipaddr");
					int udpxy_port = nvram_get_int("udpxy_enable_x");
					if (udpxy_port > 1023 && nvram_match("xupnpd_udpxy", "1"))
						lua_rem = "";
					if (udpxy_port == 0)
						udpxy_port = 4022;
					snprintf(line, sizeof(line), "%scfg.udpxy_url='http://%s:%d'\n", lua_rem, lan_add, udpxy_port);
				}
				else if (strstr(line, "cfg.daemon") && !strstr(line, "--")) {
					snprintf(line, sizeof(line), "cfg.daemon=true\n");
					has_daemon = 1;
				}
				fprintf(fp1, "%s", line);
			}
			fclose(fp2);
		}
		fclose(fp1);
		doSystem("mv -f %s %s", tmp1, tmp2);
		
		if (has_daemon)
			eval("/usr/bin/xupnpd");
		else
			system("/usr/bin/xupnpd &");
	}
}
#endif

void
stop_igmpproxy(char *wan_ifname)
{
	char *svcs[] = { "udpxy", "igmpproxy", NULL };
	char *viptv_iflast;

	/* check used IPTV via VLAN interface */
	viptv_iflast = nvram_safe_get("viptv_ifname");
	if (*viptv_iflast && is_interface_exist(viptv_iflast) && strcmp(wan_ifname, viptv_iflast))
		return;

	kill_services(svcs, 3, 1);
}

void
start_igmpproxy(char *wan_ifname)
{
	FILE *fp;
	char *igmpproxy_conf = "/etc/igmpproxy.conf";
	char *altnet, *altnet_mask, *viptv_iflast;

	/* check used IPTV via VLAN interface */
	viptv_iflast = nvram_safe_get("viptv_ifname");
	if (*viptv_iflast && is_interface_exist(viptv_iflast) && strcmp(wan_ifname, viptv_iflast))
		return;

	// Allways close old instance of igmpproxy and udpxy (interface may changed)
#if defined(APP_XUPNPD)
	stop_xupnpd();
#endif
	stop_igmpproxy(wan_ifname);

	if (nvram_get_int("udpxy_enable_x") > 1023)
	{
		eval("/usr/sbin/udpxy", 
			"-m", wan_ifname, 
			"-p", nvram_safe_get("udpxy_enable_x"),
			"-B", "65536",
			"-c", "5"
			);
	}

#if defined(APP_XUPNPD)
	start_xupnpd(wan_ifname);
#endif

	if (nvram_match("mr_enable_x", "1")) 
	{
		if ((fp = fopen(igmpproxy_conf, "w")))
		{
			altnet = nvram_safe_get("mr_altnet_x");
			if (altnet && strlen(altnet) > 0)
				altnet_mask = altnet;
			else
				altnet_mask = "0.0.0.0/0";
			fprintf(fp, "# automagically generated from web settings\n"
				"quickleave\n\n"
				"phyint %s upstream  ratelimit 0  threshold 1\n"
				"\taltnet %s\n\n"
				"phyint %s downstream  ratelimit 0  threshold 1\n\n",
				wan_ifname, 
				altnet_mask, 
				IFNAME_BR);
			fclose(fp);
			
			eval("/bin/igmpproxy", igmpproxy_conf);
		}
	}
}

void
restart_iptv(void)
{
	char *wan_ifname, *viptv_iflast;
	
	/* check used IPTV via VLAN interface */
	viptv_iflast = nvram_safe_get("viptv_ifname");
	if (*viptv_iflast && is_interface_exist(viptv_iflast))
		wan_ifname = viptv_iflast;
	else
		wan_ifname = get_man_ifname(0);
	
	config_bridge();
	start_igmpproxy(wan_ifname);
}

void 
flush_conntrack_caches(void)
{
	fput_int("/proc/sys/net/nf_conntrack_table_flush", 1);
//	fput_string("/proc/net/nf_conntrack", "f");
}

void 
flush_route_caches(void)
{
	system("ip route flush cache");
}

void 
clear_if_route4(char *ifname)
{
	doSystem("ip route flush dev %s scope %s", ifname, "global");
}

int is_ftp_conntrack_loaded(int ftp_port0, int ftp_port1)
{
	char ports_use[64] = {0};
	char ports_val[64];

	if (module_param_get("nf_conntrack_ftp", "ports", ports_use, sizeof(ports_use)))
		return 0;

	if (ftp_port0 || ftp_port1)
	{
		if (ftp_port0 && ftp_port1)
			sprintf(ports_val, "%d,%d", ftp_port0, ftp_port1);
		else if (ftp_port0)
			sprintf(ports_val, "%d", ftp_port0);
		else
			sprintf(ports_val, "%d", ftp_port1);
		
		if (strcmp(ports_val, ports_use) == 0)
			return 2;
	}

	return 1;
}

int is_hwnat_loaded(void)
{
	char offload_val[32] = {0};

	if (module_param_get("hw_nat", "wifi_offload", offload_val, sizeof(offload_val)))
		return 0;

	if (offload_val[0] == 'Y' || offload_val[0] == '1')
		return 2;

	return 1;
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
	char hnat_param[32];
	int hnat_mode = nvram_get_int("hw_nat_mode");

	snprintf(hnat_param, sizeof(hnat_param), "wifi_offload=%d", (hnat_mode == 1 || hnat_mode == 4) ? 1 : 0);
	module_smart_load("hw_nat", hnat_param);
}

void hwnat_configure(void)
{
	int hw_nat_mode, ppe_udp, wan_vid;
	char *hwnat_status = "Disabled";

	if (!is_module_loaded("hw_nat")) {
		logmessage(LOGNAME, "%s: %s", "Hardware NAT/Routing", hwnat_status);
		return;
	}

	wan_vid = get_vlan_vid_wan();
	hw_nat_mode = nvram_get_int("hw_nat_mode");
	ppe_udp = (hw_nat_mode == 3 || hw_nat_mode == 4) ? 1 : 0;
	doSystem("/bin/hw_nat %s %d %d", "-V", 1, wan_vid);
	doSystem("/bin/hw_nat %s %d", "-Y", ppe_udp);

	if (hw_nat_mode == 1 || hw_nat_mode == 4)
#if defined(USE_WWAN_HW_NAT)
		hwnat_status = "Enabled, IPoE/PPPoE offload [WAN/WWAN]<->[LAN/WLAN]";
#else
		hwnat_status = "Enabled, IPoE/PPPoE offload [WAN]<->[LAN/WLAN]";
#endif
	else if (hw_nat_mode == 0 || hw_nat_mode == 3)
		hwnat_status = "Enabled, IPoE/PPPoE offload [WAN]<->[LAN]";

	logmessage(LOGNAME, "%s: %s", "Hardware NAT/Routing", hwnat_status);
	logmessage(LOGNAME, "%s: IPv4 UDP flow offload - %s", "Hardware NAT/Routing", (ppe_udp) ? "ON" : "OFF");

#if defined(USE_IPV6)
	{
		int ppe_ipv6 = 0;
#if defined(USE_IPV6_HW_NAT)
		int ipv6_type = get_ipv6_type();
#if defined(USE_HW_NAT_V2)
		if (nvram_get_int("ip6_ppe_on") && (ipv6_type != IPV6_DISABLED))
			ppe_ipv6 = 1;
#else
		if (nvram_get_int("ip6_ppe_on") && (ipv6_type == IPV6_NATIVE_STATIC || ipv6_type == IPV6_NATIVE_DHCP6))
			ppe_ipv6 = 1;
#endif
		doSystem("/bin/hw_nat %s %d", "-6", ppe_ipv6);
#endif
		logmessage(LOGNAME, "%s: IPv6 routes offload - %s", "Hardware NAT/Routing", (ppe_ipv6) ? "ON" : "OFF");
	}
#endif
}

void reload_nat_modules(void)
{
	int loaded_ftp;
	int needed_ftp0 = 0;
	int needed_ftp1 = 0;
	int needed_sip = 0;
	int needed_h323 = 0;
	int needed_rtsp = 0;
	int needed_pptp = 0;
	int wan_nat_x = nvram_get_int("wan_nat_x");
	int hwnat_allow = is_hwnat_allow();
	int hwnat_loaded = is_hwnat_loaded();
	
	if (!get_ap_mode())
	{
		needed_ftp0 = nvram_get_int("nf_alg_ftp0");
		needed_ftp1 = nvram_get_int("nf_alg_ftp1");
		if (needed_ftp0 <   21 || needed_ftp0 > 65535) needed_ftp0 = 0;
		if (needed_ftp1 < 1024 || needed_ftp1 > 65535) needed_ftp1 = 0;
		
		if (nvram_match("nf_alg_pptp", "1"))
			needed_pptp = 1;
		
		if (nvram_match("nf_alg_h323", "1"))
			needed_h323 = 1;
		
		if (nvram_match("nf_alg_rtsp", "1"))
			needed_rtsp = 1;
		
		if (nvram_match("nf_alg_sip", "1"))
			needed_sip = 1;
	}
	
	if ((hwnat_loaded) && ((!hwnat_allow) || (hwnat_loaded != hwnat_allow)))
	{
		hwnat_loaded = 0;
		module_smart_unload("hw_nat", 0);
	}

	if (needed_pptp)
	{
		module_smart_load("nf_conntrack_pptp", NULL);
		if (wan_nat_x != 0)
			module_smart_load("nf_nat_pptp", NULL);
	}
	else
		module_smart_unload("nf_nat_pptp", 1);

	if (needed_rtsp)
	{
		module_smart_load("nf_conntrack_rtsp", "ports=554,8554");
		if (wan_nat_x != 0)
			module_smart_load("nf_nat_rtsp", NULL);
	}
	else
		module_smart_unload("nf_nat_rtsp", 1);

	if (needed_h323)
	{
		module_smart_load("nf_conntrack_h323", NULL);
		if (wan_nat_x != 0)
			module_smart_load("nf_nat_h323", NULL);
	}
	else
		module_smart_unload("nf_nat_h323", 1);

	if (needed_sip)
	{
		module_smart_load("nf_conntrack_sip", NULL);
		if (wan_nat_x != 0)
			module_smart_load("nf_nat_sip", NULL);
	}
	else
		module_smart_unload("nf_nat_sip", 1);

	loaded_ftp = is_ftp_conntrack_loaded(needed_ftp0, needed_ftp1);
	if (loaded_ftp == 1)
	{
		module_smart_unload("nf_nat_ftp", 0);
		module_smart_unload("nf_conntrack_ftp", 0);
	}

	if ((loaded_ftp != 2) && (needed_ftp0 || needed_ftp1))
	{
		char ports_val[32];
		
		if (needed_ftp0 && needed_ftp1)
			sprintf(ports_val, "ports=%d,%d", needed_ftp0, needed_ftp1);
		else if (needed_ftp0)
			sprintf(ports_val, "ports=%d", needed_ftp0);
		else
			sprintf(ports_val, "ports=%d", needed_ftp1);
		
		module_smart_load("nf_conntrack_ftp", ports_val);
		if (wan_nat_x != 0)
			module_smart_load("nf_nat_ftp", NULL);
	}

	if (hwnat_allow && !hwnat_loaded)
		hwnat_load();

	hwnat_configure();
}

void restart_firewall(void)
{
	char wan_ifname[16];
	
	wan_ifname[0] = 0;
	strncpy(wan_ifname, nvram_safe_get("wan_ifname_t"), sizeof(wan_ifname));
	if (strlen(wan_ifname) == 0)
		get_wan_ifname(wan_ifname);
	
	start_firewall_ex(wan_ifname, nvram_safe_get("wan0_ipaddr"));
	
	/* update upnp forwards from lease file */
	update_upnp();
}

void set_ipv4_forward(void)
{
	/* Enable Forwarding IPv4 */
	fput_int("/proc/sys/net/ipv4/ip_forward", 1);
}

void set_force_igmp_mld(void)
{
	char tmp[64];
	char *ifname = get_man_ifname(0);
	int force_value;

	force_value = nvram_safe_get_int("force_igmp", 2, 0, 2);

	sprintf(tmp, "/proc/sys/net/%s/conf/%s/%s", "ipv4", "all", "force_igmp_version");
	fput_int(tmp, force_value);

	sprintf(tmp, "/proc/sys/net/%s/conf/%s/%s", "ipv4", ifname, "force_igmp_version");
	fput_int(tmp, force_value);

	sprintf(tmp, "/proc/sys/net/%s/conf/%s/%s", "ipv4", IFNAME_BR, "force_igmp_version");
	fput_int(tmp, force_value);

#if defined (USE_IPV6)
	force_value = nvram_safe_get_int("force_mld", 0, 0, 1);

	sprintf(tmp, "/proc/sys/net/%s/conf/%s/%s", "ipv6", "all", "force_mld_version");
	fput_int(tmp, force_value);

	sprintf(tmp, "/proc/sys/net/%s/conf/%s/%s", "ipv6", ifname, "force_mld_version");
	fput_int(tmp, force_value);

	sprintf(tmp, "/proc/sys/net/%s/conf/%s/%s", "ipv6", IFNAME_BR, "force_mld_version");
	fput_int(tmp, force_value);
#endif
}

void set_pppoe_passthrough(void)
{
	char pthrough[32];

	if (nvram_match("fw_pt_pppoe", "1"))
		sprintf(pthrough, "%s,%s\n", IFNAME_BR, get_man_ifname(0));
	else
		strcpy(pthrough, "null,null\n");

	fput_string("/proc/net/pthrough/pppoe", pthrough);
}

void disable_all_passthrough(void)
{
	char pthrough[16];

	strcpy(pthrough, "null,null\n");

	fput_string("/proc/net/pthrough/pppoe", pthrough);
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



