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

#include "rc.h"

#define SR_BUF_LEN (8192)

static int
route_manip(int cmd, char *ifname, int metric, char *dst, char *gateway, char *genmask)
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
	rt.rt_dev = ifname;

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
route_add(char *ifname, int metric, char *dst, char *gateway, char *genmask)
{
	return route_manip(SIOCADDRT, ifname, metric+1, dst, gateway, genmask);
}

int
route_del(char *ifname, int metric, char *dst, char *gateway, char *genmask)
{
	return route_manip(SIOCDELRT, ifname, metric+1, dst, gateway, genmask);
}

int
control_static_routes(char *ift, char *ifname, int is_add)
{
	char word[128], *next;
	char *route_buf;
	char *ipaddr, *netmask, *gateway, *metric;

	if (is_add && nvram_invmatch("sr_enable_x", "1"))
		return 0;

	route_buf = (char *)malloc(SR_BUF_LEN*sizeof(char));
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
			route_add(ifname, atoi(metric), ipaddr, gateway, netmask);
		else
			route_del(ifname, atoi(metric), ipaddr, gateway, netmask);
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
is_same_subnet(const char *ip1, const char *ip2, const char *msk)
{
	unsigned int addr1, addr2, mask;

	addr1 = ntohl(inet_addr(ip1));
	addr2 = ntohl(inet_addr(ip2));
	mask  = ntohl(inet_addr(msk));

	return (addr1 & mask) == (addr2 & mask);
}

int
is_same_subnet2(const char *ip1, const char *ip2, const char *msk1, const char *msk2)
{
	unsigned int addr1, addr2, mask, mask1, mask2;

	addr1 = ntohl(inet_addr(ip1));
	addr2 = ntohl(inet_addr(ip2));
	mask1 = ntohl(inet_addr(msk1));
	mask2 = ntohl(inet_addr(msk2));

	mask = (mask1 < mask2) ? mask1 : mask2;

	return (addr1 & mask) == (addr2 & mask);
}

void
create_vlan_iface(const char *dev_ifname, int vid, int prio, int mtu, const char *hwaddr, int do_up)
{
	char vlan_ifname[24];

	snprintf(vlan_ifname, sizeof(vlan_ifname), "%s.%d", dev_ifname, vid);
	if (!is_interface_exist(vlan_ifname))
		vconfig_add_if(dev_ifname, vid);
	else
		ifconfig(vlan_ifname, 0, NULL, NULL);

	if (prio >= 0 && prio <= 7)
		vconfig_egress_map(vlan_ifname, 0, prio);

	if (mtu >= 1280)
		set_interface_mtu(vlan_ifname, mtu);

	if (hwaddr)
		set_interface_hwaddr(vlan_ifname, hwaddr);

	if (do_up)
		ifconfig(vlan_ifname, IFUP, "0.0.0.0", NULL);
}

void
remove_vlan_iface(const char *vlan_ifname)
{
	if (!strstr(vlan_ifname, "."))
		return;
	if (!is_interface_exist(vlan_ifname))
		return;
	ifconfig(vlan_ifname, 0, NULL, NULL);
	vconfig_del_if(vlan_ifname);
}

void
stop_udpxy(void)
{
	char* svcs[] = { "udpxy", NULL };

	kill_services(svcs, 3, 1);
}

void
start_udpxy(char *wan_ifname)
{
	if (nvram_get_int("udpxy_enable_x") < 1024)
		return;

	eval("/usr/sbin/udpxy",
		"-m", wan_ifname,
		"-p", nvram_safe_get("udpxy_enable_x"),
		"-B", "65536",
		"-c", nvram_safe_get("udpxy_clients"),
		"-M", nvram_safe_get("udpxy_renew_period")
		);
}

#if defined(APP_XUPNPD)
int
is_xupnpd_support(void)
{
	return check_if_file_exist("/usr/bin/xupnpd");
}

void
stop_xupnpd(void)
{
	char* svcs[] = { "xupnpd", NULL };

	if (!is_xupnpd_support())
		return;

	kill_services(svcs, 3, 1);
}

void
start_xupnpd(char *wan_ifname)
{
	int i, xport, has_daemon;
	FILE *fp1, *fp2;
	char tmp1[64], tmp2[64], line[256];
	char *dir_src = "/etc_ro/xupnpd";
	char *dir_dst = "/etc/storage/xupnpd";
	char *xdir1[] = { "config", "playlists", NULL };
	char *xdir2[] = { "plugins", "profiles", NULL };
	char *xlua[] = { "_http", "_m3u", "_main", "_mime", "_soap", "_ssdp", "_webapp", NULL };

	if (!is_xupnpd_support())
		return;

	unlink("/var/run/xupnpd.pid");

	xport = nvram_get_int("xupnpd_enable_x");
	if (xport < 1024)
		return;

	if (!check_if_dir_exist(dir_dst))
		mkdir(dir_dst, 0755);

	for (i=0; xdir1[i]; i++) {
		snprintf(tmp2, sizeof(tmp2), "%s/%s", dir_dst, xdir1[i]);
		if (!check_if_dir_exist(tmp2))
			mkdir(tmp2, 0755);
	}

	for (i=0; xdir2[i]; i++) {
		snprintf(tmp2, sizeof(tmp2), "%s/%s", dir_dst, xdir2[i]);
		if (!check_if_dir_exist(tmp2)) {
			snprintf(tmp1, sizeof(tmp1), "%s/%s", dir_src, xdir2[i]);
			if (get_mtd_size("Storage") > 0x10000)
				doSystem("cp -rf %s %s", tmp1, tmp2);
			else
				symlink(tmp1, tmp2);
		}
	}

	for (i=0; xlua[i]; i++) {
		snprintf(tmp2, sizeof(tmp2), "%s/xupnpd%s.lua", dir_dst, xlua[i]);
		unlink(tmp2);
	}

	snprintf(tmp1, sizeof(tmp1), "%s/xupnpd%s.lua", dir_src, "");
	snprintf(tmp2, sizeof(tmp2), "%s/xupnpd%s.lua", dir_dst, "");
	if (!check_if_file_exist(tmp2))
		doSystem("cp -f %s %s", tmp1, tmp2);

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
					char *lan_addr = nvram_safe_get("lan_ipaddr_t");
					int udpxy_port = nvram_get_int("udpxy_enable_x");
					if (udpxy_port > 1023 && nvram_match("xupnpd_udpxy", "1"))
						lua_rem = "";
					if (udpxy_port == 0)
						udpxy_port = 4022;
					snprintf(line, sizeof(line), "%scfg.udpxy_url='http://%s:%d'\n", lua_rem, lan_addr, udpxy_port);
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
	char *svcs[] = { "xupnpd", "udpxy", "igmpproxy", NULL };

	/* check used IPTV via VLAN interface */
	if (wan_ifname) {
		char *viptv_iflast = nvram_safe_get("viptv_ifname");
		if (*viptv_iflast && is_interface_exist(viptv_iflast) && strcmp(wan_ifname, viptv_iflast))
			return;
	}

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

	/* Always close old instance of igmpproxy and udpxy (interface may changed) */
	stop_igmpproxy(wan_ifname);

	start_udpxy(wan_ifname);
#if defined(APP_XUPNPD)
	start_xupnpd(wan_ifname);
#endif

	if (nvram_invmatch("mr_enable_x", "1"))
		return;

	fp = fopen(igmpproxy_conf, "w");
	if (fp) {
		altnet = nvram_get("mr_altnet_x");
		if (altnet && strlen(altnet) > 6)
			altnet_mask = altnet;
		else
			altnet_mask = "0.0.0.0/0";
		fprintf(fp, "# automatically generated\n");
		if (nvram_invmatch("mr_qleave_x", "0"))
			fprintf(fp, "quickleave\n");
		fprintf(fp, "\nphyint %s %s  ratelimit 0  threshold 1\n", wan_ifname, "upstream");
		fprintf(fp, "\taltnet %s\n", altnet_mask);
		fprintf(fp, "\nphyint %s %s  ratelimit 0  threshold 1\n", IFNAME_BR, "downstream");
		fprintf(fp, "\n");
		
		fclose(fp);
	}

	eval("/bin/igmpproxy", igmpproxy_conf);
}

void
restart_iptv(int is_ap_mode)
{
	config_bridge(is_ap_mode);

	if (!is_ap_mode) {
		char *wan_ifname, *viptv_iflast;
		
		/* check used IPTV via VLAN interface */
		viptv_iflast = nvram_safe_get("viptv_ifname");
		if (*viptv_iflast && is_interface_exist(viptv_iflast))
			wan_ifname = viptv_iflast;
		else
			wan_ifname = get_man_ifname(0);
		start_igmpproxy(wan_ifname);
	} else {
		stop_igmpproxy(NULL);
		start_udpxy(IFNAME_BR);
#if defined(APP_XUPNPD)
		start_xupnpd(IFNAME_BR);
#endif
	}
}

void
flush_conntrack_table(char *ip)
{
	if (!ip)
		ip = "f"; // flush all table
	fput_string("/proc/net/nf_conntrack", ip);
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

int
is_ftp_conntrack_loaded(int ftp_port0, int ftp_port1)
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

#if defined (USE_HW_NAT)
static int
is_hwnat_loaded(void)
{
	int result = 0;
	char offload_val[32] = {0};

	if (!is_module_loaded("hw_nat"))
		return result;

	result |= 0x1;

	if (module_param_get("hw_nat", "wifi_offload", offload_val, sizeof(offload_val)) == 0) {
		if (offload_val[0] == '1' || offload_val[0] == 'Y')
			result |= 0x2;
	}

	if (module_param_get("hw_nat", "udp_offload", offload_val, sizeof(offload_val)) == 0) {
		if (offload_val[0] == '1' || offload_val[0] == 'Y')
			result |= 0x4;
	}

#if defined(USE_IPV6_HW_NAT)
	if (module_param_get("hw_nat", "ipv6_offload", offload_val, sizeof(offload_val)) == 0) {
		if (offload_val[0] == '1' || offload_val[0] == 'Y')
			result |= 0x8;
	}
#endif

	return result;
}

static int
is_hwnat_allow_ipv6(void)
{
#if defined(USE_IPV6_HW_NAT)
	int ipv6_type = get_ipv6_type();

#if defined(USE_HW_NAT_V2)
	if (nvram_get_int("ip6_ppe_on") && (ipv6_type != IPV6_DISABLED))
		return 1;
#else
	if (nvram_get_int("ip6_ppe_on") && (ipv6_type == IPV6_NATIVE_STATIC || ipv6_type == IPV6_NATIVE_DHCP6))
		return 1;
#endif
#endif
	return 0;
}

static int
is_hwnat_allow(void)
{
	int result = 0;
	int sw_mode = nvram_get_int("sw_mode");

	int hw_nat_mode = nvram_get_int("hw_nat_mode");

	if (sw_mode != 1 && sw_mode != 4)
		return result;

	if (hw_nat_mode == 2)
		return result;

	result |= 0x1;

	if (hw_nat_mode == 1 || hw_nat_mode == 4)
		result |= 0x2;	// wifi_offload=1

	if (hw_nat_mode == 3 || hw_nat_mode == 4)
		result |= 0x4;	// udp_offload=1

	if (is_hwnat_allow_ipv6())
		result |= 0x8;	// ipv6_offload=1

	return result;
}

static void
hwnat_load(int allow_mask)
{
	char hnat_param[80];
	int wifi_offload, udp_offload, wan_vid;

	wifi_offload = (allow_mask & 0x2) ? 1 : 0;
	udp_offload  = (allow_mask & 0x4) ? 1 : 0;
	wan_vid = get_vlan_vid_wan();

	snprintf(hnat_param, sizeof(hnat_param), "wifi_offload=%d udp_offload=%d wan_vid=%d", wifi_offload, udp_offload, wan_vid);
#if defined(USE_IPV6_HW_NAT)
	if (allow_mask & 0x8)
		strcat(hnat_param, " ipv6_offload=1");
#endif
	module_smart_load("hw_nat", hnat_param);
}

static void
hwnat_configure(int allow_mask)
{
	const char *hwnat_status = "Disabled";

	if (!is_module_loaded("hw_nat")) {
		logmessage(LOGNAME, "%s: %s", "Hardware NAT/Routing", hwnat_status);
		return;
	}

	module_param_set_int("hw_nat", "wan_vid", get_vlan_vid_wan());
	module_param_set_int("hw_nat", "ttl_regen", (nvram_get_int("wan_ttl_fix") == 1) ? 0 : 1);

	if (allow_mask & 0x2)
#if defined(USE_WWAN_HW_NAT)
		hwnat_status = "Enabled, IPoE/PPPoE offload [WAN/WWAN]<->[LAN/WLAN]";
#else
		hwnat_status = "Enabled, IPoE/PPPoE offload [WAN]<->[LAN/WLAN]";
#endif
	else
		hwnat_status = "Enabled, IPoE/PPPoE offload [WAN]<->[LAN]";

	logmessage(LOGNAME, "%s: %s", "Hardware NAT/Routing", hwnat_status);
	logmessage(LOGNAME, "%s: IPv4 UDP flow offload - %s", "Hardware NAT/Routing", (allow_mask & 0x4) ? "ON" : "OFF");
#if defined(USE_IPV6_HW_NAT)
	logmessage(LOGNAME, "%s: IPv6 routes offload - %s", "Hardware NAT/Routing", (allow_mask & 0x8) ? "ON" : "OFF");
#endif
}
#endif /* USE_HW_NAT */

#if 0
static int
is_fastnat_allow(void)
{
	if ( nvram_match("sw_nat_mode", "1") && nvram_match("sw_mode", "1") )
		return 1;

	return 0;
}
#endif

void
hw_vlan_tx_map(int idx, int vid)
{
	char vlan_tx_data[16];

	/* use slots 6..10 for custom VID
	   NOTE: slots 11..15 used by hw_nat WiFi/USB offload */
	if (vid < 6 || idx < 6 || idx > 10)
		return;

	/* map VLAN VID to raeth (for support RT3883/MT7620 HW_VLAN_TX with VID > 15) */
	snprintf(vlan_tx_data, sizeof(vlan_tx_data), "%d: %d", idx, vid);

#if defined (CONFIG_RALINK_RT3883)
	fput_string("/proc/rt3883/vlan_tx", vlan_tx_data);
#elif defined (CONFIG_RALINK_MT7620)
	fput_string("/proc/mt7620/vlan_tx", vlan_tx_data);
#endif
}

void
reload_nat_modules(void)
{
	int loaded_ftp;
	int needed_ftp0 = 0;
	int needed_ftp1 = 0;
	int needed_sip = 0;
	int needed_h323 = 0;
	int needed_rtsp = 0;
	int needed_pptp = 0;
	int wan_nat_x = nvram_get_int("wan_nat_x");
#if defined (USE_HW_NAT)
	int hwnat_allow = is_hwnat_allow();
	int hwnat_loaded = is_hwnat_loaded();
#endif

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

#if defined (USE_HW_NAT)
	if ((hwnat_loaded) && ((!hwnat_allow) || (hwnat_loaded != hwnat_allow)))
	{
		hwnat_loaded = 0;
		module_smart_unload("hw_nat", 0);
		if (hwnat_allow)
			sleep(1);
	}
#endif

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

#if defined (USE_HW_NAT)
	if (hwnat_allow && !hwnat_loaded)
		hwnat_load(hwnat_allow);

	hwnat_configure(hwnat_allow);
#endif
#if defined (USE_SFE)
	int sfe_enable = nvram_get_int("sfe_enable");
	int sfe_loaded = is_module_loaded("fast_classifier");

	if (sfe_loaded && !sfe_enable) {
		module_smart_unload("fast_classifier", 1);
		doSystem("echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_be_liberal");
		doSystem("echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_no_window_check");
		sfe_loaded = 0;
	}
	if (sfe_enable && !sfe_loaded) {
		doSystem("echo 0 > /proc/sys/net/netfilter/nf_conntrack_tcp_be_liberal");
		doSystem("echo 0 > /proc/sys/net/netfilter/nf_conntrack_tcp_no_window_check");
		module_smart_load("fast_classifier", NULL);
		sfe_loaded = 1;
	}
	if (sfe_loaded) {
		if (sfe_enable == 1)
			doSystem("echo 0 > /sys/fast_classifier/skip_to_bridge_ingress");
		else if (sfe_enable == 2)
			doSystem("echo 1 > /sys/fast_classifier/skip_to_bridge_ingress");
	}
#endif
}

void
restart_firewall(void)
{
	if (get_ap_mode())
		return;

	/* re-build iptables rules */
	start_firewall_ex();

	/* update UPnP forwards from lease file */
	update_upnp();
}

void
set_ipv4_forward(int is_on)
{
	char tmp[64];

	/* enable/disable forwarding IPv4 */
	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "ip_forward");
	fput_int(tmp, (is_on) ? 1 : 0);
}

void
set_nf_conntrack(void)
{
	int i_nf_nat, i_nf_val;

#if (BOARD_RAM_SIZE < 32)
	int i_nf_lim = 4096;
#elif (BOARD_RAM_SIZE < 64)
	int i_nf_lim = 16384;
#elif (BOARD_RAM_SIZE < 128)
	int i_nf_lim = 65536;
#elif (BOARD_RAM_SIZE < 256)
	int i_nf_lim = 262144;
#else
	int i_nf_lim = 327680;
#endif

	i_nf_val = nvram_get_int("nf_nat_type");
	if (i_nf_val == 2)
		i_nf_nat = 0;	// Linux
	else if (i_nf_val == 1)
		i_nf_nat = 1;	// FCONE
	else
		i_nf_nat = 2;	// RCONE
	fput_int("/proc/sys/net/netfilter/nf_conntrack_nat_mode", i_nf_nat);

	i_nf_val = nvram_safe_get_int("nf_max_conn", 16384, 4096, i_nf_lim);
	fput_int("/proc/sys/net/nf_conntrack_max", i_nf_val);
}

void
set_tcp_syncookies(void)
{
	char tmp[64];
	int fw_syn_cook = nvram_get_int("fw_syn_cook");

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_syncookies");
	fput_int(tmp, (fw_syn_cook) ? 1 : 0);

	/* Tweak DoS-related... */
	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "icmp_ignore_bogus_error_responses");
	fput_int(tmp, 1);

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "icmp_echo_ignore_broadcasts");
	fput_int(tmp, 1);
}

void
set_tcp_tweaks(void)
{
	char tmp[64];

	/* Tweak TCP IPv4 performance */
	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_fin_timeout");
	fput_int(tmp, 40);		// def: 60

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_keepalive_intvl");
	fput_int(tmp, 30);		// def: 75

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_keepalive_probes");
	fput_int(tmp, 5);		// def: 9

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_keepalive_time");
	fput_int(tmp, 1800);		// def: 7200

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_retries2");
	fput_int(tmp, 5);		// def: 15

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_syn_retries");
	fput_int(tmp, 3);		// def: 5

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_synack_retries");
	fput_int(tmp, 3);		// def: 5

//	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_tw_recycle");
//	fput_int(tmp, 1);

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_tw_reuse");
	fput_int(tmp, 1);

	sprintf(tmp, "/proc/sys/net/%s/%s", "ipv4", "tcp_rfc1337");
	fput_int(tmp, 1);
}

void
set_passthrough_pppoe(int is_on)
{
	char pthrough[32], *lan_if, *wan_if;

	lan_if = "null";
	wan_if = lan_if;

	if (is_on && nvram_match("fw_pt_pppoe", "1")) {
		lan_if = IFNAME_BR;
		wan_if = get_man_ifname(0);
	}

	snprintf(pthrough, sizeof(pthrough), "%s,%s\n", lan_if, wan_if);

	/* enable/disable kernel-mode PPPoE passthrough */
	fput_string("/proc/net/pthrough/pppoe", pthrough);
}

void
set_igmp_mld_version(void)
{
	int force_value;
	const char *ifname = get_man_ifname(0);

	force_value = nvram_safe_get_int("force_igmp", 2, 0, 2);

	set_interface_conf_int("ipv4", "all", "force_igmp_version", force_value);
	set_interface_conf_int("ipv4", ifname, "force_igmp_version", force_value);
	set_interface_conf_int("ipv4", IFNAME_BR, "force_igmp_version", force_value);

#if defined (USE_IPV6)
	force_value = nvram_safe_get_int("force_mld", 0, 0, 1);

	set_interface_conf_int("ipv6", "all", "force_mld_version", force_value);
	set_interface_conf_int("ipv6", ifname, "force_mld_version", force_value);
	set_interface_conf_int("ipv6", IFNAME_BR, "force_mld_version", force_value);
#endif
}

void
set_libc_gai(int ipv4_first)
{
	if (ipv4_first) {
		FILE *fp = fopen("/etc/gai.conf", "w");
		if (fp) {
			fprintf(fp, "precedence %s\n", "ipv4");
			fclose(fp);
		}
	} else {
		unlink("/etc/gai.conf");
	}
}

