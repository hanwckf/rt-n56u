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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>

#include "rc.h"

#define foreach_x(x)		for (i=0; i<nvram_get_int(x); i++)

#define BATTLENET_PORT		6112
#define TRANSMISSION_PPORT	51413
#define TRANSMISSION_RPORT	9091
#define ARIA_PPORT		16888
#define ARIA_RPORT		6800

#define MODULE_WEBSTR_MASK	0x01

/* state match - use xt_state (xt_conntrack more slower than xt_state) */
#define CT_STATE		"state --state"

static char *g_buf;
static char g_buf_pool[1024];

static void
g_buf_init()
{
	g_buf = g_buf_pool;
	g_buf_pool[0] = 0;
}

static char *
g_buf_alloc(char *g_buf_now)
{
	g_buf += strlen(g_buf_now)+1;

	return (g_buf_now);
}

static char *
proto_conv(const char *proto_name, int idx)
{
	char *proto;
	char itemname_arr[32];

	snprintf(itemname_arr, sizeof(itemname_arr), "%s%d", proto_name, idx);
	proto = nvram_safe_get(itemname_arr);

	if (!strncasecmp(proto, "Both", 4))
		strcpy(g_buf, "both");
	else if (!strncasecmp(proto, "TCP", 3))
		strcpy(g_buf, "tcp");
	else if (!strncasecmp(proto, "UDP", 3))
		strcpy(g_buf, "udp");
	else if (!strncasecmp(proto, "OTHER", 5))
		strcpy(g_buf, "other");
	else
		strcpy(g_buf, "tcp");

	return (g_buf_alloc(g_buf));
}

static char *
protoflag_conv(const char *proto_name, int idx, int isFlag)
{
	char *proto;
	char itemname_arr[32];

	snprintf(itemname_arr, sizeof(itemname_arr), "%s%d", proto_name, idx);
	proto = nvram_safe_get(itemname_arr);

	strcpy(g_buf, "");

	if (!isFlag) {
		if (!strncasecmp(proto, "UDP", 3))
			strcpy(g_buf, "udp");
		else if (!strncasecmp(proto, "OTHER", 5))
			strcpy(g_buf, "other");
		else
			strcpy(g_buf, "tcp");
	} else {
		if (strlen(proto)>3 && !strncasecmp(proto, "TCP", 3))
			strcpy(g_buf, proto+4);
	}

	return (g_buf_alloc(g_buf));
}

static char *
portrange_conv(const char *port_name, int idx)
{
	char itemname_arr[32];

	snprintf(itemname_arr, sizeof(itemname_arr), "%s%d", port_name, idx);
	strcpy(g_buf, nvram_safe_get(itemname_arr));

	return (g_buf_alloc(g_buf));
}

static char *
ip_conv(const char *ip_name, int idx)
{
	char itemname_arr[32];

	snprintf(itemname_arr, sizeof(itemname_arr), "%s%d", ip_name, idx);
	strcpy(g_buf, nvram_safe_get(itemname_arr));

	return (g_buf_alloc(g_buf));
}

static char *
general_conv(const char *ip_name, int idx)
{
	char itemname_arr[32];

	snprintf(itemname_arr, sizeof(itemname_arr), "%s%d", ip_name, idx);
	strcpy(g_buf, nvram_safe_get(itemname_arr));

	return (g_buf_alloc(g_buf));
}

static char *
filter_conv(char *proto, char *flag, char *srcip, char *srcport, char *dstip, char *dstport)
{
	char newstr[128];

	strcpy(g_buf, "");

	if (strcmp(proto, "") != 0) {
		snprintf(newstr, sizeof(newstr), " -p %s", proto);
		strcat(g_buf, newstr);
	}

	if (strcmp(flag, "") != 0) {
		snprintf(newstr, sizeof(newstr), " --tcp-flags %s %s", flag, flag);
		strcat(g_buf, newstr);
	}

	if (strcmp(srcip, "") != 0) {
		if (strchr(srcip , '-'))
			snprintf(newstr, sizeof(newstr), " --src-range %s", srcip);
		else
			snprintf(newstr, sizeof(newstr), " -s %s", srcip);
		strcat(g_buf, newstr);
	}

	if (strcmp(srcport, "") != 0) {
		snprintf(newstr, sizeof(newstr), " --sport %s", srcport);
		strcat(g_buf, newstr);
	}

	if (strcmp(dstip, "") != 0) {
		if (strchr(dstip, '-'))
			snprintf(newstr, sizeof(newstr), " --dst-range %s", dstip);
		else
			snprintf(newstr, sizeof(newstr), " -d %s", dstip);
		strcat(g_buf, newstr);
	}

	if (strcmp(dstport, "") != 0) {
		snprintf(newstr, sizeof(newstr), " --dport %s", dstport);
		strcat(g_buf, newstr);
	}

	return (g_buf_alloc(g_buf));
}

static void
timematch_conv(char *mstr, const char *nv_date, const char *nv_time)
{
	const char *datestr[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char time_s[8], time_e[8], *time, *date;
	int i, i_time_s, i_time_e, i_full_time, comma = 0;

	date = nvram_get(nv_date);
	if (!date)
		date = "1111111";

	time = nvram_get(nv_time);
	if (!time)
		time = "00002359";

	mstr[0] = 0;

	if (strlen(date) != 7 || strlen(time) != 8)
		return;

	if (!strcmp(date, "0000000"))
		return;

	strncpy(time_s, time+0, 4);
	strncpy(time_e, time+4, 4);

	time_s[4] = 0;
	time_e[4] = 0;

	i_time_s = atoi(time_s);
	i_time_e = atoi(time_e);

	i_full_time = ((i_time_s == i_time_e) || (i_time_s == 0 && i_time_e == 2359)) ? 1 : 0;

	/* check anytime */
	if (!strcmp(date, "1111111") && i_full_time)
		return;

	/* check whole day */
	if (i_full_time) {
		sprintf(mstr, " -m time %s", "--kerneltz");
	} else {
		const char *contiguous = "";
		
		/* check cross-night */
		if (i_time_s > i_time_e)
			contiguous = " --contiguous";
		
		sprintf(mstr, " -m time --timestart %c%c:%c%c:00 --timestop %c%c:%c%c:00%s %s",
			time[0], time[1], time[2], time[3], time[4], time[5], time[6], time[7],
			contiguous, "--kerneltz");
	}

	/* check everyday */
	if (strcmp(date, "1111111")) {
		strcat(mstr, " --weekdays ");
		for (i=0; i<7; i++) {
			if (date[i] == '1') {
				if (comma)
					strcat(mstr, ",");
				else
					comma = 1;
				
				strcat(mstr, datestr[i]);
			}
		}
	}
}

static char *
iprange_ex_conv(const char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32];
	char startip[16], endip[16];
	int i, j, k;
	int mask;

	snprintf(itemname_arr, sizeof(itemname_arr), "%s%d", ip_name, idx);
	ip = nvram_safe_get(itemname_arr);
	strcpy(g_buf, "");

	// scan all ip string
	i=j=k=0;
	mask=32;

	while (*(ip+i)) {
		if (*(ip+i)=='*') {
			startip[j++] = '0';
			endip[k++] = '0';
			// 255 is for broadcast
			mask-=8;
		} else {
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		++i;
	}

	startip[j++] = 0;
	endip[k++] = 0;

	if (mask==32)
		sprintf(g_buf, "%s", startip);
	else if (mask==0)
		strcpy(g_buf, "");
	else
		sprintf(g_buf, "%s/%d", startip, mask);

	return (g_buf_alloc(g_buf));
}

#if 0
static int
is_valid_filter_date(const char *nv_date)
{
	char *date = nvram_get(nv_date);
	if (!date)
		date = "1111111";

	if (strlen(date) != 7 || !strcmp(date, "0000000"))
		return 0;

	return 1;
}

static int
is_valid_filter_time(const char *nv_time)
{
	char time_s[8], time_e[8], *time;

	time = nvram_get(nv_time);
	if (!time)
		time = "00002359";

	if (strlen(time) != 8)
		return 0;

	strncpy(time_s, time+0, 4);
	strncpy(time_e, time+4, 4);

	time_s[4] = 0;
	time_e[4] = 0;

	if (atoi(time_s) == atoi(time_e))
		return 0;

	return 1;
}
#endif

void
ip2class(const char *addr, const char *mask, char *out_buf, size_t out_len)
{
	unsigned int val, ip;
	struct in_addr in;
	int i=0;

	// only handle class A,B,C
	val = (unsigned int)inet_addr(mask);
	ip = (unsigned int)inet_addr(addr);

	// oleg patch ~
	in.s_addr = ip & val;

	for (val = ntohl(val); val; i++)
		val <<= 1;

	snprintf(out_buf, out_len, "%s/%d", inet_ntoa(in), i);
}

// WAN, MAN, LAN
void
fill_static_routes(char *buf, int len, const char *ift)
{
	int i, len_iter;
	char buf_iter[128];
	char *ip, *netmask, *gateway, *metric, *interface;

	if (!buf || len < 32)
		return;

	buf[0] = '\0';

	foreach_x("sr_num_x") {
		g_buf_init();
		
		ip = general_conv("sr_ipaddr_x", i);
		netmask = general_conv("sr_netmask_x", i);
		gateway = general_conv("sr_gateway_x", i);
		metric = general_conv("sr_matric_x", i);
		interface = general_conv("sr_if_x", i);
		
		if (strcmp(interface, ift) == 0)
		{
			snprintf(buf_iter, sizeof(buf_iter), "%s:%s:%s:%d ", ip, netmask, gateway, atoi(metric));
			len_iter = strlen(buf_iter);
			if (len < (len_iter + 1))
				break;
			
			strcat(buf, buf_iter);
			
			len -= len_iter;
		}
	}

	/* remove last space */
	len_iter = strlen(buf);
	if (len_iter > 0)
		buf[len_iter-1] = '\0';
}

static void
include_vpns_clients(FILE *fp)
{
	FILE *fpls;
	char ifname[64] = {0};
	char *dtype = IPT_CHAIN_NAME_VPN_LIST;

	fpls = fopen(VPN_SERVER_LEASE_FILE, "r");
	if (fpls) {
		while(fscanf(fpls, "%63s %*s %*s %*s\n", ifname) > 0) {
			if (ppp_ifindex(ifname) >= VPN_SERVER_PPP_UNIT)
				fprintf(fp, "-A %s -i %s -j %s\n", dtype, ifname, "ACCEPT");
		}
		
		fclose(fpls);
	}
}

static int
include_mac_filter(FILE *fp, int mac_filter_mode, char *logdrop)
{
	int i, mac_num;
	char mac_timematch[128], mac_buf[24], nv_date[32], nv_time[32];
	char *filter_mac, *ftype;
	const char *dtype = IPT_CHAIN_NAME_MAC_LIST;

	if (mac_filter_mode > 0) {
		if (mac_filter_mode == 2)
			ftype = logdrop;
		else
			ftype = "RETURN";
		
		mac_num = 0;
		foreach_x("macfilter_num_x") {
			g_buf_init();
			
			filter_mac = mac_conv("macfilter_list_x", i, mac_buf);
			if (*filter_mac) {
				mac_num++;
				sprintf(nv_date, "macfilter_date_x%d", i);
				sprintf(nv_time, "macfilter_time_x%d", i);
				timematch_conv(mac_timematch, nv_date, nv_time);
				fprintf(fp, "-A %s -m mac --mac-source %s%s -j %s\n", dtype, filter_mac, mac_timematch, ftype);
			}
		}
		
		if (mac_filter_mode != 2 && mac_num > 0)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
		
		if (mac_num < 1)
			mac_filter_mode = 0;
	}

	return mac_filter_mode;
}

static int
include_webstr_filter(FILE *fp)
{
	int i, webstr_items, url_length, url_total;
	char url_list[256], nv_name[32], *filterstr;
	const char *dtype = IPT_CHAIN_NAME_URL_LIST;
	const char *split = "<&nbsp;>";

	/* webstr support max 256 bytes splitted URL */

	url_total = 0;
	url_list[0] = 0;

	webstr_items = 0;

	foreach_x("url_num_x") {
		sprintf(nv_name, "url_keyword_x%d", i);
		filterstr = nvram_safe_get(nv_name);
		if (strncasecmp(filterstr, "http://", 7) == 0)
			filterstr += 7;
		else if (strncasecmp(filterstr, "https://", 8) == 0)
			filterstr += 8;
		
		url_length = strlen(filterstr);
		if (url_length < 1 || url_length >= sizeof(url_list))
			continue;
		
		if (url_total > 0)
			url_length += strlen(split);
		
		if (url_total + url_length < sizeof(url_list)) {
			if (url_total > 0)
				strcat(url_list, split);
		} else {
			url_length = strlen(filterstr);
			
			/* flush merged url */
			fprintf(fp, "-A %s -p tcp -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n",
				dtype, url_list);
			webstr_items++;
			
			url_total = 0;
			url_list[0] = 0;
		}
		strcat(url_list, filterstr);
		url_total += url_length;
	}

	if (url_total) {
		fprintf(fp, "-A %s -p tcp -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n",
			dtype, url_list);
		webstr_items++;
	}

	return webstr_items;
}

static int
include_lw_filter(FILE *fp, char *wan_if, char *logaccept, char *logdrop)
{
	int i, lw_need_drop, lw_items;
	char icmp_ptr[64], *icmplist, *filterstr, *ftype;
	char *proto, *flag, *srcip, *srcport, *dstip, *dstport;
	const char *dtype = IPT_CHAIN_NAME_LWF_LIST;

	if (nvram_match("filter_lw_default_x", "DROP")) {
		ftype = logaccept;
		lw_need_drop = 1;
	} else {
		ftype = logdrop;
		lw_need_drop = 0;
	}

	lw_items = 0;

	foreach_x("filter_lw_num_x") {
		g_buf_init();
		
		proto = protoflag_conv("filter_lw_proto_x", i, 0);
		if (strcmp(proto, "other") == 0) {
			proto = portrange_conv("filter_lw_protono_x", i);
			flag = "";
			srcport = "";
			dstport = "";
		} else {
			flag = protoflag_conv("filter_lw_proto_x", i, 1);
			srcport = portrange_conv("filter_lw_srcport_x", i);
			dstport = portrange_conv("filter_lw_dstport_x", i);
		}
		
		srcip = iprange_ex_conv("filter_lw_srcip_x", i);
		dstip = iprange_ex_conv("filter_lw_dstip_x", i);
		
		filterstr = filter_conv(proto, flag, srcip, srcport, dstip, dstport);
		if (*filterstr) {
			fprintf(fp, "-A %s %s -j %s\n", dtype, filterstr, ftype);
			lw_items++;
		}
	}

	icmp_ptr[0] = 0;
	foreach(icmp_ptr, nvram_safe_get("filter_lw_icmp_x"), icmplist) {
		if (*icmp_ptr) {
			fprintf(fp, "-A %s -o %s -p icmp --icmp-type %s -j %s\n",
				dtype, wan_if, icmp_ptr, ftype);
			lw_items++;
		}
	}

	if (lw_need_drop)
		fprintf(fp, "-A %s -j %s\n", dtype, logdrop);

	return (lw_items | lw_need_drop);
}

static void
include_vts_filter(FILE *fp, char *lan_ip, char *logaccept, int forward_chain)
{
	int i;
	char *proto, *protono, *port, *lport, *srcip, *dstip, *dtype;
	char srcaddrs[64], dstports[32];

	dtype = (forward_chain) ? "FORWARD" : "INPUT";

	foreach_x("vts_num_x")
	{
		g_buf_init();
		srcaddrs[0] = 0;
		
		dstip = ip_conv("vts_ipaddr_x", i);
		if (!is_valid_ipv4(dstip))
			continue;
		
		if (forward_chain) {
			if (strcmp(lan_ip, dstip) == 0)
				continue;
		} else {
			if (strcmp(lan_ip, dstip) != 0)
				continue;
		}
		
		srcip = iprange_ex_conv("vts_srcip_x", i);
		if (*srcip)
			snprintf(srcaddrs, sizeof(srcaddrs), " -s %s", srcip);
		
		proto = proto_conv("vts_proto_x", i);
		port = portrange_conv("vts_port_x", i);
		lport = portrange_conv("vts_lport_x", i);
		
		if (lport && strlen(lport)!=0)
			snprintf(dstports, sizeof(dstports), "%s", lport);
		else
			snprintf(dstports, sizeof(dstports), "%s", port);
		
		if (*dstports) {
			if (strcmp(proto, "tcp")==0 || strcmp(proto, "both")==0)
				fprintf(fp, "-A %s -p %s%s -d %s --dport %s -j %s\n", dtype, "tcp", srcaddrs, dstip, dstports, logaccept);
			
			if (strcmp(proto, "udp")==0 || strcmp(proto, "both")==0)
				fprintf(fp, "-A %s -p %s%s -d %s --dport %s -j %s\n", dtype, "udp", srcaddrs, dstip, dstports, logaccept);
		}
		
		if (strcmp(proto, "other")==0) {
			protono = portrange_conv("vts_protono_x", i);
			if (*protono)
				fprintf(fp, "-A %s -p %s%s -d %s -j %s\n", dtype, protono, srcaddrs, dstip, logaccept);
		}
	}
}

static void
include_vts_nat(FILE *fp)
{
	int i;
	char *proto, *protono, *port, *lport, *srcip, *dstip, *dtype;
	char srcaddrs[64];

	dtype = IPT_CHAIN_NAME_VSERVER;

	foreach_x("vts_num_x")
	{
		g_buf_init();
		srcaddrs[0] = 0;
		
		dstip = ip_conv("vts_ipaddr_x", i);
		if (!is_valid_ipv4(dstip))
			continue;
		
		srcip = iprange_ex_conv("vts_srcip_x", i);
		if (*srcip)
			snprintf(srcaddrs, sizeof(srcaddrs), " -s %s", srcip);
		
		proto = proto_conv("vts_proto_x", i);
		port = portrange_conv("vts_port_x", i);
		lport = portrange_conv("vts_lport_x", i);
		
		if (strcmp(proto, "tcp")==0 || strcmp(proto, "both") == 0) {
			if (lport && strlen(lport)!=0 && strcmp(port, lport)!=0)
				fprintf(fp, "-A %s -p %s%s --dport %s -j DNAT --to-destination %s:%s\n", dtype, "tcp", srcaddrs, port, dstip, lport);
			else
				fprintf(fp, "-A %s -p %s%s --dport %s -j DNAT --to %s\n", dtype, "tcp", srcaddrs, port, dstip);
		}
		
		if (strcmp(proto, "udp")==0 || strcmp(proto, "both") == 0) {
			if (lport && strlen(lport)!=0 && strcmp(port, lport)!=0)
				fprintf(fp, "-A %s -p %s%s --dport %s -j DNAT --to-destination %s:%s\n", dtype, "udp", srcaddrs, port, dstip, lport);
			else
				fprintf(fp, "-A %s -p %s%s --dport %s -j DNAT --to %s\n", dtype, "udp", srcaddrs, port, dstip);
		}
		
		if (strcmp(proto, "other") == 0) {
			protono = portrange_conv("vts_protono_x", i);
			if (*protono)
				fprintf(fp, "-A %s -p %s%s -j DNAT --to %s\n", dtype, protono, srcaddrs, dstip);
		}
	}
}

static void
include_masquerade(FILE *fp, char *wan_if, char *wan_ip, char *lan_net)
{
	char *dtype = "POSTROUTING";

	if (wan_ip)
		fprintf(fp, "-A %s -o %s -s %s -j SNAT --to-source %s\n", dtype, wan_if, lan_net, wan_ip);
	else
		fprintf(fp, "-A %s -o %s -s %s -j MASQUERADE\n", dtype, wan_if, lan_net);
}

static int
is_need_tcp_mss_wan(int unit, int wan_proto, char *man_if)
{
	if (get_usb_modem_wan(unit) ) {
		int modem_mtu = nvram_safe_get_int("modem_mtu", 1500, 1000, 1500);
		if (modem_mtu != 1500)
			return 1;
	} else {
		if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
		    wan_proto == IPV4_WAN_PROTO_PPTP ||
		    wan_proto == IPV4_WAN_PROTO_L2TP)
			return 1;
		
		if (get_interface_mtu(man_if) != 1500)
			return 1;
	}

	return 0;
}

static char *
get_tcp_mss_ifname_vpns(int vpns_type)
{
#if defined (APP_OPENVPN)
	/* NOTE: Not use TCPMSS target for OpenVPN tunX interface:
	   1) do not use --tun-mtu or --link-mtu (this is changed MTU on tunX interface)
	   2) please use --fragment or/and --mssfix
	*/
	if (vpns_type == 2)
		return NULL;
	else
#endif
		return "ppp+";
}

static char *
get_tcp_mss_ifname_vpnc(int vpnc_type)
{
#if defined (APP_OPENVPN)
	/* NOTE: Not use TCPMSS target for OpenVPN tunX interface:
	   1) do not use --tun-mtu or --link-mtu (this is changed MTU on tunX interface)
	   2) please use --fragment or/and --mssfix
	*/
	if (vpnc_type == 2)
		return NULL;
	else
#endif
		return IFNAME_CLIENT_PPP;
}

static int
get_sshd_bfp_time(int bfp_mode)
{
	int i_bfp_time = 3600;		// 60 min

	if (bfp_mode == 1)
		i_bfp_time = 60;	// 1 min
	else if (bfp_mode == 2)
		i_bfp_time = 300;	// 5 min
	else if (bfp_mode == 3)
		i_bfp_time = 600;	// 10 min
	else if (bfp_mode == 4)
		i_bfp_time = 1800;	// 30 min

	return i_bfp_time;
}

static int
ipt_filter_rules(char *man_if, char *wan_if, char *lan_if, char *lan_ip,
                 char *logaccept, char *logdrop, int tcp_mss_need)
{
	FILE *fp;
	char timematch[160], *ftype, *dtype, *vpnc_if;
	const char *ipt_file = "/tmp/ipt_filter.rules";
	int ret, wport, lport;
	int is_nat_enabled, is_dos_enabled, is_fw_enabled, is_logaccept, is_logdrop;
	int is_url_enabled, is_lwf_enabled;
	int i_vpns_enable, i_vpns_type, i_vpns_actl, i_http_proto, i_bfplimit_ref;
	int i_vpnc_enable, i_vpnc_type, i_vpnc_sfw, i_mac_filter;
#if defined (APP_OPENVPN)
	int i_vpns_ov_mode = nvram_get_int("vpns_ov_mode");
#endif
#if defined (USE_IPV6)
	int ipv6_type = get_ipv6_type();
#endif

	ret = 0;
	i_bfplimit_ref = 0;

	is_logaccept = (strcmp(logaccept, IPT_CHAIN_NAME_LOG_ACCEPT) == 0) ? 1 : 0;
	is_logdrop = (strcmp(logdrop, IPT_CHAIN_NAME_LOG_DROP) == 0) ? 1 : 0;

	is_nat_enabled = nvram_match("wan_nat_x", "1");
	is_dos_enabled = nvram_match("fw_dos_x", "1");
	is_fw_enabled  = nvram_match("fw_enable_x", "1");
	is_url_enabled = nvram_match("url_enable_x", "1");
	is_lwf_enabled = nvram_match("fw_lw_enable_x", "1");

	i_mac_filter   = nvram_get_int("macfilter_enable_x");

	i_vpns_enable  = nvram_get_int("vpns_enable");
	i_vpns_type    = nvram_get_int("vpns_type");
	i_vpns_actl    = nvram_get_int("vpns_actl");

	i_vpnc_enable  = nvram_get_int("vpnc_enable");
	i_vpnc_type    = nvram_get_int("vpnc_type");
	i_vpnc_sfw     = nvram_get_int("vpnc_sfw");

	vpnc_if = NULL;
	if (i_vpnc_enable) {
#if defined (APP_OPENVPN)
		if (i_vpnc_type == 2) {
			if (nvram_get_int("vpnc_ov_mode") == 1)
				vpnc_if = IFNAME_CLIENT_TUN;
			else if (nvram_get_int("vpnc_ov_cnat") == 1)
				vpnc_if = IFNAME_CLIENT_TAP;
		} else
#endif
			vpnc_if = IFNAME_CLIENT_PPP;
	}

	if (!(fp=fopen(ipt_file, "w")))
		return 0;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" :  "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_FORWARD);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VPN_LIST);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_BFP_LIMIT);
	if (i_mac_filter > 0)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_MAC_LIST);
	if (is_url_enabled)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_URL_LIST);
	if (is_lwf_enabled)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LWF_LIST);
	if (is_dos_enabled)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_DOS_LIMIT);
	if (is_logaccept)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_ACCEPT);
	if (is_logdrop)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_DROP);

	// maclist chain
	i_mac_filter = include_mac_filter(fp, i_mac_filter, logdrop);

	// INPUT chain
	dtype = "INPUT";

	/* Policy for all traffic from MAC-filtered LAN clients */
	if (i_mac_filter > 0 && nvram_match("fw_mac_drop", "1"))
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Accept all traffic from LAN clients */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Accept all traffic from localhost */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", "ACCEPT");
		
#if defined (USE_IPV6)
		/* Accept SIT (IPv6-in-IPv4) packets (might be seen as INVALID) */
		if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
			fprintf(fp, "-A %s -p %d -j %s\n", dtype, 41, "ACCEPT");
#endif
		/* Drop the wrong state, INVALID, packets */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Accept multicast (all, except udp port 1900) */
		if (nvram_match("mr_enable_x", "1") || nvram_invmatch("udpxy_enable_x", "0")
#if defined (APP_XUPNPD)
		 || nvram_invmatch("xupnpd_enable_x", "0")
#endif
		) {
			fprintf(fp, "-A %s -p 2 -d 224.0.0.0/4 -j %s\n", dtype, logaccept);
			fprintf(fp, "-A %s -p udp -d 224.0.0.0/4 ! --dport %d -j %s\n", dtype, 1900, "ACCEPT");
		}
	}

	/* DoS attack limits */
	if (is_dos_enabled)
		fprintf(fp, "-A %s -i %s -m %s %s -j %s\n", dtype, wan_if, CT_STATE, "NEW", IPT_CHAIN_NAME_DOS_LIMIT);

	if (is_fw_enabled) {
		/* Block or Accept inbound traffic from VPN client */
		if (vpnc_if) {
			if (i_vpnc_sfw == 1)
				fprintf(fp, "-A %s -i %s ! -p icmp -j %s\n", dtype, vpnc_if, "DROP");
			else if (i_vpnc_sfw == 0 || i_vpnc_sfw == 2)
				fprintf(fp, "-A %s -i %s -j %s\n", dtype, vpnc_if, "ACCEPT");
		}
		
		/* Accept DHCPv4 */
		fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 67, 68, logaccept);
		
#if defined (SUPPORT_HTTPS)
		i_http_proto = nvram_get_int("http_proto");
#else
		i_http_proto = 0;
#endif
		if ((i_http_proto == 0 || i_http_proto == 2) && nvram_match("misc_http_x", "1")) {
			wport = nvram_get_int("misc_httpport_x");
			lport = nvram_get_int("http_lanport");
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#if defined (SUPPORT_HTTPS)
		if ((i_http_proto == 1 || i_http_proto == 2) && nvram_match("https_wopen", "1")) {
			wport = nvram_get_int("https_wport");
			lport = nvram_get_int("https_lport");
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#endif
#if defined (APP_SSHD)
		if (nvram_invmatch("sshd_enable", "0") && nvram_match("sshd_wopen", "1")) {
			i_bfplimit_ref++;
			wport = nvram_get_int("sshd_wport");
			lport = 22;
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, IPT_CHAIN_NAME_BFP_LIMIT);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, IPT_CHAIN_NAME_BFP_LIMIT);
		}
#endif
#if defined (APP_FTPD)
		if (nvram_invmatch("enable_ftp", "0") && nvram_match("ftpd_wopen", "1")) {
			wport = nvram_get_int("ftpd_wport");
			lport = 21;
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#endif
		lport = nvram_get_int("udpxy_enable_x");
		if (lport > 1023 && nvram_match("udpxy_wopen", "1")) {
			wport = nvram_get_int("udpxy_wport");
			if (wport == lport || !is_nat_enabled)
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
			else
				fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, lport, logaccept);
		}
#if defined (APP_TRMD)
		if (nvram_match("trmd_enable", "1") && is_torrent_support()) {
			wport = nvram_get_int("trmd_pport");
			if (wport < 1024 || wport > 65535) wport = TRANSMISSION_PPORT;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (nvram_match("trmd_ropen", "1")) {
				wport = nvram_get_int("trmd_rport");
				if (wport < 1024 || wport > 65535) wport = TRANSMISSION_RPORT;
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			}
		}
#endif
#if defined (APP_ARIA)
		if (nvram_match("aria_enable", "1") && is_aria_support()) {
			wport = nvram_get_int("aria_pport");
			if (wport < 1024 || wport > 65535) wport = ARIA_PPORT;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (nvram_match("aria_ropen", "1")) {
				wport = nvram_get_int("aria_rport");
				if (wport < 1024 || wport > 65535) wport = ARIA_RPORT;
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			}
		}
#endif
		if (i_vpns_enable) {
			int i_need_vpnlist = 1;
#if defined (APP_OPENVPN)
			if (i_vpns_type == 2) {
				const char *ov_prot = "udp";
				int i_ov_prot = nvram_get_int("vpns_ov_prot");
				int i_ov_port = nvram_safe_get_int("vpns_ov_port", 1194, 1, 65535);
				
				if (i_vpns_ov_mode == 0)
					i_need_vpnlist = 0;
				if (i_ov_prot == 1 || i_ov_prot == 3 || i_ov_prot == 5)
					ov_prot = "tcp";
				if (i_ov_prot == 0 || i_ov_prot == 1 || i_ov_prot == 4 || i_ov_prot == 5)
					fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, ov_prot, i_ov_port, logaccept);
			} else
#endif
			if (i_vpns_type == 1) {
				/* accept L2TP */
				fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, "udp", 1701, logaccept);
			} else {
				/* accept PPTP */
				fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, "tcp", 1723, logaccept);
				fprintf(fp, "-A %s -p %d -j %s\n", dtype, 47, logaccept);
			}
			
			/* Jump to vpnlist chain */
			if (i_need_vpnlist) {
				if (i_vpns_actl == 0 || i_vpns_actl == 2) {
					/* accept all packets */
					fprintf(fp, "-A %s -j %s\n", dtype, IPT_CHAIN_NAME_VPN_LIST);
				} else {
					/* accept DNS UDP packets only */
					fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, "udp", 53, IPT_CHAIN_NAME_VPN_LIST);
				}
			}
		}
		
		/* add Virtual Server rules for router host */
		if (is_nat_enabled && nvram_match("vts_enable_x", "1"))
			include_vts_filter(fp, lan_ip, logaccept, 0);
		
		/* Accept ICMPv4 packets */
		if (nvram_invmatch("misc_ping_x", "0")) {
			/* Accept all ICMP and UDP ports 33434-33534 for traceroute */
			fprintf(fp, "-A %s -p icmp -j %s\n", dtype, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d:%d -j %s\n", dtype, 33434, 33534, logaccept);
		} else {
			/* Accept all ICMP, exclude ping */
			fprintf(fp, "-A %s -p icmp ! --icmp-type %s -j %s\n", dtype, "echo-request", logaccept);
#if defined (USE_IPV6)
			if (ipv6_type == IPV6_6IN4) {
				/* pass ICMP ping from 6in4 remote endpoint */
				char *tun_remote = nvram_safe_get("ip6_6in4_remote");
				if (is_valid_ipv4(tun_remote))
					fprintf(fp, "-A %s -p icmp --icmp-type %s -s %s -j %s\n", dtype, "echo-request", tun_remote, logaccept);
			}
#endif
		}
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	// FORWARD chain
	dtype = "FORWARD";

	if (i_mac_filter > 0)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	if (is_fw_enabled) {
		/* Accept the redirect packets (might be seen as INVALID) */
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, lan_if, lan_if, "ACCEPT");
	}

	/* use url filter before accepting ESTABLISHED packets */
	if (is_url_enabled && include_webstr_filter(fp) > 0) {
		char mac_buf[24] = {0};
		timematch[0] = 0;
		timematch_conv(timematch, "url_date_x", "url_time_x");
		mac_conv("url_mac_x", -1, mac_buf);
		if (strlen(mac_buf) == 17) {
			strcat(timematch, " -m mac");
			if (nvram_match("url_inv_x", "1"))
				strcat(timematch, " !");
			strcat(timematch, " --mac-source ");
			strcat(timematch, mac_buf);
		}
		fprintf(fp, "-A %s -i %s%s -j %s\n", dtype, lan_if, timematch, IPT_CHAIN_NAME_URL_LIST);
		ret |= MODULE_WEBSTR_MASK;
	}

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	if (!tcp_mss_need) {
		if (i_vpnc_enable) {
			char *vpnc_if = get_tcp_mss_ifname_vpnc(i_vpnc_type);
			if (vpnc_if)
				fprintf(fp, "-A %s%s -o %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, "", vpnc_if);
		}
		if (i_vpns_enable) {
			char *vpns_if = get_tcp_mss_ifname_vpns(i_vpns_type);
			if (vpns_if)
				fprintf(fp, "-A %s%s -i %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, "", vpns_if);
		}
	} else {
		fprintf(fp, "-A %s%s -o %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, " !", lan_if);
	}

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Drop all packets in the INVALID state */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Pass multicast (do not log) */
		if (nvram_match("mr_enable_x", "1"))
			fprintf(fp, "-A %s -p udp -d 224.0.0.0/4 -j %s\n", dtype, "ACCEPT");
#if 0
		/* Filter out invalid WAN->WAN connections */
		fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, wan_if, lan_if, logdrop);
		if (strcmp(man_if, wan_if))
			fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, man_if, lan_if, logdrop);
#endif
	}

	/* DoS attacks from WAN */
	if (is_dos_enabled)
		fprintf(fp, "-A %s -i %s -m %s %s -j %s\n", dtype, wan_if, CT_STATE, "NEW", IPT_CHAIN_NAME_DOS_LIMIT);

	/* FILTER from LAN to WAN */
	if (is_lwf_enabled && include_lw_filter(fp, wan_if, logaccept, logdrop) > 0) {
		timematch[0] = 0;
		timematch_conv(timematch, "filter_lw_date_x", "filter_lw_time_x");
		fprintf(fp, "-A %s -i %s%s -j %s\n", dtype, lan_if, timematch, IPT_CHAIN_NAME_LWF_LIST);
	}

	if (is_fw_enabled) {
		/* Accept LAN other outbound traffic */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Pass ICMPv4 in gateway mode w/o NAT */
		if (!is_nat_enabled) {
			if (nvram_invmatch("misc_ping_x", "0"))
				fprintf(fp, "-A %s -p icmp -j %s\n", dtype, logaccept);
			else
				fprintf(fp, "-A %s -p icmp ! --icmp-type %s -j %s\n", dtype, "echo-request", logaccept);
		}
		
		/* Accept VPN server client's outbound traffic */
		if (i_vpns_enable) {
			int i_need_vpnlist = 1;
#if defined (APP_OPENVPN)
			if (i_vpns_type == 2 && i_vpns_ov_mode == 0)
				i_need_vpnlist = 0;
#endif
			/* Jump to vpnlist chain */
			if (i_need_vpnlist) {
				if (i_vpns_actl == 0 || i_vpns_actl == 1)
					fprintf(fp, "-A %s -j %s\n", dtype, IPT_CHAIN_NAME_VPN_LIST);
				else if (i_vpns_actl == 4)
					fprintf(fp, "-A %s -o %s -j %s\n", dtype, wan_if, IPT_CHAIN_NAME_VPN_LIST);
				else
					fprintf(fp, "-A %s -o %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_VPN_LIST);
			}
		}
		
		/* Block or Accept inbound traffic from VPN client (do not log) */
		if (vpnc_if) {
			if (i_vpnc_sfw == 1)
				fprintf(fp, "-A %s -i %s -j %s\n", dtype, vpnc_if, "DROP");
			else if (i_vpnc_sfw == 0 || i_vpnc_sfw == 2)
				fprintf(fp, "-A %s -i %s -j %s\n", dtype, vpnc_if, "ACCEPT");
		}
		
		if (is_nat_enabled) {
			char *dmz_ip;
			
			/* Accept to BattleNET */
			if (nvram_match("sp_battle_ips", "1"))
				fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, BATTLENET_PORT, logaccept);
			
#if defined (USE_MATCH_CONNTRACK)
			/* Accept to Virtual Servers, UPnP, DMZ */
			fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, "conntrack --ctstate", "DNAT", logaccept);
#else
			/* Accept to exposed station (DMZ) */
			dmz_ip = nvram_safe_get("dmz_ip");
			if (is_valid_ipv4(dmz_ip))
				fprintf(fp, "-A %s -d %s -j %s\n", dtype, dmz_ip, logaccept);
			
			/* Accept to Virtual Servers */
			if (nvram_match("vts_enable_x", "1"))
				include_vts_filter(fp, lan_ip, logaccept, 1);
			
			/* Jump to IGD UPnP/NAT-PMP (miniupnpd chain) */
			if (nvram_invmatch("upnp_enable_x", "0"))
				fprintf(fp, "-A %s -j %s\n", dtype, MINIUPNPD_CHAIN_IP4_FORWARD);
#endif
		}
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	/* vpnlist chain */
	if (i_vpns_enable) {
		dtype = IPT_CHAIN_NAME_VPN_LIST;
		ftype = "ACCEPT";
#if defined (APP_OPENVPN)
		if (i_vpns_type == 2 && i_vpns_ov_mode == 1)
			fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_SERVER_TUN, ftype);
		else
#endif
			include_vpns_clients(fp);
	}

	/* bfplimit chain */
	dtype = IPT_CHAIN_NAME_BFP_LIMIT;
	if (i_bfplimit_ref) {
		int i_bfp_mode = nvram_get_int("sshd_wbfp");
		if (i_bfp_mode > 0) {
			fprintf(fp, "-A %s -m recent --set --name %s\n", dtype, "blacklist");
			fprintf(fp, "-A %s -m recent --update --hitcount %d --seconds %d --name %s -j %s\n", dtype, 4, get_sshd_bfp_time(i_bfp_mode), "blacklist", logdrop);
		}
		fprintf(fp, "-A %s -j %s\n", dtype, logaccept);
	}

	/* doslimit chain */
	if (is_dos_enabled) {
		dtype = IPT_CHAIN_NAME_DOS_LIMIT;
		ftype = "RETURN";
		fprintf(fp, "-A %s -p tcp --syn -m limit --limit 20/s --limit-burst 30 -j %s\n", dtype, ftype);
		fprintf(fp, "-A %s -p tcp --syn -j %s\n", dtype, logdrop);
		fprintf(fp, "-A %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j %s\n", dtype, ftype);
		fprintf(fp, "-A %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -j %s\n", dtype, logdrop);
		fprintf(fp, "-A %s -p icmp --icmp-type %s -m limit --limit 2/s -j %s\n", dtype, "echo-request", ftype);
		fprintf(fp, "-A %s -p icmp --icmp-type %s -j %s\n", dtype, "echo-request", logdrop);
	}

	/* logaccept chain */
	if (is_logaccept) {
		dtype = IPT_CHAIN_NAME_LOG_ACCEPT;
		ftype = "ACCEPT";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	/* logdrop chain */
	if (is_logdrop) {
		dtype = IPT_CHAIN_NAME_LOG_DROP;
		ftype = "DROP";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW,INVALID", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	if (ret & MODULE_WEBSTR_MASK)
		module_smart_load("xt_webstr", NULL);

	doSystem("iptables-restore %s", ipt_file);

	return ret;
}

void
ipt_filter_default(void)
{
	FILE *fp;
	char *ftype, *dtype;
	int is_fw_enabled;
	const char *ipt_file = "/tmp/ipt_filter.default";

	is_fw_enabled = nvram_match("fw_enable_x", "1");

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_FORWARD);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VPN_LIST);

	/* INPUT chain */
	dtype = "INPUT";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 67, 68, ftype);

	/* FORWARD chain */
	dtype = "FORWARD";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, IFNAME_BR, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	doSystem("iptables-restore %s", ipt_file);
}

static void
ipt_mangle_rules(const char *man_if, const char *wan_if, int use_man)
{
	FILE *fp;
	int i_wan_ttl_fix;
	int i_wan_ttl_value;
	const char *dtype;
	const char *ipt_file = "/tmp/ipt_mangle.rules";

	i_wan_ttl_fix = nvram_get_int("wan_ttl_fix");
	i_wan_ttl_value = nvram_get_int("wan_ttl_value");

	if (i_wan_ttl_fix == 2 && nvram_invmatch("mr_enable_x", "1"))
		i_wan_ttl_fix = 0;

	if (i_wan_ttl_value > 1) {
			fput_int("/proc/sys/net/ipv4/ip_default_ttl", i_wan_ttl_value);
	}

	if (i_wan_ttl_fix) {
		module_smart_load("iptable_mangle", NULL);
		module_smart_load("xt_HL", NULL);
	}

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "mangle");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");

	dtype = "PREROUTING";

	if (i_wan_ttl_fix) {
		const char *viptv_if = man_if;
		const char *viptv_iflast = nvram_safe_get("viptv_ifname");
		if (*viptv_iflast && is_interface_exist(viptv_iflast))
			viptv_if = viptv_iflast;
		
		if (i_wan_ttl_fix == 1) {
			fprintf(fp, "-A %s -i %s -j TTL %s %d\n", dtype, wan_if, "--ttl-inc", 1);
			if (use_man)
				fprintf(fp, "-A %s -i %s -j TTL %s %d\n", dtype, man_if, "--ttl-inc", 1);
		}
		
		if (i_wan_ttl_fix == 2 || (strcmp(wan_if, viptv_if) && strcmp(man_if, viptv_if)))
			fprintf(fp, "-A %s -i %s -p udp -d 224.0.0.0/4 -j TTL %s %d\n", dtype, viptv_if, "--ttl-inc", 1);
	}

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	if (i_wan_ttl_fix || is_module_loaded("iptable_mangle"))
		doSystem("iptables-restore %s", ipt_file);
}

static void
ipt_raw_rules(void)
{
	FILE *fp;
	const char *ipt_file = "/tmp/ipt_raw.rules";

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "raw");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	if (is_module_loaded("iptable_raw"))
		doSystem("iptables-restore %s", ipt_file);
}

#if defined (USE_IPV6)
static int
ip6t_filter_rules(char *man_if, char *wan_if, char *lan_if,
                  char *logaccept, char *logdrop, int tcp_mss_need)
{
	FILE *fp;
	char timematch[160], *ftype, *dtype;
	int ret, wport, lport;
	int ipv6_type, is_fw_enabled, is_url_enabled, is_logaccept, is_logdrop;
	int i_http_proto, i_bfplimit_ref, i_mac_filter;
	const char *ipt_file = "/tmp/ip6t_filter.rules";

	ret = 0;
	i_bfplimit_ref = 0;

	ipv6_type = get_ipv6_type();

	is_fw_enabled = nvram_match("fw_enable_x", "1");
	is_url_enabled = nvram_match("url_enable_x", "1");

	i_mac_filter = nvram_get_int("macfilter_enable_x");

	is_logaccept = (strcmp(logaccept, IPT_CHAIN_NAME_LOG_ACCEPT) == 0) ? 1 : 0;
	is_logdrop = (strcmp(logdrop, IPT_CHAIN_NAME_LOG_DROP) == 0) ? 1 : 0;

	if (!(fp=fopen(ipt_file, "w")))
		return 0;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" :  "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP6_FORWARD);
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_BFP_LIMIT);
	if (i_mac_filter > 0)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_MAC_LIST);
	if (is_url_enabled)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_URL_LIST);
	if (is_logaccept)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_ACCEPT);
	if (is_logdrop)
		fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_LOG_DROP);

	// maclist chain
	i_mac_filter = include_mac_filter(fp, i_mac_filter, logdrop);

	// INPUT chain (accept_source_route=0 by default, no needed drop RH0 packet)
	dtype = "INPUT";

	/* Policy for all traffic from MAC-filtered LAN clients */
	if (i_mac_filter > 0 && nvram_match("fw_mac_drop", "1"))
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Accept all traffic from LAN clients */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Accept all traffic from localhost */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", "ACCEPT");
		
		/* Accept ICMPv6 (ICMPv6 may be INVALID state!) */
		if (nvram_invmatch("misc_ping_x", "0"))
			fprintf(fp, "-A %s -p %d -j %s\n", dtype, 58, logaccept);
		else
			fprintf(fp, "-A %s -p %d ! --icmpv6-type %s -j %s\n", dtype, 58, "echo-request", logaccept);
		
		/* Drop the wrong state, INVALID, packets */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Accept all multicast (do not log) */
		if (nvram_match("mr_enable_x", "1"))
			fprintf(fp, "-A %s -d ff00::/8 -j %s\n", dtype, "ACCEPT");
		
		/* Accept DHCPv6 */
		if (ipv6_type == IPV6_NATIVE_DHCP6)
			fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 547, 546, logaccept);
		
		// Firewall between WAN and Local
		
		/* http/https/ssh/ftp/udpxy accepted from wan only for original ports (no NAT in IPv6) */
#if defined (SUPPORT_HTTPS)
		i_http_proto = nvram_get_int("http_proto");
#else
		i_http_proto = 0;
#endif
		wport = nvram_get_int("misc_httpport_x");
		lport = nvram_get_int("http_lanport");
		if ((i_http_proto == 0 || i_http_proto == 2) && nvram_match("misc_http_x", "1") && (wport == lport))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
#if defined (SUPPORT_HTTPS)
		wport = nvram_get_int("https_wport");
		lport = nvram_get_int("https_lport");
		if ((i_http_proto == 1 || i_http_proto == 2) && nvram_match("https_wopen", "1") && (wport == lport))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
#endif
#if defined (APP_SSHD)
		wport = nvram_get_int("sshd_wport");
		lport = 22;
		if (nvram_invmatch("sshd_enable", "0") && nvram_match("sshd_wopen", "1") && (wport == lport)) {
			i_bfplimit_ref++;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, IPT_CHAIN_NAME_BFP_LIMIT);
		}
#endif
#if defined (APP_FTPD)
		wport = nvram_get_int("ftpd_wport");
		lport = 21;
		if (nvram_invmatch("enable_ftp", "0") && nvram_match("ftpd_wopen", "1") && (wport == lport))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
#endif
		wport = nvram_get_int("udpxy_wport");
		lport = nvram_get_int("udpxy_enable_x");
		if (lport > 1023 && nvram_match("udpxy_wopen", "1") && (wport == lport))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, lport, logaccept);
#if defined (APP_TRMD)
		if (nvram_match("trmd_enable", "1") && is_torrent_support()) {
			wport = nvram_get_int("trmd_pport");
			if (wport < 1024 || wport > 65535) wport = TRANSMISSION_PPORT;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (nvram_match("trmd_ropen", "1")) {
				wport = nvram_get_int("trmd_rport");
				if (wport < 1024 || wport > 65535) wport = TRANSMISSION_RPORT;
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			}
		}
#endif
#if defined (APP_ARIA)
		if (nvram_match("aria_enable", "1") && is_aria_support()) {
			wport = nvram_get_int("aria_pport");
			if (wport < 1024 || wport > 65535) wport = ARIA_PPORT;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (nvram_match("aria_ropen", "1")) {
				wport = nvram_get_int("aria_rport");
				if (wport < 1024 || wport > 65535) wport = ARIA_RPORT;
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			}
		}
#endif
		if (nvram_match("vpns_enable", "1")) {
			int i_vpns_type = nvram_get_int("vpns_type");
#if defined (APP_OPENVPN)
			if (i_vpns_type == 2) {
				const char *ov_prot = "udp";
				int i_ov_port = nvram_safe_get_int("vpns_ov_port", 1194, 1, 65535);
				int i_ov_prot = nvram_get_int("vpns_ov_prot");
				
				if (i_ov_prot == 1 || i_ov_prot == 3 || i_ov_prot == 5)
					ov_prot = "tcp";
				if (i_ov_prot == 2 || i_ov_prot == 3 || i_ov_prot == 4 || i_ov_prot == 5)
					fprintf(fp, "-A %s -p %s --dport %d -j %s\n", dtype, ov_prot, i_ov_port, logaccept);
			} else
#endif
			if (i_vpns_type == 1) {
				fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, 1701, logaccept);
			} else {
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 1723, logaccept);
				fprintf(fp, "-A %s -p %d -j %s\n", dtype, 47, logaccept);
			}
		}
		
		/* Accept UDP ports 33434-33534 for traceroute */
		if (nvram_invmatch("misc_ping_x", "0"))
			fprintf(fp, "-A %s -p udp --dport %d:%d -j %s\n", dtype, 33434, 33534, logaccept);
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	// FORWARD chain (accept_source_route=0 by default, no needed drop RH0 packet)
	dtype = "FORWARD";

	if (i_mac_filter > 0)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, IPT_CHAIN_NAME_MAC_LIST);

	if (is_fw_enabled) {
		/* Pass the redirect, might be seen as INVALID, packets */
		fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, lan_if, lan_if, "ACCEPT");
	}

	/* use url filter before accepting ESTABLISHED packets */
	if (is_url_enabled && include_webstr_filter(fp) > 0) {
		char mac_buf[24] = {0};
		
		timematch[0] = 0;
		timematch_conv(timematch, "url_date_x", "url_time_x");
		mac_conv("url_mac_x", -1, mac_buf);
		if (strlen(mac_buf) == 17) {
			strcat(timematch, " -m mac");
			if (nvram_match("url_inv_x", "1"))
				strcat(timematch, " !");
			strcat(timematch, " --mac-source ");
			strcat(timematch, mac_buf);
		}
		fprintf(fp, "-A %s -i %s%s -j %s\n", dtype, lan_if, timematch, IPT_CHAIN_NAME_URL_LIST);
		ret |= MODULE_WEBSTR_MASK;
	}

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	if ((ipv6_type != IPV6_NATIVE_STATIC && ipv6_type != IPV6_NATIVE_DHCP6) || tcp_mss_need)
		fprintf(fp, "-A %s%s -o %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype, " !", lan_if);

	/* Pass related connections, skip rest of checks */
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", "ACCEPT");

	if (is_fw_enabled) {
		/* Pass ICMPv6 (ICMPv6 may be in INVALID state!) */
		if (nvram_invmatch("misc_ping_x", "0"))
			fprintf(fp, "-A %s -p %d -j %s\n", dtype, 58, logaccept);
		else
			fprintf(fp, "-A %s -p %d ! --icmpv6-type %s -j %s\n", dtype, 58, "echo-request", logaccept);
		
		/* Drop all packets in the INVALID state */
		fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", logdrop);
		
		/* Pass multicast (do not log) */
		if (nvram_match("mr_enable_x", "1"))
			fprintf(fp, "-A %s -d ff00::/8 -j %s\n", dtype, "ACCEPT");
#if 0
		/* Filter out invalid WAN->WAN connections */
		if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
			fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, IFNAME_SIT, lan_if, logdrop);
		fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, wan_if, lan_if, logdrop);
		if (!is_wan_ipv6_if_ppp() && strcmp(man_if, wan_if))
			fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, man_if, lan_if, logdrop);
#endif
		/* Accept LAN other outbound traffic  */
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");
		
		/* Jump to IPv6 pinhole (IGDv2 IP6FC or PCP) */
		if (nvram_invmatch("upnp_enable_x", "0"))
			fprintf(fp, "-A %s -j %s\n", dtype, MINIUPNPD_CHAIN_IP6_FORWARD);
		
		/* Drop all (only for log) */
		if (is_logdrop)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	// bfplimit chain
	dtype = IPT_CHAIN_NAME_BFP_LIMIT;
	if (i_bfplimit_ref) {
		int i_bfp_mode = nvram_get_int("sshd_wbfp");
		if (i_bfp_mode > 0) {
			fprintf(fp, "-A %s -m recent --set --name %s\n", dtype, "blacklist6");
			fprintf(fp, "-A %s -m recent --update --hitcount %d --seconds %d --name %s -j %s\n", dtype, 4, get_sshd_bfp_time(i_bfp_mode), "blacklist6", logdrop);
		}
		fprintf(fp, "-A %s -j %s\n", dtype, logaccept);
	}

	// logaccept chain
	if (is_logaccept) {
		dtype = IPT_CHAIN_NAME_LOG_ACCEPT;
		ftype = "ACCEPT";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	// logdrop chain
	if (is_logdrop) {
		dtype = IPT_CHAIN_NAME_LOG_DROP;
		ftype = "DROP";
		fprintf(fp, "-A %s -m %s %s -j LOG --log-prefix \"%s \" "
			    "--log-tcp-options --log-ip-options\n", dtype, CT_STATE, "NEW,INVALID", ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, ftype);
	}

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	if (ret & MODULE_WEBSTR_MASK)
		module_smart_load("xt_webstr", NULL);

	doSystem("ip6tables-restore %s", ipt_file);

	return ret;
}

void
ip6t_filter_default(void)
{
	FILE *fp;
	char *ftype, *dtype;
	int is_fw_enabled;
	const char *ipt_file = "/tmp/ip6t_filter.default";

	is_fw_enabled = nvram_match("fw_enable_x", "1");

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "filter");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", (is_fw_enabled) ? "DROP" : "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", (is_fw_enabled) ? "DROP" :  "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP6_FORWARD);

	// INPUT chain
	dtype = "INPUT";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, "lo", ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -p %d ! --icmpv6-type %s -j %s\n", dtype, 58, "echo-request", ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 547, 546, ftype);

	// FORWARD chain
	dtype = "FORWARD";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "ESTABLISHED,RELATED", ftype);
	fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, IFNAME_BR, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -p %d -j %s\n", dtype, 58, ftype);
	fprintf(fp, "-A %s -m %s %s -j %s\n", dtype, CT_STATE, "INVALID", "DROP");
	if (is_fw_enabled)
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	doSystem("ip6tables-restore %s", ipt_file);
}

static void
ip6t_mangle_rules(char *man_if)
{
	FILE *fp;
	const char *ipt_file = "/tmp/ip6t_mangle.rules";

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "mangle");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "INPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "FORWARD", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	if (is_module_loaded("ip6table_mangle"))
		doSystem("ip6tables-restore %s", ipt_file);
}

#if defined (APP_NAPT66)
static void
ip6t_disable_filter(void)
{
	doSystem("ip6tables -P FORWARD ACCEPT");
	doSystem("ip6tables -F FORWARD");
}
#endif
#endif

static int
ipt_nat_rules(char *man_if, char *man_ip,
              char *wan_if, char *wan_ip,
              char *lan_if, char *lan_ip, char *lan_net,
              int use_man, int wan_proto)
{
	FILE *fp;
	char *dtype, *vpnc_if;
	const char *ipt_file = "/tmp/ipt_nat.rules";
	int wport, lport, is_nat_enabled, is_fw_enabled, is_upnp_enabled;
	int i_vpns_enable, i_vpnc_enable, i_vpns_type, i_vpnc_type, i_vpnc_sfw, i_http_proto;
#if defined (APP_OPENVPN)
	int i_vpns_ov_mode = nvram_get_int("vpns_ov_mode");
#endif

	is_nat_enabled = nvram_match("wan_nat_x", "1");
	is_fw_enabled = nvram_match("fw_enable_x", "1");
	is_upnp_enabled = nvram_invmatch("upnp_enable_x", "0");

	i_vpns_enable = nvram_get_int("vpns_enable");
	i_vpnc_enable = nvram_get_int("vpnc_enable");
	i_vpns_type = nvram_get_int("vpns_type");
	i_vpnc_type = nvram_get_int("vpnc_type");
	i_vpnc_sfw = nvram_get_int("vpnc_sfw");

	vpnc_if = NULL;
	if (i_vpnc_enable) {
#if defined (APP_OPENVPN)
		if (i_vpnc_type == 2) {
			if (nvram_get_int("vpnc_ov_mode") == 1)
				vpnc_if = IFNAME_CLIENT_TUN;
			else if (nvram_get_int("vpnc_ov_cnat") == 1)
				vpnc_if = IFNAME_CLIENT_TAP;
		} else
#endif
			vpnc_if = IFNAME_CLIENT_PPP;
	}

	if (!is_valid_ipv4(wan_ip))
		wan_ip = NULL;

	if (!is_valid_ipv4(man_ip))
		man_ip = NULL;

	if (!(fp=fopen(ipt_file, "w")))
		return 0;

	fprintf(fp, "*%s\n", "nat");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VSERVER);
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_NAT);
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_NAT_POST);

	// VSERVER chain
	dtype = IPT_CHAIN_NAME_VSERVER;

	/* pre-routing from WAN */
	if (wan_ip)
		fprintf(fp, "-A PREROUTING -d %s -j %s\n", wan_ip, dtype);

	/* pre-routing from MAN */
	if (man_ip && use_man)
		fprintf(fp, "-A PREROUTING -d %s -j %s\n", man_ip, dtype);

	/* pre-routing from VPN client */
	if (vpnc_if && i_vpnc_sfw == 3)
		fprintf(fp, "-A PREROUTING -i %s ! -d %s -j %s\n", vpnc_if, lan_net, dtype);

	if (is_nat_enabled) {
		char dmz_ip[16];
		int is_use_dmz;
		
		snprintf(dmz_ip, sizeof(dmz_ip), "%s", nvram_safe_get("dmz_ip"));
		is_use_dmz = (is_valid_ipv4(dmz_ip)) ? 1 : 0;
		
		/* BattleNET (PREROUTING + POSTROUTING) */
		if (wan_ip && nvram_match("sp_battle_ips", "1")) {
			fprintf(fp, "-A %s -p udp -d %s --sport %d -j NETMAP --to %s\n", "PREROUTING", wan_ip, BATTLENET_PORT, lan_net);
			fprintf(fp, "-A %s -p udp -s %s --dport %d -j NETMAP --to %s\n", "POSTROUTING", lan_net, BATTLENET_PORT, wan_ip);
		}
		
#if 0
		/* miniupnpd postrouting chain (is really needed for XBox One?) */
		if (is_upnp_enabled)
			fprintf(fp, "-A %s -o %s -j %s\n", "POSTROUTING", wan_if, MINIUPNPD_CHAIN_IP4_NAT_POST);
#endif
		
		/* masquerade WAN connection for LAN clients */
		include_masquerade(fp, wan_if, wan_ip, lan_net);
		
		/* masquerade MAN connection for LAN clients */
		if (use_man) {
#if 0
			/* miniupnpd postrouting chain (is really needed for XBox One?) */
			if (is_upnp_enabled)
				fprintf(fp, "-A %s -o %s -j %s\n", "POSTROUTING", man_if, MINIUPNPD_CHAIN_IP4_NAT_POST);
#endif
			include_masquerade(fp, man_if, man_ip, lan_net);
		}
		
		/* masquerade VPN client connection for LAN clients */
		if (vpnc_if && i_vpnc_sfw != 2)
			include_masquerade(fp, vpnc_if, NULL, lan_net);
		
		/* masquerade WAN connection for VPN server clients */
		if (i_vpns_enable) {
			char vpn_net[24] = {0};
			ip2class(nvram_safe_get("vpns_vnet"), VPN_SERVER_SUBNET_MASK, vpn_net, sizeof(vpn_net));
			if (strcmp(vpn_net, lan_net) != 0) {
				int i_vpns_vuse = nvram_get_int("vpns_vuse");
				int i_vpns_actl = nvram_get_int("vpns_actl");
#if defined (APP_OPENVPN)
				if (i_vpns_type == 2) {
					if (i_vpns_ov_mode == 1) {
						if (i_vpns_actl == 0 || i_vpns_actl == 1 || i_vpns_actl == 4)
							include_masquerade(fp, wan_if, wan_ip, vpn_net);
						
						/* masquerade VPN server clients to LAN */
						if (i_vpns_vuse == 2)
							include_masquerade(fp, lan_if, lan_ip, vpn_net);
					}
				} else
#endif
				{
					if (i_vpns_vuse && (i_vpns_actl == 0 || i_vpns_actl == 1 || i_vpns_actl == 4))
						include_masquerade(fp, wan_if, wan_ip, vpn_net);
					
					/* masquerade VPN server clients to LAN */
					if (i_vpns_vuse == 2)
						include_masquerade(fp, lan_if, lan_ip, vpn_net);
				}
			}
		}
		
		/* masquerade LAN to LAN (NAT loopback) */
		if (nvram_match("nf_nat_loop", "1"))
			fprintf(fp, "-A POSTROUTING -o %s -s %s -d %s -j SNAT --to-source %s\n", lan_if, lan_net, lan_net, lan_ip);
		
		/* Local ports remap (http/https/ssh/ftp/udpxy) */
		if (is_fw_enabled) {
#if defined (SUPPORT_HTTPS)
			i_http_proto = nvram_get_int("http_proto");
#else
			i_http_proto = 0;
#endif
			if ((i_http_proto == 0 || i_http_proto == 2) && nvram_match("misc_http_x", "1")) {
				wport = nvram_get_int("misc_httpport_x");
				lport = nvram_get_int("http_lanport");
				if (wport < 80 || wport > 65535) {
					wport = 8080;
					nvram_set_int("misc_httpport_x", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#if defined (SUPPORT_HTTPS)
			if ((i_http_proto == 1 || i_http_proto == 2) && nvram_match("https_wopen", "1")) {
				wport = nvram_get_int("https_wport");
				lport = nvram_get_int("https_lport");
				if (wport < 81 || wport > 65535) {
					wport = 8443;
					nvram_set_int("https_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#endif
#if defined (APP_SSHD)
			if (nvram_invmatch("sshd_enable", "0") && nvram_match("sshd_wopen", "1")) {
				wport = nvram_get_int("sshd_wport");
				lport = 22;
				if (wport < 22 || wport > 65535) {
					wport = 10022;
					nvram_set_int("sshd_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#endif
#if defined (APP_FTPD)
			if (nvram_invmatch("enable_ftp", "0") && nvram_match("ftpd_wopen", "1")) {
				wport = nvram_get_int("ftpd_wport");
				lport = 21;
				if (wport < 21 || wport > 65535) {
					wport = 21;
					nvram_set_int("ftpd_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
#endif
			lport = nvram_get_int("udpxy_enable_x");
			if (lport > 1023 && nvram_match("udpxy_wopen", "1")) {
				wport = nvram_get_int("udpxy_wport");
				if (wport < 1024 || wport > 65535) {
					wport = lport;
					nvram_set_int("udpxy_wport", wport);
				}
				if (wport != lport || is_use_dmz)
					fprintf(fp, "-A %s -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
							dtype, wport, lan_ip, lport);
			}
		}
		
		/* Port Forwarding (Virtual Server) mappings */
		if (nvram_match("vts_enable_x", "1"))
			include_vts_nat(fp);
		
		/* IGD UPnP mappings */
		if (is_upnp_enabled)
			fprintf(fp, "-A %s -j %s\n", dtype, MINIUPNPD_CHAIN_IP4_NAT);
		
		/* Exposed station (DMZ), skip local traffic first */
		if (is_use_dmz) {
			unsigned int vpn_proto_mask = 0;
#if defined (APP_OPENVPN)
			unsigned int ovpns_hash = 0, ovpnc_hash = 0;
#endif
			/* skip DMZ for local VPN server */
			if (i_vpns_enable) {
#if defined (APP_OPENVPN)
				if (i_vpns_type == 2) {
					int i_prot = nvram_get_int("vpns_ov_prot");
					
					/* OpenVPN IPv4 */
					if (i_prot < 2) {
						vpn_proto_mask |= 0x04;
						ovpns_hash = nvram_safe_get_int("vpns_ov_port", 1194, 1, 65535);
						if (i_prot == 1)
							ovpns_hash |= (1u << 16);
					}
				} else
#endif
				if (i_vpns_type == 1) {
					/* L2TP */
					vpn_proto_mask |= 0x02;
				} else {
					/* PPTP */
					vpn_proto_mask |= 0x01;
				}
			}
			
			/* skip DMZ for local VPN client */
			if (i_vpnc_enable) {
#if defined (APP_OPENVPN)
				if (i_vpnc_type == 2) {
					int i_prot = nvram_get_int("vpnc_ov_prot");
					
					/* OpenVPN IPv4 */
					if (i_prot < 2) {
						vpn_proto_mask |= 0x04;
						ovpnc_hash = nvram_safe_get_int("vpnc_ov_port", 1194, 1, 65535);
						if (i_prot == 1)
							ovpnc_hash |= (1u << 16);
					}
				} else
#endif
				if (i_vpnc_type == 1) {
					/* L2TP */
					vpn_proto_mask |= 0x02;
				} else {
					/* PPTP */
					vpn_proto_mask |= 0x01;
				}
			}
			
			/* skip DMZ for WAN VPN client */
			if (wan_proto == IPV4_WAN_PROTO_L2TP)
				vpn_proto_mask |= 0x02;
			else if (wan_proto == IPV4_WAN_PROTO_PPTP)
				vpn_proto_mask |= 0x01;
			
#if defined (APP_OPENVPN)
			if (vpn_proto_mask & 0x04) {
				if (ovpns_hash != 0) {
					const char *ov_prot = (ovpns_hash & (1u << 16)) ? "tcp" : "udp";
					fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n",
							dtype, ov_prot, (ovpns_hash & 0xffff));
				}
				if (ovpnc_hash != 0 && ovpnc_hash != ovpns_hash) {
					const char *ov_prot = (ovpnc_hash & (1u << 16)) ? "tcp" : "udp";
					fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n",
							dtype, ov_prot, (ovpnc_hash & 0xffff));
				}
			}
#endif
			if (vpn_proto_mask & 0x02) {
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "udp", 1701);
			}
			
			if (vpn_proto_mask & 0x01) {
				fprintf(fp, "-A %s -p %d -j RETURN\n", dtype, 47);
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "tcp", 1723);
			}
#if defined (USE_IPV6)
			/* skip DMZ for local IPv6 (SIT) */
			{
				int ipv6_type = get_ipv6_type();
				if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
					fprintf(fp, "-A %s -p %d -j RETURN\n", dtype, 41);
			}
#endif
#if defined (APP_TRMD)
			/* skip DMZ for local Transmission */
			if (nvram_match("trmd_enable", "1") && is_torrent_support()) {
				wport = nvram_get_int("trmd_pport");
				if (wport < 1024 || wport > 65535) wport = TRANSMISSION_PPORT;
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "tcp", wport);
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "udp", wport);
				
				if (nvram_match("trmd_ropen", "1")) {
					wport = nvram_get_int("trmd_rport");
					if (wport < 1024 || wport > 65535) wport = TRANSMISSION_RPORT;
					fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "tcp", wport);
				}
			}
#endif
#if defined (APP_ARIA)
			/* skip DMZ for local Aria2 */
			if (nvram_match("aria_enable", "1") && is_aria_support()) {
				wport = nvram_get_int("aria_pport");
				if (wport < 1024 || wport > 65535) wport = ARIA_PPORT;
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "tcp", wport);
				fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "udp", wport);
				
				if (nvram_match("aria_ropen", "1")) {
					wport = nvram_get_int("aria_rport");
					if (wport < 1024 || wport > 65535) wport = ARIA_RPORT;
					fprintf(fp, "-A %s -p %s --dport %d -j RETURN\n", dtype, "tcp", wport);
				}
			}
#endif
			/* DNAT all other packets to DMZ host (except ICMP - use Port Forwarding to redirect ICMP) */
			fprintf(fp, "-A %s ! -p icmp -j DNAT --to %s\n", dtype, dmz_ip);
		}
	}

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	doSystem("iptables-restore %s", ipt_file);

	return 0;
}

void
ipt_nat_default(void)
{
	FILE *fp;
	int is_nat_enabled;
	const char *ipt_file = "/tmp/ipt_nat.default";

	is_nat_enabled = nvram_match("wan_nat_x", "1");

	if (!(fp=fopen(ipt_file, "w")))
		return;

	fprintf(fp, "*%s\n", "nat");
	fprintf(fp, ":%s %s [0:0]\n", "PREROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "POSTROUTING", "ACCEPT");
	fprintf(fp, ":%s %s [0:0]\n", "OUTPUT", "ACCEPT");
	fprintf(fp, ":%s - [0:0]\n", IPT_CHAIN_NAME_VSERVER);
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_NAT);
	fprintf(fp, ":%s - [0:0]\n", MINIUPNPD_CHAIN_IP4_NAT_POST);

	if (is_nat_enabled) {
		char *lan_if = IFNAME_BR;
		char *lan_ip = nvram_safe_get("lan_ipaddr");
		if (is_valid_ipv4(lan_ip)) {
			char lan_net[24] = {0};
			ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_net, sizeof(lan_net));
			
			/* masquerade lan to lan (NAT loopback) */
			if (nvram_match("nf_nat_loop", "1")) {
				fprintf(fp, "-A POSTROUTING -o %s -s %s -d %s -j SNAT --to-source %s\n",
					lan_if, lan_net, lan_net, lan_ip);
			}
		}
	}

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	doSystem("iptables-restore %s", ipt_file);
}

void
start_firewall_ex(void)
{
	int unit, wan_proto, i_tcp_mss, i_use_man, i_rp;
	char logaccept[16], logdrop[16];
	char wan_if[16], man_if[16], lan_if[16];
	char wan_ip[16], man_ip[16], lan_ip[16], lan_net[24] = {0};
	const char *opt_iptables_script = "/opt/bin/update_iptables.sh";
	const char *int_iptables_script = SCRIPT_POST_FIREWALL;
#if defined (APP_SHADOWSOCKS)
	const char *shadowsocks_iptables_script = "/tmp/shadowsocks_iptables.save";
#endif

	unit = 0;

	snprintf(lan_if, sizeof(lan_if), "%s", IFNAME_BR);
	snprintf(man_if, sizeof(man_if), "%s", get_man_ifname(unit));
	snprintf(wan_if, sizeof(wan_if), "%s", get_wan_unit_value(unit, "ifname_t"));

	if (strlen(wan_if) < 1)
		get_wan_ifname(wan_if);

	snprintf(lan_ip, sizeof(lan_ip), "%s", nvram_safe_get("lan_ipaddr"));
	snprintf(man_ip, sizeof(man_ip), "%s", nvram_safe_get("wanx_ipaddr"));
	snprintf(wan_ip, sizeof(wan_ip), "%s", get_wan_unit_value(unit, "ipaddr"));

	ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_net, sizeof(lan_net));

	wan_proto = get_wan_proto(unit);
	i_tcp_mss = is_need_tcp_mss_wan(unit, wan_proto, man_if);

	i_use_man = 0;
	if (ppp_ifindex(wan_if) >= 0 && strcmp(man_if, wan_if)) {
		if (wan_proto != IPV4_WAN_PROTO_PPPOE || get_wan_unit_value_int(unit, "pppoe_man") > 0)
			i_use_man = 1;
	}

	/* mcast needs rp filter to be turned off only for non default iface */
	i_rp = 1;
	if (nvram_match("mr_enable_x", "1") || nvram_invmatch("udpxy_enable_x", "0")
#if defined (APP_XUPNPD)
	 || nvram_invmatch("xupnpd_enable_x", "0")
#endif
	    )
		i_rp = 0;
	set_interface_conf_int("ipv4", man_if, "rp_filter", i_rp);

	/* Determine the log type */
	if (nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
		strcpy(logaccept, IPT_CHAIN_NAME_LOG_ACCEPT);
	else
		strcpy(logaccept, "ACCEPT");

	if (nvram_match("fw_log_x", "drop") || nvram_match("fw_log_x", "both"))
		strcpy(logdrop, IPT_CHAIN_NAME_LOG_DROP);
	else
		strcpy(logdrop, "DROP");

	/* IPv4 Raw rules */
	ipt_raw_rules();

	/* IPv4 Mangle rules */
	ipt_mangle_rules(man_if, wan_if, i_use_man);

	/* IPv4 NAT rules */
	ipt_nat_rules(man_if, man_ip, wan_if, wan_ip, lan_if, lan_ip, lan_net, i_use_man, wan_proto);

	/* IPv4 Filter rules */
	ipt_filter_rules(man_if, wan_if, lan_if, lan_ip, logaccept, logdrop, i_tcp_mss);

#if defined (USE_IPV6)
	/* IPv6 Mangle rules */
	ip6t_mangle_rules(man_if);

	/* IPv6 Filter rules */
	ip6t_filter_rules(man_if, wan_if, lan_if, logaccept, logdrop, i_tcp_mss);
#if defined (APP_NAPT66)
	if (nvram_match("napt66_enable", "1"))
		ip6t_disable_filter();
#endif
#endif

#if defined (APP_SHADOWSOCKS)
	if (check_if_file_exist(shadowsocks_iptables_script))
		doSystem("sh %s", shadowsocks_iptables_script);
#endif
	if (check_if_file_exist(int_iptables_script))
		doSystem("%s", int_iptables_script);

	if (check_if_file_exist(opt_iptables_script))
		doSystem("%s update", opt_iptables_script);

	/* enable IPv4 forward */
	set_ipv4_forward(1);

	/* try unload unused iptables modules */
	module_smart_unload("xt_webstr", 0);
	module_smart_unload("xt_HL", 0);
	module_smart_unload("iptable_raw", 0);
	module_smart_unload("iptable_mangle", 0);
	module_smart_unload("ip6table_mangle", 0);
}

