/*
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#ifdef ASUS_EXT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include <sys/mount.h>
#include <nvram/bcmnvram.h>
#include <netconf.h>
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
#include <rc_event.h>
#include <dongles.h>

#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <scsi/scsi.h>
#include <scsi/scsi_ioctl.h>
#include <linux/autoconf.h>

#include <semaphore_mfp.h>
#include <disk_io_tools.h>
#include <disk_initial.h>
#include <disk_share.h>


#define logs(s) syslog(LOG_NOTICE, s)


//#define WEBCAM_SUPPORT 1
#define PRINTER_SUPPORT 1
#define MASSSTORAGE_SUPPORT 1


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

int 
start_infosvr(void)
{
	return system("/usr/sbin/infosvr br0 &");
}

int
start_8021x(void)
{
	if (	nvram_match("wl_auth_mode", "wpa") || 
		nvram_match("wl_auth_mode", "radius") || 
		nvram_match("wl_auth_mode", "wpa2") )
	{
		if (pids("rt2860apd"))
			system("killall rt2860apd");

		dbg("8021X daemon for 5G...\n");
		return doSystem("rt2860apd");
	}
	else
		return 0;
}

int
start_8021x_rt(void)
{
	if (	nvram_match("rt_auth_mode", "wpa") || 
		nvram_match("rt_auth_mode", "radius") || 
		nvram_match("rt_auth_mode", "wpa2") )
	{
		if (pids("rtinicapd"))
			system("killall rtinicapd");

		dbg("8021X daemon for 2.4G...\n");
		return doSystem("rtinicapd");
	}
	else
		return 0;
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
	int i, i_max, i_sdhcp;
	char dhcp_mac[32], dhcp_ip[32], dhcp_name[32], *smac, *sip, *sname;
	char *start, *end, *ipaddr, *mask;
	char dhcp_start[16], dhcp_end[16], lan_ipaddr[16], lan_netmask[16];
	size_t ethers = 0;
	char *resolv_conf = "/etc/resolv.conf";
	char *leases_dhcp = "/tmp/dnsmasq.leases";
	char *host_name_nbt;
	
	if (nvram_match("router_disable", "1"))
		return 0;
	
	ipaddr = nvram_safe_get("lan_ipaddr");
	
	host_name_nbt = nvram_safe_get("computer_name");
	if (!host_name_nbt[0] || !is_valid_hostname(host_name_nbt))
		host_name_nbt = nvram_safe_get("productid");
	
	i_sdhcp = atoi(nvram_safe_get("dhcp_static_x"));
	i_max  = atoi(nvram_safe_get("dhcp_staticnum_x"));
	if (i_max > 64) i_max = 64;
	
	/* create /etc/hosts */
	fp = fopen("/etc/hosts", "w+");
	if (fp) {
		fprintf(fp, "127.0.0.1 localhost.localdomain localhost\n");
		fprintf(fp, "%s my.router\n", ipaddr);
		fprintf(fp, "%s my.%s\n", ipaddr, nvram_safe_get("productid"));
		fprintf(fp, "%s %s\n", ipaddr, host_name_nbt);
		if (i_sdhcp == 1) {
			for (i = 0; i < i_max; i++) {
				sprintf(dhcp_ip, "dhcp_staticip_x%d", i);
				sprintf(dhcp_name, "dhcp_staticname_x%d", i);
				sip = nvram_safe_get(dhcp_ip);
				sname = nvram_safe_get(dhcp_name);
				if (inet_addr_(sip) != INADDR_ANY && inet_addr_(sip) != inet_addr_(ipaddr) && is_valid_hostname(sname))
				{
					fprintf(fp, "%s %s\n", sip, sname);
				}
			}
		}
		fclose(fp);
	}
	
	/* touch resolv.conf if not exist */
	spinlock_lock(SPINLOCK_DNSRenew);
	fp = fopen(resolv_conf, "a+");
	if (fp)
		fclose(fp);
	spinlock_unlock(SPINLOCK_DNSRenew);
	
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
		    "listen-address=%s,127.0.0.1\n"
		    "bind-interfaces\n", resolv_conf, ipaddr);
		
	if (nvram_invmatch("lan_domain", "")) {
		fprintf(fp, "domain=%s\n"
			    "expand-hosts\n", nvram_get("lan_domain"));
	}
	
	fprintf(fp, "no-negcache\n"
		    "cache-size=1000\n"
		    "clear-on-reload\n");
	
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
		strcpy(dhcp_end,   *start&&*end?end:"192.168.1.254");
		strcpy(lan_netmask,*start&&*end&&*mask?mask:"255.255.255.0");
		
		if (!chk_valid_startend(lan_ipaddr, dhcp_start, dhcp_end, lan_netmask))
		{
			nvram_set("dhcp_start", dhcp_start);
			nvram_set("dhcp_end", dhcp_end);
		}
		
		fprintf(fp, "dhcp-range=%s,%s,%s,%s\n", dhcp_start, dhcp_end, lan_netmask, nvram_safe_get("dhcp_lease"));
		fprintf(fp, "dhcp-leasefile=%s\n", leases_dhcp);
		fprintf(fp, "dhcp-authoritative\n");
		
		/* GATEWAY */
		if (nvram_invmatch("dhcp_gateway_x", ""))
			fprintf(fp, "dhcp-option=3,%s\n", nvram_safe_get("dhcp_gateway_x"));
		else
			fprintf(fp, "dhcp-option=3,%s\n", lan_ipaddr);
		
		/* DNS */
		fprintf(fp, "dhcp-option=6");
		if (nvram_invmatch("dhcp_dns1_x", "") && nvram_invmatch("dhcp_dns1_x", lan_ipaddr))
			fprintf(fp, ",%s", nvram_safe_get("dhcp_dns1_x"));
		if (nvram_invmatch("dhcp_dns2_x", "") && nvram_invmatch("dhcp_dns2_x", lan_ipaddr))
			fprintf(fp, ",%s", nvram_safe_get("dhcp_dns2_x"));
		fprintf(fp, ",%s\n", lan_ipaddr);
		
		/* DOMAIN */
		if (nvram_invmatch("lan_domain", ""))
			fprintf(fp, "dhcp-option=15,%s\n", nvram_safe_get("lan_domain"));
		
		/* WINS */
		if (nvram_invmatch("dhcp_wins_x", ""))
			fprintf(fp, "dhcp-option=44,%s\n", nvram_safe_get("dhcp_wins_x"));
		
		if (ethers)
			fprintf(fp, "read-ethers\n");
	}
	
	fclose(fp);
	
	return eval("dnsmasq");
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
	return system("killall -SIGHUP dnsmasq");
}


extern int valid_url_filter_time();

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
	
	logmessage("DDNS", "DDNS updated IP address [%s] SUCCESS!", ip+1);
	
	nvram_set("ddns_reset_t", "1");
	system("killall -SIGHUP watchdog");
	
	// Purge dnsmasq cache
	restart_dns();
	
	return 0;
}

void
stop_ddns(void)
{
	char* svcs[] = { "ez-ipupdate", "inadyn", NULL };
	
	if (pids("ez-ipupdate")) {
		system("killall -SIGINT ez-ipupdate");
		sleep(1);
	}
	
	kill_services(svcs, 3, 1);
}

int 
start_ddns(int forced)
{
	FILE *fp = NULL;
	char *wan_ip, *ddns_ip, *ddns_srv, *ddns_hnm, *ddns_hnm2, *ddns_hnm3, *ddns_user, *ddns_pass, *ddns_result;
	char service[32];
	char wan_ifname[16];
	
	if (nvram_match("router_disable", "1")) return -1;
	if (!nvram_match("ddns_enable_x", "1")) return -1;
	
	wan_ip = nvram_safe_get("wan_ipaddr_t");
	
	if (inet_addr_(wan_ip) == INADDR_ANY) return -1;
	
	ddns_ip       = nvram_safe_get("ddns_ipaddr");
	ddns_srv      = nvram_safe_get("ddns_server_x");
	ddns_hnm      = nvram_safe_get("ddns_hostname_x");
	ddns_hnm2     = nvram_safe_get("ddns_hostname2_x");
	ddns_hnm3     = nvram_safe_get("ddns_hostname3_x");
	
	logmessage(LOGNAME, "DDNS: Start update DDNS...");
	
	if ( (!forced) && (inet_addr_(wan_ip) == inet_addr_(ddns_ip)) && (nvram_match("ddns_updated", "1")) )
	{
		logmessage(LOGNAME, "DDNS: WAN IP address [%s] not changed since the last update. Skip update.", wan_ip);
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
	
#ifdef ASUS_DDNS
	if (!strcmp(ddns_srv, "WWW.ASUS.COM"))
	{
		wan_ifname[0] = 0;
		get_wan_ifname(wan_ifname);
		
		unlink("/var/run/ez-ipupdate.block");
		
		nvram_set("ddns_return_code", "ddns_query");
		
		doSystem("ez-ipupdate -h %s -s ns1.asuscomm.com -S dyndns -i %s -A 1", ddns_hnm, wan_ifname);
		
		ddns_result = nvram_safe_get("ddns_return_code");
		if ( !strcmp(ddns_result, "register,200") ||
		     !strcmp(ddns_result, "register,220") ||
		     !strcmp(ddns_result, "register,230") )
		{
			nvram_set("ddns_ipaddr", wan_ip);
			nvram_set("ddns_updated", "1");
			
			logmessage("DDNS", "DDNS updated IP address [%s] SUCCESS!", wan_ip);
			
			restart_dns();
		}
	}
	else
#endif
	{
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
		else if (strcmp(ddns_srv, "FREEDNS.AFRAID.ORG") == 0)
			strcpy(service, "default@freedns.afraid.org");
		else if (strcmp(ddns_srv, "DNS.HE.NET") == 0)
			strcpy(service, "dyndns@he.net");
		else
			strcpy(service, "default@dyndns.org");
		
		ddns_user = nvram_safe_get("ddns_username_x");
		ddns_pass = nvram_safe_get("ddns_passwd_x");
		
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
			
			if (*ddns_hnm2) {
				fprintf(fp, "alias %s\n", ddns_hnm2);
			}
			
			if (*ddns_hnm3) {
				fprintf(fp, "alias %s\n", ddns_hnm3);
			}
			
			if (nvram_match("ddns_wildcard_x", "1")) {
				fprintf(fp, "wildcard\n");
			}
			
			fprintf(fp,
				"background\n"
				"iterations 1\n"
				"cache_file /tmp/ddns.cache\n"
				"exec /sbin/ddns_updated\n");
			
			fclose(fp);
		}
		
		eval("inadyn", "--input_file", "/etc/inadyn.conf");
	}
	
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
		return eval("/sbin/syslogd", "-m", "0", "-S", "-O", "/tmp/syslog.log", "-R", nvram_safe_get("log_ipaddr"), "-L");
	}
	else
	{
		return eval("/sbin/syslogd", "-m", "0", "-S", "-O", "/tmp/syslog.log");
	}
}

int
start_klogd()
{
	return eval("/sbin/klogd");
}

int 
start_misc(void)
{ 
	char *watchdog_argv[] = {"watchdog", NULL};
	pid_t pid;
	
	system("/usr/sbin/infosvr br0 &");

	_eval(watchdog_argv, NULL, 0, &pid);

	return 0;
}

void
stop_misc(void)
{
	dbg("stop_misc()\n");

	char* svcs[] = { "infosvr", 
			"watchdog", 
			"ntp", 
			"ntpclient", 
			"pspfix", 
			"tcpcheck", 
			"detectWan", 
			NULL 
			};
	
	kill_services(svcs, 3, 1);

	stop_wsc();
	stop_wsc_2g();
	stop_lltd();
	stop_detect_internet();
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	stop_rstats();
#endif
}

void
stop_misc_no_watchdog(void)
{
	dbg("stop_misc_no_watchdog()\n");

	char* svcs[] = { "infosvr", 
			"ntp", 
			"ntpclient", 
			"pspfix", 
			"tcpcheck", 
			"detectWan", 
			NULL 
			};
	
	kill_services(svcs, 3, 1);
	
	stop_wsc();
	stop_wsc_2g();
	stop_lltd();	// 1017 add
	stop_detect_internet();
#if (!defined(W7_LOGO) && !defined(WIFI_LOGO))
	stop_rstats();
#endif
	dprintf("done\n");
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
				safe_remove_usb_modem();
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
				safe_remove_usb_modem();
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

int stop_service_type_99 = 0;

/* stop necessary services for firmware upgrade */	
/* stopservice: for firmware upgarde */
/* stopservice 1: for button setup   */
int
stop_service_main(int type)
{
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
			stop_service_type_99 = 1;
			stop_misc_no_watchdog();
		}
		else
		{
			stop_misc();
		}
		
		stop_services(0); // don't stop telnetd/sshd/vpn
		stop_usb();
		
		stop_igmpproxy();
		
		stop_ots();
		stop_networkmap();
	}
	
	return 0;
}

void manual_wan_disconnect(void)
{
	logmessage("wan", "perform manual disconnect");
	
	if (get_usb_modem_state()){
		stop_wan_ppp();
	}
	else
	if (nvram_match("wan0_proto", "dhcp") ||
		nvram_match("wan0_proto", "bigpond"))
	{	/* dhcp */
		logmessage("service_handle", "perform DHCP release");
		release_udhcpc_wan(0);
	}
	else if (
	nvram_match("wan0_proto", "pptp")  ||
	nvram_match("wan0_proto", "pppoe") ||
	nvram_match("wan0_proto", "l2tp")
	)
	{	/* pptp, l2tp, pppoe */
		dbg("stop wan ppp manually\n");	// tmp test
		stop_wan_ppp();
	}
	else 	/* static */
	{			
		dbg("services stop wan2 \n");	// tmp test
		stop_wan_static();
		update_wan_status(0);
		
		if (nvram_match("wan0_proto", "static"))
		{
			system("ifconfig eth3 0.0.0.0");
		}
	}
}

void manual_wan_connect(void)
{
	logmessage("wan", "perform manual connect");
	
	stop_wan();
	reset_wan_vars(0);
	select_usb_modem_to_wan(0);
	start_wan();
}

int service_handle(void)
{
	char *service;
	char wan_ifname[16];
	
	service = nvram_get("rc_service");
	
	if (strstr(service,"wan_disconnect")!=NULL)
	{
		manual_wan_disconnect();
	}
	else if (strstr(service,"wan_connect")!=NULL)
	{
		manual_wan_connect();
	}
#ifdef ASUS_DDNS //2007.03.26 Yau add for asus ddns
	else if (strstr(service,"ddns_hostname_check"))
	{
		wan_ifname[0] = 0;
		get_wan_ifname(wan_ifname);
		
		//Execute ez-ipupdate then die.
		if (pids("ez-ipupdate"))
		{
			system("killall -SIGINT ez-ipupdate");
			sleep(1);
		}
		
		nvram_set("ddns_return_code", "ddns_query");
		
		doSystem("ez-ipupdate -h %s -s ns1.asuscomm.com -S dyndns -i %s -A 1", nvram_safe_get("ddns_hostname_x"), wan_ifname);
	}
#endif
	nvram_unset("rc_service");
	return 0;
}

#endif //ASUS_EXT


void
start_u2ec(void)
{
#ifdef U2EC
	if (nvram_match("u2ec_enable", "0")) 
		return;
	
	unlink("/var/run/u2ec.pid");
	system("/usr/sbin/u2ec &");
	nvram_set("apps_u2ec_ex", "1");
#endif
}

void
stop_u2ec(void)
{
#ifdef U2EC
	char* svcs[] = { "u2ec",  NULL };
	kill_services(svcs, 3, 1);
#endif
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
start_p910nd(void)
{
	if (nvram_match("rawd_enable", "0")) 
		return;
	
	if (nvram_match("rawd_enable", "2"))
		eval("/usr/sbin/p910nd", "-b", "-f", "/dev/lp0", "0");
	else
		eval("/usr/sbin/p910nd", "-f", "/dev/lp0", "0");
}

void 
stop_p910nd(void)
{
	char* svcs[] = { "p910nd", NULL };
	kill_services(svcs, 3, 1);
}


void stop_ftp(void)
{
	char* svcs[] = { "vsftpd", NULL };
	kill_services(svcs, 5, 1);
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

void stop_torrent(int no_restart_firewall)
{
	if (!is_torrent_support())
		return;
	
	if (!is_torrent_run())
		return;
	
	eval("/usr/bin/transmission.sh", "stop");
	
	if (!no_restart_firewall && !is_torrent_run())
		rc_restart_firewall();
}

void write_vsftpd_conf(void)
{
	FILE *fp;
	char maxuser[16];

	fp=fopen("/tmp/vsftpd.conf", "w");
	if (fp==NULL) return;
	
	fprintf(fp, "background=YES\n");
	
	if (nvram_match("st_ftp_mode", "2"))
		fprintf(fp, "anonymous_enable=NO\n");
	else{
		fprintf(fp, "anonymous_enable=YES\n");
		fprintf(fp, "anon_upload_enable=YES\n");
		fprintf(fp, "anon_mkdir_write_enable=YES\n");
		fprintf(fp, "anon_other_write_enable=YES\n");
	}
	
	fprintf(fp, "nopriv_user=root\n");
	fprintf(fp, "write_enable=YES\n");
	fprintf(fp, "local_enable=YES\n");
	fprintf(fp, "force_dot_files=YES\n");
	fprintf(fp, "chroot_local_user=YES\n");
	fprintf(fp, "local_umask=000\n");
	fprintf(fp, "dirmessage_enable=NO\n");
	fprintf(fp, "xferlog_enable=NO\n");
	fprintf(fp, "syslog_enable=NO\n");
	fprintf(fp, "connect_from_port_20=YES\n");
	fprintf(fp, "listen=YES\n");
	fprintf(fp, "pasv_enable=YES\n");
	fprintf(fp, "ssl_enable=NO\n");
	fprintf(fp, "tcp_wrappers=NO\n");
	strcpy(maxuser, nvram_safe_get("st_max_user"));
	if ((atoi(maxuser)) > 0)
		fprintf(fp, "max_clients=%s\n", maxuser);
	else
		fprintf(fp, "max_clients=%s\n", "10");
	fprintf(fp, "ftp_username=anonymous\n");
	fprintf(fp, "ftpd_banner=Welcome to ASUS %s FTP service.\n", nvram_safe_get("productid"));
	
	fclose(fp);
}


void run_ftp()
{
	if (stop_service_type_99)
		return;
	
	if (nvram_match("enable_ftp", "0")) 
		return;
	
	if (pids("vsftpd"))
		return;

	write_vsftpd_conf();

	eval("/sbin/vsftpd");

	if (pids("vsftpd"))
		logmessage("FTP server", "daemon is started");
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

	memset(target, 0, 256);
	sprintf(target, "[%s]", string);

	memset(buf, 0, 4096);
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
	int n=0, sh_num=0;
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
	if (strcmp(nvram_safe_get("st_max_user"), "") != 0)
		fprintf(fp, "max connections = %s\n", nvram_safe_get("st_max_user"));
	
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
	fprintf(fp, "interfaces = lo br0\n");	// J++
	fprintf(fp, "use sendfile = yes\n");
	fprintf(fp, "unix extensions = no\n");				// Padavan, fix for MAC users (thanks mark2qualis)

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
		sh_num = atoi(nvram_safe_get("sh_num"));
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

	if (stop_service_type_99)
		return;

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
	
	sh_num = atoi(nvram_safe_get("acc_num"));
	if (sh_num > 100) sh_num = 100;
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
			
			if (!strncmp(devname, "/dev/sd", 7) && !strncmp(mpname, "/media/AiDisk_", 14))
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
	if (stop_service_type_99)
		return;

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
			
			if (strncmp(devname, "/dev/sd", 7) == 0 && strncmp(mpname, "/media/AiDisk_", 14) == 0)
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

void update_minidlna_conf(void)
{
	FILE *fp;
	char *computer_name;
	char *media_source = "/media";
	char *minidlna_conf = "/etc/minidlna.conf";
	
	unlink(minidlna_conf);
	
	fp = fopen(minidlna_conf, "w");
	if (!fp)
		return;
	
	computer_name = nvram_safe_get("computer_name");
	if (!is_valid_hostname(computer_name))
		computer_name = nvram_safe_get("productid");
	
	if (nvram_invmatch("dlna_source", ""))
		media_source = nvram_safe_get("dlna_source");
	
	fprintf(fp, "port=%d\n", 8200);
	fprintf(fp, "network_interface=%s\n", "br0");
	fprintf(fp, "media_dir=%s\n", media_source);
	fprintf(fp, "friendly_name=%s\n", computer_name);
	fprintf(fp, "db_dir=%s\n", "/mnt/minidlna");
	fprintf(fp, "log_dir=%s\n", "/mnt/minidlna");
	fprintf(fp, "album_art_names=%s\n", "Cover.jpg/cover.jpg/AlbumArtSmall.jpg/albumartsmall.jpg/AlbumArt.jpg/albumart.jpg/Album.jpg/album.jpg/Folder.jpg/folder.jpg/Thumb.jpg/thumb.jpg");
	fprintf(fp, "inotify=%s\n", "yes");
	fprintf(fp, "enable_tivo=%s\n", "no");
	fprintf(fp, "strict_dlna=%s\n", "no");
	fprintf(fp, "notify_interval=%d\n", 90);
	fprintf(fp, "model_number=%d\n", 1);
	fclose(fp);
}

void run_dms(void)
{
	int db_rescan_mode;
	char *apps_name = "Media Server";
	char *link_path = "/mnt/minidlna";
	char *dest_dir = ".dms";
	char *minidlna_argv[] = {
		"/usr/bin/minidlna",
		"-f", "/etc/minidlna.conf",
		"-s", nvram_safe_get("br0hexaddr"),
		NULL,	/* -U */
		NULL
	};
	
	if (stop_service_type_99)
		return;
	
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
			logmessage(apps_name, "Cannot start: unable to create DB dir (/.dms) on any volumes!");
			return;
		}
	}
	
	update_minidlna_conf();
	
	db_rescan_mode = atoi(nvram_safe_get("dlna_rescan"));
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

void run_torrent(int no_restart_firewall)
{
	char *apps_name = "Transmission";
	char *link_path = "/mnt/transmission";
	char *dest_dir = "transmission";
	
	if (stop_service_type_99)
		return;
	
	if (!nvram_match("trmd_enable", "1"))
		return;
	
	if (!is_torrent_support())
		return;
	
	if (is_torrent_run())
		return;
	
	unlink(link_path);
	if (!create_mp_link(dest_dir, link_path, 0))
	{
		logmessage(apps_name, "Cannot start: unable to find target dir (/transmission) on any volumes!");
		return;
	}
	
	eval("/usr/bin/transmission.sh", "start");
	
	if (!no_restart_firewall && is_torrent_run())
		rc_restart_firewall();
}

void restart_torrent(void)
{
	int is_run_before = is_torrent_run();
	int is_run_after;
	
	stop_torrent(1);
	if (count_sddev_mountpoint())
		run_torrent(1);
	
	is_run_after = is_torrent_run();
	
	if (is_run_after != is_run_before || is_run_after)
		rc_restart_firewall();
}

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
	
	nvram_set("nmap_reset_t", "1");
	system("killall -SIGHUP watchdog");
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
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96], line2[128], ptname[32];
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
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96];
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
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96];
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
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96];
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
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96];
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
	stop_u2ec();
	stop_lpd();
	stop_p910nd();
	
	safe_remove_usb_mass(0);
}

void stop_usb_apps(void)
{
	stop_nfsd();
	stop_samba();
	stop_ftp();
	stop_dms();
	stop_torrent(0);
}

void start_usb_apps(void)
{
	run_ftp();
	run_samba();
	run_nfsd();
	run_dms();
	run_torrent(0);
}

void on_hotplug_usb_storage(void)
{
	// start apps if needed
	if (count_sddev_mountpoint())
		start_usb_apps();
}

void on_removal_usb_storage(void)
{
	// restart apps if needed
	if (count_sddev_mountpoint())
		start_usb_apps();
}

int is_usb_printer_exist(void)
{
	if (strcmp(nvram_safe_get("usb_path1"), "printer") == 0 ||
	    strcmp(nvram_safe_get("usb_path2"), "printer") == 0)
		return 1;
	
	return 0;
}

void on_hotplug_usb_printer(void)
{
	if (is_usb_printer_exist())
	{
		start_u2ec();
		start_lpd();
		start_p910nd();
	}
}

void on_removal_usb_printer(void)
{
	if (!is_usb_printer_exist())
	{
		stop_u2ec();
		stop_lpd();
		stop_p910nd();
	}
}

void restart_usb_printer_spoolers(void)
{
	int has_printer = is_usb_printer_exist();
	
	stop_p910nd();
	
	if (!has_printer || nvram_match("lprd_enable", "0"))
		stop_lpd();
	
	if (!has_printer || nvram_match("u2ec_enable", "0"))
		stop_u2ec();
	
	// check printer exist again
	has_printer = is_usb_printer_exist();
	if (has_printer)
	{
		start_u2ec();
		start_lpd();
		start_p910nd();
	}
}


/*
#ifdef U2EC
#define U2EC_FIFO "/var/u2ec_fifo"
#endif


#ifdef U2EC
	int u2ec_fifo;
#endif

#ifdef U2EC
	u2ec_fifo = open(U2EC_FIFO, O_WRONLY|O_NONBLOCK);
	write(u2ec_fifo, "r", 1);
	close(u2ec_fifo);
#endif

#ifdef U2EC
	u2ec_fifo = open(U2EC_FIFO, O_WRONLY|O_NONBLOCK);
	write(u2ec_fifo, "a", 1);
	close(u2ec_fifo);
#endif

*/