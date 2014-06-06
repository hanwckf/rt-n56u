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
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>

#include <nvram/bcmnvram.h>

#include "rc.h"

static char *
get_wan_ppp_peer(const char *prefix)
{
	char tmp[100];
	char *ppp_peer = nvram_safe_get(strcat_r(prefix, "ppp_peer", tmp));
	if (!(*ppp_peer))
		ppp_peer = nvram_safe_get(strcat_r(prefix, "pppoe_gateway", tmp));
	return ppp_peer;
}

static int
write_xl2tpd_conf(char *l2tp_conf)
{
	FILE *fp;
	char *prefix = "wan0_";
	char tmp[100];
	int has_work = 0;

	if (!(fp = fopen(l2tp_conf, "w"))) {
		return -1;
	}

	fprintf(fp,
		"[global]\n"
		"port = 1701\n"
		"access control = no\n"
		"rand source = dev\n\n");

	if (nvram_match("vpns_enable", "1") && nvram_match("vpns_type", "1"))
	{
		int i_cli0, i_cli1;
		char pooll[32], pool1[32], pool2[32];
		unsigned int laddr, lmask;
		struct in_addr pool_in;
		
		if (!nvram_get_int("vpns_vuse"))
		{
			laddr = ntohl(inet_addr(nvram_safe_get("lan_ipaddr")));
			lmask = ntohl(inet_addr(nvram_safe_get("lan_netmask")));
			
			i_cli0 = nvram_get_int("vpns_cli0");
			i_cli1 = nvram_get_int("vpns_cli1");
			if (i_cli0 <   2) i_cli0 =   2;
			if (i_cli0 > 254) i_cli0 = 254;
			if (i_cli1 <   2) i_cli1 =   2;
			if (i_cli1 > 254) i_cli1 = 254;
			if (i_cli1 < i_cli0) i_cli1 = i_cli0;
		}
		else
		{
			laddr = ntohl(inet_addr(nvram_safe_get("vpns_vnet")));
			lmask = ntohl(inet_addr(VPN_SERVER_SUBNET_MASK));
			laddr = (laddr & lmask) | 1;
			
			i_cli0 = 2;
			i_cli1 = i_cli0 + MAX_CLIENTS_NUM - 1;
		}
		
		pool_in.s_addr = htonl(laddr);
		strcpy(pooll, inet_ntoa(pool_in));
		
		pool_in.s_addr = htonl((laddr & lmask) | (unsigned int)i_cli0);
		strcpy(pool1, inet_ntoa(pool_in));
		
		pool_in.s_addr = htonl((laddr & lmask) | (unsigned int)i_cli1);
		strcpy(pool2, inet_ntoa(pool_in));
		
		fprintf(fp, "[lns default]\n");
		fprintf(fp, "hostname = %s\n", get_our_hostname());
		fprintf(fp, "local ip = %s\n", pooll);
		fprintf(fp, "ip range = %s-%s\n", pool1, pool2);
		fprintf(fp, "pppoptfile = %s\n", VPN_SERVER_PPPD_OPTIONS);
		fprintf(fp, "require authentication = no\n");
		fprintf(fp, "tunnel rws = %d\n", 8);
		fprintf(fp,
			    "pass peer = yes\n"
			    "exclusive = yes\n"
			    "hidden bit = no\n"
			    "length bit = yes\n"
			    "require chap = yes\n"
			    "refuse pap = yes\n\n");
		
		has_work++;
	}

	if (nvram_match("wan_l2tpd", "0") && nvram_match(strcat_r(prefix, "proto", tmp), "l2tp"))
	{
		int unit;
		char options[64], lac_name[8];
		
		unit = nvram_get_int(strcat_r(prefix, "unit", tmp));
		if (unit < 0 || unit > WAN_PPP_UNIT_MAX) unit = WAN_PPP_UNIT;
		sprintf(lac_name, "ISP%d", unit);
		sprintf(options, "/tmp/ppp/options.wan%d", unit);
		
		fprintf(fp, "[lac %s]\n", lac_name);
		fprintf(fp, "pppoptfile = %s\n", options);
		fprintf(fp, "lns = %s\n", get_wan_ppp_peer(prefix));
		fprintf(fp, "name = %s\n", nvram_safe_get(strcat_r(prefix, "pppoe_username", tmp)));
		fprintf(fp, "require authentication = no\n");
		fprintf(fp, "tunnel rws = %d\n", 8);
		fprintf(fp, "route2man = %s\n", "yes");
		fprintf(fp,
			    "autodial = yes\n"
			    "redial = yes\n"
			    "redial timeout = 15\n"
			    "tx bps = 100000000\n"
			    "rx bps = 100000000\n\n");
		
		has_work++;
	}

	if (nvram_match("vpnc_enable", "1") && nvram_match("vpnc_type", "1"))
	{
		fprintf(fp, "[lac %s]\n", "VPNC");
		fprintf(fp, "pppoptfile = %s\n", VPN_CLIENT_PPPD_OPTIONS);
		fprintf(fp, "lns = %s\n",  nvram_safe_get("vpnc_peer"));
		fprintf(fp, "name = %s\n", nvram_safe_get("vpnc_user"));
		fprintf(fp, "require authentication = no\n");
		fprintf(fp, "tunnel rws = %d\n", 8);
		fprintf(fp, "route2man = %s\n", "no");
		fprintf(fp,
			    "autodial = yes\n"
			    "redial = yes\n"
			    "redial timeout = 15\n"
			    "tx bps = 100000000\n"
			    "rx bps = 100000000\n\n");
		
		nvram_set_int_temp("l2tp_cli_t", 1);
		
		has_work++;
	}

	fclose(fp);

	chmod(l2tp_conf, 0644);

	return has_work;
}

static int
write_rpl2tp_conf(char *l2tp_conf)
{
	FILE *fp;
	int unit;
	char tmp[100], options[64];
	char *prefix = "wan0_";

	unit = nvram_get_int(strcat_r(prefix, "unit", tmp));
	if (unit < 0 || unit > WAN_PPP_UNIT_MAX) unit = WAN_PPP_UNIT;
	sprintf(options, "/tmp/ppp/options.wan%d", unit);

	if (!(fp = fopen(l2tp_conf, "w"))) {
		return -1;
	}

	fprintf(fp, "# automagically generated\n"
		"global\n"
		"load-handler \"sync-pppd.so\"\n"
		"load-handler \"cmd.so\"\n\n"
		"section sync-pppd\n"
		"lac-pppd-opts \"file %s\"\n\n"
		"section peer\n"
		"peername %s\n"
		"lac-handler sync-pppd\n"
		"persist yes\n"
		"maxfail 0\n"    // l2tpd re-call count (0=infinite)
		"holdoff 15\n"   // l2tpd re-call time (15s)
		"route2man yes\n"
		"hide-avps no\n\n"
		"section cmd\n\n",
		options, get_wan_ppp_peer(prefix));

	fclose(fp);

	chmod(l2tp_conf, 0644);

	return 0;
}

int
safe_start_xl2tpd(void)
{
	char *l2tp_conf = "/etc/xl2tpd.conf";
	
	if (write_xl2tpd_conf(l2tp_conf) < 0)
		return -1;
	
	/* launch xl2tpd */
	if (!pids("xl2tpd"))
		return eval("/usr/sbin/xl2tpd", "-c", l2tp_conf);
	
	return 1;
}

static int
start_rpl2tp(void)
{
	char *l2tp_conf = "/etc/l2tp/l2tp.conf";

	mkdir_if_none("/etc/l2tp");

	if (write_rpl2tp_conf(l2tp_conf) < 0)
		return -1;

	/* launch rp-l2tp */
	eval("/usr/sbin/l2tpd");

	sleep(1);

	/* start-session */
	return system("/usr/sbin/l2tp-control \"start-session 0.0.0.0\"");
}

char *
safe_pppd_line(const char *line, char *tmp, size_t tmp_size)
{
	const char special_chars[] = "'\\";
	char *src, *dst;
	size_t dst_len;

	if (!tmp || tmp_size <= strlen(line))
		return (char *)line;

	dst = tmp;
	dst_len = tmp_size;
	src = (char *)line;

	while (*src != '\0' && dst_len > 2) {
		if (strchr(special_chars, *src)) {
			*dst++ = '\\';
			dst_len--;
		}
		*dst++ = *src++;
		dst_len--;
	}

	*dst = '\0';

	return tmp;
}

int
start_pppd(char *prefix, int unit, int wan_proto)
{
	FILE *fp;
	int auth_type;
	char options[64], tmp[64], tmp2[256];
	char *svcs[] = { NULL, NULL };

	if (unit < 0 || unit > WAN_PPP_UNIT_MAX)
		return -1;

	snprintf(options, sizeof(options), "/tmp/ppp/options.wan%d", unit);

	/* Generate options file */
	if (!(fp = fopen(options, "w"))) {
		perror(options);
		return -1;
	}

	/* do not authenticate peer and do not use eap */
	fprintf(fp, "noauth\n");
	fprintf(fp, "user '%s'\n", safe_pppd_line(nvram_safe_get(strcat_r(prefix, "pppoe_username", tmp)), tmp2, sizeof(tmp2)));
	fprintf(fp, "password '%s'\n", safe_pppd_line(nvram_safe_get(strcat_r(prefix, "pppoe_passwd", tmp)), tmp2, sizeof(tmp2)));
	fprintf(fp, "refuse-eap\n");

	auth_type = nvram_get_int(strcat_r(prefix, "ppp_auth", tmp));
	if (auth_type == 3) {
		/* MS-CHAPv2 */
		fprintf(fp, "refuse-pap\n");
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
	}
	else if (auth_type == 2) {
		/* CHAP */
		fprintf(fp, "refuse-pap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}
	else if (auth_type == 1) {
		/* PAP */
		fprintf(fp, "refuse-chap\n");
		fprintf(fp, "refuse-mschap\n");
		fprintf(fp, "refuse-mschap-v2\n");
	}

	if (wan_proto == IPV4_WAN_PROTO_PPTP) {
		fprintf(fp, "plugin pptp.so\n");
		fprintf(fp, "pptp_server '%s'\n", get_wan_ppp_peer(prefix));
		fprintf(fp, "route2man %d\n", 1);
		fprintf(fp, "mtu %d\n", nvram_safe_get_int(strcat_r(prefix, "pptp_mtu", tmp), 1400, 1000, 1476));
		fprintf(fp, "mru %d\n", nvram_safe_get_int(strcat_r(prefix, "pptp_mru", tmp), 1400, 1000, 1500));
	}

	if (wan_proto == IPV4_WAN_PROTO_L2TP) {
		fprintf(fp, "mtu %d\n", nvram_safe_get_int(strcat_r(prefix, "l2tp_mtu", tmp), 1460, 1000, 1460));
		fprintf(fp, "mru %d\n", nvram_safe_get_int(strcat_r(prefix, "l2tp_mru", tmp), 1460, 1000, 1500));
		
		// L2TP: Don't wait for LCP term responses; exit immediately when killed
		fprintf(fp, "lcp-max-terminate %d\n", 0);
	}

	if (wan_proto == IPV4_WAN_PROTO_PPPOE) {
		int demand;
		char *pppoe_ac = nvram_safe_get(strcat_r(prefix, "pppoe_ac", tmp));
		char *pppoe_sv = nvram_safe_get(strcat_r(prefix, "pppoe_service", tmp));
		
		fprintf(fp, "plugin rp-pppoe.so\n");
		fprintf(fp, "nic-%s\n", nvram_safe_get(strcat_r(prefix, "ifname", tmp)));
		
		if (*pppoe_ac)
			fprintf(fp, "rp_pppoe_ac '%s'\n", pppoe_ac);
		
		if (*pppoe_sv)
			fprintf(fp, "rp_pppoe_service '%s'\n", pppoe_sv);
		
		fprintf(fp, "mtu %d\n", nvram_safe_get_int(strcat_r(prefix, "pppoe_mtu", tmp), 1492, 1000, 1492));
		fprintf(fp, "mru %d\n", nvram_safe_get_int(strcat_r(prefix, "pppoe_mru", tmp), 1492, 1000, 1492));
		
		demand = nvram_get_int(strcat_r(prefix, "pppoe_idletime", tmp));
		if (demand > 0 && nvram_get_int(strcat_r(prefix, "pppoe_demand", tmp))) {
			fprintf(fp, "idle %d ", demand);
			if (nvram_invmatch(strcat_r(prefix, "pppoe_txonly_x", tmp), "0"))
				fprintf(fp, "tx_only ");
			fprintf(fp, "demand\n");
		}
	}

	if (wan_proto != IPV4_WAN_PROTO_L2TP)
		fprintf(fp, "persist\n");

	fprintf(fp, "maxfail %d\n", 0);		// pppd re-call count (0=infinite)
	fprintf(fp, "holdoff %d\n", 10);	// pppd re-call time(10s)

	fprintf(fp, "ipcp-accept-remote ipcp-accept-local\n");
	fprintf(fp, "noipdefault\n");

	if (!nvram_match(strcat_r(prefix, "dnsenable_x", tmp), "0"))
		fprintf(fp, "usepeerdns\n");

	fprintf(fp, "default-asyncmap\n");

	/* pppoe set these options automatically */
	/* looks like pptp also likes them */
	fprintf(fp, "nopcomp noaccomp\n");

	/* pppoe disables "vj bsdcomp deflate" automagically */
	/* ccp should still be enabled - mppe/mppc requires this */
	fprintf(fp, "novj nobsdcomp nodeflate\n");

	if (wan_proto != IPV4_WAN_PROTO_PPPOE) {
		int mppe_mode = nvram_get_int(strcat_r(prefix, "ppp_mppe", tmp));
		if (mppe_mode == 0) {
			fprintf(fp, "nomppe nomppc\n");
		} else {
			if (mppe_mode == 2) {
				fprintf(fp, "+mppe\n");
				fprintf(fp, "+mppe-40\n");
				fprintf(fp, "-mppe-128\n");
			} else if (mppe_mode == 3) {
				fprintf(fp, "+mppe\n");
				fprintf(fp, "-mppe-40\n");
				fprintf(fp, "+mppe-128\n");
			} else {
				fprintf(fp, "+mppe-40\n");
				fprintf(fp, "+mppe-128\n");
			}
			fprintf(fp, "nomppe-stateful\n");
		}
	} else {
		fprintf(fp, "nomppe nomppc\n");
	}

	/* echo failures (6*20s) */
	fprintf(fp, "lcp-echo-interval %d\n", 20);
	fprintf(fp, "lcp-echo-failure %d\n", 6);

	if (nvram_get_int(strcat_r(prefix, "ppp_alcp", tmp)) == 1)
		fprintf(fp, "lcp-echo-adaptive\n");

	fprintf(fp, "unit %d\n", unit);
	fprintf(fp, "ktune\n");

#if defined (USE_IPV6)
	/* Enable IPv6CP */
	if (is_wan_ipv6_type_sit() == 0 && is_wan_ipv6_if_ppp())
		fprintf(fp, "+ipv6\n");
#endif

	/* user specific options */
	fprintf(fp, "%s\n", safe_pppd_line(nvram_safe_get(strcat_r(prefix, "ppp_pppd", tmp)), tmp2, sizeof(tmp2)));

	fclose(fp);

	chmod(options, 0600);

	if (wan_proto == IPV4_WAN_PROTO_L2TP)
	{
		if (nvram_match("wan_l2tpd", "0"))
		{
			svcs[0] = "xl2tpd";
			kill_services(svcs, 5, 1);
			
			nvram_set_int_temp("l2tp_wan_t", 1);
			
			safe_start_xl2tpd();
		}
		else
		{
			svcs[0] = "l2tpd";
			kill_services(svcs, 5, 1);
			
			nvram_set_int_temp("l2tp_wan_t", 0);
			
			start_rpl2tp();
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
	char *wan_ifname = safe_getenv("IFNAME");
	char *value;
	char buf[256];
	int unit;
	char tmp[100], prefix[16];

	if (ppp_ifunit(wan_ifname) < 0)
		return -1;

	umask(0000);

	unit = 0;
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	if ((value = getenv("IPLOCAL"))) {
		ifconfig(wan_ifname, IFUP,
			 value, "255.255.255.255");
		nvram_set_temp(strcat_r(prefix, "ipaddr", tmp), value);
		nvram_set_temp(strcat_r(prefix, "netmask", tmp), "255.255.255.255");
	}

	if ((value = getenv("IPREMOTE")))
		nvram_set_temp(strcat_r(prefix, "gateway", tmp), value);

	buf[0] = 0;
	value = getenv("DNS1");
	if (value)
		snprintf(buf, sizeof(buf), "%s", value);
	value = getenv("DNS2");
	if (value && strcmp(value, buf) != 0) {
		int buf_len = strlen(buf);
		snprintf(buf + buf_len, sizeof(buf) - buf_len, "%s%s", (buf_len) ? " " : "", value);
	}
	nvram_set_temp(strcat_r(prefix, "dns", tmp), buf);

	nvram_set_int_temp(strcat_r(prefix, "time", tmp), uptime());

	wan_up(wan_ifname);

	logmessage(nvram_safe_get("wan_proto_t"), "Connected to ISP");

	return 0;
}

int
ipdown_main(int argc, char **argv)
{
	char *wan_ifname = safe_getenv("IFNAME");
	int unit;
	char tmp[100], prefix[16];

	if (ppp_ifunit(wan_ifname) < 0)
		return -1;

	umask(0000);

	if (strcmp(wan_ifname, IFNAME_RAS) == 0)
		create_file(FLAG_FILE_WWAN_GONE);

	unit = 0;
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	nvram_set_int_temp(strcat_r(prefix, "time", tmp), 0);

	wan_down(wan_ifname);

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

	umask(0000);

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

	umask(0000);

	wan6_down(wan_ifname);

	update_resolvconf(0, 0);

	return 0;
}
#endif

