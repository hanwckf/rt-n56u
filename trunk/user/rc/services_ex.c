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
#include <sys/mount.h>
#include <nvram/bcmnvram.h>
#include <shutils.h>
#include <rc.h>
#include <syslog.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <sys/mount.h>
#include "dongles.h"

#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <scsi/scsi.h>
#include <scsi/scsi_ioctl.h>

#include <disk_io_tools.h>
#include <disk_initial.h>
#include <disk_share.h>

#include <ralink.h>

#include "rtl8367.h"

#define logs(s) syslog(LOG_NOTICE, s)

int file_to_buf(char *path, char *buf, int len)
{
	FILE *fp;

	memset(buf, 0 , len);

	if ((fp = fopen(path, "r"))) {
		fgets(buf, len, fp);
		fclose(fp);
		return 1;
	}

	return 0;
}

void
stop_infosvr(void)
{
	char* svcs[] = { "infosvr", NULL };
	kill_services(svcs, 3, 1);
}

void 
start_infosvr(void)
{
	eval("/usr/sbin/infosvr", IFNAME_BR);
}

void 
start_8021x_wl(void)
{
	if (nvram_match("wl_radio_x", "0"))
		return;
	
	if (	nvram_match("wl_auth_mode", "wpa") || 
		nvram_match("wl_auth_mode", "radius") || 
		nvram_match("wl_auth_mode", "wpa2") )
	{
		eval("rt2860apd");
	}
}

void 
stop_8021x_wl(void)
{
	char* svcs[] = { "rt2860apd", NULL };
	kill_services(svcs, 3, 1);
}

void 
start_8021x_rt(void)
{
	if (nvram_match("rt_radio_x", "0"))
		return;
	
	if (	nvram_match("rt_auth_mode", "wpa") || 
		nvram_match("rt_auth_mode", "radius") || 
		nvram_match("rt_auth_mode", "wpa2") )
	{
		eval("rtinicapd");
	}
}

void 
stop_8021x_rt(void)
{
	char* svcs[] = { "rtinicapd", NULL };
	kill_services(svcs, 3, 1);
}

int
chk_same_subnet(char *ip1, char *ip2, char *sub)
{
        unsigned int addr1, addr2, submask;

        addr1 = ntohl(inet_addr(ip1));
        addr2 = ntohl(inet_addr(ip2));
        submask = ntohl(inet_addr(sub));

        return (addr1 & submask) == (addr2 & submask);
}

void
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

int
chk_valid_startend(char *ip, char *ip1, char *ip2, char *sub)
{
        int result1, result2;

        result1 = chk_same_subnet(ip, ip1, sub);
        result2 = chk_same_subnet(ip, ip2, sub);

        if(!result1 || !result2)
        {
                simple_dhcp_range(ip, ip1, ip2, sub);
                return 0;
        }
        return 1;
}

int
start_dns_dhcpd(void)
{
	FILE *fp;
	int i, i_max, i_sdhcp, i_dns, is_use_dhcp;
	char dhcp_mac[32], dhcp_ip[32], *smac, *sip;
	char *start, *end, *ipaddr, *mask, *dns1, *dns2, *dns3;
	char dhcp_start[16], dhcp_end[16], lan_ipaddr[16], lan_netmask[16];
	size_t ethers = 0;
	char *resolv_conf = "/etc/resolv.conf";
	char *dmqext_conf = "/etc/storage/dnsmasq.conf";
	char *leases_dhcp = "/tmp/dnsmasq.leases";
	
	if (nvram_match("router_disable", "1"))
		return 0;
	
	ipaddr = nvram_safe_get("lan_ipaddr");
	
	i_sdhcp = nvram_get_int("dhcp_static_x");
	i_max  = nvram_get_int("dhcp_staticnum_x");
	if (i_max > 64) i_max = 64;
	
	/* create /etc/hosts */
	update_hosts();
	
	/* touch resolv.conf if not exist */
	fp = fopen(resolv_conf, "a+");
	if (fp)
		fclose(fp);
	
	/* touch dnsmasq.leases if not exist */
	fp = fopen(leases_dhcp, "a+");
	if (fp)
		fclose(fp);
	
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
	
	/* create /etc/dnsmasq.conf */
	if (!(fp = fopen("/etc/dnsmasq.conf", "w"))) {
		return errno;
	}
	
	fprintf(fp, "user=nobody\n"
		    "resolv-file=%s\n"
		    "no-poll\n"
		    "bogus-priv\n"
		    "addn-hosts=/etc/storage/hosts\n"
		    "interface=%s\n"
		    "listen-address=%s\n"
		    "bind-interfaces\n", resolv_conf, IFNAME_BR, ipaddr);
		
	if (nvram_invmatch("lan_domain", "")) {
		fprintf(fp, "domain=%s\n"
			    "expand-hosts\n", nvram_get("lan_domain"));
	}
	
	fprintf(fp, "no-negcache\n"
		    "cache-size=1000\n"
		    "clear-on-reload\n");
	
	is_use_dhcp = 0;
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
		if (nvram_invmatch("dhcp_gateway_x", ""))
			fprintf(fp, "dhcp-option=3,%s\n", nvram_safe_get("dhcp_gateway_x"));
		else
			fprintf(fp, "dhcp-option=3,%s\n", lan_ipaddr);
		
		/* DNS server */
		i_dns = 0;
		dns1 = nvram_safe_get("dhcp_dns1_x");
		dns2 = nvram_safe_get("dhcp_dns2_x");
		dns3 = nvram_safe_get("dhcp_dns3_x");
		fprintf(fp, "dhcp-option=6");
		if ((inet_addr_(dns1) != INADDR_ANY)) {
			i_dns++;
			fprintf(fp, ",%s", dns1);
		}
		if ((inet_addr_(dns2) != INADDR_ANY) && (strcmp(dns2, dns1))) {
			i_dns++;
			fprintf(fp, ",%s", dns2);
		}
		if ((inet_addr_(dns3) != INADDR_ANY) && (strcmp(dns3, dns1) && strcmp(dns3, dns2))) {
			i_dns++;
			fprintf(fp, ",%s", dns3);
		}
		if (i_dns < 1)
			fprintf(fp, ",%s", lan_ipaddr);
		fprintf(fp, "\n");
		
		/* DOMAIN search */
		if (nvram_invmatch("lan_domain", ""))
			fprintf(fp, "dhcp-option=15,%s\n", nvram_safe_get("lan_domain"));
		
		/* WINS */
		if (nvram_invmatch("dhcp_wins_x", ""))
			fprintf(fp, "dhcp-option=44,%s\n", nvram_safe_get("dhcp_wins_x"));
		
		if (ethers)
			fprintf(fp, "read-ethers\n");
		
		is_use_dhcp = 1;
	}
	
#if defined (USE_IPV6)
	if (is_lan_radv_on() == 1 && is_lan_dhcp6s_on() > 0) {
		/* Disable Stateful and SLAAC */
		fprintf(fp, "dhcp-range=%s,%s,static,%d\n", "::", "::", 0);
		/* DNS server */
		fprintf(fp, "dhcp-option=option6:23,[::]\n");
		/* DOMAIN search */
		if (nvram_invmatch("lan_domain", ""))
			fprintf(fp, "dhcp-option=option6:24,%s\n", nvram_safe_get("lan_domain"));
		/* Information Refresh Time */
		fprintf(fp, "dhcp-option=option6:32,%d\n", 600); // 10 min (IRT_MINIMUM=600)
		
		is_use_dhcp = 1;
	}
#endif
	if (is_use_dhcp) {
		fprintf(fp, "dhcp-leasefile=%s\n", leases_dhcp);
		fprintf(fp, "dhcp-authoritative\n");
	}
	
	if (check_if_file_exist(dmqext_conf))
		fprintf(fp, "conf-file=%s\n", dmqext_conf);
	
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
try_start_dns_dhcpd(void)
{
	if (!pids("dnsmasq"))
	{
		return start_dns_dhcpd();
	}
	
	return 1;
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
	return doSystem("killall -SIGHUP %s", "dnsmasq");
}

int
ddns_updated_main(int argc, char *argv[])
{
	FILE *fp;
	char buf[64], *ip;
	
	if (!(fp=fopen("/tmp/ddns.cache", "r"))) return 0;
	
	fgets(buf, sizeof(buf), fp);
	fclose(fp);
	
	if (!(ip=strchr(buf, ','))) return 0;
	
	nvram_set("ddns_ipaddr", ip+1);
	nvram_set("ddns_updated", "1");
	
	// Purge dnsmasq cache
	restart_dns();
	
	return 0;
}

void
stop_ddns(void)
{
	char* svcs[] = { "inadyn", NULL };
	
	kill_services(svcs, 3, 1);
}

int 
start_ddns(int forced)
{
	FILE *fp;
	int i_wildcard = 0;
	char *wan_ip, *ddns_ip, *ddns_srv, *ddns_hnm, *ddns_hnm2, *ddns_hnm3, *ddns_user, *ddns_pass;
	char service[32], mac_str[16];
	unsigned char mac_bin[ETHER_ADDR_LEN] = {0};
	
	if (nvram_match("router_disable", "1")) return -1;
	if (!nvram_match("ddns_enable_x", "1")) return -1;
	
	wan_ip = nvram_safe_get("wan_ipaddr_t");
	
	if (inet_addr_(wan_ip) == INADDR_ANY) return -1;
	
	ddns_ip       = nvram_safe_get("ddns_ipaddr");
	ddns_srv      = nvram_safe_get("ddns_server_x");
	ddns_hnm      = nvram_safe_get("ddns_hostname_x");
	ddns_hnm2     = nvram_safe_get("ddns_hostname2_x");
	ddns_hnm3     = nvram_safe_get("ddns_hostname3_x");
	
	logmessage(LOGNAME, "DDNS scheduler: Start update DDNS.");
	
	if ( (!forced) && (inet_addr_(wan_ip) == inet_addr_(ddns_ip)) && (nvram_match("ddns_updated", "1")) )
	{
		logmessage(LOGNAME, "DDNS scheduler: WAN IP address [%s] not changed since the last update. Skip update.", wan_ip);
		return 0;
	}
	
	stop_ddns();
	
	// delete ddns.cache for force update
	unlink("/tmp/ddns.cache");
	
	nvram_set("ddns_ipaddr", "");
	nvram_set("ddns_updated", "0");
	
	nvram_unset("ddns_server_x_old");
	nvram_unset("ddns_hostname_x_old");
	nvram_unset("bak_ddns_wildcard_x");
	nvram_unset("bak_ddns_enable_x");
	
	ddns_user = nvram_safe_get("ddns_username_x");
	ddns_pass = nvram_safe_get("ddns_passwd_x");
	
	if (nvram_match("ddns_wildcard_x", "1"))
		i_wildcard = 1;
	
	if (strcmp(ddns_srv, "WWW.DYNDNS.ORG") == 0)
		strcpy(service, "dyndns@dyndns.org");
	else if (strcmp(ddns_srv, "WWW.DYNDNS.ORG(CUSTOM)") == 0)
		strcpy(service, "custom@dyndns.org");
	else if (strcmp(ddns_srv, "WWW.DYNDNS.ORG(STATIC)") == 0)
		strcpy(service, "statdns@dyndns.org");
	else if (strcmp(ddns_srv, "WWW.TZO.COM") == 0)
		strcpy(service, "default@tzo.com");
	else if (strcmp(ddns_srv, "WWW.ZONEEDIT.COM") == 0)
		strcpy(service, "default@zoneedit.com");
	else if (strcmp(ddns_srv, "WWW.EASYDNS.COM") == 0)
		strcpy(service, "default@easydns.com");
	else if (strcmp(ddns_srv, "WWW.NO-IP.COM") == 0)
		strcpy(service, "default@no-ip.com");
	else if (strcmp(ddns_srv, "WWW.DNSOMATIC.COM") == 0)
		strcpy(service, "default@dnsomatic.com");
	else if (strcmp(ddns_srv, "WWW.TUNNELBROKER.NET") == 0)
		strcpy(service, "ipv6tb@he.net");
	else if (strcmp(ddns_srv, "DNS.HE.NET") == 0)
		strcpy(service, "dyndns@he.net");
	else if (strcmp(ddns_srv, "FREEDNS.AFRAID.ORG") == 0)
		strcpy(service, "default@freedns.afraid.org");
	else if (strcmp(ddns_srv, "WWW.ASUS.COM") == 0) {
		strcpy(service, "update@asus.com");
		nvram_unset("ddns_return_code");
		ether_atoe(nvram_safe_get("il1macaddr"), mac_bin);
		ddns_user = ether_etoa3(mac_bin, mac_str);
		ddns_pass = nvram_safe_get("secret_code");
		ddns_hnm2 = "";
		ddns_hnm3 = "";
		i_wildcard = 0;
	}
	else
		strcpy(service, "default@dyndns.org");
	
	fp = fopen("/etc/inadyn.conf", "w");
	if (fp) {
		if (strcmp(service, "default@freedns.afraid.org") == 0) {
			fprintf(fp,
				"dyndns_system %s\n"
				"alias %s\n",
				service, ddns_hnm);
		} else {
			fprintf(fp,
				"dyndns_system %s\n"
				"username %s\n"
				"password %s\n"
				"alias %s\n",
				service, ddns_user, ddns_pass, ddns_hnm);
		}
		
		if (*ddns_hnm2)
			fprintf(fp, "alias %s\n", ddns_hnm2);
		
		if (*ddns_hnm3)
			fprintf(fp, "alias %s\n", ddns_hnm3);
		
		if (i_wildcard)
			fprintf(fp, "wildcard\n");
		
		fprintf(fp,
			"background\n"
			"verbose 0\n"
			"iterations 1\n"
			"cache_file /tmp/ddns.cache\n"
			"exec /sbin/ddns_updated\n");
		
		fclose(fp);
	}
	
	eval("inadyn", "--input_file", "/etc/inadyn.conf");
	
	return 0;
}


void
stop_syslogd()
{
	char* svcs[] = { "syslogd", NULL };
	kill_services(svcs, 3, 1);
}

void
stop_klogd()
{
	char* svcs[] = { "klogd", NULL };
	kill_services(svcs, 3, 1);
}

int 
start_syslogd()
{
	time_zone_x_mapping();
	setenv_tz();
	
	if (nvram_invmatch("log_ipaddr", ""))
	{
		return eval("/sbin/syslogd", "-b0", "-s256", "-S", "-O", "/tmp/syslog.log", "-R", nvram_safe_get("log_ipaddr"), "-L");
	}
	else
	{
		return eval("/sbin/syslogd", "-b0", "-s256", "-S", "-D", "-O", "/tmp/syslog.log");
	}
}

int
start_klogd()
{
	return eval("/sbin/klogd");
}

void
stop_misc(int stop_watchdog)
{
	char* svcs[] = {"ntpd",
			"tcpcheck", 
			"detect_wan", 
			NULL,
			NULL
			};
	
	if (stop_watchdog)
		svcs[3] = "watchdog";
	
	kill_services(svcs, 3, 1);
}

int
load_usb_printer_module(void)
{
	return system("modprobe -q usblp");
}

int
load_usb_storage_module(void)
{
	return system("modprobe -q usb-storage");
}


int
check_if_dir_exist(const char *dirpath)
{
	struct stat stat_buf;

	if (!stat(dirpath, &stat_buf))
		return S_ISDIR(stat_buf.st_mode);
	else
		return 0;
}

int
check_if_file_exist(const char *filepath)
{
	struct stat stat_buf;

	if (!stat(filepath, &stat_buf))
		return S_ISREG(stat_buf.st_mode);
	else
		return 0;
}

int
mkdir_if_none(char *dir)
{
	DIR *dp;
	if (!(dp=opendir(dir)))
	{
		umask(0000);
		return !mkdir(dir, 0777);
	}
	closedir(dp);
	return 0;
}

int rename_if_dir_exist(const char *dir, const char *subdir)
{
	DIR *dirp;
	struct dirent *direntp;
	char oldpath[64], newpath[64];

	if (!dir || !subdir)
		return 0;

	if ((dirp = opendir(dir)))
	{
		while (dirp && (direntp = readdir(dirp)))
		{
			if (!strcasecmp(direntp->d_name, subdir) && strcmp(direntp->d_name, subdir))
			{
				sprintf(oldpath, "%s/%s", dir, direntp->d_name);
				sprintf(newpath, "%s/%s", dir, subdir);
				rename(oldpath, newpath);
				return 1;
			}
		}

		closedir(dirp);
	}

	return 0;
}

char *if_dircase_exist(const char *dir, const char *subdir)
{
	DIR *dirp;
	struct dirent *direntp;
	char oldpath[64];

	if (!dir || !subdir)
		return NULL;

	if ((dirp = opendir(dir)))
	{
		while (dirp && (direntp = readdir(dirp)))
		{
			if (!strcasecmp(direntp->d_name, subdir) && strcmp(direntp->d_name, subdir))
			{
				sprintf(oldpath, "%s/%s", dir, direntp->d_name);
				return strdup(oldpath);
			}
		}

		closedir(dirp);
	}

	return NULL;
}

unsigned long
file_size(const char *filepath)
{
	struct stat stat_buf;

	if (!stat(filepath, &stat_buf) && S_ISREG(stat_buf.st_mode))
		return ((unsigned long) stat_buf.st_size);
	else
		return 0;
}

int
safe_remove_usb_mass(int port)
{
	char *usb_dev_type;
	int needed_restart_usb_apps = 0;
	
	if (port == 0 || port == 1)
	{
		usb_dev_type = nvram_safe_get("usb_path1");
		if ( !strcmp(usb_dev_type, "modem") )
		{
			if ( get_usb_modem_state() ) 
			{
				safe_remove_usb_modem();
				if (port == 1)
					try_wan_reconnect(0);
			}
		}
		else if ( !strcmp(usb_dev_type, "storage") )
		{
			stop_usb_apps();
			umount_usb_path(1);
			umount_ejected();
			needed_restart_usb_apps = 1;
			nvram_set("usb_path1_removed", "1");
		}
	}
	
	if (port == 0 || port == 2)
	{
		usb_dev_type = nvram_safe_get("usb_path2");
		if ( !strcmp(usb_dev_type, "modem") )
		{
			if ( get_usb_modem_state() )
			{
				safe_remove_usb_modem();
				if (port == 2)
					try_wan_reconnect(0);
			}
		}
		else if ( !strcmp(usb_dev_type, "storage") )
		{
			stop_usb_apps();
			umount_usb_path(2);
			umount_ejected();
			needed_restart_usb_apps = 1;
			nvram_set("usb_path2_removed", "1");
		}
	}
	
	if (port == 0)
	{
		umount_sddev_all();
	}
	
	if ((port == 1 || port == 2) && (needed_restart_usb_apps))
	{
		// restart apps if needed
		if (count_sddev_mountpoint())
			start_usb_apps();
	}
	
	return 0;
}

/* stop necessary services for firmware upgrade */
/* stopservice: for firmware upgarde */
/* stopservice 1: for button setup   */
int
stop_service_main(int argc, char *argv[])
{
	int type = 0;
	if (argc >= 2)
		type = atoi(argv[1]);

	dbg("stop service type: %d\n", type);

	if (type==1)
	{
		stop_usb();
		stop_httpd();
		stop_dns_dhcpd();
		system("killall udhcpc");
	}
	else
	{
		if (type==99)
		{
			nvram_set("reboot", "1");
			stop_misc(0);
		}
		else
		{
			stop_misc(1);
		}
		
		stop_services(0); // don't stop telnetd/sshd/vpn
		stop_usb();
		
		stop_igmpproxy("");
		
		stop_networkmap();
	}

	if (type==99)
	{
		sync();
		fput_int("/proc/sys/vm/drop_caches", 1);
		sleep(1);
	}

	return 0;
}


void manual_wan_disconnect(void)
{
	logmessage("wan", "perform manual disconnect");
	
	if (get_usb_modem_state()){
		if(nvram_match("modem_type", "3"))
			release_udhcpc_wan(0);
		else
			stop_wan_ppp();
	}
	else if (nvram_match("wan0_proto", "dhcp"))
	{	/* dhcp */
		release_udhcpc_wan(0);
	}
	else if (
	nvram_match("wan0_proto", "pptp")  ||
	nvram_match("wan0_proto", "pppoe") ||
	nvram_match("wan0_proto", "l2tp")
	)
	{	/* pptp, l2tp, pppoe */
		stop_wan_ppp();
	}
	else 	/* static */
	{
		stop_wan_static();
		update_wan_status(0);
	}
}

void manual_wan_connect(void)
{
	logmessage("wan", "perform manual connect");
	
	try_wan_reconnect(1);
}

void manual_ddns_hostname_check(void)
{
	char *ddns_hnm, *ddns_user, *ddns_pass, *nvram_key, *wan_ip;
	char mac_str[16];
	unsigned char mac_bin[ETHER_ADDR_LEN] = {0};

	nvram_key = "ddns_return_code";

	wan_ip = nvram_safe_get("wan_ipaddr_t");
	if (inet_addr_(wan_ip) == INADDR_ANY)
	{
		nvram_set(nvram_key, "connect_fail");
		return;
	}

	ether_atoe(nvram_safe_get("il1macaddr"), mac_bin);
	ddns_user = ether_etoa3(mac_bin, mac_str);
	ddns_pass = nvram_safe_get("secret_code");
	ddns_hnm  = nvram_safe_get("ddns_hostname_x");

	stop_ddns();

	nvram_set(nvram_key, "ddns_query");
	eval("inadyn", "--dyndns_system", "register@asus.com", "-u", ddns_user, "-p", ddns_pass, "-a", ddns_hnm, "--iterations", "1");
	if (nvram_match(nvram_key, "ddns_query"))
		nvram_set(nvram_key, "connect_fail");
}


void
start_u2ec(void)
{
	if (nvram_match("u2ec_enable", "0")) 
		return;
	
	unlink("/var/run/u2ec.pid");
	system("/usr/sbin/u2ec &");
	nvram_set("apps_u2ec_ex", "1");
}

void
stop_u2ec(void)
{
	char* svcs[] = { "u2ec",  NULL };
	kill_services(svcs, 3, 1);
}

void 
start_lpd(void)
{
	if (nvram_match("lprd_enable", "0")) 
		return;
	
	unlink("/var/run/lpdparent.pid");
	system("/usr/sbin/lpd &");
}

void
stop_lpd(void)
{
	char* svcs[] = { "lpd", NULL };
	kill_services(svcs, 3, 1);
	
	unlink("/var/run/lpdparent.pid");
}

void 
start_p910nd(char *devlp)
{
	if (nvram_match("rawd_enable", "0")) 
		return;
	
	if (nvram_match("rawd_enable", "2"))
		eval("/usr/sbin/p910nd", "-b", "-f", devlp, "0");
	else
		eval("/usr/sbin/p910nd", "-f", devlp, "0");
}

void 
stop_p910nd(void)
{
	char* svcs[] = { "p910nd", NULL };
	kill_services(svcs, 3, 1);
}

int is_ftp_run(void)
{
	return (pids("vsftpd")) ? 1 : 0;
}

void stop_ftp(void)
{
	char* svcs[] = { "vsftpd", NULL };
	kill_services(svcs, 3, 1);
}

void stop_samba(void)
{
	char* svcs[] = { "smbd", "nmbd", NULL };
	kill_services(svcs, 5, 1);
}

void stop_nfsd(void)
{
	eval("/usr/bin/nfsd.sh", "stop");
}



void write_vsftpd_conf(void)
{
	FILE *fp;
	int i_maxuser, i_ftp_mode;

	fp=fopen("/etc/vsftpd.conf", "w");
	if (!fp) return;
	
	fprintf(fp, "listen%s=YES\n", 
#if defined (USE_IPV6)
	(get_ipv6_type() != IPV6_DISABLED) ? "_ipv6" :
#endif
	"");
	fprintf(fp, "background=YES\n");
	fprintf(fp, "connect_from_port_20=NO\n");
	fprintf(fp, "pasv_enable=YES\n");
	fprintf(fp, "pasv_min_port=50000\n");
	fprintf(fp, "pasv_max_port=50100\n");
	fprintf(fp, "ssl_enable=NO\n");
	fprintf(fp, "tcp_wrappers=NO\n");
	fprintf(fp, "isolate=NO\n");
	fprintf(fp, "isolate_network=NO\n");
	fprintf(fp, "use_sendfile=YES\n");

	i_ftp_mode = nvram_get_int("st_ftp_mode");
	if (i_ftp_mode == 1 || i_ftp_mode == 3) {
		fprintf(fp, "local_enable=NO\n");
		fprintf(fp, "anonymous_enable=YES\n");
		if (i_ftp_mode == 1){
			fprintf(fp, "anon_upload_enable=YES\n");
			fprintf(fp, "anon_mkdir_write_enable=YES\n");
			fprintf(fp, "anon_other_write_enable=YES\n");
			fprintf(fp, "anon_umask=000\n");
		}
	}
	else {
		fprintf(fp, "local_enable=YES\n");
		fprintf(fp, "local_umask=000\n");
		if (i_ftp_mode == 2)
			fprintf(fp, "anonymous_enable=NO\n");
		else
			fprintf(fp, "anonymous_enable=YES\n");
	}
	
	fprintf(fp, "nopriv_user=root\n");
	fprintf(fp, "write_enable=YES\n");
	fprintf(fp, "chroot_local_user=YES\n");
	fprintf(fp, "allow_writable_root=YES\n");
	fprintf(fp, "check_shell=NO\n");
	fprintf(fp, "xferlog_enable=NO\n");
	fprintf(fp, "syslog_enable=NO\n");
	fprintf(fp, "force_dot_files=YES\n");
	fprintf(fp, "dirmessage_enable=YES\n");
	fprintf(fp, "hide_ids=YES\n");
	fprintf(fp, "utf8=YES\n");
	fprintf(fp, "idle_session_timeout=600\n");

	i_maxuser = nvram_get_int("st_max_user");
	if (i_maxuser < 1) i_maxuser = 1;
	if (i_maxuser > MAX_CLIENTS_NUM) i_maxuser = MAX_CLIENTS_NUM;

	fprintf(fp, "max_clients=%d\n", i_maxuser);
	fprintf(fp, "max_per_ip=%d\n", i_maxuser);
	fprintf(fp, "ftpd_banner=Welcome to ASUS %s FTP service.\n", nvram_safe_get("productid"));
	
	fclose(fp);
}


void run_ftp(void)
{
	if (nvram_match("enable_ftp", "0")) 
		return;

	if (pids("vsftpd"))
		return;

	write_vsftpd_conf();

	eval("/sbin/vsftpd");

	if (pids("vsftpd"))
		logmessage("FTP server", "daemon is started");
}

void restart_ftp(void)
{
	int is_run_before = is_ftp_run();
	int is_run_after;

	stop_ftp();
	run_ftp();

	is_run_after = is_ftp_run();

	if (is_run_after && !is_run_before && nvram_match("ftpd_wopen", "1") && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}

#define SAMBA_CONF "/etc/smb.conf"

int check_existed_share(const char *string)
{
	FILE *tp;
	char buf[4096], target[256];

	if((tp = fopen(SAMBA_CONF, "r")) == NULL)
		return 0;

	if(string == NULL || strlen(string) <= 0)
		return 0;

	sprintf(target, "[%s]", string);

	while(fgets(buf, sizeof(buf), tp) != NULL){
		if(strstr(buf, target)){
			fclose(tp);
			return 1;
		}
	}

	fclose(tp);
	return 0;
}

int write_smb_conf(void) 
{
	FILE *fp;
	DIR *dir_to_open=NULL;
	int n=0, sh_num=0, i_maxuser;
	char *tmp1=NULL;
	char SMB_SHNAME[64];
	char SHNM[16];
	char SMB_SHCOMMENT[64];
	char SHCM[16];
	char SMB_SHPATH[104];
	char SHPH[32];
	char SHAU[16];
	char SMB_SHRRIGHT[384];
	char SHRR[384];
	char SMB_SHWRIGHT[384];
	char SHWR[384];
	char SMB_SHACCUSER[384];
	char *p_computer_name = NULL;
	
	disk_info_t *follow_disk, *disks_info = NULL;
	partition_info_t *follow_partition;
	
	unlink(SAMBA_CONF);
	
	if((fp = fopen(SAMBA_CONF, "w")) == NULL)
		goto confpage;
	
	fprintf(fp, "[global]\n");
	if (nvram_safe_get("st_samba_workgroup"))
		fprintf(fp, "workgroup = %s\n", nvram_safe_get("st_samba_workgroup"));

	p_computer_name = nvram_get("computer_name") && is_valid_hostname(nvram_get("computer_name")) ? nvram_get("computer_name") : nvram_safe_get("productid");
	if (p_computer_name) {
		fprintf(fp, "netbios name = %s\n", p_computer_name);
		fprintf(fp, "server string = %s\n", p_computer_name);
	}

	unlink("/var/log/samba.log");
	fprintf(fp, "log file = /var/log/samba.log\n");
	fprintf(fp, "log level = 0\n");
	fprintf(fp, "max log size = 5\n");
	
	/* share mode */
	if (!strcmp(nvram_safe_get("st_samba_mode"), "1") || !strcmp(nvram_safe_get("st_samba_mode"), "3")) {
		fprintf(fp, "security = SHARE\n");
		fprintf(fp, "guest ok = yes\n");
		fprintf(fp, "guest only = yes\n");
	}
	else if (!strcmp(nvram_safe_get("st_samba_mode"), "2") || !strcmp(nvram_safe_get("st_samba_mode"), "4")) {
		fprintf(fp, "security = USER\n");
		fprintf(fp, "guest ok = no\n");
		fprintf(fp, "map to guest = Bad User\n");
	}
	else{
		usb_dbg("samba mode: no\n");
		goto confpage;
	}
	
	fprintf(fp, "writeable = yes\n");
	fprintf(fp, "directory mode = 0777\n");
	fprintf(fp, "create mask = 0777\n");
	fprintf(fp, "force directory mode = 0777\n");
	
	/* max users */
	i_maxuser = nvram_get_int("st_max_user");
	if (i_maxuser < 1) i_maxuser = 1;
	if (i_maxuser > MAX_CLIENTS_NUM) i_maxuser = MAX_CLIENTS_NUM;

	fprintf(fp, "max connections = %d\n", i_maxuser);
	fprintf(fp, "encrypt passwords = yes\n");
	fprintf(fp, "pam password change = no\n");
	fprintf(fp, "socket options = TCP_NODELAY SO_KEEPALIVE\n");	// Padavan, fix buffers
	fprintf(fp, "obey pam restrictions = no\n");
	fprintf(fp, "use spnego = no\n");		// ASUS add
	fprintf(fp, "client use spnego = no\n");	// ASUS add
	fprintf(fp, "disable spoolss = yes\n");		// ASUS add
	fprintf(fp, "host msdfs = no\n");		// ASUS add
	fprintf(fp, "strict allocate = No\n");		// ASUS add
	fprintf(fp, "null passwords = yes\n");		// ASUS add
	fprintf(fp, "unix charset = UTF8\n");		// ASUS add
	fprintf(fp, "display charset = UTF8\n");	// ASUS add
	fprintf(fp, "bind interfaces only = yes\n");	// ASUS add
	fprintf(fp, "interfaces = lo %s\n", IFNAME_BR);
	fprintf(fp, "use sendfile = yes\n");
	fprintf(fp, "unix extensions = no\n");		// Padavan, fix for MAC users (thanks mark2qualis)

	fprintf(fp, "dos filemode = yes\n");
	fprintf(fp, "dos filetimes = yes\n");
	fprintf(fp, "dos filetime resolution = yes\n");

	fprintf(fp, "\n");

	disks_info = read_disk_data();
	if (disks_info == NULL) {
		usb_dbg("Couldn't get disk list when writing smb.conf!\n");
		goto confpage;
	}

	/* share */
	if (!strcmp(nvram_safe_get("st_samba_mode"), "0") || !strcmp(nvram_safe_get("st_samba_mode"), "")) {
		;
	}
	else if (!strcmp(nvram_safe_get("st_samba_mode"), "1")) {
		usb_dbg("samba mode: share\n");
		
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				char *mount_folder;
				
				if (follow_partition->mount_point == NULL)
					continue;
				
				mount_folder = strrchr(follow_partition->mount_point, '/')+1;
				
				fprintf(fp, "[%s]\n", mount_folder);
				fprintf(fp, "comment = %s's %s\n", follow_disk->tag, mount_folder);//*/
				fprintf(fp, "path = %s\n", follow_partition->mount_point);
				fprintf(fp, "guest ok = yes\n");
				fprintf(fp, "writeable = yes\n");
				fprintf(fp, "directory mode = 0777\n");
				fprintf(fp, "create mask = 0777\n");
			        fprintf(fp, "map archive = no\n");
			        fprintf(fp, "map hidden = no\n");
			        fprintf(fp, "map read only = no\n");
			        fprintf(fp, "map system = no\n");
			        fprintf(fp, "store dos attributes = yes\n");
			}
		}
	}
	else if (!strcmp(nvram_safe_get("st_samba_mode"), "2")) {
		usb_dbg("samba mode: user\n");
		n = 0;
		sh_num = nvram_get_int("sh_num");
		while (n < sh_num) {
			sprintf(SHPH, "sh_path%d", n);
			sprintf(SHNM, "sh_name%d", n);
			sprintf(SHRR, "sh_rright%d", n);
			sprintf(SHWR, "sh_wright%d", n);
			sprintf(SHCM, "sh_comment%d", n);
			sprintf(SHAU, "sh_acc_user%d", n);
			sprintf(SMB_SHPATH, "%s%s", POOL_MOUNT_ROOT, nvram_safe_get(SHPH));
			sprintf(SMB_SHNAME, "%s", nvram_safe_get(SHNM));
			sprintf(SMB_SHRRIGHT, "%s", nvram_safe_get(SHRR));
			sprintf(SMB_SHWRIGHT, "%s", nvram_safe_get(SHWR));
			sprintf(SMB_SHACCUSER, "%s", nvram_safe_get(SHAU));
			
			while ((tmp1 = strchr(SMB_SHRRIGHT, ';')) != NULL)
				memcpy(tmp1, " ", 1);
			
			memcpy(SMB_SHRRIGHT+strlen(SMB_SHRRIGHT)-1, "\0", 1);
			while ((tmp1=strchr(SMB_SHWRIGHT, ';')) != NULL)
				memcpy(tmp1, " ", 1);
			
			memcpy(SMB_SHWRIGHT+strlen(SMB_SHWRIGHT)-1, "\0", 1);
			while ((tmp1=strchr(SMB_SHACCUSER, ';')) != NULL)
				memcpy(tmp1, " ", 1);
			
			memcpy(SMB_SHACCUSER+strlen(SMB_SHACCUSER)-1, "\0", 1);
			sprintf(SMB_SHCOMMENT, "%s", nvram_safe_get(SHCM));
			
			/*write to conf*/
			if (!strcmp(SMB_SHNAME, "")) {
				goto endloop;
			}
			
			if (!(dir_to_open = opendir(SMB_SHPATH))) {
				goto endloop;
			}
			else
				closedir(dir_to_open);
			
			fprintf(fp, "[%s]\n", SMB_SHNAME);
			fprintf(fp, "comment = %s\n", SMB_SHCOMMENT);
			fprintf(fp, "path = %s\n", SMB_SHPATH);
			if (strstr(SMB_SHWRIGHT, "Guest")) {
				fprintf(fp, "guest ok = yes\n");
			}
			else{
				if (strstr(SMB_SHRRIGHT, "Guest")) {
					fprintf(fp, "guest ok = yes\n");
					fprintf(fp, "writeable = no\n");
					fprintf(fp, "write list = %s\n", SMB_SHWRIGHT);
				}
				else{
					if (!strcmp(SMB_SHWRIGHT, "")&&!strcmp(SMB_SHRRIGHT, ""))
						fprintf(fp, "valid users = _an_si_un_se_shorti_\n");
					else
						fprintf(fp, "valid users = %s\n", SMB_SHACCUSER);
					
					fprintf(fp, "writeable = no\n");
					fprintf(fp, "write list = %s\n", SMB_SHWRIGHT);
					fprintf(fp, "read list = %s\n", SMB_SHRRIGHT);
				}
			}
			
			fprintf(fp, "directory mode = 0777\n");
			fprintf(fp, "create mask = 0777\n");
			
			/*write to conf*/
endloop:
			n++;
		}
	}/* st_samba_mode = 2 */
	else if (!strcmp(nvram_safe_get("st_samba_mode"), "3")) {
		usb_dbg("samba mode: share\n");
		
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				if (follow_partition->mount_point == NULL)
					continue;
				
				char **folder_list;
				
				// 1. get the folder list
				if (get_folder_list_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list) < 0) {
					usb_dbg("Can't read the folder list in %s.\n", follow_partition->mount_point);
					free_2_dimension_list(&sh_num, &folder_list);
					continue;
				}
				
				// 2. start to get every share
				for (n = 0; n < sh_num; ++n) {
					fprintf(fp, "[%s]\n", folder_list[n]);
					fprintf(fp, "comment = %s\n", folder_list[n]);
					fprintf(fp, "path = %s/%s\n", follow_partition->mount_point, folder_list[n]);
					
					fprintf(fp, "writeable = yes\n");
					
					fprintf(fp, "directory mode = 0777\n");
					fprintf(fp, "create mask = 0777\n");
				}
				
				free_2_dimension_list(&sh_num, &folder_list);
			}
		}
	}
	else if (!strcmp(nvram_safe_get("st_samba_mode"), "4")) {
		usb_dbg("samba mode: user\n");
		
		int acc_num;
		char **account_list;
		
		// get the account list
		if (get_account_list(&acc_num, &account_list) < 0) {
			usb_dbg("Can't read the account list.\n");
			free_2_dimension_list(&acc_num, &account_list);
			goto confpage;
		}
			
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				if (follow_partition->mount_point == NULL)
					continue;
				
				char **folder_list;
				
				// 1. get the folder list
				if (get_folder_list_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list) < 0) {
					usb_dbg("Can't read the folder list in %s.\n", follow_partition->mount_point);
					free_2_dimension_list(&sh_num, &folder_list);
					continue;
				}
				
				// 2. start to get every share
				for (n = 0; n < sh_num; ++n) {
					int i, right, first;
					char share[256];
					
					memset(share, 0, 256);
					strcpy(share, folder_list[n]);
					
					fclose(fp);
					
					if(check_existed_share(share)){
						i = 1;
						memset(share, 0, 256);
						sprintf(share, "%s(%d)", folder_list[n], i);
						while(check_existed_share(share)){
							++i;
							memset(share, 0, 256);
							sprintf(share, "%s(%d)", folder_list[n], i);
						}
					}
					
					if((fp = fopen(SAMBA_CONF, "a")) == NULL)
						goto confpage;
					fprintf(fp, "[%s]\n", share);
					fprintf(fp, "comment = %s\n", folder_list[n]);
					fprintf(fp, "path = %s/%s\n", follow_partition->mount_point, folder_list[n]);
					fprintf(fp, "writeable = no\n");
					
					fprintf(fp, "valid users = ");
					first = 1;
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					fprintf(fp, "invalid users = ");
					first = 1;
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (right >= 1)
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					fprintf(fp, "read list = ");
					first = 1;
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (right < 1)
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
					
					fprintf(fp, "write list = ");
					first = 1;
					for (i = 0; i < acc_num; ++i) {
						right = get_permission(account_list[i], follow_partition->mount_point, folder_list[n], "cifs");
						if (right < 2)
							continue;
						
						if (first == 1)
							first = 0;
						else
							fprintf(fp, ", ");
						
						fprintf(fp, "%s", account_list[i]);
					}
					fprintf(fp, "\n");
				}
				
				free_2_dimension_list(&sh_num, &folder_list);
			}
		}
		
		free_2_dimension_list(&acc_num, &account_list);
	}
	
confpage:
	if(fp != NULL)
		fclose(fp);
	free_disk_data(&disks_info);
	return 0;
}

void run_samba()
{
	int sh_num=0, i;
	char tmpuser[40], tmp2[40];
	char cmd[256];

	if (nvram_match("enable_samba", "0") || nvram_match("st_samba_mode", "0")) 
		return;

	if (pids("smbd") && pids("nmbd"))
	{
		// reload smb.conf
		write_smb_conf();
		system("killall -SIGHUP smbd");
		system("killall -SIGHUP nmbd");
		return;
	}
	
	mkdir_if_none("/etc/samba");
	
	unlink("/etc/smb.conf");
	unlink("/etc/samba/smbpasswd");
	unlink("/etc/samba/secrets.tdb");
	
	recreate_passwd_unix(0);
	
	write_smb_conf();
	
	sh_num = nvram_get_int("acc_num");
	if (sh_num > MAX_ACCOUNT_NUM) sh_num = MAX_ACCOUNT_NUM;
	memset(tmpuser, 0, sizeof(tmpuser));
	memset(tmp2, 0, sizeof(tmp2));
	for (i=0; i<sh_num; i++)
	{
		sprintf(tmpuser, "acc_username%d", i);
		sprintf(tmp2, "acc_password%d", i);
		sprintf(cmd, "smbpasswd %s %s", nvram_safe_get(tmpuser), nvram_safe_get(tmp2));
		system(cmd);
	}
	
	eval("/sbin/nmbd", "-D", "-s", "/etc/smb.conf");
	eval("/sbin/smbd", "-D", "-s", "/etc/smb.conf");
	
	if (pids("smbd") && pids("nmbd"))
		logmessage("Samba Server", "daemon is started");
}

void write_nfsd_exports()
{
	FILE *procpt, *fp;
	char line[256], devname[32], mpname[128], system_type[16], mount_mode[128], acl_mask[64];
	int dummy1, dummy2;
	char *nfsmm, *lan_ipaddr, *lan_netmask;
	unsigned int acl_addr;
	struct in_addr ina;
	
	unlink("/etc/exports");
	
	fp = fopen("/etc/exports", "w");
	if (fp < 0)
		return;
	
	lan_ipaddr  = nvram_safe_get("lan_ipaddr_t");
	lan_netmask = nvram_safe_get("lan_netmask_t");
	if (!lan_ipaddr || !*lan_ipaddr)
		lan_ipaddr = nvram_safe_get("lan_ipaddr");
	if (!lan_netmask || !*lan_netmask)
		lan_netmask = nvram_safe_get("lan_netmask");
	if (!lan_ipaddr || !*lan_ipaddr)
		lan_ipaddr = "192.168.1.1";
	if (!lan_netmask || !*lan_netmask)
		lan_netmask = "255.255.255.0";
	
	acl_addr = ntohl(inet_addr(lan_ipaddr));
	acl_addr = acl_addr & ntohl(inet_addr(lan_netmask));
	
	ina.s_addr = htonl(acl_addr);
	
	sprintf(acl_mask, "%s/%s", inet_ntoa(ina), lan_netmask);
	
	fprintf(fp, "# %s\n\n", "auto-created file");
	
	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			
			if (!strcmp(system_type, "fuseblk"))
				continue;
			
			if (!strncmp(devname, "/dev/sd", 7) && !strncmp(mpname, "/media/", 7))
			{
				nfsmm = "rw";
				if (!strcmp(mount_mode, "ro"))
					nfsmm = "ro";
				fprintf(fp, "%s    %s(%s,async,insecure,no_root_squash,no_subtree_check)\n", mpname, acl_mask, nfsmm);
			}
		}
		
		fclose(procpt);
	}
	
	fclose(fp);
}

void run_nfsd()
{
	if (nvram_invmatch("nfsd_enable", "1")) 
		return;
	
	// always update nfsd exports
	write_nfsd_exports();
	
	eval("/usr/bin/nfsd.sh", "start");
}

int create_mp_link(char *search_dir, char *link_path, int force_first_valid)
{
	FILE *procpt;
	char line[256], devname[32], mpname[128], system_type[16], mount_mode[128], target_path[256];
	int dummy1, dummy2, link_created;
	
	link_created = 0;
	
	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			
			if (strncmp(devname, "/dev/sd", 7) == 0 && strncmp(mpname, "/media/", 7) == 0)
			{
				sprintf(target_path, "%s/%s", mpname, search_dir);
				if (!force_first_valid)
				{
					if (check_if_dir_exist(target_path))
					{
						if (symlink(target_path, link_path) == 0)
						{
							link_created = 1;
							break;
						}
					}
				}
				else
				{
					if (mkdir_if_none(target_path))
					{
						if (symlink(target_path, link_path) == 0)
						{
							link_created = 1;
							break;
						}
					}
				
				}
			}
		}
		
		fclose(procpt);
	}
	
	return link_created;
}

#if defined(APP_MINIDLNA)
int is_dms_support(void)
{
	return check_if_file_exist("/usr/bin/minidlna");
}

int is_dms_run(void)
{
	if (!is_dms_support())
		return 0;
	
	return (pids("minidlna")) ? 1 : 0;
}

void stop_dms(void)
{
	char* svcs[] = { "minidlna", NULL };
	
	if (!is_dms_support())
		return;
	
	kill_services(svcs, 5, 1);
}

void update_minidlna_conf(const char *link_path, const char *conf_path)
{
	FILE *fp;
	int dlna_disc;
	char *computer_name;
	char *dlna_src1 = "V,/media";
	char *dlna_src2 = "P,/media";
	char *dlna_src3 = "A,/media";
	
	fp = fopen(conf_path, "w");
	if (!fp)
		return;
	
	computer_name = nvram_safe_get("computer_name");
	if (!is_valid_hostname(computer_name))
		computer_name = nvram_safe_get("productid");
	
	dlna_disc= nvram_get_int("dlna_disc");
	dlna_src1 = nvram_safe_get("dlna_src1");
	dlna_src2 = nvram_safe_get("dlna_src2");
	dlna_src3 = nvram_safe_get("dlna_src3");
	
	if (!*dlna_src1 && !*dlna_src2 && !*dlna_src3)
		dlna_src1 = "/media";
	
	if (dlna_disc < 10) dlna_disc = 10;
	if (dlna_disc > 10800) dlna_disc = 10800;
	
	fprintf(fp, "port=%d\n", 8200);
	fprintf(fp, "network_interface=%s\n", IFNAME_BR);
	if (*dlna_src1)
		fprintf(fp, "media_dir=%s\n", dlna_src1);
	if (*dlna_src2)
		fprintf(fp, "media_dir=%s\n", dlna_src2);
	if (*dlna_src3)
		fprintf(fp, "media_dir=%s\n", dlna_src3);
	fprintf(fp, "friendly_name=%s\n", computer_name);
	fprintf(fp, "db_dir=%s\n", link_path);
	fprintf(fp, "log_dir=%s\n", link_path);
	fprintf(fp, "album_art_names=%s\n", "Cover.jpg/cover.jpg/AlbumArtSmall.jpg/albumartsmall.jpg/AlbumArt.jpg/albumart.jpg/Album.jpg/album.jpg/Folder.jpg/folder.jpg/Thumb.jpg/thumb.jpg");
	fprintf(fp, "inotify=%s\n", "yes");
	fprintf(fp, "enable_tivo=%s\n", "no");
	fprintf(fp, "strict_dlna=%s\n", "no");
	fprintf(fp, "notify_interval=%d\n", dlna_disc);
	fprintf(fp, "model_number=%d\n", 1);

	fclose(fp);
}

void run_dms(void)
{
	int db_rescan_mode;
	char *apps_name = "Media Server";
	char *link_path = "/mnt/minidlna";
	char *conf_path = "/etc/minidlna.conf";
	char *dest_dir = ".dms";
	char *minidlna_argv[] = {
		"/usr/bin/minidlna",
		"-f", conf_path,
		"-s", nvram_safe_get("br0hexaddr"),
		NULL,	/* -U */
		NULL
	};
	
	if (!nvram_match("apps_dms", "1"))
		return;
	
	if (!is_dms_support())
		return;
	
	if (is_dms_run())
		return;
	
	unlink(link_path);
	if (!create_mp_link(dest_dir, link_path, 0))
	{
		if (!create_mp_link(dest_dir, link_path, 1))
		{
			logmessage(apps_name, "Cannot start: unable to create DB dir (/%d) on any volumes!", dest_dir);
			return;
		}
	}
	
	update_minidlna_conf(link_path, conf_path);
	
	db_rescan_mode = nvram_get_int("dlna_rescan");
	if (db_rescan_mode == 1)
		minidlna_argv[5] = "-U";
	else if (db_rescan_mode == 2)
		minidlna_argv[5] = "-R";
	
	_eval(minidlna_argv, NULL, 0, NULL);
	
	if (is_dms_run())
		logmessage(apps_name, "daemon is started");
}

void restart_dms(void)
{
	stop_dms();
	if (count_sddev_mountpoint())
		run_dms();
}
#endif

#if defined(APP_FIREFLY)
int is_itunes_support(void)
{
	return check_if_file_exist("/usr/bin/mt-daapd");
}

int is_itunes_run(void)
{
	if (!is_itunes_support())
		return 0;
	
	return (pids("mt-daapd")) ? 1 : 0;
}

void stop_itunes(void)
{
	char* svcs[] = { "mt-daapd", NULL };
	
	if (!is_itunes_support())
		return;
	
	kill_services(svcs, 5, 1);
}

void update_firefly_conf(const char *link_path, const char *conf_path)
{
	FILE *fp;
	
	if (check_if_file_exist(conf_path))
		return;
	
	fp = fopen(conf_path, "w");
	if (!fp)
		return;
	
	fprintf(fp, "[general]\n");
	fprintf(fp, "web_root = %s\n", "/usr/share/mt-daapd");
	fprintf(fp, "port = %d\n", 3689);
	fprintf(fp, "runas = %s\n", nvram_safe_get("http_username"));
	fprintf(fp, "admin_pw = %s\n", nvram_safe_get("http_passwd"));
	fprintf(fp, "db_type = %s\n", "sqlite3");
	fprintf(fp, "db_parms = %s\n", link_path);
	fprintf(fp, "logfile = %s/mt-daapd.log\n", link_path);
	fprintf(fp, "servername = %s\n", "Firefly on %h");
	fprintf(fp, "mp3_dir = %s\n", "/media");
	fprintf(fp, "extensions = %s\n", ".mp3,.m4a,.m4p,.flac,.alac");
	fprintf(fp, "rescan_interval = %d\n", 300);
	fprintf(fp, "always_scan = %d\n", 0);
	fprintf(fp, "scan_type = %d\n", 0);
	fprintf(fp, "debuglevel = %d\n\n", 0);
	fprintf(fp, "[scanning]\n");
	fprintf(fp, "process_playlists = %d\n", 1);
	fprintf(fp, "process_itunes = %d\n", 1);
	fprintf(fp, "process_m3u = %d\n", 1);
	fprintf(fp, "mp3_tag_codepage = %s\n\n", "WINDOWS-1251");
	fprintf(fp, "[plugins]\n");
	fprintf(fp, "plugin_dir = %s\n\n", "/usr/lib/mt-daapd");

	fclose(fp);
}

void run_itunes(void)
{
	char *apps_name = "iTunes Server";
	char *link_path = "/mnt/firefly";
	char *conf_path = "/etc/storage/mt-daapd.conf";
	char *dest_dir = ".itunes";
	char *firefly_argv[] = {
		"/usr/bin/mt-daapd",
		"-c", conf_path,
		NULL
	};
	
	if (!nvram_match("apps_itunes", "1"))
		return;
	
	if (!is_itunes_support())
		return;
	
	if (is_itunes_run())
		return;
	
	unlink(link_path);
	if (!create_mp_link(dest_dir, link_path, 0))
	{
		if (!create_mp_link(dest_dir, link_path, 1))
		{
			logmessage(apps_name, "Cannot start: unable to create DB dir (/%d) on any volumes!", dest_dir);
			return;
		}
	}
	
	update_firefly_conf(link_path, conf_path);
	
	_eval(firefly_argv, NULL, 0, NULL);
	
	if (is_itunes_run())
		logmessage(apps_name, "daemon is started");
}

void restart_itunes(void)
{
	stop_itunes();
	if (count_sddev_mountpoint())
		run_itunes();
}
#endif

#if defined(APP_TRMD)
int is_torrent_support(void)
{
	return check_if_file_exist("/usr/bin/transmission-daemon");
}

int is_torrent_run(void)
{
	if (!is_torrent_support())
		return 0;
	
	return (pids("transmission-daemon")) ? 1 : 0;
}

void stop_torrent(void)
{
	if (!is_torrent_support())
		return;
	
	if (!is_torrent_run())
		return;
	
	eval("/usr/bin/transmission.sh", "stop");
}

void run_torrent(int no_restart_firewall)
{
	char *apps_name = "Transmission";
	char *link_path = "/mnt/transmission";
	char *dest_dir = "transmission";
	
	if (!nvram_match("trmd_enable", "1"))
		return;
	
	if (!is_torrent_support())
		return;
	
	if (is_torrent_run())
		return;
	
	unlink(link_path);
	if (!create_mp_link(dest_dir, link_path, 0))
	{
		logmessage(apps_name, "Cannot start: unable to find target dir (/%s) on any volumes!", dest_dir);
		return;
	}
	
	eval("/usr/bin/transmission.sh", "start");
	
	if (!no_restart_firewall && is_torrent_run() && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}

void restart_torrent(void)
{
	int is_run_before = is_torrent_run();
	int is_run_after;
	
	stop_torrent();
	if (count_sddev_mountpoint())
		run_torrent(1);
	
	is_run_after = is_torrent_run();
	
	if (is_run_after && !is_run_before && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}
#endif

#if defined(APP_ARIA)
int is_aria_support(void)
{
	return check_if_file_exist("/usr/bin/aria2c");
}

int is_aria_run(void)
{
	if (!is_aria_support())
		return 0;
	
	return (pids("aria2c")) ? 1 : 0;
}

void stop_aria(void)
{
	if (!is_aria_support())
		return;
	
	if (!is_aria_run())
		return;
	
	eval("/usr/bin/aria.sh", "stop");
}

void run_aria(int no_restart_firewall)
{
	char *apps_name = "Aria";
	char *link_path = "/mnt/aria";
	char *dest_dir = "aria";
	
	if (!nvram_match("aria_enable", "1"))
		return;
	
	if (!is_aria_support())
		return;
	
	if (is_aria_run())
		return;
	
	unlink(link_path);
	if (!create_mp_link(dest_dir, link_path, 0))
	{
		logmessage(apps_name, "Cannot start: unable to find target dir (/%s) on any volumes!", dest_dir);
		return;
	}
	
	eval("/usr/bin/aria.sh", "start");
	
	if (!no_restart_firewall && is_aria_run() && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}

void restart_aria(void)
{
	int is_run_before = is_aria_run();
	int is_run_after;
	
	stop_aria();
	if (count_sddev_mountpoint())
		run_aria(1);
	
	is_run_after = is_aria_run();
	
	if (is_run_after && !is_run_before && nvram_match("fw_enable_x", "1"))
		restart_firewall();
}
#endif

int start_networkmap(void)
{
	if (nvram_invmatch("wan_route_x", "IP_Routed"))
		return 0;
	
	eval("networkmap");
	
	return 0;
}

void stop_networkmap(void)
{
	char* svcs[] = { "networkmap", NULL };
	kill_services(svcs, 3, 1);
}

void restart_networkmap(void)
{
	if (pids("networkmap"))
	{
		system("killall -SIGUSR1 networkmap");
	}
	else
	{
		start_networkmap();
	}
	
	notify_watchdog_nmap();
}

FILE* fopen_or_warn(const char *path, const char *mode)
{
	FILE *fp = fopen(path, mode);

	if (!fp)
	{
		dbg("hotplug USB: No such file or directory: %s\n", path);
		errno = 0;
		return NULL;
	}

	return fp;
}

void
umount_ejected(void)	// umount mount point(s) which was(were) already ejected
{
	FILE *procpt, *procpt2;
	char line[256], devname[32], mpname[128], system_type[10], mount_mode[96], line2[128], ptname[32];
	int dummy1, dummy2, ma, mi, sz;
	int active = 0;

	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			if (strncmp(devname, "/dev/sd", 7) == 0)
			{
				active = 0;
				procpt2 = fopen("/proc/partitions", "r");
				if (procpt2)
				{
					while (fgets(line2, sizeof(line2), procpt2))
					{
						if (sscanf(line2, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
							continue;
						if (strcmp(devname+5, ptname) == 0)
						{
							active = 1;
							break;
						}
					}
					
					if (!active)
					{
						umount2(mpname, MS_NOEXEC | MS_NOSUID | 0x00000002);    // 0x00000002: MNT_DETACH
						rmdir(mpname);
					}
					
					fclose(procpt2);
				}
			}
		}
		
		fclose(procpt);
	}
}

void
umount_dev(char *sd_dev)	// umount sd_dev
{
	FILE *procpt;
	char line[256], devname[32], mpname[128], system_type[10], mount_mode[96];
	int dummy1, dummy2;

	if (!sd_dev)
		return;

	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			if (!strncmp(devname, "/dev/sd", 7))
			{
				if (!strcmp(devname+5, sd_dev))
				{
					eval("/usr/bin/opt-umount.sh", devname, mpname);
					umount2(mpname, MS_NOEXEC | MS_NOSUID | 0x00000002);    // 0x00000002: MNT_DETACH
					rmdir(mpname);
					break;
				}
			}
		}
		
		fclose(procpt);
	}
}

void
umount_dev_all(char *sd_dev)	// umount sd_dev
{
	FILE *procpt;
	char line[256], devname[32], mpname[128], system_type[10], mount_mode[96];
	int dummy1, dummy2;
	
	if (!sd_dev || !(*sd_dev))
		return;
	
	detach_swap_partition(sd_dev);
	
	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			if (!strncmp(devname, "/dev/sd", 7))
			{
				if (!strncmp(devname+5, sd_dev, 3))
				{
					eval("/usr/bin/opt-umount.sh", devname, mpname);
					umount2(mpname, MS_NOEXEC | MS_NOSUID | 0x00000002);    // 0x00000002: MNT_DETACH
					rmdir(mpname);
				}
			}
		}
		
		fclose(procpt);
	}
	
	if (!strncmp(sd_dev, "sd", 2))
	{
		doSystem("/sbin/spindown.sh %s", sd_dev);
	}
}

void
umount_sddev_all(void)	// umount all sdxx
{
	FILE *procpt;
	char line[256], devname[32], mpname[128], system_type[10], mount_mode[96];
	int dummy1, dummy2;
	
	detach_swap_partition(NULL);
	
	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
			if (!strncmp(devname, "/dev/sd", 7))
			{
				umount2(mpname, MS_NOEXEC | MS_NOSUID | 0x00000002);    // 0x00000002: MNT_DETACH
				rmdir(mpname);
			}
		}
		
		fclose(procpt);
	}
}

void
umount_usb_path(int port)
{
	if (port < 1 || port > 2)
		return;

	char nvram_name[20], sdx1[5];
	int i, len;
	
	for (i = 0; i < 16 ; i++)
	{
		sprintf(nvram_name, "usb_path%d_fs_path%d", port, i);
		if ((len = strlen(nvram_safe_get(nvram_name))))
		{
			umount_dev(nvram_safe_get(nvram_name));
			
			if (len == 3)
			{
				memset(sdx1, 0x0, 5);
				sprintf(sdx1, "%s1", nvram_safe_get(nvram_name));
				dbg("try to umount %s\n", sdx1);
				umount_dev(sdx1);
			}

			nvram_unset(nvram_name);
		}
	}

	sprintf(nvram_name, "usb_path%d_act", port);
	umount_dev_all(nvram_safe_get(nvram_name));
}

int
count_sddev_mountpoint(void)
{
	FILE *procpt;
	char line[256], devname[32], mpname[128], system_type[10], mount_mode[96];
	int dummy1, dummy2, count = 0;

	procpt = fopen("/proc/mounts", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
				
			if (strstr(devname, "/dev/sd"))
				count++;
		}
		
		fclose(procpt);
	}
	
	return count;
}


int
count_sddev_partition(void)
{
	FILE *procpt;
	char line[256], ptname[32];
	int ma, mi, sz, count = 0;

	procpt = fopen("/proc/partitions", "r");
	if (procpt)
	{
		while (fgets(line, sizeof(line), procpt))
		{
			if (sscanf(line, " %d %d %d %[^\n ]", &ma, &mi, &sz, ptname) != 4)
				continue;
			
			if (!strncmp(ptname, "sd", 2))
				count++;
		}

		fclose(procpt);
	}

	return count;
}


int
is_invalid_char_for_hostname(char c)
{
	int ret = 0;

	if (c < 0x20)
		ret = 1;
	else if (c >= 0x21 && c <= 0x2c)
		ret = 1;
	else if (c >= 0x2e && c <= 0x2f)
		ret = 1;
	else if (c >= 0x3a && c <= 0x40)
		ret = 1;
#if 0
	else if (c >= 0x5b && c <= 0x60)
		ret = 1;
#else
	else if (c >= 0x5b && c <= 0x5e)
		ret = 1;
	else if (c == 0x60)
		ret = 1;
#endif
	else if (c >= 0x7b)
		ret = 1;

//	printf("%c (0x%02x) is %svalid for hostname\n", c, c, (ret == 0) ? "  " : "in");

	return ret;
}

int
is_valid_hostname(const char *name)
{
	int len, i;

	len = strlen(name);
	if (len < 1)
	{
		return 0;
	}

	for (i = 0; i < len ; i++)
	{
		if (is_invalid_char_for_hostname(name[i]))
		{
			return 0;
		}
	}

	return 1;
}

void stop_usb(void)
{
	stop_usb_printer_spoolers();
	
	safe_remove_usb_mass(0);
}

void stop_usb_apps(void)
{
	stop_nfsd();
	stop_samba();
	stop_ftp();
#if defined(APP_MINIDLNA)
	stop_dms();
#endif
#if defined(APP_FIREFLY)
	stop_itunes();
#endif
#if defined(APP_TRMD)
	stop_torrent();
#endif
#if defined(APP_ARIA)
	stop_aria();
#endif
}

void start_usb_apps(void)
{
	run_ftp();
	run_samba();
	run_nfsd();
#if defined(APP_MINIDLNA)
	run_dms();
#endif
#if defined(APP_FIREFLY)
	run_itunes();
#endif
#if defined(APP_TRMD)
	run_torrent(0);
#endif
#if defined(APP_ARIA)
	run_aria(0);
#endif
}

void try_start_usb_apps(void)
{
	// start apps if needed
	if (count_sddev_mountpoint())
		start_usb_apps();
}

int is_usb_printer_exist(char devlp[16])
{
	char *usb_path_act;
	
	if (nvram_match("usb_path1", "printer")) {
		usb_path_act = nvram_safe_get("usb_path1_act");
		if (strncmp(usb_path_act, "lp", 2) == 0) {
			sprintf(devlp, "/dev/%s", usb_path_act);
			return 1;
		}
	}
	
	if (nvram_match("usb_path2", "printer")) {
		usb_path_act = nvram_safe_get("usb_path2_act");
		if (strncmp(usb_path_act, "lp", 2) == 0) {
			sprintf(devlp, "/dev/%s", usb_path_act);
			return 1;
		}
	}
	
	return 0;
}

void try_start_usb_printer_spoolers(void)
{
	char *opt_printer_script = "/opt/bin/on_hotplug_printer.sh";
	char devlp[16] = {0};
	
	if (is_usb_printer_exist(devlp))
	{
		if (check_if_file_exist(opt_printer_script))
		{
			doSystem("%s %s", opt_printer_script, devlp);
		}
		
		start_u2ec();
		start_lpd();
		start_p910nd(devlp);
	}
}

void stop_usb_printer_spoolers(void)
{
	stop_u2ec();
	stop_lpd();
	stop_p910nd();
}

void restart_usb_printer_spoolers(void)
{
	char devlp[16] = {0};
	int has_printer = is_usb_printer_exist(devlp);
	
	stop_p910nd();
	
	if (!has_printer || nvram_match("lprd_enable", "0"))
		stop_lpd();
	
	if (!has_printer || nvram_match("u2ec_enable", "0"))
		stop_u2ec();
	
	// check printer exist again
	has_printer = is_usb_printer_exist(devlp);
	if (has_printer)
	{
		start_u2ec();
		start_lpd();
		start_p910nd(devlp);
	}
}

void try_start_usb_modem_to_wan(void)
{
	int link_wan = 0;
	int modem_rule = nvram_get_int("modem_rule");
	int modem_type = nvram_get_int("modem_type");
	int modem_arun = nvram_get_int("modem_arun");
	
	if (is_ap_mode())
		return;
	
	if (modem_rule < 1 || modem_arun < 1)
		return;
	
	if (get_usb_modem_state())
		return;
	
	if (modem_type == 3)
	{
		if ( !is_ready_modem_4g() )
			return;
	}
	else
	{
		if ( !is_ready_modem_3g() )
			return;
	}
	
	if (modem_arun == 2)
	{
		if (phy_status_port_link_wan(&link_wan) == 0 && link_wan && has_wan_ip(1) && found_default_route(1))
			return;
	}
	
	logmessage("usb modem hotplug", "try autorun modem wan connection...");
	
	try_wan_reconnect(1);
}

void on_deferred_hotplug_usb(void)
{
	if (nvram_match("usb_hotplug_ms", "1"))
	{
		nvram_set("usb_hotplug_ms", "0");
		try_start_usb_apps();
	}

	if (nvram_match("usb_hotplug_lp", "1"))
	{
		nvram_set("usb_hotplug_lp", "0");
		try_start_usb_printer_spoolers();
	}

	if (nvram_match("usb_hotplug_md", "1"))
	{
		nvram_set("usb_hotplug_md", "0");
		try_start_usb_modem_to_wan();
	}
}

