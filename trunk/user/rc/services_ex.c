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

#include "rc.h"

#define DHCPD_LEASE_FILE	"/tmp/dnsmasq.leases"
#define UPNPD_LEASE_FILE	"/tmp/miniupnpd.leases"
#define INADYN_USER_DIR		"/etc/storage/inadyn"

static void
simple_dhcp_range(const char *ip, char *dip1, char *dip2, const char *mask)
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
chk_valid_subnet_pool(const char *ip, char *dip1, char *dip2, const char *mask)
{
	if (!is_same_subnet(ip, dip1, mask) || !is_same_subnet(ip, dip2, mask)) {
		simple_dhcp_range(ip, dip1, dip2, mask);
		return 0;
	}
	return 1;
}

static void
arpbind_clear(void)
{
	FILE *fp;
	char buffer[256], arp_ip[INET_ADDRSTRLEN], arp_if[32];
	unsigned int arp_flags;

	fp = fopen("/proc/net/arp", "r");
	if (fp) {
		// skip first line
		fgets(buffer, sizeof(buffer), fp);
		
		while (fgets(buffer, sizeof(buffer), fp)) {
			arp_flags = 0;
			if (sscanf(buffer, "%s %*s 0x%x %*s %*s %s", arp_ip, &arp_flags, arp_if) == 3) {
				if ((arp_flags & 0x04) && strcmp(arp_if, IFNAME_BR) == 0)
					doSystem("arp -i %s -d %s", IFNAME_BR, arp_ip);
			}
		}
		
		fclose(fp);
	}
}

/*
static int
find_static_mac(int idx, int num_items, const char *ip, char mac[13])
{
	int i;
	in_addr_t ip1, ip2;
	char nvram_mac[32], nvram_ip[32], *smac;

	ip1 = inet_addr_safe(ip);

	for (i = idx; i < num_items; i++) {
		snprintf(nvram_mac, sizeof(nvram_mac), "dhcp_staticmac_x%d", i);
		smac = nvram_safe_get(nvram_mac);
		if (strlen(smac) == 12) {
			snprintf(nvram_ip, sizeof(nvram_ip), "dhcp_staticip_x%d", i);
			ip2 = inet_addr_safe(nvram_safe_get(nvram_ip));
			if (ip2 == ip1 && ip2 != INADDR_NONE) {
				strcpy(mac, smac);
				return i;
			}
		}
	}

	return -1;
}
*/

static int
fill_static_ethers(const char *lan_ip, const char *lan_mask)
{
	FILE *fp;
	int i, i_use_static, i_arpbind, i_max_items, i_ethers = 0;
	char nvram_mac[32], nvram_ip[32], *smac, *sip;

	i_use_static = nvram_get_int("dhcp_static_x");

	/* create /etc/ethers */
	fp = fopen("/etc/ethers", "w+");
	if (fp) {
		if (i_use_static == 1) {
			i_arpbind = nvram_get_int("dhcp_static_arp");
			i_max_items = nvram_safe_get_int("dhcp_staticnum_x", 0, 0, 64);
			for (i = 0; i < i_max_items; i++) {
				snprintf(nvram_mac, sizeof(nvram_mac), "dhcp_staticmac_x%d", i);
				smac = nvram_safe_get(nvram_mac);
				if (strlen(smac) == 12) {
					snprintf(nvram_ip, sizeof(nvram_ip), "dhcp_staticip_x%d", i);
					sip = nvram_safe_get(nvram_ip);
					if (is_valid_ipv4(sip) && is_same_subnet(sip, lan_ip, lan_mask)) {
						i_ethers++;
						fprintf(fp, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c %s\n", 
							smac[0], smac[1], smac[2], smac[3], smac[4], smac[5], 
							smac[6], smac[7], smac[8], smac[9], smac[10], smac[11], sip);
						if (i_arpbind)
							doSystem("arp -i %s -s %s %s", IFNAME_BR, sip, smac);
					}
				}
			}
		}
		fclose(fp);
	}

	return i_ethers;
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
is_dhcpd_enabled(int is_ap_mode)
{
	if (!is_ap_mode)
		return nvram_match("dhcp_enable_x", "1");
	else
		return nvram_match("lan_dhcpd_x", "1") && nvram_invmatch("lan_proto_x", "1");
}

int
start_dns_dhcpd(int is_ap_mode)
{
	FILE *fp;
	int i_ethers, i_verbose, i_dhcp_enable, is_dhcp_used, is_dns_used;
	char dhcp_start[32], dhcp_end[32], dns_all[64];
	char *ipaddr, *netmask, *gw, *dns1, *dns2, *dns3, *wins, *domain;
	char *leases_dhcp = DHCPD_LEASE_FILE;
	char *resolv_conf = "/etc/resolv.conf";
	char *storage_dir = "/etc/storage/dnsmasq";

	if (!is_ap_mode) {
		/* create /etc/hosts */
		update_hosts_router();
		
		/* touch resolv.conf if not exist */
		create_file(resolv_conf);
	}

	i_dhcp_enable = is_dhcpd_enabled(is_ap_mode);

	/* touch dnsmasq.leases if not exist */
	create_file(leases_dhcp);

	i_verbose = nvram_get_int("dhcp_verbose");

	ipaddr  = nvram_safe_get("lan_ipaddr");
	netmask = nvram_safe_get("lan_netmask");
	domain  = nvram_safe_get("lan_domain");

	/* create /etc/ethers */
	i_ethers = fill_static_ethers(ipaddr, netmask);

	/* create /etc/dnsmasq.conf */
	if (!(fp = fopen("/etc/dnsmasq.conf", "w")))
		return errno;

	fprintf(fp, "user=%s\n"
		    "resolv-file=%s\n"
		    "no-poll\n"
		    "bogus-priv\n"
		    "interface=%s\n"
		    "listen-address=%s\n"
		    "bind-dynamic\n"
		    "no-negcache\n"
		    "clear-on-reload\n",
		    SYS_USER_NOBODY,
		    resolv_conf,
		    IFNAME_BR,
		    ipaddr);

	if (!is_ap_mode) {
		is_dns_used = 1;
		fprintf(fp, "cache-size=%d\n", DNS_RELAY_CACHE_MAX);
		fprintf(fp, "addn-hosts=%s/hosts\n", storage_dir);
	} else {
		is_dns_used = 0;
		fprintf(fp, "cache-size=%d\n", 0);
		fprintf(fp, "port=%d\n", 0);
	}

	if (strlen(domain) > 1) {
		fprintf(fp, "domain=%s\n"
			    "expand-hosts\n", domain);
	}

	is_dhcp_used = 0;
	if (i_dhcp_enable) {
		snprintf(dhcp_start, sizeof(dhcp_start), "%s", nvram_safe_get("dhcp_start"));
		snprintf(dhcp_end, sizeof(dhcp_end), "%s", nvram_safe_get("dhcp_end"));
		
		if (!chk_valid_subnet_pool(ipaddr, dhcp_start, dhcp_end, netmask)) {
			nvram_set("dhcp_start", dhcp_start);
			nvram_set("dhcp_end", dhcp_end);
		}
		
		fprintf(fp, "dhcp-range=%s,%s,%s,%d\n", dhcp_start, dhcp_end, netmask, nvram_get_int("dhcp_lease"));
		
		/* GATEWAY */
		gw = nvram_safe_get("dhcp_gateway_x");
		if (!is_valid_ipv4(gw))
			gw = (!is_ap_mode) ? ipaddr : NULL;
		if (gw)
			fprintf(fp, "dhcp-option=%d,%s\n", 3, gw);
		
		/* DNS server */
		memset(dns_all, 0, sizeof(dns_all));
		dns1 = nvram_safe_get("dhcp_dns1_x");
		dns2 = nvram_safe_get("dhcp_dns2_x");
		dns3 = nvram_safe_get("dhcp_dns3_x");
		
		if (is_valid_ipv4(dns1))
			strcat(dns_all, dns1);
		if (is_valid_ipv4(dns2) && (strcmp(dns2, dns1))) {
			if (strlen(dns_all) > 0)
				strcat(dns_all, ",");
			strcat(dns_all, dns2);
		}
		if (is_valid_ipv4(dns3) && (strcmp(dns3, dns1) && strcmp(dns3, dns2))) {
			if (strlen(dns_all) > 0)
				strcat(dns_all, ",");
			strcat(dns_all, dns3);
		}
		if (strlen(dns_all) == 0 && !is_ap_mode)
			strcat(dns_all, ipaddr);
		if (strlen(dns_all) > 0)
			fprintf(fp, "dhcp-option=%d,%s\n", 6, dns_all);
		
		/* DOMAIN search */
		if (strlen(domain) > 1)
			fprintf(fp, "dhcp-option=%d,%s\n", 15, domain);
		
		/* WINS */
		wins = nvram_safe_get("dhcp_wins_x");
		if (is_valid_ipv4(wins))
			fprintf(fp, "dhcp-option=%d,%s\n", 44, wins);
#if defined(APP_SMBD) || defined(APP_NMBD)
		else if (nvram_get_int("wins_enable"))
			fprintf(fp, "dhcp-option=%d,%s\n", 44, ipaddr);
#endif
		
		if (i_verbose == 0 || i_verbose == 2)
			fprintf(fp, "quiet-dhcp\n");
		
		if (i_ethers)
			fprintf(fp, "read-ethers\n");
		
		is_dhcp_used = 1;
	}

#if defined (USE_IPV6)
	if (!is_ap_mode && is_lan_radv_on() == 1) {
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
			
			is_dhcp_used = 1;
		}
	}
#endif
	if (is_dhcp_used) {
		fprintf(fp, "dhcp-leasefile=%s\n", leases_dhcp);
		fprintf(fp, "dhcp-authoritative\n");
	}

	fprintf(fp, "conf-file=%s/dnsmasq.conf\n", storage_dir);
	fclose(fp);

	if (is_dns_used || is_dhcp_used)
		return eval("/usr/sbin/dnsmasq");

	return 0;
}

void
stop_dns_dhcpd(void)
{
	char* svcs[] = { "dnsmasq", NULL };

	kill_services(svcs, 3, 1);

	arpbind_clear();
}

int
restart_dhcpd(void)
{
	stop_dns_dhcpd();
	return start_dns_dhcpd(get_ap_mode());
}

int
restart_dns(void)
{
	if (!is_dns_dhcpd_run())
		return -1;

	return doSystem("killall %s %s", "-SIGHUP", "dnsmasq");
}

///////////////////////////////////////////////////////////////////////
// WINS (nmbd)
///////////////////////////////////////////////////////////////////////

#if defined(APP_SMBD) || defined(APP_NMBD)

FILE *
write_smb_conf_header(void)
{
	FILE *fp;
	int i_lmb, i_wins_enable;
	char *p_computer_name, *p_workgroup, *p_res_order;

	unlink(SAMBA_CONF);

	i_lmb = nvram_get_int("st_samba_lmb");
	i_wins_enable = nvram_get_int("wins_enable");
	p_workgroup = nvram_safe_get("st_samba_workgroup");
	p_computer_name = get_our_hostname();

	if (!(fp = fopen(SAMBA_CONF, "w")))
		return NULL;

	p_res_order = "lmhosts hosts bcast";

	fprintf(fp, "[global]\n");
	if (strlen(p_workgroup) > 0)
		fprintf(fp, "workgroup = %s\n", p_workgroup);
	fprintf(fp, "netbios name = %s\n", p_computer_name);
	fprintf(fp, "server string = %s\n", p_computer_name);

	if (i_lmb == 0) {
		fprintf(fp, "local master = %s\n", "no");
	} else if (i_lmb == 1) {
		fprintf(fp, "local master = %s\n", "yes");
		fprintf(fp, "os level = %d\n", 128);
	} else {
		fprintf(fp, "local master = %s\n", "yes");
		fprintf(fp, "domain master = %s\n", "yes");
		fprintf(fp, "preferred master = %s\n", "yes");
		fprintf(fp, "os level = %d\n", 128);
	}

	if (i_wins_enable) {
		fprintf(fp, "wins support = %s\n", "yes");
	} else {
		if (is_dhcpd_enabled(get_ap_mode())) {
			char *wins = nvram_safe_get("dhcp_wins_x");
			if (is_valid_ipv4(wins)) {
				fprintf(fp, "wins server = %s\n", wins);
				p_res_order = "lmhosts wins hosts bcast";
			}
		}
	}

	fprintf(fp, "name resolve order = %s\n", p_res_order);
	fprintf(fp, "log file = %s\n", "/var/log/samba.log");
	fprintf(fp, "log level = 0\n");
	fprintf(fp, "max log size = 5\n");
	fprintf(fp, "socket options = TCP_NODELAY SO_KEEPALIVE\n");
	fprintf(fp, "unix charset = UTF8\n");
	fprintf(fp, "display charset = UTF8\n");
	fprintf(fp, "bind interfaces only = %s\n", "yes");
	fprintf(fp, "interfaces = %s\n", IFNAME_BR);
	fprintf(fp, "unix extensions = no\n");			// fix for MAC users (thanks to 'mark2qualis')
	fprintf(fp, "encrypt passwords = yes\n");
	fprintf(fp, "pam password change = no\n");
	fprintf(fp, "obey pam restrictions = no\n");
	fprintf(fp, "host msdfs = no\n");
	fprintf(fp, "disable spoolss = yes\n");

	return fp;
}

void
stop_wins(void)
{
	char* svcs[] = { "nmbd", NULL };
	kill_services(svcs, 3, 1);
}

void
start_wins(void)
{
	FILE *fp;

	if (nvram_match("wins_enable", "0"))
		return;

	mkdir_if_none("/etc/samba");

	fp = write_smb_conf_header();
	if (fp)
		fclose(fp);

	eval("/sbin/nmbd", "-D", "-s", "/etc/smb.conf");
}

void
reload_wins(void)
{
	if (pids("nmbd"))
		doSystem("killall %s %s", "-SIGHUP", "nmbd");
}

void
restart_wins(void)
{
	stop_wins();
#if defined(APP_SMBD)
	if (pids("smbd")) {
		write_smb_conf();
		eval("/sbin/nmbd", "-D", "-s", "/etc/smb.conf");
		doSystem("killall %s %s", "-SIGHUP", "smbd");
	} else
#endif
	start_wins();
}
#endif

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
	char lan_class[24];
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
	ip2class(lan_addr, lan_mask, lan_class, sizeof(lan_class));
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
check_upnp_wanif_changed(char *wan_ifname)
{
	if (!is_upnp_run())
		return;

	/* restart miniupnpd only if wan interface changed */
	if (strcmp(wan_ifname, get_wan_unit_value(0, "ifname_t")) != 0)
		stop_upnp();
}

void
update_upnp(void)
{
	if (!is_upnp_run()) {
		start_upnp();
		return;
	}

	/* update upnp forwards from lease file */
	if (check_if_file_exist(UPNPD_LEASE_FILE)) {
		doSystem("killall %s %s", "-SIGUSR1", "miniupnpd");
	}
}

void
restart_upnp(void)
{
	stop_upnp();

	/* restart_firewall call update_upnp */
	restart_firewall();
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
	{ "WWW.NIC.RU",           "default@nic.ru"             },
	{ "WWW.DUCKDNS.ORG",      "default@duckdns.org"        },
	{ "WWW.TUNNELBROKER.NET", "ipv6tb@he.net"              },
	{ "DNS.HE.NET",           "dyndns@he.net"              },
	{ "TB.NETASSIST.UA",      "ipv6tb@netassist.ua"        },
	{ "IPV4.NSUPDATE.INFO",   "ipv4@nsupdate.info"         },
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
	"ipv4.nsupdate.info /myip",
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
		
#if defined (BOARD_N14U) || defined (BOARD_N11P)
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

	if (use_delay)
		use_delay = (nvram_get_int("ntpc_counter") < 1) ? 15 : 3;

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
		
		load_user_config(fp, INADYN_USER_DIR, "inadyn.conf", NULL);
		
		fclose(fp);
		
		return 1;
	}

	return 0;
}

int
start_ddns(int clear_cache)
{
	if (get_ap_mode())
		return -1;

	stop_ddns();

	if (!nvram_match("ddns_enable_x", "1"))
		return -1;

	if (clear_cache)
		doSystem("rm -rf %s", DDNS_CACHE_DIR);

	mkdir(DDNS_CACHE_DIR, 0777);

	write_inadyn_conf(DDNS_CONF_FILE, (clear_cache) ? 0 : 1);

	return eval("/bin/inadyn", "--config", DDNS_CONF_FILE);
}

int
notify_ddns_update(void)
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

	if (get_ap_mode())
		return;

	if (!has_wan_ip4(0) || !has_wan_gw4()) {
		nvram_set_temp(nvram_key, "connect_fail");
		return;
	}

	stop_ddns();

#if defined (BOARD_N14U) || defined (BOARD_N11P)
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

