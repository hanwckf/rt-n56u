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

#include "rc.h"

static char *
get_wan_ppp_peer(int unit)
{
	char *ppp_peer = get_wan_unit_value(unit, "ppp_peer");

	if (strlen(ppp_peer) < 1)
		ppp_peer = get_wan_unit_value(unit, "man_gateway");
	return ppp_peer;
}

int
safe_start_xl2tpd(void)
{
	FILE *fp;
	int unit, has_lac_lns;
	const char *l2tp_conf = "/etc/xl2tpd.conf";

	if (!(fp = fopen(l2tp_conf, "w"))) {
		perror(l2tp_conf);
		return -1;
	}

	has_lac_lns = 0;

	fprintf(fp,
		"[global]\n"
		"port = 1701\n"
		"access control = no\n"
		"rand source = dev\n\n");

	if (nvram_match("vpns_enable", "1") && nvram_match("vpns_type", "1")) {
		char sa_v[INET_ADDRSTRLEN], sp_b[INET_ADDRSTRLEN], sp_e[INET_ADDRSTRLEN];
		unsigned int vaddr, vmask, vp_b, vp_e;
		struct in_addr pool_in;
		
		get_vpns_pool(nvram_get_int("vpns_vuse"), &vaddr, &vmask, &vp_b, &vp_e);
		
		pool_in.s_addr = htonl(vaddr);
		strcpy(sa_v, inet_ntoa(pool_in));
		
		pool_in.s_addr = htonl((vaddr & vmask) | vp_b);
		strcpy(sp_b, inet_ntoa(pool_in));
		
		pool_in.s_addr = htonl((vaddr & vmask) | vp_e);
		strcpy(sp_e, inet_ntoa(pool_in));
		
		fprintf(fp, "[lns default]\n");
		fprintf(fp, "hostname = %s\n", get_our_hostname());
		fprintf(fp, "local ip = %s\n", sa_v);
		fprintf(fp, "ip range = %s-%s\n", sp_b, sp_e);
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
		
		has_lac_lns++;
	}

	unit = 0; // todo
	if (get_wan_proto(unit) == IPV4_WAN_PROTO_L2TP
#if defined (APP_RPL2TP)
	    && nvram_invmatch("wan_l2tpd", "1")
#endif
	    )
	{
		char options[64], lac_name[8];
		
		snprintf(lac_name, sizeof(lac_name), "ISP%d", unit);
		snprintf(options, sizeof(options), "/tmp/ppp/options.wan%d", unit);
		
		fprintf(fp, "[lac %s]\n", lac_name);
		fprintf(fp, "pppoptfile = %s\n", options);
		fprintf(fp, "lns = %s\n", get_wan_ppp_peer(unit));
		fprintf(fp, "name = %s\n", get_wan_unit_value(unit, "pppoe_username"));
		fprintf(fp, "require authentication = no\n");
		fprintf(fp, "tunnel rws = %d\n", 8);
		fprintf(fp, "route_rdgw = %d\n", 1);
		fprintf(fp,
			    "autodial = yes\n"
			    "redial = yes\n"
			    "redial timeout = 15\n"
			    "tx bps = 100000000\n"
			    "rx bps = 100000000\n\n");
		
		has_lac_lns++;
	}

	if (nvram_match("vpnc_enable", "1") && nvram_match("vpnc_type", "1"))
	{
		fprintf(fp, "[lac %s]\n", "VPNC");
		fprintf(fp, "pppoptfile = %s\n", VPN_CLIENT_PPPD_OPTIONS);
		fprintf(fp, "lns = %s\n",  nvram_safe_get("vpnc_peer"));
		fprintf(fp, "name = %s\n", nvram_safe_get("vpnc_user"));
		fprintf(fp, "require authentication = no\n");
		fprintf(fp, "tunnel rws = %d\n", 8);
		fprintf(fp, "route_rdgw = %d\n", (nvram_match("vpnc_dgw", "1")) ? 2 : 0);
		fprintf(fp,
			    "autodial = yes\n"
			    "redial = yes\n"
			    "redial timeout = 15\n"
			    "tx bps = 100000000\n"
			    "rx bps = 100000000\n\n");
		
		nvram_set_int_temp("l2tp_cli_t", 1);
		
		has_lac_lns++;
	}

	fclose(fp);

	chmod(l2tp_conf, 0644);

	/* launch xl2tpd */
	if (!pids("xl2tpd"))
		return eval("/usr/sbin/xl2tpd", "-c", (char *)l2tp_conf);

	return 1;
}

#if defined (APP_RPL2TP)
static int
start_rpl2tp(int unit)
{
	FILE *fp;
	char options[64];
	const char *l2tp_conf = "/etc/l2tp/l2tp.conf";

	mkdir_if_none("/etc/l2tp", "777");

	snprintf(options, sizeof(options), "/tmp/ppp/options.wan%d", unit);

	if (!(fp = fopen(l2tp_conf, "w"))) {
		perror(l2tp_conf);
		return -1;
	}

	fprintf(fp, "# automatically generated\n");
	fprintf(fp,
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
		"route_rdgw 1\n"
		"hide-avps no\n\n"
		"section cmd\n\n",
		options, get_wan_ppp_peer(unit));

	fclose(fp);

	chmod(l2tp_conf, 0644);

	/* launch rp-l2tp */
	eval("/usr/sbin/l2tpd");

	sleep(1);

	/* start-session */
	return system("/usr/sbin/l2tp-control \"start-session 0.0.0.0\"");
}
#endif

char *
safe_pppd_line(const char *line, char *buf, size_t buf_size)
{
	const char special_chars[] = "'\\";
	char *src, *dst;
	size_t dst_len;

	if (!buf || buf_size <= strlen(line))
		return (char *)line;

	dst = buf;
	dst_len = buf_size;
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

	return buf;
}

int
launch_wan_pppd(int unit, int wan_proto)
{
	FILE *fp;
	int auth_type, mtu, mru, mtu_max, mru_max, mtu_def, mru_def;
	char options[64], tmp[256];
	char *svcs[] = { NULL, NULL };

	if (unit < 0)
		return -1;

	snprintf(options, sizeof(options), "/tmp/ppp/options.wan%d", unit);

	/* Generate options file */
	if (!(fp = fopen(options, "w"))) {
		perror(options);
		return -1;
	}

	/* do not authenticate peer and do not use eap */
	fprintf(fp, "noauth\n");
	fprintf(fp, "user '%s'\n", safe_pppd_line(get_wan_unit_value(unit, "pppoe_username"), tmp, sizeof(tmp)));
	fprintf(fp, "password '%s'\n", safe_pppd_line(get_wan_unit_value(unit, "pppoe_passwd"), tmp, sizeof(tmp)));
	fprintf(fp, "refuse-eap\n");

	auth_type = get_wan_unit_value_int(unit, "ppp_auth");
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

	mtu     = 1492;
	mtu_def = 1492;
	mtu_max = 1492;

	mru     = 1492;
	mru_def = 1492;
	mru_max = 1492;

	if (wan_proto == IPV4_WAN_PROTO_PPTP) {
		mtu_def = 1400;
		mtu_max = 1476;
		mru_def = 1400;
		mru_max = 1500;
		mtu = get_wan_unit_value_int(unit, "pptp_mtu");
		mru = get_wan_unit_value_int(unit, "pptp_mru");
		
		fprintf(fp, "plugin pptp.so\n");
		fprintf(fp, "pptp_server '%s'\n", get_wan_ppp_peer(unit));
		fprintf(fp, "route_rdgw %d\n", 1);
	}

	if (wan_proto == IPV4_WAN_PROTO_L2TP) {
		mtu_def = 1460;
		mtu_max = 1460;
		mru_def = 1460;
		mru_max = 1500;
		mtu = get_wan_unit_value_int(unit, "l2tp_mtu");
		mru = get_wan_unit_value_int(unit, "l2tp_mru");
		
		// L2TP: Don't wait for LCP term responses; exit immediately when killed
		fprintf(fp, "lcp-max-terminate %d\n", 0);
	}

	if (wan_proto == IPV4_WAN_PROTO_PPPOE) {
		int demand;
		char *pppoe_ac, *pppoe_sv;
		
		mtu = get_wan_unit_value_int(unit, "pppoe_mtu");
		mru = get_wan_unit_value_int(unit, "pppoe_mru");
		
		fprintf(fp, "plugin rp-pppoe.so\n");
		fprintf(fp, "nic-%s\n", get_man_ifname(unit));
		
		pppoe_ac = get_wan_unit_value(unit, "pppoe_ac");
		if (*pppoe_ac)
			fprintf(fp, "rp_pppoe_ac '%s'\n", pppoe_ac);
		
		pppoe_sv = get_wan_unit_value(unit, "pppoe_service");
		if (*pppoe_sv)
			fprintf(fp, "rp_pppoe_service '%s'\n", pppoe_sv);
		
		demand = get_wan_unit_value_int(unit, "pppoe_idletime");
		if (demand > 0 && get_wan_unit_value_int(unit, "pppoe_demand") > 0) {
			fprintf(fp, "idle %d ", demand);
			if (get_wan_unit_value_int(unit, "pppoe_txonly_x") > 0)
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

	if (mtu < 1000) mtu = mtu_def;
	if (mtu > mtu_max) mtu = mtu_max;
	if (mru < 1000) mru = mru_def;
	if (mru > mru_max) mru = mru_max;
	fprintf(fp, "mtu %d\n", mtu);
	fprintf(fp, "mru %d\n", mru);

	if (get_wan_unit_value_int(unit, "dnsenable_x") > 0)
		fprintf(fp, "usepeerdns\n");

	fprintf(fp, "default-asyncmap\n");

	/* pppoe set these options automatically */
	/* looks like pptp also likes them */
	fprintf(fp, "nopcomp noaccomp\n");

	/* pppoe disables "vj bsdcomp deflate" automatically */
	/* ccp should still be enabled - mppe/mppc requires this */
	fprintf(fp, "novj nobsdcomp nodeflate\n");

	if (wan_proto != IPV4_WAN_PROTO_PPPOE) {
		int mppe_mode = get_wan_unit_value_int(unit, "ppp_mppe");
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

	if (get_wan_unit_value_int(unit, "ppp_alcp") > 0)
		fprintf(fp, "lcp-echo-adaptive\n");

	fprintf(fp, "minunit %d\n", unit);
	fprintf(fp, "linkname wan%d\n", unit);
	fprintf(fp, "ktune\n");

#if defined (USE_IPV6)
	/* Enable IPv6CP */
	if (is_wan_ipv6_type_sit() == 0 && is_wan_ipv6_if_ppp())
		fprintf(fp, "+ipv6\n");
#endif

	/* user specific options */
	fprintf(fp, "%s\n", safe_pppd_line(get_wan_unit_value(unit, "ppp_pppd"), tmp, sizeof(tmp)));

	fclose(fp);

	chmod(options, 0600);

	if (wan_proto == IPV4_WAN_PROTO_L2TP) {
#if defined (APP_RPL2TP)
		if (nvram_match("wan_l2tpd", "1")) {
			svcs[0] = "l2tpd";
			kill_services(svcs, 5, 1);
			
			nvram_set_int_temp("l2tp_wan_t", 0);
			
			start_rpl2tp(unit);
		} else
#endif
		{
			svcs[0] = "xl2tpd";
			kill_services(svcs, 5, 1);
			
			nvram_set_int_temp("l2tp_wan_t", 1);
			
			safe_start_xl2tpd();
		}
	} else {
		eval("/usr/sbin/pppd", "file", options);
	}

	return 0;
}

void
preset_wan_ppp_routes(char *ppp_ifname, int unit)
{
	/* Set default route to gateway if specified */
	if (get_wan_unit_value_int(unit, "primary") == 1)
		route_add(ppp_ifname, 1, "0.0.0.0", "0.0.0.0", "0.0.0.0");

	/* Install interface dependent static routes */
	add_static_wan_routes(ppp_ifname);
}

int
ppp_ifindex(char *ifname)
{
	if (strncmp(ifname, "ppp", 3))
		return -1;
	if (!isdigit(ifname[3]))
		return -1;
	return atoi(ifname+3);
}

static int
ppp_wan_unit(char *linkname)
{
	if (strncmp(linkname, "wan", 3))
		return -1;
	if (!isdigit(linkname[3]))
		return -1;
	return atoi(linkname+3);
}

static int
is_valid_wan_ppp_idx(int ifindex)
{
	if (ifindex < 0 || ifindex >= VPNC_PPP_UNIT)
		return 0;
	return 1;
}

int
ipup_main(int argc, char **argv)
{
	int unit, ppp_idx;
	char buf[256], *value;
	char *ppp_ifname = safe_getenv("IFNAME");
	char *ppp_linkname = safe_getenv("LINKNAME");
	char *ppp_mask = "255.255.255.255";

	ppp_idx = ppp_ifindex(ppp_ifname);
	if (!is_valid_wan_ppp_idx(ppp_idx))
		return -1;

	unit = ppp_wan_unit(ppp_linkname);
	if (unit < 0)
		return -1;

	/* update ifname_t */
	set_wan_unit_value(unit, "ifname_t", ppp_ifname);

	umask(0000);

	if ((value = getenv("IPLOCAL"))) {
		ifconfig(ppp_ifname, IFUP, value, ppp_mask);
		
		set_wan_unit_value(unit, "ipaddr", value);
		set_wan_unit_value(unit, "netmask", ppp_mask);
	}

	if ((value = getenv("IPREMOTE")))
		set_wan_unit_value(unit, "gateway", value);

	buf[0] = 0;
	value = getenv("DNS1");
	if (value)
		snprintf(buf, sizeof(buf), "%s", value);
	value = getenv("DNS2");
	if (value && strcmp(value, buf) != 0) {
		int buf_len = strlen(buf);
		snprintf(buf + buf_len, sizeof(buf) - buf_len, "%s%s", (buf_len) ? " " : "", value);
	}
	set_wan_unit_value(unit, "dns", buf);

	wan_up(ppp_ifname, unit, 0);

	set_vpn_balancing(ppp_ifname, 0);

	logmessage(get_wan_unit_value(unit, "proto_t"), "Connected");

	return 0;
}

int
ipdown_main(int argc, char **argv)
{
	int unit, ppp_idx;
	char *ppp_ifname = safe_getenv("IFNAME");
	char *ppp_linkname = safe_getenv("LINKNAME");

	ppp_idx = ppp_ifindex(ppp_ifname);
	if (!is_valid_wan_ppp_idx(ppp_idx))
		return -1;

	unit = ppp_wan_unit(ppp_linkname);
	if (unit < 0)
		return -1;

	umask(0000);

	wan_down(ppp_ifname, unit, 0);

	preset_wan_ppp_routes(ppp_ifname, unit);

	logmessage(get_wan_unit_value(unit, "proto_t"), "Disconnected");

	return 0;
}

#if defined (USE_IPV6)
int
ipv6up_main(int argc, char **argv)
{
	int unit, ppp_idx;
	char *ppp_ifname = safe_getenv("IFNAME");
	char *ppp_linkname = safe_getenv("LINKNAME");

	ppp_idx = ppp_ifindex(ppp_ifname);
	if (!is_valid_wan_ppp_idx(ppp_idx))
		return -1;

	unit = ppp_wan_unit(ppp_linkname);
	if (unit < 0)
		return -1;

	if (!is_wan_ipv6_if_ppp())
		return 0;

	umask(0000);

	wan6_up(ppp_ifname, unit);

	return 0;
}

int
ipv6down_main(int argc, char **argv)
{
	int unit, ppp_idx;
	char *ppp_ifname = safe_getenv("IFNAME");
	char *ppp_linkname = safe_getenv("LINKNAME");

	ppp_idx = ppp_ifindex(ppp_ifname);
	if (!is_valid_wan_ppp_idx(ppp_idx))
		return -1;

	unit = ppp_wan_unit(ppp_linkname);
	if (unit < 0)
		return -1;

	if (!is_wan_ipv6_if_ppp())
		return 0;

	umask(0000);

	wan6_down(ppp_ifname, unit);

	update_resolvconf(0, 0);

	return 0;
}
#endif

