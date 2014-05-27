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
#include <unistd.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>

#include <nvram/bcmnvram.h>

#include "rc.h"

#define DHCPD_LEASE_FILE	"/tmp/dnsmasq.leases"
#define UPNPD_LEASE_FILE	"/tmp/miniupnpd.leases"
#define INADYN_USER_DIR		"/etc/storage/inadyn"

static void
simple_dhcp_range(char *ip, char *dip1, char *dip2, char *mask)
{
	struct in_addr ina;
	unsigned int new_start, new_end, lip, lmask;

	lip   = ntohl(inet_addr(ip));
	lmask = ntohl(inet_addr(mask));

	new_start = (lip & lmask) | 1;
	new_end = (lip & lmask) | (~lmask - 1);

	ina.s_addr = htonl(new_start);
	strcpy(dip1, inet_ntoa(ina));
	ina.s_addr = htonl(new_end);
	strcpy(dip2, inet_ntoa(ina));
}

static int
chk_valid_startend(char *ip, char *ip1, char *ip2, char *sub)
{
	if (!is_same_subnet(ip, ip1, sub) || !is_same_subnet(ip, ip2, sub))
	{
		simple_dhcp_range(ip, ip1, ip2, sub);
		return 0;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////
// dnsmasq
///////////////////////////////////////////////////////////////////////

int
is_dns_dhcpd_run(void)
{
	return pids("dnsmasq");
}

int
start_dns_dhcpd(void)
{
	FILE *fp;
	int i, i_max, i_sdhcp, i_dns, is_use_dhcp, i_verbose;
	char dhcp_mac[32], dhcp_ip[32], *smac, *sip;
	char *start, *end, *ipaddr, *mask, *gw, *dns1, *dns2, *dns3, *domain;
	char dhcp_start[16], dhcp_end[16], lan_ipaddr[16], lan_netmask[16];
	size_t ethers = 0;
	char *leases_dhcp = DHCPD_LEASE_FILE;
	char *resolv_conf = "/etc/resolv.conf";
	char *storage_dir = "/etc/storage/dnsmasq";

	if (get_ap_mode())
		return 0;

	/* create /etc/hosts */
	update_hosts_router();

	/* touch resolv.conf if not exist */
	create_file(resolv_conf);

	/* touch dnsmasq.leases if not exist */
	create_file(leases_dhcp);

	i_sdhcp = nvram_get_int("dhcp_static_x");
	i_max  = nvram_get_int("dhcp_staticnum_x");
	if (i_max > 64) i_max = 64;

	/* create /etc/ethers */
	fp = fopen("/etc/ethers", "w+");
	if (fp) {
		if (i_sdhcp == 1) {
			for (i = 0; i < i_max; i++) {
				sprintf(dhcp_mac, "dhcp_staticmac_x%d", i);
				smac = nvram_safe_get(dhcp_mac);
				if (strlen(smac) == 12) {
					sprintf(dhcp_ip, "dhcp_staticip_x%d", i);
					sip = nvram_safe_get(dhcp_ip);
					ethers += fprintf(fp, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c %s\n", 
						smac[0], smac[1], smac[2], smac[3], smac[4], smac[5], 
						smac[6], smac[7], smac[8], smac[9], smac[10], smac[11], sip);
				}
			}
		}
		fclose(fp);
	}

	ipaddr = nvram_safe_get("lan_ipaddr");
	domain = nvram_safe_get("lan_domain");

	/* create /etc/dnsmasq.conf */
	if (!(fp = fopen("/etc/dnsmasq.conf", "w"))) {
		return errno;
	}

	fprintf(fp, "user=%s\n"
		    "resolv-file=%s\n"
		    "no-poll\n"
		    "bogus-priv\n"
		    "addn-hosts=%s/hosts\n"
		    "interface=%s\n"
		    "listen-address=%s\n"
		    "bind-dynamic\n", SYS_USER_NOBODY, resolv_conf, storage_dir, IFNAME_BR, ipaddr);

	if (strlen(domain) > 1) {
		fprintf(fp, "domain=%s\n"
			    "expand-hosts\n", domain);
	}

	fprintf(fp, "no-negcache\n"
		    "cache-size=%d\n"
		    "clear-on-reload\n", 1000);

	is_use_dhcp = 0;
	i_verbose = nvram_get_int("dhcp_verbose");
	if (nvram_match("dhcp_enable_x", "1")) {
		memset(dhcp_start, 0, 16);
		memset(dhcp_end, 0, 16);
		memset(lan_ipaddr, 0, 16);
		memset(lan_netmask, 0, 16);
		
		start  = nvram_safe_get("dhcp_start");
		end    = nvram_safe_get("dhcp_end");
		mask   = nvram_safe_get("lan_netmask");
		
		strcpy(lan_ipaddr, *ipaddr?ipaddr:"192.168.1.1");
		strcpy(dhcp_start, *start&&*end?start:"192.168.1.2");
		strcpy(dhcp_end,   *start&&*end?end:"192.168.1.244");
		strcpy(lan_netmask,*start&&*end&&*mask?mask:"255.255.255.0");
		
		if (!chk_valid_startend(lan_ipaddr, dhcp_start, dhcp_end, lan_netmask))
		{
			nvram_set("dhcp_start", dhcp_start);
			nvram_set("dhcp_end", dhcp_end);
		}
		
		fprintf(fp, "dhcp-range=%s,%s,%s,%s\n", dhcp_start, dhcp_end, lan_netmask, nvram_safe_get("dhcp_lease"));
		
		/* GATEWAY */
		gw = nvram_safe_get("dhcp_gateway_x");
		if (!is_valid_ipv4(gw))
			gw = lan_ipaddr;
		fprintf(fp, "dhcp-option=%d,%s\n", 3, gw);
		
		/* DNS server */
		i_dns = 0;
		dns1 = nvram_safe_get("dhcp_dns1_x");
		dns2 = nvram_safe_get("dhcp_dns2_x");
		dns3 = nvram_safe_get("dhcp_dns3_x");
		fprintf(fp, "dhcp-option=%d", 6);
		if (is_valid_ipv4(dns1)) {
			i_dns++;
			fprintf(fp, ",%s", dns1);
		}
		if (is_valid_ipv4(dns2) && (strcmp(dns2, dns1))) {
			i_dns++;
			fprintf(fp, ",%s", dns2);
		}
		if (is_valid_ipv4(dns3) && (strcmp(dns3, dns1) && strcmp(dns3, dns2))) {
			i_dns++;
			fprintf(fp, ",%s", dns3);
		}
		if (i_dns < 1)
			fprintf(fp, ",%s", lan_ipaddr);
		fprintf(fp, "\n");
		
		/* DOMAIN search */
		if (strlen(domain) > 1)
			fprintf(fp, "dhcp-option=%d,%s\n", 15, domain);
		
		/* WINS */
		if (nvram_invmatch("dhcp_wins_x", ""))
			fprintf(fp, "dhcp-option=%d,%s\n", 44, nvram_safe_get("dhcp_wins_x"));
		
		if (i_verbose == 0 || i_verbose == 2)
			fprintf(fp, "quiet-dhcp\n");
		
		if (ethers)
			fprintf(fp, "read-ethers\n");
		
		is_use_dhcp = 1;
	}

#if defined (USE_IPV6)
	if (is_lan_radv_on() == 1) {
		int i_dhcp6s_mode = get_lan_dhcp6s_mode();
		if (i_dhcp6s_mode > 0) {
			int i_dhcp6s_irt = get_lan_dhcp6s_irt();
			
			if (i_dhcp6s_mode == 1) {
				/* Disable Stateful and SLAAC */
				fprintf(fp, "dhcp-range=::,static,%d\n", 600);
			} else {
				int i_sflt = nvram_safe_get_int("ip6_lan_sflt", 1800, 120, 604800);
				int i_sfps = nvram_safe_get_int("ip6_lan_sfps", 4096, 2, 65534);
				int i_sfpe = nvram_safe_get_int("ip6_lan_sfpe", 4352, 2, 65534);
				int i_pfsz = get_lan_dhcp6s_prefix_size();
				
				if (i_sfpe < i_sfps)
					i_sfpe = i_sfps;
				
				/* Enable Stateful, Disable SLAAC */
				fprintf(fp, "dhcp-range=::%x,::%x,constructor:%s,%d,%d\n", i_sfps, i_sfpe, IFNAME_BR, i_pfsz, i_sflt);
			}
			
			/* DNS server */
			fprintf(fp, "dhcp-option=option6:%d,[::]\n", 23);
			
			/* DOMAIN search */
			if (strlen(domain) > 1)
				fprintf(fp, "dhcp-option=option6:%d,%s\n", 24, domain);
			
			/* Information Refresh Time */
			fprintf(fp, "dhcp-option=option6:%d,%d\n", 32, i_dhcp6s_irt);
			
			if (i_verbose == 0 || i_verbose == 1) {
				fprintf(fp, "quiet-ra\n");
				fprintf(fp, "quiet-dhcp6\n");
			}
			
			is_use_dhcp = 1;
		}
	}
#endif
	if (is_use_dhcp) {
		fprintf(fp, "dhcp-leasefile=%s\n", leases_dhcp);
		fprintf(fp, "dhcp-authoritative\n");
	}

	fprintf(fp, "conf-file=%s/dnsmasq.conf\n", storage_dir);

	fclose(fp);

	return eval("/usr/sbin/dnsmasq");
}

void
stop_dns_dhcpd(void)
{
	char* svcs[] = { "dnsmasq", NULL };

	kill_services(svcs, 3, 1);
}

int
restart_dhcpd(void)
{
	stop_dns_dhcpd();
	return start_dns_dhcpd();
}

int
restart_dns(void)
{
	if (!is_dns_dhcpd_run())
		return -1;

	return doSystem("killall %s %s", "-SIGHUP", "dnsmasq");
}

///////////////////////////////////////////////////////////////////////
// miniupnpd
///////////////////////////////////////////////////////////////////////

int
is_upnp_run(void)
{
	return pids("miniupnpd");
}

int
start_upnp(void)
{
	FILE *fp;
	int ret, i_proto_use, i_clean_min, i_clean_int, i_iports[2], i_eports[2];
	char *lan_addr, *lan_mask, *lan_url, *proto_upnp, *proto_npmp, *secured;
	char var[100];
	char wan_ifname[16];
	char lan_class[32];
	uint8_t lan_mac[16];

	if (!nvram_get_int("upnp_enable_x") || !nvram_get_int("wan_nat_x") || get_ap_mode())
		return 0;

	i_proto_use = nvram_get_int("upnp_proto");
	i_clean_min = nvram_safe_get_int("upnp_clean_min", 10, 1, 999);
	i_clean_int = nvram_safe_get_int("upnp_clean_int", 600, 0, 86400);
	i_iports[0] = nvram_safe_get_int("upnp_iport_min", 21, 1, 65535);
	i_iports[1] = nvram_safe_get_int("upnp_iport_max", 65535, 1, 65535);
	i_eports[0] = nvram_safe_get_int("upnp_eport_min", 21, 1, 65535);
	i_eports[1] = nvram_safe_get_int("upnp_eport_max", 65535, 1, 65535);

	if (i_iports[0] > i_iports[1])
		i_iports[0] = i_iports[1];
	if (i_eports[0] > i_eports[1])
		i_eports[0] = i_eports[1];

	wan_ifname[0] = 0;
	get_wan_ifname(wan_ifname);

	lan_addr = nvram_safe_get("lan_ipaddr");
	lan_mask = nvram_safe_get("lan_netmask");
	ip2class(lan_addr, lan_mask, lan_class);
	memset(lan_mac, 0, sizeof(lan_mac));
	ether_atoe(nvram_safe_get("lan_hwaddr"), lan_mac);

	lan_url = lan_addr;
	ret = nvram_get_int("http_lanport");
	if (ret && ret != 80) {
		sprintf(var, "%s:%d", lan_addr, ret);
		lan_url = var;
	}

	if (i_proto_use == 2) {
		proto_upnp = "yes";
		proto_npmp = "yes";
	} else if (i_proto_use == 1) {
		proto_upnp = "no";
		proto_npmp = "yes";
	} else {
		proto_upnp = "yes";
		proto_npmp = "no";
	}

	secured = (nvram_get_int("upnp_secure")) ? "yes" : "no";

	/* Write configuration file */
	if (!(fp = fopen("/etc/miniupnpd.conf", "w"))) {
		return errno;
	}

	fprintf(fp, "# automagically generated\n"
		"ext_ifname=%s\n"
		"listening_ip=%s\n"
		"port=%d\n"
		"enable_upnp=%s\n"
		"enable_natpmp=%s\n"
		"upnp_forward_chain=%s\n"
		"upnp_nat_chain=%s\n"
		"secure_mode=%s\n"
		"lease_file=%s\n"
		"presentation_url=http://%s/\n"
		"system_uptime=yes\n"
		"notify_interval=%d\n"
		"clean_ruleset_interval=%d\n"
		"clean_ruleset_threshold=%d\n"
		"uuid=75802409-bccb-40e7-8e6c-%02x%02x%02x%02x%02x%02x\n"
		"friendly_name=%s\n"
		"manufacturer_name=%s\n"
		"manufacturer_url=%s\n"
		"model_name=%s\n"
		"model_description=%s\n"
		"model_url=%s\n"
		"model_number=%s\n"
		"serial=%s\n"
		"bitrate_up=%d\n"
		"bitrate_down=%d\n"
		"allow %d-%d %s %d-%d\n"
		"deny 0-65535 0.0.0.0/0 0-65535\n",
		wan_ifname,
		IFNAME_BR, /*lan_addr, lan_mask,*/
		0,
		proto_upnp,
		proto_npmp,
		MINIUPNPD_CHAIN_IP4_FORWARD,
		MINIUPNPD_CHAIN_IP4_NAT,
		secured,
		UPNPD_LEASE_FILE,
		lan_url,
		60,
		i_clean_int,
		i_clean_min,
		lan_mac[0], lan_mac[1], lan_mac[2], lan_mac[3], lan_mac[4], lan_mac[5],
		BOARD_DESC,
		BOARD_VENDOR_NAME,
		BOARD_VENDOR_URL,
		"Wireless Router",
		BOARD_DESC,
		BOARD_MODEL_URL,
		BOARD_NAME,
		"1.0",
		100000000,
		100000000,
		i_eports[0], i_eports[1], lan_class, i_iports[0], i_iports[1]);

	fclose(fp);

	return eval("/usr/bin/miniupnpd");
}

void
stop_upnp(void)
{
	char* svcs[] = { "miniupnpd", NULL };
	kill_services(svcs, 3, 1);
}

void
smart_restart_upnp(void)
{
	char wan_ifname[16];

	if (!is_upnp_run())
	{
		start_upnp();
		
		return;
	}

	wan_ifname[0] = 0;
	get_wan_ifname(wan_ifname);

	/* restart miniupnpd only if wan interface changed */
	if (strcmp(wan_ifname, nvram_safe_get("wan_ifname_t")) != 0) {
		stop_upnp();
		start_upnp();
	}
}

void
update_upnp(void)
{
	if (!is_upnp_run())
	{
		start_upnp();
		return;
	}

	/* update upnp forwards from lease file */
	if (check_if_file_exist(UPNPD_LEASE_FILE)) {
		doSystem("killall %s %s", "-SIGUSR1", "miniupnpd");
	}
}

///////////////////////////////////////////////////////////////////////
// inadyn
///////////////////////////////////////////////////////////////////////

int
ddns_updated_main(int argc, char *argv[])
{
	// Purge dnsmasq cache
	if (getenv("INADYN_IP"))
		restart_dns();

	return 0;
}

void
stop_ddns(void)
{
	char* svcs[] = { "inadyn", NULL };

	kill_services(svcs, 3, 1);
}

static const struct inadyn_system_t {
	const char *alias;
	const char *system;
} inadyn_systems[] = {
	{ "WWW.ASUS.COM",         "update@asus.com"            },
	{ "WWW.DYNDNS.ORG",       "default@dyndns.org"         },
	{ "WWW.TZO.COM",          "default@tzo.com"            },
	{ "WWW.ZONEEDIT.COM",     "default@zoneedit.com"       },
	{ "WWW.EASYDNS.COM",      "default@easydns.com"        },
	{ "WWW.NO-IP.COM",        "default@no-ip.com"          },
	{ "WWW.DNSOMATIC.COM",    "default@dnsomatic.com"      },
	{ "WWW.DNSEXIT.COM",      "default@dnsexit.com"        },
	{ "WWW.CHANGEIP.COM",     "default@changeip.com"       },
	{ "WWW.SITELUTIONS.COM",  "default@sitelutions.com"    },
	{ "WWW.ZERIGO.COM",       "default@zerigo.com"         },
	{ "WWW.DHIS.ORG",         "default@dhis.org"           },
	{ "WWW.DUCKDNS.ORG",      "default@duckdns.org"        },
	{ "WWW.TUNNELBROKER.NET", "ipv6tb@he.net"              },
	{ "DNS.HE.NET",           "dyndns@he.net"              },
	{ "TB.NETASSIST.UA",      "ipv6tb@netassist.ua"        },
	{ "FREEDNS.AFRAID.ORG",   "default@freedns.afraid.org" },
	{ NULL, NULL }
};

static const char *
inadyn_checkip_url[] = {
	"",
	"checkip.dyndns.org /",
	"checkip.dyndns.org:8245 /",
	"echo.tzo.com /",
	"ip.dnsexit.com /",
	"ip.changeip.com /",
	"myip.dnsomatic.com /",
	"ip1.dynupdate.no-ip.com /",
	"checkip.dns.he.net /",
	"checkip.zerigo.com /",
	"checkip.two-dns.de /",
	"ipv4.wtfismyip.com /text",
};

static int
write_inadyn_conf(const char *conf_file, int use_delay)
{
	FILE *fp;
	int i_max, i_ddns_source, i_ddns_checkip, i_ddns_period, i_ddns_forced, i_ddns1_ssl;
	char *ddns1_svc, *ddns1_srv, *ddns1_hname[3], *ddns1_user, *ddns1_pass;
	char *ddns2_svc, *ddns2_srv, *ddns2_hname,    *ddns2_user, *ddns2_pass;
	char wan_ifname[16];
	struct inadyn_system_t *inadyn;

	i_ddns_period = nvram_safe_get_int("ddns_period", 24, 0, 72);
	i_ddns_period *= 3600;
	if (i_ddns_period < 600)
		i_ddns_period = 600; // 10 min

	i_ddns_forced = nvram_safe_get_int("ddns_forced", 10, 3, 50);
	i_ddns_forced *= (24 * 3600);

	wan_ifname[0] = 0;
	i_ddns_source = nvram_get_int("ddns_source");
	if (i_ddns_source == 1)
		get_wan_ifname(wan_ifname);
	else if (i_ddns_source == 2)
		strcpy(wan_ifname, get_man_ifname(0));

	i_max = ARRAY_SIZE(inadyn_checkip_url) - 1;
	i_ddns_checkip = nvram_safe_get_int("ddns_checkip", 0, 0, i_max);

	ddns1_srv = nvram_safe_get("ddns_server_x");
	ddns1_svc = NULL;
	for (inadyn = (struct inadyn_system_t *)&inadyn_systems[0]; inadyn->alias; inadyn++) {
		if (strcmp(ddns1_srv, inadyn->alias) == 0) {
			ddns1_svc = (char *)inadyn->system;
			break;
		}
	}

	if (!ddns1_svc) {
		ddns1_svc = (char *)inadyn_systems[0].system;
		nvram_set("ddns_server_x", inadyn_systems[0].alias);
	}

	i_ddns1_ssl    = nvram_get_int("ddns_ssl");
	ddns1_hname[0] = nvram_safe_get("ddns_hostname_x");
	ddns1_hname[1] = nvram_safe_get("ddns_hostname2_x");
	ddns1_hname[2] = nvram_safe_get("ddns_hostname3_x");
	ddns1_user     = nvram_safe_get("ddns_username_x");
	ddns1_pass     = nvram_safe_get("ddns_passwd_x");

	if (strcmp(ddns1_svc, "update@asus.com") == 0) {
		char mac_str[16] = {0};
		unsigned char mac_bin[ETHER_ADDR_LEN] = {0};
		
#if defined (BOARD_N14U)
		/* use original MAC from EEPROM */
		ether_atoe(nvram_safe_get("il0macaddr"), mac_bin);
#else
		ether_atoe(nvram_safe_get("il1macaddr"), mac_bin);
#endif
		i_ddns1_ssl = 0;
		ddns1_hname[1] = "";
		ddns1_hname[2] = "";
		ddns1_user = ether_etoa3(mac_bin, mac_str);
		ddns1_pass = nvram_safe_get("secret_code");
	}

	ddns2_srv = nvram_safe_get("ddns2_server");
	ddns2_svc = NULL;
	for (inadyn = (struct inadyn_system_t *)&inadyn_systems[0]; inadyn->alias; inadyn++) {
		if (strcmp(ddns2_srv, inadyn->alias) == 0) {
			ddns2_svc = (char *)inadyn->system;
			break;
		}
	}

	ddns2_hname = nvram_safe_get("ddns2_hname");
	ddns2_user  = nvram_safe_get("ddns2_user");
	ddns2_pass  = nvram_safe_get("ddns2_pass");

	if (use_delay) {
		use_delay = (nvram_get_int("ntpc_counter") < 1) ? 10 : 3;
	}

	fp = fopen(conf_file, "w");
	if (fp) {
		fprintf(fp, "background\n");
		fprintf(fp, "verbose %d\n", nvram_safe_get_int("ddns_verbose", 0, 0, 5));
		if (strlen(wan_ifname) > 0)
			fprintf(fp, "iface %s\n", wan_ifname);
		else if (strlen(inadyn_checkip_url[i_ddns_checkip]) > 0)
			fprintf(fp, "checkip-url %s\n", inadyn_checkip_url[i_ddns_checkip]);
		if (use_delay)
			fprintf(fp, "startup-delay %d\n", use_delay);
		fprintf(fp, "period %d\n", i_ddns_period);
		fprintf(fp, "forced-update %d\n", i_ddns_forced);
		fprintf(fp, "cache-dir %s\n", DDNS_CACHE_DIR);
		fprintf(fp, "exec %s\n", DDNS_DONE_SCRIPT);
		fprintf(fp, "system %s\n", ddns1_svc);
#if defined (SUPPORT_DDNS_SSL)
		if (i_ddns1_ssl)
			fprintf(fp, "  ssl\n");
#endif
		if (strlen(ddns1_user) > 0)
			fprintf(fp, "  username %s\n", ddns1_user);
		if (strlen(ddns1_pass) > 0)
			fprintf(fp, "  password %s\n", ddns1_pass);
		fprintf(fp, "  alias %s\n", ddns1_hname[0]);
		if (strlen(ddns1_hname[1]) > 2)
			fprintf(fp, "  alias %s\n", ddns1_hname[1]);
		if (strlen(ddns1_hname[2]) > 2)
			fprintf(fp, "  alias %s\n", ddns1_hname[2]);
		if (nvram_get_int("ddns_wildcard_x"))
			fprintf(fp, "  wildcard\n");
		if (ddns2_svc) {
			fprintf(fp, "system %s\n", ddns2_svc);
#if defined (SUPPORT_DDNS_SSL)
			if (nvram_get_int("ddns2_ssl"))
				fprintf(fp, "  ssl\n");
#endif
			if (strlen(ddns2_user) > 0)
				fprintf(fp, "  username %s\n", ddns2_user);
			if (strlen(ddns2_pass) > 0)
				fprintf(fp, "  password %s\n", ddns2_pass);
			fprintf(fp, "  alias %s\n", ddns2_hname);
		}
		
		load_user_config(fp, INADYN_USER_DIR, "inadyn.conf");
		
		fclose(fp);
		
		return 1;
	}

	return 0;
}

int
start_ddns(int clear_cache)
{
	if (!nvram_match("ddns_enable_x", "1") || get_ap_mode())
		return -1;

	stop_ddns();

	if (clear_cache)
		doSystem("rm -rf %s", DDNS_CACHE_DIR);

	mkdir(DDNS_CACHE_DIR, 0777);

	write_inadyn_conf(DDNS_CONF_FILE, (clear_cache) ? 0 : 1);

	return eval("/bin/inadyn", "--config", DDNS_CONF_FILE);
}

int
update_ddns(void)
{
	if (pids("inadyn"))
	{
		write_inadyn_conf(DDNS_CONF_FILE, 1);
		return doSystem("killall %s %s", "-SIGHUP", "inadyn");
	}

	return start_ddns(0);
}

void
manual_ddns_hostname_check(void)
{
	int i_ddns_source;
	char mac_str[16], wan_ifname[16];
	const char *nvram_key = "ddns_return_code";
	unsigned char mac_bin[ETHER_ADDR_LEN] = {0};
	char *inadyn_argv[] = {
		"/bin/inadyn",
		"-S", "register@asus.com",
		"-o",
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL
	};

	if (!is_valid_ipv4(nvram_safe_get("wan_ipaddr_t"))) {
		nvram_set_temp(nvram_key, "connect_fail");
		return;
	}

	stop_ddns();

#if defined (BOARD_N14U)
	/* use original MAC from EEPROM */
	ether_atoe(nvram_safe_get("il0macaddr"), mac_bin);
#else
	ether_atoe(nvram_safe_get("il1macaddr"), mac_bin);
#endif

	wan_ifname[0] = 0;
	i_ddns_source = nvram_get_int("ddns_source");
	if (i_ddns_source == 1)
		get_wan_ifname(wan_ifname);
	else if (i_ddns_source == 2)
		strcpy(wan_ifname, get_man_ifname(0));

	inadyn_argv[4] = "-u";
	inadyn_argv[5] = ether_etoa3(mac_bin, mac_str);

	inadyn_argv[6] = "-p";
	inadyn_argv[7] = nvram_safe_get("secret_code");

	inadyn_argv[8] = "-a";
	inadyn_argv[9] = nvram_safe_get("ddns_hostname_x");

	if (*wan_ifname) {
		inadyn_argv[10] = "-i";
		inadyn_argv[11] = wan_ifname;
	}

	nvram_set_temp(nvram_key, "ddns_query");

	_eval(inadyn_argv, NULL, 0, NULL);

	if (nvram_match(nvram_key, "ddns_query"))
		nvram_set_temp(nvram_key, "connect_fail");

	start_ddns(0);
}

