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
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>

#include "rc.h"

#define DHCPD_STATIC_MAX	64
#define DHCPD_MULTIMAC_MAX	8
#define DHCPD_RANGE_DEF_TAG	"lan"
#define DHCPD_HOSTS_DIR		"/etc/dnsmasq/dhcp"
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
	char buffer[256], arp_ip[16], arp_if[32];
	unsigned int arp_flags;

	fp = fopen("/proc/net/arp", "r");
	if (fp) {
		// skip first line
		fgets(buffer, sizeof(buffer), fp);
		
		while (fgets(buffer, sizeof(buffer), fp)) {
			arp_flags = 0;
			if (sscanf(buffer, "%15s %*s 0x%x %*s %*s %31s", arp_ip, &arp_flags, arp_if) == 3) {
				if ((arp_flags & 0x04) && strcmp(arp_if, IFNAME_BR) == 0)
					doSystem("arp -i %s -d %s", IFNAME_BR, arp_ip);
			}
		}
		
		fclose(fp);
	}
}

struct ip4_items_t {
	in_addr_t ip4;
	int count;
	char *hname;
	char *mac[DHCPD_MULTIMAC_MAX];
};

static void
fill_static_ethers(const char *lan_ip, const char *lan_mask)
{
	FILE *fp[3];
	in_addr_t ip4, ip4_lan;
	struct ip4_items_t ip4_list[DHCPD_STATIC_MAX];
	int i, j, i_max_items, i_arp_bind;
	char dh_fn[64], nvram_key[32], *smac, *sip4, *shname;

	memset(ip4_list, 0, sizeof(ip4_list));

	mkdir_if_none(DHCPD_HOSTS_DIR, "755");
	snprintf(dh_fn, sizeof(dh_fn), "%s/dhcp-hosts.rc", DHCPD_HOSTS_DIR);

	fp[0] = fopen(dh_fn, "w+");
	fp[1] = fopen("/etc/ethers", "w+");
	fp[2] = fopen("/tmp/hosts.static", "w+");

	if (nvram_get_int("dhcp_static_x") == 1) {
		ip4_lan = inet_addr_safe(lan_ip);
		i_arp_bind = nvram_get_int("dhcp_static_arp");
		i_max_items = nvram_get_int("dhcp_staticnum_x");
		if (i_max_items > DHCPD_STATIC_MAX)
			i_max_items = DHCPD_STATIC_MAX;
		
		/* first pass */
		for (j = 0; j < i_max_items; j++) {
			snprintf(nvram_key, sizeof(nvram_key), "dhcp_staticmac_x%d", j);
			smac = nvram_safe_get(nvram_key);
			if (strlen(smac) != 12)
				continue;
			snprintf(nvram_key, sizeof(nvram_key), "dhcp_staticip_x%d", j);
			sip4 = nvram_safe_get(nvram_key);
			ip4 = inet_addr_safe(sip4);
			if (ip4 == INADDR_ANY || ip4 == INADDR_NONE || ip4 == ip4_lan)
				continue;
			if (!is_same_subnet(sip4, lan_ip, lan_mask))
				continue;
			snprintf(nvram_key, sizeof(nvram_key), "dhcp_staticname_x%d", j);
			shname = nvram_safe_get(nvram_key);
			
			/* fill multi-items array by IP */
			for (i = 0; i < DHCPD_STATIC_MAX; i++) {
				struct ip4_items_t *ipl = &ip4_list[i];
				
				if (!ipl->ip4) {
					ipl->ip4 = ip4;
					ipl->count = 1;
					ipl->mac[0] = strdup(smac);
					if (is_valid_hostname(shname))
						ipl->hname = strdup(shname);
					break;
				}
				if (ipl->ip4 == ip4) {
					if (ipl->count < DHCPD_MULTIMAC_MAX) {
						ipl->mac[ipl->count] = strdup(smac);
						ipl->count++;
					}
					if (!ipl->hname && is_valid_hostname(shname))
						ipl->hname = strdup(shname);
					break;
				}
			}
		}
		
		/* second pass */
		for (i = 0; i < DHCPD_STATIC_MAX; i++) {
			struct ip4_items_t *ipl = &ip4_list[i];
			struct in_addr in;
			
			if (!ipl->ip4)
				break;
			
			in.s_addr = ipl->ip4;
			sip4 = inet_ntoa(in);
			if (fp[0] && ipl->count > 0) {
				for (j = 0; j < ipl->count; j++) {
					smac = ipl->mac[j];
					if (!smac)
						continue;
					fprintf(fp[0], "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c,",
						smac[0], smac[1], smac[2], smac[3], smac[4], smac[5],
						smac[6], smac[7], smac[8], smac[9], smac[10], smac[11]);
				}
				fprintf(fp[0], "set:%s,%s\n", DHCPD_RANGE_DEF_TAG, sip4);
			}
			
			/* use only unique IP for /etc/ethers (ARP binds) */
			if (i_arp_bind && ipl->count == 1 && ipl->mac[0]) {
				smac = ipl->mac[0];
				if (fp[1]) {
					fprintf(fp[1], "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c %s\n",
						smac[0], smac[1], smac[2], smac[3], smac[4], smac[5],
						smac[6], smac[7], smac[8], smac[9], smac[10], smac[11], sip4);
				}
				doSystem("arp -i %s -s %s %s", IFNAME_BR, sip4, smac);
			}
			
			if (fp[2] && ipl->hname)
				fprintf(fp[2], "%s %s\n", sip4, sanity_hostname(ipl->hname));
		}
		
		/* cleanup items */
		for (i = 0; i < DHCPD_STATIC_MAX; i++) {
			struct ip4_items_t *ipl = &ip4_list[i];
			
			for (j = 0; j < DHCPD_MULTIMAC_MAX; j++) {
				if (ipl->mac[j])
					free(ipl->mac[j]);
			}
			if (ipl->hname)
				free(ipl->hname);
		}
	}

	for (i = 0; i < ARRAY_SIZE(fp); i++) {
		if (fp[i])
			fclose(fp[i]);
	}
}

int
fill_dnsmasq_servers(void)
{
	FILE *fp;
	int lock;
	char word[256], *next, *wan_dns, *wan_dom;
	const char *storage_dir = "/etc/storage/dnsmasq";

	lock = file_lock("dservers");

	fp = fopen(DNS_SERVERS_FILE, "w");
	if (!fp) {
		file_unlock(lock);
		return -1;
	}

	/* add DNS servers (via specific domain) for static VPN client */
	if (nvram_get_int("vpnc_pdns") > 0) {
		wan_dom = nvram_safe_get("vpnc_dom_t");
		wan_dns = nvram_safe_get("vpnc_dns_t");
		if (strlen(wan_dom) > 0 && strlen(wan_dns) > 6) {
			foreach(word, wan_dns, next) {
				if (is_valid_ipv4(word))
					fprintf(fp, "server=/%s/%s\n", wan_dom, word);
			}
		}
	}

	/* add DNS servers (via specific domain) for MAN subnet */
	wan_dom = nvram_safe_get("wanx_domain");
	wan_dns = nvram_safe_get("wanx_dns");
	if (strlen(wan_dom) > 0 && strlen(wan_dns) > 6) {
		foreach(word, wan_dns, next) {
			if (is_valid_ipv4(word))
				fprintf(fp, "server=/%s/%s\n", wan_dom, word);
		}
	}

	/* fill from user dnsmasq.servers */
	//load_user_config(fp, storage_dir, "dnsmasq.servers", NULL);

	fclose(fp);

	file_unlock(lock);

	return 0;
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
	int i_verbose, i_dhcp_enable, is_dhcp_used, is_dns_used;
	char dhcp_start[32], dhcp_end[32], dns_all[64], dnsv6[40];
	char *ipaddr, *netmask, *gw, *dns1, *dns2, *dns3, *wins, *domain, *dns6;
	const char *storage_dir = "/etc/storage/dnsmasq";

	i_dhcp_enable = is_dhcpd_enabled(is_ap_mode);
	i_verbose = nvram_get_int("dhcp_verbose");

	ipaddr  = nvram_safe_get("lan_ipaddr");
	netmask = nvram_safe_get("lan_netmask");
	domain  = nvram_safe_get("lan_domain");

	/* touch dnsmasq.leases if not exist */
	create_file(DHCPD_LEASE_FILE);

	/* create /etc/ethers */
	fill_static_ethers(ipaddr, netmask);

	if (!is_ap_mode) {
		/* create /etc/hosts (run after fill_static_ethers!) */
		update_hosts_router(ipaddr);
		
		/* touch resolv.conf if not exist */
		create_file(DNS_RESOLV_CONF);
	}

	/* create /etc/dnsmasq.conf */
	if (!(fp = fopen("/etc/dnsmasq.conf", "w")))
		return errno;

	fprintf(fp, "user=%s\n"
		    "resolv-file=%s\n"
		    "no-poll\n"
		    "bogus-priv\n"
		    "no-negcache\n"
		    "clear-on-reload\n"
		    "bind-dynamic\n"
		    "interface=%s\n",
		    SYS_USER_NOBODY,
		    DNS_RESOLV_CONF,
		    IFNAME_BR);

	if (!is_ap_mode) {
		/* listen DNS queries from clients of VPN server */
		fprintf(fp, "listen-address=%s\n", ipaddr);
	}

	if (!is_ap_mode) {
		is_dns_used = 1;
		fprintf(fp, "min-port=%d\n", 4096);
		fprintf(fp, "cache-size=%d\n", DNS_RELAY_CACHE_MAX);
		fprintf(fp, "dns-forward-max=%d\n", DNS_RELAY_QUERIES_MAX);
		fprintf(fp, "addn-hosts=%s/hosts\n", storage_dir);
		fprintf(fp, "servers-file=%s\n", DNS_SERVERS_FILE);
		fprintf(fp, "dhcp-hostsfile=%s/dhcp.conf\n", storage_dir);
	} else {
		is_dns_used = 0;
		fprintf(fp, "cache-size=%d\n", 0);
		fprintf(fp, "port=%d\n", 0);
	}

	if (strlen(domain) > 0) {
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
		
		fprintf(fp, "dhcp-range=set:%s,%s,%s,%s,%d\n",
			DHCPD_RANGE_DEF_TAG, dhcp_start, dhcp_end, netmask, nvram_get_int("dhcp_lease"));
		
		/* GATEWAY */
		gw = nvram_safe_get("dhcp_gateway_x");
		if (!is_valid_ipv4(gw))
			gw = (!is_ap_mode) ? ipaddr : NULL;
		if (gw)
			fprintf(fp, "dhcp-option=tag:%s,%d,%s\n", DHCPD_RANGE_DEF_TAG, 3, gw);
		
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
			fprintf(fp, "dhcp-option=tag:%s,%d,%s\n", DHCPD_RANGE_DEF_TAG, 6, dns_all);
		
		/* DOMAIN search */
		if (strlen(domain) > 0)
			fprintf(fp, "dhcp-option=tag:%s,%d,%s\n", DHCPD_RANGE_DEF_TAG, 15, domain);
		
		/* WINS */
		wins = nvram_safe_get("dhcp_wins_x");
		if (is_valid_ipv4(wins))
			fprintf(fp, "dhcp-option=tag:%s,%d,%s\n", DHCPD_RANGE_DEF_TAG, 44, wins);
#if defined(APP_SMBD) || defined(APP_NMBD)
		else if (nvram_get_int("wins_enable"))
			fprintf(fp, "dhcp-option=tag:%s,%d,%s\n", DHCPD_RANGE_DEF_TAG, 44, ipaddr);
#endif
		if (i_verbose == 0 || i_verbose == 2)
			fprintf(fp, "quiet-dhcp\n");
		
		is_dhcp_used |= 0x1;
	}

#if defined (USE_IPV6)
	if (!is_ap_mode && is_lan_radv_on() == 1) {
		int i_dhcp6s_mode = get_lan_dhcp6s_mode();
		
		fprintf(fp, "enable-ra\n");
		if (i_verbose == 0 || i_verbose == 1)
			fprintf(fp, "quiet-ra\n");
		fprintf(fp, "ra-param=%s,%d,%d\n", IFNAME_BR, 30, 1800);
		
		is_dhcp_used |= 0x2;
		
		if (i_dhcp6s_mode == 0) {
			int i_pref_lifetime = 600;
			
			if (is_lan_addr6_static() == 1)
				i_pref_lifetime = 1800;
			
			/* Router Advertisement only, disable Stateful, disable SLAAC */
			fprintf(fp, "dhcp-range=set:%s,::,constructor:%s%s,%d,%d\n",
				DHCPD_RANGE_DEF_TAG, IFNAME_BR, ",ra-only,ra-names", 64, i_pref_lifetime);
		} else {
			int i_dhcp6s_irt = get_lan_dhcp6s_irt();
			
			if (i_dhcp6s_mode == 1) {
				fprintf(fp, "dhcp-range=set:%s,::,constructor:%s%s,%d,%d\n",
					DHCPD_RANGE_DEF_TAG, IFNAME_BR, ",ra-stateless,ra-names", 64, i_dhcp6s_irt);
			} else {
				const char *range_mode = "";
				int i_sflt = nvram_safe_get_int("ip6_lan_sflt", 1800, 120, 604800);
				int i_sfps = nvram_safe_get_int("ip6_lan_sfps", 4096, 2, 65534);
				int i_sfpe = nvram_safe_get_int("ip6_lan_sfpe", 4352, 2, 65534);
				
				if (i_sfpe < i_sfps)
					i_sfpe = i_sfps;
				
				if (i_dhcp6s_mode > 2)
					range_mode = ",slaac,ra-names";
				
				/* Enable Stateful, Enable/Disable SLAAC */
				fprintf(fp, "dhcp-range=set:%s,::%x,::%x,constructor:%s%s,%d,%d\n",
					DHCPD_RANGE_DEF_TAG, i_sfps, i_sfpe, IFNAME_BR, range_mode, 64, i_sflt);
			}
			
			/* DNS server */
			memset(dnsv6, 0, sizeof(dnsv6));
			dns6 = nvram_safe_get("dhcp_dnsv6_x");
			if (is_valid_ipv6(dns6))
				strcpy(dnsv6, dns6);
			else
				strcpy(dnsv6, "[::]");

			fprintf(fp, "dhcp-option=tag:%s,option6:%d,%s\n", DHCPD_RANGE_DEF_TAG, 23, dnsv6);
			
			/* DOMAIN search */
			if (strlen(domain) > 0)
				fprintf(fp, "dhcp-option=tag:%s,option6:%d,%s\n", DHCPD_RANGE_DEF_TAG, 24, domain);
			
			/* Information Refresh Time */
			fprintf(fp, "dhcp-option=tag:%s,option6:%d,%d\n", DHCPD_RANGE_DEF_TAG, 32, i_dhcp6s_irt);
			
			if (i_verbose == 0 || i_verbose == 1)
				fprintf(fp, "quiet-dhcp6\n");
			
			is_dhcp_used |= 0x1;
		}
	}
#endif

	if (is_dhcp_used & 0x1) {
		fprintf(fp, "dhcp-hostsfile=%s\n", DHCPD_HOSTS_DIR);
		fprintf(fp, "dhcp-leasefile=%s\n", DHCPD_LEASE_FILE);
		fprintf(fp, "dhcp-authoritative\n");
	}

#if (BOARD_NUM_USB_PORTS > 0)
	fprintf(fp, "tftp-no-fail\n");
#endif

	fprintf(fp, "conf-file=%s/dnsmasq.conf\n", storage_dir);
	fclose(fp);

	if (is_dns_used)
		fill_dnsmasq_servers();

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
#if defined (APP_SMBD36)
	fprintf(fp, "max protocol = SMB2\n");
	fprintf(fp, "passdb backend = smbpasswd\n");
	fprintf(fp, "security = USER\n");
	fprintf(fp, "username level = 8\n");
#endif
	return fp;
}

void
stop_nmbd(void)
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

	mkdir_if_none("/etc/samba", "777");

	fp = write_smb_conf_header();
	if (fp)
		fclose(fp);

	eval("/sbin/nmbd", "-D", "-s", "/etc/smb.conf");
}

void
reload_nmbd(void)
{
	if (pids("nmbd"))
		doSystem("killall %s %s", "-SIGHUP", "nmbd");
}

void
restart_nmbd(void)
{
	stop_nmbd();
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

	fprintf(fp, "# automatically generated\n");
	fprintf(fp,
		"ext_ifname=%s\n"
		"listening_ip=%s\n"
		"port=%d\n"
		"enable_upnp=%s\n"
		"enable_natpmp=%s\n"
		"upnp_forward_chain=%s\n"
		"upnp_nat_chain=%s\n"
		"upnp_nat_postrouting_chain=%s\n"
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
		MINIUPNPD_CHAIN_IP4_NAT_POST,
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

	if (nvram_get_int("wan_nat_x") == 0) {
		stop_upnp();
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
	{ "WWW.DYNDNS.ORG",       "default@dyndns.org"         },
	{ "WWW.ZONEEDIT.COM",     "default@zoneedit.com"       },
	{ "WWW.EASYDNS.COM",      "default@easydns.com"        },
	{ "WWW.NO-IP.COM",        "default@no-ip.com"          },
	{ "WWW.DNSOMATIC.COM",    "default@dnsomatic.com"      },
	{ "WWW.DNSEXIT.COM",      "default@dnsexit.com"        },
	{ "WWW.CHANGEIP.COM",     "default@changeip.com"       },
	{ "WWW.SITELUTIONS.COM",  "default@sitelutions.com"    },
	{ "WWW.DHIS.ORG",         "default@dhis.org"           },
	{ "WWW.DUCKDNS.ORG",      "default@duckdns.org"        },
	{ "WWW.OVH.COM",          "default@ovh.com"            },
	{ "WWW.LOOPIA.COM",       "default@loopia.com"         },
	{ "WWW.DUIADNS.NET",      "default@duiadns.net"        },
	{ "WWW.TUNNELBROKER.NET", "default@tunnelbroker.net"   },
	{ "DNS.HE.NET",           "dyndns@he.net"              },
	{ "DDNSS.DE",             "default@ddnss.de"           },
	{ "HOMESERVER.GIRA.DE",   "default@gira.de"            },
	{ "DOMAINS.GOOGLE.COM",   "default@domains.google.com" },
	{ "IPV4.DYNV6.COM",       "default@ipv4.dynv6.com"     },
	{ "DYNV6.COM",            "default@dynv6.com"          },
	{ "TB.NETASSIST.UA",      "ipv6tb@netassist.ua"        },
	{ "IPV4.NSUPDATE.INFO",   "ipv4@nsupdate.info"         },
	{ "FREEDNS.AFRAID.ORG",   "default@freedns.afraid.org" },
	{ "FREEMYIP.COM",         "default@freemyip.com"       },
	{ "SPDYN.DE",             "default@spdyn.de"           },
	{ "WWW.STRATO.DE",        "default@strato.com"         },
	{ "CLOUDXNS.NET",         "default@cloudxns.net"       },
	{ "3322.ORG",             "dyndns@3322.org"            },
	{ "DNSPOD.CN",            "default@dnspod.cn"          },
	{ "DYNU.COM",             "default@dynu.com"           },
	{ "SELFHOST.DE",          "default@selfhost.de"        },
	{ "PDD.YANDEX.RU",        "default@pdd.yandex.ru"      },
	{ "CLOUDFLARE.COM",       "default@cloudflare.com"     },
	{ "ORAY.COM",             "default@oray.com"           },
	{ "CUSTOM",               "custom"                     },
	{ NULL, NULL }
};

static const struct _inadyn_checkip_url {
	const char* server;
	const char* path;
	bool  ssl;
} checkip_urls[] = {
	{ "",                         ""      , false },
	{ "checkip.dyndns.org",       "/"     , false },	/* v4 only */
	{ "checkip.dyndns.org:8245",  "/"     , false },	/* v4 only */
	{ "ip.dnsexit.com",           "/"     , false },	/* v4 only */
	{ "ip.changeip.com",          "/"     , true  },	/* v4 only */
	{ "myip.dnsomatic.com",       "/"     , true  },	/* v4 only */
	{ "ip1.dynupdate.no-ip.com",  "/"     , false },	/* v4 only */
	{ "checkip.dns.he.net",       "/"     , true  },	/* dual stack */
	{ "checkip.two-dns.de",       "/"     , false },	/* v4 only */
	{ "ip.3322.net",              "/"     , false },	/* v4 only */
	{ "myip.ipip.net",            "/s"    , true  },	/* v4 only */
	{ "api.myip.la",              "/"     , true  },	/* dual stack */
	{ "ipv4.wtfismyip.com",       "/text" , true  },
	{ "ipv6.wtfismyip.com",       "/text" , true  },
	{ "ipv4.nsupdate.info",       "/myip" , true  },
	{ "ipv6.nsupdate.info",       "/myip" , true  },
	{ "api-ipv4.ip.sb",           "/ip"   , true  },
	{ "api-ipv6.ip.sb",           "/ip"   , true  },
	{ "v4.ident.me",              "/"     , true  },
	{ "v6.ident.me",              "/"     , true  },
	{ "api4.my-ip.io",            "/ip"   , true  },
	{ "api6.my-ip.io",            "/ip"   , true  },
	{ "api.ipify.org",            "/"     , true  },
	{ "api6.ipify.org",           "/"     , true  },
	{ "ipv4.duiadns.net",         "/"     , true  },
	{ "ipv6.duiadns.net",         "/"     , true  },
	{ "checkip4.spdyn.de",        "/"     , false },
	{ "checkip6.spdyn.de",        "/"     , false },
	{ "v4.ipv6-test.com",         "/api/myip.php", true },
	{ "v6.ipv6-test.com",         "/api/myip.php", true },
};

static const char *
get_inadyn_system(const char *alias)
{
	const struct inadyn_system_t *inadyn;
	const char *system = NULL;

	for (inadyn = &inadyn_systems[0]; inadyn->alias; inadyn++) {
		if (strcmp(alias, inadyn->alias) == 0) {
			system = inadyn->system;
			break;
		}
	}

	return system;
}

static int
write_inadyn_conf(const char *conf_file)
{
	FILE *fp;
	int i_max, i_ddns_source, i_ddns_period, i_ddns_forced, i_ddns_ipv6;
	int i_ddns1_ssl, i_ddns1_checkip;
	int i_ddns2_ssl, i_ddns2_checkip;
	char *ddns1_hname[3], *ddns1_user, *ddns1_pass, *ddns1_svr, *ddns1_url;
	char *ddns2_hname,    *ddns2_user, *ddns2_pass;
	const char *ddns1_svc, *ddns2_svc;
	char wan_ifname[16];

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

	i_ddns_ipv6 = nvram_get_int("ddns_ipv6");

	i_max = ARRAY_SIZE(checkip_urls) - 1;
	i_ddns1_checkip = nvram_safe_get_int("ddns_checkip", 0, 0, i_max);
	i_ddns2_checkip = nvram_safe_get_int("ddns2_checkip", 0, 0, i_max);

	i_ddns1_ssl = nvram_get_int("ddns_ssl");
	i_ddns2_ssl = nvram_get_int("ddns2_ssl");

	ddns1_svc = get_inadyn_system(nvram_safe_get("ddns_server_x"));
	if (!ddns1_svc) {
		ddns1_svc = inadyn_systems[0].system;
		nvram_set("ddns_server_x", inadyn_systems[0].alias);
	}

	ddns1_hname[0] = nvram_safe_get("ddns_hostname_x");
	ddns1_hname[1] = nvram_safe_get("ddns_hostname2_x");
	ddns1_hname[2] = nvram_safe_get("ddns_hostname3_x");
	ddns1_user     = nvram_safe_get("ddns_username_x");
	ddns1_pass     = nvram_safe_get("ddns_passwd_x");

	ddns1_svr = NULL;
	ddns1_url = NULL;

	if (strcmp(ddns1_svc, "custom") == 0) {
		ddns1_svr = nvram_safe_get("ddns_cst_svr");
		ddns1_url = nvram_safe_get("ddns_cst_url");
	}

	ddns2_svc = get_inadyn_system(nvram_safe_get("ddns2_server"));
	ddns2_hname = nvram_safe_get("ddns2_hname");
	ddns2_user  = nvram_safe_get("ddns2_user");
	ddns2_pass  = nvram_safe_get("ddns2_pass");

	fp = fopen(conf_file, "w");
	if (fp) {
		if (strlen(wan_ifname) > 0)
			fprintf(fp, "iface = %s\n", wan_ifname);
		fprintf(fp, "period = %d\n", i_ddns_period);
		fprintf(fp, "forced-update = %d\n", i_ddns_forced);
		fprintf(fp, "allow-ipv6 = %s\n", i_ddns_ipv6 ? "true" : "false");
		fprintf(fp, "secure-ssl = false\n");
		fprintf(fp, "broken-rtc = true\n");

		/* DDNS 1*/
		if (strcmp(ddns1_svc, "custom") == 0) {
			fprintf(fp, "custom %s:1 {\n", "http_srv_basic_auth");
		} else {
			fprintf(fp, "provider %s:1 {\n", ddns1_svc);
		}
		fprintf(fp, "  ssl = %s\n", i_ddns1_ssl ? "true" : "false");
		fprintf(fp, "  wildcard = %s\n", nvram_get_int("ddns_wildcard_x") ? "true" : "false");
		if (strlen(ddns1_user) > 0)
			fprintf(fp, "  username = \"%s\"\n", ddns1_user);
		if (strlen(ddns1_pass) > 0)
			fprintf(fp, "  password = \"%s\"\n", ddns1_pass);
		if (ddns1_svr && *ddns1_svr && ddns1_url && *ddns1_url) {
			/* custom ddns server */
			fprintf(fp, "  ddns-server = \"%s\"\n", ddns1_svr);
			fprintf(fp, "  ddns-path = \"/%s\"\n", ddns1_url);
		}
		if (i_ddns_source == 0 && strlen(checkip_urls[i_ddns1_checkip].server) > 0) {
			fprintf(fp, "  checkip-server = \"%s\"\n", checkip_urls[i_ddns1_checkip].server);
			fprintf(fp, "  checkip-path = \"%s\"\n", checkip_urls[i_ddns1_checkip].path);
			fprintf(fp, "  checkip-ssl = \"%s\"\n", checkip_urls[i_ddns1_checkip].ssl ? "true" : "false");
		}
		fprintf(fp, "  hostname = { \"%s\" }\n", ddns1_hname[0]);
		if (strlen(ddns1_hname[1]) > 2)
			fprintf(fp, "  hostname += { \"%s\" }\n", ddns1_hname[1]);
		if (strlen(ddns1_hname[2]) > 2)
			fprintf(fp, "  hostname += { \"%s\" }\n", ddns1_hname[2]);
		fprintf(fp, "}\n");

		/* DDNS 2*/
		if (ddns2_svc) {
			fprintf(fp, "provider %s:2 {\n", ddns2_svc);
			fprintf(fp, "  ssl = %s\n", nvram_get_int("ddns2_ssl") ? "true" : "false");
			fprintf(fp, "  wildcard = %s\n", nvram_get_int("ddns2_wildcard_x") ? "true" : "false");
			if (i_ddns_source == 0 && strlen(checkip_urls[i_ddns2_checkip].server) > 0) {
				fprintf(fp, "  checkip-server = \"%s\"\n", checkip_urls[i_ddns2_checkip].server);
				fprintf(fp, "  checkip-path = \"%s\"\n", checkip_urls[i_ddns2_checkip].path);
				fprintf(fp, "  checkip-ssl = \"%s\"\n", checkip_urls[i_ddns2_checkip].ssl ? "true" : "false");
			}
			if (strlen(ddns2_user) > 0)
				fprintf(fp, "  username = \"%s\"\n", ddns2_user);
			if (strlen(ddns2_pass) > 0)
				fprintf(fp, "  password = \"%s\"\n", ddns2_pass);
			fprintf(fp, "  hostname = \"%s\"\n", ddns2_hname);
			fprintf(fp, "}\n");
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
	int verbose_idx = nvram_safe_get_int("ddns_verbose", 0, 0, 2);
	char* ddns_verbose_str[] = { "err", "notice", "debug" };
	char* startup_delay;

	if (get_ap_mode())
		return -1;

	stop_ddns();

	if (!nvram_match("ddns_enable_x", "1"))
		return -1;

	if (clear_cache)
		doSystem("rm -rf %s", DDNS_CACHE_DIR);

	mkdir(DDNS_CACHE_DIR, 0777);

	write_inadyn_conf(DDNS_CONF_FILE);

	startup_delay = (!is_ntpc_updated()) ? "30" : "3";

	return eval("/bin/inadyn",
		"--no-pidfile",
		"-l", ddns_verbose_str[verbose_idx],
		"-t", startup_delay,
		"-f", DDNS_CONF_FILE,
		"--cache-dir", DDNS_CACHE_DIR,
		"-e", DDNS_DONE_SCRIPT,
		"--exec-mode", "compat");
}

int
notify_ddns_update(void)
{
	if (pids("inadyn")) {
		write_inadyn_conf(DDNS_CONF_FILE);
		return doSystem("killall %s %s", "-SIGHUP", "inadyn");
	}

	return start_ddns(0);
}

char *
get_ddns_fqdn(void)
{
	char *ddns_fqdn = NULL;
	const char *ddns_svc;

	if (!nvram_match("ddns_enable_x", "1"))
		return NULL;

	ddns_svc = get_inadyn_system(nvram_safe_get("ddns_server_x"));
	if (ddns_svc && strncmp(ddns_svc, "ipv6tb@", 7) != 0 && !strstr(ddns_svc, "tunnelbroker.net")) {
		ddns_fqdn = nvram_safe_get("ddns_hostname_x");
		if (strcmp(ddns_svc, "update@asus.com") == 0) {
			if (!strstr(ddns_fqdn, ".asuscomm.com"))
				ddns_fqdn = NULL;
		} else {
			if (!strchr(ddns_fqdn, '.')) {
				ddns_fqdn = nvram_safe_get("ddns_hostname2_x");
				if (!strchr(ddns_fqdn, '.')) {
					ddns_fqdn = nvram_safe_get("ddns_hostname3_x");
					if (!strchr(ddns_fqdn, '.'))
						ddns_fqdn = NULL;
				}
			}
		}
	}

	if (!ddns_fqdn) {
		ddns_svc = get_inadyn_system(nvram_safe_get("ddns2_server"));
		if (ddns_svc && strncmp(ddns_svc, "ipv6tb@", 7) != 0 && !strstr(ddns_svc, "tunnelbroker.net")) {
			ddns_fqdn = nvram_safe_get("ddns2_hname");
			if (!strchr(ddns_fqdn, '.'))
				ddns_fqdn = NULL;
		}
	}

	return ddns_fqdn;
}

void
manual_ddns_hostname_check(void)
{
	nvram_set_temp("ddns_return_code", "inadyn_unsupport");
}

